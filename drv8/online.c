/*********************************************************
 *                   CTHULHUMUD                       	 *
 * CthulhuMud  driver  version 8.x copyright (C) 2000 	 *
 * by Mik Clarke (mykael@vianet.net.au)                  *
 * and Joachim Häusler (mystery@chello.at).              *
 *                                                       *
 * While the code is original, many concepts and the     *
 * typical look & feel are derived from MERC and its     *
 * derivatives - especially the SunderMud 1.0 prototype. *
 *                                                       *
 * Therefore we'd like to thank:                         *
 * Lotherius                                             *
 * Russ Taylor, Gabrielle Taylor, Brian Moore            *
 * Michael Chastain, Michael Quan, and Mitchell Tse      *
 *                                                       *
 * Please keep this code open-source and share your      *
 * ideas. It's still a long way to go.                   *
 *********************************************************/


#include "everything.h"
#include "profile.h"
#include "version.h"
#include "online.h"

/* Write out online.html file... */

struct who_slot {
    CHAR_DATA *ch;
    struct who_slot *next;
};

void scribe_online() {
    char cmdbuf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int immMatch=0;
    int mortMatch=0;
    struct who_slot *table[MAX_LEVEL + 1]; /*want 1 to 100, not 0 to 99*/ 
    int counter;
    bool found; 
    FILE *fp;

   /* Open the output file... */ 

    fp = fopen( ONLINE_FILE_NEW, "w");

    if (fp == NULL) {
      bug("Unable to open new online file!", 0);
      return;
    }

   /* Initialize the data tables... */

    for (counter=1; counter <= MAX_LEVEL; counter++) {
        table[counter] = NULL;
    }

   /* Sort the players... */

    for ( d = descriptor_list; d != NULL; d = d->next ) {
      CHAR_DATA *wch;
	
     /* Check for match against restrictions... */
     /* Don't use trust as that exposes trusted mortals... */ 

      wch   = ( d->original != NULL ) ? d->original : d->character;
  
      if (  d->connected != CON_PLAYING  ) {
        continue;
      }

      if (IS_IMMORTAL(wch)) {
        immMatch++;
      } else {
	mortMatch++;
      }
	
      if (table[wch->level] == NULL) { 
 		
        table[wch->level] = (struct who_slot *)malloc(sizeof(struct who_slot)); 
	table[wch->level]->ch = wch;
	table[wch->level]->next = NULL;
      } else {
        struct who_slot *tmp = table[wch->level];

	for (;tmp->next != NULL;tmp = tmp->next); {
	  tmp->next = (struct who_slot *)malloc(sizeof(struct who_slot)); 
        }

	tmp->next->ch = wch;
	tmp->next->next = NULL;
      }
    } 

   /* Header and intro... */

    fprintf(fp, "<HTML>\n"
                "<HEAD>\n"
                "<TITLE>%s Status</TITLE>\n"
                "<META HTTP-EQUIV='content control' CONTENT='no-cache'>\n"
                "<META HTTP-EQUIV='pragma' CONTENT='no-cache'>\n"
                "</HEAD>\n"
                "<BODY>\n"
                "<H3>Status</H3>\n"
                "<P>%s is Online</P>"
                "<HR>" 
                "<H3>The Gods of %s</H3>\n",
                 mud.name, 
                 mud.name,
                 mud.name);
         
   /* Output gods... */

    found = FALSE;

    for ( counter=MAX_LEVEL;counter>=1;counter--) { /*outside list loop*/
      CHAR_DATA *wch;
      struct who_slot *tmp=table[counter];

      if (tmp == NULL) continue;

     /* now, follow each chain to end */

      for ( ;tmp != NULL;tmp = tmp->next) {	
	
        wch = tmp->ch;

        if (wch == NULL) {
                  log_string ("Got null table->ch, argh.");
	  continue;
        }

        if (IS_MORTAL(wch)) continue;

        if (!found) {	
          fprintf(fp, "<MENU>\n");
          found = TRUE;
        }
 
        fprintf( fp, "<LI>%s", wch->name);
     
        if (!IS_NPC(wch)) {  
                  char *modtitle = color_to_html(wch->pcdata->title);
	  fprintf( fp, " %s", modtitle);
                  free_string(modtitle);
        }

	if ( !IS_NPC(wch) 
          && wch->desc != NULL 
          && wch->desc->editor != 0 )
	  fprintf(fp, " (%sOLC%s)", HTML_MAGENTA, HTML_BLACK);

        if ( wch->desc != NULL
          && !wch->desc->ok ) {
          fprintf(fp, " (%slinkdead%s)", HTML_CYAN, HTML_BLACK);
        }

        if (IS_SET(wch->plr, PLR_AFK)) {
          fprintf(fp, " (%sAFK%s)", HTML_RED, HTML_BLACK);
        }

  	fprintf(fp, "%s\n", HTML_BLACK);

      } /*end inner list loop*/

    } /*end countdown loop*/

   /* End, separator, heading... */

    if (found) {
      fprintf(fp, "</MENU>\n");
    } else {
      fprintf(fp, "<P>There are no gods online.</P>\n");
    } 

    fprintf(fp, "<HR>\n"
                "<H3>The Players of %s</H3>\n",
                 mud.name); 

   /* List the mortal players... */

    found = FALSE; 

    for ( counter=MAX_LEVEL;counter>=1;counter--) {
      CHAR_DATA *wch;
      struct who_slot *tmp=table[counter];

      if (tmp == NULL) /* no one at this level */
        continue;

     /* now, follow each chain to end */
      for ( ;tmp != NULL;tmp = tmp->next) {	
	
	wch = tmp->ch;

	if (wch == NULL) {
	  log_string ("Got null table->ch, argh.");
	  continue;
	}

        if (IS_IMMORTAL(wch)) {
          continue;
        }
		
        if (!found) {	
          fprintf(fp, "<MENU>\n");
          found = TRUE;
        }
 
        fprintf( fp, "<LI>%s", wch->name);
     
        if (!IS_NPC(wch)) {  
                  char * modtitle = color_to_html(wch->pcdata->title);
	  fprintf( fp, " %s", modtitle);
                  free_string(modtitle);
        }

	if ( !IS_NPC(wch) 
          && wch->desc != NULL 
          && wch->desc->editor != 0 )
	  fprintf(fp, " (%sOLC%s)", HTML_MAGENTA, HTML_BLACK);

        if ( wch->desc != NULL
          && !wch->desc->ok ) {
          fprintf(fp, " (%slinkdead%s)", HTML_CYAN, HTML_BLACK);
        }

        if (IS_SET(wch->plr, PLR_AFK)) {
          fprintf(fp, " (%sAFK%s)", HTML_CYAN, HTML_BLACK);
        }

  	fprintf(fp, "%s\n", HTML_BLACK);

      } /*end inner list loop*/

    } /*end countdown loop*/

   /* Closing... */

    if (found) {
      fprintf(fp, "</MENU>\n");
    } else {
      fprintf(fp, "<P>There are no players online.</P>\n");
    } 

    fprintf( fp, "<HR>\n"
                 "<H3>Summary</H3>\n" 
                 "<P>GODS found:    %d<P>\n"
                 "<P>Players found: %d<P>\n" 
                 "<P>Max Players on during this run: %s%d%s<P>\n"
                 "<HR>\n"
                 "<FONT SIZE=-1>\n"
                 "Generated by CthulhuMud (Driver %s) at %s</P>\n"
                 "</BODY>\n"
                 "</HTML>\n",
                  immMatch, 
                  mortMatch,
                  HTML_BLUE, mud.player_count, HTML_BLACK,
                  DRIVER_VERSION,
                 (char *) ctime( &current_time ) );

   /* Close the file... */

    fclose(fp);

   /* Shift things around... */

    sprintf(cmdbuf, "mv %s %s\n", ONLINE_FILE_NEW, ONLINE_FILE);

    system(cmdbuf);

    sprintf(cmdbuf, "chmod 664 %s\n", ONLINE_FILE);

    system(cmdbuf);

   /* Hopefully all done... */ 
 
    return;
}


char* color_to_html(char * in) {
char buf[MAX_STRING_LENGTH];
char *out;
char *trans;
char c;
int ip = 0;
int op = 0;
int tp = 0;

    if (in == NULL) {
       out = strdup("without title.");
       return out;
    }

    while (op < MAX_STRING_LENGTH-1 && in[ip] != '\0' ) {
         switch (in[ip]) {
               default:
                  buf[op++] = in[ip];
                  break;

                case '{':
                  c = in[++ip];
                  
                  switch (c) {
                      case 'r':
                      case 'R':
                         trans = strdup(HTML_RED);
                         for (tp = 0; tp < 22; tp++) {
                               buf[op++] = trans[tp];
                         }
                         break;

                      case 'b':
                      case 'B':
                         trans =strdup(HTML_BLUE);
                         for (tp = 0; tp < 22; tp++) {
                               buf[op++] = trans[tp];
                         }
                         break;

                      case 'g':
                      case 'G':
                         trans = strdup(HTML_GREEN);
                         for (tp = 0; tp < 22; tp++) {
                               buf[op++] = trans[tp];
                         }
                         break;

                      case 'c':
                      case 'C':
                         trans = strdup(HTML_CYAN);
                         for (tp = 0; tp < 22; tp++) {
                               buf[op++] = trans[tp];
                         }
                         break;

                      case 'm':
                      case 'M':
                         trans = strdup(HTML_MAGENTA);
                         for (tp = 0; tp < 22; tp++) {
                               buf[op++] = trans[tp];
                         }
                         break;

                      default:
                         trans = strdup(HTML_BLACK);
                         for (tp = 0; tp < 22; tp++) {
                               buf[op++] = trans[tp];
                         }
                         break;
                  }
                  break;
         }
         ip++;
    }
    buf[op++] = '\0';
    out = strdup(buf);
    return out;
}


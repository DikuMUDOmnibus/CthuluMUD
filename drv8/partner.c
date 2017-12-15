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

 
#include <arpa/inet.h>
#include <stdarg.h>
#include "everything.h"
#include "profile.h"
#include "partner.h"
#include "interp.h"
#include "version.h"
#include "race.h"
#include "society.h"
#include "wev.h"
#include "mob.h"
#include "text.h"
#include "board.h"
#include "gsn.h"


void 		partners_update		(void);
void 		init_descriptor 		(DESCRIPTOR_DATA *dnew, int desc);
bool		write_to_descriptor	(int desc, char *txt, int length);
int 		first_free_desc		(void);
void 		save_obj_to_trans	(OBJ_DATA *obj, FILE *fp, int iNest, bool flush);
void 		load_obj_from_trans	(FILE *fp, CHAR_DATA *ch);


PARTNER_TYPE partner_array[MAX_PARTNER];
extern int control;

void load_partners() {
  FILE *fp;
  char *kwd;
  char code;
  bool done;
  bool ok;
  char *name;
  int rn;
  char buf[MAX_STRING_LENGTH];


  for(rn = 0; rn < MAX_PARTNER; rn++) {
    partner_array[rn].loaded = FALSE;
  } 

  rn = 0;
  partner_array[rn].name 		= NULL;
  partner_array[rn].ip	                = NULL;
  partner_array[rn].title	                = NULL;
  partner_array[rn].login	                = NULL;
  partner_array[rn].passwd	                = NULL;
  partner_array[rn].port	                = 0;
  partner_array[rn].fd               	= -1;
  partner_array[rn].last               	= 0;
  partner_array[rn].start               	= 1;
  partner_array[rn].dominance	= 0;
  partner_array[rn].status	                = PARTNER_DOWN;
  partner_array[rn].loaded		= TRUE;

  fp = fopen(PARTNER_FILE, "r");

  if (fp == NULL) {
    log_string("No partner file!");
    exit(1);
    return;
  }

 /* Read through it and see what we've got... */
   done = FALSE;

  while(!done) {
    kwd = fread_word(fp);
    code = kwd[0];
    ok = FALSE;

    switch (code) {

      case '#':
        ok = TRUE;
        fread_to_eol(fp); 
        break; 

      case 'D':
        if (!str_cmp(kwd, "Dominance")) {
          ok = TRUE;
          partner_array[rn].dominance = fread_number(fp);
        }
        break;
        
      case 'E':
        if (!str_cmp(kwd, "End")) {
          ok = TRUE;
          done = TRUE;
        } 
        break; 

      case 'I':
        if (!str_cmp(kwd, "IP")) {
          ok = TRUE;
          partner_array[rn].ip = strdup(fread_word(fp));
        }
        break;

      case 'L':
        if (!str_cmp(kwd, "Login")) {
          ok = TRUE;
          partner_array[rn].login = strdup(fread_word(fp));
        }
        break;

      case 'P':
        if (!str_cmp(kwd, "Port")) {
          ok = TRUE;
          partner_array[rn].port = fread_number(fp);
        }

        if (!str_cmp(kwd, "Passwd")) {
          ok = TRUE;
          partner_array[rn].passwd = strdup(fread_word(fp));
        }

         if (!str_cmp(kwd, "PARTNER")) {
          ok = TRUE;
          name = fread_word(fp); 

          if (name[0] == '\0') {
            bug("Unnammed partner in partner file!", 0);
            exit(1);
          }

          sprintf(buf, "...Loading partner '%s'...", name);
          log_string(buf);

          rn++;

          if (rn >= MAX_PARTNER) { 
            bug("To many partners in partner file!",0);
            exit(1);
          }

         /* Initialize data... */ 

          partner_array[rn].name 		= strdup(name);
          partner_array[rn].ip	                 	= NULL;
          partner_array[rn].title                 	= NULL;
          partner_array[rn].login	                = NULL;
          partner_array[rn].passwd	                = NULL;
          partner_array[rn].port	                = 0;
          partner_array[rn].fd       	                = -1;
          partner_array[rn].last       	                = 0;
          partner_array[rn].start               	= 1;
          partner_array[rn].dominance	                = 0;
          partner_array[rn].status	                = PARTNER_DOWN;
          partner_array[rn].loaded		= TRUE;
        }
        break;

      case 'S':
        if (!str_cmp(kwd, "Start")) {
          ok = TRUE;
          partner_array[rn].start = fread_number(fp);
          if (partner_array[rn].start < 1) partner_array[rn].start = 1;
        }
        break;

      case 'T':
        if (!str_cmp(kwd, "Title")) {
          ok = TRUE;
          partner_array[rn].title = strdup(fread_string(fp));
        }
        break;
    
      default:
        break;
    }

    if (!ok) {
      sprintf(buf, "Unrecognized keyword in cult file: %s", kwd);
      bug(buf, 0); 
      exit(1);
    }
 
  }

  fclose(fp);
  return;  
}

int get_partner_rn(char *name) {
  int rn;

  for(rn = 1; rn < MAX_PARTNER; rn++) {

    if (partner_array[rn].name == NULL ) return PARTNER_UNDEFINED;
    if (!str_cmp(capitalize(name), partner_array[rn].name)) return rn;
  }
  return PARTNER_UNDEFINED;
}


void list_partners(CHAR_DATA *ch) {
  int rn;

  sprintf_to_char(ch, "Partners:\r\n");
  sprintf_to_char(ch, "--------------------\r\n");
  for(rn = 1; rn < MAX_PARTNER; rn++) {
     if (partner_array[rn].name != NULL ) {
         char *part_col;

         if (partner_array[rn].status == PARTNER_CONNECTED) part_col = "{g";
         else part_col = "{r";
         sprintf_to_char(ch, "%s%s (%s: %d){x\r\n", part_col, partner_array[rn].name, partner_array[rn].ip, partner_array[rn].port);
     }
  }
  return;
}


/* Connect to this mud; return fd or -1 if failed to connect */
int     connect_mud (const char *hostname, int port) {
struct sockaddr_in sockad;
int     fd;

    /* Check if inet_addr will accept this as a valid address */
    /* rather than assuming whatever starts with a digit is an ip */
    if ((sockad.sin_addr.s_addr = inet_addr (hostname)) == (unsigned long) -1)    {
        struct hostent *h;


        if (!(h = gethostbyname(hostname))) {
            perror("connect_mud:gethostbyaddr");
            exit (1);
        }
        memcpy ((char *) &sockad.sin_addr, h->h_addr, sizeof (sockad.sin_addr));
    }

    sockad.sin_port = htons (port);
    sockad.sin_family = AF_INET;
    
    if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        perror ("connect_mud:socket");
        return -1;
    }
    
    if (connect (fd, (struct sockaddr *) &sockad, sizeof (sockad))<0) {
        perror ("connect_mud:connect");
        return -1;
    }
    return fd;
}

void disconnect_mud (int fd) {
    close(fd);
}


void write_mud (int fd, const char *fmt, ...) {
    char buf[16384];
    va_list va;
    int res, len, pos;

    va_start(va, fmt);
    vsprintf(buf, fmt, va);
    va_end(va);

    len = strlen(buf);
    pos = 0;

    do {
        res = write(fd, buf+pos, len);
        if (res < 0) {
             bug("Error while writing to MUD: %s", 0);
             return;
        }
        pos += res;
        len -= res;
    }
    while (len > 0);
}


void connect_partners(void) {
int rn;

    for (rn=0; rn < MAX_PARTNER; rn++) connect_single_partner(rn);
    return;
}


void partners_update(void) {
int rn;

    for (rn=0; rn < MAX_PARTNER; rn++) {
       if (partner_array[rn].status == PARTNER_CONNECTED) {
            if (partner_array[rn].last >=5) {
                sprintf_to_partner(rn, "quit\r\n");
                partner_array[rn].status = PARTNER_DOWN;
                partner_array[rn].fd = -1;
            } else {
                sprintf_to_partner(rn, "mudping\r\n");
            }
       }
       partner_array[rn].last++;
    }
    return;
}


void connect_single_partner(int rn) {
char buf[MAX_INPUT_LENGTH];
int fd;

         if (!partner_array[rn].ip
         || partner_array[rn].port < 1024) return;

         fd = connect_mud (partner_array[rn].ip, partner_array[rn].port);
         if (fd < 0) return;

         if (partner_array[rn].status == PARTNER_CONNECTED) return;

         partner_array[rn].fd = fd;
         sprintf(buf, "  %s found.", partner_array[rn].name);
         log_string(buf);
         partner_array[rn].status = PARTNER_CONNECTED;
         write_mud (partner_array[rn].fd, "y\r\n");
         sprintf(buf, "%s\r\n", mud.name);
         write_mud (partner_array[rn].fd, buf);
         sprintf(buf, "%s\r\n", DRIVER_VERSION);
         write_mud (partner_array[rn].fd, buf);
         sprintf(buf, "x\r\n");
         write_mud (partner_array[rn].fd, buf);
         write_mud (partner_array[rn].fd, buf);
         write_mud (partner_array[rn].fd, buf);
         return;
}


int am_i_partner(CHAR_DATA *ch) {
int rn;

    for (rn=0; rn < MAX_PARTNER; rn++) {
          if (!partner_array[rn].name) continue;
          if (!str_cmp(partner_array[rn].name, ch->name)) {
                if (!str_suffix(partner_array[rn].ip, ch->desc->host  ))	return rn;
          }
    }
    return -1;
}


void do_mudreply(CHAR_DATA *ch, char *arguments) {
CHAR_DATA *rc;
char arg1[MAX_INPUT_LENGTH];

    if (!IS_MUD(ch)) {
         send_to_char("You can't use that kind of administrative command!\r\n",ch);
         return;
    }

    arguments = one_argument(arguments, arg1);
    if ((rc = get_char_world(ch, arg1)) == NULL) {
         bug("Reply-to-target is gone...", 0);
         return;
    }

    send_to_char(arguments, rc);
    return;
}


void do_mudping(CHAR_DATA *ch, char *arguments) {
int rn;

    if (!IS_MUD(ch)) {
         send_to_char("You can't use that kind of administrative command!\r\n",ch);
         return;
    }

    rn = am_i_partner(ch);
    partner_array[rn].last = 0;
    return;
}


void do_mudprompt(CHAR_DATA *ch, char *arguments) {
CHAR_DATA *rc;
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];

    if (!IS_MUD(ch)) {
         send_to_char("You can't use that kind of administrative command!\r\n",ch);
         return;
    }

    arguments = one_argument(arguments, arg1);
    arguments = one_argument(arguments, arg2);

    if ((rc = get_char_world(ch, arg1)) == NULL) {
         bug("Reply-to-target is gone...", 0);
         return;
    }

    if (!str_cmp(arg2, "off")) {
         REMOVE_BIT(rc->comm, COMM_PROMPT);
    } else { 
         SET_BIT(rc->comm, COMM_PROMPT);
         send_to_char("\r\n",rc);
    }
    return;
}


void do_mudbug(CHAR_DATA *ch, char *arguments) {
char buf[MAX_STRING_LENGTH];

    if (!IS_MUD(ch)) {
         send_to_char("You can't use that kind of administrative command!\r\n",ch);
         return;
    }

    sprintf(buf, "%s: %s", ch->name, arguments);
    bug(buf, 0);
    return;
}


void do_mudversion(CHAR_DATA *ch, char *arguments) {
char arg1[MAX_INPUT_LENGTH];
int rn;

    if (!IS_MUD(ch)) {
         send_to_char("You can't use that kind of administrative command!\r\n",ch);
         return;
    }

    rn = am_i_partner(ch);
    if (rn < 0) return;
    if (partner_array[rn].fd < 0) return;

    arguments = one_argument(arguments, arg1);

    sprintf_to_partner(rn, "mudreply %s {gCthulhumud Driver Version:{x %s\r\n", arg1, DRIVER_VERSION);
    return;
}


void do_mudwhois(CHAR_DATA *ch, char *arguments) {
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
char buf[MAX_STRING_LENGTH];
char buf2[MAX_STRING_LENGTH];
char outbuf[MAX_STRING_LENGTH];
DESCRIPTOR_DATA *d;
LPROF_DATA *prof;
CHAR_DATA *wch;
bool found = FALSE;
int rn;

    if (!IS_MUD(ch)) {
         send_to_char("You can't use that kind of administrative command!\r\n",ch);
         return;
    }

    rn = am_i_partner(ch);
    if (rn < 0) return;
    if (partner_array[rn].fd < 0) return;

    arguments = one_argument(arguments, arg1);
    arguments = one_argument(arguments, arg2);


    if (arg2[0] == '\0')    {
	send_to_char("You must provide a name.\r\n",ch);
	return;
    }

    outbuf[0] = '\0';

    sprintf_to_partner(rn, "mudprompt %s %s\r\n", arg1, "off");

    for (d = descriptor_list; d != NULL; d = d->next)    {

 	if (d->connected != CON_PLAYING) continue;
	wch = ( d->original != NULL ) ? d->original : d->character;

	if (!str_prefix(arg2, wch->name)) {
                    found = TRUE;
                    if ( !IS_NPC(wch)
                    &&  IS_IMMORTAL(wch)
                    &&  (str_cmp(wch->pcdata->immtitle, "-GOD-") != 0)) {
                        sprintf(buf2, "%s", wch->pcdata->immtitle);
                    } else {
                        char_div(wch, buf2);
                    }   

	    sprintf(buf, "{w[{Y%2d {W%s{w]{w %s%s\r\n",wch->level,buf2,wch->name,IS_NPC(wch) ? "" : wch->pcdata->title);
                    sprintf_to_partner(rn, "mudreply %s %s", arg1, buf);

                    prof=wch->profs;
                    sprintf(buf, "  the %s %s %s.\r\n",wch->sex == 0 ? "sexless" : wch->sex == 1 ? "male" : "female",race_array[wch->race].name,prof->profession->name);
                    sprintf_to_partner(rn, "mudreply %s %s", arg1, buf);

                    if( wch->pcdata->image != NULL
                    && wch->pcdata->image[0] != '\0' ) {
                           if ( IS_SET(ch->plr, PLR_IMP_HTML) ) {
                                  sprintf(buf,"<IMG SRC=\"%s\" ALT=\"%s's image\">\r\n",wch->pcdata->image,wch->name);
                           } else {
                                  sprintf(buf, "{W%s's image is..: {c%s\r\n",wch->name,wch->pcdata->image);
                           } 
                           sprintf_to_partner(rn, "mudreply %s %s", arg1, buf);
                    }

                     if( wch->pcdata->url != NULL
                     && wch->pcdata->url[0] != '\0') {
                           if ( IS_SET(ch->plr, PLR_IMP_HTML) ) {
                                    sprintf(buf, "<A HREF=\"%s\">%s's homepage</A>\r\n",wch->pcdata->url,wch->name);
                           } else {
                                    sprintf(buf, "{W%s's hompage is: {c%s\r\n", wch->name,  wch->pcdata->url);
                           } 
                           sprintf_to_partner(rn, "mudreply %s %s", arg1, buf);
                     }

                     if( wch->pcdata->email != NULL
                     && wch->pcdata->email[0] != '\0') {
                            if ( IS_SET(ch->plr, PLR_IMP_HTML) ) {
                                     sprintf(buf,"{W%s's email is: <A HREF=\"mailto:%s\">{C%s</A>\r\n",wch->name, wch->pcdata->email,wch->pcdata->email);
                            } else {
                                     sprintf(buf, "{W%s's email is..: {c%s\r\n", wch->name, wch->pcdata->email);
                            } 
                            sprintf_to_partner(rn, "mudreply %s %s", arg1, buf);
                     }

                      if ( wch->bio[0] != '\0' ) {
                            sprintf_to_partner(rn, "mudreply %s %s", arg1, "Bio:\r\n");
                            sprintf_to_partner(rn, "mudreply %s %s", arg1, wch->bio);
                      }

	}
    }

    if (!found) {
        do_pload(ch, arg2);
         if ((wch = get_char_world( ch, arg2 ) ) == NULL ) {
              sprintf_to_partner(rn, "mudprompt %s %s\r\n", arg1, "on");
              return;
          }
          if (IS_NPC(wch)) {
              sprintf_to_partner(rn, "mudprompt %s %s\r\n", arg1, "on");
              return;
          }
          if ( !IS_NPC(wch)
          &&  IS_IMMORTAL(wch)
          &&  (str_cmp(wch->pcdata->immtitle, "-GOD-") != 0)) {
                        sprintf(buf2, "%s", wch->pcdata->immtitle);
           } else {
                        char_div(wch, buf2);
           }   

            sprintf(buf, "{w[{Y%2d {W%s{w]{w %s%s\r\n",wch->level,buf2,wch->name,IS_NPC(wch) ? "" : wch->pcdata->title);
            sprintf_to_partner(rn, "mudreply %s %s", arg1, buf);
  
            prof=wch->profs;
            sprintf(buf, "  the %s %s %s.\r\n",wch->sex == 0 ? "sexless" : wch->sex == 1 ? "male" : "female",race_array[wch->race].name,prof->profession->name);
            sprintf_to_partner(rn, "mudreply %s %s", arg1, buf);

            if( wch->pcdata->image != NULL
            && wch->pcdata->image[0] != '\0' ) {
                  if ( IS_SET(ch->plr, PLR_IMP_HTML) ) {
                                  sprintf(buf,"<IMG SRC=\"%s\" ALT=\"%s's image\">\r\n",wch->pcdata->image,wch->name);
                  } else {
                                  sprintf(buf, "{W%s's image is..: {c%s\r\n",wch->name,wch->pcdata->image);
                  } 
                  sprintf_to_partner(rn, "mudreply %s %s", arg1, buf);
            }

            if( wch->pcdata->url != NULL
            && wch->pcdata->url[0] != '\0') {
                           if ( IS_SET(ch->plr, PLR_IMP_HTML) ) {
                                    sprintf(buf, "<A HREF=\"%s\">%s's homepage</A>\r\n",wch->pcdata->url,wch->name);
                           } else {
                                    sprintf(buf, "{W%s's hompage is: {c%s\r\n", wch->name,  wch->pcdata->url);
                           } 
                           sprintf_to_partner(rn, "mudreply %s %s", arg1, buf);
            }

            if( wch->pcdata->email != NULL
            && wch->pcdata->email[0] != '\0') {
                            if ( IS_SET(ch->plr, PLR_IMP_HTML) ) {
                                     sprintf(buf,"{W%s's email is: <A HREF=\"mailto:%s\">{C%s</A>\r\n",wch->name, wch->pcdata->email,wch->pcdata->email);
                            } else {
                                     sprintf(buf, "{W%s's email is..: {c%s\r\n", wch->name, wch->pcdata->email);
                            } 
                            sprintf_to_partner(rn, "mudreply %s %s", arg1, buf);
            }

            if ( wch->bio[0] != '\0' ) {
                            sprintf_to_partner(rn, "mudreply %s %s", arg1, "Bio:\r\n");
                            sprintf_to_partner(rn, "mudreply %s %s", arg1, wch->bio);
            }
            do_punload(ch, arg2);
    }
    sprintf_to_partner(rn, "mudprompt %s %s\r\n", arg1, "on");
    return;
}


void do_mudwho(CHAR_DATA *ch, char *argument ) {
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char fname[MAX_STRING_LENGTH]; 
    char clanbuf[MAX_STRING_LENGTH];
    char form[MAX_STRING_LENGTH];
    char output[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    int nNumber;
    int immMatch=0;
    int mortMatch=0;
    struct who_slot *table[MAX_LEVEL + 1]; /*want 1 to 100, not 0 to 99*/ 
    int counter, rn;
    MEMBERSHIP *memb;
    SOCIETY *society;

    for (counter=1;counter <= MAX_LEVEL;counter++) {
        table[counter] = NULL;
    }

    if (!IS_MUD(ch)) {
         send_to_char("You can't use that kind of administrative command!\r\n",ch);
         return;
    }

    rn = am_i_partner(ch);
    if (rn < 0) return;
    if (partner_array[rn].fd < 0) return;


   /* Parse arguments... */

    nNumber = 0;
    argument = one_argument( argument, arg1);

    sprintf_to_partner(rn, "mudprompt %s %s\r\n", arg1, "off");

    for ( d = descriptor_list; d != NULL; d = d->next ) {
      CHAR_DATA *wch;
	
      wch   = ( d->original != NULL ) ? d->original : d->character;
  
      if (  d->connected != CON_PLAYING) continue;

      if (IS_IMMORTAL(wch))  immMatch++;
      else mortMatch++;
	
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

   /* Now show matching chars... */

    buf[0] = '\0';
    output[0] = '\0';

    sprintf(buf, "{C--- The Gods of %s ---{x\r\n", mud.name);
    sprintf_to_partner(rn, "mudreply %s %s", arg1, buf);
   
    for ( counter=MAX_LEVEL;counter>=1;counter--) {
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

        if (IS_MORTAL(wch)
        || IS_MUD(wch)) continue;

	strcpy (form, "%20s  %s%s%s%s%s%s %s");

       /* Get clan details... */
	
        clanbuf[0] = '\0';

        memb = wch->memberships;
        while (memb != NULL) {

          if (memb->level >= 0) {
              society = memb->society;
    
              if ( society != NULL 
              && society->secret 
              && !IS_IMMORTAL(ch)
              && !is_member(ch, society)) {
                     society = NULL;
              } 
    
              if ( society != NULL
              && society->type == SOC_TYPE_CLAN) {
                    if (society->secret) {
	       sprintf(clanbuf, "[{r%s{x]", society->name);	
                    } else {
	       sprintf(clanbuf, "[{g%s{x]", society->name);	
                    }
                    memb = NULL;
              } 
          }
          if (memb) memb = memb->next;
        }   

        if ( !IS_NPC(wch)
        &&  IS_IMMORTAL(wch)
        &&  (str_cmp(wch->pcdata->immtitle, "-GOD-") != 0)) {
          sprintf(buf2, "%s", wch->pcdata->immtitle);
        } else {
          char_div(wch, buf2);
        }   

        sprintf(fname, "{x[{Y%3d {W%s{x]                   ",  wch->level, buf2);
        fname[25] = '\0';

        if (wch != ch) {
              sprintf( buf, form, fname,
	      IS_SET(wch->plr, PLR_AFK) ? "{r(AFK){x " : "",
	      IS_SET(wch->plr, PLR_ANNOYING) ? "{r({mANNOYING{r){x " : "",
                      "", "",
	      wch->name,  IS_NPC(wch) ? "" : wch->pcdata->title, clanbuf);
         } else {
              sprintf( buf, form, fname,
	      IS_SET(wch->plr, PLR_AFK) ? "{r(AFK){x " : "",
                      "", "", "",
	      wch->name,  IS_NPC(wch) ? "" : wch->pcdata->title, clanbuf);
         }

  	if (IS_SET(wch->plr, PLR_WIZINVIS)) strcat (buf, " {r({cWizi{r){x");
	if (IS_SET(wch->plr, PLR_CLOAK)) strcat (buf, " {r({cCloak{r){x");
	if (!IS_NPC(wch) && (wch->desc!=NULL) && wch->desc->editor) strcat (buf, " {M(OLC){x ");

        if ( wch->desc != NULL
        && !wch->desc->ok ) {
          strcat(buf, " {c(linkdead){x");
        }

  	strcat (buf, "\r\n");
                sprintf_to_partner(rn, "mudreply %s %s", arg1, buf);

      }
    }

  /* Now display the mortals... */
 
    sprintf(buf, "{G--- The Players of %s ---{x\r\n", mud.name);
    sprintf_to_partner(rn, "mudreply %s %s", arg1, buf);
   
    for ( counter=MAX_LEVEL;counter>=1;counter--)  {
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

        if (IS_IMMORTAL(wch)) continue;
        strcpy (form, "%20s  %s%s%s%s%s%s %s");
		 	
       /* Get clan details... */
	
        clanbuf[0] = '\0';
        memb = wch->memberships;

        while (memb != NULL) {

          if (memb->level >= 0) {
              society = memb->society;
    
              if ( society != NULL 
              && society->secret 
              && !IS_IMMORTAL(ch)
              && !is_member(ch, society)) {
                     society = NULL;
              } 
    
              if ( society != NULL
              && society->type == SOC_TYPE_CLAN) {
                    if (society->secret) {
	       sprintf(clanbuf, "[{r%s{x]", society->name);	
                    } else {
	       sprintf(clanbuf, "[{g%s{x]", society->name);	
                    }
                    memb = NULL;
              } 
          }
          if (memb) memb = memb->next;
        }   

        if ( !IS_NPC(wch)
          && get_divinity(wch) > DIV_LEV_HERO) {
          sprintf(buf2, "%s", wch->pcdata->immtitle);
        } else {
           if (wch->level>100) {
             sprintf(buf2, "%s", wch->pcdata->immtitle);
           } else {
             char_div(wch, buf2);
           }
        }   

        sprintf(fname, "{x[{Y%3d {W%s{x]                   ", wch->level, buf2);
        fname[25] = '\0';

        if (wch !=ch) {
                      sprintf( buf, form, fname,
	      IS_SET(wch->plr, PLR_AFK) ? "{r(AFK){x " : "",
	      IS_SET(wch->plr, PLR_ANNOYING) ? "{r({mANNOYING{r){x " : "",
                      "", "",
	      wch->name, IS_NPC(wch) ? "" : wch->pcdata->title, clanbuf);
        } else {
                      sprintf( buf, form, fname,
	      IS_SET(wch->plr, PLR_AFK) ? "{r(AFK){x " : "",
                      "", "", "",
	      wch->name, IS_NPC(wch) ? "" : wch->pcdata->title, clanbuf);
         }

  	if (IS_SET(wch->plr, PLR_WIZINVIS)) strcat (buf, " {r({cWizi{r){x");
	if (IS_SET(wch->plr, PLR_CLOAK)) strcat (buf, " {r({cCloak{r){x");
	if (!IS_NPC(wch) && (wch->desc!=NULL) && wch->desc->editor) strcat (buf, " {M(OLC){x ");

        if ( wch->desc != NULL
          && !wch->desc->ok ) {
          strcat(buf, " {c(linkdead){x");
        }

  	strcat (buf, "\r\n");
                sprintf_to_partner(rn, "mudreply %s %s", arg1, buf);

      }

    }

    sprintf( buf, "{yGODS found: {C%d{x  {yPlayers found:   {G%d{x\r\n", immMatch, mortMatch );
    sprintf_to_partner(rn, "mudreply %s %s", arg1, buf);

    sprintf_to_partner(rn, "mudprompt %s %s\r\n", arg1, "on");

    return;
}


void do_mudgossip( CHAR_DATA *ch, char *argument ) {
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;    
    char msg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    int rn;

    if (!IS_MUD(ch)) {
         send_to_char("You can't use that kind of administrative command!\r\n",ch);
         return;
    }

    rn = am_i_partner(ch);

    argument = one_argument(argument, arg1);

    if ( !strip_html(argument, msg, TRUE, MAX_STRING_LENGTH, FONT_MAGENTA)) {
      send_to_char("Those words many not be uttered!\r\n", ch);
      return;
    }

    sprintf(buf, "{MFrom the realms of %s, %s gossips '@t0{M'{x\r\n", partner_array[rn].title, capitalize(arg1));

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, gsn_english, msg);
    wev = get_wev(WEV_OOCC, WEV_OOCC_GOSSIP, mcc,
                  NULL,
                  NULL,
                  buf);

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    world_issue_wev(wev, "OOC");
    free_wev(wev);
    return;
}


void do_mudhero( CHAR_DATA *ch, char *argument ) {
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;    
    char msg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    int rn;

    if (!IS_MUD(ch)) {
         send_to_char("You can't use that kind of administrative command!\r\n",ch);
         return;
    }

    rn = am_i_partner(ch);

    argument = one_argument(argument, arg1);

    if ( !strip_html(argument, msg, TRUE, MAX_STRING_LENGTH, FONT_MAGENTA)) {
      send_to_char("Those words many not be uttered!\r\n", ch);
      return;
    }

    sprintf(buf, "{GFrom the realms of %s, %s tells '@t0'{x\r\n", partner_array[rn].title, capitalize(arg1));

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, gsn_english, msg);
    wev = get_wev(WEV_OOCC, WEV_OOCC_HERO, mcc,
                  NULL,
                  NULL,
                  buf);

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    world_issue_wev(wev, "OOC");
    free_wev(wev);
    return;
}


void do_mudimmtalk( CHAR_DATA *ch, char *argument ) {
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;    
    char msg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];


    if (!IS_MUD(ch)) {
         send_to_char("You can't use that kind of administrative command!\r\n",ch);
         return;
    }

    argument = one_argument(argument, arg1);

    if ( !strip_html(argument, msg, TRUE, MAX_STRING_LENGTH, FONT_CYAN)) {
      send_to_char("Those words many not be uttered!\r\n", ch);
      return;
    }

    sprintf(buf, "{C[{y%s{C]: @t0{x\r\n", capitalize(arg1));

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, gsn_english, msg);
    wev = get_wev(WEV_OOCC, WEV_OOCC_IMMTALK, mcc,
                  NULL,
                  NULL,
                  buf);

   /* Challange... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

   /* Ok, output the wev... */

    world_issue_wev(wev, "OOC");
    free_wev(wev);
    return;
}


char* prepare_for_transfer(char * in) {
char buf[MAX_STRING_LENGTH];
char *out;
int ip = 0;
int op = 0;

    if (in == NULL) {
       out = strdup("*nothing*");
       return out;
    }

    while (op < MAX_STRING_LENGTH-1 && in[ip] != '\0' ) {
         switch (in[ip]) {
               default:
                  buf[op++] = in[ip];
                  break;

                case '%':
                  break;
         }
         ip++;
    }
    buf[op++] = '\0';
    out = strdup(buf);
    return out;
}


void do_note_prepare( CHAR_DATA *ch, char *argument ) {
char arg1[MAX_INPUT_LENGTH];
char *strtime;
int rn;

    if (!IS_MUD(ch)) {
         send_to_char("You can't use that kind of administrative command!\r\n",ch);
         return;
    }
    rn = am_i_partner(ch);
    do_board(ch, INTER_BOARD);

    argument = one_argument(argument, arg1);

    if (ch->pcdata->in_progress) {
         sprintf_to_partner(rn, "mudreply %s This servisce is not availiable at the moment.\r\n", arg1);
         sprintf_to_partner(rn, "mudreply %s Try again later.\r\n", arg1);
         return;
    }

    ch->pcdata->in_progress = new_note();
    ch->pcdata->in_progress->sender = str_dup(arg1);

    strtime = ctime (&current_time);
    strtime[strlen(strtime)-1] = '\0';
    ch->pcdata->in_progress->date = str_dup(strtime);

    free_string(ch->pcdata->in_progress->text);
    ch->pcdata->in_progress->text = strdup(" ");
    ch->pcdata->in_progress->to_list = str_dup("all");

    ch->pcdata->in_progress->expire = current_time + (ch->pcdata->board->purge_days *24L*3600L);
    return;
}


void do_note_subject( CHAR_DATA *ch, char *argument ) {
char arg1[MAX_INPUT_LENGTH];
int rn;

    if (!IS_MUD(ch)) {
         send_to_char("You can't use that kind of administrative command!\r\n",ch);
         return;
    }
    rn = am_i_partner(ch);

    argument = one_argument(argument, arg1);

    if (!ch->pcdata->in_progress) return;

    free_string(ch->pcdata->in_progress->subject);
    ch->pcdata->in_progress->subject = strdup(arg1);
    return;
}


void do_note_line( CHAR_DATA *ch, char *argument ) {
char arg1[MAX_INPUT_LENGTH];
int rn;

    if (!IS_MUD(ch)) {
         send_to_char("You can't use that kind of administrative command!\r\n",ch);
         return;
    }
    rn = am_i_partner(ch);

    argument = one_argument(argument, arg1);

    if (!ch->pcdata->in_progress) {
         sprintf_to_partner(rn, "mudreply %s The partner is not listening - please abandon process.\r\n", arg1);
         return;
    }

    strcat(ch->pcdata->in_progress->text, argument);
    return;
}


void do_note_stop( CHAR_DATA *ch, char *argument ) {

    if (!IS_MUD(ch)) {
         send_to_char("You can't use that kind of administrative command!\r\n",ch);
         return;
    }

    if (!ch->pcdata->in_progress) return;

    finish_note (ch->pcdata->board, ch->pcdata->in_progress);
    ch->desc->connected = CON_PLAYING;
    ch->pcdata->in_progress = NULL;
    return;
}


void do_mudplayerexists(CHAR_DATA *ch, char* argument) {
char arg1[MAX_INPUT_LENGTH];
bool fOld;
DESCRIPTOR_DATA d;
int rn;

     if (!IS_MUD(ch)) {
         send_to_char("You can't use that kind of administrative command!\r\n",ch);
         return;
     }

     rn = am_i_partner(ch);

     one_argument(argument, arg1);
     fOld = load_char_obj(&d, arg1);

     if (fOld) sprintf_to_partner(rn, "mudreturn 1\r\n");
     else sprintf_to_partner(rn, "mudreturn 0\r\n");

     return;
}


void do_mudreturn(CHAR_DATA *ch, char* argument) {
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
int rn;

     if (!IS_MUD(ch)) {
         send_to_char("You can't use that kind of administrative command!\r\n",ch);
         return;
     }

     argument = one_argument(argument, arg1);
     argument = one_argument(argument, arg2);

     rn = am_i_partner(ch);

     if (arg2[0] !='\0') {
        CHAR_DATA*victim;
        if ((victim = get_char_world_player(ch, arg2)) != NULL) {
            if (is_number(arg1)) {
                 victim->pcdata->answer = atoi(arg1);
            } else {
                 if (!str_cmp(arg1, "s")) victim->pcdata->answer = rn;
                 if (!str_cmp(arg1, "s+")) victim->pcdata->answer = rn + 10;
            }
            return;
        }
     }

     if (is_number(arg1)) ch->pcdata->answer = atoi(arg1);
     return;
}


void do_muddelete(CHAR_DATA *ch, char* argument) {
char arg1[MAX_INPUT_LENGTH];
char buf[MAX_STRING_LENGTH];
CHAR_DATA *vic;

     if (!IS_MUD(ch)) {
         send_to_char("You can't use that kind of administrative command!\r\n",ch);
         return;
     }

     argument = one_argument(argument, arg1);

     if ((vic = get_char_world_player(ch, arg1)) != NULL) {
         bug("Player still connected!",0);
         if (ch->desc) close_socket(ch->desc, FALSE);
     }

     sprintf(buf, "%s%s.gz", PLAYER_DIR, capitalize(arg1));
     unlink(buf);
     return;
}


void do_muddescriptor(CHAR_DATA *ch, char* argument) {
DESCRIPTOR_DATA *d;
char name [100];
char host[MAX_STRING_LENGTH];
int desc;
bool fOld;
	
     if (!IS_MUD(ch)) {
         send_to_char("You can't use that kind of administrative command!\r\n",ch);
         return;
     }

     argument = one_argument(argument, name);
     argument = one_argument(argument, host);

     desc = first_free_desc();

     if (desc == -1) return;

     d = (DESCRIPTOR_DATA*) alloc_perm (sizeof(DESCRIPTOR_DATA));
     init_descriptor (d,desc);
     d->host = str_dup (host);
     d->next = descriptor_list;
     descriptor_list = d;
     d->connected = CON_COPYOVER_RECOVER;
		
      fOld = load_char_obj (d, name);
      if (!fOld) {
          close_socket (d, FALSE);			
      } else {
          if (!d->character->in_room) d->character->in_room = get_room_index (mud.home);
          d->character->next = char_list;
          char_list = d->character;
          char_to_room (d->character, d->character->in_room);
          d->connected = CON_TRANSFER;
          d->ok = TRUE;
    }
    load_trans(d->character);
    save_char_obj(d->character);  
    return;
}


void do_mudhome(CHAR_DATA *ch, char* argument) {
char arg1[MAX_INPUT_LENGTH];
CHAR_DATA *vict;
ROOM_INDEX_DATA *room;
	
     if (!IS_MUD(ch)) {
         send_to_char("You can't use that kind of administrative command!\r\n",ch);
         return;
     }

     argument = one_argument(argument, arg1);

     if ((vict = get_char_world_player(ch, arg1)) == NULL) return;
     if ((room = get_room_index(mud.home)) == NULL) return;
    
     if (vict->in_room == room) return;
     char_from_room(vict);
     char_to_room(vict, room);
     return;
}     
     

CHAR_DATA *get_gate_partner(ROOM_INDEX_DATA *room) {
char arg[MAX_INPUT_LENGTH];
CHAR_DATA *partner;
EXTRA_DESCR_DATA *ed;
char *data = NULL;
 
         if (!room) return NULL;

         ed = room->extra_descr;
         while (ed) {
               if (is_name("partner_data", ed->keyword)) {
                   data = str_dup(ed->description);
                   break;
               }
               ed = ed->next;
         }

         if (!data) return NULL;
         one_argument(data, arg);
         if ((partner = get_char_world_player(NULL, arg)) == NULL) return NULL;
         if (!IS_MUD(partner)) return NULL;

         return partner;
}


VNUM get_gate_dest(ROOM_INDEX_DATA *room) {
char arg[MAX_INPUT_LENGTH];
EXTRA_DESCR_DATA *ed;
char *data = NULL;
VNUM value;
 
         if (!room) return 0;

         ed = room->extra_descr;
         while (ed) {
               if (is_name("partner_data", ed->keyword)) {
                   data = str_dup(ed->description);
                   break;
               }
               ed = ed->next;
         }

         data = one_argument(data, arg);
         one_argument(data, arg);
         value = atol(arg);

         return value;
}


void do_mudtransfer(CHAR_DATA *ch, char* argument) {
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
CHAR_DATA *vict;
ROOM_INDEX_DATA *room;
VNUM rnum;
	
     if (!IS_MUD(ch)) {
         send_to_char("You can't use that kind of administrative command!\r\n",ch);
         return;
     }

     argument = one_argument(argument, arg1);
     argument = one_argument(argument, arg2);
     rnum = atol(arg2);

     if ((vict = get_char_world_player(ch, arg1)) == NULL) return;
     if ((room = get_room_index(rnum)) == NULL) {
          if ((room = get_room_index(mud.home)) == NULL) return;
     }

     if (vict->in_room == room) return;
     char_from_room(vict);
     char_to_room(vict, room);
     return;
}


void transmove_player(CHAR_DATA *ch, CHAR_DATA *partner, VNUM dest) {
char buf[MAX_INPUT_LENGTH];
char buf2[MAX_INPUT_LENGTH];
int rn = am_i_partner(partner);

          signal( SIGCHLD, SIG_IGN );
          if (fork() != 0)  return;
          sprintf(buf, "%s%s.gz", PLAYER_DIR, capitalize(ch->name));
          sprintf(buf2, "mv %s %s.gz", buf, capitalize(ch->name));
          system(buf2);
          sprintf(buf2, "curl -T %s.gz -u %s:%s ftp://%s/mud/player/", capitalize(ch->name), partner_array[rn].login, partner_array[rn].passwd, partner_array[rn].ip);
          system(buf2);
          sprintf(buf2, "%s.gz", capitalize(ch->name));
          unlink(buf2);
          send_to_char("Transfer completed.\r\n", ch);
          if (IS_SET(ch->plr, PLR_IMP_HTML)) {
                sprintf_to_char(ch, "<DISCONNECT><CNTN NAME=\"%s\" HOST=\"%s\" PORT=\"%d\"><BUTTONNAME><CT><RECONNECT>\r\n", partner_array[rn].name, partner_array[rn].ip, partner_array[rn].port);
          } else {
                sprintf_to_char(ch, "Character transfered to: %s %d\r\n",partner_array[rn].ip, partner_array[rn].port);
                send_to_char("Please connect there.\r\n", ch);
          }
          sprintf_to_partner(rn, "muddescriptor %s %s\r\n", ch->name, ch->desc->host);
          if (dest > 0) sprintf_to_partner(rn, "mudtransfer %s %ld\r\n", ch->name, dest);
          else sprintf_to_partner(rn, "mudhome %s\r\n", ch->name);
          close_socket (ch->desc, FALSE);
          exit(0);
}


int first_free_desc(void) {
DESCRIPTOR_DATA *d;
int count = 1;
bool ok = FALSE;

     while (!ok) {
          ok = TRUE;
          for (d = descriptor_list; d; d = d->next)  {
                if (d->descriptor == count) ok = FALSE;
          }
          count++;
      }
  
      return count;
}


void set_index(CHAR_DATA *ch, OBJ_DATA *obj) {

    if (obj->next_content != NULL ) set_index(ch, obj->next_content);
    if (obj->orig_index == 0) obj->orig_index = obj->pIndexData->vnum;
    if ( obj->contains != NULL ) set_index(ch, obj->contains);
    return;
}


bool reindex_obj(OBJ_DATA *obj) {
OBJ_INDEX_DATA *pObj;
VNUM vn;
VNUM opt_vn = 0;
int opt_dif = 999;

    if (obj->orig_index != 0) {
         pObj = get_obj_index(obj->orig_index);
         if (pObj) {
             if (!str_cmp(pObj->name, obj->name)
             && pObj->item_type == obj->item_type) {
                obj->orig_index = 0;
                obj->pIndexData = pObj;
                return TRUE;
             }
         }

         for (vn = 65; vn < 64000; vn++) {
               pObj = get_obj_index(vn);
               if (pObj) {
                      if (pObj->item_type == obj->item_type
                      && !IS_SET(pObj->extra_flags, ITEM_ARTIFACT)
                      && pObj->wear_flags == obj->wear_flags) {
                            if (abs(pObj->level - obj->level) < opt_dif) {
                                opt_dif =  abs(pObj->level - obj->level);
                                opt_vn = vn;
                            }
                      }
               }
         }

         if (opt_vn == 0) return FALSE;

         pObj = get_obj_index(opt_vn);
         obj->pIndexData = pObj;
         return TRUE;
    }
    return FALSE;
}


void lock_away_incompatible(CHAR_DATA *ch, OBJ_DATA *obj) {
FILE *fp;
char buf[MAX_INPUT_LENGTH];
char strsave[MAX_INPUT_LENGTH];

    sprintf( strsave, "%s%s.trans", PLAYER_DIR, capitalize(ch->name));
    if (( fp = fopen(strsave, "w")) == NULL ) {
         bug( "Save_locker: fopen", 0 );
         perror( strsave );
    } else {
         save_obj_to_trans(obj, fp, 0, TRUE);
         fprintf( fp, "#END\n" );
         fclose(fp);
    }

#if defined(COMPRESS_PFILES)
      sprintf(buf,"gzip -fq %s",strsave);
      system(buf);
#endif

      return;
}


void load_trans(CHAR_DATA *ch) {
char strsave[MAX_INPUT_LENGTH];
char buf[MAX_INPUT_LENGTH];
FILE *fp;
bool found;

#if defined(COMPRESS_PFILES)
    sprintf(strsave, "%s%s.trans%s", PLAYER_DIR, capitalize(ch->name),".gz");
    if (( fp = fopen( strsave, "r" ) ) != NULL ) {
    	fclose(fp);
	sprintf(buf,"gzip -dfq %s",strsave);
	system(buf);
    }
#endif

    sprintf(strsave, "%s%s.trans", PLAYER_DIR, capitalize(ch->name));
    fp = fopen( strsave, "r" );
    if ( fp != NULL ) {
	found = TRUE;

                for ( ; ; ) {
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )  {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )  {
		bug("Load_char_obj: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );

	    if ( !str_cmp(word, "O")) {
                        load_obj_from_trans(fp, ch);
                    } else {
	        if ( !str_cmp(word, "END")) {
                            break;
                        } else { 
	            bug( "Load_Locker: bad section.", 0 );
	            break;
                        }
	    }
	}

	fclose( fp );
    }

    unlink(strsave);
    return;
}


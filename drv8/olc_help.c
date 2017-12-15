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

/***************************************************************************
 *  File: olc_help.c                                                       *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This work is a derivative of Talen's post to the Merc Mailing List.    *
 *  It has been modified by Jason Dinkel to work with the new OLC.         *
 *                                                                         *
 ***************************************************************************/

#include "everything.h"
#include "olc.h"
#include "help.h"
#include "econd.h"
#include "mob.h"
#include "profile.h"


#define HEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define EDIT_HELP(Ch, Help)	( Help = (HELP_DATA *)Ch->desc->pEdit )

/*
 * Help Editor Prototypes
 */
DECLARE_OLC_FUN( hedit_create 		);
DECLARE_OLC_FUN( hedit_delete 		);
DECLARE_OLC_FUN( hedit_desc 		);
DECLARE_OLC_FUN( hedit_level 		);
DECLARE_OLC_FUN( hedit_keywords 	);
DECLARE_OLC_FUN( hedit_show 		);
DECLARE_OLC_FUN( hedit_save		);

void 	read_help_directory	(CHAR_DATA *ch, char * argument);
int 	file_select		(const struct direct   *entry);
void 	parse_html		(CHAR_DATA *ch, FILE *fp);
bool 	read_help_entry		(CHAR_DATA *ch, char *argument);

HELP_INDEX		*hindex_free = NULL;
extern HELP_INDEX	*hindex_last;
extern HELP_INDEX	*hindex_first;

const struct olc_cmd_type hedit_table[] =
{
/*  {   command		function	}, */

    {   "commands",	show_commands	},
    {   "create",		hedit_create	},
    {   "delete",		hedit_delete	},
    {   "desc",		hedit_desc	},
    {   "level",		hedit_level	},
    {   "keywords",		hedit_keywords	},
    {   "show",		hedit_show	},
    {   "save", 		hedit_save	},
    {   "?",		show_help	},

    {	"",		0,		}
};


HELP_INDEX *new_hindex(void) {
    HELP_INDEX *NewHelp;

    if( hindex_free == NULL )   {
        NewHelp = (HELP_INDEX *) alloc_perm( sizeof(*NewHelp) );
        top_hindex++;

    } else {
        NewHelp         = hindex_free;
        hindex_free       = hindex_free->next;
    }

    NewHelp->next       	= NULL;
    NewHelp->title       	= NULL;
    NewHelp->keywords       = NULL;
    NewHelp->file       	= NULL;
    NewHelp->type        	= 0;
    NewHelp->ec       	= NULL;

    return NewHelp;
}


void free_hindex( HELP_INDEX *pHelp ) {
    if (pHelp->title != NULL ) free_string( pHelp->title);
    if (pHelp->keywords != NULL ) free_string( pHelp->keywords );
    if (pHelp->file != NULL ) free_string( pHelp->file );
    if (pHelp->ec != NULL ) free_ec( pHelp->ec );
    pHelp->type = 0;
    pHelp->next = hindex_free;
    hindex_free   = pHelp;
    return;
}


void load_hindex() {
int i, count;
char buf[MAX_STRING_LENGTH];
char buf2[MAX_STRING_LENGTH];
char *token;
struct direct **files;

     sprintf(buf, "%s/basic/", HELP_PATH);
     count = scandir(buf, &files, file_select, alphasort);

     for (i=1; i < count + 1;++i){
        token =strdup(files[i-1]->d_name);
        if (!strcmp(token, "WS_FTP.log")) continue;
        sprintf(buf2, "%s%s", buf, token);
        free_string(token);
        token = strdup(buf2);
        read_hindex(token);
     }

     sprintf(buf, "%s/comm/", HELP_PATH);
     count = scandir(buf, &files, file_select, alphasort);

     for (i=1; i < count + 1;++i){
        token =strdup(files[i-1]->d_name);
        if (!strcmp(token, "WS_FTP.log")) continue;
        sprintf(buf2, "%s%s", buf, token);
        free_string(token);
        token = strdup(buf2);
        read_hindex(token);
     }

     sprintf(buf, "%s/concept/", HELP_PATH);
     count = scandir(buf, &files, file_select, alphasort);

     for (i=1; i < count + 1;++i){
        token =strdup(files[i-1]->d_name);
        if (!strcmp(token, "WS_FTP.log")) continue;
        sprintf(buf2, "%s%s", buf, token);
        free_string(token);
        token = strdup(buf2);
        read_hindex(token);
     }

     sprintf(buf, "%s/skills/", HELP_PATH);
     count = scandir(buf, &files, file_select, alphasort);

     for (i=1; i < count + 1;++i){
        token =strdup(files[i-1]->d_name);
        if (!strcmp(token, "WS_FTP.log")) continue;
        sprintf(buf2, "%s%s", buf, token);
        free_string(token);
        token = strdup(buf2);
        read_hindex(token);
     }

     return;
}

void read_hindex(char *filename) {
HELP_INDEX *pHelp;
FILE *fp;
ECOND_DATA *new_ec;
char *kwd;
bool done;
char *name;

    fp = fopen(filename, "r");
    if (!fp) {
        log_string("No help file found!");
        exit(1);
        return;
    }

    pHelp	= (HELP_INDEX *) alloc_perm( sizeof(*pHelp) );
    pHelp->file = strdup(filename);
    pHelp->ec = NULL;

  done = FALSE;
  while(!done) {
    kwd = fread_word(fp);

    switch (kwd[0]) {

      default:
        fread_to_eol(fp); 
        break; 

      case '<':
        switch(kwd[1]) {

            default:
                fread_to_eol(fp); 
                break; 

            case 'C':
                if (!str_cmp(kwd, "<CthulhuMud-Title")) {
                     name = fread_string_eol(fp);
                     pHelp->title = html_dup(name);
                     fread_to_eol(fp); 
                }

                if (!str_cmp(kwd, "<CthulhuMud-Index")) {
                     name = fread_string_eol(fp);
                     pHelp->type = HELP_UNKNOWN;
                     if (!str_cmp(name, "Basics>")) pHelp->type = HELP_BASICS;
                     if (!str_cmp(name, "Commands>")) pHelp->type = HELP_COMMANDS;
                     if (!str_cmp(name, "Concepts>")) pHelp->type = HELP_CONCEPTS;
                     if (!str_cmp(name, "Skills>")) pHelp->type = HELP_SKILLS;
                     if (!str_cmp(name, "Spells>")) pHelp->type = HELP_SPELLS;
                     fread_to_eol(fp); 
                }

                if (!str_cmp(kwd, "<CthulhuMud-Keywords")) {
                     name = fread_string_eol(fp);
                     pHelp->keywords = html_dup(name);

                }

                if (!str_cmp(kwd, "<CthulhuMud-Cond")) {
                     new_ec = read_econd(fp);
                     if (new_ec != NULL) {
                          new_ec->next = pHelp->ec;
                          pHelp->ec = new_ec;
                     }
                }
                break;

            case '/':
                if (!str_cmp(kwd, "</html>") || !str_cmp(kwd, "</head>")) {
                    done = TRUE;
                }
                break; 
        }
        break;
    }
  }
  fclose(fp);

  if ( hindex_first == NULL )  hindex_first = pHelp;
  if ( hindex_last  != NULL )  hindex_last->next = pHelp;

  hindex_last	= pHelp;
  pHelp->next	= NULL;
  top_hindex++;
  return;
}



void hedit( CHAR_DATA *ch, char *argument ) {
    char arg [MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( ch->pcdata->security == 0 )
	send_to_char( "HEdit: Insufficient security to modify area.\r\n", ch );

    if( command[0] == '\0' )    {
	hedit_show( ch, argument );
	return;
    }

    if ( !str_cmp(command, "done") )    {
	edit_done( ch );
	return;
    }

    if ( ch->pcdata->security == 0 )    {
	interpret( ch, arg );
	return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; hedit_table[cmd].name[0] != '\0'; cmd++ )    {
	if ( !str_cmp( command, hedit_table[cmd].name ) )	{
	    (*hedit_table[cmd].olc_fun) ( ch, argument );
	    return;
	}
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}



/* Entry point for editing help_data. */
void do_hedit( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    HELP_DATA   *iHelp;

    if( IS_NPC( ch ) )
	return;

    argument = one_argument( argument, arg );

    if( arg[0] == '\0' )
    {
	send_to_char( "Syntax:  edit help <keywords>\r\n", ch );
	return;
    }
    else
    {
	for( iHelp = help_first; iHelp; iHelp = iHelp->next )
	{
	    /*
	     * This help better not exist already!
	     */
	    if( is_name( arg, iHelp->keyword ) )
	    {
		ch->desc->pEdit = (void *)iHelp;
		ch->desc->editor = ED_HELP;
		break;
	    }
	}

	if( !iHelp )
	{
	    iHelp		= new_help();
	    iHelp->keyword	= str_dup( arg );

	    if( !help_first )
		help_first	= iHelp;
	    if( help_last )
		help_last->next	= iHelp;

	    help_last	= iHelp;
	    iHelp->next	= NULL;
	    ch->desc->pEdit     = (void *)iHelp;
	    ch->desc->editor = ED_HELP;
	}
    }
    return;
}

HEDIT ( hedit_save )
{
    FILE *fp=NULL;
    HELP_DATA *pHelp;

    log_string( "Saving help.are..." );


    fp = fopen ( "help.are", "w");

        if (!fp)
        {
                bug ("Could not open help.are for writing.",0);
                return FALSE;
        }

    fprintf( fp, "#HELPS\n\n" );

    for( pHelp = help_first; pHelp; pHelp = pHelp->next )
    {
        fprintf( fp, "%d %s~\n%s~\n",
          pHelp->level,
          pHelp->keyword,
          pHelp->text);
    }

    fprintf( fp, "0 $~\n\n\n#$\n" );

    fclose( fp );

    send_to_char( "Saved.\n", ch );

    return TRUE;
}


HEDIT( hedit_show )
{
    HELP_DATA *pHelp;
    char buf[MAX_STRING_LENGTH];
    
    if ( !EDIT_HELP( ch, pHelp ) )
    {
	send_to_char( "Null help file.\r\n", ch );
	return FALSE;
    }

    sprintf( buf,
            "Seen at level: [%d]\r\n"
            "Keywords:      [%s]\r\n"
            "Text:\r\n%s\r\n",
            pHelp->level, pHelp->keyword, pHelp->text );
    send_to_char( buf, ch );

    return FALSE;
}



HEDIT( hedit_create )
{
    HELP_DATA *iHelp;
    HELP_DATA *NewHelp;
    char buf[MAX_STRING_LENGTH];

    if ( !EDIT_HELP( ch, iHelp ) )
    {
	send_to_char( "Null help file.\r\n", ch );
	return FALSE;
    }

    if( argument[0] == '\0' )
    {
	send_to_char( "Syntax: create <keywords>\r\n", ch );
	return FALSE;
    }

    /*
     * This help better not exist already!
     */
    for( iHelp = help_first; iHelp; iHelp = iHelp->next )
    {
	if( is_name( argument, iHelp->keyword ) )
	{
	    send_to_char( "That help file already exists.\r\n", ch );
	    return FALSE;
	}
    }

    NewHelp		= new_help();
    NewHelp->keyword	= str_dup( argument );

    if( !help_first )	/* If it is we have a leak */
	help_first	= NewHelp;
    if( help_last )
	help_last->next	= NewHelp;

    help_last	= NewHelp;
    NewHelp->next	= NULL;
    ch->desc->pEdit	= (void *)NewHelp;
    ch->desc->editor = ED_HELP;

    sprintf( buf, "Created help with the keyword(s): %s\r\n",
	NewHelp->keyword );
    send_to_char( buf, ch );

    return TRUE;
}



HEDIT( hedit_delete )
{
    HELP_DATA *pHelp;
    HELP_DATA *PrevHelp=NULL;

    if ( !EDIT_HELP( ch, pHelp ) )
    {
	send_to_char( "Null help file.\r\n", ch );
	return FALSE;
    }

    if( argument[0] == '\0' )
    {
	send_to_char( "Syntax: delete <keyword>\r\n", ch );
	return FALSE;
    }

    /*
     * This help better exist
     */
    for( pHelp = help_first; pHelp; PrevHelp = pHelp, pHelp = pHelp->next )
    {
	if( is_name( argument, pHelp->keyword ) )
	    break;
    }

    if( !pHelp )
    {
	send_to_char( "That help file does not exist.\r\n", ch );
	return FALSE;
    }

    if( pHelp == (HELP_DATA *)ch->desc->pEdit )
    {
	edit_done( ch );
    }

    if( !PrevHelp )          /* At first help file   */
    {
	help_first  = pHelp->next;
	free_help( pHelp );
    }
    else if( !pHelp->next )  /* At the last help file*/
    {
	help_last           = PrevHelp;
	PrevHelp->next      = NULL;
	free_help( pHelp );
    }
    else                            /* Somewhere else...    */
    {
	PrevHelp->next      = pHelp->next;
	free_help( pHelp );
    }

    send_to_char( "Help file deleted.\r\n", ch );
    return TRUE;
}



HEDIT( hedit_desc )
{
    HELP_DATA *pHelp;

    if ( !EDIT_HELP( ch, pHelp ) )
    {
	send_to_char( "Null help file.\r\n", ch );
	return FALSE;
    }

    if ( argument[0] != '\0' )
    {
	send_to_char( "Syntax:  desc\r\n", ch );
	return FALSE;
    }
    
    string_append( ch, &pHelp->text );
    return TRUE;
}



HEDIT( hedit_level )
{
    HELP_DATA *pHelp;
    int value;

    if ( !EDIT_HELP( ch, pHelp ) )
    {
	send_to_char( "Null help file.\r\n", ch );
	return FALSE;
    }

    value = atoi( argument );

    if ( argument[0] == '\0' || value < -1 )
    {
	send_to_char( "Syntax:  level [level >= -1]\r\n", ch );
	return FALSE;
    }

    pHelp->level = value;
    send_to_char( "Help level set.\r\n", ch );

    return TRUE;
}



HEDIT( hedit_keywords )
{
    HELP_DATA *pHelp;
    int i;
    int length;

    if ( !EDIT_HELP( ch, pHelp ) )
    {
	send_to_char( "Null help file.\r\n", ch );
	return FALSE;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  keywords <keywords>\r\n", ch );
	return FALSE;
    }

    length = strlen(argument);
    for (i = 0; i < length; i++)
	argument[i] = toupper(argument[i]);

    pHelp->keyword = str_dup( argument );
    send_to_char( "Help keywords set.\r\n", ch );
    return TRUE;
}


void do_hlist( CHAR_DATA *ch, char *argument ) {
    int min, max, cnt;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[8*MAX_STRING_LENGTH]; 
    HELP_DATA *help;

    buf2[0] = '\0'; 
    buf[0]='\0';

    min = -1;
    max = get_trust(ch);

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if (!strcmp(arg, "online")) {
       if ( arg2[0] != '\0' )  min = atoi(arg2);
       if ( argument[0] != '\0' )  max = atoi(argument);
       if (max > get_trust(ch))   max = get_trust(ch);
       if (min < -1 )   min = -1;

       sprintf( buf, "{WHelp Topics in level range %d to %d:{x\r\n\r\n", min, max );
       strcat( buf2,buf); 
       for ( cnt = 0, help = help_first; help; help = help->next )
    	if ( help->level >= min && help->level <= max ) {
    	    sprintf( buf, "  {G%3d {Y%s{x\r\n", help->level, help->keyword );
  	    strcat(buf2,buf);
    	    ++cnt;
   	}
       if ( cnt )   {
           sprintf( buf, "\r\n%d pages found.\r\n", cnt );
           strcat(buf2,buf);
           page_to_char (buf2, ch); 
       } else send_to_char( "None found.\r\n", ch );
    } else {
       read_help_directory(ch, arg);
    }
    return;
}


void read_help_directory(CHAR_DATA *ch, char *argument) {
HELP_INDEX *pHelp;
MOB_CMD_CONTEXT *mcc;
char buf[MAX_INPUT_LENGTH];
char outbuf[8*MAX_STRING_LENGTH]; 

  mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL); 

  sprintf(outbuf, "{WOffline Help Topic:{x\r\n\r\n");
  if (argument[0] == '\0') argument = strdup("all");

  if (!strcmp(argument, "basics")) {
     strcat(outbuf, "{WBasics:{x\r\n");
     for (pHelp = hindex_first; pHelp; pHelp = pHelp->next) {
          if (pHelp->type == HELP_BASICS) {
             if (ec_satisfied_mcc(mcc, pHelp->ec, TRUE)) sprintf(buf,"{Y%s{x\r\n", pHelp->title);
             strcat(outbuf, buf);
          }
     }

  }else if (!strcmp(argument, "concepts")) {
     strcat(outbuf, "{WConcepts:{x\r\n");
     for (pHelp = hindex_first; pHelp; pHelp = pHelp->next) {
          if (pHelp->type == HELP_CONCEPTS) {
             if (ec_satisfied_mcc(mcc, pHelp->ec, TRUE)) sprintf(buf,"{Y%s{x\r\n", pHelp->title);
             strcat(outbuf, buf);
          }
     }

  }else if (!strcmp(argument, "commands")) {
     strcat(outbuf, "{WCommands:{x\r\n");
     for (pHelp = hindex_first; pHelp; pHelp = pHelp->next) {
          if (pHelp->type == HELP_COMMANDS) {
             if (ec_satisfied_mcc(mcc, pHelp->ec, TRUE)) sprintf(buf,"{Y%s{x\r\n", pHelp->title);
             strcat(outbuf, buf);
          }
     }

  } else if (!strcmp(argument, "skills")) {
     strcat(outbuf, "{WSkills:{x\r\n");
     for (pHelp = hindex_first; pHelp; pHelp = pHelp->next) {
          if (pHelp->type == HELP_SKILLS) {
             if (ec_satisfied_mcc(mcc, pHelp->ec, TRUE)) sprintf(buf,"{Y%s{x\r\n", pHelp->title);
             strcat(outbuf, buf);
          }
     }

  } else if (!strcmp(argument, "spells")) {
     strcat(outbuf, "{WSpells:{x\r\n");
     for (pHelp = hindex_first; pHelp; pHelp = pHelp->next) {
          if (pHelp->type == HELP_SPELLS) {
             if (ec_satisfied_mcc(mcc, pHelp->ec, TRUE)) sprintf(buf,"{Y%s{x\r\n", pHelp->title);
             strcat(outbuf, buf);
          }
     }

  }else if (!strcmp(argument, "all")) {
     strcat(outbuf, "{WAll Helps:{x\r\n");
     for (pHelp = hindex_first; pHelp; pHelp = pHelp->next) {
          if (ec_satisfied_mcc(mcc, pHelp->ec, TRUE)) sprintf(buf,"{Y%s{x\r\n", pHelp->title);
          strcat(outbuf, buf);
     }
  }

  free_mcc(mcc);
  if (!strcmp(outbuf, "{WOffline Help Topic:{x\r\n\r\n")) {
      send_to_char("HLIST Syntax:\r\n", ch);
      send_to_char("HLIST ONLINE\r\n", ch);
      send_to_char("HLIST ALL\r\n", ch);
      send_to_char("HLIST <basics, concepts, commands, skills>\r\n", ch);
      return;
  }

  page_to_char(outbuf, ch);
  return;
}


bool read_help_entry(CHAR_DATA *ch, char *argument) {
HELP_INDEX *pHelp;
FILE *fp;
MOB_CMD_CONTEXT *mcc;
char arg[MAX_STRING_LENGTH];
bool found = FALSE;
int more = 0;
int count = 0;

   mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL); 
   argument = one_argument(argument, arg);

   if (!str_cmp(arg, "more") && !IS_NPC(ch)) {
       for (pHelp = hindex_first; pHelp; pHelp = pHelp->next) {
           if (is_name(ch->pcdata->help_topic, pHelp->keywords)
           && ec_satisfied_mcc(mcc, pHelp->ec, TRUE)) {
                fp = fopen(pHelp->file, "r" );
                if (!fp) {
                     sprintf_to_char(ch, "File: %s not found\r\n", pHelp->file);
                     free_mcc(mcc);
                     return FALSE;
                }
                if (count > ch->pcdata->help_count) {
                    if (!found) {
                        ch->pcdata->help_count++;
                        if (IS_SET(ch->plr, PLR_IMP_HTML)) {
                             send_to_char("Click for help on:\r\n", ch);
                             sprintf_to_char(ch, "<a href=%s/%s>%s</a>\r\n", mud.url, strip_heading(pHelp->file, 7), pHelp->title);
                        } else {
                             parse_html(ch, fp);
                        }
                        found = TRUE;
                    } else {
                        more++;
                    }
                }
                count++;
            }
       }
   } else {
       for (pHelp = hindex_first; pHelp; pHelp = pHelp->next) {
           if (is_name(arg, pHelp->keywords)
           && ec_satisfied_mcc(mcc, pHelp->ec, TRUE)) {
                fp = fopen(pHelp->file, "r" );
                if (!fp) {
                     sprintf_to_char(ch, "File: %s not found\r\n", pHelp->file);
                     free_mcc(mcc);
                     return FALSE;
                }
                if (!found) {
                    if (!IS_NPC(ch)) {
                        free_string(ch->pcdata->help_topic);
                        ch->pcdata->help_topic = strdup(arg);
                        ch->pcdata->help_count = 0;
                    }
                    if (IS_SET(ch->plr, PLR_IMP_HTML)) {
                         send_to_char("Click for help on:\r\n", ch);
                         sprintf_to_char(ch, "<a href=%s/%s>%s</a>\r\n", mud.url, strip_heading(pHelp->file, 7), pHelp->title);
                    } else {
                        parse_html(ch, fp);
                    }
                    found = TRUE;
                } else {
                    more++;
                }
           }
       }
     }
     if (more > 0) sprintf_to_char(ch, "%d more helps found.\r\n", more);
     free_mcc(mcc);
     return found;
}


void parse_html(CHAR_DATA *ch, FILE *fp) {
char * kwd;
char buf1[MAX_STRING_LENGTH];
char buf[MAX_STRING_LENGTH];
char outbuf[10*MAX_STRING_LENGTH];
int ipos = 0;
int opos = 0;
bool in_tag = FALSE;
bool active = FALSE;

    sprintf(outbuf, "{YEmulated HTML reader:{x\r\n");
    sprintf(buf1, "Better quality in real HTML at %s/help/\r\n", mud.url);
    strcat(outbuf, buf1);

    kwd = fread_html(fp);
    if (!kwd) {
        do_help (ch, "nohelp");
        return;
    }

    while ( opos < MAX_STRING_LENGTH && kwd[ipos] != '\0' ) {
        if (active) {
            switch (kwd[ipos]) {
               default:
                  if (!in_tag) buf[opos++] = kwd[ipos];
                  break;

                case '<':
                  in_tag = TRUE;
                  break;

                case '>':
                  in_tag = FALSE;
                  break;

                case '&':
                   if (!in_tag) {
                        if (kwd[ipos+1] != '\0' && kwd[ipos+1] == 'n'
                        && kwd[ipos+2] != '\0' && kwd[ipos+2] == 'b'
                        && kwd[ipos+3] != '\0' && kwd[ipos+3] == 's'
                        && kwd[ipos+4] != '\0' && kwd[ipos+4] == 'p'
                        && kwd[ipos+5] != '\0' && kwd[ipos+5] == ';') {
                                ipos +=5;

                        } else if (kwd[ipos+1] != '\0' && kwd[ipos+1] == 'l'
                        && kwd[ipos+2] != '\0' && kwd[ipos+2] == 't'
                        && kwd[ipos+3] != '\0' && kwd[ipos+3] == ';') {
                                ipos +=3;
                        }
                   }
                   break;
            }
         } else {
            if (ipos > 6
            && kwd[ipos] == '>'
            && kwd[ipos-1] == 'd'
            && kwd[ipos-2] == 'a'
            && kwd[ipos-3] == 'e'
            && kwd[ipos-4] == 'h'
            && kwd[ipos-5] == '/'
            && kwd[ipos-6] == '<') active = TRUE;
         }
         ipos++;
    }
     buf[opos++] = '\0';

    strcat(outbuf, buf);          
    page_to_char(outbuf, ch);
    return;
}


char* html_dup(char *kwd) {
char buf[MAX_STRING_LENGTH];
char *out;
int ipos = 0;
int opos = 0;


    while ( opos < MAX_STRING_LENGTH && kwd[ipos] != '\0' ) {
            switch (kwd[ipos]) {
               default:
                  buf[opos++] = kwd[ipos];
                  break;

                case '<':
                  break;

                case '>':
                  break;

            }
         ipos++;
    }
     buf[opos++] = '\0';
     out = strdup(buf);
     return out;
}

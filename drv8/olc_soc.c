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
#include "olc.h"


#define SEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define EDIT_SOCIAL(Ch, Social)	( Social = (SOCIAL_DATA *)Ch->desc->pEdit )

/*
 * Help Editor Prototypes
 */
DECLARE_OLC_FUN( sedit_create 		);
DECLARE_OLC_FUN( sedit_delete 		);
DECLARE_OLC_FUN( sedit_show 		);
DECLARE_OLC_FUN( sedit_save		);
DECLARE_OLC_FUN( sedit_iff);
DECLARE_OLC_FUN( sedit_1);
DECLARE_OLC_FUN( sedit_2);
DECLARE_OLC_FUN( sedit_3);
DECLARE_OLC_FUN( sedit_4);
DECLARE_OLC_FUN( sedit_5);
DECLARE_OLC_FUN( sedit_6);
DECLARE_OLC_FUN( sedit_7);
DECLARE_OLC_FUN( sedit_8);

void        save_socials(void);

const struct olc_cmd_type sedit_table[] =
{
/*  {   command		function	}, */

    {   "commands",	show_commands	},
    {   "create",	sedit_create	},
    {   "delete",	sedit_delete	},
    {   "show",	sedit_show	},
    {   "style", 	sedit_iff	},
    {   "1", 	sedit_1	},
    {   "2", 	sedit_2	},
    {   "3", 	sedit_3	},
    {   "4", 	sedit_4	},
    {   "5", 	sedit_5	},
    {   "6", 	sedit_6	},
    {   "7", 	sedit_7	},
    {   "8", 	sedit_8	},
    {   "save", 	sedit_save	},
    {   "?",	show_help	},
    {   "",		0,		}
};


void sedit( CHAR_DATA *ch, char *argument ) {
    char arg [MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( ch->pcdata->security == 0 ) send_to_char( "SEdit: Insufficient security to modify socials.\r\n", ch );

    if( command[0] == '\0' )    {
	sedit_show( ch, argument );
	return;
    }

    if ( !str_cmp(command, "done") )  {
	edit_done( ch );
	return;
    }

    if ( ch->pcdata->security == 0 )    {
	interpret( ch, arg );
	return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; sedit_table[cmd].name[0] != '\0'; cmd++ )   {
	if ( !str_cmp( command, sedit_table[cmd].name )) {
	    (*sedit_table[cmd].olc_fun) ( ch, argument );
	    return;
	}
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}



/* Entry point for editing help_data. */
void do_sedit( CHAR_DATA *ch, char *argument ){
    char arg[MAX_INPUT_LENGTH];
    SOCIAL_DATA   *social;
    int i;

    if( IS_NPC( ch )) return;

    argument = one_argument( argument, arg );

    if( arg[0] == '\0' )    {
	send_to_char( "Syntax:  edit social <keywords>\r\n", ch );
	return;
    } else {
	for(i = 0; social_table[i].name[0] != '\0'; i++ ) {
	    if( is_name(arg, social_table[i].name ))   {
                          social = (SOCIAL_DATA *) alloc_perm( sizeof(*social) );
                          sprintf(social->name,"%s",social_table[i].name);
                          social->s_style = social_table[i].s_style;
                          if (social_table[i].char_no_arg) social->char_no_arg = strdup(social_table[i].char_no_arg);
                          else social->char_no_arg = NULL;
                          if (social_table[i].others_no_arg) social->others_no_arg = strdup(social_table[i].others_no_arg);
                          else social->others_no_arg = NULL;
                          if (social_table[i].char_found) social->char_found = strdup(social_table[i].char_found);
                          else social->char_found = NULL;
                          if (social_table[i].others_found) social->others_found = strdup(social_table[i].others_found);
                          else social->others_found = NULL;
                          if (social_table[i].vict_found) social->vict_found = strdup(social_table[i].vict_found);
                          else social->vict_found = NULL;
                          if (social_table[i].char_not_found) social->char_not_found = strdup(social_table[i].char_not_found);
                          else social->char_not_found = NULL;
                          if (social_table[i].char_auto) social->char_auto = strdup(social_table[i].char_auto);
                          else social->char_auto = NULL;
                          if (social_table[i].others_auto) social->others_auto = strdup(social_table[i].others_auto);
                          else social->others_auto = NULL;

	          ch->desc->pEdit = (void *)social;
	          ch->desc->editor = ED_SOCIAL;
	          break;
	    }
	}
    }
    return;
}

SEDIT (sedit_save ) {
SOCIAL_DATA *social;
int i;

    if ( !EDIT_SOCIAL( ch, social ))    {
	send_to_char( "Null Social file.\r\n", ch );
	return FALSE;
    }

	for(i = 0; social_table[i].name[0] != '\0'; i++ ) {
	    if(!strcmp(social->name, social_table[i].name ))   {

                          sprintf(social_table[i].name,"%s",social->name);
                          social_table[i].s_style = social->s_style;
                          if (social->char_no_arg) social_table[i].char_no_arg = strdup(social->char_no_arg);
                          else social_table[i].char_no_arg = NULL;
                          if (social->others_no_arg) social_table[i].others_no_arg = strdup(social->others_no_arg);
                          else social_table[i].others_no_arg = NULL;
                          if (social->char_found) social_table[i].char_found = strdup(social->char_found);
                          else social_table[i].char_found = NULL;
                          if (social->others_found) social_table[i].others_found = strdup(social->others_found);
                          else social_table[i].others_found = NULL;
                          if (social->vict_found) social_table[i].vict_found = strdup(social->vict_found);
                          else social_table[i].vict_found = NULL;
                          if (social->char_not_found) social_table[i].char_not_found = strdup(social->char_not_found);
                          else social_table[i].char_not_found = NULL;
                          if (social->char_auto) social_table[i].char_auto = strdup(social->char_auto);
                          else social_table[i].char_auto = NULL;
                          if (social->others_auto) social_table[i].others_auto = strdup(social->others_auto);
                          else social_table[i].others_auto = NULL;
                    }
               }

    log_string( "Saving Socials..." );
    save_socials();
    return TRUE;
}


SEDIT( sedit_show ) {
    SOCIAL_DATA *social;
    char buf[MAX_STRING_LENGTH];
    char *col;

    if ( !EDIT_SOCIAL( ch, social ))    {
	send_to_char( "Null Social file.\r\n", ch );
	return FALSE;
    }

    switch(social->s_style) {
        default:
         col = strdup("{W");
         break;
        case 1:
         col = strdup("{G");
         break;
        case 2:
         col = strdup("{R");
         break;
        case 3:
         col = strdup("{M");
         break;
    }

    sprintf (buf, "{CSocial: %s%s{x\r\n\n"
              "{c({C[1]{c) No argument given, character sees:{x\r\n"
              "{g%s{x\r\n\r\n"
              "{c({C[2]{c) No argument given, others see:{x\r\n"
              "{g%s{x\r\n\r\n"
              "{c({C[3]{c) Target found, character sees:{x\r\n"
              "{g%s{x\r\n\r\n"
              "{c({C[4]{c) Target found, others see:{x\r\n"
              "{g%s{x\r\n\r\n"
              "{c({C[5]{c) Target not found:{x\r\n"
              "{g%s{x\r\n\r\n"
              "{c({C[6]{c) Target found, victim sees:{x\r\n"
              "{g%s{x\r\n\r\n"
              "{c({C[7]{c) Target is character himself:{x\r\n"
              "{g%s{x\r\n\r\n"
              "{c({C[8]{c) Target is character himself, others see:{x\r\n"
              "{g%s{g\r\n",
              col,
              social->name,
              social->char_no_arg,
              social->others_no_arg,
              social->char_found,
              social->others_found,
              social->char_not_found,
              social->vict_found,
              social->char_auto,
              social->others_auto);
    send_to_char( buf, ch );
    return FALSE;
}

SEDIT( sedit_iff ) {
    SOCIAL_DATA *social;
   char arg[MAX_STRING_LENGTH];    

    if ( !EDIT_SOCIAL( ch, social ))    {
	send_to_char( "Null Social file.\r\n", ch );
	return FALSE;
    }

    one_argument(argument,arg);
    if (!strcmp(arg, "neutral")) {
         social->s_style = SOCIAL_NEUTRAL;
         send_to_char( "Style changed to {Wneutral{x.\r\n", ch );
    } else if (!strcmp(arg, "friendly")) {
         social->s_style = SOCIAL_FRIENDLY;
         send_to_char( "Style changed to {Gfriendly{x.\r\n", ch );
    } else if (!strcmp(arg, "hostile")) {
         social->s_style = SOCIAL_HOSTILE;
         send_to_char( "Style changed to {Rhostile{x.\r\n", ch );
    } else if (!strcmp(arg, "sexual")) {
         social->s_style = SOCIAL_SEXUAL;
         send_to_char( "Style changed to {Msexual{x.\r\n", ch );
    } else {
         send_to_char("Invalid social style. Avalid arguments are ({Wneutral{x, {Gfriendly{x, {Rhostile{x, {Msexual{x).\r\n", ch);
    }
    return TRUE;
}

SEDIT( sedit_1 ) {
    SOCIAL_DATA *social;
    
    if ( !EDIT_SOCIAL( ch, social ))    {
	send_to_char( "Null Social file.\r\n", ch );
	return FALSE;
    }

    if (argument) {
        free_string (social->char_no_arg);
        social->char_no_arg = str_dup(argument);		
        if (!argument[0]) send_to_char ("Character will now see nothing when this social is used without arguments.\r\n",ch);
        else sprintf_to_char (ch,"New message is now:\r\n{c%s{x\r\n", argument);
    }
    return TRUE;
}

SEDIT( sedit_2 ) {
    SOCIAL_DATA *social;
    
    if ( !EDIT_SOCIAL( ch, social ))    {
	send_to_char( "Null Social file.\r\n", ch );
	return FALSE;
    }

    if (argument) {
        free_string (social->others_no_arg);
        social->others_no_arg = str_dup(argument);		
        if (!argument[0]) send_to_char ("Others will now see nothing when this social is used without arguments.\r\n",ch);
        else sprintf_to_char (ch,"New message is now:\r\n{c%s{x\r\n", argument);
    }
    return TRUE;
}

SEDIT( sedit_3 ) {
    SOCIAL_DATA *social;
    
    if ( !EDIT_SOCIAL( ch, social ))    {
	send_to_char( "Null Social file.\r\n", ch );
	return FALSE;
    }

    if (argument) {
        free_string (social->char_found);
        social->char_found = str_dup(argument);		
        if (!argument[0]) send_to_char ("The character will now see nothing when a target is found.\r\n",ch);
        else sprintf_to_char (ch,"New message is now:\r\n{c%s{x\r\n", argument);
    }
    return TRUE;
}

SEDIT( sedit_4 ) {
    SOCIAL_DATA *social;
    
    if ( !EDIT_SOCIAL( ch, social ))    {
	send_to_char( "Null Social file.\r\n", ch );
	return FALSE;
    }

    if (argument) {
        free_string (social->others_found);
        social->others_found = str_dup(argument);		
        if (!argument[0]) send_to_char ("Others will now see nothing when a target is found.\r\n",ch);
        else sprintf_to_char (ch,"New message is now:\r\n{c%s{x\r\n", argument);
    }
    return TRUE;
}

SEDIT( sedit_5 ) {
    SOCIAL_DATA *social;
    
    if ( !EDIT_SOCIAL( ch, social ))    {
	send_to_char( "Null Social file.\r\n", ch );
	return FALSE;
    }

    if (argument) {
        free_string (social->char_not_found);
        social->char_not_found = str_dup(argument);		
        if (!argument[0]) send_to_char ("You will now see nothing when a target is not found.\r\n",ch);
        else sprintf_to_char (ch,"New message is now:\r\n{c%s{x\r\n", argument);
    }
    return TRUE;
}

SEDIT( sedit_6 ) {
    SOCIAL_DATA *social;
    
    if ( !EDIT_SOCIAL( ch, social ))    {
	send_to_char( "Null Social file.\r\n", ch );
	return FALSE;
    }

    if (argument) {
        free_string (social->vict_found);
        social->vict_found = str_dup(argument);		
        if (!argument[0]) send_to_char ("Victim will now see nothing when a target is found.\r\n",ch);
        else sprintf_to_char (ch,"New message is now:\r\n{c%s{x\r\n", argument);
    }
    return TRUE;
}

SEDIT( sedit_7 ) {
    SOCIAL_DATA *social;
    
    if ( !EDIT_SOCIAL( ch, social ))    {
	send_to_char( "Null Social file.\r\n", ch );
	return FALSE;
    }

    if (argument) {
        free_string (social->char_auto);
        social->char_auto = str_dup(argument);		
        if (!argument[0]) send_to_char ("Character will now see nothing when targetting self.\r\n",ch);
        else sprintf_to_char (ch,"New message is now:\r\n{c%s{x\r\n", argument);
    }
    return TRUE;
}

SEDIT( sedit_8 ) {
    SOCIAL_DATA *social;
    
    if ( !EDIT_SOCIAL( ch, social ))    {
	send_to_char( "Null Social file.\r\n", ch );
	return FALSE;
    }

    if (argument) {
        free_string (social->others_auto);
        social->others_auto = str_dup(argument);		
        if (!argument[0]) send_to_char ("Others will now see nothing when character targets self.\r\n",ch);
        else sprintf_to_char (ch,"New message is now:\r\n{c%s{x\r\n", argument);
    }
    return TRUE;
}

SEDIT( sedit_create ) {
    SOCIAL_DATA *social;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    int i;


    if ( !EDIT_SOCIAL( ch, social))    {
	send_to_char( "Null Social file.\r\n", ch );
	return FALSE;
    }

    if( argument[0] == '\0' )  {
	send_to_char( "Syntax: create <keywords>\r\n", ch );
	return FALSE;
    }

    one_argument(argument, arg);

    for(i = 0; social_table[i].name[0] != '\0'; i++ ) {
           if( is_name(arg, social_table[i].name ))   {
	send_to_char( "Social already exists.\r\n", ch );
	return FALSE;
           }
    }

    if (i > MAX_SOCIALS) {
	send_to_char( "Too many socials.\r\n", ch );
	return FALSE;
    }

    sprintf(social_table[i].name, "%s",arg);
    social_table[i].char_no_arg = NULL;
    social_table[i].others_no_arg = NULL;
    social_table[i].char_found = NULL;
    social_table[i].others_found = NULL;
    social_table[i].vict_found = NULL;
    social_table[i].char_not_found = NULL;
    social_table[i].char_auto = NULL;
    social_table[i].others_auto = NULL;
    social_table[i].s_style = 0;

                          sprintf(social->name,"%s",social_table[i].name);
                          social->s_style = social_table[i].s_style;
                          if (social_table[i].char_no_arg) social->char_no_arg = strdup(social_table[i].char_no_arg);
                          else social->char_no_arg = NULL;
                          if (social_table[i].others_no_arg) social->others_no_arg = strdup(social_table[i].others_no_arg);
                          else social->others_no_arg = NULL;
                          if (social_table[i].char_found) social->char_found = strdup(social_table[i].char_found);
                          else social->char_found = NULL;
                          if (social_table[i].others_found) social->others_found = strdup(social_table[i].others_found);
                          else social->others_found = NULL;
                          if (social_table[i].vict_found) social->vict_found = strdup(social_table[i].vict_found);
                          else social->vict_found = NULL;
                          if (social_table[i].char_not_found) social->char_not_found = strdup(social_table[i].char_not_found);
                          else social->char_not_found = NULL;
                          if (social_table[i].char_auto) social->char_auto = strdup(social_table[i].char_auto);
                          else social->char_auto = NULL;
                          if (social_table[i].others_auto) social->others_auto = strdup(social_table[i].others_auto);
                          else social->others_auto = NULL;

	          ch->desc->pEdit = (void *)social;
	          ch->desc->editor = ED_SOCIAL;


   sprintf( buf, "Created social with the keyword(s): %s\r\n",social_table[i].name );
   send_to_char( buf, ch );

    return TRUE;
}


SEDIT( sedit_delete ) {
SOCIAL_DATA *social;
int i, j;

    if ( !EDIT_SOCIAL( ch, social ))    {
	send_to_char( "Null Social file.\r\n", ch );
	return FALSE;
    }

    for(i = 0; social_table[i].name[0] != '\0'; i++ ) {
           if( is_name(social->name, social_table[i].name ))  break;
    }
    
    sprintf_to_char (ch, "%s deleted.\r\n", social_table[i].name);
    for (j = i; social_table[j].name[0] != '\0'; j++) {
          social_table[j] = social_table[j+1];
    }
    social_table[j-1].name[0] ='\0';
    return TRUE;
}




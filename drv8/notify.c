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
#include "wev.h"
#include "mob.h"

DECLARE_DO_FUN ( do_help );
	
struct event_type 
	{
	char *event_name;
	int  event_flag;
	int  div;
	};
	
struct event_type wiznet_table [] =
{
	{	"sites",		WIZNET_SITES, 			DIV_CREATOR	}, 
	{	"newbie",	WIZNET_NEWBIE, 		DIV_CREATOR	}, 
	{	"spam", 		WIZNET_SPAM, 		DIV_CREATOR	}, 
	{	"death",		WIZNET_DEATH, 		DIV_CREATOR	}, 
	{	"reset",		WIZNET_RESET, 		DIV_CREATOR	}, 
	{	"mobdeath",	WIZNET_MOBDEATH, 		DIV_CREATOR	}, 
	{	"bug",		WIZNET_BUG, 			DIV_CREATOR	}, 
	{	"switch",	WIZNET_SWITCH, 		DIV_CREATOR	}, 
	{	"links",		WIZNET_LINK, 			DIV_CREATOR	}, 
	{	"load",		WIZNET_LOAD, 		DIV_GOD		}, 
	{	"restore",	WIZNET_RESTORE, 		DIV_GOD		}, 
	{	"snoop",	WIZNET_SNOOP, 		DIV_GOD		}, 
	{	"secure",	WIZNET_SECURE, 		DIV_GOD		}, 
	{	"",		-1,			-1		}
};

struct event_type notify_table [] =
{
	{ "level",		NOTIFY_LEVEL, 			DIV_NEWBIE	},
	{ "death",		NOTIFY_DEATH, 		DIV_NEWBIE	},
	{ "delete",		NOTIFY_DELETE, 		DIV_NEWBIE	},
	{ "login",		NOTIFY_LOGIN, 			DIV_NEWBIE	},
	{ "quitgame",		NOTIFY_QUITGAME,		DIV_NEWBIE	},
	{ "lostlink",		NOTIFY_LOSTLINK,		DIV_NEWBIE	},
	{ "reconnect",		NOTIFY_RECONNECT,		DIV_NEWBIE	},
	{ "newnote",		NOTIFY_NEWNOTE,		DIV_NEWBIE	},
	{ "tick",			NOTIFY_TICK,			DIV_CREATOR	},
	{ "weather",		NOTIFY_WEATHER,		DIV_NEWBIE	},
	{ "repop",		NOTIFY_REPOP,			DIV_CREATOR	},
	{ "",		-1			-1		}
};
	
void do_wiznet ( CHAR_DATA *ch, char *argument ){
	char arg[MAX_INPUT_LENGTH];
	int count;
	int div;
	char outbuf[MAX_STRING_LENGTH];

	div = get_divinity(ch);

	if (argument == NULL || argument[0] == '\0'
		|| !str_cmp (argument, "status") )
		{	
		send_to_char ("{CWiznet Events{x\r\n",ch);
		send_to_char ("{w----------------{x\r\n",ch);
	
		for (count = 0 ; wiznet_table[count].event_name[0] != '\0' ; count++)
			{
			if (wiznet_table[count].div <= div ) 
				{
				sprintf (outbuf, "{c%-12s{x", wiznet_table[count].event_name);
				send_to_char(outbuf, ch);	
				if (IS_SET(ch->wiznet, wiznet_table[count].event_flag))
					send_to_char (" {gON{x\r\n",ch);
				else send_to_char (" {rOFF{x\r\n",ch);		 
				}
			}
		send_to_char ("\r\n",ch);
		return;
		} /* end event status display */
	else /* check for valid command */
		{
		one_argument (argument, arg);
		
		if (!str_cmp (arg, "off"))
			{
			ch->wiznet = 0;
			send_to_char ("All Wiznet events turned off.\r\n",ch);
			return;
			}	
		else if (!str_cmp (arg, "help"))
			{
			do_help (ch, "wiznet");
			return;
			}

		for (count = 0 ; wiznet_table[count].event_name[0] != '\0' ; count++)
			{
			if (!str_cmp (arg, wiznet_table[count].event_name) 
				&& div >= wiznet_table[count].div )
				{
				if (IS_SET(ch->wiznet, wiznet_table[count].event_flag))
					{
					ch->wiznet-=wiznet_table[count].event_flag;
					sprintf (outbuf, "Wiznet {c%s{x is now {roff{x.\r\n", wiznet_table[count].event_name);
					send_to_char (outbuf, ch);
					return;
					}
				else
					{
					ch->wiznet+=wiznet_table[count].event_flag;
					sprintf (outbuf, "Wiznet {c%s{x is now {gon{x.\r\n", wiznet_table[count].event_name);
					send_to_char (outbuf, ch);
					return;
					}
				}
			} /* end match command loop */
			send_to_char ("No such wiznet event...\r\n",ch);
		} /* end match command section */
	} /* end wiznet function */

void do_notify (CHAR_DATA *ch, char *argument)
	{
	char arg[MAX_INPUT_LENGTH];
	int count;
	int div;
	char outbuf[MAX_STRING_LENGTH];

	div = get_divinity(ch);

	if (argument == NULL || argument[0] == '\0'
		|| !str_cmp (argument, "status") )
		{	
		send_to_char ("{CNotify Events{x\r\n",ch);
		send_to_char ("{w----------------{x\r\n",ch);
	
		for (count = 0 ; notify_table[count].event_name[0] != '\0' ; count++)
			{
			if (notify_table[count].div <= div ) 
				{
				sprintf (outbuf, "{c%-12s{x", notify_table[count].event_name);
				send_to_char(outbuf, ch);	
				if (IS_SET(ch->notify, notify_table[count].event_flag))
					send_to_char (" {gON{x\r\n",ch);
				else send_to_char (" {rOFF{x\r\n",ch);		 
				}
			}
		send_to_char ("\r\n",ch);
		return;
		} /* end event status display */
	else /* check for valid command */
		{
		one_argument (argument, arg);
		
		if (!str_cmp (arg, "none"))
			{
			ch->notify = 0;
			send_to_char ("All Notify events turned off.\r\n",ch);
			return;
			}	
		else if (!str_cmp (arg, "help"))
			{
			do_help (ch, "notify");
			return;
			}
		else if (!str_cmp (arg, "all"))
			{
			ch->notify = NOTIFY_ALL;
			send_to_char ("All Notify events turned on.\r\n",ch);
			return;
			}

		for (count = 0 ; notify_table[count].event_name[0] != '\0' ; count++)
			{
			if (!str_cmp (arg, notify_table[count].event_name) 
				&& div >= notify_table[count].div )
				{
				if (IS_SET(ch->notify, notify_table[count].event_flag))
					{
					ch->notify-=notify_table[count].event_flag;
					sprintf (outbuf, "Notify {c%s{x is now {roff{x.\r\n", notify_table[count].event_name);
					send_to_char (outbuf, ch);
					return;
					}
				else
					{
					ch->notify+=notify_table[count].event_flag;
					sprintf (outbuf, "Notify {c%s{x is now {gon{x.\r\n", notify_table[count].event_name);
					send_to_char (outbuf, ch);
					return;
					}
				}
			} /* end match command loop */
			send_to_char ("No such notify event...\r\n",ch);
		} /* end match command section */
	} /* end notify function */

/* function to be called throughout code whenever notify or wiznet is needed */
void notify_message (CHAR_DATA *ch, long type, long to, char *extra_name)	{
	char buf[MAX_STRING_LENGTH];
 	DESCRIPTOR_DATA *d;
	bool need_vision=FALSE;
	bool notify_note=FALSE;
	bool notify_repop=FALSE;
	bool check_vict_lvl=FALSE;
	bool is_wiznet=FALSE;
	bool is_secure=FALSE;
	long plr_var_type;

               MOB_CMD_CONTEXT *mcc;
               WEV *wev;

/* Ok, hack for wiznet messaging without duplicating all this code */
	if (to >= TO_WIZNET)	{
	char tmpbuf[MAX_STRING_LENGTH];
	is_wiznet=TRUE;
	switch (type) {
	    case WIZNET_SITES:   
		sprintf (tmpbuf, "Connect by: [ %s ]\r\n", extra_name);
		check_vict_lvl = TRUE;
		break;				
	    case WIZNET_NEWBIE:
		sprintf (tmpbuf, "New player: [ %s ]\r\n", ch->short_descr);
		break;
	    case WIZNET_LINK:
		sprintf (tmpbuf, "Links: [ %s ]\r\n", extra_name);
        	check_vict_lvl = TRUE;
		break;
	    case WIZNET_SPAM:
		sprintf (tmpbuf, "Spam: [ %s ] [ %s ]\r\n", ch->short_descr, extra_name);
		break;
	    case WIZNET_DEATH:
		sprintf (tmpbuf, "Death: [ %s ]\r\n", extra_name);
		break; 
	    case WIZNET_RESET:
		sprintf (tmpbuf, "Repop: [ %s ]\r\n", extra_name);
		break;
	    case WIZNET_MOBDEATH:
		sprintf (tmpbuf, "Mob death: [ %s ]\r\n", extra_name);
		break;
	    case WIZNET_BUG:
		sprintf (tmpbuf, "Bug: [ %s ]\r\n", extra_name);
		break;
	    case WIZNET_SWITCH:
		sprintf (tmpbuf, "Switch: by [ %s ] into [ %s ]\r\n", ch->short_descr, extra_name);
		check_vict_lvl=TRUE;
		break;
	    case WIZNET_LOAD:
		sprintf (tmpbuf, "Load: by [ %s ] of [ %s ]\r\n", ch->short_descr, extra_name);
		check_vict_lvl=TRUE;
		break;
	    case WIZNET_RESTORE:
		sprintf (tmpbuf, "Restore: by [ %s ] of [ %s ]\r\n", ch->short_descr, extra_name);
		check_vict_lvl=TRUE;
		break;
	    case WIZNET_SNOOP:
		sprintf (tmpbuf, "Snoop: by [ %s ] of [ %s ]\r\n", ch->short_descr, extra_name);
		check_vict_lvl=TRUE;
		break;
	    case WIZNET_SECURE:
		sprintf (tmpbuf, "Secure:  [ %s ]\r\n", extra_name);
		is_secure = TRUE;
		break;
	    default:
		sprintf (tmpbuf, "Unrecognized wiznet event, please inform coders.\r\n");
		break;
	} /* end switch */
	sprintf (buf, "{y---- {BWIZNET{y ----{x\r\n");
	strcat (buf, tmpbuf);
	} else {
	is_wiznet=FALSE;
	switch (type) {
	    case NOTIFY_LEVEL:
                                if (IS_NPC(ch)
                                && ch->master != NULL) {
      		     sprintf (buf, "{BNotify{r-{x({g%s{x) %s has gained a level!\r\n",ch->master->short_descr, ch->short_descr);
                                } else {
  		     sprintf (buf, "{BNotify{r-{x %s has gained a level!\r\n", ch->short_descr);
                                }
		break;
	    case NOTIFY_LOGIN:
		sprintf (buf, "{BNotify{r-{x %s has entered the portal leading to Cthulhumud.\r\n", ch->short_descr);
		need_vision=TRUE;
		break; 
	    case NOTIFY_QUITGAME:
		sprintf (buf, "{BNotify{r-{x %s has found the portal back to reality.\r\n", ch->short_descr);
		need_vision=TRUE;
		break;
	    case NOTIFY_DELETE:
		sprintf (buf, "{BNotify{r-{x %s has deleted...\r\n", ch->short_descr);
		break;
	    case NOTIFY_DEATH:
		if (str_cmp(ch->name, extra_name)) {
                                   if ( IS_SET(ch->plr, PLR_PERMADEATH) || IS_SET(mud.flags, MUD_PERMADEATH) ) {
		     sprintf(buf, "{BNotify{r-{x %s permanently slain by %s.\r\n", ch->short_descr, extra_name);
                  } else {
                           if  (ch->hit>-6) {
                                sprintf(buf,"{BNotify{r-{x %s stunned by %s.\r\n", ch->short_descr, extra_name);
                           } else {
                                sprintf(buf,"{BNotify{r-{x %s killed by %s.\r\n", ch->short_descr, extra_name);
                           }    
                  }
		} else {
                  if (IS_SET(ch->plr, PLR_PERMADEATH)) {
		    sprintf (buf, "{BNotify{r-{x %s has permanently died.\r\n", ch->short_descr);
                  } else {
                            if  (ch->hit>-6) {
                                sprintf (buf, "{BNotify{r-{x %s has been stunned.\r\n", ch->short_descr);
                            } else {
                                sprintf (buf, "{BNotify{r-{x %s has died.\r\n", ch->short_descr);
                            }
                           }
	    }
		break; 
	    case NOTIFY_LOSTLINK:
		sprintf (buf, "{BNotify{r-{x %s has gone link dead.\r\n", ch->short_descr);
		need_vision=TRUE;
                                mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL);
                                wev = get_wev(WEV_CONTROL, WEV_CONTROL_LINKDEAD, mcc,
                                            NULL,
                                            NULL,
                                            NULL);
                                room_issue_wev(ch->in_room, wev);
                                free_wev(wev);
		break;

	    case NOTIFY_RECONNECT:
		sprintf (buf, "{BNotify{r-{x %s has reconnected.\r\n", ch->short_descr);
		need_vision=TRUE;
                                mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL);
                                wev = get_wev(WEV_CONTROL, WEV_CONTROL_RECONNECT, mcc,
                                            NULL,
                                            NULL,
                                            NULL);
                                room_issue_wev(ch->in_room, wev);
                                free_wev(wev);
		break;

	    case NOTIFY_NEWNOTE:
		sprintf (buf, 
                        "{BNotify{r-{x A new message has been posted!.\r\n");
		notify_note=TRUE;	
		need_vision = TRUE;
		break;
	    case NOTIFY_TICK:
		sprintf (buf, "TICK...\r\n");
		break;
	    case NOTIFY_REPOP:
		sprintf (buf, "{CRepop:{x Area %s has reset.\r\n", extra_name);
		notify_repop=TRUE;
		break;
	    default:
		{
		char messbuf[80];
		sprintf (messbuf, "Unrecognized NOTIFY code [%ld]", type);
		bug (messbuf,0);
		break;
		}
	} /*end switch*/
    } /* end else notify message section */

	/* got message, send to appropriate characters */	
 
    for ( d = descriptor_list; d != NULL; d = d->next )
    	{ 
        CHAR_DATA *victim;
 
        victim = d->original ? d->original : d->character;
		
		if (d->connected != CON_PLAYING)
			continue;

		if (is_wiznet)
			plr_var_type = victim->wiznet;
		else
			plr_var_type = victim->notify;
 
        if ( d->connected == CON_PLAYING 
        && IS_SET(plr_var_type,type)
        && !IS_SET(victim->comm,COMM_QUIET)
        && !IS_RAFFECTED(victim->in_room, RAFF_SILENCE)) {
			if (is_secure && ch == victim)
				continue;
			if (check_vict_lvl && get_divinity(victim) < get_divinity(ch))
				continue;
			if (notify_note )
				continue;
			if (need_vision && !can_see (victim, ch) )
				continue;	
			if (notify_repop && (!IS_IMMORTAL(victim) || str_cmp(victim->in_room->area->name, extra_name) ) )
				continue;
			if ( to == TO_IMM && !IS_IMMORTAL(victim) )
				continue;
			if ( to == TO_IMM_ADMIN && get_divinity(victim) < DIV_GREATER)
				continue;
			if ( to == TO_IMP && !IS_IMP(victim) )
				continue;

			send_to_char (buf, victim);
			}
		} /*end for*/
	} /*end notify_message */

	

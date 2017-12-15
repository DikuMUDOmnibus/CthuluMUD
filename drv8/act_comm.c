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
#include "affect.h"
#include "mob.h"
#include "wev.h"
#include "skill.h"
#include "profile.h"
#include "bank.h"
#include "text.h"
#include "race.h"
#include "exp.h"
#include "board.h"
#include "partner.h"
#include "gsn.h"


/* command procedures needed */
DECLARE_DO_FUN(do_quit);
DECLARE_DO_FUN(do_converse);
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_visible);

/*
 * Local functions.
 */

void		raw_kill			(CHAR_DATA *victim, bool corpse );
bool 		channel_social		(CHAR_DATA *ch, char *command, char *argument, int channel_id );
IGNORE 	*new_ignore		(void );
void 		free_ignore		(IGNORE *ignore, IGNORE *ignore_prev);
bool 		check_ignore		(CHAR_DATA *ch, char* argument);
bool		check_social		(CHAR_DATA *ch, char *command, char *argument);



void delete_char(CHAR_DATA *ch) {
char name[MAX_STRING_LENGTH];
char strsave[MAX_INPUT_LENGTH];

   /* Safty check... */

    if ( ch == NULL
      || IS_NPC(ch) ) {
 	return;
    }
  
   /* Zeran - notify_message */

    notify_message (ch, NOTIFY_DELETE, TO_ALL, NULL);	
 
    sprintf(name, "%s%s", PLAYER_DIR, capitalize(ch->name));

    do_quit(ch,"");
  
    if (ch->pcdata->in_progress) free_note (ch->pcdata->in_progress);

   /* Make sure we get them... */

    sprintf( strsave, "%s.gz", name );
    unlink(strsave);

    sprintf( strsave, "%s", name );
    unlink(strsave);

    sprintf( strsave, "%s.locker.gz", name );
    unlink(strsave);

    sprintf( strsave, "%s.locker", name );
    unlink(strsave);

   /* Hmmm. Banks, Societies... */

    delete_accounts(ch);
 
   /* All done... */

    return;
}


/* RT code to delete yourself */

void do_delet( CHAR_DATA *ch, char *argument)
{
    send_to_char("You must type the full command to delete yourself.\r\n",ch);
}

void do_delete( CHAR_DATA *ch, char *argument) {

    if (IS_NPC(ch)) {
 	return;
    }
  
    if (ch->pcdata->confirm_delete) {

      if (argument[0] != '\0') {
        send_to_char("Delete status removed.\r\n",ch);
        ch->pcdata->confirm_delete = FALSE;
        return;
      } else {

        delete_char(ch);

        return;
      }
    }

    if (argument[0] != '\0') {
	send_to_char("Just type delete. No argument.\r\n",ch);
	return;
    }

    send_to_char("Type delete again to confirm this command.\r\n",ch);
    send_to_char("WARNING: this command is irreversible.\r\n",ch);
    send_to_char("Typing delete with an argument will undo delete status.\r\n",
	ch);
    ch->pcdata->confirm_delete = TRUE;
}
	    

/* RT code to display channel status */

void do_channels( CHAR_DATA *ch, char *argument) {
   /* lists all channels and their status */

    send_to_char("   channel     status\r\n",ch);
    send_to_char("---------------------\r\n",ch);
 
    send_to_char("gossip         ",ch);
    if (!IS_SET(ch->comm,COMM_NOGOSSIP)) send_to_char("ON\r\n",ch);
    else send_to_char("OFF\r\n",ch);

    send_to_char("music          ",ch);
    if (!IS_SET(ch->comm,COMM_NOMUSIC))
      send_to_char("ON\r\n",ch);
    else
      send_to_char("OFF\r\n",ch);

    send_to_char("Q and A        ",ch);
    if (!IS_SET(ch->comm,COMM_NOQUESTION))
      send_to_char("ON\r\n",ch);
    else
      send_to_char("OFF\r\n",ch);

    if (IS_HERO(ch)) {
      send_to_char("hero channel   ",ch);
      if (!IS_SET(ch->comm, COMM_NOHERO)) send_to_char("ON\r\n",ch);
      else send_to_char("OFF\r\n",ch);

      send_to_char("god channel    ",ch);
      if(!IS_SET(ch->comm,COMM_NOWIZ)) send_to_char("ON\r\n",ch);
      else send_to_char("OFF\r\n",ch);
    }

   /* What you listen to... */

    send_to_char("shouts         ",ch);
    if (!IS_SET(ch->comm,COMM_DEAF))
      send_to_char("ON\r\n",ch);
    else
      send_to_char("OFF\r\n",ch);

    send_to_char("quiet mode     ",ch);
    if (IS_SET(ch->comm,COMM_QUIET))
      send_to_char("ON\r\n",ch);
    else
      send_to_char("OFF\r\n",ch);

   /* Scroll length... */
   
    if (ch->lines != PAGELEN) {
	if (ch->lines) {
  	   sprintf_to_char(ch, "You display %d lines of scroll.\r\n",ch->lines+2);
 	} else {
                   send_to_char("Scroll buffering is off.\r\n",ch);
                }
    }

   /* Inhibited actions... */

    if (IS_SET(ch->comm,COMM_NOSHOUT))  send_to_char("You cannot shout.\r\n",ch);
    if (IS_SET(ch->comm,COMM_NOTELL))  send_to_char("You cannot use tell.\r\n",ch);
    if (IS_SET(ch->comm,COMM_NOCHANNELS)) send_to_char("You cannot use channels.\r\n",ch);
    if (IS_SET(ch->comm,COMM_NOEMOTE)) send_to_char("You cannot show emotions.\r\n",ch);

   /* All done... */

    return;
}

/* RT deaf blocks out all shouts */

void do_deaf( CHAR_DATA *ch, char *argument) {

    if (IS_SET(ch->comm,COMM_NOSHOUT)) {
      send_to_char("The gods have taken away your ability to shout.\r\n",ch);
      return;
    }
    
    if (IS_SET(ch->comm,COMM_DEAF)) {
      send_to_char("You can now hear shouts again.\r\n",ch);
      REMOVE_BIT(ch->comm,COMM_DEAF);
    } else {
      send_to_char("From now on, you won't hear shouts.\r\n",ch);
      SET_BIT(ch->comm,COMM_DEAF);
    }

    return;
}

/* RT quiet blocks out all Out of Character communication */

void do_quiet ( CHAR_DATA *ch, char * argument) {

    if (IS_SET(ch->comm,COMM_QUIET)) {
      send_to_char("OOC Communications enabled.\r\n",ch);
      REMOVE_BIT(ch->comm,COMM_QUIET);
    } else {
      send_to_char("OOC communications disabled.\r\n",ch);
      SET_BIT(ch->comm,COMM_QUIET);
    }

    return; 
}

/* Set the language you are speaking... */

void do_speak(CHAR_DATA *ch, char *args) {
  char lang_name[MAX_INPUT_LENGTH];
  bool priv;
  int  lang_sn;

 /* Extract first word... */

  args = one_argument(args, lang_name);

 /* Check for private? */

  priv = FALSE;

  if (!str_cmp(lang_name, "private")) {
    priv = TRUE;

    args = one_argument(args, lang_name);
  }

 /* Check syntax... */

  if ( lang_name[0] == '\0'
    || args[0] != '\0') {

    send_to_char("Syntax: speak 'language'\r\n" 
                 "        speak private 'language'\r\n", ch);

    return;
  }

 /* Now go find the language... */

  lang_sn = get_skill_sn(lang_name);

  if (lang_sn < 1) {
    send_to_char("That is not a real language!\r\n", ch);
    return;
  }

  if (!IS_SET(skill_array[lang_sn].group, SKILL_GROUP_LANG)) {
    send_to_char("That isn't a real language!\r\n", ch);
    return;
  }

  if (get_skill(ch, lang_sn) < SKILL_DABBLER ) {
    send_to_char("You don't speak it well enough.\r\n", ch);
    return;
  }

 /* Ok, set up time... */

  if (priv) {
    ch->speak[SPEAK_PRIVATE] = lang_sn;
    sprintf_to_char(ch, "You are now speaking %s for private communications.\r\n", skill_array[lang_sn].name);
  } else {
    ch->speak[SPEAK_PUBLIC] = lang_sn;
    sprintf_to_char(ch, "You are now speaking %s for public communications.\r\n", skill_array[lang_sn].name);
  }

 /* All done... */

  return;
}

/* OOCC Beep */

void do_beep( CHAR_DATA *ch, char *argument ) {

    CHAR_DATA *victim;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

    if ( IS_SET( ch->comm, COMM_NOTELL ) ) {
        send_to_char( "Your beep didn't get through.\r\n", ch );
        return;
    }

    if ( IS_SET( ch->comm, COMM_QUIET ) ) {
        send_to_char( "You must turn off quiet mode first.\r\n", ch);
        return;
    }

    victim = get_char_world( ch, argument );

    if ( victim == NULL ) {
        send_to_char( "Nobody of that name playing.\r\n", ch );
        return;
    }

   /* Check mana... */
 
    if (ch->mana < 10) {
      send_to_char("You don't have enough mana left!\r\n", ch);
      return;
    }

    ch->mana -= 10;

   /* Ok, get context and build the wev... */

    mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 0, NULL);

    wev = get_wev(WEV_OOCC, WEV_OOCC_BEEP, mcc,
                 "\a{YYou beep @v2.{x\r\n",
                 "\a{Y@a2 beeps you!{x\r\n",
                 "\a{Y@a2 beeps @v2.{x\r\n");

   /* Check to see if anyone objects... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

   /* Route the wev directly to the receiving mob... */

    mob_handle_wev(ch, wev, "OOC");
    mob_handle_wev(victim, wev, "OOC");

   /* Free the wev (will autofree the context)... */

    free_wev(wev);
    return;
}


/* OOCC Gossip */

void do_gossip( CHAR_DATA *ch, char *argument ) {
MOB_CMD_CONTEXT *mcc;
WEV *wev = NULL;    
char msg[MAX_STRING_LENGTH];
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
int i;
    
    if (argument[0] == '\0' ) {
      if (IS_SET(ch->comm,COMM_NOGOSSIP)) {
        send_to_char("Gossip channel is now ON.\r\n",ch);
        REMOVE_BIT(ch->comm,COMM_NOGOSSIP);
      } else {
        send_to_char("Gossip channel is now OFF.\r\n",ch);
        SET_BIT(ch->comm,COMM_NOGOSSIP);
      }

      return;
    }

    if (IS_RAFFECTED(ch->in_room, RAFF_SILENCE)) {
      send_to_char("You can't do that here.\r\n",ch);
      return;
    }

    if (IS_SET(ch->comm, COMM_QUIET)) {
      send_to_char("You must turn off quiet mode first.\r\n",ch);
      return;
    }
 
    if (IS_SET(ch->comm, COMM_NOCHANNELS)) {
      send_to_char("The gods have revoked your channel priviliges.\r\n",ch);
      return;
    }

    REMOVE_BIT(ch->comm,COMM_NOGOSSIP);
 
   /* Convert the message... */

   if (argument[0] =='@') {
     CHAR_DATA *mud;
     int rn;
     
     argument = one_argument(argument, arg1);
  
     for (i=1; i <= MAX_INPUT_LENGTH; i++) {
          arg1[i-1] = arg1[i];
     }

     if (!str_cmp(arg1, "all")) {

         if (ch->mana < 50) {
             send_to_char("You don't have enough mana left!\r\n", ch);
             return;
         }
         ch->mana -= 50;

         for (rn=0; rn < MAX_PARTNER; rn++) {
               if (!partner_array[rn].name) continue;

              sprintf_to_char(ch, "{MYou gossip to all '%s{M'{x\r\n", argument);
              sprintf_to_partner(rn, "mudgossip %s %s\r\n", ch->name, prepare_for_transfer(argument));
         }
         return;

     } else {
         if ((mud = get_char_world(ch, arg1)) != NULL) {
             rn = am_i_partner(mud);
             if (rn < 0) return;

             if (ch->mana < 50) {
                 send_to_char("You don't have enough mana left!\r\n", ch);
                 return;
             }
             ch->mana -= 50;

             sprintf_to_char(ch, "{MYou gossip to %s '%s{M'{x\r\n", partner_array[rn].name, argument);
         
             sprintf_to_partner(rn, "mudgossip %s %s\r\n", ch->name, prepare_for_transfer(argument));
             return;
         }
      }


   } else if (argument[0] == '#') {
       argument = one_argument(argument, arg1);
       argument = one_argument(argument, arg2);
  
       for (i=1; i <= MAX_INPUT_LENGTH; i++) {
            arg1[i-1] = arg1[i];
       }

       if (ch->mana < 10) {
            send_to_char("You don't have enough mana left!\r\n", ch);
            return;
       }

       ch->mana -= 10;

       if (!channel_social(ch, arg1, arg2, 0)) send_to_char("Unknown Social Command.\r\n", ch);
       return;    
    }

    if ( !strip_html(argument, msg, TRUE, MAX_STRING_LENGTH, FONT_MAGENTA)) {
      send_to_char("Those words many not be uttered!\r\n", ch);
      return;
    }

   /* Check mana... */
 
    if (ch->mana < 10) {
      send_to_char("You don't have enough mana left!\r\n", ch);
      return;
    }

    ch->mana -= 10;

   /* Build the event... */

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, gsn_english, msg);

          if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)) {
              wev = get_wev(WEV_OOCC, WEV_OOCC_GOSSIP, mcc,
                 "{mYou gossip '@t0{m'{x\r\n",
                  NULL,
                 "{m@a2 ({gPet{m) gossips '@t0{m'{x\r\n");
          } else {
                 if (ch->desc) {
                    if (ch->desc->original && !IS_IMMORTAL(ch->desc->original)) {
                         wev = get_wev(WEV_OOCC, WEV_OOCC_GOSSIP, mcc,
                             "{mYou gossip '@t0{m'{x\r\n",
                             NULL,
                             "{m@a2 ({gPet{m) gossips '@t0{m'{x\r\n");
                    }
                 }
          }

          if (!wev) {
             wev = get_wev(WEV_OOCC, WEV_OOCC_GOSSIP, mcc,
                 "{mYou gossip '@t0{m'{x\r\n",
                  NULL,
                 "{m@a2 gossips '@t0{m'{x\r\n");
          }

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


void do_chatter( CHAR_DATA *ch, char *argument ) {
MOB_CMD_CONTEXT *mcc;
WEV *wev = NULL;    
    
    if (IS_NPC(ch)) return;

    if (argument[0] == '\0' ) {
      if (IS_SET(ch->comm,COMM_NOCHAT)) {
        send_to_char("Chat channel is now ON.\r\n",ch);
        REMOVE_BIT(ch->comm,COMM_NOCHAT);
      } else {
        send_to_char("Chat channel is now OFF.\r\n",ch);
        SET_BIT(ch->comm,COMM_NOCHAT);
      }
      return;
    }

    REMOVE_BIT(ch->comm,COMM_NOCHAT);

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, gsn_english, argument);
    wev = get_wev(WEV_OOCC, WEV_OOCC_CHAT, mcc,
                 "{m[{M@a2{m]: @t0{x\r\n",
                  NULL,
                 "{m[{M@a2{m]: @t0{x\r\n");

    world_issue_wev(wev, "Chat");
    free_wev(wev);
    return;
}


void do_herotalk( CHAR_DATA *ch, char *argument ) {
MOB_CMD_CONTEXT *mcc;
WEV *wev;    
char msg[MAX_STRING_LENGTH];
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
int i;

    if (argument[0] == '\0' ) {
      if (IS_SET(ch->comm, COMM_NOHERO)) {
        send_to_char("Hero channel is now ON.\r\n",ch);
        REMOVE_BIT(ch->comm, COMM_NOHERO);
      } else {
        send_to_char("Hero channel is now OFF.\r\n",ch);
        SET_BIT(ch->comm, COMM_NOHERO);
      }

      return;
    }

    if (IS_RAFFECTED(ch->in_room, RAFF_SILENCE)) {
      send_to_char("You can't do that here.\r\n",ch);
      return;
    }

    if (IS_SET(ch->comm,COMM_QUIET)) {
      send_to_char("You must turn off quiet mode first.\r\n",ch);
      return;
    }
 
    if (IS_SET(ch->comm, COMM_NOCHANNELS)) {
      send_to_char("The gods have revoked your channel priviliges.\r\n",ch);
      return;
    }

    REMOVE_BIT(ch->comm, COMM_NOHERO);
 
   /* Convert the message... */

   if (argument[0] =='@') {
     CHAR_DATA *mud;
     int rn;
     
     argument = one_argument(argument, arg1);
  
     for (i=1; i <= MAX_INPUT_LENGTH; i++) {
          arg1[i-1] = arg1[i];
     }

     if (!str_cmp(arg1, "all")) {

         for (rn=0; rn < MAX_PARTNER; rn++) {
               if (!partner_array[rn].name) continue;

              sprintf_to_char(ch, "{g[{GHERO{g]: '%s'{x\r\n", argument);
              sprintf_to_partner(rn, "mudhero %s %s\r\n", ch->name, prepare_for_transfer(argument));
         }
         return;

     } else {
         if ((mud = get_char_world(ch, arg1)) != NULL) {
             rn = am_i_partner(mud);
             if (rn < 0) return;

             sprintf_to_char(ch, "{g[{GHERO{g]: '%s'{x\r\n", argument);
             sprintf_to_partner(rn, "mudhero %s %s\r\n", ch->name, prepare_for_transfer(argument));
             return;
         }
      }
   }else if (argument[0] == '#') {
       argument = one_argument(argument, arg1);
       argument = one_argument(argument, arg2);
  
       for (i=1; i <= MAX_INPUT_LENGTH; i++) {
            arg1[i-1] = arg1[i];
       }

       if (ch->mana < 10) {
            send_to_char("You don't have enough mana left!\r\n", ch);
            return;
       }

       ch->mana -= 10;

       if (!channel_social(ch, arg1, arg2, 1)) send_to_char("Unknown Social Command.\r\n", ch);
       return;    
   }

    if ( !strip_html(argument, msg, TRUE, MAX_STRING_LENGTH, FONT_MAGENTA)) {
      send_to_char("Those words many not be uttered!\r\n", ch);
      return;
    }

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, gsn_english, msg);
    wev = get_wev(WEV_OOCC, WEV_OOCC_HERO, mcc,
                 "{g[{G@a2{g]: @t0{x\r\n",
                  NULL,
                 "{g[{G@a2{g]: @t0{x\r\n");

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    world_issue_wev(wev, "OOC");
    free_wev(wev);
    return;
}


void do_investigatortalk( CHAR_DATA *ch, char *argument ) {
MOB_CMD_CONTEXT *mcc;
WEV *wev;    
char msg[MAX_STRING_LENGTH];
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
int i;

    if (argument[0] == '\0' ) {
      if (IS_SET(ch->comm, COMM_NOINV)) {
        send_to_char("Investigator channel is now ON.\r\n",ch);
        REMOVE_BIT(ch->comm, COMM_NOINV);
      } else {
        send_to_char("Investigator channel is now OFF.\r\n",ch);
        SET_BIT(ch->comm, COMM_NOINV);
      }

      return;
    }

    if (IS_RAFFECTED(ch->in_room, RAFF_SILENCE)) {
      send_to_char("You can't do that here.\r\n",ch);
      return;
    }

    if (IS_SET(ch->comm,COMM_QUIET)) {
      send_to_char("You must turn off quiet mode first.\r\n",ch);
      return;
    }
 
    if (IS_SET(ch->comm, COMM_NOCHANNELS)) {
      send_to_char("The gods have revoked your channel priviliges.\r\n",ch);
      return;
    }

    REMOVE_BIT(ch->comm, COMM_NOINV);
 
    if (argument[0] == '#') {
       argument = one_argument(argument, arg1);
       argument = one_argument(argument, arg2);
  
       for (i=1; i <= MAX_INPUT_LENGTH; i++) {
            arg1[i-1] = arg1[i];
       }

       if (ch->mana < 10) {
            send_to_char("You don't have enough mana left!\r\n", ch);
            return;
       }

       ch->mana -= 10;

       if (!channel_social(ch, arg1, arg2, 2)) send_to_char("Unknown Social Command.\r\n", ch);
       return;    
   }

    if ( !strip_html(argument, msg, TRUE, MAX_STRING_LENGTH, FONT_MAGENTA)) {
      send_to_char("Those words many not be uttered!\r\n", ch);
      return;
    }

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, gsn_english, msg);
    wev = get_wev(WEV_OOCC, WEV_OOCC_INVESTIGATOR, mcc,
                 "{b[{B@a2{b]: @t0{x\r\n",
                  NULL,
                 "{b[{B@a2{b]: @t0{x\r\n");

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    world_issue_wev(wev, "OOC");
    free_wev(wev);
    return;
}


/* OOCC Question */

void do_question( CHAR_DATA *ch, char *argument ) {
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;    

    char msg[MAX_STRING_LENGTH];

    if (argument[0] == '\0' ) {
      if (IS_SET(ch->comm,COMM_NOQUESTION)) {
        send_to_char("Q/A channel is now ON.\r\n",ch);
        REMOVE_BIT(ch->comm,COMM_NOQUESTION);
      } else {
        send_to_char("Q/A channel is now OFF.\r\n",ch);
        SET_BIT(ch->comm,COMM_NOQUESTION);
      }

      return; 
    }

    if (IS_SET(ch->comm,COMM_QUIET)) {
      send_to_char("You must turn off quiet mode first.\r\n",ch);
      return;
    }
 
    if (IS_SET(ch->comm,COMM_NOCHANNELS)) {
      send_to_char("The gods have revoked your channel priviliges.\r\n",ch);
      return;
    }
 
    REMOVE_BIT(ch->comm,COMM_NOQUESTION);
 
   /* Convert the message... */

    if ( !strip_html(argument, msg, TRUE, MAX_STRING_LENGTH, FONT_YELLOW)) {
      send_to_char("That question may not be asked!\r\n", ch);
      return;
    }

   /* Check mana... */
 
    if (ch->mana < 10) {
      send_to_char("You don't have enough mana left!\r\n", ch);
      return;
    }

    ch->mana -= 10;

   /* Build the event... */

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 
                  gsn_english, msg);

    wev = get_wev(WEV_OOCC, WEV_OOCC_QUESTION, mcc,
                 "{yYou ask '@t0{y'{x\r\n",
                  NULL,
                 "{y@a2 asks '@t0{y'{x\r\n");

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

/* OOCC Answer */

void do_answer( CHAR_DATA *ch, char *argument ) {
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;    

    char msg[MAX_STRING_LENGTH];

    if (argument[0] == '\0' ) {
      if (IS_SET(ch->comm,COMM_NOQUESTION)) {
        send_to_char("Q/A channel is now ON.\r\n",ch);
        REMOVE_BIT(ch->comm,COMM_NOQUESTION);
      } else {
        send_to_char("Q/A channel is now OFF.\r\n",ch);
        SET_BIT(ch->comm,COMM_NOQUESTION);
      }

      return;
    }

    if (IS_SET(ch->comm,COMM_QUIET)) {
      send_to_char("You must turn off quiet mode first.\r\n",ch);
      return;
    }
 
    if (IS_SET(ch->comm,COMM_NOCHANNELS)) {
      send_to_char("The gods have revoked your channel priviliges.\r\n",ch);
      return;
    }
 
    REMOVE_BIT(ch->comm,COMM_NOQUESTION);
 
   /* Convert the message... */

    if ( !strip_html(argument, msg, TRUE, MAX_STRING_LENGTH, FONT_YELLOW)) {
      send_to_char("Those words many not be uttered!\r\n", ch);
      return;
    }

   /* Check mana... */
 
    if (ch->mana < 10) {
      send_to_char("You don't have enough mana left!\r\n", ch);
      return;
    }

    ch->mana -= 10;

   /* Build the event... */

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 
                  gsn_english, msg);

    wev = get_wev(WEV_OOCC, WEV_OOCC_ANSWER, mcc,
                 "{yYou answer '@t0{y'{x\r\n",
                  NULL,
                 "{y@a2 answers '@t0{y'{x\r\n");

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

/* OOCC Music */

void do_music( CHAR_DATA *ch, char *argument ) {
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;    
    char msg[MAX_STRING_LENGTH];

    if (ch == NULL) {
          for (ch = char_list; ch != NULL; ch = ch->next)   {
              if (is_name("jukebox", ch->name)) break;
          }
    }
    if (ch==NULL) return;

    if (argument[0] == '\0' ) {
      if (IS_SET(ch->comm,COMM_NOMUSIC)) {
        send_to_char("Music channel is now ON.\r\n",ch);
        REMOVE_BIT(ch->comm,COMM_NOMUSIC);
      } else {
        send_to_char("Music channel is now OFF.\r\n",ch);
        SET_BIT(ch->comm,COMM_NOMUSIC);
      }

      return;
    }

    if (IS_SET(ch->comm,COMM_QUIET)) {
      send_to_char("You must turn off quiet mode first.\r\n",ch);
      return;
    }
 
    if (IS_SET(ch->comm,COMM_NOCHANNELS)
    && !IS_NPC(ch)) {
      send_to_char("The gods have revoked your channel priviliges.\r\n",ch);
      return;
    }
 
    REMOVE_BIT(ch->comm,COMM_NOMUSIC);
 
   /* Convert the message... */

    if ( !strip_html(argument, msg, TRUE, MAX_STRING_LENGTH, FONT_YELLOW)) {
      send_to_char("That chant many not be sung!\r\n", ch);
      return;
    }

   /* Check mana... */
 
    if (ch->mana < 10) {
      send_to_char("You don't have enough mana left!\r\n", ch);
      return;
    }

    ch->mana -= 10;

   /* Build the event... */

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 
                  gsn_english, msg);

    if (IS_NPC(ch)) {
         wev = get_wev(WEV_OOCC, WEV_OOCC_MUSIC, mcc,
                 NULL,
                  NULL,
                 "{y@a2 sounds '@t0{y'{x\r\n");
    } else {
         wev = get_wev(WEV_OOCC, WEV_OOCC_MUSIC, mcc,
                 "{yYou sing '@t0{y'{x\r\n",
                  NULL,
                 "{y@a2 chirps '@t0{y'{x\r\n");
    }
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

/* OOCC ImmTalk */

void do_immtalk( CHAR_DATA *ch, char *argument ) {
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;    
    int rn;
    char msg[MAX_STRING_LENGTH];

    if (!IS_IMMORTAL(ch)
    && !IS_SET(ch->comm, COMM_IMMACCESS)) {
	send_to_char("You got no access to this channel\r\n",ch);
                return;
    }

    if ( argument[0] == '\0' ) {
      if (IS_SET(ch->comm,COMM_NOWIZ)) {
	send_to_char("Immortal channel is now ON\r\n",ch);
	REMOVE_BIT(ch->comm,COMM_NOWIZ);
      } else {
	send_to_char("Immortal channel is now OFF\r\n",ch);
	SET_BIT(ch->comm,COMM_NOWIZ);
      } 
      return;
    }

    REMOVE_BIT(ch->comm,COMM_NOWIZ);

   /* Convert the message... */

    for (rn=0; rn < MAX_PARTNER; rn++) {
           if (partner_array[rn].name) sprintf_to_partner(rn, "mudimmtalk %s %s\r\n", ch->name, prepare_for_transfer(argument));
    }

    if ( !strip_html(argument, msg, TRUE, MAX_STRING_LENGTH, FONT_CYAN)) {
      send_to_char("Those words many not be uttered!\r\n", ch);
      return;
    }

   /* Build the event... */

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, gsn_english, msg);
    wev = get_wev(WEV_OOCC, WEV_OOCC_IMMTALK, mcc,
                 "{c[{y@a2{c]: @t0{x\r\n",
                  NULL,
                 "{c[{y@a2{c]: @t0{x\r\n");

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

/* ICC Say */

void do_say( CHAR_DATA *ch, char *argument ) {
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;    

    char msg[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' ) {
	send_to_char( "Say what?\r\n", ch );
	return;
    }

    if (IS_RAFFECTED(ch->in_room, RAFF_SILENCE)) {
	send_to_char( "You are unable to say a word.\r\n", ch );
	return;
    }

   /* Convert the message... */

    if ( !strip_html(argument, msg, TRUE, MAX_STRING_LENGTH, FONT_GREEN)) {
      send_to_char("Those words many not be uttered!\r\n", ch);
      return;
    }

   /* Build the event... */

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 
                  ch->speak[SPEAK_PUBLIC], msg);

    if (is_affected(ch, gafn_mute)) {

      wev = get_wev(WEV_ICC, WEV_ICC_SAY, mcc,
              "{gYou try to say '@t0{g'{x\r\n",
               NULL,
              "{g@a2 seems to be trying to say something!{x\r\n");
    } else {  

      wev = get_wev(WEV_ICC, WEV_ICC_SAY, mcc,
              "{gYou say '@t0{g'{x\r\n",
               NULL,
              "{g@a2 says '@t1{g'{x\r\n");
    }

   /* Challange... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

   /* Ok, go ahead and say it... */

    room_issue_wev(ch->in_room, wev);

    free_wev(wev);

    mprog_speech_trigger( argument, ch );

    return;
}

void do_telepathy( CHAR_DATA *ch, char *argument ) {
MOB_CMD_CONTEXT *mcc;
WEV *wev;    
CHAR_DATA *victim;
char msg[MAX_STRING_LENGTH];
char arg[MAX_INPUT_LENGTH];

    if (!IS_AFFECTED(ch, AFF_MORF)
    || ch->trans_obj == NULL) {
	send_to_char( "You can't do that at the moment.\r\n", ch );
	return;
    }

    if ( argument[0] == '\0' ) {
	send_to_char( "Telepathy what?\r\n", ch );
	return;
    }

    argument = one_argument(argument, arg);
    victim = get_char_room(ch, arg );
    
   /* Convert the message... */


    if (victim != NULL
    && victim != ch) {
        if ( IS_SET(victim->plr, PLR_AFK)) {
	act("$N is AFK...try again later.", ch,NULL,victim,TO_CHAR);
	return;
        }

        argument = one_argument(argument, arg);

        if ( !strip_html(argument, msg, TRUE, MAX_STRING_LENGTH, FONT_CYAN)) {
            send_to_char("Those words many not be sent!\r\n", ch);
            return;
        }
      
        mcc = get_mcc(ch, ch, victim, NULL, ch->trans_obj, NULL, ch->speak[SPEAK_PRIVATE], msg);
        wev = get_wev(WEV_ICC, WEV_ICC_TELEPATHY, mcc,
              "{cYou think '{C@t0{c'{x\r\n",
              "{c@p2 thinks '{C@t0{c'{x\r\n",
              "");
  
    } else {
        if ( !strip_html(argument, msg, TRUE, MAX_STRING_LENGTH, FONT_CYAN)) {
            send_to_char("Those words many not be sent!\r\n", ch);
            return;
        }

        mcc = get_mcc(ch, ch, NULL, NULL, ch->trans_obj, NULL, ch->speak[SPEAK_PUBLIC], msg);
        wev = get_wev(WEV_ICC, WEV_ICC_TELEPATHY, mcc,
              "{cYou think '{C@t0{c'{x\r\n",
               NULL,
              "{c@p2 thinks '{C@t0{c'{x\r\n");
    }

   /* Challange... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

   /* Ok, go ahead and say it... */

    room_issue_wev(ch->in_room, wev);

    free_wev(wev);
    return;
}


void do_idoltalk( CHAR_DATA *ch, char *argument ) {
MOB_CMD_CONTEXT *mcc;
WEV *wev;    
CHAR_DATA *victim;
OBJ_DATA *idol;
ROOM_INDEX_DATA *old_room;
char msg[MAX_STRING_LENGTH];
char arg[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];


    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg2);

    if (arg[0] == '\0' || arg2[0] == '\0') {
	send_to_char( "Syntax: IDOLTALK <at character> <idol> <text>\r\n", ch );
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
	send_to_char( "Target not found.\r\n", ch );
	return;
    }

    if (victim == ch) {
	send_to_char( "Not to yourself.\r\n", ch );
	return;
    }

    old_room = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, victim->in_room);

    if ((idol = get_obj_here(ch, arg2)) == NULL) {
	send_to_char( "Idol not found.\r\n", ch );
                char_from_room(ch);
                char_to_room(ch, old_room);
	return;
    }

    if (idol->item_type != ITEM_IDOL) {
	send_to_char( "Idol not found.\r\n", ch );
                char_from_room(ch);
                char_to_room(ch, old_room);
	return;
    }

    if ( !strip_html(argument, msg, TRUE, MAX_STRING_LENGTH, FONT_CYAN)) {
            send_to_char("Those words many not be sent!\r\n", ch);
            char_from_room(ch);
            char_to_room(ch, old_room);
            return;
    }

    mcc = get_mcc(ch, ch, victim, NULL, idol, NULL, ch->speak[SPEAK_PUBLIC], msg);
    wev = get_wev(WEV_ICC, WEV_ICC_TELEPATHY, mcc,
              "{c[{C@p2{c] '{C@t0{c'{x\r\n",
              "{c@p2 thinks '{C@t0{c'{x\r\n",
              "{c@p2 thinks '{C@t0{c'{x\r\n");

    if (!room_issue_wev_challange(victim->in_room, wev)) {
      free_wev(wev);
      char_from_room(ch);
      char_to_room(ch, old_room);
      return;
    }

    room_issue_wev(victim->in_room, wev);
    free_wev(wev);
    char_from_room(ch);
    char_to_room(ch, old_room);
    return;
}


/* ICC Shout */

void do_shout( CHAR_DATA *ch, char *argument ) {
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;    

    char msg[MAX_STRING_LENGTH];

    if ( IS_SET(ch->comm, COMM_NOSHOUT) ) {
	send_to_char( "You can't shout.\r\n", ch );
	return;
    }

    if ( IS_SET(ch->comm, COMM_DEAF)) {
	send_to_char( "Deaf people can't shout.\r\n",ch);
        return;
    }

    if ( argument[0] == '\0' ) {
	send_to_char( "Shout what?\r\n", ch );
	return;
    }

   /* Convert the message... */

    if ( !strip_html(argument, msg, TRUE, MAX_STRING_LENGTH, FONT_GREEN)) {
      send_to_char("Such profanity may not be shouted!\r\n", ch);
      return;
    }

   /* Check move... */
 
    if (ch->move < 5) {
      send_to_char("You don't have enough movement points left!\r\n", ch);
      return;
    }

    ch->move -= 5;

   /* Build the event... */

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 
                  ch->speak[SPEAK_PUBLIC], msg);

    if (is_affected(ch, gafn_mute)) {

      wev = get_wev(WEV_ICC, WEV_ICC_SHOUT, mcc,
              "{GYou try to shout '@t0{G'{x\r\n",
               NULL,
              "{G@a2 makes a strange, unnatural noise!{x\r\n");
    } else {  

      wev = get_wev(WEV_ICC, WEV_ICC_SHOUT, mcc,
              "{GYou shout '@t0{G'{x\r\n",
               NULL,
              "{G@a2 shouts '@t1{G'{x\r\n");
    }

   /* Challange... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

   /* Ok, go ahead and say it... */

    area_issue_wev(ch->in_room, wev, WEV_SCOPE_ADJACENT);

    free_wev(wev);

    return;
}

/* ICC Yell */

void do_yell( CHAR_DATA *ch, char *argument ) {
//    DESCRIPTOR_DATA *d;

    MOB_CMD_CONTEXT *mcc;
    WEV *wev;    

    char msg[MAX_STRING_LENGTH];

    if ( IS_SET(ch->comm, COMM_NOSHOUT) ) {
        send_to_char( "You can't yell.\r\n", ch );
        return;
    }
 
    if ( argument[0] == '\0' ) {
	send_to_char( "Yell what?\r\n", ch );
	return;
    }

    if (IS_RAFFECTED(ch->in_room, RAFF_SILENCE)) {
	send_to_char( "You are unable to say a word.\r\n", ch );
	return;
    }

   /* Convert the message... */

    if ( !strip_html(argument, msg, TRUE, MAX_STRING_LENGTH, FONT_YELLOW)) {
      send_to_char("Such profanity may not be yelled!\r\n", ch);
      return;
    }

   /* Check move... */
 
    if (ch->move < 15) {
      send_to_char("You don't have enough movement points left!\r\n", ch);
      return;
    }

    ch->move -= 15;

   /* Build the event... */

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 
                  ch->speak[SPEAK_PUBLIC], msg);

    if (is_affected(ch, gafn_mute)) {

      wev = get_wev(WEV_ICC, WEV_ICC_YELL, mcc,
              "{YYou try to yell '@t0{Y'{x\r\n",
               NULL,
              "{Y@a2 makes a strange, unnatural noise!{x\r\n");
    } else {  

      wev = get_wev(WEV_ICC, WEV_ICC_YELL, mcc,
              "{YYou yell '@t0{Y'{x\r\n",
               NULL,
              "{Y@a2 yells '@t1{Y'{x\r\n");
    }

   /* Challange... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

   /* Ok, go ahead and say it... */

    area_issue_wev(ch->in_room, wev, WEV_SCOPE_SUBAREA);

    free_wev(wev);

    return;
}

/* ICC Scream */

void do_scream( CHAR_DATA *ch, char *argument ) {
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;    

    char msg[MAX_STRING_LENGTH];

    if ( IS_SET(ch->comm, COMM_NOSHOUT) ) {
        send_to_char( "You can't scream.\r\n", ch );
        return;
    }

   /* Convert the message... */

    if ( !strip_html(argument, msg, TRUE, MAX_STRING_LENGTH, FONT_RED)) {
      send_to_char("To scream such words would incur terrible wrath!\r\n", ch);
      return;
    }

   /* Check move... */
 
    if (ch->move < 20) {
      send_to_char("You don't have enough movement points left!\r\n", ch);
      return;
    }

    ch->move -= 20;

    if (IS_RAFFECTED(ch->in_room, RAFF_SILENCE)) {
	send_to_char( "You are unable to say a word.\r\n", ch );
	return;
    }

   /* Build the event... */

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 
                  ch->speak[SPEAK_PUBLIC], msg);

    if (is_affected(ch, gafn_mute)) {

      if ( argument[0] == '\0' ) {
        wev = get_wev(WEV_ICC, WEV_ICC_SCREAM, mcc,
              "{RYou try to scream!{x\r\n",
               NULL,
              "{Y@a2 makes a strange, unnatural noise!{x\r\n");
      } else {
        wev = get_wev(WEV_ICC, WEV_ICC_SCREAM, mcc,
              "{RYou try to scream '@t0{R'{x\r\n",
               NULL,
              "{Y@a2 makes a strange, unnatural noise!{x\r\n");
      }
    } else {  

      if ( argument[0] == '\0' ) {
        wev = get_wev(WEV_ICC, WEV_ICC_SCREAM, mcc,
              "{RYou scream in terror!{x\r\n",
               NULL,
              "{R@a2 screams horribly!{x\r\n");
      } else {
        wev = get_wev(WEV_ICC, WEV_ICC_SCREAM, mcc,
              "{RYou scream '@t0{R'{x\r\n",
               NULL,
              "{R@a2 screams '@t1{R'{x\r\n");
      }
    }

   /* Challange... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

   /* Ok, go ahead and say it... */

    area_issue_wev(ch->in_room, wev, WEV_SCOPE_SUBAREA_PLUS);

    free_wev(wev);

    return;
}

/* OOCC/ICC Tell */

void do_tell( CHAR_DATA *ch, char *argument ) {
char arg[MAX_INPUT_LENGTH];
char *message;
CHAR_DATA *victim;
MOB_CMD_CONTEXT *mcc;
WEV *wev;    
bool ooc;
char msg[MAX_STRING_LENGTH];

    if ( IS_SET(ch->comm, COMM_NOTELL) ) {
	send_to_char( "Your message didn't get through.\r\n", ch );
	return;
    }

    if ( IS_SET(ch->comm, COMM_QUIET) ) {
	send_to_char( "You must turn off quiet mode first.\r\n", ch);
	return;
    }

    message = one_argument( argument, arg );

    if ( arg[0] == '\0' || message[0] == '\0' ) {
	send_to_char( "Tell whom what?\r\n", ch );
	return;
    }

    victim = get_char_world( ch, arg );

    if ( victim == NULL ) {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

   /* Telling a mob drives the conversation engine... */

    if (IS_NPC(victim)) {
         if (IS_RAFFECTED(ch->in_room, RAFF_SILENCE)) {
	send_to_char( "You are unable to say a word.\r\n", ch );
	return;
         }
         do_converse(ch, argument);
      return;
    }

    if ( victim->desc == NULL 
      && !IS_NPC(victim)) {
	act("$N seems to have misplaced $S link...try again later.",
	    ch,NULL,victim,TO_CHAR);
	return;
    }
	
    if ( !IS_IMMORTAL(ch) 
      && !IS_AWAKE(victim) ) {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }
  
    if ( IS_SET(victim->comm,COMM_QUIET) 
      && !IS_IMMORTAL(ch)) {
	act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
  	return;
    }

    if ( IS_SET(victim->plr, PLR_AFK)) {
	act("$N is AFK...try again later.", ch,NULL,victim,TO_CHAR);
	return;
    }

   /* Convert the message... */

    if ( !strip_html(message, msg, TRUE, MAX_STRING_LENGTH, FONT_WHITE)) {
      send_to_char("That secret must never be told!\r\n", ch);
      return;
    }

   /* Build the event... */

    ooc = FALSE;

    if (ch->in_room != victim->in_room) {
      ooc = TRUE;

     /* Check mana... */
 
      if (check_ignore(victim, ch->name)) {
          sprintf_to_char(ch, "%s is ignoring you!\r\n", victim->name);
          return;
      }

      if (ch->mana < 5) {
        send_to_char("You don't have enough mana left!\r\n", ch);
        return;
      } 

      ch->mana -= 5;
    }
 
    if (ooc) {
      mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, gsn_english, message);

      if (IS_AFFECTED(victim, AFF_POLY)) {
          wev = get_wev(WEV_OOCC, WEV_OOCC_TELL, mcc,
              "{wYou tell @v1 '@t0'.{x\r\n",
              "{C@a2{w tells you '@t0'.{x\r\n",
               NULL);
      } else {
          wev = get_wev(WEV_OOCC, WEV_OOCC_TELL, mcc,
              "{wYou tell @v2 '@t0'.{x\r\n",
              "{C@a2{w tells you '@t0'.{x\r\n",
               NULL);
      }

    } else {
      mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, ch->speak[SPEAK_PRIVATE], message);

      if (is_affected(ch, gafn_mute)) {
          if (IS_AFFECTED(victim, AFF_POLY)) {
              wev = get_wev(WEV_ICC, WEV_ICC_TELL, mcc,
              "{wYou try to whisper to @v1 '@t0'.{x\r\n",
              "{C@a2{w is trying to whisper something to you!{x\r\n",
              "{C@a2{w is trying to whisper to @v1!{x\r\n");
          } else {
              wev = get_wev(WEV_ICC, WEV_ICC_TELL, mcc,
              "{wYou try to whisper to @v2 '@t0'.{x\r\n",
              "{C@a2{w is trying to whisper something to you!{x\r\n",
              "{C@a2{w is trying to whisper to @v2!{x\r\n");
          }
      } else {  
          if (IS_AFFECTED(victim, AFF_POLY)) {
              wev = get_wev(WEV_ICC, WEV_ICC_TELL, mcc,
              "{wYou whisper to @v1 '@t0'.{x\r\n",
              "{C@a2{w whispers to you '@t1'.{x\r\n",
              "{C@a2{w whispers something to @v1.{x\r\n");
          } else {
              wev = get_wev(WEV_ICC, WEV_ICC_TELL, mcc,
              "{wYou whisper to @v2 '@t0'.{x\r\n",
              "{C@a2{w whispers to you '@t1'.{x\r\n",
              "{C@a2{w whispers something to @v2.{x\r\n");
          }
      }
    }

   /* Challange... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

   /* Ok, send the wevs directly... */

    if (ooc) {
      mob_handle_wev(ch, wev, "OOC");
      mob_handle_wev(victim, wev, "OOC");
    } else {
      room_issue_wev(ch->in_room, wev);
    }

    free_wev(wev);

    victim->reply = ch;

    return;
}

/* OOCC/ICC Reply */

void do_reply( CHAR_DATA *ch, char *argument ) {
    CHAR_DATA *victim;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;    
    bool ooc;

    char msg[MAX_STRING_LENGTH];

    if ( IS_SET(ch->comm, COMM_NOTELL) ) {
	send_to_char( "Your message did not get through.\r\n", ch );
	return;
    }

    victim = ch->reply;
 
    if ( victim == NULL ) {
	send_to_char( "You have no one to reply to!\r\n", ch );
	return;
    }

/* This looks like crash city if the victim has logged off. I'll lay
   odd this pointer won't have been cleaned up... */

    if ( victim->desc == NULL 
      && !IS_NPC(victim)) {
        act("$N seems to have misplaced $S link...try again later.", ch, NULL,victim,TO_CHAR);
        return;
    }

    if ( !IS_IMMORTAL(ch) 
      && !IS_AWAKE(victim) ) {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }

    if ( IS_SET(victim->comm,COMM_QUIET) 
      && !IS_IMMORTAL(ch)) {
        act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
        return;
    }

   /* Convert the message... */

    if ( !strip_html(argument, msg, TRUE, MAX_STRING_LENGTH, FONT_WHITE)) {
      send_to_char("That secret must never be told!\r\n", ch);
      return;
    }

   /* Build the event... */

    ooc = FALSE;

    if (ch->in_room != victim->in_room) {
      ooc = TRUE;

      if (ch->mana < 5) {
        send_to_char("You don't have enough mana left!\r\n", ch);
        return;
      } 

      ch->mana -= 5;
    }

    if (ooc) {
      mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, gsn_english, msg);

      wev = get_wev(WEV_OOCC, WEV_OOCC_TELL, mcc,
              "{wYou reply to @v2 '@t0{w'{x\r\n",
              "{C@a2{w reply '@t0{w'{x\r\n",
              NULL);
    } else {
      mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, ch->speak[SPEAK_PRIVATE], msg);

      if (is_affected(ch, gafn_mute)) {

        wev = get_wev(WEV_ICC, WEV_ICC_TELL, mcc,
              "{wYou try to whisper to @v2 '@t0{w'{x\r\n",
              "{C@a2{w is trying to whisper something to you!{x\r\n",
              "{C@a2{w is trying to whisper to @v2!{x\r\n");
      } else {  

        wev = get_wev(WEV_ICC, WEV_ICC_TELL, mcc,
              "{wYou whisper to @v2 '@t0{w'{x\r\n",
              "{C@a2{w whispers to you '@t1{w'{x\r\n",
              "{C@a2{w whispers something to @v2{x\r\n");
      }
    }

   /* Challange... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

   /* Ok, send the wevs directly... */

    if (ooc) {
      mob_handle_wev(ch, wev, "OOC");
      mob_handle_wev(victim, wev, "OOC");
    } else {
      room_issue_wev(ch->in_room, wev);
    }

    free_wev(wev);
    victim->reply = ch;
    return;
}


/* OOCC Group Tell */

void do_gtell( CHAR_DATA *ch, char *argument ) {
    CHAR_DATA *gch;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;   
    char msg[MAX_STRING_LENGTH];
 
    if ( argument[0] == '\0' ) {
	send_to_char( "Tell your group what?\r\n", ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_NOTELL ) ) {
	send_to_char( "Your message did not get through!\r\n", ch );
	return;
    }

   /* Check mana... */
 
    if (ch->mana < 5) {
      send_to_char("You don't have enough mana left!\r\n", ch);
      return;
    }

    ch->mana -= 5;

   /* Convert the message... */

    if ( !strip_html(argument, msg, TRUE, MAX_STRING_LENGTH, FONT_CYAN)) {
      send_to_char("Those words may not be uttered!\r\n", ch);
      return;
    }

   /* Build the event... */

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 
                  gsn_english, msg);

    wev = get_wev(WEV_OOCC, WEV_OOCC_GTELL, mcc,
            "{cYou tell your group '@t0{c'{x\r\n",
             NULL,
            "{c@a2 tells the group '@t0{c'!{x\r\n");

   /* Challange... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

   /* Ok, direct send to each member... */

    gch = char_list;

    while (gch != NULL) {

      if (is_same_group(gch, ch)) {
        mob_handle_wev(gch, wev, "OOC");
      }

      gch = gch->next;
    }

   /* Free the wev... */

    free_wev(wev);

    return;
}

/* ICC Emote */

void do_emote( CHAR_DATA *ch, char *argument )
{

    char msg[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch)
      && IS_SET(ch->comm, COMM_NOEMOTE) ) {
	send_to_char( "You can't show your emotions.\r\n", ch );
	return;
    }

    if ( argument[0] == '\0' ) {
	send_to_char( "Emote what?\r\n", ch );
	return;
    }

   /* Convert the activity... */

    if ( !strip_html(argument, msg, TRUE, MAX_STRING_LENGTH, FONT_CLEAR)) {
      send_to_char("That would attract the disfavour of the gods!\r\n", ch);
      return;
    }

   /* Go do it... */ 

    act( "$n $T", ch, NULL, msg, TO_ROOM );
    act( "$n $T", ch, NULL, msg, TO_CHAR );

    return;
}


/*
 * Structure for a Quote
 */

struct quote_type
{
    char * 	text;
    char * 	by;
};

/* 
 * The Quotes - insert yours in, and increase MAX_QUOTES in merc.h 
 */

const struct quote_type quote_table [MAX_QUOTES] =
{
    { "Cogito Ergo Sum", 
                                                           "Descartes" },
    { "Your lucky color has faded.", 
                                                             "Unknown" },
    { "Don't mess with Dragons, for thou art Crunchy and go well with milk.",
                                                      "Unknown Source" },
    { "... and furthermore ... I don't like your trousers.", 
                                                             "Unknown" }, 
    { "They're only kobolds!", 
                                 "Dragon Magazine 'Famous Last Words'" },
    { "I wouldn't want to be around when a mage says 'Oops'", 
                                                             "Unknown" }, 
    { "No matter how subtle the sorcerer,"
      " a knife in the back will seriously cramp his style.",
                                                             "Unknown" },
    { "...and when they returned it was gone.",
                                                             "Unknown" },
    { "And there dwelt upon the peak a being of most unimaginable foulness.",
                                                             "Unknown" },
    { "Arrrghh! No! Help!",
                                                             "Unknown" },
// ------------------------------------------------------------------------//
    { "I drink, therefore I am.",
                                                         "Traditional" },
    { "I'm drunk, therefore I was?",
                                                         "Traditional" },
    { "I COULD REALLY MURDER A GOOD CURRY.",
                                                     "Terry Pratchett" }, 
    { "Oook!",
                                                     "Terry Pratchett" }, 
    { "There's a thin line between genius and madness...",
                                                     "Terry Pratchett" }, 
    { "And within lay a book of darkness, clad in human skin.",
                                                             "Unknown" }, 
    { "I wonder what this does? *click* Uh-Oh!",
                                                         "Traditional" }, 
    { "Oh, is that really a piece of fairy cake?",
                                                       "Douglas Adams" }, 
    { "That is not dead which can eternal lie\r\n"
      "And with strange aeons, even death may die.",
                                                    "Howard Lovecraft" }

};


/* 
 * The Routine
 */

void do_quote( CHAR_DATA *ch ) {
    int quote = 0;
    char buf[MAX_STRING_LENGTH];

    quote = number_range( 0, MAX_QUOTES-1);

    if (quote_table[quote].text == NULL)    {
 	sprintf(buf,"DO_QUOTE: Null Quote %d",quote);
	log_string(buf);
	return;
    }

    sprintf_to_char(ch, "\r\n{W%s\r\n{Y - %s{x\r\n", quote_table[quote].text, quote_table[quote].by);
    return;
}


void do_bug( CHAR_DATA *ch, char *argument ) {
    append_file( ch, BUG_FILE, argument );
    send_to_char( "Bug logged.\r\n", ch );
	notify_message (NULL, WIZNET_BUG, TO_IMM, argument);
    return;
}


void do_typo( CHAR_DATA *ch, char *argument ) {
    append_file( ch, TYPO_FILE, argument );
    send_to_char( "Typo logged.\r\n", ch );
    return;
}


void do_rent( CHAR_DATA *ch, char *argument ) {
    send_to_char( "There is no rent here.  Just save and quit.\r\n", ch );
    send_to_char( "If you are looking to rent a room, not quit, try LEASE.\r\n",ch);
    return;
}


void do_qui( CHAR_DATA *ch, char *argument ) {
    send_to_char( "If you want to QUIT, you have to spell it out.\r\n", ch );
    return;
}



void do_quit( CHAR_DATA *ch, char *argument ) {
    DESCRIPTOR_DATA *d, *d_next;
    char old_name[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) ) return;

    if ( ch->position == POS_FIGHTING ) {
	send_to_char( "No way! You are fighting.\r\n", ch );
	return;
    }

    if ( ch->position  < POS_STUNNED  ) {
	send_to_char( "You're not DEAD yet.\r\n", ch );
	return;
    }

    if ( auction->item != NULL && ((ch == auction->buyer) || (ch == auction->seller)) )    {
        send_to_char ("Wait till you have sold/bought the item on auction.\r\n",ch);
        return;
    }

    if (IS_AFFECTED(ch, AFF_MORF)
    || is_affected(ch, gafn_morf)
    || ch->trans_obj != NULL) { 
         affect_strip_afn( ch, gafn_morf);
         undo_morf(ch, ch->trans_obj);
         REMOVE_BIT( ch->affected_by, AFF_MORF );
    }
    undo_mask(ch, TRUE);

    if ( ch->carrying != NULL ) drop_artifact(ch, ch->carrying);
    if ( ch->ooz != NULL ) drop_artifact(ch, ch->ooz);

   /* Zeran - notify message */

    notify_message (ch, NOTIFY_QUITGAME, TO_ALL, NULL);

    act( "$n has left the game.", ch, NULL, NULL, TO_ROOM );

    sprintf( log_buf, "%s has quit.", ch->name );
    log_string( log_buf );

    do_quote(ch); 

    ch->quitting = TRUE;

   /* After extract_char the ch is no longer valid! */

    save_char_obj( ch );

    if (IS_MUD(ch)) {
        int rn = am_i_partner(ch);
        if (rn >= 0) partner_array[rn].status = PARTNER_DOWN;
    }

    d = ch->desc;

    strcpy (old_name, ch->name);

    extract_char( ch, TRUE );

    if ( d != NULL ) {
      d->character = NULL;
      close_socket(d, TRUE);
    }

    for ( d = descriptor_list; d != NULL; d = d_next) {

      CHAR_DATA *tch;
			
      d_next = d->next;

      tch = (d->original ? d->original : d->character);

      if ( tch 
        && !str_cmp(old_name, tch->name) 
        && tch != ch) {                     // !! ch ?? //
        extract_char (tch, TRUE); 
        d->character = NULL;
        close_socket (d, TRUE);
      }
    } 

    return;
}



void do_save( CHAR_DATA *ch, char *argument) {
 
   if ( IS_NPC(ch)) return;

    save_char_obj( ch );
    send_to_char("Character saved - remember we have autosave.\r\n", ch);
    return;
}

void do_follow( CHAR_DATA *ch, char *argument ) {
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )    {
	send_to_char( "Follow whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )    {
	act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if ( victim == ch )    {
	if (ch->master == NULL)	{
	    send_to_char( "You already follow yourself.\r\n", ch );
	    return;
	}
	stop_follower(ch);
	return;
    }

    if (victim->master == ch) stop_follower(victim);

    if ( !IS_NPC(victim) 
      &&  IS_SET(victim->plr, PLR_NOFOLLOW) 
      && !IS_HERO(ch)) {
	act("$N doesn't seem to want any followers.\r\n", ch,NULL,victim, TO_CHAR);
        return;
    }

    REMOVE_BIT(ch->plr, PLR_NOFOLLOW);
    
    if ( ch->master != NULL ) stop_follower( ch );
    add_follower( ch, victim );
    return;
}


void add_follower( CHAR_DATA *ch, CHAR_DATA *master ) {
    if ( ch->master != NULL )    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch )) act( "$n now follows you.", ch, NULL, master, TO_VICT );
    act( "You now follow $N.",  ch, NULL, master, TO_CHAR );
    return;
}


void stop_follower( CHAR_DATA *ch ) {
    if ( ch->master == NULL ) return;
    
    if ( IS_AFFECTED(ch, AFF_CHARM) )    {
	REMOVE_BIT( ch->affected_by, AFF_CHARM );
	affect_strip_afn( ch, gafn_charm );
    }

    if ( can_see( ch->master, ch ) && ch->in_room != NULL)   {
	act( "$n stops following you.",     ch, NULL, ch->master, TO_VICT    );
    	act( "You stop following $N.",      ch, NULL, ch->master, TO_CHAR    );
    }

   if (ch->ridden==TRUE) {
       ch->master->pcdata->mounted=FALSE;
       ch->ridden=FALSE;
    }

    if (ch->master->pet == ch) ch->master->pet = NULL;

    ch->master = NULL;
    ch->leader = NULL;
    return;
}

/* nukes charmed monsters and pets */
void nuke_pets( CHAR_DATA *ch ) {    
    CHAR_DATA *pet;

    if ((pet = ch->pet) != NULL)   {
    	stop_follower(pet);
    	if (pet->in_room != NULL)
    	    act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);
    	extract_char(pet,TRUE);
    }
    ch->pet = NULL;
    return;
}


void die_follower( CHAR_DATA *ch ) {
    CHAR_DATA *fch;

    if ( ch->master != NULL )    {
    	if (ch->master->pet == ch)
    	    ch->master->pet = NULL;
	stop_follower( ch );
    }

    ch->leader = NULL;

    for ( fch = char_list; fch != NULL; fch = fch->next )    {
	if ( fch->master == ch ) {
                    if (fch->position == POS_STUNNED ) {
                           send_to_char("Your you drop your victim to the floor.\r\n", ch);
                           send_to_char( "You hit the floor very hard.\r\n", fch );
                           act("$n drops $n to the ground.",ch,fch,NULL,TO_ROOM);
                           ch->carry_weight -= 150;
                     }                     
                     stop_follower( fch );
                }
	if ( fch->leader == ch )  fch->leader = fch;
    }

    return;
}



void do_order( CHAR_DATA *ch, char *argument ) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    argument = one_argument( argument, arg );
    one_argument(argument,arg2);

    if (!str_cmp(arg2,"delete"))  {
        send_to_char("That will NOT be done.\r\n",ch);
        return;
    }

    if ( arg[0] == '\0' || argument[0] == '\0' )    {
	send_to_char( "Order whom to do what?\r\n", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ))    {
	send_to_char( "You feel like taking, not giving, orders.\r\n", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )    {
	fAll   = TRUE;
	victim = NULL;
    }    else    {
	fAll   = FALSE;
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )	{
	    send_to_char( "They aren't here.\r\n", ch );
	    return;
	}

	if ( victim == ch )	{
	    send_to_char( "Aye aye, right away!\r\n", ch );
	    return;
	}

                if (!str_cmp(race_array[victim->race].name,"machine")
                || IS_SET(victim->form, FORM_MACHINE)) {
	    send_to_char( "Order a machine?!\r\n", ch );
	    return;
	}

	if ( !IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch )	{
	    send_to_char( "Do it yourself!\r\n", ch );
	    return;
	}
    }

    found = FALSE;
    for ( och = ch->in_room->people; och != NULL; och = och_next )    {
	och_next = och->next_in_room;

	if ( IS_AFFECTED(och, AFF_CHARM)
	&&   och->master == ch
	&& ( fAll || och == victim ) )
	{
		if (och->wait == 0)
			{
	   		found = TRUE;
	   		sprintf( buf, "$n orders you to '%s'.", argument );
	   		act( buf, ch, NULL, och, TO_VICT );
                                                log_string(argument);
	   		interpret( och, argument );
			}
		else
			{
			char buf[128];
			sprintf (buf, "%s is too busy right now.\r\n", 
					(IS_NPC(och) ? och->short_descr : och->name) );
			send_to_char (buf, ch); 
			}
			
	}
    }

    if ( found ) send_to_char( "Ok.\r\n", ch );
    else send_to_char( "You have no followers here.\r\n", ch );
    return;
}


void do_group( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = (ch->leader != NULL) ? ch->leader : ch;
	sprintf_to_char(ch, "%s's group:\r\n", PERS(leader, ch) );

	for ( gch = char_list; gch != NULL; gch = gch->next )	{
	    if ( is_same_group( gch, ch ) )	    {
		sprintf_to_char(ch, "[%2d %s] %-20s %5d/%5d hp %5d/%5d mana %5d/%5d mv\r\n",
                                gch->level,
                                (IS_NPC(gch) ? "Mob" : ( ch->profs == NULL ? class_table[gch->pc_class].who_name : "Pro" )),
                                capitalize( PERSMASK(gch, ch)),
                                gch->hit,
                                gch->max_hit,
                                gch->mana,
                                gch->max_mana,
                                gch->move,
                                gch->max_move);
	    }
	}
	return;
    }

    victim = get_char_room( ch, arg );

    if ( victim == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )    {
	send_to_char( "But you are following someone else!\r\n", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )    {
	act( "$N isn't following you.", ch, NULL, victim, TO_CHAR );
	return;
    }
    
    if (IS_AFFECTED(victim,AFF_CHARM))    {
        send_to_char("You can't remove charmed mobs from your group.\r\n",ch);
        return;
    }
    
    if (IS_AFFECTED(ch,AFF_CHARM))    {
    	act("You like your master too much to leave $m!",ch,NULL,victim,TO_VICT);
    	return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )    {
	victim->leader = NULL;
	act( "$n removes $N from $s group.",   ch, NULL, victim, TO_NOTVICT );
	act( "$n removes you from $s group.",  ch, NULL, victim, TO_VICT    );
	act( "You remove $N from your group.", ch, NULL, victim, TO_CHAR    );
	return;
    }
/*
    if ( ch->level - victim->level < -8
    ||   ch->level - victim->level >  8 )    {
	act( "$N cannot join $n's group.",     ch, NULL, victim, TO_NOTVICT );
	act( "You cannot join $n's group.",    ch, NULL, victim, TO_VICT    );
	act( "$N cannot join your group.",     ch, NULL, victim, TO_CHAR    );
	return;
    }
*/

    victim->leader = ch;
    act( "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT );
    act( "You join $n's group.", ch, NULL, victim, TO_VICT    );
    act( "$N joins your group.", ch, NULL, victim, TO_CHAR    );
    return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument ){
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members, amount, share, extra;
    int type = 0;
   
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' )    {
	send_to_char( "Split how much?\r\n", ch );
	return;
    }
    amount = atoi( arg );

    if ( amount < 0 )    {
	send_to_char( "Your group wouldn't like that.\r\n", ch );
	return;
    }

    if ( amount == 0 )    {
	send_to_char( "You hand out zero coins, but no one notices.\r\n", ch );
	return;
    }

    if (arg2[0] !='\0') type = flag_value(currency_accept, arg2);
 

    if ( ch->gold[type] < amount )    {
	send_to_char( "You don't have that much gold.\r\n", ch );
	return;
    }
  
    members = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )    {
	if ( is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM))  members++;
    }

    if ( members < 2 )    {
	send_to_char( "Just keep it all.\r\n", ch );
	return;
    }
	    
    share = amount / members;
    extra = amount % members;

    if ( share == 0 )    {
	send_to_char( "Don't even bother, cheapskate.\r\n", ch );
	return;
    }

    ch->gold[type] -= amount;
    ch->gold[type] += share + extra;

    sprintf_to_char(ch, "You split %d %s.  Your share is %d.\r\n", amount, flag_string(currency_type, type), share + extra );
    sprintf( buf, "$n splits %d %s.  Your share is %d.",	amount, flag_string(currency_type, type), share );

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )    {
	if ( gch != ch && is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM)) {
	    act( buf, ch, NULL, gch, TO_VICT );
	    gch->gold[type] += share;
	}
    }

    return;
}



/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 *
 * Extended to cover pets and followers followers
 *
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch ) {
    int max;
    max = 0;

    while ( ach->leader != NULL
         && max < 10 ) {
      ach = ach->leader;
      max++;
    }

    max = 0;

    while ( bch->leader != NULL
         && max < 10 ) {
      bch = bch->leader;
      max++;
    }

    return ach == bch;
}

/*
 * Colour setting and unsetting, way cool, Lope Oct '94
 */
void do_colour( CHAR_DATA *ch, char *argument ) {

    if( !IS_SET( ch->plr, PLR_COLOUR ) ) {
      SET_BIT( ch->plr, PLR_COLOUR );
      send_to_char( "{bC{ro{yl{co{mu{gr{x is now {rON{x, Way Cool!\r\n", ch );
    } else {
      REMOVE_BIT( ch->plr, PLR_COLOUR );
      send_to_char_bw( "Colour is now OFF, <sigh>\r\n", ch );
    }

    return;
}

/*
 * HTML setting and unsetting, Mik Nov '99
 */
void do_imp_html( CHAR_DATA *ch, char *argument ) {

    if( !IS_SET( ch->plr, PLR_IMP_HTML ) ) {
      SET_BIT( ch->plr, PLR_IMP_HTML );
      send_to_char( "<B>IMP HTML Format now active...</B>\r\n", ch );
    } else {
      REMOVE_BIT( ch->plr, PLR_IMP_HTML );
      send_to_char( "IMP HTML support inactive...\r\n", ch );
    }

    return;
}

void do_cursor( CHAR_DATA *ch, char *argument )
{
    char        arg[ MAX_STRING_LENGTH ];
    argument = one_argument( argument, arg );

    if( !*arg )
    {
        if( !IS_SET( ch->plr, PLR_CURSOR ) )
        {
            SET_BIT( ch->plr, PLR_CURSOR );
	    send_to_char( VT_CLS, ch );
	    gotoxy(ch, 0,0);
            send_to_char( "Cursor Control is Now On!\r\n", ch );
	    send_to_char( "The following numbers should run down the screen, moving to the right.", ch );
	    gotoxy(ch, 4,1);
	    send_to_char( "1",ch);
	    gotoxy(ch, 5,3);
	    send_to_char( "2",ch);
	    gotoxy(ch, 6,5);
	    send_to_char( "3",ch);
	    gotoxy(ch, 7,7);
	    send_to_char( "4\r\n",ch);
	    send_to_char( "If the numbers 1-4 run together, please turn cursor off, your terminal\r\n doesn't support it.", ch);
        }
        else
        {
            send_to_char_bw( "Cursor control is now off\r\n", ch );
            REMOVE_BIT( ch->plr, PLR_CURSOR );
        }
        return;
    }
    else
    {
	send_to_char("Just type cursor by itself.",ch);
    }
    return;
}



void do_alias (CHAR_DATA *ch, char *argument)	{
	struct alias_data *tmp;
	char alias_name[MAX_INPUT_LENGTH];
	char *alias_string	;
	char d_alias[MAX_INPUT_LENGTH];
	int counter, number;
	bool got_one=FALSE;
	int last_free=-1;

	/* NO NPC's!!!!!!!!! */	
	if (IS_NPC(ch))
		return;
	
	smash_tilde(argument);
	
	/* if no arguments, just print out current aliases */	
	if (argument == NULL || argument[0] == '\0')
		{
		for (counter = 0 ; counter < MAX_ALIAS ; counter++)
			{
			tmp = ch->pcdata->aliases[counter];
			if (tmp != NULL)	
				{
				sprintf_to_char(ch, "Alias %d:  (%s) = (%s)\r\n", counter, ch->pcdata->aliases[counter]->name, ch->pcdata->aliases[counter]->command_string);
				got_one=TRUE;
				}
			}
		if (!got_one) send_to_char ("You have no aliases defined.\r\n",ch);
		return;
		}

	/* get arguments */	
	/* if alias_string is "delete", interpret as command to delete 
		a current alias identified by argument in alias_name */

	alias_string = one_argument (argument, alias_name);
	
	if (alias_string == NULL || alias_string[0] == '\0')		{
		send_to_char ("Syntax:  alias 'name' 'command string'\r\n",ch);
		return;
	}	
	
	if (!str_cmp(alias_name, "delete"))	{
		if (alias_string[0] == '\0')	{
			send_to_char ("Syntax for alias deletion:  alias delete <number>\r\n",ch);
			return;
			}
		one_argument (alias_string, d_alias);
		/* delete designated alias if exits*/ 
		for (counter=0 ; counter < MAX_ALIAS ; counter++)
			{
			tmp = ch->pcdata->aliases[counter]	;
			if (tmp && !str_cmp (tmp->name, d_alias))
				{
				got_one = TRUE;
				break;
				}	
			}	
		if (!got_one)
			{
			send_to_char ("No alias found with that name.\r\n",ch);
			return;
			}
		free_string (tmp->name);
		free_string (tmp->command_string);
		ch->pcdata->aliases[counter] = NULL;
		free (tmp);	
		send_to_char ("Alias deleted.\r\n",ch);
		got_one=FALSE;
		/* check if all deleted, set has_alias to false */
		for (number = 0; number < MAX_ALIAS ; number++)
			{
			if (ch->pcdata->aliases[number] != NULL)
				{
				got_one = TRUE;
				break;
				}
			}
		if (!got_one)
			ch->pcdata->has_alias = FALSE;
		return;
		}		
	
	/*check for rediculous size of alias*/
	if (strlen(alias_string) > MAX_ALIAS_LENGTH)
		{
		send_to_char ("Alias too long, limit is 50 characters.\r\n",ch);
		return;
		}

	/* find first open alias in array and check for duplication of name */
	for (counter = MAX_ALIAS-1 ; counter >=0 ; counter--)
		{
		if (ch->pcdata->aliases[counter] == NULL)
			last_free = counter;
		else if (!str_cmp (ch->pcdata->aliases[counter]->name, alias_name))
			{
			send_to_char ("An alias with that name is already defined.\r\n",ch);
			return;
			}
		}

	/* if no free aliases, tell player to delete an alias first */
	if (last_free == -1)
		{
		send_to_char ("All your alias slots are in use, please delete an alias first.\r\n",ch);
		return;
		}

	/* malloc alias and assign its values */
	tmp = (struct alias_data *) malloc (sizeof (struct alias_data));
	ch->pcdata->aliases[last_free] = tmp;
	tmp->name 			= strdup (alias_name);
	tmp->command_string = strdup (alias_string);
	send_to_char ("Alias set.\r\n",ch);
	ch->pcdata->has_alias = TRUE;
	return;
	}

void do_unalias (CHAR_DATA *ch, char *argument) {
	
    char final[MAX_INPUT_LENGTH];
	
    if (argument == NULL || argument[0] == '\0') {
      send_to_char ("Usage:  unalias <alias_name>\r\n",ch);
      return;
    }

    strcpy (final, "delete ");
    strcat (final, argument);

    do_alias (ch, final);

    return; 
}

/* Provie a random dream... */

#define NUM_DREAMS 30

static char *dreams[] = {

  "{cYou dream of seas and distant ports.{x\r\n",
  "{cYou dream of walking over hills and mountains.{x\r\n",
  "{cYou dream of awakening in a golden dawn.{x\r\n",
  "{cYou dream of silence and endless night.{x\r\n",
  "{cYou dream of absolute stillness.{x\r\n",

  "{cYou dream of a nihilistic paradise.{x\r\n",
  "{cYou dream of shattered lands and broken bones.{x\r\n",
  "{cYou dream of confusion and misunderstanding.{x\r\n",
  "{cYou dream of baying hounds and gibberous moons.{x\r\n",
  "{cYou dream of flying through icy clouds.{x\r\n",

  "{cYou dream of hanging over an endless drop.{x\r\n",
  "{cYou dream of being burried alive!{x\r\n",
  "{cYou dream of sea green pearls and a mermaid with silver hair.{x\r\n",
  "{cYou dream you are walrus, dreaming he is a butterfly.{x\r\n",
  "{cYou dream of a cat in a box who is not.{x\r\n",

  "{cYou dream of travelling at great speed through you know not where.{x\r\n",
  "{cYou dream of being here and of stumbling and tripping.{x\r\n",
  "{cYou dream of endlessly arriving.{x\r\n",
  "{cYou dream of awakening, but you cannot!{x\r\n",
  "{cYou dream of uncertainty, both of place and mind.{x\r\n",

  "{cYou dream of Morphius, in his palace of dreams.{x\r\n",  
  "{cYou dream that your brother is trying to murder you.{x\r\n",
  "{cYou dream of a pleasent and green place with willows and a stream.{x\r\n",
  "{cYou dream of an Aegyptian princess and nights of passion.{x\r\n",
  "{cYou dream of a gemstone, shaped like a heart.\r\n",

  "{cYou dream of the lady of change.{x\r\n",
  "{cYou dream of the lady of good fortune.{x\r\n",
  "{cYou dream of Death, Desire and Dispair, having tea together.{x\r\n",
  "{cYou dream of a bottle that holds the sun.{x\r\n",
  "{cYou dream of darkness and seaweed, preasure and eternal gloom.{x\r\n"
};

void random_dream(CHAR_DATA *ch) {

  char *dream;
 
  int index;

  index = number_range(1, NUM_DREAMS);

  index -= 1;

  dream = dreams[index];

  send_to_char(dream, ch);

  return;
}

/* IC Dream */
extern char fright_msg[16][80];

void do_dream(CHAR_DATA *ch, char *argument ) {
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;    
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    char cmd[MAX_STRING_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *room;
    ROOM_INDEX_DATA *mareroom;
    int roll,skill;
    bool no_mare = FALSE;

    if (IS_AFFECTED(ch, AFF_MORF)) { 
          send_to_char( "You can't do that!\r\n", ch );
          return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_NO_DREAM)) {
          send_to_char( "{cYour dreams are disturbing and frightening.{x\r\n", ch );
          return;
    }

/* Check for always dreaming... */

  if ( IS_SET(ch->act, ACT_DREAM_NATIVE)) {
    if ( argument[0] == '?'
    ||  argument[0] == '\0') { 
      send_to_char(
        "Syntax: Dream\r\n"
        "        Dream ?\r\n"
        "        Dream scare <player>\r\n"
        "        Dream watch <player>\r\n", ch);
        return;
    }

    argument = one_argument(argument, cmd);
    victim = NULL;
      
    if ( ch->position != POS_SLEEPING ) {
        send_to_char("Day dreaming is not a good idea.\r\n", ch);
        return;
    }  
  
    if ( !str_cmp(cmd, "scare") ) {
        one_argument( argument, arg );
         if ( arg[0] == '\0' )    {
	send_to_char( "Dream scare whom?\r\n", ch );
	return;
         }

         if (( victim = get_char_room( ch, arg ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
         }

         if (IS_NPC(victim))    {
	send_to_char( "You can't do that.\r\n", ch );
	return;
         }

         if ( victim->waking_room == NULL ) {
             send_to_char("They're not dreaming.\r\n", ch);
             return;
         } 
 
         if (ch->mana < 50) {
             send_to_char("You can't concentrate.\r\n", ch);
             return;
         } 

         ch->mana -= 50;

         skill = 20 + (ch->level - victim->level)*2;
         if (skill < 50) {
             send_to_char("You don't feel frightening.\r\n", ch);
             return;
         } 

         send_to_char("Everything becomes extremely scary!\r\n", victim);
         send_to_char("You manipulate you victims dreams!\r\n", ch);
         mcc = get_mcc(victim, ch, NULL, NULL, NULL, NULL, 0, NULL);
   
         if (!insanity(mcc, skill, NULL)) {
                send_to_char(fright_msg[number_bits(4)], victim);
         }
         free_mcc(mcc); 

         WAIT_STATE(ch, 24);
          return;


    } else if ( !str_cmp(cmd, "watch") ) {
        one_argument( argument, arg );
         if ( arg[0] == '\0' )    {
	send_to_char( "Dream watch whom?\r\n", ch );
	return;
         }

         if (( victim = get_char_room( ch, arg ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
         }

         if ( victim->desc == NULL )    {
	send_to_char( "You can't do that.\r\n", ch );
	return;
         }

         if ( victim->waking_room == NULL ) {
             send_to_char("They're not dreaming.\r\n", ch);
             return;
         } 

         if ( victim->desc->snoop_by != NULL )    {
	send_to_char( "Busy already.\r\n", ch );
	return;
         }

         if ( victim->level >= ch->level )    {
	send_to_char( "You failed.\r\n", ch );
	return;
         }

         if ( ch->desc != NULL ) {
	for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )	{
	    if ( d->character == victim || d->original == victim )	    {
		send_to_char( "No snoop loops.\r\n", ch );
		return;
	    }
	}
         }

         victim->desc->snoop_by = ch->desc;
         send_to_char( "Ok.\r\n", ch );
         send_to_char( "You feel watched...\r\n", ch );
          notify_message (ch, WIZNET_SNOOP, TO_IMM_ADMIN, victim->name);
          return;
    }

    return;
}


   /* Check for help... */

    if ( argument[0] == '?' ) { 
      send_to_char(
        "Syntax: Dream\r\n"
        "        Dream ?\r\n"
        "        Dream walk\r\n"
        "        Dream awaken\r\n"
        "        Dream resume\r\n"
        "        Dream [of char] say|shout|yell|scream|whisper <message>\r\n"
        "        Dream [of char] emote <action>\r\n"
        "        Dream [of char] cast <echo>\r\n", ch);
       if (ch->effective->skill[gsn_master_dreamer] >0) {
           send_to_char("        Dream deny <char>\r\n",ch);
       }
      return;
    }

   /* No parms means a prerecorded message... */

    if ( argument[0] == '\0' ) {
      random_dream(ch);
      return;
    }

   /* Parse the args... */

    argument = one_argument(argument, cmd);

   /* Now, do we need to pull the target character out? */

    victim = NULL;

    if ( !str_cmp(cmd, "of") ) {
       
  if ( ch->position != POS_SLEEPING ) {
      send_to_char("Day dreaming is not a good idea.\r\n", ch);
      return;
    }  
  
  argument = one_argument(argument, cmd);

      victim = get_char_world(ch, cmd);

     /* Now pull off the real command... */

      argument = one_argument(argument, cmd);

     /* Now, if we have a target, check dreaming skill... */

      if ( victim != NULL ) {
        if ( !check_skill(ch, gsn_dreaming, 0) ) {
          victim = NULL;
        }
      }
    }


   /* Dream deny */

    if ( !str_cmp(cmd, "deny") ) {

      if (IS_NPC(ch)) {
           send_to_char("Don't be silly!\r\n",ch);
            return;
      }

      if (ch->nightmare == TRUE) {
           send_to_char("You got bigger problems right now!!\r\n",ch);
            return;
      }

      if (ch->effective->skill[gsn_master_dreamer] <1) {
           send_to_char("You're no master dreamer!\r\n",ch);
            return;
      }

      argument = one_argument(argument, cmd);
      victim = get_char_room(ch, cmd);
     
      if (victim == ch) {
            send_to_char("You really hope you are real!!\r\n",ch);
            return;
      }

      if ( victim == NULL ) {
            send_to_char("That's not here!\r\n",ch);
            return;
      }

      if ( ch->waking_room == NULL ) {
          send_to_char("{cEverything is much too real.{x\r\n", ch);
          return;
      } 

    if ( IS_NPC(victim) 
      && victim->pIndexData->pShop != NULL) {
	send_to_char("The shopkeeper wouldn't like that.\r\n",ch); 
        return;
    }

    if ( IS_NPC(victim) 
      && ( IS_SET(victim->act,ACT_PRACTICE)
        ||  IS_SET(victim->act,ACT_IS_ALIENIST)
        ||  IS_SET(victim->act,ACT_IS_HEALER))) {
	send_to_char("I think Hypnos would {rNOT{x approve.\r\n",ch);
	return;
    }

    if (IS_SET(victim->act, ACT_DREAM_NATIVE)) {
	send_to_char("Oh no. They {rare{x real..\r\n",ch);
	return;
    }

      if (ch->mana<80) {
            send_to_char("Not enough mana!\r\n",ch);
            return;
      }

     /* First a skill check... */
      ch->mana -=40;

      if ( !check_skill(ch, gsn_master_dreamer, 0) ) {
           send_to_char("You can't believe it's not real.\r\n", ch);
           return;
      } 

       ch->mana -=40;
       roll = number_percent() + ch->level - victim->level;     
       
       WAIT_STATE(ch, 24);

       if (roll>75) {
          check_improve(ch, gsn_master_dreamer, TRUE, 2);

           if (IS_NPC(victim)) {
                send_to_char("Yes, it's all just a dream...\r\n", ch);
                act( "$n suddenly seems very unreal and dies...", victim, NULL, NULL, TO_ROOM );
                raw_kill( victim, TRUE );
           } else {
                if (victim->waking_room == NULL) {
                     send_to_char("Your victim is more at home than you are.\r\n", ch);
                } else {
                     act( "$n fades out of the world...", victim, NULL, NULL, TO_ROOM );
                     char_from_room(victim);
                     victim->in_zone = -1;
                     char_to_room(victim, victim->waking_room);
                     victim->waking_room = NULL;
                     victim->nightmare = FALSE;
                     send_to_char("{rSomething takes over the control of your dreams!{x\r\n", victim);
                     send_to_char("You awaken back screaming in the real world...\r\n", victim);
                     act( "$n awakens with a scream.", victim, NULL, NULL, TO_ROOM );
                     set_activity( victim, POS_STANDING, NULL, ACV_NONE, NULL);
                     do_scream(victim, "");
                     do_look(victim, "auto");
                }
            }
       } else {
           check_improve(ch, gsn_master_dreamer, FALSE, 2);
           roll = number_percent();
           if (roll>9) {
               send_to_char("You are very unsure about your own existence.\r\n", ch);
           } else {
               roll += VNUM_NIGHTMARE_LOW;
               if (victim->level > 40) roll +=10;
               send_to_char("Something is not right...\r\n", ch);
               raw_kill( victim, TRUE );
               sprintf (buf, "deny_char: %d", roll);
               log_string (buf);
               if ( ( pMobIndex = get_mob_index( roll ) ) == NULL ) {
                    bug( "Nightmare - Bad mob vnum: vnum %d.", roll );
	     return;
               }
               victim = create_mobile( pMobIndex );
               char_to_room( victim, ch->in_room );
               act( "$n suddenly undergoes a terrible change...", victim, NULL, NULL, TO_ROOM );
               act( "Something horrible sheds $ns mask.", victim, NULL, NULL, TO_ROOM );
           }
        }
      
      return;
    }


   /* Dream walking ? */

    if ( !str_cmp(cmd, "walk") ) {
        int check, bonus;
        
  if ( ch->position != POS_SLEEPING ) {
      send_to_char("Day dreaming is not a good idea.\r\n", ch);
      return;
    }
   
  /* Can only do this once... */

      if ( ch->waking_room != NULL ) {
        send_to_char("You dream of dreams within a dream.\r\n", ch); 
        return;
      } 

     /* First a skill check... */

      if ( !check_skill(ch, gsn_dreaming, 0) ) {
        random_dream(ch);
        return;
      } 

      check_improve(ch, gsn_dreaming, TRUE, 2);

      check = 100 - (100 * ch->hit / ch->max_hit);
      if (IS_AFFECTED(ch, AFF_CALM))		check = (check *  8)/10;
      if (IS_AFFECTED(ch, AFF_SANCTUARY))	check = (check *  5)/10;
      if (IS_AFFECTED(ch, AFF_RELAXED))		check = (check *  5)/10;
      if (IS_AFFECTED(ch, AFF_CURSE))		check = (check * 12)/10;
      if (IS_AFFECTED(ch, AFF_POISON))		check = (check * 12)/10;
      if (IS_AFFECTED(ch, AFF_PLAGUE))		check = (check * 12)/10;
      if (IS_AFFECTED(ch, AFF_BERSERK))		check = (check * 12)/10;
      if (IS_AFFECTED(ch, AFF_HALLUCINATING))	check =  check * 2;

      if (IS_RAFFECTED(ch->in_room, RAFF_HIGH_MAGIC)) bonus =20;
      else bonus = 10;

     /* Now, where should we go? */
 
      room = NULL; 
      mareroom = NULL;

      if ( ch->in_room != NULL
      && ch->in_room->dream != 0 ) {
        room = get_room_index(ch->in_room->dream);
      }

      if ( ch->in_room != NULL
      && ch->in_room->mare != 0 ) {
        if (ch->in_room->mare >0) mareroom = get_room_index(ch->in_room->mare);
        else no_mare = TRUE;
      }

      if ( room == NULL 
      && ch->in_room != NULL
      && ch->in_room->area != NULL
      && ch->in_room->area->dream != 0 ) {
        room = get_room_index(ch->in_room->area->dream);
      }

      if (mareroom == NULL 
      && !no_mare
      && ch->in_room != NULL
      && ch->in_room->area != NULL
      && ch->in_room->area->mare != 0 ) {
        if (ch->in_room->area->mare >0) mareroom = get_room_index(ch->in_room->area->mare);
        else no_mare = TRUE;
      }

      if ( room == NULL 
        && ch->in_room != NULL
        && ch->in_room->area != NULL
        && zones[ch->in_room->area->zone].dream != 0 ) {
        room = get_room_index(zones[ch->in_room->area->zone].dream);
      }

      if ( room == NULL 
        && mud.dream != 0 ) {
        room = get_room_index(mud.dream);
      }
       
      if (mareroom == NULL 
      && !no_mare
      && mud.mare != 0 ) {
        mareroom = get_room_index(mud.mare);
      }

      ch->nightmare = FALSE;

      if ( !check_skill(ch, gsn_dreaming, check - bonus) 
      && mareroom 
      && !IS_SET(ch->in_room->room_flags, ROOM_DREAM_SAFE)
      && !no_mare
      && ch->level >= 15) {
           room = mareroom;
           ch->nightmare = TRUE;
      } 
      
      if (IS_RAFFECTED(ch->in_room, RAFF_MARE) && mareroom)	{
           room = mareroom;
           ch->nightmare = TRUE;
      } 

     /* Just send a dream if nowhere to go... */

      if ( room == NULL ) {
        random_dream(ch);
        return;
      } 

     /* Ok, hidden movement time... */

      act( "$n fades into the world of dreams...", ch, NULL, NULL, TO_ROOM );
      ch->waking_room = ch->in_room;
      char_from_room(ch);
      ch->in_zone = -1;
      char_to_room(ch, room);
      ch->dreaming_room = NULL;

     /* Just make sure it worked... */

      if ( ch->in_room == NULL ) {
        char_to_room(ch, ch->waking_room);
        ch->waking_room = NULL;
        send_to_char("{cHowling demons chase you through your dreams!{x\r\n",  ch);
        act( "$n fades back into the world...", ch, NULL, NULL, TO_ROOM );

        return;
      }

     /* Success! */       

      set_activity( ch, POS_STANDING, NULL, ACV_NONE, NULL);
      if (ch->on_furniture != NULL) remove_furnaff(ch, ch->on_furniture);
      ch->on_furniture = NULL;

      send_to_char("You awaken in another world...\r\n", ch);
      if (ch->nightmare == TRUE) send_to_char("This place gives you the creeps...\r\n", ch);
      else send_to_char("Sleep and 'dream awaken' to return...\r\n", ch);
     
      act( "$n fades into existance...", ch, NULL, NULL, TO_ROOM );

      do_look(ch, "auto");

      return;
    }

   /* Dream awaken ? */

    
if ( !str_cmp(cmd, "awaken") ) {
  int bonus =25;

  if ( ch->position != POS_SLEEPING ) {
      send_to_char("Day dreaming is not a good idea.\r\n", ch);
      return;
    }

     /* Can only do this once... */

      if ( ch->waking_room == NULL ) {
        send_to_char("{cYou dream of awakening in a better world.{x\r\n", ch);
        return;
      } 

     /* First a skill check... */

      if (ch->nightmare == TRUE) {
          if (IS_RAFFECTED(ch->in_room, RAFF_HIGH_MAGIC)) bonus =15;
   
          if (ch->level <= 15) ch->nightmare = FALSE;
          if (ch->move < ch->level * 2) {
               send_to_char("{cYou are too horrified.{x\r\n", ch);
               return;
          }
          if ( !check_skill(ch, gsn_dreaming, bonus)
          && !IS_SET(ch->in_room->room_flags, ROOM_DREAM_SAFE)) {
               random_dream(ch);
               send_to_char("{cYou seem to be trapped in a nightmare.{x\r\n", ch);
               ch->move -= ch->level * 2;
               ch->move = UMAX(ch->move, 0);
               return;
          }
      } else {
           if ( !check_skill(ch, gsn_dreaming, 0) ) {
                 random_dream(ch);
                 return;
           }
      }

      check_improve(ch, gsn_dreaming, TRUE, 2);

     /* Where do we go? */

      room = ch->in_room;

     /* Silent movement... */

      act( "$n fades out of the world...", ch, NULL, NULL, TO_ROOM );

      char_from_room(ch);

      ch->in_zone = -1;

      char_to_room(ch, ch->waking_room);
      ch->nightmare = FALSE;

     /* Check it worked... */

      if ( ch->in_room == NULL ) {
        char_to_room(ch, room);
        send_to_char("{cYou dream of an endless, impassable wall!{x\r\n", ch);
        act( "$n fades back into the world...", ch, NULL, NULL, TO_ROOM );
        return;
      }

     /* Success! */

      ch->waking_room = NULL;

      send_to_char("You awaken back in the real world...\r\n", ch);

      act( "$n awakens from a deep sleep.", ch, NULL, NULL, TO_ROOM );

      set_activity( ch, POS_STANDING, NULL, ACV_NONE, NULL);
      if (ch->on_furniture != NULL) remove_furnaff(ch, ch->on_furniture);
      ch->on_furniture = NULL;

      do_look(ch, "auto");

     /* See if we can remember where we were... */

      if ( check_skill(ch, gsn_dreaming, 0) ) {
        check_improve(ch, gsn_dreaming, TRUE, 2);
        ch->dreaming_room = room;
        return;
      } 

      return;
    }

   /* Dream resuming ? */

    if ( !str_cmp(cmd, "resume") ) {

  if ( ch->position != POS_SLEEPING ) {
      send_to_char("Day dreaming is not a good idea.\r\n", ch);
      return;
    }

     /* Can only do this once... */

      if ( ch->waking_room != NULL ) {
        send_to_char("You dream of dreams within a dream.\r\n", ch); 
        return;
      } 

     /* Do we have somewhere to go? */

      if ( ch->dreaming_room == NULL ) {
        random_dream(ch); 
        return;
      } 

     /* First a skill check... */

      skill=get_skill(ch, gsn_master_dreamer);

      if ( !check_skill(ch, gsn_dreaming, 30 -skill) ) {
        ch->dreaming_room = NULL;
        random_dream(ch);
        return;
      } 

      check_improve(ch, gsn_dreaming, TRUE, 2);

     /* Now, where should we go? */
 
      room = ch->dreaming_room; 

     /* Just send a dream if nowhere to go... */

      if ( room == NULL ) {
        random_dream(ch);
        return;
      } 

     /* Ok, hidden movement time... */

      act( "$n fades into the world of dreams...", ch, NULL, NULL, TO_ROOM );
      ch->waking_room = ch->in_room;
      char_from_room(ch);
      ch->in_zone = -1;
      char_to_room(ch, room);

     /* Just make sure it worked... */

      if ( ch->in_room == NULL ) {
        char_to_room(ch, ch->waking_room);
        ch->waking_room = NULL;
        send_to_char("{cHowling demons chase you through your dreams!{x\r\n", ch);
        act( "$n fades back into the world...", ch, NULL, NULL, TO_ROOM );
        return;
      }

     /* Success! */       

      set_activity( ch, POS_STANDING, NULL, ACV_NONE, NULL);
      if (ch->on_furniture != NULL) remove_furnaff(ch, ch->on_furniture);
      ch->on_furniture = NULL;

      send_to_char("You awaken in another world...\r\n", ch);
      act( "$n fades into existance...", ch, NULL, NULL, TO_ROOM );
      do_look(ch, "auto");
      return;
    }

   /* Build WEV and MCC... */

    mcc = NULL;
    wev = NULL;

   /* Small mana charge... */

    if ( ch->mana < 10 ) {
      random_dream(ch);
      return;
    }

    ch->mana -= 2;

   /* Dream whisper ? */

    if ( !str_cmp(cmd, "whisper") ) {

  if ( ch->position != POS_SLEEPING ) {
      send_to_char("Day dreaming is not a good idea.\r\n", ch);
      return;
    }
      
if ( argument[0] == '\0' ) {
        send_to_char("{cYou dream of silence and endless night.{x\r\n", ch);
        return;
      } 

      if ( victim == NULL ) {
        mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 
                      ch->speak[SPEAK_PUBLIC], argument);

        wev = get_wev(WEV_DREAM, WEV_DREAM_SAY, mcc,
                     "{CYou whispers '@t1'{x\r\n",
                      NULL,
                     "{C@a2 whisper '@t1'{x\r\n");
      } else {
        mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 
                      ch->speak[SPEAK_PRIVATE], argument);

        wev = get_wev(WEV_DREAM, WEV_DREAM_PSAY, mcc,
                     "{CYou whisper '@t1'{x\r\n",
                     "{C@a2 whispers '@t1'{x\r\n",
                      NULL);
      }
    }

   /* Dream say ? */

    if ( !str_cmp(cmd, "say") ) {

  if ( ch->position != POS_SLEEPING ) {
      send_to_char("Day dreaming is not a good idea.\r\n", ch);
      return;
    }

      if ( argument[0] == '\0' ) {
        send_to_char("{cYou dream of silence and endless night.{x\r\n", ch);
        return;
      } 

      if ( victim == NULL ) {
        mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 
                      ch->speak[SPEAK_PUBLIC], argument);

        wev = get_wev(WEV_DREAM, WEV_DREAM_SAY, mcc,
                     "{cYou say '@t1'{x\r\n",
                      NULL,
                     "{c@a2 says '@t1'{x\r\n");
      } else {
        mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 
                      ch->speak[SPEAK_PRIVATE], argument);

        wev = get_wev(WEV_DREAM, WEV_DREAM_PSAY, mcc,
                     "{cYou say '@t1'{x\r\n",
                     "{c@a2 says '@t1'{x\r\n",
                      NULL);
      }
    }

   /* Dream shout ? */

    if ( !str_cmp(cmd, "shout") ) {

  if ( ch->position != POS_SLEEPING ) {
      send_to_char("Day dreaming is not a good idea.\r\n", ch);
      return;
    }
      
if ( argument[0] == '\0' ) {
        send_to_char("{cYou dream of silence and endless night.{x\r\n", ch);
        return;
      } 

      if ( victim == NULL ) {
        mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 
                      ch->speak[SPEAK_PUBLIC], argument);

        wev = get_wev(WEV_DREAM, WEV_DREAM_SAY, mcc,
                     "{CYou shout '@t1'{x\r\n",
                      NULL,
                     "{C@a2 shouts '@t1'{x\r\n");
      } else {
        mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 
                      ch->speak[SPEAK_PRIVATE], argument);

        wev = get_wev(WEV_DREAM, WEV_DREAM_PSAY, mcc,
                     "{CYou shout '@t1'{x\r\n",
                     "{C@a2 shouts '@t1'{x\r\n",
                      NULL);
      }
    }

   /* Dream yell ? */

    if ( !str_cmp(cmd, "yell") ) {

      if ( argument[0] == '\0' ) {
        send_to_char("{cYou dream of silence and endless night.{x\r\n", ch);
        return;
      } 

      if ( victim == NULL ) {
        mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 
                      ch->speak[SPEAK_PUBLIC], argument);

        wev = get_wev(WEV_DREAM, WEV_DREAM_SAY, mcc,
                     "{CYou yell '@t1'{x\r\n",
                      NULL,
                     "{C@a2 yells '@t1'{x\r\n");
      } else {
        mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 
                      ch->speak[SPEAK_PRIVATE], argument);

        wev = get_wev(WEV_DREAM, WEV_DREAM_PSAY, mcc,
                     "{CYou yell '@t1'{x\r\n",
                     "{C@a2 yells '@t1'{x\r\n",
                      NULL);
      }
    }

   /* Dream scream ? */

    if ( !str_cmp(cmd, "scream") ) {

  if ( ch->position != POS_SLEEPING ) {
      send_to_char("Day dreaming is not a good idea.\r\n", ch);
      return;
    }

      if ( argument[0] == '\0' ) {

        mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 
                      gsn_english, argument);

        if ( victim == NULL ) {
          wev = get_wev(WEV_DREAM, WEV_DREAM_SAY, mcc,
                       "{MYou scream in terror!{x\r\n",
                        NULL,
                       "{M@a2 screams in terror!{x\r\n");
        } else {
          wev = get_wev(WEV_DREAM, WEV_DREAM_PSAY, mcc,
                       "{MYou scream in terror!{x\r\n",
                       "{M@a2 screams in terror!{x\r\n",
                        NULL);
        }
      } else { 

        if ( victim == NULL ) {
          mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 
                        ch->speak[SPEAK_PUBLIC], argument);

          wev = get_wev(WEV_DREAM, WEV_DREAM_SAY, mcc,
                       "{MYou scream '@t1'{x\r\n",
                        NULL,
                       "{M@a2 screams '@t1'{x\r\n");
        } else {
          mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 
                        ch->speak[SPEAK_PRIVATE], argument);

          wev = get_wev(WEV_DREAM, WEV_DREAM_PSAY, mcc,
                       "{MYou scream '@t1'{x\r\n",
                       "{M@a2 screams '@t1'{x\r\n",
                        NULL);
        }
      }
    }

   /* Dream emote ? */

    if ( !str_cmp(cmd, "emote") ) {

  if ( ch->position != POS_SLEEPING ) {
      send_to_char("Day dreaming is not a good idea.\r\n", ch);
      return;
    }

      if ( argument[0] == '\0' ) {
        send_to_char("{cYou dream of absolute stillness.{x\r\n", ch);
        return;
      } 

      mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 
                    gsn_english, argument);

      if ( victim == NULL ) {
        wev = get_wev(WEV_DREAM, WEV_DREAM_EMOTE, mcc,
                     "{cYou @t0{x\r\n",
                      NULL,
                     "{c@a2 @t0{x\r\n");
      } else {
        wev = get_wev(WEV_DREAM, WEV_DREAM_PEMOTE, mcc,
                     "{cYou @t0{x\r\n",
                     "{c@a2 @t0{x\r\n",
                      NULL);
      }
    }

   /* Dream cast ? */

    if ( !str_cmp(cmd, "cast") ) {

  if ( ch->position != POS_SLEEPING ) {
      send_to_char("Day dreaming is not a good idea.\r\n", ch);
      return;
    }

      if ( argument[0] == '\0' ) {
        send_to_char("{cYou dream of a nihilistic paradise.{x\r\n", ch);
        return;
      } 

      if ( !check_skill(ch, gsn_dreaming, 50) ) {
        send_to_char("{cYou dream of shattered lands and broken bones.{x\r\n",
                                                                           ch);
        return;
      }

      mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 
                    gsn_english, argument);

      if ( victim == NULL ) {
        wev = get_wev(WEV_DREAM, WEV_DREAM_CAST, mcc,
                     "{c@t0{x\r\n",
                      NULL,
                     "{c@t0{x\r\n");
      } else {
        wev = get_wev(WEV_DREAM, WEV_DREAM_PCAST, mcc,
                     "{c@t0{x\r\n",
                     "{c@t0{x\r\n",
                      NULL);
      }
    }

   /* Should have a wev at this point... */

    if ( wev == NULL 
      || mcc == NULL ) {
      send_to_char("{cYou dream of confusion and misunderstanding.\r\n{x", ch);
      return;
    }

   /* Challange... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

   /* Ok, output the wev... */

    if ( victim != NULL ) {
      mob_handle_wev(ch, wev, "Dream");
      mob_handle_wev(victim, wev, "Dream");
    } else { 
      world_issue_wev(wev, "Dream");
    } 

   /* Free the wev... */

    free_wev(wev);

   /* All done... */

    return;
}

void talk_auction (char *argument){
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *original;

    sprintf (buf,"{rAUCTION:{x %s", argument);

    for (d = descriptor_list; d != NULL; d = d->next)    {
        original = d->original ? d->original : d->character; /* if switched */
        if ((d->connected == CON_PLAYING) && !IS_SET(original->comm, COMM_NOAUCTION) && auction->auction_room->area->zone == original->in_room->area->zone)
            act (buf, original, NULL, NULL, TO_CHAR);

    }
}


void drop_artifact(CHAR_DATA *ch, OBJ_DATA *obj) {

    if (obj->next_content != NULL ) drop_artifact(ch, obj->next_content);
    if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)) {
        if (obj->carried_by) obj_from_char(obj);
        else obj_from_obj(obj);
        obj_to_room(obj, ch->in_room);
        return;
    }
    if ( obj->contains != NULL ) drop_artifact(ch, obj->contains);
    return;
}


void do_stell(CHAR_DATA *ch, char *args) {
char arg[MAX_INPUT_LENGTH];
int type = 0;
MOB_CMD_CONTEXT *mcc;
WEV *wev;   
 
  if ( args[0] == '\0' ) {
    send_to_char("Tell who?\r\n", ch);
    return;  
  }

  args = one_argument(args, arg);
  type = flag_value(act_flags, arg);
  if (type <= 0) {
    send_to_char("Unknown special group.\r\n", ch);
    return;
  }

  if (!IS_SET(ch->act, type)) {
    send_to_char("You don't belong to that special group.\r\n", ch);
    return;
  }

  if (ch->mana < 25) {
    send_to_char("You don't have enough mana left!\r\n", ch);
    return;
  }

  ch->mana -= 25;

  mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, type, args);
  wev = get_wev(WEV_OOCC, WEV_OOCC_STELL, mcc,
  "{r[{R@a2{r]: @t0{x\r\n",
  NULL,
  "{r[{R@a2{r]: @t0{x\r\n");

  if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
  }

   world_issue_wev(wev, flag_string(act_flags, type));
   free_wev(wev);
   return;
}


void do_mtell(CHAR_DATA *ch, char *args) {
CHAR_DATA *spouse;
MOB_CMD_CONTEXT *mcc;
WEV *wev;   
 
  if (IS_NPC(ch)) return;

  if (!str_cmp(ch->pcdata->spouse, "")) {
    send_to_char("You are not married.\r\n", ch);
    return;  
  }

  if ((spouse =get_char_world_player(ch, ch->pcdata->spouse)) == NULL) {
    send_to_char("Your partner is not on.\r\n", ch);
    return;  
  }

  if (ch->mana < 15) {
    send_to_char("You don't have enough mana left!\r\n", ch);
    return;
  }

  ch->mana -= 15;

  mcc = get_mcc(ch, ch, spouse, NULL, NULL, NULL, 0, args);
  wev = get_wev(WEV_OOCC, WEV_OOCC_MTELL, mcc,
  "{r[{R@a2{r]: @t0{x\r\n",
  "{r[{R@a2{r]: @t0{x\r\n",
  NULL);

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

   world_issue_wev(wev, "OOC:");
   free_wev(wev);
   return;
}


void free_ignore(IGNORE *ignore, IGNORE *ignore_prev) {

      if (!ignore) return;

      free_string(ignore->name);
      ignore->name = NULL;

      if (ignore_prev) ignore_prev->next = ignore->next;
      ignore = NULL;
      return;
}


IGNORE *new_ignore(void ) {
IGNORE *ignore;

    ignore =(IGNORE *) alloc_perm( sizeof(*ignore));
    ignore->name = NULL;
    return ignore;
}


void do_ignore(CHAR_DATA *ch, char *args) {
char arg[MAX_INPUT_LENGTH];
IGNORE *ignore;
IGNORE *ignore_prev = NULL;

     if (IS_NPC(ch)) return;

     if (args[0] == '\0' ) {
         if (!ch->pcdata->ignore_list) {
             send_to_char("You're not ignoring anyone.\r\n", ch);
             return;
         }

         send_to_char("{CYou're ignoring:{x\r\n", ch);
         for (ignore = ch->pcdata->ignore_list; ignore; ignore = ignore->next) {
             sprintf_to_char(ch, "     {g%s{x\r\n", capitalize(ignore->name));
         }
         return;
     }

     one_argument(args, arg);
     
     if (check_ignore(ch, arg)) {
         for(ignore = ch->pcdata->ignore_list; ignore; ignore = ignore->next) {
              if (!str_cmp(ignore->name, arg)) break;
              ignore_prev = ignore;
         }

         if (ignore) {
             sprintf_to_char(ch, "You remove the IGNORE tag from %s.\r\n", ignore->name);
             if (!ignore_prev) ch->pcdata->ignore_list = ignore->next;
             free_ignore(ignore, ignore_prev);
         }

     } else {
         IGNORE * ignore_new;
         CHAR_DATA *victim;

         if ((victim = get_char_world_player(ch, arg)) == NULL) {
             sprintf_to_char(ch, "You can't find %s.\r\n", arg);
             return;
         }

         if (IS_NPC(victim)) {
             send_to_char("This is no player!\r\n",ch);
             return;
         }  

         if (IS_IMMORTAL(victim)) {
             send_to_char("You can't ignore the Gods!\r\n",ch);
             return;
         }  

         if (victim == ch) {
             send_to_char("Ignoring you sounds like a good idea!\r\n",ch);
             return;
         }  

         ignore_new  = new_ignore();

         ignore_new->name = str_dup(victim->name);
         ignore_new->next = ch->pcdata->ignore_list;
         ch->pcdata->ignore_list = ignore_new;
         sprintf_to_char(ch, "You will now ignore %s.\r\n", ignore_new->name);
     }
     return;
}


bool check_ignore(CHAR_DATA *ch, char* argument) {
char arg[MAX_INPUT_LENGTH];
IGNORE *ignore;

      if (IS_NPC(ch)) return FALSE;

      one_argument(argument, arg);

      for (ignore = ch->pcdata->ignore_list; ignore; ignore = ignore->next) {
             if (!str_cmp(ignore->name, arg)) return TRUE;
      }
      return FALSE;
}


void haggle_emote(CHAR_DATA *ch) {

    switch(number_range(0, 5)) {
          default:
             check_social(ch, "growl", "");
             break;
          case 1:
             do_say(ch, "Do you think I'm foolish?");
             break;
          case 2:
             do_say(ch, "What do you think? I got children to feed!");
             break;
          case 3:
             check_social(ch, "shake", "");
             break;
          case 4:
             do_say(ch, "Never!");
             break;
          case 5:
             do_say(ch, "Make a better offer - this one I can't accept!");
             break;
    }

    return;
}



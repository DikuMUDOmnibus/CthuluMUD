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
#include "skill.h"
#include "spell.h"
#include "affect.h"
#include "exp.h"
#include "prof.h"
#include "econd.h"
#include "mob.h"
#include "wev.h"
#include "society.h"
#include "profile.h"
#include "race.h"
#include "bank.h"
#include "vlib.h"
#include "partner.h"
#include "version.h"

/* command procedures needed */
DECLARE_DO_FUN(do_rstat);
DECLARE_DO_FUN(do_mstat);
DECLARE_DO_FUN(do_ostat);
DECLARE_DO_FUN(do_skillstat);
DECLARE_DO_FUN(do_rset);
DECLARE_DO_FUN(do_mset);
DECLARE_DO_FUN(do_oset);
DECLARE_DO_FUN(do_sset);
DECLARE_DO_FUN(do_mfind);
DECLARE_DO_FUN(do_ofind);
DECLARE_DO_FUN(do_slookup);
DECLARE_DO_FUN(do_mload);
DECLARE_DO_FUN(do_oload);
DECLARE_DO_FUN(do_force);
DECLARE_DO_FUN(do_quit);
DECLARE_DO_FUN(do_save);
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_pk);
DECLARE_DO_FUN(do_echo);
DECLARE_DO_FUN(do_delete);
DECLARE_DO_FUN(do_goto);


/* Local functions... */
ROOM_INDEX_DATA *	find_location		(CHAR_DATA *ch, char *arg );
FILE 			*popen			(const char *command, const char *type );
int 			pclose			(FILE *stream );
char			*fgetf			(char *s, int n, register FILE *iop ); 
void			save_ban		(void);
void 			save_artifacts		(void);
void			save_cults		(void);
bool			write_to_descriptor	(int desc, char *txt, int length);
CHAR_DATA 		*highest_ranking_admin	(void);
bool 			check_parse_name 	(char* name);
void    			reboot_shutdown	(short type);
void    			check_muddeath		(void);
int 			invis_lev		(CHAR_DATA *ch);
void 			execute_approve		(CHAR_DATA *ch, char *argument, bool ok);
void 			approve_list		(CHAR_DATA *ch);
bool 			duplicate_ip		(DESCRIPTOR_DATA *d);


/* External functions... */
extern void 		save_all_lockers		(void);
extern void 		save_banks		(void);
extern void 		raw_kill			(CHAR_DATA *ch, bool corpse);
extern AREA_DATA 	*get_vnum_area  	(VNUM vnum);
extern void 		copyover 		(bool crash);


short reboot_type =-1;
int pulse_muddeath = -1;

BAN_DATA *		ban_free;
BAN_DATA *		ban_list;


/* equips a character */

void do_outfit ( CHAR_DATA *ch, char *argument ) {
    if (!IS_NEWBIE(ch) || IS_NPC(ch)){
	send_to_char("Find it yourself!\r\n",ch);
	return;
    }

    equip_char_by_prof(ch, FALSE);
    send_to_char("You have been equipped by Marduk.\r\n",ch);
}


/* Dump a mobs commands... */

void do_queue(CHAR_DATA *ch, char *args) {
CHAR_DATA *mob;
MOB_CMD_BUF *mcb;
char buf[MAX_STRING_LENGTH];

    if ( args[0] == '\0'  || args[0] == '?' ) {
      send_to_char("Syntax: queue mob\r\n", ch);
      return;
    }

    mob = get_char_room(ch, args);

    if (mob == NULL) { 
      send_to_char("No such mob here.\r\n", ch);
      return;
    }

    mcb = mob->mcb;

    if (mcb == NULL) {
      send_to_char("No queued commands.\r\n", ch);
      return;
    } 

    sprintf(buf, "A %s has the following enqueued:\r\n"
                 "  Delay CmdId Command\r\n"
                 "  ----- ----- ------------------------------------\r\n",
                 mob->short_descr);

    send_to_char(buf, ch);

    while ( mcb != NULL ) {

      sprintf(buf, "  %5d %5d %s\r\n", mcb->delay, mcb->cmd_id, mcb->cmd);

      send_to_char(buf, ch);
 
      mcb = mcb->next;
    }

    return;
}

     
/* RT nochannels command, for those spammers */
void do_nochannels( CHAR_DATA *ch, char *argument ){
char arg[MAX_INPUT_LENGTH];
char buf[MAX_STRING_LENGTH];
CHAR_DATA *victim;
 
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )    {
        send_to_char( "Nochannel whom?", ch );
        return;
    }
 
    if ((victim = get_char_world( ch, arg ) ) == NULL )    {
        send_to_char( "They aren't here.\r\n", ch );
        return;
    }
 
    if (get_divinity(ch) >= DIV_GOD) {
        if (IS_SET(victim->comm, COMM_NOCHANNELS) )    {
            if ( get_trust(victim) > get_trust( ch ) )    {
	send_to_char( "You failed.\r\n", ch );
	return;
            }

            REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
            send_to_char( "The gods have restored your channel priviliges.\r\n",  victim );
            send_to_char( "NOCHANNELS removed.\r\n", ch );
    
        } else {
            if ( get_trust( victim ) >= get_trust( ch ) )    {
	send_to_char( "You failed.\r\n", ch );
	return;
            }

            SET_BIT(victim->comm, COMM_NOCHANNELS);
            send_to_char( "The gods have revoked your channel priviliges.\r\n", victim );
            send_to_char( "NOCHANNELS set.\r\n", ch );
        }
    } else {
        if (!IS_IMMORTAL(ch) && !IS_SET(ch->plr, PLR_HELPER)) {
	send_to_char( "You failed.\r\n", ch );
	return;
        }

        if (!IS_NEWBIE(victim)) {
	send_to_char( "You failed.\r\n", ch );
	return;
        }

        if (IS_SET(victim->comm, COMM_NOCHANNELS) )    {
            REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
            send_to_char( "The gods have restored your channel priviliges.\r\n",  victim );
            send_to_char( "NOCHANNELS removed.\r\n", ch );
        } else {
            SET_BIT(victim->comm, COMM_NOCHANNELS);
            send_to_char( "The gods have revoked your channel priviliges.\r\n", victim );
            send_to_char( "NOCHANNELS set.\r\n", ch );
            sprintf(buf, "%s has gagged %s.\r\n", ch->name, victim->name);
            make_note ("Immortals", ch->name, "imm", "gag report", 3, buf);
        }
    }

    return;
}


void do_noport( CHAR_DATA *ch, char *argument ){
char arg[MAX_INPUT_LENGTH];
CHAR_DATA *victim;
 
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )    {
        send_to_char( "Noport whom?", ch );
        return;
    }
 
    if ((victim = get_char_world( ch, arg ) ) == NULL )    {
        send_to_char( "They aren't here.\r\n", ch );
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char( "That's no player.\r\n", ch );
        return;
    }
 
    if (!IS_IMMORTAL(ch)
    || IS_IMMORTAL(victim)) {
        send_to_char( "You can't do that.\r\n", ch );
        return;
    }

     if (IS_SET(victim->comm, COMM_NOPORT)) {
         REMOVE_BIT(victim->comm, COMM_NOPORT);
         send_to_char( "The gods have restored your teleport priviliges.\r\n",  victim );
         send_to_char( "NOPORT removed.\r\n", ch );
    } else {
         SET_BIT(victim->comm, COMM_NOPORT);
         send_to_char( "The gods have revoked your teleport priviliges.\r\n", victim );
         send_to_char( "NOPORT set.\r\n", ch );
    }
    return;
}


void do_bamfin( CHAR_DATA *ch, char *argument ) {
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) )    {
	smash_tilde( argument );

	if (argument[0] == '\0')	{
	    sprintf(buf,"Your poofin is %s\r\n",ch->pcdata->bamfin);
	    send_to_char(buf,ch);
	    return;
	}

	if ( strstr(argument,ch->name) == NULL)	{
	    send_to_char("You must include your name.\r\n",ch);
	    return;
	}
	     
	free_string( ch->pcdata->bamfin );
	ch->pcdata->bamfin = str_dup( argument );

        sprintf(buf,"Your poofin is now %s\r\n",ch->pcdata->bamfin);
        send_to_char(buf,ch);
    }
    return;
}



void do_bamfout( CHAR_DATA *ch, char *argument ) {
    char buf[MAX_STRING_LENGTH];
 
    if ( !IS_NPC(ch) )    {
        smash_tilde( argument );
 
        if (argument[0] == '\0')        {
            sprintf(buf,"Your poofout is %s\r\n",ch->pcdata->bamfout);
            send_to_char(buf,ch);
            return;
        }
 
        if ( strstr(argument,ch->name) == NULL)        {
            send_to_char("You must include your name.\r\n",ch);
            return;
        }
 
        free_string( ch->pcdata->bamfout );
        ch->pcdata->bamfout = str_dup( argument );
 
        sprintf(buf,"Your poofout is now %s\r\n",ch->pcdata->bamfout);
        send_to_char(buf,ch);
    }
    return;
}



void do_deny( CHAR_DATA *ch, char *argument ){
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    if (IS_MUD(ch)) return;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )    {
	send_to_char( "Deny whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( IS_NPC(victim) )    {
	send_to_char( "Not on NPC's.\r\n", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )    {
	send_to_char( "You failed.\r\n", ch );
	return;
    }

    SET_BIT(victim->plr, PLR_DENY);
    send_to_char( "You are denied access!\r\n", victim );
    send_to_char( "OK.\r\n", ch );
    save_char_obj(victim);
    do_quit( victim, "" );

    return;
}


void do_disconnect( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )    {
	send_to_char( "Disconnect whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )    {
	send_to_char( "You failed.\r\n", ch );
	return;
    }
    
    if ( victim->desc == NULL )    {
	act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
	return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )    {
	if ( d == victim->desc )	{
                    send_to_char ("You have been disconnected.\r\n",d->character);
	    close_socket(d, TRUE);
	    send_to_char( "Ok.\r\n", ch );
	    return;
	}
    }

    bug( "Do_disconnect: desc not found.", 0 );
    send_to_char( "Descriptor not found!\r\n", ch );
    return;
}


void do_pardon( CHAR_DATA *ch, char *argument ) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )    {
	send_to_char( "Syntax: pardon <character> <annoying|pk>.\r\n", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( IS_NPC(victim) )    {
	send_to_char( "Not on NPC's.\r\n", ch );
	return;
    }

    if ( !str_cmp( arg2, "annoying" ) )    {
	if ( IS_SET(victim->plr, PLR_ANNOYING) )	{
	    REMOVE_BIT( victim->plr, PLR_ANNOYING );
	    send_to_char( "Annoying flag removed.\r\n", ch );
	    send_to_char( "You are no longer ANNOYING.\r\n", victim );
	}
	return;
    }

    if ( !str_cmp( arg2, "pk" ) )    {
	if ( IS_SET(victim->plr, PLR_PK) )
	{
	    REMOVE_BIT( victim->plr, PLR_PK );
	    send_to_char( "pk flag removed.\r\n", ch );
	    send_to_char( "Thank the gods pk removed!\r\n", victim);
	}
	return;
    }

    if ( !str_cmp( arg2, "addpk" ) )    {
	SET_BIT (victim->plr, PLR_PK );
	send_to_char( "pk set\r\n", ch );
	send_to_char( "you can now PKILL!\r\n", victim);
    }

    send_to_char( "Syntax: pardon <character> <killer|thief|pk>.\r\n", ch );
    return;
}


void do_setannoying( CHAR_DATA *ch, char *argument ) {
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0')     {
	send_to_char( "Syntax: setannoying <character>\r\n", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( IS_NPC(victim) )    {
	send_to_char( "Not on NPC's.\r\n", ch );
	return;
    }

    if (get_divinity(ch) > get_divinity(victim)) {
        if (IS_SET(victim->plr, PLR_ANNOYING)) {
             send_to_char( "Annoying flag already set.\r\n", ch );
        } else {
             SET_BIT( victim->plr, PLR_ANNOYING);
             send_to_char( "Annoying flag set.\r\n", ch );
             send_to_char( "You seem to be annoying the Gods.\r\n", victim );
        }
    } else {
        SET_BIT( ch->plr, PLR_ANNOYING );
       send_to_char( "How DARE you!.\r\n", ch );
       sprintf(buf,"%s tried to set you annoying.\r\n",ch->name);
       send_to_char( buf, victim );
    }
    return;
}


void do_quitannoying( CHAR_DATA *ch, char *argument ) {
    char arg1[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0')     {
	send_to_char( "Syntax: quitannoying <character>\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if (IS_NPC(victim)) {
	send_to_char( "Not on a NPC!\r\n", ch );
	return;
    }

    if (!IS_SET(victim->plr,PLR_ANNOYING)) {
	send_to_char( "But that player isn't annoying.\r\n", ch );
	return;
    }
          
    send_to_char( "You annoy people too much, it seems.!\r\n", victim );
    send_to_char( "OK.\r\n", ch );
    save_char_obj(victim);
    do_quit( victim, "" );
    return;
}

/* Move a character to their devoured location... */ 

void dev_port(CHAR_DATA *ch, CHAR_DATA *devourer) {  

  ROOM_INDEX_DATA *belly;

  char log_buf[MAX_STRING_LENGTH];
  char vb[MAX_STRING_LENGTH];

 /* NPCs get a short ride... */

  if (IS_NPC(ch)) {
    raw_kill(ch, FALSE);

   /* Notify message to the other wizards... */

    format_vnum(devourer->in_room->vnum, vb);

    sprintf( log_buf, "Mob %s devoured by %s at %s",
   		     ch->short_descr,
 	             devourer->name,  
		     vb);

    notify_message (NULL, WIZNET_MOBDEATH, TO_IMM, log_buf	);

    return;
  }

 /* A little penalty... */

  if (get_sanity(ch) > 16) ch->sanity -= dice(1,4);

 /* Where are we going? */

  belly = find_location(devourer, "20030"); 

  if (belly == NULL) {
    send_to_char("{yOuugh! Yuck! What a revolting taste!{x\r\n", devourer);
    act("{m$n spits you out!{x", devourer, NULL, ch, TO_VICT);
    act("{m$n spits $N out!{x", devourer, NULL, ch, TO_NOTVICT);
    return;
  }

 /* Stop fighting... */

  if ( ch->fighting != NULL )
    stop_fighting( ch, TRUE );

 /* Teleport */

  char_from_room( ch );
  char_to_room( ch, belly );

 /* Tell the character they have arrived... */

  if (ch != devourer) {
    act("{m$n burps loudly!{x", devourer, NULL, NULL, TO_ROOM);
    send_to_char("{yYum yum. Nice and crunchy.{x\r\n", devourer); 
  }

  send_to_char("\r\n{cYou awaken somewhere strange...{x\r\n", ch);

  gain_exp(ch, 5, FALSE);

 /* Notify message to the other wizards... */

  format_vnum(devourer->in_room->vnum, vb);

  sprintf( log_buf, "%s devoured by %s at %s",
   		     ch->short_descr,
 	             devourer->name,  
		     vb);

  notify_message (NULL, WIZNET_DEATH, TO_IMM, log_buf	);

 /* All done... */

  return; 
}


void do_devour(CHAR_DATA *ch, char *argument) {

  CHAR_DATA *victim;

 /* Must devour something... */
 
  if (argument[0] == '\0') {
    send_to_char("Syntax: devour snack\r\n", ch);
    return;
  } 

 /* First we see if it is a character... */ 

  victim = get_char_room(ch, argument);

  if (victim != NULL) {

    if (victim == ch) {
      act("{m$n devours himself!{x", ch, NULL, NULL, TO_ROOM);
      send_to_char("{yMuch to the amazement of all, you devour yourself!{x\r\n", ch);      
      dev_port(victim, ch);
 
      return;
    }

    if (get_divinity(ch) < get_divinity(victim)) {

      act("{R$N is stronger than you are and devours you instead!{x", 
                                            ch, NULL, victim, TO_CHAR);
                  
      act("{y$n tries to devours you, but you are stronger!{x", 
                                            ch, NULL, victim, TO_VICT);
                 
      act("{m$n tries to devour $N, but $N is stronger and devours $n!{x", 
                                            ch, NULL, victim, TO_NOTVICT);
                  
      dev_port(ch, victim);

      return; 
    }

    act("{yYou noisily devour $N!{x", 
                                          ch, NULL, victim, TO_CHAR);
                  
    act("{R$n grabs you and devours you, crunching up your bones!{x", 
                                          ch, NULL, victim, TO_VICT);
                 
    act("{m$n grabs $N and noisily devours them!{x", 
                                          ch, NULL, victim, TO_NOTVICT);
                  
    dev_port(victim, ch); 

    return;
  }

  send_to_char("You can't devour that!\r\n", ch);

  return;
}


void do_echo( CHAR_DATA *ch, char *argument ) {
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )    {
	send_to_char( "Global echo what?\r\n", ch );
	return;
    }
    
    for ( d = descriptor_list; d; d = d->next )    {
	if ( d->connected == CON_PLAYING )
	{
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char( "global> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\r\n",   d->character );
	}
    }

    return;
}



void do_recho( CHAR_DATA *ch, char *argument ) {
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' ) {
	send_to_char( "Local echo what?\r\n", ch );

	return;
    }

    for ( d = descriptor_list; d; d = d->next )    {
	if ( d->connected == CON_PLAYING
	&&   d->character->in_room == ch->in_room )	{
            if (get_trust(d->character) >= get_trust(ch))
                send_to_char( "local> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\r\n",   d->character );
	}
    }

    return;
}


void do_pecho( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);
 
    if ( argument[0] == '\0' || arg[0] == '\0' )    {
	send_to_char("Personal echo what?\r\n", ch); 
	return;
    }
   
    if  ( (victim = get_char_world(ch, arg) ) == NULL )    {
	send_to_char("Target not found.\r\n",ch);
	return;
    }

    if (get_trust(victim) >= get_trust(ch) && get_trust(ch) != MAX_LEVEL)
        send_to_char( "personal> ",victim);

    send_to_char(argument,victim);
    send_to_char("\r\n",victim);
    send_to_char( "personal> ",ch);
    send_to_char(argument,ch);
    send_to_char("\r\n",ch);
}


ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg ) {
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) ) return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )	return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL ) return obj->in_room;

    return NULL;
}


void do_transfer( CHAR_DATA *ch, char *argument ) {
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
ROOM_INDEX_DATA *location, *old_room;
DESCRIPTOR_DATA *d;
CHAR_DATA *victim, *rch, *next_rch;
char buf[256];

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )    {
	send_to_char( "Transfer whom (and where)?\r\n", ch );
	return;
    }

    if ( !str_cmp( arg1, "all" ) )    {
	for ( d = descriptor_list; d != NULL; d = d->next )	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room != NULL
	    &&   can_see( ch, d->character ) )    {
		char buf[MAX_STRING_LENGTH];
		sprintf( buf, "%s %s", d->character->name, arg2 );
		do_transfer( ch, buf );
	    }
	}
	return;
    }

    if ( arg2[0] == '\0' )    {
	location = ch->in_room;
    }    else    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )	{
	    send_to_char( "No such location.\r\n", ch );
	    return;
	}

	if ( room_is_private(NULL, location ) && get_trust(ch) < MAX_LEVEL) {
	    send_to_char( "That room is private right now.\r\n", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( victim->in_room == NULL )    {
	send_to_char( "They are in limbo.\r\n", ch );
	return;
    }

    if (!IS_NPC(victim) && get_trust(victim) > get_trust(ch))  {
		sprintf (buf, "%s is too powerful to transfer.\r\n", victim->name);
		send_to_char (buf, ch);
		return;
    }

    old_room = victim->in_room;

    if ( victim->fighting != NULL ) {
	stop_fighting( victim, TRUE );
    }

    act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM );

    char_from_room( victim );
    char_to_room( victim, location );

    act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );

    if ( ch != victim ) {
	act( "$n has transferred you.", ch, NULL, victim, TO_VICT );
    }

    do_look( victim, "auto" );

   /* Transfer pets as well... */

    if (old_room != NULL) {
      for (rch = old_room->people; rch != NULL; rch = next_rch) {

        next_rch = rch->next_in_room;

        if ( rch->in_room == old_room 
          && rch->master == victim ) { 
          do_goto(rch, argument);
        }
      }
    }

    send_to_char( "Ok.\r\n", ch );
}



void do_at( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA *wch;
    
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )   {
	send_to_char( "At where what?\r\n", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )    {
	send_to_char( "No such location.\r\n", ch );
	return;
    }

    if (!IS_IMP(ch)) {
        if ( room_is_private(ch, location ) && get_trust(ch) < MAX_LEVEL)    {
	send_to_char( "That room is private right now.\r\n", ch );
	return;
        }
    }

    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != NULL; wch = wch->next )    {
	if ( wch == ch )	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    break;
	}
    }

    return;
}


void do_goto( CHAR_DATA *ch, char *argument ) {
ROOM_INDEX_DATA *location;
CHAR_DATA *rch, *next_rch;
ROOM_INDEX_DATA *old_room; 
FILE *fp;

    if ( argument[0] == '\0' )    {
	send_to_char( "Goto where?\r\n", ch );
	return;
    }

    if (argument[0] =='@' && !IS_NPC(ch)) {
       CHAR_DATA *partner;
       char arg1[MAX_INPUT_LENGTH];
       char buf[MAX_STRING_LENGTH];
       int i, rn;
      
       argument = one_argument(argument, arg1);
  
       for (i=1; i <= MAX_INPUT_LENGTH; i++) {
            arg1[i-1] = arg1[i];
       }

       if ((partner = get_char_world(ch, arg1)) == NULL) {
            send_to_char( "Partner not found.\r\n", ch );
            return;
       }

       rn = am_i_partner(partner);
       if (rn < 0) {
            send_to_char( "Partner not found.\r\n", ch );
            return;
       }

       if (ch->carrying) drop_artifact(ch, ch->carrying);
       if (ch->carrying) lock_away_incompatible(ch, ch->carrying);
       if (ch->carrying) set_index(ch, ch->carrying);

       if (ch->ooz) drop_artifact(ch, ch->ooz);
       if (ch->ooz) lock_away_incompatible(ch, ch->ooz);
       if (ch->ooz) set_index(ch, ch->ooz);

       if (ch->pet) stop_follower(ch->pet);
       char_from_room(ch);
       char_to_room(ch, get_room_index(1));
       save_char_obj(ch);

       sprintf(buf, "%s%s.gz", PLAYER_DIR, capitalize(ch->name));
       if ((fp = fopen(buf, "r" )) != NULL) {
          fclose(fp);
          transmove_player(ch, partner, 0);
          sprintf(buf, "muddelete %s", ch->name);
          enqueue_mob_cmd(partner, buf, 15, 0);
          return;
       }
    }

    if ( ( location = find_location( ch, argument ) ) == NULL )    {
	send_to_char( "No such location.\r\n", ch );
	return;
    }

    if (!IS_IMP(ch)) {
        if ( room_is_private(ch, location ) && get_trust(ch) < MAX_LEVEL)    {
	send_to_char( "That room is private right now.\r\n", ch );
	return;
        }
    }

    if ( ch->fighting != NULL ) {
	stop_fighting( ch, TRUE );
    } 

    if (ch->in_room != NULL) {
      for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)      {
	if (get_trust(rch) >= ch->invis_level)	{
	    if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')	act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
	    else	act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
	}
      }
    }

    old_room = ch->in_room;

    char_from_room( ch );
    char_to_room( ch, location );


    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }

    do_look( ch, "auto" );

   /* Transfer pets as well... */

    if (old_room != NULL) {
      for (rch = old_room->people; rch != NULL; rch = next_rch) {

        next_rch = rch->next_in_room;

        if ( rch->in_room == old_room 
          && rch->master == ch ) { 
          do_goto(rch, argument);
        }
      }
    }

    return;
}


void do_stat ( CHAR_DATA *ch, char *argument ){
   char arg[MAX_INPUT_LENGTH];
   char *string;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *location;
   CHAR_DATA *victim;

   string = one_argument(argument, arg);
   if ( arg[0] == '\0')   {
	send_to_char("Syntax:\r\n",ch);
	send_to_char("  stat <name>\r\n",ch);
	send_to_char("  stat obj <name>\r\n",ch);
	send_to_char("  stat mob <name>\r\n",ch);
 	send_to_char("  stat room <number>\r\n",ch);
	send_to_char("  stat skill <name>\r\n",ch);
	return;
   }
   
   if (!str_cmp(arg,"skill"))
   {
	do_skillstat(ch,string);
	return;
   }

   if (!str_cmp(arg,"room"))
   {
	do_rstat(ch,string);
	return;
   }
  
   if (!str_cmp(arg,"obj")) {
	do_ostat(ch,string);
	return;
   }

   if(!str_cmp(arg,"char")  || !str_cmp(arg,"mob"))
   {
	do_mstat(ch,string);
	return;
   }
   
   /* do it the old way */

   obj = get_obj_world(ch,argument);
   if (obj != NULL) {
     do_ostat(ch,argument);
     return;
   }

  victim = get_char_world(ch,argument);
  if (victim != NULL)  {
    do_mstat(ch,argument);
    return;
  }

  location = find_location(ch,argument);
  if (location != NULL)
  {
    do_rstat(ch,argument);
    return;
  }

  send_to_char("Nothing by that name found anywhere.\r\n",ch);
}

void do_skillstat (CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int sn, spn, afn, efn;

    buf[0] = '\0';

    if (argument[0] == '\0')
    {
        send_to_char ("Syntax:  stat skill/spell <name> \r\n",ch);
        return;
    }

    sn = get_skill_sn(argument);

    if ( sn == SKILL_UNDEFINED ) {
	send_to_char("There is no such skill as that.\r\n",ch);
	return;
    }

    sprintf(buf, "{WStats for [{Y%s{W] - SN [{G%4d{W]\r\n",
	    skill_array[sn].name,
	    sn);
    send_to_char(buf,ch);

    sprintf(buf, "Group    : [{Y%4d{W]\r\n", skill_array[sn].group );
    send_to_char(buf,ch);
    sprintf(buf, "Position : [{Y%4d{W]\r\n", skill_array[sn].minpos );
    send_to_char(buf,ch);
    sprintf(buf, "Wait     : [{Y%4d{W]\r\n", skill_array[sn].beats );
    send_to_char(buf,ch);

    if (skill_array[sn].stats[0] != STAT_NONE) {
   
      sprintf(buf, "Stats    : [{Y");
 
      switch (skill_array[sn].stats[0]) {
        case STAT_STR:
          strcat(buf, "STR");
          break;
        case STAT_CON:
          strcat(buf, "CON");
          break;
        case STAT_DEX:
          strcat(buf, "DEX");
          break;
        case STAT_INT:
          strcat(buf, "INT");
          break;
        case STAT_WIS:
          strcat(buf, "WIS");
          break;
        default:
          strcat(buf, "{R???{Y");
          break;
      }  

      if (skill_array[sn].stats[1] != STAT_NONE) {
        
        switch (skill_array[sn].stats[0]) {
          case STAT_STR:
            strcat(buf, ", STR");
            break;
          case STAT_CON:
            strcat(buf, ", CON");
            break;
          case STAT_DEX:
            strcat(buf, ", DEX");
            break;
          case STAT_INT:
            strcat(buf, ", INT");
            break;
          case STAT_WIS:
            strcat(buf, ", WIS");
            break;
          default:
            strcat(buf, ", {R???{Y");
            break;
        }  
      } 

      strcat(buf, "{W]\r\n");
      send_to_char(buf, ch);
    }

    if (skill_array[sn].learnable) {
      sprintf(buf, "Learn    : [{Y%4d{W]\r\n", skill_array[sn].learn );
      send_to_char(buf,ch);

      print_econd(ch, skill_array[sn].ec, "{WCondition - {Y");

      send_to_char("{W", ch);

    }

    spn = get_spell_spn(argument);

    if ( spn != SPELL_UNDEFINED) {
      sprintf(buf, "Spell    : [{Y%s{W]\r\n", spell_array[spn].name );
      send_to_char(buf,ch);
      sprintf(buf, "Targets  : [{Y%4d{W]\r\n", spell_array[spn].target );
      send_to_char(buf,ch);
      sprintf(buf, "Mana     : [{Y%4d{W]\r\n", spell_array[spn].mana );
      send_to_char(buf,ch);

      if ( spell_array[spn].component != NULL) {
        sprintf(buf, "Component: [{Y%s{W]\r\n", spell_array[spn].component );
      } else {
        sprintf(buf, "Component: [{Ynone{W]\r\n");
      } 
      send_to_char(buf,ch);

      efn = spell_array[spn].effect;
      sprintf(buf, "Effect   : [{Y%3d{W\r\n", efn );

      sprintf(buf, "Noun     : [{Y%s{W]\r\n", effect_array[efn].noun_damage );
      send_to_char(buf,ch);
      sprintf(buf, "Slot     : [{Y%4d{W]\r\n", effect_array[efn].slot );
      send_to_char(buf,ch);

      afn = get_affect_afn_for_effect(efn);

      if (afn != AFFECT_UNDEFINED) {

        sprintf(buf, "Affect   : [{Y%s{W]\r\n", affect_array[afn].name );
        send_to_char(buf,ch);

        sprintf(buf, "Wearoff  : [{Y%s{W]\r\n", affect_array[afn].end_msg );
        send_to_char(buf,ch);

      }
    }

    sprintf(buf, "{x");
    send_to_char(buf, ch);

    return;
} /* end of skillstat */


void do_rstat( CHAR_DATA *ch, char *argument ) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char vb[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;
    MONITORING_LIST *monitor;
    AREA_DATA *area;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )    {
	send_to_char( "No such location.\r\n", ch );
	return;
    }

    if ( ch->in_room != location && room_is_private(ch, location ) && get_trust(ch) < MAX_LEVEL) {
	send_to_char( "That room is private right now.\r\n", ch );
	return;
    }

    area = location->area;

    sprintf( buf, "Name: '%s'  SubArea: %d\r\n", location->name, location->subarea );
    send_to_char( buf, ch );

    if (area == NULL) {
      send_to_char("{yNot in an area!\r\n", ch);
    } else {
      format_vnum(area->vnum, vb);
      sprintf( buf, "Area - %s : '%s'  Zone - %d : '%s'\r\n", vb, area->name, area->zone,  zones[area->zone].name);
      send_to_char( buf, ch );

      sprintf( buf, "Climate - %s  Weather - %d  Old - %d  Surface - %d\r\n", 
                     flag_string( area_climate, area->climate ), area->weather,  area->old_weather, area->surface );
      send_to_char( buf, ch );

    } 

    // Special rooms

    format_vnum(location->recall, vb);
    sprintf( buf, "Room - Recall: %s", vb);
    send_to_char( buf, ch ); 

    format_vnum(location->respawn, vb);
    sprintf( buf, "  Respawn: %s", vb);
    send_to_char( buf, ch ); 

    format_vnum(location->morgue, vb);
    sprintf( buf, "  Morgue: %s", vb);
    send_to_char( buf, ch ); 

    format_vnum(location->dream, vb);
    sprintf( buf, "  Dream: %s\r\n", vb);
    send_to_char( buf, ch ); 

    if (area != NULL) {

      // Area rooms

      format_vnum(area->recall, vb);
      sprintf( buf, "Area - Recall: %s", vb);
      send_to_char( buf, ch ); 

      format_vnum(area->respawn, vb);
      sprintf( buf, "  Respawn: %s", vb);
      send_to_char( buf, ch ); 

      format_vnum(area->morgue, vb);
      sprintf( buf, "  Morgue: %s", vb);
      send_to_char( buf, ch ); 

      format_vnum(area->dream, vb);
      sprintf( buf, "  Dream: %s\r\n", vb);
      send_to_char( buf, ch ); 

      // Zone rooms	

      format_vnum(zones[area->zone].recall, vb);
      sprintf( buf, "Zone - Recall: %s", vb);
      send_to_char( buf, ch ); 

      format_vnum(zones[area->zone].respawn, vb);
      sprintf( buf, "  Respawn: %s", vb);
      send_to_char( buf, ch ); 

      format_vnum(zones[area->zone].morgue, vb);
      sprintf( buf, "  Morgue: %s", vb);
      send_to_char( buf, ch ); 

      format_vnum(zones[area->zone].dream, vb);
      sprintf( buf, "  Dream: %s\r\n", vb);
      send_to_char( buf, ch ); 

    }

    format_vnum(location->night, vb);
    sprintf( buf, "Night: %s", vb);
    send_to_char( buf, ch ); 

    format_vnum(location->day, vb);
    sprintf( buf, "  Day: %s\r\n", vb);
    send_to_char( buf, ch ); 

    sprintf( buf, "Vnum: %ld  Sector: %d  Light: %d  Heal: %d  Mana: %d\r\n", location->vnum, location->sector_type, location->light, get_heal_rate(location), get_mana_rate(location));
    send_to_char( buf, ch );

    sprintf( buf, "Room flags: %s.\r\nDescription:\r\n%s",flag_string(room_flags, location->room_flags), location->description );
    send_to_char( buf, ch );

    if ( location->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );
	for ( ed = location->extra_descr; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}
	send_to_char( "'.\r\n", ch );
    }

    send_to_char( "Characters:", ch );
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
	if (can_see(ch,rch))
        {
	    send_to_char( " ", ch );
	    one_argument( rch->name, buf );
	    send_to_char( buf, ch );
	}
    }

    send_to_char( ".\r\nObjects:   ", ch );
    if (location->contents == NULL) {
      send_to_char(" None.\r\n", ch);
    } else {
      for ( obj = location->contents; obj; obj = obj->next_content )
      {
  	  send_to_char( " ", ch );
	  one_argument( obj->name, buf );
	  send_to_char( buf, ch );
      }
      send_to_char( ".\r\n", ch );
    }

    send_to_char("Monitors:  ", ch);

    monitor = location->monitors;
 
    if (monitor == NULL) {
      send_to_char(" None.\r\n", ch);
    } else {
      while (monitor != NULL) {

        if (monitor->monitor->in_room == NULL) {
          sprintf(vb, "0");
        } else {
          format_vnum(monitor->monitor->in_room->vnum, vb);
        }

        sprintf(buf, " %s (%s)",
                       monitor->monitor->short_descr,
                       vb);
        send_to_char(buf, ch);
 
        monitor = monitor->next; 
      }
      send_to_char(".\r\n", ch);
    }

    for ( door = 0; door < DIR_MAX; door++ ) {

	EXIT_DATA *pexit;

	if ( ( pexit = location->exit[door] ) != NULL ) {

            if (pexit->u1.to_room == NULL) {
              sprintf(vb, "-1");
            } else {
              format_vnum(pexit->u1.to_room->vnum, vb);
            } 

	    sprintf( buf, "Door: %d  To: %s  Key: %ld  Exit flags: %d\r\n"
                          "Keyword: '%s'  Description: %s",
		           door,
                           vb,
	    	           pexit->key,
	    	           pexit->exit_info,
	    	           pexit->keyword,
	    	           pexit->description[0] != '\0'
		                           ? pexit->description : "(none).\r\n" );
	    send_to_char( buf, ch );
	}
    }

    return;
}


void do_ostat( CHAR_DATA *ch, char *argument ) {
char vb[MAX_STRING_LENGTH];
char arg[MAX_INPUT_LENGTH];
AFFECT_DATA *paf;
OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' ) {
	send_to_char( "Stat what?\r\n", ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, argument ) ) == NULL ) {
	send_to_char( "Nothing like that in hell, earth, or heaven.\r\n", ch );
	return;
    }

    sprintf_to_char(ch, "Name(s): %s\r\n", obj->name );

    format_vnum(obj->pIndexData->vnum, vb);
    sprintf_to_char(ch, "Vnum: %s / %d  Format: %s  Type: %s  Resets: %d  Size: %s\r\n", vb, obj->orig_index, obj->pIndexData->new_format ? "new" : "old",
                   item_type_name(obj), obj->pIndexData->reset_num , (obj->size >=0 && obj->size <= 5) ? size_table[obj->size] : "unknown");

    sprintf_to_char(ch, "Short description: %s\r\nLong description: %s\r\n", obj->short_descr, obj->description );
    sprintf_to_char(ch, "Zones: %s\r\n", flag_string( zmask_flags, obj->zmask));  
	
    sprintf_to_char (ch, "Material: %s\r\n", material_name (obj->material));
    sprintf_to_char(ch, "Wear bits: %s\r\nExtra bits: %s\r\n", wear_bit_name(obj->wear_flags), extra_bit_name( obj->extra_flags ) );

    sprintf_to_char(ch, "Number: 1/%d  Weight: %d/%d\r\n", get_obj_number(obj), obj->weight, get_obj_weight( obj ) );
    sprintf_to_char(ch, "Level: %d  Cost: %d  Condition: %d  Timer: %d\r\n", obj->level, obj->cost, obj->condition, obj->timer );

    if (obj->in_room == NULL) {
      sprintf(vb, "(none)");
    } else {
      format_vnum(obj->in_room->vnum, vb);
    }
    sprintf_to_char(ch, "In room: %s  In object: %s  Carried by: %s  Wear_loc: %d\r\n", vb, obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
	           obj->carried_by == NULL    ? "(none)" : can_see(ch,obj->carried_by) ? obj->carried_by->name : "someone", obj->wear_loc );
    
    sprintf_to_char(ch, "Orig Values: %ld %ld %ld %ld %ld\r\n", obj->valueorig[0], obj->valueorig[1], obj->valueorig[2], obj->valueorig[3], obj->valueorig[4] );
    sprintf_to_char(ch, "Values:      %ld %ld %ld %ld %ld\r\n", obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4] );
    
    /* now give out vital statistics as per identify */
    
    switch ( obj->item_type )  {
    	case ITEM_SCROLL: 
    	case ITEM_POTION:
    	case ITEM_PILL:
	    sprintf_to_char(ch, "Level %ld spells of:", obj->value[0] );

	    if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )  {
	    	send_to_char( " '", ch );
	    	send_to_char( effect_array[obj->value[1]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )  {
	    	send_to_char( " '", ch );
	    	send_to_char( effect_array[obj->value[2]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )  {
	    	send_to_char( " '", ch );
	    	send_to_char( effect_array[obj->value[3]].name, ch );
	    	send_to_char( "'", ch );
	    }
  	    send_to_char( ".\r\n", ch );
	    break;

    	case ITEM_WAND: 
    	case ITEM_STAFF: 
	    sprintf_to_char(ch, "Has %ld(%ld) charges of level %ld", obj->value[1], obj->value[2], obj->value[0] );
      
	    if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL ) {
	    	send_to_char( " '", ch );
	    	send_to_char( effect_array[obj->value[3]].name, ch );
	    	send_to_char( "'", ch );
	    }
	    send_to_char( ".\r\n", ch );
   	    break;
      
    	case ITEM_WEAPON:
 	    send_to_char("Weapon type is ",ch);
	    switch (obj->value[0]) {
	    	case(WEAPON_EXOTIC): 
			send_to_char("exotic\r\n",ch);
                                    	break;
	    	case(WEAPON_SWORD): 
			send_to_char("sword\r\n",ch);	
			break;	
	    	case(WEAPON_DAGGER): 
			send_to_char("dagger\r\n",ch);	
			break;
	    	case(WEAPON_SPEAR): 
			send_to_char("spear\r\n",ch);
			break;
	    	case(WEAPON_MACE): 
			send_to_char("mace\r\n",ch);
			break;
	   	case(WEAPON_AXE): 
			send_to_char("axe\r\n",ch);
			break;
	    	case(WEAPON_FLAIL): 
			send_to_char("flail\r\n",ch);
			break;
	    	case(WEAPON_WHIP): 
			send_to_char("whip\r\n",ch);		
			break;
	    	case(WEAPON_POLEARM): 
			send_to_char("polearm\r\n",ch);	
			break;
		case(WEAPON_GUN): 
			send_to_char("gun\r\n", ch);
			break;
		case(WEAPON_BOW): 
			send_to_char("bow\r\n", ch);
			break;
		case(WEAPON_HANDGUN): 
			send_to_char("handgun\r\n", ch);
			break;
		case(WEAPON_SMG): 
			send_to_char("submachinegun\r\n", ch);
			break;
	    	default: 
			send_to_char("unknown\r\n",ch);	
			break;
 	    }
	    if (obj->pIndexData->new_format) sprintf_to_char(ch,"Damage is %ldd%ld (average %ld)\r\n", obj->value[1],obj->value[2], (1 + obj->value[2]) * obj->value[1] / 2);
	    else sprintf_to_char(ch, "Damage is %ld to %ld (average %ld)\r\n", obj->value[1], obj->value[2], ( obj->value[1] + obj->value[2] ) / 2 );

                    send_to_char("Damage type: ", ch);
                    send_to_char(flag_string(weapon_flags, obj->value[3]), ch);
                    send_to_char("\r\n", ch);

	    if (obj->value[4])  sprintf_to_char(ch,"Weapons flags: %s\r\n",weapon_bit_name(obj->value[4]));
       	    break;
  
    	case ITEM_ARMOR:
	    sprintf_to_char(ch, "Armor class is %ld pierce, %ld bash, %ld slash, and %ld vs. magic\r\n", obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	break;
    }

   /* Show extra description keywords... */

    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL ) {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: [", ch );

	for ( ed = obj->extra_descr; ed != NULL; ed = ed->next ) {
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL ) send_to_char( "] [", ch );
	}

	for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next ) {
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL ) send_to_char( "] [", ch );
	}
	send_to_char( "]\r\n", ch );
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next ) {
	if (!paf->bitvector ) {
                    sprintf_to_char(ch, "Affects %s by %d, level %d", affect_loc_name( paf->location ), paf->modifier, paf->level );
                    if (paf->duration > -1) sprintf_to_char(ch, " (%d).\r\n", paf->duration);
                    else send_to_char(".\r\n",ch);
                } else {
                    sprintf_to_char(ch, "Spell affect with bits [%s] for %d hours.\r\n", affect_bit_name(paf->bitvector), paf->duration );
                }
    }

    if (!obj->enchanted) {
        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next ) {
	if (!paf->bitvector) {
                    sprintf_to_char(ch, "Affects %s by %d, level %d", affect_loc_name( paf->location ), paf->modifier,paf->level );
                    if (paf->duration > -1) sprintf_to_char(ch, " (%d).\r\n", paf->duration);
                    else send_to_char(".\r\n",ch);
	} else {
                    sprintf_to_char(ch, "Spell affect with bits [%s] for %d hours.\r\n", affect_bit_name(paf->bitvector), paf->duration );
                }
        }
    }
    return;
}



void do_mstat( CHAR_DATA *ch, char *argument ) {
    char buf[MAX_STRING_LENGTH];
    char vb[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int i;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )    {
	send_to_char( "Stat whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if (IS_NPC(victim)) sprintf( buf, "Name: %s ", victim->name);
    else sprintf( buf, "Name: %s (from %s) ", victim->name, victim->pcdata->native );
    send_to_char( buf, ch );

    if (!IS_NPC(victim)) {

      if (IS_NEWBIE(victim)) 	send_to_char(" (Newbie)", ch); 
      if (IS_HUMAN(victim))		send_to_char(" (Player)", ch); 
      if (IS_HERO(victim)) 		send_to_char(" (Hero)", ch); 
      if (IS_CREATOR(victim)) 	send_to_char(" (Creator)", ch); 
      if (IS_LESSER(victim)) 		send_to_char(" (Lesser God)", ch); 
      if (IS_GOD(victim)) 		send_to_char(" (God)", ch); 
      if (IS_GREATER(victim)) 	send_to_char(" (Greater God)", ch); 
      if (IS_IMP(victim)) 		send_to_char(" (Implementor)", ch); 
      if (IS_MUD(victim)) 		send_to_char(" (Mud-Handler)", ch); 

    }

    send_to_char("  True name: ", ch);
 
    if (victim->true_name != NULL) {
      send_to_char(victim->true_name, ch);
    } else {
      send_to_char("none", ch);
    }

    send_to_char("\r\n", ch);

    if ( victim->desc != NULL
      && victim->desc->original != NULL ) {
      sprintf(buf, "Posessed by: %s %s\r\n",  victim->desc->original->name,  victim->desc->original->pcdata->title); 
      send_to_char(buf, ch);
    } 

    if ( !IS_NPC(victim) ) {
      sprintf(vb, "Player");
    } else {
      format_vnum(victim->pIndexData->vnum, vb);
    }

    sprintf( buf, "Vnum: %s  Format: %s  Race: %s  Sex: %s  Size: %s",
	vb,
	IS_NPC(victim) ? victim->pIndexData->new_format ? "new" : "old" : "pc",
	race_array[victim->race].name,
	victim->sex == SEX_MALE    ? "male"   :
	victim->sex == SEX_FEMALE  ? "female" : "neutral",
                flag_string(size_flags, get_char_size(victim)));
    send_to_char( buf, ch );

    if ( victim->in_room == NULL ) {
      sprintf(vb, "Limbo");
    } else {
      format_vnum(victim->in_room->vnum, vb);
    }

    sprintf( buf, "  Room: %s\r\n", vb);
    send_to_char( buf, ch );

    if ( victim->waking_room != NULL ) {
      format_vnum(victim->waking_room->vnum, vb);
      if (victim->nightmare == FALSE) {
         sprintf(buf, "Dreaming (from room %s : %s)\r\n", vb, victim->waking_room->name);
      } else {
         sprintf(buf, "Nightmare (from room %s : %s)\r\n", vb, victim->waking_room->name);
      }
      send_to_char(buf, ch);
    }
 
    if ( victim->dreaming_room != NULL ) {
      format_vnum(victim->dreaming_room->vnum, vb);
      sprintf(buf, "Awake (can return in dream to room %s : %s)\r\n", vb, victim->dreaming_room->name);
      send_to_char(buf, ch);
    }
 
    sprintf( buf, "Race number: %d  Version: %d\r\n", victim->race, victim->version);
    send_to_char( buf, ch);

    if (IS_NPC(victim))   {
	sprintf(buf, "Count: %d  Killed: %d  Spawned: %ld\r\n", victim->pIndexData->count, victim->pIndexData->killed, victim->recall_perm);
	send_to_char(buf,ch);
    }

    sprintf(buf, "Nature: %s\r\n",nature_bit_name(victim->nature));
    send_to_char(buf,ch);
    
    sprintf( buf, "Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)\r\nCon: %d(%d) Luck: %d(%d)  Cha: %d(%d)\r\n",
	victim->perm_stat[STAT_STR],
	get_curr_stat(victim,STAT_STR),
	victim->perm_stat[STAT_INT],
	get_curr_stat(victim,STAT_INT),
	victim->perm_stat[STAT_WIS],
	get_curr_stat(victim,STAT_WIS),
	victim->perm_stat[STAT_DEX],
	get_curr_stat(victim,STAT_DEX),
	victim->perm_stat[STAT_CON],
	get_curr_stat(victim,STAT_CON),
	victim->perm_stat[STAT_LUC],
	get_curr_stat(victim,STAT_LUC),
	victim->perm_stat[STAT_CHA],
	get_curr_stat(victim,STAT_CHA));
    send_to_char( buf, ch );

    sprintf( buf, "Hp: %d/%d  Mana: %d/%d  Move: %d/%d  Sanity: %d\r\n",
	victim->hit,         victim->max_hit,
	victim->mana,        victim->max_mana,
	victim->move,        victim->max_move,
	victim->sanity );
    send_to_char( buf, ch );

    if (!IS_NPC(ch)) {	
      sprintf( buf, "Practices: %d  Train: %d\r\n", victim->practice, victim->train );
      send_to_char( buf, ch );
    }
	
    sprintf( buf,
	"Lv: %d  Class: %s  Align: %d  Exp: %d\r\n",
	victim->level,       
	IS_NPC(victim) ? "mobile" : ((victim->profs == NULL) ? class_table[victim->pc_class].name : "Professional"),            
	victim->alignment, victim->exp );
    send_to_char( buf, ch );

    sprintf(buf, "Cash: %10ld / %10ld / %10ld / %10ld / %10ld\r\n", victim->gold[0], victim->gold[1], victim->gold[2], victim->gold[3], victim->gold[4]);
    send_to_char( buf, ch );

    if (victim->profs != NULL) {
      LPROF_DATA *lprof;

      lprof = victim->profs;
 
      while (lprof != NULL) {
        
        sprintf(buf, "Profession: %s, level %d\r\n",
                     lprof->profession->name,
                     lprof->level);
        send_to_char(buf, ch);

        lprof = lprof->next;
      }
    }

    if ( victim->speak[SPEAK_PUBLIC] > 1
      && victim->speak[SPEAK_PRIVATE] > 1) {
     sprintf(buf, "Speaking: Public: %s  Private: %s\r\n",
                  skill_array[victim->speak[SPEAK_PUBLIC]].name,
                  skill_array[victim->speak[SPEAK_PRIVATE]].name);
     send_to_char(buf, ch);
    } else {
     send_to_char("Speaking: Settings corrupt!\r\n", ch);
    }

    sprintf(buf,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\r\n",
	    GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
	    GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));
    send_to_char(buf,ch);

    sprintf( buf, "Hit: %d  Dam: %d  Saves: %d  Position: %d  Wimpy: %d\r\n",
	GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throw,
	victim->position,    victim->wimpy );
    send_to_char( buf, ch );

    if (IS_NPC(victim) && victim->pIndexData->new_format)    {
	sprintf(buf, "Damage: %dd%d  Message:  %s  Damtype: %s\r\n",
	    victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE],
	    attack_table[victim->atk_type].name,
            damage_type[victim->dam_type].name );
	send_to_char(buf,ch);
    }

    sprintf( buf, "Fighting: %s\r\n", victim->fighting ? victim->fighting->name : "(none)" );
    send_to_char( buf, ch );

    if (IS_NPC(victim)) {
        sprintf_to_char(ch, "Carry number: %d  Carry weight: %d\r\n", victim->carry_number, victim->carry_weight);
    } else {
	sprintf_to_char(ch, "Food: %d  Drink: %d  Drunk: %d  Fat: %d\r\n", victim->condition[COND_FOOD], victim->condition[COND_DRINK], victim->condition[COND_DRUNK], victim->condition[COND_FAT] );
                sprintf_to_char(ch, "Carry number: %d  Carry weight: %d  Fame: %d\r\n", victim->carry_number, victim->carry_weight, victim->pcdata->questpoints);
    	sprintf_to_char(ch, "Age: %d  Played: %d  Last Level: %d  Timer: %d\r\n", get_age(victim),  (int) (victim->pcdata->played + current_time - victim->pcdata->logon) / 3600, victim->pcdata->last_level, victim->timer );
                sprintf_to_char(ch, "Lifespan: %d\r\n",victim->pcdata->lifespan);
    }
        
    sprintf_to_char(ch, "Act: %s\r\n", act_bit_name(victim->act));
    sprintf_to_char(ch, "Plr: %s\r\n", plr_bit_name(victim->plr));
   
    if (victim->comm) 			sprintf_to_char(ch,"Comm: %s\r\n",comm_bit_name(victim->comm));
    if (IS_NPC(victim) && victim->off_flags) 	sprintf_to_char(ch, "Offense: %s\r\n",off_bit_name(victim->off_flags));
    if (victim->imm_flags) 			sprintf_to_char(ch, "Immune: %s\r\n",imm_bit_name(victim->imm_flags));
    if (victim->envimm_flags) 		sprintf_to_char(ch, "Env-Immune: %s\r\n",imm_bit_name(victim->envimm_flags));
    if (victim->res_flags) 			sprintf_to_char(ch, "Resist: %s\r\n", imm_bit_name(victim->res_flags));
    if (victim->vuln_flags) 			sprintf_to_char(ch, "Vulnerable: %s\r\n", imm_bit_name(victim->vuln_flags));

    sprintf_to_char(ch, "Form: %s %s\r\nParts: %s\r\n", form_bit_name(victim->form), flag_string(material_type, victim->material), part_bit_name(victim->parts));

    if (victim->affected_by) 			sprintf_to_char(ch, "Affected by %s\r\n", affect_bit_name(victim->affected_by));

    sprintf_to_char(ch, "Master: %s  Leader: %s  Pet: %s\r\n",	victim->master      ? victim->master->name   : "(none)", victim->leader      ? victim->leader->name   : "(none)",	victim->pet 	    ? victim->pet->name	     : "(none)");

    if (!IS_NPC(victim)) {
      sprintf( buf, "Security: %d.\r\n", victim->pcdata->security );  /* OLC */
      send_to_char( buf, ch );                                        /* OLC */
    }

  /* Hunter/prey info... */

    sprintf( buf, "Hunting: %s  Hunted By: %s\r\n",
             victim->prey == NULL ? "(no one)" : victim->prey->short_descr,
             victim->huntedBy == NULL ? "(no one)" : victim->huntedBy->short_descr);
    send_to_char(buf, ch); 

    sprintf( buf, "Short description: %s\r\nLong  description: %s",
	victim->short_descr,
	victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\r\n" );
    send_to_char( buf, ch );

    if ( IS_NPC(victim) && victim->spec_fun != 0 )
	send_to_char( "Mobile has special procedure.\r\n", ch );

    sprintf(buf, "Chat state: %d\r\n", victim->chat_state);
    send_to_char(buf, ch);

    for(i = 0; i < MOB_MEMORY; i++) {

      if (victim->memory[i] != NULL) {
        sprintf(buf, "Memory slot %d: '%s'\r\n",
                     i, victim->memory[i]);
        send_to_char(buf, ch);
      }
    }

    return;
}

/* ofind and mfind replaced with vnum, vnum skill also added */

void do_vnum(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *string;

    string = one_argument(argument,arg);
 
    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\r\n",ch);
	send_to_char("  vnum obj <name>\r\n",ch);
	send_to_char("  vnum mob <name>\r\n",ch);
	send_to_char("  vnum skill <skill or spell>\r\n",ch);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_ofind(ch,string);
 	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    { 
	do_mfind(ch,string);
	return;
    }

    if (!str_cmp(arg,"skill") || !str_cmp(arg,"spell"))
    {
	do_slookup(ch,string);
	return;
    }
    /* do both */
    do_mfind(ch,argument);
    do_ofind(ch,argument);
}


void do_mfind( CHAR_DATA *ch, char *argument )
{
    extern int top_mob_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char vb[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find whom?\r\n", ch );
	return;
    }

    fAll	= FALSE; /* !str_cmp( arg, "all" ); */
    found	= FALSE;
    nMatch	= 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pMobIndex->player_name ) )
	    {
		found = TRUE;
                format_vnum(pMobIndex->vnum, vb);
		sprintf( buf, "[%11s] %s\r\n",
		               vb,
                               pMobIndex->short_descr );
		send_to_char( buf, ch );
	    }
	}
    }

    if ( !found )
	send_to_char( "No mobiles by that name.\r\n", ch );

    return;
}



void do_ofind( CHAR_DATA *ch, char *argument )
{
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char vb[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find what?\r\n", ch );
	return;
    }

    fAll	= FALSE; /* !str_cmp( arg, "all" ); */
    found	= FALSE;
    nMatch	= 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pObjIndex->name ) )
	    {
		found = TRUE;
                format_vnum(pObjIndex->vnum, vb);	
		sprintf( buf, "[%11s] %s\r\n",
		               vb,
                               pObjIndex->short_descr );
		send_to_char( buf, ch );
	    }
	}
    }

    if ( !found )
	send_to_char( "No objects by that name.\r\n", ch );

    return;
}

void do_mwhere( CHAR_DATA *ch, char *argument ) {
    char buf[MAX_STRING_LENGTH];
    char buffer[8*MAX_STRING_LENGTH];
    char vb[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    bool found;
    int count = 0;

    if ( argument[0] == '\0' )    {
	send_to_char( "Mwhere whom?\r\n", ch );
	return;
    }

    found = FALSE;
    buffer[0] = '\0';
    for ( victim = char_list; victim != NULL; victim = victim->next )    {
	if ( IS_NPC(victim)
	&&   victim->in_room != NULL
	&&   is_name( argument, victim->name ) 
                && count < 100){
                    count++;
	    found = TRUE;

            format_vnum(victim->pIndexData->vnum, vb); 
	    sprintf( buf, "[%11s] %-28s",
                          vb,
                          victim->short_descr);
	    strcat(buffer,buf);

            format_vnum(victim->in_room->vnum, vb); 
	    sprintf( buf, " [%11s] %s\r\n",
                          vb,
                          victim->in_room->name );
	    strcat(buffer,buf);
	}
    }

    if ( !found )
	act( "You didn't find any $T.", ch, NULL, argument, TO_CHAR );
    else
        page_to_char(buffer,ch);

    return;
}

void do_owhere (CHAR_DATA *ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    char buffer[8*MAX_STRING_LENGTH];
    char vb[MAX_STRING_LENGTH];
    char vb2[MAX_STRING_LENGTH];
    OBJ_DATA *search_obj;
    bool found;
    int count = 0;

    if ( argument[0] == '\0' )    {
	send_to_char( "Owhere what?\r\n", ch );
	return;
    }
	
    buffer[0] = '\0';
	sprintf (buffer, "{c[ vnum]  name                   location{x\r\n");
	strcat  (buffer, "----------------------------------------------------------------------\r\n");
 
    found = FALSE;
    for ( search_obj = object_list; search_obj != NULL; search_obj = search_obj->next )    {
	if (is_name_abbv( argument, search_obj->name )
                && count < 100) {
                                count++;
		if (search_obj->in_room != NULL )			{
                        format_vnum(search_obj->pIndexData->vnum, vb);
                        format_vnum(search_obj->in_room->vnum, vb2);
	   		sprintf( buf, "[%11s] %-20s  in room [%11s] %s\r\n",
                                       vb,
                                       search_obj->short_descr,
                                       vb2,
                                       search_obj->in_room->name );
			found=TRUE;
		} else if (search_obj->in_obj != NULL) {
                        format_vnum(search_obj->pIndexData->vnum, vb);
                        format_vnum(search_obj->in_obj->pIndexData->vnum, vb2);
			sprintf( buf, "[%11s] %-20s  inside  [%11s] %-20s\r\n",
                                       vb,
                                       search_obj->short_descr,
                                       vb2,
                                       search_obj->in_obj->short_descr);
			found=TRUE;
		}
		else if (search_obj->carried_by != NULL) {
                        format_vnum(search_obj->pIndexData->vnum, vb);
			sprintf( buf, "[%11s] %-20s  carried by         %-20s\r\n",
                                       vb,
                                       search_obj->short_descr,
                                       PERS(search_obj->carried_by, ch));
			found=TRUE;
		} else {
                                                continue;
                }
	        strcat(buffer,buf);
	}
    }

    if ( !found ) {
	act( "You didn't find any $T.", ch, NULL, argument, TO_CHAR );
    }else {
	page_to_char(buffer,ch);
    }
    return;
}

void do_pwhere (CHAR_DATA *ch, char *argument) {
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;
	char outbuf[MAX_STRING_LENGTH];
	char vb[MAX_STRING_LENGTH];
	bool fall;
	bool found=FALSE;
	
	if (argument == NULL || argument[0] == '\0')
		fall = TRUE;
	else
		fall = FALSE;

	for (d = descriptor_list ; d ; d=d->next) {

		if (d->connected != CON_PLAYING) {
			continue;
                }

		if (d->original != NULL) {
                         victim = d->original;
                } else {
                         victim = d->character;
                }

		if (!fall) {
			if ( !str_cmp(victim->name, argument) 
                          && can_see (ch, victim) ) {
				format_vnum(victim->in_room->vnum, vb);
				sprintf (outbuf, "[{g%s{x] is in room [{r%11s{x] [{c%s{x]\r\n",
                                                  victim->name,
					          vb,
                                                  victim->in_room->name);
				send_to_char (outbuf, ch);
				found = TRUE;
				break;
				}
			}
		else if (can_see(ch, victim)) {
			format_vnum(victim->in_room->vnum, vb);
			sprintf (outbuf, "[{g%s{x] is in room [{r%11s{x] [{c%s{x]\r\n",
                                          victim->name,
				          vb,
                                          victim->in_room->name);
			found = TRUE;
			send_to_char (outbuf,ch);
			}
		}
	
	if (!found) {
		send_to_char ("No matching characters found.\r\n",ch);
        }

	return;
	} /* end pwhere */
						

void do_reboo( CHAR_DATA *ch, char *argument ) {
    send_to_char( "If you want to REBOOT, spell it out.\r\n", ch );
    return;
}


void do_reboot( CHAR_DATA *ch, char *argument ) {
char arg1[MAX_STRING_LENGTH];

    if (IS_MUD(ch)) return;

    one_argument(argument, arg1);

    if (arg1[0] == '\0')    {
        send_to_char("Syntax:  Reboot Now\r\n"
                     "         Reboot Stop\r\n"
                     "         Reboot <time>\r\n", ch);
        return;
    }

    if (!str_cmp (arg1, "now"))    {
        reboot_shutdown(1);
        return;
    }

    if (!str_cmp (arg1, "stop"))    {
        if (pulse_muddeath >= 0 && reboot_type ==1)        {
            pulse_muddeath = -1;
            reboot_type = -1;
            do_echo(ch,"{gReboot has been stopped, resume normal activities.{x\r\n");
            return;
        }else {
            send_to_char("There is no reboot countdown in progress.\r\n",ch);
            return;
        }
    }

    if (is_number(arg1))    {
        if (atoi(arg1) < 1 || atoi(arg1) > 10) {
            send_to_char("Time range is between 1 and 10 minutes.\r\n", ch);
            return;
        } else {
            pulse_muddeath = atoi(arg1);
            reboot_type = 1;
            check_muddeath( );
            return;
        }
    } else {
        send_to_char("Syntax:  Reboot Now\r\n"
                     "         Reboot Stop\r\n"
                     "         Reboot <time>\r\n", ch);
        return; 
    }

    return;
}


void do_refresh( CHAR_DATA *ch, char *argument ) {
char arg1[MAX_STRING_LENGTH];

    if (IS_MUD(ch)) return;

    one_argument(argument, arg1);

    if (arg1[0] == '\0')    {
        send_to_char("Syntax:  Refresh Now\r\n"
                     "         Refresh Stop\r\n"
                     "         Refresh <time>\r\n", ch);
        return;
    }

    if (!str_cmp (arg1, "now"))    {
        reboot_shutdown(0);
        return;
    }

    if (!str_cmp (arg1, "stop"))    {
        if (pulse_muddeath >= 0 && reboot_type == 0)        {
            pulse_muddeath = -1;
            reboot_type = -1;
            do_echo(ch,"{gRefresh has been stopped, resume normal activities.{x\r\n");
            return;
        }else {
            send_to_char("There is no refresh countdown in progress.\r\n",ch);
            return;
        }
    }

    if (is_number(arg1))    {
        if (atoi(arg1) < 1 || atoi(arg1) > 10) {
            send_to_char("Time range is between 1 and 10 minutes.\r\n", ch);
            return;
        } else {
            pulse_muddeath = atoi(arg1);
            reboot_type = 0;
            check_muddeath( );
            return;
        }
    } else {
        send_to_char("Syntax:  Refresh Now\r\n"
                     "         Refresh Stop\r\n"
                     "         Refresh <time>\r\n", ch);
        return; 
    }

    return;
}


void reboot_shutdown(short type) {
extern bool merc_down;
DESCRIPTOR_DATA *d,*d_next;

    if (IS_SET(mud.flags, MUD_AUTOOLC)) olcautosave();

    log_string("Saving Morgues...");
    save_morgue();

    log_string("Saving Trees...:");
    save_tree();

    log_string("Saving Shops...");
    save_shop_obj();

    log_string("Saving leases...");
    save_leases();

    log_string("Saving lockers...");
    save_all_lockers();

    log_string("Saving banks...");
    save_banks();

    log_string("Saving memberships...");
    save_socialites();

    log_string("Saving Mana...");
    save_cults();

    log_string("Saving Stock Market...");
    save_shares();

    log_string("Saving Artifacts...");
    save_artifacts();

    if (type == 0) {
        copyover(FALSE);
    } else {
        merc_down = TRUE;

        for (d = descriptor_list; d != NULL; d = d_next)  {
            d_next = d->next;
            if (d->connected == CON_PLAYING) save_char_obj(d->original ? d->original : d->character);
            close_socket(d, TRUE);
        }

        if (type ==2) {
            FILE *fp = fopen ("../run.lock", "w");
            if (fp) fclose (fp);
        }
    }
    return;
}


void do_shutdow( CHAR_DATA *ch, char *argument ) {
    send_to_char( "If you want to SHUTDOWN, spell it out.\r\n", ch );
    return;
}


void do_shutdown( CHAR_DATA *ch, char *argument ) {
    char arg1[MAX_STRING_LENGTH];

    if (IS_MUD(ch)) return;

    one_argument(argument, arg1);

    if (arg1[0] == '\0')    {
        send_to_char("Syntax:  Shutdown Now\r\n"
                     "         Shutdown Stop\r\n"
                     "         Shutdown <time>\r\n", ch);
        return;
    }

    if (!str_cmp (arg1, "now"))    {
        reboot_shutdown(2);
        return;
    }

    if (!str_cmp (arg1, "stop"))  {
        if (pulse_muddeath >= 0 && !reboot_type == 2) {
            pulse_muddeath = -1;
            reboot_type = -1;
            do_echo(ch,"{gShutdown has been stopped, resume normal activities.{x\r\n");
            return;
        }else {
            send_to_char("There is no shutdown countdown in progress.\r\n",ch);
            return;
        }
    }

    if (is_number(arg1))  {
        if (atoi(arg1) < 1 || atoi(arg1) > 10)  {
            send_to_char("Time range is between 1 and 10 minutes.\r\n", ch);
            return;
        } else  {
            pulse_muddeath = atoi(arg1);
            reboot_type = 2;
            check_muddeath( );
            return;
        }
    }  else  {
        send_to_char("Syntax:  Shutdown Now\r\n"
                     "         Shutdown Stop\r\n"
                     "         Shutdown <time>\r\n", ch);
        return; 
    }
    return;
}


void do_snoop( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Snoop whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	send_to_char( "No descriptor to snoop.\r\n", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Cancelling all snoops.\r\n", ch );
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == ch->desc )
		d->snoop_by = NULL;
	}
	notify_message (ch, WIZNET_SNOOP, TO_IMM_ADMIN, victim->name);
	return;
    }

    if ( victim->desc->snoop_by != NULL )
    {
	send_to_char( "Busy already.\r\n", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\r\n", ch );
	return;
    }

    if ( ch->desc != NULL )
    {
	for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
	{
	    if ( d->character == victim || d->original == victim )
	    {
		send_to_char( "No snoop loops.\r\n", ch );
		return;
	    }
	}
    }

    victim->desc->snoop_by = ch->desc;
    send_to_char( "Ok.\r\n", ch );
	notify_message (ch, WIZNET_SNOOP, TO_IMM_ADMIN, victim->name);
    return;
}



void do_switch( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    
    if ( arg[0] == '\0' ) {
	send_to_char( "Switch into whom?\r\n", ch );
	return;
    }

    if ( ch->desc == NULL ) {
	return;
    }
    
    if ( ch->desc->original != NULL ) {
	send_to_char( "You are already switched.\r\n", ch );
	return;
    }

    victim = get_char_world( ch, arg );

    if ( victim == NULL ) {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( victim == ch ) {
	send_to_char( "Ok.\r\n", ch );
	return;
    }

    if (!IS_NPC(victim)) {
	send_to_char("You can only switch into mobiles.\r\n",ch);
	return;
    }

    if ( victim->desc != NULL ) {
	send_to_char( "Character in use.\r\n", ch );
	return;
    }
	
   /* Zeran - to fix null pointer dereferencing all over in the code,
	give switched player access to his pcdata fields. */

    victim->pcdata = ch->pcdata;

    ch->desc->character = victim;
    ch->desc->original  = ch;

    victim->desc        = ch->desc;

    /* change communications to match */

    victim->comm = ch->comm;
    victim->lines = ch->lines;

    send_to_char( "Ok.\r\n", victim );

    log_string ("switch complete");

    notify_message (ch, WIZNET_SWITCH, TO_IMM, victim->short_descr);

    return;
}



void do_return( CHAR_DATA *ch, char *argument ) {

    if ( ch->desc == NULL ) return;

    if ( ch->desc->original == NULL ) {
	send_to_char( "You aren't switched.\r\n", ch );
	return;
    }

    send_to_char( "You return to your original body.\r\n", ch );
    if (ch->leader >0) SET_BIT(ch->affected_by, AFF_CHARM);

    ch->pcdata		= NULL;
    ch->desc->character       	= ch->desc->original;
    ch->desc->original        	= NULL;

    ch->desc->character->desc 	= ch->desc; 
    ch->desc                  		= NULL;

    if (ch->pIndexData) {
        if (ch->pIndexData->vnum == MOB_VNUM_ASTRAL) {
            act("{mThe astral body dissolves.{x", ch, NULL, NULL, TO_ROOM);
            extract_char(ch, TRUE);
        }
    }
    return;
}

/* trust levels for load and clone */
bool obj_check (CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (IS_TRUSTED(ch,GOD)
	|| (IS_TRUSTED(ch,IMMORTAL) && obj->level <= 20 && obj->cost <= 1000)
	|| (IS_TRUSTED(ch,DEMI)	    && obj->level <= 10 && obj->cost <= 500)
	|| (IS_TRUSTED(ch,ANGEL)    && obj->level <=  5 && obj->cost <= 250)
	|| (IS_TRUSTED(ch,AVATAR)   && obj->level ==  0 && obj->cost <= 100))
	return TRUE;
    else
	return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
    OBJ_DATA *c_obj, *t_obj;


    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
	if (obj_check(ch,c_obj))
	{
	    t_obj = create_object(c_obj->pIndexData,0);
	    clone_object(c_obj,t_obj);
	    obj_to_obj(t_obj,clone);
	    recursive_clone(ch,c_obj,t_obj);
	}
    }
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char *rest;
    CHAR_DATA *mob;
    OBJ_DATA  *obj;

    rest = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Clone what?\r\n",ch);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	mob = NULL;
	obj = get_obj_here(ch,rest);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\r\n",ch);
	    return;
	}
    }
    else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	obj = NULL;
	mob = get_char_room(ch,rest);
	if (mob == NULL)
	{
	    send_to_char("You don't see that here.\r\n",ch);
	    return;
	}
    }
    else /* find both */
    {
	mob = get_char_room(ch,argument);
	obj = get_obj_here(ch,argument);
	if (mob == NULL && obj == NULL)
	{
	    send_to_char("You don't see that here.\r\n",ch);
	    return;
	}
    }

    /* clone an object */
    if (obj != NULL)
    {
	OBJ_DATA *clone;

	if (!obj_check(ch,obj))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\r\n",ch);
	    return;
	}

	clone = create_object(obj->pIndexData,0); 
	clone_object(obj,clone);
	if (obj->carried_by != NULL)
	    obj_to_char(clone,ch);
	else
	    obj_to_room(clone,ch->in_room);
 	recursive_clone(ch,obj,clone);

	act("$n has created $p.",ch,clone,NULL,TO_ROOM);
	act("You clone $p.",ch,clone,NULL,TO_CHAR);
	return;
    }
    else if (mob != NULL)
    {
	CHAR_DATA *clone;
	OBJ_DATA *new_obj;

	if (!IS_NPC(mob))
	{
	    send_to_char("You can only clone mobiles.\r\n",ch);
	    return;
	}

	if ((mob->level > 20 && !IS_TRUSTED(ch,GOD))
	||  (mob->level > 10 && !IS_TRUSTED(ch,IMMORTAL))
	||  (mob->level >  5 && !IS_TRUSTED(ch,DEMI))
	||  (mob->level >  0 && !IS_TRUSTED(ch,ANGEL))
	||  !IS_TRUSTED(ch,AVATAR))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\r\n",ch);
	    return;
	}

	clone = create_mobile(mob->pIndexData);
	clone_mobile(mob,clone); 
	
	for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
	{
	    if (obj_check(ch,obj))
	    {
		new_obj = create_object(obj->pIndexData,0);
		clone_object(obj,new_obj);
		recursive_clone(ch,obj,new_obj);
		obj_to_char(new_obj,clone);
		new_obj->wear_loc = obj->wear_loc;
	    }
	}
	char_to_room(clone,ch->in_room);
        act("$n has created $N.",ch,NULL,clone,TO_ROOM);
        act("You clone $N.",ch,NULL,clone,TO_CHAR);
        return;
    }
}

/* RT to replace the two load commands */

void do_load(CHAR_DATA *ch, char *argument ) {
   char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')    {
	send_to_char("Syntax:\r\n",ch);
	send_to_char("  load mob <vnum>\r\n",ch);
	send_to_char("  load obj <vnum> <level>\r\n",ch);
	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))    {
	do_mload(ch,argument);
	return;
    }

    if (!str_cmp(arg,"obj"))    {
	do_oload(ch,argument);
	return;
    }
    /* echo syntax */
    do_load(ch,"");
}


void do_mload( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	send_to_char( "Syntax: load mob <vnum>.\r\n", ch );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	send_to_char( "No mob has that vnum.\r\n", ch );
	return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    act( "$n has created $N!", ch, NULL, victim, TO_ROOM );
    send_to_char( "Ok.\r\n", ch );
	notify_message (ch, WIZNET_LOAD, TO_IMM_ADMIN, victim->short_descr);
    return;
}



void do_oload( CHAR_DATA *ch, char *argument ) {
    char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;
    
    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1))
    {
	send_to_char( "Syntax: load obj <vnum> <level>.\r\n", ch );
	return;
    }
    
    level = get_trust(ch); /* default */
  
    if ( arg2[0] != '\0')      {
	if (!is_number(arg2))        {
	  send_to_char( "Syntax: oload <vnum> <level>.\r\n", ch );
	  return;
	}
        level = atoi(arg2);
        if (level < 0 || level > get_trust(ch))	{
	  send_to_char( "Level must be be between 0 and your level.\r\n",ch);
  	  return;
	}
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )    {
	send_to_char( "No object has that vnum.\r\n", ch );
	return;
    }

    obj = create_object( pObjIndex, level );
	/* Zeran - set size equal to creator's size */
	obj->size = get_char_size(ch);
	obj->level = level;
    if ( CAN_WEAR(obj, ITEM_TAKE) )
	obj_to_char( obj, ch );
    else
	obj_to_room( obj, ch->in_room );
    act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
    send_to_char( "Ok.\r\n", ch );
	notify_message (ch, WIZNET_LOAD, TO_IMM, obj->short_descr);
    return;
}



void do_purge( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg1 );

    if ( arg[0] == '\0' || arg[0] == '!' )    {
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	for ( victim = ch->in_room->people; victim != NULL; victim = vnext ) {
	
	    vnext = victim->next_in_room;

                    if (!str_cmp(arg, "!") && IS_IMP(ch)) {
  	         if (IS_NPC(victim) 
                         &&  victim != ch) extract_char( victim, TRUE );
                    } else {
	         if (  IS_NPC(victim)                      
                         && !IS_SET(victim->act,ACT_NOPURGE)
                         &&  victim != ch                        
                         && !IS_SET(victim->act, ACT_PET)) extract_char( victim, TRUE );
                    }
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next ){
	    obj_next = obj->next_content;
                    if (!str_cmp(arg, "!") && IS_IMP(ch)) {
                         extract_obj(obj );
                    } else {
	         if (!IS_OBJ_STAT(obj, ITEM_NOPURGE)) extract_obj(obj );
                    }
	}

	act( "$n purges the room!", ch, NULL, NULL, TO_ROOM);
	send_to_char( "Ok.\r\n", ch );
	return;
    }

    if ((victim = get_char_world( ch, arg ) ) == NULL )    {
             if ((obj = get_obj_here( ch, arg ) ) == NULL )    {
	  send_to_char( "They aren't here.\r\n", ch );
             } else {
                  if (!str_cmp(arg1, "!") && IS_IMP(ch)) {
                       extract_obj(obj );
                  } else {
	       if (!IS_OBJ_STAT(obj, ITEM_NOPURGE)) extract_obj(obj );
                  }
             }
             return;
    }

    if ( !IS_NPC(victim) )    {
	if (ch == victim)	{
	  send_to_char("Be reasonable.\r\n",ch);
	  return;
	}

	if (get_trust(ch) <= get_trust(victim)) {
	  send_to_char("Maybe that wasn't a good idea...\r\n",ch);
	  sprintf_to_char(victim, "%s tried to purge you!\r\n",ch->name);
	  return;
	}

	act("$n disintegrates $N.",ch,0,victim,TO_NOTVICT);

    	if (victim->level > 1)  save_char_obj( victim );
    	d = victim->desc;
    	extract_char( victim, TRUE );
    	if ( d != NULL )  close_socket(d, FALSE);
	return;
    }

    act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    extract_char( victim, TRUE );
    return;
}


int eff_lev(CHAR_DATA *ch) {

    if (IS_IMP(ch)) 		return MAX_LEVEL;
    if (IS_GREATER(ch)) 	return MAX_LEVEL - 1;
    if (IS_GOD(ch)) 	return MAX_LEVEL - 2;
    if (IS_LESSER(ch)) 	return MAX_LEVEL - 3;
    if (IS_CREATOR(ch)) 	return MAX_LEVEL - 4;
    return 0;
}


void do_advance( CHAR_DATA *ch, char *argument ) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    char buf[MAX_STRING_LENGTH];
    int max_level;

    if (IS_MUD(ch)) return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: advance <char> <level>.\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They are not here.\r\n", ch);
	return;
    }

    max_level = eff_lev(ch);

    level = atoi( arg2 );
    if ( level < 1 
      || level > max_level ) {
        sprintf(buf, "Level must be between 1 and %d\r\n", max_level);
	send_to_char( buf, ch );
	return;
    }
bug("Requested level is %d", level);
bug("Current level is %d", victim->level);
    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */

    if (level == victim->level) {
      send_to_char("Nothing seems to happen.\r\n", ch);
      send_to_char("You feel a chill shiver.\r\n", victim); 
      return;
    }

    if ( level < victim->level ) {
    
      send_to_char( "Lowering a player's level!\r\n", ch );
      send_to_char( "**** OOOOHHHHHHHHHH  NNNNOOOO ****\r\n", victim );

      gain_exp( victim, ( victim->exp - exp_for_level(level)), TRUE);
      
      while (victim->level > level) {
        drop_level( victim );

      }

    } else {

      send_to_char( "Raising a player's level!\r\n", ch );
      send_to_char( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\r\n", victim );

      gain_exp( victim, ( exp_for_level(level) - victim->exp), TRUE);

    }

   /* Round exps... */

    victim->exp += dice(5, 100); 

   /* Save and all done... */

    save_char_obj(victim);

    return;
}


void do_trust( CHAR_DATA *ch, char *argument ) {
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
CHAR_DATA *victim;
int div;

    if (IS_MUD(ch)) return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )    {
	send_to_char( "Syntax: trust <char> <divinity>.\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )    {
	send_to_char( "That player is not here.\r\n", ch);
	return;
    }

    if (IS_NPC(victim)) {
	send_to_char( "Not on an NPC.\r\n", ch);
	return;
    }

    if ( !str_cmp(arg2, "imp")) {
      div = DIV_IMPLEMENTOR;
    } else if ( !str_cmp(arg2, "greater")) {
      div = DIV_GREATER;
    } else if ( !str_cmp(arg2, "god")) {
      div = DIV_GOD;
    } else if ( !str_cmp(arg2, "lesser")) {
      div = DIV_LESSER;
    } else if ( !str_cmp(arg2, "creator")) {
      div = DIV_CREATOR;
    } else if ( !str_cmp(arg2, "hero")) {
      div = DIV_HERO;
    } else if ( !str_cmp(arg2, "player")) {
      div = 0;
    } else if ( !str_cmp(arg2, "mud")) {
      div = DIV_MUD;
    } else {
	send_to_char( "divinity can be imp, greater, god, lesser, creator, hero, player.\r\n", ch );
	return;
    }

    if ( div > get_divinity(ch) 
    && !IS_IMP(ch))    {
	send_to_char( "Limited to your own divinity.\r\n", ch );
	return;
    }

    if (victim->level < DIV_LEV_HUMAN) {
    } else if (victim->level < DIV_LEV_INVESTIGATOR) {
      div = div | DIV_INVESTIGATOR;
    } else if (victim->level < DIV_LEV_HERO) {
      div = div | DIV_HUMAN;
    } else {
      div = div | DIV_HERO;
    } 

    if (div > get_divinity(victim)) {
      send_to_char("{WYou feel more trusted.{x\r\n", victim);
    } else {
      send_to_char("{RYou feel less trusted.{x\r\n", victim);    
    }  

    victim->pcdata->divinity = div; 
    if (IS_IMP(victim)) victim->pcdata->authority = IMMAUTH_ALL;

    if (!IS_IMMORTAL(victim)) {
         REMOVE_BIT(victim->plr, PLR_HOLYLIGHT);
         REMOVE_BIT(victim->plr, PLR_WIZINVIS);
         REMOVE_BIT(victim->plr, PLR_XINFO);
    }
    send_to_char("Done.\r\n", ch);
    return;
}


void do_restore( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );
    if (arg[0] == '\0' || !str_cmp(arg,"room"))
    {
    /* cure room */
    	
        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)        {
            affect_strip_afn(vch, gafn_plague);
            affect_strip_afn(vch, gafn_poison);
            affect_strip_afn(vch, gafn_blindness);
            affect_strip_afn(vch, gafn_sleep);
            affect_strip_afn(vch, gafn_curse);
            
            vch->hit 	= vch->max_hit;
            vch->mana	= vch->max_mana;
            vch->move = vch->max_move;
            vch->limb[0]=0;
            vch->limb[1]=0;
            vch->limb[2]=0;
            vch->limb[3]=0;
            vch->limb[4]=0;
            vch->limb[5]=0;
            vch->limb[6]=0;
            REMOVE_BIT(vch->form, FORM_BLEEDING);
            update_pos( vch);
            update_wounds(vch);
            act("$n has restored you.",ch,NULL,vch,TO_VICT);
        }
        
        send_to_char("Room restored.\r\n",ch);
		notify_message(ch, WIZNET_RESTORE, TO_IMM_ADMIN, "local room");
        return;

    }
    
    if ( get_trust(ch) >=  MAX_LEVEL && !str_cmp(arg,"all"))    {
    /* cure all */
    	
        for (d = descriptor_list; d != NULL; d = d->next)
        {
	    victim = d->character;

	    if (victim == NULL || IS_NPC(victim))
		continue;
                
            affect_strip_afn(victim, gafn_plague);
            affect_strip_afn(victim, gafn_poison);
            affect_strip_afn(victim, gafn_blindness);
            affect_strip_afn(victim, gafn_sleep);
            affect_strip_afn(victim, gafn_curse);
            
            victim->hit 	= victim->max_hit;
            victim->mana	= victim->max_mana;
            victim->move	= victim->max_move;
            victim->limb[0]=0;
            victim->limb[1]=0;
            victim->limb[2]=0;
            victim->limb[3]=0;
            victim->limb[4]=0;
            victim->limb[5]=0;
            victim->limb[6]=0;
            REMOVE_BIT(victim->form, FORM_BLEEDING);
            update_pos( victim);
            update_wounds(victim);
	    if (victim->in_room != NULL)
                act("$n has restored you.",ch,NULL,victim,TO_VICT);
        }
	send_to_char("All active players restored.\r\n",ch);
	notify_message(ch, WIZNET_RESTORE, TO_IMM_ADMIN, "all players");
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    affect_strip_afn(victim, gafn_plague);
    affect_strip_afn(victim, gafn_poison);
    affect_strip_afn(victim, gafn_blindness);
    affect_strip_afn(victim, gafn_sleep);
    affect_strip_afn(victim, gafn_curse);

    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    victim->limb[0]=0;
    victim->limb[1]=0;
    victim->limb[2]=0;
    victim->limb[3]=0;
    victim->limb[4]=0;
    victim->limb[5]=0;
    victim->limb[6]=0;
    REMOVE_BIT(victim->form, FORM_BLEEDING);
    update_pos( victim );
    update_wounds(victim);

    act( "$n has restored you.", ch, NULL, victim, TO_VICT );
    send_to_char( "Ok.\r\n", ch );
	notify_message(ch, WIZNET_RESTORE, TO_IMM_ADMIN, IS_NPC(victim) ? victim->short_descr : victim->name );
    return;
}


void do_freeze( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Freeze whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\r\n", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\r\n", ch );
	return;
    }

    if ( IS_SET(victim->plr, PLR_FREEZE) )
    {
	REMOVE_BIT(victim->plr, PLR_FREEZE);
	send_to_char( "You can play again.\r\n", victim );
	send_to_char( "FREEZE removed.\r\n", ch );
    }
    else
    {
	SET_BIT(victim->plr, PLR_FREEZE);
	send_to_char( "You can't do ANYthing!\r\n", victim );
	send_to_char( "FREEZE set.\r\n", ch );
    }

    save_char_obj( victim );

    return;
}



void do_log( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Log whom?\r\n", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	if ( fLogAll )
	{
	    fLogAll = FALSE;
	    send_to_char( "Log ALL off.\r\n", ch );
	}
	else
	{
	    fLogAll = TRUE;
	    send_to_char( "Log ALL on.\r\n", ch );
	}
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\r\n", ch );
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->plr, PLR_LOG) )
    {
	REMOVE_BIT(victim->plr, PLR_LOG);
	send_to_char( "LOG removed.\r\n", ch );
    }
    else
    {
	SET_BIT(victim->plr, PLR_LOG);
	send_to_char( "LOG set.\r\n", ch );
    }

    return;
}



void do_noemote( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noemote whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }


    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\r\n", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOEMOTE) )
    {
	REMOVE_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can emote again.\r\n", victim );
	send_to_char( "NOEMOTE removed.\r\n", ch );
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can't emote!\r\n", victim );
	send_to_char( "NOEMOTE set.\r\n", ch );
    }

    return;
}



void do_noshout( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )    {
	send_to_char( "Noshout whom?\r\n",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( IS_NPC(victim) )    {
	send_to_char( "Not on NPC's.\r\n", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOSHOUT))    {
                if ( get_trust( victim ) > get_trust( ch ) )    {
	      send_to_char( "You failed.\r\n", ch );
	      return;
               }

	REMOVE_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can shout again.\r\n", victim );
	send_to_char( "NOSHOUT removed.\r\n", ch );

    }    else    {
                if ( get_trust( victim ) >= get_trust( ch ) )    {
	     send_to_char( "You failed.\r\n", ch );
	     return;
                }

	SET_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can't shout!\r\n", victim );
	send_to_char( "NOSHOUT set.\r\n", ch );
    }

    return;
}



void do_notell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Notell whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\r\n", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOTELL) )
    {
	REMOVE_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can tell again.\r\n", victim );
	send_to_char( "NOTELL removed.\r\n", ch );
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can't tell!\r\n", victim );
	send_to_char( "NOTELL set.\r\n", ch );
    }

    return;
}



void do_peace( CHAR_DATA *ch, char *argument ) {
    CHAR_DATA *rch;

    if (IS_AFFECTED(ch, AFF_INCARNATED)) {
        send_to_char( "You lack that power in your mortal form.\r\n", ch );
        return;
    }

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )    {
	if ( rch->fighting != NULL )
	    stop_fighting( rch, TRUE );
	if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
	    REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
    }

    send_to_char( "Ok.\r\n", ch );
    return;
}


void do_ban( CHAR_DATA *ch, char *argument ) {
char arg[MAX_INPUT_LENGTH];
char *name;
CHAR_DATA *victim;
BAN_DATA *pban;
char trans[5];

    if (IS_NPC(ch) || IS_MUD(ch)) return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )    {
	sprintf_to_char(ch, "Banned sites:\r\n" );
	for ( pban = ban_list; pban != NULL; pban = pban->next )	{
	    sprintf_to_char(ch, "%s\r\n", pban->name );
	}
	return;
    }

    if ((victim = get_char_world_player(ch, arg)) != NULL) {
          if (!IS_NPC(victim)) sprintf(arg, "%s", victim->desc->host);
    }

    if (victim == ch) {
          send_to_char( "Don't be silly!\r\n", ch );
          return;
    }       

    trans[0] = arg[0];
    trans[1] = '\0';
    if (!is_number(trans) && trans[0] != '*') {
          send_to_char( "Not found!\r\n", ch );
          return;
    }       
        
    for ( pban = ban_list; pban != NULL; pban = pban->next )    {
	if (!str_cmp( arg, pban->name )) {
	    send_to_char( "That site is already banned!\r\n", ch );
	    return;
	}
    }

    if (!str_cmp(arg,"195.34.155.132")) {
	    send_to_char( "{MMuahaha!{x\r\n", ch );
	    return;
    }

    if ( ban_free == NULL )    {
	pban		= (BAN_DATA *) alloc_perm( sizeof(*pban) );
    }    else    {
	pban		= ban_free;
	ban_free	= ban_free->next;
    }

    name = strdup(arg);
    pban->type = 0;

    if (name[0] == '*') {
        pban->type = BAN_PREFIX;
        name++;
    } else if (name[strlen(name) - 1] == '*') {
        pban->type = BAN_SUFFIX;
        name[strlen(name) - 1] = '\0';
    }
    pban->name	= str_dup(name);
    pban->next	= ban_list;
    ban_list	= pban;
    send_to_char( "Ok.\r\n", ch );
    save_ban();
    return;
}


void save_ban(void) {
FILE *fp;
BAN_DATA *pban;

    fp = fopen(BAN_FILE, "w");
    if (!fp) {
        log_string("Can't write ban!");
        return;
    }

    for (pban = ban_list; pban; pban = pban->next )    {
          fprintf( fp, "%s %d\n", pban->name, pban->type);
    }
    fprintf( fp, "End\n");

    fclose(fp);
    return;
}
   

void load_ban(void) {
FILE *fp;
BAN_DATA *pban;
char *word;

    fp = fopen(BAN_FILE, "r");
    if (!fp) {
        log_string("No bans - lucky you...");
        return;
    }

    for ( ; ; ) {
         word   = fread_word( fp );

         if (!str_cmp(word, "End")) break;
         if (ban_free == NULL)    {
	pban = (BAN_DATA *) alloc_perm( sizeof(*pban));
         } else {
	pban = ban_free;
	ban_free	= ban_free->next;
         }

         pban->name	= str_dup(word);
         pban->type = fread_number(fp);
         pban->next	= ban_list;
         ban_list	= pban;
    }         

    log_string("... ban loaded.");
    fclose(fp);
    return;
}


void do_allow( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA *prev;
    BAN_DATA *curr;

    one_argument( argument, arg );

    if ( arg[0] == '\0' ) {
	send_to_char( "Remove which site from the ban list?\r\n", ch );
	return;
    }

    prev = NULL;
    for ( curr = ban_list; curr != NULL; prev = curr, curr = curr->next )  {
	if ( !str_cmp( arg, curr->name )) {
	    if ( prev == NULL ) ban_list   = ban_list->next;
	    else prev->next = curr->next;

	    free_string( curr->name );
	    curr->next	= ban_free;
	    ban_free	= curr;
	    send_to_char( "Ok.\r\n", ch );
                    save_ban();
	    return;
	}
    }

    send_to_char( "Site is not banned.\r\n", ch );
    return;
}


void do_wizlock( CHAR_DATA *ch, char *argument ) {

    if ( IS_SET(mud.flags, MUD_WIZLOCK) ) {
	send_to_char( "Game un-wizlocked.\r\n", ch );
        REMOVE_BIT(mud.flags, MUD_WIZLOCK);
    } else {
	send_to_char( "Game wizlocked.\r\n", ch );
        SET_BIT(mud.flags, MUD_WIZLOCK);
    }

    return;
}

/* RT anti-newbie code */

void do_newlock( CHAR_DATA *ch, char *argument ) {

    if ( IS_SET(mud.flags, MUD_NEWLOCK) ) {
        send_to_char( "Newlock removed.\r\n", ch );
        REMOVE_BIT(mud.flags, MUD_NEWLOCK);
    } else {
        send_to_char( "New characters have been locked out.\r\n", ch );
        SET_BIT(mud.flags, MUD_NEWLOCK);
    }

    return;
}

/* Toggle the perma PK setting... */

void do_permapk( CHAR_DATA *ch, char *argument ) {

    if ( IS_SET(mud.flags, MUD_PERMAPK) ) {
        send_to_char( "Mud now in free player combat mode.\r\n", ch );
        REMOVE_BIT(mud.flags, MUD_PERMAPK);
        do_echo(ch, "{RThe mud is now in free player combat mode.{x");
    } else {
        send_to_char( "Mud now in perma death player combat mode.\r\n", ch );
        SET_BIT(mud.flags, MUD_PERMAPK);
        do_echo(ch, "{RThe mud is now in perma death player combat mode.{x");
    }

    return;
}


void do_slookup( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int efn;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Lookup which skill or spell?\r\n", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	for ( efn = 0; efn < MAX_EFFECT; efn++ )
	{
	    if (effect_array[efn].name == NULL)
		break;

	    sprintf( buf, "Efn: %3d  Slot: %3d  Spell: '%s'\r\n",
		efn, effect_array[efn].slot, effect_array[efn].name );

	    send_to_char( buf, ch );
	}
    }
    else
    {
	efn = get_spell_spn( arg );

	if ( efn == EFFECT_UNDEFINED) {
	    send_to_char( "No such effect.\r\n", ch );
	    return;
	}

	sprintf( buf, "Efn: %3d  Slot: %3d  Spell: '%s'\r\n",
	    efn, effect_array[efn].slot, effect_array[efn].name );
	send_to_char( buf, ch );
    }

    return;
}


void do_set( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')    {
	send_to_char("Syntax:\r\n",ch);
	send_to_char("  set mob   <name> <field> <value>\r\n",ch);
	send_to_char("  set obj   <name> <field> <value>\r\n",ch);
	send_to_char("  set room  <room> <field> <value>\r\n",ch);
        send_to_char("  set skill <name> <spell or skill> <value>\r\n",ch);
	return;
    }

    if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))   {
	do_mset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell"))    {
	do_sset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"object"))    {
	do_oset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"room"))    {
	do_rset(ch,argument);
	return;
    }
    /* echo syntax */
    do_set(ch,"");
}


void do_sset( CHAR_DATA *ch, char *argument ){
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\r\n",ch);
	send_to_char( "  set skill <name> <spell or skill> <value>\r\n", ch);
	send_to_char( "  set skill <name> all <value>\r\n",ch);  
	send_to_char("   (use the name of the skill, not the number)\r\n",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( !isSkilled(victim) )
    {
	send_to_char( "Not on unskilled NPC's.\r\n", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = get_skill_sn( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\r\n", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\r\n", ch );
	return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 150 )
    {
	send_to_char( "Value range is 0 to 150.\r\n", ch );
	return;
    }

    if ( fAll )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_array[sn].loaded) {
		setSkill(victim->effective, sn, value);
            }
	}
    }
    else
    {
	setSkill(victim->effective, sn, value);
    }

    return;
}


void do_mset( CHAR_DATA *ch, char *argument ) {
char arg1 [MAX_INPUT_LENGTH];
char arg2 [MAX_INPUT_LENGTH];
char arg3 [MAX_INPUT_LENGTH];
char buf[100];
CHAR_DATA *victim;
int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )    {
	send_to_char("Syntax:\r\n",ch);
	send_to_char("  set char <name> <field> <value>\r\n",ch); 
	send_to_char( "  Field being one of:\r\n",			ch );
	send_to_char( "    str int wis dex con luc cha sex level imm\r\n",ch );
	send_to_char( "    race gold hp mana move practice align\r\n",	ch );
	send_to_char( "    train drink drunk food security timer\r\n",	ch );
	send_to_char( "    recall affect sanity permadeath mare\r\n", ch );
	return;
    }

    victim = get_char_world( ch, arg1 );

    if ( victim == NULL ) {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( !IS_NPC(victim) 
    && !IS_IMP(ch) ) {
      if (ch->level < victim->level) {
        send_to_char ("You do not have authority to do that.\r\n",ch);
        return;
      }
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    /*
     * Set something.
     */

    if (flag_value(currency_accept, arg2) >= 0) {
                victim->gold[flag_value(currency_accept, arg2)] = value;
                send_to_char("Ok.",ch);

    } else if ( !str_cmp( arg2, "str" ))    {
	if ( value < STAT_MINIMUM || value > STAT_MAXIMUM )	{
	    sprintf(buf, "Range is from %d to %d.\r\n", STAT_MINIMUM, STAT_MAXIMUM);
	    send_to_char(buf,ch);
	    return;
	}

	victim->perm_stat[STAT_STR] = value;
                send_to_char("Ok.",ch);

    } else if ( !str_cmp( arg2, "luc" ))    {
	if ( value < STAT_MINIMUM || value > STAT_MAXIMUM )	{
	    sprintf(buf, "Range is from %d to %d.\r\n", STAT_MINIMUM, STAT_MAXIMUM);
	    send_to_char(buf,ch);
	    return;
	}

	victim->perm_stat[STAT_LUC] = value;
                send_to_char("Ok.",ch);

    } else if ( !str_cmp( arg2, "cha" ))    {
	if ( value < STAT_MINIMUM || value > STAT_MAXIMUM )	{
	    sprintf(buf, "Range is from %d to %d.\r\n", STAT_MINIMUM, STAT_MAXIMUM);
	    send_to_char(buf,ch);
	    return;
	}

	victim->perm_stat[STAT_CHA] = value;
                send_to_char("Ok.",ch);

    } else if ( !str_cmp( arg2, "recall" ) )    {
	victim->recall_perm = value;
                send_to_char("Ok.",ch);

    } else if ( !str_cmp( arg2, "life" ) )    {
                if (!IS_NPC(victim)) {
    	     victim->pcdata->lifespan = value;
                }
                send_to_char("Ok.",ch);

    }else if ( !str_cmp( arg2, "mare" ) )    {
	if (value > 0) victim->nightmare = TRUE;
                else victim->nightmare = FALSE;
                send_to_char("Ok.",ch);

    } else if ( !str_cmp( arg2, "imm" ) )    {
	if (value > 0) SET_BIT(victim->comm, COMM_IMMACCESS);
                else REMOVE_BIT(victim->comm, COMM_IMMACCESS);
                send_to_char("Ok.",ch);

    } else if ( !str_cmp( arg2, "int" )) {
        if ( value < STAT_MINIMUM || value > STAT_MAXIMUM )  {
	    sprintf(buf, "Range is from %d to %d.\r\n", STAT_MINIMUM, STAT_MAXIMUM);
                    send_to_char(buf,ch);
                    return;
        }
 
        victim->perm_stat[STAT_INT] = value;
        send_to_char("Ok.",ch);

    } else if ( !str_cmp( arg2, "wis" ))    {
	if ( value < STAT_MINIMUM || value > STAT_MAXIMUM ) {
	    sprintf(buf, "Range is from %d to %d.\r\n", STAT_MINIMUM, STAT_MAXIMUM);
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_WIS] = value;
                send_to_char("Ok.",ch);

    } else if ( !str_cmp( arg2, "dex" ))  {
	if ( value < STAT_MINIMUM || value > STAT_MAXIMUM ){
	    sprintf(buf, "Range is from %d to %d.\r\n", STAT_MINIMUM, STAT_MAXIMUM);
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_DEX] = value;
                send_to_char("Ok.",ch);

    } else  if ( !str_cmp( arg2, "con" )) {
	if ( value < STAT_MINIMUM || value > STAT_MAXIMUM ) {
	    sprintf(buf, "Range is from %d to %d.\r\n", STAT_MINIMUM, STAT_MAXIMUM);
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_CON] = value;
                send_to_char("Ok.",ch);

    } else if ( !str_prefix( arg2, "sex" ))  {
	if ( value < 0 || value > 2 ){
	    send_to_char( "Sex range is 0 to 2.\r\n", ch );
	    return;
	}
	victim->sex = value;
	if (!IS_NPC(victim)) victim->pcdata->true_sex = value;
                send_to_char("Ok.",ch);

    } else  if ( !str_prefix( arg2, "level" )) {
	if ( !IS_NPC(victim)) {
	    send_to_char( "Not on PC's.\r\n", ch );
	    return;
	}

	if ( value < 0 || value > MAX_LEVEL ) {
	    send_to_char( "Level range is 0 to 60.\r\n", ch );
	    return;
	}
	victim->level = value;
                send_to_char("Ok.",ch);

    } else if ( !str_prefix( arg2, "hp" )) {
	if ( value < -10 || value > 30000 ) {
	    send_to_char( "Hp range is -10 to 30,000 hit points.\r\n", ch );
	    return;
	}
	victim->max_hit = value;
                if (!IS_NPC(victim))   victim->pcdata->perm_hit = value;
                send_to_char("Ok.",ch);

    } else if ( !str_prefix( arg2, "mana" ))  {
	if ( value < 0 || value > 30000 ) {
	    send_to_char( "Mana range is 0 to 30,000 mana points.\r\n", ch );
	    return;
	}
	victim->max_mana = value;
                if (!IS_NPC(victim))  victim->pcdata->perm_mana = value;
                send_to_char("Ok.",ch);

    } else if ( !str_prefix( arg2, "move" )) {
	if ( value < 0 || value > 30000 ) {
	    send_to_char( "Move range is 0 to 30,000 move points.\r\n", ch );
	    return;
	}
	victim->max_move = value;
                if (!IS_NPC(victim))  victim->pcdata->perm_move = value;
                send_to_char("Ok.",ch);

    } else if ( !str_prefix( arg2, "practice" )) {
	if ( value < 0 || value > 250 )	{
	    send_to_char( "Practice range is 0 to 250 sessions.\r\n", ch );
	    return;
	}
	victim->practice = value;
                send_to_char("Ok.",ch);

    } else if ( !str_prefix( arg2, "fame" )) {
	if ( value < 0)	{
	    send_to_char( "Invalid value.\r\n", ch );
	    return;
	}
                if (IS_NPC(victim)) return;
	victim->pcdata->questpoints = value;
                send_to_char("Ok.",ch);

    } else if ( !str_prefix( arg2, "train" )) {
	if (value < 0 || value > 50 ) {
	    send_to_char("Training session range is 0 to 50 sessions.\r\n",ch);
	    return;
	}
	victim->train = value;
                send_to_char("Ok.",ch);

    } else if ( !str_prefix( arg2, "sanity" ))  {
	if (value < -128 || value > 128 ) {
	    send_to_char("Sanity range is -128 to 128 (60 is normal).\r\n",ch);
	    return;
	}
	victim->sanity = value;
                send_to_char("Ok.",ch);

    } else if ( !str_cmp( arg2, "security" )) {
        if ( IS_NPC( victim ))  {
            send_to_char( "Not on NPC's.\r\n", ch );
            return;
        }

        if ( value > ch->pcdata->security || value < 0 )   {
            if ( ch->pcdata->security != 0 )     {
                sprintf( buf, "Valid security is 0-%d.\r\n",  ch->pcdata->security );
                send_to_char( buf, ch );
            }  else   {
                send_to_char( "Valid security is 0 only.\r\n", ch );
            }
            return;
        }
        victim->pcdata->security = value;
        send_to_char("Ok.",ch);

    } else if ( !str_prefix( arg2, "align" ))   {
	if ( value < -1000 || value > 1000 ) {
	    send_to_char( "Alignment range is -1000 to 1000.\r\n", ch );
	    return;
	}
	victim->alignment = value;
                send_to_char("Ok.",ch);

    } else if ( !str_prefix( arg2, "timer" )) {
	if ( value < 0 || value > 99 ) {
	    send_to_char( "Timer Range 0 - 99.\r\n",ch );
	    return;
	}
    	victim->timer = value;
                send_to_char("Ok.",ch);

    } else if ( !str_prefix( arg2, "food" )) {
	if ( IS_NPC(victim)) {
	    send_to_char( "Not on NPC's.\r\n", ch );
	    return;
	}

	if ( value < -48 || value > 48 ) {
	    send_to_char( "Food range is -48 to 48 hours.\r\n", ch );
	    return;
	}

                gain_condition(victim, COND_FOOD, (24 * value) - victim->condition[COND_FOOD]); 
                send_to_char("Ok.",ch);

    }else if ( !str_prefix( arg2, "drink" ))  {
	if ( IS_NPC(victim)) {
	    send_to_char( "Not on NPC's.\r\n", ch );
	    return;
	}

	if ( value < -24 || value > 24 ) {
	    send_to_char( "Drink range is -24 to 24 hours.\r\n", ch );
	    return;
	}

                gain_condition(victim, COND_DRINK, (24 * value) - victim->condition[COND_DRINK]); 
                send_to_char("Ok.",ch);

    } else if ( !str_prefix( arg2, "drunk" )) {
	if ( IS_NPC(victim) ) {
	    send_to_char( "Not on NPC's.\r\n", ch );
	    return;
	}

	if ( value < 0 || value > 24 ) {
	    send_to_char( "Drunk range is 0 to 24 hours.\r\n", ch );
	    return;
	}

                gain_condition(victim, COND_DRUNK, value - (victim->condition[COND_DRUNK]) ); 
                send_to_char("Ok.",ch);

    } else if ( !str_prefix( arg2, "fat" )) {
	if ( IS_NPC(victim)) {
	    send_to_char( "Not on NPC's.\r\n", ch );
	    return;
	}

	if ( value < 0 || value > 300 ) {
	    send_to_char( "Fat range is 0 to 300 pounds.\r\n", ch );
	    return;
	}

                gain_condition(victim, COND_FAT, value - (victim->condition[COND_FAT]) ); 
                send_to_char("Ok.",ch);

    } else if (!str_prefix( arg2, "race" )) {
	int race;

	race = race_lookup(arg3);
	if ( race == 0) {
	    send_to_char("That is not a valid race.\r\n",ch);
	    return;
	}

	if (!IS_NPC(victim) && !race_array[race].pc_race && !IS_SET(get_authority(ch), IMMAUTH_ADMIN)) {
	    send_to_char("That is not a valid player race.\r\n",ch);
	    return;
	}

	victim->race = race;
                send_to_char("Ok.",ch);

    } else if (!str_prefix( arg2, "affect" ))     {
	long long affbit;

	affbit = flag_value_long( affect_flags, argument );

	if ( affbit != NO_FLAG
                && affbit != 0) {
	    victim->affected_by ^= affbit;
	    send_to_char( "Affect toggled.\r\n", ch);
	} else {
                    send_to_char("That is not a valid affect!\r\n", ch);
                }
 
     } else  if (!str_prefix( arg2, "act" ))     {
	long long affbit;

	affbit = flag_value_long( act_flags, argument );

	if ( affbit != NO_FLAG) {
	    victim->act ^= affbit;
	    send_to_char( "Act toggled.\r\n", ch);
	} else {
                    send_to_char("That is not a valid act!\r\n", ch);
                }
 
     } else if (!str_prefix( arg2, "plr" ))     {
	long long affbit;

                if (IS_NPC(ch)) {
             	    send_to_char( "Not on NPCs.\r\n", ch);
                    return;
                }

	affbit = flag_value_long(plr_flags, argument );

	if ( affbit != NO_FLAG) {
	    victim->plr ^= affbit;
	    send_to_char( "Player flag toggled.\r\n", ch);
	} else {
                    send_to_char("That is not a valid player flag!\r\n", ch);
                }
 
     } else if (!str_prefix( arg2, "permadeath" ) ) {
      if (IS_SET(victim->plr, PLR_PERMADEATH)) {
        REMOVE_BIT(victim->plr, PLR_PERMADEATH);
        send_to_char( "They are now immortal.\r\n", ch);
      } else {
        SET_BIT(victim->plr, PLR_PERMADEATH);
        send_to_char( "They are now mortal.\r\n", ch);
        send_to_char( "{rYou feel very mortal.{x\r\n", victim);
      } 
    } else {
       do_mset( ch, "" );
    }
    return;
}


void do_string( CHAR_DATA *ch, char *argument ){
    char type [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    smash_tilde( argument );
    argument = one_argument( argument, type );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )  {
	send_to_char("Syntax:\r\n",ch);
	send_to_char("  string char <name> <field> <string>\r\n",ch);
	send_to_char("    fields: name short long desc title spec\r\n",ch);
	send_to_char("  string obj  <name> <field> <string>\r\n",ch);
	send_to_char("    fields: name short long extended wear owear\r\n",ch);
	send_to_char("            use ouse remove oremove\r\n",ch);
	return;
    }
    
    if (!str_prefix(type,"character") || !str_prefix(type,"mobile")) {
    	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL ) {
	    send_to_char( "They aren't here.\r\n", ch );
	    return;
    	}

	/* string something */

     	if ( !str_prefix( arg2, "name" ) ){
	    if ( !IS_NPC(victim) )   {
	    	send_to_char( "Not on PC's.\r\n", ch );
	    	return;
	    }

	    free_string( victim->name );
	    victim->name = str_dup( arg3 );
	    return;
    	}
    	
    	if ( !str_prefix( arg2, "description" ) )
    	{
    	    free_string(victim->description);
    	    victim->description = str_dup(arg3);
    	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( victim->short_descr );
	    victim->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( victim->long_descr );
	    strcat(arg3,"\r\n");
	    victim->long_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "title" ) )
    	{
	    if ( IS_NPC(victim) )
	    {
	    	send_to_char( "Not on NPC's.\r\n", ch );
	    	return;
	    }

	    set_title( victim, arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "spec" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\r\n", ch );
	    	return;
	    }

	    if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
	    {
	    	send_to_char( "No such spec fun.\r\n", ch );
	    	return;
	    }

	    return;
    	}
    }
    
    if (!str_prefix(type,"object"))
    {
    	/* string an obj */
    	
   	if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "Nothing like that in heaven or earth.\r\n", ch );
	    return;
    	}
    	
        if ( !str_prefix( arg2, "name" ) )
    	{
	    free_string( obj->name );
	    obj->name = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( obj->description );
	    obj->description = str_dup( arg3 );
	    return;
    	}
    	
    	if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
    	{
	    EXTRA_DESCR_DATA *ed;

	    argument = one_argument( argument, arg3 );
	    if ( argument == NULL )
	    {
	    	send_to_char( "Syntax: oset <object> ed <keyword> <string>\r\n",
		    ch );
	    	return;
	    }

 	    strcat(argument,"\r\n");

	    if ( extra_descr_free == NULL )
	    {
	        ed			= (EXTRA_DESCR_DATA *) alloc_perm( sizeof(*ed) );
	    }
	    else
	    {
	    	ed			= extra_descr_free;
	    	extra_descr_free	= ed->next;
	    }

	    ed->keyword		= str_dup( arg3     );
	    ed->description	= str_dup( argument );
	    ed->next		= obj->extra_descr;
	    obj->extra_descr	= ed;
	    return;
    	}
    }
    
    	
    /* echo bad use message */
    do_string(ch,"");
}



void do_oset( CHAR_DATA *ch, char *argument ) {
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\r\n",ch);
	send_to_char("  set obj <object> <field> <value>\r\n",ch);
	send_to_char("  Field being one of:\r\n",				ch );
	send_to_char("    value0 value1 value2 value3 value4 (v1-v4)\r\n",	ch );
	send_to_char("    extra wear level weight cost timer size condition\r\n",		ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "Nothing like that in heaven or earth.\r\n", ch );
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi( arg3 );

    /*
     * Set something.
     */
	
	if ( !str_cmp( arg2, "condition" ))
	{
	set_obj_cond (obj, (URANGE(1, value, 100)) );
	send_to_char ("Condition set.\r\n",ch);
	return;
	}

    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
	obj->value[0] = UMIN(1000,value);
	obj->valueorig[0] = UMIN(1000,value);
	return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
	obj->value[1] = value;
	obj->valueorig[1] = value;
	return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
	obj->value[2] = value;
	obj->valueorig[2] = value;
	return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
	obj->value[3] = value;
	obj->valueorig[3] = value;
	return;
    }

    if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
	obj->value[4] = value;
	obj->valueorig[4] = value;
	return;
    }

    if ( !str_prefix( arg2, "extra" ))   {
	obj->extra_flags = value;
	return;
    }

    if ( !str_prefix( arg2, "wear" ) )
    {
	obj->wear_flags = value;
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	obj->level = value;
	return;
    }
	
    if ( !str_prefix( arg2, "weight" ) )
    {
	obj->weight = value;
	return;
    }

    if ( !str_prefix( arg2, "cost" ) )
    {
	obj->cost = value;
	return;
    }

    if ( !str_prefix( arg2, "timer" ) )
    {
	obj->timer = value;
	return;
    }
	
	if (!str_prefix( arg2, "size" ) )
	{
	char buf[128];
	if (value < 0 || value > 5)
		{
		send_to_char ("Size range is 0-5.\r\n",ch);
		return;
		}
	obj->size = value;
	sprintf (buf, "Size set to %s.\r\n", size_table[value]);
	send_to_char (buf, ch);
	return;
	}
	
    /*
     * Generate usage message.
     */
    do_oset( ch, "" );
    return;
}



void do_rset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\r\n",ch);
	send_to_char( "  set room <location> <field> <value>\r\n",ch);
	send_to_char( "  Field being one of:\r\n",			ch );
	send_to_char( "    flags sector\r\n",				ch );
	return;
    }

    if ( ( location = find_location( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No such location.\r\n", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\r\n", ch );
	return;
    }
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_prefix( arg2, "flags" ) )
    {
	location->room_flags	= value;
	return;
    }

    if ( !str_prefix( arg2, "sector" ) )
    {
	location->sector_type	= value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_rset( ch, "" );
    return;
}


bool duplicate_ip(DESCRIPTOR_DATA *d) {
DESCRIPTOR_DATA *ds;

     if (!d) return FALSE;

     for (ds = descriptor_list; ds; ds = ds->next) {
           if (ds == d) continue;
           if (!ds->character || !ds->ok) continue;

           if (!str_cmp(ds->host, d->host)) return TRUE;
     }

     return FALSE;
}


void do_sockets( CHAR_DATA *ch, char *argument ) {
char buf[2 * MAX_STRING_LENGTH];
char buf2[MAX_STRING_LENGTH];
char arg[MAX_INPUT_LENGTH];
char col[9];
DESCRIPTOR_DATA *d;
CHAR_DATA *vi;
int count = 0;

    buf[0]	= '\0';

    one_argument(argument,arg);
    for ( d = descriptor_list; d != NULL; d = d->next )    {
	if ( d->character != NULL && can_see( ch, d->character ) 
	&& (arg[0] == '\0' || is_name(arg,d->character->name)
			   || (d->original && is_name(arg,d->original->name))))	{

                    if (d->original) vi = d->original;
                    else vi = d->character;
                    if (!vi) continue;

	    count++;
                    
	    sprintf(buf2, "[%3d %15s] ", d->descriptor, flag_string(connect_type, d->connected));
                    strcat(buf, buf2);
                    if (duplicate_ip(d)) sprintf(col, "{Y");
                    else sprintf(col, "{x");

                    if (vi->timer > 0) sprintf(buf2, "%3d %s *** (%d) %s@%s%s{x\r\n", vi->level, race_array[vi->race].name, vi->timer, vi->name ? vi->name : "(none)", col, d->host);
  	    else sprintf(buf2, "%3d %s *** %s@%s%s{x\r\n", vi->level, race_array[vi->race].name, vi->name ? vi->name : "(none)", col, d->host);
                    strcat(buf, buf2);
	}
    }
    if (count == 0) {
	send_to_char("No one by that name is connected.\r\n",ch);
	return;
    }

    sprintf(buf2, "%d user%s\r\n", count, count == 1 ? "" : "s" );
    strcat(buf, buf2);
    page_to_char(buf, ch );
    return;
}


void do_force( CHAR_DATA *ch, char *argument ) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )    {
	send_to_char( "Force whom to do what?\r\n", ch );
	return;
    }

    one_argument(argument,arg2);
  
    sprintf( buf, "$n forces you to '%s'.", argument );

    if ( !str_cmp( arg, "all" ) )    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

                if (!str_cmp(arg2,"delete"))    {
	       send_to_char("That will NOT be done.\r\n",ch);
     	       return;
                }

	if ( !IS_GREATER(ch) 
                && !IS_IMP(ch))	{
	    send_to_char("You may not do that...\r\n",ch);
	    return;
	}

	for ( vch = char_list; vch != NULL; vch = vch_next )	{
	    vch_next = vch->next;

	    if ( !IS_NPC(vch) && get_divinity(vch) < get_divinity(ch))	    {
		act( buf, ch, NULL, vch, TO_VICT );
		interpret( vch, argument );
	    }
	}
    }else if (!str_cmp(arg,"players"))  {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
                if (!str_cmp(arg2,"delete"))    {
	       send_to_char("That will NOT be done.\r\n",ch);
     	       return;
                }

        if ( !IS_GREATER(ch)
        && !IS_IMP(ch))        {
            send_to_char("You may not do that...\r\n",ch);
            return;
        }
 
        for ( vch = char_list; vch != NULL; vch = vch_next )        {
            vch_next = vch->next;
 
            if ( !IS_NPC(vch) 
            &&  get_divinity(vch) < get_divinity(ch) 
            &&  IS_MORTAL(vch))            {
                act( buf, ch, NULL, vch, TO_VICT );
                interpret( vch, argument );
            }
        }
    }    else if (!str_cmp(arg,"gods"))    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
                if (!str_cmp(arg2,"delete"))    {
	       send_to_char("That will NOT be done.\r\n",ch);
     	       return;
                }

        if ( !IS_GREATER(ch)
        && !IS_IMP(ch) )        {
            send_to_char("You may not do that...\r\n",ch);
            return;
        }
 
        for ( vch = char_list; vch != NULL; vch = vch_next )        {
            vch_next = vch->next;
 
            if ( !IS_NPC(vch) 
              &&  get_divinity(vch) < get_divinity(ch)
              &&  IS_IMMORTAL(vch))   {
                act( buf, ch, NULL, vch, TO_VICT );
                interpret( vch, argument );
            }
        }
    }    else    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )	{
	    send_to_char( "They aren't here.\r\n", ch );
	    return;
	}

	if ( victim == ch )	{
	    send_to_char( "Aye aye, right away!\r\n", ch );
	    return;
	}

                if (!str_cmp(arg2,"delete"))    {
                      if (!IS_IMP(ch)
                      || IS_IMMORTAL(victim)) {
       	             send_to_char("That will NOT be done.\r\n",ch);
	             return;
                      }
                }

	if (get_divinity(victim) >= get_divinity(ch))	{
	    send_to_char( "You do not have the power...\r\n", ch );
	    return;
	}

	if ( !IS_NPC(victim) 
                && !IS_GREATER(ch)
                && !IS_IMP(ch) ) {
	    send_to_char("You may not do that...\r\n",ch);
	    return;
	}

	act( buf, ch, NULL, victim, TO_VICT );
	interpret( victim, argument );
    }

    send_to_char( "Ok.\r\n", ch );
    return;
}


int invis_lev(CHAR_DATA *ch) {

  return ch->level;
}

/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, char *argument )
{
    int level, max_level;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
	return;

    /* RT code for taking a level argument */
    one_argument( argument, arg );

    if ( arg[0] == '\0' ) { 
    /* take the default path */

      if ( IS_SET(ch->plr, PLR_WIZINVIS) ) {
	  REMOVE_BIT(ch->plr, PLR_WIZINVIS);
	  ch->invis_level = 0;
	  act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
	  send_to_char( "You slowly fade back into existence.\r\n", ch );
      } else {
	  SET_BIT(ch->plr, PLR_WIZINVIS);
	  ch->invis_level = invis_lev(ch);
	  act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
	  send_to_char( "You slowly vanish into thin air.\r\n", ch );
      }
    } else {
    /* do the level thing */
    
      level = atoi(arg);
      max_level = invis_lev(ch);
      if (level < 2 || level > max_level) {
        sprintf(buf, "Invis level must be between 2 and %d.\r\n", max_level);
	send_to_char(buf,ch);
        return;
      } else {
	ch->reply = NULL;
        SET_BIT(ch->plr, PLR_WIZINVIS);
        ch->invis_level = level;
        act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
        send_to_char( "You slowly vanish into thin air.\r\n", ch );
      }
    }

    return;
}

void do_cloak( CHAR_DATA *ch, char *argument )
{
    int level, max_level;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
	return;

    /* RT code for taking a level argument */
    one_argument( argument, arg );

    if ( arg[0] == '\0' ) { 
    /* take the default path */

      if ( IS_SET(ch->plr, PLR_CLOAK) ) {
	  REMOVE_BIT(ch->plr, PLR_CLOAK);
	  ch->cloak_level = 0;
	  act( "$n's immortal cloak vanishes.", ch, NULL, NULL, TO_ROOM );
	  send_to_char( "You dismiss your mystical cloak.\r\n", ch );
      } else {
	  SET_BIT(ch->plr, PLR_CLOAK);
	  ch->cloak_level = invis_lev(ch);
	  act( "$n calls forth a mystic cloak to hide $s presence.", ch, NULL, NULL, TO_ROOM );
	  send_to_char( "You call forth a mystic cloak to hide your presence.\r\n", ch );
      }
    } else {
    /* do the level thing */
    
      level = atoi(arg);
      max_level = invis_lev(ch); 
      if (level < 2 || level > max_level)
      {
        sprintf(buf, "Cloak level must be between 2 and %d.\r\n", max_level);
	send_to_char(buf, ch);
    	return;
      }
      else
      {
	ch->reply = NULL;
      	SET_BIT(ch->plr, PLR_CLOAK);
      	ch->cloak_level = level;
	act( "$n calls forth a mystic cloak to hide $s presence.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "You call forth a mystic cloak to hide your presence.\r\n", ch );
      }
    }

    return;
}

void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->plr, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->plr, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode off.\r\n", ch );
    }
    else
    {
	SET_BIT(ch->plr, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode on.\r\n", ch );
    }

    return;
}

void do_world (CHAR_DATA *ch, char *argument)
	{
	char arg[MAX_INPUT_LENGTH];
	char output[MAX_STRING_LENGTH*4];
	char buf[256];
	OBJ_INDEX_DATA *obj;
	MOB_INDEX_DATA *mob;
	bool check_mob = FALSE;
	int level[71];
	AREA_DATA *pArea;
	int counter;

	for (counter = 0 ; counter < 71 ; counter++)
		level[counter]=0;

	if ( argument == NULL || argument[0] == '\0' )	
		{
		send_to_char ("Syntax:  world <obj | mob>\r\n",ch);
		return;
		}
	
	one_argument (argument, arg);
	
	if (!str_cmp(arg, "mob"))
		check_mob=TRUE;	
	else if (!str_cmp(arg, "obj"))
		check_mob=FALSE;
	else
		{
		send_to_char ("Syntax:  world <obj | mob>\r\n",ch);
		return;
		}

	for (pArea = area_first ; pArea ; pArea = pArea->next)
		{
		for (counter = pArea->lvnum ; counter <= pArea->uvnum ; counter++)
			{
			if (check_mob)
				{
				if ((mob = get_mob_index (counter) ) == NULL)
					continue;
				if (mob->level > 70)
					level[70]++;
				else
					level[mob->level]++;
				}
			else
				{
				if ((obj = get_obj_index (counter) ) == NULL) 
					continue;	
				if (obj->level > 70)
					level[70]++;
				else
					level[obj->level]++;
				}
			} /* end for counter loop */
		} /* end for pArea loop */ 
				
	/*generate output*/	
	if (check_mob)
		sprintf (output, "MOBILES [{cL<level>  {m<vnum count>{x]\r\n");
	else
		sprintf (output, "OBJECTS [{cL<level>  {m<vnum count>{x]\r\n");
	for (counter = 1 ; counter <= 69 ; counter++)
		{
		sprintf (buf, "[{cL%-3d  {m%4d{x]   ", counter, level[counter]);
		strcat  (output, buf);
		if (counter % 4 == 0)
			strcat (output, "\r\n");
		}
	sprintf (buf, "[{cL70+ {m%4d{x]\r\n", level[70]);
	strcat (output, buf);
	page_to_char (output, ch);
	}


void do_award (CHAR_DATA *ch, char *argument)	{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	char vict_messbuf[MAX_STRING_LENGTH];
	char imm_messbuf[MAX_STRING_LENGTH];
	int xp_modifier;

                if (IS_MUD(ch)) return;

	if (!ch->desc)  {
		send_to_char ("Switched immortals cannot use this command.\r\n",ch);
		return;
	}	
	
	if (argument[0] == '\0')	{
		send_to_char ("Syntax:  award <player> xp <xp>\r\n",ch);
            		send_to_char ("Syntax:  award <player> fame <fame>\r\n",ch);
            		send_to_char ("Syntax:  award <player> prac <pracs>\r\n",ch);
            		send_to_char ("Syntax:  award <player> train <trains>\r\n",ch);
		return;
	}

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);

	if ((victim = get_char_room (ch, arg1)) == NULL) {
		send_to_char ("That player is not in this room.\r\n",ch);
		return;
	}
	
	if (IS_NPC(victim)) {
		send_to_char ("You cannot reward mobiles.\r\n",ch);
		return;
	}
	
	if (IS_IMMORTAL(victim)) {
		send_to_char ("You cannot reward immortals.\r\n",ch);
		return;
	}
	
                if (is_number(arg2)) {
                      sprintf(arg3, "%d", atoi(arg2));
                      sprintf(arg2, "xp");
                }

                xp_modifier = atoi (arg3);

                if (xp_modifier == 0) {
		send_to_char ("<xp> argument must be a non-zero value between -10000 and 10000.\r\n",ch);
		return;
	}		
	
                if (!str_cmp(arg2, "xp")) {
	       if (xp_modifier > 10000 || xp_modifier < -10000) {
		send_to_char ("<xp> value must be between -10000 and 10000, excluding 0.\r\n",ch);
		return;
	       }
	
	       if (xp_modifier < 0) {
		sprintf (vict_messbuf, "%s has penalized you %d experience points!\r\n", (can_see (victim,ch) ? ch->name : "Someone"), xp_modifier );
		sprintf (imm_messbuf, "You penalize %s %d experience points!\r\n", victim->name, xp_modifier );
                       } else {
		sprintf (vict_messbuf, "%s has awarded you %d experience points!\r\n", (can_see (victim,ch) ? ch->name : "Someone"), xp_modifier );
		sprintf (imm_messbuf, "You award %s %d experience points!\r\n", victim->name, xp_modifier );
	       }

       	       gain_exp(victim, xp_modifier, FALSE);

                } else if (!str_cmp(arg2, "fame")) {
	       if (xp_modifier > 20 || xp_modifier < -20) {
		send_to_char ("<fame> value must be between -20 and 20, excluding 0.\r\n",ch);
		return;
	       }

                       if (IS_NPC(victim)) return;
	
	       if (xp_modifier < 0) {
		sprintf (vict_messbuf, "%s has penalized you %d fame points!\r\n", (can_see (victim,ch) ? ch->name : "Someone"), xp_modifier );
		sprintf (imm_messbuf, "You penalize %s %d fame points!\r\n", victim->name, xp_modifier );
                       } else {
		sprintf (vict_messbuf, "%s has awarded you %d fame points!\r\n", (can_see (victim,ch) ? ch->name : "Someone"), xp_modifier );
		sprintf (imm_messbuf, "You award %s %d fame points!\r\n", victim->name, xp_modifier );
	       }
              
                       victim->pcdata->questpoints = UMAX(victim->pcdata->questpoints + xp_modifier, 0);

                } else if (!str_cmp(arg2, "prac")) {
	       if (xp_modifier > 20 || xp_modifier < -20) {
		send_to_char ("<prac> value must be between -20 and 20, excluding 0.\r\n",ch);
		return;
	       }
	
	       if (xp_modifier < 0) {
		sprintf (vict_messbuf, "%s has penalized you %d pracs!\r\n", (can_see (victim,ch) ? ch->name : "Someone"), xp_modifier );
		sprintf (imm_messbuf, "You penalize %s %d pracs!\r\n", victim->name, xp_modifier );
                       } else {
		sprintf (vict_messbuf, "%s has awarded you %d pracs!\r\n", (can_see (victim,ch) ? ch->name : "Someone"), xp_modifier );
		sprintf (imm_messbuf, "You award %s %d pracs!\r\n", victim->name, xp_modifier );
	       }

                       victim->practice = UMAX(victim->practice + xp_modifier, 0);

                } else if (!str_cmp(arg2, "train")) {
	       if (xp_modifier > 5 || xp_modifier < -5) {
		send_to_char ("<fame> value must be between -5 and 5, excluding 0.\r\n",ch);
		return;
	       }
	
	       if (xp_modifier < 0) {
		sprintf (vict_messbuf, "%s has penalized you %d trains!\r\n", (can_see (victim,ch) ? ch->name : "Someone"), xp_modifier );
		sprintf (imm_messbuf, "You penalize %s %d trains!\r\n", victim->name, xp_modifier );
                       } else {
		sprintf (vict_messbuf, "%s has awarded you %d trains!\r\n", (can_see (victim,ch) ? ch->name : "Someone"), xp_modifier );
		sprintf (imm_messbuf, "You award %s %d trains!\r\n", victim->name, xp_modifier );
	       }

                       victim->train = UMAX(victim->train + xp_modifier, 0);

                } else {
		send_to_char ("Syntax:  award <player> xp <xp>\r\n",ch);
            		send_to_char ("Syntax:  award <player> fame <fame>\r\n",ch);
            		send_to_char ("Syntax:  award <player> prac <pracs>\r\n",ch);
            		send_to_char ("Syntax:  award <player> train <trains>\r\n",ch);
		return;
                }

	send_to_char (vict_messbuf, victim);
	send_to_char (imm_messbuf, ch);
	return;
}


void do_shell( CHAR_DATA *ch, char *argument ) {
    char buf[5000];
    FILE *fp;

    if (IS_MUD(ch)) return;

    if (argument[0] == '\0') {
            send_to_char("Syntax: SHELL <command>",ch);
            return;
    }

    fp = popen( argument, "r" );
    fgetf( buf, 5000, fp );
    page_to_char( buf, ch );
    pclose( fp );
    return;
}
 
char *fgetf( char *s, int n, register FILE *iop ) {
    register int c;
    register char *cs;
    c = '\0';
    cs = s;
    while( --n > 0 && (c = getc(iop)) != EOF)
	if ((*cs++ = c) == '\0')
	    break;
    *cs = '\0';
     return((c == EOF && cs == s) ? NULL : s);
}

void do_rename (CHAR_DATA* ch, char* argument) {
	char old_name[MAX_INPUT_LENGTH], 
                new_name[MAX_INPUT_LENGTH],
	strsave [MAX_INPUT_LENGTH];

	CHAR_DATA* victim;
	FILE* file;
	
	argument = one_argument(argument, old_name); /* find new/old name */
	one_argument (argument, new_name);
	
	/* Trivial checks */
	if (!old_name[0])	{
		send_to_char ("Rename who?\r\n",ch);
		return;
	}
	
	victim = get_char_world (ch, old_name);
	
	if (!victim)	{
		send_to_char ("There is no such a person online.\r\n",ch);
		return;
	}
	
	if (IS_NPC(victim)) {   
		send_to_char ("You cannot use Rename on NPCs.\r\n",ch);
		return;
	}

	/* allow rename self new_name,but otherwise only lower level */	
	if ( (victim != ch) && (get_trust (victim) >= get_trust (ch)) )	{
		send_to_char ("You failed.\r\n",ch);
		return;
	}
	
	if (!victim->desc || (victim->desc->connected != CON_PLAYING) )	{
		send_to_char ("This player has lost his link or is inside a pager or the like.\r\n",ch);
		return;
	}

	if (!new_name[0])	{
		send_to_char ("Rename to what new name?\r\n",ch);
		return;
	}
	
	if (!check_parse_name(new_name))	{
		send_to_char ("The new name is illegal.\r\n",ch);
		return;
	}

	/* First, check if there is a player named that off-line */	

        sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( new_name ) );

	fclose (fpReserve); /* close the reserve file */
	file = fopen (strsave, "r"); /* attempt to to open pfile */
	if (file)	{
		send_to_char ("A player with that name already exists!\r\n",ch);
		fclose (file);
    	fpReserve = fopen( NULL_FILE, "r" ); /* is this really necessary these days? */
		return;		
	}
   	fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */

	/* Check .gz file ! */

        sprintf( strsave, "%s%s.gz", PLAYER_DIR, capitalize( new_name ) );

	fclose (fpReserve); /* close the reserve file */
	file = fopen (strsave, "r"); /* attempt to to open pfile */
	if (file)	{
		send_to_char ("A player with that name already exists in a compressed file!\r\n",ch);
		fclose (file);
     	                fpReserve = fopen( NULL_FILE, "r" ); 
		return;		
	}
   	fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */

	if (get_char_world(ch,new_name)) 	{
		send_to_char ("A player with the name you specified already exists!\r\n",ch);
		return;
	}

	/* Save the filename of the old name */

      sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( victim->name ) );

	/* Rename the character and save him to a new file */
	/* NOTE: Players who are level 1 do NOT get saved under a new name */

	free_string (victim->name);
                free_string (victim->short_descr);
	victim->name = str_dup (capitalize(new_name));
                victim->short_descr = str_dup (capitalize(new_name));

	save_char_obj (victim);
	
	/* unlink the old file */
	unlink (strsave); /* unlink does return a value.. but we do not care */

	/* That's it! */
	
	send_to_char ("Character renamed.\r\n",ch);

	victim->position = POS_STANDING; /* I am laaazy */
	act ("$n has renamed you to $N!",ch,NULL,victim,TO_VICT);
			
}



void do_distribute( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj; 
    OBJ_INDEX_DATA *pObjIndex;
    char buf[MAX_STRING_LENGTH];

    if (IS_MUD(ch)) return;
        
    one_argument( argument, arg);

    if ( arg[0] == '\0' )    {
	send_to_char("Syntax: distribute <vnum>\r\n",ch);
	return;
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg ) ) ) == NULL )    {
	send_to_char( "No object has that vnum.\r\n", ch );
	return;
    }

    sprintf(buf, "You distribute %s.\r\n", pObjIndex->short_descr);
    send_to_char(buf, ch);

    for (victim = char_list; victim != NULL; victim = victim->next) {
          if (!IS_NPC(victim)
          && ch != victim) {
              obj = create_object( pObjIndex, 1 );
              obj->size = get_char_size(victim);
              obj->level = 1;
              if (CAN_WEAR(obj, ITEM_TAKE))	{
                   obj_to_char( obj, victim);
                   sprintf(buf,"%s created %s for you.\r\n",ch->name, obj->short_descr);
                   send_to_char(buf, victim);
              }
          }
    }
    return;
}


void do_forceremove( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *victim;
	
    argument = one_argument( argument, arg );
    victim = get_char_room(ch, arg);

    if (victim == NULL) {
	send_to_char( "That character is not here.\r\n", ch );
	return;
    }

    if (!IS_NPC(victim) && get_divinity(victim) >= get_divinity(ch) && victim !=ch)    {
	send_to_char( "You are not allowed to do that..\r\n", ch );
	return;
    }


    argument = one_argument( argument, arg2);
    if ( arg2[0] == '\0' )   {
	send_to_char( "Remove what?\r\n", ch );
	return;
    }

     if ( (obj = get_obj_wear( victim, arg2) ) == NULL )	{
        	send_to_char( "There is no such object.\r\n", ch );
                return;
     }

      act( "$N forces you to remove & drop $p.", victim, obj, ch, TO_CHAR );
      unequip_char(victim, obj );
      obj_from_char(obj );
      obj_to_room( obj, victim->in_room );
      send_to_char( "OK.\r\n", ch );
      return;
}


void do_check( CHAR_DATA *ch, char *argument ) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char buffer[8*MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int count = 1;
    
    one_argument( argument, arg );
    
    if (arg[0] == '\0'|| !str_prefix(arg,"stats"))    {

                buffer[0] = '\0';

    	for (victim = char_list; victim != NULL; victim = victim->next)    	{
    	    if (IS_NPC(victim) || !can_see(ch,victim)) continue;
    	    	
	    if (victim->desc == NULL)	    {
	    	sprintf(buf,"%3d) %s is linkdead.\r\n", count, victim->name);
	    	strcat(buffer, buf);
	    	count++;
	    	continue;	    	
	    }
	    
	    if (victim->desc->connected >= CON_GET_NEW_RACE
	     && victim->desc->connected <= CON_GET_AMATEUR)	    {
	    	sprintf(buf,"%3d) %s is being created.\r\n",
 	    	    count, victim->name);
	    	strcat(buffer, buf);
	    	count++;
	    	continue;
	    }
	    
	    if ( (victim->desc->connected == CON_GET_OLD_PASSWORD
	       || victim->desc->connected >= CON_READ_IMOTD)
	     && get_trust(victim) <= get_trust(ch) )	    {
	    	sprintf(buf,"%3d) %s is connecting.\r\n", count, victim->name);
	    	strcat(buffer, buf);
	    	count++;
	    	continue; 	    		 
	    }
	    
	    if (victim->desc->connected == CON_PLAYING)    {
	        if (get_trust(victim) > get_trust(ch)) sprintf(buf,"%3d) %s.\r\n", count, victim->name);
	        else {
		    sprintf(buf,"%3d) %s, Level %d connected since %d hours (%d total hours)\r\n", count, victim->name,victim->level, ((int)(current_time - victim->pcdata->logon)) /3600, (victim->pcdata->played + (int)(current_time - victim->pcdata->logon)) /3600 );
		    strcat(buffer, buf);
		    if (arg[0]!='\0' && !str_prefix(arg,"stats"))	    {
                                      if (IS_NPC(victim)) {
  		          sprintf(buf,"  %d HP %d Mana (%d %d %d %d %d) %ld cash %d Tr %d Pr.\r\n",
                                                victim->max_hit, victim->max_mana,victim->perm_stat[STAT_STR],
                                                victim->perm_stat[STAT_INT],victim->perm_stat[STAT_WIS],
		    	victim->perm_stat[STAT_DEX],victim->perm_stat[STAT_CON],
		    	get_full_cash(victim),
		    	victim->train, victim->practice);
                                      } else {
  		          sprintf(buf,"  %d HP %d Mana (%d %d %d %d %d) %ld cash %d Tr %d Pr %d Qpts.\r\n",
                                                victim->max_hit, victim->max_mana,victim->perm_stat[STAT_STR],
                                                victim->perm_stat[STAT_INT],victim->perm_stat[STAT_WIS],
		    	victim->perm_stat[STAT_DEX],victim->perm_stat[STAT_CON],
		    	get_full_cash(victim),
		    	victim->train, victim->practice, victim->pcdata->questpoints);
                                      }
		      strcat(buffer, buf);
		    }
		    count++;
		}
	        continue;
	    }
	    
	    sprintf(buf,"%3d) bug (oops)...please report to Admin: %s %d\r\n", count, victim->name, victim->desc->connected);
	    strcat(buffer, buf);
	    count++;   
    	}
    	page_to_char(buffer,ch);
    	return;
    }
    
    if (!str_prefix(arg,"eq"))    {

                buffer[0] = '\0';

    	for (victim = char_list; victim != NULL; victim = victim->next)    	{
    	    if (IS_NPC(victim) 
    	     || victim->desc->connected != CON_PLAYING
    	     || !can_see(ch,victim)
    	     || get_trust(victim) > get_trust(ch) )
    	    	continue;
    	    	
    	    sprintf(buf,"%3d) %s, %d items (weight %d) Hit:%d Dam:%d Save:%d AC:%d %d %d %d.\r\n",
    	    	count, victim->name, victim->carry_number, victim->carry_weight, 
    	    	victim->hitroll, victim->damroll, victim->saving_throw,
    	    	victim->armor[AC_PIERCE], victim->armor[AC_BASH],
    	    	victim->armor[AC_SLASH], victim->armor[AC_EXOTIC]);
    	    strcat(buffer, buf);
    	    count++;  
    	}
    	page_to_char(buffer,ch);
    	return;
    }

  if (!str_prefix(arg,"snoop"))     {
        char bufsnoop [100];

                buffer[0] = '\0';

        for (victim = char_list; victim != NULL; victim = victim->next)        {
            if (IS_NPC(victim)) continue;
            if (victim->desc == NULL) continue;
            if (victim->desc->connected != CON_PLAYING
             || !can_see(ch,victim)
             || get_trust(victim) > get_trust(ch) )
                continue;
         
            if(victim->desc->snoop_by != NULL)  sprintf(bufsnoop," %15s .",victim->desc->snoop_by->character->name);
            else sprintf(bufsnoop,"     (none)      ." );

            sprintf(buf,"%3d %15s : %s \r\n",count,victim->name, bufsnoop);
            strcat(buffer, buf);
            count++;
        }
        page_to_char(buffer,ch);
        return;
    }

        
    send_to_char("Syntax: 'check'       display info about players\r\n",ch);
    send_to_char("        'check stats' display info and resume stats\r\n",ch);
    send_to_char("        'check eq'    resume eq of all players\r\n",ch);
    send_to_char("        'check snoop'    displays active snoops\r\n",ch);
    return;
}


void do_pload( CHAR_DATA *ch, char *argument ) {
  DESCRIPTOR_DATA d;
  CHAR_DATA *victim;
  bool isChar = FALSE;
  char name[MAX_INPUT_LENGTH];

  if (argument[0] == '\0')  {
    send_to_char("Load who?\r\n", ch);
    return;
  }

  argument[0] = UPPER(argument[0]);
  argument = one_argument(argument, name);

  if ((victim = get_char_world_player( ch, name )) != NULL )  {
    if (!IS_NPC(victim)) return;
  }

  isChar = load_char_obj(&d, name);

  if (!isChar)  {
    send_to_char("I cant seem to find them.\r\n", ch);
    return;
  }

  d.character->desc     = NULL;
  d.character->next     = char_list;
  char_list                     = d.character;
  d.connected              = CON_PLAYING;
  reset_char(d.character);

  return;
}


void do_punload( CHAR_DATA *ch, char *argument ) {
  CHAR_DATA *victim;
  char who[MAX_INPUT_LENGTH];

  argument = one_argument(argument, who);

  if ( ( victim = get_char_world_player( ch, who ) ) == NULL )  {
    send_to_char( "They aren't here.\r\n", ch );
    return;
  }

  if (victim->desc != NULL)  return;

  do_quit(victim,"");
  return;
}


void do_olevel(CHAR_DATA *ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    char level[MAX_STRING_LENGTH];
    char buffer[10*MAX_STRING_LENGTH];
    char vb[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found = FALSE;
    int number = 0;
    int max_found = 30;
    int llev = 1;
    int ulev = 150;

    argument = one_argument(argument, level);
    if (level[0] == '\0')    {
        send_to_char("Syntax: olevel <low> <high>\r\n",ch);
        return;
    }
    llev=atoi(level);
    argument = one_argument(argument, level);
    if (level[0] == '\0')    {
        send_to_char("Syntax: olevel <low> <high>\r\n",ch);
        return;
    }
    ulev=atoi(level);

    sprintf(buffer, "Objects of level %d - %d\r\n",llev, ulev);

    for ( obj = object_list; obj != NULL; obj = obj->next )    {
        if ( obj->level < llev )  continue;
        if ( obj->level > ulev )  continue;
        if (number > max_found) continue;
        found = TRUE;
        number++;
 
        for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj );
 
        if ( in_obj->carried_by != NULL
          && can_see(ch,in_obj->carried_by)
          && in_obj->carried_by->in_room != NULL) {
            format_vnum(in_obj->carried_by->in_room->vnum, vb);
            sprintf( buf, "%3d) %s is carried by %s [Room %11s]\r\n",
                           number,
                           obj->short_descr,
                           PERS(in_obj->carried_by, ch),
                           vb );
        } else if ( in_obj->in_room != NULL
                 && can_see_room(ch,in_obj->in_room)) {
            format_vnum(in_obj->in_room->vnum, vb);
            sprintf( buf, "%3d) %s is in %s [Room %11s]\r\n",
                           number,
                           obj->short_descr,
                           in_obj->in_room->name,
                           vb);
        } else {
             sprintf( buf, "%3d) %s is somewhere\r\n",number, obj->short_descr);
        }
 
        buf[0] = UPPER(buf[0]);
        strcat(buffer,buf);

        if (number >= max_found) {
          break;
        }
    }
 
    if ( !found ) {
        send_to_char( "Nothing like that in heaven or earth.\r\n", ch );
    } else {
        page_to_char(buffer,ch);
    }

    return;
}


void do_mlevel( CHAR_DATA *ch, char *argument ) {
    char buf[MAX_STRING_LENGTH];
    char buffer[10*MAX_STRING_LENGTH];
    char level[MAX_STRING_LENGTH];
    char vb[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    bool found;
    int max_found = 30;
    int number = 0;
    int llev = 1;
    int ulev = 150;

    found = FALSE;

    argument = one_argument(argument, level);
    if (level[0] == '\0')    {
        send_to_char("Syntax: mlevel <low> <high>\r\n",ch);
        return;
    }
    llev=atoi(level);
    argument = one_argument(argument, level);
    if (level[0] == '\0')    {
        send_to_char("Syntax: mlevel <low> <high>\r\n",ch);
        return;
    }
    ulev=atoi(level);

    sprintf(buffer, "Mobs of level %d - %d\r\n",llev, ulev);

    for ( victim = char_list; victim != NULL; victim = victim->next )    {
        if ( victim->in_room != NULL) {
            if ( victim->level < llev )  continue;
            if ( victim->level > ulev )  continue;
            if (number > max_found) continue;
            found = TRUE;
            number++;

            if (!IS_NPC(victim)) {
              sprintf(vb, "0");
            } else {
              format_vnum(victim->pIndexData->vnum, vb);
            } 

            sprintf( buf, "%3d) [%11s] %-28s", 
                           number,  
                           vb,
                           IS_NPC(victim) ? victim->short_descr : victim->name );
            strcat(buffer,buf);

            format_vnum(victim->in_room->vnum, vb);

            sprintf( buf, " [%11s] %s\r\n", 
                           vb,
                           victim->in_room->name );
            strcat(buffer,buf);
        }
    }

    if ( !found ) act( "You didn't find any mob of level $T.", ch, NULL, argument, TO_CHAR );
    else page_to_char(buffer,ch);
    return;
}


int file_select(const struct direct   *entry){
      if ((str_cmp(entry->d_name, ".") == 0)
      || (str_cmp(entry->d_name, "..") == 0)) return (FALSE); 
     else return (TRUE);
}


void do_cleanup(CHAR_DATA *ch, char *argument ) {
char buf[MAX_INPUT_LENGTH];
char arg[MAX_INPUT_LENGTH];
CHAR_DATA *victim;
int count,i;
struct direct **files;
char *token;
char *lasttoken;
struct stat fst;
double tdif;
bool dodel = FALSE;

    if (IS_MUD(ch)) return;
    unlink("*.trans.gz");

  lasttoken = strdup("..");
  count = scandir(PLAYER_DIR, &files, file_select, alphasort);

  if (count <= 0) {
      send_to_char("No files in directory.\r\n", ch);
      return;
  }

  sprintf(buf,"Number of files = %d\n",count);
  send_to_char(buf,ch);

  for (i=1; i < count + 1;++i){
        dodel = FALSE;
        token = strtok(files[i-1]->d_name,".");
        if (str_cmp(token, lasttoken)
        && str_cmp(token, "Clan")
        && str_cmp(token, "Society")) {
            sprintf( buf, "%s/%s.gz", PLAYER_DIR, token );
            if (stat(buf, &fst ) == -1 )  {
                sprintf( buf, "%s/%s", PLAYER_DIR, token );
                 if (stat(buf, &fst ) == -1 )  {
                      sprintf(buf,"Problem with file %s.\r\n",token);
                      send_to_char(buf, ch );
                       return;
                 }
            }

            tdif = ((difftime( current_time,  fst.st_mtime ) / 24) / 60) / 60;

            if (tdif > 14) {
                 do_pload(ch, token);
                 one_argument(token, arg);
                 if ((victim = get_char_world( ch, arg )) == NULL )    {
                       send_to_char( "They aren't here.\r\n", ch );
                       return;
                 }

                  if (IS_HERO(victim) || IS_CREATOR(victim)) {
                         if (tdif > 180) dodel = TRUE;
                  } else if (IS_HUMAN(victim)) {
                         if (tdif > 60) dodel = TRUE;
                  } else if (IS_NEWBIE(victim)) {
                         dodel = TRUE;
                  } else dodel = FALSE;

                  sprintf(buf, "%s => %f days offline.\r\n", token, tdif);
                  send_to_char(buf,ch);                

                 if (dodel) {
                       delete_accounts(victim);
                       send_to_char("{rDELETED...{x\r\n",ch);                
                }

                if (dodel) {
                    victim->pcdata->confirm_delete = TRUE;
                    do_delete(victim, "");
                } else do_punload(ch, token);

                 lasttoken = strdup(token);
            }
        }
     }
     return;
}


void do_incarnate(CHAR_DATA *ch, char *argument ) {

    if (is_affected(ch, gafn_incarnated)) {
       send_to_char("You're already forced into a mortal incarnation.\r\n",ch);                
       return;
    }

    if (IS_AFFECTED(ch, AFF_INCARNATED)) {
       REMOVE_BIT(ch->affected_by, AFF_INCARNATED);
       send_to_char("You are immortal again.\r\n",ch);                
    } else {
         SET_BIT(ch->affected_by, AFF_INCARNATED);
         send_to_char("You incarnate in a mortal form.\r\n",ch);                
    }

    return;
}


void do_remort(CHAR_DATA *ch, char *argument) {
CHAR_DATA *victim;
char arg[MAX_STRING_LENGTH];
char buf[MAX_STRING_LENGTH];
  
  one_argument(argument,arg);

  if (arg[0] == '\0') {
    send_to_char("Remort who?\r\n", ch);
    return;
  } 

  victim = get_char_room(ch, argument);
  if (victim == NULL) {
    send_to_char("That character isn't here.\r\n",ch);
    return;
  } 

  if (IS_NPC(victim)) {
    send_to_char("You can't remort NPCs.\r\n",ch);
    return;
  } 

  if (IS_IMMORTAL(victim)) {
    send_to_char("You can't remort Immortals.\r\n",ch);
    return;
  } 

  if (victim->level < REMORT_LEVEL) {
    send_to_char("That character is too low level.\r\n",ch);
    return;
  } 

  sprintf(buf, "%s remove all", victim->name);
  do_force(ch, buf);
  victim->level = 1;
  victim->max_hit /=2;
  victim->max_mana /=2;
  victim->max_move /=2;
  victim->hit = victim->max_hit;
  victim->mana = victim->max_mana;
  victim->move = victim->max_move;
  if (victim->pcdata) {
      victim->pcdata->perm_hit = victim->max_hit;
      victim->pcdata->perm_mana = victim->max_mana;
      victim->pcdata->perm_move = victim->max_move;
  }
  sprintf_to_world("{BNotify{r-{x %s remorted.\r\n", victim->name);
  do_save(victim,"");
  sprintf(buf, "%s quit", victim->name);
  do_force(ch, buf);
  return;
}


void do_connect(CHAR_DATA *ch, char *argument ) {
char arg1[MAX_STRING_LENGTH];
int rn;

   one_argument(argument, arg1);

    if (!str_cmp(arg1, "list")) {
        list_partners(ch);
        return;
    }

    rn = get_partner_rn(arg1);
    if (rn < 0) {
       send_to_char("This partner doesn't exist.\r\n",ch);                
       return;
    }

    connect_single_partner(rn);
    return;
}


void do_version(CHAR_DATA *ch, char *argument ) {
char buf[MAX_STRING_LENGTH];
char arg1[MAX_STRING_LENGTH];
CHAR_DATA *much;
int rn;

    if (argument[0] == '\0') {
         sprintf_to_char(ch, "{gCurrent Driver Version:{x %s\r\n", DRIVER_VERSION);
         return;
    }

    one_argument(argument, arg1);
    if ((much = get_char_world(ch, arg1)) == NULL) {
         send_to_char("That partner does not exist!\r\n",ch);
         return;
    }

    rn = am_i_partner(much);
    if (rn < 0) {
         send_to_char("That partner does not exist!\r\n",ch);
         return;
    }

    if (partner_array[rn].fd < 0) {
         send_to_char("That partner does not exist!\r\n",ch);
         return;
    }

    sprintf(buf, "mudversion %s\r\n", ch->name);
    write_mud (partner_array[rn].fd, buf);
    return;
}


void do_authorize( CHAR_DATA *ch, char *argument ) {
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
CHAR_DATA *victim;
int div;

    if (IS_MUD(ch)) return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (arg1[0] == '\0')    {
	send_to_char( "Syntax: authority <char> <flag>\r\n", ch );
	send_to_char( "        builder, questor, enforcer, admin\r\n", ch );
	return;
    }

    if ((victim = get_char_room( ch, arg1 ) ) == NULL )    {
	send_to_char( "That player is not here.\r\n", ch);
	return;
    }

    if (victim == ch) {
	send_to_char( "That's not nescessary.\r\n", ch);
	return;
    }

    if (IS_NPC(victim)) {
	send_to_char( "Not on an NPC.\r\n", ch);
	return;
    }

    if (!IS_IMMORTAL(victim)) {
	send_to_char( "Not on a MORTAL.\r\n", ch);
	return;
    }

    if (arg2[0] == '\0')    {
	sprintf_to_char(ch, "%ss authority:\r\n",victim->name );
	sprintf_to_char(ch, "     {Y%s{x\r\n", flag_string(imm_auth, get_authority(victim)));
	return;
    }
    
    if ((div = flag_value(imm_auth, arg2)) == 0) {
	send_to_char( "Invalid Authority.\r\n", ch);
	return;
    }

    if (IS_SET(victim->pcdata->authority, div)) {
         if (IS_IMP(victim)) return;
         REMOVE_BIT(victim->pcdata->authority, div);
         sprintf_to_char(ch, "%s authority removed.\r\n", capitalize(flag_string(imm_auth, div)));
         sprintf_to_char(victim, "You lose auth %s.\r\n", capitalize(flag_string(imm_auth, div)));
    } else {
         SET_BIT(victim->pcdata->authority, div);
         sprintf_to_char(ch, "%s authority set.\r\n", capitalize(flag_string(imm_auth, div)));
         sprintf_to_char(victim, "You gain auth %s.\r\n", capitalize(flag_string(imm_auth, div)));
    }

    return;
}


void do_become( CHAR_DATA *ch, char *argument ) {
char modpass[MAX_INPUT_LENGTH];
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
CHAR_DATA *victim;
CHAR_DATA *spouse;
ROOM_INDEX_DATA *start_room;
ROOM_INDEX_DATA *start_room2;
DESCRIPTOR_DATA *d = ch->desc;
MOB_CMD_CONTEXT *mcc;
WEV *wev;
int transdiv;

    if (IS_MUD(ch)) return;
    if (IS_NPC(ch)) return;
    if (!d) return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (arg1[0] == '\0' || arg2[0] == '\0')    {
	send_to_char( "Syntax: become <char> <password>\r\n", ch );
	return;
    }

    if ((victim = get_char_world_player(ch, arg1)) != NULL) {
	send_to_char( "That player is already connected.\r\n", ch );
	return;
    }
 
     if (d->original) {
	send_to_char( "You are switched - return first.\r\n", ch );
	return;
    }

    save_char_obj(ch);
    transdiv = ch->pcdata->divinity;

    d->original = ch;
    d->character = NULL;
    ch->desc = NULL;

    if (load_char_obj(d, arg1) == FALSE) {
	write_to_descriptor(d->descriptor, "Character not found!.\r\n", 0);
                ch->desc = d;
                d->character = ch;            
                d->original = NULL;
	return;
    }

    victim = d->character;
    victim->desc = d;
    
    sprintf(modpass,"%s",victim->pcdata->pwd);

    if (str_cmp(arg2, modpass)
    && (!IS_IMMORTAL(ch) || str_cmp(arg2,"!") || !IS_SET(ch->pcdata->authority, IMMAUTH_ADMIN))) { 
         d->character = ch;
         ch->desc = d;
         d->original = NULL;
         victim->desc = NULL;
         if (victim->pet) extract_char(victim->pet, TRUE);
         extract_char(victim, FALSE);
         send_to_char("No authority. Login failed.\r\n", ch);
         return;
    }

    d->original = NULL;
    ch->desc = NULL;
    if (ch->pet) extract_char(ch->pet, TRUE);
    extract_char(ch, TRUE);

    if (victim->in_room) {
         start_room = victim->in_room;
    } else {
         write_to_descriptor(d->descriptor, "Character not found.\r\n", 0);
         close_socket(d, FALSE);
         return;
    }
    
    if ( IS_SET(time_info.flags, TIME_NIGHT) ) {
           if (start_room->night != 0) {
               start_room2 = get_room_index(start_room->night);
               if (start_room2 != NULL) start_room = start_room2;
           }

    } else if ( IS_SET(time_info.flags, TIME_DAY) ) {
           if (start_room->day != 0) {
               start_room2 = get_room_index(start_room->day);
                if (start_room2 != NULL) start_room = start_room2;
           }
    }

    victim->next	= char_list;
    char_list	= victim;
    reset_char(victim);

    char_to_room(victim, start_room );

    mcc = get_mcc(victim, victim, NULL, NULL, NULL, NULL, 0, NULL);
    wev = get_wev(WEV_CONTROL, WEV_CONTROL_LOGIN, mcc,
                     "{MYou step out of the shadows.{x\r\n",
                      NULL,
                     "{M@a2 steps out of the shadows.{x\r\n");

    room_issue_wev(victim->in_room, wev);
    free_wev(wev);

    if (!IS_IMMORTAL(victim)) notify_message (victim, NOTIFY_LOGIN, TO_ALL, NULL);
         
    do_look(victim, "auto" );
    if (str_cmp(victim->pcdata->spouse,"")) {
            if ((spouse = get_char_world(victim, victim->pcdata->spouse) ) != NULL) {
                 sprintf_to_char(ch, "{r%s{x is currently online...\r\n", spouse->name);
            }
    }

    if (victim->pet != NULL ) {
          if (victim->pet->in_room == NULL) char_to_room(victim->pet, ch->in_room);
          else char_to_room(victim->pet, victim->pet->in_room);

          mcc = get_mcc(victim->pet, victim->pet, NULL, NULL, NULL, NULL, 0, NULL);
          wev = get_wev(WEV_CONTROL, WEV_CONTROL_LOGIN, mcc,
                       "{MYou step out of the shadows.{x\r\n",
                        NULL,
                       "{M@a2 steps out of the shadows.{x\r\n");

          room_issue_wev(victim->pet->in_room, wev);
          free_wev(wev);
    }

    victim->pcdata->moddiv = transdiv;
    victim->desc->editor = 0;
    sprintf_to_char(ch, "You login as %s.\r\n", victim->name);
    return;
}


void do_cr( CHAR_DATA *ch, char *argument ) {
char arg1[MAX_INPUT_LENGTH];
OBJ_DATA *obj;

    if (IS_MUD(ch)) return;
    if (IS_NPC(ch)) return;

    argument = one_argument( argument, arg1 );

    for (obj = object_list; obj; obj = obj->next) {
          if (obj->item_type == ITEM_CORPSE_PC) {
                if (!str_cmp(obj->owner, arg1)) break;
          }
    }

    if (!obj) {
         sprintf_to_char(ch, "The corpse of %s could not be found.\r\n", capitalize(arg1));
         return;
    }

    if (obj->in_room) obj_from_room(obj);
    else if (obj->carried_by) obj_from_char(obj);
    else if (obj->in_obj) obj_from_obj(obj);

    obj_to_room(obj, ch->in_room);
    sprintf_to_room(ch->in_room, "{mA corpse is carried in by some strange robed creatures.{x\r\n");
    return;
}


void do_chunk(CHAR_DATA *ch, char *argument ) {
char arg1[MAX_INPUT_LENGTH];
char buf[MAX_STRING_LENGTH];
char outbuf[2*MAX_STRING_LENGTH];
int size;
VNUM i;
VNUM low = 0;
bool found = FALSE;

    if (IS_MUD(ch)) return;

    argument = one_argument(argument, arg1);
    if (arg1[0] == '\0') size = 0;
    else size = atoi (arg1);

     sprintf(outbuf, "Chunks bigger than %d vnums found:\r\n", size);
     for (i = 65; i < 65536; i++) {
        if (low == 0) {
           if (!get_vnum_area(i)) low = i;
        } else {          
           if (get_vnum_area(i) != NULL || i == 65535) {
                if (i - low -1 > size) {
                    sprintf(buf, "{g    From {C%5ld {gto {C%5ld{g.{x\r\n", low, i-1);
                    strcat(outbuf, buf);
                    found = TRUE;
                }
                low = 0;
           }
        }
     }

     if (!found) {
         send_to_char("No chunks of that size found.\r\n", ch);
         return;
     }

     page_to_char(outbuf, ch);
     return;
}

void inform_approval(CHAR_DATA *ch) {
CHAR_DATA *admin = highest_ranking_admin();

     if (admin) {
         send_to_char("{r********************************************{x\r\n", admin);
         sprintf_to_char(admin, "{r*{x APPROVAL for %15s nescessary! {r*{x\r\n" , ch->name);
         send_to_char("{r********************************************{x\r\n", admin);
     } else {
         for (admin = char_list; admin; admin = admin->next) {
           if (get_divinity(admin) < DIV_HERO || IS_MUD(admin)) continue;
           if (!IS_SET(admin->plr, PLR_HELPER) && !IS_SET(admin->comm, COMM_IMMACCESS)) continue;
           send_to_char("{r********************************************{x\r\n", admin);
           sprintf_to_char(admin, "{r*{x APPROVAL for %15s nescessary! {r*{x\r\n" , ch->name);
           send_to_char("{r********************************************{x\r\n", admin);
         }
     }
     return;
}


CHAR_DATA *highest_ranking_admin(void) {
CHAR_DATA *ch;
CHAR_DATA *admin = NULL;

     for (ch = char_list; ch; ch = ch->next) {
          if (!IS_IMMORTAL(ch)) continue;
          if (IS_SET(ch->plr, PLR_AFK)) continue;

          if (!admin) {
             admin = ch;
             continue;
          }

          if (get_divinity(ch) > get_divinity(admin)) {
              admin = ch;
          } else if (get_divinity(ch) == get_divinity(admin)) {
              if (get_authority(ch) > get_authority(admin)) {
                  admin = ch;
              }
          }
     }

     return admin;
}
    

void do_approve(CHAR_DATA *ch, char *argument ) {

    if (IS_MUD(ch)) return;

    if (mud.approval_timer == 0) {
         send_to_char("Approval is not nescessary.\r\n", ch);
         return;
    }

    if (argument[0] == '\0') {
       approve_list(ch);
       return;
    }

    if (!IS_IMMORTAL(ch)
    && highest_ranking_admin()) {
         send_to_char("Don't worry. There is an immortal on.\r\n", ch);
         return;
    }

    execute_approve(ch, argument, TRUE);
    return;
}


void do_disapprove(CHAR_DATA *ch, char *argument ) {

    if (IS_MUD(ch)) return;

    if (mud.approval_timer == 0) {
         send_to_char("Approval is not nescessary.\r\n", ch);
         return;
    }

    if (argument[0] == '\0') {
       approve_list(ch);
       return;
    }

    if (!IS_IMMORTAL(ch)
    && highest_ranking_admin()) {
         send_to_char("Don't worry. There is an immortal on.\r\n", ch);
         return;
    }

    execute_approve(ch, argument, FALSE);
    return;
}


void approve_list(CHAR_DATA *ch) {
DESCRIPTOR_DATA *d;
CHAR_DATA *vict;
bool ok = FALSE;

      send_to_char("Waiting for approval:\r\n", ch);
      for (d = descriptor_list; d; d = d->next) {
         if (d->character && !d->original) {
              vict = d->character;

              if (vict->desc->connected == CON_GET_NEW_PASSWORD
              || vict->desc->connected == CON_CONFIRM_NEW_PASSWORD
              || vict->desc->connected == CON_GET_NEW_RACE
              || vict->desc->connected == CON_MENU
              || (vict->desc->connected == CON_CHATTING
                 && (vict->desc->connected_old == CON_GET_NEW_PASSWORD
                        || vict->desc->connected_old == CON_CONFIRM_NEW_PASSWORD
                        || vict->desc->connected_old == CON_GET_NEW_RACE
                        || vict->desc->connected_old == CON_MENU))) {
                   if (vict->pcdata->store_number[0] == 0)  {
                       if (vict->race > 0) sprintf_to_char(ch, "{g%15s ({r%d{g){x\r\n", capitalize(vict->name), vict->pcdata->store_number[1]);
                       ok = TRUE;
                   }
              } else if (vict->desc->connected == CON_GET_PROFESSION
              || vict->desc->connected == CON_GET_NEW_SEX
              || vict->desc->connected == CON_GET_ALIGNMENT
              || vict->desc->connected == CON_GET_AMATEUR
              || vict->desc->connected == CON_CHECK_APPROVAL
              || (vict->desc->connected == CON_CHATTING
                && (vict->desc->connected_old == CON_GET_PROFESSION
                       || vict->desc->connected_old == CON_GET_NEW_SEX
                       || vict->desc->connected_old == CON_GET_ALIGNMENT
                       || vict->desc->connected_old == CON_GET_AMATEUR
                       || vict->desc->connected_old == CON_CHECK_APPROVAL))) {
                   if (vict->pcdata->store_number[0] == 0)  {
                       if (vict->race > 0) sprintf_to_char(ch, "{g%15s - %15s ({r%d{g){x\r\n", capitalize(vict->name), race_array[vict->race].name, vict->pcdata->store_number[1]);
                       ok = TRUE;
                   }
              }
         }
      }

      if (!ok) send_to_char("     {gNobody{x\r\n",ch);
      return;
}       


void execute_approve(CHAR_DATA *ch, char *argument, bool ok) {
char arg1[MAX_INPUT_LENGTH];
DESCRIPTOR_DATA *d;
CHAR_DATA *vict = NULL;

    argument = one_argument(argument, arg1);

    if (is_number(arg1)) {
      int j = atoi(arg1);

      for (d = descriptor_list; d; d = d->next) {
         if (d->character && !d->original) {
              vict = d->character;

              if (vict->desc->connected == CON_GET_NEW_PASSWORD
              || vict->desc->connected == CON_CONFIRM_NEW_PASSWORD
              || vict->desc->connected == CON_GET_NEW_RACE
              || vict->desc->connected == CON_GET_PROFESSION
              || vict->desc->connected == CON_GET_NEW_SEX
              || vict->desc->connected == CON_GET_ALIGNMENT
              || vict->desc->connected == CON_GET_AMATEUR
              || vict->desc->connected == CON_CHECK_APPROVAL
              || vict->desc->connected == CON_MENU
              || (vict->desc->connected == CON_CHATTING
                 && (vict->desc->connected_old == CON_GET_NEW_PASSWORD
                        || vict->desc->connected_old == CON_CONFIRM_NEW_PASSWORD
                        || vict->desc->connected_old == CON_GET_NEW_RACE
                        || vict->desc->connected_old == CON_MENU
                        ||vict->desc->connected_old == CON_GET_PROFESSION
                        || vict->desc->connected_old == CON_GET_NEW_SEX
                        || vict->desc->connected_old == CON_GET_ALIGNMENT
                        || vict->desc->connected_old == CON_GET_AMATEUR
                        || vict->desc->connected_old == CON_CHECK_APPROVAL))) {
                   if (vict->pcdata->store_number[0] == 0) {
                        if (--j == 0) break;
                   }
              }
         }
      }

      if (j > 0 || !vict) {
         send_to_char("Player not found.\r\n", ch);
         return;
      }

    } else {
      for (d = descriptor_list; d; d = d->next) {
         if (d->character && !d->original) {
              vict = d->character;

              if (vict->desc->connected == CON_GET_NEW_PASSWORD
              || vict->desc->connected == CON_CONFIRM_NEW_PASSWORD
              || vict->desc->connected == CON_GET_NEW_RACE
              || vict->desc->connected == CON_GET_PROFESSION
              || vict->desc->connected == CON_GET_NEW_SEX
              || vict->desc->connected == CON_GET_ALIGNMENT
              || vict->desc->connected == CON_GET_AMATEUR
              || vict->desc->connected == CON_CHECK_APPROVAL
              || vict->desc->connected == CON_MENU
              || (vict->desc->connected == CON_CHATTING
                 && (vict->desc->connected_old == CON_GET_NEW_PASSWORD
                        || vict->desc->connected_old == CON_CONFIRM_NEW_PASSWORD
                        || vict->desc->connected_old == CON_GET_NEW_RACE
                        || vict->desc->connected_old == CON_MENU
                        ||vict->desc->connected_old == CON_GET_PROFESSION
                        || vict->desc->connected_old == CON_GET_NEW_SEX
                        || vict->desc->connected_old == CON_GET_ALIGNMENT
                        || vict->desc->connected_old == CON_GET_AMATEUR
                        || vict->desc->connected_old == CON_CHECK_APPROVAL))) {

                   if (vict->pcdata->store_number[0] == 0) {
                        if (!str_cmp(vict->name, arg1)) break;
                   }
              }
         }
      }

      if (!vict || !d) {
         send_to_char("Player not found.\r\n", ch);
         return;
      }
    }

    if (vict->pcdata->store_number[0] == 1) {
        sprintf_to_char(ch, "%s has already been approved.\r\n", capitalize(vict->name));
        return;
    }

    if (vict->pcdata->store_number[0] == -1) {
        sprintf_to_char(ch, "%s has already been disapproved.\r\n", capitalize(vict->name));
        return;
    }

    if (ok) {
       vict->pcdata->store_number[0] = 1;
       sprintf_to_char(ch, "%s approved.\r\n", capitalize(vict->name));
    } else {
       vict->pcdata->store_number[0] = -1;
       sprintf_to_char(ch, "%s disapproved.\r\n", capitalize(vict->name));
       if (argument[0] != '\0') {
           free_string(vict->pcdata->questplr);
           vict->pcdata->questplr = str_dup(capitalize(argument));
       }
    }

    if (vict->desc->connected == CON_CHECK_APPROVAL) sprintf(vict->desc->inbuf, "\r\n");
    return;
}


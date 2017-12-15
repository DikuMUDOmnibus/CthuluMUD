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
#include "interp.h"
#include "conv.h"
#include "mob.h"
#include "exp.h"
#include "deeds.h"
#include "quests.h"
#include "wev.h"
#include "chat.h"
#include "spell.h"


void		raw_kill			(CHAR_DATA *victim, bool corpse);


char *mprog_type_to_name( int type ) {
    switch ( type )    {
    case IN_FILE_PROG:          return "in_file_prog";
    case ACT_PROG:              return "act_prog";
    case SPEECH_PROG:           return "speech_prog";
    case RAND_PROG:             return "rand_prog";
    case FIGHT_PROG:            return "fight_prog";
    case HITPRCNT_PROG:         return "hitprcnt_prog";
    case DEATH_PROG:            return "death_prog";
    case ENTRY_PROG:            return "entry_prog";
    case GREET_PROG:            return "greet_prog";
    case ALL_GREET_PROG:        return "all_greet_prog";
    case GIVE_PROG:             return "give_prog";
    case BRIBE_PROG:            return "bribe_prog";
    default:                    return "ERROR_PROG";
    }
}


//
// Subroutine to check authorization to use MPxxx commands
//

bool can_use_mp_commands(CHAR_DATA *ch) {

 /* Any NPC... */

  if ( IS_NPC(ch) ) {

   /* But switched NPCs must also pass the god check... */

    if ( ch->desc != NULL 
      && ch->desc->original != NULL
      && get_divinity(ch) <= DIV_LESSER ) {
            send_to_char("You are not allowed to do that.\r\n", ch);
            return FALSE;
    }
    if (IS_AFFECTED(ch, AFF_CHARM)) {
        do_say(ch, "No Master - i won't.");
        return FALSE;
    }
    return TRUE;
  }

  send_to_char("You are not allowed to do that.\r\n", ch);
  return FALSE;
}


/* prints the argument to all the rooms aroud the mobile */

//
// MPASOUND text
//
//   Echos text to all surrounding rooms.
//

void do_mpasound( CHAR_DATA *ch, char *argument ) {

    ROOM_INDEX_DATA *was_in_room;
    int              door;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

    if ( argument[0] == '\0' )
    {
        bug( "Mpasound - No argument: vnum %d.", ch->pIndexData->vnum );
	return;
    }

    was_in_room = ch->in_room;
    for ( door = 0; door < DIR_MAX; door++ )
    {
      EXIT_DATA       *pexit;
      
      if ( ( pexit = was_in_room->exit[door] ) != NULL
	  &&   pexit->u1.to_room != NULL
	  &&   pexit->u1.to_room != was_in_room )
      {
	ch->in_room = pexit->u1.to_room;
	MOBtrigger  = FALSE;
	act( argument, ch, NULL, NULL, TO_ROOM );
      }
    }

  ch->in_room = was_in_room;
  return;

}

/* lets the mobile kill any player or mobile without murder*/

//
// MPKILL mobile
//
//   Initiates combat skipping some of the PK checks
//

void do_mpkill( CHAR_DATA *ch, char *argument ) {
    char      arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	bug( "MpKill - no argument: vnum %d.",
		ch->pIndexData->vnum );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	bug( "MpKill - Victim not in room: vnum %d.",
	    ch->pIndexData->vnum );
	return;
    }

    if ( victim == ch )
    {
	bug( "MpKill - Bad victim to attack: vnum %d.",
	    ch->pIndexData->vnum );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
    {
	bug( "MpKill - Charmed mob attacking master: vnum %d.",
	    ch->pIndexData->vnum );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {	
	bug( "MpKill - Already fighting: vnum %d",
	    ch->pIndexData->vnum );
	return;
    }

    multi_hit( ch, victim, TYPE_UNDEFINED );
    check_assist(ch,victim);
    return;
}


/* lets the mobile destroy an object in its inventory
   it can also destroy a worn object and it can destroy 
   items using all.xxxxx or just plain all of them */

//
// MPJUNK object
//
//   Destroys an object in the mobs inventory or that is being worn
//

void do_mpjunk( CHAR_DATA *ch, char *argument )
{
    char      arg[ MAX_INPUT_LENGTH ];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0')
    {
        bug( "Mpjunk - No argument: vnum %d.", ch->pIndexData->vnum );
	return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
      if ( ( obj = get_obj_wear( ch, arg ) ) != NULL )
      {
	unequip_char( ch, obj );
	extract_obj( obj );
	return;
      }
      if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	return; 
      extract_obj( obj );
    }
    else
      for ( obj = ch->carrying; obj != NULL; obj = obj_next )
      {
        obj_next = obj->next_content;
        if ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
        {
          if ( obj->wear_loc != WEAR_NONE)
	    unequip_char( ch, obj );
          extract_obj( obj );
        } 
      }

    return;

}

/* prints the message to everyone in the room other than the mob and victim */

// 
// MPECHOAROUND victim text
//
//   Echos text to everyone except victim and mob
//

void do_mpechoaround( CHAR_DATA *ch, char *argument )
{
  char       arg[ MAX_INPUT_LENGTH ];
  CHAR_DATA *victim;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
       bug( "Mpechoaround - No argument:  vnum %d.", ch->pIndexData->vnum );
       return;
    }

    if ( !( victim=get_char_room( ch, arg ) ) )
    {
        bug( "Mpechoaround - victim does not exist: vnum %d.",
	    ch->pIndexData->vnum );
	return;
    }

    act( argument, ch, NULL, victim, TO_NOTVICT );
    return;
}

/* prints the message to only the victim */

//
// MPECHOAT victim text
//
//   Echos text just to the victim
//

void do_mpechoat( CHAR_DATA *ch, char *argument )
{
  char       arg[ MAX_INPUT_LENGTH ];
  CHAR_DATA *victim;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
       bug( "Mpechoat - No argument:  vnum %d.",
	   ch->pIndexData->vnum );
       return;
    }

    if ( !( victim = get_char_room( ch, arg ) ) )
    {
        bug( "Mpechoat - victim does not exist: vnum %d.",
	    ch->pIndexData->vnum );
	return;
    }

    act( argument, ch, NULL, victim, TO_VICT );
    return;
}

/* prints the message to the room at large */

//
// MPECHO _scope text
//
//   Echos text to all in scope, default scope is _room
//
//   Other scopes are: _room, _room_plus, _subarea, _subarea_plus,
//                     _area, _area_plus, _zone, _universe, _group
//

void do_mpecho( CHAR_DATA *ch, char *argument ) {

    char buf[MAX_STRING_LENGTH];
    char message[MAX_STRING_LENGTH];
    
    char *arg2;

    MOB_CMD_CONTEXT *mcc;
    WEV *wev;    

    int scope;

   /* Mobs only... */
  
   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

   /* Check we have some parms... */ 

    if ( argument[0] == '\0' ) {
        bug( "Mpecho - called w/o argument: vnum %d.",
	    ch->pIndexData->vnum );
        return;
    }

   /* Determine the scope... */

    arg2 = one_argument(argument, buf);

    if (!str_cmp(buf, "_room")) {
      scope = WEV_SCOPE_ROOM;
    } else if (!str_cmp(buf, "_room_plus")) {
      scope = WEV_SCOPE_ADJACENT;
    } else if (!str_cmp(buf, "_subarea")) {
      scope = WEV_SCOPE_SUBAREA;
    } else if (!str_cmp(buf, "_subarea_plus")) {
      scope = WEV_SCOPE_SUBAREA_PLUS;
    } else if (!str_cmp(buf, "_area")) {
      scope = WEV_SCOPE_AREA;
    } else if (!str_cmp(buf, "_area_plus")) {
      scope = WEV_SCOPE_AREA_PLUS;
    } else if (!str_cmp(buf, "_zone")) {
      scope = WEV_SCOPE_ZONE;
    } else if (!str_cmp(buf, "_universe")) {
      scope = WEV_SCOPE_UNIVERSE;
    } else if (!str_cmp(buf, "_group")) {
      scope = WEV_SCOPE_GROUP;
    } else {
      scope = WEV_SCOPE_ROOM;
      arg2 = argument;
    }      

   /* Build wev... */

    sprintf(message, "%s\r\n", arg2);

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, scope, arg2);

    wev = get_wev(WEV_MOB, WEV_MOB_ECHO, mcc,
                  message,
                  NULL,
                  message);

   /* No challange, so just issue... */

    area_issue_wev(ch->in_room, wev, scope);

    free_wev(wev);

//    act( argument, ch, NULL, NULL, TO_ROOM );

    return;

}

/* lets the mobile load an item or mobile.  All items
are loaded into inventory.  you can specify a level with
the load object portion as well. */

//
// MPMLOAD vnum
//
//   Creates an instance of the mob specified and puts it in the mobiles room
//

void do_mpmload( CHAR_DATA *ch, char *argument )
{
    char            arg[ MAX_INPUT_LENGTH ];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA      *victim;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	bug( "Mpmload - Bad vnum as arg: vnum %d.", ch->pIndexData->vnum );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	bug( "Mpmload - Bad mob vnum: vnum %d.", ch->pIndexData->vnum );
	return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    return;
}

//
// MPOLOAD vnum level
//
//   Creates an instance of the object at the specified level and puts it
//   into the mobs inventory if it an be taken, or into the room if it cannot.
//

void do_mpoload( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA       *obj;
    int             level;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
 
    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
        bug( "Mpoload - Bad syntax: vnum %d.",
	    ch->pIndexData->vnum );
        return;
    }
 
    if ( arg2[0] == '\0' )
    {
	level = ch->level;
    }
    else
    {
	/*
	 * New feature from Alander.
	 */
        if ( !is_number( arg2 ) )
        {
	    bug( "Mpoload - Bad syntax: vnum %d.", ch->pIndexData->vnum );
	    return;
        }
	level = atoi( arg2 );
	if ( level < 0 || level > (ch->level + 10))
	{
	    bug( "Mpoload - Bad level: vnum %d.", ch->pIndexData->vnum );
            bug( "          Level is: %d.", level ); 
            bug( "          Trust is: %d.", ch->level + 10 ); 
	    level = ch->level;
	}
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	bug( "Mpoload - Bad vnum arg: vnum %d.", ch->pIndexData->vnum );
	return;
    }

    obj = create_object( pObjIndex, level );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
    {
	obj_to_char( obj, ch );
    }
    else
    {
	obj_to_room( obj, ch->in_room );
    }

    return;
}

/* lets the mobile purge all objects and other npcs in the room,
   or purge a specified object or mob in the room.  It can purge
   itself, but this had best be the last command in the MOBprogram
   otherwise ugly stuff will happen */

//
// MPPURGE
//
//   Destroy all purgable objects and mobiles in the same room
//
// MPPURGE object
//
//   Destroy an instance of the object that is in the room
//
// MPPURGE mobile
//
//   Destroy an instance of the mobile that is in the room
// 

void do_mppurge( CHAR_DATA *ch, char *argument ) {
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        /* 'purge' */
        CHAR_DATA *vnext;
        OBJ_DATA  *obj_next;

	for ( victim = ch->in_room->people; victim != NULL; victim = vnext ) {
	  vnext = victim->next_in_room;
	  if ( IS_NPC( victim ) 
                  && victim != ch 
                  && victim->master == NULL )
	    extract_char( victim, TRUE );
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next ) {
	  obj_next = obj->next_content;
	  extract_obj( obj );
	}

	return;
    }

    victim = get_char_room(ch, arg );
    if ( victim == NULL ) {
	obj = get_obj_here(ch, arg );
	if (obj != NULL ) {
	    extract_obj(obj);
	} else {
	    bug( "Mppurge - Bad argument: vnum %d.", ch->pIndexData->vnum );
	}
	return;
    }

    if ( !IS_NPC( victim ) )    {
	bug( "Mppurge - Purging a PC: vnum %d.", ch->pIndexData->vnum );
	return;
    }

    extract_char(victim, TRUE );
    return;
}


/* lets the mobile goto any location it wishes that is not private */

//
// MPGOTO vnum
//
//   Silently teleport the mob to room vnum.
//

void do_mpgoto( CHAR_DATA *ch, char *argument )
{
    char             arg[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *location;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	bug( "Mpgoto - No argument: vnum %d.", ch->pIndexData->vnum );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	bug( "Mpgoto - No such location: vnum %d.", ch->pIndexData->vnum );
	return;
    }

    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );

    char_from_room( ch );
    char_to_room( ch, location );

    return;
}

/* lets the mobile do a command at another location. Very useful */

//
// MPAT vnum command
//
//   Silently teleport the mob to room vnum, issue the command and then
//   silently teleport it back to where it started.
//
// MPAT 'Mmobile' command
//
//   Silently teleport the mob to room where mobile is, issue the command
//   and then silently teleport it back to where it started. Aiming can be
//   eratic if not used with a true name or a unique mobile.
//
// MPAT 'Oobject' command
//
//   Silently teleport the mob to room where object is, issue the command
//   and then silently teleport it back to where it started.  Unless the 
//   object is guarenteed unique, aiming may be somewhat eratic.
//

void do_mpat( CHAR_DATA *ch, char *argument )
{
    char             arg[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA       *wch;
    OBJ_DATA        *wobj;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "Mpat - Bad argument: vnum %d.", ch->pIndexData->vnum );
	return;
    }

    location = NULL;

    if (arg[0] == 'M') {
      wch = get_char_world(ch, arg+1);

      if (wch != NULL) {
        location = wch->in_room;
      }
    } else if (arg[0] == 'O') {
      wobj = get_obj_world(ch, arg+1);

      if (wobj != NULL) {

        while (wobj->in_obj != NULL) {
          wobj = wobj->in_obj;
        }

        if (wobj->in_room != NULL) {
          location = wobj->in_room;
        } else if (wobj->carried_by != NULL) {
          location = wobj->carried_by->in_room;
        } 
      }

    } else { 
      location = find_location( ch, arg );
    }

    if ( location == NULL ) {
	bug( "MPAT - No such location: vnum %d.", ch->pIndexData->vnum );
	return;
    }

    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    break;
	}
    }

    return;
}
 
//
// Subroutine to transfer a single victim...
//

void transfer_victim(	CHAR_DATA *ch,
			CHAR_DATA *victim,
			char *vname, 
 			ROOM_INDEX_DATA *location,
			bool world) {

   /* Must have somewhere to send them... */

    if ( location == NULL ) {
      return;
    }

   /* Work out who we're sending... */
 
    if ( victim == NULL ) {

      if ( vname == NULL ) {
        return;
      }

      if ( world ) {
        victim = get_char_world( ch, vname );
      } else {
        victim = get_char_room( ch, vname );
      }

      if ( victim == NULL ) {
  	return;
      }
    }

   /* Are they allowed to go there? */ 

    if ( !can_see_room(victim, location ) ) {
      return;
    }

   /* Stop them fighting... */ 

    if ( victim->fighting != NULL ) {
	stop_fighting( victim, TRUE );
    }

   /* Extract... */

    if ( victim->in_room != NULL ) {
      char_from_room( victim );
    }
 
   /* ...and deliver... */

    char_to_room( victim, location );

   /* All done. */

    return;
}

//
// MPTRANSFER all
//
//   Silently teleport all players on the mud that the mob can see
//   to its room
//
// MPTRANSFER all vnum
//
//   Silently teleport all players on the mud that the mob can see
//   to room vnum.
//
// MPTRANSFER all_room vnum
//
//   Silently teleport all players in the mobs room that the mob
//   can see to room vnum.
//
// MPTRANSFER victim
//
//   Silently teleport the victim to the mobs room
//
// MPTRANSFER victim vnum
//
//   Silently teleport the victim to room vnum
//

void do_mptransfer( CHAR_DATA *ch, char *argument ) {

    char             arg1[ MAX_INPUT_LENGTH ];
    char             arg2[ MAX_INPUT_LENGTH ];

    ROOM_INDEX_DATA *location;

    DESCRIPTOR_DATA *d;

    CHAR_DATA *victim, *next_victim;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' ) {
	bug( "Mptransfer - Bad syntax: vnum %d.", ch->pIndexData->vnum );
	return;
    }

   /* Work out the transfer location... */
   /* Thanks to Grodyn for the optional location parameter. */

    if ( arg2[0] == '\0' ) {
	location = ch->in_room;
    } else {
	location = find_location( ch, arg2 );

	if ( location == NULL ) {
	    bug( "Mptransfer - No such location: vnum %d.",
	        ch->pIndexData->vnum );
	    return;
	}

	if ( !can_see_room(ch, location ) ) {
	    return;
	}
    }

   /* Transfer all players on the mud... */

    if ( !str_cmp( arg1, "all" ) ) {

	for ( d = descriptor_list; d != NULL; d = d->next ) {
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room != NULL
	    &&   can_see( ch, d->character ) ) {
		transfer_victim( ch, d->character, NULL, location, TRUE );
	    }
	}

	return;
    }

   /* Transfer all players in the mobs room... */

    if ( !str_cmp( arg1, "all_room" ) ) {

        if ( ch->in_room == NULL
          || location == ch->in_room ) {
          return;
        }
      
        victim = ch->in_room->people;

        while ( victim != NULL ) {

          next_victim = victim->next_in_room;
  
	  if ( victim != ch
	    && can_see( ch, victim ) ) {
            transfer_victim( ch, victim, NULL, location, FALSE );
	  }

          victim = next_victim;

	}

	return;
    }

   /* Transfer a single player... */

    if ( location == ch->in_room ) {
      transfer_victim( ch, NULL, arg1, location, TRUE );
    } else {
      transfer_victim( ch, NULL, arg1, location, FALSE );
    }

    return;
}

/* lets the mobile force someone to do something.  must be mortal level
   and the all argument only affects those in the room with the mobile */

//
// MPFORCE all command
//
//   Force all players in the mobs room to perform command
//
// MPFORCE victim command
//
//   Force victim (in the mobs room) to perform command
//

void do_mpforce( CHAR_DATA *ch, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "Mpforce - Bad syntax: vnum %d.", ch->pIndexData->vnum );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( vch->in_room == ch->in_room
		&& get_trust( vch ) < get_trust( ch ) 
		&& can_see( ch, vch ) )
	    {
		interpret( vch, argument );
	    }
	}
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    bug( "Mpforce - No such victim: vnum %d.",
	  	ch->pIndexData->vnum );
	    return;
	}

	if ( victim == ch )
    	{
	    bug( "Mpforce - Forcing oneself: vnum %d.",
	    	ch->pIndexData->vnum );
	    return;
	}

	interpret( victim, argument );
    }

    return;
}

//
// MPSCRIPT RUN script_id
//
//   Schedule script x on this mob
//
// MPSCRIPT RUN script_id mobile
//
//   Schedule the mobile to run its script x
//
// MPSCRIPT CANCEL command_id
//
//   Purge all commands with id x enqueued on this mob
//
// MPSCRIPT CANCEL command_id mobile
//
//   Purge all commands eith id x enqueued on the target mobile
//

void do_mpscript(CHAR_DATA *ch, char *args) {

    char act[MAX_INPUT_LENGTH];
    char scr_id[MAX_INPUT_LENGTH];
    char tgt_mob[MAX_INPUT_LENGTH]; 

    int id;

    CHAR_DATA *tgt;

    MOB_SCRIPT *scr;
    MOB_CMD_CONTEXT *mcc;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

   /* Grab the parms... */ 
  
    args = one_argument(args, act);
    args = one_argument(args, scr_id);
    args = one_argument(args, tgt_mob);

   /* Evaluate the id... */

    if (scr_id[0] == '\0') {
      return;
    }

    id = atoi(scr_id);

    if (id <= 0) {
      return;
    }

   /* Find the target mob (or bail out if it's missing)... */

    if (tgt_mob[0] == '\0') {
      tgt = ch;
    } else {
      tgt = get_char_room(ch, tgt_mob);

      if (tgt == NULL) {
        return;
      }
    }
 
   /* Schedule action... */

    if (!str_cmp(act, "run")) {

      scr = find_script(tgt, id);

      if (scr == NULL) {
        bug("Bad script id (%d) on MPSCRIPT", id);
        return;
      }
   
      mcc = get_mcc(tgt, ch, tgt, random_char(ch), NULL, NULL, 0, NULL);

      enqueue_script(tgt, scr, mcc);

      free_mcc(mcc);

      return;
    }

   /* Cancel command... */

    if (!str_cmp(act, "cancel")) {

      purge_mcbs(tgt, id);

      return;
    }

   /* Oops... */

    bug("Bad MPSCRIPT in mob script.", 0);

    return;
}

//
// MPREWARD mobile xps
//
//   Give the player the indicated number of xps as a group reward
//

void do_mpreward(CHAR_DATA *ch, char *args) {
    char pc_name[MAX_INPUT_LENGTH];
    char ammount[MAX_INPUT_LENGTH];
    char type[MAX_INPUT_LENGTH];
    int amt;
    CHAR_DATA *pc;
    CHAR_DATA *tpc = NULL;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) return;
    
   /* Grab the parms... */ 
  
    args = one_argument(args, pc_name);
    args = one_argument(args, type);

   /* Find the pc... */

    pc = get_char_room(ch, pc_name);
    if ( pc == NULL ) return;
    if (pc->desc != NULL) {
        if (pc->desc->original != NULL) tpc = pc->desc->original;
    }  
    if (tpc == NULL) tpc = pc;

   /* Work out the ammount... */

    if (is_number(type)) {
          amt = atoi(type);

          if (amt < 10) {
               bug("Bad reward ammount %d.", amt);
               return;
          }   

          group_reward(tpc, amt);

    } else {
         args = one_argument(args, ammount);

         amt = atoi(ammount);
         if (amt < 1) {
               bug("Bad reward ammount %d.", amt);
               return;
         }   

          if (!str_cmp(type, "xp")) {
                 group_reward(tpc, amt);

          } else if (!str_cmp(type, "fame")) {
                 if (IS_NPC(tpc)) return;

                 tpc->pcdata->questpoints = UMAX(tpc->pcdata->questpoints + amt, 0);
                 send_to_char("You feel more famous.\r\n",tpc);

          } else if (!str_cmp(type, "prac")) {
                 tpc->practice = UMAX(tpc->practice + amt, 0);
                 send_to_char("You feel more studious.\r\n",tpc);

          } else if (!str_cmp(type, "train")) {
                 tpc->train = UMAX(tpc->train + amt, 0);
                 send_to_char("You feel more trained.\r\n",tpc);
          }
    }

    return;
}


//
// MPFAME mobile xps
//
//   Give the player the indicated number of xps as a group reward
//

void do_mpfame(CHAR_DATA *ch, char *args) {
    char pc_name[MAX_INPUT_LENGTH];
    char ammount[MAX_INPUT_LENGTH];
    int amt;
    CHAR_DATA *pc;
    CHAR_DATA *tpc = NULL;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) return;
    
   /* Grab the parms... */ 
  
    args = one_argument(args, pc_name);
    args = one_argument(args, ammount);

   /* Find the pc... */

    pc = get_char_room(ch, pc_name);
    if ( pc == NULL ) return;
    if (pc->desc != NULL) {
        if (pc->desc->original != NULL) tpc = pc->desc->original;
    }  
    if (tpc == NULL) tpc = pc;
    if (IS_NPC(tpc)) return;

   /* Work out the ammount... */

    amt = atoi(ammount);

    if (amt > 15 || amt < 0) {
      bug("Bad reward ammount %d.", amt);
      return;
    }   

   /* Give them the reward... */
    tpc->pcdata->questpoints += amt;
 
   /* All done... */

    return;
}


//
// MPSANITY mobile CHECK threshold action
//
//   Force the mobile to make a sanity chech against the given threshold and
//   to take thespecified action if they fail.
//
// MPSANITY mobile CHECK threshold
//
//   Force the mobile to make a sanity check against the given threshold and
//   take a random action if they fail.
//
// MPSANITY mobile RECOVER ammount
//
//   Increase the target mobiles sanity by the ammount given (max 20).
//

void do_mpsanity(CHAR_DATA *ch, char *args) {
char pc_name[MAX_INPUT_LENGTH];
char action[MAX_INPUT_LENGTH];
char ammount[MAX_INPUT_LENGTH];
int amt;
CHAR_DATA *pc;
MOB_CMD_CONTEXT *context;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) return;

   /* Grab the parms... */ 
  
    args = one_argument(args, pc_name);
    args = one_argument(args, action);
    args = one_argument(args, ammount);

   /* Find the pc... */

    pc = get_char_room(ch, pc_name);

    if ( pc == NULL ) return;

   /* Work out the ammount... */

    amt = atoi(ammount);

   /* Build the context... */

    if (!str_cmp(action, "check")) {

      if (amt < 30) {
        bug("Bad sanity threat %d - minimum is 30", amt);
        return;
      }   

      context = get_mcc(pc, ch, NULL, NULL, NULL, NULL, 0, NULL);

      context->room = pc->in_room;
 
     /* Give them the reward... */

      if (args[0] != '\0') {
        insanity(context, amt, args);
      } else {
        insanity(context, amt, NULL);
      }

     /* Lose the old context... */

      free_mcc(context);
    }
 
    if (!str_cmp(action, "recover")) {
    
      if (amt < 1) {
        bug("Bad sanity recovery %d - minimum is 1", amt);
        return;
      }   

      if (amt > 30) {
        bug("Bad sanity recovery %d - maximum is 30", amt);
        return;
      }   

      pc->sanity += number_range(amt/2, amt);
      send_to_char("You feel a little more in control.\r\n", pc);

    }

    return;
}

/* Update conversation status... */

//
// MPCONV mobile conv_id subj_id state
//
//   Update the state of the mobiles conversation conv_id about subject subj_id
//   to the specified state.
//

void do_mpconv(CHAR_DATA *ch, char *args) {

  char pc_name[MAX_INPUT_LENGTH];
  char conv_str[MAX_INPUT_LENGTH];
  char subj_str[MAX_INPUT_LENGTH];
  char stat_str[MAX_INPUT_LENGTH];

  int conv, subj, state;

  CHAR_DATA *pc;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

 /* Grab the parms... */ 
  
  args = one_argument(args, pc_name);
  args = one_argument(args, conv_str);
  args = one_argument(args, subj_str);
  args = one_argument(args, stat_str);

 /* Find the pc... */

  pc = get_char_room(ch, pc_name);

  if ( pc == NULL ) {
    return;
  }

 /* Translate strings to values... */

  conv = atoi(conv_str);

  if (conv < 1) {
    bug("Bad conversation (%d).", conv);
    return;
  }   

  subj = atoi(subj_str);

  if (subj < 1) {
    bug("Bad subject (%d).", subj);
    return;
  }   

  state = atoi(stat_str);

  if (state < 0) {
    bug("Bad state (%d).", state);
    return;
  }   

 /* Now set the new state... */

  set_csr(pc, conv, subj, state);
 
 /* All done... */

  return;
}

/* Deeds... */

//
// MPDEED mobile ADD deed_id type description
//
//   Records that the mobile has performed the deed
//
//	Type:    0  normal
//		+1  secret
//		+2  public
//		+4  good
//		+8  bad
//
// MPDEED mobile REMOVE deed_id
//
//   Makes the mobile forget that they have doone the deed
// 

void do_mpdeed(CHAR_DATA *ch, char *args) {
  char argx[MAX_INPUT_LENGTH];
  int id, type;
  bool add;
  CHAR_DATA *pc;
  CHAR_DATA *tpc = NULL;

   /* Authorized? */
    if ( !can_use_mp_commands(ch) ) return;
    
 /* Find the target player... */

  args = one_argument(args, argx);
  pc = get_char_room(ch, argx);

  if ( pc == NULL ) return;
  if (pc->desc != NULL) {
      if (pc->desc->original != NULL) tpc = pc->desc->original;
  }  
  if (tpc == NULL) tpc = pc;

  if (IS_NPC(tpc)) return;

 /* Work out the action... */
  args = one_argument(args, argx);

  if (!str_cmp(argx, "add")) {
    add = TRUE;
  } else if (!str_cmp(argx, "remove")) {
    add = FALSE;
  } else {
    bug("Bad action on MPDEED", 0);
    return;
  }
     
 /* Get the deed id... */

  args = one_argument(args, argx);

  id = atoi(argx);

 /* Process an remove... */

  if (!add) {
     if (id < 1) {
        bug("Invalid Id!", 0);
        return;
     }
     remove_deed(tpc, id);
     return;
  }

 /* Extract the deed type... */

  args = one_argument(args, argx);

  if (args[0] == '\0') {
    bug("No title on MPDEED ADD!", 0);
    return;
  }

  type = atoi(argx);

  if (id == -1) {
        long value;

        value = flag_value(discipline_type, args);
        if ( value != NO_FLAG ) {
             if(!knows_discipline(tpc, flag_value(discipline_type, args))) write_to_grimoire(tpc, args);
        }
        return;
  }

  if (type < 0 || type >= DEED_MAX) {
    bug("Bad type (%d) on MPDEED!", type);
    return;
  }

  add_deed(tpc, id, type, args);

 /* All done... */

  return;
}


/* Quests... */

//
// MPQUEST mobile START quest_id description
//
//   Starts the mobile one the quest, initially in state 0
//
// MPQUEST mobile UPDATE quest_id state description
//
//   Updates the state and 'next task' description of the mobile for the
//   specified quest.
//
// MPQUEST mobile COMPLETE quest_id description
//
//   Marks the mobile as having completed the quest.
//
// MPQUEST mobile FORGET quest_id
//
//   Makes the mobile forget all about the quest.
//

#define MPDQ_START	1
#define MPDQ_UPDATE	2
#define MPDQ_COMPLETE	3
#define MPDQ_FORGET	4

void do_mpquest(CHAR_DATA *ch, char *args) {

  char argx[MAX_INPUT_LENGTH];

  int id, state;

  int action;

  CHAR_DATA *pc;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

 /* Find the target player... */

  args = one_argument(args, argx);

  pc = get_char_room(ch, argx);

  if ( pc == NULL ) {
    return;
  }

 /* Work out the action... */

  args = one_argument(args, argx);

  if (!str_cmp(argx, "start")) {
    action = MPDQ_START;
  } else if (!str_cmp(argx, "update")) {
    action = MPDQ_UPDATE;
  } else if (!str_cmp(argx, "complete")) {
    action = MPDQ_COMPLETE;
  } else if (!str_cmp(argx, "forget")) {
    action = MPDQ_FORGET;
  } else {
    bug("Bad action on MPQUEST", 0);
    return;
  }
     
 /* Get the quest id... */

  args = one_argument(args, argx);

  id = atoi(argx);

  if (id <= 0) {
    bug("Invalid id (%d) on MPQUEST!", id);
    return;
  }

 /* What next... */

  state = 0;

  switch (action) {
 
    case MPDQ_START:

      if (args[0] == '\0') {
        bug("No title on MPQUEST START!", 0);
        return;
      }

      break;

    case MPDQ_UPDATE:
      args = one_argument(args, argx);

      state = atoi(argx);

      if ( state < QUEST_STARTED   
        || state >= QUEST_COMPLETED) {
        bug("Bad state (%d) on MPQUEST!", state);
        return;
      }

      if (args[0] == '\0') {
        bug("No description on MPQUEST UPDATE!", 0);
        return;
      }

      break;

    case MPDQ_COMPLETE:

      if (args[0] == '\0') {
        bug("No description on MPQUEST COMPLETE!", 0);
        return;
      }

      break;

    case MPDQ_FORGET:
      break;

    default:
      break; 
  }

 /* Apply to all in the characters group... */

  ch = char_list;

  while (ch != NULL) {

    if (is_same_group(pc, ch)) {

      switch (action) {
 
        case MPDQ_START:
          add_quest(ch, id, args);
          break;

        case MPDQ_UPDATE:
          update_quest(ch, id, state, args);
          break;

        case MPDQ_COMPLETE:
          update_quest(ch, id, QUEST_COMPLETED, args);
          break;

        case MPDQ_FORGET:
          remove_quest(ch, id);
          break;

        default:
          break; 
      }
    }

    ch = ch->next;
  }

 /* All done... */

  return;
}

/* Stop... */

//
// MPSTOP mobile action
//
//   Echos a set of 'you stop mobile from action' messages
//

void do_mpstop(CHAR_DATA *ch, char *args) {

    char argx[MAX_INPUT_LENGTH];

    CHAR_DATA *pc;

    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

   /* Find the target player... */

    args = one_argument(args, argx);

    pc = get_char_room(ch, argx);

    if ( pc == NULL ) {
      return;
    }

   /* Build mcc and wev... */

    mcc = get_mcc(ch, ch, pc, NULL, NULL, NULL, 0, args);

    wev = get_wev(WEV_MOB, WEV_MOB_STOP, mcc,
             "You stop @v2 from @t0.\r\n",
             "@a2 stops you from @t0!\r\n",
             "@a2 stops @v2 from @t0.\r\n");

   /* Send it out... */

    room_issue_wev(ch->in_room, wev);

    free_wev(wev);

   /* All done... */

    return;
}


/* Hurt... */

//
// MPHURT mobile hd hs mh ms vd vd
//
//   Causes damage to the mobile, of the soeciufied number and type of dice
//   for hits, mana and move.
//

void do_mphurt(CHAR_DATA *ch, char *args) {

  char argx[MAX_INPUT_LENGTH];

  int hpd, hps, mpd, mps, vpd, vps;
  int hdam, mdam, vdam;

  CHAR_DATA *mob;

  MOB_CMD_CONTEXT *mcc;
  WEV *wev;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) return;
    
 /* Find the target player... */

  args = one_argument(args, argx);

  mob = get_char_room(ch, argx);

  if ( mob == NULL ) return;
  
 /* Read the damage values... */

  args = one_argument(args, argx);

  hpd = atoi(argx);

  args = one_argument(args, argx);

  hps = atoi(argx);

  args = one_argument(args, argx);

  mpd = atoi(argx);

  args = one_argument(args, argx);

  mps = atoi(argx);

  args = one_argument(args, argx);

  vpd = atoi(argx);

  args = one_argument(args, argx);

  vps = atoi(argx);

 /* Calculate damage... */

  hdam = dice(hpd, hps);

  mdam = dice(mpd, mps);

  vdam = dice(vpd, vps);

 /* Apply the damage... */

  mob->hit -= hdam;

  if ( (!IS_NPC(mob))
  && (IS_IMMORTAL(mob))
  && !IS_AFFECTED(mob, AFF_INCARNATED)
  && (mob->hit < 1) ) {
    mob->hit = 1;
  }

  mob->mana -= mdam;

  mob->mana = UMAX(mob->mana, 0);

  mob->move -= vdam;

  mob->move = UMAX(mob->move, 0);

 /* No physical damage, means we're done... */

  if (hdam == 0
  && vdam == 0) return;
  
 /* Work out new position... */ 

  update_pos( mob );

  switch( mob->position ) {
    
    case POS_MORTAL:
      act( "$n is mortally wounded, and will die soon, if not aided.",
        mob, NULL, NULL, TO_ROOM );
      send_to_char("You are mortally wounded, and will die soon, if not aided.\r\n",
        mob );
      break;

    case POS_INCAP:
      act( "$n is incapacitated and will slowly die, if not aided.",
        mob, NULL, NULL, TO_ROOM );
      send_to_char(
        "You are incapacitated and will slowly die, if not aided.\r\n",
        mob );
      break;

    case POS_STUNNED:
      act( "$n is stunned, but will probably recover.",
	    mob, NULL, NULL, TO_ROOM );
      send_to_char("You are stunned, but will probably recover.\r\n",
	    mob );
      break;

    case POS_DEAD:
      act( "{Y$n is DEAD!!{x", mob, 0, 0, TO_ROOM );
      send_to_char( "{RYou have been KILLED!!{x\r\n\r\n", mob );
      break;

    default:

      if ( hdam > mob->max_hit / 4 ) {

        mcc = get_mcc( mob, mob, NULL, NULL, NULL, NULL, hdam, NULL);

        wev = get_wev( WEV_DAMAGE, WEV_DAMAGE_HURT, mcc,
                    "{RThat hurt you badly!{x\r\n", 
                     NULL,
                    "{r@a2 looks badly hurt!{x\r\n");

        room_issue_wev(mob->in_room, wev);

        free_wev(wev);
      }

      if ( mob->hit < mob->max_hit / 4 ) {

        mcc = get_mcc( mob, mob, NULL, NULL, NULL, NULL, mob->hit, NULL);

        wev = get_wev( WEV_DAMAGE, WEV_DAMAGE_INJURED, mcc,
                    "{RYou are seriously injured!{x\r\n", 
                     NULL,
                    "{r@a2 looks seriously injured!{x\r\n");

        room_issue_wev(mob->in_room, wev);

        free_wev(wev);
      }

      break;
  }

 /* Oh-oh, it's dead... */

  if ( mob->position == POS_DEAD ) {
    
    if ( IS_NPC(mob) ) {
      sprintf( log_buf, "%s killed by MPHURT at %ld",  mob->short_descr, mob->in_room->vnum);
      notify_message (NULL, WIZNET_MOBDEATH, TO_IMM, log_buf	);
    }

    if ( !IS_NPC(mob) ) {
      sprintf( log_buf, "%s killed by MPHURT at %ld",
                         mob->name,
                         mob->in_room->vnum );
      log_string( log_buf );

      notify_message (NULL, WIZNET_DEATH, TO_IMM, log_buf);

     /* PC's lose xps if they die... */
	   
      lose_exp_for_dieing(mob); 

    }
 
   /* Lose the body, completely... */

    raw_kill( mob, TRUE );
  }

 /* All done... */

  return;
}


void do_mpwound(CHAR_DATA *ch, char *args) {
char argx[MAX_INPUT_LENGTH];
CHAR_DATA *mob;
int limb;

    if ( !can_use_mp_commands(ch) ) return;
  
  args = one_argument(args, argx);
  mob = get_char_room(ch, argx);
  if (!mob) return;
  
  for (limb = 0; limb < 5; limb++) {
      args = one_argument(args, argx);
      if (argx[0] != '\0') {
          ch->limb[limb] += atoi(argx);
          ch->limb[limb] = URANGE(0, ch->limb[limb], 9);
      }
  }
  update_wounds(mob);
  return;
}


/* Selection... */

#define MAX_SEL_CAND 10

CHAR_DATA *select_mob(CHAR_DATA *ch, char *selector, CHAR_DATA *avoid) {

  char sel[MAX_STRING_LENGTH];
  char mod[MAX_STRING_LENGTH];

  char *pdot;

  bool found;

  CHAR_DATA *available[MAX_SEL_CAND]; 

  CHAR_DATA *mob;

  int num_mobs;

#define SEL_RANDOM	0
#define SEL_HIGHEST	1
#define SEL_LOWEST	2
 
  int selection = SEL_RANDOM;

  int index;

  num_mobs = 0;

 /* Check for silly calls... */

  if (ch == NULL) {
    return NULL;
  }

  if (ch->in_room == NULL) {
    return NULL;
  }

 /* Check for selection modifiers... */

  found = FALSE;

  pdot = selector;
  index = 0;

  while ( *pdot != '\0') {
    
    if ( *pdot == '.' 
      && !found ) {
      found = TRUE;
      mod[index] = '\0';
      index = 0;
    } else {
      if (found) {
        sel[index++] = *pdot;
      } else { 
        mod[index++] = *pdot;
      }
    }

    pdot++;
  }  
 
  if (found) {
    sel[index] = '\0';
  } else {
    mod[index] = '\0';
    strcpy(sel, mod);
    mod[0] = '\0';
  } 
   
  if (!str_cmp(mod, "highest")) {
    selection = SEL_HIGHEST;
  } else if (!str_cmp(mod, "lowest")) {
    selection = SEL_LOWEST;
  } else {
    selection = SEL_RANDOM;
  }  

 /* Work out what is available... */

  if (!str_cmp(sel, "mob_none")) {
    return NULL;
  }
 
  if (!str_cmp(sel, "mob_me")) {
    return ch;
  }
 
 /* Hmmm. Maybe it's a name... */

  mob = get_char_room(ch, sel);

  if ( mob != NULL
    && mob != ch
    && mob != avoid) {
    return mob;
  }

 /* Any mob will do... */

  if (!str_cmp(sel, "mob_any")) {
    
    mob = ch->in_room->people;

    while ( mob != NULL
         && num_mobs < MAX_SEL_CAND ) {

      if ( mob != ch
        && mob != avoid
        && can_see(ch, mob)) {
        
        available[num_mobs++] = mob;
      }

      mob = mob->next_in_room;
    } 
  }

 /* NPCs only... */

  if (!str_cmp(sel, "mob_npc")) {
    
    mob = ch->in_room->people;

    while ( mob != NULL
         && num_mobs < MAX_SEL_CAND ) {

      if ( mob != ch
        && mob != avoid
        && IS_NPC(mob)
        && can_see(ch, mob)) {
        
        available[num_mobs++] = mob;
      }

      mob = mob->next_in_room;
    } 
  }

 /* PCs only... */

  if ( !str_cmp(sel, "mob_pc") ) {
    
    mob = ch->in_room->people;

    while ( mob != NULL
         && num_mobs < MAX_SEL_CAND ) {

      if ( mob != ch
        && mob != avoid
        && !IS_NPC(mob) 
        && can_see(ch, mob)) {
        
        available[num_mobs++] = mob;
      }

      mob = mob->next_in_room;
    } 
  }

 /* Mortal PCs only... */

  if ( !str_cmp(sel, "mob_pc_mortal") ) {
    
    mob = ch->in_room->people;

    while ( mob != NULL
         && num_mobs < MAX_SEL_CAND ) {

      if ( mob != ch
        && mob != avoid
        && !IS_NPC(mob) 
        && (!IS_IMMORTAL(mob) 
          || IS_AFFECTED(mob, AFF_INCARNATED))
        && can_see(ch, mob)) {
        
        available[num_mobs++] = mob;
      }

      mob = mob->next_in_room;
    } 
  }

 /* Ok, time to pick one... */

  if (num_mobs == 0) {
    return NULL;
  }

  if (num_mobs == 1) {
    return available[0];
  }

  switch (selection) {
    default:
    case SEL_RANDOM:
      mob = available[number_range(0, num_mobs - 1)];
      break;

    case SEL_HIGHEST:

      mob = available[0];

      index = 1;
      while (index < num_mobs) { 

        if ( available[index]->level > mob->level ) {
          mob = available[index];
        }

        index++;
      }

      break;

    case SEL_LOWEST:

      mob = available[0];

      index = 1;
      while (index < num_mobs) { 

        if ( available[index]->level < mob->level ) {
          mob = available[index];
        }

        index++;
      }

      break;

  }

  return mob;
}


OBJ_DATA *select_object(char *selector, MOB_CMD_CONTEXT *mcc) {

  OBJ_DATA *available[MAX_SEL_CAND]; 

  OBJ_DATA *obj;

  int num_objs;

  num_objs = 0;

 /* Check for silly calls... */

  if (mcc == NULL) {
    return NULL;
  }

  if (mcc->mob == NULL) {
    return NULL;
  } 

  if (mcc->mob->in_room == NULL) {
    return NULL;
  }

 /* Work out what is available... */

 /* Nothing desired... */

  if (!str_cmp(selector, "obj_none")) {
    return NULL;
  }
    
 /* Any object will do... */

  if (!str_cmp(selector, "obj_any")) {
    
    obj = mcc->mob->in_room->contents;

    while ( obj != NULL
         && num_objs < MAX_SEL_CAND ) {

      if ( obj != mcc->obj
        && can_see_obj(mcc->mob, obj)) {
        
        available[num_objs++] = obj;
      }

      obj = obj->next_content;
    } 
  }

 /* Any object from actor will do... */

  if (!str_cmp(selector, "obj_any_actor")) {
    
    if (mcc->actor == NULL) {
      return NULL;
    }

    obj = mcc->actor->carrying;

    while ( obj != NULL
         && num_objs < MAX_SEL_CAND ) {

      if ( obj != mcc->obj
        && can_see_obj(mcc->mob, obj)) {
        
        available[num_objs++] = obj;
      }

      obj = obj->next_content;
    } 
  }

 /* Any object from victim will do... */

  if (!str_cmp(selector, "obj_any_victim")) {
    
    if (mcc->victim == NULL) {
      return NULL;
    }

    obj = mcc->victim->carrying;

    while ( obj != NULL
         && num_objs < MAX_SEL_CAND ) {

      if ( obj != mcc->obj
        && can_see_obj(mcc->mob, obj)) {
        
        available[num_objs++] = obj;
      }

      obj = obj->next_content;
    } 
  }

 /* Any object from the mob will do... */

  if (!str_cmp(selector, "obj_any_mine")) {
    
    obj = mcc->mob->carrying;

    while ( obj != NULL
         && num_objs < MAX_SEL_CAND ) {

      if ( obj != mcc->obj
        && can_see_obj(mcc->mob, obj)) {
        
        available[num_objs++] = obj;
      }

      obj = obj->next_content;
    } 
  }

 /* Ok, time to pick one... */

  if (num_objs == 0) {
    return NULL;
  }

  if (num_objs == 1) {
    return available[0];
  }

  return available[number_range(0, num_objs - 1)];
}

//
// MPSELECT actor victim pri_object sec_object number text
//
//   Selects one or more mobs or objects from the mobs room and issues
//   a selection WEV. 
//

void do_mpselect(CHAR_DATA *ch, char *args) {

    char argx[MAX_INPUT_LENGTH];

    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

   /* Get the context... */ 

    mcc = get_mcc(ch, NULL, NULL, NULL, NULL, NULL, 0, NULL);

   /* First parm is mob spec for actor... */

    args = one_argument(args, argx);

    mcc->actor = select_mob(ch, argx, NULL);

   /* Second parm is mob spec for victim... */

    args = one_argument(args, argx);

    mcc->victim = select_mob(ch, argx, mcc->actor);

   /* Third parm is primary object spec... */

    args = one_argument(args, argx);

    mcc->obj = select_object(argx, mcc);

   /* Fourth parm is secondary object spec... */

    args = one_argument(args, argx);

    mcc->obj2 = select_object(argx, mcc);

   /* Fifth parm is number... */

    args = one_argument(args, argx);

    mcc->number = atoi(argx);

   /* And what ever is left is text... */

    mcc->text = args;

   /* Build and issue the WEV for processing... */

    wev = get_wev(WEV_MOB, WEV_MOB_SELECT, mcc, NULL, NULL, NULL);

    mob_handle_wev(ch, wev, NULL);

   /* All done... */

    return;
}

//
// MPALIGN mobile target
//
//   Changes the mobiles alignment towards the target.
//

void do_mpalign(CHAR_DATA *ch, char *args) {

  char pc_name[MAX_INPUT_LENGTH];
  char ammount[MAX_INPUT_LENGTH];

  int amt, delta;

  CHAR_DATA *pc;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

   /* Grab the parms... */ 
  
    args = one_argument(args, pc_name);
    args = one_argument(args, ammount);

   /* Find the pc... */

    pc = get_char_room(ch, pc_name);

    if ( pc == NULL) {
      return;
    }

   /* Work out the ammount... */

    amt = atoi(ammount);

    if ( amt < -10000 
      || amt > 10000) {
      bug("Bad alignment ammount %d.", amt);
      return;
    }   

   /* Now figure the change ( 1 tenth of the difference greater than 10)... */

    delta = amt - pc->alignment;

    delta /= 10;

    if ( delta == 0 ) {
      delta = (amt - pc->alignment);
    }

   /* And change the PCs alignment... */

    pc->alignment += delta;

    pc->alignment = URANGE(-1000, pc->alignment, 1000);
 
   /* All done... */

    return;
}

//
// MPEFFECT mobile 'effect'
//
//   Applies the named effect to the mobile.
//
// MPEFFECT mobile 'effect' level
//
//   Applies the named effect and the specified level to the mobile.
//

void do_mpeffect(CHAR_DATA *ch, char *args) {
    char pc_name[MAX_INPUT_LENGTH];
    char effect[MAX_INPUT_LENGTH];
    char ammount[MAX_INPUT_LENGTH];

    int level, efn;

    CHAR_DATA *pc;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

   /* Grab the parms... */ 
  
    args = one_argument(args, pc_name);
    args = one_argument(args, effect);
    args = one_argument(args, ammount);

   /* Find the pc... */

    pc = get_char_room(ch, pc_name);

    if ( pc == NULL) {
      return;
    }

   /* Find the effect... */

    efn = get_effect_efn(effect);

    if (efn == EFFECT_UNDEFINED) {
      log_string(effect);
      bug("Bad effect number %d.", efn);
      return;
    }

   /* Work out the level... */

    if (ammount[0] != '\0'
    && is_number(ammount)) {
      level = atoi(ammount);

      if ( level < 0
      || level > MAX_LEVEL) {
          bug("Bad effect level %d.", level);
          return;
      } 
    } else {
      level = ch->level;
      args = strdup(ammount);
    } 

   /* Applying time... */

    obj_cast_spell(efn, level, ch, pc, NULL, args);

   /* All done... */

    return;
}

/* Change a mobs level... */

//
// MPRELEV base delta
//
//   Changes the mobs level (and stats) to be the sum of base and delta.
//   Normally used where base is the substituted level of another mob.
//

void do_mprelev(CHAR_DATA *ch, char *args) {

    char argx[MAX_INPUT_LENGTH];

    int base;
    int delta;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

   /* Get parms... */
 
    args = one_argument(args, argx);

    base = atoi(argx);
    delta = atoi(args);

   /* Validate... */

    base += delta;

    if ( base < 1 ) {
      base = 1;
    }

    if (base > MAX_LEVEL ) {
      base = MAX_LEVEL;
    }

   /* Ok, go for it... */

    set_mobile_level(ch, NULL, base);

   /* All done... */

    return;  
}

/* Make a mob do something... */

//
// MPCHAT
//
//   Mob chatters at its current chatter state
//
// MPCHAT state
//
//   Mob changes to the new chatter state
//

void do_mpchat(CHAR_DATA *ch, char *args) {

    int chat_state;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
     }

   /* No parms, means we just chatter... */

    if (args[0] == '\0') {
      do_chat(ch, "");
      return;
    }

   /* Get parms... */
 
    chat_state = atoi(args);

   /* Ok, go for it... */

    if (chat_state >= 0) {
      ch->chat_state = chat_state;
    }

   /* All done... */

    return;  
}

//
// MPMEMORY mobile REMEMBER slot text
//
//   Target mob remembers the text string in the specified memory slot. 
//
// MPMEMORY mobile FORGET slot
//
//   Target mob forgets the text string in the specified memory slot. 
//

void do_mpmemory(CHAR_DATA *ch, char *args) {

    char pc_name[MAX_INPUT_LENGTH];
    char action[MAX_INPUT_LENGTH];
    char slot[MAX_INPUT_LENGTH];

    int slt;

    CHAR_DATA *pc;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

   /* Grab the parms... */ 
  
    args = one_argument(args, pc_name);
    args = one_argument(args, action);
    args = one_argument(args, slot);

   /* Find the pc... */

    pc = get_char_room(ch, pc_name);

    if ( pc == NULL ) {
      bug("MPMEMORY - pc not found", 0);
      return;
    }

   /* Work out the slot... */

    slt = atoi(slot);

    if ( slt < 0
      || slt >= MOB_MEMORY) {
      bug("Bad slot id %d.", slt);
      return;
    }   

   /* Now what do we do? */
  
    if (!str_cmp(action, "remember")) {

      if (args[0] == '\0') {
        bug("MPMEMORY - Nothing to remember!", 0);
        return;
      }

      if (pc->memory[slt] != NULL) {
        free_string(pc->memory[slt]);
        pc->memory[slt] = NULL;
      }

      pc->memory[slt] = str_dup(args);

      return; 
    } 
 
    if (!str_cmp(action, "forget")) {

      if (ch->memory[slt] != NULL) {
        free_string(ch->memory[slt]);
        ch->memory[slt] = NULL;
      } 

      return; 
    } 

    bug("MPMEMORY - Unrecognized action.", 0);
 
   /* All done... */

    return;
}

//
// MPHUNT mobile
//
//   Starts the mob hunting the mobile. 
//

void do_mphunt(CHAR_DATA *ch, char *args) {

    char pc_name[MAX_INPUT_LENGTH];

    CHAR_DATA *pc;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

   /* Grab the parms... */ 
  
    args = one_argument(args, pc_name);

   /* Find the pc... */

    pc = get_char_world(ch, pc_name);

    if ( pc == NULL 
      || pc == ch ) {

      if (ch->prey != NULL) {
        ch->prey->huntedBy = NULL;
      } 

      ch->prey = NULL;

      return;
    }

    if (pc->huntedBy == NULL) {
      pc->huntedBy = ch;
      ch->prey = pc;
    }

   /* All done... */

    return;
}

//
// MPSTEAL mobile object
//
//   Remove an object from a player and puts it in the mobiles inventory
//

void do_mpsteal(CHAR_DATA *ch, char *args) {
    char pc_name[MAX_INPUT_LENGTH];
    char obj_name[MAX_INPUT_LENGTH];
    char obj_gold[MAX_INPUT_LENGTH];
    CHAR_DATA *pc;
    OBJ_DATA *obj;
    int gloss;
    int currency = find_currency(ch);

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) {
	return;
    }

   /* Grab the parms... */ 
  
    args = one_argument(args, pc_name);
    args = one_argument(args, obj_name);

   /* Find the pc... */

    pc = get_char_room_unseen(ch, pc_name);

    if ( pc == NULL) return;
    
    if (!str_cmp(obj_name,"gold")) {
       args = one_argument(args, obj_gold);
       gloss = atoi(obj_gold);
       gloss = gloss * ((50 + number_percent())/100);
       pc->gold[currency] -= gloss;
       pc->gold[currency] = UMAX(pc->gold[currency], 0);
   } else {

    obj = get_obj_carry( pc, obj_name);

   /* Maybe they are wearing it... */

    if ( obj == NULL ) {
      obj = get_obj_wear( pc, obj_name);

      if ( obj != NULL ) {
        unequip_char( pc, obj );
      }
    }

   /* Transfer the object, if there is one... */

    if ( obj != NULL ) {

      obj_from_char(obj);

      obj_to_char(obj, ch);
    } 
   }
   /* All done... */

    return;
}

//
// MPPET mobile player
//
//   Give the player the indicated number of xps as a group reward
//

void do_mppet(CHAR_DATA *ch, char *args) {

    char player[MAX_INPUT_LENGTH];
    CHAR_DATA *pc;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) return;

   /* Grab the parms... */ 
  
    args = one_argument(args, player);

   /* Find the pc... */

    pc = get_char_room(ch, player);

    if ( pc == NULL ) return;
    if ( IS_NPC(pc)) return;

    if( pc->pet != NULL ) {
        send_to_char( "That player already has a pet.\r\n", ch );
        return;
    }

    SET_BIT(ch->affected_by, AFF_CHARM);
    SET_BIT(ch->act, ACT_FOLLOWER);
    SET_BIT(ch->act, ACT_PET);

    add_follower(ch, pc );
    ch->leader = pc;
    pc->pet = ch;

   /* For a little flavor... */

    do_emote(ch, "looks at you expectently" );

    return;
}


void do_mpinvasion(CHAR_DATA *ch, char *args) {

   /* Authorized? */
    if ( !can_use_mp_commands(ch) ) return;

    ch->in_room->area->invasion = TRUE;

    sprintf(log_buf, "%s begins revolution in %s",ch->name, ch->in_room->area->name); 
    log_string( log_buf );
    return;
}


void do_mposet(CHAR_DATA *ch, char *args) {
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value,set;

   /* Authorized? */

    if ( !can_use_mp_commands(ch) ) return;
    
    args = one_argument(args, arg);
    args = one_argument(args, arg2);
    args = one_argument(args, arg3);
    value = atoi(arg3);
    args = one_argument(args, arg4);
    set = atoi(arg4);

    obj  = get_obj_here(ch, arg);
    if ( obj == NULL ) return;

    if ( !str_cmp( arg2, "extra" ))    {
	if ( ( value = flag_value( extra_flags, arg3)) != NO_FLAG ){
                    if (set > 0) SET_BIT(obj->extra_flags, value);
                    else REMOVE_BIT(obj->extra_flags, value);
	}
    } else if ( !str_cmp( arg2, "wear" ))    {
	if ( ( value = flag_value( wear_flags, arg3)) != NO_FLAG ){
                    if (set > 0) SET_BIT(obj->wear_flags, value);
                    else REMOVE_BIT(obj->wear_flags, value);
	}
    } else if ( !str_cmp( arg2, "level" ) )    {
	obj->level = value;
    } else if ( !str_cmp( arg2, "weight" ) )    {
	obj->weight = value;
    } else if ( !str_cmp( arg2, "cost" ) )    {
	obj->cost = value;
    } else if ( !str_cmp( arg2, "timer" ) )    {
	obj->timer = value;
    }
    return;
}


void do_mpmset( CHAR_DATA *ch, char *args ) {
char arg1 [MAX_INPUT_LENGTH];
char arg2 [MAX_INPUT_LENGTH];
char arg3 [MAX_INPUT_LENGTH];
char arg4[MAX_INPUT_LENGTH];
CHAR_DATA *victim;
int value, set;

    if ( !can_use_mp_commands(ch) ) return;
    
    args = one_argument(args, arg1);
    args = one_argument(args, arg2);
    args = one_argument(args, arg3);
    value = atoi(arg3);
    args = one_argument(args, arg4);
    set = atoi(arg4);

    if ((victim  = get_char_room(ch, arg1)) == NULL) return;

    if ( !str_cmp( arg2, "sex" ))    {
	if ( value < 0 || value > 2 ) return;
	victim->sex = value;
	if (!IS_NPC(victim)) victim->pcdata->true_sex = value;

    } else if ( !str_cmp( arg2, "gold" )) {
	victim->gold[0] = value;

    } else if ( !str_cmp( arg2, "align" )) {
	if ( value < -1000 || value > 1000 ) return;
	victim->alignment = value;

    } else if ( !str_cmp( arg2, "timer" )) {
	if ( value < 0 || value > 30000 ) return;
    	victim->timer = value;

    } else if (!str_cmp( arg2, "race" )) {
	int race;

	race = race_lookup(arg3);
	if ( race == 0) return;
                victim->race = race;

    } else if (!str_cmp( arg2, "affect" )) {
	int affbit;

	affbit = flag_value( affect_flags, arg3 );
                if (set > 0) SET_BIT(victim->affected_by, affbit);
                else REMOVE_BIT(victim->affected_by, affbit);

     } else if (!str_cmp( arg2, "act" )) {
	int affbit;

	affbit = flag_value( act_flags, arg3 );
                if (set > 0) SET_BIT(victim->act, affbit);
                else REMOVE_BIT(victim->act, affbit);

     } else if (!str_cmp(arg2, "hp")) {
                victim->hit = URANGE(-victim->max_hit, value, victim->max_hit);
                update_pos(victim);

     } else if (!str_cmp(arg2, "mana")) {
                victim->mana = URANGE(0, value, victim->max_mana);
                update_pos(victim);

     } else if (!str_cmp(arg2, "move")) {
                victim->move = URANGE(-victim->max_move, value, victim->max_move);
                update_pos(victim);
     }

      return; 
}


void do_mpstate( CHAR_DATA *ch, char *args ) {
    char arg1 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    if ( !can_use_mp_commands(ch) ) return;
    
    args = one_argument(args, arg1);

    victim  = get_char_room(ch, arg1);
    if (victim == NULL) return;

    args = one_argument(args, arg1);

    if (!str_cmp(arg1, "conv")) {
        int conv, sub, state;

        if (IS_NPC(victim)) return;
        args = one_argument(args, arg1);
        conv = atoi(arg1);
        args = one_argument(args, arg1);
        sub = atoi(arg1);
        args = one_argument(args, arg1);
        state = atoi(arg1);

        if (conv > 0
        && sub > 0
        && state >=0) {
            set_csr(victim, conv, sub, state);
        }
        return;

    }else if (!str_cmp(arg1, "chat")) {
        int chat;

        if (!IS_NPC(victim)) return;
        args = one_argument(args, arg1);
        chat = atoi(arg1);
        victim->chat_state = chat;
        return;
    }
    return;
}


void do_mpartifact( CHAR_DATA *ch, char *args ) {
char arg1 [MAX_INPUT_LENGTH];
char buf [MAX_STRING_LENGTH];
OBJ_DATA *obj;
OBJ_DATA *cont;
ROOM_INDEX_DATA *room = NULL;
bool found = FALSE;

    if ( !can_use_mp_commands(ch) ) return;

    args = one_argument(args, arg1);

    for (obj = object_list; obj; obj=obj->next) {
           if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)
           || IS_SET(obj->pIndexData->extra_flags, ITEM_ARTIFACT)) {
              if (!str_cmp(arg1, "all")
              || is_name(arg1, obj->name)) {
                   if (!found) {
                       found = TRUE;
                       do_say(ch, "Yes, i know about the whereabouts of mystical artifacts.");
                   }
                   if (obj->in_room) {
                       room = obj->in_room;
                   } else if (obj->carried_by) {
                       room = obj->carried_by->in_room;
                   } else if (obj->in_obj) {
                       cont = obj->in_obj;
                       while (!cont->in_room && !cont->carried_by) {
                              cont = cont->in_obj;
                       }
                       if (cont->in_room) room = cont->in_room;
                       else room = cont->carried_by->in_room;
                   }
                   
                   if (!room) continue;
                   sprintf(buf, "%s is somewhere in the vicinity of %s", capitalize(obj->short_descr), room->name);
                   do_say(ch, buf);
              }
           }
    }
    if (!found) do_say(ch, "I can't tell you anything about those artifacts.");
    return;
}


void do_mpemergency( CHAR_DATA *ch, char *args ) {
char arg [MAX_INPUT_LENGTH];
CHAR_DATA *guard;
bool sub = FALSE;

    if ( !can_use_mp_commands(ch) ) return;

    args = one_argument(args, arg);
    if (!str_cmp(arg, "subarea")) sub = TRUE;

    for (guard = char_list; guard; guard = guard->next) {
           if (!IS_SET(guard->off_flags, ASSIST_GUARD)) continue;
           if (guard->in_room->area != ch->in_room->area) continue;
           if (sub && guard->in_room->subarea != ch->in_room->subarea) continue;
           if (number_percent() >25 || IS_SET(guard->act, ACT_SENTINEL)) continue;

           char_from_room(guard);
           char_to_room(guard, ch->in_room);
           act( "$n hurries in.", guard, NULL, NULL, TO_ROOM);
    }

    return;
}


void do_mppay( CHAR_DATA *ch, char *args ) {
char arg [MAX_INPUT_LENGTH];
CHAR_DATA *victim;
int cost;
int currency = find_currency(ch);

    if ( !can_use_mp_commands(ch) ) return;

    args = one_argument(args, arg);
    if ((victim = get_char_room(ch, arg)) == NULL) return;
        
    if (!is_number(args)) return;
    cost = atoi(args);

    if (!can_see(victim, ch)) return;
   
    if (cost > 0) {
        if (cost > victim->gold[currency]) cost = victim->gold[currency];
        if (cost == 0) return;

        sprintf_to_char(victim, "{mYou pay %s %d %s.{x\r\n", ch->short_descr, cost, flag_string(currency_type, currency));
        victim->gold[currency] -= cost;

    } else { 
        cost = -cost;
        if (cost > ch->gold[currency]) cost = ch->gold[currency];
        if (cost == 0) return;

        sprintf_to_char(victim, "{m%s pays you %d %s.{x\r\n", capitalize(ch->short_descr), cost, flag_string(currency_type, currency));
        ch->gold[currency] -= cost;
        victim->gold[currency] +=cost;
    }
    return;
}


void do_mpreset( CHAR_DATA *ch, char *args ) {
ROOM_INDEX_DATA *room;
ROOM_INDEX_DATA *room2;
AREA_DATA *area;
int iroom;

    if ( !can_use_mp_commands(ch) ) return;

    if ((room = ch->in_room) == NULL) return;
    if ((area = room->area) == NULL) return;
    
    if (!str_cmp(args, "room")) {
        reset_room(room, FALSE, TRUE);
    } else if (!str_cmp(args, "subarea")) {
       for (iroom = area->lvnum; iroom <= area->uvnum; iroom++) {
          room2 = get_room_index(iroom);
          if (room2) {
              if (room->subarea == room2->subarea) reset_room(room, FALSE, TRUE);
          }
       }
    } else {
        reset_area(area, FALSE);
    }

    return;
}


void do_mpnote( CHAR_DATA *ch, char *args ) {
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
char arg3[MAX_INPUT_LENGTH];

    if ( !can_use_mp_commands(ch)) return;

    args = one_argument(args, arg1);
    args = one_argument(args, arg2);
    args = one_argument(args, arg3);

    make_note (arg1, ch->short_descr, arg2, arg3, 7, args);
    return;
}


void do_mppassport( CHAR_DATA *ch, char *args ) {
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
char arg3[MAX_INPUT_LENGTH];
CHAR_DATA *victim;
int crime;

    if ( !can_use_mp_commands(ch)) return;

    args = one_argument(args, arg1);
    args = one_argument(args, arg2);
    args = one_argument(args, arg3);

    if ((victim = get_char_room(ch, arg1)) == NULL) return;
    
    if (!str_cmp(arg2, "add")) {
       if ((crime = atoi(arg3)) < 1) return;
       add_passport_entry(victim, crime);

    } else if (!str_cmp(arg2, "remove")) {
       if (!str_cmp(arg3, "all")) crime = -1;
       else crime = atoi(arg3);
       clear_passport_entry(victim, crime);
    }

    return;
}  


void do_mppeace( CHAR_DATA *ch, char *args ) {
CHAR_DATA *rch = NULL;

    if ( !can_use_mp_commands(ch)) return;

    if (args[0] != '\0') rch = get_char_room(ch, args);
    if (rch) {
           if (rch->fighting) stop_fighting(rch, TRUE );
           if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))  REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
           return;
    }

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )    {
	if ( rch->fighting) stop_fighting( rch, TRUE );
	if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))  REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
    }
    return;
}


void do_mpanswer(CHAR_DATA *ch, char *args) {
char arg1[MAX_INPUT_LENGTH];
CHAR_DATA *rch;

    if ( !can_use_mp_commands(ch)) return;

    args = one_argument(args, arg1);

    if ((rch = get_char_room(ch, arg1)) == NULL) return;

    chatperform(ch, rch, args);
    return;
}


void do_mppktimer(CHAR_DATA *ch, char *args) {
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
CHAR_DATA *victim;
int type = 0;
int timer;

    if ( !can_use_mp_commands(ch)) return;

    args = one_argument(args, arg1);
    args = one_argument(args, arg2);

    if ((victim = get_char_room(ch, arg1)) == NULL) return;
    if (IS_NPC(victim) || !victim->pcdata) return;

    if (arg2[0] =='+') {
        type = 1;
        args = one_argument(args, arg2);
    } else if (arg2[0] =='-') {
        type = -1;
        args = one_argument(args, arg2);
    }

    timer = atoi(arg2);
    if (timer <= 0) return;

    switch(type) {
       case -1:
         victim->pcdata->pk_timer -= timer;
         break;

       case 0:
         victim->pcdata->pk_timer = timer;
         break;

       case 1:
         victim->pcdata->pk_timer += timer;
         break;
    }

    victim->pcdata-> pk_timer = URANGE(0, victim->pcdata->pk_timer, 59);
    sprintf_to_char(victim, "You're now protected from PK for {r%d{x minutes.\r\n", victim->pcdata->pk_timer);
    return;
}



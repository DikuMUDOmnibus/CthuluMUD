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
#include "affect.h"
#include "doors.h"
#include "wev.h"
#include "mob.h"
#include "profile.h"
#include "olc.h"
#include "tracks.h"
#include "exp.h"
#include "society.h"
#include "race.h"
#include "gsn.h"
#include "econd.h"
#include "partner.h"

/* command procedures needed */
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_recall);
DECLARE_DO_FUN(do_stand);
DECLARE_DO_FUN(do_recruit);
DECLARE_DO_FUN(do_flee);

void 	panic 			(CHAR_DATA *ch);
void 	check_ritual_interrupt	(CHAR_DATA *ch);
void 	cancel_acv		(CHAR_DATA *ch);
void 	figurine_trigger		(OBJ_DATA *obj, OBJ_DATA *cont, CHAR_DATA *ch);

const	short	rev_dir		[]		=
{
    DIR_SOUTH, DIR_WEST, DIR_NORTH, DIR_EAST,
    DIR_DOWN, DIR_UP,
    DIR_SW, DIR_NW, DIR_NE, DIR_SE,
    DIR_OUT, DIR_IN 
};

char *const dir_name[] = {
    "north", "east", "south", "west", 
    "up", "down", 
    "northeast", "southeast", "southwest", "northwest",
    "in", "out",
};

const	short	movement_loss	[SECT_MAX]	=
{
    1,		// Inside
    2,		// City
    2,		// Fields
    6,		// Forest
    4,		// Hills
    6,		// Mountains
    4,		// Water - swim 
    1,		// Water - no_swim
    6,		// ? 	
   10,		// Air 
    6,		// Desert 
    4,		// Underground 
    8,		// Swamp 
    4,		// Moor 
   10,		// Space
    8,		// Underwater
    4,		// Small fire
    5,		// Fire
    6,		// Big Fire	
    4,		// Cold
    6,		// Acid
    4,		// Lightning
    4,		// Holy
    4,		// Evil
    8,		// Jungle
    2,		// Path
    1,		// Road
    2		// Plains
};

/* Local functions... */

int	find_door	args( ( CHAR_DATA *ch, char *arg ) );
bool	has_key		args( ( CHAR_DATA *ch, int key ) );
bool	has_locker_key	args( ( CHAR_DATA *ch ) );
void         kill_msg          ( CHAR_DATA *ch);  
void move_vehicle( CHAR_DATA *ch, int door);

/* External functions... */

extern void add_open_locker(char *name, OBJ_DATA *obj);
extern void remove_open_locker(OBJ_DATA *obj);
extern bool is_locker_open(char *name);

extern void save_locker( char *name, OBJ_DATA *cont, bool flush);
extern void load_locker(char *name, OBJ_DATA *cont);

/* Generate a silly message when a character moves... */


void move_message(CHAR_DATA *ch, ROOM_INDEX_DATA *in_room, ROOM_INDEX_DATA *to_room ) {
char buf[100];
char weathermsg[20];
char walkmsg[10];
weathermsg[0] = '\0';
walkmsg[0] = '\0';

	if (time_info.season != SEASON_WINTER) /* summer */
        {
	    switch ( weather_info.sky )
	    {
		case SKY_CLOUDLESS:
		    strcat(weathermsg,"hot");
		    strcat(walkmsg,"walk");
		    break;
		case SKY_CLOUDY:
		    strcat(weathermsg,"warm");
		    strcat(walkmsg,"walk");
		    break;
		case SKY_RAINING:
		    strcat(weathermsg,"wet");
		    strcat(walkmsg,"splash");
		    break;
		case SKY_LIGHTNING:
		    strcat(weathermsg,"stormy");
		    strcat(walkmsg,"splash");
		    break;
	    } /* end of switch */
	} /* end of if */
	else
	{
	    switch ( weather_info.sky)
	    {
		case SKY_CLOUDLESS:
		    strcat(weathermsg,"frigid");
		    strcat(walkmsg,"walk");
		    break;
		case SKY_CLOUDY:
		    strcat(weathermsg,"cold");
		    strcat(walkmsg,"walk");
		    break;
		case SKY_RAINING:
		    strcat(weathermsg,"snowy");
		    strcat(walkmsg,"trudge");
		    break;
		case SKY_LIGHTNING:
		    strcat(weathermsg,"blizzard-stricken");
		    strcat(walkmsg,"trudge");
		    break;
	    } /* end of switch */
	}  /* end of else */

	if ( IS_AFFECTED(ch,AFF_FLYING)
                || IS_AFFECTED(ch, AFF_MIST)) {
	    walkmsg[0] = '\0';
	    strcat(walkmsg,"fly");
	}

	/* leaving an indoors room */

        if ( ( in_room->sector_type == SECT_INSIDE )
	   ||( in_room->sector_type == SECT_UNDERGROUND ) )
	{
	switch(to_room->sector_type)
	{
            case SECT_ACID:
            case SECT_HOLY:
            case SECT_EVIL:
	    case SECT_WATER_SWIM:
	    case SECT_WATER_NOSWIM:
	    case SECT_INSIDE:
	    case SECT_MAX: 
	    case SECT_UNDERGROUND:
                break; /* no message for this movement */

            case SECT_SMALL_FIRE:
            case SECT_FIRE:
            case SECT_BIG_FIRE:  
                send_to_char("{RYou run into the flames!{x\r\n", ch);
                break; 

            case SECT_UNDERWATER:
                send_to_char("{CSPLASH!{x\r\n", ch);
                break;

            case SECT_COLD:
                send_to_char("{WBrrrr!{x\r\n", ch);
                break;

            case SECT_LIGHTNING:
                send_to_char("You walking under stormy skies.\r\n", ch);
                break; 
   
	    default:
	        buf[0] = '\0';
		sprintf(buf,"You %s into the %s %s.\r\n", walkmsg, weathermsg, flag_string(sector_name, to_room->sector_type) ); 
		send_to_char(buf,ch);
		break;
	} /* end of switch */
	} /* end of indoors room leave routine */

	/* leaving a city */

        if ( in_room->sector_type == SECT_CITY )
        {
        switch(to_room->sector_type)
	{
	    case SECT_CITY:
	    case SECT_WATER_SWIM:
	    case SECT_WATER_NOSWIM:
            case SECT_UNDERWATER:
            case SECT_SMALL_FIRE:
            case SECT_FIRE:
            case SECT_BIG_FIRE:
            case SECT_COLD:
            case SECT_ACID:
            case SECT_LIGHTNING:
            case SECT_HOLY:
            case SECT_EVIL:    
	    case SECT_MAX:
                break; /* no message for this movement */
	    case SECT_INSIDE:
                buf[0] = '\0';
		sprintf(buf,"You leave the %s city streets and take shelter indoors.\r\n",	weathermsg );
		send_to_char(buf,ch);
		break;
	    case SECT_UNDERGROUND:
                send_to_char("You go underground.\r\n",ch);
		break;
	    default:
                buf[0] = '\0';
                sprintf(buf,"You %s out of the city for the %s %s.\r\n",
                        walkmsg,
                        weathermsg,
                        flag_string(sector_name, to_room->sector_type) );
                send_to_char(buf,ch);
		break;
	} /* end of switch */
	} /* end of leaving city */

	/* leaving a field, forest, hills, or mountain. */

        if ( ( in_room->sector_type == SECT_FIELD )
	    || (in_room->sector_type == SECT_FOREST )
	    || (in_room->sector_type == SECT_JUNGLE )
	    || (in_room->sector_type == SECT_PLAIN )
	    || (in_room->sector_type == SECT_HILLS )
	    || (in_room->sector_type == SECT_DESERT )
	    || (in_room->sector_type == SECT_MOORS )
	    || (in_room->sector_type == SECT_SWAMP )
	    || (in_room->sector_type == SECT_MOUNTAIN ) )
        {
	switch (to_room->sector_type)
	{
            case SECT_ACID:
            case SECT_HOLY:
            case SECT_EVIL:
	    case SECT_WATER_SWIM:
	    case SECT_WATER_NOSWIM:
	    case SECT_MAX: 
                break; /* no message for this movement */

            case SECT_SMALL_FIRE:
            case SECT_FIRE:
            case SECT_BIG_FIRE:  
                send_to_char("{RYou run into the flames!{x\r\n", ch);
                break; 

            case SECT_UNDERWATER:
                send_to_char("{CSPLASH!{x\r\n", ch);
                break;

            case SECT_COLD:
                send_to_char("{WBrrrr!{x\r\n", ch);
                break;

            case SECT_LIGHTNING:
                send_to_char("You walking under stormy skies.\r\n", ch);
                break; 
   
	    case SECT_INSIDE:
                buf[0] = '\0';
		sprintf(buf,"You take shelter indoors from the %s %s.\r\n",
			weathermsg,
			flag_string( sector_name, in_room->sector_type) );
		send_to_char(buf,ch);
		break;

	    case SECT_UNDERGROUND:
		send_to_char("You go underground.\r\n",ch);
		break;

	    case SECT_CITY:
		sprintf(buf,"You leave the %s and enter a %s city.\r\n",
			flag_string( sector_name, in_room->sector_type),
			weathermsg );
		send_to_char(buf,ch);
		break;

	    default:
		if (in_room->sector_type == to_room->sector_type)
			break; /* no msg if no change */
		sprintf(buf,"You head into the %s.\r\n",
			flag_string( sector_name, to_room->sector_type) );
		send_to_char(buf,ch);
		break;
	} /* end switch */
        } /* end of leaving forest, field, hills, mountain, desert */

	/* leaving the air */

	if ( (in_room->sector_type == SECT_AIR)
	  || (in_room->sector_type == SECT_SPACE) )
	{
	    switch (to_room->sector_type)
	    {
		case SECT_AIR:
                case SECT_SPACE: 
		case SECT_MAX:
			break; /* no msg */
                case SECT_HOLY:
                case SECT_EVIL:
                        send_to_char("You land.\r\n", ch);
		default:
			buf[0]='\0';
			sprintf(buf,"You land in the %s %s.\r\n",
			    weathermsg,
			    flag_string(sector_name, to_room->sector_type) );
			send_to_char(buf,ch);
			break;
	    } /* end switch */
	} /* end leaving the air */

	return;
}
  
/* Move character X in direction Y to room Z... */

void make_char_move(CHAR_DATA *ch, int door, ROOM_INDEX_DATA *to_room, int  move) {
CHAR_DATA *fch;
CHAR_DATA *fch_next;
OBJ_DATA *trap;
EXIT_DATA *pexit;
ROOM_INDEX_DATA *in_room = ch->in_room;
MOB_CMD_CONTEXT *mcc;
WEV *wev = NULL;

   /* Tell everyone we're leaving... */

    if (ch->wimpy_dir < DIR_NONE) ch->wimpy_dir = -2 - rev_dir[door];

    pexit = ch->in_room->exit[door];

    if ( !IS_SET(ch->plr, PLR_WIZINVIS)) { 
      mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, door, dir_name[door]);

      if (ch->position == POS_STUNNED) {

           wev = get_wev(WEV_DEPART, WEV_DEPART_WALK, mcc,
                      NULL,
                      NULL,
                     "{W@a2 is being carried @t0.{x\r\n");

      } else { 
           
           if (IS_NPC(ch)) {

              if (IS_AFFECTED(ch, AFF_FLYING)) {
                      wev = get_wev(WEV_DEPART, WEV_DEPART_FLY, mcc,
                      NULL,
                      NULL,
                      "{Y@a2 flies @t0.{x\r\n");

              } else if (IS_AFFECTED(ch, AFF_MIST)) {
                      wev = get_wev(WEV_DEPART, WEV_DEPART_FLY, mcc,
                      NULL,
                      NULL,
                      "{YA cloud of mist flies @t0.{x\r\n");

              } else if ( IS_AFFECTED(ch, AFF_SNEAK) ) {
                      wev = get_wev(WEV_DEPART, WEV_DEPART_SNEAK, mcc,
                      NULL,
                      NULL,
                      NULL);

              } else  if ( in_room->sector_type == SECT_WATER_NOSWIM
              || to_room->sector_type == SECT_WATER_NOSWIM ) {
                      wev = get_wev(WEV_DEPART, WEV_DEPART_SAIL, mcc,
                      NULL,
                      NULL,
                      "{C@a2 sails @t0.{x\r\n");

              } else  if ( in_room->sector_type == SECT_WATER_SWIM
              || in_room->sector_type == SECT_UNDERWATER
              || to_room->sector_type == SECT_WATER_SWIM
              || to_room->sector_type == SECT_UNDERWATER ) {
                      wev = get_wev(WEV_DEPART, WEV_DEPART_SWIM, mcc,
                      NULL,
                      NULL,
                      "{C@a2 swims @t0.{x\r\n");

              } else if (IS_FLEEING(ch)) {

                      if (str_cmp(race_array[ch->race].name,"machine")
                      && !IS_SET(ch->form, FORM_MACHINE)) {
                           wev = get_wev(WEV_DEPART, WEV_DEPART_FLEE, mcc,
                           NULL,
                           NULL,
                           "{W@a2 flees @t0.{x\r\n");
                      } else {
                           wev = get_wev(WEV_DEPART, WEV_DEPART_WALK, mcc,
                           NULL,
                           NULL,
                          "{W@a2 drives @t0.{x\r\n");
                      }

               } else {

                      if (str_cmp(race_array[ch->race].name,"machine")
                      && !IS_SET(ch->form, FORM_MACHINE)) {
                           wev = get_wev(WEV_DEPART, WEV_DEPART_WALK, mcc,
                           NULL,
                           NULL,
                           "{W@a2 walks @t0.{x\r\n");
                      } else {
                           wev = get_wev(WEV_DEPART, WEV_DEPART_WALK, mcc,
                           NULL,
                           NULL,
                           "{W@a2 drives @t0.{x\r\n");
                      }

               }

           } else {

               if (ch->pcdata->mounted==TRUE) {
                      wev = get_wev(WEV_DEPART, WEV_DEPART_WALK, mcc,
                      NULL,
                      NULL,
                      "{W@a2 rides @t0.{x\r\n");
               } else {

                   if (IS_AFFECTED(ch, AFF_FLYING)) {
                      wev = get_wev(WEV_DEPART, WEV_DEPART_FLY, mcc,
                      NULL,
                      NULL,
                      "{Y@a2 flies @t0.{x\r\n");

                   }else if (IS_AFFECTED(ch, AFF_MIST)) {
                      wev = get_wev(WEV_DEPART, WEV_DEPART_FLY, mcc,
                      NULL,
                      NULL,
                      "{Y Acloud of mist flies @t0.{x\r\n");

                   }else if ( IS_AFFECTED(ch, AFF_SNEAK) ) {
                      wev = get_wev(WEV_DEPART, WEV_DEPART_SNEAK, mcc,
                      NULL,
                      NULL,
                      NULL);

                    } else  if ( in_room->sector_type == SECT_WATER_NOSWIM
                    || to_room->sector_type == SECT_WATER_NOSWIM ) {
                      wev = get_wev(WEV_DEPART, WEV_DEPART_SAIL, mcc,
                      NULL,
                      NULL,
                      "{C@a2 sails @t0.{x\r\n");

                    } else  if ( in_room->sector_type == SECT_WATER_SWIM
                    || in_room->sector_type == SECT_UNDERWATER
                    || to_room->sector_type == SECT_WATER_SWIM
                    || to_room->sector_type == SECT_UNDERWATER ) {

                      wev = get_wev(WEV_DEPART, WEV_DEPART_SWIM, mcc,
                      NULL,
                      NULL,
                      "{C@a2 swims @t0.{x\r\n");

                    } else if (IS_FLEEING(ch)) {
                     wev = get_wev(WEV_DEPART, WEV_DEPART_FLEE, mcc,
                      NULL,
                      NULL,
                      "{W@a2 flees @t0.{x\r\n");

                    } else {
                      wev = get_wev(WEV_DEPART, WEV_DEPART_WALK, mcc,
                      NULL,
                      NULL,
                      "{W@a2 walks @t0.{x\r\n");
                    }
               }
           }
     }
   }
     /* Ask if we can leave... */     

      if (!room_issue_wev_challange(in_room, wev)) {
        free_wev(wev);
        return;
      }

   /* Do the move... */

    if (ch->fighting != NULL) stop_fighting(ch, TRUE);
    if (!IS_AFFECTED(ch, AFF_SNEAK)) add_tracks(ch, DIR_NONE, door);
    if (pexit && pexit->transition) {
       sprintf_to_char(ch, "%s\n", pexit->transition);
    }
    char_from_room(ch);
    char_to_room( ch, to_room );
    if (!IS_AFFECTED(ch, AFF_SNEAK)) add_tracks(ch, rev_dir[door], DIR_HERE);
    

   /* Pay the price... */

    if (!IS_NPC(ch)) {

      WAIT_STATE( ch, 1 );

     /*Being carried around does NOT affect moves*/

     if (ch->position == POS_STANDING 
     || ch->master==NULL ) {
           if (ch->pcdata->mounted == TRUE) {
               if (str_cmp(race_array[ch->pet->race].name,"machine")
               && !IS_SET(ch->pet->form, FORM_MACHINE)) {
                     ch->pet->move -=move;
                     if (ch->pet->move < 0)  ch->pet->move = 0;
               }
            } else {    
                 if (str_cmp(race_array[ch->race].name,"machine")
                 && !IS_SET(ch->form, FORM_MACHINE)) {
                     ch->move -= move;
                     if (ch->move < 0) ch->move = 0;
                }
            }
     } else {
                ch->master->move -=move;
                 if (ch->master->move < 0)   ch->master->move = 0;
      }
    }
   /* Tell the old room that we've gone... */

    if ( !IS_SET(ch->plr, PLR_WIZINVIS) ) {
      room_issue_wev(in_room, wev);
 
      free_wev(wev);
    }

   /* Tell everyone we've arrived... */

    if ( !IS_SET(ch->plr, PLR_WIZINVIS) ) {

     /* Work out where we came from... */

      door = rev_dir[door];

      mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, door, dir_name[door]);

     
      if (ch->position == POS_STUNNED) {

        wev = get_wev(WEV_ARRIVE, WEV_ARRIVE_WALK, mcc,
                      NULL,
                      NULL,
                     "{W@a2 is being carried in.{x\r\n");

       } else { 
     
      if (IS_NPC(ch)) {

      if (IS_AFFECTED(ch, AFF_FLYING)) {
        wev = get_wev(WEV_ARRIVE, WEV_ARRIVE_FLY, mcc,
                      NULL,
                      NULL,
                     "{Y@a2 flies in.{x\r\n");

      } else if (IS_AFFECTED(ch, AFF_MIST)) {
        wev = get_wev(WEV_ARRIVE, WEV_ARRIVE_FLY, mcc,
                      NULL,
                      NULL,
                     "{YA cloud of mist flies in.{x\r\n");

      } else if ( IS_AFFECTED(ch, AFF_SNEAK) ) {
        wev = get_wev(WEV_ARRIVE, WEV_ARRIVE_SNEAK, mcc,
                      NULL,
                      NULL,
                      NULL);

      } else  if ( in_room->sector_type == SECT_WATER_NOSWIM
                || to_room->sector_type == SECT_WATER_NOSWIM ) {

        wev = get_wev(WEV_ARRIVE, WEV_ARRIVE_SAIL, mcc,
                      NULL,
                      NULL,
                     "{C@a2 sails over.{x\r\n");

      } else  if ( in_room->sector_type == SECT_WATER_SWIM
                || in_room->sector_type == SECT_UNDERWATER
                || to_room->sector_type == SECT_WATER_SWIM
                || to_room->sector_type == SECT_UNDERWATER ) {

        wev = get_wev(WEV_ARRIVE, WEV_ARRIVE_SWIM, mcc,
                      NULL,
                      NULL,
                     "{C@a2 swims over.{x\r\n");

      } else if (IS_FLEEING(ch)) {

        wev = get_wev(WEV_ARRIVE, WEV_ARRIVE_FLEE, mcc,
                      NULL,
                      NULL,
                     "{W@a2 dashes in looking harried.{x\r\n");

      } else {

        wev = get_wev(WEV_ARRIVE, WEV_ARRIVE_WALK, mcc,
                      NULL,
                      NULL,
                     "{W@a2 arrives.{x\r\n");
      }

     } else {

           if (ch->pcdata->mounted==TRUE) {
              wev = get_wev(WEV_ARRIVE, WEV_ARRIVE_WALK, mcc,
                      NULL,
                      NULL,
                     "{W@a2 rides in.{x\r\n");
            } else {

      if (IS_AFFECTED(ch, AFF_FLYING)) {
        wev = get_wev(WEV_ARRIVE, WEV_ARRIVE_FLY, mcc,
                      NULL,
                      NULL,
                     "{Y@a2 flies in.{x\r\n");

      } else if (IS_AFFECTED(ch, AFF_MIST)) {
        wev = get_wev(WEV_ARRIVE, WEV_ARRIVE_FLY, mcc,
                      NULL,
                      NULL,
                     "{YA cloud of mist flies in.{x\r\n");

      } else if ( IS_AFFECTED(ch, AFF_SNEAK) ) {
        wev = get_wev(WEV_ARRIVE, WEV_ARRIVE_SNEAK, mcc,
                      NULL,
                      NULL,
                      NULL);

      } else  if ( in_room->sector_type == SECT_WATER_NOSWIM
                || to_room->sector_type == SECT_WATER_NOSWIM ) {

        wev = get_wev(WEV_ARRIVE, WEV_ARRIVE_SAIL, mcc,
                      NULL,
                      NULL,
                     "{C@a2 sails over.{x\r\n");

      } else  if ( in_room->sector_type == SECT_WATER_SWIM
                || in_room->sector_type == SECT_UNDERWATER
                || to_room->sector_type == SECT_WATER_SWIM
                || to_room->sector_type == SECT_UNDERWATER ) {

        wev = get_wev(WEV_ARRIVE, WEV_ARRIVE_SWIM, mcc,
                      NULL,
                      NULL,
                     "{C@a2 swims over.{x\r\n");

      } else if (IS_FLEEING(ch)) {

        wev = get_wev(WEV_ARRIVE, WEV_ARRIVE_FLEE, mcc,
                      NULL,
                      NULL,
                     "{W@a2 dashes in looking harried.{x\r\n");

      } else {

        wev = get_wev(WEV_ARRIVE, WEV_ARRIVE_WALK, mcc,
                      NULL,
                      NULL,
                     "{W@a2 arrives.{x\r\n");
      }

      }
     }

     }
      room_issue_wev(to_room, wev);
 
      free_wev(wev);

     /* Put the door back so folks can follow us... */  

      door = rev_dir[door];

    }

   /* Tell them the good news if they are fleeing... */

    if (IS_FLEEING(ch)) {
      send_to_char("{CYou escaped with your life!{x\r\n", ch);
    }

   /* Have a look around now we've got there... */

    do_look( ch, "auto" );

   /* Chop OLC if the area is edit locked... */

    if ( ch->desc != NULL
      && ch->desc->editor == ED_ROOM 
      && IS_SET(to_room->area->area_flags, AREA_EDLOCK) ) {
      send_to_char("{YArea is edit locked!{x\r\n", ch);
      edit_done(ch);
    }


    /* The Trap goes snap */

    trap = ch->in_room->contents;
    while ( trap != NULL
    &&  trap->item_type != ITEM_TRAP) {
             trap = trap->next_content;
    }
    
    if ( trap != NULL ) trap_trigger(ch, NULL);

     if (IS_AFFECTED(ch, AFF_AURA)) panic (ch);
     if (!IS_SET(ch->plr, PLR_NOFOLLOW)) do_recruit(ch, "");

   /* Send pets and followers through... */

    for ( fch = in_room->people; fch != NULL; fch = fch_next ) {

        fch_next = fch->next_in_room;

	if ( fch->master == ch 
               && fch->position == POS_STANDING ) {
                    if ((str_cmp(race_array[fch->race].name,"machine")
                        && !IS_SET(fch->form, FORM_MACHINE))
                    || fch->ridden == TRUE) {

   	         if ((IS_SET(ch->in_room->room_flags, ROOM_LAW) || IS_SET(ch->in_room->area->area_flags, AREA_LAW))
	         && IS_NPC(fch) 
                         && IS_SET(fch->act,ACT_AGGRESSIVE)) {
		act("You can't bring $N here.", ch,NULL,fch,TO_CHAR);
		act("You aren't allowed here.", fch,NULL,NULL,TO_CHAR);
		return;
	         }
   	         act( "You follow $N.", fch, NULL, ch, TO_CHAR );
  	         move_char( fch, door, TRUE );
                     }                
                }

                /*Carry the stunned*/
	if ( fch->master == ch 
               && fch->position == POS_STUNNED ) {
                   act( "$N carries you away.", fch, NULL, ch, TO_CHAR );
                   move_char( fch, door, TRUE );
	}


    }

   /* All done... */

    return;
}

/* Move a character... */

void move_char( CHAR_DATA *ch, int door, bool follow ) {
ROOM_INDEX_DATA *in_room;
ROOM_INDEX_DATA *to_room;
EXIT_DATA *pexit;
int move;

   /* No transdimensional movement... */

    if ( door <= DIR_NONE || door >= DIR_MAX ) {
	bug( "move_char: bad door %d.", door );
                send_to_char("You really don't want to go there!\r\n", ch);
	return;
    }

   /* Where am I? */

    in_room = ch->in_room;

   /* Can't move when fighting... */

    if (ch->fighting != NULL) {
      send_to_char("To leave a fight you must flee!\r\n", ch);
      return;
    }

   /* What am I trying to walk through? */

    pexit = in_room->exit[door];

   /* No exit gives a nasty message... */

    if ( pexit   == NULL ) {
	send_to_char( "Alas, you cannot go that way.\r\n", ch );
	return;
    }

   /* Same message if the door is invisible... */

    if (!exit_visible(ch, pexit)) { 
	send_to_char ( "Alas, you cannot go that way.\r\n", ch);
	return;
    }
	
   /* Now, where does it go? */

    to_room = get_exit_destination(ch, ch->in_room, pexit, TRUE);
 
   /* Nowhere is also the same nasty message... */
    if (to_room == NULL) { 
	send_to_char( "Alas, you cannot go that way.\r\n", ch );
	return;
    }

   /* Strange check - same message if you can't see the room. */

    if (!can_see_room(ch, to_room)) {
	send_to_char( "Alas, you cannot go that way.\r\n", ch );
	return;
    }

    if ((IS_RAFFECTED(in_room, RAFF_ENCLOSED)
    || IS_RAFFECTED(to_room, RAFF_ENCLOSED))
    && !IS_IMMORTAL(ch)) {
        send_to_char("A translucent force-field blocks your path.\r\n", ch);
        return;
    } 

    if (IS_SET(to_room->room_flags, ROOM_MUDGATE)) {
        char buf[MAX_STRING_LENGTH];
        CHAR_DATA *partner;
        FILE *fp;
        VNUM dest;
        int rn;

        if ((partner = get_gate_partner(to_room)) != NULL) {
              rn = am_i_partner(partner);
              dest = get_gate_dest(to_room);

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
                 transmove_player(ch, partner, dest);
                 sprintf(buf, "muddelete %s", ch->name);
                 enqueue_mob_cmd(partner, buf, 15, 0);
              }
              return;
        }
    }


   /* Another message if we're looping... */

    if (in_room == to_room) {
        if (ch->move > 0) ch->move -= 1;
        do_look(ch, "auto");
        send_to_char("This place looks familier...\r\n", ch);
        return;
    } 

   /* Pass door lets you do a few odd things... */

    if (!IS_AFFECTED(ch, AFF_PASS_DOOR)) {

     /* Can't walk through closed doors... */

      if ( IS_SET(pexit->exit_info, EX_CLOSED)) {
              if ( IS_SET(pexit->exit_info, EX_WALL)
              || IS_SET(pexit->exit_info, EX_HIDDEN)) {
                    send_to_char( "Alas, you cannot go that way.\r\n", ch );
                    return;
              } else {
                    if (!IS_AFFECTED(ch, AFF_MIST)
                    || IS_SET(pexit->exit_info, EX_NO_PASS)
                    || IS_SET(pexit->exit_info, EX_ROBUST)
                    || IS_SET(pexit->exit_info, EX_ARMOURED)) {
                            act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
                            return;
                    } else {
                            act( "$n diffuses through the door.", ch, NULL, NULL, TO_ROOM );
                    }
              }
      }

    } else {
    
	if ( IS_SET(pexit->exit_info, EX_CLOSED)
                && IS_SET(pexit->exit_info, EX_NO_PASS)) {

	  if (IS_SET(pexit->exit_info, EX_HIDDEN)) {
	    send_to_char ( "Alas, you cannot go that way.\r\n", ch);
                  } else {
	    send_to_char ( "{YA magical barrier blocks the doorway!{x\r\n",ch);
                  }
	  return;
	}
    }

   /* Must stay with your master... */

    if ( IS_AFFECTED(ch, AFF_CHARM)
    &&   ch->master != NULL
    &&   in_room == ch->master->in_room ) {
	send_to_char( "What?  And leave your beloved master?\r\n", ch );
	return;
    }

   /* Cannot enter a private room... */

    if ( room_is_private(ch, to_room ) ) {
	send_to_char( "That room is private right now.\r\n", ch );
	return;
    }

   /* Can't enter a locked area... */

    if ( (in_room->area != NULL)
      && (to_room->area != NULL) 
      && (!IS_SET(in_room->area->area_flags, AREA_LOCKED)) 
      && ( IS_SET(to_room->area->area_flags, AREA_LOCKED)) ) {

     if (!IS_IMMORTAL(ch)) {
        send_to_char("{YA magical gate bars your way!{x\r\n", ch);
        return;
      }
      send_to_char("{MYou feel a tingle crawl across your flesh!{x\r\n", ch); 

    }

    if (ch->position == POS_STANDING ) {

     if (!IS_NPC(ch)) { 
        if (ch->pcdata->mounted==TRUE && !IS_AFFECTED(ch->pet, AFF_FLYING)) {
               if (to_room->sector_type == SECT_INSIDE
               || to_room->sector_type == SECT_WATER_SWIM
               || to_room->sector_type == SECT_WATER_NOSWIM
               || to_room->sector_type == SECT_UNDERGROUND
               || to_room->sector_type == SECT_SWAMP
               || to_room->sector_type == SECT_MOUNTAIN
               || to_room->sector_type == SECT_UNDERWATER) {
                       send_to_char( "You cannot ride there.\r\n", ch );
                       return;
               }
        }
   }

   /* Must fly to enter space of an AIR square... */

    if (to_room->sector_type == SECT_AIR 
    || to_room->sector_type == SECT_SPACE ) {
         if (!IS_AFFECTED(ch, AFF_FLYING)
         && !IS_AFFECTED(ch, AFF_MIST)
         && !IS_IMMORTAL(ch)) {
                if (IS_NPC(ch)) {
                      send_to_char( "You cannot fly.\r\n", ch );
                      return;
                } else {
                      if (ch->pcdata->mounted==TRUE) {
                              if ( !IS_AFFECTED(ch->pet, AFF_FLYING) ) {
                                     send_to_char( "Your mount cannot fly.\r\n", ch );
                                     return;
                               }
                        } else {
                               send_to_char( "You cannot fly.\r\n", ch );
                               return;
                        }   
                 }
          }    
    }

   /* You need to carry (?!) a boat or fly to travel the ocean... */

    if ( ( in_room->sector_type == SECT_WATER_NOSWIM || to_room->sector_type == SECT_WATER_NOSWIM )
      && !IS_AFFECTED(ch, AFF_FLYING)
      && !IS_AFFECTED(ch, AFF_MIST)) {

      OBJ_DATA *obj;
      bool found = FALSE;

      if (IS_IMMORTAL(ch)) found = TRUE;

      obj = ch->carrying;

      while ( !found && obj != NULL ) {
        if ( obj->item_type == ITEM_BOAT ) found = TRUE;
        obj = obj->next_content;
      }

      if ( !found ) {
         send_to_char( "You need a boat to go there.\r\n", ch );
         return;
      }
    }

   /* No flying underwater... */
   /* ...without Globe of Protection */

    if ( ( in_room->sector_type == SECT_UNDERWATER
        || to_room->sector_type == SECT_UNDERWATER )
      && (IS_AFFECTED(ch, AFF_FLYING)
            || IS_AFFECTED(ch, AFF_MIST))
      && !IS_AFFECTED(ch, AFF_GLOBE)) {
         send_to_char( "You can't fly underwater!\r\n", ch);
         return;
    }

   /* Water's sticky... */
 
    if (( in_room->sector_type == SECT_WATER_SWIM || in_room->sector_type == SECT_UNDERWATER )
    && !IS_AFFECTED(ch, AFF_FLYING)
    && !IS_AFFECTED(ch, AFF_MIST)
    && !IS_AFFECTED(ch, AFF_SWIM)) {
          if (!check_skill(ch, gsn_swim, 0)) {
                if (ch->move > 5) ch->move -= 5;
                send_to_char("You splash around quite a lot, but don't seem to get anywhere.\r\n", ch);
                return;
          }
    }

    if ( to_room->sector_type == SECT_MOUNTAIN
    && !IS_AFFECTED(ch, AFF_FLYING)
    && !IS_AFFECTED(ch, AFF_MIST)) {
         int diff;
         if (!IS_NPC(ch)) {
             if (ch->pcdata->mounted==TRUE) {
                   if (!IS_AFFECTED(ch->pet, AFF_FLYING)) {
                       send_to_char( "Your mount cannot fly.\r\n", ch );
                       return;
                   }
             } else {
                   if (in_room->sector_type == SECT_MOUNTAIN) diff = 0;
                   else diff = -20;

                   if (!check_skill(ch, gsn_climb, diff)) {
                       if (ch->move > 15 ) ch->move -= 15;
                       send_to_char("You try to climb this passage but fail.\r\n", ch);
                       return;
                  }
             }
         }
    }


    }
 
   /* Find the price... */

    move = 0;

    if ( !IS_NPC(ch) ) {
    
      move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)] + movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)];
      move /= 2;  /* i.e. the average */

      switch (get_surface(ch->in_room)) {
        case SURFACE_SNOW:
        case SURFACE_FROZEN:
          move += 2;
          break;

        case SURFACE_DEEP_SNOW:
          move += 2;
          move *= 2;
          break;
 
        default:
          break;
      }

      if (IS_AFFECTED(ch, AFF_FLYING)) move = 3;
      if (IS_AFFECTED(ch, AFF_MIST)) move = 10;
 
      if (IS_AFFECTED(ch, AFF_SWIM)
      && (in_room->sector_type == SECT_WATER_SWIM
      || in_room->sector_type == SECT_UNDERWATER )) {
         move /=2;
      } 

      if ( ch->move < move 
      && ch->position == POS_STANDING ) {
	send_to_char( "You are too exhausted.\r\n", ch );
	return;
      }
    }


   /* Reset activity... */

    if (IS_FLEEING(ch)) {
      cancel_acv(ch);
      set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
    } else {
      switch (ch->activity) {

        case ACV_SNEAKING:
        case ACV_TRACKING:
          break;

        default:
          cancel_acv(ch);
          set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
          break;
      }
    }

    check_ritual_interrupt(ch);

   /* Do the actual move... */

    make_char_move(ch, door, to_room, move);

   /* Drive old mob_prog triggers... */

    mprog_entry_trigger( ch );
    mprog_greet_trigger( ch );

   /* All done... */
   
    return;
} 


void do_drag( CHAR_DATA *ch, char *argument ) {
char arg[MAX_STRING_LENGTH];
char arg2[MAX_STRING_LENGTH];
OBJ_DATA *obj;
ROOM_INDEX_DATA *lastroom;
int dir = 0;

    if ( argument[0] == '\0' )    {
	send_to_char( "Drag:\r\n", ch );
	send_to_char( "SYNTAX: DRAG <corpse> <direction>\r\n", ch );
	return;
    }

    if (ch->fighting != NULL) {
        send_to_char("You can't do that while fighting!\r\n", ch);
        return;
    }

    argument = one_argument(argument,arg);
    argument = one_argument(argument,arg2);

    if ((obj = get_obj_here(ch, arg)) == NULL) {
        send_to_char("That's not here!\r\n", ch);
        return;
    }

    if (!obj->in_room) {
        send_to_char("That's not here!\r\n", ch);
        return;
    }

    if (!IS_SET(obj->wear_flags, ITEM_TAKE)) {
        send_to_char("You can't move that!\r\n", ch);
        return;
    }

    if (ch->move < (obj->level + obj->weight)) {
        send_to_char("You are too exhausted!\r\n", ch);
        return;
    }

    if (!str_cmp(arg2, "n")
    || !str_cmp(arg2, "north")) {
         dir = DIR_NORTH;

    } else if (!str_cmp(arg2, "e")
    || !str_cmp(arg2, "east")) {
         dir = DIR_EAST;

    } else if (!str_cmp(arg2, "s")
    || !str_cmp(arg2, "south")) {
         dir = DIR_SOUTH;

    } else if (!str_cmp(arg2, "w")
    || !str_cmp(arg2, "west")) {
         dir = DIR_WEST;

    } else if (!str_cmp(arg2, "ne")
    || !str_cmp(arg2, "northeast")) {
         dir = DIR_NE;

    } else if (!str_cmp(arg2, "se")
    || !str_cmp(arg2, "southeast")) {
         dir = DIR_SE;

    } else if (!str_cmp(arg2, "sw")
    || !str_cmp(arg2, "southwest")) {
         dir = DIR_SW;

    } else if (!str_cmp(arg2, "nw")
    || !str_cmp(arg2, "northwest")) {
         dir = DIR_NW;

    } else if (!str_cmp(arg2, "u")
    || !str_cmp(arg2, "up")) {
         dir = DIR_UP;

    } else if (!str_cmp(arg2, "d")
    || !str_cmp(arg2, "down")) {
         dir = DIR_DOWN;

    } else if (!str_cmp(arg2, "i")
    || !str_cmp(arg2, "in")) {
         dir = DIR_IN;

    } else if (!str_cmp(arg2, "o")
    || !str_cmp(arg2, "out")) {
         dir = DIR_OUT;

    } else {
         send_to_char( "Drag:\r\n", ch );
         send_to_char( "SYNTAX: DRAG <corpse> <direction>\r\n", ch );
         return;
    }

    lastroom = ch->in_room;    
    move_char(ch, dir, FALSE );

    if (ch->in_room != lastroom) {
         ch->move -= (obj->level + obj->weight);

          obj_from_room(obj);
          obj_to_room(obj, ch->in_room);
          act( "{m$n drags $p in.{x", ch, obj, NULL, TO_ROOM );
          act( "{mYou drags $p in.{x", ch, obj, NULL, TO_CHAR );

    }
    return;
}


void do_drive( CHAR_DATA *ch, char *argument ) {
char arg[MAX_STRING_LENGTH];
char arg2[MAX_STRING_LENGTH];
ROOM_INDEX_DATA *outroom;
MOB_CMD_CONTEXT *mcc;
WEV *wev;    
OBJ_DATA *portal;
OBJ_DATA *fuel;
int cost;


    if ( argument[0] == '\0' )    {
	send_to_char( "Drive:\r\n", ch );
	send_to_char( "SYNTAX: DRIVE <direction>\r\n", ch );
	send_to_char( "        DRIVE BEEP\r\n", ch );
	send_to_char( "        DRIVE REFUEL <vehicle>\r\n", ch );
	send_to_char( "        DRIVE REPAIR <vehicle>\r\n", ch );
	return;
    }

    if (ch->fighting != NULL) {
        send_to_char("You can't drive while fighting!\r\n", ch);
        return;
    }
    argument = one_argument(argument,arg);

    if (!str_cmp(arg, "repair")) {
          CHAR_DATA *keeper;

          keeper = find_keeper(ch, TRUE);
          if ( keeper == NULL ) return;

          if (str_cmp("spec_mechanic", spec_string(keeper->spec_fun))) {
	send_to_char( "There is no mechanic.\r\n", ch );
	return;
          }

          argument = one_argument(argument,arg2);
         
          if ( ( portal = get_obj_here(ch, arg2)) == NULL ) {
	send_to_char( "There is nothing to repair here.\r\n", ch );
	return;
          }

           if (portal->item_type != ITEM_PORTAL
           || portal->value[4] != PORTAL_VEHICLE) { 
	send_to_char( "There is nothing to refuel here.\r\n", ch );
	return;
          }
   
          if (portal->condition >= 100) {
	send_to_char( "It's completely ok.\r\n", ch );
	return;
          }

          if (!IS_SET(ch->in_room->room_flags, ROOM_GAS)) {
	send_to_char( "You can't get it repaired here.\r\n", ch );
	return;
          }

           SET_BIT(portal->wear_flags, ITEM_TAKE);
           portal->cost = (100 - portal->condition) * 50;
           if (portal->level > 20) portal->cost *= portal->level / 20;
           
           obj_from_room(portal);
           obj_to_char(portal, keeper);

          act( "$n begins to repair $p.", keeper, portal, NULL,TO_ROOM );
          SET_BIT( portal->pIndexData->area->area_flags, AREA_CHANGED );
          return;
    }

    if (!str_cmp(arg, "refuel")) {

          argument = one_argument(argument,arg2);
         
          if ( ( portal = get_obj_here(ch, arg2)) == NULL ) {
	send_to_char( "There is nothing to refuel here.\r\n", ch );
	return;
          }

           if (portal->item_type != ITEM_PORTAL
           || portal->value[4] != PORTAL_VEHICLE) { 
	send_to_char( "There is nothing to refuel here.\r\n", ch );
	return;
          }
   
          if (portal->weight >= 100) {
	send_to_char( "It's already full.\r\n", ch );
	return;
          }

          if (!IS_SET(ch->in_room->room_flags, ROOM_GAS)) {
               fuel = ch->carrying;
               while ( fuel != NULL
               && (fuel->item_type != ITEM_DRINK_CON
                  || fuel->value[2] != LIQ_PETROL
                  || fuel->wear_loc != -1
                  || !can_see_obj(ch, fuel) ) ) {
                       fuel = fuel->next_content;
               }

               if (fuel == NULL ) {
  	   send_to_char( "You can't refuel here.\r\n", ch );
	   return;
               }

               if (fuel->value[1] == 0) {
  	   send_to_char( "Your container is already empty.\r\n", ch );
	   return;
               }

               fuel->value[1]--;
               portal->weight += 15;
               portal->weight = UMIN(portal->weight, 100);
               return;
          }

          cost = (100 - portal->weight) * 5;

          if (ch->gold[find_currency(ch)] < cost) {
	send_to_char( "You can't afford to fill it completely.\r\n", ch );
                portal->weight += ch->gold[find_currency(ch)]/5;
                portal->pIndexData->weight += ch->gold[find_currency(ch)]/5;
                ch->gold[find_currency(ch)] = 0;
          } else {
	send_to_char( "You refuel it.\r\n", ch );
                portal->weight = 100;
                portal->pIndexData->weight = 100;
                ch->gold[find_currency(ch)] -= cost;
          }
          act( "$n refuels $p.", ch, portal, NULL,TO_ROOM );
          SET_BIT( portal->pIndexData->area->area_flags, AREA_CHANGED );
          return;
    }


    if (!IS_SET(ch->in_room->room_flags, ROOM_VEHICLE)) {
	send_to_char( "You're not in a vehicle.\r\n", ch );
	return;
    }

    if (!str_cmp(arg, "beep")) {
          if (ch->in_room->exit[DIR_OUT] == NULL) {
	send_to_char( "Lost OUT exit on the way.\r\n", ch );
	return;
          }

          outroom = ch->in_room->exit[DIR_OUT]->u1.to_room;

          if (IS_RAFFECTED(outroom, RAFF_SILENCE)) {
	send_to_char( "Nothing seems to happen...\r\n", ch );
	return;
          }

          send_to_char( "You hit the horn.\r\n", ch );

          mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, ch->speak[SPEAK_PUBLIC], "Beep! Beep!");
          wev = get_wev(WEV_ICC, WEV_ICC_SAY, mcc,
              "{g@t0{x\r\n",
               NULL,
              "{g@t0{x\r\n");
    
          if (!room_issue_wev_challange(outroom, wev)) {
                 free_wev(wev);
                 return;
          }
          room_issue_wev(outroom, wev);
          free_wev(wev);
          return;
    }

    if (!str_cmp(arg, "n")
    || !str_cmp(arg, "north")) {
         move_vehicle( ch, DIR_NORTH);
         return;
    }

    if (!str_cmp(arg, "e")
    || !str_cmp(arg, "east")) {
         move_vehicle( ch, DIR_EAST);
         return;
    }

    if (!str_cmp(arg, "s")
    || !str_cmp(arg, "south")) {
         move_vehicle( ch, DIR_SOUTH);
         return;
    }

    if (!str_cmp(arg, "w")
    || !str_cmp(arg, "west")) {
         move_vehicle( ch, DIR_WEST);
         return;
    }

    if (!str_cmp(arg, "ne")
    || !str_cmp(arg, "northeast")) {
         move_vehicle( ch, DIR_NE);
         return;
    }

    if (!str_cmp(arg, "se")
    || !str_cmp(arg, "southeast")) {
         move_vehicle( ch, DIR_SE);
         return;
    }

    if (!str_cmp(arg, "sw")
    || !str_cmp(arg, "southwest")) {
         move_vehicle( ch, DIR_SW);
         return;
    }

    if (!str_cmp(arg, "nw")
    || !str_cmp(arg, "northwest")) {
         move_vehicle( ch, DIR_NW);
         return;
    }

    send_to_char( "You can't drive in such a direction!.\r\n", ch );
    return;
}


void move_vehicle( CHAR_DATA *ch, int door) {
ROOM_INDEX_DATA *in_room;
ROOM_INDEX_DATA *to_room;
EXIT_DATA *pexit;
OBJ_DATA *portal;
MOB_CMD_CONTEXT *mcc;
WEV *wev = NULL;
int portalvnum;

   /* No transdimensional movement... */

    if ( door <= DIR_NONE || door >= DIR_MAX ) {
	bug( "move_char: bad door %d.", door );
                send_to_char("You really don't want to drive there!\r\n", ch);
	return;
    }

   if (ch->in_room->exit[DIR_OUT] == NULL) {
	send_to_char( "Lost OUT exit on the way.\r\n", ch );
	return;
    }

   in_room =  ch->in_room->exit[DIR_OUT]->u1.to_room;
    pexit = in_room->exit[door];

    if ( pexit   == NULL ) {
	send_to_char( "Alas, you cannot drive way.\r\n", ch );
	return;
    }

    if (!exit_visible(ch, pexit)) { 
	send_to_char ( "Alas, you cannot driveway.\r\n", ch);
	return;
    }
	
    to_room = get_exit_destination(ch, in_room, pexit, TRUE);
 
    if (to_room == NULL) { 
	send_to_char( "Alas, you cannot drive way.\r\n", ch );
	return;
    }

    if (IS_SET(to_room->room_flags, ROOM_MUDGATE)
    || !can_see_room(ch, to_room)) {
	send_to_char( "Alas, you cannot drive way.\r\n", ch );
	return;
    }

    if (in_room == to_room) {
        do_look(ch, "auto");
        send_to_char("This place looks familier...\r\n", ch);
        return;
    } 

      if ( IS_SET(pexit->exit_info, EX_ISDOOR)) {
           send_to_char( "You don't fit through the door.\r\n", ch );
           return;
      }

    if ( room_is_private(ch, to_room ) ) {
	send_to_char( "That room is private right now.\r\n", ch );
	return;
    }

    if ( (in_room->area != NULL)
      && (to_room->area != NULL) 
      && (!IS_SET(in_room->area->area_flags, AREA_LOCKED)) 
      && ( IS_SET(to_room->area->area_flags, AREA_LOCKED)) ) {

     if (!IS_IMMORTAL(ch)) {
        send_to_char("{YA magical gate bars your way!{x\r\n", ch);
        return;
      }
      send_to_char("{MYou feel a tingle crawl across your flesh!{x\r\n", ch); 
    }

    if (to_room->sector_type == SECT_INSIDE
    || to_room->sector_type == SECT_WATER_SWIM
    || to_room->sector_type == SECT_WATER_NOSWIM
    || to_room->sector_type == SECT_UNDERGROUND
    || to_room->sector_type == SECT_SWAMP
    || to_room->sector_type == SECT_MOUNTAIN
    || to_room->sector_type == SECT_AIR
    || to_room->sector_type == SECT_SPACE
    || to_room->sector_type == SECT_UNDERWATER) {
            send_to_char( "You cannot drive there.\r\n", ch );
            return;
    }

    /* Move the Portal */

     portalvnum = ch->in_room->vnum;
     portal = in_room->contents;

     while ( portal != NULL
     && (portal->item_type != ITEM_PORTAL
           || portal->pIndexData->vnum != portalvnum)) {
               portal = portal->next_content;
     }
    
     if (portal == NULL ) {
            send_to_char( "Vehicle portal lost - Problem!.\r\n", ch );
            return;
    }

    if (portal->weight <= 0) {
            send_to_char( "You're out of gas.\r\n", ch );
            return;
    }

    if (number_percent() < 5) {
         portal->weight--;
         portal->pIndexData->weight--;
    }

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, door, dir_name[door]);
    wev = get_wev(WEV_DEPART, WEV_DEPART_WALK, mcc,  NULL, NULL, "{W@a2 drives @t0.{x\r\n");

     if (!room_issue_wev_challange(in_room, wev)) {
        free_wev(wev);
        return;
     }

     room_issue_wev(in_room, wev);
     free_wev(wev);
     WAIT_STATE( ch, 1 );

      obj_from_room(portal);
      ch->in_room->exit[DIR_OUT]->u1.to_room = to_room;
      obj_to_room(portal, to_room );
 
      mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, door, dir_name[rev_dir[door]]);
      wev = get_wev(WEV_ARRIVE, WEV_ARRIVE_WALK, mcc,  NULL, NULL, "{W@a2 drives in.{x\r\n");

      if (!room_issue_wev_challange(to_room, wev)) {
        free_wev(wev);
        return;
      }

      room_issue_wev(to_room, wev);
      free_wev(wev);
      
      SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
      do_look( ch, "auto" );
      return;
} 




void do_north( CHAR_DATA *ch, char *argument ) {
    move_char( ch, DIR_NORTH, FALSE );
    return;
}

void do_east( CHAR_DATA *ch, char *argument ) {
    move_char( ch, DIR_EAST, FALSE );
    return;
}

void do_south( CHAR_DATA *ch, char *argument ) {
    move_char( ch, DIR_SOUTH, FALSE );
    return;
}

void do_west( CHAR_DATA *ch, char *argument ) {
    move_char( ch, DIR_WEST, FALSE );
    return;
}

void do_up( CHAR_DATA *ch, char *argument ) {
    move_char( ch, DIR_UP, FALSE );
    return;
}

void do_down( CHAR_DATA *ch, char *argument ) {
    move_char( ch, DIR_DOWN, FALSE );
    return;
}

void do_northeast( CHAR_DATA *ch, char *argument ) {
    move_char( ch, DIR_NE, FALSE );
    return;
}

void do_southeast( CHAR_DATA *ch, char *argument ) {
    move_char( ch, DIR_SE, FALSE );
    return;
}

void do_southwest( CHAR_DATA *ch, char *argument ) {
    move_char( ch, DIR_SW, FALSE );
    return;
}

void do_northwest( CHAR_DATA *ch, char *argument ) {
    move_char( ch, DIR_NW, FALSE );
    return;
}

void do_in( CHAR_DATA *ch, char *argument ) {
    move_char( ch, DIR_IN, FALSE );
    return;
}

void do_out( CHAR_DATA *ch, char *argument ) {
    move_char( ch, DIR_OUT, FALSE );
    return;
}


/* Run away from trouble... */

void do_flee( CHAR_DATA *ch, char *argument ) {
CHAR_DATA *victim;
ROOM_INDEX_DATA *dest;
int door, count, move, loss;
char buf[MAX_STRING_LENGTH];
bool tried[DIR_MAX];

    if ( ch->position < POS_FIGHTING) return;

   /* Can't leave your master... */

    if ( IS_AFFECTED(ch, AFF_CHARM)
      && ch->master != NULL
      && ch->master->in_room == ch->in_room) {
          send_to_char("{YYou may not desert your master!{x\r\n", ch);
          return;
    }

    if ( is_affected(ch, gafn_frenzy) 
    || IS_AFFECTED(ch, AFF_BERSERK) ) {
         send_to_char("You won't leave until one of you is dead!\r\n", ch);
         return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_NO_FLEE)
    || (ch->waking_room != NULL
       && ch->nightmare == TRUE
       && number_percent() < 80)) {
            send_to_char("Your feet seem to be frozen on the floor!\r\n", ch);
            return;
    }

    if (IS_RAFFECTED(ch->in_room, RAFF_ENCLOSED)) {
            send_to_char("PANIC! There is no way out!\r\n", ch);
            return;
    }


   /* Water's sticky... */
 
    if ( ch->in_room->sector_type == SECT_WATER_SWIM
      || ch->in_room->sector_type == SECT_UNDERWATER ) {
         if (!IS_AFFECTED(ch, AFF_SWIM) ) {
             if (!check_skill(ch, gsn_swim, 0)) {
                  if ( ch->move > 0 ) {
                        ch->move -= 5;
                        if (ch->move < 0) ch->move = 0;
                  }
                  send_to_char("{CYou splash around but don't seem to be getting anywhere.{x\r\n", ch);
                  return;
             }
         }
    }

   /* Are we running from a fight? */

    victim = ch->fighting;

    if ( victim != NULL ) ch->fleeing = TRUE;

   /* Can't run when knackered... */

    if (ch->move == 0) {
      if (IS_FLEEING(ch)) {
          send_to_char("{YYou are to exhausted to flee!{x\r\n", ch); 
          ch->fleeing = FALSE;
      } else {
          send_to_char("You need to sit down and have a rest.{x\r\n", ch);
      }

      return;
    }

   /* Initially we don't know where to go... */

    dest = NULL;
    door = DIR_NONE;

   /* Have we been given a direction? */

    if (argument[0] != '\0') {
      door = door_index(ch, argument);
      if (door > DIR_NONE) dest = can_flee(ch, door);
     
      if ( dest == NULL ) {
        
        if (IS_FLEEING(ch)) {
          send_to_char("{YYou can't flee that way!{x\r\n", ch); 
          ch->fleeing = FALSE;
        } else {
          send_to_char("You can't go that way.\r\n", ch);
        }
  
        return;         
      }
    }   

   /* Is there a wimpy direction set? */

    if ( dest == NULL ) {

        if (ch->wimpy_dir < DIR_NONE) door = -ch->wimpy_dir - 2;
        else door = ch->wimpy_dir; 

        if (door != DIR_NONE) dest = can_flee(ch, door);
    }

   /* Last option is to pick an exit at random... */

    if (dest == NULL) {

      for (count = 0; count < DIR_MAX; count++) {
        tried[count] = FALSE;
      } 

      count = 0;

      while ( dest == NULL
           && count < DIR_MAX) {

        door = number_door( );

        while (tried[door]) {
          door++;
 
          if (door == DIR_MAX) {
            door = DIR_NORTH;
          }
        }

        tried[door] = TRUE; 

        dest = can_flee(ch, door); 

        count += 1;
      }
    } 

   /* Tell the player if there is no escape... */

    if (dest == NULL) {
      if (IS_FLEEING(ch)) {
        send_to_char("{YThere is no where to flee!{x\r\n", ch);
        ch->fleeing = FALSE;
      } else {
        send_to_char("Oh-oh. There aren't any exits...\r\n", ch);
      } 
  
      return;
    }

   /* Find the price... */

    move = 0;

    if ( !IS_NPC(ch) ) {
    
      move = movement_loss[UMIN(SECT_MAX-1, ch->in_room->sector_type)] + movement_loss[UMIN(SECT_MAX-1, dest->sector_type)];
      move /= 2;  /* i.e. the average */

      if ( ch->move < move
        && !IS_FLEEING(ch) ) {
	send_to_char( "You are too exhausted.\r\n", ch );
	return;
      }
    }

   /* Reset activity... */

    if (IS_FLEEING(ch)) {
      set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
    } else {
      switch (ch->activity) {

        case ACV_SNEAKING:
        case ACV_TRACKING:
          break;

        default:
          set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
          break;
      }
    }

   /* Tell them where they are going... */

    if (IS_FLEEING(ch)) {
      sprintf(buf, "{YYou try and flee %s!{x\r\n", dir_name[door]);
    } else {
      sprintf(buf, "{GYou wander %s.{x\r\n", dir_name[door]);
    } 
       
    send_to_char(buf, ch);

   /* Now, do the move... */

   if (IS_FLEEING(ch)
   && (!str_cmp("spec_hunter", spec_string(victim->spec_fun))
         || !str_cmp("spec_tracker", spec_string(victim->spec_fun)))) {
              ch->huntedBy = victim;
              victim->prey = ch;
   }

    make_char_move(ch, door, dest, move);

   /* Ok, move complete... */

    if (IS_FLEEING(ch)) { 
      ch->fleeing = FALSE;
      loss = exp_to_advance(ch->level)/200;
      gain_exp(ch, -1 * loss, FALSE);
    }

    return;
}

/* Stupid auto recruit proc... */

void do_recruit(CHAR_DATA *ch, char *args) {
CHAR_DATA *fch;
CHAR_DATA *fch_next;

    for ( fch = ch->in_room->people; fch != NULL; fch = fch_next )    {
	fch_next = fch->next_in_room;

        if (!can_see( fch, ch )) continue;

       if (IS_SET(fch->act, ACT_PET)
       && fch->leader== NULL
       && !IS_SET(fch->imm_flags,IMM_CHARM)
       && ch->pet == NULL
       && !IS_NPC(ch)) {
              SET_BIT(fch->act, ACT_FOLLOWER);
              SET_BIT(fch->affected_by, AFF_CHARM);
              fch->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
              add_follower( fch, ch );
              fch->leader = ch;
              act( "The stray $N follows you!", fch, NULL, NULL,TO_CHAR );
              act( "The stray $n now follows $N!", fch, NULL, ch, TO_ROOM );
       }

        if ( IS_NPC(fch) && (fch->level < ch->level)
        && (!IS_NPC(ch) )
        && (!IS_SET(fch->act, ACT_SENTINEL) )
        && (!IS_SET(fch->act, ACT_AGGRESSIVE) )
        && (!IS_SET(fch->act, ACT_PET) )
        && (fch->pIndexData->pShop == NULL)
        && (!IS_SET(fch->act, ACT_FOLLOWER) )
        && (!IS_SET(fch->act, ACT_PRACTICE) )
        && (!IS_SET(fch->act, ACT_NOPURGE) )
        && (!IS_SET(fch->act, ACT_IS_ALIENIST) )
        && (!IS_SET(fch->act, ACT_IS_HEALER) )
        && (!IS_AFFECTED(fch, AFF_CHARM) ) )   {
            int pass = 1;
            int chance;
            int skillchance;
            int diff;

	    /* bonus if alignment is within 150 of char */
            diff = fch->alignment - ch->alignment;
            if ((diff <= 150) && (diff >= -150)) pass +=1;

            if ((fch->alignment>250) && (ch->alignment<250)) pass =0;
            skillchance = number_percent();
            chance = number_percent();


             if (pass >= chance && skillchance < get_skill(ch, gsn_recruit) + get_curr_stat(ch, STAT_CHA) - 50) {
	     SET_BIT(fch->act, ACT_FOLLOWER);
        	     SET_BIT(fch->affected_by, AFF_CHARM);
	     fch->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
	     add_follower( fch, ch );
        	     fch->leader = ch;
	     act( "$N has heard of your reputation and starts following you!", fch, NULL, NULL,TO_CHAR );
        	     act( "$n now follows $N!", fch, NULL, ch, TO_ROOM );
	     check_improve(ch, gsn_recruit, TRUE, 1);
             }
         }
    }
    return;
}


/* Locate a door in a given direction... */

int find_door( CHAR_DATA *ch, char *arg ) {

    EXIT_DATA *pexit;
    int door;

   /* Go find the door... */

    door = door_index(ch, arg);

   /* Did we find something? */

    if (door <= DIR_NONE) {
      return door;
    }

   /* ...and is it a door... */

    pexit = ch->in_room->exit[door];

    if ( !IS_SET(pexit->exit_info, EX_ISDOOR) ) {
      send_to_char( "There is not door on that exit.\r\n", ch );
      return -1;
    }

   /* Yep, so return the direction... */

    return door;
}

/* Always polite to knock first... */

void do_knock( CHAR_DATA *ch, char *argument ) {

    char arg[MAX_INPUT_LENGTH];
    int door=-1;

    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

    one_argument( argument, arg );

    if ( arg[0] == '\0' ) {
	send_to_char( "Knock on what?\r\n", ch );
	return;
    }

    door = find_door( ch, arg );

    if ( door >= 0 ) {

	/* 'knock door' */
	EXIT_DATA *pexit;

	pexit = ch->in_room->exit[door];

       /* Build context and wev... */ 

        mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, door, dir_name[door]);

        wev = get_wev(WEV_KNOCK, WEV_KNOCK_DOOR, mcc,
                    "You knock on the @t0 door.\r\n",
                     NULL,
                    "@a2 knocks on the @t0 door.\r\n");

       /* Issue as challange... */ 

        if (!room_issue_wev_challange(ch->in_room, wev)) {
          free_wev(wev);
          return;
        }

       /* Send to command issuers room... */

        room_issue_door_wev(ch->in_room, wev,
                                       "Someone knocks at the @t0 door!\r\n");

       /* Free the wev (will autofree the context)... */

        free_wev(wev);

	return;
    }
    
    act( "There is no $T door here.", ch, NULL, arg, TO_CHAR );

    return;
}


/* Open a door or an object... */

void do_open( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *vch;
    int door=-1;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

    one_argument( argument, arg );

    if (IS_AFFECTED(ch, AFF_MIST)) {
	send_to_char( "You can't do that in mist form?\r\n", ch );
	return;
    }

    if ( arg[0] == '\0' )    {
	send_to_char( "Open what?\r\n", ch );
	return;
    }

    door = find_door( ch, arg );

    if ( door >= 0) { 

	/* 'open door' */
	EXIT_DATA *pexit;

	pexit = ch->in_room->exit[door];

                if (IS_SET(pexit->exit_info, EX_WALL))  {
                    act( "There is no $T here.", ch, NULL, arg, TO_CHAR );
                    return;
                 }

	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already open.\r\n",      ch ); return; }
	if (  IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's locked.\r\n",            ch ); return; }

       /* Build context and wev... */ 

        mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, door, dir_name[door]);
        wev = get_wev(WEV_LOCK, WEV_LOCK_OPEN_DOOR, mcc,
                    "You open the @t0 door.\r\n",
                     NULL,
                    "@a2 opens the @t0 door.\r\n");

       /* Issue as challange... */ 

        if (!room_issue_wev_challange(ch->in_room, wev)) {
          free_wev(wev);
          return;
        }

       /* Go ahead and open the door... */

        set_door_flags(ch->in_room, door, pexit, EX_CLOSED, FALSE);

        if (IS_SET(pexit->exit_info, EX_CREAKY)) {
              for ( vch = char_list; vch != NULL; vch = vch->next )    {
  	    if ( vch->in_room == NULL ) continue;
                    if (IS_NPC(vch)) continue;
                    if ( vch->in_room->area == ch->in_room->area
                    && vch->in_room->subarea == ch->in_room->subarea) {
                            if (ch==vch)  send_to_char("The door emits a loud creaking sound.\r\n", vch);
                            else send_to_char("You hear the creaking of a door.\r\n", vch);
                    }
              }
        }

       /* Send to command issuers room... */

        room_issue_door_wev(ch->in_room, wev, "The @t0 door opens.\r\n");
        free_wev(wev);
        return;
    }
    
    obj = get_obj_here(ch, arg);

    if (obj == NULL
    && IS_SET(ch->in_room->room_flags, ROOM_VEHICLE)) {
          if (ch->in_room->exit[DIR_OUT] != NULL) {
               obj = ch->in_room->exit[DIR_OUT]->u1.to_room->contents;

               while ( obj != NULL
               && ( obj->item_type != ITEM_PORTAL
                       || obj->value[4] != PORTAL_VEHICLE
                       || obj->value[0] != ch->in_room->vnum)) { 
                           obj = obj->next_content;
               }
          }
    }      

    if  (obj != NULL) {
	/* 'open object' */

                 if (obj->item_type == ITEM_PORTAL
                 && obj->value[4] == PORTAL_VEHICLE) { 

      	      if ( !IS_SET(obj->value[3], CONT_CLOSED) ) {
	             send_to_char( "It's not closed.\r\n", ch); 
                             return; 
                      }

      	      if ( IS_SET(obj->value[3], CONT_LOCKED) ) {
	             send_to_char( "It's locked.\r\n", ch); 
                             return; 
                      }

      	       if ( !IS_SET(obj->value[3], CONT_CLOSEABLE) ) {
                               send_to_char( "You can't do that.\r\n", ch );
                               return;
                       }

                      mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);
                      wev = get_wev(WEV_LOCK, WEV_LOCK_OPEN_ITEM, mcc,
                      "You open @p2.\r\n",
                       NULL,
                      "@a2 open @p2.\r\n");

                       if (!room_issue_wev_challange(ch->in_room, wev)) {
                               free_wev(wev);
                               return;
                       }
      
                       REMOVE_BIT(obj->value[3], CONT_CLOSED);
                       room_issue_wev(ch->in_room, wev);
                       free_wev(wev);
                       return;
                }

	if ( (obj->item_type != ITEM_CONTAINER)
               && (obj->item_type != ITEM_CAMERA) 
               && (obj->item_type != ITEM_LOCKER) 
               && (obj->item_type != ITEM_KEY_RING) 
               && (obj->item_type != ITEM_CLAN_LOCKER)) {
                      send_to_char( "That's not a container.\r\n", ch );
                      return;
               }

               if (obj->item_type == ITEM_CAMERA
               && obj->contains != NULL) {
                    if (obj->contains->value[0] == PHOTO_EXPOSING) {
                        send_to_char( "That'd destroy the photo.\r\n", ch );
                        return;
                    }
               }

	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE)) { 
                      send_to_char( "You can't do that.\r\n",      ch );
                      return;
                }

	if ( !IS_SET(obj->value[1], CONT_CLOSED)) {
                      send_to_char( "It's already open.\r\n",      ch );
                      return;
                }

	if ( IS_SET(obj->value[1], CONT_LOCKED)) {
                      send_to_char( "It's locked.\r\n",            ch );
                      return;
                }

                if (obj->item_type == ITEM_CAMERA) {
                    obj->value[4] = 0;
                    if (ch->activity == ACV_PHOTO) set_activity(ch, POS_STANDING, NULL, ACV_NONE, NULL);
                }

       /* Build context and wev... */ 

        mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);
        wev = get_wev(WEV_LOCK, WEV_LOCK_OPEN_ITEM, mcc,
                    "You open @p2.\r\n",
                     NULL,
                    "@a2 opens @p2.\r\n");

       /* Issue as challange... */ 

        if (!room_issue_wev_challange(ch->in_room, wev)) {
          free_wev(wev);
          return;
        }

       /* Ok, go ahead and open it... */
      
        REMOVE_BIT(obj->value[1], CONT_CLOSED);

        room_issue_wev(ch->in_room, wev);
        if (obj->item_type == ITEM_CONTAINER) {
            OBJ_DATA *trap;

            for(trap = obj->contains; trap; trap = trap->next_content) {
                    if (trap->item_type == ITEM_FIGURINE) {
                           if (trap->value[2] == FIGURINE_CONT) figurine_trigger(trap, obj, ch);
                    }
            }
            trap = NULL;
            
            for(trap = obj->contains; trap; trap = trap->next_content) {
                    if (trap->item_type == ITEM_TRAP) break;
            }
            if (trap) trap_trigger(ch, obj);
        }

        free_wev(wev);
        return;
    }

    act( "There is no $T here.", ch, NULL, arg, TO_CHAR );
    return;
}


/* Close a door or an object... */

void do_close( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *vch;
    int door;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

    one_argument( argument, arg );

    if (IS_AFFECTED(ch, AFF_MIST)) {
	send_to_char( "You can't do that in mist form?\r\n", ch );
	return;
    }

    if ( arg[0] == '\0' )    {
	send_to_char( "Close what?\r\n", ch );
	return;
    }

    door = find_door( ch, arg );

    if ( door >= 0) {
	/* 'close door' */
	EXIT_DATA *pexit;
	pexit	= ch->in_room->exit[door];
                if (IS_SET(pexit->exit_info, EX_WALL))  {
                    act( "There is no $T here.", ch, NULL, arg, TO_CHAR );
                    return;
                 }

	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already closed.\r\n",    ch ); return; }

       /* Build context and wev... */ 

        mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, door, dir_name[door]);

        wev = get_wev(WEV_LOCK, WEV_LOCK_CLOSE_DOOR, mcc,
                    "You close the @t0 door.\r\n",
                     NULL,
                    "@a2 closes the @t0 door.\r\n");

       /* Issue as challange... */ 

        if (!room_issue_wev_challange(ch->in_room, wev)) {
          free_wev(wev);
          return;
        }

       /* Go ahead and lock the door... */

        set_door_flags(ch->in_room, door, pexit, EX_CLOSED, TRUE);
        if (IS_SET(pexit->exit_info, EX_CREAKY)) {
              for ( vch = char_list; vch != NULL; vch = vch->next )    {
  	    if ( vch->in_room == NULL ) continue;
                    if (IS_NPC(vch)) continue;
                    if ( vch->in_room->area == ch->in_room->area
                    && vch->in_room->subarea == ch->in_room->subarea) {
                            if (ch==vch)  send_to_char("The door emits a loud creaking sound.\r\n", vch);
                            else send_to_char("You hear the creaking of a door.\r\n", vch);
                    }
              }
        }


       /* Send to command issuers room... */

        room_issue_door_wev(ch->in_room, wev, "The @t0 door closes.\r\n");

       /* Free the wev (will autofree the context)... */

        free_wev(wev);

	return;
    }

    obj = get_obj_here( ch, arg );

    if (obj == NULL
    && IS_SET(ch->in_room->room_flags, ROOM_VEHICLE)) {
          if (ch->in_room->exit[DIR_OUT] != NULL) {
               obj = ch->in_room->exit[DIR_OUT]->u1.to_room->contents;

               while ( obj != NULL
               && ( obj->item_type != ITEM_PORTAL
                       || obj->value[4] != PORTAL_VEHICLE
                       || obj->value[0] != ch->in_room->vnum)) { 
                           obj = obj->next_content;
               }
          }
    }             

    if ( obj != NULL ) {
	/* 'close object' */

                 if (obj->item_type == ITEM_PORTAL
                 && obj->value[4] == PORTAL_VEHICLE) { 

      	      if ( IS_SET(obj->value[3], CONT_CLOSED) ) {
	             send_to_char( "It's already closed.\r\n", ch); 
                             return; 
                      }

      	       if ( !IS_SET(obj->value[3], CONT_CLOSEABLE) ) {
                               send_to_char( "You can't do that.\r\n", ch );
                               return;
                       }

                      mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);
                      wev = get_wev(WEV_LOCK, WEV_LOCK_CLOSE_ITEM, mcc,
                      "You close @p2.\r\n",
                       NULL,
                      "@a2 close @p2.\r\n");

                       if (!room_issue_wev_challange(ch->in_room, wev)) {
                               free_wev(wev);
                               return;
                       }
      
                       SET_BIT(obj->value[3], CONT_CLOSED);
                       room_issue_wev(ch->in_room, wev);
                       free_wev(wev);
                       return;
                }

	if ( (obj->item_type != ITEM_CONTAINER)
               && (obj->item_type != ITEM_CAMERA) 
               && (obj->item_type != ITEM_KEY_RING) 
               && (obj->item_type != ITEM_LOCKER) 
               && (obj->item_type != ITEM_CLAN_LOCKER)) {
                      send_to_char( "That's not a container.\r\n", ch );
                      return;
               }

	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\r\n",      ch ); return; }

	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already closed.\r\n",    ch ); return; }

       /* Build context and wev... */ 

        mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);

        wev = get_wev(WEV_LOCK, WEV_LOCK_CLOSE_ITEM, mcc,
                    "You close @p2.\r\n",
                     NULL,
                    "@a2 closes @p2.\r\n");

       /* Issue as challange... */ 

        if (!room_issue_wev_challange(ch->in_room, wev)) {
          free_wev(wev);
          return;
        }

       /* Ok, go ahead and open it... */
      
	SET_BIT(obj->value[1], CONT_CLOSED);

       /* Send to command issuers room... */

        room_issue_wev(ch->in_room, wev);

       /* Free the wev (will autofree the context)... */

        free_wev(wev);

	return;
   	}

    act( "There is no $T here.", ch, NULL, arg, TO_CHAR );
    return;
}



bool has_key( CHAR_DATA *ch, int key ) {
    OBJ_DATA *obj, *obj2;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )   {
	if (obj->pIndexData->vnum == key)  {
                   if (obj->item_type == ITEM_KEY) {
                         if (obj->value[0] > 0) {
                                if (--obj->value[0] == 0) {
                                      act("$p crumbles to dust.", ch, obj, NULL, TO_CHAR );
                                      extract_obj(obj);
                                }
                         }
                    }
                    return TRUE;
                }
        if (obj->item_type == ITEM_KEY_RING) {
            for(obj2 = obj->contains; obj2 != NULL; obj2 = obj2->next_content) {
                 if (obj2->pIndexData->vnum == key) {
                   if (obj2->item_type == ITEM_KEY) {
                         if (obj2->value[0] > 0) {
                                if (--obj2->value[0] == 0) {
                                      act("$p crumbles to dust.", ch, obj2, NULL, TO_CHAR );
                                      extract_obj(obj2);
                                }
                         }
                    }
                    return TRUE;
                 }
            }
        }
    }

    /* Nothing found - Now check for Master Key */

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )   {
	if (obj->pIndexData->vnum == OBJ_VNUM_MASTER_KEY)  {
                   if (obj->item_type == ITEM_KEY) {
                         if (obj->value[0] > 0) {
                                if (--obj->value[0] == 0) {
                                      act("$p crumbles to dust.", ch, obj, NULL, TO_CHAR );
                                      extract_obj(obj);
                                }
                         }
                    }
                    send_to_char("{mYou use your master key.\r\n", ch);
                    return TRUE;
                }
        if (obj->item_type == ITEM_KEY_RING) {
            for(obj2 = obj->contains; obj2 != NULL; obj2 = obj2->next_content) {
                 if (obj2->pIndexData->vnum == OBJ_VNUM_MASTER_KEY) {
                   if (obj2->item_type == ITEM_KEY) {
                         if (obj2->value[0] > 0) {
                                if (--obj2->value[0] == 0) {
                                      act("$p crumbles to dust.", ch, obj2, NULL, TO_CHAR );
                                      extract_obj(obj2);
                                }
                         }
                    }
                    send_to_char("{mYou use your master key.\r\n", ch);
                    return TRUE;
                 }
            }
        }
    }

    return FALSE;
}


bool has_locker_key( CHAR_DATA *ch ) {
    OBJ_DATA *obj, *obj2;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )    {
	if (obj->item_type == ITEM_LOCKER_KEY)  return TRUE;

        if (obj->item_type == ITEM_KEY_RING ) { 

          for(obj2 = obj->contains; obj2 != NULL; obj2 = obj2->next) {
          
            if (obj2->item_type == ITEM_LOCKER_KEY) return TRUE;
          }
        }
    }

    return FALSE;
}

/* Lock a door or an object with a key... */

void do_lock( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;
    char *lastarg;

    one_argument( argument, arg );

    if (IS_AFFECTED(ch, AFF_MIST)) {
	send_to_char( "You can't do that in mist form?\r\n", ch );
	return;
    }

    if ( arg[0] == '\0' )    {
	send_to_char( "Lock what?\r\n", ch );
	return;
    }

    door = find_door( ch, arg );

    if ( door >= 0) {
    
	/* 'lock door' */
	EXIT_DATA *pexit;

	pexit	= ch->in_room->exit[door];

                if (IS_SET(pexit->exit_info, EX_WALL))  {
                    act( "There is no $T here.", ch, NULL, arg, TO_CHAR );
                    return;
                 }

	if ( !IS_SET(pexit->exit_info, EX_CLOSED)) {
                     do_close(ch, argument);
   	     if ( !IS_SET(pexit->exit_info, EX_CLOSED)) {
                          send_to_char( "It's not closed.\r\n", ch);
                          return;
                     }
                }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be locked.\r\n",     ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\r\n",       ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already locked.\r\n",    ch ); return; }

       /* Build context and wev... */ 

        mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, door, dir_name[door]);

        wev = get_wev(WEV_LOCK, WEV_LOCK_LOCK_DOOR, mcc,
                    "You lock the @t0 door.\r\n",
                     NULL,
                    "@a2 locks the @t0 door.\r\n");

       /* Issue as challange... */ 

        if (!room_issue_wev_challange(ch->in_room, wev)) {
          free_wev(wev);
          return;
        }

       /* Go ahead and lock the door... */

        set_door_flags(ch->in_room, door, pexit, EX_LOCKED, TRUE);

       /* Send to command issuers room... */

        room_issue_door_wev(ch->in_room, wev, "The @t0 door locks.\r\n");

       /* Free the wev (will autofree the context)... */

        free_wev(wev);

        return;
    }

    obj = get_obj_here(ch, arg);

    if (obj == NULL
    && IS_SET(ch->in_room->room_flags, ROOM_VEHICLE)) {
          if (ch->in_room->exit[DIR_OUT] != NULL) {
               obj = ch->in_room->exit[DIR_OUT]->u1.to_room->contents;

               while ( obj != NULL
               && ( obj->item_type != ITEM_PORTAL
                       || obj->value[4] != PORTAL_VEHICLE
                       || obj->value[0] != ch->in_room->vnum)) { 
                           obj = obj->next_content;
               }
          }
    }      

    if (obj != NULL )    {
	/* 'lock object' */

                 if (obj->item_type == ITEM_PORTAL
                 && obj->value[4] == PORTAL_VEHICLE) { 

      	      if ( !IS_SET(obj->value[3], CONT_CLOSED) ) {
                          do_close(ch, argument);
        	          if ( !IS_SET(obj->value[3], CONT_CLOSED) ) {
                             send_to_char( "It's not closed.\r\n", ch);
                             return;
                          }
                      }

      	      if (IS_SET(obj->value[3], CONT_LOCKED) ) {
	             send_to_char( "It's already locked.\r\n", ch); 
                             return; 
                      }

                      lastarg = check_obj_owner(obj);
                      if (str_cmp(lastarg, "not found")
                      && str_cmp(lastarg, ch->name)) {
                               act("You can't lock $p!",ch, obj, NULL, TO_CHAR);
                               act("$n fumbles to lock $p!",ch, obj, NULL, TO_ROOM);
                               return;
                      }      

                      mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);
                      wev = get_wev(WEV_LOCK, WEV_LOCK_LOCK_ITEM, mcc,
                      "You lock @p2.\r\n",
                       NULL,
                      "@a2 locks @p2.\r\n");

                       if (!room_issue_wev_challange(ch->in_room, wev)) {
                               free_wev(wev);
                               return;
                       }
      
                       SET_BIT(obj->value[3], CONT_LOCKED);
                       room_issue_wev(ch->in_room, wev);
                       free_wev(wev);
                       return;
                }

	if ( (obj->item_type != ITEM_CONTAINER)
               && (obj->item_type != ITEM_KEY_RING)
               && (obj->item_type != ITEM_LOCKER)
               && (obj->item_type != ITEM_CLAN_LOCKER)) {
	  send_to_char("That's not a container.\r\n", ch); 
                  return; 
               }

      	      if ( !IS_SET(obj->value[1], CONT_CLOSED) ) {
                          do_close(ch, argument);
        	          if ( !IS_SET(obj->value[1], CONT_CLOSED) ) {
                             send_to_char( "It's not closed.\r\n", ch);
                             return;
                          }
                      }

	if ( obj->value[2] < 0 ) {
	  send_to_char( "It can't be locked.\r\n", ch); 
                  return; 
                }

        if ( obj->item_type == ITEM_CONTAINER
          || obj->item_type == ITEM_KEY_RING) {
          if ( !has_key( ch, obj->value[2] ) ) {
	    send_to_char( "You lack the key.\r\n", ch); 
            return; 
          }
        } else {
          if ( !has_locker_key( ch ) ) {
	    send_to_char( "You do not have a key.\r\n", ch); 
            return; 
          }
        }

	if ( IS_SET(obj->value[1], CONT_LOCKED) ) {
	  send_to_char( "It's already locked.\r\n", ch); 
                  return; 
                }

       /* Build context and wev... */ 

        mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);

        wev = get_wev(WEV_LOCK, WEV_LOCK_LOCK_ITEM, mcc,
                    "You lock @p2.\r\n",
                     NULL,
                    "@a2 locks @p2.\r\n");

       /* Issue as challange... */ 

        if (!room_issue_wev_challange(ch->in_room, wev)) {
          free_wev(wev);
          return;
        }

       /* Ok, go ahead and open it... */
      
	SET_BIT(obj->value[1], CONT_LOCKED);

       /* Send to command issuers room... */

        room_issue_wev(ch->in_room, wev);

       /* Free the wev (will autofree the context)... */

        free_wev(wev);

       /* If we're a locker, it needs saving and closing... */ 

        if ( ( (obj->item_type == ITEM_LOCKER)
            || (obj->item_type == ITEM_CLAN_LOCKER))
          && (obj->owner != NULL)) {

          save_locker(obj->owner, obj, TRUE);

          remove_open_locker(obj);

          free_string(obj->owner);

          obj->owner = NULL;

          send_to_char("{YLocker locked.{x\r\n", ch);
          send_to_char("{WLocker contents saved to disk.{x\r\n", ch);
          return;
        }            
    }

    act( "There is no $T here.", ch, NULL, arg, TO_CHAR );
    return;
}

/* Unlock a door or an object... */

void do_unlock( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;
    char buf[MAX_STRING_LENGTH]; 
    SOCIETY *soc;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;
    char *lastarg;

    one_argument( argument, arg );

    if (IS_AFFECTED(ch, AFF_MIST)) {
	send_to_char( "You can't do that in mist form?\r\n", ch );
	return;
    }

    if ( arg[0] == '\0' )    {
	send_to_char( "Unlock what?\r\n", ch );
	return;
    }

    door = find_door( ch, arg );

    if ( door >= 0) {
    
	/* 'unlock door' */
	EXIT_DATA *pexit;

	pexit = ch->in_room->exit[door];

                if (IS_SET(pexit->exit_info, EX_WALL))  {
                    act( "There is no $T here.", ch, NULL, arg, TO_CHAR );
                    return;
                 }

	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\r\n", ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be unlocked.\r\n",   ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\r\n",       ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\r\n",  ch ); return; }

       /* Build context and wev... */ 

        mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, door, dir_name[door]);

        wev = get_wev(WEV_LOCK, WEV_LOCK_UNLOCK_DOOR, mcc,
                    "You unlock the @t0 door.\r\n",
                     NULL,
                    "@a2 unlocks the @t0 door.\r\n");

       /* Issue as challange... */ 

        if (!room_issue_wev_challange(ch->in_room, wev)) {
          free_wev(wev);
          return;
        }

       /* Ok, go ahead and do it... */

        set_door_flags(ch->in_room, door, pexit, EX_LOCKED, FALSE);

       /* Send to command issuers room... */

        room_issue_door_wev(ch->in_room, wev, "The @t0 door unlocks.\r\n");

       /* Free the wev (will autofree the context)... */

        free_wev(wev);

        return;
    }

     obj = get_obj_here(ch, arg);

    if (obj == NULL
    && IS_SET(ch->in_room->room_flags, ROOM_VEHICLE)) {
          if (ch->in_room->exit[DIR_OUT] != NULL) {
               obj = ch->in_room->exit[DIR_OUT]->u1.to_room->contents;

               while ( obj != NULL
               && ( obj->item_type != ITEM_PORTAL
                       || obj->value[4] != PORTAL_VEHICLE
                       || obj->value[0] != ch->in_room->vnum)) { 
                           obj = obj->next_content;
               }
          }
    }      

    if (obj != NULL )    {
	/* 'unlock object' */
                 if (obj->item_type == ITEM_PORTAL
                 && obj->value[4] == PORTAL_VEHICLE) { 

      	      if ( !IS_SET(obj->value[3], CONT_CLOSED) ) {
	             send_to_char( "It's not closed.\r\n", ch); 
                             return; 
                      }

      	      if ( !IS_SET(obj->value[3], CONT_LOCKED) ) {
	             send_to_char( "It's already unlocked.\r\n", ch); 
                             return; 
                      }

                      lastarg = check_obj_owner(obj);

                      bug (lastarg, 0);

                      if (str_cmp(lastarg, "not found")
                      && str_cmp(lastarg, ch->name)) {
                               act("You can't unlock $p!",ch, obj, NULL, TO_CHAR);
                               act("$n fumbles to unlock $p!",ch, obj, NULL, TO_ROOM);
                               return;
                      }      

                      mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);
                      wev = get_wev(WEV_LOCK, WEV_LOCK_UNLOCK_ITEM, mcc,
                      "You unlock @p2.\r\n",
                       NULL,
                      "@a2 unlocks @p2.\r\n");

                       if (!room_issue_wev_challange(ch->in_room, wev)) {
                               free_wev(wev);
                               return;
                       }
      
                       REMOVE_BIT(obj->value[3], CONT_LOCKED);
                       room_issue_wev(ch->in_room, wev);
                       free_wev(wev);
                       return;
                }

	if ( (obj->item_type != ITEM_CONTAINER)
	  && (obj->item_type != ITEM_KEY_RING)
                  && (obj->item_type != ITEM_LOCKER)
                  && (obj->item_type != ITEM_CLAN_LOCKER)) {
	        send_to_char( "That's not a container.\r\n", ch); 
                        return; 
                  }

	if ( !IS_SET(obj->value[1], CONT_CLOSED) ) {
	  send_to_char( "It's not closed.\r\n", ch); 
                  return; 
                }

	if ( obj->value[2] < 0 ) {
	  send_to_char( "It can't be unlocked.\r\n", ch); 
                  return; 
                }

        if ( obj->item_type == ITEM_CONTAINER
          || obj->item_type == ITEM_KEY_RING ) {
          if ( !has_key( ch, obj->value[2] ) ) {
	    send_to_char( "You lack the key.\r\n", ch); 
            return; 
          }
        } else {
          if ( !has_locker_key( ch ) ) {
	    send_to_char( "You do not have a key.\r\n", ch); 
            return; 
          }
        }

	if ( !IS_SET(obj->value[1], CONT_LOCKED) ) {
	  send_to_char( "It's already unlocked.\r\n", ch); 
          return; 
        }

       /* If it's a container, we just have to open it and we're done... */ 

        if ( obj->item_type == ITEM_CONTAINER
          || obj->item_type == ITEM_KEY_RING ) {
         /* Build context and wev... */ 

          mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);
          wev = get_wev(WEV_LOCK, WEV_LOCK_UNLOCK_ITEM, mcc,
                      "You unlock @p2.\r\n",
                       NULL,
                      "@a2 unlocks @p2.\r\n");

         /* Issue as challange... */ 

          if (!room_issue_wev_challange(ch->in_room, wev)) {
            free_wev(wev);
            return;
          }

         /* Ok, go ahead and open it... */
      
          REMOVE_BIT(obj->value[1], CONT_LOCKED);

          room_issue_wev(ch->in_room, wev);
          free_wev(wev);

          return;
        }

       /* NPCs do not have lockers... */

        if (IS_NPC(ch)) {
          send_to_char("Lockers are only for PCs.", ch);
          act("$n can't unlock $p", ch, obj, NULL, TO_ROOM);
          return;
        }

       /* Work out the locker name... */

        if (obj->item_type == ITEM_LOCKER) sprintf(buf, "%s", ch->name);

        if (obj->item_type == ITEM_CLAN_LOCKER) {

          soc = find_society_by_id(obj->value[4]);

          if ( soc == NULL
            || ! is_member(ch, soc) ) { 
              send_to_char("You can't open this locker.",ch);
              act("$n struggles to unlock $p.", ch, obj, NULL, TO_ROOM);
              return;
          }
          sprintf(buf, "society.%ld", obj->value[4]);
        }

       /* Lockers can only be open once... */

        if (is_locker_open(buf)) {
          send_to_char("The key won't turn!",ch);
          act("$n struggles to unlock $p.", ch, obj, NULL, TO_ROOM);
          return;
        }
 
       /* Build context and wev... */ 

        mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);
        wev = get_wev(WEV_LOCK, WEV_LOCK_UNLOCK_ITEM, mcc,
                    "You unlock @p2.\r\n",
                     NULL,
                    "@a2 unlocks @p2.\r\n");

       /* Issue as challange... */ 

        if (!room_issue_wev_challange(ch->in_room, wev)) {
          free_wev(wev);
          return;
        }

       /* Ok, go ahead and open it... */
      
        REMOVE_BIT(obj->value[1], CONT_LOCKED);

       /* Send to command issuers room... */

        room_issue_wev(ch->in_room, wev);

       /* Free the wev (will autofree the context)... */

        free_wev(wev);

       /* Load the locker and mark it as open... */ 
    
        obj->owner = str_dup(buf);

        load_locker(obj->owner, obj);

        add_open_locker(buf, obj);            

        send_to_char("{YLocker unlocked.{x\r\n", ch);
        send_to_char("{WPlease lock it when you are finished.{x\r\n", ch);  
        return;
    }

    act( "There is no $T here.", ch, NULL, arg, TO_CHAR );
    return;
}

/* Pick the lock on a door or an object... */

#define ACV_PICK_DOOR		1
#define ACV_PICK_OBJECT		2

void do_pick( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door,mid,lockdif;

    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

    EXIT_DATA *pexit;

    char buf[MAX_STRING_LENGTH];

    obj = NULL;

   /* Check we have some parms... */

    one_argument( argument, arg );

    if (IS_AFFECTED(ch, AFF_MIST)) {
	send_to_char( "You can't do that in mist form?\r\n", ch );
	return;
    }

    if ( arg[0] == '\0' ) {
	send_to_char( "Pick what?\r\n", ch );
	return;
    }

   /* Retrieve key from command... */

    if (!check_activity_key(ch, argument)) return;
    
   /* If already picking and not a recall, complain... */

    if ( ch->activity == ACV_PICKING
      && argument[0] != '*' ) {
      send_to_char("You are already picking a lock!\r\n", ch);
      return;
    }

   /* Avoid problems switching activities... */

    if (ch->activity != ACV_PICKING) ch->acv_state = 0;
    
   /* Switch to approrpiate routine... */

    switch ( ch->acv_state ) {

     /* Initial parameter parsing and set up... */

      default:

       /* Ok, now go see what we've got to open... */
        door = find_door( ch, arg );

        if ( door >= 0 ) {

         /* Found a door... */
	  pexit = ch->in_room->exit[door];

          if ( !IS_SET(pexit->exit_info, EX_CLOSED) 
          && !IS_IMMORTAL(ch)) {
            send_to_char( "It's not closed.\r\n", ch);
            return;
          }

          if ( !IS_SET(pexit->exit_info, EX_LOCKED) ) {
            send_to_char( "It's already unlocked.\r\n", ch);
            return;
          }

         /* Build context and wev... */ 

          mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, door, dir_name[door]);

          wev = get_wev(WEV_LOCK, WEV_LOCK_PICK_DOOR, mcc,
                      "You start picking the lock on the @t0 door.\r\n",
                       NULL,
                      "@a2 starts picking the lock on the @t0 door.\r\n");

         /* Set up process parms... */
       
          sprintf(buf, "picking the lock on the %s door.", dir_name[door]);

        } else {

          obj = get_obj_here( ch, arg );
 
          if ( obj == NULL ) {
            act( "There is no $T here.", ch, NULL, arg, TO_CHAR );
            return;
          } 

         /* Found an object... */
    
          if ( (obj->item_type != ITEM_CONTAINER)
            && (obj->item_type != ITEM_KEY_RING)
            && (obj->item_type != ITEM_LOCKER) 
            && (obj->item_type != ITEM_CLAN_LOCKER)) { 
                  send_to_char( "That's not a container.\r\n", ch ); 
                  return;
          }

          if ( !IS_SET(obj->value[1], CONT_CLOSED) ) { 
            send_to_char( "It's not closed.\r\n", ch );
            return;
          }

          if ( !IS_SET(obj->value[1], CONT_LOCKED) ) { 
                  send_to_char( "It's already unlocked.\r\n", ch );
                  return;
          }

         /* Build context and wev... */ 

          mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);
          wev = get_wev(WEV_LOCK, WEV_LOCK_PICK_ITEM, mcc,
                      "You start picking the lock on @p2.\r\n",
                       NULL,
                      "@a2 starts picking the lock on @p2.\r\n");

         /* Set up process parms... */
       
          sprintf(buf, "picking the lock on %s.", obj->short_descr);

        }

       /* Challange... */

        if (!room_issue_wev_challange(ch->in_room, wev)) {
          free_wev(wev);
          clear_activity(ch);
          return;
        }

       /* Send to command issuers room... */

//      room_issue_wev( ch->in_room, wev);

        set_activity( ch, ch->position, NULL, ACV_PICKING, buf);

       /* Free the wev (will autofree the context)... */

        free_wev(wev);

       /* Set up the activity state engine... */

        set_activity_key(ch); 
 
        if (obj != NULL) {
          ch->acv_state = ACV_PICK_OBJECT;
          free_string(ch->acv_text);
          ch->acv_text = str_dup(obj->name);
        } else {
          ch->acv_state = ACV_PICK_DOOR;
          ch->acv_int = door;
        } 

       /* Schedule the callback... */

        schedule_activity( ch, 1 + number_bits(2), "Pick" );

        break;


     /* Picking the lock on a door... */

      case ACV_PICK_DOOR:

        door = ch->acv_int;

        if ( door < 0
          || door > DIR_MAX ) {
          send_to_char("The door has vanished!\r\n", ch);
          set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
          return;
        }

	pexit = ch->in_room->exit[door];

        if ( !exit_visible(ch, pexit)) {
          send_to_char("The door has vanished!\r\n", ch);
          set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
          return;
        }

	if ( !IS_SET(pexit->exit_info, EX_CLOSED) 
          && !IS_IMMORTAL(ch)) {
	  send_to_char( "It is no longer closed!\r\n", ch);
          set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
          return;
        }

	if ( !IS_SET(pexit->exit_info, EX_LOCKED) ) {
	  send_to_char( "It is no longer locked!\r\n", ch);
          set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
          return;
        }

          if (IS_SET(pexit->exit_info, EX_TRICKY)) lockdif = 50;
          else lockdif = 0;

          if (  pexit->key > 0
          && !IS_SET(pexit->exit_info, EX_PICKPROOF)
          &&  check_skill(ch, gsn_pick_lock, lockdif) ) {

         /* Build context and wev... */ 

          mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, door, dir_name[door]);
          wev = get_wev(WEV_LOCK, WEV_LOCK_PICK_DOOR, mcc,
                      "You pick the lock on the @t0 door.\r\n",
                       NULL,
                      "@a2 picks the lock on the @t0 door.\r\n");

         /* Ok, go ahead and make the change... */

          set_door_flags(ch->in_room, door, pexit, EX_LOCKED, FALSE);

          check_improve(ch,gsn_pick_lock,TRUE,2);

         /* Send to command issuers room... */

          room_issue_door_wev(ch->in_room, wev, "The @t0 door unlocks.\r\n");

         /* Free the wev (will autofree the context)... */

          free_wev(wev);

          set_activity(ch, ch->position, NULL, ACV_NONE, NULL);

          return; 
        }

       /* Message to the player... */ 
 
        mid = number_bits(2);

        if ( pexit->key <= 0
          || IS_SET( pexit->exit_info, EX_PICKPROOF ) ) {
          mid += 2;
        }
    
        switch (mid) {

          case 5:
            send_to_char("...this lock seems impossible to pick...\r\n", ch);
            break;
           
          case 4:
            send_to_char("...incredibly tricky lock...\r\n", ch);
            break;
           
          case 3:
          case 2:
            send_to_char("...very tricky lock...\r\n", ch);
            break;
           
          default:  
            send_to_char("...tricky lock...\r\n", ch);
            break;

        }

       /* Schedule the callback... */

        schedule_activity( ch, 1 + number_bits(2), "Pick" );

        break;


     /* Picking the lock on an object... */ 

      case ACV_PICK_OBJECT: 

        if ( ch->acv_text == NULL ) {
          send_to_char( "You can no longer find it!\r\n", ch );
          set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
          return;
        }

        obj = get_obj_here( ch, ch->acv_text );

        if ( obj == NULL ) {
          send_to_char( "You can no longer find it!\r\n", ch );
          set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
          return;
        }

	if ( !IS_SET(obj->value[1], CONT_CLOSED) ) { 
          send_to_char( "It is no longer closed!\r\n", ch );
          set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
          return;
        }


	if ( !IS_SET(obj->value[1], CONT_LOCKED) ) { 
          send_to_char( "It is no longer locked!\r\n", ch );
          set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
          return;
        }

	if (  obj->value[2] > 0  
	  && !IS_SET(obj->value[1], CONT_PICKPROOF)
          &&  check_skill(ch, gsn_pick_lock, 0) ) {

         /* Build context and wev... */ 

          mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);

          wev = get_wev(WEV_LOCK, WEV_LOCK_PICK_ITEM, mcc,
                      "You pick the lock on @p2.\r\n",
                       NULL,
                      "@a2 picks the lock on @p2.\r\n");

         /* Make the change... */
 
          REMOVE_BIT(obj->value[1], CONT_LOCKED);
  	
          check_improve(ch,gsn_pick_lock,TRUE,2);

         /* Send to command issuers room... */

          room_issue_wev(ch->in_room, wev);

         /* Free the wev (will autofree the context)... */

          free_wev(wev);
     
          set_activity(ch, ch->position, NULL, ACV_NONE, NULL);

          return; 
        }

       /* Message to the player... */ 
 
        mid = number_bits(2);

        if ( obj->value[2] <= 0
          || IS_SET( obj->value[1], CONT_PICKPROOF ) ) {
          mid += 2;
        }
    
        switch (mid) {

          case 5:
            send_to_char("...this lock seems impossible to pick...\r\n", ch);
            break;
           
          case 4:
            send_to_char("...incredibly tricky lock...\r\n", ch);
            break;
           
          case 3:
          case 2:
            send_to_char("...very tricky lock...\r\n", ch);
            break;
           
          default:  
            send_to_char("...tricky lock...\r\n", ch);
            break;

        }

       /* Schedule the callback... */

        schedule_activity( ch, 1 + number_bits(2), "Pick" );

        break;

    }

    return;
}

void do_stand( CHAR_DATA *ch, char *argument ) {
    char *new_pos_desc;
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (IS_AFFECTED(ch, AFF_MORF)) { 
          send_to_char( "You can't do that!\r\n", ch );
          return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) ) {
        send_to_char( "You are asleep and cannot wake up!\r\n", ch );
        return; 
    }

    if (IS_AFFECTED(ch, AFF_MIST)) {
         affect_strip_afn( ch, gafn_mist);
         REMOVE_BIT( ch->affected_by, AFF_MIST);
    }

    new_pos_desc = NULL;
    
    if ( argument[0] != '\0' ) {
      sprintf( buf, "standing %s", argument);
      new_pos_desc = buf;
    }

     if (!IS_NPC(ch)) { 
        if (ch->pcdata->mounted==TRUE) {
            ch->pcdata->mounted = FALSE;
            send_to_char( "You get down from your mount.\r\n", ch );
            if (ch->pet != NULL) {
                 ch->pet->ridden = FALSE;
                 act("$n gets down from the $N",ch,NULL,ch->pet,TO_ROOM);
            }
         }
     }

    cancel_acv(ch);

    switch ( ch->position ) {

      case POS_FIGHTING:
      case POS_SLEEPING:
      case POS_RESTING:
      case POS_STANDING:
      case POS_SITTING:
        if (ch->on_furniture != NULL) remove_furnaff(ch, ch->on_furniture);
        ch->on_furniture = NULL;
        set_activity( ch, POS_STANDING, new_pos_desc, ACV_NONE, NULL);
        for ( d = descriptor_list; d != NULL; d = d->next ) {
            if ( d->snoop_by == ch->desc ) {
                    d->snoop_by = NULL;
                    send_to_char( "You lose your contact.\r\n", ch );
            }
        }
        break;

      default:
        send_to_char( "You are to stunned and injured for that!\r\n", ch);
        break;
    }

     return;
}



void do_rest( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    char *new_pos_desc;
    char buf[MAX_STRING_LENGTH];

    if (IS_SET(ch->in_room->room_flags, ROOM_RENT)
    && (ch->in_room->rented_by == NULL
        ||str_cmp(ch->in_room->rented_by, ch->short_descr))) {
                  send_to_char ("You are not allowed to rest here!\r\n",ch);
                  return;
    }

    if (IS_AFFECTED(ch, AFF_SLEEP)) {
        send_to_char ("You are asleep and cannot wake up!\r\n",ch);
        return;
    }

   if (!IS_NPC(ch)) {
       if (ch->pcdata->mounted==TRUE) {
           send_to_char( "You can't rest in the saddle!\r\n", ch );
           return;
       }
   }

    if (IS_AFFECTED(ch, AFF_MIST)) {
         affect_strip_afn( ch, gafn_mist);
         REMOVE_BIT( ch->affected_by, AFF_MIST);
    }

    new_pos_desc = NULL;

    if ( argument[0] != '\0' ) {
         argument = one_argument( argument, arg);
         obj = get_obj_list( ch, arg, ch->in_room->contents );
          if ( obj == NULL )    {
               act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return;
          }
          if (!obj->item_type == ITEM_FURNITURE
          || !IS_SET(obj->value[0], FURN_REST)) {
             send_to_char( "You can't rest on that!\r\n", ch );
             return;
          }

        if (obj->condition < 5) {
             send_to_char( "It's much too damaged.\r\n", ch );
             return;
        }

          ch->on_furniture = obj;
          set_furnaff(ch, obj);
          sprintf( buf, "resting on %s", obj->short_descr);
          new_pos_desc = buf;
    }

    cancel_acv(ch);

    switch ( ch->position ) {

      case POS_FIGHTING:
	send_to_char( "You cannot rest when you are fighting!\r\n", ch );
	break;

      case POS_SLEEPING:
      case POS_RESTING:
      case POS_STANDING:
      case POS_SITTING:
        set_activity( ch, POS_RESTING, new_pos_desc, ACV_NONE, NULL);
        break;

      default:
        send_to_char( "You are to stunned and injured for that!\r\n", ch);
        break;
    }

    return;
}


void do_sit (CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    char *new_pos_desc;
    char buf[MAX_STRING_LENGTH];

    if (IS_SET(ch->in_room->room_flags, ROOM_RENT)
    && (ch->in_room->rented_by == NULL
       ||   str_cmp(ch->in_room->rented_by, ch->short_descr))) {
                  send_to_char ("You are not allowed to sit here!\r\n",ch);
                  return;
    }

    if (IS_AFFECTED(ch, AFF_SLEEP)) {
        send_to_char ("You are asleep and cannot wake up!\r\n",ch);
        return;
    }

   if (!IS_NPC(ch)) {
       if (ch->pcdata->mounted==TRUE) {
           send_to_char( "You you are already sitting!\r\n", ch );
           return;
       }
   }

    if (IS_AFFECTED(ch, AFF_MIST)) {
         affect_strip_afn( ch, gafn_mist);
         REMOVE_BIT( ch->affected_by, AFF_MIST);
    }

    new_pos_desc = NULL;

    if ( argument[0] != '\0' ) {
         argument = one_argument( argument, arg);
         obj = get_obj_list( ch, arg, ch->in_room->contents );
          if ( obj == NULL )    {
               act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return;
          }
          if (!obj->item_type == ITEM_FURNITURE
          || !IS_SET(obj->value[0], FURN_SIT)) {
             send_to_char( "You can't sit on that!\r\n", ch );
             return;
          }

        if (obj->condition < 5) {
             send_to_char( "It's much too damaged.\r\n", ch );
             return;
        }

          ch->on_furniture = obj;
          set_furnaff(ch, obj);
          sprintf( buf, "sitting on %s", obj->short_descr);
          new_pos_desc = buf;
    }

    cancel_acv(ch);

    switch ( ch->position ) {

      case POS_FIGHTING:
	send_to_char( "You cannot sit when you are fighting!\r\n", ch );
	break;

      case POS_SLEEPING:
      case POS_RESTING:
      case POS_STANDING:
      case POS_SITTING:
        set_activity( ch, POS_SITTING, new_pos_desc, ACV_NONE, NULL);
        break;

      default:
        send_to_char( "You are to stunned and injured for that!\r\n", ch);
        break;
    }

    return;
}


void do_sleep( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    char *new_pos_desc;
    char buf[MAX_STRING_LENGTH];

    if (IS_SET(ch->in_room->room_flags, ROOM_RENT)
    && (ch->in_room->rented_by == NULL
      || str_cmp(ch->in_room->rented_by, ch->short_descr))) {
                  send_to_char ("You are not allowed to sleep here!\r\n",ch);
                  return;
    }

    if (IS_AFFECTED(ch, AFF_MIST)) {
         affect_strip_afn( ch, gafn_mist);
         REMOVE_BIT( ch->affected_by, AFF_MIST);
    }

    new_pos_desc = NULL;

    if ( argument[0] != '\0' ) {
         argument = one_argument( argument, arg);
         obj = get_obj_list( ch, arg, ch->in_room->contents );
          if ( obj == NULL )    {
               act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return;
          }
          if (!obj->item_type == ITEM_FURNITURE
          || !IS_SET(obj->value[0], FURN_SLEEP)) {
             send_to_char( "You can't sleep on that!\r\n", ch );
             return;
          }

        if (obj->condition < 5) {
             send_to_char( "It's much too damaged.\r\n", ch );
             return;
        }

          ch->on_furniture = obj;
          set_furnaff(ch, obj);
          sprintf( buf, "sleeping on %s", obj->short_descr);
          new_pos_desc = buf;
    }

   if (!IS_NPC(ch)) {
       if (ch->pcdata->mounted==TRUE) {
           send_to_char( "You can't sleep in the saddle!\r\n", ch );
           return;
       }
   }

    cancel_acv(ch);

    switch ( ch->position ) {

      case POS_FIGHTING:
	send_to_char( "You cannot sleep when you are fighting!\r\n", ch );
	break;

      case POS_SLEEPING:
      case POS_RESTING:
      case POS_STANDING:
      case POS_SITTING:
        set_activity( ch, POS_SLEEPING, new_pos_desc, ACV_NONE, NULL);
        break;

      default:
        send_to_char( "You are to stunned and injured for that!\r\n", ch);
        break;
    }

    return;
}

void do_wake( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );

    if ( arg[0] == '\0' ) { 
        do_stand( ch, argument );
        return;
    }

    if (!IS_AWAKE(ch) )	{
        send_to_char( "You are asleep yourself!\r\n",       ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL ) {
          send_to_char( "They aren't here.\r\n",              ch ); 
          return;
    }

    if ( IS_AWAKE(victim) ) {
         act( "$N is already awake.", ch, NULL, victim, TO_CHAR );
          return; 
    }

    if ( IS_AFFECTED(victim, AFF_SLEEP)) { 
          act( "You can't wake $M!",   ch, NULL, victim, TO_CHAR );  
          return;
    }

    if (victim->position <= POS_STUNNED) {
          act( "You can't wake $M!",   ch, NULL, victim, TO_CHAR );  
          return;
    }

    set_activity( victim, POS_STANDING, NULL, ACV_NONE, NULL);
    if (victim->on_furniture != NULL) remove_furnaff(victim, victim->on_furniture);
    victim->on_furniture = NULL;    
    
    for ( d = descriptor_list; d != NULL; d = d->next ) {
            if ( d->snoop_by == victim->desc ) {
                  d->snoop_by = NULL;
                  send_to_char( "You lose your contact.\r\n", victim );
            }
    }

    act( "You wake $M.", ch, NULL, victim, TO_CHAR );
    act( "$n wakes you.", ch, NULL, victim, TO_VICT );

    return;
}



void do_sneak( CHAR_DATA *ch, char *argument ) {
int diff = 0;

    if ( ch->position < POS_FIGHTING ) {
      send_to_char("You can only sneak when you are standing...\r\n", ch);
      return;
    } 

    if (!IS_NPC(ch)) {
        if (ch->pcdata->mounted==TRUE) {
            send_to_char("You cannot sneak when riding.\r\n", ch);
            return;
        } 
    }       

    if (!check_activity_key(ch, argument)) return;

    if (ch->fighting) {
        if (IS_NPC(ch)) {
            return;
        } else {
            if (ch->pcdata->style != STYLE_NINJITSU) {
                 send_to_char("You can't sneak during combat.\r\n", ch);
                 return;
            } 
        }
        diff = 35;
    }

   /* Set sneak bit as appropriate... */

    if (check_skill(ch, gsn_sneak, diff)) {
        SET_BIT(ch->affected_by, AFF_SNEAK);
        if (ch->fighting) stop_fighting(ch, TRUE);
    } else {
        REMOVE_BIT(ch->affected_by, AFF_SNEAK);
    }

    check_improve(ch,gsn_sneak,TRUE,3);

   /* Set activity... */

    set_activity( ch, ch->position, NULL, ACV_SNEAKING, "sneaking around.");

    set_activity_key(ch);

   /* All done... */

    return;
}


void do_hide( CHAR_DATA *ch, char *argument ) {
int diff = 0;

    if ( ch->position < POS_FIGHTING ) {
      send_to_char("You can only hide when you are standing...\r\n", ch);
      return;
    } 

    if (!check_activity_key(ch, argument)) return;
    if (ch->fighting) {
        if (IS_NPC(ch)) {
            return;
        } else {
            if (ch->pcdata->style != STYLE_NINJITSU) {
                 send_to_char("You can't hide during combat.\r\n", ch);
                 return;
            } 
        }
        diff = 35;
    }

   /* Set hide bit as appropriate... */

    if (check_skill(ch, gsn_hide, diff)) {
         SET_BIT(ch->affected_by, AFF_HIDE);
         if (ch->fighting) stop_fighting(ch, TRUE);
    } else {
         REMOVE_BIT(ch->affected_by, AFF_HIDE);
    }
    check_improve(ch,gsn_hide,TRUE,3);

   /* Set the activity... */

    set_activity( ch, ch->position, NULL, ACV_HIDING, "skulking in the shadows.");
    set_activity_key(ch);

   /* All done... */

    return;
}


void do_visible( CHAR_DATA *ch, char *argument ) {

    affect_strip_afn( ch, gafn_invisability);

    REMOVE_BIT( ch->affected_by, AFF_HIDE	);
    REMOVE_BIT( ch->affected_by, AFF_INVISIBLE	);
    REMOVE_BIT( ch->affected_by, AFF_SNEAK	);

    if (IS_AFFECTED(ch, AFF_MIST)) {
         affect_strip_afn( ch, gafn_mist);
         REMOVE_BIT( ch->affected_by, AFF_MIST);
    }

   /* Remove wiz invis... */

    REMOVE_BIT(ch->plr, PLR_WIZINVIS);
    ch->invis_level = 0;

   /* Remove player cloak... */

    REMOVE_BIT(ch->plr, PLR_CLOAK);
    ch->cloak_level = 0;

   /* Remove polymorphs... */

    if (IS_AFFECTED (ch, AFF_POLY) && !is_affected(ch, gafn_mask_hide) && !is_affected(ch, gafn_mask_force)) {
      affect_strip_afn( ch, gafn_mask_self);
      undo_mask(ch, FALSE);
      if (!IS_SET(ch->act, ACT_WERE)) REMOVE_BIT( ch->affected_by, AFF_POLY );
    }

    if (IS_AFFECTED (ch, AFF_MORF)) {
        affect_strip_afn( ch, gafn_morf);
        REMOVE_BIT( ch->affected_by, AFF_MORF );
        undo_morf(ch, ch->trans_obj);
    }

   /* Ok, should be visible... */

    act( "$n's reveals $mself.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "Ok, you can now be seen.\r\n", ch );

    set_activity( ch, ch->position, NULL, ACV_NONE, NULL);

    return;
}


void do_land( CHAR_DATA *ch, char *argument ) {

    if (!IS_AFFECTED(ch, AFF_FLYING)) {
             send_to_char( "You aren't flying!\r\n", ch );
             return;
    }

    if (ch->in_room->sector_type == SECT_WATER_SWIM
    || ch->in_room->sector_type == SECT_WATER_NOSWIM
    || ch->in_room->sector_type == SECT_AIR
    || ch->in_room->sector_type == SECT_UNDERWATER
    || ch->in_room->sector_type == SECT_SPACE) {
             send_to_char( "You can't land here!\r\n", ch );
             return;
    }

    affect_strip_afn(ch, gafn_fly);
    REMOVE_BIT(ch->affected_by, AFF_FLYING);
    act("You slowly float to the ground." , ch, NULL, NULL, TO_CHAR);
    act("$n slowly float to the ground." , ch, NULL, NULL, TO_ROOM);
    return;
}


void do_launch( CHAR_DATA *ch, char *argument ) {

    if (IS_AFFECTED(ch, AFF_FLYING)
    || is_affected(ch, gafn_fly)) {
             send_to_char( "You are already flying!\r\n", ch );
             return;
    }

    if (ch->race_orig != 0
    || !IS_SET(race_array[ch->race].aff, AFF_FLYING)) {
             send_to_char( "You don't know how to fly!\r\n", ch );
             return;
    }

    if (ch->in_room->sector_type == SECT_WATER_SWIM
    || ch->in_room->sector_type == SECT_WATER_NOSWIM
    || ch->in_room->sector_type == SECT_UNDERWATER) {
             send_to_char( "You can't launch here!\r\n", ch );
             return;
    }

    SET_BIT(ch->affected_by, AFF_FLYING);
    act("You launch youself up into the air." , ch, NULL, NULL, TO_CHAR);
    act("$n launches $mself up into the air." , ch, NULL, NULL, TO_ROOM);
    return;
}


void do_recall( CHAR_DATA *ch, char *argument ) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    CHAR_DATA *victim, *rch, *next_rch, *spouse;
    ROOM_INDEX_DATA *location, *old_room, *loc2;

    if ( IS_NPC(ch) 
      && (!IS_SET(ch->act,ACT_PET) 
	|| !IS_SET(ch->act,ACT_FOLLOWER) ) ) {
	send_to_char("Only players can recall.\r\n",ch);
	return;
    }

    act( "$n prays for transportation!", ch, 0, 0, TO_ROOM );

   /* Determine recall location... */

    location = NULL;

    if ( argument[0] == '\0' ) {
      /* Zeroth we check the rooms recall location... */

       if ( ch->in_room != NULL
       && ch->in_room->recall != 0 ) {
            location = get_room_index( ch->in_room->recall);
       }

      /* First we check the areas recall location... */

        if ( location == NULL
       && ch->in_room != NULL
       && ch->in_room->area != NULL 
       && ch->in_room->area->recall != 0 ) {
            location = get_room_index( ch->in_room->area->recall);
       }

      /* Then we check zone recall location... */

        if ( location == NULL
       && ch->in_room != NULL
       && ch->in_room->area != NULL 
       && zones[ch->in_room->area->zone].recall != 0 ) {
             location = get_room_index( zones[ch->in_room->area->zone].recall);
       }

      /* Finally we check the default location... */

        if ( location == NULL ) location = get_room_index( mud.recall );
    

      /* Next we check the characters permanent recall location... */
    
        if ( location == NULL ) location = get_room_index( ch->recall_perm );
    
   } else {
    
        argument = one_argument(argument, arg);

        if (str_cmp(capitalize(arg), ch->pcdata->spouse)) {
             send_to_char( "Why should you be able to recall there!", ch );
             return;
        }

        if ((spouse = get_char_world( ch, ch->pcdata->spouse) ) == NULL) {
             send_to_char( "They are currently not logged in!", ch );
             return;
        }

        if (ch->in_room->area->zone != spouse->in_room->area->zone) {
             send_to_char( "You're too far away!", ch );
             return;
        }

        location = spouse->in_room;
   }

      /* If still not there, then give up... */

     if ( location == NULL ) {
	send_to_char( "You are completely lost.\r\n", ch );
	return;
    }

   /* Check for day/night shift... */

    if ( IS_SET(time_info.flags, TIME_NIGHT) ) {

      if (location->night != 0) { 
        loc2 = get_room_index(location->night);

        if ( loc2 != NULL ) {
          location = loc2;
        }
      }
    } else if ( IS_SET(time_info.flags, TIME_DAY) ) {
      if (location->day != 0) { 
        loc2 = get_room_index(location->day);

        if ( loc2 != NULL ) {
          location = loc2;
        }
      }
    }

   /* No where to go from here... */

    if ( ch->in_room == location )
	return;

   /* Check for permission... */

    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_AFFECTED(ch, AFF_CURSE) ) {
	send_to_char( "Marduk has forsaken you.\r\n", ch );
	return;
    }

    if IS_AFFECTED(ch, AFF_MELD) {
	send_to_char( "You cannot concentrate enough to pray.\r\n", ch );
	return;
    }

   /* Cross zone recall is a no-no... */

    if ( ch->in_room->area != NULL
      && location->area != NULL
      && ch->in_room->area->zone != location->area->zone ) {
      send_to_char("You cannot feel your recall point!\r\n", ch);
      return;
    }

   /* Recall from combat is equivilent to flee... */

    victim = ch->fighting;

    if ( victim != NULL ) {

	int lose;

	if (!check_skill(ch, gsn_recall, -20)) {
	    check_improve(ch,gsn_recall,FALSE,6);
	    WAIT_STATE( ch, 4 );
	    sprintf( buf, "You failed!.\r\n");
	    send_to_char( buf, ch );
	    return;
	}

	lose = (ch->desc != NULL) ? 15 : 30;
	gain_exp( ch, 0 - lose, FALSE );
	check_improve(ch,gsn_recall,TRUE,4);
	send_to_char( "You recall from combat!\r\n", ch );
	stop_fighting( ch, TRUE );
	
    }

    ch->move /= 2;

    old_room = ch->in_room;

    if (old_room != NULL) {
      act( "$n disappears.", ch, NULL, NULL, TO_ROOM );
    } 

    if (!IS_AFFECTED(ch, AFF_SNEAK)) {
      add_tracks(ch, DIR_NONE, DIR_OTHER);
    }

    char_from_room( ch );

    char_to_room( ch, location );

    if (!IS_AFFECTED(ch, AFF_SNEAK)) {
      add_tracks(ch, DIR_OTHER, DIR_HERE);
    }

    act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );

    do_look( ch, "auto" );

   /* Recall the pet as well... */
 
    if (old_room != NULL) {
      for (rch = old_room->people; rch != NULL; rch = next_rch) {

        next_rch = rch->next_in_room;

        if ( rch->in_room == old_room 
        && str_cmp(race_array[rch->race].name,"machine")
        && !IS_SET(rch->form, FORM_MACHINE)
        && rch->master == ch ) { 
          act( "$n disappears.", rch, NULL, NULL, TO_ROOM );
          char_from_room( rch );
          char_to_room( rch, location );
          act( "$n appears in the room.", rch, NULL, NULL, TO_ROOM );
        }
      }
    }

    return;
}

void do_enter (CHAR_DATA *ch, char *argument) {
OBJ_DATA *portal;
OBJ_DATA *pkey=NULL;
OBJ_DATA *tmp=NULL;
ROOM_INDEX_DATA *dest, *dest2;
ROOM_INDEX_DATA *old_room;
char buf[MAX_STRING_LENGTH];
char arg1[MAX_INPUT_LENGTH];
char * lastarg;
int room;
bool pet=FALSE;
bool link_obj=FALSE;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

    if (argument[0] == '\0') {
	send_to_char ("Enter what?\r\n",ch);
	return;
    }

   /* check fighting */	

    if (ch->fighting != NULL) {
      send_to_char ("Not while you are fighting!\r\n", ch);
      return;
    }

   /* get portal name */	
    one_argument (argument, arg1);
	
     portal = get_eq_char( ch, WEAR_HOLD );
     if (!portal) {
        portal = get_obj_here (ch, arg1);
     } else if (portal->item_type != ITEM_PORTAL) {
        portal = get_obj_here (ch, arg1);
     }
	
    if (!portal) {
      sprintf_to_char(ch, "You see no %s here.\r\n", arg1);
      return;
    }

    if (portal->item_type != ITEM_PORTAL) {
      send_to_char ("You can't enter that.\r\n", ch);
      return;
    }

     if (portal->condition < 5) {
             send_to_char( "It's much too damaged.\r\n", ch );
             return;
     }

    if (portal->value[4] == PORTAL_VEHICLE
    && !IS_NPC(ch)) {
         if (ch->pcdata->mounted==TRUE) {
             send_to_char ("Get of your mount first.\r\n", ch);
             return;
         }

         if (IS_SET(portal->value[3], CONT_CLOSED) ) {
             send_to_char( "It's closed.\r\n", ch); 
             return; 
         }
    }
   	
   /* check level of portal vs level of character */	

    if (get_trust(ch) + 10 < portal->level) {
        sprintf (buf, "You step into %s, but it throws you back out!\r\n", portal->short_descr) ;
        send_to_char (buf, ch);
        return;
    }

    if (portal->value[2] > 0) {
         pkey = ch->carrying;
         while ( pkey != NULL
         && ( pkey->pIndexData->vnum != portal->value[2]
                 || pkey->wear_loc != -1
                 || !can_see_obj(ch, pkey) ) ) {
                      pkey = pkey->next_content;
         }

         if ( pkey == NULL ) {
                send_to_char ("You lack the key.\r\n", ch);
                return;
         }
    }

    if (portal->value[4] == PORTAL_MAGIC) {
        lastarg = check_obj_owner(portal);
        if (str_cmp(lastarg, "not found")
        && str_cmp(capitalize(lastarg), ch->short_descr)) {
            sprintf (buf, "%s growls menacingly as you approach!\r\n", capitalize(portal->short_descr)) ;
            send_to_char (buf, ch);
            act("$p begins to growl menacingly!.",ch, portal, NULL, TO_ROOM);
            return;
        }      
    }

    mcc = get_mcc(ch, ch, NULL, NULL, portal, NULL, 0, NULL);

    if (portal->pIndexData
    && portal->pIndexData->wear_cond) {
        if (!ec_satisfied_mcc(mcc, portal->pIndexData->wear_cond, TRUE)) {
            free_mcc(mcc);
            sprintf_to_char(ch, "You are unable to use %s.\r\n", portal->short_descr );
            act( "$n tries to use $p, but fails.", ch, portal, NULL, TO_ROOM );
            return;
        }
    }
    free_mcc(mcc);
    mcc = NULL;

    room = portal->value[0];
    if (room == 0
    && (portal->value[4] == PORTAL_MAGIC || portal->value[4] == PORTAL_MIRROR)) {
        int count;

        for (count = 0 ; count < 50 ; count++ ) {
               dest = get_room_index( number_range( 0, 32767));
               if (!dest ||  !dest->area) continue;
  
               if (can_see_room(ch, dest)
               && !IS_SET(dest->room_flags, ROOM_SOLITARY)
               && !IS_SET(dest->area->area_flags, AREA_LOCKED)) break;
        }

        if (count >= 50 || !dest) {
           send_to_char("They fade, but then return...\r\n", ch); 
           return;
        }
        send_to_char("{mYou feel torn in different directions.{x\r\n",ch);

    } else {
        dest = get_room_index(room);
        if (!dest) {
            bug("Bad portal room vnum %d", room);
            send_to_char ("Bad portal, please inform the Imp.\r\n", ch);
            return;
        }
    }
	
   /* Check for day/night shift... */

    if ( IS_SET(time_info.flags, TIME_NIGHT) ) {
        if (dest->night != 0) { 
            dest2 = get_room_index(dest->night);
            if (dest2 ) dest = dest2;
        }
    } else if ( IS_SET(time_info.flags, TIME_DAY) ) {
        if (dest->day != 0) { 
            dest2 = get_room_index(dest->day);
            if (dest2)  dest = dest2;
        }
    }

   /* determination if link object exists in destination room */

    if ( portal->value[1] == 0 ) {
        link_obj = FALSE;	
    } else if ( dest->contents == NULL ) {
        link_obj = FALSE;
    } else {

        tmp = dest->contents;

        while ( tmp != NULL && !link_obj) {
             if ( tmp->pIndexData->vnum == portal->value[1]) link_obj = TRUE;
             if (!link_obj) tmp = tmp->next_content;
        }	
    }
	
    if (ch->pet && ch->in_room == ch->pet->in_room) pet = TRUE;
	
    mcc = get_mcc(ch, ch, NULL, NULL, portal, tmp, DIR_OTHER, NULL);

    switch (portal->value[4]) {
          default:
              wev = get_wev(WEV_DEPART, WEV_DEPART_PORTAL, mcc,
                   "{mYou step into the shadows of @p2.{x\r\n",
                    NULL,
                   "{M@a2 steps into the shadows of @p2!{x\r\n");
              break;
          case 1:
              wev = get_wev(WEV_DEPART, WEV_DEPART_PORTAL, mcc,
                   "{mYou enter @p2.{x\r\n",
                    NULL,
                   "{M@a2 enters @p2!{x\r\n");
              break;
          case 2:
              wev = get_wev(WEV_DEPART, WEV_DEPART_PORTAL, mcc,
                   "{mYou step through the surface of @p2.{x\r\n",
                    NULL,
                   "{M@a2 steps through the surface of @p2!{x\r\n");
              break;
    }

     /* Ask if we can leave... */     
    if (!IS_SET(ch->plr, PLR_WIZINVIS)) {
        if (!room_issue_wev_challange(ch->in_room, wev)) {
            free_wev(wev);
            return;
        }
    }

   /* Declare our departure (has to be done first, se we get the messages... */
 
    if (!IS_SET(ch->plr, PLR_WIZINVIS)) room_issue_wev(ch->in_room, wev);
    free_wev(wev);

   /* Move the character... */

    old_room = ch->in_room;

    if (!IS_AFFECTED(ch, AFF_SNEAK)) add_tracks(ch, DIR_NONE, DIR_OTHER);

    char_from_room (ch);
    char_to_room(ch, dest);

    if (!IS_AFFECTED(ch, AFF_SNEAK)) add_tracks(ch, DIR_OTHER, DIR_HERE);

   /* Same for the pet, but no challange... */

    if (pet) {
        mcc = get_mcc(ch->pet, ch->pet, NULL, NULL, portal, tmp, DIR_OTHER, NULL);
        switch (portal->value[4]) {
          default:
              wev = get_wev(WEV_DEPART, WEV_DEPART_PORTAL, mcc,
                   "{mYou step into the shadows of @p2.{x\r\n",
                    NULL,
                   "{M@a2 steps into the shadows of @p2!{x\r\n");
              break;
          case 1:
              wev = get_wev(WEV_DEPART, WEV_DEPART_PORTAL, mcc,
                   "{mYou enter @p2.{x\r\n",
                    NULL,
                   "{M@a2 enters @p2!{x\r\n");
              break;
          case 2:
              wev = get_wev(WEV_DEPART, WEV_DEPART_PORTAL, mcc,
                   "{mYou step through the surface of @p2.{x\r\n",
                    NULL,
                   "{M@a2 steps through the surface of @p2!{x\r\n");
              break;
        }

       /* Tell the old room that the pet has gone... */

        if (!IS_SET(ch->plr, PLR_WIZINVIS)) room_issue_wev(old_room, wev);

        free_wev(wev);
        char_from_room(ch->pet);
        char_to_room(ch->pet,dest);
    }

   /* Declare our arrival... */
 
    mcc = get_mcc(ch, ch, NULL, NULL, portal, tmp, DIR_OTHER, NULL);

    if (link_obj) { 
         switch(portal->value[4]) {
              default:
                   wev = get_wev(WEV_DEPART, WEV_DEPART_PORTAL, mcc,
                   "{mYou step out of the shadows of @s2.{x\r\n",
                    NULL,
                   "{M@a2 steps out of @p2!{x\r\n");
                   break;
              case 1:
                   wev = get_wev(WEV_DEPART, WEV_DEPART_PORTAL, mcc,
                   "{mYou enter through @s2.{x\r\n",
                    NULL,
                   "{M@a2 enters through @p2!{x\r\n");
                   break;
              case 2:
                   wev = get_wev(WEV_DEPART, WEV_DEPART_PORTAL, mcc,
                   "{mYou appear through @s2.{x\r\n",
                    NULL,
                   "{M@a2 appears through @p2!{x\r\n");
                   break;
         }
      } else {
         switch(portal->value[4]) {
              default:
                  wev = get_wev(WEV_DEPART, WEV_DEPART_PORTAL, mcc,
                   "{mYou step out of the shadows.{x\r\n",
                    NULL,
                   "{M@a2 steps out of the shadows.{x\r\n");
                   break;
              case 1:
                   wev = get_wev(WEV_DEPART, WEV_DEPART_PORTAL, mcc,
                   "{mYou enter.{x\r\n",
                    NULL,
                   "{M@a2 enters!{x\r\n");
                   break;
              case 2:
                   wev = get_wev(WEV_DEPART, WEV_DEPART_PORTAL, mcc,
                   "{mYou appear.{x\r\n",
                    NULL,
                   "{M@a2 appears!{x\r\n");
                   break;
         }
      }

     /* Tell the new room that we are here... */

      if (!IS_SET(ch->plr, PLR_WIZINVIS)) room_issue_wev(ch->in_room, wev);
      free_wev(wev);

     /* Now declare the pets arrival... */

    if (pet) {
        mcc = get_mcc(ch->pet, ch->pet, NULL, NULL, portal, tmp, DIR_OTHER, NULL);
 
        if (link_obj) { 
           switch(portal->value[4]) {
              default:
                   wev = get_wev(WEV_DEPART, WEV_DEPART_PORTAL, mcc,
                   "{mYou step out of the shadows of @s2.{x\r\n",
                    NULL,
                   "{M@a2 steps out of @p2!{x\r\n");
                   break;
              case 1:
                   wev = get_wev(WEV_DEPART, WEV_DEPART_PORTAL, mcc,
                   "{mYou enter through @s2.{x\r\n",
                    NULL,
                   "{M@a2 enters through @p2!{x\r\n");
                   break;
              case 2:
                   wev = get_wev(WEV_DEPART, WEV_DEPART_PORTAL, mcc,
                   "{mYou appear through @s2.{x\r\n",
                    NULL,
                   "{M@a2 appears through @p2!{x\r\n");
                   break;
           }
        } else {
           switch(portal->value[4]) {
              default:
                  wev = get_wev(WEV_DEPART, WEV_DEPART_PORTAL, mcc,
                   "{mYou step out of the shadows.{x\r\n",
                    NULL,
                   "{M@a2 steps out of the shadows.{x\r\n");
                   break;
              case 1:
                   wev = get_wev(WEV_DEPART, WEV_DEPART_PORTAL, mcc,
                   "{mYou enter.{x\r\n",
                    NULL,
                   "{M@a2 enters!{x\r\n");
                   break;
              case 2:
                   wev = get_wev(WEV_DEPART, WEV_DEPART_PORTAL, mcc,
                   "{mYou appear.{x\r\n",
                    NULL,
                   "{M@a2 appears!{x\r\n");
                   break;
           }
        }

       /* Tell the new room that the pet is here... */

        room_issue_wev(ch->pet->in_room, wev);
        free_wev(wev);
    }
    
   /* Lets see where we are... */
  
    do_look (ch, "auto"); 
    if (pet) do_look(ch->pet,"auto");
    return;
}


void do_ride( CHAR_DATA *ch, char *argument ) {
  int chance;
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  if(IS_NPC(ch))  return;

  one_argument( argument, arg );

  if( arg[0] == '\0' )    {
      send_to_char("What do you want to mount?\r\n",ch);
      return;
    }

  if( ( victim = get_char_room( ch, arg ) ) == NULL )    {
      send_to_char("That's not there.\r\n",ch);
      return;
    }

  if( !IS_NPC(victim)
  || victim != ch->pet
  || victim->fighting != NULL
  || !IS_SET(victim->act, ACT_MOUNT) )    {
      act("You can't ride $N.",ch,NULL,victim,TO_CHAR);
      return;
    }

  if( !IS_NPC(ch) && ch->pcdata->mounted )    {
      send_to_char("You must dismount first.\r\n",ch);
      return;
    }

  if(IS_NPC(victim) && victim->ridden)    {
      act("$N already has a rider.",ch,NULL,victim,TO_CHAR);
      return;
    }

  chance = get_skill(ch,gsn_riding);

  if( chance < 2 )   {
      send_to_char( "You lack the skills...\r\n",ch);
      return;
    }

  chance = UMIN(chance, 90);

  if( IS_IMMORTAL(ch)) chance = 100;

  /* lets see if the mount bucks the rider */

  if( number_percent () > chance)    {
      act("You fall down.",ch,NULL,NULL,TO_CHAR);
       if(ch->hit < 25) ch->hit = 1;
       else ch->hit -= 25;

       check_improve(ch,gsn_riding, FALSE,6);
      return;
    }

  WAIT_STATE( ch, skill_array[gsn_riding].beats );

  if(!IS_SET(ch->plr, PLR_WIZINVIS) ) {
     sprintf(buf,"%s saddles %s and mounts $m.", ch->name, victim->short_descr ? victim->short_descr: victim->name );
    act( buf,ch,NULL,NULL,TO_ROOM );
   }
   act( "You mount your $N.",ch , NULL, victim, TO_CHAR );
   check_improve(ch,gsn_riding,FALSE,6);

  SET_BIT(victim->act, ACT_PET);
  victim->master = ch;
  ch->pet = victim;

  ch->pet->ridden = TRUE;
  ch->pcdata->mounted = TRUE;

   return;
}


void do_smash( CHAR_DATA *ch, char *argument ) {
char arg[MAX_INPUT_LENGTH];
int door=-1;
int doorpwr, toolpwr, doordam;
OBJ_DATA *tool;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

    one_argument( argument, arg );

    if (IS_AFFECTED(ch, AFF_MIST)) {
	send_to_char( "You can't do that in mist form?\r\n", ch );
	return;
    }

    if ( arg[0] == '\0' )    {
	send_to_char( "Smash what?\r\n", ch );
	return;
    }

    toolpwr=0;

    if ((tool = get_eq_char(ch, WEAR_HOLD)) != NULL) {
         if (tool->item_type == ITEM_DIGGING )  toolpwr=tool->level/4;
    }

    if (toolpwr==0) {
    if ((tool = get_eq_char(ch, WEAR_WIELD)) != NULL) { 
         if (tool->value[0] == WEAPON_AXE) toolpwr = tool->level/8;
         if (tool->value[0] == WEAPON_MACE) toolpwr = tool->level/6;
    }
    }
    door = find_door( ch, arg );

    if ( door >= 0 ) {
	/* 'open door' */
	EXIT_DATA *pexit;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) ) {
                     send_to_char( "It's already open.\r\n",ch );
                     return;
                }

                doorpwr=number_percent();
                if ( IS_SET(pexit->exit_info, EX_LOCKED) ) doorpwr +=10;
                if ( IS_SET(pexit->exit_info, EX_PICKPROOF) ) doorpwr +=5;
                if ( IS_SET(pexit->exit_info, EX_NO_PASS) ) doorpwr +=15;
                if ( IS_SET(pexit->exit_info, EX_ROBUST) ) doorpwr +=35;
                if ( IS_SET(pexit->exit_info, EX_ARMOURED) ) doorpwr +=50;
                if ( IS_SET(pexit->exit_info, EX_WALL) ) doorpwr +=40;
                if (toolpwr>0) {
                      act("You ready $p...",ch,tool,NULL,TO_CHAR );
                      act("$n readies $p...",ch,tool,NULL,TO_ROOM );
                }
                doordam = (get_curr_stat(ch, STAT_STR) + toolpwr + ch->level/10 - 7) * 5 / (UMAX(doorpwr * pexit->condition / 100, 5)) + 1;
                pexit->condition -= doordam;
               
                if (pexit->condition > 0) {
                      send_to_char("You crash against the door.\r\n",ch);
                      send_to_char("{rThat hurts!{x.\r\n",ch);
                      act("With all $s might, $n crashes against the door...",ch,NULL,NULL,TO_ROOM );
                      ch->hit -=doorpwr * pexit->condition / 200;
                      ch->move -=doorpwr * pexit->condition / 25;
                      update_pos( ch );
                      kill_msg( ch);
                 } else {
                     pexit->condition = 0;
                     mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, door, dir_name[door]);
                     wev = get_wev(WEV_LOCK, WEV_LOCK_OPEN_DOOR, mcc,
                           "You smash the @t0 door.\r\n",
                            NULL,
                            "@a2 smashes the @t0 door.\r\n");
                     if (!room_issue_wev_challange(ch->in_room, wev)) {
                           free_wev(wev);
                           return;
                     }
                    set_door_flags(ch->in_room, door, pexit, EX_LOCKED, FALSE);
                    set_door_flags(ch->in_room, door, pexit, EX_CLOSED, FALSE);
                    room_issue_door_wev(ch->in_room, wev, "The @t0 door breaks open.\r\n");
                    free_wev(wev);
                }                    
       }
   return;
}

void panic(CHAR_DATA *ch) {
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    int aligndiff;
    int chance;

     for ( fch = ch->in_room->people; fch != NULL; fch = fch_next )    {
            fch_next = fch->next_in_room;
            if (ch==fch) continue;
            if (!can_see( fch, ch )) continue;

            if (IS_NPC(fch)
            && (IS_SET(fch->act, ACT_PET)
                  || IS_SET(fch->act, ACT_FOLLOWER)
                  || IS_SET(fch->act, ACT_SENTINEL)
                  || IS_SET(fch->act, ACT_PRACTICE)
                  || IS_SET(fch->act, ACT_IS_ALIENIST)
                  || IS_SET(fch->act, ACT_IS_HEALER)
                  || IS_SET(fch->act, ACT_AUTOHOME)
                  || IS_AFFECTED(fch, AFF_CHARM))) {
                          continue;
            }

            aligndiff = ch->alignment - fch->alignment;
            if (abs(aligndiff)  > 1000) {
                 chance = (ch->level + 5 - fch->level) * 3;
                 if (number_percent() < chance) {
                       act("{y$n flees in terror at $Ns sight.{x\r\n", fch,NULL,ch,TO_ROOM);
                       send_to_char("{rYor are filled with terror!{x\r\n", fch);
                       do_flee(fch,"");
                 }
            }
    }

  return;
}


void set_furnaff( CHAR_DATA *ch, OBJ_DATA *obj) {
    AFFECT_DATA *paf;

    if (!obj->enchanted)
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
		{
	   		affect_modify( ch, paf, TRUE ); 
		}
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
		{
			affect_modify( ch, paf, TRUE );
		}

    return;
}


void remove_furnaff( CHAR_DATA *ch, OBJ_DATA *obj) {
    AFFECT_DATA *paf;

    if (!obj->enchanted)
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
		{
	 	  	affect_modify( ch, paf, FALSE );
		}
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
		{
			affect_modify( ch, paf, FALSE );
		}
    return;
}


void undo_morf(CHAR_DATA *ch, OBJ_DATA *obj) {
        ch->trans_obj = NULL;
        set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
        if (obj) {
           act("$p changes its shape to look like $n.", ch, obj, NULL, TO_ROOM);
            extract_obj(obj);
        }
        send_to_char("You return to your original form.\r\n", ch);
        return;
}


void check_ritual_interrupt(CHAR_DATA *ch) {
     CHAR_DATA *fch = ch->in_room->people;

     while(fch) {
          if (is_same_group(fch, ch)
          && fch != ch) {
                if (fch->activity == ACV_CASTING
                && fch->acv_int3 > 0) {
                         set_activity(fch, fch->position, NULL, ACV_NONE, NULL);
                          act( "{YThe ritual is interrupted.{x", ch, NULL, NULL,TO_ROOM );
                }
          }
          fch = fch->next_in_room;
     }
     return;
}


ROOM_INDEX_DATA *find_telekinesis_room(CHAR_DATA *ch, int dir) {
EXIT_DATA *pexit;
ROOM_INDEX_DATA *room;

         pexit = ch->in_room->exit[dir];
         if (!pexit) {
            send_to_char("That exit doesn't exist.\r\n", ch);
            return NULL;
         }

         if (!exit_visible(ch, pexit)) {
            send_to_char("That exit doesn't exist.\r\n", ch);
            return NULL;
         }

         if ( IS_SET(pexit->exit_info, EX_CLOSED)) {
              if ( IS_SET(pexit->exit_info, EX_WALL)
              || IS_SET(pexit->exit_info, EX_HIDDEN)) {
                    send_to_char("That exit doesn't exist.\r\n", ch);
                    return NULL;
              }
              send_to_char("That exit is closed.\r\n", ch);
              return NULL;
         }

         room = get_exit_destination(ch, ch->in_room, pexit, TRUE);
         return room;
}


void cancel_acv(CHAR_DATA *ch) {
CHAR_DATA *victim;
OBJ_DATA *obj;
AFFECT_DATA af;
int level;

     if (!ch->acv_text) return;
     victim = get_char_room(ch, ch->acv_text);

     if (ch->activity != ACV_PHOTO) {
         if ((obj = get_eq_char(ch, WEAR_HOLD)) != NULL) {
            if (obj->item_type == ITEM_CAMERA) obj->value[4] = 0;
         }
     }

     switch(ch->activity) {

       case ACV_DEBATING:
         send_to_char("{mYou finally give in.{x\r\n", ch);
         gain_exp(ch, -100, FALSE );

         set_activity(ch, ch->position, ch->pos_desc, ACV_NONE, NULL);
         if (victim) set_activity(victim, victim->position, victim->pos_desc, ACV_NONE, NULL);
         break;

       case ACV_DUEL:
         send_to_char("{mYou finally give up.{x\r\n", ch);

         if (victim) {
             level = (victim->acv_int - ch->acv_int) /5;
             if (level > 100) level = (level -100) /2 +100;
             if (level > 125) level = (level -125) /2 +125;
             if (level > 150) level = (level -150) /2 +150;
             if (level < 20) level = 20;

             af.type      	= 0;
             af.afn 	= gafn_fatigue;
             af.level	= level;
             af.duration  	= level /2;
             af.location  	= APPLY_INT;
             af.modifier  	= -level /10;
             af.bitvector 	= AFF_MELD;
             affect_to_char(ch, &af );

             victim->mana = UMIN(victim->mana + ch->mana, victim->max_mana);
             ch->mana = 0;
             victim->move = UMIN(victim->mana + ch->move/5, victim->max_mana);
             ch->move = 0;
         }
 
         update_pos(ch);
         set_activity(ch, ch->position, ch->pos_desc, ACV_NONE, NULL);
         if (victim) set_activity(victim, victim->position, victim->pos_desc, ACV_NONE, NULL);
         break;

     }
     return;
}


void figurine_trigger(OBJ_DATA *obj, OBJ_DATA *cont, CHAR_DATA *ch) {
MOB_INDEX_DATA *pMob;  
CHAR_DATA *mob;
EXTRA_DESCR_DATA *ed;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

          if ((pMob = get_mob_index(obj->value[0])) == NULL) return;
          if ((mob = create_mobile_level(pMob, NULL, obj->level )) == NULL) return;

          ed = obj->extra_descr;
          while (ed != NULL) {
               if (is_name("name", ed->keyword)) {
                      free_string(mob->name);
                      mob->name = strdup(ed->description);
               }
               if (is_name("short", ed->keyword)) {
                      free_string(mob->short_descr);
                      mob->short_descr = strdup(ed->description);
               }
               if (is_name("long", ed->keyword)) {
                   free_string(mob->long_descr);
                   free_string(mob->description);
                   mob->long_descr = strdup(ed->description);
                   mob->description = strdup(mob->long_descr);
               }
               ed = ed->next;
          }
          if (obj->value[3] >0) mob->race = obj->value[3];

          if (obj->value[1] == FIGURINE_AGGRESSIVE) {
                char_to_room(mob, ch->in_room );
                SET_BIT(mob->act, ACT_AGGRESSIVE);
          } else {
                char_to_room(mob, ch->in_room );
          }

          mcc = get_mcc(ch, ch, mob, NULL, obj, cont, -1, "figurine");
          wev = get_wev(WEV_TRAP, WEV_TRAP_TRIGGER, mcc,
                    "@v2 jumps out of @s2.\r\n",
                    NULL,
                    "@v2 jumps out of @s2.\r\n");

          room_issue_wev( ch->in_room, wev);
          free_wev(wev);
          extract_obj( obj );
          return;
   }


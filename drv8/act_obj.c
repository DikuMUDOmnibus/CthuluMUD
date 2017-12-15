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
#include "doors.h"
#include "econd.h"
#include "mob.h"
#include "wev.h"
#include "affect.h"
#include "exp.h"
#include "magic.h"
#include "profile.h"
#include "olc.h"
#include "race.h"
#include "bank.h"
#include "cult.h"
#include "gsn.h"


/* command procedures needed */
DECLARE_DO_FUN(do_split);
DECLARE_DO_FUN(do_yell);
DECLARE_DO_FUN(do_kill);
DECLARE_DO_FUN(do_look);


/*
 * Local functions.
 */
#define CD CHAR_DATA
#define OD OBJ_DATA 

bool		remove_obj		(CHAR_DATA *ch, int iWear, bool fReplace);
void		wear_obj		(CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, int slot);
void    		obj_to_keeper   		(OBJ_DATA *obj, CHAR_DATA *ch );
OD 		*get_obj_keeper  	(CHAR_DATA *ch,CHAR_DATA *keeper,char *argument);
void 		reload_gun 		(CHAR_DATA *ch, OBJ_DATA *obj, int gnum, bool loud);
void 		slotmachine		(CHAR_DATA *ch, OBJ_DATA *obj);
char		*photo_char  		(CHAR_DATA *victim, CHAR_DATA *ch);
void 		do_real_sac		(CHAR_DATA *ch, OBJ_DATA *obj);
int 		soc_tax			(CHAR_DATA *ch, int gold, int currency);
int		get_cost			(CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy );
void 		order_passport		(CHAR_DATA *ch, int type);

#undef  OD
#undef	CD

/* RT part of the corpse looting code */

bool can_loot(CHAR_DATA *ch, OBJ_DATA *obj) {
    CHAR_DATA *owner, *wch;

    if (IS_IMMORTAL(ch))	return TRUE;

    if (!obj->owner || obj->owner == NULL)	return TRUE;

    owner = NULL;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
        if (!str_cmp(wch->name,obj->owner))
            owner = wch;

    if (owner == NULL) return TRUE;

    if (!str_cmp(ch->name,owner->name)) return TRUE;

    if (!IS_NPC(owner) && IS_SET(owner->plr, PLR_CANLOOT)) return TRUE;

    if (is_same_group(ch,owner)) return TRUE;

    return FALSE;
}


void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container ) {
    CHAR_DATA *gch;
    CHAR_DATA *sleeper;
    EXTRA_DESCR_DATA *ed;
    int members;
    int gold;
    int zmask;
    int currency = 0;
    char buffer[100];
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

    if ( !CAN_WEAR(obj, ITEM_TAKE))    {
	send_to_char( "You can't take that.\r\n", ch );
	return;
    }

    if ( IS_SET(obj->extra_flags, ITEM_SCENIC) )    {
	send_to_char( "You can't take that.\r\n", ch );
	return;
    }

    if (obj->item_type == ITEM_FURNITURE) {
        for ( sleeper = ch->in_room->people; sleeper != NULL; sleeper = sleeper->next_in_room ) {
             if (IS_NPC(sleeper)) continue;
             if (sleeper->on_furniture == obj
             && sleeper->position <= POS_RESTING) {
	send_to_char( "Someone is using it right now.\r\n", ch );
	return;
             }
        }
    }
    
     if (obj->owner != NULL
     && str_cmp(obj->owner, ch->short_descr)
     && str_cmp(obj->owner, ch->short_descr_orig)) {
            act( "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
            act( "$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM );
            return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ))    {
	act( "$d: you can't carry that many items.", ch, NULL, obj->name, TO_CHAR );
	return;
    }


    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )    {
	act( "$d: you can't carry that much weight.", ch, NULL, obj->name, TO_CHAR );
	return;
    }

    if (!can_loot(ch,obj))    {
	act("Corpse looting is not permitted.",ch,NULL,NULL,TO_CHAR );
	return;
    }

   /* Check on getting items out of the pit... */

    if ( container != NULL 
      && container->pIndexData->vnum == OBJ_VNUM_PIT) {

      if ( (ch->level + 10) < obj->level) {
        send_to_char("You are not powerful enough to use it.\r\n",ch);
        return;
      }

      if ( !CAN_WEAR(container, ITEM_TAKE) 
        && obj->timer != 0) {
         obj->timer = 0;	
      }
    }

   /* Ok, get context and build the wev... */
        
    if (obj->item_type == ITEM_MONEY) {
      currency = obj->value[1];
      mcc = get_mcc(ch, ch, NULL, NULL, NULL, container, obj->value[0], flag_string(currency_type,currency));

      if ( container != NULL ) {
        wev = get_wev(WEV_GET, WEV_GET_GOLD, mcc,
                    "You get @n0 @t0 from @s1.\r\n",
                    "@a2 gets @n0 @t0 from @s1.\r\n",
                    "@a2 gets @n0 @t0 from @s1.\r\n");
      } else {
        wev = get_wev(WEV_GET, WEV_GET_GOLD, mcc,
                    "You get @n0 @t0.\r\n",
                    "@a2 gets @n0 @t0.\r\n",
                    "@a2 gets @n0 @t0.\r\n");
      }
    } else {
      mcc = get_mcc(ch, ch, NULL, NULL, obj, container, 0, NULL);

      if ( container != NULL ) {
        wev = get_wev(WEV_GET, WEV_GET_ITEM, mcc,
                    "You get @p2 from @s2.\r\n",
                    "@a2 gets @p2 from @s2.\r\n",
                    "@a2 gets @p2 from @s2.\r\n");
      } else {
        wev = get_wev(WEV_GET, WEV_GET_ITEM, mcc,
                    "You get @p2.\r\n",
                    "@a2 gets @p2.\r\n",
                    "@a2 gets @p2.\r\n");

         ed = obj->pIndexData->extra_descr;
         while (ed != NULL) {
               if (is_name("secondary_long", ed->keyword)) {
                     if (str_cmp(obj->description, ed->description)) {
                          free_string(obj->description);
                          obj->description = strtok(ed->description, "\n");
                          break;
                     }
               }
               ed = ed->next;
         }
 
      }
    }

   /* Challange... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

   /* Pull the object out of it's current location... */

    if ( container != NULL ) obj_from_obj( obj );
    else obj_from_room( obj );
    
   /* Give it to the character... */

    gold = 0;

    if ( obj->item_type == ITEM_MONEY) {
      gold = obj->value[0];
      gold = soc_tax(ch, gold, currency);
      ch->gold[currency] += gold;
      extract_obj( obj );

    } else {
      obj_to_char( obj, ch );
      REMOVE_BIT(obj->extra_flags, ITEM_CONCEALED);
      if (obj->item_type == ITEM_HERB) obj->timer=-1;
    
       zmask = zones[ch->in_room->area->zone].mask;

         if ( zmask != 0 ) {
              if ( obj->zmask != 0
              && (zmask & obj->zmask) == 0) {
                  obj_from_char(obj);
                  obj_to_char_ooz(obj, ch);  
                  sprintf(buffer, "{m%s evaporates!{x\r\n", obj->short_descr);
                  buffer[2] = UPPER(buffer[2]);
                  send_to_char(buffer, ch);
               }
           }

    }

   /* Send to command issuers room... */

    room_issue_wev( ch->in_room, wev);

   /* Free the wev (will autofree the context)... */

    free_wev(wev);

   /* Now split the gold up... */ 

    if ( gold > 0 
    && IS_SET(ch->plr, PLR_AUTOSPLIT) ) {

     /* See how many party members there are... */
      
      members = 0;

      gch = ch->in_room->people; 

      while ( gch != NULL ) { 
    	  
        if ( is_same_group( gch, ch ) ) {
          members++;
        }

        gch = gch->next_in_room;
      }

     /* Go split... */ 

      if ( members > 1 ) {
	sprintf(buffer, "%d %s", gold, flag_string(currency_accept, currency));
	do_split(ch,buffer);	
      }
    }
 
   /* All done... */

    return;
}



void do_get( CHAR_DATA *ch, char *argument ) {
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
OBJ_DATA *obj;
OBJ_DATA *obj_next;
OBJ_DATA *container;
bool found;

    if (IS_SET(ch->form, FORM_MIST)) {
	send_to_char( "You can't get a grip on it?\r\n", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"from"))
	argument = one_argument(argument,arg2);

    if (IS_AFFECTED(ch, AFF_MIST)) {
	send_to_char( "You can't do that in mist form?\r\n", ch );
	return;
    }

    /* Get type. */
    if ( arg1[0] == '\0' ) {
	send_to_char( "Get what?\r\n", ch );
	return;
    }

    if ( arg2[0] == '\0' ) {
	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )	{
	    /* 'get obj' */
	    obj = get_obj_list( ch, arg1, ch->in_room->contents );
	    if ( obj == NULL )    {
		act( "I see no $T here.", ch, NULL, arg1, TO_CHAR );
		return;
	    }

	    get_obj( ch, obj, NULL );
	} else {
	    /* 'get all' or 'get all.obj' */
	    found = FALSE;
	    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) ) {
		    found = TRUE;
		    get_obj( ch, obj, NULL );
		}
	    }

	    if ( !found )     {
		if ( arg1[3] == '\0' )
		    send_to_char( "I see nothing here.\r\n", ch );
		else
		    act( "I see no $T here.", ch, NULL, &arg1[4], TO_CHAR );
	    }
	}
    } else {
	/* 'get ... container' */
	if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) ) {
	    send_to_char( "You can't do that.\r\n", ch );
	    return;
	}

	if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )	{
	    act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	    return;
	}

	switch ( container->item_type ) {
	  default:
	    send_to_char( "That's not a container.\r\n", ch );
	    return;

	  case ITEM_CONTAINER:
                  case ITEM_LOCKER:
                  case ITEM_CLAN_LOCKER:
	  case ITEM_KEY_RING:
	  case ITEM_CAMERA:

	    if ( IS_SET(container->value[1], CONT_CLOSED)
                    && IS_SET(container->value[1], CONT_CLOSEABLE) ) {
	        act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	        return;
	    }

            break;

	  case ITEM_CORPSE_NPC:
                  case ITEM_TREE:
	    break;

	  case ITEM_CORPSE_PC:

	    if (!can_loot(ch,container)) {
		send_to_char( "You can't loot that corpse.\r\n", ch );
	        return;
	    }

            break; 
	}

	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) ) {
	    /* 'get obj container' */
	    obj = get_obj_list( ch, arg1, container->contains );
	    if ( obj == NULL )
	    {
		act( "I see nothing like that in the $T.",
		    ch, NULL, arg2, TO_CHAR );
		return;
	    }
	    get_obj( ch, obj, container );
	}
	else
	{
	    /* 'get all container' or 'get all.obj container' */
	    found = FALSE;
	    for ( obj = container->contains; obj != NULL; obj = obj_next )    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) ) {
		    found = TRUE;
		    if (container->pIndexData->vnum == OBJ_VNUM_PIT
		    &&  !IS_IMMORTAL(ch))   {
			send_to_char("Don't be so greedy!\r\n",ch);
			return;
		    }
		    get_obj( ch, obj, container );
		}
	    }

	    if ( !found )    {
		if ( arg1[3] == '\0' )
		    act( "I see nothing in the $T.", ch, NULL, arg2, TO_CHAR );
		else
		    act( "I see nothing like that in the $T.", ch, NULL, arg2, TO_CHAR );
	    }
	}
    }

    return;
}


void do_weigh(CHAR_DATA *ch, char *args ) {

    OBJ_DATA *obj;

    char *pdesc;
    char buf[MAX_STRING_LENGTH];  

   /* Syntax */

    if ( args[0] == '\0'
      || args[0] == '?' ) {
      send_to_char("Syntax: weight object\r\n", ch);
      return;
    } 

   /* Find the object... */

    obj = get_obj_carry(ch, args);

    if ( obj != NULL ) { 
      pdesc = "Your";
    } else {

      obj = get_obj_here(ch, args);
  
      if ( obj != NULL ) {
        pdesc = "The";
      } else {
        
        obj = get_obj_wear(ch, args);

        if ( obj != NULL ) {
          pdesc = "Your";
        } else {
          send_to_char("You can not find one of those!\r\n", ch);
          return;
        }
      }
    }

   /* Send the data... */     

    sprintf(buf, "%s %s has a weight of %d units.\r\n", pdesc, obj->short_descr, get_obj_weight(obj));
    send_to_char(buf, ch);

   /* Capacity for containers... */

    if ( obj->item_type == ITEM_CONTAINER ) {
      sprintf(buf, "It has capacity for %ld units" " and currently contains %d units.\r\n", obj->value[0] - obj->weight, get_obj_weight(obj) - obj->weight );
      send_to_char(buf, ch);
    }

    return;
}


void do_put( CHAR_DATA *ch, char *argument ) {
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
OBJ_DATA *container;
OBJ_DATA *obj;
OBJ_DATA *obj_next;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"in"))
	argument = one_argument(argument,arg2);

    if ( arg1[0] == '\0' || arg2[0] == '\0' )    {
	send_to_char( "Put what in what?\r\n", ch );
	return;
    }

    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ))    {
	send_to_char( "You can't do that.\r\n", ch );
	return;
    }

    if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )    {
	act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	return;
    }

    if ( (container->item_type != ITEM_CONTAINER )
      && (container->item_type != ITEM_CAMERA)
      && (container->item_type != ITEM_KEY_RING)
      && (container->item_type != ITEM_LOCKER)
      && (container->item_type != ITEM_CORPSE_PC)
      && (container->item_type != ITEM_CORPSE_NPC)
      && (container->item_type != ITEM_CLAN_LOCKER)) {
	send_to_char( "That's not a container.\r\n", ch );
	return;
    }

    if ( container->item_type == ITEM_CONTAINER
      || container->item_type == ITEM_CAMERA
      || container->item_type == ITEM_LOCKER
      || container->item_type == ITEM_KEY_RING
      || container->item_type == ITEM_CLAN_LOCKER) {

      if ( IS_SET(container->value[1], CONT_CLOSED)
        && IS_SET(container->value[1], CONT_CLOSEABLE) ) {
  	act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	return;
      }
    }

    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ))   {
	/* 'put obj container' */
	if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )	{
	    send_to_char( "You do not have that item.\r\n", ch );
	    return;
	}

	if ( obj == container )	{
	    send_to_char( "You can't fold it into itself!\r\n", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )	{
	    send_to_char( "You can't let go of it!\r\n", ch );
	    return;
	}

	if ( (get_obj_weight( obj ) + get_obj_weight( container ) > container->value[0] )
                && (container->value[0] != -1)) {
	    send_to_char( "It won't fit!\r\n", ch );
	    return;
	}

                if (obj->trans_char != NULL) {
	    send_to_char( "It resists!\r\n", ch );
	    return;
	}
	
        if ( container->item_type == ITEM_KEY_RING
        && obj->item_type != ITEM_KEY
        && obj->item_type != ITEM_LOCKER_KEY) {
            send_to_char("You can't put that in there!\r\n", ch);
            return;  
        }

        if ( container->item_type == ITEM_CAMERA) {
             if (obj->item_type != ITEM_PHOTOGRAPH
             || container->contains != NULL) {
                  send_to_char("You can't put that in there!\r\n", ch);
                  return;  
             }
        }

        if ( container->item_type == ITEM_LOCKER
        || container->item_type == ITEM_CLAN_LOCKER) {
                if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)
                || obj->item_type == ITEM_PASSPORT) {
                     send_to_char("You can't put that in there!\r\n", ch);
                     return;  
                }
        }

         if (container->pIndexData->vnum == OBJ_VNUM_PIT 
         &&  !CAN_WEAR(container,ITEM_TAKE)) {
                if (obj->timer) {
	      send_to_char( "Only permanent items may go in the pit.\r\n",ch);
	      return;
	} else {
	      obj->timer = number_range(100,200);
                }
         }

       /* Set up context and get WEV... */

        mcc = get_mcc(ch, ch, NULL, NULL, obj, container, 0, NULL);
        wev = get_wev(WEV_PUT, WEV_PUT_ITEM, mcc,
                    "You put @p2 into @s2.\r\n",
                    "@a2 puts @p2 into @s2.\r\n",
                    "@a2 puts @p2 into @s2.\r\n");

       /* Challange... */

        if (!room_issue_wev_challange(ch->in_room, wev)) {
          free_wev(wev);
          return;
        }

       /* Do the transfer... */

	obj_from_char( obj );
	obj_to_obj( obj, container );

       /* Send to command issuers room... */

        room_issue_wev( ch->in_room, wev);

       /* Free the wev... */

        free_wev(wev);

    } else {
	/* 'put all container' or 'put all.obj container' */
	for ( obj = ch->carrying; obj != NULL; obj = obj_next ) {
	    obj_next = obj->next_content;

                    if (obj->trans_char != NULL) continue;

	    if ( (arg1[3] == '\0' || is_name(&arg1[4], obj->name))
	      && (can_see_obj(ch, obj))
	      && (obj->wear_loc == WEAR_NONE)
	      && (obj != container)
	      && (can_drop_obj(ch, obj))
	      && ( (get_obj_weight(obj) + get_obj_weight(container) <= container->value[0]) || (container->value[0] == -1) ))    {
	    	if ( container->pIndexData->vnum == OBJ_VNUM_PIT
	      	  && !CAN_WEAR(obj, ITEM_TAKE) ) {
	    	    if (obj->timer) continue;
	    	    else obj->timer = number_range(100,200);
                }

                if ( container->item_type == ITEM_KEY_RING
                && obj->item_type != ITEM_KEY
                && obj->item_type != ITEM_LOCKER_KEY ) continue;

                if ( container->item_type == ITEM_CAMERA) {
                       if (obj->item_type != ITEM_PHOTOGRAPH
                       || container->contains != NULL) continue;
                }

                if ( container->item_type == ITEM_LOCKER
                || container->item_type == ITEM_CLAN_LOCKER) {
                       if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)) continue;
                       if(obj->item_type == ITEM_PASSPORT) continue;
                }

                  /* Set up context and get WEV... */

                mcc = get_mcc(ch, ch, NULL, NULL, obj, container, 0, NULL);
                wev = get_wev(WEV_PUT, WEV_PUT_ITEM, mcc,
                             "You put @p2 into @s2.\r\n",
                             "@a2 puts @p2 into @s2.\r\n",
                             "@a2 puts @p2 into @s2.\r\n");

               /* Challange... */

                if (!room_issue_wev_challange(ch->in_room, wev)) {
                  free_wev(wev);
                  return;
                }

               /* Do the transfer... */

         	obj_from_char( obj );
	obj_to_obj( obj, container );

               /* Send to command issuers room... */

                room_issue_wev( ch->in_room, wev);

               /* Free the wev... */

                free_wev(wev);

	    }
	}
    }

    return;
}


void drop_obj(CHAR_DATA *ch, OBJ_DATA *obj) {
        MOB_CMD_CONTEXT *mcc;
        WEV *wev;

       /* Set up context and get WEV... */

        mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);

        if (IS_SET(obj->extra_flags,ITEM_MELT_DROP)) {
            wev = get_wev(WEV_DROP, WEV_DROP_ITEM_MELT, mcc,
                         "The @p2 dissolves into smoke when you drop it!\r\n",
                         "@a2 drops @p2 and it dissolves into smoke!\r\n",
                         "@a2 drops @p2 and it dissolves into smoke!\r\n");
        } else {
            wev = get_wev(WEV_DROP, WEV_DROP_ITEM, mcc,
                         "You drop @p2.\r\n",
                         "@a2 drops @p2.\r\n",
                         "@a2 drops @p2.\r\n");
        }

       /* Challange... */

        if (!room_issue_wev_challange(ch->in_room, wev)) {
          free_wev(wev);
          return;
        }

       /* Do the transfer... */

        obj_from_char( obj );

       /* ...but only complete it if the item dosen't melt... */

        if (!IS_SET(obj->extra_flags, ITEM_MELT_DROP)) obj_to_room( obj, ch->in_room );
        if (IS_SET(ch->in_room->room_flags, ROOM_DUMP)) obj->timer = number_range(10, UMAX(obj->level, 11));

       /* Send to command issuers room... */

        room_issue_wev( ch->in_room, wev);

       /* Free the wev... */

        free_wev(wev);

        if (obj->in_room) obj_fall(obj, 0);

       /* Lose the object if it melted... */	

        if (IS_SET(obj->extra_flags,ITEM_MELT_DROP)) extract_obj(obj);
        return;
}


void do_drop( CHAR_DATA *ch, char *argument ) {
char arg[MAX_INPUT_LENGTH];
OBJ_DATA *obj;
OBJ_DATA *obj_next;
CHAR_DATA *victim;
bool found;
int currency;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

    argument = one_argument( argument, arg );

    if (IS_AFFECTED(ch, AFF_MIST)) {
	send_to_char( "You can't do that in mist form?\r\n", ch );
	return;
    }

    if ( arg[0] == '\0' )    {
	send_to_char( "Drop what?\r\n", ch );
	return;
    }

    if (!str_cmp(arg, "money")) {
         char buf[MAX_STRING_LENGTH];
         int i;
        
         for (i = 0; i < MAX_CURRENCY; i++) {
              if (ch->gold[i] > 0) {
                  sprintf(buf, "%ld %s", ch->gold[i], flag_string(currency_accept, i));
                  do_drop(ch, buf);
              }
         }
         return;
    }

    if (is_number( arg ))    {
	/* 'drop NNNN coins' */
	int amount;

	amount   = atoi(arg);
	argument = one_argument( argument, arg );

                currency = flag_value(currency_accept, arg);

	if ( amount <= 0 || currency < 0) {
	    send_to_char( "Sorry, you can't do that.\r\n", ch );
	    return;
	}

	if ( ch->gold[currency] < amount ) {
	    sprintf_to_char(ch, "You haven't got that many %s.\r\n", flag_string(currency_type, currency));
	    return;
	}

       /* Set up context and get WEV... */

        mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, amount, flag_string(currency_type, currency));
        wev = get_wev(WEV_DROP, WEV_DROP_GOLD, mcc,
                     "You drop @n0 @t0.\r\n",
                     "@a2 drops @n0 @t0.\r\n",
                     "@a2 drops @n0 @t0.\r\n");

       /* Challange... */

        if (!room_issue_wev_challange(ch->in_room, wev)) {
          free_wev(wev); 
          return;
        }

       /* Do the transfer... */

	ch->gold[currency] -= amount;

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next ) {
	    obj_next = obj->next_content;

                    if (obj->value[1] == currency) {
	        switch ( obj->pIndexData->vnum )    {
	            case OBJ_VNUM_MONEY_ONE:
		amount += 1;
		extract_obj( obj );
		break;

	            case OBJ_VNUM_MONEY_SOME:
		amount += obj->value[0];
		extract_obj( obj );
		break;
                         }
	    }
	}

	obj_to_room(create_money(amount, currency), ch->in_room );

       /* Send to command issuers room... */

        room_issue_wev( ch->in_room, wev);

       /* Free the wev... */

        free_wev(wev);
        return;
    }

    /* Release a carried Character */

     victim = get_char_room( ch, arg );
 
     if ( victim != NULL 
       && victim->position == POS_STUNNED
       && victim->master !=NULL
       && victim->master ==ch) {
                 send_to_char("Your you drop your victim to the floor.\r\n", ch);
                 send_to_char( "You hit the floor very hard.\r\n", victim );
                 act("$n drops $n to the ground.",ch,victim,NULL,TO_ROOM);
                 stop_follower(victim);
                 ch->carry_weight -= 150;
                 return;
     }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) ) {
	/* 'drop obj' */
	obj = get_obj_carry( ch, arg );

	if ( obj == NULL ) {
	    send_to_char( "You do not have that item.\r\n", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) ) {
	    send_to_char( "You can't let go of it.\r\n", ch );
	    return;
	}

	drop_obj(ch, obj);

    } else {

	/* 'drop all' or 'drop all.obj' */
	found = FALSE;
	for ( obj = ch->carrying; obj != NULL; obj = obj_next ){
	    obj_next = obj->next_content;

	    if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   can_drop_obj( ch, obj ) ) {

		found = TRUE;

	        drop_obj(ch, obj);
	    }
	}

	if ( !found ) {
	    if ( arg[3] == '\0' )
		act( "You are not carrying anything you can drop.",  ch, NULL, arg, TO_CHAR );
	    else
		act( "You are not carrying any $T that you can drop.", ch, NULL, &arg[4], TO_CHAR );
	}
    }

    return;
}


void do_give( CHAR_DATA *ch, char *argument ){
char arg1 [MAX_INPUT_LENGTH];
char arg2 [MAX_INPUT_LENGTH];
char buf[MAX_STRING_LENGTH];
CHAR_DATA *victim;
OBJ_DATA  *obj;
int currency;
MOB_CMD_CONTEXT *mcc;
WEV *wev;  

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (IS_AFFECTED(ch, AFF_MIST)) {
	send_to_char( "You can't do that in mist form?\r\n", ch );
	return;
    }

    if ( arg1[0] == '\0' || arg2[0] == '\0' ) {
	send_to_char( "Give what to whom?\r\n", ch );
	return;
    }

    if ( is_number( arg1 ))  {
	/* 'give NNNN coins victim' */
	int amount;

	amount   = atoi(arg1);

                currency = flag_value(currency_accept, arg2);

	if ( amount <= 0 || currency < 0) {
	    send_to_char( "Sorry, you can't do that.\r\n", ch );
	    return;
	}

	argument = one_argument( argument, arg2 );
	if ( arg2[0] == '\0' ) {
	    send_to_char( "Give what to whom?\r\n", ch );
	    return;
	}

	if ( ( victim = get_char_room( ch, arg2 ) ) == NULL ) {
	    send_to_char( "They aren't here.\r\n", ch );
	    return;
	}

	if ( ch->gold[currency] < amount ) {
	    sprintf_to_char(ch, "You haven't got that much %s.\r\n", flag_string(currency_type, currency));
	    return;
	}

       /* Ok, get context and build the wev... */

        mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, amount, flag_string(currency_type, currency));
        wev = get_wev(WEV_GIVE, WEV_GIVE_GOLD, mcc,
                    "You give @n0 @t0 to @v2.\r\n",
                    "@a2 gives you @n0 @t0!\r\n",
                    "@a2 gives @v2 @n0 @t0.\r\n");

       /* Challange... */

        if (!room_issue_wev_challange(ch->in_room, wev)) {
          free_wev(wev);
          return;
        }

       /* Make the transfer... */

	ch->gold[currency]     -= amount;
	victim->gold[currency] += amount;

       /* Send to command issuers room... */

        room_issue_wev( ch->in_room, wev);

       /* Free the wev (will autofree the context)... */

        free_wev(wev);

        mprog_bribe_trigger( victim, ch, amount );
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )   {
	send_to_char( "You do not have that item.\r\n", ch );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE ) {
	send_to_char( "You must remove it first.\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL ) {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( !can_drop_obj( ch, obj ) ) {
	send_to_char( "You can't let go of it.\r\n", ch );
	return;
    }

    if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )  {
	act( "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( victim->carry_weight + get_obj_weight( obj ) > can_carry_w( victim ) )  {
	act( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( !can_see_obj( victim, obj ) ) {
	act( "$N can't see it.", ch, NULL, victim, TO_CHAR );
	return;
    }

   /* Ok, get context and build the wev... */

    mcc = get_mcc(ch, ch, victim, NULL, obj, NULL, 0, NULL);
    wev = get_wev(WEV_GIVE, WEV_GIVE_ITEM, mcc,
                "You give @p2 to @v2.\r\n",
                "@a2 gives you @p2!\r\n",
                "@a2 gives @v2 @p2.\r\n");

   /* Challange... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

   /* Make the transfer... */

    obj_from_char( obj );
    obj_to_char( obj, victim );

    if (obj->pIndexData && victim->pIndexData && !IS_NPC(ch)) {
        if (obj->pIndexData->vnum == OBJ_VNUM_QUEST_DOCUMENT) {
             if (ch->pcdata->questmob == victim->pIndexData->vnum
             && !IS_SET(victim->act, ACT_CRIMINAL)) {
                    int reward, pracreward, pointreward;
         
                    do_say(victim, "So you really got trough to me.");
                    reward = number_range(1000,10000);
                    pointreward = number_range(2,5);
                    sprintf(buf, "As a reward, I am giving you some fame, and %d {rCopper{x.",reward);
                    do_say(victim,buf);
                    if (chance(15))  {
                        pracreward = number_range(1,3);
                        sprintf(buf, "You gain %d practices!\r\n",pracreward);
                        send_to_char(buf, ch);
                        ch->practice += pracreward;
                    }
                    extract_obj(obj);
                    REMOVE_BIT(ch->plr, PLR_QUESTOR);
                    ch->pcdata->questgiver = NULL;
                    free_string(ch->pcdata->questplr);
                    ch->pcdata->questplr = strdup("");
                    ch->pcdata->countdown = 0;
                    ch->pcdata->questmob = 0;
                    ch->pcdata->questobj = 0;
                    ch->pcdata->nextquest = 30;
                    ch->gold[0] += reward;
                    ch->pcdata->questpoints += pointreward;
              }
        }
    }

   /* Send to command issuers room... */

    room_issue_wev( ch->in_room, wev);

   /* Free the wev (will autofree the context)... */

    free_wev(wev);

    mprog_give_trigger( victim, ch, obj );
    return;
}


void do_fill( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
     

    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

   /* What do they want to fill? */

    one_argument( argument, arg );

    if ( arg[0] == '\0' ) {
	send_to_char( "Fill what?\r\n", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL ) {
	send_to_char( "You do not have that item.\r\n", ch );
	return;
    }

    if (obj->item_type != ITEM_DRINK_CON
    && (obj->item_type != ITEM_LIGHT || obj->value[1] == 0)) {
	send_to_char( "You can't fill that.\r\n", ch );
	return;
    }

    if (obj->item_type == ITEM_LIGHT) {
        OBJ_DATA *refill;
        int capacity = UMAX(obj->pIndexData->value[2], obj->pIndexData->value[4]);

        for (refill = ch->carrying; refill; refill = refill->next_content) {
               if (refill->item_type == ITEM_LIGHT_REFILL && refill->value[1] == obj->value[1]) break;
        }

        if (!refill) {
	send_to_char( "You have nothing to refill it.\r\n", ch );
	return;
        }

        if (obj->value[2] > refill->value[2] || obj->value[2] < 0) {
	send_to_char( "No need to refill it yet.\r\n", ch );
	return;
        }

        mcc = get_mcc(ch, ch, NULL, NULL, obj, refill, obj->value[1], flag_string(refill_type, obj->value[1]));
        wev = get_wev(WEV_FILL, WEV_FILL_LIGHT, mcc,
                "You refill @p2 with @s2.\r\n",
                "@a2 refills @p2 with @s2.\r\n",
                "@a2 refills @p2 with @s2.\r\n");

        if (!room_issue_wev_challange(ch->in_room, wev)) {
            free_wev(wev);
            return;
        }

        obj->value[2] = refill->value[2];
        if (obj->pIndexData) {
             if (obj->value[2] > capacity) obj->value[2] = capacity;
        }

        obj_from_char(refill);
        extract_obj(refill);

        room_issue_wev( ch->in_room, wev);
        free_wev(wev);
        return;

    } else {
        OBJ_DATA *fountain;

        if ( obj->value[1] >= obj->value[0] ) {

            if ( obj->value[2] >= 0
            && obj->value[2] < LIQ_MAX) {
                  sprintf(buf, "Your container already holds %s fluid.\r\n", liq_table[obj->value[2]].liq_color);
            } else {
                  sprintf(buf, "Your container already holds a strange fluid!\r\n");
            } 

            send_to_char( buf, ch );
            return;
        }

        fountain = ch->in_room->contents; 
 
        while ( fountain != NULL 
        && fountain->item_type != ITEM_FOUNTAIN ) {
            fountain = fountain->next_content;
        }

        if (fountain == NULL) {
	send_to_char( "There is no fountain here!\r\n", ch );
	return;
        }

        if ( fountain->value[0] < 0 
        || fountain->value[0] >= LIQ_MAX) {
            send_to_char("You don't want a bottle of that!\r\n", ch);
            return;
        } 

        if ( obj->value[1] != 0 
        && obj->value[2] != 0 ) {

            if ( obj->value[2] > 0
            && obj->value[2] < LIQ_MAX) {
                   sprintf(buf, "Your container already holds %s fluid.\r\n", liq_table[obj->value[2]].liq_color);
            } else {
                   sprintf(buf, "Your container already holds a strange fluid!\r\n");
            } 

            send_to_char( buf, ch );
            return;
        }

        mcc = get_mcc(ch, ch, NULL, NULL, obj, fountain, 0, liq_table[fountain->value[0]].liq_name);
        wev = get_wev(WEV_FILL, WEV_FILL_FOUNTAIN, mcc,
                "You fill @p2 with @t0 from @s2.\r\n",
                "@a2 fills @p2 from @s2.\r\n",
                "@a2 fills @p2 from @s2.\r\n");

        if (!room_issue_wev_challange(ch->in_room, wev)) {
            free_wev(wev);
            return;
        }

        obj->value[1] = obj->value[0];
        obj->value[2] = fountain->value[0];
        obj->value[4] = fountain->value[3];

        if (obj->value[3] == 0) {
           if ( fountain->value[1] == 1 ) {
               if (number_open() > 100) {
                    fountain->value[1] = 1;
               } else {
                    obj->value[3] = fountain->value[1];
               }
           }
        } else {
           if ( fountain->value[1] == 0 ) {
               if (number_open() > 100) {
                    fountain->value[1] = 1;
               }
           }
        }

        room_issue_wev( ch->in_room, wev);
        free_wev(wev);
        return;
    }
}


void do_pour( CHAR_DATA *ch, char * argument ) {
    char buf[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *to_obj;
    int translevel;

   /* What do they want to fill? */

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg1 );

    if ( arg[0] == '\0' ) {
	send_to_char( "Pour what?\r\n", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL ) {
	send_to_char( "You do not have that item.\r\n", ch );
	return;
    }

    if (obj->item_type != ITEM_DRINK_CON 
    && obj->item_type != ITEM_POTION ) {
	send_to_char( "You can't pour that anywhere.\r\n", ch );
	return;
    }

    if (!str_cmp(arg1, "out")
    || arg1[0] =='\0') {
        if (obj->item_type == ITEM_DRINK_CON) {
            if (obj->value[1] == 0) {
                send_to_char( "It's already empty.\r\n", ch );
                return;
            }
            obj->value[1]=0;
            act( "$n pours $p out.",ch, obj, NULL, TO_ROOM );
            send_to_char( "You pour it out.\r\n", ch );

        } else if (obj->item_type == ITEM_POTION) {
            if (obj->value[1] == 0
            && obj->value[2] == 0
            && obj->value[3] == 0) {
                send_to_char( "It's already empty.\r\n", ch );
                return;
            }
            obj->value[0] = 0;
            obj->value[1] = 0;
            obj->value[2] = 0;
            obj->value[3] = 0;
            act( "$n pours $p out.",ch, obj, NULL, TO_ROOM );
            send_to_char( "You pour it out.\r\n", ch );
            free_string(obj->name);
            obj->name = str_dup("vial empty");
            free_string(obj->short_descr);
            obj->short_descr = str_dup("an empty vial");
            free_string(obj->description);
            obj->description = str_dup("An empty vial lies here.\n");
        }

    } else {
       if (obj->item_type != ITEM_POTION) {
	send_to_char( "The concentration in your drink container is much too low.\r\n", ch );
	return;
        }
     
        if ((to_obj = get_obj_here(ch, arg1) ) == NULL ) {
	send_to_char( "Where do you want to pour it in?\r\n", ch );
	return;
        }

        if (to_obj->item_type != ITEM_DRINK_CON 
        && to_obj->item_type != ITEM_FOUNTAIN ) {
	send_to_char( "You can't pour your potion into that.\r\n", ch );
	return;
        }

        if (obj->value[1] == 0
        && obj->value[2] == 0
        && obj->value[3] == 0) {
	send_to_char( "This vial is empty.\r\n", ch );
	return;
        }

        sprintf(buf, "$n pours $p into %s.", to_obj->short_descr);
        act(buf, ch, obj, NULL, TO_ROOM );
        sprintf(buf, "You pour $p into %s.", to_obj->short_descr);
        act(buf, ch, obj, NULL, TO_CHAR);

        if (to_obj->item_type == ITEM_FOUNTAIN) {
                translevel = obj->value[0]/25;
                if (translevel < to_obj->value[2]) {
                      send_to_char("The concentration seems to be too low.\r\n", ch);
                } else {
                      send_to_char("The color of the liquit changes a tiny bit.\r\n", ch);
                      to_obj->value[3] = obj->value[1];
                      to_obj->value[2] = translevel;
                }
        } else if (to_obj->item_type == ITEM_DRINK_CON) {
                if (to_obj->value[4] > 0) {
                      send_to_char("The concentration seems to be too low.\r\n", ch);
                } else {
                      send_to_char("The color of the liquit changes a tiny bit.\r\n", ch);
                      to_obj->value[4] = obj->value[1];
                }
        }
        obj->value[0] = 0;
        obj->value[1] = 0;
        obj->value[2] = 0;
        obj->value[3] = 0;
        free_string(obj->name);
        obj->name = str_dup("vial empty");
        free_string(obj->short_descr);
        obj->short_descr = str_dup("an empty vial");
        free_string(obj->description);
        obj->description = str_dup("An empty vial lies here.\n");
    }     

    if (obj->condition < 10) {
         obj_from_char(obj);
         extract_obj(obj);
    }
    return;
}


void do_drink( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int amount;
    int liquid;
    int aff_liq;
    bool poisoned;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

    wev = NULL; 

   /* Hunt the object time... */

    one_argument( argument, arg );

    if ( arg[0] == '\0' ) {
	for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content ) {
	    if ( obj->item_type == ITEM_FOUNTAIN || obj->item_type == ITEM_POOL) break;
                }

	if ( obj == NULL ) {
	    send_to_char( "Drink what?\r\n", ch );
	    return;
	}

       /* Work out what we're drinking... */

        liquid = obj->value[0];
        poisoned = (obj->value[1] != 0);
        amount = 0;

    } else {

	obj = get_obj_here( ch, arg );

	if ( obj == NULL ) {
	    send_to_char( "You can't find it.\r\n", ch );
	    return;
	}

       /* Check that it is a bottle... */

        if ( obj->item_type != ITEM_DRINK_CON
        && obj->item_type != ITEM_FOUNTAIN 
        && obj->item_type != ITEM_POOL 
        && obj->pIndexData->vnum != 54) {
          send_to_char( "You can't drink from that.\r\n", ch );
          return;
        }

        if (obj->condition < 5) {
             send_to_char( "It's much too damaged.\r\n", ch );
             return;
        }

       /* Check that there's something in it... */

        if (obj->item_type == ITEM_DRINK_CON) {
          amount = obj->value[1];
    	  if ( amount <= 0 ) {
  	      send_to_char( "It is already empty.\r\n", ch );
	      return;
   	  }

         /* Work out what we're drinking from a bottle... */

          liquid = obj->value[2];
          poisoned = (obj->value[3] != 0);

        } else if (obj->item_type == ITEM_FOUNTAIN || obj->item_type == ITEM_POOL) {
         /* Work out what we're drinking from a fountain... */

          liquid = obj->value[0];
          poisoned = (obj->value[1] != 0);
          amount = 0;
        } else {
         /* Licking blood */

          if (is_affected(ch, gafn_poison)) {
              send_to_char("You don't want to drink that horrible stuff again!\r\n", ch);
              return;
          }

          liquid = LIQ_BLOOD;
          poisoned = TRUE;
          amount = 0;
        }
    }

   /* Check the liquid is valid... */

    if ( liquid < 0 
      || liquid >= LIQ_MAX ) {
          send_to_char("Looks like it's gone off!\r\n", ch);
          return;
    }

   /* To drunk to drink? */

    if ( !IS_NPC(ch) 
      && ch->condition[COND_DRUNK] > 10 + number_range(0,15)) {
	send_to_char( "You fail to reach your mouth.  *Hic*\r\n", ch );
	return;
    }

   /* Got any room? */

    if ( !IS_NPC(ch)
      && ch->condition[COND_DRINK] > 600) {
      send_to_char("You're not thirsty!\r\n", ch);
      return;
    }

   /* Ok, get context and build the wev... */

    mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, amount - 1, liq_table[liquid].liq_name);

   /* What's it like going down... */

    aff_liq = liquid;

    if ( liq_table[aff_liq].immune != 0 
      && IS_SET(ch->imm_flags, liq_table[aff_liq].immune) ) {
      aff_liq = LIQ_WATER;
    }

    if ( liq_table[aff_liq].food != 0 
      && IS_SET(ch->imm_flags, liq_table[aff_liq].food) ) {
      aff_liq = LIQ_JUICE;
    }

    /*Water creatures like salt water too*/
     if (IS_AFFECTED(ch, AFF_SWIM)
     && IS_AFFECTED(ch, AFF_WATER_BREATHING)
     && aff_liq == LIQ_SALT_WATER) {
              aff_liq = LIQ_WATER;
              send_to_char("{cYou're used to salt-water.{x\r\n", ch);
      }

    /*Drinking  Blood*/
     if (!IS_NPC(ch)
     && IS_SET(ch->act, ACT_VAMPIRE)
     && aff_liq == LIQ_BLOOD) {
              send_to_char("{rYou're used to fresh blood.{x\r\n", ch);
              ch->pcdata->blood = UMIN(ch->pcdata->blood+10, ch->level);
              if (is_affected(ch, gafn_frenzy)) {
                    if (check_dispel(255,ch, gafn_frenzy)) {
                           send_to_char ("You really needed that drink...\r\n",ch);
                           REMOVE_BIT(ch->parts, PART_FANGS);
                    }
              }
      }


   /* Work out the wev's... */

    if (obj->item_type == ITEM_FOUNTAIN || obj->item_type == ITEM_POOL) {

      if (liq_table[aff_liq].liq_affect[COND_DRINK] > 0) {
        wev = get_wev(WEV_DRINK, WEV_DRINK_FOUNTAIN_OK, mcc,
                     "You drink lots of @t0 from @p2.\r\n",
                     "@a2 drinks a lot from @p2.\r\n",
                     "@a2 drinks a lot from @p2.\r\n");
      } else {
        wev = get_wev(WEV_DRINK, WEV_DRINK_FOUNTAIN_BAD, mcc,
                     "You drink some @t0 from @p2! Yuck!\r\n",
                     "@a2 drinks from @p2 and wishes @a5 hadn't!\r\n",
                     "@a2 drinks from @p2 and wishes @a5 hadn't!\r\n");
      }
    } 

    if (obj->item_type == ITEM_DRINK_CON) {

      if (liq_table[aff_liq].liq_affect[COND_DRINK] > 0) {

        wev = get_wev(WEV_DRINK, WEV_DRINK_ITEM_OK, mcc,
                     "You drink @t0 from @p2.\r\n",
                     "@a2 drinks from @p2.\r\n",
                     "@a2 drinks from @p2.\r\n");

      } else {

        wev = get_wev(WEV_DRINK, WEV_DRINK_ITEM_BAD, mcc,
                     "You drink some @t0 from @p2! Yuck!\r\n",
                     "@a2 drinks from @p2 and wishes @a5 hadn't!\r\n",
                     "@a2 drinks from @p2 and wishes @a5 hadn't!\r\n");
      }
    }

    if (obj->item_type == ITEM_TRASH) {
        wev = get_wev(WEV_DRINK, WEV_DRINK_ITEM_BAD, mcc,
                     "You lick stained blood from @p2!\r\n",
                     "@a2 licks stained blood from @p2 and wishes @a5 hadn't!\r\n",
                     "@a2 licks stained blood from @p2 and wishes @a5 hadn't!\r\n");
    }

    

   /* Challange... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    room_issue_wev( ch->in_room, wev);
    free_wev(wev);

   /* Ok, do the drinking... */

    if (obj->item_type == ITEM_FOUNTAIN) {
          if (obj->value[3] != EFFECT_UNDEFINED) obj_cast_spell( obj->value[3], obj->value[2]+1, ch, ch, obj, NULL );
          if ( !IS_NPC(ch) ) {
                if ( ch->condition[COND_FOOD] >= COND_STUFFED 
                && liq_table[aff_liq].liq_affect[COND_FOOD] > 0 ) {
                        gain_condition( ch, COND_FAT, 1 );
                }

                if (liq_table[aff_liq].liq_affect[COND_DRINK] > 0) {

  	    amount = number_range(1, 3);
                    if (!IS_SET(ch->act, ACT_VAMPIRE)) {
    	      gain_condition(ch, COND_FOOD,  27 * amount * liq_table[aff_liq].liq_affect[COND_FOOD]);
	      gain_condition(ch, COND_DRINK,  27 * amount * liq_table[aff_liq].liq_affect[COND_DRINK]);
                    }
	    gain_condition(ch, COND_DRUNK,  amount * liq_table[aff_liq].liq_affect[COND_DRUNK]);
                } else {
                    if (!IS_SET(ch->act, ACT_VAMPIRE)) {
   	       gain_condition(ch, COND_FOOD,  27 * liq_table[aff_liq].liq_affect[COND_FOOD]);
	       gain_condition(ch, COND_DRINK,  27 * liq_table[aff_liq].liq_affect[COND_DRINK]);
                    }
	    gain_condition(ch, COND_DRUNK,  liq_table[aff_liq].liq_affect[COND_DRUNK]);
                }
          } 
    }

    if (obj->item_type == ITEM_POOL) {
          int avail, need;

          switch(obj->value[2]) {
                default:
                    break;

                case POOL_HEALTH:
                    need = ch->max_hit - ch->hit;
                    avail = obj->value[3] /4;
                    if (need > avail) {
                        ch->hit +=avail;
                        obj->value[3] -=avail;
                    } else {
                        ch->hit += need;
                        obj->value[3] -= need;
                    }
                    break;

                case POOL_MOVES:
                    need = ch->max_move - ch->move;
                    avail = obj->value[3] /4;
                    if (need > avail) {
                        ch->move +=avail;
                        obj->value[3] -=avail;
                    } else {
                        ch->move += need;
                        obj->value[3] -= need;
                    }
                    break;

                case POOL_MAGIC:
                    need = ch->max_mana - ch->mana;
                    avail = obj->value[3] /4;
                    if (need > avail) {
                        ch->mana +=avail;
                        obj->value[3] -=avail;
                    } else {
                        ch->mana += need;
                        obj->value[3] -= need;
                    }
                    break;
          }

          if ( !IS_NPC(ch) ) {
                if ( ch->condition[COND_FOOD] >= COND_STUFFED 
                && liq_table[aff_liq].liq_affect[COND_FOOD] > 0 ) {
                        gain_condition( ch, COND_FAT, 1 );
                }

                if (liq_table[aff_liq].liq_affect[COND_DRINK] > 0) {

  	    amount = number_range(1, 3);
                    if (!IS_SET(ch->act, ACT_VAMPIRE)) {
    	      gain_condition(ch, COND_FOOD,  27 * amount * liq_table[aff_liq].liq_affect[COND_FOOD]);
	      gain_condition(ch, COND_DRINK,  27 * amount * liq_table[aff_liq].liq_affect[COND_DRINK]);
                    }
	    gain_condition(ch, COND_DRUNK,  amount * liq_table[aff_liq].liq_affect[COND_DRUNK]);
                } else {
                    if (!IS_SET(ch->act, ACT_VAMPIRE)) {
   	       gain_condition(ch, COND_FOOD,  27 * liq_table[aff_liq].liq_affect[COND_FOOD]);
	       gain_condition(ch, COND_DRINK,  27 * liq_table[aff_liq].liq_affect[COND_DRINK]);
                    }
	    gain_condition(ch, COND_DRUNK,  liq_table[aff_liq].liq_affect[COND_DRUNK]);
                }
          } 
    }

   /* But only a little from a bottle... */
    if (obj->item_type == ITEM_DRINK_CON) {
         if (obj->value[4] != EFFECT_UNDEFINED) obj_cast_spell( obj->value[4], obj->level/2 +1, ch, ch, obj, NULL );

         if ( !IS_NPC(ch) ) {
              if ( ch->condition[COND_FOOD] >= COND_STUFFED 
              && liq_table[aff_liq].liq_affect[COND_FOOD] > 0 ) {
                   gain_condition( ch, COND_FAT, 1 + (obj->value[0]/10) );
              }

              if (!IS_SET(ch->act, ACT_VAMPIRE)) {
                 gain_condition(ch, COND_FOOD,  27 * liq_table[aff_liq].liq_affect[COND_FOOD]);
                 gain_condition(ch, COND_DRINK, 27 * liq_table[aff_liq].liq_affect[COND_DRINK]);
              }
              gain_condition(ch, COND_DRUNK,  liq_table[aff_liq].liq_affect[COND_DRUNK]);
         }

         obj->value[1] -= 1;
         if (obj->value[1] == 0) obj->value[4] = 0;
    }

   /* Let the poison take affect... */

    if (obj->item_type == ITEM_TRASH) {
         if ( !IS_NPC(ch) ) {
              if ( ch->condition[COND_FOOD] >= COND_STUFFED 
              && liq_table[aff_liq].liq_affect[COND_FOOD] > 0 ) {
                   gain_condition( ch, COND_FAT, 1 + (obj->value[0]/10) );
              }

              if (!IS_SET(ch->act, ACT_VAMPIRE)) {
                 gain_condition(ch, COND_FOOD,  27 * liq_table[aff_liq].liq_affect[COND_FOOD]);
                 gain_condition(ch, COND_DRINK, 27 * liq_table[aff_liq].liq_affect[COND_DRINK]);
              }
              gain_condition(ch, COND_DRUNK,  liq_table[aff_liq].liq_affect[COND_DRUNK]);
         }

         obj_from_room(obj);
         extract_obj(obj);
    }
 
    if (poisoned) {
	   
      AFFECT_DATA af;

      af.afn       = gafn_poison;
      af.type      = gsn_poison;
      af.level	   = number_range(4, obj->level+5); 
      af.duration  = 3 * number_range(5, 20);
      af.location  = APPLY_NONE;
      af.modifier  = 0;
      af.bitvector = AFF_POISON;
      affect_join( ch, &af );

      act( "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You choke and gag.\r\n", ch );
    }

    return;
}


void do_eat( CHAR_DATA *ch, char *argument ) {
char arg[MAX_INPUT_LENGTH];
OBJ_DATA *obj = NULL;
AFFECT_DATA *pAf;
AFFECT_DATA af;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

    bool poisoned = FALSE;

    int i;

    int spell[4] = { 0, 0, 0, 0};

    wev = NULL; 

   /* What are we trying to eat? */

    one_argument( argument, arg );

    if ( arg[0] == '\0' ) {
               for (obj = ch->carrying; obj; obj = obj->next_content) {
                     if (obj->item_type == ITEM_FOOD) break;
               }
               if (!obj) {
  	   send_to_char( "Eat what?\r\n", ch );
	   return;
               }
    }

   /* Find food item in inventory... or on the floor */ 

    if (!obj) {
        if ((obj = get_obj_here(ch, arg)) == NULL) {
	send_to_char( "You do not have that item.\r\n", ch );
	return;
        }
    }

   /* Check it's edible... */

    if ( obj->item_type != ITEM_FOOD 
      && obj->item_type != ITEM_PILL 
      && obj->item_type != ITEM_HERB 
      && obj->item_type != ITEM_CORPSE_NPC) {
        send_to_char( "That's not edible.\r\n", ch );
        return;
    }

   /* See if the player has room... */

    if ( !IS_IMMORTAL(ch)
    && !IS_NPC(ch) 
    && ch->condition[COND_FOOD] >= COND_OVER_STUFFED ) {
        send_to_char( "You are too full to eat any more.\r\n", ch );
        return;
    }

   /* Ok, get context and build the wev... */

    mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);

   /* Work out the wevs... */

    switch (obj->item_type) { 

      case ITEM_FOOD:
 
        poisoned = (obj->value[3] != 0);
        if (!poisoned) {
          wev = get_wev(WEV_EAT, WEV_EAT_FOOD_OK, mcc,
                       "You eat @p2.\r\n",
                       "@a2 eats @p2.\r\n",
                       "@a2 eats @p2.\r\n");
        } else {
          wev = get_wev(WEV_EAT, WEV_EAT_FOOD_BAD, mcc,
                       "You eat @p2.\r\n",
                       "@a2 eats @p2.\r\n",
                       "@a2 eats @p2.\r\n");
        }
      
        break;

      case ITEM_PILL:

          wev = get_wev(WEV_EAT, WEV_EAT_PILL, mcc,
                       "You pop @p2.\r\n",
                       "@a2 pops @p2.\r\n",
                       "@a2 pops @p2.\r\n");

          break;

      case ITEM_HERB:

          poisoned = (obj->value[2] != 0);
          wev = get_wev(WEV_EAT, WEV_EAT_HERB, mcc,
                       "You chew @p2.\r\n",
                       "@a2 chews @p2.\r\n",
                       "@a2 chews @p2.\r\n");

          break;

      case ITEM_CORPSE_NPC:

          if (str_cmp(race_array[ch->race].name,"deep one")) {
               send_to_char( "You don't feel like cannibalizing that corpse.\r\n", ch );
               return;
          }

          if (obj->contains != NULL) {
               send_to_char( "You don't want to eat all the stuff on that corpse.\r\n", ch );
               return;
          }

          poisoned = FALSE;
          wev = get_wev(WEV_EAT, WEV_EAT_CORPSE, mcc,
                       "You cannibalize a @p2.\r\n",
                       "@a2 cannibalizes a @p2.\r\n",
                       "@a2 cannibalizes a @p2.\r\n");

          break;

      default:
    
        bug("Wierd food found!", 0); 
        free_mcc(mcc);
        return;

    }        

   /* Challange... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    room_issue_wev( ch->in_room, wev);
    free_wev(wev);

   /* Ok, allowed to eat it... */

    switch ( obj->item_type ) {
    
      case ITEM_FOOD:

                if (obj->value[2] != EFFECT_UNDEFINED) {
                        if (obj->value[2] == get_effect_efn("oracle")) {
                              sprintf_to_char(ch, "%s contains a scrap of paper. It reads:\r\n", capitalize(obj->short_descr));
                        }
                        obj_cast_spell( obj->value[2], obj->value[1], ch, ch, obj, NULL );
                }
	if ( !IS_NPC(ch) ) {
                      if ( ch->condition[COND_FOOD] >= COND_STUFFED ) {
                            gain_condition( ch, COND_FAT, 1 + (obj->value[0]/10) );
                      }
  	      gain_condition( ch, COND_FOOD, 27 * obj->value[0] );
	}

        break;

      case ITEM_CORPSE_NPC:

	if ( !IS_NPC(ch) ) {
                      if ( ch->condition[COND_FOOD] >= COND_STUFFED ) {
                            gain_condition( ch, COND_FAT, 2 );
                      }
  	      gain_condition( ch, COND_FOOD, 270 );
	}

                for (pAf = obj->affected; pAf != NULL; pAf = pAf->next) {
                           if (IS_AFFECTED(ch, pAf->bitvector)) continue;

                           af.type	    = pAf->type;
                           af.afn	    = pAf->afn;
                           af.level	    = obj->level/2;
                           af.duration     = obj->level / number_range(5, 15) +1;
                           af.location      = pAf->location;
                           af.modifier      = pAf->modifier; 
                           af.bitvector    = pAf->bitvector;
                           affect_to_char(ch, &af);
                }

        break;

      case ITEM_PILL:
        spell[0] = obj->value[0];
        spell[1] = obj->value[1];
        spell[2] = obj->value[2];
        spell[3] = obj->value[3];
	break;

      case ITEM_HERB:
        spell[0] = obj->value[0];
        spell[1] = obj->value[1];
        spell[2] = obj->value[2];
	break;
    }


   /* Apply poison... */

    if (poisoned) {
        /* The shit was poisoned! */
	 AFFECT_DATA af;

	 act( "$n chokes and gags.", ch, 0, 0, TO_ROOM );
	 send_to_char( "That didn't taste so good...\r\n", ch );

                 if (obj->item_type == ITEM_HERB ) {
               	      af.type      	= gsn_poison;
	      af.afn       	= gafn_poison;
	      af.level	= obj->level;
	      af.duration  	= obj->level/2;
	      af.location  	= APPLY_STR;
	      af.modifier 	= -obj->level/10;
	      af.bitvector 	= AFF_POISON;
	      affect_join( ch, &af );
                 } else {
               	      af.type      	= gsn_poison;
	      af.afn       	= gafn_poison;
	      af.level	= number_range(3,8);
	      af.duration  	= 2 * number_range(2, 24);
	      af.location  	= APPLY_NONE;
	      af.modifier 	= 0;
	      af.bitvector 	= AFF_POISON;
	      affect_join( ch, &af );
                 }
    }

   /* Cast imbedded spells... */

    if (spell[0] > 0) {
      for(i=1; i < 4; i++) {
        if (spell[i] > 0) {
          obj_cast_spell( spell[i], spell[0], ch, ch, obj, NULL );
        }
      }
    }

   /* Lose the object... */
    extract_obj( obj );

    return;
}



/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace ) {
    OBJ_DATA *obj;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

    if ((obj = get_eq_char( ch, iWear )) == NULL ) return TRUE;

    if ( !fReplace ) return FALSE;

    if ( IS_SET(obj->extra_flags, ITEM_NOREMOVE))   {
	act( "You can't remove $p.", ch, obj, NULL, TO_CHAR );
	return FALSE;
    }

    if (obj->item_type == ITEM_CAMERA) {
         obj->value[4] = 0;
         if (ch->activity == ACV_PHOTO) set_activity(ch, POS_STANDING, NULL, ACV_NONE, NULL);
    }

    mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);
    wev = get_wev(WEV_OPROG, WEV_OPROG_REMOVE, mcc,
                     NULL,
                     NULL,
                     NULL);

    if (!room_issue_wev_challange(get_room_index(mud.monitor), wev)) {
        room_issue_wev( ch->in_room, wev);
        free_wev(wev);
        return FALSE;
    }

    room_issue_wev( ch->in_room, wev);
    free_wev(wev);

    act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
    act( "You stop using $p.", ch, obj, NULL, TO_CHAR );
    if (obj->item_type == ITEM_LIGHT
    && obj->value[0] == FALSE && obj->value[3] == TRUE
    && (obj->value[2] != -1 && obj->value[2] < 999)) {
          light_obj(ch, obj, FALSE);
    } 
    unequip_char( ch, obj );
    return TRUE;
}



/*
 * Wear one object.
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, int slot) {
MOB_CMD_CONTEXT *mcc;
WEV *wev;
char buf[MAX_STRING_LENGTH];
int tech;
    
    if ( (ch->level) < obj->level - 10 )    {
	sprintf( buf, "You must be level %d to use this object.\r\n",    obj->level - 10 );
	send_to_char( buf, ch );
	act( "$n tries to use $p, but is too inexperienced.",    ch, obj, NULL, TO_ROOM );
	return;
    }

    if (IS_SET(obj->extra_flags, ITEM_HYPER_TECH)) {
          tech = (get_skill(ch, gsn_yuggoth) + ch->perm_stat[STAT_INT] + ch->level)/4;
          if (tech < obj->level) {
	send_to_char("You don't quite understand this object.\r\n", ch );
	act( "$n tries to use $p, but doesn't know how.", ch, obj, NULL, TO_ROOM );
	return;
          }
    }

    mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);

    if (obj->pIndexData
    && obj->pIndexData->wear_cond) {
        if (!ec_satisfied_mcc(mcc, obj->pIndexData->wear_cond, TRUE)) {
            free_mcc(mcc);
            sprintf_to_char(ch, "You are unable to use %s.\r\n", obj->short_descr );
            act( "$n tries to use $p, but fails.", ch, obj, NULL, TO_ROOM );
            return;
        }
    }

    wev = get_wev(WEV_OPROG, WEV_OPROG_WEAR, mcc, NULL, NULL, NULL);

    if (!room_issue_wev_challange(get_room_index(mud.monitor), wev)) {
        free_wev(wev);
        return;
    }

    room_issue_wev( ch->in_room, wev);
    free_wev(wev);


   if (slot == WEAR_WIELD) {
        if (!wear_obj_size(ch, obj)) return;

        if ( CAN_WEAR( obj, ITEM_WIELD ) 
         && ch->limb[3]<9) {
	int sn,skill;

	if ( !remove_obj( ch, WEAR_WIELD, fReplace ))   return;

               if (IS_SET(ch->act,ACT_WERE)
               && IS_AFFECTED(ch, AFF_POLY)) {
 	    send_to_char("You don't need that!.\r\n",ch);
                    return;
               }

               if (IS_SET(ch->form, FORM_NOWEAPON)) {
 	    send_to_char("You don't need that!\r\n",ch);
                    return;
               }

	if ( !IS_NPC(ch) && ch->pc_class == class_lookup("monk"))	{
	    send_to_char( "Your code does not allow you to take up a weapon\r\n", ch );
	    return;
	}

	if ( !IS_NPC(ch) 
	&& get_obj_weight(obj ) > str_app[get_curr_stat(ch,STAT_STR)].wield ){
	    send_to_char( "It is too heavy for you to wield.\r\n", ch );
	    return;
	}

	if (!IS_NPC(ch) && get_char_size(ch) < SIZE_LARGE 
	&&  IS_SET(obj->wear_flags, ITEM_TWO_HANDS)
 	&&  get_eq_char(ch,WEAR_SHIELD) != NULL
	&&  get_eq_char(ch,WEAR_WIELD2) != NULL) {
	    send_to_char("You need two hands free for that weapon.\r\n",ch);
	    return;
	}

	act( "$n wields $p.", ch, obj, NULL, TO_ROOM );
	act( "You wield $p.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WIELD );

                sn = get_weapon_sn(ch, FALSE);
	if (sn == gsn_hand_to_hand) return;

               skill = get_weapon_skill(ch,sn);
 
               if (skill >= 100) 	act("$p feels like a part of you!",ch,obj,NULL,TO_CHAR);
               else if (skill > 85) 	act("You feel quite confident with $p.",ch,obj,NULL,TO_CHAR);
               else if (skill > 70) 	act("You are skilled with $p.",ch,obj,NULL,TO_CHAR);
               else if (skill > 50) 	act("Your skill with $p is adequate.",ch,obj,NULL,TO_CHAR);
               else if (skill > 25) 	act("$p feels a little clumsy in your hands.",ch,obj,NULL,TO_CHAR);
               else if (skill > 1) 	act("You fumble and almost drop $p.",ch,obj,NULL,TO_CHAR);
               else 		act("You don't even know which is end is up on $p.", ch,obj,NULL,TO_CHAR);

       /* Autoload and clear the gun when you wield it... */ 

               if ( (skill > 0) 
               && ( (obj->value[0] == WEAPON_GUN)
                   || (obj->value[0] == WEAPON_HANDGUN)
                   || (obj->value[0] == WEAPON_BOW)
                   || (obj->value[0] == WEAPON_SMG))) reload_gun (ch,obj, 0, TRUE);
          
                return;
        }

        if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) 
        && ch->limb[2]<9) {
	OBJ_DATA *weapon;

	if (!remove_obj( ch, WEAR_SHIELD, fReplace )) return;

               if (IS_SET(ch->act,ACT_WERE)
               && IS_AFFECTED(ch, AFF_POLY)) {
 	    send_to_char("You don't need that!\r\n",ch);
                    return;
               }

               if (IS_SET(ch->form, FORM_NOWEAPON)) {
 	    send_to_char("You don't need that!\r\n",ch);
                    return;
               }

	weapon = get_eq_char(ch,WEAR_WIELD);
	if (weapon != NULL && get_char_size(ch) < SIZE_LARGE 
	&&  IS_SET(weapon->wear_flags, ITEM_TWO_HANDS)) {
	    send_to_char("Your hands are tied up with your weapon!\r\n",ch);
	    return;
	}

	weapon = get_eq_char(ch,WEAR_WIELD2);
	if (weapon != NULL) {
		send_to_char("You cannot use a shield while dual wielding!\r\n",ch);
		return;
	}
		
	if ( ch->pc_class == class_lookup("monk") )	{
	    send_to_char("A shield would impair your fighting style too much.\r\n",ch);
	    return;
	}

	act( "$n wears $p as a shield.", ch, obj, NULL, TO_ROOM );
	act( "You wear $p as a shield.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_SHIELD );
	return;
        }
    }

    if (slot == WEAR_HOLD) {

        if ( CAN_WEAR( obj, ITEM_HOLD ) 
        && ch->limb[2]<9) {

                if (!wear_obj_size(ch, obj)) return;
	if ( !remove_obj( ch, WEAR_HOLD, fReplace ))  return;
	act( "$n holds $p in $s hands.",   ch, obj, NULL, TO_ROOM );
	act( "You hold $p in your hands.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HOLD );
	return;
        }

        if ( obj->item_type == ITEM_PRIDE )    {
                if (!wear_obj_size(ch, obj)) return;
	if ( !remove_obj( ch, WEAR_PRIDE, fReplace ) )  return;
	act( "$n wears $p with pride.", ch, obj, NULL, TO_ROOM );
	act( "You wear $p with pride.",  ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_PRIDE );
	return;
        }

        if ( obj->item_type == ITEM_LIGHT )    {
	if ( !remove_obj( ch, WEAR_LIGHT, fReplace)) return;
	act( "$n holds $p.", ch, obj, NULL, TO_ROOM );
	act( "You hold $p.",  ch, obj, NULL, TO_CHAR );
                if (obj->value[0] == FALSE && obj->value[3] == FALSE
                && (obj->value[2] != -1 && obj->value[2] < 999)) {
                      light_obj(ch, obj, TRUE);
                } 
	equip_char( ch, obj, WEAR_LIGHT );
	return;
        }

        if ( CAN_WEAR( obj, ITEM_WEAR_FLOAT ) )    {
	if ( !remove_obj( ch, WEAR_FLOAT, fReplace ))  return;
	act( "$p starts floating near $n.",   ch, obj, NULL, TO_ROOM );
	act( "$p starts floating beside you.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_FLOAT );
	return;
        }

    }

    if ( obj->item_type == ITEM_TATTOO )    {
                if (!wear_obj_size(ch, obj)) return;
	if ( !remove_obj( ch, WEAR_TATTOO, fReplace ) )  return;
	act( "$p is being tattooed in $ns skin.", ch, obj, NULL, TO_ROOM );
	act( "$p is being tattooed in your skin.",  ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_TATTOO );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) )    {
                if (!wear_obj_size(ch, obj)) return;
	if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
	&&   get_eq_char( ch, WEAR_FINGER_R ) != NULL
	&&   !remove_obj( ch, WEAR_FINGER_L, fReplace )
	&&   !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL 
                && ch->limb[2]<9) {
	    act( "$n wears $p on $s left finger.",    ch, obj, NULL, TO_ROOM );
	    act( "You wear $p on your left finger.",  ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL 
                && ch->limb[3]<9) {
	    act( "$n wears $p on $s right finger.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p on your right finger.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_R );
	    return;
	}

	bug( "Wear_obj: no free finger.", 0 );
	send_to_char( "You already wear two rings.\r\n", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) )    {
                if (!wear_obj_size(ch, obj)) return;
	if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
	&&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
	&&   !remove_obj( ch, WEAR_NECK_1, fReplace )
	&&   !remove_obj( ch, WEAR_NECK_2, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL )	{
	    act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_1 );
	    return;
	}

	if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL )	{
	    act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_2 );
	    return;
	}

	bug( "Wear_obj: no free neck.", 0 );
	send_to_char( "You already wear two neck items.\r\n", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )    {
                if (!wear_obj_size(ch, obj)) return;
	if ( !remove_obj( ch, WEAR_BODY, fReplace ))  return;
	act( "$n wears $p on $s body.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your body.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_BODY );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) )    {
                if (!wear_obj_size(ch, obj)) return;
	if ( !remove_obj( ch, WEAR_HEAD, fReplace )) return;
	act( "$n wears $p on $s head.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your head.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HEAD );
	return;
    }
    
    if ( CAN_WEAR( obj, ITEM_WEAR_FACE ) )    {
                        if (!wear_obj_size(ch, obj)) return;
                        if ( !remove_obj( ch, WEAR_FACE, fReplace ) )    return;
	        act( "$n wears $p on $s face.",   ch, obj, NULL, TO_ROOM );
                        act( "You wear $p on your face.", ch, obj, NULL, TO_CHAR );
	        equip_char( ch, obj, WEAR_FACE );
	        return;
    }
    
    if ( CAN_WEAR( obj, ITEM_WEAR_EARS ) ) {
                if (!wear_obj_size(ch, obj)) return;
	if ( !remove_obj( ch, WEAR_EARS, fReplace ) )  return;
	act( "$n wears $p on $s ears.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your ears.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_EARS );
	return;
    }
    
    if ( CAN_WEAR( obj, ITEM_WEAR_LEGS )) {
                if (!wear_obj_size(ch, obj)) return;
	if ( !remove_obj( ch, WEAR_LEGS, fReplace ))  return;
	act( "$n wears $p on $s legs.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your legs.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LEGS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FEET )) {
                if (!wear_obj_size(ch, obj)) return;
	if ( !remove_obj( ch, WEAR_FEET, fReplace ))   return;
	act( "$n wears $p on $s feet.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your feet.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_FEET );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HANDS )) {
                if (!wear_obj_size(ch, obj)) return;
	if ( !remove_obj( ch, WEAR_HANDS, fReplace ))  return;
	act( "$n wears $p on $s hands.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your hands.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HANDS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ARMS ))  {
                if (!wear_obj_size(ch, obj)) return;
	if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )  return;
	act( "$n wears $p on $s arms.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your arms.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ARMS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ))  {
                if (!wear_obj_size(ch, obj)) return;
	if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )  return;
	act( "$n wears $p about $s body.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p about your body.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ABOUT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ))  {
                if (!wear_obj_size(ch, obj)) return;
	if ( !remove_obj( ch, WEAR_WAIST, fReplace ) ) return;
	act( "$n wears $p about $s waist.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p about your waist.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WAIST );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) ) {
                if (!wear_obj_size(ch, obj)) return;
	if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
	&&   get_eq_char( ch, WEAR_WRIST_R ) != NULL
	&&   !remove_obj( ch, WEAR_WRIST_L, fReplace )
	&&   !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL ) {
	    act( "$n wears $p around $s left wrist.", ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your left wrist.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL ) {
	    act( "$n wears $p around $s right wrist.", ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your right wrist.",ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_R );
	    return;
	}

	bug( "Wear_obj: no free wrist.", 0 );
	send_to_char( "You already wear two wrist items.\r\n", ch );
	return;
    }

    if (CAN_WEAR(obj, ITEM_WEAR_EYES)) {
      if (!wear_obj_size(ch, obj)) return;
      if (!remove_obj(ch, WEAR_EYES, fReplace)) return;
      act("$n puts $p over $s eyes.", ch, obj, NULL, TO_ROOM);
      act("You cover your eyes with $p.", ch, obj, NULL, TO_CHAR);
      equip_char(ch, obj, WEAR_EYES);
      return;
    }

    if (CAN_WEAR(obj, ITEM_WEAR_BACK)) {
      if (!wear_obj_size(ch, obj)) return;
      if (!remove_obj(ch, WEAR_BACK, fReplace))  return;
      act("$n puts $p on $s back.", ch, obj, NULL, TO_ROOM);
      act("You put $p on your back.", ch, obj, NULL, TO_CHAR);
      equip_char(ch, obj, WEAR_BACK);
      return;
    }

    if ( obj->item_type == ITEM_PRIDE )    {
                if (!wear_obj_size(ch, obj)) return;
	if ( !remove_obj( ch, WEAR_PRIDE, fReplace ) )  return;
	act( "$n wears $p with pride.", ch, obj, NULL, TO_ROOM );
	act( "You wear $p with pride.",  ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_PRIDE );
	return;
    }

    if ( obj->item_type == ITEM_LIGHT )    {
	if ( !remove_obj( ch, WEAR_LIGHT, fReplace)) return;
	act( "$n holds $p.", ch, obj, NULL, TO_ROOM );
	act( "You hold $p.",  ch, obj, NULL, TO_CHAR );
                if (obj->value[0] == FALSE && obj->value[3] == FALSE
                && (obj->value[2] != -1 && obj->value[2] < 999)) {
                      light_obj(ch, obj, TRUE);
                } 
	equip_char( ch, obj, WEAR_LIGHT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FLOAT ) )    {
	if ( !remove_obj( ch, WEAR_FLOAT, fReplace ))  return;
	act( "$p starts floating near $n.",   ch, obj, NULL, TO_ROOM );
	act( "$p starts floating beside you.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_FLOAT );
	return;
    }

    if (!wear_obj_size(ch, obj)) return;
 
   if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) 
    && ch->limb[2]<9) {
	OBJ_DATA *weapon;

	if ( !remove_obj( ch, WEAR_SHIELD, fReplace )) return;

               if (IS_SET(ch->act,ACT_WERE)
               && IS_AFFECTED(ch, AFF_POLY)) {
 	    send_to_char("You don't need that!.\r\n",ch);
                    return;
               }

               if (IS_SET(ch->form, FORM_NOWEAPON)) {
 	    send_to_char("You don't need that!\r\n",ch);
                    return;
               }

	weapon = get_eq_char(ch,WEAR_WIELD);
	if (weapon != NULL && get_char_size(ch) < SIZE_LARGE 
	&&  IS_SET(weapon->wear_flags, ITEM_TWO_HANDS)) {
	    send_to_char("Your hands are tied up with your weapon!\r\n",ch);
	    return;
	}

	weapon = get_eq_char(ch,WEAR_WIELD2);
	if (weapon != NULL) {
		send_to_char("You cannot use a shield while dual wielding!\r\n",ch);
		return;
	}
		
	if ( ch->pc_class == class_lookup("monk") )	{
	    send_to_char("A shield would impair your fighting style too much.\r\n",ch);
	    return;
	}

	act( "$n wears $p as a shield.", ch, obj, NULL, TO_ROOM );
	act( "You wear $p as a shield.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_SHIELD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WIELD ) 
     && ch->limb[3]<9) {
	int sn,skill;

	if ( !remove_obj( ch, WEAR_WIELD, fReplace ))   return;

               if (IS_SET(ch->act,ACT_WERE)
               && IS_AFFECTED(ch, AFF_POLY)) {
 	    send_to_char("You don't need that!.\r\n",ch);
                    return;
               }

               if (IS_SET(ch->form, FORM_NOWEAPON)) {
 	    send_to_char("You don't need that!\r\n",ch);
                    return;
               }

	if ( !IS_NPC(ch) && ch->pc_class == class_lookup("monk"))	{
	    send_to_char( "Your code does not allow you to take up a weapon\r\n", ch );
	    return;
	}

	if ( !IS_NPC(ch) 
	&& get_obj_weight(obj ) > str_app[get_curr_stat(ch,STAT_STR)].wield ){
	    send_to_char( "It is too heavy for you to wield.\r\n", ch );
	    return;
	}

	if (!IS_NPC(ch) && get_char_size(ch) < SIZE_LARGE 
	&&  IS_SET(obj->wear_flags, ITEM_TWO_HANDS)
 	&&  get_eq_char(ch,WEAR_SHIELD) != NULL
	&&  get_eq_char(ch,WEAR_WIELD2) != NULL)
	{
	    send_to_char("You need two hands free for that weapon.\r\n",ch);
	    return;
	}

	act( "$n wields $p.", ch, obj, NULL, TO_ROOM );
	act( "You wield $p.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WIELD );

        sn = get_weapon_sn(ch, FALSE);

	if (sn == gsn_hand_to_hand)
	   return;

        skill = get_weapon_skill(ch,sn);
 
        if (skill >= 100) 	act("$p feels like a part of you!",ch,obj,NULL,TO_CHAR);
        else if (skill > 85) 	act("You feel quite confident with $p.",ch,obj,NULL,TO_CHAR);
        else if (skill > 70) 	act("You are skilled with $p.",ch,obj,NULL,TO_CHAR);
        else if (skill > 50) 	act("Your skill with $p is adequate.",ch,obj,NULL,TO_CHAR);
        else if (skill > 25) 	act("$p feels a little clumsy in your hands.",ch,obj,NULL,TO_CHAR);
        else if (skill > 1) 	act("You fumble and almost drop $p.",ch,obj,NULL,TO_CHAR);
        else 		act("You don't even know which is end is up on $p.", ch,obj,NULL,TO_CHAR);

       /* Autoload and clear the gun when you wield it... */ 

        if ( (skill > 0) 
          && ( (obj->value[0] == WEAPON_GUN)
            || (obj->value[0] == WEAPON_HANDGUN)
            || (obj->value[0] == WEAPON_BOW)
            || (obj->value[0] == WEAPON_SMG))) reload_gun (ch,obj, 0, TRUE);
          
        return;
    }

    if ( CAN_WEAR( obj, ITEM_HOLD ) 
    && ch->limb[2]<9) {
	if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
	    return;
	act( "$n holds $p in $s hands.",   ch, obj, NULL, TO_ROOM );
	act( "You hold $p in your hands.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HOLD );
	return;
    }

    if ( fReplace ) send_to_char( "You can't wear, wield, or hold that.\r\n", ch );
    return;
}


void do_dual (CHAR_DATA *ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' ) {
	send_to_char( "Dual wield what?\r\n", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL ) {
        send_to_char( "You do not have that item.\r\n", ch );
        return;
    }
  
    if ( (ch->level) < obj->level - 10) {
  	sprintf( buf, "You must be level %d to use this object.\r\n", obj->level-10 );
  	send_to_char( buf, ch );
   	act( "$n tries to use $p, but is too inexperienced.", ch, obj, NULL, TO_ROOM );
  	return;
    }
   
    if ( CAN_WEAR( obj, ITEM_WIELD ) 
    && ch->limb[2]<9) {
	int sn,skill;

              if ( !remove_obj( ch, WEAR_WIELD2, TRUE )) return;

               if (IS_SET(ch->act,ACT_WERE)
               && IS_AFFECTED(ch, AFF_POLY)) {
 	    send_to_char("You don't need that!.\r\n",ch);
                    return;
               }

               if (IS_SET(ch->form, FORM_NOWEAPON)) {
 	    send_to_char("You don't need that!\r\n",ch);
                    return;
               }

	if ( !IS_NPC(ch) 
	  && get_obj_weight( obj ) > ((str_app[get_curr_stat(ch,STAT_STR)].wield)  / 2)) {
	    send_to_char( "It is too heavy for you to wield in your off-hand.\r\n", ch );
	    return;
	}

	if ( !IS_NPC(ch) && get_char_size(ch) < SIZE_LARGE 
	&&  IS_SET(obj->wear_flags, ITEM_TWO_HANDS)
	&&  get_eq_char(ch,WEAR_WIELD) !=NULL) {
 	    send_to_char("You need two hands free for that weapon.\r\n",ch);
	    return;
	}
 
	if (get_eq_char(ch, WEAR_SHIELD) != NULL) {
	    send_to_char ("You cannot dual wield while using a shield!\r\n",ch);
	    return;
	}

	act( "$n wields $p.", ch, obj, NULL, TO_ROOM );
	act( "You wield $p.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WIELD2 );

                sn = get_weapon_sn(ch, TRUE);

	if (sn == gsn_hand_to_hand)  return;

        skill = get_weapon_skill(ch,sn);
 
        if (skill >= 100) 	act("$p feels like a part of you!",ch,obj,NULL,TO_CHAR);
        else if (skill > 85) 	act("You feel quite confident with $p.",ch,obj,NULL,TO_CHAR);
        else if (skill > 70) 	act("You are skilled with $p.",ch,obj,NULL,TO_CHAR);
        else if (skill > 50) 	act("Your skill with $p is adequate.",ch,obj,NULL,TO_CHAR);
        else if (skill > 25) 	act("$p feels a little clumsy in your hands.",ch,obj,NULL,TO_CHAR);
        else if (skill > 1)  	act("You fumble and almost drop $p.",ch,obj,NULL,TO_CHAR);
        else 		act("You don't even know which is end is up on $p.", ch,obj,NULL,TO_CHAR);

       /* Autoload and clear the gun when you wield it... */ 

        if ( (skill > 0) 
          && ( obj->value[0] == WEAPON_GUN
            || obj->value[0] == WEAPON_HANDGUN
            || obj->value[0] == WEAPON_BOW
            || obj->value[0] == WEAPON_SMG)) reload_gun (ch,obj, 1, TRUE);

	return;
    }
}


void do_wear( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if (IS_AFFECTED(ch, AFF_MIST)) {
	send_to_char( "You can't do that in mist form?\r\n", ch );
	return;
    }

    if ( arg[0] == '\0' )    {
	send_to_char( "Wear, wield, or hold what?\r\n", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )    {
	OBJ_DATA *obj_next;

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )	{
	     obj_next = obj->next_content;
	      if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) ) {
	                if (obj->condition != 0) {
		         wear_obj( ch, obj, FALSE, 0);
		} else {
		         act ("$p is too damaged to wear!", ch, obj, NULL, TO_CHAR);
                                }
                       }
	}
	return;
    }    else    {
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )	{
                      send_to_char( "You do not have that item.\r\n", ch );
	       return;
	}
	if (obj->condition == 0) act ("$p is too damaged to wear!", ch, obj, NULL, TO_CHAR);
	else wear_obj(ch, obj, TRUE, 0);
    }
    return;
}


void do_wield( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if (IS_AFFECTED(ch, AFF_MIST)) {
	send_to_char( "You can't do that in mist form?\r\n", ch );
	return;
    }

    if ( arg[0] == '\0' )    {
	send_to_char( "Wield what?\r\n", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )    {
	OBJ_DATA *obj_next;

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )	{
	     obj_next = obj->next_content;
	      if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) ) {
	                if (obj->condition != 0) {
		         wear_obj(ch, obj, FALSE, 0);
		} else {
		         act ("$p is too damaged to wield!", ch, obj, NULL, TO_CHAR);
                                }
                       }
	}
	return;
    }    else    {
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )	{
                      send_to_char( "You do not have that item.\r\n", ch );
	       return;
	}

                if (obj->condition == 0) {
                     act ("$p is too damaged to wield!", ch, obj, NULL, TO_CHAR);
	} else {
                     if (IS_SET(obj->wear_flags, ITEM_WIELD) || IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD)) wear_obj( ch, obj, TRUE, WEAR_WIELD);
                     else if (IS_SET(obj->wear_flags, ITEM_HOLD) || IS_SET(obj->wear_flags, ITEM_WEAR_PRIDE) || IS_SET(obj->wear_flags, ITEM_WEAR_FLOAT)) wear_obj( ch, obj, TRUE, WEAR_HOLD);
                     else wear_obj( ch, obj, TRUE, 0 );
                }
    }
    return;
}


void do_hold( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if (IS_AFFECTED(ch, AFF_MIST)) {
	send_to_char( "You can't do that in mist form?\r\n", ch );
	return;
    }

    if ( arg[0] == '\0' )    {
	send_to_char( "Hold what?\r\n", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )    {
	OBJ_DATA *obj_next;

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )	{
	     obj_next = obj->next_content;
	      if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) ) {
	                if (obj->condition != 0) {
		         wear_obj(ch, obj, FALSE, 0);
		} else {
		         act ("$p is too damaged to hold!", ch, obj, NULL, TO_CHAR);
                                }
                       }
	}
	return;
    }    else    {
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )	{
                      send_to_char( "You do not have that item.\r\n", ch );
	       return;
	}

                if (obj->condition == 0) {
                     act ("$p is too damaged to hold!", ch, obj, NULL, TO_CHAR);
	} else {
                     if (IS_SET(obj->wear_flags, ITEM_HOLD) || IS_SET(obj->wear_flags, ITEM_WEAR_PRIDE) || IS_SET(obj->wear_flags, ITEM_WEAR_FLOAT)) wear_obj( ch, obj, TRUE, WEAR_HOLD);
                     else if (IS_SET(obj->wear_flags, ITEM_WIELD) || IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD)) wear_obj( ch, obj, TRUE, WEAR_WIELD);
                     else wear_obj( ch, obj, TRUE, 0 );
                }
    }
    return;
}


void do_remove( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *tmp;
    int number;
    bool all_type = FALSE;
    bool done = FALSE;
	
    one_argument( argument, arg );

	/* Zeran - check for all.foo */
	number = number_argument (arg, arg2);
	if (number == -1)	all_type = TRUE;

    if (IS_AFFECTED(ch, AFF_MIST)) {
	send_to_char( "You can't do that in mist form?\r\n", ch );
	return;
    }

    if ( arg[0] == '\0' )    {
	send_to_char( "Remove what?\r\n", ch );
	return;
    }
	
	/* check remove all.foo */
	if (all_type) {
		number = 0;
		while (!done) {
			if ( (obj = get_obj_wear( ch, arg2) ) == NULL ) {
				if (!number)
					send_to_char( "You do not have any of those items.\r\n", ch );
				done = TRUE;
				continue;	
				}
                                               if ( obj->item_type == ITEM_TATTOO) {
                                 	     send_to_char( "You can't remove a tattoo!\r\n", ch );
                                               } else {
                                                     remove_obj (ch, obj->wear_loc, TRUE);
                                               }
			number++;
			}
		return;
		}

	/* check "remove all" */
	if (!str_cmp(arg, "all"))
		{
		for (tmp = ch->carrying ; tmp != NULL ; tmp = tmp->next_content)
			{
			if (tmp->wear_loc != WEAR_NONE) {

                                                    if ( tmp->item_type == ITEM_TATTOO) {
                                 	         send_to_char( "You can't remove a tattoo!\r\n", ch );
                                                    } else {
                                                         remove_obj(ch, tmp->wear_loc, TRUE);	    
                                                    }
	
                                                }
			}
		return;
		}
	

    if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that item.\r\n", ch );
	return;
    }

    if ( obj->item_type == ITEM_TATTOO) {
          send_to_char( "You can't remove a tattoo!\r\n", ch );
    } else {
          remove_obj (ch, obj->wear_loc, TRUE);
    }

    return;
}


void do_sacrifice( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;


    one_argument( argument, arg );

    if (IS_NPC(ch)) {
         act( "$n wouldn't know whom to offer it to.", ch, NULL, NULL, TO_ROOM );
         return;
    }

    if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) ) {
	act( "$n offers $mself to the Gods, who graciously decline.", ch, NULL, NULL, TO_ROOM );
                sprintf_to_char(ch, "{R%s{x appreciates your offer and may accept it later.\r\n",ch->pcdata->cult);
                return;
    }

    if (!str_cmp(arg, "all")) {
        if ((ch->in_room->vnum == ch->in_room->area->morgue
   	|| ch->in_room->vnum == zones[ch->in_room->area->zone].morgue
    	|| ch->in_room->vnum == mud.morgue)
        && !IS_IMMORTAL(ch)) {
             send_to_char( "Be a bit more selective.\r\n", ch );
             return;
        }

        if (!ch->in_room->contents) {
	send_to_char( "There is nothing.\r\n", ch );
	return;
        }

        for (obj = ch->in_room->contents; obj; obj = obj_next) {
              obj_next = obj->next_content;
              do_real_sac(ch, obj);
        }
        return;
    }

    obj = get_obj_list(ch, arg, ch->in_room->contents );

    if ( obj == NULL ) {
	send_to_char( "You can't find it.\r\n", ch );
	return;
    }

    do_real_sac(ch, obj);
    return;
}


void do_real_sac(CHAR_DATA *ch, OBJ_DATA *obj) {
    MOB_CMD_CONTEXT *mcc;
    WEV *wev; 
    CHAR_DATA *gch;
    int gold, goswp, rn, members;
    char buffer[100];
    char buf[MAX_STRING_LENGTH];
    int currency = 0;

   /* Check it can be sacrificed... */

    if (IS_SET(obj->extra_flags, ITEM_NO_SAC)
    || IS_SET(obj->extra_flags, ITEM_SCENIC)
    || IS_SET(obj->extra_flags, ITEM_ARTIFACT)) {
	send_to_char ("That object cannot be sacrificed.\r\n",ch);
	return;
    }

    if ((ch->in_room->vnum == ch->in_room->area->morgue
   	|| ch->in_room->vnum == zones[ch->in_room->area->zone].morgue
    	|| ch->in_room->vnum == mud.morgue)
    && !IS_IMMORTAL(ch)
    && obj->item_type != ITEM_CORPSE_PC
    && obj->item_type != ITEM_CORPSE_NPC) {
         send_to_char( "You can't that sacrifice here.\r\n", ch );
         return;
    }

    if ( obj->item_type == ITEM_CORPSE_PC
    && obj->contains != NULL) {
        send_to_char("You should loot it first.\r\n",ch);
        return;
    }

   /* Set up context and get WEV... */

    mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);

   /* What's it worth... */

    switch (obj->item_type) {

      case ITEM_CORPSE_PC:
        gold = UMAX(5*obj->level, 1);
        mcc->number = gold;

        wev = get_wev(WEV_SAC, WEV_SAC_PC_CORPSE, mcc,
                     "You sacrifice the @p2.\r\n"
                     "{WThe corpse is consumed by a bright white light!{x\r\n",
                     "@a2 sacrifices the @p2.\r\n"
                     "{WThe corpse is consumed by a bright white light!{x\r\n",
                     "@a2 sacrifices the @p2.\r\n"
                     "{WThe corpse is consumed by a bright white light!{x\r\n");

        break;

      case ITEM_CORPSE_NPC:
        gold = UMAX(obj->level/5, 1);
         mcc->number = gold;

        wev = get_wev(WEV_SAC, WEV_SAC_CORPSE, mcc,
                     "You sacrifice the @p2.\r\n"
                     "{RThe corpse is consumed by burning flames!{x\r\n",
                     "@a2 sacrifices the @p2.\r\n"
                     "{RThe corpse is consumed by burning flames!{x\r\n",
                     "@a2 sacrifices the @p2.\r\n"
                     "{RThe corpse is consumed by burning flames!{x\r\n");

        break;

      case ITEM_TRASH:
        gold = 1;
        mcc->number = gold;

        wev = get_wev(WEV_SAC, WEV_SAC_TRASH, mcc,
                     "You sacrifice @p2.\r\n"
                     "{GIt rots and decays before your eyes!{x\r\n",
                     "@a2 sacrifices @p2.\r\n"
                     "{GIt rots and decays before your eyes!{x\r\n",
                     "@a2 sacrifices @p2.\r\n"
                     "{GIt rots and decays before your eyes!{x\r\n");

        break;

      case ITEM_TREASURE:
        gold = UMAX(10, obj->cost/5);
        mcc->number = gold;

        wev = get_wev(WEV_SAC, WEV_SAC_TREASURE, mcc,
                    "You sacrifice @p2.\r\n"
                    "{YIt shimmers with a golden radiance and vanishes!{x\r\n",
                    "@a2 sacrifices @p2.\r\n"
                    "{YIt shimmers with a golden radiance and vanishes!{x\r\n",
                    "@a2 sacrifices @p2.\r\n"
                    "{YIt shimmers with a golden radiance and vanishes!{x\r\n");

        break;
 
      default:
        gold = UMAX(obj->level/10, 1);
         mcc->number = gold;

        wev = get_wev(WEV_SAC, WEV_SAC_ITEM, mcc,
                     "You sacrifice @p2.\r\n"
                     "{CIt vanishes in a flash of blue light!{x\r\n",
                     "@a2 sacrifices @p2.\r\n"
                     "{CIt vanishes in a flash of blue light!{x\r\n",
                     "@a2 sacrifices @p2.\r\n"
                     "{CIt vanishes in a flash of blue light!{x\r\n");

        break;
    }
   
   /* Challange... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

   /* Send to command issuers room... */

    room_issue_wev( ch->in_room, wev);

   /* Free the wev... */

    free_wev(wev);

   /* Lose the object... */

    extract_obj( obj );

   /* Now work out the consequences... */

    if (gold == 0) return;
    
    if (!IS_NPC(ch)
    && gold > 15) {
        sac_align(ch, gold/15);
    }

    if (gold == 1) {
      sprintf(buf,"{R%s{x gives you {yone %s{x for your sacrifice.\r\n",ch->pcdata->cult, flag_string(currency_type, currency));
      send_to_char(buf, ch);   
      ch->gold[currency] += 1;
      return;
    }

    sprintf(buf,"{R%s{x gives you {y%d %s{x for your sacrifice.\r\n",ch->pcdata->cult, gold, flag_string(currency_type, currency));
    send_to_char(buf, ch);   
 
     /* Saving Godmana */

      rn = get_cult_rn(ch->pcdata->cult);
      if (rn > 0) {
          goswp=gold;
          if (goswp>10) goswp=(goswp-10)/2+10;
          if (goswp>30) goswp=(goswp-30)/3+30;
          if (goswp>60) goswp=(goswp-60)/10+60;
          cult_array[rn].power += goswp;
          cult_array[rn].power = UMIN(cult_array[rn].power,50000);
      }

    /* Reward the character... */

    gold = soc_tax(ch, gold, currency);
    ch->gold[currency] += gold;
    
   /* Do we get to share the gold? */ 

    if (IS_SET(ch->plr, PLR_AUTOSPLIT)) {

         members = 0;
         gch = ch->in_room->people;
         while (gch != NULL) { 
    	if (is_same_group(gch, ch)) members++;
                gch = gch->next_in_room;
          }
          if ( members > 1 ) {
	sprintf(buffer,"%d %s",gold, flag_string(currency_accept, currency));
	do_split(ch,buffer);	
          }
    }

    return;
}


void do_quaff( CHAR_DATA *ch, char *argument ) {
char arg[MAX_INPUT_LENGTH];
OBJ_DATA *obj;
MOB_CMD_CONTEXT *mcc;
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' )    {
	send_to_char( "Quaff what?\r\n", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )    {
	send_to_char( "You do not have that potion.\r\n", ch );
	return;
    }

    if ( obj->item_type != ITEM_POTION )    {
	send_to_char( "You can quaff only potions.\r\n", ch );
	return;
    }

    if (ch->level < obj->level - 15)    {
	send_to_char("This liquid is too powerful for you to drink.\r\n",ch);
	return;
    }

    mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);

    if (obj->pIndexData
    && obj->pIndexData->wear_cond) {
        if (!ec_satisfied_mcc(mcc, obj->pIndexData->wear_cond, TRUE)) {
            free_mcc(mcc);
            sprintf_to_char(ch, "You are unable to use %s.\r\n", obj->short_descr );
            act( "$n tries to use $p, but fails.", ch, obj, NULL, TO_ROOM );
            return;
        }
    }
    free_mcc(mcc);

    if (ch->quaffLast == current_time) { 
      act( "$n spills $p!", ch, obj, NULL, TO_ROOM);
      act( "Oops! In your haste, you spill $p!", ch, obj, NULL, TO_CHAR);
    } else {
      act( "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
      act( "You quaff $p.", ch, obj, NULL ,TO_CHAR );

      if (obj->value[1] != EFFECT_UNDEFINED) {
        obj_cast_spell( obj->value[1], obj->value[0], ch, ch, obj, NULL );
      }

      if (obj->value[2] != EFFECT_UNDEFINED) {
        obj_cast_spell( obj->value[2], obj->value[0], ch, ch, obj, NULL );
      }

      if (obj->value[3] != EFFECT_UNDEFINED) {
        obj_cast_spell( obj->value[3], obj->value[0], ch, ch, obj, NULL );
      }

    }

   /* Remember when we did this... */

    ch->quaffLast = current_time; 

/* Zeran - add waitstate to stop speed quaffing. */

/*    WAIT_STATE (ch, PULSE_VIOLENCE/2); *//* 2 potions per fight round */

    extract_obj( obj );
    return;
}


void do_recite( CHAR_DATA *ch, char *argument ){
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
CHAR_DATA *victim;
OBJ_DATA *scroll;
OBJ_DATA *obj;
MOB_CMD_CONTEXT *mcc;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ( scroll = get_obj_carry( ch, arg1 ) ) == NULL )    {
	send_to_char( "You do not have that scroll.\r\n", ch );
	return;
    }

    if ( scroll->item_type != ITEM_SCROLL )    {
	send_to_char( "You can recite only scrolls.\r\n", ch );
	return;
    }

    if ( ch->level < scroll->level)    {
	send_to_char("This scroll is too complex for you to comprehend.\r\n",ch);
	return;
    }

    mcc = get_mcc(ch, ch, NULL, NULL, scroll, NULL, 0, NULL);

    if (scroll->pIndexData
    && scroll->pIndexData->wear_cond) {
        if (!ec_satisfied_mcc(mcc, scroll->pIndexData->wear_cond, TRUE)) {
            free_mcc(mcc);
            sprintf_to_char(ch, "You are unable to use %s.\r\n", scroll->short_descr );
            act( "$n tries to use $p, but fails.", ch, scroll, NULL, TO_ROOM );
            return;
        }
    }
    free_mcc(mcc);

    obj = NULL;
    if ( arg2[0] == '\0' )    {
	victim = ch;
    }    else    {
	if ( ( victim = get_char_room ( ch, arg2 ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg2 ) ) == NULL )	{
	    send_to_char( "You can't find it.\r\n", ch );
	    return;
	}
    }

    act( "$n recites $p.", ch, scroll, NULL, TO_ROOM );
    act( "You recite $p.", ch, scroll, NULL, TO_CHAR );
    WAIT_STATE(ch, PULSE_VIOLENCE/2);

    if (!check_skill(ch, gsn_scrolls, -20)
    || ch->condition[COND_DRUNK] > 8) {
	send_to_char("You mispronounce a syllable.\r\n",ch);
	check_improve(ch,gsn_scrolls,FALSE,2);
    }    else    {
    	obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj, NULL );
    	obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj, NULL );
    	obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj, NULL );
	check_improve(ch,gsn_scrolls,TRUE,2);
    }

    extract_obj( scroll );
    return;
}



void do_brandish( CHAR_DATA *ch, char *argument ) {
CHAR_DATA *vch;
CHAR_DATA *vch_next;
OBJ_DATA *staff;
MOB_CMD_CONTEXT *mcc;
int spn;

    if ( ( staff = get_eq_char( ch, WEAR_HOLD ) ) == NULL )    {
	send_to_char( "You hold nothing in your hand.\r\n", ch );
	return;
    }

    if ( staff->item_type != ITEM_STAFF )    {
	send_to_char( "You can brandish only with a staff.\r\n", ch );
	return;
    }

    mcc = get_mcc(ch, ch, NULL, NULL, staff, NULL, 0, NULL);

    if (staff->pIndexData
    && staff->pIndexData->wear_cond) {
        if (!ec_satisfied_mcc(mcc, staff->pIndexData->wear_cond, TRUE)) {
            free_mcc(mcc);
            sprintf_to_char(ch, "You are unable to use %s.\r\n", staff->short_descr );
            act( "$n tries to use $p, but fails.", ch, staff, NULL, TO_ROOM );
            return;
        }
    }
    free_mcc(mcc);

    spn = staff->value[3];

    if (!valid_spell(spn)) {
	bug( "Do_brandish: bad spn %d.", spn );
	return;
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( staff->value[2] > 0 )
    {
	act( "$n brandishes $p.", ch, staff, NULL, TO_ROOM );
	act( "You brandish $p.",  ch, staff, NULL, TO_CHAR );
	if ( ch->level < staff->level 
	||   !check_skill(ch, gsn_staves, -20))
 	{
	    act ("You fail to invoke $p.",ch,staff,NULL,TO_CHAR);
	    act ("...and nothing happens.",ch,NULL,NULL,TO_ROOM);
	    check_improve(ch,gsn_staves,FALSE,2);
	}
	
	else for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next	= vch->next_in_room;

	    switch ( spell_array[spn].target )
	    {
	    default:
		bug( "Do_brandish: bad target for spn %d.", spn );
		return;

	    case TAR_IGNORE:
		if ( vch != ch )
		    continue;
		break;

	    case TAR_CHAR_OFFENSIVE:
		if ( IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch) )
		    continue;
		break;
		
	    case TAR_CHAR_DEFENSIVE:
		if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
		    continue;
		break;

	    case TAR_CHAR_SELF:
		if ( vch != ch )
		    continue;
		break;
	    }

                    if (vch==NULL) vch=ch;
	    obj_cast_spell( spn, staff->value[0], ch, vch, NULL, NULL );
	    check_improve(ch,gsn_staves,TRUE,2);
	}
    }

    if ( --staff->value[2] <= 0 )
    {
	act( "$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM );
	act( "Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR );
	extract_obj( staff );
    }

    return;
}



void do_zap( CHAR_DATA *ch, char *argument ) {
char arg[MAX_INPUT_LENGTH];
CHAR_DATA *victim;
OBJ_DATA *wand;
OBJ_DATA *obj;
MOB_CMD_CONTEXT *mcc;

    one_argument( argument, arg );
    if ( arg[0] == '\0' && ch->fighting == NULL )    {
	send_to_char( "Zap whom or what?\r\n", ch );
	return;
    }

    if ( ( wand = get_eq_char( ch, WEAR_HOLD ) ) == NULL )    {
	send_to_char( "You hold nothing in your hand.\r\n", ch );
	return;
    }

    if ( wand->item_type != ITEM_WAND )    {
	send_to_char( "You can zap only with a wand.\r\n", ch );
	return;
    }

    mcc = get_mcc(ch, ch, NULL, NULL, wand, NULL, 0, NULL);

    if (wand->pIndexData
    && wand->pIndexData->wear_cond) {
        if (!ec_satisfied_mcc(mcc, wand->pIndexData->wear_cond, TRUE)) {
            free_mcc(mcc);
            sprintf_to_char(ch, "You are unable to use %s.\r\n", wand->short_descr );
            act( "$n tries to use $p, but fails.", ch, wand, NULL, TO_ROOM );
            return;
        }
    }
    free_mcc(mcc);

    obj = NULL;
    if ( arg[0] == '\0' )    {
	if ( ch->fighting != NULL )	{
	    victim = ch->fighting;
	} else {
	    send_to_char( "Zap whom or what?\r\n", ch );
	    return;
	}
    }    else    {
	if ( ( victim = get_char_room ( ch, arg ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg ) ) == NULL )	{
	    send_to_char( "You can't find it.\r\n", ch );
	    return;
	}
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( wand->value[2] > 0 )    {
	if ( victim != NULL )	{
	    act( "$n zaps $N with $p.", ch, wand, victim, TO_ROOM );
	    act( "You zap $N with $p.", ch, wand, victim, TO_CHAR );
	}	else	{
	    act( "$n zaps $P with $p.", ch, wand, obj, TO_ROOM );
	    act( "You zap $P with $p.", ch, wand, obj, TO_CHAR );
	}
	
 	if (ch->level < wand->level 
	|| !check_skill(ch, gsn_wands, -20) ) 	{
	    act( "Your efforts with $p produce only smoke and sparks.",
		 ch,wand,NULL,TO_CHAR);
	    act( "$n's efforts with $p produce only smoke and sparks.",
		 ch,wand,NULL,TO_ROOM);
	    check_improve(ch,gsn_wands,FALSE,2);
	}	else	{
	    obj_cast_spell( wand->value[3], wand->value[0], ch, victim, obj, NULL );
	    check_improve(ch,gsn_wands,TRUE,2);
	}
    }

    if ( --wand->value[2] <= 0 )    {
	act( "$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM );
	act( "Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR );
	extract_obj( wand );
    }

    return;
}


void do_steal( CHAR_DATA *ch, char *argument ) {
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent;
    int togain;

    MOB_CMD_CONTEXT *mcc;
    WEV *wev;    

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )    {
	send_to_char( "Steal what from whom?\r\n", ch );
	return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE) || IS_SET(ch->in_room->room_flags, ROOM_LAW)) {
	send_to_char( "You don't dare to steal here.\r\n", ch );
	return;
    }

    if (arg2[0] =='\0') {

        if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )    {
    	send_to_char( "They aren't here.\r\n", ch );
	return;
        }

        if (str_cmp(race_array[victim->race].name,"machine")
        && !IS_SET(victim->form, FORM_MACHINE)) {
    	send_to_char( "You can't steal that.\r\n", ch );
	return;
        }

        if ( victim->master == ch )    {
	send_to_char( "It's already yours!\r\n", ch );
	return;
        }

        if ( victim->ridden == TRUE )    {
	send_to_char( "There is someone riding it right now!\r\n", ch );
	return;
        }

        percent =  ch->level - victim->level;
        if (!check_skill(ch, gsn_steal, percent)) {
            if (victim->master != NULL) {
                 mcc = get_mcc(victim, victim, ch, NULL, NULL, NULL, 0, victim->master->name );
                 wev = get_wev(WEV_ICC, WEV_ICC_SCREAM, mcc,
                  "{R@t0's @a2 begins to hoot!{x\r\n",
                  NULL,
                  "{R@t0's @a2 begins to hoot!{x\r\n");
                  if (!room_issue_wev_challange(ch->in_room, wev)) {
                         free_wev(wev);
                         return;
                  }
                  area_issue_wev(ch->in_room, wev, WEV_SCOPE_SUBAREA_PLUS);
                  free_wev(wev);
                  WAIT_STATE( ch, skill_array[gsn_steal].beats );
             }
        } else {
             die_follower(victim->master );
             SET_BIT(victim->act, ACT_FOLLOWER);
             SET_BIT(victim->affected_by, AFF_CHARM);
             victim->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
             add_follower( victim, ch );
             victim->leader = ch;
             act( "You've now got a $N!", ch, NULL, victim,TO_CHAR );
             act( "$n steals a $N!", ch, NULL, victim, TO_ROOM );
             WAIT_STATE( ch, skill_array[gsn_steal].beats );
       }
       return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( victim == ch )    {
	send_to_char( "That's pointless.\r\n", ch );
	return;
    }

    if ( !IS_NPC(victim) && !can_murder(ch, victim) )    {
  	send_to_char ("That player is outside your stealing range.\r\n", ch);
	return;
    }

    if (victim->position == POS_FIGHTING)     {
	send_to_char("You'd better not -- you might get hit.\r\n",ch);
	return;
    }

    WAIT_STATE( ch, skill_array[gsn_steal].beats );
	
	if (victim->position == POS_FIGHTING)	{
		send_to_char ("Damn, your victim didn't hold still long enough.\r\n",ch);
		return;
		}
	
    percent  = IS_AWAKE(victim) ? 0 : -50;

    if (!can_see(victim, ch)) percent -= 25;

    percent -= (3*((get_curr_stat(ch, STAT_DEX)/4) - 20)); /*dex bonus*/

    {
        int diff = ch->level - victim->level; 
	int modifier = 5*(diff - 10);
	if (abs(diff) > 10)
	percent -= modifier;
    }

   	/* ok, lets see what happens */ 
	if (!check_skill(ch, gsn_steal, percent)) { 
		char messbuf[80];
		sprintf(messbuf, "You failed to steal from %s.\r\n", PERS(victim,ch));
		send_to_char (messbuf, ch);
	                check_improve(ch,gsn_steal,FALSE,2);
	} else {
		int visionplus=0;
		int visionminus=0;
		
		if (IS_AFFECTED(ch, AFF_DETECT_INVIS)) {
			visionplus=AFF_DETECT_INVIS;	
			SET_BIT(victim->affected_by, AFF_DETECT_INVIS);
		}
		if (IS_AFFECTED(victim, AFF_BLIND)) {
			visionminus=AFF_BLIND;
			REMOVE_BIT(victim->affected_by, AFF_BLIND);
		}
				
	    if ( !str_cmp( arg1, "coin"  )
	    ||   !str_cmp( arg1, "coins" )
	    ||   !str_cmp( arg1, "money" )
	    ||   !str_cmp( arg1, "gold"  ) ) {
			int amount;
                                                int currency = number_range(0, MAX_CURRENCY - 1);

			amount = victim->gold[currency] * number_range(1, 10) / 100;
			if ( amount <= 0)	{
			    sprintf_to_char(ch, "You couldn't get any %s.\r\n", flag_string(currency_type, currency));

			} else {
				ch->gold[currency]     += amount;
				victim->gold[currency] -= amount;
				sprintf_to_char(ch, "Bingo!  You got %d %s.\r\n", amount, flag_string(currency_type, currency));
				check_improve(ch,gsn_steal,TRUE,2);
				togain = (victim->level - ch->level) +3;
				if (togain >= 1 ) gain_exp(ch,togain, FALSE);
	  	 	}

	    } else if ( ( obj = get_obj_carry( victim, arg1 ) ) == NULL ) {
			send_to_char( "You can't find it.\r\n", ch );

    	    } else if ( !can_drop_obj( ch, obj )  || IS_SET(obj->extra_flags, ITEM_INVENTORY)) {
			send_to_char( "You can't pry it away.\r\n", ch );

     	    } else if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) ) {
			send_to_char( "You have your hands full.\r\n", ch );

	    } else if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) ) {
			send_to_char( "You can't carry that much weight.\r\n", ch );

	    } else {
			char buf[80];
			sprintf(buf, "Bingo! You got %s.  Better run while you can.\r\n", obj->short_descr);	
			send_to_char (buf, ch);
			obj_from_char( obj );
	  		obj_to_char( obj, ch );
	  		check_improve(ch,gsn_steal,TRUE,2);
			togain = (victim->level - ch->level) +3;
			if ( togain >= 1 ) gain_exp(ch,togain, FALSE);
	     }

		REMOVE_BIT(victim->affected_by, visionplus);
		SET_BIT (victim->affected_by, visionminus);
		}	
	/* Now, see if anyone noticed the thievery */
	{
	CHAR_DATA *person;
	int chance;
	char messbuf[80];
	
	person = ch->in_room->people;
	for (;person;person=person->next_in_room)
		{
		if ( (person!=victim) && ( (person->position < POS_RESTING)  ||  !can_see(person, ch) || !can_see(person, victim)))	continue;

		chance = ((get_curr_stat(person, STAT_INT))/4) + person->level - ch->level;	
		if ((person == victim) && (person->position >= POS_RESTING) && can_see(person, ch)) 	chance += 20;

		if (number_percent() <= chance) {
			if ((person != victim) && (person != ch) && !IS_NPC(person))	{
				sprintf (messbuf, "You notice %s trying to steal from %s!\r\n",	PERS(ch, person), PERS(victim, person ) );
				send_to_char (messbuf, person);
				continue;
			}

			if ((person == victim) && !IS_NPC(person)) {
				sprintf (messbuf, "%s is trying to steal from you!\r\n", (victim->position >= POS_RESTING) ? PERS(ch, person) : "Someone");
				send_to_char (messbuf, person);
				continue;
			}

			if ( (person == victim) && IS_NPC(victim) )	{
				do_say (person, "This is what I do to thieves!\r\n");
				multi_hit (person, ch, TYPE_UNDEFINED);			
			}
		}

		} /*end character for loop*/
	} /*end check notice block*/	
} /*end function*/



/* Say when a shop is open... */

void do_hours( CHAR_DATA *ch, char *args ) {

    char buf[MAX_STRING_LENGTH];

    CHAR_DATA *keeper;
    SHOP_DATA *pShop;

   /* Check parms... */

    if ( args[0] != '\0' ) {
      send_to_char("Syntax: hours\r\n", ch);
      return;
    }

   /* Find shop keeper... */

    pShop = NULL;

    keeper = ch->in_room->people;

    while ( keeper != NULL
         && pShop == NULL ) {

	if ( keeper->pIndexData != NULL ) {
          pShop = keeper->pIndexData->pShop;
        }

        if (pShop == NULL) {
          keeper = keeper->next_in_room;
        }
    }

    if ( pShop == NULL ) {
	send_to_char( "There is no shop keeper here.\r\n", ch );
	return;
    }

   /* Invisible or hidden people... */

    if ( !can_see( keeper, ch ) ) {
	do_say( keeper, "What? Who said that? Is somebody there?" );
	return;
    }

   /* Report opening hours... */

    if ( pShop->open_hour == 0 
      && pShop->close_hour == 23 ) { 

      do_say(keeper, "I am always open!");

    } else if ( pShop->open_hour == 0 ) {

      sprintf(buf, "I am open from midnight until %d:00.",
                   (pShop->close_hour + 1)); 

      do_say(keeper, buf);

    } else if ( pShop->close_hour == 23 ) {

      sprintf(buf, "I am open from %d:00 until midnight.",
                   pShop->open_hour);

      do_say(keeper, buf);

    } else {

      sprintf(buf, "I am open from %d:00 until %d:00.",
                    pShop->open_hour,
                   (pShop->close_hour + 1)); 

      do_say(keeper, buf);
    }

   /* All done... */

    return;
}

		
/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper( CHAR_DATA *ch, bool loud) {
CHAR_DATA *keeper;
SHOP_DATA *pShop;

   /* Find shop keeper... */

    pShop = NULL;

    keeper = ch->in_room->people;

    while ( keeper != NULL  && pShop == NULL ) {
        if ( keeper->pIndexData != NULL ) pShop = keeper->pIndexData->pShop;
        if (pShop == NULL) keeper = keeper->next_in_room;
    }

    if ( pShop == NULL ) {
	if (loud) send_to_char( "There is no shop keeper here.\r\n", ch );
	return NULL;
    }
 
    /*
     * Invisible or hidden people.
     */
    if ( !can_see( keeper, ch )) {
	if (loud) do_say( keeper, "What? Who said that? Is somebody there?" );
	return NULL;
    }

    /*
     * Shop hours.
     */
    if ( time_info.hour < pShop->open_hour ) {
	if (loud) do_say( keeper, "Sorry, I am closed. Come back later." );
	return NULL;
    }
    
    if ( time_info.hour > pShop->close_hour ) {
	if (loud) do_say( keeper, "Sorry, I am closed. Come back tomorrow." );
	return NULL;
    }

    return keeper;
}


void obj_to_keeper( OBJ_DATA *obj, CHAR_DATA *ch ) {
    OBJ_DATA *t_obj, *t_obj_next;

     /* see if any duplicates are found */
    for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next) {
    t_obj_next = t_obj->next_content;

    if (obj->pIndexData == t_obj->pIndexData
    &&  !str_cmp(obj->short_descr,t_obj->short_descr))  {
        /* if this is an unlimited item, destroy the new one */
        if (IS_OBJ_STAT(t_obj,ITEM_INVENTORY))  {
        extract_obj(obj);
        return;
         }
         obj->cost = t_obj->cost; /* keep it standard */
        break;
    }
    }

    if (t_obj == NULL)
     {
    obj->next_content = ch->carrying;
    ch->carrying = obj;
    }
    else
    {
    obj->next_content = t_obj->next_content;
    t_obj->next_content = obj;
     }

    obj->carried_by      = ch;
     obj->in_room         = NULL;
    obj->in_obj          = NULL;
    ch->carry_number    += get_obj_number( obj );
    ch->carry_weight    += get_obj_weight( obj );
}

/* elfren added next prog */
/* get an object from a shopkeeper's list */
OBJ_DATA *get_obj_keeper( CHAR_DATA *ch, CHAR_DATA *keeper, char *argument )
{
     char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
     for ( obj = keeper->carrying; obj != NULL; obj = obj->next_content )
    {
          if (obj->wear_loc == WEAR_NONE
        &&  can_see_obj( keeper, obj )
    &&  can_see_obj(ch,obj)
        &&  is_name( arg, obj->name ) )
        {
            if ( ++count == number )
                return obj;

        /* skip other objects of the same name */
        while (obj->next_content != NULL
        && obj->pIndexData == obj->next_content->pIndexData
         && !str_cmp(obj->short_descr,obj->next_content->short_descr))
        obj = obj->next_content;
          }
    }
    return NULL;
}


int get_cost( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy ) {
SHOP_DATA *pShop;
int cost;
OBJ_DATA *obj2;
int itype;
int i; 
float cost_mod, cost_mod2;
int currency = find_currency(keeper);

if (keeper->pIndexData->pShop->currency > -1) currency = keeper->pIndexData->pShop->currency;

cost_mod = get_currency_modifier(keeper->in_room->area, currency);
cost_mod2 = get_currency_modifier(keeper->in_room->area, find_obj_currency(obj));

if (cost_mod < .05) cost_mod = .05;
if (cost_mod2 < .05) cost_mod2 = .05;

   /* Nothing is free */

    if ( obj == NULL )         return 0;

   /* We need a shop keeper... */

    pShop = keeper->pIndexData->pShop;

    if ( pShop == NULL ) return 0;
    
   /* Work out the price... */

    if ( fBuy ) {
	cost = obj->cost * pShop->profit_buy  / 100;
    } else {

	cost = 0;
	for ( itype = 0; itype < MAX_TRADE; itype++ ) {
	    if ( obj->item_type == pShop->buy_type[itype] ) {
		cost = obj->cost * pShop->profit_sell / 100;
		break;
	    }
	}

       /* start multiples price checking */

        if (!IS_SET(obj->extra_flags, ITEM_SELL_EXTRACT)) {
            for ( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content ) {
                if (  obj->pIndexData == obj2->pIndexData
                  && !str_cmp(obj->short_descr,obj2->short_descr) ) {
                    if (IS_OBJ_STAT(obj2,ITEM_INVENTORY)) {
                        cost /= 2;
                    } else {
                        cost = cost * 3 / 4;
                    }
                }
            }
        }
    }

   /* Staves and wands get marked down for wear... */

    if ( obj->item_type == ITEM_STAFF 
      || obj->item_type == ITEM_WAND ) {

        if (obj->value[1] == 0) {
           cost /= 4;
        } else {
           cost = cost * obj->value[2] / obj->value[1];
        }
    }

   /* Reduce offer price to conserve gold... */

    if (!fBuy) {
 
      i = 9000;

      while ( keeper->gold[currency] < i ) {
        cost -= (cost/10);
        i -= 1000;
      }

      if ( cost > keeper->gold[currency] / 2 ) {
        cost /= 2;
      }

    }

   /* Boost price to increase gold reserves... */

    if (fBuy) {

      i = 9000;

      while ( keeper->gold[currency] < i ) {
 
        cost += (cost/20);
        i -= 1000;

      }  
 
    }

    if (cost <1) return 0;
    if (obj->pIndexData->area == keeper->pIndexData->area) return cost;

    cost = (int) (cost * (cost_mod2 /cost_mod));
    if (cost < 1) return 1;
    
    return cost;
}


void do_buy( CHAR_DATA *ch, char *argument ) {
char buf[MAX_STRING_LENGTH];
int cost;
char *lastarg;
int currency = find_currency(ch);

    if ( argument[0] == '\0' )    {
	send_to_char( "Buy what?\r\n", ch );
	return;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP))    {
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *pet;
	ROOM_INDEX_DATA *pRoomIndexNext;
	ROOM_INDEX_DATA *in_room;

	smash_tilde(argument); 
	argument = one_argument(argument,arg);

	pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	if ( pRoomIndexNext == NULL )	{
	    bug( "Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "Sorry, you can't buy that here.\r\n", ch );
	    return;
	}

	in_room     = ch->in_room;
	ch->in_room = pRoomIndexNext;
	pet         = get_char_room( ch, arg );
	ch->in_room = in_room;

	if (pet == NULL || !IS_SET(pet->act, ACT_PET) )	{
	    send_to_char( "Sorry, you can't buy that here.\r\n", ch );
	    return;
	}

	if ( ch->pet != NULL )	{
	    send_to_char("You already own a pet.\r\n",ch);
	    return;
	}

 	cost = 10 * pet->level * pet->level;

	if ( ch->gold[currency] < cost )	{
	    send_to_char( "You can't afford it.\r\n", ch );
	    return;
	}

	if ( ch->level < pet->level )	{
	    send_to_char( "You're not powerful enough to master this pet.\r\n", ch );
	    return;
	}

	ch->gold[currency]	-= cost;
	pet			= create_mobile(pet->pIndexData);
	SET_BIT(ch->plr, PLR_BOUGHT_PET);
	SET_BIT(pet->act, ACT_PET);
	SET_BIT(pet->affected_by, AFF_CHARM);
	pet->comm = COMM_NOTELL;

	argument = one_argument( argument, arg );
	if ( arg[0] != '\0' ) {
	    sprintf( buf, "%s %s", pet->name, arg );
	    free_string( pet->name );
	    pet->name = str_dup( buf );
	}

	sprintf( buf, "%sA neck tag says 'I belong to %s'.\r\n", pet->description, ch->name );
	free_string( pet->description );
	pet->description = str_dup( buf );

	char_to_room( pet, ch->in_room );
	add_follower( pet, ch );
	pet->leader = ch;
	ch->pet = pet;
	send_to_char( "Enjoy your pet.\r\n", ch );
	act( "$n bought $N as a pet.", ch, NULL, pet, TO_ROOM );
	return;
    }    else    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj,*t_obj,*tattoo;
                char arg[MAX_INPUT_LENGTH];
                int number, count = 1;

	keeper = find_keeper(ch, TRUE);
	if ( keeper == NULL ) return;

                if (get_cust_rating(keeper->pIndexData->pShop, ch->name) < -10) {
                     do_say(keeper, "You're not welcome here.");
                     return;
                }

                if (keeper->pIndexData->pShop->currency > -1) currency = keeper->pIndexData->pShop->currency;
        
                number = mult_argument(argument,arg);

                if (number < 1) {
	      act( "$n tells you 'That's a silly number of things to buy!'", keeper, NULL, ch, TO_VICT );
	      ch->reply = keeper;
                      return;
                }
        
                obj = get_obj_carry( keeper, argument );

                if (!obj) {
	      act( "$n tells you 'I don't sell that -- try 'list''.", keeper, NULL, ch, TO_VICT );
	      ch->reply = keeper;
                      return;
                }

                if (obj->item_type == ITEM_PASSPORT) {
                    bool ok = FALSE;
                    int i;

                    for(i = 0; i < MAX_TRADE; i++) {
                       if (keeper->pIndexData->pShop->buy_type[i] == 994) ok = TRUE;
                    }
                    if (!ok) {
	       act( "$n tells you 'I don't sell that'.", keeper, NULL, ch, TO_VICT );
	       ch->reply = keeper;
                       return;
                    }

                    if (find_passport(ch)) {
	       act( "$n tells you 'You already got a passport'.", keeper, NULL, ch, TO_VICT );
	       ch->reply = keeper;
                       return;
                    }

                    if (get_passport_order(ch)) {
	       act( "$n tells you 'You already got a passport ordered'.", keeper, NULL, ch, TO_VICT );
	       ch->reply = keeper;
                       return;
                    }
                }

                tattoo = get_eq_char(ch, WEAR_TATTOO);

                if ( tattoo != NULL
                && obj->item_type == ITEM_TATTOO ) {
	      act( "$n tells you 'You already got a tattoo'.", keeper, NULL, ch, TO_VICT );
	      ch->reply = keeper;
                      return;
                }

	cost = get_cost( keeper, obj, TRUE );

	if (  cost <= 0  
                || !can_see_obj( ch, obj ) ) {
  	      act( "$n tells you 'I don't sell that -- try 'list''.", keeper, NULL, ch, TO_VICT );
	      ch->reply = keeper;
	      return;
	}

             if (keeper->spec_fun > 0
             && !str_cmp("spec_translator", spec_string(keeper->spec_fun))
             && obj->item_type == ITEM_BOOK) {
                        if (obj->condition < 100) {
                                act("$n tells you 'Give me more time - not yet finished.", keeper, NULL, ch,TO_VICT);
                                ch->reply = keeper;
                                return;
                        }

                        if (!obj->owner) {
                               act("$n tells you 'This is not your stuff!.", keeper, NULL, ch,TO_VICT);
                               ch->reply = keeper;
                               obj_from_char(obj);
                               extract_obj(obj);
                               return;
                        }

                        if (str_cmp(obj->owner, ch->name)) {
                               act("$n tells you 'This is not your stuff!.", keeper, NULL, ch,TO_VICT);
                               ch->reply = keeper;
                               return;
                        }
             }

             if (keeper->spec_fun > 0
             && !str_cmp("spec_mechanic", spec_string(keeper->spec_fun))
             && obj->item_type == ITEM_PORTAL
             && obj->value[4] == PORTAL_VEHICLE) {
                  if (obj->condition < 100) {
                      act("$n tells you 'Give me more time - not yet finished.", keeper, NULL, ch,TO_VICT);
                      ch->reply = keeper;
                      return;
                  }
                  lastarg = check_obj_owner(obj);
                  if (str_cmp(lastarg, "not found")
                  && str_cmp(capitalize(lastarg), ch->short_descr)) {
                        act("$n tells you 'This is not your stuff!.", keeper, NULL, ch,TO_VICT);
                        ch->reply = keeper;
                        return;
                  }
             }

        if (obj->owner != NULL && str_cmp(obj->owner, ch->name)) {
             act("$n tells you 'This is not your stuff!.", keeper, NULL, ch,TO_VICT);
             ch->reply = keeper;
             return;
        }

        if (!IS_OBJ_STAT(obj, ITEM_INVENTORY)) {
              for ( t_obj = obj->next_content;
                    count < number && t_obj != NULL;
                    t_obj = t_obj->next_content) {

                  if ( t_obj->pIndexData == obj->pIndexData
                    && !str_cmp(t_obj->short_descr,obj->short_descr)) {
                       count++;
                  } else {
                       break;
                  }
              }

              if ( count < number
                || number > 99) {
                   act("$n tells you 'I don't have that many in stock.", keeper,NULL,ch,TO_VICT);
                   ch->reply = keeper;
                   return;
              }
        }

        if (!IS_NPC(ch)
        && get_skill(ch, gsn_haggle) > 0
        && IS_SET(ch->plr, PLR_HAGGLE)) {
            sprintf(buf, "I want %d %s for %s.", cost, flag_string(currency_type, currency), obj->short_descr);
            do_say(keeper, buf);
            send_to_char("Accept?\r\n", ch);
            ch->pcdata->store_char = keeper;
            ch->pcdata->store_obj = obj;
            ch->pcdata->store_number[0] = cost;
            ch->pcdata->store_number[1] = number;
            ch->desc->connected = CON_CONFIRM_BUY;

        } else {
            execute_buy(ch, keeper, obj, cost, number);
        }
    }
    return;
}


void execute_buy(CHAR_DATA *ch, CHAR_DATA *keeper, OBJ_DATA *obj, int cost, int number) {
OBJ_DATA *t_obj;
char buf[MAX_STRING_LENGTH];
int count;
int currency = find_currency(ch);


        if (!keeper || !obj || cost <= 0) return;
        if (obj->carried_by != keeper) {
            send_to_char("They don't have that anymore.\r\n", ch);
            return;
        }

        if (keeper->pIndexData->pShop->currency > -1) currency = keeper->pIndexData->pShop->currency;

        if ( ch->gold[currency] < cost * number ) {
              if (number > 1) {
                  act("$n tells you 'You can't afford to buy that many.", keeper,obj,ch,TO_VICT);
              } else {
                  act( "$n tells you 'You can't afford to buy $p'.", keeper, obj, ch, TO_VICT );
              } 
              ch->reply = keeper;
              return;
        }

         if ( obj->level > ch->level+10
         && obj->owner == NULL ) {
	    act( "$n tells you 'You can't use $p yet'.", keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
         }

         if ( ch->carry_number + number * get_obj_number( obj ) > can_carry_n( ch )
         && (obj->item_type != ITEM_PORTAL  || obj->value[4] != PORTAL_VEHICLE)) {
    	            send_to_char( "You can't carry that many items.\r\n", ch );
	            return;
         }

         if ( ch->carry_weight +  number * get_obj_weight( obj ) > can_carry_w( ch )
         && (obj->item_type != ITEM_PORTAL  || obj->value[4] != PORTAL_VEHICLE)) {
	           send_to_char( "You can't carry that much weight.\r\n", ch );
	           return;
         }

       /* added for multiple items */

        if (number > 1) {
          sprintf(buf,"$n buys $p[%d].",number);
          act(buf,ch,obj,NULL,TO_ROOM);
          sprintf(buf,"You buy $p[%d] for %d %s.",number, cost * number, flag_string(currency_type, currency));
          act(buf,ch,obj,NULL,TO_CHAR);

        } else {

          act( "$n buys $p.", ch, obj, NULL, TO_ROOM );
          sprintf(buf,"You buy $p for %d %s.",cost, flag_string(currency_type, currency));
          act( buf, ch, obj, NULL, TO_CHAR );
        }

        ch->gold[currency]     -= cost * number;
        keeper->gold[currency] += cost * number;

        for (count = 0; count < number; count++) {

            if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) ) {
                 t_obj = create_object( obj->pIndexData, obj->level );
            } else {
                 t_obj = obj;
                 if (t_obj == NULL) {
                      act("$n shouts 'I've been robbed!", keeper,obj,ch,TO_VICT);
                      bug("Keeper (room %d) sold what he didn't have...", keeper->in_room->vnum);
                      return;
                 }
                 obj_from_char( t_obj );
            }

            if ( t_obj->item_type == ITEM_POTION 
            || t_obj->item_type == ITEM_SCROLL ) {
                t_obj->timer = UMAX(48, ( 2 * keeper->level));
            } else {
                t_obj->timer = 0;
            }

            if (t_obj->item_type == ITEM_PORTAL
            && t_obj->value[4] == PORTAL_VEHICLE) {
               obj_to_room( t_obj, ch->in_room );
               if (t_obj->cost == t_obj->pIndexData->cost) create_vehicle(ch, t_obj);
               REMOVE_BIT(t_obj->wear_flags, ITEM_TAKE);
            }else {
               if (obj->item_type == ITEM_PASSPORT) {
                   if (IS_HERO(ch) || IS_IMMORTAL(ch)) order_passport(ch, 1);
                   else order_passport(ch, 0);
                   return;
               } else {
                   if (t_obj->owner != NULL) {
                        t_obj->owner = NULL;
                        t_obj->cost = t_obj->pIndexData->cost;
                   }
                   obj_to_char( t_obj, ch );
               }
            }

            t_obj->size = get_char_size(ch);
            if ( t_obj->item_type == ITEM_TATTOO ) wear_obj(ch, t_obj, TRUE, 0);
        }

        if (number_percent() < 10) improve_rating(keeper->pIndexData->pShop, ch->name);
        return;
}


void do_appraise( CHAR_DATA *ch, char *argument ) {
CHAR_DATA *keeper;
OBJ_DATA *obj;
char arg[MAX_INPUT_LENGTH];
int cost;
int currency = find_currency(ch);

       argument = one_argument(argument,arg);

       if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP))    {
	CHAR_DATA *pet;
	ROOM_INDEX_DATA *pRoomIndexNext;
	ROOM_INDEX_DATA *in_room;

	pRoomIndexNext = get_room_index( ch->in_room->vnum + 1);
	if (pRoomIndexNext == NULL )	{
	    bug( "Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "Sorry, you can't appraise that here.\r\n", ch );
	    return;
	}

	in_room     = ch->in_room;
	ch->in_room = pRoomIndexNext;
	pet         = get_char_room( ch, arg );
                ch->in_room = in_room;

	if ( pet == NULL || !IS_SET(pet->act, ACT_PET) )	{
	    send_to_char( "Sorry, you can't find that here.\r\n", ch );
	    return;
	}
                sprintf_to_char(ch, "%s", pet->long_descr);
                if (pet->description) sprintf_to_char(ch, "%s", pet->description);

       } else {
           keeper = find_keeper(ch, TRUE);
           if ( keeper == NULL ) return;

           if (get_cust_rating(keeper->pIndexData->pShop, ch->name) < -10) {
                do_say(keeper, "You're not welcome here.");
                return;
           }

           if (keeper->pIndexData->pShop->currency > -1) currency = keeper->pIndexData->pShop->currency;

           obj = get_obj_carry( keeper, arg );

           if ( obj == NULL ) {
	    act( "$n tells you 'I don't sell that -- try 'list''.", keeper, NULL, ch, TO_VICT );
	    ch->reply = keeper;
                    return;
            }

            cost = get_cost( keeper, obj, TRUE );
            cost /=10;
            if (ch->gold[currency] < cost) {
               send_to_char("You can't afford to appraise that.\r\n",ch);
               return;
            }

            if (cost<1) cost=1;

            ch->gold[currency] -=cost;
            spell_identify( gsn_lore, ch->level, ch, obj);
       } 
       return;
  }


void do_list( CHAR_DATA *ch, char *argument ) {
char buf[MAX_STRING_LENGTH];
int currency = find_currency(ch);

    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )    {
	ROOM_INDEX_DATA *pRoomIndexNext;
	CHAR_DATA *pet;
	bool found;

	pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	if ( pRoomIndexNext == NULL )	{
	    bug( "Do_list: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "You can't do that here.\r\n", ch );
	    return;
	}

	found = FALSE;
	for ( pet = pRoomIndexNext->people; pet; pet = pet->next_in_room ) {
	    if ( IS_SET(pet->act, ACT_PET) )	    {
		if ( !found )		{
		    found = TRUE;
		    send_to_char( "Pets for sale:\r\n", ch );
		    sprintf_to_char(ch, "[{gLv {y%10s{w]  Pet \r\n",capitalize(flag_string(currency_type, currency)));
		}
		sprintf( buf, "[%2d]  %8d - %s\r\n", pet->level, 10 * pet->level * pet->level,  pet->short_descr );
		send_to_char( buf, ch );
	    }
	}
	if ( !found ) send_to_char( "Sorry, we're out of pets right now.\r\n", ch );

    }    else    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
                PASSPORT *pass;
	int cost,count;
	bool found;
	char arg[MAX_INPUT_LENGTH];

	if (( keeper = find_keeper(ch, TRUE)) == NULL )   return;
                if (keeper->pIndexData->pShop->currency > -1) currency = keeper->pIndexData->pShop->currency;

                if (get_cust_rating(keeper->pIndexData->pShop, ch->name) < -10) {
                      do_say(keeper, "You're not welcome here.");
                      return;
                }

                one_argument(argument,arg);

	found = FALSE;
	for ( obj = keeper->carrying; obj; obj = obj->next_content )	{
	    if ( obj->wear_loc == WEAR_NONE
	    &&   can_see_obj( ch, obj )
	    &&   (cost = get_cost( keeper, obj, TRUE ) ) > 0 
	    &&   ( arg[0] == '\0'  
 	       ||  is_name(arg,obj->name) ))	    {
		if ( !found )		{
		    found = TRUE;
		    sprintf_to_char(ch, "[{gLv {y%10s   {rQty{w] Item\r\n", capitalize(flag_string(currency_type, currency)));
		}


        if (IS_OBJ_STAT(obj, ITEM_INVENTORY))  {
            sprintf(buf,"[{G%2d     {Y%5d    {R-- {w] {W%s\r\n", obj->level,cost,obj->short_descr);
        } else {
            count = 1;

            while (obj->next_content != NULL
            && obj->pIndexData == obj->next_content->pIndexData
            && !str_cmp(obj->short_descr, obj->next_content->short_descr)) {
               obj = obj->next_content;
               count++;
            }

            if (keeper->spec_fun > 0
            && !str_cmp("spec_translator", spec_string(keeper->spec_fun))
            && obj->item_type == ITEM_BOOK) {
                if (obj->condition < 100) sprintf(buf,"[{G%2d     {Y%5d    {R%2d {w] {W%s ({r%d %% done{W) => %s\r\n", obj->level, cost, count, obj->short_descr, obj->condition, obj->owner);
                else sprintf(buf,"[{G%2d     {Y%5d    {R%2d {w] {W%s ({g%d %% done{W) => %s\r\n", obj->level, cost, count, obj->short_descr, obj->condition, obj->owner);
            } else {
                sprintf(buf,"[{G%2d     {Y%5d    {R%2d {w] {W%s\r\n", obj->level,cost,count,obj->short_descr);
            }

            if (keeper->spec_fun > 0
            &&!str_cmp("spec_mechanic", spec_string(keeper->spec_fun))
            && obj->item_type == ITEM_PORTAL
            && obj->value[4] == PORTAL_VEHICLE) {
                if (obj->condition < 100) sprintf(buf,"[{G%2d     {Y%5d    {R%2d {w] {W%s ({r%d %% done{W) => %s\r\n", obj->level, cost, count, obj->short_descr, obj->condition, capitalize(check_obj_owner(obj)));
                else sprintf(buf,"[{G%2d     {Y%5d    {R%2d {w] {W%s ({g%d %% done{W) => %s\r\n", obj->level, cost, count, obj->short_descr, obj->condition, capitalize(check_obj_owner(obj)));
            } else {
                if (obj->owner !=NULL) {
                     if (obj->item_type == ITEM_FIGURINE && obj->value[2] != FIGURINE_TRUE) {
                         sprintf(buf,"[{G%2d     {Y%5d    {R%2d {w] {W%s => %s{x\r\n", obj->level, cost, count, obj->short_descr, capitalize(obj->owner));
                     } else {
                         if (obj->condition < 100) sprintf(buf,"[{G%2d     {Y%5d    {R%2d {w] {W%s ({r%d %% done{W) => %s\r\n", obj->level, cost, count, obj->short_descr, obj->condition, capitalize(obj->owner));
                         else sprintf(buf,"[{G%2d     {Y%5d    {R%2d {w] {W%s ({g%d %% done{W) => %s\r\n", obj->level, cost, count, obj->short_descr, obj->condition, capitalize(obj->owner));
                     }
                }else {
                     sprintf(buf,"[{G%2d     {Y%5d    {R%2d {w] {W%s\r\n", obj->level,cost,count,obj->short_descr);
                }
            }
        }
        send_to_char( buf, ch );
        }
    }

    if ( !found ) send_to_char( "You can't buy anything here.\r\n", ch );

    if ((pass = get_passport_order(ch)) != NULL) {
       bool ok = FALSE;
       int i;

       for(i = 0; i < MAX_TRADE; i++) {
            if (keeper->pIndexData->pShop->buy_type[i] == 994) ok = TRUE;
       }

       if (ok) {
            sprintf(buf, "Your passport will be ready around %s-%d.", month_name[pass->month], pass->day);
            do_say(keeper, buf);
       }
    }
    }
    return;
}


void do_translate( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;

    smash_tilde(argument);
    one_argument( argument, arg );

    if ( arg[0] == '\0' )    {
	send_to_char( "Translate what?\r\n", ch );
	return;
    }

    keeper = find_keeper(ch, TRUE);
    if ( keeper == NULL ) return;


    if (keeper->spec_fun == 0
    || str_cmp("spec_translator", spec_string(keeper->spec_fun))) {
	send_to_char( "There is no translator.\r\n", ch );
	return;
    }

    obj = get_obj_carry(ch, arg );

    if ( obj == NULL ) {
	act( "$n tells you 'You don't have that item'.", keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if (obj->item_type != ITEM_BOOK) {
	act( "$n tells you 'I can't translate that'.", keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if ( !can_drop_obj( ch, obj ) )    {
	send_to_char( "You can't let go of it.\r\n", ch );
	return;
    }

    if (!can_see_obj(keeper,obj))    {
	act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
	return;
    }

    if (obj->level > ( keeper->level + 10 )) {
        act_new("{g$n says 'I can't translate that, it's to cryptic!!'{x", keeper, NULL, ch, TO_ROOM, POS_DEAD);
        return;
    }

    if (get_skill(keeper, obj->value[0]) < 30) {
        act_new("{g$n says 'I can't translate that, don't know that language!!'{x", keeper, NULL, ch, TO_ROOM, POS_DEAD);
        return;
    }

   /* Won't buy damaged goods... */

    if (obj->condition < 20) {
      send_to_char("That is to damaged for me to translate.\r\n", ch);
      return;
    }

   /* won't buy rotting goods */

    if ( obj->timer != 0) {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

    act( "$n gives $p to $N.", ch, obj, keeper, TO_ROOM );
    act( "$n examines $p briefly.", keeper, obj, NULL, TO_ROOM );

     obj_from_char(obj);
     obj_to_keeper(obj, keeper);

     obj->condition = 1;
     obj->cost = obj->value[1] * obj->level * 20;     
     free_string(obj->owner);
     obj->owner = str_dup(ch->name);

      do_say(keeper, "I'll try to get them done as soon as possible.");
      return;
}


void do_sell( CHAR_DATA *ch, char *argument ) {
char buf[MAX_STRING_LENGTH];
char arg[MAX_INPUT_LENGTH];
CHAR_DATA *keeper;
OBJ_DATA *obj;
int cost;
int currency = find_currency(ch);

    smash_tilde(argument);
    one_argument( argument, arg );

    if ( arg[0] == '\0' )    {
	send_to_char( "Sell what?\r\n", ch );
	return;
    }

    keeper = find_keeper(ch, TRUE);
    if ( keeper == NULL ) return;

    if (get_cust_rating(keeper->pIndexData->pShop, ch->name) < -10) {
            do_say(keeper, "You're not welcome here.");
            return;
    }

    if (keeper->pIndexData->pShop->currency > -1) currency = keeper->pIndexData->pShop->currency;
    
    obj = get_obj_carry( ch, arg );

    if ( obj == NULL ) {
	act( "$n tells you 'You don't have that item'.", keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if ( !can_drop_obj( ch, obj ) )    {
	send_to_char( "You can't let go of it.\r\n", ch );
	return;
    }

    if (!can_see_obj(keeper,obj))    {
	act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
	return;
    }

    if (obj->level > ( keeper->level + 10 )) {
        act_new("{g$n says 'I can't buy that, it's to powerful!!'{x", keeper, NULL, ch, TO_ROOM, POS_DEAD);
        return;
    }

    if (keeper->spec_fun > 0
    && !str_cmp("spec_translator", spec_string(keeper->spec_fun))) {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

   /* Won't buy damaged goods... */

    if (obj->condition < 20) {
      send_to_char("That is to damaged for me to buy. Just sacrifice it.", ch);
      return;
    }

    cost = get_cost( keeper, obj, FALSE );
    if (obj->pIndexData->vnum <= 65) cost = cost/25 + 1;    

    if ( obj->timer != 0  || cost <= 10 ) {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

    if (!IS_NPC(ch)
    && get_skill(ch, gsn_haggle) > 0
    && IS_SET(ch->plr, PLR_HAGGLE)) {
        sprintf(buf, "I'll give you %d %s for %s.", cost, flag_string(currency_type, currency), obj->short_descr);
        do_say(keeper, buf);
        send_to_char("Accept?\r\n", ch);
        ch->pcdata->store_char = keeper;
        ch->pcdata->store_obj = obj;
        ch->pcdata->store_number[0] = cost;
        ch->desc->connected = CON_CONFIRM_SELL;

    } else {
        execute_sell(ch, keeper, obj, cost);
    }

    return;
}


void execute_sell(CHAR_DATA *ch, CHAR_DATA *keeper, OBJ_DATA *obj, int cost) {
char buf[MAX_STRING_LENGTH];
int currency = find_currency(ch);

    if (!keeper || !obj || cost <= 0) return;
    if (obj->carried_by != ch) {
        send_to_char("You don't have that anymore.\r\n", ch);
        return;
    }

    if (keeper->pIndexData->pShop->currency > -1) currency = keeper->pIndexData->pShop->currency;

    if ( cost > keeper->gold[currency] ) {
	act("$n tells you 'I'm afraid I don't have enough money to buy $p.",  keeper,obj,ch,TO_VICT);
	return;
    }

    act( "$n sells $p.", ch, obj, NULL, TO_ROOM );

    sprintf( buf, "You sell $p for %d %s.",cost, flag_string(currency_type, currency));
    act( buf, ch, obj, NULL, TO_CHAR );

    ch->gold[currency]     += cost;
    keeper->gold[currency] -= cost;

    if ( keeper->gold[currency] < 0 ) keeper->gold[currency] = 0;

    if ( obj->item_type == ITEM_TRASH 
    || IS_SET(obj->extra_flags, ITEM_SELL_EXTRACT) ) {
        extract_obj( obj );
    } else {
        obj_from_char( obj );
        obj_to_keeper( obj, keeper );
    }

    if (number_percent() < 10) improve_rating(keeper->pIndexData->pShop, ch->name);
    return;
}


void do_value( CHAR_DATA *ch, char *argument ) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )   {
	send_to_char( "Value what?\r\n", ch );
	return;
    }

    if ( ( keeper = find_keeper(ch, TRUE)) == NULL ) return;

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )   {
	act( "$n tells you 'You don't have that item'.", keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if (!can_see_obj(keeper,obj))    {
        act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
        return;
    }

    if ( !can_drop_obj( ch, obj ))   {
	send_to_char( "You can't let go of it.\r\n", ch );
	return;
    }

    if (obj->level > ( keeper->level + 10 )) {
        act_new("{g$n says 'I can't buy that, it's to powerful!!'{x", keeper, NULL, ch, TO_ROOM, POS_DEAD);
        return;
    }

    cost = get_cost( keeper, obj, FALSE );

    if ( cost <= 10 ) {
        act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
        return;
    }
 
    if ( cost <= keeper->gold[find_currency(ch)] ) {
      sprintf( buf, "$n tells you '%d %s is all I will pay for $p'.", cost, flag_string(currency_type, find_currency(ch)));
    } else { 
      sprintf( buf, "$n tells you 'If I had it, I would pay you %d %s for $p'.", cost, flag_string(currency_type, find_currency(ch)));
    }

    act( buf, keeper, obj, ch, TO_VICT );
    ch->reply = keeper;

    return;
}

/* Zeran - check size of object to wear, and kick out size message. */ 
bool wear_obj_size (CHAR_DATA *ch, OBJ_DATA *obj) {
	if (obj->size == SIZE_UNKNOWN) 	return TRUE;
	if (IS_IMMORTAL(ch)) return TRUE;	
                if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)) return TRUE;

	if (ch->level <= 10) {
		obj->size = get_char_size(ch);
		return TRUE;
	}

	if (obj->size < get_char_size(ch)) {
		send_to_char ("That object is too small for you!\r\n",ch);
		act( "$n tries to use $p, but it is too small for him.", ch, obj, NULL, TO_ROOM );
		return FALSE;
	}

	if (obj->size > get_char_size(ch)) {
		send_to_char ("That object is too large for you!\r\n",ch);
		act( "$n tries to use $p, but it is too large for him.", ch, obj, NULL, TO_ROOM );
		return FALSE;
	}

	return TRUE;
}	

	
void do_resize( CHAR_DATA *ch, char *argument ) {
	char *remainder;
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;

    remainder = one_argument( argument, arg );

    if ( arg[0] == '\0' )   {
	send_to_char( "Resize what?\r\n", ch );
	return;
    }

    if ( ( keeper = find_keeper(ch, TRUE)) == NULL ) return;

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	act( "$n tells you 'You don't have that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\r\n", ch );
	return;
    }

    if (!can_see_obj(keeper,obj)) {
	act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
	return;
    }
    
    if ( obj->timer || ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )  {
	act( "$n looks very puzzled, he doesn't seem to know how to resize $p.", keeper, obj, ch, TO_VICT );
	return;
    }
	
	if (obj->size == get_char_size(ch)) {
		act( "$n tells you 'Quit wasting my time, that thing is already perfectly sized for you.'", keeper, NULL, ch, TO_VICT );
		return;
	}
		
	cost = (abs(get_char_size(ch) - obj->size) / 10) * obj->pIndexData->cost;
	remainder = one_argument (remainder, arg);
	if (!str_cmp (arg, "cost"))
		{
		char outbuf[128];
		sprintf (outbuf, "$n tells you 'Resizing that will cost %d %s.'",cost, flag_string(currency_type, find_currency(keeper)));
		act (outbuf, keeper, NULL, ch, TO_VICT );
		return;
		}	
	
	if (cost > ch->gold[find_currency(ch)]) {
		char outbuf[128];
		sprintf (outbuf, "$n tells you 'Resizing that will cost %d %s.'",cost, flag_string(currency_type, find_currency(ch)));
		act (outbuf, keeper, NULL, ch, TO_VICT ); 
                                act( "$n tells you 'You don't have enough money'.", keeper, NULL, ch, TO_VICT );
		return;
		}
	
    act( "$n painstakingly resizes $p.", keeper, obj, NULL, TO_ROOM );
	obj->size = get_char_size(ch);
	WAIT_STATE (ch, 2*PULSE_VIOLENCE);

    ch->gold[find_currency(ch)]     -= cost;
    keeper->gold[find_currency(ch)] += cost;
    return;
}

bool do_real_repair(CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *keeper, bool cost_only, bool all);  

void do_repair( CHAR_DATA *ch, char *argument ){
char *remainder;
char arg[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
CHAR_DATA *keeper;
OBJ_DATA *obj;
OBJ_DATA *obj_next;
bool cost_only;
bool msg;

   /* Must specify item... */

    remainder = one_argument( argument, arg );

    if ( arg[0] == '\0' ) {
	send_to_char( "Syntax: repair <item|all> <cost>\r\n", ch );
	return;
    }

   /* See if they just want the costs... */

    remainder = one_argument (remainder, arg2);

    if (!str_cmp (arg2, "cost")) cost_only = TRUE;
    else cost_only = FALSE;
    
   /* Locate the shop keeper... */

    keeper = find_keeper(ch, TRUE);
    if ( keeper == NULL ) return;
  
    if (get_cust_rating(keeper->pIndexData->pShop, ch->name) < -10) {
            do_say(keeper, "You're not welcome here.");
            return;
    }
  
   /* All or a single object? */
 
    if (!str_cmp(arg, "all")) {
         msg = FALSE;
         for ( obj = ch->carrying; obj != NULL; obj = obj_next) {
               obj_next = obj->next_content;

               if ( obj->wear_loc == WEAR_NONE
               && can_see_obj( ch, obj )) { 
                     msg = msg | do_real_repair(ch, obj, keeper, cost_only, TRUE);
               }
         }
     
          if (!msg) {
               send_to_char("{yYou don't have anything that can be repaired here.{x\r\n", ch);
               return;
          } else {
               act("$n begins to repair some things.", keeper, NULL, NULL, TO_ROOM);
               send_to_char("Ok.\r\n",ch);
          }
    } else {
          obj = get_obj_carry( ch, arg );
          if ( obj == NULL ) {
               act( "$n tells you 'You don't have that item'.", keeper, NULL, ch, TO_VICT );
               ch->reply = keeper;
               return;
          }

          if (do_real_repair(ch, obj, keeper, cost_only, FALSE)) {
              act("$n begins to repair $T.", keeper, NULL, arg, TO_ROOM);
          }
    } 
    return;
}


bool do_real_repair(CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *keeper, bool cost_only, bool all) {
int cost;

   /* You must be able to let go of it... */

    if ( !can_drop_obj( ch, obj ) ) {
         sprintf_to_char(ch, "{y%35s: You can't drop it{x\r\n", obj->short_descr);  
         return FALSE;
    }

    if (obj->owner != NULL) {
         sprintf_to_char(ch, "{y%35s: %s can't take it{x\r\n", obj->short_descr, keeper->short_descr);  
         return FALSE;
    }

   /* The keeper must be able to see it... */

    if (!can_see_obj(keeper,obj)) {
            sprintf_to_char(ch, "{y%35s: The keeper can't see it{x\r\n",  obj->short_descr);
            return FALSE;
    }

   /* Can only repair things the keeper sells... */
  
    if ( get_cost( keeper, obj, FALSE ) <= 0 ) {
            sprintf_to_char(ch, "{y%35s: Can't repair it here{x\r\n",  obj->short_descr);
            return FALSE;
    }

   /* Can't repair decaying objects... */ 

    if (obj->timer != 0 
    || obj->item_type == ITEM_CONTAINER
    || obj->item_type == ITEM_POTION
    || obj->pIndexData->vnum < 65) { 
            sprintf_to_char(ch, "{y%35s: Can't be repaired{x\r\n", obj->short_descr);
            return FALSE;
    }

   /* Don't fix good stuff... */

    if (obj->condition == 100) {
         sprintf_to_char(ch, "{g%35s: Doesn't need repairing{x\r\n", obj->short_descr);
         return FALSE;
    }

   /* Don't fix junk... */

    if (obj->condition < 5) {
        sprintf_to_char(ch, "{y%35s: Is to broken to fix{x\r\n", obj->short_descr);
        return FALSE;
    }

   /* Work out the repair cost... */

    cost = obj->pIndexData->cost * (120 - obj->condition) / 100;

   /* If they just want the cost, we're done... */

    if (cost_only) {
        sprintf_to_char(ch, "{g%35s: Will cost {Y%d{g %s{x\r\n", obj->short_descr, cost, flag_string(currency_type, find_currency(ch)));   
        return FALSE;
    }	
	
   /* Repair time... */
   obj->cost = cost;
   free_string(obj->owner);
   obj->owner = strdup(ch->name);
   obj_from_char(obj);
   obj_to_char(obj, keeper);

   if (all) act("$n begins to repair $T.", keeper, NULL, obj->short_descr, TO_ROOM);
 
    return TRUE;
}

#define ACV_SEARCH_ROOM_MOB	1
#define ACV_SEARCH_ROOM_ITEM	2
#define ACV_SEARCH_ROOM_DOOR	3
#define ACV_SEARCH_ITEM		4

void do_search (CHAR_DATA *ch, char *argument) {
  OBJ_DATA *inside_obj;
  OBJ_DATA *ptr;
  EXIT_DATA *pexit;
  int base_chance;
  int door_chance;
  int chance;
  int count;
  int pInt = get_curr_stat (ch, STAT_INT);

  ROOM_INDEX_DATA *dest;

  bool ec_sat;
  CHAR_DATA *mob;

  MOB_CMD_CONTEXT *mcc;
  WEV *wev;

  char buf[MAX_STRING_LENGTH];

  inside_obj = NULL;

 /* Retrieve key from command... */

  if (!check_activity_key(ch, argument)) {
    return;
  }

 /* If already searching and not a recall, complain... */

  if ( ch->activity == ACV_SEARCHING
    && argument[0] != '*' ) {
    send_to_char("You are already searching!\r\n", ch);
    return;
  }

 /* Avoid problems switching activities... */

  if (ch->activity != ACV_SEARCHING) {
    ch->acv_state = 0;
  }

 /* Get searching skill... */

  base_chance = get_skill(ch, gsn_search);

 /* Switch to approrpiate routine... */

  switch ( ch->acv_state ) {

   /* Default is to start a new search... */ 

    default:
  
     /* Look inside an object (search object) */
	
      if (argument[0] != '\0') {

        inside_obj = get_obj_here (ch, argument);

        if (inside_obj == NULL) {
          send_to_char ("You can't find it.\r\n",ch);
          return;
        }

        mcc = get_mcc(ch, ch, NULL, NULL, inside_obj, NULL, 0, NULL);

        wev = get_wev(WEV_SEARCH, WEV_SEARCH_ITEM, mcc,
                     "You start searching @p2.\r\n",
                      NULL,
                     "@a2 starts searching @p2.\r\n");

        sprintf(buf, "searching inside %s.", inside_obj->short_descr);

      } else {
 
        mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL);

        wev = get_wev(WEV_SEARCH, WEV_SEARCH_ROOM, mcc,
                     "You start searching the room.\r\n",
                      NULL,
                     "@a2 starts searching the room.\r\n");

        strcpy(buf, "searching the room");
      }

     /* Challange... */

      if (!room_issue_wev_challange(ch->in_room, wev)) {
        free_wev(wev);
        return;
      }

     /* Send to command issuers room... */

//    room_issue_wev( ch->in_room, wev);

      set_activity( ch, ch->position, NULL, ACV_SEARCHING, buf);

     /* Free the wev (will autofree the context)... */

      free_wev(wev);

     /* Set up the activity state engine... */

      set_activity_key(ch); 

      if (inside_obj == NULL) {
        ch->acv_state = ACV_SEARCH_ROOM_MOB;
      } else {
        ch->acv_state = ACV_SEARCH_ITEM;
        free_string(ch->acv_text);
        ch->acv_text = str_dup(inside_obj->name);
      }

     /* Schedule the callback... */

      schedule_activity( ch, 2, "Search" );

      break;

   case ACV_SEARCH_ITEM:

      if ( ch->acv_text == NULL ) {
        send_to_char( "You can no longer find it!\r\n", ch );
        set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
        return;
      }

      inside_obj = get_obj_here( ch, ch->acv_text );

      if ( inside_obj == NULL ) {
        send_to_char( "You can no longer find it!\r\n", ch );
        set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
        return;
      }

     /* Change base from -5* to -3* to give idiots a chance          Mik */

      base_chance = base_chance + 3 * (pInt - 80)	;

      ptr = inside_obj->contains;

      for (;ptr != NULL; ptr=ptr->next_content) {
        if (IS_SET(ptr->extra_flags, ITEM_CONCEALED)) { 
         
          REMOVE_BIT(ptr->extra_flags, ITEM_CONCEALED);
 
          if (can_see_obj(ch, ptr)) {

            chance = base_chance + (ch->level - ptr->level);

            chance = URANGE (1, chance, 99);

            if (number_percent() < chance) {

              mcc = get_mcc(ch, ch, NULL, NULL, ptr, inside_obj, 0, NULL);

              wev = get_wev(WEV_SEARCH, WEV_FIND_ITEM, mcc,
                           "{YYou find @p2 inside @s2!{x\r\n",
                           "{Y@a2 finds something inside @s2!{x\r\n",
                           "{Y@a2 finds something inside @s2!{x\r\n");

              room_issue_wev(ch->in_room, wev);
  
              free_wev(wev);

            } else {

              SET_BIT(ptr->extra_flags, ITEM_CONCEALED);
            }
          } 
        }
      }

     /* You only get one pass searching an item... */

      send_to_char("You do not find anything of interest.\r\n", ch);
   
      set_activity( ch, ch->position, NULL, ACV_NONE, NULL);

      break;

   /* Search the room for Mobs... */
 
    case ACV_SEARCH_ROOM_MOB:

     /* Change base from -5* to -3* to give idiots a chance          Mik */

      base_chance = base_chance + 3 * (pInt - 80)	;

     /* Search for hidden mobs... */

      mob = ch->in_room->people;

      while (mob != NULL) {

        if (IS_AFFECTED(mob, AFF_HIDE)) {
          chance = base_chance + (ch->level - mob->level);
          chance = URANGE (1, chance, 99);

         /* See if you get jumped... */

          if (number_percent() < chance) {
            REMOVE_BIT(mob->affected_by, AFF_HIDE);
            mcc = get_mcc(ch, ch, mob, NULL, NULL, NULL, 0, NULL);
            wev = get_wev(WEV_SEARCH, WEV_FIND_MOB, mcc,
                         "{YYou are surprised by @v2!{x\r\n",
                         "{Y@a2 finds you!{x\r\n",
                         "{Y@a2 is jumped by @v2!{x\r\n");

            room_issue_wev(ch->in_room, wev);
  
            free_wev(wev);

            if (IS_NPC(mob)
            && IS_SET(mob->act, ACT_AGGRESSIVE)) {
              multi_hit(mob, ch, TYPE_UNDEFINED);
            }
 
            set_activity( ch, ch->position, NULL, ACV_NONE, NULL);

            return;
          }
        } 
 
        mob = mob->next_in_room;
      }

     /* Arrange the callback... */

      send_to_char("...Nobody lurking in ambush...\r\n", ch);

      ch->acv_state = ACV_SEARCH_ROOM_ITEM;
 
      schedule_activity( ch, 2, "search" );

      break;

   /* Search the room for items... */

    case ACV_SEARCH_ROOM_ITEM:
 
     /* Change base from -5* to -3* to give idiots a chance          Mik */

      base_chance = base_chance + 3 * (pInt - 80);

     /* Search for objects... */

      ptr = ch->in_room->contents;

      for (;ptr != NULL ; ptr=ptr->next_content) {
        if (IS_SET(ptr->extra_flags, ITEM_CONCEALED)) {
 
          REMOVE_BIT(ptr->extra_flags, ITEM_CONCEALED);
 
          if (can_see_obj (ch, ptr)) {

            chance = base_chance + (ch->level - ptr->level);
            chance = URANGE (1, chance, 99);
            if (number_percent() < chance) {

              mcc = get_mcc(ch, ch, NULL, NULL, ptr, inside_obj, 0, NULL);

              wev = get_wev(WEV_SEARCH, WEV_FIND_ITEM, mcc,
                           "{YYou find @p2!{x\r\n",
                           "{Y@a2 finds something!{x\r\n",
                           "{Y@a2 finds something!{x\r\n");
  
              room_issue_wev(ch->in_room, wev);

              free_wev(wev);
 
              set_activity( ch, ch->position, NULL, ACV_NONE, NULL);

              return;
            }
          }

          SET_BIT(ptr->extra_flags, ITEM_CONCEALED);

        } 
      }

     /* Arrange the callback... */

      send_to_char("...No hidden items...\r\n", ch);

      ch->acv_state = ACV_SEARCH_ROOM_DOOR;
 
      schedule_activity( ch, 2, "search" );

      break;

   /* Search the room for secret doors... */

    case ACV_SEARCH_ROOM_DOOR:

     /* Increase door chance by 25, change int from 5* to 3*  Mik */ 

      door_chance = base_chance + ch->level/3;

      if (pInt > 72) {    
        door_chance = door_chance + 3*(pInt - 72);
      }

     /* Search for secret doors... */

      for (count =0; count < DIR_MAX ; count++) {

       /* Must be an exit... */

        pexit = ch->in_room->exit[count];

        if ( pexit == NULL ) continue;

       /* Must be hidden and closed... */

        if ( IS_SET(pexit->exit_info, EX_CLOSED) 
          && IS_SET(pexit->exit_info, EX_HIDDEN) ) { 

         /* Must be visible... */

          if ( pexit->invis_cond != NULL ) {

            ec_sat = FALSE;

            mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0 , NULL);

            ec_sat = ec_satisfied_mcc(mcc, pexit->invis_cond, TRUE);

            free_mcc(mcc);

            if (!ec_sat) continue;
          }

         /* Must go somewhere... */

          dest = get_exit_destination(ch, ch->in_room, pexit, TRUE);

          if (dest == NULL) continue;

         /* See if we can find it... */

          if (number_percent() < door_chance) {

            pexit->exit_info -= EX_HIDDEN; 

            mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, count, dir_name[count]);

            wev = get_wev(WEV_SEARCH, WEV_FIND_DOOR, mcc,
                         "{YYou find a secret door leading @t0!{x\r\n",
                         "{Y@a2 finds a secret door leading @t0!{x\r\n",
                         "{Y@a2 finds a secret door leading @t0!{x\r\n");

            room_issue_wev(ch->in_room, wev);

            free_wev(wev);
 
            set_activity( ch, ch->position, NULL, ACV_NONE, NULL);

            return;
          }
        }
      }

     /* Arrange the callback... */

      send_to_char("...No secret doors...\r\n", ch);

      ch->acv_state = ACV_SEARCH_ROOM_ITEM;
 
      schedule_activity( ch, 2, "search" );

      break;

  }

  return;
}

/* Locate an idol relative to a character... */

OBJ_DATA *find_idol(CHAR_DATA *ch, char *desc) {

    OBJ_DATA *idol;

   /* See if they are holding one? */

    idol = get_obj_wear( ch, desc);

   /* Nope, so search the room... */

    if (idol == NULL) {
      idol = get_obj_here( ch, desc);
    } 

   /* Warn if nothing found... */

    if (idol == NULL) {
      send_to_char("No idol like that here.\r\n", ch);
      return NULL;
    }

   /* Return the idol... */

    return idol;
} 

/* Offer gold or an object to an idol... */

void do_offer( CHAR_DATA *ch, char *argument ) {
char arg1 [MAX_INPUT_LENGTH];
char arg2 [MAX_INPUT_LENGTH];
OBJ_DATA  *obj;
OBJ_DATA *idol;
ROOM_INDEX_DATA *room;
MOB_CMD_CONTEXT *mcc;
WEV *wev;  

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' 
      || arg2[0] == '\0' ) {
	send_to_char( "Syntax: offer idol item\r\n"
                      "        offer idol nnn gold\r\n", ch );
	return;
    }

   /* Hunt the idol time... */

    idol = find_idol(ch, arg1);

    if (idol == NULL) return;

     if (idol->condition < 5) {
             send_to_char( "It's much too damaged.\r\n", ch );
             return;
     }

   /* Offering gold... */

    if ( is_number( arg2 ) ) {
	/* 'give NNNN coins victim' */
	int amount;
                int currency;
 
	amount   = atoi(arg2);
                currency = flag_value(currency_accept, argument);
	if ( amount <= 0 || currency < 0) {
	    send_to_char( "Sorry, you can't do that.\r\n", ch );
	    return;
	}

	if ( ch->gold[currency] < amount ) {
	    sprintf_to_char(ch, "You haven't got that much %s.\r\n", flag_string(currency_type, currency));
	    return;
	}

       /* Ok, get context and build the wev... */

               mcc = get_mcc(ch, ch, NULL, NULL, idol, NULL, amount, flag_string(currency_type, currency));
               wev = get_wev(WEV_IDOL, WEV_IDOL_OFFER_GOLD, mcc,
                    "You offer @n0 @t0 to @p2.\r\n",
                     NULL,
                    "@a2 offers some @t0 to @p2.\r\n");

       /* Challange... */

               if (!room_issue_wev_challange(ch->in_room, wev)) {
                   free_wev(wev);
                   return;
               } 

       /* Make the transfer... */
	ch->gold[currency]     -= amount;

       /* Send to command issuers room... */
               room_issue_wev( ch->in_room, wev);

       /* Now, do we have any rooms to tell? */
               ch_issue_idol_wev( ch, wev);

       /* Free the wev (will autofree the context)... */
               free_wev(wev);
               return;
    }

   /* Offering an object... */

    obj = get_obj_carry( ch, arg2);

    if ( obj == NULL ) {
	send_to_char( "You do not have that item.\r\n", ch );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE ) {
	send_to_char( "You must remove it first.\r\n", ch );
	return;
    }

    if ( !can_drop_obj( ch, obj ) ) {
	send_to_char( "You can't let go of it.\r\n", ch );
	return;
    }

    if (IS_SET(obj->extra_flags, ITEM_ANIMATED)) {
	send_to_char( "This object belongs to you.\r\n", ch );
	return;
    }

    if (obj->item_type == ITEM_IDOL) {
	send_to_char( "But that's an Idol!\r\n", ch );
	return;
    }

   /* Ok, get context and build the wev... */

    mcc = get_mcc(ch, ch, NULL, NULL, idol, obj, 0, obj->description);
    wev = get_wev(WEV_IDOL, WEV_IDOL_OFFER_ITEM, mcc,
                "You offer @s2 to @p2.\r\n",
                 NULL,
                "@a2 offers @s2 to @p2.\r\n");

   /* Challange... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

   /* Make the transfer... */

    obj_from_char( obj );

    if (idol->item_type == ITEM_IDOL) {

      if (idol->value[0] == 0) {
        extract_obj(obj);
      } else {
        room = get_room_index(idol->value[0]);

        if (room != NULL) {
          obj_to_room( obj, room );
        } else {
          extract_obj(obj);
        }
      }
    } 

   /* Send to command issuers room... */

    room_issue_wev( ch->in_room, wev);

   /* Now, do we have any rooms to tell? */

    ch_issue_idol_wev( ch, wev);

   /* Free the wev (will autofree the context)... */

    free_wev(wev);

    return;
}

/* Pray to an an idol... */

void do_pray( CHAR_DATA *ch, char *argument ) {
    char arg1 [MAX_INPUT_LENGTH];
    OBJ_DATA *idol;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;  

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' 
      || argument[0] == '\0' ) {
	send_to_char( "Syntax: pray idol prayer\r\n", ch );
	return;
    }

   /* Hunt the idol time... */

    idol = find_idol(ch, arg1);

    if (idol == NULL) {
      return;
    } 

     if (idol->condition < 5) {
             send_to_char( "It's much too damaged.\r\n", ch );
             return;
     }

   /* Ok, get context and build the wev... */

    mcc = get_mcc(ch, ch, NULL, NULL, idol, NULL, ch->speak[SPEAK_PRIVATE], argument);
    wev = get_wev(WEV_IDOL, WEV_IDOL_PRAY, mcc,
                "You pray to @p2 for '@t0'.\r\n",
                 NULL,
                "@a2 prays to @p2.\r\n");

   /* Challange... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    room_issue_wev( ch->in_room, wev);
    ch_issue_idol_wev( ch, wev);
    free_wev(wev);

    mcc = get_mcc(ch, ch, NULL, NULL, idol, NULL, ch->speak[SPEAK_PRIVATE], argument);
    wev = get_wev(WEV_ICC, WEV_ICC_PRAY, mcc,
                 NULL,
                 NULL,
                "{W[@a2 --- @p2]: '{Y@t0{W'.{x\r\n");


    world_issue_wev(wev, "ICC");
    free_wev(wev);
    return;
}

void do_restring (CHAR_DATA *ch, char * argument) {
OBJ_DATA *obj;
CHAR_DATA *keeper;
char arg1 [MAX_INPUT_LENGTH];
char arg2 [MAX_INPUT_LENGTH];

    if ( argument[0] == '\0' ) {
	send_to_char( "Restring what?\r\n", ch );
	return;
    }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

    if ( ( obj = get_obj_carry( ch, arg2 ) ) == NULL ) {
	send_to_char( "You do not have that item.\r\n", ch );
	return;
    }

    if (obj->item_type == ITEM_DOLL
    || obj->pIndexData->vnum == 6
    || obj->pIndexData->vnum == 7) {
	send_to_char( "Don't be funny.\r\n", ch );
	return;
    }

    if (!IS_IMMORTAL(ch)) {
        
       keeper = find_keeper(ch, TRUE);        
       if ( keeper == NULL ) {
	send_to_char( "You can't do that here.\r\n", ch );
	return;
       }

       if (get_cust_rating(keeper->pIndexData->pShop, ch->name) < -10) {
            do_say(keeper, "You're not welcome here.");
            return;
       }

       if (!can_see_obj(keeper,obj)) {
	act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
	return;
       }

       if ( get_cost( keeper, obj, FALSE ) <= 0 ) {
	act("$n doesn't know much about such an object.",keeper,NULL,ch,TO_VICT);
	return;
       }

        if (ch->gold[find_currency(ch)] < 10000) {
	send_to_char( "You can't afford?\r\n", ch );
	return;
        }

      }

    if (!str_cmp(arg1, "name"   )) {
         free_string( obj->name );
         obj->name = str_dup( argument );
    }
    else if (!str_cmp(arg1, "short"     )) {
         free_string( obj->short_descr );
         obj->short_descr = str_dup( argument );
    }
    else if (!str_cmp(arg1, "long"     )) {
         free_string( obj->description );
         obj->description = str_dup( argument );
    } else {
          send_to_char ("SYNTAX: restring <name/short/long> <object> <text>\r\n",ch);
          return;
    }

     if (!IS_IMMORTAL(ch)) ch->gold[find_currency(ch)] -=10000;

    return;
}

void do_use( CHAR_DATA *ch, char *argument ) {
    char arg1[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
   MOB_CMD_CONTEXT *mcc;
    WEV *wev;

    argument = one_argument( argument, arg1 );
    
    if ( arg1[0] == '\0' )    {
	send_to_char( "Use what?\r\n", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL ) {
          obj = get_obj_list( ch, arg1, ch->in_room->contents );
          if ( obj == NULL) {
                act( "I see no $T here.", ch, NULL, arg1, TO_CHAR );
                 return;
          }
      }

      if (obj->item_type == ITEM_FIGURINE
      && obj->value[2] == FIGURINE_TRUE) {
          MOB_INDEX_DATA *pMob;  
          CHAR_DATA *mob;
          EXTRA_DESCR_DATA *ed;

          if (ch->pet) {
	send_to_char( "You already got a pet.\r\n", ch );
	return;
          }

          if ((pMob = get_mob_index(obj->value[0])) == NULL) return;
          if ((mob = create_mobile_level(pMob, NULL, obj->level )) == NULL) return;

          ed = obj->extra_descr;
          while (ed != NULL) {
               if (is_name("name", ed->keyword)) {
                      free_string(mob->name);
                      mob->name = str_dup(ed->description);
               }
               if (is_name("short", ed->keyword)) {
                      free_string(mob->short_descr);
                      mob->short_descr = str_dup(ed->description);
               }
               if (is_name("long", ed->keyword)) {
                   free_string(mob->long_descr);
                   free_string(mob->description);
                   mob->long_descr = str_dup(ed->description);
                   mob->description = str_dup(mob->long_descr);
               }
               ed = ed->next;
          }
          if (obj->value[3] >0) mob->race = obj->value[3];
          if (obj->owner) {
              if (str_cmp(obj->owner, ch->name)) {
                    int chance = (ch->level + 20 - obj->level) * 3;
                    if (number_percent() > chance) obj->value[1] = FIGURINE_AGGRESSIVE;
              }
          }

          if (obj->value[1] == FIGURINE_AGGRESSIVE) {
                char_to_room(mob, ch->in_room );
                SET_BIT(mob->act, ACT_AGGRESSIVE);
          } else if (obj->value[1] == FIGURINE_TAME) {
                char_to_room(mob, ch->in_room );
                REMOVE_BIT(mob->act, ACT_AGGRESSIVE);
                SET_BIT(mob->affected_by, AFF_CHARM);
                SET_BIT(mob->act, ACT_FOLLOWER);
                mob->comm = COMM_NOCHANNELS;

                add_follower(mob, ch );
                mob->leader = ch;
                ch->pet = mob;
                do_emote(mob, "looks at you expectently" );
          } else {
                char_to_room(mob, ch->in_room );
          }

          mcc = get_mcc(ch, ch, mob, NULL, obj, NULL, 0, NULL);
          wev = get_wev(WEV_OPROG, WEV_OPROG_USE, mcc,
                    "You use @p2 and @v2 appears.\r\n",
                    "@a2 uses @p2 and @v2 appears.\r\n",
                    "@a2 uses @p2 and @v2 appears.\r\n");

          room_issue_wev( ch->in_room, wev);
          free_wev(wev);
          extract_obj( obj );
          return;
   }


    if ( !IS_SET(obj->extra_flags, ITEM_USABLE)
    && !IS_SET(obj->extra_flags, ITEM_USABLE_INF) )    {
	send_to_char( "You can't use that.\r\n", ch );
	return;
    }

     mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);

     wev = get_wev(WEV_OPROG, WEV_OPROG_USE, mcc,
                    "You use @p2.\r\n",
                    "@a2 uses @p2.\r\n",
                    "@a2 uses @p2.\r\n");

     room_issue_wev( ch->in_room, wev);
     free_wev(wev);

     if (IS_SET(obj->extra_flags, ITEM_USABLE)) {
          extract_obj( obj );
          send_to_char( "Then it vanishes...\r\n", ch );
     }
     return;
}
	

void do_write(CHAR_DATA *ch, char *argument ) {
OBJ_DATA *obj;
EXTRA_DESCR_DATA *ed;
char arg[MAX_INPUT_LENGTH];
char buf[MAX_INPUT_LENGTH];

    if (IS_NPC(ch)) return;

    argument = one_argument( argument, arg );

    if (arg[0] == '\0')    {
        send_to_char( "Syntax: write <obj>\r\n", ch);
        return;
    }

     if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )	{
            obj = ch->in_room->contents;
            while ( obj != NULL
            && ( obj->item_type != ITEM_PAPER
            || !can_see_obj(ch, obj) ) ) {
                     obj = obj->next_content;
            }
            if (obj == NULL) {    
                send_to_char( "You do not have that item.\r\n", ch );
                return;
            }
     }

     if (obj->item_type != ITEM_PAPER) {
            send_to_char( "You can't write on that.\r\n", ch );
            return;
     }

/* Apply sequential ids to all of the ed records... */

     ed = obj->extra_descr;
     if (ed==NULL) {
          ed =   new_extra_descr();
          sprintf (buf, "read %s",obj->name);
          ed->keyword=str_dup( buf ); /* Mik */
          ed->description		=   str_dup( "" );
          ed->next		=   obj->extra_descr;
          obj->extra_descr	=   ed;
     }

     string_append( ch, &ed->description );
     return;
}


void do_reload( CHAR_DATA *ch, char *argument ) {
    OBJ_DATA *obj;

     if ((obj = get_eq_char( ch, WEAR_WIELD))  != NULL ) {
          if (obj->item_type == ITEM_WEAPON
          && (obj->value[0] == WEAPON_GUN
                 || obj->value[0] == WEAPON_HANDGUN
                 || obj->value[0] == WEAPON_SMG
                 || obj->value[0] == WEAPON_BOW)) {
                    WAIT_STATE(ch, 12);
                    reload_gun (ch,obj, 0, TRUE);
           }
     }

     if ((obj = get_eq_char( ch, WEAR_WIELD2)) != NULL ) {
          if (obj->item_type == ITEM_WEAPON
          && (obj->value[0] == WEAPON_GUN
                 || obj->value[0] == WEAPON_HANDGUN
                 || obj->value[0] == WEAPON_SMG
                 || obj->value[0] == WEAPON_BOW)) {
                    WAIT_STATE(ch, 24);
                    reload_gun (ch,obj, 1, TRUE);
           }
     }

     return;
}

void reload_gun ( CHAR_DATA *ch, OBJ_DATA *obj, int gnum, bool loud) {
    OBJ_DATA *ammo;
    bool ammodef;

         /* Assume no ammo... */       

            if (ch->gun_state[gnum] <=0) ch->gun_state[gnum] = GUN_NO_AMMO;

            ammodef = FALSE;            
            ammo = ch->carrying;

            if (IS_SET(obj->value[4], WEAPON_PARABELLUM)) {
                 ammodef = TRUE;            
                 while ( ammo != NULL
                 && ( ammo->item_type != ITEM_AMMO
                 || ammo->wear_loc != -1 
                 || !IS_SET(obj->value[4], WEAPON_PARABELLUM))) {
                      ammo = ammo->next_content;
                 }
            }
            if (IS_SET(obj->value[4], WEAPON_CAL45)) {
                 ammodef = TRUE;            
                 while ( ammo != NULL
                 && ( ammo->item_type != ITEM_AMMO
                 || ammo->wear_loc != -1 
                 || !IS_SET(obj->value[4], WEAPON_CAL45))) {
                      ammo = ammo->next_content;
                 }
            }
            if (IS_SET(obj->value[4], WEAPON_357MAGNUM)) {
                 ammodef = TRUE;            
                 while ( ammo != NULL
                 && ( ammo->item_type != ITEM_AMMO
                 || ammo->wear_loc != -1 
                 || !IS_SET(obj->value[4], WEAPON_357MAGNUM))) {
                      ammo = ammo->next_content;
                 }
            }
            if (IS_SET(obj->value[4], WEAPON_22RIFLE)) {
                 ammodef = TRUE;            
                 while ( ammo != NULL
                 && ( ammo->item_type != ITEM_AMMO
                 || ammo->wear_loc != -1 
                 || !IS_SET(obj->value[4], WEAPON_22RIFLE))) {
                      ammo = ammo->next_content;
                 }
            }
            if (IS_SET(obj->value[4], WEAPON_30CARABINE)) {
                 ammodef = TRUE;            
                 while ( ammo != NULL
                 && ( ammo->item_type != ITEM_AMMO
                 || ammo->wear_loc != -1 
                 || !IS_SET(obj->value[4], WEAPON_30CARABINE))) {
                      ammo = ammo->next_content;
                 }
            }
            if (IS_SET(obj->value[4], WEAPON_12GUAGE)) {
                 ammodef = TRUE;            
                 while ( ammo != NULL
                 && ( ammo->item_type != ITEM_AMMO
                 || ammo->wear_loc != -1 
                 || !IS_SET(obj->value[4], WEAPON_12GUAGE))) {
                      ammo = ammo->next_content;
                 }
            }
            if (IS_SET(obj->value[4], WEAPON_ARROW)) {
                 ammodef = TRUE;            
                 while ( ammo != NULL
                 && ( ammo->item_type != ITEM_AMMO
                 || ammo->wear_loc != -1 
                 || !IS_SET(obj->value[4], WEAPON_ARROW))) {
                      ammo = ammo->next_content;
                 }
            }
            if (IS_SET(obj->value[4], WEAPON_ENERGY)) {
                 ammodef = TRUE;            
                 while ( ammo != NULL
                 && ( ammo->item_type != ITEM_AMMO
                 || ammo->wear_loc != -1 
                 || !IS_SET(obj->value[4], WEAPON_ENERGY))) {
                      ammo = ammo->next_content;
                 }
            }
            if (IS_SET(obj->value[4], WEAPON_458WINCHESTER)) {
                 ammodef = TRUE;            
                 while ( ammo != NULL
                 && ( ammo->item_type != ITEM_AMMO
                 || ammo->wear_loc != -1 
                 || !IS_SET(obj->value[4], WEAPON_458WINCHESTER))) {
                      ammo = ammo->next_content;
                 }
            }

           if (!ammodef) {
          if (IS_SET(obj->value[4], WEAPON_ONE_SHOT)) {
            ch->gun_state[gnum] = 1;
          } else { 
            if (IS_SET(obj->value[4], WEAPON_TWO_SHOT)) {
              ch->gun_state[gnum] = 2;
            } else {
              if (IS_SET(obj->value[4], WEAPON_SIX_SHOT)) {
                ch->gun_state[gnum] = 6;
              } else {
                if (IS_SET(obj->value[4], WEAPON_TWELVE_SHOT)) {
                  ch->gun_state[gnum] = 12;
                } else {
                  if (IS_SET(obj->value[4], WEAPON_36_SHOT)) {
                    ch->gun_state[gnum] = 36;
                  } else {
                    if (IS_SET(obj->value[4], WEAPON_108_SHOT)) {
                      ch->gun_state[gnum] = 108;
                    }
                  }  
                }
              }
            }
          }   
          } else {

            if (ammo == NULL) {
                    if (loud) send_to_char("You got no fitting ammo.\r\n",ch);
            } else {
          
         /* How much do we give... */ 

          if (IS_SET(obj->value[4], WEAPON_ONE_SHOT)) {
            ammo->value[0] += ch->gun_state[gnum] - 1;
            ch->gun_state[gnum] = 1;
          } else { 
            if (IS_SET(obj->value[4], WEAPON_TWO_SHOT)) {
              ammo->value[0] += ch->gun_state[gnum] - 2;
              ch->gun_state[gnum] = 2;
            } else {
              if (IS_SET(obj->value[4], WEAPON_SIX_SHOT)) {
                ammo->value[0] += ch->gun_state[gnum] - 6;
                ch->gun_state[gnum] = 6;
              } else {
                if (IS_SET(obj->value[4], WEAPON_TWELVE_SHOT)) {
                  ammo->value[0] += ch->gun_state[gnum] - 12;
                  ch->gun_state[gnum] = 12;
                } else {
                  if (IS_SET(obj->value[4], WEAPON_36_SHOT)) {
                    ammo->value[0] += ch->gun_state[gnum] - 36;
                    ch->gun_state[gnum] = 36;
                  } else {
                    if (IS_SET(obj->value[4], WEAPON_108_SHOT)) {
                      ammo->value[0] += ch->gun_state[gnum] - 108;
                      ch->gun_state[gnum] = 108;
                    }
                  }  
                }
              }
            }
          }   
          if (ammo->value[0]<=0) {
                ch->gun_state[gnum] += ammo->value[0];
                obj_from_char(ammo);
                extract_obj(ammo);
          }
        }
        }
         /* Warn if we didn't give any... */ 

          if (loud) {
               if (ch->gun_state[gnum] == GUN_NO_AMMO) {
                  act("You don't have any ammunition for $p", ch, obj, NULL, TO_CHAR);
               } else {
                   if (obj->value[0] == WEAPON_BOW) {
                        act("You ready $p.", ch, obj, NULL, TO_CHAR);
                   } else {
                        act("You clear and load $p.", ch, obj, NULL, TO_CHAR);
                   }
               }
        }
        return;
}


void do_gamble( CHAR_DATA *ch, char *argument ) {
char    arg1[MAX_INPUT_LENGTH];
OBJ_DATA       *obj;

     argument = one_argument(argument, arg1);
  
     if ( arg1[0] == '\0' )    {
         send_to_char("What do you want to play?\r\n", ch);
         return;
     }

     obj = get_obj_list( ch, arg1, ch->in_room->contents);
     if ( obj == NULL ) {
         act( "I see no $T here.", ch, NULL, arg1, TO_CHAR );
         return;
     }

     if (obj->item_type != ITEM_SLOTMACHINE) {
         send_to_char("This is no slot machine.\r\n", ch);
         return;
     }

     if (obj->condition < 5) {
             send_to_char( "It's much too damaged.\r\n", ch );
             return;
     }

     slotmachine(ch, obj);
     return;
}


void slotmachine( CHAR_DATA *ch, OBJ_DATA *slotMachine) {
  char   buf[MAX_STRING_LENGTH];
  char   soc_acnt[MAX_STRING_LENGTH];
  CHAR_DATA *teller;
  BANK *bp;
  ACCOUNT *ap;
  int cost, jackpot, winnings, numberMatched;
  int bar1, bar2, bar3;
  bool won, wonJackpot;

  char *bar_messages[] = {
    "<------------>",
    "{YGold Coin{x",               
    "{R Cherry  {x",
    "{MTommyGun {x",           
    "{c Cthulhu {x",
    "{C  Bell   {x",             
    "{y Orange  {x",
    "{rFireball {x",
    "{G DeepOne {x",
    "{W  Star   {x",
    "{B Arqubis {x",      
  };

  cost = slotMachine->value[0];
  if(cost <= 0)   {
      send_to_char("This slot machine seems to be broken.\r\n", ch);
      return;
    }

  if(cost > ch->gold[0])    {
      sprintf(buf, "This slot machine costs %d %s to play.\r\n", cost, flag_string(currency_type, 0));
      send_to_char(buf, ch);
      return;
    }

  ch->gold[0] -= cost;
  slotMachine->value[1] += (cost * 75)/100;
  if (slotMachine->value[1] >30000) slotMachine->value[1] = 30000;
 
  if (slotMachine->value[2] >0) {
           teller = find_teller(ch);
           if ( teller == NULL ) {
               slotMachine->value[2]=0;
               bug( "Slot Machine Society not found => Reset.", 0 );
           } else {
               sprintf(soc_acnt, "Society_%ld", slotMachine->value[2]);
               bp = find_bank(ch->in_room->name, TRUE);
               ap = find_account(bp, soc_acnt, TRUE);
               ap->gold += cost; 
               save_banks();
           }
  }

  sprintf(buf, "You insert %d %s.\r\n", cost, flag_string(currency_type, 0));
  send_to_char(buf, ch);
  act( "$n inserts money in a slot machine.", ch, NULL, NULL, TO_ROOM );
  jackpot = slotMachine->value[1];

  WAIT_STATE(ch, 6);
 
  bar1 = number_range( 1, 10 );
  bar2 = number_range( 1, 10 );
  bar3 = number_range( 1, 10 );

  send_to_char("{g////------------{MSlot Machine{g------------\\\\\\\\{x\r\n", ch);
  sprintf(buf, "{g|{C{{}{g|{x  %s  %s  %s  {g|{C{{}{g|{x\r\n", bar_messages[bar1],bar_messages[bar2], bar_messages[bar3]);
  send_to_char(buf, ch);
  send_to_char("{g\\\\\\\\------------------------------------////{x\r\n", ch);

  wonJackpot = FALSE;
  winnings = 0;
  won = FALSE;
  numberMatched = 0;

  if( (bar1 == bar2) && (bar2 == bar3) ) {
     winnings = jackpot;  /* they won the jackpot, make it */
     won = TRUE;          /* worth their while!            */
     wonJackpot = TRUE;
  }

  if(!won)    {
          if(bar1 == bar2)            {
              winnings += cost;
              won = TRUE;
              numberMatched++;
            }
          if(bar1 == bar3)            {
              numberMatched++;
              if(won) winnings += cost;
              else  {
                  winnings += cost;
                  won = TRUE;
              }
          }
          if(bar2 == bar3) {
              numberMatched++;
              if(won) winnings += cost;
              else  {
                  winnings += cost;
                  won = TRUE;
              }
          }
  }

  if (slotMachine->value[2] >0) {
           teller = find_teller(ch);
           if ( teller == NULL ) {
               slotMachine->value[2]=0;
               bug( "Slot Machine Society not found => Reset.", 0 );
           } else {
               sprintf(soc_acnt, "Society_%ld", slotMachine->value[2]);
               bp = find_bank(ch->in_room->name, TRUE);
               ap = find_account(bp, soc_acnt, TRUE);

               if (ap->gold < winnings) {
                       winnings=ap->gold;
                       send_to_char("{RThe casino goes bankrupt!{x\r\n", ch);
               }
               ap->gold -=winnings;
               if (wonJackpot) ap->gold -=slotMachine->value[0]*10;
               if (ap->gold < 0) ap->gold=0;
               save_banks();
           }
      
  }

  ch->gold[0] += winnings;
  slotMachine->value[1] -= winnings;
  if (wonJackpot) slotMachine->value[1]=slotMachine->value[0]*10;

  if(won && wonJackpot)    {
      sprintf(buf, "{YYou won the jackpot worth %d {rCopper{x!!{*\r\n"
                   "{xThe jackpot now stands at %ld {rCopper{x.\r\n", 
                    winnings, slotMachine->value[1]);
      send_to_char(buf, ch);
  }
  if(won && !wonJackpot)    {
      sprintf(buf, "You matched %d bars and won %d {rCopper{x!\r\n"
                   "The jackpot is now worth %ld {rCopper{x.\r\n", 
                    numberMatched, winnings, slotMachine->value[1]);
      send_to_char(buf, ch);
  }
  if(!won)   {
      sprintf(buf, "Sorry you didn't win anything. The jackpot is now worth %ld {rCopper{x.\r\n",
                    slotMachine->value[1]);
      send_to_char(buf, ch);
    }

  return;
}


void do_tamp (CHAR_DATA *ch, char *argument)	{
               	OBJ_DATA *pipe;
                OBJ_DATA *herb;
                char    arg[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )    {
	send_to_char( "Tamp what?\r\n", ch );
	return;
    }
   
    if ( ( pipe = get_obj_carry( ch, arg ) ) == NULL ) {
	    send_to_char( "You do not have that item.\r\n", ch );
	    return;
	}

    if (pipe->item_type !=ITEM_PIPE) {
	send_to_char( "This is no pipe.\r\n", ch );
	return;
    }

    if ( ( herb = get_eq_char( ch, WEAR_HOLD ) ) == NULL )    {
	send_to_char( "You are not holding any herbs.\r\n", ch );
	return;
    }

    if (herb->item_type !=ITEM_HERB) {
	send_to_char( "You are not holding any herbs.\r\n", ch );
	return;
    }

    if (pipe->value[1] != TRUE) {
	send_to_char( "You can't tamp this kind of smoke.\r\n", ch );
	return;
    }

    if (pipe->value[2] !=0
    || pipe->value[3] !=0
    || pipe->value[4] !=0) {
	send_to_char( "You empty the pipe first.\r\n", ch );
    }

    pipe->value[2] = UMAX(herb->value[0]-15,1);
    pipe->value[3] = herb->value[1];
    pipe->value[4] = herb->value[2];
    obj_from_char(herb);
    extract_obj(herb);
    send_to_char("You tamp your pipe.\r\n", ch);
    act( "$n tamps a pipe.", ch, NULL, NULL, TO_ROOM );
    return;
}


void do_smoke (CHAR_DATA *ch, char *argument)	{
               	OBJ_DATA *pipe;
                int level;

    if ( ( pipe = get_eq_char( ch, WEAR_FACE)) == NULL )    {
	send_to_char( "You better wear a pipe first.\r\n", ch );
	return;
    }

    if (pipe->item_type !=ITEM_PIPE) {
	send_to_char( "This is no pipe.\r\n", ch );
	return;
    }
   
    if (pipe->value[2] ==0
    || pipe->value[3] ==0) {
                pipe->value[2]=0;
                pipe->value[3]=0;
	send_to_char( "You must tamp the pipe first.\r\n", ch );
                return;
    }

    level = pipe->value[0] + pipe->value[2];
    send_to_char("You begin to smoke...\r\n", ch);
    act( "$n smokes $p.", ch, pipe, NULL, TO_ROOM );
    obj_cast_spell( pipe->value[3], UMAX(level,5), ch, ch, pipe, NULL );
    obj_cast_spell( pipe->value[4], UMAX(level,5), ch, ch, pipe, NULL );

     if (pipe->value[1] == FALSE) {
          obj_from_char(pipe);
          extract_obj(pipe);
     } else {
         pipe->value[2] = 0;
         pipe->value[3] = 0;
         pipe->value[4] = 0;
     }
     return;
}
    
void do_auction (CHAR_DATA *ch, char *argument){
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int minpr=0;

    argument = one_argument (argument, arg1);

    if (IS_NPC(ch)) return;

    if (arg1[0] == '\0') {
        if (auction->item != NULL)        {
            /* show item data here */

            if (auction->bet > 0) sprintf (buf, "Current bet on this item is %d {rCopper{x.\r\n",auction->bet);
            else sprintf (buf, "No bets on this item have been received.\r\n");
            send_to_char (buf,ch);
            spell_identify (0, 150, ch, auction->item); 
            return;
        }else {
            send_to_char ("Auction WHAT?\r\n",ch);
            return;
        }
    }
    if (IS_IMMORTAL(ch) && !str_cmp(arg1,"stop")) {
    if (auction->item == NULL)    {
        send_to_char ("There is no auction going on you can stop.\r\n",ch);
        return;
    } else {
        sprintf (buf,"Sale of %s has been stopped by God. Item confiscated.", auction->item->short_descr);
        talk_auction (buf);
        obj_to_char (auction->item, ch);
        auction->item = NULL;
        if (auction->buyer != NULL) {
            auction->buyer->gold[0] += auction->bet;
            send_to_char ("Your money has been returned.\r\n",auction->buyer);
        }
        return;
    }
    }
    if (!str_cmp(arg1,"bet") ) {
        if (auction->item != NULL) {
            int newbet;

            /* make - perhaps - a bet now */
            if (argument[0] == '\0') {
                send_to_char ("Bet how much?\r\n",ch);
                return;
            }
           
            if (ch == auction->seller) {
                send_to_char ("This is your own object!\r\n",ch);
                return;
            }

            if (ch == auction->buyer) {
                send_to_char ("Your bet is already the highest!\r\n",ch);
                return;
            }

            newbet = parsebet (auction->bet, argument);
            
            if (newbet < (auction->bet + 100))  {
                send_to_char ("You must at least bid 100 {rCopper{x over the current bet.\r\n",ch);
                return;
            }

            if (newbet > ch->gold[0])  {
                send_to_char ("You don't have that much money!\r\n",ch);
                return;
            }

            /* the actual bet is OK! */

            /* return the gold to the last buyer, if one exists */
            if (auction->buyer != NULL && auction->buyer != auction->seller) auction->buyer->gold[0] += auction->bet;

            ch->gold[0] -= newbet; /* substract the gold - important :) */
            auction->buyer = ch;
            auction->bet   = newbet;
            auction->going = 0;
            auction->pulse = PULSE_AUCTION; /* start the auction over again */

            sprintf (buf,"A bet of %d {rCopper{x have been received on %s.\r\n",newbet,auction->item->short_descr);
            talk_auction (buf);
            return;


        } else {
            send_to_char ("There isn't anything being auctioned right now.\r\n",ch);
            return;
        }
    }
/* finally... */

    obj = get_obj_list (ch, arg1, ch->carrying);

    if (obj == NULL)  {
        send_to_char ("You aren't carrying that.\r\n",ch);
        return;
    }

    if (auction->item == NULL) {
         if (!IS_SET(ch->in_room->room_flags, ROOM_AUCTION_HALL)) {
              send_to_char ("You have to go to an auction hall.\r\n",ch);
              return;
         }

         if (obj->condition < 35) {
              send_to_char ("We don't take damaged stuff.\r\n",ch);
              return;
         }

         if (obj->timer > 0) {
              send_to_char ("We don't take decaying stuff.\r\n",ch);
              return;
         }

         if (argument[0] != '\0') {
             minpr=atoi(argument);
             minpr=UMAX(minpr,0);
         }     

         if (obj->zmask != 0
         || obj->timer >0) {
                 send_to_char ("This item is not stable enough to auction it.\r\n",ch);
                 return;
         } else {
                  obj_from_char (obj);
                  auction->item = obj;
                  auction->bet = minpr;
                  auction->buyer = ch;
                  auction->seller = ch;
                  auction->pulse = PULSE_AUCTION;
                  auction->going = 0;
                  auction->auction_room = ch->in_room;
                  sprintf (buf, "A new item has been received:{m %s{x.", obj->short_descr);
                  talk_auction (buf);
                  if (auction->bet > 0) {
                     sprintf (buf, "Starting bet is:{y%d{x.", auction->bet);
                     talk_auction (buf);
                  }
                  return;
         }
    } else {
        act ("Try again later - $p is being auctioned right now!",ch,auction->item,NULL,TO_CHAR);
        return;
    }
return;
}


void do_plant( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' ) {
	send_to_char( "Plant what?\r\n", ch );
	return;
    }

    obj = ch->in_room->contents;
    while ( obj != NULL
    && ( obj->item_type != ITEM_TREE
       || !can_see_obj(ch, obj))) {
            obj = obj->next_content;
    }

    if (obj) {
       if (obj->item_type == ITEM_TREE) {
            send_to_char( "There is already a tree planted here.\r\n", ch );
            return;
       }
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_TREE)) {
            send_to_char( "There is already a tree planted here.\r\n", ch );
            return;
    }

    if ((obj = get_obj_carry( ch, arg )) == NULL ) {
          send_to_char( "You do not have that item.\r\n", ch );
          return;
    }

    if (obj->item_type != ITEM_TREE
    || obj->value[0] != TREE_SEED) {
          send_to_char( "This is no seed.\r\n", ch );
          return;
     }

     if (!can_drop_obj( ch, obj )) {
          send_to_char( "You can't let go of it.\r\n", ch );
          return;
     }

     if (ch->in_room->sector_type != SECT_FIELD
     && ch->in_room->sector_type != SECT_FOREST
     && ch->in_room->sector_type != SECT_PLAIN
     && ch->in_room->sector_type != SECT_HILLS
     && ch->in_room->sector_type != SECT_SWAMP
     && ch->in_room->sector_type != SECT_MOORS) {
          send_to_char( "You can't plant the seed here.\r\n", ch );
          return;
     }

     if (IS_RAFFECTED(ch->in_room, RAFF_NOBREATHE)
     || IS_RAFFECTED(ch->in_room, RAFF_DARKNESS)) {
          send_to_char( "You can't plant the seed here.\r\n", ch );
          return;
     }

     mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);
     wev = get_wev(WEV_DROP, WEV_DROP_PLANT, mcc,
                     "You plant @p2.\r\n",
                     "@a2 plants @p2.\r\n",
                     "@a2 plants @p2.\r\n");

     if (!room_issue_wev_challange(ch->in_room, wev)) {
          free_wev(wev);
          return;
     }

     obj_from_char( obj );
     obj_to_room( obj, ch->in_room );

     obj->value[0] = TREE_SPROUT;
     REMOVE_BIT(obj->wear_flags, ITEM_TAKE);
     SET_BIT(obj->extra_flags, ITEM_NO_SAC);
     if (IS_IMMORTAL(ch)) SET_BIT(obj->extra_flags, ITEM_NO_COND);
     if (obj->value[1] != 0) SET_BIT(obj->extra_flags, ITEM_MAGIC);
     SET_BIT(ch->in_room->room_flags, ROOM_TREE);
     obj->value[2] = time_info.year;

     free_string(obj->description);
     obj->description = str_dup("A tiny sprout grows here.\n");
     free_string(obj->short_descr);
     obj->short_descr = str_dup("a tiny sprout");
     free_string(obj->name);
     obj->name = str_dup("sprout");
      SET_BIT(ch->in_room->area->area_flags, AREA_CHANGED);
   
      room_issue_wev( ch->in_room, wev);
      free_wev(wev);
      return;
}


void do_photograph( CHAR_DATA *ch, char *argument ) {
char arg[MAX_INPUT_LENGTH];
OBJ_DATA *camera;
OBJ_DATA *photo;
int event = 0;

     if ((camera = get_eq_char( ch, WEAR_HOLD )) == NULL) {
          send_to_char ("You're not holding a camera.\r\n",ch);
          return;
     }  

     if (camera->item_type != ITEM_CAMERA) {
          send_to_char ("You're not holding a camera.\r\n",ch);
          return;
     }  

     if (!IS_SET(camera->value[1], CONT_CLOSED)) {
          send_to_char ("Close the camera first.\r\n",ch);
          return;
     }  

     photo = camera->contains;

     if (photo == NULL) {
          send_to_char ("There is no photographic plate in the camera.\r\n",ch);
          return;
     }  

     if (photo->item_type != ITEM_PHOTOGRAPH) {
          send_to_char ("There is no photographic plate in the camera.\r\n",ch);
          return;
     }  

     if (photo->value[0] != PHOTO_BLANK) {
          send_to_char ("This plate has already been exposed.\r\n",ch);
          return;
     }  
     
     if (argument[0] !='\0') {
          one_argument(argument, arg);

          if (!str_cmp(arg, "give"))  		event = 2;
          else if (!str_cmp(arg, "get"))  		event = 3;
          else if (!str_cmp(arg, "put"))  		event = 4;
          else if (!str_cmp(arg, "drop"))  	event = 5;
          else if (!str_cmp(arg, "poison"))  	event = 6;
          else if (!str_cmp(arg, "fill"))  		event = 7;
          else if (!str_cmp(arg, "drink")) 	event = 8;
          else if (!str_cmp(arg, "eat"))  		event = 9;
          else if (!str_cmp(arg, "sac"))  		event = 10;
          else if (!str_cmp(arg, "gadget"))  	event = 11;
          else if (!str_cmp(arg, "search"))  	event = 12;
          else if (!str_cmp(arg, "depart"))  	event = 13;
          else if (!str_cmp(arg, "arrive")) 	event = 14;
          else if (!str_cmp(arg, "death"))  	event = 19;
          else if (!str_cmp(arg, "attack"))  	event = 20;
          else if (!str_cmp(arg, "combat"))  	event = 21;
          else if (!str_cmp(arg, "damage"))  	event = 22;
          else if (!str_cmp(arg, "social"))  	event = 25;
          else if (!str_cmp(arg, "idol"))  		event = 26;
          else if (!str_cmp(arg, "lock"))  		event = 27;
          else if (!str_cmp(arg, "activity"))  	event = 30;
          else if (!str_cmp(arg, "knock"))  	event = 34;
          else if (!str_cmp(arg, "learn"))  	event = 37;
          else if (!str_cmp(arg, "trap"))  		event = 39;
     }

     if (event > 0) {
        camera->value[4] = event;
        set_activity(ch, ch->position, "waiting here.", ACV_PHOTO, NULL);
     } else {
         make_photo(ch, camera, photo);
     }
     return;
}


void make_photo(CHAR_DATA *ch, OBJ_DATA *camera, OBJ_DATA *photo) {
EXTRA_DESCR_DATA *ed;
CHAR_DATA *rch;
MOB_CMD_CONTEXT *mcc;
WEV *wev;
char buf[10*MAX_STRING_LENGTH];

         mcc = get_mcc(ch, ch, NULL, NULL, camera, photo, 0, NULL);
         wev = get_wev(WEV_OPROG, WEV_OPROG_PHOTO, mcc,
                     "You take a photograph.\r\n",
                     "",
                     "@a2 takes a photograph. \r\n");

         if (!room_issue_wev_challange(ch->in_room, wev)) {
              free_wev(wev);
              return;
         }

         photo->value[0] = PHOTO_EXPOSING;

         if (ch->in_room == NULL) {
                 free_string(photo->description);
                 photo->description = str_dup("A photograph lies here.\n");
                 free_string(photo->short_descr);
                 photo->short_descr = str_dup("a completely black photograph");
                 free_string(photo->name);
                 photo->name = str_dup("photograph");
         } else {
                 free_string(photo->description);
                 photo->description = str_dup("A photograph lies here.\n");
                 free_string(photo->short_descr);
                 sprintf(buf, "a photograph of %s", ch->in_room->name);
                 photo->short_descr = strdup(buf);
                 free_string(photo->name);
                 photo->name = str_dup("photograph");

                ed			=   new_extra_descr();
	ed->keyword		=   str_dup(photo->name);
	ed->description		=   str_dup( "" );
	ed->next		                =   photo->extra_descr;
	photo->extra_descr	=   ed;
                
                 sprintf(buf, "%s\r\n", ch->in_room->name);
                 strcat(buf, ch->in_room->description);
                 strcat(buf, "\r\n");

                for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )    {
  	     if ( rch == ch )    continue;
                     if (IS_AFFECTED(rch, AFF_MORF)) continue;

	     if ( IS_SET(rch->plr, PLR_WIZINVIS)
                     && !IS_IMMORTAL(ch)
                     && get_trust( ch ) < rch->invis_level)  continue;

                      if (can_see(ch, rch)) strcat(buf, photo_char( rch, ch ));
                 }
                 free_string(ed->description);
                 ed->description = str_dup(buf);
         }

          room_issue_wev( ch->in_room, wev);
          free_wev(wev);
          return;
}


char* photo_char(CHAR_DATA *victim, CHAR_DATA *ch ) {
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char *trans;
    char msg[MAX_STRING_LENGTH];
    int percent, diff;
 
    buf[0] = '\0';

    if (IS_AFFECTED(victim, AFF_MIST)) {
          strcat(buf, "A small cloud if mist is flying here.\r\n");
          trans=strdup(buf);
          return trans;
    }

   /* Generate leading tags for short look... */ 

 if (IS_NPC(victim) && !IS_NPC(ch)) {
    if (ch->pcdata->questmob > 0 
    && victim->pIndexData->vnum == ch->pcdata->questmob) {
         strcat( buf, "{r[{RTARGET{r]{x ");
    }

    if (ch->pcdata->questmob ==-100 
    && !str_cmp(ch->pcdata->questplr, victim->short_descr)) {
         strcat( buf, "{r[{RTARGET{r]{x ");
    }
 }

    if ( IS_AFFECTED(victim, AFF_INVISIBLE))
      strcat( buf, "{x({bInvis{x) ");

    if ( IS_SET(victim->form, FORM_BLEEDING))
      strcat( buf, "{x(Losing {rBlood{x) ");

    if ( !IS_NPC(victim) 
      && IS_SET(victim->plr, PLR_WIZINVIS)) 
      strcat ( buf, "{x({mWizi{x) ");

    if ( !IS_NPC(victim) 
      && IS_SET(victim->plr, PLR_CLOAK)) 
      strcat ( buf, "{x({mCloak{x) ");

    if (str_cmp(race_array[victim->race].name,"machine")
    && !IS_SET(victim->form, FORM_MACHINE)) {
       if ( IS_AFFECTED(victim, AFF_FIRE_SHIELD)) strcat( buf, "{x({RFlames{x) ");
       if ( IS_AFFECTED(victim, AFF_FROST_SHIELD)) strcat( buf, "{x({CFrost{x) ");
       if ( IS_AFFECTED(victim, AFF_LIGHTNING_SHIELD)) strcat( buf, "{x({WElectricity{x) ");
       if ( IS_AFFECTED(victim, AFF_HIDE)) strcat( buf, "{x({bHide{x) ");
       if ( IS_AFFECTED(victim, AFF_CHARM)) strcat( buf, "{x({gCharmed{x) ");
       if ( IS_AFFECTED(victim, AFF_PASS_DOOR)) strcat( buf, "{x({cTranslucent{x) ");
       if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE))  strcat( buf, "{x({rPink Aura{x) ");
       if ( IS_EVIL(victim)) {
             if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) strcat( buf, "{x({rRed Aura{x) " );
             if (IS_AFFECTED(victim, AFF_AURA)) strcat( buf, "{x({mDark Aura{x) " );
       }
       if ( IS_GOOD(victim)) {
            if (is_affected(ch, gafn_detect_good) ) strcat( buf, "{x({gGreen Aura{x) " );
             if (IS_AFFECTED(victim, AFF_AURA)) strcat( buf, "{x({WLight Aura{x) " );
       }
       if ( IS_AFFECTED(victim, AFF_SANCTUARY)) strcat( buf, "{m({wWhite Aura{m){x ");
       if ( IS_AFFECTED(victim, AFF_FLYING))  strcat( buf, "{x({yFlying{x) ");
       if ( IS_AFFECTED(victim, AFF_SNEAK)) strcat( buf, "{x({mSneaking{x) ");
    } else {
          if (victim->leader != NULL
          && victim->ridden != TRUE){
              sprintf (buf2,"{x({r%s{x) ",victim->master->name);
              strcat( buf, buf2);
          }
    }

   /* Calculate health... */

    if (victim->max_hit > 0) percent = victim->hit * 100 / victim->max_hit;
    else percent = -1;
     
    if (percent >= 100) 
      ; //strcat(buf,"{x({gexcellent{x) ");   
    else if (percent >= 85)
      strcat(buf,"{x({yScratches{x) ");
    else if (percent >= 70)
        strcat(buf,"{x({YBruises{x) ");
    else if (percent >= 55)
        strcat(buf,"{x({wCuts{x) ");
    else if (percent >= 40)
        strcat(buf,"{x({WWounds{x) ");
    else if (percent >= 30)
        strcat(buf,"{x({mWounds{x) ");
    else if (percent >= 20)
        strcat(buf,"{x({MBlood Covered{x) ");
    else if (percent >= 10)
        strcat(buf,"{x({rHurt{x) ");
    else if (percent >= 0)
        strcat(buf,"{x({rAwful{x) ");
    else
        strcat(buf,"{x({RBleeding{x) ");

    if (victim->max_move > 0) {
      percent = victim->move * 100 / victim->max_move;
    } else {
       percent = -1;
    }
 
    if (percent >= 90) 
      ; //strcat(buf,"{x({gexcellent{x) ");   
    else if (percent >= 75)
        strcat(buf,"{x({wTired{x) ");
    else if (percent >= 50)
        strcat(buf,"{x({WExhausted{x) ");
    else if (percent >= 30)
        strcat(buf,"{x({mWeak{x) ");
    else if (percent >= 15)
        strcat(buf,"{x({rStaggering{x) ");
    else if (percent >= 0)
        strcat(buf,"{x({BFainting{x) ");
    else
        strcat(buf,"{x({bStunned{x) ");

    if (IS_SET(ch->plr, PLR_AUTOCON)) {
          diff = get_effective_level(victim) - get_effective_level(ch);
          if ( diff <= -10 ) sprintf(msg, "{x({CToo weak{x) ");
          else if ( diff <=  -5 ) sprintf(msg, "{x({cWeak{x) ");
          else if ( diff <=  5 ) sprintf(msg, "{x({WFair opponent{x) ");
          else if ( diff <=  10 ) sprintf(msg, "{x({rStrong{x) ");
          else sprintf(msg, "{x({RToo strong{x) ");
          strcat(buf, msg);
    }

    if ( victim->desc != NULL
      && !victim->desc->ok ) {
      strcat(buf, "{x({clinkdead{x) ");
    }

   /* If it's in its starting position, just send its description... */

    if ( victim->position == victim->start_pos 
    && victim->long_descr[0] != '\0' ) {
          strcat( buf, victim->long_descr );
    } else {

     /* Copy PERSMASK (- persons name under mask) over... */ 
  
      strcat( buf, PERSMASK( victim, ch ) );

     /* Add player title... */ 

      if ( !IS_NPC(victim) 
        && !IS_SET(ch->comm, COMM_BRIEF) 
        && (!IS_AFFECTED(victim, AFF_POLY))) {
  	  strcat( buf, victim->pcdata->title );
      }
 
     /* Position qualifier... */ 

      strcat(buf, " is ");
      pos_text(victim, buf);

     /* Make it look nice... */ 
 
      strcat(buf, "{x\r\n" );

      buf[0] = toupper(buf[0]);
    }

    trans =strdup(buf);
    return trans;
}


#define ACV_DESTROY_OBJECT		1

void do_destroy( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj = NULL;
    OBJ_DATA *tool;
    OBJ_INDEX_DATA *pObj;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;
    char buf[MAX_STRING_LENGTH];
    int pwr;

    one_argument( argument, arg );

    if (IS_AFFECTED(ch, AFF_MIST)) {
	send_to_char( "You can't do that in mist form?\r\n", ch );
	return;
    }

    if ( arg[0] == '\0' ) {
	send_to_char( "Destroy what?\r\n", ch );
	return;
    }

    if (!check_activity_key(ch, argument)) return;

    if ( ch->activity == ACV_DESTROYING
      && argument[0] != '*' ) {
      send_to_char("You are already destroying something!\r\n", ch);
      return;
    }

    if (ch->activity != ACV_DESTROYING) ch->acv_state = 0;
    
    switch ( ch->acv_state ) {
      default:
          obj = get_obj_here( ch, arg );
 
          if ( obj == NULL ) {
              act( "There is no $T here.", ch, NULL, arg, TO_CHAR );
              return;
          } 

          if (IS_SET(obj->wear_flags, ITEM_TAKE)
          || IS_SET(obj->extra_flags, ITEM_NO_COND)
          || obj->item_type == ITEM_TREASURE
          || obj->item_type == ITEM_CONTAINER
          || obj->item_type == ITEM_FOOD
          || obj->item_type == ITEM_MONEY
          || obj->item_type == ITEM_CORPSE_NPC
          || obj->item_type == ITEM_CORPSE_PC
          || obj->item_type == ITEM_LOCKER
          || obj->item_type == ITEM_CLAN_LOCKER
          || obj->item_type == ITEM_BONDAGE
          || obj->item_type == ITEM_TIMEBOMB) {
              send_to_char( "You can't destroy that!\r\n", ch );
              return;
        }

          mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);
          wev = get_wev(WEV_OPROG, WEV_OPROG_DESTROY, mcc,
                      "You start destroying @p2.\r\n",
                       NULL,
                      "@a2 starts destroying @p2.\r\n");

          sprintf(buf, "destroying %s.", obj->short_descr);

        
       /* Challange... */

        if (!room_issue_wev_challange(ch->in_room, wev)) {
          free_wev(wev);
          clear_activity(ch);
          return;
        }

       /* Send to command issuers room... */

        set_activity( ch, ch->position, NULL, ACV_DESTROYING, buf);

       /* Free the wev (will autofree the context)... */

        free_wev(wev);

       /* Set up the activity state engine... */

        set_activity_key(ch); 
 
          ch->acv_state = ACV_DESTROY_OBJECT;
          free_string(ch->acv_text);
          ch->acv_text = str_dup(obj->name);

       /* Schedule the callback... */

        schedule_activity( ch, 10, "Destroy" );
        break;


      case ACV_DESTROY_OBJECT: 

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

        pObj = obj->pIndexData;

         pwr=0;
         if ((tool = get_eq_char(ch, WEAR_HOLD)) != NULL) {
                if (tool->item_type == ITEM_DIGGING )  pwr=tool->level/4;
         }

         if (pwr==0) {
                if ((tool = get_eq_char(ch, WEAR_WIELD)) != NULL) { 
                       if (tool->value[0] == WEAPON_AXE) pwr = tool->level/8;
                       if (tool->value[0] == WEAPON_MACE) pwr = tool->level/6;
                }
         }
         pwr = (pwr + get_curr_stat(ch, STAT_STR) + ch->level/10 - 7 - obj->level + number_percent())/2;
         if (IS_SET(obj->extra_flags, ITEM_MAGIC)) pwr /=2;

         if (pwr > number_percent()) {
             act( "You damage $p.", ch, obj, NULL, TO_CHAR );
             act( "$n damages $p.", ch, obj, NULL, TO_ROOM );
             obj->condition--;
             if (pObj != NULL
             && !str_cmp(obj->description, pObj->description)
             && obj->enchanted == FALSE) {
                  pObj->condition = obj->condition;
                  SET_BIT(ch->in_room->area->area_flags, AREA_CHANGED);
             }


         } else {
             act( "You fail to damage $p.", ch, obj, NULL, TO_CHAR );
             act( "$n fails to damages $p.", ch, obj, NULL, TO_ROOM );
         }

        if (obj->condition < 1) {
          obj->condition = 1;
          if (pObj != NULL
          && !str_cmp(obj->description, pObj->description)
          && obj->enchanted == FALSE) {
               pObj->condition = 1;
          }

          mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);
          wev = get_wev(WEV_OPROG, WEV_OPROG_DESTROY, mcc,
                      "You destroy @p2.\r\n",
                       NULL,
                      "@a2 destroys @p2.\r\n");

         /* Make the change... */
 
          room_issue_wev(ch->in_room, wev);

         /* Free the wev (will autofree the context)... */

          free_wev(wev);
     
          set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
          return; 
        }

        schedule_activity( ch, 10, "Destroy" );
        break;
    }

    return;
}


#define ACV_REBUILD_OBJECT		1

void do_rebuild( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj = NULL;
    OBJ_INDEX_DATA *pObj;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;
    char buf[MAX_STRING_LENGTH];
    int pwr;

    one_argument( argument, arg );

    if (IS_AFFECTED(ch, AFF_MIST)) {
	send_to_char( "You can't do that in mist form?\r\n", ch );
	return;
    }

    if ( arg[0] == '\0' ) {
	send_to_char( "rebuild what?\r\n", ch );
	return;
    }

    if (!check_activity_key(ch, argument)) return;

    if ( ch->activity == ACV_REBUILDING
      && argument[0] != '*' ) {
      send_to_char("You are already rebuilding something!\r\n", ch);
      return;
    }

    if (ch->activity != ACV_REBUILDING) ch->acv_state = 0;
    
    switch ( ch->acv_state ) {
      default:
          obj = get_obj_here( ch, arg );
 
          if ( obj == NULL ) {
            act( "There is no $T here.", ch, NULL, arg, TO_CHAR );
            return;
          } 

          if (IS_SET(obj->wear_flags, ITEM_TAKE)
          || IS_SET(obj->extra_flags, ITEM_NO_COND)
          || obj->item_type == ITEM_TREASURE
          || obj->item_type == ITEM_CONTAINER
          || obj->item_type == ITEM_FOOD
          || obj->item_type == ITEM_MONEY
          || obj->item_type == ITEM_CORPSE_NPC
          || obj->item_type == ITEM_CORPSE_PC
          || obj->item_type == ITEM_LOCKER
          || obj->item_type == ITEM_CLAN_LOCKER
          || obj->item_type == ITEM_BONDAGE
          || obj->item_type == ITEM_TIMEBOMB) {
              send_to_char( "You can't rebuild that!\r\n", ch );
              return;
        }

          mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);
          wev = get_wev(WEV_OPROG, WEV_OPROG_REBUILD, mcc,
                      "You start rebuilding @p2.\r\n",
                       NULL,
                      "@a2 starts rebuilding @p2.\r\n");

          sprintf(buf, "rebuilding %s.", obj->short_descr);

        /* Challange... */

        if (!room_issue_wev_challange(ch->in_room, wev)) {
          free_wev(wev);
          clear_activity(ch);
          return;
        }

       /* Send to command issuers room... */

        set_activity( ch, ch->position, NULL, ACV_REBUILDING, buf);

       /* Free the wev (will autofree the context)... */

        free_wev(wev);

       /* Set up the activity state engine... */

        set_activity_key(ch); 
 
          ch->acv_state = ACV_REBUILD_OBJECT;
          free_string(ch->acv_text);
          ch->acv_text = str_dup(obj->name);

       /* Schedule the callback... */

        schedule_activity( ch, 15, "Rebuild" );
        break;


      case ACV_REBUILD_OBJECT: 

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

        pObj = obj->pIndexData;

         pwr=0;
         pwr = (pwr + get_curr_stat(ch, STAT_DEX) + get_curr_stat(ch, STAT_INT) + ch->level/10 - 60 - obj->level + number_percent())/2;
         if (IS_SET(obj->extra_flags, ITEM_MAGIC)) pwr /=2;

         if (pwr > number_percent()) {
             act( "You repair $p.", ch, obj, NULL, TO_CHAR );
             act( "$n repairs $p.", ch, obj, NULL, TO_ROOM );
             obj->condition++;
             if (pObj != NULL
             && !str_cmp(obj->description, pObj->description)
             && obj->enchanted == FALSE) {
                  pObj->condition++;
                  SET_BIT(ch->in_room->area->area_flags, AREA_CHANGED);
             }
         } else {
             act( "You fail to repair $p.", ch, obj, NULL, TO_CHAR );
             act( "$n fails to repair $p.", ch, obj, NULL, TO_ROOM );
         }

        if (obj->condition >99) {
          obj->condition = 100;
          if (pObj != NULL
          && !str_cmp(obj->description, pObj->description)
          && obj->enchanted == FALSE) {
               pObj->condition = 100;
          }

          mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);
          wev = get_wev(WEV_OPROG, WEV_OPROG_REBUILD, mcc,
                      "You complete your work on @p2.\r\n",
                       NULL,
                      "@a2 completes @a3 work on @p2.\r\n");

         /* Make the change... */
 
          room_issue_wev(ch->in_room, wev);

         /* Free the wev (will autofree the context)... */

          free_wev(wev);
     
          set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
          return; 
        }

        schedule_activity( ch, 15, "Rebuild" );
        break;
    }

    return;
}


void do_store(CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    CHAR_DATA *mob;
    OBJ_DATA *obj;
    SHOP_DATA *pShop;
    int i;
    bool found = FALSE;
   
    smash_tilde(argument);
    one_argument( argument, arg );

    if ( arg[0] == '\0' )    {
	send_to_char( "Store what?\r\n", ch );
	return;
    }

    keeper = find_keeper(ch, TRUE);
    if (!keeper) return;

    pShop = keeper->pIndexData->pShop;
    for ( i = 0; i < MAX_TRADE; i++ ) {
         if (pShop->buy_type[i] == ITEM_FIGURINE ) found= TRUE;
    }

    if (!found) {
        act( "$n tells you 'I won't store your pet for you'.", keeper, NULL, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

    mob = get_char_room(ch, arg);

    if (!mob) {
        act( "$n tells you 'That's not here'.", keeper, NULL, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

    if (mob != ch->pet
    || IS_SET(mob->form, FORM_SENTIENT)) {
        act( "$n tells you 'That's not your pet'.", keeper, NULL, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

    if (!can_see(keeper,mob))    {
          act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
          return;
    }

    act( "$n stores $N.", ch, NULL, mob, TO_ROOM );
    act( "You store $N.", ch, NULL, mob, TO_CHAR );
    die_follower(ch);

    obj = create_object(get_obj_index(OBJ_FIGURINE), mob->level);
    obj->level = mob->level;
    free_string(obj->name);
    obj->name = str_dup(mob->name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(mob->short_descr);
    free_string(obj->description);
    obj->description = str_dup(mob->long_descr);
    obj->item_type = ITEM_FIGURINE;
    obj->value[0] = mob->pIndexData->vnum;
    obj->value[1] = FIGURINE_TAME;
    obj->value[2] = FIGURINE_FALSE;
    obj->value[3] = mob->race;
    obj->cost = 25*obj->level;
    free_string(obj->owner);
    obj->owner = strdup(ch->name);
    char_from_room(mob);
    extract_char(mob, TRUE);
    obj_to_keeper(obj, keeper );
    return;
}


void do_fetch(CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    CHAR_DATA *mob;
    OBJ_DATA *obj;
    SHOP_DATA *pShop;
    int i, cost;
    bool found = FALSE;
    int currency = find_currency(ch);

    smash_tilde(argument);
    one_argument( argument, arg );

    if ( arg[0] == '\0' )    {
	send_to_char( "Fetch what?\r\n", ch );
	return;
    }

    if (ch->pet) {
	send_to_char( "You already got a pet.\r\n", ch );
	return;
    }

    keeper = find_keeper(ch, TRUE);
    if (!keeper) return;

    pShop = keeper->pIndexData->pShop;
    for ( i = 0; i < MAX_TRADE; i++ ) {
         if (pShop->buy_type[i] == ITEM_FIGURINE ) found= TRUE;
    }

    if (!found) {
        act( "$n tells you 'I got no pets in store'.", keeper, NULL, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

    obj = get_obj_carry(keeper, arg);

    if (!obj) {
        act( "$n tells you 'That's not here'.", keeper, NULL, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

    if (obj->item_type != ITEM_FIGURINE
    || obj->value[2] == FIGURINE_TRUE
    || str_cmp(obj->owner, ch->name)) {
        act( "$n tells you 'That's not your pet'.", keeper, NULL, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

    cost = get_cost( keeper, obj, TRUE );
    if (ch->gold[currency] < cost) {
        act( "$n tells you 'I'll keep your pet until you can afford'.", keeper, NULL, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

    ch->gold[currency] -= cost;
    keeper->gold[currency] +=cost;

    act( "$n fetches $p.", ch, obj, NULL, TO_ROOM );
    act( "You fetch $p.", ch, obj, NULL, TO_CHAR );

    mob = create_mobile_level(get_mob_index(obj->value[0]), NULL, obj->level);
    mob->level = obj->level;
    free_string(mob->name);
    mob->name = str_dup(obj->name);
    free_string(mob->short_descr);
    mob->short_descr = str_dup(obj->short_descr);
    free_string(mob->description);
    free_string(mob->long_descr);
    mob->description = str_dup(obj->description);
    mob->long_descr = str_dup(obj->description);
    obj_from_char(obj);
    extract_obj(obj);
    char_to_room(mob, ch->in_room);
    REMOVE_BIT(mob->act, ACT_AGGRESSIVE);
    SET_BIT(mob->affected_by, AFF_CHARM);
    SET_BIT(mob->act, ACT_FOLLOWER);
    SET_BIT(ch->plr, PLR_BOUGHT_PET);
    SET_BIT(mob->act, ACT_PET);
    mob->comm = COMM_NOCHANNELS;

    add_follower(mob, ch );
    mob->leader = ch;
    ch->pet = mob;
    do_emote(mob, "looks at you expectently" );
    return;
}


void do_feed(CHAR_DATA *ch, char *argument) {
CHAR_DATA *victim;
OBJ_DATA *obj;
MOB_CMD_CONTEXT *mcc;
WEV *wev;
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
bool poisoned = FALSE;
int amount, liquid, aff_liq;

    if (argument[0] == '\0') {
         if (IS_NPC(ch)) return;
         if (IS_SET(ch->plr, PLR_FEED))    {
              send_to_char("You'll no longer allow others to feed you.\r\n",ch);
              REMOVE_BIT(ch->plr, PLR_FEED);
         }    else    {
              send_to_char("You allow others to feed you.\r\n",ch);
              SET_BIT(ch->plr, PLR_FEED);
         }

    } else {
         if (ch->position < POS_STANDING) {
                posmessage(ch);
                return;
         }

         argument = one_argument(argument, arg1);
         argument = one_argument(argument, arg2);

         if ((victim = get_char_room(ch, arg1)) == NULL) {
              send_to_char("They're not here.\r\n",ch);
              return;
         }

         if ((obj = get_obj_carry(ch, arg2)) == NULL) {
              send_to_char("You don't have that.\r\n",ch);
              return;
         }

         if (obj->item_type != ITEM_FOOD
         && obj->item_type != ITEM_DRINK_CON) {
              send_to_char("That doesn't make sense.\r\n",ch);
              return;
         }

         if (IS_NPC(victim)
         || !IS_SET(victim->plr, PLR_FEED)) {
              sprintf_to_char(ch, "%s refuses %s.\r\n", victim->short_descr, obj->short_descr);
              return;
         }

         if (obj->item_type == ITEM_FOOD) {
             if (victim->condition[COND_FOOD] >= COND_OVER_STUFFED ) {
                 sprintf_to_char(ch, "%s refuses %s.\r\n", victim->short_descr, obj->short_descr);
                 return;
             }

             mcc = get_mcc(ch, ch, victim, NULL, obj, NULL, 0, NULL);
             poisoned = (obj->value[3] != 0);
             if (!poisoned) {
                  wev = get_wev(WEV_EAT, WEV_EAT_FEED_OK, mcc,
                       "You feed @v2 some @p2.\r\n",
                       "@a2 feeds you something.\r\n",
                       "@a2 feeds @v2 some @p2.\r\n");
             } else {
                  wev = get_wev(WEV_EAT, WEV_EAT_FEED_BAD, mcc,
                       "You feed @v2 some @p2.\r\n",
                       "@a2 feeds you something.\r\n",
                       "@a2 feeds @v2 some @p2.\r\n");
             }

             if (!room_issue_wev_challange(ch->in_room, wev)) {
                  free_wev(wev);
                  return;
             }

             if (obj->value[2] != EFFECT_UNDEFINED) obj_cast_spell( obj->value[2], obj->value[1], victim, victim, obj, NULL );
             if (victim->condition[COND_FOOD] >= COND_STUFFED ) gain_condition(victim, COND_FAT, 1 + (obj->value[0]/10) );
             gain_condition(victim, COND_FOOD, 27 * obj->value[0] );

             room_issue_wev(ch->in_room, wev);
             free_wev(wev);
             extract_obj( obj );

             if (poisoned) {
	 AFFECT_DATA af;

	 act( "$n chokes and gags.", victim, 0, 0, TO_ROOM );
	 send_to_char("That didn't taste so good...\r\n", victim);
              	 af.type      	= gsn_poison;
	 af.afn       	= gafn_poison;
	 af.level		= number_range(3,8);
	 af.duration  	= 2 * number_range(2, 24);
	 af.location  	= APPLY_NONE;
	 af.modifier 	= 0;
	 af.bitvector 	= AFF_POISON;
	 affect_join(victim, &af );
             }

         } else {
             amount = obj->value[1];
             if ( amount <= 0 ) {
  	      send_to_char( "It is already empty.\r\n", ch );
	      return;
             }

             if (victim->condition[COND_DRINK] > 600) {
                  sprintf_to_char(ch, "%s refuses %s.\r\n", victim->short_descr, obj->short_descr);
                  return;
             }

             liquid = obj->value[2];
             poisoned = (obj->value[3] != 0);

             if ( liquid < 0 || liquid >= LIQ_MAX ) {
                  send_to_char("Looks like it's gone off!\r\n", ch);
                  return;
             }

             mcc = get_mcc(ch, ch, victim, NULL, obj, NULL, amount - 1, liq_table[liquid].liq_name);
             aff_liq = liquid;

             if ( liq_table[aff_liq].immune != 0 
             && IS_SET(victim->imm_flags, liq_table[aff_liq].immune) ) {
                    aff_liq = LIQ_WATER;
             }

             if (liq_table[aff_liq].food != 0 
             && IS_SET(victim->imm_flags, liq_table[aff_liq].food) ) {
                    aff_liq = LIQ_JUICE;
             }

             if (IS_AFFECTED(victim, AFF_SWIM)
             && IS_AFFECTED(victim, AFF_WATER_BREATHING)
             && aff_liq == LIQ_SALT_WATER) {
                aff_liq = LIQ_WATER;
             }

             if (IS_SET(victim->act, ACT_VAMPIRE)
             && aff_liq == LIQ_BLOOD) {
                 victim->pcdata->blood = UMIN(victim->pcdata->blood+10, victim->level);
                 if (is_affected(victim, gafn_frenzy)) {
                    if (check_dispel(255, victim, gafn_frenzy)) {
                           send_to_char ("You really needed that drink...\r\n",victim);
                           REMOVE_BIT(victim->parts, PART_FANGS);
                    }
                 }
             }

             if (liq_table[aff_liq].liq_affect[COND_DRINK] > 0) {
                 wev = get_wev(WEV_DRINK, WEV_DRINK_FEED_OK, mcc,
                     "You feed @v2 @t0 from @p2.\r\n",
                     "@a2 feeds you @t0.\r\n",
                     "@a2 feeds @v2 from @p2.\r\n");

             } else {
                 wev = get_wev(WEV_DRINK, WEV_DRINK_FEED_BAD, mcc,
                     "You feed @v2 @t0 from @p2.\r\n",
                     "@a2 feeds you poisonous @t0.\r\n",
                     "@a2 feeds @v2 from @p2.\r\n");
             }

             if (!room_issue_wev_challange(ch->in_room, wev)) {
                 free_wev(wev);
                 return;
             }

             if (obj->value[4] != EFFECT_UNDEFINED) obj_cast_spell( obj->value[4], obj->level/2 +1, victim, victim, obj, NULL );

             if (victim->condition[COND_FOOD] >= COND_STUFFED 
             && liq_table[aff_liq].liq_affect[COND_FOOD] > 0 ) gain_condition(victim, COND_FAT, 1 + (obj->value[0]/10) );

             if (!IS_SET(victim->act, ACT_VAMPIRE)) {
                 gain_condition(victim, COND_FOOD,  27 * liq_table[aff_liq].liq_affect[COND_FOOD]);
                 gain_condition(victim, COND_DRINK, 27 * liq_table[aff_liq].liq_affect[COND_DRINK]);
             }
             gain_condition(victim, COND_DRUNK,  liq_table[aff_liq].liq_affect[COND_DRUNK]);

             obj->value[1] -= 1;
             if (obj->value[1] == 0) obj->value[4] = 0;

             room_issue_wev(ch->in_room, wev);
             free_wev(wev);
 
             if (poisoned) {
                 AFFECT_DATA af;

                 af.afn       = gafn_poison;
                 af.type      = gsn_poison;
                 af.level	   = number_range(4, obj->level+5); 
                 af.duration  = 3 * number_range(5, 20);
                 af.location  = APPLY_NONE;
                 af.modifier  = 0;
                 af.bitvector = AFF_POISON;
                 affect_join(victim, &af );

                 act( "$n chokes and gags.", victim, NULL, NULL, TO_ROOM );
                 send_to_char( "You choke and gag.\r\n", victim);
             }
         }
     }
     return;
}


void light_obj(CHAR_DATA *ch, OBJ_DATA *obj, bool on) {
ROOM_INDEX_DATA *room = NULL;
bool effect = TRUE;

       if (obj->in_room) {
           room = obj->in_room;
       } else if (obj->carried_by) {
           room = obj->carried_by->in_room;
           if (obj->wear_loc == WEAR_NONE) effect = FALSE;  
       }

       if (!room) return;

       if (on) {
           if (obj->value[3] == TRUE) return;
           obj->value[3] = TRUE;
           if (effect) room->light++;
           act( "$n lights $p.", ch, obj, NULL, TO_ROOM );
           act( "You light $p.", ch, obj, NULL, TO_CHAR );
       } else {           
           if (obj->value[3] == FALSE) return;
           obj->value[3] = FALSE;
           if (effect) room->light--;
           act( "$n puts out $p.", ch, obj, NULL, TO_ROOM );
           act( "You put out $p.", ch, obj, NULL, TO_CHAR );
       }
       return;
}


void do_light( CHAR_DATA *ch, char *argument ) {
char arg[MAX_INPUT_LENGTH];
OBJ_DATA *obj;

    argument = one_argument( argument, arg );

    if (arg[0] == '\0' )    {
	send_to_char( "Light what?\r\n", ch );
	return;
    }

    if ((obj = get_obj_here(ch, arg)) == NULL )   {
	send_to_char( "That's not here.\r\n", ch );
	return;
    }

    if (obj->item_type != ITEM_LIGHT) {
	send_to_char( "That's no light.\r\n", ch );
	return;
    }

    if (obj->value[0] == TRUE
    || obj->value[2] == -1
    || obj->value[2] >= 999) {
	send_to_char( "You can't turn that light on.\r\n", ch );
	return;
    }

    if (obj->value[3] == TRUE) {
	send_to_char( "It's already on.\r\n", ch );
	return;
    }

    light_obj(ch, obj, TRUE);
    return;
}


void do_extinguish( CHAR_DATA *ch, char *argument ) {
char arg[MAX_INPUT_LENGTH];
OBJ_DATA *obj;

    argument = one_argument( argument, arg );

    if (arg[0] == '\0' )    {
	send_to_char( "Extinguish what?\r\n", ch );
	return;
    }

    if ((obj = get_obj_here(ch, arg)) == NULL )   {
	send_to_char( "That's not here.\r\n", ch );
	return;
    }

    if (obj->item_type != ITEM_LIGHT) {
	send_to_char( "That's no light.\r\n", ch );
	return;
    }

    if (obj->value[0] == TRUE
    || obj->value[2] == -1
    || obj->value[2] >= 999) {
	send_to_char( "You can't extinguish that light.\r\n", ch );
	return;
    }

    if (obj->value[3] == FALSE) {
	send_to_char( "It's already out.\r\n", ch );
	return;
    }

    light_obj(ch, obj, FALSE);
    return;
}


void do_sprinkle(CHAR_DATA *ch, char *argument ) {
char arg[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
OBJ_DATA *obj;
CHAR_DATA *victim;
MOB_CMD_CONTEXT *mcc;
WEV *wev;
int level;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2);

    if ( arg[0] == '\0' )    {
	send_to_char( "Sprinkle what?\r\n", ch );
	return;
    }

    if ((obj = get_obj_carry(ch, arg)) == NULL )    {
	send_to_char( "You do not have that potion.\r\n", ch );
	return;
    }

    if ( obj->item_type != ITEM_POTION
    && obj->item_type != ITEM_DRINK_CON)    {
	send_to_char( "You can't sprinkle that.\r\n", ch );
	return;
    }

    if ((victim = get_char_room(ch, arg2)) == NULL )    {
	send_to_char( "They're not here.\r\n", ch );
	return;
    }

    if (victim == ch) {
	send_to_char( "That doesn't make sense.\r\n", ch );
	return;
    }

    if (is_safe(ch, victim)) return;

    if (!IS_NPC(ch)) ch->pcdata->pk_timer = 0;    

    if (!IS_NPC(victim)) {
         if (victim->pcdata->pk_timer > 0) {
             sprintf_to_char(ch, "%s is protected at the moment.\r\n", victim->name);
             return;
         }
    }

    if (obj->item_type == ITEM_POTION) {

         mcc = get_mcc( ch, ch, victim, NULL, obj, NULL, 0, NULL);
         wev = get_wev( WEV_ATTACK, WEV_ATTACK_SPRINKLE, mcc,
                  "{RYou sprinkle @p2 on @v2!{x\r\n", 
                  "{R@a2 sprinkles @p2 on you!{x\r\n",
                  "{r@a2 sprinkles @p2 on @v2!{x\r\n");

         if (!room_issue_wev_challange(ch->in_room, wev)) {
               free_wev(wev);
               return;
         }

         room_issue_wev(ch->in_room, wev);
         free_wev(wev);
         level = UMAX(obj->value[0] /5, 1);
         if (obj->value[1] != EFFECT_UNDEFINED) obj_cast_spell( obj->value[1], level, ch, victim, obj, NULL );
         if (obj->value[2] != EFFECT_UNDEFINED) obj_cast_spell( obj->value[2], level, ch, victim, obj, NULL );
         if (obj->value[3] != EFFECT_UNDEFINED) obj_cast_spell( obj->value[3], level, ch, victim, obj, NULL );
         extract_obj(obj);

    } else {
         mcc = get_mcc( ch, ch, victim, NULL, obj, NULL, 0, NULL);
         wev = get_wev( WEV_ATTACK, WEV_ATTACK_SPRINKLE, mcc,
                  "{RYou sprinkle @p2 on @v2!{x\r\n", 
                  "{R@a2 sprinkles @p2 on you!{x\r\n",
                  "{r@a2 sprinkles @p2 on @v2!{x\r\n");

         if (!room_issue_wev_challange(ch->in_room, wev)) {
               free_wev(wev);
               return;
         }

         room_issue_wev(ch->in_room, wev);
         free_wev(wev);

         level = UMAX(obj->level /2, 1);
         if (obj->value[4] != EFFECT_UNDEFINED) obj_cast_spell( obj->value[4], level, ch, victim, obj, NULL );
         obj->value[1] -= 1;
         if (obj->value[1] == 0) obj->value[4] = 0;

         if (obj->value[3] != 0) {
              AFFECT_DATA af;

              af.afn       = gafn_poison;
              af.type      = gsn_poison;
              af.level	   = number_range(4, level+5); 
              af.duration  = 3 * number_range(5, 20);
              af.location  = APPLY_NONE;
              af.modifier  = 0;
              af.bitvector = AFF_POISON;
              affect_join(victim, &af );

              act( "$n chokes and gags.", victim, NULL, NULL, TO_ROOM );
              send_to_char( "You choke and gag.\r\n", victim);
         }
    }

    WAIT_STATE(ch, 12);
    return;
}


void do_conceal( CHAR_DATA *ch, char *argument ) {
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
OBJ_DATA *obj, *container;
MOB_CMD_CONTEXT *mcc;
WEV *wev;
int value, chance;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"in")) argument = one_argument(argument,arg2);

    if ( arg1[0] == '\0' ) {
	send_to_char( "Conceal what?\r\n", ch );
	return;
    }

    if ( arg2[0] == '\0' ) {
	    obj = get_obj_list(ch, arg1, ch->in_room->contents );
	    if ( obj == NULL )    {
		act( "I see no $T here.", ch, NULL, arg1, TO_CHAR );
		return;
	    }

                    if (!IS_SET(obj->wear_flags, ITEM_TAKE)) {
                                sprintf_to_char(ch, "%s is too bulky.\r\n", capitalize(obj->short_descr));
                                return;
                    }

                    if (IS_SET(obj->extra_flags, ITEM_CONCEALED)) {
                                sprintf_to_char(ch, "%s is anready concealed.\r\n", capitalize(obj->short_descr));
                                return;
                    }

                    chance = obj->weight * 3;
                    if (IS_SET(obj->extra_flags, ITEM_GLOW)) chance += 15;
                    if (IS_SET(obj->extra_flags, ITEM_HUM)) chance += 15;
                    if (IS_SET(obj->extra_flags, ITEM_DARK)) chance -= 15;
                    value = get_curr_stat(ch, STAT_WIS);

                    if (number_range(value/3, value) > chance) {
                          mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 1, NULL);
                    } else {
                          mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);
                    }

                    wev = get_wev(WEV_SEARCH, WEV_CONCEAL, mcc,
                                  "You try to conceal @p1.\r\n",
                                  "@a2 tries to conceal @p1.\r\n",
                                  "@a2 tries to conceal @p1.\r\n");

    } else {

	if ((container = get_obj_here( ch, arg2 ) ) == NULL )	{
	    act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	    return;
	}

	 if (container->item_type != ITEM_CONTAINER) {
	    send_to_char( "That's not a container.\r\n", ch );
  	    return;
                 }

	 if ( IS_SET(container->value[1], CONT_CLOSED)
                 && IS_SET(container->value[1], CONT_CLOSEABLE) ) {
	        act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	        return;
	 }

                    if (IS_SET(container->wear_flags, ITEM_TAKE)) {
                                sprintf_to_char(ch, "%s is too small.\r\n", capitalize(container->short_descr));
                                return;
                    }


	 obj = get_obj_list(ch, arg1, container->contains );
	  if (!obj) {
		act( "I see nothing like that in the $T.", ch, NULL, arg2, TO_CHAR );
		return;
	  }

                    if (!IS_SET(obj->wear_flags, ITEM_TAKE)) {
                                sprintf_to_char(ch, "%s is too bulky.\r\n", capitalize(obj->short_descr));
                                return;
                    }

                    if (IS_SET(obj->extra_flags, ITEM_CONCEALED)) {
                                sprintf_to_char(ch, "%s is anready concealed.\r\n", capitalize(obj->short_descr));
                                return;
                    }

                    chance = obj->weight * 3;
                    if (obj->weight > container->value[0] /5) chance +=10;
                    if (obj->weight > container->value[0] /3) chance +=20;

                    if (IS_SET(obj->extra_flags, ITEM_GLOW)) chance += 15;
                    if (IS_SET(obj->extra_flags, ITEM_HUM)) chance += 15;
                    if (IS_SET(obj->extra_flags, ITEM_DARK)) chance -= 15;
                    value = get_curr_stat(ch, STAT_WIS);

                    if (number_range(value/3, value) > chance) {
                          mcc = get_mcc(ch, ch, NULL, NULL, obj, container, 1, NULL);
                    } else {
                          mcc = get_mcc(ch, ch, NULL, NULL, obj, container, 0, NULL);
                    }
                    wev = get_wev(WEV_SEARCH, WEV_CONCEAL, mcc,
                                  "You try to conceal @p1 in @s1.\r\n",
                                  "@a2 tries to conceal @p1 in @s1.\r\n",
                                  "@a2 tries to conceal @p1 in @s1.\r\n");
    }

    WAIT_STATE(ch, 18);

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    if (mcc->number > 0) {
         SET_BIT(obj->extra_flags, ITEM_CONCEALED);
    }

    room_issue_wev( ch->in_room, wev);
    free_wev(wev);
    return;
}


void order_passport(CHAR_DATA *ch, int type) {
PASSPORT *pass;

    if (IS_NPC(ch)) return;

    pass = new_pass_order();
    pass->name 	= str_dup(ch->name);
    pass->type 	= type;
    pass->day 	= time_info.day + (100 - get_curr_stat(ch, STAT_CHA))/3;
    pass->month 	= time_info.month+1;
    pass->year 	= time_info.year;
    if (pass->day > 32) {
       pass->day -=32;
       pass->month +=1;
    }
    
    if (pass->month > 12) {
       pass->month -=12;
       pass->year +=1;
    }
    save_passport();
    return;
}


PASSPORT *get_passport_order(CHAR_DATA *ch) {
PASSPORT *pass;

      if (IS_NPC(ch)) return NULL;

      for (pass = passport_list; pass; pass = pass->next) {
          if (pass->name &&!str_cmp(pass->name, ch->name)) return pass;
      }
      return NULL;
}

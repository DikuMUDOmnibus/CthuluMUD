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
#include "fight.h"
#include "skill.h"
#include "affect.h"
#include "exp.h"
#include "doors.h"
#include "mob.h"
#include "wev.h"
#include "profile.h"
#include "race.h"
#include "olc.h"
#include "cult.h"
#include "gsn.h"

#define MAX_DAMAGE_MESSAGE 37

/* command procedures needed */
DECLARE_DO_FUN(do_circle);
DECLARE_DO_FUN(do_berserk);
DECLARE_DO_FUN(do_bash);
DECLARE_DO_FUN(do_trip);
DECLARE_DO_FUN(do_tail);
DECLARE_DO_FUN(do_crush);
DECLARE_DO_FUN(do_dirt);
DECLARE_DO_FUN(do_flee);
DECLARE_DO_FUN(do_kick);
DECLARE_DO_FUN(do_strangle);
DECLARE_DO_FUN(do_disarm);
DECLARE_DO_FUN(do_get);
DECLARE_DO_FUN(do_recall);
DECLARE_DO_FUN(do_yell);
DECLARE_DO_FUN(do_sacrifice);
DECLARE_DO_FUN(do_rotate);
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_distract);


/*
 * Local functions.
 */
void	check_killer		(CHAR_DATA *ch, CHAR_DATA *victim );
void	check_pk		(CHAR_DATA *ch, CHAR_DATA *victim );
void	death_cry		(CHAR_DATA *ch, bool corpse );
bool 	is_safe			(CHAR_DATA *ch, CHAR_DATA *victim);
void 	make_corpse		(CHAR_DATA *ch );
void	do_one_hit		(COMBAT_DATA *cd );
void	special_one_hit		(CHAR_DATA *ch, CHAR_DATA *victim, int dt );
void        mob_hit			(CHAR_DATA *ch, CHAR_DATA *victim, int dt );
void	raw_kill			(CHAR_DATA *victim, bool corpse );
void	set_fighting		(CHAR_DATA *ch, CHAR_DATA *victim );
void	disarm			(CHAR_DATA *ch, CHAR_DATA *victim , bool dual);
bool	can_murder 		(CHAR_DATA *ch, CHAR_DATA *victim );
void        go_bang         		(CHAR_DATA *ch);  
bool	remove_obj		(CHAR_DATA *ch, int iWear, bool fReplace );
void        blast_room 		(CHAR_DATA *ch, OBJ_DATA *grenade, ROOM_INDEX_DATA *room, int door, OBJ_DATA *portal);
void        blast_door 		(CHAR_DATA *ch, OBJ_DATA *grenade, EXIT_DATA *pexit, int door);
void        set_bleed 		(CHAR_DATA *ch);
void	one_hit			(CHAR_DATA *ch, CHAR_DATA *victim, 	int dt, bool dual, int atk_gsn, int difficulty );
bool 	apply_damage		(COMBAT_DATA *cd);
void        switch_update		(CHAR_DATA *ch);
void        throw_spear 		(CHAR_DATA *ch, OBJ_DATA *spear, ROOM_INDEX_DATA *room, int door);
bool 	check_critical		(COMBAT_DATA *cd);
void 	find_aux_weapon		(COMBAT_DATA *cd, bool hand);
void 	nature_to_app		(OBJ_DATA *obj, int loc, int mod, long long bv);
void 	disband_hunters		(CHAR_DATA *ch);

void process_miss(COMBAT_DATA *cd);
void process_hit(COMBAT_DATA *cd);

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void ) {
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;
    bool disok;

    for ( ch = char_list; ch != NULL; ch = ch->next ) {
    	ch_next	= ch->next;

	victim = ch->fighting;
	if ( victim == NULL 
                || ch->in_room == NULL ) continue;

	if ( IS_AWAKE(ch)) {

                    if (ch->distractedBy) {
                         if ((get_curr_stat(ch, STAT_INT) + get_skill(ch, gsn_tactics))/6 > number_percent()) {
                               send_to_char("You regain control about the situation.\r\n", ch);
                               ch->distractedBy = NULL;
                         }
                    }

                    if (ch->in_room == victim->in_room ) {
    	           multi_hit( ch, victim, TYPE_UNDEFINED );
                    } else {
                           if (IS_SET(ch->in_room->room_flags, ROOM_VEHICLE)
                           || IS_SET(victim->in_room->room_flags, ROOM_VEHICLE)) {              
                           
                                disok = FALSE;
                                if(ch->in_room->exit[DIR_OUT] != NULL) {
                                       if (ch->in_room->exit[DIR_OUT]->u1.to_room == victim->in_room) disok = TRUE;
                                }
                                if(victim->in_room->exit[DIR_OUT] != NULL) {
                                       if (victim->in_room->exit[DIR_OUT]->u1.to_room == ch->in_room) disok = TRUE;
                                }
                
       	                if (disok) multi_hit( ch, victim, TYPE_UNDEFINED );
                                else stop_fighting( ch, FALSE );
                           } else {
                     	stop_fighting( ch, FALSE );
                           }
                    }
	} else {
	    stop_fighting( ch, FALSE );
                }

	victim = ch->fighting;
	if ( victim == NULL ) continue;

    mprog_hitprcnt_trigger( ch, victim );
    mprog_fight_trigger( ch, victim );

    if ((number_percent() >= 85) && (IS_NPC(ch)) && (!IS_AFFECTED(ch, AFF_CHARM))) switch_update(ch);
    

	/*
	 * Fun for the whole family!
	 */
	check_assist(ch,victim);

    }

    return;
}

void switch_update(CHAR_DATA *ch) {
  CHAR_DATA *vch, *vch_next;

  if (ch->fighting != NULL
  && ch->in_room != ch->fighting->in_room) return;
    
  for (vch = ch->in_room->people; vch != NULL; vch = vch_next)   {
      vch_next =vch->next_in_room;
      if (vch->leader != NULL) {
            if (vch != ch->fighting
            && vch->fighting == NULL
            &&  vch->leader->fighting == ch) {
	stop_fighting(ch,FALSE);
	set_fighting(ch,vch);
	return;
            }
      }

      if (vch->master != NULL) {
            if (vch != ch->fighting
            && vch->fighting == NULL
            &&  vch->master->fighting == ch) {
	stop_fighting(ch,FALSE);
	set_fighting(ch,vch);
	return;
            }
      }
    }
  return;
}


/* for auto assisting */
void check_assist(CHAR_DATA *ch, CHAR_DATA *victim) {
    CHAR_DATA *rch, *rch_next;

    if (ch->in_room != victim->in_room) return;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)    {
	rch_next = rch->next_in_room;
	
	if (IS_AWAKE(rch) && rch->fighting == NULL)	{

	    /* quick check for ASSIST_PLAYER */
	    if (!IS_NPC(ch) && IS_NPC(victim) 
	    && IS_SET(rch->off_flags,ASSIST_PLAYERS)
                    && can_see(rch, ch)
	    &&  rch->level + 6 > victim->level)    {
		do_emote(rch,"screams and attacks!");
		if (IS_NPC(ch)) {
                                    multi_hit(rch,ch,TYPE_UNDEFINED);
                                    continue;
                                }
		if (IS_NPC(victim)) {
                                     multi_hit(rch,victim,TYPE_UNDEFINED);
		     continue;
                                }
	    }

                    if (IS_NPC(rch)
  	    && IS_SET(rch->off_flags,ASSIST_GUARD)) {

                            if ((IS_SET(victim->off_flags, ASSIST_GUARD) || IS_SET(victim->act, ACT_MARTIAL))
                            && can_see(rch, ch)) {
		    do_emote(rch,"tries to protect the innocent!");
		    multi_hit(rch,ch,TYPE_UNDEFINED);
		    continue;
                            }

                            if ((IS_SET(ch->off_flags, ASSIST_GUARD) || IS_SET(ch->act, ACT_MARTIAL))
                            && can_see(rch, victim)) {
		    do_emote(rch,"tries to protect the innocent!");
		    multi_hit(rch,victim,TYPE_UNDEFINED);
		    continue;
                            }

                            if ((IS_SET(ch->act, ACT_CRIMINAL) || IS_SET(ch->act, ACT_AGGRESSIVE) || IS_SET(ch->act, ACT_INVADER))
                            && can_see(rch, ch)) {
		    do_emote(rch,"tries to protect the innocent!");
		    multi_hit(rch,ch,TYPE_UNDEFINED);
		    continue;
                            }        

                            if ((IS_SET(victim->act, ACT_CRIMINAL) || IS_SET(victim->act, ACT_AGGRESSIVE) || IS_SET(victim->act, ACT_INVADER))
                            && can_see(rch, victim)) {
		    do_emote(rch,"tries to protect the innocent!");
		    multi_hit(rch,victim,TYPE_UNDEFINED);
		    continue;
                            }        

                            if (can_see(rch, ch)) {
		do_emote(rch,"tries to protect the innocent!");
		multi_hit(rch,ch,TYPE_UNDEFINED);
		continue;
                            }
	    }

	    /* PCs next */
	    if (!IS_NPC(ch) || IS_AFFECTED(ch,AFF_CHARM))   {
                         if (((!IS_NPC(rch) 
                         && IS_SET(rch->plr, PLR_AUTOASSIST))
	         || (IS_AFFECTED(rch,AFF_CHARM) 
                         && str_cmp(race_array[rch->race].name,"machine")
                         && !IS_SET(rch->form, FORM_MACHINE)))
                         && can_see(rch, victim)
	         &&   is_same_group(ch,rch))
		    multi_hit (rch,victim,TYPE_UNDEFINED);
		    continue;
	    }
  	
	    /* now check the NPC cases */
	    
 	    if (IS_NPC(ch) && !IS_AFFECTED(ch,AFF_CHARM)) {
		if ( (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALL))
		||   (IS_NPC(rch) && rch->race == ch->race 
	               && IS_SET(rch->off_flags,ASSIST_RACE))
		||   (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALIGN)
                                &&   ((IS_GOOD(rch)    && IS_GOOD(ch))
		||  (IS_EVIL(rch)    && IS_EVIL(ch))
		||  (IS_NEUTRAL(rch) && IS_NEUTRAL(ch)))) 
		||   (rch->pIndexData == ch->pIndexData 
                               && IS_SET(rch->off_flags,ASSIST_VNUM))) {
		    CHAR_DATA *vch;
		    CHAR_DATA *target;
		    int number;

		    if (number_bits(1) == 0)
			continue;
		
		    target = NULL;
		    number = 0;
		    for (vch = ch->in_room->people; vch; vch = vch->next)   {
			if (can_see(rch,vch)
			&&  is_same_group(vch,victim)
                                                && str_cmp(race_array[vch->race].name,"machine")
                                                && !IS_SET(vch->form, FORM_MACHINE)
			&&  number_range(0,number) == 0)	{
			    target = vch;
			    number++;
			}
		    }

		    if (target != NULL)	    {
			do_emote(rch,"screams and attacks!");
			multi_hit(rch,target,TYPE_UNDEFINED);
		    }
		}	
	    }
	}
    }
}

/* Guns don't have silencers... */

void go_bang(CHAR_DATA *ch) {

    ROOM_INDEX_DATA *was_in_room;
    EXIT_DATA *pexit;

    char msg[] = "{CBANG!{x";

    int door;

   /* Remember where we are... */

    was_in_room = ch->in_room;

   /* For each adjacent room... */ 

    for ( door = 0; door < DIR_MAX; door++ ) {
	if ( ( pexit = was_in_room->exit[door] ) != NULL
                &&   pexit->u1.to_room != NULL
                &&   pexit->u1.to_room != was_in_room ) {
 	    ch->in_room = pexit->u1.to_room;
 	    act( msg, ch, NULL, NULL, TO_ROOM );
	}
    }

   /* Back where I belong... */

    ch->in_room = was_in_room;

   /* Go BANG! here, too */

    act( msg, ch, NULL, NULL, TO_ROOM );
    act( msg, ch, NULL, NULL, TO_CHAR );
    return;
 }

/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) {
    int skill;
    bool dual;
    dual = FALSE;
 
   /* decrement the wait */

    if (ch->desc == NULL) ch->wait = UMAX(0,ch->wait - PULSE_VIOLENCE);

   /* no attacks for stunnies -- just a check */

    if (ch->position < POS_RESTING) return;

   /* Special attack sequence for NPCs... */
 
    if (IS_NPC(ch)) {
	mob_hit(ch, victim, dt);
	return;
    }

   /* First blow is guarenteed... */

    one_hit( ch, victim, dt, FALSE, 0, 0 );

   /* For backstab, that's it... */

    if ( ch->fighting != victim 
      || dt == gsn_assassinate
      || dt == gsn_backstab ) {
	return;
    }

   /* Dual wield gives a second attack... */ 

    if (get_eq_char(ch, WEAR_WIELD2) != NULL) {
        dual = TRUE;
        one_hit( ch, victim, dt, TRUE, 0, 0 );
        if (ch->fighting != victim) return;
    }

   /* Haste gives a third (and dual haste a fourth)... */

    if (IS_AFFECTED(ch, AFF_HASTE) 
    || IS_SET(ch->off_flags, OFF_FAST)) {
        one_hit(ch,victim,dt, FALSE, 0, 0);

        if (ch->fighting != victim) return;
        if (dual) {
            one_hit (ch, victim, dt, TRUE, 0, 0);
            if (ch->fighting != victim) return;
        }	
    } 

   /* Second, third and fourth attack give extra blows with the
      primary weapon... */
    
    if (check_skill(ch, gsn_second_attack, 0)
    && (!IS_AFFECTED(ch, AFF_SLOW)
           ||check_skill(ch, gsn_second_attack, 15))) {
    	one_hit( ch, victim, dt, FALSE, 0, 0 );
	check_improve(ch, gsn_second_attack, TRUE, 5);

        if (ch->fighting != victim) return;
        
        skill = get_skill(ch, gsn_third_attack);

        if ( skill > 0
          && skill + number_open() > 100 ) {

           /* Third strike with dual weapon if wielded... */
       
            if ( dual ) {  
              one_hit( ch, victim, dt, TRUE, 0, 0 );
            } else {
              one_hit( ch, victim, dt, FALSE, 0, 0 );
            }

	    check_improve(ch, gsn_third_attack, TRUE, 6);
 	    if ( ch->fighting != victim ) return;
                    skill = get_skill(ch, gsn_fourth_attack);

            if ( skill > 0
            && skill + number_open() > 100 ) {
                        one_hit( ch, victim, dt, FALSE, 0, 0 );
	        check_improve(ch, gsn_fourth_attack, TRUE, 8);
                        if ( ch->fighting != victim ) return;
            }
        }
    }

   /* All done... */

    return;
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt) {

    int number;
    int skill;
    CHAR_DATA *vch, *vch_next;
    bool dual;

    dual = FALSE;

   /* Start with one attack... */

    one_hit(ch, victim, dt, FALSE, 0, 0);

    if (ch->fighting != victim) {
	return;
    }

   /* For backstab that's all you get... */

    if ( ch->fighting != victim  
      || dt == gsn_assassinate
      || dt == gsn_backstab ) {
	return;
    }

   /* Area attack -- BALLS nasty! */
 
    if (IS_SET(ch->off_flags, OFF_AREA_ATTACK)) {
	for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
	    vch_next = vch->next;
	    if ((vch != victim && vch->fighting == ch))
		one_hit(ch,vch,dt, FALSE, 0, 0);
	}

        if (ch->fighting != victim) return;
    }

   /* Dual wield gives a second attack... */ 

    if (get_eq_char(ch, WEAR_WIELD2) != NULL) {
        dual = TRUE;
    	one_hit( ch, victim, dt, TRUE, 0, 0 );

        if (ch->fighting != victim) {
	    return;
        }
    }

   /* Haste gives another attack, although not an area one... */

    if ( IS_AFFECTED(ch, AFF_HASTE) 
      || IS_SET(ch->off_flags, OFF_FAST) ) {
	one_hit(ch, victim, dt, FALSE, 0, 0);

        if (ch->fighting != victim) {
	    return;
        }

        if (get_eq_char(ch, WEAR_WIELD2) != NULL) {
            dual = TRUE;
    	    one_hit( ch, victim, dt, TRUE, 0, 0 );
        }

        if (ch->fighting != victim) {
            return;
        }
    }

   /* Skills may give second, third and even fourth attacks... */

    if (check_skill(ch, gsn_second_attack, 0)
     && (!IS_AFFECTED(ch, AFF_SLOW)
             || check_skill(ch, gsn_second_attack, 15))) {

	one_hit(ch, victim, dt, FALSE, 0, 0);

	if (ch->fighting != victim) {
	    return;
        }

        skill = get_skill(ch, gsn_third_attack);

        if ( skill > 0
          && skill + number_open() > 100 ) {

            if ( dual ) { 
	      one_hit(ch, victim, dt, TRUE, 0, 0);
            } else { 
	      one_hit(ch, victim, dt, FALSE, 0, 0);
            }

	    if (ch->fighting != victim) {
	        return;
            }
 
            skill = get_skill(ch, gsn_fourth_attack);

            if ( skill > 0
              && skill + number_open() > 100) {

	        one_hit(ch, victim, dt, FALSE, 0, 0);

	        if (ch->fighting != victim) {
	            return;
                } 
            }
        }
    } 

   /* oh boy!  Fun stuff! */

    if (ch->wait > 0) return;

    number = number_range(0,8);

    switch(number) {
    case (0) :
	if (IS_SET(ch->off_flags,OFF_BASH))  do_bash(ch,"");
	break;

    case (1) :
	if (IS_SET(ch->off_flags,OFF_BERSERK) && !IS_AFFECTED(ch,AFF_BERSERK)) do_berserk(ch,"");
	break;


    case (2) :
	if (IS_SET(ch->off_flags,OFF_DISARM) 
	|| (get_weapon_sn(ch, FALSE) != gsn_hand_to_hand 
	&& (IS_SET(ch->act,ACT_WARRIOR)
   	||  IS_SET(ch->act,ACT_THIEF))))
	    do_disarm(ch,"");
	break;

    case (3) :
	if (IS_SET(ch->off_flags,OFF_KICK))  do_kick(ch,"");
	break;

    case (4) :
	if (IS_SET(ch->off_flags,OFF_KICK_DIRT))   do_dirt(ch,"");
	break;

    case (5) :
	if ( IS_SET(ch->off_flags,OFF_TAIL)
                && IS_SET(ch->parts, PART_LONG_TAIL) )
	    do_tail(ch,"");
	break; 

    case (6) :
	if (IS_SET(ch->off_flags,OFF_TRIP)) do_trip(ch,"");
	break;

    case (7) :
	if (IS_SET(ch->off_flags,OFF_CRUSH))  do_crush(ch,"");
	break;

    case (8) :
	if ((IS_SET(ch->off_flags,OFF_DISTRACT) ||  IS_SET(ch->act,ACT_THIEF))
                && has_skill(ch, gsn_tactics)) 
                    do_distract(ch,"");
	break;
    }

    return;
}

/* Work out the damage type... */
	
void set_damage_type(COMBAT_DATA *cd) {
int dt;
int dam_type;
int atk_type;

 /* Get the one given with the attack... */

  dt = cd->atk_dt;

  atk_type = WDT_HIT;
  dam_type = DAM_BASH;

 /* ...if not given, it's a hit by claws or wielded weapon... */

  if ( dt == TYPE_UNDEFINED ) {
    if ( cd->atk_weapon != NULL ) {
      if (cd->atk_weapon->item_type == ITEM_WEAPON ) {

       /* Attack with a weapon... */	

        atk_type = check_weapon_affect_at(cd->atk_weapon);
        if (atk_type == WDT_HIT) atk_type = cd->atk_weapon->value[3];

        if ( atk_type < 0 
        || atk_type > MAX_ATTACK_TYPES ) {
          atk_type = WDT_HIT;
        }	
        dam_type = attack_table[atk_type].damage;

      } else {

       /* Wiedling something other than a weapon! */
 
        atk_type = WDT_HIT;
        dam_type = DAM_BASH;
        dt       = TYPE_HIT;
      }

    } else { 

     /* Unarmed attack... */

      atk_type = cd->attacker->atk_type;

      if ( atk_type < 0 
        || atk_type > MAX_ATTACK_TYPES ) {
        atk_type = WDT_HIT;
      }	

      dam_type = cd->attacker->dam_type;
    }

   /* Set dt and attack description... */

    dt = TYPE_HIT + atk_type;

    cd->atk_desc = attack_table[atk_type].name;
  }

 /* If a dt was given, use it unless they are attacking with a weapon... */ 
 /* This uses the weapons attack attributes, but not its description... */
 
  if (dt < TYPE_HIT) {

    if ( cd->atk_weapon != NULL ) {
      if ( cd->atk_weapon->item_type == ITEM_WEAPON ) {

        atk_type = check_weapon_affect_at(cd->atk_weapon);
        if (atk_type == WDT_HIT) atk_type = cd->atk_weapon->value[3];

        if ( atk_type < 0 
        || atk_type > MAX_ATTACK_TYPES ) {
          atk_type = WDT_HIT;
        }	

        dam_type = attack_table[atk_type].damage;

      } else {

       /* Wiedling something other than a weapon! */
 
        atk_type = WDT_HIT;
        dam_type = DAM_BASH;
        dt       = TYPE_HIT;
      }
    } else {

     /* Unarmed attack... */

      atk_type = cd->attacker->atk_type;

      if ( atk_type < 0 
        || atk_type > MAX_ATTACK_TYPES ) {
        atk_type = WDT_HIT;
      }	

      dam_type = cd->attacker->dam_type;
    }
  }
	
 /* ...default to a HIT if we still don't know... */

  if ( dam_type < 0
    || dam_type >= MAX_DAM ) {
     atk_type = WDT_HIT;
     dam_type = DAM_BASH;
     dt       = TYPE_HIT;
  }

 /* Set the values... */
 
  cd->atk_dt = dt;
  cd->atk_type = atk_type;

  cd->atk_dam_type = dam_type;

  cd->atk_blk_type = can_block_dam[dam_type];

  return;
}


static COMBAT_DATA *cdb;

/* Set up the combat data block... */

COMBAT_DATA *getCDB(CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool dual, int atk_gsn, int diff ) {

 /* Quick sanity/relaity check... */

  if ( ch == victim
    || ch == NULL 
    || victim == NULL) {
    return NULL;
  }

 /* Get the combat data block (first blow only)... */

  if (cdb == NULL) {
    cdb = (COMBAT_DATA *) alloc_perm(sizeof(*cdb));
  }

 /* Set up the combat data block... */

 /* Attacker and defeneder... */

  cdb->attacker = ch;
  cdb->defender = victim;

 /* Attackers weapons... */

  cdb->atk_primary_weapon = get_eq_char(cdb->attacker, WEAR_WIELD); 
  cdb->atk_secondary_weapon = get_eq_char(cdb->attacker, WEAR_WIELD2); 

  if (cdb->attacker->gun_state[0] == GUN_AUX) {
           EXTRA_DESCR_DATA *ed;
           OBJ_INDEX_DATA *pObj;
           OBJ_DATA *aux_wp;
           char arg[MAX_INPUT_LENGTH];
           int new_wp = 0;

           ed = cdb->atk_primary_weapon->pIndexData->extra_descr;
           while (ed) {
               if (is_name("aux_weapon", ed->keyword)) {
                   one_argument(ed->description, arg);
                   new_wp = atol(arg);
               }
               ed = ed->next;
         }

         pObj = get_obj_index(new_wp);
         aux_wp = create_object(pObj, pObj->level);
         cdb->atk_primary_weapon = aux_wp;
  }
  if (cdb->attacker->gun_state[1] == GUN_AUX) {
           EXTRA_DESCR_DATA *ed;
           OBJ_INDEX_DATA *pObj;
           OBJ_DATA *aux_wp;
           char arg[MAX_INPUT_LENGTH];
           int new_wp = 0;

           ed = cdb->atk_secondary_weapon->pIndexData->extra_descr;
           while (ed) {
               if (is_name("aux_weapon", ed->keyword)) {
                   one_argument(ed->description, arg);
                   new_wp = atol(arg);
               }
               ed = ed->next;
         }

         pObj = get_obj_index(new_wp);
         aux_wp = create_object(pObj, pObj->level);
         cdb->atk_secondary_weapon = aux_wp;
  }

 /* Attacking with dual weapon? */

  cdb->atk_dual = dual;

 /* If atk_gsn specified, use that instead of weapon skill... */

  if (atk_gsn > 0) {
    if (validSkill(atk_gsn)) {
      cdb->atk_dual = FALSE;
      cdb->atk_weapon = NULL;
      cdb->atk_sn = atk_gsn;
      cdb->atk_skill = get_skill(cdb->attacker, cdb->atk_sn);
    } else {
      atk_gsn = 0;
    } 
  }

 /* If atk_gsn not specified, use skill for primary/secondary weapon... */

  if (atk_gsn <= 0) {
    if (dual) {
      cdb->atk_weapon = cdb->atk_secondary_weapon;
      cdb->atk_sn = get_sn_for_weapon(cdb->atk_weapon); 
      cdb->atk_skill = get_skill_by(cdb->attacker, cdb->atk_sn, gsn_dual);
    } else {
      cdb->atk_weapon = cdb->atk_primary_weapon; 
      cdb->atk_sn = get_sn_for_weapon(cdb->atk_weapon); 
      cdb->atk_skill = get_skill(cdb->attacker, cdb->atk_sn);
    }
    cdb->atk_desc = skill_array[cdb->atk_sn].name;
  } else {
    cdb->atk_desc = skill_array[atk_gsn].name;
  } 
    
 /* Attack difficulty... */

  cdb->difficulty = diff;

 /* Attack damage type... */

  cdb->atk_dt = dt;

  set_damage_type(cdb);

 /* Attacker berserk? */

  cdb->atk_berserk = (bool) (IS_AFFECTED(cdb->attacker, AFF_BERSERK));

 /* Work out the defenders weapons... */ 

  cdb->def_primary_weapon = get_eq_char(cdb->defender, WEAR_WIELD); 
  cdb->def_secondary_weapon = get_eq_char(cdb->defender, WEAR_WIELD2); 

 /* Defenders shield... */

  cdb->def_shield = get_eq_char(cdb->defender, WEAR_SHIELD);

 /* Defenders hand to hand skill... */

  cdb->def_h2h_skill = get_skill(cdb->defender, gsn_hand_to_hand);

 /* Clear the damage value... */

  cdb->damage = 0;
  cdb->damage_mult = 100; 
 
 /* All done, combat may proceed... */

  return cdb;
} 

/*
 * This routine is called to make one character attack another. 
 * If damage type (dt) is not specified, it is taken from the attackers
 * wielded weapon.
 *
 */  

/* Intercept one_hit so SMGs can fire twice per attack... */

void one_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool dual, int atk_gsn, int diff) {

  COMBAT_DATA *cd;

 /* Setup the cdb... */

  cd = getCDB(ch, victim, dt, dual, atk_gsn, diff);

 /* No CDB, means we can't fight... */

  if (cd == NULL) return;
  
 /* Normal attack... */

  do_one_hit(cd);

 /* If it's not an SMG, we're done... */

  if ( (cd->atk_weapon == NULL)
    || (cd->atk_weapon->item_type != ITEM_WEAPON) 
    || (cd->atk_weapon->value[0] != WEAPON_SMG)) {
    return;
  }

 /* Reset cd to account for potential changes due to equipment damage */
 /* and learning.                                                     */ 

  cd = getCDB( ch, victim, dt, dual, atk_gsn, diff);

 /* No CDB, means we can't fight... */

  if (cd == NULL) return;
 
 /* If the SMG broke, it can't fire again... */

  if (cd->atk_weapon == NULL) return;
  
 /* Antoher attack... */

  do_one_hit(cd);

 /* All done... */

  return;
}

/* Work out a characters attack roll... */

int get_attack_roll(COMBAT_DATA *cd) {

  int roll;

 /* Generate the base roll... */

  roll = cd->attacker->level + number_open() + cd->atk_skill - cd->difficulty;

 /* Adjust for hit roll... */

  roll += GET_HITROLL(cd->attacker);

 /* Return the value... */

  return roll;
}

/* Work out a characters shield block roll... */

int get_shield_block_roll(COMBAT_DATA *cd) {

  int roll;

 /* Find the attackers weapon... */

  if (cd->def_shield == NULL) return -1;
  
 /* Those who are berserk do not shield block... */

  if (cd->atk_berserk) return -1;
 
 /* Generate the base roll... */

  roll = cd->defender->level + number_open() - cd->defender->limb[2]*5;

 /* Adjust for shield_block skill... */

  roll += get_skill(cd->defender, gsn_shield_block);

 /* Return the value... */

  return roll;
}


/* Work out a characters shield block roll... */

int get_parry_roll(COMBAT_DATA *cd, bool secondary) {
int roll;

 /* Can only parry unarmed with some hand to hand skill... */
  
  if (cd->def_h2h_skill <= 0) {
 
    if (secondary) {
      if (cd->def_secondary_weapon == NULL) return -1;
    } else {
      if (cd->def_primary_weapon == NULL) return -1;
    }
  }  

 /* Can't secondary parry if... */

  if (secondary) {

   /* ...you're holding a shield... */

    if (cd->def_shield != NULL) return -1;
   
   /* ...you don't have any dual weapon training... */

    if (get_skill(cd->defender, gsn_dual) <= 0) return -1;
  }

 /* Those who are berserk do not parry... */
  if (cd->atk_berserk) return -1;
  
 /* Generate the base roll... */

  roll = cd->defender->level + number_open() - cd->defender->limb[2]*3 - cd->defender->limb[3]*3;

 /* Adjust for parry skill... */

  if (secondary) {
    roll += get_skill_by(cd->defender, gsn_parry, gsn_dual);
  } else {
    roll += get_skill(cd->defender, gsn_parry);
  } 

  if (cd->defender->distractedBy && cd->defender->distractedBy != cd->attacker) roll /=2;

 /* Return the value... */

  if (IS_NPC(cd->defender)) {
      if (!IS_SET(cd->defender->off_flags, OFF_PARRY)
      && !IS_SET(cd->defender->act, ACT_WARRIOR)) {
           roll /=2;
      }
  }

  return roll;
}


/* Work out a characters dodge roll... */

int get_dodge_roll(COMBAT_DATA *cd) {
int roll;

 /* Those who are berserk do not dodge... */
  if (cd->atk_berserk) return -1;

 /* Generate the base roll... */

  roll = cd->defender->level + number_open()- cd->defender->limb[4]*5;

 /* Adjust for dodge skill... */

  roll += get_skill(cd->defender, gsn_dodge);

  if (cd->defender->distractedBy && cd->defender->distractedBy != cd->attacker) roll /=2;

  if (IS_NPC(cd->defender)) {
      if (!IS_SET(cd->defender->off_flags, OFF_DODGE)
      && !IS_SET(cd->defender->act, ACT_THIEF)) {
           roll /=2;
      }
  }

  return roll;
}


int get_fade_roll(COMBAT_DATA *cd) {
int roll;

    if (!IS_SET(cd->defender->off_flags, OFF_FADE)) return -1;
    
    roll = cd->defender->level + number_open() - cd->defender->limb[0]*5;
    roll += get_curr_stat(cd->defender, STAT_WIS)/2;

    if (cd->defender->distractedBy && cd->defender->distractedBy != cd->attacker) roll /=2;

    return roll;
}


/* See if one character can hit another... */

bool check_hit(COMBAT_DATA *cd) {
int aroll;
int droll;
int mdroll;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

 /* Calculate the attack roll... */

  aroll = get_attack_roll(cd) - cd->attacker->limb[2]*3 - cd->attacker->limb[2]*3;

 /* Check for a complete miss... */

 if (!IS_NPC(cd->attacker)
 && cd->atk_weapon == NULL ) {
      if (cd->attacker->pcdata->style == STYLE_DRUNKEN_BUDDHA) aroll +=5;
      if (cd->attacker->pcdata->style == STYLE_SNAKE) aroll +=10;
      if (cd->attacker->pcdata->style == STYLE_DRAGON) aroll +=15;
 }

 if (cd->attacker->master) {
      int leadership = (get_skill(cd->attacker->master, gsn_leadership) - 50) /3;
      
      if (leadership < 0) leadership /= 2;
      aroll += leadership;
 }

  if (aroll < 0) {

    mcc = get_mcc( cd->attacker, cd->attacker, cd->defender, NULL, NULL, NULL, 0, cd->atk_desc);
    wev = get_wev( WEV_COMBAT, WEV_COMBAT_MISS, mcc,
                  "Your @t0 completely misses @v2!\r\n", 
                  "@a2s @t0 completely misses you!\r\n",
                  "@a2s @t0 completely misses @v2!\r\n");

    room_issue_wev(cd->attacker->in_room, wev);
    free_wev(wev);

    check_improve(cd->attacker, cd->atk_sn, FALSE, 5);

    return FALSE;
  }

 /* Now check to see if the attack is resisted... */

  mdroll = 0;

 /* Check if shield block stops it... */

  if (IS_SET(cd->atk_blk_type, BLOCK_SHIELD)) {

    droll = get_shield_block_roll(cd);
    mdroll = UMAX(droll, mdroll);

    if (droll > aroll) {

      mcc = get_mcc( cd->attacker, cd->attacker, cd->defender,  NULL, NULL, NULL, 0, cd->atk_desc);
      wev = get_wev( WEV_COMBAT, WEV_COMBAT_BLOCK, mcc,
                    "@v2 blocks your @t0 with @v3 shield!\r\n", 
                    "You block @a2s @t0 with your shield!\r\n",
                    "@v2 blocks @a2s @t0 with @v3 shield!\r\n");

      room_issue_wev(cd->attacker->in_room, wev);
      free_wev(wev);

      check_improve(cd->defender, gsn_shield_block, TRUE, 5);

      if ( cd->def_shield != NULL ) {
        check_damage_obj(cd->defender, cd->def_shield, 2);
 
        if ( cd->def_shield != NULL ) {
          if ( cd->def_shield->condition == 0 ) {
            cd->def_shield = NULL;
          }
        }

        if ( cd->def_shield != NULL ) {
          if ( cd->atk_weapon != NULL
            && (cd->atk_weapon->item_type == ITEM_WEAPON) 
            && (cd->atk_weapon->value[0] == WEAPON_AXE)) { 
       
            damage_obj(cd->defender, cd->def_shield, number_bits(3));
          }

          if ( cd->def_shield != NULL ) {
            if ( cd->def_shield->condition == 0 ) {
              cd->def_shield = NULL;
            }
          } 
        }
      }  

      return FALSE;
    }
  }

 /* Check if primary parry stops it... */

  if (IS_SET(cd->atk_blk_type, BLOCK_PARRY)) {

    droll = get_parry_roll(cd, FALSE);
    mdroll = UMAX(mdroll,droll);

 if (cd->defender->master) {
      int leadership = (get_skill(cd->defender->master, gsn_leadership) - 50) /3;
      
      if (leadership < 0) leadership /= 2;
      droll += leadership;
 }

    if (droll > aroll) {
   
      mcc = get_mcc( cd->attacker, cd->attacker, cd->defender, NULL, NULL, NULL, 0, cd->atk_desc);
      wev = get_wev( WEV_COMBAT, WEV_COMBAT_PARRY, mcc,
                    "@v2 parries your @t0!\r\n", 
                    "You parry @a2s @t0!\r\n",
                    "@v2 parries @a2s @t0!\r\n");

      room_issue_wev(cd->attacker->in_room, wev);
      free_wev(wev);

      check_improve(cd->defender, gsn_parry, TRUE, 5);

      if (cd->def_primary_weapon != NULL) {
          check_damage_obj(cd->defender, cd->def_primary_weapon, 2);
          if ( cd->def_primary_weapon->condition == 0 ) cd->def_primary_weapon = NULL;
      }

      return FALSE;
    }

   /* Check if secondary parry stops it... */

    droll = get_parry_roll(cd, TRUE);
    mdroll = UMAX(mdroll,droll);

    if (droll > aroll) {
   
      mcc = get_mcc( cd->attacker, cd->attacker, cd->defender, NULL, NULL, NULL, 0, cd->atk_desc);
      if (cd->def_secondary_weapon != NULL) {
        wev = get_wev( WEV_COMBAT, WEV_COMBAT_PARRY, mcc,
                      "@v2 parries your @t0 with his second weapon!\r\n", 
                      "You parry @a2s @t0 with your second weapon!\r\n",
                      "@v2 parries @a2s @t0 with his second weapon!\r\n");
      } else {
        wev = get_wev( WEV_COMBAT, WEV_COMBAT_PARRY, mcc,
                      "@v2 parries your @t0 with @a3 off hand!\r\n", 
                      "You parry @a2s @t0 with your off hand!\r\n",
                      "@v2 parries @a2s @t0 with @a3 off hand!\r\n");
      }

      room_issue_wev(cd->attacker->in_room, wev);
      free_wev(wev);

      check_improve(cd->defender, gsn_parry, TRUE, 5);

      if (cd->def_secondary_weapon != NULL) {
          check_damage_obj(cd->defender, cd->def_secondary_weapon, 2);
          if ( cd->def_secondary_weapon->condition == 0 ) cd->def_secondary_weapon = NULL;
      }

      return FALSE;
    }
  }

  if (IS_SET(cd->atk_blk_type, BLOCK_DODGE)) {

    droll = get_fade_roll(cd);

    if (droll > aroll) {
  
      mcc = get_mcc( cd->attacker, cd->attacker, cd->defender, NULL, NULL, NULL, 0, cd->atk_desc);
      wev = get_wev( WEV_COMBAT, WEV_COMBAT_FADE, mcc,
                    "@v2 fades away before you can @t0 @v4!\r\n", 
                    "You fade before @a2 can @t0 you!\r\n",
                    "@v2 fades away before @a2 can @t0 @v4!\r\n");

      room_issue_wev(cd->attacker->in_room, wev);
      free_wev(wev);

      return FALSE;
    }
  }

 /* Check if dodge avoids it... */

  if (IS_SET(cd->atk_blk_type, BLOCK_DODGE)) {

    droll = get_dodge_roll(cd);
    mdroll = UMAX(mdroll,droll);

 if (cd->defender->master) {
      int leadership = (get_skill(cd->defender->master, gsn_leadership) - 50) /3;
      
      if (leadership < 0) leadership /= 2;
      droll += leadership;
 }

    if (droll > aroll) {
  
      mcc = get_mcc( cd->attacker, cd->attacker, cd->defender, NULL, NULL, NULL, 0, cd->atk_desc);
      wev = get_wev( WEV_COMBAT, WEV_COMBAT_DODGE, mcc,
                    "@v2 dodges your @t0!\r\n", 
                    "You dodge @a2s @t0!\r\n",
                    "@v2 dodges @a2s @t0!\r\n");

      room_issue_wev(cd->attacker->in_room, wev);
      free_wev(wev);

      check_improve(cd->defender, gsn_dodge, TRUE, 5);

      return FALSE;
    }
    if (!IS_NPC(cd->defender)) {
    if (cd->defender->pcdata->style == STYLE_CRANE
    || cd->defender->pcdata->style == STYLE_DRAGON) {
        droll = get_dodge_roll(cd);
         mdroll = UMAX(mdroll,droll);
         if (droll-25> aroll) {
             mcc = get_mcc( cd->attacker, cd->attacker, cd->defender, NULL, NULL, NULL, 0, cd->atk_desc);
             wev = get_wev( WEV_COMBAT, WEV_COMBAT_DODGE, mcc,
                    "@v2 evades your @t0!\r\n", 
                    "You evade @a2s @t0!\r\n",
                    "@v2 evades @a2s @t0!\r\n");
              room_issue_wev(cd->attacker->in_room, wev);
              free_wev(wev);
              return FALSE;
          }
     }
    if (cd->defender->pcdata->style == STYLE_DRUNKEN_BUDDHA) {
        droll = get_dodge_roll(cd);
         mdroll = UMAX(mdroll,droll);
         if (droll > aroll) {
             mcc = get_mcc( cd->attacker, cd->attacker, cd->defender, NULL, NULL, NULL, 0, cd->atk_desc);
             wev = get_wev( WEV_COMBAT, WEV_COMBAT_DODGE, mcc,
                    "@v2 lurches unpredictably!\r\n", 
                    "You lurch out of  @a2s range!\r\n",
                    "@v2 lurches out of @a2s range!\r\n");
              room_issue_wev(cd->attacker->in_room, wev);
              free_wev(wev);
              return FALSE;
          }
     }
     }
  }

 /* Otherwise it's a hit... */

  check_improve(cd->attacker, cd->atk_sn, TRUE, 6);

  if ( cd->atk_weapon != NULL 
    && cd->atk_sn != gsn_hand_to_hand ) {
    check_damage_obj(cd->attacker, cd->atk_weapon, 2);
  }

  cd->damage_mult = UMAX(100, aroll - mdroll);

  return TRUE;
}

/* See if one character can hit another with a spell... */

bool check_spell_hit(COMBAT_DATA *cd, int sroll) {

  int droll;
  int mdroll;

  MOB_CMD_CONTEXT *mcc;
  WEV *wev;

 /* Check for a complete miss... */

  if (sroll < 0) {

    mcc = get_mcc( cd->attacker, cd->attacker, cd->defender, NULL, NULL, NULL, 0, cd->atk_desc);
    wev = get_wev( WEV_COMBAT, WEV_COMBAT_MISS, mcc,
                  "Your @t0 completely misses @v2!\r\n", 
                  "@a2s @t0 completely misses you!\r\n",
                  "@a2s @t0 completely misses @v2!\r\n");

    room_issue_wev(cd->attacker->in_room, wev);
    free_wev(wev);

    check_improve(cd->attacker, cd->atk_sn, FALSE, 5);

    return FALSE;
  }

 /* Now check to see if the attack is resisted... */

  mdroll = 0;

 /* Absortion first... */

  if ( IS_AFFECTED(cd->defender, AFF_ABSORB) 
    && IS_SET(cd->atk_blk_type, BLOCK_ABSORB)) {
	
    droll = number_open() + (2 * cd->defender->level); 
 
    mdroll = UMAX(droll, mdroll);

    if (droll > sroll) {

      mcc = get_mcc( cd->attacker, cd->attacker, cd->defender, NULL, NULL, NULL, 0, cd->atk_desc);
      wev = get_wev( WEV_COMBAT, WEV_COMBAT_ABSORB, mcc,
                    "@v2 absorbs your @t0!\r\n", 
                    "You absorbs @a2s @t0!\r\n",
                    "@v2 absorbs @a2s @t0!\r\n");

      room_issue_wev(cd->attacker->in_room, wev);
      free_wev(wev);

      return FALSE;
    }
  }

 /* Check if shield block stops it... */

  if (IS_SET(cd->atk_blk_type, BLOCK_SHIELD)) {

    droll = get_shield_block_roll(cd);

    mdroll = UMAX(droll, mdroll);

    if (droll > sroll) {

      mcc = get_mcc( cd->attacker, cd->attacker, cd->defender, NULL, NULL, NULL, 0, cd->atk_desc);
      wev = get_wev( WEV_COMBAT, WEV_COMBAT_BLOCK, mcc,
                    "@v2 blocks your @t0 with @v3 shield!\r\n", 
                    "You block @a2s @t0 with your shield!\r\n",
                    "@v2 blocks @a2s @t0 with @v3 shield!\r\n");

      room_issue_wev(cd->attacker->in_room, wev);
      free_wev(wev);

      check_improve(cd->defender, gsn_shield_block, TRUE, 5);

      if ( cd->def_shield != NULL ) {
        damage_obj(cd->defender, cd->def_shield, number_bits(2));

        if ( cd->def_shield != NULL ) {
          if ( cd->def_shield->condition == 0 ) {
            cd->def_shield = NULL;
          }
        }
      }

      return FALSE;
    }
  }

 /* Check if primary parry stops it... */

  if (IS_SET(cd->atk_blk_type, BLOCK_PARRY)) {

    droll = get_parry_roll(cd, FALSE);

    mdroll = UMAX(mdroll,droll);

    if (droll > sroll) {
   
      mcc = get_mcc( cd->attacker, cd->attacker, cd->defender, NULL, NULL, NULL, 0, cd->atk_desc);
      wev = get_wev( WEV_COMBAT, WEV_COMBAT_PARRY, mcc,
                    "@v2 parries your @t0!\r\n", 
                    "You parry @a2s @t0!\r\n",
                    "@v2 parries @a2s @t0!\r\n");

      room_issue_wev(cd->attacker->in_room, wev);
      free_wev(wev);

      check_improve(cd->defender, gsn_parry, TRUE, 5);

      if (cd->def_primary_weapon != NULL) {
        damage_obj(cd->defender, cd->def_primary_weapon, number_bits(1));
        if ( cd->def_primary_weapon->condition == 0 ) {
          cd->def_primary_weapon = NULL;
        }
      }

      return FALSE;
    }

   /* Check if secondary parry stops it... */

    droll = get_parry_roll(cd, TRUE);

    mdroll = UMAX(mdroll,droll);

    if (droll > sroll) {
   
      mcc = get_mcc( cd->attacker, cd->attacker, cd->defender, NULL, NULL, NULL, 0, cd->atk_desc);

      if (cd->def_secondary_weapon != NULL) {
        wev = get_wev( WEV_COMBAT, WEV_COMBAT_PARRY, mcc,
                      "@v2 parries your @t0 with his second weapon!\r\n", 
                      "You parry @a2s @t0 with your second weapon!\r\n",
                      "@v2 parries @a2s @t0 with his second weapon!\r\n");
      } else {
        wev = get_wev( WEV_COMBAT, WEV_COMBAT_PARRY, mcc,
                      "@v2 parries your @t0 with @a3 off hand!\r\n", 
                      "You parry @a2s @t0 with your off hand!\r\n",
                      "@v2 parries @a2s @t0 with @a3 off hand!\r\n");
      }

      room_issue_wev(cd->attacker->in_room, wev);
      free_wev(wev);

      check_improve(cd->defender, gsn_parry, TRUE, 5);

      if (cd->def_secondary_weapon != NULL) {
        damage_obj(cd->defender, cd->def_secondary_weapon, number_bits(1));
        if ( cd->def_secondary_weapon->condition == 0 ) {
          cd->def_secondary_weapon = NULL;
        }
      }

      return FALSE;
    }
  }

  if (IS_SET(cd->atk_blk_type, BLOCK_DODGE)) {

    droll = get_fade_roll(cd);

    if (droll > sroll) {
  
      mcc = get_mcc( cd->attacker, cd->attacker, cd->defender, NULL, NULL, NULL, 0, cd->atk_desc);
      wev = get_wev( WEV_COMBAT, WEV_COMBAT_FADE, mcc,
                    "@v2 fades away before you can @t0 @v3!\r\n", 
                    "You fade before @a2 can @t0 you!\r\n",
                    "@v2 fades away before @a2 can @t0 @v3!\r\n");

      room_issue_wev(cd->attacker->in_room, wev);
      free_wev(wev);

      return FALSE;
    }
  }

 /* Check if dodge avoids it... */

  if (IS_SET(cd->atk_blk_type, BLOCK_DODGE)) {

    droll = get_dodge_roll(cd);

    mdroll = UMAX(mdroll,droll);

    if (droll > sroll) {
  
      mcc = get_mcc( cd->attacker, cd->attacker, cd->defender, NULL, NULL, NULL, 0, cd->atk_desc);
      wev = get_wev( WEV_COMBAT, WEV_COMBAT_DODGE, mcc,
                    "@v2 dodges your @t0!\r\n", 
                    "You dodge @a2s @t0!\r\n",
                    "@v2 dodges @a2s @t0!\r\n");

      room_issue_wev(cd->attacker->in_room, wev);
      free_wev(wev);

      check_improve(cd->defender, gsn_dodge, TRUE, 5);

      return FALSE;
    }
  }

 /* Otherwise it's a hit... */

  check_improve(cd->attacker, cd->atk_sn, TRUE, 5);

  cd->damage_mult = UMAX(100, sroll - mdroll);

  return TRUE;
}

/* Check the attack wepon for brands and the like.  These give a   */
/* damage bonus.  Converted to use integer arithmetic though.       */

void check_brand2 (COMBAT_DATA *cd) {
    
    int level; 
    int brand;

    int dam_type = -1;

    int dam_total;
    int immune;
    
    AFFECT_DATA af;

   /* No weapon, no bonus... */

    if (cd->atk_weapon == NULL) {
	return;
    } 

   /* No brand, no bonuses... */

    if (!(cd->atk_weapon->value[4] & CHECK_BRAND)) {
      return;
    }  

   /* Times 2 because we're stepping through flags here... */

    for (brand = 1; brand <= MAX_BRAND; brand *= 2) {

    /* No brand means nothing to do... */
		
        if (!IS_SET(cd->atk_weapon->value[4], brand)) {
          continue;
        }

      /* Now, what does it do? */

	switch (brand) { 

           /* Flaming weapons blind your opponent with acrid smoke... */

	    case (WEAPON_FLAMING):
	        	
		dam_type = DAM_FIRE;
	
		if (number_percent() < (cd->atk_weapon->level / 3)) {
			
		    act ("$n's weapon spews forth a blast of acrid {Dsmoke{x!",
                         cd->attacker, NULL, NULL, TO_ROOM);

		    send_to_char("Your weapon spews forth a blast of acrid {rsmoke{x!\r\n", cd->attacker);

 		    if ( !IS_AFFECTED(cd->defender, AFF_BLIND) 
                      && !saves_spell(cd->atk_weapon->level, cd->defender) ) {
		        				
			af.type      = gsn_blindness;
                        af.afn	     = gafn_blindness; 
    			af.level     = cd->atk_weapon->level;
    			af.duration  = number_range(2,4);
    			af.location  = APPLY_HITROLL;
    			af.modifier  = -4;
    			af.bitvector = AFF_BLIND;
    			affect_to_char( cd->defender, &af );
			send_to_char ("You are blinded by the smoke!\r\n", 
                                       cd->defender);
			act ("$n appears to be blinded by smoke!", 
                             cd->defender, NULL, NULL, TO_ROOM);
		    }

                    cd->damage += dice(3,6);
		}

		break;
		
          /* Icy weapons sap the defenders strength... */

	    case (WEAPON_FROST):
		
		dam_type = DAM_COLD;

		if (number_percent() < (cd->atk_weapon->level / 3)) {
					
		    act ("$n's weapon suddenly radiates a bone-chilling, frigid aura!",
                          cd->attacker, NULL, NULL, TO_ROOM);
		    send_to_char ("Your weapon radiates an {Bicy{x aura!\r\n",
                          cd->attacker);

 		    if ( !IS_AFFECTED(cd->defender, gafn_chill_touch ) 
                      && !saves_spell(cd->atk_weapon->level, cd->defender)) {
						
			af.afn       = gafn_chill_touch; 
    			af.level     = cd->atk_weapon->level;
    			af.duration  = number_range (2,4);
    			af.location  = APPLY_STR;
    			af.modifier  = -(cd->atk_weapon->level/12);
    			af.bitvector = 0;
    			affect_to_char ( cd->defender, &af );
			send_to_char ("You are chilled to the core by the extreme cold!\r\n", cd->defender);
			act ("$n starts shivering violently.", 
                                         cd->defender, NULL, NULL, TO_ROOM);
		    }

                    cd->damage += dice(2,6);
		}

		break;

           /* Lightning weapons have a shocking effect... */
		
	    case (WEAPON_LIGHTNING):
	 	
	 	dam_type = DAM_LIGHTNING;

		if ( (number_percent() < (cd->atk_weapon->level / 3))
		  && (!saves_spell(cd->atk_weapon->level, cd->defender))) {
					
		    act ("$n's weapon {Cshocks{x $N!", 
                            cd->attacker, NULL, cd->defender, TO_NOTVICT);

		    send_to_char ("Your weapon {Cshocks{x your enemy with an electrical charge!\r\n", cd->attacker);
		    act ("$N's weapon has {Yshocked{x you!  You are numbed by the blast.", cd->defender, NULL, cd->attacker, TO_CHAR);
		    WAIT_STATE (cd->defender, (2 * PULSE_VIOLENCE));	

                    cd->damage += dice(3,10);
		}
		break;
		
           /* Acidic weapons just do a little more damage... */

	    case (WEAPON_ACID):
			
		dam_type = DAM_ACID;

		if (number_percent() < (cd->atk_weapon->level / 3)) {
                  cd->damage += dice(3,8);
                }

		break;
	    
           /* Vampiric weapons transfer hits and mana... */

	    case (WEAPON_VAMPIRIC): 
				
		dam_type = DAM_NEGATIVE;

		if ( number_percent() < (cd->atk_weapon->level / 3) ) {
					
		    send_to_char ("Your weapon pulses with a sickly, {mpurple{x light!\r\n", cd->attacker);
		    act ("$n's weapon pulses with a sickly {mpurple{x light!", cd->attacker, NULL, NULL, TO_ROOM);
		    if (!saves_spell(cd->atk_weapon->level, cd->defender)) {
						
			dam_total = number_range((cd->atk_weapon->level/2), (cd->atk_weapon->level));

			act ("$n looks drained!", cd->defender, NULL, NULL, TO_ROOM);
			send_to_char ("You feel drained!\r\n", cd->defender);

			cd->attacker->hit += dam_total;
			cd->attacker->mana += (dam_total/2);	

			cd->defender->mana -= (dam_total/2);
			cd->defender->hit -= dam_total;
                                                cd->defender->hit = UMAX(cd->defender->hit, -10);

                        update_pos(cd->defender);

		    }
		}
		break;
		
           /* Sharp weapons have a chance of doing 2x damage... */

	    case (WEAPON_SHARP):
				
		if (number_percent() < (cd->atk_weapon->level)/3) {		
						
		    cd->damage_mult += 100;

		    act ("Your weapon strikes $N with additional power!",
                             cd->attacker, NULL, cd->defender, TO_CHAR);

		    act ("$n's weapon strikes you with added force!", 
                             cd->attacker, NULL, cd->defender, TO_VICT);  
		}

		dam_type = -1;
		break;
 
           /* Vorpal weapons have a change of doing 3x damage */	
	
	    case (WEAPON_VORPAL):
				
		if (number_percent() < (cd->atk_weapon->level)/4) cd->damage_mult += 200; 
		dam_type = -1;
		break;

    	
	    case (WEAPON_POISON):

		level = cd->atk_weapon->level;

		if (IS_AFFECTED(cd->defender, AFF_POISON)) {
    		    act("$n is already poisoned.", cd->attacker, NULL, cd->defender, TO_VICT);
		} else {
    		    if ( saves_spell( level, cd->defender ) ) {
    		           act("$n turns slightly green, but it passes.", cd->defender, NULL, NULL, TO_ROOM);
    		           send_to_char("You feel momentarily ill, but it passes.\r\n", cd->defender);
		           dam_type = -1;
    		    } else {
		           af.type      = gsn_poison;
                                           af.afn       = gafn_poison;
    		           af.level     = 1 + (level/2);
    		           af.duration  = 1 + (level/10);
    		           af.location  = APPLY_STR;
    		           af.modifier  = -1 * dice(2,4);
    		           af.bitvector = AFF_POISON;
    		           affect_join( cd->defender, &af );
    		            send_to_char( "You feel a burning sensation from the weapon's cut.\r\n", cd->defender );
    		            act("$n looks very ill.", cd->defender, NULL, NULL, TO_ROOM);
    		            dam_type = -1;
		    }
		}

               /* Poison wears off of weapons after a while... */

		if (number_percent() < 10 && !IS_SET(cd->atk_weapon->pIndexData->value[4], WEAPON_POISON)) {
		    send_to_char ("The vemom on the blade dries up.\r\n", cd->attacker);
		    REMOVE_BIT(cd->atk_weapon->value[4], WEAPON_POISON);
		}

         	break;

	    case (WEAPON_PLAGUE):

		level = cd->atk_weapon->level;

		if (IS_AFFECTED(cd->defender, AFF_PLAGUE)) {
    		    act("$n is already sick.", cd->attacker, NULL, cd->defender, TO_VICT);
		} else {
    		    if ( saves_spell( level, cd->defender ) ) {
    		           act("$n turns slightly green, but it passes.", cd->defender, NULL, NULL, TO_ROOM);
    		           send_to_char("You feel momentarily sick, but it passes.\r\n", cd->defender);
		           dam_type = -1;
    		    } else {
		           af.type      = gsn_plague;
                                           af.afn       = gafn_plague;
    		           af.level     = 1 + (level/2);
    		           af.duration  = 1 + (level/10);
    		           af.location  = APPLY_STR;
    		           af.modifier  = -1 * dice(3,6);
    		           af.bitvector = AFF_PLAGUE;
    		           affect_join( cd->defender, &af );
    		           send_to_char( "The wound begins to fester.\r\n", cd->defender );
    		           act("$n looks very sick.", cd->defender, NULL, NULL, TO_ROOM);
    		           dam_type = -1;
		    }
		}

         	break;

           /* Default brand does nothing... */

    	    default:
		dam_type = -1;
		break;
	    	
	}

       /* Adjust damage for vulnerability if it has a brand... */

        if (dam_type != -1) { 
				
	    immune = check_immune(cd->defender, dam_type);
		
            switch (immune) {

		case IS_IMMUNE:
		    break;
		
		case IS_RESISTANT:
		case IS_RESISTANT_PLUS:
		    cd->damage_mult += 10;
                                    cd->damage += dice(2,4);
		    break;
		
		case IS_VULNERABLE:
		    cd->damage_mult += 50;
                                    cd->damage += dice(2,10);
		    break;
		
		default:
		    cd->damage_mult += 25;
                                    cd->damage += dice(2,6);
	 	    break;
		
	    };
	} 
    }

    return;
} 
		

void find_aux_weapon(COMBAT_DATA *cd, bool hand) {
OBJ_INDEX_DATA *pObj;
EXTRA_DESCR_DATA *ed;
char arg[MAX_INPUT_LENGTH];
VNUM new_wp = 0;

    if (cd->atk_weapon->pIndexData) {
         ed = cd->atk_weapon->pIndexData->extra_descr;
         while (ed) {
               if (is_name("aux_weapon", ed->keyword)) {
                   one_argument(ed->description, arg);
                   new_wp = atol(arg);
               }
               ed = ed->next;
         }

         if (new_wp != 0) {
               pObj = get_obj_index(new_wp);
               if (pObj->item_type == ITEM_WEAPON) cd->attacker->gun_state[hand] = GUN_AUX;
               send_to_char("You rely on your auxiliary weapon.\r\n", cd->attacker);
         } 
    }
    return;
}


/* Hit one guy once... */

void do_one_hit(COMBAT_DATA *cd) {
int gs = 0;
bool distok;
	
   /* Check that they fighters are in the same room... */

    if ( cd->attacker->in_room != cd->defender->in_room ) {
          if (cd->attacker == NULL) return;
          if (cd->defender == NULL) return;
          if (cd->attacker->in_room == NULL) return;
          if (cd->defender->in_room == NULL) return;

          if (!IS_SET(cd->attacker->in_room->room_flags, ROOM_VEHICLE)
          && !IS_SET(cd->defender->in_room->room_flags, ROOM_VEHICLE)) return;

          distok = FALSE;
          if(cd->attacker->in_room->exit[DIR_OUT] != NULL) {
                 if (cd->attacker->in_room->exit[DIR_OUT]->u1.to_room == cd->defender->in_room) distok = TRUE;
          }
          if(cd->defender->in_room->exit[DIR_OUT] != NULL) {
                 if (cd->defender->in_room->exit[DIR_OUT]->u1.to_room == cd->attacker->in_room) distok = TRUE;
          }
          if (!distok) return;

          if (cd->atk_weapon == NULL
          || cd->atk_weapon->item_type != ITEM_WEAPON
          || (cd->atk_weapon->value[0] != WEAPON_HANDGUN
          && cd->atk_weapon->value[0] != WEAPON_GUN
          && cd->atk_weapon->value[0] != WEAPON_BOW
          && cd->atk_weapon->value[0] != WEAPON_SMG)) return;
    }
   /* Check the victim is still active... */
     
    if ( cd->defender->position < POS_STUNNED ) return;
    
    if ( cd->defender->position == POS_STUNNED
    && cd->defender->move<1 ) return;
    
   /* We are now in combat... */

    if ( cd->defender->position > POS_STUNNED ) {

      if ( cd->defender->fighting == NULL
        && cd->defender->activity != ACV_CASTING ) {
	set_fighting(cd->defender, cd->attacker);
      }

      if ( cd->defender->timer <= 4   
      && cd->defender->wait == 0   
      && cd->defender->activity != ACV_CASTING ) {
        set_activity( cd->defender, POS_FIGHTING, NULL, ACV_NONE, NULL);
      }
    }

    /* Guns need a few extra checks... */

    if ( (cd->atk_weapon != NULL)
      && (cd->atk_weapon->item_type == ITEM_WEAPON) 
      && ( (cd->atk_weapon->value[0] == WEAPON_HANDGUN) 
        || (cd->atk_weapon->value[0] == WEAPON_GUN)
        || (cd->atk_weapon->value[0] == WEAPON_BOW)
        || (cd->atk_weapon->value[0] == WEAPON_SMG))) {
 
     /* Which gun are we firing? */ 

      if (cd->atk_dual) {
        gs = 1;
      } else {
        gs = 0;
      }

     /* NPCs must remember to reload... */

      if (IS_NPC(cd->attacker)) {
        if(cd->attacker->gun_state[gs] < 1) {
          if (number_percent() < get_curr_stat(cd->attacker, STAT_INT)) {

           /* Assume no ammo... */       
 
            cd->attacker->gun_state[gs] = GUN_NO_AMMO;
          
           /* How much do we give... */ 

            if (cd->atk_skill > 0) {

              if (IS_SET(cd->atk_weapon->value[4], WEAPON_ONE_SHOT)) {
                cd->attacker->gun_state[gs] = 1;
              } else {  
                if (IS_SET(cd->atk_weapon->value[4], WEAPON_TWO_SHOT)) {
                  cd->attacker->gun_state[gs] = 2;
                } else {
                  if (IS_SET(cd->atk_weapon->value[4], WEAPON_SIX_SHOT)) {
                    cd->attacker->gun_state[gs] = 6;
                  } else {
                    if (IS_SET(cd->atk_weapon->value[4], WEAPON_TWELVE_SHOT)) {
                      cd->attacker->gun_state[gs] = 12;
                    } else {
                      if (IS_SET(cd->atk_weapon->value[4], WEAPON_36_SHOT)) {
                       cd->attacker->gun_state[gs] = 36;
                      } else {
                        if (IS_SET(cd->atk_weapon->value[4], WEAPON_108_SHOT)) {
                          cd->attacker->gun_state[gs] = 108;
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }

     /* See if it becomes jammed... */

      if (number_percent() <= (11 - ((cd->atk_weapon->condition)/10))
      && cd->atk_weapon->value[0] != WEAPON_BOW) {
        cd->attacker->gun_state[gs] = GUN_JAMMED;
      }  

     /* See if it is jammed... */ 

      if (cd->attacker->gun_state[gs] == GUN_JAMMED) {
        act( "{rYour gun is jammed!{x", cd->attacker, NULL, NULL, TO_CHAR );

        process_miss(cd); 
        find_aux_weapon(cd, gs);
        tail_chain();  
        return;
      }

     /* See if we have any shells left... */ 

      if (cd->attacker->gun_state[gs] == GUN_NO_AMMO) {

        if (cd->atk_weapon->value[0] == WEAPON_BOW) {
            act( "{cNeed more Arrows!{x", cd->attacker, NULL, NULL, TO_CHAR);
        } else {
            act( "{cCLICK!{x", cd->attacker, NULL, NULL, TO_ROOM);
            act( "{cCLICK!{x", cd->attacker, NULL, NULL, TO_CHAR);
        }

        process_miss(cd);
        find_aux_weapon(cd, gs);
        tail_chain();  
        return;
      }

          if (cd->atk_weapon->value[0] == WEAPON_BOW) {
            act( "{gTWACK!{x", cd->attacker, NULL, NULL, TO_ROOM);
            act( "{gTWACK!{x", cd->attacker, NULL, NULL, TO_CHAR);
          } else {
            go_bang(cd->attacker);
          }

          cd->attacker->gun_state[gs] -= 1;

          check_damage_obj(cd->attacker, cd->atk_weapon, 3);
          if ( cd->atk_weapon->condition == 0 ) cd->atk_weapon = NULL;
    } 

   /* See if we hit... */

    if (!check_hit(cd)) {
      process_miss(cd);
      if (cd->attacker->gun_state[gs] == GUN_AUX) extract_obj(cd->atk_weapon);
      tail_chain();
      return;
    }

    process_hit(cd);
    if (cd->attacker->gun_state[gs] == GUN_AUX) extract_obj(cd->atk_weapon);
    return;
}  


void process_miss(COMBAT_DATA *cd) {

   /* Missing with some skills isn't free... */
   /* Missing with dirt is nothing special... */   
 
    if (cd->atk_sn == gsn_dirt) {

	/* check_improve(ch,gsn_dirt,FALSE,2); */

	WAIT_STATE(cd->attacker, skill_array[gsn_dirt].beats);

    }

    if (cd->atk_sn == gsn_strangle) {

	/* check_improve(ch,gsn_strangle,FALSE,2); */

	WAIT_STATE(cd->attacker, skill_array[gsn_strangle].beats);

    }

   /* If you miss with bash you fall over... */

    if (cd->atk_sn == gsn_bash) {
   
	act("You fall flat on your face!", cd->attacker, NULL, cd->defender, TO_CHAR);
	act("$n falls flat on $s face.", cd->attacker, NULL, cd->defender, TO_NOTVICT);
	act("You evade $n's bash, causing $m to fall flat on $s face.", cd->attacker, NULL, cd->defender, TO_VICT);

	set_activity( cd->attacker, POS_RESTING, "lying flat on the floor", ACV_NONE, NULL);

	WAIT_STATE(cd->attacker, skill_array[gsn_bash].beats * 3/2);
    }
 
   /* Missing with a trip isn't so bad... */ 

    if (cd->atk_sn == gsn_trip) {
	WAIT_STATE(cd->attacker, skill_array[gsn_trip].beats*2/3);
    }

   /* Missing with a tail isn't so bad... */ 

    if (cd->atk_sn == gsn_tail) {
	WAIT_STATE(cd->attacker, skill_array[gsn_tail].beats*2/3);
    }

   /* If you miss with crush you may fall over... */

    if ( cd->atk_sn == gsn_crush ) {

      if ( get_char_size(cd->attacker) < get_char_size(cd->defender) + 1 ) {   

	act("You fall flat on your face!", cd->attacker, NULL, cd->defender, TO_CHAR);
	act("$n falls flat on $s face.", cd->attacker, NULL, cd->defender, TO_NOTVICT);
	act("You evade $n's crush, causing $m to fall flat on $s face.", cd->attacker, NULL, cd->defender, TO_VICT);

	set_activity( cd->attacker, POS_RESTING, "lying flat on the floor",  ACV_NONE, NULL);
      }

      WAIT_STATE(cd->attacker, skill_array[gsn_crush].beats * 3/2);

    }
 
   /* All done... */

    return;
}

bool is_gun(int sn) {

   /* Is this an attack with a gun? */

    if ( sn == gsn_gun ) {
      return TRUE; 
    }
 
    if ( sn == gsn_handgun ) {
      return TRUE; 
    }
 
    if ( sn == gsn_smg ) {
      return TRUE; 
    }

    if ( sn == gsn_bow ) {
      return TRUE; 
    }
 
    return FALSE;
}

int get_master_sn(int sn) {

    if ( sn == gsn_axe ) {
      return gsn_axe_master;
    }
 
    if ( sn == gsn_dagger ) {
      return gsn_dagger_master;
    }
 
    if ( sn == gsn_flail ) {
      return gsn_flail_master;
    }
 
    if ( sn == gsn_mace ) {
      return gsn_mace_master;
    }
 
    if ( sn == gsn_polearm ) {
      return gsn_polearm_master;
    }
 
    if ( sn == gsn_spear ) {
      return gsn_spear_master;
    }
 
    if ( sn == gsn_sword ) {
      return gsn_sword_master;
    }
 
    if ( sn == gsn_staff ) {
      return gsn_staff_master;
    }
 
    if ( sn == gsn_whip ) {
      return gsn_whip_master;
    }
 
    if ( sn == gsn_gun
      || sn == gsn_handgun ) {
      return gsn_marksman;
    }
 
    if ( sn == gsn_bow) {
      return gsn_master_archer;
    }

    if ( sn == gsn_backstab) {
      return gsn_backstab;
    }

    return 0;
}  

void process_hit(COMBAT_DATA *cd) {
    EXTRA_DESCR_DATA *ed;
    char arg[MAX_STRING_LENGTH];
    int protection = 0;
    int dam;
    int diceroll;
    int dbns;
    int skill;
    int master_sn;
    bool martial;
    bool gun;

    gun = is_gun(cd->atk_sn);

   /* Ok, determine base damage... */

    if ( cd->atk_weapon != NULL ) {
	
     /* Roll damage by weapon... */
      dam = dice(cd->atk_weapon->value[1], cd->atk_weapon->value[2]);
      if (!gun) dam = (dam * 3 * cd->atk_weapon->condition) / 200;

    } else {

     /* Determine hand to hand damage... */
    
      if (IS_NPC(cd->attacker)) {
	dam = dice(cd->attacker->damage[DICE_NUMBER],
                   cd->attacker->damage[DICE_TYPE]);
      } else {
        dam = number_range(2, ( 6 * (4 + cd->attacker->level) )/17 );
        if (cd->attacker->pcdata->style == STYLE_TIGER) dam = (dam * 12)/10;
        if (cd->attacker->pcdata->style == STYLE_DRAGON) dam = (dam * 14)/10;
      }

    }

   /* Scale damage by attack skills... */

    if ( !gun
      &&  isSkilled(cd->attacker)) {
      dam = (dam * cd->atk_skill) /100;
    }

   /* All add the damage bonus, scaled by skill... */
 
    if (gun) {
      dbns =  cd->attacker->damroll + str_app[get_curr_stat(cd->attacker, STAT_LUC)].todam;
    } else {
      dbns =  cd->attacker->damroll + str_app[get_curr_stat(cd->attacker, STAT_STR)].todam;
    }
    dam += (dbns * cd->atk_skill)/100; 
    if (cd->atk_weapon) dam += UMAX(get_weapon_affect_level(cd->atk_weapon)/10, 0);

   /* Exhaustion modifies in-fight damage */

    if ( !gun ) {
       if (cd->attacker->move < cd->attacker->max_move /2) {
         dam = (dam * 8)/10;
       }
       if (cd->attacker->move < cd->attacker->max_move /4) {
         dam = (dam * 8)/10;
       }
       if (cd->attacker->move < cd->attacker->max_move /10) {
         dam = (dam * 5)/10;
       }           
    }

           
   /* We should have at least one point at this stage... */

    if (dam < 1 ) dam = 1;
    
   /* Apply all of the bonuses... */

   /* Martial Arts gives a damage increase... */

    if (cd->atk_weapon == NULL) {
      skill = get_skill(cd->attacker, gsn_martial_arts);
      if (skill > 0) {
        martial = FALSE;
 
        if ( cd->atk_sn == gsn_hand_to_hand) { 
          cd->damage_mult += 100;
          martial = TRUE;
        }

        if ( cd->atk_sn == gsn_kick ) {
          cd->damage_mult += 200;
          martial = TRUE;
        }

        if ( cd->atk_sn == gsn_strangle ) {
          cd->damage_mult += 100;
          martial = TRUE;
        }

        if ( cd->atk_sn == gsn_bash ) {
          cd->damage_mult += 30;
          martial = TRUE;
        }

        if ( cd->atk_dt == gsn_backstab ) {
          cd->damage_mult += 80;
          martial = TRUE;
        }

        if ( cd->atk_sn == gsn_trip ) {
          cd->damage_mult += 30;
          martial = TRUE;
        }

        if ( cd->atk_sn == gsn_crush ) {
          cd->damage_mult += 50;
          martial = TRUE;
        }

        if ( cd->atk_sn == gsn_tail ) {
          cd->damage_mult += 30;
          martial = TRUE;
        }

        if (martial) {
          diceroll = number_open() + skill;
       
          if (diceroll > 100) {
            cd->damage_mult += (diceroll - 100);
            check_improve(cd->attacker, gsn_martial_arts, TRUE, 7);

            skill = get_skill(cd->attacker, gsn_black_belt);

            if ( skill > 0) {
              diceroll = number_open() + skill;

              if (diceroll > 100) {
                cd->damage_mult += (diceroll - 100);
                check_improve(cd->attacker, gsn_black_belt, TRUE, 9);

               /* Special effects? */

              }
            }
          }
        } 
      }
    }

   /* Weapon masters also get a damage boost... */

    master_sn = get_master_sn(cd->atk_sn);

    if ( master_sn > 0 ) {

      skill = get_skill(cd->attacker, master_sn);

      if ( skill > 0 ) {
        diceroll = number_open() + skill;
       
        if (diceroll > 100) {
          cd->damage_mult += (diceroll - 100);
          check_improve(cd->attacker, master_sn, TRUE, 7);
     
         /* Special effects? */

          if ( master_sn == gsn_axe_master ) {

            if ( cd->def_shield != NULL ) {
              damage_obj(cd->defender, cd->def_shield, number_bits(5));

              if ( cd->def_shield != NULL ) {
                if ( cd->def_shield->condition == 0 ) {
                  cd->def_shield = NULL;
                }
              }
            }

          }
        }
      } 
    }

   /* Enchanced damage makes it higher... */

    skill = get_skill(cd->attacker, gsn_enhanced_damage);

    if ( skill > 0 ) {
      diceroll = number_open() + skill;

      if (diceroll > 100) {
        check_improve(cd->attacker, gsn_enhanced_damage, TRUE, 6);
        cd->damage_mult += (diceroll - 100);

       /* So does ultra damage... */

        skill = get_skill(cd->attacker, gsn_ultra_damage);

        if ( !gun
          &&  skill > 0 ) {
          diceroll = number_open() + skill;

          if (diceroll > 100) {
            check_improve(cd->attacker, gsn_ultra_damage, TRUE, 7);
            cd->damage_mult += (diceroll - 100);

           /* So does lethal damage... */

            skill = get_skill(cd->attacker, gsn_lethal_damage);

            if ( skill > 0 ) {
              diceroll = number_open() + skill;

              if (diceroll > 100) {
                check_improve(cd->attacker, gsn_lethal_damage, TRUE, 9);
                cd->damage_mult += (diceroll - 100);

               /* Special effects? */

              }
            } 
          }
        } 
      }
    }

   /* More damage if the victim is asleep or not fighting... */

    if ( !IS_AWAKE(cd->defender) ) {
	cd->damage_mult *= 2;
    } else {
      if (cd->defender->position < POS_FIGHTING) {
	cd->damage_mult *= 3;
	cd->damage_mult /= 2;
      }
    }

    if (cd->attacker->master) {
      int leadership = (get_skill(cd->attacker->master, gsn_leadership) - 75) / 10;
      
      cd->damage_mult = cd->damage_mult * (10+leadership) /10;
    }

   /* Backstabbing boosts damage... */

    if ( cd->atk_dt == gsn_backstab 
      && cd->atk_weapon != NULL
      && !gun ) {
    	if ( cd->atk_weapon->value[0] != WEAPON_DAGGER ) {
	    cd->damage_mult *= (2 + (cd->attacker->level / 8)); 
	} else { 
	    cd->damage_mult *= (2 + (cd->attacker->level / 6));
        }
    }

    if ( cd->atk_dt == gsn_assassinate 
      && cd->atk_weapon != NULL ) {
    	if ( cd->atk_weapon->value[0] == WEAPON_HANDGUN
    	|| cd->atk_weapon->value[0] == WEAPON_GUN
    	|| cd->atk_weapon->value[0] == WEAPON_BOW
    	||cd->atk_weapon->value[0] == WEAPON_SMG ) {
	    cd->damage_mult *= (4 + (cd->attacker->level /4 )); 
	} else { 
	    cd->damage_mult *= (4 + (cd->attacker->level / 6));
        }
    }

   /* Circle also boosts damage... */

    if ( cd->atk_dt == gsn_circle 
      && cd->atk_weapon != NULL
      && !gun ) {
      if ( cd->atk_weapon->value[0] != WEAPON_DAGGER ) {
        cd->damage_mult *= (2 + (cd->attacker->level / 50));
      } else {
        cd->damage_mult *= (2 + (cd->attacker->level / 33));
      }
    }

   /* Berzerk characters also get a damage boost... */

    if ( !gun
      &&  cd->atk_berserk) {
      cd->damage_mult += 100 + 2 * cd->attacker->level;
    }

    if (IS_SET(cd->defender->in_room->room_flags, ROOM_VEHICLE)
    && cd->attacker->in_room != cd->defender->in_room) { 
        ed = cd->defender->in_room->extra_descr;
        while (ed != NULL) {
            if (is_name("armor", ed->keyword)) {
                 one_argument(ed->description, arg);
                 protection = atoi(arg);
            }
            ed = ed->next;
        }
        if (protection < 1) protection = 0;
     }

     dam = dam * 10 / (10 + protection);
     dam -= protection * 3;
     if (dam < 0 ) dam = 0;

   /* Put the damage into the cd... */
 
    cd->damage = dam;

   /* Weapon branding and material vulnerability... */

    if ( cd->atk_dt < TYPE_HIT + WDT_MAX 
      && cd->atk_dt > TYPE_HIT
      && dam > 0 ) {
		
	if (cd->atk_weapon != NULL) {

	    check_brand2(cd);
			
	    if (check_material_vuln(cd->atk_weapon, cd->defender)) {
	        cd->damage_mult *= 3;  
	        cd->damage_mult /= 2;  
	    }	
	}
    }

   /* Apply the multiplier... */ 
    cd->damage = UMAX((cd->damage * cd->damage_mult)/100 , 1); 
 
   /* Apply the damage... */
    apply_damage(cd);

   /* All done... */
    tail_chain( );
    return;
}

/* Put out a message about some damage... */

void dam_message( COMBAT_DATA *cd, int dam, bool immune ) {

    char buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;

    MOB_CMD_CONTEXT *mcc;
    WEV *wev;
    int wev_subtype;

    buf1[0] = '\0';
    buf2[0] = '\0';
    buf3[0] = '\0';

    wev_subtype = WEV_COMBAT_HIT;

  if ( dam ==   0 ) { 
      vs = "tickle";	
      vp = "tickles";
    } else if ( dam <=   4 ) { 
      vs = "bruise";	
      vp = "bruises";
    } else if ( dam <=   8 ) { 
      vs = "{wscratch{x";	
      vp = "{wscratches{x";	
    } else if ( dam <=   12 ) { 
      vs = "{wgraze{x";	
      vp = "{wgrazes{x";
    } else if ( dam <=  16 ) { 
      vs = "{whit{x";
      vp = "{whits{x";
    } else if ( dam <=  20 ) { 
      vs = "{binjure{x";	
      vp = "{binjures{x";
    } else if ( dam <=  24 ) { 
      vs = "{bwound{x";	
      vp = "{bwounds{x";
    } else if ( dam <=  28 ) { 
      vs = "{bmaul{x";       
      vp = "{bmauls{x";
    } else if ( dam <=  32 ) { 
      vs = "{gdecimate{x";	
      vp = "{gdecimates{x";	
    } else if ( dam <=  38 ) { 
      vs = "{gdevastate{x";	
      vp = "{gdevastates{x";	
    } else if ( dam <=  45 ) { 
      vs = "{gmaim{x";	
      vp = "{gmaims{x";
    } else if ( dam <=  60 ) { 
      vs = "{cMUTILATE{x";	
      vp = "{cMUTILATES{x";	
    } else if ( dam <=  75 ) { 
      vs = "{cDISEMBOWEL{x";	
      vp = "{cDISEMBOWELS{x";	
    } else if ( dam <=  90 ) { 
      vs = "{cDISMEMBER{x";	
      vp = "{cDISMEMBERS{x";	
    } else if ( dam <=  110 ) { 
      vs = "{cMASSACRE{x";	
      vp = "{cMASSACRES{x";	
    } else if ( dam <=  140 ) { 
      vs = "{cMANGLE{x";
      vp = "{cMANGLES{x";
    } else if ( dam <=  170 ) { 
      vs = "{G*** DEMOLISH ***{x";
      vp = "{G*** DEMOLISHES ***{x";
    } else if ( dam <=  200 ) { 
      vs = "{g*** {GDEVASTATE {g***{x";
      vp = "{g*** {GDEVASTATES {g***{x";
    } else if ( dam <= 240)  { 
      vs = "{c=== {COBLITERATE {C==={x";
      vp = "{c=== {COBLITERATES {C==={x";
    } else if ( dam <= 280)  { 
      vs = "{r){g-{r){g-{r) {RANNIHILATE {r({g-{r({g-{r({x";
      vp = "{r){g-{r){g-{r) {RANNIHILATES {r({g-{r({g-{r({x";
    } else if ( dam <= 320)  { 
      vs = "{r({B---{r) {BERADICATE {r({B---{r){x";
      vp = "{r({B---{r) {BERADICATES {r({B---{r){x";
    } else if ( dam <= 400)  { 
      vs = "{R(*(*(* {gSHRED {R*)*)*){x";
      vp = "{R(*(*(* {gSHREDS {R*)*)*){x";
    } else if ( dam <= 500)  { 
      vs = "{R(((=== {yE{rX{yP{rL{yO{rD{yE {R===))){x";
      vp = "{R(((=== {yE{rX{yP{rL{yO{rD{yE{rS {R===))){x";
    } else if ( dam <= 600)  { 
      vs = "{G(={B*{G=){y(*) {WA{bT{WO{bM{WI{bZ{WE {y(*) {G(={B*{G=){x";
      vp = "{G(={B*{G=){y(*) {WA{bT{WO{bM{WI{bZ{WE{bS {y(*) {G(={B*{G=){x";
    } else { 
      vs = "do {gUN{YSPEAK{gABLE{x things to";
      vp = "{gdoes UN{YSPEAK{gABLE things to{x";
    }

    punct   = (dam <= 24) ? '.' : '!';

    if ( cd->atk_dt == TYPE_HIT )    {
	if (cd->attacker  == cd->defender) {
	    sprintf( buf1, "@a2 %s @a4melf%c [%d]\r\n", vp, punct, dam);
	    sprintf( buf2, "You %s yourself%c [%d]\r\n", vs, punct, dam);
	} else {
	    sprintf( buf1, "@a2 %s @v2%c [%d]\r\n",  vp, punct, dam );
	    sprintf( buf2, "{yYou %s {y@v2%c{x [{y%d{x]\r\n", vs, punct, dam );
	    sprintf( buf3, "{r@a2 %s {ryou%c{x [{r%d{x]\r\n", vp, punct, dam );
	}
    } else {
        attack = cd->atk_desc;

    if ( !IS_SET(cd->attacker->plr, PLR_AUTOKILL) 
    && !IS_NPC(cd->attacker)
    && cd->atk_weapon == NULL) {   
           if (cd->atk_dam_type == DAM_HARM) {
               attack="pillow";
           } else if (cd->atk_dam_type == DAM_BASH) {
                  if (cd->atk_dt == gsn_strangle) {
                            attack="strangling";
                  } else if (cd->atk_dt == gsn_blackjack) {
                            attack="blackjack";
                  } else {
                            attack="brawling";
                  }
            }

     } else {
            if (((IS_SET(cd->attacker->off_flags, ASSIST_GUARD) && !IS_SET(cd->defender->act, ACT_CRIMINAL) && !IS_SET(cd->defender->act, ACT_AGGRESSIVE) && !IS_SET(cd->defender->act, ACT_INVADER))
              || (IS_SET(cd->attacker->off_flags, OFF_STUN))) 
            && IS_NPC(cd->attacker)            
            && cd->atk_weapon == NULL) {
                            attack="brawling";
            }
     }             

     if (IS_SET(cd->attacker->act,ACT_WERE)
      && IS_AFFECTED(cd->attacker, AFF_POLY)) {
            attack="Claw";
      }

	if (immune) {

            wev_subtype = WEV_COMBAT_IMMUNE;
	
	    if (cd->attacker == cd->defender)	    {
		sprintf(buf1,"@a2 is unaffected by @a3 own %s.\r\n", attack);
		sprintf(buf2,"Luckily, you are immune to that.\r\n");
	    }     else    {
	    	sprintf(buf1,"@v2 is unaffected by @a2's %s!\r\n", attack);
	    	sprintf(buf2,"@v2 is {gunaffected{x by your %s!\r\n", attack);
	    	sprintf(buf3,"@v2's %s is {cpowerless{x against you.\r\n", attack);
	    }
	} else {
	
            wev_subtype = WEV_COMBAT_HIT;
	
	    if (cd->attacker == cd->defender)	    {
		sprintf( buf1, "@a2's %s %s @a4%c [%d]\r\n",  attack, vp, punct, dam);
		sprintf( buf2, "Your %s %s you%c [%d]\r\n", attack, vp, punct, dam);
	    }    else    {
	    	sprintf( buf1, "@a2's %s %s @v2%c [%d]\r\n",   attack, vp, punct, dam);
	    	sprintf( buf2, "{yYour %s %s {y@v2%c{x [{y%d{x]\r\n",  attack, vp, punct, dam);
	    	sprintf( buf3, "{r@a2's %s %s {ryou%c{x [{r%d{x]\r\n", attack, vp, punct, dam );
	    }
	}
    }

   /* Combat event... */

    mcc = get_mcc( cd->attacker, cd->attacker, cd->defender,  NULL, NULL, NULL, 0, NULL);
    wev = get_wev( WEV_COMBAT, wev_subtype, mcc, buf2, buf3, buf1);
    room_issue_wev(cd->defender->in_room, wev);
    free_wev(wev);
    return;
}

/* See if an slaying justifies the permadeath flag... */

void check_perma( CHAR_DATA *ch, CHAR_DATA *victim ) {

    if ( ch == NULL || victim == NULL || ch == victim )  return;

   /* Ignore if PermaPK option not set... */

    if (!IS_NPC(ch)
    && !IS_NPC(victim)
    && !IS_SET(victim->act, ACT_CRIMINAL)) {
             if (!IS_SET(ch->in_room->room_flags, ROOM_PKILL)) SET_BIT(ch->act, ACT_CRIMINAL);
    }

    if (!IS_SET(mud.flags, MUD_PERMAPK)) return;
    
   /* Player killing players get permadeath... */

    if ( !IS_NPC(ch)
      && !IS_NPC(victim) ) {

      if ( !IS_IMMORTAL(ch) ) {
 
        if ( !IS_SET(ch->plr, PLR_PERMADEATH) ) {
          SET_BIT(ch->plr, PLR_PERMADEATH);
          send_to_char("{RYou feel very mortal!{x\r\n", ch);
          save_char_obj(ch);
        }
      }
    } 
 
    return;
} 

/* Inflict damage from a hit... */
 
bool apply_damage( COMBAT_DATA *cd) {
    OBJ_DATA *corpse;
    bool immune;
    int dam;
    int armor;
    AFFECT_DATA af;
    char buf[MAX_STRING_LENGTH];
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;
    
   /* Check the defender is still alive... */

    if ( cd->defender->position == POS_DEAD ) return FALSE;
    if ( cd->defender->position == POS_STUNNED
    && cd->defender->move<1) return FALSE;
   
   /* Maximum damage is 12000 points... */
   
    dam = cd->damage;  

    if ( dam > MAX_DAMAGE ) {
	bug( "Damage: %d: more than Max Damage!", dam );
	dam = MAX_DAMAGE;
    }

   /* Now, what does this attack change... */

    if ( cd->defender != cd->attacker ) {
    
       /* Certain attacks are forbidden... */

	if ( is_safe(cd->attacker, cd->defender) ) return FALSE;
        
	check_killer(cd->attacker, cd->defender);

	if ( cd->defender->position > POS_STUNNED ) {
	
	    if ( cd->defender->fighting == NULL
                    && cd->defender->activity != ACV_CASTING ) 	set_fighting(cd->defender, cd->attacker);
	    
	    if ( cd->defender->timer <= 4 
                    && cd->defender->wait == 0
                    && cd->defender->activity != ACV_CASTING ) set_activity(cd->defender, POS_FIGHTING, NULL, ACV_NONE, NULL);
            
	}

       /* If the defender is now fighting... */

	if ( cd->defender->position > POS_STUNNED ) {
	
	    if ( cd->attacker->fighting == NULL 
	    && cd->attacker->activity != ACV_CASTING  ) set_fighting( cd->attacker, cd->defender );
	    
	   /* If victim is charmed, ch might attack victim's master... */
	    
	    if ( IS_NPC(cd->attacker)
	      && IS_NPC(cd->defender)
	      && IS_AFFECTED(cd->defender, AFF_CHARM)
	      && cd->defender->master != NULL
	      && cd->defender->master->in_room == cd->attacker->in_room
	      && number_bits( 3 ) == 0 ) {
		stop_fighting( cd->attacker, FALSE );
		multi_hit( cd->attacker, cd->defender->master, TYPE_UNDEFINED );
		return FALSE;
	      }
 	}

       /* If a follower attacks its master, it stops following... */
	 
	if ( cd->defender->master == cd->attacker ) stop_follower(cd->defender);
       
    }

   /* Attacking destroys invisability... */
     
    if ( IS_AFFECTED(cd->attacker, AFF_INVISIBLE) ) {
	affect_strip_afn(cd->attacker, gafn_invisability );
	REMOVE_BIT( cd->attacker->affected_by, AFF_INVISIBLE );
	act( "$n fades into existence.", cd->attacker, NULL, NULL, TO_ROOM );
    }

    if ( IS_AFFECTED(cd->attacker, AFF_HIDE) ) {
	REMOVE_BIT( cd->attacker->affected_by, AFF_HIDE );
	act( "$n appears.", cd->attacker, NULL, NULL, TO_ROOM );
    }

    if ( IS_AFFECTED(cd->attacker, AFF_SNEAK) ) {
	affect_strip_afn(cd->attacker, gafn_sneak );
	REMOVE_BIT( cd->attacker->affected_by, AFF_SNEAK );
	act( "$n appears.", cd->attacker, NULL, NULL, TO_ROOM );
    }


   /* Being hit destroys equipment... */

    check_damage_obj(cd->defender, NULL, 10);

   /* Special skills have different outcomes... */

   /* Dirt kicking will blind your opponent... */

    if (cd->atk_sn == gsn_dirt) {

        mcc = get_mcc( cd->defender, cd->defender, cd->attacker, NULL, NULL, NULL, 0, NULL);
        wev = get_wev( WEV_DAMAGE, WEV_DAMAGE_DIRT, mcc,
                      "{Y@v2 kicks dirt into your eyes!{x\r\n",
                      "{YYou kick dirt into @a2s eyes!{x\r\n", 
                      "{y@a2 kicks dirt into @v2s eyes!{x\r\n");

        room_issue_wev(cd->defender->in_room, wev);
        free_wev(wev);

	WAIT_STATE(cd->attacker, skill_array[gsn_dirt].beats);

	af.type 	= gsn_dirt;
        	af.afn  	= gafn_dirt_kicking;
	af.level 	= cd->attacker->level;
	af.duration	= 0;
	af.location	= APPLY_HITROLL;
	af.modifier	= -4;
	af.bitvector 	= AFF_BLIND;
	affect_to_char(cd->defender, &af);
	dam /= 10; 
    }

   /* Bash will knock your opponent over... */

    if (cd->atk_sn == gsn_bash) {

        mcc = get_mcc( cd->defender, cd->defender, cd->attacker,  NULL, NULL, NULL, 0, NULL);
        wev = get_wev( WEV_DAMAGE, WEV_DAMAGE_BASH, mcc,
                      "{Y@v2 sends you flying with a powerful bash!{x\r\n",
                      "{YYou bash into @a2, sending them flying!{x\r\n", 
                      "{y@a2 sends @v2 flying with a powerful bash!{x\r\n");

        room_issue_wev(cd->defender->in_room, wev);
        free_wev(wev);

	WAIT_STATE(cd->defender, 3 * PULSE_VIOLENCE);
	WAIT_STATE(cd->attacker, skill_array[gsn_bash].beats);
 	set_activity( cd->defender, POS_RESTING, "lying on the floor", ACV_NONE, NULL);

	dam /= 6;
    }

   /* Tail will knock your opponent over... */

    if (cd->atk_sn == gsn_tail) {

        mcc = get_mcc( cd->defender, cd->defender, cd->attacker,  NULL, NULL, NULL, 0, NULL);

       /* May inject poison or knock you over... */

        if ( IS_SET(cd->attacker->parts, PART_STINGER) ) {

          wev = get_wev( WEV_DAMAGE, WEV_DAMAGE_TAIL, mcc,
                        "{Y@v2 stings you with their tail!{x\r\n",
                        "{YYou sting @a2 with your tail!{x\r\n", 
                        "{y@a2 stings @v2 with thier tail!{x\r\n");
  
        } else {

          wev = get_wev( WEV_DAMAGE, WEV_DAMAGE_TAIL, mcc,
                        "{Y@v2 sends you flying with their tail!{x\r\n",
                        "{YYou send @a2 flying with your tail!{x\r\n", 
                        "{y@a2 sends @v2 flying with thier tail!{x\r\n");
  
        }

        room_issue_wev(cd->defender->in_room, wev);
        free_wev(wev);

        if ( IS_SET(cd->attacker->parts, PART_STINGER) ) {
 
          if ( !saves_spell(cd->attacker->level, cd->defender) ) {

            af.type	= 0;
            af.afn	= gafn_poison;
            af.level	= cd->attacker->level;
            af.duration	= 1 + (cd->attacker->level/10);
            af.location	= APPLY_STR;
            af.modifier	= -1 * dice(2,4);
            af.bitvector	= AFF_POISON;
            affect_join( cd->defender, &af );

            send_to_char( "You feel very sick!\r\n", cd->defender );
 
          } else {
            send_to_char( "You are splattered with venom!\r\n", cd->defender );
          }

          dam /= 8;

        } else {

	  set_activity( cd->defender, POS_RESTING, "lying on the floor", ACV_NONE, NULL);

  	  WAIT_STATE(cd->defender, 4 * PULSE_VIOLENCE);

  	  dam /= 4;
        }

	WAIT_STATE(cd->attacker, skill_array[gsn_tail].beats);

    }

   /* Trip will also knock your opponent over... */ 

    if (cd->atk_sn == gsn_trip) {

        mcc = get_mcc( cd->defender, cd->defender, cd->attacker,  NULL, NULL, NULL, 0, NULL);
        wev = get_wev( WEV_DAMAGE, WEV_DAMAGE_TRIP, mcc,
                      "{Y@v2 trips you up!{x\r\n", 
                      "{YYou trips @a2 up!{x\r\n",
                      "{y@a2 trips @v2 up!{x\r\n");

        room_issue_wev(cd->defender->in_room, wev);
        free_wev(wev);

	WAIT_STATE(cd->defender, 2 * PULSE_VIOLENCE);
	WAIT_STATE(cd->attacker, skill_array[gsn_trip].beats);

	set_activity( cd->defender, POS_RESTING, "lying on the floor", ACV_NONE, NULL);

        dam /= 6;
    }

   /* Crush will pound your opponent into the ground... */

    if (cd->atk_sn == gsn_crush) {

        mcc = get_mcc( cd->defender, cd->defender, cd->attacker, NULL, NULL, NULL, 0, NULL);
        wev = get_wev( WEV_DAMAGE, WEV_DAMAGE_CRUSH, mcc,
                      "{Y@v2 pounds you into the ground!{x\r\n",
                      "{YYou pound @a2 into the ground!{x\r\n", 
                      "{y@a2 pounds @v2 into the ground!{x\r\n");

        room_issue_wev(cd->defender->in_room, wev);
        free_wev(wev);

	WAIT_STATE(cd->defender, 3 * PULSE_VIOLENCE);

	WAIT_STATE(cd->attacker, skill_array[gsn_crush].beats);
 
	set_activity( cd->defender, POS_RESTING, "lying on the floor",  ACV_NONE, NULL);

       /* Attackers size increases damage... */
 
        switch (get_char_size(cd->attacker)) {

          case SIZE_TINY:
            break;

          case SIZE_SMALL:
            dam *= 2;
            break;
 
          default:
          case SIZE_MEDIUM:
            dam *= 3;
            break;

          case SIZE_LARGE:
            dam *= 6; 
            break;

          case SIZE_HUGE: 
            dam *= 8;
            break;

          case SIZE_GIANT: 
            dam *= 12;
            break;

        }

       /* Defenders size reduces damage... */

        switch (get_char_size(cd->defender)) {

          case SIZE_TINY:
            break;

          case SIZE_SMALL:
            dam /= 2;
            break;
 
          default:
          case SIZE_MEDIUM:
            dam /= 3;
            break;

          case SIZE_LARGE:
            dam /= 6; 
            break;

          case SIZE_HUGE: 
            dam /= 8;
            break;

          case SIZE_GIANT: 
            dam /= 12;
            break;

        }

       /* Base damage is 50%... */

        dam /= 2;

    }

   /* Damage Reducers... */

   /* Reduce high value damage... */

    if ( dam > 400) {
	dam = (dam - 400)/2 + 400;
                if ( dam > 800) {
	    dam = (dam - 800)/2 + 800; 
                    if ( dam > 1200) {
                         dam = (dam - 1200)/2 + 1200; 
                         if ( dam > 1600) dam = (dam - 1600)/2 + 1600; 
                    }
                }
    }
  
   /* Sanctuary halves all damage... */

    if ( IS_AFFECTED(cd->defender, AFF_SANCTUARY) ) dam /= 2;

    if ( IS_AFFECTED(cd->defender, AFF_ELDER_SHIELD)) {
         AFFECT_DATA *af;

         if (cd->atk_dam_type == DAM_PIERCE
         || cd->atk_dam_type == DAM_BASH
         || cd->atk_dam_type == DAM_SLASH) {
              dam /= 4;
              for (af = cd->defender->affected; af ; af = af->next) {
                   if (af->bitvector == AFF_ELDER_SHIELD) {
                        if (number_percent() < 5) {
                            af->duration = UMAX(af->duration--, 0);
                            break;
                        }
                   }
              }
         }
    }

   /* Protection from good/evil give 25% reductions... */

    if (cd->attacker != cd->defender) {
      if ( IS_AFFECTED(cd->defender, AFF_PROTECTE) 
        && IS_EVIL(cd->attacker) ) {
        dam -= dam / 4;
      } else if ( IS_AFFECTED(cd->defender, AFF_PROTECTG) 
               && IS_GOOD(cd->attacker) ) {
  	dam -= dam / 4;
      }
    }
  	
   /* Fire shield gives a 50% reduction... */

    if ( cd->atk_dam_type == DAM_COLD
    && IS_AFFECTED(cd->defender, gafn_fire_shield)) {
     	dam -= dam/2;
    }

   /* Armour giives 50% damage reduction for every 100 pts... */

    switch (cd->atk_dam_type) {
	case(DAM_PIERCE): 
                      armor = GET_AC(cd->defender, AC_PIERCE);	
                      break;

	case(DAM_BASH):	 
                      armor = GET_AC(cd->defender, AC_BASH);
                       break;

	case(DAM_SLASH):
                       armor = GET_AC(cd->defender, AC_SLASH);
                       break;

	default:
                       armor = GET_AC(cd->defender, AC_EXOTIC);
                        break;
    }; 

     /* Convert to linear scale... */ 

    armor = 100 - armor;

     /* Maximum is 600 pts - 1/16th damage */

    armor = UMIN(armor, ARMOR_DAM_MAX);

    if (cd->defender->distractedBy && cd->defender->distractedBy != cd->attacker) armor /=2;

    if (IS_SET(cd->atk_blk_type, BLOCK_ARMOR)) {

        dam += 1;
        while (armor > ARMOR_DAM_PTS) {
            dam /= 2;
            armor -= ARMOR_DAM_PTS;
        } 

        dam *= (ARMOR_DAM_PTS2 - armor);
        dam /=  ARMOR_DAM_PTS2;
    } else {
        armor /= 3;
        dam += 1;
        while (armor > ARMOR_DAM_PTS) {
            dam /= 2;
            armor -= ARMOR_DAM_PTS;
        } 

        dam *= (ARMOR_DAM_PTS2 - armor);
        dam /=  ARMOR_DAM_PTS2;
    }


   /* Check for damage immunity... */ 

    immune = FALSE;

    switch (check_immune(cd->defender, cd->atk_dam_type)) {
    
	case(IS_IMMUNE):
	    immune = TRUE;
	    dam = 0;
	    break;

	case(IS_RESISTANT):	
	case(IS_RESISTANT_PLUS):	
	    dam -= dam/3;
	    break;

	case(IS_VULNERABLE):
	    dam += dam/2;
	    break;

        default:
            break;
    }

   /* Output the damage message... */

    dam_message( cd, dam, immune );
    if (dam == 0) return FALSE;
    
    /* Hurt the victim... */

    if (( !IS_SET(cd->attacker->plr, PLR_AUTOKILL) && !IS_NPC(cd->attacker))
    || (IS_SET(cd->attacker->off_flags,ASSIST_GUARD) && !IS_SET(cd->defender->act, ACT_CRIMINAL) && !IS_SET(cd->defender->act, ACT_AGGRESSIVE) && !IS_SET(cd->defender->act, ACT_INVADER) && IS_NPC(cd->attacker))
    || (IS_SET(cd->attacker->off_flags, OFF_STUN) && IS_NPC(cd->attacker))) {
           if (cd->atk_weapon == NULL) {
               if (cd->atk_dam_type == DAM_BASH
               || cd->atk_dam_type == DAM_HARM
               || IS_NPC(cd->attacker)) {
                      cd->defender->move -= (dam * 75)/100;
               } else {
                     cd->defender->hit -= dam;
                     if (dam*.75 > cd->defender->max_hit / 10
                     || cd->defender->hit < cd->defender->max_hit/10)  set_bleed(cd->defender);
               } 
           } else {
                if (IS_SET(cd->atk_weapon->value[4], WEAPON_STUN)) {
                     cd->defender->move -= dam;
                } else {
                     cd->defender->hit -= (dam * 75)/100;
                     if (dam*.75 > cd->defender->max_hit / 10
                     || cd->defender->hit < cd->defender->max_hit/10)  set_bleed(cd->defender);
                }
           }
     } else {
           cd->defender->hit -= dam;
           if (dam > cd->defender->max_hit / 10
           || cd->defender->hit < cd->defender->max_hit/10)  set_bleed(cd->defender);
     }

   if (IS_AFFECTED(cd->defender, AFF_FIRE_SHIELD)) {
            env_damage(cd->attacker, dice(3,5), DAM_FIRE,
            "{R@v2 is {yburned{R by the flames! [@n0]{x\r\n",
            "{RThe flames {yburn{R you! [@n0]{x\r\n");
   }

   if (IS_AFFECTED(cd->defender, AFF_FROST_SHIELD)) {
            env_damage(cd->attacker, dice(3,5), DAM_COLD,
            "{C@v2 is {yhurt{C by the frost! [@n0]{x\r\n",
            "{CThe frost {yhurts{C you! [@n0]{x\r\n");
   }

   if (IS_AFFECTED(cd->defender, AFF_LIGHTNING_SHIELD)) {
            env_damage(cd->attacker, dice(3,5), DAM_LIGHTNING,
            "{W@v2 is {yshocked{W by electricity! [@n0]{x\r\n",
            "{WThe electricity {yshocks{W you! [@n0]{x\r\n");
   }

    if (!IS_NPC(cd->attacker)) {
         if (cd->attacker->pcdata->style == STYLE_TOUCH) {
              if (check_critical(cd)) {
                  if (IS_SET(cd->attacker->plr, PLR_AUTOKILL)) {
                       cd->defender->hit = -50;
                       sprintf_to_char(cd->attacker, "You hit %s with a lethal blow.\r\n", cd->defender->name);
      	       act( "$n collapses, gasping for air and finally dies.",  cd->defender, NULL, cd->attacker, TO_ROOM );
                  } else {
                       cd->defender->move = UMIN(cd->defender->move, -cd->defender->max_move/4);
                       sprintf_to_char(cd->attacker, "You hit %s with a stunning blow.\r\n", cd->defender->name);
      	       act( "$n collapses stunned.",  cd->defender, NULL, cd->attacker, TO_ROOM );
                  }
              }
         }
    }


    if ( !IS_NPC(cd->defender)
    &&  IS_IMMORTAL(cd->defender)
    && !IS_AFFECTED(cd->defender, AFF_INCARNATED)
    && cd->defender->hit < 1 ) { 
	cd->defender->hit = 1;
    } 

    if (!IS_NPC(cd->defender) && IS_NPC(cd->attacker)
    && IS_NEWBIE(cd->defender)
    && (IS_SET(cd->defender->in_room->area->area_flags, AREA_NEWBIE) || IS_SET(cd->defender->in_room->room_flags, ROOM_NEWBIES_ONLY))) {
       if (cd->defender->hit < 1) cd->defender->hit =1;
    }

    if ( !IS_NPC(cd->defender)
    &&  IS_IMMORTAL(cd->defender)
    && !IS_AFFECTED(cd->defender, AFF_INCARNATED)
    && cd->defender->move < 1 ) { 
                cd->defender->move = 1;
    } 

    update_pos( cd->defender );
    

   /* Report on the effect of that blow... */

    switch( cd->defender->position ) {
    
      case POS_MORTAL:
   	act( "$n is mortally wounded, and will die soon, if not aided.",  cd->defender, NULL, NULL, TO_ROOM );
	send_to_char( "You are mortally wounded, and will die soon, if not aided.\r\n",  cd->defender );
	break;

      case POS_INCAP:
	act( "$n is incapacitated and will slowly die, if not aided.", cd->defender, NULL, NULL, TO_ROOM );
	send_to_char("You are incapacitated and will slowly die, if not aided.\r\n", cd->defender );
	break;

      case POS_STUNNED:
	act( "$n is stunned, but will probably recover.", cd->defender, NULL, NULL, TO_ROOM );
	send_to_char("You are stunned, but will probably recover.\r\n", cd->defender );
	break;

      case POS_DEAD:
	break;

    default:

        if ( cd->defender->activity != ACV_NONE
          && number_bits(8) < dam ) {
          set_activity(cd->defender, POS_FIGHTING, NULL, ACV_NONE, NULL); 
        }

        if ( dam > cd->defender->max_hit / 4 ) {

            mcc = get_mcc( cd->defender, cd->defender, cd->attacker, NULL, NULL, NULL, dam, NULL);
            wev = get_wev( WEV_DAMAGE, WEV_DAMAGE_HURT, mcc,
                      "{RThat blow hurt you badly!{x\r\n", 
                       NULL,
                      "{r@a2 looks badly hurt!{x\r\n");

          if (!room_issue_wev_challange(cd->defender->in_room, wev)) {
              free_wev(wev);
              stop_fighting(cd->attacker, TRUE);
              return TRUE;
          }

            room_issue_wev(cd->defender->in_room, wev);
            free_wev(wev);

            if ( cd->defender->activity != ACV_NONE 
            && number_bits(7) < dam ) {
                set_activity(cd->defender, POS_FIGHTING, NULL, ACV_NONE, NULL); 
            }
        }

        if ( cd->defender->hit < cd->defender->max_hit / 4 ) {

          mcc = get_mcc(cd->defender, cd->defender, cd->attacker,  NULL, NULL, NULL, cd->defender->hit, NULL);
          wev = get_wev( WEV_DAMAGE, WEV_DAMAGE_INJURED, mcc,
                      "{RYou are seriously injured!{x\r\n", 
                       NULL,
                      "{r@a2 looks seriously injured!{x\r\n");

          if (!room_issue_wev_challange(cd->defender->in_room, wev)) {
              free_wev(wev);
              stop_fighting(cd->attacker, TRUE);
              return TRUE;
          }

          room_issue_wev(cd->defender->in_room, wev);
          free_wev(wev);

          if ( cd->defender->activity != ACV_NONE 
          && number_bits(7) < dam ) {
            set_activity(cd->defender, POS_FIGHTING, NULL, ACV_NONE, NULL); 
          }
        }
        break;

    }

   /* Sleep spells and extremely wounded folks... */
     
    if (!IS_AWAKE(cd->defender)) stop_fighting( cd->defender, FALSE );

   /* Payoff for killing things... */
 
    if ( cd->defender->position == POS_DEAD ) {

       /* Purge any old queued mob commands... */

        release_mcbs(cd->defender);

       /* Death event... */

        mcc = get_mcc( cd->defender, cd->defender, cd->attacker, NULL, NULL, NULL, 0, NULL);
        wev = get_wev( WEV_DEATH, WEV_DEATH_SLAIN, mcc,
                      "{YYou are killed by @v2!{x\r\n", 
                      "{YYou have killed @a2!{x\r\n",
                      "{y@a2 is killed by @v2!{x\r\n");

        room_issue_wev(cd->defender->in_room, wev);
        free_wev(wev);

       /* Temporarily ressurect mob to react to its own death... */

        cd->defender->position = POS_STANDING;

        while ( cd->defender->mcb != NULL
        && --cd->defender->mcb->delay < 0) {
            execute_mcb(cd->defender);
        }

        cd->defender->position = POS_DEAD;

        if (!IS_NPC(cd->attacker) && !IS_NPC(cd->defender)) {
              if (!IS_SET(cd->defender->plr, PLR_ANNOYING))  cd->defender->pcdata->pk_timer = mud.pk_timer;
        
        }

       /* Give out some xps... */

	group_gain(cd->attacker, cd->defender);

	if (  IS_NPC(cd->defender) 
                && !IS_NPC(cd->attacker)) {
		
	    sprintf( log_buf, "%s killed by %s at %ld", cd->defender->short_descr, cd->attacker->name, cd->defender->in_room->vnum);
	    notify_message (NULL, WIZNET_MOBDEATH, TO_IMM, log_buf	);
	}

	if (!IS_NPC(cd->defender)) {
	
	    sprintf( log_buf, "%s killed by %s at %ld", cd->defender->name, cd->attacker->short_descr, cd->defender->in_room->vnum );
     	    log_string( log_buf );

	    notify_message (cd->defender, NOTIFY_DEATH, TO_ALL, cd->attacker->short_descr);
	    notify_message (NULL, WIZNET_DEATH, TO_IMM, log_buf);

           /* Bounty Stuff */

                    if (!IS_NPC(cd->attacker)) {
                         if (!IS_NPC(cd->defender) && cd->defender->pcdata->bounty > 0) {
                              sprintf(buf, "You recive a %ld {rCopper{x bounty, for killing %s.\r\n", cd->defender->pcdata->bounty, cd->defender->name);
                              send_to_char(buf, cd->attacker);
                              cd->attacker->gold[0] += cd->defender->pcdata->bounty;
                              cd->defender->pcdata->bounty =0;
                         }
                    }

	   /* Death penalty... */

                    lose_exp_for_dieing(cd->defender);

           /* Death sentence for attacker... */
 
                    check_perma(cd->attacker, cd->defender);

               } else {

                    if (!IS_NPC(cd->attacker)) {
                       if (IS_SET(cd->defender->act, ACT_PROTECTED)  || IS_SET(cd->defender->off_flags, ASSIST_GUARD)) {
                           send_to_char("You have a bad feeling about this\r\n", cd->attacker);
                           if (IS_SET(cd->attacker->act, ACT_CRIMINAL)) {
                                 cd->attacker->pcdata->bounty += (cd->defender->level*10);
                           } else {
                                 SET_BIT(cd->attacker->act, ACT_CRIMINAL);
                           }
                       }
                    }
               }

                 if (!IS_NPC(cd->attacker)) {
                     if (IS_NPC(cd->defender)
                     && cd->attacker->waking_room != NULL) {
                            if (IS_SET(cd->defender->form,FORM_UNDEAD)
                            || IS_SET(cd->defender->act,ACT_UNDEAD)) {
                                  if ( number_range(0,10) == 0 && IS_NPC(cd->defender)) { 
                                       raw_kill( cd->defender, FALSE );
                                       if (cd->attacker->nightmare) {
                                             if (!check_skill(cd->attacker, gsn_dreaming, 20) ) check_undead(cd->attacker,cd->defender);
                                       } else {
                                             if (!check_skill(cd->attacker, gsn_dreaming, -10) ) check_undead(cd->attacker,cd->defender);
                                       }
                                   } else {
                                       raw_kill( cd->defender, TRUE );
                                   }
                            } else {
                                   raw_kill( cd->defender, TRUE );
                                       if (cd->attacker->nightmare) {
                                             if (!check_skill(cd->attacker, gsn_dreaming, 20) ) check_spirit(cd->attacker,cd->defender);
                                       } else {
                                             if (!check_skill(cd->attacker, gsn_dreaming, -10) ) check_spirit(cd->attacker,cd->defender);
                                       }
                            }
                     } else {
                            raw_kill( cd->defender, TRUE );
                     }
                } else raw_kill( cd->defender, TRUE );

                if (cd->defender != NULL) {
                     if (cd->defender->master !=NULL) stop_follower(cd->defender);
                     die_follower(cd->defender );
                }

       /* cd->defender is no longer valid (in defender dead sequence)... */

        cd->defender = NULL;

       /* Only PCs can autoloot NPC corpses... */

       if ( !IS_NPC(cd->attacker)) {

            corpse = get_obj_list( cd->attacker, "corpse", cd->attacker->in_room->contents ); 

           /* Check AUTOLOOT and AUTOGOLD.. */
   
            if ( corpse != NULL && can_loot(cd->attacker, corpse) ) { 
                  if ( corpse->contains != NULL ) {  
	        if ( IS_SET(cd->attacker->plr, PLR_AUTOLOOT)) {
                            do_get( cd->attacker, "all corpse" );
                        } else {
 	            if ( IS_SET(cd->attacker->plr, PLR_AUTOGOLD)) do_get(cd->attacker, "money corpse");
                        }
                  }
            }
            
           /* Then check AUTOSAC... */
 
            if ( IS_SET(cd->attacker->plr, PLR_AUTOSAC) ) {
       	if ( !IS_SET(cd->attacker->plr, PLR_AUTOLOOT) 
                || corpse == NULL 
                || corpse->contains == NULL) {
		do_sacrifice( cd->attacker, "corpse" );
                }
            }
       }

       return TRUE;
    }

   /* BEGIN STUN PART... */
 
    if (cd->defender->position == POS_STUNNED ) {

       /* Purge any old queued mob commands... */

        release_mcbs(cd->defender);

       /* Death event... */

        mcc = get_mcc( cd->defender, cd->defender, cd->attacker, NULL, NULL, NULL, 0, NULL);
        wev = get_wev( WEV_DEATH, WEV_DEATH_STUN, mcc,
                      "{YYou are knocked out by @v2!{x\r\n", 
                      "{YYou have knocked out @a2!{x\r\n",
                      "{y@a2 is knocked out by @v2!{x\r\n");

        room_issue_wev(cd->defender->in_room, wev);
        free_wev(wev);

       /* Temporarily ressurect mob to react to its own death... */

        cd->defender->position = POS_STANDING;

        while ( cd->defender->mcb != NULL
        && --cd->defender->mcb->delay < 0) {
             execute_mcb(cd->defender);
        }
   
	if (  IS_NPC(cd->defender) 
                && !IS_NPC(cd->attacker)) {
	    sprintf( log_buf, "%s knocked out by %s at %ld", cd->defender->short_descr, cd->attacker->name, cd->defender->in_room->vnum);
	    notify_message (NULL, WIZNET_MOBDEATH, TO_IMM, log_buf	);
	}

	if ( !IS_NPC(cd->defender) ) {
	    sprintf( log_buf, "%s knocked out by %s at %ld", cd->defender->name, cd->attacker->short_descr, cd->defender->in_room->vnum );
	    log_string( log_buf );
	    notify_message (cd->defender, NOTIFY_DEATH, TO_ALL, cd->attacker->short_descr);
	    notify_message (NULL, WIZNET_DEATH, TO_IMM, log_buf);
                 }

          cd->defender->position = POS_STUNNED;

           if (cd->defender->master !=NULL) stop_follower(cd->defender);
           die_follower(cd->defender );
           cd->defender->fighting = NULL;
           cd->attacker->fighting = NULL;
           stop_fighting( cd->attacker, TRUE );
           stop_fighting( cd->defender, TRUE );

           if (!IS_NPC(cd->defender)) {
               if (cd->defender->pet !=NULL) cd->defender->pet->ridden = FALSE;
               cd->defender->pcdata->mounted = FALSE;
            }
            
           set_activity( cd->attacker, POS_STANDING, NULL, ACV_NONE, NULL);

           if (cd->defender->in_room->area->prison > 0
           && IS_SET(cd->attacker->off_flags, ASSIST_GUARD)) {
               ROOM_INDEX_DATA *prison = get_room_index(cd->defender->in_room->area->prison);

               act("$N takes $n away.", cd->defender, NULL, cd->attacker, TO_ROOM);
               char_from_room(cd->defender);
               char_to_room(cd->defender, prison);
               sprintf_to_char(cd->defender, "You've been taken to %s.\r\n", prison->name);
           }
           
            cd->defender = NULL;
            return TRUE;
    }

/*End*/

    if ( cd->attacker == cd->defender ) return TRUE;

   /* Take care of link dead people... */ 
     
    if ( !IS_NPC(cd->defender) 
      && ( cd->defender->desc == NULL 
        || cd->defender->desc->ok != TRUE )) {
    
	if ( number_range( 0, cd->defender->wait ) == 0 ) {

	    do_recall( cd->defender, "" );
	    return TRUE;
	}
    }

   /* Wimp out? */
     
    if ( IS_NPC(cd->defender) 
      && cd->defender->wait < PULSE_VIOLENCE / 2) {

            if ( IS_SET(cd->defender->act, ACT_WIMPY) 
            && number_bits( 2 ) == 0
            && cd->defender->hit < cd->defender->max_hit / 5 ) {
                  do_flee( cd->defender, "");
            } else if ( IS_AFFECTED(cd->defender, AFF_CHARM) 
            && cd->defender->master != NULL
            && str_cmp(race_array[cd->defender->race].name,"machine")
            && !IS_SET(cd->defender->form, FORM_MACHINE)
            && cd->defender->master->in_room != cd->defender->in_room ) {
                  do_flee( cd->defender, "" );
            }
    }

    if (!IS_NPC(cd->defender) && IS_NPC(cd->attacker)
    && IS_NEWBIE(cd->defender)
    && (IS_SET(cd->defender->in_room->area->area_flags, AREA_NEWBIE) || IS_SET(cd->defender->in_room->room_flags, ROOM_NEWBIES_ONLY))) {
        if (cd->defender->hit < cd->defender->max_hit /10) {
             cd->defender->hit = UMAX(1, cd->defender->hit);
             stop_fighting(cd->defender, TRUE);
             send_to_char("That was close!\r\n", cd->defender);
        }
    }

    if (!IS_NPC(cd->defender)
      && cd->defender->hit > 0
      && cd->defender->hit <= cd->defender->wimpy
      && cd->defender->wait < PULSE_VIOLENCE / 2 ) {
	do_flee( cd->defender, "" );
    }

    if (!IS_NPC(cd->defender)
    && IS_NPC(cd->attacker)
    && cd->defender->hit < cd->defender->max_hit /20
    && cd->attacker->hit > cd->attacker->max_hit /5
    && IS_SET(cd->attacker->off_flags, OFF_MERCY)) {
                do_say(cd->attacker, "Come back when you're a real challange!");
                stop_fighting(cd->attacker, TRUE);
    }

    tail_chain( );
    return TRUE;
}


/* Inflict damage from the environment... */
 
/* Retruns TRUE if the victim is alive, FALSE otherwise... */

bool env_damage( CHAR_DATA *ch, int dam, int dam_type, char *msg_all, char *msg_victim) {
bool immune;
int imm_lev;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

   /* Can't damage a corpse any more... */

    if (ch->position == POS_DEAD ) return FALSE;
    
    if (ch->position == POS_STUNNED
    && ch->move<1) {
      return FALSE;
    }

   /* Stop up any residual loopholes... */
    
    if ( dam > 1000 ) {
      bug( "Environmental damage: %d > 1000 points!", dam );
      dam = 1000;
    }

   /* damage reduction */
   /* This *is* needed, do not remove. Changed values though */

    if ( dam > 60) dam = (dam - 60)/2 + 60;
    if ( dam > 125) dam = (dam - 125)/2 + 125; 
      

   /* Check for Bit affects... */
   
    if ( IS_AFFECTED(ch, AFF_SANCTUARY) ) dam /= 2;

     if ( IS_AFFECTED(ch, AFF_ELDER_SHIELD) ) {
         AFFECT_DATA *af;

         if (dam_type == DAM_PIERCE
         || dam_type == DAM_BASH
         ||  dam_type == DAM_SLASH) {
             dam /= 4;
             for (af = ch->affected; af ; af = af->next) {
                  if (af->bitvector == AFF_ELDER_SHIELD) {
                       if (number_percent() < 5) af->duration = UMAX(af->duration--, 0);
                  }
             }
        }
     }

    if ( (IS_AFFECTED(ch, AFF_PROTECTE)) 
      && (dam_type == DAM_NEGATIVE) ) {
      dam /= 2;
    }

    if ( (IS_AFFECTED(ch, AFF_PROTECTG))
      && (dam_type == DAM_HOLY) ) {
      dam /= 2;
    }
 
    if ( dam_type == DAM_DROWNING
      && IS_AFFECTED(ch, AFF_WATER_BREATHING)) {  
      dam = 0; 
    }

   /* Check for Spell affects... */
 	
    if ( dam_type == DAM_COLD 
      && is_affected(ch, gafn_fire_shield)) {
      dam -= dam/2;
    }

    if ( dam_type == DAM_FIRE 
      && is_affected(ch, gafn_frost_shield)) {
      dam -= dam/2;
    }

   /* No damage, no message... */

    if ( dam == 0 ) return TRUE; 
    
   /* Check for damage immunity... */

    immune = FALSE;
    imm_lev = check_immune(ch, dam_type);

    switch (imm_lev) {

      case(IS_IMMUNE):
                immune = TRUE;
	dam = 0;
	break;

      case(IS_ENV_IMMUNE):
      case(IS_RESISTANT_PLUS):	
                immune = TRUE;
	dam = 0;
	break;

      case(IS_RESISTANT):	
	dam -= dam/3;
	break;

      case(IS_VULNERABLE):
	dam += dam/2;
	break;

      default:
        break;
    }

   /* No messages if immune either... */

    if (immune) return TRUE;
       	
   /* Notify of damage... */

    mcc = get_mcc( ch, NULL, ch, NULL, NULL, NULL, dam, NULL);
    wev = get_wev( WEV_DAMAGE, WEV_DAMAGE_ENV, mcc, NULL, msg_victim, msg_all);
    room_issue_wev(ch->in_room, wev);
    free_wev(wev);

   /* Bail out if no damage... */
 
    if (dam == 0) return TRUE;
    
   /* Apply damage... */

    ch->hit -= dam;
    if (dam > ch->max_hit / 8 && !IS_SET(ch->act, ACT_UNDEAD))  SET_BIT(ch->form,FORM_BLEEDING);

   /* Immortals cannot die... */

    if ( (!IS_NPC(ch))
      && (IS_IMMORTAL(ch))
      && !IS_AFFECTED(ch, AFF_INCARNATED)
      && (ch->hit < 1) ) {
      ch->hit = 1;
    }

/* Mobs shouldn't die from bleeding during combat*/

    if (ch->fighting != NULL
    && ch->hit < 1) {
       ch->hit = 1;
    }
    
   /* Work out new position... */ 

    update_pos( ch );
    switch( ch->position ) {
  
      case POS_MORTAL:
        act( "$n is mortally wounded, and will die soon, if not aided.", ch, NULL, NULL, TO_ROOM );
        send_to_char("You are mortally wounded, and will die soon, if not aided.\r\n",   ch );
        break;

      case POS_INCAP:
	act( "$n is incapacitated and will slowly die, if not aided.",
	    ch, NULL, NULL, TO_ROOM );
	send_to_char(
	    "You are incapacitated and will slowly die, if not aided.\r\n",
	    ch );
	break;

      case POS_STUNNED:
	act( "$n is stunned, but will probably recover.",
	    ch, NULL, NULL, TO_ROOM );
	send_to_char("You are stunned, but will probably recover.\r\n",
	    ch );
	break;

      case POS_DEAD:
	act( "{Y$n is DEAD!!{x", ch, 0, 0, TO_ROOM );
	send_to_char( "{RYou have been KILLED!!{x\r\n\r\n", ch );
	break;

      default:
	if ( dam > ch->max_hit / 4 ) {

          mcc = get_mcc( ch, ch, NULL, NULL, NULL, NULL, dam, NULL);
          wev = get_wev( WEV_DAMAGE, WEV_DAMAGE_HURT, mcc,
                      "{RThat hurt you badly!{x\r\n", 
                       NULL,
                      "{r@a2 looks badly hurt!{x\r\n");

          room_issue_wev(ch->in_room, wev);
          free_wev(wev);
        }

	if ( ch->hit < ch->max_hit / 4 ) {

          mcc = get_mcc( ch, ch, NULL, NULL, NULL, NULL, ch->hit, NULL);
          wev = get_wev( WEV_DAMAGE, WEV_DAMAGE_INJURED, mcc,
                      "{RYou are seriously injured!{x\r\n", 
                       NULL,
                      "{r@a2 looks seriously injured!{x\r\n");

          room_issue_wev(ch->in_room, wev);
          free_wev(wev);
        }

	break;
    }

   /* Oh-oh, it's dead... */

    if ( ch->position == POS_DEAD ) {

      if (ch->fighting == NULL) {
          sprintf( log_buf, "%s killed by environment at %ld",
                             ch->short_descr,
                             ch->in_room->vnum);
      } else {
          sprintf( log_buf, "%s killed indirectly by %s at %ld",
                             ch->short_descr,
                             ch->fighting->short_descr,
                             ch->in_room->vnum);
      }
      
      if ( IS_NPC(ch) ) {
	notify_message (NULL, WIZNET_MOBDEATH, TO_IMM, log_buf	);
      } else {
        notify_message (NULL, WIZNET_DEATH, TO_IMM, log_buf);
        log_string( log_buf );
        lose_exp_for_dieing(ch); 

       /* Death sentence for attacker... */
 
        if ( ch->fighting != NULL ) {
          check_perma(ch->fighting, ch);
        } 
      }

     /* Lose the body, completely... */

      raw_kill( ch, TRUE );

     /* Tell the caller we killed them... */ 

      return FALSE;
    }

    
   /* Take care of link dead people... */
    
    if ( !IS_NPC(ch) 
      && ( ch->desc == NULL 
        || ch->desc->ok != TRUE )) {
     
      if ( number_range( 0, ch->wait ) == 0 ) {
	
	do_flee( ch, "" );
	return TRUE;
      }
    }
    
   /* Wimp out? */
     
    if ( IS_NPC(ch) && dam > 0 && ch->wait < PULSE_VIOLENCE / 2) {
      if ( ( IS_SET(ch->act, ACT_WIMPY) && number_bits( 2 ) == 0
	&&   ch->hit < ch->max_hit / 5) 
	||   ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL
	&&     ch->master->in_room != ch->in_room ) ) {
	do_flee( ch, "" );
      }
    }

    return TRUE;
}


bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim ) {

   /* No killing in shops */

    if ( IS_NPC(victim) 
    && victim->pIndexData->pShop != NULL
    && victim->fighting == NULL) {
	send_to_char("The shopkeeper wouldn't like that.\r\n",ch); 
                return TRUE;
    }

   /* No killing healers, adepts, etc */

    if ( IS_NPC(victim) 
      && ( IS_SET(victim->act, ACT_PRACTICE)
        ||  IS_SET(victim->act, ACT_IS_ALIENIST)
        ||  IS_SET(victim->act, ACT_WATCHER)
        ||  IS_SET(victim->act, ACT_IS_HEALER))) {
	send_to_char("I think Marduk would {rNOT{x approve.\r\n",ch);
	return TRUE;
    }

   /* No fighting in safe rooms */

    if (IS_SET(ch->in_room->room_flags,ROOM_SAFE)) {
	send_to_char("Not in this room.\r\n",ch);
	return TRUE;
    }

   /* Not safe if already fighting... */

    if (victim->fighting == ch) return FALSE;

   /* Simplified PK rules... */

    if ( !IS_NPC(ch)
      && !IS_NPC(victim) ) {
 
     /* Newbies may not initiate PK... */

      if ( IS_NEWBIE(ch) ) {
        send_to_char("Newbies may not initiate combat with players.\r\n", ch);
        return TRUE;
      }

     /* Newbies may not be attacked in PK... */

      if ( IS_NEWBIE(victim)
      && !IS_IMMORTAL(ch) ) {
        send_to_char("You may not initiate combat with newbies.\r\n", ch);
        return TRUE;
      }

     /* Mortals may not attack gods... */

      if ( IS_IMMORTAL(victim)
      && !IS_AFFECTED(victim, AFF_INCARNATED)
      && !IS_IMMORTAL(ch) ) {
        send_to_char("Mortals may not attack gods.\r\n", ch);
        return TRUE;
      }

    }

    return FALSE;
}

bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area ) {

   /* Immortals not hurt in area attacks */

    if ( IS_IMMORTAL(victim)  
    && !IS_AFFECTED(victim, AFF_INCARNATED)
    &&  area)   return TRUE;
    
   /* No killing in shops */

    if ( IS_NPC(victim) 
    && victim->pIndexData->pShop != NULL
    && victim->fighting == NULL) {
        return TRUE;
    }

   /* No killing healers, adepts, etc */

    if ( IS_NPC(victim)
      && ( IS_SET(victim->act, ACT_PRACTICE)
        ||  IS_SET(victim->act, ACT_IS_ALIENIST)
        ||  IS_SET(victim->act, ACT_WATCHER)
        ||  IS_SET(victim->act, ACT_IS_HEALER))) {
	return TRUE;
    } 

   /* No fighting in safe rooms */

    if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE)) {
        return TRUE;
    }

   /* You can do wahtever if already in combat... */

    if (victim->fighting == ch) return FALSE;
 
   /* Cannot use spells if not in same group */

    if ( victim->fighting != NULL 
      && !is_same_group(ch, victim->fighting)) {
      return TRUE;
    }
 
   /* Simplified PK rules... */

    if ( !IS_NPC(ch)
      && !IS_NPC(victim) ) {
 
     /* Newbies may not initiate PK... */

      if ( IS_NEWBIE(ch) ) {
        return TRUE;
      }

     /* Mortals may not attack gods... */

      if ( IS_IMMORTAL(victim)
      && !IS_AFFECTED(victim, AFF_INCARNATED)
      && !IS_IMMORTAL(ch) ) {
        return TRUE;
      }

    }

    return FALSE;
}


/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim ) {

    /*
     * Follow charm thread to responsible character.
     * Attacking someone's charmed char is hostile!
     */
    while ( IS_AFFECTED(victim, AFF_CHARM) 
         && victim->master != NULL ) {
	victim = victim->master;
    }

    /*
     * NPC's are fair game.
     * So are killers and thieves.
     */
    if ( IS_NPC(victim)
    ||   IS_SET(victim->plr, PLR_ANNOYING)
    ||	 IS_SET(victim->plr, PLR_PK)) {
	return;
    }

    /*
     * Charm-o-rama.
     */
    if ( IS_SET(ch->affected_by, AFF_CHARM) ) {

	if ( ch->master == NULL ) {

	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "Check_killer: %s bad AFF_CHARM",
		IS_NPC(ch) ? ch->short_descr : ch->name );
	    bug( buf, 0 );
	    affect_strip_afn( ch, gafn_charm);
	    REMOVE_BIT( ch->affected_by, AFF_CHARM );
	    return;
	}

	return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * So is being immortal (Alander's idea).
     * And current killers stay as they are.
     */
    if ( IS_NPC(ch)
    ||   ch == victim
    ||   IS_IMMORTAL(ch)
    ||   IS_SET(ch->plr, PLR_ANNOYING) )
	return;

    return;
}


/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim ) {
   OBJ_DATA *obj;

    victim->move = UMAX(victim->move, -2*victim->max_move);

    if ( victim->hit > 0
    && victim->move>0) {
            if ( victim->position <= POS_STUNNED) { 
              if (!IS_SET(victim->act, ACT_BOUND)) {
	    set_activity( victim, POS_STANDING, NULL, ACV_NONE, NULL);
                   act("$n awakes with a splitting headache.",victim,NULL,NULL,TO_ROOM);
                   send_to_char("You awake with a splitting headache.\r\n", victim);
                   if (IS_NPC(victim)) {
                        victim->move=victim->max_move;
                   } else {
                        if (victim->move < victim->max_move/3) victim->move = victim->max_move/3;
                   }
                    if (victim->master !=NULL) {
                        send_to_char("Your victim suddenly awakes.\r\n", victim->master);
                        stop_follower(victim);
                    }
              } else {
                    if ( ( obj = get_eq_char( victim, WEAR_ARMS ) ) != NULL ) {
                           if (obj->value[0] < get_curr_stat(victim, STAT_STR)
                           &&  number_percent() < 25) {
                                 unequip_char( victim, obj );
                                 REMOVE_BIT (victim->act, ACT_BOUND);
                                 send_to_char("You struggle your way out of your chains.\r\n", victim);
                                 set_activity( victim, POS_STANDING, NULL, ACV_NONE, NULL);
                                 if (IS_NPC(victim)) {
                                      victim->move=victim->max_move;
                                 } else {
                                      if (victim->move < victim->max_move/3) victim->move = victim->max_move/3;
                                 }
                                 if (victim->master !=NULL) {
                                    act("$n frees his hands and escapes.",victim,NULL,NULL,TO_ROOM);
                                    stop_follower(victim);
                                  }
                           }else {
                                  send_to_char("You try to free your hands.\r\n", victim);
                                  act("$n struggles against his chains.",victim,NULL,NULL,TO_ROOM);
                           }
                     } else {
                           REMOVE_BIT (victim->act, ACT_BOUND);
                           send_to_char("Your chains are suddenly gone.\r\n", victim);
                           set_activity( victim, POS_STANDING, NULL, ACV_NONE, NULL);
                           if (IS_NPC(victim)) {
                                  victim->move=victim->max_move;
                           } else {
                                   if (victim->move < victim->max_move/3) victim->move = victim->max_move/3;
                           }
                           if (victim->master !=NULL) {
                                act("$n frees his hands and escapes.",victim,NULL,NULL,TO_ROOM);
                                stop_follower(victim);
                           }
                      }  
                 }
              }
               return;
     }

    if ( IS_NPC(victim) && victim->hit < 1) {
	set_activity( victim, POS_DEAD, NULL, ACV_NONE, NULL);
	return;
    }

    if (victim->move<0) {
       set_activity( victim, POS_STUNNED, NULL, ACV_NONE, NULL);
       if (IS_NPC(victim)) {
           for (obj = victim->carrying ; obj != NULL ; obj = obj->next_content) {
                 if (obj->wear_loc != WEAR_NONE) {
                       if ( obj->item_type != ITEM_TATTOO) {
                             remove_obj(victim, obj->wear_loc, TRUE);	    
                       }
	 }
            }
        }
        return;
    }

    if ( victim->hit <= -11 ) {
	set_activity( victim, POS_DEAD, NULL, ACV_NONE, NULL);
                if (victim->huntedBy != NULL) {
                       victim->huntedBy->prey = NULL;
                       victim->huntedBy = NULL;
                }
	return;
    }

    if ( victim->hit <= -6 ) {
	set_activity( victim, POS_MORTAL, NULL, ACV_NONE, NULL);
    } else {
      if ( victim->hit <= -3 ) {
        set_activity( victim, POS_INCAP, NULL, ACV_NONE, NULL);
      } else { 
        set_activity( victim, POS_STUNNED, NULL, ACV_NONE, NULL);
      }
    }  
    
    return;
}



/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim ) {

    if ( ch->fighting != NULL ) {
	bug( "Set_fighting: already fighting", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) ) {
	affect_strip_afn( ch, gafn_sleep );
    }

    ch->fighting = victim;

    set_activity( ch, POS_FIGHTING, NULL, ACV_NONE, NULL);

    if (ch->chat_state == 0) {
      ch->chat_state = 1;
    } 

    return;
}



/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth ) {
    CHAR_DATA *fch;

    for ( fch = char_list; fch != NULL; fch = fch->next ) {

	if ( fch == ch 
                || ( fBoth && fch->fighting == ch ) ) {
	    fch->fighting = NULL;
                    fch->distractedBy = NULL;

                    if ( fch->activity != ACV_CASTING ) {
                         if (IS_NPC(fch) ) {
                            set_activity( fch, fch->default_pos, NULL, ACV_NONE, NULL);
                         } else {
                            set_activity( fch, POS_STANDING, NULL, ACV_NONE, NULL);
                         }
                    }
   
                    if (fch->chat_state == 1) fch->chat_state = 0;
	    update_pos( fch );
	}
    }

    return;
}



/*
 * Make a corpse out of a character.
 */

void make_corpse( CHAR_DATA *ch ) {
char buf[MAX_STRING_LENGTH];
OBJ_DATA *corpse;
OBJ_DATA *obj;
OBJ_DATA *obj_next;
char *name;
ROOM_INDEX_DATA *location;
long zmask;
bool create_corpse = TRUE;

   /* Work out where the corpse will go... */

    location	= NULL;
    zmask		= 0;
    ch->awakening	= FALSE;

    if ( !IS_NPC(ch) ) {

     /* Dreamers may get a rude awakening... */

      if ( ch->waking_room != NULL ) {
        if (ch->nightmare) {
             if (check_skill(ch, gsn_dreaming, 0) ) {
                  location = ch->waking_room;
                  ch->awakening = TRUE;
                  ch->nightmare = FALSE;
             }
        } else {
             if ( !check_skill(ch, gsn_dreaming, 0) ) {
                   location = ch->waking_room;
                   ch->awakening = TRUE;
             }
        }
      }
     
     /* Determine normal morgue location... */ 
      
      if ( location == NULL
        && ch->in_room != NULL
        && ch->in_room->morgue != 0 ) {

        location = get_room_index( ch->in_room->morgue );
      } 

      if ( location == NULL
        && ch->in_room != NULL
        && ch->in_room->area != NULL
        && ch->in_room->area->morgue != 0 ) {

        location = get_room_index( ch->in_room->area->morgue );
      } 

      if ( location == NULL
        && ch->in_room != NULL
        && ch->in_room->area != NULL
        && zones[ch->in_room->area->zone].morgue != 0 ) {

        location = get_room_index( ch->in_room->area->morgue );
      }
  
      if ( location == NULL
        && mud.morgue != 0 ) { 
        location = get_room_index(mud.morgue);
      } 
    } 

    if (location == NULL) location = ch->in_room;
   
    if ( location != 0
    && location->area != 0 ) {
        zmask = zones[location->area->zone].mask;
    }

    if (IS_RAFFECTED(ch->in_room, RAFF_NOMORGUE)) {
        location = ch->in_room;
    }    

   /* Make the basic corpse... */

    if (IS_NPC(ch) ) {
        int t;
	name		= ch->short_descr;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
	corpse->timer	= number_range( 4, 8 );
                for (t = 0; t < MAX_CURRENCY; t++) { 
	    if (ch->gold[t] > 0 ) {
	         obj_to_obj(create_money(ch->gold[t], t), corpse );
	         ch->gold[t] = 0;
                    }
	}
	corpse->cost = 0;

                 if (IS_SET(ch->form, FORM_INSTANT_DECAY)) create_corpse = FALSE;
    } else {
	name		= ch->name;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
	corpse->timer	= number_range( 30, 50 );
	REMOVE_BIT(ch->plr, PLR_CANLOOT);
	if ( !IS_SET(ch->plr, PLR_ANNOYING)) {
	    corpse->owner = str_dup(ch->name);
	} else {
	    corpse->owner = NULL;
        }
	corpse->cost = 0;
    }

    if (IS_SET(ch->form, FORM_UNDEAD)) corpse->timer = UMAX(corpse->timer/4, 1);
    corpse->level = ch->level;
    switch(get_char_size(ch)) {
          default:
              corpse->weight = 240;
              break;

          case SIZE_TINY:
              corpse->weight = 25;
              break;

          case SIZE_SMALL:
              corpse->weight = 75;
              break;

          case SIZE_MEDIUM:
              corpse->weight = 200;
              break;

          case SIZE_LARGE:
              corpse->weight = 300;
              break;

          case SIZE_HUGE:
              corpse->weight = 600;
              break;

          case SIZE_GIANT:
              corpse->weight = 1500;
              break;
     }

    corpse->material = 0;
    if (ch->material == 0) corpse->material = race_array[ch->race].material;
    else corpse->material = ch->material;
    if (corpse->material == 0) corpse->material = MATERIAL_FLESH;

    if(IS_SET(ch->nature, NATURE_STRONG)) nature_to_app(corpse, APPLY_STR, 1, 0);
    if(IS_SET(ch->nature, NATURE_FEEBLE)) nature_to_app(corpse, APPLY_STR, -2, 0);
    if(IS_SET(ch->nature, NATURE_SMART)) nature_to_app(corpse, APPLY_INT, 1, 0);
    if(IS_SET(ch->nature, NATURE_DUMB)) nature_to_app(corpse, APPLY_INT, -2, 0);
    if(IS_SET(ch->nature, NATURE_AGILE)) nature_to_app(corpse, APPLY_DEX, 1, 0);
    if(IS_SET(ch->nature, NATURE_LUMBERING)) nature_to_app(corpse, APPLY_DEX, -2, 0);
    if(IS_SET(ch->nature, NATURE_ROBUST)) nature_to_app(corpse, APPLY_CON, 1, 0);
    if(IS_SET(ch->nature, NATURE_SICKLY)) nature_to_app(corpse, APPLY_CON, -2, 0);
    if(IS_SET(ch->nature, NATURE_SLY)) nature_to_app(corpse, APPLY_WIS, 1, 0);
    if(IS_SET(ch->nature, NATURE_GULLIBLE)) nature_to_app(corpse, APPLY_WIS, -2, 0);
    if(IS_SET(ch->nature, NATURE_STURDY)) nature_to_app(corpse, APPLY_HIT, 25, 0);
    if(IS_SET(ch->nature, NATURE_FRAGILE)) nature_to_app(corpse, APPLY_HIT, -50, 0);
    if(IS_SET(ch->nature, NATURE_MAGICAL)) nature_to_app(corpse, APPLY_MANA, 25, 0);
    if(IS_SET(ch->nature, NATURE_MUNDAIN)) nature_to_app(corpse, APPLY_MANA, -50, 0);
    if(IS_SET(ch->nature, NATURE_VISCIOUS)) nature_to_app(corpse, APPLY_HITROLL, 3, 0);
    if(IS_SET(ch->nature, NATURE_HARMLESS)) nature_to_app(corpse, APPLY_HITROLL, -6, 0);
    if(IS_SET(ch->nature, NATURE_ARMOURED)) nature_to_app(corpse, APPLY_AC, -5, 0);
    if(IS_SET(ch->nature, NATURE_EXPOSED)) nature_to_app(corpse, APPLY_AC, 10, 0);
    if(IS_SET(ch->nature, NATURE_MONSTEROUS)) {
            nature_to_app(corpse, APPLY_HITROLL, 3, 0);
            nature_to_app(corpse, APPLY_DAMROLL, 3, 0);
    }

    if(IS_AFFECTED(ch, AFF_WATER_BREATHING) && number_percent() < 12) nature_to_app(corpse, 0, 0, AFF_WATER_BREATHING);
    if(IS_AFFECTED(ch, AFF_INVISIBLE) && number_percent() < 5) nature_to_app(corpse, 0, 0, AFF_INVISIBLE);
    if(IS_AFFECTED(ch, AFF_DETECT_EVIL) && number_percent() < 15) nature_to_app(corpse, 0, 0, AFF_DETECT_EVIL);
    if(IS_AFFECTED(ch, AFF_DETECT_INVIS) && number_percent() < 10) nature_to_app(corpse, 0, 0, AFF_DETECT_INVIS);
    if(IS_AFFECTED(ch, AFF_DETECT_MAGIC) && number_percent() < 15) nature_to_app(corpse, 0, 0, AFF_DETECT_MAGIC);
    if(IS_AFFECTED(ch, AFF_DETECT_HIDDEN) && number_percent() < 10) nature_to_app(corpse, 0, 0, AFF_DETECT_HIDDEN);
    if(IS_AFFECTED(ch, AFF_SANCTUARY) && number_percent() < 2) nature_to_app(corpse, 0, 0, AFF_SANCTUARY);
    if(IS_AFFECTED(ch, AFF_FAERIE_FIRE) && number_percent() < 20) nature_to_app(corpse, 0, 0, AFF_FAERIE_FIRE);
    if(IS_AFFECTED(ch, AFF_INFRARED) && number_percent() < 12) nature_to_app(corpse, 0, 0, AFF_INFRARED);
    if(IS_AFFECTED(ch, AFF_CURSE) && number_percent() < 15) nature_to_app(corpse, 0, 0, AFF_CURSE);
    if(IS_AFFECTED(ch, AFF_SNEAK) && number_percent() < 5) nature_to_app(corpse, 0, 0, AFF_SNEAK);
    if(IS_AFFECTED(ch, AFF_HIDE) && number_percent() < 8) nature_to_app(corpse, 0, 0, AFF_HIDE);
    if(IS_AFFECTED(ch, AFF_FLYING) && number_percent() < 10) nature_to_app(corpse, 0, 0, AFF_FLYING);
    if(IS_AFFECTED(ch, AFF_HASTE) && number_percent() < 4) nature_to_app(corpse, 0, 0, AFF_HASTE);
    if(IS_AFFECTED(ch, AFF_DARK_VISION) && number_percent() < 9) nature_to_app(corpse, 0, 0, AFF_DARK_VISION);
    if(IS_AFFECTED(ch, AFF_SWIM) && number_percent() < 15) nature_to_app(corpse, 0, 0, AFF_SWIM);
    if(IS_AFFECTED(ch, AFF_REGENERATION) && number_percent() < 4) nature_to_app(corpse, 0, 0, AFF_REGENERATION);
    if(IS_AFFECTED(ch, AFF_ABSORB) && number_percent() < 2) nature_to_app(corpse, 0, 0, AFF_ABSORB);

   /* Create descriptions... */

    sprintf( buf, corpse->short_descr, name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );

   /* Transfer equipment... */

    for ( obj = ch->carrying; obj != NULL; obj = obj_next ) {
	obj_next = obj->next_content;

        if (create_corpse) {
	obj_from_char( obj );
                if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)) {
                    obj_to_room(obj, ch->in_room);
                    continue;
                }

       /* Adjust timers... */
	if (obj->item_type == ITEM_POTION) obj->timer = number_range(500,1000);
        	if (obj->item_type == ITEM_SCROLL) obj->timer = number_range(1000,2500);
	if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH)) obj->timer = number_range(5,10);
        
       /* Remove death effect and troublesome flags... */
	REMOVE_BIT(obj->extra_flags, ITEM_VIS_DEATH);  /* visible upon death */
	REMOVE_BIT(obj->extra_flags, ITEM_ROT_DEATH);  /* decays on death */
	REMOVE_BIT(obj->extra_flags, ITEM_INVENTORY);

       /* Out of corpses zone equipment... */
 
        if ( zmask != 0
          && obj->zmask != 0
          && (zmask & obj->zmask) == 0 ) {

         /* Goes into thier OOZ list... */

          obj_to_char_ooz( obj, ch ); 

        } else {

         /* The rest goes to the corpse... */

          obj_to_obj( obj, corpse );
       
        }
       } else {
           obj_from_char(obj );
           extract_obj(obj);
       }


    }

   /* Transfer ooz equipment... */

    for ( obj = ch->ooz; obj != NULL; obj = obj_next ) {
	obj_next = obj->next_content;

          if (create_corpse) {
	obj_from_char_ooz( obj );
                if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)) {
                    obj_to_room(obj, ch->in_room);
                    continue;
                }

       /* Adjust timers... */
	if (obj->item_type == ITEM_POTION) obj->timer = number_range(500,1000);
        	if (obj->item_type == ITEM_SCROLL) obj->timer = number_range(1000,2500);
        	if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH)) obj->timer = number_range(5,10);
        
       /* Remove death effect and troublesome flags... */
	REMOVE_BIT(obj->extra_flags, ITEM_VIS_DEATH);  /* visible upon death */
	REMOVE_BIT(obj->extra_flags, ITEM_ROT_DEATH);  /* decays on death */
	REMOVE_BIT(obj->extra_flags, ITEM_INVENTORY);

       /* Out of corpses zone equipment... */
 
        if ( zmask != 0
          && obj->zmask != 0
          && (zmask & obj->zmask) == 0 ) {

         /* Goes into thier OOZ list... */

          obj_to_char_ooz( obj, ch ); 

        } else {

         /* The rest goes to the corpse... */

          obj_to_obj( obj, corpse );
        }
       } else {
            obj_from_char_ooz(obj );
            extract_obj(obj);
       }
    }

   /* Set up the corpses values to reflect the thing that was killed... */

    corpse->value[0] = 0;
    if (ch->pIndexData != NULL) corpse->value[0] = ch->pIndexData->vnum;
    corpse->value[1] = ch->alignment;
    corpse->value[2] = ch->level;
    corpse->value[3] = ch->sex;
    corpse->value[4] = 0;
    if (IS_SET(ch->form, FORM_SENTIENT)) corpse->value[4] = 1;
    if (IS_SET(ch->form, FORM_UNDEAD)) corpse->value[4] += 2;
   /* Time to deliver it... */

  
   if (create_corpse) obj_to_room( corpse, location );
   else extract_obj(corpse);

    return;
}


void nature_to_app(OBJ_DATA *obj, int loc, int mod, long long bv) {
AFFECT_DATA *pAf;

        if (number_percent() > 30) return;
        pAf             =   new_affect();
        pAf->location   =   loc;
        pAf->skill	    =   0;
        pAf->modifier   =   mod;
        pAf->type       =   SKILL_UNDEFINED;
        pAf->afn        =   0;
        pAf->duration   =   -1;
        pAf->bitvector  =   bv;
        pAf->next       =   obj->affected;
        obj->affected  =   pAf;
        return;
}


/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( CHAR_DATA *ch, bool corpse) {
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg = "You hear the horrible death cry of $n.";

    if (corpse) {
      switch ( number_bits(4))
      {
      case  0: msg  = "$n hits the ground ... {RDEAD{x."; 	break;
      case  1: 
  	if (ch->material == 0)
  	{
	    msg  = "$n splatters {Rblood{x on your armor.";		
	    break;
	}
      case  2: 							
  	if (IS_SET(ch->parts,PART_GUTS))
	{
	    msg = "$n spills $s guts all over the floor.";
	    vnum = OBJ_VNUM_GUTS;
	}
	break;
      case  3: 
	if (IS_SET(ch->parts,PART_HEAD))
	{
	    msg  = "$n's severed head plops on the ground.";
	    vnum = OBJ_VNUM_SEVERED_HEAD;				
	}
	break;
      case  4: 
	if (IS_SET(ch->parts,PART_HEART))
	{
	    msg  = "$n's heart is torn from $s chest.";
	    vnum = OBJ_VNUM_TORN_HEART;				
	}
	break;
      case  5: 
	if (IS_SET(ch->parts,PART_ARMS))
	{
	    msg  = "$n's arm is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_ARM;				
	}
	break;
      case  6: 
	if (IS_SET(ch->parts,PART_LEGS))
	{
	    msg  = "$n's leg is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_LEG;				
	}
	break;
      case 7:
	if (IS_SET(ch->parts,PART_BRAINS))
	{
	    msg = "$n's head is shattered, and $s brains splash all over you.";
	    vnum = OBJ_VNUM_BRAINS;
  	}
      }

      act( msg, ch, NULL, NULL, TO_ROOM );

      if ( vnum != 0 )
      {
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char *name;

	name		= IS_NPC(ch) ? ch->short_descr : ch->name;
	obj		= create_object( get_obj_index( vnum ), 0 );
	obj->timer	= number_range( 4, 7 );

	sprintf( buf, obj->short_descr, name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, obj->description, name );
	free_string( obj->description );
	obj->description = str_dup( buf );

	if (obj->item_type == ITEM_FOOD)
	{
	    if (IS_SET(ch->form,FORM_POISON))
		obj->value[3] = 1;
	    else if (!IS_SET(ch->form,FORM_EDIBLE))
		obj->item_type = ITEM_TRASH;
	}

	obj_to_room( obj, ch->in_room );
      }
    }

    if ( IS_NPC(ch) )
	msg = "You hear something's death cry.";
    else
	msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    for ( door = 0; door < DIR_MAX; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = was_in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   pexit->u1.to_room != was_in_room )
	{
	    ch->in_room = pexit->u1.to_room;
	    act( msg, ch, NULL, NULL, TO_ROOM );
	}
    }
    ch->in_room = was_in_room;

    return;
}


void disband_hunters(CHAR_DATA *ch) {
CHAR_DATA *hunter;

     for (hunter = char_list; hunter; hunter = hunter->next) {
         if (hunter == ch) continue;
         if (!IS_NPC(hunter)) continue;
         if (hunter->prey == ch) {
             hunter->prey = NULL;
         }
     }

     ch->huntedBy = NULL;
     return;
}


void raw_kill( CHAR_DATA *victim, bool corpse ) {
OBJ_DATA *tattoo;
int i;

    stop_fighting( victim, TRUE );
    disband_hunters(victim);
    mprog_death_trigger( victim, corpse );

    tattoo = NULL;
 
    if (corpse 
    || !IS_NPC(victim)) {
       tattoo=get_eq_char(victim, WEAR_TATTOO);
       if (tattoo != NULL) obj_from_char(tattoo);
       REMOVE_BIT(victim->act,ACT_CRIMINAL);
       make_corpse( victim );
    }

    if ( IS_NPC(victim) )    {
	victim->pIndexData->killed++;
	kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
	extract_char( victim, TRUE );
	return;
    }
    
    victim->limb[0]=0;
    victim->limb[1]=0;
    victim->limb[2]=0;
    victim->limb[3]=0;
    victim->limb[4]=0;
    victim->limb[5]=0;
    extract_char( victim, FALSE );

     if (tattoo !=NULL) {
         obj_to_char(tattoo,victim);
         equip_char(victim,tattoo,WEAR_TATTOO);
     }

     if (victim->race_orig != 0 
     && IS_AFFECTED(victim, AFF_POLY)) {
          REMOVE_BIT( victim->affected_by, AFF_POLY );
          free_string(victim->description);
          free_string(victim->short_descr);
          free_string(victim->long_descr);
          free_string(victim->poly_name);
           
          victim->description = str_dup (victim->description_orig);
          victim->short_descr = str_dup (victim->short_descr_orig); 
          victim->long_descr  = str_dup (victim->long_descr_orig);
          victim->poly_name	= str_dup ("");
          victim->race = victim->race_orig;
          send_to_char( "Your change back to your normal form!\r\n", victim );
          act("$n changes back to his normal form.",victim,NULL,NULL,TO_ROOM);
          victim->race_orig = 0;
          if (is_affected(victim, gafn_mask_hide)) affect_strip_afn(victim, gafn_mask_hide);
      }

    if (!IS_NPC(victim)
    && IS_IMMORTAL(victim)) {
      int rn;
      rn = get_cult_rn(victim->name);
      if (rn > 0) cult_array[rn].power = 0;
    }

    REMOVE_BIT( victim->form, FORM_BLEEDING );
    while ( victim->affected ) {
	affect_remove( victim, victim->affected );
    }

    victim->affected_by	= race_array[victim->race].aff;

    for (i = 0; i < 4; i++) {
    	victim->armor[i]= 100;
    }

    set_activity( victim, POS_RESTING, NULL, ACV_NONE, NULL);
    victim->hit		= UMAX( (short)(victim->max_hit/2),  victim->hit  );
    victim->mana	= UMAX( (short)(victim->max_mana/2), victim->mana );
    victim->move	= UMAX( (short)(victim->max_move/2), victim->move );

    REMOVE_BIT(victim->plr, PLR_BOUGHT_PET);

    victim->recall_temp = 0;
    save_char_obj( victim );

    if ( IS_SET(victim->plr, PLR_PERMADEATH) 
    || IS_SET(mud.flags, MUD_PERMADEATH) ) {
      delete_char(victim);
    }
    return;
}


/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim, bool dual ) {
    OBJ_DATA *obj;
    char messbuf[128];
    
	if (!dual)	{
      	    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL ) return;
	} else {
    	    if ( ( obj = get_eq_char( victim, WEAR_WIELD2 ) ) == NULL ) return;
	}

    if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE)
    || IS_OBJ_STAT(obj,ITEM_NODISARM))    {
	act("$S weapon won't budge!",ch,NULL,victim,TO_CHAR);
	act("$n tries to disarm you, but your weapon won't budge!",ch,NULL,victim,TO_VICT);
	act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
	return;
    }

    if (get_skill(victim,gsn_strong_grip) > number_percent()) {
	act("$S holds on to his weapon!",ch,NULL,victim,TO_CHAR);
	act("$n tries to disarm you, but hold on to your weapon!", ch,NULL,victim,TO_VICT);
	act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
	return;
    }

    sprintf (messbuf, "$n disarms you and sends %s flying!", obj->short_descr);
    act( messbuf, ch, NULL, victim, TO_VICT    );
    sprintf (messbuf, "You disarm $N! %s goes flying!", obj->short_descr); 
    act( messbuf,  ch, NULL, victim, TO_CHAR    );
    act( "$n disarms $N!",  ch, NULL, victim, TO_NOTVICT );

    obj_from_char( obj );
    if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY)) obj_to_char( obj, victim );
    else    {

              if (IS_SET(obj->extra_flags, ITEM_MELT_DROP)) {
                     extract_obj(obj);
              } else {
	     obj_to_room( obj, victim->in_room );
	     if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim,obj)) get_obj(victim,obj,NULL);
              }
    }

    return;
}

void do_berserk( CHAR_DATA *ch, char *argument)
{
    int chance, hp_percent;

    if (!isSkillAvailable(ch, gsn_berserk)) {
	send_to_char("You turn red in the face, but nothing happens.\r\n",ch);
	return;
    }

    if ( IS_NPC(ch)
      && !IS_SET(ch->off_flags,OFF_BERSERK) ) {
	send_to_char("You turn red in the face, but nothing happens.\r\n",ch);
	return;
    }

    chance = get_skill(ch,gsn_berserk);

    if (  chance == 0 ) {
	send_to_char("Oh-oh. Feels like you pulled something...\r\n",ch);
	return;
    }

    if ( IS_AFFECTED(ch, AFF_BERSERK) 
      || is_affected(ch, gafn_berserk)
      || is_affected(ch, gafn_frenzy))
    {
	send_to_char("You get a little madder.\r\n",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CALM))
    {
	send_to_char("You're feeling to mellow to berserk.\r\n",ch);
	return;
    }

    if (ch->mana < 50)
    {
	send_to_char("You can't get up enough energy.\r\n",ch);
	return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
	chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit/ch->max_hit;
    chance += 25 - hp_percent/2;

    if (number_percent() < chance)
    {
	AFFECT_DATA af;

	WAIT_STATE(ch,PULSE_VIOLENCE);
	ch->mana -= 50;
	ch->move /= 2;

	/* heal a little damage */
	ch->hit += ch->level * 2;
	ch->hit = UMIN(ch->hit,ch->max_hit);

	send_to_char("{RYour pulse races as you are consumned by rage!{x\r\n",ch);
	act("{R$n gets a wild look in $s eyes.{x",ch,NULL,NULL,TO_ROOM);
	check_improve(ch,gsn_berserk,TRUE,2);

	af.type		= gsn_berserk;
        af.afn		= gafn_berserk;
	af.level	= ch->level;
	af.duration	= number_fuzzy(ch->level / 8);
	af.modifier	= UMAX(1,ch->level/5);
	af.bitvector 	= AFF_BERSERK;

	af.location	= APPLY_HITROLL;
	affect_to_char(ch,&af);

	af.location	= APPLY_DAMROLL;
	affect_to_char(ch,&af);

	af.modifier	= UMAX(10,10 * (ch->level/5));
	af.location	= APPLY_AC;
	affect_to_char(ch,&af);
    }

    else
    {
	WAIT_STATE(ch,3 * PULSE_VIOLENCE);
	ch->mana -= 25;
	ch->move /= 2;

	send_to_char("Your pulse speeds up, but nothing happens.\r\n",ch);
	check_improve(ch,gsn_berserk,FALSE,2);
    }
}

/* Locate a suitable opponent in the same room, preferable the guy you
   are fighting... */

CHAR_DATA *find_opponent(CHAR_DATA *ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *room;
    ROOM_INDEX_DATA *outroom;
    OBJ_DATA *weapon;
    bool wpok;

   /* Those who are afraid, do not fight... */

    if (IS_AFFECTED(ch, AFF_FEAR)) {
	if (!IS_NPC(ch)) send_to_char ("You are too scared to kill anyone...\r\n",ch);
	return NULL;
    }	

    if (IS_AFFECTED(ch, AFF_MIST)) {
	if (!IS_NPC(ch)) send_to_char ("You can't attack in this form...\r\n",ch);
	return NULL;
    }	
    
   /* Get their name... */

    one_argument( argument, arg );

   /* No args means you keep fighting you opponent... */

    if ( arg[0] == '\0' ) return ch->fighting;
    
   /* Locate them... */

    victim = get_char_room( ch, arg );

    if (victim == NULL
    && IS_SET(ch->in_room->room_flags, ROOM_VEHICLE)
    && ch->in_room->exit[DIR_OUT] != NULL) {
           wpok = FALSE;
           if ((weapon = get_eq_char(ch,WEAR_WIELD)) != NULL) {
                 if (weapon->value[0] == WEAPON_HANDGUN
                 || weapon->value[0] == WEAPON_GUN
                 || weapon->value[0] == WEAPON_SMG
                 || weapon->value[0] == WEAPON_BOW) wpok = TRUE;
           }
           if ((weapon = get_eq_char(ch,WEAR_WIELD2)) != NULL) {
                 if (weapon->value[0] == WEAPON_HANDGUN
                 || weapon->value[0] == WEAPON_GUN
                 || weapon->value[0] == WEAPON_SMG
                 || weapon->value[0] == WEAPON_BOW) wpok = TRUE;
           }
           if (!wpok) {
	send_to_char( "No one by that name within reach...\r\n", ch );
	return NULL;
           }
        
           room = ch->in_room;
           outroom = ch->in_room->exit[DIR_OUT]->u1.to_room;
           char_from_room(ch);
           char_to_room(ch, outroom);
           victim = get_char_room( ch, arg );
           char_from_room(ch);
           char_to_room(ch, room);
    }

    if ( victim == NULL ) {
	send_to_char( "No one by that name here...\r\n", ch );
	return NULL;
    }

   /* Can't do it to yourself... */

    if ( victim == ch ) return NULL;

   /* Can't do it to safe characters... */ 

    if (is_safe( ch, victim ) ) 	return NULL;

   /* Already fighting? */

    if (ch->fighting != NULL) {
      if (victim->fighting == ch) {
        return victim;
      } else {
        return ch->fighting;
      } 
    }  

   /* No kill stealing... */

    if ( ( victim->fighting != NULL 
        && !is_same_group(ch,victim->fighting) ) 
      && ( !IS_SET(ch->in_room->room_flags, ROOM_PKILL) ) ) {
        send_to_char("Kill stealing is not permitted.\r\n",ch);
        return NULL;
    }

   /* Can't attack your master if you are charmed... */

    if ( IS_AFFECTED(ch, AFF_CHARM) 
      && ch->master == victim ) {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return NULL;
    }

   /* Shouldn't ever get here... */ 

    if ( ch->position == POS_FIGHTING ) {
	send_to_char( "You do the best you can!\r\n", ch );
	return ch->fighting;
    }

    return victim;
}


void do_bash( CHAR_DATA *ch, char *argument ) {
CHAR_DATA *victim;
int chance;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

    if ( ! ( has_skill(ch, gsn_bash) 
          || ( IS_NPC(ch)  
            && IS_SET(ch->off_flags,OFF_BASH)))) {
    	
	send_to_char("You don't know how to bash someone...\r\n",ch);
	return;
    }

    victim = find_opponent(ch, argument); 

    if (victim == NULL) {
	send_to_char("They aren't here.\r\n",ch);
	return;
    }

    if (victim->position < POS_FIGHTING)  {
	act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
	return;
    } 

   /* Attack event... */

    mcc = get_mcc( ch, ch, victim, NULL, NULL, NULL, 0, NULL);
    wev = get_wev( WEV_ATTACK, WEV_ATTACK_BASH, mcc,
                  "{rYou try to bash @v2!{x\r\n", 
                  "{r@a2 tries to bash you!{x\r\n",
                  "{r@a2 tries to bash @v2!{x\r\n");

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    free_wev(wev);

   /* modifiers */

    chance = 0;

   /* size  and weight */

    chance += ch->carry_weight / 25;
    chance -= victim->carry_weight / 20;

    if (get_char_size(ch) < get_char_size(victim))
	chance += (get_char_size(ch) - get_char_size(victim)) * 25;
    else
	chance += (get_char_size(ch) - get_char_size(victim)) * 10; 

   /* stats */

    chance += (get_curr_stat(ch,STAT_STR) / 4);
    chance -= (get_curr_stat(victim,STAT_DEX)) / 3;

   /* speed */

    if (IS_SET(ch->off_flags,OFF_FAST))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST))
	chance -= 20;

   /* level */

    chance += ch->level - victim->level;

   /* now the attack */

    one_hit(ch, victim, gsn_bash, FALSE, gsn_bash, -chance);

    return;
}


void do_crush( CHAR_DATA *ch, char *argument ) {
CHAR_DATA *victim;
int chance;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

    if ( ! ( has_skill(ch, gsn_crush) 
          || ( IS_NPC(ch)  
            && IS_SET(ch->off_flags, OFF_CRUSH)))) {
    	
	send_to_char("You don't know how to crush someone...\r\n",ch);
	return;
    }

    victim = find_opponent(ch, argument); 

    if (victim == NULL) {
	send_to_char("They aren't here.\r\n",ch);
	return;
    }

    if (victim->position < POS_FIGHTING)  {
	act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
	return;
    } 

   /* Attack event... */

    mcc = get_mcc( ch, ch, victim,
                   NULL, NULL, NULL, 0, NULL);

    wev = get_wev( WEV_ATTACK, WEV_ATTACK_CRUSH, mcc,
                  "{rYou try to crush @v2!{x\r\n", 
                  "{r@a2 tries to crush you!{x\r\n",
                  "{r@a2 tries to crush @v2!{x\r\n");

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    free_wev(wev);

   /* modifiers */

    chance = 0;

   /* size  and weight */

    chance += ch->carry_weight / 25;
    chance -= victim->carry_weight / 20;

    if (get_char_size(victim) > get_char_size(ch)) chance -= (get_char_size(victim) - get_char_size(ch)) * 50;
    else chance += (get_char_size(ch) - get_char_size(victim)) * 25; 

   /* stats */

    chance += (get_curr_stat(ch,STAT_STR) / 4);
    chance -= (get_curr_stat(victim,STAT_DEX)) / 3;

   /* speed */

    if (IS_SET(ch->off_flags,OFF_FAST))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST))
	chance -= 20;

   /* level */

    chance += ch->level - victim->level;

   /* now the attack */

    one_hit(ch, victim, gsn_crush, FALSE, gsn_crush, -chance);

    return;
}

void do_tail( CHAR_DATA *ch, char *argument ) {
    CHAR_DATA *victim;
    int chance;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

    if ( !IS_SET(ch->parts, PART_LONG_TAIL) ) {
      send_to_char("You don't have a long tail!\r\n", ch);
      return;
    }

    if ( ! ( has_skill(ch, gsn_tail) 
          || ( IS_NPC(ch)  
            && IS_SET(ch->off_flags,OFF_TAIL)))) {
    	
	send_to_char("You don't know how to tail someone...\r\n",ch);
	return;
    }

    victim = find_opponent(ch, argument); 

    if (victim == NULL) {
	send_to_char("They aren't here.\r\n",ch);
	return;
    }

    if (victim->position < POS_FIGHTING) {
	act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
	return;
    } 

   /* Attack event... */

    mcc = get_mcc( ch, ch, victim,  NULL, NULL, NULL, 0, NULL);
    wev = get_wev( WEV_ATTACK, WEV_ATTACK_TAIL, mcc,
                  "{rYou try to hit @v2 with your tail!{x\r\n", 
                  "{r@a2 tries to hit you with thier tail!{x\r\n",
                  "{r@a2 tries to hit @v2 with their tail!{x\r\n");

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    free_wev(wev);

   /* modifiers */

    chance = 0;

   /* size  and weight */

    chance += ch->carry_weight / 25;
    chance -= victim->carry_weight / 20;

    if (get_char_size(victim) > get_char_size(ch)) chance -= (get_char_size(victim) - get_char_size(ch)) * 50;
    else chance += (get_char_size(ch) - get_char_size(victim)) * 20; 

   /* stats */

    chance += (get_curr_stat(ch,STAT_STR) / 4);
    chance -= (get_curr_stat(victim,STAT_DEX)) / 3;

   /* speed */

    if (IS_SET(ch->off_flags,OFF_FAST))
	chance += 10;

    if (IS_SET(victim->off_flags,OFF_FAST))
	chance -= 20;

   /* level */

    chance += ch->level - victim->level;

   /* now the attack */

    one_hit(ch, victim, gsn_tail, FALSE, gsn_tail, -chance);

    return;
}

void do_dirt( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int chance;

    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

   /* Can we kick? */

    if ( ! ( has_skill(ch, gsn_dirt)
        || (  IS_NPC(ch) 
          && !IS_SET(ch->off_flags, OFF_KICK_DIRT)) )) {
	send_to_char("You get your feet dirty.\r\n",ch);
	return;
    }

   /* Who are we kicking? */

    victim = find_opponent(ch, argument);

    if ( ch->fighting == NULL ) {
      if ( victim == NULL ) {
  	send_to_char( "You aren't fighting anyone.\r\n", ch );
	return;
      }
    } else {
      if (victim != ch->fighting) {
        send_to_char("You are already fighting someone else!\r\n", ch);
      }
    } 

   /* Check we do have a victim... */

    if ( victim == NULL ) {
      send_to_char("You try to kick dirt at air!\r\n", ch);
      return; 
    }

   /* No point if they are already blind... */

    if (IS_AFFECTED(victim,AFF_BLIND)) {
	act("$e's already been blinded.",ch,NULL,victim,TO_CHAR);
	return;
    }

   /* Not wise to blind yourself either... */

    if (victim == ch) {
	send_to_char("Very funny.\r\n",ch);
	return;
    }

   /* Attack event... */

    mcc = get_mcc( ch, ch, victim,
                   NULL, NULL, NULL, 0, NULL);

    wev = get_wev( WEV_ATTACK, WEV_ATTACK_DIRT, mcc,
                  "{rYou kick dirt at @v2!{x\r\n", 
                  "{r@a2 kicks dirt at you!{x\r\n",
                  "{r@a2 kicks dirt at @v2!{x\r\n");

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

//    room_issue_wev(ch->in_room, wev);

    free_wev(wev);

   /* modifiers */

    chance = 0;

   /* dexterity */

    chance += (get_curr_stat(ch,STAT_DEX)/4);
    chance -= (get_curr_stat(victim,STAT_DEX)/2);

   /* speed  */

    if ( IS_SET(ch->off_flags,OFF_FAST) 
      || IS_AFFECTED(ch,AFF_HASTE)) {
	chance += 10;
    }

    if ( IS_SET(victim->off_flags,OFF_FAST) 
      || IS_AFFECTED(victim,AFF_HASTE)) {
	chance -= 25;
    }

   /* level */

    chance += ch->level - victim->level;

   /* terrain */

    switch(ch->in_room->sector_type) {

	case(SECT_UNDERWATER):	
	case(SECT_SPACE):	
	case(SECT_WATER_SWIM):	
	case(SECT_WATER_NOSWIM):
	case(SECT_AIR):			
          send_to_char("There is no dirt here.", ch);
          return;
          break;

	case(SECT_INSIDE):		
          chance -= 20;	
          break;

	case(SECT_CITY):
	case(SECT_MOUNTAIN):
          chance -= 10;	
          break;

	case(SECT_FIELD):
	case(SECT_MOORS):
          chance +=  5;	
          break;

	case(SECT_DESERT):		
          chance += 10;   
          break;

        default:
          break;
    }

   /* now the attack */

    one_hit(ch, victim, gsn_dirt, FALSE, gsn_dirt, -chance);
    return;
}


void do_trip( CHAR_DATA *ch, char *argument ) {
    CHAR_DATA *victim;
    int chance;

    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

   /* Can we trip? */

    if ( ! ( has_skill(ch, gsn_trip)
          || ( IS_NPC(ch) 
            && IS_SET(ch->off_flags,OFF_TRIP)))) {
	send_to_char("You don't know how to do that.\r\n",ch);
	return;
    }

   /* Who are we tripping? */ 

    victim = find_opponent(ch, argument);

    if ( ch->fighting == NULL ) {
      if ( victim == NULL ) {
  	send_to_char( "You aren't fighting anyone.\r\n", ch );
	return;
      }
    } else {
      if (victim != ch->fighting) {
        send_to_char("You are already fighting someone else!\r\n", ch);
      }
    } 

   /* Check we do have a victim... */

    if ( victim == NULL ) {
      send_to_char("You try to trip thin air!\r\n", ch);
      return; 
    }

    if (IS_AFFECTED(victim, AFF_FLYING)) {
      send_to_char("But they're flying!\r\n", ch);
      return; 
    }

   /* Attack event... */

    mcc = get_mcc( ch, ch, victim, NULL, NULL, NULL, 0, NULL);

    wev = get_wev( WEV_ATTACK, WEV_ATTACK_TRIP, mcc,
                  "{rYou try to trip @v2!{x\r\n", 
                  "{r@a2 tries to trip you!{x\r\n",
                  "{r@a2 tries to trip @v2!{x\r\n");

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    free_wev(wev);

   /* modifiers */

    chance = 0;

   /* size */
    if (get_char_size(ch) < get_char_size(victim)) chance += (get_char_size(ch) - get_char_size(victim)) * 10;

   /* dex */
    chance += (get_curr_stat(ch,STAT_DEX)/4);
    chance -= (get_curr_stat(victim,STAT_DEX) * 3) / 8;

   /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 20;

   /* level */
    chance += ch->level - victim->level;

   /* now the attack */

    one_hit(ch, victim, gsn_trip, FALSE, gsn_trip, -chance);

    return; 
}


void do_distract(CHAR_DATA *ch, char *argument ) {
CHAR_DATA *victim;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

    if ( !isSkillAvailable(ch, gsn_tactics)) {
	send_to_char("You don't know how to do that...\r\n", ch );
	return;
    }

    victim = find_opponent(ch, argument);

    if ( ch->fighting == NULL ) {
  	send_to_char( "You aren't fighting anyone.\r\n", ch );
	return;
    }
    if (!victim) victim = ch->fighting;

    if (victim->distractedBy == ch) {
  	sprintf_to_char(ch, "%s is already focussed on you.\r\n", victim->short_descr);
	return;
    }

    if (get_skill(ch, gsn_tactics) > number_percent()) {
  	sprintf_to_char(ch, "You fail to attract %ss attention.\r\n", victim->short_descr);
	return;
    }

    mcc = get_mcc( ch, ch, victim, NULL, NULL, NULL, 0, NULL);
    wev = get_wev( WEV_ATTACK, WEV_ATTACK_DISTRACT, mcc,
                  "{yYou try to distract @v2!{x\r\n", 
                  "{rFighting @a2 takes all you attantion!{x\r\n",
                  "@a2 tries to distract @v2!\r\n");

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    room_issue_wev( ch->in_room, wev);

    free_wev(wev);

    WAIT_STATE( ch, skill_array[gsn_tactics].beats );
    victim->distractedBy = ch;
    return;
}


void do_kick( CHAR_DATA *ch, char *argument ) {
    CHAR_DATA *victim;

    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

   /* Can we kick? */ 

    if ( !isSkillAvailable(ch, gsn_kick)) {
	send_to_char(
	    "You don't know how to do that...\r\n", ch );
	return;
    }

    if (  IS_NPC(ch) 
      && !IS_SET(ch->off_flags, OFF_KICK))
	return;

   /* Who are we kicking? */

    victim = find_opponent(ch, argument);

    if ( ch->fighting == NULL ) {
      if ( victim == NULL ) {
  	send_to_char( "You aren't fighting anyone.\r\n", ch );
	return;
      }
    } else {
      if (victim != ch->fighting) {
        send_to_char("You are already fighting someone else!\r\n", ch);
      }
    } 

   /* Check we do have a victim... */

    if ( victim == NULL ) {
      send_to_char("You try to kick thin air!\r\n", ch);
      return; 
    }

   /* Ok, go for it... */ 

   /* Attack event... */

    mcc = get_mcc( ch, ch, victim, NULL, NULL, NULL, 0, NULL);

    wev = get_wev( WEV_ATTACK, WEV_ATTACK_KICK, mcc,
                  "{rYou try to kick @v2!{x\r\n", 
                  "{r@a2 tries to kick you!{x\r\n",
                  "{r@a2 tries to kick @v2!{x\r\n");

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    free_wev(wev);

    WAIT_STATE( ch, skill_array[gsn_kick].beats );
                if (IS_SET(ch->plr, PLR_AUTOKILL)) {
                     one_hit(ch, victim, gsn_kick, FALSE, gsn_kick, 0);
                } else {
                      SET_BIT(ch->plr, PLR_AUTOKILL);
                     one_hit(ch, victim, gsn_kick, FALSE, gsn_kick, 0);
                      REMOVE_BIT(ch->plr, PLR_AUTOKILL);
                }
    return;
}


void do_strangle( CHAR_DATA *ch, char *argument ){
    CHAR_DATA *victim;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

   /* Can we kick? */ 

    if ( !isSkillAvailable(ch, gsn_strangle)) {
	send_to_char(
	    "You don't know how to do that...\r\n", ch );
	return;
    }
    
    if (IS_NPC(ch)) {
        return;
    }

   /* Who are we kicking? */

    victim = find_opponent(ch, argument);

    if ( ch->fighting == NULL ) {
      if ( victim == NULL ) {
  	send_to_char( "You aren't fighting anyone.\r\n", ch );
	return;
      }
    } else {
      if (victim != ch->fighting) {
        send_to_char("You are already fighting someone else!\r\n", ch);
      }
    } 

   /* Check we do have a victim... */

    if ( victim == NULL ) {
      send_to_char("You try to strangle whom!\r\n", ch);
      return; 
    }

   /* Ok, go for it... */ 

   /* Attack event... */

    mcc = get_mcc( ch, ch, victim,
                   NULL, NULL, NULL, 0, NULL);

    wev = get_wev( WEV_ATTACK, WEV_ATTACK_KICK, mcc,
                  "{rYou try to strangle @v2!{x\r\n", 
                  "{r@a2 tries to strangle you!{x\r\n",
                  "{r@a2 tries to strangle @v2!{x\r\n");

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    free_wev(wev);

    WAIT_STATE( ch, skill_array[gsn_strangle].beats );

     if  (IS_SET(ch->plr, PLR_AUTOKILL)) {
         REMOVE_BIT(ch->plr, PLR_AUTOKILL) ;
        one_hit(ch, victim, gsn_strangle, FALSE, gsn_strangle, 0);
        SET_BIT(ch->plr, PLR_AUTOKILL);
     } else {
        one_hit(ch, victim, gsn_strangle, FALSE, gsn_strangle, 0);
     }

    return;
}


void do_bite( CHAR_DATA *ch, char *argument ) {
    CHAR_DATA *victim;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;
    int old_dam_type;

    if (!IS_SET(ch->parts, PART_FANGS)) {
  	send_to_char( "You doubt your teeth can do too much damage..\r\n", ch );
	return;
    }

   /* Who are we kicking? */
    victim = find_opponent(ch, argument);

    if (ch->fighting == NULL ) {
      if (victim == NULL ) {
  	send_to_char( "You aren't fighting anyone.\r\n", ch );
	return;
      }
    } else {
      if (victim != ch->fighting) {
        send_to_char("You are already fighting someone else!\r\n", ch);
      }
    } 

   /* Check we do have a victim... */

    if ( victim == NULL ) {
      send_to_char("You try to bite thin air!\r\n", ch);
      return; 
    }

   /* Ok, go for it... */ 

   /* Attack event... */

    mcc = get_mcc( ch, ch, victim, NULL, NULL, NULL, 0, NULL);
    wev = get_wev( WEV_ATTACK, WEV_ATTACK_KICK, mcc,
                  "{rYou try to bite @v2!{x\r\n", 
                  "{r@a2 tries to bite you!{x\r\n",
                  "{r@a2 tries to bite @v2!{x\r\n");

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    free_wev(wev);
    WAIT_STATE( ch, skill_array[gsn_kick].beats );

    old_dam_type = ch->atk_type;
    ch->atk_type = WDT_BITE;
    if (!IS_SET(ch->plr,PLR_AUTOKILL)) {
        SET_BIT(ch->plr, PLR_AUTOKILL);
        one_hit(ch, victim, TYPE_UNDEFINED, FALSE, gsn_hand_to_hand, 0);
        REMOVE_BIT(ch->plr, PLR_AUTOKILL);
    } else {
        one_hit(ch, victim, TYPE_UNDEFINED, FALSE, gsn_hand_to_hand, 0);
    }
    ch->atk_type = old_dam_type;
    return;
}


void do_kill( CHAR_DATA *ch, char *argument ) {
CHAR_DATA *victim;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

    victim = find_opponent(ch, argument);

    if (victim == NULL) {
      send_to_char("Syntax: kill <mob>\r\n", ch);
      return;
    }

    if (!IS_NPC(ch)) {
         if (!IS_NPC(victim)
         || IS_AFFECTED (victim,AFF_CHARM)
         || IS_SET(victim->act, ACT_PROTECTED)
         || IS_SET(victim->off_flags, ASSIST_GUARD)) {
                send_to_char("For unlawful killing please use MURDER.\r\n", ch);
                return;
         }
     }

     if (IS_SET (ch->act,ACT_PET)) {
         if (IS_SET (victim->act,ACT_PET)
         || !IS_NPC(victim)) {
               send_to_char("Your pet won't begin the combat.\r\n", ch);
               return;
         }
    }

    if ( victim->position <= POS_STUNNED) {
               send_to_char("That would be no opponent.\r\n", ch);
	return;
    }

    if (!str_cmp(race_array[ch->race].name,"machine")
    || IS_SET(ch->form, FORM_MACHINE)) {
               send_to_char("But you're a machine...\r\n", ch);
	return;
    }

    if (!str_cmp(race_array[victim->race].name,"machine")
    || IS_SET(victim->form, FORM_MACHINE)) {
               send_to_char("You can't kill that.\r\n", ch);
	return;
    }

   /* Attack event... */

    mcc = get_mcc( ch, ch, victim, NULL, NULL, NULL, 0, NULL);

    wev = get_wev( WEV_ATTACK, WEV_ATTACK_KILL, mcc,
                  "{RYou try to kill @v2!{x\r\n", 
                  "{R@a2 starts trying to kill you!{x\r\n",
                  "{r@a2 attacks @v2!{x\r\n");

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    room_issue_wev(ch->in_room, wev);

    free_wev(wev);

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );

    check_killer( ch, victim );

    if ( ch->fighting == NULL ) set_fighting(ch, victim);
    
    multi_hit( ch, victim, TYPE_UNDEFINED );
    check_assist(ch,victim);

    return;
}



void do_murde( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to MURDER, spell it out.\r\n", ch );
    return;
}


bool can_murder (CHAR_DATA *ch, CHAR_DATA *victim) {

  if (IS_IMMORTAL(ch)) return TRUE;

  if ( IS_NEWBIE(ch) 
  || IS_NEWBIE(victim))  return FALSE;

  if (IS_AFFECTED(victim, AFF_AURA)) {
         if (victim->level > ch->level - 15) {
               send_to_char("Your are frightened by the majestic aura.\r\n", ch);
               return FALSE;
         }              
  }

  return TRUE;
}

void do_murder( CHAR_DATA *ch, char *argument ){
    CHAR_DATA *victim;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

    victim = find_opponent(ch, argument);

    if (victim == NULL) {
      send_to_char("Syntax: kill <mob>\r\n", ch);
      return;
    }

    if (IS_SET (ch->act,ACT_PET)) {
         if (IS_SET (victim->act,ACT_PET)
         || !IS_NPC(victim)) {
               send_to_char("Your pet won't begin the combat.\r\n", ch);
               return;
         }
    }

  if (IS_AFFECTED(victim, AFF_AURA)) {
         if (victim->level > ch->level - 15) {
               send_to_char("Your are frightened by the majestic aura.\r\n", ch);
               return;
         }              
  }

    if ( victim->position <= POS_STUNNED) {
               send_to_char("That would be no opponent.\r\n", ch);
	return;
    }

    if (!str_cmp(race_array[victim->race].name,"machine")
    || IS_SET(victim->form, FORM_MACHINE)) {
               send_to_char("You can't kill that.\r\n", ch);
	return;
    }

    if (!str_cmp(race_array[ch->race].name,"machine")
    || IS_SET(ch->form, FORM_MACHINE)) {
               send_to_char("But you're a machine...\r\n", ch);
	return;
    }

    if (!IS_NPC(victim)) {
         if (victim->pcdata->pk_timer > 0) {
             sprintf_to_char(ch, "%s is protected against PK at the moment.\r\n", victim->name);
             return;
         }
    }

    if (!IS_NPC(ch)) ch->pcdata->pk_timer = 0;
        
   /* Attack event... */

    mcc = get_mcc( ch, ch, victim, NULL, NULL, NULL, 0, NULL);
    wev = get_wev( WEV_ATTACK, WEV_ATTACK_MURDER, mcc,
                  "{RYou try to murder @v2!{x\r\n", 
                  "{R@a2 starts trying to murder you!{x\r\n",
                  "{r@a2 attacks @v2!{x\r\n");

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    room_issue_wev(ch->in_room, wev);
    free_wev(wev);

    if (!IS_NPC(victim) && !IS_NPC(ch) && !IS_HERO(victim)
    && IS_SET(ch->plr, PLR_AUTOKILL)) {
         int ldif = ch->level -victim->level + 30;

         if (ldif > 50) {
              if (!check_sanity(ch, ldif)) send_to_char("You are horrified by your intentions!\r\n", ch);
         }
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );

    check_killer( ch, victim );
    if ( ch->fighting == NULL ) set_fighting(ch, victim);
    
    multi_hit( ch, victim, TYPE_UNDEFINED );
    check_assist(ch,victim);

    return;
}


void do_backstab( CHAR_DATA *ch, char *argument ) {
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

   /* Who are we stabbing? */

    victim = find_opponent(ch, argument);

    if (victim == NULL) {
      send_to_char("Syntax: backstab <mob>\r\n", ch);
      return;
    }

    if ( ch->fighting != NULL ) {
      send_to_char("You can't do that in a fight!\r\n", ch);
      return;
    } 

    obj = get_eq_char( ch, WEAR_WIELD );

    if ( obj == NULL) {
	send_to_char( "You need to wield a weapon to backstab.\r\n", ch );
	return;
    }

    if ( victim->fighting != NULL ) {
	send_to_char( "You can't backstab a fighting person.\r\n", ch );
	return;
    }

    if ( victim->hit < victim->max_hit 
    && can_see(victim, ch)) {
	act( "$N is hurt and suspicious ... you can't sneak up.", ch, NULL, victim, TO_CHAR );
	return;
    }

   /* Attack event... */

    mcc = get_mcc( ch, ch, victim,
                   NULL, NULL, NULL, 0, NULL);

    wev = get_wev( WEV_ATTACK, WEV_ATTACK_BACKSTAB, mcc,
                  "{RYou try to backstab @v2!{x\r\n", 
                  "{R@a2 tries to backstab you!{x\r\n",
                  "{r@a2 backstabs @v2!{x\r\n");

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    free_wev(wev);

    check_killer( ch, victim );

    WAIT_STATE( ch, skill_array[gsn_backstab].beats );

    if ( !IS_AWAKE(victim)
    ||   IS_NPC(ch)
    ||   check_skill(ch, gsn_backstab, 0))    {
	check_improve(ch, gsn_backstab, TRUE, 1);
                if (IS_SET(ch->plr, PLR_AUTOKILL)) {
                     one_hit(ch, victim, gsn_backstab, FALSE, gsn_backstab, 0);
                } else {
                      SET_BIT(ch->plr, PLR_AUTOKILL);
                     one_hit(ch, victim, gsn_backstab, FALSE, gsn_backstab, 0);
                      REMOVE_BIT(ch->plr, PLR_AUTOKILL);
                }
    }    else   {
	check_improve(ch, gsn_backstab, FALSE, 1);
        act("You try to backstab $N and miss!",  ch, NULL, victim, TO_CHAR); 
        act("$n tried to backstab you and missed!",  ch, NULL, victim, TO_VICT); 
        act("$n tried to backstab $N and missed!",  ch, NULL, victim, TO_NOTVICT); 
    }

    return;
}

void do_assassinate( CHAR_DATA *ch, char *argument ) {
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

   /* Who are we stabbing? */

    victim = find_opponent(ch, argument);

    if (victim == NULL) {
      send_to_char("Syntax: assassinate <mob>\r\n", ch);
      return;
    }

    if ( ch->fighting != NULL ) {
      send_to_char("You can't do that in a fight!\r\n", ch);
      return;
    } 

    obj = get_eq_char( ch, WEAR_WIELD );

    if ( obj == NULL) {
	send_to_char( "You need to wield a weapon to assassinate.\r\n", ch );
	return;
    }

    if ( victim->fighting != NULL ) {
	send_to_char( "You can't assassinate a fighting person.\r\n", ch );
	return;
    }

    if ( victim->hit < victim->max_hit 
    && can_see(victim, ch)) {
	act( "$N is hurt and suspicious ... you can't sneak up.",  ch, NULL, victim, TO_CHAR );
	return;
    }

   /* Attack event... */

    mcc = get_mcc( ch, ch, victim,
                   NULL, NULL, NULL, 0, NULL);

    wev = get_wev( WEV_ATTACK, WEV_ATTACK_BACKSTAB, mcc,
                  "{RYou try to assassinate @v2!{x\r\n", 
                  "{R@a2 tries to assassinate you!{x\r\n",
                  "{r@a2 assassinate @v2!{x\r\n");

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    free_wev(wev);

    check_killer( ch, victim );

    WAIT_STATE( ch, skill_array[gsn_assassinate].beats );

    if ( !IS_AWAKE(victim)
    ||   IS_NPC(ch)
    ||   check_skill(ch, gsn_assassinate, 0))  {
	check_improve(ch, gsn_assassinate, TRUE, 1);
                if (IS_SET(ch->plr, PLR_AUTOKILL)) {
                     one_hit(ch, victim, gsn_assassinate, FALSE, gsn_assassinate, 0);
                } else {
                      SET_BIT(ch->plr, PLR_AUTOKILL);
                     one_hit(ch, victim, gsn_assassinate, FALSE, gsn_assassinate, 0);
                      REMOVE_BIT(ch->plr, PLR_AUTOKILL);
                }
    }    else    {
	check_improve(ch, gsn_assassinate, FALSE, 1);
        act("You try to assassinate $N and miss!", 
             ch, NULL, victim, TO_CHAR); 
        act("$n tried to assassinate you and missed!", 
            ch, NULL, victim, TO_VICT); 
        act("$n tried to assassinate $N and missed!", 
            ch, NULL, victim, TO_NOTVICT); 
    }

    return;
}

void do_circle( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
 
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

   /* Who are we circling? */

    victim = find_opponent(ch, argument);

    if ( ch->fighting == NULL ) {
      send_to_char("You can only cicrle in a fight!\r\n", ch);
      return;
    } 

   /* Check we do have a victim... */

    if ( victim == NULL ) {
      send_to_char("You try to circle around thin air!\r\n", ch);
      return; 
    }

    if (victim->fighting != ch) {
      send_to_char("You can only circle people fighting you...\r\n", ch);
      return;
    }

    obj = get_eq_char( ch, WEAR_WIELD );

    if ( obj == NULL)   {
        send_to_char( "You need to wield a weapon to circle.\r\n", ch );
        return;
    }
 
    if ( (!IS_AFFECTED(victim, AFF_BLIND)) 
        && (victim->fighting == ch) ) { 
        send_to_char( "Your foe is watching you too closely, you can't circle.\r\n", ch);
	return;
    }  
 
   /* Attack event... */

    mcc = get_mcc( ch, ch, victim,
                   NULL, NULL, NULL, 0, NULL);

    wev = get_wev( WEV_ATTACK, WEV_ATTACK_CIRCLE, mcc,
                  "{RYou try to circle @v2!{x\r\n", 
                  "{R@a2 tries to circle you!{x\r\n",
                  "{r@a2 circles @v2!{x\r\n");

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    free_wev(wev);

    check_killer( ch, victim );

    WAIT_STATE( ch, skill_array[gsn_circle].beats );

    if ( check_skill(ch, gsn_circle, 0)
    || ( !IS_AWAKE(victim) 
      && check_skill(ch, gsn_circle, 70)))    {
        check_improve(ch,gsn_circle,TRUE,1);
                if (IS_SET(ch->plr, PLR_AUTOKILL)) {
                     one_hit( ch, victim, gsn_circle, FALSE, gsn_circle, 0);
                } else {
                      SET_BIT(ch->plr, PLR_AUTOKILL);
                     one_hit( ch, victim, gsn_circle, FALSE, gsn_circle, 0);
                      REMOVE_BIT(ch->plr, PLR_AUTOKILL);
                }

        WAIT_STATE(ch,skill_array[gsn_circle].beats);
    }  else  {
        check_improve(ch,gsn_circle,FALSE,1);
        act("You try to circle $N but fail!", ch, NULL, victim, TO_CHAR); 
        act("$n tried to circle you and fails!", ch, NULL, victim, TO_VICT); 
        act("$n tried to circle $N and fails!", ch, NULL, victim, TO_NOTVICT); 
        WAIT_STATE(ch,skill_array[gsn_circle ].beats);
    }
 
    return;
}

void do_sharpen (CHAR_DATA *ch, char *argument) {
char target[MAX_INPUT_LENGTH];
int roll;
OBJ_DATA *obj;
int move=175;
int all_sharp=WEAPON_SHARP + WEAPON_VORPAL;
int player_chance;
int skill;

        skill = get_skill(ch, gsn_sharpen);

	if (skill < 1 ) {
		send_to_char ("You have no clue how to sharpen your weapon.\r\n",ch);
		return;
	} 	

	one_argument (argument, target);	
	if ( target[0] == '\0' ) {
		send_to_char( "Sharpen what?\r\n", ch );
		return;
   	}

	if (ch->move < move) {
		send_to_char ("You do not have enough movement.\r\n",ch);
		return;
	}
	
	if (( obj = get_obj_carry (ch, target)) == NULL) {
		send_to_char ("You are not carrying that.\r\n", ch);
		return;
	}

	if (obj->item_type != ITEM_WEAPON) {
		send_to_char ("That is not a weapon.\r\n", ch);	
		return;
	}
	
                if (obj->value[0] == WEAPON_WHIP
                || obj->value[0] == WEAPON_FLAIL
                || obj->value[0] == WEAPON_MACE
                || obj->value[0] == WEAPON_GUN
                || obj->value[0] == WEAPON_HANDGUN
                || obj->value[0] == WEAPON_SMG
                || obj->value[0] == WEAPON_BOW
                || IS_SET(obj->value[4], WEAPON_STUN)) {
		send_to_char ("You can't sharpen that weapon.\r\n", ch);	
		return;
	}

	if (obj->wear_loc != -1) {
		send_to_char ("The weapon must be carried to be sharpened.\r\n",ch);
		return;
	}

	ch->move -= move;
	
	if ((obj->value[4] & all_sharp) != 0 || IS_SET(obj->extra_flags, ITEM_ARTIFACT)) {
		send_to_char ("This weapon is already as sharp as it can be...\r\n",ch);
		return;
	}

	WAIT_STATE(ch, skill_array[gsn_sharpen].beats);
	/*lets see what happens*/
	roll = number_percent();

	/*chance is skill/4 + level/4 + (str-20) + (dex -18)*/
	player_chance = (ch->level/4) 
                      + (skill/4)  
		      +	((get_curr_stat (ch, STAT_STR)/4) - 20) 
		      +	((get_curr_stat (ch, STAT_DEX)/4) - 18) 
                      + 5 ;
	
	if (roll <= 5) {
		send_to_char ("You have dulled the edge beyond repair.  The weapon is worthless!\r\n",ch);
		extract_obj(obj);	
		act("$n tries to sharpen $p and fails miserably.",ch,obj,NULL,TO_ROOM);
		check_improve(ch, gsn_sharpen, FALSE, 1);
		return;
		}
	
	else if (roll <= player_chance) /*success!*/
		{
		int flag_to_add=WEAPON_SHARP;
		char to_ch[128];
		char to_room[128];
		/*check for ultimate vorpal roll*/
		roll = number_percent();
		if (roll > 95) /*vorpal!*/
			{
			flag_to_add=WEAPON_VORPAL;
			sprintf (to_ch, "The gods silently assist you!  You've created a {mvorpal{x weapon!\r\n");
			sprintf (to_room, "$n is aided by powers unseen, and thus creates a {mvorpal{x weapon!");
			}
		else
			{
			sprintf (to_ch, "With utmost care, you sharpen your weapon to a fine edge.\r\n");
			sprintf (to_room, "$n works with great care and makes $s weapon deadlier.");
			}
		
		send_to_char (to_ch, ch);
		act(to_room,ch,obj,NULL,TO_ROOM);
		check_improve(ch, gsn_sharpen, TRUE,1);
	
		obj->value[4]+=flag_to_add; /*add sharp or vorpal*/
		obj->valueorig[4]+=flag_to_add;
		}
	else /*fail*/
		{
		send_to_char ("You fail to improve your weapon.\r\n", ch);
		check_improve(ch, gsn_sharpen, FALSE,1);
		return;
		}
	} /*end do_sharpen*/


void do_rescue( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Rescue whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "What about fleeing instead?\r\n", ch );
	return;
    }

    if ( !is_same_group(ch,victim))
    {
        send_to_char("Kill stealing is not permitted.\r\n",ch);
        return;
    }

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
	send_to_char( "Doesn't need your help!\r\n", ch );
	return;
    }

    if ( ch->fighting == victim )
    {
	send_to_char( "Too late.\r\n", ch );
	return;
    }

    if ( ( fch = victim->fighting ) == NULL )
    {
	send_to_char( "That person is not fighting right now.\r\n", ch );
	return;
    }

    WAIT_STATE( ch, skill_array[gsn_rescue].beats );

    if ( !IS_NPC(ch) && number_percent( ) > get_skill(ch, gsn_rescue) )
    {
	send_to_char( "You fail the rescue.\r\n", ch );
	check_improve(ch,gsn_rescue,FALSE,1);
	return;
    }

    act( "You rescue $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n rescues you!", ch, NULL, victim, TO_VICT    );
    act( "$n rescues $N!",  ch, NULL, victim, TO_NOTVICT );
    check_improve(ch,gsn_rescue,TRUE,1);

    stop_fighting( fch, FALSE );
    stop_fighting( victim, FALSE );

    check_killer( ch, fch );
    set_fighting( ch, fch );
    set_fighting( fch, ch );
    return;
}

void do_rotate ( CHAR_DATA *ch, char *argument)
	{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

    if (ch->fighting == NULL) {
	send_to_char ("You aren't fighting anyone.\r\n",ch);
	return;
    }

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Rotate to fight whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

	if ( victim == ch->fighting ) 
		{
		send_to_char ("You are already fighting that target!.\r\n",ch);
		return;
		}
	
	if (victim->fighting != ch)
		{
		send_to_char ("You must rotate to someone who is already fighting you.\r\n",ch);
		return;
		}
    
        /* Attack event... */

         mcc = get_mcc( ch, ch, victim,
                   NULL, NULL, NULL, 0, NULL);

         wev = get_wev( WEV_ATTACK, WEV_ATTACK_ROTATE, mcc,
                  "{RYou try to rotate to fight @v2!{x\r\n", 
                  "{R@a2 tries to rotate to fight you!{x\r\n",
                  "{r@a2 tries to rotate to fight @v2!{x\r\n");

         if (!room_issue_wev_challange(ch->in_room, wev)) {
           free_wev(wev);
           return;
         }
 
//    room_issue_wev(ch->in_room, wev);

        free_wev(wev);

	WAIT_STATE( ch, skill_array[gsn_rotate].beats );
	chance = get_skill(ch, gsn_rotate);
	/*modifiers*/
	chance = chance + (ch->level - ch->fighting->level) * 5; 
	chance = chance + ((get_curr_stat (ch, STAT_DEX)/4) - 20) * 5;
	if (number_percent() > chance)
		{
		send_to_char ("Rotation failed.\r\n",ch);
		check_improve(ch, gsn_rotate, FALSE, 1);
		return;
		}
	stop_fighting (ch, FALSE);
	set_fighting (ch, victim);	
	send_to_char ("Rotation successful.\r\n",ch);
	check_improve(ch, gsn_rotate, TRUE, 1);
    return;
}


void do_disarm( CHAR_DATA *ch, char *argument ) {
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    OBJ_DATA *obj2;
    OBJ_DATA *chwep;
    OBJ_DATA *chwep2;
    int ch_tot_weap=0;
    int vict_tot_weap=0;
    int chance1,chance2,hth,ch_weapon1,vict_weapon1,ch_vict_weapon1;
    int ch_weapon2,vict_weapon2, ch_vict_weapon2;
    bool got_one=FALSE;

    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

    hth = 0;

    if ((chance1 = get_skill(ch,gsn_disarm)) == 0)    {
	send_to_char( "You don't know how to disarm opponents.\r\n", ch );
	return;
    }

	chance2 = chance1;
	
	if ((victim = ch->fighting) == NULL)	{
		send_to_char ("You aren't fighting anyone.\r\n",ch);
		return;
		}

	/* Zeran - ok, grab all possible weapons and total them up */	
	if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
		vict_tot_weap++;
	if ((obj2 = get_eq_char(victim, WEAR_WIELD2)) != NULL)
		vict_tot_weap++;
	if ((chwep = get_eq_char(ch, WEAR_WIELD)) != NULL)
		ch_tot_weap++;
	if ((chwep2 = get_eq_char(ch, WEAR_WIELD2)) != NULL)
		ch_tot_weap++;

    if ( chwep == NULL
      && chwep2 == NULL 
      &&   ( (hth = get_skill(ch,gsn_hand_to_hand) ) == 0
          || (  IS_NPC(ch) 
            && !IS_SET(ch->off_flags,OFF_DISARM) ) ) )    {
	send_to_char( "You must wield a weapon to disarm.\r\n", ch );
	return;
    }
	
    if ( ( victim = ch->fighting ) == NULL )    {
	send_to_char( "You aren't fighting anyone.\r\n", ch );
	return;
    }

    if (obj == NULL && obj2 == NULL)    {
	send_to_char( "Your opponent is not wielding a weapon.\r\n", ch );
	return;
    }

   /* Attack event... */

    mcc = get_mcc( ch, ch, victim,
                   NULL, NULL, NULL, 0, NULL);

    wev = get_wev( WEV_ATTACK, WEV_ATTACK_DISARM, mcc,
                  "{RYou try to disarm @v2!{x\r\n", 
                  "{R@a2 tries to disarm you!{x\r\n",
                  "{r@a2 tries to disarm @v2!{x\r\n");

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }
 
    free_wev(wev);

    /* find weapon skills */
    ch_weapon1 = get_weapon_skill(ch,get_weapon_sn(ch,FALSE));
    vict_weapon1 = get_weapon_skill(victim,get_weapon_sn(victim,FALSE));
    ch_weapon2 = get_weapon_skill(ch,get_weapon_sn(ch,TRUE));
    vict_weapon2 = get_weapon_skill(victim,get_weapon_sn(victim,TRUE));
    ch_vict_weapon1 = get_weapon_skill(ch,get_weapon_sn(victim,FALSE));
    ch_vict_weapon2 = get_weapon_skill(ch,get_weapon_sn(victim,TRUE));

    /* modifiers */

    /* skill */
    if (chwep == NULL && chwep2 == NULL )	{
	chance1 = chance1 * hth/150;
	chance2 = chance2 * hth/150;
	}     else	{
	chance1 = chance1 * ch_weapon1/100;
	chance2 = chance2 * ch_weapon2/100;
	}

    chance1 += (ch_vict_weapon1/2 - vict_weapon1) / 2; 
    chance2 += (ch_vict_weapon2/2 - vict_weapon2) / 2; 

    /* dex vs. strength */
    chance1 += (get_curr_stat(ch,     STAT_DEX)/4);
    chance1 -= (get_curr_stat(victim, STAT_STR)/2);
    chance2 += (get_curr_stat(ch,     STAT_DEX)/4);
    chance2 -= (get_curr_stat(victim, STAT_STR)/2);

    /* level */
    chance1 += (ch->level - victim->level) * 2;
    chance2 += (ch->level - victim->level) * 2;
	
	/* Zeran - consider 2 weapons against 1, or vice versa */
	if (ch_tot_weap > vict_tot_weap)
		{
		chance1 += (20 * ch_vict_weapon1/100);
		chance2 += (20 * ch_vict_weapon2/100);
		}
	else if (ch_tot_weap < vict_tot_weap)
		{
		chance1 -= (20 * ch_vict_weapon1/100);
		chance2 -= (20 * ch_vict_weapon2/100);
		}
	/*Make second weapon harder to disarm if victim has 2 weapons*/
	if (obj != NULL && obj2 != NULL)
		chance2 = chance2/2;
 
    /* and now the attack */
   	WAIT_STATE( ch, skill_array[gsn_disarm].beats );
	
	if (obj != NULL)	
	{
  	  if (chance1 + number_open() > get_skill_roll(victim, gsn_disarm)-10)  	  {
		disarm( ch, victim, FALSE );
		check_improve(ch,gsn_disarm,TRUE,1);
		got_one=TRUE;
  	  }
	}

	if (obj2 != NULL)
	{
  	  if (chance2 + number_open() > get_skill_roll(victim, gsn_disarm))  	  {
		disarm( ch, victim, TRUE );
		check_improve(ch,gsn_disarm,TRUE,1);
		got_one=TRUE;
  	  }
	}

	if (!got_one)	  	  {
		act("You fail to disarm $N.",ch,NULL,victim,TO_CHAR);
		act("$n tries to disarm you, but fails.",ch,NULL,victim,TO_VICT);
		act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
		check_improve(ch,gsn_disarm,FALSE,1);
  	  }
    return;
}


void do_sla( CHAR_DATA *ch, char *argument ) {
    send_to_char( "If you want to SLAY, spell it out.\r\n", ch );
    return;
}



void do_slay( CHAR_DATA *ch, char *argument ) {
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )    {
	send_to_char( "Slay whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( ch == victim )    {
	send_to_char( "Suicide is a mortal sin.\r\n", ch );
	return;
    }

    if ( !IS_NPC(victim) && victim->level >= get_trust(ch))    {
	send_to_char( "You failed.\r\n", ch );
	return;
    }

    if (IS_AFFECTED(ch, AFF_INCARNATED)) {
        if (ch->mana < victim->level * 20) {
	send_to_char( "Somehow you lack the power to do that.\r\n", ch );
	return;
        }
        ch->mana -= victim->level * 20;
    }

    act( "You slay $M in cold blood!",  ch, NULL, victim, TO_CHAR    );
    act( "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
    act( "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );

    check_perma(ch, victim);

    raw_kill( victim, TRUE );

    return;
}

bool check_material_vuln (OBJ_DATA *obj, CHAR_DATA *victim)
	{
	long vuln;
	
	if (obj == NULL)
		{
		bug ("Null obj passed to check_material_vuln",0);
		return FALSE;
		}
	if (victim == NULL)
		{
		bug ("Null victim passed to check_material_vuln",0);
		return FALSE;
		}
	if (!str_cmp(material_name(obj->material), "unknown"))
		{
		return FALSE;
		}
	vuln = material_vuln (obj->material);
	if (IS_SET(victim->vuln_flags, vuln))
		return TRUE;
	return FALSE;
	}			


void kill_msg (CHAR_DATA *ch) {
              switch( ch->position ) {
                      case POS_MORTAL:
                            act( "$n is mortally wounded, and will die soon, if not aided.",ch, NULL, NULL, TO_ROOM );
                            send_to_char("You are mortally wounded, and will die soon, if not aided.\r\n",ch );
                            break;

                      case POS_INCAP:
                            act( "$n is incapacitated and will slowly die, if not aided.",ch, NULL, NULL, TO_ROOM );
                            send_to_char("You are incapacitated and will slowly die, if not aided.\r\n",ch );
                            break;

                      case POS_STUNNED:
                            act( "$n is stunned, but will probably recover.",ch, NULL, NULL, TO_ROOM );
                            send_to_char("You are stunned, but will probably recover.\r\n",ch );
                            break;

                      case POS_DEAD:
                            act( "{Y$n is DEAD!!{x",ch, 0, 0, TO_ROOM );
                            send_to_char( "{RYou have been KILLED!!{x\r\n\r\n", ch );
                            break;

                       default:
                            break;
                 }
}


void do_style (CHAR_DATA *ch, char *argument)	{
    int skill;

    if (ch->race !=1) {
       send_to_char( "Only Humans can do that.\r\n", ch );
       return;
    }

     if (IS_AFFECTED(ch,AFF_POLY) ) {
       send_to_char( "Not in that form.\r\n", ch );
       return;
    }

    skill = get_skill(ch, gsn_black_belt);
    if (skill<1) {
        send_to_char("You lack the fighting skills!\r\n",ch);
        return;
    }

    if ( argument[0] == '\0' ) { 
        send_to_char("Which style?\r\n",ch);
        send_to_char("iron fist / tiger / crane / snake / drunken buddha / dragon\r\n",ch);
        send_to_char("ninjitsu / touch mastery\r\n",ch);
        return;
    }
    
    if (!str_cmp(argument, "iron fist") ) {
         ch->pcdata->style = STYLE_IRON_FIST;
         send_to_char("You're now fighting in iron fist style.\r\n",ch);

    } else if ( !str_cmp(argument, "tiger") ) {
          if (skill<20) {
               send_to_char("You lack the fighting skills (20% required).\r\n",ch);
               return;
          }
          ch->pcdata->style = STYLE_TIGER;
          send_to_char("You're now fighting in tiger style.\r\n",ch);

    } else if ( !str_cmp(argument, "crane") ) {
          if (skill<20) {
               send_to_char("You lack the fighting skills (20% required).\r\n",ch);
               return;
          }
          ch->pcdata->style = STYLE_CRANE;
          send_to_char("You're now fighting in crane style.\r\n",ch);

     } else if ( !str_cmp(argument, "snake") ) {
          if (skill<40) {
               send_to_char("You lack the fighting skills (40% required).\r\n",ch);
               return;
          }
          ch->pcdata->style = STYLE_SNAKE;
          send_to_char("You're now fighting in snake style.\r\n",ch);

     } else if ( !str_cmp(argument, "drunken buddha") ) {
          if (skill<50) {
               send_to_char("You lack the fighting skills (50% required).\r\n",ch);
               return;
          }
          if ( ch->condition[COND_DRUNK] < 12 ) {
                send_to_char("But you are sober!\r\n",ch);
                return;
          }
          ch->pcdata->style = STYLE_DRUNKEN_BUDDHA;
          send_to_char("You're now fighting in drunken buddha style.\r\n",ch);

     } else if (!str_cmp(argument, "dragon")) {
          if (skill<100) {
               send_to_char("You lack the fighting skills (100% required).\r\n",ch);
               return;
          }
          if (!IS_AFFECTED(ch,AFF_FLYING) ) {
               send_to_char("You are not flying!\r\n",ch);
               return;
          }
          ch->pcdata->style = STYLE_DRAGON;
          send_to_char("You're now fighting in dragon style.\r\n",ch);

     } else if (!str_cmp(argument, "ninjitsu")) {
          if (skill<70) {
               send_to_char("You lack the fighting skills (70% required).\r\n",ch);
               return;
          }
          if (get_skill(ch, gsn_hide) < 85 || get_skill(ch, gsn_sneak) < 85) {
                send_to_char("Your hide & sneak skills are insufficient.\r\n",ch);
               return;
          }

          ch->pcdata->style = STYLE_NINJITSU;
          send_to_char("You're now fighting in ninjitsu style.\r\n",ch);

     } else if (!str_cmp(argument, "touch mastery")) {
          if (skill<125) {
               send_to_char("You lack the fighting skills (125% required).\r\n",ch);
               return;
          }

          ch->pcdata->style = STYLE_TOUCH;
          send_to_char("You're now fighting in touch mastery style.\r\n",ch);

     } else {
          send_to_char("Unknown kung fu style..\r\n",ch);
          send_to_char("iron fist / tiger / crane / snake / drunken buddha / dragon\r\n",ch);
          send_to_char("ninjitsu / touch mastery\r\n",ch);
     }

      return;
}


void do_throw (CHAR_DATA *ch, char *argument)	{
OBJ_DATA *grenade = NULL;
OBJ_DATA *spear = NULL;
OBJ_DATA *portal = NULL;
OBJ_DATA *light = NULL;
ROOM_INDEX_DATA *in_room;
ROOM_INDEX_DATA *to_room;
EXIT_DATA *pexit;
char arg1[MAX_INPUT_LENGTH];
int door;
bool explo = FALSE;

    if ( argument[0] == '\0' ) { 
      send_to_char("Syntax: THROW <light> <direction>\r\n",ch);
      send_to_char("        THROW <direction>\r\n",ch);
      return;
    }

    argument = one_argument(argument, arg1);
    if (argument[0] !='\0') {
         light = get_eq_char(ch, WEAR_LIGHT);
         if (light
         && is_name(arg1, light->name)) {
               one_argument(argument, arg1);
               door=-1;
               if (UPPER(arg1[0])== 'N') door=DIR_NORTH;
               if (UPPER(arg1[0])== 'E') door=DIR_EAST;
               if (UPPER(arg1[0])== 'S') door=DIR_SOUTH;
               if (UPPER(arg1[0])== 'W') door=DIR_WEST;
               if (UPPER(arg1[0])== 'U') door=DIR_UP;
               if (UPPER(arg1[0])== 'D') door=DIR_DOWN;
               if (UPPER(arg1[0])== 'I') door=DIR_IN;
               if (UPPER(arg1[0])== 'O') door=DIR_OUT;

               if ( door==-1 ) { 
                     send_to_char("You don't know where to throw it.\r\n",ch);
                     return;
               }

               pexit = ch->in_room->exit[door];

               if ( IS_SET(light->extra_flags, ITEM_NOREMOVE)) {
	    act( "You can't remove $p.", ch, light, NULL, TO_CHAR );
	    return;
               }

               if ( pexit   == NULL 
               || !exit_visible(ch, pexit)) { 
	   send_to_char( "The light hits the wall.\r\n", ch );
                   unequip_char(ch, light);
                   obj_from_char(light);
	   obj_to_room(light, ch->in_room );
                   return;
               }

               if ( IS_SET(pexit->exit_info, EX_CLOSED)) {
	   send_to_char( "The light hits the wall.\r\n", ch );
                   unequip_char(ch, light);
                   obj_from_char(light);
	   obj_to_room(light, ch->in_room );
                   return;
               }

               to_room = get_exit_destination(ch, ch->in_room, pexit, TRUE);
               unequip_char(ch, light);
               obj_from_char(light);
               obj_to_room(light, to_room );
               act( "You throw $p.", ch, light, NULL, TO_CHAR );
               act( "$n throw $p.", ch, light, NULL, TO_ROOM );
               return;
         }
    }

    in_room = ch->in_room;

     if ((grenade = get_eq_char( ch, WEAR_HOLD )) == NULL 
     && (spear = get_eq_char( ch, WEAR_WIELD)) == NULL) {
        send_to_char ("You're not holding a weapon you can throw.\r\n",ch);
        return;
     }  

     if (grenade != NULL
     && grenade->item_type == ITEM_EXPLOSIVE) {
           explo = TRUE;
     }  

     if (!explo) {
         if (spear == NULL) {
              send_to_char ("You're not holding a weapon you can throw.\r\n",ch);
              return;
         } else {
             if (spear->item_type != ITEM_WEAPON
             || spear->value[0]  != WEAPON_SPEAR) {
                   send_to_char ("You're not holding a weapon you can throw.\r\n",ch);
                   return;
             }  
         }       
     }

     portal = get_obj_here(ch, arg1);
     if (portal != NULL) {
           if (portal->item_type == ITEM_PORTAL) {
                  to_room = get_room_index(portal->value[0]);
                  if (explo) {
                      if ( IS_SET(grenade->extra_flags, ITEM_NOREMOVE)) {
	           act( "You can't remove $p.", ch, grenade, NULL, TO_CHAR );
                           return;
                      }
                      blast_room (ch, grenade, to_room, -1, portal);                
                      extract_obj(grenade);	
                  } else {
                      if ( IS_SET(spear->extra_flags, ITEM_NOREMOVE)) {
	            act( "You can't remove $p.", ch, spear, NULL, TO_CHAR );
	            return;
                      }
                      throw_spear(ch, spear, to_room, -1);                
                      unequip_char(ch, spear);
                      obj_from_char(spear);
                      obj_to_room(spear, to_room );
                      act( "You throw $p.", ch, spear, NULL, TO_CHAR );
                      act( "$n throw $p.", ch, spear, NULL, TO_ROOM );
                  }
                  return;
           }
     }

     door=-1;
     if (UPPER(arg1[0])== 'N') door=DIR_NORTH;
     if (UPPER(arg1[0])== 'E') door=DIR_EAST;
     if (UPPER(arg1[0])== 'S') door=DIR_SOUTH;
     if (UPPER(arg1[0])== 'W') door=DIR_WEST;
     if (UPPER(arg1[0])== 'U') door=DIR_UP;
     if (UPPER(arg1[0])== 'D') door=DIR_DOWN;
     if (UPPER(arg1[0])== 'I') door=DIR_IN;
     if (UPPER(arg1[0])== 'O') door=DIR_OUT;

     if ( door==-1 ) { 
         send_to_char("You don't know where to throw it.\r\n",ch);
         return;
     }

      pexit = in_room->exit[door];

     if (explo) {

       if ( IS_SET(grenade->extra_flags, ITEM_NOREMOVE)) {
	act( "You can't remove $p.", ch, grenade, NULL, TO_CHAR );
	return;
       }

      if ( pexit   == NULL 
      || !exit_visible(ch, pexit)) { 
	send_to_char( "The explosive bounces against the wall.\r\n", ch );
                blast_room (ch, grenade, in_room, door, NULL);                
                extract_obj(grenade);	
                return;
      }

      if ( IS_SET(pexit->exit_info, EX_WALL)
      && IS_SET(pexit->exit_info, EX_CLOSED)) {
	send_to_char( "The explosive bounces against the wall.\r\n", ch );
                blast_door (ch, grenade, pexit, door);                
                extract_obj(grenade);	
                return;
      }

      if ( IS_SET(pexit->exit_info, EX_CLOSED)) {
	send_to_char( "The explosive bounces against the door.\r\n", ch );
                blast_door (ch, grenade, pexit, door);                
                extract_obj(grenade);	
                return;
      }

       to_room = get_exit_destination(ch, in_room, pexit, TRUE);
       blast_room (ch, grenade, to_room, door, NULL);                
       extract_obj(grenade);	
       return;

       } else {

       if ( IS_SET(spear->extra_flags, ITEM_NOREMOVE)) {
	act( "You can't remove $p.", ch, spear, NULL, TO_CHAR );
	return;
       }

      if ( pexit   == NULL 
      || !exit_visible(ch, pexit)) { 
	send_to_char( "The spear hits the wall.\r\n", ch );
                unequip_char(ch, spear);
                obj_from_char(spear);
	obj_to_room(spear, ch->in_room );
                return;
      }

      if ( IS_SET(pexit->exit_info, EX_CLOSED)) {
	send_to_char( "The spear hits the wall.\r\n", ch );
                unequip_char(ch, spear);
                obj_from_char(spear);
	obj_to_room(spear, ch->in_room );
                return;
      }

       to_room = get_exit_destination(ch, in_room, pexit, TRUE);
       throw_spear(ch, spear, to_room, door);                
       unequip_char(ch, spear);
       obj_from_char(spear);
       obj_to_room(spear, to_room );
       act( "You throw $p.", ch, spear, NULL, TO_CHAR );
       act( "$n throw $p.", ch, spear, NULL, TO_ROOM );
       return;
       }      
       
       return;
}


void throw_spear (CHAR_DATA *ch, OBJ_DATA *spear, ROOM_INDEX_DATA *room, int door) {
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
     int damage;

     fch = room->people;
     while (fch != NULL) {
            fch_next = fch->next_in_room;
            if (IS_SET(fch->act, ACT_PRACTICE) 
            || IS_SET(fch->act, ACT_NOPURGE) 
            || IS_SET(fch->act, ACT_IS_ALIENIST) 
            || IS_SET(fch->act, ACT_IS_HEALER)
            || !str_cmp("spec_mechanic", spec_string(fch->spec_fun))) {
               fch = fch_next;            
               continue;
            }

            damage = spear->value[1]*spear->value[2]* 3 *number_percent()/100;
            fch->hit -=damage;
            if (damage > fch->max_hit / 8
            && !IS_SET(fch->act, ACT_UNDEAD))  SET_BIT(fch->form,FORM_BLEEDING);
            act("{RYou get hit by a spear!{x", fch, NULL, NULL,TO_CHAR );
            update_pos( fch );
            kill_msg( fch);
            if (IS_NPC(fch)
            && fch->position >= POS_STUNNED) {
                 SET_BIT(fch->act, ACT_AGGRESSIVE);
                 do_flee( fch, "");
            return;
            }
      }
      return;
}

void blast_door (CHAR_DATA *ch, OBJ_DATA *grenade, EXIT_DATA *pexit, int door) {
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;
     int damage, doorpwr, doordam;

     mcc = get_mcc(ch, ch, NULL, NULL, grenade, NULL, door, dir_name[door]);
     wev = get_wev(WEV_LOCK, WEV_OPROG_EXPLOSION, mcc,
                    "{RBOOOOM!{x\r\n",
                     NULL,
                    "{RBOOOOM!{x\r\n");

     if (!room_issue_wev_challange(ch->in_room, wev)) {
          free_wev(wev);
          return;
     }

     room_issue_door_wev(ch->in_room, wev,"{RBOOOOOM!{x");
     free_wev(wev);
     
     damage = grenade->value[1]*grenade->value[2]/10;
     doorpwr=number_percent();
      if ( IS_SET(pexit->exit_info, EX_LOCKED) ) doorpwr +=10;
      if ( IS_SET(pexit->exit_info, EX_PICKPROOF) ) doorpwr +=5;
      if ( IS_SET(pexit->exit_info, EX_NO_PASS) ) doorpwr +=15;
      if ( IS_SET(pexit->exit_info, EX_ROBUST) ) doorpwr +=35;
      if ( IS_SET(pexit->exit_info, EX_ARMOURED) ) doorpwr +=50;
      if ( IS_SET(pexit->exit_info, EX_WALL) ) doorpwr +=40;

      doordam = damage * 5 / (UMAX(doorpwr * pexit->condition / 100, 5)) + 1;
      pexit->condition -= doordam;
               
      if (pexit->condition <= 0) {
           pexit->condition = 0;
           send_to_char( "{cThe door flys open.{x\r\n", ch );
           set_door_flags(ch->in_room, door, pexit, EX_LOCKED, FALSE);
           set_door_flags(ch->in_room, door, pexit, EX_CLOSED, FALSE);
      }     
      return;
}


void blast_room (CHAR_DATA *ch, OBJ_DATA *grenade, ROOM_INDEX_DATA *room, int door, OBJ_DATA *portal) {
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    OBJ_DATA *fobj;
    OBJ_DATA *fobj_next;
    ROOM_INDEX_DATA *otherroom;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;
     int damage;

     if (door != -1) {
         mcc = get_mcc(ch, ch, NULL, NULL, grenade, NULL, door, dir_name[door]);
         wev = get_wev(WEV_LOCK, WEV_OPROG_EXPLOSION, mcc,
                    "You throw @p1 to the @t0.\r\n",
                     NULL,
                    "@a2 throws @p1 to the @t0.\r\n");
     } else {
         mcc = get_mcc(ch, ch, NULL, NULL, grenade, NULL, door, portal->short_descr);
         wev = get_wev(WEV_LOCK, WEV_OPROG_EXPLOSION, mcc,
                    "You throw @p1 in @t0.\r\n",
                     NULL,
                    "@a2 throws @p1 in @t0.\r\n");
     }

     if (!room_issue_wev_challange(ch->in_room, wev)) {
          free_wev(wev);
          return;
     }

     room_issue_door_wev(ch->in_room, wev,"{RBOOOOOM!{x");
     free_wev(wev);

     for ( fch = room->people; fch != NULL; fch = fch_next )   {
            fch_next = fch->next_in_room;
            if (!IS_SET(fch->act, ACT_PRACTICE) 
            && !IS_SET(fch->act, ACT_NOPURGE) 
            && !IS_SET(fch->act, ACT_IS_ALIENIST) 
            && !IS_SET(fch->act, ACT_IS_HEALER)
            && str_cmp("spec_mechanic", spec_string(fch->spec_fun))) {
                 harmful_effect(fch, grenade->level, TAR_CHAR_OFFENSIVE, DAM_FIRE);
                 damage = grenade->value[1]*grenade->value[2]*number_percent()/100;
                 fch->hit -=damage;
                if (damage > fch->max_hit / 8
                && !IS_SET(fch->act, ACT_UNDEAD))  SET_BIT(fch->form,FORM_BLEEDING);
	act( "{RYou get hit by the blast of the explosion!{x", fch, NULL, NULL,TO_CHAR );
                 update_pos( fch );
                 kill_msg( fch);
                 if (IS_NPC(fch)
                 && fch->position >= POS_STUNNED) {
                     SET_BIT(fch->act, ACT_AGGRESSIVE);
                     do_flee( fch, "");
                 }
            }
     }

     for ( fobj = room->contents; fobj != NULL; fobj = fobj_next )   {
            fobj_next = fobj->next_content;
 
            if (!IS_SET(fobj->extra_flags, ITEM_NO_COND)) {
                 damage = grenade->value[1]*grenade->value[2]*number_percent()/100;
                 if (!IS_SET(fobj->extra_flags, ITEM_MAGIC)) damage/= 2;
                 if (fobj->item_type != ITEM_PORTAL) {
                     damage /= fobj->weight + 1;
                     fobj->condition -= damage;
                     if (fobj->condition <= 0) {
      	          act( "{R$p shatters!{x", NULL, fobj, NULL,TO_ROOM );
                          obj_from_room(fobj);
                          extract_obj(fobj);
                     }
                 } else {
                     if (fobj->value[4] == PORTAL_MIRROR) {
      	          act( "{R$p shatters!{x", NULL, fobj, NULL,TO_ROOM );
                          obj_from_room(fobj);
                          extract_obj(fobj);
                     } else if (fobj->value[4] == PORTAL_VEHICLE) {
                          damage /= 20;
                          fobj->condition -= damage;
                          fobj->pIndexData->condition -= damage;
                          if (fobj->condition <= 0) {
                                otherroom = get_room_index(fobj->value[0]);
                                if (otherroom->exit[DIR_OUT]) {
      	                     free_exit( otherroom->exit[DIR_OUT] );
	                     otherroom->exit[DIR_OUT] = NULL;
                                }
                                REMOVE_BIT(otherroom->room_flags, ROOM_BUILDING);
                                REMOVE_BIT(otherroom->room_flags, ROOM_VEHICLE);
                                SET_BIT( room->area->area_flags, AREA_CHANGED );
                                SET_BIT( otherroom->area->area_flags, AREA_CHANGED );
      	                act( "{R$p explodes!{x", NULL, fobj, NULL,TO_ROOM );
                                obj_from_room(fobj);
                                extract_obj(fobj);
                          }
                     }
                 }
            }
     }

     return;
}


void do_blackjack( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    int dam;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_blackjack)) == 0) {
	send_to_char("You cant blackjack.\r\n",ch);
	return;
    }

    if (ch->fighting != NULL) {
	send_to_char("You are much too busy.\r\n",ch);
	return;
    }

    if ((victim = get_char_room(ch,arg)) == NULL)    {
	send_to_char("They aren't here.\r\n",ch);
	return;
    }

    if (victim->position <= POS_STUNNED) {
	act("$N's already knocked out.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch) {
	send_to_char("That will not work.\r\n",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim){
	act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    if ( IS_NPC(victim) 
      && ( IS_SET(victim->act,ACT_PRACTICE)
      ||  IS_SET(victim->act,ACT_IS_ALIENIST)
      ||  IS_SET(victim->act,ACT_IS_HEALER))) {
	send_to_char("I think Marduk would {rNOT{x approve.\r\n",ch);
	return;
    }

    if ((victim->move < victim->max_move || victim->hit < victim->max_hit) 
    && can_see(victim, ch)) {
	act( "$N is hurt and suspicious ... you can't sneak up.",  ch, NULL, victim, TO_CHAR );
	return;
    }

    chance += (ch->level - victim->level) * 2;

    set_fighting(ch, victim);

    if (chance <= 0) {
        send_to_char("You stumble!.\r\n",ch);
        ch->move -=number_percent();
        multi_hit( victim, ch, TYPE_UNDEFINED );
        WAIT_STATE(ch, skill_array[gsn_blackjack].beats);
        return;
    }

    if (number_percent() < chance) {
	act("$n is knocked out cold!",victim,NULL,NULL,TO_ROOM);
	act("$n wacks you upside the head!",ch,NULL,victim,TO_VICT);
	send_to_char("You are knocked out cold!\r\n",victim);
	check_improve(ch,gsn_blackjack,TRUE,2);
        dam = chance * ch->max_move / 300;
        victim->move -=dam;
     }

     one_hit(ch, victim, gsn_backstab, FALSE, gsn_blackjack, 0);
     WAIT_STATE(ch, skill_array[gsn_blackjack].beats);
     return;
}


void set_bleed (CHAR_DATA *ch) {
int tolimb;

      if (!IS_SET(ch->act, ACT_UNDEAD))  SET_BIT(ch->form,FORM_BLEEDING);
      tolimb=number_range(0,5);
      if (ch->limb[tolimb] >-1 && ch->limb[tolimb]<10) {
          ch->limb[tolimb] +=1;
     }
      if (ch->limb[0] > 9) {
          act("$ns head is chopped off!",ch,NULL,NULL,TO_ROOM);
          send_to_char("Your head is chopped off!\r\n",ch);
          ch->hit =-15;
      }
      if (ch->limb[1] > 9) {
          act("$ns body explodes!",ch,NULL,NULL,TO_ROOM);
          send_to_char("Your body explodes!\r\n",ch);
          ch->hit =-15;
      }
      if (ch->limb[2] > 9) {
          act("$ns left arm is chopped off!",ch,NULL,NULL,TO_ROOM);
          send_to_char("Your left arm is chopped off!\r\n",ch);
          remove_obj (ch, WEAR_SHIELD, TRUE);
          remove_obj (ch, WEAR_HOLD, TRUE);
          remove_obj (ch, WEAR_WIELD2, TRUE);
          remove_obj (ch, WEAR_FINGER_L, TRUE);
      }
      if (ch->limb[3] > 9) {
          act("$ns right arm is chopped off!",ch,NULL,NULL,TO_ROOM);
          send_to_char("Your right arm is chopped off!\r\n",ch);
          remove_obj (ch, WEAR_WIELD, TRUE);
          remove_obj (ch, WEAR_FINGER_R, TRUE);
      }
      if (ch->limb[4] > 9) {
          act("$ns legs are chopped off!",ch,NULL,NULL,TO_ROOM);
          send_to_char("Your legs are chopped off!\r\n",ch);
          ch->move =-15;
      }
      update_pos(ch);
      update_wounds(ch);
      return;
}


void update_wounds(CHAR_DATA *ch) {
    AFFECT_DATA af;

   if (ch->limb[0] > 3
   && !is_affected(ch,gafn_head_wound)) {
      af.type		= SKILL_UNDEFINED;
      af.afn		= gafn_head_wound;
      af.level		= MAX_LEVEL;
      af.duration		= -1;
      af.bitvector		= 0;
      af.location		= APPLY_INT;
      af.modifier		= -1 * ch->limb[0];
      affect_to_char( ch, &af );
      af.location		= APPLY_WIS;
      af.modifier		= -1 * ch->limb[0];
      affect_to_char( ch, &af );
      af.location		= APPLY_MANA;
      af.modifier		= -1 * dice(2 * ch->limb[0], 10);
      affect_to_char( ch, &af );
   }
   if (ch->limb[0] < 4
   && is_affected(ch,gafn_head_wound)) {
      affect_strip_afn(ch, gafn_head_wound);
   }

   if (ch->limb[1] > 4
   && !is_affected(ch,gafn_body_wound)) {
      af.type		= SKILL_UNDEFINED;
      af.afn		= gafn_body_wound;
      af.level		= MAX_LEVEL;
      af.duration		= -1;
      af.bitvector		= 0;
      af.location		= APPLY_STR;
      af.modifier		= -1 * ch->limb[1]/2;
      affect_to_char( ch, &af );
      af.location		= APPLY_CON;
      af.modifier		= -1 * ch->limb[1]/2;
      affect_to_char( ch, &af );
   }
   if (ch->limb[1] < 5
   && is_affected(ch,gafn_body_wound)) {
      affect_strip_afn(ch, gafn_head_wound);
   }

   if ((ch->limb[2] > 4
       || ch->limb[3] >4)
   && !is_affected(ch,gafn_arm_wound)) {
      af.type		= SKILL_UNDEFINED;
      af.afn		= gafn_arm_wound;
      af.level		= MAX_LEVEL;
      af.duration		= -1;
      af.bitvector		= 0;
      af.location		= APPLY_DEX;
      af.modifier		= -1 * (ch->limb[2]+ch->limb[3])/2;
      affect_to_char( ch, &af );
      af.location		= APPLY_MOVE;
      af.modifier		= -1 * dice(ch->limb[2]+ch->limb[3], 10);
      affect_to_char( ch, &af );
   }
   if (ch->limb[2] < 4
   && ch->limb[3] < 4
   && is_affected(ch,gafn_arm_wound)) {
      affect_strip_afn(ch, gafn_arm_wound);
   }

   if (ch->limb[4] > 3
   && !is_affected(ch,gafn_leg_wound)) {
      af.type		= SKILL_UNDEFINED;
      af.afn		= gafn_leg_wound;
      af.level		= MAX_LEVEL;
      af.duration		= -1;
      af.bitvector		= 0;
      af.location		= APPLY_DEX;
      af.modifier		= -1 * ch->limb[4]/2;
      affect_to_char( ch, &af );
      af.location		= APPLY_MOVE;
      af.modifier		= -1 * dice(4 * ch->limb[4], 10);
      affect_to_char( ch, &af );
   }
   if (ch->limb[4] < 4
   && is_affected(ch,gafn_leg_wound)) {
      affect_strip_afn(ch, gafn_leg_wound);
   }

return;
}


bool check_critical( COMBAT_DATA *cd) {
int chance;

    if (cd->atk_weapon) return FALSE;
    if (get_char_size(cd->defender) < SIZE_MEDIUM || get_char_size(cd->defender) > SIZE_LARGE) return FALSE;
    if (get_skill(cd->defender, gsn_black_belt) > 25) return FALSE;

    chance = (get_skill(cd->attacker, gsn_black_belt) - 2*get_skill(cd->defender, gsn_black_belt) + cd->attacker->level - cd->defender->level) /5;
    if (chance > number_percent()) {
          if (number_percent() < 3) return TRUE;
    }
    return FALSE;
}



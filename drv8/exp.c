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
#include "exp.h"
#include "prof.h"
#include "mob.h"
#include "affect.h"
#include "profile.h"
#include "skill.h"
#include "race.h"
#include "gsn.h"

DECLARE_DO_FUN(do_shout);

/* Level advancement... */
 
void advance_level( CHAR_DATA *ch ) {
char buf[MAX_STRING_LENGTH];
int add_hp;
int add_mana;
int add_move;
int add_prac;
int prime_stat;
int prac_stat;

    if (ch->pcdata != NULL) {
      ch->pcdata->last_level = (ch->pcdata->played + (int) (current_time - ch->pcdata->logon) ) / 3600;
    }

   /* Update the players level... */

    ch->level += 1;  

   /* Hp gain: about 15 per level */ 

    add_hp = number_fuzzy(8 + (get_curr_stat(ch, STAT_CON)/8));

   /* Mana gain: about 11 per level */

    add_mana = (get_curr_stat(ch, STAT_INT) + get_curr_stat(ch, STAT_WIS))/16;

    add_mana = number_fuzzy(5 + add_mana);

   /* Move gain: about 15 per level */

    add_move = number_fuzzy(8 + (get_curr_stat(ch,STAT_DEX)/8));

   /* Practice gain: About 3 sessions per level */

    if (ch->profs == NULL) {
      prime_stat = STAT_INT;
    } else {
      prime_stat = ch->profs->profession->prime_stat;
    }

    prac_stat  = get_curr_stat(ch, prime_stat);
    prac_stat += get_curr_stat(ch, STAT_WIS);
    prac_stat /= 2;

    prac_stat = UMAX(prac_stat, 3);

    add_prac	= wis_app[prac_stat].practice;

   /* Check minimum gains... */

    add_hp	= UMAX(  1, add_hp   );
    add_mana	= UMAX(  1, add_mana );
    add_move	= UMAX(  6, add_move );
    add_prac	= UMAX(  2, add_prac );

    if (ch->pcdata != NULL) {
         if (get_age(ch)*100/(ch->pcdata->lifespan+1) < number_percent()+25
         || IS_SET (ch->act, ACT_UNDEAD)
         || IS_ARTIFACTED(ch, ART_BLESSING_NEPHRAN)
         || get_age(ch) < ch->pcdata->lifespan/2) {
               if (ch->level >3) ch->train	 += 1;
         } else {
               add_prac -=2;
               add_hp /=2;
               add_move /=2;
               add_mana = (add_mana * 15)/10;
         }
    } else {
         if (ch->level >3) ch->train += 1;
    }

    ch->max_hit 	+= add_hp;
    ch->max_mana	+= add_mana;
    ch->max_move	+= add_move;
    if (ch->pcdata != NULL) {
        ch->pcdata->perm_hit	+= add_hp;
        ch->pcdata->perm_mana += add_mana;
        ch->pcdata->perm_move += add_move;
     }
     
    ch->practice	+= add_prac;

    if ( !IS_NPC(ch) ) REMOVE_BIT( ch->plr, PLR_BOUGHT_PET );

    send_to_char( "{YYou have gained a LEVEL!!!!!!\r\n{x", ch);
    sprintf( buf, "{WYour gain is: {Y%d/%d hp, %d/%d m, %d/%d mv %d/%d prac.{x\r\n", add_hp,	ch->max_hit, add_mana, ch->max_mana, add_move, ch->max_move, add_prac, ch->practice);
    send_to_char( buf, ch );

    act_new("{Y$n has gained a level!{x", ch, NULL, NULL, TO_ROOM, POS_RESTING);

   /* Adjust divinity... */

    if (!IS_NPC(ch)) {
        if (ch->level == DIV_LEV_HUMAN) {
            REMOVE_BIT(ch->pcdata->divinity, DIV_NEWBIE);
            SET_BIT(ch->pcdata->divinity, DIV_HUMAN);
            send_to_char("{YYou are no longer a newbie!{x\r\n", ch);
        }  

        if (ch->level == DIV_LEV_INVESTIGATOR) {
            REMOVE_BIT(ch->pcdata->divinity, DIV_HUMAN);
            SET_BIT(ch->pcdata->divinity, DIV_INVESTIGATOR);
            send_to_char("{YYou are getting famous!{x\r\n", ch);
        }  

        if (ch->level == DIV_LEV_HERO) {
            REMOVE_BIT(ch->pcdata->divinity, DIV_HUMAN);
            REMOVE_BIT(ch->pcdata->divinity, DIV_INVESTIGATOR);
            SET_BIT(ch->pcdata->divinity, DIV_HERO);
            send_to_char("{YYou are now a great Hero!{x\r\n", ch);
        } 
    }

   /* Also gain a professional level... */

    if (ch->sanity < 128) ch->sanity += dice(2, 4);
    gain_prof_level(ch);
    notify_message (ch, NOTIFY_LEVEL, TO_ALL, NULL);
    return;
}   


/* Going down... */

void drop_level( CHAR_DATA *ch ) {
char buf[MAX_STRING_LENGTH];
int add_mana;
int add_move;
int add_prac;
int add_hp;
int prac_stat;
int prime_stat;

   /* Can't go below level 1 */

    if (ch->level == 1) {
      ch->exp = exp_for_level(1) + number_bits(8);
      return;
    }

   /* Update the players level... */

    ch->level -= 1;  

   /* Hp loss: about 15 per level */ 

    add_hp = number_fuzzy(8 + (get_curr_stat(ch, STAT_CON)/8));

    add_hp = UMAX(add_hp, 8); 

   /* Mana loss: about 11 per level */

    add_mana = (get_curr_stat(ch, STAT_INT) + get_curr_stat(ch, STAT_WIS))/16;

    add_mana = number_fuzzy(5 + add_mana);

    add_mana = UMAX(add_mana, 8);

   /* Move loss: about 15 per level */

    add_move = number_fuzzy(8 + (get_curr_stat(ch,STAT_DEX)/8));

    add_move = UMAX(add_move, 8); 

   /* Practice loss: About 3 sessions per level */

    if (ch->profs == NULL) {
      prime_stat = STAT_INT;
    } else {
      prime_stat = ch->profs->profession->prime_stat;
    }

    prac_stat  = get_curr_stat(ch, prime_stat);
    prac_stat += get_curr_stat(ch, STAT_WIS);
    prac_stat /= 2;

    prac_stat = UMAX(prac_stat, 3);

    add_prac	= wis_app[prac_stat].practice;

    add_hp	= UMAX(  1, add_hp   );
    add_mana	= UMAX(  1, add_mana );
    add_move    = UMAX(  6, add_move );
    add_prac  	= UMAX(  2, add_prac );

   /* start subtracting here */

    ch->max_hit     -= add_hp;
    ch->max_mana    -= add_mana;
    ch->max_move    -= add_move;
    ch->practice    -= add_prac;
    ch->train       -= 1;

    if (ch->pcdata != NULL) {
      ch->pcdata->perm_hit    -= add_hp;
      ch->pcdata->perm_mana   -= add_mana;
      ch->pcdata->perm_move   -= add_move;
    }

    send_to_char( "{RYou have lost a LEVEL!!!!!!\r\n{x", ch);
    sprintf( buf, "{Rour loss is: {Y%d/%d hp, %d/%d m, %d/%d mv %d/%d prac.{x\r\n", add_hp, ch->max_hit, add_mana, ch->max_mana, add_move, ch->max_move, add_prac, ch->practice);
    send_to_char( buf, ch );

    act_new("{R$n has lost a level!{x", ch, NULL, NULL, TO_ROOM, POS_RESTING);

   /* Adjust divinity... */

    if (!IS_NPC(ch)) {
        if (ch->level == DIV_LEV_HUMAN - 1) {
            REMOVE_BIT(ch->pcdata->divinity, DIV_HUMAN);
            SET_BIT(ch->pcdata->divinity, DIV_NEWBIE);
            send_to_char("{YYou are a newbie again!{x\r\n", ch);
        } 

        if (ch->level == DIV_LEV_INVESTIGATOR - 1) {
            REMOVE_BIT(ch->pcdata->divinity, DIV_INVESTIGATOR);
            SET_BIT(ch->pcdata->divinity, DIV_HUMAN);
            send_to_char("{YYour moment of fame is gone!{x\r\n", ch);
        } 

        if (ch->level == DIV_LEV_HERO - 1) {
            REMOVE_BIT(ch->pcdata->divinity, DIV_HERO);
            SET_BIT(ch->pcdata->divinity, DIV_INVESTIGATOR);
            send_to_char("{YYou are no longer a hero!{x\r\n", ch);
        } 
    }

    if (ch->sanity > 1) ch->sanity -= dice(2, 5);
    lose_prof_level(ch);
    return;
}


/* Give an individual some xps... */

void gain_exp( CHAR_DATA *ch, long gain, bool advance) {

   /* There is an upper limit... */

    if (ch->level == MAX_LEVEL 
    || (IS_IMMORTAL(ch) && advance == FALSE)) return;

   /* Bonus for living dangerously... */

    if (!advance && gain > 0) {
        if (IS_NEWBIE(ch)) gain = gain *3 /2;
        if (IS_SET(ch->plr, PLR_ANNOYING)) gain /= 2;
        if (!IS_SET(ch->plr, PLR_PERMADEATH)) gain = gain *5 /6;
    }
   /* Gain the xps... */

    ch->exp += gain;

   /* ...and maybe go up a level... */

    while ( ch->exp >= exp_for_next_level(ch)) {
      advance_level( ch );
      save_char_obj(ch);
    }

   /* Or possibly even down... */

    while ( ch->exp < exp_for_level(ch->level)) {
        bug("Dropping level, char xps: %ld", ch->exp);
        bug("           xps for level: %ld", exp_for_level(ch->level));
        drop_level(ch);
        save_char_obj(ch);
    }

    return;
}

/* Give a group some xps... */

void group_gain( CHAR_DATA *ch, CHAR_DATA *victim ) {
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    OBJ_DATA *weap;
    int xp, transxp;
    int members;
    int group_levels;

    bool found_slayer;

   /* Dying of mortal wounds or poison doesn't give xp to anyone! */
     
    if (  victim == ch ) {
	return;
    }

   /* Count players and levels the same room as the slain monster... */

    members = 0;
    group_levels = 0;
    found_slayer = FALSE;

    for ( gch = victim->in_room->people;
          gch != NULL;
          gch = gch->next_in_room ) {

	if ( is_same_group( gch, ch ) ) {
	    members++;
            if ( IS_SET(mud.flags, MUD_EFFXPS) ) {
	      group_levels += get_effective_level(gch); 
            } else {
	      group_levels += gch->level;
            }
	}

        if (gch == ch) {
          found_slayer = TRUE;
        }
    }

   /* No one there means xps are lost (no xps for remote killing)... */

    if ( members == 0 ) {
	return;
    }

   /* If the slayer hasn't been included, add his levels... */
   /* Stops someone in same room as a remote kill from getting full xps */

    if (!found_slayer) {
      members += 1;
      if ( IS_SET(mud.flags, MUD_EFFXPS) ) {
        group_levels += get_effective_level(ch);
      } else {
        group_levels += ch->level;
      }
    }

   /* Find the group leader... */

    if (ch->leader != NULL) {
      lch = ch->leader;
    } else {
      lch = ch;
    }

   /* Now step through and give rewards... */
   /* For a remote kill, the slayer isn't in the room and gets no xps... */

    for ( gch = victim->in_room->people; 
          gch != NULL; 
          gch = gch->next_in_room ) {

	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if ( !is_same_group( gch, ch ) ) {
	    continue;
        }

	xp = xp_compute(gch, victim, group_levels );  

           if (IS_SET(ch->plr, PLR_QUESTOR) && !IS_NPC(ch)) {
              if (IS_NPC(victim)) {
                 if (ch->pcdata->questmob == victim->pIndexData->vnum
                 && !IS_SET(victim->act, ACT_PROTECTED)) {
                           send_to_char("You have almost completed your mission!\r\n",ch);
                           send_to_char("Return to the questmaster before your time runs out!\r\n",ch);
                           ch->pcdata->questmob = -1;
                 }
              }

              if (!IS_NPC(victim)  && ch->pcdata->questmob == -100) {
                 if (!strcmp(ch->pcdata->questplr, victim->short_descr)) {
                           send_to_char("You have almost completed your mission!\r\n",ch);
                           send_to_char("Return to the questmaster before your time runs out!\r\n",ch);
                           ch->pcdata->questmob = -1;
                 }
              }
           }

       /* Informative message rather than an xp value... */

        if ((weap = get_eq_char(ch, WEAR_WIELD)) != NULL) {
            if (weap->trans_char != NULL) {
                   transxp = xp / 5;
                   xp -= transxp;
                   if (transxp <= 5) {
                          send_to_char("{rYou hardly learnt anything from that fight!{x\r\n", weap->trans_char);
                   } else  if (transxp <= 20 ) { 
                          send_to_char("{yYou did not learn very much from that fight.{x\r\n", weap->trans_char);
                   } else if (transxp <= 75) {
                          send_to_char("{cYou learnt something from that fight.\r\n{x", weap->trans_char);
                   } else {
                          send_to_char("{WWow! You learnt a lot from that fight!{x\r\n", weap->trans_char);
                   }

                   gain_exp(weap->trans_char, xp, FALSE );
            }
        }

        if ((weap = get_eq_char(ch, WEAR_WIELD2)) != NULL) {
            if (weap->trans_char != NULL) {
                   transxp = xp / 5;
                   xp -= transxp;
                   if (transxp <= 5) {
                          send_to_char("{rYou hardly learnt anything from that fight!{x\r\n", weap->trans_char);
                   } else  if (transxp <= 20 ) { 
                          send_to_char("{yYou did not learn very much from that fight.{x\r\n", weap->trans_char);
                   } else if (transxp <= 75) {
                          send_to_char("{cYou learnt something from that fight.\r\n{x", weap->trans_char);
                   } else {
                          send_to_char("{WWow! You learnt a lot from that fight!{x\r\n", weap->trans_char);
                   }

                   gain_exp(weap->trans_char, xp, FALSE );
            }
        }

        if (xp <= 5) {
          send_to_char("{rYou hardly learnt anything from that fight!{x\r\n", gch);
        } else  if (xp <= 20 ) { 
          send_to_char("{yYou did not learn very much from that fight.{x\r\n", gch);
        } else if (xp <= 75) {
          send_to_char("{cYou learnt something from that fight.\r\n{x", gch);
        } else {
          send_to_char("{WWow! You learnt a lot from that fight!{x\r\n", gch);
        }

        if (gch->desc != NULL) {
             if (gch->desc->original != NULL ) {
                    if (!str_cmp(race_array[gch->desc->original->race].name,"yithian")) {
       	             gain_exp( gch, xp/2, FALSE );
                             gain_exp( gch->desc->original, xp/2, FALSE );
                    } else {
                             gain_exp( gch, xp, FALSE );
                    }
             } else {
                    gain_exp( gch, xp, FALSE );
             }
        } else {
             gain_exp( gch, xp, FALSE );
        }

	for ( obj = ch->carrying; obj != NULL; obj = obj_next ) {

	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE ) continue;
            
	    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)))	    {
		act( "You are zapped by $p.", ch, obj, NULL, TO_CHAR );
		act( "$n is zapped by $p.",   ch, obj, NULL, TO_ROOM );
		obj_from_char( obj );
                                if (IS_SET(obj->extra_flags, ITEM_MELT_DROP)) {
		     act( "And as $p hits the ground it melts.", NULL, obj, NULL, TO_ROOM);
                                     extract_obj(obj);
                                } else {
		     obj_to_room( obj, ch->in_room );
                                }
	    }
	}
    }

    return;
}

/* Give a group a reward... */

void group_reward( CHAR_DATA *ch, int xps ) {

    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp;
    int members;
    int group_levels;
    int share;

    members = 0;
    group_levels = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) )
        {
	    members++;
	    group_levels += gch->level;
	}
    }

    if ( members == 0 )
    {
	bug( "Group_gain: members.", members );
	members = 1;
	group_levels = ch->level ;
    }

    lch = (ch->leader != NULL) ? ch->leader : ch;

   /* Work out a share of the xps... */

    share = (1000 * xps)/group_levels;

    if (share < 1) {
      share = 1;
    }

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( !is_same_group( gch, ch )) { 
	    continue;
        } 

	xp = (gch->level * share)/1000; 

       /* Informative message rather than an xp value... */

        if (xp <= 10) {
          send_to_char("{rYou feel unchallanged!{x\r\n", gch);
        } else  if (xp <= 100 ) { 
          send_to_char("{cYou feel a sense of satisfaction.\r\n{x", gch);
        } else if (xp <= 1000) {
          send_to_char("{yYou feel a sense of acheivement.{x\r\n", gch);
        } else {
          send_to_char("{WYou feel a great sense of accomplishment!{x\r\n", gch);
        }
 
	gain_exp( gch, xp, FALSE );

    }

    return;
}

/* Death causes exp loss... */

void lose_exp_for_dieing(CHAR_DATA *ch) {
int loss =0;
   /* PCs only... */

    if (IS_NPC(ch)) return;
    
   /* Less than level 15, drop half points at this level... */

    if ( ch->level < 15 ) {
        if ( ch->exp > exp_for_level(ch->level) ) loss = (ch->exp - exp_for_level(ch->level)) / 2;

        if (ch->desc != NULL) {
             if (ch->desc->original != NULL ) {
                    if (!str_cmp(race_array[ch->desc->original->race].name,"yithian")) {
                             gain_exp( ch->desc->original, -1 * (loss/2), FALSE);
                    } else {
                             gain_exp( ch, -1 * (loss), FALSE);
                    }
             } else {
                    gain_exp( ch, -1 * (loss), FALSE);
             }
        } else {
             gain_exp( ch, -1 * (loss), FALSE);
        }

        return;
    }


   /* Otherwise lose half a levels worth of xps... */
    loss =  exp_to_advance(ch->level) / 3;

        if (ch->desc != NULL) {
             if (ch->desc->original != NULL ) {
                    if (!str_cmp(race_array[ch->desc->original->race].name,"yithian")) {
                             gain_exp( ch->desc->original, -1 * (loss/2), FALSE);
                    } else {
                             gain_exp( ch, -1 * (loss), FALSE);
                    }
             } else {
                    gain_exp( ch, -1 * (loss), FALSE);
             }
        } else {
             gain_exp( ch, -1 * (loss), FALSE);
        }

    return;
}

/* Old exps per level function */
//
//int exp_per_level(CHAR_DATA *ch, int points)
//{
//
//   return 1500;
//}
//
/* Experience table... */

int exp_table[MAX_LEVEL + 1];


/* Build the experience table... */

void build_exp_table() {
int i;

  exp_table[0] = 0;

  for(i = 1; i < MAX_LEVEL + 1; i++) {
 
    exp_table[i] = exp_table[i-1] + exp_to_advance(i);
// bug("Exp needed: %d", exp_table[i]);
  }

  log_string("...exp table built");

  return;
}


/* Number of xps needed for level x to level x+1... */

int exp_to_advance(int level) {
int needed = 0;

  if (level <  20) {
    needed =   750 +  150 * level;
  } else if (level <  35) {
    needed =  3750 +  200 * (level - 20);
  } else if (level <  50) {
    needed =  6750 +  250 * (level - 35);
  } else if (level <  70) {
    needed = 10500 +  300 * (level - 50);
  } else if (level <  90) {
    needed = 16500 +  350 * (level - 70);  
  } else if (level < 110) {
    needed = 23500 +  400 * (level - 90); 
  } else if (level < 130) {
    needed = 31500 +  500 * (level - 110); 
  } else if (level < 150) {
    needed = 41500 +  750 * (level - 130); 
  } else if (level < 300) {
    needed = 56500 + 1000 * (level - 150);
  } else {
    needed = 206500 + 1250 * (level - 300);
  }

  if (needed < 1000) needed = 1000;
  return needed;
}

/* Number of xps needed for level x... */

int exp_for_level(int level) {

  if (level < 0) {
    bug("Level (%d) less than 0!", level); 
    return 1;
  }  

  if (level > MAX_LEVEL) {
    return exp_table[MAX_LEVEL];
  }

  return exp_table[level];
}

/* Number of xps for a character to reach next level... */

int exp_for_next_level(CHAR_DATA *ch) {

  if (ch->level < 1) {
    bug("Level (%d) less than 1!", ch->level); 
    return 1;
  }  

  if (ch->level >= MAX_LEVEL) {
    return exp_table[MAX_LEVEL] * 2;
  }

  return exp_table[ch->level + 1];
}


/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels ) {
int xp,base_exp;
int align,level_range;
int change;
int echl;

    if ( IS_SET(mud.flags, MUD_EFFXPS)) echl = get_effective_level(gch);
    else echl = gch->level;
    
    level_range = get_effective_level(victim) - echl;
 
    /* compute the base exp */
    switch (level_range)    {
 	default : 	base_exp =   5;		break;
	case -9 :	base_exp =   6;		break;
	case -8 :	base_exp =   8;		break;
	case -7 :	base_exp =  10;		break;
	case -6 : 	base_exp =  12;		break;
	case -5 :	base_exp =  15;		break;
	case -4 :	base_exp =  20;		break;
	case -3 :	base_exp =  30;		break;
	case -2 :	base_exp =  40;		break;
	case -1 :	base_exp =  50;		break;
	case  0 :	base_exp =  70;		break;
	case  1 :	base_exp =  85;		break;
	case  2 :	base_exp = 100;		break;
	case  3 :	base_exp = 115;		break;
	case  4 :	base_exp = 130;		break;
    } 
    
    if (level_range > 4) base_exp = 130 + 12 * (level_range - 4);
    
    /* do alignment computations */
   
    align = victim->alignment - gch->alignment;
    
    if (!IS_SET(gch->plr, PLR_REASON)) {
         if (gch->alignment > 500) align = 1200;
         else if (gch->alignment < -500) align = 0;
         else align = 500; 
    }      

    if (IS_SET(victim->act,ACT_NOALIGN)) {
	/* no change */

    } else if (align >= 350)  {
	change = (align - 350) * base_exp / 500 * gch->level/total_levels; 
	change = UMAX(base_exp/10, change);
                gch->alignment = UMAX(-1000,gch->alignment - change);

    } else if (align <= -350) {
  	    change =  ( -1 * align - 350) * base_exp/500 * gch->level/total_levels;
	    change = UMAX(base_exp/10, change);
	    gch->alignment = UMIN(1000,gch->alignment + change);
    } else {
                    if (align > 0) change = -base_exp/10;
                    else change = base_exp/10;
                    gch->alignment = URANGE(-1000, gch->alignment + change, 1000);
    }
    
    if (!IS_SET(gch->plr, PLR_REASON)) {
               base_exp = base_exp *4 /5;
    }      

    /* calculate exp multiplier */
    if (IS_SET(victim->act,ACT_NOALIGN)) {
	xp = base_exp;

    } else if (gch->alignment > 500) {
	if (victim->alignment < -750)  	xp = (base_exp * 4)/3;
                else if (victim->alignment < -500) 	xp = (base_exp * 5)/4;
                else if (victim->alignment > 750) 	xp = base_exp / 4;
                else if (victim->alignment > 500) 	xp = base_exp / 2;
                else if (victim->alignment > 250)  	xp = (base_exp * 3)/4; 
	else  				xp = base_exp;

    } else if (gch->alignment < -500) {
	if (victim->alignment > 750) 	xp = (base_exp * 5)/4;
	else if (victim->alignment > 500) 	xp = (base_exp * 11)/10; 
   	else if (victim->alignment < -750) 	xp = (base_exp * 1)/2;
	else if (victim->alignment < -500) 	xp = (base_exp * 3)/4;
	else if (victim->alignment < -250) 	xp = (base_exp * 9)/10;
	else   				xp = base_exp;

    } else if (gch->alignment > 200) {
	if (victim->alignment < -500) 	xp = (base_exp * 6)/5;
 	else if (victim->alignment > 750) 	xp = (base_exp * 1)/2;
	else if (victim->alignment > 0) 	xp = (base_exp * 3)/4; 
	else 				xp = base_exp;

    } else if (gch->alignment < -200) {
	if (victim->alignment > 500) 	xp = (base_exp * 6)/5;
 	else if (victim->alignment < -750) 	xp = (base_exp * 1)/2;
	else if (victim->alignment < 0) 	xp = (base_exp * 3)/4;
	else 				xp = base_exp;

    } else {
	if (victim->alignment > 500 || victim->alignment < -500)  	xp = (base_exp * 4)/3;
	else if (victim->alignment < 200 || victim->alignment > -200) 	xp = (base_exp * 1)/2;
 	else  							xp = base_exp;
    }

    /* randomize the rewards */
    xp = number_range ((xp * 3)/4, (xp * 5)/4);

    /* adjust for grouping */
    /* Zeran - multiply by 1.5 to encourage grouping */
    /* Mik - but only when they are in a group ( and only a 1.25 bonus)... */

    if ( total_levels != echl ) xp = (xp * echl * 5)/(total_levels * 4); 
    xp = UMAX(xp, 0);

    return xp;
}


// Work out effective level due to attributes

int get_effective_level(CHAR_DATA *ch) {
int elev;
CHAR_DATA *pmi;

 /* Start off with the level... */

  elev = ch->level;

 /* Get the original Mob data... */

  pmi = ch; // ->pIndexData; 

 /* Up elve so we can work in 3rds or a level... */

  elev *= 3;

 /* ACT first... */

  if (IS_SET(pmi->act, ACT_AGGRESSIVE)) {
    elev += 3;
  }

  if (IS_SET(pmi->act, ACT_UNDEAD)) {
    elev += 3;
  }

  if (IS_SET(pmi->act, ACT_CLERIC)) {
    elev += 2;
  }

  if (IS_SET(pmi->act, ACT_MAGE)) {
    elev += 2;
  }

  if (IS_SET(pmi->act, ACT_THIEF)) {
    elev += 2;
  }

  if (IS_SET(pmi->act, ACT_WARRIOR)) {
    elev += 3;
  }

 /* The the Offense flags... */

  if (IS_SET(pmi->off_flags, OFF_AREA_ATTACK)) {
    elev += 6;
  }

  if (IS_SET(pmi->off_flags, OFF_BACKSTAB)) {
    elev += 2;
  }

  if (IS_SET(pmi->off_flags, OFF_BASH)) {
    elev += 4;
  }

  if (IS_SET(pmi->off_flags, OFF_BERSERK)) {
    elev += 3;
  }

  if (IS_SET(pmi->off_flags, OFF_DISARM)) {
    elev += 3;
  }

  if (IS_SET(pmi->off_flags, OFF_DODGE)) {
    elev += 2;
  }

  if (IS_SET(pmi->off_flags, OFF_FAST)) {
    elev += 6;
  }

  if (IS_SET(pmi->off_flags, OFF_KICK)) {
    elev += 3;
  }

  if (IS_SET(pmi->off_flags, OFF_KICK_DIRT)) {
    elev += 3;
  }

  if (IS_SET(pmi->off_flags, OFF_PARRY)) {
    elev += 2;
  }

  if (IS_SET(pmi->off_flags, OFF_TRIP)) {
    elev += 3;
  }

  if ( IS_SET(pmi->off_flags, OFF_TAIL)
    && IS_SET(pmi->parts, PART_LONG_TAIL) ) {

    elev += 5;

    if ( IS_SET(pmi->parts, PART_STINGER) ) {
      elev += 3;
    }
  }

  if (IS_SET(pmi->off_flags, OFF_CRUSH)) {
    elev += 2;
  }

  if (IS_SET(pmi->off_flags, ASSIST_ALL)) {
    elev += 3;
  }

  if (IS_SET(pmi->off_flags, ASSIST_ALIGN)) {
    elev += 4;
  }

  if (IS_SET(pmi->off_flags, ASSIST_RACE)) {
    elev += 5;
  }

  if (IS_SET(pmi->off_flags, ASSIST_GUARD)) {
    elev += 3;
  }

  if (IS_SET(pmi->off_flags, ASSIST_VNUM)) {
    elev += 2;
  }
 
 /* Immunity flags... */

  if (IS_SET(pmi->imm_flags, IMM_CHARM)) {
    elev += 3;
  }

  if (IS_SET(pmi->imm_flags, IMM_MASK)) {
    elev += 1;
  }

  if (IS_SET(pmi->imm_flags, IMM_MAGIC)) {
    elev += 9;
  }

  if (IS_SET(pmi->imm_flags, IMM_WEAPON)) {
    elev += 9;
  }

  if (IS_SET(pmi->imm_flags, IMM_BASH)) {
    elev += 3;
  }

  if (IS_SET(pmi->imm_flags, IMM_PIERCE)) {
    elev += 3;
  }

  if (IS_SET(pmi->imm_flags, IMM_SLASH)) {
    elev += 3;
  }

  if (IS_SET(pmi->imm_flags, IMM_FIRE)) {
    elev += 5;
  }

  if (IS_SET(pmi->imm_flags, IMM_COLD)) {
    elev += 4;
  }

  if (IS_SET(pmi->imm_flags, IMM_LIGHTNING)) {
    elev += 4;
  }

  if (IS_SET(pmi->imm_flags, IMM_ACID)) {
    elev += 5;
  }

  if (IS_SET(pmi->imm_flags, IMM_POISON)) {
    elev += 3;
  }

  if (IS_SET(pmi->imm_flags, IMM_NEGATIVE)) {
    elev += 3;
  }

  if (IS_SET(pmi->imm_flags, IMM_HOLY)) {
    elev += 3;
  }

  if (IS_SET(pmi->imm_flags, IMM_ENERGY)) {
    elev += 3;
  }

  if (IS_SET(pmi->imm_flags, IMM_MENTAL)) {
    elev += 3;
  }

  if (IS_SET(pmi->imm_flags, IMM_DISEASE)) {
    elev += 3;
  }

  if (IS_SET(pmi->imm_flags, IMM_DROWNING)) {
    elev += 3;
  }

  if (IS_SET(pmi->imm_flags, IMM_LIGHT)) {
    elev += 3;
  }

  if (IS_SET(pmi->imm_flags, IMM_BULLETS)) {
    elev += 3;
  }

 /* Resistance flags... */

  if (IS_SET(pmi->res_flags, RES_CHARM)) {
    elev += 2;
  }

  if (IS_SET(pmi->res_flags, RES_MAGIC)) {
    elev += 5;
  }

  if (IS_SET(pmi->res_flags, RES_WEAPON)) {
    elev += 5;
  }

  if (IS_SET(pmi->res_flags, RES_BASH)) {
    elev += 2;
  }

  if (IS_SET(pmi->res_flags, RES_PIERCE)) {
    elev += 2;
  }

  if (IS_SET(pmi->res_flags, RES_SLASH)) {
    elev += 2;
  }

  if (IS_SET(pmi->res_flags, RES_FIRE)) {
    elev += 2;
  }

  if (IS_SET(pmi->res_flags, RES_COLD)) {
    elev += 2;
  }

  if (IS_SET(pmi->res_flags, RES_LIGHTNING)) {
    elev += 2;
  }

  if (IS_SET(pmi->res_flags, RES_ACID)) {
    elev += 1;
  }

  if (IS_SET(pmi->res_flags, RES_POISON)) {
    elev += 1;
  }

  if (IS_SET(pmi->res_flags, RES_NEGATIVE)) {
    elev += 1;
  }

  if (IS_SET(pmi->res_flags, RES_HOLY)) {
    elev += 1;
  }

  if (IS_SET(pmi->res_flags, RES_ENERGY)) {
    elev += 1;
  }

  if (IS_SET(pmi->res_flags, RES_MENTAL)) {
    elev += 1;
  }

  if (IS_SET(pmi->res_flags, RES_DISEASE)) {
    elev += 1;
  }

  if (IS_SET(pmi->res_flags, IMM_DROWNING)) {
    elev += 1;
  }

  if (IS_SET(pmi->res_flags, RES_LIGHT)) {
    elev += 1;
  }

  if (IS_SET(pmi->res_flags, RES_BULLETS)) {
    elev += 2;
  }

 /* Nature... */

  if (IS_SET(pmi->nature, NATURE_STRONG)) { 
    elev += 2;
  }

  if (IS_SET(pmi->nature, NATURE_FEEBLE)) { 
    elev -= 2;
  }

  if (IS_SET(pmi->nature, NATURE_SMART)) { 
    elev += 2;
  }

  if (IS_SET(pmi->nature, NATURE_DUMB)) {
    elev -= 2;
  }

  if (IS_SET(pmi->nature, NATURE_AGILE)) { 
    elev += 3;
  }

  if (IS_SET(pmi->nature, NATURE_LUMBERING)) {
    elev -= 3;
  }

  if (IS_SET(pmi->nature, NATURE_SLY)) { 
    elev += 2;
  }

  if (IS_SET(pmi->nature, NATURE_GULLIBLE)) {
    elev -= 2;
  }

  if (IS_SET(pmi->nature, NATURE_ROBUST)) {
    elev += 3;
  }

  if (IS_SET(pmi->nature, NATURE_SICKLY)) {
    elev -= 3;
  }

  if (IS_SET(pmi->nature, NATURE_STURDY)) {
    elev += 2;
  }

  if (IS_SET(pmi->nature, NATURE_FRAGILE)) {
    elev -= 2;
  }

  if (IS_SET(pmi->nature, NATURE_MAGICAL)) {
    elev += 2;
  }

  if (IS_SET(pmi->nature, NATURE_MUNDAIN)) {
    elev -= 2;
  }

  if (IS_SET(pmi->nature, NATURE_VISCIOUS)) {
    elev += 3;
  }

  if (IS_SET(pmi->nature, NATURE_HARMLESS)) {
    elev -= 3;
  }

  if (IS_SET(pmi->nature, NATURE_ARMOURED)) {
    elev += 2;
  }

  if (IS_SET(pmi->nature, NATURE_EXPOSED)) {
    elev -= 2;
  }

  if (IS_SET(pmi->nature, NATURE_MONSTEROUS)) {
    elev += 4;
  }

 /* Affects... */

  if ( IS_AFFECTED(ch, AFF_BLIND) ) {
    elev -= 2;
  } 
  
  if ( IS_AFFECTED(ch, AFF_INVISIBLE) ) {
    elev += 2;
  } 
  
  if ( IS_AFFECTED(ch, AFF_DETECT_EVIL) ) {
    elev += 1;
  } 
  
  if ( IS_AFFECTED(ch, AFF_DETECT_INVIS) ) {
    elev += 1;
  } 
  
  if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC) ) {
    elev += 1;
  } 
  
  if ( IS_AFFECTED(ch, AFF_DETECT_HIDDEN) ) {
    elev += 2;
  } 
  
  if ( IS_AFFECTED(ch, AFF_MELD) ) {
    elev -= 1;
  } 
  
  if ( IS_AFFECTED(ch, AFF_SANCTUARY) ) {
    elev += 6;
  } 
  
  if ( IS_AFFECTED(ch, AFF_ELDER_SHIELD) ) {
    elev += 8;
  } 

  if ( IS_AFFECTED(ch, AFF_FAERIE_FIRE) ) {
    elev -= 3;
  } 
  
  if ( IS_AFFECTED(ch, AFF_INFRARED) ) {
    elev += 1;
  } 
  
  if ( IS_AFFECTED(ch, AFF_CURSE) ) {
    elev -= 2;
  } 
  
  if ( IS_AFFECTED(ch, AFF_FEAR) ) {
    elev -= 3;
  } 
  
  if ( IS_AFFECTED(ch, AFF_POISON) ) {
    elev -= 1;
  } 
  
  if ( IS_AFFECTED(ch, AFF_PROTECTG) ) {
    elev += 1;
  } 
  
  if ( IS_AFFECTED(ch, AFF_PROTECTE) ) {
    elev += 1;
  } 
  
  if ( IS_AFFECTED(ch, AFF_SNEAK) ) {
    elev += 1;
  } 
  
  if ( IS_AFFECTED(ch, AFF_HIDE) ) {
    elev += 1;
  } 
  
  if ( IS_AFFECTED(ch, AFF_SLEEP) ) {
    elev -= 3;
  } 
  
  if ( IS_AFFECTED(ch, AFF_CHARM) ) {
    elev -= 3;
  } 
  
  if ( IS_AFFECTED(ch, AFF_FLYING) ) {
    elev += 2;
  } 
  
  if ( IS_AFFECTED(ch, AFF_MIST) ) {
    elev += 6;
  } 

  if ( IS_AFFECTED(ch, AFF_PASS_DOOR) ) {
    elev += 1;
  } 
  
  if ( IS_AFFECTED(ch, AFF_HASTE)
    && !IS_SET(ch->off_flags, OFF_FAST) ) {
    elev += 6;
  } 
  
  if ( IS_AFFECTED(ch, AFF_CALM) ) {
    elev -= 1;
  } 
  
  if ( IS_AFFECTED(ch, AFF_PLAGUE) ) {
    elev -= 2;
  } 
  
  if ( IS_AFFECTED(ch, AFF_WEAKEN) ) {
    elev -= 2;
  } 
  
  if ( IS_AFFECTED(ch, AFF_DARK_VISION) ) {
    elev += 1;
  } 
  
  if ( IS_AFFECTED(ch, AFF_BERSERK) ) {
    elev += 1;
  } 
  
  if ( IS_AFFECTED(ch, AFF_SWIM) ) {
    elev += 1;
  } 
  
  if ( IS_AFFECTED(ch, AFF_REGENERATION) ) {
    elev += 6;
  } 
  
  if ( IS_AFFECTED(ch, AFF_POLY) ) {
    elev += 1;
  } 
  
  if ( IS_AFFECTED(ch, AFF_ABSORB) ) {
    elev += 6;
  } 
  
  if ( IS_AFFECTED(ch, AFF_WATER_BREATHING) ) {
    elev += 1;
  } 

   if ( IS_AFFECTED(ch, AFF_DARKNESS) ) {
    elev += 2;
  }

   if ( IS_AFFECTED(ch, AFF_AURA) ) {
    elev += 4;
  }

   if ( IS_AFFECTED(ch, AFF_RELAXED) ) {
    elev += 1;
  }

   if ( IS_AFFECTED(ch, AFF_FIRE_SHIELD) ) {
    elev += 4;
  }

   if ( IS_AFFECTED(ch, AFF_FROST_SHIELD) ) {
    elev += 4;
  }

   if ( IS_AFFECTED(ch, AFF_HALLUCINATING) ) {
    elev -= 1;
  }

 /* Spell only affects... */

  if ( is_affected(ch, gafn_armor)) {
    elev += 1;
  } 

  if ( is_affected(ch, gafn_shield)) {
    elev += 2;
  } 

  if ( is_affected(ch, gafn_hard_skin)) {
    elev += 2;
  } 


  if ( is_affected(ch, gafn_bless)) {
    elev += 2;
  } 

  if ( is_affected(ch, gafn_giant_strength)) {
    elev += 3;
  } 

 /* Reduce back to normal scale... */

  elev /= 3;

 /* Add 2 levels for a special function... */

  if (ch->spec_fun != NULL) {
    elev += 2;
  }

 /* Ok, give the value back... */ 

  return elev;
}

//
// See if a player goes insane and make them do something... 
//
// context->mob     player whitnessing the event
// context->actor   mob causing the event
//

bool insanity(MOB_CMD_CONTEXT *context, int threat, char *action) {
char buf[MAX_STRING_LENGTH];
AFFECT_DATA af;
int aff_level;
int aff_duration;

 /* Nothing to do if they are not insane... */

  if (context->mob->waking_room != NULL) {
        if (context->mob->nightmare) {
               threat = (threat * 13)/10;
        } else {
               if (!check_skill(context->mob, gsn_dreaming, threat - 65) ) {
                     if (context->mob->level >15
                     && !IS_SET(context->mob->act, ACT_BRAINSUCKED)) {
                         context->mob->nightmare = TRUE;         
                         send_to_char("Everything is so creepy here!\r\n", context->mob);
                     }
               }
        }
  }

   if (context->mob->race == context->actor->race) return TRUE;

   if (IS_SET(context->actor->act, ACT_UNDEAD)) {
       if (IS_SET(context->mob->act, ACT_UNDEAD)
       || check_skill(context->mob, gsn_necromancy, 0)) return TRUE;
   }

   if (context->actor->master == context->mob) return TRUE;
   if (check_sanity(context->mob, threat)) return TRUE;
   if (is_affected(context->mob, gafn_antipsychotica)) return TRUE;

 /* Take the specified action if there is one... */

  if (action != NULL) {
    expand_by_context(action, context, buf);
    enqueue_mob_cmd(context->mob, buf, 0, SAN_CMD_ID);
    return FALSE;
  }

 /* Take a random action if there isn't... */

  if (context->actor != NULL) aff_level = context->actor->level;
  else aff_level = context->mob->level;
  
  aff_duration = 48 - get_sanity(context->mob);

  aff_duration = UMAX(aff_duration, dice(2,6));

  switch (number_bits(4) + get_sanity(context->mob)) {

  /* Start of sanity 1 */

    case 1:
      if (context->actor == NULL) {
        sprintf(buf, "beg");
      } else {
        expand_by_context("beg @at", context, buf);
      }
      break;

  /* Start of sanity 2,3 */

    case 2:
      if (context->actor == NULL) {
        sprintf(buf, "grovel");
      } else {
        expand_by_context("grovel @at", context, buf);
      }
      break;

    case 3:

      send_to_char ("You are struck dumb!\r\n", context->mob); 

      af.type      = 0;
      af.afn	   = gafn_mute;              
      af.level     = aff_level;
      af.duration  = aff_duration;
      af.location  = APPLY_NONE;
      af.modifier  = 0;
      af.bitvector = 0;

      affect_to_char(context->mob, &af);
      stop_fighting(context->mob, TRUE);
      sprintf(buf, "flee");

      break;

  /* Start of sanity 4-7 */

    case 4:

      do_shout(context->mob, "{RGrrrrraaaaagggggghhhhhhhh!!!!!!!{x\r\n"); 

      af.type      = 0;
      af.afn	   = gafn_berserk;              
      af.level     = aff_level;
      af.duration  = aff_duration;

      af.location  = APPLY_HITROLL;
      af.modifier  = aff_duration/5;
      af.bitvector = AFF_BERSERK;

      affect_to_char(context->mob, &af);

      af.location  = APPLY_DAMROLL;
      af.modifier  = aff_duration/5;
      af.bitvector = AFF_BERSERK;

      affect_to_char(context->mob, &af);

      af.location  = APPLY_AC;
      af.modifier  = aff_duration * -1;
      af.bitvector = AFF_BERSERK;

      affect_to_char(context->mob, &af);

      if (context->actor == NULL) {
        sprintf(buf, "flee");
      } else {
        expand_by_context("kill @at", context, buf);
      }

      break;

    case 5:
      if (context->actor == NULL) {
        sprintf(buf, "flee");
      } else {
        expand_by_context("kill @at", context, buf);
      }
      break;

    case 6:
      sprintf(buf, "flee");
      break;

    case 7:
      if (context->actor == NULL) {
        sprintf(buf, "worship");
      } else {
        expand_by_context("worship @at", context, buf);
      }
      break;

   /* Start of sanity 8-15 */

    case 8:
      sprintf(buf, "threaten");
      break;

    case 9:
      sprintf(buf, "babble");
      break;

    case 10:

      send_to_char ("The strength departs your limbs!\r\n", context->mob); 

      af.type      = 0;
      af.afn	   = gafn_weakness;              
      af.level     = aff_level;
      af.duration  = aff_duration;
      af.location  = APPLY_STR;
      af.modifier  = aff_duration;
      af.bitvector = AFF_WEAKEN;

      affect_to_char(context->mob, &af);

      stop_fighting(context->mob, TRUE);

      sprintf(buf, "flee");

      break;

    case 11:
      sprintf(buf, "shout Mummy! Help!");
      break;

    case 12:
      sprintf(buf, "shout Scooby do, where are you!");
      break;

    case 13:
      sprintf(buf, "shout Hobbes!");
      break;

    case 14:
      sprintf(buf, "shout Doctor!");
      break;

    case 15:

      send_to_char ("You feel very vulnerable and scared!\r\n", context->mob); 

      af.type      = 0;
      af.afn	   = gafn_fear;              
      af.level     = aff_level;
      af.duration  = aff_duration;
      af.location  = 0;
      af.modifier  = 0;
      af.bitvector = AFF_FEAR;

      affect_to_char(context->mob, &af);

      stop_fighting(context->mob, TRUE);

      sprintf(buf, "flee");

      break;

  /* Start of sanity 16-31 */

    case 16:
      sprintf(buf, "drool");
      break;

  /* End of sanity 0 */

    case 17:
      if (context->actor == NULL) {
        sprintf(buf, "point");
      } else {
        expand_by_context("point @at", context, buf);
      }
      break;

    case 18:
      if (context->actor == NULL) {
        sprintf(buf, "cringe");
      } else {
        expand_by_context("cringe @at", context, buf);
      }
      break;

   /* End of sanity 2,3 */

    case 20:
      sprintf(buf, "mutter");
      break;

    case 21:
      sprintf(buf, "sob");
      break;

    case 22:
      sprintf(buf, "duck");
      break;

   /* End of sanity 4-7 */

    case 23:
      if (context->actor == NULL) {
        sprintf(buf, "raspberry");
      } else {
        expand_by_context("raspberry @at", context, buf);
      }
      break;

    case 24:
      sprintf(buf, "run");
      break;

    case 25:
      sprintf(buf, "insane");
      break;

    case 26:
      sprintf(buf, "puke");
      break;

    case 27:
      sprintf(buf, "gasp");
      break;

    case 28:
      sprintf(buf, "boggle");
      break;

    case 29:
      sprintf(buf, "scream");
      break;

    case 30:
      sprintf(buf, "faint");
      break;

   /* End of sanity 8-15 */

    case 31:
      if (context->actor == NULL) {
        sprintf(buf, "snarl");
      } else {
        expand_by_context("snarl @at", context, buf);
      }
      break;

    case 32:
      if (context->actor == NULL) {
        sprintf(buf, "roll");
      } else {
        expand_by_context("roll @at", context, buf);
      }
      break;

   /* End of sanity 16-31 */

    default:
      sprintf(buf, "cower");
      break;
  }

  enqueue_mob_cmd(context->mob, buf, 0, SAN_CMD_ID);
  return FALSE;
}


/* Check a players sanity... */

bool check_sanity(CHAR_DATA *ch, int threat) {
int san_roll;
int threat_roll;
int delta;
int sanity;

 /* The completely insane my act sanely... */

  if (ch->sanity < 1) return FALSE;

 /* Make some rolls... */

  san_roll = get_sanity(ch) + number_open();
  threat_roll = threat + number_open();
  delta = san_roll - threat_roll;


/*Only Brainsucked Peaople are fearless */

  if ((IS_SET(ch->act, ACT_BRAINSUCKED)
       || IS_AFFECTED(ch,AFF_RELAXED))
  && delta<0) {
       delta +=30;
       if (delta>0) delta=0;
       send_to_char("Your brain goes {g*click*{x - you're cool...\r\n", ch);
   }

   if (ch->condition[COND_DRUNK] > 15
   && delta<0) {
       delta +=15;
       if (delta>0) delta=0;
       send_to_char("Now that was scary - hohoho...\r\n", ch);
   }

 /* Good roll improves your sanity... */ 

  if (delta > 100) {
      if (get_sanity(ch) < threat) ch->sanity += dice(2,6);
      return TRUE;
  }

 /* Ok roll, means your ok... */ 

  if (delta >= 10) return TRUE;
  
 /* Borderline role makes you doubt... */

  if (delta >= 0) {
     if (get_sanity(ch) > 32) ch->sanity--;
     return TRUE;
  }

 /* Bad roll reduces sanity... */

  delta *= -1;
  
  delta = delta/10;

  sanity = get_sanity(ch);

  if (sanity == 1) {
    delta = 0;
  } else if (sanity < 8) {
    delta = UMIN(delta/3, 1);
  } else if (sanity < 16) {
    delta = UMIN(delta/3, 1);
  } else if (sanity < 32) {
    delta = UMIN(delta/2, 2);
  } else { 
    delta = UMIN(delta/2, 2);
  }  

  ch->sanity -= delta;
  return FALSE;
}

char train_names[TRAIN_MAX][15] = {
  "strength",
  "intelligence", 
  "wisdom",
  "dexterity",
  "constitution",
  "hits",
  "mana",
  "move",
  "practice",
  "sanity", 
  "charisma"
};


void do_train_status(CHAR_DATA *ch) {
char outbuf[MAX_STRING_LENGTH];
char buf[MAX_STRING_LENGTH];

  int attr, gain, count;

  char *color;

  sprintf(outbuf, "Attribute           Trains   To Gain\r\n"
                  "------------------  -------  -------\r\n");

  for (attr = 0; attr < TRAIN_MAX; attr++) {

    switch (attr) {
      default:
        gain  = 1;
        count = 1;
 
        while (gain <= ch->training[attr]) {
          count += 1;
          gain += count;
        }

        gain = gain - ch->training[attr];
        
        break;

      case TRAIN_PRACTICE:  
      case TRAIN_SANITY:
  
        gain = 1;

        break;
    }

    if (gain <= ch->train) {
      color = "{g";
    } else {
      color = "{c";
    }

    sprintf(buf, "%s%-18s{x   %5d   %5d\r\n", color, train_names[attr], ch->training[attr], gain);
    strcat(outbuf, buf);
  }

  sprintf(buf, "{wYou have %d training sessions.{x\r\n", ch->train);
  strcat(outbuf, buf);

    if (IS_NPC(ch)
    && IS_AFFECTED(ch, AFF_CHARM)
    && ch->master!=NULL) {
         send_to_char(outbuf, ch->master);
    } else {
         send_to_char(outbuf, ch);
    }

  return;
}

bool train_stat(CHAR_DATA *ch, int stat, char *maxed, char *gained) {
int value;
int fuzz = 3;

  value = ch->perm_stat[stat];

  if (value >= STAT_MAXIMUM) {
    send_to_char(maxed, ch);
    return FALSE;
  } 

  if (ch->profs) {
     fuzz = ch->profs->profession->stats[stat];
  }
  if (value > 50) fuzz = UMAX(1, fuzz-1);
  
  if (value > 97) value += 1;
  else value += number_fuzzy(fuzz);
 
  value = URANGE(STAT_MINIMUM, value, STAT_MAXIMUM);
  ch->perm_stat[stat] = value;
  send_to_char(gained, ch);
  return TRUE;
}


void do_train_attribute(CHAR_DATA *ch, int attr) {
  int gain, count;

  bool trained;

 /* Work out the cost... */

  switch (attr) {
    default:
      gain  = 1;
      count = 1;
 
      while (gain <= ch->training[attr]) {
        count += 1;
        gain += count;
      }

      gain = gain - ch->training[attr];
       
      break;

    case TRAIN_PRACTICE:  
    case TRAIN_SANITY:
  
      gain = 1;

      break;
  }

 /* See if they can afford it... */

  if (gain > ch->train) {
    send_to_char("You do not have enough training sessions...\r\n", ch);
    return;
  } 

 /* Pay the cost... */
  
  ch->train -= gain;

 /* Get the benefit... */

  switch (attr) {

    case TRAIN_STAT_STR:
      trained = train_stat(ch, STAT_STR,  "You are already as strong as you can be!\r\n", "You feel stronger!\r\n");
      break;

    case TRAIN_STAT_INT:
      trained = train_stat(ch, STAT_INT, "You are already as clever as you can be!\r\n", "You feel smarter!\r\n");
      if (trained) ch->sanity -= 1;
      break;

    case TRAIN_STAT_WIS:
      trained = train_stat(ch, STAT_WIS, "You are already as wise as you can be!\r\n", "You feel wiser!\r\n");
      if (trained) ch->sanity += 1;
      break;

    case TRAIN_STAT_DEX:
      trained = train_stat(ch, STAT_DEX, "You are already as agile as you can be!\r\n",  "You feel more dexterous!\r\n");
      break;

    case TRAIN_STAT_CON:
      trained = train_stat(ch, STAT_CON, "You are already as healthy as you can be!\r\n", "You feel tougher!\r\n");
      break;

    case TRAIN_STAT_CHA:
      trained = train_stat(ch, STAT_CHA, "You are already as charismatic as you can be!\r\n", "You feel charismatic!\r\n");
      break;

    case TRAIN_HITS:
      trained = TRUE;
      ch->max_hit += number_fuzzy(5);
      send_to_char("You feel more enduring.\r\n", ch);
      break;

    case TRAIN_MANA:
      trained = TRUE;
      ch->max_mana += number_fuzzy(5);
      send_to_char("You feel more energetic.\r\n", ch);
      break;

    case TRAIN_MOVE:
      trained = TRUE;
      ch->max_move += number_fuzzy(5);
      send_to_char("You feel more mobile.\r\n", ch);
      break;

    case TRAIN_PRACTICE:
      trained = TRUE;
      ch->practice += number_fuzzy(2);
      send_to_char("You feel more studious.\r\n", ch);
      break;

    case TRAIN_SANITY:
      trained = TRUE;
      ch->sanity += number_fuzzy(16);
      send_to_char("You feel a little more in control.\r\n", ch);
      break;

    default:
      trained = FALSE;
  }

 /* Refund if it failed, save if it worked... */

  if (!trained) {
    ch->train += gain;
  } else {
    ch->training[attr] += gain;
    if (IS_NEWBIE(ch)) gain_exp(ch, 100, FALSE);
    save_char_obj(ch);
  }

 /* All done... */

  return;
}

void do_train(CHAR_DATA *ch, char *args) {
  int attribute;

  if ( args[0] == '\0'
    || args[0] == '?' ) {
    do_train_status(ch);
    return; 
  } 

  attribute = 0;

  while ( attribute < TRAIN_MAX && str_cmp(args, train_names[attribute])) {
    attribute += 1;
  }

  if (attribute < TRAIN_MAX) {
    do_train_attribute(ch, attribute);
  } else {
    do_train_status(ch);
  }

  return;
}

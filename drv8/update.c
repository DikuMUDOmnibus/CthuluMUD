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
#include "exp.h"
#include "affect.h"
#include "mob.h"
#include "doors.h"
#include "wev.h"
#include "profile.h"
#include "chat.h"
#include "online.h"
#include "race.h"
#include "music.h"
#include "cult.h"
#include "bank.h"
#include "gsn.h"


/* command procedures needed */
DECLARE_DO_FUN(do_quit);
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_yell);
DECLARE_DO_FUN(do_echo);

/*
 * Local functions.
 */
int			hit_gain			(CHAR_DATA *ch);
int			mana_gain		(CHAR_DATA *ch);
int			move_gain		(CHAR_DATA *ch);
void			mobile_update		(void);
void			weather_update		(bool random);
void			time_update		(void);
void			char_update		(void);
void			obj_update		(void);
void			aggr_update		(void);
void        		society_update		(void);
void        		char_env_update		(void);
void        		quest_update    		(void);
void        		room_aff_update		(ROOM_INDEX_DATA *room ); 
void        		writeout			(void);
void        		reboot_shutdown 	(short type);
void        		check_muddeath		(void);
void        		update_tree		(OBJ_DATA *tree);
int           			get_effect_efn		(char *name);
void        		update_align		(CHAR_DATA *ch);
bool        		update_age		(CHAR_DATA *ch);
void        		update_charge		(OBJ_DATA *obj, AFFECT_DATA *paf);
void 			produce_money		(OBJ_DATA *obj, AFFECT_DATA *paf);
void 			produce_exp		(OBJ_DATA *obj, AFFECT_DATA *paf);
void 			produce_sanity		(OBJ_DATA *obj, AFFECT_DATA *paf);
void 			modify_align		(OBJ_DATA *obj, AFFECT_DATA *paf);
void        		update_artifact		(OBJ_DATA *obj);
void 			partners_update		(void);
AFFECT_DATA		*new_affect		(void);
void 			save_artifacts		(void);
void 			scheduled_reboot	(void);

extern     void 		save_all_lockers		(void);
extern     bool 		env_damage		(CHAR_DATA *ch, int dam, int dam_type, char *msg_all, char *msg_victim);
extern     void 		raw_kill			(CHAR_DATA *victim, bool corpse);

/* used for saving */

int	save_number = 0;
extern short reboot_type;

extern FILE *time_file;


/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch ) {
    int gain, hr;
    int number;

    if ( IS_NPC(ch) )   {
	gain =  6 + ch->level;

 	if (IS_AFFECTED(ch,AFF_REGENERATION)) gain *= 2;
 	if (!str_cmp(race_array[ch->race].name, "old one")) gain *=5;

	switch(ch->position) {
	    default : 		gain /= 2;			break;
	    case POS_SLEEPING: 	gain = 3 * gain/2;		break;
	    case POS_RESTING:  					break;
	    case POS_FIGHTING:	gain /= 3;		 	break;
 	}
    }    else    {
	gain = (get_curr_stat(ch,STAT_CON) - 12)/4 + ch->level; 
                gain = UMAX(gain, 3);
                number = get_skill_roll(ch, gsn_fast_healing);
                number = UMAX(number, 100); 

	if ( number > 100 ) {
                     gain = (number * gain) / 100;

                     if (ch->hit < ch->max_hit) check_improve(ch,gsn_fast_healing,TRUE,8);

	     if (IS_AFFECTED(ch, AFF_REGENERATION)) gain = (gain * 3)/2;
 	     if (!str_cmp(race_array[ch->race].name, "old one")) gain *=3;

	     switch ( ch->position ) {
	         default:	   	gain /= 4;			break;
	         case POS_SLEEPING: 					break;
	         case POS_RESTING:  	gain /= 2;			break;
	         case POS_FIGHTING: 	gain /= 6;			break;
	     }  

    	     if ( ch->condition[COND_FOOD]  < COND_VERY_HUNGRY ) gain /= 2;
                     if ( ch->condition[COND_DRINK] < COND_VERY_THIRSTY ) gain /= 2;
                     if ( ch->condition[COND_DRUNK] > 0 ) gain += 1;
                }

                if (ch->in_room != NULL) {
                    if (IS_SET(ch->in_room->room_flags, ROOM_FASTHEAL) ) {
	        number = number_percent();
                        gain += ((number * gain) / 100) + 1; 
                    }

                    hr = get_heal_rate(ch->in_room);
                    if (hr != 100 ) gain = gain * hr / 100;
                }

    }

    if ( IS_AFFECTED(ch, AFF_POISON)) gain /= 4;
    if (IS_AFFECTED(ch, AFF_PLAGUE)) gain /= 8;
    if (IS_AFFECTED(ch,AFF_HASTE)) gain /=2 ;

   /* Divide by 6, but round up. Apparently this routine is called six
    * times more often than it was expected to be...
    */

    if (ch->fighting == NULL) gain += 5;
  
    gain /= 6; 
    if (ch->on_furniture != NULL) gain = gain * (100 + ch->on_furniture->value[1]) / 100; 

     if (!IS_NPC(ch)) {
          if (get_age(ch)*100/(ch->pcdata->lifespan+1) > number_percent()
          && !IS_SET (ch->act, ACT_UNDEAD)
          && !IS_ARTIFACTED(ch, ART_BLESSING_NEPHRAN)
          && get_age(ch) > ch->pcdata->lifespan/2) {
                gain /=2;
          }
    }
    return UMIN(gain, ch->max_hit - ch->hit);
}



int mana_gain( CHAR_DATA *ch ) {
    int gain, mr;
    int number;

    if ( IS_NPC(ch)) {
	gain = 5 + ch->level;
	switch (ch->position) {
	    default:		gain /= 2;		break;
	    case POS_SLEEPING:	gain = 3 * gain/2;	break;
   	    case POS_RESTING:				break;
	    case POS_FIGHTING:	gain /= 3;		break;
    	}

    } else {
	gain = ( get_curr_stat(ch, STAT_WIS)  + get_curr_stat(ch, STAT_INT) )/4 + ch->level;

	number = get_skill_roll(ch, gsn_meditation);

        number = UMAX(number, 100); 

	/* elfren added and for psionic meld */

	if ( number > 100
	&& !IS_AFFECTED(ch, AFF_MELD)) {
	    gain = (number * gain) / 100;

	    if (ch->mana < ch->max_mana) check_improve(ch,gsn_meditation,TRUE,8);

	}

        gain *= 2;

        mr = get_mana_rate(ch->in_room);
        if (mr != 100 ) gain =  gain * mr  / 100;

	switch ( ch->position ) {
	    default:		gain /= 4;			break;
	    case POS_SLEEPING: 					break;
	    case POS_RESTING:	gain /= 2;			break;
	    case POS_FIGHTING:	gain /= 6;			break;
	}

	if ( ch->condition[COND_FOOD]  < COND_VERY_HUNGRY ) gain /= 2;
	if ( ch->condition[COND_DRINK] < COND_VERY_THIRSTY ) gain /= 2;
	if ( ch->condition[COND_DRUNK] > 0 ) gain /= 2;

    }

    if ( IS_AFFECTED( ch, AFF_POISON )) gain /= 4;
    if (IS_AFFECTED(ch, AFF_PLAGUE)) gain /= 8;

  /* Round up when reducing to adjust for frequency... */

    if (ch->fighting == NULL) gain += 5;
  
    gain /= 6; 
    if (ch->on_furniture != NULL) gain = gain * (100 + ch->on_furniture->value[1]) / 100; 

    if (!IS_NPC(ch)) {
        if (get_age(ch)*100/(ch->pcdata->lifespan+1) > number_percent()
        && !IS_SET (ch->act, ACT_UNDEAD)
        && !IS_ARTIFACTED(ch, ART_BLESSING_NEPHRAN)
        && get_age(ch) > ch->pcdata->lifespan/2) {
             gain /=2;
        }
    }
    return UMIN(gain, ch->max_mana - ch->mana);
}



int move_gain( CHAR_DATA *ch ) {
    int gain;

    if ( IS_NPC(ch) )    {
	gain = ch->level;
    }    else    {
	gain = UMAX( 15, ch->level );

	switch ( ch->position ) {

          case POS_SLEEPING:
            gain += (get_curr_stat(ch, STAT_DEX) / 4);
            break;

          case POS_RESTING:
            gain += (get_curr_stat(ch, STAT_DEX) / 8);
            break;
    }

	if ( ch->condition[COND_FOOD]  < COND_VERY_HUNGRY ) gain /= 2;
	if ( ch->condition[COND_DRINK] < COND_VERY_THIRSTY ) gain /= 2;
	if ( ch->condition[COND_DRUNK] > 0 ) gain /= 2;
    }

    if ( IS_AFFECTED(ch, AFF_POISON)) gain /= 4;
    if (IS_AFFECTED(ch, AFF_PLAGUE)) gain /= 8;
    if (IS_AFFECTED(ch,AFF_HASTE)) gain /=2 ;

    if (ch->fighting == NULL) gain += 5;
  
    gain /=6; 
    if (ch->on_furniture != NULL) gain = gain * (100 + ch->on_furniture->value[1]) / 100; 

    if (!IS_NPC(ch)) {
        if (get_age(ch)*100/(ch->pcdata->lifespan+1) > number_percent()
        && !IS_SET (ch->act, ACT_UNDEAD)
        && !IS_ARTIFACTED(ch, ART_BLESSING_NEPHRAN)
        && get_age(ch) > ch->pcdata->lifespan/2) {
             gain /=2;
        }
    }
    if (ch->limb[4] >9) gain =0;
    return UMIN(gain, ch->max_move - ch->move);
}


void affect_remove_afn(CHAR_DATA *ch, int afn, int count) {
AFFECT_DATA *af, *af_next;
int i;

  af = ch->affected;

  i = 0;

  while ( af != NULL
  && i < count ) {

    af_next = af->next;

    if ( af->afn == afn ) {
      affect_remove(ch, af);
      i += 1;
      af = NULL;
    }  

    af = af_next;
  } 
  return;
}


void gain_condition( CHAR_DATA *ch, int icond, int value ) {
    AFFECT_DATA af;
    int old_value, new_value;
    int old_count, new_count;
    char *msg;

   /* Initialize... */
    msg = NULL;

   /* Sanity checks... */
    if ( value == 0 
    || IS_NPC(ch)) return;
    
    if ( icond < COND_FOOD
    || icond > COND_FAT ) return;
    
   /* Calculate new value... */

    old_value = ch->condition[icond];
    new_value = ch->condition[icond] + value;

   /* See what happens... */

    switch (icond) {

      case COND_FOOD:

       /* Adjust for FAT burning... */

        if ( new_value < COND_VERY_HUNGRY
        && ch->condition[COND_FAT] > 0 ) {
          
          gain_condition(ch, COND_FAT, -10);
 
          new_value += COND_FAT_BURN;
        }

       /* Situation and message... */

        if ( new_value > COND_STUFFED ) {

          if (new_value > old_value) {

            affect_strip_afn(ch, gafn_starvation);

            if ( old_value < COND_STUFFED ) {
              msg = str_dup("You feel to stuffed to eat anything else!\r\n");
            } else {
              gain_condition(ch, COND_FAT, dice(1,6));
            }
          }

        } else if ( new_value > COND_HUNGRY ) {

          if ( new_value > old_value) {

            affect_strip_afn(ch, gafn_starvation);

            if ( old_value < COND_HUNGRY ) msg = str_dup("You no longer feel hungry.\r\n");

          } else {
            
            if ( old_value > COND_STUFFED )  msg = str_dup("You no longer feel to stuffed to eat.\r\n");
          }

        } else if ( new_value > COND_VERY_HUNGRY ) {

          if ( new_value > old_value) {

            affect_strip_afn(ch, gafn_starvation);

            if ( old_value < COND_STARVING ) msg = str_dup("Now you only feel hungry.\r\n");

          } else {
            
            if ( old_value > COND_STUFFED ) msg = str_dup("You feel hungry.\r\n");
          }

        } else if ( new_value > COND_STARVING ) {

          if ( new_value > old_value) {

            affect_strip_afn(ch, gafn_starvation);

            if ( old_value < COND_STARVING ) msg = str_dup("Now you only feel very hungry.\r\n");

          } else {
            
            if ( old_value > COND_STUFFED ) msg = str_dup("{yYou feel very hungry!{x\r\n");
          }

        } else {

          if ( old_value > COND_STARVING ) msg = str_dup("{YYou are starving to death!{x\r\n");

          old_count = -1 * ( old_value / COND_STARVE_INTV);

          new_count = -1 * ( new_value / COND_STARVE_INTV);

          if ( old_count != new_count ) {

            if (new_count > old_count) {
              msg = str_dup("{YYou are dying of starvation!{x\r\n"); 
            } else {
              msg = str_dup("{yThat feels better, but you are still starving!{x\r\n"); 
            } 

            affect_strip_afn(ch, gafn_starvation);

            af.type		= SKILL_UNDEFINED;
            af.afn		= gafn_starvation;
            af.level		= MAX_LEVEL;
            af.duration		= -1;
            af.bitvector	= 0;

            af.location		= APPLY_STR;
            af.modifier		= -1 * new_count;
            affect_to_char( ch, &af );

            af.location		= APPLY_DEX;
            af.modifier		= -1 * new_count;
            affect_to_char( ch, &af );

            af.location		= APPLY_CON;
            af.modifier		= -1 * new_count;
            affect_to_char( ch, &af );

            af.location		= APPLY_INT;
            af.modifier		= -1 * new_count;
            affect_to_char( ch, &af );

            af.location		= APPLY_WIS;
            af.modifier		= -1 * new_count;
            affect_to_char( ch, &af );
 
            af.location		= APPLY_HIT;
            af.modifier		= -1 * ch->level * new_count;
            affect_to_char( ch, &af );

            af.location		= APPLY_MOVE;
            af.modifier		= -1 * ch->level * new_count;
            affect_to_char( ch, &af );

            af.location		= APPLY_MANA;
            af.modifier		= -1 * ch->level * new_count;
            affect_to_char( ch, &af );

          } 

        }

        break;

      case COND_DRINK:

       /* Situation and message... */

        if ( new_value > COND_SLUSHY ) {

          if (new_value > old_value) {

            affect_strip_afn(ch, gafn_dehydration);

            if ( old_value < COND_SLUSHY ) msg = str_dup("You feel too full to drink anymore!\r\n");
          }

        } else if ( new_value > COND_THIRSTY ) {

          if ( new_value > old_value) {

            affect_strip_afn(ch, gafn_dehydration);

            if ( old_value < COND_THIRSTY ) msg = str_dup("You no longer feel thirsty.\r\n");

          } else {
            
            if ( old_value > COND_SLUSHY ) msg = str_dup("You no longer feel too full to drink.\r\n");
          }

        } else if ( new_value > COND_VERY_THIRSTY ) {

          if ( new_value > old_value) {

            affect_strip_afn(ch, gafn_dehydration);

            if ( old_value < COND_DEHYDRATED ) msg = str_dup("Now you only feel thirsty.\r\n");

          } else {
            
            if ( old_value > COND_SLUSHY ) msg = str_dup("You feel thirsty.\r\n");
           }

        } else if ( new_value > COND_DEHYDRATED ) {

          if ( new_value > old_value) {

            affect_strip_afn(ch, gafn_dehydration);

            if ( old_value < COND_DEHYDRATED ) msg = str_dup("Now you only feel very thirsty.\r\n");

          } else {
            
            if ( old_value > COND_SLUSHY ) msg = str_dup("{yYou feel very thirsty!{x\r\n");
          }

        } else {

          if ( old_value > COND_DEHYDRATED ) msg = str_dup("{YYou are dangerously dehydrated!{x\r\n");

          old_count = -1 * ( old_value / COND_DEHYD_INTV);

          new_count = -1 * ( new_value / COND_DEHYD_INTV);

          if ( old_count != new_count ) {

            if (new_count > old_count) {
              msg = str_dup("{YYou are dying of dehydration!{x\r\n"); 
            } else {
              msg = str_dup("{yThat feels better, but you are still dehydrated!{x\r\n"); 
            } 

            affect_strip_afn(ch, gafn_dehydration);

            af.type		= SKILL_UNDEFINED;
            af.afn		= gafn_dehydration;
            af.level		= MAX_LEVEL;
            af.duration		= -1;
            af.bitvector	= 0;

            af.location		= APPLY_STR;
            af.modifier		= -1 * new_count;
            affect_to_char( ch, &af );

            af.location		= APPLY_DEX;
            af.modifier		= -1 * new_count;
            affect_to_char( ch, &af );

            af.location		= APPLY_CON;
            af.modifier		= -1 * new_count;
            affect_to_char( ch, &af );

            af.location		= APPLY_INT;
            af.modifier		= -1 * new_count;
            affect_to_char( ch, &af );

            af.location		= APPLY_WIS;
            af.modifier		= -1 * new_count;
            affect_to_char( ch, &af );
 
            af.location		= APPLY_HIT;
            af.modifier		= -1 * ch->level * new_count;
            affect_to_char( ch, &af );

            af.location		= APPLY_MOVE;
            af.modifier		= -1 * ch->level * new_count;
            affect_to_char( ch, &af );

            af.location		= APPLY_MANA;
            af.modifier		= -1 * ch->level * new_count;
            affect_to_char( ch, &af );

          } 

        }

        break;

      case COND_DRUNK:

       /* Cannot have negative drunk... */

        new_value = UMAX(new_value, 0);

       /* Work out old and new levels... */

        if ( old_value < COND_TIPSY ) {
          old_count = COND_IS_SOBER;
        } else if ( old_value < COND_MERRY ) {
          old_count = COND_IS_TIPSY;
        } else if ( old_value < COND_DRUNK2 ) {
          old_count = COND_IS_MERRY;
        } else if ( old_value < COND_DIZZY ) {
          old_count = COND_IS_DRUNK;
        } else if ( old_value < COND_STUPOR ) {
          old_count = COND_IS_DIZZY;
        } else {
          old_count = COND_IS_SICK;
        } 

        if ( new_value < COND_TIPSY ) {
          new_count = COND_IS_SOBER;
        } else if ( new_value < COND_MERRY ) {
          new_count = COND_IS_TIPSY;
        } else if ( new_value < COND_DRUNK2 ) {
          new_count = COND_IS_MERRY;
        } else if ( new_value < COND_DIZZY ) {
          new_count = COND_IS_DRUNK;
        } else if ( new_value < COND_STUPOR ) {
          new_count = COND_IS_DIZZY;
        } else {
          new_count = COND_IS_SICK;
        } 

       /* Compare and contrast... */

        switch ( new_count ) {

          case COND_IS_SOBER:
          case COND_IS_TIPSY:

           /* Becoming tipsy? */

            if ( old_count == COND_IS_SOBER
            && new_count == COND_IS_TIPSY ) {
              msg = str_dup("You feel a warm glow inside.\r\n");
            }

           /* Sobering up? */

            if ( old_count > COND_IS_TIPSY ) {

              msg = str_dup("{YYou feel terrible...{x\r\n");
  
              affect_strip_afn(ch, gafn_intoxication);
              affect_strip_afn(ch, gafn_hangover);
        	    
              af.type		= SKILL_UNDEFINED;
              af.afn		= gafn_hangover;
              af.level		= MAX_LEVEL;
              af.duration	= 6 + number_range(1,8);
              af.bitvector	= 0;

              af.location	= APPLY_HITROLL;
              af.modifier	= -2;
              affect_to_char( ch, &af );

              af.location	= APPLY_DAMROLL;
              af.modifier	= -2;
              affect_to_char( ch, &af );

              af.location	= APPLY_MANA;
              af.modifier	= -2 * ch->level;
              affect_to_char( ch, &af );

              af.location	= APPLY_MOVE;
              af.modifier	= -2 * ch->level;
              affect_to_char( ch, &af );
            } 

            break; 

          case COND_IS_MERRY:

            if ( old_count < COND_IS_MERRY) {

              affect_strip_afn(ch, gafn_intoxication);
        	    
              msg = str_dup("{mYou feel merry!{x\r\n");

              af.type		= SKILL_UNDEFINED;
              af.afn		= gafn_intoxication;
              af.level		= MAX_LEVEL;
              af.duration	= -1;
              af.bitvector	= 0;

              af.location	= APPLY_DEX;
              af.modifier	= -5;
              affect_to_char( ch, &af );

              af.location	= APPLY_HITROLL;
              af.modifier	= -1;
              affect_to_char( ch, &af );

              af.location	= APPLY_DAMROLL;
              af.modifier	= -1;
              affect_to_char( ch, &af );

              af.location	= APPLY_MOVE;
              af.modifier	= -1 * ch->level;
              affect_to_char( ch, &af );

            }

            break; 

          case COND_IS_DRUNK:

            if ( old_count < COND_IS_DRUNK) {

              affect_strip_afn(ch, gafn_intoxication);
        	    
              msg = str_dup("{MYou feel drunk!{x\r\n");

              af.type		= SKILL_UNDEFINED;
              af.afn		= gafn_intoxication;
              af.level		= MAX_LEVEL;
              af.duration	= -1;
              af.bitvector	= 0;

              af.location	= APPLY_DEX;
              af.modifier	= -10;
              affect_to_char( ch, &af );

              af.location	= APPLY_CON;
              af.modifier	= -5;
              affect_to_char( ch, &af );

              af.location	= APPLY_HITROLL;
              af.modifier	= -2;
              affect_to_char( ch, &af );

              af.location	= APPLY_DAMROLL;
              af.modifier	= -2;
              affect_to_char( ch, &af );

              af.location	= APPLY_MOVE;
              af.modifier	= -2 * ch->level;
              affect_to_char( ch, &af );

            } 

            break; 

          case COND_IS_DIZZY:

            if ( old_count < COND_IS_DIZZY) {

              affect_strip_afn(ch, gafn_intoxication);
        	    
              msg = str_dup("{MYou feel a little dizzy!{x\r\n");

              af.type		= SKILL_UNDEFINED;
              af.afn		= gafn_intoxication;
              af.level		= MAX_LEVEL;
              af.duration	= -1;
              af.bitvector	= 0;

              af.location	= APPLY_DEX;
              af.modifier	= -20;
              affect_to_char( ch, &af );

              af.location	= APPLY_CON;
              af.modifier	= -15;
              affect_to_char( ch, &af );

              af.location	= APPLY_HITROLL;
              af.modifier	= -4;
              affect_to_char( ch, &af );

              af.location	= APPLY_DAMROLL;
              af.modifier	= -4;
              affect_to_char( ch, &af );

              af.location	= APPLY_HIT;
              af.modifier	= -1 * ch->level;
              affect_to_char( ch, &af );

              af.location	= APPLY_MANA;
              af.modifier	= -2 * ch->level;
              affect_to_char( ch, &af );

              af.location	= APPLY_MOVE;
              af.modifier	= -3 * ch->level;
              affect_to_char( ch, &af );

            }

            break; 

          case COND_IS_SICK:

            if ( old_count < COND_IS_SICK) {

              affect_strip_afn(ch, gafn_intoxication);
        	    
              msg = str_dup("{MYou feel very unwell!{x\r\n");

              af.type		= SKILL_UNDEFINED;
              af.afn		= gafn_intoxication;
              af.level		= MAX_LEVEL;
              af.duration	= -1;
              af.bitvector	= 0;

              af.location	= APPLY_DEX;
              af.modifier	= -40;
              affect_to_char( ch, &af );

              af.location	= APPLY_CON;
              af.modifier	= -40;
              affect_to_char( ch, &af );

              af.location	= APPLY_HITROLL;
              af.modifier	= -8;
              affect_to_char( ch, &af );

              af.location	= APPLY_DAMROLL;
              af.modifier	= -8;
              affect_to_char( ch, &af );

              af.location	= APPLY_HIT;
              af.modifier	= -2 * ch->level;
              affect_to_char( ch, &af );

              af.location	= APPLY_MANA;
              af.modifier	= -4 * ch->level;
              affect_to_char( ch, &af );

              af.location	= APPLY_MOVE;
              af.modifier	= -4 * ch->level;
              affect_to_char( ch, &af );

            }

            break; 

          default:
            break;
        }

        break; 

      case COND_FAT:
 
       /* Cannot have negative fat... */

        new_value = UMAX(new_value, 0);

       /* Calculate new counts... */

        old_count = old_value / 10;
        new_count = new_value / 10; 

       /* Compare and contrast... */

        if ( new_count != old_count ) {

          if ( new_count > old_count ) { 
            msg = str_dup("Hmmm. You seem to have put on a few pounds...\r\n"); 
          } else {
            msg = str_dup("You feel a little lighter on your feet.\r\n");
          }

          affect_strip_afn(ch, gafn_obesity); 

          if ( new_count > 0 ) {

            af.type		= SKILL_UNDEFINED;
            af.afn		= gafn_obesity;
            af.level		= MAX_LEVEL;
            af.duration		= -1;
            af.bitvector		= 0;

            af.location		= APPLY_STR;
            af.modifier		= -1 * new_count;
            affect_to_char( ch, &af );

            af.location		= APPLY_CON;
            af.modifier		= -1 * new_count;
            affect_to_char( ch, &af );

            af.location		= APPLY_DEX;
            af.modifier		= -1 * new_count;
            affect_to_char( ch, &af );

            af.location		= APPLY_HIT;
            af.modifier		= -1 * dice( new_count, 4);
            affect_to_char( ch, &af );

            af.location		= APPLY_MOVE;
            af.modifier		= -1 * dice(2 * new_count, 6);
            affect_to_char( ch, &af );
          }
        }

        break; 

      default:

        break;

    }
   
   /* Update and notify... */

    ch->condition[icond] = new_value;

    if ( msg != NULL ) {
      send_to_char(msg, ch);
    }

   /* The very drunk may fall asleep... */
 
    if ( ch->condition[COND_DRUNK] > COND_STUPOR
    && ch->position > POS_SLEEPING ) {

      send_to_char("{MYou pass out in a drunken stupor!{x\r\n", ch);
  
      set_activity( ch, POS_SLEEPING, "sleeping in a drunken stupor.",
                                                            ACV_NONE, NULL );
    }

   /* All done... */

    return;
}


void mob_fast_update() {
CHAR_DATA *ch, *ch_next;

   /* Examine all mobs (and players)... */

    ch = char_list;

    while ( ch != NULL ) {

     /* Get next mob incase this one dies... */
     
      ch_next = ch->next;

     /* Check we're somewhere sensible... */

      if ( ch->in_room == NULL ) {
        ch = ch_next;
        continue;
      }

     /* Send event if required... */

      if (IS_SET(ch->time_wev, SUB_PULSE_1)) {
        issue_update_wev(ch, WEV_PULSE, WEV_PULSE_1, 0);
      }

     /* Run any 'ready' enqueued commands... */

      if ( ch->mcb != NULL ) {

        ch->mcb->delay -= 1;

        while ( ch->mcb != NULL
             && ch->mcb->delay < 1 ) {
 
          execute_mcb(ch);
        }
      }

     /* NB ch might be invalid at this point if the mob got itself killed */
     /* But it will still point to a valid char_data... */

     /* Next mob... */ 

      ch = ch_next;
    }

    return;
}


/* Make a mob wander around... */

void mob_wander(CHAR_DATA *ch) {
EXIT_DATA *pexit;
ROOM_INDEX_DATA *dest;

   int door;

  /* Do we wander? (1 in 32 - approx once every 90 seconds) */

   if (number_bits(5) != 0 ) return;

  /* Which way shall we go (10 in 16 chance of valid door - 140 secs)... */

   door = (number_mm() & (15));

   if (door >= DIR_MAX) return;

  /* Is there a door there? */

   pexit = ch->in_room->exit[door];

   if (pexit == NULL) return;

  /* Can the mob see it? */

   if (!exit_visible(ch, pexit)) return;

  /* Is it open? */

   if (IS_SET(pexit->exit_info, EX_CLOSED)) return;

  /* Where does it go? */

   dest = get_exit_destination(ch, ch->in_room, pexit, TRUE);

   if (dest == NULL) return;

  /* Can the mob go there? */

   if ( !can_see_room(ch, dest)) return;

   if ( IS_NPC(ch) ) {

     if ( IS_SET(dest->room_flags, ROOM_PRIVATE )
       || IS_SET(dest->room_flags, ROOM_NO_MOB )
       || IS_SET(dest->room_flags, ROOM_SAFE )
       || IS_SET(dest->room_flags, ROOM_GODS_ONLY )
       || IS_SET(dest->room_flags, ROOM_IMP_ONLY )
       || IS_SET(dest->room_flags, ROOM_SOLITARY) ) {
       return;
     }
   }

   if ( IS_SET(ch->act, ACT_STAY_AREA)
   && ch->in_room->area != dest->area ) {
        return;
   } 

   if ( IS_SET(ch->act, ACT_STAY_SUBAREA)
   && ( ch->in_room->area != dest->area
       || ch->in_room->subarea != dest->subarea )) {
        return;
   } 

  /* Hooray! We get to move! */
     
   move_char( ch, door, FALSE );

  /* All done... */

   return; 
 }


/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update( void ){
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool rep = FALSE;

    /* Examine all mobs. */
    for ( ch = char_list; ch != NULL; ch = ch_next )    {
	ch_next = ch->next;

          if ( !IS_NPC(ch) 
          || ch->in_room == NULL 
          || IS_AFFECTED(ch,AFF_CHARM)
          || ch->position <= POS_STUNNED ) {
	    continue;
        }

          if (ch->in_room->area->empty 
          && !IS_SET(ch->act,ACT_UPDATE_ALWAYS)
          && ch->pIndexData->pShop == NULL
          && ch->spec_fun == 0) continue;
        
       /* Send event if required... */

        if (IS_SET(ch->off_flags, ASSIST_GUARD) && !ch->fighting) {
               CHAR_DATA *bystander;

                if (number_percent() < 5) {
                    for(bystander = ch->in_room->people; bystander; bystander = bystander->next_in_room) {
                         if (IS_NPC(bystander)) continue;
                         if (bystander->fighting) continue;
 
                         if (get_eq_char(bystander, WEAR_WIELD)  || get_eq_char(bystander, WEAR_WIELD2)) {
                              do_say(ch, "Better put those weapons away, before you hurt someone!");
                              break;
                         }
                    }
                }
        }

        if (IS_SET(ch->time_wev, SUB_PULSE_4)) issue_update_wev(ch, WEV_PULSE, WEV_PULSE_4, 0);

       /* Do nothing else if queued commands pending... */

        if (ch->mcb != NULL) {
              if (ch->mcb->delay < 3) continue;
        } 

        /* Chat time... */

         if ( ch->triggers != NULL
         && ch->triggers->chat != NULL ) {
             if (do_chat(ch, "")) continue;
         }

	/* Examine call for special procedure */
	if ( ch->spec_fun != 0 ) {
    	    if ((*ch->spec_fun) ( ch )) continue;
	}

	/* That's all for sleeping / busy monster, and empty zones */

        if (ch->fighting != NULL
        && ch->pIndexData->pShop != NULL) {
               	for ( obj = ch->carrying; obj != NULL; obj = obj_next )	{
	    obj_next = obj->next_content;
                    obj_from_char(obj);
                    extract_obj(obj);
                }
        }

        rep = FALSE;
        if (ch->pIndexData->pShop != NULL
        && ch->spec_fun == 0) {
               	for (obj = ch->carrying; obj != NULL; obj = obj_next )	{
	    obj_next = obj->next_content;

                    if (obj->owner) {
                        if (obj->item_type == ITEM_FIGURINE
                        && obj->value[2] == FALSE) {
                            if (number_percent() < 5) {
                                obj->cost += obj->level/2 +1;
                                if (obj->cost > 50000) {
                                     obj_from_char(obj);
                                     extract_obj(obj);
                                }
                            }                               
                        } else {
                            if (!rep
                            && obj->condition < 100) {
                                if (number_percent() < (3*ch->level - obj->level) / 6
                                || number_percent() <= 5) {
                                     obj->condition++;
                                     rep = TRUE;
                                }
                            }
                        }
                    }
                }         
        }

        if ( ch->position != POS_STANDING ) continue;
        
        /* MOBprogram random trigger */
        if ( ch->in_room->area->nplayer > 0 ) {
             mprog_random_trigger( ch );
             if ( ch->position < POS_STANDING ) continue;
        }


	/* Scavenge */
	if ( IS_SET(ch->act, ACT_SCAVENGER)
	  && ch->in_room->contents != NULL
	  && number_bits( 6 ) == 0 ) {
	    OBJ_DATA *oobj;
	    OBJ_DATA *obj_best;
	    int max;

	    max         = 1;
	    obj_best    = 0;
	    for ( oobj = ch->in_room->contents; oobj; oobj = oobj->next_content ) {
	          if ( CAN_WEAR(oobj, ITEM_TAKE) 
                          && can_loot(ch, oobj)
                          && oobj->cost > max  
                          && oobj->cost > 0) {
		    obj_best    = oobj;
		    max         = oobj->cost;
	          }
	    }

	    if ( obj_best ) {
		obj_from_room( obj_best );
		obj_to_char( obj_best, ch );
		act( "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
	    }
	}


       /* Wander */

        if (!IS_SET(ch->act, ACT_SENTINEL)
        && !ch->master) {
          mob_wander(ch);

         /* Continue if it is no longer standing... */
          if ( ch->position < POS_STANDING ) continue;
        }

    }
    return;
}


/* Moon phase transition... */

 static int moon_phase[28] = {
 
   MOON_NEW, MOON_NEW,
   MOON_CRESCENT, MOON_CRESCENT, MOON_CRESCENT,
   MOON_HALF, MOON_HALF, MOON_HALF, MOON_HALF,
   MOON_3Q, MOON_3Q,
   MOON_FULL, MOON_FULL, MOON_FULL, MOON_FULL,
   MOON_3Q, MOON_3Q,
   MOON_HALF, MOON_HALF, MOON_HALF, MOON_HALF,
   MOON_CRESCENT, MOON_CRESCENT, MOON_CRESCENT,
   MOON_NEW, MOON_NEW,

 }; 

/* Check everyone is in the right room... */

void check_day_night() {

  CHAR_DATA *ch;

  ROOM_INDEX_DATA *room, *room2;
 
  ch = char_list;

  while (ch != NULL) {

    room = ch->in_room;

    if ( room != NULL ) {

      if ( IS_SET(time_info.flags, TIME_NIGHT) ) {

        if ( room->night != 0) {  
          room2 = get_room_index(room->night);

          if (room2 != NULL) {
            char_from_room(ch);
            char_to_room(ch, room2);
            do_look(ch, "auto");
          }
        }
      } else if ( IS_SET(time_info.flags, TIME_DAY) ) {

        if ( room->day != 0) {  
          room2 = get_room_index(room->day);

          if (room2 != NULL) {
            char_from_room(ch);
            char_to_room(ch, room2);
            do_look(ch, "auto");
          }
        }
      }
    }

    ch = ch->next;
  }

  return;
}

// -----------------------------------------------------------------------------
//
//   Work out the new weather that applies in each area...
//
// -----------------------------------------------------------------------------

int weather_table[CLIMATE_MAX][SEASON_MAX][SKY_MAX] = {

 /* Climate: European... */

    /* Cloudless          Cloudy             Raining          Lightning */   
  {
    { WEATHER_SUNNY_DAY, WEATHER_CLOUDY,    WEATHER_RAINING, WEATHER_STORM    },
    { WEATHER_SUNNY_DAY, WEATHER_SUNNY_DAY, WEATHER_CLOUDY,  WEATHER_RAINING  },
    { WEATHER_SUNNY_DAY, WEATHER_CLOUDY,    WEATHER_RAINING, WEATHER_STORM    },
    { WEATHER_SUNNY_DAY, WEATHER_CLOUDY,    WEATHER_SNOWING, WEATHER_BLIZZARD }
  },

 /* Climate: Desert... */

    /* Cloudless          Cloudy             Raining          Lightning */
  {
    { WEATHER_SUNNY_DAY, WEATHER_SUNNY_DAY, WEATHER_SUNNY_DAY, WEATHER_CLOUDY    },
    { WEATHER_SUNNY_DAY, WEATHER_SUNNY_DAY, WEATHER_SUNNY_DAY, WEATHER_SUNNY_DAY },
    { WEATHER_SUNNY_DAY, WEATHER_SUNNY_DAY, WEATHER_SUNNY_DAY, WEATHER_SUNNY_DAY },
    { WEATHER_SUNNY_DAY, WEATHER_SUNNY_DAY, WEATHER_CLOUDY,    WEATHER_RAINING   },
  },

 /* Climate: Rainy... */

    /* Cloudless          Cloudy             Raining          Lightning */   
  {
    { WEATHER_CLOUDY,    WEATHER_RAINING, WEATHER_RAINING, WEATHER_STORM    },
    { WEATHER_SUNNY_DAY, WEATHER_CLOUDY,  WEATHER_RAINING, WEATHER_RAINING  },
    { WEATHER_CLOUDY,    WEATHER_RAINING, WEATHER_RAINING, WEATHER_STORM    },
    { WEATHER_CLOUDY,    WEATHER_SNOWING, WEATHER_SNOWING, WEATHER_BLIZZARD }
  },

 /* Climate: Arctic... */

    /* Cloudless          Cloudy             Raining          Lightning */   
  {
    { WEATHER_SUNNY_DAY, WEATHER_CLOUDY,  WEATHER_SNOWING, WEATHER_BLIZZARD },
    { WEATHER_SUNNY_DAY, WEATHER_CLOUDY,  WEATHER_SNOWING, WEATHER_BLIZZARD },
    { WEATHER_SUNNY_DAY, WEATHER_CLOUDY,  WEATHER_SNOWING, WEATHER_BLIZZARD },
    { WEATHER_CLOUDY,    WEATHER_SNOWING, WEATHER_SNOWING, WEATHER_BLIZZARD }
  }

};
    

void update_area_weather() {
 
  AREA_DATA *area;

  area = area_first;

  while (area != NULL) {

   /* Calculate the new weather... */

    area->old_weather = area->weather;
    area->weather = weather_table[area->climate][time_info.season][weather_info.sky];

   /* Calculate surface covering... */

    area->surface = SURFACE_CLEAR;

    switch (area->climate) {

      case CLIMATE_EUROPEAN:
      case CLIMATE_RAINY:
 
        if ( area->weather == WEATHER_RAINING
          || area->old_weather == WEATHER_RAINING ) {
          area->surface = SURFACE_PUDDLES;
        }

        if ( area->weather == WEATHER_STORM
          || area->old_weather == WEATHER_STORM ) {
          area->surface = SURFACE_PUDDLES;
        }

        if ( area->weather == WEATHER_SNOWING
          || area->old_weather == WEATHER_SNOWING ) {
          area->surface = SURFACE_SNOW;
        }

        if ( area->weather == WEATHER_BLIZZARD
          || area->old_weather == WEATHER_BLIZZARD ) {
          area->surface = SURFACE_DEEP_SNOW;
        }

        break;

      case CLIMATE_DESERT:

        if ( area->weather == WEATHER_STORM
          || area->old_weather == WEATHER_STORM ) {
          area->surface = SURFACE_PUDDLES;
        }

        if ( area->weather == WEATHER_SNOWING
          || area->old_weather == WEATHER_SNOWING ) {
          area->surface = SURFACE_SNOW;
        }

        if ( area->weather == WEATHER_BLIZZARD
          || area->old_weather == WEATHER_BLIZZARD ) {
          area->surface = SURFACE_DEEP_SNOW;
        }

        break;

      case CLIMATE_ARCTIC:

        area->surface = SURFACE_SNOW;

        if ( area->weather == WEATHER_BLIZZARD
          || area->old_weather == WEATHER_BLIZZARD ) {
          area->surface = SURFACE_DEEP_SNOW;
        }

        break;

      default:
        area->surface = SURFACE_CLEAR;
        break; 
    }

   /* All done... */

    area = area->next;
  }

  return;
}
 
// -----------------------------------------------------------------------------
//
//   Work out how the weather changes
//
// -----------------------------------------------------------------------------

/* ->dark, ->dawn, ->day, ->dusk */

char weather_time[WEATHER_MAX][SUN_MAX][80] = {

 /* Weather: None... */ 

   { "You somehow feel that is it night.\r\n", 
     "You somehow feel that it is close to dawn.\r\n",
     "You somehow feel that the sun has risen.\r\n",
     "You somehow feel that the sun is setting.\r\n" },

 /* Weather: Sunny day... */ 

   { "The sky above is dark and clear.\r\n",
     "The day has begun.\r\n", 
     "The sun rises in the eastern sky.\r\n",
     "The evening sun lights the western sky red.\r\n" },

 /* Weather: Clear Night... */ 

   { "The sky above is dark and clear.\r\n",
     "The day has begun.\r\n", 
     "The sun rises in the eastern sky.\r\n",
     "The evening sun lights the western sky red.\r\n" },

 /* Weather: Cloudy... */ 

   { "Stars twinkle between the clouds.\r\n",
     "The eastern clouds start to get lighter.\r\n", 
     "Dimly, the sun appears behind the clouds.\r\n", 
     "The sun sets in glorious colors through the clouds.\r\n" },

 /* Weather: Raining... */

   { "The rain continues to fall in the dark of night.\r\n",
     "The sky seems to be getting a little lighter.\r\n",
     "A glow forms in the east through the rain.\r\n",
     "The warmth of day begins to fade as the sun sets.\r\n" },
     
 /* Weather: Storm... */

   { "The storm rages on into the dark of night.\r\n",
     "The sky seems to be getting a little lighter.\r\n",
     "Through the storm you can see a dim glow to the east.\r\n",
     "Lightning accents the now darkening sky.\r\n" },

 /* Weather: Snowing... */

   { "The snow continues to fall in the cold darkness of night.\r\n",
     "The sky seems to be getting a little lighter.\r\n",
     "A glow forms in the east through the snow.\r\n",
     "It seems to be getting darker.\r\n" },
     
 /* Weather: Blizzard... */

   { "The blizzard rages on into the icy darkness of night.\r\n",
     "The sky seems to be getting a little lighter.\r\n",
     "Through the howling snow you can see a dim glow to the east.\r\n",
     "It seems to be getting darker.\r\n" },

};

char weather_change[WEATHER_MAX][WEATHER_MAX][80] = {

 /* Weather: None->... */ 

   { "",
     "You can see the sun!\r\n",
     "You can see the stars!\r\n",
     "You can see some clouds!\r\n",
     "It starts raining!\r\n",
     "A storm starts blowing!\r\n",
     "It starts snowing!\r\n",
     "A howling blizzard sets in!\r\n"
   },
 
 /* Weather: Sunny_Day->... */ 

   { "The sun stops shining!\r\n",
     "",
     "You can see the stars!\r\n",
     "Clouds gather in the sky.\r\n",
     "Clouds gather and it starts raining.\r\n",
     "A sudden storm sweeps in!\r\n",
     "Clouds gather and it starts snowing.\r\n",
     "A howling blizzard sweeps in!\r\n"
   },
 
 /* Weather: Clear_Night->... */ 

   { "The stars stop shining!\r\n",
     "You can see the sun!\r\n",
     "",
     "Clouds gather in the sky.\r\n",
     "Clouds gather and it starts raining.\r\n",
     "A sudden storm sweeps in!\r\n",
     "Clouds gather and it starts snowing.\r\n",
     "A howling blizzard sweeps in!\r\n"
   },
 
 /* Weather: Cloudy->... */ 

   { "The clouds vanish!\r\n",
     "The sky clears.\r\n",
     "The sky clears.\r\n",
     "",
     "The clouds get darker and it starts raining.\r\n",
     "A sudden storm sweeps in!\r\n",
     "The clouds get darker and it starts snowing.\r\n",
     "A howling blizzard sweeps in!\r\n"
   },
 
 /* Weather: Raining->... */ 

   { "The rain vanishes!\r\n",
     "The rain stops and the clouds clear.\r\n",
     "The rain stops and the clouds clear.\r\n",
     "The rain stops.\r\n",
     "",
     "The wind picks up and the rain turns into a raging storm!\r\n",
     "It gets colder, the rain turn to hail and snow!\r\n",
     "It gets colder and windier, the rain turns into a howling blizzard!\r\n"
   },
 
 /* Weather: Storm->... */ 

   { "The storm vanishes!\r\n",
     "The storm stops and the clouds clear.\r\n",
     "The storm stops and the clouds clear.\r\n",
     "The storm stops.\r\n",
     "The storm eases into heavy rain.\r\n",
     "",
     "It gets colder, the storm eases into hail and snow.\r\n",
     "It gets colder and the stom becomes a howling blizzard!\r\n"
   },
 
 /* Weather: Snow->... */ 

   { "The snow vanishes!\r\n",
     "The snow stops falling and the clouds clear.\r\n",
     "The snow stops falling and the clouds clear.\r\n",
     "The snow stops falling.\r\n",
     "The snow turns to freezing cold rain.\r\n",
     "The wind picks up and the snow turns into an icy, raging storm!\r\n",
     "",
     "The wind picks up and the snow fall becomes a howling blizzard!\r\n"
   },
 
 /* Weather: Blizzard->... */ 

   { "The blizzard vanishes!\r\n",
     "The blizzard eases and the clouds clear.\r\n",
     "The blizzard eases and the clouds clear.\r\n",
     "The blizzard stops.\r\n",
     "The blizzard eases and turns to freezing cold rain.\r\n",
     "The blizzard turns into an icy, raging storm!\r\n",
     "The blizzard eases into just a snow fall!\r\n",
     ""
   }
 
};

/* Sky transition matrix.  Values are chance of transition...  */

int sky_trans[SEASON_MAX][SKY_MAX][SKY_MAX] = {

 /* Season: Spring */
                    {  
 /* Sky: Cloudless */ { 40, 40, 15,  5 },
 /* Sky: Cloudy    */ { 20, 40, 30, 10 },
 /* Sky: Raining   */ { 10, 30, 40, 20 },
 /* Sky: Lightning */ {  5, 35, 30, 30 }
                    },
 /* Season: Summer */
                    {
 /* Sky: Cloudless */ { 50, 30, 15,  5 },
 /* Sky: Cloudy    */ { 40, 30, 20, 10 },
 /* Sky: Raining   */ { 20, 30, 30, 20 },
 /* Sky: Lightning */ { 15, 25, 30, 30 }
                    },
 /* Season: Fall */
                    {
 /* Sky: Cloudless */ { 50, 30, 15,  5 },
 /* Sky: Cloudy    */ { 30, 40, 20, 10 },
 /* Sky: Raining   */ { 10, 30, 40, 20 },
 /* Sky: Lightning */ {  5, 35, 30, 30 }
                    },
 /* Season: Winter */
                    {
 /* Sky: Cloudless */ { 40, 40, 10, 10 },
 /* Sky: Cloudy    */ { 20, 40, 20, 20 },
 /* Sky: Raining   */ { 10, 30, 30, 30 },
 /* Sky: Lightning */ {  5, 30, 25, 40 }
                    }
};

/* Northern hemisphere sunrise/sunset times... */ 

int weather_times[MONTHS_IN_YEAR][4] = {

  /* Janruary  */ { 7, 8, 17, 18 }, 
  /* February  */ { 6, 7, 17, 18 },
  /* March     */ { 6, 7, 18, 19 },
  /* April     */ { 5, 6, 18, 19 },
  /* May       */ { 5, 6, 19, 20 },
  /* June      */ { 4, 5, 20, 21 },
  /* July      */ { 5, 6, 19, 20 },
  /* August    */ { 5, 6, 18, 19 },
  /* September */ { 6, 7, 18, 19 },
  /* October   */ { 6, 7, 17, 18 },
  /* November  */ { 7, 8, 17, 18 },
  /* December  */ { 8, 9, 16, 17 }
    
};

/* Update the weather... */

void weather_update( bool random ) {
DESCRIPTOR_DATA *d;
CHAR_DATA *ch;
int weather, old_weather;
int roll, new_sky;
bool time_change;
time_change = FALSE;

   /* Weather change... */

    if (random) {

      weather_info.duration -= 1;

      if (weather_info.duration <= 0) {

        roll = dice(1,100);

        new_sky = SKY_CLOUDLESS;

        while ( new_sky < SKY_LIGHTNING 
        && sky_trans[time_info.season][weather_info.sky][new_sky] < roll) {

          new_sky += 1;

         roll -= sky_trans[time_info.season][weather_info.sky][new_sky];

        } 

        weather_info.sky = new_sky;

        switch (new_sky) {
          case SKY_CLOUDLESS:
            weather_info.duration = 3 + dice(2,6);
            break;

          case SKY_CLOUDY:
            weather_info.duration = 4 + dice(3,4);
            break;

          case SKY_RAINING:
            weather_info.duration = 2 + dice(2,4);
            break;

          case SKY_LIGHTNING:
            weather_info.duration = 1 + dice(1,6);
            break;
 
          default:
            weather_info.duration = 1; 
            break;
        }
      }

     /* Dawn */ 

      if ( time_info.hour == weather_times[time_info.month][0] ) {

        weather_info.sunlight = SUN_RISE;

        weather_info.natural_light = LIGHT_DAWN;

        REMOVE_BIT(time_info.flags, TIME_NIGHT);
        SET_BIT(   time_info.flags, TIME_DAWN );

        issue_time_wev(SUB_TIME_DAWN, WEV_TIME, WEV_TIME_DAWN, time_info.hour);

        time_change = TRUE;
      }

     /* Sunrise */

      if ( time_info.hour == weather_times[time_info.month][1] ) {

        weather_info.sunlight = SUN_LIGHT;

        weather_info.natural_light = LIGHT_SUNLIGHT;

        REMOVE_BIT(time_info.flags, TIME_DAWN);
        SET_BIT(   time_info.flags, TIME_DAY );

        issue_time_wev(SUB_TIME_SUNRISE, WEV_TIME, WEV_TIME_SUNRISE, time_info.hour);

        time_change = TRUE;
      }

     /* Dusk */

      if ( time_info.hour == weather_times[time_info.month][2] ) {

        weather_info.sunlight = SUN_SET;

        weather_info.natural_light = LIGHT_DUSK;

        REMOVE_BIT(time_info.flags, TIME_DAY  );
        SET_BIT(   time_info.flags, TIME_DUSK );

        issue_time_wev(SUB_TIME_SUNSET, WEV_TIME, WEV_TIME_SUNSET, time_info.hour);

        time_change = TRUE;
      }

     /* Sunset */

      if ( time_info.hour == weather_times[time_info.month][3] ) {

        weather_info.sunlight = SUN_DARK;

        if (weather_info.moon < MOON_HALF) {
          weather_info.natural_light = LIGHT_MOON_DARK;
        } else {
          weather_info.natural_light = LIGHT_MOON_FULL;
        }

        REMOVE_BIT(time_info.flags, TIME_DUSK );
        SET_BIT(   time_info.flags, TIME_NIGHT);

        issue_time_wev(SUB_TIME_DUSK, WEV_TIME, WEV_TIME_DUSK, time_info.hour);

        time_change = TRUE;
      }

     /* Work out what phase is the moon in... */

      weather_info.moon = moon_phase[time_info.moon];

    }

   /* Update the weather in all the areas... */

    update_area_weather();

   /* Tell everyone about the weather... */

    d = descriptor_list;

    while ( d != NULL) {

      if ( d->connected == CON_PLAYING ) {

        ch = d->character;

        if ( ch->in_room != NULL
          && ch->in_room->area != NULL
          && is_outside(ch)
          && IS_AWAKE(ch) 
          && IS_SET(ch->notify, NOTIFY_WEATHER) 
          && !IS_AFFECTED(ch, AFF_BLIND)) {

         /* Find out their weather... */

          weather = ch->in_room->area->weather; 

          if ( weather != WEATHER_NONE ) {

            if (time_change) {
                send_to_char( weather_time[weather][weather_info.sunlight], ch);
            } 

            old_weather = ch->in_room->area->old_weather;

            if ( weather != old_weather ) {
                 send_to_char( weather_change[old_weather][weather], ch);
            }
          }
        }
      } 
 
      d = d->next;
    }

   /* Activate any automatic day/night switches... */

    if (time_change) check_day_night();
    
   /* All done... */

    return;
}


// -----------------------------------------------------------------------------
//
//   Keep track of time and seasons
//
// -----------------------------------------------------------------------------

/* Northern hemisphere month->season mapping... */

int seasons[MONTHS_IN_YEAR] = { 

  SEASON_WINTER, SEASON_WINTER, SEASON_SPRING, 
  SEASON_SPRING, SEASON_SPRING, SEASON_SUMMER,
  SEASON_SUMMER, SEASON_SUMMER, SEASON_FALL,
  SEASON_FALL,   SEASON_FALL,   SEASON_WINTER

};

/* Update the time... */

void time_update( void ) {

  /* An hour has passed... */

    time_info.hour += 1;

   /* Hour 24 means we start a new day... */

    if (time_info.hour == 24) {

	time_info.hour = 0;

	time_info.day++;
 
        time_info.moon++;

       /* Update week number... */

        time_info.week = (time_info.month * DAYS_IN_MONTH) + time_info.day;
        time_info.week /= DAYS_IN_WEEK;

       /* Update the time file... */
      
	time_file = fopen(TIME_FILE,"w+");

	if (time_file == NULL) {
	  log_string ("creation of new time_file failed");
	} else {
	  int total;	
	  total = fprintf (time_file, "%d %d %d %d", time_info.hour, time_info.day, time_info.month, time_info.year);
   	  if (total<4)
	    log_string("failed fprintf to time_file");

	  fclose(time_file);
	}

       /* Moon 28 means we being a new moon... */

        if (time_info.moon == 28) time_info.moon = 0;

    }

   /* See if we begin a new month... */
   
    if ( time_info.day   >= DAYS_IN_MONTH ) {

	time_info.day = 0;
	time_info.month++;

       /* See if we begin a new year... */

        if ( time_info.month >= MONTHS_IN_YEAR ) {
	    time_info.month = 0;
	    time_info.year++;
        }

       /* Update the season... */

        time_info.season = seasons[time_info.month];

    }

   /* Send out time events... */

    issue_time_wev(SUB_TIME_HOUR, WEV_TIME, WEV_TIME_HOUR, time_info.hour);
    if (time_info.hour == 0) issue_time_wev(SUB_TIME_DAY, WEV_TIME, WEV_TIME_DAY, time_info.day);
    
   /* Weather and dawn/day/dusk/night changes... */

    weather_update(TRUE);
    check_muddeath  ( );

   /* All done... */

    return;
}

char insane_actions[][50] = {

 /* Edgy actions... (0..2), sanity 16..31 */

  "shiver", "worry", "peer",

 /* Nervous actions... (3,4), sanity 8..15 */

  "say I wonder where they are?", "insane",

 /* Paranoid actions... (5-8), sanity 4-7 */

  "stare", "point", "snicker", "say You're one of them!", 

 /* Crazy actions... (9-16), sanity 2,3 */ 
 
  "flee",  "cringe", "giggle",   "mutter", 
  "snarl", "babble", "threaten", "duck",

 /* insane actions... (17..32), sanity 1 */
 
  "run",     "cover",     "cower",   "say I hate you all!",
  "pleed",   "scream",    "grumble", "say They're coming! They're coming!",
  "slobber", "claw self", "aargh",   "howl",
  "drool",   "cackle",    "growl",   "beg"

};


/* See how sane a character is... */

void check_player_sanity(CHAR_DATA *ch) {
int chance, roll;

 /* Reject those who are not insane... */

  if (get_sanity(ch) == 0 || get_sanity(ch) >= 32 ) return;

 /* Sanity... */

  chance = 0;

  if (get_sanity(ch) > 0) {
    int sanity = get_sanity(ch);

    if (sanity < 2) {
      chance = 32;
    } else  if (sanity < 4) {
      chance = 16;
    } else  if (sanity < 8) {
      chance =  8;
    } else  if (sanity < 16) {
      chance =  4;
    } else  if (sanity < 32) {
      chance =  2;
    } 
  }

 /* Work out what happens... */

  roll = number_bits(6);

 /* See if insanity strikes... */

  if (roll > chance) return;
 
 /* Ok, now work out the effects... */

  enqueue_mob_cmd(ch, insane_actions[roll], 0, SAN_CMD_ID);

 /* All done... */

  return;
}


 /*
   * Short Char Update
   */
void char_update_short( void ) {
CHAR_DATA *ch;
CHAR_DATA *ch_next;
DESCRIPTOR_DATA *d;
int cond, img;
bool snooping = FALSE;
bool disok;

    for ( ch = char_list; ch != NULL; ch = ch_next ) {
      ch_next = ch->next;

       if (IS_SET(ch->plr, PLR_AFK)) continue;

       if ( ch->position >= POS_STUNNED ) {

       /* Heal damage, recover mana and move... */

        if ( ch->hit  != ch->max_hit  
          || ch->mana != ch->max_mana
          || ch->move != ch->max_move ) { 

         /* Set an injured players alarm clock... */ 

          if ( !IS_NPC(ch) 
            && ch->hit != ch->max_hit ) {
            SET_BIT(ch->plr, PLR_INJURED);
          }

     if (IS_IMMORTAL(ch)) {
        int rn;
        rn = get_cult_rn(ch->name);
        if (rn > 0) {
           if (cult_array[rn].power > 0) {
                if ( ch->mana < ch->max_mana ) {
                   img=mana_gain(ch);
                   if (ch->mana+img > ch->max_mana) img=ch->max_mana - ch->mana;
                    ch->mana += img;
                    cult_array[rn].power -= img;
                } else {                
                    ch->mana = ch->max_mana;
                } 
                if ( ch->hit < ch->max_hit ) {
                   img=hit_gain(ch);
                   if (ch->hit+img > ch->max_hit) img=ch->max_hit - ch->hit;
                    ch->hit += img;
                    cult_array[rn].power -= img;
                } else {                
                    ch->hit = ch->max_hit;
                } 
                if ( ch->move < ch->max_move ) {
                   img=move_gain(ch);
                   if (ch->move+img > ch->max_move) img=ch->max_move - ch->move;
                    ch->move += img;
                    cult_array[rn].power -= img;
                } else {                
                    ch->move = ch->max_move;
                } 
           }
           cult_array[rn].power = UMAX(cult_array[rn].power, 0);
        } else {
            if ( ch->mana < ch->max_mana ) ch->mana += mana_gain(ch);
            else ch->mana = ch->max_mana;
        
            if ( ch->hit < ch->max_hit ) ch->hit  += hit_gain(ch);
            else ch->hit = ch->max_hit;

            if ( ch->move < ch->max_move ) ch->move += move_gain(ch);
            else ch->move = ch->max_move;
        }
     } else {
         if ( ch->mana < ch->max_mana ) ch->mana += mana_gain(ch);
         else ch->mana = ch->max_mana;

         if ( ch->hit < ch->max_hit ) ch->hit  += hit_gain(ch);
         else ch->hit = ch->max_hit;

         if ( ch->move < ch->max_move ) ch->move += move_gain(ch);
         else ch->move = ch->max_move;
      }
  }

  if (IS_ARTIFACTED(ch, ART_ABHOTH)) {
        int rn = get_cult_rn("abhoth");
        if (rn > 0) {
           if (cult_array[rn].power > 0) {
                if (ch->mana < ch->max_mana ) {
                   img=mana_gain(ch);
                   if (ch->mana+img > ch->max_mana) img=ch->max_mana - ch->mana;
                    ch->mana += img;
                    cult_array[rn].power -= img;
                } else {                
                    ch->mana = ch->max_mana;
                } 
                if ( ch->hit < ch->max_hit ) {
                   img=hit_gain(ch);
                   if (ch->hit+img > ch->max_hit) img=ch->max_hit - ch->hit;
                    ch->hit += img;
                    cult_array[rn].power -= img;
                } else {                
                    ch->hit = ch->max_hit;
                } 
                if ( ch->move < ch->max_move ) {
                   img=move_gain(ch);
                   if (ch->move+img > ch->max_move) img=ch->max_move - ch->move;
                    ch->move += img;
                    cult_array[rn].power -= img;
                } else {                
                    ch->move = ch->max_move;
                } 
           }
           cult_array[rn].power = UMAX(cult_array[rn].power, 0);
        }
  }

       /* Injured players get an alarm clock... */
 
        if ( !IS_NPC(ch)
          &&  IS_SET(ch->plr, PLR_INJURED)
          &&  ch->hit  == ch->max_hit
          &&  ch->mana == ch->max_mana
          &&  ch->move == ch->max_move ) {

         /* Wake up if asleep... */

          for (d = descriptor_list; d != NULL; d = d->next ) {
                  if ( d->snoop_by == ch->desc ) snooping = TRUE;
          }

          if ((ch->position == POS_SLEEPING
              || ch->position == POS_RESTING)
          && (!snooping
              || IS_IMMORTAL(ch))) {
              set_activity( ch, POS_STANDING, NULL, ACV_NONE, NULL );
              if (ch->on_furniture != NULL) remove_furnaff(ch, ch->on_furniture);
              ch->on_furniture = NULL;
              send_to_char("{CYou feel wide-awake!\r\n{x", ch); 
          }

         /* Remove injured bit anyway... */
           
          REMOVE_BIT(ch->plr, PLR_INJURED);
        }
      }

     /* Send event if required... */

      if (IS_SET(ch->time_wev, SUB_PULSE_5)) {
        issue_update_wev(ch, WEV_PULSE, WEV_PULSE_5, 0);
      } 

     /* Sanity check... */

      if ( ch->position == POS_FIGHTING 
        && ch->activity != ACV_CASTING ) {

        if ( ch->fighting == NULL)  {
             stop_fighting(ch, TRUE);
        } else {
             if (ch->in_room != ch->fighting->in_room) {
                      if (IS_SET(ch->in_room->room_flags, ROOM_VEHICLE)
                      || IS_SET(ch->fighting->in_room->room_flags, ROOM_VEHICLE)) {              
                           
                                disok = FALSE;
                                if(ch->in_room->exit[DIR_OUT] != NULL) {
                                       if (ch->in_room->exit[DIR_OUT]->u1.to_room == ch->fighting->in_room) disok = TRUE;
                                }
                                if(ch->fighting->in_room->exit[DIR_OUT] != NULL) {
                                       if (ch->fighting->in_room->exit[DIR_OUT]->u1.to_room == ch->in_room) disok = TRUE;
                                }
                
       	                if (!disok) {
                                    stop_fighting( ch, FALSE );
                                } else {
                                    if (!IS_SET(ch->fighting->in_room->room_flags, ROOM_VEHICLE)
                                    && IS_NPC(ch->fighting)
                                    && number_percent() < 30) {
                                           act("$N jumps to you into the vehicle!",ch,NULL,ch->fighting,TO_CHAR);
                                           act("$n jumps into the vehicle!",ch->fighting,NULL,NULL,TO_ROOM);
                                           char_from_room(ch->fighting);
                                           char_to_room(ch->fighting, ch->in_room);
                                           SET_BIT(ch->fighting->act, ACT_AGGRESSIVE);
                                    }
                                }
                      } else {
                     	stop_fighting( ch, FALSE );
                      }
             }
        }
      }

     /* Hunger and thirst... */
    
      if (!IS_NPC(ch)) {

       /* 3 pts, nine times per hour = 27 pts/hour */ 

        switch (ch->position) {

          case POS_FIGHTING:
          case POS_STUNNED:
            cond = 5; 
            break;

          case POS_SITTING:
          case POS_RESTING:
            cond = 2;  
            break;

          case POS_SLEEPING:
            cond = 1;
            break;

          default:
            cond = 3;
            break;  
        } 
  
        if ( ch->in_room != NULL ) {

          switch (ch->in_room->sector_type) {

            case SECT_MOUNTAIN:
            case SECT_DESERT:
            case SECT_AIR:
            case SECT_UNDERGROUND:
            case SECT_SMALL_FIRE:
            case SECT_FIRE:
            case SECT_BIG_FIRE:
            case SECT_SPACE:
            case SECT_COLD:
              cond += 2;
              break;
 
            case SECT_HILLS:
            case SECT_FOREST:
            case SECT_MOORS:
              cond += 1;
              break;

            case SECT_WATER_SWIM:
            case SECT_WATER_NOSWIM:
            case SECT_SWAMP:
            case SECT_JUNGLE:
            case SECT_LIGHTNING:
              cond -= 1;
              break; 

            default:
              break;    
          }
        }

       /* Actual range is from 7 pts - fighting in a desert */
       /*                   to 1 pt  - sleeping/resting in a swamp */  

        if (IS_IMMORTAL(ch)) cond = 1;
        cond = UMAX(cond, 1);

        if (ch->in_room) {
            if (!IS_AFFECTED(ch, AFF_ASCETICISM)
            && (!IS_NEWBIE(ch) || (!IS_SET(ch->in_room->room_flags, ROOM_NEWBIES_ONLY) && !IS_SET(ch->in_room->area->area_flags, AREA_NEWBIE)))) {
               gain_condition( ch, COND_FOOD,  -1 * cond );
            }

            if (!IS_SET(ch->act, ACT_VAMPIRE)
            && !IS_AFFECTED(ch, AFF_ASCETICISM)
            && (!IS_NEWBIE(ch) || (!IS_SET(ch->in_room->room_flags, ROOM_NEWBIES_ONLY) && !IS_SET(ch->in_room->area->area_flags, AREA_NEWBIE)))) {
               gain_condition( ch, COND_DRINK, -1 * cond );
            }
        }

        /*don't annoy imms with food */
        if ((IS_IMMORTAL(ch)
       && !IS_AFFECTED(ch, AFF_INCARNATED))
       || !str_cmp(race_array[ch->race].name,"yithian")) {
            ch->condition[COND_FOOD] = 200;
            ch->condition[COND_DRINK] = 100;
        }

        if (IS_SET(ch->act, ACT_VAMPIRE)) {
             ch->condition[COND_DRINK] = 100;
             if (number_percent() < 4
             || ch->pcdata->blood == 0) {
                  if (ch->pcdata->blood > 0) {
                       ch->pcdata->blood--;
                  } else {
                       if (!is_affected(ch, gafn_frenzy) 
                       && !IS_AFFECTED(ch, AFF_BERSERK)
                       && !is_affected(ch, gafn_calm)) {
                            AFFECT_DATA af;

                            af.type 	 = 100;
                            af.afn	 = gafn_frenzy;
                            af.level	 = ch->level / 2;
                            af.duration	 = -1;
                            af.modifier  = ch->level / 8;
                            af.bitvector = 0;

                            af.location  = APPLY_HITROLL;
                            affect_to_char(ch,&af);

                            af.location  = APPLY_DAMROLL;
                            affect_to_char(ch,&af);
                            SET_BIT(ch->parts, PART_FANGS);

                            send_to_char("You are overwhelmed by your thirst!\r\n",ch);
                            act("$n gets a wild look in $s eyes!",ch,NULL,NULL,TO_ROOM);
                       }
                  }
             }
        }
        
        check_player_sanity(ch);
        if (number_percent() < 2) {
            update_align(ch);
            if (get_age(ch) > ch->pcdata->lifespan+1
            && !IS_IMMORTAL(ch)
            && !IS_SET(ch->act, ACT_UNDEAD)
            && !IS_ARTIFACTED(ch, ART_BLESSING_NEPHRAN)) {
                if (update_age(ch)) return;
            }
        }
      }

     /* Check to see if they are still alive... */

      
      if ( ch->max_hit <= 0 ) {
        send_to_char("You have died of hunger and thirst!\r\n", ch);
        raw_kill(ch, TRUE);
      } else {
       
       /* Yep, so tidy up hit, mana and move... */
 
        if (ch->hit > ch->max_hit) ch->hit = ch->max_hit;

        if (ch->mana > ch->max_mana) ch->mana = ch->max_mana;

        if (ch->move > ch->max_move) ch->move = ch->max_move;
      }  
    }

    return;
}


bool check_environment(CHAR_DATA *ch) {
    OBJ_DATA *aobj;
    bool alive = TRUE;

    if (IS_SET(ch->plr, PLR_AFK)) return TRUE;

   /* Void is void... */

    if (IS_AFFECTED(ch, AFF_MORF)) {
       if (!ch->trans_obj) {
           affect_strip_afn( ch, gafn_morf);
           REMOVE_BIT( ch->affected_by, AFF_MORF );
       } else {
           if (ch->trans_obj->in_room != NULL) ch->in_room = ch->trans_obj->in_room;
           if (ch->trans_obj->carried_by != NULL) {
                 if (ch->trans_obj->carried_by->in_room) ch->in_room = ch->trans_obj->carried_by->in_room;
           }
       }
    }

    if (ch->in_room == NULL) return alive;
        
    
/* Moon for Wolves */

   if (IS_SET(ch->act, ACT_WERE)
     && (weather_info.moon == MOON_FULL)
     && (!IS_AFFECTED(ch, AFF_POLY))
     && (ch->race_orig=0)) {
              if ( ( aobj = get_eq_char( ch, WEAR_WIELD ) ) != NULL ) {
                    unequip_char( ch, aobj );
              }  
              if ( ( aobj = get_eq_char( ch, WEAR_WIELD2 ) ) != NULL ) {
                    unequip_char( ch, aobj );
              }  
              if ( ( aobj = get_eq_char( ch, WEAR_SHIELD ) ) != NULL ) {
                    unequip_char( ch, aobj );
              }  
                  send_to_char( "You change into an animal!\r\n", ch );
                  act("$n changes into an animal.",ch,NULL,NULL,TO_ROOM);
                   SET_BIT( ch->affected_by, AFF_POLY );
                   free_string(ch->description_orig);
                   free_string(ch->short_descr_orig);
                   free_string(ch->long_descr_orig);
                   ch->description_orig = str_dup (ch->description);
                   ch->short_descr_orig = str_dup (ch->short_descr);
                   ch->long_descr_orig  = str_dup (ch->long_descr);
                   ch->race_orig=ch->race;

                    free_string(ch->description);
                    free_string(ch->short_descr);
                    free_string(ch->long_descr);

               switch(ch->were_type) {
                     default:
                        ch->description = str_dup("A large hairy wolf is here.\n");
                        ch->race = 31;
                        ch->were_type = 31;
                        ch->short_descr = str_dup ("Wolf");
                        ch->long_descr  = str_dup ("A large hairy wolf is here.\n");
                        free_string(ch->poly_name);
                        ch->poly_name = str_dup("wolf");
                        break;
        
                     case 31:
                        ch->description = str_dup("A large hairy wolf is here.\n");
                        ch->race = ch->were_type;
                        ch->short_descr = str_dup ("Wolf");
                        ch->long_descr  = str_dup ("A large hairy wolf is here.\n");
                        free_string(ch->poly_name);
                        ch->poly_name = str_dup("wolf");
                        break;

                     case 12:
                        ch->description = str_dup("A deep black panther is here.\n");
                        ch->race = ch->were_type;
                        ch->short_descr = str_dup ("Panther");
                        ch->long_descr  = str_dup ("A deep black panther is here.\n");
                        free_string(ch->poly_name);
                        ch->poly_name = str_dup("panther");
                        break;

                     case 17:
                        ch->description = str_dup("A swift grey fox is here.\n");
                        ch->race = ch->were_type;
                        ch->short_descr = str_dup ("Fox");
                        ch->long_descr  = str_dup ("A swift grey dox is here.\n");
                        free_string(ch->poly_name);
                        ch->poly_name = str_dup("fox");
                        break;
               }
       }              
            

    /*Reset Kung Fu*/

    if (!IS_NPC(ch)) {
         if ( ch->condition[COND_DRUNK] <5
         && ch->pcdata->style == STYLE_DRUNKEN_BUDDHA) {
              send_to_char("{CYou really need a drink!{x\r\n", ch);
              ch->pcdata->style = 0;
          }

           if (!IS_AFFECTED(ch,AFF_FLYING) 
           && ch->pcdata->style == STYLE_DRAGON) {
                send_to_char("You feel unable to fight in dragon style!\r\n",ch);
                ch->pcdata->style = 0;
           }

           if (ch->race!=1
           && ch->pcdata->style != STYLE_IRON_FIST) {
                send_to_char("You don't know how to use your techniques with this body.!\r\n",ch);
                ch->pcdata->style = 0;
           }

           /*Things to remove sometime*/

     }

     /* Environmental damage... */ 
    
    if (IS_RAFFECTED(ch->in_room, RAFF_HOLY)) {
        if (IS_EVIL(ch)) {
          alive = alive & env_damage(ch, (-1 * dice(5,10) * ch->alignment)/1000, DAM_HOLY, 
                 "{Y@v2 writhes in agony! [@n0]{x\r\n",
                 "{YThe light burns you! [@n0]{x\r\n");
        }
    }               
     if (!alive) return FALSE;

    if (IS_RAFFECTED(ch->in_room, RAFF_EVIL)) {
        if (IS_GOOD(ch)) {
          alive = alive & env_damage(ch, (dice(5,10) * ch->alignment)/1000, DAM_NEGATIVE, 
                 "{R@v2 chokes and gags! [@n0]{x\r\n",
                 "{RYou choke and gag in the foulness! [@n0]{x\r\n");
        }
     }
     if (!alive) return FALSE;

    if (IS_RAFFECTED(ch->in_room, RAFF_NOBREATHE)) {
        alive = alive & env_damage(ch, dice(3,4), DAM_DROWNING, 
               "{B@v2 is gasping for air! [@n0]{x\r\n",
  	       "{BThere's no air! [@n0]{x\r\n");
    }
     if (!alive) return FALSE;

    if (IS_RAFFECTED(ch->in_room, RAFF_DRAIN)) {
          alive = alive & env_damage(ch, dice(10,10), DAM_NEGATIVE, 
                 "{m@v2 begins to wither! [@n0]{x\r\n",
                 "{mYou feel your energy being drained! [@n0]{x\r\n");
    }
    if (!alive) return FALSE;

    if (IS_RAFFECTED(ch->in_room, RAFF_FATIGUE)
    && ch->move > -30) {
           int dam = number_range(10, 30);

           ch->move -= dam;
           update_pos(ch);
           sprintf_to_char(ch, "{mYou're feeling so sleepy! [%d]{x\r\n", dam);
    }
  
    if (IS_RAFFECTED(ch->in_room, RAFF_DESTRUCTIVE)) {
         harmful_effect(ch, 25, TAR_CHAR_OFFENSIVE, DAM_ACID);
    }


   /* Light for the Vampires */

   if (weather_info.natural_light > LIGHT_DUSK
   && ch->in_room->sector_type !=SECT_INSIDE
   && ch->in_room->sector_type !=SECT_UNDERGROUND
   && !IS_SET(ch->in_room->room_flags, ROOM_DARK) 
   && !IS_SET(ch->in_room->room_flags, ROOM_INDOORS) 
   && !IS_AFFECTED(ch, AFF_DARKNESS)
   && !IS_RAFFECTED(ch->in_room, RAFF_DARKNESS)
   && (IS_SET(ch->act, ACT_VAMPIRE) || IS_SET(ch->vuln_flags, VULN_LIGHT))) {  
     alive = alive & env_damage(ch, dice(5,6), DAM_FIRE, 
      "{r@v2 is {yburned{r by the light! [@n0]{x\r\n",
      "{rThe light {yburns{r you! [@n0]{x\r\n");
   }
   if (!alive) return FALSE;

   if (IS_AFFECTED(ch, AFF_FIRE_SHIELD)
   && number_percent() < 10)                             alive = alive & env_damage(ch, dice(5,6), DAM_FIRE, 
                                                                                 "{R@v2 is {Yburned{R by the flames! [@n0]{x\r\n",
                                                                                 "{RThe flames {Yburn{R you! [@n0]{x\r\n");
   if (!alive) return FALSE;

   if (IS_AFFECTED(ch, AFF_FROST_SHIELD)
   && number_percent() < 10) alive = alive & env_damage(ch, dice(5,6), DAM_COLD, 
                                                                                 "{C@v2 is {Yhurt{C by the frost! [@n0]{x\r\n",
                                                                                 "{CThe frost {Yhurts{C you! [@n0]{x\r\n");
if (!alive) return FALSE;

if (ch->in_room !=NULL) {    
 switch(ch->in_room->sector_type) {

     /* Space causes drowning and cold damage... */

      case SECT_SPACE:

        alive = alive & env_damage(ch, dice(3,4), DAM_DROWNING, 
               "{B@v2 is gasping for air! [@n0]{x\r\n",
  	       "{BThere's no air! [@n0]{x\r\n");
             
        if (alive) {
          alive = alive & env_damage(ch, dice(4,6), DAM_COLD, 
                 "{W@v2 seems very cold! [@n0]{x\r\n",
                 "{WIt is so cold! [@n0]{x\r\n");
        }
  
        break;

     /* Being underwater causes drowning damage... */

      case SECT_UNDERWATER:

        alive = alive & env_damage(ch, dice(3,4), DAM_DROWNING, 
               "{B@v2 is drowning! [@n0]{x\r\n",
  	       "{BThe water isn't good to breath! [@n0]{x\r\n");
               
        break;

     /* Fire burn... */ 

      case SECT_SMALL_FIRE:

        alive = alive & env_damage(ch, dice(3,4), DAM_FIRE, 
               "{R@v2 screams in the flames! [@n0]{x\r\n",
               "{RYou scream in the flames! [@n0]{x\r\n");
               
        break;

      case SECT_FIRE:

        alive = alive & env_damage(ch, dice(5,6), DAM_FIRE, 
               "{R@v2 screams in the flames! [@n0]{x\r\n",
               "{RYou scream in the flames! [@n0]{x\r\n");
               
        break;

      case SECT_BIG_FIRE:

        alive = alive & env_damage(ch, dice(7,8), DAM_FIRE, 
               "{R@v2 screams in the flames! [@n0]{x\r\n",
               "{RYou scream in the flames! [@n0]{x\r\n");
               
        break;

     /* Cold chills... */ 

      case SECT_COLD:

        alive = alive & env_damage(ch, dice(4,6), DAM_COLD, 
               "{W@v2 is suffering from frost bite! [@n0]{x\r\n",
  	       "{WYou feel a deadly chill seeping into your bones! [@n0]{x\r\n");
               
        break;

     /* Acid dissolves... */ 
 
      case SECT_ACID:

        alive = alive & env_damage(ch, dice(2,6), DAM_ACID, 
               "{G@v2 is dissolving! [@n0]{x\r\n",
               "{GYour flesh starts dissolving! [@n0]{x\r\n");
               
        break;
              
     /* Lightning fries... */  

      case SECT_LIGHTNING:

        act("{CLightning stabs the ground!{x\r\n", ch, NULL, NULL, TO_ROOM);
        send_to_char("{CLightning stabs the ground!{x\r\n", ch); 

        if (number_percent() < ch->level) {
          alive = alive & env_damage(ch, dice(8,10), DAM_LIGHTNING, 
                 "{CA lightning bolt strikes @v2! [@n0]{x\r\n",
                 "{CA lightning bolt stikes you! [@n0]{x\r\n");
        }

        break;

     /* Holy light purges evil... */

      case SECT_HOLY:

        if (IS_EVIL(ch)) {
          alive = alive & env_damage(ch, (-1 * dice(5,10) * ch->alignment)/1000, DAM_HOLY, 
                 "{Y@v2 writhes in agony! [@n0]{x\r\n",
                 "{YThe light burns you! [@n0]{x\r\n");
        }
               
        break;

     /* Foul darkness chokes all that is good... */

      case SECT_EVIL:

        if (IS_GOOD(ch)) {
          alive = alive & env_damage(ch, (dice(5,10) * ch->alignment)/1000, DAM_NEGATIVE, 
                 "{R@v2 chokes and gags! [@n0]{x\r\n",
                 "{RYou choke and gag in the foulness! [@n0]{x\r\n");
        }
 
        break;

      default:
        break;
    } 
   } 

    return alive;   
  }  


void char_env_update() {
CHAR_DATA *ch;
CHAR_DATA *ch_next;
OBJ_DATA *obj;

 /* Photography... */
  for ( obj = object_list; obj != NULL; obj = obj->next )    {
       if (obj->item_type == ITEM_PHOTOGRAPH
       && obj->value[0] == PHOTO_EXPOSING) {
             obj->value[0] = PHOTO_TAKEN;
             act( "{c*CLICK*{x", NULL, obj, NULL, TO_ROOM);
       }
  }
     

 /* Check_Environment might kill ch, so it won't be valid afterwards... */
  for ( ch = char_list; ch != NULL; ch = ch_next ) {
    ch_next = ch->next;  

   /* Send event if required... */

    if (IS_SET(ch->time_wev, SUB_PULSE_10)) issue_update_wev(ch, WEV_PULSE, WEV_PULSE_10, 0);
    check_environment(ch); 
  }    

  return;
}


/*
 * Update all chars, including mobs.
*/
void char_update( void ) {   
DESCRIPTOR_DATA *d;
CHAR_DATA *ch;
CHAR_DATA *ch_next;
CHAR_DATA *ch_quit;
AFFECT_DATA *paf;
AFFECT_DATA *paf_next;
OBJ_DATA *blood;
int tolimb;
short playercount = 0;
bool alive;
ROOM_INDEX_DATA *room; 


    for (d = descriptor_list; d; d = d->next) {
         if (d->character && !d->original) {
              ch = d->character;

              if (ch->desc->connected == CON_GET_NEW_PASSWORD
              || ch->desc->connected == CON_CONFIRM_NEW_PASSWORD
              || ch->desc->connected == CON_GET_NEW_RACE
              || ch->desc->connected == CON_GET_PROFESSION
              || ch->desc->connected == CON_GET_NEW_SEX
              || ch->desc->connected == CON_GET_ALIGNMENT
              || ch->desc->connected == CON_GET_AMATEUR
              || ch->desc->connected == CON_MENU
              || ch->desc->connected == CON_CHECK_APPROVAL
              || (ch->desc->connected == CON_CHATTING
                 && (ch->desc->connected_old == CON_GET_NEW_PASSWORD
                        || ch->desc->connected_old == CON_CONFIRM_NEW_PASSWORD
                        || ch->desc->connected_old == CON_GET_NEW_RACE
                        || ch->desc->connected_old == CON_MENU
                        || ch->desc->connected_old == CON_GET_PROFESSION
                        || ch->desc->connected_old == CON_GET_NEW_SEX
                        || ch->desc->connected_old == CON_GET_ALIGNMENT
                        || ch->desc->connected_old == CON_GET_AMATEUR
                        || ch->desc->connected_old == CON_CHECK_APPROVAL))) {

                 ch->pcdata->store_number[1]++;
                 if (ch->desc->connected == CON_CHECK_APPROVAL
                 && ch->pcdata->store_number[0] == 0) {
                     if (ch->pcdata->store_number[1] >= mud.approval_timer)  sprintf(ch->desc->inbuf, "\r\n");
                     else sprintf_to_char(ch, "{r%d {gminute(s) until automatic approval.{x\r\n", mud.approval_timer - ch->pcdata->store_number[1]);
                 }
              }
         }
    }


    ch_quit	= NULL;

   /* update save counter */

    save_number++;
    if (save_number > 9) save_number = 0;

    for ( ch = char_list; ch != NULL; ch = ch_next ) {
	ch_next = ch->next;

       /* Set up player for timeout quit... */

        if (!IS_NPC(ch)) {
          playercount++;
          if (ch->pcdata->pk_timer > 0) {
              if (--ch->pcdata->pk_timer == 0) send_to_char("You are no longer protected against PK.\r\n", ch);
          }

          if (IS_IMMORTAL(ch)) { 
              if ( ch->timer > 30 ) ch_quit = ch;
          } else { 
              if ( ch->timer > 10 ) ch_quit = ch;
          }
        } else {

         /* Some mobs get a free ride home if they have wandered... */

          if ( IS_SET(ch->act, ACT_AUTOHOME)  
          && ch->timer > 5 ) {
                 room = get_room_index(ch->recall_perm);
                 if (room == NULL) {
                        REMOVE_BIT(ch->act, ACT_AUTOHOME);
                 } else {
                        char_from_room(ch);
                        char_to_room(ch, room);
                        ch->timer = 0;
                 }
           }
        } 

       /* Wake up the stunned... */  

        if ( ch->position == POS_STUNNED ) update_pos( ch );

       /* Send event if required... */

        if (IS_SET(ch->time_wev, SUB_PULSE_30)) issue_update_wev(ch, WEV_PULSE, WEV_PULSE_30, 0);

        if (IS_SET(ch->plr, PLR_AFK)) continue;

       /* Soberiety comes slowly... */

        gain_condition( ch, COND_DRUNK,  -1 );

       /* Kick up inactivity timer... */

        if (!IS_NPC(ch)) {
          if ( ch->desc != NULL && ch->desc->character == ch ) ch->timer += 1;

        } else {
          if ( IS_SET(ch->act, ACT_AUTOHOME)
            && ch->next_in_room == NULL
            && ch->in_room != NULL
            && ch->in_room->people == ch ) {
            ch->timer += 1;
          } else {
            ch->timer = 0;
          }

        } 

	for ( paf = ch->affected; paf != NULL; paf = paf_next ) {

	    paf_next	= paf->next;
	    if ( paf->duration > 0 ) {
		paf->duration--;
		if (number_range(0,4) == 0 && paf->level > 0)
		  paf->level--;  /* spell strength fades with time */
            }
	    else if ( paf->duration < 0 )
		;
	    else  {
		if ( paf_next == NULL
		||   paf_next->type != paf->type
		||   paf_next->duration > 0 ) 	{
                                    if (paf != NULL) {
		        if ( paf->afn > 0 
                                        && affect_array[paf->afn].end_msg) {
			send_to_char(affect_array[paf->afn].end_msg, ch);
			send_to_char("\r\n", ch );
		        } else {
		                send_to_char( "You feel different!\r\n", ch );
		        }
                                    }
		}
                                if (paf != NULL) {
	 	       if (paf->bitvector == AFF_POLY) undo_mask(ch, FALSE);
	 	       if (paf->bitvector == AFF_MORF) undo_morf(ch, ch->trans_obj);
                                       
		       affect_remove( ch, paf );
                                }
	    }
	}

	/*
	 * Careful with the damages here,
	 *   MUST NOT refer to ch after damage taken,
	 *   as it may be lethal damage (on NPC).
	 */

        alive = TRUE;
    
        if ( (ch == NULL) 
          || (ch->in_room == NULL) ) {
          alive = FALSE;
        }

          /* Night active mobs get purged on daytime */

          if (IS_SET(ch->act, ACT_NIGHT)
          && IS_SET(time_info.flags, TIME_DAY)) {
       	act( "$n prefers to hide during daytime.", ch, NULL, NULL, TO_ROOM );
                extract_char( ch, TRUE );
                alive=FALSE;
          }

          if (alive
          && IS_SET(ch->act, ACT_MARTIAL)
          && ch->pIndexData) {
                if (ch->pIndexData->area->martial == 0) {
       	    act( "$n is no longer needed.", ch, NULL, NULL, TO_ROOM );
                    extract_char( ch, TRUE );
                    alive=FALSE;
                }
          }


       /* Plauge causes damage and infects others... */

        if (alive && is_affected(ch, gafn_plague))  {
            AFFECT_DATA *af, plague;
            CHAR_DATA *vch;
            int save, dam;

            for ( af = ch->affected; af != NULL; af = af->next )   {
            	if (af->afn == gafn_plague)  break;
            }
        
            if (af == NULL) {
            	REMOVE_BIT(ch->affected_by, AFF_PLAGUE);
            	return;
            }
        
            if (af->level == 1)
            	return;
        
            plague.afn 		= gafn_plague;
            plague.level 	= af->level - 1; 
            plague.duration 	= number_range(1,2 * plague.level);
            plague.location	= APPLY_STR;
            plague.modifier 	= -5;
            plague.bitvector 	= AFF_PLAGUE;
        
            for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)     {
            	switch(check_immune(vch,DAM_DISEASE)) {
            	    case(IS_NORMAL) 	: save = af->level - 4;	break;
            	    case(IS_IMMUNE) 	: save = 0;		break;
            	    case(IS_RESISTANT) 	: save = af->level - 8;	break;
            	    case(IS_RESISTANT_PLUS): save = af->level - 8;	break;
            	    case(IS_VULNERABLE)	: save = af->level; 	break;
            	    default		: save = af->level - 4;	break;
            	}
            
                if (save != 0 && !saves_spell(save,vch) && !IS_IMMORTAL(vch)
            	&&  !IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(4) == 0) {
            	    send_to_char("You feel hot and feverish.\r\n",vch);
            	    act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
            	    affect_join(vch,&plague);
            	}
            }

	    dam = UMIN(ch->level,5);
	    ch->mana -= dam;
                    ch->mana = UMAX(ch->mana, 0);
	    ch->move -= dam;
                    ch->move = UMAX(ch->move, 1);

            alive = alive & env_damage(ch, dam, DAM_DISEASE, 
	           "{G@v2 writhes in agony as plague sores erupt from @v3 skin. [@n0]{x\r\n",
	           "{GYou writhe in agony from the plague. [@n0]{x\r\n");

        }

       /* Poison causes damage... */ 

	if ( alive && IS_AFFECTED(ch, AFF_POISON)) {
	
            AFFECT_DATA *af;

            int dam;

            for ( af = ch->affected; af != NULL; af = af->next ) {
            	if (af->afn == gafn_poison) {
                    break;
                }
            }

            dam = 0; 
        
            if (af != NULL) {

              if ( af->level > 0 ) {
                af->level -= 1; 
              }

              if ( af->level > 1 ) {
                dam = dice(1, af->level);
              }

            } else {
              dam = dice(1, ch->level);
            } 
 
            if ( dam > 0 ) {
              alive = alive & env_damage(ch, dam, DAM_POISON, 
	             "{G@v2 shivers and suffers. [@n0]{x\r\n",
	             "{GYou shiver and suffer. [@n0]{x\r\n");
            }
	}

           if (alive
           && IS_SET(ch->form, FORM_BLEEDING)) {
          
              if (!IS_NPC(ch)) {
                      alive = alive & env_damage(ch, number_range(ch->max_hit/100,ch->max_hit/20), DAM_NONE, 
	             "{r@v2 loses blood. [@n0]{x\r\n",
	             "{rYou lose blood. [@n0]{x\r\n");
             
                      if (ch->in_room !=NULL) {
                             if (ch->in_room->sector_type != SECT_WATER_SWIM
                             && ch->in_room->sector_type != SECT_UNDERWATER
                             && ch->in_room->sector_type != SECT_AIR
                             && ch->in_room->sector_type != SECT_SPACE
                             && ch->in_room->sector_type != SECT_WATER_NOSWIM) {
                                     blood = ch->in_room->contents;
                                     while ( blood != NULL
                                     && blood->pIndexData->vnum != 54 ) {
                                            blood = blood->next_content;
                                     }
   
                                     if (blood==NULL) {
   	                            blood = create_object (get_obj_index (OBJ_VNUM_BLOOD), 0);
	                            obj_to_room (blood, ch->in_room);
	                            blood->timer = 2;
                                     }
                             }
                     }
               }
               if (number_percent() < get_curr_stat(ch, STAT_CON)/10) REMOVE_BIT(ch->form, FORM_BLEEDING);
           }
           tolimb=number_range(0,5);
           if (ch->limb[tolimb]>0 && ch->limb[tolimb]<10) {
                   if (number_range(0,ch->limb[tolimb]) < 2) ch->limb[tolimb] -=1;
           }
           update_wounds(ch);

       /* Incapacatated folks die slowly... */ 

	if ( alive && ch->position == POS_INCAP ) { 
	
            alive = alive & env_damage(ch, dice(1,2), DAM_NONE, 
	           "{R@v2 dies a little. [@n0]{x\r\n",
	           "{RYou feel your life slipping away... [@n0]{x\r\n");
	}

       /* Mortally wounded folks die a little quicker... */ 

	if ( alive && ch->position == POS_MORTAL ) {

            alive = alive & env_damage(ch, dice(1,3), DAM_NONE, 
	           "{R@v2 bleeds all over the floor. [@n0]{x\r\n",
	           "{RYou bleed all over the floor. [@n0]{x\r\n");
	}

       /* Those who are dead, should be dead... */

        if ( alive && ch->position == POS_DEAD ) {
            send_to_char(
            "A worm ridden skeleton cald in tattered rags appears before you.\r\n"
            "A charnal stench assaults your nostrils.\r\n"
            "It looks at you with empty eyes, seeming somehow puzzled.\r\n"
            "{gDeath rasps 'You should be dead.'{x\r\n", ch); 
            log_string("Slaying the living dead!"); 
            raw_kill(ch, TRUE);
            alive = FALSE;
        }
    }

    if (playercount > mud.player_count) mud.player_count = playercount;

    /*
     * Autosave and autoquit.
     * Check that these chars still exist.
     */
    for ( ch = char_list; ch != NULL; ch = ch_next )    {
        ch_next = ch->next;

            if (ch !=NULL) {
	if ( ch->desc != NULL 
                && ch->desc->descriptor % 10 == save_number) {
                      if (ch->nightmare) {
                           if (check_skill(ch, gsn_dreaming, 10)) {
                                send_to_char("You finally feel a bit safer...\r\n", ch);
                                ch->nightmare = FALSE;
                           }              
                      }
	      save_char_obj(ch);
                       if (IS_SET (ch->plr, PLR_AUTOSAVE)) send_to_char ("Character autosaved.\r\n",ch);
                 }
            }
             if ( ch == ch_quit ) do_quit( ch, "" );
    }

   /* Save stuff... */

    if (IS_SET(mud.flags, MUD_CRON)) writeout();
    save_artifacts();
    if (IS_SET(mud.flags, MUD_AUTOOLC)) olcautosave();

    log_string("Saving Morgues:");
    save_morgue();
    
    if (save_number == 3) {
          log_string("Saving Mana:");
          save_cults();
          log_string("Saving Stock Market:");
          save_shares();
    }

    if (save_number == 7) {
          log_string("Saving Trees:");
          save_tree();
          log_string("Saving Shops:");
          save_shop_obj();
          log_string("Beginning Save Locker...");
          save_all_lockers();
    }  

    return;
}


void lease_update( void ) {
    ROOM_INDEX_DATA *pRoomIndex;
    int iHash;

    for (iHash=0; iHash < MAX_KEY_HASH; iHash++ ) {
	for ( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next ) {
	    if (IS_SET(pRoomIndex->room_flags, ROOM_RENT))    {
		int timeleft = 0;

		if (pRoomIndex->paid_year < time_info.year)   	timeleft = 0;
		else if (pRoomIndex->paid_year == time_info.year)	timeleft = pRoomIndex->paid_month - time_info.month;
		else if (pRoomIndex->paid_year > time_info.year) {
		    timeleft = pRoomIndex->paid_month - time_info.month;
		    timeleft += 17;
		} if (timeleft <= 0) {
		    free_string(pRoomIndex->rented_by);
		    pRoomIndex->rented_by = NULL;
		    act( "The Lease on this room has Expired.", NULL, NULL, NULL, TO_ROOM );
		}
            }
        } 
    } 
} 


/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void ) {   
OBJ_DATA *obj;
OBJ_DATA *obj_next;
AFFECT_DATA *paf, *paf_next;
CHAR_DATA *ch = NULL;
MOB_CMD_CONTEXT *mcc;
WEV *wev;
bool outzone, canreturn;

    for ( obj = object_list; obj != NULL; obj = obj_next )    {
	CHAR_DATA *rch;
	char *message;

	obj_next = obj->next;

        for ( paf = obj->affected; paf != NULL; paf = paf_next )        {
            paf_next    = paf->next;

            if ( paf->duration > 0)   {
                paf->duration--;
                if (number_range(0,4) == 0 && paf->level > 0)  paf->level = UMAX(1, paf->level -= number_range(1,5)); 

            } else if ( paf->duration < 0 ) {

            } else  {
                if ( paf_next == NULL
                ||   paf_next->type != paf->type
                ||   paf_next->duration > 0 )                {
                    if ( obj->carried_by != NULL ) sprintf_to_char(obj->carried_by, "Your %s seems less enchanted!\r\n", obj->short_descr);
                }

                affect_remove_obj( obj, paf );
            }
        }

        if (obj->enchanted) {
            for ( paf = obj->affected; paf; paf = paf->next )        {
                  if (paf->duration < 0  && paf->location == APPLY_EFFECT) {
                        update_charge(obj, paf);
                  }

                  if (paf->duration < 0 && paf->location == APPLY_GOLD) produce_money(obj, paf);
                  if (paf->duration < 0 && paf->location == APPLY_EXP) produce_exp(obj, paf);
                  if (paf->duration < 0 && paf->location == APPLY_ALIGN) modify_align(obj, paf);
                  if (paf->duration < 0 && paf->location == APPLY_SANITY_GAIN) produce_sanity(obj, paf);
            }
       } else {
            for ( paf = obj->pIndexData->affected; paf; paf = paf->next )        {
                  if (paf->duration < 0 && paf->location == APPLY_GOLD) produce_money(obj, paf);
                  if (paf->duration < 0 && paf->location == APPLY_EXP) produce_exp(obj, paf);
                  if (paf->duration < 0 && paf->location == APPLY_ALIGN) modify_align(obj, paf);
                  if (paf->duration < 0 && paf->location == APPLY_SANITY_GAIN) produce_sanity(obj, paf);
            }
       }

       if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)
       && obj->carried_by) {
            OBJ_INDEX_DATA *pObj = obj->pIndexData;

            if (!IS_NPC(obj->carried_by)) {
                if (pObj->artifact_data) update_artifact(obj);
            }
       }
     
        /* Trees */
        if (obj->item_type == ITEM_TREE && obj->value[2] > 0) update_tree(obj);

        /* Pools */
        if (obj->item_type == ITEM_POOL) obj->value[3] = UMIN(obj->level *10, obj->value[3] + obj->level/5 +1);

       /* Check if idols are being carried... */ 
        if ( obj->item_type == ITEM_IDOL
          && obj->carried_by != NULL
          && obj->wear_loc != WEAR_NONE ) {

         /* Ok, get context and build the wev... */

          mcc = get_mcc(obj->carried_by, obj->carried_by, NULL, NULL, obj, NULL, 0, NULL);
          wev = get_wev(WEV_IDOL, WEV_IDOL_HELD, mcc,
                        NULL,
                        NULL,
                       "@a2 is holding @p2.\r\n");

         /* Now, do we have any rooms to tell? */
          idol_issue_wev( obj, wev);

         /* Free the wev (will autofree the context)... */
          free_wev(wev);
        }

        canreturn = FALSE;
        if (obj->item_type == ITEM_WEAPON) {
             if (obj->owner != NULL
             && IS_SET(obj->extra_flags, ITEM_ANIMATED)
             && obj->carried_by == NULL) {
                   ch = get_char_world(NULL, obj->owner);
                   if (ch != NULL) {
                       if (obj->in_obj != NULL) {
                             OBJ_DATA *t_obj, *next_obj;
                             for (t_obj = obj->in_obj; t_obj != NULL; t_obj = next_obj)   {
       	                next_obj = t_obj->in_obj;
                                if (next_obj == NULL) {
                                   if (t_obj->in_room !=NULL) {
                                       if (t_obj->in_room->area == ch->in_room->area)  {
                                          canreturn = TRUE;
                                          obj_from_obj(obj);
                                      }
                                   }
                                }
                             }
                       } else if (obj->in_room) {
                             if (obj->in_room->area == ch->in_room->area)  {
                                  canreturn = TRUE;
                                  obj_from_room(obj);
                             }
                       }
              	       if (canreturn) {
                           obj_to_char(obj,ch);
                           act("$p suddenly appears to stay with it's owner.\r\n", ch, obj, NULL, TO_ROOM );
                           act("$p suddenly appears to stay by with you.\r\n", ch, obj, NULL, TO_CHAR );
                       }
                   }
             }
        }
  

       /* See if the object has rotted/evaporated... */

        if ( obj->item_type == ITEM_TATTOO) {
              if (obj->carried_by != NULL) {
                   if (!IS_NPC(obj->carried_by)
                   && obj->wear_loc == WEAR_NONE ) {
                        if (obj->timer <=0 || obj->timer >3) {
                             act("$p slowly begins to fade.\r\n", obj->carried_by, obj, NULL, TO_CHAR );
                             obj->timer=3;
                        }
                   }
              } else {
                        if (obj->timer <=0 || obj->timer >3) obj->timer =3;
              }
        }

       outzone = FALSE; 
       if (obj->carried_by != NULL 
       && obj->zmask != 0
       && (obj->zmask & zones[obj->carried_by->in_zone].mask ) == 0 ) {
            outzone = TRUE;
       }

       if (obj->item_type == ITEM_TIMEBOMB) {

           if (!outzone ) {
       	if ( obj->timer <= 0 || --obj->timer > 0 ) {
	    continue;
                }
                bomb_explode(obj);
           }

       } else if (obj->item_type == ITEM_LIGHT) {
           ROOM_INDEX_DATA *room = NULL;
           bool effect = TRUE;

           if (obj->value[2] > 0
           && obj->value[3] > 0
           && !outzone) {

                if ( --obj->value[2] == 0) {

                     if (obj->in_room) {
                         room = obj->in_room;
                     } else if (obj->carried_by) {
                         room = obj->carried_by->in_room;
                         if (obj->wear_loc == WEAR_NONE) effect = FALSE;  
                     }

                     if (room) {
                           if (effect) --room->light;
                            if (obj->carried_by) act( "$p goes out.", obj->carried_by, obj, NULL, TO_CHAR);
                            else if (obj->in_room) act( "$p goes out.", NULL, obj, NULL, TO_ROOM );
                           extract_obj( obj );
                     }

	}else { 
                     if (obj->value[2] <= 5) {
                            if (obj->carried_by) act("$p flickers.", obj->carried_by, obj, NULL, TO_CHAR);
                            else if (obj->in_room) act("$p flickers.", NULL, obj, NULL, TO_ROOM);
                     }
                }
            }

       } else {
       	if ( obj->timer <= 0 
                || --obj->timer > 0 ) {
	    continue;
                }

        switch ( obj->item_type ) {
            case ITEM_FOUNTAIN:
                if (obj->pIndexData->vnum >65) {
                     obj->value[2] = obj->pIndexData->value[2];
                     obj->value[3] = obj->pIndexData->value[3];
                     continue;
                }
                message = "$p dries up."; 
                break;

            case ITEM_CORPSE_NPC:
            case ITEM_CORPSE_PC:  
            message = "$p decays into dust."; 
            break;

            case ITEM_FOOD:       
            message = "$p decomposes.";
            break;

            case ITEM_POTION:
            message = "$p has evaporated from disuse.";	
            break;

            default:
            message = "$p crumbles into dust.";
            break;
        }

        if ( obj->carried_by != NULL ) {
             if ( IS_NPC(obj->carried_by) 
             && obj->carried_by->pIndexData->pShop != NULL ) {
                     obj->carried_by->gold[0] += obj->cost/5;
             } else {
                     if ( !outzone ) act( message, obj->carried_by, obj, NULL, TO_CHAR );
             }  
        } else if ( obj->in_room != NULL
        && ( rch = obj->in_room->people ) != NULL ) {
             if ( !( obj->in_obj != NULL
             && obj->in_obj->pIndexData->vnum == OBJ_VNUM_PIT
             && !CAN_WEAR(obj->in_obj,ITEM_TAKE)))  {
    	act( message, rch, obj, NULL, TO_ROOM );
    	act( message, rch, obj, NULL, TO_CHAR );
             }
        }

        if ( obj->item_type == ITEM_CORPSE_PC 
        && obj->contains != NULL) {
            OBJ_DATA *t_obj, *next_obj;
            for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)   {
   	   next_obj = t_obj->next_content;
	   obj_from_obj(t_obj);
  	    if (obj->in_obj) 
	       obj_to_obj(t_obj,obj->in_obj);
	    else if (obj->carried_by)  
	       obj_to_char(t_obj,obj->carried_by);
	    else if (obj->in_room == NULL)  
	       extract_obj(t_obj);
 	    else 
	       obj_to_room(t_obj,obj->in_room);
            }
        }

       /* If it's out of zone, then pull it from the character... */

        if ( obj->carried_by != NULL  
        && outzone ) {
           obj_from_char_ooz(obj);
        } 

       /* Ok, remove the object... */
       extract_obj( obj );
    }
    }

    return;
}


void aggr_update( void ) {
CHAR_DATA *wch;
CHAR_DATA *wch_next;
CHAR_DATA *ch;
CHAR_DATA *ch_next;
CHAR_DATA *vch;
CHAR_DATA *vch_next;
CHAR_DATA *victim;
int chance, arange;
int count;

    for ( wch = char_list; wch != NULL; wch = wch_next )    {
	wch_next = wch->next;

        if ( IS_NPC( wch ) && wch->mpactnum > 0 && wch->in_room->area->nplayer > 0 )        {
             MPROG_ACT_LIST * tmp_act, *tmp2_act;

             for ( tmp_act = wch->mpact; tmp_act != NULL;  tmp_act = tmp_act->next )  {
               mprog_wordlist_check( tmp_act->buf,wch, tmp_act->ch,
               tmp_act->obj, tmp_act->vo, ACT_PROG );
               free_string( tmp_act->buf );
            }
            for ( tmp_act = wch->mpact; tmp_act != NULL; tmp_act = tmp2_act )    {
                  tmp2_act = tmp_act->next;
                  free_mem( tmp_act, sizeof( MPROG_ACT_LIST ) );
            }
            wch->mpactnum = 0;
            wch->mpact    = NULL;
        }


	if (IS_IMMORTAL(wch)  ||   wch->in_room == NULL  ||   wch->in_room->area->empty)  continue;
                
	for (ch = wch->in_room->people; ch != NULL; ch = ch_next )	{
	    ch_next	= ch->next_in_room;

                    if (!ch || !wch || ch == wch || !wch->in_room) continue;

                    if (ch->fighting != NULL) {
                         if (ch->in_room != ch->fighting->in_room) ch->fighting = NULL;
                         continue;
                    }
                    if (wch->fighting != NULL) {
                         if (wch->in_room != wch->fighting->in_room) wch->fighting = NULL;
                         continue;
                    }

                    if (!IS_SET(ch->act, ACT_AGGRESSIVE)  && !IS_SET(ch->act, ACT_INVADER) && !IS_SET(ch->act, ACT_MARTIAL)) continue;

                    if (IS_SET(ch->act, ACT_MARTIAL)) {
                         if (!ch->pIndexData) continue;
                         if (!ch->pIndexData->area) continue;
                         if (ch->pIndexData->area->martial == 0) continue;
                    }
                   
                       if  (!IS_NPC(ch)
	       ||   IS_SET(wch->in_room->room_flags, ROOM_SAFE)
	       ||   IS_AFFECTED(ch, AFF_CALM)
	       ||   IS_AFFECTED(ch, AFF_CHARM)
	       ||   !IS_AWAKE(ch)
                       ||   IS_SET(ch->act, ACT_PRACTICE)
                       ||   IS_SET(ch->act, ACT_IS_ALIENIST)
                       ||   IS_SET(ch->act, ACT_IS_HEALER)
	       ||   !can_see( ch, wch ) 
	       ||   number_bits(1) == 0
                       ||   IS_AFFECTED(ch, AFF_FEAR))   continue;

                       if (ch->pIndexData->pShop) continue;
             
                      /*
	     * Ok we have a 'wch' player character and a 'ch' npc aggressor.
	     * Now make the aggressor fight a RANDOM pc victim in the room,
	     *   giving each 'vch' an equal chance of selection.
	     */
	    count	= 0;
	    victim	= NULL;
	    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )	    {
		vch_next = vch->next_in_room;

                             if (vch != NULL && vch != ch) {
                                if (IS_IMMORTAL(vch)
                                || IS_AFFECTED(vch, AFF_MIST)
                                || !can_see(ch, vch)) continue;
     
                                if (IS_AFFECTED(vch, AFF_AURA)) arange = -5;
                                else arange = 15;

                                if (IS_SET(ch->act, ACT_INVADER)) {
                                     if  (!IS_SET(vch->act, ACT_INVADER)
                                     &&   !IS_SET(vch->act, ACT_WIMPY)
                                     &&   ch->fighting == NULL && vch->fighting == NULL
                                     &&   ( !IS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch))) {
		           if (vch != ch) victim = vch;
		           count++;
		     }

                                } else if (IS_SET(ch->act, ACT_MARTIAL)) {
                                     if  (!IS_NPC(vch) 
                                     && IS_SET(vch->act, ACT_CRIMINAL)) {
		          if ( number_range( 0, count ) == 0 ) victim = vch;
		          count++;
		     }

                                } else {
                                     if  (!IS_NPC(vch)
		     &&   (IS_SET(ch->act, ACT_BRAINSUCKED) || ch->level >= vch->level - arange)
		     &&   (!IS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch))) {
		          if ( number_range( 0, count ) == 0 ) victim = vch;
		          count++;
		     }
                                }
                                   
                             }
	    }
                    
             	    if ( victim == NULL ) continue;

                    chance = (victim->level + 5 - ch->level) * 3;
                     if (number_percent() < chance) continue;
                    
                    if (IS_SET(victim->form, FORM_SENTIENT) && IS_NPC(victim)) do_yell( victim, "They are coming!" );
	    multi_hit( ch, victim, TYPE_UNDEFINED );
                    if (!IS_SET(ch->act, ACT_INVADER)  && !IS_SET(victim->act, ACT_INVADER)) check_assist(ch,victim);
	}
    
   }
    return;
}


/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */

void update_handler( void ) {
static  int     pulse_area		= 0;
static  int     pulse_mobile	= 0;
static  int     pulse_violence	= 0;
static  int     pulse_point		= 0;
static  int     pulse_chup		= 0;
static  int     pulse_env		= 0;
static  int     pulse_mob_fast	= 0;
static  int    pulse_music                 = 0;

    if ( --pulse_area     <= 0 ) {
	pulse_area	= number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 );
	area_update	( );
                quest_update     ( );
    }


    if ( --pulse_music	  <= 0 )   {
	pulse_music	= PULSE_MUSIC;
	song_update();
    }

    if ( --pulse_mobile   <= 0 ) {
	pulse_mobile	= PULSE_MOBILE;
	mobile_update	( );
    }

    if ( --pulse_violence <= 0 ) {
	pulse_violence	= PULSE_VIOLENCE;
	violence_update	( );
    }

    if ( --pulse_point    <= 0 ) {

	pulse_point     = PULSE_TICK;
	time_update();
	char_update();
	obj_update();
	lease_update();
                society_update();
                partners_update();
                scribe_online();

                if (time_info.hour > 7
                && time_info.hour < 13
                && (time_info.day + 1) % 7 != 0
                && (time_info.day + 1) % 7 != 6) {
                     update_stock_market();
                }
                if (time_info.hour < 8) clear_stock_balance();
    }

   /* 180 pulses to the hour... */

    time_info.minute = (180 - pulse_point) / 3;

   /* Short healing tick for PCs */

    if ( --pulse_chup <=0 ) {
	pulse_chup	= PULSE_TICKSHORT;
	char_update_short  ( );
    }

   /* Shorter pulse for the environment... */

    if ( --pulse_env <= 0) {
      pulse_env = PULSE_ENV;
      char_env_update();
    }

   /* Fast pulse for mob commands... */

    if ( --pulse_mob_fast <= 0 ) {
        pulse_mob_fast = PULSE_MOB_FAST;
        mob_fast_update();
    }

   /* Auction pulse... */

  auction_update();

   /* Combat pulse... */

    aggr_update();
    scheduled_reboot();

    tail_chain();
    return;
}


void scheduled_reboot(void) {
int ti;

   ti = (int) (current_time);
   ti %= 86400;
   ti /=60;

   if (ti/60 == mud.reboot && ti%60 == 0) {
       if (pulse_muddeath < 0)  {
            pulse_muddeath = 5;
            reboot_type = 0;
            check_muddeath( );
       }
   }
   return;
}


void undo_mask (CHAR_DATA *ch, bool wereok) {
AFFECT_DATA *paf;

     if (is_affected(ch, gafn_mask_hide)) return;

     if (!IS_SET(ch->act,ACT_WERE) ) {
          if (ch->race_orig != 0) {
	free_string(ch->description);
	free_string(ch->short_descr);
	free_string(ch->long_descr);
	free_string(ch->poly_name);
	if (!IS_NPC(ch))	{
      	       ch->description = str_dup (ch->description_orig);
	       ch->short_descr = str_dup (ch->short_descr_orig); 
	       ch->long_descr  = str_dup (ch->long_descr_orig);
	       ch->start_pos   = -1;
	       ch->poly_name	= str_dup ("");
                       ch->race=ch->race_orig;
                       ch->race_orig = 0;
	} else {
	       ch->description = str_dup (ch->pIndexData->description);
	       ch->short_descr = str_dup (ch->pIndexData->short_descr); 
	       ch->long_descr  = str_dup (ch->pIndexData->long_descr);
	       ch->poly_name	= str_dup ("");
                       ch->race_orig = 0;
	}		
          }
     } else {
          if (IS_AFFECTED(ch, AFF_POLY) && wereok) {
               REMOVE_BIT( ch->affected_by, AFF_POLY );
               if (ch->race_orig != 0 ) {
                  send_to_char( "You change back to your normal form!\r\n", ch );
                  act("$n changes back to his normal form.", ch, NULL, NULL, TO_ROOM);

                  free_string(ch->description);
                  free_string(ch->short_descr);
                  free_string(ch->long_descr);
                  free_string(ch->poly_name);
           
                  ch->description = str_dup (ch->description_orig);
                  ch->short_descr = str_dup (ch->short_descr_orig); 
                  ch->long_descr  = str_dup (ch->long_descr_orig);
                  ch->poly_name	= str_dup ("");
                  ch->race=ch->race_orig;
                  ch->race_orig = 0;

                   paf             =   new_affect();
                   paf->location   =   APPLY_STR;
                   paf->skill	    =   0;
                   paf->modifier   =   ch->level/15;
                   paf->type	    =	SKILL_UNDEFINED;
                   paf->afn        =   0;
                   paf->duration   =   -1;
                   paf->bitvector  =   AFF_HASTE;
                   affect_modify( ch, paf, FALSE ); 

                   paf             =   new_affect();
                   paf->location   =   APPLY_INT;
                   paf->skill	    =   0;
                   paf->modifier   =   -ch->level/15;
                   paf->type	    =	SKILL_UNDEFINED;
                   paf->afn        =   0;
                   paf->duration   =   -1;
                   paf->bitvector  =   AFF_MELD;
                   affect_modify( ch, paf, FALSE ); 

               }
          }
     }
     return;
}


void room_update( AREA_DATA *pArea ) {
ROOM_INDEX_DATA *room;
int vnum;

      for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum ++ )   {
          if ( (room = get_room_index(vnum)) )   room_aff_update(room);
      }

       return;
}


void room_aff_update( ROOM_INDEX_DATA *room ) {
        AFFECT_DATA *paf;
        AFFECT_DATA *paf_next;

        for ( paf = room->affected; paf != NULL; paf = paf_next )       {
            paf_next    = paf->next;
            if ( paf->duration > 0 )  {
                paf->duration--;
                if (number_range(0,4) == 0 && paf->level > 0)
                  paf->level--;  /* spell strength fades with time */
            }
            else if ( paf->duration < 0 )
                ;
            else
            {
               
                affect_remove_room( room, paf );
            }
        }
}


void auction_update (void) {
char buf[MAX_STRING_LENGTH];

    if (auction->item != NULL)
        if (--auction->pulse <= 0) {
            auction->pulse = PULSE_AUCTION;
            switch (++auction->going)             {
            case 1 : 
            case 2 : 
            if (auction->bet > 0)
                sprintf (buf, "{m%s{x: going %s {yfor %d{x.", auction->item->short_descr,
                     ((auction->going == 1) ? "{Yonce{x" : "{Rtwice{x"), auction->bet);
            else
                sprintf (buf, "{m%s{x: going %s {y(no bet received yet){x.", auction->item->short_descr,
                     ((auction->going == 1) ? "{Yonce{x" : "{Rtwice{x"));

            talk_auction (buf);
            break;

            case 3 : /* SOLD! */
            if (auction->buyer != auction->seller)  {
                sprintf (buf, "{m%s{x sold to %s {yfor %d{x.", auction->item->short_descr, IS_NPC(auction->buyer) ? auction->buyer->short_descr : auction->buyer->name, auction->bet);
                talk_auction(buf);
                obj_to_char (auction->item,auction->buyer);
                act ("The auctioneer appears before you in a puff of smoke and hands you $p.", auction->buyer,auction->item,NULL,TO_CHAR);
                act ("The auctioneer appears before $n, and hands $m $p", auction->buyer,auction->item,NULL,TO_ROOM);

                auction->seller->gold[0] += (auction->bet * 75)/100; /* give him the money */
                auction->item = NULL; /* reset item */

            } else {
                sprintf (buf, "No bets received for %s - object has been removed.",auction->item->short_descr);
                talk_auction(buf);
                act ("The auctioneer appears before you to return $p to you.", auction->seller,auction->item,NULL,TO_CHAR);
                act ("The auctioneer appears before $n to return $p to $m.", auction->seller,auction->item,NULL,TO_ROOM);
                obj_to_char (auction->item,auction->seller);
                auction->item = NULL; /* clear auction */

            }

            }
        } 
} 


void writeout(void) {
FILE *fp = fopen ("../run.inf", "w");

    if (!fp) exit(1); 
    fclose(fp);
    return;
}


void check_muddeath(void) {
char buf[MAX_STRING_LENGTH];
buf[0] = '\0';

    if (pulse_muddeath < 0)  return;
    if (pulse_muddeath == 0) reboot_shutdown(reboot_type);
    if (pulse_muddeath > 0)    {
         switch(reboot_type) {
           case 0:
              sprintf_to_world("{gRefresh in {r%d{g minute(s)!{x\r\n", pulse_muddeath);
              break;
           case 1:
              sprintf_to_world("{gReboot in {r%d{g minute(s)!{x\r\n", pulse_muddeath);
              break;
           case 2:
              sprintf_to_world("{gShutdown in {r%d{g minute(s)!{x\r\n", pulse_muddeath);
              break;
         }
    }
    --pulse_muddeath;
    return;
}


void update_tree(OBJ_DATA *tree) {
               OBJ_DATA *fruit;

               tree->value[3] = UMAX(tree->value[3], 0);

               if (time_info.year - tree->value[2] >=0
               && tree->value[0] <= TREE_SEED) {
                       tree->value[0] = TREE_SPROUT;

                       free_string(tree->description);
                       tree->description = str_dup("A tiny sprout grows here.\n");

                       free_string(tree->short_descr);
                       tree->short_descr = str_dup("a tiny sprout");

                       free_string(tree->name);
                       tree->name = str_dup("sprout");
               }
               if (time_info.year - tree->value[2] >=1
               && tree->value[0] <= TREE_SPROUT
               && tree->value[3] > 10) {
                       tree->value[3] -=10;
                       tree->value[0] = TREE_SMALL;

                       free_string(tree->description);
                       tree->description = str_dup("A small tree grows here.\n");

                       free_string(tree->short_descr);
                       tree->short_descr = str_dup("a small tree");

                       free_string(tree->name);
                       tree->name = str_dup("tree");
               }
               if (time_info.year - tree->value[2] >=5
               && tree->value[0] <= TREE_SMALL
               && tree->value[3] > 50) {
                       tree->value[3] -=50;
                       tree->value[0] = TREE_NORMAL;

                       free_string(tree->description);
                       tree->description = str_dup("A tree grows here.\n");

                       free_string(tree->short_descr);
                       tree->short_descr = str_dup("a tree");

                       free_string(tree->name);
                       tree->name = str_dup("tree");
               }
               if (time_info.year - tree->value[2] >=20
               && tree->value[0] <= TREE_NORMAL
               && tree->value[3] > 500) {
                       tree->value[3] -=500;
                       tree->value[0] = TREE_LARGE;

                       free_string(tree->description);
                       tree->description = str_dup("An impressive large tree grows here.\n");

                       free_string(tree->short_descr);
                       tree->short_descr = str_dup("a large tree");

                       free_string(tree->name);
                       tree->name = str_dup("tree");
               }
               if (time_info.year - tree->value[2] >=60
               && tree->value[0] <= TREE_LARGE
               && tree->value[3] > 3000) {
                       tree->value[3] -=3000;
                       tree->value[0] = TREE_ANCIENT;

                       free_string(tree->description);
                       tree->description = str_dup("A huge ancient tree grows here.\n");

                       free_string(tree->short_descr);
                       tree->short_descr = str_dup("an ancient tree");

                       free_string(tree->name);
                       tree->name = str_dup("tree");
               }

               if (number_percent() <= 10) tree->value[3]++;
               tree->value[3] = UMIN(tree->value[0] * tree->value[0] * tree->value[0] * 150, tree->value[3]);
               fruit = tree->contains;
               while (fruit) {
                    if (number_percent() <= 2) tree->value[3]--;
                    fruit = fruit->next_content;
               }

               if (tree->condition < 100) {
                    tree->value[3] -=2;
                    tree->condition++;
               }
                              
               if (time_info. month >= 4 && time_info.month<12
               && tree->value[3] > 100
               && tree->value[0] > TREE_SMALL
               && tree->condition > 50
               && number_percent() < 2) {
                     fruit = create_object(get_obj_index(OBJ_VNUM_MUSHROOM), 0 );
                     obj_to_obj(fruit, tree);

                     free_string(fruit->description);
                     fruit->description = str_dup("A tasty fruit lies here.\n");

                     free_string(fruit->name);
                     fruit->name = str_dup("fruit");

                     free_string(fruit->short_descr);
                     fruit->value[0] = UMIN(tree->value[3] / 100 + 1, 14);
                     tree->value[3] = tree->value[3] - 100 - fruit->value[0]*5;

                     switch (tree->value[1]) {
                           default:                     
                               fruit->short_descr = str_dup("a fruit");
                               break;

                           case TREE_LIGHT:
                               fruit->short_descr = str_dup("a fruit of light");
                               if (tree->value[3] > 50) {
                                     fruit->value[2] = get_effect_efn("cure light");
                                     tree->value[3] -= 10;
                                     break;
                               }
                               if (tree->value[3] > 200) {
                                     fruit->value[2] = get_effect_efn("bless");
                                     tree->value[3] -= 40;
                                     break;
                               }
                               if (tree->value[3] > 500) {
                                     fruit->value[2] = get_effect_efn("regeneration");
                                     tree->value[3] -= 100;
                                     break;
                               }
                               if (tree->value[3] > 1000) {
                                     fruit->value[2] = get_effect_efn("heal");
                                     tree->value[3] -= 200;
                                     break;
                               }
                               if (tree->value[3] > 3000) {
                                     fruit->value[2] = get_effect_efn("youth");
                                     tree->value[3] -= 600;
                                     break;
                               }
                               break;
                               
                           case TREE_DARKNESS:
                               fruit->short_descr = str_dup("a fruit of darkness");
                               if (tree->value[3] > 50) {
                                     fruit->value[2] = get_effect_efn("darkness");
                                     tree->value[3] -= 10;
                                     break;
                               }
                               if (tree->value[3] > 200) {
                                     fruit->value[2] = get_effect_efn("infravision");
                                     tree->value[3] -= 40;
                                     break;
                               }
                               if (tree->value[3] > 500) {
                                     fruit->value[2] = get_effect_efn("shield");
                                     tree->value[3] -= 100;
                                     break;
                               }
                               if (tree->value[3] > 1000) {
                                     fruit->value[2] = get_effect_efn("wrath of cthugha");
                                     tree->value[3] -= 200;
                                     break;
                               }
                               if (tree->value[3] > 3000) {
                                     fruit->value[2] = get_effect_efn("lich");
                                     tree->value[3] -= 600;
                                     break;
                               }
                               break;

                           case TREE_CHAOS:
                               fruit->short_descr = str_dup("a fruit of chaos");
                               if (tree->value[3] > 50) {
                                     fruit->value[2] = get_effect_efn("frenzy");
                                     tree->value[3] -= 10;
                                     break;
                               }
                               if (tree->value[3] > 200) {
                                     fruit->value[2] = get_effect_efn("giant strength");
                                     tree->value[3] -= 40;
                                     break;
                               }
                               if (tree->value[3] > 500) {
                                     fruit->value[2] = get_effect_efn("earthquake");
                                     tree->value[3] -= 100;
                                     break;
                               }
                               if (tree->value[3] > 1000) {
                                     fruit->value[2] = get_effect_efn("teleport");
                                     tree->value[3] -= 200;
                                     break;
                               }
                               if (tree->value[3] > 3000) {
                                     fruit->value[2] = get_effect_efn("cause riot");
                                     tree->value[3] -= 600;
                                     break;
                               }
                               break;

                           case TREE_ORDER:
                               fruit->short_descr = str_dup("a fruit of order");
                               if (tree->value[3] > 50) {
                                     fruit->value[2] = get_effect_efn("bless");
                                     tree->value[3] -= 10;
                                     break;
                               }
                               if (tree->value[3] > 200) {
                                     fruit->value[2] = get_effect_efn("fire shield");
                                     tree->value[3] -= 40;
                                     break;
                               }
                               if (tree->value[3] > 500) {
                                     fruit->value[2] = get_effect_efn("iron skin");
                                     tree->value[3] -= 100;
                                     break;
                               }
                               if (tree->value[3] > 1000) {
                                     fruit->value[2] = get_effect_efn("sanctuary");
                                     tree->value[3] -= 200;
                                     break;
                               }
                               if (tree->value[3] > 3000) {
                                     fruit->value[2] = get_effect_efn("aura");
                                     tree->value[3] -= 600;
                                     break;
                               }
                               break;

                           case TREE_NATURE:
                               fruit->short_descr = str_dup("a fruit of nature");
                               if (tree->value[3] > 50) {
                                     fruit->value[2] = get_effect_efn("earthquake");
                                     tree->value[3] -= 10;
                                     break;
                               }
                               if (tree->value[3] > 200) {
                                     fruit->value[2] = get_effect_efn("water breathing");
                                     tree->value[3] -= 40;
                                     break;
                               }
                               if (tree->value[3] > 500) {
                                     fruit->value[2] = get_effect_efn("giant strength");
                                     tree->value[3] -= 100;
                                     break;
                               }
                               if (tree->value[3] > 1000) {
                                     fruit->value[2] = get_effect_efn("create buffet");
                                     tree->value[3] -= 200;
                                     break;
                               }
                               if (tree->value[3] > 3000) {
                                     fruit->value[2] = get_effect_efn("restore limb");
                                     tree->value[3] -= 600;
                                     break;
                               }
                               break;
                     }

                     if (fruit->value[2] > 0) {
                        fruit->value[1] = UMIN(tree->value[3] / 10 + 1, 200);
                        tree->value[3] -= fruit->value[1] / 10;
                     }
                     tree->value[3] = UMAX(tree->value[3], 0);
               }
               return;
}


void update_align(CHAR_DATA *ch) {
   OBJ_DATA *obj;
   int pospower = 0;
   int negpower = 0;
   int balance, power;

   for (obj = ch->carrying; obj != NULL; obj=obj->next_content) {
         if (obj->wear_loc == WEAR_NONE
         || obj->level <1) continue;

         power = obj->level - ch->level + 10;
         if (power < 1) continue;

         if (IS_OBJ_STAT(obj, ITEM_BLESS)) pospower += power;
         if (IS_OBJ_STAT(obj, ITEM_EVIL)) negpower += power;
         if (IS_OBJ_STAT(obj, ITEM_NEUTRAL)) {
                negpower -= power;
                pospower -= power;
         }
   }         

   pospower = UMAX(pospower, 2);
   negpower = UMAX(negpower, 2);

   if (pospower >= negpower) {
       balance = 1000 - (1000 * negpower / pospower);
    } else {
       balance = 1000 - (1000 * pospower / negpower);
    }

    ch->alignment = ch->alignment - (ch->alignment - balance)/50;
    return;
}


bool update_age(CHAR_DATA *ch) {

    if (number_percent() > 5) return FALSE;
    if (number_percent() > 5) {
        send_to_char("{cYou back hurts terribly.{x\r\n", ch);
        return FALSE;
    } else {
        send_to_char("{cYou die of old age.{x\r\n", ch);
        raw_kill(ch, TRUE );
        delete_char(ch);
        return TRUE;
    }
}


void update_charge(OBJ_DATA *obj, AFFECT_DATA *paf) {
MOB_CMD_CONTEXT *mcc;
WEV *wev;

       if (obj->carried_by != NULL 
       && obj->zmask != 0
       && (obj->zmask & zones[obj->carried_by->in_zone].mask ) == 0 ) {
              return;
       }

       if (paf->modifier < 100) {
            if (number_percent() * 3  < obj->level + 25) {
                 paf->modifier -=paf->duration;
                 if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)) paf->modifier +=3;
                 if (IS_SET(obj->extra_flags, ITEM_MAGIC)) paf->modifier++;
                 if (IS_SET(obj->extra_flags, ITEM_GLOW)) paf->modifier++;
                 if (IS_SET(obj->extra_flags, ITEM_HUM)) paf->modifier++;
                 if (IS_SET(obj->extra_flags, ITEM_BLESS)) paf->modifier++;
                 if (IS_SET(obj->extra_flags, ITEM_DARK)) paf->modifier++;
                 if (IS_SET(obj->extra_flags, ITEM_NEUTRAL)) paf->modifier++;
             }
            paf->modifier = UMIN(paf->modifier, 100);
       } else {
            if (obj->carried_by
            && obj->wear_loc != WEAR_NONE) {
                 mcc = get_mcc(obj->carried_by, obj->carried_by, NULL, NULL, obj, NULL, paf->skill, effect_array[paf->skill].name);
                 wev = get_wev(WEV_OPROG, WEV_OPROG_EFFECT, mcc,
                              "@p2 casts @t0 on you.\r\n",
                              "",
                              "@p2 casts a spell on @a2.\r\n");

                 if (!room_issue_wev_challange(obj->carried_by->in_room, wev)) {
                       free_wev(wev);
                       return;
                 }

                 room_issue_wev(obj->carried_by->in_room, wev);
                 obj_cast_spell(paf->skill, obj->level, obj->carried_by, obj->carried_by, obj, NULL);
                 paf->modifier = 0;
                 free_wev(wev);

            }
       }
       return;
}


void produce_money(OBJ_DATA *obj, AFFECT_DATA *paf) {
CHAR_DATA *ch;
int currency = find_obj_currency(obj);
int amount;

        if (number_percent() > 10) return;

        if (!obj->carried_by
        || obj->wear_loc == WEAR_NONE) return;

        if (paf->modifier <= 0) return;

        ch = obj->carried_by;
        if (IS_NPC(ch)) return;

        amount = number_range(2, paf->modifier);

        ch->gold[currency] += amount;
        sprintf_to_char(ch, "%d %s fall out of %s.\r\n", amount, flag_string(currency_type, currency), obj->short_descr);
        return;
}


void produce_exp(OBJ_DATA *obj, AFFECT_DATA *paf) {
CHAR_DATA *ch;
int amount;

        if (number_percent() > 10) return;

        if (!obj->carried_by
        || obj->wear_loc == WEAR_NONE) return;

        if (paf->modifier <= 0) return;

        ch = obj->carried_by;
        if (IS_NPC(ch)) return;

        amount = number_range(1, paf->modifier);
        gain_exp(ch, amount, FALSE);
       
        sprintf_to_char(ch, "%s somehow makes you feel stronger.\r\n", capitalize(obj->short_descr));
        return;
}
  

void produce_sanity(OBJ_DATA *obj, AFFECT_DATA *paf) {
CHAR_DATA *ch;
int amount = 0;

        if (number_percent() > 15) return;

        if (!obj->carried_by
        || obj->wear_loc == WEAR_NONE) return;

        ch = obj->carried_by;
        if (IS_NPC(ch)) return;

        if (paf->modifier > 0) {
            if (ch->sanity < 60 + paf->modifier * 5) amount = number_range(1, paf->modifier);
        } else if (paf->modifier < 0){
            if (ch->sanity > 40 + paf->modifier * 5) amount = number_range(paf->modifier, -1);
        }

        ch->sanity +=amount;
        if (amount > 0) sprintf_to_char(ch, "%s gives you new confidence.\r\n", capitalize(obj->short_descr));
        else if (amount < 0) sprintf_to_char(ch, "%s discourages you.\r\n", capitalize(obj->short_descr));

        return;
}               


void modify_align(OBJ_DATA *obj, AFFECT_DATA *paf) {
CHAR_DATA *ch;
int amount;

        if (number_percent() > 10) return;

        if (!obj->carried_by
        || obj->wear_loc == WEAR_NONE) return;

        if (paf->modifier <= 0) return;

        ch = obj->carried_by;
        if (IS_NPC(ch)) return;

        amount = (paf->modifier - ch->alignment) / 15;
        ch->alignment += amount;
        if (amount > 0) send_to_char("You have a feeling of {Wholiness{x.\r\n", ch);
        else send_to_char("You have a feeling of {mevil{x.\r\n", ch);
        return;
}


void update_artifact(OBJ_DATA *obj) {
ARTIFACT *art = obj->pIndexData->artifact_data;
MOB_INDEX_DATA *pMob;
CHAR_DATA *mob;
int i;

    if (!art) return;
    if (IS_SET(obj->carried_by->in_room->room_flags, ROOM_SAFE)) return;

    if ((number_percent() * 5) - 5 < art->attract) {

        if (obj->pIndexData->vnum == OBJ_VNUM_QUEST_DOCUMENT) {
              mob = create_mobile_level(get_mob_index( MOB_VNUM_ASSASSIN ),"npc soldier", obj->level + number_range(0,15));
              char_to_room(mob, obj->carried_by->in_room);
              act( "$n suddenly appears, attracted by $p.", mob, obj, NULL, TO_ROOM);
              mob->prey = obj->carried_by;
              obj->carried_by->huntedBy = mob;
        } else {
            for (i = VNUM_SYSTEM_LOW; i <= VNUM_SYSTEM_HIGH; i++) {
                  if ((pMob =get_mob_index(i)) != NULL) {
                        if (pMob->spec_fun > 0) {
                            if (!str_cmp("spec_artifactHunter", spec_string(pMob->spec_fun))
                            || !str_cmp("spec_artifactTracker", spec_string(pMob->spec_fun))) {

                                  if (art->attract * 3 / 2 > pMob->level
                                  && number_percent() < 10) {
                                        mob = create_mobile(pMob);
                                        char_to_room(mob, obj->carried_by->in_room);
                                        act( "$n suddenly appears, attracted by $p.", mob, obj, NULL, TO_ROOM);
                                        mob->prey = obj->carried_by;
                                        obj->carried_by->huntedBy = mob;
                                        break;
                                  }
                            }
                        }
                   }
            }
        }
    }      
    return;
}


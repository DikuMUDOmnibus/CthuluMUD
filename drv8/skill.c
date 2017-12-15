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
#include "prof.h"
#include "econd.h"
#include "mob.h"
#include "society.h"
#include "exp.h"
#include "race.h"
#include "spell.h"
#include "wev.h"
#include "gsn.h"


/* Declare external functions not in Merc.h... */

DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(yith_adapt);
DECLARE_DO_FUN(do_skill_info);

extern bool check_blind(CHAR_DATA *ch);
void research_prof(CHAR_DATA *ch, int sn);

/* Master skill array... */

SKILL_INFO skill_array[MAX_SKILL];

/* Chain of free skill blocks... */

SKILL_DATA *skill_free;

/* Get a skill block... */

SKILL_DATA *getSkillData() {

  SKILL_DATA *sd;

  if (skill_free == NULL) {
    sd = (SKILL_DATA *) alloc_perm( sizeof(*sd) );
  } else {
    sd = skill_free;
    skill_free = skill_free->next;
    sd->next = NULL;
  } 

  clearSkillData(sd);

  return sd;
}

/* Free a skill data block... */

void freeSkillData(SKILL_DATA *sd) {

  sd->next = skill_free;
  skill_free = sd;

  return;
}

/* Wipe a set of skills... */

void clearSkillData(SKILL_DATA *sd) {
int i;

  sd->total_points = 0;

  for(i = 0; i < MAX_SKILL; i++) {
    sd->skill[i] = 0;
  }

  return;
}


/* Set the value of a skill... */

void setSkill(SKILL_DATA *sd, int sn, int pts) {

 /* Check if the skill number is valid... */

  if (!sd) return;
  if (sn >= MAX_SKILL || sn <= 0) return;
 
  sd->total_points -= sd->skill[sn];
  sd->skill[sn] = pts;
  sd->total_points += sd->skill[sn];
  return;
} 


/* Add points onto an existing skill... */

bool addSkill(CHAR_DATA *ch, int sn, int pts) {

  SKILL_DATA *sd;

  int skill, threshold;

 /* Find the skill data block... */

  if (!isSkilled(ch)) {
    return FALSE;
  }
  
  sd = ch->effective;

 /* Check it is a valid skill... */

  if (!validSkill(sn)) {
    return FALSE;
  }
 
 /* What do we have now... */

  skill = sd->skill[sn];

 /* Threshold for next change (diminishing returns)... */ 

  threshold = SKILL_ADEPT;

  if (skill >= SKILL_ADEPT) {
    pts /= SKILL_DIVIDER;
    threshold = SKILL_MADEPT;
  }

  if (skill >= SKILL_MADEPT) {
    pts /= SKILL_DIVIDER;
    threshold = SKILL_MASTER;
  }

  if (skill >= SKILL_MASTER) {
    pts /= SKILL_DIVIDER;
    threshold = SKILL_GMASTER;
  }

  if (skill >= SKILL_GMASTER) {
    pts /= SKILL_DIVIDER;
    threshold = SKILL_EXEMPLAR;
  }

  if (skill >= SKILL_EXEMPLAR) {
    pts /= SKILL_DIVIDER;
    threshold = 10000;
  }

 /* Can only learn a positive number of points... */

  if (pts <= 0) {
    return FALSE;
  }

 /* Adjust skill for threshold... */

  skill += pts;

  if (skill > threshold) {
    skill = threshold + ((skill - threshold)/SKILL_DIVIDER);
    gain_exp(ch, 5, FALSE);
  }

 /* Make the change... */

  sd->total_points -= sd->skill[sn];

  sd->skill[sn] = skill;

  sd->total_points += sd->skill[sn];

  return TRUE;

}

/* Add points onto an existing Mobs skill... */

bool addMobSkill(MOB_INDEX_DATA *pMob, int sn, int pts) {
SKILL_DATA *sd;
int skill, threshold;

 /* Find the skill data block... */

  if (pMob->skills == NULL) {
    return FALSE;
  }
  
  sd = pMob->skills;

 /* Check it is a valid skill... */

  if (!validSkill(sn)) {
    return FALSE;
  }
 
 /* What do we have now... */

  skill = sd->skill[sn];

 /* Threshold for next change (diminishing returns)... */ 

  threshold = SKILL_ADEPT;

  if (skill >= SKILL_ADEPT) {
    pts /= SKILL_DIVIDER;
    threshold = SKILL_MADEPT;
  }

  if (skill >= SKILL_MADEPT) {
    pts /= SKILL_DIVIDER;
    threshold = SKILL_MASTER;
  }

  if (skill >= SKILL_MASTER) {
    pts /= SKILL_DIVIDER;
    threshold = SKILL_GMASTER;
  }

  if (skill >= SKILL_GMASTER) {
    pts /= SKILL_DIVIDER;
    threshold = SKILL_EXEMPLAR;
  }

  if (skill >= SKILL_EXEMPLAR) {
    pts /= SKILL_DIVIDER;
    threshold = 10000;
  }

 /* Can only learn a positive number of points... */

  if (pts <= 0) {
    return FALSE;
  }

 /* Adjust skill for threshold... */

  skill += pts;

  if (skill > threshold) {
    skill = threshold + ((skill - threshold)/SKILL_DIVIDER);
  }

 /* Make the change... */

  sd->total_points -= sd->skill[sn];

  sd->skill[sn] = skill;

  sd->total_points += sd->skill[sn];

  return TRUE;

}


bool addCharSkill(CHAR_DATA *ch, int sn, int pts) {
SKILL_DATA *sd;
int skill, threshold;

 /* Find the skill data block... */

  if (ch->effective == NULL) return FALSE;
  
  sd = ch->effective;

 /* Check it is a valid skill... */

  if (!validSkill(sn)) return FALSE;
 
 /* What do we have now... */

  skill = sd->skill[sn];

 /* Threshold for next change (diminishing returns)... */ 

  threshold = SKILL_ADEPT;

  if (skill >= SKILL_ADEPT) {
    pts /= SKILL_DIVIDER;
    threshold = SKILL_MADEPT;
  }

  if (skill >= SKILL_MADEPT) {
    pts /= SKILL_DIVIDER;
    threshold = SKILL_MASTER;
  }

  if (skill >= SKILL_MASTER) {
    pts /= SKILL_DIVIDER;
    threshold = SKILL_GMASTER;
  }

  if (skill >= SKILL_GMASTER) {
    pts /= SKILL_DIVIDER;
    threshold = SKILL_EXEMPLAR;
  }

  if (skill >= SKILL_EXEMPLAR) {
    pts /= SKILL_DIVIDER;
    threshold = 10000;
  }

 /* Can only learn a positive number of points... */

  if (pts <= 0) return FALSE;

 /* Adjust skill for threshold... */

  skill += pts;
  if (skill > threshold) skill = threshold + ((skill - threshold)/SKILL_DIVIDER);

 /* Make the change... */

  sd->total_points -= sd->skill[sn];
  sd->skill[sn] = skill;

  sd->total_points += sd->skill[sn];
  return TRUE;
}


/* See if a skill number is valid... */

bool validSkill(const int sn) {
 
  if (sn <= 0) {
    return FALSE;
  }

  if (sn >= MAX_SKILL) {
    return FALSE;
  }

  if (!skill_array[sn].loaded) {
    return FALSE;
  }

  return TRUE;
}

/* See if a character is skilled... */

bool isSkilled(const CHAR_DATA *ch) {

  if (ch->effective == NULL) {
    return FALSE;
  }

  return TRUE;
}

/* See if a specific skill is available to them... */

bool isSkillAvailable (CHAR_DATA *ch, int sn) {

  MOB_CMD_CONTEXT *mcc;

 /* Everyone gets level skills... */

  if (sn == -1) {
    return TRUE;
  }

 /* No-one gets invalid skills... */

  if (!validSkill(sn)) {
    return FALSE;
  }  

 /* Immortals get everything... */

  if (IS_IMMORTAL(ch)) {
    return TRUE;
  }

 /* So do unskilled mobs... */

  if (!isSkilled(ch)) {
    return TRUE;
  }

 /* If you've already got it, you can keep it... */

  if( ch->effective->skill[sn] > 0 ) {
    return TRUE;
  } 

 /* Check prereqs... */

  mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL);

  if (ec_satisfied_mcc(mcc, skill_array[sn].ec, TRUE)) {
    free_mcc(mcc);
    return TRUE;
  }
 
  free_mcc(mcc);

 /* Otherwise, no skill... */
 
  return FALSE;
}

/* See if a specific skill is available to be practiced by them... */

bool isSkillAvailableForPractice (CHAR_DATA *ch, int sn) {
MOB_CMD_CONTEXT *mcc;

 /* Everyone gets level skills... */
  if (sn == -1) return TRUE;

 /* No-one gets invalid skills... */
  if (!validSkill(sn)) return FALSE;

 /* Immortals get everything... */
  if (IS_IMMORTAL(ch)) return TRUE;

 /* If you've already got it, you can keep it... */

  if ( isSkilled(ch)  
    && ch->effective->skill[sn] > 0 ) {
    return TRUE;
  } 

 /* First check prereqs... */

  mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL);

  if (!ec_satisfied_mcc(mcc, skill_array[sn].ec, TRUE)) {
    free_mcc(mcc);
    return FALSE;
  }

  free_mcc(mcc);

 /* ...then check profession table... */

  if (skill_available_by_prof(sn, ch)) {
    return TRUE;
  }

 /* ...then check society professions... */

  if (skill_available_by_society(sn, ch)) {
    return TRUE;
  }

 /* Otherwise, no skill... */
 
  return FALSE;
}

/* Scale a mobs skill... */

int scale_skill(CHAR_DATA *ch, int base, int stat, int per) {
  int skill;
  skill = base;

  if (stat != STAT_NONE) { 
    skill += (get_curr_stat(ch, stat)/4);
  }

  skill += (per * ch->level)/2;

  if (skill > 75) {
    skill = 75 + ((skill - 75)/2);
  }

  if (skill > 110) {
    skill = 110 + ((skill - 110)/2);
  }

  if (skill > 150) {
    skill = 150 + ((skill - 150)/2);
  }

  return skill;
}

/* Return a characters skill... */

int get_skill(CHAR_DATA *ch, int sn) {
int skill;
char buf[MAX_STRING_LENGTH];

 /* Sn SKILL_UNDEFINED means it is a level based skill.. */

  if (sn == SKILL_UNDEFINED) {
    skill = scale_skill(ch, 0, STAT_NONE, 5);
    if (IS_AFFECTED(ch, AFF_BERSERK)) skill -= ch->level / 2;

   /* Dreaming characters can do things a little bit better... */

    if ( ch->waking_room != NULL ) {
         if (ch->nightmare) {
             if ( isSkilled(ch) ) skill -= (ch->effective->skill[gsn_dreaming] / 10); 
             else skill -= (ch->level/10);
         } else {
             if ( isSkilled(ch) ) skill += (ch->effective->skill[gsn_dreaming] / 10); 
             else skill += (ch->level/10);
         }
    }

   /* Drunkeness reduces all skills... */

    if ( ch->condition[COND_DRUNK] > COND_STUPOR ) {
      skill -= 50;
    } else if ( ch->condition[COND_DRUNK] > COND_DIZZY ) {
      skill -= 30;
    } else if ( ch->condition[COND_DRUNK] > COND_DRUNK2 ) {
      skill -= 10;
    }  

    return UMAX(0, skill);
  }
 
 /* Otherwise it must be a known skill... */

  if (!validSkill(sn)) {
    sprintf(buf, "%s: Bad sn %d in get_skill.", ch->name, sn);
    bug(buf, 0);
    return 0;
  }

 /* For characters with skills, we just look it up... */  

  if (isSkilled(ch)) {

    if (ch->desc != NULL) {
          if (ch->desc->original != NULL ) {
                if (!str_cmp(race_array[ch->desc->original->race].name,"yithian")) {
                     skill = (ch->effective->skill[sn] + ch->desc->original->effective->skill[sn]) / 2;
                } else {
                     skill = ch->effective->skill[sn];
                }
          } else {
                skill = ch->effective->skill[sn];
          }
    } else {
          skill = ch->effective->skill[sn];
    }

    if ( skill > 0 ) {

     /* Berserk characters have a problem... */
       if (IS_AFFECTED(ch, AFF_BERSERK)) skill -= ch->level / 2;
      
     /* Dreaming characters can do things a little bit better... */

      if ( ch->waking_room != NULL ) {
           if (ch->nightmare) skill -= (ch->effective->skill[gsn_dreaming] / 10); 
           else skill += (ch->effective->skill[gsn_dreaming] / 10); 
      }

     /* High sanity reduces certain skills... */

      if ( sn == gsn_dreaming
      || sn == gsn_spell_casting ) {

        if (get_sanity(ch) > 120 ) {

          skill -= (( get_sanity(ch) - 120 ) / 2 );

        }
      } 

     /* Drunkeness reduces all skills... */

      if ( ch->condition[COND_DRUNK] > COND_STUPOR ) {
        skill -= 50;
      } else if ( ch->condition[COND_DRUNK] > COND_DIZZY ) {
        skill -= 30;
      } else if ( ch->condition[COND_DRUNK] > COND_DRUNK2 ) {
        skill -= 10;
      }  

    }

    return UMAX(0, skill);
  }

 /* Ok, just unskilled mobiles left... */
 
  skill = 0;
 
  if (sn == gsn_sneak) {
              skill = scale_skill(ch, 10, STAT_DEX, 4);

  } else if ( sn == gsn_second_attack && ch->level > 10 ) {
 
           if ( IS_SET(ch->act, ACT_WARRIOR) 
             || IS_SET(ch->act, ACT_THIEF) ) {
              skill = scale_skill(ch, 30, STAT_DEX, 8);
           } else {
              skill = scale_skill(ch, 25, STAT_DEX, 4);
           }
  
  } else if ( sn == gsn_third_attack 
         && ch->level > 20 ) {
 
           if (IS_SET(ch->act, ACT_WARRIOR)) {
              skill = scale_skill(ch, 20, STAT_DEX, 6);
           } else {
              skill = scale_skill(ch, 15, STAT_DEX, 2);
           }

  } else if ( sn == gsn_fourth_attack 
           && ch->level > 40 ) {
  
           if ( IS_SET(ch->act, ACT_WARRIOR) ) {
              skill = scale_skill(ch, 10, STAT_DEX, 4);
           }

  } else if ( sn == gsn_hand_to_hand ) {

           if ( IS_SET(ch->act, ACT_WARRIOR) ) {
              skill = scale_skill(ch, 35, STAT_STR, 8);
           } else {
     	      skill = scale_skill(ch, 30, STAT_STR, 6);
           }

  } else if ( sn == gsn_martial_arts 
           && ch->level > 10 ) {
  
           if ( IS_SET(ch->act, ACT_WARRIOR)
             || IS_SET(ch->act, ACT_THIEF) ) {
              skill = scale_skill(ch, 20, STAT_DEX, 6);
           } else {
              skill = scale_skill(ch, 15, STAT_DEX, 3);
           }

  } else if ( sn == gsn_black_belt 
           && ch->level > 30 ) {
  
           if ( IS_SET(ch->act, ACT_WARRIOR) ) {
              skill = scale_skill(ch, 10, STAT_DEX, 4);
           } else if ( IS_SET(ch->act, ACT_THIEF) ) {
              skill = scale_skill(ch,  5, STAT_DEX, 2);
           } else {
             skill = 0;
           }

  } else if ( sn == gsn_enhanced_damage 
           && ch->level > 10 ) {
  
           if ( IS_SET(ch->act, ACT_WARRIOR)
             || IS_SET(ch->act, ACT_THIEF) ) {
              skill = scale_skill(ch, 20, STAT_DEX, 4);
           } else { 
              skill = scale_skill(ch, 15, STAT_DEX, 2);
           }

  } else if ( sn == gsn_ultra_damage 
           && ch->level > 25 ) { 
      
           if ( IS_SET(ch->act, ACT_WARRIOR) ) {
              skill = scale_skill(ch, 10, STAT_DEX, 3);
           } else {
              skill = scale_skill(ch,  5, STAT_DEX, 2);
           } 

  } else if ( sn == gsn_lethal_damage 
           && ch->level > 40 ) {
 
           if ( IS_SET(ch->act, ACT_WARRIOR) ) {
              skill = scale_skill(ch, 0, STAT_DEX, 2);
           } else {
              skill = 0;
           } 
  
  } else if ( sn == gsn_trip 
           && IS_SET(ch->off_flags, OFF_TRIP) ) {

           if ( IS_SET(ch->act, ACT_WARRIOR) ) {
	      skill = scale_skill(ch, 20, STAT_DEX, 6);
           } else {
              skill = scale_skill(ch, 15, STAT_DEX, 4);
           } 

  } else if ( sn == gsn_bash 
           && IS_SET(ch->off_flags, OFF_BASH) ) {

           if ( IS_SET(ch->act, ACT_WARRIOR) ) {
	      skill = scale_skill(ch, 20, STAT_DEX, 6);
           } else {
              skill = scale_skill(ch, 15, STAT_DEX, 4);
           } 

  } else if ( sn == gsn_crush 
           && IS_SET(ch->off_flags, OFF_CRUSH) ) {

           skill = 5 * get_char_size(ch);

           if ( IS_SET(ch->act, ACT_WARRIOR) ) {
	      skill = scale_skill(ch, skill + 5, STAT_STR, 6);
           } else {
              skill = scale_skill(ch, skill, STAT_DEX, 4);
           } 

  } else if ( sn == gsn_tail 
           && IS_SET(ch->off_flags, OFF_TAIL) ) {

           if ( IS_SET(ch->act, ACT_WARRIOR) ) {
	      skill = scale_skill(ch, 30, STAT_DEX, 6);
           } else {
              skill = scale_skill(ch, 20, STAT_DEX, 4);
           } 

  } else if ( sn == gsn_disarm 
	    && ( IS_SET(ch->off_flags, OFF_DISARM) 
	      || IS_SET(ch->off_flags, ACT_WARRIOR)
	      || IS_SET(ch->off_flags, ACT_THIEF))) {
	      skill = scale_skill(ch, 30, STAT_DEX, 6);

  } else if ( sn == gsn_dual 
	    && ( IS_SET(ch->off_flags, ACT_WARRIOR)
	      || IS_SET(ch->off_flags, ACT_THIEF))) {
	      skill = scale_skill(ch, 30, STAT_DEX, 6);
 
  } else if ( sn == gsn_berserk 
           && IS_SET(ch->off_flags, OFF_BERSERK)) {
 	      skill = scale_skill(ch, 10, STAT_STR, 6);

  } else if ( sn == gsn_sword
 	   || sn == gsn_dagger
	   || sn == gsn_spear
	   || sn == gsn_mace
	   || sn == gsn_axe
	   || sn == gsn_flail
	   || sn == gsn_whip
           || sn == gsn_gun
           || sn == gsn_bow
           || sn == gsn_handgun  
           || sn == gsn_smg  
           || sn == gsn_staff  
	   || sn == gsn_polearm) {

           if ( IS_SET(ch->act, ACT_WARRIOR) ) {
     	      skill = scale_skill(ch, 35, STAT_STR, 8);
           } else {
     	      skill = scale_skill(ch, 30, STAT_STR, 6);
           }

  } else if ( sn == gsn_sword_master
 	   || sn == gsn_dagger_master
	   || sn == gsn_spear_master
	   || sn == gsn_mace_master
	   || sn == gsn_axe_master
	   || sn == gsn_flail_master
	   || sn == gsn_whip_master
	   || sn == gsn_staff_master
	   || sn == gsn_master_archer
           || sn == gsn_marksman
  	   || sn == gsn_polearm_master ) {

           if ( ch->level > 20 ) {
             if ( IS_SET(ch->act, ACT_WARRIOR) ) {
       	        skill = scale_skill(ch, 10, STAT_DEX, 5);
             } else {
     	        skill = 0;
             } 
           }

  } else if ( sn == gsn_recall) {
              skill = scale_skill(ch, 40, STAT_WIS, 2);

  } else if ( sn == gsn_parry 
           || sn == gsn_shield_block 
           || sn == gsn_dodge) {

           if ( IS_SET(ch->act, ACT_WARRIOR) ) {
       	      skill = scale_skill(ch, 15, STAT_DEX, 5);
           } else {
     	      skill = scale_skill(ch, 05, STAT_DEX, 3);
           }

  } else if ( sn == gsn_spell_casting
         && ( IS_SET(ch->act, ACT_MAGE)
           || IS_SET(ch->act, ACT_CLERIC)
           || IS_SET(ch->act, ACT_UNDEAD)
           || IS_SET(ch->act, ACT_IS_HEALER) )) {
       	      skill = scale_skill(ch, 40, STAT_INT, 6);

  } else if ( sn == gsn_natural_magic ) {
     	      skill = scale_skill(ch, 50, STAT_CON, 8);

  } else if ( sn == gsn_latin
           && ( IS_SET(ch->act, ACT_CLERIC)
             || IS_SET(ch->act, ACT_MAGE))) {
              skill = 60;

  } else if ( sn == gsn_greek
           && IS_SET(ch->act, ACT_MAGE)) {
              skill = 60;

  } else if (sn == gsn_swim ) {
              skill = scale_skill(ch, 20, STAT_CON, 2);

  } else if ( sn == gsn_english ) {
              skill = UMAX(scale_skill(ch, 35, STAT_INT, 5), 51);
   
  } else if ( sn == gsn_dreaming ) {
              skill = scale_skill(ch, 0, STAT_WIS, 1);

  } else if ( sn == gsn_debating ) {
              skill = scale_skill(ch, 0, STAT_INT, 2);

  } else if ( sn == gsn_tracking ) {
              skill = scale_skill(ch, 20, STAT_INT, 2);

  } else if ( sn == race_array[ch->race].language ) {
              skill = 60;
  } 

  if (ch->pIndexData) {
     int language = 0;

     if (race_array[ch->race].language > 0) language = race_array[ch->race].language;
     if (ch->pIndexData->language > 0) language = ch->pIndexData->language;

     if (language > 0
     && language == sn) {
          skill = UMAX(scale_skill(ch, 35, STAT_INT, 5), 51);
     }
  }

 /* Modifiers only to possessed skills... */

    if (ch->desc != NULL) {
          if (ch->desc->original != NULL ) {
                if (!str_cmp(race_array[ch->desc->original->race].name,"yithian")) {
                     skill = (skill + ch->desc->original->effective->skill[sn]) / 2;
                }
          }
    }

  if (skill > 0 ) {

   /* Confusion from berserk... */
    if (IS_AFFECTED(ch, AFF_BERSERK)) skill -= ch->level / 2;
    
   /* Dreaming characters can do things a little bit better... */

    if ( ch->waking_room != NULL ) {
        if (ch->nightmare) skill -= (ch->level/10);
        else skill += (ch->level/10);
    }

   /* Drunkeness reduces all skills... */

    if ( ch->condition[COND_DRUNK] > COND_STUPOR ) {
      skill -= 50;
    } else if ( ch->condition[COND_DRUNK] > COND_DIZZY ) {
      skill -= 30;
    } else if ( ch->condition[COND_DRUNK] > COND_DRUNK2 ) {
      skill -= 10;
    }  

  }

  return UMAX(0,skill);
}

/* Work out a characters weapon skill... */

int get_weapon_skill(CHAR_DATA *ch, int sn) {
  return get_skill(ch, sn);
} 

/* Get a characters skill modified by another skill... */

int get_skill_by(CHAR_DATA *ch, int sn, int sn2) {

  return (get_skill(ch, sn) * get_skill(ch, sn2)) / 100;
} 

/* Work out a characters base skill... */

int get_base_skill(int sn, CHAR_DATA *ch) {

  int skill = 0;
  int stat = STAT_NONE;
  int i;

  if (!validSkill(sn)) {
    return 0;
  }

  skill = 42;

  for(i = 0; i < 2; i++) {

    stat = skill_array[sn].stats[i];

    if (stat != STAT_NONE) {
      skill = skill - 4 + (ch->perm_stat[stat]/12);
    }
  } 

  return number_fuzzy(skill);
}

/* See if a skill works... */

bool check_skill(CHAR_DATA *ch, int sn, int diff) {
  int roll;

  roll = number_open() + get_skill(ch, sn);

  if (roll < (100 + diff))  return FALSE;
  return TRUE;
}

/* See if a character has a skill... */

bool has_skill(CHAR_DATA *ch, int sn) {

  if (!isSkilled(ch)) {
    return FALSE;
  }

  if (ch->effective->skill[sn] > 0) {
    return TRUE;
  }  

  return FALSE;
}

/* Get a skill roll... */

int get_skill_roll(CHAR_DATA *ch, int sn) {
 
  return number_open() + get_skill(ch, sn);
}

/* Skills improve through usage... */

void check_improve(CHAR_DATA *ch, int sn, bool success, int multiplier) {

  int chance;
  char buf[100];
  int skill;
  int gain;

 /* Is it a learnable skill? */
 
  if (sn <= 0) {
    return;
  }

 /* Only those with skills may improve them... */

  if (!isSkilled(ch)) {
    return;
  }

 /* See if they character is allowed to learn... */

  if ( !isSkillAvailable(ch, sn)) { 
    return;
  }
 
 /* See what they already know... */

  skill = ch->effective->skill[sn];

 /* Scale the multiplier up... */

  multiplier *= 100;

 /* Adjust multiplier for learning difficulty... */

  multiplier = (multiplier * 100) / skill_array[sn].learn;

 /* Adjust 'multiplier' for higher skills... */ 

  if (skill >= SKILL_MADEPT) {
    multiplier *= 2;
  }

  if (skill >= SKILL_MASTER) {
    multiplier *= 2;
  }

  if (skill >= SKILL_GMASTER) {
    multiplier *= 2;
  }

  if (skill >= SKILL_EXEMPLAR) {
    multiplier *= 2;
  }

 /* Scale the multiplier down... */
 
  multiplier = (4 * multiplier) / 100;

  if (multiplier <= 0) {
    return;
  }

 /* Check to see if the character has a chance to learn... */

  chance  = 10 * int_app[get_curr_stat(ch,STAT_INT)].learn;   /* 220 */
  chance += ch->level/4;                                      /* 250 */
  chance /= multiplier;                                       /* <40 */

  if (number_range(1,1000) > chance) {
    return;
  } 

 /* Now that the character has a CHANCE to learn, see if they really have */	

  if (success) {

    chance = URANGE(5, 100 - ch->effective->skill[sn], 95);

    if (number_percent() < chance) {
      sprintf(buf,"You have become better at %s!\r\n", skill_array[sn].name);
      send_to_char(buf,ch);
      gain = 1;
      ch->effective->skill[sn] += gain;
      ch->effective->total_points += gain;
      gain_exp(ch, 2, FALSE);
    }

  } else {

    chance = URANGE(5, ch->effective->skill[sn]/2, 30);

    if (number_percent() < chance) {
      sprintf(buf,
             "You learn from your mistakes, and your %s skill improves.\r\n",
	      skill_array[sn].name);
      send_to_char(buf,ch);
      gain = number_range(1,3);
      ch->effective->skill[sn] += gain;
      ch->effective->total_points += gain;
      gain_exp(ch, 2, FALSE);
    }
  }
  
  return;
}


/* The practice command... */

void do_practice( CHAR_DATA *ch, char *argument ) {
char buf[MAX_STRING_LENGTH];
int sn;
CHAR_DATA *mob;
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
int num_prac = 0;
int num_max = 0;
int before = 0;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

 /* Only those who are skilled may practice... */

  if ( !isSkilled(ch) ) return;
  
 /* Berserkers won't sit still long enough to learn anything... */

  if (IS_AFFECTED(ch, AFF_BERSERK)) {
    send_to_char("{rYou are to {Rangry{x to worry about skills..."
                              "you just want to {RKILL SOMEONE!{x\r\n", ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_MELD)) {
        send_to_char("You're feeling stupid.\r\n", ch);
        return;
  }

 /* No parms, means a review of the characters current skills... */

  if ( argument[0] == '\0' ) {
    int col;
    int skill;
    char outbuf[MAX_SKILL * 50];
    outbuf[0] = '\0';

    sprintf(outbuf, "You can practice these skills:\r\n"); 

    col    = 0;
    for ( sn = 0; sn < MAX_SKILL; sn++ ) {
      if ( !skill_array[sn].loaded )  break;
      if (!isSkillAvailableForPractice(ch, sn))  continue;

      skill = get_skill(ch, sn);

      if (skill < 1) {
        strcat(outbuf, "{x");
      } else if (skill < SKILL_ADEPT) {
        strcat(outbuf, "{g");
      } else if (skill < SKILL_MASTER) {
        strcat(outbuf, "{c");
      } else if (skill < SKILL_EXEMPLAR) {
        strcat(outbuf, "{w");
      } else {
        strcat(outbuf, "{Y");
      }

      sprintf( buf, "%-18s %3d   ", skill_array[sn].name, skill);
      strcat(outbuf, buf);

      if ( ++col % 3 == 0 ) strcat(outbuf, "{x\r\n" );
    }

    if ( col % 3 != 0 ) strcat(outbuf, "{x\r\n");
     
    sprintf( buf, "{wYou have {W%ld{w skill points in total "
                    "and {W%d{w practice sessions left.{x\r\n",
                     ch->effective->total_points, ch->practice );

    strcat( outbuf, buf);

    if (IS_NPC(ch)
    && IS_AFFECTED(ch, AFF_CHARM)
    && ch->master!=NULL) {
        page_to_char( outbuf, ch->master );
   } else {
        page_to_char( outbuf, ch );
   }
    return;
  } 

 /* No sleeping in class... */

  if ( !IS_AWAKE(ch) ) {
    send_to_char( "No sleeping in class!\r\n", ch );
    return;
  }

 /* First argument is skill... */

  argument = one_argument(argument, arg1); 

 /* Check if the skill can be practiced... */ 

  sn = get_skill_sn( arg1 );

  if (  sn < 0
    || !isSkillAvailableForPractice(ch, sn) ) {
    send_to_char( "You can't practice that.\r\n", ch );
    return;
  }
 
 /* Second argument is number of practice sessions... */

  if (argument[0] != '\0') {
    if (argument[0] == '>' ) {
        argument = one_argument(argument, arg2); 
        num_max = atoi(argument);
        num_max=UMIN(num_max, 130);
        num_prac = 1;
    } else {
        num_prac = atoi(argument);
        num_max = 0;
    }
  } else {
    num_prac = 1;
    num_max = 0;
  }

  if ( ch->practice < num_prac ) {
    send_to_char( "You don't have enough practice sessions left.\r\n", ch );
    return;
  }

  if (num_prac == 0) {
    send_to_char("Syntax: practice 'skill' count\r\n", ch);
    return;
  } 

 /* Find a teacher... */

  for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room ) {
    if ( IS_NPC(mob) 
    && IS_SET(mob->act, ACT_PRACTICE)) break;
  }

  if ( mob == NULL ) {
    send_to_char( "You practice alone...\r\n", ch );
    mob = ch;
  }

 /* Go learn the skill... */

        mcc = get_mcc(ch, ch, mob, NULL, NULL, NULL, get_skill_sn(arg1), arg1);
        wev = get_wev(WEV_LEARN, WEV_LEARN_PRACTICE, mcc,
                    NULL,
                    NULL,
                    NULL);

        if (!room_issue_wev_challange(ch->in_room, wev)) {
            free_wev(wev);
            return;
        }

  if (num_max > 0
  && get_skill(ch, get_skill_sn(arg1)) < num_max) {
       before =0;
       while (get_skill(ch, get_skill_sn(arg1)) < num_max
       && ch->practice > 0
       && before < get_skill(ch, get_skill_sn(arg1))) {
              before = get_skill(ch, get_skill_sn(arg1)); 
              practice_skill(ch, mob, 1, arg1, TRUE);
     }
  } else {
      practice_skill(ch, mob, num_prac, arg1, TRUE);
  }

        room_issue_wev( ch->in_room, wev);
        free_wev(wev);

  return;
}


/* Learn a skill from a teacher... */

void do_learn( CHAR_DATA *ch, char *argument ) {
char buf[MAX_STRING_LENGTH];
int sn;
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
int num_prac;
CHAR_DATA *teacher;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

 /* Skills players and mobs only... */ 

  if ( !isSkilled(ch)) return;
  
 /* No sleeping in class... */

  if (!IS_AWAKE(ch) ) {
    send_to_char( "No sleeping in class...\r\n", ch );
    return;
  }

      if (IS_AFFECTED(ch, AFF_MELD)) {
        send_to_char("You're feeling stupid.\r\n", ch);
        return;
      }

 /* Syntax: learn teacher skill number */

  argument = one_argument(argument, arg1); 
  argument = one_argument(argument, arg2); 

 /* Must have a teacher... */

  if (arg1[0] == 0) {
    send_to_char("Syntax: learn 'teacher' <'skill' <count>>\r\n", ch);
    return;
  }

  teacher = get_char_room(ch, arg1);

  if (teacher == NULL) {
    send_to_char("No teacher by that name here.\r\n", ch);
    return;
  }

  if ( teacher == ch ) {
    send_to_char("You give yourself a stern lecture.\r\n", ch);
    return;
  }

 /* Helps if the teacher can teach... */

  if ( !isSkilled(teacher) ) {

    if ( !IS_SET(teacher->act, ACT_PRACTICE) ) {
      sprintf(buf, "%s can't teach you anything!\r\n", capitalize(teacher->short_descr));
      send_to_char(buf, ch);
      return;  
    } else {
      sprintf(buf, "Use practice to learn from %s!\r\n", capitalize(teacher->short_descr));
      send_to_char(buf, ch);
      return;  
    }
  }
 
  if ( isSkilled(teacher) 
    && get_skill(teacher, gsn_teach) < 25 ) {
    sprintf(buf, "%s isn't a very good teacher.\r\n", capitalize(teacher->short_descr));
    send_to_char(buf, ch);
    return;  
  }

 /* Translate the number of practices if we have one... */
 
  if (argument[0] != '\0') num_prac = atoi(argument);
  else num_prac = 1;
  
  if (num_prac == 0) {
    send_to_char("Syntax: learn 'teacher' <'skill' <count>>\r\n", ch);
    return;
  } 

 /* Berserkers don't pay attention... */ 

  if (IS_AFFECTED(ch, AFF_BERSERK)) {
    send_to_char("{rYou are to {Rangry{x to worry about learning..."
                              "you just want to {RKILL SOMEONE!{x\r\n", ch);
    return;
  }

 /* No skill, means we show a prospectus... */

  if ( arg2[0] == '\0' ) {
    int col;
    int skill;
    char outbuf[MAX_SKILL * 50];

   /* Short display for unskilled mobs... */

    if (!isSkilled(teacher)) {
      if ( teacher->pIndexData != NULL
        && teacher->pIndexData->pShop != NULL
        && teacher->pIndexData->pShop->profit_buy > 0) {
        sprintf(outbuf, "%s can teach you many things, for a fee.\r\n"
                                   "You have {W%d{x practice sessions left.\r\n", capitalize(teacher->short_descr), ch->practice);
      } else {
        sprintf(outbuf, "%s can teach you many things.\r\n"
                                   "You have {W%d{x practice sessions left.\r\n", capitalize(teacher->short_descr), ch->practice);
      }

      send_to_char(outbuf, ch); 
      return;
    }

    if ( teacher->pIndexData != NULL
      && teacher->pIndexData->pShop != NULL
      && teacher->pIndexData->pShop->profit_buy > 0) {
      sprintf(outbuf, "%s can teach you, for a fee:\r\n", capitalize(teacher->short_descr));
    } else {
      sprintf(outbuf, "%s can teach you:\r\n", capitalize(teacher->short_descr));
    }

    col    = 0;
    for ( sn = 0; sn < MAX_SKILL; sn++ ) {
	
      if ( !skill_array[sn].loaded ) break;
      if (!isSkillAvailable(teacher, sn)) continue;
      skill = get_skill_by(teacher, sn, gsn_teach);

      if (skill < 25) {
        continue;
      } else if (skill < SKILL_ADEPT) {
        strcat(outbuf, "{g");
      } else if (skill < SKILL_MASTER) {
        strcat(outbuf, "{c");
      } else if (skill < SKILL_EXEMPLAR) {
        strcat(outbuf, "{w");
      } else {
        strcat(outbuf, "{Y");
      }

      sprintf( buf, "%-18s %3d   ", skill_array[sn].name, skill);
      strcat(outbuf, buf);
      if ( ++col % 3 == 0 ) strcat(outbuf, "{x\r\n" );
    }

    if ( col % 3 != 0 ) strcat(outbuf, "{x\r\n");
    sprintf( buf, "{wYou have {W%d{w practice sessions left.{x\r\n", ch->practice );
    strcat( outbuf, buf);
    page_to_char( outbuf, ch );
    return;
  } 

 /* You can only ask PCs to teach you... */

  if (!IS_NPC(teacher)) {
    sprintf(buf, "You ask %s to teach you '%s'.\r\n", teacher->short_descr, arg2);
    send_to_char(buf, ch);
    sprintf(buf, "%s asks you to teach them '%s'.\r\n", capitalize(ch->short_descr), arg2);
    send_to_char(buf, teacher);
    return;
  }

 /* Ok, learn a specific skill... */ 
        mcc = get_mcc(ch, ch, teacher, NULL, NULL, NULL, get_skill_sn(arg2), arg2);
        wev = get_wev(WEV_LEARN, WEV_LEARN_LEARN, mcc,
                    NULL,
                    NULL,
                    NULL);

        if (!room_issue_wev_challange(ch->in_room, wev)) {
            free_wev(wev);
            return;
        }

  practice_skill(ch, teacher, num_prac, arg2, FALSE);
  room_issue_wev( ch->in_room, wev);
  free_wev(wev);
  return;
}

/* Teach a skill to a pupil... */

void do_teach( CHAR_DATA *ch, char *argument ) {

  char buf[MAX_STRING_LENGTH];
  int sn;

  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int num_prac;

  CHAR_DATA *pupil;
 
 /* Skilled players and mobs only... */ 

  if ( !isSkilled(ch) ) {
    send_to_char("You may not do this, you are not skilled.\r\n", ch);
    return;
  }

 /* Berserkers don't pay attention... */ 

  if (IS_AFFECTED(ch, AFF_BERSERK)) {
    send_to_char("{rYou are to {Rangry{x to teach anyone anything..."
                 "you just want to {RKILL SOMEONE!{x\r\n", ch);
    return;
  }

 /* Helps if the teacher can teach... */

  if ( isSkilled(ch) 
    && get_skill(ch, gsn_teach) < 25 ) {
    send_to_char("You aren't a good enough teacher yet.\r\n", ch); 
    return;  
  }

 /* No sleeping in class... */

  if ( !IS_AWAKE(ch) ) {
    send_to_char( "Not when you're asleep...\r\n", ch );
    return;
  }

 /* Syntax: teach pupil skill number */

  argument = one_argument(argument, arg1); 
  argument = one_argument(argument, arg2); 

 /* Must have a pupil... */

  if (arg1[0] == 0) {
    send_to_char("Syntax: teach 'pupil' <'skill' <count>>\r\n", ch);
    return;
  }

  pupil = get_char_room(ch, arg1);

  if (pupil == NULL) {
    send_to_char("No pupil by that name here.\r\n", ch);
    return;
  }

  if ( pupil == ch ) {
    send_to_char("You give yourself a stern lecture!\r\n", ch);
    return;
  }

 /* Helps if they can learn... */

  if (!isSkilled(pupil)) {
    sprintf(buf, "You can't teach %s anything.\r\n",
                  pupil->short_descr);
    send_to_char(buf, ch);
    return;
  } 

  if (IS_AFFECTED(pupil, AFF_BERSERK)) {
    sprintf(buf, "%s won't pay attention to you.\r\n",
                  capitalize(pupil->short_descr));
    send_to_char(buf, ch);
    return;
  } 

 /* They must also be following you... */

  if (pupil->master != ch) {
    send_to_char("You can only teach people that are grouped to you.\r\n", ch);
    return;
  }

 /* Translate the number of practices if we have one... */
 
  if (argument[0] != '\0') {
    num_prac = atoi(argument);
  } else {
    num_prac = 1;
  }

  if (num_prac == 0) {
    send_to_char("Syntax: teach 'pupil' <'skill' <count>>\r\n", ch);
    return;
  } 

 /* No skill, means we show a prospectus... */

  if ( arg2[0] == '\0' ) {
    
    int col;
    int skill;
    char outbuf[MAX_SKILL * 50];

   /* Short display for unskilled mobs... */

    sprintf(outbuf, "You can teach %s:\r\n",
                     pupil->short_descr); 

    col    = 0;
    for ( sn = 0; sn < MAX_SKILL; sn++ ) {
	
      if ( !skill_array[sn].loaded )
        break;

      if (!isSkillAvailable(ch, sn))
        continue;

      if (!isSkillAvailable(pupil, sn))
        continue;

      skill = get_skill_by(ch, sn, gsn_teach);

      if (skill < 25) {
        continue;
      } else if (skill < SKILL_ADEPT) {
        strcat(outbuf, "{g");
      } else if (skill < SKILL_MASTER) {
        strcat(outbuf, "{c");
      } else if (skill < SKILL_EXEMPLAR) {
        strcat(outbuf, "{w");
      } else {
        strcat(outbuf, "{Y");
      }

      sprintf( buf, "%-18s %3d   ", skill_array[sn].name, skill);

      strcat(outbuf, buf);

      if ( ++col % 3 == 0 ) {
        strcat(outbuf, "{x\r\n" );
      } 
    }

    if ( col % 3 != 0 ) {
      strcat(outbuf, "{x\r\n");
    }
 
    page_to_char( outbuf, ch );

    return;
  } 

 /* Helps if the teacher knows what they are teaching... */

 /* Check it's a real skill... */

  sn = get_skill_sn( arg2 );

  if (sn < 1) {
    send_to_char("That isn't a real skill!\r\n", ch);
    return;
  } 

  if ( get_skill(ch, sn) < SKILL_MASTER ) {
    send_to_char("You aren't skilled enough to teach that!\r\n", ch); 
    return;  
  }

 /* Ok, teach a specific skill... */ 

  sprintf(buf, "You teach %s '%s'\r\n", pupil->short_descr, arg2);
  send_to_char(buf, ch);

  sprintf(buf, "%s teaches you '%s'.\r\n", capitalize(ch->short_descr), arg2);
  send_to_char(buf, pupil);

  practice_skill(pupil, ch, num_prac, arg2, FALSE);
 
  return;
}


/* Learn a skill from a specific teacher... */

void practice_skill(CHAR_DATA *ch, CHAR_DATA *teacher, int num_prac, char *skill, bool isPractice) {
int stat_prime;  
int learn_stat, gain, i, thr, sn;
char amsg[MAX_STRING_LENGTH]; 
char buf[MAX_STRING_LENGTH];
int cost;
SHOP_DATA *pShop; 
int currency = find_currency(ch);

 /* You must have enough practice sessions... */
 
  if ( ch->practice < num_prac ) {
    send_to_char( "You do not have enough practice sessions left.\r\n", ch );
    return;
  }

 /* Check it's a real skill... */

  sn = get_skill_sn( skill );

  if ( sn < 0 ) {
    send_to_char("That's not a real skill!\r\n", ch);
    return;
  }
 
 /* Check if the character is allowed to learn it... */
 
  if (!isSkilled(ch)) {
    send_to_char("Sorry, no education for unskilled mobs.", ch);
    return;
  }

  if (!skill_array[sn].learnable && !isPractice) {
    send_to_char("No one can teach you about that!\r\n", ch);
    return;
  }

  if (!isSkillAvailable(ch, sn) ) {
    send_to_char( "You can't practice that yet.\r\n", ch );
    return;
  }

  if ( IS_SET(teacher->act, ACT_PRACTICE)
    && isPractice
    && !isSkillAvailableForPractice(ch, sn)) {
    send_to_char("You can't practice that.\r\n", ch);
    return;
  }  

 /* Check the teacher is awake... */

  if (!IS_AWAKE(teacher)) {
    sprintf(buf, "%s is fast asleep - better not wake them.\r\n", capitalize(teacher->short_descr));
    send_to_char(buf, ch);
    return;
  }

 /* Now, what's the threshold... */

  if ( ( isPractice 
      && IS_SET(teacher->act, ACT_PRACTICE) )
    || !isSkilled(teacher)) {
    thr = 60;
  } else {
    thr = get_skill_by(teacher, sn, gsn_teach);
  }   

 /* Weed out the teachers bad subjects... */

  if ( thr < 25 ) {
    sprintf(buf, "%s isn't very good at teaching '%s'.\r\n", capitalize(teacher->short_descr), skill);
    send_to_char(buf, ch);
    return;  
  }

 /* Teach may only be practiced... */

  if ( sn == gsn_teach && !isPractice ) {
    send_to_char("You can only practice teach.\r\n", ch);
    return;
  }

 /* Work out the cost... */

  cost = 0;

  pShop = NULL;

  if (teacher->pIndexData != NULL) {
    pShop = teacher->pIndexData->pShop;

    if (pShop != NULL) {

     /* See if the teacher is willing to teach... */

      if ( time_info.hour < pShop->open_hour 
        || time_info.hour > pShop->close_hour ) { 
        do_say( teacher, "I am busy, come back later." );
        return;
      }


     /* Base cost is 1000 + threshold * 10 - 1250 to 2000+gp */

      cost = 1000 + (10 * thr);

      cost *= num_prac;
 
     /* Apply the markup... */

      cost = (cost * pShop->profit_buy)/100;

    }
  }

 /* PC's always charge a fixed rate... */

  if (!IS_NPC(teacher)) {
     cost = 1000 + (10 * thr);
    cost *= num_prac;
    cost = (cost * (100 + teacher->level)/100);
  }

  if (teacher == ch) cost = 0;

 /* See if the character has enough money... */

  if ( cost > 0 ) {

    if (cost > ch->gold[currency]) { 
      sprintf(buf, "%s wants %d %s to teach you.\r\n" 
                           "You don't have that much.\r\n", capitalize(teacher->short_descr), cost, flag_string(currency_type, currency));
      send_to_char(buf, ch);
      sprintf(buf, "%s doesn't have enough money.\r\n", capitalize(ch->short_descr));
      send_to_char(buf, teacher);
      return;  
    }  

    ch->gold[currency] -= cost;

    sprintf(buf, "You give %s %d %s for their services.\r\n", teacher->short_descr, cost, flag_string(currency_type, currency));
    send_to_char(buf, ch);
    sprintf(buf, "%s gives you %d %s for teaching them.\r\n", capitalize(ch->short_descr), cost, flag_string(currency_type, currency));
    send_to_char(buf, teacher);

    if (!IS_NPC(teacher)) teacher->gold[currency] += cost;
  }

 /* Ammount learned is based on average of INT and your         */
 /* current professions prime stat...                           */
 /* Number fuzzy is used to disguise the exact ammount.         */

  if (ch->profs == NULL) {
    stat_prime = STAT_WIS;
  } else {
    stat_prime = ch->profs->profession->prime_stat;
  }

  learn_stat  = get_curr_stat(ch, stat_prime); 
  learn_stat += get_curr_stat(ch, STAT_INT); 
  learn_stat /= 2;

 /* Calculate the gain... */
       
  gain = 0;

  for(i = 0; i < num_prac; i++) {
    gain += number_fuzzy((int_app[learn_stat].learn)/4);
  }

 /* Adjust gain for learning difficulty... */

  gain = (gain * 100)/skill_array[sn].learn;
 
 /* If your current skill is higher than your teachers, the gain is halved. */

  if (ch->effective->skill[sn] >= thr) {
    send_to_char("Unfortunately, you know as much as your teacher...\r\n", ch);
    gain /= 2;
  }

 /* See if they learn anything... */

  if ( !addSkill(ch, sn, gain) ) {
    send_to_char("You don't learn anything new...\r\n", ch);
    return;
  }
  
 /* Pay for it if they do... */ 

  ch->practice -= num_prac;

 /* Tell the character that they practice... */

  sprintf(buf, "You learn '%s' from %s.\r\n", skill_array[sn].name,  teacher->short_descr); 
  send_to_char(buf, ch);

 /* Tell the character how they are doing... */

  if (ch->effective->skill[sn] == SKILL_UNKNOWN) {
    sprintf(buf,"{YYou don't understand a thing about '%s'!{x\r\n", skill_array[sn].name );
    sprintf(amsg, "$n looks very confused as %s tries to explain '%s'.", teacher->short_descr, skill_array[sn].name);
  } else if (ch->effective->skill[sn] < SKILL_NOVICE) {
    sprintf(buf, "{gYou are a Dabbler in '%s'.{x\r\n", skill_array[sn].name );
    sprintf(amsg, "$n fumbles about learning '%s' from %s.", skill_array[sn].name, teacher->short_descr); 
  } else if (ch->effective->skill[sn] < SKILL_STUDENT) {
    sprintf(buf, "{gYou are a Novice in '%s'.{x\r\n", skill_array[sn].name );
    sprintf(amsg, "$n learns '%s' from %s.", skill_array[sn].name, teacher->short_descr); 
  } else if (ch->effective->skill[sn] < SKILL_ADEPT) {
    sprintf(buf, "{gYou are a Student in '%s'.{x\r\n", skill_array[sn].name );
    sprintf(amsg, "$n learns '%s' from %s.", skill_array[sn].name, teacher->short_descr); 
  } else if (ch->effective->skill[sn] < SKILL_MADEPT) {
    sprintf(buf, "{cYou are an Adept in '%s'.{x\r\n", skill_array[sn].name );
    sprintf(amsg, "$n artfully learns '%s' from %s.", skill_array[sn].name, teacher->short_descr); 
  } else if (ch->effective->skill[sn] < SKILL_MASTER) {
    sprintf(buf, "{cYou are a Master Adept in '%s'.{x\r\n", skill_array[sn].name );
    sprintf(amsg, "$n artfully learns '%s' from %s.", skill_array[sn].name, teacher->short_descr); 
  } else if (ch->effective->skill[sn] < SKILL_GMASTER) {
    sprintf(buf, "{wYou are a Master in '%s'.{x\r\n", skill_array[sn].name );
    sprintf(amsg, "$n respectfully learns '%s' from %s.", skill_array[sn].name, teacher->short_descr); 
  } else if (ch->effective->skill[sn] < SKILL_EXEMPLAR) {
    sprintf(buf, "{wYou are a Grand Master in '%s'.{x\r\n", skill_array[sn].name );
    sprintf(amsg, "$n studies '%s' with %s.", skill_array[sn].name, teacher->short_descr); 
  } else {
    sprintf(buf, "{WYou are an Examplar in '%s'.{x\r\n", skill_array[sn].name );
    sprintf(amsg, "$n considers '%s' with %s.", skill_array[sn].name, teacher->short_descr); 
  } 

  send_to_char(buf,ch);

 /* Tell their audience about it... */

  act( amsg, ch, NULL, NULL, TO_ROOM );

 /* Side effects... */ 

  if (ch->effective->skill[sn] >= SKILL_EXEMPLAR) {

    CHAR_DATA *wch;

    for(wch = ch->in_room->people; wch != NULL; wch = wch->next) {

      if (!isSkilled(wch)) {
        continue;
      }

      if (wch == ch) continue;
      check_improve(wch, sn, TRUE, 2);          

    }
  }

 /* Teachers learn to teach... */

  if (isSkilled(teacher)) {
    check_improve(teacher, gsn_teach, TRUE, 2);

   /* Teachers may also learn as they teach... */

    if (get_skill(ch, sn) > thr) {
      check_improve(teacher, sn, FALSE, 3);
    }
  }

 /* All done... */

 if (IS_NEWBIE(ch)) gain_exp(ch, 50, FALSE);
 return;
}

/* Find the sn of a given skill... */

int get_skill_sn(char *name) {

  int sn;

  for(sn = 0; sn < MAX_SKILL; sn++) {
    if (  skill_array[sn].loaded 
      &&  name[0] == skill_array[sn].name[0] 
      && !str_cmp(name, skill_array[sn].name) ) {
      return sn;
    }
  }

  return SKILL_UNDEFINED;
}


/* Find the gsn for a skill and complain if it can't be found... */

int get_skill_gsn(char *name) {

  int gsn;
  char buf[MAX_STRING_LENGTH];

  gsn = get_skill_sn(name);

  if (gsn == SKILL_UNDEFINED) {
    sprintf(buf, "Skill '%s' undefined, can't set gsn.", name);
    bug(buf, 0);
    exit(1);
  } 

  return gsn;
}

/* Load the skills from disk... */

void load_skills() {
FILE *fp;
char *kwd;
char code;
bool done;
bool ok;
char *name;
int sn;
int num;

  char buf[MAX_STRING_LENGTH];

 /* Initialize incase of problems... */

  for(sn = 0; sn < MAX_SKILL; sn++) {
    skill_array[sn].loaded = FALSE;
  } 

 /* Initialize skill 0 */

  sn = 0;

  skill_array[sn].name = "none";
  skill_array[sn].learn = 100;
  skill_array[sn].stats[0] = STAT_NONE;
  skill_array[sn].stats[1] = STAT_NONE;
  skill_array[sn].group = SKILL_GROUP_NONE;
  skill_array[sn].beats = 12;
  skill_array[sn].minpos = POS_RESTING;
  skill_array[sn].learnable = TRUE;
  skill_array[sn].debate = TRUE;
  skill_array[sn].loaded = TRUE;
  skill_array[sn].amateur = FALSE;
  skill_array[sn].ec = NULL;

 /* Find the banks file... */

  fp = fopen(SKILL_FILE, "r");

  if (fp == NULL) {
    log_string("No skill file!");
    exit(1);
    return;
  }

 /* Read through it and see what we've got... */
 
  done = FALSE;

  while(!done) {

    kwd = fread_word(fp);

    code = kwd[0];

    ok = FALSE;

    switch (code) {

    /* Comments */

      case '#':
        
        ok = TRUE;

        fread_to_eol(fp); 

        break; 

      case 'A':
    
        if (!str_cmp(kwd, "Amateur")) {
          ok = TRUE;
          skill_array[sn].amateur = TRUE;
        }
        break;


     /* Beats */

      case 'B':

       /* Beats b */ 

        if (!str_cmp(kwd, "Beats")) {
 
          ok = TRUE;

         /* Pull in the number... */

          num = fread_number(fp);

          if ( num < 1 
            || num > 240 ) {
            sprintf(buf, "Bad skill beats %d for skill %s.",
                          num, skill_array[sn].name);
            bug(buf, 0);
            exit(1);
          } else {
            skill_array[sn].beats = num;
          }
        }

        break; 

     /* Group */
      
      case 'G':

       /* Group g */ 

        if (!str_cmp(kwd, "Group")) {
 
          ok = TRUE;

         /* Pull in the number... */

          num = fread_number(fp);

          if ( num < SKILL_GROUP_NONE 
            || num > SKILL_GROUP_MAX ) {
            sprintf(buf, "Bad skill group %d for skill %s.",  num, skill_array[sn].name);
            bug(buf, 0);
            exit(1);
          } else {
            skill_array[sn].group = num;
          }
        }

        break; 

     /* Learn... */

      case 'L':

       /* Learn n */ 

        if (!str_cmp(kwd, "Learn")) {
 
          ok = TRUE;

         /* Pull in the number... */

          num = fread_number(fp);

          if ( num < 10 
            || num > 1000 ) {
            sprintf(buf, "Bad learn value %d for skill %s.",  num, skill_array[sn].name);
            bug(buf, 0);
            exit(1);
          } else {
            skill_array[sn].learn = num;
          }
        }

        break;

     /* MinPos */

      case 'M':

       /* MinPos m */ 

        if (!str_cmp(kwd, "MinPos")) {
 
          ok = TRUE;

         /* Pull in the number... */

          num = fread_number(fp);

          if ( num < POS_DEAD 
            || num > POS_STANDING ) {
            sprintf(buf, "Bad skill minmum position %d for skill %s.",
                          num, skill_array[sn].name);
            bug(buf, 0);
            exit(1);
          } else {
            skill_array[sn].minpos = num;
          }
        }

        break; 

     /* PreCond condition */

      case 'P':
 
        if (!str_cmp(kwd, "PreCond")) {

          ECOND_DATA *new_ec;

          ok = TRUE;
          
          new_ec = read_econd(fp);

          if (new_ec == NULL) {
            bug("Bad PreCond in Skills.txt", 0);
            exit(1);
          }

          new_ec->next = skill_array[sn].ec;
          skill_array[sn].ec = new_ec;

        } 

    /* Skill... */

      case 'S':
 
       /* SKILL name~*/

        if (!str_cmp(kwd, "SKILL")) {

          ok = TRUE;

         /* Read the skill name... */

          name = fread_string(fp); 

          if (name[0] == '\0') {
            bug("Unnammed skill in skill file!", 0);
            exit(1);
          }

         /* Get the skill number... */
 
          sn++;

          if (sn >= MAX_SKILL) { 
            bug("To many skills in skill file!",0);
            exit(1);
          }

         /* Initialize data... */ 

          skill_array[sn].name = name;
          skill_array[sn].learn = 100;
          skill_array[sn].stats[0] = STAT_NONE;
          skill_array[sn].stats[1] = STAT_NONE;
          skill_array[sn].group = SKILL_GROUP_NONE;
          skill_array[sn].beats = 12;
          skill_array[sn].minpos = POS_RESTING;
          skill_array[sn].learnable = TRUE;
          skill_array[sn].debate = TRUE;
          skill_array[sn].amateur = FALSE;
          skill_array[sn].loaded = TRUE;
          skill_array[sn].ec = NULL;
        }

       /* Stats sn1 sn2 */

        if (!str_cmp(kwd, "Stats")) {
  
          ok = TRUE;

          num = fread_number(fp);

          if ( num < STAT_NONE 
            || num > STAT_HIGH) {
            sprintf(buf, "Bad stat 0 number %d for skill %s", 
                         num, skill_array[sn].name);
            bug(buf,0);
            exit(1);
          } else {
            skill_array[sn].stats[0] = num;
          } 

          num = fread_number(fp);

          if ( num < STAT_NONE 
            || num > STAT_HIGH) {
            sprintf(buf, "Bad stat 1 number %d for skill %s", 
                         num, skill_array[sn].name);
            bug(buf,0);
            exit(1);
          } else {
            skill_array[sn].stats[1] = num;
          } 

        }

        break;

     /* Unlearnable... */

      case 'U':
    
        if (!str_cmp(kwd, "Unlearnable")) {
          ok = TRUE;
          skill_array[sn].learnable = FALSE;
        }

        if (!str_cmp(kwd, "Undebate")) {
          ok = TRUE;
          skill_array[sn].debate = FALSE;
        }

        break;

     /* File ends with an E */

      case 'E':
        ok = TRUE;
        done = TRUE;
        break; 

     /* Anything else is an error... */

      default:
        break;
    }

    if (!ok) {
      sprintf(buf, "Unrecognized keyword in skills file: %s", kwd);
      bug(buf, 0); 
      exit(1);
    }
 
  }

 /* Close it now we're finished with it... */ 

  fclose(fp);

 /* All done... */

  log_string("...skills loaded");

  return;
}

/* Display skills... */

void do_skills(CHAR_DATA *ch, char *argument) {
char spell_list[SKILL_RANKS+1][MAX_STRING_LENGTH * 2];
char spell_columns[SKILL_RANKS+1];
int sn,lev,skill, last_lev;
bool found = FALSE;
char buf[MAX_STRING_LENGTH];
char outbuf[(MAX_SKILL+10) * 80];

    CHAR_DATA *victim;

    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int group;

   /* Initialize output buffer... */
 
    outbuf[0] = '\0';

   /* Args are group and mob */

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

   /* What group do we want to look at? */

      sn = get_skill_sn(arg1);

      if (sn > 0) {
        do_skill_info(ch, arg1);
        return;
      }

    if (!str_cmp(arg1, "all")) {
      group = SKILL_GROUP_ALL;
      strcat(outbuf, "{WAll skills{x\r\n"); 
    } else if (!str_cmp(arg1, "magic")) {
      group = SKILL_GROUP_MAGIC;
      strcat(outbuf, "{WMagic skills{x\r\n"); 
    } else if (!str_cmp(arg1, "combat")) {
      group = SKILL_GROUP_COMBAT;
      strcat(outbuf, "{WCombat skills{x\r\n"); 
    } else if (!str_cmp(arg1, "general")) {
      group = SKILL_GROUP_GENERAL;
      strcat(outbuf, "{WGeneral skills{x\r\n"); 
    } else if (!str_cmp(arg1, "mlang")) {
      group = SKILL_GROUP_MLANG;
      strcat(outbuf, "{WModern Language skills{x\r\n"); 
    } else if (!str_cmp(arg1, "alang")) {
      group = SKILL_GROUP_ALANG;
      strcat(outbuf, "{WAncient Language skills{x\r\n"); 
    } else if (!str_cmp(arg1, "clang")) {
      group = SKILL_GROUP_CLANG;
      strcat(outbuf, "{WCthonic Language skills{x\r\n"); 
    } else if (!str_cmp(arg1, "academic")) {
      group = SKILL_GROUP_ACADEMIC;
      strcat(outbuf, "{WAcademic skills{x\r\n"); 
    } else if (!str_cmp(arg1, "production")) {
      group = SKILL_GROUP_PRODUCTION;
      strcat(outbuf, "{WProduction skills{x\r\n"); 
    } else if (!str_cmp(arg1, "lang")) {
      group = SKILL_GROUP_LANG;
      strcat(outbuf, "{WAll Language skills{x\r\n"); 
    } else if (!str_cmp(arg1, "nolang")) {
      group = SKILL_GROUP_NOLANG;
      strcat(outbuf, "{WAll Magic, Combat and General skills{x\r\n"); 
    } else if (!str_cmp(arg1, "cng")) {
      group = SKILL_GROUP_COMBAT | SKILL_GROUP_GENERAL;
      strcat(outbuf, "{WCombat and General skills{x\r\n"); 
    } else if (arg1[0] == '\0') {
      group = SKILL_GROUP_COMBAT | SKILL_GROUP_GENERAL;
      strcat(outbuf, "{WCombat and General skills{x\r\n"); 
    } else {
      send_to_char("Syntax: skills <group>\r\n", ch);
      send_to_char("Group: combat magic general mlang alang clang academic\r\n"
                                "       lang nolang cng production\r\n", ch);
      return;
    } 

   /* Immortals may also see others skills... */

    if (arg2[0] != '\0') {
        if (!IS_IMMORTAL(ch)) {
            send_to_char("Syntax: skills <group>\r\n", ch);
            return;
        }

        victim = get_char_room(ch, arg2);

        if (victim == NULL) {
            send_to_char("They are not here.\n\e", ch);
            return;
        }
    } else {
        victim = ch;
    }

    if (IS_AFFECTED(ch, AFF_BERSERK)) {
      send_to_char("{rYou are to {Rangry{x to worry about skills..."
                                "you just want to {RKILL SOMEONE!{x\r\n", ch);
      return;
    }

    /* initilize data */

    for (lev = 0; lev < SKILL_RANKS + 1; lev++) {
	spell_columns[lev] = 0;
	spell_list[lev][0] = '\0';
    }
 
    for (sn = 0; sn < MAX_SKILL; sn++) {
        if (!skill_array[sn].loaded) break;
  
        if (IS_SET(skill_array[sn].group, group)  
        && isSkillAvailable(victim, sn) ) {
              skill = get_skill(victim, sn);
              if (skill >= SKILL_EXEMPLAR) {
                   lev = 0;
              } else if (skill >= SKILL_GMASTER) {
                   lev = 1;
              } else if (skill >= SKILL_MASTER) { 
                   lev = 2;
              } else if (skill >= SKILL_MADEPT) {
                   lev = 3;  
              } else if (skill >= SKILL_ADEPT) {
                   lev = 4;
              } else if (skill >= SKILL_STUDENT) {
                   lev = 5;
              } else if (skill >= SKILL_NOVICE) {
                   lev = 6;
              } else if (skill >= SKILL_DABBLER) {
                   lev = 7;
              } else {
                   continue;
              }    
              found = TRUE;

              sprintf(buf," %-19s %3d  ", skill_array[sn].name, skill);
              strcat(spell_list[lev],buf);
              if ( (++spell_columns[lev]) % 3 == 0) strcat(spell_list[lev],"\r\n");
        } 
    }

    /* return results */
 
    if (!found) {
        if (victim == ch) send_to_char("You know no skills of that sort.\r\n",ch);
        else send_to_char("They know no skills of that sort.\r\n", ch);
        return;
    }
 
    last_lev = -1;

   /* Output time... */
  
    for (lev = 0; lev < SKILL_RANKS; lev++) {
      if (spell_list[lev][0] != '\0') {
        if (last_lev > -1) {
             if ( (spell_columns[last_lev] % 3) != 0 ) strcat(outbuf, "\r\n");
        }
 
        last_lev = lev;

        switch (lev) {
          case 0:
            strcat(outbuf, "{y[Exemplar]{W\r\n");
            break;
          case 1:
            strcat(outbuf, "{y[Grand Master]{w\r\n");
            break;
          case 2:
            strcat(outbuf, "{y[Master]{w\r\n");
            break;
          case 3:
            strcat(outbuf, "{y[Master Adept]{c\r\n");
            break;
          case 4:
            strcat(outbuf, "{y[Adept]{c\r\n");
            break;
          case 5:
            strcat(outbuf, "{y[Student]{g\r\n");
            break;
          case 6:
            strcat(outbuf, "{y[Novice]{g\r\n");
            break;
          case 7:
            strcat(outbuf, "{y[Dabbler]{g\r\n");
            break;
          case 8:
            strcat(outbuf, "{y[Unknown]{x\r\n");
            break;
          default:
            strcat(outbuf, "{y[Unknown]{r\r\n");
            break; 
        }

        strcat(outbuf, spell_list[lev]);

      }
    }

    strcat(outbuf,"{x\r\n");
    page_to_char(outbuf,ch);
    return;
}


void do_skill_info(CHAR_DATA *ch, char *args) {
char buf[MAX_STRING_LENGTH];
char buf2[MAX_STRING_LENGTH];
MOB_CMD_CONTEXT *mcc;
int spn, skl;

    spn = get_skill_sn(args);

    if ( spn <= 0 ) {
        send_to_char("No such skill.\r\n", ch);
        return;
    }

    skl = get_skill (ch, spn);
    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL);

    sprintf(buf, "{cSkill: {g%s{c\r\n"
                         "--------------------------{x\r\n", skill_array[spn].name);

    sprintf(buf2, "{cRating    : {g%d %%{x\r\n", skl ); 
    strcat(buf, buf2); 
    
    if (skill_array[spn].learn != 100) {
         sprintf(buf2, "{cLearning  : {g%d %%{x\r\n", skill_array[spn].learn);
         strcat(buf, buf2);
    }

    print_econd_to_buffer(buf,  skill_array[spn].ec, "{WConditions -{x ", mcc);
    send_to_char(buf, ch);
    free_mcc(mcc);
    return;
}


void do_study(CHAR_DATA *ch, char *argument) {
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];

  char buf[MAX_STRING_LENGTH];

  OBJ_DATA *obj;

  int number, count;

  bool found;

  int rating, sn;

 /* Must be skilled... */

  if (!isSkilled(ch)) {
    send_to_char("You can't understand it.\r\n", ch);
    return;
  }

 /* Corpses don't show things... */

  if ( ch->position < POS_SLEEPING ) {
    send_to_char( "You can't see anything but stars!\r\n", ch );
    return;
  }

 /* Not in your sleep... */ 

  if ( ch->position == POS_SLEEPING ) {
    send_to_char( "You can't do that, you're sleeping!\r\n", ch );
    return;
  }

 /* Nor when you are fighting... */

  if (ch->position == POS_FIGHTING) {
    send_to_char("You're to busy for that now...", ch);
    return; 
  }

 /* You have to be able to see it to show it... */ 

  if (!check_blind( ch )) {
    send_to_char("You can't see it. You're blind!", ch);
    return;
  }

 /* Need light to find it... */

  if ( !IS_NPC(ch)
    && !IS_SET(ch->plr, PLR_HOLYLIGHT)
    && room_is_dark( ch->in_room ) ) {
    send_to_char( "It is pitch black ... \r\n", ch );
    return;
  }

 /* Now, what is it? */
  
  if (argument[0] == '\0') {
    send_to_char("Syntax: study book\r\n", ch);
    return;
  }

 /* Get object name (1st word) - use quotes */
 
  argument = one_argument( argument, arg1 );

 /* Split off the leading number (no support for all.)... */ 

  number = number_argument(arg1,arg2);

 /* Check for stupidity... */

  if (arg2[0] == '\0') {
    send_to_char("That's just a number!", ch);
    return;
  } 

 /* Locate the object... */ 

  count = 0; 
  found = FALSE;

 /* First check inventory... */

  for ( obj = ch->carrying; obj != NULL; obj = obj->next_content ) {

   /* Can the show-er see it? */

    if ( !can_see_obj( ch, obj ) ) {
      continue;
    }

   /* Locate the object by name alone... */

    if (!is_name( arg2, obj->name)) {
      continue;
    }

   /* Is it the right one? */

    if (++count != number) {
      continue;
    } 

    found = TRUE;
    break;
  }
 
 /* Then check the room... */

  if (!found) {

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content ) {

     /* Can the show-er see it? */

      if ( !can_see_obj( ch, obj ) ) {
        continue;
      }

     /* Locate the object by name alone... */

      if (!is_name( arg2, obj->name)) {
        continue;
      }

     /* Is it the right one? */

      if (++count != number) {
        continue;
      } 

      found = TRUE;
      break;
    }
  }

 /* Bail out if not found... */

  if (!found) {
    send_to_char("You can't find that!", ch);
    return;
  }  

  if (obj == NULL) {
    send_to_char("You thought you had found it, but you hadn't.\r\n", ch);
    return;
  }

 /* Bail out if it's not a book... */

  if (obj->item_type != ITEM_BOOK) {
    send_to_char("You broaden your mind...\r\n"
                 "...so far it starts dripping out of your ears.\r\n", ch);
    return;
  }

 /* Ok, now we skip the language check and the skill downgrade... */

  rating = obj->value[1];

  rating *= obj->condition;

  rating *= get_skill(ch, obj->value[0]);

  rating /= 10000;

  if (rating < 10) {
    send_to_char("You can't understand it!\r\n", ch);
    return;
  }

 /* Quick level check... */

  if (ch->level + (get_curr_stat(ch, STAT_INT)/4) < obj->level) {
    send_to_char("This text is to complex for you...\r\n", ch);
    return;
  }

 /* Ok, do they have a practice session spare... */

  if (ch->practice < 1) {
    send_to_char("Studying a book costs 1 practice.\r\n", ch);
    return;
  }

  ch->practice -= 1;

 /* Now apply the skill gains... */  

  send_to_char("You are deeply engrossed in study...\r\n", ch);

  act_new("$n studies $p.", ch, obj, NULL, TO_ROOM, POS_DEAD);

  for(number = 2; number < 5; number++) { 
    
    sn = obj->value[number];

    if (sn < 1) {
      continue;
    }

    if (ch->effective->skill[sn] < rating) {

      addSkill(ch, sn, dice(2,6) );

      sprintf(buf, "You learn '%s'!\r\n",
                   skill_array[sn].name);
      send_to_char(buf, ch);
      
    } else {
      check_improve(ch, sn, TRUE, 1);
    }
 
  }

 /* Books wear out... */

  damage_obj(ch, obj, dice(2,6));

 /* All done... */

  return;
}


void yith_adapt( CHAR_DATA *ch, char *argument ) {
  char buf[MAX_STRING_LENGTH];
  int sn;
  char arg1[MAX_INPUT_LENGTH];
  int num_prac;
  CHAR_DATA *teacher;
  CHAR_DATA *pupil;
 
  argument = one_argument(argument, arg1); 

 /* Must have a teacher... */

  teacher = ch;
  pupil = ch->desc->original;

  if (argument[0] != '\0') num_prac = atoi(argument);
  else num_prac = 1;
  
  if (num_prac == 0) {
    send_to_char("Syntax: adapt <skill> [count]\r\n", ch);
    return;
  } 

 /* Berserkers don't pay attention... */ 

  if (IS_AFFECTED(ch, AFF_BERSERK)) {
    send_to_char("{rYou are to {Rangry{x to worry about learning..."
                              "you just want to {RKILL SOMEONE!{x\r\n", ch);
    return;
  }

  if ( arg1[0] == '\0' ) {
    int col;
    int skill;
    char outbuf[MAX_SKILL * 50];

    sprintf(outbuf, "%s can teach you:\r\n", capitalize(teacher->short_descr));
    
    col    = 0;
    for ( sn = 0; sn < MAX_SKILL; sn++ ) {
	
      if ( !skill_array[sn].loaded ) break;
      if (!isSkillAvailable(teacher, sn)) continue;
      skill = get_skill_by(teacher, sn, gsn_teach);

      if (skill < 25) {
        continue;
      } else if (skill < SKILL_ADEPT) {
        strcat(outbuf, "{g");
      } else if (skill < SKILL_MASTER) {
        strcat(outbuf, "{c");
      } else if (skill < SKILL_EXEMPLAR) {
        strcat(outbuf, "{w");
      } else {
        strcat(outbuf, "{Y");
      }

      sprintf( buf, "%-18s %3d   ", skill_array[sn].name, skill);
      strcat(outbuf, buf);
      if ( ++col % 3 == 0 ) strcat(outbuf, "{x\r\n" );
    }

    if ( col % 3 != 0 ) strcat(outbuf, "{x\r\n");
    sprintf( buf, "{wYou have {W%d{w practice sessions left.{x\r\n", ch->practice );
    strcat( outbuf, buf);
    page_to_char( outbuf, ch );
    return;
  } 

  adapt_skill(pupil, teacher, num_prac, arg1);
 
  return;
}


void adapt_skill(CHAR_DATA *ch, CHAR_DATA *teacher, int num_prac, char *skill) {
  int stat_prime;  
  int learn_stat, gain, i, thr, sn;
  char buf[MAX_STRING_LENGTH];

  if ( ch->practice < num_prac ) {
    send_to_char( "You do not have enough practice sessions left.\r\n", teacher);
    return;
  }

  sn = get_skill_sn(skill );

  if ( sn < 0 ) {
    send_to_char("That's not a real skill!\r\n", teacher);
    return;
  }
 
 /* Check if the character is allowed to learn it... */
 
  if (!isSkilled(ch)) {
    send_to_char("Sorry, no education for unskilled mobs.", teacher);
    return;
  }

  if (!isSkillAvailable(ch, sn) ) {
    send_to_char( "You can't adapt that yet.\r\n", teacher);
    return;
  }

   thr = get_skill(teacher, sn);

  if (isSkilled(teacher)) {
     if ( thr < 15 ) {
        sprintf(buf, "%s isn't very skilled at '%s'.\r\n", capitalize(teacher->short_descr), skill);
        send_to_char(buf, teacher);
        return;  
     }
  } else {
     if ( thr < 35 ) {
        sprintf(buf, "%s isn't very skilled at '%s'.\r\n", capitalize(teacher->short_descr), skill);
        send_to_char(buf, teacher);
        return;  
     }
  }

 /* Ammount learned is based on average of INT and your         */
 /* current professions prime stat...                           */
 /* Number fuzzy is used to disguise the exact ammount.         */

  if (ch->profs == NULL) stat_prime = STAT_WIS;
  else stat_prime = ch->profs->profession->prime_stat;
  
  learn_stat  = get_curr_stat(ch, stat_prime); 
  learn_stat += get_curr_stat(ch, STAT_INT); 
  learn_stat /= 2;

 /* Calculate the gain... */
       
  gain = 0;

  for(i = 0; i < num_prac; i++) {
    gain += number_fuzzy((int_app[learn_stat].learn)/5);
  }

 /* Adjust gain for learning difficulty... */

  gain = (gain* 50) /  (get_skill(ch, sn) + 1) * get_skill(teacher,sn) / 4;
  gain = gain / (skill_array[sn].learn +1) + 1;

  if (gain > 8) gain = (gain - 8)/2 +8;
  if (gain > 12) gain =(gain -12)/2 + 12;
  if (gain > 16) gain =(gain -16)/2 + 16;

 /* If your current skill is higher than your teachers, the gain is halved. */

  if (ch->effective->skill[sn] >= thr) {
    send_to_char("Unfortunately, you can't adapt anything new...\r\n", ch);
    gain /= 5;
  }

 /* See if they learn anything... */

  if ( !addSkill(ch, sn, gain) ) {
    send_to_char("You don't adapt anything new...\r\n", teacher);
    return;
  }
  
 /* Pay for it if they do... */ 

  ch->practice -= num_prac;

 /* Tell the character that they practice... */

  sprintf(buf, "You adapt '%s' from %s.\r\n", skill_array[sn].name, teacher->short_descr); 
  send_to_char(buf, teacher);

 /* Tell the character how they are doing... */

  if (ch->effective->skill[sn] == SKILL_UNKNOWN) {
    sprintf(buf,"{YYou don't understand a thing about '%s'!{x\r\n",  skill_array[sn].name );
  } else if (ch->effective->skill[sn] < SKILL_NOVICE) {
    sprintf(buf, "{gYou are a Dabbler in '%s'.{x\r\n", skill_array[sn].name );
  } else if (ch->effective->skill[sn] < SKILL_STUDENT) {
    sprintf(buf, "{gYou are a Novice in '%s'.{x\r\n", skill_array[sn].name );
  } else if (ch->effective->skill[sn] < SKILL_ADEPT) {
    sprintf(buf, "{gYou are a Student in '%s'.{x\r\n", skill_array[sn].name );
  } else if (ch->effective->skill[sn] < SKILL_MADEPT) {
    sprintf(buf, "{cYou are an Adept in '%s'.{x\r\n", skill_array[sn].name );
  } else if (ch->effective->skill[sn] < SKILL_MASTER) {
    sprintf(buf, "{cYou are a Master Adept in '%s'.{x\r\n", skill_array[sn].name );
  } else if (ch->effective->skill[sn] < SKILL_GMASTER) {
    sprintf(buf, "{wYou are a Master in '%s'.{x\r\n", skill_array[sn].name );
  } else if (ch->effective->skill[sn] < SKILL_EXEMPLAR) {
    sprintf(buf, "{wYou are a Grand Master in '%s'.{x\r\n", skill_array[sn].name );
  } else {
    sprintf(buf, "{WYou are an Examplar in '%s'.{x\r\n", skill_array[sn].name );
  } 
  send_to_char(buf,teacher);
  act("$n thinks deeply.", teacher, NULL, NULL, TO_ROOM );

  return;
}


void do_research(CHAR_DATA *ch, char *argument) {
    ECOND_DATA *ec = NULL;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char outbuf[3*MAX_STRING_LENGTH];
    int sn, snt;

    argument = one_argument(argument, arg);

    sn = get_skill_sn(arg);
    if (sn <= 0) {
       send_to_char("Syntax: RESEARCH '<skill>'\r\n", ch);
       return;
    }
    do_skill_info(ch, arg);

    sprintf(outbuf, "\r\n{cNeeded for Skills:{x\r\n");
    for(snt = 0; snt < MAX_SKILL; snt++) {
         ec = skill_array[snt].ec;   

         while (ec != NULL) {
              if (ec->type == ECOND_SKILL
              && sn == ec->index
              && sn != snt) {
                    sprintf(buf, "   %s\r\n", skill_array[snt].name);
                    strcat(outbuf, buf);
              }
              ec = ec->next;
         }                           
    }

    strcat(outbuf, "\r\n{cNeeded for Spells:{x\r\n");
    for(snt = 0; snt < MAX_SPELL; snt++) {
       if (spell_array[snt].sn == sn
       || spell_array[snt].sn2 == sn) {
               sprintf(buf, "   %s\r\n", spell_array[snt].name);
               strcat(outbuf, buf);
       }
    } 
 
    page_to_char(outbuf, ch);
    research_prof(ch, sn);    
    return;
}


char *get_skill_group_name(int sn) {
    if (sn == 0) return "none";

    if (IS_SET(skill_array[sn].group, SKILL_GROUP_MAGIC)) return "magic";
    if (IS_SET(skill_array[sn].group, SKILL_GROUP_COMBAT)) return "combat";
    if (IS_SET(skill_array[sn].group, SKILL_GROUP_GENERAL)) return "general";
    if (IS_SET(skill_array[sn].group, SKILL_GROUP_PRODUCTION)) return "production";
    if (IS_SET(skill_array[sn].group, SKILL_GROUP_ACADEMIC)) return "academic";
    if (IS_SET(skill_array[sn].group, SKILL_GROUP_MLANG)
    || IS_SET(skill_array[sn].group, SKILL_GROUP_ALANG)
    || IS_SET(skill_array[sn].group, SKILL_GROUP_CLANG)) return "language";

    return "none";
}


void prompt_amateur(DESCRIPTOR_DATA *d) {
CHAR_DATA *ch = d->character;
char buf[MAX_STRING_LENGTH];
char outbuf[MAX_SKILL * 50];
int col, skill, sn;

outbuf[0] = '\0';

  if (IS_SET(ch->plr, PLR_IMP_HTML)) write_to_buffer( d, IMP_CLS, 0 );
  else write_to_buffer(d,VT_CLS, 0);

  sprintf(outbuf, "******************************************************************************\r\n"); 
  strcat(outbuf, "\r\n"); 
  strcat(outbuf, "   Select your amateur skills:\r\n"); 
  strcat(outbuf, "\r\n"); 

    col    = 0;
    for ( sn = 0; sn < MAX_SKILL; sn++ ) {
      if (!skill_array[sn].loaded )  break;
      if (!skill_array[sn].amateur) continue;

      skill = get_skill(ch, sn);

      sprintf( buf, "%-18s %3d   ", skill_array[sn].name, skill);
      strcat(outbuf, buf);

      if ( ++col % 3 == 0 ) strcat(outbuf, "\r\n" );
    }

    if ( col % 3 != 0 ) strcat(outbuf, "\r\n");
     
    sprintf(buf, "\r\nYou have %d practice sessions left.\r\n", ch->practice);
    strcat( outbuf, buf);
    strcat(outbuf, "\r\n"); 
    strcat(outbuf, "******************************************************************************\r\n"); 
    strcat(outbuf, "\r\n"); 
    strcat(outbuf, "Enter <skill name>, ?, ? <skill>, CHAT or DONE\r\n");
    write_to_buffer(d, outbuf, 0);
    return;
}


void practice_amateur(CHAR_DATA *ch, int sn) {
int stat_prime;  
int learn_stat, gain, thr;
char buf[MAX_STRING_LENGTH];

  thr = 25;
  stat_prime = STAT_WIS;

  learn_stat  = get_curr_stat(ch, stat_prime); 
  learn_stat += get_curr_stat(ch, STAT_INT); 
  learn_stat /= 2;

  gain = number_fuzzy((int_app[learn_stat].learn)/4);

  gain = (gain * 100)/skill_array[sn].learn;
 
  if (ch->effective->skill[sn] >= thr) {
    send_to_char("Unfortunately, you know a lot of that amateur skill.\r\n", ch);
    gain /= 2;
  }

  if ( !addSkill(ch, sn, gain) ) {
    send_to_char("You don't learn anything new...\r\n", ch);
    return;
  }
  
  ch->practice--;

  sprintf(buf, "You learn '%s'.\r\n", skill_array[sn].name); 
  send_to_char(buf, ch);
  prompt_amateur(ch->desc);
  return;
}

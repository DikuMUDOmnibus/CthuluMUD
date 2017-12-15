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
#include "conv.h"
#include "deeds.h"
#include "quests.h"
#include "gadgets.h"
#include "affect.h"
#include "society.h"
#include "race.h"
#include "profile.h"
#include "partner.h"

int 		arrange_div		(int divinity);
extern int 	casting_level		(CHAR_DATA *ch);

/* Static anchor... */

 static ECOND_DATA *free_econd = NULL;

/* Get an econd... */

 ECOND_DATA *get_ec() {

   ECOND_DATA *new_ec;

  /* Acquire an ec... */

   if (free_econd != NULL) {
     new_ec = free_econd;
     free_econd = free_econd->next;
   } else {
     new_ec = (ECOND_DATA *) alloc_perm( sizeof(*new_ec));
   }

  /* ...initialize it... */

   new_ec->type		= ECOND_NONE;
   new_ec->subject	= ECOND_SUBJECT_WORLD;
   new_ec->index	= 0;
   new_ec->low		= 0;
   new_ec->high		= 0;
   new_ec->text		= NULL;
   new_ec->next		= NULL;
 
  /* ...and give it to our caller... */

   return new_ec;
 }

/* Free an econd... */

 void free_ec(ECOND_DATA *ec) {

   if ( ec->text != NULL ) {
     free_string(ec->text);
     ec->text = NULL;
   }

   if ( ec->next != NULL ) {
     free_ec(ec->next);
     ec->next = NULL;
   }

   ec->next = free_econd;
   free_econd = ec;

   return;
 }

 void free_ec_norecurse(ECOND_DATA *ec) {

   if ( ec->text != NULL ) {
     free_string(ec->text);
     ec->text = NULL;
   }

   ec->next = free_econd;
   free_econd = ec;

   return;
 }

/* Evaluate a world condition... */

 bool ec_satisfied_world(ECOND_DATA *ec) {
int val;
int type;
ROOM_INDEX_DATA *room;
GADGET_DATA *gadget;
CHAR_DATA *mob;
OBJ_DATA *obj;
OBJ_DATA *container;

   bool found;
  
  /* Adjust for NOT... */

   type = ec->type;

   if (type < 0) {
     type *= -1;
   }

  /* Check by condition type... */

   switch (type) {

    /* moon high low */

      case ECOND_MOON: 

        if (weather_info.moon < ec->low) {
          return FALSE;
        }

        if (weather_info.moon > ec->high) {
          return FALSE;
        }

        break;

    /* random low high */

      case ECOND_RANDOM: 

        val = number_range(0, 1023);

        if (val < ec->low) {
          return FALSE;
        }

        if (val > ec->high) {
          return FALSE;
        }

        break;

     /* hour_of_day low high */

      case ECOND_HOUR_OF_DAY:

        if (time_info.hour < ec->low) {
          return FALSE;
        }

        if (time_info.hour > ec->high) {
          return FALSE;
        }

        break;

     /* hour_of_day_mod mod low high */

      case ECOND_HOUR_OF_DAY_MOD:

        if (ec->index == 0) {
          return FALSE;
        }

        val = time_info.hour % ec->index;

        if (val < ec->low) {
          return FALSE;
        }

        if (val > ec->high) {
          return FALSE;
        }

        break;

     /* day_of_month low high */

      case ECOND_DAY_OF_MONTH:
 
        val = time_info.day + 1;

        if (val < ec->low) {
          return FALSE;
        }

        if (val > ec->high) {
          return FALSE;
        }

        break;

     /* day_of_month_mod mod low high */

      case ECOND_DAY_OF_MONTH_MOD:

        if (ec->index == 0) {
          return FALSE;
        }

        val = (time_info.day + 1) % ec->index;

        if (val < ec->low) {
          return FALSE;
        }

        if (val > ec->high) {
          return FALSE;
        }

        break;

     /* day_of_year low high */

      case ECOND_DAY_OF_YEAR:

        val = ((DAYS_IN_MONTH * time_info.month) + time_info.day + 1 );

        if (val < ec->low) {
          return FALSE;
        }

        if (val > ec->high) {
          return FALSE;
        }

        break;

     /* day_of_year_mod mod low high */

      case ECOND_DAY_OF_YEAR_MOD:

        if (ec->index == 0) {
          return FALSE;
        }

        val = (DAYS_IN_MONTH * time_info.month ) + time_info.day + 1; 

        val = val % ec->index;

        if (val < ec->low) {
          return FALSE;
        }

        if (val > ec->high) {
          return FALSE;
        }

        break;

     /* week_of_year low high */

      case ECOND_WEEK_OF_YEAR:

        val = time_info.week + 1;

        if (val < ec->low) {
          return FALSE;
        }

        if (val > ec->high) {
          return FALSE;
        }

        break;

     /* week_of_year_mod mod low high */

      case ECOND_WEEK_OF_YEAR_MOD:

        if (ec->index == 0) {
          return FALSE;
        }

        val = ( time_info.week + 1 ) % ec->index;

        if (val < ec->low) {
          return FALSE;
        }

        if (val > ec->high) {
          return FALSE;
        }

        break;

     /* month_of_year low high */

      case ECOND_MONTH_OF_YEAR:

        val = time_info.month + 1;

        if (val < ec->low) {
          return FALSE;
        }

        if (val > ec->high) {
          return FALSE;
        }

        break;

     /* month_of_year_mod mod low high */

      case ECOND_MONTH_OF_YEAR_MOD:

        if (ec->index == 0) {
          return FALSE;
        }

        val = ( time_info.month + 1 ) % ec->index;

        if (val < ec->low) {
          return FALSE;
        }

        if (val > ec->high) {
          return FALSE;
        }

        break;

     /* Gadgets... */

      case ECOND_GADGET:

        if (ec->index <= 0) {
          return FALSE;
        }

        room = get_room_index(ec->index);

        if (room == NULL) {
          return FALSE;
        }

        gadget = find_gadget_by_id(room, ec->low);

        if (gadget == NULL) {
          return FALSE;
        } 

        if (gadget->state != ec->high) {
          return FALSE;
        } 

        break;

     /* Mob_in_room... */

      case ECOND_MOB_IN_ROOM:

        if (ec->index <= 0) {
          return FALSE;
        }

        room = get_room_index(ec->index);

        if (room == NULL) {
          return FALSE;
        }

        found = FALSE;

        mob = room->people;

        while ( mob != NULL 
             && !found ) {
          
          if ( mob->pIndexData != NULL ) {
            if (mob->pIndexData->vnum >= ec->low) {
              if (ec->high == -1) {
                found = TRUE;
              } else { 
                if (mob->pIndexData->vnum <= ec->high) {
                  found = TRUE;
                }
              }
            } 
          }

          mob = mob->next_in_room; 
        }

        if (!found) {
          return FALSE;
        }

        break;

     /* Player_in_room... */

      case ECOND_PLAYER_IN_ROOM:

        if (ec->index <= 0) return FALSE;
        room = get_room_index(ec->index);

        if (room == NULL) return FALSE;
        found = FALSE;

        mob = room->people;

        while (mob != NULL) {
          if ( mob->pIndexData == NULL ) found = TRUE;
          mob = mob->next_in_room; 
        }

        if (!found) return FALSE;

        break;


     /* Room_Empty_Mob... */

      case ECOND_ROOM_EMPTY_MOB:

        if (ec->index <= 0) return FALSE;
        room = get_room_index(ec->index);
        if (room == NULL) return FALSE;
        found = FALSE;
        if (room->people != NULL) return FALSE;
        break;


     /* Obj_in_room... */

      case ECOND_OBJ_IN_ROOM:
        if (ec->index <= 0) return FALSE;
        room = get_room_index(ec->index);
        if (room == NULL) return FALSE;
        found = FALSE;
        obj = room->contents;

        while ( obj != NULL && !found ) {
          
          if ( obj->pIndexData != NULL ) {
            if (obj->pIndexData->vnum >= ec->low) {
              if (ec->high == -1) {
                found = TRUE;
              } else { 
                if (obj->pIndexData->vnum <= ec->high) {
                  found = TRUE;
                }
              }
            } 
          }

          obj = obj->next_content; 
        }

        if (!found) return FALSE;
        break;


      case ECOND_OBJ_ROOM_CONTAINER:
        if (ec->index <= 0) return FALSE;
        room = get_room_index(ec->index);
        if (room == NULL)  return FALSE;
        found = FALSE;

        container = room->contents;
        while (container != NULL && !found ) {
             if (container->pIndexData->vnum == ec->low) {
                   obj = container->contains;
                   while (obj != NULL && !found ) {
                         if (obj->pIndexData->vnum == ec->high) found = TRUE;
                         obj = obj->next_content; 
                   }
             } 
             container = container->next_content; 
         }

        if (!found) return FALSE;
        break;


     /* Room_Empty_Obj... */

      case ECOND_ROOM_EMPTY_OBJ:
        if (ec->index <= 0) return FALSE;
        room = get_room_index(ec->index);
        if (room == NULL) return FALSE;
        if (room->contents != NULL) return FALSE;
        break;

      case ECOND_PARTNER:
        if ( ec->text == NULL ) return FALSE;

        if ((mob = get_char_world_player(NULL, ec->text)) == NULL) return FALSE;
        if (am_i_partner(mob) < 0) return FALSE;
        break;


     /* Unrecognized condition is False... */

      default:

        bug("Unrecognized WORLD condition %d!", ec->type);

        return FALSE;
    }
      
    return TRUE;
  }

/* Evaluate an IS condition against a mob... */

 bool ec_satisfied_mob_is(CHAR_DATA *ch, int index) {

  /* No mob means it's false... */

   if (ch == NULL) {
     return FALSE;
   }

  /* Ok, see what we have to check... */

   switch (index) {

     case ECOND_IS_STARVING:
       if (IS_NPC(ch)) return FALSE;
       if (ch->condition[COND_FOOD] < -200) return TRUE;
       break;

     case ECOND_IS_HUNGRY:
       if (IS_NPC(ch)) return FALSE;
       if (ch->condition[COND_FOOD] <= 600) return TRUE;
       break;

     case ECOND_IS_FULL:
       if (IS_NPC(ch)) return FALSE;
       if (ch->condition[COND_FOOD] > 1200) return TRUE;
       break;

     case ECOND_IS_OVERWEIGHT:
       if (IS_NPC(ch)) return FALSE;
       if (ch->condition[COND_FAT] > 10) return TRUE;
       break;

     case ECOND_IS_DEHYDRATED:
       if (IS_NPC(ch)) return FALSE;
       if (ch->condition[COND_DRINK] <= -100) return TRUE;
       break;

     case ECOND_IS_THIRSTY:
       if (IS_NPC(ch)) return FALSE;
       if (ch->condition[COND_DRINK] <= 300) return TRUE;
       break;

     case ECOND_IS_REFRESHED:
       if (IS_NPC(ch)) return FALSE;
       if (ch->condition[COND_DRINK] > 600) return TRUE;
       break;

     case ECOND_IS_SOBER:
       if (IS_NPC(ch)) return FALSE;
       if (ch->condition[COND_DRUNK] == 0) return TRUE;
       break;

     case ECOND_IS_DRUNK:
       if (IS_NPC(ch)) return FALSE;
       if (ch->condition[COND_DRUNK] > 10) return TRUE;
       break;

     case ECOND_IS_STANDING:
       if (ch->position == POS_STANDING) return TRUE;
       break;

     case ECOND_IS_PLAYER:
       if (!IS_NPC(ch)) return TRUE;
       break;

     case ECOND_IS_FIGHTING:
       if (ch->position == POS_FIGHTING) return TRUE;
       break;

     case ECOND_IS_SITTING:
       if (ch->position == POS_SITTING) return TRUE;
       break;

     case ECOND_IS_RESTING:
       if (ch->position == POS_RESTING) return TRUE;
       break;

     case ECOND_IS_AWAKE:
       if (ch->position > POS_SLEEPING) return TRUE;
       break;

     case ECOND_IS_SLEEPING:
       if (ch->position == POS_SLEEPING) return TRUE;
       break;

     case ECOND_IS_STUNNED:
       if (ch->position == POS_STUNNED) return TRUE;
       break;

     case ECOND_IS_DIEING:
       if ( ch->position == POS_INCAP  || ch->position == POS_MORTAL ) return TRUE;
       break;

     case ECOND_IS_DEAD:
       if (ch->position == POS_DEAD) return TRUE;
       break;

     case ECOND_IS_BLIND:
       if (IS_AFFECTED(ch, AFF_BLIND)) return TRUE;
       break;

     case ECOND_IS_INVISIBLE:
       if (IS_AFFECTED(ch, AFF_INVISIBLE)) return TRUE;
       break;

     case ECOND_IS_DETECT_EVIL:
       if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) return TRUE;
       break;

     case ECOND_IS_DETECT_INVIS:
       if (IS_AFFECTED(ch, AFF_DETECT_INVIS)) return TRUE;
       break;

     case ECOND_IS_DETECT_MAGIC:
       if (IS_AFFECTED(ch, AFF_DETECT_MAGIC)) return TRUE;
       break;

     case ECOND_IS_DETECT_HIDDEN:
       if (IS_AFFECTED(ch, AFF_DETECT_HIDDEN)) return TRUE;
       break;

     case ECOND_IS_MELD:
       if (IS_AFFECTED(ch, AFF_MELD)) return TRUE;
       break;

     case ECOND_IS_SANCTUARY:
       if (IS_AFFECTED(ch, AFF_SANCTUARY)) return TRUE;
       break;

     case ECOND_IS_FAERIE_FIRE:
       if (IS_AFFECTED(ch, AFF_FAERIE_FIRE)) return TRUE;
       break;

     case ECOND_IS_INFRARED:
       if (IS_AFFECTED(ch, AFF_INFRARED)) return TRUE;
       break;

     case ECOND_IS_CURSE:
       if (IS_AFFECTED(ch, AFF_CURSE)) return TRUE;
       break;

     case ECOND_IS_FEAR:
       if (IS_AFFECTED(ch, AFF_FEAR)) return TRUE;
       break;

     case ECOND_IS_POISON:
       if (IS_AFFECTED(ch, AFF_POISON)) return TRUE;
       break;

     case ECOND_IS_PROT_GOOD:
       if (IS_AFFECTED(ch, AFF_PROTECTG)) return TRUE;
       break;

     case ECOND_IS_PROT_EVIL:
       if (IS_AFFECTED(ch, AFF_PROTECTE)) return TRUE;
       break;

     case ECOND_IS_SNEAK:
       if (IS_AFFECTED(ch, AFF_SNEAK)) return TRUE;
       break;

     case ECOND_IS_HIDE:
       if (IS_AFFECTED(ch, AFF_HIDE)) return TRUE;
       break;

     case ECOND_IS_SLEEP:
       if (IS_AFFECTED(ch, AFF_SLEEP)) return TRUE;
       break;

     case ECOND_IS_CHARM:
       if (IS_AFFECTED(ch, AFF_CHARM)) return TRUE;
       break;

     case ECOND_IS_FLYING:
       if (IS_AFFECTED(ch, AFF_FLYING)) return TRUE;
       break;

     case ECOND_IS_MIST:
       if (IS_AFFECTED(ch, AFF_MIST)) return TRUE;
       break;

     case ECOND_IS_PASS_DOOR:
       if (IS_AFFECTED(ch, AFF_PASS_DOOR)) return TRUE;
       break;

     case ECOND_IS_HASTE:
       if (IS_AFFECTED(ch, AFF_HASTE)) return TRUE;
       break;

     case ECOND_IS_CALM:
       if (IS_AFFECTED(ch, AFF_CALM)) return TRUE;
       break;

     case ECOND_IS_PLAGUE:
       if (IS_AFFECTED(ch, AFF_PLAGUE)) return TRUE;
       break;

     case ECOND_IS_WEAKEN:
       if (IS_AFFECTED(ch, AFF_WEAKEN)) return TRUE;
       break;

     case ECOND_IS_DARK_VISION:
       if (IS_AFFECTED(ch, AFF_DARK_VISION)) return TRUE;
       break;

     case ECOND_IS_BERSERK:
       if (IS_AFFECTED(ch, AFF_BERSERK)) return TRUE;
       break;

     case ECOND_IS_SWIM:
       if (IS_AFFECTED(ch, AFF_SWIM)) return TRUE;
       break;

     case ECOND_IS_REGENERATION:
       if (IS_AFFECTED(ch, AFF_REGENERATION)) return TRUE;
       break;

     case ECOND_IS_POLY:
       if (IS_AFFECTED(ch, AFF_POLY)) return TRUE;
       break;

     case ECOND_IS_ABSORB:
       if (IS_AFFECTED(ch, AFF_ABSORB)) return TRUE;
       break;

     case ECOND_IS_FLEEING:
       if (IS_FLEEING(ch)) return TRUE;
       break;

     case ECOND_IS_DREAMING:
       if ( ch->waking_room != NULL ) return TRUE;
       break;

     case ECOND_IS_DARKNESS:
       if (IS_AFFECTED(ch, AFF_DARKNESS)) return TRUE;
       break;

     case ECOND_IS_AURA:
       if (IS_AFFECTED(ch, AFF_AURA)) return TRUE;
       break;

     case ECOND_IS_SLOW:
       if (IS_AFFECTED(ch, AFF_SLOW)) return TRUE;
       break;

     case ECOND_IS_GLOBE:
       if (IS_AFFECTED(ch, AFF_GLOBE)) return TRUE;
       break;

     case ECOND_IS_HALLUCINATING:
       if (IS_AFFECTED(ch, AFF_HALLUCINATING)) return TRUE;
       break;

     case ECOND_IS_RELAXED:
       if (IS_AFFECTED(ch, AFF_RELAXED)) return TRUE;
       break;

     case ECOND_IS_FIRE_SHIELD:
       if (IS_AFFECTED(ch, AFF_FIRE_SHIELD)) return TRUE;
       break;

     case ECOND_IS_FROST_SHIELD:
       if (IS_AFFECTED(ch, AFF_FROST_SHIELD)) return TRUE;
       break;

     case ECOND_IS_MORF:
       if (IS_AFFECTED(ch, AFF_MORF)) return TRUE;
       break;

     case ECOND_IS_INCARNATED:
       if (IS_AFFECTED(ch, AFF_INCARNATED)) return TRUE;
       break;

     case ECOND_IS_CRIMINAL:
       if (IS_SET(ch->act, ACT_CRIMINAL)) return TRUE;
       break;

     case ECOND_IS_ELDER_SHIELD:
       if (IS_AFFECTED(ch, AFF_ELDER_SHIELD)) return TRUE;
       break;

     case ECOND_IS_FARSIGHT:
       if (IS_AFFECTED(ch, AFF_FARSIGHT)) return TRUE;
       break;

     case ECOND_IS_ASCETICISM:
       if (IS_AFFECTED(ch, AFF_ASCETICISM)) return TRUE;
       break;

     case ECOND_IS_WERE:
       if (IS_SET(ch->act, ACT_WERE)) return TRUE;
       break;

     case ECOND_IS_VAMPIRE:
       if (IS_SET(ch->act, ACT_VAMPIRE)) return TRUE;
       break;

     case ECOND_IS_UNDEAD:
       if (IS_SET(ch->act, ACT_UNDEAD)) return TRUE;
       break;

     case ECOND_IS_STRANGER:
       if (!ch->pcdata) return FALSE;
       if (!str_cmp(ch->pcdata->native, mud.name)) return TRUE;
       break;

     case ECOND_IS_PASSPORT:
       if (find_passport(ch)) return TRUE;
       break;

     default:
       break;
   }

   return FALSE;
 }


/* Evaluate a condition against a mob... */

 bool ec_satisfied_mob(ECOND_DATA *ec, CHAR_DATA *ch) {
   int val;
   bool sat;
   LPROF_DATA *lp;
   CONV_SUB_RECORD *csr;
   OBJ_DATA *obj;
   CHAR_DATA *mob;
   SOCIETY *society;
   SOCIALITE *social;
   MEMBERSHIP *memb;
    int type;
   bool found;
   long value;

  /* No char means the condition is false... */

   if (ch == NULL) return FALSE;
   
  /* Adjust for NOT... */

   type = ec->type;

   if (type < 0) {
     type *= -1;
   }

  /* Check by condition type... */

   switch (type) {

     /* Here_Empty_Mob... */

      case ECOND_IS_ALONE:

        if (ch->in_room == NULL) return FALSE;
        
        found = FALSE;
        mob = ch->in_room->people;
        while (mob != NULL) {
           if ( mob != ch ) found = TRUE;
           mob = mob->next_in_room; 
        }

        if (found) return FALSE;
        break;


      case ECOND_USES_IMP:

        if (IS_NPC(ch)) return FALSE;
        if (!IS_SET(ch->plr, PLR_IMP_HTML)) return FALSE;        
        break;


     /* skill 'skill' low high */

      case ECOND_SKILL:

        val = get_skill(ch, ec->index);

        if (val < ec->low) return FALSE;

        if ( ec->high != -1 
        && val > ec->high) {
            return FALSE;
        }
        break;  


      case ECOND_CRIMINAL_RATING:

        val = get_criminal_rating(ch);

        if (val < ec->low) return FALSE;

        if ( ec->high != -1 
        && val > ec->high) {
            return FALSE;
        }
        break;  


      case ECOND_ATTRIBUTE:

        val = get_curr_stat(ch, ec->index);

        if (val < ec->low) return FALSE;

        if ( ec->high != -1 
        && val > ec->high) {
            return FALSE;
        }

        break;  


     /* race 'race' */

      case ECOND_RACE:
        if (ch->race != ec->index) return FALSE;
        break; 


     /* level low high */

      case ECOND_LEVEL:

        if (ch->level < ec->low) return FALSE;

        if ( ec->high != -1 
        && ch->level > ec->high) {
          return FALSE;
        }

        break;  


      case ECOND_DIVINITY:

        val = (long) arrange_div(get_divinity(ch));

        if (val < ec->low) return FALSE;

        if ( ec->high != -1 
        && val > ec->high) {
          return FALSE;
        }

        break;  


      case ECOND_SIZE:

        if (get_char_size(ch) < ec->low) return FALSE;

        if ( ec->high != -1 
          && get_char_size(ch) > ec->high) {
          return FALSE;
        }

        break;  

     /* hits low high */

      case ECOND_HITS:

        if (ch->hit < ec->low) return FALSE;

        if ( ec->high != -1 
        && ch->hit > ec->high) {
            return FALSE;
        }

        break;  

     /* mana low high */

      case ECOND_MANA:

        if (ch->mana < ec->low) {
          return FALSE;
        }

        if ( ec->high != -1 
          && ch->mana > ec->high) {
          return FALSE;
        }

        break;  

     /* move low high */

      case ECOND_MOVE:

        if (ch->move < ec->low) {
          return FALSE;
        }

        if ( ec->high != -1 
          && ch->move > ec->high) {
          return FALSE;
        }

        break;  

     /* hits_percent low high */

      case ECOND_HITS_PERCENT:

        val = (100 * ch->hit)/100;

        if (val < ec->low) {
          return FALSE;
        }

        if ( ec->high != -1 
          && val > ec->high) {
          return FALSE;
        }

        break;  

     /* mana_percent low high */

      case ECOND_MANA_PERCENT:

        val = (100 * ch->mana)/100;

        if (val < ec->low) {
          return FALSE;
        }

        if ( ec->high != -1 
          && val > ec->high) {
          return FALSE;
        }

        break;  

     /* move_percent low high */

      case ECOND_MOVE_PERCENT:

        val = (100 * ch->move)/100;

        if (val < ec->low) {
          return FALSE;
        }

        if ( ec->high != -1 
          && val > ec->high) {
          return FALSE;
        }

        break;  

     /* gold low high */
      case ECOND_GOLD:
        if (get_full_cash(ch) < ec->low) return FALSE;

        if ( ec->high != -1 
        && get_full_cash(ch) > ec->high) {
          return FALSE;
        }
        break;  

     /* pracs low high */
      case ECOND_PRACTICE:
        if (ch->practice < ec->low) return FALSE;

        if ( ec->high != -1 
        && ch->practice > ec->high) {
          return FALSE;
        }
        break;  

     /* train low high */
      case ECOND_TRAIN:
        if (ch->train < ec->low) return FALSE;

        if ( ec->high != -1 
        && ch->train > ec->high) {
          return FALSE;
        }
        break;  

      case ECOND_FAME:

        if (IS_NPC(ch)) return FALSE;

        if (ch->pcdata->questpoints < ec->low) return FALSE;
        if ( ec->high != -1 && ch->pcdata->questpoints > ec->high) return FALSE;
        break;  

     /* sanity low high */

      case ECOND_SANITY:

        if (get_sanity(ch) < ec->low) {
          return FALSE;
        }

        if ( ec->high != -1 
          && get_sanity(ch) > ec->high) {
          return FALSE;
        }

        break;  

      case ECOND_MEMORY:

        if ( ec->index < 0 
          || ec->index >= MOB_MEMORY) {
          return FALSE;
        } 

        if (ch->memory[ec->index] == NULL) {
          return FALSE;
        }

        break;  

      case ECOND_MEMORYV:

        if ( ec->index < 0 
          || ec->index >= MOB_MEMORY) {
          return FALSE;
        } 

        val = atoi(ch->memory[ec->index]);

        if (val < ec->low) {
          return FALSE;
        }

        if ( ec->high != -1 
          && val > ec->high) {
          return FALSE;
        }

        break;  

     /* casting_level high low */

      case ECOND_CASTING_LEVEL:

        val = casting_level(ch);

        if (val < ec->low) {
          return FALSE;
        }

        if ( ec->high != -1 
          && val > ec->high) {
          return FALSE;
        }

        break;  

     /* Align high low */

      case ECOND_ALIGN:

        if (ch->alignment < ec->low) {
          return FALSE;
        }

        if ( ch->alignment > ec->high) {
          return FALSE;
        }

        break;  

     /* prof 'profession' high low */

      case ECOND_PROF:
        sat = FALSE;
        for(lp = ch->profs; lp != NULL; lp = lp->next) {
            if (lp->profession->id == ec->index) {
                sat = TRUE;
                if (lp->level < ec->low) return FALSE;
                else if ( ec->high != -1 
                && lp->level > ec->high) return FALSE;
            }
        }
        if (!sat) return FALSE;
        break;

      case ECOND_START_ROOM:
        if ((lp = ch->profs) == NULL) return FALSE;

        if (lp->profession->start < ec->low) return FALSE;
        if (ec->high != -1 && lp->profession->start > ec->high) return FALSE;
        break;

     /* soc_rank 'soc_id' template 0 */

      case ECOND_SOC_RANK:

        society = find_society_by_id(ec->index); 

        if (society == NULL) {
          return FALSE;
        }

        val = ECOND_SOC_RANK_NONE;

        memb = find_membership(ch, society);

        if (memb != NULL) {
          
          if (memb->level == SOC_LEVEL_INVITED) {
            val = ECOND_SOC_RANK_INVITED; 
          } else {
            social = find_socialite(society, ch->name);

            if (social != NULL) {
              switch (social->rank) {
                case SOC_RANK_MEMBER:
                  val = ECOND_SOC_RANK_MEMBER;
                  break;
                case SOC_RANK_COUNCIL:
                  val = ECOND_SOC_RANK_COUNCIL;
                  break;
                case SOC_RANK_LEADER:
                  val = ECOND_SOC_RANK_LEADER;
                  break;
                default:
                  break; 
              }
            }
          }
        }
 
        if ((val & ec->low) == 0) {
          return FALSE;
        }

        break;

     /* soc_level 'soc_id' low high */

      case ECOND_SOC_LEVEL:

        society = find_society_by_id(ec->index); 
        if (society == NULL) return FALSE;

        memb = find_membership(ch, society);
        if (memb == NULL) return FALSE;
          
        if (memb->level < ec->low) return FALSE;

        if ( ec->high != -1 
        && memb->level > ec->high) {
          return FALSE;
        }

        break;

     /* sex sex1 sex2 */

      case ECOND_SEX:

        if ( ch->sex != ec->low
          && ch->sex != ec->high ) {
          return FALSE;
        }

        break; 

     /* soc_auth 'soc_id' template 0 */

      case ECOND_SOC_AUTH:

        society = find_society_by_id(ec->index); 
        if (society == NULL) return FALSE;

        social = find_socialite(society, ch->name);
        if (social == NULL) return FALSE;

        if ((social->authority & ec->low) == 0) return FALSE;
        break;


     /* conv conv_id subj state */

      case ECOND_CONV:

        csr = find_csr(ch, ec->index, ec->low);

        if (csr == NULL) {
          return FALSE;
        }

        if (csr->state != ec->high) {
          return FALSE;
        }

        break;    

     /* carrying vnum vnum */

      case ECOND_CARRYING: 
        obj = ch->carrying;
        found = FALSE;

        while (obj) { 

          if ( obj->pIndexData != NULL
          && ( obj->pIndexData->vnum == ec->low  || obj->pIndexData->vnum == ec->high)) {
                found = TRUE; 
          }
          obj = obj->next_content;
        }

        if (!found) return FALSE;
        break;


      case ECOND_CARRYING_TYPE: 
        obj = ch->carrying;
        found = FALSE;

        while (obj) { 

          if ( obj->pIndexData != NULL
          && (obj->pIndexData->item_type == ec->low  || obj->pIndexData->item_type == ec->high)) {
                found = TRUE; 
          }
          obj = obj->next_content;
        }

        if (!found) return FALSE;
        break;


     /* wearing vnum vnum */

      case ECOND_WEARING: 
        obj = ch->carrying;
        found = FALSE;

        while (obj) {
          if ( obj->wear_loc != WEAR_NONE 
          && obj->pIndexData != NULL 
          && ( obj->pIndexData->vnum == ec->low  || obj->pIndexData->vnum == ec->high)) {
            found = TRUE; 
          }
          obj = obj->next_content;
        }

        if (!found) return FALSE;
        break;


      case ECOND_WEARING_TYPE: 
        obj = ch->carrying;
        found = FALSE;

        while (obj) {
          if ( obj->wear_loc != WEAR_NONE 
          && obj->pIndexData != NULL 
          && ( obj->pIndexData->item_type == ec->low  || obj->pIndexData->item_type == ec->high)) {
            found = TRUE; 
          }
          obj = obj->next_content;
        }

        if (!found) return FALSE;
        break;


     /* vnum high low */

      case ECOND_VNUM:

        if (ch->pIndexData == NULL) {
          return FALSE;
        }

        if (ch->pIndexData->vnum < ec->low) {
          return FALSE;
        }

        if ( ec->high != -1 
          && ch->pIndexData->vnum > ec->high) {
          return FALSE;
        }

        break;  

     /* in_room high low */

      case ECOND_IN_ROOM:

        if ( ch->in_room == NULL ) {
          return FALSE;
        }

        if (ch->in_room->vnum < ec->low) {
          return FALSE;
        }

        if ( ec->high != -1 
          && ch->in_room->vnum > ec->high) {
          return FALSE;
        }

        break;  


     /* in_subarea high low */

      case ECOND_IN_SUBAREA:

        if ( ch->in_room == NULL ) {
          return FALSE;
        }

        if (ch->in_room->subarea < ec->low) {
          return FALSE;
        }

        if ( ec->high != -1 
          && ch->in_room->subarea > ec->high) {
          return FALSE;
        }

        break;  

     /* deed id flag */

      case ECOND_DEED:

          if (ch->desc != NULL) {
                if (ch->desc->original != NULL) {
                     if ( ec->high == 1
                     && !doneDeed(ch->desc->original, ec->low)) {
                         return FALSE;
                     }

                     if ( ec->high == 0
                     && doneDeed(ch->desc->original, ec->low)) {
                         return FALSE;
                     } 
                     break;
                }
          }

        if ( ec->high == 1
          && !doneDeed(ch, ec->low)) {
          return FALSE;
        }

        if ( ec->high == 0
          && doneDeed(ch, ec->low)) {
          return FALSE;
        } 

        break; 

     /* quest id high low */

      case ECOND_QUEST:

        val = quest_state(ch, ec->index);

        if (val < ec->low) return FALSE;
        
        if ( ec->high != -1 
          && val > ec->high) {
          return FALSE;
        }

        break;  

     /* is condition */
 
      case ECOND_IS:

        if (!ec_satisfied_mob_is(ch, ec->index)) {
          return FALSE;
        }

        break; 

     /* is not condition */

      case ECOND_IS_NOT:

        if (ec_satisfied_mob_is(ch, ec->index)) {
          return FALSE;
        }

        break; 

     /* affected affect */

      case ECOND_AFFECTED:

        if (!is_affected(ch, ec->index)) {
          return FALSE;
        }

        break;  

     /* TBD */

      case ECOND_LIGHT:

     /* name 'name' */

      case ECOND_NAME:

        if ( ec->text == NULL ) return FALSE;
        if ( !is_name(ch->name, ec->text) ) return FALSE;
        break;

      case ECOND_SHORT:

        if ( ec->text == NULL ) return FALSE;
        if ( !is_name(ch->short_descr, ec->text) ) return FALSE;
        break;

      case ECOND_CULT:

        if (ec->text == NULL ) return FALSE;
        if (IS_NPC(ch)) return FALSE;
        if ( !is_name(ch->pcdata->cult, ec->text))  return FALSE;
        break;

      case ECOND_KNOWS_DISCIPLINE:

        if (ec->text == NULL ) return FALSE;
        value = flag_value(discipline_type, ec->text);
        if ( value == NO_FLAG ) return FALSE;
        if (!knows_discipline(ch, value)) return FALSE;
        break;

     /* Unrecognized condition is False... */

      default:

        bug("Unrecognized MOB condition %d!", ec->type);

        return FALSE;
    }
      
    return TRUE;
  }

/* Evaluate an object condition... */

 bool ec_satisfied_obj(ECOND_DATA *ec, OBJ_DATA *obj) {
int type;
bool found;

  /* No object means it's false... */

   if (obj == NULL) return FALSE;
  
  /* Adjust for NOT... */

   type = ec->type;
   if (type < 0) type *= -1;


  /* Check by condition type... */

   switch (type) {

      case ECOND_LEVEL:

        if (obj->level < ec->low) return FALSE;

        if ( ec->high != -1 
        && obj->level > ec->high) {
          return FALSE;
        }
        break;  

      case ECOND_NAME:

        if ( ec->text == NULL ) return FALSE;
        if ( !is_name(obj->name, ec->text) ) return FALSE;
        break;

      case ECOND_OWNER:

        if ( ec->text == NULL ) return FALSE;
        if ( !is_name(obj->owner, ec->text) ) return FALSE;
        break;

      case ECOND_TYPE:

        if (obj->item_type < ec->low) return FALSE;

        if ( ec->high != -1 
        && obj->item_type > ec->high) {
          return FALSE;
        }
        break;  


      case ECOND_VNUM:

        if (obj->pIndexData == NULL) return FALSE;
        if (obj->pIndexData->vnum < ec->low) return FALSE;

        if ( ec->high != -1 
        && obj->pIndexData->vnum > ec->high) {
          return FALSE;
        }
        break;  


      case ECOND_VALUE:

        if (obj->value[ec->index] < ec->low) return FALSE;

        if ( ec->high != -1 
        && obj->value[ec->index] > ec->high) {
          return FALSE;
        }
        break;  

     /* contains vnum vnum */

      case ECOND_CONTAINS: 

        switch (obj->item_type) {

          case ITEM_SCROLL:
          case ITEM_PILL:
          case ITEM_POTION:

            if ( obj->value[1] != ec->low
              && obj->value[2] != ec->low
              && obj->value[3] != ec->low ) {
              return FALSE;
            }
    
            if ( ec->high != -1
              && obj->value[1] != ec->high
              && obj->value[2] != ec->high
              && obj->value[3] != ec->high ) {
              return FALSE;
            }  

            break;             

          case ITEM_LIGHT:

            if ( obj->value[2] != -1
              && obj->value[2] != 999
              && obj->value[2] < ec->low  ) {
              return FALSE;
            }  

            break;             

          case ITEM_WAND:
          case ITEM_STAFF:

            if ( obj->value[3] != ec->low 
              || obj->value[2] < ec->high ) {
              return FALSE;
            }  

            break;             

          case ITEM_FOUNTAIN:

            if ( obj->value[0] != ec->low ) {
              return FALSE;
            }  

            break;             

          case ITEM_DRINK_CON:

            if ( obj->value[2] != ec->low 
              || obj->value[1] < ec->high ) {
              return FALSE;
            }  

            break;             

          case ITEM_CONTAINER:
          case ITEM_LOCKER:
          case ITEM_KEY_RING: 

            obj = obj->contains;

            found = FALSE;

            while ( obj != NULL ) { 
 
              if ( obj->pIndexData != NULL
                && ( obj->pIndexData->vnum == ec->low
                  || obj->pIndexData->vnum == ec->high)) {
                found = TRUE; 
              }

              obj = obj->next_content;
            }

            if (!found) return FALSE;
            break;

          default:
            return FALSE;
        }

        break;

     /* Unrecognized condition is False... */

      default:
        bug("Unrecognized OBJ condition %d!", ec->type);
        return FALSE;
    }
      
    return TRUE;
  }

/* Evaluate an wev condition... */

 bool ec_satisfied_wev(ECOND_DATA *ec, WEV *wev) {

   int type;
   int val;

  /* No event means it's false... */

   if (wev == NULL) {
     return FALSE;
   }  
  
  /* Adjust for NOT... */

   type = ec->type;

   if (type < 0) {
     type *= -1;
   }

  /* Check by condition type... */

   switch (type) {

     /* type high low */

      case ECOND_TYPE:

        if (wev->type < ec->low) {
          return FALSE;
        }

        if ( ec->high != -1 
          && wev->type > ec->high) {
          return FALSE;
        }

        break;  

     /* subtype high low */

      case ECOND_SUBTYPE:

        if (wev->subtype < ec->low) {
          return FALSE;
        }

        if ( ec->high != -1 
          && wev->subtype > ec->high) {
          return FALSE;
        }

        break;  

     /* number high low */

      case ECOND_NUMBER:

        if (wev->context->number < ec->low) {
          return FALSE;
        }

        if ( ec->high != -1 
          && wev->context->number > ec->high) {
          return FALSE;
        }

        break;  

     /* number_mod mod low high */

      case ECOND_NUMBER_MOD:

        if (ec->index == 0) {
          return FALSE;
        }

        val = wev->context->number % ec->index;

        if (val < ec->low) {
          return FALSE;
        }

        if ( ec->high != -1 
          && val > ec->high) {
          return FALSE;
        }

        break;

     /* am_actor 0 0 */

      case ECOND_AM_ACTOR:

        if (wev->context->mob != wev->context->actor) {
          return FALSE;
        }

        break;  

     /* am_victim 0 0 */

      case ECOND_AM_VICTIM:

        if (wev->context->mob != wev->context->victim) {
          return FALSE;
        }

        break;  

     /* local 0 0 */

      case ECOND_LOCAL:

        if (wev->context->mob->in_room != wev->context->room) {
          return FALSE;
        }

        break;  

     /* actor_grouped 0 0 */

      case ECOND_ACTOR_GROUPED:

        if (!is_same_group(wev->context->mob, wev->context->actor)) {
          return FALSE;
        }

        break;  

     /* victim_grouped 0 0 */

      case ECOND_VICTIM_GROUPED:

        if (!is_same_group(wev->context->mob, wev->context->victim)) {
          return FALSE;
        }

        break;  

     /* actor_remembered slot */

      case ECOND_ACTOR_REMEMBERED:

        if (wev->context->actor == NULL) {
          return FALSE;
        }

        if ( ec->low < 0
          || ec->low >= MOB_MEMORY ) {
          return FALSE;
        }

        if (wev->context->mob->memory[ec->low] == NULL) {
          return FALSE;
        } 

        if ( str_cmp(wev->context->actor->true_name,
                     wev->context->mob->memory[ec->low])
          && !is_name(wev->context->actor->name,  
                      wev->context->mob->memory[ec->low])) {
          return FALSE;
        }

        break;  

     /* victim_remembered slot */

      case ECOND_VICTIM_REMEMBERED:

        if (wev->context->victim == NULL) {
          return FALSE;
        }

        if ( ec->low < 0
          || ec->low >= MOB_MEMORY ) {
          return FALSE;
        }

        if (wev->context->mob->memory[ec->low] == NULL) {
          return FALSE;
        } 
      
        if ( str_cmp(wev->context->victim->true_name,
                     wev->context->mob->memory[ec->low])
          && !is_name(wev->context->victim->name,  
                      wev->context->mob->memory[ec->low])) {
          return FALSE;
        }

        break;  

     /* actor_friend 0 0 */

      case ECOND_ACTOR_FRIEND:

        if (!is_friend(wev->context->mob, wev->context->actor)) {
          return FALSE;
        }

        break;  

     /* actor_foe 0 0 */

      case ECOND_ACTOR_FOE:

        if (!is_foe(wev->context->mob, wev->context->actor)) {
          return FALSE;
        }

        break;  

     /* victim_friend 0 0 */

      case ECOND_VICTIM_FRIEND:

        if (!is_friend(wev->context->mob, wev->context->victim)) {
          return FALSE;
        }

        break;  

     /* victim_foe 0 0 */

      case ECOND_VICTIM_FOE:

        if (!is_foe(wev->context->mob, wev->context->victim)) {
          return FALSE;
        }

        break;  

     /* actor_visible 0 0 */

      case ECOND_ACTOR_VISIBLE:

        if (!can_see(wev->context->mob, wev->context->actor)) {
          return FALSE;
        }

        break;  

     /* victim_visible 0 0 */

      case ECOND_VICTIM_VISIBLE:

        if (!can_see(wev->context->mob, wev->context->victim)) {
          return FALSE;
        }

        break;  

     /* Unrecognized condition is False... */

      default:

        bug("Unrecognized WEV condition %d!", ec->type);

        return FALSE;
    }
      
    return TRUE;
  }

/* Find a mobs ultimate leader or master... */

CHAR_DATA *find_boss(CHAR_DATA *ch) {

  CHAR_DATA *boss;

 /* No character, no leader... */

  if (ch == NULL) {
    return NULL;
  }

 /* Start as your own boss... */

  boss = ch;

 /* Then wind up the command chain... */ 

  while ( boss->leader != NULL
       && boss->master != NULL ) {

   /* Note that leader takes preference of master... */

    if (boss->leader != NULL) {
      boss = boss->leader;
    } else if (boss->master != NULL) {
      boss = boss->master;
    }

  } 

 /* Return whomever we have found... */

  return boss;
}

/* Evaluate an econd against an mcc... */

bool ec_satisfied_mcc(MOB_CMD_CONTEXT *mcc, ECOND_DATA *ec, bool recurse) {
CHAR_DATA *ch;
bool sat;
OBJ_DATA *obj;
WEV *wev;
bool enot;

  /* No condition is true... */

   if (!ec) return TRUE;

  /* Id the char time... */

   ch = NULL;
   obj = NULL;
   wev = NULL;

   switch (ec->subject) {

     case ECOND_SUBJECT_WORLD:
       break;
 
     case ECOND_SUBJECT_ACTOR:
       ch = mcc->actor;
       break;

     case ECOND_SUBJECT_VICTIM:
       ch = mcc->victim;
       break;

     case ECOND_SUBJECT_OBSERVER:
       ch = mcc->mob;
       break; 

     case ECOND_SUBJECT_RANDOM:
       ch = mcc->random;
       break;

     case ECOND_SUBJECT_L_ACTOR:
       ch = find_boss(mcc->actor);
       break;

     case ECOND_SUBJECT_L_VICTIM:
       ch = find_boss(mcc->victim);
       break;

     case ECOND_SUBJECT_L_OBSERVER:
       ch = find_boss(mcc->mob);
       break; 

     case ECOND_SUBJECT_L_RANDOM:
       ch = find_boss(mcc->random);
       break;

     case ECOND_SUBJECT_P_OBJ:
       obj = mcc->obj;
       break; 

     case ECOND_SUBJECT_S_OBJ:
       obj = mcc->obj2;
       break; 

     case ECOND_SUBJECT_WEV:
       wev = mcc->wev;
       break; 

     default:
       ch = mcc->mob;
   }

  /* Pick out NOT conditions... */

   enot = FALSE;

   if (ec->type < 0) {
     enot = TRUE;
   }

  /* Make the appropriate evaluation... */

   sat = FALSE;

   if (ch != NULL) {
     sat = ec_satisfied_mob(ec, ch);
   } else if (obj != NULL) {
     sat = ec_satisfied_obj(ec, obj);
   } else if (wev != NULL) {
     sat = ec_satisfied_wev(ec, wev);
   } else {
     sat = ec_satisfied_world(ec);
   }

  /* Return FALSE if not satisfied... */

   if (!enot & !sat) return FALSE;
    
  /* Return FALSE if not (not satisfied)... */

   if (enot & sat) return FALSE;
    
  /* Check the rest... */
  
   if (recurse) {
       return ec_satisfied_mcc(mcc, ec->next, recurse);
   } else {
       return TRUE;
   }
 }

/* Word/number mapping for is conditions... */

static struct is_mapping {
  char 	*name;
  int	 number;
} is_map[] = {

	{	"hungry",		ECOND_IS_HUNGRY},
	{	"full",			ECOND_IS_FULL},
	{	"thirsty",		ECOND_IS_THIRSTY},
	{	"refreshed",		ECOND_IS_REFRESHED},
	{	"sober",			ECOND_IS_SOBER},
	{	"drunk",			ECOND_IS_DRUNK},
	{	"player",		ECOND_IS_PLAYER},
	{	"standing",		ECOND_IS_STANDING},
	{	"fighting",		ECOND_IS_FIGHTING},
	{	"sitting",		ECOND_IS_SITTING},
	{	"resting",		ECOND_IS_RESTING},
	{	"awake",		ECOND_IS_AWAKE},
	{	"sleeping",		ECOND_IS_SLEEPING},
	{	"stunned",		ECOND_IS_STUNNED},
	{	"dieing",		ECOND_IS_DIEING},
	{	"dead",			ECOND_IS_DEAD},
	{	"blind",			ECOND_IS_BLIND},
	{	"invisible",		ECOND_IS_INVISIBLE},
	{	"seeing_evil",		ECOND_IS_DETECT_EVIL},
	{	"seeing_invisible",	ECOND_IS_DETECT_INVIS},
	{	"seeing_magic",		ECOND_IS_DETECT_MAGIC},
	{	"seeing_hidden",		ECOND_IS_DETECT_HIDDEN},
	{	"mind_melded",		ECOND_IS_MELD},
	{	"sanctuary",		ECOND_IS_SANCTUARY},
	{	"faerie_fire",		ECOND_IS_FAERIE_FIRE	},
	{	"seeing_infrared",	ECOND_IS_INFRARED},
	{	"cursed",		ECOND_IS_CURSE},
	{	"scared",		ECOND_IS_FEAR},
	{	"poisoned",		ECOND_IS_POISON},
	{	"protected_from_good",	ECOND_IS_PROT_GOOD},
	{	"protected_from_evil",	ECOND_IS_PROT_EVIL},
	{	"sneaking",		ECOND_IS_SNEAK},
	{	"charmed",		ECOND_IS_CHARM},
	{	"flying",			ECOND_IS_FLYING},
	{	"pass_door",		ECOND_IS_PASS_DOOR},
	{	"hasted",		ECOND_IS_HASTE},
	{	"calmed",		ECOND_IS_CALM},
	{	"plauge_ridden",		ECOND_IS_PLAGUE},
	{	"weakened",		ECOND_IS_WEAKEN},
	{	"seeing_in_darkness",	ECOND_IS_DARK_VISION},
	{	"berserk",		ECOND_IS_BERSERK},
	{	"swimming",		ECOND_IS_SWIM},
	{	"regenerating",		ECOND_IS_REGENERATION},
	{	"polymorphed",		ECOND_IS_POLY},
	{	"absorbing_magic",	ECOND_IS_ABSORB},
	{	"fleeing",		ECOND_IS_FLEEING},
	{	"dreaming",		ECOND_IS_DREAMING},
	{	"starving",		ECOND_IS_STARVING},
	{	"dehydrated",		ECOND_IS_DEHYDRATED},
	{	"overweight",		ECOND_IS_OVERWEIGHT},
	{	"darkness",		ECOND_IS_DARKNESS},
	{	"aura",			ECOND_IS_AURA},
	{	"hallucinating",		ECOND_IS_HALLUCINATING},
	{	"relaxed",		ECOND_IS_RELAXED},
	{	"fire-shield",		ECOND_IS_FIRE_SHIELD},
	{	"frost-shield",		ECOND_IS_FROST_SHIELD},
	{	"morf",	                   	ECOND_IS_MORF},
	{	"incarnated",		ECOND_IS_INCARNATED},
	{	"criminal",		ECOND_IS_CRIMINAL},
	{	"mist",	                   	ECOND_IS_MIST},
	{	"elder-shield",		ECOND_IS_ELDER_SHIELD},
	{	"farsight",		ECOND_IS_FARSIGHT},
	{	"asceticism",		ECOND_IS_ASCETICISM},
	{	"were",	                   	ECOND_IS_WERE},
	{	"vampire",		ECOND_IS_VAMPIRE},
	{	"undead",		ECOND_IS_UNDEAD},
	{	"stranger",	              	ECOND_IS_STRANGER},
	{	"passport",	              	ECOND_IS_PASSPORT},
	{	"",			0			}

};

/* Map from string to number... */

int get_is_number(char *name) {

  int i;

  i = 0;

  while (is_map[i].number != 0) {

    if (name[0] == is_map[i].name[0]
      && !str_cmp(name, is_map[i].name)) {
      return is_map[i].number;
    } 

    i += 1;
  }

  return 0;
}

/* Map from number to string... */

char *get_is_name(int number) {

  int i;

  i = 0;

  while (is_map[i].number != 0) {

    if (number == is_map[i].number) {
      return is_map[i].name;
    } 

    i += 1;
  }

  return 0;
}

/* Read an econd from a file... */

/* NB. Do NOT return until all numbers have been read... */

ECOND_DATA *read_econd(FILE *fp) {

  char *qualif;

  ECOND_DATA *ec;

  int type;

 /* First, we need an econd... */

  ec = get_ec();

 /* Subject... */ 

  ec->subject = fread_number(fp);

 /* Condition type */ 

  ec->type = fread_number(fp);

 /* The the target or 'ignored'... */

  qualif = fread_word(fp);

 /* Adjust for not... */

  type = ec->type;

  if (type < 0) {
    type *= -1;
  }

 /* Translate this to a number... */

  switch (type) {

    case ECOND_SKILL:
      ec->index = get_skill_sn(qualif);
      break;

    case ECOND_ATTRIBUTE:
        if (!str_cmp(qualif, "strength") || !str_cmp(qualif, "str")) ec->index = STAT_STR;
        else if (!str_cmp(qualif, "intelligence") || !str_cmp(qualif, "int")) ec->index = STAT_INT;
        else if (!str_cmp(qualif, "wisdom") || !str_cmp(qualif, "wis")) ec->index = STAT_WIS;
        else if (!str_cmp(qualif, "dexterity") || !str_cmp(qualif, "dex")) ec->index = STAT_DEX;
        else if (!str_cmp(qualif, "constitution") || !str_cmp(qualif, "con")) ec->index = STAT_INT;
        else ec->index = 0;
      break;

    case ECOND_RACE:
      ec->index = race_lookup(qualif);
      break;

    case ECOND_PROF:
      ec->index = get_prof_id(qualif);
      break;

    case ECOND_AFFECTED:
      ec->index = get_affect_afn(qualif);
      break;

    case ECOND_NAME:
    case ECOND_OWNER:
    case ECOND_SHORT:
    case ECOND_CULT:
    case ECOND_KNOWS_DISCIPLINE:
    case ECOND_PARTNER:
      ec->text = str_dup(qualif);
      break;

    case ECOND_LEVEL:
    case ECOND_DIVINITY:
    case ECOND_CRIMINAL_RATING:
    case ECOND_SIZE: 
    case ECOND_IS_ALONE: 
    case ECOND_USES_IMP:
    case ECOND_HITS:
    case ECOND_MANA:
    case ECOND_MOVE:
    case ECOND_HITS_PERCENT:
    case ECOND_MANA_PERCENT:
    case ECOND_MOVE_PERCENT:
    case ECOND_SANITY:
    case ECOND_GOLD:
    case ECOND_START_ROOM:
    case ECOND_PRACTICE:
    case ECOND_TRAIN:
    case ECOND_FAME:
    case ECOND_CASTING_LEVEL:
    case ECOND_LIGHT:
    case ECOND_ALIGN:
    case ECOND_MOON:
    case ECOND_NONE:
    case ECOND_SEX:  
    case ECOND_RANDOM:  
    case ECOND_CARRYING:  
    case ECOND_WEARING:  
    case ECOND_CARRYING_TYPE:  
    case ECOND_WEARING_TYPE:  
    case ECOND_HOUR_OF_DAY:
    case ECOND_DAY_OF_MONTH:
    case ECOND_DAY_OF_YEAR:
    case ECOND_WEEK_OF_YEAR: 
    case ECOND_MONTH_OF_YEAR: 
    case ECOND_TYPE:  
    case ECOND_SUBTYPE:  
    case ECOND_VNUM:  
    case ECOND_IN_ROOM:  
    case ECOND_IN_SUBAREA:  
    case ECOND_DEED:  
    case ECOND_NUMBER:  
    case ECOND_AM_ACTOR:  
    case ECOND_AM_VICTIM:  
    case ECOND_LOCAL:  
    case ECOND_ACTOR_GROUPED:  
    case ECOND_VICTIM_GROUPED:  
    case ECOND_CONTAINS:  
    case ECOND_ACTOR_REMEMBERED:  
    case ECOND_VICTIM_REMEMBERED:  
    case ECOND_ACTOR_FRIEND:  
    case ECOND_ACTOR_FOE:  
    case ECOND_VICTIM_FRIEND:  
    case ECOND_VICTIM_FOE:  
    case ECOND_ACTOR_VISIBLE:  
    case ECOND_VICTIM_VISIBLE:  
      ec->index = 0;
      break;

    case ECOND_HOUR_OF_DAY_MOD:
    case ECOND_DAY_OF_MONTH_MOD:
    case ECOND_DAY_OF_YEAR_MOD:
    case ECOND_WEEK_OF_YEAR_MOD: 
    case ECOND_MONTH_OF_YEAR_MOD: 
    case ECOND_NUMBER_MOD:
    case ECOND_SOC_RANK:
    case ECOND_SOC_LEVEL:
    case ECOND_SOC_AUTH:
    case ECOND_QUEST:
      ec->index = atoi(qualif); 

      if (ec->index == 0) {
        bug("Econd numeric is zero.", 0);
        ec->index = 1000; 
      }

      if (ec->index < 0) {
        bug("Econd numeric is negative!", 0);
        ec->index = 1000; 
      }

      break; 

    case ECOND_CONV:
    case ECOND_GADGET: 
    case ECOND_MOB_IN_ROOM: 
    case ECOND_PLAYER_IN_ROOM: 
    case ECOND_ROOM_EMPTY_MOB: 
    case ECOND_OBJ_IN_ROOM: 
    case ECOND_OBJ_ROOM_CONTAINER: 
    case ECOND_ROOM_EMPTY_OBJ: 
    case ECOND_MEMORY: 
    case ECOND_MEMORYV: 
      ec->index = atoi(qualif); 

      if (ec->index < 0) {
        bug("Econd numeric is negative!", 0);
        ec->index = 0;
      }

      break; 

    case ECOND_VALUE:
      ec->index = atoi(qualif); 

      if (ec->index < 0) {
        bug("Econd numeric is negative!", 0);
        ec->index = 0;
      }

      if (ec->index > 4) {
        bug("Econd numeric is greater than 4!", 0);
        ec->index = 0;
      }

      break; 

    case ECOND_IS:  
    case ECOND_IS_NOT: 
      ec->index = get_is_number(qualif);
 
      if (ec->index <= 0) {
        bug("Is/Is_Not index resolved to 0!", 0);
      }

      break;      
 
    default:
      ec->type = ECOND_NONE;
      ec->index = 0;
      bug("Invalid condition!", 0); 
  }  

 /* Then follow 2 more range values... */ 
 
  ec->low  = fread_number(fp);
  ec->high = fread_number(fp);

  ec->next = NULL;

 /* All done... */

  return ec;
}


/* Write a string of ec's out... */

void write_econd(ECOND_DATA *ec, FILE *fp, char *hdr) {
char buf[MAX_STRING_LENGTH];
char *qualif;
PROF_DATA *pd;
int type;

 /* Write the chain out one by one... */

  while (ec != NULL) {
  
   /* Work out the qualifier... */

    buf[0] = '\0';
    qualif = buf;

   /* Adjust for not... */
 
    type = ec->type;

    if (type < 0) {
      type *= -1;
    }

    switch (type) {

      case ECOND_SKILL:
        if (ec->index != SKILL_UNDEFINED) {
          qualif = skill_array[ec->index].name;
        }
        break; 

      case ECOND_ATTRIBUTE:
        if (ec->index == STAT_STR) qualif = "strength";
        else if (ec->index == STAT_INT) qualif = "intelligence";
        else if (ec->index == STAT_WIS) qualif = "wisdom";
        else if (ec->index == STAT_DEX) qualif = "dexterity";
        else if (ec->index == STAT_CON) qualif = "constitution";
        else qualif = "strength";
        break; 

      case ECOND_NAME:
      case ECOND_OWNER:
      case ECOND_SHORT:
      case ECOND_CULT:
      case ECOND_PARTNER:
      case ECOND_KNOWS_DISCIPLINE:
        if ( ec->text == NULL ) {
          qualif = "(null)";
        } else {
          qualif = ec->text;
        }
        break;

      case ECOND_ALIGN:
        qualif = "alignment";
        break;

      case ECOND_LIGHT:
        qualif = "light";
        break;

      case ECOND_LEVEL:
        qualif = "level";
        break;

      case ECOND_DIVINITY:
        qualif = "divinity";
        break;

      case ECOND_CRIMINAL_RATING:
        qualif = "criminal_rating";
        break;

      case ECOND_HITS:
        qualif = "hits";
        break;

      case ECOND_MANA:
        qualif = "mana";
        break;

      case ECOND_MOVE:
        qualif = "move";
        break;

      case ECOND_HITS_PERCENT:
        qualif = "hits_percent";
        break;

      case ECOND_MANA_PERCENT:
        qualif = "mana_percent";
        break;

      case ECOND_MOVE_PERCENT:
        qualif = "move_percent";
        break;

      case ECOND_GOLD:
        qualif = "gold";
        break;

      case ECOND_PRACTICE:
        qualif = "prac";
        break;

      case ECOND_TRAIN:
        qualif = "train";
        break;

      case ECOND_START_ROOM:
        qualif = "start";
        break;

      case ECOND_FAME:
        qualif = "fame";
        break;

      case ECOND_SANITY:
        qualif = "sanity";
        break;

      case ECOND_CASTING_LEVEL:
        qualif = "casting_level";
        break;

      case ECOND_RACE:
        qualif = race_array[ec->index].name;  
        break;

      case ECOND_MOON:
        qualif = "moon";
        break;

      case ECOND_SEX:
        qualif = "gender";
        break;

      case ECOND_PROF:
        pd = get_prof_by_id(ec->index);
        if (pd != NULL) {
          qualif = pd->name;
        }
        break;

      case ECOND_AFFECTED:
        if (ec->index != AFFECT_UNDEFINED) {
          qualif = affect_array[ec->index].name;
        }
        break; 

      case ECOND_RANDOM:
        qualif = "random";
        break;

      case ECOND_CARRYING:
        qualif = "carrying";
        break;

      case ECOND_WEARING:
        qualif = "wearing";
        break;

      case ECOND_CARRYING_TYPE:
        qualif = "carrying_type";
        break;

      case ECOND_WEARING_TYPE:
        qualif = "wearing_type";
        break;

      case ECOND_CONTAINS:
        qualif = "contains";
        break;

      case ECOND_CONV:
      case ECOND_HOUR_OF_DAY_MOD:
      case ECOND_DAY_OF_MONTH_MOD:
      case ECOND_DAY_OF_YEAR_MOD:
      case ECOND_WEEK_OF_YEAR_MOD:
      case ECOND_MONTH_OF_YEAR_MOD:
      case ECOND_GADGET:
      case ECOND_NUMBER_MOD:
      case ECOND_VALUE:
      case ECOND_SOC_RANK:
      case ECOND_SOC_LEVEL:
      case ECOND_SOC_AUTH:
      case ECOND_MOB_IN_ROOM: 
      case ECOND_PLAYER_IN_ROOM: 
      case ECOND_ROOM_EMPTY_MOB: 
      case ECOND_OBJ_IN_ROOM: 
      case ECOND_OBJ_ROOM_CONTAINER: 
      case ECOND_ROOM_EMPTY_OBJ: 
      case ECOND_QUEST: 
      case ECOND_MEMORY: 
      case ECOND_MEMORYV: 
        sprintf(buf, "%d", ec->index);
        break;

      case ECOND_HOUR_OF_DAY:
      case ECOND_DAY_OF_MONTH:
      case ECOND_DAY_OF_YEAR:
      case ECOND_WEEK_OF_YEAR:
      case ECOND_MONTH_OF_YEAR:
        qualif = "calander";
        break;

      case ECOND_IS_ALONE: 
        qualif = "is_alone";
        break;

      case ECOND_USES_IMP: 
        qualif = "uses_imp";
        break;

      case ECOND_TYPE:
        qualif = "type";
        break;

      case ECOND_SUBTYPE:
        qualif = "subtype";
        break;

      case ECOND_NUMBER:
        qualif = "number";
        break;

      case ECOND_AM_ACTOR:
        qualif = "am_actor";
        break;

      case ECOND_AM_VICTIM:
        qualif = "am_victim";
        break;

      case ECOND_LOCAL:
        qualif = "local";
        break;

      case ECOND_ACTOR_GROUPED:
        qualif = "actor_grouped";
        break;

      case ECOND_VICTIM_GROUPED:
        qualif = "victim_grouped";
        break;

      case ECOND_ACTOR_REMEMBERED:
        qualif = "actor_remembered";
        break;

      case ECOND_VICTIM_REMEMBERED:
        qualif = "victim_remembered";
        break;

      case ECOND_VNUM:
        qualif = "vnum";
        break;

      case ECOND_SIZE:
        qualif = "size";
        break;

      case ECOND_IN_ROOM:
        qualif = "in_room";
        break;

      case ECOND_IN_SUBAREA:
        qualif = "in_subarea";
        break;

      case ECOND_DEED:
        qualif = "deed";
        break;

      case ECOND_IS:
        qualif = get_is_name(ec->index);
        break;

      case ECOND_IS_NOT:
        qualif = get_is_name(ec->index);
        break;

      case ECOND_ACTOR_FRIEND:
        qualif = "actor_friend";
        break;

      case ECOND_ACTOR_FOE:
        qualif = "actor_foe";
        break;

      case ECOND_VICTIM_FRIEND:
        qualif = "victim_friend";
        break;

      case ECOND_VICTIM_FOE:
        qualif = "victim_foe";
        break;

      case ECOND_ACTOR_VISIBLE:
        qualif = "actor_visible";
        break;

      case ECOND_VICTIM_VISIBLE:
        qualif = "victim_visible";
        break;

      default:
        break; 
    }

   /* Scribing time... */ 

    if (qualif != NULL) {
      fprintf( fp, "%s %d %d '%s' %d %d\n", hdr, ec->subject, ec->type, qualif, ec->low, ec->high );
    } 

   /* Get the next one to write out... */

    ec = ec->next;

  }

  return;
}


/* Print out an extra condition to a buffer... */

void print_econd_to_buffer(char *buffer, ECOND_DATA *ec, char *hdr, MOB_CMD_CONTEXT *mcc){
PROF_DATA *pd;
SOCIETY *society;
char buf[MAX_STRING_LENGTH];
char buf2[MAX_STRING_LENGTH];
int type;


  if (!ec) return;

 /* Prep the output buffer... */

  buf[0] = '\0';
  strcpy(buf, hdr);

  if (mcc) {
    if (ec_satisfied_mcc(mcc, ec, FALSE)) strcat(buf,"{g");
    else strcat(buf,"{y");
  }

 /* Subject... */

  switch (ec->subject) {
    
    case ECOND_SUBJECT_WORLD:
      strcat(buf, " world->");
      break;

    case ECOND_SUBJECT_ACTOR:
      strcat(buf, " actor->"); 
      break;

    case ECOND_SUBJECT_VICTIM:
      strcat(buf, " victim->"); 
      break;

    case ECOND_SUBJECT_OBSERVER:
      strcat(buf, " observer->"); 
      break;

    case ECOND_SUBJECT_RANDOM:
      strcat(buf, " randomc->"); 
      break;

    case ECOND_SUBJECT_L_ACTOR:
      strcat(buf, " actor->leader->"); 
      break;

    case ECOND_SUBJECT_L_VICTIM:
      strcat(buf, " victim->leader->"); 
      break;

    case ECOND_SUBJECT_L_OBSERVER:
      strcat(buf, " observer->leader->"); 
      break;

    case ECOND_SUBJECT_L_RANDOM:
      strcat(buf, " randomc->leader->"); 
      break;

    case ECOND_SUBJECT_P_OBJ:
      strcat(buf, " primary_object->"); 
      break;

    case ECOND_SUBJECT_S_OBJ:
      strcat(buf, " secondary_object->"); 
      break;

    case ECOND_SUBJECT_WEV:
      strcat(buf, " event->"); 
      break;

    default:
      break; 
  }

 /* Extract a not if there is one... */

  type = ec->type;

  if (type < 0) {
    strcat(buf, "not->");
    type *= -1;
  }

 /* Print type and qualifier... */

  switch(type) {

    case ECOND_NONE:
      strcat(buf, "none");
      break;

    case ECOND_SKILL:
      strcat(buf, "skill: '");
      if (ec->index != SKILL_UNDEFINED) {
        strcat(buf, skill_array[ec->index].name);
      } else {
        strcat(buf, "{Runknown skill{x");
      }
      strcat(buf, "'");
      break; 

    case ECOND_ATTRIBUTE:
      if (ec->index == STAT_STR) strcat(buf, "Strength");
      else if (ec->index == STAT_INT) strcat(buf, "Intelligence");
      else if (ec->index == STAT_WIS) strcat(buf, "Wisdom");
      else if (ec->index == STAT_DEX) strcat(buf, "Dexterity");
      else if (ec->index == STAT_CON) strcat(buf, "Constitution");
      break; 

    case ECOND_NAME:
      strcat(buf, "Name: '");
      if ( ec->text == NULL ) {
        strcat(buf, "{R(null){x");
      } else {
        strcat(buf, ec->text);
      }  
      strcat(buf, "'");
      break;

    case ECOND_OWNER:
      strcat(buf, "Owner: '");
      if ( ec->text == NULL ) {
        strcat(buf, "{R(null){x");
      } else {
        strcat(buf, ec->text);
      }  
      strcat(buf, "'");
      break;

    case ECOND_PARTNER:
      strcat(buf, "Partner: '");
      if ( ec->text == NULL ) {
        strcat(buf, "{R(null){x");
      } else {
        strcat(buf, ec->text);
      }  
      strcat(buf, "'");
      break;

    case ECOND_SHORT:
      strcat(buf, "Short: '");
      if ( ec->text == NULL ) {
        strcat(buf, "{R(null){x");
      } else {
        strcat(buf, ec->text);
      }  
      strcat(buf, "'");
      break;

    case ECOND_CULT:
      strcat(buf, "Cult: '");
      if ( ec->text == NULL ) {
        strcat(buf, "{R(null){x");
      } else {
        strcat(buf, ec->text);
      }  
      strcat(buf, "'");
      break;

    case ECOND_KNOWS_DISCIPLINE:
      strcat(buf, "Discipline: '");
      if ( ec->text == NULL ) {
        strcat(buf, "{R(null){x");
      } else {
        strcat(buf, ec->text);
      }  
      strcat(buf, "'");
      break;

    case ECOND_ALIGN:
      strcat(buf, "alignment");
      break;

    case ECOND_LIGHT:
      strcat(buf, "light");
      break;

    case ECOND_LEVEL:
      strcat(buf, "level");
      break;

    case ECOND_DIVINITY:
      strcat(buf, "divinity");
      break;

    case ECOND_CRIMINAL_RATING:
      strcat(buf, "criminal_rating");
      break;

    case ECOND_HITS:
      strcat(buf, "hits");
      break;

    case ECOND_MANA:
      strcat(buf, "mana");
      break;

    case ECOND_MOVE:
      strcat(buf, "move");
      break;

    case ECOND_HITS_PERCENT:
      strcat(buf, "hits_percent");
      break;

    case ECOND_MANA_PERCENT:
      strcat(buf, "mana_percent");
      break;

    case ECOND_MOVE_PERCENT:
      strcat(buf, "move_percent");
      break;

    case ECOND_GOLD:
      strcat(buf, "gold");
      break;

    case ECOND_PRACTICE:
      strcat(buf, "prac");
      break;

    case ECOND_TRAIN:
      strcat(buf, "train");
      break;

    case ECOND_FAME:
      strcat(buf, "fame");
      break;

    case ECOND_START_ROOM:
      strcat(buf, "start");
      break;

    case ECOND_SANITY:
      strcat(buf, "sanity");
      break;

    case ECOND_CASTING_LEVEL:
      strcat(buf, "casting_level");
      break;

    case ECOND_RACE:
      strcat(buf, "race: '");
      strcat(buf, race_array[ec->index].name);  
      strcat(buf, "'");
      break;

    case ECOND_MOON:
      strcat(buf, "moon");
      break;

    case ECOND_SEX:
      strcat(buf, "gender");
      break;

    case ECOND_PROF:
      strcat(buf, "profession: '");
      pd = get_prof_by_id(ec->index);
      if (pd != NULL) {
        strcat(buf, pd->name);
      } else {
        strcat(buf, "{Runknown profession{x");
      }
      strcat(buf, "'");
      break;

    case ECOND_SOC_RANK:
      sprintf(buf2, "soc_rank: %d (", ec->index);
      strcat(buf, buf2);
      society = find_society_by_id(ec->index);
      if (society != NULL) {
        strcat(buf, society->name);
      } else {
        strcat(buf, "{Runknown society{x");
      }
      strcat(buf, ")");
      break;

    case ECOND_SOC_LEVEL:
      sprintf(buf2, "soc_level: %d (", ec->index);
      strcat(buf, buf2);
      society = find_society_by_id(ec->index);
      if (society != NULL) {
        strcat(buf, society->name);
      } else {
        strcat(buf, "{Runknown society{x");
      }
      strcat(buf, ")");
      break;

    case ECOND_SOC_AUTH:
      sprintf(buf2, "soc_auth: %d (", ec->index);
      strcat(buf, buf2);
      society = find_society_by_id(ec->index);
      if (society != NULL) {
        strcat(buf, society->name);
      } else {
        strcat(buf, "{Runknown society{x");
      }
      strcat(buf, ")");
      break;

    case ECOND_AFFECTED:
      strcat(buf, "affected: '");
      if (ec->index != SKILL_UNDEFINED) {
        strcat(buf, affect_array[ec->index].name);
      } else {
        strcat(buf, "{Runknown affect{x");
      }
      strcat(buf, "'");
      break; 

    case ECOND_CONV:
      sprintf(buf2, "conversation %d", ec->index);
      strcat(buf, buf2);
      break;

    case ECOND_RANDOM:
      strcat(buf, "random");
      break;

    case ECOND_CARRYING:
      strcat(buf, "carrying");
      break;

    case ECOND_WEARING:
      strcat(buf, "wearing");
      break;

    case ECOND_CARRYING_TYPE:
      strcat(buf, "carrying_type");
      break;

    case ECOND_WEARING_TYPE:
      strcat(buf, "wearing_type");
      break;

    case ECOND_HOUR_OF_DAY:
      strcat(buf, "hour_of_day");
      break;

    case ECOND_HOUR_OF_DAY_MOD:
      sprintf(buf2, "hour_of_day_mod %d", ec->index);
      strcat(buf, buf2);
      break; 

    case ECOND_DAY_OF_MONTH:
      strcat(buf, "day_of_month");
      break;

    case ECOND_DAY_OF_MONTH_MOD:
      sprintf(buf2, "day_of_month_mod %d", ec->index);
      strcat(buf, buf2);
      break; 

    case ECOND_DAY_OF_YEAR:
      strcat(buf, "day_of_year");
      break;

    case ECOND_DAY_OF_YEAR_MOD:
      sprintf(buf2, "day_of_year_mod %d", ec->index);
      strcat(buf, buf2);
      break; 

    case ECOND_WEEK_OF_YEAR:
      strcat(buf, "week_of_year");
      break;

    case ECOND_WEEK_OF_YEAR_MOD:
      sprintf(buf2, "week_of_year_mod %d", ec->index);
      strcat(buf, buf2);
      break; 

    case ECOND_MONTH_OF_YEAR:
      strcat(buf, "month_of_year");
      break;

    case ECOND_MONTH_OF_YEAR_MOD:
      sprintf(buf2, "month_of_year_mod %d", ec->index);
      strcat(buf, buf2);
      break; 

    case ECOND_GADGET:
      sprintf(buf2, "room %d->gadget", ec->index);
      strcat(buf, buf2);
      break; 

    case ECOND_MOB_IN_ROOM: 
      sprintf(buf2, "mob_in_room %d", ec->index);
      strcat(buf, buf2);
      break; 

    case ECOND_PLAYER_IN_ROOM: 
      sprintf(buf2, "player_in_room %d", ec->index);
      strcat(buf, buf2);
      break; 

    case ECOND_ROOM_EMPTY_MOB: 
      sprintf(buf2, "room %d->empty_mob", ec->index);
      strcat(buf, buf2);
      break; 

    case ECOND_OBJ_IN_ROOM: 
      sprintf(buf2, "obj_in_room %d", ec->index);
      strcat(buf, buf2);
      break; 

    case ECOND_OBJ_ROOM_CONTAINER: 
      sprintf(buf2, "obj_room %d in container %d", ec->index, ec->low);
      strcat(buf, buf2);
      break; 

    case ECOND_ROOM_EMPTY_OBJ: 
      sprintf(buf2, "room %d->empty_obj", ec->index);
      strcat(buf, buf2);
      break; 

    case ECOND_TYPE:
      strcat(buf, "type");
      break;

    case ECOND_SUBTYPE:
      strcat(buf, "subtype");
      break;

    case ECOND_NUMBER:
      strcat(buf, "number");
      break;

    case ECOND_NUMBER_MOD:
      sprintf(buf2, "number_mod %d", ec->index);
      strcat(buf, buf2);
      break; 

    case ECOND_AM_ACTOR:
      strcat(buf, "am_actor");
      break;

    case ECOND_AM_VICTIM:
      strcat(buf, "am_victim");
      break;

    case ECOND_LOCAL:
      strcat(buf, "local");
      break;

    case ECOND_ACTOR_GROUPED:
      strcat(buf, "actor_grouped");
      break;

    case ECOND_VICTIM_GROUPED:
      strcat(buf, "victim_grouped");
      break;

    case ECOND_ACTOR_REMEMBERED:
      strcat(buf, "actor_remembered");
      break;

    case ECOND_VICTIM_REMEMBERED:
      strcat(buf, "victim_remembered");
      break;

    case ECOND_VNUM:
      strcat(buf, "vnum");
      break;

    case ECOND_SIZE:
      strcat(buf, "size");
      break;

    case ECOND_IN_ROOM:
      strcat(buf, "in_room");
      break;

    case ECOND_IN_SUBAREA:
      strcat(buf, "in_subarea");
      break;

    case ECOND_DEED:
      strcat(buf, "deed");
      break;

    case ECOND_QUEST:
      sprintf(buf2, "quest %d", ec->index);
      strcat(buf, buf2);
      break; 

    case ECOND_MEMORY:
      sprintf(buf2, "memory[%d] not null", ec->index);
      strcat(buf, buf2);
      break; 

    case ECOND_MEMORYV:
      sprintf(buf2, "memory[%d] value", ec->index);
      strcat(buf, buf2);
      break; 

    case ECOND_IS_ALONE:
      strcat(buf, "is_alone");
      break;

    case ECOND_USES_IMP:
      strcat(buf, "uses_imp");
      break;

    case ECOND_IS:
      strcat(buf, "is");
      break;

    case ECOND_IS_NOT:
      strcat(buf, "is_not");
      break;

    case ECOND_VALUE:
      sprintf(buf2, "value %d", ec->index);
      strcat(buf, buf2);
      break; 

    case ECOND_CONTAINS:
      strcat(buf, "contains");
      break;

    case ECOND_ACTOR_FRIEND:
      strcat(buf, "actor_friend");
      break;

    case ECOND_ACTOR_FOE:
      strcat(buf, "actor_foe");
      break;

    case ECOND_VICTIM_FRIEND:
      strcat(buf, "victim_friend");
      break;

    case ECOND_VICTIM_FOE:
      strcat(buf, "victim_foe");
      break;

    case ECOND_ACTOR_VISIBLE:
      strcat(buf, "actor_visible");
      break;

    case ECOND_VICTIM_VISIBLE:
      strcat(buf, "victim_visible");
      break;

    default:
      strcat(buf, "{Rcorrupted{x");
      break;
  }

 /* Add the low and high value... */

  switch (type) {

    default:
      sprintf(buf2, "%s %d %d\r\n", buf,  ec->low,  ec->high);
      strcat(buffer, buf2);
      break;

    case ECOND_ACTOR_REMEMBERED: 
    case ECOND_VICTIM_REMEMBERED: 
      sprintf(buf2, "%s %d\r\n", buf, ec->low);
      strcat(buffer, buf2);
      break;

    case ECOND_OBJ_ROOM_CONTAINER:
      sprintf(buf2, "%s %d\r\n", buf, ec->high);
      strcat(buffer, buf2);
      break;

    case ECOND_AFFECTED:
    case ECOND_IS_ALONE:
    case ECOND_USES_IMP:
    case ECOND_AM_ACTOR:
    case ECOND_AM_VICTIM:
    case ECOND_LOCAL:
    case ECOND_ACTOR_GROUPED:
    case ECOND_VICTIM_GROUPED:
    case ECOND_PLAYER_IN_ROOM:
    case ECOND_ROOM_EMPTY_MOB:
    case ECOND_ROOM_EMPTY_OBJ:
    case ECOND_MEMORY:
    case ECOND_ACTOR_FRIEND:
    case ECOND_ACTOR_FOE:
    case ECOND_VICTIM_FRIEND:
    case ECOND_VICTIM_FOE:
    case ECOND_ACTOR_VISIBLE:
    case ECOND_VICTIM_VISIBLE:
    case ECOND_NAME:
    case ECOND_OWNER:
    case ECOND_PARTNER:
    case ECOND_CULT:
    case ECOND_KNOWS_DISCIPLINE:
    case ECOND_SHORT:
      strcat(buffer, buf);
      strcat(buffer, "\r\n");
      break;

    case ECOND_DEED:
      if (ec->high == 1) {
        sprintf(buf2, "%s id: %d done (1)\r\n", buf, ec->low);
      } else {
        sprintf(buf2, "%s id: %d undone (0)\r\n", buf, ec->low);
      }
      strcat(buffer, buf2);
      break;

    case ECOND_IS:
    case ECOND_IS_NOT:
      strcat(buffer, buf);
      sprintf(buf2, " %s\r\n", get_is_name(ec->index));
      strcat(buffer, buf2);
      break;

    case ECOND_SEX:
      switch (ec->low) {
        case SEX_MALE:
          strcat(buf, " male (1)");
          break;
        case SEX_FEMALE:
          strcat(buf, " female (2)");
          break;
        case SEX_NEUTRAL:
          strcat(buf, " neuter (0)");
          break;
        default:
          break;
      }  
 
      switch (ec->high) {
        case SEX_MALE:
          strcat(buf, " male (1)");
          break;
        case SEX_FEMALE:
          strcat(buf, " female (2)");
          break;
        case SEX_NEUTRAL:
          strcat(buf, " neuter (0)");
          break;
        default:
          break;
      }  

      strcat(buf, "\r\n"); 
      strcat(buffer, buf);
      break;
  }

  if (mcc) strcat(buffer, "{x");  

 /* Print the next one, if there is one... */

  if (ec->next) print_econd_to_buffer(buffer, ec->next, hdr, mcc);
  return;
}


/* Print out an extra condition... */

void print_econd(CHAR_DATA *ch, ECOND_DATA *ec, char *hdr) {
char buffer[MAX_STRING_LENGTH];

 /* Safty check... */
  if (ec == NULL) return;

 /* Go print... */

  buffer[0] = '\0';

  print_econd_to_buffer(buffer, ec, hdr, NULL);

 /* Send to char */
  send_to_char(buffer, ch);
  return;
}

/* Create an econd from scratch... */

ECOND_DATA *create_econd(char *keyword, CHAR_DATA *ch) {

  ECOND_DATA *new_ec;

  SOCIETY *society;

  char ctype[MAX_STRING_LENGTH];
  char cname[MAX_STRING_LENGTH];
  char clow[MAX_STRING_LENGTH];
  char chi[MAX_STRING_LENGTH];

  int subject;

  int enot;

 /* keyword must be supplied... */

  if ( keyword[0] == '\0' || keyword[0] == '?') {
    sprintf_to_char(ch,
      "Syntax: <subject> <not> condition parms\r\n"
      "{c subject...{x\r\n"
      "  actor, victim, observer, randomc\r\n"
      "  lactor, lvictim, lobserver, lrandomc\r\n"
      "  world pobj sobj event\r\n"
      "  or omitted\r\n"
      "{c not{x\r\n"
      "{c condition...{x\r\n"
      "  none (this removes ALL conditions)\r\n"
      "  Fpr a list of conds, see out page at:\r\n"
      "  %s/help/\r\n", mud.url);
    return NULL;
  }

 /* Work out what we're doing here... */

  keyword = one_argument(keyword, ctype);
 
 /* Do we need to pull a subject off? */

  subject = ECOND_SUBJECT_WORLD;

  if (!strcmp(ctype, "world")) {
    subject = ECOND_SUBJECT_WORLD;
    keyword = one_argument(keyword, ctype);
  } else if (!strcmp(ctype, "actor")) {
    subject = ECOND_SUBJECT_ACTOR;
    keyword = one_argument(keyword, ctype);
  } else if (!strcmp(ctype, "victim")) {
    subject = ECOND_SUBJECT_VICTIM;
    keyword = one_argument(keyword, ctype);
  } else if (!strcmp(ctype, "observer")) {
    subject = ECOND_SUBJECT_OBSERVER;
    keyword = one_argument(keyword, ctype);
  } else if (!strcmp(ctype, "randomc")) {
    subject = ECOND_SUBJECT_RANDOM;
    keyword = one_argument(keyword, ctype);
  } else if (!strcmp(ctype, "lactor")) {
    subject = ECOND_SUBJECT_L_ACTOR;
    keyword = one_argument(keyword, ctype);
  } else if (!strcmp(ctype, "lvictim")) {
    subject = ECOND_SUBJECT_L_VICTIM;
    keyword = one_argument(keyword, ctype);
  } else if (!strcmp(ctype, "lobserver")) {
    subject = ECOND_SUBJECT_L_OBSERVER;
    keyword = one_argument(keyword, ctype);
  } else if (!strcmp(ctype, "lrandomc")) {
    subject = ECOND_SUBJECT_L_RANDOM;
    keyword = one_argument(keyword, ctype);
  } else if (!strcmp(ctype, "pobj")) {
    subject = ECOND_SUBJECT_P_OBJ;
    keyword = one_argument(keyword, ctype);
  } else if (!strcmp(ctype, "sobj")) {
    subject = ECOND_SUBJECT_S_OBJ;
    keyword = one_argument(keyword, ctype);
  } else if (!strcmp(ctype, "event")) {
    subject = ECOND_SUBJECT_WEV;
    keyword = one_argument(keyword, ctype);
  }

 /* not */

  if (!strcmp(ctype, "not")) {
    enot = -1;
    keyword = one_argument(keyword, ctype);
  } else {
    enot = 1;
  } 

 /* none */

  if (!strcmp(ctype, "none")) {

    if (keyword[0] != '\0') {
      send_to_char("Condition: none\r\n", ch);
      return NULL;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_NONE * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;
    new_ec->index = 0;
    new_ec->low = 0;
    new_ec->high = 0;

    send_to_char("None condition created!\r\n", ch); 

    return new_ec; 
  }

 /* level low hi */

  if (!strcmp(ctype, "level")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: level [low] [hi]\r\n", ch);
      send_to_char("Values: character levels (0..100+)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_LEVEL * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Level condition set.\r\n", ch); 
 
    return new_ec; 
  }


  if (!strcmp(ctype, "divinity")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: divinity [low] [high]\r\n", ch);
      send_to_char("Values: divinity levels (0..9)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_DIVINITY * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Divinity condition set.\r\n", ch); 
    return new_ec; 
  }


  if (!strcmp(ctype, "criminal_rating")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: criminal_rating [low] [hi]\r\n", ch);
      send_to_char("Values: character rating (>= 0)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_CRIMINAL_RATING * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Criminal rating condition set.\r\n", ch); 
     return new_ec; 
  }


 /* hits low hi */

  if (!strcmp(ctype, "hits")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: hits [low] [hi]\r\n", ch);
      send_to_char("Values: hit points (0..1000+)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_HITS * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Hits condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* mana low hi */

  if (!strcmp(ctype, "mana")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: mana [low] [hi]\r\n", ch);
      send_to_char("Values: mana points (0..1000+)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_MANA * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Mana condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* move low hi */

  if (!strcmp(ctype, "move")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: move [low] [hi]\r\n", ch);
      send_to_char("Values: move points (0..1000+)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_MANA * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Move condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* hits_percent low hi */

  if (!strcmp(ctype, "hits_percent")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: hits_percent [low] [hi]\r\n", ch);
      send_to_char("Values: percent hit points (0..100)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_HITS_PERCENT * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Hits_percent condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* mana_percent low hi */

  if (!strcmp(ctype, "mana_percent")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: mana_percent [low] [hi]\r\n", ch);
      send_to_char("Values: percent mana points (0..100)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_MANA_PERCENT * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Mana_percent condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* move_percent low hi */

  if (!strcmp(ctype, "move_percent")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: move_percent [low] [hi]\r\n", ch);
      send_to_char("Values: percent move points (0..100)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_MOVE_PERCENT * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Move_percent condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* gold low hi */

  if (!strcmp(ctype, "gold")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: gold [low] [hi]\r\n", ch);
      send_to_char("Values: character gold (0..37000)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_GOLD * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Gold condition set.\r\n", ch); 
 
    return new_ec; 
  }

  if (!strcmp(ctype, "prac")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: prac [low] [hi]\r\n", ch);
      send_to_char("Values: character prac (0..999)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_PRACTICE * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Practice condition set.\r\n", ch); 
 
    return new_ec; 
  }

  if (!strcmp(ctype, "start")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: start [low] [hi]\r\n", ch);
      send_to_char("Values: character start (0..37000)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_START_ROOM * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Start Room condition set.\r\n", ch); 
 
    return new_ec; 
  }

  if (!strcmp(ctype, "train")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: train [low] [hi]\r\n", ch);
      send_to_char("Values: character train (0..999)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_TRAIN * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Train condition set.\r\n", ch); 
 
    return new_ec; 
  }

  if (!strcmp(ctype, "fame")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: fame [low] [hi]\r\n", ch);
      send_to_char("Values: character fame (0..37000)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_FAME * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Fame condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* sanity low hi */

  if (!strcmp(ctype, "sanity")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: sanity [low] [hi]\r\n", ch);
      send_to_char("Values: character sanity (0..100+)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_SANITY * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Sanity condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* casting_level low hi */

  if (!strcmp(ctype, "casting_level")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: casting_level [low] [hi]\r\n", ch);
      send_to_char("Values: character levels (0..100+)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_CASTING_LEVEL * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Casting level condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* align low hi */

  if (!strcmp(ctype, "align")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: align [low] [hi]\r\n", ch);
      send_to_char("Values: alignement values (-1000..1000)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_ALIGN * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = -500;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = 500;
    }

    send_to_char("Alignment condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* gender mfn mfn */

  if (!strcmp(ctype, "gender")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: gender [mfn] [mfn]\r\n", ch);
      send_to_char("Values: 0 neuter, 1 male, 2 female\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_SEX * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = -1;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Gender condition set.\r\n", ch); 

    return new_ec; 
  }

 /* moon low hi */

  if (!strcmp(ctype, "moon")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: moon [low] [hi]\r\n", ch);
      send_to_char("Values: phases of the moon (0..4)\r\n", ch);
      send_to_char("        (0 is new, 2 is half, 4 is full)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    new_ec = get_ec();

    new_ec->type = ECOND_MOON * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = 4;
    }

    send_to_char("Moon condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* light low hi */

  if (!strcmp(ctype, "light")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Conditions: light [low] [hi]\r\n", ch);
      send_to_char("Values: Light level (0..100)\r\n", ch);
      send_to_char("          0 - Absolute Darkness\r\n", ch);
      send_to_char("         10 - Dark Underground\r\n", ch);
      send_to_char("         20 - Dark indoors\r\n", ch);
      send_to_char("         30 - Dark, moonless night\r\n", ch);
      send_to_char("         40 - Dark, new moon\r\n", ch);
      send_to_char("         50 - Dim, half moon\r\n", ch);
      send_to_char("         60 - Dim, full moon\r\n", ch);
      send_to_char("         65 - Dim, dawn\r\n", ch);
      send_to_char("         70 - Dim, dusk, new moon\r\n", ch);
      send_to_char("         80 - Light, artificial\r\n", ch);
      send_to_char("         85 - Light, magical\r\n", ch);
      send_to_char("         90 - Light, daylight\r\n", ch);
      send_to_char("        100 - Light, sunlight\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    new_ec = get_ec();

    new_ec->type = ECOND_LIGHT * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = 100;
    }

    send_to_char("Light condition set.\r\n", ch); 

    return new_ec; 
  }

 /* race name */

  if (!strcmp(ctype, "race")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: race [name]\r\n", ch);
      send_to_char("Value: name of a race\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_RACE * enot;
    new_ec->subject = subject;

    new_ec->index = race_lookup(cname);

    new_ec->low = 0;

    new_ec->high = -1;

    send_to_char("Race condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* skill name low hi */

  if (!strcmp(ctype, "skill")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Conditions: skill [name] [low] [hi]\r\n", ch);
      send_to_char("Value: name of a skill, skill rating (0..100+)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);
    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_SKILL * enot;
    new_ec->subject = subject;

    new_ec->index = get_skill_sn(cname);

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Skill condition set.\r\n", ch); 

    return new_ec; 
  }


  if (!strcmp(ctype, "attribute")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Conditions: attribute [name] [low] [hi]\r\n", ch);
      send_to_char("Value: name of an attribute, attribute rating (0..112)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);
    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_ATTRIBUTE * enot;
    new_ec->subject = subject;

    if (!str_cmp(cname, "strength") || !str_cmp(cname, "str")) new_ec->index = STAT_STR;
    else if (!str_cmp(cname, "intelligence") || !str_cmp(cname, "int")) new_ec->index = STAT_INT;
    else if (!str_cmp(cname, "wisdom") || !str_cmp(cname, "wis")) new_ec->index = STAT_WIS;
    else if (!str_cmp(cname, "dexterity") || !str_cmp(cname, "dex")) new_ec->index = STAT_DEX;
    else if (!str_cmp(cname, "constitution") || !str_cmp(cname, "con")) new_ec->index = STAT_INT;
    else {
      send_to_char("Conditions: attribute [name] [low] [hi]\r\n", ch);
      send_to_char("Value: name of an attribute, attribute rating (0..112)\r\n", ch);
      return NULL;
    }
        
    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Attribute condition set.\r\n", ch); 
    return new_ec; 
  }


 /* name name */

  if (!strcmp(ctype, "name")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Conditions: name [name]\r\n", ch);
      send_to_char("Value: name of a mob or object\r\n", ch);
      return NULL;
    }

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_NAME * enot;
    new_ec->subject = subject;

    new_ec->text = str_dup(keyword);

    new_ec->index = 0;
    new_ec->low = 0;
    new_ec->high = 0;

    send_to_char("Name condition set.\r\n", ch); 

    return new_ec; 
  }


  if (!strcmp(ctype, "owner")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Conditions:  owner [name]\r\n", ch);
      send_to_char("Value: Owner of an object\r\n", ch);
      return NULL;
    }

    if (subject != ECOND_SUBJECT_S_OBJ) {
      subject = ECOND_SUBJECT_P_OBJ;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_OWNER * enot;
    new_ec->subject = subject;

    new_ec->text = str_dup(keyword);

    new_ec->index = 0;
    new_ec->low = 0;
    new_ec->high = 0;

    send_to_char("Owner condition set.\r\n", ch); 
    return new_ec; 
  }


  if (!strcmp(ctype, "short")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Conditions: short [name]\r\n", ch);
      send_to_char("Value: short of a mob\r\n", ch);
      return NULL;
    }

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_SHORT * enot;
    new_ec->subject = subject;

    new_ec->text = str_dup(keyword);

    new_ec->index = 0;
    new_ec->low = 0;
    new_ec->high = 0;

    send_to_char("Short condition set.\r\n", ch); 

    return new_ec; 
  }

  if (!strcmp(ctype, "cult")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Conditions: cult [name]\r\n", ch);
      send_to_char("Value: cult of a player\r\n", ch);
      return NULL;
    }

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_CULT * enot;
    new_ec->subject = subject;

    new_ec->text = str_dup(keyword);

    new_ec->index = 0;
    new_ec->low = 0;
    new_ec->high = 0;

    send_to_char("Cult condition set.\r\n", ch); 

    return new_ec; 
  }


  if (!strcmp(ctype, "discipline")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Conditions: discipline [name]\r\n", ch);
      send_to_char("Value: Discipline of a player\r\n", ch);
      return NULL;
    }

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_KNOWS_DISCIPLINE * enot;
    new_ec->subject = subject;

    new_ec->text = str_dup(keyword);

    new_ec->index = 0;
    new_ec->low = 0;
    new_ec->high = 0;

    send_to_char("Discipline condition set.\r\n", ch); 
    return new_ec; 
  }


 /* prof name lo hi */

  if (!strcmp(ctype, "prof")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: prof [name] [low] [hi]\r\n", ch);
      send_to_char("Value: name of a profession, prof level (0..100+)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);
    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_PROF * enot;
    new_ec->subject = subject;

    new_ec->index = get_prof_id(cname);

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Prof condition set.\r\n", ch); 

    return new_ec; 
  }

 /* affected name */

  if (!strcmp(ctype, "affected")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Conditions: affected [affect]\r\n", ch);
      send_to_char("Affect: name of an affect\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_AFFECTED * enot;
    new_ec->subject = subject;

    new_ec->index = get_affect_afn(cname);

    new_ec->low = 0;
    new_ec->high = -1;

    send_to_char("Affect condition set.\r\n", ch); 

    return new_ec; 
  }

 /* soc_rank sid template */

  if (!strcmp(ctype, "soc_rank")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: soc_rank [soc_id] [template]\r\n", ch);
      send_to_char("Values: 1) Invited, 2) Member, 4) Council, 8) Leader\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);
    keyword = one_argument(keyword, clow);

    new_ec = get_ec();

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec->type = ECOND_SOC_RANK * enot;
    new_ec->subject = subject;

    new_ec->index = atoi(cname);

    if (new_ec->index <= 0) {
      send_to_char("Society id must be > 0!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    society = find_society_by_id(new_ec->index);

    if (society == NULL) {
      send_to_char("That is not a valid society id!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    new_ec->high = 0;

    send_to_char("soc_rank condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* soc_level sid low hi */

  if (!strcmp(ctype, "soc_level")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: soc_level [soc_id] [low] [high]\r\n", ch);
      send_to_char("Values: Level within society\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);
    keyword = one_argument(keyword, clow);

    new_ec = get_ec();

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec->type = ECOND_SOC_AUTH * enot;
    new_ec->subject = subject;

    new_ec->index = atoi(cname);

    if (new_ec->index <= 0) {
      send_to_char("Society id must be > 0!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    society = find_society_by_id(new_ec->index);

    if (society == NULL) {
      send_to_char("That is not a valid society id!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (clow[0] != '\0') {
      new_ec->high = atoi(clow);
    } else {
      new_ec->high = -1;
    }

    send_to_char("soc_level condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* soc_auth sid template */

  if (!strcmp(ctype, "soc_auth")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: soc_auth [soc_id] [template]\r\n", ch);
      send_to_char("Values: Society authority flags (see society.h)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);
    keyword = one_argument(keyword, clow);

    new_ec = get_ec();

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec->type = ECOND_SOC_AUTH * enot;
    new_ec->subject = subject;

    new_ec->index = atoi(cname);

    if (new_ec->index <= 0) {
      send_to_char("Society id must be > 0!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    society = find_society_by_id(new_ec->index);

    if (society == NULL) {
      send_to_char("That is not a valid society id!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    new_ec->high = 0;

    send_to_char("soc_auth condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* conv conv_id sub_id state */

  if (!strcmp(ctype, "conv")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Conditions: conv [conv_id] [sub_id] [state]\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);
    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_CONV * enot;
    new_ec->subject = subject;

    new_ec->index = atoi(cname);

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = 0;
    }

    send_to_char("Conversation condition set.\r\n", ch); 

    return new_ec; 
  }

 /* random low hi */

  if (!strcmp(ctype, "random")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: random [low] [hi]\r\n", ch);
      send_to_char("Values: numbers from 0 to 1023)\r\n", ch);
      send_to_char("Note: Random conds are checked independently\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    new_ec = get_ec();

    new_ec->type = ECOND_RANDOM * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = 1023;
    }

    send_to_char("Random condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* carrying low hi */

  if (!strcmp(ctype, "carrying")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: carrying [vnum1] [vnum2]\r\n", ch);
      send_to_char("Values: vnums of item in inventory\r\n", ch);
      send_to_char("Note: Satisfied if either item is carried.\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_CARRYING * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = 0;
    }

    send_to_char("Carrying condition set.\r\n", ch); 
    return new_ec; 
  }


  if (!strcmp(ctype, "carrying_type")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: carrying_type [type1] [type2]\r\n", ch);
      send_to_char("Values: Item type names\r\n", ch);
      send_to_char("Note: Satisfied if either item is carried.\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_CARRYING_TYPE * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = flag_value(type_flags, clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = flag_value(type_flags, chi);
    } else {
      new_ec->high = 0;
    }

    send_to_char("Carrying_type condition set.\r\n", ch); 
     return new_ec; 
  }


 /* wearing low hi */

  if (!strcmp(ctype, "wearing")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: wearing [vnum1] [vnum2]\r\n", ch);
      send_to_char("Values: vnums of item in worn or held\r\n", ch);
      send_to_char("Note: Satisfied if either item is worn or held.\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_WEARING * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = 0;
    }

    send_to_char("Wearing condition set.\r\n", ch); 
    return new_ec; 
  }


  if (!strcmp(ctype, "wearing_type")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: wearing_type [type1] [type2]\r\n", ch);
      send_to_char("Values: item type names in worn or held\r\n", ch);
      send_to_char("Note: Satisfied if either item is worn or held.\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_WEARING_TYPE * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = flag_value(type_flags, clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = flag_value(type_flags, chi);
    } else {
      new_ec->high = 0;
    }

    send_to_char("Wearing_type condition set.\r\n", ch); 
    return new_ec; 
  }


 /* hour low hi */

  if (!strcmp(ctype, "hour_of_day")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: hour_of_day [low] [hi]\r\n", ch);
      send_to_char("Values: hours of the day (0..23)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    new_ec = get_ec();

    new_ec->type = ECOND_HOUR_OF_DAY * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = 23;
    }

    send_to_char("Hour_of_day condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* hour_mod mod low hi */

  if (!strcmp(ctype, "hour_of_day_mod")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: hour_of_day_mod [mod] [low] [hi]\r\n", ch);
      send_to_char("Values: hours of the day (0..23)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);
    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    new_ec = get_ec();

    new_ec->type = ECOND_HOUR_OF_DAY_MOD * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;

    new_ec->index = atoi(cname);

    if (new_ec->index <= 1) {
      send_to_char("Mod value must be > 1!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = (new_ec->index - 1);
    }

    send_to_char("Hour_of_day_mod condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* day_of_month low hi */

  if (!strcmp(ctype, "day_of_month")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: day_of_month [low] [hi]\r\n", ch);
      send_to_char("Values: days of the month (0..31)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    new_ec = get_ec();

    new_ec->type = ECOND_DAY_OF_MONTH * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = 29;
    }

    send_to_char("Day_of_month condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* day_of_month_mod mod low hi */

  if (!strcmp(ctype, "day_of_month_mod")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: day_of_month_mod [mod] [low] [hi]\r\n", ch);
      send_to_char("Values: days of the month (0..31)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);
    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    new_ec = get_ec();

    new_ec->type = ECOND_DAY_OF_MONTH_MOD * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;

    new_ec->index = atoi(cname);

    if (new_ec->index <= 1) {
      send_to_char("Mod value must be > 1!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = (new_ec->index - 1);
    }

    send_to_char("Day_of_month_mod condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* day_of_year low hi */

  if (!strcmp(ctype, "day_of_year")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: day_of_year [low] [hi]\r\n", ch);
      send_to_char("Values: days of the year (0..383)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    new_ec = get_ec();

    new_ec->type = ECOND_DAY_OF_YEAR * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = 29;
    }

    send_to_char("Day_of_year condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* day_of_year_mod mod low hi */

  if (!strcmp(ctype, "day_of_year_mod")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: day_of_year_mod [mod] [low] [hi]\r\n", ch);
      send_to_char("Values: days of the year (0..383)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);
    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    new_ec = get_ec();

    new_ec->type = ECOND_DAY_OF_YEAR_MOD * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;

    new_ec->index = atoi(cname);

    if (new_ec->index <= 1) {
      send_to_char("Mod value must be > 1!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = (new_ec->index - 1);
    }

    send_to_char("Day_of_year_mod condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* week_of_year low hi */

  if (!strcmp(ctype, "week_of_year")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: week_of_year [low] [hi]\r\n", ch);
      send_to_char("Values: week of the year (0..47)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    new_ec = get_ec();

    new_ec->type = ECOND_WEEK_OF_YEAR * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = 11;
    }

    send_to_char("Week_of_year condition set.\r\n", ch); 

    return new_ec; 
  }

 /* week_of_year_mod mod low hi */

  if (!strcmp(ctype, "week_of_year_mod")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: week_of_year_mod [mod] [low] [hi]\r\n", ch);
      send_to_char("Values: weeks of the year (0..47)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);
    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    new_ec = get_ec();

    new_ec->type = ECOND_WEEK_OF_YEAR_MOD * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;

    new_ec->index = atoi(cname);

    if (new_ec->index <= 1) {
      send_to_char("Mod value must be > 1!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = (new_ec->index - 1);
    }

    send_to_char("Week_of_year_mod condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* month_of_year low hi */

  if (!strcmp(ctype, "month_of_year")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: month_of_year [low] [hi]\r\n", ch);
      send_to_char("Values: months of the year (0..11)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    new_ec = get_ec();

    new_ec->type = ECOND_MONTH_OF_YEAR * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = 11;
    }

    send_to_char("Month_of_year condition set.\r\n", ch); 

    return new_ec; 
  }

 /* month_of_year_mod mod low hi */

  if (!strcmp(ctype, "month_of_year_mod")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: month_of_year_mod [mod] [low] [hi]\r\n", ch);
      send_to_char("Values: months of the year (0..11)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);
    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    new_ec = get_ec();

    new_ec->type = ECOND_MONTH_OF_YEAR_MOD * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;

    new_ec->index = atoi(cname);

    if (new_ec->index <= 1) {
      send_to_char("Mod value must be > 1!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = (new_ec->index - 1);
    }

    send_to_char("Month_of_year_mod condition set.\r\n", ch); 
 
    return new_ec; 
  }


  if (!strcmp(ctype, "partner")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Conditions: partner [name]\r\n", ch);
      send_to_char("Value: name of a partner mob\r\n", ch);
      return NULL;
    }

    new_ec = get_ec();

    new_ec->subject = ECOND_SUBJECT_WORLD;
    new_ec->type = ECOND_PARTNER * enot;
    new_ec->subject = subject;

    new_ec->text = str_dup(keyword);

    new_ec->index = 0;
    new_ec->low = 0;
    new_ec->high = 0;

    send_to_char("Partner condition set.\r\n", ch); 
    return new_ec; 
  }


 /* gadget room id state */

  if (!strcmp(ctype, "gadget")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: gadget [room] [id] [state]\r\n", ch);
      send_to_char("Values: vnum of room, id and state of gadget\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);
    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    new_ec = get_ec();

    new_ec->type = ECOND_GADGET * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;

    new_ec->index = atoi(cname);

    if (new_ec->index <= 1) {
      send_to_char("Room must be > 1!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 1;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = 0;
    }

    send_to_char("Gadget condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* mob_in_room room lvnum hvnum */

  if (!strcmp(ctype, "mob_in_room")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: mob_in_room [room] [lvnum] [hvnum]\r\n", ch);
      send_to_char("Values: vnum of room, range for mob\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);
    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    new_ec = get_ec();

    new_ec->type = ECOND_MOB_IN_ROOM * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;

    new_ec->index = atoi(cname);

    if (new_ec->index <= 1) {
      send_to_char("Room must be > 1!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 1;
    }

    if (new_ec->low <= 1) {
      send_to_char("Mob low vnum must be > 1!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = 0;
    }

    if ( new_ec->high <=  1
      && new_ec->high != -1 ) {
      send_to_char("Mob high vnum must be > 1 or -1!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    send_to_char("Mob_in_room condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* obj_in_room room lvnum hvnum */

  if (!strcmp(ctype, "obj_in_room")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: obj_in_room [room] [lvnum] [hvnum]\r\n", ch);
      send_to_char("Values: vnum of room, range for object\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);
    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    new_ec = get_ec();

    new_ec->type = ECOND_OBJ_IN_ROOM * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;

    new_ec->index = atoi(cname);

    if (new_ec->index <= 1) {
      send_to_char("Room must be > 1!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 1;
    }

    if (new_ec->low <= 1) {
      send_to_char("Obj low vnum must be > 1!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = 0;
    }

    if ( new_ec->high <=  1
      && new_ec->high != -1 ) {
      send_to_char("Obj high vnum must be > 1 or -1!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    send_to_char("Obj_in_room condition set.\r\n", ch); 
 
    return new_ec; 
  }


  if (!strcmp(ctype, "obj_room_container")) {
    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: obj_room_container [room] [container] [vnum]\r\n", ch);
      send_to_char("Values: vnum of room, vnum of container, vnum of object\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);
    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    new_ec = get_ec();

    new_ec->type = ECOND_OBJ_ROOM_CONTAINER * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;

    new_ec->index = atoi(cname);

    if (new_ec->index <= 1) {
      send_to_char("Room must be > 1!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 1;
    }

    if (new_ec->low <= 1) {
      send_to_char("Container vnum must be > 1!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = 0;
    }

    if ( new_ec->high <=  1) {
      send_to_char("Obj vnum must be > 1!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    send_to_char("Obj_room_container condition set.\r\n", ch); 
 
    return new_ec; 
  }


 /* player_in_room room */

  if (!strcmp(ctype, "player_in_room")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: player_in_room [room]\r\n", ch);
      send_to_char("Values: vnum of room\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);

    new_ec = get_ec();

    new_ec->type = ECOND_PLAYER_IN_ROOM * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;

    new_ec->index = atoi(cname);

    if (new_ec->index <= 1) {
      send_to_char("Room must be > 1!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    send_to_char("Player_in_room condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* room_empty_mob room */

  if (!strcmp(ctype, "room_empty_mob")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: room_empty_mob [room]\r\n", ch);
      send_to_char("Values: vnum of room\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);

    new_ec = get_ec();

    new_ec->type = ECOND_ROOM_EMPTY_MOB * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;

    new_ec->index = atoi(cname);

    if (new_ec->index <= 1) {
      send_to_char("Room must be > 1!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    send_to_char("Room_empty_mob condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* room_empty_obj room */

  if (!strcmp(ctype, "room_empty_obj")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: room_empty_obj [room]\r\n", ch);
      send_to_char("Values: vnum of room\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);

    new_ec = get_ec();

    new_ec->type = ECOND_ROOM_EMPTY_OBJ * enot;
    new_ec->subject = ECOND_SUBJECT_WORLD;

    new_ec->index = atoi(cname);

    if (new_ec->index <= 1) {
      send_to_char("Room must be > 1!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    send_to_char("Room_empty_obj condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* type low hi */

  if (!strcmp(ctype, "type")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: type [low] [hi]\r\n", ch);
      send_to_char("Values: event types (0..x)\r\n", ch);
      send_to_char("    or: item types (0..x)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_WEV * enot;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_TYPE;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Type condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* subtype low hi */

  if (!strcmp(ctype, "subtype")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: subtype [low] [hi]\r\n", ch);
      send_to_char("Values: event subtypes (0..x)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_WEV;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_SUBTYPE * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Subtype condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* number low hi */

  if (!strcmp(ctype, "number")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: number [low] [hi]\r\n", ch);
      send_to_char("Values: event numbers (-OO..+00)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_WEV;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_NUMBER * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Number condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* number_mod mod low hi */

  if (!strcmp(ctype, "number_mod")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: number_mod [mod] [low] [hi]\r\n", ch);
      send_to_char("Values: event numbers (-00..+00)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);
    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    new_ec = get_ec();

    new_ec->type = ECOND_NUMBER_MOD * enot;
    new_ec->subject = ECOND_SUBJECT_WEV;

    new_ec->index = atoi(cname);

    if (new_ec->index <= 1) {
      send_to_char("Mod value must be > 1!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Number_mod condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* am_actor 0 0 */

  if (!strcmp(ctype, "am_actor")) {

    if (keyword[0] != '\0' || keyword[0] == '?') {
      send_to_char("Condition: am_actor\r\n", ch);
      return NULL;
    }

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_WEV;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_AM_ACTOR * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    new_ec->low = 0;
    new_ec->high = -1;

    send_to_char("Am_Actor condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* am_victim 0 0 */

  if (!strcmp(ctype, "am_victim")) {

    if (keyword[0] != '\0' || keyword[0] == '?') {
      send_to_char("Condition: am_victim\r\n", ch);
      return NULL;
    }

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_WEV;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_AM_VICTIM * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    new_ec->low = 0;
    new_ec->high = -1;

    send_to_char("Am_Victim condition set.\r\n", ch); 
 
    return new_ec; 
  }


  if (!strcmp(ctype, "is_alone")) {

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_WEV;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_IS_ALONE * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    new_ec->low = 0;
    new_ec->high = -1;

    send_to_char("Is_alone condition set.\r\n", ch); 
 
    return new_ec; 
  }


  if (!strcmp(ctype, "uses_imp")) {

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_WEV;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_USES_IMP * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    new_ec->low = 0;
    new_ec->high = -1;

    send_to_char("Uses_IMP condition set.\r\n", ch); 
     return new_ec; 
  }


 /* local 0 0 */

  if (!strcmp(ctype, "local")) {

    if (keyword[0] != '\0' || keyword[0] == '?') {
      send_to_char("Condition: local\r\n", ch);
      return NULL;
    }

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_WEV;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_LOCAL * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    new_ec->low = 0;
    new_ec->high = -1;

    send_to_char("Local condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* actor_grouped 0 0 */

  if (!strcmp(ctype, "actor_grouped")) {

    if (keyword[0] != '\0' || keyword[0] == '?') {
      send_to_char("Condition: actor_grouped\r\n", ch);
      return NULL;
    }

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_WEV;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_ACTOR_GROUPED * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    new_ec->low = 0;
    new_ec->high = -1;

    send_to_char("Actor_Grouped condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* victim_grouped 0 0 */

  if (!strcmp(ctype, "victim_grouped")) {

    if (keyword[0] != '\0' || keyword[0] == '?') {
      send_to_char("Condition: victim_grouped\r\n", ch);
      return NULL;
    }

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_WEV;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_VICTIM_GROUPED * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    new_ec->low = 0;
    new_ec->high = -1;

    send_to_char("Victim_Grouped condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* actor_remembered slot */

  if (!strcmp(ctype, "actor_remembered")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: actor_remembered [slot]\r\n", ch);
      send_to_char("Values: memory slot (0..9)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_WEV;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_ACTOR_REMEMBERED * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if ( new_ec->low < 0
      || new_ec->low >= MOB_MEMORY) {
      send_to_char("Memory slot must be between 0 and 9.", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    new_ec->high = 0;

    send_to_char("Actor Remembered condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* victim_remembered slot */

  if (!strcmp(ctype, "victim_remembered")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: victim_remembered [slot]\r\n", ch);
      send_to_char("Values: memory slot (0..9)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_WEV;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_VICTIM_REMEMBERED * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if ( new_ec->low < 0
      || new_ec->low >= MOB_MEMORY) {
      send_to_char("Memory slot must be between 0 and 9.", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    new_ec->high = 0;

    send_to_char("Victim Remembered condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* actor_friend 0 0 */

  if (!strcmp(ctype, "actor_friend")) {

    if (keyword[0] != '\0' || keyword[0] == '?') {
      send_to_char("Condition: actor_friend\r\n", ch);
      return NULL;
    }

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_WEV;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_ACTOR_FRIEND * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    new_ec->low = 0;
    new_ec->high = -1;

    send_to_char("Actor_Friend condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* actor_foe 0 0 */

  if (!strcmp(ctype, "actor_foe")) {

    if (keyword[0] != '\0' || keyword[0] == '?') {
      send_to_char("Condition: actor_foe\r\n", ch);
      return NULL;
    }

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_WEV;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_ACTOR_FOE * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    new_ec->low = 0;
    new_ec->high = -1;

    send_to_char("Actor_Foe condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* victim_friend 0 0 */

  if (!strcmp(ctype, "victim_friend")) {

    if (keyword[0] != '\0' || keyword[0] == '?') {
      send_to_char("Condition: victim_friend\r\n", ch);
      return NULL;
    }

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_WEV;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_VICTIM_FRIEND * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    new_ec->low = 0;
    new_ec->high = -1;

    send_to_char("Victim_Friend condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* victim_foe 0 0 */

  if (!strcmp(ctype, "victim_foe")) {

    if (keyword[0] != '\0' || keyword[0] == '?') {
      send_to_char("Condition: victim_foe\r\n", ch);
      return NULL;
    }

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_WEV;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_VICTIM_FOE * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    new_ec->low = 0;
    new_ec->high = -1;

    send_to_char("Victim_Foe condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* actor_visible 0 0 */

  if (!strcmp(ctype, "actor_visible")) {

    if (keyword[0] != '\0' || keyword[0] == '?') {
      send_to_char("Condition: actor_visible\r\n", ch);
      return NULL;
    }

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_WEV;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_ACTOR_VISIBLE * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    new_ec->low = 0;
    new_ec->high = -1;

    send_to_char("Actor_Visible condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* victim_visible 0 0 */

  if (!strcmp(ctype, "victim_visible")) {

    if (keyword[0] != '\0' || keyword[0] == '?') {
      send_to_char("Condition: victim_visible\r\n", ch);
      return NULL;
    }

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_WEV;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_VICTIM_VISIBLE * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    new_ec->low = 0;
    new_ec->high = -1;

    send_to_char("Victim_Visible condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* vnum vlow vhi */

  if (!strcmp(ctype, "vnum")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: vnum [vlow] [vhi]\r\n", ch);
      send_to_char("Values: obj/mob vnums (0..x)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_P_OBJ;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_VNUM * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Vnum condition set.\r\n", ch); 
 
    return new_ec; 
  }


  if (!strcmp(ctype, "size")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: size [size low] [size high]\r\n", ch);
      send_to_char("Values: 0 - 5\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_P_OBJ;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_SIZE * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
      new_ec->low = URANGE(0, new_ec->low, 5);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
      new_ec->high = URANGE(-1, new_ec->high, 5);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Size condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* in_room vlow vhi */

  if (!strcmp(ctype, "in_room")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: in_room [vlow] [vhi]\r\n", ch);
      send_to_char("Values: room vnums (0..x)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_IN_ROOM * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("In_Room condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* in_subarea vlow vhi */

  if (!strcmp(ctype, "in_subarea")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: in_subarea [low] [hi]\r\n", ch);
      send_to_char("Values: subarea numbers (0..x)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_IN_SUBAREA * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("In_SubArea condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* deed id done */

  if (!strcmp(ctype, "deed")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: deed [id] [flag]\r\n", ch);
      send_to_char("Values: id: deed id, flag: 1 done, 0 undone\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_DEED * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (new_ec->low < 0) {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = 1;
    }

    send_to_char("Deed condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* quest id low hi */

  if (!strcmp(ctype, "quest")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: quest [id] [low] [hi]\r\n", ch);
      send_to_char("Values: quest states (0..9999)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);
    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_QUEST * enot;
    new_ec->subject = subject;

    new_ec->index = atoi(cname);

    if (new_ec->index <= 1) {
      send_to_char("Id must be > 1!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Quest condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* memory slot */

  if (!strcmp(ctype, "memory")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: memory [slot]\r\n", ch);
      send_to_char("Values: slot number (0..9)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_MEMORY * enot;
    new_ec->subject = subject;

    new_ec->index = atoi(cname);

    if (new_ec->index < 0) {
      send_to_char("Slot must be >= 0!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (new_ec->index >= MOB_MEMORY) {
      send_to_char("Slot must be < 10!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    new_ec->low = 0;
    new_ec->high = 0;

    send_to_char("Memory condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* memory_value id low hi */

  if (!strcmp(ctype, "memory_value")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: memory_value [slot] [low] [hi]\r\n", ch);
      send_to_char("Values: numbers (-32768..32768)\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);
    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_MEMORYV * enot;
    new_ec->subject = subject;

    new_ec->index = atoi(cname);

    if (new_ec->index < 0) {
      send_to_char("Slot must be >= 0!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (new_ec->index >= MOB_MEMORY) {
      send_to_char("Slot must be < 10!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("Memory Value condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* value number low hi */

  if (!strcmp(ctype, "value")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Condition: value [number] [low] [hi]\r\n", ch);
      send_to_char("Values: object values\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, cname);
    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject != ECOND_SUBJECT_S_OBJ) {
      subject = ECOND_SUBJECT_P_OBJ;
    }
    
    new_ec = get_ec();

    new_ec->type = ECOND_VALUE * enot;

    new_ec->subject = subject;

    new_ec->index = atoi(cname);

    if ( new_ec->index < 0
      || new_ec->index > 4) {
      send_to_char("Number must be betwen 0 and 4!\r\n", ch);
      free_ec(new_ec); 
      return NULL;
    } 

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = -1;
    }

    send_to_char("value condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* contains low hi */

  if (!strcmp(ctype, "contains")) {

    if (keyword[0] == '\0' || keyword[0] == '?') {
      send_to_char("Container, Locker, Keyring\r\n"
                   "  contains vnum vnum\r\n"
                   "Drink Container\r\n"
                   "  contains liquid hours\r\n"
                   "Fountain\r\n"
                   "  contains liquid\r\n"
                   "Wand, Staff\r\n"
                   "  contains spell charges\r\n"
                   "Potion, Pill, Scroll\r\n"
                   "  contains spell spell\r\n"
                   "Light\r\n"
                   "  contains hours\r\n", ch);
      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject != ECOND_SUBJECT_S_OBJ) {
      subject = ECOND_SUBJECT_P_OBJ;
    }
    
    new_ec = get_ec();

    new_ec->type = ECOND_CONTAINS * enot;
    new_ec->subject = subject;

    new_ec->index = 0;

    if (clow[0] != '\0') {
      new_ec->low = atoi(clow);
    } else {
      new_ec->low = 0;
    }

    if (chi[0] != '\0') {
      new_ec->high = atoi(chi);
    } else {
      new_ec->high = 0;
    }

    send_to_char("Contains condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* is state */

  if (!strcmp(ctype, "is")) {

    int index;

    if (keyword[0] == '\0' || keyword[0] == '?') {

      send_to_char(
  "Condition: is state\r\n"
  "State:\r\n"
  "  starving hungry full overweight\r\n"
  "  dehydrated thirsty refreshed sober drunk\r\n"
  "  standing fighting sitting resting awake sleeping stunned dieing dead\r\n"
  "  criminal were vampire undead stranger passport\r\n"
  "  blind invisibile seeing_evil seeing_invisible seeing_magic\r\n"
  "  seeing_hidden mind_melded sanctuary faerie_fire seeing_infrared\r\n"
  "  cursed scared poisoned protected_from_good protected_from_evil\r\n"
  "  sneaking charmed flying pass_door hasted calmed plauge_ridden\r\n"
  "  weakened seeing_in_darkness berserk swimming regenerating\r\n"
  "  polymorphed absorbing_magic fleeing dreaming darkness aura\r\n"
  "  hallucinating relaxed fire-shield frost-shield incarnated morf\r\n"
  "  mist elder-shield farsight asceticism\r\n", ch);

      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    index = get_is_number(clow);

    if (index <= 0) {
      return NULL;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_IS * enot;
    new_ec->subject = subject;

    new_ec->index = index;

    new_ec->low  = 0;
    new_ec->high = 0;

    send_to_char("Is condition set.\r\n", ch); 
 
    return new_ec; 
  }

 /* is_not state */

  if (!strcmp(ctype, "is_not")) {

    int index;

    if (keyword[0] == '\0' || keyword[0] == '?') {

      send_to_char(
  "Condition: is_not state\r\n"
  "State:\r\n"
  "  starving hungry full overweight\r\n"
  "  dehydrated thirsty refreshed sober drunk\r\n"
  "  standing fighting sitting resting awake sleeping stunned dieing dead\r\n"
  "  criminal were vampire undead stranger passport\r\n"
  "  blind invisibile seeing_evil seeing_invisible seeing_magic\r\n"
  "  seeing_hidden mind_melded sanctuary faerie_fire seeing_infrared\r\n"
  "  cursed scared poisoned protected_from_good protected_from_evil\r\n"
  "  sneaking charmed flying pass_door hasted calmed plauge_ridden\r\n"
  "  weakened seeing_in_darkness berserk swimming regenerating\r\n"
  "  polymorphed absorbing_magic fleeing dreaming darkness aura\r\n"
  "  hallucinating relaxed fire-shield frost-shield incarnated morf\r\n"
  "  mist elder-shield farsight asceticism\r\n", ch);
      send_to_char("{rThis is redundent, use {ynot is{r instead.{x\r\n", ch);

      return NULL;
    }

    keyword = one_argument(keyword, clow);
    keyword = one_argument(keyword, chi);  

    if (subject == ECOND_SUBJECT_WORLD) {
      subject = ECOND_SUBJECT_ACTOR;
    }

    index = get_is_number(clow);

    if (index <= 0) {
      return NULL;
    }

    new_ec = get_ec();

    new_ec->type = ECOND_IS_NOT * enot;
    new_ec->subject = subject;

    new_ec->index = index;

    new_ec->low  = 0;
    new_ec->high = 0;

    send_to_char("Is_Not condition set.\r\n", ch); 
 
    return new_ec; 
  }

  send_to_char("Invalid condition specification!\r\n", ch);
  send_to_char( "Syntax: <subject> <not> condition parms\r\n", ch);

  return NULL;
}


int arrange_div(int divinity) {
     if (IS_SET(divinity, DIV_IMPLEMENTOR)) 	return 8;
     if (IS_SET(divinity, DIV_GREATER)) 		return 7;
     if (IS_SET(divinity, DIV_GOD)) 			return 6;
     if (IS_SET(divinity, DIV_LESSER)) 		return 5;
     if (IS_SET(divinity, DIV_CREATOR)) 		return 4;
     if (IS_SET(divinity, DIV_HERO)) 		return 3;
     if (IS_SET(divinity, DIV_INVESTIGATOR))	return 2;
     if (IS_SET(divinity, DIV_HUMAN)) 		return 1;
     if (divinity == 0) 				return 0;
     if (IS_SET(divinity, DIV_MUD)) 			return 9;
     return 0;
}

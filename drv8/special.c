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
#include "magic.h"
#include "affect.h"
#include "spell.h"
#include "exp.h"
#include "society.h"
#include "race.h"
#include "profile.h"
#include "skill.h"
#include "mob.h"
#include "wev.h"
#include "map.h"

/* command procedures needed */
DECLARE_DO_FUN(do_yell);
DECLARE_DO_FUN(do_open);
DECLARE_DO_FUN(do_close);
DECLARE_DO_FUN(do_cast);
DECLARE_DO_FUN(do_scream);
DECLARE_DO_FUN(do_shout);
DECLARE_DO_FUN(do_socials);
DECLARE_DO_FUN(do_rest);
DECLARE_DO_FUN(do_stand);
DECLARE_DO_FUN(do_get);
DECLARE_DO_FUN(do_backstab);
DECLARE_DO_FUN(do_murder);
DECLARE_DO_FUN(do_flee);

extern void 		do_mob_cast		(CHAR_DATA *mob, char *spell_name, CHAR_DATA *victim);

/*
 * The following special functions are available for mobiles.
 */
DECLARE_SPEC_FUN(spec_breath_any);
DECLARE_SPEC_FUN(spec_breath_acid);
DECLARE_SPEC_FUN(spec_breath_fire);
DECLARE_SPEC_FUN(spec_breath_frost);
DECLARE_SPEC_FUN(spec_breath_gas);
DECLARE_SPEC_FUN(spec_breath_lightning);
DECLARE_SPEC_FUN(spec_cast_adept);
DECLARE_SPEC_FUN(spec_cast_cleric);
DECLARE_SPEC_FUN(spec_cast_judge);
DECLARE_SPEC_FUN(spec_cast_mage);
DECLARE_SPEC_FUN(spec_cast_undead);
DECLARE_SPEC_FUN(spec_cast_nature);
DECLARE_SPEC_FUN(spec_cast_power);
DECLARE_SPEC_FUN(spec_executioner);
DECLARE_SPEC_FUN(spec_fido);
DECLARE_SPEC_FUN(spec_guard	);
DECLARE_SPEC_FUN(spec_janitor);
DECLARE_SPEC_FUN(spec_mayor);
DECLARE_SPEC_FUN(spec_poison);
DECLARE_SPEC_FUN(spec_plague);
DECLARE_SPEC_FUN(spec_thief);
DECLARE_SPEC_FUN(spec_puff);
DECLARE_SPEC_FUN(spec_clanguard);
DECLARE_SPEC_FUN(spec_homeguard);
DECLARE_SPEC_FUN(spec_hunter);
DECLARE_SPEC_FUN(spec_tracker);
DECLARE_SPEC_FUN(spec_teleTracker);
DECLARE_SPEC_FUN(spec_teleHunter);
DECLARE_SPEC_FUN(spec_artifact_tracker);
DECLARE_SPEC_FUN(spec_artifact_hunter);
DECLARE_SPEC_FUN(spec_hound);
DECLARE_SPEC_FUN(spec_clanhealer);
DECLARE_SPEC_FUN(spec_questmaster); 
DECLARE_SPEC_FUN(spec_playershop); 
DECLARE_SPEC_FUN(spec_miner); 
DECLARE_SPEC_FUN(spec_translator); 
DECLARE_SPEC_FUN(spec_mechanic); 
DECLARE_SPEC_FUN(spec_nasty);
DECLARE_SPEC_FUN(spec_troll_member);
DECLARE_SPEC_FUN(spec_ogre_member);
DECLARE_SPEC_FUN(spec_patrolman);
DECLARE_SPEC_FUN(spec_clanmember);
DECLARE_SPEC_FUN(spec_social);


/*
 * Special Functions Table.
 */
const   struct  spec_type       spec_table      [ ] =
{
    /*
     * Special function commands.
     */
    { "spec_breath_any",		spec_breath_any},
    { "spec_breath_acid",		spec_breath_acid	},
    { "spec_breath_fire",		spec_breath_fire},
    { "spec_breath_frost",		spec_breath_frost},
    { "spec_breath_gas",		spec_breath_gas},
    { "spec_breath_lightning",	spec_breath_lightning},
    { "spec_cast_adept",		spec_cast_adept},
    { "spec_cast_cleric",		spec_cast_cleric},
    { "spec_cast_mage",		spec_cast_mage},
    { "spec_cast_undead",		spec_cast_undead},
    { "spec_cast_judge",		spec_cast_judge},
    { "spec_cast_nature",              	spec_cast_nature},
    { "spec_cast_power",               	spec_cast_power},
    { "spec_executioner",		spec_executioner},
    { "spec_fido",			spec_fido},
    { "spec_guard",		spec_guard},
    { "spec_janitor",		spec_janitor},
    { "spec_mayor",		spec_mayor},
    { "spec_poison",		spec_poison},
    { "spec_plague",		spec_plague},
    { "spec_thief",			spec_thief},
    { "spec_puff",			spec_puff},    
    { "spec_clanguard",		spec_clanguard},      
    { "spec_homeguard",		spec_homeguard},      
    { "spec_hunter",		spec_hunter},      
    { "spec_tracker",		spec_tracker},      
    { "spec_teleTracker",		spec_teleTracker},      
    { "spec_teleHunter",		spec_teleHunter},      
    { "spec_artifactTracker",	spec_artifact_tracker},      
    { "spec_artifactHunter",		spec_artifact_hunter},      
    { "spec_hound",		spec_hound},      
    { "spec_clanhealer",		spec_clanhealer},      
    { "spec_questmaster",		spec_questmaster}, 
    { "spec_playershop",		spec_playershop}, 
    { "spec_miner",		spec_miner}, 
    { "spec_translator",		spec_translator}, 
    { "spec_mechanic",		spec_mechanic}, 
    { "spec_nasty",		spec_nasty},
    { "spec_troll_member",		spec_troll_member},
    { "spec_ogre_member",		spec_ogre_member},
    { "spec_patrolman",		spec_patrolman},
    { "spec_clanmember",		spec_clanmember},
    { "spec_social",		spec_social},
    { "",				NULL 			}
};

extern void raw_kill( CHAR_DATA *victim, bool corpse );

void spc_port( CHAR_DATA *ch, ROOM_INDEX_DATA *from, ROOM_INDEX_DATA *to);  

/*****************************************************************************
 Name:          spec_lookup
 Purpose:       Given a name, return the appropriate spec fun.
 Called by:     do_mset(act_wiz.c) load_specials,reset_area(db.c)
 ****************************************************************************/
SPEC_FUN *spec_lookup( const char *name )       /* OLC */
{
    int cmd;

    for ( cmd = 0; spec_table[cmd].spec_name[0] != '\0'; cmd++ )
        if ( !str_cmp( name, spec_table[cmd].spec_name ) )
            return spec_table[cmd].spec_fun;

    return 0;
}

/* Hound combo -- Track and kill, suicide if it dies. */

bool spec_hound( CHAR_DATA *hound) {
  ROOM_INDEX_DATA *marshes;

 /* If we have prey, go kill it... */ 

  if (hound->prey != NULL) {
    return spec_teleHunter(hound);
  }

 /* Otherwise we go home... */ 

  marshes = find_location(hound, "20000");

  if (marshes != NULL) {
    spc_port(hound, hound->in_room, marshes);   
  }

 /* And drop dead! */

  raw_kill(hound, FALSE);

  return TRUE;
}

/* Combo - track it then kill it... */

bool spec_teleHunter( CHAR_DATA *hunter) {

  if (spec_teleTracker(hunter)) {
    return TRUE;
  }  

  return spec_hunter(hunter);
}

/* Special function for tracking prey... */

/* Teletracker - If the monster has acquired some prey and the prey
 * isn't in the same room as the monster, it will teleport to the preys
 * room.
 */

bool spec_teleTracker( CHAR_DATA *tracker) {
  int nogo;

 /* If I'm hunting someone... */
  if (tracker->prey == NULL) return FALSE;

 /* If they are somewhere sensible... */
  if (tracker->prey->in_room == NULL) return FALSE;

 /* If I'm not already there... */ 

  if (tracker->in_room == tracker->prey->in_room) {
    return FALSE;
  }

 /* Can I go there... */

  nogo =   ROOM_NO_MOB    | ROOM_SAFE  | ROOM_PRIVATE   | ROOM_SOLITARY    | ROOM_IMP_ONLY | ROOM_GODS_ONLY | ROOM_HEROES_ONLY;
  if (IS_AFFECTED(tracker, AFF_FLYING)) nogo |= ROOM_INDOORS;
  if ( IS_SET(tracker->prey->in_room->room_flags, nogo)) return FALSE;

 /* Teleport time... */
  spc_port(tracker, tracker->in_room, tracker->prey->in_room);
  return TRUE;
}


bool spec_tracker( CHAR_DATA *tracker) {

  if (!tracker->prey) return spec_hunter(tracker);
  if (!tracker->prey->in_room) return FALSE;

  if (tracker->in_room->area != tracker->prey->in_room->area) {
        tracker->prey->huntedBy = NULL;
        tracker->prey = NULL;
        return FALSE;
  }

  if (tracker->in_room == tracker->prey->in_room) return spec_hunter(tracker);

  if (number_percent() > 50) return FALSE;
  track(tracker, tracker->prey);
  return TRUE;
}


/* Generic teleport a player/mob subroutine... */ 
void spc_port( CHAR_DATA *ch, ROOM_INDEX_DATA *from, ROOM_INDEX_DATA *to) {  
  CHAR_DATA *rch;
  bool fly;

 /* Stop fighting... */

  if ( ch->fighting != NULL )
    stop_fighting( ch, TRUE );

 /* How do we go? */

  if (  IS_AFFECTED(ch, AFF_FLYING)
    && !IS_SET(from->room_flags, ROOM_INDOORS)
    && !IS_SET(  to->room_flags, ROOM_INDOORS)) {
    fly = TRUE;
  } else {
    fly = FALSE;
  }

 /* Tell everyone I'm going... */

  for (rch = from->people; rch != NULL; rch = rch->next_in_room) {
    if (get_trust(rch) >= ch->invis_level) {
      if (fly) {
        act("$n flies up into the stars...", ch, NULL, rch, TO_VICT);
      } else { 
        act("$n leaves in a swirling mist.", ch, NULL, rch, TO_VICT);
      }
    }
  }

 /* Tell the player... */ 

  if (fly) {
    act("You fly through the stars...", ch, NULL, NULL, TO_CHAR);
  } else {
    act("You step through the swirling mists...", ch, NULL, NULL, TO_CHAR);
  }

 /* Teleport */

  char_from_room( ch );
  char_to_room( ch, to );

 /* Tell everyone I've arrived... */ 

  for (rch = to->people; rch != NULL; rch = rch->next_in_room) {
    if (get_trust(rch) >= ch->invis_level) {
      if (fly) {
        act("$n flies down from the stars...", ch, NULL, rch, TO_VICT);
      } else { 
        act("$n appears in a swirling mist.", ch, NULL, rch, TO_VICT);
      }
    }
  } 

 /* All done... */

  return; 
}

/* Special function for hunting monsters... */

/* Hunter - A monster which will remember who it is fighting.  If it sees
 * them again, it will attack them again.   
 * 
 * This uses the prey pointer on the mob to keep track of whoever it is
 * meant to be killing.  If the pointer is set by some other mechanism and
 * the mob ends up in the same room as the player, it will attack them.
 */ 

bool spec_hunter( CHAR_DATA *hunter) {

 /* If we're fighting, then that's who we'll hunt... */

  if (hunter->fighting != NULL) {

   /* Time to switch prey? */

    if ( hunter->fighting != hunter->prey ) {
        
      if (hunter->prey != NULL) {
         hunter->prey->huntedBy = NULL;
      }

      hunter->prey = hunter->fighting;
      hunter->prey->huntedBy = hunter;

      if ( hunter->in_room  == hunter->prey->in_room ) {
        act("$n looks at you hungrily!", hunter, NULL, hunter->prey, TO_VICT);
        act("$n looks hungrily at $N!", hunter, NULL, hunter->prey, TO_NOTVICT);
        act("You are now hunting $N!", hunter, NULL, hunter->prey, TO_CHAR);  
      }

      return TRUE;
    }

    return FALSE;
  }

 /* If we're not fighting, see if the prey is here and fight it if it is... */

  if (hunter->prey != NULL) {

    if (hunter->prey->in_room == hunter->in_room) {

      if (can_see(hunter, hunter->prey)) {
        act("$n attacks $N!", hunter, NULL, hunter->prey, TO_NOTVICT);
        act("$n attacks you!", hunter, NULL, hunter->prey, TO_VICT);
        act("You attack $N!", hunter, NULL, hunter->prey, TO_CHAR);
        multi_hit( hunter, hunter->prey, TYPE_UNDEFINED );

        return TRUE;
      }
    }
  }

  return FALSE;
}

/* Special function for clanguards... */

/* Clan Guard - A monster which will attack PCs in the same room as it,
 * if they do not have the same clan aliegence. 
 *
 * The main problem is telling the guard which clan he is working for.
 *
 * I'll use a trick with the guards name. The last word in its name should
 * be of the format _soc_id, where n is the number of the clan it works for.
 *
 * For societies use _soc_id.
 *
 */ 

bool spec_clanguard( CHAR_DATA *ch) {
  CHAR_DATA *suspect, *susfoe, *testsus;
  SOCIETY *society;
  MEMBERSHIP *membership;
  char buf[MAX_STRING_LENGTH];
  int i, j, k;
  char *check;
  int c_num;

 /* Do we have to do this? */

 /* Not if we're not in a room... */

  if (ch->in_room == NULL) return FALSE;
 
 /* Nor it we're the only person in the room... */

  if ( ch->in_room->people == ch 
    && ch->next_in_room == NULL) {
    return FALSE;
  }

 /* ...and we should be standing guard... */ 

  if ( ch->position != POS_STANDING 
    && ch->position != POS_FIGHTING ) {
    return FALSE;
  }

  c_num = -1;

  if (ch->pIndexData->group > 0) {
        c_num = ch->pIndexData->group;

  } else {
      check = "_soc_";

 /* Search name for _soc_nnnn and extract the number... */

      i = -1;
      j = 0;

      while ((ch->name[++i] != '\0') && (c_num == -1)) {

        if (ch->name[i] == ' ') {
            k = 0;
            while (check[k] != '\0' && ch->name[++i] == check[k++]) {
            }

            if (k == 5) {
                 j = 0;
                 while (isdigit(ch->name[++i]) && j < (MAX_STRING_LENGTH - 1)) {
                     buf[j++] = ch->name[i];
                 }
                 if (j > 0) {
                     buf[j] = '\0';
                     c_num = atoi(buf);
                 } else {
                     bug("_soc_ without society id on clanguard in %d", ch->in_room->vnum); 
                     return TRUE;
                 }
            }
        } 
      }   
   }
 /* Complain if the number isn't valid... */

  if (c_num < 0) {
    bug("No society num on clanguard in %d", ch->in_room->vnum);
    return TRUE;
  } 
      
  society = find_society_by_id(c_num);

  if (society == NULL) {
    bug("Invalid society id on clanguard in %d",ch->in_room->vnum);
    return TRUE;
  } 

 /* Better check I'm a member... */

  if (!is_member(ch, society)) create_membership(ch, society, SOC_RANK_MEMBER);
  membership = find_membership(ch, society);

  if (membership == NULL) {

    membership = get_membership();

   /* Add profession and level, if appropriate... */

    membership->society = society;
    membership->level = SOC_LEVEL_MEMBERSHIP;
    membership->profession = society->profession;

   /* Splice in... */

    membership->next = ch->memberships;
    ch->memberships = membership;

  } 

 /* If we're fighting, check it's not against a clan member... */

  if (ch->fighting != NULL) {
    membership = find_membership(ch->fighting, society);
    if ( membership != NULL ) {

      switch (membership->level) {
        default:
          sprintf(buf,"%s is a Traitor to the '%s'!", ch->fighting->short_descr, society->name);
          break;
        case SOC_LEVEL_FOE:
        case SOC_LEVEL_EXPEL:
          sprintf(buf,"Our foe, %s is attacking the '%s'!", ch->fighting->short_descr, society->name);
          break;
      }
    } else {
      sprintf(buf, "%s is attacking the '%s'!", ch->fighting->short_descr, society->name);
    }
    do_shout(ch, buf);
    return TRUE;
  }

 /* Check for non-clan members in the room... */

  suspect = ch->in_room->people; 

  while (suspect != NULL) {

    testsus = suspect;
    suspect = suspect->next_in_room;

    if (testsus == ch) continue;
    if (IS_IMMORTAL(testsus)) continue; 
    if (!can_see(ch, testsus)) continue;
   
    if (is_foe(ch, testsus)) {
       if (str_cmp(race_array[testsus->race].name,"machine")
       && !IS_SET(testsus->form, FORM_MACHINE)) {
            if (IS_NPC(testsus)) {
                if (testsus->pIndexData->pShop == NULL) {
                      do_shout(ch, "Intruder! Attack! Intruder!"); 
                      multi_hit( ch, testsus, TYPE_UNDEFINED );
                      break;
                }
            } else {
                do_shout(ch, "Intruder! Attack! Intruder!"); 
                multi_hit( ch, testsus, TYPE_UNDEFINED );
                break;
            }
        }
    } else if (is_friend(ch, testsus)) {
       if (str_cmp(race_array[testsus->race].name,"machine")
       && !IS_SET(testsus->form, FORM_MACHINE)) {
            susfoe = testsus->fighting;
            if (susfoe != NULL) {
                if (IS_NPC(susfoe)) {
                        if (susfoe->pIndexData->pShop == NULL) {
                              do_shout(ch, "Protect the way! Protect the way!"); 
                              multi_hit( ch, susfoe, TYPE_UNDEFINED );
                              break;
                        }
                } else {
                        do_shout(ch, "Protect the way! Protect the way!"); 
                        multi_hit( ch, susfoe, TYPE_UNDEFINED );
                        break;
                } 
            }
       } 
   } else { 
      act("$n looks at $N suspisiously.", ch, NULL, testsus, TO_NOTVICT);  
      act("$n looks at you suspisiously!", ch, NULL, testsus, TO_VICT);
      if (number_bits(2) == 0) {
           if (str_cmp(race_array[testsus->race].name,"machine")
           && !IS_SET(testsus->form, FORM_MACHINE)) {
                   if (IS_NPC(testsus)) {
                        if (testsus->pIndexData->pShop == NULL) {
                                do_shout(ch, "Intruder! Attack! Intruder!"); 
                                 multi_hit( ch, testsus, TYPE_UNDEFINED );
                        }
                   } else {
                        do_shout(ch, "Intruder! Attack! Intruder!"); 
                        multi_hit( ch, testsus, TYPE_UNDEFINED );
                   }
            }                
            break;
       }
    }
  }

  return TRUE;
}


bool spec_clanmember( CHAR_DATA *ch) {
  CHAR_DATA *suspect, *testsus;
  SOCIETY *society;
  MEMBERSHIP *membership;
  char buf[MAX_STRING_LENGTH];
  int i, j, k;
  char *check;
  int c_num;

 /* Do we have to do this? */

 /* Not if we're not in a room... */

  if (ch->in_room == NULL) return FALSE;
 
 /* Nor it we're the only person in the room... */

  if ( ch->in_room->people == ch 
    && ch->next_in_room == NULL) {
    return FALSE;
  }

 /* ...and we should be standing guard... */ 

  if ( ch->position != POS_STANDING 
    && ch->position != POS_FIGHTING ) {
    return FALSE;
  }

  c_num = -1;

  if (ch->pIndexData->group > 0) {
        c_num = ch->pIndexData->group;

  } else {
      check = "_soc_";

 /* Search name for _soc_nnnn and extract the number... */

      i = -1;
      j = 0;

      while ((ch->name[++i] != '\0') && (c_num == -1)) {

        if (ch->name[i] == ' ') {
            k = 0;
            while (check[k] != '\0' && ch->name[++i] == check[k++]) {
            }

            if (k == 5) {
                 j = 0;
                 while (isdigit(ch->name[++i]) && j < (MAX_STRING_LENGTH - 1)) {
                     buf[j++] = ch->name[i];
                 }
                 if (j > 0) {
                     buf[j] = '\0';
                     c_num = atoi(buf);
                 } else {
                     bug("_soc_ without society id on clanguard in %d", ch->in_room->vnum); 
                     return TRUE;
                 }
            }
        } 
      }   
   }
 /* Complain if the number isn't valid... */

  if (c_num < 0) {
    bug("No society num on clanguard in %d", ch->in_room->vnum);
    return TRUE;
  } 
      
  society = find_society_by_id(c_num);

  if (society == NULL) {
    bug("Invalid society id on clanguard in %d",ch->in_room->vnum);
    return TRUE;
  } 

 /* Better check I'm a member... */

  if (!is_member(ch, society)) create_membership(ch, society, SOC_RANK_MEMBER);
  membership = find_membership(ch, society);

  if (membership == NULL) {

    membership = get_membership();

   /* Add profession and level, if appropriate... */

    membership->society = society;
    membership->level = SOC_LEVEL_MEMBERSHIP;
    membership->profession = society->profession;

   /* Splice in... */

    membership->next = ch->memberships;
    ch->memberships = membership;

  } 

 /* If we're fighting, check it's not against a clan member... */

  if (ch->fighting != NULL) {
    membership = find_membership(ch->fighting, society);
    if ( membership != NULL ) {

      switch (membership->level) {
        default:
          sprintf(buf,"%s is a Traitor to the '%s'!", ch->fighting->short_descr, society->name);
          break;
        case SOC_LEVEL_FOE:
        case SOC_LEVEL_EXPEL:
          sprintf(buf,"Our foe, %s is attacking the '%s'!", ch->fighting->short_descr, society->name);
          break;
      }
    } else {
      sprintf(buf, "%s is attacking the '%s'!", ch->fighting->short_descr, society->name);
    }
    do_shout(ch, buf);
    return TRUE;
  }

 /* Check for non-clan members in the room... */

  suspect = ch->in_room->people; 

  while (suspect != NULL) {

    testsus = suspect;
    suspect = suspect->next_in_room;

    if (testsus == ch) continue;
    if (IS_IMMORTAL(testsus)) continue; 
    if (!can_see(ch, testsus)) continue;
   
    if (is_foe(ch, testsus)) {
       if (str_cmp(race_array[testsus->race].name,"machine")
       && !IS_SET(testsus->form, FORM_MACHINE)) {
            if (IS_NPC(testsus)) {
                if (testsus->pIndexData->pShop == NULL) {
                      do_shout(ch, "Death to all our foes!"); 
                      multi_hit( ch, testsus, TYPE_UNDEFINED );
                      break;
                }
            } else {
                do_shout(ch, "Death to all our foes!"); 
                multi_hit( ch, testsus, TYPE_UNDEFINED );
                break;
            }
        }
    }
  }

  return TRUE;
}


bool spec_homeguard( CHAR_DATA *ch) {
  CHAR_DATA *suspect, *susfoe, *testsus;
  char* lastarg;
  char buf[MAX_STRING_LENGTH];

  if (ch->in_room == NULL) return FALSE;
  
  if ( ch->in_room->people == ch 
  && ch->next_in_room == NULL) {
        return FALSE;
  }

  if ( ch->position != POS_STANDING 
  && ch->position != POS_FIGHTING ) {
        return FALSE;
  }

  lastarg = check_mob_owner(ch);
  if (!strcmp(lastarg, "not found") || lastarg == NULL) return TRUE;
     
  if (ch->fighting != NULL) {
        sprintf(buf, "%s is attacking  %s's home!", ch->fighting->short_descr, capitalize(lastarg));
        do_shout(ch, buf);
        return TRUE;
  }

  suspect = ch->in_room->people; 

  while (suspect != NULL) {
       testsus = suspect;
       suspect = suspect->next_in_room;
       if (testsus == ch) continue;
    
       if (IS_IMMORTAL(testsus)) continue; 
       if (!can_see(ch, testsus)) continue;
  
       if (!str_cmp(testsus->short_descr, lastarg)) {
            if (str_cmp(race_array[testsus->race].name,"machine")
            && !IS_SET(testsus->form, FORM_MACHINE)) {
                 susfoe = testsus->fighting;
                 if (susfoe != NULL) {
                      do_shout(ch, "Protect the way! Protect the way!"); 
                      multi_hit( ch, susfoe, TYPE_UNDEFINED );
                      break;
                 } 
            } 
       } else {
            if (!IS_NPC(testsus)) {
                if (!str_cmp(testsus->pcdata->spouse, lastarg)) {
                      if (str_cmp(race_array[testsus->race].name,"machine")
                      && !IS_SET(testsus->form, FORM_MACHINE)) {
                           susfoe = testsus->fighting;
                           if (susfoe != NULL) {
                                 do_shout(ch, "Protect the way! Protect the way!"); 
                                 multi_hit( ch, susfoe, TYPE_UNDEFINED );
                           } 
                      } 
                      break;
                }
            }

            if (is_foe(ch, testsus)) {
                if (str_cmp(race_array[testsus->race].name,"machine")
                && !IS_SET(testsus->form, FORM_MACHINE)) {
                     do_shout(ch, "Intruder! Attack! Intruder!"); 
                     multi_hit( ch, testsus, TYPE_UNDEFINED );
                }
                break;
            } else { 
                act("$n looks at $N suspisiously.", ch, NULL, testsus, TO_NOTVICT);  
                act("$n looks at you suspisiously!", ch, NULL, testsus, TO_VICT);
                 if (number_bits(2) == 0) {
                      if (str_cmp(race_array[testsus->race].name,"machine")
                      && !IS_SET(testsus->form, FORM_MACHINE)) {
                           do_shout(ch, "Intruder! Attack! Intruder!"); 
                           multi_hit( ch, testsus, TYPE_UNDEFINED );
                      }                
                      break;
                 }
            }
       }
  }

  return TRUE;
}


/*
 * Core procedure for dragons.
 */
bool dragon( CHAR_DATA *ch, char *spell_name ) {
    CHAR_DATA *victim;

    int efn;

   /* Do we need to do this? */

    if ( ch->position != POS_FIGHTING
      || ch->in_room == NULL
      || ( ch->in_room->people == ch
        && ch->next_in_room == NULL ) ) {
      return FALSE;
    }
 
   /* Pick a victim... */

    victim = ch->in_room->people;

    while ( victim != NULL ) {

       if ( victim != ch
         && is_foe(ch, victim)
         && number_bits(2) == 0 ) {
         break;
       }

       victim = victim->next_in_room;
    } 

   /* Did we get anyone? */

    if ( victim == NULL ) {
      return FALSE;
    }

   /* Find the spell effect... */

    efn = get_effect_efn( spell_name );

    if ( efn <= 0 ) {
      bug("Bad spell name in spec_dragon", 0);
      return FALSE;
    }

   /* Go use it... */ 

    do_dragon_cast( efn, ch->level, ch, victim );

   /* All done... */

    return TRUE;
}


/*****************************************************************************
 Name:          spec_string
 Purpose:       Given a function, return the appropriate name.
 Called by:     <???>
 ****************************************************************************/
char *spec_string( SPEC_FUN *fun )      /* OLC */
{
    int cmd;

    for ( cmd = 0; spec_table[cmd].spec_fun != NULL; cmd++ )
        if ( fun == spec_table[cmd].spec_fun )
            return spec_table[cmd].spec_name;

    return 0;
}



/*
 * Special procedures for mobiles.
 */
bool spec_breath_any( CHAR_DATA *ch )
{
    if ( ch->position != POS_FIGHTING )
	return FALSE;

    switch ( number_bits( 3 ) )
    {
    case 0: return spec_breath_fire		( ch );
    case 1:
    case 2: return spec_breath_lightning	( ch );
    case 3: return spec_breath_gas		( ch );
    case 4: return spec_breath_acid		( ch );
    case 5:
    case 6:
    case 7: return spec_breath_frost		( ch );
    }

    return FALSE;
}



bool spec_breath_acid( CHAR_DATA *ch ) {
    return dragon( ch, "acid breath" );
}

bool spec_breath_fire( CHAR_DATA *ch ) {
    return dragon( ch, "fire breath" );
}

bool spec_breath_frost( CHAR_DATA *ch ) {
    return dragon( ch, "frost breath" );
}

bool spec_breath_gas( CHAR_DATA *ch ) {
    int sn;

    if (ch->position != POS_FIGHTING) return FALSE;
    if ( ( sn = get_effect_efn( "gas breath" ) ) < 0 ) return FALSE;

    (*effect_array[sn].spell_fun) ( sn, ch->level, ch, NULL );
    return TRUE;
}

bool spec_breath_lightning( CHAR_DATA *ch ) {
    return dragon( ch, "lightning breath" );
}


bool spec_cast_adept( CHAR_DATA *ch ) {
    CHAR_DATA *victim;
    CHAR_DATA *v_next;

   /* Have we regenned enough mana? */

    if ( ch->position == POS_RESTING 
      && ch->mana > (ch->max_mana / 2) ) {
      do_stand(ch, "");
      return FALSE;
    }

   /* Do we need to rest ? */

    if ( ch->mana < 50 
      && ch->position == POS_STANDING ) {
      do_rest(ch, ""); 
    }

    if ( !IS_AWAKE(ch)) return FALSE;

    if (ch->activity == ACV_CASTING) return TRUE;
    
    if ( is_affected(ch, gafn_mute) && !is_affected(ch, gafn_vocalize)) return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next ) {
	v_next = victim->next_in_room;
	if ( victim != ch 
        && can_see( ch, victim ) 
        && number_bits( 1 ) == 0 
	&& !IS_NPC(victim) 
        && victim->level < 11)
	    break;
    }

    if ( victim == NULL) return FALSE;

    switch ( number_bits( 4 ))  {
    case 0:
        if (is_affected(victim, gafn_armor)) return FALSE;
        do_mob_cast(ch, "armor", victim);
        return TRUE;

    case 1:
        if (is_affected(victim, gafn_bless)) return FALSE;
        do_mob_cast(ch, "bless", victim);
        return TRUE;

    case 2:
        if (!is_affected(victim, gafn_blindness)) return FALSE;
        do_mob_cast(ch, "cure blindness", victim);
        return TRUE;

    case 3:
        if (victim->hit > (victim->max_hit - 5)) return FALSE;
        do_mob_cast(ch, "cure light", victim);
        return TRUE;

    case 4:
        if (is_affected(victim, gafn_poison)) return FALSE;
	do_mob_cast(ch, "cure poison", victim);
	return TRUE;

    case 5:
        if (victim->move > (victim->max_move - 10)) return FALSE;
	do_mob_cast(ch, "refresh", victim);
	return TRUE;
    }
    return FALSE;
}



bool spec_cast_cleric( CHAR_DATA *ch) {
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

   /* Have we regenned enough mana? */

    if ( ch->position == POS_RESTING 
    && ch->mana > (ch->max_mana / 2) ) {
      do_stand(ch, "");
      return FALSE;
    }

   /* Do we need to rest ? */

    if ( ch->mana < 50 
    && ch->position == POS_STANDING ) {
      do_rest(ch, ""); 
    }

    if ( ch->position != POS_FIGHTING) return FALSE;
	
    if (ch->activity == ACV_CASTING) return TRUE;

    if ( is_affected(ch, gafn_mute) && !is_affected(ch, gafn_vocalize)) return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next ) {
	v_next = victim->next_in_room;
        if ( victim->fighting == ch && number_bits( 2 ) == 0) break;
    }

    if ( victim == NULL ) return FALSE;

    for ( ;; ) {
	int min_level;

	switch ( number_bits( 4 ) ) {
	
	  case  0: min_level =  0; spell = "blindness";      break;
	  case  1: min_level =  3; spell = "cause serious";  break;
	  case  2: min_level =  7; spell = "mute";	     break;
	  case  3: min_level =  9; spell = "cause critical"; break;
	  case  4: min_level = 10; spell = "dispel evil";    break;
	  case  5: min_level = 12; spell = "curse";          break;
	  case  6: min_level = 12; spell = "change sex";     break;
	  case  7: min_level = 13; spell = "flamestrike";    break;
	  case  8:
	  case  9:
	  case 10: min_level = 15; spell = "harm";           break;
	  case 11: min_level = 15; spell = "plague";	     break;
	  case 12:
	  case 13:
	  case 14: min_level = 34; spell = (char*)( (IS_EVIL(ch) ? "demonfire" : "exorcism") );  break;
	  default: min_level = 16; spell = "dispel magic";   break;
	}

	if ( ch->hit < (ch->max_hit/5) 
        && ch->level > 13) { 
	    spell = "cure critical";
	    victim = ch;
            min_level = 14;
	}

        if ( ch->level >= min_level) break;
    }
		
    sn = get_spell_spn(spell);

    if (sn == SPELL_UNDEFINED)return FALSE;

    if (ch->mana < spell_array[sn].mana) return FALSE;

    switch (spell_array[sn].target) {
      case TAR_CHAR_OFFENSIVE:
        do_mob_cast(ch, spell, victim);
        break;

      case TAR_OBJ_INV:
        bug("Mob trying to cast TAR_OBJ_INV", 0);
        break;

      default:
        do_cast(ch, spell);
    }

    return TRUE;
}


bool spec_cast_nature(CHAR_DATA *ch) {
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

   /* Have we regenned enough mana? */

    if ( ch->position == POS_RESTING 
    && ch->mana > (ch->max_mana / 2) ) {
      do_stand(ch, "");
      return FALSE;
    }

   /* Do we need to rest ? */

    if ( ch->mana < 50 
    && ch->position == POS_STANDING ) {
      do_rest(ch, ""); 
    }

    if ( ch->position != POS_FIGHTING) return FALSE;
	
    if (ch->activity == ACV_CASTING) return TRUE;

    if ( is_affected(ch, gafn_mute) && !is_affected(ch, gafn_vocalize)) return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next ) {
	v_next = victim->next_in_room;
        if ( victim->fighting == ch && number_bits( 2 ) == 0) break;
    }

    if ( victim == NULL ) return FALSE;

    for ( ;; ) {
	int min_level;

	switch ( number_bits( 4 ) ) {
	
	  case  0: min_level =  0; spell = "blindness";      break;
          case  1: min_level =  3; spell = "armor";          break;
          case  2: min_level =  5; spell = "faerie fire";    break;
          case  3: min_level = 10; spell = "gnawing hunger"; break;
          case  4: min_level = 15; spell = "burning thirst"; break;
          case  5: min_level = 20; spell = "burning hands";  break;
          case  6: min_level = 25; spell = "stone skin";     break;
          case  7: min_level = 30; spell = "lightning bolt"; break;
          case  8: min_level = 35; spell = "frost shield";   break;
          case  9: min_level = 40; spell = "acid breath";    break;
          case 10: min_level = 15; spell = "poison";         break;
          case 11: min_level = 25; spell = "plague";         break;
          case 12: min_level = 35; spell = "summon familier";break;
          default: min_level =  0; spell = "cure light";     break;
	}

	if ( ch->hit < (ch->max_hit/5) 
        && ch->level > 13) { 
	    spell = "cure critical";
	    victim = ch;
            min_level = 15;
	}

        if ( ch->level >= min_level) break;
    }
		
    sn = get_spell_spn(spell);

    if (sn == SPELL_UNDEFINED)return FALSE;

    if (ch->mana < spell_array[sn].mana) return FALSE;

    switch (spell_array[sn].target) {
      case TAR_CHAR_OFFENSIVE:
        do_mob_cast(ch, spell, victim);
        break;

      case TAR_OBJ_INV:
        bug("Mob trying to cast TAR_OBJ_INV", 0);
        break;

      default:
        do_cast(ch, spell);
    }

    return TRUE;
}



bool spec_cast_power(CHAR_DATA *ch) {
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

   /* Have we regenned enough mana? */

    if ( ch->position == POS_RESTING 
    && ch->mana > (ch->max_mana / 2) ) {
      do_stand(ch, "");
      return FALSE;
    }

   /* Do we need to rest ? */

    if ( ch->mana < 50 
    && ch->position == POS_STANDING ) {
      do_rest(ch, ""); 
    }

    if ( ch->position != POS_FIGHTING) return FALSE;
	
    if (ch->activity == ACV_CASTING) return TRUE;

    if ( is_affected(ch, gafn_mute) && !is_affected(ch, gafn_vocalize)) return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next ) {
	v_next = victim->next_in_room;
        if ( victim->fighting == ch && number_bits( 2 ) == 0) break;
    }

    if ( victim == NULL ) return FALSE;

    for ( ;; ) {
	int min_level;

	switch ( number_bits( 4 ) ) {
	
          case  0: min_level =  0; spell = "shield";           break;
          case  1: min_level =  3; spell = "shocking grasp";   break;
          case  2: min_level =  5; spell = "invis";            break;
          case  3: min_level = 10; spell = "haste";            break;
          case  4: min_level = 12; spell = "slow";             break;
          case  5: min_level = 18; spell = "energy drain";     break;
          case  6: min_level = 23; spell = "regeneration";     break;
          case  7: min_level = 26; spell = "colour spray";     break;
          case  8: min_level = 30; spell = "chill touch";      break;
          case  9: min_level = 35; spell = "absorb magic";     break;
          case 10: min_level = 40; spell = "aura";             break;
          case 11: min_level = 45; spell = "age";              break;
          case 12: min_level = 50; spell = "fist of azathoth"; break;
          default: min_level = 0; spell = "cure light";        break;
	}

	if ( ch->hit < (ch->max_hit/5) 
        && ch->level > 13) { 
	    spell = "cure critical";
	    victim = ch;
            min_level = 15;
	}

        if ( ch->level >= min_level) break;
    }
		
    sn = get_spell_spn(spell);

    if (sn == SPELL_UNDEFINED)return FALSE;

    if (ch->mana < spell_array[sn].mana) return FALSE;

    switch (spell_array[sn].target) {
      case TAR_CHAR_OFFENSIVE:
        do_mob_cast(ch, spell, victim);
        break;

      case TAR_OBJ_INV:
        bug("Mob trying to cast TAR_OBJ_INV", 0);
        break;

      default:
        do_cast(ch, spell);
    }

    return TRUE;
}

bool spec_cast_judge(CHAR_DATA *ch) {
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;
 
   /* Have we regenned enough mana? */

    if ( ch->position == POS_RESTING 
      && ch->mana > (ch->max_mana / 2) ) {
      do_stand(ch, "");
      return FALSE;
    }

   /* Do we need to rest ? */

    if ( ch->mana < 50 
      && ch->position == POS_STANDING ) {
      do_rest(ch, ""); 
    }

    if ( ch->position != POS_FIGHTING )
        return FALSE;
	
    if ( is_affected(ch, gafn_mute) &&
        !is_affected(ch, gafn_vocalize) )
        return FALSE;
 
    if (ch->activity == ACV_CASTING) {
      return TRUE;
    }

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
        v_next = victim->next_in_room;
        if ( victim->fighting == ch && number_bits( 2 ) == 0 )
            break;
    }
 
    if ( victim == NULL )
        return FALSE;
 
    spell = "high explosive";

    sn = get_spell_spn( spell );

    if (sn == SPELL_UNDEFINED)
	return FALSE;

    if (ch->mana < spell_array[sn].mana) {
        return FALSE;
    }

    switch (spell_array[sn].target) {
      case TAR_CHAR_OFFENSIVE:
        do_mob_cast(ch, spell, victim);
        break;

      case TAR_OBJ_INV:
        bug("Mob trying to cast TAR_OBJ_INV", 0);
        break;

      default:
        do_cast(ch, spell);
    }

    return TRUE;
}



bool spec_cast_mage( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

   /* Have we regenned enough mana? */

    if ( ch->position == POS_RESTING 
      && ch->mana > (ch->max_mana / 2) ) {
      do_stand(ch, "");
      return FALSE;
    }

   /* Do we need to rest ? */

    if ( ch->mana < 50 
      && ch->position == POS_STANDING ) {
      do_rest(ch, ""); 
    }

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    if ( is_affected(ch, gafn_mute) 
      && !is_affected(ch, gafn_vocalize) )
	return FALSE;

    if (ch->activity == ACV_CASTING) {
      return TRUE;
    }

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "blindness";      break;
	case  1: min_level =  3; spell = "chill touch";    break;
	case  2: min_level =  7; spell = "weaken";         break;
	case  3: min_level =  8; spell = "teleport";       break;
	case  4: min_level = 11; spell = "colour spray";   break;
	case  5: min_level = 12; spell = "change sex";     break;
	case  6: min_level = 13; spell = "energy drain";   break;
	case  7:
	case  8:
	case  9: min_level = 15; spell = "fireball";       break;
	case 10: min_level = 20; spell = "plague";	   break;
	default: min_level = 20; spell = "acid blast";     break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    sn = get_spell_spn( spell );

    if (sn == SPELL_UNDEFINED)
	return FALSE;

    if (ch->mana < spell_array[sn].mana) {
        return FALSE;
    }

    switch (spell_array[sn].target) {
      case TAR_CHAR_OFFENSIVE:
        do_mob_cast(ch, spell, victim);
        break;

      case TAR_OBJ_INV:
        bug("Mob trying to cast TAR_OBJ_INV", 0);
        break;

      default:
        do_cast(ch, spell);
    }

    return TRUE;
}


bool spec_cast_undead( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;
	
   /* Have we regenned enough mana? */

    if ( ch->position == POS_RESTING 
      && ch->mana > (ch->max_mana / 2) ) {
      do_stand(ch, "");
      return FALSE;
    }

   /* Do we need to rest ? */

    if ( ch->mana < 50 
      && ch->position == POS_STANDING ) {
      do_rest(ch, ""); 
    }

    if ( ch->position != POS_FIGHTING )
	return FALSE;
	
    if ( is_affected(ch, gafn_mute) &&
        !is_affected(ch, gafn_vocalize) )
	return FALSE;

    if (ch->activity == ACV_CASTING) {
      return TRUE;
    }

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "curse";          break;
	case  1: min_level =  3; spell = "weaken";         break;
	case  2: min_level =  6; spell = "chill touch";    break;
	case  3: min_level =  9; spell = "blindness";      break;
	case  4: min_level = 12; spell = "poison";         break;
	case  5: min_level = 15; spell = "energy drain";   break;
	case  6: min_level = 18; spell = "harm";           break;
	case  7: min_level = 21; spell = "teleport";       break;
	case  8: min_level = 20; spell = "plague";	   break;
	default: min_level = 18; spell = "harm";           break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    sn = get_spell_spn( spell );

    if (sn == SPELL_UNDEFINED)
	return FALSE;

    if (ch->mana < spell_array[sn].mana) {
        return FALSE;
    }

    switch (spell_array[sn].target) {
      case TAR_CHAR_OFFENSIVE:
        do_mob_cast(ch, spell, victim);
        break;

      case TAR_OBJ_INV:
        bug("Mob trying to cast TAR_OBJ_INV", 0);
        break;

      default:
        do_cast(ch, spell);
    }

    return TRUE;
}


bool spec_executioner( CHAR_DATA *ch ) {
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *crime;

    if ( !IS_AWAKE(ch) || ch->fighting != NULL )
	return FALSE;

    crime = "";
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )    {
	v_next = victim->next_in_room;

	if (!IS_NPC(victim) && IS_SET(victim->plr, PLR_ANNOYING)) {
                     crime = "annoying";
                     break;

                }else if (!IS_NPC(victim) && IS_SET(victim->act, ACT_CRIMINAL)) {
                     crime = "a criminal";
                     break;
                }
    }

    if ( victim == NULL ) return FALSE;

    sprintf( buf, "%s is %s!  Protect the Innocent!  More Blood!", victim->name, crime );
    do_yell( ch, buf );
    multi_hit( ch, victim, TYPE_UNDEFINED );

//    char_to_room( create_mobile( get_mob_index(MOB_VNUM_CITYGUARD) ), ch->in_room );
//    char_to_room( create_mobile( get_mob_index(MOB_VNUM_CITYGUARD) ), ch->in_room );
    return TRUE;
}

/* A procedure for Puff the Fractal Dragon--> it gives her an attitude.
	Note that though this procedure may make Puff look busy, she in
	fact does nothing quite more often than she did in Merc 1.0;
	due to null victim traps, my own do-nothing options, and various ways
	to return without doing much, Puff is... well, not as BAD of a gadfly
	as she may look, I assure you.  But I think she's fun this way ;)

	(btw--- should you ever want to test out your socials, just tweak
	the percentage table ('silliness') to make her do lots of socials,
	and then go to a quiet room and load up about thirty Puffs... ;) 
			
		written by Seth of Rivers of Mud         */
			
bool spec_puff( CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];
	int rnd_social, spn, silliness;
	bool pc_found = TRUE;
    CHAR_DATA *v_next;
    CHAR_DATA *wch;
    CHAR_DATA *wch_next;
    CHAR_DATA *nch;
    CHAR_DATA *ch_next;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    CHAR_DATA *victim;
	extern int social_count;
 	
	if ( !IS_AWAKE(ch) )
		return FALSE;

	victim = NULL;
  
/* Here's Furey's aggress routine, with some surgery done to it.  
  	All it does is pick a potential victim for a social.  
  	(Thank you, Furey-- I screwed this up many times until I
  	learned of your way of doing it)                      */
  		
    for ( wch = char_list; wch != NULL; wch = wch_next )
    {
	wch_next = wch->next;
	if ( IS_NPC(wch)
	||   wch->in_room == NULL )
	    continue;

	for ( nch = wch->in_room->people; nch != NULL; nch = ch_next )
	{
	    int count;

	    ch_next	= nch->next_in_room;

	    if ( !IS_NPC(nch) 
	    ||   number_bits(1) == 0)
		continue;

	    /*
	     * Ok we have a 'wch' player character and a 'nch' npc aggressor.
	     * Now make the aggressor fight a RANDOM pc victim in the room,
	     *   giving each 'vch' an equal chance of selection.
	     */
	    count	= 0;
	    victim	= NULL;
	    for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
	    {
		vch_next = vch->next_in_room;

		if ( !IS_NPC(vch) )
		{
		    if ( number_range( 0, count ) == 0 )
			victim = vch;
		    count++;
		}
	    }

	    if (victim == NULL)
			return FALSE;
	}

	}
		rnd_social = (number_range (0, ( social_count - 1)) );
			
	/* Choose some manner of silliness to perpetrate.  */
	
	silliness = number_range (1, 100);
		
	if ( silliness <= 20)
		return TRUE;
	else if ( silliness <= 30)
		{
		sprintf( buf, "Tongue-tied and twisted, just an earthbound misfit, ..."); 
		do_say ( ch, buf);
		}
	else if ( silliness <= 40)
		{
		 sprintf( buf, "The colors, the colors!");
		do_say ( ch, buf);
		}
	else if ( silliness <= 55)
		{
		sprintf( buf, "Did you know that I'm written in C?");
		do_say ( ch, buf);
		}
	else if ( silliness <= 75)
		{
		act( social_table[rnd_social].others_no_arg, 
			ch, NULL, NULL, TO_ROOM    );
		act( social_table[rnd_social].char_no_arg,   
			ch, NULL, NULL, TO_CHAR    );
		}
	else if ( silliness <= 85)
		{		
		if ( (!pc_found)
		|| 	 (victim != ch->in_room->people) ) 
			return FALSE;
		act( social_table[rnd_social].others_found, 
			 ch, NULL, victim, TO_NOTVICT );
		act( social_table[rnd_social].char_found,  
			 ch, NULL, victim, TO_CHAR    );
		act( social_table[rnd_social].vict_found, 
			 ch, NULL, victim, TO_VICT    );
		}
		
	else if ( silliness <= 97)	
		{	
		act( "For a moment, $n flickers and phases.", 
			ch, NULL, NULL, TO_ROOM );
		act( "For a moment, you flicker and phase.", 
			ch, NULL, NULL, TO_CHAR );
		}
	
/* The Fractal Dragon sometimes teleports herself around, to check out
	new and stranger things.  HOWEVER, to stave off some possible Puff
	repop problems, and to make it possible to play her as a mob without
	teleporting helplessly, Puff does NOT teleport if she's in Limbo,
	OR if she's not fighting or standing.  If you're playing Puff and 
	you want to talk with someone, just rest or sit!
*/
	
	else{
		if (ch->position < POS_FIGHTING)
			{
			act( "For a moment, $n seems lucid...", 
				ch, NULL, NULL, TO_ROOM );
			act( "   ...but then $e returns to $s contemplations once again.", 
				ch, NULL, NULL, TO_ROOM );
			act( "For a moment, the world's mathematical beauty is lost to you!",
				ch, NULL, NULL, TO_CHAR );
			act( "   ...but joy! yet another novel phenomenon seizes your attention.", 
				ch, NULL, NULL, TO_CHAR);
			return TRUE;
			}
		if ( ( spn = get_effect_efn( "teleport" ) ) < 0 )
			return FALSE;
    	(*effect_array[spn].spell_fun) ( spn, ch->level, ch, ch );
 		}


/* Puff has only one spell, and it's the most annoying one, of course.
  	(excepting energy drain, natch)  But to a bemused mathematician,
  	what could possibly be a better resolution to conflict? ;) 
  	Oh-- and notice that Puff casts her one spell VERY well.     */
  			
	if ( ch->position != POS_FIGHTING )
		return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( 2 ) == 0 )
	   	break;
    }

    if ( victim == NULL )
		return FALSE;

    if ( ( spn = get_effect_efn( "teleport" ) ) < 0 )
		return FALSE;
    (*effect_array[spn].spell_fun) ( spn, 50, ch, victim );
    	return TRUE;

}

bool spec_fido( CHAR_DATA *ch )
{
    OBJ_DATA *corpse;
    OBJ_DATA *c_next;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( corpse = ch->in_room->contents; corpse != NULL; corpse = c_next )
    {
	c_next = corpse->next_content;
	if ( corpse->item_type != ITEM_CORPSE_NPC )
	    continue;

	act( "$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM );
	for ( obj = corpse->contains; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    obj_from_obj( obj );
	    obj_to_room( obj, ch->in_room );
	}
	extract_obj( corpse );
	return TRUE;
    }

    return FALSE;
}



bool spec_guard( CHAR_DATA *ch ) {
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    CHAR_DATA *ech;
    char *crime;
    int max_evil;

    if ( !IS_AWAKE(ch) || ch->fighting != NULL )
	return FALSE;

    max_evil = 300;
    ech      = NULL;
    crime    = "";

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next ) {
	v_next = victim->next_in_room;

	if ( !IS_NPC(victim) && IS_SET(victim->plr, PLR_ANNOYING)) {
                     crime = "annoying";
                     break;

                }else if (!IS_NPC(victim) && IS_SET(victim->act, ACT_CRIMINAL)) {
                     crime = "a criminal";
                     break;
                }

	if ( victim->fighting != NULL
	&&   victim->fighting != ch
	&&   victim->alignment < max_evil )	{
	    max_evil = victim->alignment;
	    ech      = victim;
	}
    }

    if ( victim != NULL ) {
	sprintf( buf, "%s is %s!  Protect the Innocent!", victim->name, crime );
	do_yell( ch, buf );
	multi_hit( ch, victim, TYPE_UNDEFINED );
	return TRUE;
    }

    if ( ech != NULL ) {
	act( "$n screams 'Protect the Innocent!", ch, NULL, NULL, TO_ROOM );
	multi_hit( ch, ech, TYPE_UNDEFINED );
	return TRUE;
    }

    return FALSE;
}


bool spec_janitor( CHAR_DATA *ch ) {
    OBJ_DATA *trash;
    OBJ_DATA *trash_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( trash = ch->in_room->contents; trash != NULL; trash = trash_next )
    {
	trash_next = trash->next_content;
	if ( !IS_SET( trash->wear_flags, ITEM_TAKE ) || !can_loot(ch,trash))
	    continue;
	if ( trash->item_type == ITEM_DRINK_CON
	||   trash->item_type == ITEM_TRASH
	||   trash->cost < 10 )
	{
	    act( "$n picks up some trash.", ch, NULL, NULL, TO_ROOM );
	    obj_from_room( trash );
	    obj_to_char( trash, ch );
	    return TRUE;
	}
    }

    return FALSE;
}



bool spec_mayor( CHAR_DATA *ch ){
    static const char open_path[] = "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";
    static const char close_path[] = "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

    static const char *path;
    static int pos;
    static bool move;

    if ( !move )    {
	if ( time_info.hour ==  6 )	{
	    path = open_path;
	    move = TRUE;
	    pos  = 0;
	}

	if ( time_info.hour == 20 )	{
	    path = close_path;
	    move = TRUE;
	    pos  = 0;
	}
    }

    if ( ch->fighting != NULL ) return spec_cast_cleric( ch );
    if ( !move || ch->position < POS_SLEEPING ) return FALSE;

    switch ( path[pos] )    {
    case '0':
    case '1':
    case '2':
    case '3':
	move_char( ch, path[pos] - '0', FALSE );
	break;

    case 'W':
	set_activity( ch, POS_STANDING, NULL, ACV_NONE, NULL);
	act( "$n awakens and groans loudly.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'S':
	set_activity( ch, POS_STANDING, NULL, ACV_NONE, NULL);
	act( "$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'a':
	act( "$n says 'Hello Honey!'", ch, NULL, NULL, TO_ROOM );
	break;

    case 'b':
	act( "$n says 'What a view!  I must do something about that dump!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'c':
	act( "$n says 'Vandals!  Youngsters have no respect for anything!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'd':
	act( "$n says 'Good day, citizens!'", ch, NULL, NULL, TO_ROOM );
	break;

    case 'e':
	act( "$n says 'I hereby declare the city of Midgaard open!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'E':
	act( "$n says 'I hereby declare the city of Midgaard closed!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'O':
/*	do_unlock( ch, "gate" ); */
	do_open( ch, "gate" );
	break;

    case 'C':
	do_close( ch, "gate" );
/*	do_lock( ch, "gate" ); */
	break;

    case '.' :
	move = FALSE;
	break;
    }

    pos++;
    return FALSE;
}



bool spec_plague( CHAR_DATA *ch ) {
    CHAR_DATA *victim;

    int efn;

    if ( ch->position != POS_FIGHTING
    || ( victim = ch->fighting ) == NULL
    ||   number_percent( ) > 2 * ch->level ) {
	return FALSE;
    } 

    act( "You coughs at $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n coughs at $N!",  ch, NULL, victim, TO_NOTVICT );
    act( "$n coughs at you!", ch, NULL, victim, TO_VICT    );

   /* Find the spell effect... */

    efn = get_effect_efn( "plague" );

    if ( efn <= 0 ) {
      bug("Effect 'plague' not defined - spec_plague", 0);
      return FALSE;
    }

   /* Go use it... */ 

    do_dragon_cast( efn, ch->level + 5, ch, victim );

   /* Ok, done something... */

    return TRUE;
}

bool spec_poison( CHAR_DATA *ch ) {
    CHAR_DATA *victim;

    int efn;

    if ( ch->position != POS_FIGHTING
    || ( victim = ch->fighting ) == NULL
    ||   number_percent( ) > 2 * ch->level ) {
	return FALSE;
    } 

    act( "You bite $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n bites $N!",  ch, NULL, victim, TO_NOTVICT );
    act( "$n bites you!", ch, NULL, victim, TO_VICT    );

   /* Find the spell effect... */

    efn = get_effect_efn( "poison" );

    if ( efn <= 0 ) {
      bug("Effect 'poison' not defined - spec_poison", 0);
      return FALSE;
    }

   /* Go use it... */ 

    do_dragon_cast( efn, ch->level + 5, ch, victim );

   /* Ok, done something... */

    return TRUE;
}



bool spec_thief( CHAR_DATA *ch ) {
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    long gold;
    int currency = find_currency(ch);

    if ( ch->position != POS_STANDING )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( IS_NPC(victim)
	||   IS_IMMORTAL(victim)
	||   number_bits( 5 ) != 0 
	||   !can_see(ch,victim))
	    continue;

	if ( IS_AWAKE(victim) && number_range( 0, ch->level ) == 0 )
	{
	    act( "You discover $n's hands in your wallet!",
		ch, NULL, victim, TO_VICT );
	    act( "$N discovers $n's hands in $S wallet!",
		ch, NULL, victim, TO_NOTVICT );
	    return TRUE;
	}
	else
	{
	    gold = victim->gold[currency] * UMIN(number_range( 1, 20 ),ch->level) / 100;
	    gold = UMIN(gold, ch->level * ch->level * 20 );
	    ch->gold[currency]     += gold;
	    victim->gold[currency] -= gold;
	    return TRUE;
	}
    }

    return FALSE;
}


void do_heal(CHAR_DATA *ch, char *argument) {
CHAR_DATA *mob;
char arg[MAX_INPUT_LENGTH];
int cost = 0;
int sn;
char *spell = NULL;
char *words;
bool special;	
int currency;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

    if (IS_NEWBIE(ch)) currency = 0;
    else currency = find_currency(ch);

    /* check for healer */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )    {
        if ( IS_NPC(mob) && IS_SET(mob->act, ACT_IS_HEALER) )
            break;
    }
 
    if ( mob == NULL )    {
        send_to_char( "You can't do that here.\r\n", ch );
        return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')  {
        /* display price list */
	act("$N says 'I offer the following spells:'",ch,NULL,mob,TO_CHAR);
                if (IS_NEWBIE(ch)) {
   	    sprintf_to_char(ch, "  light  : cure light wounds       {y    6{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  serious: cure serious wounds     {y   10{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  critic : cure critical wounds    {y   20{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  heal   : healing spell           {y  200{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  blind  : cure blindness          {y   60{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  disease: cure disease            {y  200{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  poison :  cure poison            {y   60{x %s\r\n", flag_string(currency_type, currency)); 
	    sprintf_to_char(ch, "  uncurse: remove curse            {y  400{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  dispel : removes affects         {y  400{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  refresh: restore movement        {y  100{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  mana   :  restore mana           {y  200{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  aid    : bandage                 {y  300{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  {gfull{x   : all mana, hp, movement  {y10000{x %s\r\n", flag_string(currency_type, currency));
                } else {
   	    sprintf_to_char(ch, "  light  : cure light wounds       {y   30{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  serious: cure serious wounds     {y   50{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  critic : cure critical wounds    {y  100{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  heal   : healing spell           {y 1000{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  blind  : cure blindness          {y  300{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  disease: cure disease            {y 1000{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  poison :  cure poison            {y  300{x %s\r\n", flag_string(currency_type, currency)); 
	    sprintf_to_char(ch, "  uncurse: remove curse            {y 2000{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  dispel : removes affects         {y 2000{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  refresh: restore movement        {y  500{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  mana   :  restore mana           {y 1000{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  aid    : bandage                 {y 1500{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  {gfull{x   : all mana, hp, movement  {y50000{x %s\r\n", flag_string(currency_type, currency));
                }
	send_to_char(" Type heal <type> to be healed.\r\n",ch);
	return;
    }

    if (!can_see(mob, ch)) do_say(mob, "Who's there?");

    special = FALSE;
    words = "";
 
    sn = SPELL_UNDEFINED;

    switch (arg[0])    {
	case 'l' :
	    spell = "cure light";
	    cost  = 30;
	    break;

	case 's' :
	    spell = "cure serious";
	    cost  = 50;
	    break;

	case 'c' :
	    spell = "cure critical";
	    cost  = 100;
	    break;

	case 'h' :
	    spell = "heal";
	    cost  = 1000;
	    break;

	case 'b' :
	    spell = "cure blindness";
                    cost  = 300;
	    break;

    	case 'd' :
                    if (!str_cmp(arg, "disease")) {
  	        spell = "cure disease";
	        cost = 1000;
                    } else if (!str_cmp(arg, "dispel")) {
  	        spell = "cancellation";
	        cost = 2000;
                    } else {
  	        act("$N says 'Type 'heal' for a list of spells.'", ch,NULL,mob,TO_CHAR);
	        return;
                    }
	    break;

	case 'p' :
	    spell = "cure poison";
	    cost  = 300;
	    break;
	
	case 'u' :
	    spell = "remove curse"; 
	    cost  = 2000;
	    break;

	case 'r' :
	    spell = "refresh";
	    cost  = 500;
	    break;

	case 'a' :
	    spell = "bandage";
	    cost  = 1500;
                    special = TRUE;
	    break;

	case 'm' :
	    spell = "mana";
	    words = "ka-ro-kai hi";
	    cost = 1000;
                    special = TRUE;
	    break;
	
	case 'f' :
	    spell = "full";
	    words = "{rSim{gsal{ybim{wbam{cba {bsal{ga{ydu {Gsal{Ya{Rdim{m!{x";
	    cost = 50000;
                    special = TRUE;
	    break;

	default :
	    act("$N says 'Type 'heal' for a list of spells.'", ch,NULL,mob,TO_CHAR);
	    return;
    }

    if (IS_NEWBIE(ch)) cost /=5;

    if (cost > ch->gold[currency])    {
	act("$N says 'You do not have enough money for my services.'", ch,NULL,mob,TO_CHAR);
	return;
    }

    if (!special) {
      sn = get_spell_spn( spell );

      if (sn == SPELL_UNDEFINED) {
        act("$n looks rather worried.", mob, NULL, words, TO_ROOM);
        do_say(mob, "I...I have forgotten how!");
        return;
      }

      if (mob->mana < spell_array[sn].mana) {
        do_say(mob, "Sorry, I am to tired at the moment, let me rest.");
        return;
      }
    } else {
      if (mob->mana < 75) {
        do_say(mob, "Sorry, I an to tired at the moment, let me rest.");
        return;
      }
    }

    WAIT_STATE(ch,PULSE_VIOLENCE);

    mcc = get_mcc(ch, ch, mob, NULL, NULL, NULL, cost, spell);
    wev = get_wev(WEV_HEAL, WEV_HEAL_HEALER, mcc, NULL, NULL, NULL);

    if (!room_issue_wev_challange(ch->in_room, wev)) {
        free_wev(wev);
        return;
    }

    room_issue_wev( ch->in_room, wev);
    free_wev(wev);

    ch->gold[currency] -= cost;
    mob->gold[currency] += cost;

    if (!strcmp(spell, "mana")) {
        act("$n chants '$T'.",mob,NULL,words,TO_ROOM);
        mob->mana -= 50;

        ch->mana += dice(100,2) + mob->level / 4;
        ch->mana = UMIN(ch->mana,ch->max_mana);

        send_to_char("A warm glow passes through you.\r\n",ch);
        return;

    } else if (!strcmp(spell, "bandage")) {
        act("$n bandages $N.",mob,NULL,ch,TO_ROOM);
        mob->mana -= 100;

        if (IS_SET(ch->form, FORM_BLEEDING)) {
            REMOVE_BIT(ch->form, FORM_BLEEDING);
            send_to_char("The bleeding stops.\r\n",ch);
        } 

        if (ch->limb[0] > 0) ch->limb[0] -=1;
        if (ch->limb[1] > 0) ch->limb[1] -=1;
        if (ch->limb[2] > 0) ch->limb[2] -=1;
        if (ch->limb[3] > 0) ch->limb[3] -=1;
        if (ch->limb[4] > 0) ch->limb[4] -=1;
        if (ch->limb[5] > 0) ch->limb[5] -=1;
        return;

    } else if (!strcmp(spell, "full")) {
        act("$n chants '$T'.",mob,NULL,words,TO_ROOM);
        mob->mana -= 75;

        ch->mana = ch->max_mana;
        ch->hit = ch->max_hit;
        ch->move = ch->max_move;

        send_to_char("The {gpower{x of {YMarduk{x flows into you!\r\n",ch);
        return;
    }

    switch (spell_array[sn].target) {
      case TAR_CHAR_OFFENSIVE:
      case TAR_CHAR_DEFENSIVE:
        do_mob_cast(mob, spell, ch);
        break;

      case TAR_OBJ_INV:
        bug("Mob trying to cast TAR_OBJ_INV", 0);
        break;

      default:
        do_cast(mob, spell);
    }

    return; 
}


#define TR_EXERCISE			0
#define TR_HYPNOSES			1
#define TR_AVERSION			2
#define TR_ELIXIR			3
#define TR_TALK			4
#define TR_EXORCISM			5
#define TR_SHOCK_CONFIRM		6
#define TR_SHOCK			7
#define TR_LOBO_CONFIRM		8
#define TR_LOBO			9
#define TR_DESENS			10

static int tr_costs[] = {200, 400, 600, 800, 1000, 2000, 0, 4000, 0, 10000, 1500};

void do_therapy(CHAR_DATA *ch, char *argument) {
CHAR_DATA *mob;
char arg[MAX_INPUT_LENGTH];
int tr, roll, sn;
AFFECT_DATA af;
int currency, cost;
MOB_CMD_CONTEXT *mcc;
WEV *wev;
    
    if (IS_NEWBIE(ch)) currency = 0;
    else currency = find_currency(ch);
            
    /* check for alienist */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room ) {
        if ( IS_NPC(mob) && IS_SET(mob->act, ACT_IS_ALIENIST)) break;
    }
 
    if ( mob == NULL ) {
        send_to_char( "You can't do that here.\r\n", ch );
        return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0') {
        /* display price list */
	act("$N says 'I offer the following treatments:'",ch,NULL,mob,TO_CHAR);
                if (IS_NEWBIE(ch)) {
  	    sprintf_to_char(ch, "  exercise                  {y   40{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  hypnoses                  {y   80{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  aversion                  {y  120{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  elixir                    {y  160{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  talk                      {y  200{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  exorcism                  {y  400{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  shock                     {y  800{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  lobotomy                  {y 2000{x %s\r\n", flag_string(currency_type, currency)); 
	    sprintf_to_char(ch, "  desensitization            {y 300{x %s\r\n", flag_string(currency_type, currency)); 
                } else {
  	    sprintf_to_char(ch, "  exercise                  {y  200{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  hypnoses                  {y  400{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  aversion                  {y  600{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  elixir                    {y  800{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  talk                      {y 1000{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  exorcism                  {y 2000{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  shock                     {y 4000{x %s\r\n", flag_string(currency_type, currency));
	    sprintf_to_char(ch, "  lobotomy                  {y10000{x %s\r\n", flag_string(currency_type, currency)); 
	    sprintf_to_char(ch, "  desensitization            {y1500{x %s\r\n", flag_string(currency_type, currency)); 
                }
	send_to_char(" Type 'therapy <treatment>' to be healed.\r\n",ch);
	send_to_char(" All therapy is at your own risk.\r\n",ch);
	send_to_char(" Results cannot be guarenteed.\r\n",ch);
	send_to_char(" No refunds given if anything goes wrong.\r\n",ch);
	return;
    }

   /* What to they want? */ 
  
    if (!str_cmp(arg, "exercise")) {
       tr = TR_EXERCISE;
    } else if (!str_cmp(arg, "desensitization")) {
       tr = TR_DESENS;
    } else if (!str_cmp(arg, "hypnoses")) {
       tr = TR_HYPNOSES;
    } else if (!str_cmp(arg, "aversion")) {
       tr = TR_AVERSION;
    } else if (!str_cmp(arg, "talk")) {
       tr = TR_TALK;
    } else if (!str_cmp(arg, "elixir")) {
       tr = TR_ELIXIR;
    } else if (!str_cmp(arg, "exorcism")) {
       tr = TR_EXORCISM;
    } else if (!str_cmp(arg, "shock")) {
       tr = TR_SHOCK_CONFIRM;
    } else if (!str_cmp(arg, "shock_confirm")) {
       tr = TR_SHOCK;
    } else if (!str_cmp(arg, "lobotomy")) {
              if (IS_SET(ch->act,ACT_BRAINSUCKED)) {
                    send_to_char(" Your brain is too damaged for an effective lobotomy.\r\n",ch);
	    return;
               } else {
                    tr = TR_LOBO_CONFIRM;
               }
    } else if (!str_cmp(arg, "lobotomy_confirm")) {
       tr = TR_LOBO;
    } else { 
       act("$N says 'Type 'therapy' for a list of treatments.'", ch, NULL, mob, TO_CHAR);
       return;
    }

    if (get_sanity(ch) >100 && tr != TR_DESENS) {
	send_to_char("You are completely sane.\r\n",ch);
	return;
    }

   /* Can they afford it? */

    cost = tr_costs[tr];
    if (IS_NEWBIE(ch)) cost /=5;

    if (cost > ch->gold[currency]) {
	act("$N says 'You do not have enough money for my services.'", ch, NULL, mob, TO_CHAR);
	return;
    }

    mcc = get_mcc(ch, ch, mob, NULL, NULL, NULL, tr_costs[tr], arg);
    wev = get_wev(WEV_HEAL, WEV_HEAL_THERAPY, mcc, NULL, NULL, NULL);

    if (!room_issue_wev_challange(ch->in_room, wev)) {
        free_wev(wev);
        return;
    }

    room_issue_wev( ch->in_room, wev);
    free_wev(wev);

    ch->gold[currency] -= cost;
    mob->gold[currency] += cost;

    switch (tr) {

      case TR_DESENS:
	act("$N tells you to imagine a sweet and cuddly shuggoth...", ch,NULL,mob,TO_CHAR);
	act("$N and $n talk about shuggoths...", ch,NULL,mob,TO_NOTVICT);

                sn = get_spell_spn("remove fear");
                if (sn == SPELL_UNDEFINED) {
                     act("$n looks rather worried.", mob, NULL, NULL, TO_ROOM);
                     do_say(mob, "I...I have forgotten how!");
                     return;
                }

                WAIT_STATE(ch,PULSE_VIOLENCE);
                do_mob_cast(mob, "remove fear", ch);
                break;


      case TR_EXERCISE:
	act("$N makes you do lots of physical exercise...", ch,NULL,mob,TO_CHAR);
	act("$N makes $n do lots of physical exercise...", ch,NULL,mob,TO_NOTVICT);
        ch->sanity += dice(1,4);

        if ( (number_open() + get_curr_stat(ch, STAT_STR)) < 100 ) {

	  send_to_char("You pull a muscle...\r\n", ch);

          af.type		= SKILL_UNDEFINED;
          af.afn		= gafn_weakness;
          af.level		= ch->level;
          af.duration		= 12 + dice(2,12);
          af.bitvector		= AFF_WEAKEN;

          af.location		= APPLY_STR;
          af.modifier		= -1 * dice(2,6);
          affect_to_char( ch, &af );
        }
        break;


      case TR_HYPNOSES:
	act("$N makes you look at the shining light...", ch,NULL,mob,TO_CHAR);
	act("$N makes $n look at his pocket watch...", ch,NULL,mob,TO_NOTVICT);
                ch->sanity += dice(1,7);

                if ((number_open() + get_curr_stat(ch, STAT_INT)) < 100) {
	send_to_char("You still feel a little confused...\r\n", ch);

                af.type		= SKILL_UNDEFINED;
                af.afn		= gafn_mind_meld;
                af.level		= ch->level;
                af.duration		= 12 + dice(2,12);
                af.bitvector		= AFF_MELD;
                af.location		= APPLY_INT;
                af.modifier		= -1 * dice(2,6);
                affect_to_char( ch, &af );
        }
        break;


      case TR_AVERSION:
	act("$N shows you horrible things!", ch,NULL,mob,TO_CHAR);
	act("$N shows $n some horrible things!", ch,NULL,mob,TO_NOTVICT);
                ch->sanity += dice(1,8);

                if ((number_open() + get_sanity(ch)) < 100) {

	if (IS_SET(ch->act,ACT_BRAINSUCKED)) {
                       send_to_char("You know you should be scared...\r\n", ch);
                       send_to_char("... but somehow you aren't.\r\n", ch);
                  } else {
                       send_to_char("...they scare you!\r\n", ch);
                       af.type		= SKILL_UNDEFINED;
                       af.afn		= gafn_fear;
                       af.level		= ch->level;
                       af.duration		= 12 + dice(2,12);
                       af.bitvector		= AFF_FEAR;
                       af.location		= 0;
                       af.modifier		= 0;
                       affect_to_char( ch, &af );
                  }
                 }
                 break;

      case TR_ELIXIR:
	act("$N gives you a special scientific drink!", ch,NULL,mob,TO_CHAR);
	act("$N gives $n a special scientific drink!",ch,NULL,mob,TO_NOTVICT);
        ch->sanity += dice(1,5);

        if ((number_open() + get_sanity(ch)) < 100) {

	  send_to_char("...you feel very confident!\r\n", ch);

          af.type		= SKILL_UNDEFINED;
          af.afn		= gafn_frenzy;
          af.level		= MAX_LEVEL;
          af.duration		= 12 + dice(2,12);
          af.bitvector		= 0;

          af.location		= APPLY_SANITY;
          af.modifier		= dice(3,8);
          affect_to_char( ch, &af );
        }

        break;

      case TR_TALK:
	act("$N talks with you about your problems!", ch,NULL,mob,TO_CHAR);
	act("$N spends a long time talking to $n!", ch,NULL,mob,TO_NOTVICT);
        roll = number_open() + get_sanity(ch);

        if ( roll < 0 ) {

          check_sanity(ch, 100);

	  send_to_char("Ahhh! Noo! It can't have been that!\r\n", ch);

        } else if ( roll < 100 ) {

          ch->sanity += dice(1,9);

	  send_to_char("...you learn things you sooner you hadn't!\r\n", ch);

          af.type		= SKILL_UNDEFINED;
          af.afn		= gafn_fear;
          af.level		= ch->level;
          af.duration		= 12 + dice(2,12);
          af.bitvector		= AFF_FEAR;

          af.location		= 0;
          af.modifier		= 0;
          affect_to_char( ch, &af );

        } else {

	  send_to_char("...you feel a lot better!\r\n", ch);

          ch->sanity += dice(2,6);

        }

        break;

      case TR_EXORCISM:
	act("$N tries to exorcise your demons!", ch,NULL,mob,TO_CHAR);
	act("$N tries to exorcise $ns demons!", ch,NULL,mob,TO_NOTVICT);

        roll = number_open() + get_sanity(ch);

        if ( roll < 0 ) {

          check_sanity(ch, 100);

          ch->alignment -= 100 + 10 * dice(5,6);

          ch->alignment = URANGE(-1000, ch->alignment, 250);

	  send_to_char("Bealzebub, Lucifer, I welcome you!\r\n", ch);

        } else if ( roll < 100 ) {

          ch->sanity += dice(1,8);

	  send_to_char("...you change you way of viewing the world!\r\n", ch);

          ch->alignment += 100 + 10 * dice(5,6);

          ch->alignment = URANGE(-250, ch->alignment, 1000);

        } else {

	  send_to_char("...you feel a lot better!\r\n", ch);

          ch->sanity += dice(2,8);

        }

        break;

      case TR_SHOCK_CONFIRM:
        
	act("$N says 'Are you sure you want electro-shock therapy?'", ch,NULL,mob,TO_CHAR);
	act("$N says 'It can be very dangerous!'",  ch,NULL,mob,TO_CHAR);
	act("Type 'therapy shock_confirm' if you are", ch,NULL,mob,TO_CHAR);
        break;

      case TR_SHOCK:
	act("$N gives you electro-shock therapy!'", ch,NULL,mob,TO_CHAR);
	act("$N gives $n electro-shock therapy!'", ch,NULL,mob,TO_NOTVICT);

        do_scream(ch, "");

        roll = number_open();

        if ( roll < 100 ) {

          check_sanity(ch, 100);

          if (roll > 0) {
            ch->sanity += dice(3,8);
          } else {
	    send_to_char("You see a vision of distant dimensions...\r\n", ch);
          }

	  send_to_char("...your head hurts!\r\n", ch);

          ch->perm_stat[STAT_INT] += dice(1,6) - dice(1,8); 
          ch->perm_stat[STAT_INT] = URANGE(STAT_MINIMUM, ch->perm_stat[STAT_INT], STAT_MAXIMUM);

          ch->training[TRAIN_STAT_INT] /= 2;

          ch->perm_stat[STAT_WIS] += dice(1,6) - dice(1,8); 
          ch->perm_stat[STAT_WIS] = URANGE(STAT_MINIMUM, ch->perm_stat[STAT_WIS], STAT_MAXIMUM);
          ch->training[TRAIN_STAT_WIS] /= 2;

          af.type		= SKILL_UNDEFINED;
          af.afn		= gafn_blindness;
          af.level		= ch->level;
          af.duration		= 12 + dice(2,12);
          af.bitvector		= AFF_BLIND;

          af.location		= APPLY_WIS;
          af.modifier		= -1 * dice(2,6);
          affect_to_char( ch, &af );

          af.location		= APPLY_HITROLL;
          af.modifier		= -2 - dice(1,6);
          affect_to_char( ch, &af );

        } else {

	  send_to_char("...you feel a lot better!\r\n", ch);

          ch->sanity += 8 + dice(3,8);

        }

        break;

      case TR_LOBO_CONFIRM:
        
	act("$N says 'Are you sure you want a lobotomy?'", ch,NULL,mob,TO_CHAR);
	act("$N says 'It can have a profound impact on your life!'", ch,NULL,mob,TO_CHAR);
	act("Type 'therapy lobotomy_confirm' if you are.", ch,NULL,mob,TO_CHAR);
        break;

      case TR_LOBO:
	act("$N gives you a lobotomy!'", ch,NULL,mob,TO_CHAR);
	act("$N gives $n a lobotomy!'", ch,NULL,mob,TO_NOTVICT);

        do_scream(ch, "");

        roll = number_open();

        if ( roll < 100 ) {

          check_sanity(ch, 100);

          if (roll > 0) {
            ch->sanity += dice(4,12);
          } else {
	    send_to_char("You see a vision of bleakness...\r\n", ch);
          }

	  send_to_char("...your head hurts!\r\n", ch);

          ch->perm_stat[STAT_INT] += dice(1,6) - dice(2,10); 
          ch->perm_stat[STAT_INT] = URANGE(STAT_MINIMUM, ch->perm_stat[STAT_INT],  STAT_MAXIMUM);
          ch->training[TRAIN_STAT_INT] /= 3;
          ch->perm_stat[STAT_WIS] += dice(1,6) - dice(2,10); 
          ch->perm_stat[STAT_WIS] = URANGE(STAT_MINIMUM, ch->perm_stat[STAT_WIS], STAT_MAXIMUM);
          ch->training[TRAIN_STAT_WIS] /= 3;

          af.type		= SKILL_UNDEFINED;
          af.afn		= gafn_calm;
          af.level		= MAX_LEVEL;
          af.duration		= 12 + dice(2,12);
          af.bitvector		= AFF_CALM;

          af.location		= APPLY_WIS;
          af.modifier		= -1 * dice(2,6);
          affect_to_char( ch, &af );

          af.location		= APPLY_INT;
          af.modifier		= -1 * dice(2,6);
          affect_to_char( ch, &af );

          af.location		= APPLY_STR;
          af.modifier		= -1 * dice(2,6);
          affect_to_char( ch, &af );

          af.location		= APPLY_DEX;
          af.modifier		= -1 * dice(2,6);
          affect_to_char( ch, &af );

          af.location		= APPLY_CON;
          af.modifier		= -1 * dice(2,6);
          affect_to_char( ch, &af );

          af.location		= APPLY_HITROLL;
          af.modifier		= -1 * dice(2,6);
          affect_to_char( ch, &af );

          af.location		= APPLY_DAMROLL;
          af.modifier		= -1 * dice(2,6);
          affect_to_char( ch, &af );

          if (number_percent()<20) {
                 send_to_char("{rYour brain has taken severe damage!\r\n", ch);
                 ch->sanity += dice(4,4);
                 ch->perm_stat[STAT_INT] -= dice(2,4); 
                 ch->perm_stat[STAT_WIS] -= dice(2,4); 
                 SET_BIT(ch->act,ACT_BRAINSUCKED);
            }
        } else {
             send_to_char("...you feel a lot better!\r\n", ch);
             ch->sanity += 10 + dice(4,12);
        }

        break;

      default: 
	act("$N says 'Hmmm. I don't think I know how to do that!'", ch, NULL, mob, TO_CHAR);
        break;
    }
    return; 
}


bool spec_clanhealer( CHAR_DATA *ch ) {

    CHAR_DATA *victim;
    CHAR_DATA *v_next;

    SOCIETY *society;
    MEMBERSHIP *membership;

    char buf[MAX_STRING_LENGTH];

    int i, j, k;

    char *check;

    int c_num;

   /* Do we have to do this? */

   /* Not if we're not in a room... */

    if (ch->in_room == NULL) {
      return FALSE;
    }

   /* Nor it we're the only person in the room... */

    if ( ch->in_room->people == ch 
      && ch->next_in_room == NULL) {
      return FALSE;
    }

   /* Have we regenned enough mana? */

    if ( ch->position == POS_RESTING 
      && ch->mana > (ch->max_mana / 2) ) {
      do_stand(ch, "");
      return FALSE;
    }

   /* ...and we should be standing up... */ 

    if ( ch->position != POS_STANDING 
      && ch->position != POS_FIGHTING ) {
      return FALSE;
    }

   /* Not if I'm already casting something... */

    if (ch->activity == ACV_CASTING) {
      return TRUE;
    }

   /* Not if I'm muted... */

    if ( is_affected(ch, gafn_mute) &&
	!is_affected(ch, gafn_vocalize) ) {
	return FALSE;
    }

   /* How much mana remains? */

    if (ch->mana < 50) {
      do_rest(ch, "");
      return FALSE;
    }

    c_num = -1;
    if (ch->pIndexData->group > 0) {
        c_num = ch->pIndexData->group;

    } else {

        check = "_soc_";

   /* Search name for _soc_nnnn and extract the number... */

         i = -1;
         j = 0;

         while ((ch->name[++i] != '\0') && (c_num == -1)) {

              if (ch->name[i] == ' ') {
                   k = 0;
                   while (check[k] != '\0' && ch->name[++i] == check[k++]) {
                   }

                   if (k == 5) {
                        j = 0;
                        while (isdigit(ch->name[++i]) && j < (MAX_STRING_LENGTH - 1)) {
                              buf[j++] = ch->name[i];
                        }
                        if (j > 0) {
                              buf[j] = '\0';
                              c_num = atoi(buf);
                        } else {
                              bug("_soc_ without society id on clanhealer in %d", ch->in_room->vnum); 
                              return TRUE;
                        }
                   }
              } 
         }   
    }

   /* Complain if the number isn't valid... */

    if (c_num < 0) {
      bug("No society id on clanhealer in %d", ch->in_room->vnum);
      return TRUE;
    } 
      
    society = find_society_by_id(c_num);

    if (society == NULL) {
      bug("Invalid society id on clanhealer in %d",ch->in_room->vnum);
      return TRUE;
    } 

   /* Better check I'm a member... */

    if (!is_member(ch, society)) create_membership(ch, society, SOC_RANK_MEMBER);

    membership = find_membership(ch, society);

    if (membership == NULL) {

      membership = get_membership();

     /* Add profession and level, if appropriate... */

      membership->society = society;
      membership->level = SOC_LEVEL_MEMBERSHIP;
      membership->profession = society->profession;

     /* Splice in... */

      membership->next = ch->memberships;
      ch->memberships = membership;

    } 

   /* Pick someone at random... */

    for ( victim = ch->in_room->people; 
          victim != NULL;
          victim = v_next ) {

      v_next = victim->next_in_room;

      if ( victim != ch 
        && can_see( ch, victim ) 
        && number_bits( 1 ) == 0 ) {
	  break;
      }
    }

   /* All done if we didn't find anyone... */

    if ( victim == NULL ) {
	return FALSE;
    }

   /* Now, what to we do to foes... */

    if (is_foe(ch, victim)) {

      switch ( number_bits( 4 ) ) {

        case 0:

          if (is_affected(victim, gafn_faerie_fire)) {
            return FALSE;
          }

  	  do_mob_cast(ch, "faerie fire", victim);

	  return TRUE;

        case 1:

          if (is_affected(victim, gafn_curse)) {
            return FALSE;
          }

	  do_mob_cast(ch, "curse", victim);

	  return TRUE;

        case 2:

          if (is_affected(victim, gafn_blindness)) {
            return FALSE;
          }

   	  do_mob_cast(ch, "blindness", victim);

	  return TRUE;

        case 3:

	  do_mob_cast(ch, "cause light", victim);

	  return TRUE;

        case 4:

          if (!is_affected(victim, gafn_poison)) {
            return FALSE;
          }

	  do_mob_cast(ch, "poison", victim);

	  return TRUE;

        case 5:

          if (is_affected(ch, gafn_weakness)) {
            return FALSE;
          }
        
	  do_mob_cast(ch, "weaken", victim);

	  return TRUE;

        default:
          break;
      }

      return FALSE;
    } 

   /* Now, what to we do to friends... */

    if (is_friend(ch, victim)) {

      switch ( number_bits( 4 ) ) {

        case 0:

          if (is_affected(victim, gafn_armor)) {
            return FALSE;
          }

  	  do_mob_cast(ch, "armor", victim);

	  return TRUE;

        case 1:

          if (is_affected(victim, gafn_bless)) {
            return FALSE;
          }

	  do_mob_cast(ch, "bless", victim);

	  return TRUE;

        case 2:

          if (!is_affected(victim, gafn_blindness)) {
            return FALSE;
          }

   	  do_mob_cast(ch, "cure blindness", victim);

	  return TRUE;

        case 3:

          if (victim->hit > (victim->max_hit - 5)) {
            return FALSE;
          }

	  do_mob_cast(ch, "cure light", victim);

	  return TRUE;

        case 4:

          if (is_affected(victim, gafn_poison)) {
            return FALSE;
          }

	  do_mob_cast(ch, "cure poison", victim);

	  return TRUE;

        case 5:

          if (victim->move > (victim->max_move - 10)) {
            return FALSE;
          }

	  do_mob_cast(ch, "refresh", victim);

	  return TRUE;

        default:
          break;
      }

      return FALSE;
    } 

   /* And everyone else we ignore... */ 

    return FALSE;
}

bool spec_questmaster (CHAR_DATA *ch) {
    if (ch->fighting != NULL) return spec_cast_mage( ch );
    return FALSE;
}

bool spec_playershop( CHAR_DATA *ch) {
  CHAR_DATA *suspect, *testsus;
  char* lastarg;

  if (ch->in_room == NULL) return FALSE;
  
  if ( ch->in_room->people == ch 
  && ch->next_in_room == NULL) {
        return FALSE;
  }

  if ( ch->position != POS_STANDING 
  && ch->position != POS_FIGHTING ) {
        return FALSE;
  }

  lastarg = check_mob_owner(ch);
  if (!strcmp(lastarg, "not found") || lastarg == NULL) return TRUE;
     
  suspect = ch->in_room->people; 

  while (suspect != NULL) {
       testsus = suspect;
       suspect = suspect->next_in_room;
       if (testsus == ch) continue;
    
       if (IS_IMMORTAL(testsus)) continue; 
       if (!can_see(ch, testsus)) continue;
  
       if (!str_cmp(testsus->short_descr, lastarg)) {
             break;
       } 
  }
  return TRUE;
}


bool spec_miner( CHAR_DATA *ch) {
  CHAR_DATA *suspect, *testsus;
  OBJ_INDEX_DATA *oreIndex;
  OBJ_DATA *obj;
  char* lastarg;
  int mat;  


  if (ch->in_room == NULL) return FALSE;
  if (ch->in_room->sector_type != SECT_UNDERGROUND) return FALSE;  

  lastarg = check_mob_owner(ch);
  if (lastarg == NULL || !strcmp(lastarg, "not found")) return TRUE;

  if ( ch->in_room->people == ch 
  && ch->next_in_room == NULL) {
     if (number_percent() <= 1) {
         for(mat=OBJ_VNUM_ORE_LOW; mat<=OBJ_VNUM_ORE_LOW+10; mat++) {
              oreIndex = get_obj_index(mat);
              if (oreIndex == NULL) continue;
              if (oreIndex->zmask != 0) {
                  if ((oreIndex->zmask & zones[ch->in_room->area->zone].mask)  == 0 ) continue;
              }
              if (number_range(0, oreIndex->cost / 7) == 0) {
                  obj = create_object (oreIndex, oreIndex->level);
                  obj_to_room (obj, ch->in_room);
                  do_get(ch, "all");
              }
        }
     }
     return TRUE;
  }

  if ( ch->position != POS_STANDING 
  && ch->position != POS_FIGHTING ) {
        return FALSE;
  }
  
  suspect = ch->in_room->people; 

  while (suspect != NULL) {
       testsus = suspect;
       suspect = suspect->next_in_room;
       if (testsus == ch) continue;
    
       if (IS_IMMORTAL(testsus)) continue; 
       if (!can_see(ch, testsus)) continue;
  
       if (!str_cmp(testsus->short_descr, lastarg)) break;
  }
  return TRUE;
}


bool spec_translator( CHAR_DATA *ch ){
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    for (obj = ch->carrying; obj != NULL; obj = obj_next ) {
         obj_next = obj->next_content;
         
         if (obj->item_type == ITEM_BOOK) {
             if (number_percent() < (2*ch->level - obj->level)*get_skill(ch, obj->value[0]) / 100
             || number_percent() == 1) {
                  if (obj->condition < 100) obj->condition++;
                  else obj->value[0] = get_skill_sn("english");
             }
         }
    }

    return TRUE;
}    

bool spec_mechanic( CHAR_DATA *ch ){
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    for (obj = ch->carrying; obj != NULL; obj = obj_next ) {
         obj_next = obj->next_content;
         
         if (obj->item_type == ITEM_PORTAL
         && obj->value[4] == PORTAL_VEHICLE) {
             if (number_percent() < (2*ch->level - obj->level) / 5
             || number_percent() == 1) {
                  if (obj->condition < 100) {
                      obj->condition++;
                      obj->pIndexData->condition = obj->condition;
                      SET_BIT( obj->pIndexData->area->area_flags, AREA_CHANGED );
                  }
             }
         }
    }

    return TRUE;
}    


char* check_mob_owner(CHAR_DATA *mob) {
char* argument;
char transarg[MAX_INPUT_LENGTH];
char* lastarg = NULL;
bool namefound = FALSE;
char *check = str_dup("_plr:");

  argument = str_dup(mob->name);

  while (argument[0] != '\0' && namefound == FALSE) {
     argument = one_argument(argument, transarg);
     if (transarg[0] != '\0') {
         if (!str_cmp(transarg, check)) {
              if (argument[0] != '\0') {
                  namefound = TRUE;
                  argument = one_argument(argument, transarg);
              }
         }
     }
  }

  if (namefound == FALSE)  return "not found";
  if (transarg[0] == '\0') return "not found";

  lastarg = str_dup(transarg);
  return lastarg;
}


char* check_obj_owner(OBJ_DATA *obj) {
char* argument;
char* lastarg = NULL;
char transarg[MAX_INPUT_LENGTH];
bool namefound = FALSE;
char *check =  str_dup("_plr:");

  argument = str_dup(obj->name);

  while (argument[0] != '\0' && namefound == FALSE) {
     argument = one_argument(argument, transarg);
     if (transarg[0] != '\0') {
         if (!str_cmp(transarg, check)) {
              if (argument[0] != '\0') {
                  namefound = TRUE;
                  argument = one_argument(argument, transarg);
              }
         }
     }
  }
  
  if (namefound == FALSE)  return "not found";
  if (transarg[0] == '\0') return "not found";

  lastarg =str_dup(transarg);
  return lastarg;
}


bool spec_troll_member( CHAR_DATA *ch) {
    CHAR_DATA *vch, *victim = NULL;
    int count = 0;
    char *message;

    if (!IS_AWAKE(ch) 
    || IS_AFFECTED(ch,AFF_CALM)
    || ch->in_room == NULL 
    ||  IS_AFFECTED(ch,AFF_CHARM)
    || ch->fighting != NULL)
	return FALSE;

    for (vch = ch->in_room->people;  vch != NULL;  vch = vch->next_in_room) {
	if (!IS_NPC(vch) || ch == vch) continue;

	if (vch->pIndexData->vnum == MOB_VNUM_PATROLMAN
                || (vch->pIndexData->spec_fun && !str_cmp("spec_patrolman", spec_string(vch->pIndexData->spec_fun)))) return FALSE;

	if ((vch->pIndexData->group == GROUP_VNUM_OGRES || (vch->pIndexData->spec_fun && !str_cmp("spec_ogre_member", spec_string(vch->pIndexData->spec_fun))))
	&&  ch->level > vch->level - 2 && !is_safe(ch,vch)) {
	    if (number_range(0,count) == 0) victim = vch;
	    count++;
	}
    }

    if (victim == NULL) return FALSE;

    switch (number_range(0,6)) {
	default:
                    message = NULL;
   	    break;
	case 0:
   	    message = "$n yells 'I've been looking for you, punk!'";
	    break;
	case 1:
                    message = "With a scream of rage, $n attacks $N.";
	    break;
	case 2:
                     message = "$n says 'What's slimy trash like you doing around here?'";
	     break;
	case 3:
                     message = "$n cracks his knuckles and says 'Do ya feel lucky?'";
	     break;
	case 4:
                      message = "$n says 'There's no cops to save you this time!'";
	      break;	
	case 5:
                      message = "$n says 'Time to join your brother, spud.'";
	      break;
	case 6:
                      message = "$n says 'Let's rock.'";
	      break;
    }

    if (message != NULL) act(message,ch,NULL,victim,TO_ALL);
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return TRUE;
}


bool spec_ogre_member( CHAR_DATA *ch) {
    CHAR_DATA *vch, *victim = NULL;
    int count = 0;
    char *message;
 
    if (!IS_AWAKE(ch)
    || IS_AFFECTED(ch,AFF_CALM)
    || ch->in_room == NULL
    ||  IS_AFFECTED(ch,AFF_CHARM)
    || ch->fighting != NULL)
        return FALSE;

    for (vch = ch->in_room->people;  vch != NULL;  vch = vch->next_in_room) {
        if (!IS_NPC(vch) || ch == vch) continue;
 
        if (vch->pIndexData->vnum == MOB_VNUM_PATROLMAN
        || (vch->pIndexData->spec_fun && !str_cmp("spec_patrolman", spec_string(vch->pIndexData->spec_fun)))) return FALSE;
 
        if ((vch->pIndexData->group == GROUP_VNUM_TROLLS || (vch->pIndexData->spec_fun && !str_cmp("spec_troll_member", spec_string(vch->pIndexData->spec_fun))))
        &&  ch->level > vch->level - 2 && !is_safe(ch,vch))  {
            if (number_range(0,count) == 0) victim = vch;
            count++;
        }
    }
 
    if (victim == NULL)  return FALSE;
 
    switch (number_range(0,6)) {
        default:
            message = NULL;
            break;
        case 0:
            message = "$n yells 'I've been looking for you, punk!'";
            break;
        case 1:
            message = "With a scream of rage, $n attacks $N.'";
            break;
        case 2:
            message ="$n says 'What's filth like you doing around here?'";
            break;
        case 3:
            message = "$n cracks his knuckles and says 'Do ya feel lucky?'";
            break;
        case 4:
            message = "$n says 'There's no cops to save you this time!'";
            break;
        case 5:
            message = "$n says 'Time to join your brother, spud.'";
            break;
        case 6:
            message = "$n says 'Let's rock.'";
            break;
    }
 
    if (message != NULL) act(message,ch,NULL,victim,TO_ALL);
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return TRUE;
}


bool spec_patrolman(CHAR_DATA *ch) {
    CHAR_DATA *vch,*victim = NULL;
    OBJ_DATA *obj;
    char *message;
    int count = 0;

    if (!IS_AWAKE(ch) 
    || IS_AFFECTED(ch,AFF_CALM)
    || ch->in_room == NULL
    ||  IS_AFFECTED(ch,AFF_CHARM) 
    || ch->fighting != NULL)
        return FALSE;

    /* look for a fight in the room */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)   {
	if (vch == ch)   continue;

	if (vch->fighting != NULL) {
	    if (number_range(0,count) == 0) victim = (vch->level > vch->fighting->level) ? vch : vch->fighting;
	    count++;
	}
    }

    if (victim == NULL || (IS_NPC(victim) && victim->spec_fun == ch->spec_fun)) return FALSE;

    if (((obj = get_eq_char(ch,WEAR_NECK_1)) != NULL 
    &&   obj->pIndexData->vnum == OBJ_VNUM_WHISTLE)
    ||  ((obj = get_eq_char(ch,WEAR_NECK_2)) != NULL
    &&   obj->pIndexData->vnum == OBJ_VNUM_WHISTLE)) {
	act("You blow down hard on $p.",ch,obj,NULL,TO_CHAR);
	act("$n blows on $p, ***WHEEEEEEEEEEEET***",ch,obj,NULL,TO_ROOM);

    	for ( vch = char_list; vch != NULL; vch = vch->next )	{
                     if ( vch->in_room == NULL ) 	continue;

                     if (vch->in_room != ch->in_room 
                     &&  vch->in_room->area == ch->in_room->area)
            	             send_to_char( "You hear a shrill whistling sound.\r\n", vch );
    	     }
                }

                switch (number_range(0,6))  {
	     default:
                         message = NULL;
 	         break;
	     case 0:
	          message = "$n yells 'All roit! All roit! break it up!'";
	          break;
	      case 1:
 	          message = "$n says 'Society's to blame, but what's a bloke to do?'";
	          break;
	      case 2:
                          message = "$n mumbles 'bloody kids will be the death of us all.'";
	          break;
	      case 3:
                          message = "$n shouts 'Stop that! Stop that!' and attacks.";
	          break;
	      case 4:
                          message = "$n pulls out his billy and goes to work.";
	          break;
	      case 5:
                          message = "$n sighs in resignation and proceeds to break up the fight.";
	          break;
	      case 6:
                           message = "$n says 'Settle down, you hooligans!'";
	           break;
    }

    if (message != NULL) act(message, ch, NULL, NULL, TO_ALL);
    multi_hit(ch,victim,TYPE_UNDEFINED);
    return TRUE;
}
	

bool spec_nasty( CHAR_DATA *ch ) {
    CHAR_DATA *victim, *v_next;
    long gold;
    char buf[MAX_STRING_LENGTH]; 
    int i;

    if (!IS_AWAKE(ch)) return FALSE;
 
    if (ch->position != POS_FIGHTING) {
       for ( victim = ch->in_room->people; victim != NULL; victim = v_next)
       {
          v_next = victim->next_in_room;
          if (!IS_NPC(victim)
             && (victim->level > ch->level)
             && (victim->level < ch->level + 10))
          {
             sprintf(buf, "%s", victim->name);		
	     do_backstab(ch, buf);

             if (ch->position != POS_FIGHTING)
	     {
                 do_murder(ch, buf);
	     }

             /* should steal some coins right away? :) */
             return TRUE;
          }
       }
       return FALSE;    /*  No one to attack */
    }
 
    /* okay, we must be fighting.... steal some coins and flee */
    if ( (victim = ch->fighting) == NULL)
        return FALSE;   /* let's be paranoid.... */
 
    switch ( number_bits(2) )   {
        case 0:  act( "$n rips apart your coin purse, spilling your money!",  ch, NULL, victim, TO_VICT);
                 act( "You slash apart $N's coin purse and gather his gold.", ch, NULL, victim, TO_CHAR);
                 act( "$N's coin purse is ripped apart!", ch, NULL, victim, TO_NOTVICT);
                 for (i = 0; i < MAX_CURRENCY; i++) {
                     gold = victim->gold[i] / 10; 
                     victim->gold[i] -= gold;
                     ch->gold[i]     += gold;
                 }
                 return TRUE;
 
        case 1:  do_flee(ch, "");
                 return TRUE;
 
        default: return FALSE;
    }
}


bool spec_artifact_tracker (CHAR_DATA *ch) {
    return spec_teleTracker( ch );
}

bool spec_artifact_hunter (CHAR_DATA *ch) {
    return spec_teleHunter( ch );
}


bool spec_social(CHAR_DATA *ch) {
CHAR_DATA *victim, *v_next;

    if (!ch->pIndexData) return FALSE;
    if (!IS_AWAKE(ch)) return FALSE;
    if (ch->master || ch->leader) return FALSE;
    if (ch->position < POS_STANDING) return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next) {
        v_next = victim->next_in_room;

        if (victim == ch) continue;
        if (victim->position < POS_STANDING) continue;
        if (victim->master || victim->leader) continue;

        if (victim->pIndexData) {
           if (victim->pIndexData->vnum == ch->pIndexData->vnum) {
                add_follower( ch, victim );
                ch->leader = victim;
                act( "$n joins $N's group.", ch, NULL, victim, TO_ROOM );
                return TRUE;
           }
       }
    }
    return FALSE; 
}


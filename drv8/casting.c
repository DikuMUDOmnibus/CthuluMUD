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
#include "skill.h"
#include "fight.h"
#include "affect.h"
#include "spell.h"
#include "exp.h"
#include "profile.h"
#include "wev.h"
#include "mob.h"
#include "econd.h"
#include "rooms.h"
#include "society.h"
#include "gsn.h"
#include "music.h"

/* command procedures needed */
DECLARE_DO_FUN(do_cast);


// -----------------------------------------------------------------------
// Assorted (short lived) globals...
// -----------------------------------------------------------------------

char *target_name;
int spell_roll;
int modlev;
COMBAT_DATA *spell_CDB;
bool in_seq = FALSE;


void write_to_grimoire(CHAR_DATA *ch, char *discipline) {
OBJ_DATA *grim;
EXTRA_DESCR_DATA *ed;
char buf[MAX_INPUT_LENGTH];


      for (grim = ch->carrying; grim; grim = grim->next_content) {
            if (grim->item_type == ITEM_GRIMOIRE) {
                  for (ed = grim->extra_descr; ed; ed = ed->next) {
                         if (!strcmp(ed->keyword, "read grimoire")) {
                              sprintf(buf, "%s%s ",ed->description, discipline);
                              free_string(ed->description);
                              ed->description = strdup(buf);
                              SET_BIT(grim->value[0], flag_value(discipline_type, discipline));
                              sprintf_to_char(ch, "{mYou write the discipline %s to your Grimoire.{x\r\n", capitalize(discipline));
                              return;
                         }
                  }
            }
      }
      return;
}


bool knows_discipline(CHAR_DATA *ch, long discipline) {
OBJ_DATA *obj;

    if (IS_NPC(ch) && !IS_AFFECTED(ch, AFF_CHARM) && !ch->master && !ch->leader) return TRUE;
    if (IS_IMMORTAL(ch)) return TRUE;

    for(obj = ch->carrying; obj; obj = obj->next_content) {
          if (obj->item_type != ITEM_GRIMOIRE) continue;

          if (IS_SET(obj->value[0], discipline)) return TRUE;
    }
    return FALSE;
}


void do_ritual( CHAR_DATA *ch, char *argument ) {
OBJ_DATA *obj;
AFFECT_DATA af;

if (argument[0] =='\0') {
    obj = ch->carrying;

    while (obj != NULL
    && (obj->pIndexData->vnum != OBJ_VNUM_RITUAL 
          || obj->wear_loc != -1
          || !can_see_obj(ch, obj) ) ) {
                obj = obj->next_content;
    }

    if ( obj == NULL ) {
       send_to_char("You have no suitable ritual candles!\r\n", ch);
       return;
    }

     if (IS_RAFFECTED(ch->in_room, RAFF_HIGH_MAGIC))	{
          send_to_char ("This room seems pretty much prepared.\r\n",ch);
          return;
     }

     if (IS_RAFFECTED(ch->in_room, RAFF_LOW_MAGIC))	{
          send_to_char ("This won't help very much.\r\n",ch);
          return;
     }

     obj_from_char( obj );
     extract_obj(obj);	

     WAIT_STATE(ch, 24);

     if (check_skill(ch, gsn_occult, 15)) {
         af.type        	= SKILL_UNDEFINED;
         af.afn	= gafn_room_high_magic; 
         af.level     	= ch->level;
         af.duration  	= ch->level/10+1;
         af.location  	= APPLY_NONE;
         af.modifier  	= 0;
         af.bitvector 	= RAFF_HIGH_MAGIC;
         affect_to_room(ch->in_room, &af );

         act("You prepare the place for a mighty ritual.", ch, NULL, NULL, TO_CHAR);
         act("$n prepares the place for a mighty ritual.", ch, NULL, NULL, TO_ROOM);
     } else {
         act("You fail to prepare the place for a mighty ritual.", ch, NULL, NULL, TO_CHAR);
         act("$n tries to prepare a ritual but fails.", ch, NULL, NULL, TO_ROOM);
     }
     return;

} else {
     CHAR_DATA *fch;

     if (get_skill(ch, gsn_ritual_mastery) == 0) {
          send_to_char ("You're no experienced ritual master.\r\n",ch);
          return;
     }

     if (!IS_RAFFECTED(ch->in_room, RAFF_HIGH_MAGIC))	{
          send_to_char ("This room is not prepared.\r\n",ch);
          return;
     }

     modlev = ch->level * get_skill(ch, gsn_spell_casting) / 150;
     fch = ch->in_room->people;
     while (fch) {
          if (is_same_group(fch, ch)
          && fch != ch) {
              modlev += fch->level * get_skill(fch, gsn_spell_casting) / 150;
          }
          fch = fch->next_in_room;
     }
     modlev = modlev * get_skill(ch, gsn_ritual_mastery) / 75; 
     modlev = UMAX(modlev, 1);
     do_cast(ch, argument);
     modlev = 0;
     return;
}
}


/* Check for skill improvements after casting a spell... */

void check_improve_spell(CHAR_DATA *ch, int spn, bool success) {

  if (!isSkilled(ch)) return;
  if (validSkill(spell_array[spn].sn)) check_improve(ch, spell_array[spn].sn, success, 1);
  if (validSkill(spell_array[spn].sn2)) check_improve(ch, spell_array[spn].sn2, success, 1);
  return; 
}


/* Work out skill with a spell... */

int spell_skill(CHAR_DATA *ch, int spn) {
int skill;

  skill = 0;

  if (valid_spell(spn)) {
    if (isSkilled(ch)) {
 
      if (spell_array[spn].sn != SKILL_UNDEFINED) skill = get_skill(ch, spell_array[spn].sn);
      if (spell_array[spn].sn2 != SKILL_UNDEFINED) skill = ( skill + get_skill(ch, spell_array[spn].sn2)) / 2;

    } else {
      skill = ch->level;
    }

    skill = skill - spell_array[spn].diff;
  }

  return skill;
}


/* Work out a spells base mana cost... */

int mana_cost(CHAR_DATA *ch, int spn) {
int min_mana;
int char_mana;
int mana;
int skill;

  if (!valid_spell(spn)) return 10000;

  min_mana = spell_array[spn].mana;

  skill = spell_skill(ch, spn);

  char_mana = (min_mana * 4 * (100 - skill)) / 100;

  mana = UMAX(min_mana, char_mana);

 /* Modify mana for area/zone magic capabilities... */

  if ( ch->in_room != NULL
    && ch->in_room->area != NULL) {

   /* First room... */

   if (IS_RAFFECTED(ch->in_room, RAFF_LOW_MAGIC)) mana *=2;
   if (IS_RAFFECTED(ch->in_room, RAFF_HIGH_MAGIC)) mana /=2;


    if ( IS_SET(ch->in_room->area->area_flags, AREA_NOMAGIC)
    || IS_SET(ch->in_room->room_flags, ROOM_NOMAGIC)) { 
      mana = 999;
    } else if ( IS_SET(ch->in_room->area->area_flags, AREA_LOWMAGIC) ) { 
      mana *= 2;
    } else if ( IS_SET(ch->in_room->area->area_flags, AREA_HIGHMAGIC) ) { 
      mana -= (mana/5);
    } else if ( IS_SET(ch->in_room->area->area_flags, AREA_SUPERMAGIC) || IS_ARTIFACTED(ch, ART_NECRONOMICON)) { 
      mana /= 2;
    }
 
   /* Then zone... */

    if ( IS_SET(zones[ch->in_room->area->zone].flags, ZONE_NOMAGIC) ) { 
      mana = 999;
    } else if ( IS_SET(zones[ch->in_room->area->zone].flags, ZONE_LOWMAGIC) ) { 
      mana *= 2;
    } else if ( IS_SET(zones[ch->in_room->area->zone].flags, ZONE_HIGHMAGIC) ) { 
      mana -= (mana/5);
    } else if ( IS_SET(zones[ch->in_room->area->zone].flags, ZONE_SUPERMAGIC) ) { 
      mana /= 2;
    }
 
  }

  return mana;
}

//
// Display information about a spell
//

void do_spell_info(CHAR_DATA *ch, char *args) {
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int spn, skl;

   /* Identify the spell... */

    spn = get_spell_spn(args);

    if ( spn <= 0 ) {
      send_to_char("No such spell.\r\n", ch);
      return;
    }

   /* Check thier skill... */

    skl = spell_skill(ch, spn);

   /* Format details... */

   /* Name and description... */

    sprintf(buf, "{cSpell: {g%s{c\r\n"
                 "-----------------------------------------------------{x\r\n",
                  spell_array[spn].name);

    if (spell_array[spn].desc != NULL) {
      strcat(buf, "{g"); 
      strcat(buf, spell_array[spn].desc);
      strcat(buf, "{x\r\n");
    }

   /* Thats all if you do not know the spell... */

    if ( skl <= 0 ) { 
      send_to_char(buf, ch);
      send_to_char("{yYou do not know how to cast this spell.\r\n", ch);
      return;
    }

   /* Mana cost... */

    sprintf(buf2, "{cMana - base: {g%d   {ccurrent: {g%d{x\r\n",  spell_array[spn].mana,  mana_cost(ch, spn));
    strcat(buf, buf2);  

   /* Skills used... */

    if ( spell_array[spn].sn > 0 ) {
      strcat(buf, "{cSkills     : {g");
      strcat(buf, skill_array[spell_array[spn].sn].name);
 
      if ( spell_array[spn].sn2 > 0 ) {
        strcat(buf, ", "); 
        strcat(buf, skill_array[spell_array[spn].sn2].name);
      }

      sprintf(buf2, "{c - current: {g%d{x\r\n", spell_skill(ch, spn) ); 
      strcat(buf, buf2); 
    }

   /* Difficulty... */

    sprintf(buf2, "{cDifficulty : {g%d{x\r\n", spell_array[spn].diff);
    strcat(buf, buf2);

    if (spell_array[spn].discipline > 0) {
        sprintf(buf2, "{cDiscipline : {g%s{x\r\n", flag_string(discipline_type, spell_array[spn].discipline));
        strcat(buf, buf2);
    }

    if (spell_array[spn].music_style > 0) {
        if (spell_array[spn].music_power == 0) sprintf(buf2, "{cMusic      : {g%s ({r++{g){x\r\n", music_styles[spell_array[spn].music_style].name);
        else sprintf(buf2, "{cMusic      : {g%s ({r%d %%{g){x\r\n", music_styles[spell_array[spn].music_style].name, spell_array[spn].music_power);
        strcat(buf, buf2);
    }        

    if (IS_IMMORTAL(ch)) {
        sprintf(buf2, "{cInfo       : {g%s{x\r\n", spell_array[spn].info);
        strcat(buf, buf2);
    }

   /* Component... */

    if ( spell_array[spn].component == NULL ) {
      strcat(buf, "{cComponent  : {gnone{x\r\n");
    } else {
      strcat(buf, "{cComponent  : {g");
      strcat(buf, spell_array[spn].component);
      strcat(buf, "{x\r\n");
    }

   /* Target... */

    switch (spell_array[spn].target) {

      default:
        strcat(buf, "{cTarget     : {RInvalid!{x\r\n");
        break;   

      case TAR_IGNORE:
        strcat(buf, "{cTarget     : {gSpecial{x\r\n");
        break;   

      case TAR_CHAR_SELF:
        strcat(buf, "{cTarget     : {gCaster{x\r\n");
        break;   

      case TAR_CHAR_DEFENSIVE:
        strcat(buf, "{cTarget     : {gCaster or Friend{x\r\n");
        break;   

      case TAR_CHAR_ANYWHERE:
        strcat(buf, "{cTarget     : {gAny Person in the World{x\r\n");
        break;   

      case TAR_CHAR_OFFENSIVE:
        strcat(buf, "{cTarget     : {gFoe{x\r\n");
        break;   

      case TAR_OBJ_INV:
        strcat(buf, "{cTarget     : {gObject in casters inventory{x\r\n");
        break;   

      case TAR_OBJ_ROOM:
        strcat(buf, "{cTarget     : {gObject in room{x\r\n");
        break;   

    }

   /* Effect... */

    if ( spell_array[spn].effect > 0 ) {
      strcat(buf, "{cEffect     : {g");
      strcat(buf, effect_array[spell_array[spn].effect].name); 
      strcat(buf, "{x\r\n");
    }

   /* Send the data... */

    send_to_char(buf, ch);

    return;
}


/* RT spells and skills show the players spells (or skills) */

void do_spells(CHAR_DATA *ch, char *argument) {

    char spell_list[SKILL_RANKS+1][4 * MAX_STRING_LENGTH];
    char spell_columns[SKILL_RANKS+1];
    int spn,lev,mana, skill, last_lev;
    bool found = FALSE;
    char buf[MAX_STRING_LENGTH];
    char outbuf[(MAX_SKILL+10) * 80];

    CHAR_DATA *victim;

    outbuf[0] = '\0';

    if (argument[0] != '\0') {
      if (!IS_IMMORTAL(ch)) {
        do_spell_info(ch, argument);
        return;
      }

      spn = get_spell_spn(argument);

      if (spn > 0) {
        do_spell_info(ch, argument);
        return;
      }

      victim = get_char_world(ch, argument);

      if ( victim == NULL ) {
        send_to_char("No such person.\r\n", ch);
        return;
      } 
    } else {
      victim = ch;
    }

    if (IS_AFFECTED(ch, AFF_BERSERK)) {
      send_to_char("{rYou are to {Rangry{x to worry about spells..."
                   "you just want to {RKILL SOMEONE!{x\r\n", ch);
      return;
    }

    /* initilize data */

    for (lev = 0; lev < SKILL_RANKS + 1; lev++) {
	spell_columns[lev] = 0;
	spell_list[lev][0] = '\0';
    }
 
    for (spn = 0; spn < MAX_SKILL; spn++) {

      if (spell_array[spn].loaded != TRUE) break; 
      
      if ( validSkill(spell_array[spn].sn)
        && isSkillAvailable(victim, spell_array[spn].sn) ) {
     
        skill = spell_skill(victim, spn);

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
          lev = 8;
        }    

        if (lev != 8) {
          found = TRUE;
        }

        mana = mana_cost(victim, spn); 

	sprintf(buf," %-19s %3d  ", spell_array[spn].name, mana);
	
	strcat(spell_list[lev],buf);

	if ( ++spell_columns[lev] % 3 == 0) {
          strcat(spell_list[lev],"\r\n");
        }

      }
    }

    /* return results */
 
    if (!found) {

      if (victim == ch) { 
        send_to_char("You know no spells.\r\n",ch);
      } else {
        send_to_char("They know no spells.\r\n", ch);
      }
 
      return;
    }
    
    last_lev = -1;

    for (lev = 0; lev < (SKILL_RANKS - 1); lev++) {
  
      if (spell_list[lev][0] != '\0') {

        if (last_lev > -1) {
          if ( (spell_columns[last_lev] % 3) != 0 ) {
            strcat(outbuf, "\r\n");
          }
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
            strcat(outbuf, "{y[Hopeless]{g\r\n");
            break;
          default:
            strcat(outbuf, "{R[Unknown!]\r\n");
            break; 
        }

        strcat(outbuf,spell_list[lev]);

      }
    }

    strcat(outbuf,"{x\r\n");

    page_to_char(outbuf,ch);
}

// -----------------------------------------------------------------------
// Start of the spell casting routines...
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// Chant a phrase in Cthonic...
// -----------------------------------------------------------------------

void say_spell( CHAR_DATA *ch, char *text ) {
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

   /* Tell everyone else... */

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, gsn_cthonic, text);
    wev = get_wev(WEV_SPELL, WEV_SPELL_CHANT, mcc,
                 "{gYou chant '@t2'{x\r\n",
                  NULL,
                 "{g@a2 chants '@t2'{x\r\n");

    room_issue_wev( ch->in_room, wev);
    free_wev(wev);
    return;
}


void yell_spell( CHAR_DATA *ch, char *text ) {
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

   /* Tell everyone else... */

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, gsn_cthonic, text);
    wev = get_wev(WEV_SPELL, WEV_SPELL_YELL, mcc,
                 "{YYou yell '@t2'{x\r\n",
                  NULL,
                 "{Y@a2 yells '@t2'{x\r\n");

    area_issue_wev(ch->in_room, wev, WEV_SCOPE_SUBAREA);
    free_wev(wev);
    return;
}


// -----------------------------------------------------------------------
// Run away or turn and fight when a spell is cast at you...
// -----------------------------------------------------------------------

void react_to_cast(CHAR_DATA *ch, CHAR_DATA *attacker) {

    char buf[MAX_STRING_LENGTH];

    int adj;

   /* We can beat ourselves up... */

    if ( ch == attacker ) {
      return;
    }

   /* Cannot retaliate (yet) if they are not in the same room... */

    if ( ch->in_room != attacker->in_room ) {
      return;
    }
 
   /* Our master must be doing this for our own good... */ 

    if ( ch->master == attacker ) {
      return;
    }

   /* Cannot do anything if already busy fighting... */

    if ( ch->fighting != NULL ) {  
      return;
    }

    if ( ch->position == POS_FIGHTING ) {
      return;
    }

   /* It's probably more important to complete the spell... */

    if ( ch->activity == ACV_CASTING ) {
      return;
    }

   /* Work out effective level adjustment... */

    adj = 5;

   /* Can we see the caster? */

    if (!can_see(ch, attacker)) {

      if (number_bits(1) == 0) {
        enqueue_mob_cmd(ch, "emote starts looking around curiously!", 1, 0);
        return;
      }   

      adj -= 5;
    }
 
   /* Ok, so now what do we do? */

    if (  IS_NPC(ch)
      &&  (  IS_SET(ch->act, ACT_WIMPY) 
         ||  IS_SET(ch->act, ACT_THIEF)
         ||  IS_SET(ch->act, ACT_MAGE ) )
      && !IS_SET(ch->act, ACT_AGGRESSIVE)
      && !IS_SET(ch->act, ACT_SENTINEL) ) {
      adj -= 7;
    } 

    if ( ( attacker->level > (ch->level + adj)
        || !can_see(ch, attacker) )
      && !IS_IMMORTAL( attacker ) 
      &&  number_bits(1) == 0 ) {
      strcpy(buf, "flee");
    } else {
      sprintf(buf, "kill %s", attacker->true_name);
    }

    if ( ch->position < POS_STANDING ) {
      enqueue_mob_cmd(ch, "stand", 1, 0);
      enqueue_mob_cmd(ch, buf, 2, 0);
    } else {
      enqueue_mob_cmd(ch, buf, 1, 0);
    }

   /* All done... */

    return;
}


// -----------------------------------------------------------------------
// Identify spell, skills and target
// -----------------------------------------------------------------------

bool do_cast_begin(CHAR_DATA *ch, char *argument) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int sn; 
    int spn;	
    int cast_pos;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;
    char buf[MAX_STRING_LENGTH];

    if (get_skill(ch, gsn_spell_casting) < 1) {
      send_to_char("You do not know how to cast spells...\r\n", ch);
      return FALSE;
    }

    target_name = one_argument(argument, arg1 );
    one_argument(target_name, arg2 );

    if ( arg1[0] == '\0' ) {
      send_to_char("Cast which what where?\r\n", ch);
      return FALSE;
    }

   /* Find the spell... */

    spn = get_spell_spn(arg1);

    if (!spell_array[spn].parse_info) {
        target_name = str_dup(" ");
    }

    if ( !valid_spell(spn) ) {
      send_to_char("That is not a real spell!\r\n", ch);
      return FALSE;
    }

    if ( spell_array[spn].loaded != TRUE ) { 
      send_to_char("Fakery and trickery!\r\n", ch);
      return FALSE;
    }

    if ((is_affected(ch, gafn_mute)
    || IS_RAFFECTED(ch->in_room, RAFF_SILENCE))
      && spn != get_spell_spn("cancellation")
      && spn != get_spell_spn("vocalize")
      && !is_affected(ch, gafn_vocalize) ) {
      send_to_char ("You would not be able to say the words!\r\n",ch);
      return FALSE;
    }  
  
   /* Then find the skill... */

    sn = spell_array[spn].sn;

    if (!validSkill(sn)) {
      send_to_char("No one knows how to cast that spell!\r\n", ch);
      bug("Spell %d does not have matching skill.", spn);
      return FALSE;
    } 

    if (isSkilled(ch)) {
      if (ch->effective->skill[sn] < 1) {
        send_to_char("You have no idea how to cast that spell!\r\n", ch );
        return FALSE;
      }
    }

   /* Check environment... */

    if ( ch->position < skill_array[spn].minpos ) {
      send_to_char( "You can not concentrate enough.\r\n", ch );
      return FALSE;
    }

   /* Locate targets... */
   
    victim	= NULL;
    obj		= NULL;
      
    cast_pos = ch->position;

    switch ( spell_array[spn].target ) {
     
      default:
        bug( "Do_cast: bad target for spn %d.", spn );
        return FALSE;

      case TAR_IGNORE:

        mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, spn, spell_array[spn].name);
        wev = get_wev(WEV_SPELL, WEV_SPELL_PREP, mcc,
                     "You prepare to cast '@t0'.\r\n",
                      NULL,
                     "@a2 is preparing to cast a spell!\r\n");

        sprintf(buf, "casting '%s'.", spell_array[spn].name);
    
        break;

      case TAR_CHAR_OFFENSIVE:

        if (IS_AFFECTED(ch, AFF_FEAR)) {
          send_to_char ("You are too scared to attack anyone...\r\n",ch);
          return FALSE;
        }

	if ( arg2[0] == '\0' ) {
	  victim = ch->fighting;
	  if ( victim == NULL ) {
	    send_to_char( "Cast the spell on whom?\r\n", ch );
	    return FALSE;
	  }
	} else {

	  victim = get_char_room( ch, arg2 );

  	  if ( victim == NULL ) {
	    send_to_char( "They are not here.\r\n", ch );
	    return FALSE;
	  }
        }

        if ( ch == victim ) {
          send_to_char( "You do not want to do that to yourself!\r\n", ch );
          return FALSE;
        }

	if ( !IS_NPC(ch) ) {

          if (is_safe_spell(ch,victim,FALSE) && victim != ch) {
	    send_to_char("Not on that target!\r\n",ch);
	    return FALSE; 
	  }
	
	  if (!IS_NPC(victim) && !can_murder(ch, victim)) {
	    send_to_char("That target is outside your pkill range.\r\n", ch);
	    return FALSE;
	  }	
	}

        if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim ) {
	  send_to_char( "You can not do that to your good friend...\r\n", ch );
          return FALSE;
        }

        mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, spn, spell_array[spn].name);
        wev = get_wev(WEV_SPELL, WEV_SPELL_PREP, mcc,
                     "You prepare to cast '@t0' at @v2.\r\n",
                     "@a2 is preparing to cast a spell at you!\r\n",
                     "@a2 is preparing to cast a spell at @v2!\r\n");

        sprintf(buf, "casting '%s'.", spell_array[spn].name);
    
        cast_pos = POS_FIGHTING; 

        break;

      case TAR_CHAR_DEFENSIVE:

        if ( arg2[0] == '\0' ) {
           victim = ch;
        } else {

          victim = get_char_room( ch, arg2 );

          if ( victim == NULL ) {
            send_to_char( "They are not here.\r\n", ch );
            return FALSE;
	  }
        } 

        if ( victim == ch ) {
          mcc = get_mcc(ch, ch, ch, NULL, NULL, NULL, spn, spell_array[spn].name);

          wev = get_wev(WEV_SPELL, WEV_SPELL_PREP, mcc,
                       "You prepare to cast '@t0' upon yourself.\r\n",
                        NULL,
                       "@a2 is preparing to cast a spell upon @a4self!\r\n");

        } else {
          mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, spn, spell_array[spn].name);
          wev = get_wev(WEV_SPELL, WEV_SPELL_PREP, mcc,
                       "You prepare to cast '@t0' upon @v2.\r\n",
                       "@a2 is preparing to cast a spell upon you!\r\n",
                       "@a2 is preparing to cast a spell upon @v2!\r\n");
        } 

        sprintf(buf, "casting '%s'.", spell_array[spn].name);
    
        break;

      case TAR_CHAR_ANYWHERE:

        if ( arg2[0] == '\0' ) {
           victim = ch;
        } else {

        victim = get_char_world(ch, arg2 );

          if ( victim == NULL ) {
            send_to_char( "They are not here.\r\n", ch );
            return FALSE;
          }
        } 

        if ( victim == ch ) {
          mcc = get_mcc(ch, ch, ch, NULL, NULL, NULL, spn, spell_array[spn].name);
          wev = get_wev(WEV_SPELL, WEV_SPELL_PREP, mcc,
                       "You prepare to cast '@t0' upon yourself.\r\n",
                        NULL,
                       "@a2 is preparing to cast a spell upon @a4self!\r\n");

        } else {
          mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, spn, spell_array[spn].name);

          wev = get_wev(WEV_SPELL, WEV_SPELL_PREP, mcc,
                       "You prepare to cast '@t0' upon @v2.\r\n",
                       "@a2 is preparing to cast a spell upon you!\r\n",
                       "@a2 is preparing to cast a spell upon @v2!\r\n");
        } 

        sprintf(buf, "casting '%s'.", spell_array[spn].name);
    
        break;


      case TAR_CHAR_SELF:
        if (arg2[0] != '\0' && !is_name(arg2, ch->name )
        && !spell_array[spn].parse_info) {
          send_to_char( "You cannot cast this spell on another.\r\n", ch );
          return FALSE;
        }

	victim = ch;

        mcc = get_mcc(ch, ch, ch, NULL, NULL, NULL, spn, spell_array[spn].name);

        wev = get_wev(WEV_SPELL, WEV_SPELL_PREP, mcc,
                     "You prepare to cast '@t0' upon yourself.\r\n",
                      NULL,
                     "@a2 is preparing to cast a spell upon @a4self!\r\n");

        sprintf(buf, "casting '%s'.", spell_array[spn].name);
    
	break;

      case TAR_OBJ_INV:

        if ( arg2[0] == '\0' ) {
          send_to_char( "What should the spell be cast upon?\r\n", ch );
          return FALSE;
        }

	obj = get_obj_carry( ch, arg2 );

	if ( obj == NULL ) {
  	  send_to_char( "You are not carrying that.\r\n", ch );
	  return FALSE;
	}

        mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, spn, spell_array[spn].name);
        wev = get_wev(WEV_SPELL, WEV_SPELL_PREP, mcc,
                     "You prepare to cast '@t0' upon @p2.\r\n",
                      NULL,
                     "@a2 is preparing to cast a spell upon @p2!\r\n");

        sprintf(buf, "casting '%s'.", spell_array[spn].name);
    
	break;

      case TAR_OBJ_ROOM:

        if ( arg2[0] == '\0' ) {
          send_to_char( "What should the spell be cast upon?\r\n", ch );
          return FALSE;
        }

        obj = get_obj_here( ch, arg2);

         if ( obj == NULL ) {
	    send_to_char( "You can't find it.\r\n", ch );
	    return FALSE;
         }

        mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, spn, spell_array[spn].name);
        wev = get_wev(WEV_SPELL, WEV_SPELL_PREP, mcc,
                     "You prepare to cast '@t0' upon @p2.\r\n",
                      NULL,
                     "@a2 is preparing to cast a spell upon @p2!\r\n");

        sprintf(buf, "casting '%s'.", spell_array[spn].name);
    
	break;
    }

   /* Challange... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return FALSE;
    }

   /* Send to command issuers room... */

    set_activity( ch, cast_pos, NULL, ACV_CASTING, "casting a spell.");

    room_issue_wev( ch->in_room, wev);

   /* Free the wev (will autofree the context)... */

    free_wev(wev);

   /* Set up the activity state engine... */

    set_activity_key(ch); 

    ch->acv_int = spn;
    
    switch (spell_array[spn].target) {

      default:
        break;

      case TAR_CHAR_OFFENSIVE:
      case TAR_CHAR_ANYWHERE:
      case TAR_CHAR_DEFENSIVE:
        free_string(ch->acv_text);
        ch->acv_text = str_dup(victim->true_name);
        break;  

      case TAR_OBJ_ROOM:
      case TAR_OBJ_INV:
        free_string(ch->acv_text);
        ch->acv_text = str_dup(obj->name);
        break;

      case TAR_IGNORE:
      case TAR_CHAR_SELF:
        free_string(ch->acv_text);
        ch->acv_text = str_dup(target_name);
        break;
    }
           
    return TRUE;
}


// -----------------------------------------------------------------------
// Check mana and component, begin casting...
//
//  context->actor				spell caster
//  context->actor->acv_int			spell number
//  context->actor->acv_text			spell target (ignore)
//  context->victim				spell target (other)
//  context->obj				spell target (obj inv) 
//
// -----------------------------------------------------------------------

bool do_cast_prepare(MOB_CMD_CONTEXT *context, SEQ_STEP *step) {
    CHAR_DATA *ch;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    OBJ_DATA *component = NULL;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;
    int spn;	
    int mana;

    ch = context->actor;

   /* Get spell number... */

    spn = ch->acv_int;

   /* Check out the component... */

    if ( spell_array[spn].component != NULL ) { 

      component = get_obj_carry(ch, spell_array[spn].component);
      if ( component == NULL ) {
        sprintf_to_char(ch, "You need to have the component '%s' to cast this.\r\n", spell_array[spn].component );
        return FALSE;
      }

      if ( component->item_type != ITEM_COMPONENT ) {
        sprintf_to_char(ch,"Your '%s' is not the correct component.\r\n",  component->short_descr );
        return FALSE;
      }
    }

    if (spell_array[spn].discipline > 0) {
        if (!knows_discipline(ch, spell_array[spn].discipline)) {
            sprintf_to_char(ch, "You need to know the discipline '%s' to cast this.\r\n", capitalize(flag_string(discipline_type, spell_array[spn].discipline)));
            return FALSE;
        }
    }

   /* Check out the casting cost... */

    mana = mana_cost(ch, spn);
    
    if (ch->acv_int3 == 0) {
        if ( ch->mana < mana ) {
           send_to_char( "You do not have enough mana.\r\n", ch );
           return FALSE;
        }
    } else {
        CHAR_DATA *fch = ch->in_room->people;
        char *fail = NULL;

         while (fch) {
             if (is_same_group(fch, ch)) {
                 if (fch->mana < mana) fail = strdup(fch->short_descr);
             }
             fch = fch->next_in_room;
         }
         if (fail) {
            fch = ch->in_room->people;
            while (fch) {
                if (is_same_group(fch, ch)) {
                      sprintf_to_char(fch, "%s lacks the mana.\r\n", capitalize(fail));
                }
                fch = fch->next_in_room;
            }
             return FALSE;
         }
    }

   /* Tell the world what is happenening... */

    victim	= NULL;
    obj		= NULL;
      
    switch ( spell_array[spn].target ) {
     
      default:
        bug( "Do_cast: bad target for spn %d.", spn );
        return FALSE;

      case TAR_IGNORE:

        mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, spn, spell_array[spn].name);
        wev = get_wev(WEV_SPELL, WEV_SPELL_START, mcc,
                     "You begin to cast '@t0'.\r\n",
                      NULL,
                     "@a2 is begining to cast a spell!\r\n");
        break;


      case TAR_CHAR_OFFENSIVE:

        if (IS_AFFECTED(ch, AFF_FEAR)) {
          send_to_char ("You are too scared to attack anyone...\r\n",ch);
          return FALSE;
        }

        victim = context->victim;

        if ( IS_AFFECTED(ch, AFF_CHARM) 
        && ch->master == victim ) {
	  send_to_char( "You decide you don't want to hurt them...\r\n", ch );
                  return FALSE;
        }

        mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, spn, spell_array[spn].name);
        wev = get_wev(WEV_SPELL, WEV_SPELL_START, mcc,
                     "You begin casting '@t0' at @v2.\r\n",
                     "@a2 begins casting a spell at you!\r\n",
                     "@a2 begins casting a spell at @v2!\r\n");

        react_to_cast(victim, ch);
        break;


      case TAR_CHAR_ANYWHERE:
      case TAR_CHAR_DEFENSIVE:
        victim = context->victim;

        if ( victim == ch ) {
          mcc = get_mcc(ch, ch, ch, NULL, NULL, NULL, spn, spell_array[spn].name);
          wev = get_wev(WEV_SPELL, WEV_SPELL_START, mcc,
                       "You begin casting '@t0' upon yourself.\r\n",
                        NULL,
                       "@a2 begins casting a spell upon @a4self!\r\n");

        } else {
          mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, spn, spell_array[spn].name);
          wev = get_wev(WEV_SPELL, WEV_SPELL_START, mcc,
                       "You begin casting '@t0' upon @v2.\r\n",
                       "@a2 begins casting a spell upon you!\r\n",
                       "@a2 begins casting a spell upon @v2!\r\n");
        } 
        break;


      case TAR_CHAR_SELF:

        mcc = get_mcc(ch, ch, ch, NULL, NULL, NULL, spn, spell_array[spn].name);
        wev = get_wev(WEV_SPELL, WEV_SPELL_START, mcc,
                     "You begin casting '@t0' upon yourself.\r\n",
                      NULL,
                     "@a2 begins casting a spell upon @a4self!\r\n");
        break;


     case TAR_OBJ_ROOM:
     case TAR_OBJ_INV:

       obj = context->obj;

       mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, spn, spell_array[spn].name);
       wev = get_wev(WEV_SPELL, WEV_SPELL_START, mcc,
                    "You begin casting '@t0' upon @p2.\r\n",
                     NULL,
                    "@a2 begins casting a spell upon @p2!\r\n");
       break;
    }

   /* Challange... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
      return FALSE;
    } 
 
   /* Spell conditions... */

    if (!ec_satisfied_mcc(mcc, spell_array[spn].cond, TRUE)) {
      free_wev(wev);
      send_to_char("Something is not right!\r\n", ch);
      return FALSE;
    } 
 
   /* Send the message out... */

    room_issue_wev( ch->in_room, wev);

    free_wev(wev);

   /* Set up the activity state engine... */

    if ( ch->activity != ACV_CASTING ) {
      return FALSE;
    }

    ch->acv_int2 = mana;

    return TRUE;   
}


// -----------------------------------------------------------------------
// Chant mystic Cthonic phrases...
//
//  context->actor			spell caster
//  context->actor->acv_int2		spell mana
//
//  step->sparms[0]			text to chant
//
// -----------------------------------------------------------------------

bool do_cast_chant(MOB_CMD_CONTEXT *context, SEQ_STEP *step) {
    CHAR_DATA *ch;
    int spn;

    ch = context->actor;

   /* Get spell number... */

    spn = ch->acv_int; 

   /* Say the magic words... */

    if ( spn != get_spell_spn("vocalize") ) {

      if (( is_affected(ch, gafn_mute)           
      || IS_RAFFECTED(ch->in_room, RAFF_SILENCE))
        && spn != get_spell_spn("cancellation")
        && !is_affected(ch, gafn_vocalize) ) {
        send_to_char ("You cannot say the words!\r\n",ch);
        return FALSE;
      }  
  
      say_spell(ch, step->sparms[0]);
    }

   /* Set up the activity state engine... */

    if ( ch->activity != ACV_CASTING ) return FALSE;

    return TRUE;
}


bool do_cast_yell(MOB_CMD_CONTEXT *context, SEQ_STEP *step) {
    CHAR_DATA *ch;
    int spn;

    ch = context->actor;

   /* Get spell number... */

    spn = ch->acv_int; 

   /* Say the magic words... */

    if ( spn != get_spell_spn("vocalize") ) {

      if (( is_affected(ch, gafn_mute)           
      || IS_RAFFECTED(ch->in_room, RAFF_SILENCE))
        && spn != get_spell_spn("cancellation")
        && !is_affected(ch, gafn_vocalize) ) {
        send_to_char ("You cannot say the words!\r\n",ch);
        return FALSE;
      }  
  
      yell_spell(ch, step->sparms[0]);
    }

   /* Set up the activity state engine... */

    if ( ch->activity != ACV_CASTING ) return FALSE;

    return TRUE;
}


// -----------------------------------------------------------------------
// Utter a word of power!
//
//  context->actor			spell caster
//  context->text			user entered powerword
//
//  step->sparms[0]			text to chant
//
// -----------------------------------------------------------------------

bool do_cast_power_word(MOB_CMD_CONTEXT *context, SEQ_STEP *step) {

    CHAR_DATA *ch;

    int spn;

    MOB_CMD_CONTEXT *mcc;
    WEV *wev;
 
    char *text;

   /* Find the caster... */

    ch = context->actor;

   /* Get spell number... */

    spn = ch->acv_int; 

   /* Say the magic words... */

    if ( is_affected(ch, gafn_mute)
    || IS_RAFFECTED(ch->in_room, RAFF_SILENCE)) {
      send_to_char ("You cannot say the words!\r\n",ch);
      return FALSE;
    }  

    text = step->sparms[0];

    if ( text[0] == '\0' ) {
      text = context->text;
    }

    if ( text == NULL 
      || text[0] == '\0' ) {
      text = ch->acv_text;
    }

    if ( text == NULL
      || text[0] == '\0' ) {
      switch (number_bits(3)) {

        case 0: 
          text = "{Rthe FORBIDDEN name!{Y";
          break;

        case 1:
          text = "{MI love you!{Y";
          break;

        case 2:
          text = "{ROh shit!{Y";
          break;

        case 3:
          text = "{GFor a good time call 321-6789!{Y";
          break;

        case 4:
          text = "{CMade to make your mouth water!{Y";
          break;

        case 5:
          text = "{CI hate this!{Y";
          break;

        case 6:
          text = "{GBlaaarrrggghhhhh!{Y";
          break;

        case 7:
          text = "{MCthulhu! Ia! Ia! Cthulhu!{Y";
          break;

        default:
          text = "Errr. Ummm...Oh-oh!";
          break; 
      } 
    }
  
   /* Tell everyone else... */

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 
                                           gsn_cthonic, text);

    wev = get_wev(WEV_SPELL, WEV_SPELL_POWER, mcc,
                 "{YYou utter the words of power: '@t2'{x\r\n",
                  NULL,
                 "{Y@a2 utters the words of power: '@t2'{x\r\n");

    room_issue_wev( ch->in_room, wev);

    free_wev(wev);

   /* Set up the activity state engine... */

    if ( ch->activity != ACV_CASTING ) {
      return FALSE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------
// Send a message around...
//
//  context->actor			spell caster
//  context->actor->acv_int		spell number
//
//  step->sparms[0]			text to echo to caster
//  step->sparms[1]			text to echo to victim
//  step->sparms[2]			text to echo to observers
//  step->iparms[0]			scope of echo (WEV_SCOPE_	
//  step->iparms[1]			echo focus (caster/victim)
//
// -----------------------------------------------------------------------

bool do_cast_echo(MOB_CMD_CONTEXT *context, SEQ_STEP *step) {

    char message[3][MAX_STRING_LENGTH];
    
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;    

   /* Format messages... */

    message[0][0] = '\0';
    message[1][0] = '\0';
    message[2][0] = '\0';
 
    if ( step->sparms[0] != NULL ) {
      sprintf(message[0], "{m%s{x\r\n", step->sparms[0]);
    } else {

      if (context->actor == context->victim) sprintf(message[0], "{m%s{x\r\n", step->sparms[1]);
      else sprintf(message[0], "{m%s{x\r\n", step->sparms[2]);
    }

    if ( step->sparms[1] != NULL ) sprintf(message[1], "{m%s{x\r\n", step->sparms[1]);

    if ( step->sparms[2] != NULL ) sprintf(message[2], "{m%s{x\r\n", step->sparms[2]);

   /* Build WEV... */

    mcc = get_mcc(context->actor, context->actor, context->victim, NULL, context->obj, NULL, context->actor->acv_int, NULL);
    wev = get_wev(WEV_SPELL, WEV_SPELL_ECHO, mcc,
                  message[0],
                  message[1],
                  message[2]);

   /* Issue with appopriate focus... */

    if ( step->iparms[1] != STEP_FCS_VICTIM ) {

      area_issue_wev(context->actor->in_room, wev, step->iparms[0]);

    } else { 

      if ( context->victim == NULL ) return FALSE;
      area_issue_wev(context->victim->in_room, wev, step->iparms[0]);
    }

   /* Lose the WEV... */

    free_wev(wev);

   /* All done... */

    return TRUE;
}

// -----------------------------------------------------------------------
// Actually cast the spell and consume its components...
//
//  context->actor				spell caster
//  context->actor->acv_int			spell number
//  context->actor->acv_int2			spell mana
//  context->actor->acv_text			spell target (ignore)
//  context->victim				spell target (other)
//  context->obj				spell target (obj inv) 
//
// -----------------------------------------------------------------------

bool do_cast_consume(MOB_CMD_CONTEXT *context, SEQ_STEP *step) {
CHAR_DATA *ch;
CHAR_DATA *victim;
OBJ_DATA *obj;
OBJ_DATA *component = NULL;
int sn; 
int hpp;
int cast_skill;
int cast_roll;
int cast_chal;
int cast_base;
MOB_CMD_CONTEXT *mcc;
WEV *wev;
char buf[MAX_STRING_LENGTH];
int spn;

    ch = context->actor;

   /* Get spell number... */

    spn = ch->acv_int; 

   /* Magic doesn't work in some area... */

    if ( ch->in_room != NULL
      && ch->in_room->area != NULL
      && ( IS_SET(ch->in_room->area->area_flags, AREA_NOMAGIC)
        || IS_SET(ch->in_room->room_flags, ROOM_NOMAGIC)
        || IS_SET(zones[ch->in_room->area->zone].flags, ZONE_NOMAGIC) )) {
 
      mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, spn, spell_array[spn].name);
      wev = get_wev(WEV_SPELL, WEV_SPELL_FAIL, mcc,
                   "You cannot feel the magic...\r\n",
                    NULL,
                   "@a2 looks puzzeled!\r\n");

      room_issue_wev( ch->in_room, wev);
      free_wev(wev);
      return FALSE;
    }

   /* Find casting skill... */
    cast_skill = get_skill(ch, gsn_spell_casting);

   /* Modifiers... */

    if (get_eq_char(ch, WEAR_WIELD) == NULL) cast_skill += 10;

    if (get_eq_char(ch, WEAR_HOLD) == NULL) cast_skill += 15;

   /* Find challange... */

    cast_base = 0;		// Target for roll to beat
    cast_chal = 0;		// Reduction in roll for difficulty

   /* Start based upon the target... */

    victim	= NULL;
    obj		= NULL;

    switch (spell_array[spn].target) {

      case TAR_IGNORE:
        cast_base =  75;
        cast_chal =   0;
        break;
	 
      case TAR_CHAR_OFFENSIVE:

        if (IS_AFFECTED(ch, AFF_FEAR)) {
          send_to_char ("You are too scared to attack anyone...\r\n",ch);
          return FALSE;
        }

        victim = context->victim;

        cast_base = 90; 
        cast_chal = victim->level;

        if ( ch->fighting != NULL
        && victim->fighting != ch ) {
          cast_chal += 25;
        }

        break;

      case TAR_CHAR_ANYWHERE:
      case TAR_CHAR_DEFENSIVE:
        victim = context->victim;

        if (victim == ch) {
          cast_base = 50;
          cast_chal =  0;
        } else {
          cast_base = 75;
          cast_chal = victim->level / 2;
        }

        break; 

      case TAR_CHAR_SELF:

        victim = ch;

        cast_base = 50;
        cast_chal =  0;

        break;

      case TAR_OBJ_ROOM:
      case TAR_OBJ_INV:
        obj = context->obj;
        cast_base = 75;
        cast_chal =  0;
        break;

      default:
        cast_base = 100;
        cast_chal =   0;
    }

   /* Modifiers... */

    if (ch->fighting != NULL) cast_chal += 10;
    if (get_eq_char(ch, WEAR_SHIELD) != NULL) cast_chal += 20;
    if (get_eq_char(ch, WEAR_WIELD2) != NULL) cast_chal += 15;

    if (is_affected(ch, gafn_blindness)) cast_chal += 25;

    hpp = (100 * ch->hit)/ch->max_hit;

    if (hpp < 50) {
      cast_chal += 5;

      if (hpp < 30) {
        cast_chal += 5;

        if (hpp < 15) cast_chal += 5;
      }
    }

    switch (ch->in_room->sector_type) {

      case SECT_FIRE:
      case SECT_SMALL_FIRE:
      case SECT_BIG_FIRE:
      case SECT_SPACE:
      case SECT_COLD:
      case SECT_ACID:
        if (!IS_AFFECTED(ch, AFF_SANCTUARY)) cast_chal += 15;
        break;

      case SECT_UNDERWATER:
        cast_chal += 25;
        break; 

      default:
        break;
    }

    cast_chal += spell_array[spn].diff;

   /* Modify difficulty for area/zone magic capabilities... */

    if ( ch->in_room != NULL
    && ch->in_room->area != NULL) {

     /* First room... */

     if (IS_RAFFECTED(ch->in_room, RAFF_LOW_MAGIC)) cast_chal *=2;
     if (IS_RAFFECTED(ch->in_room, RAFF_HIGH_MAGIC)) cast_chal /=2;

     /* Then area... */

      if ( IS_SET(ch->in_room->area->area_flags, AREA_LOWMAGIC) ) cast_chal *= 2;
      else if ( IS_SET(ch->in_room->area->area_flags, AREA_HIGHMAGIC) ) cast_chal /= 2;
      else if ( IS_SET(ch->in_room->area->area_flags, AREA_SUPERMAGIC) || IS_ARTIFACTED(ch, ART_NECRONOMICON)) cast_chal /= 3;

     /* Then zone... */

      if ( IS_SET(zones[ch->in_room->area->zone].flags, ZONE_LOWMAGIC) ) cast_chal *= 2;
      else if ( IS_SET(zones[ch->in_room->area->zone].flags, ZONE_HIGHMAGIC) ) cast_chal /= 2;
      else if ( IS_SET(zones[ch->in_room->area->zone].flags, ZONE_SUPERMAGIC) ) cast_chal /= 3;
    }

    if (spell_array[spn].music_style > 0) {
        int mpower;

        mpower = get_room_music(ch->in_room, spell_array[spn].music_style);

        if (mpower < spell_array[spn].music_power) {
           mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, spn, spell_array[spn].name);
           wev = get_wev(WEV_SPELL, WEV_SPELL_FAIL, mcc,
                   "You lack the harmony!\r\n",
                    NULL,
                   "@a2 lacks the harmony!\r\n");

           room_issue_wev( ch->in_room, wev);
           free_wev(wev);
           check_improve(ch, gsn_spell_casting, FALSE, 2);
           return FALSE;
        }

        mpower -= spell_array[spn].music_power;
        cast_chal = UMAX(0, cast_chal - mpower/2);
    }

   /* Roll time... */

    cast_roll = cast_skill + number_open() + ch->level - ch->acv_int2/2;

   /* See if the casting worked... */

    cast_roll = cast_roll - cast_chal;

    if (cast_roll < cast_base) {

      mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, spn, spell_array[spn].name);
      wev = get_wev(WEV_SPELL, WEV_SPELL_FAIL, mcc,
                   "You lose concentration...\r\n",
                    NULL,
                   "@a2 loses concentration!\r\n");

      room_issue_wev( ch->in_room, wev);
      free_wev(wev);
      check_improve(ch, gsn_spell_casting, FALSE, 2);
      return FALSE;
    } 

   /* Ok, spell was cast ok... */

    if (ch->acv_int3 == 0) {
         ch->mana -= ch->acv_int2;
         check_improve(ch, gsn_spell_casting, TRUE, 2);
    } else {
         CHAR_DATA *fch = ch->in_room->people;

         while (fch) {
             if (is_same_group(fch, ch)) {
                 fch->mana -= ch->acv_int2;
                 check_improve(ch, gsn_spell_casting, TRUE, 1);
            }
             fch = fch->next_in_room;
         }
    }
   /* Locate and consume the spell component... */

    if ( spell_array[spn].component != NULL ) { 

      component = get_obj_carry(ch, spell_array[spn].component);

      if ( component == NULL ) {
        sprintf( buf, "You no longer have your '%s'!\r\n", spell_array[spn].component );
        send_to_char(buf,ch);
        return FALSE;
      }

      if ( component->item_type != ITEM_COMPONENT ) {
        sprintf_to_char(ch, "Your '%s' is not the correct component.\r\n",component->short_descr );
        return FALSE;
      }

      mcc = get_mcc(ch, ch, NULL, NULL, component, NULL, spn, spell_array[spn].name);
      wev = get_wev(WEV_SPELL, WEV_SPELL_CONSUME, mcc,
                   "Your @p2 is consumed by blue flames!\r\n",
                    NULL,
                   "@a2s @p2 is consumed by blue flames!\r\n");

      room_issue_wev( ch->in_room, wev);
      free_wev(wev);
      extract_obj(component);
    } 

   /* Now, what does it do... */

    sn = get_skill_sn(spell_array[spn].name);

   /* Work out the spell roll... */

    spell_roll = cast_roll - cast_base + spell_skill(ch, spn) + number_open();

   /* See what happened with the spell... */

    if (spell_roll < 95) {

      mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, spn, spell_array[spn].name);
      wev = get_wev(WEV_SPELL, WEV_SPELL_FAIL, mcc,
                   "Your '@t0' spell fails...\r\n",
                    NULL,
                   "@a2s spell fails.\r\n");

      room_issue_wev( ch->in_room, wev);
      free_wev(wev);
      check_improve_spell(ch, spn, FALSE);
      return FALSE;
    } 

   /* Success! */

    check_improve_spell(ch, spn, TRUE);

   /* Tell the world what happened... */ 

    switch ( spell_array[spn].target ) {
     
      default:
        bug( "Do_cast: bad target for sn %d.", sn );
        return FALSE;

      case TAR_IGNORE:

        mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, spn, spell_array[spn].name);
        wev = get_wev(WEV_SPELL, WEV_SPELL_CAST, mcc,
                     "You cast '@t0'.\r\n",
                      NULL,
                     "@a2 casts '@t2'!\r\n");

        break;

      case TAR_CHAR_OFFENSIVE:

        mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, spn, spell_array[spn].name);
        wev = get_wev(WEV_SPELL, WEV_SPELL_CAST, mcc,
                     "You cast '@t0' at @v2.\r\n",
                     "@a2 casts '@t2' at you!\r\n",
                     "@a2 casts '@t2' at @v2!\r\n");

        break;

      case TAR_CHAR_ANYWHERE:
      case TAR_CHAR_DEFENSIVE:

        if ( victim == ch ) {
          mcc = get_mcc(ch, ch, ch, NULL, NULL, NULL, spn, spell_array[spn].name);
          wev = get_wev(WEV_SPELL, WEV_SPELL_CAST, mcc,
                       "You cast '@t0' upon yourself.\r\n",
                        NULL,
                       "@a2 casts '@t2' upon @a4self!\r\n");

        } else {
          mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, spn, spell_array[spn].name);
          wev = get_wev(WEV_SPELL, WEV_SPELL_CAST, mcc,
                       "You cast '@t0' upon @v2.\r\n",
                       "@a2 casts '@t2' upon you!\r\n",
                       "@a2 casts '@t2' upon @v2!\r\n");
        } 

        break;

      case TAR_CHAR_SELF:

        mcc = get_mcc(ch, ch, ch, NULL, NULL, NULL, spn, spell_array[spn].name);
        wev = get_wev(WEV_SPELL, WEV_SPELL_CAST, mcc,
                     "You cast '@t0' upon yourself.\r\n",
                      NULL,
                     "@a2 casts '@t2' upon @a4self!\r\n");

        break;

      case TAR_OBJ_ROOM:
      case TAR_OBJ_INV:

        mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, spn, spell_array[spn].name);
        wev = get_wev(WEV_SPELL, WEV_SPELL_CAST, mcc,
                     "You cast '@t0' upon @p2.\r\n",
                      NULL,
                     "@a2 casts '@t2' upon @p2!\r\n");

        break;
    }

    room_issue_wev( ch->in_room, wev);

    free_wev(wev);

   /* Set up the activity state engine... */

    if ( ch->activity != ACV_CASTING ) return FALSE;
    ch->acv_int2 = spell_roll;
    return TRUE;
}


// -----------------------------------------------------------------------
// Work out a characters level adjusted for magic density...
// -----------------------------------------------------------------------

int casting_level(CHAR_DATA *ch) {
OBJ_DATA *obj;
AFFECT_DATA *paf;
int level;

    if (ch->acv_int3 == 0) level = ch->level;
    else level = ch->acv_int3;

    if ( ch->in_room != NULL && ch->in_room->area != NULL) {

     /* First room... */

         if (IS_RAFFECTED(ch->in_room, RAFF_LOW_MAGIC)) level -= number_range(1, level/2);
         if (IS_RAFFECTED(ch->in_room, RAFF_HIGH_MAGIC)) level += number_range(1, level);


     /* Then area... */

         if ( IS_SET(ch->in_room->area->area_flags, AREA_LOWMAGIC) ) { 
              level -= number_range(1, level/2);
         } else if ( IS_SET(ch->in_room->area->area_flags, AREA_HIGHMAGIC) ) { 
              level += number_range(1, level);
         } else if ( IS_SET(ch->in_room->area->area_flags, AREA_SUPERMAGIC) || IS_ARTIFACTED(ch, ART_NECRONOMICON)) { 
              level += number_range(1, 2 * level);
         }
 
     /* Then zone... */

         if ( IS_SET(zones[ch->in_room->area->zone].flags, ZONE_LOWMAGIC) ) { 
              level -= number_range(1, level/2);
         } else if ( IS_SET(zones[ch->in_room->area->zone].flags, ZONE_HIGHMAGIC) ) { 
              level += number_range(1, level);
         } else if ( IS_SET(zones[ch->in_room->area->zone].flags, ZONE_SUPERMAGIC) ) { 
              level += number_range(1, 2 * level);
         }
    }

    for (obj = ch->carrying; obj; obj = obj->next_content) {
           if (obj->wear_loc == WEAR_NONE) continue;

           if (obj->enchanted) {
               for (paf = obj->affected; paf; paf = paf->next ) {
                     if (paf->location == APPLY_MAGIC) level += paf->modifier;
               }
           } else {
               for (paf = obj->pIndexData->affected; paf; paf = paf->next ) {
                     if (paf->location == APPLY_MAGIC) level += paf->modifier;
               }
           }
    }
    return UMAX(level, 1);
}


// -----------------------------------------------------------------------
// Create the effect of the spell...
//
//  context->actor				spell caster
//  context->actor->acv_int			spell number
//  context->actor->acv_int2			spell casting roll
//  context->actor->acv_text			spell target (ignore)
//  context->victim				spell target (other)
//  context->obj				spell target (obj inv) 
//  step->iparms[0]				effect number
//
// -----------------------------------------------------------------------

bool do_cast_effect(MOB_CMD_CONTEXT *context, SEQ_STEP *step) {
    CHAR_DATA *ch;
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    void *vo;
    int sn, spn, efn, mefn, level;

    ch = context->actor;

   /* Get spell number... */

    spn = ch->acv_int; 

    sn = spell_array[spn].sn;

    efn = step->iparms[0];

    spell_roll		= ch->acv_int2;
    target_name 		= NULL; 
    victim			= NULL;
    obj			= NULL;
    vo			= NULL;
    mefn = efn;

    switch(spell_array[spn].range) {
         case WEV_SCOPE_NONE:
               switch ( effect_array[efn].target ) {
                   default:
                      bug( "Do_cast: bad target for efn %d.", efn );
                      return FALSE;

                   case TAR_IGNORE:
                      target_name = strdup(ch->acv_text);
                      break;

                   case TAR_CHAR_OFFENSIVE:
                       if (IS_ARTIFACTED(context->victim, ART_MIST)) {
                           send_to_char("{mA mysterious power interrupts your spell!{x\r\n", ch);
                           return FALSE;
                       }

                       if (!IS_NPC(context->victim)) {
                           if (context->victim->pcdata->pk_timer > 0) {
                                sprintf_to_char(ch, "%s is protected against PK at the moment.\r\n", context->victim->name);
                                return FALSE;
                           }
                       }

                       if (!IS_NPC(context->victim) && !IS_NPC(ch) && !IS_HERO(context->victim)) {
                           int ldif = ch->level - context->victim->level + 30;

                           if (ldif > 50) {
                                if (!check_sanity(ch, ldif)) send_to_char("You are horrified by your intentions!\r\n", ch);
                           }
                       }


                   case TAR_CHAR_ANYWHERE:
                   case TAR_CHAR_DEFENSIVE:
                       victim = context->victim;
                       target_name = strdup(victim->name);
                       vo = (void *) victim;
                       break;

                   case TAR_CHAR_SELF:
                       victim = ch;
                       if (spell_array[spn].parse_info) target_name = strdup(ch->acv_text);
                       else target_name = strdup(victim->name);
                       vo = (void *) victim;
                       break;

                   case TAR_OBJ_ROOM:
                   case TAR_OBJ_INV:
                       obj = context->obj;
                       target_name = strdup(obj->short_descr);
                       vo = (void *) obj;
                       break;
               }

   /* Adjust spell casting level for area/zone magic capabilities... */

               level = casting_level(ch);

   /* Apply the spell... */

               if (IS_RAFFECTED(ch->in_room, RAFF_WILD_MAGIC)) efn=wild_magic(ch,efn,spn);
               if ( spell_array[spn].target == TAR_CHAR_OFFENSIVE ) spell_CDB = getCDB(ch, victim, sn, FALSE, sn, 0);
               in_seq = TRUE;

               if (spell_array[spn].info != NULL) {
                    free_string(target_name);
                    target_name = str_dup(spell_array[spn].info);
               }

               (*effect_array[efn].spell_fun) ( sn, level, ch, vo );
               break;

         case WEV_SCOPE_ROOM:     
               switch ( effect_array[efn].target ) {
                   default:
                      bug( "Do_cast: bad target for efn %d.", efn );
                      return FALSE;
                   
                   case TAR_CHAR_OFFENSIVE:
                   case TAR_CHAR_ANYWHERE:
                   case TAR_CHAR_DEFENSIVE:
                        for ( vch = char_list; vch != NULL; vch = vch_next )    {
	             vch_next = vch->next;

	             if ( vch->in_room == NULL ) continue;
                             if (vch == ch) continue;
              	             if ( vch->in_room == ch->in_room) {
                                        victim = vch;
                                        target_name = strdup(vch->name);
                                        vo = (void *) vch;

                                        level = casting_level(ch);

                                        if (IS_RAFFECTED(ch->in_room, RAFF_WILD_MAGIC)) mefn=wild_magic(ch,efn,spn);
                                        if ( spell_array[spn].target == TAR_CHAR_OFFENSIVE ) spell_CDB = getCDB(ch, victim, sn, FALSE, sn, 0);
                                        in_seq = TRUE;

                                        if (spell_array[spn].info != NULL) {
                                             free_string(target_name);
                                             target_name = str_dup(spell_array[spn].info);
                                        }
                                       (*effect_array[mefn].spell_fun) ( sn, level, ch, vo );
                             }
                        }
                        break;

	   case TAR_OBJ_ROOM:
                        for (obj = object_list; obj; obj = obj_next )    {
	             obj_next = obj->next;

	             if (obj->in_room == NULL) continue;

              	             if (obj->in_room == ch->in_room) {
                                   target_name = strdup(obj->short_descr);
                                   vo = (void *) obj;

                                   level = casting_level(ch);
                                   in_seq = TRUE;

                                   if (spell_array[spn].info != NULL) {
                                        free_string(target_name);
                                        target_name = str_dup(spell_array[spn].info);
                                   }
                                   (*effect_array[mefn].spell_fun) ( sn, level, ch, vo );
                             }
                        }
                        break;
                }
                break;

         case WEV_SCOPE_SUBAREA:     
                switch ( effect_array[efn].target ) {
                     default:
                        bug( "Do_cast: bad target for efn %d.", efn );
                        return FALSE;

                     case TAR_CHAR_OFFENSIVE:
                     case TAR_CHAR_ANYWHERE:
                     case TAR_CHAR_DEFENSIVE:
                           for ( vch = char_list; vch != NULL; vch = vch_next )    {
	                vch_next = vch->next;

	                if ( vch->in_room == NULL ) continue;
                                if (vch == ch) continue;

              	                if ( vch->in_room->area == ch->in_room->area
                                && vch->in_room->subarea == ch->in_room->subarea) {
                                        victim = vch;
                                        target_name = strdup(vch->name);
                                        vo = (void *) vch;

                                        level = casting_level(ch);

                                        if (IS_RAFFECTED(ch->in_room, RAFF_WILD_MAGIC)) mefn=wild_magic(ch,efn,spn);
                                        if ( spell_array[spn].target == TAR_CHAR_OFFENSIVE ) spell_CDB = getCDB(ch, victim, sn, FALSE, sn, 0);
                                        in_seq = TRUE;

                                        if (spell_array[spn].info != NULL) {
                                            free_string(target_name);
                                            target_name = str_dup(spell_array[spn].info);
                                        }

                                        (*effect_array[mefn].spell_fun) ( sn, level, ch, vo );
                                }
                           }
                           break;

	   case TAR_OBJ_ROOM:
                        for (obj = object_list; obj; obj = obj_next )    {
	             obj_next = obj->next;

	             if (obj->in_room == NULL) continue;

              	             if (obj->in_room->area == ch->in_room->area
                             && obj->in_room->subarea == ch->in_room->subarea) {
                                   target_name = strdup(obj->short_descr);
                                   vo = (void *) obj;

                                   level = casting_level(ch);
                                   in_seq = TRUE;

                                   if (spell_array[spn].info != NULL) {
                                       free_string(target_name);
                                       target_name = str_dup(spell_array[spn].info);
                                   }

                                   (*effect_array[mefn].spell_fun) ( sn, level, ch, vo );
                             }
                        }
                        break;
                }
                break;

         case WEV_SCOPE_AREA:     
                switch ( effect_array[efn].target ) {
                      default:
                           bug( "Do_cast: bad target for efn %d.", efn );
                            return FALSE;

                      case TAR_CHAR_OFFENSIVE:
                      case TAR_CHAR_ANYWHERE:
                      case TAR_CHAR_DEFENSIVE:
                             for ( vch = char_list; vch != NULL; vch = vch_next )    {
	                    vch_next = vch->next;

	                    if ( vch->in_room == NULL ) continue;
                                    if (vch == ch) continue;

              	                    if ( vch->in_room->area == ch->in_room->area) {
                                        victim = vch;
                                        target_name = strdup(vch->name);
                                        vo = (void *) vch;

                                        level = casting_level(ch);

                                        if (IS_RAFFECTED(ch->in_room, RAFF_WILD_MAGIC)) mefn=wild_magic(ch,efn,spn);
                                        if ( spell_array[spn].target == TAR_CHAR_OFFENSIVE ) spell_CDB = getCDB(ch, victim, sn, FALSE, sn, 0);
                                        in_seq = TRUE;

                                        if (spell_array[spn].info != NULL) {
                                             free_string(target_name);
                                             target_name = str_dup(spell_array[spn].info);
                                        }

                                         (*effect_array[mefn].spell_fun) ( sn, level, ch, vo );
                                    }
                             }
                             break;

	      case TAR_OBJ_ROOM:
                             for (obj = object_list; obj; obj = obj_next )    {
	                 obj_next = obj->next;

	                  if (obj->in_room == NULL) continue;

              	                  if (obj->in_room->area == ch->in_room->area) {
                                       target_name = strdup(obj->short_descr);
                                       vo = (void *) obj;

                                       level = casting_level(ch);
                                       in_seq = TRUE;

                                       if (spell_array[spn].info != NULL) {
                                           free_string(target_name);
                                           target_name = str_dup(spell_array[spn].info);
                                       }

                                       (*effect_array[mefn].spell_fun) ( sn, level, ch, vo );
                                 }
                            }
                            break;
                }
                break;
                     
             case WEV_SCOPE_ZONE:     
                  switch ( effect_array[efn].target ) {
                        default:
                            bug( "Do_cast: bad target for efn %d.", efn );
                            return FALSE;

                        case TAR_CHAR_OFFENSIVE:
                        case TAR_CHAR_ANYWHERE:
                        case TAR_CHAR_DEFENSIVE:
                              for (vch = char_list; vch != NULL; vch = vch_next )    {
	                    vch_next = vch->next;

	                    if ( vch->in_room == NULL ) continue;
                                    if (vch == ch) continue;
              	                    if ( vch->in_room->area->zone == ch->in_room->area->zone) {
                                        victim = vch;
                                        target_name = strdup(vch->name);
                                        vo = (void *) vch;
      
                                        level = casting_level(ch);

                                        if (IS_RAFFECTED(ch->in_room, RAFF_WILD_MAGIC)) mefn=wild_magic(ch,efn,spn);
                                        if ( spell_array[spn].target == TAR_CHAR_OFFENSIVE ) spell_CDB = getCDB(ch, victim, sn, FALSE, sn, 0);
                                        in_seq = TRUE;

                                        if (spell_array[spn].info != NULL) {
                                            free_string(target_name);
                                            target_name = str_dup(spell_array[spn].info);
                                        }

                                        (*effect_array[mefn].spell_fun) ( sn, level, ch, vo );
                                    }
                              }
                              break;

	      case TAR_OBJ_ROOM:
                             for (obj = object_list; obj; obj = obj_next )    {
	                 obj_next = obj->next;

	                  if (obj->in_room == NULL) continue;

              	                  if (obj->in_room->area->zone == ch->in_room->area->zone) {
                                       target_name = strdup(obj->short_descr);
                                       vo = (void *) obj;

                                       level = casting_level(ch);
                                       in_seq = TRUE;

                                       if (spell_array[spn].info != NULL) {
                                           free_string(target_name);
                                           target_name = str_dup(spell_array[spn].info);
                                       }

                                       (*effect_array[mefn].spell_fun) ( sn, level, ch, vo );
                                 }
                            }
                            break;
                }
                break;

         case WEV_SCOPE_UNIVERSE:     
              switch ( effect_array[efn].target ) {
                  default:
                     bug( "Do_cast: bad target for efn %d.", efn );
                     return FALSE;

              case TAR_CHAR_OFFENSIVE:
              case TAR_CHAR_ANYWHERE:
              case TAR_CHAR_DEFENSIVE:
                    for ( vch = char_list; vch != NULL; vch = vch_next )    {
	          vch_next = vch->next;

	          if ( vch->in_room == NULL ) continue;
                          if (vch == ch) continue;
                          victim = vch;
                          target_name = strdup(vch->name);
                          vo = (void *) vch;

                          level = casting_level(ch);

                          if (IS_RAFFECTED(ch->in_room, RAFF_WILD_MAGIC)) mefn=wild_magic(ch,efn,spn);
                          if ( spell_array[spn].target == TAR_CHAR_OFFENSIVE ) spell_CDB = getCDB(ch, victim, sn, FALSE, sn, 0);
                          in_seq = TRUE;

                          if (spell_array[spn].info != NULL) {
                             free_string(target_name);
                             target_name = str_dup(spell_array[spn].info);
                          }

                          (*effect_array[mefn].spell_fun) ( sn, level, ch, vo );
                    }
                    break;

	    case TAR_OBJ_ROOM:
                             for (obj = object_list; obj; obj = obj_next )    {
	                 obj_next = obj->next;

	                  if (obj->in_room == NULL) continue;
                                  obj = context->obj;
                                  target_name = strdup(obj->short_descr);
                                  vo = (void *) obj;

                                  level = casting_level(ch);
                                  in_seq = TRUE;

                                  if (spell_array[spn].info != NULL) {
                                       free_string(target_name);
                                       target_name = str_dup(spell_array[spn].info);
                                  }

                                  (*effect_array[mefn].spell_fun) ( sn, level, ch, vo );
                            }
                            break;
                }
                break;
   }

   in_seq = FALSE; 

   /* victim is no longer valid if the damage killed them... */
    victim = NULL;

   /* Set up the activity state engine... */

    if ( ch->activity != ACV_CASTING ) return FALSE;
    return TRUE;
}


// -----------------------------------------------------------------------
// Send a simple message...
//
//  context->actor			spell caster
//  context->victim			spell victim
//
//  step->iparms[0]			focus of message
//  step->sparms[0]			text of message
//
// -----------------------------------------------------------------------

bool do_cast_message(MOB_CMD_CONTEXT *context, SEQ_STEP *step) {

    char message[MAX_STRING_LENGTH];

    sprintf(message, "%s/n/r", step->sparms[0]);

    switch (step->iparms[0]) {

      case STEP_FCS_ACTOR:
        send_to_char(message, context->actor);
        break; 
        
      case STEP_FCS_VICTIM:
        if (context->victim != NULL) { 
          send_to_char(message, context->actor);
        }
        break; 

      default:
        bug("Bad message target %d", step->iparms[0]); 
        break;
    }

   /* Set up the activity state engine... */

    if ( context->actor->activity != ACV_CASTING ) {
      return FALSE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------
// Calculate damage from a spell...
//
//  Average human gets 15 hits/level    
//                                     
//                      level   10    40    60    90   150
//                            ----  ----  ----  ----  ----
//                      hits   150   600   900  1350  2250
//                            ----  ----  ----  ----  ----
//  Minimum  : l/4 d4  +  2      7    27    39    57    94    4%/  4%  25
//  Low      : l/3 d6  +  4     14    49    74   109   179    9%/  7%  15
//  Med.Low  : l/2 d8  +  6     28    96   141   208   343   19%/ 15%   7
//  Medium   : l/2 d10 +  8     35   118   173   255   420   23%/ 18%  5/6
//  Med.High : l/2 d12 + 12     44   142   207   304   499   29%/ 22%  4/5
//  High     : l   d10 + 16     71   236   346   511   841   47%/ 37%   3 
//  Fatal    : 2l  d12 + 32    162   552   812  1202  1982  108%/ 88%  1/2
//  Overkill : 3l  d20 + 64    379  1324  1954  2899  4789  252%/212%   1
// -----------------------------------------------------------------------

int calculate_damage(CHAR_DATA *ch, int flags) {

    int damage;
    int level;

    if (ch == NULL) {
      return 1;
    }

    level = casting_level(ch);    

    if        ( IS_SET(flags, STEP_DMG_MIN)      ) {
      damage = dice(1 + level/4, 4) + 2;
    } else if ( IS_SET(flags, STEP_DMG_LOW)      ) {
      damage = dice(1 + level/3, 6) + 4;
    } else if ( IS_SET(flags, STEP_DMG_MED_LOW)  ) {
      damage = dice(1 + level/3, 8) + 6;
    } else if ( IS_SET(flags, STEP_DMG_MED)      ) {
      damage = dice(1 + level/2, 10) + 8;
    } else if ( IS_SET(flags, STEP_DMG_MED_HIGH) ) {
      damage = dice(1 + level/2, 12) + 12;
    } else if ( IS_SET(flags, STEP_DMG_HIGH)     ) {
      damage = dice(1 + level,   10) + 16;
    } else if ( IS_SET(flags, STEP_DMG_FATAL)    ) {
      damage = dice(2 * level,   12) + 32;
    } else if ( IS_SET(flags, STEP_DMG_OVERKILL) ) {
      damage = dice(3 * level,   20) + 64;
    } else {
      damage = 1;
    }

    return damage;
}

// -----------------------------------------------------------------------
// Deal damage...
//
//  context->actor			spell caster
//  context->victim			spell victim
//
//  step->iparms[0]			Damage type ( DAM_         )
//  step->iparms[1]			Block type  ( BLOCK_       )
//  step->iparms[2]			Focus Flags ( STEP_FCS_    )
//  step->iparms[3]			Dmg Flags   ( STEP_DMG_    )
//  step->sparms[0]			Attack description
//
// -----------------------------------------------------------------------

bool do_cast_damage(MOB_CMD_CONTEXT *context, SEQ_STEP *step) {

    int level, spn, sn;

   /* Get spell number and details... */

    spn = context->actor->acv_int; 

    sn = spell_array[spn].sn;

    spell_roll = context->actor->acv_int2;

   /* Obtain the spell CDB */
  
    if (IS_SET(step->iparms[2], STEP_FCS_ACTOR)) {
      spell_CDB = getCDB(context->actor, context->actor, sn, FALSE, sn, 0);
    } else {
      spell_CDB = getCDB(context->actor, context->victim, sn, FALSE, sn, 0);
    }

   /* Set up combat data... */

    spell_CDB->atk_dam_type = step->iparms[0];
    spell_CDB->atk_blk_type = step->iparms[1];

   /* Bail out if it misses... */

    if (IS_SET(step->iparms[3], STEP_DMG_HIT)) { 
      if (!check_spell_hit(spell_CDB, spell_roll)) {
        return TRUE;
      }
    }  
 
   /* Set up and adjust damage... */

    level = casting_level(context->actor);

    spell_CDB->damage = calculate_damage(context->actor, step->iparms[3]);

   /* Set description... */

    spell_CDB->atk_desc = step->sparms[0];
 
   /* Save to reduce damage... */

    if (IS_SET(step->iparms[3], STEP_DMG_SAVE_HALF)) {
      if ( saves_spell( level, spell_CDB->defender ) ) {
  	spell_CDB->damage /= 2;
      } 
    }

    if (IS_SET(step->iparms[3], STEP_DMG_SAVE_NONE)) {
      if ( saves_spell( level, spell_CDB->defender ) ) {
  	return TRUE;
      } 
    }

   /* Roasting time... */

    apply_damage( spell_CDB);

   /* Set up the activity state engine... */

    if ( context->actor->activity != ACV_CASTING ) {
      return FALSE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------
//
// Deal area damage to a specific char...
//
//  context->actor			spell caster
//  context->victim			spell victim
//
//  ch					character being damaged
//
//  step->iparms[0]			Damage type ( DAM_       )
//  step->iparms[1]			Block type  ( BLOCK_     )
//  step->iparms[2]			Fcs Flags   ( STEP_FCS   )
//  step->iparms[3]			Dmg Flags   ( STEP_DMG   )
//  step->iparms[4]			Scope       ( WEV_SCOPE_ )
//  step->sparms[0]			Attack description
//
// -----------------------------------------------------------------------

void do_cast_area_damage_char(	MOB_CMD_CONTEXT	*context, CHAR_DATA	*ch, SEQ_STEP	*step) {
    int tgt, spn, sn, level;

   /* Get spell number and details... */

    spn = context->actor->acv_int; 

    sn = spell_array[spn].sn;

    spell_roll = context->actor->acv_int2;

   /* Check if the character is a valid target... */

    tgt = step->iparms[2];

//    tgt &= (!STEP_FCS_ACTOR);
//    tgt &= (!STEP_FCS_VICTIM);
if (IS_SET(tgt, STEP_FCS_ACTOR)) {
  REMOVE_BIT(tgt, STEP_FCS_ACTOR);
}
   
if (IS_SET(tgt, STEP_FCS_VICTIM)) {
  REMOVE_BIT(tgt, STEP_FCS_VICTIM);
}

    switch (tgt) {

      case STEP_FCS_NOT_FRIEND:
        if (is_friend(ch, context->actor)) {
          return;
        }
        break; 

      case STEP_FCS_NOT_FOE:
        if (is_foe(ch, context->actor)) {
          return;
        }
        break; 

      case STEP_FCS_FRIEND:
        if (!is_friend(ch, context->actor)) {
          return;
        }
        break; 

      case STEP_FCS_FOE:
        if (!is_foe(ch, context->actor)) {
          return;
        }
        break; 

      case STEP_FCS_ALL:
        break; 

      default:
        bug("Bad area damage target %d", tgt);
        return;  
    }

   /* Obtain the spell CDB */
  
    spell_CDB = getCDB(context->actor, ch, sn, FALSE, sn, 0);

   /* Set up combat data... */

    spell_CDB->atk_dam_type = step->iparms[0];
    spell_CDB->atk_blk_type = step->iparms[1];

   /* Bail out if it misses... */

    if (IS_SET(step->iparms[3], STEP_DMG_HIT)) { 
      if (!check_spell_hit(spell_CDB, spell_roll)) {
        return;
      }
    }  
 
   /* Set up and adjust damage... */

    level = casting_level(context->actor);

    spell_CDB->damage = calculate_damage(context->actor, step->iparms[3]);

   /* Set description... */

    spell_CDB->atk_desc = step->sparms[0];
 
   /* Save to reduce damage... */

    if (IS_SET(step->iparms[3], STEP_DMG_SAVE_HALF)) {
      if ( saves_spell( level, spell_CDB->defender ) ) {
        spell_CDB->damage /= 2;
      } 
    }

    if (IS_SET(step->iparms[3], STEP_DMG_SAVE_NONE)) {
      if ( saves_spell( level, spell_CDB->defender ) ) {
        return;
      } 
    }

   /* Roasting time... */

    apply_damage( spell_CDB);

   /* All done... */

    return;
}


// -----------------------------------------------------------------------
// Deal area damage...
//
//  context->actor			spell caster
//  context->victim			spell victim
//
//  step->iparms[0]			Damage type ( DAM_       )
//  step->iparms[1]			Block type  ( BLOCK_     )
//  step->iparms[2]			Fcs Flags   ( STEP_FCS   )
//  step->iparms[3]			Dmg Flags   ( STEP_DMG   )
//  step->iparms[4]			Scope       ( WEV_SCOPE_ )
//  step->sparms[0]			Attack description
//
// -----------------------------------------------------------------------

bool do_cast_area_damage(MOB_CMD_CONTEXT *context, SEQ_STEP *step) {

    ROOM_INDEX_DATA *center;
    ROOM_INDEX_DATA *collection;
    ROOM_INDEX_DATA *next_room;

    CHAR_DATA *ch, *next_ch;

   /* Find the starting point of the spell... */

    center = NULL;

    if ( IS_SET(step->iparms[2], STEP_FCS_ACTOR)
      && context->actor != NULL ) {
      center = context->actor->in_room;
    }
    
    if ( IS_SET(step->iparms[2], STEP_FCS_ACTOR)
      && context->victim != NULL ) {
      center = context->victim->in_room;
    }

    if (center == NULL) {
      return FALSE;
    }
      
   /* Now find the target rooms... */

    collection = collect_rooms(center, step->iparms[4], context->actor);

    if (collection == NULL) {
      return FALSE;
    }

   /* Iterate through the collection... */

    while (collection != NULL) {

     /* Check each person in the room... */

      ch = collection->people;

      while (ch != NULL) {

       /* Remember next char incase ch gets killed... */ 

        next_ch = ch->next_in_room;

       /* Run damage assessment for the char... */ 

        do_cast_area_damage_char(context, ch, step);

       /* Onto the next character... */ 

        ch = next_ch;
      } 
 
     /* Remove the room from the collection and get the next one... */

      next_room = collection->next_collected;
      collection->next_collected = NULL;

      collection = next_room;
    }

   /* Set up the activity state engine... */

    if ( context->actor->activity != ACV_CASTING ) {
      return FALSE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------
// Heal damage..
//
//  context->actor			spell caster
//  context->victim			spell victim
//
//  step->iparms[0]			Dmg  Flags  ( STEP_DMG     )
//  step->iparms[1]			Heal Flags  ( STEP_HEAL    )
//  step->iparms[2]			Fcs Flags   ( STEP_FCS   )
//  step->sparms[0]			Healing description
//
// -----------------------------------------------------------------------

bool do_cast_heal(MOB_CMD_CONTEXT *context, SEQ_STEP *step) {

    int level, amt;

    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

    CHAR_DATA *victim;

   /* Get spell number and details... */

    spell_roll = context->actor->acv_int2;

   /* Find the recipient... */
  
    if (IS_SET(step->iparms[2], STEP_FCS_ACTOR)) {
      victim = context->actor;
    } else {
      victim = context->victim;
    }

    if ( victim == NULL ) {
      return FALSE;
    }

   /* Set up and adjust healing... */

    level = casting_level(context->actor);    

    amt = calculate_damage(victim, step->iparms[0]);

   /* Healing time... */

    mcc = get_mcc(context->actor, context->actor, victim, 
                                   NULL, NULL, NULL, amt, step->sparms[0]);

    switch (step->iparms[1]) {

      case STEP_HEAL_MOVE:
        victim->move += amt;
        victim->move = UMIN(victim->move, victim->max_move);
 
        if ( context->actor == victim ) { 
          wev = get_wev(WEV_HEAL, WEV_HEAL_MOVE, mcc,
                       "{CYour @t0 revitalises you! [@n0]{x\r\n",
                        NULL,
                       "@a2s @t0 revitalizes @a4self.\r\n");
        } else {
          wev = get_wev(WEV_HEAL, WEV_HEAL_MOVE, mcc,
                       "{CYour @t0 revitalises @v2! [@n0]{x\r\n",
                       "{C@a2s @t0 revitalises you! [@n0]{x\r\n",
                       "@a2s @t0 revitalizes @v2!\r\n");
        }

        break;

      case STEP_HEAL_MANA:
        victim->mana += amt;
        victim->mana = UMIN(victim->mana, victim->max_mana);
 
        if ( context->actor == victim ) { 
          wev = get_wev(WEV_HEAL, WEV_HEAL_MANA, mcc,
                       "{CYour @t0 reenergizes you! [@n0]{x\r\n",
                        NULL,
                       "@a2s @t0 reenergizes @a4self.\r\n");
        } else {
          wev = get_wev(WEV_HEAL, WEV_HEAL_MANA, mcc,
                       "{CYour @t0 reenergizes @v2! [@n0]{x\r\n",
                       "{C@a2s @t0 reenergizes you! [@n0]{x\r\n",
                       "@a2s @t0 reenergizes @v2!\r\n");
        }

        break;

      default:
      case STEP_HEAL_HITS:
        victim->hit += amt;
        victim->hit = UMIN(victim->hit, victim->max_hit);
 
        if ( context->actor == victim ) { 
          wev = get_wev(WEV_HEAL, WEV_HEAL_HITS, mcc,
                       "{cYour @t0 heals you! [@n0]{x\r\n",
                        NULL,
                       "@a2s @t0 heals @a4self.\r\n");
        } else {
          wev = get_wev(WEV_HEAL, WEV_HEAL_HITS, mcc,
                       "{cYour @t0 heals @v2! [@n0]{x\r\n",
                       "{c@a2s @t0 heals you! [@n0]{x\r\n",
                       "@a2s @t0 heals @v2!\r\n");
        }

        break;
    }

    room_issue_wev(victim->in_room, wev);

    free_wev(wev);

   /* Set up the activity state engine... */

    if ( context->actor->activity != ACV_CASTING ) {
      return FALSE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------
//
// Deal area healing to a specific char...
//
//  context->actor			spell caster
//  context->victim			spell victim
//
//  ch					character being damaged
//
//  step->iparms[0]			Dmg  Flags  ( STEP_DMG     )
//  step->iparms[1]			Heal Flags  ( STEP_HEAL    )
//  step->iparms[2]			Fcs Flags   ( STEP_FCS   )
//  step->iparms[3]			Scope       ( WEV_SCOPE_ )
//  step->sparms[0]			Healing description
//
// -----------------------------------------------------------------------

void do_cast_area_heal_char(MOB_CMD_CONTEXT *context, CHAR_DATA	*ch, SEQ_STEP *step) {
    int tgt, spn, sn, level, amt;
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

   /* Get spell number and details... */

    spn = context->actor->acv_int; 

    sn = spell_array[spn].sn;

    spell_roll = context->actor->acv_int2;

   /* Check if the character is a valid target... */

    tgt = step->iparms[2];

    REMOVE_BIT(tgt, STEP_FCS_ACTOR);
    REMOVE_BIT(tgt, STEP_FCS_VICTIM);
   
    switch (tgt) {

      case STEP_FCS_NOT_FRIEND:
        if (is_friend(ch, context->actor)) {
          return;
        }
        break; 

      case STEP_FCS_NOT_FOE:
        if (is_foe(ch, context->actor)) {
          return;
        }
        break; 

      case STEP_FCS_FRIEND:
        if (!is_friend(ch, context->actor)) {
          return;
        }
        break; 

      case STEP_FCS_FOE:
        if (!is_foe(ch, context->actor)) {
          return;
        }
        break; 

      case STEP_FCS_ALL:
        break; 

      default:
        bug("Bad area healing target %d", tgt);
        return;  
    }

   /* Get spell number and details... */

    spell_roll = context->actor->acv_int2;

   /* Set up and adjust healing... */

    level = casting_level(context->actor);    

    amt = calculate_damage(ch, step->iparms[0]);

   /* Healing time... */

    mcc = get_mcc(context->actor, context->actor, ch, NULL, NULL, NULL, amt, step->sparms[0]);

    switch (step->iparms[1]) {

      case STEP_HEAL_MOVE:
        ch->move += amt;
        ch->move = UMIN(ch->move, ch->max_move);
 
        if ( context->actor == ch ) { 
          wev = get_wev(WEV_HEAL, WEV_HEAL_MOVE, mcc,
                       "{CYour @t0 revitalises you! [@n0]{x\r\n",
                        NULL,
                       "@a2s @t0 revitalizes @a4self.\r\n");
        } else {
          wev = get_wev(WEV_HEAL, WEV_HEAL_MOVE, mcc,
                       "{CYour @t0 revitalises @v2! [@n0]{x\r\n",
                       "{C@a2s @t0 revitalises you! [@n0]{x\r\n",
                       "@a2s @t0 revitalizes @v2!\r\n");
        }

        break;

      case STEP_HEAL_MANA:
        ch->mana += amt;
        ch->mana = UMIN(ch->mana, ch->max_mana);
 
        if ( context->actor == ch ) { 
          wev = get_wev(WEV_HEAL, WEV_HEAL_MANA, mcc,
                       "{CYour @t0 reenergizes you! [@n0]{x\r\n",
                        NULL,
                       "@a2s @t0 reenergizes @a4self.\r\n");
        } else {
          wev = get_wev(WEV_HEAL, WEV_HEAL_MANA, mcc,
                       "{CYour @t0 reenergizes @v2! [@n0]{x\r\n",
                       "{C@a2s @t0 reenergizes you! [@n0]{x\r\n",
                       "@a2s @t0 reenergizes @v2!\r\n");
        }

        break;

      case STEP_HEAL_HITS:
      default:
        ch->hit += amt;
        ch->hit = UMIN(ch->hit, ch->max_hit);
 
        if ( context->actor == ch ) { 
          wev = get_wev(WEV_HEAL, WEV_HEAL_HITS, mcc,
                       "{cYour @t0 heals you! [@n0]{x\r\n",
                        NULL,
                       "@a2s @t0 heals @a4self.\r\n");
        } else {
          wev = get_wev(WEV_HEAL, WEV_HEAL_HITS, mcc,
                       "{cYour @t0 heals @v2! [@n0]{x\r\n",
                       "{c@a2s @t0 heals you! [@n0]{x\r\n",
                       "@a2s @t0 heals @v2!\r\n");
        }

        break;
    }

    room_issue_wev(ch->in_room, wev);

    free_wev(wev);

   /* All done... */

    return;
}


// -----------------------------------------------------------------------
// Deal area healing...
//
//  context->actor			spell caster
//  context->victim			spell victim
//
//  step->iparms[0]			Dmg  Flags  ( STEP_DMG     )
//  step->iparms[1]			Heal Flags  ( STEP_HEAL    )
//  step->iparms[2]			Fcs Flags   ( STEP_FCS   )
//  step->iparms[3]			Scope       ( WEV_SCOPE_ )
//  step->sparms[0]			Healing description
//
// -----------------------------------------------------------------------

bool do_cast_area_heal(MOB_CMD_CONTEXT *context, SEQ_STEP *step) {

    ROOM_INDEX_DATA *center;
    ROOM_INDEX_DATA *collection;
    ROOM_INDEX_DATA *next_room;

    CHAR_DATA *ch, *next_ch;

   /* Find the starting point of the spell... */

    center = NULL;

    if ( IS_SET(step->iparms[2], STEP_FCS_ACTOR)
      && context->actor != NULL ) {
      center = context->actor->in_room;
    }
    
    if ( IS_SET(step->iparms[2], STEP_FCS_ACTOR)
      && context->victim != NULL ) {
      center = context->victim->in_room;
    }

    if (center == NULL) {
      return FALSE;
    }
      
   /* Now find the target rooms... */

    collection = collect_rooms(center, step->iparms[3], context->actor);

    if (collection == NULL) {
      return FALSE;
    }

   /* Iterate through the collection... */

    while (collection != NULL) {

     /* Check each person in the room... */

      ch = collection->people;

      while (ch != NULL) {

       /* Remember next char incase ch gets killed... */ 

        next_ch = ch->next_in_room;

       /* Run damage assessment for the char... */ 

        do_cast_area_heal_char(context, ch, step);

       /* Onto the next character... */ 

        ch = next_ch;
      } 
 
     /* Remove the room from the collection and get the next one... */

      next_room = collection->next_collected;
      collection->next_collected = NULL;

      collection = next_room;
    }

   /* Set up the activity state engine... */

    if ( context->actor->activity != ACV_CASTING ) {
      return FALSE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------
// Lose sanity...
//
//  context->actor			spell caster
//  context->victim			spell victim
//
//  step->iparms[0]			Sanity threshold for caster
//  step->iparms[1]			Sanity threshold for victim
//  step->iparms[2]			Sanity threshold for observer
//
// -----------------------------------------------------------------------

bool do_cast_sanity(MOB_CMD_CONTEXT *context, SEQ_STEP *step) {

    CHAR_DATA *ch, *next_ch;

   /* Caster first... */

    if ( context->actor != NULL
      && step->iparms[0] != 0) {
      check_sanity(context->actor, step->iparms[0]);
    }

   /* Victim next... */

    if ( context->victim != NULL
      && step->iparms[1] != 0) {
      check_sanity(context->victim, step->iparms[1]);
    }

   /* Observers next... */

    if (step->iparms[2] != 0) {
      
     /* Casters observers... */

      if (context->actor != NULL) {
        ch = context->actor->in_room->people;

        while (ch != NULL) {

          next_ch = ch->next;

          if ( ch != context->actor
            && ch != context->victim ) {
            check_sanity(ch, step->iparms[2]);
          } 

          ch = next_ch;
        }
      }
      
     /* Victims observers... */

      if ( context->victim != NULL
        && context->actor != NULL
        && context->actor->in_room != context->victim->in_room) {

        ch = context->victim->in_room->people;

        while (ch != NULL) {

          next_ch = ch->next;

          if ( ch != context->actor
            && ch != context->victim ) {
            check_sanity(ch, step->iparms[2]);
          } 

          ch = next_ch;
        }
      }
    }  

   /* Set up the activity state engine... */

    if ( context->actor->activity != ACV_CASTING ) {
      return FALSE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------
// Recover from spell casting...
// -----------------------------------------------------------------------

bool do_cast_recover(CHAR_DATA *ch) {

    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

    int spn; 

   /* Get spell number... */

    spn = ch->acv_int; 

   /* Tell the world we're recovered... */

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 
                                           spn, spell_array[spn].name);

    wev = get_wev(WEV_SPELL, WEV_SPELL_RECOVER, mcc,
                 "You recover from casting '@t0'.\r\n",
                  NULL,
                 "@a2 recovers from casting @a3 spell.\r\n");

    room_issue_wev( ch->in_room, wev);

    free_wev(wev);

   /* Set up the activity state engine... */

    return TRUE;
}



// -----------------------------------------------------------------------
// Execute a single step of the spell casting process...
// -----------------------------------------------------------------------

bool do_cast_step(MOB_CMD_CONTEXT *mcc, SEQ_STEP *step) {

    bool casting_ok;

    casting_ok = FALSE;

   /* Branch to the appropriate routine... */
 
    switch (step->instr) {

     /* Begin the casting, validate components... */

      case STEP_PREPARE_SPELL:

        casting_ok = do_cast_prepare(mcc, step);

        break;

     /* Mystial chanting and hand waving... */

      case STEP_CHANT:

        casting_ok = do_cast_chant(mcc, step);
        break;

      case STEP_YELL:

        casting_ok = do_cast_yell(mcc, step);
        break;

     /* Echos and other effects... */

      case STEP_ECHO:

        casting_ok = do_cast_echo(mcc, step);

        break;

     /* Messages to caster and victim... */

      case STEP_MESSAGE:

        casting_ok = do_cast_message(mcc, step);

        break;

     /* Make the first casting roll and consume the components... */ 

      case STEP_CAST_SPELL:

        casting_ok = do_cast_consume(mcc, step);

        break;

     /* Cause some damage... */ 

      case STEP_DAMAGE:

        casting_ok = do_cast_damage(mcc, step);

        break;

     /* Heal some damage... */ 

      case STEP_HEAL:

        casting_ok = do_cast_heal(mcc, step);

        break;

     /* Cause a lot of damage... */ 

      case STEP_AREA_DAMAGE:

        casting_ok = do_cast_area_damage(mcc, step);

        break;

     /* Heal a lot of damage... */ 

      case STEP_AREA_HEAL:

        casting_ok = do_cast_area_heal(mcc, step);

        break;

     /* Magical words of power! */

      case STEP_POWER_WORD:

        casting_ok = do_cast_power_word(mcc, step);

        break;

     /* Sanity for all! */

      case STEP_SANITY:

        casting_ok = do_cast_sanity(mcc, step);

        break;

     /* Make the first casting roll and consume the components... */ 

      case STEP_EFFECT:

        casting_ok = do_cast_effect(mcc, step);

        break;

     /* Anything else is a problem... */

      default:
        bug("Unknown sequence instruction %d", step->instr);
        casting_ok = FALSE; 
    }

   /* Ok... */

    return casting_ok;
}


// -----------------------------------------------------------------------
// Locate the target of a spell...
// -----------------------------------------------------------------------

bool find_spell_target(MOB_CMD_CONTEXT *mcc) {

    CHAR_DATA *victim;
    OBJ_DATA *obj;

    int sn; 
    int spn;	

   /* Safety... */

    if ( mcc == NULL
      || mcc->actor == NULL ) {
      bug("Find_spell_target called with bad context!", 0);
      return FALSE;
    }

   /* Get spell number... */

    spn = mcc->actor->acv_int; 

    sn = spell_array[spn].sn;

    victim		= NULL;
    obj			= NULL;
      
    switch ( spell_array[spn].target ) {
     
      default:
        bug( "Do_cast: bad target for spn %d.", spn );
        return FALSE;

      case TAR_IGNORE:
        break;

      case TAR_CHAR_ANYWHERE:
        victim = get_char_world(mcc->actor, mcc->actor->acv_text );

        if ( victim == NULL ) {
          send_to_char( "{YYour victim has departed!{x\r\n", mcc->actor );
          return FALSE;
        }

         break;

      case TAR_CHAR_OFFENSIVE:
      case TAR_CHAR_DEFENSIVE:
        victim = get_char_room( mcc->actor, mcc->actor->acv_text );

        if ( victim == NULL ) {
          send_to_char( "{YYour victim has departed!{x\r\n", mcc->actor );
          return FALSE;
        }

        break;

      case TAR_CHAR_SELF:

        victim = mcc->actor;

        break;

      case TAR_OBJ_INV:

        obj = get_obj_carry( mcc->actor, mcc->actor->acv_text );

        if ( obj == NULL ) {
          send_to_char( "You no longer have the object!\r\n", mcc->actor );
	  return FALSE;
	}

	break;

      case TAR_OBJ_ROOM:

        obj = get_obj_here( mcc->actor, mcc->actor->acv_text );

        if ( obj == NULL ) {
          send_to_char( "You no longer find the object!\r\n", mcc->actor );
	  return FALSE;
	}

	break;
    }

   /* Store victim and object into the command context... */
 
    mcc->victim	= victim;
    mcc->obj	= obj;

    mcc->number	= spn;
     
   /* All done... */

    return TRUE;
}


// -----------------------------------------------------------------------
// Main driver for spell casting...
// -----------------------------------------------------------------------

void do_cast( CHAR_DATA *ch, char *argument ) {
    bool casting_ok, done, nzd, found;
    int cast_pos;
    SEQ_STEP *step;
    int offset, spn;
    MOB_CMD_CONTEXT *mcc;
    int count;

    if (ch->activity == ACV_DUEL) return;
    
   /* Retrieve key from command... */

    if (!check_activity_key(ch, argument)) return;
   
   /* If already casting and not a recall, complain... */

    if ( ch->activity == ACV_CASTING
      && argument[0] != '*' ) {
      send_to_char("You are already casting a spell!\r\n", ch);
      return;
    }

   /* Not casting means a new spell... */
 
    if ( ch->activity != ACV_CASTING ) ch->acv_state = 0;

   /* Switch to appropriate routine... */

    cast_pos            = ch->position;
    casting_ok	= TRUE;
    done	                = FALSE;
    nzd		= FALSE;
    step                    = NULL;
    offset                  = 0;

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL);

   /* State 0 means we haven't begin the spell yet... */
 
    if ( ch->acv_state == 0 ) { 
  
      casting_ok = do_cast_begin(ch, argument);
      if (modlev != 0) ch->acv_int3 = modlev;
      else ch->acv_int3 = 0;
      spn = ch->acv_int;
     
      step = spell_array[spn].seq;

      if (step == NULL) {
        bug("Spell %d has no casting sequence!", spn);
      }

      offset = 1;

    } else {

     /* Anything else means we're in the casting sequence... */

     /* Fast forward to the correct offset... */

      offset = 1;

      spn = ch->acv_int;

      step = spell_array[spn].seq;

      while ( offset < ch->acv_state
      && step != NULL ) {
        step = step->next; 
        offset += 1; 
      }

     /* Abort if we ran out... */
       
      if ( step == NULL ) {

        do_cast_recover(ch); 

        done = TRUE;

        offset = ACV_STATE_MAX;

      } else {
 
       /* Run the sequence... */

        if (! find_spell_target(mcc)) {

          do_cast_recover(ch); 

          offset = ACV_STATE_MAX;

          step = NULL;

        } else {
 
          if (ec_satisfied_mcc(mcc, step->cond, TRUE)) {

            switch (step->instr) {

              case STEP_ABORT:
                casting_ok = FALSE;
                break;

              case STEP_SKIP:
                count = step->iparms[0];
                while ( count > 0 
                     && step != NULL ) { 
                  offset += 1;
                  count -= 1;
                  step = step->next;
                } 
                break;

              default:
                casting_ok = do_cast_step(mcc, step); 
                break;
            }
          } else {
            casting_ok = TRUE;
          } 
 
          step = step->next;

          offset += 1;

          if ( step == NULL
            || step->delay != 0 ) {
            nzd = TRUE;
          } else {
            nzd = FALSE;
          }   
      
          while ( casting_ok
               && step != NULL
               && !nzd ) {

           /* Bail out if we cannot find the spell target... */

            found = find_spell_target(mcc);

            if (!found) {
              casting_ok = FALSE;
              offset = ACV_STATE_MAX;
              step = NULL;
            } else {

             /* If the econd isn't satisfied, skip the step... */ 

              if (!ec_satisfied_mcc(mcc, step->cond, TRUE)) {
                casting_ok = TRUE;
                offset += 1;
                step = step->next;
              } else {

               /* If the delay is zero, we need to stop... */ 

                if (step->delay != 0) {
                  nzd = TRUE;
                } else {

                 /* Now we need to run the step... */

                  switch (step->instr) {

                    case STEP_ABORT:
                      casting_ok = FALSE;
                      break;

                    case STEP_SKIP:
                      count = step->iparms[0];
                      while ( count > 0 
                           && step != NULL ) { 
                        offset += 1;
                        count -= 1;
                        step = step->next;
                      } 
                      break;

                    default:
                      casting_ok = do_cast_step(mcc, step); 
                      break;
                  }

                 /* ...and then find the next one... */
 
                  offset += 1;
                  step = step->next;

                } 
              }
            } 
          }
        }
      }
    }

   /* No step found means... */

    if ( step == NULL ) {

     /* ...we've finished the spell and are post recovery... */

      if (done) {

       /* Reset the activity state engine... */

        cast_pos = ch->position;

        if ( cast_pos == POS_FIGHTING 
          && ( ch->fighting == NULL 
            || ch->fighting->in_room != ch->in_room) ) { 
          cast_pos = POS_STANDING;
        }

        ch->acv_state = 0;                              

        set_activity(ch, cast_pos, NULL, ACV_NONE, NULL);

      } else {

       /* Or we've reached the end and need to recover... */

        ch->acv_state = offset;

        schedule_activity(ch, 1, "cast");

      }
    } else { 

     /* Abort if something went wrong... */ 

      if ( !casting_ok ) {

       /* If still in control, can try a soft exit... */

        if ( ch->activity == ACV_CASTING ) {

          ch->acv_state = ACV_STATE_MAX;

          schedule_activity( ch, 1, "cast" );
      
        } else {

         /* ...otherwise it's a hard exit... */

          cast_pos = ch->position;

          if ( ch->fighting == NULL 
            && cast_pos == POS_FIGHTING ) {
            cast_pos = POS_STANDING;
          }

          set_activity(ch, cast_pos, NULL, ACV_NONE, NULL);
        }
      } else {
 
       /* All ok, schedule next step... */

        ch->acv_state = offset;

        schedule_activity( ch, step->delay, "cast" );
      }
    }

   /* All done, we might be back... */

    return;
}

// -----------------------------------------------------------------------
// Cast spells from objects...
// -----------------------------------------------------------------------

void obj_cast_spell( int efn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, char *info ) {
    void *vo;

   /* efn 0 is always (reserved) */

    if (efn <= 0) return;
    
   /* Other values should be valid... */

    if (!valid_effect(efn)) {
	bug( "Obj_cast_spell: bad efn %d.", efn );
	return;
    }

   /* Now sort the target out... */

    switch ( effect_array[efn].target ) {
      default:
	bug( "Obj_cast_spell: bad target for effect %d.", efn );
	return;

      case TAR_IGNORE:
	if (!victim) victim = ch;
	vo = (void *) victim;
	break;

      case TAR_CHAR_OFFENSIVE:

                if (ch==victim) {
	    send_to_char( "You shiver for a bit.\r\n", ch );
	    return;
	}

	if ( victim == NULL ) {
	    victim = ch->fighting;
                }

	if ( victim == NULL ) {
	    send_to_char( "You can't do that.\r\n", ch );
	    return;
	}

	if (is_safe_spell(ch,victim,FALSE) && ch != victim) {
	    send_to_char("Something isn't right...\r\n",ch);
	    return;
	}

	vo = (void *) victim;

                react_to_cast(victim, ch);

	break;

      case TAR_CHAR_ANYWHERE:
      case TAR_CHAR_DEFENSIVE:
	if ( victim == NULL ) {
	    victim = ch;
                }
	vo = (void *) victim;
	break;

      case TAR_CHAR_SELF:
	if ( victim == NULL ) {
	    victim = ch;
               }
	vo = (void *) ch;
	break;

      case TAR_OBJ_ROOM:
      case TAR_OBJ_INV:
	if ( obj == NULL ) {
	    send_to_char( "You can't do that.\r\n", ch );
	    return;
	}
	vo = (void *) obj;
	break;
    }

    if (info == NULL) {
         target_name = "";
    } else {
         target_name = str_dup(info);
    }

   /* Magic doesn't work in some area... */

    if ( ch->in_room != NULL
      && ch->in_room->area != NULL
      && ( IS_SET(ch->in_room->area->area_flags, AREA_NOMAGIC)
        || IS_SET(ch->in_room->room_flags, ROOM_NOMAGIC)
        || IS_SET(zones[ch->in_room->area->zone].flags, ZONE_NOMAGIC) )) { 
      send_to_char("Hmmm. Nothing seems to happen...\r\n", ch);
      act("$n looks a little puzzeled after trying to use something.", ch, NULL, NULL, TO_ROOM);
    }
 
   /* Adjust spell casting level for area/zone magic capabilities... */

    if (obj != NULL) {
      switch (obj->item_type) {
        case ITEM_WAND: 
        case ITEM_STAFF:
        case ITEM_POTION:
        case ITEM_PILL:
        case ITEM_SCROLL: 
          level = obj->value[0];
          break;

        default:
          level = obj->level;
          break;
      } 
    } else {
      level = ch->level;
    }

    if ( ch->in_room != NULL
      && ch->in_room->area != NULL) {

     /* First room... */

      if (IS_RAFFECTED(ch->in_room, RAFF_LOW_MAGIC)) level -= number_range(1, level/2);
      if (IS_RAFFECTED(ch->in_room, RAFF_HIGH_MAGIC)) level += number_range(1, level);

     /* Then area... */

      if ( IS_SET(ch->in_room->area->area_flags, AREA_LOWMAGIC) ) { 
        level -= number_range(1, level/2);
      } else if ( IS_SET(ch->in_room->area->area_flags, AREA_HIGHMAGIC) ) { 
        level += number_range(1, level);
      } else if ( IS_SET(ch->in_room->area->area_flags, AREA_SUPERMAGIC) || IS_ARTIFACTED(ch, ART_NECRONOMICON)) { 
        level += number_range(1, 2 * level);
      }
 
     /* Then zone... */

      if ( IS_SET(zones[ch->in_room->area->zone].flags, ZONE_LOWMAGIC) ) { 
        level -= number_range(1, level/2);
      } else if ( IS_SET(zones[ch->in_room->area->zone].flags, ZONE_HIGHMAGIC) ) { 
        level += number_range(1, level);
      } else if ( IS_SET(zones[ch->in_room->area->zone].flags, ZONE_SUPERMAGIC) ) { 
        level += number_range(1, 2 * level);
      }

      level = UMAX(level, 1);
    }

   /* To hit roll for offensive wands, staves and srolls... */

    spell_roll = level;

    if ( effect_array[efn].target == TAR_CHAR_OFFENSIVE ) {

      if (obj != NULL) {
        switch (obj->item_type) {

          case ITEM_WAND:
            spell_roll = get_skill(ch, gsn_wands) + number_open(); 
            break;

          case ITEM_STAFF:
            spell_roll = get_skill(ch, gsn_staves) + number_open(); 
            break;

          case ITEM_SCROLL:
            spell_roll = get_skill(ch, gsn_scrolls) + number_open(); 
            break;

          default:
            spell_roll = 80; 
            break;
        }

        spell_roll -= 80;

        if (spell_roll < 0) {
          send_to_char("Ooops....missed!\r\n", ch);
          return;
        }
      }
    }
     
    spell_CDB = getCDB(ch, victim, efn, FALSE, efn, 0);

   /* Apply the spell effect if it wasn't absorbed, dodged or whatever... */

   (*effect_array[efn].spell_fun) ( efn, level, ch, vo );

    return;
}

// -----------------------------------------------------------------------
// Aid for mobs casting spells...
// -----------------------------------------------------------------------

void do_mob_cast(CHAR_DATA *mob, char *spell_name, CHAR_DATA *victim) {

  char buf[MAX_STRING_LENGTH];

 /* Prevent mobs from double casting... */

  if (mob->activity == ACV_CASTING) {
    return;
  }

 /* Mana check... */

  if (mob->mana < 30) {
    return;
  }

 /* Format and send the cast command... */

  sprintf(buf, "'%s' %s", spell_name, victim->name);

  do_cast(mob, buf);

 /* All done... */

  return;
}

// -----------------------------------------------------------------------
// Mobs using 'natural' spell abilities...
// -----------------------------------------------------------------------

void do_dragon_cast( int efn, int level, CHAR_DATA *ch, CHAR_DATA *victim) {
    void *vo;

   /* efn 0 is always (reserved) */

    if (efn <= 0) {
      return;
    }

   /* Other values should be valid... */

    if (!valid_effect(efn)) {
	bug( "do_dragon_cast: bad efn %d.", efn );
	return;
    }

   /* Now sort the target out... */

    switch ( effect_array[efn].target ) {

      default:
	bug( "Obj_cast_spell: bad target for effect %d.", efn );
	return;

      case TAR_IGNORE:
	vo = NULL;
	break;

      case TAR_CHAR_OFFENSIVE:

	if ( victim == NULL ) {
	    victim = ch->fighting;
        }

	if ( victim == NULL ) {
	    send_to_char( "You can't do that.\r\n", ch );
	    return;
	}

	if ( is_safe_spell(ch,victim,FALSE) 
          && ch != victim) {
	    send_to_char("Something isn't right...\r\n",ch);
	    return;
	}

	vo = (void *) victim;

        react_to_cast(victim, ch);

	break;

      case TAR_CHAR_ANYWHERE:
      case TAR_CHAR_DEFENSIVE:

	if ( victim == NULL ) {
	  victim = ch;
        }

	vo = (void *) victim;

	break;

      case TAR_CHAR_SELF:

	vo = (void *) ch;

	break;

      case TAR_OBJ_ROOM:
      case TAR_OBJ_INV:
	send_to_char( "You can't do that.\r\n", ch );
                return; 
    }

    target_name = "";

   /* Magic doesn't work in some area... */

    if ( ch->in_room != NULL
      && ch->in_room->area != NULL
      && ( IS_SET(ch->in_room->area->area_flags, AREA_NOMAGIC)
        || IS_SET(ch->in_room->room_flags, ROOM_NOMAGIC)
        || IS_SET(zones[ch->in_room->area->zone].flags, ZONE_NOMAGIC) )) { 
      send_to_char("Hmmm. Nothing seems to happen...\r\n", ch);
      act("$n looks rather puzzeled...", ch, NULL, NULL, TO_ROOM);
    }

   /* Each usage costs 50 mana... */

    if ( ch->mana < 50 ) {
      send_to_char("You don't have enough mana.\r\n", ch);
      return;
    }

    ch->mana -= 50;

   /* Adjust casting level... */
 
    level = casting_level(ch);

   /* To hit roll for the mob... */

    spell_roll = level;

    if ( effect_array[efn].target == TAR_CHAR_OFFENSIVE ) {

      spell_roll = get_skill(ch, gsn_natural_magic) + number_open(); 

      spell_roll -= 50;  // Dragons know what they are doing...

      if (spell_roll < 0) {
        send_to_char("Ooops....missed!\r\n", ch);
        return;
      }
    }

    spell_CDB = getCDB(ch, victim, efn, FALSE, efn, 0);

   /* Apply the spell effect... */

   (*effect_array[efn].spell_fun) ( efn, level, ch, vo );

    return;
}

/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell( int level, CHAR_DATA *victim ) {
int save;

    save = get_curr_stat(victim, STAT_LUC) + ( victim->level - level - victim->saving_throw ) * 5;

    if (IS_AFFECTED(victim, AFF_BERSERK)) save += victim->level/2;
    if (IS_AFFECTED(victim, AFF_RELAXED)) save += victim->level/2;
    return (save + number_open() > 100);
}


/* RT save for dispels */

bool saves_dispel( int dis_level, int aff_level, int duration) {
int save;

    save = 50 + dis_level - aff_level;

   /* Permenent enchantments are (much) harder to shift... */   
 
    if (duration == -1) {
      save /= 2;  
    }

   /* Now make the save... */

    if (number_open() < (100 - save)) {
      return TRUE;
    }

    return FALSE;
}


/* co-routine for dispel magic and cancellation */

bool check_dispel( int dis_level, CHAR_DATA *victim, int afn) {
    AFFECT_DATA *af;

    if (! valid_affect(afn)) {
      bug("Dispel for invalid affect %d", afn);
      send_to_char("DISPEL BUG - Invalid affect number!\r\n", victim);
      return FALSE;
    }

    if (is_affected(victim, afn)) {
    
      for ( af = victim->affected; af != NULL; af = af->next ) {
       
        if ( af->afn == afn ) {
            
          send_to_char("Your flesh tingles.\r\n", victim);

          if (!saves_dispel(dis_level,af->level,af->duration)) {
                
            affect_strip_afn(victim, afn);
        	    
            send_to_char( affect_array[afn].end_msg, victim );
            send_to_char( "\r\n", victim );
        	    
	    return TRUE;
	  } else {
	    af->level -= 1;
          }
        }
      }
    }
 
    return FALSE;
}


int wild_magic( CHAR_DATA *ch, int efn, int spn) {
int count;
int altfn;

    if (spn <= 0) return efn;
    if (spell_array[spn].info || spell_array[spn].parse_info) return efn;

    for (count = 0 ; count < 50 ; count++ ) {
        altfn = number_range(1,700);
        if (effect_array[efn].target == effect_array[altfn].target
        && valid_spell(altfn))  break;
    }

    if (count < 50 && effect_array[altfn].name) efn=altfn;

    return efn;
}


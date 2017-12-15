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
#include "tracks.h"
#include "race.h"
#include "olc.h"
#include "gsn.h"

/* command procedures needed */
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_flee);
DECLARE_DO_FUN(do_stand);

extern void 	random_dream	(CHAR_DATA *ch);

void 	kill_msg			(CHAR_DATA *ch);
void 	raw_kill			(CHAR_DATA *victim, bool corpse ) ;
void 	heat_obj			(CHAR_DATA *ch, OBJ_DATA *obj, int level);
void 	identify_affect		(CHAR_DATA *ch, AFFECT_DATA *paf);

bool 	saves_dispel		(int dis_level, int aff_level, int duration);


extern int modlev;
extern char *target_name;
extern int spell_roll;
extern COMBAT_DATA *spell_CDB;
extern bool in_seq;


/*
 * Spell functions.
 */

void spell_absorb_magic( int sn, int level, CHAR_DATA *ch, void *vo ) {
    AFFECT_DATA af;

    if ( IS_AFFECTED(ch, AFF_ABSORB) ) {
        send_to_char("You are already able to absorb magic.\r\n",ch);
	return;
    }

    af.type      = sn;
    af.afn       = gafn_absorb_magic; 
    af.level     = level;
    af.duration  = number_fuzzy( level / 6 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_ABSORB;
    affect_to_char( ch, &af );

    act( "$n is surrounded by a strange mystical aura.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by a mystical absorbing aura.\r\n", ch  );
    return;
}

void spell_acid_blast( int sn, int level, CHAR_DATA *ch, void *vo ) {

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_ACID;
    spell_CDB->atk_blk_type = BLOCK_BOLT;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
   /* Set up and adjust damage... */

    spell_CDB->damage = dice(level/2, 10) + 6;

    if ( saves_spell( level, spell_CDB->defender ) ) {
     	spell_CDB->damage /= 2;
    }

   /* Apply the damage... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

   /* All done... */

    return;
}

void spell_entropy (int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_ENERGY;
    spell_CDB->atk_blk_type = BLOCK_BOLT;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
   /* Describe what happens... */

    act("A blast of chaotic magic surronds $N.",
        ch,NULL,victim,TO_ROOM);
    act("You release a blast of entropy to $N.",
        ch,NULL,victim,TO_CHAR);
    act("A blast of chaotic energy screaches from $n's hand and surrounds you!",
        ch,NULL,victim,TO_VICT);

   /* See what the impact is... */

    if ( !saves_spell ( level, victim ) ) {
	check_damage_obj (victim, NULL, 100);	/* gotcher equipment sucker! */
    } else {
	check_damage_obj(ch, NULL, 15);
    }

    return;
}

void spell_animate( int sn, int level, CHAR_DATA *ch, void *vo) {
    OBJ_DATA *obj;
    CHAR_DATA *mob;
    bool angry;
    int zlevel;

    obj = get_obj_here(ch, target_name );

    if ( obj == NULL ) {
        send_to_char( "Resurrect what?\r\n", ch );
        return;
    }

    /* Nothing but NPC corpses. */

    if( obj->item_type != ITEM_CORPSE_NPC ) {
        if( obj->item_type == ITEM_CORPSE_PC )
            send_to_char( "You can't resurrect players.\r\n", ch );
        else
            send_to_char( "It would serve no purpose...\r\n", ch );
        return;
    }

    angry = FALSE;

    zlevel = UMAX(obj->level - 4, 1);

    if( zlevel > (level) ) {
        angry = TRUE;
        zlevel += dice(1,6);
    }

    if( !angry 
     && ch->pet != NULL ) {
        send_to_char( "You already have a pet.\r\n", ch );
        return;
    }

    /* Chew on the zombie a little bit, recalculate level-dependant stats */

    mob = create_mobile_level(get_mob_index( MOB_VNUM_ZOMBIE ), "mob monster", zlevel );

    /* You rang? */

    char_to_room( mob, ch->in_room );

    act( "$p springs to life as a hideous zombie!", ch, obj, NULL, TO_ROOM );
    act( "$p springs to life as a hideous zombie!", ch, obj, NULL, TO_CHAR );

    extract_obj(obj);

    /* Yessssss, massssssster... */

    if (!angry) {
      SET_BIT(mob->affected_by, AFF_CHARM);
      SET_BIT(mob->act, ACT_FOLLOWER);

      add_follower( mob, ch );
      mob->leader = ch;
      ch->pet = mob;

     /* For a little flavor... */

      do_say( mob, "How may I serve you, master?" );

    } else {
      SET_BIT(mob->act, ACT_AGGRESSIVE);
      SET_BIT(mob->affected_by, AFF_HASTE);

      do_emote(mob, "Screams with rage and attacks!");

      multi_hit(mob, ch, TYPE_UNDEFINED);
    } 

    return;
}


void spell_mummify( int sn, int level, CHAR_DATA *ch, void *vo) {
OBJ_DATA *obj;
CHAR_DATA *mob;
bool angry;
int zlevel;

    obj = get_obj_here( ch, target_name );

    if ( obj == NULL ) {
        send_to_char( "Mummify what?\r\n", ch );
        return;
    }

    /* Nothing but NPC corpses. */

    if( obj->item_type != ITEM_CORPSE_NPC ) {
            send_to_char( "It would serve no purpose...\r\n", ch );
        return;
    }

    angry = FALSE;
    zlevel = UMAX(obj->level - 2, 1);

    if( zlevel > (level) ) {
        angry = TRUE;
        zlevel += dice(1,6);
    } else {
        if (number_percent() < 20)  {
            angry = TRUE;
            zlevel += dice(2,6);
        }

    }
    if( !angry 
     && ch->pet != NULL ) {
        send_to_char( "You already have a pet.\r\n", ch );
        return;
    }

    /* Chew on the zombie a little bit, recalculate level-dependant stats */

    mob = create_mobile_level( get_mob_index(MOB_VNUM_MUMMY), "mob monster", zlevel );
    char_to_room( mob, ch->in_room );

    act( "$p slowly comes to life as a monsterous mummy!", ch, obj, NULL, TO_ROOM );
    act( "$p slowly comes to life as a monsterous mummy!", ch, obj, NULL, TO_CHAR );

    extract_obj(obj);

    if (!angry) {
      SET_BIT(mob->affected_by, AFF_CHARM);
      SET_BIT(mob->act, ACT_FOLLOWER);

      add_follower( mob, ch );
      mob->leader = ch;
      ch->pet = mob;

     /* For a little flavor... */

      do_say( mob, "How may I serve you, master?" );

    } else {
      SET_BIT(mob->act, ACT_AGGRESSIVE);
      SET_BIT(mob->affected_by, AFF_HASTE);

      do_emote(mob, "Screams in rage and attacks!");

      multi_hit(mob, ch, TYPE_UNDEFINED);
    } 

    return;
}

void spell_summon_familier( int sn, int level, CHAR_DATA *ch, void *vo) {
CHAR_DATA *mob;

    int mob_level, mob_num;

   /* No previous pet... */

    if( ch->pet != NULL ) {
        send_to_char( "You already have a pet.\r\n", ch );
        return;
    }

   /* Create the mob... */

    mob_level = ch->level - 4;

    mob = create_mobile_level( get_mob_index(20020), "mob monster", mob_level );

   /* Work out what sort of familier they get... */

    mob_num = dice(1, 4) + 2;

    if (IS_EVIL(ch)) {
      mob_num -= 2;
    } else if (IS_GOOD(ch)) {
      mob_num += 2;
    } 

   /* 1 in 128 chance of getting a raw one... */
  
    if (number_bits(8) == 0) {
      mob_num = -1;
    }

   /* Taylor description and names... */

    switch (mob_num) {
      
      default:
        break; 

      case 1:
        free_string(mob->name);
        free_string(mob->short_descr);
        free_string(mob->long_descr);
        free_string(mob->description);

        mob->name = strdup("homunlcus disgusting green");
        mob->short_descr = strdup("a disgusting green homunlcus");
        mob->long_descr = strdup("A disgusting green homunlcus\n");
        mob->description = strdup("It looks like the stories about the revolting things these abominations are\n made from are true. Yuk!\n");

        mob->sex = SEX_NEUTRAL;
        mob->race=15;
        mob->alignment = -1000;

        break;

      case 2:
        free_string(mob->name);
        free_string(mob->short_descr);
        free_string(mob->long_descr);
        free_string(mob->description);

        mob->name = strdup("imp short red");
        mob->short_descr = strdup("a short red imp");
        mob->long_descr = strdup("A short, red imp\n");
        mob->description = strdup("He gives you a nasty look!\n");

        mob->sex = SEX_MALE;
        mob->race=20;
        mob->alignment = -750;

        SET_BIT(mob->affected_by, AFF_FLYING);

        break;

      case 3:
        free_string(mob->name);
        free_string(mob->short_descr);
        free_string(mob->long_descr);
        free_string(mob->description);

        mob->name = strdup("python long");
        mob->short_descr = strdup("a long python");
        mob->long_descr = strdup("A long python\n");
        mob->description = strdup("It seems quite friendly.\n");

        mob->sex = SEX_NEUTRAL;
        mob->race=27;
        mob->alignment = -300;

        break;

      case 4:
        free_string(mob->name);
        free_string(mob->short_descr);
        free_string(mob->long_descr);
        free_string(mob->description);

        mob->name = strdup("toad grey");
        mob->short_descr = strdup("a grey toad");
        mob->long_descr = strdup("A grey toad\n");
        mob->description = strdup("He is covered in ugly warts.\n");

        mob->sex = SEX_MALE ;
        mob->race=21;
        mob->alignment = -100;

        break;

      case 5:
        free_string(mob->name);
        free_string(mob->short_descr);
        free_string(mob->long_descr);
        free_string(mob->description);

        mob->name = strdup("parrot green");
        mob->short_descr = strdup("a green parrot");
        mob->long_descr = strdup("A green parrot\n");
        mob->description = strdup("He looks like he belonged to a sailor.\n");

        mob->sex = SEX_MALE;
        mob->race=28;
        mob->alignment = 100;

        SET_BIT(mob->affected_by, AFF_FLYING);

        break;

      case 6:
        free_string(mob->name);
        free_string(mob->short_descr);
        free_string(mob->long_descr);
        free_string(mob->description);

        mob->name = strdup("cat black");
        mob->short_descr = strdup("a black cat");
        mob->long_descr = strdup("A black cat\n");
        mob->description = strdup("She looks quite refined.\n");

        mob->sex = SEX_FEMALE;
        mob->race=12;
        mob->alignment = 300;

        break;

      case 7:
        free_string(mob->name);
        free_string(mob->short_descr);
        free_string(mob->long_descr);
        free_string(mob->description);

        mob->name = strdup("bird blue beautiful");
        mob->short_descr = strdup("a blue bird");
        mob->long_descr = strdup("A beautiful blue bird\n");
        mob->description = strdup("She looks very happy (and somewhat like a chicken)!\n");

        mob->sex = SEX_FEMALE;
        mob->race=28;
        mob->alignment = 750;

        SET_BIT(mob->affected_by, AFF_FLYING);

        break;

      case 8:
        free_string(mob->name);
        free_string(mob->short_descr);
        free_string(mob->long_descr);
        free_string(mob->description);

        mob->name = strdup("cherub cute");
        mob->short_descr = strdup("a cute cherub");
        mob->long_descr = strdup("A cute cherub\n");
        mob->description = strdup("He looks very beautiful, and has little wings on his back.\n");

        mob->sex = SEX_MALE;
        mob->race=2;
        mob->alignment = 1000;

        SET_BIT(mob->affected_by, AFF_FLYING);

        break;

    }  

   /* Deliver the familier */

    char_to_room( mob, ch->in_room );

    act( "$N steps out of the shadows...", ch, NULL, mob, TO_ROOM );
    act( "$N steps out of the shadows...", ch, NULL, mob, TO_CHAR );

   /* Enslave the familier... */

    SET_BIT(mob->affected_by, AFF_CHARM);
    SET_BIT(mob->act, ACT_FOLLOWER);

    add_follower( mob, ch );
    mob->leader = ch;
    ch->pet = mob;

   /* For a little flavor... */

    do_emote( mob, "looks at you expectently" );

    return;
}

void spell_summon_spirit( int sn, int level, CHAR_DATA *ch, void *vo) {
CHAR_DATA *mob;
int mob_level, mob_num;

   /* No previous pet... */

    if( ch->pet != NULL ) {
        send_to_char( "You already have a pet.\r\n", ch );
        return;
    }

   /* Create the mob... */

    mob_level = ch->level - 6;

    mob = create_mobile_level( get_mob_index( 20022 ), "mob mage", mob_level );

   /* Work out what sort of familier they get... */

    mob_num = 2;

    if (IS_EVIL(ch)) {
      mob_num = 1;
    } else if (IS_GOOD(ch)) {
      mob_num = 3;
    } 

    /* Taylor description and names... */

    switch (mob_num) {
      
      default:
        break; 

 case 1:
        free_string(mob->name);
        free_string(mob->short_descr);
        free_string(mob->long_descr);
        free_string(mob->description);

        mob->name = strdup("efreet fiery");
        mob->short_descr = strdup("Efreet");
        mob->long_descr = strdup("An efreet is burning brightly here.\n");
        mob->description = strdup("An efreet is burning brightly here.\n");
       
        mob->sex = SEX_MALE;
        mob->alignment = -500;
        SET_BIT(mob->affected_by, AFF_FLYING);
        SET_BIT(mob->imm_flags, IMM_FIRE);
        SET_BIT(mob->vuln_flags, VULN_COLD);
        SET_BIT(mob->dam_type,WDT_FLAMING_BITE);
  
        break;

      case 2:
        free_string(mob->name);
        free_string(mob->short_descr);
        free_string(mob->long_descr);
        free_string(mob->description);

        mob->name = strdup("genie aetheral");
        mob->short_descr = strdup("Genie");
        mob->long_descr = strdup("An aetheral genie is floating here.\n");
        mob->description = strdup("An aetheral genie is floating here.\n");

        mob->sex = SEX_MALE;
        mob->alignment = 0;
        SET_BIT(mob->affected_by, AFF_FLYING);
        SET_BIT(mob->affected_by, AFF_HASTE);
        SET_BIT(mob->dam_type,WDT_SHOCKING_BITE);

        break;

      case 3:
        free_string(mob->name);
        free_string(mob->short_descr);
        free_string(mob->long_descr);
        free_string(mob->description);

        mob->name = strdup("seraph");
        mob->short_descr = strdup("Seraph");
        mob->long_descr = strdup("A seraph is here, enveloped by a glowing halo.\n");
        mob->description = strdup("A seraph is here, enveloped by a glowing halo.\n");

        mob->sex = SEX_MALE;
        mob->alignment = 500;
        SET_BIT(mob->affected_by, AFF_FLYING);
        SET_BIT(mob->imm_flags, IMM_HOLY);
        SET_BIT(mob->vuln_flags, VULN_NEGATIVE);
        SET_BIT(mob->dam_type,WDT_DIVINE_POWER);

        break;

    }  

   /* Deliver the familier */

    char_to_room(mob, ch->in_room );

    act( "$N steps out of the shadows...", ch, NULL, mob, TO_ROOM );
    act( "$N steps out of the shadows...", ch, NULL, mob, TO_CHAR );

   /* Enslave the familier... */

    SET_BIT(mob->affected_by, AFF_CHARM);
    SET_BIT(mob->act, ACT_FOLLOWER);

    add_follower(mob, ch );
    mob->leader = ch;
    ch->pet = mob;

   /* For a little flavor... */

    do_emote(mob, "looks at you expectently" );

    return;
}


void spell_elder_watcher( int sn, int level, CHAR_DATA *ch, void *vo) {
CHAR_DATA *mob;
int mob_level;

   /* Create the mob... */

    mob_level = ch->level - 10;

    mob = create_mobile_level( get_mob_index( 20021 ), "mob monster", mob_level );

 
   /* Deliver the watcher */

    char_to_room( mob, ch->in_room );

    act( "The Watcher steps out of the shadows...", ch, NULL, NULL, TO_ROOM );
    act( "The Watcher steps out of the shadows...", ch, NULL, NULL, TO_CHAR );

   return;
}


void spell_minor_creation( int sn, int level, CHAR_DATA *ch, void *vo) {
OBJ_DATA *obj;
   
    if (!str_cmp(target_name, "boat"))    {
	obj = create_object (get_obj_index (OBJ_VNUM_BOAT), 0);
	obj->cost=0;
	obj_to_room (obj, ch->in_room);
	act ("$n mutters some words and $p materializes from thin air.",ch,obj,NULL,TO_ROOM);
	send_to_char("A boat materializes before you.\r\n",ch);

    } else if (!str_cmp(target_name, "bag"))   {
	obj = create_object (get_obj_index (OBJ_VNUM_BAG), 0);
	obj->cost=0;
	obj_to_room (obj, ch->in_room);
	act ("$n mutters some words and $p materializes from thin air.",ch,obj,NULL,TO_ROOM);
	send_to_char("A bag materializes before you.\r\n",ch);

    } else if (!str_cmp(target_name, "tool"))   {
	obj = create_object (get_obj_index (26), 0);
	obj->cost=0;
	obj_to_room (obj, ch->in_room);
	act ("$n mutters some words and $p materializes from thin air.",ch,obj,NULL,TO_ROOM);
	send_to_char("A shovel materializes before you.\r\n",ch);

    } else if (!str_cmp(target_name, "camp"))   {
                for (obj = ch->in_room->contents; obj; obj =obj->next_content) {
                    if (obj->item_type == ITEM_DECORATION) break;
                }
                if (obj) {
 	    send_to_char( "You can't do that here.\r\n", ch );
 	    return;
                }

	obj = create_object (get_obj_index (OBJ_CAMP), 0);
                obj->value[0] = number_range(level/2, level);
                obj->value[1] = number_range(level/2, level);
	obj->cost=0;
                free_string(obj->name);
                obj->name = str_dup("sphere light");
                free_string(obj->short_descr);
                obj->short_descr = str_dup("a sphere of {Bblue light{x");
                free_string(obj->description);
                obj->description = str_dup("A sphere of {Bblue light{x floats here.\n");
	obj_to_room (obj, ch->in_room);
	act ("$n mutters some words and $p materializes from thin air.", ch, obj, NULL, TO_ROOM);
	send_to_char("A sphere of light materializes before you.\r\n",ch);

    } else {
               send_to_char("You don't know how to make that kind of object.\r\n",ch);
    }
    return;

}


void spell_greater_creation( int sn, int level, CHAR_DATA *ch, void *vo) {
OBJ_DATA *obj;

    if (!str_cmp(target_name, "amber"))    {
	obj = create_object (get_obj_index (5589), 0);
	obj->cost=0;
	obj_to_room (obj, ch->in_room);
                obj->timer = number_range(20,30);
	act ("$n mutters some words and $p materializes from thin air.",ch,obj,NULL,TO_ROOM);
	send_to_char("Some scrying amber materializes before you.\r\n",ch);

    } else if (!str_cmp(target_name, "chain"))   {
	obj = create_object (get_obj_index (29), 0);
	obj->cost=0;
                obj->value[0]=50+(ch->level/25);
	obj_to_room (obj, ch->in_room);
                obj->timer = number_range(20,30);
	act ("$n mutters some words and $p materializes from thin air.",ch,obj,NULL,TO_ROOM);
	send_to_char("Some magic chains materializes before you.\r\n",ch);

    } else if (!str_cmp(target_name, "anvil"))  {
	obj = create_object (get_obj_index (28), 0);
	obj->cost=0;
	obj_to_room (obj, ch->in_room);
                obj->timer = number_range(10,20);
	act ("$n mutters some words and $p materializes from thin air.",ch,obj,NULL,TO_ROOM);
	send_to_char("A heavy anvil materializes before you.\r\n",ch);

    } else if (!str_cmp(target_name, "grimoire"))  {
                EXTRA_DESCR_DATA *ed;
                char buf[MAX_INPUT_LENGTH];

	obj = create_object (get_obj_index (51), 0);
	obj->cost=0;
	obj_to_room (obj, ch->in_room);
                obj->enchanted = TRUE;
                ed =   new_extra_descr();
                sprintf (buf, "read %s", obj->name);
                ed->keyword		= str_dup(buf);
                ed->description		= str_dup( "Disciplines: " );
                ed->next			= obj->extra_descr;
                obj->extra_descr		= ed;
	act ("$n mutters some words and $p materializes from thin air.",ch,obj,NULL,TO_ROOM);
	send_to_char("An ancient grimoire materializes before you.\r\n",ch);

    } else if (!str_cmp(target_name, "focus"))  {
	obj = create_object (get_obj_index (49), 0);
	obj->cost=0;
	obj_to_room (obj, ch->in_room);
                obj->enchanted = TRUE;
                free_string(obj->owner);
                obj->owner = str_dup(ch->name);
                obj->timer = number_range(10, 50);
	act ("$n mutters some words and $p materializes from thin air.",ch,obj,NULL,TO_ROOM);
	send_to_char("A magical focus materializes before you.\r\n",ch);

    } else if (!str_cmp(target_name, "protection"))   {
	obj = create_object (get_obj_index (27), 0);
	obj->cost=0;
                obj->level=(ch->level+2);
	obj->value[0] = obj->level / 8;
	obj->value[1] = obj->level / 8;
	obj->value[2] = obj->level / 8;
	obj->value[3] = obj->level / 8 + 12;
                obj_to_room (obj, ch->in_room);
                obj->timer = number_range(20,30);
	act ("$n mutters some words and $p materializes from thin air.",ch,obj,NULL,TO_ROOM);
	send_to_char("Some protective field materializes before you.\r\n",ch);

    } else {
               send_to_char("You don't know how to make that kind of object.\r\n",ch);
    }
    return;
}


void spell_soul_blade( int sn, int level, CHAR_DATA *ch, void *vo) {
    OBJ_DATA *obj;
    OBJ_DATA *sblade;
    int number,type;

    obj = get_obj_here( ch, target_name );

    if ( obj == NULL )    {
        send_to_char( "Cast Soul Blade on What?\r\n", ch );
        return;
    }

    /* Nothing but NPC corpses. */

    if( obj->item_type != ITEM_CORPSE_NPC )    {
        if( obj->item_type == ITEM_CORPSE_PC )
            send_to_char( "The player wishes to keep their soul.\r\n", ch );
        else
            send_to_char( "It would serve no purpose...\r\n", ch );
        return;
    }

    if( obj->level > (level + 2) )    {
        send_to_char( "You cannot forge such a soul into a blade.\r\n", ch );
        return;
    }

    /* Create the soulblade */

    sblade = create_object ( get_obj_index (OBJ_VNUM_SOULBLADE), 0);
    sblade->level                  = obj->level;
    if (sblade->level < 5 ) sblade->level=5;
    number = UMIN(sblade->level/4 + 1, 5);
    type   = (sblade->level + 10)/number;

    sblade->value[1] = number;
    sblade->value[2] = type;

    sblade->timer = (sblade->level +10)/2;

    /* Action! */
    obj_to_room( sblade, ch->in_room );
    act( "$n waves dramatically and $p appears.",   ch, sblade, NULL, TO_ROOM );
    act( "You wave dramatically and $p appears.", ch, sblade, NULL, TO_CHAR );

    extract_obj(obj);

    return;
}

void spell_armor( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, gafn_armor ) ) {
        if (!in_seq) {
  	  if (victim == ch)
	    send_to_char("You are already protected.\r\n",ch);
	  else
	    act("$N is already armored.",ch,NULL,victim,TO_CHAR);
        }
	return;
    }
    af.type      = sn;
    af.afn	 = gafn_armor;
    af.level	 = level;
    af.duration  = 24 * (1 + level/30);
    af.modifier  = -1 * ( 50 + (level/2));
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    if (!in_seq) {
      send_to_char( "You feel someone protecting you.\r\n", victim );
      if ( ch != victim )
  	  act("$N is protected by your magic.",ch,NULL,victim,TO_CHAR);
    }
    return;
}


void spell_bless( int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA *victim = (CHAR_DATA *) vo;
AFFECT_DATA af;

    if ( victim->position == POS_FIGHTING ) {
	if (victim == ch) send_to_char("You cannot be blessed in combat!\r\n",ch);
	else act("$N is in combat!", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (!IS_SET(victim->act, ACT_UNDEAD) && !str_cmp(target_name, "evil")) {
	  send_to_char("Better don't.\r\n",ch);
                  return;
    }

    if (IS_SET(victim->act, ACT_UNDEAD) && str_cmp(target_name, "evil")) {
            if (victim == ch) {
	  send_to_char("Better don't.\r\n",ch);
                  return;
            }

            spell_CDB = getCDB(ch, victim, get_spell_spn("bless"), FALSE, get_spell_spn("bless"), 0);
            spell_CDB->atk_dam_type = DAM_ENERGY;
            spell_CDB->atk_blk_type = BLOCK_NONE;

            spell_CDB->damage = dice(level, 6);
            send_to_char( "The blessing burns your flesh.\r\n", victim );

            if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
                   SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
                   apply_damage( spell_CDB);  
                   REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
            } else {
                   apply_damage( spell_CDB);
            }

    } else {
            if (is_affected( victim, gafn_bless )) {
	  if (victim == ch) send_to_char("You are already blessed.\r\n",ch);
	  else act("$N already has divine favor.",ch,NULL,victim,TO_CHAR);
	  return;
            }

            af.type      = sn;
            af.afn	 = gafn_bless;
            af.level	 = level;
            af.duration  = 6+level;
            af.location  = APPLY_HITROLL;
            af.modifier  = 1 + (level / 8);
            af.bitvector = 0;
            affect_to_char( victim, &af );

            af.location  = APPLY_SAVING_SPELL;
            af.modifier  = -1 - (level / 8);
            affect_to_char( victim, &af );

            send_to_char( "You feel righteous.\r\n", victim );
            if ( ch != victim ) act("You grant $N the favor of your god.",ch,NULL,victim,TO_CHAR);
    }
    return;
}



void spell_blindness( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_ENERGY;
    spell_CDB->atk_blk_type = BLOCK_ABSORB;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
    if ( IS_AFFECTED(victim, AFF_BLIND) || saves_spell( level, victim ) )
	return;

    af.type      = sn;
    af.afn	 = gafn_blindness;
    af.level     = level;
    af.location  = APPLY_HITROLL;
    af.modifier  = -4;
    af.duration  = 1+level;
    af.bitvector = AFF_BLIND;
    affect_to_char( victim, &af );
    send_to_char( "You are blinded!\r\n", victim );
    act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);
    return;
}



void spell_burning_hands( int sn, int level, CHAR_DATA *ch, void *vo ) {
//    CHAR_DATA *victim = (CHAR_DATA *) vo;

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_FIRE;
    spell_CDB->atk_blk_type = BLOCK_BALL;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
    if (number_percent() < 5) harmful_effect(vo, level, TAR_CHAR_OFFENSIVE, DAM_FIRE);

   /* Set up and adjust damage... */

    spell_CDB->damage = 16 + dice((level/2), 3);

    if ( saves_spell( level, spell_CDB->defender ) )
	spell_CDB->damage /= 2;

   /* Roasting time... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

    return;
}



void spell_call_lightning( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    if ( !is_outside(ch) ) {
	send_to_char( "You must be out of doors.\r\n", ch );
	return;
    }

    if ( weather_info.sky < SKY_RAINING ) {
	send_to_char( "You need bad weather.\r\n", ch );
	return;
    }

    send_to_char( "Lightning strikes from the skies!\r\n", ch );
    act( "$n calls down Lightning from the skies!",
	  ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next ) {

	vch_next	= vch->next;

       /* All characters in a room... */

	if ( vch->in_room == NULL )
	    continue;

       /* Those nearby see the lighting... */

	if ( vch->in_room->area == ch->in_room->area
	&&   is_outside(vch)
	&&   IS_AWAKE(vch) ) {
	    send_to_char( "Lightning flashes in the sky.\r\n", vch );
                }

       /* Can only hit those in the same sub-area... */

	if ( vch->in_room->subarea != ch->in_room->subarea
                || ch==vch) {
                      continue;
                }
  
       /* Reset spell_CDB... */

        spell_CDB = getCDB(ch, vch, sn, FALSE, sn, 0);

       /* Set up combat data... */

        spell_CDB->atk_dam_type = DAM_LIGHTNING;
        spell_CDB->atk_blk_type = BLOCK_LIGHTNING;
        spell_CDB->atk_desc = "lightning";

       /* Bail out if it misses... */
 
        if (!check_spell_hit(spell_CDB, spell_roll)) {
          continue;
        }   

       /* Set up and adjust damage... */

        dam = dice(level/2, 8) + 3;

        if ( vch->in_room != ch->in_room ) {
          dam /= 2;
        }

        spell_CDB->damage = dam;

        if ( saves_spell( level, spell_CDB->defender ) ) {
        	spell_CDB->damage /= 2;
        }

       /* Charging time... */

 if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

    }

    return;
}

/* RT calm spell stops all fighting in the room */

void spell_calm( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *vch;
    int mlevel = 0;
    int count = 0;
    int high_level = 0;    
    int chance;
    AFFECT_DATA af;

    /* get sum of all mobile levels in the room */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if (vch->position == POS_FIGHTING)
	{
	    count++;
	    if (IS_NPC(vch))
	      mlevel += vch->level;
	    else
	      mlevel += vch->level/2;
	    high_level = UMAX(high_level,vch->level);
	}
    }

    /* compute chance of stopping combat */
    chance = 4 * level - high_level + 2 * count;

    if (IS_IMMORTAL(ch)) /* always works */
      mlevel = 0;

    if (number_range(0, chance) >= mlevel)  /* hard to stop large fights */
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
   	{
	    if (IS_NPC(vch) && (IS_SET(vch->imm_flags,IMM_MAGIC) ||
				IS_SET(vch->act,ACT_UNDEAD)))
	      return;

	    if (IS_AFFECTED(vch,AFF_CALM) || IS_AFFECTED(vch,AFF_BERSERK)
	    ||  is_affected(vch, gafn_frenzy))
	      return;
	    
	    send_to_char("A wave of calm passes over you.\r\n",vch);

	    if (vch->fighting || vch->position == POS_FIGHTING)
	      stop_fighting(vch,FALSE);

	    af.type = sn;
                    af.afn = gafn_calm;
  	    af.level = level;
	    af.duration = level/4;
	    af.location = APPLY_HITROLL;
	    if (!IS_NPC(vch))
	      af.modifier = -5;
	    else
	      af.modifier = -2;
	    af.bitvector = AFF_CALM;
	    affect_to_char(vch,&af);

	    af.location = APPLY_DAMROLL;
	    affect_to_char(vch,&af);
	}
    }
}

void spell_cancellation( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;
 
    level += 2;

    if ((!IS_NPC(ch) && IS_NPC(victim) && 
	 !(IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) ) ||
        (IS_NPC(ch) && !IS_NPC(victim)) )
    {
	send_to_char("You failed, try dispel magic.\r\n",ch);
	return;
    }

    /* unlike dispel magic, the victim gets NO save */
 
    /* begin running through the spells */
	
 
    if (check_dispel(level,victim, gafn_absorb_magic))
        found = TRUE;
    
    if (check_dispel(level,victim, gafn_regeneration))
        found = TRUE;

    if (check_dispel(level,victim, gafn_armor))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_bless))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_blindness))
    {
        found = TRUE;
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }

    if (check_dispel(level,victim, gafn_calm))
    {
	found = TRUE;
	act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim, gafn_sex_change))
    {
        found = TRUE;
        act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
	send_to_char("You feel more like yourself again.\r\n",victim);
    }
 
    if (check_dispel(level,victim, gafn_charm))
    {
        found = TRUE;
        act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim, gafn_chill_touch))
    {
        found = TRUE;
        act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim, gafn_curse))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_detect_evil))
        found = TRUE;
    
    if (check_dispel(level,victim, gafn_detect_good))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_detect_hidden))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_detect_invis))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_detect_hidden))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_detect_magic))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_faerie_fire))
    {
        act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
	
	if (check_dispel(level,victim, gafn_fear))
    {
        act("$n no longer looks so scared.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim, gafn_fly))
    {
        act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim, gafn_frenzy)
    || check_dispel(level,victim, gafn_berserk))    {
	act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim, gafn_giant_strength))
    {
        act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim, gafn_haste))    {
	act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level,victim, gafn_slow))    {
	act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim, gafn_infravision))   found = TRUE;
    if (check_dispel(level,victim, gafn_farsight))    found = TRUE;
 
    if (check_dispel(level,victim, gafn_invisability))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
    if (check_dispel(level,victim, gafn_mind_meld))
    {
        act("$n has regained $s senses.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
    
    if (check_dispel(level,victim, gafn_mute))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_pass_door))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_protection_evil))
        found = TRUE;
    
    if (check_dispel(level,victim, gafn_protection_good))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_sanctuary))
    {
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim, gafn_shield))
    {
        act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim, gafn_sleep))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_hard_skin))    {
        act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim, gafn_globe))    {
        act("$n's globe of protection vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim, gafn_size))    {
        act("$n's sizereturns to normal.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim, gafn_vocalize))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_weakness))    {
        act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim, gafn_mist))    {
        act("$n looks solid again.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim, gafn_water_breathing))
        found = TRUE;
 
    if (found)
        send_to_char("Ok.\r\n",ch);
    else
        send_to_char("Spell failed.\r\n",ch);
}

void spell_cause_light( int sn, int level, CHAR_DATA *ch, void *vo ) {
   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_HARM;
    spell_CDB->atk_blk_type = BLOCK_ABSORB;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
   /* Set up and adjust damage... */

    spell_CDB->damage = dice(1, 8) + level/3;

    if ( saves_spell( level, spell_CDB->defender ) ) {
     	spell_CDB->damage /= 2;
    }

   /* Apply the damage... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
 } else {
    apply_damage( spell_CDB);
 }

 return;
}


void spell_cause_critical( int sn, int level, CHAR_DATA *ch, void *vo ) {
   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_HARM;
    spell_CDB->atk_blk_type = BLOCK_ABSORB;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
   /* Set up and adjust damage... */

    spell_CDB->damage = dice(3, 8) + level;

    if ( saves_spell( level, spell_CDB->defender ) ) {
     	spell_CDB->damage /= 2;
    }

   /* Apply the damage... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
 } else {
    apply_damage( spell_CDB);
 }

    return;
}



void spell_cause_serious( int sn, int level, CHAR_DATA *ch, void *vo ) {
   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_HARM;
    spell_CDB->atk_blk_type = BLOCK_ABSORB;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
   /* Set up and adjust damage... */

    spell_CDB->damage = dice(2, 8) + level/2;

    if ( saves_spell( level, spell_CDB->defender ) ) {
     	spell_CDB->damage /= 2;
    }

   /* Apply the damage... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
 } else {
    apply_damage( spell_CDB);
 }

    return;
}

void spell_youth(int sn, int level, CHAR_DATA *ch, void *vo) {
        CHAR_DATA *victim = (CHAR_DATA *) vo;

        if ( IS_NPC(victim) ) {
	    send_to_char("This spell only affects players.", ch);
	    return;
        }

         if ( IS_SET(ch->act,ACT_UNDEAD)) {
	    send_to_char("You're beyond that point.", ch);
	    return;
        }

        send_to_char( "You feel a great weight lifted off your shoulders.\r\n", victim );
        act("$n's skin takes on a youthful glow.",victim,NULL,NULL,TO_ROOM);
	victim->pcdata->age_mod += number_range ( 1, 3 );
                if (get_age(victim) < 17) victim->pcdata->age_mod += 3;
	return;
}    

void spell_age(int sn, int level, CHAR_DATA *ch, void *vo) {
        CHAR_DATA *victim = (CHAR_DATA *) vo;

        if ( IS_NPC(victim))      {
            send_to_char("This spell only affects players.", ch);
            return;
        }

        send_to_char( "Your skin crawls as your life is sucked away\r\n",victim );
        act("$n appears to grow old before your eyes.",victim,NULL,NULL,TO_ROOM);
        victim->pcdata->age_mod -= number_range ( 1, 5 );
        return;
}

void spell_chain_lightning(int sn, int level, CHAR_DATA *ch, void *vo) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *tmp_vict,*last_vict,*next_vict;
    bool found;
    int dam;

    /* first strike */

    act("A lightning bolt leaps from $n's hand and arcs to $N.",
        ch,NULL,victim,TO_ROOM);
    act("A lightning bolt leaps from your hand and arcs to $N.",
	ch,NULL,victim,TO_CHAR);
    act("A lightning bolt leaps from $n's hand and hits you!",
	ch,NULL,victim,TO_VICT);  

   /* Set up and adjust damage... */

    spell_CDB->damage = dice(level, 6);

    if ( saves_spell( level, spell_CDB->defender ) ) {
     	spell_CDB->damage /= 3;
    }

   /* Charging time... */

    apply_damage(spell_CDB);

    level -= 4;   /* decrement damage */

   /* Fry a few people... */

    found = FALSE;

    for ( tmp_vict = ch->in_room->people; 
          tmp_vict != NULL && level > 0; 
          tmp_vict = next_vict) { 
	
     /* Save next victim incase this one dies... */

      next_vict = tmp_vict->next_in_room;

     /* Do not fry the caster (yet)... */

      if (tmp_vict == ch) {
        continue;
      }

     /* Ok, frying time... */

      if (!is_safe_spell(ch,tmp_vict,TRUE)) { 
	
        found = TRUE;
        last_vict = tmp_vict;
        act("The bolt arcs to $n!",tmp_vict,NULL,NULL,TO_ROOM);

       /* Reset spell_CDB... */

        spell_CDB = getCDB(ch, tmp_vict, sn, FALSE, sn, 0);

       /* Set up combat data... */

        spell_CDB->atk_dam_type = DAM_LIGHTNING;
        spell_CDB->atk_blk_type = BLOCK_LIGHTNING;
        spell_CDB->atk_desc = "lightning";

       /* Bail out if it misses... */
 
        if (!check_spell_hit(spell_CDB, spell_roll)) {
          continue;
        }   

        act("The bolt hits you!",tmp_vict,NULL,NULL,TO_CHAR);

       /* Set up and adjust damage... */

        spell_CDB->damage = dice(level, 6);

        if ( saves_spell( level, spell_CDB->defender ) ) {
          spell_CDB->damage /= 3;
        }

       /* Charging time... */

 if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

        level -= 4;  /* decrement damage */
      }
    }

    if ( level > 0 
      && ch != NULL) {

      if (found) {
	act("The bolt seems to have fizzled out.",ch,NULL,NULL,TO_ROOM);
	act("The bolt grounds out through your body.",
	ch,NULL,NULL,TO_CHAR);
	return;
      } else {
        dam = dice(level,6);
        if (saves_spell(level,ch))
          dam /= 3;
        env_damage(ch, dam, DAM_LIGHTNING,
           "@v2 is struck by his own lightning bolt! [@n0]{x\r\n",
           "You are struck by your own lightning bolt! [@n0]{x\r\n" );
          
        level -= 4;  /* decrement damage */
      }
    }

    return;
}
	  

void spell_change_sex( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, gafn_sex_change )) {
	if (victim == ch)
	  send_to_char("Your sex has already been changed.\r\n",ch);
	else
	  act("$N has already had $s(?) sex changed.",ch,NULL,victim,TO_CHAR);
	return;
    }
    if (saves_spell(level , victim))
	return;	
    af.type      = sn;
    af.afn	 = gafn_sex_change;
    af.level     = level;
    af.duration  = 2 * level;
    af.location  = APPLY_SEX;
    do
    {
	af.modifier  = number_range( 0, 2 ) - victim->sex;
    }
    while ( af.modifier == 0 );
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel different.\r\n", victim );
    act("$n doesn't look like $mself anymore...",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_lesser_possession(int sn, int level, CHAR_DATA *ch, void *vo) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( ch->desc == NULL ) return;
        
    if ( ch->desc->original != NULL ) {
	send_to_char( "You are already switched.\r\n", ch );
	return;
    }

    if ( victim == NULL ) {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( victim == ch ) {
	return;
    }

    if (!IS_NPC(victim)
    || !IS_AFFECTED(victim,AFF_CHARM)
    || IS_SET(victim->imm_flags, IMM_CHARM)){
	send_to_char("You can only switch into charmed pets.\r\n",ch);
	return;
    }

    if (victim->leader != ch) {
	send_to_char( "Not your follower.\r\n", ch );
	return;
    }

    if ( victim->desc != NULL ) {
	send_to_char( "Character in use.\r\n", ch );
	return;
    }
	
   /* Zeran - to fix null pointer dereferencing all over in the code,
	give switched player access to his pcdata fields. */

    victim->pcdata = ch->pcdata;
    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    victim->comm = ch->comm;
    victim->lines = ch->lines;
    REMOVE_BIT(victim->affected_by, AFF_CHARM);

    act("You possess $N.",ch, NULL, victim,TO_CHAR);
    act("$n possesses $N.",ch, NULL, victim,TO_ROOM);
    notify_message (ch, WIZNET_SWITCH, TO_IMM, victim->short_descr);

    return;
}


void spell_greater_possession(int sn, int level, CHAR_DATA *ch, void *vo) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int chance;

    if ( ch->desc == NULL ) return;
       
    if ( ch->desc->original != NULL ) {
	send_to_char( "You are already switched.\r\n", ch );
	return;
    }

    if ( victim == NULL ) {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( victim == ch ) {
	send_to_char( "Ok.\r\n", ch );
	return;
    }

    if (!IS_NPC(victim)
    || IS_SET(victim->act, ACT_AGGRESSIVE)
    || IS_SET(victim->act, ACT_INVADER)
    || IS_SET(victim->imm_flags, IMM_MAGIC)
    || IS_SET(victim->act, ACT_IS_HEALER)
    || IS_SET(victim->act, ACT_IS_ALIENIST)
    || IS_SET(victim->act, ACT_PRACTICE)
    || victim->pIndexData->pShop != NULL
    || IS_SET(victim->imm_flags, IMM_CHARM)){
	send_to_char("You can't switch into that mob.\r\n",ch);
	return;
    }

    if ( victim->desc != NULL ) {
	send_to_char( "Character in use.\r\n", ch );
	return;
    }
	
    chance = (level - victim->level)*5;
    if (number_percent() < chance) {
        victim->pcdata = ch->pcdata;
        ch->desc->character = victim;
        ch->desc->original  = ch;
        victim->desc        = ch->desc;
        victim->comm = ch->comm;
        victim->lines = ch->lines;
        act("You possess $N.",ch, NULL, victim,TO_CHAR);
        act("$n possesses $N.",ch, NULL, victim,TO_ROOM);
        notify_message (ch, WIZNET_SWITCH, TO_IMM, victim->short_descr);
     } else {
        if (victim->level < ch->level) {
             do_flee(victim,"");
        } else {
             SET_BIT(victim->act, ACT_AGGRESSIVE);
        }
     }
     return;
}


void spell_charm_person( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == ch )    {
	send_to_char( "You like yourself even better!\r\n", ch );
	return;
    }

    if (!str_cmp(race_array[victim->race].name,"undead")
    || IS_SET(victim->act, ACT_UNDEAD)) {
        send_to_char( "You can't charm an undead.\r\n", ch );
        return;
      }

    if (!str_cmp(race_array[victim->race].name,"machine")
    || IS_SET(victim->form, FORM_MACHINE)) {
        send_to_char( "You can't charm a machine.\r\n", ch );
        return;
      }

   if (IS_SET(victim->act, ACT_AGGRESSIVE)) level /= 2;

    if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   level < victim->level
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   saves_spell( level, victim ) )
	return;

    if (IS_SET(victim->in_room->room_flags,ROOM_LAW)
    || IS_SET(victim->in_room->area->area_flags, AREA_LAW))    {
	send_to_char("Marduk does not allow charming in the city limits.\r\n",ch);
	return;
    }

    if ( victim->master )
    stop_follower( victim );
    add_follower( victim, ch );
    victim->leader = ch;
    af.type      = sn;
    af.afn	 = gafn_charm;
    af.level	 = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );
    act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
    if ( ch != victim ) act("$N looks at you with adoring eyes.",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_chill_touch( int sn, int level, CHAR_DATA *ch, void *vo ) {
//    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_COLD;
    spell_CDB->atk_blk_type = BLOCK_ALL;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
   /* Set up and adjust damage... */

    spell_CDB->damage = 10 + dice((level/4), 3);

    if ( saves_spell( level, spell_CDB->defender ) ) {
	spell_CDB->damage /= 2;
    } else {
	act("$n turns blue and shivers.",
             spell_CDB->defender,NULL,NULL,TO_ROOM);
	af.type      = sn;
	af.afn	     = gafn_chill_touch;	
        af.level     = level;
	af.duration  = 6;
	af.location  = APPLY_STR;
	af.modifier  = -1 * (1 + (spell_CDB->attacker->level/15));
	af.bitvector = 0;
	affect_join( spell_CDB->defender, &af );
    }

   /* Chilling time... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

    return;
}



void spell_colour_spray( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_LIGHT;
    spell_CDB->atk_blk_type = BLOCK_LIGHT;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
   /* Set up and adjust damage... */

    spell_CDB->damage = 30 + dice((level/3), 5);

    if ( saves_spell( level, spell_CDB->defender ) ) {
	spell_CDB->damage /= 2;
    } else {
	spell_blindness(get_skill_sn("blindness"),
                                    level/2, ch, (void *) victim);
    }

   /* Illuminating experience... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

    return;
}

/* Hmmmm....

void spell_darkness( int sn, int level, CHAR_DATA *ch, void *vo ) {
    int chance;

    chance = number_percent();
    if (chance >=5)    {
	--ch->in_room->light;
    }
}
*/

void spell_continual_light( int sn, int level, CHAR_DATA *ch, void *vo ) {
    OBJ_DATA *light;

    light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
    obj_to_room( light, ch->in_room );
    light->timer = level * 72;                          /* 3 days per level */
    if (!in_seq) { 
      act( "$n twiddles $s thumbs and $p appears.",   ch,light,NULL,TO_ROOM );
      act( "You twiddle your thumbs and $p appears.", ch,light,NULL,TO_CHAR );
    }
    return;
}

void spell_mage_light( int sn, int level, CHAR_DATA *ch, void *vo ) {
    OBJ_DATA *light;

    light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
    light->timer = 12 + level/3;           /* 12 hours plus 1 hour/3 levels */
    obj_to_room( light, ch->in_room );
    if (!in_seq) { 
      act( "$n twiddles $s thumbs and $p appears.",   ch,light,NULL,TO_ROOM );
      act( "You twiddle your thumbs and $p appears.", ch,light,NULL,TO_CHAR );
    }
    return;
}


void spell_control_weather( int sn, int level, CHAR_DATA *ch, void *vo ) {

    if ( !str_cmp( target_name, "better" ) ) {
      if ( weather_info.sky > SKY_CLOUDLESS ) {
        weather_info.sky -= 1;
        weather_update(FALSE);
      }
    } else if ( !str_cmp( target_name, "worse" ) ) {
      if ( weather_info.sky < SKY_LIGHTNING ) {
        weather_info.sky += 1;
        weather_update(FALSE);
      }
    } else {
      if (number_bits(2) > 1) {
        weather_info.sky += 1;
      } else { 
        weather_info.sky -= 1;
      }
      weather_info.sky = URANGE(SKY_CLOUDLESS, weather_info.sky, SKY_LIGHTNING);
      weather_update(FALSE);
    } 

    send_to_char( "Ok.\r\n", ch );

    return;
}


void spell_create_food( int sn, int level, CHAR_DATA *ch, void *vo ) {
    OBJ_DATA *mushroom;

    mushroom = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ), 0 );
    mushroom->value[0] = 4 + (level/5);
    obj_to_room( mushroom, ch->in_room );
    if (!in_seq) {
      act( "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM );
      act( "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR );
    }
    return;
}

void spell_create_buffet( int sn, int level, CHAR_DATA *ch, void *vo) {
     OBJ_DATA *mushroom;
     int counter=0;
     for(counter=0; counter < (level/10) + 1; counter++) {
       mushroom = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ), 0 );
       mushroom->value[0] = 4 + (level / 10);
       obj_to_room( mushroom, ch->in_room );
       if (!in_seq) {
         act( "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM );
         act( "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR );
       } 
     }
     return;
}

void spell_create_spring( int sn, int level, CHAR_DATA *ch, void *vo ) {
    OBJ_DATA *spring;

        spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 );
        spring->timer = level;
        obj_to_room( spring, ch->in_room );
          if (!strcmp(target_name, "blood")) {
               spring->value[0] = LIQ_BLOOD;
               free_string(spring->name);
               spring->name = strdup("spring blood");
               free_string(spring->short_descr);
               spring->short_descr = strdup("a spring of blood");
               free_string(spring->description);
               spring->description = strdup("A spring of blood flows from the ground here.\n");
          }
        if (!in_seq) {
          act( "$p flows from the ground.", ch, spring, NULL, TO_ROOM );
          act( "$p flows from the ground.", ch, spring, NULL, TO_CHAR );
        }
        return;



}


void spell_create_water( int sn, int level, CHAR_DATA *ch, void *vo ) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int water;

    if ( obj->item_type != ITEM_DRINK_CON )    {
	send_to_char( "It is unable to hold water.\r\n", ch );
	return;
    }

    if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )    {
	send_to_char( "It contains some other liquid.\r\n", ch );
	return;
    }

    water = UMIN(	level * (weather_info.sky >= SKY_RAINING ? 4 : 2), obj->value[0] - obj->value[1]);
  
    if ( water > 0 )    {
	obj->value[2] = LIQ_WATER;
	obj->value[1] += water;
	if ( !is_name( "water", obj->name ) )	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "%s water", obj->name );
	    free_string( obj->name );
	    obj->name = str_dup( buf );
	}
	act( "$p is filled.", ch, obj, NULL, TO_CHAR );
    }

    return;
}


void spell_cure_blindness( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gafn_blindness ) )
    {
        if (victim == ch)
          send_to_char("You aren't blind.\r\n",ch);
        else
          act("$N doesn't appear to be blinded.",ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (check_dispel(level, victim, gafn_blindness))
    {
        send_to_char( "Your vision returns!\r\n", victim );
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }
    else
        send_to_char("Spell failed.\r\n",ch);

    return;
}

void spell_transformation( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int loss;

    if (victim->level<15
    || IS_NPC(victim)
    || IS_SET(victim->act, ACT_UNDEAD)) {
	send_to_char( "Your victim is not worthy to become a Were.\r\n", ch );
	return;
     }

    if (IS_SET(victim->act, ACT_WERE)) {
	send_to_char("It's already a Were.\r\n", ch );
	return;
     }

    if(!IS_NPC(ch)
    || IS_AFFECTED(ch, AFF_CHARM)
    || ch->desc) {
        if (victim->level*2 > ch->level) {
	send_to_char( "Your victim is too powerful.\r\n", ch );
	return;
         }
   
         if (weather_info.moon != MOON_FULL) {
	send_to_char( "The moon is not right.\r\n", ch );
	return;
         }
         WAIT_STATE(ch, 64);
         loss = victim->exp;
         gain_exp(ch, -1 * loss, FALSE);
    }

    SET_BIT(victim->act,ACT_WERE);

    if (!strcmp(target_name, "wolf")) {
         victim->were_type = 31;
    } else if (!strcmp(target_name, "cat")) {
         victim->were_type = 12;
    } else if (!strcmp(target_name, "fox")) {
         victim->were_type = 17;
    } else {
         victim->were_type = 31;
    }

    send_to_char ("You transfer the Power of Nature !\r\n",ch);
    send_to_char ("You feel your inner strength awaken!\r\n",victim);
    return;
}


void spell_cure_critical( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;
    int tolimb;

    heal = dice(3, 8) + level - 6;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    if (number_percent()<50) {
           REMOVE_BIT(victim->form,FORM_BLEEDING);
           tolimb=number_range(0,5);
           if (ch->limb[tolimb]>0 && ch->limb[tolimb]<10) {
                   if (number_range(0,ch->limb[tolimb])/3 < 2) ch->limb[tolimb] -=1;
           }
           update_wounds(ch);
    }
    update_pos( victim );
    send_to_char( "You feel better!\r\n", victim );
    if ( ch != victim )
	send_to_char( "Ok.\r\n", ch );
    return;
}

/* RT added to cure plague */
void spell_cure_disease( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gafn_plague ) )
    {
        if (victim == ch)
          send_to_char("You aren't ill.\r\n",ch);
        else
          act("$N doesn't appear to be diseased.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    if (check_dispel(level,victim, gafn_plague))
    {
	send_to_char("Your sores vanish.\r\n",victim);
	act("$n looks relieves as $s sores vanish.",victim,NULL,NULL,TO_ROOM);
    }
    else
	send_to_char("Spell failed.\r\n",ch);
}


void spell_cure_light( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;
    int tolimb;

    heal = dice(1, 8) + level / 3;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    if (number_percent()<10) {
          REMOVE_BIT(victim->form,FORM_BLEEDING);
           tolimb=number_range(0,5);
           if (ch->limb[tolimb]>0 && ch->limb[tolimb]<10) {
                   if (number_range(0,ch->limb[tolimb])/2 < 2) ch->limb[tolimb] -=1;
           }
           update_wounds(ch);
    }
    update_pos( victim );
    send_to_char( "You feel better!\r\n", victim );
    if ( ch != victim )
	send_to_char( "Ok.\r\n", ch );
    return;
}


void spell_cure_poison( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
 
    if ( !is_affected( victim, gafn_poison ) )
    {
        if (victim == ch)
          send_to_char("You aren't poisoned.\r\n",ch);
        else
          act("$N doesn't appear to be poisoned.",ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (check_dispel(level,victim, gafn_poison))
    {
        send_to_char("A warm feeling runs through your body.\r\n",victim);
        act("$n looks much better.",victim,NULL,NULL,TO_ROOM);
    }
    else
        send_to_char("Spell failed.\r\n",ch);
}

void spell_cure_serious( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;
    int tolimb;

    heal = dice(2, 8) + level /2 ;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    if (number_percent()<25) {
           REMOVE_BIT(victim->form,FORM_BLEEDING);
           tolimb=number_range(0,5);
           if (ch->limb[tolimb]>0 && ch->limb[tolimb]<10) {
                   if (number_range(0,ch->limb[tolimb])/2 < 2) ch->limb[tolimb] -=1;
           }
           update_wounds(ch);
    }
    update_pos( victim );
    send_to_char( "You feel better!\r\n", victim );
    if ( ch != victim )
	send_to_char( "Ok.\r\n", ch );
    return;
}


void spell_curse( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_CURSE) || saves_spell( level, victim ) )
	return;
    af.type      = sn;
    af.afn	 = gafn_curse;
    af.level     = level;
    af.duration  = 2*level;
    af.location  = APPLY_HITROLL;
    af.modifier  = -1 * (level / 8);
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = level / 8;
    affect_to_char( victim, &af );

    send_to_char( "You feel unclean.\r\n", victim );
    if ( ch != victim )
	act("$N looks very uncomfortable.",ch,NULL,victim,TO_CHAR);
    return;
}


void spell_exorcism(int sn, int level, CHAR_DATA *ch, void *vo) {
CHAR_DATA *victim = (CHAR_DATA *) vo;

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_HOLY;
    spell_CDB->atk_blk_type = BLOCK_ABSORB;

   /* Set up and adjust damage... */

    spell_CDB->damage = dice(level, 10);
    if ( IS_SET(victim->act,ACT_UNDEAD)) spell_CDB->damage *= 2;

   /* Theatrics... */

    act("$n invokes the wrath of his God on $N!", ch,NULL,victim,TO_ROOM);
    act("$n has called forth the Angels of heaven to exorcise you!", ch,NULL,victim,TO_VICT);
    send_to_char("You call forth Angels of God!\r\n",ch);
    
   /* Upset gods are a pain... */ 

    if ( !IS_NPC(ch) && !IS_GOOD(ch) ) {
        act("The Angels turn and attack $n!", ch, NULL,victim,TO_ROOM);
        send_to_char("The Angels turn to attack you!\r\n",ch);

        if ( saves_spell( level, ch ) ) {
	  spell_CDB->damage /= 2;
        } 

        env_damage(ch, spell_CDB->damage, DAM_HOLY,
           "@v2 is struck down by his own Angels! [@n0]{x\r\n",
           "The Angles strike you down! [@n0]{x\r\n" );

        return;  
    }

   /* ok, we're live... */

    ch->alignment = UMAX(1000, ch->alignment + 50);

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  

   /* Give the defender a saving throw... */
 
    if ( saves_spell( level, spell_CDB->defender ) ) {
	spell_CDB->damage /= 2;
    } 

   /* Illuminating experience... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

    return;
}

/* Mik demonfire, evil version of Exorcism */

void spell_demonfire(int sn, int level, CHAR_DATA *ch, void *vo) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_NEGATIVE;
    spell_CDB->atk_blk_type = BLOCK_ABSORB;

   /* Set up and adjust damage... */

    spell_CDB->damage = dice(level, 10);

   /* Theatrics... */

    act("$n commands a mutlitude of demons to attack $N!",
         ch,NULL,victim,TO_ROOM);
    act("$n has command a multitude of demons to slay you!",
         ch,NULL,victim,TO_VICT);
    send_to_char("You send forth a multitude of demons!\r\n",ch);
    
   /* Upset gods are a pain... */ 

    if ( !IS_NPC(ch) && !IS_EVIL(ch) ) {
        act("The Demons turn and attack $n!",
             ch,NULL,victim,TO_ROOM);
        send_to_char("The Demons turn and attack you!\r\n",ch);

        if ( saves_spell( level, ch ) ) {
	  spell_CDB->damage /= 2;
        } 

        env_damage(ch, spell_CDB->damage, DAM_NEGATIVE,
           "@v2 is pulled down by his own Demons! [@n0]{x\r\n",
           "The Demons pull you down! [@n0]{x\r\n" );

        return;  
    }

   /* ok, we're live... */

    ch->alignment = UMAX(-1000, ch->alignment - 50);

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  

   /* Give the defender a saving throw... */
 
    if ( saves_spell( level, spell_CDB->defender ) ) {
	spell_CDB->damage /= 2;
    } 

   /* Illuminating experience... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

    return;
}

void spell_detect_evil( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_EVIL) )    {
	if (victim == ch)
	  send_to_char("You can already sense evil.\r\n",ch);
	else
	  act("$N can already detect evil.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.type      = sn;
    af.afn	 = gafn_detect_evil;	
    af.level	 = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_EVIL;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\r\n", victim );
    if ( ch != victim )
	send_to_char( "Ok.\r\n", ch );
    return;
}

void spell_detect_good( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected(victim, gafn_detect_good) )
    {
	if (victim == ch)
	  send_to_char("You can already sense good.\r\n",ch);
	else
	  act("$N can already detect good.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.type      = sn;
    af.afn 	 = gafn_detect_good;
    af.level	 = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
	af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\r\n", victim );
    if ( ch != victim )
	send_to_char( "Ok.\r\n", ch );
    return;
}


void spell_water_breathing( int sn, int level, CHAR_DATA *ch, void *vo ) {

    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_WATER_BREATHING) ) {

	if (victim == ch)
	  send_to_char("You can already breath water.\r\n",ch);
	else
	  act("$N can already breath water.",ch,NULL,victim,TO_CHAR);

	return;
    }

    af.type      = sn;
    af.afn 	 = gafn_water_breathing;
    af.level	 = level;
    af.duration  = level/6 + dice(1,6);
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_WATER_BREATHING;

    affect_to_char( victim, &af );

    send_to_char( "The air suddenly feels very dry!\r\n", victim );

    if ( ch != victim )
	send_to_char( "Ok.\r\n", ch );

    return;
}


void spell_true_dreaming( int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA *victim = (CHAR_DATA *) vo;

   /* Only works on dreaming victims... */

    if ( victim->waking_room == NULL ) { 
      if ( victim != ch ) send_to_char("They are not dreaming!\r\n", ch);
      else send_to_char("You are not dreaming!\r\n", ch);
      return;
    }
 
   /* And not the badly injured... */
 
    if ( victim->position < POS_STUNNED ) {
      send_to_char("Dreams won't help them now...\r\n", ch);
      return;
    }

    if (IS_SET(victim->in_room->room_flags, ROOM_NO_DREAM)) {
          send_to_char( "{cYour dreams are disturbing and frightening.{x\r\n", victim);
          return;
    }

   /* Others get a save... */

    if ( victim != ch ) {
      if ( saves_spell(level, victim) ) {
        send_to_char("You feel a little strange.\r\n", victim);
        send_to_char("They resist your spell!\r\n", ch);
        return;
      } 
    }

   /* Now you must make a dreaming check... */

    if ( !check_skill(ch, gsn_dreaming, ch->level - victim->level) ) {

      if (victim != ch) {
        send_to_char("You feel very strange!\r\n", victim);
      }

      send_to_char("Your dream manipulations fail!\r\n", ch);

      return;
    }

   /* Success, transform from dreaming to really here... */

    victim->waking_room = NULL;
    victim->dreaming_room = NULL;

    send_to_char("You feel like you have been turned inside out!\r\n", victim);

    if ( victim != ch ) {
      send_to_char("Success! They are now here for real!\r\n", ch);
    }

    return;
}


void spell_rude_awakening( int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA *victim = (CHAR_DATA *) vo;

   /* Only works on dreaming victims... */

    if ( victim->waking_room == NULL ) { 
      if ( victim != ch ) send_to_char("They are not dreaming!\r\n", ch);
      else send_to_char("You are not dreaming!\r\n", ch);
      return;
    }

   /* And not the badly injured... */
 
    if ( victim->position < POS_STUNNED ) {
      send_to_char("Dreams won't help them now...\r\n", ch);
      return;
    }

    if (IS_SET(victim->in_room->room_flags, ROOM_NO_DREAM)) {
          send_to_char( "{cYour dreams are disturbing and frightening.{x\r\n", victim);
          return;
    }

   /* Others get a save... */

    if ( victim != ch ) {
      if ( saves_spell(level, victim) ) {

        if ( victim->position == POS_SLEEPING
          || victim->position == POS_SITTING
          || victim->position == POS_RESTING ) {
          set_activity(victim, POS_STANDING, NULL, ACV_NONE, NULL);
        }  

        send_to_char("You feel wide awake!\r\n", victim);
        send_to_char("They resist your spell!\r\n", ch);

        return;
      } 
    }

   /* Now you must make a dreaming check... */

    if ( !check_skill(ch, gsn_dreaming, ch->level - victim->level) ) {

      if ( victim->position == POS_SLEEPING
        || victim->position == POS_SITTING
        || victim->position == POS_RESTING ) {
        set_activity(victim, POS_STANDING, NULL, ACV_NONE, NULL);
      }  

      if (victim != ch) {
        send_to_char("You feel wide awake!\r\n", victim);
      }

      send_to_char("Your dream manipulations fail!\r\n", ch);

      return;
    }

   /* Success, kick them back to the real world... */

    stop_fighting(victim, TRUE);
 
    act( "$n abruptly vanishes!", victim, NULL, NULL, TO_ROOM );

    victim->dreaming_room = victim->in_room;  /* Allow return */
    victim->nightmare = FALSE;
    add_tracks(ch, DIR_NONE, DIR_OTHER);

    char_from_room(victim);

    victim->in_zone = -1;

    char_to_room(victim, victim->waking_room);

    add_tracks(ch, DIR_OTHER, DIR_HERE);

    set_activity(victim, POS_STANDING, NULL, ACV_NONE, NULL);

    send_to_char("{mYou are suddenly wide awake!{x\r\n", victim);

    if ( victim != ch ) {
      send_to_char("Success! They are returned to the real world!\r\n", ch);
    }

    do_look(victim, "auto");

    return;
}


void spell_recurring_dream( int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA *victim = (CHAR_DATA *) vo;

   /* Only works on non-dreaming victims... */

    if ( victim->waking_room != NULL ) { 
      if ( victim != ch ) send_to_char("They are already dreaming!\r\n", ch);
      else send_to_char("You are already dreaming!\r\n", ch);
      return;
    }
 
   /* And not the badly injured... */
 
    if ( victim->position < POS_STUNNED ) {
      send_to_char("Dreams won't help them now...\r\n", ch);
      return;
    }

   /* ...with a dream to return to */

    if ( victim->dreaming_room == NULL ) { 
      if ( victim != ch ) send_to_char("They have lost their dream!\r\n", ch);
      else send_to_char("You have lost your dream!\r\n", ch);
      return;
    }
 
    if (IS_SET(victim->in_room->room_flags, ROOM_NO_DREAM)) {
          send_to_char( "{cYour dreams are disturbing and frightening.{x\r\n", victim );
          return;
    }

   /* Others get a save... */

    if ( victim != ch ) {
      if ( saves_spell(level, victim) ) {

        send_to_char("You feel a little sleepy!\r\n", victim);
        send_to_char("They resist your spell!\r\n", ch);

        return;
      } 
    }

   /* Now you must make a dreaming check... */

    if ( !check_skill(ch, gsn_dreaming, ch->level - victim->level) ) {

      stop_fighting(victim, TRUE);

      set_activity(victim, POS_RESTING, NULL, ACV_NONE, NULL);

      send_to_char("You feel very sleepy!\r\n", victim);
      send_to_char("Your dream manipulations fail!\r\n", ch);

      victim->dreaming_room = NULL;

      return;
    }

   /* Success, transform from dreaming to really here... */

    stop_fighting(victim, TRUE);
 
    act( "$n goes to sleep and fades away to the world of dreams...", 
            victim, NULL, NULL, TO_ROOM );

    send_to_char("You feel very, very tired...{x\r\n", victim);

    victim->waking_room = victim->in_room;    /* Allow awaken */

    add_tracks(ch, DIR_NONE, DIR_OTHER);

    char_from_room(victim);

    victim->in_zone = -1;

    char_to_room(victim, victim->dreaming_room);

    add_tracks(ch, DIR_OTHER, DIR_HERE);

    set_activity(victim, POS_STANDING, NULL, ACV_NONE, NULL);

    send_to_char("{mWhen you awaken you are somewhere else!{x\r\n", victim);

    if ( victim != ch ) {
      send_to_char("Success! They are returned to thier dreams!\r\n", ch);
    }

    do_look(victim, "auto");

    return;
}


void spell_enchanted_sleep( int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA *victim = (CHAR_DATA *) vo;

    ROOM_INDEX_DATA *room;

   /* Only works on non-dreaming victims... */

    if ( victim->waking_room != NULL ) { 
      if ( victim != ch ) send_to_char("They are already dreaming!\r\n", ch);
      else send_to_char("You are already dreaming!\r\n", ch);
      return;
    }
 
   /* And not the badly injured... */
 
    if ( victim->position < POS_STUNNED ) {
      send_to_char("Dreams won't help them now...\r\n", ch);
      return;
    }

    if (IS_SET(victim->in_room->room_flags, ROOM_NO_DREAM)) {
          send_to_char( "{cYour dreams are disturbing and frightening.{x\r\n", victim );
          return;
    }

   /* Others get a save... */

    if ( victim != ch ) {
      if ( saves_spell(level, victim) ) {

        send_to_char("You feel a little sleepy!\r\n", victim);
        send_to_char("They resist your spell!\r\n", ch);

        return;
      } 
    }

   /* Now you must make a dreaming check... */

    if ( !check_skill(ch, gsn_dreaming, ch->level - victim->level) ) {

      stop_fighting(victim, TRUE);

      set_activity(victim, POS_RESTING, NULL, ACV_NONE, NULL);

      send_to_char("You feel very sleepy!\r\n", victim);
      send_to_char("Your dream manipulations fail!\r\n", ch);

      return;
    }

   /* Now, where should we go? */
 
    room = NULL; 

    if ( ch->in_room != NULL
      && ch->in_room->dream != 0 ) {
      room = get_room_index(ch->in_room->dream);
    }

    if ( room == NULL 
      && ch->in_room != NULL
      && ch->in_room->area != NULL
      && ch->in_room->area->dream != 0 ) {
      room = get_room_index(ch->in_room->area->dream);
    }

    if ( room == NULL 
      && ch->in_room != NULL
      && ch->in_room->area != NULL
      && zones[ch->in_room->area->zone].dream != 0 ) {
      room = get_room_index(zones[ch->in_room->area->zone].dream);
    }

    if ( room == NULL 
      && mud.dream != 0 ) {
      room = get_room_index(mud.dream);
    }
       
   /* Just put to sleep and send a dream if nowhere to go... */

    if ( room == NULL ) {

      stop_fighting(victim, TRUE);

      act( "$n goes to sleep...", 
              victim, NULL, NULL, TO_ROOM );

      send_to_char("You feel very, very tired...{x\r\n", victim);

      if ( victim->position == POS_STANDING
        || victim->position == POS_SITTING
        || victim->position == POS_RESTING ) {
        set_activity(victim, POS_SLEEPING, NULL, ACV_NONE, NULL);
      }
 
      if (victim->position == POS_SLEEPING) { 
        random_dream(victim);
      }

      return;
    } 

   /* Success, transform from dreaming to really here... */

    stop_fighting(victim, TRUE);
 
    act( "$n goes to sleep and fades away to the world of dreams...", 
            victim, NULL, NULL, TO_ROOM );

    send_to_char("You feel very, very tired...{x\r\n", victim);

    victim->waking_room = victim->in_room;    /* Allow awaken */

    add_tracks(ch, DIR_NONE, DIR_OTHER);

    char_from_room(victim);

    victim->in_zone = -1;

    char_to_room(victim, room);

    add_tracks(ch, DIR_OTHER, DIR_HERE);

    victim->position = POS_STANDING;

    victim->dreaming_room = NULL;

    send_to_char("{mWhen you awaken you are somewhere else!{x\r\n", victim);

    if ( victim != ch ) {
      send_to_char("Success! They are returned to thier dreams!\r\n", ch);
    }

    do_look(victim, "auto");

    return;
}



void spell_detect_hidden( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_HIDDEN) )
    {
        if (victim == ch)
          send_to_char("You are already as alert as you can be. \r\n",ch);
        else
          act("$N can already sense hidden lifeforms.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.type      = sn;
    af.afn	 = gafn_detect_hidden;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DETECT_HIDDEN;
    affect_to_char( victim, &af );
    send_to_char( "Your awareness improves.\r\n", victim );
    if ( ch != victim )
	send_to_char( "Ok.\r\n", ch );
    return;
}



void spell_detect_invis( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_INVIS) )
    {
        if (victim == ch)
          send_to_char("You can already see invisible.\r\n",ch);
        else
          act("$N can already see invisible things.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.type      = sn;
    af.afn 	 = gafn_detect_invis;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_INVIS;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\r\n", victim );
    if ( ch != victim )
	send_to_char( "Ok.\r\n", ch );
    return;
}



void spell_detect_magic( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_MAGIC) )
    {
        if (victim == ch)
          send_to_char("You can already sense magical auras.\r\n",ch);
        else
          act("$N can already detect magic.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.type      = sn;
    af.afn	 = gafn_detect_magic;
    af.level	 = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_MAGIC;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\r\n", victim );
    if ( ch != victim )
	send_to_char( "Ok.\r\n", ch );
    return;
}



void spell_detect_poison( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
    {
	if ( obj->value[3] != 0 )
	    send_to_char( "You smell poisonous fumes.\r\n", ch );
	else
	    send_to_char( "It looks delicious.\r\n", ch );
    }
    else
    {
	send_to_char( "It doesn't look poisoned.\r\n", ch );
    }

    return;
}



void spell_dispel_evil( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
  
    if ( !IS_NPC(ch) && IS_EVIL(ch) )
	victim = ch;
  
    if ( IS_GOOD(victim) )
    {
	act( "Dispel evil has no effect upon $N.", ch, NULL, victim, TO_ROOM );
	return;
    }

    if ( IS_NEUTRAL(victim) )
    {
	act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
	return;
    }

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_HOLY;
    spell_CDB->atk_blk_type = BLOCK_ABSORB;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
   /* Set up and adjust damage... */

    spell_CDB->damage = dice(level, 4);

    if ( saves_spell( level, spell_CDB->defender ) ) {
	spell_CDB->damage /= 2;
    } 

   /* Illuminating experience... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

    return;
}

void spell_dispel_good( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
 	
    if ( !IS_NPC(ch) && IS_GOOD(ch) )
	victim = ch;
 
    if ( IS_EVIL(victim) )
    {
	act( "Dispel good has no effect upon $N.", ch, NULL, victim, TO_ROOM );
	return;
    }

    if ( IS_NEUTRAL(victim) )
    {
	act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
	return;
    }

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_NEGATIVE;
    spell_CDB->atk_blk_type = BLOCK_ABSORB;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
   /* Set up and adjust damage... */

    spell_CDB->damage = dice(level, 4);

    if ( saves_spell( level, spell_CDB->defender ) ) {
	spell_CDB->damage /= 2;
    } 

   /* Illuminating experience... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

    return;
}


/* modified for enhanced use */

void spell_dispel_magic( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;

    if (saves_spell(level, victim))
    {
	send_to_char( "You feel a brief tingling sensation.\r\n",victim);
	send_to_char( "You failed.\r\n", ch);
	return;
    }

    /* begin running through the spells */ 

    if (check_dispel(level,victim, gafn_absorb_magic))
        found = TRUE;
       
    if (check_dispel(level,victim, gafn_regeneration))
        found = TRUE;
   		 
    if (check_dispel(level,victim, gafn_armor))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_bless))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_blindness))
    {
        found = TRUE;
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim, gafn_calm))
    {
        found = TRUE;
        act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim, gafn_sex_change))
    {
        found = TRUE;
        act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim, gafn_charm))
    {
        found = TRUE;
        act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim, gafn_chill_touch))
    {
        found = TRUE;
        act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim, gafn_curse))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_detect_evil))
        found = TRUE;
    
    if (check_dispel(level,victim, gafn_detect_good))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_detect_hidden))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_detect_invis))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_detect_hidden))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_detect_magic))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_faerie_fire))
    {
        act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
    
    if (check_dispel(level,victim, gafn_fear))
    {
        act("$n no longer looks so scared.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
	
 
    if (check_dispel(level,victim, gafn_fly))
    {
        act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim, gafn_frenzy)
    || check_dispel(level,victim, gafn_berserk))    {
        act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
        found = TRUE;
    }
 
    if (check_dispel(level,victim, gafn_giant_strength))
    {
        act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim, gafn_haste))    {
        act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim, gafn_slow))    {
        act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim, gafn_infravision))   found = TRUE;
     if (check_dispel(level,victim, gafn_farsight))   found = TRUE;

    if (check_dispel(level,victim, gafn_invisability))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim, gafn_mask_self))    {
        act("$n suddenly appears as $s mask vanishes.",victim,NULL,NULL,TO_ROOM);
        undo_mask(victim, FALSE);
        found = TRUE;
    }
    
    if (check_dispel(level,victim, gafn_mind_meld))
    {
        act("$n has regained $s senses.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim, gafn_mute))
        found = TRUE;
    
    if (check_dispel(level,victim, gafn_pass_door))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_protection_good))
        found = TRUE;
    
    if (check_dispel(level,victim, gafn_protection_evil))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_sanctuary))
    {
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (IS_AFFECTED(victim,AFF_SANCTUARY) 
	&& !saves_dispel(level, victim->level,-1)
	&& !is_affected(victim, gafn_sanctuary)) {
	REMOVE_BIT(victim->affected_by,AFF_SANCTUARY);
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim, gafn_shield))
    {
        act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim, gafn_sleep))
        found = TRUE;
 
    if (check_dispel(level,victim, gafn_hard_skin))    {
        act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim, gafn_globe))    {
        act("$n's globe of protection vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim, gafn_size))    {
        act("$n's size returns to normal.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim, gafn_vocalize))
        found = TRUE;
    
    if (check_dispel(level,victim, gafn_weakness))    {
        act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim, gafn_mist))    {
        act("$n looks solid again.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (found) send_to_char("Ok.\r\n",ch);
    else send_to_char("Spell failed.\r\n",ch);
}


void spell_earthquake( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    send_to_char( "The world shudders around you!\r\n", ch );
    act( "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )    {
	vch_next	= vch->next;

	if ( vch->in_room == NULL ) continue;

	if ( vch->in_room->area == ch->in_room->area) {

  	  if ( vch->in_room->subarea == ch->in_room->subarea
                  && vch != ch
                  && !is_safe_spell(ch, vch, TRUE) ) {

           /* Reset spell_CDB... */

            spell_CDB = getCDB(ch, vch, sn, FALSE, sn, 0);

           /* Set up combat data... */

            spell_CDB->atk_dam_type = DAM_BASH;
            spell_CDB->atk_blk_type = BLOCK_ABSORB;
            spell_CDB->atk_desc = "earthquake";

           /* Set up and adjust damage... */

            if ( ch->in_room == vch->in_room ) { 
              spell_CDB->damage = level + dice(4, 12);
            } else {						
              spell_CDB->damage = level/2 + dice(3, 6);
            }

            spell_CDB->damage += spell_roll;

            if ( vch == ch ) {
              spell_CDB->damage = (spell_CDB->damage * 3)/2; 
            } 

            if ( IS_AFFECTED(vch, AFF_FLYING) ) {
              spell_CDB->damage /= 2;
            }

            if ( saves_spell( level, spell_CDB->defender ) ) {
              spell_CDB->damage /= 2;
            }

           /* Charging time... */
   
 if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

          } else {

	    send_to_char( "The earth trembles and shivers.\r\n", vch );
          }
        }
    }

    return;
}

void spell_enchant_armor( int sn, int level, CHAR_DATA *ch, void *vo ) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf; 
    int result, fail;
    int ac_bonus, added;
    bool ac_found = FALSE;

    if (obj->item_type != ITEM_ARMOR)   {
	send_to_char("That isn't an armor.\r\n",ch);
	return;
    }

    if (obj->wear_loc != -1)   {
	send_to_char("The item must be carried to be enchanted.\r\n",ch);
	return;
    }

   if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)) {
      sprintf_to_char(ch, "%s is much too powerful!\r\n", capitalize(obj->short_descr));
      return;
   }

    /* this means they have no bonus */
    ac_bonus = 0;
    fail = 25;	/* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
    	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    	{
            if ( paf->location == APPLY_AC )
            {
	    	ac_bonus = paf->modifier;
		ac_found = TRUE;
	    	fail += 5 * (ac_bonus * ac_bonus);
 	    }

	    else  /* things get a little harder */
	    	fail += 20;
    	}
 
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_AC )
  	{
	    ac_bonus = paf->modifier;
	    ac_found = TRUE;
	    fail += 5 * (ac_bonus * ac_bonus);
	}

	else /* things get a little harder */
	    fail += 20;
    }

    /* apply other modifiers */
    fail -= level;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail -= 15;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail -= 5;

    fail = URANGE(5,fail,95);

    result = number_percent();

    /* the moment of truth */
    if (result < (fail / 5))  /* item destroyed */
    {
	act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_CHAR);
	act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
	return;
    }

    if (result < (fail / 2)) /* item disenchanted */
    {
	AFFECT_DATA *paf_next;

	act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
	act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next; 
	    paf->next = affect_free;
	    affect_free = paf;
	}
	obj->affected = NULL;

	/* clear all flags */
	obj->extra_flags = 0;
	return;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
	send_to_char("Nothing seemed to happen.\r\n",ch);
	return;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
	AFFECT_DATA *af_new;
	obj->enchanted = TRUE;

	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) {
	
            af_new = new_affect();
	
	    af_new->next = obj->affected;
	    obj->affected = af_new;

	    af_new->type 	= UMAX(0,paf->type);
            af_new->afn		= paf->afn;       
	    af_new->level	= paf->level;
	    af_new->duration	= paf->duration;
	    af_new->location	= paf->location;
	    af_new->skill	= paf->skill;
	    af_new->modifier	= paf->modifier;
	    af_new->bitvector	= paf->bitvector;
	}
    }

    if (result <= (100 - level/5))  /* success! */
    {
	act("$p shimmers with a gold aura.",ch,obj,NULL,TO_CHAR);
	act("$p shimmers with a gold aura.",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);
	added = -1;
    }
    
    else  /* exceptional enchant */
    {
	act("$p glows a brillant gold!",ch,obj,NULL,TO_CHAR);
	act("$p glows a brillant gold!",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags,ITEM_MAGIC);
	SET_BIT(obj->extra_flags,ITEM_GLOW);
	added = -2;
    }
		
    /* now add the enchantments */ 

    if (obj->level < MAX_LEVEL)
	obj->level = UMIN(MAX_LEVEL - 1,obj->level + 1);

    if (ac_found)
    {
	for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
	    if ( paf->location == APPLY_AC)
	    {
		paf->type = sn;
		paf->afn = gafn_enchant_armor;
		paf->modifier += added;
		paf->level = UMAX(paf->level,level);
	    }
	}
    }
    else {/* add a new affect */

        paf = new_affect();    

	paf->type	= sn;
        paf->afn	= gafn_enchant_armor;
	paf->level	= level;
	paf->duration	= -1;
	paf->location	= APPLY_AC;
        paf->skill      = SKILL_UNDEFINED;
	paf->modifier	=  added;
	paf->bitvector  = 0;
    	paf->next	= obj->affected;
    	obj->affected	= paf;
    }

}

void spell_brand (int sn, int level, CHAR_DATA *ch, void *vo) {
	OBJ_DATA *obj = (OBJ_DATA *) vo; 
    int roll;
	int all_brand=WEAPON_LIGHTNING+WEAPON_ACID+WEAPON_FLAMING+WEAPON_FROST;
	
	if (obj->item_type != ITEM_WEAPON)
		{
		send_to_char ("That isn't a weapon.\r\n",ch);
		return;
		}
    if (obj->wear_loc != -1)
    	{
		send_to_char("The item must be carried to be enchanted.\r\n",ch);
		return;
    	}
	if ((obj->value[4]&all_brand)!=0) /*Already branded*/
		{
		send_to_char("This weapon is already branded.\r\n",ch);
		return;
		}
	roll = number_percent();
	if (roll <= 5) /*oops, destroy weapon*/ 
		{	
		extract_obj(obj);
		send_to_char ("{mKaboom!{x  The weapon explodes!  *sigh*\r\n",ch);
		act("$p shivers violently and explodes!",ch,obj,NULL,TO_ROOM);
		return;
		}
	if (roll > ((get_curr_stat(ch,STAT_INT)/4)-18)*2 + 5 + ((int)(level/3))) /*spell failed*/
		{
		send_to_char ("Spell failed.\r\n",ch);
		return;
		}
	else /*spell worked!*/
		{
		int which=number_percent();
		int brand=0;
		char buf[80];
	
		if (which <= 25)
			{
			brand=WEAPON_FLAMING;
			strcpy (buf, "You have created a {rFLAME{x brand!\r\n");
			}
		else if (which <= 50)
			{
			brand=WEAPON_FROST;
			strcpy (buf, "You have created a {bFROST{x brand!\r\n");
			}
		else if (which <= 75)
			{
			brand=WEAPON_ACID;
			strcpy (buf, "You have created an {gACID{x brand!\r\n");
			}
		else 
			{
			brand=WEAPON_LIGHTNING;
			strcpy (buf, "You have created a {yLIGHTNING{x brand!\r\n");
			}
		
		/*set new brand on weapon*/
		obj->value[4]+=brand;
		obj->valueorig[4]+=brand;
		send_to_char(buf,ch);
		}
}	/*end*/

extern void create_potion(CHAR_DATA *ch, int s1, int s2, int s3);

void spell_create_potion( int sn, int level, CHAR_DATA *ch, void *vo) {

  create_potion(ch, 0, 0, 0);
 
  act("You create a potion!",ch,NULL,NULL,TO_CHAR);
  act("$n creates a potion!",ch,NULL,NULL,TO_ROOM);

  return;
}


void spell_enchant_weapon( int sn, int level, CHAR_DATA *ch, void *vo ) {
OBJ_DATA *obj = (OBJ_DATA *) vo;
AFFECT_DATA *paf; 
int result, fail;
int hit_bonus, dam_bonus, added;
bool hit_found = FALSE, dam_found = FALSE;

    if (obj->item_type != ITEM_WEAPON)    {
	send_to_char("That isn't a weapon.\r\n",ch);
	return;
    }

    if (obj->wear_loc != -1)    {
	send_to_char("The item must be carried to be enchanted.\r\n",ch);
	return;
    }

   if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)) {
      sprintf_to_char(ch, "%s is much too powerful!\r\n", capitalize(obj->short_descr));
      return;
   }

    /* this means they have no bonus */
    hit_bonus = 0;
    dam_bonus = 0;
    fail = 25;	/* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
    	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )   	{
            if ( paf->location == APPLY_HITROLL )            {
	    	hit_bonus = paf->modifier;
		hit_found = TRUE;
	    	fail += 2 * (hit_bonus * hit_bonus);

 	    }   else if (paf->location == APPLY_DAMROLL )    {
	    	dam_bonus = paf->modifier;
		dam_found = TRUE;
	    	fail += 2 * (dam_bonus * dam_bonus);
	    }    else  fail += 25;
    	}
 
    for ( paf = obj->affected; paf != NULL; paf = paf->next )    {
	if ( paf->location == APPLY_HITROLL )  	{
	    hit_bonus = paf->modifier;
	    hit_found = TRUE;
	    fail += 2 * (hit_bonus * hit_bonus);
	} else if (paf->location == APPLY_DAMROLL ) {
	    dam_bonus = paf->modifier;
	    dam_found = TRUE;
	    fail += 2 * (dam_bonus * dam_bonus);
	} else     fail += 25;
    }

    /* apply other modifiers */
    fail -= 3 * level/2;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail -= 15;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail -= 5;

    fail = URANGE(5,fail,95);

    result = number_percent();

    /* the moment of truth */
    if (result < (fail / 5))  /* item destroyed */
    {
	act("$p shivers violently and explodes!",ch,obj,NULL,TO_CHAR);
	act("$p shivers violently and explodes!",ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
	return;
    }

    if (result < (fail / 2)) /* item disenchanted */
    {
	AFFECT_DATA *paf_next;

	act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
	act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next; 
	    paf->next = affect_free;
	    affect_free = paf;
	}
	obj->affected = NULL;

	/* clear all flags */
	obj->extra_flags = 0;
	return;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
	send_to_char("Nothing seemed to happen.\r\n",ch);
	return;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
	AFFECT_DATA *af_new;
	obj->enchanted = TRUE;

	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) { 
	
            af_new = new_affect();
	
	    af_new->next = obj->affected;
	    obj->affected = af_new;

	    af_new->type 	= UMAX(0,paf->type);
            af_new->afn		= paf->afn;
	    af_new->level	= paf->level;
	    af_new->duration	= paf->duration;
	    af_new->location	= paf->location;
	    af_new->skill	= paf->skill;
	    af_new->modifier	= paf->modifier;
	    af_new->bitvector	= paf->bitvector;
	}
    }

    if (result <= (100 - level/5))  /* success! */
    {
	act("$p glows blue.",ch,obj,NULL,TO_CHAR);
	act("$p glows blue.",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);
	added = 1;
    }
    
    else  /* exceptional enchant */
    {
	act("$p glows a brillant blue!",ch,obj,NULL,TO_CHAR);
	act("$p glows a brillant blue!",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags,ITEM_MAGIC);
	SET_BIT(obj->extra_flags,ITEM_GLOW);
	added = 2;
    }
		
    /* now add the enchantments */ 

    if (obj->level < MAX_LEVEL - 1)
	obj->level = UMIN(MAX_LEVEL - 1,obj->level + 1);

    if (dam_found)
    {
	for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
	    if ( paf->location == APPLY_DAMROLL)
	    {
		paf->type = sn;
                paf->afn = gafn_enchant_weapon;
		paf->modifier += added;
		paf->level = UMAX(paf->level,level);
		if (paf->modifier > 4)
		    SET_BIT(obj->extra_flags,ITEM_HUM);
	    }
	}
    }
    else { /* add a new affect */
    
        paf = new_affect();

	paf->type	= sn;
	paf->afn	= gafn_enchant_weapon;
	paf->level	= level;
	paf->duration	= -1;
	paf->location	= APPLY_DAMROLL;
        paf->skill      = SKILL_UNDEFINED;
	paf->modifier	=  added;
	paf->bitvector  = 0;
    	paf->next	= obj->affected;
    	obj->affected	= paf;
    }

    if (hit_found)
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
            if ( paf->location == APPLY_HITROLL)
            {
		paf->type = sn;
		paf->afn = gafn_enchant_weapon;
                paf->modifier += added;
                paf->level = UMAX(paf->level,level);
                if (paf->modifier > 4)
                    SET_BIT(obj->extra_flags,ITEM_HUM);
            }
	}
    }
    else { /* add a new affect */
    
        paf = new_affect();
 
        paf->type       = sn;
	paf->afn	= gafn_enchant_weapon;
        paf->level      = level;
        paf->duration   = -1;
        paf->location   = APPLY_HITROLL;
        paf->skill      = SKILL_UNDEFINED;
        paf->modifier   =  added;
        paf->bitvector  = 0;
        paf->next       = obj->affected;
        obj->affected   = paf;
    }

}


/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
void spell_energy_drain( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( saves_spell( level, victim ) )
    {
	send_to_char("You feel a momentary chill.\r\n",victim);  	
	return;
    }

    ch->alignment = UMAX(-1000, ch->alignment - 50);
    if ( victim->level <= 2 )    {
	dam		 = ch->hit + 1;
    }    else    {
	gain_exp( victim, 0 -  5 * number_range( level/2, 3 * level / 2 ), FALSE);
	victim->mana	/= 2;
	victim->move	/= 2;
	dam		 = dice(1, level);
	ch->hit		= UMIN (ch->max_hit, (ch->hit+dam));

    }

    send_to_char("You feel your life slipping away!\r\n",victim);
    send_to_char("Wow....what a rush!\r\n",ch);

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_NEGATIVE;
    spell_CDB->atk_blk_type = BLOCK_NONE;

   /* Set up and adjust damage... */

    spell_CDB->damage = dam;

   /* Illuminating experience... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

    return;
}



void spell_fireball( int sn, int level, CHAR_DATA *ch, void *vo ) {
//    CHAR_DATA *victim = (CHAR_DATA *) vo;

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_FIRE;
    spell_CDB->atk_blk_type = BLOCK_BALL;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
    if (number_percent() < 15) harmful_effect(vo, level, TAR_CHAR_OFFENSIVE, DAM_FIRE);

   /* Set up and adjust damage... */

    spell_CDB->damage = 25 + dice((level/2), 7);

    if ( saves_spell( level, spell_CDB->defender ) ) {
	spell_CDB->damage /= 2;
    } 

   /* Illuminating experience... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

    return;
}

void spell_fire_shield (int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
	
    if (is_affected(victim, gafn_fire_shield)
    || is_affected(victim, gafn_frost_shield)
    || is_affected(victim, gafn_lightning_shield)) {
	send_to_char ("That target already has an elemental shield.\r\n",ch);
	return;
    }
	
    af.type      = sn;
    af.afn	       = gafn_fire_shield; 
    af.level       = level;
    af.duration  = level/3;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_FIRE_SHIELD;
    affect_to_char( victim, &af );
    send_to_char( "A small shield of living flame pops up before you.\r\n", victim );
    act( "$n is suddenly defended by a small shield of living flame.", victim, NULL, NULL, TO_ROOM );
    return;
}

void spell_frost_shield (int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
	
    if (is_affected(victim, gafn_fire_shield)
    || is_affected(victim, gafn_frost_shield)
    || is_affected(victim, gafn_lightning_shield)) {
	send_to_char ("That target already has an elemental shield.\r\n",ch);
	return;
    }
	
    af.type      = sn;
    af.afn	       = gafn_frost_shield; 
    af.level       = level;
    af.duration  = level/3;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_FROST_SHIELD;
    affect_to_char( victim, &af );
    send_to_char( "A small shield of frost pops up before you.\r\n", victim );
    act( "$n is suddenly defended by a small shield of frost.", victim, NULL, NULL, TO_ROOM );
    return;
}

void spell_lightning_shield (int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
	
    if (is_affected(victim, gafn_fire_shield)
    || is_affected(victim, gafn_frost_shield)
    || is_affected(victim, gafn_lightning_shield)) {
	send_to_char ("That target already has an elemental shield.\r\n",ch);
	return;
    }
	
    af.type      = sn;
    af.afn	       = gafn_lightning_shield; 
    af.level       = level;
    af.duration  = level/3;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_LIGHTNING_SHIELD;
    affect_to_char( victim, &af );
    send_to_char( "A small shield of electricity pops up before you.\r\n", victim );
    act( "$n is suddenly defended by a small shield of electricity.", victim, NULL, NULL, TO_ROOM );
    return;
}

void spell_flamestrike( int sn, int level, CHAR_DATA *ch, void *vo ) {
//    CHAR_DATA *victim = (CHAR_DATA *) vo;

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_FIRE;
    spell_CDB->atk_blk_type = BLOCK_BOLT;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
   /* Set up and adjust damage... */

    spell_CDB->damage = 5 + dice((level/4), 8);

    if ( saves_spell( level, spell_CDB->defender ) ) {
	spell_CDB->damage /= 2;
    } 

   /* Roasting time... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

    return;
}



void spell_faerie_fire( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) )
	return;
    af.type      = sn;
    af.afn	 = gafn_faerie_fire;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = 2 * level;
    af.bitvector = AFF_FAERIE_FIRE;
    affect_to_char( victim, &af );
    send_to_char( "You are surrounded by a pink outline.\r\n", victim );
    act( "$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM );
    return;
}



void spell_faerie_fog( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *ich;

    act( "$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You conjure a cloud of purple smoke.\r\n", ch );

    for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room )
    {
	if ( !IS_NPC(ich) && IS_SET(ich->plr, PLR_WIZINVIS) )
	    continue;

	if ( ich == ch || saves_spell( level, ich ) )
	    continue;

	affect_strip_afn ( ich, gafn_invisability	);
	affect_strip_afn ( ich, gafn_sneak		);
	REMOVE_BIT   ( ich->affected_by, AFF_HIDE	);
	REMOVE_BIT   ( ich->affected_by, AFF_INVISIBLE	);
	REMOVE_BIT   ( ich->affected_by, AFF_SNEAK	);
	act( "$n is revealed!", ich, NULL, NULL, TO_ROOM );
	send_to_char( "You are revealed!\r\n", ich );
    }

    return;
}

void spell_fear (int sn, int level, CHAR_DATA *ch, void *vo)	{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (victim->fighting == NULL)	{
		send_to_char ("Fear can only be cast upon a fighting creature.\r\n",ch);
		return;
		}
	
	if (IS_AFFECTED(victim, AFF_FEAR))		{
		send_to_char ("That target is already scared sh*tless...\r\n",ch);
		return;
		}
	
                if (IS_SET(victim->act,ACT_UNDEAD)
                || IS_SET(victim->act, ACT_BRAINSUCKED)) {
		send_to_char ("Yout target doesn't know fear!\r\n",ch);
		return;
		}

	if (!saves_spell(level, victim))	{
		if (!IS_NPC(victim)) send_to_char ("You feel very vulnerable and scared!\r\n",victim); 
		do_say(victim, "Please, don't hurt me!");

   		af.type      = sn;
		af.afn	     = gafn_fear;              
   		af.level     = level;
   		af.duration  = level / 6;
   		af.location  = 0;
   		af.modifier  = 0;
   		af.bitvector = AFF_FEAR;
   		affect_to_char( victim, &af );
		do_flee(victim, "");
		stop_fighting(victim, TRUE);
		return;
		}	else		{
		if (!IS_NPC(victim))
		   send_to_char ("For just a moment, an intense feeling of fear washes over you.\r\n",victim);
		   send_to_char ("Spell failed.\r\n",ch);
		   return;
		}
	}		
	
void spell_fly( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FLYING) )
    {
	if (victim == ch)
	  send_to_char("You are already airborne.\r\n",ch);
	else
	  act("$N doesn't need your help to fly.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.type      = sn;
    af.afn 	 = gafn_fly;
    af.level	 = level;
    af.duration  = level + 3;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( victim, &af );
    send_to_char( "Your feet rise off the ground.\r\n", victim );
    act( "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );
    return;
}

/* RT clerical berserking spell */

void spell_frenzy(int sn, int level, CHAR_DATA *ch, void *vo) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected(victim, gafn_frenzy) 
      || IS_AFFECTED(victim, AFF_BERSERK) )
    {
	if (victim == ch)
	  send_to_char("You are already in a frenzy.\r\n",ch);
	else
	  act("$N is already in a frenzy.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if ( is_affected(victim, gafn_calm) )
    {
	if (victim == ch)
	  send_to_char("Why don't you just relax for a while?\r\n",ch);
	else
	  act("$N doesn't look like $e wants to fight anymore.",
	      ch,NULL,victim,TO_CHAR);
	return;
    }

    if ((IS_GOOD(ch) && !IS_GOOD(victim)) ||
	(IS_NEUTRAL(ch) && !IS_NEUTRAL(victim)) ||
	(IS_EVIL(ch) && !IS_EVIL(victim))
       )
    {
	act("Your god doesn't seem to like $N",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.type 	 = sn;
    af.afn	 = gafn_frenzy;
    af.level	 = level;
    af.duration	 = level / 3;
    af.modifier  = level / 6;
    af.bitvector = 0;

    af.location  = APPLY_HITROLL;
    affect_to_char(victim,&af);

    af.location  = APPLY_DAMROLL;
    affect_to_char(victim,&af);

    af.modifier  = 10 * (level / 6);
    af.location  = APPLY_AC;
    affect_to_char(victim,&af);

    send_to_char("You are filled with holy wrath!\r\n",victim);
    act("$n gets a wild look in $s eyes!",victim,NULL,NULL,TO_ROOM);
}

    
void spell_gate( int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA *victim;
bool gate_pet;


    if (IS_SET(ch->comm, COMM_NOPORT)) {
        send_to_char("The powers of order deny this dimensional travel!\r\n", ch);
        return; 
    }

   /* Only work from somewhere real... */

    if ( ch->in_room == NULL ) {
      send_to_char("Your gate shatters and disperses!\r\n", ch);
      return;
    }

   /* Find the victim... */

    victim = get_char_world( ch, target_name );

    if ( victim == NULL ) {
        send_to_char( "Your magic cannot find them!\r\n", ch );
        return;
    }

   /* Check for sillyness... */

    if ( victim == ch ) {
        send_to_char( "Look in a mirror!\r\n", ch );
        return;
    }

    if ( victim->in_room == ch->in_room ) {
      send_to_char("You gate implodes!\r\n", ch);
      return;
    }

    if (room_is_private(victim, victim->in_room )) {
         send_to_char( "That room is private right now.\r\n", ch );
         return;
    }

    if (time_info.moon == MOON_NEW
    && !knows_discipline(ch, DISCIPLINE_LUNAR)) {
       send_to_char("The power of the moon disturbs your magical flux!\r\n", ch);
       return; 
    }

   /* Check for range... */ 

    if (  victim->in_room == NULL
      ||  victim->in_room->area == NULL
      || !can_see_room(ch, victim->in_room) ) {
        send_to_char( "They are beyond your sight!\r\n", ch );
        return;
    }

    if (  ch->in_room->area != NULL
      &&  ch->in_room->area->zone != ch->in_room->area->zone ) {
        send_to_char( "They are beyond your reach!\r\n", ch );
        return;
    }

     if (IS_SET(victim->act,ACT_CRIMINAL)) {
        send_to_char( "They seem to be hiding!\r\n", ch );
        return;
    }

    if (victim->pIndexData && !IS_NPC(ch)) {
       if (ch->pcdata->questmob == victim->pIndexData->vnum) {
          send_to_char( "They seem to be hiding!\r\n", ch );
          return;
       }
    }         
 
    if (IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
      || IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
      || IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
      || IS_SET(victim->in_room->area->area_flags, AREA_LOCKED) ) {
        send_to_char( "A strange energy disrupts your spell!\r\n", ch );
        return;
    }

   /* Finally give them a save... */

    if ( saves_spell( level, victim ) ) {
        send_to_char( "Your spell fails!\r\n", ch );
        return;
    }
	
   /* Hmmm. Spell seems to allow free exit from combat... */

    if (victim->fighting != NULL) {
	stop_fighting(victim, TRUE);
    } 

   /* Do the move... */    

    if ( ch->pet != NULL 
      && ch->in_room == ch->pet->in_room) {
	gate_pet = TRUE;
    } else {
	gate_pet = FALSE;
    }
    
    act("$n steps through a gate and vanishes.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You step through a gate and vanish.\r\n",ch);

    add_tracks(ch, DIR_NONE, DIR_OTHER);

    char_from_room(ch);
    char_to_room(ch,victim->in_room);

    add_tracks(ch, DIR_OTHER, DIR_HERE);

    act("$n has arrived through a gate.",ch,NULL,NULL,TO_ROOM);
    do_look(ch,"auto");

    if (gate_pet) {

	act("$n steps through a gate and vanishes.",ch->pet,NULL,NULL,TO_ROOM);
	send_to_char("You step through a gate and vanish.\r\n",ch->pet);

        add_tracks(ch->pet, DIR_NONE, DIR_OTHER);

	char_from_room(ch->pet);
	char_to_room(ch->pet,victim->in_room);

        add_tracks(ch->pet, DIR_OTHER, DIR_HERE);

	act("$n has arrived through a gate.",ch->pet,NULL,NULL,TO_ROOM);
	do_look(ch->pet,"auto");
    }
}



void spell_giant_strength( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, gafn_giant_strength) )
    {
	if (victim == ch)
	  send_to_char("You are stronger then usual!\r\n",ch);
	else
	  act("$N can't get any stronger.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.type      = sn;
    af.afn 	 = gafn_giant_strength;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "Your muscles surge with heightened power!\r\n", victim );
    act("$n's muscles surge with heightened power.",victim,NULL,NULL,TO_ROOM);
    return;
}



void spell_harm( int sn, int level, CHAR_DATA *ch, void *vo ) {
//    CHAR_DATA *victim = (CHAR_DATA *) vo;

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_HARM;
    spell_CDB->atk_blk_type = BLOCK_ABSORB;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
   /* Set up and adjust damage... */

    if (spell_CDB->defender->level < 15) {
         spell_CDB->damage = dice(5,10);
    } else {
         spell_CDB->damage = spell_CDB->defender->hit - dice(5,9);
    } 

    if ( saves_spell( level, spell_CDB->defender ) ) {
	spell_CDB->damage /= 2;
    } 

   /* Roasting time... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
 } else {
    apply_damage( spell_CDB);
 }
 return;
}


void spell_fatigue( int sn, int level, CHAR_DATA *ch, void *vo ) {

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_HARM;
    spell_CDB->atk_blk_type = BLOCK_ABSORB;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
   /* Set up and adjust damage... */

    if (spell_CDB->defender->level < 15) {
         spell_CDB->damage = dice(5,10);
    } else {
         spell_CDB->damage = spell_CDB->defender->move - dice(5,7);
    } 

    if ( saves_spell( level, spell_CDB->defender ) ) {
	spell_CDB->damage /= 2;
    } 

   /* Roasting time... */


if  (IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
     apply_damage( spell_CDB);  
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
 } else {
    apply_damage( spell_CDB);
 }
 return;
}



void spell_haste( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, gafn_haste ) 
    || IS_AFFECTED(victim, AFF_HASTE)
    ||   IS_SET(victim->off_flags,OFF_FAST))    {
	if (victim == ch)
	  send_to_char("You can't move any faster!\r\n",ch);
 	else
	  act("$N is already moving as fast as $e can.",
	      ch,NULL,victim,TO_CHAR);
        return;
    }

    if (IS_AFFECTED(victim, AFF_SLOW)) {
           if (check_dispel(level,victim, gafn_slow)) act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
    } else {
        af.type      = sn;
        af.afn 	 = gafn_haste;
        af.level     = level;
        if (victim == ch)  af.duration  = level/2;
        else af.duration  = level/4;
        af.location  = APPLY_DEX;
        af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32) + (level >= 50) + (level >= 75) + (level >= 100) + (level >= 125);
        af.bitvector = AFF_HASTE;
        affect_to_char( victim, &af );
    }
    send_to_char( "You feel yourself moving more quickly.\r\n", victim );
    act("$n is moving more quickly.",victim,NULL,NULL,TO_ROOM);
    if ( ch != victim )
        send_to_char( "Ok.\r\n", ch );
    return;
}

void spell_heal( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int tolimb;

    victim->hit = UMIN( victim->hit + 100, victim->max_hit );
    update_pos( victim );
    REMOVE_BIT(victim->form,FORM_BLEEDING);
           tolimb=number_range(0,5);
           if (ch->limb[tolimb]>0 && ch->limb[tolimb]<10) {
                   if (number_range(0,ch->limb[tolimb])/3 < 2) ch->limb[tolimb] -=2;
                   if (ch->limb[tolimb]<0) ch->limb[tolimb] = 0;
           }
           update_wounds(ch);
    send_to_char( "A warm feeling fills your body.\r\n", victim );
    if ( ch != victim )
	send_to_char( "Ok.\r\n", ch );
    return;
}


void spell_holy_word(int sn, int level, CHAR_DATA *ch, void *vo) {
CHAR_DATA *vch;
CHAR_DATA *vch_next;
int bless_num, curse_num, frenzy_num;
   
    bless_num = get_skill_sn("bless");
    curse_num = get_skill_sn("curse"); 
    frenzy_num = get_skill_sn("frenzy");

    act("$n utters a word of divine power!",ch,NULL,NULL,TO_ROOM);
    send_to_char("You utter a word of divine power.\r\n",ch);

    if (ch->in_room == NULL) {
      send_to_char("Its echos fade in the void...\r\n", ch);
      return;
    }

    if (IS_SET(ch->act, ACT_UNDEAD)) {
      send_to_char("You don't dare...\r\n", ch);
      return;
    }
 
    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )   {
        vch_next = vch->next_in_room;

	if (ch == vch || (IS_GOOD(ch) && IS_GOOD(vch)) || (IS_EVIL(ch) && IS_EVIL(vch)) || (IS_NEUTRAL(ch) && IS_NEUTRAL(vch))) {
 	  send_to_char("You feel more powerful.\r\n",vch);
	  spell_frenzy(frenzy_num,level,ch,(void *) vch); 
	  spell_bless(bless_num,level,ch,(void *) vch);
	} else if ((IS_GOOD(ch) && IS_EVIL(vch)) ||  (IS_EVIL(ch) && IS_GOOD(vch)) ) {
	  if (!is_safe_spell(ch,vch,TRUE))  {
                    spell_curse(curse_num,level,ch,(void *) vch);
	    send_to_char("You are struck down!\r\n",vch);
                    vch->hit /= 2;
                    vch->mana /= 2;
                  }

	} else if (IS_NEUTRAL(ch)) {
	  if (!is_safe_spell(ch,vch,TRUE)) {
                       spell_curse(curse_num,level/2,ch,(void *) vch);
	       send_to_char("You are struck down!\r\n",vch);
                       vch->hit = vch->hit *2 /3;
                       vch->mana = vch->mana *2 /3;
   	  }
	}
    }  
    
    if (!IS_NPC(ch)) {
        send_to_char("You feel drained.\r\n",ch);
        gain_exp( ch, -1 * number_range(1,10) * 10, FALSE);
    }
    ch->move = 0;
    ch->hit /= 2;
}
 

void spell_identify( int sn, int level, CHAR_DATA *ch, void *vo ) {
OBJ_DATA *obj = (OBJ_DATA *) vo;
char buf[MAX_STRING_LENGTH];
AFFECT_DATA *paf;
int i, vnum;
ROOM_INDEX_DATA *room;

    sprintf( buf,
	"Object '%s' is type: [%s]\r\n"
        "extra flags: [%s]\r\n"
        "special flags: [%s]\r\n"
        "material: [%s]\r\n"
        "Weight is %d, value is %d, level is %d.\r\n",
	obj->name,
	item_type_name( obj ),
	extra_bit_name( obj->extra_flags ),
	weapon_bit_name( obj->value[4] ),
	material_name(obj->material),
	obj->weight,
	obj->cost,
	obj->level
	);
    send_to_char( buf, ch );

    sprintf( buf, "Zones: [%s]\r\n", flag_string( zmask_flags, obj->zmask));  
    send_to_char( buf, ch);
	
    switch ( obj->item_type )    {
    case ITEM_SCROLL: 
    case ITEM_POTION:
    case ITEM_PILL:
	sprintf_to_char( ch, "Level %ld spells of:", obj->value[0] );

	if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )	{
	    send_to_char( " '", ch );
	    send_to_char( effect_array[obj->value[1]].name, ch );
	    send_to_char( "'", ch );
	}

	if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )	{
	    send_to_char( " '", ch );
	    send_to_char( effect_array[obj->value[2]].name, ch );
	    send_to_char( "'", ch );
	}

	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )	{
	    send_to_char( " '", ch );
	    send_to_char( effect_array[obj->value[3]].name, ch );
	    send_to_char( "'", ch );
	}

	send_to_char( ".\r\n", ch );
	break;

    case ITEM_FOOD:
	if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )	{
	    send_to_char( " '", ch );
	    send_to_char( effect_array[obj->value[2]].name, ch );
	    send_to_char( "'", ch );
	}
                 break;


    case ITEM_HERB:
	sprintf_to_char( ch, "Level %ld spells of:", obj->value[0] );

	if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )	{
	    send_to_char( " '", ch );
	    send_to_char( effect_array[obj->value[1]].name, ch );
	    send_to_char( "'", ch );
	}
	send_to_char( ".\r\n", ch );
	break;

    case ITEM_PIPE:
	sprintf_to_char(ch, "Bonus %ld spells of:", obj->value[0] );
	break;

    case ITEM_INSTRUMENT:
               sprintf_to_char(ch, "Type: %s.\r\n", flag_string(instrument_type, obj->value[0]));
               if (obj->value[1] > 0) sprintf_to_char(ch, "Power: %d.\r\n", obj->value[1]);
               break;

    case ITEM_WAND: 
    case ITEM_STAFF: 
	sprintf_to_char(ch, "Has %ld(%ld) charges of level %ld",
                                                       obj->value[1], obj->value[2], obj->value[0] );
      
	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL ) {
	    send_to_char( " '", ch );
	    send_to_char( effect_array[obj->value[3]].name, ch );
	    send_to_char( "'", ch );
	}
	send_to_char( ".\r\n", ch );
	break;
      
    case ITEM_WEAPON:
 	send_to_char("Weapon type is ",ch);
	switch (obj->value[0]) {
	  case(WEAPON_EXOTIC) : send_to_char("exotic.\r\n",ch);		break;
	  case(WEAPON_SWORD)  : send_to_char("sword.\r\n",ch);		break;	
	  case(WEAPON_DAGGER) : send_to_char("dagger.\r\n",ch);		break;
	  case(WEAPON_SPEAR)  : send_to_char("spear/staff.\r\n",ch);	break;
	  case(WEAPON_MACE)   : send_to_char("mace/club.\r\n",ch);	break;
	  case(WEAPON_AXE)    : send_to_char("axe.\r\n",ch);		break;
	  case(WEAPON_FLAIL)  : send_to_char("flail.\r\n",ch);		break;
	  case(WEAPON_WHIP)   : send_to_char("whip.\r\n",ch);		break;
	  case(WEAPON_POLEARM): send_to_char("polearm.\r\n",ch);	break;
	  case(WEAPON_GUN)    : send_to_char("gun.\r\n",ch);		break;
	  case(WEAPON_HANDGUN): send_to_char("handgun.\r\n",ch);	break;
	  case(WEAPON_SMG)    : send_to_char("submachinegun.\r\n",ch);	break;
	  case(WEAPON_BOW)    : send_to_char("bow.\r\n",ch);		break;
	  case(WEAPON_STAFF)  : send_to_char("staff.\r\n",ch);		break;
	  default	      : send_to_char("unknown.\r\n",ch);	break;
 	}
	if (obj->pIndexData->new_format)
	    sprintf_to_char(ch,"Damage is %ldd%ld (average %ld).\r\n",
		obj->value[1],obj->value[2],
		(1 + obj->value[2]) * obj->value[1] / 2);
	else
	    sprintf_to_char( ch, "Damage is %ld to %ld (average %ld).\r\n",
	    	obj->value[1], obj->value[2],
	    	( obj->value[1] + obj->value[2] ) / 2 );
               sprintf_to_char( ch, "Special type:   %s\r\n",
                                    flag_string( weapon_type,  obj->value[4] ) );
	break;

    case ITEM_EXPLOSIVE:
    case ITEM_TIMEBOMB:
	if (obj->pIndexData->new_format)  sprintf_to_char(ch,"Damage is %ldd%ld (average %ld).\r\n", obj->value[1],obj->value[2], (1 + obj->value[2]) * obj->value[1] / 2);
	else sprintf_to_char( ch, "Damage is %ld to %ld (average %ld).\r\n", obj->value[1], obj->value[2], ( obj->value[1] + obj->value[2] ) / 2 );
                sprintf_to_char( ch, "Timer:   %ld - %ld\r\n", obj->value[3], obj->value[4] );
 	break;

    case ITEM_AMMO:
               sprintf_to_char( ch, "Shots: %ld \r\n", obj->value[0]);
               sprintf_to_char( ch, "Special type:   %s\r\n",  flag_string( weapon_type,  obj->value[4] ) );
               break;

    case ITEM_TREE:
               sprintf_to_char(ch, "Type: %s\r\n", flag_string(tree_type, obj->value[1]));
               break;			

    case ITEM_ARMOR:
	sprintf_to_char( ch, "Armor class is %ld pierce, %ld bash, %ld slash, and %ld vs. magic.\r\n", obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	break;

    case ITEM_RAW:
        sprintf_to_char( ch, "Combat Mod: %ld\r\n"
                             "Weight Mod: %ld\r\n",
                              obj->value[0],
                              obj->value[1]);
        break;

    case ITEM_BONDAGE:
        sprintf_to_char( ch, "Strength: %ld\r\n",obj->value[0]);
        break;

    case ITEM_DOLL:
        sprintf_to_char( ch, "Power: %ld\r\n",obj->value[0]);
        break;

    case ITEM_BOOK:
        sprintf( buf, "Language: %s\r\n"
                      "Rating  : %ld\r\n"
                      "Skill 1 : %s\r\n"
                      "Skill 2 : %s\r\n"
                      "Skill 3 : %s\r\n",
                       skill_array[obj->value[0]].name,
                       obj->value[1],
                       skill_array[obj->value[2]].name, 
                       skill_array[obj->value[3]].name, 
                       skill_array[obj->value[4]].name);
        send_to_char(buf, ch);
        break;

    case ITEM_IDOL:
      for (i = 0; i < 5; i++) {
        vnum = obj->value[i];
        if (vnum != 0) {
          room = get_room_index(vnum);
          if (room != NULL) sprintf_to_char(ch, "Sends to room %ld: '%s'\r\n", vnum, room->name);
        }
      }  
    }

    if (!obj->enchanted) {
        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )    {
              identify_affect(ch, paf);
        }
    }

    for (paf = obj->affected; paf != NULL; paf = paf->next ) {
          identify_affect(ch, paf);
    }
    return;
}


void identify_affect(CHAR_DATA *ch, AFFECT_DATA *paf) {

        if (paf->bitvector) {
           sprintf_to_char(ch, "Affects %s\r\n", flag_string(affect_flags, paf->bitvector));
        } else if ( paf->location != APPLY_NONE && paf->modifier != 0 )	{
           switch (paf->location) {
               default:
                   sprintf_to_char(ch, "Affects %s by %d.\r\n", affect_loc_name( paf->location ), paf->modifier );
                   break;
               case APPLY_IMMUNITY:
               case APPLY_RESISTANCE:
               case APPLY_ENV_IMMUNITY:
                   sprintf_to_char(ch, "Affects %s ", affect_loc_name( paf->location));
                   sprintf_to_char(ch, "%s.\r\n", flag_string(immapp_flags, paf->modifier));
                   break;
               case APPLY_SKILL:
                   sprintf_to_char(ch, "Affects Skill %s by %d.\r\n", skill_array[paf->skill].name, paf->modifier );
                   break;
               case APPLY_EFFECT:
                   sprintf_to_char(ch, "Affects Effect %s\r\n", effect_array[paf->skill].name);
                   break;
            }
        }
        return;
}


void spell_universality( int sn, int level, CHAR_DATA *ch, void *vo ) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    int chance;

   /* Check for sillyness... */

    if ( obj->zmask == 0 ) {
      sprintf_to_char(ch, "Your %s is already universal!\r\n", capitalize(obj->short_descr));
      return;
    }

   if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)) {
      sprintf_to_char(ch, "%s is much too powerful!\r\n", capitalize(obj->short_descr));
      return;
   }

   /* See if the object survives... */

    chance = level - obj->level + 32;     /* 25% if same level */  

    if (number_bits(8) > chance) {
      sprintf(buf, "Your %s evaporates!\r\n", obj->short_descr);
      send_to_char(buf, ch);
      obj_from_char(obj);
      extract_obj(obj);
      return;
    }  

   /* Success! */ 

    obj->zmask = 0;

    sprintf(buf, "Your %s seems more solid now.\r\n", 
                  obj->short_descr);
    send_to_char(buf, ch);

    return;
}


void spell_permanence( int sn, int level, CHAR_DATA *ch, void *vo ) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    int chance;

  if (!IS_SET(obj->extra_flags, ITEM_MAGIC)) {
      sprintf_to_char(ch, "%s is much too ordinary!\r\n", capitalize(obj->short_descr));
      return;
   }

   if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)) {
      sprintf_to_char(ch, "%s is much too powerful!\r\n", capitalize(obj->short_descr));
      return;
   }
 
   /* See if the object survives... */

    chance = level - obj->level + 32;

    if (number_bits(8) > chance) {
      sprintf(buf, "Your %s turns to dust!\r\n", obj->short_descr);
      send_to_char(buf, ch);
      obj_from_char(obj);
      extract_obj(obj);
      return;
    }  

   /* Success! */ 

   SET_BIT(obj->extra_flags, ITEM_NO_COND);
   sprintf(buf, "Your %s seems almost indestructible now.\r\n", obj->short_descr);
   send_to_char(buf, ch);
   return;
}


void spell_consistence( int sn, int level, CHAR_DATA *ch, void *vo ) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    int chance;

  if (!IS_SET(obj->extra_flags, ITEM_ROT_DEATH)) {
      sprintf_to_char(ch, "%s is already consistent!\r\n", capitalize(obj->short_descr));
      return;
   }

   if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)) {
      sprintf_to_char(ch, "%s is much too powerful!\r\n", capitalize(obj->short_descr));
      return;
   }

   /* See if the object survives... */

    chance = level - obj->level + 32;     /* 25% if same level */  

    if (number_bits(8) > chance) {
      sprintf(buf, "Your %s turns to slime!\r\n", obj->short_descr);
      send_to_char(buf, ch);
      obj_from_char(obj);
      extract_obj(obj);
      return;
    }  

   /* Success! */ 

   REMOVE_BIT(obj->extra_flags, ITEM_ROT_DEATH);
   sprintf(buf, "Your %s now seems much more consistent.\r\n", obj->short_descr);
   send_to_char(buf, ch);
   return;
}


void spell_infravision( int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA *victim = (CHAR_DATA *) vo;
AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_INFRARED))   {
	if (victim == ch) send_to_char("You can already see in the dark.\r\n",ch);
	else act("$N already has infravision.\r\n",ch,NULL,victim,TO_CHAR);
	return;
    }
    act( "$n's eyes glow red.\r\n", ch, NULL, NULL, TO_ROOM );

    af.type      = sn;
    af.afn	 = gafn_infravision;
    af.level	 = level;
    af.duration  = 2 * level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INFRARED;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes glow red.\r\n", victim );
    return;
}



void spell_invis( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_INVISIBLE) )
	return;

    act( "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );
    af.type      = sn;
    af.afn	 = gafn_invisability;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INVISIBLE;
    affect_to_char( victim, &af );
    send_to_char( "You fade out of existence.\r\n", victim );
    return;
}



void spell_know_alignment( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char *msg;
    int ap;

    ap = victim->alignment;

         if ( ap >  700 ) msg = "$N has a pure and good aura.";
    else if ( ap >  350 ) msg = "$N is of excellent moral character.";
    else if ( ap >  100 ) msg = "$N is often kind and thoughtful.";
    else if ( ap > -100 ) msg = "$N doesn't have a firm moral commitment.";
    else if ( ap > -350 ) msg = "$N lies to $S friends.";
    else if ( ap > -700 ) msg = "$N is a black-hearted murderer.";
    else msg = "$N is the embodiment of pure evil!.";

    act( msg, ch, NULL, victim, TO_CHAR );
    return;
}



void spell_lightning_bolt( int sn, int level, CHAR_DATA *ch, void *vo )
{
//    CHAR_DATA *victim = (CHAR_DATA *) vo;

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_LIGHTNING;
    spell_CDB->atk_blk_type = BLOCK_LIGHTNING;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
   /* Set up and adjust damage... */

    spell_CDB->damage = 20 + dice(level/2, 5);

    if ( saves_spell( level, spell_CDB->defender ) ) {
	spell_CDB->damage /= 2;
    } 

   /* Roasting time... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

    return;
}


void spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo ) {
char buf[MAX_INPUT_LENGTH];
char buffer[4*MAX_STRING_LENGTH];
OBJ_DATA *obj;
OBJ_DATA *in_obj;
bool found;
int number = 0, max_found;

    found = FALSE;
    number = 0;
    buffer[0] = '\0';
    max_found = IS_IMMORTAL(ch) ? 200 : 2 * level;
 
    if (time_info.moon == MOON_NEW
    && !knows_discipline(ch, DISCIPLINE_LUNAR)) {
       send_to_char("The power of the moon disturbs your magical flux!\r\n", ch);
       return; 
    }

    if (target_name[0] == '\0') {
       send_to_char("What do you want to locate?\r\n", ch);
       return; 
    }

    for (obj = object_list; obj; obj = obj->next )    {
	if ( !can_see_obj(ch, obj)
                || !is_name(target_name, obj->name) 
    	|| ( !IS_IMMORTAL(ch) 
                    && number_percent() > 2 * level )
                || ( obj->pIndexData->vnum > 20000 
                    && obj->pIndexData->vnum < 20100 )
	||   level < obj->level) {
	    continue;
                }

                if (IS_SET(obj->extra_flags, ITEM_NOLOCATE)
                || IS_SET(obj->extra_flags, ITEM_ARTIFACT)) {
                    send_to_char("You see a vague undefined shape floating is a hellish mist!\r\n", ch);
	    continue;
                }

                found = TRUE;
                number++;
                if (number > 29) break;

                for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj );

	if ( in_obj->carried_by != NULL && can_see_obj( ch, in_obj ) && !IS_IMMORTAL(in_obj->carried_by) )	{
	    sprintf( buf, "%s carried by %s\r\n", obj->short_descr, PERS(in_obj->carried_by, ch) );
	} else {
	    if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
		sprintf( buf, "[{Y%s{x] in [{Y%s{x] Room [{Y%ld{x] Level [{Y%d{x]\r\n", obj->short_descr,  in_obj->in_room->name, in_obj->in_room->vnum, obj->level);
	    else
	    	sprintf( buf, "%s in %s\r\n", obj->short_descr, in_obj->in_room == NULL ? "somewhere" : in_obj->in_room->name );
	}

	buf[0] = UPPER(buf[0]);
	strcat(buffer,buf);

    	if (number >= max_found)  break;
    }

    if ( !found ) send_to_char( "Nothing like that in heaven or earth.\r\n", ch );
    else if (ch->lines) page_to_char(buffer, ch);
    else send_to_char(buffer,ch);

    return;
}



void spell_magic_missile( int sn, int level, CHAR_DATA *ch, void *vo ) {

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_ENERGY;
    spell_CDB->atk_blk_type = BLOCK_ABSORB;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
   /* Set up and adjust damage... */

    spell_CDB->damage = dice(1 + level/3, 5) + 5;

    if ( saves_spell( level, spell_CDB->defender ) ) {
	spell_CDB->damage /= 2;
    } 

   /* Roasting time... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

    return;
}

void spell_mass_healing(int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *gch;
    int heal_num, refresh_num;
    
    heal_num = get_skill_sn("heal");
    refresh_num = get_skill_sn("refresh"); 

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ((IS_NPC(ch) && IS_NPC(gch)) ||
	    (!IS_NPC(ch) && !IS_NPC(gch)))
	{
	    spell_heal(heal_num,level,ch,(void *) gch);
	    spell_refresh(refresh_num,level,ch,(void *) gch);  
	}
    }
}
	    
void spell_mass_invis( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;
    CHAR_DATA *gch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( !is_same_group( gch, ch ) || IS_AFFECTED(gch, AFF_INVISIBLE) )
	    continue;
	act( "$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM );
	send_to_char( "You slowly fade out of existence.\r\n", gch );
	af.type      = sn;
        af.afn       = gafn_invisability;	 	
    	af.level     = level/2;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_INVISIBLE;
	affect_to_char( gch, &af );
    }
    send_to_char( "Ok.\r\n", ch );

    return;
}

void spell_mute (int sn, int level, CHAR_DATA *ch, void *vo) {
	CHAR_DATA *victim=(CHAR_DATA *)vo;
	AFFECT_DATA af;

	if (is_affected(victim, gafn_mute))	{
		send_to_char ("That character is already mute.\r\n",ch);
		return;
		}
	
	if (saves_spell(level, victim))	{
		send_to_char ("Spell failed.\r\n",ch);
		return;
		}

	af.type      = sn;
                af.afn	     = gafn_mute; 
   	af.level     = level;
	af.duration  = level/7;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = 0;
	affect_to_char( victim, &af );
	send_to_char ("Mute spell successful.\r\n",ch);
	send_to_char ("You have been muted!\r\n",victim);
	}
	

void spell_negate_alignment (int sn, int level, CHAR_DATA *ch, void *vo) {
OBJ_DATA  *obj=(OBJ_DATA *)vo;
int flags_to_rm=0;	
int risk=10;

                if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)) {
                      sprintf_to_char(ch, "%s is much too powerful!\r\n", capitalize(obj->short_descr));
                      return;
                }
	
	if (ch->alignment >350) {
		if (IS_SET(obj->extra_flags, ITEM_EVIL)) flags_to_rm+=ITEM_EVIL;
		if (IS_SET(obj->extra_flags, ITEM_ANTI_GOOD)) flags_to_rm+=ITEM_ANTI_GOOD;
	}

	if ((ch->alignment <=350) && (ch->alignment >=-350)) {
		if (IS_SET(obj->extra_flags, ITEM_ANTI_NEUTRAL)) flags_to_rm+=ITEM_ANTI_NEUTRAL;
	} else {
		if (IS_SET(obj->extra_flags, ITEM_ANTI_EVIL)) flags_to_rm+=ITEM_ANTI_EVIL;
	}
	
	if (!flags_to_rm) {
		send_to_char ("Your gods find nothing offensive about this item.\r\n",ch);
		return;
	}
	
	if ((obj->level) > (level+10)) risk+=5*((obj->level) - (level+10));

	if (number_percent() <= risk) {	
		extract_obj(obj);
		send_to_char ("{rYou have offended your gods! {mKaboom!{x  {rThe item explodes!  *sigh*{x\r\n",ch);
		act("{r$p shivers violently and explodes!{x",ch,obj,NULL,TO_ROOM);
		return;
	}
	
	if (number_percent() < (level*2/3 + ((get_curr_stat(ch,STAT_WIS)/4)-20)))  {
		send_to_char ("{cYour gods have favored you...they negate the alignment of the item. {x\r\n",ch);
		act("{c$p glows with the color of neutrality{c",ch,obj,NULL,TO_ROOM);
		obj->extra_flags-=flags_to_rm;
		return;
	} else	{
		send_to_char ("The item resists your efforts at negation.\r\n",ch);
		return;	
	}
                return;
}
		

void spell_null( int sn, int level, CHAR_DATA *ch, void *vo )
{
    send_to_char( "That's not a spell!\r\n", ch );
    return;
}


void spell_pass_door( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_PASS_DOOR) )
    {
	if (victim == ch)
	  send_to_char("You are already out of phase.\r\n",ch);
	else
	  act("$N is already shifted out of phase.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.type      = sn;
    af.afn 	 = gafn_pass_door;
    af.level     = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );
    act( "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You turn translucent.\r\n", victim );
    return;
}

/* RT plague spell, very nasty */

void spell_plague( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (saves_spell(level,victim) || (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)))    {
	if (ch == victim)  send_to_char("You feel momentarily ill, but it passes.\r\n",ch);
	else  act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.type 	  = sn;
    af.afn	  = gafn_plague;
    af.level	  = level * 3/4;
    af.duration   = level;
    af.location   = APPLY_STR;
    af.modifier   = -5; 
    af.bitvector  = AFF_PLAGUE;
    affect_join(victim,&af);
   
    send_to_char
      ("You scream in agony as plague sores erupt from your skin.\r\n",victim);
    act("$n screams in agony as plague sores erupt from $s skin.",
	victim,NULL,NULL,TO_ROOM);
}

void spell_poison( int sn, int level, CHAR_DATA *ch, void *vo) {
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;

    if ( saves_spell( level, victim ) ) {
	act("$n turns slightly green, but it passes.",victim,NULL,NULL,TO_ROOM);
	send_to_char("You feel momentarily ill, but it passes.\r\n",victim);
	return;
    }

    af.type      = sn;
    af.afn	 = gafn_poison;
    af.level     = level;
    af.duration  = 1 + (level/10);
    af.location  = APPLY_STR;
    af.modifier  = -1 * dice(2,4);
    af.bitvector = AFF_POISON;

    affect_join( victim, &af );

    send_to_char( "You feel very sick.\r\n", victim );

    act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);

    return;
}


void spell_portal (int sn, int level, CHAR_DATA *ch, void *vo ) {
char arg1[MAX_INPUT_LENGTH];
char buf[MAX_STRING_LENGTH];
char *remainder;
CHAR_DATA *victim;
OBJ_DATA *portal1;
OBJ_DATA *portal2;
int portal_level;

    if (IS_SET(ch->comm, COMM_NOPORT)) {
        send_to_char("The powers of order deny this dimensional travel!\r\n", ch);
        return; 
    }

   /* Must have a target... */

    if ( target_name == NULL 
      || target_name[0] == '\0' ) {
      send_to_char ("You failed.\r\n",ch);	
      return;
    }
	
   /* Work out the level of the portal... */

    remainder = one_argument(target_name, arg1);	

    if ( remainder != NULL 
      && remainder[0] != '\0' 
      && is_number(remainder) ) {
      portal_level = atoi(remainder);
    } else {
      portal_level = level;
    }
    
    if (portal_level > ch->level/2) {
      portal_level = ch->level/2;	
    }

   /* Reset target. Hmmm. Does this work? */
 
    target_name = arg1;	

   /* Only work from somewhere real... */

    if ( ch->in_room == NULL ) {
      send_to_char("Your portal shatters and disperses!\r\n", ch);
      return;
    }

   /* Find the victim... */

    victim = get_char_world( ch, target_name );

    if ( victim == NULL ) {
        send_to_char( "Your magic cannot find them!\r\n", ch );
        return;
    }

    if (room_is_private(victim, victim->in_room )) {
         send_to_char( "That room is private right now.\r\n", ch );
         return;
    }

    if (time_info.moon == MOON_NEW
    && !knows_discipline(ch, DISCIPLINE_LUNAR)) {
       send_to_char("The power of the moon disturbs your magical flux!\r\n", ch);
       return; 
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_LAW)
    || IS_SET(ch->in_room->area->area_flags, AREA_LAW)) {
        send_to_char( "You can't create portals in protected areas!\r\n", ch );
        return;
    }

   /* Check for sillyness... */

    if ( victim == ch ) {
        send_to_char( "Look in a mirror!\r\n", ch );
        return;
    }

     if (IS_SET(victim->act,ACT_CRIMINAL)) {
        send_to_char( "They seem to be hiding!\r\n", ch );
        return;
    }

    if (victim->pIndexData && !IS_NPC(ch)) {
       if (ch->pcdata->questmob == victim->pIndexData->vnum) {
          send_to_char( "They seem to be hiding!\r\n", ch );
          return;
       }
    }         

    if ( victim->in_room == ch->in_room ) {
      send_to_char("You portals implodes!\r\n", ch);
      return;
    }

   /* Check for range... */ 

    if (  victim->in_room == NULL
      ||  victim->in_room->area == NULL
      || !can_see_room(ch, victim->in_room) ) {
        send_to_char( "They are beyond your sight!\r\n", ch );
        return;
    }

    if (  ch->in_room->area != NULL
      &&  ch->in_room->area->zone != ch->in_room->area->zone ) {
        send_to_char( "They are beyond your reach!\r\n", ch );
        return;
    }

    if (IS_SET(victim->in_room->room_flags, ROOM_LAW)
    || IS_SET(victim->in_room->area->area_flags, AREA_LAW)) {
        send_to_char( "You can't create portals in protected areas!\r\n", ch );
        return;
    }

    if (IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
      || IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
      || IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
      || IS_SET(victim->in_room->area->area_flags, AREA_LOCKED) ) {
        send_to_char( "A strange energy disrupts your spell!\r\n", ch );
        return;
    }

   /* Finally give them a save... */

    if ( saves_spell( level, victim ) ) {
        send_to_char( "Your spell fails!\r\n", ch );
        return;
    }
	
   /* Hmmm. Spell seems to allow free exit from combat... */

    if (victim->fighting != NULL) {
	stop_fighting(victim,TRUE);
    } 
    
   /* caster room messages */

    send_to_char(
       "Swirling magic creates a portal before you!\r\n", ch);

    act("$n swirls mystic energy together to form a portal!",
         ch, NULL, NULL, TO_ROOM);

   /* victim room message */

    act ("A sudden swirl of magical energy transforms into a portal!", victim, NULL, NULL, TO_ROOM);

    send_to_char("A sudden swirl of magical energy transforms into a portal!\r\n", victim);

   /* create the portals now */

    portal1 = create_object (get_obj_index(OBJ_VNUM_PORTAL), portal_level);
    portal1->level = portal_level;
    portal1->timer = level / 5;
    portal1->value[0] = victim->in_room->vnum;
    portal1->value[1] = OBJ_VNUM_PORTAL;

    free_string (portal1->description);

    sprintf (buf, "A portal leading to %s is hovering here.",
                   victim->in_room->name);
    portal1->description = strdup (buf);
	
    portal2 = create_object (get_obj_index(OBJ_VNUM_PORTAL), portal_level);
    portal2->level = portal_level;
    portal2->timer = level / 5;
    portal2->value[0] = ch->in_room->vnum;
    portal2->value[1] = OBJ_VNUM_PORTAL;

    free_string (portal2->description);

    sprintf (buf, "A portal leading to %s is hovering here.",
                   ch->in_room->name);
    portal2->description = strdup(buf);

   /* put portals in rooms */
	
    obj_to_room (portal1, ch->in_room);
    obj_to_room (portal2, victim->in_room);
	
    return;
}	


void spell_destroy_portal (int sn, int level, CHAR_DATA *ch, void *vo ) {
    OBJ_DATA *portal = (OBJ_DATA *) vo;
    OBJ_DATA *portal2 = NULL;
    ROOM_INDEX_DATA *room;
    char * lastarg;
    int chance;

    if (portal->item_type != ITEM_PORTAL)    {
	send_to_char("That isn't a portal.\r\n",ch);
	return;
    }

    if (portal->value[4] != PORTAL_MAGIC)    {
	send_to_char("That isn't a portal.\r\n",ch);
	return;
    }

    lastarg = check_obj_owner(portal);
    if (strcmp(lastarg, "not found")
    && strcmp(capitalize(lastarg), ch->short_descr)) {
         send_to_char ("This portal is not yours!\r\n", ch);
         return;
    }      

    chance = (level - portal->level +15) * 3;
    if (number_percent() > chance) {
         act("$p glows for a moment.",NULL, portal , NULL,TO_ROOM);
         send_to_char ("The portal resists!\r\n", ch);
         return;
    }      


    if (portal->value[0] >0
    && portal->value[1] >0) {
        room = get_room_index(portal->value[0]);

        portal2 = room->contents;
        while (portal2 != NULL
        && (portal2->item_type != ITEM_PORTAL
              || ch->in_room != get_room_index(portal2->value[0]))) {
             portal2 = portal2->next_content;
        }
    
         if (portal2 != NULL ) {
                 act("{Y$p suddenly explodes.{x",NULL, portal2 , NULL,TO_ROOM);
                 obj_from_room(portal2);
                 extract_obj(portal2);
         }        
    }

    act("{Y$p suddenly explodes.{x",NULL, portal , NULL,TO_ROOM);
    obj_from_room(portal);
    extract_obj(portal);
    return;
}

	
void spell_protection_evil ( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_PROTECTE) || 
		 IS_AFFECTED(victim, AFF_PROTECTG) )
    {
	if (victim == ch)
	  send_to_char("You are already protected.\r\n",ch);
	else
	  act("$N is already protected.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.type      = sn;
    af.afn 	 = gafn_protection_evil;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PROTECTE;
    affect_to_char( victim, &af );
    send_to_char( "You feel protected from evil.\r\n", victim );
    if ( ch != victim ) act("$N is protected from harm.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_protection_good ( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_PROTECTE) || 
		 IS_AFFECTED(victim, AFF_PROTECTG) )
    {
	if (victim == ch)
	  send_to_char("You are already protected.\r\n",ch);
	else
	  act("$N is already protected.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.type      = sn;
    af.afn	 = gafn_protection_good;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PROTECTG;
    affect_to_char( victim, &af );
    send_to_char( "You feel protected from good.\r\n", victim );
    if ( ch != victim )
	act("$N is protected from harm.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_psychic_anchor (int sn, int level, CHAR_DATA *ch, void *vo) {

    if (IS_AFFECTED (ch, AFF_MELD)) {
        send_to_char ("Your head is too scrambled to concentrate on this spell.\r\n", ch);
        return;
    }	
	
    if ( IS_SET(ch->in_room->room_flags, ROOM_PRIVATE) ) {
        send_to_char ("You cannot make a private room your recall point.\r\n", ch);
        return;
    }
	
    send_to_char("Your thoughts form a perfect mental image of this room.\r\n", ch);
    send_to_char("Word of recall will now bring you to this room.\r\n", ch);

    ch->recall_temp=ch->in_room->vnum;	

    return;
}


void spell_mask_self ( int sn, int level, CHAR_DATA *ch, void *vo )	{
CHAR_DATA *victim=(CHAR_DATA *)vo;
AFFECT_DATA af;

                if (IS_SET(ch->act,ACT_WERE) ) {
                       send_to_char ("You are immune against Mask Self!\r\n",ch);
                       return;
                 }                

	if ( IS_AFFECTED(ch, AFF_POLY)
                || IS_AFFECTED(ch, AFF_MORF)){
		send_to_char ("You are already masked.\r\n",ch);
		return;
	}

	if (victim == ch) {
		send_to_char ("You must specify a target creature.\r\n",ch);
		return;
	}
	
	if (IS_MORTAL(ch) && !IS_NPC(victim)) {
		send_to_char ("You can only mask yourself to look like mobs.\r\n",ch);
		return;
	}

                if (!str_cmp(race_array[victim->race].name,"yithian")) {
		send_to_char ("You can' mask as a Yithian.\r\n",ch);
		return;
	}

                if (IS_SET(victim->imm_flags, IMM_MASK)) {
		send_to_char ("You can' mask as that.\r\n",ch);
		return;
	}

                if (IS_AFFECTED(victim, AFF_CHARM)) {
		send_to_char ("Wouldn't that confuse you?.\r\n",ch);
		return;
	}

		
    af.type      = sn;
    af.afn	 = gafn_mask_self;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_POLY;
    affect_to_char( ch, &af );
    send_to_char( "You reshape yourself into the likeness of your target.\r\n", ch );

	/*save old descriptions*/	
	free_string(ch->description_orig);
	free_string(ch->short_descr_orig);
	free_string(ch->long_descr_orig);
	ch->description_orig = str_dup (ch->description);
	ch->short_descr_orig = str_dup (ch->short_descr);
	ch->long_descr_orig  = str_dup (ch->long_descr);
                ch->race_orig=ch->race;

	/*apply poly descriptions*/
	free_string(ch->description);
	free_string(ch->short_descr);
	free_string(ch->long_descr);
	ch->description = str_dup (victim->description);
                ch->race=victim->race;
	if (IS_NPC(victim))
		ch->short_descr = str_dup (victim->short_descr);
	else
		ch->short_descr = str_dup (victim->name);
	ch->long_descr  = str_dup (victim->long_descr);
	free_string(ch->poly_name);
	ch->poly_name = str_dup	(victim->name);
	ch->start_pos = victim->start_pos; 
 	}	


void spell_refresh( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->move = UMIN( victim->move + level*2, victim->max_move );
    if (victim->max_move == victim->move)
	send_to_char("You feel fully refreshed!\r\n",victim);
    else
    	send_to_char( "You feel less tired.\r\n", victim );
    affect_strip_afn(victim, gafn_fatigue);
    if ( ch != victim )
	send_to_char( "Ok.\r\n", ch );
    return;
}

void spell_regeneration( int sn, int level, CHAR_DATA *ch, void *vo )	{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	
	if (IS_AFFECTED (victim, AFF_REGENERATION))
		{
		send_to_char ("That target is already regenerating.\r\n",ch);
		return;
		}
	
    af.type      = sn;
    af.afn	 = gafn_regeneration; 
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_NONE;
	af.modifier  = 0;
    af.bitvector = AFF_REGENERATION;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself healing your wounds more quickly.\r\n", victim );
    act("$n is healing much more quickly.",victim,NULL,NULL,TO_ROOM);
	return;
	}


void spell_remove_curse( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;
    OBJ_DATA *obj;
    int iWear;
    char buf[MAX_STRING_LENGTH]; 

   /* Remove personal curses... */

    if (is_affected(victim, gafn_curse)) {

      send_to_char("Attempting to remove personal curse.\r\n", ch);
  
      if (check_dispel(level,victim, gafn_curse)) {
	send_to_char("You feel better.\r\n", victim);
	act("$n looks more relaxed.",victim,NULL,NULL,TO_ROOM);
                send_to_char("Successful.\r\n", ch);
      } else {
                send_to_char("Unsuccessful.\r\n", ch);
      }
    }

   /* Remove worn equipment curses... */

    for ( iWear = 0; ( iWear < MAX_WEAR && !found); iWear ++) {
    
	obj = get_eq_char(victim, iWear);

	if (obj == NULL) continue;
                if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)) continue;

                if ( IS_SET(obj->extra_flags, ITEM_NODROP) 
                || IS_SET(obj->extra_flags, ITEM_NOREMOVE) ) {
       
                    sprintf_to_char(ch, "Attempting to uncurse %s.\r\n", obj->short_descr); 

           /* Some curses cannot be lifted... */

            if (IS_SET(obj->extra_flags, ITEM_NOUNCURSE)) {
                act("{YYour $p glows a sickly yellow!{x",victim,obj,NULL,TO_CHAR);
                act("{Y$n's $p glows a sickly yellow!{x",victim,obj,NULL,TO_ROOM);
                send_to_char("Permanently cursed!\r\n", ch);
            } else {

                if (!saves_dispel(level, obj->level, 0)) {
                    found = TRUE; 
                    REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
                    REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
                    act("{CYour $p glows a beautiful blue!{x",victim,obj,NULL,TO_CHAR);
                    act("{C$n's $p glows a beautiful blue!{x",victim,obj,NULL,TO_ROOM);
                    send_to_char("Success!\r\n", ch);
                } else {
                    act("{RYour $p glows a stubbon red!{x",victim,obj,NULL,TO_CHAR);
                    act("{R$n's $p glows a stubbon red!{x",victim,obj,NULL,TO_ROOM);
                    send_to_char("The items darkness has resisted your power!\r\n", ch);
                }
            } 
        }
    }

    for ( obj = victim->carrying; 
         ( obj != NULL 
        && !found ); 
          obj = obj->next_content) {
   
        if ( IS_SET(obj->extra_flags, ITEM_NODROP) 
          || IS_SET(obj->extra_flags, ITEM_NOREMOVE) ) {
       
            sprintf(buf, "Attempting to uncurse %s.\r\n", obj->short_descr); 

            send_to_char(buf, ch);

           /* Some curses cannot be lifted... */

            if (IS_SET(obj->extra_flags, ITEM_NOUNCURSE)) {
                act("{YYour $p glows a sickly yellow!{x",victim,obj,NULL,TO_CHAR);
                act("{Y$n's $p glows a sickly yellow!{x",victim,obj,NULL,TO_ROOM);
                send_to_char("Permanently cursed!\r\n", ch);
            } else {

                if (!saves_dispel(level, obj->level, 0)) {
                    found = TRUE; 
                    REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
                    REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
                    act("{CYour $p glows a beautiful blue!{x",victim,obj,NULL,TO_CHAR);
                    act("{C$n's $p glows a beautiful blue!{x",victim,obj,NULL,TO_ROOM);
                    send_to_char("Successful!\r\n", ch);
                } else {
                    act("{RYour $p glows a stubbon red!{x",victim,obj,NULL,TO_CHAR);
                    act("{R$n's $p glows a stubbon red!{x",victim,obj,NULL,TO_ROOM);
                    send_to_char("The items darkness has resisted your power!\r\n", ch);
                }
            } 
        }
    }

   /* Default message if no cursed items... */

    if (!found) {
        if ( ch != victim ) {
            send_to_char("Victim has no cursed items.\r\n", ch);
            send_to_char("You feel unusually clean!\r\n", victim);
        } else {
            send_to_char("You feel purified!\r\n", ch);
        }	
    }

    return;
}

void spell_remove_fear( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (check_dispel(level,victim, gafn_fear) )
    	{
		act("$n no longer looks terrified.",victim,NULL,NULL,TO_ROOM);
    	}
	else 
		send_to_char ("Spell failed.\r\n",ch);
}

void spell_remove_invis( int sn, int level, CHAR_DATA *ch, void *vo) {
	OBJ_DATA  *obj=(OBJ_DATA *)vo;
	int risk=5;
	
	if (!IS_SET(obj->extra_flags, ITEM_INVIS))
		{
		send_to_char ("Strange, everyone else can already see that item.\r\n",ch);
		send_to_char ("Spell failed.\r\n",ch);
		return;
		}

	if ((obj->level) > (level+5))	/*risky*/
		risk+=4*((obj->level) - (level+10));

	/* hack, remove invis now so act's will show it properly */
	obj->extra_flags -= ITEM_INVIS;

	if (number_percent() <= risk)
		{	
		extract_obj(obj);
		send_to_char ("Just as the item starts to appear...{mKaboom!{x  It {mexplodes!{x\r\n",ch);
		act("$p appears suddenly...then shivers violently and {mexplodes{!{x",ch,obj,NULL,TO_ROOM);
		return;
		}
	
	if (number_percent() < (level*2/3 + ((get_curr_stat(ch,STAT_INT)/4)-20)))  
		{
		send_to_char ("{cA bright flash of light appears and fades, revealing the item to all.{x\r\n",ch);
		act("{cA bright flash of light appears and fades, revealing $p.{x",ch,obj,NULL,TO_ROOM);
		send_to_char ("Spell successful.\r\n",ch);
		return;
		}
	else	
		{
		send_to_char ("The item resists your efforts at removing its invisibility.\r\n",ch);
		obj->extra_flags += ITEM_INVIS;
		return;	
		}
	} /*end spell*/
		

void spell_sanctuary( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
    {
	if (victim == ch)
	  send_to_char("You are already in sanctuary.\r\n",ch);
	else
	  act("$N is already in sanctuary.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.type      = sn;
    af.afn 	 = gafn_sanctuary;
    af.level     = level;
    af.duration  = number_fuzzy( level / 6 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char( victim, &af );
    act( "$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by a white aura.\r\n", victim );
    return;
}


void spell_shield( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, gafn_shield)
    || is_affected(victim, gafn_mana_shield)) {
        if (!in_seq) {
    	  if (victim == ch) send_to_char("You are already shielded from harm.\r\n",ch);
	  else act("$N is already protected by a shield.", ch, NULL, victim, TO_CHAR);
        }
        return;
    }

    af.type      = sn;
    if (!str_cmp(target_name, "mana")) af.afn = gafn_mana_shield;
    else af.afn = gafn_shield;
    af.level     = level;
    af.duration  = 8 + level;
    af.location  = APPLY_AC;
    af.modifier  = -1 * ( 50 + (level/2));
    af.bitvector = 0;
    affect_to_char( victim, &af ); 

    if (!in_seq) {
      act( "$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM );
      send_to_char( "You are surrounded by a force shield.\r\n", victim );
    } 
    return;
}


void spell_shocking_grasp( int sn, int level, CHAR_DATA *ch, void *vo ) {

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_LIGHTNING;
    spell_CDB->atk_blk_type = BLOCK_ALL;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
   /* Set up and adjust damage... */

    spell_CDB->damage = 27 + dice(level/5, 5);

    if ( saves_spell( level, spell_CDB->defender ) ) {
	spell_CDB->damage /= 2;
    } 

   /* Roasting time... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

    return;
}


void spell_sleep( int sn, int level, CHAR_DATA *ch, void *vo ){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
  
    if ( IS_AFFECTED(victim, AFF_SLEEP)
    ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD))
    ||   level < victim->level
    ||   saves_spell( level, victim ) )
	return;

    af.type      = sn;
    af.afn	 = gafn_sleep;
    af.level     = level;
    af.duration  = 4 + level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );

    if ( IS_AWAKE(victim) )
    {
	send_to_char( "You feel very sleepy ..... zzzzzz.\r\n", victim );
	act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
        set_activity( victim, POS_SLEEPING, "sleeping soundly.", ACV_NONE, NULL);
    }

    return;
}


void spell_hard_skin( int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA *victim = (CHAR_DATA *) vo;
AFFECT_DATA af;
int armor;

    if ( is_affected( ch, gafn_hard_skin ) ) {
        if (!in_seq) {
   	  if (victim == ch) send_to_char("Your skin is already hardened.\r\n",ch); 
	  else act("$N is already hardened.",ch,NULL,victim,TO_CHAR);
        }
        return;
    }

    if (!str_cmp(target_name, "steel")) armor = 120 + ch->level;
    else if (!str_cmp(target_name, "iron")) armor = 80 + ch->level;
    else if (!str_cmp(target_name, "stone")) armor = 60;
    else armor = 30;

    af.type      	= sn;
    af.afn	 	= gafn_hard_skin;
    af.level     	= level;
    af.duration  	= level;
    af.location  	= APPLY_AC;
    af.modifier  	= -armor;
    af.bitvector 	= 0;
    affect_to_char( victim, &af );

    if (!in_seq) {

        if (!str_cmp(target_name, "steel")) {
              act( "$n's skin turns to steel.", victim, NULL, NULL, TO_ROOM );
              send_to_char( "Your skin turns to steel.\r\n", victim );
        } else if (!str_cmp(target_name, "iron")) {
              act( "$n's skin turns to iron.", victim, NULL, NULL, TO_ROOM );
              send_to_char( "Your skin turns to iron.\r\n", victim );
        } else if (!str_cmp(target_name, "stone")) {
              act( "$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM );
              send_to_char( "Your skin turns to stone.\r\n", victim );
        } else {
              act( "$n's skin turns to bark.", victim, NULL, NULL, TO_ROOM );
              send_to_char( "Your skin turns to bark.\r\n", victim );
        }
    }
    return;
}


void spell_summon( int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (IS_SET(ch->comm, COMM_NOPORT)) {
        send_to_char("The powers of order deny this dimensional travel!\r\n", ch);
        return; 
    }

    if (!strcmp(target_name, "immortal")) level *= 2;

    if ( ch->in_room == NULL ) {
      send_to_char("Your magic dissapates into the void!\r\n", ch);
      return;
    }

    if ( victim == ch ) {
        send_to_char( "Look in a mirror!\r\n", ch );
        return;
    }

    if (time_info.moon == MOON_NEW
    && !knows_discipline(ch, DISCIPLINE_LUNAR)) {
       send_to_char("The power of the moon disturbs your magical flux!\r\n", ch);
       return; 
    }

     if (!IS_NPC(victim)) {
         if (victim->desc->connected != 0 || victim->desc->editor != 0) {
             send_to_char( "They are beyond your sight!\r\n", ch );
             return;
         }
     }

     if (IS_SET(victim->act, ACT_WATCHER)) {
             send_to_char( "They are beyond your sight!\r\n", ch );
             return;
     }

     if (IS_SET(victim->act, ACT_CRIMINAL)) {
        send_to_char( "They seem to be hiding!\r\n", ch );
        return;
    }

    if (victim->pIndexData && !IS_NPC(ch)) {
       if (ch->pcdata->questmob == victim->pIndexData->vnum) {
          send_to_char( "They seem to be hiding!\r\n", ch );
          return;
       }
    }         

    if (IS_SET(ch->in_room->room_flags, ROOM_LAW)
    || IS_SET(ch->in_room->area->area_flags, AREA_LAW)) {
        send_to_char( "You can't summon to protected areas!\r\n", ch );
        return;
    }

    if ( victim->in_room == ch->in_room ) {
      send_to_char("Nothing happens!\r\n", ch);
      return;
    }

   /* Check for range... */ 

    if (strcmp(target_name, "immortal")) {
        if (  victim->in_room == NULL
        ||  victim->in_room->area == NULL
        || !can_see_room(ch, victim->in_room) ) {
             send_to_char( "They are beyond your sight!\r\n", ch );
             return;
        }

        if (  ch->in_room->area != NULL
        &&  ch->in_room->area->zone != ch->in_room->area->zone ) {
             send_to_char( "They are beyond your reach!\r\n", ch );
             return;
        }
    }

    if (IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
      || IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
      || IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
      || IS_SET(victim->in_room->area->area_flags, AREA_LOCKED) ) {
        send_to_char( "A strange energy disrupts your spell!\r\n", ch );
        return;
    }

   /* Check for permission... */

    if ( IS_NPC(victim) ) {
        if ( IS_SET(victim->imm_flags, IMM_SUMMON)) {
          send_to_char( "Your spell fails!\r\n", ch );
          return;
        }

        if ( IS_SET(victim->act, ACT_AGGRESSIVE)) {
          send_to_char( "Your spell fails!\r\n", ch );
          return;
        }

    } else {
        if (strcmp(target_name, "immortal")) level /=2;
        
        if (IS_NEWBIE(victim) && IS_SET(victim->plr, PLR_NOSUMMON)) {
          send_to_char( "Your spell cannot touch them!\r\n", ch );
          return;
        }

        if (IS_IMMORTAL(victim)
        && strcmp(target_name, "immortal")) {
          send_to_char( "Your spell cannot touch them!\r\n", ch );
          return;
        }

    }
	
    if ( victim->level > (level + 3) ) {
        send_to_char( "You are not powerful enough!\r\n", ch );
        return;
    }
	
    if ( victim->fighting != NULL ) {
        send_to_char( "Your magic cannot grasp them!\r\n", ch );
        return;
    }
	
   /* Finally give them a save... */

    if (IS_SET(victim->plr, PLR_NOSUMMON) && saves_spell( level, victim )) {
        send_to_char( "They resist your spell!\r\n", ch );
        return;
    }
	
    if (victim->fighting != NULL) stop_fighting(victim,TRUE);
    act("{m$n disappears into a fold in space!{x", victim, NULL, NULL, TO_ROOM );
    add_tracks(ch, DIR_NONE, DIR_OTHER);

    char_from_room( victim );
    char_to_room( victim, ch->in_room );

    add_tracks(ch, DIR_OTHER, DIR_HERE);

    act("{m$n appears out of thin air!{x", victim, NULL, NULL, TO_ROOM );
    sprintf_to_char(victim,"{m%s has summoned you!{x\r\n",ch->short_descr);
    do_look( victim, "auto" );
    return;
}



void spell_teleport( int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA *victim = (CHAR_DATA *) vo;
ROOM_INDEX_DATA *pRoomIndex, *pRoom2;
int count;

   /* Can't teleport out of the void... */

    if (IS_SET(victim->comm, COMM_NOPORT)) {
        send_to_char("The powers of order deny this dimensional travel!\r\n", ch);
        return; 
    }

    if ( victim->in_room == NULL ) {
       send_to_char("You feel a strange empty feeling!\r\n", victim);
       if ( ch != victim ) {
         send_to_char("Nothing seems to happen!\r\n", ch);
       }
       return; 
    }

   /* Can't teleport out of a NO_RECALL room... */

    if ( IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL) ) {
       send_to_char("You feel a magical preasure upon your mind!\r\n", victim);
       if ( ch != victim ) {
         send_to_char("A mysterious force blocks your spell!\r\n", ch);
       }
       return; 
    }

   /* Can't teleport dangerous mobs... */

    if ( IS_NPC(ch) ) {
      if ( IS_SET(victim->act, ACT_AGGRESSIVE) ) {
         send_to_char("You briefly feel a magical force!\r\n", victim);
         if ( ch != victim ) {
           send_to_char("Your spell cannot grasp them!\r\n", ch);
         }
         return; 
      }
    }

   /* You can teleport yourself, but others get a save... */

    if ( ch != victim 
      && saves_spell(level, victim) ) {
       send_to_char("You feel a magical grasp upon your body!\r\n", victim);
       send_to_char("They resist your spell!\r\n", ch);
       return; 
    }

    if (time_info.moon == MOON_NEW
    && !knows_discipline(ch, DISCIPLINE_LUNAR)) {
       send_to_char("The power of the moon disturbs your magical flux!\r\n", ch);
       return; 
    }


   /* Ok, where do we send them... */ 
 
    for (count = 0 ; count < 50 ; count++ ) {

        pRoomIndex = get_room_index( number_range( 0, 32767 ) );

        if ( pRoomIndex == NULL 
         ||  pRoomIndex->area == NULL ) {
          continue;
        }
  
        if ( victim->in_room->area != NULL
          && victim->in_room->area->zone != pRoomIndex->area->zone ) {
          continue;
        }   

        if (  can_see_room(victim, pRoomIndex)
          && !IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY)
          && !IS_SET(pRoomIndex->area->area_flags, AREA_LOCKED) ) {
	  break;
       }
    }

    if ( count >= 50 || pRoomIndex == NULL ) {
      send_to_char("You feel dizzy and disorientated!\r\n", victim);
      if ( victim != ch ) {
        send_to_char("They fade, but then return...\r\n", ch); 
      } 
      return;
    }

   /* Check for day/night shift... */

    if ( IS_SET(time_info.flags, TIME_NIGHT) ) {

      if (pRoomIndex->night != 0) { 
        pRoom2 = get_room_index(pRoomIndex->night);

        if ( pRoom2 != NULL ) {
          pRoomIndex = pRoom2;
        }
      }
    } else if ( IS_SET(time_info.flags, TIME_DAY) ) {

      if (pRoomIndex->day != 0) { 
        pRoom2 = get_room_index(pRoomIndex->day);

        if ( pRoom2 != NULL ) {
          pRoomIndex = pRoom2;
        }
      }
    }

   /* Hmmm. Spell seems to allow free exit from combat... */

    if (victim->fighting != NULL) {
	stop_fighting(victim,TRUE);
    } 
    
    if (victim != ch) {
	send_to_char("You have been teleported!\r\n", victim);
    } 

    act( "$n vanishes!", victim, NULL, NULL, TO_ROOM );

    add_tracks(victim, DIR_NONE, DIR_OTHER);

    char_from_room( victim );
    char_to_room( victim, pRoomIndex );

    add_tracks(victim, DIR_OTHER, DIR_HERE);

    act( "$n appears in a blinding flash of light!", victim, NULL, NULL, TO_ROOM );

    do_look( victim, "auto" );

    return;
}



void spell_ventriloquate( int sn, int level, CHAR_DATA *ch, void *vo ) {
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char speaker[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;
    CHAR_DATA *victim;

    target_name = one_argument(target_name, speaker );
    if ((victim = get_char_room(ch, speaker)) == NULL) {
          send_to_char ("They're not here.\r\n",ch);
          return;
    }
  
    sprintf(buf1, "%s says '%s'.\r\n",  IS_NPC(victim) ? victim->short_descr : victim->name, target_name);
    sprintf(buf2, "Someone makes %s say '%s'.\r\n", IS_NPC(victim) ? victim->short_descr : victim->name, target_name);
    buf1[0] = UPPER(buf1[0]);
 
   for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )    {
	if ( !is_name( speaker, vch->name ))  send_to_char( saves_spell( level, vch ) ? buf2 : buf1, vch );
    }
    return;
}

void spell_vocalize ( int sn, int level, CHAR_DATA *ch, void *vo){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	
	if (is_affected(victim, gafn_vocalize)) {
		send_to_char ("That target already has the benefit of vocalize.\r\n",ch);
		return;
		}
	
    af.type      = sn;
    af.afn	 = gafn_vocalize;
    af.level     = level;
    af.duration  = level / 7;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = 0;
    affect_to_char( victim, &af );
	send_to_char ("Vocalize spell successful.\r\n",ch);
	send_to_char ("You feel you can cast spells without speaking.\r\n",victim);
	return;
	}


void spell_weaken( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, gafn_weakness ) 
      || saves_spell( level, victim ) )
	return;
    af.type      = sn;
    af.afn	 = gafn_weakness;
    af.level     = level;
    af.duration  = level / 2;
    af.location  = APPLY_STR;
    af.modifier  = -1 * (level / 5);
    af.bitvector = AFF_WEAKEN;
    affect_to_char( victim, &af );
    send_to_char( "You feel weaker.\r\n", victim );
    act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM);
    return;
}


void spell_word_of_recall( int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA *victim = (CHAR_DATA *) vo;
OBJ_DATA *obj;
ROOM_INDEX_DATA *location, *old_room, *loc2;
CHAR_DATA *rch, *next_rch;
bool zone_check = FALSE;
    
    if ( IS_NPC(victim)) return;
  	
   /* Determine recall location... */

    location = NULL;

   /* look for material anchors first... */

    for (obj = object_list; obj; obj = obj->next) {
         if (obj->item_type != ITEM_FOCUS) continue;
         if (!obj->owner) continue;

         if (!strcmp(obj->owner, victim->name)) break;
    }

    if (obj) {    
        if (obj->value[0] == 0
        || obj->value[0] & zones[ch->in_zone].mask) {
            if (obj->in_room) location = obj->in_room;
            else if (obj->carried_by) location = obj->carried_by->in_room;
            else if (obj->in_obj) location = obj->in_obj->in_room;
        }
    }

    if (!location) location = get_room_index(ch->recall_temp);

    if (!location) zone_check = TRUE;

    if ( location == NULL
    && victim->in_room != NULL
    && victim->in_room->recall != 0 ) { 

      location = get_room_index( victim->in_room->recall);
    }

   /* First we check the areas recall location... */

    if (!location
      && victim->in_room != NULL
      && victim->in_room->area != NULL 
      && victim->in_room->area->recall != 0 ) { 

      location = get_room_index( victim->in_room->area->recall);
    }

   /* Then we check the zones recall location... */

    if (!location
      && victim->in_room != NULL
      && victim->in_room->area != NULL 
      && zones[victim->in_room->area->zone].recall != 0 ) { 

      location = get_room_index( zones[victim->in_room->area->zone].recall);
    }

   /* Finally we check the default location... */

    if (!location) {
      location = get_room_index( mud.recall );
    }

   /* Then their permanent recall location... */

    if (!location) {
        location = get_room_index( ch->recall_perm );
    }
 
   /* If still not there, then give up... */

    if (!location) {
        send_to_char("You are completely lost.\r\n",victim);
        return;
    }

   /* Check for day/night shift... */

    if ( IS_SET(time_info.flags, TIME_NIGHT) ) {

      if (location->night != 0) { 
        loc2 = get_room_index(location->night);

        if (loc2) location = loc2;
      }
    } else if ( IS_SET(time_info.flags, TIME_DAY) ) {
      if (location->day != 0) { 
        loc2 = get_room_index(location->day);

        if (loc2) location = loc2;
      }
    }

   /* Some places the spell won't work... */

    if ( IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
      || IS_AFFECTED(victim, AFF_CURSE)) {
	send_to_char("A mysterious force disrupts the spell!\r\n",victim);
	return;
    }

   /* Can't be used to simply go nowhere... */

    if ( victim->in_room == location ) return;

   /* Cross zone recalls are a no-no... */

    if (zone_check
    && victim->in_room->area != NULL
    && location->area != NULL
    && victim->in_room->area->zone != location->area->zone ) {
         send_to_char("You cannot feel your recall point!\r\n", victim); 
         return;
    } 

   /* Hmmm. Spell seems to allow free exit from combat... */

    if (victim->fighting != NULL) stop_fighting(victim,TRUE);
    
   /* Do the move... */
    victim->move /= 2;

    old_room = victim->in_room;

    if (old_room != NULL) act( "$n disappears.", victim, NULL, NULL, TO_ROOM );
    add_tracks(ch, DIR_NONE, DIR_OTHER);
    char_from_room( victim );
    char_to_room( victim, location );
    add_tracks(ch, DIR_OTHER, DIR_HERE);

    act( "$n appears in the room.", victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );

   /* Recall the pet as well... */
 
    if (old_room != NULL) {
      for (rch = old_room->people; rch != NULL; rch = next_rch) {

        next_rch = rch->next_in_room;

        if ( rch->in_room == old_room 
        && rch->master == victim ) { 
          act( "$n disappears.", rch, NULL, NULL, TO_ROOM );

          add_tracks(rch, DIR_NONE, DIR_OTHER);
          char_from_room( rch );
          char_to_room( rch, location );
          add_tracks(rch, DIR_OTHER, DIR_HERE);
          act( "$n appears in the room.", rch, NULL, NULL, TO_ROOM );
        }
      }
    }

     return;
}


/*
 * NPC spells.
 */
void spell_acid_breath( int sn, int level, CHAR_DATA *ch, void *vo ) {

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_ACID;
    spell_CDB->atk_blk_type = BLOCK_BALL;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  

    if (number_percent() < 25) harmful_effect(vo, level, TAR_CHAR_OFFENSIVE, DAM_ACID);

   /* Set up and adjust damage... */

    spell_CDB->damage = 10 + dice(level/10, 5);

    if ( saves_spell( level, spell_CDB->defender ) ) {
	spell_CDB->damage /= 2;
    } 

   /* Roasting time... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

    return;
}



void spell_fire_breath( int sn, int level, CHAR_DATA *ch, void *vo ) {

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_FIRE;
    spell_CDB->atk_blk_type = BLOCK_BALL;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
    if (number_percent() < 25) harmful_effect(vo, level, TAR_CHAR_OFFENSIVE, DAM_FIRE);

   /* Set up and adjust damage... */

    spell_CDB->damage = 10 + dice(level/10, 5);

    if ( saves_spell( level, spell_CDB->defender ) ) {
	spell_CDB->damage /= 2;
    } 

   /* Roasting time... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

    return;
}



void spell_frost_breath( int sn, int level, CHAR_DATA *ch, void *vo ) {

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_COLD;
    spell_CDB->atk_blk_type = BLOCK_BALL;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
    if (number_percent() < 25) harmful_effect(vo, level, TAR_CHAR_OFFENSIVE, DAM_COLD);

   /* Set up and adjust damage... */

    spell_CDB->damage = 10 + dice(level/10, 5);

    if ( saves_spell( level, spell_CDB->defender ) ) {
	spell_CDB->damage /= 2;
    } 

   /* Roasting time... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

    return;
}


void spell_gas_breath( int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA *vch;
CHAR_DATA *vch_next;
int hpch;

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )   {
	vch_next = vch->next_in_room;
                if (vch == ch) continue;

	if ( !is_safe_spell(ch,vch,TRUE)){
           /* Reset spell_CDB... */

            spell_CDB = getCDB(ch, vch, sn, FALSE, sn, 0);

           /* Set up combat data... */

            spell_CDB->atk_dam_type = DAM_POISON;
            spell_CDB->atk_blk_type = BLOCK_ABSORB;

           /* Bail out if it misses... */
 
            if (!check_spell_hit(spell_CDB, spell_roll)) {
              return;
            }  
 
           /* Set up and adjust damage... */

	    hpch = UMAX( 10, ch->hit );

            spell_CDB->damage = number_range(hpch/16+1, hpch/8);

            if ( saves_spell( level, spell_CDB->defender ) ) {
         	spell_CDB->damage /= 2;
            } 

           /* Roasting time... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

	}
    }

    return;
}


void spell_lightning_breath( int sn, int level, CHAR_DATA *ch, void *vo ) {
//    CHAR_DATA *victim = (CHAR_DATA *) vo;

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_LIGHTNING;
    spell_CDB->atk_blk_type = BLOCK_LIGHTNING;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
    if (number_percent() < 25) harmful_effect(vo, level, TAR_CHAR_OFFENSIVE, DAM_LIGHTNING);

   /* Set up and adjust damage... */

    spell_CDB->damage = 10 + dice(level/10, 5);

    if ( saves_spell( level, spell_CDB->defender ) ) {
	spell_CDB->damage /= 2;
    } 

   /* Roasting time... */

if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
     SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
     apply_damage( spell_CDB);  
     REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
 } else {
    apply_damage( spell_CDB);
 }

    return;
}

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
void spell_general_purpose( int sn, int level, CHAR_DATA *ch, void *vo ) {
//    CHAR_DATA *victim = (CHAR_DATA *) vo;
 
   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_PIERCE;
    spell_CDB->atk_blk_type = BLOCK_COMBAT;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
   /* Set up and adjust damage... */

    spell_CDB->damage = 20 + dice(8, 10);

    if ( saves_spell( level, spell_CDB->defender ) ) {
	spell_CDB->damage /= 2;
    } 

   /* Roasting time... */

    apply_damage( spell_CDB);

    return;
}

void spell_high_explosive( int sn, int level, CHAR_DATA *ch, void *vo )
{
//    CHAR_DATA *victim = (CHAR_DATA *) vo;
 
   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_PIERCE;
    spell_CDB->atk_blk_type = BLOCK_COMBAT;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
   /* Set up and adjust damage... */

    spell_CDB->damage = 30 + dice(10, 10);

    if ( saves_spell( level, spell_CDB->defender ) ) {
	spell_CDB->damage /= 2;
    } 

   /* Roasting time... */

    apply_damage( spell_CDB);

    return;
}

/* psionics begin here */

void spell_mind_meld ( int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA  *victim = (CHAR_DATA *) vo;
AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_MELD ) || saves_spell( level, victim ) )
    {
        send_to_char( "Your efforts fail to produce melding.\r\n", ch );
        return;
    }

   /* Set up combat data... */

    spell_CDB->atk_dam_type = DAM_MENTAL;
    spell_CDB->atk_blk_type = BLOCK_ABSORB;

   /* Bail out if it misses... */
 
    if (!check_spell_hit(spell_CDB, spell_roll)) {
      return;
    }  
 
    af.type      = sn; 
    af.afn	 = gafn_mind_meld;
    af.level     = level;
    af.duration  = 2 + level;
    af.location  = APPLY_INT;
    af.modifier  = -5;
    af.bitvector = AFF_MELD;
    affect_to_char( victim, &af );

   /* Set up and adjust damage... */

    spell_CDB->damage = dice(6, level);

   /* Roasting time... */

    apply_damage( spell_CDB);

   /* Theatrics... */

    act( "$N has been Mind Melded.", ch, NULL, victim, TO_CHAR    );
    send_to_char( "You feel an immense pain in your head!\r\n", victim );
    act( "$N grimaces in pain!", ch, NULL, victim, TO_NOTVICT );

    return;
}


void spell_chaos(int sn, int level, CHAR_DATA *ch, void *vo) {
CHAR_DATA *victim = (CHAR_DATA *) vo;
CHAR_DATA *tmp_vict,*last_vict,*next_vict;
bool found;
int dam;
int damtype;

    damtype = number_range (4, 17);
    act("A cracking sound fills the room as $n bursts into energy, sending an arc of pure magical force into $N.", ch,NULL,victim,TO_ROOM);

    if (IS_NPC(victim)) {
	sprintf_to_char(ch, "With a crack, your %s hits %s!\r\n", flag_string(damage_type, damtype), victim->short_descr );

    } else {
                sprintf_to_char(ch, "With a crack, your %s hits %s!\r\n", flag_string(damage_type, damtype), victim->name );
	sprintf_to_char(victim, "You are hit by %s's %s!\r\n", ch->name, flag_string( damage_type, damtype ) );
    }

    dam = dice(level, 6);

    spell_CDB->atk_dam_type = damtype;
    spell_CDB->atk_blk_type = BLOCK_ABSORB;
    spell_CDB->atk_desc = "power";

   /* Set up and adjust damage... */

    spell_CDB->damage = dam;

    if ( saves_spell( level, spell_CDB->defender )) spell_CDB->damage /= 3;

   /* Charging time... */

    if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
        SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
        apply_damage( spell_CDB);  
        REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
    } else {
        apply_damage( spell_CDB);
    }

    last_vict = victim;
    level -= 3;

    while (level > 0) {
        found = FALSE;
        for (tmp_vict = ch->in_room->people; tmp_vict; tmp_vict = next_vict)  {
          next_vict = tmp_vict->next_in_room;

          if (tmp_vict == ch) continue;

          if (!is_safe_spell(ch,tmp_vict,TRUE) && tmp_vict != last_vict) {
            found = TRUE;
            last_vict = tmp_vict;
            act("$n is enveloped by the raw power!",tmp_vict,NULL,NULL,TO_ROOM);

            dam = dice(level,6);
            if (saves_spell(level,tmp_vict)) dam /= 3;

            damtype = number_range (4, 17);

            if (IS_NPC(tmp_vict)) {
	sprintf_to_char(ch, "Your %s hits %s!\r\n",  flag_string( damage_type, damtype ), tmp_vict->short_descr );
            } else {
	sprintf_to_char(ch, "Your %s hits %s!\r\n", flag_string( damage_type, damtype ), tmp_vict->name );
	sprintf_to_char(tmp_vict, "You are hit by %s's %s!\r\n", ch->name, flag_string( damage_type, damtype ) );
             }

            spell_CDB = getCDB(ch, tmp_vict, sn, FALSE, sn, 0);

            spell_CDB->atk_dam_type = damtype;
            spell_CDB->atk_blk_type = BLOCK_ABSORB;
            spell_CDB->atk_desc = "power";

            if (!check_spell_hit(spell_CDB, spell_roll)) continue;

           /* Set up and adjust damage... */

            spell_CDB->damage = dam;

            if ( saves_spell( level, spell_CDB->defender )) spell_CDB->damage /= 2;

            if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
                  SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
                  apply_damage( spell_CDB);  
                  REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
            } else {
                  apply_damage( spell_CDB);
            }

            level -= 4;
          }
        }

        if (!found) {
          if (ch == NULL) return;

          if (last_vict == ch) {
            act("The energy flows back into the ether.",ch,NULL,NULL,TO_ROOM);
            act("The energy returns to you, somewhat spent.", ch,NULL,NULL,TO_CHAR);
            return;
          }

          last_vict = ch;
          act("The magic backfires on $n...whoops!",ch,NULL,NULL,TO_ROOM);
          damtype = number_range (4, 17);
          sprintf_to_char(ch, "Your %s hits YOU!\r\n", flag_string(damage_type, damtype ) );

          dam = dice(level,6);
          if (saves_spell(level,ch)) dam /= 3;

          env_damage(ch, dam, damtype, "@v2 is fried by his own power! [@n0]{x\r\n", "Your power hurts you! [@n0]{x\r\n" );

          level -= 4;
          if (ch == NULL)  return;
        }
    }
    return;
}


void spell_psi_twister( int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA *vch;
CHAR_DATA *vch_next;
int chance = 150;    

    send_to_char( "You release your mental powers to do its will in the room!\r\n", ch );
    act( "$n levitates amid a swirl of psychic energy.", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch; vch = vch_next )   {
        vch_next        = vch->next;

        if ( vch->in_room == NULL )  continue;

        if ( vch->in_room == ch->in_room 
        && chance > number_percent())    {
             if (vch != ch && !is_safe_spell(ch,vch,TRUE)) {

                  spell_CDB = getCDB(ch, vch, sn, FALSE, sn, 0);

                  spell_CDB->atk_dam_type = DAM_MENTAL;
                  spell_CDB->atk_blk_type = BLOCK_ABSORB;
                  spell_CDB->atk_desc = "power";

                   if (!check_spell_hit(spell_CDB, spell_roll)) continue;

                   spell_CDB->damage = level + dice(3, 8);

                   if (saves_spell( level, spell_CDB->defender )) spell_CDB->damage /= 2;

                   if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
                         SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
                         apply_damage( spell_CDB);  
                         REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
                   } else {
                         apply_damage( spell_CDB);
                   }
             }
             chance /= 2;
        }
    }

    return;
}


void do_brew (CHAR_DATA *ch, char *argument) {
    OBJ_DATA *obj = NULL;
    OBJ_DATA *obj2 = NULL;
    int sn;
    char messbuf[128];
    char typebuf[128];
    int chance;
    int mana;
    char arg1[MAX_INPUT_LENGTH];
    char *spell;
    int spn;
    int efn;
    int bsn, bskill;

    one_argument (argument, arg1);

    if ( arg1[0] == '\0' ) {
	send_to_char( "Brew what?\r\n", ch );
	return;
    }

    bsn = get_skill_sn("brew");

    if ( bsn < 1 ) {
      send_to_char("No one knows how to brew potions...\r\n", ch);
      return;
    }

    bskill = get_skill(ch, bsn);

    if ( bskill < 1 ) {
      send_to_char("You have no idea how to do that!\r\n", ch);
      return;
    } 

     obj2 = ch->carrying;

     while(obj2 != NULL
     && (obj2->item_type != ITEM_POTION
          || !can_see_obj(ch, obj2)
          || obj2->value[1] != 0
          || obj2->value[2] != 0
          || obj2->value[3] != 0)) {
             obj2 = obj2->next_content;
    }

    if (!obj2) {
      send_to_char("You have no empty vial!\r\n", ch);
      return;
    } 
    
    spn = get_spell_spn(arg1);

    if ( !valid_spell(spn) ) {
      send_to_char("That's not a valid spell!\r\n", ch);
      return;
    }

    if (spell_array[spn].discipline > 0) {
      send_to_char("You can't brew this spell!\r\n", ch);
      return;
    }

    sn = spell_array[spn].sn;

    if ( !isSkillAvailable(ch, sn) 
      || get_skill(ch,sn) < 1 ) {
	send_to_char( "You don't know any spells of that name.\r\n", ch );
	return;
    }

    efn = spell_array[spn].effect; 

    if ( !valid_effect(efn) ) {
      send_to_char("That spell cannot be bottled!\r\n", ch);
      return;
    }

    mana = (3 * mana_cost(ch, spn))/2;
 
    if (ch->mana < mana) {
	send_to_char ("You do not have enough mana to brew that.\r\n",ch);
	return;
    }

    if ( spell_array[spn].component != NULL ) {
 
        obj = get_obj_carry (ch, spell_array[spn].component);

        if (obj == NULL) {
	    sprintf (messbuf, "You need '%s' to brew that potion.\r\n", spell_array[spn].component);
	    send_to_char (messbuf, ch);
	    return;
	}
    }
 
   /* Ok, checks done, lets whip up a potion! */

    ch->mana -= mana;
    if (obj != NULL) extract_obj (obj);
    if (obj2 != NULL) extract_obj (obj2);

    chance = bskill + number_open();	

    if ( chance < 100 ) { 
      send_to_char ("Oh-oh, that didn't work...\r\n",ch);
      return;
    }
	
    chance += get_skill(ch, sn) + number_open() - 100;	

    obj 		= create_object ( (get_obj_index(OBJ_POTION)), 0);
    obj->value[0] 	= ch->level/2;

    if ( chance > 110 ) { 
      obj->value[1] 	= efn;
    } else {
      obj->value[1]	= get_effect_efn("poison");
    }

    obj->value[2]	= EFFECT_UNDEFINED;
    obj->value[3]	= EFFECT_UNDEFINED;
 
    obj->level    	= 1;
    obj->cost = 0;
    obj->timer	  	= UMAX(24, (ch->level*2));

    free_string(obj->name);
    free_string(obj->short_descr);
    free_string(obj->description);
 		
    spell = spell_array[spn].name;
	
    sprintf (typebuf, "potion %s", spell);
    obj->name = strdup (typebuf);
    sprintf (typebuf, "a potion of %s", spell);
    obj->short_descr = strdup (typebuf);
    sprintf (typebuf, "A potion of %s is just lying here.", spell);
    obj->description = strdup (typebuf);
    obj_to_room (obj, ch->in_room);
    send_to_char ("You whip up a fantastic concoction!\r\n",ch);
    act( "$n grabs a kettle and whips up a potion.", ch, NULL, NULL, TO_ROOM );
    return;
}
		
void do_scribe (CHAR_DATA *ch, char *argument)	{
        OBJ_DATA *obj = NULL;
        OBJ_DATA *obj2 = NULL;
        int sn;
	char messbuf[MAX_STRING_LENGTH];
	char typebuf[MAX_STRING_LENGTH];
	int chance;
	char arg1[MAX_INPUT_LENGTH];
	char *spell;
	int gold=0;
	int mana=0;
        int spn;
	int efn;
        int bsn, bskill;

	one_argument (argument, arg1);


    if ( arg1[0] == '\0' )    {
	send_to_char( "Scribe what?\r\n", ch );
	return;
    }

    bsn = get_skill_sn("scribe");

    if ( bsn < 1 ) {
      send_to_char("No one knows how to scribe scrolls...\r\n", ch);
      return;
    }

    bskill = get_skill(ch, bsn);

    if ( bskill < 1 ) {
      send_to_char("You have no idea how to do that!\r\n", ch);
      return;
    } 

    spn = get_spell_spn(arg1);

    if ( !valid_spell(spn) ) {
      send_to_char("That's not a valid spell!\r\n", ch);
      return;
    }

    if (spell_array[spn].discipline > 0) {
      send_to_char("You can't scribe this spell!\r\n", ch);
      return;
    }

    sn = spell_array[spn].sn;

    if ( !isSkillAvailable(ch, sn) 
      || get_skill(ch,sn) < 1 ) {
	send_to_char( "You don't know any spells of that name.\r\n", ch );
	return;
    }

    efn = spell_array[spn].effect; 

    if ( !valid_effect(efn) ) {
      send_to_char("That spell cannot be bottled!\r\n", ch);
      return;
    }

    mana = (3 * mana_cost(ch, sn))/2;
		
    gold = (77 * mana)/10;

/* Verify character has necessary gold, mana, and components. */

    if (ch->gold[0] < gold) {
	send_to_char ("You do not have enough {rCopper{x to scribe that.\r\n",ch);
	return;
    }

    if (ch->mana < mana) {
	send_to_char ("You do not have enough mana to scribe that.\r\n",ch);
	return;
    }

    obj = get_obj_carry (ch, "vellum");

    if ( obj == NULL) {
	send_to_char ("You need vellum to write on.\r\n",ch);
	return;
    }

    if ( spell_array[spn].component != NULL ) {
         obj2 = get_obj_carry (ch, spell_array[spn].component);

        if (obj2 == NULL) {
	    sprintf (messbuf, "You need '%s' to scribe that spell.\r\n", spell_array[spn].component);
	    send_to_char (messbuf, ch);
	    return;
	}
    }

/* Ok, checks done, lets whip up a potion! */

    ch->gold[0] -= gold;
    ch->mana -= mana;
    extract_obj (obj);
    if (obj2!=NULL) extract_obj (obj2);

    chance = bskill + number_open();	

    if ( chance < 100 ) { 
      send_to_char ("Oh-oh, that didn't work...\r\n",ch);
      return;
    }
	
    chance += get_skill(ch, sn) + number_open() - 100;	

    obj = create_object ( (get_obj_index(OBJ_SCROLL)), 0);
    obj->value[0] = ch->level/2;

    if ( chance > 110 ) { 
      obj->value[1] 	= efn;
    } else {
      switch (number_bits(2)) {
        default:
          obj->value[1]	= get_effect_efn("fear");
          break;
        case 1:
          obj->value[1]	= get_effect_efn("weaken");
          break;
        case 2:
          obj->value[1]	= get_effect_efn("curse");
          break;
        case 3:
          obj->value[1]	= get_effect_efn("faerie fire");
          break;
      }
    }
 
    obj->value[2]	= EFFECT_UNDEFINED;
    obj->value[3]	= EFFECT_UNDEFINED;
 
    obj->level    = 1;
    obj->cost = 0;
    obj->timer	  = UMAX(24, (ch->level*2));
    free_string (obj->name);
    free_string (obj->short_descr);
    free_string (obj->description);
		
    spell = spell_array[spn].name;
	
    sprintf (typebuf, "scroll %s", spell);
    obj->name = strdup (typebuf);
    sprintf (typebuf, "a scroll of %s", spell);
    obj->short_descr = strdup (typebuf);
    sprintf (typebuf, "A tightly rolled scroll of %s is lying here.", spell);
    obj->description = strdup (typebuf);
   
    obj_to_room(obj, ch->in_room);
    send_to_char("You carefully fill the scroll with mystical runes!\r\n",ch);
    act( "$n writes a scroll with strange symbols on it.", ch, NULL, NULL, TO_ROOM );
    return;
}

		
void spell_gnawing_hunger( int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( saves_spell( level, victim )) return;

   /* Now, consume a lot of the victims food... */ 

    gain_condition(victim, COND_FOOD, -1 * number_range(1, ch->level) * 27);

   /* This consumes up to 1 hours food for every level of the caster,
    *   averaging at half thier level but with a flat distribution.
    * A typical player can lose 150+ hours worth before they die, but will
    *   start to suffer after losing 20-30 hours worth. 
    */

   /* Notify... */

    send_to_char("{YYou feel a terrible, gnawing hunger!{x\r\n", victim );
    act("$n stomach growls!",victim,NULL,NULL,TO_ROOM);

   /* All done... */
 
    return;
}


void spell_burning_thirst( int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA *victim = (CHAR_DATA *) vo;

   /* They do get a save... */

    if ( saves_spell( level, victim )) return;

   /* Now, consume a lot of the victims fluids... */ 

    if (IS_SET(victim->act, ACT_VAMPIRE)) {
           if (!IS_NPC(victim)) {
                 if (victim->pcdata->blood >1) {
                        victim->pcdata->blood -= level / 2;
                        victim->pcdata->blood = UMAX(victim->pcdata->blood, 1);
                 }
           }
    } else {
           gain_condition(victim, COND_DRINK, -1 * number_range(1, ch->level) * 13);
    }    


   /* This consumes up to 1/2 hour drink for every level of the caster,
    *   averaging at half thier level but with a flat distribution.
    * A typical player can lose 75+ hours worth before they die, but will
    *   start to suffer after losing 10-50 hours worth. 
    */

   /* Notify... */

    send_to_char("{YYou feel a terrible, burning thirst!{x\r\n", victim );
    act("$n croaks something inaudible!",victim,NULL,NULL,TO_ROOM);

   /* All done... */
 
    return;
}


void spell_free_grog( int sn, int level, CHAR_DATA *ch, void *vo ) {
 CHAR_DATA *victim = (CHAR_DATA *) vo;

   /* They do get a save... */

    if ( saves_spell( level, victim ) )
	return;

   /* Now, give them a lot of alcohol... */ 

    gain_condition(victim, COND_DRUNK, 2 + number_range(1, ch->level/5) );

   /* This gives from 3 to 3 + (casters level)/5 intoxication
    *   points to the character.
    * A typical player will pass out when they have 20 or more intoxication
    *   points, but start to suffer after gaining 12 points. 
    */

   /* Notify... */

    send_to_char("{YYou feel a little dizzy!{x\r\n", victim );
    act("$n staggers and sways!",victim,NULL,NULL,TO_ROOM);

   /* All done... */
 
    return;
}


void spell_ghastly_sobriety( int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA *victim = (CHAR_DATA *) vo;

   /* They do get a save... */

    if ( saves_spell( level, victim ) )
	return;

   /* Now, remove all thier alcohol... */ 

    gain_condition(victim, COND_DRUNK, -1 * victim->condition[COND_DRUNK]);

   /* This removes all alcohol from the victim and will give them a hangover
    *   affect if they had more than 11 points.
    */

   /* Notify... */

    send_to_char("{CYou feel a an icy chill run through your body!{x\r\n",
                                                                    victim );
    act("$n grunts and moans!",victim,NULL,NULL,TO_ROOM);

   /* All done... */
 
    return;
}

void spell_burden_of_blubber( int sn, int level, CHAR_DATA *ch, void *vo ) {

    CHAR_DATA *victim = (CHAR_DATA *) vo;

   /* They do get a save... */

    if ( saves_spell( level, victim ) )
	return;

   /* Now, give them a lot of fat... */ 

    gain_condition(victim, COND_FAT, 10 + number_range(1, ch->level/3) );

   /* This gives the character from 1 to casters level/3 plus 10 fat points.
    * While this won't kill them, it will reduce STR, CON and move, by 1
    * point for every 10 fat points gained. 
    */

   /* Notify... */

    send_to_char("{CYou feel your body swell and distend!{x\r\n",
                                                                    victim );
    act("$n seems to inflate!",victim,NULL,NULL,TO_ROOM);

   /* All done... */
 
    return;
}


void spell_slender_lines( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;


    if (saves_spell( level, victim )
    && victim != ch) return;

    gain_condition(victim, COND_FAT, -10 - number_range(1, level/3) );

   /* This removes fat from the caster.  The gain-condition stops negative 
    * fat from occuring.  Every 10 points removed, removes a fat effect.
    */

   /* Notify... */

    send_to_char("{CYou feel smaller and sleeker!{x\r\n", victim );
    act("$n looks a lot thinner!",victim,NULL,NULL,TO_ROOM);
    return;
}


void spell_mana_transfer( int sn, int level, CHAR_DATA *ch, void *vo) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int transmana;

    if (ch == victim)    {
	send_to_char( "That does not make any sense.\r\n", ch );
	return;
    }

    if ( ch->move < 100 )    {
	send_to_char( "You are too exhausted to do that.\r\n", ch );
	return;
    }
    ch->move = ch->move - 100;
    transmana=ch->mana/5;
    ch->mana -=transmana;
    victim->mana +=transmana;
    if (victim->mana > victim->max_mana) victim->mana=victim->max_mana;
    act("You draw in energy from around you and channel it into $N.", ch, NULL, victim, TO_CHAR);
    act("$n draws in energy and channels it into $N.", ch, NULL, victim, TO_NOTVICT);
    act("$n draws in energy and channels it into you.", ch, NULL, victim, TO_VICT);
    
     return;
}


void spell_consecrate_doll(int sn, int level, CHAR_DATA *ch, void *vo)	{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *hair; 
    OBJ_DATA *doll; 
    char     part1 [MAX_INPUT_LENGTH];
    char     part2 [MAX_INPUT_LENGTH];
    char     buf [MAX_INPUT_LENGTH];
    int power;
    
    if (victim==NULL) {
         send_to_char ("No victim.\r\n",ch);
         return;
    }  

     if ( ( hair = get_eq_char( ch, WEAR_HOLD ) ) == NULL ) {
         send_to_char ("You have no hair in your hands.\r\n",ch);
         return;
    }  

    if (hair->pIndexData->vnum !=OBJ_VNUM_HAIR )	{
         send_to_char ("That's useless to createa voodoo doll.\r\n",ch);
         return;
    }

   sprintf(part1,hair->name);
   sprintf(part2,"hair streak %s",victim->name);

    if ( str_cmp(part1,part2) )    {
          sprintf(buf,"But you are holding %s, not %s!\r\n",part1,part2);
          send_to_char( buf, ch );
          return;
    }

    act("{M$p vanishes from your hand in a puff of smoke.{x", ch, hair, NULL, TO_CHAR);
    act("{M$p vanishes from $n's hand in a puff of smoke.{x", ch, hair, NULL, TO_ROOM);
    obj_from_char(hair);
    extract_obj(hair);

    doll = create_object( get_obj_index( OBJ_VNUM_VOODOO_DOLL ), 0 );

    sprintf(buf,"%s voodoo doll",victim->name);
    free_string(doll->name);
    doll->name=str_dup(buf);

    sprintf(buf,"a voodoo doll of %s",victim->name);
    free_string(doll->short_descr);
    doll->short_descr=str_dup(buf);

    sprintf(buf,"A voodoo doll of %s lies here.",victim->name);
    free_string(doll->description);
    doll->description=str_dup(buf);

    power=ch->level/30;
    if (check_skill(ch, gsn_voodoo, 0)) power +=10;
    if (check_skill(ch, gsn_voodoo, 0)) power +=10;
    if (check_skill(ch, gsn_voodoo, 0)) power +=10;
    if (check_skill(ch, gsn_voodoo, 0)) power +=10;
    if (check_skill(ch, gsn_voodoo, 0)) power +=10;

    doll->value[0]=power;

    obj_to_char(doll,ch);
    equip_char(ch,doll,WEAR_HOLD);

    act("{M$p appears in your hand.{x", ch, doll, NULL, TO_CHAR);
    act("{M$p appears in $n's hand.{x", ch, doll, NULL, TO_ROOM);

     return;
}   

void spell_lich( int sn, int level, CHAR_DATA *ch, void *vo ) {

if (ch->alignment > -500) {
          send_to_char("You decide, you won't do such a terrible thing!\r\n", ch );
          return;
}

if (IS_SET(ch->act,ACT_UNDEAD)) {
          send_to_char("You are already undead!\r\n", ch );
          return;
}

if (get_divinity(ch) < DIV_HERO) {
          send_to_char("You lack the power to do that!\r\n", ch );
          return;
}

if ((get_curr_stat(ch, STAT_WIS)+level)/2 < number_percent()) {
          send_to_char("You die!\r\n", ch );
          act("{r$n dies!{x", ch, NULL, NULL, TO_ROOM);
          ch->hit = 0;
          set_activity( ch, POS_DEAD, NULL, ACV_NONE, NULL);
          raw_kill(ch, TRUE);
          return;
}

send_to_char("You awake as something very different!\r\n", ch );
act("{W$n awakes as something very different!{x", ch, NULL, NULL, TO_ROOM);
SET_BIT(ch->act,ACT_UNDEAD);
return;
}


void spell_wrath_cthuga( int sn, int level, CHAR_DATA *ch, void *vo){
    OBJ_DATA *obj;

     if ( (get_eq_char( ch, WEAR_HOLD ) ) != NULL ) {
         send_to_char ("You must have your hands free to cast that spell.\r\n",ch);
         return;
    }  
     obj = create_object (get_obj_index (OBJ_VNUM_WRATH), 0);
     obj->cost=0;
     obj->level= level;
     obj->value[1]=7+(level/3);
     obj->value[2]=7+(level/3);
     obj_to_char(obj,ch);
     obj->timer = number_range(2,5);
     equip_char(ch,obj,WEAR_HOLD);
     act ("$n mutters some words and $p materializes from thin air.",ch,obj,NULL,TO_ROOM);
     send_to_char("A ball of fire materializes in your hand.\r\n",ch);
     return;

}


void spell_aura ( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    victim=ch;

    if ( IS_AFFECTED(victim, AFF_AURA))    {
         send_to_char("You already show your aura.\r\n",ch);
         return;
    }

    if ( victim->alignment <600
    && victim->alignment > -600)    {
         send_to_char("There is no aura of neutrality.\r\n",ch);
         return;
    }

    af.type      = sn;
    af.afn 	 = gafn_aura;
    af.level     = level;
    af.duration  = level/4;
    af.location  = APPLY_AC;
    af.modifier  = -level/10;
    af.bitvector = AFF_AURA;
    affect_to_char( victim, &af );
    if (victim->alignment>0) {
         send_to_char( "You reveal your good aura.\r\n", victim );
    } else {
         send_to_char( "You reveal your evil aura.\r\n", victim );
    }

    return;
}


void spell_hallucinate( int sn, int level, CHAR_DATA *ch, void *vo) {
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;

    if ( saves_spell( level, victim )
    && victim != ch) {
	act("$ns eyes turn wide, but it passes.",victim,NULL,NULL,TO_ROOM);
	send_to_char("You feel a switching in your vision, but then it passes.\r\n",victim);
	return;
    }

    af.type      = sn;
    af.afn	 = gafn_hallucinating;
    af.level     = level;
    af.duration  = 1 + (level/20);
    af.location  = APPLY_DEX;
    af.modifier  = -1 * dice(3,5);
    af.bitvector = AFF_HALLUCINATING;
    affect_join( victim, &af );

    send_to_char( "What a trippy feeling...\r\n", victim );

    act("$n looks amazed.",victim,NULL,NULL,TO_ROOM);

    return;
}


void spell_silence (int sn, int level, CHAR_DATA *ch, void *vo) {
AFFECT_DATA af;

	if (IS_RAFFECTED(ch->in_room, RAFF_SILENCE))	{
		send_to_char ("This room is already silent.\r\n",ch);
		return;
		}
	
	af.type        = sn;
                af.afn	    = gafn_room_silence; 
   	af.level     = level;
	af.duration  = level/7;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = RAFF_SILENCE;
	affect_to_room(ch->in_room, &af );
	send_to_char ("The room get silent..\r\n",ch);
}
	

void spell_darkness (int sn, int level, CHAR_DATA *ch, void *vo) {
AFFECT_DATA af;

	if (IS_RAFFECTED(ch->in_room, RAFF_DARKNESS))	{
		send_to_char ("This room is already dark.\r\n",ch);
		return;
		}
	
	af.type        = sn;
                af.afn	    = gafn_room_darkness; 
   	af.level     = level;
	af.duration  = level/5;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = RAFF_DARKNESS;
	affect_to_room(ch->in_room, &af );
	send_to_char ("The room get dark..\r\n",ch);
}


void spell_relax( int sn, int level, CHAR_DATA *ch, void *vo) {
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;

    if (IS_AFFECTED(victim, AFF_RELAXED)) {
	send_to_char ("Your victim is already relaxed.\r\n",ch);
	return;
    }
      
 
    af.type      = sn;
    af.afn	 = gafn_relaxed;
    af.level     = level;
    af.duration  = 1 + (level/20);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_RELAXED;
    affect_join( victim, &af );

    send_to_char( "Wow, that feels good...\r\n", victim );
    act("$n looks very relaxed.",victim,NULL,NULL,TO_ROOM);

    return;
}


void spell_cause_death( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int chance;
  
    act("You invoke death upon $N.",ch,NULL,victim,TO_CHAR);
    act("$n invokes death upon $N.",ch,NULL,victim,TO_ROOM);


     chance = ((ch->level - victim->level -10)*10 + number_percent())/5;
     if (number_percent() > chance) {
          send_to_char( "... but nothing happens.\r\n", victim );
     } else {
          spell_CDB->atk_dam_type = DAM_HARM;
          spell_CDB->atk_blk_type = BLOCK_ABSORB;

   /* Bail out if it misses... */
 
           if (!check_spell_hit(spell_CDB, spell_roll)) return;
     
   /* Set up and adjust damage... */

           spell_CDB->damage = (dice(2, 2) * ch->max_hit)/4;
           if ( saves_spell( level, spell_CDB->defender ) ) spell_CDB->damage /= 2;

   /* Apply the damage... */
           send_to_char( "Rest in Peace!\r\n", victim );
           act("$n becomes very pale.",victim,NULL,NULL,TO_ROOM);
           SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
           apply_damage( spell_CDB);  
           REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
    }
    return;
}


void spell_protection_fire (int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
	
    if (IS_SET(ch->imm_flags, IMM_FIRE)) {
	send_to_char ("That target is already protected from fire.\r\n",ch);
	return;
    }
	
    af.type      = sn;
    af.afn 	 = gafn_protection_fire;
    af.level	 = level;
    af.duration  = level/4;
    af.location  = APPLY_IMMUNITY;
    af.modifier  = IMMAPP_FIRE; 
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char( "You don't feel the heat...\r\n", victim );
    return;
}


void spell_protection_frost (int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
	
    if (IS_SET(ch->imm_flags, IMM_COLD)) {
	send_to_char ("That target is already protected from fire.\r\n",ch);
	return;
    }
	
    af.type      = sn;
    af.afn 	 = gafn_protection_frost;
    af.level	 = level;
    af.duration  = level/4;
    af.location  = APPLY_IMMUNITY;
    af.modifier  = IMMAPP_COLD; 
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char( "You don't feel the cold...\r\n", victim );
    return;
}

void spell_protection_lightning (int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
	
    if (IS_SET(ch->imm_flags, IMM_COLD)) {
	send_to_char ("That target is already protected from fire.\r\n",ch);
	return;
    }
	
    af.type      = sn;
    af.afn 	 = gafn_protection_lightning;
    af.level	 = level;
    af.duration  = level/4;
    af.location  = APPLY_IMMUNITY;
    af.modifier  = IMMAPP_LIGHTNING; 
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char( "You feel grounded...\r\n", victim );
    return;
}


void spell_restore_limb( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    int tolimb=0;
    int lc;

   for (lc=1; lc<6; lc++) {
       if (victim->limb[lc] > victim->limb[tolimb]) tolimb=lc;
   }

   victim->limb[tolimb] -=number_range(0,3);
   victim->limb[tolimb] = UMAX(0, victim->limb[tolimb]);

   switch(tolimb) {
       case 0: sprintf(buf,"Your head feels better.\r\n"); break;
       case 1: sprintf(buf,"Your body feels better.\r\n"); break;
       case 2: sprintf(buf,"Your left arm feels better.\r\n"); break;
       case 3: sprintf(buf,"Your right arm feels better.\r\n"); break;
       case 4: sprintf(buf,"Your legs feel better.\r\n"); break;
       default: break;
   }

   update_wounds(ch);
   send_to_char( buf, victim );
    if ( ch != victim ) send_to_char( "Ok.\r\n", ch );
    return;
}


void spell_cause_riot( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *vch;
    int chance;

    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
         if (IS_NPC(vch)
         && !IS_SET(vch->imm_flags, IMM_MAGIC)
         && !IS_SET(vch->act, ACT_UNDEAD)
         && !IS_SET(vch->act, ACT_INVADER)
         && !IS_SET(vch->act, ACT_IS_HEALER)
         && !IS_SET(vch->act, ACT_IS_ALIENIST)
         && !IS_SET(vch->act, ACT_PRACTICE)
         && !IS_AFFECTED(vch, AFF_CHARM)
         && !IS_AFFECTED(vch, AFF_CALM)) {
                if (number_percent() < 35) {
                    chance = (level - vch->level + 5)*7;
                    if (number_percent() < chance) {
                         SET_BIT(vch->act, ACT_INVADER);
                         REMOVE_BIT(vch->act, ACT_SENTINEL);
                   }
                }
         }
    }

    act("Everything becomes very chaotic....",ch,NULL,NULL,TO_CHAR);
    act("Everything becomes very chaotic...",ch,NULL,NULL,TO_ROOM);
    return;
}


void spell_trance (int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SET(victim->act, ACT_UNDEAD)
    || IS_SET(victim->act, ACT_DREAM_NATIVE)) {
	send_to_char ("You are immune.\r\n",ch);
	return;
    }

    if (IS_AFFECTED(victim, AFF_CALM)) {
	send_to_char ("You don't feel anything special.\r\n",ch);
	return;
    }

    af.type         = sn;
    af.level         = level;
    af.afn 	         = 0;
    af.duration  = level/10+5;
    af.location  = APPLY_SKILL;
    af.skill          = gsn_dreaming;
    af.modifier  = 30; 
    af.bitvector = AFF_CALM;
    affect_to_char( victim, &af );

    act("You feel detached from this world.",ch,NULL,NULL,TO_CHAR);
    act("$n gets a really silly look in the face.",ch,NULL,NULL,TO_ROOM);
    return;
}


void spell_personalize_portal( int sn, int level, CHAR_DATA *ch, void *vo ) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    char * lastarg;

    if (obj->item_type != ITEM_PORTAL)    {
	send_to_char("That isn't a portal.\r\n",ch);
	return;
    }

    if (obj->value[4] != PORTAL_MAGIC)    {
	send_to_char("That isn't a portal.\r\n",ch);
	return;
    }

    lastarg = check_obj_owner(obj);
    if (strcmp(lastarg, "not found")) {
         send_to_char ("This portal is already personalized!\r\n", ch);
         return;
    }      

    sprintf(buf, "%s _plr: %s", obj->name, ch->name);
    free_string(obj->name);
    obj->name = strdup(buf);

    act("$p will now recognize you.", ch, obj, NULL,TO_CHAR);
    act("$p suddenly seems very alive.", ch, obj, NULL, TO_ROOM);
    return;
}


void spell_slow( int sn, int level, CHAR_DATA *ch, void *vo) {
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;

    if (IS_AFFECTED(victim, AFF_SLOW)) {
	send_to_char ("Your victim is already slowed.\r\n",ch);
	return;
    }
 
    if (IS_AFFECTED(victim, AFF_HASTE)) {
           if (check_dispel(level,victim, gafn_haste)) act("$n is no longer moving so fast.",victim,NULL,NULL,TO_ROOM);
    } else {
        af.type      = sn;
        af.afn	 = gafn_slow;
        af.level     = level;
        af.duration  = 1 + (level/12);
        af.location  = APPLY_DEX;
        af.modifier  = -1 - ((level >= 18) + (level >= 25) + (level >= 32) + (level >= 50) + (level >= 75) + (level >= 100) + (level >= 125));
        af.bitvector = AFF_SLOW;
        affect_join( victim, &af );
    }
    send_to_char( "You feel yourself slowing down.\r\n", victim );
    act("$ns moves slow down.",victim, NULL, NULL, TO_ROOM);
    send_to_char( "Ok.\r\n", ch );
    return;
}


void spell_globe_of_protection( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(ch, AFF_GLOBE)) {
       send_to_char("You are already surrounded by a globe of protection.\r\n",ch); 
       return;
    }

    af.type      = sn;
    af.afn	      = gafn_globe;
    af.level     = level;
    af.duration  = 1+level/5;
    af.location  = APPLY_AC;
    af.modifier  = -1 * (30 + ch->level );
    af.bitvector = AFF_GLOBE;
    affect_to_char( victim, &af );

      act( "{m$n is surrounded by a globe of protection.{x", victim, NULL, NULL, TO_ROOM );
      send_to_char( "{mYou are surrounded by a globe of protection.{x\r\n", victim );
       return;
}


void spell_personalize_weapon( int sn, int level, CHAR_DATA *ch, void *vo ) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    int chance;

    if (obj->item_type != ITEM_WEAPON)    {
	send_to_char("That isn't a weapon.\r\n",ch);
	return;
    }

    if (obj->wear_loc != -1)    {
	send_to_char("The item must be carried to be enchanted.\r\n",ch);
	return;
    }

    if (obj->owner != NULL) {
	send_to_char("This weapon already got an owner.\r\n",ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_POLY) || is_affected(ch, gafn_mask_self)) {
	send_to_char("You are not in your original form..\r\n",ch);
	return;
    }

    chance = level - obj->level + 22;  

    if (chance < number_range(0,10)) {
       sprintf(buf, "Your %s turns to dust!\r\n", obj->short_descr);
       send_to_char(buf, ch);
       obj_from_char(obj);
       extract_obj(obj);
       return;
    }  

    if (chance < number_percent()) {
        send_to_char("You fail.\r\n", ch);
        if (number_percent() < 20) {
             sprintf(buf, "Your %s turns to dust!\r\n", obj->short_descr);
             send_to_char(buf, ch);
             obj_from_char(obj);
             extract_obj(obj);
        }  
        return;
    }

    free_string(obj->owner);
    obj->owner = strdup(ch->short_descr);
    send_to_char("This weapon now belongs to you.\r\n",ch);
    return;
}


void spell_animate_weapon( int sn, int level, CHAR_DATA *ch, void *vo ) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    int chance;

    if (obj->item_type != ITEM_WEAPON)    {
	send_to_char("That isn't a weapon.\r\n",ch);
	return;
    }

    if (obj->wear_loc != -1)    {
	send_to_char("The item must be carried to be enchanted.\r\n",ch);
	return;
    }

    if (obj->owner == NULL) {
	send_to_char("This weapon is not personalized.\r\n",ch);
	return;
    }

    chance = level - obj->level + 22;  

    if (chance < number_percent()) {
        send_to_char("You fail.\r\n", ch);
        if (number_percent() < 20) {
             sprintf(buf, "Your %s turns to dust!\r\n", obj->short_descr);
             send_to_char(buf, ch);
             obj_from_char(obj);
             extract_obj(obj);
        }  
        return;
    }

    SET_BIT(obj->extra_flags, ITEM_ANIMATED);
    send_to_char("The weapon slowly begins to move in your hands.\r\n",ch);
    return;
}


void spell_pestillence( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    AFFECT_DATA af;

    send_to_char( "You exhale death and decay!\r\n", ch );
    act( "$n exhales a foul stench.", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )    {
	vch_next = vch->next;
	if ( vch->in_room == NULL ) continue;
                if (number_percent() > 15) continue;
               
              	if ( vch->in_room->area->zone == ch->in_room->area->zone) {
   	      if ( vch->in_room->area == ch->in_room->area
                      && vch != ch
                      && !is_safe_spell(ch, vch, TRUE)) {
                           if (saves_spell(level,vch) || IS_SET(vch->act, ACT_UNDEAD))    {
	                 send_to_char("You feel momentarily ill, but it passes.\r\n",vch);
	                 act("$N seems to be unaffected.",vch, NULL, NULL,TO_ROOM);
	                 continue;
                           }

                           af.type 	  = sn;
                           af.afn	  = gafn_plague;
                           af.level	  = level * 3/4;
                           af.duration   = level;
                           af.location   = APPLY_STR;
                           af.modifier   = -5; 
                           af.bitvector  = AFF_PLAGUE;
                           affect_join(vch, &af);
                           send_to_char("You scream in agony as plague sores erupt from your skin.\r\n",vch);
                           act("$n screams in agony as plague sores erupt from $s skin.", vch, NULL, NULL,TO_ROOM);
                      } else {
    	           send_to_char( "There is a foul smell in the air.\r\n", vch );
                      }
                }
    }
    return;
}


void spell_bestow_blessing (int sn, int level, CHAR_DATA *ch, void *vo ) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if (obj->item_type == ITEM_DRINK_CON) {
        if (obj->value[4] > 0) {
	send_to_char("The liquid is already magical.\r\n",ch);
	return;
        }

        if (obj->value[1] == 0) {
	send_to_char("It's empty.\r\n",ch);
	return;
        }

        if (obj->value[3] == TRUE) {
                obj->value[3] = FALSE;
        } else {
                obj->value[4] = get_effect_efn("bless");
        }
        send_to_char( "The drink is now blessed.\r\n", ch );
        act( "$p begins to glow in a holy light.", ch, obj, NULL, TO_ROOM );

    } else if (obj->item_type == ITEM_FOUNTAIN) {

        if (obj->value[3] > 0) {
	send_to_char("This is a magic fountain.\r\n",ch);
	return;
        }

        obj->value[1] = FALSE;
        obj->value[2] = 1+ level / 2;
        obj->value[3] = get_effect_efn("bless");
        if (obj->timer > 0) {
            obj->timer = UMIN(obj->timer, 1+ level);
        } else {
            obj->timer = 1 + level;
        }

        send_to_char( "The fountain is now blessed.\r\n", ch );
        act( "$p begins to glow in a holy light.", ch, obj, NULL, TO_ROOM );
    } else {
        send_to_char("You can't bestow a blessing on that.\r\n",ch);
    }
    return;
}
  

void spell_metamorphosis( int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA *victim = (CHAR_DATA *)vo;
OBJ_DATA *clone = NULL;
AFFECT_DATA af;

     if (IS_SET(victim->act,ACT_WERE) ) {
              send_to_char ("You are immune against morphing\r\n",victim);
              return;
     }                

     if (IS_AFFECTED(victim, AFF_POLY) ) {
	send_to_char ("You are already masked.\r\n",victim);
	return;
     }

     if (victim->trans_obj == NULL) {
	send_to_char ("You have no special obj in mind.\r\n",victim);
	return;
     }

     if (victim->trans_obj->zmask != 0
     && (victim->trans_obj->zmask & zones[victim->in_zone].mask ) == 0 ) {
	send_to_char ("You have bad feeling about doing that here.\r\n",victim);
	return;
     }
	
    af.type      = sn;
    af.afn	 = gafn_morf;
    af.level     = level;
    af.duration  = level / 2 + 1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_MORF;
    affect_to_char( victim, &af );
    send_to_char( "You change your shape.\r\n", victim);

    clone = create_object(victim->trans_obj->pIndexData,0); 
    clone_object(victim->trans_obj, clone);
    victim->trans_obj = clone;
    clone->trans_char = victim;
    obj_to_room(clone, victim->in_room);
    
    act("$ns shape changes into the form of $p.", victim, clone, NULL,TO_ROOM);
    return;
}	

    
void spell_mortalize( int sn, int level, CHAR_DATA *ch, void *vo) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(victim, AFF_INCARNATED)
    || !IS_IMMORTAL(victim)) {
	send_to_char ("Your victim is already mortal.\r\n",ch);
	return;
    }

     if (!IS_NPC(victim)) {
         if (victim->desc->connected != 0 || victim->desc->editor != 0) {
             send_to_char( "They are beyond your might!\r\n", ch );
             return;
         }
    }

        af.type      = sn;
        af.afn           = gafn_incarnated;
        af.level     = level;
        af.duration  = 1 + (level/30);
        af.location  = APPLY_NONE;
        af.modifier  =  0;
        af.bitvector = AFF_INCARNATED;
        affect_join( victim, &af );

    send_to_char( "You suddenly feel vulnerable.\r\n", victim );
    act("$ns aura of godly power fades.",victim, NULL, NULL, TO_ROOM);
    return;
}


void spell_incognito( int sn, int level, CHAR_DATA *ch, void *vo )	{
CHAR_DATA *victim=(CHAR_DATA *)vo;
AFFECT_DATA af;

                if (IS_SET(ch->act,ACT_WERE) ) {
                       send_to_char ("You are immune against Camouflage!\r\n",ch);
                       return;
                 }                

	if ( IS_AFFECTED(ch, AFF_POLY)
                || IS_AFFECTED(ch, AFF_MORF)){
		send_to_char ("You are already masked.\r\n",ch);
		return;
	}

	if (victim == ch)	{
		send_to_char ("You must specify a target creature.\r\n",ch);
		return;
	}
	
	if (IS_MORTAL(ch) && !IS_NPC(victim))	{
		send_to_char ("You can only mask yourself to look like mobs.\r\n",ch);
		return;
	}

                if (!str_cmp(race_array[victim->race].name,"yithian")) {
		send_to_char ("You can' mask as a Yithian.\r\n",ch);
		return;
	}

                if (IS_SET(victim->imm_flags, IMM_MASK)) {
		send_to_char ("You can' mask as that.\r\n",ch);
		return;
	}

                if (IS_AFFECTED(victim, AFF_CHARM)) {
		send_to_char ("Wouldn't that confuse you?.\r\n",ch);
		return;
	}
		
    af.type      = sn;
    af.afn	 = gafn_mask_self;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_POLY;
    affect_to_char( ch, &af );
    send_to_char( "You reshape yourself into the likeness of your target.\r\n", ch );

	/*save old descriptions*/	
	free_string(ch->description_orig);
	free_string(ch->short_descr_orig);
	free_string(ch->long_descr_orig);
	ch->description_orig = str_dup (ch->description);
	ch->short_descr_orig = str_dup (ch->short_descr);
	ch->long_descr_orig  = str_dup (ch->long_descr);
                ch->race_orig=ch->race;

	/*apply poly descriptions*/
	free_string(ch->description);
	free_string(ch->short_descr);
	free_string(ch->long_descr);
	ch->description = str_dup (victim->description);
                ch->race=victim->race;

	if (IS_NPC(victim)) ch->short_descr = str_dup (victim->short_descr);
	else ch->short_descr = str_dup (victim->name);

	ch->long_descr  = str_dup (victim->long_descr);
	free_string(ch->poly_name);
	ch->poly_name = str_dup	(victim->name);
	ch->start_pos = victim->start_pos; 
     return;
}	

void spell_create_seed( int sn, int level, CHAR_DATA *ch, void *vo) {
    OBJ_DATA *obj;
   
    obj = create_object (get_obj_index (OBJ_VNUM_SEED), 0);
    obj->cost=0;
    obj_to_room (obj, ch->in_room);
    obj->value[0] = TREE_SEED;
    if (number_percent() > 30 + (150 - level)/2) obj->value[1] = number_range(1, MAX_TREE);
    
    act ("$n mutters some words and $p materializes from thin air.",ch,obj,NULL,TO_ROOM);
    send_to_char("A small seed materializes before you.\r\n",ch);
    return;
}


void spell_call_pet( int sn, int level, CHAR_DATA *ch, void *vo) {
    CHAR_DATA *pet = ch->pet;

    if (pet == NULL) {
       send_to_char("You own no pet.\r\n", ch);
       return;
    }

    if (pet->in_room == ch->in_room
    || ch->in_room == NULL) {
       send_to_char("Your pet is already here.\r\n", ch);
       return;
    }
       
    if (pet->fighting || pet->position == POS_FIGHTING)  stop_fighting(pet,FALSE);
    char_from_room(pet);
    char_to_room(pet, ch->in_room);

    act ("$n hurries in, looking for $N.",pet,NULL,ch,TO_ROOM);
    return;
}
    

void spell_curse_mummy( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim=(CHAR_DATA *)vo;
    AFFECT_DATA af;
    int damage;

    if (IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    || IS_SET(victim->in_room->room_flags, ROOM_LAW)
    || IS_SET(victim->in_room->area->area_flags, AREA_LAW)
    || IS_RAFFECTED(victim->in_room, RAFF_LOW_MAGIC)
    || ch == victim) {
          send_to_char("You can't reach your victim.\r\n", ch);
          return;
    }

    if (!IS_NPC(victim)) {
        if (victim->pcdata->pk_timer > 0) {
          send_to_char("You can't reach your victim.\r\n", ch);
          return;
        }
    }
         
   /* Set up and adjust damage... */
    damage = (level + number_range(0, level))/4;
    if (IS_AFFECTED(victim, AFF_ABSORB)) damage /=2;

     send_to_char( "You feel unclean.\r\n", victim);
    send_to_char( "You bestow the curse of the mummy.\r\n", ch);
    if (!IS_AFFECTED(victim, AFF_CURSE) 
    && !saves_spell( level, victim )) {
       af.type      = sn;
       af.afn	 = gafn_curse;
       af.level     = level;
       af.duration  = level/2;
       af.location  = APPLY_HITROLL;
       af.modifier  = -1 * (level / 20);
       af.bitvector = AFF_CURSE;
       affect_to_char(victim, &af );

       af.location  = APPLY_SAVING_SPELL;
       af.modifier  = level / 20;
       affect_to_char(victim, &af );
     }
     send_to_char( "You choke.\r\n", victim);
     victim->hit -=damage;
     update_pos(victim);
     kill_msg(victim);
     if (victim->position == POS_DEAD) raw_kill(victim, TRUE);

    return;
}


void spell_enclosure(int sn, int level, CHAR_DATA *ch, void *vo) {
AFFECT_DATA af;

	if (IS_RAFFECTED(ch->in_room, RAFF_ENCLOSED))	{
		send_to_char ("This room is already enclosed.\r\n",ch);
		return;
	}
	
	af.type        = sn;
                af.afn	    = gafn_room_enclosed; 
   	af.level     = level;
	af.duration  = level/20 +1;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = RAFF_ENCLOSED;
	affect_to_room(ch->in_room, &af );
	send_to_char ("A translucent force-field blocks all exits.\r\n",ch);
                return;
}


void spell_recharge(int sn, int level, CHAR_DATA *ch, void *vo) {
OBJ_DATA *obj = (OBJ_DATA *) vo;
AFFECT_DATA *af;
int toraise;

        if (!obj->enchanted) {
	send_to_char ("You can't recharge that object.\r\n",ch);
                return;
        }

        if (ch->mana < obj->level/2) return;

        for ( af = obj->affected; af != NULL; af = af->next )        {
                  if (af->duration < 0
                  && af->location == APPLY_EFFECT) {
                              toraise = af->modifier * obj->level / 2;
                              if (ch->mana > toraise) {
                                  ch->mana -=toraise;
                                  af->modifier = 100;
                              } else {
                                  af->modifier += ch->mana / (obj->level/2 +1);
                                  ch->mana = 0;
                              }
                  }
           }
           sprintf_to_char(ch, "%s emmits a {Ggreenish radiation{x for a while.\r\n", capitalize(obj->short_descr));
           return;
}


void spell_create_figurine(int sn, int level, CHAR_DATA *ch, void *vo) {
CHAR_DATA *mob = (CHAR_DATA *) vo;
OBJ_DATA *obj;
EXTRA_DESCR_DATA *ed;
char buf[MAX_STRING_LENGTH];
int chance;

    if (mob != ch->pet) {
	send_to_char ("You can do that only to your pet.\r\n",ch);
                return;
    }

    die_follower(ch);

    obj = create_object(get_obj_index(OBJ_FIGURINE), mob->level);
    obj->level = mob->level;
    free_string(obj->name);
    sprintf(buf, "figurine %s", mob->name);
    obj->name = strdup(buf);
    free_string(obj->short_descr);
    sprintf(buf, "a figurine of  a %s", mob->short_descr);
    obj->short_descr = strdup(buf);
    free_string(obj->description);
    sprintf(buf, "A figurine of  a %s is here.\n", mob->short_descr);
    obj->description = strdup(buf);

     ed = obj->extra_descr;
     ed =   new_extra_descr();
     ed->keyword		= str_dup("name");
     ed->description 	= str_dup(mob->name);
     ed->next		= obj->extra_descr;
     obj->extra_descr	= ed;
     ed =   new_extra_descr();
     ed->keyword		= str_dup("short");
     ed->description 	= str_dup(mob->short_descr);
     ed->next		= obj->extra_descr;
     obj->extra_descr	= ed;
     ed =   new_extra_descr();
     ed->keyword		= str_dup("long");
     ed->description 	= str_dup(mob->long_descr);
     ed->next		= obj->extra_descr;
     obj->extra_descr	= ed;

    obj->item_type = ITEM_FIGURINE;
    obj->value[0] = mob->pIndexData->vnum;
    chance = (level + 10 - mob->level) * 3;
    if (number_percent() > chance) {
           if (number_percent() > 70) obj->value[1] = FIGURINE_AGGRESSIVE;
           else obj->value[1] = FIGURINE_IGNORE;
    } else {
           obj->value[1] = FIGURINE_TAME;
    }
    obj->value[2] = TRUE;
    obj->value[3] = mob->race;
    obj->cost = 0;
    free_string(obj->owner);
    obj->owner = str_dup(ch->name);
    char_from_room(mob);
    extract_char(mob, TRUE);
    obj_to_room(obj, ch->in_room);
    return;
}


void spell_elder_shield( int sn, int level, CHAR_DATA *ch, void *vo ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (ch == victim) level /=2;
    level = UMIN(level * victim->level / 50, level);
    
    if (IS_AFFECTED(victim, AFF_ELDER_SHIELD))   {
           if (ch == victim) send_to_char("You are already protected by the armor of Y'Golonac.\r\n",ch);
           else sprintf_to_char(ch, "%s is already protected by the armor of Y'Golonac.\r\n",victim->name);
           return;
    }

    af.type      = sn;
    af.afn 	 = gafn_elder_shield;
    af.level     = level;
    af.duration  = level/7  + 1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_ELDER_SHIELD;
    affect_to_char(victim, &af );
    act( "$n is surrounded by an elder shield.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by an elder shield.\r\n", victim);
    return;
}


void spell_telekinesis( int sn, int level, CHAR_DATA *ch, void *vo ) {
OBJ_DATA *obj;
ROOM_INDEX_DATA *room;
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
int dir;
   
    target_name = one_argument(target_name, arg1);
    target_name = one_argument(target_name, arg2);

    if (!strcmp(arg2, "n")
    || !strcmp(arg2, "north")) {
         dir = DIR_NORTH;
         room = find_telekinesis_room(ch, dir);

    } else if (!strcmp(arg2, "e")
    || !strcmp(arg2, "east")) {
         dir = DIR_EAST;
         room = find_telekinesis_room(ch, dir);

    } else if (!strcmp(arg2, "s")
    || !strcmp(arg2, "south")) {
         dir = DIR_SOUTH;
         room = find_telekinesis_room(ch, dir);

    } else if (!strcmp(arg2, "w")
    || !strcmp(arg2, "west")) {
         dir = DIR_WEST;
         room = find_telekinesis_room(ch, dir);

    } else if (!strcmp(arg2, "ne")
    || !strcmp(arg2, "northeast")) {
         dir = DIR_NE;
         room = find_telekinesis_room(ch, dir);

    } else if (!strcmp(arg2, "se")
    || !strcmp(arg2, "southeast")) {
         dir = DIR_SE;
         room = find_telekinesis_room(ch, dir);

    } else if (!strcmp(arg2, "sw")
    || !strcmp(arg2, "southwest")) {
         dir = DIR_SW;
         room = find_telekinesis_room(ch, dir);

    } else if (!strcmp(arg2, "nw")
    || !strcmp(arg2, "northwest")) {
         dir = DIR_NW;
         room = find_telekinesis_room(ch, dir);

    } else if (!strcmp(arg2, "u")
    || !strcmp(arg2, "up")) {
         dir = DIR_UP;
         room = find_telekinesis_room(ch, dir);

    } else if (!strcmp(arg2, "d")
    || !strcmp(arg2, "down")) {
         dir = DIR_DOWN;
         room = find_telekinesis_room(ch, dir);

    } else if (!strcmp(arg2, "i")
    || !strcmp(arg2, "in")) {
         dir = DIR_IN;
         room = find_telekinesis_room(ch, dir);

    } else if (!strcmp(arg2, "o")
    || !strcmp(arg2, "out")) {
         dir = DIR_OUT;
         room = find_telekinesis_room(ch, dir);

    } else {
         OBJ_DATA *portal;
      
         if ((portal = get_obj_here(ch, arg2)) == NULL) {
              send_to_char("You can't find that portal.\r\n", ch);
              return;
         }
         
         if (portal->item_type != ITEM_PORTAL) {
              send_to_char("You can't find that portal.\r\n", ch);
              return;
         }

         room = get_room_index(portal->value[0]);
    }

    if (!room) {
              send_to_char("You're utterly confused.\r\n", ch);
              return;
    }
        
    if (!can_see_room(ch, room)) {
              send_to_char("You're utterly confused.\r\n", ch);
              return;
    }

    if ((obj = get_obj_room(room, arg1)) == NULL) {
              send_to_char("You can't feel that object.\r\n", ch);
              return;
    }

    if (!IS_SET(obj->wear_flags, ITEM_TAKE)
    || obj->weight > level) {
              send_to_char("That is too heavy.\r\n", ch);
              return;
    }

    act("$p starts floating out...", NULL, obj, NULL, TO_ROOM);

    obj_from_room(obj);
    obj_to_room(obj, ch->in_room);

    act("$p floats in.", NULL, obj, NULL, TO_ROOM);
    return;
}


void spell_duel( int sn, int level, CHAR_DATA *ch, void *vo ) {
char buf[MAX_STRING_LENGTH];
CHAR_DATA *victim = (CHAR_DATA *)vo;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

    do_stand(ch, "");
    ch->acv_state = 0;

    if (victim == ch) {
          send_to_char("Be reasonable!\r\n", ch);
          return;
    }  

    if ( ch->move < ( 10 * DUEL_COST )) {
        send_to_char("You are to tired to start a duel now.\r\n", ch);
        return;
    } 

    mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 0, NULL);
    wev = get_wev(WEV_DUEL, WEV_DUEL_START, mcc,
                  "You initiate a magical duel with @v2!{x\r\n",
                  "@a2 initiates a magical duel with you!{x\r\n",
                  "@a2 and @v2 start a magical duel!\r\n{x");

    if (!room_issue_wev_challange( ch->in_room, wev)) {
        free_wev(wev);
        return;
    } 

    room_issue_wev( ch->in_room, wev);
    free_wev(wev);

    sprintf( buf, "duelling with %s.", victim->short_descr);
    set_activity(ch, ch->position, ch->pos_desc, ACV_DUEL, buf);

    sprintf( buf, "duelling with %s.", ch->short_descr);
    set_activity(victim, victim->position, victim->pos_desc,  ACV_DUEL, buf);

    set_activity_key(ch);
    ch->acv_state = 25; 

    ch->acv_int = 100;
    victim->acv_int = 100;
    free_string(ch->acv_text);
    free_string(victim->acv_text);
    ch->acv_text = str_dup(victim->true_name);
    victim->acv_text = str_dup(ch->true_name);

    do_duel_prompt(ch, victim);
    schedule_activity( ch, DUEL_SPEED, "induel" );
    return;
}


void spell_heat_weapon( int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA *victim = (CHAR_DATA *) vo;
OBJ_DATA *obj;

    if ( saves_spell( level, victim )) return;

    act( "$n is surrounded by a glowing plasma.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by glowing plasma.\r\n", victim);

    if ((obj = get_eq_char( victim, WEAR_WIELD)) != NULL ) heat_obj(victim, obj, level);
    if ((obj = get_eq_char( victim, WEAR_WIELD2)) != NULL ) heat_obj(victim, obj, level);
    if ((obj = get_eq_char( victim, WEAR_SHIELD)) != NULL ) heat_obj(victim, obj, level);
    return;
}

void heat_obj(CHAR_DATA *ch, OBJ_DATA *obj, int level) {
int heat;

     if (number_percent() > 33) return;

     switch (obj->material) {
         default:
             heat=0;
             break;

         case MATERIAL_CLOTH:
         case MATERIAL_PAPER:
         case MATERIAL_VELLUM:
         case MATERIAL_HAIR:   
         case MATERIAL_SILK:
              heat = 0;
              harmful_effect(obj, level*2, TAR_OBJ_INV, DAM_FIRE);
              break;

         case MATERIAL_WOOD:
         case MATERIAL_FEATHERS:
         case MATERIAL_LEATHER:
         case MATERIAL_SKIN:            
         case MATERIAL_REPTILE_LEATHER:            
         case MATERIAL_FUR:
              heat = 0;
              harmful_effect(obj, level, TAR_OBJ_INV, DAM_FIRE);
              break;

         case MATERIAL_PLASTIC:
         case MATERIAL_RUBBER:
               heat = 25;
               harmful_effect(obj, level*2, TAR_OBJ_INV, DAM_FIRE);
               break;

         case MATERIAL_ALUMINIUM:
               heat = 25;
               break;

         case MATERIAL_IRON:
         case MATERIAL_STEEL:
               heat = 50;
               break;

         case MATERIAL_SILVER:
         case MATERIAL_GOLD:
         case MATERIAL_BRONZE:
               heat = 70;
               break;

         case MATERIAL_BRASS:
         case MATERIAL_COPPER:
               heat = 90;
               break;

     }

     sprintf_to_char(ch, "{R%s heats up considerably!{x\r\n", obj->short_descr);
     if (heat + level - ch->level> number_percent()) {
              if (IS_OBJ_STAT(obj, ITEM_NOREMOVE)
              || IS_OBJ_STAT(obj, ITEM_NODROP))    {
                  send_to_char("You can't get rid of it!\r\n", ch);
                  return;
              }

              act("You drop $p hastily!",ch, obj, NULL, TO_CHAR);
              act("$n drops $p hastily!",ch, obj, NULL, TO_ROOM);

              obj_from_char(obj);
              if (IS_SET(obj->extra_flags, ITEM_MELT_DROP)) extract_obj(obj);
              else obj_to_room(obj, ch->in_room );
     }
     return;
}


void spell_agony(int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA *victim=(CHAR_DATA *)vo;
int damage;

    if (IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    || IS_SET(victim->in_room->room_flags, ROOM_LAW)
    || IS_SET(victim->in_room->area->area_flags, AREA_LAW)
    || ch == victim) {
          send_to_char("You can't reach your victim.\r\n", ch);
          return;
    }

    if (!IS_NPC(victim)) {
        if (victim->pcdata->pk_timer > 0) {
          send_to_char("You can't reach your victim.\r\n", ch);
          return;
        }
    }

    if (ch->in_room->area->zone != victim->in_room->area->zone) {
         sprintf_to_char(ch, "%s is too far away.\r\n", victim->short_descr);
         return;
    }
             
   /* Set up and adjust damage... */
    damage = (level + number_range(0, level));
    if (IS_AFFECTED(victim, AFF_ABSORB)) damage /=2;

    send_to_char( "Terrible pain floods through your bones.\r\n", victim);
    send_to_char( "You feel your victim suffering.\r\n", ch);

     if (IS_NPC(victim)) {
          victim->prey =ch;
          ch->huntedBy = victim;
     }
     victim->move -=damage;
     victim->hit -= damage/3;
     update_pos(victim);
     kill_msg(victim);
     if (victim->position == POS_DEAD) raw_kill(victim, TRUE);

    return;
}


void spell_true_sight(int sn, int level, CHAR_DATA *ch, void *vo ) {
CHAR_DATA *victim=(CHAR_DATA *)vo;

    if (!victim->short_descr_orig || !IS_AFFECTED(victim, AFF_POLY)) {
         send_to_char("{mYou're magic can't discover anything new...{x\r\n", ch);
         return;
    }

    sprintf_to_char(ch, "{mIn fact, this is %s.{x\r\n", victim->short_descr_orig);
    return;
}


void spell_drain_vitality(int sn, int level, CHAR_DATA *ch, void *vo) {
OBJ_DATA *tree=(OBJ_DATA *)vo;
int diff, drain;

    if (tree->item_type != ITEM_TREE) {
        send_to_char("That's no tree.\r\n", ch);
        return;
    }

    drain = atoi(target_name);
    if (drain == 0) drain = ch->level * 3;
    
    diff = drain / ch->level /2;

    bug("a:%d", drain);

    if (number_percent() < diff) {
        send_to_char("{mYou lose concentration...{x\r\n", ch);
        return;
    }

    if (ch->mana + drain > ch->max_mana) drain = ch->max_mana - ch->mana;    
    if (drain > tree->value[3]/3) drain = tree->value[3]/3;
    bug("b:%d", drain);

    if (drain < 1) return;

    tree->value[3] = tree->value[3] - (3*drain);
    ch->mana += drain;
    sprintf_to_char(ch, "You drain vitality from the tree [{y%d{x].\r\n", drain);
    return;
}


void spell_confuse_hunters(int sn, int level, CHAR_DATA *ch, void *vo) {
CHAR_DATA *hunter;
CHAR_DATA *hunter_next;
ROOM_INDEX_DATA *room;
bool found = FALSE;
int chance;

       if (ch->fighting) {
           send_to_char("You are fighting!\r\n", ch);
           return;
       }

       for(hunter = char_list; hunter; hunter = hunter_next) {
            hunter_next = hunter->next;
            if (hunter->prey == ch) {
                 if (hunter->fighting) continue;
                 chance = ch->level - hunter->level + 75;
                 if (number_percent() > chance) continue;
                 hunter->prey = NULL;
                 found = TRUE;
                 room = get_room_index(hunter->recall_perm);
                 if (room) {
                        char_from_room(hunter);
                        char_to_room(hunter, room);
                 }
            }
       }
       ch->huntedBy = NULL;

        if (found) {
           send_to_char("The hunters have been shaken off!\r\n", ch);
        } else {
           send_to_char("There was nobody found hunting you!\r\n", ch);
        }

        return;
}


void spell_farsight( int sn, int level, CHAR_DATA *ch, void *vo ) {
AFFECT_DATA af;

    if (IS_AFFECTED(ch, AFF_BLIND))   {
           send_to_char("You are blind!\r\n",ch);
           return;
    }

    if (IS_AFFECTED(ch, AFF_FARSIGHT))   {
           send_to_char("Your vision range is already enhanced.\r\n",ch);
           return;
    }

    af.type      = sn;
    af.afn 	 = gafn_farsight;
    af.level     = level;
    af.duration  = level/3  + 1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_FARSIGHT;
    affect_to_char(ch, &af );
    send_to_char( "You open your inner eye.\r\n", ch);
    return;
}


void spell_change_size( int sn, int level, CHAR_DATA *ch, void *vo ) {
AFFECT_DATA af;
int mod;

    if (is_affected(ch, gafn_size)) {
        send_to_char("You can't change your size anymore.\r\n", ch);
        return;
    }

    if ( !str_cmp( target_name, "smaller")) {
        mod = -1;
    } else if ( !str_cmp( target_name, "larger")) {
        mod = 1;
    } else {
        send_to_char("You don't exactly know what to do.\r\n", ch);
        return;
    } 

    af.type 	= sn;
    af.afn	 	= gafn_size;
    af.level	= level;
    af.duration	= level / 15 +5;
    af.modifier  	= mod;
    af.bitvector 	= 0;
    af.location  	= APPLY_HEIGHT;
    affect_to_char(ch, &af);

    if (mod == 1) {
        send_to_char("You grow taller.\r\n", ch);
        act("$n seems to grow!",ch, NULL, NULL, TO_ROOM);
    } else {
        send_to_char("You begin to shrink.\r\n", ch);
        act("$n seems to shrink!",ch, NULL, NULL, TO_ROOM);
    }

    return;
}


void spell_astral_walk( int sn, int level, CHAR_DATA *ch, void *vo) {
CHAR_DATA *mob;

    level = UMAX(1, level /2);
    mob = create_mobile_level( get_mob_index(MOB_VNUM_ASTRAL), "mob astral", level);
    char_to_room( mob, ch->in_room );

    if (!ch->desc) return;
    if (ch->desc->original) {
	send_to_char( "You are already switched.\r\n", ch );
	return;
    }

    send_to_char("{mYou create your astral body.{x\r\n", ch);
    act("{m$ns astral body appears!{x",ch, NULL, NULL,TO_ROOM);

    mob->pcdata = ch->pcdata;
    ch->desc->character = mob;
    ch->desc->original  = ch;
    mob->desc        = ch->desc;
    mob->comm = ch->comm;
    mob->lines = ch->lines;
    notify_message (ch, WIZNET_SWITCH, TO_IMM, mob->short_descr);
    return;
}


void spell_astral_blast(int sn, int level, CHAR_DATA *ch, void *vo) {
CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (!IS_SET(victim->form, FORM_MIST)) {
         send_to_char("Your victim is unaffected!\r\n",ch);
         if (victim !=ch) send_to_char("You feel a tickling sensation!\r\n",victim);
         return;
    }        

    spell_CDB->atk_dam_type = DAM_HOLY;
    spell_CDB->atk_blk_type = BLOCK_ABSORB;
    spell_CDB->damage = dice(level, 30);


    act("{CThe astral sphere explodes.{x", ch,NULL,victim,TO_VICT);
    send_to_char("The spell produces a strange senstion.\r\n", ch);

    if (saves_spell( level, spell_CDB->defender ) ) spell_CDB->damage /= 2;

    if  (!IS_SET(spell_CDB->attacker->plr, PLR_AUTOKILL)) {
         SET_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL);
         apply_damage( spell_CDB);  
         REMOVE_BIT(spell_CDB->attacker->plr, PLR_AUTOKILL) ;
    } else {
         apply_damage( spell_CDB);
    }
    return;
}


void spell_oracle(int sn, int level, CHAR_DATA *ch, void *vo) {
MOB_CMD_CONTEXT *mcc;
EXTRA_DESCR_DATA *ed;
char buf[MAX_STRING_LENGTH];
char res_buf[MAX_STRING_LENGTH];
char arg[MAX_INPUT_LENGTH];
char *desccopy;
int num = -1;
bool ok = FALSE;

     mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL);
    
     ed = ch->in_room->extra_descr;
     while (ed) {
         if (is_name("static_rumor", ed->keyword)) break;
         ed = ed->next;
     }

     if (ed) {
         desccopy = str_dup(ed->description);
         one_argument(desccopy, arg);
         num = atoi(arg);
         num = URANGE(0, num, rumor_count);
         if (!ec_satisfied_mcc(mcc, rumor_array[num].ec, TRUE)) num = -1;
     }           

     if (num < 0) {
         while(!ok) {
             num = number_range(0, rumor_count);
             if (!rumor_array[num].greater && rumor_array[num].fortune) ok = TRUE; 
             if (rumor_array[num].ec) {
                  if (!ec_satisfied_mcc(mcc, rumor_array[num].ec, TRUE)) ok = FALSE;
             }
         }
     }

     sprintf_to_char(ch, "{C%s{x\r\n", rumor_array[num].fortune);

     if (rumor_array[num].command) {
          CHAR_DATA *admin;

          for(admin = char_list; admin; admin = admin->next) {
               if (admin->pIndexData) {
                    if (admin->pIndexData->vnum == MOB_VNUM_ADMIN) break;
               }
          }

          if (admin) {
              sprintf(buf, "mpat @at %s", rumor_array[num].command);
              expand_by_context(buf, mcc, res_buf);
              enqueue_mob_cmd(admin, res_buf, 1, 0);
          }
     }

     free_mcc(mcc);
     return;
}


void spell_doppelganger( int sn, int level, CHAR_DATA *ch, void *vo) {
CHAR_DATA *mob;
int zlevel;

    zlevel = UMAX(level *2 /3, 1);

    if(ch->pet) {
        send_to_char( "You already have a pet.\r\n", ch );
        return;
    }

    if(!ch->profs || !ch->profs->profession) {
        mob = create_mobile_level(get_mob_index( MOB_VNUM_ZOMBIE ), "mob monster", zlevel );
    } else {
        mob = create_mobile_level(get_mob_index( MOB_VNUM_ZOMBIE ), ch->profs->profession->name, zlevel );
    }
    free_string(mob->name);
    free_string(mob->short_descr);
    free_string(mob->long_descr);
    free_string(mob->description);
    mob->name = str_dup(ch->name);
    mob->short_descr = str_dup(ch->short_descr);
    mob->long_descr = str_dup(ch->long_descr);
    mob->description = str_dup(ch->description);
    mob->race = ch->race;
    mob->nature = ch->nature;
    mob->imm_flags = ch->imm_flags;
    mob->res_flags = ch->res_flags;
    mob->vuln_flags = ch->vuln_flags;
    mob->fright = 0;
    char_to_room( mob, ch->in_room );

    act( "$n creates a terrible doppelganger!", ch, NULL, NULL, TO_ROOM );
    send_to_char("You create a terrible doppelganger.\r\n",ch);

     SET_BIT(mob->affected_by, AFF_CHARM);
     SET_BIT(mob->act, ACT_FOLLOWER|ACT_UNDEAD);
     SET_BIT(mob->form, FORM_UNDEAD|FORM_INSTANT_DECAY);

     add_follower( mob, ch );
     mob->leader = ch;
     ch->pet = mob;

     do_say( mob, "How may I serve you, master?" );
     return;
}


void spell_blade_affect( int sn, int level, CHAR_DATA *ch, void *vo) {
OBJ_DATA *obj = (OBJ_DATA *) vo;
AFFECT_DATA *paf; 
int orig_dt;

    if (obj->item_type != ITEM_WEAPON)    {
	send_to_char("That isn't a weapon.\r\n",ch);
	return;
    }

    if (obj->value[0] == WEAPON_EXOTIC
    || obj->value[0] == WEAPON_MACE
    || obj->value[0] == WEAPON_FLAIL
    || obj->value[0] == WEAPON_WHIP
    || obj->value[0] == WEAPON_GUN
    || obj->value[0] == WEAPON_HANDGUN
    || obj->value[0] == WEAPON_SMG
    || obj->value[0] == WEAPON_BOW
    || obj->value[0] == WEAPON_STAFF) {
	send_to_char("That will not work on such a weapon.\r\n",ch);
	return;
    }

    orig_dt = attack_table[obj->value[3]].damage;
    if (orig_dt != DAM_NONE
    && orig_dt != DAM_BASH
    && orig_dt != DAM_PIERCE
    && orig_dt != DAM_SLASH) {
	send_to_char("This weapon is already very special.\r\n",ch);
	return;
    }

    if (obj->wear_loc != -1)    {
	send_to_char("The item must be carried to be activated.\r\n",ch);
	return;
    }

   if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)) {
      sprintf_to_char(ch, "%s is much too powerful!\r\n", capitalize(obj->short_descr));
      return;
   }

   if (obj_affected(obj, gafn_obj_blade_affect)) {
      send_to_char("It's already activated.\r\n",ch);
      return;
   }

   if (number_percent() < 5) {
      sprintf_to_char(ch, "%s turns to fine grey dust.\r\n",capitalize(obj->short_descr));
      extract_obj(obj);
      return;
   }


   paf = new_affect();
   paf->next = obj->affected;
   obj->affected = paf;

    paf->type 	= sn;
    paf->afn	= gafn_obj_blade_affect;
    paf->level	= UMAX(number_range(level/4, level/2), 10);
    paf->duration	= level /4;
    paf->modifier  	= WDT_FLAME;
    paf->bitvector 	= 0;
    paf->location  	= APPLY_OBJECT;
  
    sprintf_to_char(ch, "{m%s starts to burn brightly.{x\r\n", capitalize(obj->short_descr));
    act("{m$p starts to burn!{x",ch, obj, NULL, TO_ROOM);
    return;
}


void spell_release_mana( int sn, int level, CHAR_DATA *ch, void *vo) {
     ch->mana = UMIN(ch->mana + level *10, ch->max_mana);
     return;
}


void spell_store_mana(int sn, int level, CHAR_DATA *ch, void *vo) {
OBJ_DATA *obj;
int drain = atoi(target_name);

    if (drain <= 0 || drain > ch->mana) drain = ch->mana;
    ch->mana -= drain;

    if (number_percent() < 20) {
        if (number_percent() < 20) {
            send_to_char("{mYou lose concentration...{x\r\n", ch);
            return;
        }
        drain /=3;
    } 
    drain /=3;

    if (drain  < 20) {
            send_to_char("{mYou lose concentration...{x\r\n", ch);
            return;
    }
    
    obj = create_object (get_obj_index (OBJ_POTION), ch->level);
    obj->cost=0;
    obj->item_type = ITEM_PILL;
    obj->material = MATERIAL_PILL;
    obj->weight = 1;
    obj->value[0] = drain/10;
    obj->value[1] = get_effect_efn("release mana");
    obj->timer = number_range(24, 48);
    obj_to_room (obj, ch->in_room);
    free_string(obj->name);
    obj->name = str_dup("pill blue");
    free_string(obj->short_descr);
    obj->short_descr = str_dup("a little blue pill");
    free_string(obj->description);
    obj->description = str_dup("A little blue pill lies here.");

    act ("$n mutters some words and $p materializes from thin air.",ch,obj,NULL,TO_ROOM);
    send_to_char("A {Bblue pill{x materializes before you.\r\n",ch);
    return;
}


void spell_asceticism( int sn, int level, CHAR_DATA *ch, void *vo ) {
AFFECT_DATA af;

    if (IS_AFFECTED(ch, AFF_ASCETICISM))   {
           send_to_char("You're already living very ascetic!\r\n",ch);
           return;
    }

    af.type      	= sn;
    af.afn 	 	= gafn_asceticism;
    af.level     	= level;
    af.duration  	= level/8  + 5;
    af.location  	= APPLY_NONE;
    af.modifier  	= 0;
    af.bitvector 	= AFF_ASCETICISM;
    affect_to_char(ch, &af );
    send_to_char( "You take total control about your body.\r\n", ch);
    return;
}


void spell_antipsychotica( int sn, int level, CHAR_DATA *ch, void *vo) {
AFFECT_DATA af;

    if (!is_affected(ch, gafn_antipsychotica)) {
        af.type      	= sn;
        af.afn	 = gafn_antipsychotica;
        af.level     	= level;
        af.duration  	= level;
        af.location  	= APPLY_NONE;
        af.modifier  	= number_range(45, 65);
        if (IS_AFFECTED(ch, AFF_RELAXED)) af.bitvector = 0;
        else af.bitvector = AFF_RELAXED;
        affect_to_char(ch, &af );

        af.location	= APPLY_DEX;
        af.modifier	= - UMAX(level/5, 1);
        affect_to_char(ch, &af );
    } else {
        if (number_percent() < 25) ch->perm_stat[STAT_INT]--; 
    }

    ch->sanity = UMAX(ch->sanity- dice(1,6), -9);

    send_to_char("Nothing seems to matter anymore...\r\n", ch);
    act("$n looks very dull.", ch, NULL, NULL, TO_ROOM);

    if (number_percent() < 5) spell_calm(get_skill_sn("calm"), level, ch, (void *) ch);
    else if (number_percent() < 5) spell_frenzy(get_skill_sn("frenzy"), level, ch, (void *) ch);
    else if (number_percent() < 5) spell_hallucinate(get_skill_sn("hallucinate"), level, ch, (void *) ch);

    return;
}

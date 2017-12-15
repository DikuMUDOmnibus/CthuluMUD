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
#include "deeds.h"
#include "quests.h"
#include "mob.h"
#include "monitor.h"
#include "profile.h"
#include "wev.h"
#include "econd.h"
#include "race.h"
#include "gsn.h"
#include "partner.h"
#include "olc.h"

/* command procedures needed */
DECLARE_DO_FUN(do_return);
DECLARE_DO_FUN(do_look);

void 		raw_kill			(CHAR_DATA *victim, bool corpse );
int 		mod_max_stat		(CHAR_DATA *ch, int stat);
void 		check_vanish		(CHAR_DATA *ch, OBJ_DATA *olist);
CUSTOMER 	*new_customer		(void );
ROOM_INDEX_DATA *get_exit_destination(CHAR_DATA *ch, ROOM_INDEX_DATA *room, EXIT_DATA *pexit, bool recurse);

/* External functions... */

extern int invis_lev(CHAR_DATA *ch);

/* return size number... */

int size_lookup(const char *size) {

    if (!strcmp(size, "tiny")) 		return SIZE_TINY;
    if (!strcmp(size, "small")) 	return SIZE_SMALL;
    if (!strcmp(size, "medium")) 	return SIZE_MEDIUM;
    if (!strcmp(size, "large")) 	return SIZE_LARGE;
    if (!strcmp(size, "huge")) 	return SIZE_HUGE;
    if (!strcmp(size, "giant")) 	return SIZE_GIANT;

    return SIZE_MEDIUM;
}


char *size_name(short value) {

    switch (value) {
        default:
            break;

        case SIZE_TINY:
            return "tiny";

        case SIZE_SMALL:
            return "small";

    	case SIZE_MEDIUM:
            return "medium";

        case SIZE_LARGE:
            return "large";

        case SIZE_HUGE:
            return "huge";

        case SIZE_GIANT:
            return "giant";

    }

    return "medium";
}


/* returns sex_number... */

int sex_lookup(const char *sex) {

    if (!strcmp(sex, "male")) 		return SEX_MALE;
    if (!strcmp(sex, "female")) 	return SEX_FEMALE;
    if (!strcmp(sex, "random")) 	return SEX_RANDOM;

    return SEX_NEUTRAL;
}


/* Returns sex name... */

char *sex_name(short value) {

    switch (value) {

 	case SEX_MALE:
	    return "male";

        case SEX_FEMALE:
            return "female";

        case SEX_RANDOM:
            return "random";

        default:
            break;
    }

    return "neutral";
}

/* returns poisition number */
int position_lookup (const char *name)
{
	int counter;
	for (counter = 0; counter <= MAX_POS; counter++)
		{
		if (!str_cmp(position_flags[counter].name, name))
			return position_flags[counter].bit;
		}
    return POS_STANDING;
}

/* returns dam_type */
char *position_name( short num )
{
	int counter;
	for (counter =0; counter <= MAX_POS; counter++)
		{
		if (position_flags[counter].bit == num)
			return position_flags[counter].name;
		}
    return "standing";
}

/* returns damage type number */
int dam_type_lookup (const char *name)
{
	int counter;
	for (counter = 0; counter <= MAX_DAM; counter++)
		{
		if (!str_cmp(damage_type[counter].name, name))
			return damage_type[counter].bit;
		}
    return 0;
}

/* returns dam_type */
char *dam_type_name( short num )
{
	int counter;
	for (counter =0; counter <= MAX_DAM; counter++)
		{
		if (damage_type[counter].bit == num)
			return damage_type[counter].name;
		}
    return "none";
}

/* returns liquid number */
int liquid_lookup (const char *name)
{
	int counter;
	for (counter = 0; counter <= LIQ_MAX; counter++)
		{
		if (!str_cmp(liquid_flags[counter].name, name))
			return liquid_flags[counter].bit;
		}
    return 0;
}

/* returns liquid */
char *liquid_name( short num )
{
	int counter;
	for (counter =0; counter <= LIQ_MAX; counter++)
		{
		if (liquid_flags[counter].bit == num)
			return liquid_flags[counter].name;
		}
    return "none";
}

/* returns attack type number */
int attack_type_lookup (const char *name) {
	int counter;
	for (counter = 0; counter <= MAX_ATTACK_TYPES; counter++) {
		if (!str_cmp(attack_types[counter].name, name)) {
			return attack_types[counter].bit;
		}
	}
    return 0;
}

/* returns attack type */
char *attack_type_name( short num ) {
	int counter;
	for (counter = 0; counter <= MAX_ATTACK_TYPES; counter++) {
		if (attack_types[counter].bit == num) {
			return attack_types[counter].name;
		}
	}
    return "none";
}

/* returns weapon class number */
int weapon_class_lookup (const char *name) {
	int counter;
	for (counter = 1; weapon_class[counter].bit != 0; counter++) {
		if (!str_cmp(weapon_class[counter].name, name)) {
			return weapon_class[counter].bit;
		}
        }
    return WEAPON_EXOTIC;
}

/* returns weapon class */
char *weapon_class_name( short num )
{
	int counter;
	for (counter =0; counter <= MAX_WEAPON_CLASS; counter++)
		{
		if (weapon_class[counter].bit == num)
			return weapon_class[counter].name;
		}
    return "exotic";
}

/* returns item number */
int item_lookup (const char *name) {
	int counter;
	for (counter = 0; type_flags[counter].bit != 0; counter++) {
		if (!str_cmp(type_flags[counter].name, name)) {
			return type_flags[counter].bit;
		}
	}

    return ITEM_TRASH;
}

/* returns item name */
char *item_name( short num ) {
	int counter;
	for (counter =0; type_flags[counter].bit != 0; counter++) {
		if (type_flags[counter].bit == num)
			return type_flags[counter].name;
	}
    return "trash";
}


/* returns material number */
int material_lookup (const char *name)
{
	int counter;
	for (counter = 0; counter <= MAX_MATERIAL; counter++)
		{
		if (!str_cmp(material_table[counter].name, name))
			return material_table[counter].type;
		}
    return 0;
}

/* returns material name -- ROM OLC temp patch */
char *material_name( short num )
{
	int counter;
	for (counter =0; counter <= MAX_MATERIAL; counter++)
		{
		if (material_table[counter].type == num)
			return material_table[counter].name;
		}
    return "unknown";
}

/* returns material vulnerability flag */
long material_vuln (short num)
{
	int counter;
	for (counter = 0;counter <= MAX_MATERIAL; counter++)
		{
		if (material_table[counter].type == num)
			return material_table[counter].vuln_flag;
		}
	return 0;
}

/* returns race number */
int race_lookup (const char *name)
{
   int race;

   for ( race = 0; race_array[race].loaded; race++)
   {
	if (LOWER(name[0]) == LOWER(race_array[race].name[0])
	&&  !str_prefix( name, race_array[race].name))
	    return race;
   }

   return 0;
} 

/* returns class number */
int class_lookup (const char *name){
int pc_class;
 
   for ( pc_class = 0; pc_class < MAX_CLASS; pc_class++) {
        if (LOWER(name[0]) == LOWER(class_table[pc_class].name[0])
        &&  !str_prefix( name,class_table[pc_class].name))
            return pc_class;
   }
 
   return -1;
}



int check_immune(CHAR_DATA *ch, int dam_type) {
int immune;
int bit;

    immune = IS_NORMAL;

    if (ch->fighting) {
        if (IS_ARTIFACTED(ch->fighting, ART_EIBON)) return IS_NORMAL;
    }

    if (dam_type == DAM_NONE) return IS_RESISTANT;

    if ( (dam_type == DAM_BASH)
      || (dam_type == DAM_PIERCE)
      || (dam_type == DAM_SLASH)
      || (dam_type == DAM_SHOT)) {
	if (IS_SET(ch->imm_flags,IMM_WEAPON)) immune = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_WEAPON)) immune = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_WEAPON)) immune = IS_VULNERABLE;

    } else {	
	if (IS_SET(ch->imm_flags,IMM_MAGIC)) immune = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_MAGIC))  immune = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_MAGIC))  immune = IS_VULNERABLE;
    }

    /* set bits to check -- VULN etc. must ALL be the same or this will fail */
    switch (dam_type)    {
	case(DAM_BASH):		bit = IMM_BASH;	break;
	case(DAM_PIERCE):		bit = IMM_PIERCE;	break;
	case(DAM_SLASH):		bit = IMM_SLASH;	break;
	case(DAM_FIRE):		bit = IMM_FIRE;		break;
	case(DAM_COLD):		bit = IMM_COLD;	break;
	case(DAM_LIGHTNING):		bit = IMM_LIGHTNING;	break;
	case(DAM_ACID):		bit = IMM_ACID;	break;
	case(DAM_POISON):		bit = IMM_POISON;	break;
	case(DAM_NEGATIVE):		bit = IMM_NEGATIVE;	break;
	case(DAM_HOLY):		bit = IMM_HOLY;	break;
	case(DAM_ENERGY):		bit = IMM_ENERGY;	break;
	case(DAM_MENTAL):		bit = IMM_MENTAL;	break;
	case(DAM_DISEASE):		bit = IMM_DISEASE;	break;
	case(DAM_DROWNING):		bit = IMM_DROWNING;	break;
	case(DAM_LIGHT):		bit = IMM_LIGHT;	break;
                case(DAM_SHOT):       		bit = IMM_BULLETS;      	break;
                case(DAM_OLD):    		bit = IMM_OLD;      	break;
                case(DAM_SOUND):    		bit = IMM_SOUND;      	break;
	default:		return immune;
    }

    if (IS_SET(ch->imm_flags,bit)) {
       immune = IS_IMMUNE;
    } else if (IS_SET(ch->res_flags,bit)) {
        if (IS_SET(ch->envimm_flags,bit)) immune = IS_RESISTANT_PLUS;
        else immune = IS_RESISTANT;
    } else if (IS_SET(ch->envimm_flags,bit)) {
        immune = IS_ENV_IMMUNE;
    } else if (IS_SET(ch->vuln_flags,bit)) {
        immune = IS_VULNERABLE;
    }

    return immune;
}


/*
 * See if a string is one of the names of an object.
 */

bool is_full_name( const char *str, char *namelist ){
    char name[MAX_INPUT_LENGTH];

    for ( ; ; )    {
        namelist = one_argument( namelist, name );
        if ( name[0] == '\0' )
            return FALSE;
        if ( !str_cmp( str, name ) )
            return TRUE;
    }
}


/* checks mob format */
bool is_old_mob(CHAR_DATA *ch)
{
    if (ch->pIndexData == NULL)
	return FALSE;
    else if (ch->pIndexData->new_format)
	return FALSE;
    return TRUE;
}
 
/* Returning sn for a weapon... */

int get_sn_for_weapon(OBJ_DATA *wield) {

    int sn;
	
    if ( wield == NULL 
      || wield->item_type != ITEM_WEAPON) {
        sn = gsn_hand_to_hand;
    } else {
        switch (wield->value[0]) {
    
          case(WEAPON_SWORD):     sn = gsn_sword;         break;
          case(WEAPON_DAGGER):    sn = gsn_dagger;        break;
          case(WEAPON_SPEAR):     sn = gsn_spear;         break;
          case(WEAPON_MACE):      sn = gsn_mace;          break;
          case(WEAPON_AXE):       sn = gsn_axe;           break;
          case(WEAPON_FLAIL):     sn = gsn_flail;         break;
          case(WEAPON_WHIP):      sn = gsn_whip;          break;
          case(WEAPON_POLEARM):   sn = gsn_polearm;       break;
          case(WEAPON_GUN):       sn = gsn_gun;           break;
          case(WEAPON_HANDGUN):   sn = gsn_handgun;       break;  
          case(WEAPON_SMG):       sn = gsn_smg;           break;  
          case(WEAPON_BOW):       sn = gsn_bow;           break;  
          case(WEAPON_STAFF):     sn = gsn_staff;         break;  
          default :               sn = -1;                break;
       }
   }

   return sn;
}

/* Returning sn for attack weapon... */

int get_weapon_sn(CHAR_DATA *ch, bool dual) {

    OBJ_DATA *wield;

    if (!dual)
       	wield = get_eq_char( ch, WEAR_WIELD );
    else
        wield = get_eq_char( ch, WEAR_WIELD2 );

  return get_sn_for_weapon(wield);
}


/* used to de-screw characters */
void reset_char(CHAR_DATA *ch) {
int loc,mod,stat;
OBJ_DATA *obj;
AFFECT_DATA *af;
int i;

     if (IS_NPC(ch)) return;

    if (ch->pcdata->perm_hit == 0 
    || ch->pcdata->perm_mana == 0
    || ch->pcdata->perm_move == 0) {
    /* do a FULL reset */

	for (loc = 0; loc < MAX_WEAR; loc++) {
	    obj = get_eq_char(ch,loc);
	    if (obj == NULL) continue;
	    if (!obj->enchanted)
	    for ( af = obj->pIndexData->affected; af != NULL; af = af->next )   {
		mod = af->modifier;
		switch(af->location) {
		    case APPLY_SEX:	ch->sex		-= mod;
					if (ch->sex < 0 || ch->sex >2)    ch->sex = IS_NPC(ch) ?	0 : ch->pcdata->true_sex;
									break;
		    case APPLY_MANA:	ch->max_mana	-= mod;		break;
		    case APPLY_HIT:	ch->max_hit	-= mod;		break;
		    case APPLY_MOVE:	ch->max_move	-= mod;		break;
		}
	    }

            for ( af = obj->affected; af != NULL; af = af->next ) {
                mod = af->modifier;
                switch(af->location) {
                    case APPLY_SEX:     ch->sex         -= mod;         break;
                    case APPLY_MANA:    ch->max_mana    -= mod;         break;
                    case APPLY_HIT:     ch->max_hit     -= mod;         break;
                    case APPLY_MOVE:    ch->max_move    -= mod;         break;
                }
            }
	}
	/* now reset the permanent stats */
	ch->pcdata->perm_hit 	= ch->max_hit;
	ch->pcdata->perm_mana 	= ch->max_mana;
	ch->pcdata->perm_move	= ch->max_move;
	ch->pcdata->last_level	= ch->pcdata->played/3600;
	if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2) {
		if (ch->sex > 0 && ch->sex < 3) {
	    	    ch->pcdata->true_sex	= ch->sex;
		} else {
		    ch->pcdata->true_sex 	= 0;
                 	}
                } 
    }

    /* now restore the character to his/her true condition */
    for (stat = 0; stat < MAX_STATS; stat++)	ch->mod_stat[stat] = 0;

    if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)ch->pcdata->true_sex = 0; 
    ch->sex	= ch->pcdata->true_sex;
    ch->max_hit 	= ch->pcdata->perm_hit;
    ch->max_mana	= ch->pcdata->perm_mana;
    ch->max_move	= ch->pcdata->perm_move;
   
    for (i = 0; i < 4; i++)  	ch->armor[i]	= 100;

    ch->hitroll		= 0;
    ch->damroll		= 0;
    ch->saving_throw	= 0;


    for (af = ch->affected; af; af = af->next)   {

        mod = af->modifier;
        switch(af->location)        {
                case APPLY_STR:         	ch->mod_stat[STAT_STR]  	+= mod; break;
                case APPLY_DEX:         	ch->mod_stat[STAT_DEX]  	+= mod; break;
                case APPLY_INT:         	ch->mod_stat[STAT_INT]  	+= mod; break;
                case APPLY_WIS:         	ch->mod_stat[STAT_WIS]  	+= mod; break;
                case APPLY_CON:         	ch->mod_stat[STAT_CON]  	+= mod; break;
	case APPLY_LUCK:	ch->mod_stat[STAT_LUC]	+= mod; break;
	case APPLY_CHA:	ch->mod_stat[STAT_CHA]	+= mod; break;
 
                case APPLY_SEX:         	ch->sex                 		+= mod; break;
                case APPLY_MANA:        	ch->max_mana            	+= mod; break;
                case APPLY_HIT:         	ch->max_hit             	+= mod; break;
                case APPLY_MOVE:        	ch->max_move            	+= mod; break;
 
                case APPLY_AC:
                    for (i = 0; i < 4; i ++) ch->armor[i] += mod;
                    break;
                case APPLY_HITROLL:     		ch->hitroll             	+= mod; break;
                case APPLY_DAMROLL:     	ch->damroll          	+= mod; break;
 
                case APPLY_SAVING_PARA:         	ch->saving_throw += mod; break;
                case APPLY_SAVING_ROD:          	ch->saving_throw += mod; break;
                case APPLY_SAVING_PETRI:        	ch->saving_throw += mod; break;
                case APPLY_SAVING_BREATH:   	ch->saving_throw += mod; break;
                case APPLY_SAVING_SPELL:        	ch->saving_throw += mod; break;

		case APPLY_IMMUNITY:
		           switch (mod) {            
		              case 1: SET_BIT(ch->imm_flags, IMM_SUMMON); 	break;
		              case 2: SET_BIT(ch->imm_flags, IMM_CHARM); 	break;
		              case 3: SET_BIT(ch->imm_flags, IMM_MAGIC); 	break;
		              case 4: SET_BIT(ch->imm_flags, IMM_WEAPON); 	break;
		              case 5: SET_BIT(ch->imm_flags, IMM_BASH); 	break;
		              case 6: SET_BIT(ch->imm_flags, IMM_PIERCE); 	break;
		              case 7: SET_BIT(ch->imm_flags, IMM_SLASH); 	break;
		              case 8: SET_BIT(ch->imm_flags, IMM_FIRE); 	break;
		              case 9: SET_BIT(ch->imm_flags, IMM_COLD); 	break;
		              case 10: SET_BIT(ch->imm_flags, IMM_LIGHTNING); break;
		              case 11: SET_BIT(ch->imm_flags, IMM_ACID); 	break;
		              case 12: SET_BIT(ch->imm_flags, IMM_POISON); 	break;
		              case 13: SET_BIT(ch->imm_flags, IMM_NEGATIVE); 	break;
		              case 14: SET_BIT(ch->imm_flags, IMM_HOLY); 	break;
		              case 15: SET_BIT(ch->imm_flags, IMM_ENERGY); 	break;
		              case 16: SET_BIT(ch->imm_flags, IMM_MENTAL); 	break;
		              case 17: SET_BIT(ch->imm_flags, IMM_DISEASE); 	break;
		              case 18: SET_BIT(ch->imm_flags, IMM_DROWNING); break;
		              case 19: SET_BIT(ch->imm_flags, IMM_LIGHT); 	break;
		              case 20: SET_BIT(ch->imm_flags, IMM_BULLETS); 	break;
		              case 21: SET_BIT(ch->imm_flags, IMM_MASK); 	break;
		              case 22: SET_BIT(ch->imm_flags, IMM_OLD); 	break;
		              case 23: SET_BIT(ch->imm_flags, IMM_SOUND); 	break;
		               default: break;
		           }
		           break;

    		case APPLY_ENV_IMMUNITY:
           		           switch (mod) {            
                                              case 3: SET_BIT(ch->envimm_flags, IMM_MAGIC); 		break;
                                              case 8: SET_BIT(ch->envimm_flags, IMM_FIRE); 		break;
                                              case 9: SET_BIT(ch->envimm_flags, IMM_COLD); 		break;
                                              case 10: SET_BIT(ch->envimm_flags, IMM_LIGHTNING); 	break;
                                              case 11: SET_BIT(ch->envimm_flags, IMM_ACID); 		break;
                                              case 12: SET_BIT(ch->envimm_flags, IMM_POISON); 		break;
                                              case 13: SET_BIT(ch->envimm_flags, IMM_NEGATIVE); 	break;
                                              case 14: SET_BIT(ch->envimm_flags, IMM_HOLY); 		break;
                                              case 15: SET_BIT(ch->envimm_flags, IMM_ENERGY); 	break;
                                              case 16: SET_BIT(ch->envimm_flags, IMM_MENTAL); 	break;
                                              case 17: SET_BIT(ch->envimm_flags, IMM_DISEASE); 	break;
                                              case 18: SET_BIT(ch->envimm_flags, IMM_DROWNING); 	break;
                                              case 19: SET_BIT(ch->envimm_flags, IMM_LIGHT); 		break;
                                              case 22: SET_BIT(ch->envimm_flags, IMM_OLD); 		break;
                                              case 23: SET_BIT(ch->envimm_flags, IMM_SOUND); 		break;
                                                default: break;
                                           }
                                           break;

		    case APPLY_RESISTANCE:
		           switch (mod) {            
		              case 2: SET_BIT(ch->res_flags, RES_CHARM); 	break;
		              case 3: SET_BIT(ch->res_flags, RES_MAGIC); 	break;
		              case 4: SET_BIT(ch->res_flags, RES_WEAPON); 	break;
		              case 5: SET_BIT(ch->res_flags, RES_BASH); 		break;
		              case 6: SET_BIT(ch->res_flags, RES_PIERCE); 	break;
  		              case 7: SET_BIT(ch->res_flags, RES_SLASH); 	break;
		              case 8: SET_BIT(ch->res_flags, RES_FIRE); 		break;
		              case 9: SET_BIT(ch->res_flags, RES_COLD); 		break;
		              case 10: SET_BIT(ch->res_flags, RES_LIGHTNING); 	break;
		              case 11: SET_BIT(ch->res_flags, RES_ACID); 	break;
		              case 12: SET_BIT(ch->res_flags, RES_POISON); 	break;
		              case 13: SET_BIT(ch->res_flags, RES_NEGATIVE); 	break;
		              case 14: SET_BIT(ch->res_flags, RES_HOLY); 	break;
		              case 15: SET_BIT(ch->res_flags, RES_ENERGY); 	break;
		              case 16: SET_BIT(ch->res_flags, RES_MENTAL); 	break;
		              case 17: SET_BIT(ch->res_flags, RES_DISEASE); 	break;
		              case 18: SET_BIT(ch->res_flags, RES_DROWNING); 	break;
		              case 19: SET_BIT(ch->res_flags, RES_LIGHT); 	break;
		              case 20: SET_BIT(ch->res_flags, RES_BULLETS); 	break;
		              case 21: SET_BIT(ch->res_flags, RES_MASK); 	break;
		              case 22: SET_BIT(ch->res_flags, RES_OLD); 		break;
		              case 23: SET_BIT(ch->res_flags, RES_SOUND); 	break;
		               default: break;
		           }
		       break;
        } 
    }

    /* now start adding back the effects */
    for (loc = 0; loc < MAX_WEAR; loc++)   {
        obj = get_eq_char(ch,loc);
        if (obj == NULL) continue;
        for (i = 0; i < 4; i++) ch->armor[i] -= apply_ac( obj, loc, i );

        if (!obj->enchanted)
	for ( af = obj->pIndexData->affected; af; af = af->next )     {
                      mod = af->modifier;
                      switch(af->location)    {
		case APPLY_STR:		ch->mod_stat[STAT_STR]	+= mod;	break;
		case APPLY_DEX:		ch->mod_stat[STAT_DEX]	+= mod; break;
		case APPLY_INT:		ch->mod_stat[STAT_INT]		+= mod; break;
		case APPLY_WIS:		ch->mod_stat[STAT_WIS]	+= mod; break;
		case APPLY_CON:		ch->mod_stat[STAT_CON]	+= mod; break;
		case APPLY_LUCK:		ch->mod_stat[STAT_LUC]	+= mod; break;
		case APPLY_CHA:		ch->mod_stat[STAT_CHA]	+= mod; break;

		case APPLY_SEX:		ch->sex			+= mod; break;
		case APPLY_MANA:		ch->max_mana		+= mod; break;
		case APPLY_HIT:		ch->max_hit		+= mod; break;
		case APPLY_MOVE:		ch->max_move		+= mod; break;

		case APPLY_AC:		
		    for (i = 0; i < 4; i ++) ch->armor[i] += mod; 
		    break;
		case APPLY_HITROLL:	ch->hitroll		+= mod; break;
		case APPLY_DAMROLL:	ch->damroll		+= mod; break;
	
		case APPLY_SAVING_PARA:		ch->saving_throw += mod; break;
		case APPLY_SAVING_ROD: 		ch->saving_throw += mod; break;
		case APPLY_SAVING_PETRI:	ch->saving_throw += mod; break;
		case APPLY_SAVING_BREATH: 	ch->saving_throw += mod; break;
		case APPLY_SAVING_SPELL:	ch->saving_throw += mod; break;
                                case APPLY_AGE:
                                    if (!IS_NPC(ch)) ch->pcdata->age_mod -= mod;
                                    break;

		case APPLY_IMMUNITY:
		           switch (mod) {            
		              case 1: SET_BIT(ch->imm_flags, IMM_SUMMON); 	break;
		              case 2: SET_BIT(ch->imm_flags, IMM_CHARM); 	break;
		              case 3: SET_BIT(ch->imm_flags, IMM_MAGIC); 	break;
		              case 4: SET_BIT(ch->imm_flags, IMM_WEAPON); 	break;
		              case 5: SET_BIT(ch->imm_flags, IMM_BASH); 	break;
		              case 6: SET_BIT(ch->imm_flags, IMM_PIERCE); 	break;
		              case 7: SET_BIT(ch->imm_flags, IMM_SLASH); 	break;
		              case 8: SET_BIT(ch->imm_flags, IMM_FIRE); 	break;
		              case 9: SET_BIT(ch->imm_flags, IMM_COLD); 	break;
		              case 10: SET_BIT(ch->imm_flags, IMM_LIGHTNING); break;
		              case 11: SET_BIT(ch->imm_flags, IMM_ACID); 	break;
		              case 12: SET_BIT(ch->imm_flags, IMM_POISON); 	break;
		              case 13: SET_BIT(ch->imm_flags, IMM_NEGATIVE); 	break;
		              case 14: SET_BIT(ch->imm_flags, IMM_HOLY); 	break;
		              case 15: SET_BIT(ch->imm_flags, IMM_ENERGY); 	break;
		              case 16: SET_BIT(ch->imm_flags, IMM_MENTAL); 	break;
		              case 17: SET_BIT(ch->imm_flags, IMM_DISEASE); 	break;
		              case 18: SET_BIT(ch->imm_flags, IMM_DROWNING); break;
		              case 19: SET_BIT(ch->imm_flags, IMM_LIGHT); 	break;
		              case 20: SET_BIT(ch->imm_flags, IMM_BULLETS); 	break;
		              case 21: SET_BIT(ch->imm_flags, IMM_MASK); 	break;
		              case 22: SET_BIT(ch->imm_flags, IMM_OLD); 	break;
		              case 23: SET_BIT(ch->imm_flags, IMM_SOUND); 	break;
		               default: break;
		           }
		           break;

    		case APPLY_ENV_IMMUNITY:
           		           switch (mod) {            
                                              case 3: SET_BIT(ch->envimm_flags, IMM_MAGIC); 		break;
                                              case 8: SET_BIT(ch->envimm_flags, IMM_FIRE); 		break;
                                              case 9: SET_BIT(ch->envimm_flags, IMM_COLD); 		break;
                                              case 10: SET_BIT(ch->envimm_flags, IMM_LIGHTNING); 	break;
                                              case 11: SET_BIT(ch->envimm_flags, IMM_ACID); 		break;
                                              case 12: SET_BIT(ch->envimm_flags, IMM_POISON); 		break;
                                              case 13: SET_BIT(ch->envimm_flags, IMM_NEGATIVE); 	break;
                                              case 14: SET_BIT(ch->envimm_flags, IMM_HOLY); 		break;
                                              case 15: SET_BIT(ch->envimm_flags, IMM_ENERGY); 	break;
                                              case 16: SET_BIT(ch->envimm_flags, IMM_MENTAL); 	break;
                                              case 17: SET_BIT(ch->envimm_flags, IMM_DISEASE); 	break;
                                              case 18: SET_BIT(ch->envimm_flags, IMM_DROWNING); 	break;
                                              case 19: SET_BIT(ch->envimm_flags, IMM_LIGHT); 		break;
                                              case 22: SET_BIT(ch->envimm_flags, IMM_OLD); 		break;
                                              case 23: SET_BIT(ch->envimm_flags, IMM_SOUND); 		break;
                                                default: break;
                                           }
                                           break;

		    case APPLY_RESISTANCE:
		           switch (mod) {            
		              case 2: SET_BIT(ch->res_flags, RES_CHARM); 	break;
		              case 3: SET_BIT(ch->res_flags, RES_MAGIC); 	break;
		              case 4: SET_BIT(ch->res_flags, RES_WEAPON); 	break;
		              case 5: SET_BIT(ch->res_flags, RES_BASH); 		break;
		              case 6: SET_BIT(ch->res_flags, RES_PIERCE); 	break;
  		              case 7: SET_BIT(ch->res_flags, RES_SLASH); 	break;
		              case 8: SET_BIT(ch->res_flags, RES_FIRE); 		break;
		              case 9: SET_BIT(ch->res_flags, RES_COLD); 		break;
		              case 10: SET_BIT(ch->res_flags, RES_LIGHTNING); 	break;
		              case 11: SET_BIT(ch->res_flags, RES_ACID); 	break;
		              case 12: SET_BIT(ch->res_flags, RES_POISON); 	break;
		              case 13: SET_BIT(ch->res_flags, RES_NEGATIVE); 	break;
		              case 14: SET_BIT(ch->res_flags, RES_HOLY); 	break;
		              case 15: SET_BIT(ch->res_flags, RES_ENERGY); 	break;
		              case 16: SET_BIT(ch->res_flags, RES_MENTAL); 	break;
		              case 17: SET_BIT(ch->res_flags, RES_DISEASE); 	break;
		              case 18: SET_BIT(ch->res_flags, RES_DROWNING); 	break;
		              case 19: SET_BIT(ch->res_flags, RES_LIGHT); 	break;
		              case 20: SET_BIT(ch->res_flags, RES_BULLETS); 	break;
		              case 21: SET_BIT(ch->res_flags, RES_MASK); 	break;
		              case 22: SET_BIT(ch->res_flags, RES_OLD); 		break;
		              case 23: SET_BIT(ch->res_flags, RES_SOUND); 	break;
		               default: break;
		           }
		       break;
	    }
        }
 
        for ( af = obj->affected; af != NULL; af = af->next )      {
            mod = af->modifier;
            switch(af->location)      {
                case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
                case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
                case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
                case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
                case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;
	case APPLY_LUCK:		ch->mod_stat[STAT_LUC]	+= mod; break;
	case APPLY_CHA:		ch->mod_stat[STAT_CHA]	+= mod; break;
 
                case APPLY_SEX:         ch->sex                 += mod; break;
                case APPLY_MANA:        ch->max_mana            += mod; break;
                case APPLY_HIT:         ch->max_hit             += mod; break;
                case APPLY_MOVE:        ch->max_move            += mod; break;
 
                case APPLY_AC:
                    for (i = 0; i < 4; i ++) ch->armor[i] += mod;
                    break;

	case APPLY_HITROLL:     ch->hitroll             += mod; break;
                case APPLY_DAMROLL:     ch->damroll             += mod; break;
 
                case APPLY_SAVING_PARA:         ch->saving_throw += mod; break;
                case APPLY_SAVING_ROD:          ch->saving_throw += mod; break;
                case APPLY_SAVING_PETRI:        ch->saving_throw += mod; break;
                case APPLY_SAVING_BREATH:       ch->saving_throw += mod; break;
                case APPLY_SAVING_SPELL:        ch->saving_throw += mod; break;
                case APPLY_AGE:
                        if (!IS_NPC(ch)) ch->pcdata->age_mod -= mod;
                        break;

		case APPLY_IMMUNITY:
		           switch (mod) {            
		              case 1: SET_BIT(ch->imm_flags, IMM_SUMMON); 	break;
		              case 2: SET_BIT(ch->imm_flags, IMM_CHARM); 	break;
		              case 3: SET_BIT(ch->imm_flags, IMM_MAGIC); 	break;
		              case 4: SET_BIT(ch->imm_flags, IMM_WEAPON); 	break;
		              case 5: SET_BIT(ch->imm_flags, IMM_BASH); 	break;
		              case 6: SET_BIT(ch->imm_flags, IMM_PIERCE); 	break;
		              case 7: SET_BIT(ch->imm_flags, IMM_SLASH); 	break;
		              case 8: SET_BIT(ch->imm_flags, IMM_FIRE); 	break;
		              case 9: SET_BIT(ch->imm_flags, IMM_COLD); 	break;
		              case 10: SET_BIT(ch->imm_flags, IMM_LIGHTNING); break;
		              case 11: SET_BIT(ch->imm_flags, IMM_ACID); 	break;
		              case 12: SET_BIT(ch->imm_flags, IMM_POISON); 	break;
		              case 13: SET_BIT(ch->imm_flags, IMM_NEGATIVE); 	break;
		              case 14: SET_BIT(ch->imm_flags, IMM_HOLY); 	break;
		              case 15: SET_BIT(ch->imm_flags, IMM_ENERGY); 	break;
		              case 16: SET_BIT(ch->imm_flags, IMM_MENTAL); 	break;
		              case 17: SET_BIT(ch->imm_flags, IMM_DISEASE); 	break;
		              case 18: SET_BIT(ch->imm_flags, IMM_DROWNING); break;
		              case 19: SET_BIT(ch->imm_flags, IMM_LIGHT); 	break;
		              case 20: SET_BIT(ch->imm_flags, IMM_BULLETS); 	break;
		              case 21: SET_BIT(ch->imm_flags, IMM_MASK); 	break;
		              case 22: SET_BIT(ch->imm_flags, IMM_OLD); 	break;
		              case 23: SET_BIT(ch->imm_flags, IMM_SOUND); 	break;
		               default: break;
		           }
		           break;

    		case APPLY_ENV_IMMUNITY:
           		           switch (mod) {            
                                              case 3: SET_BIT(ch->envimm_flags, IMM_MAGIC); 		break;
                                              case 8: SET_BIT(ch->envimm_flags, IMM_FIRE); 		break;
                                              case 9: SET_BIT(ch->envimm_flags, IMM_COLD); 		break;
                                              case 10: SET_BIT(ch->envimm_flags, IMM_LIGHTNING); 	break;
                                              case 11: SET_BIT(ch->envimm_flags, IMM_ACID); 		break;
                                              case 12: SET_BIT(ch->envimm_flags, IMM_POISON); 		break;
                                              case 13: SET_BIT(ch->envimm_flags, IMM_NEGATIVE); 	break;
                                              case 14: SET_BIT(ch->envimm_flags, IMM_HOLY); 		break;
                                              case 15: SET_BIT(ch->envimm_flags, IMM_ENERGY); 	break;
                                              case 16: SET_BIT(ch->envimm_flags, IMM_MENTAL); 	break;
                                              case 17: SET_BIT(ch->envimm_flags, IMM_DISEASE); 	break;
                                              case 18: SET_BIT(ch->envimm_flags, IMM_DROWNING); 	break;
                                              case 19: SET_BIT(ch->envimm_flags, IMM_LIGHT); 		break;
                                              case 22: SET_BIT(ch->envimm_flags, IMM_OLD); 		break;
                                              case 23: SET_BIT(ch->envimm_flags, IMM_SOUND); 		break;
                                                default: break;
                                           }
                                           break;

		    case APPLY_RESISTANCE:
		           switch (mod) {            
		              case 2: SET_BIT(ch->res_flags, RES_CHARM); 	break;
		              case 3: SET_BIT(ch->res_flags, RES_MAGIC); 	break;
		              case 4: SET_BIT(ch->res_flags, RES_WEAPON); 	break;
		              case 5: SET_BIT(ch->res_flags, RES_BASH); 		break;
		              case 6: SET_BIT(ch->res_flags, RES_PIERCE); 	break;
  		              case 7: SET_BIT(ch->res_flags, RES_SLASH); 	break;
		              case 8: SET_BIT(ch->res_flags, RES_FIRE); 		break;
		              case 9: SET_BIT(ch->res_flags, RES_COLD); 		break;
		              case 10: SET_BIT(ch->res_flags, RES_LIGHTNING); 	break;
		              case 11: SET_BIT(ch->res_flags, RES_ACID); 	break;
		              case 12: SET_BIT(ch->res_flags, RES_POISON); 	break;
		              case 13: SET_BIT(ch->res_flags, RES_NEGATIVE); 	break;
		              case 14: SET_BIT(ch->res_flags, RES_HOLY); 	break;
		              case 15: SET_BIT(ch->res_flags, RES_ENERGY); 	break;
		              case 16: SET_BIT(ch->res_flags, RES_MENTAL); 	break;
		              case 17: SET_BIT(ch->res_flags, RES_DISEASE); 	break;
		              case 18: SET_BIT(ch->res_flags, RES_DROWNING); 	break;
		              case 19: SET_BIT(ch->res_flags, RES_LIGHT); 	break;
		              case 20: SET_BIT(ch->res_flags, RES_BULLETS); 	break;
		              case 21: SET_BIT(ch->res_flags, RES_MASK); 	break;
		              case 22: SET_BIT(ch->res_flags, RES_OLD); 		break;
		              case 23: SET_BIT(ch->res_flags, RES_SOUND); 	break;
		               default: break;
		           }
		       break;
            }
	}
    }

    ch->imm_flags 		= ch->imm_flags | race_array[ch->race].imm;
    ch->envimm_flags 	= ch->envimm_flags | race_array[ch->race].envimm;
    ch->res_flags 		= ch->res_flags | race_array[ch->race].res;

    if (ch->sex < 0 || ch->sex > 2) ch->sex = ch->pcdata->true_sex;
}


/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust( CHAR_DATA *ch ) {
    if (ch->desc != NULL && ch->desc->original != NULL ) ch = ch->desc->original;
    if (IS_NPC(ch)) return 0;

    return ch->level;
}


int get_divinity(CHAR_DATA *ch ) {
CHAR_DATA *partner;
int rn, divmod, newdiv;

    if (!ch->pcdata) return 0; 

    if (ch->pcdata->divinity < DIV_CREATOR) {
         if (IS_SET(ch->plr, PLR_ANNOYING)) return UMIN(ch->pcdata->divinity, DIV_HUMAN);
         else return ch->pcdata->divinity;
    }

    if (!str_cmp(ch->pcdata->native, mud.name)
    || ch->pcdata->divinity >= DIV_MUD) {
        return ch->pcdata->divinity;
    }

    /* Partner not connected? There must be a reason, better don't give out imm privileges...*/
    if ((partner = get_char_world_player(NULL, ch->pcdata->native)) == NULL) return DIV_HERO;
    rn = am_i_partner(partner);

    divmod = partner_array[rn].dominance -5;
    newdiv = ch->pcdata->divinity;

    while (divmod !=0) {
       if (divmod > 0) {
            newdiv *= 2;
            divmod--;
       } else {
            newdiv /= 2;
            divmod++;
       }
    }

    if (IS_SET(get_authority(ch), IMMAUTH_ADMIN))  newdiv = URANGE(DIV_HERO, newdiv, DIV_IMPLEMENTOR);
    else newdiv = URANGE(DIV_HERO, newdiv, DIV_GREATER);
    return newdiv;
}


int get_authority(CHAR_DATA *ch ) {

    if (!ch->pcdata) return 0; 
    return ch->pcdata->authority;
}

/*
 * Retrieve a character's age.
 */
int get_age( CHAR_DATA *ch ) {
int age;

    if (IS_NPC(ch)) return 16;

    age = 16 + (time_info.year - ch->pcdata->startyear);

/*  check for a modifier */

    age -= ch->pcdata->age_mod;

/* check for birthdate yet this year */

    if ( (time_info.month >= ch->pcdata->startmonth) && (time_info.day >= ch->pcdata->startday)) age++;

    age = UMAX(age, 15);
    return age;
}


int mod_max_stat(CHAR_DATA *ch, int stat) {
int value = STAT_MAXIMUM;
OBJ_DATA *obj;
AFFECT_DATA *paf;


    for (obj = ch->carrying; obj; obj = obj->next_content) {
          if (obj->wear_loc == WEAR_NONE) continue;
          if (obj->item_type == ITEM_RAW || obj->item_type == ITEM_GEM || obj->item_type == ITEM_PASSPORT) continue;

          if (!obj->enchanted) {
  	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )	{
                     switch(stat) {
                         case STAT_STR:
                             if (paf->location == APPLY_MAX_STR) value += paf->modifier;
                             break;
                         case STAT_INT:
                             if (paf->location == APPLY_MAX_INT) value += paf->modifier;
                             break;
                         case STAT_WIS:
                             if (paf->location == APPLY_MAX_WIS) value += paf->modifier;
                             break;
                         case STAT_DEX:
                             if (paf->location == APPLY_MAX_DEX) value += paf->modifier;
                             break;
                         case STAT_CON:
                             if (paf->location == APPLY_MAX_CON) value += paf->modifier;
                             break;
                         case STAT_LUC:
                             if (paf->location == APPLY_MAX_LUCK) value += paf->modifier;
                             break;
                         case STAT_CHA:
                             if (paf->location == APPLY_MAX_CHA) value += paf->modifier;
                             break;
                      }
	}
           } else {
                for ( paf = obj->affected; paf != NULL; paf = paf->next )		{
                     switch(stat) {
                         case STAT_STR:
                             if (paf->location == APPLY_MAX_STR) value += paf->modifier;
                             break;
                         case STAT_INT:
                             if (paf->location == APPLY_MAX_INT) value += paf->modifier;
                             break;
                         case STAT_WIS:
                             if (paf->location == APPLY_MAX_WIS) value += paf->modifier;
                             break;
                         case STAT_DEX:
                             if (paf->location == APPLY_MAX_DEX) value += paf->modifier;
                             break;
                         case STAT_CON:
                             if (paf->location == APPLY_MAX_CON) value += paf->modifier;
                             break;
                         case STAT_LUC:
                             if (paf->location == APPLY_MAX_LUCK) value += paf->modifier;
                             break;
                         case STAT_CHA:
                             if (paf->location == APPLY_MAX_CHA) value += paf->modifier;
                             break;
                      }
	}
           }
    }

    value = UMIN(value, ABS_STAT_MAXIMUM);
    return value;
}

/* command for retrieving stats */
int get_curr_stat( CHAR_DATA *ch, int stat ) {
    int value;

    value = ch->perm_stat[stat] + ch->mod_stat[stat];
    value = URANGE(STAT_MINIMUM, value, mod_max_stat(ch, stat));
    return value;
}

/* command for returning max training score */
int get_max_train( CHAR_DATA *ch, int stat ) {

      return STAT_MAXIMUM;
}
   
	
/* How many objects can a character carry? */

int can_carry_n( CHAR_DATA *ch ) {

    return MAX_WEAR +  2 * (get_curr_stat(ch,STAT_DEX)/4) + ch->level;
}



/* How much weight can a character carry? */ 

int can_carry_w( CHAR_DATA *ch ) {

    return (str_app[get_curr_stat(ch,STAT_STR)].carry + ch->level * 5) / 2;
}



/*
 * See if a string is one of the names of an object.
 */
/*
bool is_name( const char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    for ( ; ; ) {
    
	namelist = one_argument( namelist, name );

	if ( name[0] == '\0' )
	    break;

	if ( !str_cmp( str, name ) )
	    return TRUE; 

	if ((strstr(name, str)==name) 
		return TRUE;
    }

    return FALSE;
}
*/

bool is_name ( char *str, char *namelist ) {

    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list;

   /* Safety check... */

    if ( str == NULL
      || namelist == NULL ) {
      return FALSE;
    }  

   /* we need ALL parts of string to match part of namelist */

    for ( ; ; ) { /* start parsing string */
    
	str = one_argument(str, part);

	if (part[0] == '\0' ) {
	  return TRUE;
        }

       /* check to see if this is part of namelist */

	list = namelist;

	for ( ; ; ) {  /* start parsing namelist */
	
	    list = one_argument(list, name);

	    if (name[0] == '\0')  /* Current part has not matched */
		return FALSE;

	    if (!str_cmp(part, name))
		break;
	}
    }
}

bool is_name_abbv ( char *str, char *namelist ) /*Zeran, abbrev is_name*/
{
    char name[MAX_INPUT_LENGTH];
    char *list, *string;

   /* Safety check... */

    if ( str == NULL
      || namelist == NULL ) {
      return FALSE;
    }  

    if (strstr(namelist, str) == namelist) /* got partial match already*/
      return TRUE;

    string = str;

   /* check to see if this is part of namelist */

    list = namelist;

    for ( ; ; )  /* start parsing namelist */
	{
	    list = one_argument(list,name);
	    if (name[0] == '\0')  /* this name was not found */
		return FALSE;

		if (strstr(name, string)==name) /*Zeran: hack for abbreviations*/
		return TRUE;  /*abbreviated string, return a match*/

	}
}

/* Work out if an object matches a given string... */

bool obj_called(OBJ_DATA *obj, char *str) {

    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list, *string;
    char namebuf[MAX_STRING_LENGTH]; 
    int liquid;

   /* Work out the objects full name string... */

    namebuf[0] = '\0';

    strcat(namebuf, obj->name);

    if (obj->item_type == ITEM_FOUNTAIN) {
      liquid = obj->value[0];
      if ( liquid >= 0 
        && liquid < LIQ_MAX) {
        strcat(namebuf, " ");
        strcat(namebuf, liq_table[liquid].liq_name); 
        strcat(namebuf, " ");
        strcat(namebuf, liq_table[liquid].liq_color);
        strcat(namebuf, " liquid");
      }
    } 

    if (obj->item_type == ITEM_DRINK_CON) {
      liquid = obj->value[2];
      if ( liquid >= 0 
        && liquid < LIQ_MAX) {
        strcat(namebuf, " ");
        strcat(namebuf, liq_table[liquid].liq_name); 
        strcat(namebuf, " ");
        strcat(namebuf, liq_table[liquid].liq_color);
        strcat(namebuf, " liquid");
      }
    } 

    string = str;

   /* We need ALL parts of string to match part of namelist */

    str = one_argument(str, part);

    while (part[0] != '\0') {

       /* Check the part against the complete name... */

	list = namebuf;

	list = one_argument(list, name);

	while (name[0] != '\0') {  /* start parsing namelist */
	
            if (!str_cmp(part, name)) {
		break;
            } 

	    list = one_argument(list, name);
	}

       /* Not a match if we fell off the end... */

	if (name[0] == '\0')  /* Current part has not matched */
	  return FALSE;

       /* Get the next part of the string to check... */

        str = one_argument(str, part);

    }

    return TRUE;
}

/* Work out if an abbreviated object matches a given string... */

bool obj_called_abbrev(OBJ_DATA *obj, char *str) {

    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list, *string;
    char namebuf[MAX_STRING_LENGTH]; 
    int liquid;

   /* Work out the objects full name string... */

    namebuf[0] = '\0';

    strcat(namebuf, obj->name);

    if (obj->item_type == ITEM_FOUNTAIN) {
      liquid = obj->value[0];
      if ( liquid >= 0 
        && liquid < LIQ_MAX) {
        strcat(namebuf, " ");
        strcat(namebuf, liq_table[liquid].liq_name); 
        strcat(namebuf, " ");
        strcat(namebuf, liq_table[liquid].liq_color);
        strcat(namebuf, " liquid");
      }
    } 

    if (obj->item_type == ITEM_DRINK_CON) {
      if (obj->value[1] == 0) {
        strcat(namebuf, " empty empties");
      } else {
        liquid = obj->value[2];
        if ( liquid >= 0 
          && liquid < LIQ_MAX) {
          strcat(namebuf, " ");
          strcat(namebuf, liq_table[liquid].liq_name); 
          strcat(namebuf, " ");
          strcat(namebuf, liq_table[liquid].liq_color);
          strcat(namebuf, " liquid");
        }
      }
    } 

    string = str;

   /* We need ALL parts of string to match part of namelist */

    str = one_argument(str, part);

    while (part[0] != '\0') {

       /* Check the part against the complete name... */

	list = namebuf;

	list = one_argument(list, name);

	while (name[0] != '\0') {  /* start parsing namelist */

            if (!str_prefix(part, name) ) {
		break;
            } 

	    list = one_argument(list, name);
	}

       /* Not a match if we fell off the end... */

	if (name[0] == '\0')  /* Current part has not matched */
	  return FALSE;

       /* Get the next part of the string to check... */

        str = one_argument(str, part);

    }

    return TRUE;
}

/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA *ch ) {
    OBJ_DATA *obj;
    CHAR_DATA *prev;

    if ( ch->in_room == NULL ) {
	bug( "Char_from_room: NULL.", 0 );
	return;
    }

   /* Decrement number of players in area... */

    if ( !IS_NPC(ch) ) {
	--ch->in_room->area->nplayer;
    }

   /* Decrement room light level... */

    obj = get_eq_char( ch, WEAR_LIGHT );

    if ( obj != NULL
      && obj->item_type == ITEM_LIGHT
      && obj->value[2] != 0
      && (obj->value[3] != 0 || obj->value[2] <0)
      && ch->in_room->light > 0 ) {
	ch->in_room->light -= 1;
    }

   /* Remove from lists... */

    if ( ch == ch->in_room->people ) {
	ch->in_room->people = ch->next_in_room;
    } else {
	for ( prev = ch->in_room->people; prev; prev = prev->next_in_room ) {

	    if ( prev->next_in_room == ch ) {
		prev->next_in_room = ch->next_in_room;
		break;
	    }
	}

	if ( prev == NULL ) {
	    bug( "Char_from_room: ch not found.", 0 );
        }
    }

   /* Null characters pointers... */

    ch->in_room      = NULL;
    ch->next_in_room = NULL;

   /* All done... */

    return;
}


/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) {
OBJ_DATA *obj, *next_obj;
PASSPORT *pass;
int zmask;
char buf[MAX_STRING_LENGTH];
bool login;

   /* Check parms... */

    if ( pRoomIndex == NULL ) {
	bug( "Char_to_room: NULL.", 0 );
	return;
    }

   /* Check they are not already somewhere else.. */

    if ( ch->in_room != NULL 
      && ch->in_room != pRoomIndex ) {
      bug("Char to room when already in room!", 0); 
      char_from_room(ch);
    }  

   /* Make the basic move... */

    ch->in_room		= pRoomIndex;
    ch->next_in_room	= pRoomIndex->people;
    pRoomIndex->people	= ch;

   /* Update area flags... */

    if ( !IS_NPC(ch) ) {

	if (ch->in_room->area->empty) {
	    ch->in_room->area->empty = FALSE;
	    ch->in_room->area->age = 0;
	}

	ch->in_room->area->nplayer += 1;
    }

   /* Check for zone change and filter equipment... */

    check_vanish(ch, ch->carrying);

    if ( ch->in_zone != ch->in_room->area->zone ) {

      zmask = zones[ch->in_room->area->zone].mask;

      login = TRUE; 
 
      if ( ch->in_zone != -1) {
        send_to_char("{mA shiver runs along your spine!{x\r\n", ch);
        login = FALSE;
      }

      if ( zmask != 0 ) {

        /* Hide carried stuff... */

         obj = ch->carrying;

         while (obj != NULL) {

           next_obj = obj->next_content;

           if ( obj->zmask != 0
             && (zmask & obj->zmask) == 0) {

             obj_from_char(obj);
             obj_to_char_ooz(obj, ch);  

             if (!login) {
               sprintf(buf, "{m%s evaporates!{x\r\n", obj->short_descr);
               buf[2] = UPPER(buf[2]);
               send_to_char(buf, ch);
             }
           }

           obj = next_obj;
         }

        /* Reveal hidden stuff... */
  
         obj = ch->ooz;

         while (obj != NULL) { 

           next_obj = obj->next_content;

           if ( obj->zmask != 0
             && (zmask & obj->zmask) != 0) {

             obj_from_char_ooz(obj);
             obj_to_char(obj, ch);

            /* Auto re-equip? */

             if (!login) {
               sprintf(buf, "{m%s condenses into your hands!{x\r\n", obj->short_descr);
               buf[2] = UPPER(buf[2]);
               send_to_char(buf, ch);
             }

           }

           obj = next_obj;
         }

      }

      ch->in_zone = ch->in_room->area->zone;
    }

   /* Update room lighting... */

    obj = get_eq_char( ch, WEAR_LIGHT );

    if ( obj != NULL
      && obj->item_type == ITEM_LIGHT
      && obj->value[2] != 0
      && (obj->value[3] != 0 || obj->value[2] <0)) {
	ch->in_room->light += 1;
    }

   /* Spread the plague... */

    if (IS_AFFECTED(ch,AFF_PLAGUE)) {
        AFFECT_DATA *af, plague;
        CHAR_DATA *vch;
        int save;
        
        for ( af = ch->affected; af != NULL; af = af->next ) {
            if (af->type == gsn_plague) break;
        }
        
        if (af == NULL) {
            REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
            return;
        }
        
        if (af->level == 1) return;
        
        plague.type 		= gsn_plague;
        plague.level 		= af->level - 1; 
        plague.duration 	= number_range(1,2 * plague.level);
        plague.location		= APPLY_STR;
        plague.modifier 	= -5;
        plague.bitvector 	= AFF_PLAGUE;
        
        for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
            switch(check_immune(vch,DAM_DISEASE)) {
            	case(IS_NORMAL) 	: save = af->level - 4;	break;
            	case(IS_IMMUNE) 	: save = 0;		break;
            	case(IS_RESISTANT) 	: save = af->level - 8;	break;
            	case(IS_RESISTANT_PLUS): save = af->level - 8;	break;
            	case(IS_VULNERABLE)	: save = af->level; 	break;
            	default			: save = af->level - 4;	break;
            }
            
            if (save != 0 && !saves_spell(save,vch) && !IS_IMMORTAL(vch)
            && !IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(6) == 0) {
            	send_to_char("You feel hot and feverish.\r\n",vch);
            	act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
            	affect_join(vch,&plague);
            }
        }
    }

    if ((pass = get_passport_order(ch)) != NULL) {
          bool ok = FALSE;
          CHAR_DATA *keeper = find_keeper(ch, FALSE);
          int i;

          if (keeper
          && can_see(keeper, ch)) {
              if (pass->year < time_info.year
              || (pass->year == time_info.year && pass->month < time_info.month)
              || (pass->year == time_info.year && pass->month == time_info.month && pass->day <= time_info.day)) {
                  for(i = 0; i < MAX_TRADE; i++) {
                      if (keeper->pIndexData->pShop->buy_type[i] == 994) ok = TRUE;
                  }
                  if (ok) {
                      OBJ_DATA *po = create_object(get_obj_index(OBJ_PASSPORT), 0);
                      po->owner = str_dup(ch->name); 
                      do_say(keeper, "I got something for you!");
                      sprintf_to_char(ch, "%s gives you %s\r\n", capitalize(keeper->short_descr), po->short_descr);
                      obj_to_char(po, ch);
                      free_pass_order(pass);
                      save_passport();
                  }
              }
          }
    }

    return;
}


/*
 * Give an obj to a char.
 */
void obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch ) {
    obj->next_content	 = ch->carrying;
    ch->carrying	 = obj;
    obj->carried_by	 = ch;
    obj->in_room	 = NULL;
    obj->in_obj		 = NULL;
    ch->carry_number	+= get_obj_number( obj );
    ch->carry_weight	+= get_obj_weight( obj );
}

/* Put an object into a chars ooz list... */

void obj_to_char_ooz( OBJ_DATA *obj, CHAR_DATA *ch ) {
    if (obj->trans_char != NULL) {
         extract_obj(obj);
         return;
    }

    obj->next_content	 = ch->ooz;
    ch->ooz		 = obj;
    obj->carried_by	 = ch;
    obj->in_room	 = NULL;
    obj->in_obj		 = NULL;
}


/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA *obj ) {
    CHAR_DATA *ch;

    ch = obj->carried_by;

    if ( ch == NULL ) {
	bug( "Obj_from_char: null ch.", 0 );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE ) {
	unequip_char( ch, obj );
    }

    if ( ch->carrying == obj ) {
	ch->carrying = obj->next_content;
    } else {
	OBJ_DATA *prev;

	for ( prev = ch->carrying; prev != NULL; prev = prev->next_content ) {

	    if ( prev->next_content == obj ) {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL ) {
                    char buf[MAX_STRING_LENGTH];

                    sprintf(buf, "%s Obj_from_char: obj not in list.", obj->name);
	    bug(buf, 0 ); 
                    exit(1);
                }
    }

    obj->carried_by	 = NULL;
    obj->next_content	 = NULL;

    ch->carry_number	-= get_obj_number( obj );
    ch->carry_weight	-= get_obj_weight( obj );

    return;
}



/* Take an obj from a chars ooz list */

void obj_from_char_ooz( OBJ_DATA *obj ) {

    CHAR_DATA *ch;

    ch = obj->carried_by;

    if ( ch == NULL ) {
	bug( "Obj_from_char_ooz: null ch.", 0 );
	return;
    }

    if ( ch->ooz == obj ) {
	ch->ooz = obj->next_content;
    } else {
	OBJ_DATA *prev;

	for ( prev = ch->ooz; prev != NULL; prev = prev->next_content ) {

	    if ( prev->next_content == obj ) {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL ) {
	    bug( "Obj_from_char_ooz: obj not in list.", 0 );
            exit(1);
        }
    }

    obj->carried_by	 = NULL;
    obj->next_content	 = NULL;

    return;
}


/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( OBJ_DATA *obj, int iWear, int type ) {
    if ( obj->item_type != ITEM_ARMOR )
	return 0;

    switch ( iWear )
    {
    case WEAR_BODY:	return 3 * obj->value[type];
    case WEAR_HEAD:	return 2 * obj->value[type];
    case WEAR_LEGS:	return 2 * obj->value[type];
    case WEAR_FEET:	return     obj->value[type];
    case WEAR_HANDS:	return     obj->value[type];
    case WEAR_ARMS:	return     obj->value[type];
    case WEAR_SHIELD:	return     obj->value[type];
    case WEAR_FINGER_L:	return     obj->value[type];
    case WEAR_FINGER_R: return     obj->value[type];
    case WEAR_NECK_1:	return     obj->value[type];
    case WEAR_NECK_2:	return     obj->value[type];
    case WEAR_ABOUT:	return 2 * obj->value[type];
    case WEAR_WAIST:	return     obj->value[type];
    case WEAR_WRIST_L:	return     obj->value[type];
    case WEAR_WRIST_R:	return     obj->value[type];
    case WEAR_HOLD:	return     obj->value[type];
    case WEAR_PRIDE: 	return     obj->value[type];
    case WEAR_FACE:	return     obj->value[type];
    case WEAR_EARS: 	return     obj->value[type];
    case WEAR_FLOAT: 	return     obj->value[type];
    case WEAR_EYES:     return     obj->value[type]; 
    case WEAR_BACK:     return     obj->value[type]; 
    case WEAR_TATTOO:     return     obj->value[type]; 
    }

    return 0;
}



/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
    OBJ_DATA *obj;

    if (ch == NULL)
	return NULL;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == iWear )
	    return obj;
    }

    return NULL;
}



/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) {
AFFECT_DATA *paf;
AFFECT_DATA af;
EXTRA_DESCR_DATA *ed;
char buf[MAX_STRING_LENGTH];
int i;

    if ( get_eq_char( ch, iWear ) != NULL )    {
	bug( "Equip_char: already equipped (%d).", iWear );
	return;
    }

    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)   && IS_EVIL(ch))
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch))
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))
    ||   (obj->owner != NULL && strcmp(obj->owner, ch->short_descr)))    {
	act( "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
	act( "$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM );
	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
	return;
    }

    for (i = 0; i < 4; i++)    	ch->armor[i]      	-= apply_ac( obj, iWear,i );

    obj->wear_loc	 = iWear;

    if (obj->item_type != ITEM_RAW && obj->item_type != ITEM_GEM && obj->item_type != ITEM_PASSPORT) {
           if (!obj->enchanted) {
  	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )	{
                                    if (paf->bitvector == AFF_POLY) {
                                         if (!IS_SET(ch->act, ACT_WERE)) {
                                         if (!is_affected(ch, gafn_mask_hide) && !IS_AFFECTED(ch, AFF_POLY)) {
                                             if (!is_affected(ch, gafn_mask_self) && ch->race_orig == 0) {
                                                   ch->race_orig = ch->race;
                                                   free_string(ch->description_orig);
                                                   free_string(ch->short_descr_orig);
                                                   free_string(ch->long_descr_orig);
                                                   ch->description_orig = str_dup (ch->description);
                                                   ch->short_descr_orig = str_dup (ch->short_descr);
                                                   ch->long_descr_orig  = str_dup (ch->long_descr);

                                                   free_string(ch->description);
                                                   free_string(ch->short_descr);
                                                   free_string(ch->long_descr);
                                                   free_string(ch->poly_name);

                                                   sprintf(buf, "A masked %s", race_array[ch->race].name); 
                                                   ch->short_descr = str_dup(buf);
                                                   ch->poly_name = str_dup(race_array[ch->race].name);
                                                   sprintf(buf, "A masked %s is here.\n", race_array[ch->race].name); 

                                                   ed = obj->pIndexData->extra_descr;
                                                   while (ed != NULL) {
                                                       if (is_name("mask_long", ed->keyword)) {
                                                           sprintf(buf, ed->description, race_array[ch->race].name);
                                                           break;
                                                       }           
                                                       ed = ed->next;
                                                   }

                                                   ch->long_descr  = str_dup (buf);
                                                   ch->description = str_dup(ch->long_descr);

                                                   af.type      	= SKILL_UNDEFINED;
                                                   af.afn	 	= gafn_mask_hide;
                                                   af.level     	= obj->level;
                                                   af.duration  	= -1;
                                                   af.location  	= APPLY_NONE;
                                                   af.modifier  	= 0;
                                                   af.bitvector 	= AFF_POLY;
                                                   affect_to_char( ch, &af );
                                                   send_to_char( "You mask yourself.\r\n", ch );
                                             }
                                         }
                                         }
                                    } else {
	                         affect_modify( ch, paf, TRUE ); 
                                    }
	}
           } else {
                for ( paf = obj->affected; paf != NULL; paf = paf->next )		{
                                    if (paf->bitvector == AFF_POLY) {
                                         if (!IS_SET(ch->act, ACT_WERE)) {
                                         if (!is_affected(ch, gafn_mask_hide) && !IS_AFFECTED(ch, AFF_POLY)) {
                                             if (!is_affected(ch, gafn_mask_self) && ch->race_orig == 0) {
                                                   ch->race_orig = ch->race;
                                                   free_string(ch->description_orig);
                                                   free_string(ch->short_descr_orig);
                                                   free_string(ch->long_descr_orig);
                                                   ch->description_orig = str_dup (ch->description);
                                                   ch->short_descr_orig = str_dup (ch->short_descr);
                                                   ch->long_descr_orig  = str_dup (ch->long_descr);

                                                   free_string(ch->description);
                                                   free_string(ch->short_descr);
                                                   free_string(ch->long_descr);
                                                   free_string(ch->poly_name);

                                                   sprintf(buf, "A masked %s", race_array[ch->race].name); 
                                                   ch->short_descr = str_dup(buf);
                                                   ch->poly_name = str_dup(race_array[ch->race].name);
                                                   sprintf(buf, "A masked %s is here.\n", race_array[ch->race].name); 
 
                                                   ed = obj->pIndexData->extra_descr;
                                                   while (ed != NULL) {
                                                       if (is_name("mask_long", ed->keyword)) {
                                                           sprintf(buf, ed->description, race_array[ch->race].name);
                                                           break;
                                                       }           
                                                       ed = ed->next;
                                                   }

                                                   ch->long_descr  = str_dup (buf);
                                                   ch->description = str_dup(ch->long_descr);

                                                   af.type      	= SKILL_UNDEFINED;
                                                   af.afn	 	= gafn_mask_hide;
                                                   af.level     	= obj->level;
                                                   af.duration  	= -1;
                                                   af.location  	= APPLY_NONE;
                                                   af.modifier  	= 0;
                                                   af.bitvector 	= AFF_POLY;
                                                   affect_to_char( ch, &af );
                                                   send_to_char( "You mask yourself.\r\n", ch );
                                             }
                                         }
                                         }
                                    } else {
		         affect_modify( ch, paf, TRUE );
                                    }
	}
           }

           if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)
           && obj->pIndexData->artifact_data) {
                   SET_BIT(ch->artifacted_by, obj->pIndexData->artifact_data->power);
           }           
    }
    if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    && (obj->value[3] != 0 || obj->value[2] <0)
    &&   ch->in_room != NULL )
	ch->in_room->light += 1;
	
    return;
}



/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj ) {
    AFFECT_DATA *paf;
    int i;

    if ( obj->wear_loc == WEAR_NONE )    {
	bug( "Unequip_char: already unequipped.", 0 );
	return;
    }

    for (i = 0; i < 4; i++) ch->armor[i]	+= apply_ac( obj, obj->wear_loc,i );
    obj->wear_loc	 = -1;

    if (obj->item_type != ITEM_RAW && obj->item_type != ITEM_GEM && obj->item_type != ITEM_PASSPORT) {
         if (!obj->enchanted) {
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )	{
                                    if (paf->bitvector == AFF_POLY) {
                                         if (!IS_SET(ch->act, ACT_WERE)) {
                                         if (is_affected(ch, gafn_mask_hide) && IS_AFFECTED(ch, AFF_POLY)) {
                                             if (!is_affected(ch, gafn_mask_self) && ch->race == ch->race_orig) {
                                                     if (ch->description) free_string(ch->description);
                                                     if (ch->short_descr) free_string(ch->short_descr);
                                                     if (ch->long_descr) free_string(ch->long_descr);
                                                     if (ch->poly_name) free_string(ch->poly_name);

                                                     ch->description = str_dup (ch->description_orig);
                                                     ch->short_descr = str_dup (ch->short_descr_orig); 
                                                     ch->long_descr  = str_dup (ch->long_descr_orig);
			     ch->poly_name = str_dup ("");
                                                     ch->race=ch->race_orig;
                                                     ch->race_orig = 0;
   
                                                     affect_strip_afn(ch, gafn_mask_hide);
                                                     send_to_char( "You unmask yourself.\r\n", ch );
                                             }
                                         }
                                         }
                                    } else {
	 	         affect_modify( ch, paf, FALSE );
                                    }
	}
         } else {
                for ( paf = obj->affected; paf != NULL; paf = paf->next )		{
                                    if (paf->bitvector == AFF_POLY) {
                                         if (!IS_SET(ch->act, ACT_WERE)) {
                                         if (is_affected(ch, gafn_mask_hide) && IS_AFFECTED(ch, AFF_POLY)) {
                                             if (!is_affected(ch, gafn_mask_self) && ch->race == ch->race_orig) {
                                                     if (ch->description) free_string(ch->description);
                                                     if (ch->short_descr) free_string(ch->short_descr);
                                                     if (ch->long_descr) free_string(ch->long_descr);
                                                     if (ch->poly_name) free_string(ch->poly_name);

                                                     ch->description = str_dup (ch->description_orig);
                                                     ch->short_descr = str_dup (ch->short_descr_orig); 
                                                     ch->long_descr  = str_dup (ch->long_descr_orig);
			     ch->poly_name = str_dup ("");
                                                     ch->race=ch->race_orig;
                                                     ch->race_orig = 0;
   
                                                     affect_strip_afn(ch, gafn_mask_hide);
                                                     send_to_char( "You unmask yourself.\r\n", ch );
                                             }
                                         }
                                         }
                                    } else {
		         affect_modify( ch, paf, FALSE );
                                    }
	}
         }

           if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)
           && obj->pIndexData->artifact_data) {
                  OBJ_DATA *allobj;
                  int powers = 0;

                  for (allobj = ch->carrying; allobj; allobj = allobj->next_content) {
                        if (allobj == obj) continue;
                        if (!IS_SET(allobj->extra_flags, ITEM_ARTIFACT)
                        || !allobj->pIndexData->artifact_data) continue;
                        if (allobj->pIndexData->artifact_data == 0) continue; 

                        powers ^= allobj->pIndexData->artifact_data->power;
                  }
                  ch->artifacted_by = powers;
           }           
    }

    if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    && (obj->value[3] != 0 || obj->value[2] <0)
    &&   ch->in_room != NULL
    &&   ch->in_room->light > 0 )
	ch->in_room->light -= 1; 
	
    return;
}



/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list )
{
    OBJ_DATA *obj;
    int nMatch;

    nMatch = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData == pObjIndex )
	    nMatch++;
    }

    return nMatch;
}



/*
 * Move an obj out of a room.
 */
void obj_from_room( OBJ_DATA *obj ) {
    ROOM_INDEX_DATA *in_room;

    if ( ( in_room = obj->in_room ) == NULL )
    {
	bug( "obj_from_room: NULL.", 0 );
	return;
    }

    if ( obj == in_room->contents )
    {
	in_room->contents = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = in_room->contents; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_room: obj not found.", 0 );
	    return;
	}
    }

   /* Decrement room light level... */

    if ( obj->item_type == ITEM_LIGHT
      && obj->value[2] != 0
      && (obj->value[3] != 0 || obj->value[2] <0)
      && in_room->light > 0 ) {
	in_room->light -= 1;
    }

    obj->in_room      = NULL;
    obj->next_content = NULL;
    return;
}



/*
 * Move an obj into a room.
 */
void obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) {

obj->next_content	= pRoomIndex->contents;
pRoomIndex->contents	= obj;
obj->in_room		= pRoomIndex;
obj->carried_by		= NULL;
obj->in_obj		= NULL;

   /* Increment room light level... */

    if ( obj->item_type == ITEM_LIGHT
    && obj->value[2] != 0
    && (obj->value[3] != 0 || obj->value[2] <0)) {
      pRoomIndex->light += 1;
    }

    return;
}



/*
 * Move an object into an object.
 */
void obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{
    obj->next_content		= obj_to->contains;
    obj_to->contains		= obj;
    obj->in_obj			= obj_to;
    obj->in_room		= NULL;
    obj->carried_by		= NULL;
    if (obj_to->pIndexData->vnum == OBJ_VNUM_PIT)
        obj->cost = 1; 

    for ( ; obj_to != NULL; obj_to = obj_to->in_obj )
    {
	if ( obj_to->carried_by != NULL )
	{
//	    obj_to->carried_by->carry_number += get_obj_number( obj );
	    obj_to->carried_by->carry_weight += get_obj_weight( obj );
	}
    }

    return;
}



/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_from;

    if ( ( obj_from = obj->in_obj ) == NULL )
    {
	bug( "Obj_from_obj: null obj_from.", 0 );
	return;
    }

    if ( obj == obj_from->contains )
    {
	obj_from->contains = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = obj_from->contains; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_obj: obj not found.", 0 );
	    return;
	}
    }

    obj->next_content = NULL;
    obj->in_obj       = NULL;

    for ( ; obj_from != NULL; obj_from = obj_from->in_obj )
    {
	if ( obj_from->carried_by != NULL )
	{
//	    obj_from->carried_by->carry_number -= get_obj_number( obj );
	    obj_from->carried_by->carry_weight -= get_obj_weight( obj );
	}
    }

    return;
}



/*
 * Extract an obj from the world.
 */
void extract_obj( OBJ_DATA *obj ) {
    CHAR_DATA *ch;
    OBJ_DATA *obj_content;
    OBJ_DATA *obj_next;

    ch = obj->carried_by;

    if ( obj->in_room != NULL ) {
	obj_from_room( obj );
    } else if ( obj->carried_by != NULL ) {
	obj_from_char( obj );
    } else if ( obj->in_obj != NULL ) {
	obj_from_obj( obj );
    }

    if (obj->trans_char != NULL) {
         if (IS_AFFECTED(obj->trans_char, AFF_MORF)) {
               act( "You $p in cold blood!",  ch, obj, NULL, TO_CHAR    );
               act( "$n destroys you in cold blood!", ch, NULL, obj->trans_char, TO_VICT    );
               act( "$n destroys $p in cold blood!",  ch, obj, NULL, TO_NOTVICT );
               affect_strip_afn(obj->trans_char, gafn_morf);
               REMOVE_BIT( obj->trans_char->affected_by, AFF_MORF );
               obj->trans_char->trans_obj = NULL;
               raw_kill(obj->trans_char, TRUE );
         } else {
               obj->trans_char->trans_obj = NULL;
         }
    }

    for ( obj_content = obj->contains; obj_content; obj_content = obj_next )    {
	obj_next = obj_content->next_content;
	extract_obj( obj->contains );
    }

    if ( object_list == obj )    {
	object_list = obj->next;
    }    else    {
	OBJ_DATA *prev;

	for ( prev = object_list; prev != NULL; prev = prev->next ) {

	    if ( prev->next == obj ) {
		prev->next = obj->next;
		break;
	    }
	}

	if ( prev == NULL 
                && obj->pIndexData != NULL) {
	    bug( "Extract_obj: obj %d not found.", obj->pIndexData->vnum );
	    return;
	}
    }

    {
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	for ( paf = obj->affected; paf != NULL; paf = paf_next )
	{
	    paf_next    = paf->next;
	    paf->next   = affect_free;
	    affect_free = paf;
	}
    }

    {
	EXTRA_DESCR_DATA *ed;
	EXTRA_DESCR_DATA *ed_next;

	for ( ed = obj->extra_descr; ed != NULL; ed = ed_next ) {
	    ed_next		= ed->next;
	    free_string( ed->description );
	    free_string( ed->keyword     );
	    ed->next=extra_descr_free;
	    extra_descr_free	= ed;
	}
    }

    free_string( obj->name);
    free_string( obj->description);
    free_string( obj->short_descr);
    free_string( obj->owner);



    if (obj->pIndexData == NULL) bug ("Bug: (extract_obj)  NULL obj->pIndexData", 0);
    else --obj->pIndexData->count;

    if (obj->reset != NULL) {
	    obj->reset->count--;
    }

    obj->reset  = NULL;

    obj->next	= obj_free;
    obj_free	= obj;

    return;
}



/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA *ch, bool fPull ) {
CHAR_DATA *wch;
OBJ_DATA *obj;
OBJ_DATA *obj_next;
ROOM_INDEX_DATA *location;
ROOM_INDEX_DATA *room;

    int i;

    if ( ch->in_room == NULL )  {
	bug( "Extract_char: NULL.", 0 );
	return;
    }
    
   /* Remove pets... */

    nuke_pets(ch);
    ch->pet = NULL; /* just in case */

   /* Remove followers... */

    if ( fPull ) {
	die_follower( ch );
    }
  
   /* Stop fighting... */
  
    stop_fighting( ch, TRUE );

   /* Lose hunted by info... */

    if (ch->huntedBy != NULL) {
      ch->huntedBy->prey = NULL;
      ch->huntedBy = NULL;
    }

   /* Lose hunting info... */

    if (ch->prey != NULL) {
      ch->prey->huntedBy = NULL;
      ch->prey = NULL;
    }

   /* Work out respawn BEFORE we remove them from the room... */

    location = NULL;

    if (!fPull) {

        if ( ch->awakening ) {
            location = ch->waking_room;
            ch->nightmare = FALSE;        
        }

        if ( location == NULL
          && ch->in_room != NULL
          && ch->in_room->respawn != 0 ) {

          location = get_room_index(ch->in_room->respawn);
        }

        if ( location == NULL
          && ch->in_room != NULL
          && ch->in_room->area != NULL
          && ch->in_room->area->respawn != 0 ) {

          location = get_room_index(ch->in_room->area->respawn);
        }

        if ( location == NULL
          && ch->in_room != NULL
          && ch->in_room->area != NULL
          && zones[ch->in_room->area->zone].respawn != 0 ) {

          location = get_room_index(zones[ch->in_room->area->zone].respawn);
        }

        if ( location == NULL
          && mud.respawn != 0 ) {
 
          location = get_room_index( mud.respawn );
        } 

        if ( location == NULL ) {

          location = ch->in_room;
        }
    }

   /* Remove from current room... */

    room = ch->in_room;

    char_from_room( ch );

   /* If it's a dead player, repair and send to respawn... */

    if ( !fPull ) {
        int sanity = get_sanity(ch);

       /* Reset transient condition... */

        ch->condition[COND_FOOD] = COND_STUFFED - 1;
        ch->condition[COND_DRINK] = COND_SLUSHY - 1;
        ch->condition[COND_DRUNK] = 0;

       /* A little insanity... */

        if (sanity > 32) {
          ch->sanity -= dice(1,6);
        } else if (sanity > 8) {
          ch->sanity -= dice(1,4);
        } else if (sanity > 1) {
          ch->sanity -= 1;
        }

       /* Move to new room... */

        char_to_room( ch, location);

       /* Undo dreaming if we need to... */

        if ( ch->awakening ) {
          ch->awakening     = FALSE;
          ch->nightmare = FALSE;
          ch->waking_room   = NULL;
          ch->dreaming_room = room;       /* So they can come back... */
          send_to_char("{mYou awaken with a deadly start!{x\r\n", ch); 
        } else {
          send_to_char("{mYou revive somewhere strange...{x\r\n", ch); 
        } 

	return;
    }

   /* Ok, really dead/quitting now... */

   /* Destroy all carried items... */

    for ( obj = ch->carrying; obj != NULL; obj = obj_next ) {
	obj_next = obj->next_content;
	extract_obj( obj );
    }
    
   /* Destroy all out of zone carried items... */

    for ( obj = ch->ooz; obj != NULL; obj = obj_next ) {
	obj_next = obj->next_content;
        obj_from_char_ooz( obj ); 
	extract_obj( obj );
    }
    
   /* Up the body count for monsters... */

    if ( IS_NPC(ch) ) {

	--ch->pIndexData->count;

	if (ch->reset != NULL) {
		--ch->reset->count;  /* Zeran - added */
        }
    }
	
   /* Undo any polymorphs or transfers... */ 

    if ( ch->desc != NULL && ch->desc->original != NULL ) {
	do_return( ch, "" );
    }

   /* Lose any reply references... */ 

    for ( wch = char_list; wch != NULL; wch = wch->next )   {
	if ( wch->reply == ch )  wch->reply = NULL;
    }
    ch->reply = NULL;

   /* Chop out of list of all characters... */

    if ( ch == char_list )  {
       char_list = ch->next;
    } else  {
	CHAR_DATA *prev;

	for ( prev = char_list; prev != NULL; prev = prev->next ) {
	    if ( prev->next == ch ) {
		prev->next = ch->next;
		break;
	    }
	}

	if ( prev == NULL ) {
	    bug( "Extract_char: char not found.", 0 );
	    return;
	}
    }

   /* Stop monitoring... */

    stop_monitors(ch);

   /* Lose the descriptor if we have one... */

    if ( ch->desc ) ch->desc->character = NULL;

   /* Lose a few other things... */

    if (ch->deeds != NULL) {
        free_deed(ch->deeds);
        ch->deeds = NULL;
    }
 
    if (ch->quests != NULL) {
        free_quest(ch->quests);
        ch->quests = NULL;
    }
 
    if (ch->mcb != NULL) {
        free_mcb(ch->mcb);
        ch->mcb = NULL;
    }

    for(i = 0; i < MOB_MEMORY; i++) {
      if (ch->memory[i] != NULL) {
        free_string(ch->memory[i]);
        ch->memory[i] = NULL;
      }
    }

   /* Free the memory... */  

    free_char( ch );
    return;
}


/* Find a char by their true name... */

CHAR_DATA *get_char_room_true(CHAR_DATA *ch, char *argument) {
CHAR_DATA *rch;

 /* Validate... */

  if ( ch == NULL
    || ch->in_room == NULL
    || argument == NULL
    || argument[0] != '@'
    || argument[1] == '\0' ) {
    return NULL;
  }  

 /* Search... */

  if (ch->in_room == NULL) return NULL;
  rch = ch->in_room->people;

  while (rch != NULL) {

    if (rch->true_name != NULL) {
      if ( rch->true_name[1] == argument[1] ) {
        if (!str_cmp(rch->true_name, argument)) {
          return rch;
        }
      }
    }

    rch = rch->next_in_room;
  }

  return NULL;
}

/*
 * Find a char in the room even if you cannot see them.
 */
CHAR_DATA *get_char_room_unseen( CHAR_DATA *ch, char *argument ){
char arg[MAX_INPUT_LENGTH];
CHAR_DATA *rch;
int number;
int count;
bool partial=FALSE;
CHAR_DATA *pch=NULL;
int pcount=0;

   /* Check for truename search... */

    if (argument[0] == '@') {
      return get_char_room_true(ch, argument);
    } 

   /* Ok, normal wordy search... */

    number = number_argument( argument, arg );

    count  = 0;

    if ( !str_cmp( arg, "self" )) return ch;
    if (ch->in_room == NULL) return NULL;
 
    for ( rch = ch->in_room->people; 
          rch != NULL; 
          rch = rch->next_in_room ) {

	if  ( !partial 
           && ( ( is_name_abbv( arg, rch->name) 
               || ( !IS_NPC(rch) 
                 && IS_AFFECTED(rch, AFF_POLY) 
                 && is_name_abbv (arg, rch->poly_name) 
                  )
                )  
              )) {                 /*abbreviation found, save*/
		partial=TRUE;
		pch=rch;
		pcount=count+1;
	}

	if ( ( !is_name( arg, rch->name )
            && (!IS_NPC(rch) ?
                  IS_AFFECTED(rch, AFF_POLY) ?
                     (!is_name(arg, rch->poly_name)) :
                      1 :
                  1 ) 
                )  ) {
	    continue;
        }

	if ( ++count == number ) {
	    return rch;
        }
    }
	
    if ( (partial) 
      && (pcount == number)) {
	return pch;
    } 

    return NULL;
}

/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA *ch, char *argument ) {
char arg[MAX_INPUT_LENGTH];
CHAR_DATA *rch;
int number;
int count;
bool partial=FALSE;
CHAR_DATA *pch=NULL;
int pcount=0;

   /* Check for truename search... */

    if (argument[0] == '@') return get_char_room_true(ch, argument);
   
   /* Ok, normal wordy search... */

    number = number_argument( argument, arg );

    count  = 0;

    if (!str_cmp(arg, "self")) return ch;
    if (ch->in_room == NULL) return NULL;
     
    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room ) {

           if  (!partial 
           && ( can_see( ch, rch ) 
           && ( is_name_abbv( arg, rch->name) 
           || ( !IS_NPC(rch) && IS_AFFECTED(rch, AFF_POLY) && is_name_abbv (arg, rch->poly_name))))) {                 /*abbreviation found, save*/
		partial=TRUE;
		pch=rch;
		pcount=count+1;
           }

            if ( !can_see( ch, rch ) 
            || ( !is_name( arg, rch->name )
            && (!IS_NPC(rch) ? IS_AFFECTED(rch, AFF_POLY) ? (!is_name(arg, rch->poly_name)) : 1 :  1 ))) {
	    continue;
            }

            if (++count == number && !IS_AFFECTED(rch, AFF_MIST)) return rch;
    }
    	
    if (partial 
    && pcount == number
    && !IS_AFFECTED(pch, AFF_MIST)) return pch;

    return NULL;
}


/* Find a char by their true name... */

CHAR_DATA *get_char_world_true(CHAR_DATA *ch, char *argument) {
CHAR_DATA *rch;

 /* Validate... */

  if (argument == NULL
    || argument[0] != '@'
    || argument[1] == '\0' ) {
    return NULL;
  }  

 /* Search... */

  rch = char_list;

  while (rch != NULL) {

    if (rch->true_name != NULL) {
      if ( rch->true_name[1] == argument[1] ) {
        if (!str_cmp(rch->true_name, argument)) {
          return rch;
        }
      }
    }

    rch = rch->next;
  }

  return NULL;
}



/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument ) {
char arg[MAX_INPUT_LENGTH];
CHAR_DATA *wch = NULL;
int number;
int count;
CHAR_DATA *pch=NULL;
bool partial=FALSE;
int pcount=0;

    if (!str_cmp ("self", argument)) return ch;

    if (ch != NULL) wch = get_char_room( ch, argument );
    if ( wch != NULL ) return wch;
    
   /* Check for truename search... */

    if (argument[0] == '@') {
      return get_char_world_true(ch, argument);
    } 

    /* Ok, slow search... */

    number = number_argument( argument, arg );
    count  = 0;

    for ( wch = char_list; wch != NULL ; wch = wch->next ) {
    
	if ( !partial 
                    && ( wch->in_room != NULL 
	    &&   is_name_abbv( arg, wch->name ) ) ) {
	    partial=TRUE;
	    pcount=count+1;
	    pch=wch;
        }

	if (  wch->in_room == NULL 
                || !is_name( arg, wch->name ) ) {
	    continue;
        }

	if ( ++count == number ) {
	    return wch;
        }
    }
    if ( partial 
      && pcount == number ) {
        return pch;
    }

    return NULL;
}


CHAR_DATA *get_char_world_player( CHAR_DATA *ch, char *argument ) {
char arg[MAX_INPUT_LENGTH];
CHAR_DATA *wch = NULL;
int number;
int count;
CHAR_DATA *pch=NULL;
bool partial=FALSE;
int pcount=0;

    if (!str_cmp ("self", argument)) return ch;

    if (ch != NULL) wch = get_char_room(ch, argument );
    if ( wch != NULL ) return wch;
    
   /* Check for truename search... */

    if (argument[0] == '@') {
      return get_char_world_true(ch, argument);
    } 

    /* Ok, slow search... */

    number = number_argument( argument, arg );
    count  = 0;

    for ( wch = char_list; wch != NULL ; wch = wch->next ) {
    
	if (!partial 
                && !IS_NPC(wch)
                && ( wch->in_room != NULL 
	&&  is_name_abbv( arg, wch->name )) ) {
	    partial=TRUE;
	    pcount=count+1;
	    pch=wch;
                }

	if (  wch->in_room == NULL || !is_name( arg, wch->name ) || IS_NPC(wch)) continue;
	if ( ++count == number ) return wch;
    }
    if ( partial && pcount == number) return pch;

    return NULL;
}


/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex )
{
    OBJ_DATA *obj;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( obj->pIndexData == pObjIndex )
	    return obj;
    }

    return NULL;
}


/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, char *argument, OBJ_DATA *list ) {
char arg[MAX_INPUT_LENGTH];
OBJ_DATA *obj;
int number;
int count;
OBJ_DATA *pobj=NULL;
bool partial=FALSE;
int pcount=0;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content ) {
    
	if (!partial 
          && can_see_obj(ch, obj) 
          && obj_called_abbrev(obj, arg) ) {
		
		pobj=obj;
		pcount=count+1;
		partial=TRUE;
	}

	if ( can_see_obj( ch, obj ) 
          && obj_called(obj, arg) ) {
	
            if ( ++count == number )
		return obj;
	}
    }

    if ((partial) && (pcount==number))
	return pobj;

    return NULL;
}



/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;
    OBJ_DATA *pobj=NULL;
    bool partial=FALSE;
    int pcount=0;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content ) {
    
	if (!partial 
          && obj->wear_loc == WEAR_NONE
	  && can_see_obj( ch, obj ) 
	  && obj_called_abbrev(obj, arg) ) {
		
		partial=TRUE;
		pcount=count+1;
		pobj=obj;
	}

	if ( obj->wear_loc == WEAR_NONE
	  && can_see_obj(ch, obj) 
	  && obj_called(obj, arg) ) {
	
	    if ( ++count == number )
		return obj;
	}
    }

    if ((partial) && (pcount==number))
	return pobj;

    return NULL;
}



/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument ) {

    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;
    int pcount=0;
    OBJ_DATA *pobj=NULL;
    bool partial=FALSE;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content ) {

	if ( !partial 
          && obj->wear_loc != WEAR_NONE
	  && can_see_obj(ch, obj)
	  && obj_called_abbrev(obj, arg) ) {
		
		partial=TRUE;
		pcount = count+1;
		pobj = obj;
	}

	if ( obj->wear_loc != WEAR_NONE
	  && can_see_obj( ch, obj )
	  && obj_called(obj, arg) ) {
	
	    if ( ++count == number )
		return obj;
	}
    }

    if ((partial) && (pcount == number))
 	return pobj;

    return NULL;
}



/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, char *argument ) {
    OBJ_DATA *obj;

   /* Check the room... */

    obj = get_obj_list( ch, argument, ch->in_room->contents );

    if ( obj != NULL )
	return obj;

   /* Check inventory... */

    obj = get_obj_carry( ch, argument );

    if ( obj != NULL )
	return obj;

   /* Check equipment... */

    obj = get_obj_wear( ch, argument );

    if ( obj != NULL )
	return obj;

   /* Not found anywhere... */

    return NULL;
}


OBJ_DATA *get_obj_room(ROOM_INDEX_DATA *room, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;
    OBJ_DATA *pobj=NULL;
    bool partial=FALSE;
    int pcount=0;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = room->contents; obj != NULL; obj = obj->next_content ) {
          if (!partial && obj_called_abbrev(obj, arg) ) {
		pobj=obj;
		pcount=count+1;
		partial=TRUE;
	}

	if (obj_called(obj, arg) ) {
	            if ( ++count == number ) return obj;
	}
    }

    if ((partial) && (pcount==number)) return pobj;
    return NULL;
}


/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;
    int pcount=0;
    bool partial=FALSE;
    OBJ_DATA *pobj=NULL;

    if ( ( obj = get_obj_here( ch, argument ) ) != NULL )
	return obj;

    number = number_argument( argument, arg );
    count  = 0;

    for ( obj = object_list; obj != NULL; obj = obj->next ) {
    
	if (!partial 
          && can_see_obj( ch, obj ) 
          && obj_called_abbrev(obj, arg) ) {
		
		partial=TRUE;
		pcount=count+1;
		pobj=obj;
	}

	if ( can_see_obj( ch, obj ) 
          && obj_called(obj, arg) ) {

	    if ( ++count == number )
		return obj;
	}
    }

    if ((partial) && (pcount==number))
        return pobj;

    return NULL;
}



/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money( int amount, int currency ){
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    if ( amount <= 0 )    {
	bug( "Create_money: zero or negative money %d.", amount );
	amount = 1;
    }

    if ( amount == 1 )   {
	obj = create_object( get_obj_index( OBJ_VNUM_MONEY_ONE ), 0 );
                sprintf(buf, obj->name, flag_string(currency_accept, currency));
                free_string(obj->name);
                obj->name = str_dup(buf);
                sprintf(buf, obj->short_descr, flag_string(currency_type, currency));
                free_string(obj->short_descr);
                obj->short_descr = str_dup(buf);
                sprintf(buf, obj->description, flag_string(currency_type, currency));
                free_string(obj->description);
                obj->description = str_dup(buf);
                obj->value[1] = currency;

    } else  {
	obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SOME ), 0 );
                sprintf(buf, obj->name, flag_string(currency_accept, currency));
                free_string(obj->name);
                obj->name = str_dup(buf);
	sprintf( buf, obj->short_descr, amount, flag_string(currency_type, currency));
	free_string( obj->short_descr );
	obj->short_descr	= str_dup( buf );
                sprintf(buf, obj->description, flag_string(currency_type, currency));
                free_string(obj->description);
                obj->description = str_dup(buf);
	obj->value[0]		= amount;
	obj->cost		= amount;
                obj->value[1] = currency;
    }

    return obj;
}


int get_obj_number( OBJ_DATA *obj ) {
return 1;
}


/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA *obj ) {
    int weight;
    int oweight;

    OBJ_DATA *cobj;

   /* Zeran - if object is a floating object, weight is irrelevant... */
   /* So is weight of contained items...                              */

    if (IS_SET(obj->wear_flags, ITEM_WEAR_FLOAT)) {
 	return 0;
    }

   /* Start with the base weight of the object... */ 

    weight  = obj->weight;

   /* Add the weight of its contents... */

    for ( cobj = obj->contains; cobj != NULL; cobj = cobj->next_content ) {

	oweight = get_obj_weight( cobj );

        if ( obj->item_type == ITEM_CONTAINER 
          && obj->value[3] != 100 ) {
          oweight = ( obj->value[3] * oweight ) / 100;
        } 

        weight += oweight;
    } 

    return weight;
}


/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex ) {
   CHAR_DATA *rch, *rch_next;
    
    for (rch = pRoomIndex->people; rch != NULL; rch = rch_next)    {
           rch_next = rch->next_in_room;
           if (IS_AFFECTED(rch, AFF_DARKNESS)) return TRUE;
    }

    if (IS_RAFFECTED(pRoomIndex, RAFF_DARKNESS)) return TRUE;
    
    if ( pRoomIndex->light > 0 ) return FALSE;


  /* By default some places are always dark and always light... */

    switch (pRoomIndex->sector_type) {

      case SECT_INSIDE:
      case SECT_CITY:
      case SECT_SMALL_FIRE:
      case SECT_FIRE:
      case SECT_BIG_FIRE:
      case SECT_SPACE:
      case SECT_LIGHTNING:
      case SECT_ACID:
      case SECT_COLD:
      case SECT_HOLY:

       /* ...unless explicitly dark */

        if ( IS_SET(pRoomIndex->room_flags, ROOM_DARK) )
	  return TRUE;

	return FALSE; 

      case SECT_EVIL:
      case SECT_UNDERWATER:
      case SECT_UNDERGROUND:

       /* ...unless explicitly not dark */

        if ( !IS_SET(pRoomIndex->room_flags, ROOM_DARK) )
	  return FALSE;

        return TRUE; 

      default:
        break; 
    }

   /* Daylight unless explicitly dark... */

    if ( IS_SET(pRoomIndex->room_flags, ROOM_DARK) )
	return TRUE;

   /* The rest follow the weather... */

    if ( weather_info.sunlight == SUN_SET
    ||   weather_info.sunlight == SUN_DARK )
	return TRUE;

    return FALSE;
}



/*
 * True if room is private.
 */
bool room_is_private(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) {
ROOM_INDEX_DATA *croom;
CHAR_DATA *rch;
int count, iroom;

    count = 0;
    for ( rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room ) {
                if (!IS_NPC(rch)) count++;
    }
                
    if ( IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)  && count >= 4 ) return TRUE;
    if ( IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1 ) return TRUE;

    if (IS_SET(pRoomIndex->area->area_flags, AREA_SOLITARY)) {
           for (iroom = pRoomIndex->area->lvnum; iroom <= pRoomIndex->area->uvnum; iroom++) {
                croom = get_room_index(iroom);
                count = 0;
                for (rch = pRoomIndex->people; rch; rch = rch->next_in_room ) {
                      if (!IS_NPC(rch) && ch != rch) count++;
                }               
           }
           if (count >= 1) return TRUE;
    }
    
    return FALSE;
}

/* visibility on a room -- for entering and exits */
bool can_see_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) {
CHAR_DATA *partner;

    if (IS_SET(pRoomIndex->room_flags, ROOM_MUDGATE)) {
         if (IS_NEWBIE(ch)) return FALSE;
         if (IS_SET(ch->in_room->room_flags, ROOM_VEHICLE)) return FALSE;
         if ((partner = get_gate_partner(pRoomIndex)) == NULL) return FALSE;
    }

    if ( IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) 
    && !IS_IMP(ch)) {
	return FALSE;
    }

    if ( IS_SET(pRoomIndex->room_flags, ROOM_GODS_ONLY)
    && !IS_IMMORTAL(ch)) {
	return FALSE;
    }

    if ( IS_SET(pRoomIndex->room_flags, ROOM_HEROES_ONLY)
    && !IS_HERO(ch)) {
	return FALSE;
    }

    if (( IS_SET(pRoomIndex->room_flags,ROOM_NEWBIES_ONLY) || IS_SET(pRoomIndex->area->area_flags, AREA_NEWBIE))
    && !IS_NEWBIE(ch)
    && !IS_HELPER(ch)   
    && !IS_IMMORTAL(ch)) {
	return FALSE;
    } 

    return TRUE;
}



/*
 * True if char can see unhidden victim.
 */
bool can_see_hidden( CHAR_DATA *ch, CHAR_DATA *victim ) {
/* RT changed so that WIZ_INVIS has levels */

    MOB_CMD_CONTEXT *mcc;

   /* You can always see yourself... */

    if ( ch == victim ) {
	return TRUE;
    }
    
   /* Holylight can see everything... */

    if ( IS_SET(ch->plr, PLR_HOLYLIGHT) ) {
      return TRUE;
    }

   /* Wizi victims can be seen sometimes... */ 

    if ( IS_SET(victim->plr, PLR_WIZINVIS)
      && invis_lev( ch ) <= victim->invis_level ) {
       return FALSE;
    }

   /* Special vision conditions... */

    if ( ch->pIndexData != NULL 
      && ch->pIndexData->can_see != NULL ) {

      mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 0, NULL);

      if (ec_satisfied_mcc(mcc, ch->pIndexData->can_see, TRUE)) {
        free_mcc(mcc);
        return TRUE; 
      }

      free_mcc(mcc);
    } 

   /* Blind people cannot see anyone... */

    if ( IS_AFFECTED(ch, AFF_BLIND) ) {
	return FALSE;
    }

   /* Cloaked players can be seen in the same room... */
	
    if ( IS_SET(victim->plr, PLR_CLOAK)
      && invis_lev( ch ) < victim->cloak_level 
      && ch->in_room != victim->in_room ) {
	return FALSE;	
    }

   /* Only those with IR or DV can see in the dark... */
  
    if ( room_is_dark( ch->in_room ) 
     && !IS_AFFECTED(ch, AFF_INFRARED) 
     && !IS_AFFECTED(ch, AFF_DARK_VISION) ) {
	return FALSE; 
    }

   /* Only those with detect invisible can see invisible people... */

    if (  IS_AFFECTED(victim, AFF_INVISIBLE)
      && !IS_AFFECTED(ch, AFF_DETECT_INVIS) ) {
	return FALSE;
    }

   /* Special vision conditions... */

    if ( ch->pIndexData != NULL 
      && ch->pIndexData->can_not_see != NULL ) {

      mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 0, NULL);

      if (ec_satisfied_mcc(mcc, ch->pIndexData->can_not_see, TRUE)) {
        free_mcc(mcc);
        return FALSE; 
      }

      free_mcc(mcc);
    } 

   /* Otherwise, anyone can see anyone else... */

    return TRUE;
}


/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim ) {
    MOB_CMD_CONTEXT *mcc;

   /* You can always see yourself... */

    if ( ch == victim ) return TRUE;
    if (IS_AFFECTED(victim, AFF_MORF)) return FALSE;

    if (IS_MUD(victim)) {
        if (IS_IMP(ch)) return TRUE;
        else return FALSE;
    }    

    if (IS_SET(victim->act, ACT_WATCHER) || IS_SET(victim->plr, PLR_AFK)) {
        if (IS_IMMORTAL(ch)) return TRUE;
        else return FALSE;
    }    

   /* Holylight can see everything... */
    if ( IS_SET(ch->plr, PLR_HOLYLIGHT) ) return TRUE;

   /* Special vision conditions... */

    if ( ch->pIndexData != NULL 
      && ch->pIndexData->can_see != NULL ) {

      mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 0, NULL);

      if (ec_satisfied_mcc(mcc, ch->pIndexData->can_see, TRUE)) {
        free_mcc(mcc);
        return TRUE; 
      }

      free_mcc(mcc);
    } 

   /* Can't see them if you can't see them... */

    if (!can_see_hidden(ch, victim)) return FALSE;
    
   /* Detect hidden can see sneaking players... */
    
    if (IS_NPC(victim)
    || victim->pcdata->style != STYLE_NINJITSU) {
         if ( IS_AFFECTED(victim, AFF_SNEAK)
         && !IS_AFFECTED(ch, AFF_DETECT_HIDDEN)
         &&  victim->fighting == NULL ) {
             return FALSE;
         }

   /* Detect hidden can also see hidden players... */

         if (  IS_AFFECTED(victim, AFF_HIDE)
         && !IS_AFFECTED(ch, AFF_DETECT_HIDDEN)
         &&  victim->fighting == NULL ) {
	return FALSE;
         }
    } else {
         if (IS_AFFECTED(victim, AFF_HIDE)) {
            if (ch->level - victim->level -50 + number_percent() < get_skill(victim, gsn_hide)) return FALSE;
         }
         if (IS_AFFECTED(victim, AFF_SNEAK)) {
            if (ch->level - victim->level -50 + number_percent() < get_skill(victim, gsn_sneak)) return FALSE;
         }
    }

    return TRUE;
}


/*
 * True if char can see obj.
 */
bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj ) {

   /* Holy light can see everything... */
    if ( IS_SET(ch->plr, PLR_HOLYLIGHT)) return TRUE;

   /* Vis-death objects only appear upon death... */
    if ( IS_SET(obj->extra_flags,ITEM_VIS_DEATH)) return FALSE;

   /* Blind people can only see potions... */

    if ( IS_AFFECTED( ch, AFF_BLIND ) 
      && obj->item_type != ITEM_POTION) {
	return FALSE;
    }

   /* Concealed objects can only be seen with detect hidden... */
	
    if ( IS_SET(obj->extra_flags, ITEM_CONCEALED)
      && !IS_AFFECTED(ch, AFF_DETECT_HIDDEN) ) {
	return FALSE;
    }

   /* Invisible things can only be seen with detect invisible... */

    if ( IS_SET(obj->extra_flags, ITEM_INVIS)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) ) {
        return FALSE;
    }

   /* Light sources with fuel can always be seen... */

    if ( obj->item_type == ITEM_LIGHT 
      && obj->value[2] != 0 ) {
	return TRUE;
    }

   /* Glowing items can always be seen... */

    if ( IS_OBJ_STAT(obj, ITEM_GLOW)) {
	return TRUE;
    }

   /* Those with infra_red and dark vision can see in the dark... */

    if (room_is_dark(ch->in_room)) {
         if ((!IS_AFFECTED(ch, AFF_INFRARED)
         && !IS_AFFECTED(ch, AFF_DARK_VISION))
         || IS_OBJ_STAT(obj, ITEM_DARK)) {
	return FALSE;
         }
    }

    return TRUE;
}


/*
 * True if char can see obj.
 */
bool can_see_obj_aura(CHAR_DATA *ch, OBJ_DATA *obj ) {

   /* Holy light can see everything... */
    if ( IS_SET(ch->plr, PLR_HOLYLIGHT)) return TRUE;

   /* Vis-death objects only appear upon death... */
    if ( IS_SET(obj->extra_flags,ITEM_VIS_DEATH)) return FALSE;

   /* Blind people can only see potions... */

    if ( IS_AFFECTED( ch, AFF_BLIND ) 
      && obj->item_type != ITEM_POTION) {
	return FALSE;
    }

   /* Concealed objects can only be seen with detect hidden... */
	
    if ( IS_SET(obj->extra_flags, ITEM_CONCEALED)
      && !IS_AFFECTED(ch, AFF_DETECT_HIDDEN) ) {
	return FALSE;
    }

    if ( IS_SET(obj->extra_flags, ITEM_MAGIC)
      && IS_AFFECTED(ch, AFF_DETECT_MAGIC)) {
	return TRUE;
    }

   /* Invisible things can only be seen with detect invisible... */

    if ( IS_SET(obj->extra_flags, ITEM_INVIS)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) ) {
        return FALSE;
    }

   /* Light sources with fuel can always be seen... */

    if ( obj->item_type == ITEM_LIGHT 
      && obj->value[2] != 0 ) {
	return TRUE;
    }

   /* Glowing items can always be seen... */

    if ( IS_OBJ_STAT(obj, ITEM_GLOW)) return TRUE;

   /* Those with infra_red and dark vision can see in the dark... */

    if (room_is_dark(ch->in_room)) {
         if ((!IS_AFFECTED(ch, AFF_INFRARED)
         && !IS_AFFECTED(ch, AFF_DARK_VISION))
         || IS_OBJ_STAT(obj, ITEM_DARK)) {
	return FALSE;
         }
    }

    return TRUE;
}


/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj ) {
    if ( !IS_SET(obj->extra_flags, ITEM_NODROP) )
	return TRUE;

    if ( !IS_NPC(ch) 
      && IS_IMMORTAL(ch) )
	return TRUE;

    return FALSE;
}



/*
 * Return ascii name of an item type.
 */
char *item_type_name( OBJ_DATA *obj ){
    switch ( obj->item_type )    {
    case ITEM_LIGHT:		return "light";
    case ITEM_LIGHT_REFILL:	return "light_refill";
    case ITEM_SCROLL:		return "scroll";
    case ITEM_WAND:		return "wand";
    case ITEM_STAFF:		return "staff";
    case ITEM_WEAPON:		return "weapon";
    case ITEM_TREASURE:	return "treasure";
    case ITEM_ARMOR:		return "armor";
    case ITEM_POTION:		return "potion";
    case ITEM_FURNITURE:	return "furniture";
    case ITEM_TRASH:		return "trash";
    case ITEM_CONTAINER:	return "container";
    case ITEM_DRINK_CON:	return "drink";
    case ITEM_KEY:		return "key";
    case ITEM_FOOD:		return "food";
    case ITEM_MONEY:		return "money";
    case ITEM_BOAT:		return "boat";
    case ITEM_CORPSE_NPC:	return "npc_corpse";
    case ITEM_CORPSE_PC:	return "pc_corpse";
    case ITEM_FOUNTAIN:	return "fountain";
    case ITEM_PILL:		return "pill";
    case ITEM_MAP:		return "map";
    case ITEM_PRIDE:            	return "pride";
    case ITEM_COMPONENT:        	return "component";
    case ITEM_PORTAL:           	return "portal";
    case ITEM_LOCKER:           	return "locker";
    case ITEM_LOCKER_KEY:       	return "locker_key";
    case ITEM_CLAN_LOCKER:      	return "clan_locker";
    case ITEM_KEY_RING:         	return "key_ring";
    case ITEM_BOOK:             	return "book";
    case ITEM_IDOL:             	return "idol";
    case ITEM_SCRY:             	return "scry";
    case ITEM_DIGGING:          	return "digging";
    case ITEM_FORGING:          	return "forging";
    case ITEM_RAW:              	return "raw_material";
    case ITEM_BONDAGE:          	return "bondage";
    case ITEM_TATTOO:           	return "tattoo";
    case ITEM_TRAP:             	return "trap";
    case ITEM_DOLL:             	return "voodoo_doll";
    case ITEM_PAPER:            	return "paper";
    case ITEM_EXPLOSIVE:        	return "explosive";
    case ITEM_AMMO:             	return "ammo";
    case ITEM_SLOTMACHINE:      	return "slotmachine";
    case ITEM_HERB:             	return "herb";
    case ITEM_PIPE:             	return "pipe";
    case ITEM_TIMEBOMB:         	return "time_bomb";
    case ITEM_TREE:	        	return "tree";
    case ITEM_CAMERA:	        	return "camera";
    case ITEM_PHOTOGRAPH:	return "photograph";
    case ITEM_WARP_STONE:       	return "warp_stone";
    case ITEM_ROOM_KEY:         	return "room_key";
    case ITEM_GEM:              	return "gem";
    case ITEM_JEWELRY:          	return "jewelry";
    case ITEM_JUKEBOX:          	return "jukebox";
    case ITEM_SHARE:          	return "share";
    case ITEM_FIGURINE:          	return "figurine";
    case ITEM_POOL:          		return "pool";
    case ITEM_GRIMOIRE:        	return "grimoire";
    case ITEM_FOCUS:        	return "focus";
    case ITEM_PROTOCOL:        	return "protocol";
    case ITEM_DECORATION:        	return "decoration";
    case ITEM_PASSPORT:        	return "passport";
    case ITEM_INSTRUMENT:        	return "instrument";
    }

    bug( "Item_type_name: unknown type %d.", obj->item_type );
    return "(unknown)";
}

/* Names for Extra flags... */

static const struct extra_map_type {
long long bits;
char *label;
} extra_map[] = {
    {	ITEM_GLOW,			" glow"},
    {	ITEM_HUM,			" hum"},
    {	ITEM_DARK,			" dark"},
    {	ITEM_NEUTRAL,		" neutral"},
    {	ITEM_EVIL,			" evil"},
    {	ITEM_INVIS,			" invis"},
    {	ITEM_MAGIC,			" magic"},
    {	ITEM_NODROP,			" nodrop"},
    {	ITEM_BLESS,			" bless"},
    {	ITEM_ANTI_GOOD,		" anti_good"},
    {	ITEM_ANTI_EVIL,		" anti_evil"},
    {	ITEM_ANTI_NEUTRAL,		" anti_neutral"},
    {	ITEM_NOREMOVE,		" noremove"},
    {	ITEM_NO_SAC,			" no_sac"},
    {	ITEM_NO_COND,		" no_cond"},
    {	ITEM_CONCEALED,		" concealed"},
    {	ITEM_INVENTORY,		" inventory"},
    {	ITEM_NOPURGE,		" no_purge"},
    {	ITEM_VIS_DEATH,		" vis_death"},
    {	ITEM_ROT_DEATH,		" rot_death"},
    {	ITEM_NODISARM,		" nodisarm"},
    {	ITEM_HYPER_TECH,		" hypertech"},
    {	ITEM_ANIMATED,		" animated"},
    {	ITEM_MELT_DROP,		" melt_drop"},
    {	ITEM_NOLOCATE,		" no_locate"},
    {	ITEM_SELL_EXTRACT,		" sell_extract"},
    {	ITEM_NOUNCURSE,		" no_uncurse"},
    {	ITEM_BURN_PROOF,		" burn_proof"},
    {	ITEM_ARTIFACT,		" artifact"},
    {	ITEM_VANISH,			" vanish"},
    {	0,			NULL			}
};

char *extra_bit_name(long long extra_flags ) {
static char buf[512];
int i;
buf[0] = '\0';
i = 0;

    while (extra_map[i].bits != 0) {

      if (extra_map[i].bits & extra_flags) {
        strcat(buf, extra_map[i].label);
      }
 
      i += 1;
    }

    return (char*)( ( buf[0] != '\0' ) ? buf+1 : "none" );
}


/* Names for Nature flags... */

static const struct nature_map_type {
    long  	 bits;
    char	*label;
} nature_map[] = {
    {	NATURE_STRONG,		" strong"},
    {	NATURE_FEEBLE,		" feeble"},
    {	NATURE_SMART,		" smart"},
    {	NATURE_DUMB,		" dumb"},
    {	NATURE_AGILE,		" agile"},
    {	NATURE_LUMBERING,		" lumbering"},
    {	NATURE_SLY,			" sly"},
    {	NATURE_GULLIBLE,		" gullible"},
    {	NATURE_ROBUST,		" robust"},
    {	NATURE_SICKLY,		" sickly"},
    {	NATURE_STURDY,		" sturdy"},
    {	NATURE_FRAGILE,		" fragile"},
    {	NATURE_MAGICAL,		" magical"},
    {	NATURE_MUNDAIN,		" mundain"},
    {	NATURE_VISCIOUS,		" viscious"},
    {	NATURE_HARMLESS,		" harmless"},
    {	NATURE_ARMOURED,		" armoured"},
    {	NATURE_EXPOSED,		" exposed"},
    {	NATURE_MONSTEROUS,	" monsterous"},
    {	NATURE_LUCKY,		" lucky"},
    {	NATURE_UNLUCKY,		" unlucky"},
    {	NATURE_CHARISMATIC,	" charismatic"},
    {	NATURE_DISGUSTING,		" disgusting"},
    {	0,			NULL			}
};

char *nature_bit_name( long nature_flags ) {

    static char buf[512];

    int i;

    buf[0] = '\0';

    i = 0;

    while (nature_map[i].bits != 0) {

      if (nature_map[i].bits & nature_flags) {
        strcat(buf, nature_map[i].label);
      }
 
      i += 1;
    }

    return (char*)( ( buf[0] != '\0' ) ? buf+1 : "none" );
}


/* Names for ACT flags... */

static const struct act_map_type {
    long long  bits;
    char	*label;
} act_map[] = {
    {	ACT_IS_NPC,			" npc"			},
    {	ACT_SENTINEL,			" sentinel"		},
    {	ACT_SCAVENGER,		" scavenger"		},
    {	ACT_NIGHT,			" night-active"		},
    {	ACT_AGGRESSIVE,		" aggressive"		},
    {	ACT_STAY_AREA,		" stay_area"		},
    {	ACT_WIMPY,			" wimpy"		},
    {	ACT_PET,			" pet"			},
    {	ACT_FOLLOWER,		" follower"		},
    {	ACT_PRACTICE,		" practice"		},
    {	ACT_VAMPIRE,			" vampire"		},
    {	ACT_BOUND,			" bound"		},
    {	ACT_WERE,			" were"		},
    {	ACT_UNDEAD,			" undead"		},
    {	ACT_MOUNT,			" mount"		},
    {	ACT_CLERIC,			" cleric"		},
    {	ACT_MAGE,			" mage"			},
    {	ACT_THIEF,			" thief"		},
    {	ACT_WARRIOR,		" warrior"		},
    {	ACT_NOALIGN,			" no_align"		},
    {	ACT_NOPURGE,			" no_purge"		},
    {	ACT_DREAM_NATIVE,		" dream-native"		},
    {	ACT_BRAINSUCKED,		" brainsucked"		},
    {	ACT_CRIMINAL,		" criminal"		},
    {	ACT_IS_ALIENIST,		" alienist"		},
    {	ACT_IS_HEALER,		" healer"		},
    {	ACT_TELEPOP,			" telepop"		},
    {	ACT_UPDATE_ALWAYS,	" update_always"	},
    {	ACT_RAND_KIT,		" rand_kit"		},
    {	ACT_STAY_SUBAREA,		" stay_subarea"		},
    {	ACT_INVADER,			" invader"		},
    {	ACT_WATCHER,		" watcher"		},
    {	ACT_PROTECTED,		" protected"		},
    {	ACT_MARTIAL,			" martial"		},
    {	0,			NULL			}
};


static const struct plr_map_type {
    long long  	 bits;
    char	*label;
} plr_map[] = {
    {PLR_IS_NPC,			" is_npc!"},
    {PLR_BOUGHT_PET,		" owner"},
    {PLR_AUTOASSIST,		" autoassist"},
    {PLR_AUTOEXIT,		" autoexit"},
    {PLR_AUTOLOOT,		" autoloot"},
    {PLR_AUTOSAC,		" autosac"},
    {PLR_AUTOGOLD,		" autogold"},
    {PLR_AUTOSPLIT,		" autosplit"},
    {PLR_PK,			" player_killer"},
    {PLR_INJURED,		" player_injured"},
    {PLR_PERMADEATH,		" perma-death"},
    {PLR_IMP_HTML,		" imp-html"},
    {PLR_AUTOKILL,		" autokill"},
    {PLR_HOLYLIGHT,		" holylight"},
    {PLR_WIZINVIS,		" wizinvis"},
    {PLR_CANLOOT,		" loot_corpse"},
    {PLR_NOSUMMON,		" nosummon"},
    {PLR_NOFOLLOW,		" nofollow"},
    {PLR_XINFO,			" xinfo"},
    {PLR_COLOUR,		" colour"},
    {PLR_CURSOR,		" cursor"},
    {PLR_PUEBLO,		" pueblo"},
    {PLR_LOG,			" log"},
    {PLR_DENY,			" deny"},
    {PLR_FREEZE,			" freeze"},
    {PLR_AUTOCON,		" autoconsider"},
    {PLR_ANNOYING,		" annoying"},
    {PLR_AFK,			" afk"},
    {PLR_HELPER,			" helper"},
    {PLR_AUTOSAVE,		" autosave"},
    {PLR_CLOAK,			" cloak"},
    {PLR_FEED,			" feed"},
    {PLR_QUESTOR,		" questing"},
    {PLR_AUTOTAX,		" autotax"},
    {PLR_REASON,		" reason"},
    {PLR_HAGGLE,		" haggle"},
    {	0,			NULL}
};


char *act_bit_name( long long act_flags ) {
    static char buf[512];

    int i;

    buf[0] = '\0';

    i = 0;

    while (act_map[i].bits != 0) {

      if (act_map[i].bits & act_flags) {
        strcat(buf, act_map[i].label);
      }
 
      i += 1;
    }

    return (char*)( ( buf[0] != '\0' ) ? buf+1 : "none" );
}


/* Names for PLR flags... */
char *plr_bit_name( long long plr_flags ) {
    static char buf[512];
    int i;

    buf[0] = '\0';
    i = 0;

    while (plr_map[i].bits != 0) {

      if (plr_map[i].bits & plr_flags) {
        strcat(buf, plr_map[i].label);
      }
 
      i += 1;
    }

    return (char*)( ( buf[0] != '\0' ) ? buf+1 : "none" );
}


/* Names for COMM flags... */

static const struct comm_map_type {
    long  	 bits;
    char	*label;
} comm_map[] = {
    {COMM_QUIET,		" quiet"		},
    {COMM_DEAF,		" deaf"			},
    {COMM_NOWIZ,		" no_wiz"		},
    {COMM_NOAUCTION,	" no_auction"		},
    {COMM_NOGOSSIP,		" no_gossip"		},
    {COMM_NOQUESTION,	" no_question"		},
    {COMM_NOMUSIC,		" no_music"		},
    {COMM_COMPACT,		" compact"		},
    {COMM_FULLFIGHT,		" fullfight"		},
    {COMM_FULLCAST,		" fullcast"		},
    {COMM_FULLWEATHER,	" fullweather"		},
    {COMM_BRIEF,		" brief"		},
    {COMM_PROMPT,		" prompt"		},
    {COMM_COMBINE,		" combine"		},
    {COMM_TELNET_GA,		" telnet_goahead"		},
    {COMM_NOEMOTE,		" no_emote"		},
    {COMM_NOSHOUT,		" no_shout"		},
    {COMM_NOTELL,		" no_tell"		},
    {COMM_NOCHANNELS,	" no_channels"		},
    {COMM_IMMACCESS,	" immaccess"		},
    {COMM_NOHERO,		" no_herotalk"		},
    {COMM_NOPORT,		" no_teleport"		},
    {COMM_NOINV,		" no_invtalk"		},
    {	0,			NULL			}
};

char *comm_bit_name( long comm_flags ) {

    static char buf[512];

    int i;

    buf[0] = '\0';

    i = 0;

    while (comm_map[i].bits != 0) {

      if (comm_map[i].bits & comm_flags) {
        strcat(buf, comm_map[i].label);
      }
 
      i += 1;
    }

    return (char*)( ( buf[0] != '\0' ) ? buf+1 : "none" );
}

char *imm_bit_name(int imm_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (imm_flags & IMM_SUMMON		) strcat(buf, " summon");
    if (imm_flags & IMM_CHARM		) strcat(buf, " charm");
    if (imm_flags & IMM_MASK		) strcat(buf, " mask");
    if (imm_flags & IMM_MAGIC		) strcat(buf, " magic");
    if (imm_flags & IMM_WEAPON		) strcat(buf, " weapon");
    if (imm_flags & IMM_BASH		) strcat(buf, " blunt");
    if (imm_flags & IMM_PIERCE		) strcat(buf, " piercing");
    if (imm_flags & IMM_SLASH		) strcat(buf, " slashing");
    if (imm_flags & IMM_FIRE		) strcat(buf, " fire");
    if (imm_flags & IMM_COLD		) strcat(buf, " cold");
    if (imm_flags & IMM_LIGHTNING	) strcat(buf, " lightning");
    if (imm_flags & IMM_ACID		) strcat(buf, " acid");
    if (imm_flags & IMM_POISON		) strcat(buf, " poison");
    if (imm_flags & IMM_NEGATIVE	) strcat(buf, " negative");
    if (imm_flags & IMM_HOLY		) strcat(buf, " holy");
    if (imm_flags & IMM_ENERGY		) strcat(buf, " energy");
    if (imm_flags & IMM_MENTAL		) strcat(buf, " mental");
    if (imm_flags & IMM_DISEASE	) strcat(buf, " disease");
    if (imm_flags & IMM_DROWNING	) strcat(buf, " drowning");
    if (imm_flags & IMM_LIGHT		) strcat(buf, " light");
    if (imm_flags & VULN_IRON		) strcat(buf, " iron");
    if (imm_flags & VULN_WOOD		) strcat(buf, " wood");
    if (imm_flags & VULN_SILVER	) strcat(buf, " silver");
    if (imm_flags & VULN_STEEL	) strcat(buf, " steel");
    if (imm_flags & VULN_ADAMANTITE	) strcat(buf, " adamantite");
    if (imm_flags & VULN_MITHRIL	) strcat(buf, " steel");
    if (imm_flags & VULN_ALUMINIUM) strcat(buf, " aluminium");
    if (imm_flags & VULN_COPPER) strcat(buf, " copper");
	

    return (char*)( ( buf[0] != '\0' ) ? buf+1 : "none" );
}

char *wear_bit_name(int wear_flags)
{
    static char buf[512];

    buf [0] = '\0';
    if (wear_flags & ITEM_TAKE		) strcat(buf, " take");
    if (wear_flags & ITEM_WEAR_FINGER	) strcat(buf, " finger");
    if (wear_flags & ITEM_WEAR_NECK	) strcat(buf, " neck");
    if (wear_flags & ITEM_WEAR_BODY	) strcat(buf, " torso");
    if (wear_flags & ITEM_WEAR_HEAD	) strcat(buf, " head");
    if (wear_flags & ITEM_WEAR_LEGS	) strcat(buf, " legs");
    if (wear_flags & ITEM_WEAR_FEET	) strcat(buf, " feet");
    if (wear_flags & ITEM_WEAR_HANDS	) strcat(buf, " hands");
    if (wear_flags & ITEM_WEAR_ARMS	) strcat(buf, " arms");
    if (wear_flags & ITEM_WEAR_SHIELD	) strcat(buf, " shield");
    if (wear_flags & ITEM_WEAR_ABOUT	) strcat(buf, " body");
    if (wear_flags & ITEM_WEAR_WAIST	) strcat(buf, " waist");
    if (wear_flags & ITEM_WEAR_WRIST	) strcat(buf, " wrist");
    if (wear_flags & ITEM_WIELD		) strcat(buf, " wield");
    if (wear_flags & ITEM_HOLD		) strcat(buf, " hold");
    if (wear_flags & ITEM_WEAR_PRIDE	) strcat(buf, " pride");
    if (wear_flags & ITEM_WEAR_FACE	) strcat(buf, " face");
    if (wear_flags & ITEM_WEAR_EARS	) strcat(buf, " ears");
    if (wear_flags & ITEM_WEAR_FLOAT	) strcat(buf, " float");
    if (wear_flags & ITEM_WEAR_EYES     ) strcat(buf, " eyes");
    if (wear_flags & ITEM_WEAR_BACK     ) strcat(buf, " back");
    if (wear_flags & ITEM_WEAR_TATTOO     ) strcat(buf, " tattoo");

    return (char*)( ( buf[0] != '\0' ) ? buf+1 : "none" );
}

char *form_bit_name(int form_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (form_flags & FORM_POISON		) strcat(buf, " poison");
    else if (form_flags & FORM_EDIBLE	) strcat(buf, " edible");
    if (form_flags & FORM_MAGICAL	) strcat(buf, " magical");
    if (form_flags & FORM_INSTANT_DECAY	) strcat(buf, " instant_rot");
    if (form_flags & FORM_OTHER		) strcat(buf, " other");
    if (form_flags & FORM_BLEEDING	) strcat(buf, " bleeding");
    if (form_flags & FORM_ANIMAL	) strcat(buf, " animal");
    if (form_flags & FORM_SENTIENT	) strcat(buf, " sentient");
    if (form_flags & FORM_UNDEAD	) strcat(buf, " undead");
    if (form_flags & FORM_CONSTRUCT	) strcat(buf, " construct");
    if (form_flags & FORM_MIST		) strcat(buf, " mist");
    if (form_flags & FORM_INTANGIBLE	) strcat(buf, " intangible");
    if (form_flags & FORM_BIPED		) strcat(buf, " biped");
    if (form_flags & FORM_CENTAUR	) strcat(buf, " centaur");
    if (form_flags & FORM_INSECT		) strcat(buf, " insect");
    if (form_flags & FORM_SPIDER		) strcat(buf, " spider");
    if (form_flags & FORM_CRUSTACEAN	) strcat(buf, " crustacean");
    if (form_flags & FORM_WORM		) strcat(buf, " worm");
    if (form_flags & FORM_BLOB		) strcat(buf, " blob");
    if (form_flags & FORM_MAMMAL	) strcat(buf, " mammal");
    if (form_flags & FORM_BIRD		) strcat(buf, " bird");
    if (form_flags & FORM_REPTILE		) strcat(buf, " reptile");
    if (form_flags & FORM_SNAKE		) strcat(buf, " snake");
    if (form_flags & FORM_DRAGON	) strcat(buf, " dragon");
    if (form_flags & FORM_AMPHIBIAN	) strcat(buf, " amphibian");
    if (form_flags & FORM_FISH		) strcat(buf, " fish");
    if (form_flags & FORM_COLD_BLOOD 	) strcat(buf, " cold_blooded");
    if (form_flags & FORM_MACHINE 	) strcat(buf, " machine");
    if (form_flags & FORM_PLANT 	                ) strcat(buf, " plant");
    if (form_flags & FORM_NOWEAPON	) strcat(buf, " noweapon");

    return (char*)( ( buf[0] != '\0' ) ? buf+1 : "none" );
}

char *part_bit_name(int part_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (part_flags & PART_HEAD		) strcat(buf, " head");
    if (part_flags & PART_ARMS		) strcat(buf, " arms");
    if (part_flags & PART_LEGS		) strcat(buf, " legs");
    if (part_flags & PART_HEART		) strcat(buf, " heart");
    if (part_flags & PART_BRAINS	) strcat(buf, " brains");
    if (part_flags & PART_GUTS		) strcat(buf, " guts");
    if (part_flags & PART_HANDS		) strcat(buf, " hands");
    if (part_flags & PART_FEET		) strcat(buf, " feet");
    if (part_flags & PART_FINGERS	) strcat(buf, " fingers");
    if (part_flags & PART_EAR		) strcat(buf, " ears");
    if (part_flags & PART_EYE		) strcat(buf, " eyes");
    if (part_flags & PART_LONG_TONGUE	) strcat(buf, " long_tongue");
    if (part_flags & PART_EYESTALKS	) strcat(buf, " eyestalks");
    if (part_flags & PART_TENTACLES	) strcat(buf, " tentacles");
    if (part_flags & PART_FINS		) strcat(buf, " fins");
    if (part_flags & PART_WINGS		) strcat(buf, " wings");
    if (part_flags & PART_TAIL		) strcat(buf, " tail");
    if (part_flags & PART_CLAWS		) strcat(buf, " claws");
    if (part_flags & PART_FANGS		) strcat(buf, " fangs");
    if (part_flags & PART_HORNS		) strcat(buf, " horns");
    if (part_flags & PART_SCALES	) strcat(buf, " scales");
    if (part_flags & PART_HOOFS		) strcat(buf, " hooves");
    if (part_flags & PART_LEAVES		) strcat(buf, " leaves");

    return (char*)( ( buf[0] != '\0' ) ? buf+1 : "none" );
}

char *weapon_bit_name(int weapon_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (weapon_flags & WEAPON_FLAMING	 ) strcat(buf, " {rflaming{x");
    if (weapon_flags & WEAPON_ACID	 ) strcat(buf, " {Gacidic{x");
    if (weapon_flags & WEAPON_LIGHTNING	 ) strcat(buf, " {Yelectric{x");
    if (weapon_flags & WEAPON_FROST	 ) strcat(buf, " {Bfrost{x");
    if (weapon_flags & WEAPON_VAMPIRIC	 ) strcat(buf, " {Mvampiric{x");
    if (weapon_flags & WEAPON_SHARP	 ) strcat(buf, " {Wsharp{x");
    if (weapon_flags & WEAPON_VORPAL 	 ) strcat(buf, " {mvorpal{x");
    if (weapon_flags & WEAPON_PLAGUE  ) strcat(buf, " {gplague{x");
    if (weapon_flags & WEAPON_POISON     ) strcat(buf, " {gpoisoned{x");
    if (weapon_flags & WEAPON_ONE_SHOT   ) strcat(buf, " one-shot");
    if (weapon_flags & WEAPON_TWO_SHOT   ) strcat(buf, " two-shot");
    if (weapon_flags & WEAPON_SIX_SHOT   ) strcat(buf, " six-shot");
    if (weapon_flags & WEAPON_TWELVE_SHOT) strcat(buf, " twelve-shot");
    if (weapon_flags & WEAPON_36_SHOT    ) strcat(buf, " 36-shot");
    if (weapon_flags & WEAPON_108_SHOT   ) strcat(buf, " 108-shot");
    if (weapon_flags & WEAPON_STUN   ) strcat(buf, " stun");

    return (char*)( ( buf[0] != '\0' ) ? buf+1 : "none" );
}

char *off_bit_name(int off_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (off_flags & OFF_AREA_ATTACK) 	strcat(buf, " area attack");
    if (off_flags & OFF_BACKSTAB) 	strcat(buf, " backstab");
    if (off_flags & OFF_BASH) 		strcat(buf, " bash");
    if (off_flags & OFF_BERSERK) 		strcat(buf, " berserk");
    if (off_flags & OFF_DISARM) 		strcat(buf, " disarm");
    if (off_flags & OFF_DODGE) 		strcat(buf, " dodge");
    if (off_flags & OFF_FADE) 		strcat(buf, " fade");
    if (off_flags & OFF_FAST) 		strcat(buf, " fast");
    if (off_flags & OFF_KICK) 		strcat(buf, " kick");
    if (off_flags & OFF_KICK_DIRT) 	strcat(buf, " kick_dirt");
    if (off_flags & OFF_PARRY) 		strcat(buf, " parry");
    if (off_flags & OFF_RESCUE) 		strcat(buf, " rescue");
    if (off_flags & OFF_TAIL) 		strcat(buf, " tail");
    if (off_flags & OFF_TRIP) 		strcat(buf, " trip");
    if (off_flags & OFF_CRUSH) 		strcat(buf, " crush");
    if (off_flags & ASSIST_ALL) 		strcat(buf, " assist_all");
    if (off_flags & ASSIST_ALIGN) 		strcat(buf, " assist_align");
    if (off_flags & ASSIST_RACE) 		strcat(buf, " assist_race");
    if (off_flags & ASSIST_PLAYERS) 	strcat(buf, " assist_players");
    if (off_flags & ASSIST_GUARD) 		strcat(buf, " assist_guard");
    if (off_flags & ASSIST_VNUM) 		strcat(buf, " assist_vnum");
    if (off_flags & OFF_STUN) 		strcat(buf, " stun");
    if (off_flags & OFF_DISTRACT) 		strcat(buf, " distract");
    if (off_flags & OFF_MERCY) 		strcat(buf, " mercy");

    return (char*)( ( buf[0] != '\0' ) ? buf+1 : "none" );
}

/* Text for current activity/position... */

void pos_text(CHAR_DATA *ch, char *buf) {
    char buf2[MAX_STRING_LENGTH];

    if (ch->acv_desc != NULL) {

      strcat(buf, ch->acv_desc);

    } else {
      if (ch->pos_desc != NULL) {

        strcat(buf, ch->pos_desc);

      } else {
        switch ( ch->position ) {

          case POS_DEAD:     
            strcat( buf, "DEAD!!" );
            break;

          case POS_MORTAL:   
            strcat( buf, "{Rmortally wounded.{w" );   
            break;

          case POS_INCAP:    
            strcat( buf, "{Rincapacitated.{w" );      
            break;

          case POS_STUNNED:  
             if ( IS_SET(ch->act,ACT_BOUND)
             && ch->hit>0
             && ch->move>0) {
                   strcat( buf, "lying here gagged and bound." ); 
             } else {
                   strcat( buf, "lying here stunned." ); 
             }
            break;

          case POS_SLEEPING: 
            strcat( buf, "sleeping here." );      
            break;

          case POS_RESTING:  
            strcat( buf, "resting here." );       
            break;

          case POS_SITTING:  
            strcat( buf, "sitting here." );	 
            break;

          case POS_STANDING: 
            if (!IS_NPC(ch)) {     
                 if (ch->pet !=NULL
                 && ch->pcdata->mounted==TRUE) {
                       sprintf(buf2, "riding on a %s.", ch->pet->short_descr); 
                       strcat( buf, buf2 );               
                 } else {
                       strcat( buf, "standing here." );               
                 }
             } else {
                  strcat( buf, "standing here." );               
             }
            break;

          case POS_FIGHTING:
            strcat( buf, "fighting " );
            if ( ch->fighting == NULL ) {
              strcat( buf, "thin air??" );
            } else {
              if ( ch->fighting == ch ) {
                strcat( buf, "YOU!" );
              } else {
                if ( ch->in_room == ch->fighting->in_room ) {
                  strcat( buf, PERSMASK( ch->fighting, ch ) );
                  strcat( buf, "." );
                } else {
                  strcat(buf, "someone who left??" );
                } 
              }
            }
            break;

          default:
            strcat(buf, "confused!");
            break;  
        }
      } 
    }

    return;
}

char *pos_text_short(CHAR_DATA *ch) {

    char *pos_text;

    pos_text = "{YConfused{x";

    switch ( ch->position ) {

      case POS_DEAD:     
        pos_text = "{RDead{x";
        break;

      case POS_MORTAL:   
        pos_text = "{RDieing{x";   
        break;

      case POS_INCAP:    
        pos_text = "{RIncapacitated{x";      
        break;

      case POS_STUNNED:  
             if ( IS_SET(ch->act,ACT_BOUND)
             && ch->hit>0
             && ch->move>0) {
                    pos_text = "{RBound{x"; 
             } else {
                    pos_text = "{RStunned{x"; 
             }
        break;

      case POS_SLEEPING: 
        pos_text = "{cSleeping{x";      
        break;

      case POS_RESTING:  
        pos_text = "{cResting{x";       
        break;

      case POS_SITTING:  
        pos_text = "{cSitting{x";	 
        break;

      case POS_STANDING: 
        pos_text = "{cStanding{x";               
        break;

      case POS_FIGHTING:
        pos_text = "{yFighting{x";
        break;

      default:
        break;  
    }

    return pos_text;
}

char *acv_text_short(CHAR_DATA *ch) {
char *acv_text;

    acv_text = "{YConfused{x";

    switch ( ch->activity ) {

      case ACV_NONE:     
        acv_text = "{cNone{x";
        break;

      case ACV_SEARCHING:   
        acv_text = "{ySearching{x";   
        break;

      case ACV_DESTROYING:   
        acv_text = "{yDestroying{x";   
        break;

      case ACV_REBUILDING:   
        acv_text = "{yRebuilding{x";   
        break;

      case ACV_PICKING:    
        acv_text = "{yPicking{x";      
        break;

      case ACV_HIDING:  
        acv_text = "{mHiding{x"; 
        break;

      case ACV_SNEAKING: 
        acv_text = "{mSneaking{x";      
        break;

      case ACV_CASTING:  
        acv_text = "{yCasting{x";       
        break;

      case ACV_TRACKING:  
        acv_text = "{yTracking{x";       
        break;

      case ACV_DEBATING:  
        acv_text = "{yDebating{x";       
        break;

      case ACV_DUEL:  
        acv_text = "{yDuelling{x";       
        break;

      default:
        break;  
    }

    return acv_text;
}


/* Set current activity... */

void set_activity( CHAR_DATA *ch, int new_position, char *new_pos_desc, int new_activity, char *new_acv_desc) {				
AFFECT_DATA af;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

    int old_pos, old_acv;

    char buf[MAX_STRING_LENGTH];

    mcc = NULL;

   /* Message if the change is visible... */

    if ( ( ch->position != new_position
        && ch->position != POS_DEAD 
        && ch->position != POS_MORTAL 
        && ch->position != POS_INCAP 
        && ch->position != POS_STUNNED 
        && ch->position != POS_STANDING 
        && ch->position != POS_FIGHTING )
      || ( ch->activity != new_activity 
        && ch->activity != ACV_NONE )
      || ch->pos_desc != NULL ) {

      buf[0] = '\0';
      pos_text(ch, buf);

      mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, new_activity, buf);

      if (mcc != NULL) {
        if ( ch->activity == ACV_CASTING ) {

          if ( ch->acv_state > 0 
            && ch->acv_state < ACV_STATE_MAX ) {
            wev = get_wev(WEV_ACTIVITY, WEV_ACTIVITY_STOP, mcc,
                         "{YYour spell casting is disrupted!{x\r\n",
                          NULL,
                         "{y@a2s spell casting is disrupted!{x\r\n");
          } else {
            wev = get_wev(WEV_ACTIVITY, WEV_ACTIVITY_STOP, mcc,
                         "You finish casting your spell.\r\n",
                          NULL,
                         "@a2 finishes casting their spell.\r\n");
          }
        } else if (ch->activity == ACV_TRACKING) {
          wev = get_wev(WEV_ACTIVITY, WEV_ACTIVITY_STOP, mcc,
                       "You stop @t0\r\n",
                        NULL,
                        NULL);
        } else {
          wev = get_wev(WEV_ACTIVITY, WEV_ACTIVITY_STOP, mcc,
                       "You stop @t0\r\n",
                        NULL,
                       "@a2 stops @t0\r\n");
        }

        room_issue_wev( ch->in_room, wev);
        free_wev(wev);
      }
    } 

   /* Undo the effects of the old activity... */

    if (ch->activity != new_activity) {

      switch (ch->activity) {

        case ACV_SNEAKING:
          REMOVE_BIT(ch->affected_by, AFF_SNEAK);
          break;

        case ACV_HIDING:
          REMOVE_BIT(ch->affected_by, AFF_HIDE);
          break;

        case ACV_CASTING:
          if ( ch->acv_state > 0
            && ch->acv_state < ACV_STATE_MAX
            && number_open() < 2 * ch->acv_state) {
            send_to_char("{ROughh! That hurts...{x\r\n", ch);

            af.type		= SKILL_UNDEFINED;
            af.afn		= gafn_fatigue;
            af.level		= ch->level;
            af.duration		= 10 + dice(1,6);
            af.bitvector	= 0;

            af.location		= APPLY_INT;
            af.modifier		= -1;
            affect_to_char( ch, &af );

            af.location		= APPLY_WIS;
            af.modifier		= -1;
            affect_to_char( ch, &af );

            af.location		= APPLY_DEX;
            af.modifier		= -1;
            affect_to_char( ch, &af );

            af.location		= APPLY_MOVE;
            af.modifier		= -ch->level;
            affect_to_char( ch, &af );
          } 
          break;

        default:
          break;
      }

     /* Zap all of the old control values... */

      clear_activity(ch);
    }

   /* Store new position... */

    old_pos = ch->position;
    old_acv = ch->activity;

    if (ch->position != new_position) {
         mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, new_position, NULL);
         wev = get_wev(WEV_ACTIVITY, WEV_ACTIVITY_POSCHANGE, mcc, NULL, NULL, NULL);

         ch->position = new_position;

         room_issue_wev( ch->in_room, wev);
         free_wev(wev);
    }

    if (ch->pos_desc != NULL) {
      free_string(ch->pos_desc);
      ch->pos_desc = NULL;
    }

    if (new_pos_desc != NULL) {
      ch->pos_desc = str_dup(new_pos_desc);
    }

   /* Store new activity... */
 
   ch->activity = new_activity;

    if (ch->acv_desc != NULL) {
      free_string(ch->acv_desc);
      ch->acv_desc = NULL;
    }

    if (new_acv_desc != NULL) {
      ch->acv_desc = str_dup(new_acv_desc);
    }

   /* Get new activity text... */

    if ( ch->pos_desc != NULL
      || ch->acv_desc != NULL 
      || ( old_pos != ch->position
        && ch->position != POS_DEAD 
        && ch->position != POS_MORTAL 
        && ch->position != POS_INCAP 
        && ch->position != POS_STUNNED 
        && ch->position != POS_STANDING 
        && ch->position != POS_FIGHTING ) 
      || ( old_acv != ch->activity
        && ch->activity != ACV_NONE) ) {
      
      buf[0] = '\0';
      pos_text(ch, buf);

      mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, new_activity, buf);
      wev = get_wev(WEV_ACTIVITY, WEV_ACTIVITY_START, mcc,
                   "You start @t0\r\n",
                    NULL,
                   "@a2 starts @t0\r\n");

      room_issue_wev( ch->in_room, wev);
      free_wev(wev);
    }
    return;
}


void set_activity_key( CHAR_DATA *ch ) {

  ch->acv_key = number_bits(12);

  return;
}

void clear_activity( CHAR_DATA *ch ) {

  ch->acv_key = 0;
  ch->acv_state = 0;
  ch->acv_int = 0;
  ch->acv_int2 = 0;
  ch->acv_int3 = 0;

  if (ch->acv_text != NULL) {
    free_string(ch->acv_text);
    ch->acv_text = NULL;
  } 

  purge_mcbs(ch, ACV_CMD_ID);

  return;
}

bool check_activity_key( CHAR_DATA *ch, char *args) {

  int old_key;

 /* If it's not a key, then it either a new command or the end of an old one... */

  if (args[0] != '*') {
    return TRUE;
  }  

 /* If the key matches, we can continue... */

  old_key = atoi(args + 1);

  if (old_key == ch->acv_key) {
    return TRUE;
  }

 /* if it doesn't match, then someones trying to hack the interface... */

  set_activity(ch, ch->position, "looking guilty", ACV_NONE, NULL);

  clear_activity(ch);

  return FALSE; 
}

void schedule_activity( CHAR_DATA *ch, int delay, char *cmd) {

  char buf[MAX_STRING_LENGTH];

 /* Must have an activity key... */

  if (ch->acv_key == 0) {
    return;
  }

 /* Build up the complete command... */

  sprintf(buf, "%s *%d", cmd, ch->acv_key); 

 /* Schedule it... */

  enqueue_mob_cmd(ch, buf, delay, ACV_CMD_ID);

 /* All done... */

  return;
}

void check_spirit( CHAR_DATA *ch, CHAR_DATA *victim ) {
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *spirit;
    int modlev;

        /* only happens 1 in 25 times and only to NPCs */

    if ( number_range(0,14) != 0 || !IS_NPC(victim))
        return;

    modlev = UMIN (140, victim->level + 5);
    spirit = create_mobile_level(victim->pIndexData,"mob undead", modlev );

    SET_BIT ( spirit->form, FORM_INSTANT_DECAY|FORM_UNDEAD|FORM_INTANGIBLE|FORM_MIST );
    SET_BIT ( spirit->act, ACT_AGGRESSIVE );
    SET_BIT ( spirit->affected_by, AFF_PASS_DOOR );

    sprintf(buf,"the spirit of %s",victim->short_descr);
    spirit->short_descr = str_dup(buf);

    sprintf(buf,"spirit %s",victim->name);
    spirit->name = str_dup(buf);

    char_to_room( spirit, ch->in_room );

    send_to_char("{REverything has become so nightmarish...{x\r\n", ch);
    act("You cower in fear as $N appears before you!",ch,NULL,spirit,TO_CHAR);
    act("$N suddenly appears and attacks $n!",ch,NULL,spirit,TO_ROOM);

    multi_hit( spirit, ch, TYPE_UNDEFINED );

    return;
}

void check_undead( CHAR_DATA *ch, CHAR_DATA *victim) {
    CHAR_DATA *spirit;
    int modlev;

    modlev = UMIN (140, victim->level + 10);
    spirit = create_mobile_level(victim->pIndexData, "mob undead", modlev );

    SET_BIT ( spirit->form, FORM_INSTANT_DECAY|FORM_UNDEAD|FORM_INTANGIBLE );
    SET_BIT ( spirit->act, ACT_AGGRESSIVE );

    char_to_room( spirit, ch->in_room );

    send_to_char("{REverything has become so nightmarish...{x\r\n", ch);
    act("With a wicked grin $N stands up again!",ch,NULL,spirit,TO_CHAR);
    act("With a wicked grin $N stands up again!",ch,NULL,spirit,TO_ROOM);

    multi_hit( spirit, ch, TYPE_UNDEFINED );

    return;
}



void harmful_effect(void *vo, int level, int target, int effect) {

    if (target == TAR_OBJ_ROOM)  {
	ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	for (obj = room->contents; obj != NULL; obj = obj_next) {
	    obj_next = obj->next_content;
                    harmful_effect(obj, level, TAR_OBJ_INV, effect);
	}

    } else if (target == TAR_CHAR_OFFENSIVE) {
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj, *obj_next;
	
	for (obj = victim->carrying; obj != NULL; obj = obj_next) {
	    obj_next = obj->next_content;
                    harmful_effect(obj, level, TAR_OBJ_INV, effect);
	}

    } else if (target == TAR_OBJ_INV) {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	OBJ_DATA *t_obj,*n_obj;
	int chance;
	char *msg;

              	if (IS_OBJ_STAT(obj, ITEM_NO_COND)) return;
	chance = level /3 + number_range(1, level/3);

                switch (effect) {
                   default:
  	       return;

                   case DAM_FIRE:
  	       if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF) ||  number_range(0,4) == 0) return;
                       switch ( obj->item_type ) {
                          default:             
    	             return;
                          case ITEM_CONTAINER:
                             msg = "$p ignites and burns!";
                             break;
                          case ITEM_POTION:
                             chance += 25;
                             msg = "$p bubbles and boils!";
                             break;
                          case ITEM_SCROLL:
                             chance += 50;
                             msg = "$p crackles and burns!";
                             break;
                          case ITEM_STAFF:
                             chance += 10;
                             msg = "$p smokes and chars!";
                             break;
                          case ITEM_WAND:
                             msg = "$p sparks and sputters!";
                             break;
                          case ITEM_FOOD:
                             msg = "$p blackens and crisps!";
                             break;
                          case ITEM_PILL:
                              msg = "$p melts and drips!";
                              break;
                       }
                       switch ( obj->material) {
                          default:
                              break;

                          case MATERIAL_WOOD:
                          case MATERIAL_CLOTH:
                          case MATERIAL_VELLUM:
                          case MATERIAL_SILK:
                              chance +=10;
                              break;

                          case MATERIAL_PAPER:
                              chance +=25;
                              break;

                          case MATERIAL_LIQUID:
                          case MATERIAL_FIRE:
                          case MATERIAL_WATER:
                              chance -=25;
                              break;
                       }
                       break;

                   case DAM_COLD:
  	       if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF) ||  number_range(0,4) == 0) return;
                       switch(obj->item_type) {
	           default:
		return;
	           case ITEM_POTION:
		msg = "$p freezes!";
		chance += 25;
		break;
	           case ITEM_DRINK_CON:
		msg = "$p freezes!";
		chance += 5;
		break;
	       } 
                       switch ( obj->material) {
                          default:
                              break;

                          case MATERIAL_LIQUID:
                          case MATERIAL_FIRE:
                          case MATERIAL_WATER:
                              chance +=25;
                              break;
                       }
                       break;

                   case DAM_LIGHTNING:
  	       if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF) ||  number_range(0,4) == 0) return;
         	       switch(obj->item_type) {
	           default:
		return;
	           case ITEM_WAND:
	           case ITEM_STAFF:
		chance += 10;
		msg = "$p overloads!";
		break;
          	            case ITEM_JEWELRY:
		chance -= 10;
		msg = "$p is fused.";
	       }
                       switch ( obj->material) {
                          default:
                              break;

                          case MATERIAL_GOLD:
                          case MATERIAL_SILVER:
                              chance +=10;
                              break;

                          case MATERIAL_IRON:
                          case MATERIAL_STEEL:
                              chance +=25;
                              break;

                          case MATERIAL_RUBBER:
                          case MATERIAL_PLASTIC:
                          case MATERIAL_LEATHER:
                              chance -=25;
                              break;
                       }
                       break;

                   case DAM_ACID:
  	       if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF) ||  number_range(0,4) == 0) return;
       	       switch (obj->item_type) {
	           default:
		return;
        	           case ITEM_CONTAINER:
	           case ITEM_CORPSE_PC:
	           case ITEM_CORPSE_NPC:
	           case ITEM_KEY_RING:
		msg = "$p fumes.";
		break;
	           case ITEM_ARMOR:
		msg = "$p is pitted and etched.";
		break;
	            case ITEM_CLOTHING:
	            case ITEM_WEAPON:
		msg = "$p corrodes.";
	 	break;
	            case ITEM_STAFF:
	            case ITEM_WAND:
		chance -= 10;
		msg = "$p corrodes.";
		break;
	       }
                       switch ( obj->material) {
                          default:
                              break;

                          case MATERIAL_STEEL:
                          case MATERIAL_GOLD:
                          case MATERIAL_SILVER:
                          case MATERIAL_BRASS:
                              chance +=10;
                              break;

                          case MATERIAL_IRON:
                          case MATERIAL_ALUMINIUM:
                          case MATERIAL_BRONZE:
                              chance +=25;
                              break;

                          case MATERIAL_SCALES:
                          case MATERIAL_GLASS:
                          case MATERIAL_LEATHER:
                              chance -=25;
                              break;
                       }
                       break;
                }

                if (chance > 50) chance = (chance - 50) / 2 + 50;

	if (IS_OBJ_STAT(obj, ITEM_MAGIC)) chance -= 5;

	chance -= obj->level * 2;
	chance = URANGE(5, chance, 95);
	if (number_percent() > chance)  return;

	if (obj->carried_by != NULL) act(msg, obj->carried_by,obj, NULL, TO_CHAR);

                if (chance > 90) obj->condition -= number_range(26, 50);
                else obj->condition -= number_range(1, 25);

                if (obj->condition < 0) {
  	     if (obj->contains) {
  	         for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)  {
		n_obj = t_obj->next_content;
		obj_from_obj(t_obj);
		if (obj->in_room != NULL)  obj_to_room(t_obj,obj->in_room);
		else if (obj->carried_by != NULL)  obj_to_room(t_obj,obj->carried_by->in_room);
		else {
		    extract_obj(t_obj);
		    continue;
		}

                                harmful_effect(t_obj, level, TAR_OBJ_INV, effect);
	         }
 	     }
       	     extract_obj(obj);
                }
    }
    return;
}


CHAR_DATA *get_char_area( CHAR_DATA *ch, char *argument, bool subarea ) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number,count;

    if (!ch) return NULL;
    if (!str_cmp ("self", argument)) return ch;
    if ((rch = get_char_room( ch, argument )) != NULL ) return rch;

    if (argument[0] == '@') {
      return get_char_area_true(ch, argument, subarea);
    } 

    number = number_argument( argument, arg );
    count = 0;
    for (rch = char_list; rch; rch = rch->next )  {
	if (rch->in_room->area != ch->in_room->area
	||  !can_see(ch, rch) || !is_name(arg, rch->name))  continue;
                if (subarea && ch->in_room->subarea != rch->in_room->subarea) continue;
	if (++count == number)  return rch;
    }

    return rch;
}


CHAR_DATA *get_char_area_true(CHAR_DATA *ch, char *argument, bool subarea) {
  CHAR_DATA *rch;

  if (argument == NULL  || argument[0] != '@'  || argument[1] == '\0' ) {
    return NULL;
  }  

 /* Search... */
  for(rch = char_list; rch; rch = rch->next) {
      if (ch->in_room->area != rch->in_room->area) continue;
      if (subarea && ch->in_room->subarea != rch->in_room->subarea) continue;

      if (rch->true_name != NULL) {
          if ( rch->true_name[1] == argument[1] ) {
                 if (!str_cmp(rch->true_name, argument)) return rch;
          }
      }
  }

  return NULL;
}


void check_vanish(CHAR_DATA *ch, OBJ_DATA *olist) {
OBJ_DATA *obj, *obj_next;

      for (obj = olist; obj; obj = obj_next) {
            obj_next = obj->next_content;

            if (obj->item_type == ITEM_CONTAINER
            || obj->item_type == ITEM_KEY_RING) {
                  check_vanish(ch, obj->contains);
            } else {
                  if (obj->pIndexData) {
                       if (IS_SET(obj->extra_flags, ITEM_VANISH)
                       && obj->pIndexData->area != ch->in_room->area) {
                            if (obj->carried_by) sprintf_to_char(ch, "{m%s slowly fades away.{x\r\n", capitalize(obj->short_descr));
                            extract_obj(obj);
                       }
                  }
            }
      }
      return;
}


int get_char_size(CHAR_DATA *ch) {
OBJ_DATA *obj;
AFFECT_DATA *paf;
int size = ch->size;

     if (IS_AFFECTED(ch, AFF_POLY)
     && ch->race_orig != 0) {
            size = race_array[ch->race].size;
     }

     for (obj = ch->carrying; obj; obj = obj->next_content) {
           if (obj->wear_loc == WEAR_NONE) continue;
           if (obj->item_type == ITEM_RAW || obj->item_type == ITEM_GEM || obj->item_type == ITEM_PASSPORT) continue;

           if (obj->enchanted) {
               for (paf = obj->affected; paf; paf = paf->next ) {
                     if (paf->location == APPLY_HEIGHT) size += paf->modifier;
               }
           } else {
               for (paf = obj->pIndexData->affected; paf; paf = paf->next ) {
                     if (paf->location == APPLY_HEIGHT) size += paf->modifier;
               }
           }
     }

     if (is_affected(ch, gafn_size)) {
           for (paf = ch->affected; paf; paf = paf->next) {
                   if (paf->afn == gafn_size) size += paf->modifier;
           }
     }

     size = URANGE(0, size, 5);
     return size;
}


int get_cust_rating(SHOP_DATA *shop, char *name) {
CUSTOMER *cust;

     for (cust = shop->customer_list; cust; cust = cust->next) {
            if (!str_cmp(cust->name, name)) return cust->rating;
     }
     return 0;
}


CUSTOMER *get_customer(SHOP_DATA * shop, char *name) {
CUSTOMER *cust;

     for (cust = shop->customer_list; cust; cust = cust->next) {
            if (!str_cmp(cust->name, name)) return cust;
     }
     return NULL;
}


void improve_rating(SHOP_DATA *shop, char *name) {
CUSTOMER *cust;

          cust = get_customer(shop, name);
          if (!cust) {
              cust =new_customer();
              cust->next = shop->customer_list;
              shop->customer_list = cust;
              cust->name = str_dup(name);
              cust->rating = 0;
          }
          cust->rating++;

          return;
}


void obj_fall(OBJ_DATA *obj, int recursion) {
ROOM_INDEX_DATA *to_room;
EXIT_DATA *pexit;

        if (obj->in_room->sector_type == SECT_AIR) {
             if ((pexit = obj->in_room->exit[DIR_DOWN]) != NULL) {
                if ((to_room = get_exit_destination(NULL, obj->in_room, pexit, FALSE)) != NULL) {
                       sprintf_to_room(obj->in_room, "{m%s falls down.{x\r\n", capitalize(obj->short_descr));
                       sprintf_to_room(to_room, "{m%s falls from the sky.{x\r\n", capitalize(obj->short_descr));
                       obj_from_room(obj);
                       obj_to_room(obj, to_room);
                       if (recursion < 10) obj_fall(obj, recursion+1);
                }                       
             }
        }
        return;
}


int get_heal_rate(ROOM_INDEX_DATA *room) {
OBJ_DATA *obj;
int heal;

     if (!room) return 100;

     heal = room->heal_rate;

     for (obj = room->contents; obj; obj = obj->next_content) {
           if (obj->item_type == ITEM_DECORATION) {
               heal += obj->value[0];
           }
     }

     heal = UMAX(0, heal);
     return heal;
}


int get_mana_rate(ROOM_INDEX_DATA *room) {
OBJ_DATA *obj;
int mana;

     if (!room) return 100;

     mana = room->mana_rate;

     for (obj = room->contents; obj; obj = obj->next_content) {
           if (obj->item_type == ITEM_DECORATION) {
               mana += obj->value[1];
           }
     }

     mana = UMAX(0, mana);
     return mana;
}


OBJ_DATA *find_passport(CHAR_DATA *ch) {
OBJ_DATA *pass;

     for(pass = object_list; pass; pass = pass->next) {
         if (pass->item_type == ITEM_PASSPORT && pass->owner) {
             if (!str_cmp(pass->owner, ch->name)) break;
         }
     }

     return pass;
}


int get_criminal_rating(CHAR_DATA *ch) {
OBJ_DATA *pass;
AFFECT_DATA *af;
int rating;

     if ((pass = find_passport(ch)) == NULL) return 0;
     if (!pass->affected) return 0;

     switch(pass->value[0]) {
        default:
           rating = 0;
           break;
        case 1:
           rating = -10;
           break;
        case 2:
           rating = -30;
           break;
     }

     for(af = pass->affected; af; af = af->next) {
         if (af->location == APPLY_CRIME_RECORD) rating +=af->modifier;
     }

     rating = UMAX(rating, 0);
     return rating;
}


void add_passport_entry(CHAR_DATA *ch, int crime) {
OBJ_DATA *pass;
AFFECT_DATA *af;
int penalty;

     if ((pass = find_passport(ch)) == NULL) return;

     switch(crime) {
        default:
           penalty = 1;
           break;
        case 2:
           penalty = 3;
           break;
        case 3:
           penalty = 10;
           break;
     }

     af             	= new_affect();
     af->location   	= APPLY_CRIME_RECORD;
     af->modifier   	= penalty;
     af->type       	= crime;
     af->duration   	= -1;
     af->next       	= pass->affected;
     pass->affected = af;
     return;
}


void clear_passport_entry(CHAR_DATA *ch, int crime) {
OBJ_DATA *pass;
AFFECT_DATA *af;
AFFECT_DATA *af_prev = NULL;

     if ((pass = find_passport(ch)) == NULL) return;
     if (!pass->affected) return;

     if (crime == -1) {
        for (af_prev = pass->affected; af_prev; af_prev = af) {
            af = af_prev->next;

            free_affect(af_prev);
        } 
        pass->affected = NULL;
        return;
     } else if (crime == 0) {
        af = pass->affected;
     } else {
        for (af = pass->affected; af; af = af->next) {
           if (af->location == crime) break;
           af_prev = af;
        }
     }
     if (!af) return;

     if(!af_prev) {
	pass->affected = af->next;
	free_affect(af);
    } else {
	af_prev->next = af->next;
	free_affect(af);
    }

    return;
}


int get_professional_level(CHAR_DATA *ch, int level) {
OBJ_DATA *obj;
AFFECT_DATA *paf;

     for (obj = ch->carrying; obj; obj = obj->next_content) {
           if (obj->wear_loc == WEAR_NONE) continue;
           if (obj->item_type == ITEM_RAW || obj->item_type == ITEM_GEM || obj->item_type == ITEM_PASSPORT) continue;

           if (obj->enchanted) {
               for (paf = obj->affected; paf; paf = paf->next ) {
                     if (paf->location == APPLY_LEVEL) level += paf->modifier;
               }
           } else {
               for (paf = obj->pIndexData->affected; paf; paf = paf->next ) {
                     if (paf->location == APPLY_LEVEL) level += paf->modifier;
               }
           }
     }

     level = UMAX(0, level);
     return level;
}


int get_sanity(CHAR_DATA *ch) {
AFFECT_DATA *af;
int sanity;

     if (is_affected(ch, gafn_antipsychotica)) {
         for (af = ch->affected; af; af = af->next) {
               if (af->afn == gafn_antipsychotica && af->location == APPLY_NONE) return af->modifier;
         }
     }

     sanity = UMAX(ch->sanity, 1);
     return sanity;
}

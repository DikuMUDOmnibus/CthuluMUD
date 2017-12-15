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
#include "affect.h"
#include "skill.h"
#include "race.h"
#include "olc.h"

bool remove_obj (CHAR_DATA *ch, int iWear, bool fReplace );

AFFECT_DATA *affect_free;


const AFFECT_INFO affect_array[MAX_AFFECT]	= {

  { "stats",
	"You feel different.",
	"You feel different.",
	 0						},

  { "armor", 		
	"You feel protected.",   
	"You feel less protected.",
	 0						},

  { "bless",
	"You feel righteous.",
	"You feel less righteous.",
	 0						},

  { "blindness",
	"A darkness falls across your vision!",
	"You can see again.",
	AFF_BLIND					},

  { "charm",
	"You feel remote and distanced.",
	"You feel more in control of yourself.",
	 AFF_CHARM					}, 

  { "chill touch",
	"An icy chill runs through your bones!",
	"You feel less cold.",
	 0						},

  { "curse",
	"You feel a darkness within you!",
	"The curse wears off.",
	AFF_CURSE					},

  { "detect evil",
	"Your eyes itch.",
	"Your eyes stop itching.",
	AFF_DETECT_EVIL					},

  { "detect invis",	
	"Your eyes itch.",
	"Your eyes stop itching.",
	AFF_DETECT_INVIS				},

  { "detect magic",
	"Your eyes itch.",
	"Your eyes stop itching.",
	AFF_DETECT_MAGIC				},

  { "invisability",
	"You fade from the mortal world.", 
	"You are no longer invisible.",
	AFF_INVISIBLE					},

  { "poison",
	"You feel deadly ill.",
	"You feel better.",
	AFF_POISON					},

  { "protection evil",
	"You feel a warm glow withiin you.",
	"You feel cold.",
	AFF_PROTECTE					},

  { "sanctuary",
	"You glow with a white aura!",
	"The white aura around your body fades.",
	AFF_SANCTUARY					},

  { "sleep",
	"You feel sooo tired...Zzzzz....Zzzzzz....Zzzzz....", 
	"You awaken feeling refreshed.",
	AFF_SLEEP					},

  { "giant strength",	
	"You feel so much stonger!",
	"You feel weaker.",
	0						},

  { "detect hidden",
	"The world comes into focus.",
	"The world goes out of focus.",
	AFF_DETECT_HIDDEN				},

  { "fly",
	"Wow, you're flying!",
	"You slowly float to the ground.",
	AFF_FLYING					},

  { "mist",
	"You dissolve into thin air!",
	"You slowly take back your form.",
	AFF_MIST					},

  { "elder shield",
	"You feel almost invulnerable!",
	"You feel vulnerable again.",
	AFF_ELDER_SHIELD					},

  { "farsight",
	"Your sight is much clearer now!",
	"Your sight worsens.",
	AFF_FARSIGHT					},

  { "hard skin",
	"Your skin becomes hard!",
	"Your skin feels soft again.",
	0						},

  { "shield",
	"A shield of shimmering energy surrounds you!",
	"Your force shield shimmers then fades away.",
	0						},

  { "mana shield",
	"A shield of mana surrounds you!",
	"Your mana shield shimmers then fades away.",
	0						},

  { "weakness",
	"You feel very weak!",
	"Your strength returns.",
	AFF_WEAKEN					},

  { "faerie fire",
	"You glow with a pink aura!", 
	"The pink glow around your body fades.",
	AFF_FAERIE_FIRE					},

  { "pass door",
	"You feel somehow intangible!",
	"You feel solid again.",
	AFF_PASS_DOOR					},

  { "infravision",
	"The world looks different, all reds and yellows!",
	"The world looks normal again.",
	AFF_INFRARED					},

  { "sex change",
	"Something seems fundamentally different.",
	"Your body feels familiar again.",
	0						},

  { "mind meld",
	"You feel an alien presence in your head!",
	"Your feel your head clearing.",
	AFF_MELD					},

  { "haste",
	"Oh, wow, man, like everything looks so slow.",
	"The world speeds up around you.",
	AFF_HASTE					}, 

  { "plague",
	"Evil sores break out all over your body!",
	"Your sores vanish.",
	AFF_PLAGUE					}, 

  { "frenzy",
	"Rage consumes your mind!",
	"Your rage ebbs.",
	0						},

  { "calm",
	"You feel a great sense of peace and tranquility.",
	"You have lost your peace of mind.",
	AFF_CALM			 		},

  { "mask self",
	"Your body shifts and changes!",
	"You return to normal form.",
	AFF_POLY					},

  { "mask hide",
	"You mask!",
	"You remove the mask.",
	0					},

  { "mask force",
	"You change your form!",
	"You finally come back to your original form.",
	0					},

  { "absorb magic",
	"You glow with a purple aura.",
	"The purple glow around your body fades.",
	AFF_ABSORB					}, 

  { "fear",
	"A great angst consumes you!",
	"You stop feeling so scared.",
	AFF_FEAR					},

  { "protection good",	
	"You feel an icy chill within you!",
	"You feel warm again.",
	AFF_PROTECTG					},

  { "detect good",
	"Your eyes itch.",
	"Your eyes stop itching.",
	0						},

  { "regeneration",
	"Your body feels unnaturally alive!",
	"Your body feels normal again.",
	AFF_REGENERATION				}, 

  { "fire shield",
	"You are surrounded by a wall of flames!",
	"The flames around you fade away.",
	0						},

  { "vocalize",
	"You throat starts to itch.",
	"Your throat stops itching.",
	0						},

  { "mute",
	"You mouth disappears as your lips weld themselves together!",
	"You face splits open and your mouth reappears.",
	0						},

  { "dirt kicking",	
	"The dirt in your eyes blindes you!",
	"You rub the dirt out of your eyes.",
	0						}, 

  { "berserk",		
	"You really, really wanna kill someone!",
	"You feel your pulse subside.",
	AFF_BERSERK					},

  { "sneak",
	"You enter a state of peace.",
	"Your sense of peace is gone!",
	AFF_SNEAK					},

  { "enchant armor",
	"Your armor glows slightly!",
	"Your armor stops glowing.",
	0						},

  { "enchant weapon",
	"You weapon glows slightly!",
	"Your weapon stops glowing.",
	0						},

  { "water breathing",
	"The air suddenly feels very dry!",
	"Your lungs feel wet!",
	AFF_WATER_BREATHING				},

 { "darkness",
	"You are covered by utter darkness.",
	"Bright light hurts your eyes!",
	AFF_DARKNESS				},

 { "aura",
	"You reveal your true aura.",
	"Your aura disappears!",
	AFF_AURA				},

 { "hallucinating",
	"{mW{co{yw{r. {mJ{ce{ys{ru{ms {co{yn {ra {mb{ci{yc{ry{mc{cl{ye{r.",
	"Everything is so boring again!",
	AFF_HALLUCINATING				},

 { "relaxed",
	"You are completely cool now.",
	"You're feeling nervous!",
	AFF_RELAXED				},

 { "fire shield",
	"You are surrounded by flames.",
	"The flames fade!",
	AFF_FIRE_SHIELD				},

 { "frost shield",
	"You are surrounded by frost.",
	"The frost fades!",
	AFF_FROST_SHIELD				},

 { "lightning shield",
	"You are surrounded by electricity.",
	"The electricity fades!",
	AFF_LIGHTNING_SHIELD				},

 { "slowed",
	"It's hard to move.",
	"Moving becomes easier again!",
	AFF_SLOW				},

 { "globe-of-protection",
	"You are surrounded by a globe of protection.",
	"The globe of protection fades!",
	AFF_GLOBE				},

 { "morf",
	"Your shape feels most uncertain.",
	"You feel solid again",
	AFF_MORF				},

 { "incarnated",
	"You become mortal.",
	"You are immortal again.",
	AFF_INCARNATED				},


 { "asceticism",
	"You feel no more needs.",
	"You feel your needs again.",
	AFF_ASCETICISM				},

  { "starvation",
	"Your stomach growls!",
	"You stomach feels contented!",
	0						},

  { "dehydration",
	"Your throat is very dry!",
	"Your throat feels better!",
	0						},

  { "obesity",
	"Hmmm. You seem to be putting on a few pounds!",
	"You feel fitter!",
	0						},

  { "head wound",
	"Your head hurts terribly!",
	"You head feels better!",
	0						},

  { "body wound",
	"Your torso hurts terribly!",
	"You torso feels better!",
	0						},

  { "arm wound",
	"Your arms hurts terribly!",
	"You arms feels better!",
	0						},

  { "leg wound",
	"Your legs hurts terribly!",
	"You legs feels better!",
	0						},

  { "intoxication",
	"You feel somewhat merry!",
	"You feel somewhat more sober!",
	0						},

  { "hangover",
	"Now you just feel sick!",
	"You feel better!",
	0						},

  { "size",
	"You feel your size changed.",
	"You feel your size is normal again",
	0						},

  { "fatigue",
	"Your brain feels all fuzzy!",
	"You feel more normal!",
	0						},

  { "iron skin",
	"Your skin becomes as hard as iron!",
	"Your skin feels soft again.",
	0						},

  { "protection fire",
	"You don't feel the heat!",
	"You feel less protected.",
	0						},

  { "protection frost",
	"You don't feel the cold!",
	"You feel less protected.",
	0						},

  { "protection lightning",
	"You feel grounded!",
	"You feel less protected.",
	0						},

  { "protection energy",
	"You feel shielded!",
	"You feel less protected.",
	0						},

  { "antipsychotica",
	"You feel calm and peaceful.",
	"You feel disgusted and angry.",
	AFF_RELAXED					},

  { "room holy",
	"Everything seems so full of awe.",
	"Everything seems normal again.",
	RAFF_HOLY						},

  { "room evil",
	"Everything semms filled with the power of evil.",
	"Everything seems normal again.",
	RAFF_EVIL						},

  { "room silence",
	"Your can't hear anything.",
	"You can hear again.",
	RAFF_SILENCE						},

  { "room darkness",
	"Your can't see anything.",
	"Your van see again.",
	RAFF_DARKNESS						},

  { "room wild magic",
	"Magic seems very chaotic now.",
	"Magic seems normal again.",
	RAFF_WILD_MAGIC						},

  { "room no_breathe",
	"You can't seem to breathe.",
	"You can breathe again.",
	RAFF_NOBREATHE						},

  { "room drain",
	"You begin to feel weak.",
	"You feel better.",
	RAFF_DRAIN						},

  { "room mare",
	"This is a creepy place.",
	"You feel relieved.",
	RAFF_MARE						},

  { "room low magic",
	"Magic seems weak here.",
	"Magic seems normal again.",
	RAFF_LOW_MAGIC						},

  { "room high magic",
	"Magic seems strong here.",
	"Magic seems normal again.",
	RAFF_HIGH_MAGIC						},

  { "room enclosed",
	"A force field surrounds this room.",
	"The force field collapses.",
	RAFF_ENCLOSED						},

  { "room no_morgue",
	"No powers of light are protecting you here.",
	"The powers of light are protecting you again.",
	RAFF_NOMORGUE						},

  { "room fatigue",
	"You feel so tired.",
	"You feel better again.",
	RAFF_FATIGUE						},

  { "room destructive",
	"You feel your possessions degrade.",
	"You stuff is safe again.",
	RAFF_DESTRUCTIVE						},

  { "object blade affect",
	"Your blade starts to hunger for blood.",
	"You blade is cold and dead.",
	0						},

  { NULL,		NULL				}
};


/* Find the afn of a given affect... */

int get_affect_afn(char *name) {
int afn;

  for(afn = 0; afn < MAX_AFFECT; afn++) {

    if ( affect_array[afn].name == NULL ) return AFFECT_UNDEFINED;

    if ( name[0] == affect_array[afn].name[0] 
    && !str_cmp(name, affect_array[afn].name) ) {
      return afn;
    }
  }

  return AFFECT_UNDEFINED;
}


/* Find the gafn for an affect and complain if it can't be found... */

int get_affect_gafn(char *name) {
int gafn;
char buf[MAX_STRING_LENGTH];

  gafn = get_affect_afn(name);

  if (gafn == AFFECT_UNDEFINED) {
    sprintf(buf, "Affect '%s' undefined, can't set gafn.", name);
    bug(buf, 0);
    exit(1);
  } 

  return gafn;
}


/* See if an afn is valid... */

bool valid_affect(int afn) {

  if (afn < 0 
  || afn > MAX_AFFECT) {
    return FALSE;
  }

  if (affect_array[afn].name == NULL) return FALSE;

  return TRUE;
}


/* Apply an affect to a character... */

void affect_char(CHAR_DATA *ch, int afn, bool join, int level, int duration, int location, int modifier, int skill, long long bitvector) {
AFFECT_DATA af;

 /* Quick check... */

  if (!valid_affect(afn)) {
    bug("Invalid affect '%d' passed to affect_char", afn);
    return;
  }

 /* Set up the parameters... */

  af.type	= SKILL_UNDEFINED;
  af.afn 	= afn;	
  af.level	= level;
  af.duration	= duration;
  af.location	= location;
  af.modifier	= modifier;
  af.skill      = skill;
  af.bitvector	= bitvector;

 /* Apply... */

  if (join) affect_join(ch, &af);
  else affect_to_char( ch, &af );

  return;
}


/* Give an affect to a char... */
 
void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf ) {
AFFECT_DATA *paf_new;

   /* Get a new (empty) affect block... */

    paf_new = new_affect(); 

   /* Next statement seems to copy the effect details over... */

    *paf_new		= *paf;

   /* Sort out the skill... */

    if ( paf_new->location != APPLY_SKILL ) paf_new->skill = SKILL_UNDEFINED;

   /* Link into the characters chain... */

    paf_new->next	= ch->affected;
    ch->affected	= paf_new;

   /* Sort of the afn... */

    if (!valid_affect(paf->afn)) paf->afn = AFFECT_UNDEFINED;

   /* Apply whatever it does... */

    affect_modify( ch, paf_new, TRUE );
    return;
}


/* Give an affect to an object */

void affect_to_obj(OBJ_DATA *obj, AFFECT_DATA *paf) {
AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new		= *paf;
    paf_new->next	= obj->affected;
    obj->affected	= paf_new;

    return;
}


/* Remove an affect from a char... */

void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf ) {

    if ( ch->affected == NULL ) {
	bug( "Affect_remove: no affect.", 0 );
	return;
    }

    affect_modify( ch, paf, FALSE );

    if ( paf == ch->affected ) {
	ch->affected	= paf->next;
    } else {
	AFFECT_DATA *prev;

	for ( prev = ch->affected; prev != NULL; prev = prev->next ) {
	    if ( prev->next == paf ) {
		prev->next = paf->next;
		break;
	    }
	}

	if ( prev == NULL ) {
	    bug( "Affect_remove: cannot find paf.", 0 );
	    return;
	}
    }

    paf->next	= affect_free;
    affect_free	= paf;
    return;
}


/* Remove an affect from an object... */

void affect_remove_obj( OBJ_DATA *obj, AFFECT_DATA *paf) {
    if ( obj->affected == NULL ) {
        bug( "Affect_remove_object: no affect.", 0 );
        return;
    }

    if (obj->carried_by != NULL && obj->wear_loc != -1) affect_modify( obj->carried_by, paf, FALSE );

    if ( paf == obj->affected ) {
        obj->affected    = paf->next;
    } else {
        AFFECT_DATA *prev;

        for ( prev = obj->affected; prev != NULL; prev = prev->next )  {
            if ( prev->next == paf ) {
                prev->next = paf->next;
                break;
            }
        }

        if ( prev == NULL ) {
            bug( "Affect_remove_object: cannot find paf.", 0 );
            return;
        }
    }

    paf->next   = affect_free;
    affect_free = paf;
    return;
}


/* Remove all affects of a given sn from a char... */

void affect_strip( CHAR_DATA *ch, int sn ) {
AFFECT_DATA *paf;
AFFECT_DATA *paf_next;

    for ( paf = ch->affected; paf != NULL; paf = paf_next ) {
	paf_next = paf->next;
	if ( paf->type == sn ) affect_remove( ch, paf );
    }

    return;
}


/* Remove all affects of a given afn from a char... */

void affect_strip_afn( CHAR_DATA *ch, int afn ) {
AFFECT_DATA *paf;
AFFECT_DATA *paf_next;

    for ( paf = ch->affected; paf != NULL; paf = paf_next ) {
                paf_next = paf->next;
	if ( paf->afn == afn ) affect_remove( ch, paf );
    }

    return;
}


/* Return true if a char is affected by an affect... */

bool is_affected( CHAR_DATA *ch, int afn ) {
AFFECT_DATA *paf;

    for ( paf = ch->affected; paf; paf = paf->next ) {
        if (paf->afn == afn) return TRUE;
    }

    return FALSE;
}


bool obj_affected(OBJ_DATA *obj, int afn ) {
AFFECT_DATA *paf;

    for (paf = obj->affected; paf; paf = paf->next ) {
        if (paf->afn == afn) return TRUE;
    }

    if (obj->pIndexData) {
        for (paf = obj->pIndexData->affected; paf; paf = paf->next ) {
            if (paf->afn == afn) return TRUE;
        }
    }

    return FALSE;
}


/* Add or enhance an affect upon a character... */

void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf ) {
AFFECT_DATA *paf_old;
bool found;

    found = FALSE;
    for ( paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next ) {
	if ( paf_old->afn == paf->afn ) {
	    paf->level = (paf->level += paf_old->level) / 2;
	    paf->duration += paf_old->duration;
	    paf->modifier += paf_old->modifier;
	    affect_remove( ch, paf_old );
	    break;
	}
    }

    affect_to_char( ch, paf );
    return;
}

/* Static vars to track recursion when removing effects... */

static int depth;
static int depth2;


/* Apply or remove an affect to a character... */ 

void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) {
OBJ_DATA *wield;
OBJ_DATA *objtmp;
int mod,i;
bool duplicate=FALSE;
AFFECT_DATA *tmp;

    mod = paf->modifier;
    if (paf->location == APPLY_OBJECT) return;


    if ( fAdd ) {
      SET_BIT( ch->affected_by, paf->bitvector );
    } else {
      if (paf->bitvector) {
	    
	for (tmp = ch->affected; tmp; tmp = tmp->next) {
	  if ((tmp != paf) && (tmp->bitvector == paf->bitvector)) {
	    duplicate=TRUE;
	    break;
	  }
	}

	/* check against other worn objects if no duplicate found yet*/
	if (!duplicate) {
	  for ( objtmp = ch->carrying; objtmp; objtmp = objtmp->next_content ) { 
		
	    if (objtmp->wear_loc != WEAR_NONE) {
	        for (tmp = objtmp->affected; tmp; tmp = tmp->next) {
	            if ((tmp != paf) 
                            && (tmp->bitvector == paf->bitvector)) {
		  duplicate=TRUE;
		  break;
	            }
	        }	

	        for (tmp = objtmp->pIndexData->affected; tmp; tmp = tmp->next) {
	            if ((tmp != paf) 
                            && (tmp->bitvector == paf->bitvector)) {
		  duplicate=TRUE;
		  break;
	            }
	        }
                    }

	    if (duplicate) break;
                  }
	}

       /* If it was the only one, we can remove it bitmod... */

        if (!duplicate) {
              REMOVE_BIT( ch->affected_by, paf->bitvector );
              ch->affected_by = ch->affected_by | race_array[ch->race].aff;
        }    
      }
    }

   /* If we're not adding, remove the modifier... */ 

    if (!fAdd) mod = 0 - mod;

   /* Now make the change... */

    switch ( paf->location )    {
    default:
	bug( "Affect_modify: unknown location %d.", paf->location );
	return;

    case APPLY_NONE:
    case APPLY_MAX_STR:
    case APPLY_MAX_DEX:
    case APPLY_MAX_INT:
    case APPLY_MAX_WIS:
    case APPLY_MAX_CON:
    case APPLY_MAX_LUCK:
    case APPLY_MAX_CHA:
        break;
    case APPLY_STR:           ch->mod_stat[STAT_STR]	+= mod;	break;
    case APPLY_DEX:          ch->mod_stat[STAT_DEX]	+= mod;	break;
    case APPLY_INT:           	ch->mod_stat[STAT_INT]		+= mod;	break;
    case APPLY_WIS:          	ch->mod_stat[STAT_WIS]	+= mod;	break;
    case APPLY_CON:          ch->mod_stat[STAT_CON]	+= mod;	break;
    case APPLY_LUCK:       	ch->mod_stat[STAT_LUC]	+= mod;	break;
    case APPLY_CHA:        	ch->mod_stat[STAT_CHA]	+= mod;	break;
    case APPLY_SEX:          	ch->sex				+= mod;	break;
    case APPLY_CLASS:						break;
    case APPLY_LEVEL:						break;
    case APPLY_AGE:
          if (!IS_NPC(ch)) {
             ch->pcdata->age_mod -= mod;
             if (mod < 0) send_to_char("You feel a lot more vital.\r\n",ch);
             else if (mod > 0) send_to_char("You somehow feel older.\r\n",ch);
          }
          break;
    case APPLY_HEIGHT:						break;
    case APPLY_WEIGHT:						break;
    case APPLY_MAGIC:						break;
    case APPLY_ALIGN:						break;
    case APPLY_MANA:    	ch->max_mana			+= mod;	break;
    case APPLY_HIT:          	ch->max_hit			+= mod;	break;
    case APPLY_MOVE:      	ch->max_move			+= mod;	break;
    case APPLY_GOLD:						break;
    case APPLY_SANITY_GAIN:					break;
    case APPLY_EXP:						break;
    case APPLY_AC:
        for (i = 0; i < 4; i ++)
            ch->armor[i] += mod;
        break;
    case APPLY_HITROLL:       	ch->hitroll		+= mod;	break;
    case APPLY_DAMROLL:       	ch->damroll		+= mod;	break;
    case APPLY_SAVING_PARA:   	ch->saving_throw	+= mod;	break;
    case APPLY_SAVING_ROD:    	ch->saving_throw	+= mod;	break;
    case APPLY_SAVING_PETRI:  	ch->saving_throw	+= mod;	break;
    case APPLY_SAVING_BREATH:	ch->saving_throw	+= mod;	break;
    case APPLY_SAVING_SPELL:  	ch->saving_throw	+= mod;	break;
    case APPLY_SANITY:                	ch->sanity += mod;
        if (ch->sanity < 1) {
	if (!IS_AFFECTED(ch, AFF_FEAR)
	&& !IS_SET(ch->act,ACT_UNDEAD)) {
  	    AFFECT_DATA af;
   	    af.type      = SKILL_UNDEFINED;
	    af.afn	     = gafn_fear;              
   	    af.level     = ch->level;
   	    af.duration  = ch->level / 6;
   	    af.location  = 0;
   	    af.modifier  = 0;
   	    af.bitvector = AFF_FEAR;
   	    affect_to_char(ch, &af );
	    send_to_char ("For just a moment, an intense feeling of fear washes over you.\r\n",ch);
              }
        } 
        break;

    case APPLY_SKILL:
      if ( validSkill(paf->skill)
        && paf->skill > 0
        && isSkilled(ch) ) {
        ch->effective->skill[paf->skill] += mod;
      }
      break;

    case APPLY_EFFECT:
      break;

    case APPLY_IMMUNITY:
       if (mod>0) {
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
       }
       if (mod<0) {
          mod =0-mod;
          switch (mod) {            
              case 1: REMOVE_BIT(ch->imm_flags, IMM_SUMMON); 	break;
              case 2: REMOVE_BIT(ch->imm_flags, IMM_CHARM); 	break;
              case 3: REMOVE_BIT(ch->imm_flags, IMM_MAGIC); 		break;
              case 4: REMOVE_BIT(ch->imm_flags, IMM_WEAPON); 	break;
              case 5: REMOVE_BIT(ch->imm_flags, IMM_BASH); 		break;
              case 6: REMOVE_BIT(ch->imm_flags, IMM_PIERCE); 		break;
              case 7: REMOVE_BIT(ch->imm_flags, IMM_SLASH); 		break;
              case 8: REMOVE_BIT(ch->imm_flags, IMM_FIRE); 		break;
              case 9: REMOVE_BIT(ch->imm_flags, IMM_COLD); 		break;
              case 10: REMOVE_BIT(ch->imm_flags, IMM_LIGHTNING); 	break;
              case 11: REMOVE_BIT(ch->imm_flags, IMM_ACID); 		break;
              case 12: REMOVE_BIT(ch->imm_flags, IMM_POISON); 	break;
              case 13: REMOVE_BIT(ch->imm_flags, IMM_NEGATIVE); 	break;
              case 14: REMOVE_BIT(ch->imm_flags, IMM_HOLY); 		break;
              case 15: REMOVE_BIT(ch->imm_flags, IMM_ENERGY); 	break;
              case 16: REMOVE_BIT(ch->imm_flags, IMM_MENTAL); 	break;
              case 17: REMOVE_BIT(ch->imm_flags, IMM_DISEASE); 	break;
              case 18: REMOVE_BIT(ch->imm_flags, IMM_DROWNING); 	break;
              case 19: REMOVE_BIT(ch->imm_flags, IMM_LIGHT); 		break;
              case 20: REMOVE_BIT(ch->imm_flags, IMM_BULLETS); 	break;
              case 21: REMOVE_BIT(ch->imm_flags, IMM_MASK); 	break;
              case 22: REMOVE_BIT(ch->imm_flags, IMM_OLD); 		break;
              case 23: REMOVE_BIT(ch->imm_flags, IMM_SOUND); 	break;
               default: break;
          }
          ch->imm_flags = ch->imm_flags | race_array[ch->race].imm;
       }     
       break;

    case APPLY_ENV_IMMUNITY:
       if (mod>0) {
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
       }
       if (mod<0) {
          mod =0-mod;
          switch (mod) {            
              case 3: REMOVE_BIT(ch->envimm_flags, IMM_MAGIC); 	break;
              case 8: REMOVE_BIT(ch->envimm_flags, IMM_FIRE); 	break;
              case 9: REMOVE_BIT(ch->envimm_flags, IMM_COLD); 	break;
              case 10: REMOVE_BIT(ch->envimm_flags, IMM_LIGHTNING); break;
              case 11: REMOVE_BIT(ch->envimm_flags, IMM_ACID); 	break;
              case 12: REMOVE_BIT(ch->envimm_flags, IMM_POISON); 	break;
              case 13: REMOVE_BIT(ch->envimm_flags, IMM_NEGATIVE); 	break;
              case 14: REMOVE_BIT(ch->envimm_flags, IMM_HOLY); 	break;
              case 15: REMOVE_BIT(ch->envimm_flags, IMM_ENERGY); 	break;
              case 16: REMOVE_BIT(ch->envimm_flags, IMM_MENTAL); 	break;
              case 17: REMOVE_BIT(ch->envimm_flags, IMM_DISEASE); 	break;
              case 18: REMOVE_BIT(ch->envimm_flags, IMM_DROWNING);break;
              case 19: REMOVE_BIT(ch->envimm_flags, IMM_LIGHT); 	break;
              case 22: REMOVE_BIT(ch->envimm_flags, IMM_OLD); 	break;
              case 23: REMOVE_BIT(ch->envimm_flags, IMM_SOUND); 	break;
               default: break;
          }
          ch->imm_flags = ch->imm_flags | race_array[ch->race].imm;
       }     
       break;

    case APPLY_RESISTANCE:
       if (mod>0) {
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
       }
       if (mod<0) {
          mod =0-mod;
          switch (mod) {            
              case 2: REMOVE_BIT(ch->res_flags, RES_CHARM);	 	break;
              case 3: REMOVE_BIT(ch->res_flags, RES_MAGIC); 		break;
              case 4: REMOVE_BIT(ch->res_flags, RES_WEAPON); 	break;
              case 5: REMOVE_BIT(ch->res_flags, RES_BASH); 		break;
              case 6: REMOVE_BIT(ch->res_flags, RES_PIERCE); 		break;
              case 7: REMOVE_BIT(ch->res_flags, RES_SLASH); 		break;
              case 8: REMOVE_BIT(ch->res_flags, RES_FIRE); 		break;
              case 9: REMOVE_BIT(ch->res_flags, RES_COLD); 		break;
              case 10: REMOVE_BIT(ch->res_flags, RES_LIGHTNING); 	break;
              case 11: REMOVE_BIT(ch->res_flags, RES_ACID); 		break;
              case 12: REMOVE_BIT(ch->res_flags, RES_POISON); 		break;
              case 13: REMOVE_BIT(ch->res_flags, RES_NEGATIVE); 	break;
              case 14: REMOVE_BIT(ch->res_flags, RES_HOLY); 		break;
              case 15: REMOVE_BIT(ch->res_flags, RES_ENERGY); 		break;
              case 16: REMOVE_BIT(ch->res_flags, RES_MENTAL); 	break;
              case 17: REMOVE_BIT(ch->res_flags, RES_DISEASE); 	break;
              case 18: REMOVE_BIT(ch->res_flags, RES_DROWNING); 	break;
              case 19: REMOVE_BIT(ch->res_flags, RES_LIGHT); 		break;
              case 20: REMOVE_BIT(ch->res_flags, RES_BULLETS); 	break;
              case 21: REMOVE_BIT(ch->res_flags, RES_MASK); 		break;
              case 22: REMOVE_BIT(ch->res_flags, RES_OLD); 		break;
              case 23: REMOVE_BIT(ch->res_flags, RES_SOUND); 		break;
               default: break;
          }
          ch->res_flags = ch->res_flags | race_array[ch->race].res;
       }     
       break;
    }


    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */

    if ( !IS_NPC(ch) ) {

     /* Check primary wield... */

      wield = get_eq_char( ch, WEAR_WIELD );

      if ( wield != NULL
        && get_obj_weight(wield) > 
                         str_app[get_curr_stat(ch,STAT_STR)].wield ) {

	if ( depth == 0 ) {

	    depth++;

	    act( "You are no longer strong enough to wield $p.", ch, wield, NULL, TO_CHAR );
	    act( "$n unwields $p.", ch, wield, NULL, TO_ROOM );
                    remove_obj(ch, WEAR_WIELD, TRUE);
	    depth--;
        }
      }

     /* now check the dual wield weapon */ 

      wield = get_eq_char( ch, WEAR_WIELD2 );

      if ( wield != NULL
        && get_obj_weight(wield) > 
                      (str_app[get_curr_stat(ch,STAT_STR)].wield * 3)/4 ) {
    
	if ( depth2 == 0 ) {

	    depth2++;

	    act( "You are no longer strong enough to dual wield $p.", ch, wield, NULL, TO_CHAR );
	    act( "$n unwields $p.", ch, wield, NULL, TO_ROOM );
                    remove_obj(ch, WEAR_WIELD2, TRUE);
	    depth2--;
	}
      }
    }

    return;
}

/* Return ascii name of an affect location... */

char *affect_loc_name( int location ) {

    switch ( location ) {
    case APPLY_NONE:		return "none";
    case APPLY_STR:		return "strength";
    case APPLY_DEX:		return "dexterity";
    case APPLY_INT:		return "intelligence";
    case APPLY_WIS:		return "wisdom";
    case APPLY_CON:		return "constitution";
    case APPLY_LUCK:		return "luck";
    case APPLY_CHA:		return "charisma";
    case APPLY_MAX_STR:              return "max strength";
    case APPLY_MAX_DEX:           	return "max dexterity";
    case APPLY_MAX_INT:           	return "max intelligence";
    case APPLY_MAX_WIS:	return "max wisdom";
    case APPLY_MAX_CON: 	return "max constritution";
    case APPLY_MAX_LUCK:	return "max luck";
    case APPLY_MAX_CHA: 	return "max charisma";
    case APPLY_SEX:		return "sex";
    case APPLY_CLASS:		return "class";
    case APPLY_LEVEL:		return "level";
    case APPLY_SANITY:		return "sanity";
    case APPLY_SKILL:		return "skill";
    case APPLY_AGE:		return "age";
    case APPLY_MANA:		return "mana";
    case APPLY_HIT:		return "hp";
    case APPLY_MOVE:		return "moves";
    case APPLY_GOLD:		return "gold";
    case APPLY_EXP:		return "experience";
    case APPLY_AC:		return "armor class";
    case APPLY_HITROLL:		return "hit roll";
    case APPLY_DAMROLL:	return "damage roll";
    case APPLY_SAVING_PARA:	return "save vs paralysis";
    case APPLY_SAVING_ROD:	return "save vs rod";
    case APPLY_SAVING_PETRI:	return "save vs petrification";
    case APPLY_SAVING_BREATH:	return "save vs breath";
    case APPLY_SAVING_SPELL:	return "save vs spell";
    case APPLY_IMMUNITY:	return "immunity";
    case APPLY_RESISTANCE:	return "resistance";
    case APPLY_EFFECT:		return "effect";
    case APPLY_HEIGHT:		return "size";
    case APPLY_MAGIC:		return "magic";
    case APPLY_ALIGN:		return "alignment";
    case APPLY_CRIME_RECORD:	return "crime record";
    case APPLY_OBJECT:		return "object apply";
    case APPLY_SANITY_GAIN:	return "sanity gain";
    case APPLY_ENV_IMMUNITY:	return "env-immunity";
    }

    bug( "Affect_location_name: unknown location %d.", location );
    return "(unknown)";
}


/* Return ascii name of an affect bit vector... */

char *affect_bit_name( long long vector ) {
static char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';
    if ( vector & AFF_BLIND) 		strcat( buf, " blind");
    if ( vector & AFF_INVISIBLE) 		strcat( buf, " invisible");
    if ( vector & AFF_DETECT_EVIL) 	strcat( buf, " detect-evil");
    if ( vector & AFF_DETECT_INVIS) 	strcat( buf, " detect-invis");
    if ( vector & AFF_DETECT_MAGIC) 	strcat( buf, " detect-magic");
    if ( vector & AFF_DETECT_HIDDEN) 	strcat( buf, " detect-hidden");
    if ( vector & AFF_MELD) 		strcat( buf, " mind_meld");
    if ( vector & AFF_SANCTUARY) 	strcat( buf, " sanctuary");
    if ( vector & AFF_FAERIE_FIRE) 	strcat( buf, " faerie_fire");
    if ( vector & AFF_INFRARED) 		strcat( buf, " infrared");
    if ( vector & AFF_CURSE) 		strcat( buf, " curse");
    if ( vector & AFF_FEAR) 		strcat( buf, " fear");
    if ( vector & AFF_POISON) 		strcat( buf, " poison");
    if ( vector & AFF_PROTECTG) 		strcat( buf, " protect-good");
    if ( vector & AFF_PROTECTE) 		strcat( buf, " protect-evil");
    if ( vector & AFF_SNEAK) 		strcat( buf, " sneak");
    if ( vector & AFF_HIDE) 		strcat( buf, " hide");
    if ( vector & AFF_SLEEP) 		strcat( buf, " sleep");
    if ( vector & AFF_CHARM) 		strcat( buf, " charm");
    if ( vector & AFF_FLYING) 		strcat( buf, " flying");
    if ( vector & AFF_PASS_DOOR) 		strcat( buf, " pass-door");
    if ( vector & AFF_HASTE) 		strcat( buf, " haste");
    if ( vector & AFF_CALM) 		strcat( buf, " calm");
    if ( vector & AFF_PLAGUE) 		strcat( buf, " plague");
    if ( vector & AFF_WEAKEN) 		strcat( buf, " weaken");
    if ( vector & AFF_DARK_VISION) 	strcat( buf, " dark-vision");
    if ( vector & AFF_BERSERK) 		strcat( buf, " berserk");
    if ( vector & AFF_SWIM) 		strcat( buf, " swim");
    if ( vector & AFF_REGENERATION) 	strcat( buf, " regeneration");
    if ( vector & AFF_POLY) 		strcat( buf, " polymorph");
    if ( vector & AFF_ABSORB) 		strcat( buf, " absorb-magic");
    if ( vector & AFF_WATER_BREATHING)	strcat( buf, " water-breathing");
    if ( vector & AFF_DARKNESS) 		strcat( buf, " darkness");
    if ( vector & AFF_AURA) 		strcat( buf, " aura");
    if ( vector & AFF_HALLUCINATING) 	strcat( buf, " hallucinating");
    if ( vector & AFF_RELAXED) 		strcat( buf, " relaxed");
    if ( vector & AFF_FIRE_SHIELD) 	strcat( buf, " fire-shield");
    if ( vector & AFF_FROST_SHIELD) 	strcat( buf, " frost-shield");
    if ( vector & AFF_SLOW) 		strcat( buf, " slowed");
    if ( vector & AFF_GLOBE) 		strcat( buf, " globe-of-protection");
    if ( vector & AFF_MORF) 		strcat( buf, " morf");
    if ( vector & AFF_INCARNATED) 	strcat( buf, " incarnated");
    if ( vector & AFF_MIST) 		strcat( buf, " mist");
    if ( vector & AFF_ELDER_SHIELD) 	strcat( buf, " elder-shield");
    if ( vector & AFF_FARSIGHT) 		strcat( buf, " farsight");

    return (char*)( ( buf[0] != '\0' ) ? buf+1 : "none" );
}

void affect_to_room( ROOM_INDEX_DATA *room, AFFECT_DATA *paf ) {
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new            = *paf;
    paf_new->next       = room->affected;
    room->affected      = paf_new;
    SET_BIT(room->affected_by,paf->bitvector);

    return;
}

void affect_remove_room( ROOM_INDEX_DATA *room, AFFECT_DATA *paf ) {
int vector;

    if ( room->affected == NULL )    {
        bug( "Affect_remove_room: no affect.", 0 );
        return;
    }
 
    vector = paf->bitvector;

    if (paf->bitvector)  REMOVE_BIT(room->affected_by,paf->bitvector);

    if ( paf == room->affected )    {    
        room->affected    = paf->next;
    }    else    {
        AFFECT_DATA *prev;

        for ( prev = room->affected; prev != NULL; prev = prev->next ) {
            if ( prev->next == paf ) {
                prev->next = paf->next;
                break;
            }
        }

        if ( prev == NULL )        {
            bug( "Affect_remove_room: cannot find paf.", 0 );
            return;
        }
    }

    free_affect(paf);
    return;
}


int check_weapon_affect_at(OBJ_DATA *obj) {
AFFECT_DATA *af;

     if (!obj_affected(obj, gafn_obj_blade_affect)) return WDT_HIT;

     for (af = obj->affected; af; af = af->next) {
         if (af->location == APPLY_OBJECT) {
              if (af->afn == gafn_obj_blade_affect) return af->modifier;
         }
     }

     return WDT_HIT;
}


int get_weapon_affect_level(OBJ_DATA *obj) {
AFFECT_DATA *af;

     if (!obj_affected(obj, gafn_obj_blade_affect)) return 0;

     for (af = obj->affected; af; af = af->next) {
         if (af->location == APPLY_OBJECT) {
              if (af->afn == gafn_obj_blade_affect) return af->level;
         }
     }

     return 0;
}


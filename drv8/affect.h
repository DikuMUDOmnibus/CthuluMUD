/*
 * CthulhuMud
 */

/* Number of effects defined + 1 */
 
#define MAX_AFFECT		96

/* Place holder for undefined affects... */

#define AFFECT_UNDEFINED	-1  

/* Affect info structure and data type... */

typedef struct	affect_info		AFFECT_INFO;

struct affect_info {
	char *name;
	char *start_msg;
	char *end_msg;
	long long bitvector;
};

/* Main external array... */

extern const AFFECT_INFO affect_array[MAX_AFFECT];

/* Declare common gafns... */

#ifdef IN_DB_C
#define GAFN(x) int gafn_ ##x;
#else
#define GAFN(x) extern int gafn_ ## x;
#endif

GAFN(stats)
GAFN(armor)
GAFN(bless)
GAFN(blindness)
GAFN(charm)
GAFN(chill_touch)
GAFN(curse)
GAFN(detect_evil)
GAFN(detect_invis)
GAFN(detect_magic)
GAFN(invisability)
GAFN(poison)
GAFN(protection_evil)
GAFN(sanctuary)
GAFN(sleep)
GAFN(giant_strength)
GAFN(detect_hidden)
GAFN(fly)
GAFN(mist)
GAFN(elder_shield)
GAFN(farsight)
GAFN(hard_skin)
GAFN(shield)
GAFN(mana_shield)
GAFN(weakness)
GAFN(faerie_fire)
GAFN(pass_door)
GAFN(infravision)
GAFN(sex_change)
GAFN(mind_meld)
GAFN(haste)
GAFN(plague)
GAFN(frenzy)
GAFN(calm)
GAFN(mask_self)
GAFN(mask_hide)
GAFN(mask_force)
GAFN(absorb_magic)
GAFN(fear)
GAFN(protection_good)
GAFN(detect_good)
GAFN(regeneration)
GAFN(vocalize)
GAFN(mute)
GAFN(berserk)
GAFN(sneak)
GAFN(water_breathing)
GAFN(dirt_kicking)
GAFN(enchant_weapon)
GAFN(enchant_armor)
GAFN(fatigue)
GAFN(starvation)
GAFN(dehydration)
GAFN(obesity)
GAFN(head_wound)
GAFN(body_wound)
GAFN(arm_wound)
GAFN(leg_wound)
GAFN(intoxication)
GAFN(hangover)
GAFN(size)
GAFN(darkness)
GAFN(aura)
GAFN(hallucinating)
GAFN(relaxed)
GAFN(fire_shield)
GAFN(frost_shield)
GAFN(lightning_shield)
GAFN(protection_fire)
GAFN(protection_frost)
GAFN(protection_lightning)
GAFN(protection_energy)
GAFN(slow)
GAFN(globe)
GAFN(morf)
GAFN(incarnated)
GAFN(asceticism)
GAFN(antipsychotica)
GAFN(room_holy)
GAFN(room_evil)
GAFN(room_silence)
GAFN(room_darkness)
GAFN(room_wild_magic)
GAFN(room_low_magic)
GAFN(room_high_magic)
GAFN(room_no_breathe)
GAFN(room_drain)
GAFN(room_mare)
GAFN(room_enclosed)
GAFN(room_no_morgue)
GAFN(room_fatigue)
GAFN(room_destructive)
GAFN(obj_blade_affect)

/* Find the afn of a given affect... */

extern int get_affect_afn(char *name);

/* Find the gafn for an affect and complain if it can't be found... */

extern int get_affect_gafn(char *name);

/* See if an afn is valid... */

extern bool valid_affect(int afn);

/* Free chain for affect blocks... */

extern AFFECT_DATA *affect_free;

/* Apply an affect to a character... */

extern void affect_char(CHAR_DATA *ch, int afn, bool join, int level, int duration, int location, int modifier, int skill, long long bitvector);

/* Give an affect to a char... */
 
extern void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf );

/* Give an affect to an object */

extern void affect_to_obj(OBJ_DATA *obj, AFFECT_DATA *paf);

/* Give an affect to a room... */

extern void affect_to_room(ROOM_INDEX_DATA *room, AFFECT_DATA *paf); 

/* Remove an affect from a room... */

extern void affect_remove_room( ROOM_INDEX_DATA *room, AFFECT_DATA *paf); 

/* Remove an affect from a char... */

extern void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf );

/* Remove an affect from an object... */

extern void affect_remove_obj( OBJ_DATA *obj, AFFECT_DATA *paf);

/* Remove all affects of a given sn from a char... */

extern void affect_strip( CHAR_DATA *ch, int sn );

/* Remove all affects of a given afn from a char... */

extern void affect_strip_afn( CHAR_DATA *ch, int afn );

/* Return true if a char is affected by an affect... */

extern bool is_affected(CHAR_DATA *ch, int sn);
extern bool obj_affected(OBJ_DATA *obj, int sn);

/* Add or enhance an affect upon a character... */

extern void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf );

/* Apply or remove an affect to a character... */ 

extern void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd );

/* Work out a name for an affect location... */

extern char *affect_loc_name(int loc);

/* Work out a name for a bit effect... */

extern char *affect_bit_name(long long bitset);

int check_weapon_affect_at (OBJ_DATA *obj);
int get_weapon_affect_level(OBJ_DATA *obj);


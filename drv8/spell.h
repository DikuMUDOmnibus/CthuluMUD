/*
 * CthulhuMud
 */

/* Number of spells defined + 1 */
 
#define MAX_SPELL		301
#define MAX_EFFECT		301

/* Place holder for undefined affects... */

#define SPELL_UNDEFINED		-1  

#define EFFECT_UNDEFINED	-1  

/* Structure to hold spell instructions - 3 ints and one string */

typedef struct seq_step SEQ_STEP;

struct seq_step {
	int		 delay;
	int		 instr;
	int		 iparms[5];
	char		*sparms[4];
	ECOND_DATA	*cond;
	SEQ_STEP	*next;  
};

/* Spell info structure and data type... */

typedef struct	spell_info	SPELL_INFO;

struct	spell_info {
	char		*name;
                char		*desc;	
                char                        *info;     
	short		target;
	short		mana;
	short		diff;
                short                     range;
                short		sn;
                short		sn2;
                short		discipline;
	short		music_style;
       	int		music_power;
	bool		parse_info;
	int		effect;
	char		*component;
	SEQ_STEP	*seq;
	ECOND_DATA	*cond;
	bool		 loaded;
};

/* Effect info structure and data type... */

typedef struct	effect_info	EFFECT_INFO;

struct	effect_info {
	char		*name;	
    	SPELL_FUN	*spell_fun;
	int		 slot;
	int		 target;
	char		*noun_damage;
	char 		*affect;
};

/* Spell instruction codes... */

#define STEP_NONE	 0

#define STEP_PREPARE_SPELL	1
#define STEP_CHANT		2
#define STEP_CAST_SPELL	3
#define STEP_EFFECT		4
#define STEP_ECHO		5
#define STEP_MESSAGE		6
#define STEP_ABORT		7
#define STEP_SKIP		8
#define STEP_DAMAGE		9
#define STEP_HEAL		10
#define STEP_POWER_WORD	11
#define STEP_SANITY		12
#define STEP_AREA_DAMAGE	13
#define STEP_AREA_HEAL	14
#define STEP_YELL		15

// P is the limit

#define STEP_FCS_NONE		 0
#define STEP_FCS_ACTOR	(A)
#define STEP_FCS_VICTIM	(B)
#define STEP_FCS_FRIEND	(C)
#define STEP_FCS_FOE		(D)
#define STEP_FCS_ALL		(E)
#define STEP_FCS_NOT_FRIEND	(F)
#define STEP_FCS_NOT_FOE	(G)

// P is the limit

#define STEP_DMG_MIN		(A)
#define STEP_DMG_LOW	(B)
#define STEP_DMG_MED_LOW	(C)
#define STEP_DMG_MED		(D)
#define STEP_DMG_MED_HIGH	(E)
#define STEP_DMG_HIGH	(F)
#define STEP_DMG_FATAL	(G)
#define STEP_DMG_OVERKILL	(H)

#define STEP_DMG_HIT		(I)
#define STEP_DMG_SAVE_HALF	(J)
#define STEP_DMG_SAVE_NONE	(K)

// P is the limit

#define STEP_HEAL_HITS		(A)
#define STEP_HEAL_MANA		(B)
#define STEP_HEAL_MOVE		(C)

/* Main external arrays... */

extern SPELL_INFO spell_array[MAX_SPELL];

extern EFFECT_INFO effect_array[MAX_EFFECT];

/* Declare common gafns... */

/* Find the spn of a given spell... */

extern int get_spell_spn(char *name);

/* Find the efn of a given effect... */

extern int get_effect_efn(char *name);

/* Find the gspn for a spell and complain if it can't be found... */

extern int get_spell_gspn(char *name);

/* Find the gefn for an effect and complain if it can't be found... */

extern int get_effect_gefn(char *name);

/* See if a spell number is valid... */

extern bool valid_spell(int spn);

/* See if an effect number is valid... */

extern bool valid_effect(int efn);

/* Set up spell sns... */

extern void setup_spells();

/* Look up a effect by slot... */

extern int get_effect_for_slot(int slot);

/* Look up the affect number for an effect... */

extern int get_affect_afn_for_effect(int efn);

/* Spell loader... */

void load_spells();

/*
 * CthulhuMud
 */

/* Define combat data structure... */

typedef struct combat_data COMBAT_DATA;

struct combat_data {
  CHAR_DATA 	*attacker;
  CHAR_DATA 	*defender;

  OBJ_DATA 	*atk_primary_weapon;
  OBJ_DATA 	*atk_secondary_weapon;

  OBJ_DATA 	*def_primary_weapon;
  OBJ_DATA 	*def_secondary_weapon;

  bool 		 atk_dual;

  OBJ_DATA 	*atk_weapon;
  short 	 atk_sn;
  int 		 atk_skill;

  int		 difficulty;

  int            atk_dt;
  int 		 atk_type;
  int 		 atk_dam_type;
  int		 atk_blk_type;
  char	 	*atk_desc;

  bool		 atk_berserk;

  OBJ_DATA 	*def_shield;

  int 		 def_h2h_skill;

  int		 damage;
  int		 damage_mult;
};

/* Function declarations... */

extern	void	one_hit(CHAR_DATA *ch, 
			CHAR_DATA *victim, 
			int dt, 
			bool dual, 
			int atk_gsn,
			int difficulty );

extern 	bool 	apply_damage(	COMBAT_DATA *cd	);

extern 	COMBAT_DATA *getCDB(CHAR_DATA *ch, CHAR_DATA *victim, int dt, 
                           bool dual, int atk_gsn, int diff ); 

extern 	bool 	check_spell_hit(COMBAT_DATA *cd, int spell_roll);

bool env_damage( CHAR_DATA *ch, int dam, int dam_type, 
                                                    char *msg1, char *msg2); 

/* Define constants for blocking and parrying... */

#define BLOCK_NONE	(0)

#define BLOCK_SHIELD	(A)
#define BLOCK_PARRY	(B)
#define BLOCK_DODGE	(C)
#define BLOCK_ARMOR	(D)
#define BLOCK_ABSORB    (E)

#define BLOCK_NOPARRY 	(A|C|D|E)

#define BLOCK_ALL 	(A|B|C|D|E)
#define BLOCK_COMBAT	(A|B|C|D)
#define BLOCK_NOSHIELD  (B|C|D)

#define BLOCK_BALL	(A|D|E)
#define BLOCK_BOLT	(A|C|D|E)
#define BLOCK_LIGHTNING (D|E)
#define BLOCK_LIGHT	(A|E)

extern int can_block_dam[];

/* Define constants for armour damage division... */

/* Ever ARMOR_DAM_PTS of armour reduces damage by half... */

#define ARMOR_DAM_PTS  150
#define ARMOR_DAM_PTS2 300

/* Maximum armour is 600 pts - 1/16th damage */

#define ARMOR_DAM_MAX  600 

/* Maximum damage that can be delivered... */

#define MAX_DAMAGE 12000


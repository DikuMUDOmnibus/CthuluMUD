/*
 * CthulhuMud
 */

/* Skill thresholds... */

#define SKILL_UNKNOWN	  0
#define SKILL_DABBLER	  1 
#define SKILL_NOVICE 	 20
#define SKILL_STUDENT	 40
#define SKILL_ADEPT	 60
#define SKILL_MADEPT	 80
#define SKILL_MASTER	100
#define SKILL_GMASTER	120
#define SKILL_EXEMPLAR	150

#define SKILL_DIVIDER	  2 

#define SKILL_RANKS 	  9

#define SKILL_GROUP_NONE   	 0
#define SKILL_GROUP_MAGIC	(A)
#define SKILL_GROUP_COMBAT	(B)
#define SKILL_GROUP_GENERAL	(C)
#define SKILL_GROUP_MLANG	(D)
#define SKILL_GROUP_ALANG	(E)
#define SKILL_GROUP_CLANG	(F)
#define SKILL_GROUP_ACADEMIC	(G)
#define SKILL_GROUP_PRODUCTION  (H)
#define SKILL_GROUP_MAX		(I)

#define SKILL_GROUP_ALL		(A)|(B)|(C)|(D)|(E)|(F)|(G)|(H)
#define SKILL_GROUP_LANG	(D)|(E)|(F)
#define SKILL_GROUP_NOLANG	(A)|(B)|(C)|(G)|(H)
#define SKILL_GROUP_CNG         (B)|(C)

/* Return a new (and cleared) skill data block... */

SKILL_DATA *getSkillData();

/* Returns a skill data block that's no longer needed... */

void freeSkillData(SKILL_DATA *sd);

/* Clears a skill data block... */ 

void clearSkillData(SKILL_DATA *sd);

/* Sets the specified skill to the specified value... */

void setSkill(SKILL_DATA *sd, int sn, int pts);
 
/* Adds a number of points to a specified skill... */

bool addSkill(CHAR_DATA *ch, int sn, int pts);

/* Adds a number of points to a Mobs specified skill... */

bool addMobSkill(MOB_INDEX_DATA *pMob, int sn, int pts);
bool addCharSkill(CHAR_DATA *ch, int sn, int pts);

/* See if a skill number is valid... */

bool validSkill(const int sn);

/* See if a character is skilled... */

bool isSkilled(const CHAR_DATA *ch);

/* Does a character have a skill? */

bool isSkillAvailable(CHAR_DATA *ch, int sn);

/* Can a character practice a skill? */

bool isSkillAvailableForPractice(CHAR_DATA *ch, int sn);

/* Return a characters skill... */

int get_skill(CHAR_DATA *ch, int sn);

/* Return a characters weapon skill... */

int get_weapon_skill(CHAR_DATA *ch, int sn);

/* Return one skill modified by another... */

int get_skill_by(CHAR_DATA *ch, int sn, int sn2);

/* Just make a skill roll... */

int get_skill_roll(CHAR_DATA *ch, int sn);

/* See if a skill works... */

bool check_skill(CHAR_DATA *ch, int sn, int diff);

/* See if a skill is posessed... */

bool has_skill(CHAR_DATA *ch, int sn);

/* Skills improve through usage... */

void check_improve(CHAR_DATA *ch, int sn, bool success, int multiplier);

/* Practice a particular skill... */

void practice_skill(CHAR_DATA *ch, CHAR_DATA *teacher, int num_prac, char *skill, bool isPractice);

/* Get the sn for a skill... */

int get_skill_sn(char *name);

/* Get a skill gsn... */

int get_skill_gsn(char *name);

/* Load skill definitions... */

void load_skills(); 

int 	get_base_skill		(int sn, CHAR_DATA *ch);
void 	adapt_skill		(CHAR_DATA *ch, CHAR_DATA *teacher, int num_prac, char *skill);
char 	*get_skill_group_name	(int sn);
void 	prompt_amateur		(DESCRIPTOR_DATA *d);
void 	practice_amateur		(CHAR_DATA *ch, int sn);




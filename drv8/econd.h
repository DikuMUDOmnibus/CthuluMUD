/*
 * CthulhuMud
 */

/* Subjects for extended conditions... */

#define ECOND_SUBJECT_WORLD		0
#define ECOND_SUBJECT_ACTOR		1
#define ECOND_SUBJECT_VICTIM		2
#define ECOND_SUBJECT_OBSERVER		3
#define ECOND_SUBJECT_RANDOM		4
#define ECOND_SUBJECT_L_ACTOR		5
#define ECOND_SUBJECT_L_VICTIM		6
#define ECOND_SUBJECT_L_OBSERVER		7
#define ECOND_SUBJECT_L_RANDOM		8
#define ECOND_SUBJECT_P_OBJ		 	9
#define ECOND_SUBJECT_S_OBJ			10	
#define ECOND_SUBJECT_WEV			11

/* Extended Conditions */

#define ECOND_NONE		  	0

#define ECOND_SKILL		  	1
#define ECOND_LEVEL		  	2
#define ECOND_RACE		  	3
#define ECOND_PROF		  	4
#define ECOND_LIGHT		  	5
#define ECOND_ALIGN		  	6
#define ECOND_MOON		  	7
#define ECOND_CASTING_LEVEL	  	8
#define ECOND_SIZE	  		9

#define ECOND_SEX	 	 	11
#define ECOND_CONV		 	12
#define ECOND_RANDOM		13
#define ECOND_CARRYING		14
#define ECOND_WEARING		15
#define ECOND_TYPE		 	16
#define ECOND_VNUM		 	17
#define ECOND_IN_ROOM		18
#define ECOND_DEED		 	19
#define ECOND_SUBTYPE		20
#define ECOND_IS		 	21
#define ECOND_IS_NOT		 	22
#define ECOND_GADGET		 	23
#define ECOND_AFFECTED		24
#define ECOND_NUMBER		25
#define ECOND_AM_ACTOR		26
#define ECOND_AM_VICTIM		27
#define ECOND_LOCAL		 	28
#define ECOND_NUMBER_MOD	 	29
#define ECOND_IN_SUBAREA	 	30
#define ECOND_VALUE		 	31
#define ECOND_CONTAINS		35
#define ECOND_QUEST		 	36
#define ECOND_ACTOR_GROUPED	37
#define ECOND_VICTIM_GROUPED	38
#define ECOND_SANITY		 	39
#define ECOND_MEMORY		40
#define ECOND_MEMORYV		41
#define ECOND_ACTOR_REMEMBERED	42
#define ECOND_VICTIM_REMEMBERED	43
#define ECOND_ACTOR_FRIEND	 	44
#define ECOND_ACTOR_FOE		45
#define ECOND_VICTIM_FRIEND	 	46
#define ECOND_VICTIM_FOE	 	47
#define ECOND_ACTOR_VISIBLE	 	48
#define ECOND_VICTIM_VISIBLE	 	49

#define ECOND_HITS		 	60
#define ECOND_MANA		 	61
#define ECOND_MOVE		 	62
#define ECOND_HITS_PERCENT	 	63
#define ECOND_MANA_PERCENT	64
#define ECOND_MOVE_PERCENT	 	65
#define ECOND_GOLD		 	66
#define ECOND_NAME		 	67
#define ECOND_CULT		 	68
#define ECOND_FAME		 	69
#define ECOND_IS_ALONE	 	70
#define ECOND_SHORT        	 	71
#define ECOND_PRACTICE	 	72
#define ECOND_TRAIN        	 	73
#define ECOND_START_ROOM   	 	74
#define ECOND_KNOWS_DISCIPLINE   	75
#define ECOND_ATTRIBUTE	  	76
#define ECOND_CARRYING_TYPE	77
#define ECOND_WEARING_TYPE		78
#define ECOND_USES_IMP	 	79
#define ECOND_PARTNER	 	80
#define ECOND_CRIMINAL_RATING 	81
#define ECOND_OWNER 			82
#define ECOND_DIVINITY		83

/* Societies */

#define ECOND_SOC_RANK		 	32
#define ECOND_SOC_LEVEL		 	33
#define ECOND_SOC_AUTH		 	34

#define ECOND_SOC_RANK_NONE	  	0
#define ECOND_SOC_RANK_INVITED	  	1
#define ECOND_SOC_RANK_MEMBER	  	2
#define ECOND_SOC_RANK_COUNCIL	  	4
#define ECOND_SOC_RANK_LEADER	  	8

/* Probes */

#define ECOND_MOB_IN_ROOM	 		50
#define ECOND_PLAYER_IN_ROOM	 	51
#define ECOND_ROOM_EMPTY_MOB	 	52
#define ECOND_OBJ_IN_ROOM	 		53
#define ECOND_ROOM_EMPTY_OBJ	 	54
#define ECOND_OBJ_ROOM_CONTAINER	 	55

/* Hour */

#define ECOND_HOUR_OF_DAY			100
#define ECOND_HOUR_OF_DAY_MOD		101

/* Day */

#define ECOND_DAY_OF_MONTH		112
#define ECOND_DAY_OF_MONTH_MOD		113
#define ECOND_DAY_OF_YEAR			114
#define ECOND_DAY_OF_YEAR_MOD		115

/* Week */

#define ECOND_WEEK_OF_YEAR			120
#define ECOND_WEEK_OF_YEAR_MOD		121

/* Month */

#define ECOND_MONTH_OF_YEAR		130
#define ECOND_MONTH_OF_YEAR_MOD		131

/* Affect/state conditions for IS/IS_NOT */

#define ECOND_IS_HUNGRY		1
#define ECOND_IS_FULL		   	2
#define ECOND_IS_THIRSTY	   	3
#define ECOND_IS_REFRESHED         	4
#define ECOND_IS_SOBER		5
#define ECOND_IS_DRUNK		6

#define ECOND_IS_PLAYER		9
#define ECOND_IS_STANDING	  	10
#define ECOND_IS_FIGHTING	  	11
#define ECOND_IS_SITTING	  	12
#define ECOND_IS_RESTING	  	13
#define ECOND_IS_AWAKE		14
#define ECOND_IS_SLEEPING         	15
#define ECOND_IS_STUNNED	  	16
#define ECOND_IS_DIEING           		17
#define ECOND_IS_DEAD		18
#define ECOND_IS_BLIND		19
#define ECOND_IS_INVISIBLE	  	20
#define ECOND_IS_DETECT_EVIL	  	21
#define ECOND_IS_DETECT_INVIS	22
#define ECOND_IS_DETECT_MAGIC     	23
#define ECOND_IS_DETECT_HIDDEN    	24
#define ECOND_IS_MELD		25
#define ECOND_IS_SANCTUARY	  	26
#define ECOND_IS_FAERIE_FIRE	  	27
#define ECOND_IS_INFRARED	  	28
#define ECOND_IS_CURSE		29
#define ECOND_IS_FEAR		  	30
#define ECOND_IS_POISON		31
#define ECOND_IS_PROT_GOOD	  	32
#define ECOND_IS_PROT_EVIL	  	33
#define ECOND_IS_SNEAK		34
#define ECOND_IS_HIDE		  	35
#define ECOND_IS_SLEEP		36
#define ECOND_IS_CHARM		37
#define ECOND_IS_FLYING		38
#define ECOND_IS_PASS_DOOR	  	39
#define ECOND_IS_HASTE		40
#define ECOND_IS_CALM		41
#define ECOND_IS_PLAGUE		42
#define ECOND_IS_WEAKEN		43
#define ECOND_IS_DARK_VISION      	44
#define ECOND_IS_BERSERK	  	45
#define ECOND_IS_SWIM		46
#define ECOND_IS_REGENERATION	47
#define ECOND_IS_POLY		  	48
#define ECOND_IS_ABSORB		49
#define ECOND_IS_FLEEING	  	50
#define ECOND_IS_DREAMING	  	51
#define ECOND_IS_STARVING	  	52
#define ECOND_IS_DEHYDRATED	53
#define ECOND_IS_OVERWEIGHT	  	54
#define ECOND_IS_DARKNESS		55
#define ECOND_IS_AURA		56
#define ECOND_IS_HALLUCINATING	57
#define ECOND_IS_RELAXED		58
#define ECOND_IS_FIRE_SHIELD		59
#define ECOND_IS_FROST_SHIELD	60
#define ECOND_IS_SLOW		61
#define ECOND_IS_GLOBE		62
#define ECOND_IS_CRIMINAL		63
#define ECOND_IS_MORF		64
#define ECOND_IS_INCARNATED	65
#define ECOND_IS_MIST                   	66
#define ECOND_IS_ELDER_SHIELD	67
#define ECOND_IS_FARSIGHT                   	68
#define ECOND_IS_WERE		69
#define ECOND_IS_VAMPIRE                   	70
#define ECOND_IS_UNDEAD                   	71
#define ECOND_IS_STRANGER                   	72
#define ECOND_IS_PASSPORT                   	73
#define ECOND_IS_ASCETICISM                  	74

/* Access functions... */

extern ECOND_DATA 	*get_ec			();
extern void 		free_ec			(ECOND_DATA *ec);
extern void 		free_ec_norecurse	(ECOND_DATA *ec);
extern bool 		ec_satisfied_mcc		(MOB_CMD_CONTEXT *mcc, ECOND_DATA *ec, bool recurse);
extern ECOND_DATA 	*read_econd		(FILE *fp);
extern void 		print_econd		(CHAR_DATA *ch, ECOND_DATA *ec, char *hdr);
extern void 		print_econd_to_buffer	(char *buffer, ECOND_DATA *ec, char *hdr,  MOB_CMD_CONTEXT *mcc);
extern ECOND_DATA 	*create_econd		(char *cond, CHAR_DATA *ch);
extern void 		write_econd		(ECOND_DATA *ec, FILE *fp, char *hdr);


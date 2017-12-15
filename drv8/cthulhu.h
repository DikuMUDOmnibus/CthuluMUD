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

#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun

/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if	!defined(FALSE)
#define FALSE	 false
#endif

#if	!defined(TRUE)
#define TRUE	 true
#endif

/*
 * Structure types.
 */

typedef struct	affect_data		AFFECT_DATA;
typedef struct	area_data		AREA_DATA;
typedef struct	ban_data		BAN_DATA;
typedef struct	char_data		CHAR_DATA;
typedef struct  	skill_data		SKILL_DATA;
typedef struct	descriptor_data		DESCRIPTOR_DATA;
typedef struct	exit_data		EXIT_DATA;
typedef struct	extra_descr_data		EXTRA_DESCR_DATA;
typedef struct	help_data		HELP_DATA;
typedef struct	kill_data			KILL_DATA;
typedef struct	mob_index_data		MOB_INDEX_DATA;
typedef struct	note_data		NOTE_DATA;
typedef struct	obj_data			OBJ_DATA;
typedef struct	obj_index_data		OBJ_INDEX_DATA;
typedef struct	pc_data			PC_DATA;
typedef struct  	gen_data		GEN_DATA;
typedef struct	reset_data		RESET_DATA;
typedef struct	room_index_data		ROOM_INDEX_DATA;
typedef struct	shop_data		SHOP_DATA;
typedef struct	time_info_data		TIME_INFO_DATA;
typedef struct	weather_data		WEATHER_DATA;
typedef struct	skill_info		SKILL_INFO;
typedef struct  	zone_data               	ZONE_DATA;
typedef struct  	mud_data                	MUD_DATA;
typedef struct  	auction_data            	AUCTION_DATA;

/*
 * Function types.
 */
typedef	void DO_FUN	args( ( CHAR_DATA *ch, char *argument ) );
typedef bool SPEC_FUN	args( ( CHAR_DATA *ch ) );
typedef void SPELL_FUN	args( ( int sn, int level, CHAR_DATA *ch, void *vo ) );



/*
 * String and memory management parameters.
 */
#define	MAX_KEY_HASH		1024
#define MAX_STRING_LENGTH	 	4096
#define MAX_INPUT_LENGTH	  	256
#define PAGELEN			20
/* pagelen lowered to lessen msgs from scrolling off unread data */


/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_QUOTES		   	19
#define MAX_SOCIALS		  	256
#define MAX_RUMOR		  	256
#define MAX_SKILL		  	512
#define MAX_RACE		   	64
#define MAX_CULT		   	16
#define MAX_CLASS		    	7

#define MAX_LEVEL		   	300
#define REMORT_LEVEL		   	150

#define PULSE_PER_SECOND	    	4
#define PULSE_MOB_FAST		(PULSE_PER_SECOND)
#define PULSE_VIOLENCE		(3 * PULSE_PER_SECOND)
#define PULSE_MOBILE		  	(4 * PULSE_PER_SECOND)
#define PULSE_TICK		  	(45 * PULSE_PER_SECOND) 
#define PULSE_TICKSHORT		(5 * PULSE_PER_SECOND) 
#define PULSE_AREA		  	(60 * PULSE_PER_SECOND)
#define PULSE_ENV                 		(10 * PULSE_PER_SECOND)
#define PULSE_AUCTION             		(20 * PULSE_PER_SECOND) 

#define IMPLEMENTOR			MAX_LEVEL
#define	CREATOR			(MAX_LEVEL - 1) 
#define SUPREME			(MAX_LEVEL - 2) 
#define DEITY				(MAX_LEVEL - 3) 
#define GOD				(MAX_LEVEL - 4) 
#define IMMORTAL			(MAX_LEVEL - 5) 
#define DEMI				(MAX_LEVEL - 6) 
#define ANGEL				(MAX_LEVEL - 7) 
#define AVATAR			(MAX_LEVEL - 8) 

#define DIV_NEWBIE		  0
#define DIV_HUMAN		  1
#define DIV_INVESTIGATOR	  2
#define DIV_HERO		  4
#define DIV_CREATOR		  8
#define DIV_LESSER		 16
#define DIV_GOD		 32
#define DIV_GREATER		 64
#define DIV_IMPLEMENTOR	128
#define DIV_MUD		256

#define NEWBIE_START_LEVEL	  	3
#define DIV_LEV_NEWBIE 	  	1
#define DIV_LEV_HUMAN             		15
#define DIV_LEV_INVESTIGATOR 	51
#define DIV_LEV_HERO			101

#define DIV_MASK                 (DIV_IMPLEMENTOR | DIV_GREATER | DIV_GOD | DIV_LESSER | DIV_CREATOR)   

/* Null file, held open to reserve a file handle... */

#define NULL_FILE		"/dev/null"

/* Area files... (R/W) */

#define AREA_DIR        	                "../area"
#define AREA_LIST		"../area/area.lst"

#define MOB_DIR		"MOBProgs/"

/* Configuration files... (R) */

#define CONFIG_DIR      	                "../config"
#define SKILL_FILE 		"../config/skills.txt"
#define SPELL_FILE 		"../config/spells.txt"
#define PROF_FILE       	                "../config/profs.txt"
#define SOC_FILE        	                "../config/social.txt" 
#define SOCIETY_FILE    	                "../config/society.txt"
#define SOCIETY2_FILE    	"../config/socrace.txt"
#define PROFILE_FILE    	                "../config/profile.txt"
#define RACE_FILE	    	"../config/races.txt"
#define CULT_FILE	    	"../config/cults.txt"
#define PARTNER_FILE	    	"../config/partners.txt"
#define RUMOR_FILE	    	"../config/rumors.txt"
#define CHAT_FILE                  	"../config/chat.txt"
#define EXE_FILE                   	"../drv8/cthulhu"

/* Runtime data files... (R/W) */

#define DATA_DIR 		"../data"
#define DISABLE_FILE 		"../data/disable.dat"
#define TIME_FILE 		"../data/time.dat"
#define BANK_FILE 		"../data/banks.dat"
#define BANK_TEMP    		"../data/banks.temp"
#define SHARES_FILE 	               	"../data/shares.dat"
#define SOCIETY_MEMBERS_FILE  "../data/socmemb.dat"
#define SHOP_FILE 	                "../data/shop.dat"
#define MORGUE_FILE 	                "../data/morgue.dat"
#define ARTIFACT_FILE 	                "../data/artifact.dat"
#define TREE_FILE 	                "../data/tree.dat"
#define BAN_FILE	    	"../data/ban.dat"
#define COPYOVER_FILE                  "../data/co.dat"
#define PASS_FILE                  	"../data/pass.dat"

/* Runtime logs... (W) */

#define LOG_DIR               		"../log"
#define BUG_FILE		"../log/bugs.txt"  
#define TYPO_FILE		"../log/typos.txt" 
#define SHUTDOWN_FILE	"../log/sdown.txt" 
#define LASTCOMMAND_FILE	"../log/last.txt" 

/* Runtime html files... (W) */

#define ONLINE_FILE		"../html/online.html" 
#define ONLINE_FILE_NEW	"../html/online.html~" 

#define GOD_DIR		"../gods"
#define MAIL_DIR        	                "../mail"

/* Player data... (R/W) */

#define PLAYER_DIR		"../player/"
#define PLAYER_TEMP		"../player/temp"
#define LOCKER_TEMP		"../player/temp_l"

#define COMPRESS_PFILES   /* gzip pfiles upon do_quit */

/* Various old Sunder/Merc files that will, eventually, be deleted... */

#define MAX_DIR 10
#define NO_FLAG -99     /* Must not be used in flags or stats. */
#define MAX_ADMIN 5

/* define DEBUGINFO for debugging features. Use only when other methods fail,
                    because all new code will be writting to send extra info when
		    this is on! */
/* #define DEBUGINFO */



#define CLEAR		"[0m"
#define FG_BLACK      	"\033[0;30m"
#define C_RED		"[0;31m"
#define C_GREEN	"[0;32m"     
#define C_YELLOW	"[0;33m"
#define C_BLUE		"[0;34m"
#define C_MAGENTA	"[0;35m"
#define C_CYAN		"[0;36m"
#define C_WHITE	"[0;37m"
#define C_D_GREY	"[1;30m"
#define C_B_RED	"[1;31m"
#define C_B_GREEN	"[1;32m"
#define C_B_YELLOW	"[1;33m"
#define C_B_BLUE	"[1;34m"
#define C_B_MAGENTA	"[1;35m"
#define C_B_CYAN	"[1;36m"
#define C_B_WHITE	"[1;37m"

#define MOD_UNDERLINE 	"\033[4m"
#define MOD_BLINK     		"\033[5m"
#define MOD_REVERSE   		"\033[7m"

#define VT_CLS	    		"\033[2J"
#define VT_SAVEC      		"\033[s"
#define VT_RESTOREC  		"\033[u"
#define VT_CLINE   		"\033[K"

#define IMP_CLS	    		"<CT>"


#define MAX_BOARD 	  10

struct board_data
{
	char *short_name; /* Max 8 chars */
	char *long_name;  /* Explanatory text, should be no more than 40 ? chars */
	
	int read_div; /* minimum divinity to see board */
	int write_div; /* minimum divinity to post notes */

	char *names;       /* Default recipient */
	int force_type; /* Default action (DEF_XXX) */
	
	int purge_days; /* Default expiration */

	/* Non-constant data */
		
	NOTE_DATA *note_first; /* pointer to board's first note */
	bool changed; /* currently unused */
                bool true_board;		
};

typedef struct board_data BOARD_DATA;

/*
 * Site ban structure.
 */
struct	ban_data
{
    BAN_DATA *	next;
    char *	name;
    int	type;
};

#define BAN_NORMAL	0
#define BAN_PREFIX 	1
#define BAN_SUFFIX	2


/*
 * Time and weather stuff.
 */
#define SUN_DARK		0
#define SUN_RISE		1
#define SUN_LIGHT		2
#define SUN_SET		3

#define SUN_MAX		4 

#define SKY_CLOUDLESS	0
#define SKY_CLOUDY		1
#define SKY_RAINING		2
#define SKY_LIGHTNING		3

#define SKY_MAX		4

#define WEATHER_NONE		0
#define WEATHER_SUNNY_DAY		1
#define WEATHER_CLEAR_NIGHT	2
#define WEATHER_CLOUDY		3
#define WEATHER_RAINING		4
#define WEATHER_STORM		5
#define WEATHER_SNOWING		6
#define WEATHER_BLIZZARD		7

#define WEATHER_MAX			8

#define SURFACE_CLEAR		0
#define SURFACE_PUDDLES		1
#define SURFACE_SNOW		2
#define SURFACE_DEEP_SNOW		3
#define SURFACE_FROZEN		4
#define SURFACE_LEAVES		5

#define SURFACE_MAX			6

#define CLIMATE_EUROPEAN		0
#define CLIMATE_DESERT		1
#define CLIMATE_RAINY		2
#define CLIMATE_ARCTIC		3

#define CLIMATE_MAX			4

#define SEASON_SPRING			0
#define SEASON_SUMMER		1
#define SEASON_FALL			2
#define SEASON_WINTER		3

#define SEASON_MAX			4


/* Moon states */

#define MOON_NEW		0
#define MOON_CRESCENT	1
#define MOON_HALF		2
#define MOON_3Q		3
#define MOON_FULL		4

#define MOON_MAX		5

/* Calander */

#define DAYS_IN_WEEK		 8
#define DAYS_IN_MONTH	32
#define MONTHS_IN_YEAR	12
#define SEASONS_IN_YEAR	 4

#define TIME_NIGHT		 1
#define TIME_DAY		 2
#define TIME_DAWN		 4
#define TIME_DUSK		 8

struct	time_info_data
{
    int		minute;   
    int		hour;
    int		day;
    int		week;
    int		month;
    int		season;	
    int		year;
    int 	                moon;
    int		flags;
};

struct	weather_data
{
    int		sky;
    int		sunlight;
    int		moon;
    int		natural_light;
    int		duration;
};


struct alias_data
{
	char *name;
	char *command_string;
};

#define MAX_ALIAS 20
#define MAX_ALIAS_LENGTH 160 
#define MAX_ALIAS_PARMS 10


struct material_data
{
	char *name;
	short type;
	long vuln_flag;
};

struct wizcommand_type
{
	char 	*name;
	short 	auth;
	struct wizcommand_type *next;
};
 
struct  disable_cmd_type
{
    char * name;
    int    div;
    struct disable_cmd_type * next;
    DO_FUN *disable_fcn;
};

extern  struct disable_cmd_type  * disable_cmd_list;


/*
 * Structure for a command in the command lookup table.
 */
struct  cmd_type
{
    char * const    	name;
    DO_FUN *        do_fun;
    short      	position;
    short      	div;
    short      	log;
    bool        	show;
    short		cclass;
    bool       	disabled;
    int         	disabled_div;
};


/*
 * Connected state for a channel.
 */
#define CON_COPYOVER_RECOVER		-15
#define CON_PLAYING			 	0
#define CON_GET_NAME			1
#define CON_GET_OLD_PASSWORD		2
#define CON_CONFIRM_NEW_NAME		3
#define CON_GET_NEW_PASSWORD		4
#define CON_CONFIRM_NEW_PASSWORD	5
#define CON_GET_NEW_RACE		 	6
#define CON_GET_NEW_SEX			7
#define CON_GET_NEW_CLASS		 	8
#define CON_GET_ALIGNMENT		 	9
#define CON_CHANGE_NAME   			10
#define CON_CHOOSE_TERM			11
#define CON_GET_AMATEUR			12
#define CON_READ_IMOTD			13
#define CON_READ_MOTD			14
#define CON_BREAK_CONNECT			15
#define CON_NOTE_TO                     		16
#define CON_NOTE_SUBJECT                		17
#define CON_NOTE_EXPIRE                 		18
#define CON_NOTE_TEXT                   		19
#define CON_NOTE_FINISH                 		20
#define CON_GET_PROFESSION			21
#define CON_WAITING_NAME			22
#define CON_TRANSFER				23
#define CON_CHECK_APPROVAL			24
#define CON_NOTE_BOARD			25
#define CON_CONFIRM_SELL			26
#define CON_CONFIRM_BUY			27
#define CON_WAITING_NAME2			28
#define CON_CHATTING				29
#define CON_MENU				30


/*
 * Descriptor (channel) structure.
 */
struct	descriptor_data
{
    DESCRIPTOR_DATA *	next;
    DESCRIPTOR_DATA *	snoop_by;
    CHAR_DATA *		character;
    CHAR_DATA *		original;
    char *		host;
    short		descriptor;
    short		connected;
    short		connected_old;
    bool		fcommand;
    char		inbuf		[4 * MAX_INPUT_LENGTH];
    char		incomm		[MAX_INPUT_LENGTH];
    char		inlast		[MAX_INPUT_LENGTH];
    int		repeat;
    int		outsize;
    int		outtop;
    char *		outbuf;
    char *		afk_outbuf;
    bool		multi_comm;
    char *		showstr_head;
    char *		showstr_point;
    void *              	pEdit;
    char **             	pString;
    short                 editor;  
    bool		pueblo;
    bool		ansi;     
    bool		ok;
    bool		imp_html;
};


/*
 * Attribute bonus structures.
 */
struct	str_app_type
{
    short	tohit;
    short	todam;
    short	carry;
    short	wield;
};

struct	int_app_type
{
    short	learn;
};

struct	wis_app_type
{
    short	practice;
};

struct	dex_app_type
{
    short	defensive;
};

struct	con_app_type
{
    short	hitp;
    short	shock;
};

typedef struct  mob_prog_data           MPROG_DATA;
typedef struct  mob_prog_act_list       MPROG_ACT_LIST;


/*
 * TO types for act.
 */
#define TO_ROOM		    0
#define TO_NOTVICT	    1
#define TO_VICT		    2
#define TO_CHAR		    3



/*
 * Help table types.
 */
struct	help_data
{
    HELP_DATA *	next;
    short	level;
    char *	keyword;
    char *	text;
};


/*
 * Per-class stuff.
 */

#define CLASS_MAGE		0
#define CLASS_GOOD_PRIEST	1
#define CLASS_THIEF		2
#define CLASS_WARRIOR	 3
#define CLASS_CHAOSMAGE 	4
#define CLASS_MONK		5
#define	CLASS_EVIL_PRIEST	6

#define MAX_STATS 	7

#define STAT_NONE	-1
#define STAT_STR 	0
#define STAT_INT	1
#define STAT_WIS	2
#define STAT_DEX	3
#define STAT_CON	4
#define STAT_LUC	5
#define STAT_CHA	6
#define STAT_HIGH	6

#define ABS_STAT_MAXIMUM	111
#define STAT_MAXIMUM	103
#define STAT_MINIMUM	12

#define TRAIN_STAT_STR	0
#define TRAIN_STAT_INT	1
#define TRAIN_STAT_WIS	2
#define TRAIN_STAT_DEX	3
#define TRAIN_STAT_CON	4
#define TRAIN_HITS		5
#define TRAIN_MANA		6
#define TRAIN_MOVE		7
#define TRAIN_PRACTICE	8
#define TRAIN_SANITY		9
#define TRAIN_STAT_CHA	10

#define TRAIN_MAX		11


struct	class_type
{
    char *	name;			/* the full name of the class */
    char 	who_name	[4];	/* Three-letter name for 'who'	*/
};

struct attack_type
{
    char *	name;			/* name and message */
    short   	damage;			/* damage class */
};



/*
 * Data structure for notes.
 */
struct	note_data
{
    NOTE_DATA *	next;
    short			to_partner;
    char *			sender;
    char *			date;
    char *			to_list;
    char *			subject;
    char *			text;
    time_t  		date_stamp;
    time_t			expire;
};


/*
 * An affect.
 */
struct	affect_data
{
    AFFECT_DATA *	next;
    short		type;
    short		level;
    short		duration;
    short		location;
    short		modifier;
    short		skill;
    short 	afn;
    short 	where;
    long long	bitvector;
    char *		caster;
};



/*
 * A kill structure (indexed by level).
 */
struct	kill_data
{
    short		number;
    short		killed;
};

/* Old mobprogs... */

struct  mob_prog_act_list
{
    MPROG_ACT_LIST * next;
    char *           buf;
    CHAR_DATA *      ch;
    OBJ_DATA *       obj;
    void *           vo;
};

struct  mob_prog_data
{
    MPROG_DATA *next;
    int         type;
    char *      arglist;
    char *      comlist;
};

extern bool    MOBtrigger;

#define ERROR_PROG        -1
#define IN_FILE_PROG       0
#define ACT_PROG           1
#define SPEECH_PROG        2
#define RAND_PROG          4
#define FIGHT_PROG         8
#define DEATH_PROG        16
#define HITPRCNT_PROG     32
#define ENTRY_PROG        64
#define GREET_PROG       128
#define ALL_GREET_PROG   256
#define GIVE_PROG        512
#define BRIBE_PROG      1024
 


/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_ZOMBIE            	2
#define MOB_VNUM_MUMMY             	3
#define MOB_VNUM_MIRROR            	4
#define MOB_VNUM_ASTRAL            	5
#define MOB_VNUM_ADMIN            	18

#define MOB_VNUM_CITYGUARD	   3060

#define MOB_VNUM_PATROLMAN	   2106
#define GROUP_VNUM_TROLLS	   2100
#define GROUP_VNUM_OGRES	   2101

/* VNUM ranges for system and limbo areas... */

#define VNUM_LIMBO_LOW			    0
#define VNUM_LIMBO_HIGH		 	   65
#define VNUM_SYSTEM_LOW			20000
#define VNUM_SYSTEM_HIGH		                20099
#define VNUM_NIGHTMARE_LOW	                20150
#define VNUM_NIGHTMARE_HIGH                             20199

/* RT ASCII conversions -- used so we can have letters in this file */

#define A			            0x0001
#define B			            0x0002
#define C			            0x0004
#define D			            0x0008

#define E			            0x0010
#define F			            0x0020
#define G			            0x0040
#define H			            0x0080

#define I		 	            	            0x0100
#define J			            0x0200
#define K		                            0x0400
#define L		 	            0x0800

#define M			            0x1000
#define N		 	            0x2000
#define O			            0x4000  // max for signed int
#define P			            0x8000  // max for unsigned int

#define Q			        0x00010000
#define R			        0x00020000
#define S			        0x00040000
#define T			        0x00080000

#define U		 	        0x00100000
#define V			        0x00200000
#define W		  	        0x00400000
#define X			        0x00800000

#define Y			        0x01000000
#define Z			        0x02000000
#define aa			        0x04000000
#define bb			        0x08000000

#define cc			        0x10000000  
#define dd			        0x20000000
#define ee			        0x40000000  // max for signed long
#define ff			        0x80000000  // max for unsigned long

#define gg			0x0000000100000000LL
#define hh			0x0000000200000000LL
#define ii			0x0000000400000000LL
#define jj			0x0000000800000000LL
#define kk			0x0000001000000000LL
#define ll			0x0000002000000000LL
#define mm			0x0000004000000000LL
#define nn			0x0000008000000000LL

#define oo			0x0000010000000000LL
#define pp			0x0000020000000000LL
#define qq			0x0000040000000000LL
#define rr			0x0000080000000000LL

#define ssx			0x0000100000000000LL  // Something in sigcontext
#define tt			0x0000200000000000LL
#define uu			0x0000400000000000LL
#define vv			0x0000800000000000LL

#define ww			0x0001000000000000LL
#define xx			0x0002000000000000LL
#define yy                                             0x0004000000000000LL
#define zz			0x0008000000000000LL

#define aaa			0x0010000000000000LL
#define bbb			0x0020000000000000LL
#define ccc			0x0040000000000000LL
#define ddd			0x0080000000000000LL

#define eee			0x0100000000000000LL
#define fff			0x0200000000000000LL
#define ggg			0x0400000000000000LL
#define hhh			0x0800000000000000LL

#define iii			0x1000000000000000LL
#define jjj			0x2000000000000000LL
#define kkk			0x4000000000000000LL  // max for signed long long
#define lll			0x8000000000000000LL

/* Light levels */

#define LIGHT_NONE		  0
#define LIGHT_UNDERGROUND	 10
#define LIGHT_INDOORS		 20
#define LIGHT_NIGHT		 30
#define LIGHT_MOON_DARK	 40

#define LIGHT_MOON_HALF	 50
#define LIGHT_MOON_FULL	 60
#define LIGHT_DAWN		 65
#define LIGHT_DUSK		 70

#define LIGHT_ARTIFICIAL	 80
#define LIGHT_MAGICAL	 85
#define LIGHT_DAY		 90

#define LIGHT_SUNLIGHT	100


/*Kung Fu */
#define STYLE_IRON_FIST		0
#define STYLE_TIGER		 	1
#define STYLE_CRANE		 	2
#define STYLE_SNAKE		 	3
#define STYLE_DRUNKEN_BUDDHA	4
#define STYLE_DRAGON		 	5
#define STYLE_NINJITSU		 	6
#define STYLE_TOUCH		 	7


#define MATERIAL_WOOD  		1
#define MATERIAL_IRON                   	2
#define MATERIAL_SILVER                 	3
#define MATERIAL_GOLD                   	4
#define MATERIAL_ADAMANTITE            5
#define MATERIAL_CLOTH  		6	
#define MATERIAL_GLASS  		7		
#define MATERIAL_LIQUID  		8
#define MATERIAL_FOOD  		9	
#define MATERIAL_STEEL  		10		
#define MATERIAL_MITHRIL  		11	
#define MATERIAL_PAPER  		12		
#define MATERIAL_MEAT  		13	
#define MATERIAL_FLESH  		14		
#define MATERIAL_LEATHER  		15		
#define MATERIAL_PILL  		16	
#define MATERIAL_VELLUM  		17		
#define MATERIAL_BRONZE  		18	
#define MATERIAL_BRASS  		19
#define MATERIAL_STONE  		20
#define MATERIAL_BONE	  	21
#define MATERIAL_UNIQUE  		22	
#define MATERIAL_CRYSTAL		23
#define MATERIAL_DIAMOND		24
#define MATERIAL_FUR                                 25
#define MATERIAL_PLASTIC		26
#define MATERIAL_MARBLE		27
#define MATERIAL_GRANITE		28
#define MATERIAL_RUBBER		29
#define MATERIAL_IVORY		30
#define MATERIAL_EARTH		31
#define MATERIAL_AIR			32
#define MATERIAL_FIRE			33
#define MATERIAL_WATER		34
#define MATERIAL_SANDSTONE		35
#define MATERIAL_LIMESTONE		36
#define MATERIAL_CHALK		37
#define MATERIAL_HAIR		38   
#define MATERIAL_SKIN		39            
#define MATERIAL_SILK			40
#define MATERIAL_REPTILE_LEATHER	41
#define MATERIAL_CHITIN		42
#define MATERIAL_LEATHERY_SKIN	43
#define MATERIAL_FEATHERS		44
#define MATERIAL_PROTOPLASMA	45
#define MATERIAL_SCALES          	46
#define MATERIAL_PLANT                  	47
#define MATERIAL_PEARL                 	48
#define MATERIAL_TIME                   	49
#define MATERIAL_ALUMINIUM          	50
#define MATERIAL_BRICK               	51
#define MATERIAL_COPPER               	52

#define MAX_MATERIAL		52



#define CURRENCY_DEFAULT                   	0
#define CURRENCY_DOLLAR        		1
#define CURRENCY_GOLD         		2
#define CURRENCY_CROWN         		3
#define CURRENCY_EMPTY           		4

#define MAX_CURRENCY		5



/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC			(A)
#define ACT_SENTINEL	 	   	(B)
#define ACT_SCAVENGER	      	(C)
#define ACT_AUTOHOME 	      	(D)
#define ACT_NIGHT			(E) 
#define ACT_AGGRESSIVE		(F) 
#define ACT_STAY_AREA		(G)
#define ACT_WIMPY			(H)
#define ACT_PET			(I)
#define ACT_FOLLOWER			(J)
#define ACT_PRACTICE			(K)
#define ACT_VAMPIRE			(L)
#define ACT_BOUND			(M)
#define ACT_WERE			(N) 
#define ACT_UNDEAD			(O)	
#define ACT_MOUNT			(P)	
#define ACT_CLERIC			(Q)
#define ACT_MAGE			(R)
#define ACT_THIEF			(S)
#define ACT_WARRIOR			(T)
#define ACT_NOALIGN			(U)
#define ACT_NOPURGE			(V)
#define ACT_DREAM_NATIVE		(W)
#define ACT_BRAINSUCKED		(X)
#define ACT_CRIMINAL  	     	(Y)
#define ACT_IS_ALIENIST		(Z)
#define ACT_IS_HEALER			(aa)
#define ACT_TELEPOP			(bb)    
#define ACT_UPDATE_ALWAYS		(cc)
#define ACT_RAND_KIT           		(dd)         
#define ACT_STAY_SUBAREA       	(ee)  
#define ACT_INVADER             		(ff)            
#define ACT_WATCHER 		       	(gg)            
#define ACT_PROTECTED	       	(hh)            
#define ACT_MARTIAL	       		(ii)            


/* damage classes */
#define DAM_NONE                	0
#define DAM_BASH                	1
#define DAM_PIERCE              	2
#define DAM_SLASH               	3
#define DAM_FIRE                	4
#define DAM_COLD                	5
#define DAM_LIGHTNING           	6
#define DAM_ACID                	7
#define DAM_POISON              	8
#define DAM_NEGATIVE            	9
#define DAM_HOLY                	10
#define DAM_ENERGY              	11
#define DAM_MENTAL              	12
#define DAM_DISEASE             	13
#define DAM_DROWNING            	14
#define DAM_LIGHT		15
#define DAM_OTHER               	16
#define DAM_HARM		17
#define DAM_SHOT                	18
#define DAM_OLD                	19
#define DAM_SOUND                	20
#define MAX_DAM		21

/* OFF bits for mobiles */
#define OFF_AREA_ATTACK        (A)
#define OFF_BACKSTAB            	(B)
#define OFF_BASH                	(C)
#define OFF_BERSERK             	(D)
#define OFF_DISARM              	(E)
#define OFF_DODGE               	(F)
#define OFF_FADE                	(G)
#define OFF_FAST                	(H)
#define OFF_KICK                	(I)
#define OFF_KICK_DIRT           	(J)
#define OFF_PARRY               	(K)
#define OFF_RESCUE              	(L)
#define OFF_TAIL                	(M)
#define OFF_TRIP                	(N)
#define OFF_CRUSH		(O)
#define ASSIST_ALL       		(P)
#define ASSIST_ALIGN	        	(Q)
#define ASSIST_RACE    	     	(R)
#define ASSIST_PLAYERS      	(S)
#define ASSIST_GUARD        	(T)
#define ASSIST_VNUM		(U)
#define OFF_STUN		(V)
#define OFF_DISTRACT		(W)
#define OFF_MERCY		(X)

/* return values for check_imm */
#define IS_NORMAL		0
#define IS_IMMUNE		1
#define IS_RESISTANT		2
#define IS_VULNERABLE		3
#define IS_ENV_IMMUNE	4
#define IS_RESISTANT_PLUS	5


/* IMM bits for mobs */
#define IMM_SUMMON              	(A)
#define IMM_CHARM               	(B)
#define IMM_MAGIC               	(C)
#define IMM_WEAPON              	(D)
#define IMM_BASH                	(E)
#define IMM_PIERCE              	(F)
#define IMM_SLASH               	(G)
#define IMM_FIRE                	(H)
#define IMM_COLD                	(I)
#define IMM_LIGHTNING           	(J)
#define IMM_ACID                	(K)
#define IMM_POISON              	(L)
#define IMM_NEGATIVE            	(M)
#define IMM_HOLY                	(N)
#define IMM_ENERGY              	(O)
#define IMM_MENTAL              	(P)
#define IMM_DISEASE             	(Q)
#define IMM_DROWNING            	(R)
#define IMM_LIGHT		(S)
#define IMM_BULLETS             	(T)
#define IMM_MASK                 	(U)
#define IMM_OLD                 	(V)
#define IMM_SOUND                	(W)

#define IMMAPP_SUMMON           1
#define IMMAPP_CHARM              	2
#define IMMAPP_MAGIC                3
#define IMMAPP_WEAPON            4
#define IMMAPP_BASH                   5
#define IMMAPP_PIERCE                	6
#define IMMAPP_SLASH                	7
#define IMMAPP_FIRE                     	8
#define IMMAPP_COLD                   9
#define IMMAPP_LIGHTNING        10
#define IMMAPP_ACID                    11
#define IMMAPP_POISON               12
#define IMMAPP_NEGATIVE          13
#define IMMAPP_HOLY                   14
#define IMMAPP_ENERGY              	15
#define IMMAPP_MENTAL            	16
#define IMMAPP_DISEASE             17
#define IMMAPP_DROWNING       18
#define IMMAPP_LIGHT		19
#define IMMAPP_BULLETS            	20
#define IMMAPP_MASK                 	21
#define IMMAPP_OLD	                22
#define IMMAPP_SOUND                23
 
/* RES bits for mobs */
#define RES_CHARM		(B)
#define RES_MAGIC               	(C)
#define RES_WEAPON              	(D)
#define RES_BASH                	(E)
#define RES_PIERCE              	(F)
#define RES_SLASH               	(G)
#define RES_FIRE                	(H)
#define RES_COLD                	(I)
#define RES_LIGHTNING           	(J)
#define RES_ACID                	(K)
#define RES_POISON              	(L)
#define RES_NEGATIVE            	(M)
#define RES_HOLY                	(N)
#define RES_ENERGY              	(O)
#define RES_MENTAL              	(P)
#define RES_DISEASE             	(Q)
#define RES_DROWNING            	(R)
#define RES_LIGHT		(S)
#define RES_BULLETS             	(T)
#define RES_MASK		(U)
#define RES_OLD		(V)
#define RES_SOUND		(W)
 
/* VULN bits for mobs */
#define VULN_MAGIC              	(C)
#define VULN_WEAPON             	(D)
#define VULN_BASH               	(E)
#define VULN_PIERCE             	(F)
#define VULN_SLASH              	(G)
#define VULN_FIRE               	(H)
#define VULN_COLD               	(I)
#define VULN_LIGHTNING          	(J)
#define VULN_ACID               	(K)
#define VULN_POISON             	(L)
#define VULN_NEGATIVE           	(M)
#define VULN_HOLY               	(N)
#define VULN_ENERGY             	(O)
#define VULN_MENTAL             	(P)
#define VULN_DISEASE            	(Q)
#define VULN_DROWNING           	(R)
#define VULN_LIGHT		(S)
#define VULN_BULLETS            	(T)
#define VULN_OLD		(V)
#define VULN_SOUND		(W)

#define VULN_WOOD               	(X)
#define VULN_SILVER             	(Y)
#define VULN_IRON		(Z)
#define VULN_MITHRIL		(aa)
#define VULN_ADAMANTITE 	(bb)
#define VULN_STEEL		(cc)
#define VULN_ALUMINIUM	(dd)
#define VULN_COPPER         	(ee)

/* body form */
#define FORM_EDIBLE             	(A)
#define FORM_POISON             	(B)
#define FORM_MAGICAL         	(C)
#define FORM_INSTANT_DECAY (D)
#define FORM_OTHER               	(E)  
#define FORM_BLEEDING	(F) 
#define FORM_ANIMAL             	(G)
#define FORM_SENTIENT           	(H)
#define FORM_UNDEAD             	(I)
#define FORM_CONSTRUCT          	(J)
#define FORM_MIST               	(K)
#define FORM_INTANGIBLE         	(L)
#define FORM_BIPED              	(M)
#define FORM_CENTAUR            	(N)
#define FORM_INSECT             	(O)
#define FORM_SPIDER             	(P)
#define FORM_CRUSTACEAN       	(Q)
#define FORM_WORM               	(R)
#define FORM_BLOB		(S)
#define FORM_PLANT		(T)
#define FORM_NOWEAPON	(U)
#define FORM_MAMMAL             	(V)
#define FORM_BIRD               	(W)
#define FORM_REPTILE            	(X)
#define FORM_SNAKE              	(Y)
#define FORM_DRAGON             	(Z)
#define FORM_AMPHIBIAN          	(aa)
#define FORM_FISH               	(bb)
#define FORM_COLD_BLOOD	(cc)	
#define FORM_MACHINE	(dd)	
 
/* body parts */
#define PART_HEAD               	(A)
#define PART_ARMS               	(B)
#define PART_LEGS               	(C)
#define PART_HEART              	(D)
#define PART_BRAINS             	(E)
#define PART_GUTS               	(F)
#define PART_HANDS              	(G)
#define PART_FEET               	(H)
#define PART_FINGERS            	(I)
#define PART_EAR                	(J)
#define PART_EYE		(K)
#define PART_LONG_TONGUE      	(L)
#define PART_EYESTALKS          	(M)
#define PART_TENTACLES          	(N)
#define PART_FINS               	(O)
#define PART_WINGS              	(P)
#define PART_TAIL               	(Q)
#define PART_LEAVES               	(R)

#define PART_CLAWS              	(U)
#define PART_FANGS              	(V)
#define PART_HORNS              	(W)
#define PART_SCALES             	(X)
#define PART_TUSKS		(Y)
#define PART_HOOFS		(Z)
#define PART_LONG_TAIL          	(aa)
#define PART_STINGER            	(bb)


/*
 * Bits for 'affected_by'.
 * Used in #MOBILES (and other places).
 * Stored in a 'long long' - 63 bits plus sign.
 */
#define AFF_WATER_BREATHING	(A)
#define AFF_INVISIBLE			(B)
#define AFF_DETECT_EVIL		(C)
#define AFF_DETECT_INVIS		(D)
#define AFF_DETECT_MAGIC		(E)
#define AFF_DETECT_HIDDEN		(F)
#define AFF_MELD			(G)
#define AFF_SANCTUARY		(H)
#define AFF_FAERIE_FIRE		(I)
#define AFF_INFRARED			(J)
#define AFF_CURSE			(K)
#define AFF_FEAR			(L)
#define AFF_POISON			(M)
#define AFF_PROTECTG			(N) 
#define AFF_PROTECTE			(O) 	
#define AFF_SNEAK			(P)
#define AFF_HIDE			(Q)
#define AFF_SLEEP			(R)
#define AFF_CHARM			(S)
#define AFF_FLYING			(T)
#define AFF_PASS_DOOR		(U)
#define AFF_HASTE			(V)
#define AFF_CALM			(W)
#define AFF_PLAGUE			(X)
#define AFF_WEAKEN			(Y)
#define AFF_DARK_VISION		(Z)
#define AFF_BERSERK			(aa)
#define AFF_SWIM			(bb)
#define AFF_REGENERATION		(cc)
#define AFF_POLY			(dd)
#define AFF_ABSORB			(ee)  
#define AFF_BLIND			(ff)
#define AFF_DARKNESS			(gg)
#define AFF_AURA			(hh)
#define AFF_HALLUCINATING		(ii)
#define AFF_RELAXED			(jj)
#define AFF_FIRE_SHIELD		(kk)
#define AFF_FROST_SHIELD		(ll)
#define AFF_LIGHTNING_SHIELD		(mm)
#define AFF_SLOW			(nn)
#define AFF_GLOBE			(oo)
#define AFF_MORF			(pp)
#define AFF_INCARNATED		(qq)
#define AFF_MIST			(rr)
#define AFF_ELDER_SHIELD		(ssx)
#define AFF_FARSIGHT			(tt)
#define AFF_ASCETICISM		(uu)


/*
 * Bits for 'affected_by'.
 * Used in #ROOMS.
 */
#define RAFF_HOLY			(A)
#define RAFF_EVIL			(B)
#define RAFF_SILENCE			(C)
#define RAFF_DARKNESS		(D)
#define RAFF_WILD_MAGIC		(E)
#define RAFF_LOW_MAGIC		(F)
#define RAFF_HIGH_MAGIC		(G)
#define RAFF_NOBREATHE		(H)
#define RAFF_DRAIN			(I)
#define RAFF_MARE			(J)
#define RAFF_ENCLOSED		(K)
#define RAFF_NOMORGUE		(L)
#define RAFF_FATIGUE			(M)
#define RAFF_DESTRUCTIVE		(N)


/*
 * Artifact bits
 */
#define ART_MIST			(A)
#define ART_EIBON			(B)
#define ART_NECRONOMICON		(C)
#define ART_BLESSING_NEPHRAN	(D)
#define ART_ABHOTH			(E)


/*
 * Sex.
 * Used in #MOBILES.
 */
#define SEX_NEUTRAL		      0
#define SEX_MALE		      1
#define SEX_FEMALE		      2
#define SEX_RANDOM		      3

/* AC types */
#define AC_PIERCE			0
#define AC_BASH			1
#define AC_SLASH			2
#define AC_EXOTIC			3

/* dice */
#define DICE_NUMBER			0
#define DICE_TYPE			1
#define DICE_BONUS			2

/* size */
#define SIZE_TINY			0
#define SIZE_SMALL			1
#define SIZE_MEDIUM			2
#define SIZE_LARGE			3
#define SIZE_HUGE			4
#define SIZE_GIANT			5
#define SIZE_UNKNOWN        		6

#define SIZE_MAXI			6

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_DUMMY  		1
#define OBJ_VNUM_MONEY_ONE	2
#define OBJ_VNUM_MONEY_SOME	3
#define OBJ_VNUM_MASTER_KEY	5
#define OBJ_VNUM_HAIR                       	6
#define OBJ_VNUM_VOODOO_DOLL          	7
#define OBJ_VNUM_WRATH                      	8
#define OBJ_VNUM_AMMO                       	9
#define OBJ_VNUM_CORPSE_NPC	10
#define OBJ_VNUM_CORPSE_PC		11
#define OBJ_VNUM_SEVERED_HEAD	12
#define OBJ_VNUM_TORN_HEART	13
#define OBJ_VNUM_SLICED_ARM	14
#define OBJ_VNUM_SLICED_LEG		15
#define OBJ_VNUM_GUTS		16
#define OBJ_VNUM_BRAINS		17
#define OBJ_VNUM_SOULBLADE                 18
#define OBJ_VNUM_TRAP                      	19
#define OBJ_VNUM_MUSHROOM	20
#define OBJ_VNUM_LIGHT_BALL	21
#define OBJ_VNUM_SPRING		22
#define OBJ_POTION		    	23
#define OBJ_SCROLL			24
#define OBJ_VNUM_SEED		31

#define OBJ_PASSPORT			47
#define OBJ_SHARE			48
#define OBJ_CAMP			50

#define OBJ_FIGURINE			52
#define OBJ_VNUM_GUNPOWDER	53
#define OBJ_VNUM_BLOOD		54
#define OBJ_VNUM_PORTAL		55
#define OBJ_VNUM_SCHOOL_MACE	56
#define OBJ_VNUM_SCHOOL_DAGGER	57
#define OBJ_VNUM_SCHOOL_SWORD	58
#define OBJ_VNUM_SCHOOL_VEST	59
#define OBJ_VNUM_SCHOOL_SHIELD	60
#define OBJ_VNUM_SCHOOL_BANNER	61
#define OBJ_VNUM_MAP		62
#define OBJ_VNUM_BOAT		63 
#define OBJ_VNUM_BAG		          	64 
#define OBJ_VNUM_PIT			65

#define OBJ_VNUM_WHISTLE		 2116

#define OBJ_VNUM_WEDDING_RING		20011
#define OBJ_VNUM_WEDDING_BAND		20012
#define OBJ_VNUM_ORE_LOW			20070
#define OBJ_VNUM_QUEST_DOCUMENT		20059


#define MOB_VNUM_ASSASSIN			20023
#define MOB_VNUM_BODYGUARD		20024
#define MOB_VNUM_THIEF			20025

/*
 * Disciplines of Magic.
 */
#define DISCIPLINE_FIRE				(A)
#define DISCIPLINE_WATER			(B)
#define DISCIPLINE_AIR				(C)
#define DISCIPLINE_EARTH			(D)
#define DISCIPLINE_ENCHANTMENT		(E)
#define DISCIPLINE_LIFE				(F)
#define DISCIPLINE_NECROMANCY         		(G)
#define DISCIPLINE_CURSES			(H)
#define DISCIPLINE_ELDER_MAGIC		(I)
#define DISCIPLINE_DEMONOLOGY		(J)
#define DISCIPLINE_COMBAT			(K)
#define DISCIPLINE_KNOWLEDGE		(L)
#define DISCIPLINE_PROTECTION		(M)
#define DISCIPLINE_LUNAR			(N)
#define DISCIPLINE_TRANSDIMENSIONAL	(O)
#define DISCIPLINE_POWER			(P)

/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT		1
#define ITEM_SCROLL		2
#define ITEM_WAND		3
#define ITEM_STAFF		4
#define ITEM_WEAPON		5
#define ITEM_CAMERA		6
#define ITEM_PHOTOGRAPH         7
#define ITEM_TREASURE	8
#define ITEM_ARMOR		9
#define ITEM_POTION		10
#define ITEM_CLOTHING		11
#define ITEM_FURNITURE	12
#define ITEM_TRASH		13
#define ITEM_POOL		14
#define ITEM_GRIMOIRE		15
#define ITEM_CONTAINER	16
#define ITEM_DRINK_CON	17
#define ITEM_KEY		18
#define ITEM_FOOD		19
#define ITEM_MONEY		20
#define ITEM_LIGHT_REFILL	21
#define ITEM_BOAT		22
#define ITEM_CORPSE_NPC	23
#define ITEM_CORPSE_PC	24
#define ITEM_FOUNTAIN	25
#define ITEM_PILL		26
#define ITEM_PROTECT		27
#define ITEM_MAP		28
#define ITEM_PRIDE                   	29
#define ITEM_COMPONENT	30
#define ITEM_PORTAL		31
#define ITEM_LOCKER                  	32
#define ITEM_LOCKER_KEY           33
#define ITEM_CLAN_LOCKER        34
#define ITEM_KEY_RING		 35
#define ITEM_BOOK		 36
#define ITEM_IDOL		 37
#define ITEM_SCRY		 38
#define ITEM_DIGGING		 39
#define ITEM_FORGING		 40
#define ITEM_RAW		 41
#define ITEM_BONDAGE		 42
#define ITEM_TATTOO		 43
#define ITEM_TRAP		 44
#define ITEM_DOLL		 45
#define ITEM_PAPER		 46
#define ITEM_EXPLOSIVE	 47
#define ITEM_AMMO		 48
#define ITEM_SLOTMACHINE	 49
#define ITEM_HERB		 50
#define ITEM_PIPE		 51
#define ITEM_TIMEBOMB	 52
#define ITEM_TREE		 53
#define ITEM_WARP_STONE	 54
#define ITEM_ROOM_KEY	 55
#define ITEM_GEM		 56
#define ITEM_JEWELRY		 57
#define ITEM_JUKEBOX		 58
#define ITEM_SHARE		 59
#define ITEM_FIGURINE		 60
#define ITEM_FOCUS		 61
#define ITEM_PROTOCOL	 62
#define ITEM_DECORATION	 63
#define ITEM_PASSPORT	 64
#define ITEM_INSTRUMENT	 65
#define MAX_ITEM		 65

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW		(A)
#define ITEM_HUM		(B)
#define ITEM_DARK		(C)
#define ITEM_NEUTRAL		(D)
#define ITEM_EVIL		(E)
#define ITEM_INVIS		(F)
#define ITEM_MAGIC		(G)
#define ITEM_NODROP		(H)
#define ITEM_BLESS		(I)
#define ITEM_ANTI_GOOD	(J)
#define ITEM_ANTI_EVIL	(K)
#define ITEM_ANTI_NEUTRAL	(L)
#define ITEM_NOREMOVE	(M)
#define ITEM_INVENTORY	(N)
#define ITEM_NOPURGE		(O)
#define ITEM_ROT_DEATH	(P)
#define ITEM_VIS_DEATH	(Q)
#define ITEM_NO_SAC		(R)
#define ITEM_CONCEALED	(S)
#define ITEM_NO_COND		(T)
#define ITEM_SCENIC		(U)
#define ITEM_NODISARM	(V)
#define ITEM_USABLE		(W)
#define ITEM_USABLE_INF	(X)
#define ITEM_HYPER_TECH	(Y)
#define ITEM_ANIMATED	(Z)
#define ITEM_MELT_DROP	(aa)
#define ITEM_NOLOCATE	(bb)
#define ITEM_SELL_EXTRACT	(cc)
#define ITEM_NOUNCURSE	(dd)
#define ITEM_BURN_PROOF	(ee)
#define ITEM_ARTIFACT	(ff)
#define ITEM_VANISH		(gg)



#define ROM_ITEM_MASK		(S)|(T)|(U)|(V)|(W)|(X)|(Y)|(Z)
#define ROM_NONMETAL		(S)
#define ROM_NOLOCATE		(T)
#define ROM_MELT_DROP		(U)
#define ROM_HAD_TIMER		(V)
#define ROM_SELL_EXTRACT	(W)
#define ROM_BURN_PROOF		(Y)
#define ROM_NOUNCURSE		(Z)

/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE			(A)
#define ITEM_WEAR_FINGER		(B)
#define ITEM_WEAR_NECK		(C)
#define ITEM_WEAR_BODY		(D)
#define ITEM_WEAR_HEAD		(E)
#define ITEM_WEAR_LEGS		(F)
#define ITEM_WEAR_FEET		(G)
#define ITEM_WEAR_HANDS		(H)
#define ITEM_WEAR_ARMS		(I)
#define ITEM_WEAR_SHIELD		(J)
#define ITEM_WEAR_ABOUT		(K)
#define ITEM_WEAR_WAIST		(L)
#define ITEM_WEAR_WRIST		(M)
#define ITEM_WIELD			(N)
#define ITEM_HOLD			(O)
#define ITEM_TWO_HANDS		(P)
#define ITEM_WEAR_PRIDE		(Q)
#define ITEM_WEAR_FACE		(R)
#define ITEM_WEAR_EARS		(S)
#define ITEM_WEAR_FLOAT		(T)
#define ITEM_WEAR_EYES		(U)
#define ITEM_WEAR_BACK		(V)
#define ITEM_WEAR_TATTOO		(W)


/*instrument types*/
#define INSTR_VOCAL		0
#define INSTR_PERCUSSION	1
#define INSTR_STRINGS		2
#define INSTR_FLUTE		3
#define INSTR_BRASS		4
#define INSTR_PIANO		5
#define INSTR_ORGAN		6
#define INSTR_CRYSTAL		7


/*trap types*/
#define TRAP_FIGURINE		-1
#define TRAP_TANGLE		1
#define TRAP_WOUND		2
#define TRAP_DART		3
#define TRAP_MYSTIC		4
#define TRAP_MAGIC		5
#define TRAP_SCARE		6
#define TRAP_POLY		7
#define TRAP_RUST		8


/* Tree size (v0) */
#define TREE_SEED		0
#define TREE_SPROUT           	1
#define TREE_SMALL		2
#define TREE_NORMAL		3
#define TREE_LARGE           	4
#define TREE_ANCIENT		5

/* Tree type (v1) */
#define TREE_ORDINARY	0
#define TREE_LIGHT           	1
#define TREE_DARKNESS	2
#define TREE_CHAOS		3
#define TREE_ORDER           	4
#define TREE_NATURE		5
#define MAX_TREE		5


/* Photo type (v0) */
#define PHOTO_BLANK		0
#define PHOTO_EXPOSING           	1
#define PHOTO_TAKEN		2

/* Figurine behavior (v1) */
#define FIGURINE_IGNORE	0
#define FIGURINE_AGGRESSIVE   	1
#define FIGURINE_TAME		2

/* Figurine type (v2) */
#define FIGURINE_FALSE	0
#define FIGURINE_TRUE   	1
#define FIGURINE_CONT		2

/* portal class (v4) */
#define PORTAL_MAGIC			0
#define PORTAL_BUILDING		1
#define PORTAL_MIRROR		2
#define PORTAL_VEHICLE		3

/* Furniture class (v0) */
#define FURN_SIT		(A)
#define FURN_REST           	(B)
#define FURN_SLEEP		(C)

/* Pool Type (v2) */
#define POOL_GENERIC			0
#define POOL_HEALTH			1
#define POOL_MAGIC			2
#define POOL_MOVES			3

/* Refill Type (v1) */
#define REFILL_NONE			0
#define REFILL_OIL			1
#define REFILL_PARAFFIN		2
#define REFILL_KEROSENE		3
#define REFILL_BATTERY		4

/* weapon class (v0) */
#define WEAPON_EXOTIC		0
#define WEAPON_SWORD		1
#define WEAPON_DAGGER		2
#define WEAPON_SPEAR		3
#define WEAPON_MACE		 	4
#define WEAPON_AXE		 	5
#define WEAPON_FLAIL		 	6
#define WEAPON_WHIP		 	7	
#define WEAPON_POLEARM 		8
#define WEAPON_GUN               		9
#define WEAPON_HANDGUN          	10 
#define WEAPON_SMG              		11 
#define WEAPON_BOW              		12
#define WEAPON_STAFF            		13

#define MAX_WEAPON_CLASS	14

/* weapon damage types (v3) */

#define WDT_HIT		0
#define WDT_SLICE		1
#define WDT_STAB		2
#define WDT_SLASH		3
#define WDT_WHIP		4
#define WDT_CLAW		5
#define WDT_BLAST		6
#define WDT_POUND		7
#define WDT_CRUSH		8
#define WDT_GREP		9
#define WDT_BITE		10
#define WDT_PIERCE		11
#define WDT_SUCTION		12
#define WDT_BEATING		13
#define WDT_DIGESTION	14
#define WDT_CHARGE		15
#define WDT_SLAP		16
#define WDT_PUNCH		17
#define WDT_WRATH		18
#define WDT_MAGIC		19
#define WDT_DIVINE_POWER	20
#define WDT_CLEAVE		21
#define WDT_SCRATCH		22
#define WDT_PECK_PIERCE	23
#define WDT_PECK_BASH	24
#define WDT_CHOP		25
#define WDT_STING		26
#define WDT_SMASH		27
#define WDT_SHOCKING_BITE	28
#define WDT_FLAMING_BITE	29
#define WDT_FREEZING_BITE	30
#define WDT_ACIDIC_BITE	31
#define WDT_CHOMP		32
#define WDT_SHOT		33
#define WDT_LIFE_DRAIN	34
#define WDT_THIRST		35
#define WDT_SLIME		36
#define WDT_THWACK		37
#define WDT_FLAME		38
#define WDT_CHILL		39
#define WDT_ELDER		40
#define WDT_SCREECH		41

#define WDT_MAX		41
#define MAX_ATTACK_TYPES	42

/* TYPES OF ATTACKS.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 *
 * TYPE_HIT seems to get added to the Weapon_Damage_Type at some
 * point during combat.
 */

#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000


/* weapon types (v4) */

#define WEAPON_FLAMING		(A)
#define WEAPON_FROST		(B)
#define WEAPON_VAMPIRIC		(C)
#define WEAPON_SHARP		(D)
#define WEAPON_VORPAL		(E)
#define WEAPON_PLAGUE		(F)
#define WEAPON_ACID			(G)
#define WEAPON_LIGHTNING		(H)
#define WEAPON_POISON           		(I)

#define WEAPON_ONE_SHOT         	(J)
#define WEAPON_TWO_SHOT         	(K) 
#define WEAPON_SIX_SHOT         	(L)
#define WEAPON_TWELVE_SHOT      	(M)
#define WEAPON_36_SHOT          		(N)
#define WEAPON_108_SHOT         		(O)
#define WEAPON_STUN			(P)

#define WEAPON_PARABELLUM		(Q)
#define WEAPON_CAL45			(R) 
#define WEAPON_357MAGNUM		(S)
#define WEAPON_458WINCHESTER	(T)
#define WEAPON_22RIFLE		(U)
#define WEAPON_30CARABINE		(V) 
#define WEAPON_12GUAGE		(W)
#define WEAPON_ARROW		(X)
#define WEAPON_ENERGY		(Y)


/* Max brand is used to check weapon brands... */

#define MAX_BRAND		(I)
#define CHECK_BRAND		(A|B|C|D|E|F|G|H|I)

/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE		      	0
#define APPLY_STR		      	1
#define APPLY_DEX		      	2
#define APPLY_INT		      	3
#define APPLY_WIS		      	4
#define APPLY_CON		      	5
#define APPLY_SEX		      	6
#define APPLY_CLASS		      	7
#define APPLY_LEVEL		      	8
#define APPLY_AGE		      	9
#define APPLY_HEIGHT		     	10
#define APPLY_WEIGHT		     	11
#define APPLY_MANA		     	12
#define APPLY_HIT		     	13
#define APPLY_MOVE		     	14
#define APPLY_GOLD		     	15
#define APPLY_EXP		     	16
#define APPLY_AC		     	17
#define APPLY_HITROLL		     	18
#define APPLY_DAMROLL	     	19
#define APPLY_SAVING_PARA	     	20
#define APPLY_SAVING_ROD	     	21
#define APPLY_SAVING_PETRI	     	22
#define APPLY_SAVING_BREATH     	23
#define APPLY_SAVING_SPELL		24
#define APPLY_SANITY			25
#define APPLY_SKILL			26
#define APPLY_IMMUNITY     		27
#define APPLY_RESISTANCE   		28
#define APPLY_EFFECT          		29
#define APPLY_MAX_STR	      	30
#define APPLY_MAX_DEX	      	31
#define APPLY_MAX_INT	      	32
#define APPLY_MAX_WIS	      	33
#define APPLY_MAX_CON	      	34
#define APPLY_MAGIC		      	35
#define APPLY_ALIGN		      	36
#define APPLY_LUCK		      	37
#define APPLY_CHA		      	38
#define APPLY_MAX_LUCK		39
#define APPLY_MAX_CHA	      	40
#define APPLY_CRIME_RECORD	      	41
#define APPLY_OBJECT		      	42
#define APPLY_SANITY_GAIN	      	43
#define APPLY_ENV_IMMUNITY	      	44

#define APPLY_MAX		     	45


#define APPLY_TO_AFFECTS              	0
#define APPLY_TO_OBJECT               	1
#define APPLY_TO_IMMUNE               	2
#define APPLY_TO_RESIST               	3
#define APPLY_TO_VULN                 	4
#define APPLY_TO_WEAPON               	5
#define APPLY_TO_ENV_IMMUNE              	6


/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		1
#define CONT_PICKPROOF		2
#define CONT_CLOSED		      	4
#define CONT_LOCKED		      	8


/*
 * Area flags.
 */
#define   AREA_NONE		0x0000
#define   AREA_CHANGED	0x0001
#define   AREA_ADDED		0x0002
#define   AREA_LOADING	0x0004
#define   AREA_LOCKED		0x0008
#define	AREA_SECRET		0x0010
#define	AREA_EDLOCK		0x0020
#define	AREA_NOMAGIC	0x0040
#define	AREA_LOWMAGIC	0x0080
#define	AREA_HIGHMAGIC	0x0100
#define	AREA_SUPERMAGIC	0x0200
#define	AREA_NEWBIE		0x0400	
#define	AREA_LAW		0x0800	
#define	AREA_SOLITARY	0x1000	


#define VERSION_ROM		-1
#define VERSION_CTHULHU_0	0
#define VERSION_CTHULHU_1	1

#define VERSION_CTHULHU_CUR	1

#define VERSION_CTHULHU_MAX	1

/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO		2
#define ROOM_VNUM_CHAT		1300

#define DEFAULT_HOME	   	3123
#define DEFAULT_RECALL		3123
#define DEFAULT_RESPAWN         	3123
#define DEFAULT_MORGUE		3123

/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK		(A)
#define ROOM_DUMP		(B)
#define ROOM_NO_MOB		(C)
#define ROOM_INDOORS		(D)
#define ROOM_FASTHEAL	(E)
#define ROOM_PKILL		(F)
#define ROOM_RENT		(G)
#define ROOM_AUCTION_HALL	(H)
#define ROOM_BOUNTY_OFFICE	(I)
#define ROOM_PRIVATE		(J)
#define ROOM_SAFE		(K)
#define ROOM_SOLITARY	(L)
#define ROOM_PET_SHOP	(M)
#define ROOM_NO_RECALL	(N)
#define ROOM_IMP_ONLY	(O)
#define ROOM_GODS_ONLY	(P)
#define ROOM_HEROES_ONLY	(Q)
#define ROOM_NEWBIES_ONLY	(R)
#define ROOM_LAW		(S)
#define ROOM_MISTY             	(T)
#define ROOM_ECHO           	(U)
#define ROOM_BUILDING      	(V)
#define ROOM_VEHICLE         	(W)
#define ROOM_GAS                	(X)
#define ROOM_DREAM_SAFE  	(Y)
#define ROOM_NO_FLEE   	(Z)
#define ROOM_TREE           	(aa)
#define ROOM_NO_DREAM          	(bb)
#define ROOM_ALWAYS_RESET 	(cc)
#define ROOM_NOMAGIC 	(dd)
#define ROOM_MUDGATE 	(ee)


/*
 * Directions.
 * Used in #ROOMS.
 */

#define DIR_NONE			-1
#define DIR_NO_EXIT			-2
#define DIR_NOT_VISIBLE		-3

#define DIR_NORTH			 0
#define DIR_EAST			 1
#define DIR_SOUTH			 2
#define DIR_WEST		 	 3

#define DIR_UP				 4
#define DIR_DOWN			 5

#define DIR_NE				 6
#define DIR_SE				 7
#define DIR_SW				 8
#define DIR_NW				 9

#define DIR_IN				10
#define DIR_OUT			11

#define DIR_MAX 			12
#define DIR_HERE 			12
#define DIR_OTHER			13


/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR		(A)
#define EX_CLOSED		(B)
#define EX_LOCKED		(C)
#define EX_RANDOM		(D)
#define EX_PICKPROOF		(F)
#define EX_HIDDEN		(G)
#define EX_NO_PASS		(H)
#define EX_ECHO_ALL		(I)
#define EX_ECHO_SOUND	(J)
#define EX_ECHO_VISION	(K)
#define EX_ROBUST		(L)
#define EX_ARMOURED		(M)
#define EX_TRICKY		(N)
#define EX_WALL		(O)
#define EX_CREAKY		(P)


#define EX_FILE_NONE			0
#define EX_FILE_DOOR			1
#define EX_FILE_DOOR_PP		2
#define EX_FILE_DOOR_H		3
#define EX_FILE_DOOR_PP_H		4
#define EX_FILE_DOOR_NP		5
#define EX_FILE_DOOR_PP_NP		6
#define EX_FILE_DOOR_H_NP		7
#define EX_FILE_DOOR_PP_H_NP	8
#define EX_FILE_RANDOM		9

#define EX_MASK_SIGNIFICANT     	0xFFE9
#define EX_MASK_TEST			0x00E9
#define EX_MASK_CARRY		0xFF00

/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE		 	0
#define SECT_CITY		 	1
#define SECT_FIELD		 	2
#define SECT_FOREST		 	3
#define SECT_HILLS		 	4
#define SECT_MOUNTAIN		5
#define SECT_WATER_SWIM		6
#define SECT_WATER_NOSWIM	 	7

#define SECT_AIR		 	9
#define SECT_DESERT			10
#define SECT_UNDERGROUND		11
#define SECT_SWAMP			12
#define SECT_MOORS			13
#define SECT_SPACE			14
#define SECT_UNDERWATER         	15 
#define SECT_SMALL_FIRE         		16
#define SECT_FIRE               		17
#define SECT_BIG_FIRE           		18
#define SECT_COLD               		19
#define SECT_ACID               		20
#define SECT_LIGHTNING          		21
#define SECT_HOLY               		22
#define SECT_EVIL               		23
#define SECT_JUNGLE			24
#define SECT_PATH			25
#define SECT_ROAD			26
#define SECT_PLAIN			27

#define SECT_MAX			28


/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE		-1
#define WEAR_LIGHT		0
#define WEAR_FINGER_L	1
#define WEAR_FINGER_R	2
#define WEAR_NECK_1		3
#define WEAR_NECK_2		4
#define WEAR_BODY		5
#define WEAR_HEAD		6
#define WEAR_LEGS		7
#define WEAR_FEET		8
#define WEAR_HANDS		9
#define WEAR_ARMS		10
#define WEAR_SHIELD		11
#define WEAR_ABOUT		12
#define WEAR_WAIST		13
#define WEAR_WRIST_L		14
#define WEAR_WRIST_R		15
#define WEAR_WIELD		16
#define WEAR_HOLD		17
#define WEAR_WIELD2		18
#define WEAR_PRIDE		19
#define WEAR_FACE		20
#define WEAR_EARS		21
#define WEAR_FLOAT		22
#define WEAR_EYES	 	23	
#define WEAR_BACK	 	24	
#define WEAR_TATTOO	 	25	
#define MAX_WEAR		26


/* Conditions... */
 
#define COND_FOOD		      0
#define COND_DRINK		      1
#define COND_DRUNK		      2
#define COND_FAT		      3

#define NUM_CONDS		      4

// 27 points per hour, 648 points per day...

#define COND_STARVING		-120
#define COND_VERY_HUNGRY	 	000
#define COND_HUNGRY		 	120    
#define COND_STUFFED			2400
#define COND_OVER_STUFFED		3600
#define COND_STARVE_INTV	 	240
#define COND_FAT_BURN		120
#define COND_DEHYDRATED		-72
#define COND_VERY_THIRSTY	 	000
#define COND_THIRSTY		  	72
#define COND_SLUSHY			1000
#define COND_DEHYD_INTV		 96

#define COND_TIPSY		   7
#define COND_MERRY		  12
#define COND_DRUNK2		  16
#define COND_DIZZY		  20
#define COND_STUPOR		  24

#define COND_IS_SOBER		   0
#define COND_IS_TIPSY		   1
#define COND_IS_MERRY	   2
#define COND_IS_DRUNK	   3
#define COND_IS_DIZZY		   4
#define COND_IS_SICK		   5

/*
 * Positions.
 */
#define POS_DEAD		   0
#define POS_MORTAL		   1
#define POS_INCAP		   2
#define POS_STUNNED		   3
#define POS_SLEEPING		   4
#define POS_RESTING		   5
#define POS_SITTING		   6
#define POS_FIGHTING		   7
#define POS_STANDING		   8

#define MAX_POS		   9

/* Activities */

#define ACV_NONE			0
#define ACV_CASTING			1
#define ACV_SEARCHING		2
#define ACV_PICKING			3
#define ACV_SNEAKING			4
#define ACV_HIDING			5
#define ACV_TRACKING			6
#define ACV_DEBATING			7
#define ACV_DESTROYING		8
#define ACV_REBUILDING		9
#define ACV_DUEL			10
#define ACV_PHOTO			11
#define ACV_MUSIC			12

#define ACV_TRACKING_SEARCH	1
#define ACV_TRACKING_MOVE		2

#define ACV_CMD_ID			1024
#define SAN_CMD_ID			1025

#define ACV_STATE_MAX		32766

/*
 * ACT bits for players.
 */
#define PLR_IS_NPC			(A)		/* Don't EVER set.	*/
#define PLR_BOUGHT_PET		(B)
#define PLR_AUTOASSIST		(C)
#define PLR_AUTOEXIT			(D)
#define PLR_AUTOLOOT			(E)
#define PLR_AUTOSAC    	        	(F)
#define PLR_AUTOGOLD			(G)
#define PLR_AUTOSPLIT			(H)
#define PLR_PK                 			(I)     
#define PLR_INJURED			(J)
#define PLR_PERMADEATH		(K)
#define PLR_IMP_HTML			(L)
#define PLR_AUTOKILL			(M)
#define PLR_HOLYLIGHT			(N)
#define PLR_WIZINVIS			(O)
#define PLR_CANLOOT			(P)
#define PLR_NOSUMMON		(Q)
#define PLR_NOFOLLOW			(R)
#define PLR_XINFO			(S)
#define PLR_COLOUR			(T)    
#define PLR_CURSOR			(U)   
#define PLR_PUEBLO			(V)   
#define PLR_LOG			(W)
#define PLR_DENY			(X)
#define PLR_FREEZE			(Y)
#define PLR_AUTOCON			(Z)
#define PLR_ANNOYING			(aa)
#define PLR_AFK               	 		(bb)
#define PLR_HELPER			(cc)
#define PLR_AUTOSAVE			(dd)
#define PLR_CLOAK			(ee)
#define PLR_QUESTOR			(gg)
#define PLR_AUTOTAX			(hh)
#define PLR_FEED			(ii)
#define PLR_REASON			(jj)
#define PLR_HAGGLE			(kk)


#define COMM_QUIET              		(A)
#define COMM_DEAF               		(B)
#define COMM_NOWIZ              		(C)
#define COMM_NOAUCTION          	(D)
#define COMM_NOGOSSIP           		(E)
#define COMM_NOQUESTION         	(F)
#define COMM_NOMUSIC            		(G)
#define COMM_IMMACCESS		(H)
#define COMM_NOHERO			(I)
#define COMM_NOINV			(J)
#define COMM_NOCHAT		(K)
#define COMM_COMPACT		(L)
#define COMM_BRIEF			(M)
#define COMM_PROMPT			(N)
#define COMM_COMBINE		(O)
#define COMM_TELNET_GA		(P)
#define COMM_FULLFIGHT		(Q)
#define COMM_FULLCAST		(R)
#define COMM_FULLWEATHER		(S)
#define COMM_NOEMOTE		(T)
#define COMM_NOSHOUT		(U)
#define COMM_NOTELL			(V)
#define COMM_NOCHANNELS		(W) 
#define COMM_NOPORT			(X) 


#define NOTIFY_NONE                      	0
#define NOTIFY_LEVEL			(A)
#define NOTIFY_DEATH			(B)
#define NOTIFY_DELETE			(C)
#define NOTIFY_LOGIN			(D)
#define NOTIFY_QUITGAME		(E)
#define NOTIFY_LOSTLINK		(F)
#define NOTIFY_CLANPROMOTE		(G)
#define NOTIFY_UNUSED1		(H) 
#define NOTIFY_CLANACCEPT		(I)
#define NOTIFY_UNUSED2		(J) 
#define NOTIFY_CLANQUIT		(K)
#define NOTIFY_UNUSED3                  	(L) 
#define NOTIFY_UNUSED4   		(M)
#define NOTIFY_NEWNOTE		(N)
#define NOTIFY_RECONNECT		(O)
#define NOTIFY_REPOP			(P)
#define NOTIFY_WEATHER		(Q)
#define NOTIFY_TICK			(R)
#define NOTIFY_ALL			(S-1) /*flags all lower bits*/

#define TO_ALL 				1
#define TO_CLAN			2 
#define TO_PERS			3

/* anything above 52 will be interpeted as a wiznet message */
#define TO_WIZNET				52
#define TO_IMM					52 /* level 52+ */
#define TO_IMM_ADMIN	              		56 /* level 56+ */
#define TO_IMP					60 /* level 60 */

#define WIZNET_SITES			(A)
#define WIZNET_NEWBIE		(B)
#define WIZNET_SPAM			(C)
#define WIZNET_DEATH			(D)
#define WIZNET_RESET			(E)
#define WIZNET_MOBDEATH		(F)
#define WIZNET_BUG			(G)
#define WIZNET_SWITCH		(H)
#define WIZNET_LINK			(I)

/* remainder are administrative level (56+) only */
#define WIZNET_LOAD			(M)
#define WIZNET_RESTORE		(N)
#define WIZNET_SNOOP			(O)
#define WIZNET_SECURE		(P)


/* Mob natures... */

#define NATURE_STRONG			(A)
#define NATURE_FEEBLE			(B)
#define NATURE_SMART			(C)
#define NATURE_DUMB				(D)
#define NATURE_AGILE				(E)
#define NATURE_LUMBERING			(F)
#define NATURE_ROBUST			(G)
#define NATURE_SICKLY			(H)
#define NATURE_SLY				(I)
#define NATURE_GULLIBLE			(J)
#define NATURE_STURDY			(K)
#define NATURE_FRAGILE			(L)
#define NATURE_MAGICAL			(M)
#define NATURE_MUNDAIN			(N)
#define NATURE_VISCIOUS			(O)
#define NATURE_HARMLESS			(P)
#define NATURE_ARMOURED			(Q)
#define NATURE_EXPOSED			(R)
#define NATURE_MONSTEROUS			(S)
#define NATURE_LUCKY				(T)
#define NATURE_UNLUCKY			(U)
#define NATURE_CHARISMATIC			(V)
#define NATURE_DISGUSTING			(W)


/*
 *  Target types.
 */
#define TAR_IGNORE		    0
#define TAR_CHAR_SELF	    1
#define TAR_CHAR_DEFENSIVE	    2
#define TAR_CHAR_OFFENSIVE	    3
#define TAR_OBJ_INV		    4
#define TAR_CHAR_ANYWHERE	    5
#define TAR_OBJ_ROOM		    6

/* Vnums and libraries... */

typedef long VNUM;


/*
 * Shop types.
 */

#define MAX_TRADE	 5
typedef struct customer CUSTOMER;

struct customer {
	CUSTOMER	*next;
	char		*name;
	short		rating;
};

struct	shop_data
{
    SHOP_DATA *	next;
    VNUM	keeper;
    CUSTOMER	*customer_list;
    short		currency;
    short		haggle;
    short		buy_type [MAX_TRADE];
    short		profit_buy;
    short		profit_sell;
    short		open_hour;
    short		close_hour;
};


/* Deeds... */
typedef struct deed DEED;

struct deed {
	int		 id;
	short		 type;
	char		*title;
	DEED		*next;
};
 

/* Quests... */
typedef struct quest QUEST;

struct quest {
	int		 id;
	int		 state;
	char		*title;
	char		*desc;
	QUEST		*next;
};
 
/* Extended conditions... */

typedef struct econd_data ECOND_DATA;

struct econd_data {
  int		 type;
  int		 subject;
  int		 index;
  int		 low;
  int		 high;
  char		*text;
  ECOND_DATA	*next;
};

/* Mob scripts... */

typedef struct mob_script MOB_SCRIPT;
typedef struct mob_script_line MOB_SCRIPT_LINE;

struct mob_script {
        int		 id;
        char		*name;
        MOB_SCRIPT_LINE	*lines;
        MOB_SCRIPT	*next;
};

struct mob_script_line {
        short	 	seq;
        short 	 	delay;
        int		 	cmd_id;
        char 		*cmd;		 
        MOB_SCRIPT_LINE	*next;
};

/* Mob command buffers... */

typedef struct mob_cmd_buf MOB_CMD_BUF;

struct mob_cmd_buf {
        short	 	delay;
        int 		 	cmd_id;
        char			*cmd;
        MOB_CMD_BUF 	*next;
};

/* Mob command contexts... */

typedef struct mob_cmd_context MOB_CMD_CONTEXT;

typedef struct wev WEV;

struct mob_cmd_context {
	CHAR_DATA		*mob;		// The mob
        	CHAR_DATA 		*actor;		// The trigger person
	CHAR_DATA		*victim;		// Their target (maybe not mob)
	CHAR_DATA		*random;	// A random char (not the mob)
	OBJ_DATA		*obj;		// The object involved
	OBJ_DATA		*obj2;		// The object involved
        	int		 	number;		// The number involved
	char			*text;		// The text involved
        	WEV			*wev;		// World event
	ROOM_INDEX_DATA	*room;          	// Room where it occured
        	MOB_CMD_CONTEXT	*next;		// Next context for free chaining
};

/* World Events... */

struct wev {
	short		 type;
	short		 subtype;
	MOB_CMD_CONTEXT	*context;
        	char 		*msg_actor;
	char 		*msg_victim;
	char		*msg_observer;
	WEV		*next;
};

/* Mob Triggers... */

typedef struct mob_trigger MOB_TRIGGER;

struct mob_trigger {
	short		 seq;
	short		 type;
        	MOB_TRIGGER     *next_type;
        	short	 	 sub_type;
	ECOND_DATA 	*cond;
	short 		 script_id;
	char		*text;
        	MOB_TRIGGER	*next;
};


/* Conversational records... */

typedef struct conv_sub_record CONV_SUB_RECORD;

struct conv_sub_record {
	short		 sub_id;	// Subject of the conversation	
	short		 state;		// State within conversation
	CONV_SUB_RECORD *next;		// Next subject record
};

typedef struct conv_record CONV_RECORD;

struct conv_record {
	short		 conv_id;	// Conversation id
	CONV_SUB_RECORD *csr;		// Subject lists	
	CONV_RECORD 	*next;		// Next in the list
};

typedef struct conv_sub_trans_data CONV_SUB_TRANS_DATA;

struct conv_sub_trans_data {
        	short 			 seq;		// Sequence number
	short			 in;		// Input state
	ECOND_DATA		*cond;		// Input condition
	short 			 out;		// Output state
	char			*action;	// Output action
	CONV_SUB_TRANS_DATA 	*next;		// Next transition block
};		 

typedef struct conv_sub_data CONV_SUB_DATA;

struct conv_sub_data {
	short			 sub_id;	// Subject id
	char 			*name;		// Name of subject
	char			*kwds;		// Keywords
	CONV_SUB_TRANS_DATA 	*cstd;		// Transitions
	CONV_SUB_DATA 		*next;		// Next subject block
};		 

typedef struct conv_data CONV_DATA;

struct conv_data {
	short		 conv_id; 	// Conversation id
	char 		*name;		// Conversation name
	CONV_SUB_DATA 	*csd;		// Conversation subjects
	CONV_DATA	*next;		// Next conversation
};

/* Profession data... */

#define PROF_MAX_KIT 10

#define PROF_UNDEFINED -1

typedef struct prof_data PROF_DATA; 

struct prof_data {
  char  		*name;
  char  		*desc;
  SKILL_DATA	*levels;
  PROF_DATA	*next;
  int		kit[PROF_MAX_KIT]; 
  int		num_kit;
  short		stats[MAX_STATS];
  short		prime_stat;
  VNUM		start;
  short		id;
  short                  	default_soc;
  bool		pc;
  bool		initial;
  ECOND_DATA 	*ec;
  ECOND_DATA 	*iec;
};

typedef struct lprof_data LPROF_DATA;

struct lprof_data {
  PROF_DATA	*profession;
  short		 level;
  LPROF_DATA	*next;
};
  
/* A societies record of someones membership... */

typedef struct socialite SOCIALITE;

struct socialite {
  short		rank;
  int		authority;
  char		*name;
  bool		vote_cast;
  SOCIALITE	*next;
};

/* A society profession advancement condition record... */

typedef struct socadv SOCADV;

struct socadv {
  int		 level;
  ECOND_DATA 	*condition;
  SOCADV	*next;
};

/* A society record... */

typedef struct society SOCIETY;

struct society {
  short		 id;
  short		 type;
  char		*name;
  char 		*desc;
  char 		*ctitle1;
  char 		*ctitle2;
  char 		*ctitle3;
  char 		*csoul1;
  char 		*csoul2;
  SOCIALITE    	*members; 
  PROF_DATA	*profession;
  SOCADV      	*advconds;
  bool		secret;
  char		*junta;
  short                   votes;
  short                   timeout_day;
  short                   timeout_month;
  short                   tax;
  char		*bank;
  short                   home_area;
  SOCIETY	*next;
};  

/* A players/mobs record of their society membership... */

typedef struct membership MEMBERSHIP;

struct membership {
  SOCIETY	*society;
  PROF_DATA	*profession;	
  short 		 level;
  MEMBERSHIP    *next;
};

/* Chatter data... */

#define MAX_CHAT 8

typedef struct chat_data CHAT_DATA;

struct chat_data {
	short 		 state;
	short		 chance;
	short		 script[MAX_CHAT];
	short		 states[MAX_CHAT];
	CHAT_DATA	*next;
};

/* This is a collection of various mob related thingies... */

typedef struct mob_trigger_set MOB_TRIGGER_SET;

struct mob_trigger_set {
        MOB_SCRIPT      *scripts;
	MOB_TRIGGER	*challange;
	MOB_TRIGGER	*reaction;
        MEMBERSHIP	*membership;
        CHAT_DATA 	*chat;
        MOB_TRIGGER_SET *next;
};

/* Remote Event Monitoring... */

typedef struct monitor_list MONITOR_LIST;

struct monitor_list {
	short		monitor_type; 
        	short		monitor_id;
        	VNUM		vnum;
	MONITOR_LIST   *next;
};

typedef struct monitoring_list MONITORING_LIST;

struct monitoring_list {
        	short		 type;
	CHAR_DATA	*monitor;
	MONITORING_LIST	*next;
};

typedef struct anchor_list ANCHOR_LIST;

struct anchor_list {
        short		anchor_type; 
        short		anchor_id;
        VNUM		vnum;
        short                   	restrict;
        ANCHOR_LIST   *next;
};

/* Time subscription values... */

#define SUB_PULSE_1		(A)
#define SUB_PULSE_3		(B)
#define SUB_PULSE_4		(C)
#define SUB_PULSE_5		(D)
#define SUB_PULSE_10		(E)
#define SUB_PULSE_30		(F)
#define SUB_PULSE_AREA		(G)

#define SUB_TIME_HOUR		(H)
#define SUB_TIME_DAY		(I)
#define SUB_TIME_SUNRISE	(J)
#define SUB_TIME_SUNSET		(L)
#define SUB_TIME_DAWN		(K)
#define SUB_TIME_DUSK		(L)

//
// Track structures...
//

typedef struct mob_tracks	MOB_TRACKS;
typedef struct tracks		TRACKS;

struct mob_tracks {
	char		*name;
	char		*name2;
	int		 count;
	short		 size;
	MOB_TRACKS	*next;
};

struct tracks {
	MOB_TRACKS	*mob;
	short		 size;
	short		 from;
	short		 to;
	short		 script;
	TRACKS		*next;
};

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct	mob_index_data
{
    AREA_DATA		*area;
    MOB_INDEX_DATA	*next;
    SPEC_FUN		*spec_fun;
    SHOP_DATA		*pShop;
    VNUM		 vnum;
    bool		 new_format;
    short		 count;
    short		 killed;
    char		*player_name;
    char		*short_descr;
    char		*long_descr;
    char 		*description;
    char 		*bio;
    char 		*image;
    long long	act;
    long long	affected_by;
    short		alignment;
    short		level;
    short		hitroll;
    short		hit[3];
    short		mana[3];
    short		damage[3];
    short		ac[4];
    short 	atk_type;
    short 	dam_type;
    long		off_flags;
    long		imm_flags;
    long		envimm_flags;
    long		res_flags;
    long		vuln_flags;
    short		start_pos;
    short		default_pos;
    short		sex;
    short		race;
    long		gold;
    long		form;
    long		parts;
    short		size;
    short		material;
    MPROG_DATA		*mobprogs;
    int                  	progtypes;
    SKILL_DATA	*skills;
    short		language;
    LPROF_DATA	*profs;	
    CONV_DATA	*cd;		
    MOB_TRIGGER_SET	*triggers;
    int		nature;
    MONITOR_LIST	*monitors;
    MOB_TRACKS		*tracks;	
    int		time_wev;
    short		fright;
    short		group;
    ECOND_DATA		*can_see;
    ECOND_DATA		*can_not_see;
};


/*
 * One character (PC or NPC).  Be sure to add fields here always!
 */

#define MOB_MEMORY	10

struct	char_data
{
    CHAR_DATA		*next;
    CHAR_DATA		*next_in_room;
    CHAR_DATA		*master;
    CHAR_DATA		*leader;
    CHAR_DATA		*fighting;
    CHAR_DATA		*huntedBy;
    CHAR_DATA 		*prey;  
    CHAR_DATA 		*reply;  
    CHAR_DATA		*distractedBy;
    CHAR_DATA		*pet;
    SPEC_FUN		*spec_fun;
    MOB_INDEX_DATA	*pIndexData;
    RESET_DATA		*reset;
    DESCRIPTOR_DATA	*desc;
    AFFECT_DATA	*affected;
    OBJ_DATA		*carrying;
    OBJ_DATA		*ooz;
    OBJ_DATA		*on_furniture;
    OBJ_DATA		*trans_obj;
    ROOM_INDEX_DATA	*in_room;
    ROOM_INDEX_DATA	*was_in_room; 
    ROOM_INDEX_DATA	*waking_room;
    ROOM_INDEX_DATA	*dreaming_room;
    PC_DATA		*pcdata;
    char			*memory[MOB_MEMORY];
    char 			*name;
    short		 	version;
    char			*short_descr;
    char			*long_descr;
    char			*description;
    char			*true_name;
    char 			*bio;
    bool		 	quitting; 
    bool		 	fleeing;
    bool		 	awakening;
    bool		 	nightmare;
    time_t               		quaffLast; 
    char			*poly_name;
    char			*short_descr_orig;
    char			*long_descr_orig;
    char 			*description_orig;
    short		 	race_orig;
    short		 	sex;
    short		 	pc_class;
    short		 	race;
    short		 	level;
    short		 	in_zone; 
    VNUM		 recall_perm;
    VNUM		 recall_temp;
    int			 lines;
    short 		 chat_state;
    short		 	timer;
    short		 	wait;
    short		 	hit;
    short		 	max_hit;
    short		 	mana;
    short		 	max_mana;
    short		 	move;
    short		 	max_move;
    long		 	gold[MAX_CURRENCY];
    long long	 	act;
    long long	 	plr;
    long		 	comm;
    long		 	notify;
    long		 	wiznet;
    long		 	imm_flags;
    long		 	envimm_flags;
    long		 	res_flags;
    long		 	vuln_flags;
    long		 	form;
    long		 	parts;
    long		 	off_flags;
    long long	 	affected_by;
    long                     	artifacted_by;     
    int			exp;
    short			group;
    short		 	invis_level;
    short		 	cloak_level;
    short		 	practice;
    short		 	train;
    short		 	sanity;
    short		 	carry_weight;
    short		 	carry_number;
    short		 	saving_throw;
    short		 	alignment;
    short		 	hitroll;
    short		 	damroll;
    short		 	armor[4];
    short		 	wimpy;
    short		 	wimpy_dir;
    short			position;
    short			activity;
    char 			*pos_desc;
    char			*acv_desc;
    int			acv_key;
    int			acv_state;
    int			acv_int;
    int			acv_int2;
    int			acv_int3;
    char *		 	acv_text;
    short			perm_stat[MAX_STATS];
    short			mod_stat[MAX_STATS];
    short			training[TRAIN_MAX];
    short		 	size;
    short		 	material;
    short		 	damage[3];
    short		 	atk_type;
    short		 	dam_type;
    short		 	start_pos;
    short		 	default_pos;
    short		 	condition[NUM_CONDS];
    short		 	limb[6];
    MPROG_ACT_LIST	*mpact;
    int			mpactnum;
    short		 	gun_state[2]; 
    SKILL_DATA		*effective;
    LPROF_DATA		*profs;
    CONV_RECORD	*cr;
    MOB_CMD_BUF	*mcb;
    DEED			*deeds;	
    QUEST		*quests;	
    MOB_TRIGGER_SET	*triggers;	
    MEMBERSHIP		*memberships;
    int			nature;
    int			time_wev;
    short			speak[2];
    short			fright;
    MOB_TRACKS	*tracks;
    short              		ridden; 			
    short              		were_type;
};

#define GUN_AUX	 	-2
#define GUN_JAMMED    	 	-1
#define GUN_NO_AMMO 	0

#define SPEAK_PUBLIC	0
#define SPEAK_PRIVATE	1

struct skill_data {
    SKILL_DATA         	*next; 
    long			total_points;
    short 		skill[MAX_SKILL];
}; 


typedef struct ignore IGNORE;

struct ignore {
	IGNORE		*next;
	char		*name;
};

typedef struct passport PASSPORT;

struct passport {
	PASSPORT	*next;
	char		*name;
	short		type;
	short		day;
	short		month;
	int		year;
};

/*
 * IMM Authority.
 */
#define IMMAUTH_BUILDER	(A)
#define IMMAUTH_QUESTOR	(B)
#define IMMAUTH_ENFORCER	(C)
#define IMMAUTH_ADMIN	(D)
#define IMMAUTH_ALL		(A) | (B) | (C) | (D)


/*
 * Data which only PC's have.
 */
struct	pc_data
{
    PC_DATA *		next;
    char *			pwd;
     short		pwd_tries;
    char *			native;
    char *			bamfin;
    char *			bamfout;
    char *			title;
    char *             		email;
    char *            		image;
    char *             		url;
    char * 		immtitle;
    struct alias_data	*aliases[MAX_ALIAS];
    bool			has_alias;
    IGNORE		*ignore_list;
    short             		startyear;
    short             		startmonth;
    short             		startday;
    int			age_mod;
    short			perm_hit;
    short			perm_mana;
    short			perm_move;
    short			true_sex;
    int			last_level;
    short          	 	divinity; 
    short          	 	moddiv; 
    short          	 	authority; 
    short                		security;
    char*                  	cult;
    short                 	style;
    short                 	blood;
    char*                  	bloodline;
    short                 	lifespan;
    long                    	bounty;
    short                 	mounted;
    char*                  	spouse;
    char*			prompt;
    bool              		confirm_delete;
    short			pk_timer;
    char			*help_topic;
    short			help_count;
    VNUM		home_room;
    bool			macro;
    short          		macro_script;
    short			macro_line;
    short			macro_delay;
    int      			answer;
    int   		 	played;
    time_t	 		logon;
    CHAR_DATA		*questgiver; 
    short			 questpoints;  
    short		 	nextquest;
    short		 	countdown;
    short		 	questmob;
    short		 	questobj;
    char			*questplr;
    BOARD_DATA 	*board;
    time_t          		last_note[MAX_BOARD];
    NOTE_DATA 		*in_progress;
    CHAR_DATA		*store_char;
    OBJ_DATA		*store_obj;
    int			store_number[2];
};

/* Data for generating characters -- only used during generation */
struct gen_data
{
    GEN_DATA	*next;
    bool		skill_chosen[MAX_SKILL];
    int		points_chosen;
};


/*
 * Liquids.
 */
#define LIQ_WATER		 0
#define LIQ_BEER		 1
#define LIQ_RED_WINE		 2
#define LIQ_ALE			 3
#define LIQ_DARK_ALE		 4

#define LIQ_WHISKY		 5
#define LIQ_LEMONADE		 6
#define LIQ_FIREBREATHER	 7
#define LIQ_SPECIAL		 8
#define LIQ_JUICE		 9

#define LIQ_MILK		10
#define LIQ_TEA			11
#define	LIQ_COFFEE		12
#define LIQ_BLOOD		13
#define LIQ_SALT_WATER		14

#define LIQ_CHERRY_COLA		15
#define LIQ_DIRTY_WATER		16
#define LIQ_AMBROSIA		17
#define LIQ_SLIME		18
#define LIQ_DARK_COFFEE		19

#define LIQ_VODKA		20
#define LIQ_SODA_WATER		21
#define LIQ_PETROL   	        22
#define LIQ_ROOT_BEER  	        23
#define LIQ_ELF_WINE  	        24

#define LIQ_WHITE_WINE 	        25
#define LIQ_CHAMPAGNE 	        26
#define LIQ_MEAD 	        27
#define LIQ_ROSE_WINE 	        28
#define LIQ_BENEDICTINE	        29

#define LIQ_CRANBERRY	        30
#define LIQ_ORANGE	        31
#define LIQ_ABSINTHE	        32
#define LIQ_BRANDY	        33
#define LIQ_AQUAVIT	        34

#define LIQ_SCHNAPPS	        35
#define LIQ_ICEWINE	        36
#define LIQ_AMONTILLADO	        37
#define LIQ_SHERRY	        38
#define LIQ_FRAMBOISE	        39

#define LIQ_RUM		        40
#define LIQ_CORDIAL	        41
#define LIQ_PORT	        42
#define LIQ_WHEATGRASS	        43

#define LIQ_MAX			44

struct	liq_type
{
    char *	liq_name;
    char *	liq_color;
    short	liq_affect[3];
    long	immune;
    long	food;
};



/*
 * Extra description data for a room or object.
 */
struct	extra_descr_data
{
    EXTRA_DESCR_DATA *next;	/* Next in list                     */
    char *keyword;              /* Keyword in look/examine          */
    char *description;          /* What to see                      */
    ECOND_DATA *ec;		/* Conditions			    */
    short  id;			/* Identifier                       */
    DEED *deed;			/* Deed				    */
};


typedef struct artifact	ARTIFACT;

/*
 * Special Artifact data
 */
struct	artifact
{
    ARTIFACT	 *next;
    long		power;
    short		attract;
    short                 pulse;
};


/*
 * Prototype for an object.
 */
struct	obj_index_data
{
    AREA_DATA *         area;
    OBJ_INDEX_DATA *	next;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    ANCHOR_LIST	*anchors;
    bool		new_format;
    char *		name;
    char *		short_descr;
    char *		description;
    char *		image;
    VNUM	vnum;
    short	 	repop;	
    short		reset_num;
    short		material;
    short		item_type;
    long long	extra_flags;
    int		wear_flags;
    ECOND_DATA *wear_cond;
    ARTIFACT       *artifact_data;
    short		level;
    short 	condition;
    short		count;
    short		weight;
    short		size;
    int		cost;
    long		value[5];
    long		zmask;
};



/*
 * One object.
 */
struct	obj_data
{
    OBJ_DATA		*next;
    OBJ_DATA		*next_content;
    OBJ_DATA		*contains;
    OBJ_DATA		*in_obj;
    CHAR_DATA		*carried_by;
    CHAR_DATA		*trans_char;
    RESET_DATA		*reset;
    EXTRA_DESCR_DATA *extra_descr;
    AFFECT_DATA	*affected;
    OBJ_INDEX_DATA	*pIndexData;
    VNUM		orig_index;
    ROOM_INDEX_DATA	*in_room;
    bool		 	enchanted;
    char			*owner;
    char			*name;
    char			*short_descr;
    char			*description;
    short		 	item_type;
    long long	 	extra_flags;
    int			wear_flags;
    short		 	wear_loc;
    short	 	 	weight;
    int			cost; 
    short		 	level;
    short 		condition;
    short		 	material;
    short		 	size;
    short		 	timer;
    long		 	value	[5];
    long 		 	valueorig [5];
    long		 	zmask; 
};


typedef struct cond_dest_data COND_DEST_DATA;

struct cond_dest_data {

    int		 	 seq;		// Sequence number for ordering

    VNUM		 vnum;		// Vnum of destination room
    ROOM_INDEX_DATA	*dest;		// Destination room

    char		*name;		// Description type info

    ECOND_DATA		*cond;		// Condition
    COND_DEST_DATA	*next;		// Next cdd
}; 

/*
 * Exit data.
 */
struct	exit_data
{
    union
    {
	ROOM_INDEX_DATA *	to_room;
	VNUM			vnum;
    } u1;
    short		 	exit_info;
    VNUM		key;
    char 			*keyword;
    char 			*description;
    char 			*transition;
    ECOND_DATA 	*invis_cond;	
    COND_DEST_DATA	*cond_dest;
    EXIT_DATA 		*next;
    int                  		rs_flags;
    int                  		orig_door;
    short           		condition;
};


struct  auction_data {
    OBJ_DATA  		*item; 
    CHAR_DATA 		*seller;
    CHAR_DATA 		*buyer;
    ROOM_INDEX_DATA 	*auction_room;
    int         		bet;
    short      		going;
    short      		pulse;
};


/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile 
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct	reset_data
{
    RESET_DATA *	next;
    char		command;
    VNUM	arg1;
    short		arg2;
    VNUM	arg3;
    int		count;
};


/*
 * Area definition.
 */

struct  area_data
{
    AREA_DATA	*next;           
    char		*name;
    char		*copyright;
    short		 zone;
    short                	age;
    int                  	nplayer;
    bool                 	empty;
    char	        	*filename;
    char	        	*builders;
    short                	security;
    VNUM               	lvnum;
    VNUM               	uvnum;
    int                  	vnum;
    int                  	area_flags;
    VNUM	recall;
    VNUM	respawn;
    VNUM	morgue;
    VNUM	dream;
    VNUM	mare;
    VNUM	prison;
    short		weather;
    short		old_weather;
    short		climate;
    short		surface;
    char	        	*map;
    bool                 	invasion;
    short		martial;
    short		version;              
    short               	worth[MAX_CURRENCY];
};


/* Gadgets */

typedef struct gadget_trans_data GADGET_TRANS_DATA;

struct gadget_trans_data {
	int			 seq;
                int			 action;
	int			 in;
	int			 out;
	char			*message;
	ECOND_DATA		*cond;
	GADGET_TRANS_DATA	*next; 
};


typedef struct gadget_data GADGET_DATA;

struct gadget_data {
	int			 id;
	char			*name;
	int			 state;
                VNUM                                    link_room;
                int                                            link_id;
	GADGET_TRANS_DATA	*gdt;
                GADGET_DATA		*next; 
};

/* Currents */

typedef struct current CURRENT;

struct current {
        	short		 seq;
	char		*name;
	char 		*destination;
	short		 dir;
	char 		*action; 
        	ECOND_DATA	*cond;
	CURRENT		*next;
};

/*
 * Room type.
 */
struct	room_index_data
{ 
    ROOM_INDEX_DATA	*next;
    ROOM_INDEX_DATA 	*next_collected;
    CHAR_DATA		*people;
    OBJ_DATA		*contents;
    AFFECT_DATA	*affected;
    EXTRA_DESCR_DATA*extra_descr;
    AREA_DATA		*area;
    short			 subarea;
    EXIT_DATA		*exit[DIR_MAX];
    RESET_DATA		*reset_first;
    RESET_DATA		*reset_last;
    char			*name;
    char			*description;
    char			*image;
    VNUM		 vnum;
    long long		 room_flags;
    short		 	light;
    short		 	sector_type;
    long		 	room_rent;
    char 			*owner;
    char 			*rented_by;
    short		 	paid_month;
    short 	 	paid_day;
    short		 	paid_year;
    VNUM		recall;
    VNUM		respawn;
    VNUM		morgue;
    VNUM		dream;
    VNUM		mare;
    VNUM		night;
    VNUM		day;
    short			heal_rate;
    short			mana_rate;	
    long                		affected_by;
    GADGET_DATA	*gadgets;
    CURRENT 		*currents;
    MONITORING_LIST	*monitors;
    TRACKS		*tracks;
    TRACKS		*ptracks;
    short			distance;
};


/*****************************************************************************
 *                                    OLC                                    *
 *****************************************************************************/

/*
 * This structure is used in special.c to lookup spec funcs and
 * also in olc_act.c to display listings of spec funcs.
 */
struct spec_type
{
    char *      spec_name;
    SPEC_FUN *  spec_fun;
};



/*
 * This structure is used in bit.c to lookup flags and stats.
 */
struct flag_type
{
    char * name;
    long long  bit;
    bool settable;
};


#define SKILL_UNDEFINED		-1

struct skill_info {
	char		*name;
	short	 	 learn;
	int	 	 stats[2];
	short	 	 group;
	short	 	 minpos;
	short	 	 beats;
	bool	 	 learnable;
	bool	 	 debate;
	bool	 	 amateur;
        	bool	 	 loaded;
        	ECOND_DATA 	*ec;
};

extern  int alphasort();
extern SKILL_INFO skill_array[MAX_SKILL];

/*
 * Utility macros.
 */
#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))


/*
 * Character macros.
 */
#define IS_NPC(ch)		(IS_SET((ch)->act, ACT_IS_NPC))
#define IS_TRUSTED(ch,level)	(get_trust((ch)) >= (level))
#define IS_AFFECTED(ch, sn)	(IS_SET((ch)->affected_by, (sn)))
#define IS_ARTIFACTED(ch, sn)	(IS_SET((ch)->artifacted_by, (sn)))
#define IS_RAFFECTED(room, sn)	(IS_SET((room)->affected_by, (sn)))

#define IS_FLEEING(ch)	 	(ch->fleeing)

#define IS_NEWBIE(ch)         	(get_divinity((ch))== 0)
#define IS_HUMAN(ch)          	(IS_SET(get_divinity((ch)), DIV_HUMAN))
#define IS_INVESTIGATOR(ch)      	(IS_SET(get_divinity((ch)), DIV_INVESTIGATOR))
#define IS_HERO(ch)            	(IS_SET(get_divinity((ch)), DIV_HERO))
#define IS_CREATOR(ch)        	(IS_SET(get_divinity((ch)), DIV_CREATOR))
#define IS_LESSER(ch)         	(IS_SET(get_divinity((ch)), DIV_LESSER))
#define IS_GOD(ch)             	(IS_SET(get_divinity((ch)), DIV_GOD))
#define IS_GREATER(ch)         	(IS_SET(get_divinity((ch)), DIV_GREATER))
#define IS_IMP(ch)             	(IS_SET(get_divinity((ch)), DIV_IMPLEMENTOR))
#define IS_MUD(ch)             	(IS_SET(get_divinity((ch)), DIV_MUD))

#define IS_MORTAL(ch)           	(get_divinity((ch)) < DIV_CREATOR)
#define IS_IMMORTAL(ch)       	(get_divinity((ch)) >= DIV_CREATOR)
#define IS_DIVINE(ch)          	(get_divinity((ch)) > DIV_HERO)
#define IS_QUESTOR(ch)                 	(IS_SET((ch)->act, PLR_QUESTOR))
#define IS_HELPER(ch)  		(IS_SET((ch)->plr, PLR_HELPER))

#define IS_GOOD(ch)		(ch->alignment >= 350)
#define IS_EVIL(ch)		(ch->alignment <= -350)
#define IS_NEUTRAL(ch)		(!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)		(ch->position > POS_SLEEPING)
#define GET_AC(ch,type)		((ch)->armor[type]  + ( IS_AWAKE(ch) ? dex_app[get_curr_stat(ch,STAT_DEX)].defensive : 0 ))  
#define GET_HITROLL(ch) 	((ch)->hitroll+str_app[get_curr_stat(ch,STAT_STR)].tohit)
#define GET_DAMROLL(ch) 	((ch)->damroll+str_app[get_curr_stat(ch,STAT_STR)].todam)


#define WAIT_STATE(ch, npulse)	(IS_IMMORTAL((ch)) ? (ch)->wait = UMAX((ch)->wait, (npulse/4)) : (ch)->wait = UMAX((ch)->wait, (npulse)))

#define CH(descriptor)  ((descriptor)->original ? (descriptor)->original : (descriptor)->character)



/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))
#define IS_WEAPON_STAT(obj,stat)(IS_SET((obj)->value[4],(stat)))



/*
 * Description macros.
 */
#define PERSMASK(ch, looker)	( can_see( looker, (ch) ) ?		\
				 IS_AFFECTED((ch),AFF_POLY) ?  \
				(ch)->short_descr  \
				: IS_NPC(ch) ? (ch)->short_descr	\
				: (ch)->name : IS_IMMORTAL(ch) ? "an Immortal" : "someone" )


#define PERSORIG(ch, looker)	( can_see( looker, (ch) ) ?		\
				 IS_AFFECTED((ch),AFF_POLY) ?  \
				(ch)->short_descr_orig  : (ch)->short_descr : IS_IMMORTAL(ch) ? "an Immortal" : "someone" )

#define PERS(ch, looker)		( can_see( looker, (ch) ) ?		\
				 IS_AFFECTED((ch),AFF_POLY) ? IS_NPC(ch) ?  \
				(ch)->short_descr :  (ch)->poly_name   \
				: IS_NPC(ch) ? (ch)->short_descr	\
				: (ch)->name : IS_IMMORTAL(ch) ? "an Immortal" : "someone" )

/*
 * Structure for a social in the socials table.
 */
struct	social_type
{
    char      name[20];
    char *    char_no_arg;
    char *    others_no_arg;
    char *    char_found;
    char *    others_found;
    char *    vict_found;
    char *    char_not_found;
    char *    char_auto;
    char *    others_auto;
    int          s_style;
};

/* For the use as s_styles */
#define SOCIAL_NEUTRAL	0
#define SOCIAL_FRIENDLY	1
#define SOCIAL_HOSTILE	2
#define SOCIAL_SEXUAL	3


typedef struct rumor RUMOR_DATA;

struct rumor {
	char		*fortune;
	char		*command;
	bool		greater;
	ECOND_DATA	*ec;
	RUMOR_DATA	*next;
};

extern RUMOR_DATA rumor_array[MAX_RUMOR];

/*
 * Global constants.
 */
extern const struct str_app_type	str_app[112];
extern const struct int_app_type	int_app[112];
extern const struct wis_app_type	wis_app[112];
extern const struct dex_app_type	dex_app[112];
extern const struct con_app_type	con_app[112];
extern const struct class_type	class_table[MAX_CLASS];
extern const struct attack_type	attack_table[];
extern const struct race_type	race_table[];
extern const struct pc_race_type	pc_race_table[];
extern const struct liq_type	liq_table[LIQ_MAX];
extern struct social_type 		social_table[MAX_SOCIALS];
extern const char 			*size_table[];
extern const char 			*cond_table[];
extern const struct material_data	material_table[];
extern const short 		rev_dir[];
extern const struct spec_type       	spec_table[];

/*
 * Global variables.
 */
extern		HELP_DATA	  	*help_first;
extern		SHOP_DATA	  	*shop_first;
extern		BAN_DATA	  	*ban_list;
extern		PASSPORT	  	*passport_list;
extern		CHAR_DATA	  	*char_list;
extern		DESCRIPTOR_DATA 	*descriptor_list;
extern		NOTE_DATA	  	*note_list;
extern		OBJ_DATA	  	*object_list;
extern		AFFECT_DATA	  	*affect_free;
extern		BAN_DATA	  	*ban_free;
extern		CHAR_DATA	  	*char_free;
extern		DESCRIPTOR_DATA	*descriptor_free;
extern		EXTRA_DESCR_DATA  	*extra_descr_free;
extern		NOTE_DATA	  	*note_free;
extern		OBJ_DATA	  	*obj_free;
extern		PC_DATA		*pcdata_free;

extern		char			bug_buf[];
extern		time_t			current_time;
extern		bool			fLogAll;
extern		FILE 			*fpReserve;
extern		KILL_DATA		kill_table[];
extern		char			log_buf	[];
extern		TIME_INFO_DATA	time_info;
extern		WEATHER_DATA	weather_info;

extern          	AREA_DATA 		*area_first;
extern          	AREA_DATA 		*area_last;
extern          	SHOP_DATA 		*shop_last;
extern          	AUCTION_DATA      	*auction;

extern          	int		        	top_affect;
extern          	int                     		top_area;
extern          	int                     		top_ed;
extern          	int                     		top_exit;
extern          	int                     		top_help;
extern          	int                     		top_mob_index;
extern          	int                     		top_obj_index;
extern          	int                     		top_reset;
extern          	int                     		top_room;
extern          	int                     		top_shop;
extern          	int                     		rumor_count;
extern          	VNUM                    	top_vnum_mob;
extern          	VNUM                    	top_vnum_obj;
extern          	VNUM                    	top_vnum_room;
extern          	char                    		str_empty[1];

extern  		MOB_INDEX_DATA 	*mob_index_hash[MAX_KEY_HASH];
extern  		OBJ_INDEX_DATA 	*obj_index_hash[MAX_KEY_HASH];
extern  		ROOM_INDEX_DATA 	*room_index_hash[MAX_KEY_HASH];


char *	crypt		args( ( const char *key, const char *salt ) );

extern bool isreboot;
extern int pulse_muddeath;
extern CHAR_DATA * mudkiller;

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define SF	SPEC_FUN

/* lease.c */
void    save_leases	args ((void));

/* act_comm.c */
void  	delete_char		(CHAR_DATA *ch);
void  	check_sex		(CHAR_DATA *ch);
void	add_follower		(CHAR_DATA *ch, CHAR_DATA *master);
void	stop_follower		(CHAR_DATA *ch);
void 	nuke_pets		(CHAR_DATA *ch);
void	die_follower		(CHAR_DATA *ch);
bool	is_same_group		(CHAR_DATA *ach, CHAR_DATA *bch);
void 	talk_auction 		(char *argument);
void        drop_artifact		(CHAR_DATA *ch, OBJ_DATA *obj);

/* act_info.c */
void	set_title			(CHAR_DATA *ch, char *title);
void	set_worship		(CHAR_DATA *ch, char *title);
bool	is_outside		(CHAR_DATA *ch);
int	get_weather		(ROOM_INDEX_DATA *room);
int	get_surface		(ROOM_INDEX_DATA *room);
long 	get_full_cash		(CHAR_DATA *ch);
int 	find_currency		(CHAR_DATA *ch);
int 	find_currency_unreset	(CHAR_DATA *ch);
int 	find_obj_currency	(OBJ_DATA *obj);
float 	get_currency_modifier	(AREA_DATA *area, int currency);
void 	examine_object		(CHAR_DATA *ch, OBJ_DATA *obj);
extern char *const month_name[];


/* act_move.c */
void	move_char		(CHAR_DATA *ch, int door, bool follow);
void	make_char_move		(CHAR_DATA *ch, int door, ROOM_INDEX_DATA *dest, int move);
void        set_furnaff		(CHAR_DATA *ch, OBJ_DATA *obj);
void        remove_furnaff		(CHAR_DATA *ch, OBJ_DATA *obj);
extern char *const dir_name[];


/* act_obj.c */
bool    	can_loot			(CHAR_DATA *ch, OBJ_DATA *obj);
void    	get_obj         		(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container);
bool	wear_obj_size		(CHAR_DATA *ch, OBJ_DATA *obj);
CD *	find_keeper		(CHAR_DATA *ch, bool loud);
void 	make_photo		(CHAR_DATA *ch, OBJ_DATA *camera, OBJ_DATA *photo);
void 	light_obj		(CHAR_DATA *ch, OBJ_DATA *obj, bool on);
void 	execute_sell		(CHAR_DATA *ch, CHAR_DATA *keeper, OBJ_DATA *obj, int cost);
void 	execute_buy		(CHAR_DATA *ch, CHAR_DATA *keeper, OBJ_DATA *obj, int cost, int number);
PASSPORT *get_passport_order	(CHAR_DATA *ch);

/* obj_cond.c */
void	show_obj_cond 		(CHAR_DATA *ch, OBJ_DATA *obj);
void 	check_damage_obj 	(CHAR_DATA *ch, OBJ_DATA *obj, int chance);
void 	damage_obj		(CHAR_DATA *ch, OBJ_DATA *obj, int damage);
void 	set_obj_cond		(OBJ_DATA *obj, int condition);

/* act_wiz.c */
ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg );
void 	inform_approval		(CHAR_DATA *ch);

/* comm.c */
void	show_string		(struct descriptor_data *d, char *input);
void	close_socket		(DESCRIPTOR_DATA *dclose, bool save_first);
void	write_to_buffer		(DESCRIPTOR_DATA *d, const char *txt, int length);
void	send_to_char		(const char *txt, CHAR_DATA *ch);
void	page_to_char		(const char *txt, CHAR_DATA *ch);
void	act			(const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type);
void	act_new			(const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type, int min_pos);
int	colour			(char type, CHAR_DATA *ch, char *string );
void	colourconv		(char *buffer, const char *txt, CHAR_DATA *ch);
void	send_to_char_bw	(const char *txt, CHAR_DATA *ch);
void	page_to_char_bw	(const char *txt, CHAR_DATA *ch );
void        gotoxy          		(CHAR_DATA *ch, int arg1, int arg2);
int           advatoi 			(const char *s);
int           parsebet 		(const int currentbet, const char *argument);
void        sprintf_to_char    		(CHAR_DATA *, char *, ... );
void        sprintf_to_world   	(char *, ...);
void        sprintf_to_room    	(ROOM_INDEX_DATA *, char *, ...);
char 	*strip_heading		(char *kwd, int feed);
bool 	chance			(int num);
void 	clear_screen		(DESCRIPTOR_DATA *d);


/* db.c */
void 	fwrite_disable      		(void);
void	boot_db			(bool fCopyOver);
void	area_update		(void);
void 	random_apply		(OBJ_DATA *obj, CHAR_DATA *mob);
int 	random_spell		(int level, int mask, short *type);
void 	wield_random_magic	(CHAR_DATA *mob );
void 	wield_random_armor	(CHAR_DATA *mob );
CD *	create_mobile		(MOB_INDEX_DATA *pMobIndex);
CD *	create_mobile_level	(MOB_INDEX_DATA *pMobIndex, char *profession, int level);
void	set_mobile_level		(CHAR_DATA *mob, PROF_DATA *prof, int level );
void	clone_mobile		(CHAR_DATA *parent, CHAR_DATA *clone);
OD *	create_object		(OBJ_INDEX_DATA *pObjIndex, int level);
void	clone_object		(OBJ_DATA *parent, OBJ_DATA *clone);
void	clear_char		(CHAR_DATA *ch);
void	free_char		(CHAR_DATA *ch );
char *	get_extra_descr		(char *name, EXTRA_DESCR_DATA *ed);
MID *	get_mob_index		(VNUM vnum);
OID *	get_obj_index		(VNUM vnum);
RID *	get_room_index		(VNUM vnum);
char	fread_letter		(FILE *fp);
int		fread_number		(FILE *fp);
long		fread_number_long	(FILE *fp);
long long	fread_number_long_long	(FILE *fp);
long 		fread_flag		(FILE *fp);
long long 	fread_flag_long		(FILE *fp);
char *	fread_string		(FILE *fp);
char *	fread_html		(FILE *fp);
char *  	fread_string_eol 		(FILE *fp);
void	fread_to_eol		(FILE *fp);
char *	fread_word		(FILE *fp);
long	flag_convert		(char letter);
void *	alloc_mem		(int sMem);
void *	alloc_perm		(int sMem);
void	free_mem		(void *pMem, int sMem);
char *	str_dup			(const char *str);
void	free_string		(char *pstr);
int	number_fuzzy		(int number);
int	number_range		(int from, int to);
int	number_percent		(void);
int 	number_open		(void);
int	number_door		(void );
int	number_bits		(int width);
int     	number_mm       		(void);
int	dice			(int number, int size);
int	interpolate		(int level, int value_00, int value_32);
void	smash_tilde		(char *str);
bool	str_cmp			(const char *astr, const char *bstr);
bool	str_prefix		(const char *astr, const char *bstr);
bool	str_infix			(const char *astr, const char *bstr);
bool	str_suffix		(const char *astr, const char *bstr);
char *	capitalize		(const char *str);
void	append_file		(CHAR_DATA *ch, char *file, char *str);
void	bug			(const char *str, int param);
void	log_string		(const char *str);
void	tail_chain		(void);
void    	reset_area      		(AREA_DATA * pArea, bool forcereset);
void    	reset_room      		(ROOM_INDEX_DATA *pRoom, bool invasion, bool forcereset);
void 	adjust_stats_by_nature	(CHAR_DATA *mob);
PASSPORT *new_pass_order	(void);
void 	free_pass_order		(PASSPORT *pass);
void 	save_passport		(void);


extern void inline null_extra_cond(EXTRA_DESCR_DATA *ed);
extern void read_extra_cond(EXTRA_DESCR_DATA *ed, FILE *fp);
extern bool show_extra_cond(CHAR_DATA *ch, EXTRA_DESCR_DATA *ed, char *kwds);

/* string.c */
void    	string_edit     		(CHAR_DATA *ch, char **pString);
void    	string_append   		(CHAR_DATA *ch, char **pString);
char *  	string_replace  		(char * orig, char * old_str, char * new_str);
void    	string_add      		(CHAR_DATA *ch, char *argument);
char *  	format_string   		(char *oldstring /*, bool fSpace */ );
char *  	first_arg       		(char *argument, char *arg_first, bool fCase);
char *  	string_unpad    		(char * argument);
char *  	string_proper   		(char * argument);

/* olc.c */
char 	*strip_cr        		( char *str  );
bool    	run_olc_editor  		(DESCRIPTOR_DATA *d);
char    	*olc_ed_name    		(CHAR_DATA *ch);
char    	*olc_ed_vnum    		(CHAR_DATA *ch);
long	flag_value		(const struct flag_type *flag_table, char *argument);
long long flag_value_long		(const struct flag_type *flag_table, char *argument);
char 	*flag_string       		(const struct flag_type *flag_table, long long bits);
void 	create_vehicle		(CHAR_DATA *ch, OBJ_DATA *t_obj);


/* mob_commands.c */
char *	mprog_type_to_name  	(int type);

/* fight.c */
bool 	is_safe			(CHAR_DATA *ch, CHAR_DATA *victim);
bool 	is_safe_spell		(CHAR_DATA *ch, CHAR_DATA *victim, bool area);
void	violence_update		(void);
void	multi_hit			(CHAR_DATA *ch, CHAR_DATA *victim, int dt);
bool	damage			(CHAR_DATA *ch, CHAR_DATA *victim, int dam,  int dt, int dam_class);
void	update_pos		(CHAR_DATA *victim);
void	stop_fighting		(CHAR_DATA *ch, bool fBoth);
void    	death_cry   		(CHAR_DATA *ch, bool corpse);
void	drop_level  		(CHAR_DATA *ch);
bool 	can_murder   		(CHAR_DATA *ch, CHAR_DATA *victim);
bool 	check_material_vuln 	(OBJ_DATA *obj, CHAR_DATA *victim);
void    	check_spirit    		(CHAR_DATA *ch, CHAR_DATA *victim);
void    	check_undead    		(CHAR_DATA *ch, CHAR_DATA *victim);
void 	trap_trigger 		(CHAR_DATA *ch, OBJ_DATA *container);
void	check_assist		(CHAR_DATA *ch, CHAR_DATA *victim);
void 	update_wounds		(CHAR_DATA *ch);
void 	bomb_explode 		(OBJ_DATA *bomb);

/* mob_prog.c */
void    mprog_wordlist_check    args ( ( char * arg, CHAR_DATA *mob, CHAR_DATA* actor, OBJ_DATA* object, void* vo, int type ) );
void    mprog_percent_check     args ( ( CHAR_DATA *mob, CHAR_DATA* actor,  OBJ_DATA* object, void* vo, int type ) );
void    mprog_act_trigger       args ( ( char* buf, CHAR_DATA* mob, CHAR_DATA* ch, OBJ_DATA* obj, void* vo ) );
void    mprog_bribe_trigger     args ( ( CHAR_DATA* mob, CHAR_DATA* ch, int amount ) );
void    mprog_entry_trigger     args ( ( CHAR_DATA* mob ) );
void    mprog_give_trigger      args ( ( CHAR_DATA* mob, CHAR_DATA* ch,  OBJ_DATA* obj ) );
void    mprog_greet_trigger     args ( ( CHAR_DATA* mob ) );
void    mprog_fight_trigger     args ( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_hitprcnt_trigger  args ( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_death_trigger     args ( ( CHAR_DATA* mob, bool corpse ) );
void    mprog_random_trigger    args ( ( CHAR_DATA* mob ) );
void    mprog_speech_trigger    args ( ( char* txt, CHAR_DATA* mob ) );

/* notify.c */
void notify_message args ( ( CHAR_DATA *ch, long type, long to, char *extra_name ) );

/* handler.c */

ARTIFACT *new_artifact( void );
int 	 material_lookup		(const char *name);
char	*material_name		(short num);
int 	 item_lookup		(const char *name);
char	*item_name		(short num);
int 	 weapon_class_lookup	(const char *name);
char	*weapon_class_name	(short num);
int 	 attack_type_lookup	(const char *name);
char	*attack_type_name	(short num);
int 	 liquid_lookup		(const char *name);
char	*liquid_name		(short num);
int 	 dam_type_lookup	(const char *name);
char	*dam_type_name		(short num);
int 	 position_lookup		(const char *name);
char	*position_name		(short num);
int 	 sex_lookup		(const char *name);
char	*sex_name		(short num);
int 	 size_lookup		(const char *name);
char	*size_name		(short num);
int 	check_immune		(CHAR_DATA *ch, int dam_type);
long	material_vuln		(short num);
int	race_lookup		(const char *name);
int	class_lookup		(const char *name);
bool	is_old_mob		(CHAR_DATA *ch);
int	get_sn_for_weapon 	(OBJ_DATA *wield);
int	get_weapon_sn		(CHAR_DATA *ch, bool dual);
void	reset_char		(CHAR_DATA *ch);
int	get_trust		(CHAR_DATA *ch);
int	get_divinity		(CHAR_DATA *ch);
int	get_authority		(CHAR_DATA *ch);
int	get_curr_stat		(CHAR_DATA *ch, int stat);
int 	get_max_train		(CHAR_DATA *ch, int stat);
int	can_carry_n		(CHAR_DATA *ch);
int	can_carry_w		(CHAR_DATA *ch);
bool	is_name			(char *str, char *namelist);
bool    	is_full_name    		(const char *str, char *namelist);
bool 	is_name_abbv 		(char *str, char *namelist);
bool	obj_called		(OBJ_DATA *obj, char *string);
bool	obj_called_abbrev	(OBJ_DATA *obj, char *string);
void	affect_to_char		(CHAR_DATA *ch, AFFECT_DATA *paf);
void	affect_to_obj		(OBJ_DATA *obj, AFFECT_DATA *paf);
void	affect_remove		(CHAR_DATA *ch, AFFECT_DATA *paf);
void	affect_remove_obj 	(OBJ_DATA *obj, AFFECT_DATA *paf );
void	affect_strip		(CHAR_DATA *ch, int sn);
bool	is_affected		(CHAR_DATA *ch, int sn);
void	affect_join		(CHAR_DATA *ch, AFFECT_DATA *paf);
void	char_from_room		(CHAR_DATA *ch);
void	char_to_room		(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex);
void	obj_to_char		(OBJ_DATA *obj, CHAR_DATA *ch);
void	obj_from_char		(OBJ_DATA *obj);
void	obj_to_char_ooz		(OBJ_DATA *obj, CHAR_DATA *ch);
void	obj_from_char_ooz	(OBJ_DATA *obj);
int	apply_ac		(OBJ_DATA *obj, int iWear, int type);
OD *	get_eq_char		(CHAR_DATA *ch, int iWear);
void	equip_char		(CHAR_DATA *ch, OBJ_DATA *obj, int iWear);
void	unequip_char		(CHAR_DATA *ch, OBJ_DATA *obj);
int	count_obj_list		(OBJ_INDEX_DATA *obj, OBJ_DATA *list);
void	obj_from_room		(OBJ_DATA *obj );
void	obj_to_room		(OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex);
void	obj_to_obj		(OBJ_DATA *obj, OBJ_DATA *obj_to);
void	obj_from_obj		(OBJ_DATA *obj);
void	extract_obj		(OBJ_DATA *obj);
void	extract_char		(CHAR_DATA *ch, bool fPull);
CD *	get_char_room_unseen	(CHAR_DATA *ch, char *argument);
CD *	get_char_room		( CHAR_DATA *ch, char *argument);
CD *	get_char_world		( CHAR_DATA *ch, char *argument);
CD *	get_char_world_player	(CHAR_DATA *ch, char *argument );
OD *	get_obj_type		(OBJ_INDEX_DATA *pObjIndexData);
OD *	get_obj_list		(CHAR_DATA *ch, char *argument,  OBJ_DATA *list);
OD *	get_obj_carry		(CHAR_DATA *ch, char *argument);
OD *	get_obj_wear		(CHAR_DATA *ch, char *argument);
OD *	get_obj_here		(CHAR_DATA *ch, char *argument);
OD *	get_obj_world		(CHAR_DATA *ch, char *argument);
OD *	create_money		(int amount, int currency);
OD *	get_obj_room		(ROOM_INDEX_DATA *room, char *argument );
int	get_obj_number		(OBJ_DATA *obj);
int	get_obj_weight		(OBJ_DATA *obj);
bool	room_is_dark		(ROOM_INDEX_DATA *pRoomIndex);
bool	room_is_private		(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex );
bool	can_see			(CHAR_DATA *ch, CHAR_DATA *victim);
bool	can_see_hidden		(CHAR_DATA *ch, CHAR_DATA *victim);
bool	can_see_obj		(CHAR_DATA *ch, OBJ_DATA *obj);
bool	can_see_obj_aura	(CHAR_DATA *ch, OBJ_DATA *obj);
bool	can_see_room		(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex);
bool	can_drop_obj		(CHAR_DATA *ch, OBJ_DATA *obj);
char *	item_type_name		(OBJ_DATA *obj);
char *	affect_loc_name		(int location);
char *	affect_bit_name		(long long vector);
char *	extra_bit_name		(long long extra_flags);
char * 	wear_bit_name		(int wear_flags);
char *	nature_bit_name		(long nature_flags);
char *	act_bit_name		(long long act_flags);
char *	plr_bit_name		(long long plr_flags);
char *	off_bit_name		(int off_flags);
char *  	imm_bit_name		(int imm_flags);
char * 	form_bit_name		(int form_flags);
char *	part_bit_name		(int part_flags);
char *	weapon_bit_name	(int weapon_flags);
char *  	comm_bit_name		(long comm_flags);
void    	pos_text			(CHAR_DATA *ch, char *buf );
char *  	pos_text_short		(CHAR_DATA *ch);
char *  	acv_text_short		(CHAR_DATA *ch);
void    	set_activity		(CHAR_DATA *ch, int new_position, char *new_pos_desc, int new_activity, char *new_acv_desc);
void	set_activity_key		(CHAR_DATA *ch);
bool	check_activity_key	(CHAR_DATA *ch, char *args );
void	clear_activity		(CHAR_DATA *ch );
void	schedule_activity		(CHAR_DATA *ch, int delay, char *cmd );	
void    	affect_to_room  		(ROOM_INDEX_DATA *room, AFFECT_DATA *paf );
void    	affect_remove_room 	(ROOM_INDEX_DATA *room, AFFECT_DATA *paf);
void 	harmful_effect		(void *vo, int level, int target, int effect);
CD*	get_char_area_true	(CHAR_DATA *ch, char *argument, bool subarea);
CD*	get_char_area		(CHAR_DATA *ch, char *argument, bool subarea );
int 	get_age			(CHAR_DATA *ch);
int 	get_char_size		(CHAR_DATA *ch);
CUSTOMER *get_customer	(SHOP_DATA *shop, char *name);
int 	get_cust_rating		(SHOP_DATA *shop, char *name);
void 	improve_rating		(SHOP_DATA *shop, char *name);
void 	obj_fall			(OBJ_DATA *obj, int recursion);
int 	get_mana_rate		(ROOM_INDEX_DATA *room);
int 	get_heal_rate		(ROOM_INDEX_DATA *room);
OD*	find_passport		(CHAR_DATA *ch);
int 	get_criminal_rating	(CHAR_DATA *ch);
void 	add_passport_entry	(CHAR_DATA *ch, int crime);
void 	clear_passport_entry	(CHAR_DATA *ch, int crime);
int 	get_professional_level	(CHAR_DATA *ch, int level);
int 	get_sanity		(CHAR_DATA *ch);

	
/* interp.c */
extern const struct cmd_type   cmd_table [];
void	interpret			(CHAR_DATA *ch, char *argument );
bool	is_number		(char *arg );
int	number_argument	(char *argument, char *arg);
int 	mult_argument   		(char *argument, char *arg);
char *	one_argument		(char *argument, char *arg_first);
void 	gen_wiz_table 		(void);
void 	posmessage		(CHAR_DATA *ch);

DECLARE_DO_FUN( do_olc);
DECLARE_DO_FUN( do_asave);
DECLARE_DO_FUN( do_alist);
DECLARE_DO_FUN( do_resets);
DECLARE_DO_FUN( do_say);
DECLARE_DO_FUN( do_emote);

/* magic.c */
int	skill_lookup	args( ( const char *name ) );
int	slot_lookup	args( ( int slot ) );
bool	saves_spell	args( ( int level, CHAR_DATA *victim ) );
void	obj_cast_spell	args( ( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, char *info));
void	do_dragon_cast	args( ( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim ) );
bool        check_dispel         (int dis_level, CHAR_DATA *victim, int afn);
int	mana_cost	args( ( CHAR_DATA *ch, int spn ) );
ROOM_INDEX_DATA 	*find_telekinesis_room(CHAR_DATA *ch, int dir);
bool 	knows_discipline	(CHAR_DATA *ch, long discipline);
void	write_to_grimoire (CHAR_DATA *ch, char *discipline);

/* save.c */
void	save_char_obj	args( ( CHAR_DATA *ch ) );
bool	load_char_obj	args( ( DESCRIPTOR_DATA *d, char *name ) );
void	save_shop_obj	(void);
void        load_shop_obj     (CHAR_DATA *ch, ROOM_INDEX_DATA *room);
void        save_morgue        (void);
void        save_tree              (void);
OD *      fread_morgue_obj args((FILE *fp, ROOM_INDEX_DATA *room));
void       load_morgue         (ROOM_INDEX_DATA *room);
void       load_tree                (ROOM_INDEX_DATA *room);

/* skills.c */
int 	exp_per_level	args( ( CHAR_DATA *ch, int points ) );

/* special.c */
SF *	spec_lookup	args( ( const char *name ) );
char *  spec_string     args( ( SPEC_FUN *fun ) );      /* OLC */
char *  check_mob_owner(CHAR_DATA *mob);
char *  check_obj_owner(OBJ_DATA *obj);

/* bit.c */
extern const struct flag_type   connect_type[];
extern const struct flag_type   zmask_flags[];
extern const struct flag_type   zone_flags[];
extern const struct flag_type   area_flags[];
extern const struct flag_type   sex_flags[];
extern const struct flag_type   exit_flags[];
extern const struct flag_type   limb_status[];
extern const struct flag_type   limb_name[];
extern const struct flag_type   door_resets[];
extern const struct flag_type   room_flags[];
extern const struct flag_type   sector_flags[];
extern const struct flag_type   type_flags[];
extern const struct flag_type   discipline_type[];
extern const struct flag_type   extra_flags[];
extern const struct flag_type   artifact_flags[];
extern const struct flag_type   imm_auth[];
extern const struct flag_type   wear_flags[];
extern const struct flag_type   act_flags[];
extern const struct flag_type   plr_flags[];
extern const struct flag_type   affect_flags[];
extern const struct flag_type   raffect_flags[];
extern const struct flag_type   apply_flags[];
extern const struct flag_type   wear_loc_strings[];
extern const struct flag_type   wear_loc_flags[];
extern const struct flag_type   weapon_flags[];
extern const struct flag_type   container_flags[];
extern const struct flag_type   liquid_flags[];
extern const struct flag_type   material_type[];
extern const struct flag_type   material_string[];
extern const struct flag_type   form_flags[];
extern const struct flag_type   part_flags[];
extern const struct flag_type   ac_type[];
extern const struct flag_type   size_flags[];
extern const struct flag_type   off_flags[];
extern const struct flag_type   imm_flags[];
extern const struct flag_type   immapp_flags[];
extern const struct flag_type   res_flags[];
extern const struct flag_type   vuln_flags[];
extern const struct flag_type   position_flags[];
extern const struct flag_type   weapon_class[];
extern const struct flag_type   portal_class[];
extern const struct flag_type   trap_class[];
extern const struct flag_type   weapon_type[];
extern const struct flag_type   furniture_class[];
extern const struct flag_type   attack_type[];
extern const struct flag_type   attack_types[];
extern const struct flag_type   damage_type[];
extern const struct flag_type   sector_name[];
extern const struct flag_type   mob_nature[];
extern const struct flag_type   area_climate[];
extern const struct flag_type   time_sub[];
extern const struct flag_type   soc_auth[];
extern const struct flag_type   tree_size[];
extern const struct flag_type   tree_type[];
extern const struct flag_type   photo_state[];
extern const struct flag_type   pool_type[];
extern const struct flag_type   refill_type[];
extern const struct flag_type   figurine_behavior[];
extern const struct flag_type   figurine_type[];
extern const struct flag_type   currency_type[];
extern const struct flag_type   currency_accept[];
extern const struct flag_type   pass_type[];
extern const struct flag_type   crime_type[];
extern const struct flag_type   instrument_type[];

/* update.c */
void	advance_level	(CHAR_DATA *ch);
void	gain_condition	(CHAR_DATA *ch, int iCond, int value);
void	update_handler	(void);
void 	undo_mask	(CHAR_DATA *ch, bool wereok);
void        undo_morf           	(CHAR_DATA *ch, OBJ_DATA *obj);
void 	weather_update	(bool random);
void        auction_update 	(void);
void        olcautosave	(void );

char	*dochat		(char* talker,char *msg,char* target);
void 	make_note 	(const char* board_name, const char *sender, const char *to, const char *subject, const int expire_days, const char *text);

#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
 

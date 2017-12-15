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
#include "interp.h"
#include "wev.h"
#include "mob.h"
#include "profile.h"
#include "affect.h"
#include "olc.h"

bool	check_social		(CHAR_DATA *ch, char *command, char *argument);
int            social_iff		(CHAR_DATA *ch, CHAR_DATA *victim, int cmd);
bool         is_friend		(CHAR_DATA *mob1, CHAR_DATA *mob2);
bool 	expand_aliases 		(CHAR_DATA *ch, char *orig_command, char *final_command);
void 	check_multi_cmd 	(CHAR_DATA *ch, char *orig_cmd, char *final_cmd);
char 	*get_argument		(char *old_argument, int anum);
char 	*one_argument_cap	(char *argument, char *arg_first );
void 	show_wizhelp_ext	(CHAR_DATA *ch);


#define DIV_INDEX 			10
#define DIV_INDEX_IMP 			9
#define DIV_INDEX_GREATER		8
#define DIV_INDEX_GOD			7
#define DIV_INDEX_LESSER		6
#define DIV_INDEX_CREATOR		5
#define DIV_INDEX_HERO		4
#define DIV_INDEX_INVESTIGATOR 	3
#define DIV_INDEX_HUMAN		2
#define DIV_INDEX_NEWBIE		1
#define DIV_INDEX_NONE		0

struct wizcommand_type *wizcommand_table[DIV_INDEX];

struct disable_cmd_type *disable_cmd_list;

void gen_wiz_table (void);

inline int get_div_index(int divinity);

/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2



/*
 * Log-all switch.
 */
bool				fLogAll		= FALSE;



/*
 * Command table.
 */
const	struct	cmd_type	cmd_table	[] =
{
    /*
     * Common movement commands.
     */
    { "north",		do_north,	POS_STANDING,    	0,  LOG_NEVER, 1, 	COMMAND_MOVEMENT},
    { "east",		do_east,		POS_STANDING,	0,  LOG_NEVER, 1, 	COMMAND_MOVEMENT},
    { "south",		do_south,	POS_STANDING,	0,  LOG_NEVER, 1, 	COMMAND_MOVEMENT},
    { "west",		do_west,	POS_STANDING,	0,  LOG_NEVER, 1, 	COMMAND_MOVEMENT},
    { "up",		do_up,		POS_STANDING,	0,  LOG_NEVER, 1, 	COMMAND_MOVEMENT},
    { "down",		do_down,	POS_STANDING,	0,  LOG_NEVER, 1, 	COMMAND_MOVEMENT},
    { "northeast",		do_northeast,	POS_STANDING,    	0,  LOG_NEVER, 1, 	COMMAND_MOVEMENT},
    { "southeast",		do_southeast,	POS_STANDING,	0,  LOG_NEVER, 1, 	COMMAND_MOVEMENT},
    { "southwest",		do_southwest,	POS_STANDING,	0,  LOG_NEVER, 1,	COMMAND_MOVEMENT},
    { "northwest",		do_northwest,	POS_STANDING,	0,  LOG_NEVER, 1, 	COMMAND_MOVEMENT},

    { "neast",		do_northeast,	POS_STANDING,    	0,  LOG_NEVER, 0, 0},
    { "seast",		do_southeast,	POS_STANDING,	0,  LOG_NEVER, 0, 0},
    { "swest",		do_southwest,	POS_STANDING,	0,  LOG_NEVER, 0, 0},
    { "nwest",		do_northwest,	POS_STANDING,	0,  LOG_NEVER, 0, 0},
    { "northe",		do_northeast,	POS_STANDING,    	0,  LOG_NEVER, 0, 0},
    { "southe",		do_southeast,	POS_STANDING,	0,  LOG_NEVER, 0, 0},
    { "southw",		do_southwest,	POS_STANDING,	0,  LOG_NEVER, 0, 0},
    { "northw",		do_northwest,	POS_STANDING,	0,  LOG_NEVER, 0, 0},
    { "noreast",		do_northeast,	POS_STANDING,    	0,  LOG_NEVER, 0, 0},
    { "soueast",		do_southeast,	POS_STANDING,	0,  LOG_NEVER, 0, 0},
    { "souwest",		do_southwest,	POS_STANDING,	0,  LOG_NEVER, 0, 0},
    { "norwest",		do_northwest,	POS_STANDING,	0,  LOG_NEVER, 0, 0},

    { "i",			do_inventory,	POS_DEAD,	 	0,  LOG_NORMAL, 0, 0},
    { "in",			do_in,		POS_STANDING,	0,  LOG_NEVER, 1, 	COMMAND_MOVEMENT},
    { "out",		do_out,		POS_STANDING,	0,  LOG_NEVER, 1, 	COMMAND_MOVEMENT},

    { "currents",		do_currents,	POS_SLEEPING,	 	0,  LOG_NEVER, 0, 0},

    { "flee",		do_flee,		POS_FIGHTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MOVEMENT},
    { "wander",		do_flee,		POS_STANDING,	0,  LOG_NORMAL, 1, 	COMMAND_MOVEMENT},
    { "ride",		do_ride,		POS_STANDING,	0,  LOG_NORMAL, 1, 	COMMAND_MOVEMENT},
    { "drive",		do_drive,	POS_SITTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MOVEMENT},
    { "tame",		do_tame,	POS_STANDING,	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},

    /*
     * Common other commands.
     * Placed here so one and two letter abbreviations work.
     */
    { "at",             		do_at,          	POS_DEAD,       		L6,  LOG_ALWAYS, 1, 	0},
    { "transfer",		do_transfer,	POS_DEAD,		L5,  LOG_ALWAYS, 1, 	IMMAUTH_QUESTOR},
    { "cr",			do_cr,		POS_DEAD,		L8,  LOG_ALWAYS, 1, 	IMMAUTH_QUESTOR},
    { "attack",		do_kill,		POS_STANDING, 	0,  LOG_NORMAL, 0, 0},
    { "buy",		do_buy,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "translate",		do_translate,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "auction",		do_auction,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "appraise",		do_appraise,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "cast",		do_cast,		POS_FIGHTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MAGIC},
    { "induel",		do_magical_duel,	POS_STANDING,	0,  LOG_NORMAL, 0, 0},
    { "duel",		do_duel,		POS_STANDING,	0,  LOG_NORMAL, 1, 	COMMAND_MAGIC},
    { "ritualize",		do_ritual,	POS_STANDING,	0,  LOG_NORMAL, 1, 	COMMAND_MAGIC},
    { "channels",       	do_channels,    	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "debate",         	do_debate,      	POS_STANDING,	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "enter",          		do_enter,       	POS_STANDING,    	0,  LOG_NORMAL, 1, 	COMMAND_MOVEMENT},
    { "exits",		do_exits,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "get",		do_get,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "use",		do_use,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "combine",		do_combine,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "carry",		do_carry,	POS_STANDING,	0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "drag",		do_drag,	POS_STANDING,	0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "cut",		do_cut,		POS_STANDING,	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "voodoo",		do_voodoo,	POS_STANDING,	0,  LOG_NORMAL, 1, 	COMMAND_MAGIC},
    { "goto",           		do_goto,        	POS_DEAD,       		L8,  LOG_NORMAL, 1, 0},
    { "hit",		do_kill,		POS_STANDING, 	0,  LOG_NORMAL, 0, 0},
    { "hours",		do_hours,	POS_RESTING,	 	0,  LOG_NORMAL, 0, 	COMMAND_GENERAL},
    { "inventory",		do_inventory,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "interrogate",		do_interrogate,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "kill",		do_kill,		POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "look",		do_look,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "learn",		do_learn,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "yith",		do_yith,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_SPECIAL},
    { "last", 		do_last,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "music",          	do_music,   	POS_SLEEPING,    	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION}, 
    { "play",            		do_play,   	POS_STANDING,    	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION}, 
    { "sing",            		do_sing,   	POS_STANDING,    	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE}, 
    { "marry",		do_marry,	POS_STANDING,	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "order",		do_order,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "petname",		do_petname,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "store",		do_store,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "fetch",		do_fetch,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "practice",       	do_practice,	POS_SLEEPING,    	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "prof", 		do_prof,		POS_SLEEPING,    	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "rest",		do_rest,		POS_SLEEPING,	 	0,  LOG_NORMAL, 1,	COMMAND_GENERAL},
    { "sit",		do_sit,		POS_SLEEPING,    	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "camp",		do_camp,	POS_STANDING,    	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "restring",		do_restring,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "sc",		do_score,	POS_DEAD,	 	0,  LOG_NORMAL, 0, 0 },
    { "map", 		do_map, 	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "scan", 		do_scan, 	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "sca", 		do_scan, 	POS_RESTING, 	 	0,  LOG_NORMAL, 0, 0},
    { "shoot",		do_kill,		POS_STANDING, 	0,  LOG_NORMAL, 0, 0},
    { "stand",		do_stand,	POS_SLEEPING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "study",		do_study,	POS_SLEEPING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "tell",		do_tell,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "ignore",		do_ignore,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "telepathy",		do_telepathy,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "teach",		do_teach,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "train",		do_train,	POS_STANDING,	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "weight",		do_weigh,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "wield",		do_wield,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "light",		do_light,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "extinguish",		do_extinguish,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "reload",		do_reload,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "wizhelp",		do_wizhelp,	POS_DEAD,		HE,  LOG_NORMAL, 1, 	-1},

    /*
     * Informational commands.
     */
    { "affect",		do_affect,	POS_DEAD,	 	0,  LOG_NORMAL, 0, 0},
    { "affects",		do_affect,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "raffect",		do_room_affect,	POS_DEAD,	 	0,  LOG_NORMAL, 0, 0 },
    { "raffects",		do_room_affect,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "bug",		do_bug,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "ba", 		do_bank,	POS_RESTING, 	 	0,  LOG_NORMAL, 0, 0},
    { "ban",		do_ban,		POS_DEAD,		L2,  LOG_ALWAYS, 1, 	IMMAUTH_ENFORCER},
    { "bank", 		do_bank,	POS_RESTING, 	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "gamble", 		do_gamble,	POS_RESTING, 	 	0,  LOG_NORMAL, 0, 	COMMAND_INTERACTIVE},
    { "commands",		do_commands,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "compare",		do_compare,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "consider",		do_consider,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "count",		do_count,	POS_SLEEPING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "credits",	do_credits,	POS_SLEEPING,		0,  LOG_NORMAL, 1,	COMMAND_GENERAL},
    { "deeds",		do_deeds,	POS_SLEEPING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "quests",		do_quests,	POS_SLEEPING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "equipment",		do_equipment,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "examine",		do_examine,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "glance",		do_glance,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "help",		do_help,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "hlist",          		do_hlist,       	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "list",		do_list,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "listen",		do_listen,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "lore",		do_lore,		POS_STANDING,	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "motd",		do_motd,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "notify", 		do_notify,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "read",		do_read,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "write",		do_write,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "report",		do_report,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "rules",		do_rules,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "score",		do_score,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "body",		do_body,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "scry",		do_scry,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_MAGIC},
    { "show",		do_show,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "skills",		do_skills,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "research",		do_research,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "smell",		do_smell,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "societies",		do_society,	POS_DEAD,	 	0,  LOG_NORMAL, 0, 0},
    { "society",		do_society,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "guards",		do_guard,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_SPECIAL},
    { "socials",		do_socials,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "sockets",        	do_sockets,	POS_DEAD,       		L4,  LOG_NORMAL, 1, 	IMMAUTH_ADMIN},
    { "spells",		do_spells,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_MAGIC},
    { "story",		do_story,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "time",		do_time,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "typo",		do_typo,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "view",		do_view,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "who",		do_who,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "whois",		do_whois,	POS_DEAD,	 	0,  LOG_NORMAL, 1,	COMMAND_GENERAL},
    { "bounty",		do_bounty,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "mission",		do_mission,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "wizlist",		do_wizlist,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "money",		do_worth,	POS_SLEEPING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "worth",		do_worth,	POS_SLEEPING,	 	0,  LOG_NORMAL, 0, 0},

    /*
     * Configuration commands.
     */
    { "autolist",		do_autolist,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "alias",		do_alias,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "unalias",		do_unalias,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "autoassist",		do_autoassist,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "autoexit",		do_autoexit,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "autogold",		do_autogold,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "autoloot",		do_autoloot,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "autosac",		do_autosac,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "autosave",		do_autosave,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "autosplit",		do_autosplit,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "autokill",		do_autokill,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "righteouskill",	do_righteouskill,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "autoconsider",	do_autocon,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "autotax",		do_autotax,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "haggle",		do_haggle,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "brief",		do_brief,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "colour",		do_colour,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "color", 		do_colour,  	POS_DEAD, 	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "autocombine",	do_autocombine,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "compact",		do_compact,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "cursor",		do_cursor,	POS_DEAD,		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "description",	do_description,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "bio",		do_bio,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "delet",		do_delet,	POS_DEAD,	 	0,  LOG_NORMAL, 0, 0 },
    { "delete",		do_delete,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "email",          		do_email,       	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "fullcast",		do_fullcast,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "fullfight",		do_fullfight,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "fullweather",		do_fullweather,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "home",		do_home,	POS_DEAD,		HE,  LOG_NORMAL, 1, 	COMMAND_GENERAL},   
    { "homepage",		do_homepage,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "html",		do_imp_html,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "image",		do_image,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "imp_html",		do_imp_html,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "nofollow",		do_nofollow,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "nosummon",		do_nosummon,	POS_DEAD,       	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "noloot",		do_noloot,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "outfit",		do_outfit,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "password",		do_password,	POS_DEAD,	 	0,  LOG_NEVER,  1, 	COMMAND_GENERAL},
    { "prompt",		do_prompt,	POS_DEAD,        		0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "scroll",		do_scroll,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "title",		do_title,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "worship",		do_worship,	POS_DEAD,	 	0,  LOG_NORMAL, 1 , 	COMMAND_GENERAL},
    { "style",		do_style,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "wimpy",		do_wimpy,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "xinfo",		do_xinfo,	POS_DEAD,		IM,  LOG_NORMAL, 1, 	0},

   /* Conversive commands... */

    { "ask",		do_converse,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "converse",		do_converse,	POS_RESTING,	 	0,  LOG_NORMAL, 1,	COMMAND_INTERACTIVE},
    { "talk",		do_converse,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},

  /* In Character Communication commands... */

    { "emote",		do_emote,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "pose",		do_emote,	POS_RESTING,	 	0,  LOG_NORMAL, 0, 0 },
    { ",",			do_emote,	POS_RESTING,	 	0,  LOG_NORMAL, 0, 0 },
    { "gtell",		do_gtell,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { ";",			do_gtell,		POS_RESTING,	 	0,  LOG_NORMAL, 0, 0 },
    { "say",		do_say,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "ic",			do_say,		POS_RESTING,	 	0,  LOG_NORMAL, 0, 0 },
    { "'",			do_say,		POS_RESTING,	 	0,  LOG_NORMAL, 0, 0 },
    { "scream",		do_scream,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "shout",		do_shout,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "yell",		do_yell,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "whisper",		do_tell,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "dream",		do_dream,	POS_SLEEPING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MAGIC},
    { ">",			do_dream,	POS_SLEEPING,	 	0,  LOG_NORMAL, 0, 0 },

  /* Out Of Character Communication commands... */

    { "answer",		do_answer,	POS_SLEEPING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "question",		do_question,	POS_SLEEPING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "chat",		do_chatter,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "=",			do_chatter,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "gossip",		do_gossip,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "ooc",		do_gossip,	POS_DEAD,	 	0,  LOG_NORMAL, 0, 0 },
    { ".",			do_gossip,	POS_DEAD,	 	0,  LOG_NORMAL, 0, 0 },
    { "herotalk",		do_herotalk,	POS_DEAD,	 	HE,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "+",			do_herotalk,	POS_DEAD,		HE,  LOG_NORMAL, 0, 0},
    { "investigatortalk",	do_investigatortalk,POS_DEAD,	 	IN,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "-",			do_investigatortalk,POS_DEAD,	 	IN,  LOG_NORMAL, 1, 0},
    { "beep",           		do_beep,        	POS_DEAD,      	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "immtalk",		do_immtalk,	POS_DEAD,		HE,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { ":",			do_immtalk,	POS_DEAD,		HE,  LOG_NORMAL, 0, 0},
    { "tell",		do_tell,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "reply",		do_reply,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "ct",			do_clan_tell,	POS_DEAD,   	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "?",			do_clan_tell, 	POS_DEAD,   	 	0,  LOG_NORMAL, 0, 0 },
    { "mtell",		do_mtell,	POS_SLEEPING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "stell",		do_stell,		POS_SLEEPING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},

  /* Communications control commands... */

    { "afk", 		do_afk, 		POS_STANDING,    	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "deaf",		do_deaf,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "quiet",		do_quiet,	POS_SLEEPING, 	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "speak",		do_speak,	POS_SLEEPING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},

  /* Notes and boards... */

    { "boards",		do_board,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},
    { "notes",		do_note,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMMUNICATION},

    /*
     * Object manipulation commands.
     */
    { "brandish",		do_brandish,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MAGIC},
    { "brew",           		do_brew,        	POS_STANDING,    	0,  LOG_NORMAL, 1, 	COMMAND_MAGIC},
    { "scribe",         		do_scribe,      	POS_STANDING,    	0,  LOG_NORMAL, 1, 	COMMAND_MAGIC},
    { "close",		do_close,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "drink",		do_drink,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "drop",		do_drop,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "plant",		do_plant,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "photograph",	do_photograph,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "eat",		do_eat,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "feed",		do_feed,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "envenom",    	do_envenom, 	POS_RESTING,     	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "fill",		do_fill,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "pour",		do_pour,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "give",		do_give,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "hold",		do_hold,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "lock",		do_lock,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "knock",		do_knock,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "open",		do_open,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "smash",		do_smash,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "throw",		do_throw,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "sprinkle",		do_sprinkle,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "offer",		do_offer,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MAGIC},
    { "pick",		do_pick,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "pray",		do_pray,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MAGIC},
    { "put",		do_put,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "quaff",		do_quaff,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MAGIC},
    { "recite",		do_recite,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MAGIC},
    { "remove",		do_remove,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "remember",		do_remember,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MAGIC},
    { "resize",		do_resize,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "repair",		do_repair,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "destroy",		do_destroy,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "rebuild",		do_rebuild,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "search",		do_search,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "conceal",		do_conceal,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "tracks",		do_tracks,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "sell",		do_sell,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "take",		do_get,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "tamp",		do_tamp,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "smoke",		do_smoke,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "sacrifice",		do_sacrifice,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "junk",           		do_sacrifice,   	POS_RESTING,     	0,  LOG_NORMAL, 0, 0 },
    { "unlock",		do_unlock,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "value",		do_value,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "wear",		do_wear,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "zap",		do_zap,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MAGIC},
    { "sharpen", 		do_sharpen,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "fix", 		do_fix,		POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "forge", 		do_forge,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "gunsmith", 		do_gunsmith,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "skin", 		do_skin,		POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "refit", 		do_refit,		POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},

    /*
     * Combat commands.
     */
    { "assassinate",	do_assassinate,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "backstab",		do_backstab,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "blackjack",		do_blackjack,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "bash",		do_bash,	POS_FIGHTING,   	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "bs",		do_backstab,	POS_STANDING, 	0,  LOG_NORMAL, 0, 0 },
    { "berserk",		do_berserk,	POS_FIGHTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "circle",         		do_circle,      	POS_FIGHTING,    	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "crush",          	do_crush,       	POS_FIGHTING,    	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "dual",		do_dual,		POS_FIGHTING,    	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "dirt",		do_dirt,		POS_FIGHTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "disarm",		do_disarm,	POS_FIGHTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "distract",		do_distract,	POS_FIGHTING,   	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "kick",		do_kick,		POS_FIGHTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "strangle",		do_strangle,	POS_FIGHTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "bite",		do_bite,		POS_FIGHTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "murde",		do_murde,	POS_FIGHTING,		0,  LOG_NORMAL, 0, 0 },
    { "murder",		do_murder,	POS_STANDING,	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "rescue",		do_rescue,	POS_FIGHTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "rotate",		do_rotate,	POS_FIGHTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "tail",		do_tail,		POS_FIGHTING,    	0,  LOG_NORMAL, 1,	COMMAND_COMBAT},
    { "trip",		do_trip,		POS_FIGHTING,    	0,  LOG_NORMAL, 1, 	COMMAND_COMBAT},
    { "trap",		do_trap,		POS_STANDING,   	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "bomb",		do_bomb,	POS_STANDING,    	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "intimidate",		do_intimidate,	POS_STANDING,    	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},

    /*
     * Miscellaneous commands.
     */
    { "follow",		do_follow,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "group",		do_group,	POS_SLEEPING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "hide",		do_hide,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MOVEMENT},
    { "qui",		do_qui,		POS_DEAD,	 	0,  LOG_NORMAL, 0, 0 },
    { "quit",		do_quit,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "recall",		do_recall,	POS_FIGHTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_MOVEMENT},
    { "/",			do_recall,	POS_FIGHTING,	 	0,  LOG_NORMAL, 0, 0},
    { "rent",		do_rent,		POS_DEAD,	 	0,  LOG_NORMAL, 0, 0},
    { "lease",		do_lease,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "save",		do_save,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "sleep",		do_sleep,	POS_SLEEPING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "sneak",		do_sneak,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_MOVEMENT},
    { "split",		do_split,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "steal",		do_steal,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "rob",		do_rob,		POS_STANDING, 	0,  LOG_NORMAL, 1,	COMMAND_INTERACTIVE},
    { "visible",	                do_visible,	POS_SLEEPING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "land",	                do_land,		POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "launch",	                do_launch,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "wake",		do_wake,	POS_SLEEPING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},
    { "where",		do_where,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_GENERAL},

   /* Special mob interaction commands... */     

    { "heal",		do_heal,		POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE}, 
    { "therapy",		do_therapy,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE}, 

  /* Player Interaction */
    { "bandage",         	do_bandage,      	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "psychology",        	do_psychology,   POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "vampire", 		do_vampire,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_SPECIAL},
    { "were", 		do_were,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_SPECIAL},
    { "lich", 		do_lich,		POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_SPECIAL},
    { "bind", 		do_bind,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "free", 		do_untie,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},
    { "brainsuck", 		do_brainsuck,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_SPECIAL},
    { "mindtransfer", 	do_mindtransfer,	POS_STANDING, 	0,  LOG_NORMAL, 1, 	COMMAND_SPECIAL},
    { "sermonize",		do_sermonize,	POS_RESTING,	 	0,  LOG_NORMAL, 1, 	COMMAND_INTERACTIVE},

   /* Gadget commands... */

    { "push",	   	do_gadget_push,	POS_STANDING,	 0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "pull",	   	do_gadget_pull,	POS_STANDING,	 0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "twist",	   	do_gadget_twist,	POS_STANDING,	 0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "turn",	   	do_gadget_turn,	POS_STANDING,	 0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "turnback",  		do_gadget_turnback,POS_STANDING,	 0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "move",	   	do_gadget_move,POS_STANDING,	 0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "lift",	   	do_gadget_lift,	POS_STANDING,	 0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "press",	   	do_gadget_press,POS_STANDING,	 0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},
    { "dig",	   	do_gadget_digin,	POS_STANDING,	 0,  LOG_NORMAL, 1, 	COMMAND_MANIPULATION},

   /* Implementors */

    { "advance",	                do_advance,	POS_DEAD,		ML,  LOG_ALWAYS, 1, IMMAUTH_ADMIN},
    { "shell",	                do_shell,	POS_DEAD,		ML,  LOG_ALWAYS, 1, IMMAUTH_ADMIN },
    { "trust",		do_trust,	POS_DEAD,		ML,  LOG_ALWAYS, 1, IMMAUTH_ADMIN },
    { "authorize",		do_authorize,	POS_DEAD,		ML,  LOG_ALWAYS, 1, 0},
    { "zcopy",		do_zcopy,	POS_DEAD,		ML,  LOG_ALWAYS, 1, IMMAUTH_BUILDER},
    { "zclear",		do_zclear,	POS_DEAD,		ML,  LOG_ALWAYS, 1, IMMAUTH_BUILDER },
    { "amerge",		do_amerge,	POS_DEAD,		ML,  LOG_ALWAYS, 1, IMMAUTH_BUILDER },
    { "arebase",	                do_arebase,	POS_DEAD,		ML,  LOG_ALWAYS, 1, IMMAUTH_BUILDER },
    { "asplit",		do_asplit,	POS_DEAD,		ML,  LOG_ALWAYS, 1, IMMAUTH_BUILDER },
    { "asuck",		do_asuck,	POS_DEAD,		ML,  LOG_ALWAYS, 1, IMMAUTH_BUILDER },
    { "clone",		do_clone,	POS_DEAD,		ML,  LOG_ALWAYS, 1, 0 },
    { "dump", 		do_dump,	POS_DEAD,		ML,  LOG_ALWAYS, 0, 0 },
    { "permapk",	                do_permapk,	POS_DEAD,		ML,  LOG_ALWAYS, 1, IMMAUTH_ADMIN },
    { "pload",		do_pload,	POS_DEAD,		ML,  LOG_ALWAYS, 1, IMMAUTH_ADMIN },
    { "punload",		do_punload,	POS_DEAD,		ML,  LOG_ALWAYS, 1, IMMAUTH_ADMIN },
    { "cleanup",		do_cleanup,	POS_DEAD,		ML,  LOG_ALWAYS, 1, IMMAUTH_ADMIN },

   /* Greater */

    { "award",		do_award,	POS_DEAD,		L1,  LOG_ALWAYS, 1, IMMAUTH_QUESTOR },
    { "deny",		do_deny,	POS_DEAD,		L1,  LOG_ALWAYS, 1, IMMAUTH_ADMIN },
    { "reboo",		do_reboo,	POS_DEAD,		L1,  LOG_NORMAL, 0, 0 },
    { "reboot",		do_reboot,	POS_DEAD,		L1,  LOG_ALWAYS, 1, IMMAUTH_ADMIN },
    { "shutdow",	                do_shutdow,	POS_DEAD,		L1,  LOG_NORMAL, 0, 0 },
    { "shutdown",	                do_shutdown,	POS_DEAD,		L1,  LOG_ALWAYS, 1, IMMAUTH_ADMIN },
    { "log",		do_log,		POS_DEAD,		L1,  LOG_ALWAYS, 1, IMMAUTH_ENFORCER },
    { "set",		do_set,		POS_DEAD,		L2,  LOG_ALWAYS, 1, IMMAUTH_QUESTOR },
    { "allow",		do_allow,	POS_DEAD,		L2,  LOG_ALWAYS, 1, IMMAUTH_ENFORCER },
    { "wizlock",	                do_wizlock,	POS_DEAD,		L2,  LOG_ALWAYS, 1, IMMAUTH_ADMIN },
    { "disconnect",	                do_disconnect,	POS_DEAD,		L3,  LOG_ALWAYS, 1, IMMAUTH_ADMIN },
    { "connect",	                do_connect,	POS_DEAD,		L3,  LOG_ALWAYS, 1, IMMAUTH_ADMIN },
    { "freeze",		do_freeze,	POS_DEAD,		L3,  LOG_ALWAYS, 1, IMMAUTH_ENFORCER },
    { "newlock",	                do_newlock,	POS_DEAD,		L3,  LOG_ALWAYS, 1, IMMAUTH_ADMIN },
    { "sla",		do_sla,		POS_DEAD,		L3,  LOG_NORMAL, 0, 0 },
    { "slay",		do_slay,	                POS_DEAD,		L3,  LOG_ALWAYS, 1, IMMAUTH_ENFORCER },
    { "pardon",		do_pardon,	POS_DEAD,		L3,  LOG_ALWAYS, 1, IMMAUTH_ENFORCER },
    { "distribute",		do_distribute,	POS_DEAD,		L3,  LOG_ALWAYS, 1, IMMAUTH_QUESTOR },

   /* God */

    { "remort",	                do_remort,	POS_DEAD,		L4,  LOG_ALWAYS, 1, 0 },
    { "refresh",	                do_refresh,	POS_DEAD,		L4,  LOG_ALWAYS, 1, IMMAUTH_ADMIN },
    { "become",		do_become,	POS_DEAD,		L3,  LOG_ALWAYS, 1, -1 },
    { "check",		do_check,	POS_DEAD,		L4,  LOG_ALWAYS, 1, IMMAUTH_ADMIN },
    { "enable",		do_enable,	POS_DEAD,		L4,  LOG_ALWAYS, 1, 0 },
    { "disable",		do_disable,	POS_DEAD,		L4,  LOG_ALWAYS, 1, 0 },
    { "pecho",		do_pecho,	POS_DEAD,		L4,  LOG_ALWAYS, 1, IMMAUTH_QUESTOR }, 
    { "restore",		do_restore,	POS_DEAD,		L4,  LOG_ALWAYS, 1, IMMAUTH_QUESTOR },
    { "gecho",		do_echo,	POS_DEAD,		L4,  LOG_ALWAYS, 1, IMMAUTH_QUESTOR },
    { "vnum",		do_vnum,	POS_DEAD,		L4,  LOG_ALWAYS, 1, 0 },
    { "rename",		do_rename,	POS_DEAD,		L4,  LOG_ALWAYS, 1, IMMAUTH_ADMIN },
    { "nochannels",	do_nochannels,	POS_DEAD,		L5,  LOG_ALWAYS, 1, IMMAUTH_ENFORCER },
    { "noport",		do_noport,	POS_DEAD,		L5,  LOG_ALWAYS, 1, IMMAUTH_ENFORCER },
    { "noemote",		do_noemote,	POS_DEAD,		L5,  LOG_ALWAYS, 1, IMMAUTH_ENFORCER },
    { "noshout",		do_noshout,	POS_DEAD,		L5,  LOG_ALWAYS, 1, IMMAUTH_ENFORCER },
    { "notell",		do_notell,	POS_DEAD,		L5,  LOG_ALWAYS, 1, IMMAUTH_ENFORCER },
    { "teleport",		do_transfer,    	POS_DEAD,		L5,  LOG_ALWAYS, 1, IMMAUTH_QUESTOR },	
    { "snoop",		do_snoop,	POS_DEAD,		L5,  LOG_ALWAYS, 1, IMMAUTH_ADMIN },
    { "string",		do_string,	POS_DEAD,		L5,  LOG_ALWAYS, 1, IMMAUTH_ADMIN },
    { "load",		do_load,		POS_DEAD,		L6,  LOG_ALWAYS, 1, IMMAUTH_QUESTOR },
    { "echo",		do_recho,	POS_DEAD,		L6,  LOG_ALWAYS, 1, IMMAUTH_QUESTOR },
    { "setannoying",             	do_setannoying, 	POS_DEAD,       		L6,  LOG_ALWAYS, 1, 0 },

   /* Lesser */

    { "devour",		do_devour,	POS_DEAD,		L7,  LOG_ALWAYS, 1, 0 },
    { "idoltalk",		do_idoltalk,	POS_DEAD,		L7,  LOG_ALWAYS, 1, 0 },
    { "incarnate",		do_incarnate,	POS_DEAD,		L7,  LOG_ALWAYS, 1, IMMAUTH_QUESTOR },
    { "force",		do_force,	POS_DEAD,		L7,  LOG_ALWAYS, 1, IMMAUTH_QUESTOR },
    { "forceremove",              do_forceremove,	POS_DEAD,		L7,  LOG_ALWAYS, 1, IMMAUTH_QUESTOR },
    { "return",                         do_return,      	POS_DEAD,        		0,  LOG_ALWAYS, 1, COMMAND_GENERAL},
    { "switch",		do_switch,	POS_DEAD,		L7,  LOG_ALWAYS, 1, IMMAUTH_QUESTOR },
    { "world",		do_world,	POS_DEAD,       		L7,  LOG_ALWAYS, 1, 0 },
    { "setlease",	                do_setlease,	POS_DEAD,		L7,  LOG_NORMAL, 1, IMMAUTH_BUILDER },
    { "checklease",	                do_checklease,	POS_DEAD,		L7,  LOG_NORMAL, 1, IMMAUTH_BUILDER },
    { "chunk",	                do_chunk,	POS_DEAD,		L7,  LOG_ALWAYS, 1, IMMAUTH_BUILDER },

   /* Creators */

    { "edit",                             do_olc,         	POS_DEAD,    		L8,  LOG_NORMAL, 1, IMMAUTH_BUILDER },
    { "asave",                         do_asave,       	POS_DEAD,    		L8,  LOG_NORMAL, 1, 0 },
    { "resets",                         do_resets,      	POS_DEAD,    		L8,  LOG_NORMAL, 1, IMMAUTH_BUILDER },
    { "purge",		do_purge,	POS_DEAD,		L8,  LOG_ALWAYS, 1, IMMAUTH_BUILDER },
    { "ainfo",                           do_ainfo,       	POS_DEAD,    		L8,  LOG_NORMAL, 1, 0 },
    { "poofin",		do_bamfin,	POS_DEAD,		L8,  LOG_NORMAL, 1, 0 },
    { "poofout",	                do_bamfout,	POS_DEAD,		L8,  LOG_NORMAL, 1, 0 },
    { "peace",		do_peace,	POS_DEAD,		L8,  LOG_NORMAL, 1, 0 },
    { "holylight",      	do_holylight,	POS_DEAD,		IM,  LOG_NORMAL, 1, 0 },
    { "invis",		do_invis,	POS_DEAD,		IM,  LOG_NORMAL, 0, 0 },
    { "cloak",		do_cloak,	POS_DEAD,		IM,  LOG_NORMAL, 1, 0 },
    { "memory",		do_memory,	POS_DEAD,		IM,  LOG_NORMAL, 1, IMMAUTH_ADMIN },
    { "mwhere",		do_mwhere,	POS_DEAD,		IM,  LOG_NORMAL, 1, 0 },
    { "mlevel",		do_mlevel,	POS_DEAD,		IM,  LOG_NORMAL, 1, 0 },
    { "owhere",		do_owhere,	POS_DEAD,		IM,  LOG_NORMAL, 1, 0 },
    { "olevel",		do_olevel,	POS_DEAD,		IM,  LOG_NORMAL, 1, 0 },
    { "pwhere",		do_pwhere,	POS_DEAD,		IM,  LOG_NORMAL, 1, 0 },
    { "stat",		do_stat,      	POS_DEAD,		IM,  LOG_NORMAL, 1, 0 },
    { "wizinvis",	                do_invis,	POS_DEAD,		IM,  LOG_NORMAL, 1, 0 },
    { "wiznet",		do_wiznet,	POS_DEAD,		IM,  LOG_NORMAL, 1, 0 },
    { "queue",		do_queue,	POS_DEAD,		IM,  LOG_NORMAL, 1, 0 },
    { "version",		do_version,	POS_DEAD,		IM,  LOG_NORMAL, 1, 0 },

   /* Heroes */

    { "immtitle",	                do_immtitle,	POS_DEAD,		HE,  LOG_NORMAL, 1, COMMAND_GENERAL},
    { "imotd",                          do_imotd,             	POS_DEAD,       		HE,  LOG_NORMAL, 1, COMMAND_GENERAL },
    { "gag",		do_nochannels,	POS_DEAD,		HE,  LOG_ALWAYS, 1, 0 },
    { "quitannoying",            do_quitannoying,POS_STANDING,	HE,  LOG_ALWAYS, 1, 0 },
    { "build",                           do_build,              	POS_STANDING,	HE,  LOG_NORMAL, 1, COMMAND_INTERACTIVE},
    { "shop",                           do_shop,              	POS_RESTING,    	HE,  LOG_NORMAL, 1, COMMAND_INTERACTIVE},
    { "approve",                     	do_approve,         	POS_DEAD,		HE,  LOG_NORMAL, 1, COMMAND_GENERAL},
    { "disapprove",            	do_disapprove,   	POS_DEAD,    		HE,  LOG_ALWAYS, 1, COMMAND_GENERAL},


   /* Mortals, Newbies even... */
     
    { "alist",          		do_alist,       	POS_DEAD,    	 	0,  LOG_NORMAL, 1, COMMAND_GENERAL},
    { "zlist",          		do_zlist,       	POS_DEAD,    	 	0,  LOG_NORMAL, 1, COMMAND_GENERAL},
    { "end",		do_end,		POS_DEAD,    	 	0,  LOG_NORMAL, 1, COMMAND_GENERAL},

    /*
     * MOBprogram commands.
     */
    { "mpasound", 	    	do_mpasound,    POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpjunk",       	do_mpjunk,      	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpecho",        	do_mpecho,      	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpechoat",     	do_mpechoat,    	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpechoaround",   	do_mpechoaround,POS_DEAD,       0,  LOG_NORMAL, 0, 0 },
    { "mpkill",         		do_mpkill      ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpmload",        	do_mpmload     ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpoload",        	do_mpoload     ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mppurge",        	do_mppurge     ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpgoto",         	do_mpgoto      ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpat",           		do_mpat        ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mptransfer",     	do_mptransfer  ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpforce",        	do_mpforce     ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpsanity",		do_mpsanity    ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpscript",		do_mpscript    ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpreward",		do_mpreward    ,	POS_DEAD,        	0,  LOG_ALWAYS, 0, 0 },
    { "mpfame",		do_mpfame    ,	POS_DEAD,        	0,  LOG_ALWAYS, 0, 0 },
    { "mpconv",		do_mpconv      ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpdeed",		do_mpdeed      ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpquest",		do_mpquest     ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpstop",		do_mpstop      ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mphurt",		do_mphurt      ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpwound",		do_mpwound      ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpselect",		do_mpselect    ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpalign",		do_mpalign     ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mprelev",		do_mprelev     ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpeffect",		do_mpeffect    ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpmemory",		do_mpmemory    ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mphunt",		do_mphunt      ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpsteal",		do_mpsteal     ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mppet",		do_mppet     ,	POS_STANDING,0,  LOG_NORMAL, 0, 0 },
    { "mpinvasion",	do_mpinvasion,	POS_RESTING,    0,  LOG_NORMAL, 0, 0 },
    { "mpemergency",	do_mpemergency,POS_RESTING,    0,  LOG_NORMAL, 0, 0 },
    { "mpreset",		do_mpreset,	POS_DEAD,   	 0,  LOG_NORMAL, 0, 0 },
    { "mposet",		do_mposet     ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpmset",		do_mpmset     ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpstate",		do_mpstate     ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpartifact",		do_mpartifact     ,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mppay",		do_mppay,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mpnote",		do_mpnote,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mppassport",	do_mppassport,	POS_DEAD,        	0,  LOG_NORMAL, 0, 0 },
    { "mppeace",		do_mppeace,	POS_FIGHTING, 	0,  LOG_NORMAL, 0, 0 },
    { "mpanswer",		do_mpanswer,	POS_RESTING, 	0,  LOG_NORMAL, 0, 0 },
    { "mppktimer",		do_mppktimer,	POS_DEAD, 	0,  LOG_NORMAL, 0, 0 },

    /*
     * MUDprogram commands.
     */
    { "mudreply",       	do_mudreply,      	POS_DEAD,        	0, LOG_NORMAL, 0, 0 },
    { "mudprompt",       	do_mudprompt,   	POS_DEAD,        	0, LOG_NORMAL, 0, 0 },
    { "mudbug",       	do_mudbug,   	POS_DEAD,        	0, LOG_NORMAL, 0, 0 },
    { "mudping",       	do_mudping,   	POS_DEAD,        	0, LOG_NORMAL, 0, 0 },
    { "mudversion",    	do_mudversion,   POS_DEAD,        	0, LOG_NORMAL, 0, 0 },
    { "mudwho",       	do_mudwho,     	POS_DEAD,        	0, LOG_NORMAL, 0, 0 },
    { "mudwhois",       	do_mudwhois,     	POS_DEAD,        	0, LOG_NORMAL, 0, 0 },
    { "mudgossip",       	do_mudgossip,    	POS_DEAD,        	0, LOG_NORMAL, 0, 0 },
    { "mudimmtalk",       	do_mudimmtalk, 	POS_DEAD,        	0, LOG_NORMAL, 0, 0 },
    { "mudhero",       	do_mudhero, 	POS_DEAD,        	0, LOG_NORMAL, 0, 0 },
    { "mudnoteprepare",      	do_note_prepare, POS_DEAD,	0, LOG_NORMAL, 0, 0 },
    { "mudnotesubject",      	do_note_subject,	POS_DEAD,	0, LOG_NORMAL, 0, 0 },
    { "mudnoteline",      	do_note_line, 	POS_DEAD,	0, LOG_NORMAL, 0, 0 },
    { "mudnotestop",      	do_note_stop, 	POS_DEAD,	0, LOG_NORMAL, 0, 0 },
    { "mudplayerexists",      	do_mudplayerexists, POS_DEAD,	0, LOG_NORMAL, 0, 0 },
    { "mudreturn",      	do_mudreturn, 	POS_DEAD,	0, LOG_NORMAL, 0, 0 },
    { "muddelete",      	do_muddelete, 	POS_DEAD,	0, LOG_NORMAL, 0, 0 },
    { "muddescriptor",      	do_muddescriptor,POS_DEAD,	0, LOG_NORMAL, 0, 0 },
    { "mudhome",      	do_mudhome,	POS_DEAD,	0, LOG_NORMAL, 0, 0 },
    { "mudtransfer",      	do_mudtransfer,	POS_DEAD,	0, LOG_NORMAL, 0, 0 },


    /*
     * End of list.
     */
    { "",		0,		POS_DEAD,	 	0,  LOG_NORMAL, 0, 0 }
};




/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA *ch, char *argument ){
    char command[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
    char new_argument[MAX_STRING_LENGTH];	
    char new2_argument[MAX_STRING_LENGTH];	
    FILE *fp;
    int depth = 0;
    bool expand = TRUE; 
    int cmd;
    bool found;
    bool alias_cmd=FALSE;
    char *old_argument;
    char *work_argument;
    char arg[MAX_INPUT_LENGTH];

    CHAR_DATA *chk_ch;
    WEV *wev;
     MOB_CMD_CONTEXT *mcc;

    if (IS_AFFECTED(ch, AFF_MORF)) {
       if (!ch->trans_obj) {
           affect_strip_afn( ch, gafn_morf);
           REMOVE_BIT( ch->affected_by, AFF_MORF );
       } else {
           if (ch->trans_obj->in_room != NULL) ch->in_room = ch->trans_obj->in_room;
           if (ch->trans_obj->carried_by != NULL) {
                 if (ch->trans_obj->carried_by->in_room) ch->in_room = ch->trans_obj->carried_by->in_room;
           }
       }
    }

    /*
     * Strip leading spaces.
     */
    while ( isspace(*argument)) argument++;
    if ( argument[0] == '\0' )	return;

   /* check for alias command, if so, bypass multi command and alias
	expansion functions -- allows embedded multi command aliasing */

    one_argument (argument, command);

    if (!str_cmp (command, "alias"))  alias_cmd = TRUE;

    /*
     * Implement freeze command.
     */
    if ( !IS_NPC(ch) && IS_SET(ch->plr, PLR_FREEZE) )    {
	send_to_char( "You're totally frozen!\r\n", ch );
	return;
    }

    if (IS_SET(ch->plr, PLR_AFK)) REMOVE_BIT(ch->plr, PLR_AFK);

/* Zeran - call check_multi_cmd and expand_aliases for any PC who
	is not switched and argument is ch->desc->incomm.  Prevents
	flooding of commands with the force or order command against
	other PCs.  Continue expansion until a max depth of 2
	is reached, or no more alias expansions have occured. */

                old_argument =str_dup(argument);

	if (!IS_NPC(ch) && !alias_cmd && ch->desc && argument == ch->desc->incomm ) {
                      OBJ_DATA *obj;
	      while ( depth < 3 && expand ) {
		
		new_argument[0]='\0';
		check_multi_cmd (ch, argument, new_argument);
		argument = new_argument;

		if (ch->pcdata->has_alias) {
			new2_argument[0]='\0';
			expand = expand_aliases (ch, argument, new2_argument);
			if (expand) ch->wait+=2; 
			argument = new2_argument;
		} else {
			break; 
                	}
 	        	depth++;
                      }

                      if (ch->pcdata->has_alias) {
                            char * rarg;
                            char comp[MAX_INPUT_LENGTH];
                            bool stop_it = FALSE;
                            
                            work_argument = one_argument_cap(argument, arg);
                            new_argument[0]='\0';

                            while (!stop_it) {
                                rarg = strdup(arg);
    
                                if (arg[0] == '@' && isdigit(arg[1])) {
                                     int anum;

                                     arg[0] = ' ';
                                     anum =atoi(arg);
                                     rarg = get_argument(old_argument, anum);
                                }

                                if (arg[0] == '@' && arg[1] == 'w' && arg[2] == 'p') {
                                     char fn[MAX_INPUT_LENGTH];
                                     if ((obj = get_eq_char(ch, WEAR_WIELD )) != NULL) {
                                           one_argument(obj->name, fn);
                                           rarg = str_dup(fn);
                                     }
                                }

                                if (arg[0] == '@' && arg[1] == 'w' && arg[2] == '2') {
                                     char fn[MAX_INPUT_LENGTH];
                                     if ((obj = get_eq_char(ch, WEAR_WIELD2 )) != NULL) {
                                           one_argument(obj->name, fn);
                                           rarg = str_dup(fn);
                                     }
                                }

                                if (rarg) {
                                    one_argument_cap(rarg, comp);
                                    if (str_cmp(rarg, comp)) {
                                         sprintf(comp, "'%s'",rarg);
                                         rarg = str_dup(comp);
                                    }
                                }

                                if (work_argument[0] == '\0') stop_it = TRUE;
                                work_argument = one_argument_cap(work_argument, arg);
                                strcpy( new2_argument, new_argument );
                                if (new2_argument[0] == '\0') sprintf(new_argument, "%s", rarg);
                                else sprintf(new_argument, "%s %s", new2_argument, rarg);
                            }
                            argument = strdup(new_argument);

                            if (ch->desc->incomm[0] != '\0') {
                                 char * trans;
                                 stop_it = FALSE;
            
                                 trans = strdup(ch->desc->incomm);
                                 work_argument = one_argument_cap(trans, arg);
                                 new_argument[0]='\0';

                                 while (!stop_it) {
                                      rarg = strdup(arg);
    
                                     if (arg[0] == '@' && isdigit(arg[1])) {
                                         int anum;

                                         arg[0] = ' ';
                                         anum =atoi(arg);
                                         rarg = get_argument(old_argument, anum);
                                     } 

                                      if (arg[0] == '@' && arg[1] == 'w' && arg[2] == 'p') {
                                         char fn[MAX_INPUT_LENGTH];
                                         if ((obj = get_eq_char(ch, WEAR_WIELD )) != NULL) {
                                               one_argument(obj->name, fn);
                                               rarg = str_dup(fn);
                                         }
                                    }

                                    if (arg[0] == '@' && arg[1] == 'w' && arg[2] == '2') {
                                         char fn[MAX_INPUT_LENGTH];
                                         if ((obj = get_eq_char(ch, WEAR_WIELD2 )) != NULL) {
                                               one_argument(obj->name, fn);
                                               rarg = str_dup(fn);
                                         }
                                    }

                                    if (rarg) {
                                        one_argument_cap(rarg, comp);
                                        if (str_cmp(rarg, comp)) {
                                             sprintf(comp, "'%s'",rarg);
                                             rarg = str_dup(comp);
                                        }
                                    }

                                     if (work_argument[0] == '\0') stop_it = TRUE;
                                     work_argument = one_argument_cap(work_argument, arg);
                                     strcpy( new2_argument, new_argument );
                                     if (new2_argument[0] == '\0') sprintf(new_argument, "%s", rarg);
                                     else sprintf(new_argument, "%s %s", new2_argument, rarg);
                                 }
                                 trans = strdup(new_argument);
                                 sprintf(ch->desc->incomm, "%s", trans);
                            }

                      }

              }
 

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */

    strcpy( logline, argument );
  
     fp = fopen(LASTCOMMAND_FILE, "w");
     fprintf(fp, "%s => %s~\n",ch->name, logline );
     fclose(fp);

     if (!IS_NPC(ch)) {
          if (ch->pcdata->macro) {
              char scriptbuf[MAX_STRING_LENGTH];
              char *trans;

              sprintf(scriptbuf, "addl %d %d %d 0 %s", ch->pcdata->macro_script, ch->pcdata->macro_line, ch->pcdata->macro_delay, logline);
              trans = strdup(scriptbuf);
              medit_script(ch, trans);
              ch->pcdata->macro_line += 10;
          }
     }

    if ( !isalpha(argument[0]) && !isdigit(argument[0]) )  {
	command[0] = argument[0];
	command[1] = '\0';
	argument++;
	while ( isspace(*argument))    argument++;
    } else {
	argument = one_argument( argument, command );
    }

   /* Switched characters use their original authority... */
    
    if ( ch->desc != NULL
      && ch->desc->original != NULL ) {
      chk_ch = ch->desc->original;
    } else {
      chk_ch = ch;
    }

   /* Look for command in command table... */
    
    found = FALSE;

    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )    {
	if ( command[0] == cmd_table[cmd].name[0]
	&&   !str_prefix( command, cmd_table[cmd].name )
	&&   (cmd_table[cmd].div <= get_divinity(chk_ch) || cmd_table[cmd].cclass == -1))	{

                    if (cmd_table[cmd].cclass > -1) {
                         if (chk_ch->pcdata) {
                              if (chk_ch->pcdata->moddiv > DIV_LESSER) {
                                    found = TRUE;
                                    break;
                              }
                         }
                    }
                    if (cmd_table[cmd].div > DIV_HERO
                    && cmd_table[cmd].cclass > 0
                    && !IS_SET(get_authority(chk_ch), cmd_table[cmd].cclass)) continue;
	    found = TRUE;
	    break;
	}
    }

    /*
     * Log and snoop.
     */
    if ( cmd_table[cmd].log == LOG_NEVER ) strcpy( logline, "" );

    if ( ( !IS_NPC(ch) && IS_SET(ch->plr, PLR_LOG) )
    ||   fLogAll
    ||   cmd_table[cmd].log == LOG_ALWAYS ) {
	sprintf( log_buf, "Log %s: %s", ch->name, logline );
	log_string( log_buf );
    }
	
    if (  !IS_NPC(ch) && IS_SET(ch->plr, PLR_LOG) )	{
		sprintf( log_buf, "Log %s: %s", ch->name, logline );
		notify_message (ch, WIZNET_SECURE, TO_IMM_ADMIN, log_buf);
    }

    if ( ch->desc != NULL && ch->desc->snoop_by != NULL )    {
	write_to_buffer( ch->desc->snoop_by, "% ",    2 );
	write_to_buffer( ch->desc->snoop_by, logline, 0 );
	write_to_buffer( ch->desc->snoop_by, "\r\n",  2 );
    }

/*  Zeran - Now, check against disabled commands.  This should be done
			after logging so that disabled commands still register 
			for tracking repeat hackers/offenders. */
	{
	struct disable_cmd_type *tmp;
	
 	for (tmp = disable_cmd_list; tmp != NULL ; tmp=tmp->next)
		{
		if ( (!str_prefix(command, tmp->name) || cmd_table[cmd].do_fun == tmp->disable_fcn) 
			&& get_divinity(ch) < tmp->div)
			{
			send_to_char ("This command has been disabled by the staff, sorry for the inconvenience.\r\n",ch);
			return;
			}
		}
	} /* end checking for disabled command */

    if ( !found )
    {
	/*
	 * Look for command in socials table.
	 */
	if ( !check_social( ch, command, argument ) ) {

                    if (!IS_NPC(ch)) {
                        mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, logline);
                        wev = get_wev(WEV_INTERPRET, WEV_INTERPRET_STRANGE, mcc,
                        "Trying to interpret a {mstrange{x command.\r\n",
                         NULL,
                         NULL);

                         if (!room_issue_wev_challange(get_room_index(mud.monitor), wev)) {
                              free_wev(wev);
                              return;
                         }

                          free_wev(wev);
                     }
       	     send_to_char( "Type {YCOMMANDS{x for a list of valid commands.\r\n", ch );
                 }
                 return;
    }

    /*
     * Character not in position for command?
     */

    if (IS_AFFECTED(ch, AFF_MORF)
    && POS_SLEEPING < cmd_table[cmd].position) {
            send_to_char( "You can't do that during metamorphosis.\r\n", ch );
            return;
    }

    if ( ch->position < cmd_table[cmd].position) {
                posmessage(ch);
	return;
    }

    /*
     * Dispatch the command.
     */
    (*cmd_table[cmd].do_fun) ( ch, argument );

    tail_chain( );
    return;
}


void posmessage(CHAR_DATA *ch) {

    	switch( ch->position )	{
	case POS_DEAD:
	    send_to_char( "Lie still; you are DEAD.\r\n", ch );
	    break;

	case POS_MORTAL:
	case POS_INCAP:
	    send_to_char( "You are hurt far too bad for that.\r\n", ch );
	    break;

	case POS_STUNNED:
                    if ( IS_SET(ch->act,ACT_BOUND)
                    && ch->hit>0
                    && ch->move>0) {
                          send_to_char( "You are gagged and bound.\r\n", ch );
                    } else {
                          send_to_char( "You are too stunned to do that.\r\n", ch );
                    }
	    break;

	case POS_SLEEPING:
	    send_to_char( "In your dreams, or what?\r\n", ch );
	    break;

	case POS_RESTING:
	    send_to_char( "Nah... You feel too relaxed...\r\n", ch);
	    break;

	case POS_SITTING:
	    send_to_char( "Better stand up first.\r\n",ch);
	    break;

	case POS_FIGHTING:
	    send_to_char( "No way!  You are still fighting!\r\n", ch);
	    break;

	}
                return;
}


bool check_social( CHAR_DATA *ch, char *command, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;
    int subwev;

    WEV *wev;
    MOB_CMD_CONTEXT *mcc;

    char msg_actor[MAX_STRING_LENGTH];
    char msg_victim[MAX_STRING_LENGTH];
    char msg_observer[MAX_STRING_LENGTH];

    char *mactor;
    char *mvictim;
    char *mobserver;

    found  = FALSE;

    for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ ) {
	if ( command[0] == social_table[cmd].name[0]
	&&   !str_prefix( command, social_table[cmd].name ) ) {
	    found = TRUE;
	    break;
	}
    }

    if ( !found ) return FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) ) {
	send_to_char( "You are anti-social!\r\n", ch );
	return TRUE;
    }

    switch ( ch->position ) {

      case POS_DEAD:
	send_to_char( "Lie still; you are DEAD.\r\n", ch );
	return TRUE;

      case POS_INCAP:
      case POS_MORTAL:
	send_to_char( "You are hurt far too bad for that.\r\n", ch );
	return TRUE;

      case POS_STUNNED:
	send_to_char( "You are too stunned to do that.\r\n", ch );
	return TRUE;

      case POS_SLEEPING:
	if ( !str_cmp( social_table[cmd].name, "snore" ) ) {
	    break;
               }
	send_to_char( "In your dreams, or what?\r\n", ch );
	return TRUE;

    }

   /* See if there is a victim... */

    one_argument( argument, arg );
    victim = NULL;

   /* Nope, solo command... */
    if ( arg[0] == '\0' ) {
     /* Build the wev... */

      subwev = social_iff(ch, NULL, cmd);
      mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, command);

      if (social_table[cmd].char_no_arg != NULL) {
        sprintf(msg_actor, "%s\r\n", social_table[cmd].char_no_arg);
        mactor = msg_actor;
      } else {
        mactor = NULL;
      }

      if (social_table[cmd].others_no_arg != NULL) {
        sprintf(msg_observer, "%s\r\n", social_table[cmd].others_no_arg);
        mobserver = msg_observer;
      } else {
        mobserver = NULL;
      }

      wev = get_wev(WEV_SOCIAL, subwev, mcc, mactor, NULL, mobserver);

     /* Check for permission... */     

      if (!room_issue_wev_challange(ch->in_room, wev)) {
        free_wev(wev);
        return TRUE;
      }

     /* Issue the wev... */

      room_issue_wev(ch->in_room, wev);
      free_wev(wev);
      return TRUE;
    }

   /* Go find the victim... */

    victim = get_char_room( ch, arg );
    if ( victim == NULL ) {
	send_to_char( "They aren't here.\r\n", ch );
                return TRUE;
    }

   /* Ah, it's me! */

    if ( victim == ch ) {

     /* Build the wev... */

      subwev = social_iff(ch, victim, cmd);
      mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, command);

      if (social_table[cmd].char_auto != NULL) {
        sprintf(msg_actor, "%s\r\n", social_table[cmd].char_auto);
        mactor = msg_actor;
      } else {
        mactor = NULL;
      }

      if (social_table[cmd].others_auto != NULL) {
        sprintf(msg_observer, "%s\r\n", social_table[cmd].others_auto);
        mobserver = msg_observer;
      } else {
        mobserver = NULL;
      }

      wev = get_wev(WEV_SOCIAL, subwev, mcc,  mactor, NULL, mobserver);

     /* Check for permission... */     

      if (!room_issue_wev_challange(ch->in_room, wev)) {
        free_wev(wev);
        return TRUE;
      }

     /* Issue the wev... */

      room_issue_wev(ch->in_room, wev);
      free_wev(wev);
      return TRUE;
    }

   /* Do unto others... */ 

   /* Build the wev... */

    subwev = social_iff(ch, victim, cmd);
    mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 0, command);

    if (social_table[cmd].char_found != NULL) {
      sprintf(msg_actor, "%s\r\n", social_table[cmd].char_found);
      mactor = msg_actor;
    } else {
      mactor = NULL;
    }

    if (social_table[cmd].vict_found != NULL) {
      sprintf(msg_victim, "%s\r\n", social_table[cmd].vict_found);
      mvictim = msg_victim;
    } else {
      mvictim = NULL;
    }

    if (social_table[cmd].others_found != NULL) {
      sprintf(msg_observer, "%s\r\n", social_table[cmd].others_found);
      mobserver = msg_observer;
    } else {
      mobserver = NULL;
    }

    wev = get_wev(WEV_SOCIAL, subwev, mcc, mactor, mvictim, mobserver);

   /* Check for permission... */     

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return TRUE;
    }

   /* Issue the wev... */

    room_issue_wev(ch->in_room, wev);
    free_wev(wev);
    return TRUE;
}



/*
 * Return true if an argument is completely numeric.
 */
bool is_number ( char *arg ) {
 
    if ( *arg == '\0' ) return FALSE;
 
    if ( *arg == '+' || *arg == '-' )   arg++;
 
    for ( ; *arg != '\0'; arg++ ) {
        if ( !isdigit( *arg ) ) return FALSE;
    }
 
    return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 * Zeran - modify to return -1 for all.foo number 
 */
int number_argument( char *argument, char *arg ) {
    char *pdot;
    int number;
    
    for ( pdot = argument; *pdot != '\0'; pdot++ )    {
	if ( *pdot == '.' ) {
	    *pdot = '\0';
		if (!str_cmp(argument, "all")) number = -1;
		else number = atoi( argument );
	    *pdot = '.';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }

    strcpy( arg, argument );
    return 1;
}


/* elfren added for buying multiple items 
 *
 * Given a string like 14*foo, return 14 and 'foo'
 * argument is original string, arg will get name of the object
*/
int mult_argument(char *argument, char *arg) { 
    char *pdot;
    int number;

    for ( pdot = argument; *pdot != '\0'; pdot++ )    {
        if ( *pdot == '*' )       {
            *pdot = '\0';
            number = atoi( argument );
                *pdot = '*';
            strcpy( argument, pdot+1 );
            return number;
        }
    }

    return 1;
}


/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument_cap(char *argument, char *arg_first ) {
    char cEnd;

    while ( isspace(*argument)) argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' ) cEnd = *argument++;

    while ( *argument != '\0' )    {
	if ( *argument == cEnd ) {
	    argument++;
	    break;
	}
	*arg_first = *argument;
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument)) argument++;

    return argument;
}


char *one_argument(char *argument, char *arg_first ) {
    char cEnd;

    while ( isspace(*argument)) argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' ) cEnd = *argument++;

    while ( *argument != '\0' )    {
	if ( *argument == cEnd ) {
	    argument++;
	    break;
	}
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument)) argument++;

    return argument;
}


char *get_argument(char *old_argument, int anum) {
char * out;
char arg[MAX_INPUT_LENGTH];

    if (anum == 0) return NULL;

    while(anum > 0 && arg[0] != '\0') {
        old_argument = one_argument_cap(old_argument, arg);
        anum--;
    }        

    if (arg[0] == '\0') return NULL;
    out = strdup(arg);
    return out;
}   


void do_commands( CHAR_DATA *ch, char *argument ) {
char arg1[MAX_INPUT_LENGTH];   
char buf[MAX_STRING_LENGTH];
int cmd, col, cclass;
 
   
    one_argument(argument, arg1);
    if (arg1[0] == '\0') {
        cclass = -1;
        send_to_char("{CGlobal Command List:{x\r\n", ch);
        send_to_char("{CAvailiable sublists:{x movement, communication, interaction\r\n", ch);
        send_to_char("                         manipulation, combat, magic, special\r\n\r\n", ch);

    } else if (!strcmp(arg1, "movement")) {
        cclass = 1;
        send_to_char("{CMovement Command List:{x\r\n\r\n", ch);

    } else if (!strcmp(arg1, "communication")) {
        cclass = 2;
        send_to_char("{CCommunication Command List:{x\r\n\r\n", ch);
 
    } else if (!strcmp(arg1, "combat")) {
        cclass = 3;
        send_to_char("{CCombat Command List:{x\r\n\r\n", ch);
 
    } else if (!strcmp(arg1, "magic")) {
        cclass = 4;
        send_to_char("{CMagic Command List:{x\r\n\r\n", ch);

    } else if (!strcmp(arg1, "interaction")) {
        cclass = 5;
        send_to_char("{CInteraction Command List:{x\r\n\r\n", ch);

    } else if (!strcmp(arg1, "special")) {
        cclass = 6;
        send_to_char("{CSpecial Command List:{x\r\n\r\n", ch); 

    } else if (!strcmp(arg1, "manipulation")) {
        cclass = 7;
        send_to_char("{CManipulation Command List:{x\r\n\r\n", ch);

    } else {
        cclass = 0;
        send_to_char("{CGeneral Command List:{x\r\n", ch);
        send_to_char("{CAvailiable sublists:{x movement, communication, interaction\r\n", ch);
        send_to_char("                         manipulation, combat, magic, special\r\n\r\n", ch);
    }

    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )    {
        if ( cmd_table[cmd].div <  DIV_HERO
        &&   cmd_table[cmd].div <= get_divinity(ch) 
        &&   cmd_table[cmd].show) {
              if(cclass < 0 || cclass == cmd_table[cmd].cclass) {
	    sprintf( buf, "%-12s", cmd_table[cmd].name );
	    send_to_char( buf, ch );
	    if ( ++col % 6 == 0 ) send_to_char( "\r\n", ch );
              }
        }
    }
 
    if ( col % 6 != 0 ) send_to_char( "\r\n", ch );
    return;
}


void show_wizhelp( CHAR_DATA *ch, int index, char *label) {
char buf[MAX_STRING_LENGTH];
char head[20];
int col;
struct wizcommand_type *tmp;
	
  col = 0;

  sprintf (head, "%s  ", label);

  for (tmp = wizcommand_table[index]; tmp != NULL ; tmp = tmp->next) {
    if (tmp->auth != 0 && !IS_SET(get_authority(ch), tmp->auth)) continue;	    
    if (col % 5 == 0) {
        send_to_char(head, ch);
        if (col == 0) sprintf(head,"          ");
    }

    sprintf( buf, "%-12s", tmp->name );

    send_to_char( buf, ch );

    if ( ++col % 5 == 0 ) send_to_char( "\r\n", ch );
  }

  if (col % 5 != 0) {
    send_to_char ("\r\n",ch);
  }

  return;
}


void show_wizhelp_ext(CHAR_DATA *ch) {
char buf[MAX_STRING_LENGTH];
int col = 0;
struct wizcommand_type *tmp;

      if (get_divinity(ch) <= DIV_LESSER
      && ch->pcdata->moddiv > DIV_LESSER) {
             send_to_char("{YSpecial:{x", ch);
             for (tmp = wizcommand_table[DIV_INDEX_IMP]; tmp != NULL ; tmp = tmp->next) {
                  if (tmp->auth != -1) continue;	    
                  if (col % 5 == 0) send_to_char("          ", ch);
                  sprintf( buf, "%-12s", tmp->name );
                  send_to_char( buf, ch );
                  if ( ++col % 5 == 0 ) send_to_char( "\r\n", ch );
             }
             for (tmp = wizcommand_table[DIV_INDEX_GREATER]; tmp != NULL ; tmp = tmp->next) {
                  if (tmp->auth != -1) continue;	    
                  if (col % 5 == 0) send_to_char("          ", ch);
                  sprintf( buf, "%-12s", tmp->name );
                  send_to_char( buf, ch );
                  if ( ++col % 5 == 0 ) send_to_char( "\r\n", ch );
             }
             for (tmp = wizcommand_table[DIV_INDEX_GOD]; tmp != NULL ; tmp = tmp->next) {
                  if (tmp->auth != -1) continue;	    
                  if (col % 5 == 0) send_to_char("          ", ch);
                  sprintf( buf, "%-12s", tmp->name );
                  send_to_char( buf, ch );
                  if ( ++col % 5 == 0 ) send_to_char( "\r\n", ch );
             }
             for (tmp = wizcommand_table[DIV_INDEX_LESSER]; tmp != NULL ; tmp = tmp->next) {
                  if (tmp->auth != -1) continue;	    
                  if (col % 5 == 0) send_to_char("          ", ch);
                  sprintf( buf, "%-12s", tmp->name );
                  send_to_char( buf, ch );
                  if ( ++col % 5 == 0 ) send_to_char( "\r\n", ch );
             }
             for (tmp = wizcommand_table[DIV_INDEX_CREATOR]; tmp != NULL ; tmp = tmp->next) {
                  if (tmp->auth != -1) continue;	    
                  if (col % 5 == 0) send_to_char("          ", ch);
                  sprintf( buf, "%-12s", tmp->name );
                  send_to_char( buf, ch );
                  if ( ++col % 5 == 0 ) send_to_char( "\r\n", ch );
             }

             if (col % 5 != 0) send_to_char ("\r\n",ch);
      }
      return;
}


void do_wizhelp( CHAR_DATA *ch, char *argument ) {

  if IS_MUD(ch) return;

  sprintf_to_char(ch, "{CAuthority: {Y%s\r\n", flag_string(imm_auth, get_authority(ch)));
  if (get_divinity(ch) >= DIV_IMPLEMENTOR) 	show_wizhelp(ch, DIV_INDEX_IMP, "{YImp....:{x");
  if (get_divinity(ch) >= DIV_GREATER) 		show_wizhelp(ch, DIV_INDEX_GREATER, 	"{YGreater:{x");
  if (get_divinity(ch) >= DIV_GOD) 			show_wizhelp(ch, DIV_INDEX_GOD, "{YGod....:{x");
  if (get_divinity(ch) >= DIV_LESSER) 		show_wizhelp(ch, DIV_INDEX_LESSER, "{YLesser.:{x");
  if (get_divinity(ch) >= DIV_CREATOR) 		show_wizhelp(ch, DIV_INDEX_CREATOR, "{YCreator:{x");
  if (get_divinity(ch) >= DIV_HERO) 		show_wizhelp(ch, DIV_INDEX_HERO, "{CHero...:{x");

  if (ch->pcdata) show_wizhelp_ext(ch);
  return;
}

inline int get_div_index(int divinity) {

  switch (divinity) {

    case DIV_IMPLEMENTOR:
      return DIV_INDEX_IMP;

    case DIV_GREATER:
      return DIV_INDEX_GREATER;

    case DIV_GOD:
      return DIV_INDEX_GOD;

    case DIV_LESSER:
      return DIV_INDEX_LESSER;

    case DIV_CREATOR:
      return DIV_INDEX_CREATOR; 

    case DIV_HERO:
      return DIV_INDEX_HERO;

    case DIV_INVESTIGATOR:
      return DIV_INDEX_INVESTIGATOR;

    case DIV_HUMAN:
      return DIV_INDEX_HUMAN;

    case DIV_NEWBIE:
      return DIV_INDEX_NEWBIE; 

    default:
      break;
  }

  return DIV_INDEX_NEWBIE;
}

void gen_wiz_table (void) {
	
  int cmd;
  int div;
  int div_index;
  struct wizcommand_type *tmp;
  bool show;

  for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ ) {

    div = cmd_table[cmd].div;
    div_index = get_div_index(div);

    show = cmd_table[cmd].show;

    if (div >= HE && show) { 
			
      if (wizcommand_table[div_index] == NULL) {
			
	tmp = (struct wizcommand_type *) malloc (sizeof (struct wizcommand_type));
	tmp->name = strdup (cmd_table[cmd].name); 
	tmp->auth = cmd_table[cmd].cclass; 
	tmp->next = NULL;	
	wizcommand_table[div_index] = tmp;
      } else {
	for (tmp = wizcommand_table[div_index] ; tmp->next != NULL ; tmp = tmp->next);
	tmp->next = (struct wizcommand_type *) malloc (sizeof (struct wizcommand_type));
	tmp->next->name = strdup (cmd_table[cmd].name); 
	tmp->next->auth = cmd_table[cmd].cclass; 
	tmp->next->next = NULL;	
      }
    }
  }

  return;
}			

static const char div_name[DIV_INDEX][9] = {
  "None   ",
  "Newbie ",
  "Player ",
  "Hero   ",
  "Creator",
  "Lesser ",
  "God    ",
  "Greater",
  "Imp    "   
};

const char *get_div_name(int div) {

  int d_index = get_div_index(div);

  switch(d_index) {
    
    case DIV_IMPLEMENTOR:
      return div_name[DIV_INDEX_IMP];

    case DIV_GREATER:
      return div_name[DIV_INDEX_GREATER];

    case DIV_GOD:
      return div_name[DIV_INDEX_GOD];

    case DIV_LESSER:
      return div_name[DIV_INDEX_LESSER];

    case DIV_CREATOR:
      return div_name[DIV_INDEX_CREATOR];

    case DIV_HERO:
      return div_name[DIV_INDEX_HERO];

    case DIV_HUMAN:
      return div_name[DIV_INDEX_HUMAN];

    case DIV_NEWBIE:
      return div_name[DIV_INDEX_NEWBIE];

    default:
      break;
  }

  return div_name[DIV_INDEX_NONE];
}

/* Zeran - these functions should go in act_wiz.c, but putting them here so 
			don't have to declare the cmd_table globally. */

void do_disable (CHAR_DATA *ch, char *argument) {
    
    struct disable_cmd_type *tmp, *last_disabled = NULL;

    char command[MAX_INPUT_LENGTH];
    char *level_string;
    char outbuf[MAX_STRING_LENGTH];
    int div;
    int cmd;
    bool found=FALSE;
	
   /* No args means showtime... */
	
    if (argument == NULL || argument[0] == '\0') {
        
	send_to_char ("Disabled Commands\r\n",ch);
	send_to_char ("-----------------\r\n",ch);
	for (tmp = disable_cmd_list ; tmp != NULL ; tmp=tmp->next) {
	    
	    sprintf (outbuf, "[ {c%-12s{x ] at divinity [ {c%s{x ]\r\n", tmp->name, get_div_name(tmp->div));
	    send_to_char (outbuf,ch);
            found=TRUE;
	} 

	if (!found) 
	    send_to_char ("No commands are disabled at this time.\r\n",ch);

	return;
    }
   
   /* Find the supplied divinity... */ 
    	
    div = get_divinity(ch) & DIV_MASK;

    level_string = one_argument (argument, command);
  
    if ( level_string != NULL 
      && level_string[0] != '\0' ) {
        
        if (!strcmp(level_string, "imp")) {
          div = DIV_IMPLEMENTOR;
        } else if (!strcmp(level_string, "greater")) {
          div = DIV_GREATER;
        } else if (!strcmp(level_string, "god")) {
          div = DIV_GOD;
        } else if (!strcmp(level_string, "lesser")) {
          div = DIV_LESSER;
        } else if (!strcmp(level_string, "creator")) {
          div = DIV_CREATOR;
        } else if (!strcmp(level_string, "hero")) {
          div = DIV_HERO;
        } else if (!strcmp(level_string, "player")) {
          div = DIV_HUMAN;
        } else {
          send_to_char("disable <command> <imp|greater|god|lesser|creator|hero|player>\r\n", ch);
          return;
        }

	div = UMIN (div , get_divinity(ch));
    }
				
   /* See if the command is valid... */

    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ ) {
	    
	if ( command[0] == cmd_table[cmd].name[0]
	  &&   !str_prefix( command, cmd_table[cmd].name )
	  &&   cmd_table[cmd].div <= get_divinity(ch)) {
		
	    found = TRUE;
	    break;
	}
    }
		
    if (!found) {
	    
	send_to_char ("No such command found, or command restricted to higher level.\r\n",ch);
	return;
    }
		
   /* See if the command is already disabled... */
 
    for (tmp = disable_cmd_list; tmp != NULL ; tmp = tmp->next) {
 	    
	if (!str_prefix(command, tmp->name)) { 
	
	    sprintf (outbuf, "[ {c%s{x ] is already disabled for all characters below divinity [ {c%s{x ].\r\n", tmp->name, get_div_name(tmp->div));
	    send_to_char (outbuf,ch);
	    return;
	}

	last_disabled = tmp;
    }
		
   /* Do the disabling... */
 
    tmp = (struct disable_cmd_type *) malloc (sizeof (struct disable_cmd_type));
    tmp->next = NULL;
    tmp->name = strdup (cmd_table[cmd].name);
    tmp->div = div;
    tmp->disable_fcn=cmd_table[cmd].do_fun;
		
    if (disable_cmd_list == NULL)
	disable_cmd_list = tmp;
    else
	last_disabled->next = tmp;

   /* Notify the player... */
		
    sprintf (outbuf, "You have disabled [ {c%s{x ] for characters below level [ {c%s{x ].\r\n", tmp->name, get_div_name(tmp->div));
    send_to_char (outbuf, ch);

   /* Write out the updated disable list... */

    fwrite_disable();

    return;
}

void do_enable  (CHAR_DATA *ch, char *argument) {
    
    struct disable_cmd_type *tmp, *last_disabled=NULL;
    
    char command[MAX_INPUT_LENGTH];
    char outbuf[MAX_STRING_LENGTH];
    bool found = FALSE;
	
    if (argument == NULL || argument[0] == '\0') {
		
        send_to_char ("Syntax:  enable <command>\r\n",ch);
	return;
    }
	
    one_argument (argument, command);
	
   /* find command in disable list */

    for (tmp = disable_cmd_list; tmp != NULL ; tmp=tmp->next) {

 	if (!str_prefix(command, tmp->name)) {
	
	    found = TRUE;
	    break;
	}
     
	last_disabled = tmp;
    }
	
    if (!found) {
	send_to_char ("That command is not currently disabled...\r\n",ch);
	return;
    }
	
   /* remove command from disabled list */

    sprintf (outbuf, "[ {c%s{x ] disabling removed.\r\n", tmp->name);
    send_to_char (outbuf, ch);

    free_string (tmp->name);

    if (tmp == disable_cmd_list ) 
	disable_cmd_list = tmp->next;
    else 
	last_disabled->next = tmp->next; 

    free (tmp);

   /* Update the ondisk copy... */

    fwrite_disable();

    return;
}

/* Zeran - This function has a horribly inelegant method of watching for
	quoted arguments, but at least it works. *sigh* */

/* Reformatted - Mik */
 
bool expand_aliases (CHAR_DATA *ch, char *orig_command, char *final_command) {
	
  char arg[MAX_INPUT_LENGTH];
  char *remainder;
  char *tmp_remainder;

  struct alias_data *tmp;

  int counter;

  int fcp, ocp;

  bool match = FALSE;

 /* Command and up to 9 parms... Mik. */

  char allargs[10][MAX_INPUT_LENGTH];

  int total_args;
  int tmp_count;
  int tmp_len;
  int value;

  char single[3];

  single[0]='\0';
	
  final_command[0]='\0';	

  fcp = 0;
  ocp = 0;

 /* smash out tildes..sheesh */

  smash_tilde (orig_command);

 /* Null command or empty command gives empty response. */	
  
  if (orig_command == NULL
   || orig_command == '\0') {
    return FALSE;
  }

 /* Command starting with ' is returned unchanged. */

  if (orig_command[0] == '\'') {
   
    while (orig_command[ocp] != '\0') {
      final_command[fcp++] = orig_command[ocp++];
    } 
    final_command[fcp] = '\0';

    return FALSE;
  }
	
 /* Check the first word to see if is an alias name... */
	
  remainder = one_argument (orig_command, arg);

  for (counter = 0 ; counter < MAX_ALIAS ; counter++) {

   /* Get next alias... */
 
    tmp = ch->pcdata->aliases[counter];

   /* Bail out if we've passed the last alias... */ 
 
    if (tmp == NULL)
      continue;

   /* If we've got a match... */

    if (!strcmp (tmp->name, arg)) {
			
     /* check for parameters required for alias */

      total_args=1;
      tmp_remainder = remainder;

     /* allargs has 10 slots. 1 thru 9 hold parms, 0 seems empty. Mik. */

      while (tmp_remainder != NULL 
          && tmp_remainder[0] != '\0' 
          && total_args < 10) {

       /* Copy word/phrase 'n' into allargs(n)... Mik. */

        tmp_remainder = one_argument (tmp_remainder, allargs[total_args]);

        total_args++;
      }
			
      tmp_remainder = tmp->command_string;
      tmp_len = strlen(tmp->command_string);		

     /* Copy expanded text 1 char at a time, looking for parms... */

      for (tmp_count = 0; tmp_count < (tmp_len) ; tmp_count++) {

       /* If the char is a %, check for parm substitution... */

        if ( tmp_remainder[tmp_count] == '%' 
          && isdigit(tmp_remainder[tmp_count +1])
          && !isdigit(tmp_remainder[tmp_count + 2])) { 
					
         /* Parm number is from 1 to 9... */
 
          value = (tmp_remainder[tmp_count+1] - '0');
          if (value < 1 || value > 9)
            value=-1;

         /* Substitue if we have a valid value... */ 

          if ( value != -1) {
	
           /* Replace parm by given value or a space if not specified. */
	
            if ( value < total_args) {
              ocp = 0;
              while (allargs[value][ocp] != '\0') {
                final_command[fcp++] = allargs[value][ocp++];
              }
            } else {
              final_command[fcp++] = ' ';
            }

           /* Skip over the number... */ 
 
            tmp_count++;

          } else {

           /* Invalid value, so copy it over... */

            final_command[fcp++] = tmp_remainder[tmp_count++]; /* % */
            final_command[fcp++] = tmp_remainder[tmp_count]; /* x */

          }
        } else {	

         /* Just copy over the next character... */
    
          final_command[fcp++] = tmp_remainder[tmp_count];    /* x */
        }
      }
 
     /* Finish the command. */
    
      final_command[fcp] = '\0';
       
      match = TRUE;
    }
  }

 /* No match means we return a copy of the original. */ 

  if (!match) {
    ocp = 0;
    while (orig_command[ocp] != '\0') {
      final_command[fcp++] = orig_command[ocp++];
    }
    final_command[fcp] = '\0'; 
    return FALSE;
  }

 /* Report our success! */

  return TRUE;
}

/* Zeran - procedure to scan for multiple commands */
void check_multi_cmd (CHAR_DATA *ch, char *orig_cmd, char *final_cmd) {
	int count;
	int len;
	char *tmp_ptr=NULL;
	bool first_s_quote=FALSE;
	bool first_d_quote=FALSE;
	char *tmp_incomm=NULL;
	bool need_tmp_incomm=FALSE;

	len = strlen (orig_cmd) ;
	
	/* are we parsing an alias, or just parsing incomm? */	
	if (orig_cmd != ch->desc->incomm) need_tmp_incomm = TRUE;
		
	for (count = 0; count < len ; count++) {

		switch (orig_cmd[count]) {

			case '"':
				if (!first_s_quote) {
					if (!first_d_quote) {
						first_d_quote = TRUE;
					} else {
						first_d_quote = FALSE;
                                        } 
                                }
				break;

                       /* skip if count is 0...its the short say command */

			case '\'': 
				if (count == 0) {
					break;
                                }
				if (!first_d_quote) {
					if (!first_s_quote) {
						first_s_quote = TRUE;
					} else {
						first_s_quote = FALSE;
                                        }
                                }   
				break;

			case '|':
				if ( !tmp_ptr 
                                  && !first_d_quote 
                                  && !first_s_quote) {
					tmp_ptr = &(orig_cmd[count]);
                                }
				break;

			default:
				break;
		} /* end switch */
		/* if got a separator pointer, break */
		if (tmp_ptr) {
			break;
                }
	} /* end for loop */

	if (tmp_ptr != NULL && !first_s_quote && !first_d_quote)
		{
		ch->desc->multi_comm=TRUE;

		/* copy ch->desc->incomm if needed*/
		if (need_tmp_incomm)	
			tmp_incomm = str_dup (ch->desc->incomm);
		*tmp_ptr = '\0';
		tmp_ptr++;
		while (isspace(*tmp_ptr))
			tmp_ptr++;
		strcpy (final_cmd, orig_cmd);
		strcpy (ch->desc->incomm,tmp_ptr);
		if (need_tmp_incomm)
			{
			if ( ( strlen(ch->desc->incomm) + strlen((tmp_ptr+1)) ) >= (MAX_INPUT_LENGTH-10) )
				{
				send_to_char ("Command expansion too large, ignoring last command.\r\n",ch);
				ch->desc->incomm[0]='\0';
				final_cmd[0]='\0';
				return;
				}
			strcat (ch->desc->incomm, "|");
			strcat (ch->desc->incomm, tmp_incomm);		
			}
		free_string (tmp_incomm);
		return;
		}
	else
		{
		strcpy (final_cmd, orig_cmd);
		if (orig_cmd == ch->desc->incomm)
			ch->desc->incomm[0]='\0';
		}
	return;
	}	


int social_iff(CHAR_DATA *ch, CHAR_DATA *victim, int cmd) {
int subwev = WEV_SOCIAL_NEUTRAL;

    if (ch == victim) return WEV_SOCIAL_NEUTRAL;

    switch(social_table[cmd].s_style) {
         default:
             subwev = WEV_SOCIAL_NEUTRAL;
             break;
         case SOCIAL_FRIENDLY:
             subwev = WEV_SOCIAL_FRIENDLY;
             break;
         case SOCIAL_HOSTILE:
             subwev = WEV_SOCIAL_HOSTILE;
             break;
         case SOCIAL_SEXUAL:
             if (IS_NEWBIE(ch)) {
  	send_to_char( "Newbies are not allowed to use that social.\r\n", ch );
	return TRUE;
             }
             if (victim) {
                  if (is_friend(victim, ch)) {
                      subwev = WEV_SOCIAL_FRIENDLY;
                  } else {
                      subwev = WEV_SOCIAL_HOSTILE;
                  }
             } else {
                  subwev = WEV_SOCIAL_FRIENDLY;
             }
             break;
    }
    return subwev;
}


bool channel_social( CHAR_DATA *ch, char *command, char *argument, int channel_id) {
char arg[MAX_INPUT_LENGTH];
CHAR_DATA *victim;
int cmd;
bool found;
WEV *wev;
MOB_CMD_CONTEXT *mcc;
char msg_actor[MAX_STRING_LENGTH];
char msg_victim[MAX_STRING_LENGTH];
char msg_observer[MAX_STRING_LENGTH];
char *mactor;
char *mvictim;
char *mobserver;

    found  = FALSE;

    for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ ) {
	if ( command[0] == social_table[cmd].name[0]
	&&   !str_prefix( command, social_table[cmd].name ) ) {
	    found = TRUE;
	    break;
	}
    }

    if (!found) return FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) ) {
	send_to_char( "You are anti-social!\r\n", ch );
	return TRUE;
    }

    switch ( ch->position ) {

      case POS_DEAD:
	send_to_char( "Lie still; you are DEAD.\r\n", ch );
	return TRUE;

      case POS_INCAP:
      case POS_MORTAL:
	send_to_char( "You are hurt far too bad for that.\r\n", ch );
	return TRUE;

      case POS_STUNNED:
	send_to_char( "You are too stunned to do that.\r\n", ch );
	return TRUE;

      case POS_SLEEPING:
	if ( !str_cmp( social_table[cmd].name, "snore" ) ) {
	    break;
               }
	send_to_char( "In your dreams, or what?\r\n", ch );
	return TRUE;

    }

    one_argument(argument, arg );
    victim = NULL;

    if (arg[0] == '\0') {
      mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, channel_id, command);

      if (social_table[cmd].char_no_arg != NULL) {
        sprintf(msg_actor, "%s*%s*{x\r\n", channel_col[channel_id], social_table[cmd].char_no_arg);
        mactor = msg_actor;
      } else {
        mactor = NULL;
      }

      if (social_table[cmd].others_no_arg != NULL) {
          sprintf(msg_observer, "%s*%s*{x\r\n", channel_col[channel_id], social_table[cmd].others_no_arg);
          mobserver = msg_observer;
      } else {
          mobserver = NULL;
      }

      wev = get_wev(WEV_OOCC, WEV_OOCC_SOCIAL, mcc, mactor, NULL, mobserver);

      if (!room_issue_wev_challange(ch->in_room, wev)) {
          free_wev(wev);
          return TRUE;
      }

      world_issue_wev(wev, "OOC");
      free_wev(wev);
      return TRUE;
    }

   /* Go find the victim... */

    victim = get_char_world(ch, arg );
    if ( victim == NULL ) {
	send_to_char( "They aren't here.\r\n", ch );
                return TRUE;
    }

    if ( victim == ch ) {

      mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, channel_id, command);

      if (social_table[cmd].char_auto != NULL) {
        sprintf(msg_actor, "%s*%s*{x\r\n", channel_col[channel_id], social_table[cmd].char_auto);
        mactor = msg_actor;
      } else {
        mactor = NULL;
      }

      if (social_table[cmd].others_auto != NULL) {
        sprintf(msg_observer, "%s*%s*{x\r\n", channel_col[channel_id], social_table[cmd].others_auto);
        mobserver = msg_observer;
      } else {
        mobserver = NULL;
      }

      wev = get_wev(WEV_OOCC, WEV_OOCC_SOCIAL, mcc,  mactor, NULL, mobserver);

      if (!room_issue_wev_challange(ch->in_room, wev)) {
        free_wev(wev);
        return TRUE;
      }

      world_issue_wev(wev, "OOC");
      free_wev(wev);
      return TRUE;
    }

    mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, channel_id, command);

    if (social_table[cmd].char_found != NULL) {
      sprintf(msg_actor, "%s*%s*{x\r\n", channel_col[channel_id], social_table[cmd].char_found);
      mactor = msg_actor;
    } else {
      mactor = NULL;
    }

    if (social_table[cmd].vict_found != NULL) {
      sprintf(msg_victim, "%s*%s*{x\r\n", channel_col[channel_id], social_table[cmd].vict_found);
      mvictim = msg_victim;
    } else {
      mvictim = NULL;
    }

    if (social_table[cmd].others_found != NULL) {
      sprintf(msg_observer, "%s*%s*{x\r\n", channel_col[channel_id], social_table[cmd].others_found);
      mobserver = msg_observer;
    } else {
      mobserver = NULL;
    }

    wev = get_wev(WEV_OOCC, WEV_OOCC_SOCIAL, mcc, mactor, mvictim, mobserver);

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return TRUE;
    }

    world_issue_wev(wev, "OOC");
    free_wev(wev);
    return TRUE;
}


    

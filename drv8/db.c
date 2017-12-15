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


#define IN_DB_C

#include "everything.h"
#include "db.h"
#include "descrip.h"
#include "skill.h"
#include "affect.h"
#include "spell.h"
#include "exp.h"
#include "prof.h"
#include "econd.h"
#include "conv.h"
#include "mob.h"
#include "doors.h"
#include "wev.h"
#include "deeds.h"
#include "quests.h"
#include "gadgets.h"
#include "triggers.h"
#include "society.h"
#include "monitor.h"
#include "current.h"
#include "fight.h"
#include "profile.h"
#include "tracks.h"
#include "race.h"
#include "olc.h"
#include "music.h"
#include "cult.h"
#include "board.h"
#include "partner.h"
#include "gsn.h"
#include "help.h"

#undef IN_DB_C

#if defined(unix)
/*extern int getrlimit(int resource, struct rlimit *rlp);*/
/*extern int setrlimit(int resource, struct rlimit *rlp);*/
#endif


#if !defined(macintosh)
extern	int	_filbuf		args( (FILE *) );
#endif

/* External functions... */

extern void load_banks();
extern void load_shares();

/* declare and initialize globals.... */

FILE *time_file	= NULL;

HELP_DATA *help_first	= NULL;
HELP_DATA *help_last	= NULL;

HELP_INDEX *hindex_first	= NULL;
HELP_INDEX *hindex_last	= NULL;

SHOP_DATA *shop_first	= NULL;
SHOP_DATA *shop_last	= NULL;

CHAR_DATA *char_free	= NULL;
NOTE_DATA *note_free	= NULL;
OBJ_DATA *obj_free	= NULL;
PC_DATA *pcdata_free	= NULL;

extern EXTRA_DESCR_DATA *extra_descr_free;

CHAR_DATA *char_list	= NULL;
char *help_greeting	= NULL;
char *help_impgreeting	= NULL;
OBJ_DATA *object_list	= NULL;

TIME_INFO_DATA		time_info;
WEATHER_DATA		weather_info;
bool                                        startup = TRUE;

char			log_buf		[2*MAX_INPUT_LENGTH];
char			bug_buf		[2*MAX_INPUT_LENGTH];

KILL_DATA		kill_table	[MAX_LEVEL];


/*
 * MOBprogram locals
*/

int 			mprog_name_to_type	(char* name);
MPROG_DATA *	mprog_file_read 		(char* f, MPROG_DATA* mprg, MOB_INDEX_DATA *pMobIndex);
void			load_mobprogs           	(FILE* fp);
void   			mprog_read_programs     	(FILE* fp, MOB_INDEX_DATA *pMobIndex);
bool			MOBtrigger;

/*
 * Locals.
 */
MOB_INDEX_DATA *	mob_index_hash		[MAX_KEY_HASH];
OBJ_INDEX_DATA *	obj_index_hash		[MAX_KEY_HASH];
ROOM_INDEX_DATA *	room_index_hash		[MAX_KEY_HASH];
char *			string_hash		[MAX_KEY_HASH];

AREA_DATA 		*area_first;
AREA_DATA 		*area_last;
PASSPORT 		*passport_list;

char *			string_space;
char *			top_string;
char			str_empty	[1];

int			top_affect;
int			top_area;
int			top_ed;
int			top_exit;
int			top_help;
int			top_hindex;
int			top_mob_index;
int			top_obj_index;
int			top_reset;
int			top_room;
int			top_shop;
VNUM                    	top_vnum_room;
VNUM                    	top_vnum_mob;
VNUM                    	top_vnum_obj; 
int 			mobile_count = 0;
int			newmobs = 0;
int			newobjs = 0;
int 			rumor_count;

/*
 * Memory management.
 * Increase MAX_STRING if you have too.
 * Tune the others only if you understand what you're doing.
 */
#define			MAX_STRING		4301952
#define			MAX_PERM_BLOCK	131072
#define			MAX_MEM_LIST	12

void *			rgFreeList	[MAX_MEM_LIST];
const int		rgSizeList	[MAX_MEM_LIST]	=
{
    16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768-64, 65536-128
};

int			nAllocString;
int			sAllocString;
int			nAllocPerm;
int			sAllocPerm;


/*
 * Semi-locals.
 */
bool			fBootDb;
FILE *			fpArea;
char			strArea[MAX_INPUT_LENGTH];


extern int port,control;


/*
 * Local booting procedures.
*/
void    	init_mm         		(void);
void	load_area		(FILE *fp);
void    	new_load_area   		(FILE *fp);
void	load_helps		(FILE *fp);
void	load_old_mob		(FILE *fp);
void 	load_mobiles		(FILE *fp);
void	load_old_obj		(FILE *fp);
void 	load_objects		(FILE *fp, bool is_new);
void	load_resets		(FILE *fp);
void	load_rooms		(FILE *fp);
void	load_shops		(FILE *fp);
void	load_specials		(FILE *fp);
void	load_disable		(void);
void	fix_exits			(void);
void	check_area_recalls	(void);
void	reset_area		(AREA_DATA * pArea, bool forcereset);
void 	load_socials		(void);
void    	room_update 		(AREA_DATA *pArea );
void	load_ban		(void);
bool 	load_artifact		(RESET_DATA *pReset);
void 	save_artifacts_forward	(void);
void	copyover_recover 	(void);
void 	init_descriptor 		(DESCRIPTOR_DATA *dnew, int desc);
void 	update_obj_orig 		(OBJ_DATA *obj);
void 	load_leases		(void);
void 	load_rumors		(void);
void 	load_passport		(void);

extern bool write_to_descriptor	( int desc, char *txt, int length );

DECLARE_DO_FUN(do_look);


#if defined(unix)
/* RT max open files fix */
 
void maxfilelimit() {
    struct rlimit r;
 
    getrlimit(RLIMIT_NOFILE, &r);
    r.rlim_cur = r.rlim_max;
    setrlimit(RLIMIT_NOFILE, &r);
}
#endif

extern int seasons[];

/*
 * Big mama top level function.
 */
void boot_db(bool fCopyOver) {
char buf[MAX_STRING_LENGTH];
long lhour, lday, lmonth;
int total;
bool fail=FALSE;

#if defined(unix)
    /* open file fix */
    maxfilelimit();
#endif

   /* Reinit to suppress warnings... */

    note_free = NULL;
    extra_descr_free = NULL;

   /* Obtain memory for strings... */

    string_space = (char *) calloc( 1, MAX_STRING );

    if ( string_space == NULL ) {
      bug( "Boot_db: can't alloc %d string space.", MAX_STRING );
      exit( 1 );
    }

    top_string	= string_space;

   /* Note we're in boot mode... */

    fBootDb = TRUE;

   /* Initialize the PRN... */

    init_mm( );

   /* Read the time file... */
 
    time_file = fopen (TIME_FILE, "r");

    if ( time_file != NULL) {

      total = fscanf(time_file, "%d %d %d %d", 
                    &(time_info.hour),
                    &(time_info.day),
                    &(time_info.month),
                    &(time_info.year));

      if (total < 4) {
        log_string ("loading stored time failed, using default");
        fail=TRUE;
      }

      fclose (time_file);

    } else {
      log_string ("failed open of time_file, loading default time.");		 
      fail=TRUE;
    }

   /* If that failed, make up a time from the system time... */ 

    if (fail) {

      lhour = (current_time - 650336715) / (PULSE_TICK / PULSE_PER_SECOND);

      time_info.hour	= lhour  % 24;
 
      lday		= lhour  / 24;
      time_info.day	= lday   % DAYS_IN_MONTH;

      lmonth		= lday   / DAYS_IN_MONTH;
      time_info.month	= lmonth % MONTHS_IN_YEAR;
      time_info.year	= lmonth / MONTHS_IN_YEAR;
    }

    time_info.season = seasons[time_info.month];

    time_info.minute = 0;
    time_info.moon = 0;

    time_info.week = (time_info.month * DAYS_IN_MONTH) + time_info.day;
    time_info.week /= DAYS_IN_WEEK;

    time_info.flags = 0;

   /* Determine temporary weather settings until next update call... */

    weather_info.moon = MOON_NEW;
    weather_info.natural_light = LIGHT_DAY;

    if ( time_info.hour <  5 ) {
      weather_info.sunlight = SUN_DARK;
      SET_BIT(time_info.flags, TIME_NIGHT); 
    } else if ( time_info.hour <  6 ) {
      weather_info.sunlight = SUN_RISE;
      SET_BIT(time_info.flags, TIME_DAWN); 
    } else if ( time_info.hour < 19 ) {
      weather_info.sunlight = SUN_LIGHT;
      SET_BIT(time_info.flags, TIME_DAY); 
    } else if ( time_info.hour < 20 ) { 
      weather_info.sunlight = SUN_SET;
      SET_BIT(time_info.flags, TIME_DUSK); 
    } else {
      weather_info.sunlight = SUN_DARK;
      SET_BIT(time_info.flags, TIME_NIGHT); 
    } 

    weather_info.sky = SKY_CLOUDY;
    weather_info.duration = dice(1,4);

   /* Load the skills... */
   
    log_string("Loading Skills...");
    load_skills();
    
   /* Assign gsns... */
 
    gsn_backstab 		= get_skill_gsn("backstab");
    gsn_blackjack   	= get_skill_gsn("blackjack");
    gsn_assassinate 	= get_skill_gsn("assassinate");
    gsn_circle		= get_skill_gsn("circle");
    gsn_envenom		= get_skill_gsn("envenom");
    gsn_hide		= get_skill_gsn("hide");
    gsn_peek		= get_skill_gsn("peek");
    gsn_pick_lock		= get_skill_gsn("pick lock");
    gsn_sneak		= get_skill_gsn("sneak");
    gsn_steal		= get_skill_gsn("steal");
    gsn_teach		= get_skill_gsn("teach");
    gsn_disarm		= get_skill_gsn("disarm");
    gsn_strong_grip	= get_skill_gsn("strong grip");
    gsn_kick		= get_skill_gsn("kick");
    gsn_strangle		= get_skill_gsn("strangle");
    gsn_rescue		= get_skill_gsn("rescue");
    gsn_rotate		= get_skill_gsn("rotate");
    gsn_sharpen		= get_skill_gsn("sharpen");
    gsn_forging		= get_skill_gsn("forging");
    gsn_tailor		= get_skill_gsn("tailor");
    gsn_bandage		= get_skill_gsn("bandage");
    gsn_psychology	= get_skill_gsn("psychology");
    gsn_intimidate		= get_skill_gsn("intimidate");
    gsn_riding		= get_skill_gsn("riding");
    gsn_traps		= get_skill_gsn("traps");
    gsn_explosives		= get_skill_gsn("explosives");
    gsn_chemistry		= get_skill_gsn("chemistry");
    gsn_voodoo		= get_skill_gsn("voodoo");
    gsn_blindness 		= get_skill_gsn("blindness");
    gsn_charm_person	= get_skill_gsn("charm person");
    gsn_curse		= get_skill_gsn("curse");
    gsn_invis		= get_skill_gsn("invis");
    gsn_mass_invis	= get_skill_gsn("mass invis");
    gsn_poison		= get_skill_gsn("poison");
    gsn_plague		= get_skill_gsn("plague");
    gsn_sleep		= get_skill_gsn("sleep");
    gsn_search		= get_skill_gsn("search");
    gsn_detection		= get_skill_gsn("detection");
    gsn_theology		= get_skill_gsn("theology");
    gsn_survival		= get_skill_gsn("survival");
    gsn_yuggoth		= get_skill_gsn("yuggoth");
    gsn_tame		= get_skill_gsn("tame");
    gsn_axe		= get_skill_gsn("axe");
    gsn_dagger		= get_skill_gsn("dagger");;
    gsn_flail 		= get_skill_gsn("flail");
    gsn_mace 		= get_skill_gsn("mace");
    gsn_polearm		= get_skill_gsn("polearm");
    gsn_spear		= get_skill_gsn("spear");
    gsn_sword		= get_skill_gsn("sword");
    gsn_whip		= get_skill_gsn("whip");
    gsn_staff		= get_skill_gsn("staff");
    gsn_axe_master	= get_skill_gsn("axe master");
    gsn_dagger_master	= get_skill_gsn("dagger master");;
    gsn_flail_master	= get_skill_gsn("flail master");
    gsn_mace_master	= get_skill_gsn("mace master");
    gsn_polearm_master	= get_skill_gsn("polearm master");
    gsn_spear_master	= get_skill_gsn("spear master");
    gsn_sword_master	= get_skill_gsn("sword master");
    gsn_whip_master	= get_skill_gsn("whip master");
    gsn_staff_master	= get_skill_gsn("staff master");
    gsn_gun		= get_skill_gsn("gun");
    gsn_handgun 		= get_skill_gsn("handgun");
    gsn_smg 		= get_skill_gsn("submachinegun");
    gsn_marksman		= get_skill_gsn("marksman");
    gsn_bow		= get_skill_gsn("bow");
    gsn_master_archer 	= get_skill_gsn("master archer");
    gsn_hand_to_hand	= get_skill_gsn("hand to hand");
    gsn_martial_arts	= get_skill_gsn("martial arts");
    gsn_black_belt  	= get_skill_gsn("black belt");
    gsn_second_attack	= get_skill_gsn("second attack");
    gsn_third_attack	= get_skill_gsn("third attack");
    gsn_fourth_attack	= get_skill_gsn("fourth attack");
    gsn_enhanced_damage	= get_skill_gsn("enhanced damage");
    gsn_ultra_damage	= get_skill_gsn("ultra damage");
    gsn_lethal_damage	= get_skill_gsn("lethal damage");
    gsn_parry		= get_skill_gsn("parry");
    gsn_dodge		= get_skill_gsn("dodge");
    gsn_shield_block	= get_skill_gsn("shield block");
    gsn_bash		= get_skill_gsn("bash");
    gsn_berserk		= get_skill_gsn("berserk");
    gsn_dual		= get_skill_gsn("dual");
    gsn_dirt		= get_skill_gsn("dirt kicking");
    gsn_trip		= get_skill_gsn("trip");
    gsn_tail		= get_skill_gsn("tail");
    gsn_crush		= get_skill_gsn("crush");
    gsn_fast_healing	= get_skill_gsn("fast healing");
    gsn_haggle		= get_skill_gsn("haggle");
    gsn_lore		= get_skill_gsn("lore");
    gsn_scrying		= get_skill_gsn("scrying");
    gsn_recruit		= get_skill_gsn("recruit");
    gsn_leadership		= get_skill_gsn("leadership");
    gsn_meditation		= get_skill_gsn("meditation");
    gsn_spell_casting 	= get_skill_gsn("spell casting");
    gsn_ritual_mastery 	= get_skill_gsn("ritual mastery");
    gsn_elder_magic 	= get_skill_gsn("elder magic");
    gsn_natural_magic 	= get_skill_gsn("natural magic");
    gsn_scrolls		= get_skill_gsn("scrolls");
    gsn_staves		= get_skill_gsn("staves");
    gsn_wands		= get_skill_gsn("wands");
    gsn_recall		= get_skill_gsn("recall");
    gsn_swim		= get_skill_gsn("swim");
    gsn_climb		= get_skill_gsn("climb");
    gsn_dreaming		= get_skill_gsn("dreaming");
    gsn_master_dreamer	= get_skill_gsn("master dreamer");
    gsn_tracking		= get_skill_gsn("tracking");
    gsn_debating 		= get_skill_gsn("debating");
    gsn_interrogate 	= get_skill_gsn("interrogate");
    gsn_english		= get_skill_gsn("english");
    gsn_spanish		= get_skill_gsn("spanish");
    gsn_french		= get_skill_gsn("french");
    gsn_german		= get_skill_gsn("german");
    gsn_gaelic		= get_skill_gsn("gaelic");
    gsn_polish		= get_skill_gsn("polish");
    gsn_italien		= get_skill_gsn("italian");
    gsn_chinese		= get_skill_gsn("chinese");
    gsn_japanese		= get_skill_gsn("japanese");
    gsn_hebrew		= get_skill_gsn("hebrew");
    gsn_arabic		= get_skill_gsn("arabic");
    gsn_greek		= get_skill_gsn("greek");
    gsn_latin		= get_skill_gsn("latin");
    gsn_heiroglyphics	= get_skill_gsn("heiroglyphics");
    gsn_old_english	= get_skill_gsn("old english");
    gsn_romany		= get_skill_gsn("romany");
    gsn_stygian		= get_skill_gsn("stygian");
    gsn_atlantean		= get_skill_gsn("atlantean");
    gsn_cthonic		= get_skill_gsn("cthonic");
    gsn_mayan		= get_skill_gsn("mayan");
    gsn_occult		= get_skill_gsn("occult");
    gsn_necromancy	= get_skill_gsn("necromancy");
    gsn_tactics		= get_skill_gsn("tactics");
    gsn_music		= get_skill_gsn("music");
    gsn_singing		= get_skill_gsn("singing");
    gsn_percussion	= get_skill_gsn("percussion");
    gsn_strings		= get_skill_gsn("strings");
    gsn_flute		= get_skill_gsn("flute");
    gsn_brass		= get_skill_gsn("brass");
    gsn_piano		= get_skill_gsn("piano");
    gsn_organ		= get_skill_gsn("organ");
    gsn_crystal		= get_skill_gsn("sound crystal");

    log_string("Loading music styles..."); 
    load_mstyles();

   /* Set up spell/skill links... */ 

    log_string("Loading spells...");
    load_spells();
    setup_spells();

   /* Initialize gafns... */

    gafn_stats			= get_affect_gafn("stats");

    gafn_armor			= get_affect_gafn("armor");
    gafn_bless			= get_affect_gafn("bless");
    gafn_blindness			= get_affect_gafn("blindness");
    gafn_charm			= get_affect_gafn("charm");
    gafn_chill_touch		= get_affect_gafn("chill touch");
    gafn_curse			= get_affect_gafn("curse");
    gafn_detect_evil		= get_affect_gafn("detect evil");
    gafn_detect_invis		= get_affect_gafn("detect invis");
    gafn_detect_magic		= get_affect_gafn("detect magic");
    gafn_invisability		= get_affect_gafn("invisability");
    gafn_poison			= get_affect_gafn("poison");
    gafn_protection_evil		= get_affect_gafn("protection evil");
    gafn_sanctuary		= get_affect_gafn("sanctuary");
    gafn_sleep			= get_affect_gafn("sleep");
    gafn_giant_strength		= get_affect_gafn("giant strength");
    gafn_detect_hidden		= get_affect_gafn("detect hidden");
    gafn_fly			= get_affect_gafn("fly");
    gafn_mist			= get_affect_gafn("mist");
    gafn_elder_shield		= get_affect_gafn("elder shield");
    gafn_farsight			= get_affect_gafn("farsight");
    gafn_hard_skin		= get_affect_gafn("hard skin");
    gafn_shield			= get_affect_gafn("shield");
    gafn_mana_shield		= get_affect_gafn("mana shield");
    gafn_weakness			= get_affect_gafn("weakness");
    gafn_faerie_fire		= get_affect_gafn("faerie fire");
    gafn_pass_door		= get_affect_gafn("pass door");
    gafn_infravision		= get_affect_gafn("infravision");
    gafn_sex_change		= get_affect_gafn("sex change");
    gafn_mind_meld		= get_affect_gafn("mind meld");
    gafn_haste			= get_affect_gafn("haste");
    gafn_plague			= get_affect_gafn("plague");
    gafn_frenzy			= get_affect_gafn("frenzy");
    gafn_calm			= get_affect_gafn("calm");
    gafn_mask_self		= get_affect_gafn("mask self");
    gafn_mask_hide		= get_affect_gafn("mask hide");
    gafn_mask_force		= get_affect_gafn("mask force");
    gafn_absorb_magic		= get_affect_gafn("absorb magic");
    gafn_fear			= get_affect_gafn("fear");
    gafn_protection_good		= get_affect_gafn("protection good");
    gafn_detect_good		= get_affect_gafn("detect good");
    gafn_regeneration		= get_affect_gafn("regeneration");
    gafn_vocalize			= get_affect_gafn("vocalize");
    gafn_mute			= get_affect_gafn("mute");
    gafn_dirt_kicking		= get_affect_gafn("dirt kicking");
    gafn_berserk			= get_affect_gafn("berserk");
    gafn_sneak			= get_affect_gafn("sneak");
    gafn_enchant_armor		= get_affect_gafn("enchant armor");
    gafn_enchant_weapon		= get_affect_gafn("enchant weapon");
    gafn_water_breathing		= get_affect_gafn("water breathing");
    gafn_starvation		= get_affect_gafn("starvation");
    gafn_dehydration		= get_affect_gafn("dehydration");
    gafn_obesity			= get_affect_gafn("obesity");
    gafn_head_wound		= get_affect_gafn("head wound");
    gafn_body_wound		= get_affect_gafn("body wound");
    gafn_arm_wound		= get_affect_gafn("arm wound");
    gafn_leg_wound		= get_affect_gafn("leg wound");
    gafn_intoxication		= get_affect_gafn("intoxication");
    gafn_hangover			= get_affect_gafn("hangover");
    gafn_size			= get_affect_gafn("size");
    gafn_fatigue			= get_affect_gafn("fatigue");
    gafn_darkness			= get_affect_gafn("darkness");
    gafn_aura                          	= get_affect_gafn("aura");
    gafn_hallucinating		= get_affect_gafn("hallucinating");
    gafn_relaxed                         	= get_affect_gafn("relaxed");
    gafn_fire_shield		= get_affect_gafn("fire shield");
    gafn_frost_shield                          	= get_affect_gafn("frost shield");
    gafn_lightning_shield                     = get_affect_gafn("lightning shield");
    gafn_protection_fire		= get_affect_gafn("protection fire");
    gafn_protection_frost		= get_affect_gafn("protection frost");
    gafn_protection_lightning	= get_affect_gafn("protection lightning");
    gafn_protection_energy		= get_affect_gafn("protection energy");
    gafn_slow			= get_affect_gafn("slowed");
    gafn_globe			= get_affect_gafn("globe-of-protection");
    gafn_morf			= get_affect_gafn("morf");
    gafn_incarnated		= get_affect_gafn("incarnated");
    gafn_asceticism		= get_affect_gafn("asceticism");
    gafn_antipsychotica		= get_affect_gafn("antipsychotica");

    gafn_room_holy		= get_affect_gafn("room holy");
    gafn_room_evil		= get_affect_gafn("room evil");
    gafn_room_silence                          = get_affect_gafn("room silence");
    gafn_room_darkness		= get_affect_gafn("room darkness");
    gafn_room_wild_magic		= get_affect_gafn("room wild magic");
    gafn_room_low_magic		= get_affect_gafn("room low magic");
    gafn_room_high_magic		= get_affect_gafn("room high magic");
    gafn_room_no_breathe		= get_affect_gafn("room no_breathe");
    gafn_room_drain		= get_affect_gafn("room drain");
    gafn_room_mare		= get_affect_gafn("room mare");
    gafn_room_enclosed		= get_affect_gafn("room enclosed");
    gafn_room_no_morgue		= get_affect_gafn("room no_morgue");
    gafn_room_fatigue		= get_affect_gafn("room fatigue");
    gafn_room_destructive		= get_affect_gafn("room destructive");

    gafn_obj_blade_affect		= get_affect_gafn("object blade affect");


   /* Load the cults... */
   
    log_string("Loading Cults...");
    load_cults();

   /* Load the races... */
   
    log_string("Loading Races...");
    load_races();
    
   /* Load the professions... */
   
    log_string("Loading Professions...");
    load_professions();
    
   /* Load the societies... */
   
    log_string("Loading Societies...");
    load_societies();
    
    log_string("Loading Society Members...");
    load_society_members();

   /* Load the socials... */
    
    log_string("Loading socials");
    load_socials();

   /* Load the areas... */
     
    {
	FILE *fpList;

        log_string("Loading Areas...");

	fpList = fopen( AREA_LIST, "r" );

	if ( fpList == NULL ) {
            bug("Unable to open area file!", 0);
	    perror( AREA_LIST );
	    exit( 1 );
	}

	for ( ; ; )
	{
	    strcpy( strArea, fread_word( fpList ) );
	    if ( strArea[0] == '$' )
		break;

	    if ( strArea[0] == '-' )
	    {
		fpArea = stdin;
	    }
	    else
	    {
                sprintf(buf, "...Loading: %s", strArea);
                log_string(buf);
		if ( ( fpArea = fopen( strArea, "r" ) ) == NULL )
		{
		    perror( strArea );
		    exit( 1 );
		}
	    }

	    for ( ; ; )
	    {
		char *word;
		char letter;

		letter = fread_letter( fpArea );

		if ( letter != '#' ) {
                    printf("Found letter %c\n", letter);
		    bug( "Boot_db: # not found.", 0 );
		    exit( 1 );
		}

		word = fread_word( fpArea );

		     if ( word[0] == '$'               )                 break;
		else if ( !str_cmp( word, "AREA"     ) ) load_area    (fpArea);
                                else if ( !str_cmp( word, "AREADATA" ) ) new_load_area(fpArea);
		else if ( !str_cmp( word, "HELPS"    ) ) load_helps   (fpArea);
		else if ( !str_cmp( word, "MOBOLD"   ) ) load_old_mob (fpArea);
		else if ( !str_cmp( word, "MOBILES"  ) ) load_mobiles (fpArea);
                                else if ( !str_cmp( word, "MOBPROGS" ) ) load_mobprogs( fpArea );
		else if ( !str_cmp( word, "OBJOLD"   ) ) load_old_obj (fpArea);
	  	else if ( !str_cmp( word, "OBJECTS"  ) ) load_objects (fpArea, FALSE);
	  	else if ( !str_cmp( word, "NEWOBJECTS"  ) ) load_objects (fpArea, TRUE);
		else if ( !str_cmp( word, "RESETS"   ) ) load_resets  (fpArea);
		else if ( !str_cmp( word, "ROOMS"    ) ) load_rooms   (fpArea);
		else if ( !str_cmp( word, "SHOPS"    ) ) load_shops   (fpArea);
		else if ( !str_cmp( word, "SPECIALS" ) ) load_specials(fpArea);
		else {
		    bug( "Boot_db: bad section name.", 0 );
		    exit( 1 );
		}
	    }

	    if ( fpArea != stdin )
		fclose( fpArea );
	    fpArea = NULL;
	}
	fclose( fpList );
    }

    log_string("...areas loaded");

   /* Loading banks... */
    log_string("Loading Banks...");
    load_banks(); 

    log_string("Loading Shares...");
    load_shares(); 

   /* Set up the exp table... */

    log_string("Building exp table...");
    build_exp_table();

   /* Load lease information... */

    log_string("Loading Lease Information...");
    load_leases();

   /* Load disable commands... */

    log_string("Loading Disabled Commands...");
    load_disable();

   /* Generate wizcommand table... */

    log_string("Building wizcommands...");

    gen_wiz_table();

   /* Load and activate profile... */
    log_string("Loading profile...");
    load_profile();

   /* Load Bans... */
    log_string("Loading bans...");
    load_ban();

   /* Load Help Index... */

    log_string("Loading Help Index..."); 
    load_hindex();

    log_string("Loading Rumors..."); 
    load_rumors();

   /* Check/fix the exits... */

    log_string("Resolving exits...");

    fix_exits( );

    log_string("Checking recalls...");

    check_area_recalls();

    log_string("Checking profession starts...");

    check_prof_starts();

   /* End of boot phase... */

    log_string("Boot complete, preparing...");

    fBootDb = FALSE;

   /* Convert old format objects... */

    log_string("Converting objects...");
 
    convert_objects();

   /* Reset the areas... */

    log_string("Resetting areas...");
    area_update();
    save_artifacts_forward();
    
   /* Load the message boards... */

    log_string("Loading boards..."); 
    load_boards();
    save_notes();

   /* Load the jukebox... */

    log_string("Loading songs..."); 
    load_songs();

   /* Load Passport orders... */
    log_string("Loading Passports...");
    load_passport();

   /* Load the Partners... */

    log_string("Loading partners..."); 
    load_partners();

   /* Enable mob triggers... */
   
    MOBtrigger=TRUE;

   /* Default help greeting... */

    if ( !help_greeting )    {
        bug( "boot_db: No help_greeting read.", 0 );
        help_greeting = "By what name do you wish to be known ? ";
    }

    if ( !help_impgreeting )    {
        bug( "boot_db: No help_impgreeting read.", 0 );
        help_impgreeting = "By what name do you wish to be known ? ";
    }

    log_string("...Database loaded");

   /* Update the weather... */

    weather_update(TRUE);

    auction = (AUCTION_DATA *) malloc (sizeof(AUCTION_DATA));
    if (auction == NULL) {
	bug ("malloc'ing AUCTION_DATA didn't give %d bytes",sizeof(AUCTION_DATA));
	exit (1);
    }
    auction->item = NULL;
    startup = FALSE;

    if (fCopyOver) copyover_recover();
    return;
}



/*
 * Snarf an 'area' header line.
 */

void load_area( FILE *fp ) {
AREA_DATA *pArea;

    pArea               = (AREA_DATA *) alloc_perm( sizeof( *pArea ) );

    pArea->area_flags   = AREA_LOADING;

    pArea->vnum         = top_area;

    pArea->version      = VERSION_ROM;

    pArea->filename     = fread_string(fp);
    pArea->copyright    = fread_string(fp);
    pArea->name         = fread_string(fp);
    pArea->lvnum        = fread_number_long(fp);
    pArea->uvnum        = fread_number_long(fp);
 
    pArea->security     = 5;
    pArea->builders     = str_dup( "None" );
    pArea->map          = str_dup("None");
    pArea->age          = 15;
    pArea->nplayer      = 0;
    pArea->empty        = FALSE;

    pArea->recall 	= 0;
    pArea->respawn	= 0;
    pArea->morgue	= 0;
    pArea->dream	= 0;
    pArea->mare	= 0;
    pArea->prison	= 0;
    pArea->invasion     = FALSE;
    pArea->martial     = 0;
    pArea->climate 	= CLIMATE_EUROPEAN;

    if ( !area_first ) {
        area_first = pArea;
    }

    if (  area_last != NULL ) {
        area_last->next = pArea;
        REMOVE_BIT(area_last->area_flags, AREA_LOADING);  
    }

    area_last   = pArea;
    pArea->next = NULL;

    top_area++;

    return;
}

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                \
                if ( !str_cmp( word, literal ) )    \
                {                                   \
                    field  = value;                 \
                    fMatch = TRUE;                  \
                    break;                          \
                                }

#define SKEY( string, field )                       \
                if ( !str_cmp( word, string ) )     \
                {                                   \
                    free_string( field );           \
                    field = fread_string( fp );     \
                    fMatch = TRUE;                  \
                    break;                          \
                                }



void new_load_area( FILE *fp ) {
    AREA_DATA *pArea;
    const char      *word;
    bool      fMatch;
    int i;
    int max_c = 0;

    pArea               = (AREA_DATA *) alloc_perm( sizeof(*pArea) );

    pArea->version      = VERSION_CTHULHU_0;

    pArea->age          = 15;
    pArea->nplayer      = 0;
    pArea->filename     = str_dup( strArea );
    pArea->vnum         = top_area;
    pArea->name         = str_dup( "New Area" );
    pArea->copyright    = str_dup( "Not specified" );
    pArea->builders     = str_dup( "" );
    pArea->map          = str_dup("None");
    pArea->security     = 9;               
    pArea->lvnum        = 0;
    pArea->uvnum        = 0;
    pArea->area_flags   = 0;
    pArea->zone 	= 0;

    pArea->recall 	= 0;
    pArea->respawn 	= 0;
    pArea->morgue 	= 0;
    pArea->dream 	= 0;
    pArea->mare 	= 0;
    pArea->prison 	= 0;
    for (i = 0; i < MAX_CURRENCY; i++) pArea->worth[i] = -1;
    pArea->invasion     = FALSE;
    pArea->martial = 0;
    pArea->climate	= CLIMATE_EUROPEAN;

    word = "End";

    for ( ; ; ) {
       word   = feof( fp ) ? "End" : fread_word(fp);
       fMatch = FALSE;

       switch ( UPPER(word[0]))  {
           case 'B':
             SKEY( "Builders", pArea->builders );
             break;
           case 'C':
             SKEY( "Copyright", pArea->copyright );
             KEY( "Climate", pArea->climate, fread_number( fp ) );
             if (!str_cmp( word, "Currency" )) {
                   if (max_c > 0) {
                       for (i = 0; i < max_c; i++) {
                             pArea->worth[i] = fread_number(fp);
                       }
                   }
             }
             break;
           case 'D':
             KEY( "Dream", pArea->dream, fread_number( fp ) );
             break;
           case 'E':
              if ( !str_cmp( word, "End" ) )
              {
                 fMatch = TRUE;
                 if ( area_first == NULL )
                    area_first = pArea;
                 if ( area_last  != NULL )
                    area_last->next = pArea;
                 area_last   = pArea;
                 pArea->next = NULL;
                 top_area++;
                 return;
             }
             break;
           case 'F':
             KEY( "Flags", pArea->area_flags, fread_number(fp) ); 
             break;
           case 'M':
             KEY( "Morgue", pArea->morgue, fread_number( fp ) );
             SKEY( "Map", pArea->map);
             KEY( "Mare", pArea->mare, fread_number( fp ) );
             KEY( "Martial", pArea->martial, fread_number( fp ) );
             KEY( "MAX_CURRENCY", max_c, fread_number( fp ) );
             break;
           case 'N':
             SKEY( "Name", pArea->name );
             break;
           case 'P':
             KEY( "Prison", pArea->prison, fread_number(fp) );
             break;
           case 'S':
             KEY( "Security", pArea->security, fread_number(fp) );
             break;
           case 'R':
             KEY( "Recall", pArea->recall, fread_number( fp ) );
             KEY( "Respawn", pArea->respawn, fread_number( fp ) );
             break;
           case 'V':
             if ( !str_cmp( word, "VNUMs" ) )        {
                 pArea->lvnum = fread_number( fp );
                 pArea->uvnum = fread_number( fp );
		 break;
             }
             if ( !str_cmp( word, "Version" ) )
             {
               pArea->version = fread_number( fp );

               if ( pArea->version > VERSION_CTHULHU_MAX
                 || pArea->version < VERSION_ROM ) {
                 bug("Bad area version value %d!",pArea->version);
                 exit(1);
               }

	       break;
             }
             break;
           case 'Z':
             if (!str_cmp(word, "Zone")) {
	       pArea->zone = fread_number(fp);
              
               if ( pArea->zone < 0 
                 || pArea->zone >= NUM_ZONES ) {
                 bug("Bad zone (%d)! Changed to 0.", pArea->zone);
                 pArea->zone = 0;
               }
             }
	     break;
        }
    }

}

/*
 * Sets vnum range for area using OLC protection features.
 */
void assign_area_vnum( int vnum )
{
    if ( area_last->lvnum == 0 || area_last->uvnum == 0 )
        area_last->lvnum = area_last->uvnum = vnum;
    if ( vnum != URANGE( area_last->lvnum, vnum, area_last->uvnum ) ) {
        if ( vnum < area_last->lvnum ) {
            area_last->lvnum = vnum;
        } else {
            area_last->uvnum = vnum;
        }
    } 
    return;
}


/*
 * Snarf a help section.
 */
void load_helps( FILE *fp ) {
HELP_DATA *pHelp;

    for ( ; ; ) {
	pHelp		= (HELP_DATA *) alloc_perm( sizeof(*pHelp) );
	pHelp->level	= fread_number( fp );
	pHelp->keyword	= fread_string( fp );
	if ( pHelp->keyword[0] == '$' ) break;

	pHelp->text	= fread_string(fp);

	if ( !str_cmp( pHelp->keyword, "greeting" ))	     help_greeting = pHelp->text;
	if ( !str_cmp( pHelp->keyword, "impgreeting"))  help_impgreeting = pHelp->text;

	if (!help_first) help_first = pHelp;
	if (help_last) help_last->next = pHelp;

	help_last	= pHelp;
	pHelp->next	= NULL;
	top_help++;
    }

    return;
}



/*
 * Snarf a mob section.  old style 
 */
void load_old_mob( FILE *fp ) {
    MOB_INDEX_DATA *pMobIndex;
    /* for race updating */
    int race;
    char name[MAX_STRING_LENGTH];

/* before the loop? I assume they mean before the for loop. Hope so. */

    if ( !area_last ) {
        bug( "Load_mobiles: no #AREA seen yet.", 0 );
        exit( 1 );
    }

    for ( ; ; ) {
	long vnum;
	char letter;
	int iHash;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_mobiles: # not found.", 0 );
	    exit( 1 );
	}

	vnum				= fread_number_long( fp );
	if ( vnum == 0 )  break;

	fBootDb = FALSE;
	if ( get_mob_index( vnum ) != NULL ) {
	    bug( "Load_mobiles: vnum %d duplicated.", vnum );
	    exit( 1 );
	}
	fBootDb = TRUE;

	pMobIndex			= (MOB_INDEX_DATA *) alloc_perm( sizeof(*pMobIndex) );
	pMobIndex->vnum		= vnum;
                pMobIndex->area                 	= area_last;
	pMobIndex->new_format		= FALSE;
	pMobIndex->player_name		= fread_string( fp );
	pMobIndex->short_descr		= fread_string( fp );
	pMobIndex->long_descr		= fread_string( fp );
	pMobIndex->description		= fread_string( fp );

	pMobIndex->long_descr[0]	= UPPER(pMobIndex->long_descr[0]);
	pMobIndex->description[0]	= UPPER(pMobIndex->description[0]);

	pMobIndex->act			= fread_flag_long( fp ) | ACT_IS_NPC;
	pMobIndex->affected_by		= fread_flag_long( fp );
	pMobIndex->pShop		= NULL;
	pMobIndex->alignment		= fread_number( fp );
	pMobIndex->group		= 0;
	letter				= fread_letter( fp );
	pMobIndex->level		= number_fuzzy( fread_number( fp ) );

	/*
	 * The unused stuff is for imps who want to use the old-style
	 * stats-in-files method.
	 */
					  fread_number( fp );	/* Unused */
					  fread_number( fp );	/* Unused */
					  fread_number( fp );	/* Unused */
	/* 'd'		*/		  fread_letter( fp );	/* Unused */
					  fread_number( fp );	/* Unused */
	/* '+'		*/		  fread_letter( fp );	/* Unused */
					  fread_number( fp );	/* Unused */
					  fread_number( fp );	/* Unused */
	/* 'd'		*/		  fread_letter( fp );	/* Unused */
					  fread_number( fp );	/* Unused */
	/* '+'		*/		  fread_letter( fp );	/* Unused */
					  fread_number( fp );	/* Unused */
                pMobIndex->gold                 	= fread_number( fp );
	/* xp can't be used! */		  fread_number( fp );	/* Unused */
	pMobIndex->start_pos		= fread_number( fp );
	pMobIndex->default_pos		= fread_number( fp );

  	if (pMobIndex->start_pos < POS_SLEEPING)       pMobIndex->start_pos = POS_STANDING;
	if (pMobIndex->default_pos < POS_SLEEPING)   pMobIndex->default_pos = POS_STANDING;

	/*
	 * Back to meaningful values.
	 */
	pMobIndex->sex			= fread_number( fp );

    	/* compute the race BS */
   	one_argument(pMobIndex->player_name,name);
 
   	if (name[0] == '\0' || (race =  race_lookup(name)) == 0) {
                           race = race_lookup("human");
                }
         
        if ( race == 0 ) {

            /* fill in with blanks */
            pMobIndex->race = race_lookup("human");
            pMobIndex->off_flags = OFF_DODGE|OFF_DISARM|OFF_TRIP|ASSIST_VNUM;
            pMobIndex->imm_flags = 0;
            pMobIndex->res_flags = 0;
            pMobIndex->vuln_flags = 0;
            pMobIndex->form = FORM_EDIBLE|FORM_SENTIENT|FORM_BIPED|FORM_MAMMAL;
            pMobIndex->parts = PART_HEAD|PART_ARMS|PART_LEGS|PART_HEART|
                               PART_BRAINS|PART_GUTS;
        } else {
            pMobIndex->race = race;
            pMobIndex->off_flags  = OFF_DODGE|OFF_DISARM|OFF_TRIP|ASSIST_RACE|race_array[race].off;
            pMobIndex->imm_flags  = race_array[race].imm;
            pMobIndex->res_flags  = race_array[race].res;
            pMobIndex->vuln_flags = race_array[race].vuln;
            pMobIndex->form	  = race_array[race].form;
            pMobIndex->parts	  = race_array[race].parts;
        }

	if ( letter != 'S' ) {
	    bug( "Load_mobiles: vnum %d non-S.", vnum );
	    exit( 1 );
	}

        pMobIndex->skills = NULL;

        convert_mobile( pMobIndex );          

	iHash			= vnum % MAX_KEY_HASH;
	pMobIndex->next		= mob_index_hash[iHash];
	mob_index_hash[iHash]	= pMobIndex;
	top_mob_index++;
                top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;
                assign_area_vnum( vnum );                            
	kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
    }

    return;
}


/*
 * Snarf an obj section.  old style 
 */
void load_old_obj( FILE *fp ) {
    OBJ_INDEX_DATA *pObjIndex;
    EXTRA_DESCR_DATA *ed;
    ECOND_DATA *new_ec;

    if ( !area_last )  {
        bug( "Load_objects: no #AREA seen yet.", 0 );
        exit( 1 );
    }

    for ( ; ; )   {
	long vnum;
	char letter;
	int iHash;

	letter				= fread_letter( fp );
	if ( letter != '#' ) {
	    bug( "Load_objects: # not found.", 0 );
	    exit( 1 );
	}

	vnum				= fread_number_long( fp );
	if ( vnum == 0 )  break;

	fBootDb = FALSE;
	if ( get_obj_index( vnum ) != NULL ) {
	    bug( "Load_objects: vnum %d duplicated.", vnum );
	    exit( 1 );
	}
	fBootDb = TRUE;

	pObjIndex			= (OBJ_INDEX_DATA *) alloc_perm( sizeof(*pObjIndex) );
	pObjIndex->vnum		= vnum;
                pObjIndex->area                 		= area_last;        
	pObjIndex->new_format		= FALSE;
	pObjIndex->reset_num	 	= 0;
	pObjIndex->name			= fread_string( fp );
	pObjIndex->short_descr		= fread_string( fp );
	pObjIndex->description		= fread_string( fp );
	/* Action description */	  fread_string( fp );

	pObjIndex->short_descr[0]	= LOWER(pObjIndex->short_descr[0]);
	pObjIndex->description[0]	= UPPER(pObjIndex->description[0]);

	pObjIndex->item_type		= fread_number( fp );
	pObjIndex->extra_flags		= fread_flag_long( fp );
	pObjIndex->wear_flags		= fread_flag( fp );
	pObjIndex->value[0]		= fread_number( fp );
	pObjIndex->value[1]		= fread_number( fp );
	pObjIndex->value[2]		= fread_number( fp );
	pObjIndex->value[3]		= fread_number( fp );
	pObjIndex->value[4]		= 0;
	pObjIndex->level			= 0;
	pObjIndex->condition 		= 100;
	pObjIndex->weight		= fread_number( fp );
	pObjIndex->cost			= fread_number( fp );	
	/* Cost per day */		  fread_number( fp );


	if (pObjIndex->item_type == ITEM_PORTAL                 
                && pObjIndex->vnum >=  mud.pattern[0]
                && pObjIndex->vnum <= mud.pattern[1]) {
                        pObjIndex->weight=100; 
                        pObjIndex->condition=100; 
                }

        ed = NULL;

	for ( ; ; )	{
	    char letter;

	    letter = fread_letter( fp );

	    if ( letter == 'A' )	    {
		AFFECT_DATA *paf;

		paf			= new_affect();

		paf->type		= -1;
		paf->level		= 20; /* RT temp fix */
		paf->duration		= -1;
		paf->location		= fread_number( fp );
		paf->modifier		= fread_number( fp );
		paf->bitvector		= 0;

		paf->next		= pObjIndex->affected;
		pObjIndex->affected	= paf;

		top_affect++;
	    }

	    else if ( letter == 'E' )
	    {
		ed			= (EXTRA_DESCR_DATA *) alloc_perm( sizeof(*ed) );
		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= pObjIndex->extra_descr;
		pObjIndex->extra_descr	= ed;
                ed->ec 			= NULL;
                ed->deed                = NULL; 
		top_ed++;

	    } else if ( letter == 'C' ) {
                if (ed == NULL) {
                  bug("C before E in object!", 0);
                } else {
                  new_ec = read_econd(fp);
                  new_ec->next = ed->ec;
                  ed->ec = new_ec;
                }

	    } else {
		ungetc( letter, fp );
		break;
	    }
	}

        /* fix armors */
        if (pObjIndex->item_type == ITEM_ARMOR)        {
            pObjIndex->value[1] = pObjIndex->value[0];
            pObjIndex->value[2] = pObjIndex->value[1];
        }

	/*
	 * Translate spell "slot numbers" to internal "skill numbers."
	 */
	switch ( pObjIndex->item_type ) {
	case ITEM_PILL:
	case ITEM_POTION:
	case ITEM_SCROLL:
	    pObjIndex->value[1] = get_effect_for_slot( pObjIndex->value[1] );
	    pObjIndex->value[2] = get_effect_for_slot( pObjIndex->value[2] );
	    pObjIndex->value[3] = get_effect_for_slot( pObjIndex->value[3] );
	    pObjIndex->value[4] = get_effect_for_slot( pObjIndex->value[4] );
	    break;

	case ITEM_STAFF:
	case ITEM_WAND:
	    pObjIndex->value[3] = get_effect_for_slot( pObjIndex->value[3] );
	    break;

        case ITEM_CONTAINER:
           if ( pObjIndex->value[3] <= 25 
             || pObjIndex->value[3] >= 200 ) {
             pObjIndex->value[3] = 100;
           }
           break;
	}

	iHash			= vnum % MAX_KEY_HASH;
	pObjIndex->next		= obj_index_hash[iHash];
	obj_index_hash[iHash]	= pObjIndex;
	top_obj_index++;
        top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;   /* OLC */
        assign_area_vnum( vnum );                                   /* OLC */
    }

    return;
}



/*
 * Snarf a reset section.
 */

/*
 * Adds a reset to a room.  OLC
 * Similar to add_reset in olc.c
 */
void new_reset( ROOM_INDEX_DATA *pR, RESET_DATA *pReset ) {
RESET_DATA *pr;

    if ( !pR ) return;

    pr = pR->reset_last;

    if ( !pr ) {
        pR->reset_first = pReset;
        pR->reset_last  = pReset;

    } else {
        pR->reset_last->next = pReset;
        pR->reset_last       = pReset;
        pR->reset_last->next = NULL;
    }
    top_reset++;
    return;
}

/*
 * Snarf a reset section.       Changed for OLC.
 */
void load_resets( FILE *fp ) {
RESET_DATA  *pReset;
int         iLastRoom = 0;
int         iLastObj  = 0;
EXIT_DATA       *pexit;
ROOM_INDEX_DATA *pRoomIndex;
char             letter;

    if ( !area_last ) {
        bug( "Load_resets: no #AREA seen yet.", 0 );
        exit( 1 );
    }

    pReset = NULL;

    letter = '*';

    for ( ; ; )  {

        letter = fread_letter( fp );

        if ( letter == 'S' ) {
            break;
        }

        if ( letter == '*' ) {
            fread_to_eol( fp );
            continue;
        }

        pReset          = (RESET_DATA *) alloc_perm( sizeof( *pReset ) );

        pReset->command = letter;

        /* if_flag */     fread_number( fp );

        pReset->arg1    = fread_number_long( fp );
        pReset->arg2    = fread_number( fp );

        pReset->arg3    = ( letter == 'G' || letter == 'R' ) ? 0 : fread_number_long( fp );
        fread_to_eol( fp );
        pReset->count	= 0;

       /* Validate/default the reset command... */
   
        switch (pReset->command) {
          case 'G': 
          case 'O':
          case 'M':
          case 'P':
  	    if (pReset->arg2 < -1) {
 	      pReset->arg2 = 1;
            }
            
            if (pReset->arg2 == 0) {
              pReset->arg2 = 1;
            }

            if (area_last->version == VERSION_ROM) {
              if (pReset->arg2 > 1) {
                pReset->arg2 = 1;
              }
            } else {
              if (pReset->arg2 > 10) {
                pReset->arg2 = 10;
              }
            }

            break;
 
          case 'E':
            pReset->arg2 = 1;
            
            break;

          default:
            break;
        }
  
        /*
         * Validate parameters.
         */
        switch ( letter )
        {
        default:
            bug( "Load_resets: bad command '%c'.", letter );
            exit( 1 );
            break;

        case 'M':
            pRoomIndex = get_room_index ( pReset->arg3 );

            if ( pRoomIndex == NULL ) {
                bug( "Load_resets: 'M' for room %d: Room not found!", pReset->arg3 );
                exit( 1 );
            }

            new_reset( pRoomIndex, pReset );
            iLastRoom = pReset->arg3;

            break;

        case 'O':
            pRoomIndex = get_room_index ( pReset->arg3 );

            if ( pRoomIndex == NULL ) {
                bug( "Load_resets: 'O' for room %d: Room not found!", pReset->arg3 );
                exit( 1 );
            }

            new_reset( pRoomIndex, pReset );
            iLastObj = pReset->arg3;

            break;

        case 'P':

            pRoomIndex = get_room_index ( iLastObj );

            if ( pRoomIndex != NULL ) {
                new_reset( pRoomIndex, pReset );
            }

            break;

        case 'G':
        case 'E':

            pRoomIndex = get_room_index ( iLastRoom );

            if ( pRoomIndex != NULL ) {
                new_reset( pRoomIndex, pReset );
                iLastObj = iLastRoom;
            }

            break;

        case 'D':
            pRoomIndex = get_room_index( pReset->arg1 );

            if ( pRoomIndex == NULL ) {
                bug( "Load_resets: 'D' for room %d: Room not found!", pReset->arg1 );
                exit( 1 );
            }

            if (pReset->arg2 < 0
                || pReset->arg2 >= DIR_MAX
                || !( pexit = pRoomIndex->exit[pReset->arg2] )
                || !IS_SET( pexit->rs_flags, EX_ISDOOR ) ) {
                bug( "Load_resets: 'D': exit %d not door.", pReset->arg2 );
                exit(1);
            }

            switch ( pReset->arg3 ) {
                default:
                  bug( "Load_resets: 'D': bad 'locks': %d." , pReset->arg3);
                case 0:
                        break;
                case 1: SET_BIT( pexit->rs_flags, EX_CLOSED );
                        break;
                case 2: SET_BIT( pexit->rs_flags, EX_LOCKED );
                        SET_BIT( pexit->rs_flags, EX_CLOSED );
                        break;
            }

            break;		

        case 'R':

            if ( pReset->arg2 < 0 
              || pReset->arg2 >= DIR_MAX ) {
                bug( "Load_resets: 'R': bad exit %d.", pReset->arg2 );
                exit( 1 );
            }

            pRoomIndex = get_room_index( pReset->arg1 );

            if ( pRoomIndex == NULL ) {
                bug( "Load_resets: 'R' for room %d: Room not found!", pReset->arg1 );
                exit( 1 );
            }

            new_reset( pRoomIndex, pReset );

            break;
        }
    }

    return;
}

void validate_resets(ROOM_INDEX_DATA *room) {

    RESET_DATA *reset;

    OBJ_INDEX_DATA *pCont;

    reset = room->reset_first;

    while (reset != NULL) {

       /* Check the reset mob/object exists... */

        switch ( reset->command ) {

          default:
            break;

          case 'R':
            break;

          case 'M':
            if ( get_mob_index( reset->arg1 ) == NULL ) {
              bug("Bad M reset for room %d", room->vnum);
              bug("Mobile %d not defined!", reset->arg1);
              exit(1); 
            } 
            break;

          case 'G':
          case 'E':
          case 'O':
            pCont = get_obj_index( reset->arg1 );
            if ( pCont == NULL ) {
              bug("Bad G/E/O reset for room %d", room->vnum);
              bug("Object %d not defined!", reset->arg1);
              exit(1); 
            } 
            break;

          case 'P':
            pCont = get_obj_index( reset->arg1 );
            if ( pCont == NULL ) {
              bug("Bad P reset for room %d", room->vnum);
              bug("Object %d not defined!", reset->arg1);
              exit(1); 
            } 
            pCont = get_obj_index( reset->arg3 );
            if ( pCont == NULL ) {
              bug("Bad P reset for room %d", room->vnum);
              bug("Container Object %d not defined!", reset->arg3);
              exit(1); 
            } 
            if (  pCont->item_type != ITEM_CONTAINER
               && pCont->item_type != ITEM_KEY_RING
               && pCont->item_type != ITEM_TREE
               && pCont->item_type != ITEM_CORPSE_NPC
               && pCont->item_type != ITEM_CAMERA ) {
              bug("Bad P reset for room %d", room->vnum);
              bug("Container Object %d is not an object container!", reset->arg3);
              return;
            }   
            break;

        }

       /* Get the next reset... */

        reset = reset->next;

        if (reset == room->reset_first) {
          reset = NULL;
        }
    } 

    return;
}

/* Extended (keyword based) mob load... */

void load_room_extended(FILE *fp, ROOM_INDEX_DATA *pRoomIndex) {
char *word;
TRACKS *tracks, *new_tracks;
bool done, ok;

 /* Ok, straight processing loop... */

  done = FALSE;

  tracks = NULL;
  new_tracks = NULL;

  while (!done) {
    word = fread_word(fp);

    ok = FALSE;

    switch (word[0]) {

     /* Affect */ 

      case 'A':

       /* Affect flags */

        if (!str_cmp(word, "Affect")) {
          ok = TRUE;

          pRoomIndex->affected_by = fread_flag(fp);
          
          break;
        }

        break;

     /* Dream */ 

      case 'D':

       /* Dream vnum */

        if (!str_cmp(word, "Dream")) {
          ok = TRUE;

          pRoomIndex->dream = fread_number(fp);
          
          break;
        }

       /* Day vnum */

        if (!str_cmp(word, "Day")) {
          ok = TRUE;

          pRoomIndex->day = fread_number(fp);
          
          break;
        }

        break;

     /* End */ 

      case 'E':

       /* End */

        if (!str_cmp(word, "End")) {
          ok = TRUE;
          done = TRUE; 
          break;
        }

        break;

     /* Image */ 

      case 'I':

       /* Image url */

        if (!str_cmp(word, "Image")) {
          ok = TRUE;

          pRoomIndex->image = str_dup(fread_word(fp));
          
          break;
        }

        break;

     /* Morgue */ 

      case 'M':
        if (!str_cmp(word, "Morgue")) {
          ok = TRUE;
          pRoomIndex->morgue = fread_number(fp);
        }

        if (!str_cmp(word, "Mare")) {
          ok = TRUE;
          pRoomIndex->mare = fread_number(fp);
          break;
        }
        break;

     /* Night */ 

      case 'N':

       /* Night vnum */

        if (!str_cmp(word, "Night")) {
          ok = TRUE;

          pRoomIndex->night = fread_number(fp);
          
          break;
        }

        break;

     /* PTracks */

      case 'P':

        if (!str_cmp(word, "PTracks")) {
          ok = TRUE;

          new_tracks = read_tracks(fp);

          if (new_tracks != NULL) {
            if (tracks == NULL) {
              pRoomIndex->ptracks = new_tracks;
            } else {
              tracks->next = new_tracks;
            }
            tracks = new_tracks;
          } else {
            bug("Bad PTracks in room %d!", pRoomIndex->vnum);
            exit(1);
          }   
        }
  
        break;   

     /* Recall, Respawn */ 

      case 'R':

       /* Recall vnum */

        if (!str_cmp(word, "Recall")) {
          ok = TRUE;

          pRoomIndex->recall = fread_number(fp);
          
          break;
        }

       /* Respawn vnum */

        if (!str_cmp(word, "Respawn")) {
          ok = TRUE;

          pRoomIndex->respawn = fread_number(fp);
          
          break;
        }

        break;

     /* X */

      case 'X':

        if (!str_cmp(word, "X")) {
          ok = TRUE;
          break;
        }
 
        break;

      default:
        break;
    }

    if (!ok) {
      log_string(word);
      bug("Unrecognized keyword in extended mob data!", 0);
    }
  }

 /* All done... */

  return;
}



/*
 * Snarf a room section.
 */
void load_rooms( FILE *fp ) {
ROOM_INDEX_DATA *pRoomIndex;
EXTRA_DESCR_DATA *ed;
ECOND_DATA *new_ec;
DEED *new_deed;
EXIT_DATA *pexit;
COND_DEST_DATA *cdd;
GADGET_DATA *gadget;
GADGET_TRANS_DATA *gdt; 
CURRENT *new_cur;

    if ( area_last == NULL )    {
	bug( "Load_resets: no #AREA seen yet.", 0 );
	exit( 1 );
    }

    for ( ; ; )    {
	VNUM vnum;
	char letter;
	int door;
	int iHash;

	letter				= fread_letter( fp );
	if ( letter != '#' )	{
	    bug( "Load_rooms: # not found.", 0 );
	    exit( 1 );
	}

       /* Last room should be followed by #0 */

	vnum				= fread_number_long( fp );
	if ( vnum == 0 ) {
	    break;
        }

	fBootDb = FALSE;
	if ( get_room_index( vnum ) != NULL ) {
	    bug( "Load_rooms: vnum %d duplicated.", vnum );
	    exit( 1 );
	}
	fBootDb = TRUE;

	pRoomIndex			= (ROOM_INDEX_DATA *) alloc_perm( sizeof(*pRoomIndex) );
	pRoomIndex->people		= NULL;
	pRoomIndex->contents		= NULL;
	pRoomIndex->extra_descr		= NULL;
	pRoomIndex->area		= area_last;
	pRoomIndex->vnum		= vnum;
	pRoomIndex->name		= fread_string( fp );
	pRoomIndex->description		= fread_string( fp );
	/* Area number */		  fread_number( fp );
	pRoomIndex->room_flags		= fread_flag_long( fp );
	pRoomIndex->sector_type		= fread_number( fp );
        	pRoomIndex->affected                        = NULL;

	if (area_last->version == VERSION_CTHULHU_0) {
                         pRoomIndex->affected_by = fread_flag( fp );
                } 

	pRoomIndex->light		= 0;
	pRoomIndex->room_rent		= 5000; 
	pRoomIndex->rented_by		= 0;
	pRoomIndex->paid_month		= 0;	
	pRoomIndex->paid_day		= 0;
	pRoomIndex->paid_year		= 0;

	for ( door = 0; door < DIR_MAX; door++ )  pRoomIndex->exit[door] = NULL;

                pRoomIndex->recall		= 0;
                pRoomIndex->respawn		= 0;
                pRoomIndex->morgue		= 0;
                pRoomIndex->dream		= 0;
                pRoomIndex->mare		= 0;
                pRoomIndex->night		= 0;
                pRoomIndex->day		= 0;
                pRoomIndex->heal_rate		= 100;
                pRoomIndex->mana_rate		= 100;
                pRoomIndex->distance		= -1;

        	ed = NULL;
        	pexit = NULL;
        	cdd = NULL;
        	gadget = NULL;
        	gdt = NULL;
        	new_cur = NULL; 

	for ( ; ; )	{
	    letter = fread_letter( fp );

           /* Room ends with an 'S'... */

	    if ( letter == 'S' )
		break;

	    if ( letter == 'D' )  {
		int locks;
                                int test_lock;
                                int carry_lock;   

		door = fread_number( fp );
		if ( door < 0 || door >= DIR_MAX ) {
		    bug( "Fread_rooms: vnum %d has bad door number.", vnum );
		    exit( 1 );
		}

		pexit			= (EXIT_DATA *) alloc_perm( sizeof(*pexit) );
		pexit->description	= fread_string( fp );
		pexit->keyword		= fread_string( fp );
		pexit->exit_info		= 0;
                                pexit->rs_flags         	= 0;
		locks			= fread_number( fp );
		pexit->key		= fread_number( fp );
		pexit->u1.vnum		= fread_number( fp );
                	pexit->orig_door        	= door;
                	pexit->invis_cond 	= NULL;
                	pexit->cond_dest 	= NULL;
                	pexit->transition 		= NULL;
                	pexit->condition        	= 100;

                                carry_lock = locks & EX_MASK_CARRY;
                                test_lock = locks - carry_lock;

                                switch ( test_lock ) {
               
                                      case EX_FILE_NONE:
                                           pexit->rs_flags = 0;
                                           break;
 
                                      case EX_FILE_DOOR: 
                                           pexit->rs_flags  = EX_ISDOOR;
		           break;

                                      case EX_FILE_RANDOM: 
                                           pexit->rs_flags  = EX_RANDOM;
		           break;

                                      case EX_FILE_DOOR_PP: 
                                           pexit->rs_flags  = EX_ISDOOR | EX_PICKPROOF; 
		           break;

		      case EX_FILE_DOOR_H: 
                                            pexit->rs_flags  = EX_ISDOOR | EX_HIDDEN;
		            break;

                                      case EX_FILE_DOOR_PP_H: 
                                            pexit->rs_flags  = EX_ISDOOR | EX_PICKPROOF | EX_HIDDEN;
		            break;

                                      case EX_FILE_DOOR_NP: 
                                            pexit->rs_flags  = EX_ISDOOR | EX_NO_PASS;
		            break;

                                      case EX_FILE_DOOR_PP_NP: 
                                            pexit->rs_flags  = EX_ISDOOR | EX_PICKPROOF | EX_NO_PASS;
		            break;

                                      case EX_FILE_DOOR_H_NP: 
                                            pexit->rs_flags  = EX_ISDOOR | EX_HIDDEN | EX_NO_PASS;
		            break;

                                      case EX_FILE_DOOR_PP_H_NP: 
                                            pexit->rs_flags  = EX_ISDOOR | EX_PICKPROOF | EX_HIDDEN | EX_NO_PASS;
		            break;
                
                                      default:
                                            bug("Bad test lock %d", test_lock);
                                            pexit->rs_flags = EX_ISDOOR;
                                            break; 
                                }

                                pexit->rs_flags += carry_lock;

		pRoomIndex->exit[door]	= pexit;
		top_exit++;

	    } else if ( letter == 'I' ) {
                                new_ec = read_econd(fp);
                                new_ec->next = NULL;
                                if (pexit == NULL) {
                                      bug("I before D in room %d!", vnum);
                                      free_ec(new_ec);
                                } else {
                                      new_ec->next = pexit->invis_cond;
                                      pexit->invis_cond = new_ec;
                                }

	    } else if ( letter == 'O' )  {
		if (area_last->version == VERSION_CTHULHU_0 ) {
                                    cdd = read_cdd(fp);

                                    if (pexit == NULL) {
                                         bug("O before D in room %d!", vnum);
                                         free_cdd(cdd);
                                    } else {
                                         insert_cdd(cdd, pexit);
                                    }
                                } else {
	  	  if ( pRoomIndex->owner != NULL ) {
		    bug("Load_rooms: duplicate owner.",0);
		    exit(1);
		  }

		  pRoomIndex->owner = fread_string(fp);

                                }

	    } else if ( letter == 't' ) {
		pexit->transition = fread_string( fp );

	    } else if ( letter == 'Z' ) {
                cdd = read_cdd(fp);

                if (pexit == NULL) {
                  bug("Z before D in room %d!", vnum);
                  free_cdd(cdd);
                } else {
                  insert_cdd(cdd, pexit);
                }
	    }
	    else if ( letter == 'N' )
	    {
                new_ec = read_econd(fp);
                if (cdd == NULL) {
                  bug("N before O in room %d!", vnum);
                  free_ec(new_ec);
                } else {
                  new_ec->next = cdd->cond;
                  cdd->cond = new_ec;
                }
	    }
	    else if ( letter == 'E' )
	    {
		ed			= (EXTRA_DESCR_DATA *) alloc_perm( sizeof(*ed) );
		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= pRoomIndex->extra_descr;
                ed->ec 			= NULL;
                ed->deed		= NULL;  
		pRoomIndex->extra_descr	= ed;
		top_ed++;
	    }
	    else if ( letter == 'C' )
	    {
                new_ec = read_econd(fp);
                if (ed == NULL) {
                  bug("C before E in room %d!", vnum);
                  free_ec(new_ec);
                } else {
                  new_ec->next = ed->ec;
                  ed->ec = new_ec;
                }
	    }
	    else if ( letter == 'V' )
	    {
                new_deed = read_deed(fp);
                if (ed == NULL) {
                  bug("V before E in room %d!", vnum);
                  free_deed(new_deed);
                } else {
                  new_deed->next = ed->deed;
                  ed->deed = new_deed;
                }

	    }  else if ( letter == 'G' )   {
                gadget = read_gadget(fp);
                gadget->state = 0;
                gadget->next = pRoomIndex->gadgets;
                gadget->gdt = NULL;
                pRoomIndex->gadgets = gadget;

	    } else if ( letter == 'L' ) {
                         gadget->link_room = fread_number(fp);
                         gadget->link_id = fread_number(fp);

	    } else if ( letter == 'T' ) {
                gdt = read_gdt(fp);
                if (gadget == NULL) {
                  bug("T before G in room %d!", vnum);
                  free_gdt(gdt);
                } else {
                  insert_gdt(gdt, gadget);
                }
	    }
	    else if ( letter == 'A' )
	    {
                new_ec = read_econd(fp);
                if (gdt == NULL) {
                  bug("A before T in room %d!", vnum);
                  free_ec(new_ec);
                } else {
                  new_ec->next = gdt->cond;
                  gdt->cond = new_ec;
                }
	    }
	    else if ( letter == 'R' )
	    {
                new_cur = read_current(fp);
                pRoomIndex->currents = 
                              insert_current(pRoomIndex->currents, new_cur);
	    }
	    else if ( letter == 'U' )
	    {
                new_ec = read_econd(fp);
                if (new_cur == NULL) {
                  bug("U before R in room %d!", vnum);
                  free_ec(new_ec);
                } else {
                  new_ec->next = new_cur->cond;
                  new_cur->cond = new_ec;
                }
	    }
	    else if ( letter == 'B' )
	    {
                pRoomIndex->subarea = fread_number(fp);
	    }
	    else if ( letter == 'H' )
	    {
                pRoomIndex->heal_rate = fread_number(fp);
                letter = fread_letter(fp);
		if (letter != 'M') {
                  bug("Bad Heal/Mana line!",0);
                  exit(1);
                }
                pRoomIndex->mana_rate = fread_number(fp);
	    }
	    else if ( letter == 'X' )
	    {
                load_room_extended(fp, pRoomIndex);
	    }
	    else
	    {
		bug( "Load_rooms: vnum %d has flag not 'DECISONAGTVBHURZ'.", vnum );
		exit( 1 );
	    }
	}

	iHash			= vnum % MAX_KEY_HASH;
	pRoomIndex->next	= room_index_hash[iHash];
	room_index_hash[iHash]	= pRoomIndex;
	top_room++;
        top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room; /* OLC */
        assign_area_vnum( vnum );                                    /* OLC */

    }

    return;
}



/*
 * Snarf a shop section.
 */
void load_shops( FILE *fp ) {
SHOP_DATA *pShop;
char * word;
char arg[MAX_INPUT_LENGTH];
int currency;


    for ( ; ; )    {
	MOB_INDEX_DATA *pMobIndex;
	int iTrade;

	pShop = (SHOP_DATA *) alloc_perm( sizeof(*pShop) );
                pShop->currency = -1;

                word = str_dup(fread_string_eol(fp));

                word = one_argument(word, arg);
	pShop->keeper		= atol(arg);
	if ( pShop->keeper == 0 )  break;

	for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ ) {
                       word = one_argument(word, arg);
                       pShop->buy_type[iTrade]	= atoi(arg);
                }

                word = one_argument(word, arg);
	pShop->profit_buy	= atoi(arg);

                word = one_argument(word, arg);
	pShop->profit_sell	= atoi(arg);

                word = one_argument(word, arg);
	pShop->open_hour	= atoi(arg);

                word = one_argument(word, arg);
	pShop->close_hour	= atoi(arg);
                
                
                word = one_argument(word, arg);
                if (arg[0] !='\0') {
                   if (is_number(arg)) {
                      currency = atoi(arg);
                      if (currency >= 0 && currency < MAX_CURRENCY) pShop->currency = currency;
                   }
                }                     

                word = one_argument(word, arg);
                if (arg[0] !='\0') {
                   if (is_number(arg)) {
                      currency = atoi(arg);
                      if (currency > 0 && currency < 100) pShop->haggle = currency;
                      if (pShop->haggle <= 0 || pShop->haggle >= 100) pShop->haggle = 50;
                   }
                }                     

	pMobIndex		= get_mob_index( pShop->keeper );
	pMobIndex->pShop	= pShop;

	if ( shop_first == NULL )
	    shop_first = pShop;
	if ( shop_last  != NULL )
	    shop_last->next = pShop;

	shop_last	= pShop;
	pShop->next	= NULL;
	top_shop++;
    }

    return;
}


/*
 * Snarf spec proc declarations.
 */
void load_specials( FILE *fp )
{
    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	char letter;

	switch ( letter = fread_letter( fp ) )
	{
	default:
	    bug( "Load_specials: letter '%c' not *MS.", letter );
	    exit( 1 );

	case 'S':
	    return;

	case '*':
	    break;

	case 'M':
	    pMobIndex		= get_mob_index	( fread_number ( fp ) );
	    pMobIndex->spec_fun	= spec_lookup	( fread_word   ( fp ) );
	    if ( pMobIndex->spec_fun == 0 )
	    {
		bug( "Load_specials: 'M': vnum %d.", pMobIndex->vnum );
		exit( 1 );
	    }
	    break;
	}

	fread_to_eol( fp );
    }
}



/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits( void )
{
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pexit;
    int iHash;
    int door;
    bool fexit;

    COND_DEST_DATA *cdd;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pRoomIndex  = room_index_hash[iHash];
	      pRoomIndex != NULL;
	      pRoomIndex  = pRoomIndex->next )
	{

           /* Work out where all the exits go... */

	    fexit = FALSE;
	    for ( door = 0; door < DIR_MAX; door++ ) {
	
               /* Find the exit... */
	
		pexit = pRoomIndex->exit[door];

		if ( pexit == NULL ) {
                  continue;
                }
 
               /* Fix it's main door... */

                pexit->u1.to_room = get_room_index(pexit->u1.vnum);

    	        if ( pexit->u1.to_room != NULL) { 
	          fexit = TRUE; 
		} 
       
               /* Hmmm. If that failed the union (and thus the vnum?)
                  is set to NULL (0?)... Yulck. Double Yulch. */ 
 
               /* Now fix it's conditional exits... */

                fBootDb = FALSE;
                
                cdd = pexit->cond_dest; 

                while (cdd != NULL) {

                  cdd->dest = get_room_index(cdd->vnum);

                  if (cdd->dest == NULL) {
                    bug("Cond dest to undefined room %d!", cdd->vnum);
                  }

                  if (cdd->cond == NULL) {
                    bug("Cond dest without cond (room %d)!", pRoomIndex->vnum);
                  }

                  cdd = cdd->next;
                }

                fBootDb = TRUE;
                
	    }

           /* Mobs shouldn't enter rooms without exits... */

	    if (!fexit) {
		SET_BIT(pRoomIndex->room_flags,ROOM_NO_MOB);
            }

           /* Validate Recall, Respawn, Morgue and Dream settings... */

            if (pRoomIndex->recall != 0) {
              if (!room_ok_for_recall(pRoomIndex->recall)) {
                bug("Bad recall room on room %d removed", pRoomIndex->vnum);
                pRoomIndex->recall = 0;
              }
            } 

            if (pRoomIndex->respawn != 0) {
              if (!room_ok_for_recall(pRoomIndex->respawn)) {
                bug("Bad respawn room on room %d removed", pRoomIndex->vnum);
                pRoomIndex->respawn = 0;
              }
            } 

            if (pRoomIndex->morgue != 0) {
              if (!room_ok_for_recall(pRoomIndex->morgue)) {
                bug("Bad morgue room on room %d removed", pRoomIndex->vnum);
                pRoomIndex->morgue = 0;
              }
            } 

            if (pRoomIndex->dream != 0) {
              if (!room_ok_for_recall(pRoomIndex->dream)) {
                bug("Bad dream room on room %d removed", pRoomIndex->vnum);
                pRoomIndex->dream = 0;
              }
            }

            if (pRoomIndex->mare > 0) {
              if (!room_ok_for_recall(pRoomIndex->mare)) {
                bug("Bad mare room on room %d removed", pRoomIndex->vnum);
                pRoomIndex->mare = 0;
              }
            }

            if (pRoomIndex->night != 0) {
              if (!room_ok_for_recall(pRoomIndex->night)) {
                bug("Bad night room on room %d removed", pRoomIndex->vnum);
                pRoomIndex->night = 0;
              }
            }

            if (pRoomIndex->day != 0) {
              if (!room_ok_for_recall(pRoomIndex->day)) {
                bug("Bad day room on room %d removed", pRoomIndex->vnum);
                pRoomIndex->day = 0;
              }
            }

           /* Validate resets */ 
 
            validate_resets(pRoomIndex);

	}
    }

    return;
}


/* Check area recall, respawn, morgue and dream settings... */

void check_area_recalls() {
  AREA_DATA *area;
  area = area_first;
  while (area != NULL) {

    if (area->recall != 0) {
      if (!room_ok_for_recall(area->recall)) {
        bug("Bad recall room on area %d fixed.", area->vnum); 
        area->recall = 0;
      }
    } 

    if (area->respawn != 0) {
      if (!room_ok_for_recall(area->respawn)) {
        bug("Bad respawn room on area %d fixed.", area->vnum); 
        area->respawn = 0;
      }
    } 

    if (area->morgue != 0) {
      if (!room_ok_for_recall(area->morgue)) {
        bug("Bad morgue room on area %d fixed.", area->vnum); 
        area->morgue = 0;
      }
    } 

    if (area->dream != 0) {
      if (!room_ok_for_recall(area->dream)) {
        bug("Bad dream room on area %d fixed.", area->vnum); 
        area->dream = 0;
      }
    } 

    if (area->mare > 0) {
      if (!room_ok_for_recall(area->mare)) {
        bug("Bad mare room on area %d fixed.", area->vnum); 
        area->mare = 0;
      }
    } 

    area = area->next;
  }

  return;
}


/*
 * Repopulate areas periodically.
 */
void area_update( void ) {

  AREA_DATA *pArea;

 /* Reset those areas that need it... */
 
  for ( pArea = area_first; pArea != NULL; pArea = pArea->next ) {

   /* Increment age since last rest... */ 

    pArea->age += 1;

    room_update ( pArea );

   /* If less than 3, no reset possible... */

    if ( pArea->age < 3 ) {
      continue;
    }

   /* Check age and reset... */

    if ( (!pArea->empty 
        && ( pArea->nplayer == 0 
          || pArea->age >= 15))
      || pArea->age >= 31) {

     /* Do the reset... */

      reset_area(pArea, FALSE );

     /* Set the new starting age... */

      pArea->age = number_range( 0, 3 );

     /* Reset the area empty flag... */

      if (pArea->nplayer == 0) { 
        pArea->empty = TRUE;
      }
    }
  }

 /* Send out area update pulses... */

  issue_time_wev(SUB_PULSE_AREA, WEV_PULSE, WEV_PULSE_AREA, 0);

 /* All done... */ 

  return;
}

/* M mob_vnum mob_count */

int reset_room_m(ROOM_INDEX_DATA *room, RESET_DATA *reset) {
    int mob_vnum;
 
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *pMob;
    ROOM_INDEX_DATA *pRoomIndexPrev; 
    int nfound;
 
   /* Find the template for the mob... */

    mob_vnum = 0;

    pMobIndex = get_mob_index( reset->arg1 );

    if ( pMobIndex == NULL ) {
      bug( "Reset_room: 'M': bad vnum %d.", reset->arg1 );
      return 0;
    }

    if (IS_SET(pMobIndex->act, ACT_NIGHT)
    && IS_SET(time_info.flags, TIME_DAY)) return pMobIndex->vnum;

    if (IS_SET(pMobIndex->act, ACT_MARTIAL)
    && pMobIndex->area->martial == 0) return pMobIndex->vnum;

    mob_vnum = reset->arg1;

   /* No reset if we already have enough within the world... */

    if ( reset->count >= reset->arg2 ) return mob_vnum;
    
   /* Recreate the missing mobs in this room... */

    nfound = reset->count;

    while (nfound < reset->arg2) {

     /* We're trying this one... */

      nfound += 1;

     /* Load and taylor an instance of the mob... */

      pMob = create_mobile( pMobIndex );
      pMobIndex->count++;

      pMob->reset = reset;

      pMob->recall_perm = room->vnum;

      reset->count++;

     /* Pet shop mobiles get ACT_PET set... */
           
      pRoomIndexPrev = get_room_index( room->vnum - 1 );

      if ( pRoomIndexPrev != NULL
      && IS_SET( pRoomIndexPrev->room_flags, ROOM_PET_SHOP))  SET_BIT( pMob->act, ACT_PET);
      
    /* No more charming of Proggies */
       if ((pMob->triggers->reaction != NULL 
       || pMob->triggers->challange != NULL
       || pMob->pIndexData->pShop != NULL)
       && !IS_SET(pMob->act, ACT_PET))  {
             SET_BIT( pMob->imm_flags, IMM_CHARM);
             SET_BIT( pMob->imm_flags, IMM_SUMMON);
       }

       if (pMob->pIndexData->pShop != NULL) SET_BIT(pMob->form, FORM_INSTANT_DECAY);

     /* Bring the mob here... */

      char_to_room( pMob, room );

      if (pMob->pIndexData->pShop != NULL
      && startup == TRUE) load_shop_obj(pMob, room);

    }
 
    return mob_vnum;
}

/* O obj_vnum obj_count */

int reset_room_o(ROOM_INDEX_DATA *room, RESET_DATA *reset) {
    int obj_vnum;
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *pObj;

    int nfound;

   /* Find the template for the object being reset... */

    pObjIndex = get_obj_index( reset->arg1 );

    if ( pObjIndex == NULL ) {
      bug( "Reset_room: 'O': bad vnum %d.", reset->arg1 );
      return 0;
    }

    if (IS_SET(pObjIndex->extra_flags, ITEM_ARTIFACT)
    && !startup) {
      return 0;
    }

    obj_vnum = reset->arg1;

   /* Make sure we have enough in the room... */

    nfound = 0;

    pObj = room->contents;

    while (pObj != NULL) {

      if (pObj->reset == reset) {
        nfound += 1;
      }   

      pObj = pObj->next_content;
    }

   /* Create the missing objects... */

    while (nfound < reset->arg2) {

     /* We're trying this one... */

      nfound += 1;

     /* Only a chance of repopping... (Zeran) */

      if (number_percent() <= pObjIndex->repop) {

       /* Create the object... */

        if (!load_artifact(reset)) {
            pObj = create_object( pObjIndex, pObjIndex->level); 

            pObj->reset = reset;
            reset->count++;

            if (pObj->item_type == ITEM_HERB) pObj->timer=48;
            obj_to_room( pObj, room );
        }
      }
    }

    return obj_vnum;
}  

/* P obj_vnum obj_count */

void reset_room_p(ROOM_INDEX_DATA *pRoom, RESET_DATA *reset) {
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *pCont, *pObj;

    CHAR_DATA *pMob;

    int nfound; 

   /* Must have vnum of the container... */

    if (reset->arg3 == 0) {
      bug("P reset with null containter vnum for room %d", pRoom->vnum);
      return;
    }

   /* Find the template for the object being reset... */

    pObjIndex = get_obj_index( reset->arg1 );

    if ( pObjIndex == NULL ) {
      bug( "Reset_room: 'P': bad object vnum %d.", reset->arg1 );
      return;
    }

    if (IS_SET(pObjIndex->extra_flags, ITEM_ARTIFACT)
    && !startup) {
      return;
    }

   /* Find each obj of the right type within the room... */

    pCont = pRoom->contents;

    while (pCont != NULL) {

     /* Is it an instance of the container? */

      if ( pCont->pIndexData != NULL
        && pCont->pIndexData->vnum == reset->arg3 ) {

       /* Is it a container? */ 
      
        if ( !( pCont->pIndexData->item_type == ITEM_CONTAINER
             || pCont->pIndexData->item_type == ITEM_KEY_RING
             || pCont->pIndexData->item_type == ITEM_TREE
             || pCont->pIndexData->item_type == ITEM_CORPSE_NPC
             || pCont->pIndexData->item_type == ITEM_CAMERA)) {
          bug("P reset for non container in room %d",pRoom->vnum);
          return;
        } else {    

         /* Yes. Now how many objects does it have inside it? */

          nfound = 0;

          pObj = pCont->contains; 

          while ( pObj != NULL ) {
  
            if ( pObj->pIndexData == pObjIndex ) {
              nfound += 1;
            }

            pObj = pObj->next_content;
          }

         /* Create the missing objects... */ 

          while (nfound < reset->arg2) {

           /* We're trying this one... */
 
            nfound += 1;

            if (number_percent() <= pObjIndex->repop) {

             /* Create the object... */
  
                if (!load_artifact(reset)) {
                    pObj = create_object( pObjIndex, pObjIndex->level);
 
                    pObj->reset = reset;
                    reset->count++;

                    obj_to_obj( pObj, pCont );
                }
            }
          }
        }  
      }

      pCont = pCont->next_content;
    }
 
   /* Now check each mob in the room... */

    pMob = pRoom->people;

    while (pMob != NULL) {

     /* Find each obj of the right type within the room... */

      pCont = pRoom->contents;

      while (pCont != NULL) {

       /* Is it an instance of the container? */

        if ( pCont->pIndexData != NULL
          && pCont->pIndexData->vnum == reset->arg3 ) {

         /* Is it a container? */ 
      
          if ( !( pCont->pIndexData->item_type == ITEM_CONTAINER
               || pCont->pIndexData->item_type == ITEM_KEY_RING
               || pCont->pIndexData->item_type == ITEM_TREE
               || pCont->pIndexData->item_type == ITEM_CORPSE_NPC
               || pCont->pIndexData->item_type == ITEM_CAMERA)) {
            bug("P reset for non container object in room %d",pRoom->vnum);
            return;
          } else {    

           /* Yes. Now how many objects does it have inside it? */
 
            nfound = 0;

            pObj = pCont->contains; 

            while ( pObj != NULL ) {
    
              if ( pObj->pIndexData == pObjIndex ) {
                nfound += 1;
              }

              pObj = pObj->next_content;
            }

           /* Create the missing objects... */ 
  
            while (nfound < reset->arg2) {

             /* We're trying this one... */
 
              nfound += 1;

             /* Only a change of repopping... (Zeran) */

              if (number_percent() <= pObjIndex->repop) {

               /* Create the object... */
  
                  if (!load_artifact(reset)) {
                        pObj = create_object( pObjIndex, pObjIndex->level);
 
                        pObj->reset = reset;
                        reset->count++;

                        obj_to_obj( pObj, pCont );
                  }
              }
            }
          }  
        }

        pCont = pCont->next_content;
      }

      pMob = pMob->next_in_room;
    }

    return;
}

/* E obj_vnum obj_count location */
/* G obj_vnum obj_count */


void reset_room_eg(ROOM_INDEX_DATA *pRoom, RESET_DATA *reset, int mob_vnum) {
OBJ_INDEX_DATA *pObjIndex;
OBJ_DATA *pObj;
CHAR_DATA *pMob;
bool skeeper;
int nfound; 
int target;

   /* Must have vnum of the mob... */

    if (mob_vnum == 0) {
      bug("G or E reset before M reset for room %d.", pRoom->vnum);
      return;
    }

   /* Find the template for the object being reset... */

    pObjIndex = get_obj_index( reset->arg1 );

    if ( pObjIndex == NULL ) {
      bug( "Reset_room: 'EG': bad vnum %d.", reset->arg1 );
      return;
    }

    if (IS_SET(pObjIndex->extra_flags, ITEM_ARTIFACT)
    && !startup) {
      return;
    }

   /* Find each obj of the right type within the room... */

    pMob = pRoom->people;

    while (pMob != NULL) {

     /* Is it an instance of the mob? */

      if ( pMob->pIndexData != NULL
        && pMob->pIndexData->vnum == mob_vnum ) {

       /* How many are we looking for? */

        target = reset->arg2; 

       /* Shop keepers get stock through type G resets... */

        if ( pMob->pIndexData != NULL
          && pMob->pIndexData->pShop != NULL
          && reset->command == 'G' ) {
          skeeper = TRUE;
          target = 1; 
        } else {
          skeeper = FALSE;
        } 

       /* Type E resets MUST have a target of 1... */

        if (reset->command == 'E') {
          target = 1;
        }

       /* Yes... Now how many objects does it have? */

        nfound = 0;

        pObj = pMob->carrying; 

        while ( pObj != NULL ) {

          if ( pObj->pIndexData == pObjIndex ) {

           /* G items simply have to be held (or worn) */

            if ( reset->command == 'G' ) {
              nfound += 1;
            }

           /* E items should be in the right place... */ 

            if ( reset->command == 'E' ) { 

              if ( pObj->wear_loc == reset->arg3 ) {
                nfound += 1;  
              }
            }
          }

          pObj = pObj->next_content;
        }

       /* Check for already being equipped... */

        if ( reset->command == 'E') {

          pObj = get_eq_char(pMob, reset->arg3);

	  if ( pObj != NULL ) {
	    nfound = target;
          }
        }

       /* Create the missing objects... */ 

        while (nfound < target) {

         /* We're trying this one... */

          nfound += 1;

         /* Only a chance of repopping... (Zeran) */

          if (number_percent() <= pObjIndex->repop) {

           /* Create the object... */

              if (!load_artifact(reset)) {
                   pObj = create_object( pObjIndex, pMob->level);
 
                   pObj->reset = reset;
                   reset->count++;

                   if ( skeeper ) SET_BIT( pObj->extra_flags, ITEM_INVENTORY );
                  obj_to_char( pObj, pMob );

                  if ( reset->command == 'E' ) {
                       pObj->size = pMob->size;
                       equip_char( pMob, pObj, reset->arg3 );
                  }
              }
          }
        }  
      }

      pMob = pMob->next_in_room;
    }
 
    return;
}

/* R room_vnum dir_count */

void reset_room_r(ROOM_INDEX_DATA *pRoom, RESET_DATA *reset) {
    EXIT_DATA *pexit;

   /* Must have a room... */

    if (pRoom == NULL) {
        return;
    }

   /* Just check the meddling count... */

    if ( reset->arg2 <= 1 
      || reset->arg2 > DIR_MAX ) {
      bug("Bad exit count on R reset in room %d.", pRoom->vnum);
      return;
    } 

   /* Do some muddling... */

    int d0;
    int d1;

    for ( d0 = 0; d0 < reset->arg2 - 1; d0++ ) {
      d1                   = number_range( d0, reset->arg2-1 );
      pexit                = pRoom->exit[d0];
      pRoom->exit[d0]      = pRoom->exit[d1];
      pRoom->exit[d1]      = pexit;
    }

    return;
}

/*
 * Reset one room.  Called by reset_area and olc.
 */
void reset_room( ROOM_INDEX_DATA *pRoom, bool invasion, bool forcereset) {
RESET_DATA  *pReset;
CHAR_DATA   *pMob;
int iExit;
VNUM low, high, rvnum;
int zone;
ROOM_INDEX_DATA *randroom;
ROOM_INDEX_DATA *otherroom;
AREA_DATA *inarea;
int insubarea;
CHAR_DATA *pMob_next;
GADGET_DATA *gadget;
OBJ_DATA *obj;
OBJ_DATA *obj_next;
OBJ_DATA *portal;
VNUM last_mob_vnum;
VNUM last_obj_vnum;
VNUM portalvnum;
 
   /* Must have a room... */
    if (pRoom == NULL ) return;

    if (!IS_SET(pRoom->room_flags, ROOM_ALWAYS_RESET)
    && !forcereset) {
        for (pMob = pRoom->people; pMob; pMob = pMob->next_in_room) {
            if (!IS_NPC(pMob)) return;
        }    
    }

   /* Close and lock the doors... */

    for ( iExit = 0;  iExit < DIR_MAX;  iExit++ )    {
        EXIT_DATA *pExit;
        if ( ( pExit = pRoom->exit[iExit] ))        {
            pExit->exit_info = pExit->rs_flags;
            pExit->condition = 100;
            if ( ( pExit->u1.to_room != NULL )
            && ( ( pExit = pExit->u1.to_room->exit[rev_dir[iExit]] ) ) )            {
                  pExit->exit_info = pExit->rs_flags;
            }
        }
    }

     for ( obj = pRoom->contents; obj != NULL; obj = obj_next )	    {
             obj_next = obj->next_content;
              if (obj->item_type == ITEM_CONTAINER) {
                    obj->value[1] = obj->valueorig[1];
              }
     }

   /* Reset all of the gadgets... */

    gadget = pRoom->gadgets;

    while (gadget != NULL) {

      if (gadget->state != 0) {
        gadget->state = 0;
      }

      gadget = gadget->next;
    }

   /* NULL last mob loaded and last object loaded... */

    last_mob_vnum = 0;
    last_obj_vnum = 0;

   /* Age all of the tracks... */

    if (pRoom->tracks != NULL) age_tracks(pRoom);
    
    if (startup) {
        if (pRoom->vnum == pRoom->area->respawn
        || pRoom->vnum == zones[pRoom->area->zone].respawn
        || pRoom->vnum == mud.respawn) {
              SET_BIT(pRoom->room_flags, ROOM_SAFE);
        }

        if (pRoom->vnum == pRoom->area->morgue
        || pRoom->vnum == zones[pRoom->area->zone].morgue
        || pRoom->vnum == mud.morgue
        || IS_SET(pRoom->room_flags, ROOM_PRIVATE)) {
              load_morgue(pRoom);
        }

        if (IS_SET(pRoom->room_flags, ROOM_TREE)) {
              load_tree(pRoom);
        }
    }

   /* Drive all of the resets... */

    pReset = pRoom->reset_first;

    while (pReset != NULL) {

        switch ( pReset->command ) {
        
          default:
            bug( "Reset_room: bad command %c.", pReset->command );
            bug( "For room %d", pRoom->vnum);

            break;

        /* M mob_vnum mob_count */

          case 'M':

            last_mob_vnum = reset_room_m(pRoom, pReset);
 
            break;

         /* O obj_vnum obj_count */

          case 'O':

            last_obj_vnum = reset_room_o(pRoom, pReset);

            break;

        /* P object count to_object */

          case 'P':

            reset_room_p(pRoom, pReset);

            break;

         /* G object */
         /* E object location */   

          case 'G':
          case 'E':

            reset_room_eg(pRoom, pReset, last_mob_vnum);
	
            break;

         /* R room max_direction */

          case 'R':

            reset_room_r(pRoom, pReset);		

            break;

         /* D room direction lock_flags */

          case 'D':
            break;

        }

        pReset = pReset->next;
    }

    
   /* Create vehicle on startup */

    if (IS_SET(pRoom->room_flags, ROOM_VEHICLE)
    && (pRoom->vnum <  mud.pattern[0]
    || pRoom->vnum > mud.pattern[1])) {
        otherroom = pRoom->exit[DIR_OUT]->u1.to_room;
        portalvnum = pRoom->vnum;
        portal = otherroom->contents;

        while ( portal != NULL
        && (portal->item_type != ITEM_PORTAL
             || portal->pIndexData->vnum != portalvnum)) {
               portal = portal->next_content;
        }
    
         if (portal == NULL ) {
              portal = create_object( get_obj_index(pRoom->vnum), 1 );
              REMOVE_BIT(portal->wear_flags, ITEM_TAKE);
              obj_to_room(portal, otherroom);             
         }
    }


   /* Now we drive the telepops... */

    pMob = pRoom->people;
    while (pMob != NULL) {
      pMob_next = pMob->next_in_room;

     /* Check telepop is possible... */
  
      if ( IS_SET( pMob->act, ACT_TELEPOP)
        && pMob->in_room == NULL ) {
        log_string(pMob->short_descr);
        bug("Telepop with no room for mob!", 0);
        REMOVE_BIT(pMob->act, ACT_TELEPOP); 
      } 

     /* See if it lives somewhere else... */
 
      if ( IS_SET( pMob->act, ACT_TELEPOP)) {
            
       /* Select destination rooms... */

        inarea = NULL;
        insubarea = 0;
	low = 0;
	high = 65535;
        
        zone = 0;
  
        if ( pMob->in_room->area != NULL ) {
          zone = pMob->in_room->area->zone;
        }

      if (!IS_SET( pMob->act, ACT_INVADER)
      || invasion == TRUE) {

         REMOVE_BIT( pMob->act, ACT_TELEPOP);       
      
       /* Restrict to area if needed... */

        if ( ( IS_SET( pMob->act, ACT_STAY_AREA) )
          || ( IS_SET( pMob->act, ACT_STAY_SUBAREA) )
	  || ( IS_SET( pMob->act, ACT_SENTINEL) ) ) {

	  inarea = pMob->in_room->area;
                  insubarea = pMob->in_room->subarea;
	  low = inarea->lvnum;
	  high = inarea->uvnum;
	} 

       /* Find an acceptable room... */

 	randroom = NULL;

	while (randroom == NULL) {
 	  rvnum = number_range( low, high );
 	  randroom = get_room_index( rvnum );

          if ( randroom != NULL
            && randroom->area != NULL
            && randroom->area->zone != zone ) {
            randroom = NULL;
          } 

          if ( randroom != NULL
            && inarea != NULL
            && randroom->subarea != insubarea) {
            randroom = NULL;
          }

          if ( randroom != NULL ) {
 
	    if (IS_SET(randroom->room_flags, ROOM_NO_MOB )
	      || IS_SET(randroom->room_flags, ROOM_SAFE )
	      || IS_SET(randroom->room_flags, ROOM_LAW)
   	      || IS_SET(randroom->room_flags, ROOM_GODS_ONLY )
 	      || IS_SET(randroom->room_flags, ROOM_SOLITARY) ) {
	      randroom = NULL;
            }
          }

	  if ( randroom != NULL 
	    && !can_see_room(pMob,randroom) ) {
            randroom = NULL;
          }
    	}

       /* Move the mob... */
 
        if (randroom != NULL) { 
          char_from_room( pMob );
	  char_to_room( pMob, randroom );
        } else {
          bug("Bad telepop in room %d", pMob->in_room->vnum); 
        }

      }
      }
 
     /* Next mob in the original room... */

      pMob = pMob_next; 
    }

   /* Now we drive the telepop objs */

     for ( obj = pRoom->contents; obj != NULL; obj = obj_next )	    {
             obj_next = obj->next_content;
 
             if (obj->item_type == ITEM_HERB
             && obj->value[4] == TRUE) {
                      obj->value[4] = FALSE;
                      low = 0;
	      high = 65535;
  
     	      randroom = NULL;
  	      while (randroom == NULL) {
		rvnum = number_range( low, high );
 	                randroom = get_room_index( rvnum );

                                 if ( randroom != NULL ) {
 	                     if (IS_SET(randroom->room_flags, ROOM_GODS_ONLY )
 	                     || IS_SET(randroom->room_flags, ROOM_SOLITARY) ) {
	                          randroom = NULL;
                                     }
                                }
                               
	                if (randroom != NULL) {
                                     if (randroom->sector_type != SECT_MOUNTAIN
                                     && randroom->sector_type != SECT_JUNGLE
                                     && randroom->sector_type != SECT_FIELD
                                     && randroom->sector_type != SECT_FOREST
                                     && randroom->sector_type != SECT_HILLS
                                     && randroom->sector_type != SECT_MOORS
                                     && randroom->sector_type != SECT_SWAMP) {
                                            randroom = NULL;
                                     }
                                }
                      }

                      if (randroom != NULL) { 
                            obj_from_room( obj );
	            obj_to_room( obj, randroom );
                      } else {
                            bug("Bad OBJ telepop in room %d", pRoom->vnum); 
                      }
              }
              obj = obj_next; 
     }

    return;
}


/*
 * Reset one area.
 */

void reset_area( AREA_DATA *pArea, bool forcereset) {
ROOM_INDEX_DATA *pRoom;
VNUM vnum;
bool invasion;

    if (number_percent() < 3
    || pArea->invasion == TRUE) {
         invasion=TRUE;
         pArea->invasion = FALSE;
    } else {
         invasion = FALSE;
    }

    for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )    {
        if ( ( pRoom = get_room_index(vnum) ) )	{
            reset_room(pRoom, invasion, forcereset);
        }
    }

     notify_message (NULL, NOTIFY_REPOP, TO_ALL, pArea->name);
     notify_message (NULL, WIZNET_RESET, TO_IMM, pArea->name);
     return;
}

/* experimental random object code - elfren */	
		
#define nelems(a) (sizeof (a)/sizeof (a)[0])

// Calculate a meaningful modifier and amount
void random_apply( OBJ_DATA *obj, CHAR_DATA *mob ) {
static int attrib_types[] = { APPLY_STR, APPLY_DEX, APPLY_DEX, APPLY_INT, APPLY_INT, APPLY_WIS, APPLY_CON, APPLY_CON, APPLY_CON };
static int power_types[] = { APPLY_MANA, APPLY_HIT, APPLY_MOVE, APPLY_AC };
static int combat_types[] = { APPLY_HITROLL, APPLY_HITROLL, APPLY_DAMROLL, APPLY_SAVING_SPELL, APPLY_SAVING_SPELL, APPLY_SAVING_BREATH };
AFFECT_DATA *paf = new_affect();

   paf->type	= -1;
   paf->duration  	= -1;
   paf->bitvector 	= 0;
   paf->next	= obj->affected;
   obj->affected  	= paf;

   switch (number_bits(2)) {
   case 0:
      paf->location  = attrib_types[number_range(0, nelems(attrib_types)-1)];
      paf->modifier  = 1;
      break;
   case 1:
      paf->location  = power_types[number_range(0, nelems(power_types)-1)];
      paf->modifier  = number_range(mob->level/2, mob->level);
      break;
   case 2:
   case 3:
      paf->location  = combat_types[number_range(0, nelems(combat_types)-1)];
      paf->modifier  = number_range(1, mob->level/6+1);
      break;
   }

   SET_BIT(obj->extra_flags, ITEM_MAGIC);

   // Is item cursed?
   if (number_percent() <= 5) {
      paf->modifier = -paf->modifier;
      SET_BIT(obj->extra_flags, ITEM_NODROP);
      if (number_percent() <= 15)  SET_BIT(obj->extra_flags, ITEM_NOREMOVE);
   }
   return;
}

// Jewelry stuff
static char *adj1[] = { "splendid", "ancient", "dusty", "scratched", "flawed", "burnt", "heavy", "gilded", "spooky", "flaming", "plain", "ornate", "inscrutable", "obscene", "wrinkled" };
static char *adj2[] = { "diamond", "emerald", "topaz", "wooden", "jade", "white gold", "onyx", "tin", "glass", "marble", "black", "granite" };

#define MASK_IGNORE     (1<<TAR_IGNORE)
#define MASK_OFFENSIVE  (1<<TAR_CHAR_OFFENSIVE)
#define MASK_DEFENSIVE  (1<<TAR_CHAR_DEFENSIVE)
#define MASK_SELF       (1<<TAR_CHAR_SELF)
#define MASK_INV        (1<<TAR_OBJ_INV)


// Returns a clerical or magical spell of the appropriate (masked) type
/* Zeran - fixed, no longer generates skills by mistake */

int random_spell( int level, int mask, short *type ) {
int spn;

   for ( ;; )  {
        spn = number_range(1, MAX_SPELL-1);

        if (!valid_spell(spn)) continue;
      
        if (mask & (1<<spell_array[spn].target)) {
           *type = spell_array[spn].target;
           return spn;
        }
   }
   return 0;
}


/* A or An ? */

const char *a_or_an(char initial) {
static const char a[]  = { 'a', '\0' };
static const char an[] = { 'a', 'n', '\0'};

   if ( initial == 'a'
     || initial == 'e'
     || initial == 'i'
     || initial == 'o'
     || initial == 'u') {
     return an;
   }
 
   return a;
 }


/* Make a random potion... */

/* NOTE: S1, s2 and s3 are effect array indexes, not slot numbers. */

void create_potion( CHAR_DATA *ch, int s1, int s2, int s3) {
OBJ_INDEX_DATA *potIndex;
OBJ_DATA *potion;
int pot_level, pot_con, pot_col;
int spell1 = 0, spell2 = 0, spell3 = 0;
char buf[MAX_STRING_LENGTH];

 /* Find the prototype... */ 

  potIndex = get_obj_index(OBJ_POTION);

  if (potIndex == NULL) {
    send_to_char("You feel that something is missing...\r\n", ch);
    return;
  }

 /* Set its level... */

  pot_level = UMIN(number_range(1, ch->level + 8), MAX_LEVEL - 1);

 /* Now create the potion... */

  potion = create_object( potIndex, pot_level );

 /* Initialize the cost... */

  potion->cost = pot_level * 3;

 /* Decide spells... */

  spell1 = number_range(1, NUM_PS_RANK1);
  
  SET_BIT(potion->extra_flags, ITEM_MAGIC); 

  if (number_bits(2) == 0 || s2 != 0) {
   
    spell2 = number_range(1, NUM_PS_RANK2);
   
    potion->cost *= 2;
 
    SET_BIT(potion->extra_flags, ITEM_GLOW);

    if (number_bits(2) == 0 || s3 != 0) {

      spell3 = number_range(1, NUM_PS_RANK3);
 
      SET_BIT(potion->extra_flags, ITEM_HUM);

      potion->cost *= 2; 
    }
  }

 /* Null out pre-defined spells... */

  if (s1 != 0) spell1 = 0;
  if (s2 != 0) spell2 = 0;
  if (s3 != 0) spell3 = 0;

 /* Set up the values... */

  potion->level = pot_level;

  potion->value[0] = pot_level; 

  potion->value[1] = (spell1 == 0 ? s1 : pot_spells[spell1 - 1]);
  potion->value[2] = (spell2 == 0 ? s2 : pot_spells[spell2 - 1]);
  potion->value[3] = (spell3 == 0 ? s3 : pot_spells[spell3 - 1]);

 /* Now a description... */

  pot_con = number_range(1, NUM_DESC_CON) - 1;
  pot_col = number_range(1, NUM_DESC_COL) - 1;
 
  sprintf(buf, "%s %s %s potion", a_or_an(desc_con[pot_con][0]), desc_con[pot_con], desc_col[pot_col]); 
 
 /* Set for short and long... */

  free_string(potion->short_descr);
  potion->short_descr = str_dup(buf);

  buf[0] = toupper(buf[0]);

  free_string(potion->description);
  potion->description = str_dup(buf);
  
 /* Fix the name... */

  sprintf(buf, "potion %s %s", desc_col[pot_col], desc_con[pot_con]);

  free_string(potion->name);
  potion->name = str_dup(buf);

 /* Give it to the mob... */

  potion->condition = 1;
  obj_to_char( potion, ch );

  return;
} 


// Anything wearable, and trinkets
void wield_random_armor( CHAR_DATA *mob ) {
int item_type = number_range(32, 46);
OBJ_INDEX_DATA *pObjIndex = get_obj_index(item_type);
OBJ_DATA *obj = create_object(pObjIndex, number_fuzzy(mob->level));
int    n_adj1 = number_range(0, nelems(adj1)-1);
int    n_adj2 = number_range(0, nelems(adj2)-1);
char *name= ""; 
int number;
int type;

   // Armor stuff
   static char *armor_types[] = { "leather", "studded leather", "bronze", "chain", "plate", "mithril" };
   static int armor_mul[] = { 1, 3, 2, 5, 10, 10 };
   static int armor_div[] = { 1, 2, 1, 1,  1,  3 };

   // Weapon stuff
   static char *weapon_types[] = { "sword", "broadword", "rapier", "longsword", "sword", "short sword", "dagger", "knife", "hammer", "mace", "mace", "whip",   "spear", "pike", "flail" };
   static int weapon_dam[] = { 3, 3, 3, 3, 3, 11, 11, 11, 0, 7, 7, 4, 11, 11, 27 };
   static int weapon_class[] = { 1, 1, 1, 1, 1, 1, 2, 2, 4, 4, 4, 7, 3, 8, 6 };

   // Trinket stuff
   static char *noun[] = { "pebble", "bauble", "stone", "charm", "fetish", "bone", "trinket" };
   char buffer[64];

   char buf[MAX_STRING_LENGTH];

   if (obj->item_type == ITEM_ARMOR)  {
      int ac_type = URANGE(0, (unsigned) mob->level/5, nelems(armor_types)-1);
      name = armor_types[ac_type];
      obj->weight *= armor_mul[ac_type];
      obj->weight /= armor_div[ac_type];
      obj->value[0] = mob->level / 5 +3;
      obj->value[1] = obj->value[0];
      obj->value[2] = obj->value[0];
      obj->value[3] = obj->value[0] *2 /3;
      if (number_percent() < mob->level / 3) random_apply(obj, mob);

   } else if (obj->item_type == ITEM_WEAPON) {
      int wea_type = number_range(0, nelems(weapon_types)-1);
      name = weapon_types[wea_type];
      obj->value[3] = weapon_dam[wea_type];
      number = UMIN(mob->level/4 + 1, 5);
      type   = (mob->level + 7)/number;
      obj->value[0] = weapon_class[wea_type];
      obj->value[1] = number;
      obj->value[2] = type;

   } else if (obj->item_type == ITEM_JEWELRY)      {
      if (number_percent() < mob->level) {
          random_apply(obj, mob);
          if (number_percent() < mob->level / 3) random_apply(obj, mob);
      }

      if (obj->wear_flags & ITEM_HOLD) sprintf(buffer, "%s %s %s", adj1[n_adj1], adj2[n_adj2], noun[number_range(0, nelems(noun)-1)]);
      else sprintf(buffer, "%s %s", adj1[n_adj1], adj2[n_adj2]);
      name = buffer;
   }

   sprintf( buf, "a%s %s %s", (buffer[0] == 'a' || buffer[0] == 'e' || buffer[0] == 'i' || buffer[0] == 'o' || buffer[0] == 'u') ? "n" : "", name,obj->short_descr );
   free_string( obj->short_descr );
   obj->short_descr = str_dup( buf );

   if ( !str_cmp( obj->name, "weapon template" )) {
       sprintf( buf, "a%s %s", (buffer[0] == 'a' || buffer[0] == 'e' || buffer[0] == 'i' || buffer[0] == 'o' || buffer[0] == 'u') ? "n" : "",name );
       free_string( obj->short_descr );
       obj->short_descr = str_dup( buf );   
       free_string(obj->name);
       obj->name = str_dup( name );
   }

   free_string(obj->description );
   sprintf(buf, "%s lies here.", obj->short_descr);
   obj->description = str_dup(buf);
   obj->description[0] = toupper(obj->description[0]);

   obj->level = mob->level;
   update_obj_orig(obj);

   obj->cost = mob->level * 30;
   obj_to_char(obj, mob);

   equip_char( mob, obj, item_type );
}



/* Create an instance of a mobile at default level... */

CHAR_DATA *create_mobile( MOB_INDEX_DATA *pMobIndex ) {
CHAR_DATA *mob;

 /* Check we have a valid template... */

  if ( pMobIndex == NULL ) {
    bug( "Create_mobile: NULL pMobIndex.", 0 );
    exit( 1 );
  }

 /* Create at default level... */

  mob = create_mobile_level(pMobIndex, NULL, pMobIndex->level);

  return mob;
}


/* Create a mob a a given level... */
CHAR_DATA *create_mobile_level( MOB_INDEX_DATA *pMobIndex, char *profession, int clev ) {
CHAR_DATA *mob;
PROF_DATA *prof = NULL;
char buf[MAX_STRING_LENGTH];
int i, currency;
float mod, mod2;

    mobile_count++;

   /* Must have a valid template... */

    if ( pMobIndex == NULL ) {
	bug( "Create_mobile_level: NULL pMobIndex.", 0 );
	exit( 1 );
    }

   /* Get some memory... */

    if ( char_free == NULL ) {
	mob		= (CHAR_DATA *) alloc_perm( sizeof(*mob) );
    } else {
	mob		= char_free;
	char_free	= char_free->next;
    }

   /* Wipe it... */

    clear_char( mob );

    mob->pIndexData	= pMobIndex;
    mob->reset		= NULL;  /* Zeran */

   /* Work out the mobs true_name... */

    sprintf(buf, "@m%p", mob);

    mob->true_name = str_dup(buf);

   /* Set up name and description... */ 

    mob->name           = str_dup( pMobIndex->player_name );   
    mob->short_descr    = str_dup( pMobIndex->short_descr );
    mob->long_descr     = str_dup( pMobIndex->long_descr );  
    mob->description    = str_dup( pMobIndex->description );   
    mob->bio            = str_dup( pMobIndex->bio );           
    mob->spec_fun	= pMobIndex->spec_fun;
    
   /* Set basic parameters from prototype... */

    if (pMobIndex->new_format) {
	
	mob->act 		= pMobIndex->act;
	mob->comm		= 0;
	mob->affected_by	= pMobIndex->affected_by;
	mob->alignment		= pMobIndex->alignment;
	mob->group		= pMobIndex->group;
	mob->hitroll		= pMobIndex->hitroll;
	mob->atk_type		= pMobIndex->atk_type;
	mob->dam_type		= pMobIndex->dam_type;
	mob->off_flags		= pMobIndex->off_flags;
	mob->imm_flags		= pMobIndex->imm_flags;
	mob->envimm_flags	= pMobIndex->envimm_flags;
	mob->res_flags		= pMobIndex->res_flags;
	mob->vuln_flags		= pMobIndex->vuln_flags;
	mob->start_pos		= pMobIndex->start_pos;
	mob->default_pos	= pMobIndex->default_pos;
	mob->sex		= pMobIndex->sex;
        if (mob->sex == 3) /* random sex */
            mob->sex = number_range(1,2);
	mob->race		= pMobIndex->race;

                currency = find_currency_unreset(mob);
	if (pMobIndex->gold == 0)  mob->gold[currency] = 0;
	else mob->gold[currency] = number_range(pMobIndex->gold/2, pMobIndex->gold * 3/2);

                mod = get_currency_modifier(pMobIndex->area, currency);
                mod2 = get_currency_modifier(pMobIndex->area, 0);

                mod = (mod - 1.0) / 2.0 + 1.0;
                mod2 = (mod2 - 1.0) / 2.0 + 1.0;
                
                mob->gold[currency] = (long) (mob->gold[currency] * (mod2 /mod));
                if (pMobIndex->pShop && pMobIndex->gold > 1000) mob->gold[currency] /= 2;

	mob->form		= pMobIndex->form;
	mob->parts		= pMobIndex->parts;
	mob->size		= pMobIndex->size;
	mob->material		= pMobIndex->material;

    } else {

	mob->act		= pMobIndex->act | ACT_WARRIOR;
	mob->affected_by	= pMobIndex->affected_by;
	mob->alignment		= pMobIndex->alignment;
	mob->group		= pMobIndex->group;
	mob->hitroll		= pMobIndex->hitroll;

	switch(number_range(1,3)) {
	    case (1):
                mob->atk_type = WDT_PIERCE;
                mob->dam_type = DAM_PIERCE;
              	break;

	    case (2):
                mob->atk_type = WDT_SLASH;
                mob->dam_type = DAM_SLASH;
                break;

	    case (3):
                mob->atk_type = WDT_PUNCH;
                mob->dam_type = DAM_BASH;
                break;
	}

	mob->race		= pMobIndex->race;
	mob->off_flags		= pMobIndex->off_flags;
	mob->imm_flags		= pMobIndex->imm_flags;
	mob->envimm_flags	= pMobIndex->envimm_flags;
	mob->res_flags		= pMobIndex->res_flags;
	mob->vuln_flags		= pMobIndex->vuln_flags;
	mob->start_pos		= pMobIndex->start_pos;
	mob->default_pos	= pMobIndex->default_pos;
	mob->sex		= pMobIndex->sex;
	mob->gold[find_currency_unreset(mob)] = pMobIndex->gold/100;
	mob->form		= pMobIndex->form;
	mob->parts		= pMobIndex->parts;
	mob->size		= SIZE_MEDIUM;
	mob->material		= 0;

    }

   /* Some mobs speak english, other have their own tongue... */ 

    if (pMobIndex->language > 0) {
         mob->speak[SPEAK_PUBLIC]  = pMobIndex->language;
         mob->speak[SPEAK_PRIVATE] = pMobIndex->language;

    } else if ( race_array[mob->race].language > 0 ) {
         mob->speak[SPEAK_PUBLIC]  = race_array[mob->race].language;
         mob->speak[SPEAK_PRIVATE] = race_array[mob->race].language;

    } else {
         mob->speak[SPEAK_PUBLIC]  = gsn_english;
         mob->speak[SPEAK_PRIVATE] = gsn_english;
    }

   /* Start off in top condition... */

    mob->max_hit  = 100; mob->hit  = 100;
    mob->max_mana = 100; mob->mana = 100;
    mob->max_move = 100; mob->move = 100;

    if (mud.diff == DIFF_EASY) {
        mob->max_hit = mob->max_hit *2 / 3;
        mob->max_mana = mob->max_mana *2 / 3;
        mob->max_move = mob->max_move *2 / 3;
    }
    if (mud.diff == DIFF_HARD) {
        mob->max_hit = mob->max_hit *3 / 2;
        mob->max_mana = mob->max_mana *3 / 2;
        mob->max_move = mob->max_move *3 / 2;
    }

    if (IS_SET(mob->act,ACT_MOUNT)) {
        mob->max_move *=3;
        mob->move *=3;
    }

   /* Determine the Mobs nature */

    mob->nature = pMobIndex->nature;

   /* Base stats... */

    for (i = 0; i < MAX_STATS; i ++) {
      mob->perm_stat[i] = 39 + 4 * dice(2,4) + dice(1,4);
    }
            
   /* Adjust stats for ACT... */

    if (IS_SET(mob->act,ACT_WARRIOR)) {
      mob->perm_stat[STAT_STR] += 12;
      mob->perm_stat[STAT_INT] -= 4;
      mob->perm_stat[STAT_CON] += 8;
    }
        
    if (IS_SET(mob->act,ACT_THIEF)) {
      mob->perm_stat[STAT_DEX] += 12;
      mob->perm_stat[STAT_INT] += 4;
      mob->perm_stat[STAT_WIS] -= 4;
      mob->perm_stat[STAT_LUC] += 8;
    }
        
    if (IS_SET(mob->act,ACT_CLERIC)) {
      mob->perm_stat[STAT_WIS] += 12;
      mob->perm_stat[STAT_DEX] -= 4;
      mob->perm_stat[STAT_STR] += 4;
      mob->perm_stat[STAT_CHA] += 8;
    }
        
    if (IS_SET(mob->act,ACT_MAGE)) {
      mob->perm_stat[STAT_INT] += 12;
      mob->perm_stat[STAT_STR] -= 4;
      mob->perm_stat[STAT_DEX] += 4;
    }
     
   /* Adjust stats for offensive abilties... */
   
    if (IS_SET(mob->off_flags,OFF_FAST)) {
      mob->perm_stat[STAT_DEX] += 8;
    }
            
   /* Adjust stats for size... */

    mob->perm_stat[STAT_STR] += 8 * (mob->size - SIZE_MEDIUM);
    mob->perm_stat[STAT_CON] += 4 * (mob->size - SIZE_MEDIUM);

   /* Adjust stats for nature... */
    adjust_stats_by_nature(mob);

   /* Set level dependent stuff... */

    if (profession) prof = get_profession(profession);
    set_mobile_level(mob, prof, clev);

    if (!prof) mob->effective = pMobIndex->skills;

   /* Set the Mobs start position... */

    mob->position = mob->start_pos;

   /* link the mob to the world list */
    mob->next		= char_list;
    char_list		= mob;
    pMobIndex->count++;

    if (IS_SET(mob->act, ACT_RAND_KIT)) {
        if (number_percent() <= 25) wield_random_armor(mob);
    }

   /* Null conversational records... */

    mob->cr = NULL;

   /* All instances of a mob use the same trigger set... */

    mob->triggers = pMobIndex->triggers;

   /* No deeds, No quests and no professions... */

    mob->deeds = NULL;

    mob->quests = NULL;

    mob->profs = NULL;

   /* Wipe memory */

    for(i = 0; i < MOB_MEMORY; i++) {
      mob->memory[i] = NULL;
    }     
 
   /* Set up subscriptions... */

    mob->time_wev = pMobIndex->time_wev;

    if (pMobIndex->monitors != NULL) {
      start_monitors(mob);
    }

   /* Remember how frightning it is... */

    mob->fright = pMobIndex->fright;

   /* Remember what its tracks are like... */

    mob->tracks = pMobIndex->tracks;

    if (mob->tracks != NULL) {
      increase_usage(mob->tracks);
    }

   /* All done... */  

    return mob;
}



/* Set a mobiles level... */

void set_mobile_level( CHAR_DATA *mob, PROF_DATA *prof, int clev ) {
MOB_INDEX_DATA *pMobIndex;
int old_max_hit;
int old_max_mana;
int old_max_move;

   /* Find the mobs index... */

    pMobIndex = mob->pIndexData;

    if (pMobIndex == NULL) return;

   /* Set the Mob base level... */

    mob->level		= clev;

   /* Random level variation... */
 
    if (mob->level >= 10) {
       mob->level += dice(1, mob->level/10);
       mob->level -= dice(1, mob->level/10);
    }
 
    mob->level = URANGE(1, mob->level, MAX_LEVEL);

    mob->level = number_fuzzy(mob->level);

   /* Set base experience... */

    mob->exp		= exp_for_level(mob->level);

   /* Base combat values... */

    old_max_hit = mob->max_hit;

    mob->max_hit  = mob->level * ( 8 + (( mob->perm_stat[STAT_CON] )/8 ));

    mob->max_hit += dice(1, mob->level);

    old_max_mana = mob->max_mana;

    mob->max_mana = mob->level * ( 5 + (( mob->perm_stat[STAT_INT]  + mob->perm_stat[STAT_WIS] )/16));

    old_max_move = mob->max_move;

    mob->max_move = mob->level * ( 8 + (( mob->perm_stat[STAT_DEX] )/8 ));

    mob->damage[DICE_NUMBER]	= 1 + (mob->level/8);
    mob->damage[DICE_TYPE]	= 5 + (mob->level/6);
    if (mud.diff == DIFF_EASY) {
        mob->damage[DICE_NUMBER] = mob->damage[DICE_NUMBER] *4 /5;
        mob->damage[DICE_TYPE] = mob->damage[DICE_TYPE] *4 /5;
    }
    if (mud.diff == DIFF_HARD) {
        mob->damage[DICE_NUMBER] = mob->damage[DICE_NUMBER] *5 /4;
        mob->damage[DICE_TYPE] = mob->damage[DICE_TYPE] *5 /4;
    }

    if (mob->level > 50) mob->hitroll = UMAX(mob->hitroll, mob->level);
    mob->damroll		= mob->hitroll + (mob->level)/2 ;
    if (mud.diff == DIFF_EASY) {
        mob->hitroll = mob->hitroll *2 /3;
        mob->damroll = mob->damroll *2 /3;
    }
    if (mud.diff == DIFF_HARD) {
        mob->hitroll = mob->hitroll *3 /2;
        mob->damroll = mob->damroll *3 /2;
    }


   /* Adjust combat values for nature... */

    if (IS_SET(mob->nature, NATURE_STURDY)) {
      mob->max_hit += 2*mob->level; 
    }

    if (IS_SET(mob->nature, NATURE_FRAGILE)) {
      mob->max_hit -= 2*mob->level; 
    }

    if (IS_SET(mob->nature, NATURE_MAGICAL)) {
      mob->max_mana += 2*mob->level; 
    }

    if (IS_SET(mob->nature, NATURE_MUNDAIN)) {
      mob->max_mana -= 2*mob->level; 
    }

    if (IS_SET(mob->nature, NATURE_VISCIOUS)) {
      mob->damroll += mob->level; 
    }

    if (IS_SET(mob->nature, NATURE_HARMLESS)) {
      mob->damroll += mob->level; 
    }

    if (IS_SET(mob->nature, NATURE_MONSTEROUS)) {
      mob->max_hit += mob->level; 
      mob->max_mana += mob->level; 
      mob->damroll += (mob->level)/2; 
    }

   /* Apply the values... */

    mob->hit  = (mob->max_hit  * mob->hit )/old_max_hit;
    mob->mana = (mob->max_mana * mob->mana)/old_max_mana;
    mob->move = (mob->max_move * mob->move)/old_max_move;

   /* Default AC... */

    mob->armor[AC_PIERCE]	= 100 - (4 * mob->level);
    mob->armor[AC_BASH]		= 100 - (4 * mob->level);
    mob->armor[AC_SLASH]	= 100 - (4 * mob->level);
    mob->armor[AC_EXOTIC]	= 100 - (2* mob->level);

   /* Adjust AC for DEX... */

    mob->armor[AC_PIERCE]	-= (mob->perm_stat[STAT_DEX]/3);
    mob->armor[AC_BASH]		-= (mob->perm_stat[STAT_DEX]/4); 
    mob->armor[AC_SLASH]	-= (mob->perm_stat[STAT_DEX]/3);
    mob->armor[AC_EXOTIC]	-= (mob->perm_stat[STAT_DEX]/4);

   /* Adjust AC for nature... */

    if (IS_SET(mob->nature, NATURE_ARMOURED)) {
      mob->armor[AC_PIERCE]	-= ARMOR_DAM_PTS;
      mob->armor[AC_BASH]	-= ARMOR_DAM_PTS;
      mob->armor[AC_SLASH]	-= ARMOR_DAM_PTS;
      mob->armor[AC_EXOTIC]	-= ARMOR_DAM_PTS;
    }

    if (IS_SET(mob->nature, NATURE_EXPOSED)) {
      mob->armor[AC_PIERCE]	+= mob->level;
      mob->armor[AC_BASH]	+= mob->level;
      mob->armor[AC_SLASH]	+= mob->level;
    }

    if (IS_SET(mob->nature, NATURE_MAGICAL)) {
      mob->armor[AC_EXOTIC]	-= mob->level * 2;
    }

    if (IS_SET(mob->nature, NATURE_MONSTEROUS)) {
      mob->armor[AC_PIERCE]	-= mob->level;
      mob->armor[AC_BASH]	-= mob->level * 2;
      mob->armor[AC_SLASH]	-= mob->level;
      mob->armor[AC_EXOTIC]	-= mob->level;
    }

   if (prof && mob->level > 0) {
      int num_prac = -1;
      int practice[MAX_SKILL];
      int sn, lev;

      if (!mob->effective) mob->effective = getSkillData();

      for(sn = 0; sn < MAX_SKILL; sn++) {
            if (prof->levels->skill[sn] == 0) {
                 setSkill(mob->effective, sn, number_fuzzy(42));
                 practice[++num_prac] = sn;
            } else {
                 setSkill(mob->effective, sn, 0);
            }
      }

      for(lev = 1; lev < mob->level; lev++) {
             for(sn = 0; sn < MAX_SKILL; sn++) {
                 if (prof->levels->skill[sn] == lev) practice[++num_prac] = sn;
             }

             sn = practice[number_range(0, num_prac)];
             addCharSkill(mob, sn, number_fuzzy(8));
             sn = practice[number_range(0, num_prac)];
             addCharSkill(mob, sn, number_fuzzy(8));
             sn = practice[number_range(0, num_prac)];
             addCharSkill(mob, sn, number_fuzzy(8));
             sn = practice[number_range(0, num_prac)];
             addCharSkill(mob, sn, number_fuzzy(8));
      }
   }
   return;
}


/* duplicate a mobile exactly -- except inventory */
void clone_mobile(CHAR_DATA *parent, CHAR_DATA *clone) {
int i;
AFFECT_DATA *paf;

    if ( parent == NULL || clone == NULL || !IS_NPC(parent)) return;
    
    /* start fixing values */ 
    clone->name 	= str_dup(parent->name);
    clone->version	= parent->version;
    clone->short_descr	= str_dup(parent->short_descr);
    clone->long_descr	= str_dup(parent->long_descr);
    clone->description	= str_dup(parent->description);
    clone->bio		= str_dup(parent->bio);
    clone->sex		= parent->sex;
    clone->pc_class	= parent->pc_class;
    clone->race		= parent->race;
    clone->level	= parent->level;
    clone->timer	= parent->timer;
    clone->wait		= parent->wait;
    clone->hit		= parent->hit;
    clone->max_hit	= parent->max_hit;
    clone->mana		= parent->mana;
    clone->max_mana	= parent->max_mana;
    clone->move		= parent->move;
    clone->max_move	= parent->max_move;

    for (i = 0; i < MAX_CURRENCY; i++) clone->gold[i] = parent->gold[i];

    clone->exp		= parent->exp;
    clone->act		= parent->act;
    clone->plr		= parent->plr;
    clone->comm		= parent->comm;
    clone->imm_flags	= parent->imm_flags;
    clone->envimm_flags	= parent->envimm_flags;
    clone->res_flags	= parent->res_flags;
    clone->vuln_flags	= parent->vuln_flags;
    clone->invis_level	= parent->invis_level;
    clone->affected_by	= parent->affected_by;
    clone->position	= parent->position;
    clone->activity	= parent->activity;

    if (parent->pos_desc != NULL) {
      clone->pos_desc	= str_dup(parent->pos_desc);
    }

    if (parent->acv_desc != NULL) {
      clone->acv_desc	= str_dup(parent->acv_desc);
    }

    clone->acv_key	= parent->acv_key;
    clone->acv_state	= parent->acv_state;
    clone->acv_int	                = parent->acv_int;
    clone->acv_int2	= parent->acv_int2;
    clone->acv_int3	= parent->acv_int3;
 
    if (parent->acv_text != NULL) {      
      clone->acv_text	= str_dup(parent->acv_text);
    }      

    clone->practice	= parent->practice;
    clone->train	= parent->train;
    clone->sanity	= parent->sanity;
    clone->saving_throw	= parent->saving_throw;
    clone->alignment	= parent->alignment;
    clone->group	= parent->group;
    clone->hitroll	= parent->hitroll;
    clone->damroll	= parent->damroll;
    clone->wimpy	= parent->wimpy;
    clone->form		= parent->form;
    clone->parts	= parent->parts;
    clone->size		= parent->size;
    clone->material	= parent->material;
    clone->off_flags	= parent->off_flags;
    clone->atk_type	= parent->atk_type;
    clone->dam_type	= parent->dam_type;
    clone->start_pos	= parent->start_pos;
    clone->default_pos	= parent->default_pos;
    clone->spec_fun	= parent->spec_fun;
    clone->were_type	= parent->were_type;

    for (i = 0; i < 4; i++)
    	clone->armor[i]	= parent->armor[i];

    for (i = 0; i < MAX_STATS; i++)
    {
	clone->perm_stat[i]	= parent->perm_stat[i];
	clone->mod_stat[i]	= parent->mod_stat[i];
    }

    for (i = 0; i < 3; i++)
	clone->damage[i]	= parent->damage[i];

    /* now add the affects */
    for (paf = parent->affected; paf != NULL; paf = paf->next)
        affect_to_char(clone,paf);

    clone->gun_state[0] = parent->gun_state[0];
    clone->gun_state[1] = parent->gun_state[1];
    clone->effective    = parent->effective;
    clone->cr 		= NULL;
    clone->mcb 		= NULL;
    clone->triggers	= parent->triggers;
    clone->deeds 	= NULL; 
    clone->quests 	= NULL; 

   /* Clone memory */

    for(i = 0; i < MOB_MEMORY; i++) {
      if (parent->memory[i] != NULL) {
        clone->memory[i] = str_dup(parent->memory[i]);
      } else {
        clone->memory[i] = NULL;
      }
    }     
 
    clone->time_wev = parent->time_wev;
    clone->fright   = parent->fright;

   /* Remember what its tracks are like... */

    clone->tracks = parent->tracks;

    if (clone->tracks != NULL) {
      increase_usage(clone->tracks);
    }

    return; 
}


/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object( OBJ_INDEX_DATA *pObjIndex, int level ){
static OBJ_DATA obj_zero;
OBJ_DATA *obj;
AFFECT_DATA *paf;
bool mustenchant;

    if (pObjIndex == NULL )    {
	bug( "Create_object: NULL pObjIndex - defaulting to 20", 0 );
                 pObjIndex = get_obj_index( 20 );
    }

    if (pObjIndex->item_type == ITEM_PROTOCOL )    {
	bug( "Create_object: Won't create protocol - defaulting to 20", 0 );
                 pObjIndex = get_obj_index( 20 );
    }

    if ( obj_free == NULL )    {
	obj		= (OBJ_DATA *) alloc_perm( sizeof(*obj) );
    }    else    {
	obj		= obj_free;
	obj_free	= obj_free->next;
    }

    *obj		= obj_zero;
    obj->pIndexData	= pObjIndex;
    obj->in_room	= NULL;
    obj->enchanted	= FALSE;

    if (pObjIndex->new_format)
	obj->level = pObjIndex->level;
    else
	obj->level		= UMAX(0,level);
    obj->wear_loc	= -1;
    obj->name           = str_dup( pObjIndex->name );
    obj->short_descr    = str_dup( pObjIndex->short_descr );
    obj->description    = str_dup( pObjIndex->description );

    obj->material	= pObjIndex->material;
    obj->item_type	= pObjIndex->item_type;
    obj->extra_flags	= pObjIndex->extra_flags;
    obj->wear_flags	= pObjIndex->wear_flags;
    obj->condition  	= pObjIndex->condition;
    obj->value[0]	= pObjIndex->value[0];
    obj->value[1]	= pObjIndex->value[1];
    obj->value[2]	= pObjIndex->value[2];
    obj->value[3]	= pObjIndex->value[3];
    obj->value[4]	= pObjIndex->value[4];
    obj->valueorig[0]	= pObjIndex->value[0];
    obj->valueorig[1]	= pObjIndex->value[1];
    obj->valueorig[2]	= pObjIndex->value[2];
    obj->valueorig[3]	= pObjIndex->value[3];
    obj->valueorig[4]	= pObjIndex->value[4];
    obj->weight		= pObjIndex->weight;
    obj->size		= SIZE_UNKNOWN;

    if (level == -1 || pObjIndex->new_format) obj->cost	= pObjIndex->cost;
    else obj->cost	= number_fuzzy( 10 ) * number_fuzzy( level ) * number_fuzzy( level );

    obj->zmask		= pObjIndex->zmask;

    if (obj->item_type == ITEM_PASSPORT) mustenchant = TRUE;
    else mustenchant = FALSE;
    for (paf = pObjIndex->affected; paf != NULL; paf = paf->next) {
          if (paf->location == APPLY_EFFECT) mustenchant = TRUE;
    }

    if (mustenchant
    && !obj->enchanted
    && level >= 0) {
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


    /*
     * Mess with object properties.
     */
    switch ( obj->item_type )    {
    default:
	bug( "Read_object: vnum %d bad type.", pObjIndex->vnum );
	break;

    case ITEM_LIGHT:
	if (obj->value[2] == 999) obj->value[2] = -1;
	break;

    case ITEM_TREASURE:
    case ITEM_FURNITURE:
    case ITEM_TRASH:
    case ITEM_CONTAINER:
    case ITEM_KEY_RING:
    case ITEM_DRINK_CON:
    case ITEM_KEY:
    case ITEM_FOOD:
    case ITEM_BOAT:
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
    case ITEM_FOUNTAIN:
    case ITEM_MAP:
    case ITEM_CLOTHING:
    case ITEM_PRIDE:
    case ITEM_COMPONENT:
    case ITEM_PORTAL: 
    case ITEM_LOCKER:
    case ITEM_LOCKER_KEY:
    case ITEM_CLAN_LOCKER: 
    case ITEM_BOOK:
    case ITEM_IDOL:
    case ITEM_SCRY:
    case ITEM_DIGGING:
    case ITEM_FORGING:
    case ITEM_RAW:
    case ITEM_BONDAGE:
    case ITEM_TATTOO:
    case ITEM_TRAP:
    case ITEM_PAPER:
    case ITEM_EXPLOSIVE:
    case ITEM_AMMO:
    case ITEM_SLOTMACHINE:
    case ITEM_HERB:
    case ITEM_PIPE:
    case ITEM_TIMEBOMB:
    case ITEM_TREE:
    case ITEM_CAMERA:
    case ITEM_PHOTOGRAPH:
    case ITEM_WARP_STONE:
    case ITEM_ROOM_KEY:
    case ITEM_GEM:
    case ITEM_JEWELRY:
    case ITEM_SHARE:
    case ITEM_FIGURINE:
    case ITEM_POOL:
    case ITEM_GRIMOIRE:
    case ITEM_LIGHT_REFILL:
    case ITEM_DOLL:
    case ITEM_FOCUS:
    case ITEM_PROTOCOL:
    case ITEM_DECORATION:
    case ITEM_PASSPORT:
    case ITEM_INSTRUMENT:
	break;

    case ITEM_JUKEBOX:
                int i;

	for (i = 0; i < 5; i++)
	   obj->value[i] = -1;
	break;

    case ITEM_SCROLL:
	if (level != -1 && !pObjIndex->new_format)
	    obj->value[0]	= number_fuzzy( obj->value[0] );
	break;

    case ITEM_WAND:
    case ITEM_STAFF:
	if (level != -1 && !pObjIndex->new_format)	{
	    obj->value[0]	= number_fuzzy( obj->value[0] );
	    obj->value[1]	= number_fuzzy( obj->value[1] );
	    obj->value[2]	= obj->value[1];
	}
	break;

    case ITEM_WEAPON:
	if (level != -1 && !pObjIndex->new_format)	{
	    obj->value[1] = number_fuzzy( number_fuzzy( 1 * level / 4 + 2 ) );
	    obj->value[2] = number_fuzzy( number_fuzzy( 3 * level / 4 + 6 ) );
	}
	break;

    case ITEM_ARMOR:
	if (level != -1 && !pObjIndex->new_format)	{
	    obj->value[0]	= number_fuzzy( level / 5 + 3 );
	    obj->value[1]	= number_fuzzy( level / 5 + 3 );
	    obj->value[2]	= number_fuzzy( level / 5 + 3 );
	}
	break;

    case ITEM_POTION:
    case ITEM_PILL:
	if (level != -1 && !pObjIndex->new_format)
	    obj->value[0] = number_fuzzy( number_fuzzy( obj->value[0] ) );
	break;

    case ITEM_MONEY:
	if (!pObjIndex->new_format)
	    obj->value[0]	= obj->cost;
	break;
    }

    obj->next		= object_list;
    object_list		= obj;
    pObjIndex->count++;

    return obj;
}

/* duplicate an object exactly -- except contents */
void clone_object(OBJ_DATA *parent, OBJ_DATA *clone) {
int i;
AFFECT_DATA *paf;

    if (parent == NULL || clone == NULL) return;

    /* start fixing the object */
    clone->name 		= str_dup(parent->name);
    clone->short_descr 	= str_dup(parent->short_descr);
    clone->description	= str_dup(parent->description);
    clone->item_type	= parent->item_type;
    clone->extra_flags	= parent->extra_flags;
    clone->wear_flags	= parent->wear_flags;
    clone->weight		= parent->weight;
    clone->cost		= parent->cost;
    clone->level		= parent->level;
    clone->condition	= parent->condition;
    clone->material		= parent->material;
    clone->timer		= parent->timer;
    clone->zmask		= parent->zmask;
    clone->orig_index 	= parent->orig_index;

    for (i = 0;  i < 5; i ++)
	clone->value[i]	= parent->value[i];
	clone->valueorig[i] = parent->value[i];

    /* affects */
    clone->enchanted	= parent->enchanted;
  
    for (paf = parent->affected; paf != NULL; paf = paf->next) affect_to_obj(clone,paf);

}



/*
 * Clear a new character.
 */
void clear_char( CHAR_DATA *ch ) {
static CHAR_DATA ch_zero;
int i;

    *ch				= ch_zero;

    ch->next_in_room		= NULL;
    ch->master			= NULL;
    ch->leader			= NULL;
    ch->fighting			= NULL;
    ch->huntedBy			= NULL;
    ch->distractedBy		= NULL;
    ch->reply			= NULL;
    ch->pet			= NULL;
    ch->spec_fun			= NULL;
    ch->pIndexData		= NULL;
    ch->reset			= NULL;
    ch->desc			= NULL;
    ch->affected			= NULL;
    ch->carrying			= NULL;
    ch->in_room			= NULL;
    ch->was_in_room		= NULL;
    ch->waking_room		= NULL;
    ch->dreaming_room		= NULL;
    ch->pcdata 			= NULL;
    ch->on_furniture                             = NULL;
    ch->trans_obj                                   = NULL;
    ch->ooz			= NULL;

    ch->imm_flags 			= 0;
    ch->envimm_flags		= 0;
    ch->res_flags 			= 0;
    ch->vuln_flags			= 0;

    ch->act 			= 0;
    ch->plr			= 0;

    ch->in_zone 		= -1;

    ch->name			= &str_empty[0];
    ch->short_descr		= &str_empty[0];
    ch->long_descr		= &str_empty[0];
    ch->description		= &str_empty[0];
    ch->bio			= &str_empty[0];
    ch->lines			= PAGELEN;
    for (i = 0; i < 4; i++)
    	ch->armor[i]		= 100;
    ch->comm			= 0;
    ch->position		= POS_STANDING;
    ch->activity		= ACV_NONE;      
    ch->pos_desc		= NULL;      
    ch->acv_desc		= NULL;      
    ch->acv_key			= 0;      
    ch->acv_state		= 0;   
    ch->acv_int			= 0;   
    ch->acv_int2		= 0;   
    ch->acv_int3		= 0;   
    ch->acv_text		= NULL;      
    ch->practice		= 0;
    ch->train			= 0;
    ch->affected_by			= 0;
    ch->artifacted_by			= 0;
    ch->sanity			= 0;
    ch->hit			= 20;
    ch->max_hit			= 20;
    ch->mana			= 100;
    ch->max_mana		= 100;
    ch->move			= 100;
    ch->max_move		= 100;
    ch->chat_state 		= 0;

    for (i = 0; i < MAX_STATS; i ++)  {
	ch->perm_stat[i] = 52; 
	ch->mod_stat[i] = 0;
    }

    ch->huntedBy                = NULL;
    ch->prey                    = NULL;

    ch->true_name 		= NULL;

    ch->quaffLast		= current_time;         /* Mik */

    ch->gun_state[0] 		= GUN_NO_AMMO;          /* Mik */
    ch->gun_state[1]            = GUN_NO_AMMO;          /* Mik */

    ch->cr 			= NULL;

    ch->mcb			= NULL;
 
    ch->triggers		= get_trs();

    ch->deeds			= NULL;

    ch->quests			= NULL;

    ch->wimpy			= 0;
    ch->wimpy_dir		= DIR_NONE;

    ch->quitting 		= FALSE;
    ch->fleeing			= FALSE;

    ch->condition[COND_FOOD]	= COND_STUFFED - 1;
    ch->condition[COND_DRINK]	= COND_SLUSHY - 1;
    ch->condition[COND_DRUNK]	= 0;
    ch->condition[COND_FAT]	= 0;
    
     ch->limb[0] = 0;
     ch->limb[0] = 0;
     ch->limb[0] = 0;
     ch->limb[0] = 0;
     ch->limb[0] = 0;
     ch->limb[0] = 0;

    ch->version 		= 6;
    ch->were_type 		= 0;
    return;
}


/*
 * Free a character.
 */
void free_char( CHAR_DATA *ch ) {
OBJ_DATA *obj;
OBJ_DATA *obj_next;
AFFECT_DATA *paf;
AFFECT_DATA *paf_next;
LPROF_DATA *lprof, *next_lprof;
CONV_RECORD *cr, *next_cr;
CONV_SUB_RECORD *csr, *next_csr;
int i;

    if (IS_NPC(ch))	mobile_count--;

   /* Ditch activities... */

    if (ch->pos_desc != NULL) {
      free_string(ch->pos_desc);
      ch->pos_desc = NULL;
    }
 
    if (ch->acv_desc != NULL) {
      free_string(ch->acv_desc);
      ch->acv_desc = NULL;
    }
 
    if (ch->acv_text != NULL) {
      free_string(ch->acv_text);
      ch->acv_text = NULL;
    }

    clear_activity(ch); 
 
   /* Lose inventory... */

    for ( obj = ch->carrying; obj != NULL; obj = obj_next ) {
	obj_next = obj->next_content;
        obj_from_char(obj);
	extract_obj(obj);
    }

   /* Lose out of zone inventory... */

    for ( obj = ch->ooz; obj != NULL; obj = obj_next ) {
	obj_next = obj->next_content;
                obj_from_char_ooz(obj);
	extract_obj(obj);
    }

   /* Lose affect records... */

    for ( paf = ch->affected; paf != NULL; paf = paf_next )   {
	paf_next = paf->next;
	affect_remove( ch, paf );
    }

   /* Get rid of any lprofs... */

    for( lprof = ch->profs; lprof != NULL;) {
      next_lprof = lprof->next;
      free_lprof(lprof);
      lprof = next_lprof;
    }

   /* Lose the strings... */

    free_string( ch->name);
    free_string( ch->short_descr);
    free_string( ch->long_descr);
    free_string( ch->description);
    free_string( ch->bio);
    free_string(ch->poly_name);
    free_string(ch->short_descr_orig);
    free_string(ch->long_descr_orig);
    free_string(ch->description_orig);

    if (ch->true_name) 	free_string(ch->true_name);
    if (ch->acv_text) 	free_string(ch->acv_text);

    if ( ch->pcdata != NULL )  {
	free_string( ch->pcdata->pwd);
	free_string( ch->pcdata->bamfin);
	free_string( ch->pcdata->bamfout);
	free_string( ch->pcdata->title);
	free_string( ch->pcdata->email);
	free_string( ch->pcdata->image);
	free_string( ch->pcdata->url);
	free_string( ch->pcdata->immtitle); 
	free_string( ch->pcdata->prompt);
	free_string( ch->pcdata->native);
	free_string( ch->pcdata->spouse);
	free_string( ch->pcdata->help_topic);
	free_string( ch->pcdata->bloodline);
	free_string( ch->pcdata->cult);
   	free_string( ch->pcdata->questplr);

                if (ch->pcdata->ignore_list) {
                     IGNORE *ig;

                     for (ig = ch->pcdata->ignore_list; ig; ig = ig->next) {
                             free_string(ig->name);
                     }                           
                }

	ch->pcdata->next = pcdata_free;
	pcdata_free      = ch->pcdata;
    }
  
   /* Lose skill data block... */ 

    if (!IS_NPC(ch)) {
         if (ch->effective) freeSkillData(ch->effective);
    } else {
         if (ch->effective && ch->effective != ch->pIndexData->skills) freeSkillData(ch->effective);
    }

    ch->effective = NULL;
 
   /* Lose conversation records... */

    cr = ch->cr;

    while(cr != NULL) {

      csr = cr->csr;

      while (csr != NULL) { 
        next_csr = csr->next;
        free_conv_sub_record(csr);
        csr = next_csr;
      } 

      next_cr = cr->next;
      free_conv_record(cr);
      cr = next_cr;
    }

    ch->cr = NULL;

   /* Lose mcbs... */

    release_mcbs(ch);
 
   /* Lose scripts... */

    if (IS_NPC(ch)) {
      ch->triggers = NULL;
    } else {
      free_trs(ch->triggers);
      ch->triggers = NULL;
    } 

   /* Forget deeds... */

    if (ch->deeds != NULL) {
      free_deed(ch->deeds);
      ch->deeds = NULL;
    }

   /* Forget quests... */

    if (ch->quests != NULL) {
      free_quest(ch->quests);
      ch->quests = NULL;
    }

   /* Forget memberships... */

    if (ch->memberships != NULL) {
      free_membership(ch->memberships);
      ch->memberships = NULL;
    }

   /* Forget memory */

    for(i = 0; i < MOB_MEMORY; i++) {
      if (ch->memory[i] != NULL) {
        free_string(ch->memory[i]);
        ch->memory[i] = NULL;
      }
    }     

   /* Forget tracks... */

    ch->tracks = NULL;

    reduce_usage(ch->tracks);
 
   /* Final clean up... */

    ch->reset		 = NULL;
    ch->quitting	 = FALSE;

   /* Return to free list... */

    ch->next	     = char_free;
    char_free	     = ch;

   /* All done... */

    return;
}



/*
 * Get an extra description from a list.
 */
char *get_extra_descr( char *name, EXTRA_DESCR_DATA *ed )
{
    for ( ; ed != NULL; ed = ed->next )
    {
	if ( is_name(name, ed->keyword ) )
	    return ed->description;
    }
    return NULL;
}


bool show_extra_cond(CHAR_DATA *ch, EXTRA_DESCR_DATA *ed, char *kwds) {
bool shown = FALSE;
MOB_CMD_CONTEXT *mcc;
DEED *deed;


  mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL);

  while (ed != NULL) {

    if ( is_name(kwds, ed->keyword) 
    && ec_satisfied_mcc(mcc, ed->ec, TRUE)) {

     /* Send the description... */ 

      send_to_char("  ", ch);
      send_to_char(ed->description, ch);

     /* See if we have to make a note of it... */

      if (ed->deed != NULL) {

        deed = ed->deed;

        while (deed != NULL) { 
          if (deed->id < 0) {
             long value;

             value = flag_value(discipline_type, deed->title);
             if ( value != NO_FLAG ) {
                 if(!knows_discipline(ch, flag_value(discipline_type, deed->title))) write_to_grimoire(ch, deed->title);
             }

          } else {
              add_deed(ch, deed->id, deed->type, deed->title);
          }
          deed = deed->next;
        }
      }

     /* Note that we have shown something... */

      shown = TRUE;
    }

    ed = ed->next;
  }

  free_mcc(mcc);

  return shown;
}



/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index(VNUM vnum ){
MOB_INDEX_DATA *pMobIndex;

    for ( pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH]; pMobIndex != NULL; pMobIndex  = pMobIndex->next ) {
	if ( pMobIndex->vnum == vnum )  return pMobIndex;
    }

    if ( fBootDb )   {
	bug( "Get_mob_index: bad vnum %ld.", vnum );
	exit( 1 );
    }

    return NULL;
}



/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index(VNUM vnum) {
OBJ_INDEX_DATA *pObjIndex;

    for ( pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH]; pObjIndex != NULL; pObjIndex  = pObjIndex->next )    {
	if ( pObjIndex->vnum == vnum ) return pObjIndex;
    }

    if ( fBootDb )    {
	bug( "Get_obj_index: bad vnum %ld.", vnum );
	exit( 1 );
    }

    return NULL;
}



/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index( VNUM vnum ) {
    ROOM_INDEX_DATA *pRoomIndex;

    if (vnum <= 0) { 
      return NULL;
    }    
 
    for ( pRoomIndex  = room_index_hash[vnum % MAX_KEY_HASH]; pRoomIndex != NULL; pRoomIndex  = pRoomIndex->next ) {
	if ( pRoomIndex->vnum == vnum )  return pRoomIndex;
    }

    if ( fBootDb ) {
bug("Room Vnum is %ld", vnum);
bug("Index is %ld", vnum % MAX_KEY_HASH);
    for ( pRoomIndex  = room_index_hash[vnum % MAX_KEY_HASH];
	  pRoomIndex != NULL;
	  pRoomIndex  = pRoomIndex->next ) {

	bug("Has room %ld", pRoomIndex->vnum);
    }
	bug( "Get_room_index: bad vnum %ld.", vnum );
	exit( 1 );
    }

    return NULL;
}



/*
 * Read a letter from a file.
 */
char fread_letter( FILE *fp )
{
    char c;

    c = ' ';

    if ( fp == NULL ) {
        bug("Attempting to read from a NULL file!",0);
//  The following getc will go BANG! Use gdb backtrace top find the caller   
    }

    do
    {
	c = getc( fp );
    }
    while ( isspace(c) );

    return c;
}



/*
 * Read a number from a file.
 */
int fread_number( FILE *fp )
{
    int number;
    bool sign;
    char c;

    if ( fp == NULL ) {
        bug("Attempting to read from a NULL file!",0);
//  The following getc will go BANG! Use gdb backtrace top find the caller   
    }

    do
    {
	c = getc( fp );
    }
    while ( isspace(c) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
	c = getc( fp );
    }
    else if ( c == '-' )
    {
	sign = TRUE;
	c = getc( fp );
    }

    if ( !isdigit(c) )
    {
	bug( "Fread_number: bad format.", 0 );
	exit( 1 );
    }

    while ( isdigit(c) )
    {
	number = number * 10 + c - '0';
	c      = getc( fp );
    }

    if ( sign )
	number *= -1;

    if ( c == '|' )
	number += fread_number( fp );
    else if ( c != ' ' )
	ungetc( c, fp );

    return number;
}

/*
 * Read a number from a file.
 */
long fread_number_long( FILE *fp )
{
    long number;
    bool sign;
    char c;

    if ( fp == NULL ) {
        bug("Attempting to read from a NULL file!",0);
//  The following getc will go BANG! Use gdb backtrace top find the caller   
    }

    do
    {
	c = getc( fp );
    }
    while ( isspace(c) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
	c = getc( fp );
    }
    else if ( c == '-' )
    {
	sign = TRUE;
	c = getc( fp );
    }

    if ( !isdigit(c) )
    {
	bug( "Fread_number: bad format.", 0 );
	exit( 1 );
    }

    while ( isdigit(c) )
    {
	number = number * 10 + c - '0';
	c      = getc( fp );
    }

    if ( sign )
	number *= -1;

    if ( c == '|' )
	number += fread_number( fp );
    else if ( c != ' ' )
	ungetc( c, fp );

    return number;
}

/*
 * Read a number from a file.
 */
long long fread_number_long_long( FILE *fp )
{
    long long number;
    bool sign;
    char c;

    if ( fp == NULL ) {
        bug("Attempting to read from a NULL file!",0);
//  The following getc will go BANG! Use gdb backtrace top find the caller   
    }

    do
    {
	c = getc( fp );
    }
    while ( isspace(c) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
	c = getc( fp );
    }
    else if ( c == '-' )
    {
	sign = TRUE;
	c = getc( fp );
    }

    if ( !isdigit(c) )
    {
	bug( "Fread_number: bad format.", 0 );
	exit( 1 );
    }

    while ( isdigit(c) )
    {
	number = number * 10 + (c - '0');
	c      = getc( fp );
    }

    if ( sign )
	number *= -1;

    if ( c == '|' )
	number += fread_number( fp );
    else if ( c != ' ' )
	ungetc( c, fp );

    return number;
}

long flag_convert(char letter )
{
    long bitsum = 0;
    char i;

    if ('A' <= letter && letter <= 'Z') 
    {
	bitsum = 1;
	for (i = letter; i > 'A'; i--)
	    bitsum *= 2;
    }
    else if ('a' <= letter && letter <= 'z')
    {
/* Zeran - IDIOTS want 2^26!!! ---> bitsum = 33554432;  2^25 */
	bitsum = 67108864;  /* 2^26 */
	for (i = letter; i > 'a'; i --)
	    bitsum *= 2;
    }

    return bitsum;
}


long fread_flag( FILE *fp) {
    long number;
    char c;

    if ( fp == NULL ) {
        bug("Attempting to read from a NULL file!",0);
//  The following getc will go BANG! Use gdb backtrace top find the caller   
    }

    do
    {
	c = getc(fp);
    }
    while ( isspace(c));

    number = 0;

    if (!isdigit(c) && c != '-' )  /* ROM OLC */
    {
        while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'))
        {
            number += flag_convert(c);
            c = getc(fp);
        }
    }

    if ( c == '-' )      /* ROM OLC */
    {
        number = fread_number( fp );
        return -number;
    }


    while (isdigit(c))
    {
	number = number * 10 + c - '0';
	c = getc(fp);
    }

    if (c == '|')
	number += fread_flag(fp);

    else if  ( c != ' ')
	ungetc(c,fp);

    return number;
}

long long flag_convert_long(char letter ) {
    long long bitsum = 0;
    char i;

    if ('A' <= letter && letter <= 'Z') {
	bitsum = 1;
	for (i = letter; i > 'A'; i--)
	    bitsum *= 2;
    } else if ('a' <= letter && letter <= 'z') {
/* Zeran - IDIOTS want 2^26!!! ---> bitsum = 33554432;  2^25 */
	bitsum = 67108864;  /* 2^26 */
	for (i = letter; i > 'a'; i --)
	    bitsum *= 2;
    }

    return bitsum;
}


long long fread_flag_long( FILE *fp) {
long long number;
char c;

    if ( fp == NULL ) {
        bug("Attempting to read from a NULL file!",0);
//  The following getc will go BANG! Use gdb backtrace top find the caller   
    }

    do {
	c = getc(fp);
    }
    while ( isspace(c));

    number = 0;

    if (!isdigit(c) && c != '-' )  {
        while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
            number += flag_convert_long(c);
            c = getc(fp);
        }
    }

    if ( c == '-' ) {
        number = fread_number( fp );
        return -number;
    }


    while (isdigit(c))  {
	number = number * 10 + (c - '0');
	c = getc(fp);
    }

    if (c == '|') number += fread_flag_long(fp);

    else if  ( c != ' ') ungetc(c,fp);

    return number;
}



/*
 * Read and allocate space for a string from a file.
 * These strings are read-only and shared.
 * Strings are hashed:
 *   each string prepended with hash pointer to prev string,
 *   hash code is simply the string length.
 *   this function takes 40% to 50% of boot-up time.
 */
char *fread_string( FILE *fp ) {

    char *plast;
    char c;
    int check;

    if ( fp == NULL ) {
        bug("Attempting to read from a NULL file!",0);
//  The following getc will go BANG! Use gdb backtrace top find the caller   
    }

    plast = top_string + sizeof(char *);
    if ( plast > &string_space[MAX_STRING - MAX_STRING_LENGTH] ) {
	bug( "Fread_string: MAX_STRING %d exceeded.", MAX_STRING );
	exit( 1 );
    }

    /*
     * Skip blanks.
     * Read first char.
     */

    check = 0;

    do {
	c = getc( fp );
 	check++; 
    } while ( isspace(c) && check < 100000 );

    if ( ( *plast++ = c ) == '~' ) {
	return &str_empty[0];
    }

    for ( ;check < 100000; check++ ) {
        /*
         * Back off the char type lookup,
         *   it was too dirty for portability.
         *   -- Furey
         */

	switch ( *plast = getc(fp) ) {

          default:
            plast++;
            break;
 
          case EOF:
	  /* temp fix */
            bug( "Fread_string: EOF", 0 );
	    return NULL;
            /* exit( 1 ); */
            break;
 
          case '\n':
            plast++;
            *plast++ = '\r';
            break;
 
          case '\r':
            break;
 
          case '~':
            plast++;
	    {
		union
		{
		    char *	pc;
		    char	rgc[sizeof(char *)];
		} u1;
		unsigned int ic;
		int iHash;
		char *pHash;
		char *pHashPrev;
		char *pString;

		plast[-1] = '\0';
		iHash     = UMIN( MAX_KEY_HASH - 1, plast - 1 - top_string );
		for ( pHash = string_hash[iHash]; pHash; pHash = pHashPrev )
		{
		    for ( ic = 0; ic < sizeof(char *); ic++ )
			u1.rgc[ic] = pHash[ic];
		    pHashPrev = u1.pc;
		    pHash    += sizeof(char *);

		    if ( top_string[sizeof(char *)] == pHash[0]
		    &&   !str_cmp( top_string+sizeof(char *)+1, pHash+1 ) )
			return pHash;
		}

		if ( fBootDb )
		{
		    pString		= top_string;
		    top_string		= plast;
		    u1.pc		= string_hash[iHash];
		    for ( ic = 0; ic < sizeof(char *); ic++ )
			pString[ic] = u1.rgc[ic];
		    string_hash[iHash]	= pString;

		    nAllocString += 1;
		    sAllocString += top_string - pString;
		    return pString + sizeof(char *);
		}
		else
		{
		    return str_dup( top_string + sizeof(char *) );
		}
	    }
	}
    }

    if (check > 100000) {
      bug("Fread_string check > 100,000!", 0); 
      exit(1);
    }

    return NULL;
}


char *fread_string_eol( FILE *fp ) {
    static bool char_special[256-EOF];
    char *plast;
    char c;
 
    if ( fp == NULL ) {
        bug("Attempting to read from a NULL file!",0);
    }

    if ( char_special[EOF-EOF] != TRUE )   {
        char_special[EOF -  EOF] = TRUE;
        char_special['\n' - EOF] = TRUE;
        char_special['\r' - EOF] = TRUE;
    }
 
    plast = top_string + sizeof(char *);
    if ( plast > &string_space[MAX_STRING - MAX_STRING_LENGTH] )    {
        bug( "Fread_string: MAX_STRING %d exceeded.", MAX_STRING );
        exit( 1 );
    }
 
    /*
     * Skip blanks.
     * Read first char.
     */
    do  {
        c = getc( fp );
    }
    while ( isspace(c) );
 
    if (( *plast++ = c ) == '\n')  return &str_empty[0];
 
    for ( ;; )  {
        if ( !char_special[ ( *plast++ = getc( fp ) ) - EOF ] )
            continue;
 
        switch ( plast[-1] )    {
        default:
            break;
 
        case EOF:
            bug( "Fread_string_eol  EOF", 0 );
            exit( 1 );
            break;
 
        case '\n':  case '\r':
            {
                union
                {
                    char *      pc;
                    char        rgc[sizeof(char *)];
                } u1;
                unsigned int ic;
                int iHash;
                char *pHash;
                char *pHashPrev;
                char *pString;
 
                plast[-1] = '\0';
                iHash     = UMIN( MAX_KEY_HASH - 1, plast - 1 - top_string );
                for ( pHash = string_hash[iHash]; pHash; pHash = pHashPrev )
                {
                    for ( ic = 0; ic < sizeof(char *); ic++ )
                        u1.rgc[ic] = pHash[ic];
                    pHashPrev = u1.pc;
                    pHash    += sizeof(char *);
 
                    if ( top_string[sizeof(char *)] == pHash[0]
                    &&   !str_cmp( top_string+sizeof(char *)+1, pHash+1 ) )
                        return pHash;
                }
 
                if ( fBootDb )
                {
                    pString             = top_string;
                    top_string          = plast;
                    u1.pc               = string_hash[iHash];
                    for ( ic = 0; ic < sizeof(char *); ic++ )
                        pString[ic] = u1.rgc[ic];
                    string_hash[iHash]  = pString;
 
                    nAllocString += 1;
                    sAllocString += top_string - pString;
                    return pString + sizeof(char *);
                }
                else
                {
                    return str_dup( top_string + sizeof(char *) );
                }
            }
        }
    }
}



/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp ) {
    char c;

    if ( fp == NULL ) {
        bug("Attempting to read from a NULL file!",0);
    }

    do
    {
	c = getc( fp );
    }
    while ( c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}



/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp )
{
    static char word[MAX_INPUT_LENGTH];
    char *pword;
    char cEnd;

    if ( fp == NULL ) {
        bug("Attempting to read from a NULL file!",0);
//  The following getc will go BANG! Use gdb backtrace top find the caller   
    }

    do
    {
	cEnd = getc( fp );
    }
    while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
	pword   = word;
    }
    else
    {
	word[0] = cEnd;
	pword   = word+1;
	cEnd    = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
	*pword = getc( fp );
	if ( cEnd == ' ' ? isspace(*pword) : *pword == cEnd )
	{
	    if ( cEnd == ' ' )
		ungetc( *pword, fp );
	    *pword = '\0';
	    return word;
	}
    }

    bug( "Fread_word: word too long.", 0 );
    exit( 1 );
    return NULL;
}



/*
 * Allocate some ordinary memory,
 *   with the expectation of freeing it someday.
 */
/* void *alloc_mem( int sMem ) {
    void *pMem;
    int iList;

    for ( iList = 0; iList < MAX_MEM_LIST; iList++ ) {
	if ( sMem <= rgSizeList[iList] )
	    break;
    }

    if ( iList == MAX_MEM_LIST ) {
	bug( "Alloc_mem: size %d too large.", sMem );
	exit( 1 );
    }

    if ( rgFreeList[iList] == NULL ) {
	pMem		  = alloc_perm( rgSizeList[iList] );
    } else  {
	pMem              = rgFreeList[iList];
	rgFreeList[iList] = * ((void **) rgFreeList[iList]);
    }

    return pMem;
} */

/* Zeran - this probably isn't the best fix for memory handling, but the
standard routines are flaky. */
void *alloc_mem (int sMem)   { 
	void *pMem;

	if (sMem > 65524) {
		bug( "Alloc_mem: size %d too large.", sMem );
		exit( 1 );
  	}
	
	pMem = malloc (sMem);
	return pMem;	
}


/*
 * Free some memory.
 * Recycle it back onto the free list for blocks of that size.
 */
/* void free_mem( void *pMem, int sMem ) {
    int iList;

	if (pMem == NULL) log_string ("free_mem: hmmm, pMem=NULL, suspicious?");

    for ( iList = 0; iList < MAX_MEM_LIST; iList++ ) {
	if ( sMem <= rgSizeList[iList] )  break;
    }

    if ( iList == MAX_MEM_LIST ) {
	bug( "Free_mem: size %d too large.", sMem );
	exit( 1 );
    }

    * ((void **) pMem) = rgFreeList[iList];
    rgFreeList[iList]  = pMem;

    return;
} */

/* Zeran - changed this to conform with the alloc_mem that I wrote */ 
void free_mem( void *pMem, int sMem ) {

    if (pMem == NULL) {
        log_string ("free_mem: hmmm, pMem=NULL, suspicious?");
    } 

    if (sMem > 65524) {
        bug( "Alloc_mem: size %d too large.", sMem );
        exit( 1 );
    }

    free(pMem);
     return;
}
	
/*
 * Allocate some permanent memory.
 * Permanent memory is never freed,
 *   pointers into it may be copied safely.
 */
void *alloc_perm( int sMem ) {
    static char *pMemPerm;
    static int iMemPerm;
    void *pMem;
    int sm2;

   /* Round up to nearest multiple of 32... */

    sm2 = sMem % sizeof(long);

    if ( sm2 != 0 ) {
      sMem = (sm2 + 1) * sizeof(long);
    }

   /* Cannot allocate larger than the max block size... */

    if ( sMem > MAX_PERM_BLOCK ) {
	bug( "Alloc_perm: %d too large.", sMem );
	exit( 1 );
    }

   /* We need a new block if there's not enough space in the old one... */

    if ( pMemPerm == NULL 
      || iMemPerm + sMem > MAX_PERM_BLOCK ) {

	iMemPerm = 0;

	pMemPerm = (char *) calloc( 1, MAX_PERM_BLOCK );

	if ( pMemPerm == NULL ) {
	    perror( "Alloc_perm" );
	    exit( 1 );
	}
    }

   /* Give out the memory... */

    pMem        = pMemPerm + iMemPerm;

    iMemPerm   += sMem;

   /* Increment records... */

    nAllocPerm += 1;
    sAllocPerm += sMem;

   /* Return a pointer to the momery... */

    return pMem;
}



/*
 * Duplicate a string into dynamic memory.
 * Fread_strings are read-only and shared.
 */
char *str_dup( const char *str ) {

    char *str_new;

    if ( str == NULL ) {
        return NULL;
    }

    if ( str[0] == '\0' ) {
	return &str_empty[0];
    }

    if ( str >= string_space 
      && str  < top_string ) {
	return (char *) str;
    }

    str_new = (char *) alloc_mem( strlen(str) + 1 );
    strcpy( str_new, str );

    return str_new;
}



/*
 * Free a string.
 * Null is legal here to simplify callers.
 * Read-only shared strings are not touched.
 */
void free_string( char *pstr ) {

    if ( pstr == NULL ) return;
    if (pstr[0] == '\0') return; 
    if ( pstr == &str_empty[0] ) return;

    if ( pstr >= string_space 
      && pstr  < top_string )
	return;

    free_mem( pstr, strlen(pstr) + 1 );

    return;
}


/* 
 * Old do_areas commented to use helpfile instead. Moved to diff file.
 */

/*
void do_areas( CHAR_DATA *ch, char *argument ) {
    char buf[MAX_STRING_LENGTH];
    AREA_DATA *pArea1;
    AREA_DATA *pArea2;
    int iArea;
    int iAreaHalf;

    if (argument[0] != '\0')
    {
	send_to_char("No argument is used with this command.\r\n",ch);
	return;
    }

    iAreaHalf = (top_area + 1) / 2;
    pArea1    = area_first;
    pArea2    = area_first;
    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
	pArea2 = pArea2->next;

    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
    {
	sprintf( buf, "{%-39s{%-39s\r\n",
	    pArea1->name, (pArea2 != NULL) ? pArea2->name : "" );
	send_to_char( buf, ch );
	pArea1 = pArea1->next;
	if ( pArea2 != NULL )
	    pArea2 = pArea2->next;
    }

    return;
}

*/

void do_memory( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    sprintf( buf, "Affects %5d\r\n", top_affect    ); send_to_char( buf, ch );
    sprintf( buf, "Areas   %5d\r\n", top_area      ); send_to_char( buf, ch );
    sprintf( buf, "ExDes   %5d\r\n", top_ed        ); send_to_char( buf, ch );
    sprintf( buf, "Exits   %5d\r\n", top_exit      ); send_to_char( buf, ch );
    sprintf( buf, "Helps   %5d\r\n", top_help      ); send_to_char( buf, ch );
    sprintf( buf, "Socials %5d\r\n", social_count  ); send_to_char( buf, ch );
    sprintf( buf, "Mobs    %5d(%d new format)\r\n", top_mob_index,newmobs ); 
    send_to_char( buf, ch );
    sprintf( buf, "(in use)%5d\r\n", mobile_count  ); send_to_char( buf, ch );
    sprintf( buf, "Objs    %5d(%d new format)\r\n", top_obj_index,newobjs ); 
    send_to_char( buf, ch );
    sprintf( buf, "Resets  %5d\r\n", top_reset     ); send_to_char( buf, ch );
    sprintf( buf, "Rooms   %5d\r\n", top_room      ); send_to_char( buf, ch );
    sprintf( buf, "Shops   %5d\r\n", top_shop      ); send_to_char( buf, ch );

    sprintf( buf, "Strings %5d strings of %7d bytes (max %d).\r\n",
	nAllocString, sAllocString, MAX_STRING );
    send_to_char( buf, ch );

    sprintf( buf, "Perms   %5d blocks  of %7d bytes.\r\n",
	nAllocPerm, sAllocPerm );
    send_to_char( buf, ch );

    return;
}

void do_dump( CHAR_DATA *ch, char *argument )
{
    int count,count2,num_pcs,aff_count;
    CHAR_DATA *fch;
    MOB_INDEX_DATA *pMobIndex;
    PC_DATA *pc;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    ROOM_INDEX_DATA *room;
    EXIT_DATA *exit;
    DESCRIPTOR_DATA *d;
    AFFECT_DATA *af;
    FILE *fp;
    int vnum,nMatch = 0;

    /* open file */
    fclose(fpReserve);
    fp = fopen("mem.dmp","w");

    /* report use of data structures */
    
    num_pcs = 0;
    aff_count = 0;

    /* mobile prototypes */
    fprintf(fp,"MobProt	%4d (%8d bytes)\n",
	top_mob_index, top_mob_index * (sizeof(*pMobIndex))); 

    /* mobs */
    count = 0;  count2 = 0;
    for (fch = char_list; fch != NULL; fch = fch->next)
    {
	count++;
	if (fch->pcdata != NULL)
	    num_pcs++;
	for (af = fch->affected; af != NULL; af = af->next)
	    aff_count++;
    }
    for (fch = char_free; fch != NULL; fch = fch->next)
	count2++;

    fprintf(fp,"Mobs	%4d (%8d bytes), %2d free (%d bytes)\n",
	count, count * (sizeof(*fch)), count2, count2 * (sizeof(*fch)));

    /* pcdata */
    count = 0;
    for (pc = pcdata_free; pc != NULL; pc = pc->next)
	count++; 

    fprintf(fp,"Pcdata	%4d (%8d bytes), %2d free (%d bytes)\n",
	num_pcs, num_pcs * (sizeof(*pc)), count, count * (sizeof(*pc)));

    /* descriptors */
    count = 0; count2 = 0;
    for (d = descriptor_list; d != NULL; d = d->next)
	count++;
    for (d= descriptor_free; d != NULL; d = d->next)
	count2++;

    fprintf(fp, "Descs	%4d (%8d bytes), %2d free (%d bytes)\n",
	count, count * (sizeof(*d)), count2, count2 * (sizeof(*d)));

    /* object prototypes */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
        if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
        {
	    for (af = pObjIndex->affected; af != NULL; af = af->next)
		aff_count++;
            nMatch++;
        }

    fprintf(fp,"ObjProt	%4d (%8d bytes)\n",
	top_obj_index, top_obj_index * (sizeof(*pObjIndex)));


    /* objects */
    count = 0;  count2 = 0;
    for (obj = object_list; obj != NULL; obj = obj->next)
    {
	count++;
	for (af = obj->affected; af != NULL; af = af->next)
	    aff_count++;
    }
    for (obj = obj_free; obj != NULL; obj = obj->next)
	count2++;

    fprintf(fp,"Objs	%4d (%8d bytes), %2d free (%d bytes)\n",
	count, count * (sizeof(*obj)), count2, count2 * (sizeof(*obj)));

    /* affects */
    count = 0;
    for (af = affect_free; af != NULL; af = af->next)
	count++;

    fprintf(fp,"Affects	%4d (%8d bytes), %2d free (%d bytes)\n",
	aff_count, aff_count * (sizeof(*af)), count, count * (sizeof(*af)));

    /* rooms */
    fprintf(fp,"Rooms	%4d (%8d bytes)\n",
	top_room, top_room * (sizeof(*room)));

     /* exits */
    fprintf(fp,"Exits	%4d (%8d bytes)\n",
	top_exit, top_exit * (sizeof(*exit)));

    fclose(fp);

    /* start printing out mobile data */
    fp = fopen("mob.dmp","w");

    fprintf(fp,"\nMobile Analysis\n");
    fprintf(fp,  "---------------\n");
    nMatch = 0;
    for (vnum = 0; nMatch < top_mob_index; vnum++)
	if ((pMobIndex = get_mob_index(vnum)) != NULL)
	{
	    nMatch++;
	    fprintf(fp,"#%5ld %3d actv %3d kild level %3d     %s\n",
		pMobIndex->vnum,pMobIndex->count,
		pMobIndex->killed,pMobIndex->level,pMobIndex->short_descr);
	}
    fclose(fp);

    /* start printing out object data */
    fp = fopen("obj.dmp","w");

    fprintf(fp,"\nObject Analysis\n");
    fprintf(fp,  "---------------\n");
    nMatch = 0;
    for (vnum = 0; nMatch < top_obj_index; vnum++)
	if ((pObjIndex = get_obj_index(vnum)) != NULL)
	{
	    nMatch++;
	    fprintf(fp,"#%5ld %3d actv %3d rest level %3d      %s\n",
		pObjIndex->vnum,pObjIndex->count,
		pObjIndex->reset_num,pObjIndex->level,pObjIndex->short_descr);
	}

    /* close file */
    fclose(fp);
    fpReserve = fopen( NULL_FILE, "r" );
}



/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy( int number )
{
    switch ( number_bits( 2 ) )
    {
    case 0:  number -= 1; break;
    case 3:  number += 1; break;
    }

    return UMAX( 1, number );
}



/*
 * Generate a random number.
 */
int number_range( int from, int to )
{
    int power;
    int number;

    if (from == 0 && to == 0)
	return 0;

    if ( ( to = to - from + 1 ) <= 1 )
	return from;

    for ( power = 2; power < to; power <<= 1 )
	;

    while ( ( number = number_mm() & (power -1 ) ) >= to )
	;
 
    return from + number;
}



/*
 * Generate a percentile roll.
 */
int number_percent( void )
{
    int percent;

    while ( (percent = number_mm() & (128-1) ) > 99 )
	;

    return 1 + percent;
}

/*
 * Gernerate an open ended percentile roll
 */

int number_open( void ) {

  int roll;
  int new_roll;

  roll = number_percent();

  if (roll < 6) {

    new_roll = number_percent();

    roll -= new_roll;

    while (new_roll > 95) {
      new_roll = number_percent();
      roll -= new_roll;
    }  
  } else if (roll > 95) {

    new_roll = number_percent();

    roll += new_roll;

    while (new_roll > 95) {
      new_roll = number_percent();
      roll += new_roll;
    }  
  }

  return roll;
}


/*
 * Generate a random door.
 */
int number_door( void ) {

    int door;

    while ( ( door = number_mm() & (15) ) >= DIR_MAX)
	;

    return door;
}

int number_bits( int width )
{
    return number_mm( ) & ( ( 1 << width ) - 1 );
}




/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */
static  int     rgiState[2+55];
 
void init_mm( )
{
    int *piState;
    int iState;
 
    piState     = &rgiState[2];
 
    piState[-2] = 55 - 55;
    piState[-1] = 55 - 24;
 
    piState[0]  = ((int) current_time) & ((1 << 30) - 1);
    piState[1]  = 1;
    for ( iState = 2; iState < 55; iState++ )
    {
        piState[iState] = (piState[iState-1] + piState[iState-2])
                        & ((1 << 30) - 1);
    }
    return;
}
 
 
 
int number_mm( void )
{
    int *piState;
    int iState1;
    int iState2;
    int iRand;
 
    piState             = &rgiState[2];
    iState1             = piState[-2];
    iState2             = piState[-1];
    iRand               = (piState[iState1] + piState[iState2])
                        & ((1 << 30) - 1);
    piState[iState1]    = iRand;
    if ( ++iState1 == 55 )
        iState1 = 0;
    if ( ++iState2 == 55 )
        iState2 = 0;
    piState[-2]         = iState1;
    piState[-1]         = iState2;
    return iRand >> 6;
}


/*
 * Roll some dice.
 */
int dice( int number, int size )
{
    int idice;
    int sum;
 
    switch ( size )
    {
    case 0: return 0;
    case 1: return number;
    }

    for ( idice = 0, sum = 0; idice < number; idice++ )
	sum += number_range( 1, size );

    return sum;
}



/*
 * Simple linear interpolation.
 */
int interpolate( int level, int value_00, int value_32 )
{
    return value_00 + level * (value_32 - value_00) / 32;
}



/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
    {
	if ( *str == '~' )
	    *str = '-';
    }

    return;
}



/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr ) {

    if ( astr == NULL )    {
	bug( "Str_cmp: null astr.", 0 );
	return TRUE;
    }

    if ( bstr == NULL )    {
	bug( "Str_cmp: null bstr.", 0 );
	return TRUE;
    }

    for ( ; *astr || *bstr; astr++, bstr++ )    {
	if ( LOWER(*astr) != LOWER(*bstr) )  return TRUE;
    }

    return FALSE;
}



/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr ){

    if ( astr == NULL )    {
	bug( "Strn_cmp: null astr.", 0 );
	return TRUE;
    }

    if ( bstr == NULL )    {
	bug( "Strn_cmp: null bstr.", 0 );
	return TRUE;
    }

    for ( ; *astr; astr++, bstr++ )    {
	if ( LOWER(*astr) != LOWER(*bstr) )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr ) {
    int sstr1;
    int sstr2;
    int ichar;
    char c0;

    if ( ( c0 = LOWER(astr[0]) ) == '\0' )
	return FALSE;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);

    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
    {
	if ( c0 == LOWER(bstr[ichar]) && !str_prefix( astr, bstr + ichar ) )
	    return FALSE;
    }

    return TRUE;
}



/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);
    if ( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
	return FALSE;
    else
	return TRUE;
}



/*
 * Returns an initial-capped string.
 */
char *capitalize( const char *str )
{
    static char strcap[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
	strcap[i] = LOWER(str[i]);
    strcap[i] = '\0';
    strcap[0] = UPPER(strcap[0]);
    return strcap;
}


/*
 * Append a string to a file.
 */
void append_file( CHAR_DATA *ch, char *file, char *str )
{
    FILE *fp;

    if ( IS_NPC(ch) || str[0] == '\0' )
	return;

    fclose( fpReserve );
    if ( ( fp = fopen( file, "a" ) ) == NULL )
    {
	perror( file );
	send_to_char( "Could not open the file!\r\n", ch );
    }
    else
    {
	fprintf( fp, "[%5ld] %s: %s\n",
	    ch->in_room ? ch->in_room->vnum : 0, ch->name, str );
	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return;
}



/*
 * Reports a bug.
 */
void bug( const char *str, int param )
{
    char buf[MAX_STRING_LENGTH];
    FILE *fp;

    if ( fpArea != NULL )
    {
	int iLine;
	int iChar;

	if ( fpArea == stdin )
	{
	    iLine = 0;
	}
	else
	{
	    iChar = ftell( fpArea );
	    fseek( fpArea, 0, 0 );
	    for ( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
	    {
		while ( getc( fpArea ) != '\n' )
		    ;
	    }
	    fseek( fpArea, iChar, 0 );
	}

	sprintf( buf, "[*****] FILE: %s LINE: %d", strArea, iLine );
	log_string( buf );

	if ( ( fp = fopen( "shutdown.txt", "a" ) ) != NULL )
	{
	    fprintf( fp, "[*****] %s\n", buf );
	    fclose( fp );
	}
    }

    strcpy( buf, "[*****] BUG: " );
    sprintf( buf + strlen(buf), str, param );
    log_string( buf );
/* RT removed due to bug-file spamming 
    fclose( fpReserve );
    if ( ( fp = fopen( BUG_FILE, "a" ) ) != NULL )
    {
	fprintf( fp, "%s\n", buf );
	fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" );
*/

    return;
}



/*
 * Writes a string to the log.
 */
void log_string( const char *str )
{
    char *strtime;

    strtime                    = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';
    fprintf( stderr, "%s :: %s\n", strtime, str );
    return;
}



/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain( void )
{
    return;
}

/* the functions */

/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

int mprog_name_to_type ( char *name )
{
   if ( !str_cmp( name, "in_file_prog"   ) )	return IN_FILE_PROG;
   if ( !str_cmp( name, "act_prog"       ) )    return ACT_PROG;
   if ( !str_cmp( name, "speech_prog"    ) )	return SPEECH_PROG;
   if ( !str_cmp( name, "rand_prog"      ) ) 	return RAND_PROG;
   if ( !str_cmp( name, "fight_prog"     ) )	return FIGHT_PROG;
   if ( !str_cmp( name, "hitprcnt_prog"  ) )	return HITPRCNT_PROG;
   if ( !str_cmp( name, "death_prog"     ) )	return DEATH_PROG;
   if ( !str_cmp( name, "entry_prog"     ) )	return ENTRY_PROG;
   if ( !str_cmp( name, "greet_prog"     ) )	return GREET_PROG;
   if ( !str_cmp( name, "all_greet_prog" ) )	return ALL_GREET_PROG;
   if ( !str_cmp( name, "give_prog"      ) ) 	return GIVE_PROG;
   if ( !str_cmp( name, "bribe_prog"     ) )	return BRIBE_PROG;

   return( ERROR_PROG );
}

/* This routine reads in scripts of MOBprograms from a file */

MPROG_DATA* mprog_file_read( char *f, MPROG_DATA *mprg,
			    MOB_INDEX_DATA *pMobIndex )
{

  char        MOBProgfile[ MAX_INPUT_LENGTH ];
  MPROG_DATA *mprg2;
  FILE       *progfile;
  char        letter;
  bool        done = FALSE;

  sprintf( MOBProgfile, "%s%s", MOB_DIR, f );

  progfile = fopen( MOBProgfile, "r" );
  if ( !progfile )
  {
     bug( "Mob: %d couldnt open mobprog file", pMobIndex->vnum );
     exit( 1 );
  }

  mprg2 = mprg;
  switch ( letter = fread_letter( progfile ) )
  {
    case '>':
     break;
    case '|':
       bug( "empty mobprog file.", 0 );
       exit( 1 );
     break;
    default:
       bug( "in mobprog file syntax error.", 0 );
       exit( 1 );
     break;
  }

  while ( !done )
  {
    mprg2->type = mprog_name_to_type( fread_word( progfile ) );
    switch ( mprg2->type )
    {
     case ERROR_PROG:
        bug( "mobprog file type error", 0 );
        exit( 1 );
      break;
     case IN_FILE_PROG:
        bug( "mprog file contains a call to file.", 0 );
        exit( 1 );
      break;
     default:
        pMobIndex->progtypes = pMobIndex->progtypes | mprg2->type;
        mprg2->arglist       = fread_string( progfile );
        mprg2->comlist       = fread_string( progfile );
        switch ( letter = fread_letter( progfile ) )
        {
          case '>':
             mprg2->next = (MPROG_DATA *) alloc_perm( sizeof( MPROG_DATA ) );
             mprg2       = mprg2->next;
             mprg2->next = NULL;
           break;
          case '|':
             done = TRUE;
           break;
          default:
             bug( "in mobprog file syntax error.", 0 );
             exit( 1 );
           break;
        }
      break;
    }
  }
  fclose( progfile );
  return mprg2;
}

/* Snarf a MOBprogram section from the area file.
 */
void load_mobprogs( FILE *fp ) {
  MOB_INDEX_DATA *iMob;
  MPROG_DATA     *original;
  MPROG_DATA     *working;
  char            letter;
  int             value;

  for ( ; ; )
    switch ( letter = fread_letter( fp ) )    {
    default:
      bug( "Load_mobprogs: bad command '%c'.",letter);
      exit(1);
      break;
    case 'S':
    case 's':
      fread_to_eol( fp ); 
      return;
    case '*':
      fread_to_eol( fp ); 
      break;
    case 'M':
    case 'm':
      value = fread_number( fp );
      if ( ( iMob = get_mob_index( value ) ) == NULL )
      {
	bug( "Load_mobprogs: vnum %d doesnt exist", value );
	exit( 1 );
      }
    
      /* Go to the end of the prog command list if other commands
         exist */

      if ( ( original = iMob->mobprogs ) )
	for ( ; original->next != NULL; original = original->next );

      working = (MPROG_DATA *) alloc_perm( sizeof( MPROG_DATA ) );
      if ( original )
	original->next = working;
      else
	iMob->mobprogs = working;
      working       = mprog_file_read( fread_word( fp ), working, iMob );
      working->next = NULL;
      fread_to_eol( fp );
      break;
    }

  return;

} 

/* This procedure is responsible for reading any in_file MOBprograms.
 */

void mprog_read_programs( FILE *fp, MOB_INDEX_DATA *pMobIndex) {
  MPROG_DATA *mprg;
  char        letter;
  bool        done = FALSE;

  if ( ( letter = fread_letter( fp ) ) != '>' )
  {
      bug( "Load_mobiles: vnum %d MOBPROG char", pMobIndex->vnum );
      exit( 1 );
  }
  pMobIndex->mobprogs = (MPROG_DATA *) alloc_perm( sizeof( MPROG_DATA ) );
  mprg = pMobIndex->mobprogs;

  while ( !done )
  {
    mprg->type = mprog_name_to_type( fread_word( fp ) );
    switch ( mprg->type )
    {
     case ERROR_PROG:
        bug( "Load_mobiles: vnum %d MOBPROG type.", pMobIndex->vnum );
        exit( 1 );
      break;
     case IN_FILE_PROG:
        mprg = mprog_file_read( fread_string( fp ), mprg,pMobIndex );
        fread_to_eol( fp );
        switch ( letter = fread_letter( fp ) )
        {
          case '>':
             mprg->next = (MPROG_DATA *) alloc_perm( sizeof( MPROG_DATA ) );
             mprg       = mprg->next;
             mprg->next = NULL;
           break;
          case '|':
             mprg->next = NULL;
             fread_to_eol( fp );
             done = TRUE;
           break;
          default:
             bug( "Load_mobiles: vnum %d bad MOBPROG.", pMobIndex->vnum );
             exit( 1 );
           break;
        }
      break;
     default:
        pMobIndex->progtypes = pMobIndex->progtypes | mprg->type;
        mprg->arglist        = fread_string( fp );
        fread_to_eol( fp );
        mprg->comlist        = fread_string( fp );
        fread_to_eol( fp );
        switch ( letter = fread_letter( fp ) )
        {
          case '>':
             mprg->next = (MPROG_DATA *) alloc_perm( sizeof( MPROG_DATA ) );
             mprg       = mprg->next;
             mprg->next = NULL;
           break;
          case '|':
             mprg->next = NULL;
             fread_to_eol( fp );
             done = TRUE;
           break;
          default:
             bug( "Load_mobiles: vnum %d bad MOBPROG.", pMobIndex->vnum );
             exit( 1 );
           break;
        }
      break;
    }
  }

  return;

}

void update_obj_orig (OBJ_DATA *obj)
	{
	int count;
	if (obj == NULL)
		{
		bug ("Null object passed to update_obj_orig.",0);
		return;
		}
	for (count=0;count<=4;count++)
		{
		obj->valueorig[count] = obj->value[count];
		}
	return;
	}

void fwrite_disable (void)
	{
	FILE *fptr;
	struct disable_cmd_type *tmp;
	
	if ((fptr = fopen (DISABLE_FILE, "w+")) == NULL)
		{
		bug("Failed openning disable.txt for writing.", 0);
		return;
		}
	
	for (tmp = disable_cmd_list ; tmp != NULL ; tmp = tmp->next)
		{
		fprintf (fptr, "%s~\n", tmp->name);
		fprintf (fptr, "%d\n", tmp->div);
		}

	fprintf (fptr, "@~\n");
	fclose (fptr);
	return;
	}

void load_disable (void)
	{
	FILE *fptr;
	struct disable_cmd_type *tmp=NULL;
	struct disable_cmd_type *previous=NULL;
	bool done=FALSE;
	bool found=FALSE;
	char *word;
	int cmd;

	/* make sure list is null to start with*/
	disable_cmd_list = NULL;
	
	if ((fptr = fopen (DISABLE_FILE, "r")) == NULL)
		{
		bug ("failed openning disable.txt for reading.", 0);
		return;
		}
	
	while (!done)
		{
		word = fread_string (fptr);
		if (!str_cmp (word, "@"))
			{
			done = TRUE;
			}
		else
			{
	        for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
   		         {
   		         if ( word[0] == cmd_table[cmd].name[0]
   		         &&   !str_prefix( word, cmd_table[cmd].name ))
   		             {
   		             found = TRUE;
   		             break;
   		             }
       		     }
			if (!found)
				{
				bug("Bad disable command name in disable.txt",0);
				fread_number (fptr);
				continue;
				}
			tmp = (struct disable_cmd_type *) malloc (sizeof (struct disable_cmd_type));
			tmp->next = NULL;
			tmp->name = str_dup (word);
			tmp->div = fread_number (fptr);
			tmp->div = UMIN (DIV_IMPLEMENTOR, tmp->div);
			tmp->disable_fcn = cmd_table[cmd].do_fun;

			if (disable_cmd_list == NULL)
				disable_cmd_list = tmp;
			else 
				previous->next = tmp;				

			previous = tmp;
			}
		free_string (word);
		}
	fclose (fptr);
	return;
	}


char *fread_html( FILE *fp ) {
    char *plast;
    char c;
    int check;

    if ( fp == NULL ) {
        bug("Attempting to read from a NULL file!",0);
    }

    plast = top_string + sizeof(char *);
    if ( plast > &string_space[MAX_STRING - MAX_STRING_LENGTH] ) {
	bug( "Fread_string: MAX_STRING %d exceeded.", MAX_STRING );
	exit( 1 );
    }

    check = 0;

    do {
	c = getc( fp );
 	check++; 
    } while ( isspace(c) && check < 100000 );

    if ( ( *plast++ = c ) == EOF ) {
	return &str_empty[0];
    }

    for ( ;check < 100000; check++ ) {
	switch ( *plast = getc(fp) ) {

          default:
            plast++;
            break;
 
 
          case '\n':
            plast++;
            *plast++ = '\r';
            break;
 
          case '\r':
            break;
 
          case EOF:
            plast++;
	    {
		union
		{
		    char *	pc;
		    char	rgc[sizeof(char *)];
		} u1;
		unsigned int ic;
		int iHash;
		char *pHash;
		char *pHashPrev;
		char *pString;

		plast[-1] = '\0';
		iHash     = UMIN( MAX_KEY_HASH - 1, plast - 1 - top_string );
		for ( pHash = string_hash[iHash]; pHash; pHash = pHashPrev )
		{
		    for ( ic = 0; ic < sizeof(char *); ic++ )
			u1.rgc[ic] = pHash[ic];
		    pHashPrev = u1.pc;
		    pHash    += sizeof(char *);

		    if ( top_string[sizeof(char *)] == pHash[0]
		    &&   !str_cmp( top_string+sizeof(char *)+1, pHash+1 ) )
			return pHash;
		}

		if ( fBootDb )
		{
		    pString		= top_string;
		    top_string		= plast;
		    u1.pc		= string_hash[iHash];
		    for ( ic = 0; ic < sizeof(char *); ic++ )
			pString[ic] = u1.rgc[ic];
		    string_hash[iHash]	= pString;

		    nAllocString += 1;
		    sAllocString += top_string - pString;
		    return pString + sizeof(char *);
		}
		else
		{
		    return str_dup( top_string + sizeof(char *) );
		}
	    }
	}
    }

    if (check > 100000) {
      bug("Fread_string check > 100,000!", 0); 
      exit(1);
    }

    return NULL;
}


void copyover_recover () {
DESCRIPTOR_DATA *d;
FILE *fp;
char name[100];
char host[MAX_STRING_LENGTH];
char buf[MAX_STRING_LENGTH];
int desc;
bool fOld;
bool crash = FALSE;
char *kwd;
char * lb = NULL;

                fp = fopen(LASTCOMMAND_FILE, "r");

                if (fp) {
                    lb = fread_string(fp);
                    fclose(fp);
	}

	fp = fopen (COPYOVER_FILE, "r");
	
	if (!fp) {
		perror ("copyover_recover:fopen");
		return;
	}

	unlink (COPYOVER_FILE);
	
	for (;;){
                               
                                kwd = fread_word(fp);

                                if (kwd[0] == 'D') {

    		    fscanf (fp, "%d %s %s\n", &desc, name, host);
		    if (desc == -1) break;

                                    if (crash) sprintf(buf, "\r\nEmergency RESTORE...\r\n");
                                    else sprintf(buf, "\r\nRestoring from REFRESH...\r\n");

		    if (!write_to_descriptor (desc, buf, 0)) {
			close (desc);
			continue;
		    }
		
		    d = (DESCRIPTOR_DATA*) alloc_perm (sizeof(DESCRIPTOR_DATA));
		    init_descriptor (d,desc);
		
		    d->host = str_dup (host);
		    d->next = descriptor_list;
		    descriptor_list = d;
		    d->connected = CON_COPYOVER_RECOVER;
		
		    fOld = load_char_obj (d, name);
		
		    if (!fOld) {
			write_to_descriptor (desc, "\r\nSomehow, your character was lost in the refresh. Sorry.\r\n", 0);
			close_socket (d, TRUE);			

		    }else {
                                                if (crash) sprintf(buf, "\r\nCrash recovery successful.\r\n");
                                                else sprintf(buf, "\r\nRefresh recovery complete - Have fun!\r\n");

			write_to_descriptor (desc, buf, 0);
	
			if (!d->character->in_room) d->character->in_room = get_room_index (mud.home);

			d->character->next = char_list;
			char_list = d->character;

			char_to_room (d->character, d->character->in_room);
			do_look (d->character, "");
			act ("$n materializes!", d->character, NULL, NULL, TO_ROOM);
      	                                reset_char(d->character);
                                                sprintf(buf, "@p%p_%s", d->character, d->character->name);
                                                free_string(d->character->true_name);
                                                d->character->true_name = str_dup(buf);
			d->connected = CON_PLAYING;
		    }

                                } else if (kwd[0] == 'C') {

                                    if (fread_number(fp) != 0
                                    && lb) {
                                       crash = TRUE;
                                       make_note ("Immortals", "Driver", "imp", "crash report", 3, lb);
                                    }

                                } else if (kwd[0] == 'E') {
                                    break;
                                }
	}
	
	fclose (fp);
                return;
}


void adjust_stats_by_nature(CHAR_DATA *mob) {

    if (IS_SET(mob->nature, NATURE_STRONG)) {
      mob->perm_stat[STAT_STR] += 4 * dice(2,4);
      mob->perm_stat[STAT_CON] += 4 * dice(1,4);
    }

    if (IS_SET(mob->nature, NATURE_FEEBLE)) {
      mob->perm_stat[STAT_STR] -= 4 * dice(2,4);
      mob->perm_stat[STAT_CON] -= 4 * dice(1,4);
    }

    if (IS_SET(mob->nature, NATURE_SMART)) {
      mob->perm_stat[STAT_INT] += 4 * dice(2,4);
      mob->perm_stat[STAT_WIS] += 4 * dice(1,4);
    }

    if (IS_SET(mob->nature, NATURE_DUMB)) {
      mob->perm_stat[STAT_INT] -= 4 * dice(2,4);
      mob->perm_stat[STAT_WIS] -= 4 * dice(1,4);
    }

    if (IS_SET(mob->nature, NATURE_AGILE)) {
      mob->perm_stat[STAT_DEX] += 4 * dice(2,4);
      mob->perm_stat[STAT_STR] += 4 * dice(1,4);
    }

    if (IS_SET(mob->nature, NATURE_LUMBERING)) {
      mob->perm_stat[STAT_DEX] -= 4 * dice(2,4);
      mob->perm_stat[STAT_INT] -= 4 * dice(1,4);
    }

    if (IS_SET(mob->nature, NATURE_SLY)) {
      mob->perm_stat[STAT_WIS] += 4 * dice(2,4);
      mob->perm_stat[STAT_INT] += 4 * dice(1,4);
    }

    if (IS_SET(mob->nature, NATURE_GULLIBLE)) {
      mob->perm_stat[STAT_WIS] -= 4 * dice(2,4);
      mob->perm_stat[STAT_INT] -= 4 * dice(1,4);
    }

    if (IS_SET(mob->nature, NATURE_ROBUST)) {
      mob->perm_stat[STAT_CON] += 4 * dice(2,4);
      mob->perm_stat[STAT_STR] += 4 *  dice(1,4);
    }

    if (IS_SET(mob->nature, NATURE_SICKLY)) {
      mob->perm_stat[STAT_CON] -= 4 * dice(2,4);
      mob->perm_stat[STAT_STR] -= 4 * dice(1,4);
    }

    if (IS_SET(mob->nature, NATURE_LUCKY)) {
      mob->perm_stat[STAT_LUC] += 6 * dice(2,4);
    }

    if (IS_SET(mob->nature, NATURE_UNLUCKY)) {
      mob->perm_stat[STAT_LUC] -= 6 * dice(2,4);
    }

    if (IS_SET(mob->nature, NATURE_CHARISMATIC)) {
      mob->perm_stat[STAT_LUC] += 6 * dice(2,4);
    }

    if (IS_SET(mob->nature, NATURE_DISGUSTING)) {
      mob->perm_stat[STAT_LUC] -= 6 * dice(2,4);
    }

    if (IS_SET(mob->nature, NATURE_MONSTEROUS)) {
      mob->perm_stat[STAT_STR] += 4 * dice(1,4);
      mob->perm_stat[STAT_CON] += 4 * dice(1,4);
      mob->perm_stat[STAT_DEX] += 4 * dice(1,4);
    }

    mob->perm_stat[STAT_STR] = URANGE(3, mob->perm_stat[STAT_STR], STAT_MAXIMUM);
    mob->perm_stat[STAT_INT] = URANGE(3, mob->perm_stat[STAT_INT], STAT_MAXIMUM);
    mob->perm_stat[STAT_DEX] = URANGE(3, mob->perm_stat[STAT_DEX], STAT_MAXIMUM);
    mob->perm_stat[STAT_WIS] = URANGE(3, mob->perm_stat[STAT_WIS], STAT_MAXIMUM);
    mob->perm_stat[STAT_CON] = URANGE(3, mob->perm_stat[STAT_CON], STAT_MAXIMUM);
    mob->perm_stat[STAT_LUC] = URANGE(3, mob->perm_stat[STAT_LUC], STAT_MAXIMUM);
    mob->perm_stat[STAT_CHA] = URANGE(3, mob->perm_stat[STAT_CHA], STAT_MAXIMUM);

    return;
}


void save_passport(void) {
FILE *fp;
PASSPORT *pass;

    fp = fopen(PASS_FILE, "w");
    if (!fp) {
        log_string("Can't write passports!");
        return;
    }

    for (pass = passport_list; pass; pass = pass->next )    {
          fprintf( fp, "%s %d %d %d %d\n", pass->name, pass->type, pass->day, pass->month, pass->year);
    }
    fprintf( fp, "End\n");

    fclose(fp);
    return;
}
   

void load_passport(void) {
FILE *fp;
PASSPORT *pass;
char *word;

    fp = fopen(PASS_FILE, "r");
    if (!fp) return;

    for ( ; ; ) {
         word   = fread_word( fp );

         if (!str_cmp(word, "End")) break;
         pass = new_pass_order();
         pass->name	= str_dup(word);
         pass->type 	= fread_number(fp);
         pass->day 	= fread_number(fp);
         pass->month= fread_number(fp);
         pass->year 	= fread_number(fp);
    }         

    log_string("... Passports loaded.");
    fclose(fp);
    return;
}



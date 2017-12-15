/*
 * CthulhuMud
 */

extern PROF_DATA *get_profession(char *prof_name);

extern int get_prof_id(char *prof_name);

extern PROF_DATA *get_prof_by_id(int id);

extern void load_professions();

extern void check_prof_starts();

extern void default_mob(MOB_INDEX_DATA *mob, PROF_DATA *prof);

extern void default_char(CHAR_DATA *ch, PROF_DATA *prof);

extern void do_prof_list(CHAR_DATA *ch);

extern void do_prof_detail(CHAR_DATA *ch, char *prof_name);

extern LPROF_DATA *get_lprof();

extern void free_lprof(LPROF_DATA *lprof);

extern bool skill_available_by_prof(int sn, CHAR_DATA *ch); 

extern void gain_prof_level(CHAR_DATA *ch);

extern void lose_prof_level(CHAR_DATA *ch);

extern void equip_char_by_prof(CHAR_DATA *prof, bool startup);

extern void prompt_profession(DESCRIPTOR_DATA *d);

extern void prompt_profession_info(DESCRIPTOR_DATA *d, char *args);

extern void do_change_prof(CHAR_DATA *ch, char *prof_name);

#define PROF_MAGE		"dreamlands mage"
#define PROF_GOOD_PRIEST	"dreamlands good priest"
#define PROF_THIEF		"dreamlands thief"
#define PROF_WARRIOR		"dreamlands warrior"
#define PROF_CHAOSMAGE	"dreamlands chaos mage"
#define PROF_MONK		"dreamlands monk"
#define PROF_EVIL_PRIEST	"dreamlands evil priest"


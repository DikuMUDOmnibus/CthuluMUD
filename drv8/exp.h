/*
 * CthulhuMud
 */

/* Level advancement... */
 
extern void advance_level( CHAR_DATA *ch );

/* Going down... */

extern void drop_level( CHAR_DATA *ch );

/* Give an individual some xps... */

extern void gain_exp( CHAR_DATA *ch, long gain, bool advance );

/* Give a group some xps... */

extern void group_gain( CHAR_DATA *ch, CHAR_DATA *victim );

/* Give a group a reward... */

extern void group_reward( CHAR_DATA *ch, int xps );

/* Death causes exp loss... */

extern void lose_exp_for_dieing(CHAR_DATA *ch);

/* Build the exp table... */

extern void build_exp_table();

/* Number of xps needed for level x to level x+1... */

extern int exp_to_advance(int level);

/* Number of xps needed for level x... */

extern int exp_for_level(int level);

/* Xps needed for a character to get to their next level... */

extern int exp_for_next_level(CHAR_DATA *ch);

/* Compute xp for a kill... */
 
extern int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels );

/* Work out a Mobs effective level... */

extern int get_effective_level(CHAR_DATA *ch);

/* See if a character goes insane... */

bool insanity(MOB_CMD_CONTEXT *context, int threat, char *action);

bool check_sanity(CHAR_DATA *ch, int threat);


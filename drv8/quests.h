/*
 * CthulhuMud
 */

/* Quest types... */

#define QUEST_NONE		  -1
#define QUEST_STARTED		   0
#define QUEST_COMPLETED		9999

/* Quest management... */

QUEST *get_quest();
  
void free_quest(QUEST *old_quest);

/* Save and restore from file... */

void save_quest(FILE *fp, QUEST *quest);

QUEST *read_quest(FILE *fp);

/* Addition and removal... */

void add_quest(CHAR_DATA *ch, int id, char *title);

void update_quest(CHAR_DATA *ch, int id, int state, char *desc);

int quest_state(CHAR_DATA *ch, int id);

void remove_quest(CHAR_DATA *ch, int id);


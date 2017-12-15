/*
 * CthulhuMud
 */

/* Adjust a destination for day/night... */

ROOM_INDEX_DATA *day_night(ROOM_INDEX_DATA *room);

/* Work out where an exit goes... */

ROOM_INDEX_DATA *get_exit_destination(CHAR_DATA *ch, ROOM_INDEX_DATA *room, EXIT_DATA *pexit, bool recurse);

/* See if an exit can be seen... */

bool exit_visible(CHAR_DATA *ch, EXIT_DATA *pexit);

/* Set the bits on a door and ALL reverse doors... */

void set_door_flags(ROOM_INDEX_DATA *room, int door, EXIT_DATA *pexit, int bit_flag, bool set);

/* See if a mob can flee in a given direction... */

ROOM_INDEX_DATA *can_flee(CHAR_DATA *ch, int door); 

/* CDD management... */

COND_DEST_DATA *get_cdd();

void free_cdd(COND_DEST_DATA *cdd);

COND_DEST_DATA *read_cdd(FILE *fp);

void insert_cdd(COND_DEST_DATA *cdd, EXIT_DATA *pexit);

void write_cdd(COND_DEST_DATA *cdd, FILE *fp, char *hdr1, char *hdr2);

/* Turn a direction or gate into a room exit index... */

int door_index(CHAR_DATA *ch, char *name);
 
/* Turn a direction into an exit index... */

int door_index2(CHAR_DATA *ch, char *name);
 

/*
 * CthulhuMud
 */

/* Tracks left by mobiles... */

void free_mob_tracks(MOB_TRACKS *tracks);

MOB_TRACKS *get_mob_tracks();

void reduce_usage(MOB_TRACKS *tracks);

void increase_usage(MOB_TRACKS *tracks);

MOB_TRACKS *find_mob_tracks(CHAR_DATA *ch);

/* Tracks in a room... */

void free_tracks(TRACKS *tracks);

TRACKS *get_tracks();

void add_tracks(CHAR_DATA *ch, int from, int to);

void age_tracks(ROOM_INDEX_DATA *room);

void save_tracks(FILE *fp, TRACKS *tracks, char *hdr);

TRACKS *read_tracks(FILE *fp);

bool redit_ptracks(CHAR_DATA *ch, char *args);

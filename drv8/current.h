/*
 * CthulhuMud
 */

CURRENT *get_current();

void free_current(CURRENT *old_current);

CURRENT *create_current(int 	 seq,
			char 	*name, 
			char 	*action, 
			char	*destination,
			int 	 dir);

void apply_current(ROOM_INDEX_DATA *room, CHAR_DATA *ch);

void do_currents(CHAR_DATA *ch, char *args);

CURRENT *insert_current(CURRENT *chain, CURRENT *new_current);

CURRENT *delete_current(CURRENT *chain, int seq);

void save_current(CURRENT *cur, FILE *fp, char *hdr, char *hdr2);
 
CURRENT *read_current(FILE *fp);
 

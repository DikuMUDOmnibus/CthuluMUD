/*
 * CthulhuMud
 */

CHAT_DATA *get_chat();

void free_chat(CHAT_DATA *old);

CHAT_DATA *read_chat(FILE *fp);

void write_chat(FILE *fp, CHAT_DATA *chat, char *hdr);

CHAT_DATA *find_chat(CHAT_DATA *chat, int state);

bool do_chat(CHAR_DATA *ch, char *args);


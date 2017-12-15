/*
 * CthulhuMud
 */

/* Deed types... */

#define DEED_SECRET	(A)	// 1
#define DEED_PUBLIC     (B)	// 2
#define DEED_GOOD	(C)	// 4
#define DEED_BAD	(D)	// 8

#define DEED_MAX	(E)     // 16  

/* Deed management... */

DEED *get_deed();
  
void free_deed(DEED *old_deed);

/* See if a character has done a deed... */

bool doneDeed(CHAR_DATA *ch, int id);

/* Save and restore from file... */

void save_deed(FILE *fp, DEED *deed);

void save_extra_deed(FILE *fp, DEED *deed, char *hdr);

DEED *read_deed(FILE *fp);

/* Addition and removal... */

void add_deed(CHAR_DATA *ch, int id, int type, char *title);

void remove_deed(CHAR_DATA *ch, int id);


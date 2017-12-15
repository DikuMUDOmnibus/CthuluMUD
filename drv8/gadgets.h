/*
 * CthulhuMud
 */

/* Gadget actions... */

#define GADGET_ACTION_NONE 	 0

#define GADGET_ACTION_PULL	 1
#define GADGET_ACTION_PUSH	 2
#define GADGET_ACTION_TWIST	 3
#define GADGET_ACTION_TURN	 4
#define GADGET_ACTION_TURNBACK	 5
#define GADGET_ACTION_MOVE	 6
#define GADGET_ACTION_LIFT	 7
#define GADGET_ACTION_PRESS	 8
#define GADGET_ACTION_DIGIN	 9

#define GADGET_ACTION_MAX	 10

/* Gadget transform data management... */

GADGET_TRANS_DATA *get_gdt();

void free_gdt(GADGET_TRANS_DATA *gdt);

void delete_gdt(GADGET_DATA *gadget, int seq);

void insert_gdt(GADGET_TRANS_DATA *new_gdt, GADGET_DATA *gadget);

GADGET_TRANS_DATA *find_gdt_by_seq(GADGET_DATA *gadget, int seq);

/* Gadget data management... */
 
GADGET_DATA *get_gadget();

void free_gadget(GADGET_DATA *gadget);

GADGET_DATA *find_gadget_by_name(ROOM_INDEX_DATA *room, char *name);

GADGET_DATA *find_gadget_by_id(ROOM_INDEX_DATA *room, int id);

/* File I/O */

GADGET_DATA *read_gadget(FILE *FP);

GADGET_TRANS_DATA *read_gdt(FILE *fp);

void write_gadget(FILE *fp, GADGET_DATA *gadget);


/*
 * CthulhuMud
 */

#define ANCHOR_TYPE_ROOM	   1
#define ANCHOR_TYPE_MOB           2
#define ANCHOR_TYPE_OBJ              3

ANCHOR_LIST *get_anchor_list();

void free_anchor_list(ANCHOR_LIST *old_list);

/* Start a mobs monitoring... */

void start_anchors(OBJ_DATA *obj);

/* Stop a mobs monitoring... */

void stop_anchors(OBJ_DATA *obj);

/* Save a monitor list... */

void write_anchors(ANCHOR_LIST *list, FILE *fp, char *hdr);

/* Read a monitor... */

ANCHOR_LIST *read_anchor(FILE *fp);

/* Insert a monitor list... */

ANCHOR_LIST *insert_anchor(ANCHOR_LIST *chain, ANCHOR_LIST *new_anchor);

/* Delete a monitor list */

ANCHOR_LIST *delete_anchor(ANCHOR_LIST *chain, int id);


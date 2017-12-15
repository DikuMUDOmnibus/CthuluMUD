/*********************************************************
 *                   CTHULHUMUD                       	 *
 * CthulhuMud  driver  version 8.x copyright (C) 2000 	 *
 * by Mik Clarke (mykael@vianet.net.au)                  *
 * and Joachim Häusler (mystery@chello.at).              *
 *                                                       *
 * While the code is original, many concepts and the     *
 * typical look & feel are derived from MERC and its     *
 * derivatives - especially the SunderMud 1.0 prototype. *
 *                                                       *
 * Therefore we'd like to thank:                         *
 * Lotherius                                             *
 * Russ Taylor, Gabrielle Taylor, Brian Moore            *
 * Michael Chastain, Michael Quan, and Mitchell Tse      *
 *                                                       *
 * Please keep this code open-source and share your      *
 * ideas. It's still a long way to go.                   *
 *********************************************************/


#include "everything.h"
#include "anchor.h"
#include "olc.h"
#include "wev.h"

extern struct ev_mapping ev_map[];

/* Memory management for monitor list chains... */

static ANCHOR_LIST *free_anchor_list_chain = NULL;
ANCHOR_LIST *get_anchor_list() {
ANCHOR_LIST *new_list;

  if (free_anchor_list_chain == NULL) {
        new_list = (ANCHOR_LIST *) alloc_perm(sizeof(*new_list));
  } else {
        new_list = free_anchor_list_chain;
        free_anchor_list_chain = new_list->next;
  }

  new_list->next = NULL;
  new_list->anchor_type = 0;
  new_list->anchor_id = 0;
  new_list->restrict = 0;
  return new_list;
}

void free_anchor_list(ANCHOR_LIST *old_list) {

  if (old_list->next != NULL) {
        free_anchor_list(old_list->next);
        old_list->next = NULL;
  }

  old_list->next = free_anchor_list_chain;
  free_anchor_list_chain = old_list;
  return;
}

/* Save an anchor list... */

void write_anchors(ANCHOR_LIST *list, FILE *fp, char *hdr) {
  if (list->next != NULL) write_anchors(list->next, fp, hdr);
  
  fprintf(fp, "%s %d %d %ld %d\n", hdr, list->anchor_type, list->anchor_id, list->vnum, list->restrict);
  return;
}

/* Read an anchor... */

ANCHOR_LIST *read_anchor(FILE *fp) {
  ANCHOR_LIST *new_anchor;

  new_anchor = get_anchor_list();
  new_anchor->anchor_type = fread_number(fp);
  new_anchor->anchor_id = fread_number(fp);
  new_anchor->vnum = fread_number(fp);
  new_anchor->restrict = fread_number(fp);
  new_anchor->next = NULL;
  return new_anchor;
}

/* Insert a monitor list... */

ANCHOR_LIST *insert_anchor(ANCHOR_LIST *chain, ANCHOR_LIST *new_anchor) {
  ANCHOR_LIST *prev, *mon;

  if (chain == NULL) {
        return new_anchor;
  }

  if (new_anchor->anchor_id < chain->anchor_id) {
      new_anchor->next = chain;
      return new_anchor;
  }

  prev = chain;  
  mon = chain->next;

  while ( mon != NULL
  && mon->anchor_id < new_anchor->anchor_id) { 
      prev = mon;
      mon = mon->next;
  }

  if (prev->anchor_id != new_anchor->anchor_id) {
      prev->next = new_anchor;
      new_anchor->next = mon;
  } else {
      prev->anchor_type = new_anchor->anchor_type;
      prev->vnum = new_anchor->vnum;
      free_anchor_list(new_anchor);
  }
 
  return chain;
}


/* Delete a monitor list */

ANCHOR_LIST *delete_anchor(ANCHOR_LIST *chain, int id) {
  ANCHOR_LIST *prev, *mon;

  if (chain == NULL) return NULL;
 
  if (id == chain->anchor_id) {
      mon = chain;
      chain = chain->next;
      mon->next = NULL;
      free_anchor_list(mon);
      return chain;
  }

  prev = chain;  
  mon = chain->next;

  while ( mon != NULL
  && mon->anchor_id != id) { 
      prev = mon;
      mon = mon->next;
  }

  if (mon != NULL) {
      prev->next = mon->next;
      mon->next = NULL;
      free_anchor_list(mon);
  }
 
   return chain;
}


/* Edit monitor... */

void oedit_anchor_list(CHAR_DATA *ch, char *args, OBJ_INDEX_DATA *pobj) {
  ANCHOR_LIST *anchor;
  ROOM_INDEX_DATA *room = NULL;
  MOB_INDEX_DATA *mob = NULL;
  OBJ_INDEX_DATA *obj = NULL;
  char *title;
  char *anch_type;
  char buf[MAX_STRING_LENGTH];

 /* Give help... */

  if (args[0] != '\0') {
      oedit_anchor(ch, "");
      return;
  }

 /* Find monitors... */

  anchor = pobj->anchors;

 /* No monitors? */

  if (anchor == NULL) {
       send_to_char("No anchors defined.\r\n", ch);
       return;
  }

 /* Output the header... */
 
  send_to_char("Id     Type   Direct to...\r\n"
                            "-----  -----  --------------------------------------\r\n", ch); 

 /* Output the monitors... */

  while (anchor != NULL) {
    
    switch (anchor->anchor_type) {
        default:
              anch_type = strdup("None!");
              title = "{RInvalid!{x";
              break;

        case ANCHOR_TYPE_ROOM:
              anch_type = strdup("Room");
              room = get_room_index(anchor->vnum);
              if (room == NULL) {
                     title = "{RInvalid!{x";
              } else {
                     title = room->name;
              }
              break;

        case ANCHOR_TYPE_MOB:
              anch_type = strdup("Mob");
              mob = get_mob_index(anchor->vnum);
              if (mob == NULL) {
                     title = "{RInvalid!{x";
              } else {
                     title = mob->short_descr;
              }
              break;

        case ANCHOR_TYPE_OBJ:
              anch_type = strdup("Obj");
              obj = get_obj_index(anchor->vnum);
              if (obj == NULL) {
                     title = "{RInvalid!{x";
              } else {
                     title = obj->short_descr;
              }
              break;
    }

    sprintf(buf, "%5d  %5s  %5ld - %s (%s)\r\n",
                  anchor->anchor_id,
                  anch_type, 
                  anchor->vnum,
                  title,
                  ev_map[anchor->restrict].name);
    send_to_char(buf, ch);
    anchor = anchor->next;
  } 

 /* All done... */

  return;
}

/* Add monitor... */

ANCHOR_LIST *oedit_anchor_add(CHAR_DATA *ch, char *args, OBJ_INDEX_DATA *pobj) {
  ANCHOR_LIST *anchor;
  char s_id[MAX_INPUT_LENGTH];
  char s_type[MAX_INPUT_LENGTH];
  char s_vnum[MAX_INPUT_LENGTH];
  char s_restrict[MAX_INPUT_LENGTH];
  int id, type, vnum, count;

 /* Give help... */

  if (args[0] == '\0') {
      oedit_anchor(ch, "");
      return pobj->anchors;
  }

 /* Validate parms... */

  args = one_argument(args, s_id);
  args = one_argument(args, s_type);
  args = one_argument(args, s_vnum);
  args = one_argument(args, s_restrict);

 /* Give help... */

  if (args[0] != '\0') {
      oedit_anchor(ch, "");
      return pobj->anchors;
  }

 /* Convert and revalidate... */

  id = atoi(s_id);

  if (id < 1) {
      send_to_char("Id must be > 0\r\n", ch);
      return pobj->anchors;
  }

   if (!str_cmp(s_type, "room")) {
         type = ANCHOR_TYPE_ROOM;
   } else if (!str_cmp(s_type, "mob")) {
         type = ANCHOR_TYPE_MOB;
   } else if (!str_cmp(s_type, "obj")) {
         type = ANCHOR_TYPE_OBJ;
   } else {
        send_to_char("Invalid type.\r\n", ch);
        return pobj->anchors;
  }

  vnum = atoi(s_vnum);

  if (vnum < 1) {
      send_to_char("Vnum must be > 0\r\n", ch);
      return pobj->anchors;
  }

  for (count = 2; count < MAX_WEV; count++) {
        if (!str_cmp(ev_map[count].name, s_restrict)) break;
  } 

  if (count == MAX_WEV) count =0;

 /* Get the new monitor_list... */

  anchor = get_anchor_list();
 
  if (anchor == NULL) {
      send_to_char("Allocate of anchor list failed!/n/r", ch); 
      return pobj->anchors;
  }

 /* Set up the new monitor list... */

  anchor->anchor_id = id;
  anchor->anchor_type = type;
  anchor->vnum = vnum;
  anchor->restrict = count;

 /* Tell them it's ok... */

  send_to_char("Anchor added.\r\n", ch);

 /* Return modified list... */

  return insert_anchor(pobj->anchors, anchor);
} 


/* Delete an anchor... */

ANCHOR_LIST *oedit_anchor_delete(CHAR_DATA *ch, char *args, OBJ_INDEX_DATA *pobj) {
  char s_id[MAX_INPUT_LENGTH];
  int id;

 /* Give help... */

  if (args[0] == '\0') {
      oedit_anchor(ch, "");
      return pobj->anchors;
  }

 /* Validate parms... */

  args = one_argument(args, s_id);

 /* Give help... */

  if (args[0] != '\0') {
      oedit_anchor(ch, "");
      return pobj->anchors;
  }

 /* Convert and revalidate... */

  id = atoi(s_id);

  if (id < 1) {
      send_to_char("Id must be > 0\r\n", ch);
      return pobj->anchors;
  }

 /* Tell the char we've done it... */

  send_to_char("Anchor deleted.\r\n", ch);

 /* Return truncated list... */

  return delete_anchor(pobj->anchors, id);
} 


bool oedit_anchor(CHAR_DATA *ch, char *args) {
  OBJ_INDEX_DATA *pobj;
  char command[MAX_INPUT_LENGTH];
  char *parms;

 /* What are we editing... */

  EDIT_OBJ(ch, pobj);

 /* Give help if needed... */ 

  if ( args[0] == '\0'
  || args[0] == '?' ) {

    send_to_char("Syntax: anchor list\r\n", ch);
    send_to_char("        anchor add seq type vnum restriction\r\n", ch);
    send_to_char("        anchor delete seq\r\n", ch);
    return FALSE;
  }   

 /* Extract command and parms... */

  parms = one_argument(args, command);

 /* List monitors... */

  if (!str_cmp(command, "list")) { 
    oedit_anchor_list(ch, parms, pobj); 
    return FALSE;
  } 

 /* Add/replace a monitor... */

  if (!str_cmp(command, "add")) { 
      pobj->anchors = oedit_anchor_add(ch, parms, pobj); 
      return TRUE;
  } 

 /* delete a monitor... */

  if (!str_cmp(command, "delete")) { 
      pobj->anchors = oedit_anchor_delete(ch, parms, pobj); 
      return TRUE;
  } 

  oedit_anchor(ch, "");

  return FALSE;
}

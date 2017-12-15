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
#include "monitor.h"
#include "olc.h"
#include "wev.h"

/* Memory management for monitor list chains... */

static MONITOR_LIST *free_monitor_list_chain = NULL;

MONITOR_LIST *get_monitor_list() {

  MONITOR_LIST *new_list;

  if (free_monitor_list_chain == NULL) {
    new_list = (MONITOR_LIST *) alloc_perm(sizeof(*new_list));
  } else {
    new_list = free_monitor_list_chain;
    free_monitor_list_chain = new_list->next;
  }

  new_list->next = NULL;

  new_list->monitor_type = 0;
  new_list->monitor_id = 0;

  return new_list;
}

void free_monitor_list(MONITOR_LIST *old_list) {

  if (old_list->next != NULL) {
    free_monitor_list(old_list->next);
    old_list->next = NULL;
  }

  old_list->next = free_monitor_list_chain;
  free_monitor_list_chain = old_list;

  return;
}

/* Memory management for monitoring list chains... */

static MONITORING_LIST *free_monitoring_list_chain = NULL;

MONITORING_LIST *get_monitoring_list() {

  MONITORING_LIST *new_list;

  if (free_monitoring_list_chain == NULL) {
    new_list = (MONITORING_LIST *) alloc_perm(sizeof(*new_list));
  } else {
    new_list = free_monitoring_list_chain;
    free_monitoring_list_chain = new_list->next;
  }

  new_list->next = NULL;

  new_list->monitor = NULL;

  return new_list;
}

void free_monitoring_list(MONITORING_LIST *old_list) {

log_string("Freeing list");
  if (old_list->next != NULL) {
    free_monitoring_list(old_list->next);
    old_list->next = NULL;
  }

  old_list->monitor = NULL;

  old_list->next = free_monitoring_list_chain;
  free_monitoring_list_chain = old_list;
log_string("List freed");
  return;
}


/* Start a mobs monitoring... */

void start_monitors(CHAR_DATA *mob) {

  MONITOR_LIST *monitor;

  MONITORING_LIST *hook;

  ROOM_INDEX_DATA *target;

 /* Check we got something to do... */

  if ( mob == NULL
    || mob->pIndexData == NULL  
    || mob->pIndexData->monitors == NULL ) {

    return;
  }

 /* Process one at a time... */

  monitor = mob->pIndexData->monitors;

  while (monitor != NULL) {

   /* Find the room... */ 

    target = get_room_index(monitor->vnum);

   /* Add the hook if we found the room... */

    if ( target != NULL ) {
    
      hook = get_monitoring_list();

      hook->monitor = mob;
      hook->type = monitor->monitor_type;

      hook->next = target->monitors;
      target->monitors = hook;
    }

    monitor = monitor->next;
  } 

 /* All done... */

  return;
}

 
/* Stop a mobs monitoring... */

void stop_monitors(CHAR_DATA *mob) {

  MONITOR_LIST *monitor;

  MONITORING_LIST *hook, *old_hook;

  ROOM_INDEX_DATA *target;

 /* Check we got something to do... */

  if ( mob == NULL
    || mob->pIndexData == NULL  
    || mob->pIndexData->monitors == NULL ) {

    return;
  }

 /* Process one at a time... */

  monitor = mob->pIndexData->monitors;

  while (monitor != NULL) {

   /* Find the room... */ 

    target = get_room_index(monitor->vnum);

   /* Remove the hook if we found the room... */

    if ( target != NULL ) {
     
     /* Find the hook... */

      hook = target->monitors;
      old_hook = NULL;

      while ( hook != NULL ) {
        
        if ( hook->type == monitor->monitor_type
          && hook->monitor == mob ) {  

          if (old_hook != NULL) {
            old_hook->next = hook->next;
          } else {
            target->monitors = hook->next;
          }
 
          hook->next = NULL;

          free_monitoring_list(hook);

          hook = NULL;

        } else {
          old_hook = hook;
          hook = hook->next; 
        }
      }
    }

    monitor = monitor->next;
  } 

 /* All done... */

  return;
} 

/* Save a monitor list... */

void write_monitors(MONITOR_LIST *list, FILE *fp, char *hdr) {

  if (list->next != NULL) {
    write_monitors(list->next, fp, hdr);
  }

  fprintf(fp, "%s %d %d %ld\n",
               hdr,
               list->monitor_type,
               list->monitor_id,
               list->vnum);

  return;
}

/* Read a monitor... */

MONITOR_LIST *read_monitor(FILE *fp) {

  MONITOR_LIST *new_monitor;

  new_monitor = get_monitor_list();

  new_monitor->monitor_type = fread_number(fp);
  new_monitor->monitor_id = fread_number(fp);
  new_monitor->vnum = fread_number(fp);

  new_monitor->next = NULL;

  return new_monitor;
}

/* Insert a monitor list... */

MONITOR_LIST *insert_monitor(MONITOR_LIST *chain, MONITOR_LIST *new_monitor) {

  MONITOR_LIST *prev, *mon;

  if (chain == NULL) {
    return new_monitor;
  }

  if (new_monitor->monitor_id < chain->monitor_id) {
    new_monitor->next = chain;
    return new_monitor;
  }

  prev = chain;  
  mon = chain->next;

  while ( mon != NULL
       && mon->monitor_id < new_monitor->monitor_id) { 

    prev = mon;
    mon = mon->next;
  }

  if (prev->monitor_id != new_monitor->monitor_id) {
    prev->next = new_monitor;
    new_monitor->next = mon;
  } else {
    prev->monitor_type = new_monitor->monitor_type;
    prev->vnum = new_monitor->vnum;
    free_monitor_list(new_monitor);
  }
 
  return chain;
}

/* Delete a monitor list */

MONITOR_LIST *delete_monitor(MONITOR_LIST *chain, int id) {

  MONITOR_LIST *prev, *mon;

  if (chain == NULL) {
    return NULL;
  }

  if (id == chain->monitor_id) {
    mon = chain;
    chain = chain->next;
    mon->next = NULL;
    free_monitor_list(mon);
    return chain;
  }

  prev = chain;  
  mon = chain->next;

  while ( mon != NULL
       && mon->monitor_id != id) { 

    prev = mon;
    mon = mon->next;
  }

  if (mon != NULL) {
    prev->next = mon->next;
    mon->next = NULL;
    free_monitor_list(mon);
  }
 
  return chain;
}

/* Edit monitor... */

void medit_monitor_list(CHAR_DATA *ch, char *args, MOB_INDEX_DATA *pmob) {

  MONITOR_LIST *monitor;
  ROOM_INDEX_DATA *room;
  char *title;
  char buf[MAX_STRING_LENGTH];

 /* Give help... */

  if (args[0] != '\0') {
    medit_monitor(ch, "");
    return;
  }

 /* Find monitors... */

  monitor = pmob->monitors;

 /* No monitors? */

  if (monitor == NULL) {
    send_to_char("No monitors defined.\r\n", ch);
    return;
  }

 /* Output the header... */
 
  send_to_char("Id     Type   Room\r\n"
               "-----  -----  --------------------------------------\r\n", ch); 

 /* Output the monitors... */

  while (monitor != NULL) {

    room = get_room_index(monitor->vnum);

    if (room == NULL) {
      title = "{RInvalid!{x";
    } else {
      title = room->name;
    }

    sprintf(buf, "%5d  %5d  %5ld - %s\r\n",
                  monitor->monitor_id,
                  monitor->monitor_type,
                  monitor->vnum,
                  title);

    send_to_char(buf, ch);

    monitor = monitor->next;
  } 

 /* All done... */

  return;
}

/* Add monitor... */

MONITOR_LIST *medit_monitor_add(CHAR_DATA *ch, 
                                char *args,
                                MOB_INDEX_DATA *pmob) {

  MONITOR_LIST *monitor;
  
  char s_id[MAX_INPUT_LENGTH];
  char s_type[MAX_INPUT_LENGTH];
  char s_vnum[MAX_INPUT_LENGTH];

  int id, type, vnum;

 /* Give help... */

  if (args[0] == '\0') {
    medit_monitor(ch, "");
    return pmob->monitors;
  }

 /* Validate parms... */

  args = one_argument(args, s_id);
  args = one_argument(args, s_type);
  args = one_argument(args, s_vnum);

 /* Give help... */

  if (args[0] != '\0') {
    medit_monitor(ch, "");
    return pmob->monitors;
  }

 /* Convert and revalidate... */

  id = atoi(s_id);

  if (id < 1) {
    send_to_char("Id must be > 0\r\n", ch);
    return pmob->monitors;
  }

  type = atoi(s_type);

  if ( type < WEV_TYPE_NONE
    || type > MAX_WEV) {
    send_to_char("Invalid type.\r\n", ch);
    return pmob->monitors;
  }

  vnum = atoi(s_vnum);

  if (vnum < 1) {
    send_to_char("Vnum must be > 0\r\n", ch);
    return pmob->monitors;
  }

 /* Get the new monitor_list... */

  monitor = get_monitor_list();
 
  if (monitor == NULL) {
    send_to_char("Allocate of monitor list failed!/n/r", ch); 
    return pmob->monitors;
  }

 /* Set up the new monitor list... */

  monitor->monitor_id = id;
  monitor->monitor_type = type;
  monitor->vnum = vnum;

 /* Tell them it's ok... */

  send_to_char("Monitor added.\r\n", ch);

 /* Return modified list... */

  return insert_monitor(pmob->monitors, monitor);
} 

/* Delete a monitor... */

MONITOR_LIST *medit_monitor_delete(CHAR_DATA *ch, 
                                   char *args,
                                   MOB_INDEX_DATA *pmob) {

  char s_id[MAX_INPUT_LENGTH];

  int id;

 /* Give help... */

  if (args[0] == '\0') {
    medit_monitor(ch, "");
    return pmob->monitors;
  }

 /* Validate parms... */

  args = one_argument(args, s_id);

 /* Give help... */

  if (args[0] != '\0') {
    medit_monitor(ch, "");
    return pmob->monitors;
  }

 /* Convert and revalidate... */

  id = atoi(s_id);

  if (id < 1) {
    send_to_char("Id must be > 0\r\n", ch);
    return pmob->monitors;
  }

 /* Tell the char we've done it... */

  send_to_char("Monitor deleted.\r\n", ch);

 /* Return truncated list... */

  return delete_monitor(pmob->monitors, id);
} 

bool medit_monitor(CHAR_DATA *ch, char *args) {

  MOB_INDEX_DATA *pMob;

  char command[MAX_INPUT_LENGTH];
  char *parms;

 /* What are we editing... */

  EDIT_MOB(ch, pMob);

 /* Give help if needed... */ 

  if ( args[0] == '\0'
    || args[0] == '?' ) {

    send_to_char("Syntax: monitor list\r\n", ch);
    send_to_char("        monitor add seq type vnum\r\n", ch);
    send_to_char("        monitor delete seq\r\n", ch);

    return FALSE;
  }   

 /* Extract command and parms... */

  parms = one_argument(args, command);

 /* List monitors... */

  if (!str_cmp(command, "list")) { 
    medit_monitor_list(ch, parms, pMob); 
    return FALSE;
  } 

 /* Add/replace a monitor... */

  if (!str_cmp(command, "add")) { 
    pMob->monitors = medit_monitor_add(ch, parms, pMob); 
    return TRUE;
  } 

 /* delete a monitor... */

  if (!str_cmp(command, "delete")) { 
    pMob->monitors = medit_monitor_delete(ch, parms, pMob); 
    return TRUE;
  } 

  medit_monitor(ch, "");

  return FALSE;
}

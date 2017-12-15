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
#include "wev.h"
#include "rooms.h"

/* Collect an individual room... */

ROOM_INDEX_DATA inline *collect_room(	ROOM_INDEX_DATA *old, 
					ROOM_INDEX_DATA *new_room) {

  if (new_room == NULL) {
    return old;
  }

  if (new_room->next_collected == NULL) {
    old->next_collected = new_room;
    return new_room;
  }

  return old;
}

/* Collect all rooms potentially adjacent to this one... */

ROOM_INDEX_DATA *collect_adjacent(ROOM_INDEX_DATA *room) {

  ROOM_INDEX_DATA *cur_room, *chk_room, *end_room;

  COND_DEST_DATA *cdd;

  int i;

  cur_room = room;

  while (cur_room->next_collected != NULL) {
    cur_room = cur_room->next_collected;
  }

  end_room = cur_room;

  chk_room = room;

  while ( chk_room != NULL ) {

   /* Check this room... */

    for (i = 0; i < DIR_MAX; i++) {

      if (chk_room->exit[i] != NULL) {

        cur_room = collect_room(cur_room, chk_room->exit[i]->u1.to_room);
       
        cdd = chk_room->exit[i]->cond_dest; 

        while (cdd != NULL) {

          cur_room = collect_room(cur_room, cdd->dest);
       
          cdd = cdd->next;
        }
      }
    }

   /* Find next room to check... */
 
    if (chk_room == end_room) {
      chk_room = NULL;
    } else {
      chk_room = chk_room->next_collected;
    }
  }

  return room;
}

/* Collect all rooms in the same area->subarea... */

ROOM_INDEX_DATA *collect_subarea(ROOM_INDEX_DATA *room) {

  int subarea;

  int i;

  ROOM_INDEX_DATA *cur_room, *chk_room;

  AREA_DATA *tgt_area;

  subarea = room->subarea;

  cur_room = room;

  if (room->area == NULL) {
    return room;
  }

  tgt_area = room->area;

  for (i = 0; i < MAX_KEY_HASH; i++) {
    chk_room = room_index_hash[i];

    while (chk_room != NULL) {
  
      if ( chk_room->area == tgt_area
        && chk_room != room
        && chk_room->subarea == subarea  ) {
        cur_room = collect_room(cur_room, chk_room);
      }    

      chk_room = chk_room->next;
    }
  }

  return room;
}

/* Collect all rooms in and adjacent to the same area->subarea... */

ROOM_INDEX_DATA *collect_subarea_plus(ROOM_INDEX_DATA *room) {

  return collect_adjacent(collect_subarea(room));
}

/* Collect all rooms in an area... */

ROOM_INDEX_DATA *collect_area(ROOM_INDEX_DATA *room) {

  int i;

  ROOM_INDEX_DATA *cur_room, *chk_room;

  AREA_DATA *tgt_area;

  cur_room = room;

  if (room->area == NULL) {
    return room;
  }

  tgt_area = room->area;

  for (i = 0; i < MAX_KEY_HASH; i++) {
    chk_room = room_index_hash[i];

    while (chk_room != NULL) {
  
      if ( chk_room->area == tgt_area
        && chk_room != room ) {
        cur_room = collect_room(cur_room, chk_room);
      }    

      chk_room = chk_room->next;
    }
  }

  return room;
}

/* Collect all rooms in and adjacent to the area... */

ROOM_INDEX_DATA *collect_area_plus(ROOM_INDEX_DATA *room) {

  return collect_adjacent(collect_area(room));
}

/* Collect all rooms in a zone... */

ROOM_INDEX_DATA *collect_zone(ROOM_INDEX_DATA *room) {

  int i;

  ROOM_INDEX_DATA *cur_room, *chk_room;

  int tgt_zone;

  cur_room = room;

  if (room->area == NULL) {
    return room;
  }

  tgt_zone = room->area->zone;

  for (i = 0; i < MAX_KEY_HASH; i++) {
    chk_room = room_index_hash[i];

    while (chk_room != NULL) {
  
      if ( chk_room->area != NULL
        && chk_room != room  
        && chk_room->area->zone == tgt_zone) {
        cur_room = collect_room(cur_room, chk_room);
      }    

      chk_room = chk_room->next;
    }
  }

  return room;
}

/* Collect all rooms... */

ROOM_INDEX_DATA *collect_all_rooms(ROOM_INDEX_DATA *room) {

  int i;

  ROOM_INDEX_DATA *cur_room, *chk_room;

  cur_room = room;

  for (i = 0; i < MAX_KEY_HASH; i++) {
    chk_room = room_index_hash[i];

    while (chk_room != NULL) {
  
      cur_room = collect_room(cur_room, chk_room);

      chk_room = chk_room->next;
    }
  }

  return room;
}

ROOM_INDEX_DATA *collect_group_rooms(ROOM_INDEX_DATA *room, CHAR_DATA *ch) {

  CHAR_DATA *gch;

  ROOM_INDEX_DATA *cur_room;

  cur_room = room;

  gch = char_list;

  while (gch != NULL) {

    if ( is_same_group( gch, ch ) ) {
      cur_room = collect_room(cur_room, gch->in_room);
    }

    gch = gch->next;
  }

  return room;
}

/* Collect all rooms within a given scope of a given room... */

ROOM_INDEX_DATA *collect_rooms(ROOM_INDEX_DATA *room, 
                               int scope,
                               CHAR_DATA *actor) {

  ROOM_INDEX_DATA *collection;

 /* Safety check... */

  if ( room == NULL ) {
    return NULL;
  }

  collection = NULL;
  
 /* Now, where do we have to send it? */

  if (scope == WEV_SCOPE_NONE) {

   /* Send nowhere... (Better not to call at all) */

    collection = NULL;
  }

  if (scope == WEV_SCOPE_ROOM) {

   /* Send to just this room... (Better to call directly) */

    collection = room; 
  }

  if (scope == WEV_SCOPE_ADJACENT) {

   /* Send to this one and all neighbouring ones... */

    collection = collect_adjacent(room);
  }

  if (scope == WEV_SCOPE_SUBAREA) {

   /* Send to all rooms in the subarea... */

    collection = collect_subarea(room);
  }

  if (scope == WEV_SCOPE_SUBAREA_PLUS) {

   /* Send to all rooms in the subarea and all neighbouring rooms... */

    collection = collect_subarea_plus(room);
  }

  if (scope == WEV_SCOPE_AREA) {

   /* Send to all rooms in the area... (Expensive) */

    collection = collect_area(room);
  }

  if (scope == WEV_SCOPE_AREA_PLUS) {

   /* Send to all rooms in and adjacent to the area... (More Expensive) */

    collection = collect_area_plus(room);
  }

  if (scope == WEV_SCOPE_ZONE) {

   /* Send to all rooms in the zone... (Very Expensive) */

    collection = collect_zone(room);
  }

  if (scope == WEV_SCOPE_UNIVERSE) {

   /* Send to all rooms in the Mud... (Horrendously expensive) */

    collection = collect_all_rooms(room);
  }

  if (scope == WEV_SCOPE_GROUP) {

   /* Send to all rooms with a group member... */

    collection = collect_group_rooms(room, actor);
  }
  
  return collection;
}  


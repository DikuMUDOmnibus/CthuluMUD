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
#include "doors.h"
#include "vlib.h"
#include "map.h"


void trace_visibility(CHAR_DATA* ch) {
AREA_DATA *area = ch->in_room->area;
ROOM_INDEX_DATA *room;
ROOM_INDEX_DATA *to_room;
EXIT_DATA *pexit;
short t, newvis, fade;
bool done = FALSE;
int count = 0;
VNUM vnum;

    while (!done && count < 100) {
          done = TRUE;
          count++;

          for (vnum = area->lvnum; vnum <= area->uvnum; vnum ++) {
                 room = get_room_index(vnum);
                 if (!room) continue;
                 if (room->distance <= 0)  {
                      continue;
                 } else {
                     for (t = 0; t < DIR_MAX; t++) {
                         pexit = room->exit[t];
                         if (pexit) {
                            if (IS_SET(pexit->exit_info, EX_CLOSED)
                            || !exit_visible(ch, pexit)) continue;
                            
                            if ((to_room = get_exit_destination(ch, room, pexit, FALSE)) != NULL) {
                                if (!can_see_room(ch, to_room)) continue;

                                fade = vismod[room->sector_type];
                                if (room_is_dark(room)) fade *= 2;
                                if (IS_SET(room->room_flags, ROOM_INDOORS)) fade *= 2;
                                if (IS_SET(room->room_flags, ROOM_MISTY)) fade += 50;

                                newvis = room->distance - fade;
                                if (newvis > to_room->distance) {
                                    to_room->distance = UMAX(newvis, 0);
                                    done = FALSE;
                                } 
                            }
                         }
                     }
                 }
          }
    }         
    return;
}


void map_sector(CHAR_DATA* ch, int vision) {
AREA_DATA *area = ch->in_room->area;
ROOM_INDEX_DATA *room;
ROOM_INDEX_DATA *to_room;
EXIT_DATA *pexit;
short t;
bool done = FALSE;
int count = 0;
VNUM vnum;

    while (!done && count < 100) {
          done = TRUE;
          count ++;

          for (vnum = area->lvnum; vnum <= area->uvnum; vnum ++) {
                 room = get_room_index(vnum);
                 if (!room) continue;
                 if (room->distance < vision)  {
                      continue;
                 } else {
                     for (t = 0; t < DIR_MAX; t++) {
                         if (mapmod[t] == 0) continue;
                         pexit = room->exit[t];
                         if (pexit) {
                            if ((to_room = get_exit_destination(ch, room, pexit, FALSE)) != NULL) {
                                if (to_room->distance < vision && to_room->distance >= 0) {
                                   to_room->distance = room->distance + mapmod[t];
                                   done = FALSE;
                                }
                            }
                         }
                     }
                 }
          }
    }         
    return;
}


void do_map(CHAR_DATA *ch, char *argument) {
int map[MAP_WIDTH] [MAP_WIDTH];
ROOM_INDEX_DATA *room;
VNUM vnum;
int x, y, vision;
char outbuf[MAX_STRING_LENGTH];


    if (IS_AFFECTED(ch, AFF_BLIND))   {
           send_to_char("You are blind!\r\n",ch);
           return;
    }

    if (ch->move < 50) {
        send_to_char("You are much too exhausted!\r\n", ch);
        return;
    }
    ch->move -= 50;
    send_to_char("{mYou check your surroundings.{x\r\n",ch);

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_WIDTH; y++) {
              map[x][y] = -1;
        }
    }

    vision = START_VISION + get_curr_stat(ch, STAT_WIS) /10;
    if (IS_AFFECTED(ch, AFF_INFRARED)) vision += 20;
    if (IS_AFFECTED(ch, AFF_DARK_VISION)) vision += 10;
    if (IS_AFFECTED(ch, AFF_FARSIGHT)) vision += 100;
    if (IS_SET(ch->plr, PLR_HOLYLIGHT)) vision += 100;
    ch->in_room->distance = vision;

    trace_visibility(ch);
    ch->in_room->distance = vision + ((MAP_WIDTH-1) /2) * (MAP_WIDTH +1);
    map_sector(ch, vision);

    for (vnum = ch->in_room->area->lvnum; vnum <= ch->in_room->area->uvnum; vnum ++) {
           room = get_room_index(vnum);
           if (!room) continue;
           if (room->distance >= vision)  {
               room->distance -= vision;
               y = room->distance / MAP_WIDTH;    
               x = room->distance - y * MAP_WIDTH;
               if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_WIDTH) {
                   map[x][y] = room->sector_type;
               } 
           }
    }

    sprintf(outbuf, "{W*");
    for (x = 0; x < MAP_WIDTH; x++) strcat(outbuf, "*");
    strcat(outbuf, "*{x\r\n");

    for (y = 0; y < MAP_WIDTH; y++) {
        strcat(outbuf, "{W*{x");
        for (x = 0; x < MAP_WIDTH; x++) {
             if (x*2 == MAP_WIDTH-1 && y*2 == MAP_WIDTH-1) strcat(outbuf, "{x@");
             else if (map[x][y] == -1) strcat(outbuf, " ");
             else strcat(outbuf, sectchar[map[x][y]]);
        }
        strcat(outbuf, "{W*{x\r\n");
    }

    strcat(outbuf, "{W*");
    for (x = 0; x < MAP_WIDTH; x++) strcat(outbuf, "*");
    strcat(outbuf, "*{x\r\n");

    page_to_char(outbuf,ch);

    clear_distance(ch->in_room->area);
    return;
}


void clear_distance(AREA_DATA *area) {
ROOM_INDEX_DATA *room;
VNUM vnum;

    for (vnum = area->lvnum; vnum <= area->uvnum; vnum ++) {
          room = get_room_index(vnum);
          if (room) room->distance = -1;
    }       
    return;
}


void trace_distance(CHAR_DATA* ch, CHAR_DATA *target, AREA_DATA *area) {
ROOM_INDEX_DATA *room;
EXIT_DATA *pexit;
short generation = 0;
short t;
bool contact;
VNUM vnum;

    while (generation < 100) {
          contact = FALSE;

          for (vnum = area->lvnum; vnum <= area->uvnum; vnum ++) {
                 room = get_room_index(vnum);
                 if (!room) continue;
                 if (room->distance == -1)  continue;
                 if (room->distance > generation) continue;

                 for (t = 0; t < DIR_MAX; t++) {
                     pexit = room->exit[t];
                     if (pexit) {
                        contact = set_room_distance(ch, area, get_exit_destination(ch, room, pexit, FALSE), generation);
                        if (contact) return;
                     }
                 }
          }
          generation++;
    }         
    return;
}


bool set_room_distance(CHAR_DATA *ch, AREA_DATA *area, ROOM_INDEX_DATA *room, short generation) {
       
      if (room->area != area) return FALSE;
      if (room->distance > -1) return FALSE;
      
      room->distance = generation +1;
      if (room == ch->in_room) return TRUE;
      return FALSE;
}


void track(CHAR_DATA *ch, CHAR_DATA *target) {
EXIT_DATA *pexit;
ROOM_INDEX_DATA *room;
int t;
int low = -1;
int lowd = 99;

    target->in_room->distance = 0;
    trace_distance(ch, target, target->in_room->area);

    for (t = 0; t < DIR_MAX; t++) {
          pexit = ch->in_room->exit[t];
          if (!pexit) continue;

          room = get_exit_destination(ch, ch->in_room, pexit, FALSE);
          if (room->distance != -1) {
               if (room->distance < lowd) {
                    lowd = room->distance;
                    low = t;
               }
          }
    }

    clear_distance(ch->in_room->area);
    if (low == -1) {
         send_to_char("You find no way there.\r\n", ch);
         return;
    }

    move_char(ch, low, FALSE);
    return;
}

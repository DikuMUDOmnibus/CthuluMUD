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
#include "tracks.h"
#include "skill.h"
#include "mob.h"
#include "olc.h"
#include "race.h"
#include "gsn.h"

/* Tracks left by mobiles... */

static MOB_TRACKS *free_mob_track_chain = NULL;

void free_mob_tracks(MOB_TRACKS *tracks) {

    if (tracks->next != NULL) {
      free_mob_tracks(tracks->next);
      tracks->next = NULL;
    }

    if (tracks->name != NULL) {
      free_string(tracks->name);
      tracks->name = NULL;
    }

    tracks->name2 = NULL;

    tracks->count = 0;
    tracks->size = 0;

    tracks->next = free_mob_track_chain; 

    free_mob_track_chain = tracks;

    return;
};

MOB_TRACKS *get_mob_tracks() {
  
    MOB_TRACKS *new_tracks;

    if (free_mob_track_chain != NULL) {
      new_tracks = free_mob_track_chain;
      free_mob_track_chain = new_tracks->next;
    } else {
      new_tracks = (MOB_TRACKS *) alloc_perm(sizeof(*new_tracks));
    }

    new_tracks->name = NULL;
    new_tracks->name2 = NULL;
  
    new_tracks->size = 0;
    new_tracks->count = 0;

    new_tracks->next = NULL;

    return new_tracks;
} 

void reduce_usage(MOB_TRACKS *tracks) {

  if (tracks != NULL) {
    tracks->count -= 1;
  }

  return;
}

void increase_usage(MOB_TRACKS *tracks) {
 
  if (tracks != NULL) {
    tracks->count += 1;
  }

  return;
} 

MOB_TRACKS *find_mob_tracks(CHAR_DATA *ch) {
OBJ_DATA *boots;

   /* If the mob doesn't have any tracks yet... */

    if (ch->tracks == NULL) {

     /* Tiny creatures never leave tracks... */

      if (get_char_size(ch) == SIZE_TINY) {
        return NULL;
      }

     /* If it has an index, check that... */ 

      if (ch->pIndexData != NULL) {

       /* Create tracks if none present... */

        if (ch->pIndexData->tracks == NULL) {
          ch->pIndexData->tracks = get_mob_tracks();

          ch->pIndexData->tracks->name = str_dup(ch->pIndexData->short_descr);
          ch->pIndexData->tracks->size = ch->pIndexData->size;

          increase_usage(ch->pIndexData->tracks);

          boots = get_eq_char(ch, WEAR_FEET);

          if (boots == NULL) {
            ch->pIndexData->tracks->name2 = race_array[ch->race].track_name;
          } else {
            ch->pIndexData->tracks->name2 = "boot";
          }
        }

        ch->tracks = ch->pIndexData->tracks;

        increase_usage(ch->tracks);

      } else { 

       /* No pIndexData means it's a player... */
    
        ch->tracks = get_mob_tracks();

        ch->tracks->name = str_dup(ch->short_descr);
        ch->tracks->size = get_char_size(ch);

        increase_usage(ch->tracks);

        boots = get_eq_char(ch, WEAR_FEET);

        if (boots == NULL) {
          ch->tracks->name2 = race_array[ch->race].track_name;
        } else {
          ch->tracks->name2 = "boot";
        }
      }
    }

   /* Flying critters do not leave tracks... */

    if (IS_AFFECTED(ch, AFF_FLYING)
    || IS_AFFECTED(ch, AFF_MIST)) {
      return NULL;
    }

   /* Tracks cannot be left in certain rooms... */

    if (ch->in_room != NULL) {

      switch (ch->in_room->sector_type) {

        case SECT_UNDERWATER:
        case SECT_WATER_SWIM:
        case SECT_WATER_NOSWIM:
        case SECT_ACID:
        case SECT_ROAD:
        case SECT_CITY:
        case SECT_INSIDE:
        case SECT_AIR:
        case SECT_SPACE:
          return NULL;

        default: 
          break;
      }
    }

    return ch->tracks;
}

/* Tracks in a room... */

static TRACKS *free_track_chain = NULL;

void free_tracks(TRACKS *tracks) {

    if (tracks->next != NULL) {
      free_tracks(tracks->next);
      tracks->next = NULL;
    }

    reduce_usage(tracks->mob);

    if (tracks->mob->count <= 0) {
      free_mob_tracks(tracks->mob);
    }

    tracks->mob = NULL; 

    tracks->from = DIR_HERE;
    tracks->to = DIR_HERE;

    tracks->size = 0;

    tracks->script = 0;

    tracks->next = free_track_chain; 

    free_track_chain = tracks;

    return;
}

TRACKS *get_tracks() {

    TRACKS *new_tracks;

    if (free_track_chain != NULL) {
      new_tracks = free_track_chain;
      free_track_chain = new_tracks->next;
    } else {
      new_tracks = (TRACKS *) alloc_perm(sizeof(*new_tracks));
    }

    new_tracks->mob = NULL;
  
    new_tracks->to = DIR_NONE;
    new_tracks->from = DIR_NONE;

    new_tracks->size = SIZE_TINY;

    new_tracks->script = 0;

    new_tracks->next = NULL;

    return new_tracks;
} 

void add_tracks(CHAR_DATA *ch, int from, int to) {

    MOB_TRACKS *mob_tracks;
    TRACKS *tracks;

   /* See if it is somewhere where it can leave tracks... */

    if (ch->in_room == NULL) {
      return;
    }

   /* See if the mob leaves tracks... */

    mob_tracks = find_mob_tracks(ch);

    if (mob_tracks == NULL) {
      return;
    }

   /* Now find a track record in this room... */
 
    tracks = ch->in_room->tracks;

    while ( tracks != NULL 
         && tracks->mob != mob_tracks ) {

      tracks = tracks->next;
    }  

   /* If none found, we must add them... */

    if (tracks == NULL) {
      tracks = get_tracks();

      tracks->mob = mob_tracks;

      increase_usage(mob_tracks);

      tracks->next = ch->in_room->tracks;

      ch->in_room->tracks = tracks;
    } 

   /* Reset track size... */

    if (mob_tracks->size > tracks->size) {
      tracks->size = mob_tracks->size;
    } 

   /* A from track, means the mob has arrived... */

    if ( from > DIR_NONE
      && from <= DIR_OTHER ) {
      tracks->from = from;
    } 

   /* A to track means it has left the room... */ 

    if ( to > DIR_NONE
      && to <= DIR_OTHER ) {
      tracks->to = to;
    } 

   /* All done... */

    return;
};

int surface_age[SURFACE_MAX] = { 1, 2, 0, 0, 1, 2 };

int weather_age[WEATHER_MAX] = { 0, 0, 0, 0, 1, 3, 3, 5 };

void age_tracks(ROOM_INDEX_DATA *room) {

    TRACKS *tracks, *next_tracks, *prev_tracks;

    int delta;

    bool erased; 

   /* Check for the easy case... */

    if ( room == NULL
      || room->tracks == NULL) {
      return;
    } 

   /* Work out the ammount of aging... */

    delta = surface_age[get_surface(room)] + weather_age[get_weather(room)];

   /* Now apply aging to all of the tracks... */

    tracks = room->tracks; 
    prev_tracks = NULL;

    while ( tracks != NULL) {
      next_tracks = tracks->next;

      erased = FALSE;

      if (tracks->size > 0) {
         
        tracks->size -= delta;

        if (tracks->size < 1) {

          erased = TRUE;

          if (prev_tracks == NULL) {
            room->tracks = next_tracks;
          } else {
            prev_tracks->next = next_tracks;
          }

          tracks->next = NULL; 

          free_tracks(tracks);
        } else {

          if (tracks->size > SIZE_GIANT) {
            tracks->size = SIZE_GIANT;
          }
        }
      }
 
      if (!erased) {
        prev_tracks = tracks;
      }

      tracks = next_tracks;
    }

    return;
};

int weather_track_mod[WEATHER_MAX] = {
  
  0, 0, 0, -5, -15, -30, -20, -50

};

int surface_track_mod[SURFACE_MAX] = {
  
  0, -20, 0, +10, -15, -10

};

int track_spot[SIZE_MAXI] = {
  
  130, 110, 100, 90, 75, 50

};

char track_desc[SIZE_MAXI][15] = {

 "(faint)", "(small)", "(medium)", "(large)", "(huge)", "(gigantic)"

};

char track_dir[DIR_OTHER + 2][15] = {

  "somewhere",
  "north", "east", "south", "west", "up", "down",
  "northeast", "southeast", "southwest", "northwest",
  "here", "somewhere"

};

void do_all_tracks(CHAR_DATA *ch) {

    MOB_TRACKS *mob_tracks;
    TRACKS *tracks;

    int roll, roll2;

    bool show_all, found;

    char buf[MAX_STRING_LENGTH]; 

    if (ch->in_room == NULL) {
      send_to_char("There are no tracks in the void...\r\n", ch); 
      return;
    }

   /* Check for tracks... */

    roll = number_open() + get_skill(ch, gsn_tracking);

    roll += weather_track_mod[get_weather(ch->in_room)];

    roll += surface_track_mod[get_surface(ch->in_room)];

    found = FALSE;

    show_all = FALSE;

    if ( IS_IMMORTAL(ch) 
      && IS_SET(ch->plr, PLR_HOLYLIGHT) ) {
      show_all = TRUE;
    }

   /* First dynamic tracks... */

    tracks = ch->in_room->tracks;

    while (tracks != NULL) {

     /* See if we spot them... */ 

      if ( show_all
        || roll > track_spot[tracks->size] ) {

        if (!found) {
          send_to_char("You find some tracks:\r\n", ch);
          found = TRUE;
        }

        roll2 = number_open() + get_skill(ch, gsn_tracking);

        roll += weather_track_mod[get_weather(ch->in_room)];

        roll += surface_track_mod[get_surface(ch->in_room)];

        mob_tracks = tracks->mob;

        if ( show_all
          || roll2 > 100) {
          sprintf(buf, "  %s tracks %s from %s to %s\r\n",
                        mob_tracks->name,
                        track_desc[tracks->size],
                        track_dir[tracks->from + 1],
                        track_dir[tracks->to + 1]);
        } else {
          sprintf(buf, "  %s tracks %s from %s to %s\r\n",
                        mob_tracks->name2,
                        track_desc[tracks->size],
                        track_dir[tracks->from + 1],
                        track_dir[tracks->to + 1]);
        }

        buf[2] = toupper(buf[2]);

        send_to_char(buf, ch);
      }

      tracks = tracks->next;
    } 

   /* Then persistent tracks... */

    tracks = ch->in_room->ptracks;

    while (tracks != NULL) {

     /* See if we spot them... */ 

      if ( show_all
        || ( tracks->mob->name[0] != '_'
          && roll > track_spot[tracks->size] )) {

        if (!found) {
          send_to_char("You find some tracks:\r\n", ch);
          found = TRUE;
        }

        roll2 = number_open() + get_skill(ch, gsn_tracking);

        roll += weather_track_mod[get_weather(ch->in_room)];

        roll += surface_track_mod[get_surface(ch->in_room)];

        mob_tracks = tracks->mob;

        if ( show_all
          || roll2 > 100) {

          if ( mob_tracks->name[0] != '_' ) {
            sprintf(buf, "  %s tracks %s from %s to %s\r\n",
                          mob_tracks->name,
                          track_desc[tracks->size],
                          track_dir[tracks->from + 1],
                          track_dir[tracks->to + 1]);
          } else {         
            sprintf(buf, "  Track %s from %s to %s, script %d\r\n",
                          mob_tracks->name,
                          track_dir[tracks->from + 1],
                          track_dir[tracks->to + 1],
                          tracks->script);
          }          
        } else {
          sprintf(buf, "  %s tracks %s from %s to %s\r\n",
                        mob_tracks->name2,
                        track_desc[tracks->size],
                        track_dir[tracks->from + 1],
                        track_dir[tracks->to + 1]);
        }

        buf[2] = toupper(buf[2]);

        send_to_char(buf, ch);
      }

      tracks = tracks->next;
    } 

   /* Complete message if tracks not found... */

    if (!found) {
      send_to_char("You find no tracks here.\r\n", ch);
      return;
    }

    check_improve(ch, gsn_tracking, TRUE, 2);

    return;
}

TRACKS *find_tracks(ROOM_INDEX_DATA *room, char *name) {

    TRACKS *tracks;

    tracks = NULL; 

    if ( name[0] != '_' ) {

      tracks = room->tracks;

      while (tracks != NULL) {

        if (is_name_abbv(name, tracks->mob->name)) {
          return tracks;
        }

        if (is_name_abbv(name, tracks->mob->name2)) {
          return tracks;
        }

        tracks = tracks->next;
      }
    }

    if ( tracks == NULL ) {

      tracks = room->ptracks;

      while ( tracks != NULL ) {

        if (is_name_abbv(name, tracks->mob->name)) {
          return tracks;
        }

        if (is_name_abbv(name, tracks->mob->name2)) {
          return tracks;
        }

        tracks = tracks->next;
      }
    }

    return NULL;
}

void do_tracks(CHAR_DATA *ch, char *args) {

    TRACKS *tracks;

    MOB_SCRIPT *script;

    MOB_CMD_CONTEXT *mcc;

    ROOM_INDEX_DATA *old_room; 

    int roll;

    int delay;

    char buf[MAX_STRING_LENGTH]; 

    CHAR_DATA *victim;

    if (args[0] == '?') {
      send_to_char("Syntax: tracks\r\n"
                   "        track 'mobile'\r\n", ch);
      return;
    }

    if (args[0] == '\0') {
      do_all_tracks(ch);
      return;
    }  

   /* Retrieve key from command... */

    if (!check_activity_key(ch, args)) {
      return;
    }

   /* If already tracking and not a recall, complain... */

    if ( ch->activity == ACV_TRACKING
      && args[0] != '*' ) {
      send_to_char("You are already tracking something!\r\n", ch);
      return;
    }

   /* State 0 means not tracking yet... */

    if ( ch->activity != ACV_TRACKING ) {
      ch->acv_state = 0;
    }

   /* Switch to the appropriate state... */

    switch (ch->acv_state) {

      default: 

        if ( args[0] != '_' ) {
          sprintf(buf, "tracking '%s'", args);
        } else {
          sprintf(buf, "tracking something");
        }

        set_activity(ch, POS_STANDING, NULL, ACV_TRACKING, buf);

        send_to_char("You starting looking for tracks...\r\n", ch);

        set_activity_key(ch);

        free_string(ch->acv_text);
        ch->acv_text = str_dup(args);

        if ( ch->acv_text[0] == '_'
          && IS_NPC(ch) ) { 
          ch->acv_state = ACV_TRACKING_MOVE;
          schedule_activity(ch, 1, "track");
        } else {
          ch->acv_state = ACV_TRACKING_SEARCH;
          schedule_activity(ch, 2, "track");
        }

        break;
 
      case ACV_TRACKING_SEARCH:

        victim = get_char_room(ch, ch->acv_text);

        if (victim != NULL) { 
          sprintf(buf, "You have found: %s!\r\n",
                        victim->short_descr);
          send_to_char(buf, ch);
          set_activity(ch, POS_STANDING, NULL, ACV_NONE, NULL);
          return;
        }

        tracks = find_tracks(ch->in_room, ch->acv_text); 

        if ( tracks != NULL
          && ch->acv_text[0] != '_' ) {
 
          roll = number_open() + get_skill(ch, gsn_tracking); 
           
          roll += weather_track_mod[get_weather(ch->in_room)];

          roll += surface_track_mod[get_surface(ch->in_room)];

          if ( roll < track_spot[tracks->size] ) {
            tracks = NULL;
          }
        }

        if (tracks != NULL) { 

          if ( tracks->to < DIR_NORTH
            || tracks->to > DIR_OUT ) {

            if ( tracks->to == DIR_HERE) {
              send_to_char("The tracks stop here.\r\n", ch);
            } else {
              send_to_char("The tracks end abruptly!\r\n", ch);
            }

            set_activity(ch, POS_STANDING, NULL, ACV_NONE, NULL);

            return;
          }  

          sprintf(buf, "You find '%s' tracks leading %s.\r\n", 
                        tracks->mob->name,
                        track_dir[tracks->to + 1]);
          send_to_char(buf, ch);

          ch->acv_state = ACV_TRACKING_MOVE;
          check_improve(ch, gsn_tracking, TRUE, 2);

        } else {

          send_to_char("...no tracks...\r\n", ch);
          ch->acv_state = ACV_TRACKING_SEARCH;
        }

        schedule_activity(ch, 2, "track");

        break;

      case ACV_TRACKING_MOVE:

        tracks = find_tracks(ch->in_room, ch->acv_text); 

        if ( tracks == NULL) {
          send_to_char("The tracks have faded...\r\n", ch);
          set_activity(ch, POS_STANDING, NULL, ACV_NONE, NULL);
          return;
        }

        if ( tracks->to < DIR_NORTH
          || tracks->to > DIR_OUT ) {
          send_to_char("...you don't know how to follow them!\r\n", ch);
          set_activity(ch, POS_STANDING, NULL, ACV_NONE, NULL);
          return;
        }

        old_room = ch->in_room;

        move_char( ch, tracks->to, FALSE);

        if (ch->activity != ACV_TRACKING) {
          send_to_char("...nature calls, you stop tracking!\r\n", ch);
          set_activity(ch, POS_STANDING, NULL, ACV_NONE, NULL);
          return;
        }

        if ( ch->in_room == old_room ) {
          send_to_char("...you are not able to follow them!\r\n", ch);
          set_activity(ch, POS_STANDING, NULL, ACV_NONE, NULL);
          return;
        }

        if ( tracks->mob->name[0] == '_' 
          && IS_NPC(ch) 
          && tracks->script != 0 ) {  
          
          script = find_script(ch, tracks->script);

          if (script != NULL) {
            set_activity(ch, POS_STANDING, NULL, ACV_NONE, NULL);

            mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL);

            enqueue_script(ch, script, mcc);

            return;
          }
        }

        if ( ch->acv_text[0] == '_'
          && IS_NPC(ch) ) { 

          ch->acv_state = ACV_TRACKING_MOVE;

          delay = tracks->size;

          if (delay < 1) {
            delay = 2;
          } else if (delay > 5) {
            delay = 5;
          }

          schedule_activity(ch, delay, "track");

        } else {
          ch->acv_state = ACV_TRACKING_SEARCH;
          schedule_activity(ch, 2, "track");
        }

        break;

    }

    return;
}


/* Save tracks to file... */

void save_tracks(FILE *fp, TRACKS *tracks, char *hdr) {

    while (tracks != NULL) {

      fprintf(fp, "%s '%s' '%s' %d %d %d %d\n",
                  hdr,
                  tracks->mob->name,
                  tracks->mob->name2,
                  tracks->size,
                  tracks->from,
                  tracks->to,
                  tracks->script);

      tracks = tracks->next; 
    }

    return;
}

TRACKS *read_tracks(FILE *fp) {

    TRACKS *tracks;

    tracks = get_tracks();

    tracks->mob = get_mob_tracks();

    increase_usage(tracks->mob);

    tracks->mob->name = str_dup(fread_word(fp)); 

    tracks->mob->name2 = str_dup(fread_word(fp)); 

    tracks->mob->size = fread_number(fp);

    tracks->size = tracks->mob->size;

    tracks->from = fread_number(fp);

    tracks->to = fread_number(fp);
 
    tracks->script = fread_number(fp); 

    return tracks;
}

/* Show all ptracks is a room... */

void redit_ptracks_list(CHAR_DATA *ch, char *args, ROOM_INDEX_DATA *room) {

    TRACKS *tracks;
    MOB_TRACKS *mob_tracks;
    char buf[MAX_STRING_LENGTH];

   /* Then persistent tracks... */

    tracks = room->ptracks;

    if (tracks == NULL) {
      send_to_char("No permanent tracks defined.\r\n", ch);
      return;
    }

    send_to_char("Permanent tracks:\r\n", ch);
 
    while (tracks != NULL) {

     /* Dump time... */ 

      mob_tracks = tracks->mob;

      if ( tracks->mob->name[0] != '_' ) {
        sprintf(buf, "  %s (%s) tracks %s from %s to %s\r\n",
                      mob_tracks->name,
                      mob_tracks->name2,
                      track_desc[tracks->size],
                      track_dir[tracks->from + 1],
                      track_dir[tracks->to + 1]);
      } else {
        sprintf(buf, "  %s tracks to %s (speed %d, script %d)\r\n",
                      mob_tracks->name,
                      track_dir[tracks->to + 1],
                      tracks->size,
                      tracks->script);
      }

      buf[2] = toupper(buf[2]);

      send_to_char(buf, ch);

      tracks = tracks->next;
    } 

    return;
}

/* Make mob tracks... */

MOB_TRACKS *make_mob_tracks(char *name1, char *name2, int size) {

    MOB_TRACKS *mob_tracks;

    mob_tracks = get_mob_tracks();

    increase_usage(mob_tracks);

    mob_tracks->name = str_dup(name1);
    mob_tracks->size = size;
    mob_tracks->name2 = str_dup(name2);

    return mob_tracks;
}

/* Add ptracks to a room... */

void redit_ptracks_add(CHAR_DATA *ch, char *args, ROOM_INDEX_DATA *room) {

    TRACKS *tracks, *old_tracks, *new_tracks;

    char name1[MAX_STRING_LENGTH];
    char name2[MAX_STRING_LENGTH];
    char parm[MAX_STRING_LENGTH];
 
    int size, from, to, script;

    name1[0] = '\0';
    name2[0] = '\0';

    size = 0;
    from = DIR_NONE;
    to = DIR_NONE;
    script = 0;

    if (args[0] == '\0') {
      send_to_char("You must specify a name!\r\n", ch);
      return;
    }

    args = one_argument(args, name1);

    if (name1[0] != '_') {

      if (args[0] == '\0') {
        send_to_char("You must specify a second name!\r\n", ch);
        return;
      }

      args = one_argument(args, name2);

      if (args[0] == '\0') {
        send_to_char("You must specify a size!\r\n", ch);
        return;
      }

      args = one_argument(args, parm);
     
      size = atoi(parm);

      if ( size < SIZE_TINY 
        || size >= SIZE_GIANT ) {
        send_to_char("Size is from 0 (tiny) to 5 (giant)\r\n", ch);
        return;
      }
          
      if (args[0] == '\0') {
        send_to_char("You must specify a from direction!\r\n", ch);
        return;
      }

      args = one_argument(args, parm);

      from = atoi(parm);

      if ( from < DIR_NONE 
        || from > DIR_OTHER ) {
        send_to_char("from is from -1 to 11\r\n", ch);
        return;
      }

      if (args[0] == '\0') {
        send_to_char("You must specify a to direction!\r\n", ch);
        return;
      }

      args = one_argument(args, parm);

      to = atoi(parm);

      if ( to < DIR_NONE 
        || to > DIR_OTHER ) {
        send_to_char("to is from -1 to 11\r\n", ch);
        return;
      }
    } else {

      if (args[0] == '\0') {
        send_to_char("You must specify a to direction!\r\n", ch);
        return;
      }

      args = one_argument(args, parm);

      to = atoi(parm);

      if ( to < DIR_NONE 
        || to > DIR_OTHER ) {
        send_to_char("to is from -1 to 11\r\n", ch);
        return;
      }

      size = 2;

      if (args[0] != '\0') {

        args = one_argument(args, parm);

        size = atoi(parm);

        if ( size < 1
          || size > 5 ) {
          send_to_char("Delay is from 1 to 5 seconds\r\n", ch);
          return;
        }
      }

      if (args[0] != '\0') {

        args = one_argument(args, parm);

        script = atoi(parm);

        if ( script < 0 ) {
          send_to_char("Script must be >= 0\r\n", ch);
          return;
        }
      }
    }

    old_tracks = NULL;
    tracks = room->ptracks;

   /* Easy case... */

    if (tracks == NULL) {
      new_tracks = get_tracks();

      new_tracks->mob    = make_mob_tracks(name1, name2, size);
      new_tracks->size   = size;
      new_tracks->from   = from;
      new_tracks->to     = to;
      new_tracks->script = script;
 
      room->ptracks = new_tracks;
      new_tracks->next = NULL;

      send_to_char("New tracks added.\r\n", ch);
    
      return;
    }

    while (tracks != NULL) {

      if (tracks->mob->name[0] == name1[0]) {
        if (!str_cmp(tracks->mob->name, name1)) {
          break;
        }
      }

      old_tracks = tracks;
      tracks = tracks->next; 
    }

   /* Add new ? */

    if (tracks == NULL) {
      new_tracks = get_tracks();

      new_tracks->mob    = make_mob_tracks(name1, name2, size);
      new_tracks->size   = size;
      new_tracks->from   = from;
      new_tracks->to     = to;
      new_tracks->script = script;
 
      if (old_tracks != NULL) {
        old_tracks->next = new_tracks;
        new_tracks->next = NULL;
      } else {
        new_tracks->next = room->ptracks;
        room->ptracks = new_tracks;
      }

      send_to_char("New tracks added.\r\n", ch);
    
      return;
    } 

   /* Update... */

    free_string(tracks->mob->name2);
    tracks->mob->name2 = str_dup(name2);

    tracks->size = size;
    tracks->from = from;
    tracks->to = to;
    tracks->script = script;

    send_to_char("Tracks updated.\r\n", ch);
    
    return;
}

/* Delete ptracks to a room... */

void redit_ptracks_delete(CHAR_DATA *ch, char *args, ROOM_INDEX_DATA *room) {

    TRACKS *tracks, *old_tracks;

    old_tracks = NULL;
    tracks = room->ptracks;

    while (tracks != NULL) {

      if (tracks->mob->name[0] == args[0]) {
        if (!str_cmp(tracks->mob->name, args)) {

          if ( old_tracks == NULL ) {
            room->ptracks = tracks->next;
          } else {
            old_tracks->next = tracks->next;
          }

          tracks->next = NULL;

          free_tracks(tracks);

          send_to_char("Tracks deleted.\r\n", ch);

          return;
        }
      }

      old_tracks = tracks;
      tracks = tracks->next;
    }

    send_to_char("Tracks not defined!\r\n", ch);

    return;
}

/* Edit a rooms ptracks... */

bool redit_ptracks(CHAR_DATA *ch, char *args) {

  ROOM_INDEX_DATA *pRoom;

  char command[MAX_INPUT_LENGTH];
  char *parms;

 /* What are we editing... */

  EDIT_ROOM(ch, pRoom);

 /* Give help if needed... */ 

  if ( args[0] == '\0'
    || args[0] == '?' ) {

    send_to_char("Syntax: ptracks list\r\n", ch);
    send_to_char("        ptracks add name1 name2 size from to\r\n", ch);
    send_to_char("        ptracks add _name1 to delay script\r\n", ch);
    send_to_char("        ptracks delete name1\r\n", ch);

    return FALSE;
  }   

 /* Extract command and parms... */

  parms = one_argument(args, command);

 /* List monitors... */

  if (!str_cmp(command, "list")) { 
    redit_ptracks_list(ch, parms, pRoom); 
    return FALSE;
  } 

 /* Add/replace a monitor... */

  if (!str_cmp(command, "add")) { 
    redit_ptracks_add(ch, parms, pRoom); 
    return TRUE;
  } 

 /* delete a monitor... */

  if (!str_cmp(command, "delete")) { 
    redit_ptracks_delete(ch, parms, pRoom); 
    return TRUE;
  } 

  redit_ptracks(ch, "");

  return FALSE;
}

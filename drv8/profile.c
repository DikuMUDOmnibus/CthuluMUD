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
#include "profile.h"

/* External function... */

 extern AREA_DATA *new_area();

/* Globals */

 MUD_DATA mud;

 ZONE_DATA zones[NUM_ZONES];

/* Check if a room is ok for recall/respawn etc... */

bool room_ok_for_recall(int vnum) {
 
  ROOM_INDEX_DATA *room;
 
 /* The room must exist... */
 
  room = get_room_index(vnum);

  if (room == NULL) return FALSE;

 /* The room should not have any strange flags... */ 

  if ( IS_SET(room->room_flags, ROOM_PRIVATE)
    || IS_SET(room->room_flags, ROOM_SOLITARY)
    || IS_SET(room->room_flags, ROOM_NO_RECALL)
    || IS_SET(room->room_flags, ROOM_IMP_ONLY)
    || IS_SET(room->room_flags, ROOM_GODS_ONLY)
    || IS_SET(room->room_flags, ROOM_NEWBIES_ONLY) ) {
    return FALSE;
  }

 /* Looks OK for now... */

  return TRUE;
}

/* Load profile... */

void load_profile() {
FILE *fp;
char *kwd;
char *opt;
char code;
bool done;
bool ok;
int i;
long zmask;
 
  char buf[MAX_STRING_LENGTH];

 /* Find the society file... */

  fp = fopen(PROFILE_FILE, "r");

  if (fp == NULL) {
    log_string("No profile file!");
    exit(1);
    return;
  }

 /* Default values... */

  mud.name = "CthulhuMud";
  mud.port = 9999;
  mud.monitor = 20;

  mud.flags = 0;

  SET_BIT(mud.flags, MUD_PERMAPK);
  SET_BIT(mud.flags, MUD_AUTOOLC);
  SET_BIT(mud.flags, MUD_EFFXPS); 

  mud.url 		= strdup("None");;
  mud.start		= DEFAULT_RECALL;
  mud.recall		= DEFAULT_RECALL;
  mud.respawn		= DEFAULT_RESPAWN;
  mud.morgue		= DEFAULT_MORGUE;
  mud.dream		= 0;              
  mud.mare		= 0;              
  mud.home		= DEFAULT_HOME;              
  mud.diff		= 0;              
  mud.pk_timer		= 15;              
  mud.approval_timer	= 0;              
  mud.player_count	= 0;              
  mud.map           		= strdup("None");
  mud.worth[0] 		= 0;
  mud.reboot 		= -1;
  for (i=1; i < MAX_CURRENCY; i++) mud.worth[i] = -1;

 /* Default zones... */

  zmask = 1;

  for( i = 0; i < NUM_ZONES; i++ ) {
    zones[i].zid	= i; 
    zones[i].name	= strdup("Unnamed");
    zones[i].map	= strdup("None");
    zones[i].flags	= 0;
    zones[i].recall	= 0;
    zones[i].respawn	= 0;
    zones[i].morgue	= 0;
    zones[i].mask	= zmask; 
    zones[i].loaded	= FALSE;
    zmask *= 2;
  }

  zones[0].loaded = TRUE;

 /* Read through it and see what we've got... */
 
  done = FALSE;

  while(!done) {
    kwd = fread_word(fp);
    code = kwd[0];
    ok = FALSE;

    switch (code) {

    /* Comments */

      case '#':
        
        ok = TRUE;

        fread_to_eol(fp); 

        break; 


       case 'A':
   
        if (!strcmp(kwd, "AutoOLC")) {

          ok = TRUE;
          opt = fread_word(fp);
          if (!strcmp(opt, "YES")) {
            bug("Mud booting in OLC autosave mode!", 0);
            SET_BIT(mud.flags, MUD_AUTOOLC); 
          } else if (!strcmp(opt, "NO")) {
            bug("Mud booting in manual OLC save mode!", 0);
            REMOVE_BIT(mud.flags, MUD_AUTOOLC); 
          } else {
            bug("Bad OLC save value in profile.txt - ignored.", 0);
          }

        }

        if (!strcmp(kwd, "Approval")) {
          ok = TRUE;
          mud.approval_timer = fread_number(fp);
          if (mud.approval_timer < 0 || mud.approval_timer > 9) mud.pk_timer = 5;
        }
         break;

      case 'B':      

        if (!strcmp(kwd, "Buildings")) {
          ok = TRUE;
          mud.building[0] = fread_number(fp);
          mud.building[1] = fread_number(fp);
          if (mud.building[0] == 0
          || mud.building[0] >= mud.building[1]) {
            bug("Bad building room (%d) in profile file!", mud.building[0]);
            exit(1);
          }

        }
        break;


      case 'C':      

        if (!strcmp(kwd, "Currency")) {
          int t;
          ok = TRUE;
          for (t=0; t < MAX_CURRENCY; t++) mud.worth[t] = fread_number(fp);
        }

        if (!strcmp(kwd, "Cron")) {
          ok = TRUE;
          opt = fread_word(fp);

          if (!strcmp(opt, "YES")) {
            SET_BIT(mud.flags, MUD_CRON); 
          } else if (!strcmp(opt, "NO")) {
            REMOVE_BIT(mud.flags, MUD_CRON); 
          }
        }
        break;

     /* Dream */

      case 'D':
     
       /* Dream vnum */
   
        if (!strcmp(kwd, "Dream")) {

          ok = TRUE;

          mud.dream = fread_number(fp);

          if ( mud.dream != 0
            && !room_ok_for_recall(mud.dream) ) {
            bug("Bad dream room (%d) in profile file!", mud.dream);
            exit(1);
          }
        }

        if (!strcmp(kwd, "Diff")) {
          ok = TRUE;
          opt = fread_word(fp);
          if (!strcmp(opt, "easy")) {
               mud.diff = DIFF_EASY;
               bug("Mud booting in EASY mode!", 0);
          } else if (!strcmp(opt, "hard")) {
               mud.diff = DIFF_HARD;
               bug("Mud booting in HARD mode!", 0);
          } else {
               mud.diff = DIFF_BALANCED;
               bug("Mud booting in BALANCED mode!", 0);
          }
        }

        break; 

     /* EffectiveXPs */

      case 'E':
     
       /* End */

        if (!str_cmp(kwd, "End")) {
          ok = TRUE;
          done = TRUE;
        }

       /* EffectiveXPs */
   
        if (!strcmp(kwd, "EffectiveXPs")) {

          ok = TRUE;

          opt = fread_word(fp);

          if (!strcmp(opt, "YES")) {
            bug("Mud booting in Effective Level mode!", 0);
            SET_BIT(mud.flags, MUD_EFFXPS); 
          } else if (!strcmp(opt, "NO")) {
            bug("Mud booting in base level mode!", 0);
            REMOVE_BIT(mud.flags, MUD_EFFXPS); 
          } else {
            bug("Bad EffectiveXPs value in profile.txt - ignored.", 0);
          }

        }

        break; 


    case 'H':

        if (!strcmp(kwd, "Home")) {
          ok = TRUE;
          mud.home = fread_number(fp);
          if ( mud.home != 0
            && !room_ok_for_recall(mud.home)) {
            bug("Bad Home room (%d) in profile file!", mud.home);
            exit(1);
          }
        }
        break;

     /* MUD */

      case 'M':
     
       /* MUD 'mud_name' */
   
        if (!strcmp(kwd, "MUD")) {
          ok = TRUE;
          mud.name = strdup(fread_word(fp));
        }

       /* MUD 'mud_name' */
   
        if (!strcmp(kwd, "Map")) {
          ok = TRUE;
          mud.map = strdup(fread_word(fp));
        }

       /* Morgue vnum */
   
        if (!strcmp(kwd, "Morgue")) {

          ok = TRUE;

          mud.morgue = fread_number(fp);

          if (  mud.morgue != 0
            && !room_ok_for_recall(mud.morgue) ) {
            bug("Bad morgue room (%d) in profile file!", mud.morgue);
            exit(1);
          }
        }

       /* Monitor vnum */
   
        if (!strcmp(kwd, "Monitor")) {

          ok = TRUE;

          mud.monitor = fread_number(fp);

          if (mud.monitor == 0) {
            bug("Bad monitor room (%d) in profile file!", mud.monitor);
            exit(1);
          }
        }

        if (!strcmp(kwd, "Mare")) {
          ok = TRUE;
          mud.mare = fread_number(fp);
          if ( mud.mare != 0
            && !room_ok_for_recall(mud.mare) ) {
            bug("Bad Nightmare room (%d) in profile file!", mud.mare);
            exit(1);
          }
        }

        break; 

     /* Newlock */

      case 'N':
     
       /* NewLock YES|NO */
   
        if (!strcmp(kwd, "NewLock")) {

          ok = TRUE;

          opt = fread_word(fp);

          if (!strcmp(opt, "YES")) {
            bug("Mud booting in NEWLOCK mode!", 0);
            SET_BIT(mud.flags, MUD_NEWLOCK); 
          } else if (strcmp(opt, "NO")) {
            bug("Bad WizLock value in profile.txt - ignored.", 0);
          }

        }

        break; 


      case 'P':
     
        if (!strcmp(kwd, "Pattern")) {
          ok = TRUE;
          mud.pattern[0] = fread_number(fp);
          mud.pattern[1] = fread_number(fp);
          if (mud.pattern[0] == 0
          || mud.pattern[0] >= mud.pattern[1]) {
            bug("Bad pattern room (%d) in profile file!", mud.pattern[0]);
            exit(1);
          }

        }

       /* Port port */
   
        if (!strcmp(kwd, "Port")) {

          ok = TRUE;

          mud.port = fread_number(fp);

          if ( mud.port <= 1024 ) {
            bug("Invalid port (%d) specified in profile file", mud.port);
            exit(1);
          }
        }

       /* PermaPK */
   
        if (!strcmp(kwd, "PermaPK")) {
          ok = TRUE;
          opt = fread_word(fp);

          if (!strcmp(opt, "YES")) {
            bug("Mud booting in Perma-death PK mode!", 0);
            SET_BIT(mud.flags, MUD_PERMAPK); 
          } else if (!strcmp(opt, "NO")) {
            bug("Mud booting in Free PK mode!", 0);
            REMOVE_BIT(mud.flags, MUD_PERMAPK); 
          } else {
            bug("Bad PermaPK value in profile.txt - ignored.", 0);
          }

        }

       /* PermaDeath */
   
        if (!strcmp(kwd, "PermaDeath")) {
          ok = TRUE;
          opt = fread_word(fp);

          if (!strcmp(opt, "YES")) {
            bug("Mud booting in Perma-death mode!", 0);
            SET_BIT(mud.flags, MUD_PERMADEATH); 
          } else if (!strcmp(opt, "NO")) {
            bug("Mud booting in immortal mode!", 0);
            REMOVE_BIT(mud.flags, MUD_PERMADEATH); 
          } else {
            bug("Bad PermaDeath value in profile.txt - ignored.", 0);
          }

        }


        if (!strcmp(kwd, "PKTimer")) {
          ok = TRUE;
          mud.pk_timer = fread_number(fp);
          if (mud.pk_timer < 0 || mud.pk_timer > 99) mud.pk_timer = 15;
        }

        break; 

     /* Recall, Respawn */

      case 'R':
     
       /* Recall vnum */
   
        if (!strcmp(kwd, "Recall")) {
          ok = TRUE;
          mud.recall = fread_number(fp);

          if ( !room_ok_for_recall(mud.recall) ) {
            bug("Bad recall room (%d) in profile file!", mud.recall);
            exit(1);
          }
        }

       /* Respawn vnum */
   
        if (!strcmp(kwd, "Respawn")) {
          ok = TRUE;
          mud.respawn = fread_number(fp);

          if ( !room_ok_for_recall(mud.respawn) ) {
            bug("Bad respawn room (%d) in profile file!", mud.respawn);
            exit(1);
          }
        }

        if (!strcmp(kwd, "Reboot")) {
          ok = TRUE;
          mud.reboot = fread_number(fp);
          if (mud.reboot < 0 || mud.reboot > 23) mud.reboot = -1;
        }

        if (!strcmp(kwd, "Recover")) {
          ok = TRUE;
          opt = fread_word(fp);

          if (!strcmp(opt, "YES")) {
            SET_BIT(mud.flags, MUD_RECOVER); 
          } else if (!strcmp(opt, "NO")) {
            REMOVE_BIT(mud.flags, MUD_RECOVER); 
          }
        }

        break; 

      case 'S':
     
       /* Start vnum */
   
        if (!strcmp(kwd, "Start")) {

          ok = TRUE;

          mud.start = fread_number(fp);

          if ( !room_ok_for_recall(mud.start) ) {
            bug("Bad start room (%d) in profile file!", mud.start);
            exit(1);
          }
        }

        break; 

      case 'U':

        if (!strcmp(kwd, "URL")) {
          ok = TRUE;
          mud.url = strdup(fread_word(fp));
        }
        break;

     /* Wizlock */

      case 'W':
     
       /* WizLock YES|NO */
   
        if (!strcmp(kwd, "WizLock")) {

          ok = TRUE;

          opt = fread_word(fp);

          if (!strcmp(opt, "YES")) {
            bug("Mud booting in WIZLOCK mode!", 0);
            SET_BIT(mud.flags, MUD_WIZLOCK); 
          } else if (strcmp(opt, "NO")) {
            bug("Bad WizLock value is profile.txt - ignored.", 0);
          }

        }

        break; 

     /* Zone */

      case 'Z':
     
       /* Zone zid 'name' recall respawn morgue flags */
   
        if (!strcmp(kwd, "Zone")) {

          ok = TRUE;

          i = fread_number(fp);
 
          if ( i < 0 
            || i >= NUM_ZONES ) {
            bug("Bad zone id (%d) in profile file.", i);
            exit(1);
          }

          zones[i].zid = i;

          free_string(zones[i].name);
          zones[i].name = strdup(fread_word(fp));

          zones[i].recall = fread_number(fp);

          if (  zones[i].recall != 0
            && !room_ok_for_recall(zones[i].recall)) { 
            bug("Invalid zone recall room (%d) in profile file.",
                                                           zones[i].recall);
            exit(1);
          }

          zones[i].respawn = fread_number(fp);

          if (  zones[i].respawn != 0
            && !room_ok_for_recall(zones[i].respawn)) { 
            bug("Invalid zone respawn room (%d) in profile file.",
                                                          zones[i].respawn);
            exit(1);
          }

          zones[i].morgue = fread_number(fp);

          if (  zones[i].morgue != 0
            && !room_ok_for_recall(zones[i].morgue)) { 
            bug("Invalid zone morgue room (%d) in profile file.",
                                                           zones[i].morgue);
            exit(1);
          }

          zones[i].dream = fread_number(fp);

          if (  zones[i].dream != 0
            && !room_ok_for_recall(zones[i].dream)) { 
            bug("Invalid zone dream room (%d) in profile file.",
                                                           zones[i].dream);
            exit(1);
          }

          zones[i].flags = fread_number(fp);

          free_string(zones[i].map);
          zones[i].map = strdup(fread_word(fp));
 
          zones[i].loaded = TRUE; 
        }

        break; 

     /* Anything else is an error... */

      default:
        break;
    }

    if (!ok) {
      sprintf(buf, "Unrecognized keyword in profile file: %s", kwd);
      bug(buf, 0); 
      exit(1);
    }
 
  }

 /* Close it now we're finished with it... */ 

  fclose(fp);

 /* All done... */

  log_string("...profile loaded");

  return;
}

void do_zlist(CHAR_DATA *ch, char *args) {

    char buf    [ MAX_STRING_LENGTH ];
    char result [ MAX_STRING_LENGTH*4 ];	/* May need tweaking. */

    int i;

    bool show;
    char *zcol;

    if ( IS_IMMORTAL(ch) ) {
      strcpy( result, 
            "Zid Name                        Recall Respawn Morgue  Dream\r\n"
            "--- --------------------------  ------ ------- ------  -----\r\n");
    } else {
      strcpy( result, 
            "Zid Name                      \r\n"
            "--- --------------------------\r\n");
    }

    for ( i = 0; i < NUM_ZONES; i++ ) {

        show = zones[i].loaded;
        zcol = "{g"; 

        if ( IS_SET(zones[i].flags, ZONE_SECRET) ) {

          zcol = "{r";

          if ( !IS_IMMORTAL(ch) ) {

            show = FALSE;
 
            if ( ch->in_room != NULL
              && ch->in_room->area != NULL 
              && ch->in_room->area->zone == i ) {
              show = zones[i].loaded;
              zcol = "{g"; 
            }
          }
        }

        if (show) {

          if ( IS_IMMORTAL(ch) ) {
            sprintf(buf, "{y%3d %s%-26.26s{x   %5ld   %5ld  %5ld  %5ld\r\n",
	                  zones[i].zid,
                          zcol,
                          zones[i].name,
                          zones[i].recall,
                          zones[i].respawn,
                          zones[i].morgue,  
                          zones[i].dream ); 
          } else {
            sprintf(buf, "{y%3d %s%-26.26s{x\r\n",
                          zones[i].zid,
                          zcol,
                          zones[i].name);
          }

          strcat(result, buf);

        }
    }

    if ( ch->in_room == NULL
      || ch->in_room->area == NULL ) {
      strcat(result, "\r\n{RYou are not currently in any zone!{x\r\n"); 
    } else {
      sprintf(buf, "You are currently in zone {y%d{x - {g%s{x\r\n",
                    zones[ch->in_room->area->zone].zid,
                    zones[ch->in_room->area->zone].name);
      strcat(result, buf);
    }
 
    page_to_char( result, ch );

    return;

}

/* Remove zone flags from ALL objects... */

void do_zclear(CHAR_DATA *ch, char *args) {

  char buf[MAX_STRING_LENGTH];

  int zid;
  int zmask;
  int i;
  int count;

  OBJ_INDEX_DATA *obj;

 /* Extract zone id from parms... */

  if ( args[0] == '\0' ) {
    send_to_char("Syntax: zclear zone_id\r\n", ch);
    return;
  }

  zid = atoi(args);

  if ( zid < 0
    || zid >= NUM_ZONES ) {
    send_to_char("Invalid zone id!\r\n", ch);
    return;
  }

 /* Prepare mask... */

  zmask = (0xFFFFFFFF ^ zones[zid].mask);

 /* Advise... */

  sprintf(buf, "{YRemoving tolerence for zone %d from ALL objects!{x\r\n",
                zid);
  send_to_char(buf, ch);

 /* Chew through the object indexes... */

  count = 0;

  for (i = 0; i < MAX_KEY_HASH; i++ ) {

    obj = obj_index_hash[i];

    if (obj != NULL) {
      if ( obj->zmask != 0 ) {
        obj->zmask &= zmask;
        count += 1; 
      }
    }
  }

 /* Ok, done... */

  sprintf(buf, "{YDone - %d objects changed.{x\r\n", count);
  send_to_char(buf, ch);

  send_to_char("{YASAVE WORLD and REBOOT to complete.{x\r\n", ch);

  return;
}

/* Copy flags for a zone on ALL objects... */

void do_zcopy(CHAR_DATA *ch, char *args) {

  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];

  int zid;
  int zid1; 
  int zmask;
  int zmask1;
  int i;
  int count;

  OBJ_INDEX_DATA *obj;

 /* Extract zone ids from parms... */

  if ( args[0] == '\0' ) {
    send_to_char("Syntax: zcopy source_zone_id target_zone_id\r\n", ch);
    return;
  }

  args = one_argument(args, arg1);

  zid  = atoi(arg1);
  zid1 = atoi(args);

  if ( zid < 0
    || zid >= NUM_ZONES ) {
    send_to_char("Invalid source zone id!\r\n", ch);
    return;
  }

  if ( zid1 < 0
    || zid1 >= NUM_ZONES ) {
    send_to_char("Invalid target zone id!\r\n", ch);
    return;
  }

 /* Prepare mask... */

  zmask  = zones[zid ].mask;
  zmask1 = zones[zid1].mask;

 /* Advise... */

  sprintf(buf, 
         "{YCopying zone %d tolerence to zone %d from ALL objects!{x\r\n",
          zid,
          zid1);
  send_to_char(buf, ch);

 /* Chew through the object indexes... */

  count = 0;

  for (i = 0; i < MAX_KEY_HASH; i++ ) {

    obj = obj_index_hash[i];

    if (obj != NULL) {
      if ( obj->zmask != 0 ) {
        if (IS_SET(obj->zmask, zmask)) {
          obj->zmask |= zmask1;
          count += 1; 
        }
      }
    }
 
  }

 /* Ok, done... */

  sprintf(buf, "{YDone - %d objects changed.{x\r\n", count);
  send_to_char(buf, ch);

  send_to_char("{YASAVE WORLD and REBOOT to complete.{x\r\n", ch);

  return;
}


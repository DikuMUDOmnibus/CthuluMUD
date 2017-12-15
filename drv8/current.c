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
#include "current.h"
#include "doors.h"
#include "econd.h"
#include "mob.h"
#include "wev.h"
#include "olc.h"

DECLARE_DO_FUN(do_look);

static CURRENT *free_current_chain = NULL;

CURRENT *get_current() {

  CURRENT *new_current;

  if (free_current_chain == NULL) {
    new_current = (CURRENT *) alloc_perm(sizeof(*new_current));
  } else {
    new_current = free_current_chain;
    free_current_chain = new_current->next;
  }

  new_current->seq = 0;
  new_current->name = NULL;
  new_current->destination = NULL;
  new_current->dir = 0;
  new_current->action = NULL;
  new_current->next = NULL;
  new_current->cond = NULL;

  return new_current;
}

void free_current(CURRENT *old_current) {

  ECOND_DATA *ec;

  if (old_current->next != NULL) {
    free_current(old_current->next);
    old_current->next = NULL;
  }

  if (old_current->name != NULL) {
    free_string(old_current->name);
    old_current->name = NULL;
  }

  if (old_current->destination != NULL) {
    free_string(old_current->destination);
    old_current->destination = NULL;
  }

  if (old_current->action != NULL) {
    free_string(old_current->action);
    old_current->action = NULL;
  }

  while (old_current->cond != NULL) {
    ec = old_current->cond->next;
    free_ec(old_current->cond);
    old_current->cond = ec;
  }

  old_current->next = free_current_chain;
  free_current_chain = old_current;

  return;
}

struct cdir_mapping {
  char		*name;
  int		 dir;
} cdir_map[] = { 
  {	"north", 	DIR_NORTH	},
  {	"south", 	DIR_SOUTH	},
  {	"east", 	DIR_EAST	},
  {	"west", 	DIR_WEST	},
  {	"northeast", 	DIR_NE		},
  {	"northwest", 	DIR_NW		},
  {	"southeast", 	DIR_SE		},
  {	"southwest", 	DIR_SW		},
  {	"neast", 	DIR_NE		},
  {	"nwest", 	DIR_NW		},
  {	"seast", 	DIR_SE		},
  {	"swest", 	DIR_SW		},
  {	"ne", 		DIR_NE		},
  {	"nw", 		DIR_NW		},
  {	"se", 		DIR_SE		},
  {	"sw", 		DIR_SW		},
  {	"up", 		DIR_UP		},
  {	"down", 	DIR_DOWN	},
  {	"", 		DIR_OTHER	},
}; 

int get_current_dir(char *destination) {

  int i;

  i = 0;

  while (cdir_map[i].dir != DIR_OTHER) {
    if (!str_cmp(cdir_map[i].name, destination)) {
      return cdir_map[i].dir;
    }
    i += 1;
  }

  return DIR_OTHER;
}

CURRENT *create_current(int 	 seq,
 			char 	*name, 
			char 	*action, 
			char 	*destination, 
			int 	dir) {

  CURRENT *nc;

  nc = get_current();

  nc->seq = seq;

  nc->name = strdup(name);
  nc->destination = strdup(destination);
  nc->action = strdup(action);

  nc->dir = dir;  

  return nc;
}

/* Insert (with replace) a current into a chain... */

CURRENT *insert_current(CURRENT *chain, CURRENT *new_current) {

  CURRENT *next, *last;

  if (chain == NULL) {
    return new_current;
  }

  if (new_current == NULL) {
    return chain;
  }

 /* Easy case first... */

  if (new_current->seq <= chain->seq) {
 
    if (new_current->seq == chain->seq) {
      new_current->next = chain->next;
      chain->next = NULL;
      free_current(chain);
      return new_current;
    }

    new_current->next = chain;

    return new_current; 
  } 

 /* Search time... */

  last = chain;
  next = chain->next;

  while ( next != NULL
       && next->seq > new_current->seq) {

    last = next;
    next = next->next; 
  }

 /* End of chain? */
 
  if (next == NULL) {
    last->next = new_current;
    new_current->next = NULL;
    return chain;
  } 

 /* Insert into chain... */

  if (next->seq == new_current->seq) {
    last->next = new_current;
    new_current->next = next->next;
    next->next = NULL;
    free_current(next);
    return chain;
  }

  last->next = new_current;
  new_current->next = next;

  return chain;
}

/* Find a current from a chain... */

CURRENT *find_current(CURRENT *chain, int seq) {

  CURRENT *pos, *last;

  if (chain == NULL) {
    return NULL;
  }

 /* Search for the record... */

  last = NULL;
  pos = chain;

  while ( pos != NULL
       && pos->seq < seq ) {
    last = pos;
    pos = pos->next;
  }

 /* Not found */
 
  if (pos == NULL) {
    return NULL;
  }

  if (pos->seq != seq) {
    return NULL;
  } 

  return pos;
}

/* Delete a current from a chain... */

CURRENT *delete_current(CURRENT *chain, int seq) {

  CURRENT *pos, *last;

  if (chain == NULL) {
    return chain;
  }

 /* Search for the record... */

  last = NULL;
  pos = chain;

  while ( pos != NULL
       && pos->seq < seq ) {
    last = pos;
    pos = pos->next;
  }

 /* Not found */
 
  if (pos == NULL) {
    return chain;
  }

  if (pos->seq != seq) {
    return chain;
  } 

 /* Head of chain... */
 
  if (last == NULL) {
    chain = pos->next;
    pos->next = NULL;
    free_current(pos);
    return chain;
  }

 /* Remove from chain... */

  last->next = pos->next;
  pos->next = NULL;
  free_current(pos);

  return chain;
}

/* Save a current to a file... */

void save_current(CURRENT *cur, FILE *fp, char *hdr, char *hdr2) {

  ECOND_DATA *ec;

  fprintf(fp, "%s %d %d '%s' '%s' '%s'\n",
               hdr, 
               cur->seq,
               cur->dir,
               cur->name,
               cur->action,
               cur->destination);

  ec = cur->cond;
 
  while (ec != NULL) {

    write_econd(ec, fp, hdr2);

    ec = ec->next;
  }

  if (cur->next != NULL) {
    save_current(cur->next, fp, hdr, hdr2);
  }

  return;
}

CURRENT *read_current(FILE *fp) {

  CURRENT *nc;

  nc = get_current();

  nc->seq = fread_number(fp);
  nc->dir = fread_number(fp);  

  nc->name = strdup(fread_word(fp));
  nc->action = strdup(fread_word(fp));
  nc->destination = strdup(fread_word(fp));

  return nc;
}

/* Apply a rooms currents to a character... */

void apply_current(ROOM_INDEX_DATA *room, CHAR_DATA *ch) {

  CURRENT *current;

  MOB_CMD_CONTEXT *mcc;

 /* No current, no search... */

  if (room->currents == NULL) {
    return;
  }

 /* Find the first current which affects them... */

  current = room->currents;

  mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL);

  while ( current != NULL 
       && !ec_satisfied_mcc(mcc, current->cond, TRUE)) { 

    current = current->next;
  }

  free_mcc(mcc);

 /* No current means we're done... */

  if (current == NULL) {
    return;
  } 

 /* Must have a sensible destination... */

  if (current->dir >= DIR_MAX) {
    return;
  }

 /* Now schedule the current command... */

  enqueue_mob_cmd(ch, "current", 0, 0);

 /* All done... */

  return;
}

/* Move according to the current... */

void do_currents(CHAR_DATA *ch, char *args) {

  CURRENT *current;

  char buf_you[MAX_STRING_LENGTH];
  char buf_obs[MAX_STRING_LENGTH];

  EXIT_DATA *pexit;

  ROOM_INDEX_DATA *room;
  ROOM_INDEX_DATA *to_room;

  MOB_CMD_CONTEXT *mcc;
  WEV *wev;

 /* No current, no search... */

  room = ch->in_room;

  if (room->currents == NULL) {
    return;
  }

 /* Find the first current which affects them... */

  current = room->currents;

  mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL);

  while ( current != NULL && !ec_satisfied_mcc(mcc, current->cond, TRUE)) { 
    current = current->next;
  }

  free_mcc(mcc);

 /* No current means we're done... */

  if (current == NULL) return;

 /* Must have a sensible destination... */

  if (current->dir >= DIR_MAX) return;

  pexit = room->exit[current->dir];

  if (pexit == NULL) return;
  if (!exit_visible(ch, pexit)) return;

  to_room = get_exit_destination(ch, room, pexit, TRUE);

  if (to_room == NULL) return;
  if (!can_see_room(ch, to_room)) return;
  if (room == to_room) return;
  if (IS_SET(pexit->exit_info, EX_CLOSED)) return;

  if ( (ch->in_room->area != NULL)
    && (to_room->area != NULL) 
    && (!IS_SET(ch->in_room->area->area_flags, AREA_LOCKED)) 
    && ( IS_SET(to_room->area->area_flags, AREA_LOCKED)) ) {

    if (!IS_IMMORTAL(ch)) return;
    send_to_char("{MYou feel a tingle crawl across your flesh!{x\r\n", ch); 

  }

 /* Print up the descriptions... */

  sprintf(buf_you, "{Y%s %s you %s!\r\n\r\n{x", current->name, current->action, current->destination);
  sprintf(buf_obs, "{Y%s %s @a2 %s!\r\n\r\n{x", current->name,  current->action, current->destination);

  mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, current->dir, dir_name[current->dir]);
  wev = get_wev(WEV_DEPART, WEV_DEPART_CURRENT, mcc,
                buf_you,
                NULL,
                buf_obs); 

 /* Say goodbye (no challange)... */

  room_issue_wev(room, wev);

  free_wev(wev);
 
 /* Do the move... */

  char_from_room( ch );

  char_to_room( ch, to_room );

 /* Work out where we came from... */

  mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL); 
  wev = get_wev(WEV_ARRIVE, WEV_ARRIVE_CURRENT, mcc,
                NULL,
                NULL,
               "{Y@a2 is swept in.{x\r\n");

  room_issue_wev(to_room, wev);

  free_wev(wev);

 /* Have a look around now we've got there... */

  do_look( ch, "auto" );
  return;
}

/* Interactive creation... */

void redit_current_list(CHAR_DATA *ch, char *args, ROOM_INDEX_DATA *room) {

  CURRENT *cur;

  char buf[MAX_STRING_LENGTH];

  if (args[0] != '\0') {
    redit_current(ch, "");
    return;
  }

  cur = room->currents;

  if (cur == NULL) {
    send_to_char("No currents defined.\r\n", ch);
    return;
  }

  send_to_char("Seq    Dir    Name; action; destination\r\n"
               "-----  -----  --------------------------------------\r\n", ch);

  while (cur != NULL) {

    sprintf(buf, "%5d  %5d  %s; %s; %s\r\n", cur->seq, cur->dir, cur->name, cur->action, cur->destination);
    send_to_char(buf, ch);

    if (cur->cond != NULL) print_econd(ch, cur->cond, " ");
    cur = cur->next;
  }

  return;
}

void redit_current_add(CHAR_DATA *ch, char *args, ROOM_INDEX_DATA *room) {

  CURRENT *cur;

  char sseq[MAX_INPUT_LENGTH];
  char sdir[MAX_INPUT_LENGTH];
  char name[MAX_INPUT_LENGTH];
  char act[MAX_INPUT_LENGTH];
  char dest[MAX_INPUT_LENGTH];

  int seq, dir;

  if (args[0] == '\0') {
    redit_current(ch, "");
    return;
  }

  args = one_argument(args, sseq);
  args = one_argument(args, sdir);
  args = one_argument(args, name);
  args = one_argument(args, act);
  args = one_argument(args, dest);

  if ( dest[0] == '\0' 
    || args[0] != '\0' ) {
    redit_current(ch, "");
    return;
  }  

  seq = atoi(sseq);
  dir = atoi(sdir);

  if ( seq < 1) {
    send_to_char("Seq must be > 0\r\n", ch);
    return;
  }

  if ( dir < 0
    || dir >= DIR_MAX ) {
    send_to_char("Invalid direction. Use 0..9\r\n", ch);
    return;
  } 

  cur = create_current(seq, name, act, dest, dir);

  room->currents = insert_current(room->currents, cur);

  send_to_char("Current added...\r\n", ch);

  return;
}

void redit_current_delete(CHAR_DATA *ch, char *args, ROOM_INDEX_DATA *room) {

  char sseq[MAX_INPUT_LENGTH];

  int seq;

  if (args[0] == '\0') {
    redit_current(ch, "");
    return;
  }

  args = one_argument(args, sseq);

  seq = atoi(sseq);

  if ( seq < 1) {
    send_to_char("Seq must be > 0\r\n", ch);
    return;
  }

  room->currents = delete_current(room->currents, seq);

  send_to_char("Current deleted...\r\n", ch);

  return;
}

void redit_current_cond(CHAR_DATA *ch, char *args, ROOM_INDEX_DATA *room) {

  CURRENT *cur;
  ECOND_DATA *ec;

  char sseq[MAX_INPUT_LENGTH];
  char *cond;

  int seq;

  if (args[0] == '\0') {
    redit_current(ch, "");
    return;
  }

  cond = one_argument(args, sseq);

  if ( cond[0] == '\0' ) { 
    redit_current(ch, "");
    return;
  }  

  seq = atoi(sseq);

  if ( seq < 1) {
    send_to_char("Seq must be > 0\r\n", ch);
    return;
  }
 
  cur = find_current(room->currents, seq);

  if (cur == NULL) {
    send_to_char("No such current defined.\r\n", ch);
    return;
  }

  if (!str_cmp(cond, "none")) {

    while (cur->cond != NULL) {
      ec = cur->cond;
      cur->cond = ec->next;
      ec->next = NULL;
      free_ec(ec);
    }

    send_to_char("Current conditions removed.\r\n", ch);

    return;
  }

  ec = create_econd(cond, ch);

  if (ec == NULL) {
    return;
  }

  ec->next = cur->cond;
  cur->cond = ec;

  send_to_char("Current conditon added...\r\n", ch);

  return;
}

void redit_current_renum(CHAR_DATA *ch, char *args, ROOM_INDEX_DATA *room) {

  CURRENT *cur;

  int seq;

  if (args[0] != '\0') {
    redit_current(ch, "");
    return;
  }

  cur = room->currents;

  if (cur == NULL) {
    send_to_char("No currents defined.\r\n", ch);
    return;
  }

  seq = 10;

  while (cur != NULL) {

    cur->seq = seq;

    seq += 10;

    cur = cur->next;
  }

  send_to_char("Currents renumbered.\r\n", ch);

  return;
}

bool redit_current(CHAR_DATA *ch, char *args) {

  ROOM_INDEX_DATA *pRoom;

  char cmd[MAX_INPUT_LENGTH];

  EDIT_ROOM(ch, pRoom);

  if ( args[0] == '\0'
    || args[0] == '?' ) {

    send_to_char(
        "Syntax: current list\r\n"
        "        current add [seq] [dir] [name] [action] [dest]\r\n"
        "        current cond [seq] [condition]\r\n"
        "        current renum\r\n"
        "        current delete [seq]\r\n", ch); 

    return FALSE;
  }

  args = one_argument(args, cmd);

  if (!str_cmp(cmd, "list")) {
    redit_current_list(ch, args, pRoom);
    return FALSE;
  } else if (!str_cmp(cmd, "add")) {
    redit_current_add(ch, args, pRoom);
    return TRUE;
  } else if (!str_cmp(cmd, "cond")) {
    redit_current_cond(ch, args, pRoom);
    return TRUE;
  } else if (!str_cmp(cmd, "renum")) {
    redit_current_renum(ch, args, pRoom);
    return TRUE;
  } else if (!str_cmp(cmd, "delete")) {
    redit_current_delete(ch, args, pRoom);
    return TRUE;
  } else {
    redit_current(ch, "");
  }

  return FALSE;
}


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
#include "gadgets.h"
#include "econd.h"
#include "mob.h"
#include "wev.h"

/* Gadget transform data management... */

static GADGET_TRANS_DATA *free_gdt_chain = NULL;

GADGET_TRANS_DATA *get_gdt() {

  GADGET_TRANS_DATA *new_gdt;

  if (free_gdt_chain == NULL) {
    new_gdt = (GADGET_TRANS_DATA *) alloc_perm(sizeof(*new_gdt));
  } else {
    new_gdt = free_gdt_chain;
    free_gdt_chain = new_gdt->next;
  }

  new_gdt->next = NULL;
  new_gdt->seq = 0;
  new_gdt->action = 0;
  new_gdt->in = 0;
  new_gdt->out = 0;
  new_gdt->message = NULL;
  new_gdt->cond = NULL;
 
  return new_gdt;
}

void free_gdt(GADGET_TRANS_DATA *gdt) {

  ECOND_DATA *ec;

  if (gdt == NULL) {
    return;
  }

  if (gdt->next != NULL) {
    free_gdt(gdt->next);
    gdt->next = NULL;
  }

  if (gdt->message != NULL) {
    free_string(gdt->message);
    gdt->message = NULL;
  }

  if (gdt->cond != NULL) {

    while (gdt->cond != NULL) {
      ec = gdt->cond->next;
      free_ec(gdt->cond);
      gdt->cond = ec;
    }
  }

  gdt->next = free_gdt_chain;
  free_gdt_chain = gdt;

  return;
}

void delete_gdt(GADGET_DATA *gadget, int seq) {

  GADGET_TRANS_DATA *gdt, *old_gdt;

  if (gadget == NULL) {
    return;
  }

  gdt = gadget->gdt;
  old_gdt = NULL;

  if (gdt == NULL) {
    return;
  }

  while ( gdt != NULL
       && gdt->seq < seq) {

    old_gdt = gdt;    
    gdt = gdt->next;
  } 

  if ( gdt == NULL
    || gdt->seq != seq) {
    return;
  }

  if (old_gdt == NULL) {
    gadget->gdt = gdt->next;
  } else {
    old_gdt->next = gdt->next;
  }    

  gdt->next = NULL;

  free_gdt(gdt);

  return;
}

void insert_gdt(GADGET_TRANS_DATA *new_gdt, GADGET_DATA *gadget) {

  GADGET_TRANS_DATA *gdt;

  if ( new_gdt == NULL
    || gadget == NULL) {
    return;
  }

  if (gadget->gdt == NULL) {
    gadget->gdt = new_gdt;
    return;
  }

  delete_gdt(gadget, new_gdt->seq);

  gdt = gadget->gdt;

  if (gdt->seq > new_gdt->seq) {
    new_gdt->next = gdt;
    gadget->gdt = new_gdt;
    return;
  }

  while ( gdt->next != NULL
       && gdt->seq < new_gdt->seq) {
    
    gdt = gdt->next;
  } 

  new_gdt->next = gdt->next;
  gdt->next = new_gdt;

  return;
}

GADGET_TRANS_DATA *find_gdt_by_seq(GADGET_DATA *gadget, int seq) {

  GADGET_TRANS_DATA *gdt;

  gdt = gadget->gdt;

  while (gdt != NULL) {

    if (gdt->seq == seq) {
      return gdt;
    }

    gdt = gdt->next;
  }

  return gdt;
}

/* Gadget data management... */
 
static GADGET_DATA *free_gadget_chain = NULL;

GADGET_DATA *get_gadget() {

  GADGET_DATA *new_gadget;

  if (free_gadget_chain == NULL) {
    new_gadget = (GADGET_DATA *) alloc_perm(sizeof(*new_gadget));
  } else {
    new_gadget = free_gadget_chain;
    free_gadget_chain = new_gadget->next;
  }

  new_gadget->next = NULL;
  new_gadget->state = 0;
  new_gadget->link_room = 0;
  new_gadget->link_id = 0;
  new_gadget->name = NULL;
  new_gadget->gdt = NULL;

  return new_gadget;
}

void free_gadget(GADGET_DATA *gadget) {

  if (gadget == NULL) {
    return;
  }

  if (gadget->next != NULL) {
    free_gadget(gadget->next);
    gadget->next = NULL;
  } 

  if (gadget->name != NULL) {
    free_string(gadget->name);
    gadget->name = NULL;
  }

  if (gadget->gdt != NULL) { 
    free_gdt(gadget->gdt);
    gadget->gdt = NULL;
  }

  gadget->next = free_gadget_chain;
  free_gadget_chain = gadget;

  return;
}

GADGET_DATA *find_gadget_by_name(ROOM_INDEX_DATA *room, char *name) {

  GADGET_DATA *gadget;

  if (room->gadgets == NULL) {
    return NULL;
  }

  gadget = room->gadgets;

  while (gadget != NULL) {

    if (is_name(name, gadget->name)) {
      return gadget;
    }

    gadget = gadget->next;
  }

  return NULL;
}

GADGET_DATA *find_gadget_by_id(ROOM_INDEX_DATA *room, int id) {

  GADGET_DATA *gadget;

  if (room->gadgets == NULL) {
    return NULL;
  }

  gadget = room->gadgets;

  while (gadget != NULL) {

    if (gadget->id == id) {
      return gadget;
    }

    gadget = gadget->next;
  }

  return NULL;
}

static char *gad_text[GADGET_ACTION_MAX] = {
	"You do something strange with %s!\r\n%s\r\n",
        "You pull %s.\r\n%s\r\n",
        "You push %s.\r\n%s\r\n",
        "You twist %s.\r\n%s\r\n",
        "You turn %s.\r\n%s\r\n",
        "You turnback %s.\r\n%s\r\n",
        "You move %s.\r\n%s\r\n",
        "You lift %s.\r\n%s\r\n",
        "You press %s.\r\n%s\r\n",
        "You dig in %s.\r\n%s\r\n" 
};

static char *gad_text2[GADGET_ACTION_MAX] = {
	"@a2 does something strange with %s!\r\n%s\r\n",
        "@a2 pulls %s.\r\n%s\r\n",
        "@a2 pushes %s.\r\n%s\r\n",
        "@a2 twists %s.\r\n%s\r\n",
        "@a2 turns %s.\r\n%s\r\n",
        "@a2 turnsback %s.\r\n%s\r\n",
        "@a2 moves %s.\r\n%s\r\n",
        "@a2 lifts %s.\r\n%s\r\n",
        "@a2 presses %s.\r\n%s\r\n",
        "@a2 digs in %s.\r\n%s\r\n",
};

void do_gadget(CHAR_DATA *ch, int action, char *name) {
  GADGET_DATA *gadget;
  GADGET_TRANS_DATA *gdt;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char gnam[MAX_STRING_LENGTH];
  MOB_CMD_CONTEXT *mcc;
  WEV *wev;

 /* Must be in a room... */

  if (ch->in_room == NULL) {
    send_to_char("No here for anything to be!\r\n", ch);
    return;
  }

 /* Stop them doing unnatural things... */

  if ( action < 1 
    || action >= GADGET_ACTION_MAX) {
    send_to_char("That's disgusting!\r\n", ch);   
    return;
  }

 /* See if we can find the gadget... */ 

  gadget = find_gadget_by_name(ch->in_room, name);

  if (gadget == NULL) {
    send_to_char("Nothing manipulable matching that name.\r\n", ch);
    return;
  }

  one_argument(gadget->name, gnam);

 /* Ok, now we got to find a matching transition... */

  mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, action, gnam);

  gdt = gadget->gdt;

  while (gdt != NULL) {

    if ( ( gadget->state == gdt->in
        || gdt->in == -1 )
      && action == gdt->action
      && ec_satisfied_mcc(mcc, gdt->cond, TRUE)) {
      break;
    }

    gdt = gdt->next;
  }

 /* Did we find one? */

  if (gdt == NULL) {
    sprintf(buf, gad_text[action], gnam, "It won't move.");
    sprintf(buf2, gad_text2[action], gnam, "It' won't move.");
    wev = get_wev(WEV_GADGET, WEV_GADGET_BAD, mcc, buf, buf2, buf2);
  } else {
    sprintf(buf, gad_text[action], gnam, gdt->message);
    sprintf(buf2, gad_text2[action], gnam, gdt->message);
    wev = get_wev(WEV_GADGET, WEV_GADGET_OK, mcc, buf, buf2, buf2);
  }

 /* Challange... */
 
  if (!room_issue_wev_challange(ch->in_room, wev)) {
    free_wev(wev);
    return;
  }

 /* Make the state change... */

  if (gdt != NULL) {
     if (gdt->out != -1) {
        gadget->state = gdt->out;
        if (gadget->link_room >0
        && gadget->link_id >0) {
             GADGET_DATA *gadget2;
             ROOM_INDEX_DATA *room2;

             if ((room2 = get_room_index(gadget->link_room)) != NULL) {
                   if ((gadget2 = find_gadget_by_id(room2, gadget->link_id)) != NULL) {
                          gadget2->state = gadget->state;
                   }
             }  
        }
     }
  }

 /* Issue the wev for real... */

  room_issue_wev(ch->in_room, wev);

 /* Free the wev... */

  free_wev(wev);

 /* All done... */

  return;
}

void do_gadget_pull(CHAR_DATA *ch, char *args) {

  do_gadget(ch, GADGET_ACTION_PULL, args);

  return;
}

void do_gadget_push(CHAR_DATA *ch, char *args) {

  do_gadget(ch, GADGET_ACTION_PUSH, args);

  return;
}

void do_gadget_twist(CHAR_DATA *ch, char *args) {

  do_gadget(ch, GADGET_ACTION_TWIST, args);

  return;
}

void do_gadget_turn(CHAR_DATA *ch, char *args) {

  do_gadget(ch, GADGET_ACTION_TURN, args);

  return;
}

void do_gadget_turnback(CHAR_DATA *ch, char *args) {

  do_gadget(ch, GADGET_ACTION_TURNBACK, args);

  return;
}

void do_gadget_move(CHAR_DATA *ch, char *args) {

  do_gadget(ch, GADGET_ACTION_MOVE, args);

  return;
}

void do_gadget_lift(CHAR_DATA *ch, char *args) {

  do_gadget(ch, GADGET_ACTION_LIFT, args);

  return;
}

void do_gadget_press(CHAR_DATA *ch, char *args) {

  do_gadget(ch, GADGET_ACTION_PRESS, args);

  return;
}

void do_gadget_digin(CHAR_DATA *ch, char *args) {

 OBJ_DATA *obj;

 /* Check for digging... */

    obj = ch->carrying;

    while ( obj != NULL
         && ( obj->item_type != ITEM_DIGGING
           || obj->wear_loc != -1
           || !can_see_obj(ch, obj) ) ) {
      obj = obj->next_content;
    }

    if ( obj == NULL ) {
      obj = ch->in_room->contents;

      while ( obj != NULL
           && ( obj->item_type != ITEM_DIGGING
             || !can_see_obj(ch, obj) ) ) {
        obj = obj->next_content;
      }
    }

    if ( obj == NULL ) {
       send_to_char("You have no suitable digging object!\r\n", ch);
       return;
    }

  do_gadget(ch, GADGET_ACTION_DIGIN, args);

  return;
}

/* File I/O Routines... */

void write_gadget(FILE *fp, GADGET_DATA *gadget) {
  GADGET_TRANS_DATA *gdt;

  if (gadget == NULL) {
    return;
  }

  if (gadget->next != NULL) {
    write_gadget(fp, gadget->next);
  }

  fprintf(fp, "G %d %s~\r\n", gadget->id, gadget->name);

  if (gadget->link_room >0
  && gadget->link_id > 0) {  
      fprintf(fp, "L %ld %d\r\n", gadget->link_room, gadget->link_id);
  }

  gdt = gadget->gdt;

  while (gdt != NULL) {

    fprintf(fp, "T %d %d %d %d %s~\r\n", gdt->seq, gdt->action, gdt->in, gdt->out, gdt->message);

    if (gdt->cond != NULL) {
      write_econd(gdt->cond, fp, "A"); 
    }
 
    gdt = gdt->next;
  }

  return;
}

GADGET_DATA *read_gadget(FILE *fp) {
  GADGET_DATA *new_gadget;

  new_gadget = get_gadget();

  new_gadget->id = fread_number(fp);
  new_gadget->name = fread_string(fp);

  new_gadget->state = 0;
  new_gadget->link_room = 0;
  new_gadget->link_id = 0;
  new_gadget->gdt = NULL;
  new_gadget->next = NULL;

  return new_gadget;
}

GADGET_TRANS_DATA *read_gdt(FILE *fp) {
  GADGET_TRANS_DATA *new_gdt;

  new_gdt = get_gdt();

  new_gdt->seq		= fread_number(fp);
  new_gdt->action	= fread_number(fp);
  new_gdt->in		= fread_number(fp);
  new_gdt->out		= fread_number(fp);

  new_gdt->message	= fread_string(fp);

  new_gdt->next = NULL;
  new_gdt->cond = NULL;

  return new_gdt;
}


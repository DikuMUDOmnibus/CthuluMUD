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
#include "chat.h"
#include "mob.h"
#include "olc.h"
#include "triggers.h"

static CHAT_DATA *free_chat_chain = NULL;

CHAT_DATA *get_chat() {

  CHAT_DATA *new_chat;
  int i;

  if ( free_chat_chain != NULL ) {
    new_chat = free_chat_chain;
    free_chat_chain = free_chat_chain->next;
  } else {
    new_chat = (CHAT_DATA *) alloc_perm( sizeof( *new_chat ));
  }

  new_chat->next = NULL;
  new_chat->chance = 0;
  new_chat->state = 0;
  
  for( i = 0; i < MAX_CHAT; i++ ) {
    new_chat->script[i] = 0;
    new_chat->states[i] = 0;
  }

  return new_chat;
}

void free_chat(CHAT_DATA *old) {

  if (old->next != NULL) {
    free_chat(old->next);
    old->next = NULL;
  }

  old->next = free_chat_chain;
  free_chat_chain = old;

  return;
}

CHAT_DATA *read_chat(FILE *fp) {

  CHAT_DATA *chat;
 
  int i;

  chat = get_chat();

  chat->state = fread_number(fp);
  chat->chance = fread_number(fp);

  for( i = 0; i < MAX_CHAT; i++ ) {
    chat->script[i] = fread_number(fp);
    chat->states[i] = fread_number(fp);
  }  

  return chat;
}

void write_chat(FILE *fp, CHAT_DATA *chat, char *hdr) {

  int i;

 /* Write in reverse order... */

  if ( chat->next != NULL ) {
    write_chat(fp, chat->next, hdr);
  }

 /* Write this one... */

  fprintf(fp, "%s %d %d",
               hdr,
               chat->state,
               chat->chance);

  for( i = 0; i < MAX_CHAT; i++) {
     fprintf(fp, "  %d %d",
                  chat->script[i], chat->states[i]);
  }

  fprintf(fp, "\n");

  return;
}

CHAT_DATA *find_chat(CHAT_DATA *chat, int state) {

  while (chat != NULL) {

    if ( chat->state == state ) {
      return chat;
    }

    chat = chat->next;
  }

  return NULL;
}

bool do_chat(CHAR_DATA *ch, char *args) {

  CHAT_DATA *chat;

  int sel;

  MOB_SCRIPT *script;
  MOB_CMD_CONTEXT *mcc;

 /* Check the mob has some chat data... */

  if ( ch->triggers == NULL ) {
    return FALSE;
  } 

  if ( ch->triggers->chat == NULL ) {
    return FALSE;
  } 

 /* Look for the block for thier current state... */
 
  chat = find_chat(ch->triggers->chat, ch->chat_state); 

 /* Bail out if not found... */

  if ( chat == NULL ) {
   
   /* Fix anyone who wandered out of the state machine... */
 
    if ( ch->chat_state != 0 ) { 
      ch->chat_state = 0;
    }
 
    return FALSE;
  }

 /* Ok, see if the chat happens... */

  if (number_bits(7) > chat->chance) {
    return FALSE;
  }

 /* Ok, which one do we run now... */

  sel = number_bits(3);

 /* Update the chat state... */

  ch->chat_state = chat->states[sel]; 

 /* Enqueue the script... */

  if (chat->script[sel] != 0) {
    script = find_script(ch, chat->script[sel]);

    if (script != NULL) {
      mcc = get_mcc(ch, ch, ch->fighting, NULL, NULL, NULL, 
                                                    ch->chat_state, NULL);
      enqueue_script(ch, script, mcc);
    } 
  }

 /* All done... */ 

  return TRUE; 
}


/* Display chat data... */

void medit_chat_show(CHAR_DATA *ch, MOB_INDEX_DATA *mob, char *args) {

  CHAT_DATA *chat;

  char buf[MAX_STRING_LENGTH * 2];
  char buf2[MAX_STRING_LENGTH];

  int i;

 /* Get the mobs chat parameters... */

  chat = mob->triggers->chat;

  if (chat == NULL) {
    send_to_char("No chat defined.\r\n", ch);
    return;
  }

 /* Output time... */ 

  sprintf(buf, "Chat definitions:\r\n"
               " St/Chc   "
        "Scr/St  Scr/St  Scr/St  Scr/St  Scr/St  Scr/St  Scr/St  Scr/St\r\n"
               " ------   "
        "------  ------  ------  ------  ------  ------  ------  ------\r\n");
 
  while (chat != NULL) {

    sprintf(buf2, " {y%2d/{g%3d{w:{x", chat->state, chat->chance);
    strcat(buf, buf2);

    for( i = 0; i < MAX_CHAT; i++) {
      sprintf(buf2, "  {c%3d{w/{c%2d{x", chat->script[i], chat->states[i]); 
      strcat(buf, buf2);
    } 

    strcat(buf, "\r\n");

    chat = chat->next;
  }

  send_to_char(buf, ch);

  return;
}

/* Delete chat data... */

void medit_chat_delete(CHAR_DATA *ch, MOB_INDEX_DATA *mob, char *args) {

  CHAT_DATA *chat, *last_chat;

  int state;

 /* Get the mobs chat parameters... */

  chat = mob->triggers->chat;

  if (chat == NULL) {
    send_to_char("No chat defined.\r\n", ch);
    return;
  }

 /* Only arg should be state... */

  state = atoi(args);

  if ( state < 0
    || state > 99 ) {
    send_to_char("Invalid state, range 0..99\r\n", ch);
    return;
  }

 /* Ok, now we must search... */ 

  last_chat = NULL;
  
  while (chat != NULL) {

    if (chat->state == state) {

      if (last_chat == NULL) {
        mob->triggers->chat = chat->next;
      } else {
        last_chat->next = chat->next;
      }

      chat->next = NULL;
      free_chat(chat);

      send_to_char("Chat state deleted.\r\n", ch);

      chat = NULL;
    } else {
      last_chat = chat;
      chat = chat->next;
    }
  }

  return;
}

/* Add chat data... */

void medit_chat_add(CHAR_DATA *ch, MOB_INDEX_DATA *mob, char *args) {

  CHAT_DATA *chat, *last_chat, *new_chat;

  char next_arg[MAX_INPUT_LENGTH];

  int i;
  
  int state;
  int chance;
  int script;

 /* Extract state... */

  args = one_argument(args, next_arg);

  state = atoi(next_arg);

  if ( state < 0
    || state > 99 ) {
    send_to_char("Invalid state, range 0..99\r\n", ch);
    return;
  }

 /* Extract_chance... */

  args = one_argument(args, next_arg);

  chance = atoi(next_arg);

  if ( chance < 1
    || chance > 127 ) {
    send_to_char("Invalid chance, range 1..127\r\n", ch);
    return;
  }

 /* ok, so lets make the new chat block... */

  new_chat = get_chat();

  new_chat->state = state;
  new_chat->chance = chance;
 
 /* Now chew through the pairs... */

  for( i = 0; i < MAX_CHAT; i++ ) {

    if (args[0] == '\0') {
      send_to_char("Not enough script/state pairs...\r\n", ch);
      free_chat(new_chat);
      return;
    }

    args = one_argument(args, next_arg);

    script = atoi(next_arg);

    if ( script < 1 
      || chance > 999 ) {
      send_to_char("Invalid script, range 1..999\r\n", ch);
      free_chat(new_chat);
      return;
    }

    args = one_argument(args, next_arg);

    state = atoi(next_arg);

    if ( state < 0 
      || state > 99 ) {
      send_to_char("Invalid state, range 0..99\r\n", ch);
      free_chat(new_chat);
      return;
    }

    new_chat->script[i] = script;
    new_chat->states[i] = state;

  }

 /* Get the mobs chat parameters... */

  chat = mob->triggers->chat;

 /* Ok, now we must search... */ 

  last_chat = NULL;

  if (chat == NULL) {
    mob->triggers->chat = new_chat;
    send_to_char("Chat state added.\r\n", ch);
  }
   
  while (chat != NULL) {

    if (chat->state == new_chat->state) {

      if (last_chat == NULL) {
        mob->triggers->chat = chat->next;
      } else {
        last_chat->next = chat->next;
      }

      chat->next = NULL;
      free_chat(chat);

      send_to_char("Replacing old chat state.\r\n", ch);

      if (last_chat == NULL) {
        mob->triggers->chat = new_chat;
        send_to_char("Chat state added, first and last.\r\n", ch);
        return;
      } else {
        chat = last_chat->next;
      }
    }

    if (chat == NULL) {
      if (last_chat == NULL) { 
        mob->triggers->chat = new_chat;
      } else {
        last_chat->next = new_chat;
      }
      send_to_char("Chat state added.\r\n", ch);
      return;  
    } else { 
      if (chat->state > new_chat->state) {

        new_chat->next = chat; 

        if (last_chat == NULL) {
          mob->triggers->chat = new_chat;
        } else {
          last_chat->next = new_chat;
        }

        send_to_char("Chat state added.\r\n", ch);
        return;
      } else {
        last_chat = chat;
        chat = chat->next;

        if (chat == NULL) {
          last_chat->next = new_chat;
          send_to_char("Chat state added.\r\n", ch);
          return;
        }
      }
    }
  }

  return;
}

/* Olc to add or remove chat... */

bool medit_chat(CHAR_DATA *ch, char *args) {

  MOB_INDEX_DATA *pMob;

  char command[MAX_INPUT_LENGTH];
  char *parms;

 /* What are we editing... */

  EDIT_MOB(ch, pMob);

 /* Give help if needed... */ 

  if ( args[0] == '\0'
    || args[0] == '?' ) {
    send_to_char(
     "Syntax: chat show\r\n"
     "        chat add [state] [chance] [scr stat] [scr stat]...\r\n"
     "        chat delete [state]\r\n"
     "   You need to specify 8 pairs of scripts and states\r\n"
     "   Chance is in 128.\r\n", ch);

    return FALSE;
  }   

 /* Extract command and parms... */

  parms = one_argument(args, command);

 /* Ensure we have a trigger block... */

  if (pMob->triggers == NULL) {
    pMob->triggers = get_trs();
  } 

 /* Show current definitions... */

  if (!str_cmp(command, "show")) {
    medit_chat_show(ch, pMob, parms);
    return FALSE;
  } 

 /* Delete a chat state... */

  if (!str_cmp(command, "delete")) {
    medit_chat_delete(ch, pMob, parms);
    return FALSE;
  } 

 /* Add a chat state... */

  if (!str_cmp(command, "add")) {
    medit_chat_add(ch, pMob, parms);
    return FALSE;
  } 

  medit_chat(ch, "");

  return FALSE;
}


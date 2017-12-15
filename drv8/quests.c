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
#include "quests.h"

/* Quest management... */

static QUEST *free_quest_chain = NULL;

QUEST *get_quest() {
  
  QUEST *new_quest;

  if (free_quest_chain == NULL) {
    new_quest = (QUEST *) alloc_perm(sizeof(*new_quest));
  } else {
    new_quest = free_quest_chain;
    free_quest_chain = new_quest->next;
  }

  new_quest->next	= NULL;
  new_quest->id		= 0;
  new_quest->title	= NULL;
  new_quest->state	= 0;
  new_quest->desc	= NULL;

  return new_quest; 
}

void free_quest(QUEST *old_quest) {

  if (old_quest->next != NULL) {
    free_quest(old_quest->next);
  }

  old_quest->id		= 0;
  old_quest->state	= 0;

  if (old_quest->title != NULL) {
    free_string(old_quest->title);
    old_quest->title = NULL; 
  }

  if (old_quest->desc != NULL) {
    free_string(old_quest->desc);
    old_quest->desc = NULL; 
  }

  old_quest->next = free_quest_chain;
  free_quest_chain = old_quest;

  return;
}

/* Show a character their quests... */

void do_own_quests(CHAR_DATA *ch, bool all) {

  QUEST *quest;

  char buf[MAX_STRING_LENGTH];

  char *color;

  bool found;

 /* Have they done any quests? */

  quest = ch->quests;

  if (ch->quests == NULL) {
    send_to_char("You have no quests of note.\r\n", ch);
    return;
  }

 /* Show what they have done... */

  found = FALSE;

  quest = ch->quests;

  while (quest != NULL) {
 
    if ( all
      || quest->state != QUEST_COMPLETED ) {
 
      if (!found) {
        send_to_char("You have the following quests:\r\n"
               " Id    Title\r\n"
               " ----- ---------------------------------------------\r\n", ch);
        found = TRUE;
      }

      switch (quest->state) {
        case QUEST_NONE:
          color = "{x";
          break;
   
        case QUEST_STARTED:
          color = "{c";
          break; 

        case QUEST_COMPLETED:
          color = "{g";
          break; 

        default:
          color = "{y";
          break;
      }

      sprintf(buf, " %5d %s%s{x\r\n       %s (%d)\r\n", 
                    quest->id, 
                    color, 
                    quest->title,
                    quest->desc,
                    quest->state);

      send_to_char(buf, ch);
    }

    quest = quest->next;
  }

 /* No in progress quests? */

  if (!found) {
    send_to_char("You have no in progress quests.\r\n", ch);
    return;
  }

 /* All done... */

  return;
}

/* Process the quests command... */

void do_quests(CHAR_DATA *ch, char *args) {

  QUEST *quest;

  char buf[MAX_STRING_LENGTH];
  char vname[MAX_STRING_LENGTH];

  char *color;

  CHAR_DATA *victim;

  bool all;
  bool found;

 /* Own quests? */

  if (args[0] == '\0') {
    do_own_quests(ch, FALSE);
    return;
  }

 /* Split parms and victim name... */

  args = one_argument(args, vname);

 /* All of own quests? */

  if (!str_cmp(vname, "all")) {
    do_own_quests(ch, TRUE);
    return;
  }

 /* Ok, see if we can locate them... */

  victim = get_char_world(ch, args);

  if (victim == NULL) {
    send_to_char ("Nobody of that name logged on.\r\n", ch);
    return;
  }

 /* All or in progress? */

  all = FALSE;

  if (!str_cmp(vname, "all")) {
    all = TRUE;
  }

 /* Have they done any quests? */

  if (victim->quests == NULL) {
    send_to_char("They have done no quests of note.\r\n", ch);
    return;
  }

 /* Show what they have done... */

  found = FALSE;

  quest = victim->quests;

  while (quest != NULL) {

    if ( all
      || quest->state != QUEST_COMPLETED ) {

      if (!found) { 
        send_to_char("They have performed the following quests:\r\n"
               " Id    Title\r\n"
               " ----- ---------------------------------------------\r\n", ch);
        found = TRUE;
      }

      switch (quest->state) {
        case QUEST_NONE:
          color = "{x";
          break;
  
        case QUEST_STARTED:
          color = "{c";
          break; 

        case QUEST_COMPLETED:
          color = "{g";
          break; 

        default:
          color = "{y";
          break;
      }

      sprintf(buf, " %5d %s%s{x\r\n       %s (%d)\r\n", 
                    quest->id, 
                    color, 
                    quest->title,
                    quest->desc,
                    quest->state);

      send_to_char(buf, ch);
    } 

    quest = quest->next;
  }

 /* No in progress quests? */

  if (!found) {
    send_to_char("They have no in progress quests.\r\n", ch);
    return;
  }

 /* All done... */

  return;
}

void save_quest(FILE *fp, QUEST *quest) {

 /* Safety check... */
 
  if (quest == NULL) {
    return;
  } 

  if (quest->state == QUEST_NONE) {
    return;
  }

 /* Write it out... */

  fprintf(fp, "Quest %d '%s'\n"
                    "%d '%s'\n",
               quest->id,
               quest->title,
               quest->state,
               quest->desc );

 /* Save the next if we're in a chain... */

  if (quest->next != NULL) {
    save_quest(fp, quest->next);
  }

 /* All done... */

  return;
}

QUEST *read_quest(FILE *fp) {

  QUEST *new_quest;

  new_quest = get_quest();

  new_quest->id		= fread_number(fp);
  new_quest->title	= strdup(fread_word(fp));
  new_quest->state	= fread_number(fp);
  new_quest->desc	= str_dup(fread_word(fp));

  new_quest->next = NULL;

  return new_quest;
}

/* Query a quests state... */
   
int quest_state(CHAR_DATA *ch, int id) {

  QUEST *quest;

 /* Simple case... */

  quest = ch->quests;

  if (quest == NULL) {
    return QUEST_NONE;
  }

  if (quest->id == id) {
    return quest->state;
  }

 /* Search time... */

  quest = quest->next;

  while (quest != NULL) {

    if (quest->id == id) {
      break;
    } 

    quest = quest->next;
  } 

 /* So what did we find? */
  
  if (quest == NULL) {
    return QUEST_NONE;
  }

  return quest->state;
}

/* Start a new quest... */
 
void add_quest(CHAR_DATA *ch, int id, char *title) {

  QUEST *new_quest;

 /* Check it's not already been done... */

  if (quest_state(ch,id) != QUEST_NONE) {
    return;
  }

 /* Get a new quest... */

  new_quest = get_quest();

  new_quest->id = id;
  new_quest->title = strdup(title);
  new_quest->state = QUEST_STARTED;
  new_quest->desc = strdup("Starting out");

 /* Now splice it onto the character... */ 

  new_quest->next = ch->quests;
  ch->quests = new_quest;
 
 /* All done... */

  return;
}

/* Update an inprocess quest... */
 
void update_quest(CHAR_DATA *ch, int id, int state, char *desc) {

  QUEST *quest;

 /* Check it's been done... */

  if (quest_state(ch, id) == QUEST_NONE) {
    return;
  }

 /* Simple case... */

  quest = ch->quests;

  if (quest == NULL) {
    return;
  }

  while (quest != NULL) {

    if (quest->id == id) {
      break;
    } 

    quest = quest->next;
  } 

 /* So what did we find? */
  
  if (quest == NULL) {
    return;
  }

 /* Update time... */

  quest->state = state;

  if (quest->desc != NULL) { 
    free_string(quest->desc);
    quest->desc = NULL;
  }

  quest->desc = strdup(desc);

  return;
} 

/* Completely forget about an old quest... */
 
void remove_quest(CHAR_DATA *ch, int id) {

  QUEST *quest, *old_quest;

 /* Check it's been done... */

  if (quest_state(ch, id) == QUEST_NONE) {
    return;
  }

 /* Simple case... */

  quest = ch->quests;

  if (quest == NULL) {
    return;
  }

  if (quest->id == id) {
    ch->quests = quest->next;
    quest->next = NULL;
    free_quest(quest);
    return;
  }

 /* Search time... */

  old_quest = quest;
  quest = quest->next;

  while (quest != NULL) {

    if (quest->id == id) {
      break;
    } 

    old_quest = quest;
    quest = quest->next;
  } 

 /* So what did we find? */
  
  if (quest == NULL) {
    return;
  }

 /* Splicing time... */

  old_quest->next = quest->next;
  quest->next = NULL;

  free_quest(quest);

  return;
} 

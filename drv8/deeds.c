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
#include "deeds.h"

/* Deed management... */

static DEED *free_deed_chain = NULL;

DEED *get_deed() {
  DEED *new_deed;

  if (free_deed_chain == NULL) {
     new_deed = (DEED *) alloc_perm(sizeof(*new_deed));
  } else {
     new_deed = free_deed_chain;
     free_deed_chain = new_deed->next;
  }

  new_deed->next = NULL;
  new_deed->id = 0;
  new_deed->title = NULL;

  return new_deed; 
}


void free_deed(DEED *old_deed) {
  if (old_deed->next != NULL) {
     free_deed(old_deed->next);
  }

  old_deed->id = 0;

  if (old_deed->title != NULL) {
     free_string(old_deed->title);
     old_deed->title = NULL; 
  }

  old_deed->next = free_deed_chain;
  free_deed_chain = old_deed;

  return;
}


/* See if a character has done a deed... */

bool doneDeed(CHAR_DATA *ch, int id) {
  DEED *deed;
  CHAR_DATA *tch = NULL;

  if (ch->desc != NULL) {
      if (ch->desc->original != NULL) tch = ch->desc->original;
  }  
  if (tch == NULL) tch = ch;

  deed = ch->deeds;

  while (deed != NULL) {
      if (deed->id == id) return TRUE;
      deed = deed->next;
  }

  return FALSE;
}


/* Show a character their deeds... */

void do_own_deeds(CHAR_DATA *ch) {
  DEED *deed;
  char buf[MAX_STRING_LENGTH];
  int count;
  char *color;

 /* Have they done any deeds? */

  count = 0;

  deed = ch->deeds;

  while (deed != NULL) {

    if (!IS_SET(deed->type, DEED_SECRET)) {
       count++;
       break;
    }

    deed = deed->next;
  }

  if (count == 0) {
     send_to_char("You have done no deeds of note.\r\n", ch);
     return;
  }

 /* Show what they have done... */

  send_to_char("You have performed the following deeds:\r\n"
               " Id    Title\r\n"
               " ----- ---------------------------------------------\r\n", ch); 

  deed = ch->deeds;

  while (deed != NULL) {

    if (!IS_SET(deed->type, DEED_SECRET)) {

      if (IS_SET(deed->type, DEED_GOOD)) {
        color = "{g";
      } else if (IS_SET(deed->type, DEED_BAD)) {
        color = "{r";
      } else {
        color = "{y";
      } 

      sprintf(buf, " %5d %s%s{x\r\n", deed->id, color, deed->title);
      send_to_char(buf, ch);
    } 

    deed = deed->next;
  }

 /* All done... */

  return;
}


/* Process the deeds command... */

void do_deeds(CHAR_DATA *ch, char *args) {
  DEED *deed = NULL;
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  int count;
  char *color;
  CHAR_DATA *victim;

  if (args[0] == '\0') {
     do_own_deeds(ch);
     return;
  }

  args = one_argument(args, arg1);

  if (!str_cmp(arg1, "add") && IS_IMMORTAL(ch)) {
      char arg2[MAX_STRING_LENGTH];
      char arg3[MAX_STRING_LENGTH];
      int id, type;

      args = one_argument(args, arg2);
      victim = get_char_world(ch, arg2);
      if (victim == NULL) {
           send_to_char ("Nobody of that name logged on.\r\n", ch);
           return;
      }
      args = one_argument(args, arg3);
      id = atoi(arg3);
      args = one_argument(args, arg3);
      type = atoi(arg3);

      if ( type < 0 
      || type >= DEED_MAX) {
         sprintf_to_char(ch,"Bad type (%d) on DEED!", type);
         return;
      }

       if (args[0] == '\0') {
            send_to_char("No title on DEED ADD!", ch);
            return;
       }
       add_deed(victim, id, type, args);
       return;

  } else if (!str_cmp(arg1, "remove") && IS_IMMORTAL(ch)) {
      char arg2[MAX_STRING_LENGTH];
      char arg3[MAX_STRING_LENGTH];
      int id;

      args = one_argument(args, arg2);
      victim = get_char_world(ch, arg2);
      if (victim == NULL) {
           send_to_char ("Nobody of that name logged on.\r\n", ch);
           return;
      }
      args = one_argument(args, arg3);
      id = atoi(arg3);
      remove_deed(victim, id);
      return;

  } else if (!str_cmp(arg1, "list") && IS_IMMORTAL(ch)) {
      MOB_SCRIPT *scr;
      MOB_SCRIPT_LINE *scr_line;
      EXTRA_DESCR_DATA *ed;
      ROOM_INDEX_DATA *room;
      OBJ_INDEX_DATA * pObj;
      char *checkarg;
      char arg[MAX_STRING_LENGTH];
      char *title;
      int num;
      int type, i;
      bool done[64000];

      for (i=0; i<64000; i++) {
          done[i] = FALSE;
      }

      for (victim = char_list; victim; victim = victim->next) {
             if (!IS_NPC(victim)) continue;
             if (!victim->pIndexData->triggers) continue;
             if (!victim->pIndexData->triggers->scripts) continue;
             for(scr = victim->pIndexData->triggers->scripts; scr; scr = scr->next) {
                  for(scr_line = scr->lines; scr_line; scr_line = scr_line->next) {
                      checkarg = strdup(scr_line->cmd);
                      checkarg = one_argument(checkarg, arg);
                      if (!str_cmp(arg, "mpat")) {
                           checkarg = one_argument(checkarg, arg);
                           checkarg = one_argument(checkarg, arg);
                      }
                      if (!str_cmp(arg, "mpdeed")) {
                           checkarg = one_argument(checkarg, arg);
                           checkarg = one_argument(checkarg, arg);
                           if (!str_cmp(arg, "add")) {
                                checkarg = one_argument(checkarg, arg);
                                num = atoi(arg);
                                checkarg = one_argument(checkarg, arg);
                                type = atoi(arg);
                                title = strdup(checkarg);
                                if (num > 0 && !done[num]) {
                                    if (IS_SET(type, DEED_GOOD)) {
                                        sprintf_to_char(ch, "{g%5d  %s{x\r\n", num, title);  
                                    } else if (IS_SET(type, DEED_BAD)) {
                                        sprintf_to_char(ch, "{r%5d  %s{x\r\n", num, title);  
                                    } else {
                                        sprintf_to_char(ch, "{y%5d  %s{x\r\n", num, title);  
                                    } 
                                    done[num] = TRUE;
                                }
                           }                                                                                            
                      }
                  }
             }
      }

      for (i = 1; i < 65536;  i++) {
            if ((room = get_room_index(i)) != NULL) {
                  if ((ed = room-> extra_descr) != NULL) {
                        if (ed->deed) {
                              if (ed->deed->id > 0 && !done[ed->deed->id]) {
                                   if (IS_SET(ed->deed->type, DEED_GOOD)) {
                                        sprintf_to_char(ch, "{g%5d  %s{x\r\n", ed->deed->id, ed->deed->title);  
                                   } else if (IS_SET(ed->deed->type, DEED_BAD)) {
                                        sprintf_to_char(ch, "{r%5d  %s{x\r\n", ed->deed->id, ed->deed->title);  
                                   } else {
                                        sprintf_to_char(ch, "{y%5d  %s{x\r\n", ed->deed->id, ed->deed->title);  
                                   } 
                                   done[ed->deed->id] = TRUE;
                              }
                        }
                  }
            }
      }

      for (i = 1; i < 65536;  i++) {
            if ((pObj = get_obj_index(i)) != NULL) {
                  if ((ed = pObj-> extra_descr) != NULL) {
                        if (ed->deed) {
                              if (ed->deed->id > 0 && !done[ed->deed->id]) {
                                   if (IS_SET(ed->deed->type, DEED_GOOD)) {
                                        sprintf_to_char(ch, "{g%5d  %s{x\r\n", ed->deed->id, ed->deed->title);  
                                   } else if (IS_SET(ed->deed->type, DEED_BAD)) {
                                        sprintf_to_char(ch, "{r%5d  %s{x\r\n", ed->deed->id, ed->deed->title);  
                                   } else {
                                        sprintf_to_char(ch, "{y%5d  %s{x\r\n", ed->deed->id, ed->deed->title);  
                                   } 
                                   done[ed->deed->id] = TRUE;
                              }
                        }
                  }
            }
      }

      return;

  } else if (!str_cmp(arg1, "discipline") && IS_IMMORTAL(ch)) {
      MOB_SCRIPT *scr;
      MOB_SCRIPT_LINE *scr_line;
      EXTRA_DESCR_DATA *ed;
      ROOM_INDEX_DATA *room;
      OBJ_INDEX_DATA * pObj;
      char *checkarg;
      char arg[MAX_STRING_LENGTH];
      char *title;
      int num;
      int type, i;

      for (victim = char_list; victim; victim = victim->next) {
             if (!IS_NPC(victim)) continue;
             if (!victim->pIndexData->triggers) continue;
             if (!victim->pIndexData->triggers->scripts) continue;
             for(scr = victim->pIndexData->triggers->scripts; scr; scr = scr->next) {
                  for(scr_line = scr->lines; scr_line; scr_line = scr_line->next) {
                      checkarg = strdup(scr_line->cmd);
                      checkarg = one_argument(checkarg, arg);
                      if (!str_cmp(arg, "mpat")) {
                           checkarg = one_argument(checkarg, arg);
                           checkarg = one_argument(checkarg, arg);
                      }
                      if (!str_cmp(arg, "mpdeed")) {
                           checkarg = one_argument(checkarg, arg);
                           checkarg = one_argument(checkarg, arg);
                           if (!str_cmp(arg, "add")) {
                                checkarg = one_argument(checkarg, arg);
                                num = atoi(arg);
                                checkarg = one_argument(checkarg, arg);
                                type = atoi(arg);
                                title = strdup(checkarg);
                                if (num == -1) sprintf_to_char(ch, "{y M%5d  %s{x\r\n",victim->pIndexData->vnum , title);  
                           }                                                                                            
                      }
                  }
             }
      }

      for (i = 1; i < 65536;  i++) {
            if ((room = get_room_index(i)) != NULL) {
                  if ((ed = room-> extra_descr) != NULL) {
                        if (ed->deed) {
                              if (ed->deed->id == -1) sprintf_to_char(ch, "{y R%5d  %s{x\r\n", i, ed->deed->title);  
                        }
                  }
            }
      }

      for (i = 1; i < 65536;  i++) {
            if ((pObj = get_obj_index(i)) != NULL) {
                  if ((ed = pObj-> extra_descr) != NULL) {
                        if (ed->deed) {
                              if (ed->deed->id == -1) sprintf_to_char(ch, "{y O%5d  %s{x\r\n", i, ed->deed->title);  
                        }
                  }
            }
      }

      return;

  } else if (!str_cmp(arg1, "brag")) {
      char arg2[MAX_STRING_LENGTH];
      int id;
      bool found = FALSE;

      args = one_argument(args, arg2);
      if ((id = atoi(arg2)) == 0) {
           send_to_char ("Syntax: DEED BRAG <id>\r\n", ch);
           return;
      }         
      
      for (deed = ch->deeds; deed; deed = deed->next) {      
           if (deed->id == id
           && !IS_SET(deed->type, 1)) {
                if (IS_SET(deed->type, 2)) {
                    send_to_char ("Everybody knows that story.\r\n", ch);
                    return;
                }
                SET_BIT(deed->type, 2);
                send_to_char ("You'll tell everyone about that.\r\n", ch);
                found = TRUE;
           }
     }

     if (!found) send_to_char ("You don't know about that deed.\r\n", ch);
     return;
  }

 /* Ok, see if we can locate them... */

  victim = get_char_world(ch, arg1);

  if (victim == NULL) {
    send_to_char ("Nobody of that name logged on.\r\n", ch);
    send_to_char ("Syntax: DEED\r\n", ch);
    send_to_char ("        DEED <character>\r\n", ch);
    send_to_char ("        DEED BRAG <id>\r\n", ch);
    if (IS_IMMORTAL(ch)) {
        send_to_char ("        DEED ADD <character> <id>\r\n", ch);
        send_to_char ("        DEED REMOVE <character> <id>\r\n", ch);
        send_to_char ("        DEED LIST\r\n", ch);
        send_to_char ("        DEED DISCIPLINE\r\n", ch);
    }
    return;
  }

 /* Have they done any deeds? */

  count = 0;

  deed = victim->deeds;

  while (deed != NULL) {

    if ( IS_SET(deed->type, DEED_PUBLIC)
    || IS_IMMORTAL(ch)) {
       count++;
       break;
    }

    deed = deed->next;
  }

  if (count == 0) {
     send_to_char("They have done no deeds of note.\r\n", ch);
     return;
  }

 /* Show what they have done... */

  send_to_char("They have performed the following (in)famous deeds:\r\n"
               " Id    Title\r\n"
               " ----- ---------------------------------------------\r\n", ch); 

  deed = victim->deeds;

  while (deed != NULL) {

    if ( IS_SET(deed->type, DEED_PUBLIC)
    || IS_IMMORTAL(ch)) {

      if (IS_SET(deed->type, DEED_GOOD)) {
        color = "{g";
      } else if (IS_SET(deed->type, DEED_BAD)) {
        color = "{r";
      } else {
        color = "{y";
      } 

      sprintf(buf, " %5d %s%s{x\r\n", deed->id, color, deed->title);
      send_to_char(buf, ch);
    } 

    deed = deed->next;
  }

 /* All done... */

  return;
}


void save_deed(FILE *fp, DEED *deed) {
 
  if (deed == NULL) {
     return;
  } 

 /* Write it out... */

  fprintf(fp, "Deed %d %d %s~\n", deed->id, deed->type, deed->title);

 /* Save the next if we're in a chain... */

  if (deed->next != NULL) {
    save_deed(fp, deed->next);
  }

 /* All done... */

  return;
}


void save_extra_deed(FILE *fp, DEED *deed, char *hdr) {
 
  if (deed == NULL) {
    return;
  } 

 /* Write it out... */

  fprintf(fp, "%s %d %d %s~\n", hdr, deed->id, deed->type, deed->title);

 /* Save the next if we're in a chain... */

  if (deed->next != NULL) {
    save_extra_deed(fp, deed->next, hdr);
  }

 /* All done... */

  return;
}


DEED *read_deed(FILE *fp) {
  DEED *new_deed;
  new_deed = get_deed();

  new_deed->id		= fread_number(fp);
  new_deed->type	= fread_number(fp);
  new_deed->title	= fread_string(fp);

  new_deed->next = NULL;

  return new_deed;
}
   

void add_deed(CHAR_DATA *ch, int id, int type, char *title) {
  DEED *new_deed;

 /* Check it's not already been done... */

  if (doneDeed(ch,id)) {
    return;
  }

 /* Get a new deed... */

  new_deed = get_deed();

  new_deed->id = id;
  new_deed->type = type;
  new_deed->title = strdup(title);

 /* Now splice it onto the character... */ 

  new_deed->next = ch->deeds;
  ch->deeds = new_deed;
 
 /* All done... */

  return;
}


void remove_deed(CHAR_DATA *ch, int id) {
  DEED *deed, *old_deed;

 /* Check it's been done... */

  if (!doneDeed(ch, id)) {
    return;
  }

 /* Simple case... */

  deed = ch->deeds;

  if (deed == NULL) {
    return;
  }

  if (deed->id == id) {
    ch->deeds = deed->next;
    deed->next = NULL;
    free_deed(deed);
    return;
  }

 /* Search time... */

  old_deed = deed;
  deed = deed->next;

  while (deed != NULL) {

    if (deed->id == id) {
      break;
    } 

    old_deed = deed;
    deed = deed->next;
  } 

 /* So what did we find? */
  
  if (deed == NULL) {
    return;
  }

 /* Splicing time... */

  old_deed->next = deed->next;
  deed->next = NULL;

  free_deed(deed);

  return;
} 

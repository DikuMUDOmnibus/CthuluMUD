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
#include "mob.h"
#include "triggers.h"
#include "current.h"
#include "rooms.h"
#include "exp.h"
#include "profile.h"
#include "anchor.h"

/* World event management... */

static WEV *free_wev_chain = NULL;

WEV *get_wev(int type, int subtype, MOB_CMD_CONTEXT	*context, char *msg_actor, char *msg_victim, char *msg_observer) {
  WEV *new_wev;

  if (free_wev_chain == NULL) {
    new_wev = (WEV *) alloc_perm(sizeof(*new_wev));
  } else {
    new_wev = free_wev_chain;
    free_wev_chain = new_wev->next;
  }

  new_wev->type		= type;
  new_wev->subtype	= subtype;

  new_wev->context	= context;

  new_wev->next		= NULL;

  context->wev		= new_wev;

  if (msg_actor != NULL) {
    new_wev->msg_actor = str_dup(msg_actor);
  } else {
    new_wev->msg_actor = NULL;
  }

  if (msg_victim != NULL) {
    new_wev->msg_victim = str_dup(msg_victim);
  } else {
    new_wev->msg_victim = NULL;
  }

  if (msg_observer != NULL) {
    new_wev->msg_observer = str_dup(msg_observer);
  } else {
    new_wev->msg_observer = NULL;
  }

  return new_wev;
}

void free_wev(WEV *wev) {

  wev->next = free_wev_chain;

  free_wev_chain = wev;

  wev->type = 0;
  wev->subtype = 0;

  if (wev->context != NULL) {
    free_mcc(wev->context);
    wev->context = NULL;
  }

  if (wev->msg_actor != NULL) {
    free_string(wev->msg_actor);
    wev->msg_actor = NULL;
  }

  if (wev->msg_victim != NULL) {
    free_string(wev->msg_victim);
    wev->msg_victim = NULL;
  }

  if (wev->msg_observer != NULL) {
    free_string(wev->msg_observer);
    wev->msg_observer = NULL;
  }

  return;
}

/* World Event Distribution... */

void room_echo_wev(ROOM_INDEX_DATA *room, WEV *wev, char *source, bool dist_further) {
  MONITORING_LIST *monitor;
  ROOM_INDEX_DATA *watcher;
  CHAR_DATA *mob;
  ANCHOR_LIST *anchor;

  OBJ_DATA *dobj;
  OBJ_DATA *obj;
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;

 /* Safety check... */

  if ( room == NULL || wev == NULL)    return;
  
 /* Validate wev... */

  if ( wev->type <= WEV_TYPE_NONE
    || wev->type >= MAX_WEV
    || wev->subtype <= WEV_SUBTYPE_NONE
    || wev->context == NULL)     return;
  
 /* Send to each mob in the room... */

  mob = room->people;

  while (mob != NULL) {

    if ( mob->desc != NULL
    || mob->triggers->reaction != NULL) {
 
         /* Set context... */
          wev->context->mob = mob;

         /* Let the mob at it... */
          if (!IS_AFFECTED(mob, AFF_MORF)) mob_handle_wev(mob, wev, source);
    }

    if (wev->context->actor != mob && mob->activity == ACV_PHOTO) {
       if ((dobj = get_eq_char(mob, WEAR_HOLD)) != NULL) {
            if (dobj->item_type == ITEM_CAMERA) {
                if (dobj->value[4] == wev->type) {
                      dobj->value[4] = 0;
                      set_activity(mob, POS_STANDING, NULL, ACV_NONE, NULL);
                      make_photo(mob, dobj, dobj->contains);
                }                 
            }
       }
    }
     
    if (dist_further) {
       dobj = mob->carrying;
       while (dobj != NULL) {
              if (dobj->trans_char != NULL) mob_handle_wev(dobj->trans_char, wev, source);

              if (dobj->item_type == ITEM_CAMERA
              && wev->context->actor != mob
              && dobj->contains != NULL
              && wev->type != WEV_OPROG) {
                  if (dobj->contains->item_type == ITEM_PHOTOGRAPH
                  && dobj->contains->value[0] == PHOTO_EXPOSING) {
                       photo_write_wev(dobj->contains, wev);
                   }
              }

              if (dobj->pIndexData->anchors != NULL) {
                     anchor = dobj->pIndexData->anchors;
                     
                     while (anchor != NULL) {
                            if (anchor->restrict != 0
                            && anchor->restrict != wev->type) {
                                    anchor = anchor->next;
                                    continue;
                            }

                            if (anchor->anchor_type == ANCHOR_TYPE_ROOM
                            && anchor->vnum != dobj->carried_by->in_room->vnum) room_echo_wev(get_room_index(anchor->vnum), wev, source, FALSE); 

                            if (anchor->anchor_type == ANCHOR_TYPE_MOB) {
                                    for ( vch = char_list; vch != NULL; vch = vch_next )    {
                    	            vch_next	= vch->next;
                                            if (vch->in_room == NULL ) continue;
                                            if (vch->in_room == mob->in_room) continue;
                
                                            if (vch->pIndexData != NULL) {
                                                         if (vch->pIndexData->vnum  == anchor->vnum
                                                         && vch->in_room != dobj->in_room) {
                                                                 wev->context->mob = vch;
                                                                 mob_handle_wev(vch, wev, source);
                                                         }
                                            }
                                    }
                            }

                            if (anchor->anchor_type == ANCHOR_TYPE_OBJ) {
                                    for (obj = object_list; obj != NULL; obj = obj->next )    {
      
                                            if (obj->pIndexData->vnum  == anchor->vnum) {
                                                   if (obj->in_room != NULL) {
                                                        if (obj->in_room == mob->in_room) continue;
                                                        room_echo_wev(obj->in_room, wev, source, FALSE); 
                                                        continue;
                                                   }
                                                   if (obj->carried_by != NULL) {
                                                        if (obj->carried_by->in_room == mob->in_room) continue;
                                                        room_echo_wev(obj->carried_by->in_room, wev, source, FALSE); 
                                                        continue;
                                                   }
                                            }
                                    }
                            }
                            anchor = anchor->next;
                     }
              }
              dobj = dobj->next_content;
       }

   }
   /* Now check for currents... */

    if ( room->currents != NULL
    && ( wev->type == WEV_ARRIVE
    || wev->type == WEV_CONTROL )) {
          apply_current(room, mob);
    }

   /* Ok, next mob... */

    mob = mob->next_in_room;
  }

  if (dist_further) {
       dobj = room->contents;
       while (dobj != NULL) {
              if (dobj->trans_char != NULL) mob_handle_wev(dobj->trans_char, wev, source);

              if (dobj->pIndexData->anchors != NULL) {
                     anchor = dobj->pIndexData->anchors;

                     while (anchor != NULL) {
                            if (anchor->restrict != 0
                            && anchor->restrict != wev->type) {
                                    anchor = anchor->next;
                                    continue;
                            }

                            if (anchor->anchor_type == ANCHOR_TYPE_ROOM
                            && anchor->vnum != dobj->in_room->vnum) room_echo_wev(get_room_index(anchor->vnum), wev, source, FALSE); 

                            if (anchor->anchor_type == ANCHOR_TYPE_MOB) {
                                    for ( vch = char_list; vch != NULL; vch = vch_next )    {
                    	            vch_next	= vch->next;
                                            if (vch->in_room == NULL ) continue;
                
                                            if (vch->pIndexData != NULL) {
                                                         if (vch->pIndexData->vnum  == anchor->vnum
                                                         && vch->in_room != dobj->in_room) {
                                                                 wev->context->mob = vch;
                                                                 mob_handle_wev(vch, wev, source);
                                                         }
                                            }
                                    }
                            }

                            if (anchor->anchor_type == ANCHOR_TYPE_OBJ) {
                                    for (obj = object_list; obj != NULL; obj = obj->next )    {
                                            if (obj->pIndexData->vnum  == anchor->vnum) {
                                                   if (obj->in_room != NULL) {
                                                       if (obj->in_room == dobj->in_room) continue;
                                                        room_echo_wev(obj->in_room, wev, source, FALSE); 
                                                        continue;
                                                   }
                                                   if (obj->carried_by != NULL) {
                                                        if (obj->carried_by->in_room == dobj->in_room) continue;
                                                        room_echo_wev(obj->carried_by->in_room, wev, source, FALSE); 
                                                        continue;
                                                   }
                                            }
                                    }
                            }

                            anchor = anchor->next;
                     }
              }
              dobj = dobj->next_content;
       }
          
 /*Send to global monitor station*/

  watcher = get_room_index(mud.monitor);

  if (watcher != NULL) {

    mob = watcher->people;

    while (mob != NULL) {

      if ( mob->desc != NULL
        || mob->triggers->reaction != NULL) {
 
       /* Set context... */

        wev->context->mob = mob;

       /* Let the mob at it... */

        mob_handle_wev(mob, wev, source);
      }
 
      mob = mob->next_in_room;
    }
  }
  


 /* Send to local monitors... */

  if (wev->context->room == room) {

    monitor = room->monitors;

    while (monitor != NULL) {

      mob = monitor->monitor;

      if ( mob->in_room != room ) {

        if ( monitor->type == WEV_TYPE_NONE
          || monitor->type == wev->type ) {

          if ( mob->triggers->reaction != NULL
            || !IS_NPC(mob) ) {

           /* Set context... */

            wev->context->mob = mob;

           /* Let the mob at it... */

            mob_handle_wev(mob, wev, source); 

          }
        }
      }
 
     /* Ok, next mob... */
 
      monitor = monitor->next;
    }
  }
 }
 /* All done... */

  return;
}

/* World Event Distribution... */

void room_issue_wev(ROOM_INDEX_DATA *room, WEV *wev) {
  int door;
  EXIT_DATA *pexit, *pexit2;
  COND_DEST_DATA *cdd;
  ROOM_INDEX_DATA *dest;
  OBJ_DATA *obj;
  char *from = NULL;
  char buf[MAX_STRING_LENGTH];  
  bool send;

 /* Safety check... */

  if ( room == NULL
  || wev == NULL)     return;
  
 /* Validate wev... */

  if ( wev->type <= WEV_TYPE_NONE
  || wev->type >= MAX_WEV
  || wev->subtype <= WEV_SUBTYPE_NONE
  || wev->context == NULL)     return;
  
 /* Set room of issue... */

  wev->context->room = room;

 /* Send to everyone in the room... */

  room_echo_wev(room, wev, NULL, TRUE); 

 /* WEV through a portal */

  obj = room->contents;

  while (obj != NULL) {
         if (obj->item_type == ITEM_PORTAL
         && obj->value[0] > 0) {
              
              dest = get_room_index(obj->value[0]);
               if (obj->value[4] == PORTAL_VEHICLE
               || obj->value[4] == PORTAL_BUILDING) {
                      sprintf(buf, "Outside %s", obj->short_descr);
                      free_string(from);
                      from = strdup(buf);
                      room_echo_wev(dest, wev, from, TRUE); 
               } else {
                      if (obj->value[1] > 0) {
                          sprintf(buf, "Through %s", obj->short_descr);
                          free_string(from);
                          from = strdup(buf);
                          room_echo_wev(dest, wev, from, TRUE); 
                      }
               }
         }
         obj = obj->next_content;
  }

 /* Now send to each interested exit... */

  send = TRUE;

  if ( !IS_SET(room->room_flags, ROOM_ECHO) ) {
    send = FALSE;
  }

 /* Send it is we need to... */

  if ( send ) {

   /* Check each exit... */

    for(door = 0; door < MAX_DIR; door++ ) {

    /* See if there is an exit and check it goes somewhere... */

      pexit = room->exit[door];

      if ( pexit != NULL  
        && pexit->u1.to_room != NULL ) {

       /* Default is we don't send... */

        send = FALSE;
 
       /* Now see if the event should be echoed... */
     
        if (IS_SET(pexit->rs_flags, EX_ECHO_ALL)) {
          send = TRUE;
        } else if ( IS_SET(pexit->rs_flags, EX_ECHO_SOUND)
                 && wev->type == WEV_ICC ) {
          send = TRUE;
        } else if ( IS_SET(pexit->rs_flags, EX_ECHO_VISION)
                 && wev->type != WEV_ICC ) {
          send =TRUE;
        }
      
       /* Now closed doors stop the event from being sent... */

        if ( send
          && IS_SET(pexit->rs_flags, EX_ISDOOR)
          && IS_SET(pexit->rs_flags, EX_CLOSED)) {
          send = FALSE;
        }

       /* Actually send the event... */

        if (send) { 
          room_echo_wev(pexit->u1.to_room, wev, dir_name[rev_dir[door]], TRUE);

         /* Then we set it for each conditional destination... */

          cdd = pexit->cond_dest;

          while (cdd != NULL) {
       
           /* ...that leads back here (at least in default... */ 

            dest = cdd->dest;

            if ( dest != NULL ) {
 
              pexit2 = dest->exit[rev_dir[door]];
 
              if ( pexit2 != NULL 
                && pexit2->u1.to_room == room ) {

                room_echo_wev(dest, wev, dir_name[rev_dir[door]], TRUE);

              }
            }
    
            cdd = cdd->next;
          }
        }
      }
    } 
  }

 /* All done... */

  return;
}

bool room_issue_wev_challange(ROOM_INDEX_DATA *room, WEV *wev) {

  CHAR_DATA *mob;

  MONITORING_LIST *monitor;

  bool permitted;

 /* Safety check... */

  if ( room == NULL
    || wev == NULL) {
    return TRUE;
  }

 /* Validate wev... */

  if ( wev->type <= WEV_TYPE_NONE
    || wev->type >= MAX_WEV
    || wev->subtype <= WEV_SUBTYPE_NONE
    || wev->context == NULL) {
    return TRUE;
  }  

 /* Set room of issue... */

  wev->context->room = room;

 /* Send to each mob in the room... */

  permitted = TRUE;

  mob = room->people;

  while ( mob != NULL
       && permitted) {

    if ( mob->triggers->challange != NULL) {
 
     /* Set context... */

      wev->context->mob = mob;

     /* Let the mob at it... */

      permitted &= mob_handle_wev_challange(mob, wev);
    } 

    mob = mob->next_in_room;
  }

 /* Now we need to check the monitors... */

  monitor = room->monitors;

  while ( monitor != NULL
       && permitted) {

    mob = monitor->monitor;

    if ( mob->in_room != room ) {

      if ( monitor->type == WEV_TYPE_NONE
        || monitor->type == wev->type ) {

        if ( mob->triggers->challange != NULL ) {
 
         /* Set context... */

          wev->context->mob = mob;

         /* Let the mob at it... */

          permitted &= mob_handle_wev_challange(mob, wev);
        }
      }
    } 

    monitor = monitor->next;
  }

  return permitted;
}


/* Issue a WEV concerning a door... */

void room_issue_door_wev(ROOM_INDEX_DATA *room, WEV *wev, char *msg) {
  
  int door;

  EXIT_DATA *pexit, *pexit2;

  ROOM_INDEX_DATA *dest;

  COND_DEST_DATA *cdd;

 /* Check for stupidity... */

  if ( room == NULL
    || wev == NULL ) {
    return;
  } 

 /* First we issue to this room... */

  room_issue_wev(room, wev);

 /* Work out the reverse direction... */ 

  if ( wev->context->number < 0
    || wev->context->number >= MAX_DIR) {
    return;
  }

 /* Now we need to edit the WEV and the context a little... */

  door = wev->context->number;

  wev->context->number = rev_dir[wev->context->number];

//  if (wev->context->text != NULL) {
//    free_string(wev->context->text);
//  }

  wev->context->text = str_dup(dir_name[wev->context->number]);

  if (wev->msg_actor != NULL) {
    free_string(wev->msg_actor);
  }

  wev->msg_actor = str_dup(msg);  

  if (wev->msg_observer != NULL) {
    free_string(wev->msg_observer);
  }

  wev->msg_observer = str_dup(msg);  

 /* Now send the WEV to all rooms potentially through the door... */

  pexit = room->exit[door];

  if (pexit == NULL) {
    return;
  }

 /* First, the direct room... */

  dest = pexit->u1.to_room;

  if ( dest != NULL ) {
 
    pexit2 = dest->exit[rev_dir[door]];

    if ( pexit2 != NULL 
      && pexit2->u1.to_room == room ) {

      room_issue_wev(dest, wev);
    }
  }

 /* Then we set it for each conditional reverse exit... */

  if (pexit->cond_dest == NULL) {
    return;
  } 

 /* Ok, search for the one we want... */

  cdd = pexit->cond_dest;

  while ( cdd != NULL ) {
       
    dest = cdd->dest;

    if ( dest != NULL ) {
 
      pexit2 = dest->exit[rev_dir[door]];

      if ( pexit2 != NULL 
        && pexit2->u1.to_room == room ) {

        room_issue_wev(dest, wev);
      }
    }
    
    cdd = cdd->next;
  }
 
 /* All done... */ 

  return;
}

/* Issue a wev to all players... */

void world_issue_wev(WEV *wev, char *source) {
DESCRIPTOR_DATA *desc;
CHAR_DATA *player;

 /* Safety check... */

  if ( wev == NULL) return;

 /* Validate wev... */

  if ( wev->type <= WEV_TYPE_NONE
    || wev->type >= MAX_WEV
    || wev->subtype <= WEV_SUBTYPE_NONE
    || wev->context == NULL) {
    return;
  }  

 /* Set room of issue... */

  if (wev->context->actor != NULL) {
    wev->context->room = wev->context->actor->in_room;
  } else {
    wev->context->room = NULL;
  }

 /* Send to each player in the game... */

  desc = descriptor_list;

  while ( desc != NULL ) {

   /* Find the descriptors character... */

      player = desc->character;

   /* If playing and not linkdead, send the message... */
 
    if (desc->connected == CON_PLAYING && desc->ok ) {
      mob_handle_wev(player, wev, source);
    } else if (desc->connected == CON_CHATTING && desc->ok) {
         if (wev->type == WEV_OOCC && wev->subtype == WEV_OOCC_CHAT)  mob_handle_chat(player, wev, source);
    }

   /* Get the next descriptor... */

    desc = desc->next;
  }

  return;
}

/* Send an event to an area based upon a room... */

void area_issue_wev(ROOM_INDEX_DATA *room, WEV *wev, int scope) {

  ROOM_INDEX_DATA *next_room;
  ROOM_INDEX_DATA *collection;

 /* Safety check... */

  if ( room == NULL
    || wev == NULL) {
    return;
  }

 /* Validate wev... */

  if ( wev->type <= WEV_TYPE_NONE
    || wev->type >= MAX_WEV
    || wev->subtype <= WEV_SUBTYPE_NONE
    || wev->context == NULL) {
    return;
  }  

 /* Set room of issue... */

  wev->context->room = room;

 /* Now, where do we have to send it? */

  collection = collect_rooms(room, scope, wev->context->actor);

 /* Now send it to the collected rooms... */

  while (collection != NULL) {

    room_issue_wev(collection, wev);

   /* Remove the room from the collection and get the next one... */

    next_room = collection->next_collected;
    collection->next_collected = NULL;

    collection = next_room;
  }

 /* All done... */

  return;
}

/* Make an idol issue a wev... */

void idol_issue_wev(OBJ_DATA *idol, WEV *wev) {

  ROOM_INDEX_DATA *room;

  int i;

 /* Check it is an idol... */

  if (idol->item_type != ITEM_IDOL) {
    return;
  } 

 /* Send to the idols rooms... */

  for(i = 0; i < 5; i++) {

    if (idol->value[i] != 0) {
      room = get_room_index(idol->value[0]); 

      if (room != NULL) {
        room_echo_wev(room, wev, idol->short_descr, TRUE);
      }
    }
  }

 /* All done... */

  return;
}

/* Make a room issue an idol wev... */

void ch_issue_idol_wev(CHAR_DATA *ch, WEV *wev) {

  OBJ_DATA *obj;

 /* Send the WEV to all idols in the room... */

  if (ch->in_room != NULL) {

    obj = ch->in_room->contents;

    while (obj != NULL) {

      if (obj->item_type == ITEM_IDOL) {
        idol_issue_wev(obj, wev);
      }

      obj = obj->next_content;
    }
  }

 /* Send to all idols being worn... */

  obj = ch->carrying;

  while (obj != NULL) {
 
    if ( obj->item_type == ITEM_IDOL
      && obj->wear_loc != WEAR_NONE) {
      idol_issue_wev(obj, wev);
    }
 
    obj = obj->next_content;
  }

 /* All done... */

  return;
}

/* Filter wevs that a mob sees... */

bool wev_seen(CHAR_DATA *mob, WEV *wev) {

  switch (wev->type) {

    case WEV_COMBAT:

     /* Fullfight suppresses all non-hit combat wevs... */

      if (wev->subtype != WEV_COMBAT_HIT) {
        if ( !IS_SET(mob->comm, COMM_FULLFIGHT)) {
          return FALSE;
        }
      }

      break;

    case WEV_ICC:

      switch (wev->subtype) {

        case WEV_ICC_PRAY:
          if (get_divinity(mob) < DIV_LESSER) return FALSE;
          break;

        default:
          break; 
      }
      break;

    case WEV_ARRIVE:
    case WEV_DEPART:
        if (!can_see(mob, wev->context->actor)) return FALSE;
        break;

    case WEV_OOCC:

     /* Quiet suppress all OOC Communication */
 
      if (IS_SET(mob->comm, COMM_QUIET)
      || IS_RAFFECTED(mob->in_room, RAFF_SILENCE)) {
        return FALSE;
      }

     /* Check individual subtypes... */  

      switch (wev->subtype) {

        case WEV_OOCC_GOSSIP:
          if (IS_SET(mob->comm, COMM_NOGOSSIP)) return FALSE;
          break;

        case WEV_OOCC_CHAT:
          if (IS_SET(mob->comm, COMM_NOCHAT)) return FALSE;
          break;

        case WEV_OOCC_SOCIAL:
          switch (wev->context->number) {
              default:
                 if (IS_SET(mob->comm, COMM_NOGOSSIP)) return FALSE;
                 break;
              case 1:
                 if (IS_SET(mob->comm, COMM_NOHERO)) return FALSE;
                 if (get_divinity(mob) < DIV_HERO) return FALSE;
                 break;
              case 2:
                 if (IS_SET(mob->comm, COMM_NOINV)) return FALSE;
                 if (get_divinity(mob) < DIV_INVESTIGATOR) return FALSE;
                 break;
          }         
          break;

        case WEV_OOCC_QUESTION:
        case WEV_OOCC_ANSWER:
          if (IS_SET(mob->comm, COMM_NOQUESTION)) return FALSE;
          break;

        case WEV_OOCC_MUSIC:
          if (IS_SET(mob->comm, COMM_NOMUSIC)) return FALSE;
          break;

        case WEV_OOCC_IMMTALK:
          if (get_divinity(mob) < DIV_HERO) return FALSE;
          if (get_divinity(mob) < DIV_CREATOR && !IS_SET(mob->comm, COMM_IMMACCESS)) return FALSE; 
          if (IS_SET(mob->comm, COMM_NOWIZ)) return FALSE;
          break;

        case WEV_OOCC_HERO:
          if (get_divinity(mob) < DIV_HERO) return FALSE;
          if (IS_SET(mob->comm, COMM_NOHERO)) return FALSE;
          break;

        case WEV_OOCC_INVESTIGATOR:
          if (get_divinity(mob) < DIV_INVESTIGATOR) return FALSE;
          if (IS_SET(mob->comm, COMM_NOINV)) return FALSE;
          break;

        case WEV_OOCC_STELL:
          if (!IS_SET(mob->act, wev->context->number)) return FALSE;
          break;

        default:
          break; 
      }

      break;

    case WEV_DREAM:
      if ( mob->position != POS_SLEEPING ) return FALSE;
      break;

    case WEV_ACTIVITY:

      if ( wev->context->number == ACV_HIDING
      || wev->context->number == ACV_SNEAKING ) {
        if (!can_see(mob, wev->context->actor)) return FALSE;
      } 

      if ( wev->context->actor->activity == ACV_HIDING
      || wev->context->actor->activity == ACV_SNEAKING ) {
        if (!can_see(mob, wev->context->actor)) return FALSE;
      } 
      break;

    case WEV_SPELL:

     /* Fullcast suppresses all mundane casting wevs... */

      if (!IS_SET(mob->comm, COMM_FULLCAST)) {

        switch (wev->subtype) {

          case WEV_SPELL_PREP:
          case WEV_SPELL_CAST:
          case WEV_SPELL_RECOVER:
            return FALSE;

          default:
            break;
        }
      }

      break;

    default:
      break;
  }

  return TRUE;
}

/* World event processing... */

extern char fright_msg[16][80];

void mob_handle_wev(CHAR_DATA *mob, WEV *wev, char *source) {
char buf[MAX_STRING_LENGTH];
char buf2[MAX_STRING_LENGTH];

  if (wev->type == WEV_SHOW && mob != wev->context->actor) {
      if (mob->position < POS_RESTING) {
        act_new("$N isn't awake.", wev->context->actor, NULL, mob, TO_CHAR, POS_RESTING);
        return; 
      }

      if (mob->position == POS_FIGHTING) {
        act_new("$N is to busy to look.", wev->context->actor, NULL, mob, TO_CHAR, POS_RESTING);
        return;
      }   

      if (!can_see_obj(mob, wev->context->obj)) {
        act_new("$N looks impressed.", wev->context->actor, NULL, mob, TO_CHAR, POS_RESTING);
        act_new("$n shows you $s hands.", wev->context->actor, NULL, mob, TO_VICT, POS_RESTING);
        return;
      } 
  }

 /* Messages for active PCs first... */

  if (mob->desc != NULL && wev_seen(mob, wev)) {

    if (mob == wev->context->actor) {
      if (wev->msg_actor != NULL) {

        expand_by_context(wev->msg_actor, wev->context, buf); 

        if (source == NULL) {
          send_to_char(buf, mob);
        } else {
          sprintf(buf2, "{w(%s){x %s", source, buf);
          send_to_char(buf2, mob);
        }
      }
    } else if (mob == wev->context->victim) {
      if (wev->msg_victim != NULL) {

        expand_by_context(wev->msg_victim, wev->context, buf); 

        if (source == NULL) {
          send_to_char(buf, mob);
        } else {
          sprintf(buf2, "{w(%s){x %s", source, buf);
          send_to_char(buf2, mob);
        }

        if (wev->type == WEV_SHOW
        && wev->context->obj
        && !IS_NPC(mob)) {
             examine_object(mob, wev->context->obj);
        }
      }
    } else {
      if (wev->msg_observer != NULL) {

        expand_by_context(wev->msg_observer, wev->context, buf); 

        if (source == NULL) {
          send_to_char(buf, mob);
        } else {
          sprintf(buf2, "{w(%s){x %s", source, buf);
          send_to_char(buf2, mob);
        }

        if (wev->type == WEV_SHOW
        && wev->context->obj
        && !IS_NPC(mob)) {
             examine_object(mob, wev->context->obj);
        }
      }
    }

   /* WEVs from scary thing... */

    if ( mob != wev->context->actor
      && wev->context->actor != NULL
      && wev->context->actor->level >= (mob->level - 5)
      && wev->context->actor->fright > 0
      && wev->context->actor->race != mob->race ) {

     /* Some actions... */

      switch (wev->type) {

        case WEV_ARRIVE:
        case WEV_SAC:
        case WEV_IDOL:

          if (!insanity(wev->context, wev->context->actor->fright, NULL)) {
            send_to_char(fright_msg[number_bits(4)], mob);
          }

          break;

        default:
          break;
      }
 
    }
  }

 /* No triggers, then all done... */
  if (mob->triggers == NULL) return;
  if (mob->triggers->reaction == NULL) return;
 
 /* Schedule scripts... */
  if (mob->desc) return;

  check_trigger(mob->triggers->reaction, wev);
  return;
} 


void mob_handle_chat(CHAR_DATA *mob, WEV *wev, char *source) {
char buf[MAX_STRING_LENGTH];
char buf2[MAX_STRING_LENGTH];


    if (mob == wev->context->actor) {
      if (wev->msg_actor != NULL) {

        expand_by_context(wev->msg_actor, wev->context, buf); 

        if (source == NULL) {
          send_to_char(buf, mob);
        } else {
          sprintf(buf2, "{w(%s){x %s", source, buf);
          send_to_char(buf2, mob);
        }
      }
    } else {
      if (wev->msg_observer != NULL) {

        expand_by_context(wev->msg_observer, wev->context, buf); 

        if (source == NULL) {
          send_to_char(buf, mob);
        } else {
          sprintf(buf2, "{w(%s){x %s", source, buf);
          send_to_char(buf2, mob);
        }

      }
    }

  return;
} 


void photo_write_wev(OBJ_DATA *photo, WEV *wev) {
  char buf[MAX_STRING_LENGTH];
  char longbuf[5*MAX_STRING_LENGTH];
  EXTRA_DESCR_DATA *ed;

  ed = photo->extra_descr;
  while (ed != NULL) {
       if (!str_cmp(photo->name, ed->keyword)) break;
       ed = ed->next;
  }
  if (ed == NULL) return;

  expand_by_context(wev->msg_observer, wev->context, buf); 
  sprintf(longbuf, "%s %s", ed->description, buf);
  free_string(ed->description);
  ed->description = strdup(longbuf);
  return;
} 


bool mob_handle_wev_challange(CHAR_DATA *mob, WEV *wev) {

 /* No triggers, then all done... */

  if (mob->triggers->challange == NULL) return TRUE;

  if (mob->desc != NULL) {
      if (mob->desc->original != NULL) return TRUE;
  }
  
 /* Go see if anything matches... */

  return check_trigger(mob->triggers->challange, wev);
} 

/* Issue a wev straight to a mob... */

void issue_update_wev(CHAR_DATA *mob, int wev_type, 
                                      int wev_sub_type, 
                                      int value) {

  MOB_CMD_CONTEXT *mcc;
  WEV *wev;

  mcc = get_mcc(mob, mob, NULL, NULL, NULL, NULL, value, NULL);

  wev = get_wev(wev_type, wev_sub_type, mcc, NULL, NULL, NULL);

  mob_handle_wev(mob, wev, NULL);

  free_wev(wev);

  return;
}

/* Issue a wev to interested mobs... */

void issue_time_wev(int sub_flag, int wev_type, int wev_sub_type, int value) {
 
  MOB_CMD_CONTEXT *mcc;
  WEV *wev;

  CHAR_DATA *ch, *ch_next;

  mcc = get_mcc(NULL, NULL, NULL, NULL, NULL, NULL, value, NULL);

  wev = get_wev(wev_type, wev_sub_type, mcc, NULL, NULL, NULL);

  ch = char_list; 

  while ( ch != NULL ) {

    ch_next = ch->next;  

   /* Send event if required... */

    if (IS_SET(ch->time_wev, sub_flag)) {
     
      mcc->mob = ch;
      mcc->actor = ch;

      mob_handle_wev(ch, wev, NULL);
    }

    ch = ch_next;
  }    
 
  free_wev(wev);

  return;
}


/* Events... */

void medit_events(CHAR_DATA *ch, char *args) {

  char buf[MAX_STRING_LENGTH];
  int i;

  char *name;
  int index;
  char *desc;
  char *actors;
  char *objects;
  char *amount;
  char *text;

  int nsubs;
  char *subs[WEV_SUBTYPE_MAX_PLUS];
 
  char *evl[6] =
    { " {cnone    give    get        put     drop     poison  fill  {x\r\n",
      " {cgadget  search  depart     arrive  pulse    time    mob   {x\r\n",
      " {cattack  combat  damage     oocc    icc      social  idol  {x\r\n", 
      " {cspell   heal    sacrifice  eat     society  drink   death {x\r\n",
      " {clock    control debate     knock   interpret oprog learn  {x\r\n",
      " {cduel    trap{x\r\n"};

 /* No parms means list them all... */

  if ( args[0] == '\0' 
    || args[0] == '?' ) {

    send_to_char("{wEvents are:\r\n\r\n", ch);

    send_to_char(evl[0], ch);
    send_to_char(evl[1], ch);
    send_to_char(evl[2], ch);
    send_to_char(evl[3], ch);
    send_to_char(evl[4], ch);
    send_to_char(evl[5], ch);

    send_to_char("\r\n{wEnter {yEvents <event_type>{x for more details.{x\r\n", ch);

    return;
  }
 
 /* Set default description... */

  name = "None";
  index = 0;
  desc = "Never issued";
  actors = "Observer, Actor";
  objects = "None";
  amount = "0";
  text = "Not set";

  nsubs = 1; 

  subs[0] = " {w[{C  0{w] {CNever issued{x\r\n";
     
 /* Comm... */

  if (!str_cmp(args, "comm")) {

    name = "Comm";
    index = 1;
    desc = "Retired - see OOCC, ICC, social";
    actors = "Observer, Actor";

    nsubs = 1;

    subs[0] = " {w[{C  1{w] {CRetired{x\r\n";

  }

 /* Give... */

  if (!str_cmp(args, "give")) {

    name = "Give";
    index = WEV_GIVE;
    desc = "Giving things to other mobs";
    actors = "Observer, Actor, Victim";
    objects = "Primary";
    amount = "Gold";

    nsubs = 2;

    subs[0] = " {w[{C  1{w] {CGiving Gold{x\r\n";
    subs[1] = " {w[{C  2{w] {CGiving an item{x\r\n";

  }

 /* Get... */

  if (!str_cmp(args, "get")) {

    name = "Get";
    index = WEV_GET;
    desc = "Picking things up";
    objects = "Primary, Secondary";
    amount = "Gold";

    nsubs = 2;

    subs[0] = " {w[{C  1{w] {CGetting gold{x\r\n";
    subs[1] = " {w[{C  2{w] {CGetting an item{x\r\n";

  }

 /* Put... */

  if (!str_cmp(args, "put")) {

    name = "Put";
    index = WEV_PUT;
    desc = "Putting things in other things";
    objects = "Primary, Secondary";

    nsubs = 1;

    subs[0] = " {w[{C  1{w] {CPutting an item{x\r\n";

  }

 /* Drop... */

  if (!str_cmp(args, "drop")) {

    name = "Drop";
    index = WEV_DROP;
    desc = "Dropping things on the floor";
    objects = "Primary";
    amount = "Gold";

    nsubs = 4;

    subs[0] = " {w[{C  1{w] {CDropping gold{x\r\n";
    subs[1] = " {w[{C  2{w] {CDropping an item{x\r\n";
    subs[2] = " {w[{C  3{w] {CPlanting a tree{x\r\n";
    subs[4] = " {w[{C  4{w] {CDropping an item that melts{x\r\n";

  }

 /* Poison... */

  if (!str_cmp(args, "poison")) {

    name = "Poison";
    index = WEV_POISON;
    desc = "Applying poison to things";
    objects = "Primary";

    nsubs = 4;

    subs[0] = " {w[{C  1{w] {CPoisoning food or drink{x\r\n";
    subs[1] = " {w[{C  2{w] {CPoisoning a weapon{x\r\n";
    subs[2] = " {w[{C  3{w] {CPoisoning a fountain{x\r\n";
    subs[3] = " {w[{C  9{w] {CPoisoning anything else{x\r\n";

  }

 /* Fill... */

  if (!str_cmp(args, "fill")) {

    name = "Fill";
    index = WEV_FILL;
    desc = "Filling bottles";
    objects = "Primary, Secondary";
    text = "Liquid name";

    nsubs = 2;

    subs[0] = " {w[{C  1{w] {CFilling from a fountain{x\r\n";
    subs[1] = " {w[{C  2{w] {CFilling a light{x\r\n";

  }

 /* Drink... */

  if (!str_cmp(args, "drink")) {

    name = "Drink";
    index = WEV_DRINK;
    desc = "Drinking liquids";
    actors = "Actor, Victim";
    objects = "Primary";
    text = "Liquid name";
    amount = "Drinks left"; 

    nsubs = 6;

    subs[0] = " {w[{C  1{w] {CDrinking from a fountain{x\r\n";
    subs[1] = " {w[{C  2{w] {CDrinking something nasty from a fountain{x\r\n";
    subs[2] = " {w[{C  3{w] {CDrinking from a bottle{x\r\n";
    subs[3] = " {w[{C  4{w] {CDrinking something nasty from a bottle{x\r\n";
    subs[4] = " {w[{C  5{w] {CFeeding from a bottle{x\r\n";
    subs[5] = " {w[{C  6{w] {CFeeding something nasty from a bottle{x\r\n";

  }

 /* Eat... */

  if (!str_cmp(args, "eat")) {

    name = "Eat";
    index = WEV_EAT;
    desc = "Eating food/pills";
    actors = "Actor, Victim";
    objects = "Primary";

    nsubs = 7;

    subs[0] = " {w[{C  1{w] {CEating food{x\r\n";
    subs[1] = " {w[{C  2{w] {CEating bad/poisoned food{x\r\n";
    subs[2] = " {w[{C  3{w] {CPopping a pill{x\r\n";
    subs[3] = " {w[{C  4{w] {CChewing a herb{x\r\n";
    subs[4] = " {w[{C  5{w] {CCannibalizing a corpse{x\r\n";
    subs[5] = " {w[{C  6{w] {CFeeding food{x\r\n";
    subs[6] = " {w[{C  7{w] {CFeeding bad/poisoned food{x\r\n";

  }

 /* Sacrifice... */

  if (!str_cmp(args, "sacrifice")) {

    name = "Sacrifice";
    index = WEV_SAC;
    desc = "Sacrifing things";
    objects = "Primary";

    nsubs = 5;

    subs[0] = " {w[{C  1{w] {CSacrificing a player corpse{x\r\n";
    subs[1] = " {w[{C  2{w] {CSacrificing a mob corpse{x\r\n";
    subs[2] = " {w[{C  3{w] {CSacrifing a trash item{x\r\n";
    subs[2] = " {w[{C  4{w] {CSacrifing a treasure item{x\r\n";
    subs[2] = " {w[{C  9{w] {CSacrifing any other item{x\r\n";

  }

 /* Gadget... */

  if (!str_cmp(args, "gadget")) {

    name = "Gadget";
    index = WEV_GADGET;
    desc = "Using Gadgets";
    text = "Gadget name";
    amount = "Gadget action";

    nsubs = 2;

    subs[0] = " {w[{C  1{w] {CSuccessful manipulation{x\r\n";
    subs[1] = " {w[{C  2{w] {CUnsuccessful manipulation{x\r\n";

  }

 /* Search... */

  if (!str_cmp(args, "search")) {

    name = "Search";
    index = WEV_SEARCH;
    desc = "Searching";
    text = "Door direction";
    amount = "Door dirn";
    objects = "Primary, Secondary";
    actors = "Observer, Actor, Victim";

    nsubs = 7;

    subs[0] = " {w[{C  1{w] {CSearching an item{x\r\n";
    subs[1] = " {w[{C  2{w] {CSearching a room{x\r\n";
    subs[2] = " {w[{C  5{w] {CFinding and item{x\r\n";
    subs[3] = " {w[{C  6{w] {CFinding a mob{x\r\n";
    subs[4] = " {w[{C  7{w] {CFinding a secret door{x\r\n";
    subs[5] = " {w[{C  8{w] {CFinding a hound{x\r\n";
    subs[6] = " {w[{C  8{w] {CConcealing an object{x\r\n";

  }

 /* Leave a room... */

  if (!str_cmp(args, "depart")) {

    name = "Depart";
    index = WEV_DEPART;
    desc = "Leave a room";
    text = "Door direction";
    amount = "Door dirn";

    nsubs = 9;

    subs[0] = " {w[{C  1{w] {CWalking out of the room{x\r\n";
    subs[1] = " {w[{C  2{w] {CSneaking out of the room{x\r\n";
    subs[2] = " {w[{C  3{w] {CFlying out of the room{x\r\n";
    subs[3] = " {w[{C  4{w] {CSwimming out of the room{x\r\n";
    subs[4] = " {w[{C  5{w] {CSailing out of the room{x\r\n";
    subs[5] = " {w[{C  6{w] {CLeaving by portal{x\r\n";
    subs[6] = " {w[{C  7{w] {CLeaving by magic{x\r\n";
    subs[7] = " {w[{C  8{w] {CLeaving by current{x\r\n";
    subs[8] = " {w[{C  9{w] {CFleeing from the room{x\r\n";

  }

 /* Enter a room... */

  if (!str_cmp(args, "arrive")) {

    name = "Arrive";
    index = WEV_ARRIVE;
    desc = "Enter a room (no challange)";
    text = "Door direction";
    amount = "Door dirn";

    nsubs = 9;

    subs[0] = " {w[{C  1{w] {CWalking into a room{x\r\n";
    subs[1] = " {w[{C  2{w] {CSneaking into a room{x\r\n";
    subs[2] = " {w[{C  3{w] {CFlying into a room{x\r\n";
    subs[3] = " {w[{C  4{w] {CSwimming into a room{x\r\n";
    subs[4] = " {w[{C  5{w] {CSailing into a room{x\r\n";
    subs[5] = " {w[{C  6{w] {CArriving by portal{x\r\n";
    subs[6] = " {w[{C  7{w] {CArriving by magic{x\r\n";
    subs[7] = " {w[{C  8{w] {CArriving by current{x\r\n";
    subs[8] = " {w[{C  9{w] {CFleeing into the room{x\r\n";

  }

 /* Pulses... */

  if (!str_cmp(args, "pulse")) {

    name = "Pulse";
    index = WEV_PULSE;
    desc = "Realtime Pulses";

    nsubs = 7;

    subs[0] = " {w[{C  1{w] {C1 second pulse{x\r\n";
    subs[1] = " {w[{C  2{w] {C3 second pulse{x\r\n";
    subs[2] = " {w[{C  3{w] {C4 second pulse{x\r\n";
    subs[3] = " {w[{C  4{w] {C5 second pulse{x\r\n";
    subs[4] = " {w[{C  5{w] {C10 second pulse{x\r\n";
    subs[5] = " {w[{C  6{w] {C30 second pulse{x\r\n";
    subs[6] = " {w[{C  7{w] {CAREA pulse (40-80 seconds){x\r\n";

  }

 /* Times... */

  if (!str_cmp(args, "time")) {

    name = "Time";
    index = WEV_TIME;
    desc = "Gametime times";
    amount = "hour/day";

    nsubs = 6;

    subs[0] = " {w[{C  1{w] {CEvery hour{x\r\n";
    subs[1] = " {w[{C  2{w] {CEvery day{x\r\n";
    subs[2] = " {w[{C  3{w] {CEvery sunrise{x\r\n";
    subs[3] = " {w[{C  4{w] {CEvery sunset{x\r\n";
    subs[4] = " {w[{C  5{w] {CEvery dawn{x\r\n";
    subs[5] = " {w[{C  6{w] {CEvery dusk{x\r\n";

  }

 /* Control logic... */

  if (!str_cmp(args, "control")) {

    name = "Control";
    index = WEV_CONTROL;
    desc = "Login/logout control";

    nsubs = 4;

    subs[0] = " {w[{C  1{w] {CEnter a room through login{x\r\n";
    subs[1] = " {w[{C  2{w] {CLeave a room through logout{x\r\n";
    subs[2] = " {w[{C  3{w] {CGoes linkdead{x\r\n";
    subs[3] = " {w[{C  4{w] {CReconnects{x\r\n";

  }

 /* Mobs doing things... */

  if (!str_cmp(args, "mob")) {

    name = "Mob";
    index = WEV_MOB;
    desc = "Mob actions";
    actors = "Observer, Actor, Victim";
    amount = "From MPSELECT"; 
    text = "From MPSELECT";
    objects = "Primary, Secondary";

    nsubs = 3;

    subs[0] = " {w[{C  1{w] {CStop someone from doing something{x\r\n";
    subs[1] = " {w[{C  2{w] {CSelection made with MPSELECT{x\r\n";
    subs[2] = " {w[{C  3{w] {CEvent generated with MPECHO{x\r\n";

  }

 /* Death... */

  if (!str_cmp(args, "death")) {

    name = "Death";
    index = WEV_DEATH;
    desc = "Mobs Dieing";
    actors = "Observer, Actor, Victim";

    nsubs = 2;

    subs[0] = " {w[{C  1{w] {CActor slain. Vicitm is killer.{x\r\n";
    subs[1] = " {w[{C  2{w] {CActor stunned. Vicitm is killer.{x\r\n";

  }

 /* Physical attack... */

  if (!str_cmp(args, "attack")) {

    name = "Attack";
    index = WEV_ATTACK;
    desc = "Mobs attacking (challange only)";
    actors = "Observer, Actor, Victim";

    nsubs = 14;

    subs[0]  = " {w[{C  1{w] {CAttack - {yKILL victim{C.{x\r\n";
    subs[1]  = " {w[{C  2{w] {CAttack - {yKICK victim{C.{x\r\n";
    subs[2]  = " {w[{C  3{w] {CAttack - {yTRIP victim{C.{x\r\n";
    subs[3]  = " {w[{C  4{w] {CAttack - {yBASH victim{C.{x\r\n";
    subs[4]  = " {w[{C  5{w] {CAttack - {yDIRT victim{C.{x\r\n";
    subs[5]  = " {w[{C  6{w] {CAttack - {yBACKSTAB victim{C.{x\r\n";
    subs[6]  = " {w[{C  7{w] {CAttack - {yDISARM victim{C.{x\r\n";
    subs[7]  = " {w[{C  8{w] {CAttack - {yCIRCLE victim{C.{x\r\n";
    subs[8]  = " {w[{C  9{w] {CAttack - {yROTATE victim{C.{x\r\n";
    subs[9]  = " {w[{C 10{w] {CAttack - {yMURDER victim{C.{x\r\n";
    subs[10] = " {w[{C 11{w] {CAttack - {yTAIL victim{C.{x\r\n";
    subs[11] = " {w[{C 12{w] {CAttack - {yCRUSH victim{C.{x\r\n";
    subs[12] = " {w[{C 13{w] {CAttack - {yDISTRACT victim{C.{x\r\n";
    subs[13] = " {w[{C 14{w] {CAttack - {ySPRINKLE victim{C.{x\r\n";

  }

 /* Combat attack... */

  if (!str_cmp(args, "combat")) {

    name = "Combat";
    index = WEV_COMBAT;
    desc = "Mobs fighting (reaction only)";
    actors = "Observer, Actor, Victim";

    nsubs = 8;

    subs[0] = " {w[{C  1{w] {CMob attacking and missing.{x\r\n";
    subs[1] = " {w[{C  2{w] {CMob attacking and hitting.{x\r\n";
    subs[2] = " {w[{C  3{w] {CDefender blocking an attack.{x\r\n";
    subs[3] = " {w[{C  4{w] {CDefender parrying an attack.{x\r\n";
    subs[4] = " {w[{C  5{w] {CDefender dodging an attack.{x\r\n";
    subs[5] = " {w[{C  6{w] {CDefender absorbing an attack.{x\r\n";
    subs[6] = " {w[{C  7{w] {CDefender immune to an attack.{x\r\n";
    subs[7] = " {w[{C  8{w] {CDefender fading before an attack.{x\r\n";

  }

 /* Damage... */

  if (!str_cmp(args, "damage")) {

    name = "Damage";
    index = WEV_DAMAGE;
    desc = "Mobs being injured (reaction only)";
    actors = "Observer, Actor, Victim";
    amount = "hps";

    nsubs = 8;

    subs[0] = " {w[{C  1{w] {CActor hurt >25% by recent damage.{x\r\n";
    subs[1] = " {w[{C  2{w] {CActor hits <25% of total.{x\r\n";
    subs[2] = " {w[{C  3{w] {CActor blinded by dirt kicking.{x\r\n";
    subs[3] = " {w[{C  4{w] {CActor tripped.{x\r\n";
    subs[4] = " {w[{C  5{w] {CActor bashed.{x\r\n";
    subs[5] = " {w[{C  6{w] {CActor injured by environment.{x\r\n";
    subs[6] = " {w[{C  7{w] {CActor tailed.{x\r\n";
    subs[7] = " {w[{C  8{w] {CActor crushed.{x\r\n";

  }

 /* OOCC... */

  if (!str_cmp(args, "oocc")) {

    name = "OOCC";
    index = WEV_OOCC;
    desc = "Out of Charactar Communications";
    actors = "Observer, Actor, Victim";
    text = "Message";
    amount = "language";

    nsubs = 14;

    subs[0] = " {w[{C  1{w] {CBeep{x\r\n";
    subs[1] = " {w[{C  2{w] {CGossip{x\r\n";
    subs[2] = " {w[{C  3{w] {CMusic{x\r\n";
    subs[3] = " {w[{C  4{w] {CImmTalk{x\r\n";
    subs[4] = " {w[{C  5{w] {CQuestion{x\r\n";
    subs[5] = " {w[{C  6{w] {CAnswer{x\r\n";
    subs[6] = " {w[{C  7{w] {CTell{x\r\n";
    subs[7] = " {w[{C  8{w] {CGroup Tell{x\r\n";
    subs[8] = " {w[{C  9{w] {CHeroTalk{x\r\n";
    subs[9] = " {w[{C10{w] {CSocial{x\r\n";
    subs[10] = " {w[{C11{w] {CSTell{x\r\n";
    subs[11] = " {w[{C12{w] {CMTell{x\r\n";
    subs[12] = " {w[{C13{w] {CInvestigatorl{x\r\n";
    subs[13] = " {w[{C14{w] {CChat{x\r\n";

  }

 /* ICC... */

  if (!str_cmp(args, "icc")) {

    name = "ICC";
    index = WEV_ICC;
    desc = "In Charactar Communications";
    actors = "Observer, Actor, Victim";
    text = "Message";
    amount = "language";

    nsubs = 8;

    subs[0] = " {w[{C  1{w] {CTell{x\r\n";
    subs[1] = " {w[{C  2{w] {CSay{x\r\n";
    subs[2] = " {w[{C  3{w] {CShout{x\r\n";
    subs[3] = " {w[{C  4{w] {CScream{x\r\n";
    subs[4] = " {w[{C  5{w] {CYell{x\r\n";
    subs[5] = " {w[{C  6{w] {CMob Tell{x\r\n";
    subs[6] = " {w[{C  7{w] {CTelepathy{x\r\n";
    subs[7] = " {w[{C  8{w] {CPray{x\r\n";
  }

 /* SOC... */

  if (!str_cmp(args, "social")) {

    name = "Social";
    index = WEV_SOCIAL;
    desc = "Non-verbal IC Communications";
    actors = "Observer, Actor, Victim";
    text = "Message";

    nsubs = 5;

    subs[0] = " {w[{C  1{w] {CEmote{x\r\n";
    subs[1] = " {w[{C  2{w] {CFriendly Social{x\r\n";
    subs[2] = " {w[{C  3{w] {CNeutral Social{x\r\n";
    subs[3] = " {w[{C  4{w] {CHostile Social{x\r\n";
    subs[4] = " {w[{C  5{w] {CSociety Social{x\r\n";

  }

 /* IDOL... */

  if (!str_cmp(args, "idol")) {

    name = "Idol";
    index = WEV_IDOL;
    desc = "Concerning idols";
    actors = "Observer, Actor";
    text = "Prayer/Secondary Obj long name";
    amount = "lang/gold";
    objects = "Primary, Secondary";

    nsubs = 4;

    subs[0] = " {w[{C  1{w] {CHolding an idol (every 60 seconds){x\r\n";
    subs[1] = " {w[{C  2{w] {CPraying to an idol{x\r\n";
    subs[2] = " {w[{C  3{w] {COffering an Object{x\r\n";
    subs[3] = " {w[{C  4{w] {COffering gold{x\r\n";

  }

 /* Doors and Locks... */

  if (!str_cmp(args, "lock")) {

    name = "Lock";
    index = WEV_LOCK;
    desc = "Open/Close/Lock/Unlock/Pick";
    actors = "Observer, Actor";
    objects = "Primary";
    amount = "Direction";
    text = "Direction name";

    nsubs = 10;

    subs[0] = " {w[{C  1{w] {COpening a door{x\r\n";
    subs[1] = " {w[{C  2{w] {COpening an item{x\r\n";
    subs[2] = " {w[{C  3{w] {CClosing a door{x\r\n";
    subs[3] = " {w[{C  4{w] {CClosing an item{x\r\n";
    subs[4] = " {w[{C  5{w] {CLocking a door{x\r\n";
    subs[5] = " {w[{C  6{w] {CLocking an item{x\r\n";
    subs[6] = " {w[{C  7{w] {CUnlocking a door{x\r\n";
    subs[7] = " {w[{C  8{w] {CUnlocking an item{x\r\n";
    subs[8] = " {w[{C  9{w] {CPicking a door{x\r\n";
    subs[9] = " {w[{C 10{w] {CPicking an item{x\r\n";

  }

  if (!str_cmp(args, "society")) {

    name = "Society";
    index = WEV_SOCIETY;
    desc = "Society actions";
    actors = "Observer, Actor, Victim";
    amount = "society id";
    text = "society name"; 

    nsubs = 9;

    subs[0] = " {w[{C  1{w] {CVictim invited to join by Actor.{x\r\n";
    subs[1] = " {w[{C  2{w] {CActor joins.{x\r\n";
    subs[2] = " {w[{C  3{w] {CActor promotes victim joins.{x\r\n";
    subs[3] = " {w[{C  4{w] {CActor resigns.{x\r\n";
    subs[4] = " {w[{C  5{w] {CActor expels victim.{x\r\n";
    subs[5] = " {w[{C  6{w] {CActor demotes victim.{x\r\n";
    subs[6] = " {w[{C  7{w] {CActor tests victim.{x\r\n";
    subs[7] = " {w[{C  8{w] {CActor declares victim a foe.{x\r\n";
    subs[8] = " {w[{C  9{w] {CActor pardons victim.{x\r\n";

  }

  if (!str_cmp(args, "dream")) {

    name = "Dream";
    index = WEV_DREAM;
    desc = "Dream actions";
    actors = "Observer, Actor, Victim";
    text = "dream message/action"; 

    nsubs = 8;

    subs[0] = " {w[{C  1{w] {CActor enters the world of dreams.{x\r\n";
    subs[1] = " {w[{C  2{w] {CActor awakens from the world of dreams.{x\r\n";
    subs[2] = " {w[{C  3{w] {CActor sends Public message.{x\r\n";
    subs[3] = " {w[{C  4{w] {CActor sends Private message.{x\r\n";
    subs[4] = " {w[{C  5{w] {CActor sends Public emote.{x\r\n";
    subs[5] = " {w[{C  6{w] {CActor sends Private emote.{x\r\n";
    subs[6] = " {w[{C  7{w] {CActor sends Public echo.{x\r\n";
    subs[7] = " {w[{C  8{w] {CActor sends Private echo.{x\r\n";

  }

  if (!str_cmp(args, "activity")) {

    name = "Activity";
    index = WEV_ACTIVITY;
    desc = "Activity actions";
    actors = "Observer, Actor";
    text = "activity message/action"; 
    amount = "Activity";

    nsubs = 3;

    subs[0] = " {w[{C  1{w] {CActor starts activity.{x\r\n";
    subs[1] = " {w[{C  2{w] {CActor stops activity.{x\r\n";
    subs[2] = " {w[{C  2{w] {CActor POSCHANGE.{x\r\n";

  }

  if (!str_cmp(args, "spell")) {

    name = "Spell";
    index = WEV_SPELL;
    desc = "Spell actions";
    actors = "Observer, Actor, Victim";
    text = "spell name"; 
    amount = "spell number";
    objects = "Primary";

    nsubs = 10;

    subs[0] = " {w[{C  1{w] {CActor preparing to cast a spell.{x\r\n";
    subs[1] = " {w[{C  2{w] {CActor starting to cast a spell.{x\r\n";
    subs[2] = " {w[{C  3{w] {CActor chanting during a spell cast.{x\r\n";
    subs[3] = " {w[{C  4{w] {CActor yelling during a spell cast.{x\r\n";
    subs[4] = " {w[{C  5{w] {CActor casting a spell.{x\r\n";
    subs[5] = " {w[{C  6{w] {CActor recovering from a spell cast.{x\r\n";
    subs[6] = " {w[{C  7{w] {CActor fails to cast a spell.{x\r\n";
    subs[7] = " {w[{C  8{w] {CComponent consumed during spell cast.{x\r\n";
    subs[8] = " {w[{C  9{w] {CEcho effect during casting.{x\r\n";
    subs[9] = " {w[{C 10{w] {CPower word cast.{x\r\n";

  }

  if (!str_cmp(args, "heal")) {

    name = "Heal";
    index = WEV_HEAL;
    desc = "Healing effects";
    actors = "Observer, Actor, Victim";
    text = "healing method"; 
    amount = "ammount healed";

    nsubs = 5;

    subs[0] = " {w[{C  1{w] {CVictims hit points hoealed.{x\r\n";
    subs[1] = " {w[{C  2{w] {CVictims mana healed.{x\r\n";
    subs[2] = " {w[{C  3{w] {CVictims move healed.{x\r\n";
    subs[3] = " {w[{C  4{w] {CUsing a Healer.{x\r\n";
    subs[4] = " {w[{C  5{w] {CUsing Therapy.{x\r\n";

  }

  if (!str_cmp(args, "debate")) {

    name = "Debate";
    index = WEV_DEBATE;
    desc = "Debate activity";
    actors = "Observer, Actor, Victim";
    amount = "points scored";

    nsubs = 3;

    subs[0] = " {w[{C  1{w] {CDebate started.{x\r\n";
    subs[1] = " {w[{C  2{w] {CDebate ongoing.{x\r\n";
    subs[2] = " {w[{C  3{w] {CDebate finished.{x\r\n";

  }

  if (!str_cmp(args, "duel")) {

    name = "Duel";
    index = WEV_DUEL;
    desc = "Duel activity";
    actors = "Observer, Actor, Victim";
    amount = "points scored";

    nsubs = 4;

    subs[0] = " {w[{C  1{w] {CDuel started.{x\r\n";
    subs[1] = " {w[{C  2{w] {CDuel ongoing.{x\r\n";
    subs[2] = " {w[{C  3{w] {CDuel finished.{x\r\n";
    subs[3] = " {w[{C  4{w] {CDuel offensive.{x\r\n";

  }

 /* Knocking on doors... */

  if (!str_cmp(args, "knock")) {

    name = "knock";
    index = WEV_KNOCK;
    desc = "Knock on door";
    actors = "Observer, Actor";
    objects = "Primary";
    amount = "Direction";
    text = "Direction name";

    nsubs = 1;

    subs[0] = " {w[{C  1{w] {CKnock on door{x\r\n";

  }

  if (!str_cmp(args, "interpret")) {

    name = "interpret";
    index = WEV_INTERPRET;
    desc = "Interpret a command";
    actors = "Actor";
    text = "Command line";

    nsubs = 1;

    subs[0] = " {w[{C  1{w] {CInterpret a STRANGE command{x\r\n";
  }


  if (!str_cmp(args, "oprog")) {

    name = "oprog";
    index = WEV_OPROG;
    desc = "Object triggered wevs";
    actors = "Observer, Actor";
    objects = "Primary";

    nsubs = 9;

    subs[0] = " {w[{C  1{w] {CUSING an object{x\r\n";
    subs[1] = " {w[{C  2{w] {CWEARING an object{x\r\n";
    subs[2] = " {w[{C  3{w] {CREMOVING an object{x\r\n";
    subs[3] = " {w[{C  4{w] {CEXPLOSION{x\r\n";
    subs[4] = " {w[{C  5{w] {CPHOTO{x\r\n";
    subs[5] = " {w[{C  6{w] {CDESTROY{x\r\n";
    subs[6] = " {w[{C  7{w] {CREBUILD{x\r\n";
    subs[7] = " {w[{C  8{w] {CEFFECT{x\r\n";
    subs[8] = " {w[{C  9{w] {CCOMBINE objects{x\r\n";

}

  if (!str_cmp(args, "learn")) {

    name = "learn";
    index = WEV_LEARN;
    desc = "Learning from a mob";
    actors = "Observer, Actor, Victim";
    text = "Skill name";
    amount = "sn";

    nsubs = 2;

    subs[0] = " {w[{C  1{w] {CPRACTICE{x\r\n";
    subs[1] = " {w[{C  2{w] {CLEARN{x\r\n";
}

if (!str_cmp(args, "trap")) {

    name = "trap";
    index = WEV_TRAP;
    desc = "Using Traps";
    actors = "Observer, Actor";
    objects = "Primary, Secondary";
    amount = "Trap Type";
    text = "Trap Name";

    nsubs = 4;

    subs[0] = " {w[{C  1{w] {CLAY{x\r\n";
    subs[1] = " {w[{C  2{w] {CDISARM{x\r\n";
    subs[2] = " {w[{C  3{w] {CINFO{x\r\n";
    subs[3] = " {w[{C  4{w] {CTRIGGER{x\r\n";
}

if (!str_cmp(args, "show")) {

    name = "show";
    index = WEV_SHOW;
    desc = "Showing objects";
    actors = "Observer, Victim, Actor";
    objects = "Primary";

    nsubs = 2;

    subs[0] = " {w[{C  1{w] {CITEM{x\r\n";
    subs[1] = " {w[{C  2{w] {CPASSPORT{x\r\n";
}

if (!str_cmp(args, "music")) {

    name = "music";
    index = WEV_MUSIC;
    desc = "Playing an instrument or singing";
    actors = "Observer, Actor";
    objects = "Primary";

    nsubs = 2;

    subs[0] = " {w[{C  1{w] {CSING{x\r\n";
    subs[1] = " {w[{C  2{w] {CPLAY{x\r\n";
}

if (!str_cmp(args, "peek")) {

    name = "peek";
    index = WEV_PEEK;
    desc = "Scanning other players objects";
    actors = "Observer, Victim, Actor";
    objects = "Primary";

    nsubs = 1;

    subs[0] = " {w[{C  1{w] {CITEM{x\r\n";
}

 /* Send the decription... */
 
  sprintf(buf, "{wEvent : [{C%-10s{w]  Index: [{C%3d{w]  Desc: [{C%-35s{w]\r\n"
               "{wActors: [{C%-35s{w]  Objects: [{C%-21s{w]\r\n"
               "{wNumber: [{C%-10s{w]  Text: [{C%-49s{w]\r\n"
               "{wSubtypes:\r\n", 
                name, index, desc, actors, objects, amount, text);
  send_to_char(buf, ch);

  for (i = 0; i < nsubs; i++) {
    send_to_char(subs[i], ch);
  }  
    
 /* All done... */

  return;
}


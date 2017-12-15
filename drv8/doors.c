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
#include "econd.h"
#include "doors.h"
#include "mob.h"

/* Stray external function... */

bool	check_blind		args( ( CHAR_DATA *ch ) );

/* Adjust an exit for day/night conditions... */

ROOM_INDEX_DATA *day_night(ROOM_INDEX_DATA *room) {
ROOM_INDEX_DATA *alt_room;

  if ( room == NULL ) return NULL;

  if ( IS_SET(time_info.flags, TIME_NIGHT) ) {
 
    if ( room->night != 0 ) { 
       alt_room = get_room_index(room->night);

       if (alt_room != NULL) return alt_room;
       else bug("Bad night room for %d", room->vnum);
    }
  } else if ( IS_SET(time_info.flags, TIME_DAY) ) {

    if ( room->day != 0 ) { 
       alt_room = get_room_index(room->day);

       if (alt_room != NULL) return alt_room;
       else bug("Bad day room for %d", room->vnum);
    }
  }

  return room;
}

/* Work out where an exit goes... */

ROOM_INDEX_DATA *get_exit_destination(CHAR_DATA *ch, ROOM_INDEX_DATA *room, EXIT_DATA *pexit, bool recurse) {
COND_DEST_DATA *cdd;
MOB_CMD_CONTEXT *mcc;

  
  if (!room) {
     if (!ch) return NULL;
     room = ch->in_room;
  }

  if (ch) {
       if (pexit->cond_dest) {
           cdd = pexit->cond_dest;

           if (IS_NPC(ch)
           && IS_AFFECTED(ch, AFF_CHARM)
           && ch->master) {
                 mcc = get_mcc(ch->master, ch->master, NULL, NULL, NULL, NULL, 0, NULL);
           } else {
                mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL);
           }

           while ( cdd != NULL
           && ( cdd->dest == NULL  || !ec_satisfied_mcc(mcc, cdd->cond, TRUE))) {
               cdd = cdd->next;
           }
 
           free_mcc(mcc);

 /* If we got one, give it back... */

           if ( cdd != NULL
           && cdd->dest != NULL ) {
                return day_night(cdd->dest);
           }
       }

       if (IS_SET(pexit->exit_info, EX_RANDOM) && recurse) {
            ROOM_INDEX_DATA *toroom = NULL;
            EXIT_DATA *alt_exit;
            int count = 0;
            int num;

            while (count < 10 && !toroom) {
                  count ++;

                  num = number_range(0, DIR_MAX-1);
                 alt_exit = room->exit[num];
                  if (!alt_exit) continue;
                  if (!IS_SET(alt_exit->exit_info, EX_RANDOM)) continue;         

                  toroom = get_exit_destination(ch, room, alt_exit, FALSE);
                  if (toroom) break;
            }
             if (toroom) return day_night(toroom);
       }
  }

 /* otherwise, just the normal location... */
  return day_night(pexit->u1.to_room);
}


bool exit_visible(CHAR_DATA *ch, EXIT_DATA *pexit) {
MOB_CMD_CONTEXT *mcc;
bool ec_sat;

 /* Safety check - a non existant exit cannot be seen... */

  if ( pexit == NULL ) return FALSE;

 /* If the invisible condition is true, the exit cannot be seen... */

  if ( pexit->invis_cond != NULL ) {

    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL);

    ec_sat = ec_satisfied_mcc(mcc, pexit->invis_cond, TRUE);

    free_mcc(mcc);

    if (!ec_sat) return FALSE;
  }
  
 /* Open doors can always be seen... */

  if (  IS_SET(pexit->exit_info, EX_ISDOOR)
    && !IS_SET(pexit->exit_info, EX_CLOSED)) {
    return TRUE;
  }

 /* Hidden exits cannot be seen... */

  if (IS_SET(pexit->exit_info, EX_HIDDEN)) {
    return FALSE;
  }

 /* Otherwise the exit can be seen... */

  return TRUE;
}

/* Toggle the bits on a door... */

void set_door_flags(	ROOM_INDEX_DATA 	*room,
			int			 door,  
			EXIT_DATA 		*pexit, 
			int 			 bit_flag, 
			bool 			 set) {

  COND_DEST_DATA *cdd;

  ROOM_INDEX_DATA *dest;

  ROOM_INDEX_DATA *alt_dest;

  EXIT_DATA *pexit2;

 /* Check for silly call... */

  if (pexit == NULL) {
    return;
  }

 /* First we set the bit on the exit... */

  if (set) { 
    SET_BIT(pexit->exit_info, bit_flag);
  } else {
    REMOVE_BIT(pexit->exit_info, bit_flag);
  }

 /* Then drive a set on the day alternate... */ 

  if (room->day != 0) {
    alt_dest = get_room_index(room->day);

    pexit2 = alt_dest->exit[door];

    if (alt_dest != NULL) {
      set_door_flags(alt_dest, door, pexit2, bit_flag, set);
    }
  } 

 /* ...and the night alternate... */

  if (room->night != 0) {
    alt_dest = get_room_index(room->night);

    pexit2 = alt_dest->exit[door];

    if (alt_dest != NULL) {
      set_door_flags(alt_dest, door, pexit2, bit_flag, set);
    }
  } 

 /* Then we set it on the normal reverse exit... */

  dest = pexit->u1.to_room;

  if ( dest != NULL ) {
 
    pexit2 = dest->exit[rev_dir[door]];

    if ( pexit2 != NULL ) {

     /* Set or Remove? */

      if (set) {

        if ( !IS_SET(pexit2->exit_info, bit_flag)) { 

         /* Flip this one... */

          SET_BIT(pexit2->exit_info, bit_flag );

         /* Drive the opposite side(s)... */

          set_door_flags(dest, rev_dir[door], pexit2, bit_flag, TRUE);

        }
      } else {
        if ( IS_SET(pexit2-> exit_info, bit_flag)) {

         /* Flip this one... */

          REMOVE_BIT(pexit2->exit_info, bit_flag);

         /* Drive the opposite side(s)... */

          set_door_flags(dest, rev_dir[door], pexit2, bit_flag, FALSE);

        }
      }
    }

   /* Fix exit for day alternate destination... */

    if (dest->day != 0) {

      alt_dest = get_room_index(dest->day);

      if (alt_dest != NULL) {

        pexit2 = alt_dest->exit[rev_dir[door]];

        if ( pexit2 != NULL ) {

         /* Set or Remove? */

          if (set) {

            if ( !IS_SET(pexit2->exit_info, bit_flag)) { 

             /* Flip this one... */

              SET_BIT(pexit2->exit_info, bit_flag );

             /* Drive the opposite side(s)... */

              set_door_flags(alt_dest, rev_dir[door], pexit2, bit_flag, TRUE);

            }
          } else {
            if ( IS_SET(pexit2-> exit_info, bit_flag)) {

             /* Flip this one... */

              REMOVE_BIT(pexit2->exit_info, bit_flag);

             /* Drive the opposite side(s)... */

              set_door_flags(alt_dest, rev_dir[door], pexit2, bit_flag, FALSE);

            }
          }
        }
      }
    } 

   /* Fix exit for night alternate destination... */

    if (dest->night != 0) {

      alt_dest = get_room_index(dest->night);

      if (alt_dest != NULL) {

        pexit2 = alt_dest->exit[rev_dir[door]];

        if ( pexit2 != NULL ) {

         /* Set or Remove? */

          if (set) {

            if ( !IS_SET(pexit2->exit_info, bit_flag)) { 

             /* Flip this one... */

              SET_BIT(pexit2->exit_info, bit_flag );

             /* Drive the opposite side(s)... */

              set_door_flags(alt_dest, rev_dir[door], pexit2, bit_flag, TRUE);

            }
          } else {
            if ( IS_SET(pexit2-> exit_info, bit_flag)) {

             /* Flip this one... */

              REMOVE_BIT(pexit2->exit_info, bit_flag);

             /* Drive the opposite side(s)... */

              set_door_flags(alt_dest, rev_dir[door], pexit2, bit_flag, FALSE);

            }
          }
        }
      }
    } 
  }

 /* Then we set it for each conditional reverse exit... */

  if (pexit->cond_dest == NULL) {
    return;
  } 

 /* Ok, set it in each potential destination... */

  cdd = pexit->cond_dest;

  while ( cdd != NULL ) {
       
    dest = cdd->dest;

    if ( dest != NULL ) {
 
      pexit2 = dest->exit[rev_dir[door]];

      if ( pexit2 != NULL ) { 

        if (set) {
          SET_BIT( pexit2->exit_info, bit_flag );
        } else {
          REMOVE_BIT( pexit2->exit_info, bit_flag );
        }
      }

      if (dest->day != 0) {
          
        alt_dest = get_room_index(dest->day);

        if (alt_dest != NULL) { 

          pexit2 = alt_dest->exit[rev_dir[door]];

          if ( pexit2 != NULL ) { 

            if (set) {
              SET_BIT( pexit2->exit_info, bit_flag );
            } else {
              REMOVE_BIT( pexit2->exit_info, bit_flag );
            }
          }
        }
      }

      if (dest->night != 0) {
          
        alt_dest = get_room_index(dest->night);

        if (alt_dest != NULL) { 

          pexit2 = alt_dest->exit[rev_dir[door]];

          if ( pexit2 != NULL ) { 

            if (set) {
              SET_BIT( pexit2->exit_info, bit_flag );
            } else {
              REMOVE_BIT( pexit2->exit_info, bit_flag );
            }
          }
        }
      }

    }

    cdd = cdd->next;
  }
 
 /* All done... */

  return;
}

/* See if a character can flee in a certain direction... */

ROOM_INDEX_DATA *can_flee(CHAR_DATA *ch, int door) {
  EXIT_DATA *pexit;
  ROOM_INDEX_DATA *dest;

 /* Can't flee from the void... */

  if (ch->in_room == NULL) {
    return NULL;
  }

 /* Do we have a valid direction? */

  if ( door == DIR_NONE
    || door >= DIR_MAX ) {
    return NULL;
  }  

 /* Can't leave your master... */

  if ( IS_AFFECTED(ch, AFF_CHARM)
    && ch->master != NULL
    && ch->master->in_room == ch->in_room) {
    return NULL;
  }

 /* Find the exit in that direction... */

  pexit = ch->in_room->exit[door];

 /* No exit means you can't flee that way... */

  if ( pexit == NULL ) {
    return NULL;
  } 

 /* You can't flee through a closed door unless you have pass door... */

  if ( !IS_AFFECTED(ch, AFF_PASS_DOOR)) {
    
    if ( IS_SET(pexit->exit_info, EX_ISDOOR)
      && IS_SET(pexit->exit_info, EX_CLOSED)) {
      return NULL;
    }

  } else {

    if ( IS_SET(pexit->exit_info, EX_ISDOOR)
      && IS_SET(pexit->exit_info, EX_CLOSED)
      && IS_SET(pexit->exit_info, EX_NO_PASS)) {
      return NULL;
    }
  }

 /* Can only flee through exits that we can see... */ 

  if (!exit_visible(ch, pexit)) {
    return NULL;
  }

 /* Find out where the exit goes... */

  dest = get_exit_destination(ch, ch->in_room, pexit, TRUE);

 /* No destination means we can't go there... */

  if ( dest == NULL ) {
    return NULL;
  }

  if (IS_RAFFECTED(dest, RAFF_ENCLOSED)) {
     return NULL;
  }

 /* Looped destination isn't much use either... */

  if (dest == ch->in_room) {
    return NULL;
  }

 /* Are we allowed to go there? */

  if ( IS_NPC(ch)
    && IS_SET(dest->room_flags, ROOM_NO_MOB) ) {
    return NULL;
  }

 /* Strange check - same message if you can't see the room. */

  if (!can_see_room(ch, dest)) return NULL;

 /* Cannot enter a private room... */

  if (room_is_private(ch, dest)) return NULL;

 /* Can't enter a locked area... */

  if ( ch->in_room->area != NULL
    && dest->area != NULL 
    && !IS_SET(ch->in_room->area->area_flags, AREA_LOCKED) 
    &&  IS_SET(dest->area->area_flags, AREA_LOCKED)
    && !IS_IMMORTAL(ch) ) {
    return NULL;
  }

 /* Must fly to enter space of an AIR square... */

  if ( (dest->sector_type == SECT_AIR )
    || (dest->sector_type == SECT_SPACE ) ) {

    if ( !IS_AFFECTED(ch, AFF_FLYING)
    && !IS_AFFECTED(ch, AFF_MIST)
    && !IS_IMMORTAL(ch)) {
      return NULL;
    }
  }

 /* You need to carry (?!) a boat or fly to travel the ocean... */

  if ( ( ch->in_room->sector_type == SECT_WATER_NOSWIM
      || dest->sector_type == SECT_WATER_NOSWIM )
    && !IS_AFFECTED(ch, AFF_FLYING)
    && !IS_AFFECTED(ch, AFF_MIST)) {

    OBJ_DATA *obj;
    bool found;

   /* Look for a boat... */

    found = FALSE;

    if (IS_IMMORTAL(ch)) {
      found = TRUE;
    }

    obj = ch->carrying;

    while ( !found 
         && obj != NULL ) {
      if ( obj->item_type == ITEM_BOAT ) {
        found = TRUE;
      }

      obj = obj->next_content;
    }

    if ( !found ) {
      return NULL;
    }
  }

 /* No flying underwater... */

  if ( ( ch->in_room->sector_type == SECT_UNDERWATER
      || dest->sector_type == SECT_UNDERWATER )
    && (IS_AFFECTED(ch,AFF_FLYING)
      || IS_AFFECTED(ch, AFF_MIST))) {

    return NULL;
  } 

 /* Ok, so return the room they flee to... */

  return dest;
}

/* Thanks to Zrin for auto-exit part... */ 
 
void do_exits( CHAR_DATA *ch, char *argument ) {
  extern char * const dir_name[];
  char buf[MAX_STRING_LENGTH];
  char buf2[128];
  EXIT_DATA *pexit;
  bool found;
  bool fAuto; 
  bool Hlight=FALSE;

  int door;

  ROOM_INDEX_DATA *dest;
  CHAR_DATA *orig;

 /* Initialize things... */
	
  buf[0] = '\0';
  buf[2] = '\0';
  fAuto  = !str_cmp( argument, "auto" );

 /* Find the original... */

  if (ch->desc != NULL) {

    if (ch != ch->desc->character) {
      return;
    }

    orig = ch->desc->original;
  
    if (orig == NULL) {
      orig = ch;
    }
  } else {
    orig = ch;
  }   

 /* Can't see any exits if you're blind... */

  if ( !check_blind( ch ) ) {
    return;
  }

 /* Gods get some extra information... */

  if (IS_SET(orig->plr, PLR_XINFO)) {
    Hlight=TRUE;
  }

  if (Hlight && fAuto) {
    strcpy (buf, "Exits:  [ flags ]\r\n");
  } else {	
    strcpy( buf, fAuto ? "{c[Exits:" : "Obvious exits:\r\n" );
  }

  found = FALSE;

  for ( door = 0; door < DIR_MAX; door++ ) {

   /* Get the exit... */ 

    pexit = ch->in_room->exit[door];

   /* Check it's valid... */

    if ( pexit == NULL ) continue;

   /* Does it go somewhere? */

    dest = get_exit_destination(ch, ch->in_room, pexit, TRUE);

    if (dest == NULL) continue;

   /* Can the player see the destination? (Strange check!) */

    if (!can_see_room(ch, dest)) continue;

   /* Ok, build up the gods display info... */
 
    if  ( Hlight && fAuto) {
		
      found = TRUE;

      sprintf (buf2, "%9s", dir_name[door] );
      strcat (buf2, "   [ ");

      if (IS_SET(pexit->exit_info, EX_ISDOOR)) {
        strcat (buf2, "door ");
      }

      if (IS_SET(pexit->exit_info, EX_CLOSED)) {
        strcat (buf2, "closed ");
      }
  
      if (IS_SET(pexit->exit_info, EX_LOCKED)) {
        strcat (buf2, "locked ");
      }

      if (IS_SET(pexit->exit_info, EX_PICKPROOF)) {
        strcat (buf2, "pickproof ");
      }

      if (IS_SET(pexit->exit_info, EX_HIDDEN)) {
        strcat (buf2, "hidden ");
      }

      if (IS_SET(pexit->exit_info, EX_NO_PASS)) {
        strcat (buf2, "no_pass ");
      }

      if (IS_SET(pexit->exit_info, EX_ROBUST)
      || IS_SET(pexit->exit_info, EX_ARMOURED)) {
        strcat (buf2, "{cre-enforced{x ");
      }

      if (IS_SET(pexit->exit_info, EX_WALL)) {
        strcat (buf2, "{ywall{x ");
      }

      if (!exit_visible(ch, pexit)) {
        strcat (buf2, "invisible ");
      }

      if (IS_RAFFECTED(ch->in_room, RAFF_ENCLOSED)
      || IS_RAFFECTED(dest, RAFF_ENCLOSED)) {
              strcat (buf2, "{Cforce-field{x ");
      }

      strcat (buf2, "]\r\n");
      strcat (buf, buf2);
  
  } else {
     /* Mortals need to check if it is visible... */

      if ( exit_visible(ch, pexit)) {

      if (!IS_SET(pexit->exit_info, EX_WALL)
      || !IS_SET(pexit->exit_info, EX_CLOSED)) {
	found = TRUE;
	if ( fAuto ) {
                      if ( !IS_SET(pexit->exit_info, EX_ISDOOR)
                      || !IS_SET(pexit->exit_info, EX_CLOSED)) {
                              strcat(buf, " {g");
                      } else {
                               if (IS_SET(pexit->exit_info, EX_ROBUST)
                               || IS_SET(pexit->exit_info, EX_ARMOURED)) {
                                           strcat(buf, " {c|#|");
                               } else {
                                            strcat(buf, " {y#");
                               }
                       }
                       strcat( buf, dir_name[door] );
	} else {
	       sprintf( buf + strlen(buf), "%-9s - %s", capitalize( dir_name[door] ), dest->name );
                       if (IS_SET(pexit->exit_info, EX_ISDOOR)) {
                                  strcat(buf, " (door)");
                       }
                       if (IS_SET(pexit->exit_info, EX_CLOSED)) {
                                   strcat(buf, " (closed)");
                       }
                       strcat(buf, "\r\n");  
	}
       }
     }
    }
  }

  if ( !found ) {
    strcat( buf, fAuto ? " none!" : "None!\r\n" );
  }

  if ( fAuto && Hlight) {
    strcat (buf, "\r\n");
  }
 
  if ( fAuto && !Hlight ) {
    strcat( buf, "{c]{x\r\n" );
  }

  send_to_char( buf, ch );

  return;
}

/* Cond_dest_data management... */

static COND_DEST_DATA *free_cdd_chain = NULL;

COND_DEST_DATA *get_cdd() {

  COND_DEST_DATA *cdd;

  if (free_cdd_chain == NULL) {
    cdd = (COND_DEST_DATA *) alloc_perm(sizeof(*cdd));
  } else {
    cdd = free_cdd_chain;
    free_cdd_chain = cdd->next;
  }

  cdd->seq  = 0;
  cdd->vnum = 0;
  cdd->dest = NULL;
  cdd->cond = NULL;
  cdd->next = NULL;
  cdd->name = NULL;

  return cdd;
}

void free_cdd(COND_DEST_DATA *cdd) {

  ECOND_DATA *ec;

  if (cdd != NULL) {

    cdd->seq  = 0;
    cdd->vnum = 0;
    cdd->dest = NULL;

    if (cdd->name != NULL) {
      free_string(cdd->name);
      cdd->name = NULL;
    } 

    while (cdd->cond != NULL) {
      ec = cdd->cond;
      cdd->cond = ec->next;
      free_ec(ec);
    }
   
    cdd->cond = NULL;

    free_cdd(cdd->next);

    cdd->next = free_cdd_chain;
    free_cdd_chain = cdd;
  }

  return;
}

COND_DEST_DATA *read_cdd(FILE *fp) {

  COND_DEST_DATA *cdd;

  cdd = get_cdd();

  cdd->seq = fread_number(fp);

  cdd->vnum = fread_number(fp);
  cdd->dest = NULL;

  cdd->name = fread_string(fp);
 
  return cdd;
}

void insert_cdd(COND_DEST_DATA *cdd, EXIT_DATA *pexit) {

  COND_DEST_DATA *old_cdd;

 /* Easy case first... */

  if (pexit->cond_dest == NULL) {
    pexit->cond_dest = cdd;
    return;
  }

 /* Ok, search time... */
 
  old_cdd = pexit->cond_dest; 

  while ( old_cdd != NULL
       && old_cdd->next != NULL
       && old_cdd->next->seq < cdd->seq ) {
     
    old_cdd = old_cdd->next;
  }

  if (old_cdd == NULL) {
    bug("CDD insert failed!", 0);
    return;
  }
 
  cdd->next = old_cdd->next;
  old_cdd->next = cdd;
   
  return;
}

void write_cdd(COND_DEST_DATA *cdd, FILE *fp, char *hdr1, char *hdr2) {

  if (cdd == NULL) {
    return;
  }

  if ( cdd->dest != NULL ) {
    fprintf(fp, "%s %d %ld %s~\n",
                 hdr1,
                 cdd->seq,
                 cdd->dest->vnum,
                 cdd->name); 
  } else {
    fprintf(fp, "%s %d %ld %s~\n",
                 hdr1,
                 cdd->seq,
                 cdd->vnum,
                 cdd->name); 
  } 

  write_econd(cdd->cond, fp, hdr2); 

  write_cdd(cdd->next, fp, hdr1, hdr2);

  return;
}

static struct door_mapping {
  char	*name;
  int	dir;
} door_map[] = {
  	{	"n",		DIR_NORTH	},
	{	"north",	DIR_NORTH	},
  	{	"s",		DIR_SOUTH	},
	{	"south",	DIR_SOUTH	},
  	{	"e",		DIR_EAST	},
	{	"east",		DIR_EAST	},
  	{	"w",		DIR_WEST	},
	{	"west",		DIR_WEST	},
  	{	"u",		DIR_UP		},
	{	"up",		DIR_UP		},
  	{	"d",		DIR_DOWN	},
	{	"down",		DIR_DOWN	},
  	{	"ne",		DIR_NE		},
	{	"neast",	DIR_NE		},
	{	"northe",	DIR_NE		},
	{	"northeast",	DIR_NE		},
  	{	"se",		DIR_SE		},
	{	"seast",	DIR_SE		},
	{	"southe",	DIR_SE		},
	{	"southeast",	DIR_SE		},
  	{	"sw",		DIR_SW		},
	{	"swest",	DIR_SW		},
	{	"southw",	DIR_SW		},
	{	"southwest",	DIR_SW		},
  	{	"nw",		DIR_NW		},
	{	"nwest",	DIR_NW		},
	{	"northw",	DIR_NW		},
	{	"northwest",	DIR_NW		},
	{	"in",		DIR_IN		},
	{	"ou",		DIR_OUT		},
	{	"out",		DIR_OUT		},
        {       "",		-1		}
};

int door_index2(CHAR_DATA *ch, char *name ) {

    int i, door;

   /* Check the door mapping... */

    door = DIR_NONE;

    i = 0;

    while( door_map[i].dir != -1
        && door == -1 ) {
      if (!str_cmp(name, door_map[i].name)) {
        door = door_map[i].dir;
      }
      i += 1;
    }

    return door;
} 
  
int door_index(CHAR_DATA *ch, char *name ) {

    EXIT_DATA *pexit;
    int i, door;

   /* Check the door mapping... */

    door = DIR_NONE;

    i = 0;

    while( door_map[i].dir != -1
        && door == -1 ) {
      if (!str_cmp(name, door_map[i].name)) {
        door = door_map[i].dir;
      }
      i += 1;
    }
  
   /* If not found, check the extended descriptions for each door... */

    if (door == DIR_NONE) {
      for ( door = 0; door < DIR_MAX; door++ ) {

	pexit = ch->in_room->exit[door];

	if ( pexit != NULL
	  && IS_SET(pexit->exit_info, EX_ISDOOR)
	  && pexit->keyword != NULL
	  && is_name( name, pexit->keyword ) ) {
	    break;
        }  
      }
    }

   /* Still no door? */

    if (door == DIR_MAX) {
      return DIR_NONE;
    }

   /* Now, is there an exit there... */

    pexit = ch->in_room->exit[door];

    if ( pexit == NULL ) {
      return DIR_NO_EXIT;
    }

   /* Can it be seen? */

    if (!exit_visible(ch, pexit)) {
      return DIR_NOT_VISIBLE;
    }
  
   /* Yep, we found an exit... */

    return door;
} 


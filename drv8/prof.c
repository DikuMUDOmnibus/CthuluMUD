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
#include "prof.h"
#include "skill.h"
#include "econd.h"
#include "mob.h"
#include "profile.h"

/* Anchor for professions... */

static PROF_DATA *prof_list = NULL;
void research_prof(CHAR_DATA *ch, int sn);
SOCIETY *find_society_by_id(int id);

/* Find a profession (returns NULL if not found)... */

PROF_DATA *get_profession(char *prof_name) {

  PROF_DATA *prof;

  for(prof = prof_list; prof != NULL; prof = prof->next) {

    if (prof == NULL) {
      break;
    }

    if (prof->name[0] == prof_name[0]) {
      if (!strcmp(prof->name, prof_name)) break;
    } 

  }

  return prof;
}

/* Get a professions id... */

int get_prof_id(char *name) {
  
  PROF_DATA *prof;

  prof = get_profession(name);

  if (prof == NULL) {
    return PROF_UNDEFINED;
  }

  return prof->id; 
}

/* Get a profession by id... */

PROF_DATA *get_prof_by_id(int id) {

  PROF_DATA *prof;

  if (id == PROF_UNDEFINED) return NULL;

  for(prof = prof_list; prof != NULL; prof = prof->next) {
    if (prof == NULL) break;
    if (prof->id == id) break;
  }

  return prof;
}

/* Load professions from disk... */

/* NB This code will terminate the mud if the profession file cannot be
 *    found as this could cause serious damage to characters.
 */  

void load_professions() {
  PROF_DATA *new_prof = NULL;
  FILE *fp;
  char *kwd;
  char code;
  bool done;
  bool ok;
  char *name;
  char buf[MAX_STRING_LENGTH];
  int sn, lev, pid;
  ECOND_DATA *new_ec;

 /* Find the professions file... */

  fp = fopen(PROF_FILE, "r");

  if (fp == NULL) {
    log_string("No professions file!");
    exit(1);
    return;
  }

 /* Read through it and see what we've got... */
 
  done = FALSE;

  pid = PROF_UNDEFINED;

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

    /* Condition */

      case 'C':
        
        if (!strcmp(kwd, "Cond")) {

          ok = TRUE;

         /* Must have an active profession... */

          if (new_prof == NULL) {
            bug("Condition before Profession in profession file!", 0);
            exit(1);
          }

         /* Read and add... */
   
          new_ec = read_econd(fp);

          if (new_ec != NULL) {
            new_ec->next = new_prof->ec;
            new_prof->ec = new_ec;
          } 
        }

        break; 

    /* Description */

      case 'D':
        
        if (!strcmp(kwd, "Desc")) {

          ok = TRUE;

         /* Must have an active profession... */

          if (new_prof == NULL) {
            bug("Description before Profession in profession file!", 0);
            exit(1);
          }

         /* Read and add... */
   
          new_prof->desc = fread_string(fp);

          if (new_prof->desc[0] == '\0') {
            bug("Bad Description profession file!", 0);
            exit(1);
          } 
        }

        break; 

      case 'I': 

       /* Initial yes/no */

        if (!strcmp(kwd, "Initial")) {

          char *pc;

          ok = TRUE;

         /* Must have an active profession... */

          if (new_prof == NULL) {
            bug("initial before Profession in profession file!", 0);
            exit(1);
          }

         /* Read the stat numbers name... */

          pc = fread_word(fp);
 
          switch (pc[0]) {
            case 'y':
            case 'Y':
              new_prof->initial = TRUE;
              break;
            case 'n':
            case 'N':
              new_prof->initial = FALSE;
              break;  
            default:
              bug("Bad initial line...", 0);
              exit(1);
          }
 
        }

       /* Initial Condition */

        if (!strcmp(kwd, "ICond")) {

          ok = TRUE;

         /* Must have an active profession... */

          if (new_prof == NULL) {
            bug("Initial Condition before Profession in profession file!", 0);
            exit(1);
          }

         /* Read and add... */
   
          new_ec = read_econd(fp);

          if (new_ec != NULL) {
            new_ec->next = new_prof->iec;
            new_prof->iec = new_ec;
          } 
        }

        break; 

    /* Kit... */
 
      case 'K':

        if (!strcmp(kwd, "Kit")) {
          long vnum = 0;

          ok = TRUE;

         /* Read the number... */

          name = fread_word(fp);
          if (is_number(name)) vnum = atol(name);

         /* Must have an active profession... */

          if (new_prof == NULL) {
            bug("Skill before Profession in profession file!", 0);
            exit(1);
          }

             /* Must have a kit slot free... */
              if (new_prof->num_kit == PROF_MAX_KIT) {
                bug("To many kit items for profession!", 0);
                exit(1);
              }

             /* Ok, add it to the kit... */
               if (vnum > 0) new_prof->kit[new_prof->num_kit++] = vnum;
          }

        break;

     /* Profession */

      case 'P':

       /* Profession name */ 

        if (!strcmp(kwd, "PROFESSION")) {
 
          ok = TRUE;

         /* Read the profession name... */

          name = fread_word(fp); 

          if (name[0] == '\0') {
            bug("Unnammed profession in profession file!", 0);
            exit(1);
          }

          sprintf(buf, "Loading profession '%s'...", name);
          log_string(buf);

         /* Time for a new profession... */

          if (new_prof == NULL) {
            new_prof = (PROF_DATA *) alloc_perm( sizeof( *new_prof));
            prof_list = new_prof;
            new_prof->next = NULL;
          } else { 
            new_prof->next = (PROF_DATA *) alloc_perm( sizeof( *new_prof));
            new_prof = new_prof->next;
            new_prof->next = NULL;
          }

          if (new_prof == NULL) {
            bug("Failed to allocate memory for new profession!", 0);
            exit(1);
          }

          new_prof->levels = getSkillData(); 

          if (new_prof->levels == NULL) {
            bug("Failed to allocate memory for new profession levels!", 0);
            exit(1);
          }
          
          new_prof->name = strdup(name);  

          new_prof->desc = strdup("No description provided.");

          for(sn = 0; sn < MAX_SKILL; sn++) {
            new_prof->levels->skill[sn] = -1;
          }

          new_prof->num_kit = 0;
          new_prof->default_soc = 0;

          for(sn = 0; sn < PROF_MAX_KIT; sn++) {
            new_prof->kit[sn] = 0;
          }

          for(sn = 0; sn < MAX_STATS; sn++) {
            new_prof->stats[sn] = 4;
          }

          new_prof->prime_stat = STAT_STR;
          new_prof->pc = TRUE;
          new_prof->id = ++pid;
          new_prof->start = 0;
          new_prof->ec = NULL;
          new_prof->iec = NULL;
          break; 
        }

       /* PRIME stat */

        if (!strcmp(kwd, "Prime")) {

          ok = TRUE;

         /* Must have an active profession... */

          if (new_prof == NULL) {
            bug("Prime before Profession in profession file!", 0);
            exit(1);
          }

         /* Read the stat numbers name... */

          new_prof->prime_stat = fread_number(fp);

          if ( new_prof->prime_stat < 0 
            || new_prof->prime_stat > MAX_STATS) {
            bug("Bad prime stat %d.", new_prof->prime_stat);
            exit(1);
          }
 
        }

       /* PC yes/no */

        if (!strcmp(kwd, "PC")) {

          char *pc;

          ok = TRUE;

         /* Must have an active profession... */

          if (new_prof == NULL) {
            bug("Prime before Profession in profession file!", 0);
            exit(1);
          }

         /* Read the stat numbers name... */

          pc = fread_word(fp);
 
          switch (pc[0]) {
            case 'y':
            case 'Y':
              new_prof->pc = TRUE;
              break;
            case 'n':
            case 'N':
              new_prof->pc = FALSE;
              break;  
            default:
              bug("Bad PC line...", 0);
              exit(1);
          }
 
        }

        break;
 
    /* Skill... */

      case 'S':
 
       /* SKILL 'name' level*/

        if (!strcmp(kwd, "Skill")) {

          ok = TRUE;

         /* Must have an active profession... */

          if (new_prof == NULL) {
            bug("Skill before Profession in profession file!", 0);
            exit(1);
          }

         /* Read the skill name... */

          name = fread_word(fp); 

          if (name[0] == '\0') {
            bug("Unnammed skill in skill file!", 0);
            exit(1);
          }

         /* Get the skill number... */
 
          sn = get_skill_sn(name);

          if (sn == SKILL_UNDEFINED) { 
            sprintf(buf, "Undefined skill '%s' in professions file!", name);
            bug(buf, 0);
            exit(1);
          }

         /* Pull in the number... */

          lev = fread_number(fp);

          if ( lev < 0 
            || lev > MAX_LEVEL) {
            sprintf(buf, "Bad level %d for skill %s.",  lev, new_prof->name);
            bug(buf, 0);
            exit(1);
          } 
          
         /* Set the value... */ 

          new_prof->levels->skill[sn] = lev;

          break;
        }

       /* STATS str int wis dex con~*/

        if (!strcmp(kwd, "Stats")) {

          ok = TRUE;

         /* Must have an active profession... */

          if (new_prof == NULL) {
            bug("Stat before Profession in profession file!", 0);
            exit(1);
          }

         /* Read the stat numbers name... */

          for(sn = 0; sn < MAX_STATS; sn++) {

            new_prof->stats[sn] = fread_number(fp);

            if ( new_prof->stats[sn] < 1 
              || new_prof->stats[sn] > 5) {
              bug("Bad dice number %d for stats.", new_prof->stats[sn]);
              exit(1);
            } 
          
          }
 
        }

       /* Start vnum */

        if (!strcmp(kwd, "Start")) {

          ok = TRUE;

         /* Must have an active profession... */

          if (new_prof == NULL) {
            bug("Start before Profession in profession file!", 0);
            exit(1);
          }

         /* Read the start vnum... */

          new_prof->start = fread_number(fp);

          if ( new_prof->start < 1 
            || new_prof->start > 32767 ) {
            bug("Bad start vnum %d for start.", new_prof->start);
            exit(1);
          } 
        }

        if (!strcmp(kwd, "Soc")) {
          ok = TRUE;

          if (new_prof == NULL) {
            bug("Start before Profession in profession file!", 0);
            exit(1);
          }

          new_prof->default_soc = fread_number(fp);
        }

        break;

     /* File ends with an E */

      case 'E':
        ok = TRUE;
        done = TRUE;
        break; 

     /* Anything else is an error... */

      default:
        break;
    }

    if (!ok) {
      sprintf(buf, "Unrecognized keyword in professions file: %s", kwd);
      bug(buf, 0); 
      exit(1);
    }
 
  }

 /* Close it now we're finished with it... */ 

  fclose(fp);

 /* All done... */

  log_string("...professions loaded");

  return;
}

/* Check starting vnum for all professions... */

void check_prof_starts() {

  PROF_DATA *prof;

  prof = prof_list;

  while (prof != NULL) {

    if (prof->start != 0) {
      if (!room_ok_for_recall(prof->start)) {
        bug("Bad starting room %d for profession.", prof->start); 
        exit(1);
      }
    }

    prof = prof->next;
  }

  return;
}

/* Anchor for free lprof blocks... */

static LPROF_DATA *free_lprofs = NULL;

/* Get an lprof block... */

LPROF_DATA *get_lprof() {

  LPROF_DATA *new_lprof;

  if (free_lprofs != NULL) {
    new_lprof = free_lprofs;
    free_lprofs = new_lprof->next;
  } else {
    new_lprof = (LPROF_DATA *) alloc_perm( sizeof( *new_lprof));
  }  

  new_lprof->next = NULL;
  new_lprof->profession = NULL;
  new_lprof->level = 0; 

  return new_lprof;
} 

/* Return an unwanted lprof... */

void free_lprof(LPROF_DATA *old_lprof) {

  old_lprof->next = free_lprofs;
  old_lprof->profession = NULL;
  old_lprof->level = 0;

  free_lprofs = old_lprof;

  return;
}

/* Display a list of all professions... */

void do_prof_list(CHAR_DATA *ch) {
int col;
PROF_DATA *prof;
MOB_CMD_CONTEXT *mcc;
char outbuf[6000];                                  // 100 Professions
char buf[MAX_STRING_LENGTH];
outbuf[0] = '\0';
bool show;
 
  sprintf(outbuf, "Defined professions are:\r\n"); 
 
  mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL); 

  col    = 0;
  for ( prof = prof_list; prof != NULL; prof = prof->next) {
 	
    if ( prof == NULL )  break;

    if (prof->pc) {
      if ( ec_satisfied_mcc(mcc, prof->ec, TRUE)) {
        strcat(outbuf, "{g");
        show = TRUE;
      } else { 
        strcat(outbuf, "{y");
        show = TRUE;
      }
    } else {
      if (IS_IMMORTAL(ch)) {
         strcat(outbuf, "{b");
         show = TRUE;
      } else {
         show = FALSE;
      }
    }

    if (show) {
        sprintf( buf, "%-25s{x ", prof->name);
        strcat(outbuf, buf);
        if ( ++col % 3 == 0 ) strcat(outbuf, "\r\n" );
    }
  }
 
  if ( col % 3 != 0 ) {
    strcat(outbuf, "\r\n");
  }
  
  page_to_char( outbuf, ch );
  free_mcc(mcc);
  return;
}


/* Display profession information... */

void do_prof_info(CHAR_DATA *ch, char *prof_name) {
PROF_DATA *prof;
char outbuf[10000];                                  // 100 Skills

  prof = get_profession(prof_name);

  if (prof == NULL) {
    sprintf_to_char(ch, "Profession '%s' not defined.\r\n", prof_name);
    return;
  }

  outbuf[0] = '\0';
  sprintf(outbuf, "{wProfession: {g%s\r\n" "{g%s{x\r\n", prof->name, prof->desc);   
  send_to_char( outbuf, ch );
  return;
}


/* Display all the skills a profession has... */

void do_prof_detail(CHAR_DATA *ch, char *prof_name) {
int col;
int sn;
PROF_DATA *prof;
MOB_CMD_CONTEXT *mcc;
char outbuf[10000];                                  // 100 Skills
char buf[MAX_STRING_LENGTH];
prof = get_profession(prof_name);

  if (prof == NULL) {
    sprintf_to_char(ch, "Profession '%s' not defined.\r\n", prof_name);
    return;
  }

  mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL);

  outbuf[0] = '\0';
  sprintf(outbuf, "{wProfession: {g%s\r\n" "{c%s{x" "Skills:\r\n" , prof->name, prof->desc);   

  col    = 0;
  for ( sn = 0; sn < MAX_SKILL; sn++) {
	
    if ( !skill_array[sn].loaded ) break;

    if (prof->levels->skill[sn] >= 0) {

      sprintf( buf, "%-18s %3d  ", skill_array[sn].name, prof->levels->skill[sn]);
      strcat(outbuf, buf);
      if ( ++col % 3 == 0 ) strcat(outbuf, "\r\n" );
    } 
  }

  if ( col % 3 != 0 ) strcat(outbuf, "\r\n");
  strcat(outbuf, "Kit:");

  if (prof->num_kit == 0) {
    strcat(outbuf, " None\r\n");
  } else {

    for(sn = 0; sn < prof->num_kit; sn++) {
      sprintf(buf, "%5d", prof->kit[sn]);
      strcat(outbuf, buf);
    }
   
    strcat(outbuf, "\r\n");
  }

  sprintf(buf, "Stats - STR: %d INT: %d WIS: %d DEX: %d CON: %d\r\n", prof->stats[0], prof->stats[1], prof->stats[2], prof->stats[3], prof->stats[4]);
  strcat(outbuf, buf); 

  strcat(outbuf, "Prime stat: ");

  switch (prof->prime_stat) {
    case 0:
      strcat(outbuf, "STR\r\n");
      break;
    case 1:
      strcat(outbuf, "INT\r\n");
      break;
    case 2:
      strcat(outbuf, "WIS\r\n");
      break;
    case 3:
      strcat(outbuf, "DEX\r\n");
      break;
    case 4:
      strcat(outbuf, "CON\r\n");
      break;
    default:
      strcat(outbuf, "{RINVALID{x\r\n");
      break;
  }

  if (prof->pc) {
    if (prof->initial) {
      strcat(outbuf, "Available to new PCs\r\n");
    } else {
      strcat(outbuf, "Only available to experienced PCs\r\n"); 
    }
  } else {
    strcat(outbuf, "Not available fo PCs\r\n"); 
  }

  if (prof->start != 0) {
    sprintf(buf, "Starting room: %ld\r\n", prof->start);
    strcat(outbuf, buf);
  }

  print_econd_to_buffer(outbuf, prof->ec, "{WConditions -{x ", mcc);
  page_to_char( outbuf, ch );
  free_mcc(mcc);
  return;
}


/* The prof command... */

void do_prof( CHAR_DATA *ch, char *argument ) {

  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
	
  argument=one_argument(argument, arg1);
  argument=one_argument(argument, arg2);
  argument=one_argument(argument, arg3);

  /* No parms, means we show some help... */

  if ( arg1[0] == '\0' ) {
    send_to_char("prof - Profession Commands\r\n\r\n", ch);
    send_to_char("  prof list          - List all professions\r\n", ch);
    send_to_char("  prof list 'prof'   - List skills for a profession\r\n", ch);
    send_to_char("  prof info 'prof'   - Show profession description\r\n", ch);
    send_to_char("  prof change 'prof' - Change to new profession\r\n", ch);
    return;
  }

 /* List list all professions... */

  if (!strcmp(arg1, "list")) {

    if (arg2[0] == '\0') {
      do_prof_list(ch);
    } else {
      do_prof_detail(ch, arg2);
    } 
 
    return;
  }

  if (!strcmp(arg1, "info")) {
    do_prof_info(ch, arg2);
    return;
  } 

  if (!strcmp(arg1, "change")) {
    do_change_prof(ch, arg2);
    return;
  } 

  send_to_char("Unknown prof subcommand.\r\n", ch);

  return;
}


/* Is a skill available to a character... */

bool skill_available_by_prof(int sn, CHAR_DATA *ch) {
PROF_DATA *prof;

 /* Current profession is first learned profession block... */
  if (ch->profs == NULL) return FALSE;

  prof = ch->profs->profession;
  if (prof == NULL) return FALSE;

 /* See if the skill is allowed to the profession... */
  if (prof->levels->skill[sn] == -1) return FALSE;

 /* Check if chars level within their profession is high enough... */
  if (prof->levels->skill[sn] > get_professional_level(ch, ch->profs->level)) return FALSE;

  return TRUE;
}

/* Gain a level in current profession... */

void gain_prof_level(CHAR_DATA *ch) {

  PROF_DATA *prof;

  int sn;

 /* Current profession is first learned profession block... */

  if (ch->profs == NULL) {
    return;
  }

  prof = ch->profs->profession;

  if (prof == NULL) {
    return;
  }

 /* Ok, so gain a level... */

  ch->profs->level += 1;

 /* Now check all of their skills to see if they learnt anything... */

  for(sn = 0; sn < MAX_SKILL; sn++) {

    if (!skill_array[sn].loaded) {
      break;
    }

    if (prof->levels->skill[sn] == -1) {
      continue;
    } 

    if (prof->levels->skill[sn] <= ch->profs->level) {
      if (ch->effective->skill[sn] > 0) {
        check_improve(ch, sn, TRUE, 1);
      }
    } 
  }

  return;
}

/* Lose a level in current profession... */

void lose_prof_level(CHAR_DATA *ch) {

  PROF_DATA *prof;

  LPROF_DATA *lprof;
 
  int sn;

 /* Current profession is first learned profession block... */

  lprof = ch->profs;

  if (lprof == NULL) {
    return;
  }

  prof = lprof->profession;

  if (prof == NULL) {
    return;
  }

 /* Ok, so lose a level... */

  if (lprof->level <= 1) {
    return;
  }

  lprof->level -= 1;

 /* Now check all of their skills to see if they learnt anything... */

  for(sn = 0; sn < MAX_SKILL; sn++) {

    if (!skill_array[sn].loaded) break;

    if (prof->levels->skill[sn] != -1) {
      if (prof->levels->skill[sn] <= lprof->level) {
        if (ch->effective->skill[sn] > 0) check_improve(ch, sn, FALSE, 1);
      } 
    }
  }
  return;
}


/* Give a character the 'default' equipment for their profession... */
void equip_char_by_prof(CHAR_DATA *ch, bool startup) {
PROF_DATA *prof;
OBJ_DATA *obj;
OBJ_INDEX_DATA *pObj;
int i;

 /* Find the characters current profession... */

  if (ch->profs == NULL) return;

  prof = ch->profs->profession;

  if (prof == NULL) {
    bug("Learned prof with NULL profession!", 0);
    return;
  }

 /* Now fabricate all of the (worthless) kit for them... */

  for (i = 0; i < prof->num_kit; i++) {
    if ((pObj = get_obj_index(prof->kit[i])) == NULL) continue;

    if (pObj->item_type == ITEM_PASSPORT) {
        if (!startup) continue; 

        if (find_passport(ch)) continue;
        obj = create_object(pObj, 0 );
        obj->owner = str_dup(ch->name);
    } else {
        obj = create_object(pObj, 0 );
    }
    obj->cost = 0;
    if ( ch->carry_number + 1 > can_carry_n( ch ))    {
	act( "$d: you can't carry that many items.", ch, NULL, obj->name, TO_CHAR );
                extract_obj(obj);
	continue;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )    {
	act( "$d: you can't carry that much weight.", ch, NULL, obj->name, TO_CHAR );
                extract_obj(obj);
	continue;
    }

    obj_to_char(obj, ch);
  }

  return;
}

/* Display logon prompt for profession... */

void prompt_profession(DESCRIPTOR_DATA *d) {
int col;
PROF_DATA *prof;
MOB_CMD_CONTEXT *mcc;
char outbuf[5000];                                  // 100 Professions
char buf[MAX_STRING_LENGTH];
 
  outbuf[0] = '\0';
 
  sprintf(outbuf, "******************************************************************************\r\n"); 
  strcat(outbuf, "\r\n"); 
  strcat(outbuf, "   Available professions are:\r\n"); 
  strcat(outbuf, "\r\n  "); 

  mcc = get_mcc(d->character, d->character, NULL, NULL, NULL, NULL, 0, NULL); 
  col    = 0;
  for ( prof = prof_list; prof != NULL; prof = prof->next) {
    if ( prof == NULL ) break;
    if (!prof->initial) continue;
    if ( ! ec_satisfied_mcc(mcc, prof->iec, TRUE)) continue;
    if ( ! ec_satisfied_mcc(mcc, prof->ec, TRUE)) continue;
    
    sprintf( buf, " %-25s", prof->name);
    strcat(outbuf, buf);
 
    if ( ++col % 3 == 0 ) strcat(outbuf, "\r\n  " );
  }
 
  if ( col % 3 != 0 ) strcat(outbuf, "\r\n");
  strcat(outbuf, "\r\n"); 
  strcat(outbuf, "******************************************************************************\r\n"); 
  strcat(outbuf, "\r\n"); 
  strcat(outbuf, "Enter <profession name>, ?, ? <profession_name> or CHAT\r\n"
                 "Which profession you like to be?\r\n");

  write_to_buffer(d, outbuf, 0);
  free_mcc(mcc);
  return;
}

/* Give information about a profession... */

void prompt_profession_info(DESCRIPTOR_DATA *d, char *args) {

  PROF_DATA *prof;
 
  char outbuf[5000];                                  // 100 Professions
 
  char buf[MAX_STRING_LENGTH];
 
  prof = get_profession(args);

  if (prof == NULL) {

    sprintf(buf, "Profession '%s' not defined.\r\n", args);

    write_to_buffer(d, buf, 0);

    return;
  }

  outbuf[0] = '\0';
 
  sprintf(outbuf, "Profession: %s\r\n"
                  "%s",
                   prof->name,
                   prof->desc);   

  strcat(outbuf, "Which profession you like to be?\r\n");

  write_to_buffer(d, outbuf, 0);

  return;
}

/* Change a characters profession... */

void do_change_prof(CHAR_DATA *ch, char *prof_name) {
PROF_DATA *new_prof;
LPROF_DATA *lprof, *plprof; 
MOB_CMD_CONTEXT *mcc;
char buf[MAX_STRING_LENGTH];
int sn, base; 

 /* PCs should already have a profession... */

  if (ch->profs == NULL) {
    send_to_char("Sorry, you are an amateur.\r\n", ch);
    return;
  }

 /* Find and validate the new profession... */

  new_prof = get_profession(prof_name);

  if (new_prof == NULL) {
    send_to_char("There is no such profession!\r\n", ch);
    return;
  }

 /* Check for stupidity... */

  if (ch->profs->profession == new_prof) {
    send_to_char("You are already one of those!\r\n", ch);
    return;
  }

 /* Stop people changing to often... */

  if (ch->profs->level < 3) {
    send_to_char("You must attain 3rd level in your current profession first.\r\n", ch);
    return;
  }

 /* Check the pre-reqs... */

  mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL);

  if (!ec_satisfied_mcc(mcc, new_prof->ec, TRUE)) {
    free_mcc(mcc);
    send_to_char("You do not meet the entry requirements:\r\n", ch);
    print_econd(ch, new_prof->ec, "Condition - ");
    return;
  }

  free_mcc(mcc);

 /* Check for and pay practice points... */

  if (ch->practice < 3) {
    send_to_char("Changing profession costs 3 practice points!\r\n", ch);
    return;
  }

  ch->practice -= 3;

 /* Now, is this a change back to an old profession... */

  plprof = ch->profs;
  for(lprof = ch->profs->next; lprof != NULL; lprof = lprof->next) {

    if (lprof->profession == new_prof) {
      plprof->next = lprof->next;
      lprof->next = ch->profs;
      ch->profs = lprof;
      
      sprintf(buf, "Profession '%s', level %d.\r\n", ch->profs->profession->name, ch->profs->level);
      send_to_char(buf, ch);

      return;  
    }

    plprof = lprof; 
  }

 /* Nope, must be a new profession... */

  lprof = get_lprof();

  lprof->profession = new_prof;

  lprof->level = 1;

  lprof->next = ch->profs;

  ch->profs = lprof;

 /* Ensure minimum scores in base skills... */

  for(sn = 0; sn < MAX_SKILL; sn++) {
		
    if (!skill_array[sn].loaded) break;
	
    if (ch->profs->profession->levels->skill[sn] == 0) {
      base = get_base_skill(sn, ch);
      if (ch->effective->skill[sn] < base) setSkill(ch->effective, sn, base); 
    }
  }

  equip_char_by_prof(ch, FALSE);
  sprintf(buf, "You are now a first level '%s'.\r\nGood luck.\r\n", ch->profs->profession->name);
  send_to_char(buf, ch);
  return;
}


void research_prof(CHAR_DATA *ch, int sn) {
  PROF_DATA *prof;
  ECOND_DATA *ec = NULL;
  char buf[MAX_STRING_LENGTH];
  char outbuf[3*MAX_STRING_LENGTH];
  
  sprintf(outbuf, "\r\n{cNeeded for Professions:{x\r\n");

  for (prof = prof_list; prof != NULL; prof = prof->next) {
         ec = prof->ec;   
         while (ec != NULL) {
              if (ec->type == ECOND_SKILL
              && sn == ec->index) {
                    sprintf(buf, "   %s\r\n", prof->name);
                    strcat(outbuf, buf);
              }
              ec = ec->next;
         }     
  }
  page_to_char(outbuf, ch);
  return;
}

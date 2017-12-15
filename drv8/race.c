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
#include "race.h"
#include "society.h"
#include "skill.h"

/* Load the races from disk... */

/* NB This routines aborts the mud if it finds an error, as it can't
 *    guarentee that the races will be properly loaded.  This could
 *    cause serious damage to characters.
 */ 

RACE_TYPE race_array[MAX_RACE];

void load_races() {
  FILE *fp;
  char *kwd;
  char code;
  bool done;
  bool ok;
  char *name;
  int rn;
  long value;
  long long lvalue;

  char buf[MAX_STRING_LENGTH];

 /* Initialize incase of problems... */

  for(rn = 0; rn < MAX_RACE; rn++) {
    race_array[rn].loaded = FALSE;
  } 

 /* Initialize race 0 */

  rn = 0;

  race_array[rn].name 		= "unique";
  race_array[rn].number		= 0;
  race_array[rn].society	= 100;
  race_array[rn].act		= 0;
  race_array[rn].aff		= 0;
  race_array[rn].off		= 0;
  race_array[rn].imm		= 0;
  race_array[rn].envimm		= 0;
  race_array[rn].res		= 0;
  race_array[rn].vuln		= 0;
  race_array[rn].form		= 0;
  race_array[rn].parts		= 0;
  race_array[rn].nature		= 0;
  race_array[rn].lifespan		= 100;
  race_array[rn].material		= 0;
  race_array[rn].language	                = SKILL_UNDEFINED;
  race_array[rn].size		= SIZE_MEDIUM;
  race_array[rn].track_name	= "strange";
  race_array[rn].cult                   	= NULL;
  race_array[rn].pc_race	                = FALSE;
  race_array[rn].loaded		= TRUE;

 /* Find the banks file... */

  fp = fopen(RACE_FILE, "r");

  if (fp == NULL) {
    log_string("No spell file!");
    exit(1);
    return;
  }

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

     /* A */

      case 'A':
 
       /* Act 'act_flags' */

        if (!strcmp(kwd, "Act")) {

          ok = TRUE;

          name = fread_word(fp);

          lvalue = flag_value_long(act_flags, name);

          if ( lvalue != NO_FLAG ) { 
            race_array[rn].act |= lvalue;
          } else {
            bug("Bad Act flag in races.txt", 0);
          }
        }
   
       /* Aff 'aff_flags' */

        if (!strcmp(kwd, "Aff")) {

          ok = TRUE;

          name = fread_word(fp);

          lvalue = flag_value_long(affect_flags, name);

          if ( lvalue != NO_FLAG ) { 
            race_array[rn].aff |= lvalue;
          } else {
            bug("Bad Aff flag in races.txt", 0);
          }
   
          break;
        }
 
        break;

     /* B */

      case 'B':
 
       /* Body 'form_flags' */

        if (!strcmp(kwd, "Body")) {

          ok = TRUE;

          name = fread_word(fp);

          value = flag_value(form_flags, name);

          if ( value != NO_FLAG ) { 
            race_array[rn].form |= value;
          } else {
            bug("Bad Body flag in races.txt", 0);
          }
       }
        break;

      case 'C':

        if (!strcmp(kwd, "Cult")) {
          ok = TRUE;
          name = fread_word(fp);
          race_array[rn].cult = strdup(name);
        }
        break;

     /* End */

      case 'E':

        if (!strcmp(kwd, "EnvImm")) {

          ok = TRUE;
          name = fread_word(fp);
          value = flag_value(imm_flags, name);

          if ( value != NO_FLAG ) { 
            race_array[rn].envimm |= value;
          } else {
            bug("Bad EnvImm flag in races.txt", 0);
          }
       }

       /* File ends with an 'End' */

        if (!strcmp(kwd, "End")) {
          ok = TRUE;
          done = TRUE;
          break; 
        } 

        break; 

     /* I */

      case 'I':
 
       /* Imm 'imm_flags' */

        if (!strcmp(kwd, "Imm")) {

          ok = TRUE;

          name = fread_word(fp);

          value = flag_value(imm_flags, name);

          if ( value != NO_FLAG ) { 
            race_array[rn].imm |= value;
          } else {
            bug("Bad Imm flag in races.txt", 0);
          }
       }
 
       break;
   
     /* L */

      case 'L':
 
       /* Lang 'language' */

        if (!strcmp(kwd, "Lang")) {

          ok = TRUE;

          name = fread_word(fp);

          value = get_skill_sn(name);

          if ( value != SKILL_UNDEFINED ) { 
            race_array[rn].language = value;
          } else {
            bug("Bad Language in races.txt", 0);
          }
       }
 
        if (!strcmp(kwd, "Life")) {
          ok = TRUE;
          race_array[rn].lifespan = fread_number(fp);
        }

       break;


      case 'M':
         if (!strcmp(kwd, "Mat")) {
            ok = TRUE;
            name = fread_word(fp);
            value = flag_value(material_type, name);

            if ( value >0
            && value <= MAX_MATERIAL) { 
                race_array[rn].material = value;
            } else {
                race_array[rn].material = 0;
            }
         }

         break;
   

     /* Number number */

      case 'N':
 
       /* Number number */

        if (!strcmp(kwd, "Number")) {

          ok = TRUE;

          race_array[rn].number = fread_number(fp);

          break;
        }
     
       /* Nature 'nature_flags' */

        if (!strcmp(kwd, "Nature")) {

          ok = TRUE;

          name = fread_word(fp);

          value = flag_value(mob_nature, name);

          if ( value != NO_FLAG ) { 
            race_array[rn].nature |= value;
          } else {
            bug("Bad Nature flag in races.txt", 0);
          }
        }
 
        break;

     /* O */

      case 'O':
 
       /* Off 'off_flags' */

        if (!strcmp(kwd, "Off")) {

          ok = TRUE;

          name = fread_word(fp);

          value = flag_value(off_flags, name);

          if ( value != NO_FLAG ) { 
            race_array[rn].off |= value;
          } else {
            bug("Bad Off flag in races.txt", 0);
          }
       }
 
       break;
   
     /* P */

      case 'P':
 
       /* PC Yes|No */

        if (!strcmp(kwd, "PC")) {

          ok = TRUE;

          name = fread_word(fp);

          if ( name[0] == 'Y' ) {
            race_array[rn].pc_race = TRUE;
          } else if ( name[0] == 'N' ) {
            race_array[rn].pc_race = FALSE;
          } else {
            bug("Invalid PC specification in races.txt!", 0);
          }

          break;
        }
     
       /* Parts 'part_flags' */

        if (!strcmp(kwd, "Parts")) {

          ok = TRUE;

          name = fread_word(fp);

          value = flag_value(part_flags, name);

          if ( value != NO_FLAG ) { 
            race_array[rn].parts |= value;
          } else {
            bug("Bad Part flag in races.txt", 0);
          }
        }
 
        break;

     /* RACE 'name' */

      case 'R':
 
       /* RACE name */

        if (!strcmp(kwd, "RACE")) {

          ok = TRUE;

         /* Read the skill name... */

          name = fread_word(fp); 

          if (name[0] == '\0') {
            bug("Unnammed race in race file!", 0);
            exit(1);
          }

          sprintf(buf, "...Loading race '%s'...", name);
          log_string(buf);

         /* Get the new race number... */
 
          rn++;

          if (rn >= MAX_RACE) { 
            bug("To many races in race file!",0);
            exit(1);
          }

         /* Initialize data... */ 

          race_array[rn].name 		= str_dup(name);
          race_array[rn].number		= 0;
          race_array[rn].society	= 100;
          race_array[rn].act		= 0;
          race_array[rn].aff		= 0;
          race_array[rn].off		= 0;
          race_array[rn].imm		= 0;
          race_array[rn].envimm	= 0;
          race_array[rn].res		= 0;
          race_array[rn].vuln		= 0;
          race_array[rn].form		= 0;
          race_array[rn].parts		= 0;
          race_array[rn].nature		= 0;
          race_array[rn].lifespan	= 100;
          race_array[rn].language	= SKILL_UNDEFINED;
          race_array[rn].size		= SIZE_MEDIUM;
          race_array[rn].track_name	= "strange";
          race_array[rn].pc_race	= FALSE;
          race_array[rn].loaded		= TRUE;

          break;
        }

       /* Res 'res_flags' */

        if (!strcmp(kwd, "Res")) {

          ok = TRUE;

          name = fread_word(fp);

          value = flag_value(res_flags, name);

          if ( value != NO_FLAG ) { 
            race_array[rn].res |= value;
          } else {
            bug("Bad Res flag in races.txt", 0);
          }
        }
 
        break; 

     /* Society number */

      case 'S':
 
       /* Society number */

        if (!strcmp(kwd, "Society")) {

          ok = TRUE;

          race_array[rn].society = fread_number(fp);

          break;
        }
     
       /* Size 'size' */

        if (!strcmp(kwd, "Size")) {

          ok = TRUE;

          name = fread_word(fp);

          value = flag_value(size_flags, name);
          race_array[rn].size = value;
          break;
        }
     
        break;

     /* Tracks 'tracks' */

      case 'T':
 
       /* Tracks 'tracks' */

        if (!strcmp(kwd, "Tracks")) {

          ok = TRUE;

          race_array[rn].track_name = str_dup(fread_word(fp));

          break;
        }
     
        break;

     /* V */

      case 'V':
 
       /* Vuln 'vuln_flags' */

        if (!strcmp(kwd, "Vuln")) {

          ok = TRUE;

          name = fread_word(fp);

          value = flag_value(vuln_flags, name);

          if ( value != NO_FLAG ) { 
            race_array[rn].vuln |= value;
          } else {
            bug("Bad Vuln flag in races.txt", 0);
          }
       }
 
       break;

     /* Anything else is an error... */

      default:
        break;
    }

    if (!ok) {
      sprintf(buf, "Unrecognized keyword in race file: %s", kwd);
      bug(buf, 0); 
      exit(1);
    }
 
  }

 /* Close it now we're finished with it... */ 

  fclose(fp);

 /* All done... */

  log_string("...races loaded");

  return;  
}

// Get a Races rn

int get_race_rn(char *name) {

  int rn;

  for(rn = 0; rn < MAX_RACE; rn++) {

    if ( race_array[rn].name == NULL ) {
      return RACE_UNDEFINED;
    }
 
    if ( name[0] == race_array[rn].name[0] 
      && !strcmp(name, race_array[rn].name) ) {
      return rn;
    }
  }

  return RACE_UNDEFINED;
}

// See if a race number in valid...

bool valid_race(int rn) {

  if ( rn < 0 
    || rn >= MAX_RACE ) {
    return FALSE;
  } 

  if (!race_array[rn].loaded) {
    return FALSE;
  }

  return TRUE;
}

// Display logon prompt for race... */

void prompt_race(DESCRIPTOR_DATA *d) {
int rn, col;
char outbuf[5000];                                  // 150 Races
char buf[MAX_STRING_LENGTH];
 
  outbuf[0] = '\0';
 
  sprintf(outbuf, "************************************************\r\n"); 
  strcat(outbuf, "\r\n"); 
  strcat(outbuf, "    Available races are:\r\n"); 
  strcat(outbuf, "\r\n"); 
  strcat(outbuf, "   "); 
 
  col = 0;
  for ( rn = 0; race_array[rn].loaded; rn++ ) {
     if ( !race_array[rn].pc_race ) continue;
     sprintf( buf, " %-15s", race_array[rn].name );
     strcat(outbuf, buf);
      if ( ++col % 3 == 0 ) strcat(outbuf, "\r\n   " );
  }
 
  if ( col % 3 != 0 ) strcat(outbuf, "\r\n");
  strcat(outbuf, "\r\n"); 
  strcat(outbuf, "************************************************\r\n"); 
  strcat(outbuf, "\r\n"); 
  strcat(outbuf, "Enter <race name>, ?, ? <race_name> or CHAT\r\n"
                            "Which race you like to be?\r\n");

  write_to_buffer(d, outbuf, 0);
  return;
}

/* Give information about a race... */

void prompt_race_info(DESCRIPTOR_DATA *d, char *args) {

  int rn;
 
  char outbuf[5000];                                  
 
  char buf[MAX_STRING_LENGTH];

  SOCIETY *soc;
 
  rn = get_race_rn(args);

  if ( rn == RACE_UNDEFINED ) {

    sprintf(buf, "Race '%s' not defined.\r\n", args);

    write_to_buffer(d, buf, 0);

    return;
  }

  soc = find_society_by_id( race_array[rn].society );

  if ( soc == NULL ) {

    sprintf(buf, "No further information available for race '%s'.\r\n", args);

    write_to_buffer(d, buf, 0);

    return;
  }

  outbuf[0] = '\0';
 
  sprintf(outbuf, "Race: %s\r\n"
                  "%s",
                   race_array[rn].name,
                   soc->desc);   

  strcat(outbuf, "Which race you like to be?\r\n");

  write_to_buffer(d, outbuf, 0);

  return;
}


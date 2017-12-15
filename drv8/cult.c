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
#include "cult.h"
#include "econd.h"
#include "mob.h"


CULT_TYPE cult_array[MAX_CULT];

void save_cults() {
  FILE *fp;
  int rn;
  
  fp = fopen(CULT_FILE, "w");

  for(rn = 1; rn < MAX_CULT; rn++) {
        if (cult_array[rn].name) {
             fprintf( fp, "CULT '%s'\n", cult_array[rn].name);
             fprintf( fp, "%s~\n", cult_array[rn].desc);
             fprintf( fp, "Number %d\n", cult_array[rn].number);
             fprintf( fp, "Align %d\n", cult_array[rn].alignment);
             fprintf( fp, "Power %ld\n\n", cult_array[rn].power);
             if (cult_array[rn].ec) {
                 write_econd(cult_array[rn].ec, fp, "Cond");
             }
        }
  }

  fprintf( fp, "End\n");
  fclose(fp);
  return;
}


void load_cults() {
  FILE *fp;
  char *kwd;
  char code;
  bool done;
  bool ok;
  char *name;
  int rn;
  char buf[MAX_STRING_LENGTH];
  ECOND_DATA *new_ec;


  for(rn = 0; rn < MAX_CULT; rn++) {
    cult_array[rn].loaded = FALSE;
  } 

  rn = 0;
  cult_array[rn].name 		= "unique";
  cult_array[rn].number		= 0;
  cult_array[rn].alignment	                = 0;
  cult_array[rn].power	                = 0;
  cult_array[rn].ec	                                = NULL;
  cult_array[rn].loaded		= TRUE;

  fp = fopen(CULT_FILE, "r");

  if (fp == NULL) {
    log_string("No cult file!");
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

      case '#':
        ok = TRUE;
        fread_to_eol(fp); 
        break; 

      case 'A':
 
        if (!str_cmp(kwd, "Align")) {
          ok = TRUE;
          cult_array[rn].alignment = fread_number(fp);
        }
        break;

      case 'C':
         if (!str_cmp(kwd, "CULT")) {
          ok = TRUE;
          name = fread_word(fp); 

          if (name[0] == '\0') {
            bug("Unnammed cult in cult file!", 0);
            exit(1);
          }

          sprintf(buf, "...Loading cult '%s'...", name);
          log_string(buf);

          rn++;

          if (rn >= MAX_CULT) { 
            bug("To many cults in cult file!",0);
            exit(1);
          }

         /* Initialize data... */ 

          cult_array[rn].name 		= str_dup(name);
          cult_array[rn].desc 		= str_dup(fread_string(fp));
          cult_array[rn].number	= 0;
          cult_array[rn].alignment	= 0;
          cult_array[rn].power       	= 0;
          cult_array[rn].ec         	= NULL;
          cult_array[rn].loaded		= TRUE;
        }

        if (!str_cmp(kwd, "Cond")) {
          ok = TRUE;

           new_ec = read_econd(fp);
           if (new_ec != NULL) {
                 new_ec->next = cult_array[rn].ec;
                 cult_array[rn].ec = new_ec;
           } 
        }
        break; 

      case 'E':
        if (!str_cmp(kwd, "End")) {
          ok = TRUE;
          done = TRUE;
          break; 
        } 
        break; 

      case 'N':
        if (!str_cmp(kwd, "Number")) {
          ok = TRUE;
          cult_array[rn].number = fread_number(fp);
          break;
        }

      case 'P':
 
        if (!str_cmp(kwd, "Power")) {
          ok = TRUE;
          cult_array[rn].power = fread_number(fp);
        }
        break;
     
      default:
        break;
    }

    if (!ok) {
      sprintf(buf, "Unrecognized keyword in cult file: %s", kwd);
      bug(buf, 0); 
      exit(1);
    }
 
  }

  fclose(fp);
  log_string("...cults loaded");
  return;  
}


int get_cult_rn(char *name) {
  int rn;

  for(rn = 0; rn < MAX_CULT; rn++) {

    if (cult_array[rn].name == NULL ) return CULT_UNDEFINED;
    if (!str_cmp(capitalize(name), cult_array[rn].name)) return rn;
  }
  return CULT_UNDEFINED;
}


void list_cult(CHAR_DATA *ch) {
MOB_CMD_CONTEXT *mcc;
int rn;

  sprintf_to_char(ch, "Cults:\r\n");
  sprintf_to_char(ch, "--------------------\r\n");
  for(rn = 1; rn < MAX_CULT; rn++) {
     if (cult_array[rn].name != NULL ) {
          mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL); 
          if (ec_satisfied_mcc(mcc, cult_array[rn].ec, TRUE)) sprintf_to_char(ch, "   %10s (%s)\r\n", cult_array[rn].name, get_align(cult_array[rn].alignment));
          free_mcc(mcc);
     }
  }
  return;
}


void info_cult(CHAR_DATA *ch, char* argument) {
char buf[MAX_STRING_LENGTH];
MOB_CMD_CONTEXT *mcc;
int rn;

       rn = get_cult_rn(argument);
       if (rn == CULT_UNDEFINED) {
           sprintf_to_char(ch, "Cult %s not found.\r\n", argument);
           return;
       }

       mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL); 

       sprintf_to_char(ch, "{g%s{x\r\n", cult_array[rn].name);
       send_to_char("{g--------------------{x\r\n", ch);
       sprintf_to_char(ch, "%s\r\n", cult_array[rn].desc);
       if (cult_array[rn].ec) {
           print_econd_to_buffer(buf,  cult_array[rn].ec, "{WConditions -{x ", mcc);
           send_to_char(buf, ch);
       }
       free_mcc(mcc);
       return;
}

void sac_align(CHAR_DATA *ch, int points) {
int rn;

    rn = get_cult_rn(ch->pcdata->cult);
    if (cult_array[rn].alignment > ch->alignment) ch->alignment = UMIN(ch->alignment + points, cult_array[rn].alignment);
    else ch->alignment = UMAX(ch->alignment - points, cult_array[rn].alignment);

    ch->alignment = UMIN(ch->alignment, 1000);
    ch->alignment = UMAX(ch->alignment, -1000);
    return;
}


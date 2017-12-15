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
#include "society.h"
#include "econd.h"
#include "prof.h"
#include "mob.h"
#include "wev.h"
#include "bank.h"

DECLARE_DO_FUN(do_pload);
DECLARE_DO_FUN(do_punload);
DECLARE_DO_FUN(do_guard_law);
DECLARE_DO_FUN(do_guard_criminals);
DECLARE_DO_FUN(do_guard_imprison);
DECLARE_DO_FUN(do_guard_pardon);
DECLARE_DO_FUN(do_society_reset);
DECLARE_DO_FUN(do_society_revolt);
DECLARE_DO_FUN(do_society_vote);

AREA_DATA *get_area_data	(int vnum);


/* Management for SOCIALITES */

static SOCIALITE *free_socialite_chain = NULL;

SOCIALITE *get_socialite() {

  SOCIALITE *new_socialite;

  if (free_socialite_chain == NULL) {
    new_socialite = (SOCIALITE *) alloc_perm(sizeof(*new_socialite));
  } else {
    new_socialite = free_socialite_chain;
    free_socialite_chain = new_socialite->next;
  }

  new_socialite->name		= NULL;
  new_socialite->rank		= SOC_RANK_NONE;
  new_socialite->authority	                = SOC_AUTH_NONE;
  new_socialite->next 		= NULL;
 
  return new_socialite;
}

void free_socialite(SOCIALITE *old_socialite) {

  if (old_socialite->next != NULL) {
    free_socialite(old_socialite->next);
    old_socialite->next = NULL;
  }

  old_socialite->next = free_socialite_chain;

  free_socialite_chain = old_socialite;

  if (old_socialite->name != NULL) {
    free_string(old_socialite->name);
    old_socialite->name = NULL;
  }

  return;
}

/* Management for SOCADVS */

static SOCADV *free_socadv_chain = NULL;

SOCADV *get_socadv() {

  SOCADV *new_socadv;

  if (free_socadv_chain == NULL) {
    new_socadv = (SOCADV *) alloc_perm(sizeof(*new_socadv));
  } else {
    new_socadv = free_socadv_chain;
    free_socadv_chain = new_socadv->next;
  }

  new_socadv->level		= 0;
  new_socadv->condition		= NULL;
  new_socadv->next 		= NULL;
 
  return new_socadv;
}

void free_socadv(SOCADV *old_socadv) {

  ECOND_DATA *ec;

  if (old_socadv->next != NULL) {
    free_socadv(old_socadv->next);
    old_socadv->next = NULL;
  }

  old_socadv->next = free_socadv_chain;

  free_socadv_chain = old_socadv;

  while (old_socadv->condition != NULL) {
    ec = old_socadv->condition->next;
    free_ec(old_socadv->condition);
    old_socadv->condition = ec;
  }

  return;
}

/* Management for SOCIETYs */

static SOCIETY *free_society_chain = NULL;

SOCIETY *get_society() {

  SOCIETY *new_society;

  if (free_society_chain == NULL) {
    new_society = (SOCIETY *) alloc_perm(sizeof(*new_society));
  } else {
    new_society = free_society_chain;
    free_society_chain = new_society->next;
  }

  new_society->name		= NULL;
  new_society->ctitle1		= NULL;
  new_society->ctitle2		= NULL;
  new_society->ctitle3		= NULL;
  new_society->csoul1		= NULL;
  new_society->csoul2		= NULL;
  new_society->type		= SOC_TYPE_NONE;
  new_society->secret		= FALSE;
  new_society->desc		= NULL;
  new_society->id		                = 0;
  new_society->members		= NULL;
  new_society->profession	                = NULL;
  new_society->next 		= NULL;
  new_society->advconds		= NULL;
  new_society->junta		= NULL;
  new_society->bank		= NULL;
  new_society->votes		= 0;
  new_society->timeout_day	= 0;
  new_society->timeout_month	= 0;
  new_society->tax		= 0;
  new_society->home_area		= 0;

  return new_society;
}

void free_society(SOCIETY *old_society) {

  if (old_society->next != NULL) {
    free_society(old_society->next);
    old_society->next = NULL;
  }

  old_society->next = free_society_chain;

  free_society_chain = old_society;

  if (old_society->name != NULL) {
    free_string(old_society->name);
    old_society->name = NULL;
  }

  if (old_society->desc != NULL) {
    free_string(old_society->desc);
    old_society->desc = NULL;
  }

  if (old_society->members != NULL) {
    free_socialite(old_society->members);
    old_society->members = NULL;
  }

  if (old_society->advconds != NULL) {
    free_socadv(old_society->advconds);
    old_society->advconds = NULL;
  }

  old_society->profession 	= NULL;
  old_society->bank 	= NULL;
  old_society->tax 	= 0;

  if (old_society->junta) {
      free_string(old_society->junta);
      old_society->junta = NULL;
      old_society->votes =0;
      old_society->timeout_day = 0;
      old_society->timeout_month = 0;
  }

  return;
}

/* Management for MEMBERSHIPs */

static MEMBERSHIP *free_membership_chain = NULL;

MEMBERSHIP *get_membership() {

  MEMBERSHIP *new_membership;

  if (free_membership_chain == NULL) {
    new_membership = (MEMBERSHIP *) alloc_perm(sizeof(*new_membership));
  } else {
    new_membership = free_membership_chain;
    free_membership_chain = new_membership->next;
  }

  new_membership->society	= NULL;
  new_membership->profession	= NULL;
  new_membership->next 		= NULL;
 
  return new_membership;
}

void free_membership(MEMBERSHIP *old_membership) {

  if (old_membership->next != NULL) {
    free_membership(old_membership->next);
    old_membership->next = NULL;
  }

  old_membership->next = free_membership_chain;

  free_membership_chain = old_membership;

  old_membership->profession = NULL;
  old_membership->society = NULL;

  return;
}


/* Anchor for all loaded societies... */

static SOCIETY *society_chain;


/* Search and retreive functions... */

SOCIETY *find_society_by_id(int id) {

  SOCIETY *society;

  society = society_chain;

  while ( society != NULL ) {

    if (society->id == id) {
      return society;
    }

    society = society->next;
  } 

  return NULL;
}


SOCADV *find_socadv(SOCIETY *society, int level) {

  SOCADV *socadv;

  socadv = society->advconds;

  while ( socadv != NULL ) {

    if (socadv->level == level) {
      return socadv;
    }

    socadv = socadv->next;
  }

  return NULL;
}


/* Get membership records... */

SOCIALITE *find_socialite(SOCIETY *society, char *name) {
  SOCIALITE *socmemb;

  if (society == NULL) return NULL;
  
  socmemb = society->members;

  while (socmemb != NULL) {

    if (name[0] == socmemb->name[0]) { 
      if (!str_cmp(name, socmemb->name)) {
        return socmemb;
      }
    }

    socmemb = socmemb->next;
  }

  return NULL;
}

/* Get a players membership records... */

MEMBERSHIP *find_membership(CHAR_DATA *ch, SOCIETY *society) {

  MEMBERSHIP *memb;

 /* Check we have something to search... */

  if ( society == NULL
    || ch == NULL
    || ch->memberships == NULL) {
    return NULL;  
  }

  memb = ch->memberships;

  while (memb != NULL) {

    if (memb->society == society) {
      return memb; 
    }

    memb = memb->next;
  } 

  return NULL;
}

/* Load societies from disk... */

static int free_socid = 0;

void load_society_file(char *file_name) {
  SOCIETY *new_society;
  FILE *fp;
  char *kwd;
  char code;
  bool done;
  bool ok;
  char buf[MAX_STRING_LENGTH];
  char *prof_name;
  int lev;
  ECOND_DATA *new_ec;
  SOCADV *lev_socadv;

 /* Find the society file... */

  fp = fopen(file_name, "r");

  if (fp == NULL) {
    sprintf(buf, "Unable to find society file: %s", file_name); 
    log_string(buf);
    exit(1);
    return;
  }

 /* Read through it and see what we've got... */
 
  done = FALSE;

  new_society = NULL;

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

    /* Advance */

      case 'A':

        if (!str_cmp(kwd, "Area")) {
          ok = TRUE;

          if (new_society == NULL) {
            bug("Area before Society in society file!", 0);
            exit(1);
          }

          new_society->home_area = fread_number(fp);
        }
        
        if (!str_cmp(kwd, "Advance")) {
          ok = TRUE;

         /* Must have an active society... */

          if (new_society == NULL) {
            bug("Advance before Society in society file!", 0);
            exit(1);
          }

         /* Read and add... */
   
          lev = fread_number(fp);

          new_ec = read_econd(fp);

          if ( lev < 1 
            || new_ec == NULL ) {
            bug("Bad Advance in society file!", 0);
            exit(1);
          }

          lev_socadv = find_socadv(new_society, lev);

          if (lev_socadv == NULL) {
            lev_socadv = get_socadv();
            lev_socadv->level = lev;
            lev_socadv->next = new_society->advconds;
            new_society->advconds = lev_socadv;
          }

          new_ec->next = lev_socadv->condition; 
          lev_socadv->condition = new_ec;

        }
        break; 

      case 'B':
        
        if (!str_cmp(kwd, "Bank")) {

          ok = TRUE;

         /* Must have an active society... */

          if (new_society == NULL) {
            bug("Bank before Society in society file!", 0);
            exit(1);
          }

          new_society->bank = strdup(fread_string(fp));
        }
        break; 

    /* Desc */

      case 'D':
        
        if (!str_cmp(kwd, "Desc")) {

          ok = TRUE;

         /* Must have an active society... */

          if (new_society == NULL) {
            bug("Desc before Society in society file!", 0);
            exit(1);
          }

         /* Read and add... */
   
          new_society->desc = strdup(fread_string(fp));

        }

        break; 

    /* Id */

      case 'I':
        
        if (!str_cmp(kwd, "Id")) {

          ok = TRUE;

         /* Must have an active society... */

          if (new_society == NULL) {
            bug("Id before Society in society file!", 0);
            exit(1);
          }

         /* Read and add... */
   
          new_society->id = fread_number(fp);

          free_socid = UMAX(free_socid, new_society->id);        

        }

        break; 


    /* Membership (advance to level 0)*/

      case 'M':
        
        if (!str_cmp(kwd, "Membership")) {

          ok = TRUE;

         /* Must have an active society... */

          if (new_society == NULL) {
            bug("Membership before Society in society file!", 0);
            exit(1);
          }

         /* Read and add... */
   
          new_ec = read_econd(fp);

          if ( new_ec == NULL ) {
            bug("Bad Membership in society file!", 0);
            exit(1);
          }

          lev_socadv = find_socadv(new_society, SOC_LEVEL_MEMBERSHIP);

          if (lev_socadv == NULL) {
            lev_socadv = get_socadv();
            lev_socadv->level = SOC_LEVEL_MEMBERSHIP;
            lev_socadv->next = new_society->advconds;
            new_society->advconds = lev_socadv;
          }

          new_ec->next = lev_socadv->condition; 
          lev_socadv->condition = new_ec;

        }

        break; 

    /* Profession */

      case 'P':
        
        if (!str_cmp(kwd, "Profession")) {

          ok = TRUE;

         /* Must have an active society... */

          if (new_society == NULL) {
            bug("Profession before Society in society file!", 0);
            exit(1);
          }

         /* Read and add... */

          prof_name = fread_word(fp);

          new_society->profession = get_profession(prof_name);

          if (new_society->profession == NULL) {
            log_string(prof_name);
            bug("Bad Profession in society file!", 0);
            exit(1);
          }

        }

        break; 

    /* Society */

      case 'S':
        
        if (!str_cmp(kwd, "Society")) {

          ok = TRUE;

         /* Get a new society... */

          new_society = get_society();

          new_society->next = society_chain;
          society_chain = new_society;

         /* Read and add... */
   
          new_society->name = strdup(fread_word(fp));
          new_society->ctitle1=strdup("Member");
          new_society->ctitle2=strdup("Council");
          new_society->ctitle3=strdup("Leader");
          new_society->csoul1=strdup("You look a bit confused, because you don't know your secret clan sign.\r\n");
          new_society->csoul2=strdup("@a2 really looks puzzled.\r\n");
        }

        if (!str_cmp(kwd, "Sign")) {

          ok = TRUE;

         /* Must have an active society... */

          if (new_society == NULL) {
            bug("Sign before Society in society file!", 0);
            exit(1);
          }

         /* Read and add... */
   
          free_string(new_society->csoul1);
          free_string(new_society->csoul2);
          new_society->csoul1 = strdup(fread_string(fp));
          new_society->csoul2 = strdup(fread_string(fp));

        }

        break; 

    /* Type */

      case 'T':
        
        if (!str_cmp(kwd, "Type")) {

          ok = TRUE;

         /* Must have an active society... */

          if (new_society == NULL) {
            bug("Type before Society in society file!", 0);
            exit(1);
          }

         /* Read and add... */
   
          new_society->type = fread_number(fp);

         /* Extract secret flag... */

          if (new_society->type < 0) {
            new_society->secret = TRUE;
            new_society->type *= -1;
          }

         /* Validate... */

          if ( new_society->type == SOC_TYPE_NONE 
            || new_society->type > SOC_TYPE_MAX ) {
            bug("Bad society type %d in society file!", new_society->type);
            new_society->type = SOC_TYPE_NONE;
          }

        }

        if (!str_cmp(kwd, "T1")) {

          ok = TRUE;

         /* Must have an active society... */

          if (new_society == NULL) {
            bug("Title before Society in society file!", 0);
            exit(1);
          }

         /* Read and add... */
   
          free_string(new_society->ctitle1);
          new_society->ctitle1 = strdup(fread_string(fp));

        }

        if (!str_cmp(kwd, "T2")) {

          ok = TRUE;

         /* Must have an active society... */

          if (new_society == NULL) {
            bug("Title before Society in society file!", 0);
            exit(1);
          }

         /* Read and add... */
   
          free_string(new_society->ctitle2);
          new_society->ctitle2 = strdup(fread_string(fp));

        }

        if (!str_cmp(kwd, "T3")) {

          ok = TRUE;

         /* Must have an active society... */

          if (new_society == NULL) {
            bug("Title before Society in society file!", 0);
            exit(1);
          }

         /* Read and add... */
   
          free_string(new_society->ctitle3);
          new_society->ctitle3 = strdup(fread_string(fp));
        }

        break;
  
     /* File ends with an E */

      case 'E':

       /* End */

        if (!str_cmp(kwd, "End")) {
          ok = TRUE;
          done = TRUE;
        }

       /* Expulsion */

        if (!str_cmp(kwd, "Expulsion")) {

          ok = TRUE;

         /* Must have an active society... */

          if (new_society == NULL) {
            bug("Expulsion before Society in society file!", 0);
            exit(1);
          }

         /* Read and add... */
   
          new_ec = read_econd(fp);

          if ( new_ec == NULL ) {
            bug("Bad Expulsion in Society file!", 0);
            exit(1);
          }

          lev_socadv = find_socadv(new_society, SOC_LEVEL_EXPEL);

          if (lev_socadv == NULL) {
            lev_socadv = get_socadv();
            lev_socadv->level = SOC_LEVEL_EXPEL;
            lev_socadv->next = new_society->advconds;
            new_society->advconds = lev_socadv;
          }

          new_ec->next = lev_socadv->condition; 
          lev_socadv->condition = new_ec;

        }

        break; 

     /* Anything else is an error... */

      default:
        break;
    }

    if (!ok) {
      sprintf(buf, "Unrecognized keyword in society file: %s", kwd);
      bug(buf, 0); 
      exit(1);
    }
 
  }

 /* Close it now we're finished with it... */ 

  fclose(fp);

 /* All done... */

  return;
}

void load_societies() {

  load_society_file(SOCIETY_FILE);

  load_society_file(SOCIETY2_FILE);

  log_string("...societies loaded");

  return;
}

void load_society_members() {
  SOCIETY *society;
  FILE *fp;
  char code;
  bool done;
  bool ok;
  int socId;
  char buf[MAX_STRING_LENGTH];
  char *name;

  SOCIALITE *new_socialite;

 /* Find the society membership file... */

  fp = fopen(SOCIETY_MEMBERS_FILE, "r");

  if (fp == NULL) {
    log_string("No society membership file!");
    exit(1);
    return;
  }

 /* Read through it and see what we've got... */
 
  done = FALSE;

  while(!done) {
    code = fread_letter(fp);
    ok = FALSE;

    switch (code) {

    /* Comments */

      case '#':
        
        ok = TRUE;

        fread_to_eol(fp); 

        break; 

    /* Advance */

      case 'M':
        ok = TRUE;
        socId = fread_number(fp);
        new_socialite = get_socialite();
        new_socialite->rank = fread_number(fp);
        new_socialite->authority = fread_number(fp);
        new_socialite->name = strdup(fread_word(fp));
        name = fread_word(fp);
        if (!str_cmp(name,"yes")) new_socialite->vote_cast = TRUE;
        else new_socialite->vote_cast = FALSE;

        society = find_society_by_id(socId);

        if (society == NULL) {
          bug("Missing society %n!", socId);
          exit(1);
        }

        new_socialite->next = society->members;
        society->members = new_socialite;

        break; 


      case 'S':
        ok = TRUE;
        socId = fread_number(fp);
        society = find_society_by_id(socId);

        if (society == NULL) {
          bug("Missing society %n!", socId);
          exit(1);
        }

         society->votes = fread_number(fp);
         society->timeout_day = fread_number(fp);
         society->timeout_month = fread_number(fp);
         name = fread_word(fp);
         if (name) society->junta = strdup(name);

        break; 

      case 'T':
        ok = TRUE;
        socId = fread_number(fp);
        society = find_society_by_id(socId);

        if (society == NULL) {
          bug("Missing society %n!", socId);
          exit(1);
        }

        society->tax = fread_number(fp);
        break; 

     /* File ends with an X */

      case 'X':

        ok = TRUE;
        done = TRUE;
        break; 

     /* Anything else is an error... */

      default:
        break;
    }

    if (!ok) {
      sprintf(buf, "Unrecognized letter in society membership file: %c", code);
      bug(buf, 0); 
      exit(1);
    }
 
  }

 /* Close it now we're finished with it... */ 

  fclose(fp);

 /* All done... */

  log_string("...society memberships loaded");

  return;
}

/* Save socialite data... */

void save_socialites() {
  SOCIETY *society;
  SOCIALITE *social;
  FILE *fp;

 /* Find the society membership file... */

  fp = fopen(SOCIETY_MEMBERS_FILE, "w");

  if (fp == NULL) {
    log_string("Unable to open society membership file for writing!");
    return;
  }

 /* Output a header... */

  fprintf(fp, "# Society membership data\n"  "#\n");

 /* Chew through the societies... */ 

  society = society_chain;

  while (society != NULL) {

    fprintf(fp, "# Members for %s\n"  "#\n"   "#  S  R  A  Name\n", society->name);

    social = society->members;

    while (social != NULL) {
      fprintf(fp, "M %2d %2d %2d '%s' '%s'\n", society->id, social->rank, social->authority, social->name, social->vote_cast ? "yes" : "no");
      social = social->next;
    }
    fprintf(fp, "T %2d %2d\n", society->id, society->tax);

    if (society->junta)  fprintf(fp, "S %2d %2d %2d %2d '%s'\n", society->id, society->votes, society->timeout_day, society->timeout_month, society->junta);
    fprintf(fp, "#\n");

    society = society->next;
  }

 /* Write out the end of file marker... */
 
  fprintf(fp, "X\n");

 /* Close it now we're finished with it... */ 

  fclose(fp);

 /* All done... */

  log_string("...society memberships saved");

  return;
}

/* Write a characters society memberships out... */

void save_char_society(CHAR_DATA *ch, FILE *fp) {
MEMBERSHIP *memb;
bool done[500];
int i;

  for(i=0; i<500; i++) done[i]=FALSE; 

  memb = ch->memberships;

  while (memb != NULL) {
    if (memb->society != NULL
    && !done[memb->society->id]) {
      fprintf(fp, "Society %d %d\n", memb->society->id, memb->level);
      done[memb->society->id] = TRUE;
    }

    memb = memb->next;
  } 

  return;
}

MEMBERSHIP *read_char_society(FILE *fp) {

  SOCIETY *soc;

  MEMBERSHIP *memb;

  int id, lev;

 /* Read the raw numbers from the file... */

  id = fread_number(fp);
  lev = fread_number(fp);

 /* Find and check society... */ 

  soc = find_society_by_id(id);

  if (soc == NULL) {
    bug("Bad society id (%d)!", id);
    return NULL;
  }

 /* Create membership... */

  memb = get_membership();

 /* Add profession and level, if appropriate... */

  memb->society = soc;
  memb->level = lev;
  memb->profession = soc->profession;

 /* All done... */

  return memb;
}

/* See if a player is a member of a society... */

bool is_member(CHAR_DATA *ch, SOCIETY *society) {
 
  SOCIALITE *social;

  if (IS_NPC(ch)) {
    social = find_socialite(society, ch->short_descr);
  } else {
    social = find_socialite(society, ch->name);
  }

  if (social == NULL) {
    return FALSE;
  } 

  if ( social->rank < SOC_RANK_MEMBER 
    || social->rank > SOC_RANK_LEADER) {
    return FALSE;
  } 

  return TRUE;
}

/* Create membership records... */

void create_membership(CHAR_DATA *ch, SOCIETY *society, int rank) {

  SOCIALITE *social;

  MEMBERSHIP *memb;

 /* Construct the socialite... */

  social = get_socialite();

  if (IS_NPC(ch)) {
    social->name = str_dup(ch->short_descr);
  } else if (ch->name != NULL) { 
    social->name = str_dup(ch->name);
  } else {
    free_socialite(social);
    bug("Trying to create membership with no name/short_descr", 0);
    return;
  }
   
  social->rank = rank; 
  social->authority = SOC_AUTH_NONE;
 
  social->next = society->members;
  society->members = social;

 /* Construct the membership... */

  memb = get_membership();

 /* Add profession and level, if appropriate... */

  memb->society = society;
  memb->level = SOC_LEVEL_MEMBERSHIP;
  memb->profession = society->profession;

 /* Splice in... */

  memb->next = ch->memberships;
  ch->memberships = memb;

 /* All done... */

  return;
}

/* Default a players membership data... */

void default_memberships(CHAR_DATA *ch) {
  SOCIETY *society;
  MEMBERSHIP *memb, *memb_next;
  LPROF_DATA *prof;
  int race;

 /* Ok, we just need to set up a race membership... */

  if (!IS_AFFECTED(ch, AFF_POLY)) {
       race = 100 + ch->race;
       society = find_society_by_id(race);

       if (society == NULL) {
         bug("Bad race/society %d", race);
         return;
       } 

       if (society->type != SOC_TYPE_RACE) {
         bug("Bad race/society type %d", race);
         return;
       } 

       if (!is_member(ch, society)) create_membership(ch, society, SOC_RANK_MEMBER);
  }

   /* Check Professional membership... */

    prof = ch->profs;
    while (prof != NULL) {
       if (prof->profession->default_soc != 0) {
            society = find_society_by_id(prof->profession->default_soc);
            if (society != NULL
            && !is_member(ch, society)) {
                 create_membership(ch, society, SOC_RANK_MEMBER);
            }
       }
       prof = prof->next;
    }

    if (ch->memberships) {
         for (memb = ch->memberships; memb->next; memb = memb_next) {
              memb_next = memb->next;

              if (memb_next->society->type == SOC_TYPE_RACE) {
                   if (ch->race != memb_next->society->id - 100
                   && ch->were_type != memb_next->society->id - 100) {
                            memb->next = memb_next->next;
                            memb_next = memb->next;
                            if (!memb_next) break;
                   }
              }
         }
    }

    memb = ch->memberships;
    if (memb) {
         if (memb->society->type == SOC_TYPE_RACE) {
                   if (ch->race != memb->society->id - 100
                   && ch->were_type != memb->society->id - 100) {
                            memb = memb->next;
                   }
         }
    }

    save_socialites();
    return;
}

/* Check a players membership data... */

void check_memberships(CHAR_DATA *ch) {
  SOCIETY *society;
  SOCIALITE *social;
  MEMBERSHIP *memb;
  bool changed;

  if (IS_AFFECTED(ch, AFF_POLY)) return;
  if (ch->desc != NULL) {
      if (ch->desc->original != NULL) return;
  }
  
 /* Check the players memberships... */

  changed = FALSE;

  memb = ch->memberships;

  while (memb != NULL) {

    if (memb->level >= SOC_LEVEL_INVITED) {

      society = memb->society;

      if (society != NULL) {

        if (IS_NPC(ch)) {
          social = find_socialite(society, ch->short_descr);
        } else {
          social = find_socialite(society, ch->name);
        }

        if (social == NULL) {

          if (memb->level > SOC_LEVEL_INVITED) {

            if (memb->society->type == SOC_TYPE_RACE) {
                   if (ch->race == memb->society->id - 100
                   || ch->were_type == memb->society->id - 100) { 
                     printf("[*****] Fixing missing socialite:\n"
                            "[*****] for %s\n"
                            "[*****] to the '%s'\n", ch->name, society->name);

                     social = get_socialite();
                     social->name = str_dup(ch->name);
                     social->rank = SOC_RANK_MEMBER; 
                     social->authority = SOC_AUTH_NONE;
                     social->next = society->members;
                     society->members = social;
                     changed = TRUE;
                   }
            } else {
                     printf("[*****] Fixing missing socialite:\n"
                            "[*****] for %s\n"
                            "[*****] to the '%s'\n", ch->name, society->name);

                     social = get_socialite();
                     social->name = str_dup(ch->name);
                     social->rank = SOC_RANK_MEMBER; 
                     social->authority = SOC_AUTH_NONE;
                     social->next = society->members;
                     society->members = social;
                     changed = TRUE;
            }
          }
        } else {
          
          if (memb->level == SOC_LEVEL_INVITED) {
            if (memb->society->type == SOC_TYPE_RACE) {
                   if (ch->race == memb->society->id - 100
                   || ch->were_type == memb->society->id - 100) { 
                     printf("[*****] Fixing missing socialite:\n"
                            "[*****] for %s\n"
                            "[*****] to the '%s'\n", ch->name, society->name);
                     if (memb->level < SOC_LEVEL_FOE) {
                          memb->level = SOC_LEVEL_FOE;
                          changed = TRUE;
                     }
                   }
            } else {
                     printf("[*****] Fixing missing socialite:\n"
                            "[*****] for %s\n"
                            "[*****] to the '%s'\n", ch->name, society->name);
                     if (memb->level < SOC_LEVEL_FOE) {
                          memb->level = SOC_LEVEL_FOE;
                          changed = TRUE;
                     }
            }
          }
        }
      } 
    }

    memb = memb->next;
  }

 /* Resave membership data if we must... */
 
  if (changed) save_socialites();

 /* Now check what all of the societies think... */

  society = society_chain;

  while (society != NULL) {

    if (IS_NPC(ch)) {
      social = find_socialite(society, ch->short_descr);
    } else {
      social = find_socialite(society, ch->name);
    }

    if (social != NULL) {
      memb = find_membership(ch, society);

      if (memb == NULL) {
            if (society->type == SOC_TYPE_RACE) {
                   if (ch->race == society->id - 100
                   || ch->were_type == society->id - 100) { 

                         printf("[*****] Fixing missing membership:\n"
                                    "[*****] for %s\n"
                                    "[*****] to the '%s'\n", ch->name, society->name);

                         memb = get_membership();
                         memb->society = society;
                         memb->level = SOC_LEVEL_MEMBERSHIP;
                         memb->profession = society->profession;
                         memb->next = ch->memberships;
                         ch->memberships = memb;
                   }
            } else {
                         printf("[*****] Fixing missing membership:\n"
                                    "[*****] for %s\n"
                                    "[*****] to the '%s'\n", ch->name, society->name);

                         memb = get_membership();
                         memb->society = society;
                         memb->level = SOC_LEVEL_MEMBERSHIP;
                         memb->profession = society->profession;
                         memb->next = ch->memberships;
                         ch->memberships = memb;
            }
      } 
    } 

    society = society->next;
  }

  return;
}

/* Is a skill available to a character... */

bool skill_available_by_society(int sn, CHAR_DATA *ch) {

  MEMBERSHIP *memb;
  PROF_DATA *prof;

 /* Check each membership in turn... */

  memb = ch->memberships;

  while (memb != NULL) {

    prof = memb->profession;

   /* See if the society has a profession... */

    if (prof != NULL) {

     /* See if the skill is allowed to the profession... */

      if (prof->levels->skill[sn] != -1) { 

       /* Check if chars level within the society is high enough... */

        if (prof->levels->skill[sn] <= memb->level) { 
          return TRUE;
        }
      }
    } 

    memb = memb->next;
  }

  return FALSE;
}

/* Process commands... */

bool society_action_promote(SOCIETY *society, CHAR_DATA *ch) {
  SOCIALITE *socialite;
  MEMBERSHIP *memb;

 /* Validate... */

  if ( society == NULL
    || ch == NULL ) {
    return FALSE;
  }

 /* First we find the socalite and membership records... */

  if (IS_NPC(ch)) {
    socialite = find_socialite(society, ch->short_descr);
  } else {
    socialite = find_socialite(society, ch->name);
  }

  memb = find_membership(ch, society);

 /* New/lost member? */

  if (socialite == NULL) {

   /* New member... */

    if (memb == NULL) {

     /* Create membership... */

      memb = get_membership();

     /* Add profession and level, if appropriate... */

      memb->society = society;
      memb->level = SOC_LEVEL_INVITED;
      memb->profession = society->profession;

     /* Splice in... */

      memb->next = ch->memberships;
      ch->memberships = memb;

     /* That'll do for now... */

      return TRUE; 
    } 

   /* Invitee taking up the offer? */

    if (memb->level == SOC_LEVEL_INVITED) {

      memb->level = SOC_LEVEL_MEMBERSHIP;

      socialite = get_socialite();

      socialite->name = str_dup(ch->name);
 
      socialite->rank = SOC_RANK_MEMBER; 
      socialite->authority = SOC_AUTH_NONE;

      socialite->next = society->members;
      society->members = socialite;

      save_socialites();

      return TRUE;
    }

   /* Hmmm. Looks like we lost a socialite record... */

    bug("Societies - missing socialite!", 0);

    socialite = get_socialite();

    socialite->name = str_dup(ch->name);
 
    socialite->rank = SOC_RANK_MEMBER; 
    socialite->authority = SOC_AUTH_NONE;

    socialite->next = society->members;
    society->members = socialite;

    save_socialites();

    return TRUE;

  }

 /* Check for matching membership... */

  if (memb == NULL) {

    bug("Society - missing membership!", 0);

   /* Create membership... */

    memb = get_membership();

   /* Add profession and level, if appropriate... */

    memb->society = society;
    memb->level = SOC_LEVEL_INVITED;
    memb->profession = society->profession;

   /* Splice in... */

    memb->next = ch->memberships;
    ch->memberships = memb;

  }

 /* Now, we definetly have both membership and socialite... */

  switch (socialite->rank) {
    case SOC_RANK_SUSPENDED: 
      socialite->rank = SOC_RANK_MEMBER;
      break;

    case SOC_RANK_MEMBER: 
      socialite->rank = SOC_RANK_COUNCIL;
      break;

    case SOC_RANK_COUNCIL: 
      socialite->rank = SOC_RANK_LEADER;
      break;

    case SOC_RANK_LEADER: 
      break;

    default:
      socialite->rank = SOC_RANK_MEMBER;
      break;
  }

  save_socialites();

 /* All done... */
 
  return TRUE;
}

/* Expel a character from a society... */

bool society_action_expel(SOCIETY *society, CHAR_DATA *ch) {
  SOCIALITE *social, *prev_social;
  MEMBERSHIP *memb, *prev_memb;

 /* Validate... */

  if ( society == NULL
  || ch == NULL ) {
    return FALSE;
  }

 /* First we destroy the membership... */

  prev_memb = NULL;
  
  memb = ch->memberships;
 
  while ( memb != NULL
       && memb->society != society) {

    prev_memb = memb;
    memb = memb->next;
  }

  if (memb != NULL) {

    if (prev_memb == NULL) {
      ch->memberships = memb->next;
    } else {
      prev_memb->next = memb->next;
    }

    memb->next = NULL;
 
    free_membership(memb);

    memb = NULL;  
  }

 /* Then we destroy the socialite... */

  prev_social = NULL;

  social = society->members;

  while ( social != NULL
       && social->name[0] != ch->name[0]
       && str_cmp(social->name, ch->name)) {

    prev_social = social;
    social = social->next;
  }

  if (social != NULL) {
    
    if (prev_social == NULL) {
      society->members = social->next;
    } else {
      prev_social->next = social->next;
    }

    social->next = NULL;
   
    free_socialite(social);  

    social = NULL;
  }

  save_socialites();
 
 /* All done... */

  return TRUE;
}

/* Reduce a characters rank... */

bool society_action_demote(SOCIETY *society, CHAR_DATA *ch) {

  SOCIALITE *socialite;

 /* Validate... */

  if ( society == NULL
    || ch == NULL ) {
    return FALSE;
  }

 /* First we find the socalite and membership records... */

  if (IS_NPC(ch)) {
    socialite = find_socialite(society, ch->short_descr);
  } else {
    socialite = find_socialite(society, ch->name);
  }

 /* No socialite or only a member means expulsion... */

  if ( socialite == NULL ) {
    return society_action_expel(society, ch);
  }
 
  if ( socialite->rank != SOC_RANK_COUNCIL
    && socialite->rank != SOC_RANK_LEADER ) {
    return society_action_expel(society, ch);
  } 

 /* Ok, demotion time... */

  switch (socialite->rank) {
    
    case SOC_RANK_COUNCIL:
      socialite->rank = SOC_RANK_MEMBER;
      break;

    case SOC_RANK_LEADER:
      socialite->rank = SOC_RANK_COUNCIL;
      break;  

    default:
      return society_action_expel(society, ch);
  }

  save_socialites();

  return TRUE;
}

/* See if a player is up to the societies standards... */

bool society_action_test(SOCIETY *society, CHAR_DATA *ch, int level) {

  SOCADV *socadv;

  MOB_CMD_CONTEXT *mcc;

 /* Validate... */

  if ( society == NULL
    || ch == NULL ) {
    return FALSE;
  }

 /* Find the socadv... */

  socadv = find_socadv(society, level);

 /* If it's not there, they have failed... */

  if (socadv == NULL) {
    return FALSE;
  }

 /* Return the result of the test... */

  mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL);

  if (ec_satisfied_mcc(mcc, socadv->condition, TRUE)) {
    return TRUE;
  }

 /* All done... */
 
  return FALSE;
}

bool society_action_foe(SOCIETY *society, CHAR_DATA *ch) {

  MEMBERSHIP *memb;

 /* Validate... */

  if ( society == NULL
    || ch == NULL ) {
    return FALSE;
  }

 /* First we find the membership record... */

  memb = find_membership(ch, society);

 /* Victim cannot be a member... */

  if (memb != NULL) {
    return FALSE;
  }

 /* Create membership... */

  memb = get_membership();

 /* Add profession and level, if appropriate... */

  memb->society = society;
  memb->level = SOC_LEVEL_FOE;
  memb->profession = NULL;

 /* Splice in... */

  memb->next = ch->memberships;
  ch->memberships = memb;

 /* That'll do for now... */

  return TRUE; 
} 

bool society_action_pardon(SOCIETY *society, CHAR_DATA *ch) {

  MEMBERSHIP *memb;

 /* Validate... */

  if ( society == NULL
    || ch == NULL ) {
    return FALSE;
  }

 /* First we find the membership record... */

  memb = find_membership(ch, society);

 /* Victim must be a foe... */

  if (memb == NULL) {
    return FALSE;
  }

  if (memb->level != SOC_LEVEL_FOE) {
    return FALSE;
  } 

 /* Destroy the foe record... */

  society_action_expel(society, ch);

 /* That'll do for now... */

  return TRUE; 
} 


/* Interactive commands... */

char *soc_rank_name(  SOCIETY *society, int rank) {
 
  switch (rank) {
    case SOC_RANK_NONE:
      return "None";

    case SOC_RANK_MEMBER:
    case SOC_RANK_SUSPENDED:
      if (society->ctitle1 == NULL) return "Member";
      return society->ctitle1;

    case SOC_RANK_COUNCIL:
      if (society->ctitle2 == NULL) return "Council";
      return society->ctitle2;

    case SOC_RANK_LEADER:
      if (society->ctitle3 == NULL) return "Leader";
      return society->ctitle3;

    default:
      break;
  }

  return "{RUnknown{x";
}

void soc_auth_names(int auth, char *buf) {
  bool found;
  buf[0] = '\0';

  if (auth == SOC_AUTH_NONE) {
    sprintf(buf, "None");
    return;
  }

  found = FALSE;

  if (IS_SET(auth, SOC_AUTH_INVITE)) {
    strcat(buf, "invite");
    found = TRUE;
  }

  if (IS_SET(auth, SOC_AUTH_EXPEL)) { 
    if (found) strcat(buf, " ");
    strcat(buf, "expel");
    found = TRUE;
  }

  if (IS_SET(auth, SOC_AUTH_BANK)) {
    if (found) strcat(buf, " ");
    strcat(buf, "bank");
    found = TRUE;
  }

  if (IS_SET(auth, SOC_AUTH_TAX)) {
    if (found) strcat(buf, " ");
    strcat(buf, "tax");
    found = TRUE;
  }

  if (IS_SET(auth, SOC_AUTH_AUTH)) {
    if (found) strcat(buf, " ");
    strcat(buf, "auth");
    found = TRUE;
  }

  if (IS_SET(auth, SOC_AUTH_PROMOTE)) {
    if (found) strcat(buf, " ");
    strcat(buf, "promote");
    found = TRUE;
  }

  if (IS_SET(auth, SOC_AUTH_DEMOTE)) {
    if (found) strcat(buf, " ");
    strcat(buf, "demote");
    found = TRUE;
  }

  if (IS_SET(auth, SOC_AUTH_TEST)) {
    if (found) strcat(buf, " ");
    strcat(buf, "test");
    found = TRUE;
  }

  if (IS_SET(auth, SOC_AUTH_FOE)) {
    if (found) strcat(buf, " ");
    strcat(buf, "foe");
    found = TRUE;
  }

  if (IS_SET(auth, SOC_AUTH_PARDON)) {
    if (found) strcat(buf, " ");
    strcat(buf, "pardon");
    found = TRUE;
  }

  if (IS_SET(auth, SOC_AUTH_LAW)) {
    if (found) strcat(buf, " ");
    strcat(buf, "law");
    found = TRUE;
  }

  if (!found) sprintf(buf, "{RUnknown{x");
  return;
}

/* Bank command... */

void do_society_bank(CHAR_DATA *ch, char *argument) {
  CHAR_DATA *teller;
  char buf[MAX_STRING_LENGTH];
  char soc_acnt[MAX_STRING_LENGTH];
  char ssid[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  int sid, currency;
  SOCIETY *society;
  SOCIALITE *social;
  bool bank = FALSE;
  SHOP_DATA *pShop;

  argument = one_argument(argument, ssid);
  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);
  argument = one_argument(argument, arg3);

  if ( (arg1[0] == '\0')
  || (!str_cmp(arg1,"help"))) {
     send_to_char("\r\nSociety Bank Commands:{x\r\n\r\n", ch);
     send_to_char("  society bank <sid> balance\r\n", ch);
     send_to_char("  society bank <sid> desposit <ammount> [currency]\r\n", ch);
     send_to_char("  society bank <sid> withdraw <ammount> [currency]\r\n", ch);
     send_to_char("  society bank <sid> help\r\n", ch);
     send_to_char("\r\nInterest is 1% per game month.\r\n", ch);
     return;
  }

 /* Find the society... */

  sid = atoi(ssid);

  if ( sid < 0
    || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

 /* Are they allowed to authorize? */

  if (!IS_IMMORTAL(ch)) {
      if (IS_NPC(ch)) {
          social = find_socialite(society, ch->short_descr);
      } else {
          social = find_socialite(society, ch->name);
      }
   
      if (social == NULL) {
          send_to_char("You are not a full member yourself!\r\n", ch);
          return;
      }
  } 
 
 /* Safty check... */

  if (ch->in_room == NULL) {
    send_to_char("This is not a bank!", ch);
    return;
  }

 /* See if we can find a teller... */ 

  teller = find_teller(ch);
  if ( teller == NULL ) return;

  pShop = teller->pIndexData->pShop;
  if (pShop != NULL ) {
      int i;

      for(i = 0; i < MAX_TRADE; i++) {
          if (pShop->buy_type[i] == 999) bank = TRUE;
      }
  }
  if (!bank) {
      do_say(teller, "We only change money.");
      return;
  }


 /* Yep, so say hi... */ 

  sprintf(buf, "Welcome to the {w%s{g", ch->in_room->name);
  do_say(teller, buf);

  sprintf(soc_acnt, "Society_%d", sid);

 /* Balance... */

  if (!str_cmp(arg1,"balance")) {
    do_balance(teller, ch, ch->in_room->name, soc_acnt);
    return;
  }

 /* Deposit... */

  if (!str_cmp(arg1, "deposit")) {
    if (arg3[0] != '\0') {
        currency = flag_value(currency_accept, arg3);
        if (currency < 0) {
           do_say(teller, "Deposit what?");
           return;
        }
    } else {
        currency = 0;
    }
    do_deposit(teller, ch, arg2, currency, ch->in_room->name, soc_acnt);
    return; 
  }   

 /* Withdraw... */

  if (!str_cmp(arg1, "withdraw")) {

    if (!IS_IMMORTAL(ch)) {
        if (IS_NPC(ch)) {
          social = find_socialite(society, ch->short_descr);
        } else {
          social = find_socialite(society, ch->name);
        }

        if (!social) {
             send_to_char("You do not have authority to do that!\r\n", ch);
             return;
        }

        if (!IS_SET(social->authority, SOC_AUTH_BANK)) {
             send_to_char("You do not have authority to do that!\r\n", ch);
             return;
        }
    }

    if (arg3[0] != '\0') {
        currency = flag_value(currency_accept, arg3);
        if (currency < 0) {
           do_say(teller, "Deposit what?");
           return;
        }
    } else {
        currency = 0;
    }

    do_withdraw(teller, ch, arg2, currency, ch->in_room->name, soc_acnt);
    return;
  }

 /* Service not available... */

  do_say(teller, "Sorry, I don't know how to do that.");

 /* All done... */

  return;
} 

/* Test a member to see if they are good enough... */

void do_society_test(CHAR_DATA *ch, char *args) {

  CHAR_DATA *invitee;

  SOCIETY *society;

  SOCIALITE *social;

  MEMBERSHIP *memb;

  char char_name[MAX_STRING_LENGTH];

  int sid;

  MOB_CMD_CONTEXT *mcc;

  WEV *wev;

  int rank;
  
  bool ok;

 /* First, chop up the parms... */

  args = one_argument(args, char_name);

 /* No-one to test means we have a problem... */

  if (char_name[0] == '\0') {
    send_to_char("Who do you want to test?\r\n", ch);
    return;
  }

 /* Who is being promoted? */

  invitee = get_char_room(ch, char_name);

  if (invitee == NULL) {
    send_to_char("They are not here!\r\n", ch);
    return;
  }

 /* Find the society... */

  sid = atoi(args);

  if ( sid < 0
    || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

 /* Are they allowed to authorize? */

  if (!IS_IMMORTAL(ch)) {

    if (IS_NPC(ch)) {
      social = find_socialite(society, ch->short_descr);
    } else {
      social = find_socialite(society, ch->name);
    }
   
    if (social == NULL) {
      send_to_char("You are not a full member yourself!\r\n", ch);
      return;
    }

    if (!IS_SET(social->authority, SOC_AUTH_TEST)) {
      send_to_char("You do not have authority to do that!\r\n", ch);
      return;
    }

    rank = social->rank;
  } else {
    rank = SOC_RANK_LEADER;  
  }
 
 /* Check if they are already a member? */

  if (IS_NPC(ch)) {
    social = find_socialite(society, invitee->short_descr);
  } else {
    social = find_socialite(society, invitee->name);
  }
   
  if (social == NULL) {
    send_to_char("They are not a member!\r\n", ch);
    return;
  }

 /* Check for sillyness... */
 
  if (!IS_IMMORTAL(ch)) {

    if (social->rank > rank) {
      send_to_char("You cannot test someone of higher rank!\r\n", ch);
      return;
    }

    if (invitee == ch) {
      send_to_char("You cannot test yourself!\r\n", ch);
      return;
    }
  }

 /* Build WEV, check for permission... */

  mcc = get_mcc(ch, ch, invitee, NULL, NULL, NULL, sid, society->name);

  wev = get_wev(WEV_SOCIETY, WEV_SOCIETY_TEST, mcc,
                "You test @v2 within the '@t0'.\r\n",
                "@a2 tests you within the '@t0'.\r\n",
                "@a2 tests @v2 within the '@t0'.\n\t"); 

 /* Challange... */

  if (!room_issue_wev_challange(ch->in_room, wev)) {
    free_wev(wev);
    return;
  }

 /* Tell the world... */

  room_issue_wev(ch->in_room, wev);

  free_wev(wev);

 /* First test for expulsion... */

  ok = society_action_test(society, invitee, SOC_LEVEL_EXPEL);

  if (!ok) {

    send_to_char("{YYou are no longer fit to be a member!{x\r\n", invitee);
    send_to_char("{YThey are no longer fit to be a member!{x\r\n", ch);

    society_action_expel(society, invitee); 

    return;
  }

 /* Further testing is only relevent for societies with professions... */

  if (society->profession == NULL) {
    send_to_char("{gThey are performing their duties adequately.{x\r\n", ch);
    send_to_char("{gYou are performing your duties adequetly.{x\r\n", invitee);
    return;
  }

 /* Ok, now find their membership details... */

  memb = find_membership(invitee, society);

 /* Test to maintain current level... */

  if (memb->level > SOC_LEVEL_MEMBERSHIP) {

    ok = society_action_test(society, invitee, memb->level);

    if (!ok) {
      send_to_char("{yYou are unfit for your current duties!{x\r\n", invitee);
      send_to_char("{yThey are unfit for their current duties!{x\r\n", ch);  

      memb->level -= 1;

      return;
    }
  }

 /* Test to gain a new level... */

  ok = society_action_test(society, invitee, memb->level + 1);

  if (ok) {
    send_to_char("{cYou are performing your duties excellently!{x\r\n",
                                                                    invitee);
    send_to_char("{cThey are performing their duties excellently!{x\r\n", ch);  

    memb->level += 1;
 
    return; 
  }

 /* No change means just a default message... */
 
  send_to_char("{gThey are performing their duties properly.{x\r\n", ch);
  send_to_char("{gYou are performing your duties properly.{x\r\n", invitee);

 /* Finally! */

  return;
}

void do_society_authorize(CHAR_DATA *ch, char *args) {
  CHAR_DATA *invitee;
  SOCIETY *society;
  SOCIALITE *social;
  char char_name[MAX_STRING_LENGTH];
  char ssid[MAX_STRING_LENGTH];
  int sid;
  MOB_CMD_CONTEXT *mcc;
  WEV *wev;
  int rank;
  bool sreset = FALSE;
  char abuf[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];

  int new_auth;

 /* First, chop up the parms... */

  args = one_argument(args, char_name);
  args = one_argument(args, ssid);


 /* No-one to authorize means we have a problem... */

  if (char_name[0] == '\0') {
    send_to_char("Who do you want to authorize?\r\n", ch);
    return;
  }

 /* Who is being promoted? */

  invitee = get_char_room(ch, char_name);

  if (invitee == NULL) {
    send_to_char("They are not here!\r\n", ch);
    return;
  }

 /* Find the society... */

  sid = atoi(ssid);

  if ( sid < 0
    || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

 /* Are they allowed to authorize? */

  if (!IS_IMMORTAL(ch)) {

    if (IS_NPC(ch)) {
      social = find_socialite(society, ch->short_descr);
    } else {
      social = find_socialite(society, ch->name);
    }
   
    if (social == NULL) {
      send_to_char("You are not a full member yourself!\r\n", ch);
      return;
    }

    if (!IS_SET(social->authority, SOC_AUTH_AUTH)) {
      send_to_char("You do not have authority to do that!\r\n", ch);
      return;
    }

    rank = social->rank;
  } else {
    rank = SOC_RANK_LEADER;  
  }
 
 /* Check if they are already a member? */

  if (IS_NPC(invitee)) {
    social = find_socialite(society, invitee->short_descr);
  } else {
    social = find_socialite(society, invitee->name);
  }
   
  if (social == NULL) {
    send_to_char("They are not a member!\r\n", ch);
    return;
  }

 /* No flags, means we should display their authorization... */

  if (args[0] == '\0') {
    soc_auth_names(social->authority, abuf); 
    sprintf(buf, "Their current authority is: %s\r\n", abuf);
    send_to_char(buf, ch);
    return;
  }

 /* Work out the new authority... */

  if (!IS_IMMORTAL(ch)) {
    if ( invitee == ch) {
      send_to_char("You cannot change your own authorization!\r\n", ch);
      return;
    }

    if ( social->rank > rank) {
      send_to_char("You cannot authorise someone of higher rank!\r\n", ch);
      return;
    }
  }

   if (!str_cmp(args, "none")) {
       new_auth = SOC_AUTH_NONE;
       sreset = TRUE;
   } else if (!str_cmp(args, "all")) {
       if (society->type == 5) new_auth = SOC_AUTH_INVITE | SOC_AUTH_EXPEL | SOC_AUTH_BANK | SOC_AUTH_AUTH | SOC_AUTH_PROMOTE | SOC_AUTH_DEMOTE | SOC_AUTH_TEST | SOC_AUTH_FOE | SOC_AUTH_PARDON | SOC_AUTH_TAX | SOC_AUTH_LAW;
       else new_auth = SOC_AUTH_INVITE | SOC_AUTH_EXPEL | SOC_AUTH_BANK | SOC_AUTH_AUTH | SOC_AUTH_PROMOTE | SOC_AUTH_DEMOTE | SOC_AUTH_TEST | SOC_AUTH_FOE | SOC_AUTH_PARDON | SOC_AUTH_TAX;
       sreset = TRUE;
   } else if (!str_cmp(args, "default")) {
       sreset = TRUE;
       switch (social->rank) {
           default:
              new_auth = SOC_AUTH_NONE;    
              break;
           case SOC_RANK_MEMBER:
              new_auth = SOC_AUTH_INVITE;
              break;

           case SOC_RANK_COUNCIL:
               new_auth = SOC_AUTH_INVITE | SOC_AUTH_TEST | SOC_AUTH_PARDON;
               break;

           case SOC_RANK_LEADER:
               if (society->type == 5) new_auth = SOC_AUTH_INVITE | SOC_AUTH_BANK | SOC_AUTH_PROMOTE | SOC_AUTH_TEST | SOC_AUTH_FOE | SOC_AUTH_PARDON | SOC_AUTH_TAX | SOC_AUTH_LAW;
               else new_auth = SOC_AUTH_INVITE | SOC_AUTH_BANK | SOC_AUTH_PROMOTE | SOC_AUTH_TEST | SOC_AUTH_FOE | SOC_AUTH_PARDON | SOC_AUTH_TAX;
               break;
       }
   } else {
       sreset = FALSE;
       new_auth = flag_value(soc_auth, args);
   }

 /* Build WEV, check for permission... */

  mcc = get_mcc(ch, ch, invitee, NULL, NULL, NULL, sid, society->name);

  wev = get_wev(WEV_SOCIETY, WEV_SOCIETY_ADVANCE, mcc,
                "You authorize @v2 within the '@t0'.\r\n",
                "@a2 authorizes you within the '@t0'.\r\n",
                "@a2 authorizes @v2 within the '@t0'.\n\t"); 

 /* Challange... */

  if (!room_issue_wev_challange(ch->in_room, wev)) {
    free_wev(wev);
    return;
  }

 /* Ok, go do it... */

  if (sreset) social->authority = SOC_AUTH_NONE;
  social->authority |= new_auth;
  save_socialites();

 /* Tell the world... */

  room_issue_wev(ch->in_room, wev);

  free_wev(wev);

 /* Tell them what happened... */

  soc_auth_names(social->authority, abuf); 

  sprintf(buf, "Their new authority is: %s\r\n", abuf);

  send_to_char(buf, ch);

  sprintf(buf, "Your new authority is: %s\r\n", abuf);

  send_to_char(buf, invitee);

 /* Finally! */

  return;
}

void do_society_resign(CHAR_DATA *ch, char *args) {

  SOCIETY *society;

  MEMBERSHIP *memb;

  int sid;

  MOB_CMD_CONTEXT *mcc;

  WEV *wev;

 /* Find the society... */

  sid = atoi(args);

  if ( sid < 0
    || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

 /* Now see if they have been invited... */
 
  memb = ch->memberships;

  while ( memb != NULL
       && memb->society != society ) {

    memb = memb->next;
  } 

  if (memb == NULL) {
    send_to_char("You are not a member!\r\n", ch);
    return;
  }

  if (memb->level < SOC_LEVEL_INVITED) {
    send_to_char("You have not been invited to join them!\r\n", ch);
    return;
  }

 /* Can't resign from the human race... */

  if (society->type == SOC_TYPE_RACE) {
    send_to_char("Don't be silly!\r\n", ch);
    return;
  }

 /* Build WEV, check for permission... */

  mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, sid, society->name);

  wev = get_wev(WEV_SOCIETY, WEV_SOCIETY_RESIGN, mcc,
                "You resign from the '@t0'\r\n",
                 NULL,
                "@a2 resigns from the '@t0'\n\t"); 

 /* Challange... */

  if (!room_issue_wev_challange(ch->in_room, wev)) {
    free_wev(wev);
    return;
  }

 /* Ok, go do it... */

  society_action_expel(society, ch);

 /* Tell the world... */

  room_issue_wev(ch->in_room, wev);

  free_wev(wev);

 /* Finally! */

  return;
}

void do_society_expel(CHAR_DATA *ch, char *args) {
  CHAR_DATA *invitee;
  SOCIETY *society;
  SOCIALITE *social;
  char char_name[MAX_STRING_LENGTH];
  int sid;
  MOB_CMD_CONTEXT *mcc;
  WEV *wev;
  int rank;
  char buf[MAX_STRING_LENGTH];

 /* First, chop up the parms... */

  args = one_argument(args, char_name);

 /* No-one to advance means we have a problem... */

  if (char_name[0] == '\0') {
    send_to_char("Who do you want to expel?\r\n", ch);
    return;
  }

 /* Who is being advanced? */

  invitee = get_char_room(ch, char_name);

  if (invitee == NULL) {
    send_to_char("They are not here!\r\n", ch);
    return;
  }

  if (invitee == ch) {
    send_to_char("You cannot expel yourself - use reisgn!\r\n", ch);
    return;
  }

 /* Find the society... */

  sid = atoi(args);

  if ( sid < 0
    || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  if (society->junta) {
    send_to_char("A revolt is going on.\r\n", ch);
    return;
  }

 /* Are they allowed to invite? */

  if (!IS_IMMORTAL(ch)) {

    if (IS_NPC(ch)) {
      social = find_socialite(society, ch->short_descr);
    } else {
      social = find_socialite(society, ch->name);
    }
   
    if (social == NULL) {
      send_to_char("You are not a full member yourself!\r\n", ch);
      return;
    }

    if (!IS_SET(social->authority, SOC_AUTH_EXPEL)) {
      send_to_char("You do not have authority to do that!\r\n", ch);
      return;
    }

    rank = social->rank;
  } else {
    rank = SOC_RANK_LEADER;  
  }
 
 /* Check if they are already a member? */

  if (IS_NPC(invitee)) {
    social = find_socialite(society, invitee->short_descr);
  } else {
    social = find_socialite(society, invitee->name);
  }
   
  if (social == NULL) {
    send_to_char("They are not a member!\r\n", ch);
    return;
  }

  if (social->rank > rank) {
    send_to_char("You cannot expel someone beyond your own rank!\r\n", ch);
    return;
  }

 /* Build WEV, check for permission... */

  mcc = get_mcc(ch, ch, invitee, NULL, NULL, NULL, sid, society->name);

  wev = get_wev(WEV_SOCIETY, WEV_SOCIETY_EXPEL, mcc,
                "You expel @v2 from the '@t0'.\r\n",
                "@a2 expels you from the '@t0'!\r\n",
                "@a2 expels @v2 from the '@t0'!\n\t"); 

 /* Challange... */

  if (!room_issue_wev_challange(ch->in_room, wev)) {
    free_wev(wev);
    return;
  }

 /* Ok, go do it... */

  society_action_expel(society, invitee);

 /* Tell the world... */

  room_issue_wev(ch->in_room, wev);

  free_wev(wev);

 /* Tell the invitee... */

  sprintf(buf, "{YYou are no longer a member of the '%s'{x\r\n", society->name);

  send_to_char(buf, invitee);

 /* Finally! */

  return;
}


void do_society_cleanup(CHAR_DATA *ch, char *args) {
CHAR_DATA *invitee;
SOCIETY *society;
SOCIALITE *social;
char char_name[MAX_STRING_LENGTH];
int sid, rank;
MOB_CMD_CONTEXT *mcc;
WEV *wev;
char buf[MAX_STRING_LENGTH];
struct stat fst;
double tdif;


  args = one_argument(args, char_name);

  if (char_name[0] == '\0') {
    send_to_char("Who do you want to expel?\r\n", ch);
    return;
  }

  invitee = get_char_world_player(ch, char_name);

  if (invitee) {
    send_to_char("They are online - use EXPEL!\r\n", ch);
    return;
  }

  sid = atoi(args);

  if ( sid < 0
  || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  if (society->junta) {
    send_to_char("A revolt is going on.\r\n", ch);
    return;
  }

  if (!IS_IMMORTAL(ch)) {
    if (IS_NPC(ch)) {
      social = find_socialite(society, ch->short_descr);
    } else {
      social = find_socialite(society, ch->name);
    }
   
    if (social == NULL) {
      send_to_char("You are not a full member yourself!\r\n", ch);
      return;
    }

    if (!IS_SET(social->authority, SOC_AUTH_EXPEL)) {
      send_to_char("You do not have authority to do that!\r\n", ch);
      return;
    }

    rank = social->rank;
  } else {
    rank = SOC_RANK_LEADER;  
  }

  sprintf( buf, "%s%s.gz", PLAYER_DIR, capitalize(char_name));
  if (stat(buf, &fst ) == -1 )  {
      sprintf( buf, "%s%s", PLAYER_DIR, capitalize(char_name));
      if (stat(buf, &fst ) == -1 )  {
           sprintf_to_char(ch,"Could not find player %s.\r\n",buf);
            return;
      }
  }

  if (!IS_IMMORTAL(ch)) {
      tdif = ((difftime(current_time,  fst.st_mtime ) / 24) / 60) / 60;
      if (tdif < 7) {
          send_to_char("Not enough offline time.\r\n", ch);
          return;
      }
  }
  
  do_pload(ch, char_name);
  invitee = get_char_world_player(ch, char_name);

  if (!invitee) {
    send_to_char("Player not found!\r\n", ch);
    return;
  }

  if (IS_NPC(invitee)) {
    social = find_socialite(society, invitee->short_descr);
  } else {
    social = find_socialite(society, invitee->name);
  }
   
  if (social == NULL) {
    send_to_char("They are not a member!\r\n", ch);
    return;
  }

  mcc = get_mcc(ch, ch, invitee, NULL, NULL, NULL, sid, society->name);
  wev = get_wev(WEV_SOCIETY, WEV_SOCIETY_EXPEL, mcc,
                "You expel @v2 from the '@t0'.\r\n",
                NULL,
                "@a2 expels @v2 from the '@t0'!\n\t"); 

  if (!room_issue_wev_challange(ch->in_room, wev)) {
    free_wev(wev);
    return;
  }

  society_action_expel(society, invitee);
  room_issue_wev(ch->in_room, wev);

  free_wev(wev);
  do_punload(ch, char_name);
  return;
}


void do_society_demote(CHAR_DATA *ch, char *args) {

  CHAR_DATA *invitee;

  SOCIETY *society;

  SOCIALITE *social;

  char char_name[MAX_STRING_LENGTH];

  int sid;

  MOB_CMD_CONTEXT *mcc;

  WEV *wev;

  int rank;
  
  char buf[MAX_STRING_LENGTH];

 /* First, chop up the parms... */

  args = one_argument(args, char_name);

 /* No-one to advance means we have a problem... */

  if (char_name[0] == '\0') {
    send_to_char("Who do you want to demote?\r\n", ch);
    return;
  }

 /* Who is being advanced? */

  invitee = get_char_room(ch, char_name);

  if (invitee == NULL) {
    send_to_char("They are not here!\r\n", ch);
    return;
  }

  if (invitee == ch) {
    send_to_char("You cannot demote yourself!\r\n", ch);
    return;
  }

 /* Find the society... */

  sid = atoi(args);

  if ( sid < 0
    || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

 /* Are they allowed to invite? */

  if (!IS_IMMORTAL(ch)) {

    if (IS_NPC(ch)) {
      social = find_socialite(society, ch->short_descr);
    } else {
      social = find_socialite(society, ch->name);
    }
   
    if (social == NULL) {
      send_to_char("You are not a full member yourself!\r\n", ch);
      return;
    }

    if (!IS_SET(social->authority, SOC_AUTH_DEMOTE)) {
      send_to_char("You do not have authority to do that!\r\n", ch);
      return;
    }

    rank = social->rank;
  } else {
    rank = SOC_RANK_LEADER;  
  }
 
 /* Check if they are already a member? */

  if (IS_NPC(invitee)) {
    social = find_socialite(society, invitee->short_descr);
  } else {
    social = find_socialite(society, invitee->name);
  }
   
  if (social == NULL) {
    send_to_char("They are not a member!\r\n", ch);
    return;
  }

  if (social->rank > rank) {
    send_to_char("You cannot demote someone beyond your own rank!\r\n", ch);
    return;
  }

  if (social->rank == SOC_RANK_MEMBER) {
    send_to_char("They cannot be demoted further within the society!\r\n", ch);
    return;
  }

 /* Build WEV, check for permission... */

  mcc = get_mcc(ch, ch, invitee, NULL, NULL, NULL, sid, society->name);

  wev = get_wev(WEV_SOCIETY, WEV_SOCIETY_DEMOTE, mcc,
                "You demote @v2 within the '@t0'.\r\n",
                "@a2 demotes you within the '@t0'!\r\n",
                "@a2 demotes @v2 within the '@t0'!\n\t"); 

 /* Challange... */

  if (!room_issue_wev_challange(ch->in_room, wev)) {
    free_wev(wev);
    return;
  }

 /* Ok, go do it... */

  society_action_demote(society, invitee);

 /* Tell the world... */

  room_issue_wev(ch->in_room, wev);

  free_wev(wev);

 /* Tell the invitee... */

  switch (social->rank) {
    case SOC_RANK_MEMBER:
      sprintf(buf, "You are now just a normal member of the '%s'\r\n", society->name);
      break;

    case SOC_RANK_COUNCIL:   
      sprintf(buf, "You are now only a council member of the '%s'\r\n",  society->name);
      break;

    default:
      sprintf(buf, "Hmmm. Something seems to have gone wrong...\r\n");
      break;  
  }

  send_to_char(buf, invitee);

 /* Finally! */

  return;
}

void do_society_promote(CHAR_DATA *ch, char *args) {
  CHAR_DATA *invitee;
  SOCIETY *society;
  SOCIALITE *social;
  char char_name[MAX_STRING_LENGTH];
  int sid;

  MOB_CMD_CONTEXT *mcc;

  WEV *wev;

  int rank;
  
  char buf[MAX_STRING_LENGTH];

 /* First, chop up the parms... */

  args = one_argument(args, char_name);

 /* No-one to advance means we have a problem... */

  if (char_name[0] == '\0') {
    send_to_char("Who do you want to promote?\r\n", ch);
    return;
  }

 /* Who is being promoted? */

  invitee = get_char_room(ch, char_name);

  if (invitee == NULL) {
    send_to_char("They are not here!\r\n", ch);
    return;
  }

  if (invitee == ch) {
    send_to_char("You cannot promote yourself!\r\n", ch);
    return;
  }

 /* Find the society... */

  sid = atoi(args);

  if ( sid < 0
    || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

 /* Are they allowed to invite? */

  if (!IS_IMMORTAL(ch)) {

    if (IS_NPC(ch)) {
      social = find_socialite(society, ch->short_descr);
    } else {
      social = find_socialite(society, ch->name);
    }
   
    if (social == NULL) {
      send_to_char("You are not a full member yourself!\r\n", ch);
      return;
    }

    if (!IS_SET(social->authority, SOC_AUTH_PROMOTE)) {
      send_to_char("You do not have authority to do that!\r\n", ch);
      return;
    }

    rank = social->rank;
  } else {
    rank = SOC_RANK_LEADER;  
  }
 
 /* Check if they are already a member? */

  if (IS_NPC(invitee)) {
    social = find_socialite(society, invitee->short_descr);
  } else {
    social = find_socialite(society, invitee->name);
  }
   
  if (social == NULL) {
    send_to_char("They are not a member!\r\n", ch);
    return;
  }

  if (social->rank >= rank) {
    send_to_char("You cannot promote someone beyond your own rank!\r\n", ch);
    return;
  }

 /* Build WEV, check for permission... */

  mcc = get_mcc(ch, ch, invitee, NULL, NULL, NULL, sid, society->name);

  wev = get_wev(WEV_SOCIETY, WEV_SOCIETY_ADVANCE, mcc,
                "You promote @v2 within the '@t0'.\r\n",
                "@a2 promotes you within the '@t0'.\r\n",
                "@a2 promotes @v2 within the '@t0'.\n\t"); 

 /* Challange... */

  if (!room_issue_wev_challange(ch->in_room, wev)) {
    free_wev(wev);
    return;
  }

 /* Ok, go do it... */

  society_action_promote(society, invitee);

 /* Tell the world... */

  room_issue_wev(ch->in_room, wev);

  free_wev(wev);

 /* Tell the invitee... */

  switch (social->rank) {
    case SOC_RANK_MEMBER:
      sprintf(buf, "You are now a full member of the '%s'\r\n", society->name);
      break;

    case SOC_RANK_COUNCIL:
      sprintf(buf, "You are now a trusted counciller of the '%s'\r\n", society->name);
      break;

    case SOC_RANK_LEADER:   
      sprintf(buf, "You are now an exhaulted leader of the '%s'\r\n", society->name);
      break;

    default:
      sprintf(buf, "Hmmm. Something seems to have gone wrong...\r\n");
      break;  
  }

  send_to_char(buf, invitee);

 /* Finally! */

  return;
}

void do_society_join(CHAR_DATA *ch, char *args) {
SOCIETY *society;
MEMBERSHIP *memb;
int sid;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

 /* Find the society... */

  sid = atoi(args);

  if ( sid < 0
    || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

 /* Now see if they have been invited... */
 
  memb = ch->memberships;

  while ( memb != NULL
       && memb->society != society ) {

    memb = memb->next;
  } 

  if (memb == NULL) {
    send_to_char("You have not been invited to join them!\r\n", ch);
    return;
  }

  if (memb->level > SOC_LEVEL_INVITED) {
    send_to_char("You are already a member!\r\n", ch);
    return;
  }

  if (memb->level < SOC_LEVEL_INVITED) {
    send_to_char("You have not been invited to join them!\r\n", ch);
    return;
  }

 /* Build WEV, check for permission... */

  mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, sid, society->name);
  wev = get_wev(WEV_SOCIETY, WEV_SOCIETY_JOIN, mcc,
                "You join the '@t0'\r\n",
                 NULL,
                "@a2 joins the '@t0'\n\t"); 

 /* Challange... */

  if (!room_issue_wev_challange(ch->in_room, wev)) {
    free_wev(wev);
    return;
  }

 /* Ok, go do it... */

  society_action_promote(society, ch);

 /* Tell the world... */

  room_issue_wev(ch->in_room, wev);

  free_wev(wev);

 /* Finally! */

  return;
}


void do_society_subscribe(CHAR_DATA *ch, char *args) {
SOCIETY *society;
MEMBERSHIP *memb;
SOCIALITE *social;
int sid;

  if (!IS_IMMORTAL(ch) || get_divinity(ch) < DIV_LESSER) {
    send_to_char("You can't subscribe to society channels.\r\n", ch);
    return;
  }

  sid = atoi(args);

  if ( sid < 0 || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);
  if (!society) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  if (IS_NPC(ch)) {
    social = find_socialite(society, ch->short_descr);
  } else {
    social = find_socialite(society, ch->name);
  }

  memb = find_membership(ch, society);

  if (!social) {
    if (!memb) {
      memb = get_membership();

      memb->society = society;
      memb->level = SOC_LEVEL_INVITED;
      memb->profession = society->profession;

      memb->next = ch->memberships;
      ch->memberships = memb;

    } 

    social = get_socialite();
    social->name = str_dup(ch->name);
    social->rank = SOC_RANK_SUBSCRIBE; 
    social->authority = SOC_AUTH_NONE;

    social->next = society->members;
    society->members = social;
    save_socialites();
    sprintf_to_char(ch, "You subscribe to the %s channel.\r\n", society->name);

  } else {
    sprintf_to_char(ch, "You already are a member of %s.\r\n", society->name);
  }
  return;
}


void do_society_invite(CHAR_DATA *ch, char *args) {
CHAR_DATA *invitee;
SOCIETY *society;
SOCIALITE *social;
MEMBERSHIP *memb;
char char_name[MAX_STRING_LENGTH];
int sid;
MOB_CMD_CONTEXT *mcc;
WEV *wev;
bool found;
char buf[MAX_STRING_LENGTH];
  
 /* First, chop up the parms... */

  args = one_argument(args, char_name);

 /* No invitee means list my invitations... */

  if (char_name[0] == '\0') {

    found = FALSE;

    memb = ch->memberships;

    while (memb != NULL) {

      if (memb->level == SOC_LEVEL_INVITED) {
        
        if (!found) {
          send_to_char("You have been invited to join:\r\n"
                       " SocId    Name\r\n"
                       " -------  ------------------------------------\r\n", ch);
          found = TRUE;
        }

        sprintf(buf, " {w[{y%5d{w]  {g%s{x\r\n",
                      memb->society->id,
                      memb->society->name);

        send_to_char(buf, ch);  
      }

      memb = memb->next;
    } 

    if (!found) {
      send_to_char("You have no outstanding invitations.\r\n", ch);
    }

    return;
  }

 /* Who is being invited? */

  invitee = get_char_room(ch, char_name);

  if (invitee == NULL) {
    send_to_char("They are not here!\r\n", ch);
    return;
  }

  if (invitee == ch) {
    send_to_char("You cannot invite yourself!\r\n", ch);
    return;
  }

 /* Find the society... */

  sid = atoi(args);

  if ( sid < 0
    || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

 /* Are they allowed to invite? */

  if (!IS_IMMORTAL(ch)) {

    if (IS_NPC(ch)) {
      social = find_socialite(society, ch->short_descr);
    } else {
      social = find_socialite(society, ch->name);
    }
   
    if (social == NULL) {
      send_to_char("You are not a full member yourself!\r\n", ch);
      return;
    }

    if (!IS_SET(social->authority, SOC_AUTH_INVITE)) {
      send_to_char("You do not have authority to do that!\r\n", ch);
      return;
    }
  }   
 
 /* Check if they are already a member? */

  memb = find_membership(invitee, society);

  if ( memb != NULL ) {

    if (memb->level == SOC_LEVEL_INVITED) {
      send_to_char("They have already been invited to join!\r\n", ch);
      return;
    }

    if (memb->level == SOC_LEVEL_FOE) {
      send_to_char("They are a foe and must be pardoned first!\r\n", ch);
      return;
    }

    if (IS_NPC(invitee)) {
      social = find_socialite(society, invitee->short_descr);
    } else {
      social = find_socialite(society, invitee->name);
    }
   
    if (social != NULL) {
      send_to_char("They are already a member!\r\n", ch);
      return;
    }
  }

 /* See if they are acceptable... */

  if (!society_action_test(society, invitee, SOC_LEVEL_MEMBERSHIP)) {
    send_to_char("{YThey are NOT suitable for membership!{x\r\n", ch);
    return;
  }

 /* Build WEV, check for permission... */

  mcc = get_mcc(ch, ch, invitee, NULL, NULL, NULL, sid, society->name);
  wev = get_wev(WEV_SOCIETY, WEV_SOCIETY_INVITE, mcc,
                "You invite @v2 to join the '@t0'.\r\n",
                "@a2 invites you to join the '@t0'.\r\n",
                "@a2 invites @v2 to join the '@t0'.\n\t"); 

 /* Challange... */

  if (!room_issue_wev_challange(ch->in_room, wev)) {
    free_wev(wev);
    return;
  }

 /* Ok, go do it... */

  society_action_promote(society, invitee);

 /* Tell the world... */

  room_issue_wev(ch->in_room, wev);

  free_wev(wev);

 /* Finally! */

  return;
}


void do_society_sign(CHAR_DATA *ch, char *args) {
  SOCIETY *society;
  SOCIALITE *social;
  MEMBERSHIP *memb;
  char snumb[MAX_STRING_LENGTH];
  int sid;
  MOB_CMD_CONTEXT *mcc;
  WEV *wev;
  
  args = one_argument(args, snumb);

  if (snumb[0] == '\0') {
    memb = ch->memberships;
    while (memb != NULL) {
        if (memb->level > SOC_LEVEL_INVITED
        && (memb->society->type == SOC_TYPE_CLAN
               || memb->society->type == SOC_TYPE_SECRET)) {
             mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, memb->society->id, memb->society->name);

             wev = get_wev(WEV_SOCIAL, WEV_SOCIAL_SOCIETY, mcc,
                        memb->society->csoul1,
                        memb->society->csoul2,
                        memb->society->csoul2);

            if (!room_issue_wev_challange(ch->in_room, wev)) {
                free_wev(wev);
                return;
            }

            room_issue_wev( ch->in_room, wev);
            free_wev(wev);
            return;
        }
        memb = memb->next;
    }
    send_to_char("You're not member of any society.\r\n", ch);
    return;
  }

  sid = atoi(snumb);

  if ( sid < 0 || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  if (IS_NPC(ch)) {
      social = find_socialite(society, ch->short_descr);
  } else {
      social = find_socialite(society, ch->name);
  }
   
  if (social == NULL) {
      send_to_char("You don't know their sign!\r\n", ch);
      return;
  }

   mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, society->id, society->name);

   wev = get_wev(WEV_SOCIAL, WEV_SOCIAL_SOCIETY, mcc,
               society->csoul1,
               society->csoul2,
               society->csoul2);

    if (!room_issue_wev_challange(ch->in_room, wev)) {
                free_wev(wev);
                return;
    }

    room_issue_wev( ch->in_room, wev);
    free_wev(wev);
    return;
}


void do_society_foe(CHAR_DATA *ch, char *args) {

  CHAR_DATA *invitee;

  SOCIETY *society;

  SOCIALITE *social;

  MEMBERSHIP *memb;

  char char_name[MAX_STRING_LENGTH];

  int sid;

  MOB_CMD_CONTEXT *mcc;

  WEV *wev;

  bool found;

  char buf[MAX_STRING_LENGTH];
  
 /* First, chop up the parms... */

  args = one_argument(args, char_name);

 /* No invitee means list my foes... */

  if (char_name[0] == '\0') {

    found = FALSE;

    memb = ch->memberships;

    while (memb != NULL) {

      if (memb->level == SOC_LEVEL_FOE) {
        
        if (!found) {
          send_to_char("You are a foe of:\r\n"
                       " SocId    Name\r\n"
                       " -------  ------------------------------------\r\n", ch);
          found = TRUE;
        }

        sprintf(buf, " {w[{y%5d{w]  {g%s{x\r\n",
                      memb->society->id,
                      memb->society->name);

        send_to_char(buf, ch);  
      }

      memb = memb->next;
    } 

    if (!found) {
      send_to_char("You have not a foe of any societies.\r\n", ch);
    }

    return;
  }

 /* Who is being foed? */

  invitee = get_char_room(ch, char_name);

  if (invitee == NULL) {
    send_to_char("They are not here!\r\n", ch);
    return;
  }

  if (invitee == ch) {
    send_to_char("You cannot foe yourself!\r\n", ch);
    return;
  }

 /* Find the society... */

  sid = atoi(args);

  if ( sid < 0
    || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

 /* Are they allowed to invite? */

  if (!IS_IMMORTAL(ch)) {

    if (IS_NPC(ch)) {
      social = find_socialite(society, ch->short_descr);
    } else {
      social = find_socialite(society, ch->name);
    }
   
    if (social == NULL) {
      send_to_char("You are not a full member yourself!\r\n", ch);
      return;
    }

    if (!IS_SET(social->authority, SOC_AUTH_FOE)) {
      send_to_char("You do not have authority to do that!\r\n", ch);
      return;
    }
  }   
 
 /* Check if they are already a member? */

  memb = find_membership(invitee, society);

  if ( memb != NULL ) {

    if (memb->level == SOC_LEVEL_FOE) {
      send_to_char("They are already a foe!\r\n", ch);
      return;
    }

    send_to_char("They must be expelled from the society first!\r\n", ch);

    return;
  }

 /* Build WEV, check for permission... */

  mcc = get_mcc(ch, ch, invitee, NULL, NULL, NULL, sid, society->name);

  wev = get_wev(WEV_SOCIETY, WEV_SOCIETY_FOE, mcc,
                "You declare @v2 to be a mortal enemy of the '@t0'.\r\n",
                "@a2 declares you to be a mortal enemy of the '@t0'.\r\n",
                "@a2 declares @v2 to be a mortal enemy of the '@t0'.\n\t"); 

 /* Challange... */

  if (!room_issue_wev_challange(ch->in_room, wev)) {
    free_wev(wev);
    return;
  }

 /* Ok, go do it... */

  society_action_foe(society, invitee);

 /* Tell the world... */

  room_issue_wev(ch->in_room, wev);

  free_wev(wev);

 /* Finally! */

  return;
}

void do_society_pardon(CHAR_DATA *ch, char *args) {

  CHAR_DATA *invitee;

  SOCIETY *society;

  SOCIALITE *social;

  MEMBERSHIP *memb;

  char char_name[MAX_STRING_LENGTH];

  int sid;

  MOB_CMD_CONTEXT *mcc;

  WEV *wev;

 /* First, chop up the parms... */

  args = one_argument(args, char_name);

 /* No invitee means list my foes... */

  if (char_name[0] == '\0') {
    send_to_char("Pardon who?", ch);
    return;
  }

 /* Who is being foed? */

  invitee = get_char_room(ch, char_name);

  if (invitee == NULL) {
    send_to_char("They are not here!\r\n", ch);
    return;
  }

  if (invitee == ch) {
    send_to_char("You cannot pardon yourself!\r\n", ch);
    return;
  }

 /* Find the society... */

  sid = atoi(args);

  if ( sid < 0
    || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

 /* Are they allowed to pardon? */

  if (!IS_IMMORTAL(ch)) {

    if (IS_NPC(ch)) {
      social = find_socialite(society, ch->short_descr);
    } else {
      social = find_socialite(society, ch->name);
    }
   
    if (social == NULL) {
      send_to_char("You are not a full member yourself!\r\n", ch);
      return;
    }

    if (!IS_SET(social->authority, SOC_AUTH_PARDON)) {
      send_to_char("You do not have authority to do that!\r\n", ch);
      return;
    }
  }   
 
 /* Check if they are already a member? */

  memb = find_membership(invitee, society);

  if ( memb == NULL ) {
    send_to_char("The society has no grievence with them...\r\n", ch);
    return;
  }

  if (memb->level != SOC_LEVEL_FOE) {
    send_to_char("They are a trustued member, not a foe!\r\n", ch);
    return;
  }

 /* Build WEV, check for permission... */

  mcc = get_mcc(ch, ch, invitee, NULL, NULL, NULL, sid, society->name);

  wev = get_wev(WEV_SOCIETY, WEV_SOCIETY_PARDON, mcc,
                "You pardon @v2 for their crimes against the '@t0'.\r\n",
                "@a2 pardons you for your crimes against the '@t0'.\r\n",
                "@a2 pardons @v2 for their crimes againts the '@t0'.\n\t"); 

 /* Challange... */

  if (!room_issue_wev_challange(ch->in_room, wev)) {
    free_wev(wev);
    return;
  }

 /* Ok, go do it... */

  society_action_pardon(society, invitee);

 /* Tell the world... */

  room_issue_wev(ch->in_room, wev);

  free_wev(wev);

 /* Finally! */

  return;
}

static const char *tname[] = {
  "none", "clan", "racial", "politcal", "cult", "guard"
};

void do_society_list(CHAR_DATA *ch, char *args) {
  SOCIETY *society;
  MEMBERSHIP *memb;
 
  char buf[MAX_STRING_LENGTH];

  int type;
  char *nc;
  bool show;
  bool shown;

 /* List all or one? */

  type = SOC_TYPE_NONE;

  if ( args[0] == '\0' ) {
    send_to_char("Society types:\r\n"
                 "  clan races political cults secret guard all\r\n", ch);
    return;
  }

  if (!str_cmp(args, "clan")) {
    type = SOC_TYPE_CLAN;
  } else if (!str_cmp(args, "races")) {
    type = SOC_TYPE_RACE;
  } else if (!str_cmp(args, "political")) {
    type = SOC_TYPE_POLITICAL;
  } else if (!str_cmp(args, "cults")) {
    type = SOC_TYPE_CULT;
  } else if (!str_cmp(args, "guard")) {
    type = SOC_TYPE_GUARD;
  } else if (!str_cmp(args, "secret")) {
    type = SOC_TYPE_SECRET;
  } else if (!str_cmp(args, "all")) {
    type = SOC_TYPE_NONE;   
  } else {
    send_to_char("Society types:\r\n"
                 "  clan races political cults secret guard all\r\n", ch);
    return;
  }

  if ( type == SOC_TYPE_SECRET 
    && !IS_IMMORTAL(ch)) { 
    send_to_char("Only the gods know that...\r\n", ch);
    return;
  }
    
 /* Display the list... */

  society = society_chain;

  if (society == NULL) {
    send_to_char("No societies defined!\r\n", ch);
    return;
  }

  shown = FALSE;

  while (society != NULL) {

    show = FALSE;

    switch (type) {
      case SOC_TYPE_NONE:
        show = TRUE; 
        break;

      case SOC_TYPE_CLAN:
      case SOC_TYPE_RACE:
      case SOC_TYPE_POLITICAL:
      case SOC_TYPE_CULT:
      case SOC_TYPE_GUARD:
        if (society->type == type) {
          show = TRUE;
        }

        break;

      case SOC_TYPE_SECRET:
        if (society->secret) {
          show = TRUE;
        }
        break;

      default:
        show = TRUE; 
        break;
    }  

   /* Mortals only see secret societies they are members of... */

    if (  show 
      && !IS_IMMORTAL(ch)
      &&  society->secret) {

      memb = find_membership(ch, society);

      if (memb == NULL) {
        show = FALSE;
      } else {
        if ( memb->level != SOC_LEVEL_INVITED
          && memb->level != SOC_LEVEL_MEMBERSHIP ) {
          show = FALSE;
        }
      } 
    } 

   /* Show the details if we need to... */

    if (show) {

      if (!shown) {
        send_to_char(" Sid     Type        Name\r\n"
                     " ------- ----------- ----------------------------\r\n", ch);
        shown = TRUE;
      }

      nc = "{g";

      if (society->secret == TRUE) {
        nc = "{r";
      } 

      sprintf(buf, " [{y%5d{x] [{c%9s{x] [%s%s{x]\r\n", society->id,  tname[society->type], nc, society->name);

      send_to_char(buf, ch);
    }     
 
    society = society->next;
  } 

 /* No matches? */
 
  if (!shown) {
    send_to_char("No societies of that type exist...\r\n", ch);
    return;
  }

 /* All done... */

  return;
}

void do_society_info(CHAR_DATA *ch, char *args) {

  SOCIETY *society;
  SOCADV *sa;
  SOCIALITE *socmemb;
  MEMBERSHIP *memb;

  char buf[MAX_STRING_LENGTH];
  char abuf[MAX_STRING_LENGTH];
  int sid;
  int rank, auth, level;
  char *nc;

  bool found;

 /* List all of one? */

  if ( args[0] == '\0' ) {
    
    society = society_chain;

    if (society == NULL) {
      send_to_char("No societies defined!\r\n", ch);
      return;
    }

    found = FALSE;

    while (society != NULL) {

      if (IS_NPC(ch)) {
        socmemb = find_socialite(society, ch->short_descr);
      } else {
        socmemb = find_socialite(society, ch->name);
      }

      if (socmemb == NULL) {
        rank = SOC_RANK_NONE;
      } else {
        rank = socmemb->rank;
      } 

      if ( rank > SOC_RANK_NONE ) {
  
        if (!found) {
          send_to_char("You are a member of the following societies:\r\n"
                 " Sid     Rank      Name\r\n"
                 " ------- --------- ----------------------------------\r\n", ch);
          found = TRUE;
        }

        nc = "{g";

        if (society->secret) {
          nc = "{r";
        }

        sprintf(buf, " [{y%5d{x] [{c%7s{x] [%s%s{x]\r\n",
                      society->id,
                      soc_rank_name(society, rank), 
                      nc, 
                      society->name);

        send_to_char(buf, ch);
      }
     
      society = society->next;
    } 

    if (!found) {
      send_to_char("You are not a member of any societies.\r\n", ch);
    }
 
    return;
  }

  sid = atoi(args);

  if ( sid < 0
    || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

 /* Find membership records... */

  if (IS_NPC(ch)) {
    socmemb = find_socialite(society, ch->short_descr);
  } else {
    socmemb = find_socialite(society, ch->name);
  }

  if (socmemb == NULL) {
    rank = SOC_RANK_NONE;
    auth = SOC_AUTH_NONE;
  } else {
    rank = socmemb->rank;
    auth = socmemb->authority; 
  } 

 /* You see nothing for secter societies if you are not member... */
  
  if ( rank == SOC_RANK_NONE
    && society->secret == TRUE
    && !IS_IMMORTAL(ch)) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

 /* Spit out the details... */

  sprintf(buf, "{wSid [{y%5d{w] Name [{g%s{w]{x\r\n"
               "%s",
                society->id,
                society->name,
                society->desc);
  
  send_to_char(buf, ch);

 /* You only see this info if you are a member... */
  
  if ( rank == SOC_RANK_NONE ) {

    send_to_char("{yYou are not a member.{x\r\n", ch);

    if (!IS_IMMORTAL(ch)) {
      return;
    }
  }

 /* Find the members level... */

  memb = NULL;

  if ( rank == SOC_RANK_NONE ) {
    level = 0;
  } else {
    memb = find_membership(ch, society);

    if (memb == NULL) {
      level = 0;
    } else {
      level = memb->level;
    }
  }

 /* Output membership details... */

  if ( memb != NULL) {

   /* Output rank... */

    soc_auth_names(auth, abuf);

    sprintf(buf, "{wRank: [{c%s{w]  Authority: [{c%s{w]{x\r\n",
                 soc_rank_name(society, rank),
                 abuf); 

    send_to_char(buf, ch);

   /* Output some profession info... */

    if (memb->profession != NULL) {
      if (memb->level == SOC_LEVEL_INVITED) {
        sprintf(buf, "{wProfession: [{c%s{w] Level: [{yInvited{w]\r\n",
                      memb->profession->name);
      } else if (memb->level == SOC_LEVEL_MEMBERSHIP) {
        sprintf(buf, "{wProfession: [{c%s{w] Level: [{gMember{w]\r\n",
                      memb->profession->name);
      } else {
        sprintf(buf, "{wProfession: [{c%s{w] Level: [{c%d{w]\r\n",
                      memb->profession->name,
                      memb->level);
      }
    } else {
      if (memb->level == SOC_LEVEL_INVITED) {
        sprintf(buf, "{wProfession: [{cNone{w] Level: [{yInvited{w]\r\n");
      } else if (memb->level == SOC_LEVEL_MEMBERSHIP) {
        sprintf(buf, "{wProfession: [{cNone{w] Level: [{gMember{w]\r\n");
      } else {
        sprintf(buf, "{wProfession: [{cNone{w] Level: [{c%d{w]\r\n",
                      memb->level);
      }
    } 
 
    send_to_char(buf, ch);
  } 

 /* Output Expulsion condition... */

  if ( level == 0 
    || IS_IMMORTAL(ch)) {
 
    send_to_char("{wExpulsion:\r\n{x", ch);

    sa = find_socadv(society, SOC_LEVEL_EXPEL);

    if (sa == NULL) {
      send_to_char("None\r\n", ch);
    } else { 
      if (sa->condition == NULL) {
        send_to_char("{RNone{x\r\n", ch);
      } else {
        print_econd(ch, sa->condition, "  ");
      }
    }
  }

 /* Output membership condition... */

  if ( level == 0 
    || IS_IMMORTAL(ch)) {
 
    send_to_char("{wMembership:\r\n{x", ch);

    sa = find_socadv(society, SOC_LEVEL_MEMBERSHIP);

    if (sa == NULL) {
      send_to_char("Open\r\n", ch);
    } else { 
      if (sa->condition == NULL) {
        send_to_char("{RNone{x\r\n", ch);
      } else {
        print_econd(ch, sa->condition, "  ");
      }
    }
  }

 /* Output level conditions... */ 

  sa = society->advconds;

  while (sa != NULL) {

    if ( sa->level > 0
      && ( IS_IMMORTAL(ch) 
        || sa->level == level
        || sa->level == (level + 1)) ) {

      sprintf(buf, "Level %d\r\n", sa->level);
      send_to_char(buf, ch); 
      if (sa->condition == NULL) {
        send_to_char("{RNone{x\r\n", ch);
      } else {
        print_econd(ch, sa->condition, "  ");
      }
    }
 
    sa = sa->next;
  }

 /* All done... */

  return;
}

void do_society_members(CHAR_DATA *ch, char *args) {
  SOCIETY *society;
  SOCIALITE *socmemb;
  char buf[MAX_STRING_LENGTH];
  int sid;
  int rank, auth;
  int i;
  bool found;

 /* List all of one? */

  if ( args[0] == '\0' ) {
    
    society = society_chain;

    if (society == NULL) {
      send_to_char("No societies defined!\r\n", ch);
      return;
    }

    found = FALSE;

    while (society != NULL) {

      if (IS_NPC(ch)) {
        socmemb = find_socialite(society, ch->short_descr);
      } else {
        socmemb = find_socialite(society, ch->name);
      }

      if (socmemb != NULL) {

        if (!found) {
          send_to_char("You are a member of the following societies:\r\n"
                 " Sid     Rank              Name\r\n"
                 " ------- ----------------- ----------------------------------\r\n", ch);
          found = TRUE;
        }

        sprintf(buf, " [{y%5d{x] [{c%15s{x] [{g%s{x]\r\n",
                      society->id,
                      soc_rank_name(society, socmemb->rank), 
                      society->name);

        send_to_char(buf, ch);
      }
     
      society = society->next;
    } 

    if (!found) {
      send_to_char("You are not a member of any societies.\r\n", ch);
    }
 
    return;
  }

  sid = atoi(args);

  if ( sid < 0
    || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

 /* Find membership records... */

  if (IS_NPC(ch)) {
    socmemb = find_socialite(society, ch->short_descr);
  } else {
    socmemb = find_socialite(society, ch->name);
  }

  if (socmemb == NULL) {
    rank = SOC_RANK_NONE;
    auth = SOC_AUTH_NONE;
  } else {
    rank = socmemb->rank;
    auth = socmemb->authority; 
  } 

 /* You only see this info if you are a member... */
  
  if ( rank == SOC_RANK_NONE
    && !IS_IMMORTAL(ch)) {
    send_to_char("You are not a member of that society.\r\n", ch);
    return;
  }

 /* Spit out the details... */

  sprintf(buf, "{wSid [{y%5d{w] Name [{g%s{w]{x\r\n"
               " Rank              Name\r\n"
               " ----------------  -----------------------------------------------\r\n",
                society->id,
                society->name);
  
  send_to_char(buf, ch);

 /* Now we need to chew through the list of socialites... */

  for( i = SOC_RANK_LEADER; i > SOC_RANK_NONE; i--) {

    socmemb = society->members;

    while (socmemb != NULL) {

      if (socmemb->rank == i) {
        sprintf(buf, " [{c%15s{x] [{g%s{x]\r\n", soc_rank_name(society, socmemb->rank), socmemb->name);
                  
        send_to_char(buf, ch); 
      }

      socmemb = socmemb->next;
    }
  }

 /* All done... */

  return;
}

void do_society_tell(CHAR_DATA *ch, char *args) {

  SOCIETY *society;
  SOCIALITE *socmemb;

  char buf_you[MAX_STRING_LENGTH];
  char buf_them[MAX_STRING_LENGTH];
  char soc_id[MAX_STRING_LENGTH];
  int sid;
  int rank, auth;

  CHAR_DATA *gch;

  MOB_CMD_CONTEXT *mcc;
  WEV *wev;   
 
 /* List all of one? */

  if ( args[0] == '\0' ) {
    send_to_char("Tell which society?\r\n", ch);
    return;  
  }

  args = one_argument(args, soc_id);

  sid = atoi(soc_id);

  if ( sid < 0
    || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

 /* Find membership records... */

  if (IS_NPC(ch)) {
    socmemb = find_socialite(society, ch->short_descr);
  } else {
    socmemb = find_socialite(society, ch->name);
  }

  if (socmemb == NULL) {
    rank = SOC_RANK_NONE;
    auth = SOC_AUTH_NONE;
  } else {
    rank = socmemb->rank;
    auth = socmemb->authority; 
  } 

 /* You only see this info if you are a member... */
  
  if ( rank == SOC_RANK_NONE
    && !IS_IMMORTAL(ch)) {
    send_to_char("You are not a member of that society.\r\n", ch);
    return;
  }

 /* Check we have a message... */

  if (args[0] == '\0') {
    send_to_char("Tell them what?\r\n", ch);
    return;
  }

 /* Check mana... */
 
  if (ch->move < 5) {
    send_to_char("You don't have enough move left!\r\n", ch);
    return;
  }

  ch->move -= 5;

 /* Build the event... */

  mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, ch->speak[SPEAK_PRIVATE], args);

  sprintf(buf_you,  "{RYou tell the %s '@t1'{x\r\n", society->name);
  sprintf(buf_them, "{R@a2 tells the %s '@t1'{x\r\n", society->name);

  wev = get_wev(WEV_OOCC, WEV_OOCC_GTELL, mcc, buf_you, NULL, buf_them);

 /* Challange... */

  if (!room_issue_wev_challange(ch->in_room, wev)) {
    free_wev(wev);
    return;
  }

 /* Ok, direct send to each member... */

  gch = char_list;

  while (gch != NULL) {

    if (IS_NPC(gch)) {
      socmemb = find_socialite(society, gch->short_descr);
    } else {
      socmemb = find_socialite(society, gch->name);
    }

    if (socmemb != NULL) {
      mob_handle_wev(gch, wev, "Society");
    }

    gch = gch->next;
  }

 /* Free the wev... */

  free_wev(wev);

 /* All done... */

  return;
}


void do_clan_tell(CHAR_DATA *ch, char *args) {

  do_society_tell(ch, args);

  return;
}

void do_society_research(CHAR_DATA *ch, char *args) {
char buf[MAX_STRING_LENGTH];
MEMBERSHIP *memb;
CHAR_DATA *victim;
bool found;
 
   /* Find the player we are researching... */

    if ( args[0] == '\0' ) {
      send_to_char("Research which character?\r\n", ch);
      return;  
    }

    if (IS_IMMORTAL(ch)) {
      victim = get_char_world(ch, args);
    } else {
      victim = get_char_room(ch, args);
    }

    if ( victim == NULL ) {
      send_to_char("They are not hear!\r\n", ch);
      return;
    }

   /* Show society membership details... */
 
    found = FALSE;
     
    memb = victim->memberships;

    while (memb != NULL) {

      if ( memb->society != NULL
      && memb->level >= SOC_LEVEL_MEMBERSHIP ) {

        if ( !memb->society->secret ) {

          if (!found) {
            send_to_char("\r\nSociety Memberships:\r\n", ch);
            found = TRUE;
          }

          sprintf(buf, "[{y%5d{x] [{g%s{x]\r\n", memb->society->id, memb->society->name);  
          send_to_char(buf, ch);

        } else {

          if ( is_member(ch, memb->society)
          || IS_IMMORTAL(ch)) {
          
            if (!found) {
              send_to_char("\r\nSociety Memberships:\r\n", ch);
              found = TRUE;
            }

            sprintf(buf, "[{y%5d{x] [{r%s{x]\r\n",  memb->society->id, memb->society->name);  
            send_to_char(buf, ch);
          }
        } 
      }

      memb = memb->next;
    }  

   /* Show foe information... */
 
    found = FALSE;
     
    memb = victim->memberships;

    while (memb != NULL) {

      if ( memb->society != NULL
        && memb->level == SOC_LEVEL_FOE ) {

        if ( is_member(ch, memb->society)
          || IS_IMMORTAL(ch)) {
          
          if (!found) {
            send_to_char("\r\nThey are a foe of:\r\n", ch);
            found = TRUE;
          }

          sprintf(buf, "[{y%5d{x] [{r%s{x]\r\n", memb->society->id, memb->society->name);  
          send_to_char(buf, ch);
        } 
      }

      memb = memb->next;
    }  

   /* All done... */

    return;
}


void do_society(CHAR_DATA *ch, char *args) {
char cmd[MAX_INPUT_LENGTH];

 /* Check for help request... */

  if ( args[0] == '\0'
    || args[0] == '?' ) {
    send_to_char("\r\n{CInformation:{x\r\n", ch); 
    send_to_char("        society list <sid>\r\n", ch); 
    send_to_char("        society info\r\n", ch); 
    send_to_char("        society info <sid>\r\n", ch); 
    send_to_char("        society members <sid>\r\n", ch); 
    send_to_char("        society research <character>\r\n", ch); 
    send_to_char("\r\n{CAdministration:{x\r\n", ch); 
    send_to_char("        society invite <character> <sid>\r\n", ch); 
    send_to_char("        society join <sid>\r\n", ch); 
    send_to_char("        society expel <character> <sid>\r\n", ch); 
    send_to_char("        society cleanup <character> <sid>\r\n", ch); 
    send_to_char("        society resign <sid>\r\n", ch); 
    send_to_char("        society promote <character> <sid>\r\n", ch); 
    send_to_char("        society demote <character> <sid>\r\n", ch); 
    send_to_char("        society test <character> <sid>\r\n", ch); 
    send_to_char("        society auth <character> <sid>\r\n", ch); 
    send_to_char("        society auth <character> <sid> <authority>\r\n", ch); 
    if (IS_IMMORTAL(ch)) send_to_char("        society reset <sid>\r\n", ch); 
    send_to_char("\r\n{CPolitics:{x\r\n", ch); 
    if (IS_IMMORTAL(ch)) send_to_char("        society subscribe <sid>\r\n", ch); 
    send_to_char("        society tell <sid> <message>\r\n", ch); 
    send_to_char("        society sign <sid>\r\n", ch); 
    send_to_char("        society foe <character> <sid>\r\n", ch); 
    send_to_char("        society pardon <character> <sid>\r\n", ch); 
    send_to_char("        society revolt <sid>\r\n", ch); 
    send_to_char("        society vote <sid> <leader>\r\n", ch); 
    send_to_char("        society law <sid> <type>\r\n", ch); 
    send_to_char("\r\n{CFinance:{x\r\n", ch); 
    send_to_char("        society bank <sid> <action>\r\n", ch); 
    send_to_char("        society tax <sid> <tax%>\r\n", ch); 
    return;
  }

 /* Extract the command... */

  args = one_argument(args, cmd);

 /* List all public societies... */

  if (!str_cmp(cmd, "list")) {
    do_society_list(ch, args);
  } else if (!str_cmp(cmd, "info")) {
    do_society_info(ch, args);
  } else if (!str_cmp(cmd, "members")) {
    do_society_members(ch, args);
  } else if (!str_cmp(cmd, "sign")) {
    do_society_sign(ch, args);
  } else if (!str_cmp(cmd, "revolt")) {
    do_society_revolt(ch, args);
  } else if (!str_cmp(cmd, "vote")) {
    do_society_vote(ch, args);
  } else if (!str_cmp(cmd, "invite")) {
    do_society_invite(ch, args);
  } else if (!str_cmp(cmd, "join")) {
    do_society_join(ch, args);
  } else if (!str_cmp(cmd, "subscribe")) {
    do_society_subscribe(ch, args);
  } else if (!str_cmp(cmd, "promote")) {
    do_society_promote(ch, args);
  } else if (!str_cmp(cmd, "resign")) {
    do_society_resign(ch, args);
  } else if (!str_cmp(cmd, "expel")) {
    do_society_expel(ch, args);
  } else if (!str_cmp(cmd, "cleanup")) {
    do_society_cleanup(ch, args);
  } else if (!str_cmp(cmd, "demote")) {
    do_society_demote(ch, args);
  } else if (!str_cmp(cmd, "auth")) {
    do_society_authorize(ch, args);
  } else if (!str_cmp(cmd, "test")) {
    do_society_test(ch, args);
  } else if (!str_cmp(cmd, "bank")) {
    do_society_bank(ch, args);
  } else if (!str_cmp(cmd, "tax")) {
    do_society_tax(ch, args);
  } else if (!str_cmp(cmd, "foe")) {
    do_society_foe(ch, args);
  } else if (!str_cmp(cmd, "pardon")) {
    do_society_pardon(ch, args);
  } else if (!str_cmp(cmd, "tell")) {
    do_society_tell(ch, args);
  } else if (!str_cmp(cmd, "research")) {
    do_society_research(ch, args);
  } else if (!str_cmp(cmd, "reset")) {
    do_society_reset(ch, args);
  } else {
    do_society(ch, "");
  }

  return;
}


void do_guard(CHAR_DATA *ch, char *args) {
char cmd[MAX_INPUT_LENGTH];
MEMBERSHIP *memb;
bool enforcer = FALSE;

   if (IS_IMMORTAL(ch)) {
       enforcer = TRUE;
   } else {
       for (memb = ch->memberships; memb; memb = memb->next) {
             if (memb->society->type == 5) enforcer = TRUE;
       }
   }
   
   if (!enforcer) {
        send_to_char("You don't belong to a law enforcment society.\r\n", ch);
        return;
   }

 /* Check for help request... */

  if ( args[0] == '\0' || args[0] == '?' ) {
    send_to_char("\r\n{CLaw Enforcment Commands:{x\r\n", ch); 
    send_to_char("        guard criminals\r\n", ch); 
    send_to_char("        guard law <sid> <type>\r\n", ch); 
    send_to_char("        guard imprison <character> [!]\r\n", ch); 
    send_to_char("        guard pardon <character>\r\n", ch); 
    return;
  }

  args = one_argument(args, cmd);

 /* List all public societies... */

  if (!str_cmp(cmd, "law")) {
    do_guard_law(ch, args);
  } else if (!str_cmp(cmd, "criminals")) {
    do_guard_criminals(ch, args);
  } else if (!str_cmp(cmd, "imprison")) {
    do_guard_imprison(ch, args);
  } else if (!str_cmp(cmd, "pardon")) {
    do_guard_pardon(ch, args);
  } else {
    do_guard(ch, "");
  }

  return;
}


// -----------------------------------------------------------------------
//
// See if mobs are foes
//
// -----------------------------------------------------------------------

bool is_foe(CHAR_DATA *mob1, CHAR_DATA *mob2) {
MEMBERSHIP *memb, *memb2;

 /* Never your own foe... */

  if ( mob1 == NULL 
    || mob2 == NULL 
    || mob1 == mob2) {
    return FALSE;
  } 

 /* Yes, if fighting each other... */ 

  if (mob1->fighting == mob2) {
    return TRUE;
  }

  if (mob2->fighting == mob1) {
    return TRUE;
  }

 /* Yes, if one is prey the other... */ 

  if (mob1->prey == mob2) {
    return TRUE;
  }

  if (mob2->prey == mob1) {
    return TRUE;
  }

 /* Yes, if they are fighting each others friends... */ 

  if ( mob1->fighting != NULL
    && is_same_group(mob2, mob1->fighting) ) {
    return TRUE;
  }

  if ( mob2->fighting != NULL
    && is_same_group(mob1, mob2->fighting) ) {
    return TRUE;
  }

 /* Yes, if they are prey each others friends... */ 

  if ( mob1->prey != NULL
    && mob1->prey != mob1->fighting 
    && is_same_group(mob2, mob1->prey) ) {
    return TRUE;
  }

  if ( mob2->prey != NULL
    && mob2->prey != mob2->fighting 
    && is_same_group(mob1, mob2->prey) ) {
    return TRUE;
  }

 /* Check for society foeing... */

  memb2 = mob1->memberships;

  while (memb2 != NULL) {

    if ( memb2->level != SOC_LEVEL_FOE
      && ( memb2->society->type == SOC_TYPE_CLAN
        || memb2->society->type == SOC_TYPE_POLITICAL
        || memb2->society->type == SOC_TYPE_CULT )) {
      memb = find_membership(mob2, memb2->society);
    
      if ( memb != NULL
        && memb->level == SOC_LEVEL_FOE ) { 
        return TRUE;
      }
    } 

    memb2 = memb2->next;
  }

  memb2 = mob2->memberships;

  while (memb2 != NULL) {

    if ( memb2->level != SOC_LEVEL_FOE
      && ( memb2->society->type == SOC_TYPE_CLAN
        || memb2->society->type == SOC_TYPE_POLITICAL
        || memb2->society->type == SOC_TYPE_CULT )) {
      memb = find_membership(mob1, memb2->society);
    
      if ( memb != NULL
        && memb->level == SOC_LEVEL_FOE ) { 
        return TRUE;
      }
    } 

    memb2 = memb2->next;
  }

  return FALSE;
}

// -----------------------------------------------------------------------
//
// See if mobs are friends
//
// -----------------------------------------------------------------------

bool is_friend(CHAR_DATA *mob1, CHAR_DATA *mob2) {

  MEMBERSHIP *memb, *memb2;

 /* Always your own friend... */

  if ( mob1 == NULL 
    || mob2 == NULL 
    || mob1 == mob2) {
    return TRUE;
  } 

 /* No, not if they are foes... */

  if (is_foe(mob1, mob2)) {
    return FALSE;
  }

 /* Yes, if they are in the same group... */

  if (is_same_group(mob1, mob2)) {
    return TRUE;
  }
 
 /* Yes, if they have a common enemy... */ 

  if ( mob1->fighting != NULL
    && mob2->fighting != NULL ) {

    if (mob1->fighting == mob2->fighting) {
      return TRUE;
    }

    if (is_same_group(mob1->fighting, mob2->fighting)) {
      return TRUE;
    } 
  }

  if ( mob1->prey != NULL
    && mob1->prey != mob1->fighting
    && mob2->fighting != NULL ) {

    if (mob1->prey == mob2->fighting) {
      return TRUE;
    }

    if (is_same_group(mob1->prey, mob2->fighting)) {
      return TRUE;
    } 
  }

  if ( mob1->fighting != NULL
    && mob2->prey != NULL
    && mob2->prey != mob2->fighting ) {

    if (mob1->fighting == mob2->prey) {
      return TRUE;
    }

    if (is_same_group(mob1->fighting, mob2->prey)) {
      return TRUE;
    } 
  }

  if ( mob1->prey != NULL
    && mob1->prey != mob1->fighting
    && mob2->prey != NULL
    && mob2->prey != mob2->fighting ) {

    if (mob1->prey == mob2->prey) {
      return TRUE;
    }

    if (is_same_group(mob1->prey, mob2->prey)) {
      return TRUE;
    } 
  }

 /* Check for society friendship... */

  memb2 = mob1->memberships;

  while (memb2 != NULL) {

    if ( memb2->level != SOC_LEVEL_FOE
      && ( memb2->society->type == SOC_TYPE_CLAN
        || memb2->society->type == SOC_TYPE_POLITICAL
        || memb2->society->type == SOC_TYPE_CULT )) {
      memb = find_membership(mob2, memb2->society);
    
      if ( memb != NULL
        && memb->level != SOC_LEVEL_FOE ) { 
        return TRUE;
      }
    } 

    memb2 = memb2->next;
  }

  memb2 = mob2->memberships;

  while (memb2 != NULL) {

    if ( memb2->level != SOC_LEVEL_FOE
      && ( memb2->society->type == SOC_TYPE_CLAN
        || memb2->society->type == SOC_TYPE_POLITICAL
        || memb2->society->type == SOC_TYPE_CULT )) {
      memb = find_membership(mob1, memb2->society);
    
      if ( memb != NULL
        && memb->level != SOC_LEVEL_FOE ) { 
        return TRUE;
      }
    } 

    memb2 = memb2->next;
  }

 /* Othewise, No */ 

  return FALSE;
}


void do_society_revolt(CHAR_DATA *ch, char *args) {
  SOCIETY *society;
  SOCIALITE *social;
  char arg1[MAX_STRING_LENGTH];
  int sid;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  args = one_argument(args, arg1);
  sid = atoi(arg1);

  if ( sid < 0
    || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  if (society->type == SOC_TYPE_RACE
  || society->type == SOC_TYPE_GUARD) {
    send_to_char("You can't revolt in a racial or guard society.\r\n", ch);
    return;
  }
  
  if (society->junta) {
    send_to_char("There is already a revolt going on.\r\n", ch);
    return;
  }
 
   if (IS_NPC(ch)) {
      social = find_socialite(society, ch->short_descr);
   } else {
      social = find_socialite(society, ch->name);
   }
   
   if (social == NULL) {
      send_to_char("You are no member!\r\n", ch);
      return;
   }

   if (social->rank == SOC_RANK_SUSPENDED) {
      send_to_char("Nobody trusts you!\r\n", ch);
      return;
   }

   free_string(society->junta);
   society->junta = strdup(capitalize(ch->name));
   society->votes = 0;
   society->timeout_day = time_info.day;
   society->timeout_month = time_info.month + 3;
   if (society->timeout_month > 12) society->timeout_month -= 12;
   
   sprintf(buf, "%d", society->id);
   sprintf(buf2, "%s has started a revolt for clan leadership.\r\nVote as soon as possible.\r\n", ch->name);
   save_socialites();
   make_note ("Society", "Overseer", buf, "revolt", 14, buf2);
   send_to_char("Ok.\r\n",ch);
   return;
}


void do_society_reset(CHAR_DATA *ch, char *args) {
  SOCIETY *society;
  char arg1[MAX_STRING_LENGTH];
  int sid;

  args = one_argument(args, arg1);
  sid = atoi(arg1);

  if (!IS_IMMORTAL(ch)) {
    send_to_char("You are mortal.\r\n", ch);
    return;
  }


  if ( sid < 0
  || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

   free_string(society->junta);
   society->junta = NULL;
   society->votes = 0;
   society->timeout_day = 0;
   society->timeout_month = 0;
   
   send_to_char("Ok.\r\n",ch);
   save_socialites();
   return;
}


void do_society_vote(CHAR_DATA *ch, char *args) {
  SOCIETY *society;
  SOCIALITE *social;
  int sid;
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];

 /* First, chop up the parms... */

  args = one_argument(args, arg1);
  args = one_argument(args, arg2);

  sid = atoi(arg1);

  if ( sid < 0
    || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  if (!society->junta) {
    send_to_char("There is no revolt going on.\r\n", ch);
    return;
  }

  if (arg2[0] == '\0') {
     sprintf_to_char(ch, "{gRevolt for Leadership on %s{x\r\n", society->name);
     sprintf_to_char(ch, "Junta leader: %s\r\n", society->junta);
     if (society->votes > 0) sprintf_to_char(ch, "Votes for junta: %d\r\n", society->votes);
     else sprintf_to_char(ch, "Votes for leaders: %d\r\n", -society->votes);
     sprintf_to_char(ch, "\r\n(You must vote until %s - %d)\r\n", month_name[society->timeout_month], society->timeout_day);
     return;
  }

    if (IS_NPC(ch)) {
      social = find_socialite(society, ch->short_descr);
    } else {
      social = find_socialite(society, ch->name);
    }
   
    if (social == NULL) {
      send_to_char("You are not a full member!\r\n", ch);
      return;
    }

    if (social->vote_cast) {
      send_to_char("You already voted!\r\n", ch);
      return;
    }

    if (social->rank < 1) {
      send_to_char("You can't vote!\r\n", ch);
      return;
    }

    if (!str_cmp(society->junta, capitalize(arg2))) {
        society->votes += social->rank;
        sprintf_to_char(ch, "You vote for %s.\r\n", capitalize(society->junta));
    } else {
        society->votes -= social->rank;
        send_to_char("You vote for the society leaders.\r\n", ch);
    }
    save_socialites();
    return;
}


void society_update(void) {
SOCIETY *society;

  society = society_chain;
  if (society == NULL) return;

  while (society != NULL) {
      if (society->junta) {
          if (society->timeout_month == time_info.month
          && society->timeout_day <= time_info.day) {
                resolve_revolt(society);
          }
      }
      society = society->next;
  } 

  return;
}


void resolve_revolt( SOCIETY *society) {
SOCIALITE *socmemb;
char buf[MAX_STRING_LENGTH];
char buf2[MAX_STRING_LENGTH];

    socmemb = society->members;

    if (society->votes > 0) {
         sprintf(buf, "%d", society->id);
         sprintf(buf2, "The revolt has been won by %s.\r\nAll hail the new leader.\r\n", society->junta);
         make_note ("Society", "Overseer", buf, "revolt", 14, buf2);

         while (socmemb != NULL) {
               if (socmemb->rank == SOC_RANK_LEADER) {
                         socmemb->rank = SOC_RANK_MEMBER;
                         socmemb->authority = SOC_AUTH_NONE;
               }
               if (!str_cmp(socmemb->name, capitalize(society->junta))) {
                           socmemb->rank = SOC_RANK_LEADER;
                           socmemb->authority = SOC_AUTH_INVITE | SOC_AUTH_EXPEL | SOC_AUTH_BANK | SOC_AUTH_AUTH | SOC_AUTH_PROMOTE | SOC_AUTH_DEMOTE | SOC_AUTH_TEST | SOC_AUTH_FOE | SOC_AUTH_PARDON | SOC_AUTH_TAX;
               }

               socmemb = socmemb->next;
         }
    } else {
         sprintf(buf, "%d", society->id);
         make_note ("Society", "Overseer", buf, "revolt", 14, "The revolt has been suppressed.\r\n");
         while (socmemb != NULL) {
               if (!str_cmp(socmemb->name, capitalize(society->junta))) {
                          socmemb->rank = SOC_RANK_SUSPENDED;
                          socmemb->authority = SOC_AUTH_NONE;
               }
               socmemb = socmemb->next;
         }
    }

    free_string(society->junta);
    society->junta = NULL;
    society->votes = 0;
    society->timeout_month = 0;
    society->timeout_day = 0;
    save_socialites();
    return;
}


void do_society_tax(CHAR_DATA *ch, char *argument) {
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int sid;
  int tax;
  SOCIETY *society;
  SOCIALITE *social;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (arg1[0] == '\0') {
     send_to_char("society tax <sid> <tax>\r\n", ch);
     return;
  }

  sid = atoi(arg1);

  if ( sid < 0
    || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

 /* Are they allowed to authorize? */

  if (!IS_IMMORTAL(ch)) {
      if (IS_NPC(ch)) {
          social = find_socialite(society, ch->short_descr);
      } else {
          social = find_socialite(society, ch->name);
      }
   
      if (social == NULL) {
          send_to_char("You are not a full member yourself!\r\n", ch);
          return;
      }

      if (!IS_SET(social->authority, SOC_AUTH_TAX)) {
          send_to_char("You do not have authority to do that!\r\n", ch);
          return;
      }
  } 
 
  if (arg2[0] == '\0') {
     sprintf_to_char(ch, "{g%s{x\r\n", society->name);
     sprintf_to_char(ch, "Tax: %d\r\n", society->tax);
     return;
  }

  tax = atoi(arg2);
  if (tax < 0 || tax >50) {
          send_to_char("This tax value doesn't make sense!\r\n", ch);
          return;
  }
 
  society->tax = tax;
  sprintf_to_char(ch, "Tax of %s set to %d.\r\n", society->name, society->tax);
  save_socialites();
  return;
}


int soc_tax(CHAR_DATA *ch, int gold, int currency) {
MEMBERSHIP *memb;
CHAR_DATA *teller;
SHOP_DATA *pShop;
char soc_acnt[MAX_INPUT_LENGTH];
int split = 0;
int i;
bool found = FALSE;
bool ok = FALSE;

    memb = ch->memberships;
    while (memb != NULL) {

      if ( memb->society != NULL
      && memb->level >= SOC_LEVEL_MEMBERSHIP ) {
          if ( is_member(ch, memb->society)) {
              if (memb->society->tax >0) { 
                   split = gold * memb->society->tax / 100;
                   gold -=split;
                   if (split > 0) {
                   sprintf(soc_acnt, "Society_%d", memb->society->id);

                   for (teller = char_list; teller != NULL ; teller = teller->next ) {
                        if (ok) continue;
                        if (IS_NPC(teller)) {
                            pShop = teller->pIndexData->pShop;
                            if (pShop != NULL ) {
                                 found = FALSE;
                                 for(i = 0; i < MAX_TRADE; i++) {
                                     if (pShop->buy_type[i] == 999) found = TRUE;
                                 }
                                 if (found && memb->society->bank) {
                                        if(str_cmp(memb->society->bank, teller->in_room->name)) found = FALSE;
                                 }
                                 if (found) {
                                    ok = FALSE;
                                    ok = identify_teller(teller->in_room->name, soc_acnt);
                                    if (ok) {
                                           do_deposit_raw(teller, split, currency, teller->in_room->name, soc_acnt);
                                           if (IS_SET(ch->plr, PLR_AUTOTAX)) sprintf_to_char(ch, "%d %s Tax for %s deposited on %s.\r\n", split, flag_string(currency_type, currency), memb->society->name, teller->in_room->name);
                                           break;
                                    }
                                 }
                            }
                         }
                   }
                   }
              }
          }
      }
      memb = memb->next;
    }
    if (!found || !ok) gold +=split;   
    if (gold < 0) gold = 0;
    if (IS_SET(ch->plr, PLR_AUTOTAX)) sprintf_to_char(ch, "%d %s taken.\r\n", gold, flag_string(currency_type, currency));
    return gold;
}


void do_guard_law(CHAR_DATA *ch, char *argument) {
  AREA_DATA *area;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int sid;
  SOCIETY *society;
  SOCIALITE *social;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (arg1[0] == '\0') {
     send_to_char("society law <sid> <normal / martial>\r\n", ch);
     return;
  }

  sid = atoi(arg1);

  if ( sid < 0
    || sid > free_socid ) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  society = find_society_by_id(sid);

  if (society == NULL) {
    send_to_char("No society with that id.\r\n", ch);
    return;
  }

  if (society->type != 5) {
    send_to_char("This society can't set the law.\r\n", ch);
    return;
  }

  if (society->home_area == 0) {
    send_to_char("No home area set.\r\n", ch);
    return;
  }

  area =get_area_data(society->home_area);

 /* Are they allowed to authorize? */

  if (!IS_IMMORTAL(ch)) {
      if (IS_NPC(ch)) {
          social = find_socialite(society, ch->short_descr);
      } else {
          social = find_socialite(society, ch->name);
      }
   
      if (social == NULL) {
          send_to_char("You are not a full member yourself!\r\n", ch);
          return;
      }

      if (!IS_SET(social->authority, SOC_AUTH_LAW)) {
          send_to_char("You do not have authority to do that!\r\n", ch);
          return;
      }
  } 
 
  if (arg2[0] == '\0') {
     send_to_char("Syntax: guard law <sid> <normal / martial>\r\n", ch);
     return;
  }

  if (!str_cmp(arg2, "martial")) {
       area->martial = 1;
       send_to_char("Martial law set.", ch);
  } else {
       area->martial = 0;
       send_to_char("Martial law removed.", ch);
  }
  SET_BIT(area->area_flags, AREA_CHANGED);

  return;
}


void do_guard_imprison(CHAR_DATA *ch, char *argument) {
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
CHAR_DATA *victim;
ROOM_INDEX_DATA *prison;
MEMBERSHIP *memb;
bool ok = FALSE;
bool remove = TRUE;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (arg1[0] == '\0') {
     send_to_char("Syntax: guard imprison <character> [!]\r\n", ch);
     return;
  }

  if ((victim = get_char_room(ch, arg1)) == NULL) {
     send_to_char("They're not here.\r\n", ch);
     return;
  }

  if (victim->position > POS_STUNNED) {
     send_to_char("They're will refuse.\r\n", ch);
     return;
  }

  if (IS_NPC(ch)) {
     send_to_char("They're harmless.\r\n", ch);
     return;
  }

  if (!IS_SET(victim->act, ACT_CRIMINAL)) {
     send_to_char("That's no criminal.\r\n", ch);
     return;
  }

  if (arg2[0] == '!') remove = FALSE;

  for (memb = ch->memberships; memb; memb = memb->next) {
             if (memb->society->type == 5 && memb->society->home_area > 0) {
                 if (ch->in_room->area == get_area_data(memb->society->home_area)) {
                     ok = TRUE;
                     break;
                 }
             }
 }

  if (!ok) {   
     send_to_char("You don't have the rights to.\r\n", ch);
     return;
  }

  if (ch->in_room->area->prison == 0) {
     send_to_char("There is no prison in town.\r\n", ch);
     return;
  }

  prison = get_room_index(ch->in_room->area->prison);
  act("$N takes $n away.", victim, NULL, ch, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, prison);
  sprintf_to_char(victim, "You've been taken to %s.\r\n", prison->name);
  sprintf_to_char(ch, "You take %s to %s.\r\n", capitalize(victim->short_descr), prison->name);
  if (remove) REMOVE_BIT(victim->act, ACT_CRIMINAL);
  return;
}


void do_guard_criminals(CHAR_DATA *ch, char *argument) {
CHAR_DATA *crim;
bool found = FALSE;

    for (crim = char_list; crim; crim = crim->next) {
         if (IS_NPC(crim)) continue;
         if (!IS_SET(crim->act, ACT_CRIMINAL)) continue;

         if (!found) {
            send_to_char("Wanted:\r\n", ch);
            found = TRUE;
         }
         sprintf_to_char(ch, "    %s\r\n", capitalize(crim->name));
    }

    if (!found) send_to_char("No criminals have been found.\r\n", ch);
    return;
}


void do_guard_pardon(CHAR_DATA *ch, char *argument) {
char arg1[MAX_INPUT_LENGTH];
CHAR_DATA *victim;
MEMBERSHIP *memb;
bool ok = FALSE;

  argument = one_argument(argument, arg1);

  if (arg1[0] == '\0') {
     send_to_char("Syntax: guard pardon <character>\r\n", ch);
     return;
  }

  if ((victim = get_char_room(ch, arg1)) == NULL) {
     send_to_char("They're not here.\r\n", ch);
     return;
  }

  if (IS_NPC(ch)) {
     send_to_char("They're harmless.\r\n", ch);
     return;
  }

  if (!IS_SET(victim->act, ACT_CRIMINAL)) {
     send_to_char("That's no criminal.\r\n", ch);
     return;
  }

  for (memb = ch->memberships; memb; memb = memb->next) {
             if (memb->society->type == 5 && memb->society->home_area > 0) {
                 if (ch->in_room->area == get_area_data(memb->society->home_area)) {
                     ok = TRUE;
                     break;
                 }
             }
 }

  if (!ok) {   
     send_to_char("You don't have the rights to.\r\n", ch);
     return;
  }

  REMOVE_BIT(victim->act, ACT_CRIMINAL);
  sprintf_to_char(ch, "You pardon %s.\r\n", capitalize(victim->short_descr));
  return;
}

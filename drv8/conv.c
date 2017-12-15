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
#include "conv.h"
#include "econd.h"
#include "mob.h"
#include "skill.h"
#include "gsn.h"


/* PC Conversation record blocks... */

static CONV_RECORD *free_cr = NULL;

static CONV_SUB_RECORD *free_csr = NULL;

CONV_RECORD *get_conv_record() {

  CONV_RECORD *cr;

  if (free_cr == NULL) {
    cr = (CONV_RECORD *) alloc_perm(sizeof(*cr));
  } else {
    cr = free_cr;
    free_cr = cr->next;
  }  

  cr->conv_id = CONV_NONE;
  cr->csr = NULL;
  cr->next = NULL;

  return cr;
}

void free_conv_record(CONV_RECORD *cr) {

  cr->next = free_cr;

  free_cr = cr;

  return;
}

CONV_SUB_RECORD *get_conv_sub_record() {

  CONV_SUB_RECORD *csr;

  if (free_csr == NULL) {
    csr = (CONV_SUB_RECORD *) alloc_perm(sizeof(*csr));
  } else {
    csr = free_csr;
    free_csr = csr->next;
  }  

  csr->sub_id = CONV_SUB_NONE;
  csr->state = CONV_STATE_NONE;
  csr->next = NULL;

  return csr;
}

void free_conv_sub_record(CONV_SUB_RECORD *csr) {
 
  csr->next = free_csr;

  free_csr = csr;

  return;
}

/* Find a characters conversation subject record... */

CONV_SUB_RECORD *find_csr(CHAR_DATA *ch, int conv_id, int sub_id) {

  CONV_RECORD *cr;

  CONV_SUB_RECORD *csr;

 /* Search for conversation record... */

  cr = ch->cr;

  while ( cr != NULL
       && cr->conv_id != conv_id) {
    cr = cr->next;
  }

 /* If not found, there is no subject record... */

  if (cr == NULL) {
    return NULL;
  }

 /* Search for conversation subject record... */

  csr = cr->csr;

  while ( csr != NULL
       && csr->sub_id != sub_id) {
    csr = csr->next;
  }

  return csr;
}

/* Set a characters conversation subject state... */

void set_csr(CHAR_DATA *ch, int conv_id, int sub_id, int state) {

  CONV_RECORD *cr;

  CONV_SUB_RECORD *csr;

 /* Search for conversation record... */

  cr = ch->cr;

  while ( cr != NULL
       && cr->conv_id != conv_id) {
    cr = cr->next;
  }

 /* If not found, we have to create one... */

  if (cr == NULL) {
    cr = get_conv_record();
    cr->conv_id = conv_id;
    cr->next = ch->cr;
    ch->cr = cr;
  }

 /* Search for conversation subject record... */

  csr = cr->csr;

  while ( csr != NULL
       && csr->sub_id != sub_id) {
    csr = csr->next;
  }

 /* If not found. we have to create one... */

  if (csr == NULL) {
    csr = get_conv_sub_record();
    csr->sub_id = sub_id;
    csr->next = cr->csr;
    cr->csr = csr; 
  }

 /* Now set the status... */

  csr->state = state;

 /* Note that character save will discard any csrs returned to state 0 */

  return;
}

/* MOB conversation definition blocks... */
 
static CONV_DATA *free_cd = NULL;

static CONV_SUB_DATA *free_csd = NULL;

static CONV_SUB_TRANS_DATA *free_cstd = NULL;

CONV_DATA *get_conv_data() {

  CONV_DATA *cd;

  if (free_cd == NULL) {
    cd = (CONV_DATA *) alloc_perm(sizeof(*cd));
  } else {
    cd = free_cd;
    free_cd = cd->next;
  }  

  cd->conv_id = CONV_NONE;
  cd->name = NULL;	
  cd->csd = NULL;
  cd->next = NULL;

  return cd;
}

void free_conv_data(CONV_DATA *cd) {

  CONV_SUB_DATA *csd, *next_csd;

  csd = cd->csd;

  while (csd != NULL) {
    next_csd = csd->next;
    free_conv_sub_data(csd);
    csd = next_csd;
  }

  if (cd->name != NULL) {
    free_string(cd->name);
  }
 
  cd->next = free_cd;

  free_cd = cd;

  return;
}

CONV_SUB_DATA *get_conv_sub_data() {

  CONV_SUB_DATA *csd;

  if (free_csd == NULL) {
    csd = (CONV_SUB_DATA *) alloc_perm(sizeof(*csd));
  } else {
    csd = free_csd;
    free_csd = csd->next;
  }  

  csd->sub_id = CONV_SUB_NONE;
  csd->name = NULL;	
  csd->kwds = NULL;
  csd->cstd = NULL;
  csd->next = NULL;

  return csd;
}

void free_conv_sub_data(CONV_SUB_DATA *csd) {

  CONV_SUB_TRANS_DATA *cstd, *next_cstd;

  if (csd->name != NULL) {
    free_string(csd->name);
  }
 
  if (csd->kwds != NULL) {
    free_string(csd->kwds);
  }

  cstd = csd->cstd;

  while (cstd != NULL) {
    next_cstd = cstd->next;
    free_conv_sub_trans_data(cstd);
    cstd = next_cstd;
  }
 
  csd->next = free_csd;

  free_csd = csd;

  return;
}

CONV_SUB_TRANS_DATA *get_conv_sub_trans_data() {

  CONV_SUB_TRANS_DATA *cstd;

  if (free_cstd == NULL) {
    cstd = (CONV_SUB_TRANS_DATA *) alloc_perm(sizeof(*cstd));
  } else {
    cstd = free_cstd;
    free_cstd = cstd->next;
  }  

  cstd->in = CONV_STATE_NONE;
  cstd->out = CONV_STATE_NONE;	
  cstd->cond = NULL;
  cstd->next = NULL;
  cstd->action = NULL;

  return cstd;
}

void free_conv_sub_trans_data(CONV_SUB_TRANS_DATA *cstd) {

  cstd->next = free_cstd;

  free_cstd = cstd;

  return;
}

/* Find a mobs conversation data record... */

CONV_DATA *find_cd(MOB_INDEX_DATA *mob, int conv_id) {

  CONV_DATA *cd;

 /* Search for conversation record... */

  cd = mob->cd;

  while ( cd != NULL
       && cd->conv_id != conv_id) {
    cd = cd->next;
  }

 /* Got it... */

  return cd;
}
 
/* Find a mobs conversation subject data record... */

CONV_SUB_DATA *find_csd(CONV_DATA *cd, int sub_id) {

  CONV_SUB_DATA *csd;

 /* Search for conversation record... */

  csd = cd->csd;

  while ( csd != NULL
       && csd->sub_id != sub_id) {
    csd = csd->next;
  }

 /* Got it... */

  return csd;
}

/* Find a mobs conversation subject transition data record... */

CONV_SUB_TRANS_DATA *find_cstd(CONV_SUB_DATA *csd, int seq) {

  CONV_SUB_TRANS_DATA *cstd;

 /* Search for conversation record... */

  cstd = csd->cstd;

  while ( cstd != NULL
       && cstd->seq != seq) {
    cstd = cstd->next;
  }

 /* Got it... */

  return cstd;
}

/* Insert a cstd into the ordered list... */

void insert_cstd(CONV_SUB_TRANS_DATA *cstd, CONV_SUB_DATA *csd) {

  CONV_SUB_TRANS_DATA *old_cstd;

 /* Simple case first... */

  if (csd->cstd == NULL) {
    csd->cstd = cstd;
    return;
  }

 /* Middling case next... */

  if ( cstd->seq < csd->cstd->seq ) {

    cstd->next = csd->cstd;
    csd->cstd = cstd;

    return;
  }
 
 /* Search for conversation record... */

  old_cstd = csd->cstd;

  while ( old_cstd->next != NULL
       && old_cstd->next->seq < cstd->seq) {
    old_cstd = old_cstd->next;
  }

 /* Ok, found where to insert... */

  cstd->next = old_cstd->next;
  old_cstd->next = cstd;

 /* All done... */

  return;
}

/* Write a conversation out to a file... */
/* (Matching read code is in db2.c) */

void write_conv(FILE *fp, CONV_DATA *cd) {

  CONV_SUB_DATA *csd;
  CONV_SUB_TRANS_DATA *cstd;

  while (cd != NULL) {

    fprintf(fp, "Conv %d '%s'\n", cd->conv_id, cd->name);

    csd = cd->csd;

    while (csd != NULL) {

      fprintf(fp, "ConvSub %d '%s' '%s'\n", csd->sub_id, csd->name, csd->kwds);

      cstd = csd->cstd;

      while (cstd != NULL) {

        fprintf(fp, "ConvSubTrans %d %d %d %s~\n", cstd->seq, cstd->in, cstd->out, cstd->action); 

        if (cstd->cond != NULL) {
          write_econd(cstd->cond, fp, "ConvCond");
        }

        cstd = cstd->next; 
      } 

      csd = csd->next;
    }

    cd = cd->next;
  }

  return;
}
 
/* Hold a conversation... */

void do_converse(CHAR_DATA *ch, char *argument) {
char mob_name[MAX_STRING_LENGTH];
char keywords[MAX_STRING_LENGTH];
char actword[MAX_STRING_LENGTH];
CHAR_DATA *mob;
MOB_INDEX_DATA *mobIndex;
CONV_SUB_RECORD *csr;
CONV_DATA *cd;
CONV_SUB_DATA *csd;
CONV_SUB_TRANS_DATA *cstd;
bool reacted;
bool found;
int in_state;
int scrid;
MOB_SCRIPT *scr;
MOB_CMD_CONTEXT *mcc;
char *p2;
 
 /* Grab arguments.. */

  argument = one_argument(argument, mob_name);
  argument = one_argument(argument, keywords);

 /* Check parameters... */

  if ( mob_name[0] == '\0' 
    || mob_name[0] == '?'
    || argument[0] != '\0' ) {
    send_to_char("Syntax: talk [mob]\r\n", ch);
    send_to_char("        talk [mob] [subject]\r\n", ch);
    send_to_char("Aliases: ask, converse, tell\r\n", ch);
    return;
  }   

 /* Default keywords... */

  if (keywords[0] == 0) {
    strcpy(keywords, "nothing");
  }

 /* Ok, find the mob... */

  mob = get_char_room(ch, mob_name);

  if ( mob == NULL ) {
    send_to_char( "No one by that name here...\r\n", ch );
    return;
  }

 /* Tell player and mob what's going down... */

  sprintf_to_char(ch, "You talk with %s about '%s'\r\n", mob->short_descr, keywords);
  sprintf_to_char(mob, "%s talks with you about '%s'\r\n", ch->short_descr, keywords);

 /* For a PC, that's all... */

  if (!IS_NPC(mob)) return;

 /* Get mob index data... */

  mobIndex = mob->pIndexData;

  if (mobIndex == NULL) return;

 /* Get a command context... */

  mcc = get_mcc(ch, ch, mob, random_char(mob), NULL, NULL, 0 , NULL);

 /* Now check each conversation... */

  reacted = FALSE;

  cd = mobIndex->cd;

  while (cd != NULL) {

    csd = cd->csd;

    while (csd != NULL && !reacted) {

     /* See if we're interested in this subject? */

      if (is_name(keywords, csd->kwds)) {

       /* Find the characters record for this subject... */

        csr = find_csr(ch, cd->conv_id, csd->sub_id);

       /* Work out the input state... */

        if (csr == NULL) {
          in_state = CONV_STATE_NONE; 
        } else {
          in_state = csr->state;
        } 

       /* Now, work out the transition... */

        found = FALSE;

        cstd = csd->cstd;

        while (cstd != NULL && !found) {

          if  ( ( in_state == cstd->in   
               || cstd->in == -1) 
            &&  ec_satisfied_mcc(mcc, cstd->cond, TRUE)) {

           /* Got one, so action it... */ 

            if (cstd->out != -1) {
              set_csr(ch, cd->conv_id, csd->sub_id, cstd->out);
            }

            p2 = one_argument(cstd->action, actword);

            if (!str_cmp(actword, "runscript")) {

              one_argument(p2, actword); 
          
              scrid = atoi(actword);

              scr = find_script(mob, scrid);

              if (scr == NULL) {
                enqueue_mob_cmd(mob, "emote looks a bit embarrased!", 0, 0);
              } else {
                mcc->mob = mob;
                enqueue_script(mob, scr, mcc);
                mcc->mob = ch; 
              }  
            } else {
              enqueue_mob_cmd(mob, cstd->action, 1, 0);
            } 

            found = TRUE;
            reacted = TRUE;
          }

          cstd = cstd->next;
        }      
      }

     /* Ok, try the next subject... */   

      csd = csd->next; 
    }

   /* Ok, try the next conversation... */

    cd = cd->next;
  }

 /* Lose the command context... */

  free_mcc(mcc);

 /* Default reaction... */

  if (!reacted) {
    chatperform(mob, ch, keywords);
  }
 
  return;
}


void chatperform(CHAR_DATA *ch, CHAR_DATA *victim, char* msg) {
char* reply = NULL;

     if( !IS_NPC(ch) || (victim && IS_NPC(victim))) return;

#ifdef ELIZA_CHAT
     reply = dochat(ch->name, msg, (char*)( victim ? victim->name : "you" ) );
#endif

     if(reply) {
        switch(reply[0]) {

            case '\0':
               break;

            case '"' :
               do_say(ch, capitalize(reply+1));
               break;

            case ':' :
               do_emote(ch, reply+1);
               break;

            case '!' :
               interpret(ch, reply+1 );
               break;

            default :
               do_say(ch, capitalize(reply));
               break;
        }
    }
    return;
}


/* Interrogate someone... */

void do_interrogate(CHAR_DATA *ch, char *argument) {
char mob_name[MAX_STRING_LENGTH];
char buf[MAX_STRING_LENGTH];
CHAR_DATA *mob;
MOB_INDEX_DATA *mobIndex;
CONV_SUB_RECORD *csr;
CONV_DATA *cd;
CONV_SUB_DATA *csd;
CONV_SUB_TRANS_DATA *cstd;
bool reacted;
bool found;
int in_state;
int skill;
char *cbuf;
MOB_CMD_CONTEXT *mcc;

 /* Grab arguments.. */

  argument = one_argument(argument, mob_name);

 /* Check parameters... */

  if ( mob_name[0] == '\0' 
    || mob_name[0] == '?'
    || argument[0] != '\0' ) {
    send_to_char("Syntax: interrogate [mob]\r\n", ch);
    return;
  }   

 /* Ok, find the mob... */

  mob = get_char_room(ch, mob_name);

  if ( mob == NULL ) {
    send_to_char( "No one by that name here...\r\n", ch );
    return;
  }

 /* Tell player and mob what's going down... */

  sprintf_to_char(ch, "You interrogate %s\r\n", mob->short_descr);
  sprintf_to_char(mob, "%s interrograte you!\r\n", ch->short_descr);

 /* For a PC, that's all... */

  if (!IS_NPC(mob)) return;

 /* Get mob index data... */

  mobIndex = mob->pIndexData;

  if (mobIndex == NULL) return;

 /* Check skill... */

  skill = get_skill(ch, gsn_interrogate);
  if ( skill == 0 ) skill = -20;

 /* Get a command context... */

  mcc = get_mcc(ch, ch, mob, random_char(mob), NULL, NULL, 0 , NULL);

 /* Now check each conversation... */

  reacted = FALSE;

  cd = mobIndex->cd;

  while (cd != NULL) {

    csd = cd->csd;

    while (csd != NULL) {

     /* Find the characters record for this subject... */

      csr = find_csr(ch, cd->conv_id, csd->sub_id);

     /* Work out the input state... */

      if (csr == NULL) {
        in_state = CONV_STATE_NONE; 
      } else {
        in_state = csr->state;
      } 

     /* Now, work out the transition... */

      found = FALSE;

      cstd = csd->cstd;

      while (cstd != NULL && !found) {

        if  ( ( in_state == cstd->in   
             || cstd->in == -1) 
           && ec_satisfied_mcc(mcc, cstd->cond, TRUE)
           && skill + number_open() > 100 ) {

          found = TRUE;

          check_improve(ch, gsn_interrogate, TRUE, 5);

         /* Got one, so action it... */ 

          if ( !reacted ) {
            sprintf(buf, "%s knows about:\r\n",
                          mob->short_descr);
            buf[0] = toupper(buf[0]);
            send_to_char(buf, ch);
            reacted = TRUE;
          }

          sprintf(buf, "  %s (%s)\r\n",
                       csd->name,
                       csd->kwds);
          send_to_char(buf, ch);
        }      

        cstd = cstd->next;
      }

     /* Ok, try the next subject... */   

      csd = csd->next; 
    }

   /* Ok, try the next conversation... */

    cd = cd->next;
  }

 /* Lose the command context... */

  free_mcc(mcc);

 /* Default reaction... */

  if (!reacted) {
    sprintf(buf, "%s has nothing to say!\r\n", 
                 mob->short_descr);
    cbuf = capitalize(buf);
    send_to_char(cbuf, ch);
  }
 
 /* All done... */

  return;
}


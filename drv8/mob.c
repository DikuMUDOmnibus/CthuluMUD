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
#include "mob.h"
#include "skill.h"
#include "profile.h"

static MOB_CMD_BUF *free_mcbs = NULL;

/* Get and free MCBs... */

MOB_CMD_BUF *get_mcb() {

  MOB_CMD_BUF *mcb;

  if (free_mcbs == NULL) { 
    mcb = (MOB_CMD_BUF *) alloc_perm(sizeof(*mcb));
  } else {
    mcb = free_mcbs;
    free_mcbs = mcb->next;
  }

  mcb->delay 	= 0;
  mcb->cmd_id	= 0;
  mcb->cmd 	= NULL;
  mcb->next 	= NULL;

  return mcb;
}

void free_mcb(MOB_CMD_BUF *mcb) {

  if (mcb == NULL) return;
  if (mcb->cmd != NULL) free_string(mcb->cmd);

  mcb->cmd = NULL;
 
  mcb->next = free_mcbs;
  free_mcbs = mcb;

  return;
}

/* Enqueue an action to a mob... */

void enqueue_mob_cmd(CHAR_DATA *ch, char *cmd, int delay, int cmd_id) {

  MOB_CMD_BUF *mcb, *last_mcb, *prev_mcb;

  int offset;

 /* Validate parms... */

  if ( cmd == NULL
    || cmd[0] == '\0') {
    return;
  }
 
  if (delay < 0) {
    delay = 0;
  }
 
 /* Ok, create the mcb... */
 
  mcb = get_mcb();

  mcb->cmd = strdup(cmd);
  mcb->cmd_id = cmd_id;

 /* Find point in list, sorted by time... */  

  last_mcb = ch->mcb;
  prev_mcb = NULL;
  offset = delay;

  while ( last_mcb != NULL
       && offset >= last_mcb->delay) {

    offset -= last_mcb->delay;

    prev_mcb = last_mcb;
    last_mcb = last_mcb->next;
  }

 /* Translate the new mcbs delay into a relative offset... */

  mcb->delay = offset;

 /* Add the new mcb into the list... */

  if (prev_mcb == NULL) {
    mcb->next = ch->mcb; 
    ch->mcb = mcb;
  } else {
    mcb->next = prev_mcb->next; 
    prev_mcb->next = mcb;
  }

 /* Reduce time interval before next event... */

  if (mcb->next != NULL) {
    mcb->next->delay -= mcb->delay;
  }

 /* All done... */

  return;
}

/* Purge unwanted commands... */

void purge_mcbs(CHAR_DATA *ch, int cmd_id) {

  MOB_CMD_BUF *mcb, *next_mcb, *prev_mcb;

  if (ch->mcb == NULL) {
    return;
  }

  next_mcb = NULL;
  prev_mcb = NULL;

  mcb = ch->mcb;

  while (mcb != NULL) {
    next_mcb = mcb->next;
    
    if (mcb->cmd_id == cmd_id) {

     /* Take this mcb out of the chain... */

      if (prev_mcb == NULL) {
        ch->mcb = next_mcb; 
      } else {
        prev_mcb->next = next_mcb;
      }

     /* Adjust relative delay on following mcb to compensate... */

      if (next_mcb != NULL) {
        next_mcb->delay += mcb->delay;
      }
   
     /* Lose this mcb... */

      free_mcb(mcb);

    } else {
      prev_mcb = mcb;
    }

    mcb = next_mcb;
  }

  if (prev_mcb == NULL) {
    ch->mcb = NULL;
  }

  return;
}

/* Dump a mobs unused MCBs... */

void release_mcbs(CHAR_DATA *ch) {

  MOB_CMD_BUF *mcb, *next_mcb;

  next_mcb = NULL;

  mcb = ch->mcb;

  while (mcb != NULL) {
    next_mcb = mcb->next;
    free_mcb(mcb);
    mcb = next_mcb;
  }

  ch->mcb = NULL;

  return;
}

/* Execute an MCB... */

void execute_mcb(CHAR_DATA *ch) {

  MOB_CMD_BUF *mcb;

  if (ch->mcb == NULL) {
    return;
  }

  mcb = ch->mcb;

  ch->mcb = mcb->next;

  interpret(ch, mcb->cmd);

  mcb->next = NULL;

  free_mcb(mcb);

  return;
}

/* Scripts */

static MOB_SCRIPT *free_mscrs = NULL;

/* Get and free MSCRs... */

MOB_SCRIPT *get_mscr() {

  MOB_SCRIPT *mscr;

  if (free_mscrs == NULL) { 
    mscr = (MOB_SCRIPT *) alloc_perm(sizeof(*mscr));
  } else {
    mscr = free_mscrs;
    free_mscrs = mscr->next;
  }

  mscr->id 	= 0;
  mscr->name 	= NULL;
  mscr->next 	= NULL;
  mscr->lines 	= NULL;

  return mscr;
}

void free_mscr(MOB_SCRIPT *mscr) {

  if (mscr == NULL) {
    return;
  }

  release_script_lines(mscr);

  if (mscr->name != NULL) {
    free_string(mscr->name);
  }

  mscr->name = NULL;
  mscr->lines = NULL;
 
  mscr->next = free_mscrs;
  free_mscrs = mscr;

  return;
}

void release_scripts(MOB_SCRIPT *base) {

  MOB_SCRIPT *mscr, *next_mscr;

 /* Nothing to do? */

  if (base == NULL) {
    return;
  }

 /* Ok, let 'em go... */ 

  next_mscr = NULL;
  mscr = base;

  while (mscr != NULL) {
    next_mscr = mscr->next;

    free_mscr(mscr);

    mscr = next_mscr;
  }

 /* All done... */

  return;
}  

/* Enqueue a script to a mob... */

void enqueue_script(	CHAR_DATA *mob,
			MOB_SCRIPT *mscr,
                        MOB_CMD_CONTEXT *mcc) {

  MOB_SCRIPT_LINE *mscrl;

  if (mscr == NULL) {
    return;
  }

  mscrl = mscr->lines;

  while (mscrl != NULL) {
    enqueue_script_line(mob, mscrl, mcc);

    mscrl = mscrl->next;
  }

  return;
}

/* Locate a specific script... */

MOB_SCRIPT *find_script(CHAR_DATA *mob, int id) {

  if ( mob == NULL ) {
    return NULL;
  }

  return find_script2(mob->triggers->scripts, id);
}

/* Locate a specific script... */

MOB_SCRIPT *find_script2(MOB_SCRIPT *scr, int id) {

  MOB_SCRIPT *mscr;

  mscr = scr;

  while ( mscr != NULL
       && mscr->id != id ) {
    mscr = mscr->next;
  } 

  return mscr;
}

/* Save a script to disk... */

void save_script(FILE *fp, MOB_SCRIPT *scr) {

  if (scr == NULL) {
    return;
  }

  if ( scr->lines != NULL
    && scr->name != NULL) {

    fprintf(fp, "Script %d '%s'\n",
                 scr->id,
                 scr->name);

    save_script_line(fp, scr->lines);
  }
 
  if (scr->next != NULL) {
    save_script(fp, scr->next);
  } 

  return;
}

/* Script Lines */

static MOB_SCRIPT_LINE *free_mscrls = NULL;

/* Get and free MSCRLs... */

MOB_SCRIPT_LINE *get_mscrl() {

  MOB_SCRIPT_LINE *mscrl;

  if (free_mscrls == NULL) { 
    mscrl = (MOB_SCRIPT_LINE *) alloc_perm(sizeof(*mscrl));
  } else {
    mscrl = free_mscrls;
    free_mscrls = mscrl->next;
  }

  mscrl->seq 	= 0;
  mscrl->delay	= 0;
  mscrl->cmd_id	= 0;
  mscrl->cmd 	= NULL;
  mscrl->next 	= NULL;

  return mscrl;
}

void free_mscrl(MOB_SCRIPT_LINE *mscrl) {

  if (mscrl == NULL) {
    return;
  }

  if (mscrl->cmd != NULL) {
    free_string(mscrl->cmd);
  }

  mscrl->cmd = NULL;
 
  mscrl->next = free_mscrls;
  free_mscrls = mscrl;

  return;
}

void release_script_lines(MOB_SCRIPT *mscr) {

  MOB_SCRIPT_LINE *mscrl, *next_mscrl;

 /* Nothing to do? */

  if (mscr->lines == NULL) {
    return;
  }

 /* Ok, let 'em go... */ 

  next_mscrl = NULL;
  mscrl = mscr->lines;

  while (mscrl != NULL) {
    next_mscrl = mscrl->next;

    free_mscrl(mscrl);

    mscrl = next_mscrl;
  }

  mscr->lines = NULL;
 
 /* All done... */

  return;
}  

/* Enqueue a script line to a mob... */

void inline enqueue_script_line(CHAR_DATA 	*mob, 
				MOB_SCRIPT_LINE *scrl,
                                MOB_CMD_CONTEXT *mcc) {

  char buf[MAX_STRING_LENGTH];

  if (scrl == NULL) {
    return;
  }

  expand_by_context(scrl->cmd, mcc, buf);

  enqueue_mob_cmd(mob, buf, scrl->delay, scrl->cmd_id);

  return;
}

/* Locate a specific script line... */

MOB_SCRIPT_LINE *find_script_line(MOB_SCRIPT *mscr, int seq) {

  MOB_SCRIPT_LINE *mscrl;

  if (mscr == NULL) {
    return NULL;
  }

  mscrl = mscr->lines;

  while ( mscrl != NULL
       && mscrl->seq != seq ) {
    mscrl = mscrl->next;
  } 

  return mscrl;
}

/* Save a script line to disk... */

void save_script_line(FILE *fp, MOB_SCRIPT_LINE *scrl) {

  if (scrl == NULL) {
    return;
  }

  if ( scrl->cmd != NULL) { 

    fprintf(fp, "ScriptLine %d %d %d %s~\n",
                 scrl->seq,
                 scrl->delay,
                 scrl->cmd_id,
                 scrl->cmd);
  }
 
  if (scrl->next != NULL) {
    save_script_line(fp, scrl->next);
  } 

  return;
}

/* Insert a line into a script... */

void insert_script_line(MOB_SCRIPT *scr, MOB_SCRIPT_LINE *scrl) {

  MOB_SCRIPT_LINE *old_scrl;

 /* Simple case first... */

  if (scr->lines == NULL) {
    scr->lines = scrl;
    return;
  }

 /* Search for conversation record... */

  old_scrl = scr->lines;

 /* Check for insert before first record... */ 

  if (old_scrl->seq > scrl->seq) {

    scr->lines = scrl;
    scrl->next = old_scrl;

    return;
  }

 /* Check for insert after a record... */

  while ( old_scrl->next != NULL
       && old_scrl->next->seq < scrl->seq) {
    old_scrl = old_scrl->next;
  }

 /* Ok, found where to insert... */

  scrl->next = old_scrl->next;
  old_scrl->next = scrl;

 /* All done... */

  return;
}

/* Mob command contexts... */

static MOB_CMD_CONTEXT *free_mcc_chain = NULL;

/* Set up the mob_cc */

MOB_CMD_CONTEXT *get_mcc(	CHAR_DATA 	*mob, 
				CHAR_DATA 	*actor, 
				CHAR_DATA 	*victim, 
				CHAR_DATA 	*random, 
				OBJ_DATA 	*obj,
				OBJ_DATA 	*obj2,
			 	int 		 number, 
				char 		*text) {

  MOB_CMD_CONTEXT *mcc;

 /* Get a mob_cc if we don't have one already... */

  if (free_mcc_chain == NULL) {
    mcc = (MOB_CMD_CONTEXT *) alloc_perm(sizeof(*mcc));
  } else {
    mcc = free_mcc_chain;
    free_mcc_chain = mcc->next;
  }

  mcc->mob 	= mob;
  mcc->actor	= actor;
  mcc->victim	= victim;
  mcc->random	= random;
  mcc->obj	= obj;
  mcc->obj2	= obj2;
  mcc->number	= number;

//  if (text != NULL) {
//    mcc->text	= str_dup(text);
//  } else {
//    mcc->text	= NULL;
//  }

  mcc->text	= text;

  mcc->next	= NULL;
  mcc->wev	= NULL;
  mcc->room	= NULL;

  return mcc;
}

/* Find a random character... */

CHAR_DATA *random_char(CHAR_DATA *mob) { 

  int count = 0;
  int index = 0;
  CHAR_DATA *vch;
  CHAR_DATA *rch;

 /* Is it possible to find a random mob? */

  if ( mob == NULL
    || mob->in_room == NULL) {
    return NULL;
  }

 /* How many people are in the room? */

  vch = mob->in_room->people; 

  while (vch != NULL) {

    if (  vch != mob
      &&  can_see( mob, vch ) ) {
       count++;
      }

    vch = vch->next_in_room;
  }

 /* Pick one at random... */ 
  
  rch = NULL;

  index = number_range( 1, count );
  count = 0; 

  vch = mob->in_room->people; 

  while (vch != NULL) {

    if (  vch != mob
      && can_see( mob, vch ) ) {

       if (count++ == index) {
         rch = vch;
         break; 
      }
    }

    vch = vch->next_in_room;
  }
  
 /* Add done... */

  return rch;
}

void free_mcc(MOB_CMD_CONTEXT *mcc) {

  if (mcc != NULL) {

    mcc->next = free_mcc_chain;
    free_mcc_chain = mcc;

    mcc->mob 	= NULL;
    mcc->actor	= NULL;
    mcc->victim	= NULL;
    mcc->random	= NULL;
    mcc->obj	= NULL;
    mcc->obj2	= NULL;
    mcc->number	= 0;
    mcc->text	= NULL;
    mcc->wev	= NULL;
    mcc->room	= NULL;
  }

  return;
}

/* Garble a spoken text string... */

struct gbl_type {
 	char *	old_text;
	char *	new_text;
};

typedef struct gbl_type GBL_TYPE;

static GBL_TYPE speech_table[] = {
	{ " ",		" "			},
	{ "ar",		"eir"			},
	{ "au",		"ow"			},
	{ "ble",	"ala"			},
        { "ss",         "ff"		        },
	{ "blind",	"schwartz"		},
	{ "bur",	"huir"			},
	{ "cu",		"koo"			},
	{ "de",		"th"			},
	{ "en",		"ein"			},
	{ "exor",	"ekyor"			},
	{ "light",	"lict"			},
        { "fire",       "fervayo"	        },
        { "flame",      "flambee"	        },
	{ "lo",		"glu"			},
	{ "mor",	"vur"			},
	{ "move",	"baume"			},
	{ "ness",	"vitch"			},
	{ "ning",	"chow"			},
	{ "per",	"ad"			},
	{ "ra",		"zunne"			},
	{ "fresh",	"britch"		},
	{ "re",		"sion"			},
	{ "son",	"tor"			},
	{ "pro",	"zu"			},
	{ "tri",	"dri"			},
	{ "ven",	"kind"			},
        { "heal",       "fuss"			},   
        { "ma",         "mama"	                }, 
        { "ha",         "lah"	                },
        { "on",         "unh"                   },
	{ "sh",         "sch"                   },
        { "ed",         "z"                     },
	{ "1",		"2"			},
	{ "3",		"8"			},
	{ "5",		"9"			},
	{ "8",		"3"			},
	{ "9",		"6"			},
	{ "a", "e" }, { "b", "p" }, { "c", "k" }, { "d", "t" },
	{ "e", "i" }, { "f", "v" }, { "g", "h" }, { "h", "g" },
	{ "i", "y" }, { "j", "y" }, { "k", "c" }, { "l", "r" },
	{ "m", "n" }, { "n", "m" }, { "o", "y" }, { "p", "b" },
	{ "q", "k" }, { "r", "a" }, { "s", "f" }, { "t", "d" },
	{ "u", "e" }, { "v", "w" }, { "w", "o" }, { "x", "x" },
	{ "y", "j" }, { "z", "ts" },
	{ "", "" }
};

static GBL_TYPE spell_table[] = {

	{ " ",		" "			},
	{ "!",		"!"			},
	{ "cthulhu",	"K'thul'hu"		},
	{ "azathoth",	"aza-thoth"		},
	{ "ia",		"ia"			},
	{ "ar",		"cth"			},
	{ "ar",		"cth"			},
	{ "au",		"ulhu "			},
	{ "ble",	"ia ia ia"		},
        { "ss",         "-yog'oth"              },
	{ "blind",	"niger-"		},
	{ "bur",	"shub"			},
	{ "cu",		"alde'bra-on"		},
	{ "de",		"krono-"		},
	{ "en",		"tel-iki"		},
	{ "exor",	"divinus"		},
	{ "light",	"kath'ga"		},
        { "fire",       "azar-th'th"            },
        { "flame",      "cth-ougaaah-"          },
	{ "lo",		"form'l't"		},
	{ "mor",	"-za'ak"		},
	{ "move",	"azrad "		},
	{ "ness",	"f'nagl"		},
	{ "ning",	"-r'yliii"		},
	{ "per",	"sha'no"		},
	{ "ra",		"zo'th"			},
	{ "fresh",	"ick"			},
	{ "re",		"-kom'men"		},
	{ "son",	"ghe-"			},
	{ "pro",	"al'"			},
	{ "tri",	"cu'la"			},
	{ "ven",	"nek'ro-"		},
        { "heal",       "go'ra go'ra gharn"     },   
        { "ma",         "si-rus"                }, 
        { "ha",         "ha-stur "              },
        { "on",         "-ri-"                  },
	{ "a", "e" }, { "b", "g" }, { "c", "k" }, { "d", "e" },
	{ "e", "i" }, { "f", "s" }, { "g", "h" }, { "h", "n" },
	{ "i", "a" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
	{ "m", "p" }, { "n", "c" }, { "o", "y" }, { "p", "s" },
	{ "q", "d" }, { "r", "f" }, { "s", "r" }, { "t", "g" },
	{ "u", "e" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
	{ "y", "i" }, { "z", "k" },
	{ "", "" }
};

static GBL_TYPE drunk_table[] = {

	{ " ",		" "			},
	{ "!",		"!"			},
	{ "?",		"?"			},
	{ ".",		"?"			},
	{ ",",		"!"			},
	{ "1",		"0"			},
	{ "3",		"5"			},
	{ "5",		"4"			},
	{ "8",		"2"			},
	{ "9",		"6"			},
	{ "sh",		"sshee"			},
	{ "you",	"jimmi"			},
	{ "can",	"karn"			},
	{ "ple",	"noew"			},
	{ "per",	"*burp*"		},
	{ "ent",	"chent"			},
	{ "wh",		"weur"			},
	{ "a", "aa" }, { "b", "b" }, { "c", "ch" }, { "d", "t" },
	{ "e", "ee" }, { "f", "f" }, { "g", "k" }, { "h", "h" },
	{ "i", "i" }, { "j", "y" }, { "k", "k" }, { "l", "ll" },
	{ "m", "n" }, { "n", "m" }, { "o", "oo" }, { "p", "p" },
	{ "q", "c" }, { "r", "a" }, { "s", "sh" }, { "t", "d" },
	{ "u", "oo" }, { "v", "f" }, { "w", "v" }, { "x", "k" },
	{ "y", "why" }, { "z", "se" },
	{ "", "" }
};

static GBL_TYPE olde_table[] =
    {
	{ " ",		" "		},
	{ "have",	"hath"		},
	{ "hello",	"hail"		},
	{ "hi ",	"hail "		},
	{ " hi",	" hail"		},
	{ "are",	"art"		},
	{ "your",	"thy"		},
	{ "you",	"thou"		},
	{ "i think",	"methinks"	},
	{ "do ",	"doth "		},
	{ " do",	" doth"		},
	{ "it was",	"twas"		},
	{ "before",	"ere"		},
	{ "his",	"$s"		},
	{ "old",	"olde"		},
	{ "magic", "magick"},
	{ "a", "a" }, { "b", "b" }, { "c", "c" }, { "d", "d" },
	{ "e", "e" }, { "f", "f" }, { "g", "g" }, { "h", "h" },
	{ "i", "i" }, { "j", "j" }, { "k", "k" }, { "l", "l" },
	{ "m", "m" }, { "n", "n" }, { "o", "o" }, { "p", "p" },
	{ "q", "q" }, { "r", "r" }, { "s", "s" }, { "t", "t" },
	{ "u", "u" }, { "v", "v" }, { "w", "w" }, { "x", "x" },
	{ "y", "y" }, { "z", "z" }, {  "",  "" }
    };

static GBL_TYPE latin_table[] =
    {
	{ " ",		" "		},
	{ "hello",	"ave"		},
	{ "hi ",	"ave "		},
	{ " hi",	" ave"		},
	{ "bye",	"vale"		},
	{ "a", "a" }, { "b", "b" }, { "c", "c" }, { "d", "d" },
	{ "e", "e" }, { "f", "f" }, { "g", "g" }, { "h", "h" },
	{ "i", "i" }, { "j", "i" }, { "k", "c" }, { "l", "l" },
	{ "m", "m" }, { "n", "n" }, { "o", "o" }, { "p", "p" },
	{ "q", "q" }, { "r", "r" }, { "s", "s" }, { "t", "t" },
	{ "u", "v" }, { "v", "v" }, { "w", "v" }, { "x", "x" },
	{ "y", "y" }, { "z", "z" }, {  "",  "" }
    };

char * langstyle( CHAR_DATA *ch, char *argument, GBL_TYPE *table ) {
    char buf  [MAX_STRING_LENGTH];
    char *pName;
    int iSyl;
    int length;
     
   buf[0]	= '\0';
 
    if ( argument[0] == '\0' ) return argument;

    for ( pName = str_dup(argument); *pName != '\0'; pName += length )    {
	for ( iSyl = 0; (length = strlen(table[iSyl].old_text)) != 0; iSyl++ ) 	{
	    if ( !str_prefix(table[iSyl].old_text, pName ) )    {
		strcat( buf, table[iSyl].new_text );
		break;
	    }
	}

	if ( length == 0 )  length = 1;
    }

    argument[0] = '\0';
    strcpy(argument,buf);
    argument[0] = UPPER(argument[0]);
    return argument;
}

  
void garble_text(MOB_CMD_CONTEXT *mcc, char *buf, GBL_TYPE *table) {
    char *pName;
    int iSyl;
    int length; 
    int skl;

   /* Initialize the output buffer... */

    buf[0] = '\0';

   /* Work out comparative skills... */
 
    skl = UMIN( get_skill(mcc->actor, mcc->number), get_skill(mcc->mob, mcc->number) );

   /* If both over 50, communication is undisturbed... */

    if (mcc->number == get_skill_sn("old english")) mcc->text = langstyle(mcc->actor, mcc->text, olde_table);
    if (mcc->number == get_skill_sn("latin")) mcc->text = langstyle(mcc->actor, mcc->text, latin_table);

    if (skl > 50) {
      strcpy(buf, mcc->text);
      return;
    }

   /* Garble time... */

    pName = mcc->text;

    while (*pName != '\0') {

      if ( *pName == '{' ) {

        pName += 1;

        if ( *pName != '\0' ) {
          pName += 1;
        } 

      }

      if ( *pName != '\0'
        && *pName != '{' ) {
        for ( iSyl = 0;
             (length = strlen(table[iSyl].old_text)) != 0;
             iSyl++ ) {

          if ( !str_prefix( table[iSyl].old_text, pName ) ) {
            if (number_bits(6) > skl) {
              strcat( buf, table[iSyl].new_text );
            } else {
              strcat( buf, table[iSyl].old_text );
            } 
            break;
          }
        }

        if ( length == 0 ) {
          strncat(buf, pName, 1); 
          length = 1;
        }

        pName += length;
      } 
    }

   /* All done... */

    return;
}

void garble_text_drunk(MOB_CMD_CONTEXT *mcc, char *buf, GBL_TYPE *table) {

    char *pName;
    int iSyl;
    int length; 
    int skl;

   /* Initialize the output buffer... */

    buf[0] = '\0';

   /* Work out comparative skills... */
 
    skl = mcc->actor->condition[COND_DRUNK]; 

   /* If sober, communication is undisturbed... */

    if (skl < COND_DRUNK2) {
      strcpy(buf, mcc->text);
      return;
    }

   /* Garble time... */

    skl = 30 - skl;

    pName = mcc->text;

    while (*pName != '\0') {

      if ( *pName == '{' ) {

        pName += 1;

        if ( *pName != '\0' ) {
          pName += 1;
        } 

      }

      if ( *pName != '\0'
        && *pName != '{' ) {
        for ( iSyl = 0; (length = strlen(table[iSyl].old_text)) != 0; iSyl++ ) {

          if ( !str_prefix( table[iSyl].old_text, pName ) ) {
            if (number_bits(6) > skl) {
              strcat( buf, table[iSyl].new_text );
            } else {
              strcat( buf, table[iSyl].old_text );
            } 
            break;
          }
        }

        if ( length == 0 ) {
          strncat(buf, pName, 1); 
          length = 1;
        }

        pName += length;
      } 
    }

   /* All done... */

    return;
}


/* Edit a string in context... */

static char *he_she        [] = { "it",     "he",  "she"   };
static char *him_her       [] = { "it",     "him", "her"   };
static char *his_her       [] = { "its",    "his", "her"   };
static char *man_woman     [] = { "thing",  "man", "woman" }; 
static char *sir_lady      [] = { "Thing",  "Sir", "Lady"  }; 
static char *lad_lass      [] = { "Thingy", "Lad", "Lass"  }; 
static char *sir_madam     [] = { "Thing",  "Sir", "Madam"  }; 
static char *invis	   [] = { "something", "somebody", "someone" };


void expand_at_code(char *ch1, char *ch2, MOB_CMD_CONTEXT *mcc, char *t) {
ROOM_INDEX_DATA *room;
CHAR_DATA *mob;
OBJ_DATA *obj;
char *text;
bool num_found;
bool memory;
CHAR_DATA *obs;
int slot;
bool global = FALSE;

  char buf[MAX_STRING_LENGTH];

 /* What are we looking up? */

  obs = mcc->mob;

  room = NULL;
  mob = NULL;
  obj = NULL;
  text = NULL;
  num_found = FALSE;
  memory = FALSE;

  switch (*ch1) {

    case 'a':
      mob = mcc->actor;
      break;

    case 'c':
      mob = mcc->random;
      break;

    case 'g':
      global = TRUE;
      break;

    case 'm':
      mob = mcc->mob;
      memory = TRUE;
      break;

    case 'n':
      num_found = TRUE;
      break;

    case 'o':
      mob = mcc->mob;
      break;

    case 'p':
      obj = mcc->obj;
      break;

    case 'r':
      room = mcc->room;
      break; 

    case 's':
      obj = mcc->obj2;
      break;

    case 't':
      text = mcc->text;
      break;

    case 'v':
      mob = mcc->victim;
      break;

    case '@':
      strcpy( t, "@" );
      return;

    default:
      break;
  }     

 /* Check we found something... */

  if ( mob == NULL
    && obj == NULL
    && text == NULL
    && room == NULL
    && !memory
    && !num_found
    && !global) {
    sprintf(t, "*@%c%c*", *ch1, *ch2);
    return;
  }

 /* Now go find the substitution string... */

  *t = '\0';

 /* Memory */
 
  if (memory) {
    switch (*ch2) {
       
      case '0':
        slot = 0;
        break;

      case '1':
        slot = 1;
        break;

      case '2':
        slot = 2;
        break;

      case '3':
        slot = 3;
        break;

      case '4':
        slot = 4;
        break;

      case '5':
        slot = 5;
        break;

      case '6':
        slot = 6;
        break;

      case '7':
        slot = 7;
        break;

      case '8':
        slot = 8;
        break;

      case '9':
        slot = 9;
        break;

      default:
        sprintf(t, "*@%c%c*", *ch1, *ch2);
        return;
    }

    if ( slot < 0 
      || slot >= MOB_MEMORY ) { 
      sprintf(t, "*@%c%d*", *ch1, slot);
      return;
    }

    if (mcc->mob->memory[slot] != NULL) {
      sprintf(t, "%s", mcc->mob->memory[slot]);
    } else {
      sprintf(t, "*@%c%d - empty*", *ch1, slot);
    }

    return; 
  }

  if (global) {
    char buf[MAX_INPUT_LENGTH];

    switch (*ch2) {
       
      case '0':
        strcpy(t, mud.name);
        break;

      case '1':
        strcpy(t, mud.url);
        break;

      case '2':
        sprintf(buf, "%d", mud.port);
        strcpy(t, buf);
        break;

      default:
        sprintf(t, "*@%c%c*", *ch1, *ch2);
        return;
    }
    return;
  }


 /* Characters... */

  if (mob != NULL) {

    if ( obs != NULL
      && *ch2 != 'r'
      && *ch2 != 't'
      && *ch2 != 'l'
      && !can_see(obs, mob) ) {

      if ( *ch2 == '0' ) {
        strcpy(t, "");
      } else {
        strcpy(t, invis[mob->sex]);
      } 
    } else {

      switch (*ch2) {
       
        case '0':
          switch ( *(mob->name) ) {

            case 'a': 
            case 'e': 
            case 'i':
            case 'o': 
            case 'u': 
              strcpy( t, "an" );
              break;

            default:
              strcpy( t, "a" );
              break;
          }

          break;

        case '1':
          one_argument( mob->name, t);
          break;

        case '2':
          strcpy(t, mob->short_descr);
          break;

        case '3':
          strcpy(t, his_her[mob->sex]);
          break;

        case '4':
          strcpy(t, him_her[mob->sex]);
          break;

        case '5': 
          strcpy(t, he_she[mob->sex]);
          break;

        case '6': 
          strcpy(t, man_woman[mob->sex]);
          break;

        case '7': 
          strcpy(t, sir_lady[mob->sex]);
          break;

        case '8': 
          strcpy(t, lad_lass[mob->sex]);
          break;

        case '9': 
          strcpy(t, sir_madam[mob->sex]);
          break;

        case 'r': 
          if (mob->in_room == NULL) {
            strcpy(t, "0");
          } else { 
            sprintf(t, "%ld", mob->in_room->vnum);
          }
          break;

        case 'd': 
          if (!mob->waking_room) {
              if (!mob->in_room) {
                  strcpy(t, "0");
              } else { 
                  sprintf(t, "%ld", mob->in_room->vnum);
              }

          } else { 
            sprintf(t, "%ld", mob->waking_room->vnum);
          }
          break;

        case 't':
          strcpy(t, mob->true_name);
          break;

        case 'l':
          sprintf(t, "%d", mob->level);
          break;

        default:
          sprintf(t, "*@%c%c*", *ch1, *ch2);
          return;
      }
    }
  } 

 /* Objects... */

  if (obj != NULL) {

    switch (*ch2) {
       
      case '0':
        switch ( *(obj->name) ) {

          case 'a': 
          case 'e': 
          case 'i':
          case 'o': 
          case 'u': 
            strcpy( t, "an" );
            break;

          default:
            strcpy( t, "a" );
            break;
        }

        break;

      case '1':
        one_argument( obj->name, t);
        break;

      case '2':
        strcpy(t, obj->short_descr);
        break;

      case 'l':
        sprintf(t, "%d", obj->level);
        break;

      default:
        sprintf(t, "*@%c%c*", *ch1, *ch2);
        return;
    }
  }

 /* Room... */

  if (room != NULL) {
     
    switch (*ch2) {
       
      case '0':
        sprintf(t, "%ld", room->vnum);
        break;

      case '1':
        strcpy(t, room->name);
        break;

      default:
        sprintf(t, "*@%c%c*", *ch1, *ch2);
        return;
    }
  }

 /* Text... */

  if (text != NULL) {
     
    switch (*ch2) {
       
      case '0':
        strcpy(t, text);
        break;

      case '1':
        if ( mcc->actor->condition[COND_DRUNK] > COND_DRUNK2 ) {
          garble_text_drunk(mcc, buf, drunk_table);
        } else {
          garble_text(mcc, buf, speech_table);
        }
        sprintf(t, "%s", buf);
        break;

      case '2':
        garble_text(mcc, buf, spell_table);
        sprintf(t, "%s", buf);
        break;

      default:
        sprintf(t, "*@%c%c*", *ch1, *ch2);
        return;
    }
  }

 /* Number... */

  if (num_found) {
    
    switch (*ch2) {
       
      case '0':
        sprintf(t, "%d", mcc->number);
        break;

      default:
        sprintf(t, "*@%c%c*", *ch1, *ch2);
        return;
    }
  }

  return;
}

/* Lifted from action by way of mob_prog... */

void expand_by_context(char *cmd, MOB_CMD_CONTEXT *mcc, char *res_cmd) {
char tmp[ MAX_INPUT_LENGTH ];
char *str;
char *i;
char *point;

  point   = res_cmd;
  str     = cmd;

  while ( *str != '\0' ) {
  
    if ( *str != '@' ) {
      *point++ = *str++;
    } else {
      str++;
      expand_at_code(str, str+1, mcc, tmp);
      i = tmp;
      ++str; 
      if (*str != '\0') {
        ++str;
        while ( ( *point = *i ) != '\0' ) {
          ++point, ++i;
        }
      }
    }
  }

  *point = '\0';

 /* Uppercase first letter, allowing for optional leading {_ */

  if (res_cmd != NULL) {
    if (res_cmd[0] != '{') {
      res_cmd[0] = UPPER(res_cmd[0]);
    } else {
      res_cmd[2] = UPPER(res_cmd[2]);
    }
  }

  return;
}


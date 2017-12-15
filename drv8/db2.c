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
#include "db.h"
#include "skill.h"
#include "spell.h"
#include "econd.h"
#include "conv.h"
#include "mob.h"
#include "deeds.h"
#include "triggers.h"
#include "monitor.h"
#include "anchor.h"
#include "chat.h"
#include "race.h"
#include "olc.h"

RUMOR_DATA rumor_array[MAX_RUMOR];

/*
 * MOBprogram locals
*/

int         mprog_name_to_type  	(char* name);
MPROG_DATA *    mprog_file_read     	(char* f, MPROG_DATA* mprg, MOB_INDEX_DATA *pMobIndex);
void        load_mobprogs           	(FILE* fp );
void        mprog_read_programs     	(FILE* fp, MOB_INDEX_DATA *pMobIndex);
void	load_mob_skills		(FILE* fp, MOB_INDEX_DATA *pMobIndex);
void	load_mob_extended	(FILE* fp, MOB_INDEX_DATA *pMobIndex);
void        save_social 		(const struct social_type *s, FILE *fp);
void        save_socials		(void);

/* values for db2.c */
struct 					social_type	social_table		[MAX_SOCIALS];
int						social_count		= 0;

/* snarf a socials file */
void load_socials() {
    FILE *fp;
    struct social_type social;
    char *temp;
    char arg[MAX_STRING_LENGTH];

   /* Open the socials file... */

    fp = fopen( SOC_FILE, "r" );

    if (fp == NULL) {
      bug("Unable to open socials file '" SOC_FILE "'", 0);
      exit(1);
    }   

   /* Process the socials... */

    for ( ; ; ) {

       /* clear social */
	social.char_no_arg = NULL;
	social.others_no_arg = NULL;
	social.char_found = NULL;
	social.others_found = NULL;
	social.vict_found = NULL; 
	social.char_not_found = NULL;
	social.char_auto = NULL;
	social.others_auto = NULL;
                social.s_style = 0;

       /* Read either name of next or #0 for end of file... */ 

    	temp = fread_word(fp);

    	if (!str_cmp(temp,"#0")) {
                     fclose(fp);
                     log_string("...loaded"); 
	     return;  /* done */
        	}

       /* Ok, it was a name, so skip the rest of the line... */ 

    	strcpy(social.name,temp);
    	fread_to_eol(fp);

       /* Read chars message for doing with no args... */

	temp = fread_string_eol(fp);

                    if (temp[0] == '>') {
                        temp=one_argument(temp, arg);
                        temp=one_argument(temp, arg);
                        social.s_style = atoi(arg);
               	        temp = fread_string_eol(fp);
                    }

                if (!str_cmp(temp,"$")) {
	     social.char_no_arg = NULL;
 	} else if (!str_cmp(temp,"#")) {
	     social_table[social_count] = social;
	     social_count++;
	     continue; 
	} else {
                     social.char_no_arg = temp;
       	}

       /* Read others message for doing with no args... */

        temp = fread_string_eol(fp);
        if (!str_cmp(temp,"$")) {
             social.others_no_arg = NULL;
        } else if (!str_cmp(temp,"#")) {
	     social_table[social_count] = social;
             social_count++;
             continue;
        } else {
	    social.others_no_arg = temp;
        }

       /* Read chars message for doing to others... */

        temp = fread_string_eol(fp);
        if (!str_cmp(temp,"$")) {
             social.char_found = NULL;
        } else if (!str_cmp(temp,"#")) {
	     social_table[social_count] = social;
             social_count++;
             continue;
        } else {
	    social.char_found = temp;
        }

       /* Read others message for doing to others... */

        temp = fread_string_eol(fp);
        if (!str_cmp(temp,"$")) {
             social.others_found = NULL;
        } else if (!str_cmp(temp,"#")) {
	     social_table[social_count] = social;
             social_count++;
             continue;
        } else {
	    social.others_found = temp; 
        }

       /* Read victims message for doing to others... */

        temp = fread_string_eol(fp);
        if (!str_cmp(temp,"$")) {
             social.vict_found = NULL;
        } else if (!str_cmp(temp,"#")) {
	     social_table[social_count] = social;
             social_count++;
             continue;
        } else {
	    social.vict_found = temp;
        } 

       /* Read chars message for doing with absent (unused)... */

        temp = fread_string_eol(fp);
        if (!str_cmp(temp,"$")) {
             social.char_not_found = NULL;
        } else if (!str_cmp(temp,"#")) {
	     social_table[social_count] = social;
             social_count++;
             continue;
        } else {
	    social.char_not_found = temp;
        }

       /* Read chars message for doing with self... */

        temp = fread_string_eol(fp);
        if (!str_cmp(temp,"$")) {
             social.char_auto = NULL;
        } else if (!str_cmp(temp,"#")) {
	     social_table[social_count] = social;
             social_count++;
             continue;
        } else {
	    social.char_auto = temp;
        } 
         
       /* Read others message for doing with self... */

        temp = fread_string_eol(fp);
        if (!str_cmp(temp,"$")) {
             social.others_auto = NULL;
        } else if (!str_cmp(temp,"#")) {
             social_table[social_count] = social;
             social_count++;
             continue;
        } else {
	    social.others_auto = temp; 
        }
	
       /* Increment and back for the next... */

	social_table[social_count] = social;
    	social_count++;
   }

   fclose(fp);
}


void load_mobiles( FILE *fp ) {
MOB_INDEX_DATA *pMobIndex;
long vnum;
char letter,temp;
int iHash;
char *word;
long vector;

    if ( !area_last ) {
        bug( "Load_mobiles: no #AREA seen yet.", 0 );
        exit( 1 );
    } 

    for ( ; ; ) {
 
        letter                          = fread_letter( fp );
        if ( letter != '#' )  {
            bug( "Load_mobiles: # not found.", 0 );
            exit( 1 );
        }
 
        vnum                            = fread_number_long( fp );
        if ( vnum == 0 ) break;

        fBootDb = FALSE;
        if ( get_mob_index( vnum ) != NULL ) {
            bug( "Load_mobiles: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;
 
        pMobIndex                       = (MOB_INDEX_DATA *) alloc_perm( sizeof(*pMobIndex) );
        pMobIndex->vnum                 = vnum;
        pMobIndex->area                 = area_last;
	pMobIndex->new_format		= TRUE;
	newmobs++;
        pMobIndex->player_name        	= fread_string( fp );
        pMobIndex->short_descr          	= fread_string( fp );
        pMobIndex->long_descr           	= fread_string( fp );
        pMobIndex->description          	= fread_string( fp );
        pMobIndex->race		= race_lookup(fread_string( fp ));
 
        pMobIndex->long_descr[0]      	= UPPER(pMobIndex->long_descr[0]);
        pMobIndex->description[0]       = UPPER(pMobIndex->description[0]);
 
        pMobIndex->act                  	= fread_flag_long( fp ) | ACT_IS_NPC | race_array[pMobIndex->race].act;
        pMobIndex->affected_by          	= fread_flag_long( fp ) | race_array[pMobIndex->race].aff;
        pMobIndex->pShop                	= NULL;
        pMobIndex->alignment            	= fread_number( fp );

	switch (area_last->version) {
	    case VERSION_ROM:
		letter                  = 'S';
		pMobIndex->group        = fread_number(fp);
                break;

            case VERSION_CTHULHU_0:
        	letter                  = fread_letter( fp );
	pMobIndex->group        = 0;
                break;

            default:
                letter                  = 'S';
	pMobIndex->group        = fread_number(fp);
                break;
        }

        pMobIndex->level                = fread_number( fp );
        pMobIndex->hitroll              = fread_number( fp );  

	/* read hit dice */
        pMobIndex->hit[DICE_NUMBER]     = fread_number( fp );  
        /* 'd'          */                fread_letter( fp ); 
        pMobIndex->hit[DICE_TYPE]   	= fread_number( fp );
        /* '+'          */                fread_letter( fp );   
        pMobIndex->hit[DICE_BONUS]      = fread_number( fp ); 

 	/* read mana dice */
	pMobIndex->mana[DICE_NUMBER]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->mana[DICE_TYPE]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->mana[DICE_BONUS]	= fread_number( fp );

	/* read damage dice */
	pMobIndex->damage[DICE_NUMBER]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->damage[DICE_TYPE]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->damage[DICE_BONUS]	= fread_number( fp );

	if ( area_last->version < VERSION_CTHULHU_1 ) {
 
            if ( area_last->version == VERSION_CTHULHU_0 ) { 
	pMobIndex->atk_type		= fread_number( fp );
            } else {
     	pMobIndex->atk_type		= attack_type_lookup(fread_word( fp ));
            }

            if ( pMobIndex->atk_type >= 0 
            && pMobIndex->atk_type < MAX_ATTACK_TYPES ) {
                pMobIndex->dam_type = attack_table[pMobIndex->atk_type].damage;
            } else {
                pMobIndex->atk_type = WDT_PUNCH;
                pMobIndex->dam_type = DAM_BASH;
            }
        } else {
	    pMobIndex->atk_type		= attack_type_lookup(fread_word( fp ));
	    pMobIndex->dam_type		= dam_type_lookup(fread_word( fp ));
        }

	/* read armor class */
	pMobIndex->ac[AC_PIERCE]	= fread_number( fp ) * 10;
	pMobIndex->ac[AC_BASH]	= fread_number( fp ) * 10;
	pMobIndex->ac[AC_SLASH]	= fread_number( fp ) * 10;
	pMobIndex->ac[AC_EXOTIC]	= fread_number( fp ) * 10;

	/* read flags and add in data from the race table */
	pMobIndex->off_flags		= fread_flag( fp )	| race_array[pMobIndex->race].off;
	pMobIndex->imm_flags		= fread_flag( fp )	| race_array[pMobIndex->race].imm;
	pMobIndex->res_flags		= fread_flag( fp )	| race_array[pMobIndex->race].res;
	pMobIndex->vuln_flags		= fread_flag( fp )	| race_array[pMobIndex->race].vuln;

	/* vital statistics */

        if (area_last->version == VERSION_CTHULHU_0) {
  	  pMobIndex->start_pos			= fread_number( fp );
	  pMobIndex->default_pos			= fread_number( fp );
	  pMobIndex->sex			= fread_number( fp );
        } else {
  	  pMobIndex->start_pos			= position_lookup(fread_word( fp ));
	  pMobIndex->default_pos			= position_lookup(fread_word( fp ));
	  pMobIndex->sex			= sex_lookup(fread_word( fp ));
        }

	pMobIndex->gold			= fread_number( fp );

	pMobIndex->form			= fread_flag( fp )	| race_array[pMobIndex->race].form;
	pMobIndex->parts			= fread_flag( fp )	| race_array[pMobIndex->race].parts;

        if (area_last->version == VERSION_CTHULHU_0) {
	    /* size */
   	    temp				= fread_letter( fp );
	    switch (temp)  {
	        case ('T') :		pMobIndex->size = SIZE_TINY;	break;
	        case ('S') :		pMobIndex->size = SIZE_SMALL;	break;
	        case ('M') :		pMobIndex->size = SIZE_MEDIUM;	break;
	        case ('L') :		pMobIndex->size = SIZE_LARGE; 	break;
	        case ('H') :		pMobIndex->size = SIZE_HUGE;	break;
	        case ('G') :		pMobIndex->size = SIZE_GIANT;	break;
	        default:		pMobIndex->size = SIZE_MEDIUM; break;
   	    }
        } else {
	    pMobIndex->size = size_lookup(fread_word(fp));
	}

	pMobIndex->material		= material_lookup(fread_word( fp ));
 
       /* Check it's a Simple type of MOB... */

        if ( letter != 'S' )  {
            bug( "Load_mobiles: vnum %d non-S.", vnum );
            exit( 1 );
        }

       /* By default, MOBs are unskilled... */

        pMobIndex->skills = NULL;

       /* They also have no skills and triggers... */

        pMobIndex->triggers = get_trs();

       /* Yuck. Peek at next char to see what follows... */

        letter = fread_letter( fp );

        while ( letter == 'F' ) {

            word                        	= fread_word(fp);
            vector		= fread_flag(fp);

	    if (!str_prefix(word,"act"))   	REMOVE_BIT(pMobIndex->act,vector);
            	    else if (!str_prefix(word,"aff"))      	REMOVE_BIT(pMobIndex->affected_by,(long long) vector);
	    else if (!str_prefix(word,"off"))     	REMOVE_BIT(pMobIndex->off_flags,vector);
	    else if (!str_prefix(word,"imm"))  	REMOVE_BIT(pMobIndex->imm_flags,vector);
	    else if (!str_prefix(word,"res"))   	REMOVE_BIT(pMobIndex->res_flags,vector);
	    else if (!str_prefix(word,"vul"))   	REMOVE_BIT(pMobIndex->vuln_flags,vector);
	    else if (!str_prefix(word,"for"))    	REMOVE_BIT(pMobIndex->form,vector);
	    else if (!str_prefix(word,"par"))   	REMOVE_BIT(pMobIndex->parts,vector);
	    else {
	        bug("Flag remove: flag not found.",0);
	        exit(1);
	    }

            letter = fread_letter( fp );
        }

       /* Put it back... */

        ungetc(letter, fp);

       /* Now what was it? */ 

        switch (letter) {

          case '>':
            mprog_read_programs(fp, pMobIndex);
            break;

          case '+':
            load_mob_skills(fp, pMobIndex);
            break;

          case 'X':
            load_mob_extended(fp, pMobIndex);
            break;

          default:
            break; 
        }
 
       /* Sort out the indexing... */

        iHash                   = vnum % MAX_KEY_HASH;
        pMobIndex->next         = mob_index_hash[iHash];
        mob_index_hash[iHash]   = pMobIndex;
        top_mob_index++;
        top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;  /* OLC */
        assign_area_vnum( vnum );

       /* Clear the kill table... */

        kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
    }
 
    return;
}


/* Load skills for a mob... 
 *
 * File Syntax is +skill rating
 */

void load_mob_skills(FILE *fp, MOB_INDEX_DATA *pMobIndex) {
bool done;
char letter;
char *sname;
short sn;
short pts; 

 /* Get a skill data block for the index... */

  pMobIndex->skills = getSkillData();

 /* Now see what's in the file... */ 

  done = FALSE;

  while (!done) {

    letter = fread_letter(fp);

    if (letter != '+') {
      ungetc(letter, fp);
      done = TRUE;
    } else {
      pts = fread_number(fp);
      sname = fread_string(fp);
      sn = get_skill_sn(sname);
      if (sname < 0) {
        bug("Load_Mob_Skills: Unknown skill", 0);
        bug(sname, 0);
      } else {
        setSkill(pMobIndex->skills, sn, pts);
      } 
    }   
  }

 /* Do we have some mob_progs to load? */
 
  if (letter == '>') {
    mprog_read_programs(fp, pMobIndex);
  }

 /* All done... */

  return;
}

/* Extended (keyword based) mob load... */

void load_mob_extended(FILE *fp, MOB_INDEX_DATA *pMobIndex) {
char letter;
char *word;
char *name;
bool done, ok;
CONV_DATA *cd;
CONV_SUB_DATA *csd;
CONV_SUB_TRANS_DATA *cstd;
ECOND_DATA *ec;
MOB_SCRIPT *scr;
MOB_SCRIPT_LINE *scrl;
MOB_TRIGGER *trig;
MONITOR_LIST *mon;
CHAT_DATA *chat;

 /* Ok, straight processing loop... */

  done = FALSE;

  cd = NULL;
  csd = NULL;
  cstd = NULL;

  ec = NULL;

  scr = NULL;
  scrl = NULL;

  trig = NULL;

  while (!done) {
    word = fread_word(fp);

    ok = FALSE;

    switch (word[0]) {

      case 'B':

       /* Bio... */

        if (!str_cmp(word, "Bio")) {
          ok = TRUE;
          pMobIndex->bio          = fread_string( fp );
          break;
        }

        break;

      case 'C':

       /* Chat... */

        if (!str_cmp(word, "Chat")) {
          ok = TRUE;
          chat = read_chat(fp);

          if (chat != NULL) {
            chat->next = pMobIndex->triggers->chat;
            pMobIndex->triggers->chat = chat;
          }

          break;
        }

       /* Conversation... */

        if (!str_cmp(word, "Conv")) {
          ok = TRUE;

          cd = get_conv_data();

          cd->conv_id = fread_number(fp);
          cd->name = str_dup(fread_word(fp));

          cd->next = pMobIndex->cd;
          pMobIndex->cd = cd;
          csd = NULL;
          cstd = NULL;   

          break;
        }

       /* Conversation Subject */

        if (!str_cmp(word, "ConvSub")) {
          ok = TRUE;

          csd = get_conv_sub_data();

          csd->sub_id = fread_number(fp);
          csd->name = str_dup(fread_word(fp));
          csd->kwds = str_dup(fread_word(fp));

          cd = pMobIndex->cd;

          if (cd == NULL) {
            bug("ConvSub before Conv in extended mob data!", 0);
            free_conv_sub_data(csd); 
          } else {
            csd->next = cd->csd;
            cd->csd = csd;
          }  

          cstd = NULL;
  
          break;
        }

       /* Conversation subject transition */

        if (!str_cmp(word, "ConvSubTrans")) {
          ok = TRUE;

          cstd = get_conv_sub_trans_data();

          cstd->seq = fread_number(fp);
          cstd->in = fread_number(fp);
          cstd->out = fread_number(fp);
          cstd->action = str_dup(fread_string(fp));

          cstd->cond = NULL;

          cd = pMobIndex->cd;
     
          if (cd != NULL) {
            csd = cd->csd;
          } else {
            csd = NULL;
          }

          if (csd == NULL) {
            bug("ConvSubTrans before ConvSub in extended mob data!", 0);
            free_conv_sub_trans_data(cstd); 
          } else {
            insert_cstd(cstd, csd);
          }  

          break;
        }

       /* CSTD Condition... */

        if (!str_cmp(word, "ConvCond")) {
          ok = TRUE;

          ec = read_econd(fp);

          if (ec == NULL) {
            bug("Bad ConvCond in extended mob data!", 0);
          } else {
            if (cstd == NULL) {
              bug("ConvCond before ConvSubTrans in extended mob data!", 0);
              free_ec(ec);
            } else {   
              ec->next = cstd->cond;
              cstd->cond = ec;
            }
          }

          break;
        }

       /* CanSee... */

        if (!str_cmp(word, "CanSee")) {
          ok = TRUE;

          ec = read_econd(fp);

          if (ec == NULL) {
            bug("Bad CanSee in extended mob data!", 0);
          } else {
            ec->next = pMobIndex->can_see;
            pMobIndex->can_see = ec;
          }
        }

       /* CanNotSee... */

        if (!str_cmp(word, "CanNotSee")) {
          ok = TRUE;

          ec = read_econd(fp);

          if (ec == NULL) {
            bug("Bad CanNotSee in extended mob data!", 0);
          } else {
            ec->next = pMobIndex->can_not_see;
            pMobIndex->can_not_see = ec;
          }

          break;
        }

        break;

      case 'E':

        if (!str_cmp(word, "EnvImm")) {
           ok = TRUE;
           pMobIndex->envimm_flags = fread_flag( fp ) | race_array[pMobIndex->race].envimm;
        } 

        if (!str_cmp(word, "End")) {
          done = TRUE;
          ok = TRUE;
        }
  
        break;   

      case 'F':

        if (!str_cmp(word, "Fright")) {
          ok = TRUE;
          pMobIndex->fright = fread_number(fp);
        } 

        break;

  
      case 'I':

       /* Image... */

        if (!str_cmp(word, "Image")) {
          ok = TRUE;
          pMobIndex->image          = str_dup(fread_word( fp ));
        }
        break;


      case 'L':

        if (!str_cmp(word, "Language")) {
          ok = TRUE;

          name = fread_string(fp);
          pMobIndex->language = UMAX(get_skill_sn(name), 0);
        } 
        break;


      case 'M':

        if (!str_cmp(word, "Monitor")) {
          ok = TRUE;
          mon = read_monitor(fp);
          if (mon != NULL) pMobIndex->monitors = insert_monitor(pMobIndex->monitors, mon);
        } 
        break;
  
      case 'N':

        if (!str_cmp(word, "Nature")) {
          ok = TRUE;

          pMobIndex->nature = fread_number(fp);
        } 

        break;
  
      case 'S':

       /* Script... */

        if (!str_cmp(word, "Script")) {
          ok = TRUE;

          scr = get_mscr();

          scr->id = fread_number(fp);
          scr->name = str_dup(fread_word(fp));

          scr->next = pMobIndex->triggers->scripts;
          pMobIndex->triggers->scripts = scr;
          scrl = NULL;

          break;
        }

       /* Script Line... */

        if (!str_cmp(word, "ScriptLine")) {
          ok = TRUE;

          scrl = get_mscrl();

          scrl->seq 	= fread_number(fp);
          scrl->delay 	= fread_number(fp);
          scrl->cmd_id	= fread_number(fp);
          scrl->cmd 	= str_dup(fread_string(fp));

          scr = pMobIndex->triggers->scripts; 

          if (scr == NULL) {
            bug("Script Line before Script in extended mob data!", 0);
            free_mscrl(scrl);
            scrl = NULL;
          } else {
            insert_script_line(scr, scrl);
          }
  
          break; 
        }

        break;

      case 'T':

       /* TriggerChallange... */

        if (!str_cmp(word, "TriggerChallange")) {
          ok = TRUE;

          trig = read_trigger(fp);

          pMobIndex->triggers->challange = 
                           add_trigger(pMobIndex->triggers->challange, trig);

          break;
        }

       /* TriggerReact... */

        if (!str_cmp(word, "TriggerReact")) {
          ok = TRUE;

          trig = read_trigger(fp);

          pMobIndex->triggers->reaction = 
                            add_trigger(pMobIndex->triggers->reaction, trig);

          break;
        }

       /* Trigger Text... */

        if (!str_cmp(word, "TriggerText")) {
          ok = TRUE;

          if (trig == NULL) {
            bug("Trigger Text before Trigger in extended mob data!", 0);
          } else if (trig->text != NULL) {
            bug("Duplicate Trigger Text in extended mob data!", 0);
          } else {
            trig->text = str_dup(fread_string(fp)); 
          }

          break;
        }

       /* Trigger Condition... */

        if (!str_cmp(word, "TriggerCond")) {
          ok = TRUE;

          ec = read_econd(fp);

          if (trig == NULL) {
            bug("Trigger Condition before Trigger in extended mob data!", 0);
            free_ec(ec);
          } else {
            ec->next = trig->cond;
            trig->cond = ec; 
          }

          break;
        }

       /* Timesub... */
   
        if (!str_cmp(word, "TimeSub")) {
          ok = TRUE;

          pMobIndex->time_wev = fread_number(fp);

          break;
        } 

        break;

      case 'X':

        if (!str_cmp(word, "X")) {
          ok = TRUE;
          break;
        }
 
      default:
        break;
    }

    if (!ok) {
      log_string(word);
      bug("Unrecognized keyword in extended mob data!", 0);
    }
  }

 /* Yuck. Peek at next char to see what follows... */

  letter = fread_letter( fp );
  ungetc(letter, fp);

 /* Do we have some mob_progs to load? */

  if (letter == '+') {
    load_mob_skills(fp, pMobIndex);
  } 
 
  if (letter == '>') {
    mprog_read_programs(fp, pMobIndex);
  }

  return;
}


/*
 * Snarf an obj section. new style
 */
void load_objects( FILE *fp, bool is_new ) {
OBJ_INDEX_DATA *pObjIndex;
EXTRA_DESCR_DATA *ed;
ECOND_DATA *new_ec;
DEED *new_deed;
ANCHOR_LIST *anchor;

    if ( !area_last ) {
        bug( "Load_objects: no #AREA seen yet.", 0 );
        exit( 1 );
    }

    ed = NULL;

    for ( ; ; ) {
        long vnum;
        char letter;
        int iHash;
        long rom_extra = 0;
 
        letter                          = fread_letter( fp );
        if ( letter != '#' ) {
            bug( "Load_objects: # not found.", 0 );
            exit( 1 );
        }
 
        vnum                            = fread_number_long( fp );
        if ( vnum == 0 ) break;
 
        fBootDb = FALSE;
        if ( get_obj_index( vnum ) != NULL ) {
            bug( "Load_objects: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;
 
        pObjIndex                       = (OBJ_INDEX_DATA *) alloc_perm( sizeof(*pObjIndex) );

        pObjIndex->repop		= 100;
        pObjIndex->vnum                 = vnum;
        pObjIndex->area                 = area_last;            /* OLC */
        pObjIndex->new_format           = TRUE;
        pObjIndex->reset_num		= 0;
        pObjIndex->zmask		= 0;
        pObjIndex->affected		= NULL;
        pObjIndex->extra_descr		= NULL;
        pObjIndex->image		= NULL;
        newobjs++;
        pObjIndex->name                 = fread_string( fp );
        pObjIndex->short_descr          = fread_string( fp );
        pObjIndex->description          = fread_string( fp );
			
        pObjIndex->material		= material_lookup(fread_string( fp ));

        if (area_last->version == VERSION_CTHULHU_0) { 
             pObjIndex->item_type            = fread_number( fp );
        } else {
             pObjIndex->item_type            = item_lookup(fread_word( fp ));
        }

        pObjIndex->extra_flags          = fread_flag_long( fp );

        if (area_last->version == VERSION_ROM) {
             rom_extra = pObjIndex->extra_flags;		
             pObjIndex->extra_flags &= !(ROM_ITEM_MASK);

             if (IS_SET(rom_extra, ROM_MELT_DROP)) {
	    SET_BIT(pObjIndex->extra_flags, ITEM_MELT_DROP);
             }

             if (IS_SET(rom_extra, ROM_NOLOCATE)) {
	    SET_BIT(pObjIndex->extra_flags, ITEM_NOLOCATE);
             }

             if (IS_SET(rom_extra, ROM_SELL_EXTRACT)) {
	    SET_BIT(pObjIndex->extra_flags, ITEM_SELL_EXTRACT);
             }

             if (IS_SET(rom_extra, ROM_NOUNCURSE)) {
	    SET_BIT(pObjIndex->extra_flags, ITEM_NOUNCURSE);
             }

             if (IS_SET(rom_extra, ROM_BURN_PROOF)) {
	    SET_BIT(pObjIndex->extra_flags, ITEM_BURN_PROOF);
             }
        }

        pObjIndex->wear_flags           = fread_flag( fp );

        if (area_last->version == VERSION_CTHULHU_0) {

            pObjIndex->value[0]             = fread_flag( fp );
            pObjIndex->value[1]             = fread_flag( fp );
            pObjIndex->value[2]             = fread_flag( fp );
            pObjIndex->value[3]             = fread_flag( fp );
            pObjIndex->value[4]             = fread_flag( fp );

    	    switch (pObjIndex->item_type) {

                default: 
                    break;

                case ITEM_PILL:
                case ITEM_POTION:
                case ITEM_SCROLL:
                    pObjIndex->value[1] = get_effect_for_slot( pObjIndex->value[1] );
                    pObjIndex->value[2] = get_effect_for_slot( pObjIndex->value[2] );
                    pObjIndex->value[3] = get_effect_for_slot( pObjIndex->value[3] );
                    break;

                case ITEM_STAFF:
                case ITEM_WAND:
                    pObjIndex->value[3] = get_effect_for_slot( pObjIndex->value[3] );
                    break;

                case ITEM_CONTAINER:
                    if ( pObjIndex->value[3] <= 25 
                      || pObjIndex->value[3] >= 200 ) {
                        pObjIndex->value[3] = 100;
                    }
                    break;
            }

        } else {

    	    switch (pObjIndex->item_type) {

	        case ITEM_WEAPON:
                    pObjIndex->value[0]             = weapon_class_lookup(fread_word( fp ));
                    pObjIndex->value[1]             = fread_number( fp );
                    pObjIndex->value[2]             = fread_number( fp );
                    pObjIndex->value[3]             = attack_type_lookup(fread_word( fp ));
                    pObjIndex->value[4]             = fread_flag( fp );
                    break; 

	        case ITEM_CONTAINER:
                    pObjIndex->value[0]             = fread_number( fp );
                    pObjIndex->value[1]             = fread_flag( fp );
                    pObjIndex->value[2]             = fread_number( fp );
                    pObjIndex->value[3]             = fread_number( fp );
                    pObjIndex->value[4]             = fread_number( fp );

                    if ( pObjIndex->value[3] <= 25 
                      || pObjIndex->value[3] >= 400 ) {
                        pObjIndex->value[3] = 100;
                    }

                    break;

	        case ITEM_DRINK_CON:
                    pObjIndex->value[0]             = fread_number( fp );
                    pObjIndex->value[1]             = fread_number( fp );
                    pObjIndex->value[2]             = liquid_lookup(fread_word( fp ));
                    pObjIndex->value[3]             = fread_number( fp );
                    pObjIndex->value[4]             = get_effect_efn(fread_word( fp ));
                    break;

	        case ITEM_FOUNTAIN:
                    if ( area_last->version == VERSION_ROM ) {
                        pObjIndex->value[0]             = fread_number( fp );
                        pObjIndex->value[1]             = fread_number( fp );
                        pObjIndex->value[2]             = liquid_lookup(fread_word( fp ));
                        pObjIndex->value[3]             = fread_number( fp );
                        pObjIndex->value[4]             = get_effect_efn(fread_word( fp ));
                    } else {
                        pObjIndex->value[0]             = liquid_lookup(fread_word( fp ));
                        pObjIndex->value[1]             = fread_number( fp );
                        pObjIndex->value[2]             = fread_number( fp );
                        pObjIndex->value[3]             = get_effect_efn(fread_word( fp ));
                        pObjIndex->value[4]             = fread_number( fp );
                    }
                    break;

	        case ITEM_FOOD:
                    pObjIndex->value[0]             = fread_number( fp );
                    pObjIndex->value[1]             = fread_number( fp );
                    pObjIndex->value[2]             = get_effect_efn(fread_word( fp ));
                    pObjIndex->value[3]             = fread_number( fp );
                    pObjIndex->value[4]             = fread_number( fp );
                    break;

	        case ITEM_WAND:
	        case ITEM_STAFF:
                    pObjIndex->value[0]             = fread_number( fp );
                    pObjIndex->value[1]             = fread_number( fp );
                    pObjIndex->value[2]             = fread_number( fp );
                    pObjIndex->value[3]             = get_effect_efn(fread_word( fp ));
                    pObjIndex->value[4]             = fread_number( fp );
                    break;

	        case ITEM_POTION:
	        case ITEM_PILL:
	        case ITEM_SCROLL:
                    pObjIndex->value[0]             = fread_number( fp );
                    pObjIndex->value[1]             = get_effect_efn(fread_word( fp ));
                    pObjIndex->value[2]             = get_effect_efn(fread_word( fp ));
                    pObjIndex->value[3]             = get_effect_efn(fread_word( fp ));
                    pObjIndex->value[4]             = get_effect_efn(fread_word( fp ));
                    break;

	        case ITEM_LIGHT:
                    pObjIndex->value[0]             = fread_number( fp );
                    pObjIndex->value[1]             = fread_number( fp );
                    pObjIndex->value[2]             = fread_number( fp );
                    pObjIndex->value[3]             = fread_number( fp );
                    pObjIndex->value[4]             = fread_number( fp );

                    if ( pObjIndex->value[2] == 999 ) {
                      pObjIndex->value[2] = -1;
                    } 

                    break;

	        case ITEM_HERB:
                    pObjIndex->value[0]             = fread_number( fp );
                    pObjIndex->value[1]             = get_effect_efn(fread_word( fp ));
                    pObjIndex->value[2]             = get_effect_efn(fread_word( fp ));
                    pObjIndex->value[3]             = fread_number( fp );
                    pObjIndex->value[4]             = fread_number( fp );
                    break;

	        case ITEM_PIPE:
                    pObjIndex->value[0]             = fread_number( fp );
                    pObjIndex->value[1]             = fread_number( fp );
                    pObjIndex->value[2]             = fread_number( fp );
                    pObjIndex->value[3]             = get_effect_efn(fread_word( fp ));
                    pObjIndex->value[4]             = get_effect_efn(fread_word( fp ));
                    break;

	        case ITEM_BOOK:
                    pObjIndex->value[0]             = get_skill_sn(fread_word( fp ));
                    pObjIndex->value[1]             = fread_number( fp );
                    pObjIndex->value[2]             = get_skill_sn(fread_word( fp ));
                    pObjIndex->value[3]             = get_skill_sn(fread_word( fp ));
                    pObjIndex->value[4]             = get_skill_sn(fread_word( fp ));
                    break;

	        default:
                    pObjIndex->value[0]             = fread_flag( fp );
                    pObjIndex->value[1]             = fread_flag( fp );
                    pObjIndex->value[2]             = fread_flag( fp );
                    pObjIndex->value[3]             = fread_flag( fp );
                    pObjIndex->value[4]             = fread_flag( fp );
                    break;
	    } 
        }

        pObjIndex->level		= fread_number( fp );
        pObjIndex->weight               = fread_number( fp );

	if (area_last->version == VERSION_ROM) {
             pObjIndex->weight /= 10;
        }

        pObjIndex->cost                 = fread_number( fp ); 
         
        /* condition */
        letter 				= fread_letter( fp );
	switch (letter)
 	{
	    case ('P') :		pObjIndex->condition = 100; break;
	    case ('G') :		pObjIndex->condition =  90; break;
	    case ('A') :		pObjIndex->condition =  75; break;
	    case ('W') :		pObjIndex->condition =  50; break;
	    case ('D') :		pObjIndex->condition =  25; break;
	    case ('B') :		pObjIndex->condition =  10; break;
	    case ('R') :		pObjIndex->condition =   0; break;
	    default:			pObjIndex->condition = 100; break;
	}
 
        for ( ; ; )     {
            char letter;
 
            letter = fread_letter( fp );
            
            if ( letter == 'A' )  {
                AFFECT_DATA *paf;
 
                paf                     = new_affect();

                paf->where              = APPLY_TO_OBJECT;

                paf->type               = SKILL_UNDEFINED;
                paf->afn		= 0;
                paf->level              = pObjIndex->level;
                paf->duration           = -1;
                paf->location           = fread_number( fp );
                paf->skill              = 0;
                paf->modifier           = fread_number( fp );
                paf->bitvector          = 0;

                paf->next               = pObjIndex->affected;
                pObjIndex->affected     = paf;

                top_affect++;
            }
 
	    else if (letter == 'F')
            {
                AFFECT_DATA *paf;
 
                paf                     = new_affect();

		letter 			= fread_letter(fp);

		switch (letter) {
	 	
		  case 'A':
                    paf->where          = APPLY_TO_AFFECTS;
		    break;
		  case 'I':
		    paf->where		= APPLY_TO_IMMUNE;
		    break;
		  case 'R':
		    paf->where		= APPLY_TO_RESIST;
		    break;
		  case 'V':
		    paf->where		= APPLY_TO_VULN;
		    break;
		  default:
            	    bug( "Load_objects: Bad where on flag set.", 0 );
            	    exit( 1 );
		}

                paf->type               = SKILL_UNDEFINED;
                paf->afn		= 0;
                
                paf->level              = pObjIndex->level;
                paf->duration           = -1;
                paf->location           = fread_number(fp);
                paf->modifier           = fread_number(fp);
                paf->bitvector          = fread_flag(fp);

                paf->next               = pObjIndex->affected;
                pObjIndex->affected     = paf;

                top_affect++;
            } else if ( letter == 'N' ) {
                AFFECT_DATA *paf;
 		int slot_num; 

                paf                     = new_affect();

		slot_num		= fread_number(fp);

		paf->type		= SKILL_UNDEFINED;
                paf->afn		= 0;
                paf->level              = pObjIndex->level;
                paf->duration           = -1;
                paf->location           = fread_number( fp );
                paf->skill              = 0;
                paf->modifier           = fread_number( fp );
                paf->bitvector          = fread_number_long_long(fp);

                paf->next               = pObjIndex->affected;
                pObjIndex->affected     = paf;

                top_affect++;
            } else if ( letter == 'P' )  {
                AFFECT_DATA *paf;

                paf                     = new_affect();
	paf->type		= SKILL_UNDEFINED;
                paf->afn		= 0;
                paf->level              = pObjIndex->level;
                paf->location           = APPLY_EFFECT;
                paf->skill              = get_effect_efn(fread_word(fp));
                paf->modifier           = fread_number(fp);
                paf->duration           = fread_number(fp);

                paf->next               = pObjIndex->affected;
                pObjIndex->affected     = paf;
                top_affect++;

            } else if ( letter == 'S' )  {
                AFFECT_DATA *paf;

                paf                     = new_affect();

		paf->type		= SKILL_UNDEFINED;
                paf->afn		= 0;
                paf->level              = pObjIndex->level;
                paf->duration           = -1;
                paf->location           = APPLY_SKILL;
                paf->skill              = get_skill_sn(fread_word(fp));
                paf->modifier           = fread_number( fp );

                paf->next               = pObjIndex->affected;
                pObjIndex->affected     = paf;

                top_affect++;

            } else if ( letter == 'E' )  {
                ed                      = (EXTRA_DESCR_DATA *) alloc_perm( sizeof(*ed) );
                ed->keyword             = fread_string( fp );
                ed->description         = fread_string( fp );
                ed->next                = pObjIndex->extra_descr;
                ed->ec  		= NULL;
                ed->deed		= NULL;
                pObjIndex->extra_descr  = ed;
                top_ed++;

            } else if ( letter == 'C' ) {
              new_ec = read_econd(fp);
              if (ed == NULL) {
                bug("C before E in object!", 0);
                free_ec(new_ec);
              } else {
                new_ec->next = ed->ec;
                ed->ec = new_ec;
              }

            } else if ( letter == 'H' ) {
                 anchor = read_anchor(fp);

                 if (anchor != NULL) pObjIndex->anchors = insert_anchor(pObjIndex->anchors, anchor);

            } else if ( letter == 'W' ) {
              new_ec = read_econd(fp);
              new_ec->next = pObjIndex->wear_cond;
              pObjIndex->wear_cond = new_ec;

            } else if ( letter == 'D' )  {
              new_deed = read_deed(fp);
              if (ed == NULL) {
                bug("D before E in object!", 0);
                free_deed(new_deed);
              } else {
                new_deed->next = ed->deed;
                ed->deed = new_deed;
              }
            }

	   /* add check for image URL */	

	    else if ( letter == 'I' )
	    {
		pObjIndex->image = str_dup(fread_word(fp));
	    }	
		
	   /* add check for new repop % designator */	

	    else if ( letter == 'R' )
	    {
	        int value;
		value = fread_number (fp);
		if (value < 1 || value > 100)
		    bug ("Invalid repop percentage, leaving default 100.", 0);
		else
		    pObjIndex->repop = value;


            } else if ( letter == 'T' ) {
                ARTIFACT *art;

	art = new_artifact();

                art->power = fread_number(fp);
                art->attract = fread_number(fp);
                art->pulse = fread_number(fp);

	art->next = pObjIndex->artifact_data;
	pObjIndex->artifact_data = art;

           }	else if ( letter == 'V' ) {

              int index;
              char type;
              char *word;  
              int val;

              index = fread_number(fp);
        
              type = fread_letter(fp);

              word = fread_word(fp);

              if ( index < 0 
                || index > 4) {
                bug("Bad extended value index %d.", index);
              } else {
                switch (type) {
                  case 'K':
                    val = get_skill_sn(word); 
                    if (val == SKILL_UNDEFINED) {
                      val = 0;
                    }
                    pObjIndex->value[index] = val; 
                    break;
                    
                  case 'P':
                    val = get_effect_efn(word); 
                    if (val == SPELL_UNDEFINED) {
                      val = 0;
                    }
                    pObjIndex->value[index] = val; 
                    break;
                    
                  default:
                    bug("Bad extended value type %c.", type); 
                    break;
                }
              }
            }

	   /* add check for zone mask */	

	    else if ( letter == 'Z' )
	    {
		pObjIndex->zmask = fread_flag(fp);
	    }	
		
			
            else
            {
                ungetc( letter, fp );
                break;
            }
        }

       /* Complete adding the object to the structues... */

        iHash                   = vnum % MAX_KEY_HASH;
        pObjIndex->next         = obj_index_hash[iHash];
        obj_index_hash[iHash]   = pObjIndex;
        top_obj_index++;
        top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;   /* OLC */
        assign_area_vnum( vnum );                                   /* OLC */
    }
 
    return;
}


/*****************************************************************************
 Name:          convert_objects
 Purpose:       Converts all old format objects to new format
 Called by:     boot_db (db.c).
 Note:          Loops over all resets to find the level of the mob
                loaded before the object to determine the level of
                the object.
                It might be better to update the levels in load_resets().
                This function is not pretty.. Sorry about that :)
 Author:        Hugin
 ****************************************************************************/
void convert_objects( void )
{
    int vnum;
    AREA_DATA  *pArea;
    RESET_DATA *pReset;
    MOB_INDEX_DATA *pMob = NULL;
    OBJ_INDEX_DATA *pObj;
    ROOM_INDEX_DATA *pRoom;

    if ( newobjs == top_obj_index ) return; /* all objects in new format */

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
        for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
        {
            if ( !( pRoom = get_room_index( vnum ) ) ) continue;

            for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
            {
                switch ( pReset->command )
                {
                case 'M':
                    if ( !( pMob = get_mob_index( pReset->arg1 ) ) )
                        bug( "Convert_objects: 'M': bad vnum %d.", pReset->arg1 );
                    break;

                case 'O':
                    if ( !( pObj = get_obj_index( pReset->arg1 ) ) )
                    {
                        bug( "Convert_objects: 'O': bad vnum %d.", pReset->arg1 );
                        break;
                    }
                    if ( pObj->new_format )
                        continue;

                    if ( !pMob )
                    {
                        bug( "Convert_objects: 'O': No mob reset yet.", 0 );
                        break;
                    }

                    pObj->level = pObj->level < 1 ? pMob->level - 2
                        : UMIN(pObj->level, pMob->level - 2);
                    break;

                case 'P':
                    {
                        OBJ_INDEX_DATA *pObj, *pObjTo;

                        if ( !( pObj = get_obj_index( pReset->arg1 ) ) )
                        {
                            bug( "Convert_objects: 'P': bad vnum %d.", pReset->arg1 );
                            break;
                        }

                        if ( pObj->new_format )
                            continue;

                        if ( !( pObjTo = get_obj_index( pReset->arg3 ) ) )
                        {
                            bug( "Convert_objects: 'P': bad vnum %d.", pReset->arg3 );
                            break;
                        }

                        pObj->level = pObj->level < 1 ? pObjTo->level
                            : UMIN(pObj->level, pObjTo->level);
                    }
                    break;

                case 'G':
                case 'E':
                    if ( !( pObj = get_obj_index( pReset->arg1 ) ) )
                    {
                        bug( "Convert_objects: 'E' or 'G': bad vnum %d.", pReset->arg1 );
                        break;
                    }

                    if ( !pMob )
                    {
                        bug( "Convert_objects: 'E' or 'G': null mob for vnum %d.",
                             pReset->arg1 );
                        break;
                    }

                    if ( pObj->new_format )
                        continue;

                    if ( pMob->pShop )
                    {
                        switch ( pObj->item_type )
                        {
                        default:
                            pObj->level = UMAX(0, pObj->level);
                            break;
                        case ITEM_PILL:
                        case ITEM_POTION:
                            pObj->level = UMAX(5, pObj->level);
                            break;
                        case ITEM_SCROLL:
                        case ITEM_ARMOR:
                        case ITEM_WEAPON:
                            pObj->level = UMAX(10, pObj->level);
                            break;
                        case ITEM_WAND:
                        case ITEM_TREASURE:
                            pObj->level = UMAX(15, pObj->level);
                            break;
                        case ITEM_STAFF:
                            pObj->level = UMAX(20, pObj->level);
                            break;
                        }
                    }
                    else
                        pObj->level = pObj->level < 1 ? pMob->level
                            : UMIN( pObj->level, pMob->level );
                    break;
                } /* switch ( pReset->command ) */
            }
        }
    }

    /* do the conversion: */

    for ( pArea = area_first; pArea ; pArea = pArea->next )
        for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
            if ( (pObj = get_obj_index( vnum )) )
                if ( !pObj->new_format )
                    convert_object( pObj );

    return;
}

/*****************************************************************************
 Name:          convert_object
 Purpose:       Converts an old_format obj to new_format
 Called by:     convert_objects (db2.c).
 Note:          Dug out of create_obj (db.c)
 Author:        Hugin
 ****************************************************************************/
void convert_object( OBJ_INDEX_DATA *pObjIndex )
{
    int level;
    int number, type;  /* for dice-conversion */

    if ( !pObjIndex || pObjIndex->new_format ) return;

    level = pObjIndex->level;

    pObjIndex->level    = UMAX( 0, pObjIndex->level ); /* just to be sure */
    pObjIndex->cost     = 10*level;

    switch ( pObjIndex->item_type )
    {
        default:
            bug( "Obj_convert: vnum %d bad type.", pObjIndex->item_type );
            break;

        case ITEM_LIGHT:
        case ITEM_LIGHT_REFILL:
        case ITEM_TREASURE:
        case ITEM_FURNITURE:
        case ITEM_TRASH:
        case ITEM_CONTAINER:
        case ITEM_KEY_RING:
        case ITEM_DRINK_CON:
        case ITEM_KEY:
        case ITEM_FOOD:
        case ITEM_BOAT:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
        case ITEM_FOUNTAIN:
        case ITEM_MAP:
        case ITEM_CLOTHING:
        case ITEM_SCROLL:
        case ITEM_LOCKER:
        case ITEM_LOCKER_KEY:
        case ITEM_CLAN_LOCKER:
        case ITEM_IDOL:
        case ITEM_SCRY:
        case ITEM_FORGING:
        case ITEM_RAW:
        case ITEM_BONDAGE:
        case ITEM_TATTOO:
        case ITEM_TRAP:
        case ITEM_DOLL:
        case ITEM_PAPER:
        case ITEM_EXPLOSIVE:
        case ITEM_AMMO:
        case ITEM_SLOTMACHINE:
        case ITEM_HERB:
        case ITEM_PIPE:
        case ITEM_TIMEBOMB:
        case ITEM_TREE:
        case ITEM_CAMERA:
        case ITEM_PHOTOGRAPH:
        case ITEM_WARP_STONE:
        case ITEM_ROOM_KEY:
        case ITEM_GEM:
        case ITEM_JEWELRY:
        case ITEM_JUKEBOX:
        case ITEM_SHARE:
        case ITEM_FIGURINE:
        case ITEM_POOL:
        case ITEM_GRIMOIRE:
        case ITEM_FOCUS:
        case ITEM_PROTOCOL:
        case ITEM_DECORATION:
        case ITEM_PASSPORT:
        case ITEM_INSTRUMENT:
            break;

        case ITEM_WAND:
        case ITEM_STAFF:
            pObjIndex->value[2] = pObjIndex->value[1];
            break;

        case ITEM_WEAPON:

            /*
             * The conversion below is based on the values generated
             * in one_hit() (fight.c).  Since I don't want a lvl 50
             * weapon to do 15d3 damage, the min value will be below
             * the one in one_hit, and to make up for it, I've made
             * the max value higher.
             * (I don't want 15d2 because this will hardly ever roll
             * 15 or 30, it will only roll damage close to 23.
             * I can't do 4d8+11, because one_hit there is no dice-
             * bounus value to set...)
             *
             * The conversion below gives:

             level:   dice      min      max      mean
               1:     1d8      1( 2)    8( 7)     5( 5)
               2:     2d5      2( 3)   10( 8)     6( 6)
               3:     2d5      2( 3)   10( 8)     6( 6)
               5:     2d6      2( 3)   12(10)     7( 7)
              10:     4d5      4( 5)   20(14)    12(10)
              20:     5d5      5( 7)   25(21)    15(14)
              30:     5d7      5(10)   35(29)    20(20)
              50:     5d11     5(15)   55(44)    30(30)

             */

            number = UMIN(level/4 + 1, 5);
            type   = (level + 7)/number;

            pObjIndex->value[1] = number;
            pObjIndex->value[2] = type;
            break;

        case ITEM_ARMOR:
            pObjIndex->value[0] = level / 5 + 3;
            pObjIndex->value[1] = pObjIndex->value[0];
            pObjIndex->value[2] = pObjIndex->value[0];
            break;

        case ITEM_POTION:
        case ITEM_PILL:
            break;

        case ITEM_MONEY:
            pObjIndex->value[0] = pObjIndex->cost;
            break;
    }

    pObjIndex->new_format = TRUE;
    ++newobjs;

    return;
}

/*****************************************************************************
 Name:          convert_mobile
 Purpose:       Converts an old_format mob into new_format
 Called by:     load_old_mob (db.c).
 Note:          Dug out of create_mobile (db.c)
 Author:        Hugin
 ****************************************************************************/
void convert_mobile( MOB_INDEX_DATA *pMobIndex )
{
    int i;
    int type, number, bonus;
    int level;

    if ( !pMobIndex || pMobIndex->new_format ) return;

    level = pMobIndex->level;

    pMobIndex->act              |= ACT_WARRIOR;

    /*
     * Calculate hit dice.  Gives close to the hitpoints
     * of old format mobs created with create_mobile()  (db.c)
     * A high number of dice makes for less variance in mobiles
     * hitpoints.
     * (might be a good idea to reduce the max number of dice)
     *
     * The conversion below gives:

       level:     dice         min         max        diff       mean
         1:       1d2+6       7(  7)     8(   8)     1(   1)     8(   8)
         2:       1d3+15     16( 15)    18(  18)     2(   3)    17(  17)
         3:       1d6+24     25( 24)    30(  30)     5(   6)    27(  27)
         5:      1d17+42     43( 42)    59(  59)    16(  17)    51(  51)
        10:      3d22+96     99( 95)   162( 162)    63(  67)   131(    )
        15:     5d30+161    166(159)   311( 311)   145( 150)   239(    )
        30:    10d61+416    426(419)  1026(1026)   600( 607)   726(    )
        50:    10d169+920   930(923)  2610(2610)  1680(1688)  1770(    )

        The values in parenthesis give the values generated in create_mobile.
        Diff = max - min.  Mean is the arithmetic mean.
        (hmm.. must be some roundoff error in my calculations.. smurfette got
         1d6+23 hp at level 3 ? -- anyway.. the values above should be
         approximately right..)
     */
    type   = level*level*27/40;
    number = UMIN(type/40 + 1, 10); /* how do they get 11 ??? */
    type   = UMAX(2, type/number);
    bonus  = UMAX(0, (level*(8 + level)*9)/10 - number*type);

    pMobIndex->hit[DICE_NUMBER]    = number;
    pMobIndex->hit[DICE_TYPE]      = type;
    pMobIndex->hit[DICE_BONUS]     = bonus;

    pMobIndex->mana[DICE_NUMBER]   = level;
    pMobIndex->mana[DICE_TYPE]     = 10;
    pMobIndex->mana[DICE_BONUS]    = 100;

    /*
     * Calculate dam dice.  Gives close to the damage
     * of old format mobs in damage()  (fight.c)
     */
    type   = level*7/4;
    number = UMIN(type/8 + 1, 5);
    type   = UMAX(2, type/number);
    bonus  = UMAX(0, level*9/4 - number*type);

    /*
     * Calculate dam dice.  Gives close to the damage
     * of old format mobs in damage()  (fight.c)
     */
    type   = level*7/4;
    number = UMIN(type/8 + 1, 5);
    type   = UMAX(2, type/number);
    bonus  = UMAX(0, level*9/4 - number*type);

    pMobIndex->damage[DICE_NUMBER] = number;
    pMobIndex->damage[DICE_TYPE]   = type;
    pMobIndex->damage[DICE_BONUS]  = bonus;

    switch ( number_range( 1, 3 ) )
    {
        case (1):
            pMobIndex->atk_type =  WDT_PIERCE;
            pMobIndex->dam_type =  DAM_PIERCE;
            break;

        case (2): 
            pMobIndex->atk_type =  WDT_SLASH;
            pMobIndex->dam_type =  DAM_SLASH;
            break;

        case (3):
            pMobIndex->atk_type =  WDT_PUNCH;
            pMobIndex->dam_type =  DAM_BASH;
            break;
    }

    for (i = 0; i < 3; i++)
        pMobIndex->ac[i]         = interpolate( level, 100, -100);
    pMobIndex->ac[3]             = interpolate( level, 100, 0);    /* exotic */

    pMobIndex->gold             /= 100;
    pMobIndex->size              = SIZE_MEDIUM;
    pMobIndex->material          = 0;

    pMobIndex->new_format        = TRUE;
    ++newmobs;

    return;
}


void save_socials() {
	FILE *fp;
	int i;
	
	fp = fopen (SOC_FILE, "w");
	
	if (!fp) {
	    bug ("Could not open " SOC_FILE " for writing.",0);
   	    return;
	}

	for ( i = 0 ; social_table[i].name[0] != '\0'; i++) {
                      save_social (&social_table[i], fp);
                      fprintf(fp, "\n");
                }
                fprintf(fp, "#0\n");
                fprintf(fp, "#$\n");
	fclose (fp);
}


void save_social (const struct social_type *s, FILE *fp) {
                fprintf(fp, "%s\n", s->name);
                if (s->s_style > 0) {
                    fprintf(fp, "> %d\n", s->s_style); 
                }
                fprintf(fp, "%s\n", s->char_no_arg ? s->char_no_arg : "$"); 
	fprintf(fp, "%s\n", s->others_no_arg  ? s->others_no_arg : "$");
                if (s->char_found) {
                       fprintf(fp, "%s\n", s->char_found);
                } else {
                       if (!s->others_found && !s->vict_found && !s->char_auto && !s->others_auto && !s->char_not_found)  {
                               fprintf(fp, "#\n");
                               return;
                       } else fprintf(fp, "$\n");
                }
                if (s->others_found) {
                       fprintf(fp, "%s\n", s->others_found);
                } else {
                       if (!s->vict_found && !s->char_auto && !s->others_auto && !s->char_not_found) {
                               fprintf(fp, "#\n");
                               return;
                       } else fprintf(fp, "$\n");
                }
                if (s->vict_found) {
                       fprintf(fp, "%s\n", s->vict_found);
                } else {
                       if (!s->char_auto && !s->others_auto && !s->char_not_found)  {
                                 fprintf(fp, "#\n");
                                 return;
                       } else fprintf(fp, "$\n");
                }
                if (s->char_not_found) {
                       fprintf(fp, "%s\n", s->char_not_found);
                } else {
                       if (!s->char_auto && !s->others_auto)  {
                                 fprintf(fp, "#\n");
                                 return;
                       } else fprintf(fp, "$\n");
                }
                if (s->char_auto) {
                       fprintf(fp, "%s\n", s->char_auto);
                } else {
                       if (!s->others_auto)   {
                                 fprintf(fp, "#\n");
                                 return;
                       } else fprintf(fp, "$\n");
                }
                if (s->others_auto) {
                       fprintf(fp, "%s\n", s->others_auto);
                } else {
                       fprintf(fp, "#\n");
                       return;
                }
                return;
}

int social_lookup (const char *name) {
    int i;
	
    for ( i = 0; social_table[i].name[0] != '\0'; i++ ) {
            if (!str_cmp(name, social_table[i].name)) return i;
    }
     return -1;
}


void load_rumors(void) {
FILE *fp;
ECOND_DATA *new_ec;
char *kwd;
bool done;
char *name;

    fp = fopen(RUMOR_FILE, "r");
    if (!fp) {
        log_string("No rumor file found!");
        exit(1);
        return;
    }
    rumor_count = -1;

  done = FALSE;
  while(!done) {
    kwd = fread_word(fp);

    switch (kwd[0]) {

      default:
        fread_to_eol(fp); 
        break; 

      case 'C':
          if (!str_cmp(kwd, "COMM")) {
               if (rumor_count == -1) {
                   bug("Rumor entry before RUMOR.",0);
                   exit(1);
               }
               name = fread_string(fp);
               rumor_array[rumor_count].command = str_dup(name);
          }
          break;

      case 'E':
          if (!str_cmp(kwd, "EC")) {
               if (rumor_count == -1) {
                   bug("Rumor entry before RUMOR.",0);
                   exit(1);
               }
                new_ec = read_econd(fp);
                if (new_ec != NULL) {
                    new_ec->next = rumor_array[rumor_count].ec;
                    rumor_array[rumor_count].ec = new_ec;
                }
          }
          if (!str_cmp(kwd, "END")) {
              done = TRUE;
          }             
          break;

      case 'G':
          if (!str_cmp(kwd, "GREATER")) {
               if (rumor_count == -1) {
                   bug("Rumor entry before RUMOR.",0);
                   exit(1);
               }
               rumor_array[rumor_count].greater = TRUE;
          }              
          break;

      case 'R':
          if (!str_cmp(kwd, "RUMOR")) {
               name = fread_string(fp);
               if (rumor_count++ > MAX_RUMOR-1) {
                     bug ("Too many rumors %d", MAX_RUMOR);
                     exit(1);
               }
               rumor_array[rumor_count].fortune = str_dup(name);
               rumor_array[rumor_count].ec = NULL;
               rumor_array[rumor_count].command = NULL;
               rumor_array[rumor_count].greater = FALSE;
          }
          break;

    }
  }
  fclose(fp);
  return;
}


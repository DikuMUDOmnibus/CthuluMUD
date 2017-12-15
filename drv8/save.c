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
#include "skill.h"
#include "affect.h"
#include "spell.h"
#include "exp.h"
#include "prof.h"
#include "conv.h" 
#include "deeds.h"
#include "quests.h"
#include "triggers.h"
#include "society.h"
#include "race.h"
#include "profile.h"
#include "olc.h"
#include "board.h"
#include "gsn.h"

extern char *fwrite_flag(long flags, char *buf);

/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];

DECLARE_DO_FUN(do_visible);

/*
 * Local functions.
 */
void	fwrite_char	(CHAR_DATA *ch,  FILE *fp );
void	fwrite_obj	(CHAR_DATA *ch,  OBJ_DATA *obj, FILE *fp, int iNest );
void        fwrite_morgue      (ROOM_INDEX_DATA *room, OBJ_DATA *obj, FILE *fp, int iNest );
void        fwrite_tree             (OBJ_DATA *obj, FILE *fp, int iNest );
void	fwrite_pet	(CHAR_DATA *pet, FILE *fp);
void	fread_char	(CHAR_DATA *ch,  FILE *fp );
void        fread_pet	(CHAR_DATA *ch,  FILE *fp );
void	fread_obj	(CHAR_DATA *ch,  FILE *fp, ROOM_INDEX_DATA *room);
void        reload_gun         	(CHAR_DATA *ch, OBJ_DATA *obj, int gnum, bool loud);
void        reset_loaded_obj	(ROOM_INDEX_DATA *room, OBJ_DATA *obj);
void 	save_ignore	(CHAR_DATA *ch, FILE *fp);
IGNORE *new_ignore	(void );
bool 	reindex_obj	(OBJ_DATA *obj);

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch ) {
char strsave[MAX_INPUT_LENGTH];
char buf[MAX_STRING_LENGTH];
FILE *fp;

   /* Don't save NPCs... */

    if ( IS_NPC(ch) ) return;
    
    if ( ch->desc != NULL 
    && ch->desc->original != NULL ) {
	ch = ch->desc->original;
    } 

    if ( ch->desc != NULL
    && ch->desc->connected != CON_PLAYING ) {
        return;
    } 

log_string("Saving character:");
log_string(ch->name);

    /* create god log */
    if (IS_IMMORTAL(ch))    {
	fclose(fpReserve);
	sprintf(strsave, "%s/%s",GOD_DIR, capitalize(ch->name));
	if ((fp = fopen(strsave,"w")) == NULL){
	    bug("save_imm: fopen",0);
	    perror(strsave);
 	}

	fprintf(fp,"Lev %2d Divinity %2d  %s%s\n", ch->level, ch->pcdata->divinity, ch->name, ch->pcdata->title);
	fclose( fp );
	fpReserve = fopen( NULL_FILE, "r" );
    }

    fclose( fpReserve );
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );

    fp = fopen( PLAYER_TEMP, "w" );

    if ( fp == NULL ) {
	bug( "Save_char_obj: fopen", 0 );
	perror( strsave );
    } else {

	fwrite_char( ch, fp );

	if ( ch->carrying != NULL ) fwrite_obj( ch, ch->carrying, fp, 0 );
	if ( ch->ooz != NULL ) fwrite_obj( ch, ch->ooz, fp, 0 );

	if (ch->pet) {
                      fwrite_pet(ch->pet, fp);
	      if (ch->pet->carrying) fwrite_obj(ch->pet, ch->pet->carrying, fp, 0 );
	      if (ch->pet->ooz) fwrite_obj(ch->pet, ch->pet->ooz, fp, 0 );
                }
	fprintf( fp, "#END\n" );
    }

    fclose( fp );

   /* move the file */

    umask(006);

    sprintf(buf,"mv %s %s",PLAYER_TEMP,strsave);
    system(buf);
#if defined(COMPRESS_PFILES)
        sprintf(buf,"gzip -fq %s",strsave);
        system(buf);
#endif
/* 	chmod (PLAYER_TEMP, 00660);
	chmod (strsave, 00660); */ 
    fpReserve = fopen( NULL_FILE, "r" );

log_string("saved");

    return;
}



/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp ) {
AFFECT_DATA *paf;
int sn,i;

    if (IS_SET(ch->form, FORM_BLEEDING)) {
            ch->limb[1] +=1;
            ch->limb[1] = UMIN(ch->limb[1], 9);
            update_wounds(ch);
    }

    fprintf( fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER"	);

    fprintf( fp, "Name %s~\n",	ch->name		);
    fprintf( fp, "Vers %d\n",   ch->version		);
    fprintf( fp, "Native %s~\n",ch->pcdata->native);
    if (ch->short_descr[0] != '\0')     	fprintf( fp, "ShD  %s~\n",	ch->short_descr	);
    if( ch->long_descr[0] != '\0')           fprintf( fp, "LnD  %s~\n",	ch->long_descr	);
    if (ch->description[0] != '\0')    	fprintf( fp, "Desc %s~\n",	ch->description	);
    if (ch->bio[0] != '\0')    	    	fprintf( fp, "Bio %s~\n",	ch->bio	);
    if (ch->short_descr_orig[0] != '\0')fprintf( fp, "ShDOrig  %s~\n",	ch->short_descr_orig	);
    if( ch->long_descr_orig[0] != '\0')	fprintf( fp, "LnDOrig  %s~\n",	ch->long_descr_orig	);
    if (ch->description_orig[0] != '\0') 	fprintf( fp, "DescOrig %s~\n",	ch->description_orig	);
    if (ch->poly_name[0] != '\0')	fprintf (fp, "Polyname %s~\n",  ch->poly_name			);
    fprintf( fp, "Race %s~\n", 	race_array[ch->race].name );
    fprintf( fp, "RaceOrig  %d\n",	ch->race_orig			);
    fprintf( fp, "Sex  %d\n",		ch->sex			);
    fprintf( fp, "Cla  %d\n",		ch->pc_class		);
    fprintf( fp, "Levl %d\n",		ch->level		);
    if (ch->pcdata->questpoints != 0)               	fprintf( fp, "QuestPnts %d\n",  ch->pcdata->questpoints );
    if (ch->pcdata->nextquest != 0)                   	fprintf( fp, "QuestNext %d\n",  ch->pcdata->nextquest   );
    else if (ch->pcdata->countdown != 0)        	fprintf( fp, "QuestNext %d\n",  30             );

    fprintf( fp, "Divinity %d\n", ch->pcdata->divinity);
    fprintf( fp, "Auth %d\n", ch->pcdata->authority);
    fprintf( fp, "Sec  %d\n",    	ch->pcdata->security   );
    if (ch->recall_perm) 		fprintf( fp, "Recall %ld\n", 	ch->recall_perm 	);
    fprintf( fp, "Home	 %ld\n",	ch->pcdata->home_room );

    if (ch->pcdata->played > 10000) ch->pcdata->played =1;
    fprintf( fp, "Plyd %d\n",		ch->pcdata->played + (int) (current_time - ch->pcdata->logon)	);
    fprintf( fp, "Scro %d\n", 		ch->lines		);
    fprintf( fp, "Room %ld\n", (ch->in_room == get_room_index(ROOM_VNUM_LIMBO)  && ch->was_in_room != NULL ) ? ch->was_in_room->vnum : ch->in_room == NULL ? ch->recall_perm : ch->in_room->vnum );

   /* Save waking room... */
    if (ch->waking_room != NULL)       fprintf(fp, "WRoom %ld\n", ch->waking_room->vnum);

   /* Save dreaming room... */
    if (ch->dreaming_room != NULL)  fprintf(fp, "DRoom %ld\n", ch->dreaming_room->vnum);

    fprintf(fp, "Nmare %d\n", ch->nightmare);
    if (IS_SET(ch->act, ACT_WERE)) fprintf(fp, "Were %d\n", ch->were_type);

   /* Save professions in sequence... */

    if (ch->profs != NULL) {

      LPROF_DATA *lprof;
 
      lprof = ch->profs;

      while (lprof != NULL) {

        if ( lprof->profession != NULL ) fprintf(fp, "Profession '%s' %d\n",  lprof->profession->name, lprof->level);
        lprof = lprof->next;
      } 

    }

    fprintf( fp, "HMV  %d %d %d %d %d %d\n", ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );

    fprintf(fp, "MAX_CURRENCY %d\n", MAX_CURRENCY);
    fprintf(fp, "Cash");
    for (i=0; i < MAX_CURRENCY; i++) fprintf(fp, " %ld ", ch->gold[i]);
    fprintf(fp, "\n");

    fprintf( fp, "Exp  %d\n",	        ch->exp		);
    if (ch->act != 0)	fprintf( fp, "Act  %lld\n",      ch->act		);
    REMOVE_BIT(ch->plr, PLR_QUESTOR);
    if (ch->plr != 0)	fprintf( fp, "Plr  %lld\n",      ch->plr		);
    if (ch->affected_by != 0)	fprintf( fp, "AfBy %lld\n",	ch->affected_by);

    fprintf( fp, "Comm %ld\n",   ch->comm		);
    fprintf( fp, "Notify %ld\n", ch->notify		);

    if (IS_IMMORTAL(ch)) fprintf (fp, "Wiznet %ld\n", ch->wiznet );

    if (ch->invis_level != 0) fprintf( fp, "Invi %d\n", 	ch->invis_level	);
    if (ch->cloak_level != 0) fprintf (fp, "Cloak %d\n",	ch->cloak_level );

    fprintf( fp, "Pos  %d\n",	ch->position == POS_FIGHTING ? POS_STANDING : ch->position );
    if (ch->pos_desc != NULL)        fprintf(fp, "PosDesc %s~\n",	ch->pos_desc	);
    if (ch->practice != 0)    	fprintf( fp, "Prac %d\n",	ch->practice	);
    fprintf( fp, "Train %d %d %d %d %d %d %d %d %d %d %d %d\n", ch->train, ch->training[TRAIN_STAT_STR], ch->training[TRAIN_STAT_INT], ch->training[TRAIN_STAT_WIS],
	ch->training[TRAIN_STAT_DEX], ch->training[TRAIN_STAT_CON], ch->training[TRAIN_STAT_CHA], ch->training[TRAIN_HITS], ch->training[TRAIN_MANA],ch->training[TRAIN_MOVE],
	ch->training[TRAIN_PRACTICE], ch->training[TRAIN_SANITY]);

    if (ch->saving_throw != 0) fprintf( fp, "Save  %d\n",	ch->saving_throw);
    fprintf( fp, "Sanity  %d\n", ch->sanity);
    fprintf( fp, "Alig  %d\n",	ch->alignment);
    if (ch->hitroll != 0) fprintf( fp, "Hit   %d\n",	ch->hitroll);
    if (ch->damroll != 0) fprintf( fp, "Dam   %d\n", ch->damroll);
    fprintf( fp, "ACs %d %d %d %d\n", ch->armor[0],ch->armor[1],ch->armor[2],ch->armor[3]);
    if (ch->wimpy !=0 ) fprintf( fp, "Wimp  %d\n", ch->wimpy);

    if (ch->wimpy_dir > DIR_NONE) fprintf( fp, "WimpDir  %d\n",   ch->wimpy_dir);
    else if (ch->wimpy_dir < DIR_NONE) fprintf( fp, "WimpDir  -2\n");

    fprintf( fp, "Attr %d %d %d %d %d %d %d\n", ch->perm_stat[STAT_STR], ch->perm_stat[STAT_INT], ch->perm_stat[STAT_WIS], ch->perm_stat[STAT_DEX], ch->perm_stat[STAT_CON], ch->perm_stat[STAT_LUC], ch->perm_stat[STAT_CHA]);
    fprintf (fp, "AMod %d %d %d %d %d  %d %d\n", ch->mod_stat[STAT_STR], ch->mod_stat[STAT_INT], ch->mod_stat[STAT_WIS], ch->mod_stat[STAT_DEX], ch->mod_stat[STAT_CON], ch->mod_stat[STAT_LUC], ch->mod_stat[STAT_CHA]);

    if ( IS_NPC(ch) )    {
	fprintf( fp, "Vnum %ld\n",	ch->pIndexData->vnum	);
    }    else    {
	fprintf( fp, "Pass %s~\n",	ch->pcdata->pwd		);
	if (ch->pcdata->bamfin[0] != '\0')	    fprintf( fp, "Bin  %s~\n",	ch->pcdata->bamfin);
	if (ch->pcdata->bamfout[0] != '\0')		fprintf( fp, "Bout %s~\n",	ch->pcdata->bamfout);
	fprintf( fp, "Titl %s~\n",	ch->pcdata->title	);
                fprintf( fp, "Bounty %ld\n",     ch->pcdata->bounty  );

                if ( ch->pcdata->email != NULL
                && ch->pcdata->email[0] != '\0' ) {
                     fprintf( fp, "Email '%s'\n",     ch->pcdata->email       );
                }

                if ( ch->pcdata->url != NULL
                && ch->pcdata->url[0] != '\0' ) {
                     fprintf( fp, "Homepage '%s'\n",     ch->pcdata->url     );
                }

                if ( ch->pcdata->image != NULL
                && ch->pcdata->image[0] != '\0' ) {
                     fprintf( fp, "Image '%s'\n",     ch->pcdata->image      );
                }
 
	fprintf( fp, "ImTi %s~\n",	ch->pcdata->immtitle	);

	if (ch->pcdata->startyear <= 1) {
	    ch->pcdata->startyear = time_info.year;
	    ch->pcdata->startmonth = time_info.month;
	    ch->pcdata->startday = time_info.day;
	}

	fprintf( fp, "Syer %d\n",	ch->pcdata->startyear );
	fprintf( fp, "Smnt %d\n",	ch->pcdata->startmonth );
	fprintf( fp, "Sday %d\n",	ch->pcdata->startday );
	fprintf( fp, "Amod %d\n", 	ch->pcdata->age_mod );
	fprintf( fp, "TSex %d\n",	ch->pcdata->true_sex	);
	fprintf( fp, "LLev %d\n",	ch->pcdata->last_level	);
	fprintf( fp, "HMVP %d %d %d\n", ch->pcdata->perm_hit, ch->pcdata->perm_mana, ch->pcdata->perm_move);
	fprintf( fp, "Cult %s~\n",ch->pcdata->cult);

                if (ch->pcdata->ignore_list) save_ignore(ch, fp);

                if (IS_SET(ch->act, ACT_VAMPIRE)) {
       	    fprintf( fp, "Blood %d\n",ch->pcdata->blood);
	    if (ch->pcdata->bloodline != NULL) fprintf( fp, "Bloodline %s~\n",ch->pcdata->bloodline);
                }

	fprintf( fp, "PKtimer %d\n",ch->pcdata->pk_timer);
	fprintf( fp, "Life %d\n",ch->pcdata->lifespan);
	fprintf( fp, "Mounted %d\n",ch->pcdata->mounted);
	fprintf( fp, "Style %d\n",ch->pcdata->style);
	if (str_cmp(ch->pcdata->spouse,"")) fprintf( fp, "Spouse %s~\n",ch->pcdata->spouse);

                fprintf( fp, "Food  %d\n", ch->condition[COND_FOOD]);
                fprintf( fp, "Drink %d\n", ch->condition[COND_DRINK]);
                if ( ch->condition[COND_FAT] != 0 ) fprintf( fp, "Fat   %d\n", ch->condition[COND_FAT]);
                if ( ch->condition[COND_DRUNK] != 0 ) fprintf( fp, "Drunk %d\n", ch->condition[COND_DRUNK]);

                ch->limb[5]=-1;
                fprintf( fp, "Bstatus %d %d %d %d %d %d\n", ch->limb[0], ch->limb[1], ch->limb[2], ch->limb[3], ch->limb[4], ch->limb[5]);

	fprintf( fp, "Prompt %s~\n", ch->pcdata->prompt   );
	{
		struct alias_data *tmp;
		int counter;
		fprintf( fp, "Alias\n");
		for (counter=0; counter < MAX_ALIAS ; counter++)
			{
			tmp = ch->pcdata->aliases[counter];
			if (tmp != NULL)
				fprintf (fp, "%s~%s~\n", tmp->name, tmp->command_string);
			else
				continue;	
			}
		fprintf (fp, "@~\n");
	}

        /* Save note board status */
        /* Save number of boards in case that number changes */
        fprintf (fp, "Boards       %d ", MAX_BOARD);
        for (i = 0; i < MAX_BOARD; i++)
                fprintf (fp, "%s %ld ", boards[i].short_name,ch->pcdata->last_note[i]);
        fprintf (fp, "\n");
		
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_array[sn].name != NULL && ch->effective->skill[sn] > 0 )
	    {
		fprintf( fp, "Sk %d '%s'\n",
		    ch->effective->skill[sn], skill_array[sn].name );
	    }
	}

    }

    for ( paf = ch->affected; paf != NULL; paf = paf->next ) {
    
	if ( paf->type < SKILL_UNDEFINED 
          || paf->type >= MAX_SKILL
          || paf->afn < AFFECT_UNDEFINED
          || paf->afn >= MAX_AFFECT ) {  
	  continue;
        }
	
        if ( paf->afn != AFFECT_UNDEFINED ) {

          if (paf->location == APPLY_SKILL) { 
            if ( validSkill(paf->skill)
            && paf->skill > 0 ) {
      	      fprintf( fp, "Affect4 '%s' %3d %3d %3d %3d '%s' %lld\n", affect_array[paf->afn].name, paf->level, paf->duration, paf->modifier, paf->location, skill_array[paf->skill].name, paf->bitvector );
            } 
          } else if (paf->location == APPLY_EFFECT) { 
            if ( valid_effect(paf->skill)
            && paf->skill > 0 ) {
      	      fprintf( fp, "Affect5 '%s' %3d %3d %3d %3d '%s' %lld\n", affect_array[paf->afn].name, paf->level, paf->duration, paf->modifier, paf->location, effect_array[paf->skill].name, paf->bitvector );
            } 
          } else {
            fprintf( fp, "Affect3 '%s' %3d %3d %3d %3d %lld\n", affect_array[paf->afn].name, paf->level, paf->duration, paf->modifier, paf->location, paf->bitvector );
          }
       }
    }

   /* Save conversation data... */

    if (ch->cr != NULL) {

      CONV_RECORD *cr;
      CONV_SUB_RECORD *csr;
 
      cr = ch->cr;

      while (cr != NULL) {

        fprintf(fp, "Conv_Main %d\n",
                    cr->conv_id);

        csr = cr->csr;

        while (csr != NULL) {

          if (csr->state != CONV_STATE_NONE) {
            fprintf(fp, "Conv_Sub %d %d\n",
                         csr->sub_id,
                         csr->state);
          }
     
          csr = csr->next;
        } 
  
        cr = cr->next;
      } 

    }

   /* Save deeds... */

    if (ch->deeds != NULL) {
      save_deed(fp, ch->deeds);
    }

   /* Save quests... */

    if (ch->quests != NULL) {
      save_quest(fp, ch->quests);
    }

   /* Write out the languages... */

    fprintf(fp, "Speak '%s' '%s'\n",
      skill_array[ch->speak[SPEAK_PUBLIC]].name,
      skill_array[ch->speak[SPEAK_PRIVATE]].name);

   /* Save society membership info... */

    save_char_society(ch, fp);

   /* Write an End and the end... */

    fprintf( fp, "End\n\n" );

   /* All done... */
 
    return;
}

/* write a pet */
void fwrite_pet( CHAR_DATA *pet, FILE *fp){
AFFECT_DATA *paf;
int i;
    
    fprintf(fp,"#PET\n");
    
    fprintf(fp,"Vnum %ld\n",pet->pIndexData->vnum);
    
    fprintf(fp,"Name %s~\n", pet->name);
    if (pet->short_descr != pet->pIndexData->short_descr)
    	fprintf(fp,"ShD  %s~\n", pet->short_descr);
    if (pet->long_descr != pet->pIndexData->long_descr)
    	fprintf(fp,"LnD  %s~\n", pet->long_descr);
    if (pet->description != pet->pIndexData->description)
    	fprintf(fp,"Desc %s~\n", pet->description);
    if (pet->bio != pet->pIndexData->bio)
    	fprintf(fp,"Bio %s~\n", pet->bio);
    if (pet->race != pet->pIndexData->race)
    	fprintf(fp,"Race %s~\n", race_array[pet->race].name);
    fprintf(fp,"Sex  %d\n", pet->sex);
    if (pet->level != pet->pIndexData->level)
    	fprintf(fp,"Levl %d\n", pet->level);
    fprintf(fp, "HMV  %d %d %d %d %d %d\n",
    	pet->hit, pet->max_hit, pet->mana, pet->max_mana, pet->move, pet->max_move);

    fprintf(fp, "MAX_CURRENCY %d\n", MAX_CURRENCY);
    fprintf(fp, "Cash");
    for (i=0; i < MAX_CURRENCY; i++) fprintf(fp, " %ld ", pet->gold[i]);
    fprintf(fp, "\n");

    if (pet->exp > 0)
    	fprintf(fp, "Exp  %d\n", pet->exp);
    if (pet->act != pet->pIndexData->act)
    	fprintf(fp, "Act  %lld\n", pet->act);
    if (pet->affected_by != pet->pIndexData->affected_by)
    	fprintf(fp, "AfBy %lld\n", pet->affected_by);
    if (pet->comm != 0)
    	fprintf(fp, "Comm %ld\n", pet->comm);
    fprintf(fp,"Pos  %d\n", pet->position = POS_FIGHTING ? POS_STANDING : pet->position);
    if (pet->pos_desc != NULL)  fprintf(fp, "PosDesc %s~\n",	pet->pos_desc	);
    if (pet->saving_throw != 0) fprintf(fp, "Save %d\n", pet->saving_throw);
    fprintf(fp, "Sanity %d\n", pet->sanity);
    if (pet->alignment != pet->pIndexData->alignment)
    	fprintf(fp, "Alig %d\n", pet->alignment);
    if (pet->hitroll != pet->pIndexData->hitroll)
    	fprintf(fp, "Hit  %d\n", pet->hitroll);
    if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
    	fprintf(fp, "Dam  %d\n", pet->damroll);
    fprintf(fp, "ACs  %d %d %d %d\n", pet->armor[0],pet->armor[1],pet->armor[2],pet->armor[3]);

    fprintf(fp, "Attr %d %d %d %d %d %d %d\n", pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT], pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX], pet->perm_stat[STAT_CON], pet->perm_stat[STAT_LUC], pet->perm_stat[STAT_CHA]);
    fprintf(fp, "AMod %d %d %d %d %d %d %d\n", pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT], pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX], pet->mod_stat[STAT_CON], pet->mod_stat[STAT_LUC], pet->mod_stat[STAT_CHA]);

   /* Save hunger and thirst... */
    
    fprintf( fp, "Food  %d\n", pet->condition[COND_FOOD]);
    fprintf( fp, "Drink %d\n", pet->condition[COND_DRINK]);
    if ( pet->condition[COND_FAT] != 0 ) fprintf( fp, "Fat   %d\n", pet->condition[COND_FAT]);
    if ( pet->condition[COND_DRUNK] != 0 ) fprintf( fp, "Drunk %d\n", pet->condition[COND_DRUNK]);
    fprintf( fp, "Bstatus %d %d %d %d %d %d\n", pet->limb[0], pet->limb[1], pet->limb[2], pet->limb[3], pet->limb[4], pet->limb[5]);
    pet->limb[5] = -1;

   /* Save affects.... */

    for ( paf = pet->affected; paf != NULL; paf = paf->next ) {
    
	if ( paf->type < SKILL_UNDEFINED 
          || paf->type >= MAX_SKILL
          || paf->afn < AFFECT_UNDEFINED
          || paf->afn >= MAX_AFFECT ) {  
	  continue;
        }
    	    
        if (paf->afn != AFFECT_UNDEFINED) { 
         
          if (paf->location == APPLY_SKILL) { 
            if ( validSkill(paf->skill)
              && paf->skill > 0 ) {
    	      fprintf( fp, "Affect4 '%s' %3d %3d %3d %3d '%s' %lld\n", affect_array[paf->afn].name, paf->level,  paf->duration, paf->modifier, paf->location, skill_array[paf->skill].name, paf->bitvector );
            } 
          } else if (paf->location == APPLY_EFFECT) { 
            if ( valid_effect(paf->skill)
            && paf->skill > 0 ) {
      	      fprintf( fp, "Affect5 '%s' %3d %3d %3d %3d '%s' %lld\n", affect_array[paf->afn].name, paf->level, paf->duration, paf->modifier, paf->location, effect_array[paf->skill].name, paf->bitvector );
            }
         } else {
            fprintf( fp, "Affect3 '%s' %3d %3d %3d %3d %lld\n",affect_array[paf->afn].name,paf->level, paf->duration, paf->modifier, paf->location,paf->bitvector );
         }
       }
    }

   /* Save quests... */

    if (pet->quests != NULL) save_quest(fp, pet->quests);
    
    fprintf(fp,"End\n");

    return;
}
    
/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest ) {
EXTRA_DESCR_DATA *ed;
AFFECT_DATA *paf;
char buf[MAX_STRING_LENGTH];
 
    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->next_content != NULL ) fwrite_obj( ch, obj->next_content, fp, iNest );
    
    if (obj->trans_char != NULL) {
         affect_strip_afn(obj->trans_char, gafn_morf);
         REMOVE_BIT(obj->trans_char->affected_by, AFF_MORF );
         if(obj->carried_by != NULL) obj->trans_char->in_room = obj->carried_by->in_room;
         obj->trans_char->trans_obj = NULL;
         return;
    }
    if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)) return;

    if (IS_NPC(ch) && ch->master && ch->leader) fprintf( fp, "#MO\n" );
    else fprintf( fp, "#O\n" );

    fprintf( fp, "Vnum %ld\n",   obj->pIndexData->vnum        );
    if ( obj->orig_index != 0) fprintf( fp, "OrigIndex   %ld\n", obj->orig_index);

    if (!obj->pIndexData->new_format) fprintf( fp, "Oldstyle\n");
    
    if (obj->enchanted) fprintf( fp,"Enchanted\n");
    
    fprintf( fp, "Nest %d\n",	iNest	  	     );
    fprintf( fp, "Size   %d\n", obj->size				);

   /* these data are only used if they do not match the defaults */

    fprintf( fp, "Name %s~\n",	obj->name);
    fprintf( fp, "ShD  %s~\n", obj->short_descr);
    fprintf( fp, "Desc %s~\n", obj->description);
    if (obj->owner != NULL) fprintf( fp, "Owner %s~\n", obj->owner);
    fprintf( fp, "ExtF %lld\n",	obj->extra_flags);
    fprintf( fp, "WeaF %d\n", obj->wear_flags);
    fprintf( fp, "Ityp %d\n", obj->item_type);
    fprintf( fp, "Wt   %d\n",	obj->weight);


   /* variable data */

    fprintf( fp, "Wear %d\n", obj->wear_loc);

    if (obj->level != 0) fprintf( fp, "Lev  %d\n",	obj->level);
    if (obj->timer != 0) fprintf( fp, "Time %d\n", obj->timer);
    fprintf( fp, "Cost %d\n",	obj->cost);
    fprintf( fp, "Cond %d\n", obj->condition);

    fprintf( fp, "Val  %ld %ld %ld %ld %ld\n", obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4]);
    fprintf( fp, "ValOrig  %ld %ld %ld %ld %ld\n", obj->valueorig[0], obj->valueorig[1], obj->valueorig[2], obj->valueorig[3], obj->valueorig[4]);

    switch ( obj->item_type ) {
    case ITEM_POTION:
    case ITEM_SCROLL:
	if ( obj->value[1] > 0 ) {
	    fprintf( fp, "Spell 1 '%s'\n", effect_array[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 ) {
	    fprintf( fp, "Spell 2 '%s'\n", effect_array[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 ) {
	    fprintf( fp, "Spell 3 '%s'\n", effect_array[obj->value[3]].name );
	}

	break;

    case ITEM_PILL:
    case ITEM_STAFF:
    case ITEM_WAND:
	if ( obj->value[3] > 0 ) {
	    fprintf( fp, "Spell 3 '%s'\n", effect_array[obj->value[3]].name );
	}

	break;
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next ) {

        if (paf->afn != AFFECT_UNDEFINED) {
          if (paf->location == APPLY_SKILL) { 
            if ( validSkill(paf->skill)
              && paf->skill > 0 ) {
              fprintf( fp, "Affect4 '%s' %3d %3d %3d %3d '%s' %lld\n", affect_array[paf->afn].name, paf->level, paf->duration, paf->modifier, paf->location, skill_array[paf->skill].name, paf->bitvector );
            } 
          } else if (paf->location == APPLY_EFFECT) { 
            if ( valid_effect(paf->skill)
            && paf->skill > 0 ) {
      	      fprintf( fp, "Affect5 '%s' %3d %3d %3d %3d '%s' %lld\n", affect_array[paf->afn].name, paf->level, paf->duration, paf->modifier, paf->location, effect_array[paf->skill].name, paf->bitvector );
            }
         } else {
            fprintf( fp, "Affect3 '%s' %3d %3d %3d %3d %lld\n", affect_array[paf->afn].name,paf->level, paf->duration, paf->modifier, paf->location, paf->bitvector );
         }
       }
    }

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )    {
	fprintf( fp, "ExDe %s~ %s~\n", ed->keyword, ed->description );
    }

    fprintf( fp, "ZMask %s\n", fwrite_flag(obj->zmask, buf));
    fprintf( fp, "End\n\n" );
    if ( obj->contains != NULL ) fwrite_obj( ch, obj->contains, fp, iNest + 1 );
    
    return;
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, char *name ) {
    static PC_DATA pcdata_zero;
    char strsave[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *ch;
    FILE *fp;
    bool found;
    int stat;
    OBJ_DATA *wield; 
    int sn = 0; 
    int i;

    PROF_DATA *prof = NULL;
    int base = 0;

   /* Get memory for the character... */

    if ( char_free == NULL ) {
	ch				= (CHAR_DATA *) alloc_perm( sizeof(*ch) );
    } else {
	ch				= char_free;
	char_free			= char_free->next;
    }

    clear_char( ch );

   /* Get memory for the characters PC_Data... */

    if ( pcdata_free == NULL )  {
	ch->pcdata			= (PC_DATA *) alloc_perm( sizeof(*ch->pcdata) );
    }  else  {
	ch->pcdata			= pcdata_free;
	pcdata_free			= pcdata_free->next;
    }
    *ch->pcdata				= pcdata_zero;

    ch->pcdata->logon = current_time;
    ch->pcdata->startyear		= time_info.year;
    ch->pcdata->startmonth		= time_info.month;
    ch->pcdata->startday		= time_info.day;

   /* Get a memory block for the characters effective skills... */

    ch->effective = getSkillData();

   /* Work out the mobs true_name... */

   /* Pointer for uniqueness
      Name incase they logoff and another player gets the same char_data */

    sprintf(buf, "@p%p_%s", ch, name);

    ch->true_name = str_dup(buf);

   /* Initialize a few things... */

    d->character			= ch;
    ch->desc				= d;

    ch->next_in_room			= NULL;
    ch->master				= NULL;
    ch->leader				= NULL;
    ch->fighting				= NULL;
    ch->huntedBy 				= NULL;
    ch->distractedBy			= NULL;
    ch->prey				= NULL;
    ch->reply				= NULL;
    ch->pet				= NULL;

    ch->name				= str_dup( name );

    ch->poly_name				= str_dup( "" );
    ch->description			= str_dup ("");
    ch->bio				= str_dup ("");
    ch->short_descr			= str_dup (name);       /* Mik */
    ch->long_descr			= str_dup ("");
    ch->description_orig			= str_dup ("");
    ch->short_descr_orig			= str_dup ("");
    ch->long_descr_orig			= str_dup ("");
    ch->version				= 0;
    ch->race				= race_lookup("human");
    ch->race_orig=0;
    ch->affected_by			= 0;
    ch->artifacted_by			= 0;
    ch->act				= 0;
    ch->plr				= PLR_NOSUMMON;
    ch->pcdata->board                   		= &boards[DEFAULT_BOARD];
    ch->comm				= COMM_COMBINE  | COMM_PROMPT;
    ch->notify				= 0;
    ch->invis_level				= 0;
    ch->cloak_level			= 0;
    ch->practice				= 0;
    ch->train				= 0;
    ch->sanity				= 0;
    ch->hitroll				= 0;
    ch->damroll				= 0;
    ch->recall_perm			= ROOM_VNUM_LIMBO;
    ch->recall_temp			= 0;
    ch->wimpy			 	= 0;
    ch->wimpy_dir		 		= DIR_NONE;
    ch->quitting				= FALSE;
    ch->saving_throw			= 0;
    ch->limb[0]                         		= 0;
    ch->limb[1]                         		= 0;
    ch->limb[2]                         		= 0;
    ch->limb[3]                         		= 0;
    ch->limb[4]                         		= 0;
    ch->limb[5]                         		= 0;
    ch->nightmare				= FALSE;
    ch->on_furniture			= NULL;
    ch->trans_obj				= NULL;
    ch->pcdata->questgiver			= NULL;
    ch->pcdata->questmob			= 0;
    ch->pcdata->questobj			= 0;
    ch->pcdata->questpoints		= 0;
    ch->pcdata->nextquest			= 0;
    ch->pcdata->countdown		= 0;
    ch->pcdata->questplr			= str_dup ("");

    ch->alignment				= 0;
    ch->group				= 0;	
    ch->pcdata->ignore_list 			= NULL;
    ch->pcdata->pwd_tries			= 0;
    ch->pcdata->confirm_delete		= FALSE;
    ch->pcdata->pwd			= str_dup( "" );
    ch->pcdata->bamfin			= str_dup( "" );
    ch->pcdata->bamfout			= str_dup( "" );
    ch->pcdata->title			= str_dup( "" );
    ch->pcdata->bounty                      		= 0;
    ch->pcdata->email			= NULL;
    ch->pcdata->image			= NULL;
    ch->pcdata->url			= NULL;
    ch->pcdata->immtitle			= str_dup( "SuperHero" );
    ch->pcdata->answer 			= -1;
    for (i =0; i < MAX_CURRENCY; i++) ch->gold[i] = 0;

    for (stat =0; stat < MAX_STATS; stat++) {
	ch->perm_stat[stat]		= 51;
    }

    for (stat =0; stat < TRAIN_MAX; stat++) {
	ch->training[stat]		= 0;
    }

    for (stat =0; stat < MOB_MEMORY; stat++) {
	ch->memory[stat]		= NULL;
    }

    ch->pcdata->startyear = 0;
    ch->pcdata->startmonth = 0;
    ch->pcdata->startday = 0;
    ch->pcdata->age_mod =0;
	{
		int counter;
		for (counter = 0 ; counter < MAX_ALIAS ; counter ++)
			ch->pcdata->aliases[counter] = NULL;
	}
    ch->pcdata->has_alias		= FALSE;
    ch->pcdata->perm_hit		= 0;
    ch->pcdata->perm_mana	= 0;
    ch->pcdata->perm_move	= 0;
    ch->pcdata->true_sex		= 0;
    ch->pcdata->last_level		= 0;
    ch->pcdata->pk_timer		= 0;
    ch->pcdata->native		= strdup(mud.name);
    ch->pcdata->cult 		= strdup("Marduk"); 
    ch->pcdata->spouse 		= strdup(""); 
    ch->pcdata->blood 		= 0; 
    ch->pcdata->help_topic 		= NULL;
    ch->pcdata->help_count	= 0;
    ch->pcdata->bloodline 		= NULL;
    ch->pcdata->macro 		= FALSE;
    ch->pcdata->macro_script 	= 0;
    ch->pcdata->macro_line 	= 0;
    ch->pcdata->macro_delay 	= 0;
    ch->pcdata->lifespan 		= 0; 
    ch->pcdata->style 		= 0; 
    ch->pcdata->mounted 		= 0; 
    ch->pcdata->security                	= 0;
    ch->pcdata->prompt 		= strdup ("{G({W%h/%Hhp %m/%Mmn %v/%Vmv{G){x");
    ch->pcdata->home_room 	= ROOM_VNUM_CHAT;
    ch->pcdata->store_char 		= NULL;
    ch->pcdata->store_obj 		= NULL;
    ch->pcdata->store_number[0]	= 0;
    ch->pcdata->store_number[1]	= 0;

    ch->huntedBy                		= NULL; 		
    ch->prey                   		= NULL;  		

    ch->quaffLast			= current_time;         

    ch->gun_state[0] 		= GUN_NO_AMMO;         
    ch->gun_state[1]            		= GUN_NO_AMMO;         

    ch->pcdata->divinity			= DIV_NEWBIE;
    ch->pcdata->moddiv			= DIV_NEWBIE;
    ch->pcdata->authority			= 0;

    ch->profs = NULL;
    ch->deeds = NULL;
    ch->quests = NULL;

    ch->cr = NULL;

    ch->triggers = get_trs();

    ch->nature = 0;
    ch->fright = 0;
    ch->ridden=0;
    

    ch->speak[SPEAK_PUBLIC] 	= gsn_english;
    ch->speak[SPEAK_PRIVATE] 	= gsn_english;

    ch->memberships = NULL;

    ch->position 		= POS_STANDING;
    ch->pos_desc		= NULL;
    ch->activity		= ACV_NONE;
    ch->acv_desc		= NULL;

    ch->tracks 			= NULL;
    ch->were_type = 0;

    found = FALSE;
    fclose( fpReserve );
    
    #if defined(unix)
    /* decompress if .gz file exists */
    sprintf( strsave, "%s%s%s", PLAYER_DIR, capitalize(name),".gz");
    if ((fp = fopen(strsave, "r")) != NULL) {
	fclose(fp);
	sprintf(buf,"gzip -dfq %s",strsave);
	system(buf);
    }
    #endif

    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( name ) );
    if ((fp = fopen( strsave, "r" ) ) != NULL )  {
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
	for ( ; ; )	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_char_obj: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );

	    if      ( !str_cmp( word, "PLAYER" ) ) fread_char ( ch, fp );
	    else if ( !str_cmp( word, "OBJECT" ) ) fread_obj  (ch, fp, NULL);
	    else if ( !str_cmp( word, "O"      ) ) fread_obj  ( ch, fp, NULL);
	    else if ( !str_cmp( word, "MO"      ) ) fread_obj  (ch->pet, fp, NULL);
	    else if ( !str_cmp( word, "PET"    ) ) fread_pet  ( ch, fp );
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else {
		bug( "Load_char_obj: bad section.", 0 );
		break;
	    }

	}

	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );

   /* Bail out if not found... */

    if (!found) return FALSE;

   /* default race */

    if (ch->race == 0) {
      ch->race = race_lookup("human");
    }

    ch->size = race_array[ch->race].size;

    ch->dam_type = DAM_BASH; 
    ch->atk_type = WDT_PUNCH;

    if (IS_AFFECTED(ch, AFF_POLY)) undo_mask(ch, TRUE);

    ch->affected_by 	= ch->affected_by | race_array[ch->race].aff;
    ch->imm_flags   	= ch->imm_flags   | race_array[ch->race].imm;
    ch->envimm_flags   	= ch->envimm_flags   | race_array[ch->race].envimm;
    ch->res_flags   		= ch->res_flags   | race_array[ch->race].res;
    ch->vuln_flags  	= ch->vuln_flags  | race_array[ch->race].vuln;

     if (IS_SET(ch->act,ACT_VAMPIRE)
     && !IS_SET(ch->vuln_flags,VULN_WOOD)) {
             SET_BIT(ch->vuln_flags,VULN_WOOD);
     }

     if (IS_SET(ch->act,ACT_WERE)
     && !IS_SET(ch->vuln_flags,VULN_SILVER)) {
             SET_BIT(ch->vuln_flags,VULN_SILVER);
     }

    ch->form	    = race_array[ch->race].form;
    ch->parts	    = race_array[ch->race].parts;
	
   /* fix levels */

    if (ch->version < 3 && ch->level > 35)    {
	switch (ch->level)	 {
	    case(40) : ch->level = 60;	break;  /* imp -> imp */
	    case(39) : ch->level = 58; 	break;	/* god -> supreme */
	    case(38) : ch->level = 56;  break;	/* deity -> god */
	    case(37) : ch->level = 53;  break;	/* angel -> demigod */
	}
    }

   /* Fix ACT/PLR split... */

    if (ch->version < 5) {

      ch->version = 5;

      ch->plr = ch->act;
      ch->act = 0;
    }

   /* Apply stat scaling... */

    if (ch->version < 6) {
      ch->version = 6;
      ch->perm_stat[STAT_STR] = 4 * ch->perm_stat[STAT_STR] + dice(1,4);
      ch->perm_stat[STAT_INT] = 4 * ch->perm_stat[STAT_INT] + dice(1,4);
      ch->perm_stat[STAT_WIS] = 4 * ch->perm_stat[STAT_WIS] + dice(1,4);
      ch->perm_stat[STAT_DEX] = 4 * ch->perm_stat[STAT_DEX] + dice(1,4);
      ch->perm_stat[STAT_CON] = 4 * ch->perm_stat[STAT_CON] + dice(1,4);

      ch->perm_stat[STAT_STR] = URANGE(STAT_MINIMUM, ch->perm_stat[STAT_STR], STAT_MAXIMUM);
      ch->perm_stat[STAT_INT] =  URANGE(STAT_MINIMUM, ch->perm_stat[STAT_INT], STAT_MAXIMUM);
      ch->perm_stat[STAT_WIS] = URANGE(STAT_MINIMUM, ch->perm_stat[STAT_WIS], STAT_MAXIMUM);
      ch->perm_stat[STAT_DEX] = URANGE(STAT_MINIMUM, ch->perm_stat[STAT_DEX], STAT_MAXIMUM);
      ch->perm_stat[STAT_CON] = URANGE(STAT_MINIMUM, ch->perm_stat[STAT_CON], STAT_MAXIMUM);
    }

   /* Fix sanity and training... */

    if (ch->version < 7) {
      ch->version = 7; 
      ch->sanity = ch->perm_stat[STAT_WIS];
      ch->train  = ch->level; 
    }

    if (ch->version < 8) {
      ch->version = 8; 
      if (ch->profs) {
              ch->perm_stat[STAT_LUC] = 10 + dice(4,4);
              ch->perm_stat[STAT_LUC] += 4 * dice(ch->profs->profession->stats[STAT_LUC], 8);
              ch->perm_stat[STAT_LUC] += 6 * (ch->profs->profession->stats[STAT_LUC]-1);
              if (ch->perm_stat[STAT_LUC] > 65) ch->perm_stat[STAT_LUC] = (ch->perm_stat[STAT_LUC] - 65)/2 + 65;

              ch->perm_stat[STAT_CHA] = 10 + dice(4,4);
              ch->perm_stat[STAT_CHA] += 4 * dice(ch->profs->profession->stats[STAT_CHA], 8);
              ch->perm_stat[STAT_CHA] += 6 * (ch->profs->profession->stats[STAT_CHA]-1);
              if (ch->perm_stat[STAT_CHA] > 65) ch->perm_stat[STAT_CHA] = (ch->perm_stat[STAT_CHA] - 65)/2 + 65;
      } else {
              ch->perm_stat[STAT_LUC] = number_range(30, 70);
              ch->perm_stat[STAT_CHA] = number_range(30, 70);
      }
    }



   /* If the character comes back wielding a gun, we'll be nice and 
      reload it for them... */

    wield = get_eq_char( ch, WEAR_WIELD);

    if ( (wield != NULL)
      && (wield->item_type == ITEM_WEAPON) 
      && ( (wield->value[0] == WEAPON_HANDGUN) 
        || (wield->value[0] == WEAPON_BOW)
        || (wield->value[0] == WEAPON_GUN)
        || (wield->value[0] == WEAPON_SMG))) reload_gun(ch,wield, 0, FALSE);
    
  /* ...same for the dual weapon... */

    wield = get_eq_char( ch, WEAR_WIELD2);

    if ( (wield != NULL)
      && (wield->item_type == ITEM_WEAPON) 
      && ( (wield->value[0] == WEAPON_HANDGUN) 
        || (wield->value[0] == WEAPON_BOW)
        || (wield->value[0] == WEAPON_GUN)
        || (wield->value[0] == WEAPON_SMG))) reload_gun(ch,wield, 1, FALSE);
    

   /* Fix spell casters... */

    if (ch->version < 5) {
      switch (ch->pc_class) {
        case CLASS_MAGE:
        case CLASS_GOOD_PRIEST:
        case CLASS_CHAOSMAGE:
        case CLASS_EVIL_PRIEST:
          if (ch->effective->skill[gsn_spell_casting] == 0) {
             setSkill(ch->effective, gsn_spell_casting, 
                         get_base_skill(gsn_spell_casting, ch) + ch->level );
          }
          break;

        default:
          break;
      }

     /* Fix monks... */

      if ( ch->pc_class == CLASS_MONK
        && ch->effective->skill[gsn_martial_arts] == 0 ) {
        ch->effective->skill[gsn_martial_arts] = 
                           get_base_skill(gsn_martial_arts, ch) + ch->level;
      }
    }

   /* Fix English... */

    if ( ch->effective->skill[gsn_english] < 50 ) {
      ch->effective->skill[gsn_english] = 51; 
    }

   /* Fix Dreaming... */

    if ( ch->effective->skill[gsn_dreaming] < 1 ) {
      ch->effective->skill[gsn_dreaming] = 1; 
    }

  if ( IS_SET(ch->act, ACT_DREAM_NATIVE)) {
      ch->effective->skill[gsn_dreaming] = 0; 
  }

   /* Fix xps... */

    if (ch->exp < exp_for_level(ch->level)) {
      ch->exp = exp_for_level(ch->level) + dice(5, 100);
    }

    if (ch->exp > exp_for_next_level(ch)) {
      ch->exp = exp_for_next_level(ch) - 1;
    }

   /* Fix pet xps... */

    if ( ch->pet != NULL ) {
      if (ch->pet->exp < exp_for_level(ch->pet->level)) {
        ch->pet->exp = exp_for_level(ch->pet->level) + dice(5, 100);
      }

      if (ch->pet->exp > exp_for_next_level(ch->pet)) {
        ch->pet->exp = exp_for_next_level(ch->pet) - 1;
      }
    }

   /* Class to profession translation... */

    if (ch->profs == NULL) {

     /* Find the corresponding profession... */

      switch (ch->pc_class) {
        case CLASS_MAGE:
          prof = get_profession(PROF_MAGE);
          break;
        case CLASS_GOOD_PRIEST:
          prof = get_profession(PROF_GOOD_PRIEST);
          break;
        case CLASS_THIEF:
          prof = get_profession(PROF_THIEF);
          break;
        case CLASS_WARRIOR:
          prof = get_profession(PROF_WARRIOR);
          break;
        case CLASS_CHAOSMAGE:
          prof = get_profession(PROF_CHAOSMAGE);
          break;
        case CLASS_MONK:
          prof = get_profession(PROF_MONK);
          break;
        case CLASS_EVIL_PRIEST:
          prof = get_profession(PROF_EVIL_PRIEST);
          break;
        default:
          break;
      }

     /* If the mapping fails, we kill everything... */

      if (prof == NULL) {
        bug("Unable to find profession for class %d!", ch->pc_class);
        exit(1);
      } 

     /* Create an LPROF for the player... */

      ch->profs = get_lprof();

      if (ch->profs == NULL) {
        bug("Allocate for new lprof failed!", 0); 
        exit(1);
      }

     /* Set it up... */

      ch->profs->profession = prof; 
      ch->profs->level = ch->level;
      ch->profs->next = NULL;

    }
 
   /* Ensure minimum scores in base skills... */

    prof = ch->profs->profession;

    for(sn = 0; sn < MAX_SKILL; sn++) {

      if (!skill_array[sn].loaded) {
        break;
      }

      if (prof->levels->skill[sn] == 0) {
        base = get_base_skill(sn, ch) + ch->level;
        if (ch->effective->skill[sn] < base) {
          setSkill(ch->effective, sn, base); 
        }
      }
    }

   /* Validate/fix languages... */
   
    sn = ch->speak[SPEAK_PUBLIC];

    if ( sn <= 1
      || !IS_SET(skill_array[sn].group, SKILL_GROUP_LANG)) {
      ch->speak[SPEAK_PUBLIC] = gsn_english;
    }

    sn = ch->speak[SPEAK_PRIVATE];

    if ( sn <= 1
      || !IS_SET(skill_array[sn].group, SKILL_GROUP_LANG)) {
      ch->speak[SPEAK_PRIVATE] = gsn_english;
    }

   /* Create new memberships... */

    default_memberships(ch);

   /* Reconcile membership data... */

    check_memberships(ch);

   /* Set activity... */

    if (IS_AFFECTED(ch, AFF_HIDE)) {
      set_activity(ch, ch->position, ch->pos_desc,
                                      ACV_HIDING, "skulking in the shadows.");
    }

    if (IS_AFFECTED(ch, AFF_SNEAK)) {
      set_activity(ch, ch->position, ch->pos_desc,
                                      ACV_SNEAKING, "sneaking around.");
    }

     if (IS_AFFECTED(ch, AFF_MORF)
     || is_affected(ch, gafn_morf)) {
          affect_strip_afn( ch, gafn_morf);
          REMOVE_BIT( ch->affected_by, AFF_MORF );
    }

   /* Fix condition effects... */

    if ( ch->condition[COND_FOOD] > 0 
      && is_affected(ch, gafn_starvation) ) {
      affect_strip_afn(ch, gafn_starvation);
    }

    if ( ch->condition[COND_DRINK] > 0 
      && is_affected(ch, gafn_dehydration) ) {
      affect_strip_afn(ch, gafn_dehydration);
    }

    if ( ch->condition[COND_DRUNK] <= 0 
      && is_affected(ch, gafn_intoxication) ) {
      ch->condition[COND_DRUNK] = 0;
      affect_strip_afn(ch, gafn_intoxication);
    }

    if ( ch->condition[COND_FAT] <= 0 
      && is_affected(ch, gafn_obesity) ) {
      ch->condition[COND_FAT] = 0;
      affect_strip_afn(ch, gafn_obesity);
    }

    if ( ch->limb[0] < 4 
      && is_affected(ch, gafn_head_wound) ) {
      affect_strip_afn(ch, gafn_head_wound);
    }
    if ( ch->limb[1] < 5 
      && is_affected(ch, gafn_body_wound) ) {
      affect_strip_afn(ch, gafn_body_wound);
    }
    if ( ch->limb[2] < 4 
      && ch->limb[3] < 4
      && is_affected(ch, gafn_arm_wound) ) {
      affect_strip_afn(ch, gafn_arm_wound);
    }
    if ( ch->limb[4] < 4 
      && is_affected(ch, gafn_leg_wound) ) {
      affect_strip_afn(ch, gafn_leg_wound);
    }


   /* All done and ready to play... */

    return found;
}



/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void fread_char( CHAR_DATA *ch, FILE *fp ) {
char buf[MAX_STRING_LENGTH];
const char *word;
char *tword;
bool fMatch;
int i;
int max_c =0;

    CONV_RECORD *cr;
    CONV_SUB_RECORD *csr;

    DEED *new_deed;
    QUEST *new_quest;

    MEMBERSHIP *memb;

    cr = NULL;
    csr = NULL;

    word = NULL;
    fMatch = FALSE;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    KEY( "Act",		ch->act,		fread_number_long_long(fp) );
	    KEY( "AffectedBy",	ch->affected_by, fread_number_long_long( fp ) );
	    KEY( "AfBy",	ch->affected_by,		fread_number_long_long( fp ) );
	    KEY( "Alignment",	ch->alignment,		fread_number( fp ) );
	    KEY( "Alig",	ch->alignment,		fread_number( fp ) );
	    KEY( "Amod", 	ch->pcdata->age_mod,   fread_number( fp ) );
	    KEY( "Auth",	ch->pcdata->authority,    	fread_number( fp ) );
		
		if (!str_cmp( word, "Alias"))
			{
			struct alias_data *tmp;
			int counter=0;
			char *word;
			bool done = FALSE;
		
			while (!done)
				{
				word=fread_string(fp);
				if (!str_cmp(word, "@"))
					done = TRUE;
				else
					{
					tmp = (struct alias_data *) malloc (sizeof (struct alias_data));
					tmp->name = strdup(word);
					tmp->command_string = fread_string (fp);
					ch->pcdata->aliases[counter]=tmp;
					ch->pcdata->has_alias = TRUE;
					counter++;
					}
				free_string(word);
				}
			fMatch=TRUE;
			break;
			}
		

	    if (!str_cmp( word, "AC") || !str_cmp(word,"Armor"))
	    {
		fread_to_eol(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word,"ACs"))
	    {
		int i;

		for (i = 0; i < 4; i++)
		    ch->armor[i] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Affect" ) || !str_cmp( word, "Aff" ) 
	    ||   !str_cmp( word, "AffD") || !str_cmp (word, "AffnewD")
	    ||   !str_cmp( word, "Affect3" ) 
	    ||   !str_cmp( word, "Affect4" )
                    || !str_cmp(word, "Affect5")) {
		AFFECT_DATA *paf;
                                tword=strdup(word);

                paf = new_affect();

                paf->type = SKILL_UNDEFINED;
	paf->afn = AFFECT_UNDEFINED;

                if ( !str_cmp(tword, "Affect3") 
                || !str_cmp(tword, "Affect4") 
                || !str_cmp(tword, "Affect5")) {
                    int afn;
                    char *aff_name;
   	    aff_name = fread_word(fp);
	    afn = get_affect_afn(aff_name);
                    if (afn != AFFECT_UNDEFINED) {
                        paf->afn = afn;
                    } else {
                        bug("Fread_char: unknown affect", 0);  
                    } 
                } else if (!str_cmp(word,"AffD") || !str_cmp(word, "AffnewD"))  {
	     int sn;
	     sn = get_effect_efn(fread_word(fp));
	     if (sn < 0) bug("Fread_char: unknown effect.",0);
                     else {
	         paf->type = sn;
                         paf->afn = get_affect_afn_for_effect(sn);
                     }
	} else  paf->type	= fread_number( fp );

	if (ch->version == 0)   paf->level = ch->level;
	else paf->level	= fread_number( fp );
	
	paf->duration	= fread_number( fp );
	paf->modifier	= fread_number( fp );
	paf->location	= fread_number( fp );

                if (!str_cmp(tword, "Affect4")) {
                  paf->skill	= get_skill_sn(fread_word(fp));
                } else if (!str_cmp(tword, "Affect5")) {
                  paf->skill	= get_effect_efn(fread_word(fp));
                } else {
                  paf->skill	= SKILL_UNDEFINED;
                }

	paf->bitvector	= fread_number_long_long( fp );

	if (!str_cmp(word, "AffnewD")) /* Zeran - compatibility only */
			{
			int os;
			os = fread_number (fp);
			}
		paf->next	= ch->affected;
		ch->affected	= paf;
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "AttrMod"  ) || !str_cmp(word,"AMod"))	    {
		int stat;
                                if (ch->version < 8) {
     		    for (stat = 0; stat < MAX_STATS-2; stat ++) ch->mod_stat[stat] = fread_number(fp);
                                } else {
     		    for (stat = 0; stat < MAX_STATS; stat ++) ch->mod_stat[stat] = fread_number(fp);
                                }
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "AttrPerm" ) || !str_cmp(word,"Attr"))	    {
		int stat;
                                if (ch->version < 8) {
   		    for (stat = 0; stat < MAX_STATS-2; stat++) ch->perm_stat[stat] = fread_number(fp);
                                } else {
   		    for (stat = 0; stat < MAX_STATS; stat++) ch->perm_stat[stat] = fread_number(fp);
                                }
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'B':
	    KEY( "Bamfin",	ch->pcdata->bamfin,	fread_string( fp ) );
	    KEY( "Bamfout",	ch->pcdata->bamfout,	fread_string( fp ) );
	    KEY( "Bio",		ch->bio,		fread_string( fp ) );
	    KEY( "Bin",		ch->pcdata->bamfin,	fread_string( fp ) );
                    KEY( "Blood",     ch->pcdata->blood, fread_number(fp ) );
  	    KEY( "Bloodline",	ch->pcdata->bloodline,		fread_string( fp ) );
                    KEY( "Bounty",  ch->pcdata->bounty,     fread_number( fp ) );
            if (!str_cmp(word, "Boards" )) {
                int i,num = fread_number (fp);
                    /* number of boards saved */
                char *boardname;
                for (; num ; num-- ) /* for each of the board saved */
                {
                   boardname = fread_word (fp);
                   i = board_lookup (boardname);
                         /* find board number */

                   if (i == BOARD_NOTFOUND)
                         /* Does board still exist ? */
                   {
                      sprintf (buf, "fread_char: %s had unknown board name: %s. Skipped.", ch->name, boardname);
                      log_string (buf);
                      fread_number (fp);
                           /* read last_note and skip info */
                    }
                    else /* Save it */
                    ch->pcdata->last_note[i] = fread_number (fp);
               }        /* for */

               fMatch = TRUE;
            } /* Boards */

	    KEY( "Bout",	ch->pcdata->bamfout,	fread_string( fp ) );
	    if (!str_cmp (word, "Bstatus")) {	
	          ch->limb[0] = fread_number( fp );
	          ch->limb[1] = fread_number( fp );
	          ch->limb[2] = fread_number( fp );
	          ch->limb[3] = fread_number( fp );
	          ch->limb[4] = fread_number( fp );
	          ch->limb[5] = fread_number( fp );
                          fMatch = TRUE;
                    }
	    break;

	case 'C':
                 KEY( "Cloak",	ch->cloak_level,	fread_number(fp) );
                 KEY( "Class",	ch->pc_class,		fread_number( fp ) );
	 KEY( "Cla",		ch->pc_class,		fread_number( fp ) );
	 KEY( "Cult",	ch->pcdata->cult,		fread_string( fp ) );

	    if (!str_cmp(word, "Cash")) {
	         fMatch=TRUE;
                         if (max_c > 0) {
                             for (i = 0; i < max_c; i++) ch->gold[i] = fread_number(fp);
                         }
                    }

	    if (!str_cmp (word, "Clrank"))   {	
		int dummy;
		dummy=fread_number(fp);
		fMatch=TRUE;
		break;
	    }	
			
	    if (!str_cmp (word, "Clan"))  {	
		char *dummy;
		dummy=fread_string(fp);
		fMatch=TRUE;
		break;
	    }	
			
	    if ( !str_cmp( word, "Condition" ) || !str_cmp(word,"Cond"))  {

              /* Discard old values so all start witha full belly */

		fread_number( fp );
		fread_number( fp );
		fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    KEY("Comm",		ch->comm,		fread_number( fp ) ); 

           /* Conv_Main */

            if (!str_cmp(word, "Conv_Main")) {
               fMatch = TRUE;

               cr = get_conv_record();

               if (cr == NULL) {
                 bug("Get_cond_record failed!", 0);
                 exit(1);
               }

               cr->conv_id = fread_number(fp);

               cr->next = ch->cr;
               ch->cr = cr;
 
            } 
          
           /* Conv_Sub */

            if (!str_cmp(word, "Conv_Sub")) {
               fMatch = TRUE;

               csr = get_conv_sub_record();

               if (csr == NULL) {
                 bug("Get_cond_sub_record failed!", 0);
                 exit(1);
               }

               csr->sub_id = fread_number(fp);
               csr->state = fread_number(fp);		

               if (cr == NULL) {
                 bug("Conv_Sub before Conv_Main!", 0);
               } else {

                 csr->next = cr->csr;
                 cr->csr = csr;
               }
 
            } 
          
	    break;

	case 'D':
	    KEY( "Damroll",	ch->damroll,		fread_number( fp ) );
	    KEY( "Dam",		ch->damroll,		fread_number( fp ) );
	    KEY( "Description",	ch->description,	fread_string( fp ) );
	    KEY( "Desc",	ch->description,	fread_string( fp ) );
	    KEY( "DescOrig",	ch->description_orig,	fread_string( fp ) );
	    KEY( "Divinity",	ch->pcdata->divinity,    	fread_number( fp ) );
	    KEY( "Drunk",	ch->condition[COND_DRUNK],
							fread_number( fp ) );
	    KEY( "Drink",	ch->condition[COND_DRINK],
							fread_number( fp ) );
 
           /* Deeds */

            if (!str_cmp(word, "Deed")) { 
              fMatch = TRUE;

              new_deed = read_deed(fp);

              new_deed->next = ch->deeds;
              ch->deeds = new_deed; 

              break;
            }

	    if ( !str_cmp( word, "DRoom" ) ) {
		ch->dreaming_room = get_room_index( fread_number( fp ) );
                if (ch->dreaming_room == NULL) {
                  bug("Char loaded with null dreaming room!",0);
                }
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) ) {
                                if (IS_IMP(ch)) ch->pcdata->authority = IMMAUTH_ALL;
  		return;
	    }
	    KEY( "Exp",		ch->exp,		fread_number( fp ) );
	    KEY( "Emai",        ch->pcdata->email,      	fread_string( fp ) ); 
	    KEY( "Email",       ch->pcdata->email,    	strdup(fread_word( fp ))  ); 
	    break;

        case 'F':
	    KEY( "Food",	ch->condition[COND_FOOD],	fread_number( fp ) );
	    KEY( "Fat",	ch->condition[COND_FAT],	fread_number( fp ) );
            break;

	case 'G':
	    KEY( "Gold",	ch->gold[0],		fread_number( fp ) );

	case 'H':
	    KEY( "Hitroll",	ch->hitroll,		fread_number( fp ) );
	    KEY( "Hit",		ch->hitroll,		fread_number( fp ) );
	    KEY( "Home", 	ch->pcdata->home_room,  fread_number (fp) );
	    KEY( "Homepage",    ch->pcdata->url, strdup(fread_word( fp ))  ); 

	    if ( !str_cmp( word, "HpManaMove" ) || !str_cmp(word,"HMV"))  {
		ch->hit		= fread_number( fp );
		ch->max_hit	= fread_number( fp );
		ch->mana	= fread_number( fp );
		ch->max_mana	= fread_number( fp );
		ch->move	= fread_number( fp );
		ch->max_move	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

            if ( !str_cmp( word, "HpManaMovePerm" ) || !str_cmp(word,"HMVP")) {
                ch->pcdata->perm_hit	= fread_number( fp );
                ch->pcdata->perm_mana   = fread_number( fp );
                ch->pcdata->perm_move   = fread_number( fp );
                fMatch = TRUE;
                break;
            }
      
	    break;

	case 'I':
	    KEY( "Image",       ch->pcdata->image, str_dup(fread_word( fp ))  ); 
	    KEY( "ImTi",        ch->pcdata->immtitle,   fread_string( fp ) );
	    KEY( "InvisLevel",	ch->invis_level,	fread_number( fp ) );
	    KEY( "Invi",	ch->invis_level,	fread_number( fp ) );

                    if (!str_cmp(word, "Ignore")) {
                           char *name = fread_string(fp);
                           IGNORE * ignore_new = new_ignore();

                           ignore_new->name = str_dup(name);
                           ignore_new->next = ch->pcdata->ignore_list;
                           ch->pcdata->ignore_list = ignore_new;
                           fMatch = TRUE;
                    }
	    break;

	case 'L':
	    KEY( "LastLevel",	ch->pcdata->last_level, fread_number( fp ) );
	    KEY( "LLev",	ch->pcdata->last_level, fread_number( fp ) );
	    KEY( "Level",	ch->level,		fread_number( fp ) );
	    KEY( "Lev",		ch->level,		fread_number( fp ) );
	    KEY( "Levl",	ch->level,		fread_number( fp ) );
                    KEY( "Life",     ch->pcdata->lifespan, fread_number(fp ) );
	    KEY( "LongDescr",	ch->long_descr,		fread_string( fp ) );
	    KEY( "LnD",		ch->long_descr,		fread_string( fp ) );
	    KEY( "LnDOrig",	ch->long_descr_orig,	fread_string( fp ) );
	    break;


	case 'M':
	    KEY( "Mounted",	ch->pcdata->mounted, fread_number( fp ) );
	    KEY( "MAX_CURRENCY", max_c, fread_number( fp ) );

	case 'N':
	    KEY( "Name",	ch->name,		fread_string( fp ) );
 	    KEY( "Native",	ch->pcdata->native,	fread_string( fp ) );
	    KEY( "Notify",	ch->notify,		fread_number( fp ) );
	    KEY( "Nmare",	ch->nightmare,		fread_number( fp ) );
	    break;

	case 'P':
	    KEY( "Password",	ch->pcdata->pwd,	fread_string( fp ) );
	    KEY( "Prompt", 	ch->pcdata->prompt,     fread_string( fp ) );
	    KEY( "Pass",	ch->pcdata->pwd,	fread_string( fp ) );
	    KEY( "Played",	ch->pcdata->played,		fread_number( fp ) );
	    KEY( "Plyd",	ch->pcdata->played,		fread_number( fp ) );
	    KEY( "Plr",		ch->plr,		fread_number_long_long(fp));
	    KEY( "Position",	ch->position,		fread_number( fp ) );
	    KEY( "Pos",		ch->position,		fread_number( fp ) );
	    KEY( "PosDesc",	ch->pos_desc,		fread_string( fp ) );
	    KEY( "Practice",	ch->practice,		fread_number( fp ) );
	    KEY( "Prac",	ch->practice,		fread_number( fp ) );
	    KEY( "Polyname",	ch->poly_name, 		fread_string( fp ) );
	    KEY( "PKtimer",	ch->pcdata->pk_timer, fread_number( fp) );

            if (!str_cmp(word, "Profession")) {

              char *pname;
              LPROF_DATA *lprof;

             /* Get a nice, new lprof... */
 
              lprof = get_lprof();
              
              if (lprof == NULL) {
                bug("Failed to allocate lprof for char!", 0);
                exit(1);
              }

             /* Read profession name and level... */

              pname = fread_word(fp);

              lprof->level = fread_number(fp);

             /* Mark as accepted... */

              fMatch = TRUE;

             /* Validate... */

              lprof->profession = get_profession(pname);

              if (lprof->profession == NULL) {
                bug("Unknown profession in player file!", 0);
                break;
              }
 
              if (lprof->level < 1) {
                lprof->level = 1;
                bug("Profession level set to 1", 0);
              }
                
              if (lprof->level > ch->level) {
                lprof->level = ch->level;
                bug("Profession level reduced to %d", lprof->level);
              }

             /* Add to the END of the chain... */

              if (ch->profs == NULL) {
                ch->profs = lprof;
              } else {
                LPROF_DATA *link;

                link = ch->profs;

                while(link->next != NULL) {
                  link = link->next;
                } 

                link->next = lprof;                   
              }

              lprof->next = NULL;
            }

	    break;

	case 'Q':
                   KEY( "QuestPnts",   ch->pcdata->questpoints,        fread_number( fp ) );
                   KEY( "QuestNext",   ch->pcdata->nextquest,          fread_number( fp ) );

           /* Quests */

            if (!str_cmp(word, "Quest")) { 
              fMatch = TRUE;
              new_quest = read_quest(fp);
              new_quest->next = ch->quests;
              ch->quests = new_quest; 

              break;
            }

	    break;

	case 'R':
	    KEY( "Race",        ch->race, race_lookup(fread_string( fp )) );
                    KEY("RaceOrig", ch->race_orig,	fread_number( fp ) );
                    KEY( "Recall", 		ch->recall_perm, fread_number(fp) );

	    if ( !str_cmp( word, "Room" ) )
	    {
		ch->in_room = get_room_index( fread_number( fp ) );
		if ( ch->in_room == NULL )
		    ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'S':
                    KEY( "Style",     ch->pcdata->style, fread_number(fp ) );
	    KEY( "Sanity",	ch->sanity,		fread_number( fp ) );
	    KEY( "SavingThrow",	ch->saving_throw,	fread_number( fp ) );
	    KEY( "Save",	ch->saving_throw,	fread_number( fp ) );
	    KEY( "Scro",	ch->lines,		fread_number( fp ) );
	    KEY( "Sex",		ch->sex,		fread_number( fp ) );
	    KEY( "ShortDescr",	ch->short_descr,	fread_string( fp ) );
	    KEY( "ShD",		ch->short_descr,	fread_string( fp ) );
                    KEY( "Sec",         ch->pcdata->security,   fread_number( fp ) );  
	    KEY( "Syer",	ch->pcdata->startyear, 		fread_number( fp ) );
	    KEY( "Smnt",	ch->pcdata->startmonth,		fread_number( fp ) );
	    KEY( "Sday",	ch->pcdata->startday,		fread_number( fp ) );
	    KEY( "ShDOrig",	ch->short_descr_orig,	fread_string( fp ) );
  	    KEY( "Spouse",	ch->pcdata->spouse,		fread_string( fp ) );

	    if ( !str_cmp( word, "Skill" ) || !str_cmp(word,"Sk"))
	    {
		int sn;
		int value;
		char *temp;

		value = fread_number( fp );
		temp = fread_word( fp ) ;

		sn = get_skill_sn(temp);
		
		if ( !validSkill(sn) )
		{
		    log_string(temp);
		    bug( "Fread_char: unknown skill. ", 0 );
		}
		else {
                    setSkill(ch->effective, sn, value);
                } 
		fMatch = TRUE;
	    }

            if (!str_cmp( word, "Speak")) {
              fMatch = TRUE;

	      ch->speak[SPEAK_PUBLIC] = get_skill_sn(fread_word(fp));
              ch->speak[SPEAK_PRIVATE] = get_skill_sn(fread_word(fp));

              if ( ch->speak[SPEAK_PRIVATE] == SKILL_UNDEFINED
                || ch->speak[SPEAK_PUBLIC] == SKILL_UNDEFINED) {
                bug("Bad speak skill!", 0);
              }
            }

            if (!str_cmp( word, "Society")) {
              fMatch = TRUE;

              memb = read_char_society(fp);

              if (memb != NULL) {
                memb->next = ch->memberships;
                ch->memberships = memb;
              }
            }

	    break;

	case 'T':
            KEY( "TrueSex",     ch->pcdata->true_sex,  	fread_number( fp ) );
	    KEY( "TSex",	ch->pcdata->true_sex,   fread_number( fp ) );
	    KEY( "Trai",	ch->train,		fread_number( fp ) );

	    if ( !str_cmp( word, "Title" )  || !str_cmp( word, "Titl"))
	    {
		ch->pcdata->title = fread_string( fp );
    		if (ch->pcdata->title[0] != '.' && ch->pcdata->title[0] != ',' 
		&&  ch->pcdata->title[0] != '!' && ch->pcdata->title[0] != '?')
		{
		    sprintf( buf, " %s", ch->pcdata->title );
		    free_string( ch->pcdata->title );
		    ch->pcdata->title = str_dup( buf );
		}
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Train" )) {

                ch->train = fread_number(fp);

                ch->training[TRAIN_STAT_STR]	= fread_number(fp);
                ch->training[TRAIN_STAT_INT]	= fread_number(fp);
                ch->training[TRAIN_STAT_WIS]	= fread_number(fp);
                ch->training[TRAIN_STAT_DEX]	= fread_number(fp);
                ch->training[TRAIN_STAT_CON]	= fread_number(fp);
                if (ch->version >= 8) {
                     ch->training[TRAIN_STAT_CHA] = fread_number(fp);
                }
                ch->training[TRAIN_HITS]	= fread_number(fp);
                ch->training[TRAIN_MANA]	= fread_number(fp);
                ch->training[TRAIN_MOVE]	= fread_number(fp);
                ch->training[TRAIN_PRACTICE]	= fread_number(fp);
                ch->training[TRAIN_SANITY]	= fread_number(fp);

		fMatch = TRUE;
		break;
	    }

	    break;

	case 'V':
	    KEY( "Version",     ch->version,		fread_number ( fp ) );
	    KEY( "Vers",	ch->version,		fread_number ( fp ) );
	    if ( !str_cmp( word, "Vnum" ) )
	    {
		ch->pIndexData = get_mob_index( fread_number( fp ) );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "Were",	ch->were_type,		fread_number( fp ) );
	    KEY( "Wimpy",	ch->wimpy,		fread_number( fp ) );
	    KEY( "Wimp",	ch->wimpy,		fread_number( fp ) );
	    KEY( "WimpDir",	ch->wimpy_dir,		fread_number( fp ) );
	    KEY( "Wiznet", 	ch->wiznet,		fread_number( fp ) );

	    if ( !str_cmp( word, "WRoom" ) ) {
		ch->waking_room = get_room_index( fread_number( fp ) );
                if (ch->waking_room == NULL) {
                  bug("Char loaded with null waking room!",0);
                }
		fMatch = TRUE;
		break;
	    }

	    break;
	}

	if ( !fMatch )
	{
                    sprintf (buf,"Fread_char: no match %s",word);
                    bug( buf, 0 );
	    fread_to_eol( fp );
	} 
	}
}

/* load a pet from the forgotten reaches */
void fread_pet( CHAR_DATA *ch, FILE *fp ) {
    const char *word;
    char *tword;
    CHAR_DATA *pet;
    bool fMatch;
    QUEST *new_quest;
    int i;
    int max_c = 0;

    log_string("Loading pet");

    /* first entry had BETTER be the vnum or we barf */
    word = feof(fp) ? "END" : fread_word(fp);

    if (!str_cmp(word,"Vnum"))
    {
    	int vnum;
    	
    	vnum = fread_number(fp);

    	if (get_mob_index(vnum) == NULL) {
    	    bug("Fread_pet: bad vnum %d.",vnum);
	    pet = create_mobile(get_mob_index(MOB_VNUM_ZOMBIE));
	} else {
    	    pet = create_mobile(get_mob_index(vnum));
        }

        pet->exp = 0;
    }
    else
    {
        bug("Fread_pet: no vnum in file.",0);
        pet = create_mobile(get_mob_index(MOB_VNUM_ZOMBIE));
    }

   /* Force wipe the pets room... */

    pet->in_room = NULL;
    
    pet->next_in_room			= NULL;
    pet->master				= NULL;
    pet->leader				= NULL;
    pet->fighting			                = NULL;
    pet->huntedBy 			= NULL;
    pet->prey				= NULL;
    pet->reply				= NULL;
    pet->pet				= NULL;

   /* Process the pets data... */

    for ( ; ; )
    {
    	word 	= feof(fp) ? "END" : fread_word(fp);

    	fMatch = FALSE;
    	
    	switch (UPPER(word[0]))   	{
    	case '*':
    	    fMatch = TRUE;
    	    fread_to_eol(fp);
    	    break;
    		
    	case 'A':
    	    KEY( "Act",		pet->act,		fread_number_long_long(fp));
    	    KEY( "AfBy",	pet->affected_by,
						fread_number_long_long(fp));
    	    KEY( "Alig",	pet->alignment,		fread_number(fp));
    	    
    	    if (!str_cmp(word,"ACs"))
    	    {
    	    	int i;
    	    	
    	    	for (i = 0; i < 4; i++)
    	    	    pet->armor[i] = fread_number(fp);
    	    	fMatch = TRUE;
    	    	break;
    	    }
    	    
    	    if ( !str_cmp(word,"AffD") 
              || !str_cmp(word, "Affect3")
              || !str_cmp(word, "Affect4")
              || !str_cmp(word, "Affect5")) {
    	    	AFFECT_DATA *paf;
    	    	int sn;
    	                tword=strdup(word);
    	    	
                paf = new_affect(); 
    	    	
                paf->type = SKILL_UNDEFINED;
		paf->afn = AFFECT_UNDEFINED;

                if ( !str_cmp(tword, "Affect3") 
                || !str_cmp(tword, "Affect4")
                || !str_cmp(tword, "Affect5")) {
                  int afn;
		  afn = get_affect_afn(fread_word(fp));
                  if (afn != AFFECT_UNDEFINED) {
                    paf->afn = afn;
                  } else {
                    bug("Fread_char: unknown affect.", 0);  
                  } 
                } else {
    	      	  sn = get_effect_efn(fread_word(fp));
    	     	  if (sn < 0) {
    	     	      bug("Fread_char: unknown effect.",0);
		  } else {
		      paf->type = sn;
                      paf->afn = get_affect_afn_for_effect(sn);
                    }
                } 
    	     	   
    	     	paf->level	= fread_number(fp);
    	     	paf->duration	= fread_number(fp);
    	     	paf->modifier	= fread_number(fp);
    	     	paf->location	= fread_number(fp);

                if (!str_cmp(tword, "Affect4")) {
                  paf->skill	= get_skill_sn(fread_word(fp));
                } else if (!str_cmp(tword, "Affect5")) {
                  paf->skill	= get_effect_efn(fread_word(fp));
                } else {
                  paf->skill	= SKILL_UNDEFINED;
                }

    	     	paf->bitvector	= fread_number_long_long(fp);
    	     	paf->next	= pet->affected;
    	     	pet->affected	= paf;
    	     	fMatch		= TRUE;
    	     	break;
    	    }
    	     
    	    if (!str_cmp(word,"AMod")) {
    	     	int stat;

                                if (ch->version < 8) {
    	     	     for (stat = 0; stat < MAX_STATS-2; stat++) pet->mod_stat[stat] = fread_number(fp);
                                } else {
     	     	     for (stat = 0; stat < MAX_STATS; stat++) pet->mod_stat[stat] = fread_number(fp);
                                }
    	     	fMatch = TRUE;
    	     	break;
    	    }
    	     
    	    if (!str_cmp(word,"Attr"))   {
    	         int stat;
                         if (ch->version < 8) {
      	              for (stat = 0; stat < MAX_STATS-2; stat++) pet->perm_stat[stat] = fread_number(fp);
                         } else {
      	              for (stat = 0; stat < MAX_STATS; stat++) pet->perm_stat[stat] = fread_number(fp);
                         }
    	         fMatch = TRUE;
    	         break;
    	    }
    	    break;
    	     
    	 case 'B':
    	     KEY( "Bio",	pet->bio,		fread_string(fp));
	    if (!str_cmp (word, "Bstatus")) {	
	          pet->limb[0] = fread_number( fp );
	          pet->limb[1] = fread_number( fp );
	          pet->limb[2] = fread_number( fp );
	          pet->limb[3] = fread_number( fp );
	          pet->limb[4] = fread_number( fp );
	          pet->limb[5] = fread_number( fp );
                          fMatch = TRUE;
                    }
    	     break;
    	     
    	 case 'C':
    	     KEY( "Comm",	pet->comm,		fread_number(fp));
	     if (!str_cmp( word, "Cash" )) {
    	         fMatch = TRUE;
                         if (max_c > 0) {
                              for (i = 0; i < max_c; i++) pet->gold[i] = fread_number(fp);
                         }
                     }
    	     break;
    	     
    	 case 'D':
    	     KEY( "Dam",	pet->damroll,		fread_number(fp));
    	     KEY( "Desc",	pet->description,	fread_string(fp));
	     KEY( "Drunk",	pet->condition[COND_DRUNK],
							fread_number( fp ) );
	     KEY( "Drink",	pet->condition[COND_DRINK],
							fread_number( fp ) );
    	     break;
    	     
    	 case 'E':
    	     if (!str_cmp(word,"End")) {

                pet->level = URANGE(1, pet->level, MAX_LEVEL);

                if (pet->exp == 0) {
                  pet->exp = exp_for_level(pet->level);
                }

		pet->leader = ch;
		pet->master = ch;
		ch->pet = pet;
log_string("End of Pet reached.");
    	     	return;
	     }
    	     KEY( "Exp",	pet->exp,		fread_number(fp));
    	     break;

         case 'F':
	     KEY( "Food",	pet->condition[COND_FOOD],
							fread_number( fp ) );
	     KEY( "Fat",	pet->condition[COND_FAT],
							fread_number( fp ) );
             break; 
    	     
    	 case 'G':
    	     KEY( "Gold",	pet->gold[0],		fread_number(fp));
    	     break;
    	     
    	 case 'H':
    	     KEY( "Hit",	pet->hitroll,		fread_number(fp));
    	     
    	     if (!str_cmp(word,"HMV"))
    	     {
    	     	pet->hit	= fread_number(fp);
    	     	pet->max_hit	= fread_number(fp);
    	     	pet->mana	= fread_number(fp);
    	     	pet->max_mana	= fread_number(fp);
    	     	pet->move	= fread_number(fp);
    	     	pet->max_move	= fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	     }
    	     break;
    	     
     	case 'L':
    	     KEY( "Levl",	pet->level,		fread_number(fp));
    	     KEY( "LnD",	pet->long_descr,	fread_string(fp));
			
    	     break;

    	case 'M':
    	     KEY( "MAX_CURRENCY", max_c, fread_number(fp));
    	     
    	case 'N':
    	     KEY( "Name",	pet->name,		fread_string(fp));
    	     break;
    	     
    	case 'P':
    	     KEY( "Pos",	pet->position,		fread_number(fp));
	     KEY( "PosDesc",	pet->pos_desc,		fread_string(fp));
    	     break;
    	     
	case 'Q':

           /* Quests */

            if (!str_cmp(word, "Quest")) { 
              fMatch = TRUE;

              new_quest = read_quest(fp);

              new_quest->next = pet->quests;
              pet->quests = new_quest; 

              break;
            }

	    break;

	case 'R':
    	    KEY( "Race",	pet->race, race_lookup(fread_string(fp)));
    	    break;
 	    
    	case 'S' :
    	    KEY( "Sanity",	pet->sanity,		fread_number(fp));
    	    KEY( "Save",	pet->saving_throw,	fread_number(fp));
    	    KEY( "Sex",		pet->sex,		fread_number(fp));
    	    KEY( "ShD",		pet->short_descr,	fread_string(fp));
    	    break;
    	    
    	if ( !fMatch )
    	{
    	    bug("Fread_pet: no match.",0);
    	    fread_to_eol(fp);
    	}
    	
    	}
    }
    
log_string("Pet loaded");
    return;
}



void fread_obj( CHAR_DATA *ch, FILE *fp, ROOM_INDEX_DATA *room) {
static OBJ_DATA obj_zero;
OBJ_DATA *obj;
const char *word;
char *tword;
int iNest;
bool fMatch;
bool fNest;
bool fVnum;
bool first;
bool new_format;  
bool make_new;   
    
    fVnum = FALSE;
    obj = NULL;
    first = TRUE;  
    new_format = FALSE;
    make_new = FALSE;

    fMatch = FALSE; 

    word   = feof( fp ) ? "End" : fread_word( fp );
    if (!str_cmp(word,"Vnum" ))    {
        int vnum;
        first = FALSE; 
 
        vnum = fread_number( fp );
        if (get_obj_index( vnum )  == NULL ) {
              bug( "Fread_obj: bad vnum %d.", vnum );
        } else {
              obj = create_object(get_obj_index(vnum),-1);
              new_format = TRUE;
        }
    }

    if (obj == NULL)     {
    	if ( obj_free == NULL )    	{
	    obj		= (OBJ_DATA *) alloc_perm( sizeof(*obj) );
    	} else {
	    obj		= obj_free;
	    obj_free	= obj_free->next;
    	}

    	*obj		= obj_zero;
    	obj->name		= str_dup( "" );
    	obj->short_descr	= str_dup( "" );
    	obj->description	= str_dup( "" );

    }

    fNest		= FALSE;
    fVnum		= TRUE;
    iNest		= 0;

    for ( ; ; )    {
	if (first) first = FALSE;
	else word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) ) {
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    if ( !str_cmp( word, "Affect" ) 
                    || !str_cmp(word,"Aff")
	    || !str_cmp( word, "AffD")
                    || !str_cmp(word, "Affect3")
                    || !str_cmp(word, "Affect4")
                    || !str_cmp(word, "Affect5")) {
		AFFECT_DATA *paf;
	        tword=strdup(word);

                paf = new_affect();

                paf->type = SKILL_UNDEFINED;
		paf->afn = AFFECT_UNDEFINED;

                if ( !str_cmp(tword, "Affect3")
                || !str_cmp(tword, "Affect4")
                || !str_cmp(tword, "Affect5")) {
                  int afn;
	  afn = get_affect_afn(fread_word(fp));
                  if (afn != AFFECT_UNDEFINED) paf->afn = afn;
                  else bug("Fread_char: unknown affect.", 0);  
                } else {
		  if (!str_cmp(word, "AffD")) {
		    int sn;
		    sn = get_effect_efn(fread_word(fp));
		    if (sn < 0)
			bug("Fread_obj: unknown effect.",0);
		    else {
		      paf->type = sn;
                                      paf->afn = get_affect_afn_for_effect(sn);
                    }
		  }
		  else /* old form */
		      paf->type	= fread_number( fp );
                }
 
		if (ch->version == 0)
		  paf->level = 20;
		else
		  paf->level	= fread_number( fp );

		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );

                if (!str_cmp(tword, "Affect4")) {
                  paf->skill	= get_skill_sn(fread_word(fp));
                } else if (!str_cmp(tword, "Affect5")) {
                  paf->skill	= get_effect_efn(fread_word(fp));
                } else {
                  paf->skill	= SKILL_UNDEFINED;
                }

		paf->bitvector	= fread_number_long_long( fp );
		paf->next	= obj->affected;
		obj->affected	= paf;
		fMatch		= TRUE;
		break;
	    }
	    break;

	case 'C':
	    KEY( "Cost",	obj->cost, fread_number( fp ) );
	    KEY( "Cond",    obj->condition,  fread_number (fp) );
	    break;

	case 'D':
	    KEY( "Description", obj->description, fread_string( fp ) );
	    KEY( "Desc",	obj->description,	fread_string( fp ) );
	    break;

	case 'E':

	    if ( !str_cmp( word, "Enchanted")) {
		obj->enchanted = TRUE;
	 	fMatch 	= TRUE;
		break;
	    }

	    KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );
	    KEY( "ExtF",	obj->extra_flags,	fread_number_long_long( fp ) );

	    if ( !str_cmp( word, "ExtraDescr" ) || !str_cmp(word,"ExDe")) {
		EXTRA_DESCR_DATA *ed;

		if ( extra_descr_free == NULL ) {
		    ed			= (EXTRA_DESCR_DATA *) alloc_perm( sizeof(*ed) );
		} else {
		    ed			= extra_descr_free;
		    extra_descr_free	= extra_descr_free->next;
		}

		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= obj->extra_descr;
		obj->extra_descr	= ed;
		fMatch = TRUE;
	    }

	    if ( !str_cmp( word, "End" ))  {
                          if (!obj->pIndexData && obj->orig_index) {
                              fVnum = reindex_obj(obj);
                          }

                          if (!fNest || (fVnum && obj->pIndexData == NULL ) ) {
                              bug( "Fread_obj: incomplete object.", 0 );
	              extract_obj (obj);
                              return;
                          } else {
                              if (!fVnum ) {
                                   free_string(obj->name);
                                   free_string(obj->description);
                                   free_string(obj->short_descr);
                                   obj->next = obj_free;
                                   obj_free  = obj;

                                   obj = create_object( get_obj_index( OBJ_VNUM_DUMMY ), 0 );
                              }

                              if (!new_format)  {
                                   obj->next       = object_list;
                                   object_list     = obj;
                                   obj->pIndexData->count++;
                              }

                    if (!obj->pIndexData->new_format
                    && obj->item_type == ITEM_ARMOR
                    &&  obj->value[1] == 0) {
                        obj->value[1] = obj->value[0];
                        obj->value[2] = obj->value[0];
                    }

                    if (make_new)
                    {
                        int wear;

                        wear = obj->wear_loc;
                        extract_obj(obj);

                        obj = create_object(obj->pIndexData,0);
                        obj->wear_loc = wear;
                    }

                    if ( iNest == 0 || rgObjNest[iNest] == NULL ) {
                             obj_to_char( obj, ch );
                             if (room != NULL) reset_loaded_obj(room, obj);
                    } else {
                             obj_to_obj( obj, rgObjNest[iNest-1] );
                             if (room != NULL) reset_loaded_obj(room, obj);
                    }

                    if (obj->zmask != 0) {
                        obj->zmask = obj->pIndexData->zmask;
                    }

                    if ( obj->item_type == ITEM_CONTAINER
                      && ( obj->value[3] <= 25
                        || obj->value[3] > 200 ) ) {
                      obj->value[3] = 100;
                    }  

                    return;
                }

	    }
	    break;

	case 'I':
	    KEY( "ItemType",	obj->item_type,		fread_number( fp ) );
	    KEY( "Ityp",		obj->item_type,		fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Level",	obj->level,		fread_number( fp ) );
	    KEY( "Lev",		obj->level,		fread_number( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	obj->name,		fread_string( fp ) );

	    if ( !str_cmp( word, "Nest" )) {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST ) {
		    bug( "Fread_obj: bad nest %d.", iNest );
		} else {
		    rgObjNest[iNest] = obj;
		    fNest = TRUE;
		}
		fMatch = TRUE;
	    }
	    break;

   	case 'O':
	    KEY( "Owner",		obj->owner, fread_string(fp) );
   	    KEY( "OrigIndex",	obj->orig_index, fread_number(fp));

                    if ( !str_cmp( word,"Oldstyle" ))   {
		if (obj->pIndexData != NULL && obj->pIndexData->new_format) make_new = TRUE;
		fMatch = TRUE;
	    }
	    break;
	
	case 'S':
	    KEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );
	    KEY( "ShD",		obj->short_descr,	fread_string( fp ) );
   	    KEY( "Size", 		obj->size, fread_number(fp));

	    if ( !str_cmp( word, "Spell" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = get_effect_efn( fread_word( fp ) );
		if ( iValue < 0 || iValue > 3 )
		{
		    bug( "Fread_obj: bad iValue %d.", iValue );
		}
		else if ( sn < 0 )
		{
		    bug( "Fread_obj: unknown effect.", 0 );
		}
		else
		{
		    obj->value[iValue] = sn;
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Timer",	obj->timer,		fread_number( fp ) );
	    KEY( "Time",	obj->timer,		fread_number( fp ) );
	    break;

	case 'V':
	    if ( !str_cmp( word, "Values" ) || !str_cmp(word,"Vals")) {
		obj->value[0]	= fread_number( fp );
		obj->value[1]	= fread_number( fp );
		obj->value[2]	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0) obj->value[0] = obj->pIndexData->value[0];
		fMatch		= TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Val" ) )
	    {
		obj->value[0] 	= fread_number( fp );
	 	obj->value[1]	= fread_number( fp );
	 	obj->value[2] 	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		obj->value[4]	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    
		if ( !str_cmp( word, "ValOrig" ) )
	    {
		obj->valueorig[0] 	= fread_number( fp );
	 	obj->valueorig[1]	= fread_number( fp );
	 	obj->valueorig[2] 	= fread_number( fp );
		obj->valueorig[3]	= fread_number( fp );
		obj->valueorig[4]	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp( word, "Vnum" )) {
		int vnum;

		vnum = fread_number( fp );
		if ((obj->pIndexData = get_obj_index( vnum)) == NULL ) {
		    bug( "Fread_obj: bad vnum %d.", vnum );
		} else {
		    fVnum = TRUE;
                                }
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WeaF",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Wear",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    KEY( "Wt",		obj->weight,		fread_number( fp ) );
	    break;

	case 'Z':
	    KEY( "ZMask",	obj->zmask,		fread_flag( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    bug( "Fread_obj: no match.", 0 );
	    fread_to_eol( fp );
	}
    }

    return;
}


void do_last( CHAR_DATA *ch, char *argument ) {
    char buf [MAX_STRING_LENGTH];
    char arg [MAX_INPUT_LENGTH];
    char name[MAX_INPUT_LENGTH];
    struct stat fst;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
    send_to_char( "Usage: last <playername>\r\n", ch );
    return;
    }
    strcpy( name, capitalize(arg) );
#if defined (COMPRESS_PFILES)
    sprintf( buf, "%s/%s.gz", PLAYER_DIR, name );
#else
    sprintf( buf, "%s/%s", PLAYER_DIR, name );
#endif
    if ( stat( buf, &fst ) != -1 )
      sprintf( buf, "%s was last on: %s\r", name, ctime( &fst.st_mtime ) );
    else
      sprintf( buf, "%s was not found.\r\n", name );
   send_to_char( buf, ch );
}


void save_shop_obj(void) {
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    FILE *fp;
    int currency;

    fp = fopen( SHOP_FILE, "w" );

    for ( ch = char_list; ch != NULL; ch = ch_next )    {
        ch_next = ch->next;

        if (!IS_NPC(ch) ) continue;
        currency = find_currency(ch);

        if (ch->pIndexData->pShop == NULL
        || ch->carrying == NULL) continue;

              	fprintf( fp, "SHOP %s~\n", ch->name);
                fprintf( fp, "%ld\n", ch->pIndexData->vnum);
                fprintf( fp, "%ld\n", ch->in_room->vnum);
              	fprintf( fp, "Gold %ld\n", ch->gold[currency]);

	if ( ch->carrying != NULL ) fwrite_obj(ch, ch->carrying, fp, 0 );
                if ( ch->ooz != NULL ) fwrite_obj( ch, ch->ooz, fp, 0 );
    }

    fprintf( fp, "END\n" );
    fclose( fp );
    log_string("Shops saved.");
    return;
}


void load_shop_obj(CHAR_DATA *ch, ROOM_INDEX_DATA *room) {
   FILE *fp;
   char buf[MAX_STRING_LENGTH];
   int mvnum = 0; 
   int rvnum = 0;
   long number =0;
   bool ok, done, rightone;
   char *kwd;
   char *name;
   char code;
   int currency;

    fp = fopen( SHOP_FILE, "r" );

    if (fp == NULL) {
        log_string("Shops not found.");
        return;
    }

    done = FALSE;
    rightone = FALSE;

    while(!done) {
         kwd = fread_word(fp);
         code = kwd[0];
         ok = FALSE;

         switch (code) {

             case '#':
                if ( !str_cmp(kwd, "#OBJECT" ) 
                || !str_cmp( kwd, "#O")) {
                        ok = TRUE;
                         if (rightone) {
                              fread_obj(ch, fp, room);
                        } else {
                             while (str_cmp(kwd, "End")) {
                                      kwd = fread_word(fp);
                                      fread_to_eol(fp); 
                             }
                        }
                }
               
                break; 

              case 'E':
                 if (!str_cmp(kwd, "END")) {
                     ok = TRUE;
                     done = TRUE;
                     break; 
                 } 

                 break; 

             case 'G':
                if (!str_cmp(kwd, "Gold")) {
                    ok = TRUE;
                    if (ch->pIndexData->pShop->currency > -1) currency = ch->pIndexData->pShop->currency;
                    else currency = find_currency_unreset(ch);
                    
                    number = fread_number(fp);
                    if (rightone) ch->gold[currency] = number;
                }  

                break;

             case 'S':
                if (!str_cmp(kwd, "SHOP")) {
                    ok = TRUE;
                    name = fread_string(fp); 
                    mvnum = fread_number(fp);
                    rvnum = fread_number(fp);

                    if (!str_cmp(name, ch->name)
                    && mvnum == ch->pIndexData->vnum
                    && rvnum == ch->in_room->vnum) {
                          rightone = TRUE;
                    } else {
                          rightone = FALSE;
                    }
                }  

                break;

             default:
                break;
         }

         if (!ok) {
            sprintf(buf, "Unrecognized keyword in Shops file: %s", kwd);
            bug(buf, 0); 
            return;
         }
    }

    fclose(fp);
    return;
}

void reset_loaded_obj(ROOM_INDEX_DATA *room, OBJ_DATA *obj) {
    RESET_DATA *pReset;

    pReset = room->reset_first;
    while (pReset != NULL) {
         if (pReset->command == 'G') {
              if (pReset->arg1 == obj->pIndexData->vnum) {
                    obj->reset = pReset;
                     pReset->count++;
                     break;
              }
         }
         pReset = pReset->next;
    }
    return;
}

void reset_morgue_obj(ROOM_INDEX_DATA *room, OBJ_DATA *obj) {
    RESET_DATA *pReset;

    pReset = room->reset_first;
    while (pReset != NULL) {
         if (pReset->command == 'O'
         || pReset->command == 'P') {
              if (pReset->arg1 == obj->pIndexData->vnum) {
                    obj->reset = pReset;
                    obj->condition = obj->pIndexData->condition;
                    pReset->count++;
                    break;
              }
         }
         pReset = pReset->next;
    }
    return;
}


void save_morgue(void) {
    ROOM_INDEX_DATA *room;
    FILE *fp;
    int vnum;

    fp = fopen(MORGUE_FILE, "w" );

    for ( vnum = 1; vnum <= 64000; vnum ++ ) {
          if ((room = get_room_index(vnum))) {
               if (room->area != NULL
               && room->contents != NULL) {
                    if (vnum == room->area->morgue
                    || vnum == zones[room->area->zone].morgue
                    || vnum == mud.morgue
                    || IS_SET(room->room_flags, ROOM_PRIVATE)) {
                          fprintf( fp, "MORGUE %d\n",	vnum);
                          fwrite_morgue(room, room->contents, fp, 0 );
                    }
               }
          }
    }
    fprintf( fp, "END\n" );
    fclose( fp );
    return;
}


void fwrite_morgue(ROOM_INDEX_DATA *room, OBJ_DATA *obj, FILE *fp, int iNest ) {
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;

    char buf[MAX_STRING_LENGTH];
 
    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->next_content != NULL ) fwrite_morgue(room, obj->next_content, fp, iNest );
    
    if (obj->trans_char != NULL) {
         affect_strip_afn(obj->trans_char, gafn_morf);
         REMOVE_BIT(obj->trans_char->affected_by, AFF_MORF );
         if(obj->carried_by != NULL) obj->trans_char->in_room = obj->carried_by->in_room;
         obj->trans_char->trans_obj = NULL;
         return;
    }
    if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)) return;

    fprintf( fp, "#O\n" );
    fprintf( fp, "Vnum %ld\n",   obj->pIndexData->vnum        );

    if (!obj->pIndexData->new_format) fprintf( fp, "Oldstyle\n");
    
    if (obj->enchanted) fprintf( fp,"Enchanted\n");
    
    fprintf( fp, "Nest %d\n",	iNest	  	     );
    fprintf( fp, "Size   %d\n", obj->size				);

   /* these data are only used if they do not match the defaults */

    if ( obj->name != obj->pIndexData->name) 	fprintf( fp, "Name %s~\n",	obj->name		     );
    if ( obj->short_descr != obj->pIndexData->short_descr) fprintf( fp, "ShD  %s~\n",	obj->short_descr	     );
    if ( obj->description != obj->pIndexData->description) fprintf( fp, "Desc %s~\n",	obj->description	     );
    if ( obj->owner != NULL) fprintf( fp, "Owner %s~\n", obj->owner);
    if ( obj->extra_flags != obj->pIndexData->extra_flags) fprintf( fp, "ExtF %lld\n",	obj->extra_flags	     );
    if ( obj->wear_flags != obj->pIndexData->wear_flags) fprintf( fp, "WeaF %d\n",	obj->wear_flags		     );
    if ( obj->item_type != obj->pIndexData->item_type) fprintf( fp, "Ityp %d\n",	obj->item_type		     );
    if ( obj->weight != obj->pIndexData->weight) fprintf( fp, "Wt   %d\n",	obj->weight		     );
    

   /* variable data */

    fprintf( fp, "Wear %d\n",   obj->wear_loc                );

    if (obj->level != 0) {
        fprintf( fp, "Lev  %d\n",	obj->level		     );
    }

    if (obj->timer != 0) {
        fprintf( fp, "Time %d\n",	obj->timer	     );
    }

    fprintf( fp, "Cost %d\n",	obj->cost		     );
    fprintf( fp, "Cond %d\n",	obj->condition		 );

    fprintf( fp, "Val  %ld %ld %ld %ld %ld\n",
  	    obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	    obj->value[4]	     );

    fprintf( fp, "ValOrig  %ld %ld %ld %ld %ld\n",
   	    obj->valueorig[0], obj->valueorig[1], obj->valueorig[2], 
            obj->valueorig[3], obj->valueorig[4]	     );

    switch ( obj->item_type )
    {
    case ITEM_POTION:
    case ITEM_SCROLL:
	if ( obj->value[1] > 0 ) {
	    fprintf( fp, "Spell 1 '%s'\n", effect_array[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 ) {
	    fprintf( fp, "Spell 2 '%s'\n", effect_array[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 ) {
	    fprintf( fp, "Spell 3 '%s'\n", effect_array[obj->value[3]].name );
	}

	break;

    case ITEM_PILL:
    case ITEM_STAFF:
    case ITEM_WAND:
	if ( obj->value[3] > 0 ) fprintf( fp, "Spell 3 '%s'\n", effect_array[obj->value[3]].name );
	break;
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next ) {

        if (paf->afn != AFFECT_UNDEFINED) {
          if (paf->location == APPLY_SKILL) { 
            if ( validSkill(paf->skill)
              && paf->skill > 0 ) {
              fprintf( fp, "Affect4 '%s' %3d %3d %3d %3d '%s' %lld\n", affect_array[paf->afn].name, paf->level, paf->duration, paf->modifier, paf->location, skill_array[paf->skill].name, paf->bitvector );
            } 
          } else if (paf->location == APPLY_EFFECT) { 
            if ( valid_effect(paf->skill)
            && paf->skill > 0 ) {
      	      fprintf( fp, "Affect5 '%s' %3d %3d %3d %3d '%s' %lld\n", affect_array[paf->afn].name, paf->level, paf->duration, paf->modifier, paf->location, effect_array[paf->skill].name, paf->bitvector );
            } 
         } else {
            fprintf( fp, "Affect3 '%s' %3d %3d %3d %3d %lld\n", affect_array[paf->afn].name, paf->level, paf->duration, paf->modifier, paf->location, paf->bitvector );
         }
       }
    }

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )    {
	fprintf( fp, "ExDe %s~ %s~\n", ed->keyword, ed->description );
    }

    fprintf( fp, "ZMask %s\n", fwrite_flag(obj->zmask, buf));
    fprintf( fp, "End\n\n" );
    if ( obj->contains != NULL ) fwrite_morgue(room, obj->contains, fp, iNest );

    return;
}


void load_morgue(ROOM_INDEX_DATA *room) {
   FILE *fp;
   OBJ_DATA *obj;
   char buf[MAX_STRING_LENGTH];
   bool ok, done, rightone;
   char *kwd;
   char code;
   int vnum = 0;

    fp = fopen(MORGUE_FILE, "r" );

    if (fp == NULL) {
        log_string("Morgues not found.");
        return;
    }

    done = FALSE;
    rightone = FALSE;

    while(!done) {
         kwd = fread_word(fp);
         code = kwd[0];
         ok = FALSE;

         switch (code) {

             case '#':
                if ( !str_cmp(kwd, "#OBJECT" ) 
                || !str_cmp( kwd, "#O")) {
                        ok = TRUE;
                         if (rightone) {
                              obj = fread_morgue_obj(fp, room);
                              if (IS_SET(obj->extra_flags, ITEM_NO_COND)) obj->condition = obj->pIndexData->condition;
                              if (obj->condition < 1) extract_obj(obj);
                        } else {
                             while (str_cmp(kwd, "End")) {
                                      kwd = fread_word(fp);
                                      fread_to_eol(fp); 
                             }
                        }
                }
               
                break; 

              case 'E':
                 if (!str_cmp(kwd, "END")) {
                     ok = TRUE;
                     done = TRUE;
                     break; 
                 } 

                 break; 

             case 'M':
                if (!str_cmp(kwd, "MORGUE")) {
                    ok = TRUE;
                    vnum = fread_number(fp);
                    if (vnum == room->vnum) rightone = TRUE;
                    else rightone = FALSE;
                }  

                break;

             default:
                break;
         }

         if (!ok) {
            sprintf(buf, "Unrecognized keyword in Morgue file: %s", kwd);
            bug(buf, 0); 
            return;
         }
    }

    fclose(fp);
    return;
}


OBJ_DATA *fread_morgue_obj(FILE *fp, ROOM_INDEX_DATA *room) {
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj;
    const char *word;
    char *tword;
    int iNest;
    bool fMatch;
    bool fNest;
    bool fVnum;
    bool first;
    bool new_format;  /* to prevent errors */
    bool make_new;    /* update object */
    
    fVnum = FALSE;
    obj = NULL;
    first = TRUE;  /* used to counter fp offset */
    new_format = FALSE;
    make_new = FALSE;

    fMatch = FALSE; 

    word   = feof( fp ) ? "End" : fread_word( fp );
    if (!str_cmp(word,"Vnum" ))    {
        int vnum;
        first = FALSE;  /* fp will be in right place */
 
        vnum = fread_number( fp );
        if (get_obj_index( vnum )  == NULL ) {
              bug( "Fread_obj: bad vnum %d.", vnum );
        } else {
              obj = create_object(get_obj_index(vnum),-1);
              new_format = TRUE;
        }
    }

    if (obj == NULL)     {
    	if ( obj_free == NULL )    	{
	    obj		= (OBJ_DATA *) alloc_perm( sizeof(*obj) );
    	} else {
	    obj		= obj_free;
	    obj_free	= obj_free->next;
    	}

    	*obj		= obj_zero;
    	obj->name		= str_dup( "" );
    	obj->short_descr	= str_dup( "" );
    	obj->description	= str_dup( "" );

       /* Yuuuch! Uninitialized type object... */
    }

    fNest		= FALSE;
    fVnum		= TRUE;
    iNest		= 0;

    for ( ; ; )    {
	if (first) first = FALSE;
	else word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) ) {
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    if ( !str_cmp( word, "Affect" ) 
                    || !str_cmp(word,"Aff")
	    || !str_cmp( word, "AffD")
                    || !str_cmp(word, "Affect3")
                    || !str_cmp(word, "Affect4")
                    || !str_cmp(word, "Affect5")) {
		AFFECT_DATA *paf;
                tword=strdup(word);

                paf = new_affect(); 	

                paf->type = SKILL_UNDEFINED;
		paf->afn = AFFECT_UNDEFINED;

                if ( !str_cmp(tword, "Affect3")
                || !str_cmp(tword, "Affect4")
                || !str_cmp(tword, "Affect5")) {
                  int afn;
	  afn = get_affect_afn(fread_word(fp));
                  if (afn != AFFECT_UNDEFINED) paf->afn = afn;
                  else bug("Fread_char: unknown affect.", 0);  
                } else {
		  if (!str_cmp(word, "AffD")) {
		    int sn;
		    sn = get_effect_efn(fread_word(fp));
		    if (sn < 0)
			bug("Fread_obj: unknown effect.",0);
		    else {
		      paf->type = sn;
                      paf->afn = get_affect_afn_for_effect(sn);
                    }
		  }
		  else /* old form */
		      paf->type	= fread_number( fp );
                }
 
	  paf->level	= fread_number( fp );
  	  paf->duration	= fread_number( fp );
	  paf->modifier	= fread_number( fp );
	  paf->location	= fread_number( fp );

                if (!str_cmp(tword, "Affect4")) {
                  paf->skill	= get_skill_sn(fread_word(fp));
                } else if (!str_cmp(tword, "Affect5")) {
                  paf->skill	= get_effect_efn(fread_word(fp));
                } else {
                  paf->skill	= SKILL_UNDEFINED;
                }

		paf->bitvector	= fread_number_long_long( fp );
		paf->next	= obj->affected;
		obj->affected	= paf;
		fMatch		= TRUE;
		break;
	    }
	    break;

	case 'C':
	    KEY( "Cost",	obj->cost,		fread_number( fp ) );

	    if ( !str_cmp( word, "Cond" )) {
		obj->condition	= fread_number( fp );
		fMatch		= TRUE;
                                obj->condition -= number_range(20, 40);
                    }
	    break;

	case 'D':
	    KEY( "Description",	obj->description,	fread_string( fp ) );
	    KEY( "Desc",	obj->description,	fread_string( fp ) );
	    break;

	case 'E':

	    if ( !str_cmp( word, "Enchanted"))   {
		obj->enchanted = TRUE;
	 	fMatch 	= TRUE;
		break;
	    }

	    KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );
	    KEY( "ExtF",	obj->extra_flags,	fread_number_long_long( fp ) );

	    if ( !str_cmp( word, "ExtraDescr" ) || !str_cmp(word,"ExDe"))  {
		EXTRA_DESCR_DATA *ed;

		if ( extra_descr_free == NULL ) {
		    ed			= (EXTRA_DESCR_DATA *) alloc_perm( sizeof(*ed) );
		} else {
		    ed			= extra_descr_free;
		    extra_descr_free	= extra_descr_free->next;
		}

		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= obj->extra_descr;
		obj->extra_descr	= ed;
		fMatch = TRUE;
	    }

	    if ( !str_cmp( word, "End" ) )    {
                if ( !fNest || ( fVnum && obj->pIndexData == NULL ) )         {
                    bug( "Fread_obj: incomplete object.", 0 );
	    extract_obj (obj);
                    return NULL;
                } else  {
                    if ( !fVnum )   {
                        free_string( obj->name        );
                        free_string( obj->description );
                        free_string( obj->short_descr );
                        obj->next = obj_free;
                        obj_free  = obj;

                        obj = create_object( get_obj_index( OBJ_VNUM_DUMMY ), 0 );
                    }

                    if (!new_format)    {
                        obj->next       = object_list;
                        object_list     = obj;
                        obj->pIndexData->count++;
                    }

                    if (!obj->pIndexData->new_format
                    && obj->item_type == ITEM_ARMOR
                    &&  obj->value[1] == 0)   {
                        obj->value[1] = obj->value[0];
                        obj->value[2] = obj->value[0];
                    }

                    if (make_new)  {
                        int wear;

                        wear = obj->wear_loc;
                        extract_obj(obj);

                        obj = create_object(obj->pIndexData,0);
                        obj->wear_loc = wear;
                    }

                    if ( iNest == 0 || rgObjNest[iNest] == NULL ) {
                             obj_to_room(obj, room);
                             reset_morgue_obj(room, obj);
                    } else {
                             obj_to_obj( obj, rgObjNest[iNest-1] );
                             reset_morgue_obj(room, obj);
                    }

                    if (obj->zmask != 0) {
                        obj->zmask = obj->pIndexData->zmask;
                    }

                    if ( obj->item_type == ITEM_CONTAINER
                      && ( obj->value[3] <= 25
                        || obj->value[3] > 200 ) ) {
                      obj->value[3] = 100;
                    }  

                    return obj;
                }

	    }
	    break;

	case 'I':
	    KEY( "ItemType",	obj->item_type,		fread_number( fp ) );
	    KEY( "Ityp",	obj->item_type,		fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Level",	obj->level,		fread_number( fp ) );
	    KEY( "Lev",		obj->level,		fread_number( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	obj->name,		fread_string( fp ) );

	    if ( !str_cmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
		    bug( "Fread_obj: bad nest %d.", iNest );
		}
		else
		{
		    rgObjNest[iNest] = obj;
		    fNest = TRUE;
		}
		fMatch = TRUE;
	    }
	    break;

   	case 'O':
	    KEY( "Owner",	obj->owner, fread_string(fp) );

                    if ( !str_cmp( word,"Oldstyle" ))   {
		if (obj->pIndexData != NULL && obj->pIndexData->new_format) make_new = TRUE;
		fMatch = TRUE;
	    }
	    break;
	
	case 'S':
	    KEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );
	    KEY( "ShD",		obj->short_descr,	fread_string( fp ) );
 	    KEY( "Size", 	obj->size,			fread_number(fp));

	    if ( !str_cmp( word, "Spell" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = get_effect_efn( fread_word( fp ) );
		if ( iValue < 0 || iValue > 3 )
		{
		    bug( "Fread_obj: bad iValue %d.", iValue );
		}
		else if ( sn < 0 )
		{
		    bug( "Fread_obj: unknown effect.", 0 );
		}
		else
		{
		    obj->value[iValue] = sn;
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Timer",	obj->timer,		fread_number( fp ) );
	    KEY( "Time",	obj->timer,		fread_number( fp ) );
	    break;

	case 'V':
	    if ( !str_cmp( word, "Values" ) || !str_cmp(word,"Vals"))
	    {
		obj->value[0]	= fread_number( fp );
		obj->value[1]	= fread_number( fp );
		obj->value[2]	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0)
		   obj->value[0] = obj->pIndexData->value[0];
		fMatch		= TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Val" ) )
	    {
		obj->value[0] 	= fread_number( fp );
	 	obj->value[1]	= fread_number( fp );
	 	obj->value[2] 	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		obj->value[4]	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    
		if ( !str_cmp( word, "ValOrig" ) )
	    {
		obj->valueorig[0] 	= fread_number( fp );
	 	obj->valueorig[1]	= fread_number( fp );
	 	obj->valueorig[2] 	= fread_number( fp );
		obj->valueorig[3]	= fread_number( fp );
		obj->valueorig[4]	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Vnum" ) )
	    {
		int vnum;

		vnum = fread_number( fp );
		if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
		    bug( "Fread_obj: bad vnum %d.", vnum );
		else
		    fVnum = TRUE;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WeaF",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Wear",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    KEY( "Wt",		obj->weight,		fread_number( fp ) );
	    break;

	case 'Z':
	    KEY( "ZMask",	obj->zmask,		fread_flag( fp ) );
	    break;

	}

	if ( !fMatch ) {
	    bug( "Fread_obj: no match.", 0 );
	    fread_to_eol( fp );
	}
    }
   
    return obj;
}


void save_tree(void) {
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    FILE *fp;

    fp = fopen( TREE_FILE, "w" );

    for (obj = object_list; obj != NULL; obj = obj_next )    {
        obj_next = obj->next;

        if (obj->item_type != ITEM_TREE) continue;
        if (obj->value[0] == 0) continue;
        if (obj->in_room == NULL) continue;
        if (!IS_SET(obj->in_room->room_flags, ROOM_TREE)) continue;

        fprintf( fp, "TREE %ld\n", obj->in_room->vnum);
        fwrite_tree(obj, fp, 0 );
    }

    fprintf( fp, "END\n" );
    fclose( fp );
    log_string("Trees saved.");
    return;
}

void load_tree(ROOM_INDEX_DATA *room) {
   FILE *fp;
   OBJ_DATA *obj;
   char buf[MAX_STRING_LENGTH];
   bool ok, done, rightone;
   char *kwd;
   char code;
   int vnum = 0;

    fp = fopen(TREE_FILE, "r" );

    if (fp == NULL) {
        log_string("Trees not found.");
        return;
    }

    done = FALSE;
    rightone = FALSE;

    while(!done) {
         kwd = fread_word(fp);
         code = kwd[0];
         ok = FALSE;

         switch (code) {

             case '#':
                if ( !str_cmp(kwd, "#OBJECT" ) 
                || !str_cmp( kwd, "#O")) {
                        ok = TRUE;
                         if (rightone) {
                              obj = fread_morgue_obj(fp, room);
                              obj->condition = obj->pIndexData->condition;
                        } else {
                             while (str_cmp(kwd, "End")) {
                                      kwd = fread_word(fp);
                                      fread_to_eol(fp); 
                             }
                        }
                }
               
                break; 

              case 'E':
                 if (!str_cmp(kwd, "END")) {
                     ok = TRUE;
                     done = TRUE;
                     break; 
                 } 

                 break; 

             case 'T':
                if (!str_cmp(kwd, "TREE")) {
                    ok = TRUE;
                    vnum = fread_number(fp);
                    if (vnum == room->vnum) rightone = TRUE;
                    else rightone = FALSE;
                }  

                break;

             default:
                break;
         }

         if (!ok) {
            sprintf(buf, "Unrecognized keyword in Tree file: %s", kwd);
            bug(buf, 0); 
            return;
         }
    }

    fclose(fp);
    return;
}


void fwrite_tree(OBJ_DATA *obj, FILE *fp, int iNest ) {
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;
    char buf[MAX_STRING_LENGTH];
 
    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->next_content != NULL ) fwrite_tree(obj->next_content, fp, iNest );
    
    if (obj->trans_char != NULL) {
         affect_strip_afn(obj->trans_char, gafn_morf);
         REMOVE_BIT(obj->trans_char->affected_by, AFF_MORF );
         if(obj->carried_by != NULL) obj->trans_char->in_room = obj->carried_by->in_room;
         obj->trans_char->trans_obj = NULL;
         return;
    }

    if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)) return;

    fprintf( fp, "#O\n" );
    fprintf( fp, "Vnum %ld\n",   obj->pIndexData->vnum        );

    if (!obj->pIndexData->new_format) fprintf( fp, "Oldstyle\n");
    
    if (obj->enchanted) fprintf( fp,"Enchanted\n");
    
    fprintf( fp, "Nest %d\n",	iNest	  	     );
    fprintf( fp, "Size   %d\n", obj->size				);

   /* these data are only used if they do not match the defaults */

    if ( obj->name != obj->pIndexData->name) 	fprintf( fp, "Name %s~\n",	obj->name		     );
    if ( obj->short_descr != obj->pIndexData->short_descr) fprintf( fp, "ShD  %s~\n",	obj->short_descr	     );
    if ( obj->description != obj->pIndexData->description) fprintf( fp, "Desc %s~\n",	obj->description	     );
    if ( obj->owner != NULL) fprintf( fp, "Owner %s~\n", obj->owner);
    if ( obj->extra_flags != obj->pIndexData->extra_flags) fprintf( fp, "ExtF %lld\n",	obj->extra_flags	     );
    if ( obj->wear_flags != obj->pIndexData->wear_flags) fprintf( fp, "WeaF %d\n",	obj->wear_flags		     );
    if ( obj->item_type != obj->pIndexData->item_type) fprintf( fp, "Ityp %d\n",	obj->item_type		     );
    if ( obj->weight != obj->pIndexData->weight) fprintf( fp, "Wt   %d\n",	obj->weight		     );
    

   /* variable data */

    fprintf( fp, "Wear %d\n",   obj->wear_loc                );

    if (obj->level != 0) {
        fprintf( fp, "Lev  %d\n",	obj->level		     );
    }

    if (obj->timer != 0) {
        fprintf( fp, "Time %d\n",	obj->timer	     );
    }

    fprintf( fp, "Cost %d\n",	obj->cost		     );
    fprintf( fp, "Cond %d\n",	obj->condition		 );

    fprintf( fp, "Val  %ld %ld %ld %ld %ld\n",
  	    obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	    obj->value[4]	     );

    fprintf( fp, "ValOrig  %ld %ld %ld %ld %ld\n",
   	    obj->valueorig[0], obj->valueorig[1], obj->valueorig[2], 
            obj->valueorig[3], obj->valueorig[4]	     );

    switch ( obj->item_type )
    {
    case ITEM_POTION:
    case ITEM_SCROLL:
	if ( obj->value[1] > 0 ) fprintf( fp, "Spell 1 '%s'\n", effect_array[obj->value[1]].name );
	if ( obj->value[2] > 0 ) fprintf( fp, "Spell 2 '%s'\n", effect_array[obj->value[2]].name );
	if ( obj->value[3] > 0 )  fprintf( fp, "Spell 3 '%s'\n", effect_array[obj->value[3]].name );
	break;

    case ITEM_PILL:
    case ITEM_STAFF:
    case ITEM_WAND:
	if ( obj->value[3] > 0 ) fprintf( fp, "Spell 3 '%s'\n", effect_array[obj->value[3]].name );
	break;
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next ) {

        if (paf->afn != AFFECT_UNDEFINED) {
          if (paf->location == APPLY_SKILL) { 
            if ( validSkill(paf->skill)
              && paf->skill > 0 ) {
              fprintf( fp, "Affect4 '%s' %3d %3d %3d %3d '%s' %lld\n", affect_array[paf->afn].name, paf->level, paf->duration, paf->modifier, paf->location, skill_array[paf->skill].name,
	        paf->bitvector );
            } 
          } else if (paf->location == APPLY_EFFECT) { 
            if ( valid_effect(paf->skill)
            && paf->skill > 0 ) {
      	      fprintf( fp, "Affect5 '%s' %3d %3d %3d %3d '%s' %lld\n", affect_array[paf->afn].name, paf->level, paf->duration, paf->modifier, paf->location, effect_array[paf->skill].name, paf->bitvector );
            }
         } else {
            fprintf( fp, "Affect3 '%s' %3d %3d %3d %3d %lld\n", affect_array[paf->afn].name, paf->level, paf->duration, paf->modifier, paf->location, paf->bitvector );
         }
       }
    }

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )    {
	fprintf( fp, "ExDe %s~ %s~\n", ed->keyword, ed->description );
    }

    fprintf( fp, "ZMask %s\n", fwrite_flag(obj->zmask, buf));
    fprintf( fp, "End\n\n" );
    if ( obj->contains != NULL ) fwrite_tree(obj->contains, fp, iNest );
    return;
}


void save_artifacts(void) {
OBJ_DATA *obj;
OBJ_DATA *obj_next;
FILE *fp;
char *name;
VNUM onum[99], in_room[99];
VNUM iroom = 0;
int i;
int fnum;

    for (i = 0; i < 99; i++) {
          onum[i] = 0;
          in_room[i] = 0;
    }
   
    fp = fopen(ARTIFACT_FILE, "r" );
    if (fp) {
        i = 0;
        while (i < 99) {
             name  = fread_word(fp);
             if (!is_number(name)) break;

             onum[i] = atol(name);
             in_room[i] = fread_number(fp);
             fread_to_eol(fp); 
             i++;
        }             
    }                     
    fclose( fp );
                  
    fp = fopen(ARTIFACT_FILE, "w" );

    for (obj = object_list; obj != NULL; obj = obj_next )    {
        obj_next = obj->next;

        if (!IS_SET(obj->extra_flags, ITEM_ARTIFACT) || !obj->pIndexData) continue;

        fnum = -1;
        for (i = 0; i < 99; i++) {
                if (onum[i] == obj->pIndexData->vnum) fnum = i;
        }

         if (obj->in_room) {
                     iroom = obj->in_room->vnum;
         } else if (obj->carried_by) {
                     iroom = obj->carried_by->in_room->vnum;
         } else if (obj->in_obj) {
                     OBJ_DATA *cont;
                     cont = obj->in_obj;
             
                     while (cont->in_obj) cont = cont->in_obj;

                     if (cont->carried_by) iroom = cont->carried_by->in_room->vnum;
                     else iroom = cont->in_room->vnum;
         }

         if (fnum == -1) {
                 fprintf( fp, "%ld %ld %ld\n", obj->pIndexData->vnum, iroom, iroom);
         } else {
                 fprintf( fp, "%ld %ld %ld\n", obj->pIndexData->vnum, in_room[fnum], iroom);
         }            
    }
    fprintf( fp, "END\n" );
    fclose( fp );
    
    log_string("Artifacts saved.");
    return;
}


bool load_artifact(RESET_DATA *pReset) {
FILE *fp;
ROOM_INDEX_DATA *room;
OBJ_INDEX_DATA *pObj;
OBJ_DATA *obj;
char *name;

    pObj = get_obj_index(pReset->arg1);
    if (!pObj) return FALSE;
    if (!IS_SET(pObj->extra_flags, ITEM_ARTIFACT)) return FALSE;

    fp = fopen(ARTIFACT_FILE, "r" );
    if (fp) {
        VNUM room1, room2;
  
        for(; ;) {
             name  = fread_word(fp);
             if (!is_number(name)) break;
             room1 = fread_number(fp);
             room2 = fread_number(fp);
             if (room1 == room2) {
                 fclose( fp );
                 return FALSE;
             }
             if (atol(name) == pReset->arg1) {
                  room = get_room_index(room2);
                  if (!room) {
                     fclose( fp );
                     return FALSE;
                  }
                  obj = create_object(pObj, pObj->level);
 
                  obj->reset = pReset;
                  pReset->count++;

                  obj_to_room(obj, room);
                  fclose( fp );
                  return TRUE;
             }
        }             
        fclose( fp );
    }                     
    return FALSE;
}
        

void save_artifacts_forward(void) {
FILE *fp;
char *name;
VNUM onum[99], in_room[99];
int i;

    for (i = 0; i < 99; i++) {
          onum[i] = 0;
          in_room[i] = 0;
    }
   
    fp = fopen(ARTIFACT_FILE, "r" );
    if (fp) {
        i = 0;
        while (i < 99) {
             name  = fread_word(fp);
             if (!is_number(name)) break;

             onum[i] = atol(name);
             in_room[i] = fread_number(fp);
             in_room[i] = fread_number(fp);
             i++;
        }             
        fclose( fp );
    }                     
                  
    fp = fopen(ARTIFACT_FILE, "w" );
    for (i = 0; i < 99; i++) {
         if (onum[i] > 0) fprintf( fp, "%ld %ld 0\n", onum[i], in_room[i]);
    }
    fprintf( fp, "END\n" );
    fclose( fp );
    
    log_string("Artifacts forwarded...");
    return;
}


void save_ignore(CHAR_DATA *ch, FILE *fp) {
IGNORE *ignore;

     if (IS_NPC(ch)) return;
     
     for (ignore = ch->pcdata->ignore_list; ignore; ignore = ignore->next) {
           fprintf( fp, "Ignore %s~\n", ignore->name);
     }
     return;
}


bool name_exists(char *name) {
char strsave[MAX_STRING_LENGTH];
char buf[MAX_STRING_LENGTH];
FILE *fp;

    #if defined(unix)
    /* decompress if .gz file exists */
    sprintf( strsave, "%s%s%s", PLAYER_DIR, capitalize(name),".gz");
    if ((fp = fopen(strsave, "r")) != NULL) {
	fclose(fp);
	sprintf(buf,"gzip -dfq %s",strsave);
	system(buf);
    }
    #endif

    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( name ) );
    if ((fp = fopen( strsave, "r" ) ) != NULL ) {
        fclose(fp);
        return TRUE;
    }

    return FALSE;
}



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

/**************************************************************************
 *  File: olc_save.c                                                       *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

#include "everything.h"
#include "olc.h"
#include "spell.h"
#include "prof.h"
#include "econd.h"
#include "conv.h" 
#include "mob.h"
#include "doors.h"
#include "gadgets.h"
#include "deeds.h"
#include "triggers.h"
#include "monitor.h"
#include "anchor.h"
#include "current.h"
#include "chat.h"
#include "tracks.h"
#include "race.h"

/*
 *  Verbose writes reset data in plain english into the comments
 *  section of the resets.  It makes areas considerably larger but
 *  may aid in debugging.
 */

/*
#define VERBOSE 
*/

/*
 * Remove carriage returns from a line
 */
char *strip_cr( char *str )
{
    static char newstr[MAX_STRING_LENGTH];
    int i, j;

    for ( i=j=0; str[i] != '\0'; i++ )
        if ( str[i] != '\r' )
        {
          newstr[j++] = str[i];
        }
    newstr[j] = '\0';
    return newstr;
}


/*****************************************************************************
 Name:		fix_string
 Purpose:	Returns a string without \r and ~.
 ****************************************************************************/
char *fix_string( const char *str )
{
    static char strfix[MAX_STRING_LENGTH];
    int i;
    int o;
    if ( str == NULL )
        return '\0';

    for ( o = i = 0; (str[i+o] != '\0' && (i+1) <= MAX_STRING_LENGTH) ; i++ )
    {
        if (str[i+o] == '\r' || str[i+o] == '~')
            o++;
        strfix[i] = str[i+o];
    }
    strfix[i] = '\0';
    return strfix;
}



/*****************************************************************************
 Name:		save_area_list
 Purpose:	Saves the listing of files to be loaded at startup.
 Called by:	do_asave(olc_save.c).
 ****************************************************************************/
void save_area_list(){
    FILE *fp;
    AREA_DATA *pArea;

    if ( ( fp = fopen( "area.lst", "w" ) ) == NULL )    {
	bug( "Save_area_list: fopen", 0 );
	perror( "area.lst" );
    }  else   {
	/*
	 * Add any help files that need to be loaded at
	 * startup to this section.
	 */
	fprintf( fp, "help.are\n"   );

	for( pArea = area_first; pArea; pArea = pArea->next ) {
	
            if ( pArea->lvnum != 0
              && pArea->uvnum != 0 ) {
	      fprintf( fp, "%s\n", pArea->filename );
            }
             
	}

	fprintf( fp, "$\n" );
	fclose( fp );
    }

    return;
}


/*
 * ROM OLC
 * Used in save_mobile and save_object below.  Writes
 * flags on the form fread_flag reads.
 * 
 * buf[] must hold at least 32+1 characters.
 *
 * -- Hugin
 */
char *fwrite_flag( long flags, char buf[] ) {
    char offset;
    char *cp;

    buf[0] = '\0';

    if ( flags == 0 )   {
	strcpy( buf, "0" );
	return buf;
    }

    /* 32 -- number of bits in a long */

    for ( offset = 0, cp = buf; offset < 32; offset++ )
	if ( flags & ( (long)1 << offset )) {
	    if ( offset <= 'Z' - 'A' ) *(cp++) = 'A' + offset;
	    else *(cp++) = 'a' + offset - ( 'Z' - 'A' + 1 );
	}

    *cp = '\0';

    return buf;
}


char *fwrite_flag_long(long long flags, char buf[] ) {
    char offset;
    char *cp;

    buf[0] = '\0';

    if ( flags == 0 )   {
	strcpy( buf, "0" );
	return buf;
    }

    /* 64 -- number of bits in a long */

    for ( offset = 0, cp = buf; offset < 64; offset++ )
	if ( flags & ( (long long)1 << offset )) {
	    if ( offset <= 'Z' - 'A' ) *(cp++) = 'A' + offset;
	    else *(cp++) = 'a' + offset - ( 'Z' - 'A' + 1 );
	}

    *cp = '\0';

    return buf;
}


/*****************************************************************************
 Name:		save_mobile
 Purpose:	Save one mobile to file, new format -- Hugin
 Called by:	save_mobiles (below).
 ****************************************************************************/
void save_mobile( FILE *fp, MOB_INDEX_DATA *pMobIndex ) {
    MPROG_DATA *mprg;
    short race = pMobIndex->race;
    char buf[MAX_STRING_LENGTH];
    int sn; 

    fprintf( fp, "#%ld\n",        pMobIndex->vnum );
    fprintf( fp, "%s~\n",         pMobIndex->player_name );
    fprintf( fp, "%s~\n",         pMobIndex->short_descr );
    fprintf( fp, "%s~\n",         fix_string( pMobIndex->long_descr ) );
    fprintf( fp, "%s~\n",         fix_string( pMobIndex->description) );
    fprintf( fp, "%s~\n",         race_array[race].name );
    fprintf( fp, "%s ",	          fwrite_flag_long( pMobIndex->act,         buf ) );
    fprintf( fp, "%s ",	          fwrite_flag_long( pMobIndex->affected_by, buf ) );
    fprintf( fp, "%d ",           pMobIndex->alignment );
    fprintf( fp, "%d\n",          pMobIndex->group );
    fprintf( fp, "%d ",	          pMobIndex->level );
    fprintf( fp, "%d ",	          pMobIndex->hitroll );
    fprintf( fp, "%dd%d+%d ",     pMobIndex->hit[DICE_NUMBER], 
	     	     	          pMobIndex->hit[DICE_TYPE], 
	     	     	          pMobIndex->hit[DICE_BONUS] );
    fprintf( fp, "%dd%d+%d ",     pMobIndex->mana[DICE_NUMBER], 
	     	     	          pMobIndex->mana[DICE_TYPE], 
	     	     	          pMobIndex->mana[DICE_BONUS] );
    fprintf( fp, "%dd%d+%d ",     pMobIndex->damage[DICE_NUMBER], 
	     	     	          pMobIndex->damage[DICE_TYPE], 
	     	     	          pMobIndex->damage[DICE_BONUS] );
    fprintf( fp, "'%s' ",         attack_type_name(pMobIndex->atk_type) );
    fprintf( fp, "'%s'\n",        dam_type_name(pMobIndex->dam_type) );
    fprintf( fp, "%d %d %d %d\n", pMobIndex->ac[AC_PIERCE] / 10, 
	     	     	          pMobIndex->ac[AC_BASH]   / 10, 
	     	     	          pMobIndex->ac[AC_SLASH]  / 10, 
	     	     	          pMobIndex->ac[AC_EXOTIC] / 10 );
    fprintf( fp, "%s ",           fwrite_flag( pMobIndex->off_flags,  buf ) );
    fprintf( fp, "%s ",	          fwrite_flag( pMobIndex->imm_flags,  buf ) );
    fprintf( fp, "%s ",           fwrite_flag( pMobIndex->res_flags,  buf ) );
    fprintf( fp, "%s\n",          fwrite_flag( pMobIndex->vuln_flags, buf ) );
    fprintf( fp, "'%s' '%s' '%s' %ld\n",
	                          position_name(pMobIndex->start_pos),
	         	     	  position_name(pMobIndex->default_pos),
	         	     	  sex_name(pMobIndex->sex),
	         	     	  pMobIndex->gold );
    fprintf( fp, "%s ",           fwrite_flag( pMobIndex->form,  buf ) );
    fprintf( fp, "%s ",      	  fwrite_flag( pMobIndex->parts, buf ) );
    fprintf( fp, "'%s' ",         size_name(pMobIndex->size));
    fprintf( fp, "'%s'\n",        material_name( pMobIndex->material ) );

   /* Mob extended data... */

    fprintf(fp, "X\n");

   /* Nature */

    fprintf(fp, "Nature %d\n", pMobIndex->nature);

   /* Conversations... */

    if (pMobIndex->cd != NULL) {
      write_conv(fp, pMobIndex->cd);
    } 

   /* Scripts... */

    if (pMobIndex->triggers->scripts != NULL) {
      save_script(fp, pMobIndex->triggers->scripts);
    } 

   /* Challange Triggers... */

    if (pMobIndex->triggers->challange != NULL) {
      write_triggers(pMobIndex->triggers->challange, fp, "TriggerChallange");
    } 

   /* Reaction Triggers... */

    if (pMobIndex->triggers->reaction != NULL) {
      write_triggers(pMobIndex->triggers->reaction, fp, "TriggerReact");
    } 

   /* Time event subscriptions... */

    if (pMobIndex->time_wev!= 0) {
      fprintf(fp, "TimeSub %d\n", pMobIndex->time_wev);
    } 

    if (pMobIndex->language>0) fprintf( fp, "Language %s~\n", skill_array[pMobIndex->language].name);
    if (pMobIndex->envimm_flags >0) fprintf( fp, "EnvImm %s\n", fwrite_flag( pMobIndex->envimm_flags,  buf ));

   /* Monitors... */

    if (pMobIndex->monitors != NULL) write_monitors(pMobIndex->monitors, fp, "Monitor");
   

   /* Chat... */

    if (pMobIndex->triggers->chat != NULL) { 
      write_chat(fp, pMobIndex->triggers->chat, "Chat");
    }  

   /* Fright... */

    if (pMobIndex->fright != 0) {
      fprintf(fp, "Fright %d\n", pMobIndex->fright);
    } 

   /* Bio... */

    if ( pMobIndex->bio != NULL
      && pMobIndex->bio[0] != '\0' ) {
      fprintf(fp, "Bio %s~\n", pMobIndex->bio);
    }

   /* Can/can_not see... */

    if (pMobIndex->can_see != NULL) {
      write_econd( pMobIndex->can_see, fp, "CanSee");
    }

    if (pMobIndex->can_not_see != NULL) {
      write_econd( pMobIndex->can_not_see, fp, "CanNotSee");
    }

   /* Image... */

    if (pMobIndex->image != NULL) {
      fprintf(fp, "Image '%s'\n", pMobIndex->image);
    } 

   /* Close mob extended data... */

    fprintf(fp, "End\n");

   /* Save mob skills */

    if (pMobIndex->skills != NULL) {
      for(sn = 0; sn < MAX_SKILL; sn++) {
        
        if (pMobIndex->skills->skill[sn] != 0) {
          fprintf(fp, "+ %d %s~\n", pMobIndex->skills->skill[sn],
                                   skill_array[sn].name);
        }
      } 
    }

    if ( pMobIndex->progtypes ) {
	    for ( mprg = pMobIndex->mobprogs; mprg != NULL; mprg = mprg->next ) {
 	        fprintf( fp, ">%s %s~\n%s~\n", mprog_type_to_name( mprg->type ), mprg->arglist, strip_cr(mprg->comlist));
	    }
    }

    if ( pMobIndex->progtypes ) fprintf (fp, "|\n" );

    return;
}


/*****************************************************************************
 Name:		save_mobiles
 Purpose:	Save #MOBILES secion of an area file.
 Called by:	save_area(olc_save.c).
 Notes:         Changed for ROM OLC.
 ****************************************************************************/
void save_mobiles( FILE *fp, AREA_DATA *pArea ) {
long i;
MOB_INDEX_DATA *pMob;

    fprintf( fp, "#MOBILES\n" );

    for( i = pArea->lvnum; i <= pArea->uvnum; i++ )   {
	if ( (pMob = get_mob_index( i ))) save_mobile( fp, pMob );
    }

    fprintf( fp, "#0\n\n\n\n" );
    return;
}


/* Write an extra description out to a file... */

void save_extra(EXTRA_DESCR_DATA *ed, FILE *fp, char *dhdr) {
 
 /* First write the extra data out... */

  fprintf( fp, "E\n%s~\n%s~\n", ed->keyword,  fix_string( ed->description ) );

 /* Then write any econds... */

  if (ed->ec != NULL) {
    write_econd(ed->ec, fp, "C");
  }

 /* And any deeds... */
 
  if (ed->deed != NULL) {
    save_extra_deed(fp, ed->deed, dhdr);
  }
 
  return;
}


char *effect_name(int i) {

    if ( i < 1 || i > MAX_EFFECT ) {
      i = 0;
    }	

  return effect_array[i].name;
}

char *skill_name(int i) {

    if ( i < 1 || i > MAX_SKILL ) {
      i = 0;
    }	

  return skill_array[i].name;
}

/*****************************************************************************
 Name:		save_object
 Purpose:	Save one object to file.
                new ROM format saving -- Hugin
 Called by:	save_objects (below).
 ****************************************************************************/
void save_object( FILE *fp, OBJ_INDEX_DATA *pObjIndex ) {
char letter;
AFFECT_DATA *pAf;
EXTRA_DESCR_DATA *pEd;
char buf[MAX_STRING_LENGTH];

    fprintf( fp, "#%ld\n",    pObjIndex->vnum );
    fprintf( fp, "%s~\n",    pObjIndex->name );
    fprintf( fp, "%s~\n",    pObjIndex->short_descr );
    fprintf( fp, "%s~\n",    pObjIndex->description );
    fprintf( fp, "%s~\n",    material_name( pObjIndex->material ) );
    fprintf( fp, "'%s' ",    item_name(pObjIndex->item_type) );
    fprintf( fp, "%s ",      fwrite_flag_long(pObjIndex->extra_flags, buf ) );
    fprintf( fp, "%s\n",     fwrite_flag( pObjIndex->wear_flags,  buf ) );

/*
 *  Using fwrite_flag to write most values gives a strange
 *  looking area file, consider making a case for each
 *  item type later.
 */

    switch ( pObjIndex->item_type )    {
        default:
	    fprintf( fp, "%s ",  fwrite_flag(       pObjIndex->value[0], buf ) );
	    fprintf( fp, "%s ",  fwrite_flag(       pObjIndex->value[1], buf ) );
	    fprintf( fp, "%s ",  fwrite_flag(       pObjIndex->value[2], buf ) );
	    fprintf( fp, "%s ",  fwrite_flag(       pObjIndex->value[3], buf ) );
	    fprintf( fp, "%s\n", fwrite_flag(       pObjIndex->value[4], buf ) );
	    break;

        case ITEM_WEAPON:
	    fprintf( fp, "'%s' ",  weapon_class_name( pObjIndex->value[0]      ) );
	    fprintf( fp, "%ld ",                      pObjIndex->value[1]        );
	    fprintf( fp, "%ld ",                      pObjIndex->value[2]        );
	    fprintf( fp, "'%s' ",  attack_type_name(  pObjIndex->value[3]      ) );
	    fprintf( fp, "%s\n",   fwrite_flag(       pObjIndex->value[4], buf ) );
	    break;

        case ITEM_CONTAINER:
	    fprintf( fp, "%ld ",                      pObjIndex->value[0]        );
	    fprintf( fp, "%s ",    fwrite_flag(       pObjIndex->value[1], buf ) );
	    fprintf( fp, "%ld ",                      pObjIndex->value[2]        );
	    fprintf( fp, "%ld ",                      pObjIndex->value[3]        );
	    fprintf( fp, "%ld\n" ,                    pObjIndex->value[4]        );
	    break;

        case ITEM_DRINK_CON:
	    fprintf( fp, "%ld ",                      pObjIndex->value[0]        );
	    fprintf( fp, "%ld ",                      pObjIndex->value[1]        );
	    fprintf( fp, "'%s' ",  liquid_name(       pObjIndex->value[2]      ) );
	    fprintf( fp, "%ld ",                      pObjIndex->value[3]        );
	    fprintf( fp, "'%s'\n", effect_name(       pObjIndex->value[4]      ) );
	    break;

        case ITEM_FOUNTAIN:
	    fprintf( fp, "'%s' ",  liquid_name(       pObjIndex->value[0]      ) );
	    fprintf( fp, "%ld ",                      pObjIndex->value[1]        );
	    fprintf( fp, "%ld ",                      pObjIndex->value[2]        );
	    fprintf( fp, "'%s' ",  effect_name(       pObjIndex->value[3]      ) );
	    fprintf( fp, "%ld\n",                     pObjIndex->value[4]        );
	    break;

        case ITEM_FOOD:
	    fprintf( fp, "%ld ",                      pObjIndex->value[0]        );
	    fprintf( fp, "%ld ",                      pObjIndex->value[1]        );
	    fprintf( fp, "'%s' ",  effect_name(       pObjIndex->value[2]      ) );
	    fprintf( fp, "%ld ",                      pObjIndex->value[3]        );
	    fprintf( fp, "%ld\n",                     pObjIndex->value[4]        );
	    break;

        case ITEM_STAFF:
        case ITEM_WAND:
	    fprintf( fp, "%ld ",                      pObjIndex->value[0]        );
	    fprintf( fp, "%ld ",                      pObjIndex->value[1]        );
	    fprintf( fp, "%ld ",                      pObjIndex->value[2]        );
	    fprintf( fp, "'%s' ",  effect_name(       pObjIndex->value[3]      ) );
	    fprintf( fp, "%ld\n",                     pObjIndex->value[4]        );
	    break;

        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL:
	    fprintf( fp, "%ld ",                      pObjIndex->value[0]        );
	    fprintf( fp, "'%s' ",  effect_name(       pObjIndex->value[1]      ) );
	    fprintf( fp, "'%s' ",  effect_name(       pObjIndex->value[2]      ) );
	    fprintf( fp, "'%s' ",  effect_name(       pObjIndex->value[3]      ) );
	    fprintf( fp, "'%s'\n", effect_name(       pObjIndex->value[4]      ) );
            break;

        case ITEM_LIGHT:
	    fprintf( fp, "%ld ",                      pObjIndex->value[0]        );
	    fprintf( fp, "%ld ",                      pObjIndex->value[1]        );
	    fprintf( fp, "%ld ",                      pObjIndex->value[2]        );
	    fprintf( fp, "%ld ",                      pObjIndex->value[3]        );
	    fprintf( fp, "%ld\n",                     pObjIndex->value[4]        );
	    break;

        case ITEM_HERB:
	    fprintf( fp, "%ld ",                      pObjIndex->value[0]        );
	    fprintf( fp, "'%s' ",  effect_name(       pObjIndex->value[1]      ) );
	    fprintf( fp, "'%s' ",  effect_name(       pObjIndex->value[2]      ) );
	    fprintf( fp, "%ld ",                      pObjIndex->value[3]        );
	    fprintf( fp, "%ld\n",                     pObjIndex->value[4]        );
            break;

        case ITEM_PIPE:
	    fprintf( fp, "%ld ",                      pObjIndex->value[0]        );
	    fprintf( fp, "%ld ",                      pObjIndex->value[1]        );
	    fprintf( fp, "%ld ",                      pObjIndex->value[2]        );
	    fprintf( fp, "'%s' ",  effect_name(       pObjIndex->value[3]      ) );
	    fprintf( fp, "'%s'\n", effect_name(       pObjIndex->value[4]      ) );
            break;

        case ITEM_BOOK:
	    fprintf( fp, "'%s' ",  skill_name(        pObjIndex->value[0]      ) );
	    fprintf( fp, "%ld ",                      pObjIndex->value[1]        );
	    fprintf( fp, "'%s' ",  skill_name(        pObjIndex->value[2]      ) );
	    fprintf( fp, "'%s' ",  skill_name(        pObjIndex->value[3]      ) );
	    fprintf( fp, "'%s'\n", skill_name(        pObjIndex->value[4]      ) );
            break;

    }

    fprintf( fp, "%d ", pObjIndex->level );
    fprintf( fp, "%d ", pObjIndex->weight );
    fprintf( fp, "%d ", pObjIndex->cost );

         if ( pObjIndex->condition >  90 ) letter = 'P';
    else if ( pObjIndex->condition >  75 ) letter = 'G';
    else if ( pObjIndex->condition >  50 ) letter = 'A';
    else if ( pObjIndex->condition >  25 ) letter = 'W';
    else if ( pObjIndex->condition >  10 ) letter = 'D';
    else if ( pObjIndex->condition >   0 ) letter = 'B';
    else                                   letter = 'R';

    fprintf( fp, "%c\n", letter );

    /* Zeran - save out repop % */	
    fprintf( fp, "R %d\n", pObjIndex->repop);

    for( pAf = pObjIndex->affected; pAf; pAf = pAf->next ) {

      if ( pAf->location == APPLY_SKILL ) { 
        fprintf( fp, "S '%s' %d\n",  skill_array[pAf->skill].name,  pAf->modifier );

      } else if ( pAf->location == APPLY_EFFECT ) { 
        fprintf( fp, "P '%s' %d %d\n",  effect_array[pAf->skill].name,  pAf->modifier, pAf->duration );

      } else if ( pAf->bitvector != 0 ) {
        fprintf (fp, "N %d %d %d %lld\n",  -1, pAf->location,  pAf->modifier,  pAf->bitvector);

      } else {
        fprintf( fp, "A %d %d\n", pAf->location,  pAf->modifier );
      } 
    }

    if (pObjIndex->wear_cond)     	write_econd(pObjIndex->wear_cond, fp, "W");
    if (pObjIndex->anchors) 		write_anchors(pObjIndex->anchors, fp, "H");
    if (pObjIndex->artifact_data) 	fprintf(fp, "T %ld %d %d\n", pObjIndex->artifact_data->power, pObjIndex->artifact_data->attract, pObjIndex->artifact_data->pulse);

   /* Extra descriptions... */

    for( pEd = pObjIndex->extra_descr; pEd; pEd = pEd->next )   {
        save_extra(pEd, fp, "D");
    }

   /* Zone mask... */

    if ( pObjIndex->zmask != 0 ) fprintf(fp, "Z %s\n", fwrite_flag(pObjIndex->zmask, buf));
   
    /* URL... */

    if ( pObjIndex->image != NULL ) {
      fprintf(fp, "I '%s'\n", pObjIndex->image);
    }

    return;
}
 



/*****************************************************************************
 Name:		save_objects
 Purpose:	Save #OBJECTS section of an area file.
 Called by:	save_area(olc_save.c).
 Notes:         Changed for ROM OLC.
 ****************************************************************************/
void save_objects( FILE *fp, AREA_DATA *pArea ) {
long i;
OBJ_INDEX_DATA *pObj;

    fprintf( fp, "#NEWOBJECTS\n" );

    for( i = pArea->lvnum; i <= pArea->uvnum; i++ )  {
	if ( (pObj = get_obj_index( i )))  save_object( fp, pObj );
    }

    fprintf( fp, "#0\n\n\n\n" );
    return;
}
 


/* Recursive code to output extra_descriptions backwards... */

void save_all_extras(EXTRA_DESCR_DATA *ed, FILE *fp, char *hdr) {

  if (ed == NULL) {
    return;
  }

  if (ed->next != NULL) {
    save_all_extras(ed->next, fp, hdr);
  }

  save_extra(ed, fp, hdr);
 
  return;
}

/*****************************************************************************
 Name:		save_rooms
 Purpose:	Save #ROOMS section of an area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_rooms( FILE *fp, AREA_DATA *pArea ) {
ROOM_INDEX_DATA *pRoomIndex;
EXIT_DATA *pExit;
char buf[MAX_STRING_LENGTH];
int iHash;
int door;
 
  fprintf( fp, "#ROOMS\n" );
  for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ ) {
    
    for( pRoomIndex = room_index_hash[iHash]; pRoomIndex != NULL; pRoomIndex = pRoomIndex->next ) {
        
      if ( pRoomIndex->area == pArea ) {

        fprintf( fp, "#%ld\n",		pRoomIndex->vnum );
        fprintf( fp, "%s~\n",		pRoomIndex->name );
        fprintf( fp, "%s~\n",		fix_string( pRoomIndex->description ) );
        fprintf( fp, "0 " );
        fprintf( fp, "%lld ",		pRoomIndex->room_flags );
        fprintf( fp, "%d\n",		pRoomIndex->sector_type );

        if (pRoomIndex->subarea > 0) fprintf(fp, "B %d\n",		pRoomIndex->subarea);

        save_all_extras(pRoomIndex->extra_descr, fp, "V");

        if (pRoomIndex->gadgets != NULL) write_gadget(fp, pRoomIndex->gadgets);
        if (pRoomIndex->currents != NULL) save_current(pRoomIndex->currents, fp, "R", "U");

        for( door = 0; door < DIR_MAX; door++ ) {

          if ((pExit = pRoomIndex->exit[door]) && pExit->u1.to_room ) {
            int locks = EX_FILE_NONE;
            int sig_lock;
            int test_lock;
            int carry_lock;

            sig_lock = pExit->rs_flags & EX_MASK_SIGNIFICANT;
  
            carry_lock = sig_lock & EX_MASK_CARRY;

            test_lock = sig_lock & EX_MASK_TEST;

            switch (test_lock) {
              case EX_ISDOOR:
                locks = EX_FILE_DOOR; 
                break;
 
              case EX_ISDOOR + EX_PICKPROOF:
                locks = EX_FILE_DOOR_PP;
                break;

              case EX_ISDOOR + EX_HIDDEN:
                locks = EX_FILE_DOOR_H;
                break;

              case EX_ISDOOR + EX_PICKPROOF + EX_HIDDEN:
                locks = EX_FILE_DOOR_PP_H;
                break;

              case EX_ISDOOR + EX_NO_PASS:
                locks = EX_FILE_DOOR_NP;
                break;

              case EX_ISDOOR + EX_PICKPROOF + EX_NO_PASS:
                locks = EX_FILE_DOOR_PP_NP;
                break;

              case EX_ISDOOR + EX_HIDDEN + EX_NO_PASS:
                locks = EX_FILE_DOOR_H_NP;
                break;

              case EX_ISDOOR + EX_PICKPROOF + EX_HIDDEN + EX_NO_PASS:
                locks = EX_FILE_DOOR_PP_H_NP;
                break;

              case EX_RANDOM:
                locks = EX_FILE_RANDOM;
                break;

              case 0:
                locks = EX_FILE_NONE;
                break; 

              default:  
                bug("Bad test lock %d", test_lock);
                bug("In room %d", pRoomIndex->vnum);
                locks = 0;
                break; 
            } 

            locks += carry_lock;


	    fprintf( fp, "D%d\n",      door );
	    fprintf( fp, "%s~\n",      fix_string(pExit->description));
	    fprintf( fp, "%s~\n",      pExit->keyword );
	    fprintf( fp, "%d %ld %ld\n", locks, pExit->key, pExit->u1.to_room->vnum );

           /* Save trans description... */
	    if (pExit->transition) fprintf( fp, "t %s~\n", pExit->transition);
	
           /* Save visability conditions... */
	    if (pExit->invis_cond != NULL) write_econd(pExit->invis_cond, fp, "I");

           /* Save multi-destination details... */
                    if (pExit->cond_dest != NULL) write_cdd(pExit->cond_dest, fp, "Z", "N");

          }
        }

       /* Heal and Mana rates... */

        if ( pRoomIndex->heal_rate != 100
        || pRoomIndex->mana_rate != 100 ) {
          fprintf(fp, "H %d M %d\n", pRoomIndex->heal_rate, pRoomIndex->mana_rate);
        } 

       /* Room owner... */

        if ( pRoomIndex->owner != NULL ) fprintf(fp, "O %s~\n", pRoomIndex->owner);

       /* Save extended room data... */
 
        fprintf( fp, "X\n");

       /* Save recall, respawn, morgue and dream rooms... */

        if (pRoomIndex->recall != 0) 		fprintf( fp, "Recall %ld\n", pRoomIndex->recall);
        if (pRoomIndex->respawn != 0) 		fprintf( fp, "Respawn %ld\n", pRoomIndex->respawn);
        if (pRoomIndex->morgue != 0) 		fprintf( fp, "Morgue %ld\n", pRoomIndex->morgue);
        if (pRoomIndex->dream != 0) 		fprintf( fp, "Dream %ld\n", pRoomIndex->dream);
        if (pRoomIndex->mare != 0) 		fprintf( fp, "Mare %ld\n", pRoomIndex->mare);
        if (pRoomIndex->night != 0) 		fprintf( fp, "Night %ld\n", pRoomIndex->night);
        if (pRoomIndex->day != 0) 		fprintf( fp, "Day %ld\n", pRoomIndex->day);
        if (pRoomIndex->ptracks != NULL) 	save_tracks(fp, pRoomIndex->ptracks, "PTracks");
        if (pRoomIndex->image != NULL) 	fprintf( fp, "Image '%s'\n", pRoomIndex->image);
        if (pRoomIndex->affected_by != 0) 	fprintf( fp, "Affect %s\n", fwrite_flag( pRoomIndex->affected_by, buf ) );

       /* End of extended data... */

        fprintf( fp, "End\n"); 
        fprintf( fp, "S\n" );
      }
    }
  }
  fprintf( fp, "#0\n\n\n\n" );
  return;
}



/*****************************************************************************
 Name:		save_specials
 Purpose:	Save #SPECIALS section of area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_specials( FILE *fp, AREA_DATA *pArea )
{
    int iHash;
    MOB_INDEX_DATA *pMobIndex;
    
    fprintf( fp, "#SPECIALS\n" );

    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pMobIndex = mob_index_hash[iHash]; pMobIndex; pMobIndex = pMobIndex->next )
        {
            if ( pMobIndex && pMobIndex->area == pArea && pMobIndex->spec_fun )
            {
                fprintf( fp, "M %ld %s Load to: %s\n", pMobIndex->vnum,
                                                       spec_string( pMobIndex->spec_fun ),
                                                       pMobIndex->short_descr );
            }
        }
    }

    fprintf( fp, "S\n\n\n\n" );
    return;
}


void save_door_resets( FILE *fp, AREA_DATA *pArea ) {
int iHash;
ROOM_INDEX_DATA *pRoomIndex;
EXIT_DATA *pExit;
int door;

    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pRoomIndex = room_index_hash[iHash]; 
             pRoomIndex; 
             pRoomIndex = pRoomIndex->next )
        {
            if ( pRoomIndex->area == pArea )
            {
                for( door = 0; door < DIR_MAX; door++ )
                {
		    	int locks=0;
                    if ( ( pExit = pRoomIndex->exit[door] )
                          && pExit->u1.to_room 
                          && ( IS_SET( pExit->rs_flags, EX_CLOSED )
                          || IS_SET( pExit->rs_flags, EX_LOCKED ) ) )
			{ 
                          if ( IS_SET( pExit->rs_flags, EX_ISDOOR )
                               && ( !IS_SET( pExit->rs_flags, EX_LOCKED )))
                              locks = 1;
                          if ( IS_SET( pExit->rs_flags, EX_ISDOOR )
                               && ( IS_SET( pExit->rs_flags, EX_LOCKED )))
                               locks = 2;
                          if ( pExit->key == 0) /* elfren since many keys are -1 */
                          pExit->key = -1; /* maybe it needs it that way.   */
             			fprintf( fp, "D 0 %ld %d %d\n", 
							pRoomIndex->vnum,
							door,
							locks );
						}
		}
	    }
	}
    }
    return;
}



/*****************************************************************************
 Name:		save_resets
 Purpose:	Saves the #RESETS section of an area file.
 Called by:	save_area(olc_save.c)
 ****************************************************************************/
void save_resets( FILE *fp, AREA_DATA *pArea ) {
RESET_DATA *pReset;
MOB_INDEX_DATA *pLastMob = NULL;
OBJ_INDEX_DATA *pLastObj;
ROOM_INDEX_DATA *pRoom;
char buf[MAX_STRING_LENGTH];
int iHash;

    fprintf( fp, "#RESETS\n" );

    save_door_resets( fp, pArea );

    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )   {
        for( pRoom = room_index_hash[iHash]; pRoom != NULL; pRoom = pRoom->next ) {
            if ( pRoom->area == pArea ) {

    for ( pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next ) {
	switch ( pReset->command ) {
	default:
	    bug( "Save_resets: bad command %c.", pReset->command );
	    break;

#define VERBOSE ON
#if defined( VERBOSE )
	case 'M':
                    pLastMob = get_mob_index( pReset->arg1 );
	    fprintf( fp, "M 0 %ld %d %ld Load %s\n",  pReset->arg1, pReset->arg2, pRoom->vnum, pLastMob->short_descr );
                    break;

	case 'O':
                    pLastObj = get_obj_index( pReset->arg1 );
	    fprintf( fp, "O 0 %ld %d %ld %s loaded to %s\n", pReset->arg1, pReset->arg2,  pRoom->vnum, capitalize(pLastObj->short_descr), pRoom->name );
            break;

	case 'P':
                    pLastObj = get_obj_index( pReset->arg1 );
	    fprintf( fp, "P 0 %ld %d %ld %s put inside %s\n", pReset->arg1, pReset->arg2, pReset->arg3, capitalize(get_obj_index( pReset->arg1 )->short_descr), pLastObj->short_descr );
                    break;

	case 'G':
	    fprintf( fp, "G 0 %ld %d %s is given to %s\n", pReset->arg1, pReset->arg2, capitalize(get_obj_index( pReset->arg1 )->short_descr), pLastMob ? pLastMob->short_descr : "!NO_MOB!" );
                    if ( !pLastMob ) {
                        sprintf( buf, "Save_resets: !NO_MOB! in [%s]", pArea->filename );
                        bug( buf, 0 );
                    }
                    break;

	case 'E':
	    fprintf( fp, "E 0 %ld 1 %ld %s is loaded %s of %s\n", pReset->arg1, pReset->arg3, capitalize(get_obj_index( pReset->arg1 )->short_descr), flag_string( wear_loc_strings, pReset->arg3 ), pLastMob ? pLastMob->short_descr : "!NO_MOB!" );
                    if ( !pLastMob ) {
                        sprintf( buf, "Save_resets: !NO_MOB! in [%s]", pArea->filename );
                        bug( buf, 0 );
                    }
                    break;

	case 'D':
                    break;

	case 'R':
                    pRoom = get_room_index( pReset->arg1 );
	    fprintf( fp, "R 0 %ld %d Randomize %s\n", pRoom->vnum, pReset->arg2, pRoom->name );
                    break;
            }
#endif
#if !defined( VERBOSE )
	case 'M':
                    pLastMob = get_mob_index( pReset->arg1 );
	    fprintf( fp, "M 0 %ld %d %ld\n", pReset->arg1, pReset->arg2,  pRoom->vnum );
                    break;

	case 'O':
                    pLastObj = get_obj_index( pReset->arg1 );
                    pRoom = get_room_index( pReset->arg3 );
	    fprintf( fp, "O 0 %ld %d %ld\n", pReset->arg1, pReset->arg2,  pRoom->vnum );
                    break;

	case 'P':
                    pLastObj = get_obj_index( pReset->arg1 );
	    fprintf( fp, "P 0 %ld %d %ld\n", pReset->arg1, pReset->arg2, pReset->arg3  );
                    break;

	case 'G':
	    fprintf( fp, "G 0 %ld %d\n", pReset->arg1, pReset->arg2 );
                    if ( !pLastMob ) {
                         sprintf( buf, "Save_resets: !NO_MOB! in [%s]", pArea->filename );
                         bug( buf, 0 );
                    }
                    break;

	case 'E':
	    fprintf( fp, "E 0 %ld %d %ld\n", pReset->arg1, pReset->arg2, pReset->arg3 );
                    if ( !pLastMob ) {
                        sprintf( buf, "Save_resets: !NO_MOB! in [%s]", pArea->filename );
                        bug( buf, 0 );
                    }
                    break;

	case 'D':
                     break;

	case 'R':
                    pRoom = get_room_index( pReset->arg1 );
	    fprintf( fp, "R 0 %ld %d\n", pRoom->vnum, pReset->arg2 );
                    break;
            }
#endif
        }
	    }	/* End if correct area */
	}	/* End for pRoom */
    }	/* End for iHash */
    fprintf( fp, "S\n\n\n\n" );
    return;
}



/*****************************************************************************
 Name:		save_shops
 Purpose:	Saves the #SHOPS section of an area file.
 Called by:	save_area(olc_save.c)
 ****************************************************************************/
void save_shops( FILE *fp, AREA_DATA *pArea ) {
SHOP_DATA *pShopIndex;
MOB_INDEX_DATA *pMobIndex;
int iTrade;
int iHash;
    
    fprintf( fp, "#SHOPS\n" );

    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )   {
        for( pMobIndex = mob_index_hash[iHash]; pMobIndex; pMobIndex = pMobIndex->next ) {
        
            if ( pMobIndex 
              && pMobIndex->area == pArea 
              && pMobIndex->pShop ) {
            
                pShopIndex = pMobIndex->pShop;

                fprintf( fp, "%ld ", pMobIndex->vnum );

                for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ ) {
                     if ( pShopIndex->buy_type[iTrade] != 0 ) fprintf( fp, "%d ", pShopIndex->buy_type[iTrade] );
                     else fprintf( fp, "0 ");
                }

                fprintf( fp, "%d %d ", pShopIndex->profit_buy, pShopIndex->profit_sell);
                fprintf( fp, "%d %d ", pShopIndex->open_hour, pShopIndex->close_hour);
                fprintf( fp, "%d ", pShopIndex->currency);
                fprintf( fp, "%d\n", pShopIndex->haggle);
            }
        }
    }

    fprintf( fp, "0\n\n\n\n" );
    return;
}



/*****************************************************************************
 Name:		save_area
 Purpose:	Save an area, note that this format is new.
 Called by:	do_asave(olc_save.c).
 ****************************************************************************/
void save_area( AREA_DATA *pArea ) {
FILE *fp;
int flag;
char buf[MAX_STRING_LENGTH];
int i;

    fclose( fpReserve );
    if ( !( fp = fopen( pArea->filename, "w" ) ) )    {
	bug( "Open_area: fopen", 0 );
	perror( pArea->filename );
    }

    if ( pArea->lvnum <= 0  || pArea->uvnum <= 0 ) {
      sprintf(buf, "Area %s NOT SAVED - empty", pArea->name);
      log_string (buf);
      return;
    } 

   /* Update area version... */

    pArea->version = VERSION_CTHULHU_1;	

   /* Write out to the file... */

    sprintf(buf, "Saving: %s", pArea->name);
    log_string (buf);

    fprintf( fp, "#AREADATA\n" );
    fprintf( fp, "Name        %s~\n",      pArea->name );
    fprintf( fp, "Copyright   %s~\n",      pArea->copyright );
    fprintf( fp, "Zone        %d\n",	   pArea->zone );
    fprintf( fp, "Builders    %s~\n",      fix_string( pArea->builders ) );
    fprintf( fp, "Map         %s~\n",      fix_string( pArea->map ) );
    fprintf( fp, "VNUMs       %ld %ld\n",    pArea->lvnum, pArea->uvnum );
    fprintf( fp, "Security    %d\n",       pArea->security );
    fprintf( fp, "Recall      %ld\n",      pArea->recall );
    fprintf( fp, "Respawn     %ld\n",      pArea->respawn );
    fprintf( fp, "Morgue      %ld\n",      pArea->morgue );
    fprintf( fp, "Dream       %ld\n",      pArea->dream );
    fprintf( fp, "Mare       %ld\n",      pArea->mare);
    fprintf( fp, "Prison     %ld\n",      pArea->prison);
    if (pArea->martial > 0) fprintf( fp, "Martial     %d\n",      pArea->martial);
    fprintf( fp, "Climate     %d\n",       pArea->climate );
    fprintf( fp, "Version     %d\n",       pArea->version );
    fprintf( fp, "MAX_CURRENCY    %d\n", MAX_CURRENCY);

    fprintf( fp, "Currency");
    for (i = 0; i < MAX_CURRENCY; i++) fprintf( fp, " %d", pArea->worth[i]);
    fprintf( fp, "\n");

/* Save some area flags... */ 

    flag = pArea->area_flags;

    REMOVE_BIT(flag, AREA_CHANGED);
    REMOVE_BIT(flag, AREA_ADDED);
    REMOVE_BIT(flag, AREA_LOADING);

    if (flag != 0) fprintf( fp, "Flags       %d\n",     flag);
    
    fprintf( fp, "End\n\n\n\n" );
    save_mobiles( fp, pArea );
    save_objects( fp, pArea );
    save_rooms( fp, pArea );
    save_specials( fp, pArea );
    save_resets( fp, pArea );
    save_shops( fp, pArea );
    fprintf( fp, "#$\n" );

    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );

    sprintf (buf, "Finished");
    log_string (buf);

    log_string("Saving leases...");
    save_leases();

    return;
}



/*****************************************************************************
 Name:		do_asave
 Purpose:	Entry point for saving area data.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_asave( CHAR_DATA *ch, char *argument ) {
    char arg1 [MAX_INPUT_LENGTH];
    AREA_DATA *pArea;
    FILE *fp;
    int value;

    char buf[MAX_STRING_LENGTH];

    fp = NULL;

    if ( !ch ) {
	save_area_list();
	for( pArea = area_first; pArea; pArea = pArea->next )	{
	    save_area( pArea );
	    REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
	}
	return;
    }

    smash_tilde( argument );
    strcpy( arg1, argument );

    if ( arg1[0] == '\0' )    {
    send_to_char( "Syntax:\r\n"
                  "  asave <vnum>   - saves a particular area\r\n"
                  "  asave list     - saves the area.lst file\r\n"
                  "  asave area     - saves the area being edited\r\n"
                  "  asave changed  - saves all changed areas\r\n"
                  "  asave world    - saves the world! (db dump)\r\n"
                  "  asave force    - saves the world! (inc. edlocked)\r\n"
                  "\r\n", ch );
        return;
    }

    /* Save area of given vnum. */
    /* ------------------------ */

    if ( is_number( arg1 ) ) {

        value = atoi( arg1 );

        pArea = get_area_data( value );
 
        if ( pArea == NULL ) { 
	  send_to_char( "That area does not exist.\r\n", ch );
	  return;
        }

	if ( !IS_BUILDER( ch, pArea ) ) {
	    send_to_char( "You are not a builder for this area.\r\n", ch );
	    return;
	}

	save_area_list();

        if ( pArea->lvnum == 0
          && pArea->uvnum == 0 ) {
	    send_to_char( "{RArea empty - removed from area list!\r\n{x", ch );
            return;
        }

	save_area( pArea );

	return;
    }

    /* Save the world, only authorized areas. */
    /* -------------------------------------- */

    if ( !str_cmp( "world", arg1 ) ) {

	save_area_list();

	for( pArea = area_first; pArea; pArea = pArea->next ) {
	
	   /* Builder must be assigned this area. */

	    if ( !IS_BUILDER( ch, pArea ) ) {
		continue;	   
            }

            if ( pArea->lvnum == 0
              && pArea->uvnum == 0 ) {  
	      sprintf(buf, "{RArea %3d %s empty - removed from area list!{x\r\n",
                            pArea->vnum, pArea->name );
	      send_to_char(buf, ch );
              continue;
            }

            if ( IS_SET(pArea->area_flags, AREA_EDLOCK)) {

                sprintf(buf, "{YArea %s is edit locked. Not Saved.{x\r\n",
                              pArea->name);
                send_to_char(buf, ch);

                sprintf(buf, "skipping edit locked area %s", pArea->name);
                log_string(buf);  

                continue;
            } 

              save_area( pArea );
              REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
	}

	send_to_char( "You saved the world.\r\n", ch );
/*	send_to_all_char( "Database saved.\r\n" );                 ROM OLC */

	return;
    }

    /* Save the world, only authorized areas, include edit locked ones. */
    /* ---------------------------------------------------------------- */

    if ( !str_cmp( "force", arg1 ) ) {

	save_area_list();

	for( pArea = area_first; pArea; pArea = pArea->next ) {
	
	   /* Builder must be assigned this area. */

	    if ( !IS_BUILDER( ch, pArea ) ) {
		continue;	   
            }

            if ( pArea->lvnum == 0
              && pArea->uvnum == 0 ) {  
	      sprintf(buf, "{RArea %3d %s empty - removed from area list!{x\r\n",
                            pArea->vnum, pArea->name );
	      send_to_char(buf, ch );
              continue;
            }

            if ( IS_SET(pArea->area_flags, AREA_EDLOCK)) {

                sprintf(buf, "{YArea %s is edit locked - {RSaved anyway!{x\r\n",
                              pArea->name);
                send_to_char(buf, ch);

                sprintf(buf, "*** saving edit locked area %s ***", pArea->name);
                log_string(buf);  

            } else { 

                sprintf (buf, "saving area %s", pArea->name);
   	        log_string (buf);
            }

	    save_area( pArea );

            sprintf (buf, "finished saving area %s", pArea->name);
	    log_string (buf);

	    REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
	}

	send_to_char( "You saved the world.\r\n", ch );
/*	send_to_all_char( "Database saved.\r\n" );                 ROM OLC */

	return;
    }

    /* Save changed areas, only authorized areas. */
    /* ------------------------------------------ */

    if ( !str_cmp( "changed", arg1 ) )
    {
	save_area_list();

	send_to_char( "Saved areas:\r\n", ch );
	sprintf( buf, "None.\r\n" );

	for( pArea = area_first; pArea; pArea = pArea->next )
	{
	    /* Builder must be assigned this area. */
	    if ( !IS_BUILDER( ch, pArea ) )
		continue;

	    /* Save changed areas. */
	    if ( IS_SET(pArea->area_flags, AREA_CHANGED) ) {

                if ( pArea->lvnum == 0
                  && pArea->uvnum == 0 ) {  
	      sprintf(buf, "{RArea %3d %s empty - removed from area list!{x\r\n",
                           pArea->vnum, pArea->name );
   	          send_to_char(buf, ch );
                  continue;
                }

                if ( IS_SET(pArea->area_flags, AREA_EDLOCK)) {

                    sprintf(buf, "{YArea %s is edit locked. Not Saved.{x\r\n",
                                  pArea->name);
                    send_to_char(buf, ch);

                    continue;
                } 

		save_area( pArea );
		sprintf( buf, "  %-11.11s - %s\r\n", 
                               pArea->filename,
                               pArea->name ); 
		send_to_char( buf, ch );
		REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
	    }
        }
	if ( !str_cmp( buf, "None.\r\n" ) )
	    send_to_char( buf, ch );
        return;
    }

    /* Save the area.lst file. */
    /* ----------------------- */
    if ( !str_cmp( arg1, "list" ) )
    {
	save_area_list();
        send_to_char("{REmpty areas will have been removed.\r\n", ch);
	return;
    }

    /* Save area being edited, if authorized. */
    /* -------------------------------------- */
    if ( !str_cmp( arg1, "area" ) )
    {
	/* Is character currently editing. */
	if ( ch->desc->editor == 0 )
	{
	    send_to_char( "You are not editing an area, "
		"therefore an area vnum is required.\r\n", ch );
	    return;
	}
	
	/* Find the area to save. */
	switch (ch->desc->editor)
	{
	    case ED_AREA:
		pArea = (AREA_DATA *)ch->desc->pEdit;
		break;
	    case ED_ROOM:
		pArea = ch->in_room->area;
		break;
	    case ED_OBJECT:
		pArea = ( (OBJ_INDEX_DATA *)ch->desc->pEdit )->area;
		break;
	    case ED_MOBILE:
		pArea = ( (MOB_INDEX_DATA *)ch->desc->pEdit )->area;
		break;
	    default:
		pArea = ch->in_room->area;
		break;
	}

	if ( !IS_BUILDER( ch, pArea ) )
	{
	    send_to_char( "You are not a builder for this area.\r\n", ch );
	    return;
	}

	save_area_list();

        if ( pArea->lvnum == 0
          && pArea->uvnum == 0 ) {  
          sprintf(buf, "{RArea %3d %s empty - removed from area list!{x\r\n",
                         pArea->vnum, pArea->name );
          send_to_char(buf, ch );
          return;
        }

        sprintf(buf, "{YArea %s is edit locked - {RSaved anyway!{x\r\n",
                      pArea->name);
        send_to_char(buf, ch);

	save_area( pArea );

	REMOVE_BIT( pArea->area_flags, AREA_CHANGED );

	send_to_char( "Area saved.\r\n", ch );

	return;
    }

    /* Show correct syntax. */
    /* -------------------- */
    do_asave( ch, "" );
    return;
}

void olcautosave( void ) {
   AREA_DATA *pArea;
   DESCRIPTOR_DATA *d;
   char buf[MAX_INPUT_LENGTH];
   bool achanged=FALSE;
         
   for( pArea = area_first; pArea; pArea = pArea->next ) {
        if ( IS_SET(pArea->area_flags, AREA_CHANGED)) achanged=TRUE;
   }    
   if (achanged == FALSE) return;

        save_area_list();
        for ( d = descriptor_list; d != NULL; d = d->next )        {
           if (d->character != NULL) {
                if (IS_IMMORTAL(d->character))  send_to_char( "OLC Autosaving:\r\n", d->character );
           }  
        }
        sprintf( buf, "None.\r\n" );
                 
        for( pArea = area_first; pArea; pArea = pArea->next ) {

            if ( IS_SET(pArea->area_flags, AREA_CHANGED)) {
                save_area(pArea );
                sprintf( buf, "%24s - '%s'\r\n", pArea->name, pArea->filename );
                for ( d = descriptor_list; d != NULL; d = d->next )   {
                   if (d->character != NULL) {
                         if (IS_IMMORTAL(d->character))  send_to_char( buf, d->character );
                   }
                }   
                REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
            }
        }       
        
        if ( !str_cmp( buf, "None.\r\n" ) ) {
           for ( d = descriptor_list; d != NULL; d = d->next )  {
                if (d->character != NULL) {
                     if (IS_IMMORTAL(d->character)) send_to_char(buf, d->character );
                }
           }     
        }
         
        return;
}

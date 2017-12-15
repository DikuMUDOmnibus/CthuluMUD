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
#include "spell.h"
#include "affect.h"
#include "skill.h"
#include "olc.h"
#include "profile.h"
 
/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];

/*
 * Local functions.
 */

void add_open_locker(char *name, OBJ_DATA *obj);
void remove_open_locker(OBJ_DATA *obj);
bool is_locker_open(char *name);
void save_all_lockers();

void save_locker( char *name, OBJ_DATA *cont, bool flush);
void save_obj_to_locker( OBJ_DATA *obj, FILE *fp, int iNest, bool flush);

void load_locker(char *name, OBJ_DATA *cont);
void load_obj_from_locker( FILE *fp, OBJ_DATA *cont );

/* Locker indexing so we can remember which ones are open... */

typedef struct lock_rec LOCK_REC;

struct lock_rec {
  LOCK_REC     *next;
  char         *owner;
  OBJ_DATA     *obj;
};       

static LOCK_REC  locker0;
static int       locker_count = 0;

/* Record a locker as being open... */

void add_open_locker(char *name, OBJ_DATA *obj) {

  LOCK_REC *lr, *nlr;

 /* First locker is always stored in locker0 */ 

  if (locker_count == 0) {

    nlr = &locker0;

    nlr->next  = NULL;
    nlr->owner = str_dup(name);
    nlr->obj   = obj;

    locker_count++;

    return;
  } 

 /* Run through to the last locker in the chain... */

  for(lr = &locker0; lr->next != NULL; lr = lr->next);

 /* Create a new locker record... */

  nlr = (LOCK_REC *) alloc_mem(sizeof(*nlr));

  nlr->next  = NULL;
  nlr->owner = str_dup(name);
  nlr->obj   = obj;

  lr->next = nlr; 

  locker_count++;

  return;
}

/* Remove a record for an open locker... */

void remove_open_locker(OBJ_DATA *obj) {

  LOCK_REC *lr, *dlr; 

 /* Special processing for locker0 */

  lr = &locker0;

  if (lr->obj == obj) {

    free_string(lr->owner);

   /* Copy up the next one... */

    if (lr->next != NULL) { 
      dlr = lr->next;

      lr->next  = dlr->next;
      lr->owner = dlr->owner; 
      lr->obj   = dlr->obj;

      free_mem(dlr, sizeof(LOCK_REC));
    } else {

      lr->next = NULL;
      lr->obj = NULL;
    }

    locker_count--;

    return;  
  }

 /* Go look for the record... */

  for( lr = &locker0; 
      (lr->next != NULL) && (lr->next->obj != obj); 
       lr = lr->next);

 /* Remove if found... */ 

  if ((lr->next != NULL) && (lr->next->obj == obj)) {

    dlr = lr->next;

    lr->next = dlr->next;

    free_string(dlr->owner);

    free_mem(dlr, sizeof(LOCK_REC)); 

    locker_count--;
  }

  return;
}

/* See if a locker is already open... */

bool is_locker_open(char *name) {

  LOCK_REC *lr;

  if (locker_count == 0) {
    return FALSE;
  }

  for( lr = &locker0; 
       lr != NULL; 
       lr = lr->next) {
   
    if (!strcmp(lr->owner, name)) {
      return TRUE;
    }
  } 

  return FALSE;
}

/* Save all open lockers... (used at shutdown) */ 

void save_all_lockers() {

  LOCK_REC *lr;

  if (locker_count == 0) {
    log_string("No open lockers.");
    return;
  }

  lr = &locker0;

  for(lr = &locker0; lr != NULL; lr = lr->next) {

    save_locker( lr->owner, lr->obj, FALSE);

  }

  log_string("Lockers saved.");

  return;
}

/* Save the contents of 'cont' to file 'name.locker' */
 
void save_locker( char *name, OBJ_DATA *cont, bool flush) {

    char strsave[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    FILE *fp;
    bool opened = FALSE;

   /* Name must be specified... */ 

    if (name[0] == '\0') {
      bug("Save_Locker called with no name.",0);
      return;
    }

   /* So must the container... */

    if (cont == NULL) {
      bug("Save_Locker called with null container!",0);
      return;
    }

    if (1 == 0) {
      return;
    } 

   /* Close the temporary file... */ 

    fclose( fpReserve );

   /* Work out the file name... */

    sprintf( strsave, "%s%s.locker", PLAYER_DIR, capitalize(name) );

   /* Open the file... */

    if ( ( fp = fopen( LOCKER_TEMP, "w")) == NULL ) {
    
       /* Warn if problems... */

	bug( "Save_locker: fopen", 0 );
	perror( strsave );

    } else {

        opened = TRUE;

       /* Write out the contents... */ 
    
        if (cont->contains != NULL) {
	  save_obj_to_locker(cont->contains, fp, 0, flush);
        }

       /* Closing tag... */

	fprintf( fp, "#END\n" );
    }

   /* Close the file (even if the open failed?) */ 

    fclose( fp );

    if (opened) {

     /* Rename the file */

      umask(006);

      sprintf(buf,"mv %s %s",LOCKER_TEMP,strsave);

      system(buf);

     /* Compress the file... */ 
 
#if defined(COMPRESS_PFILES)
      sprintf(buf,"gzip -fq %s",strsave);

      system(buf);
#endif
    } else {

     /* Open failed, so remove the temp file (probably un-needed)... */  

      sprintf(buf, "rm %s", LOCKER_TEMP);
 
      system(buf);
    }
 
   /* Reacquire our reserved file handle... */ 

    fpReserve = fopen( NULL_FILE, "r" );

    return;
}


/* Write an object and its contents... */ 
 
void save_obj_to_locker( OBJ_DATA *obj, FILE *fp, int iNest, bool flush ) {
EXTRA_DESCR_DATA *ed;
AFFECT_DATA *paf;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */

    if ( obj->next_content != NULL )
	save_obj_to_locker( obj->next_content, fp, iNest, flush );

   /* Don't save maps and non_purgable keys... */ 

    if ( (obj->item_type == ITEM_KEY && !ITEM_NOPURGE)
    ||   (obj->item_type == ITEM_MAP && !obj->value[0]))
	return;

   /* Write the standard block out... */

    fprintf( fp, "#O\n" );

    fprintf( fp, "Vnum %ld\n", obj->pIndexData->vnum);

    if (!obj->pIndexData->new_format)
	fprintf( fp, "Oldstyle\n");

    if (obj->enchanted)
	fprintf( fp,"Enchanted\n");

    fprintf( fp, "Nest %d\n",	iNest);

    fprintf( fp, "Size   %d\n", obj->size);

   /* these data are only used if they do not match the defaults */

    if ( obj->name != obj->pIndexData->name)
    	fprintf( fp, "Name %s~\n", obj->name);

    if ( obj->short_descr != obj->pIndexData->short_descr)
        fprintf( fp, "ShD  %s~\n", obj->short_descr);

    if ( obj->description != obj->pIndexData->description)
        fprintf( fp, "Desc %s~\n", obj->description);

    if ( obj->extra_flags != obj->pIndexData->extra_flags)
        fprintf( fp, "ExtF %lld\n", obj->extra_flags);

    if ( obj->wear_flags != obj->pIndexData->wear_flags)
        fprintf( fp, "WeaF %d\n", obj->wear_flags);

    if ( obj->item_type != obj->pIndexData->item_type)
        fprintf( fp, "Ityp %d\n", obj->item_type);

    if ( obj->weight != obj->pIndexData->weight)
        fprintf( fp, "Wt   %d\n", obj->weight);

    /* variable data */

    fprintf( fp, "Wear %d\n", obj->wear_loc);

    if (obj->level != 0)
        fprintf( fp, "Lev  %d\n", obj->level);

    if (obj->timer != 0)
        fprintf( fp, "Time %d\n", obj->timer);

    fprintf( fp, "Cost %d\n", obj->cost);

    fprintf( fp, "Cond %d\n", obj->condition);

    fprintf( fp, "Val  %ld %ld %ld %ld %ld\n",
                  obj->value[0], obj->value[1], obj->value[2],
                  obj->value[3], obj->value[4]);

    fprintf( fp, "ValOrig  %ld %ld %ld %ld %ld\n",
                  obj->valueorig[0], obj->valueorig[1], obj->valueorig[2], 
                  obj->valueorig[3], obj->valueorig[4]);

   /* Save spells... */

    switch ( obj->item_type ) {
    
      case ITEM_POTION:
      case ITEM_SCROLL:
	if ( obj->value[1] > 0 ) {
	
	    fprintf( fp, "Spell 1 '%s'\n", 
		effect_array[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 ) {
	
	    fprintf( fp, "Spell 2 '%s'\n", 
		effect_array[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 ) {
	
	    fprintf( fp, "Spell 3 '%s'\n", 
		effect_array[obj->value[3]].name );
	}

	break;

      case ITEM_PILL:
      case ITEM_STAFF:
      case ITEM_WAND:
	if ( obj->value[3] > 0 ) {
	
	    fprintf( fp, "Spell 3 '%s'\n", 
		effect_array[obj->value[3]].name );
	}

	break;
    }

   /* Save affects... */ 
  
    for ( paf = obj->affected; paf != NULL; paf = paf->next ) {
    
        if (paf->afn != AFFECT_UNDEFINED) {
          if (paf->location == APPLY_SKILL) { 
            if ( validSkill(paf->skill) && paf->skill > 0 ) {
              fprintf( fp, "Affect4 '%s' %3d %3d %3d %3d '%s' %lld\n", affect_array[paf->afn].name, paf->level,  paf->duration, paf->modifier, paf->location, skill_array[paf->skill].name, paf->bitvector );
            } 
          } else if (paf->location == APPLY_EFFECT) { 
            if ( valid_effect(paf->skill) && paf->skill > 0 ) {
      	      fprintf( fp, "Affect5 '%s' %3d %3d %3d %3d '%s' %lld\n", affect_array[paf->afn].name, paf->level, paf->duration, paf->modifier, paf->location, effect_array[paf->skill].name, paf->bitvector );
            }
          } else {
            fprintf( fp, "Affect3 '%s' %3d %3d %3d %3d %lld\n",
	      affect_array[paf->afn].name,
	      paf->level,
	      paf->duration,
	      paf->modifier,
	      paf->location,
	      paf->bitvector );
         }
       }
    }

   /* Save extra descriptions... */ 

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next ) {
    
	fprintf( fp, "ExDe %s~ %s~\n",
	    ed->keyword, ed->description );
    }

   /* End of object... */ 

    fprintf( fp, "End\n\n" );

   /* If it contains anything, save them next... */ 

    if ( obj->contains != NULL )
	save_obj_to_locker( obj->contains, fp, iNest + 1, flush );

   /* Lose the object now that we've saved it... */

    if (flush) {
      extract_obj(obj);
    }

   /* All done... */ 

    return;
}


void save_obj_to_trans( OBJ_DATA *obj, FILE *fp, int iNest, bool flush ) {
EXTRA_DESCR_DATA *ed;
AFFECT_DATA *paf;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */

    if ( obj->next_content != NULL ) save_obj_to_trans( obj->next_content, fp, 0, flush );

     if (obj->item_type != ITEM_KEY
     && obj->item_type != ITEM_MAP
     && obj->item_type != ITEM_IDOL
     && obj->item_type != ITEM_ROOM_KEY
     && obj->item_type != ITEM_SHARE
     && obj->item_type != ITEM_MAP
     && obj->item_type != ITEM_FIGURINE
     && obj->item_type != ITEM_FOCUS) return;

   /* Write the standard block out... */

    fprintf( fp, "#O\n" );

    fprintf( fp, "Vnum %ld\n", obj->pIndexData->vnum);

    if (!obj->pIndexData->new_format) fprintf( fp, "Oldstyle\n");

    if (obj->enchanted) fprintf( fp,"Enchanted\n");

    fprintf( fp, "Nest %d\n",	0);

    fprintf( fp, "Size   %d\n", obj->size);

   /* these data are only used if they do not match the defaults */

    if ( obj->name != obj->pIndexData->name) 	fprintf( fp, "Name %s~\n", obj->name);
    if ( obj->short_descr != obj->pIndexData->short_descr)  fprintf( fp, "ShD  %s~\n", obj->short_descr);
    if ( obj->description != obj->pIndexData->description)  fprintf( fp, "Desc %s~\n", obj->description);
    if ( obj->extra_flags != obj->pIndexData->extra_flags)  fprintf( fp, "ExtF %lld\n", obj->extra_flags);
    if ( obj->wear_flags != obj->pIndexData->wear_flags) fprintf( fp, "WeaF %d\n", obj->wear_flags);
    if ( obj->item_type != obj->pIndexData->item_type)  fprintf( fp, "Ityp %d\n", obj->item_type);
    if ( obj->weight != obj->pIndexData->weight)  fprintf( fp, "Wt   %d\n", obj->weight);

    /* variable data */

    fprintf( fp, "Wear %d\n", obj->wear_loc);

    if (obj->level != 0) fprintf( fp, "Lev  %d\n", obj->level);

    if (obj->timer != 0) fprintf( fp, "Time %d\n", obj->timer);

    fprintf( fp, "Cost %d\n", obj->cost);

    fprintf( fp, "Cond %d\n", obj->condition);

    fprintf( fp, "Val  %ld %ld %ld %ld %ld\n", obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4]);
    fprintf( fp, "ValOrig  %ld %ld %ld %ld %ld\n", obj->valueorig[0], obj->valueorig[1], obj->valueorig[2],  obj->valueorig[3], obj->valueorig[4]);

    for ( paf = obj->affected; paf != NULL; paf = paf->next ) {
        if (paf->afn != AFFECT_UNDEFINED) {
          if (paf->location == APPLY_SKILL) { 
            if ( validSkill(paf->skill)  && paf->skill > 0 ) {
              fprintf( fp, "Affect4 '%s' %3d %3d %3d %3d '%s' %lld\n", affect_array[paf->afn].name, paf->level, paf->duration, paf->modifier, paf->location, skill_array[paf->skill].name, paf->bitvector );
            } 
          } else if (paf->location == APPLY_EFFECT) { 
            if ( valid_effect(paf->skill) && paf->skill > 0 ) {
      	      fprintf( fp, "Affect5 '%s' %3d %3d %3d %3d '%s' %lld\n", affect_array[paf->afn].name, paf->level, paf->duration, paf->modifier, paf->location, effect_array[paf->skill].name, paf->bitvector );
            }
          } else {
            fprintf( fp, "Affect3 '%s' %3d %3d %3d %3d %lld\n", affect_array[paf->afn].name, paf->level, paf->duration, paf->modifier, paf->location, paf->bitvector );
         }
       }
    }

   /* Save extra descriptions... */ 

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next ) {
    	fprintf( fp, "ExDe %s~ %s~\n", ed->keyword, ed->description );
    }

   /* End of object... */ 

    fprintf( fp, "End\n\n" );

   /* If it contains anything, save them next... */ 

    if ( obj->contains != NULL ) save_obj_to_trans( obj->contains, fp, 0, flush );

   /* Lose the object now that we've saved it... */

    if (flush) extract_obj(obj);
    return;
}


/* Load a locker... */

void load_locker( char *name, OBJ_DATA *cont) {

    char strsave[MAX_INPUT_LENGTH];
    char buf[100];
    FILE *fp;
    bool found;

   /* Name must be specified... */ 

    if (name[0] == '\0') {
      bug("Load_Locker called with no name.",0);
      return;
    }

   /* So must the container... */

    if (cont == NULL) {
      bug("Load_Locker called with null container!",0);
      return;
    }

    if (1 == 0) {
      return;
    } 

   /* Go see if the file exits... */

    found = FALSE;
    fclose( fpReserve );
    
#if defined(COMPRESS_PFILES)
   /* decompress if .gz file exists */

    sprintf( strsave, "%s%s.locker%s", PLAYER_DIR, capitalize(name),".gz");

    if ( ( fp = fopen( strsave, "r" ) ) != NULL ) {
    
	fclose(fp);

	sprintf(buf,"gzip -dfq %s",strsave);

	system(buf);
    }
#endif

   /* Now see if the locker exists... */

    sprintf( strsave, "%s%s.locker", PLAYER_DIR, capitalize(name) );

    fp = fopen( strsave, "r" );

   /* Load and parse... */ 

    if ( fp != NULL ) {
    
       /* Clear nested object table... */  
       
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;

       /* Parse and process... */ 

	for ( ; ; )
	{
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
		bug("Load_char_obj: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );

	    if ( !str_cmp(word, "O")) {
              load_obj_from_locker( fp, cont);
            } else {
	      if ( !str_cmp(word, "END")) {
                break;
              } else { 
		bug( "Load_Locker: bad section.", 0 );
		break;
              }
	    }
	}

       /* Close the file... */ 

	fclose( fp );
    }

   /* Reclaim reserved file handle... */

    fpReserve = fopen( NULL_FILE, "r" );

   /* Recompress the file... */ 
 
#if defined(COMPRESS_PFILES)
      sprintf(buf,"gzip -fq %s",strsave);

      system(buf);
#endif

   /* All done... */ 

    return;
}

/* Marco to handle a simple keyword... */

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


/* Load an object... */

void load_obj_from_locker( FILE *fp, OBJ_DATA *cont) {
OBJ_DATA *obj;
const char *word;
int iNest;
bool fMatch;
bool fNest;
bool fVnum;
bool first;
    
    fVnum = FALSE;
    obj = NULL;
    first = TRUE;  /* used to counter fp offset */
     
    fMatch = FALSE;

   /* Read the first word... */

    word   = feof( fp ) ? "End" : fread_word( fp );

   /* MUST be "Vnum" */ 

    if (!str_cmp(word,"Vnum" ))
    {
        int vnum;
	first = FALSE;  /* fp will be in right place */
 
        vnum = fread_number( fp );
        if (  get_obj_index( vnum )  == NULL )
	{
            bug( "Fread_obj: bad vnum %d.", vnum );
	}
        else
	{
	    obj = create_object(get_obj_index(vnum),-1);
	}
	    
    }

    if (obj == NULL) {
      bug("Bad object in locker!",0);
      return;
    } 

    fNest		= FALSE;
    fVnum		= TRUE;
    iNest		= 0;

   /* Chew through the data value lines... */

    for ( ; ; ) {
    
	if (first)
	    first = FALSE;
	else
	    word   = feof( fp ) ? "End" : fread_word( fp );

	fMatch = FALSE;

	switch ( UPPER(word[0]))	{
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
                    || !str_cmp(word, "Affect5"))  {
		AFFECT_DATA *paf;
                                char *tword = str_dup(word);

                                paf = new_affect();

		if ( !str_cmp(tword, "Affect3")
		|| !str_cmp(tword, "Affect4") 
		|| !str_cmp(tword, "Affect5")) {
                                        int afn;
		  afn = get_affect_afn(fread_word(fp));
		  if (afn != AFFECT_UNDEFINED) {
                                         paf->afn = afn;
                                  } else {
                                         bug("Fread_obj: unknown affect",0); 
                                  }
                                } else {
		  if (!str_cmp(tword, "AffD")) {
		    int sn;
		    sn = get_effect_efn(fread_word(fp));
		    if (sn < 0) bug("Fread_obj: unknown effect.",0);
		    else paf->afn = get_affect_afn_for_effect(sn);
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
	    KEY( "Cost",		obj->cost,	fread_number( fp ) );
                    KEY( "Cond",    	obj->condition,  	fread_number (fp) );
	    break;

	case 'D':
	    KEY( "Description",	obj->description,	fread_string( fp ) );
	    KEY( "Desc",		obj->description,	fread_string( fp ) );
	    break;

	case 'E':

	    if ( !str_cmp( word, "Enchanted")) {
		obj->enchanted = TRUE;
	 	fMatch 	= TRUE;
		break;
	    }

	    KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );
	    KEY( "ExtF",		obj->extra_flags,	fread_number_long_long( fp ) );

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

	    if ( !str_cmp( word, "End" ))   {
                if ( !fNest || ( fVnum && obj->pIndexData == NULL ) )
                {
                    bug( "Fread_obj: incomplete object.", 0 );
					extract_obj (obj);
                    return;
                }
                else
                {
                    if (!obj->pIndexData->new_format
                    && obj->item_type == ITEM_ARMOR
                    &&  obj->value[1] == 0)
                    {
                        obj->value[1] = obj->value[0];
                        obj->value[2] = obj->value[0];
                    }

                    if ( iNest == 0 || rgObjNest[iNest] == NULL )
                        obj_to_obj( obj, cont );
                    else
                        obj_to_obj( obj, rgObjNest[iNest-1] );
                    return;
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

	case 'S':
	    KEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );
	    KEY( "ShD",		obj->short_descr,	fread_string( fp ) );
            KEY( "Size", 	obj->size,		fread_number(fp));

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

	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WeaF",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Wear",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    KEY( "Wt",		obj->weight,		fread_number( fp ) );
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


void load_obj_from_trans(FILE *fp, CHAR_DATA *ch) {
OBJ_DATA *obj;
const char *word;
bool fMatch;
bool fVnum;
bool first;
    
    fVnum = FALSE;
    obj = NULL;
    first = TRUE; 
    fMatch = FALSE;

   /* Read the first word... */

    word   = feof( fp ) ? "End" : fread_word( fp );

    if (!str_cmp(word,"Vnum" ))  {
        int vnum;
	first = FALSE;  /* fp will be in right place */
 
        vnum = fread_number( fp );
        if (  get_obj_index( vnum )  == NULL ) {
            bug( "Fread_obj: bad vnum %d.", vnum );
        } else {
	    obj = create_object(get_obj_index(vnum),-1);
        }
	    
    }

    if (obj == NULL) {
      bug("Bad object in locker!",0);
      return;
    } 

    fVnum		= TRUE;

   /* Chew through the data value lines... */

    for ( ; ; ) {
    
	if (first)
	    first = FALSE;
	else
	    word   = feof( fp ) ? "End" : fread_word( fp );

	fMatch = FALSE;

	switch ( UPPER(word[0])) {
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
                                char *tword = str_dup(word);

                paf = new_affect();

		if ( !str_cmp(tword, "Affect3")
		|| !str_cmp(tword, "Affect4") 
		|| !str_cmp(tword, "Affect5")) {
                                  int afn;
		  afn = get_affect_afn(fread_word(fp));
		  if (afn != AFFECT_UNDEFINED) {
                                       paf->afn = afn;
                                  } else {
                                       bug("Fread_obj: unknown affect",0); 
                                  }
                               } else {
		  if (!str_cmp(tword, "AffD"))  {
		    int sn;
		    sn = get_effect_efn(fread_word(fp));
		    if (sn < 0) bug("Fread_obj: unknown effect.",0);
		    else paf->afn = get_affect_afn_for_effect(sn);
		  } else {
		    paf->type	= fread_number( fp );
                                  }
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
            KEY( "Cond",    	obj->condition,  	fread_number (fp) );
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

		if ( extra_descr_free == NULL ){
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

	    if ( !str_cmp( word, "End" ) )   {
                if (fVnum && obj->pIndexData == NULL) {
                    bug( "Fread_obj: incomplete object.", 0 );
	    extract_obj (obj);
                    return;
                } else  {
                    if (obj->zmask != 0 && (obj->zmask & zones[ch->in_zone].mask ) == 0 ) {
                         obj_to_char_ooz(obj, ch );
                    } else {
                         obj_to_char( obj, ch );
                    }
                    return;
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

	    if ( !str_cmp( word, "Nest" ))  {
		fread_number( fp );
		fMatch = TRUE;
	    }
	    break;

	case 'S':
	    KEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );
	    KEY( "ShD",		obj->short_descr,	fread_string( fp ) );
            KEY( "Size", 	obj->size,		fread_number(fp));

	    if ( !str_cmp( word, "Spell" ))  {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = get_effect_efn( fread_word( fp ) );
		if ( iValue < 0 || iValue > 3 ) {
		    bug( "Fread_obj: bad iValue %d.", iValue );
		} else if ( sn < 0 ) {
		    bug( "Fread_obj: unknown effect.", 0 );
		} else {
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

	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WeaF",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Wear",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    KEY( "Wt",		obj->weight,		fread_number( fp ) );
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

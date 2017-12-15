/*
 * CthulhuMud
 */

/* files used in db.c */

/* vals from db.c */
extern bool fBootDb;
extern int		newmobs;
extern int		newobjs;
extern MOB_INDEX_DATA 	* mob_index_hash          [MAX_KEY_HASH];
extern OBJ_INDEX_DATA 	* obj_index_hash          [MAX_KEY_HASH];
extern int		top_mob_index;
extern int		top_obj_index;
extern int  		top_affect;
extern int		top_ed; 

/* func from db.c */
extern void assign_area_vnum( int vnum );


/* from db2.c */
extern int	social_count;

/* from db2.c */

void convert_mobile( MOB_INDEX_DATA *pMobIndex ); 
void convert_objects( void );                             
void convert_object( OBJ_INDEX_DATA *pObjIndex );

/* magic.h */
DECLARE_SPELL_FUN ( spell_null );

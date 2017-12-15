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
#include "econd.h"
#include "doors.h"
#include "deeds.h"
#include "triggers.h"
#include "monitor.h"
#include "tracks.h"
#include "skill.h"


/*
 * Globals
 */
extern          int                     top_reset;
extern          int                     top_area;
extern          int                     top_exit;
extern          int                     top_ed;
extern          int                     top_room;

AREA_DATA		*area_free = NULL;
EXTRA_DESCR_DATA	*extra_descr_free	= NULL;
EXIT_DATA		*exit_free = NULL;
ROOM_INDEX_DATA	*room_index_free = NULL;
OBJ_INDEX_DATA	*obj_index_free = NULL;
SHOP_DATA		*shop_free = NULL;
MOB_INDEX_DATA	*mob_index_free = NULL;
RESET_DATA		*reset_free = NULL;
HELP_DATA		*help_free = NULL;
extern HELP_DATA	*help_last;



void 		free_customer		(CUSTOMER *cust);


RESET_DATA *new_reset_data( void ){
    RESET_DATA *pReset;

    if ( !reset_free ) {
        pReset          =   (RESET_DATA *) alloc_perm( sizeof(*pReset) );
        top_reset++;
    } else {
        pReset          =   reset_free;
        reset_free      =   reset_free->next;
    }

    pReset->next        =   NULL;
    pReset->command     =   'X';
    pReset->arg1        =   0;
    pReset->arg2        =   0;
    pReset->arg3        =   0;

    return pReset;
}



void free_reset_data( RESET_DATA *pReset ) {
    pReset->next            = reset_free;
    reset_free              = pReset;
    return;
}



AREA_DATA *new_area( void ) {
AREA_DATA *pArea;
char buf[MAX_INPUT_LENGTH];
int i;

    if ( !area_free ) {
        pArea   =   (AREA_DATA *) alloc_perm( sizeof(*pArea) );
        top_area++;
    }  else  {
        pArea       =   area_free;
        area_free   =   area_free->next;
    }

    pArea->next			= NULL;
    pArea->name			= str_dup( "New area" );
    pArea->copyright		= str_dup( "None" );
    pArea->zone			= 0;
    pArea->area_flags		= AREA_ADDED;
    pArea->security		= 1;
    pArea->builders		= str_dup( "None" );
    pArea->map       		= str_dup("None");
    pArea->lvnum		= 0;
    pArea->uvnum		= 0;
    pArea->age			= 0;
    pArea->nplayer		= 0;
    pArea->empty		= TRUE;              /* ROM patch */

    pArea->vnum			= top_area-1;

    pArea->weather		= WEATHER_SUNNY_DAY;
    pArea->old_weather		= WEATHER_SUNNY_DAY;
    pArea->climate		= CLIMATE_EUROPEAN;
    pArea->old_weather		= SURFACE_CLEAR;
     
    sprintf( buf, "area%d.are", pArea->vnum );

    pArea->filename		= str_dup( buf );

    pArea->recall		= 0;
    pArea->respawn		= 0;
    pArea->morgue		= 0;
    pArea->dream		= 0;
    pArea->invasion = FALSE;
    pArea->martial = 0;
    for (i = 0; i < MAX_CURRENCY; i++) pArea->worth[i] = -1;
    pArea->version		= VERSION_CTHULHU_CUR;	

    return pArea;
}



void free_area( AREA_DATA *pArea ) {
    free_string( pArea->name );
    free_string( pArea->filename );
    free_string( pArea->builders );
    free_string( pArea->map);

    pArea->next         =   area_free->next;
    area_free           =   pArea;
    return;
}



EXIT_DATA *new_exit( void ) {
EXIT_DATA *pExit;

    if ( !exit_free ){
        pExit           =   (EXIT_DATA *) alloc_perm( sizeof(*pExit) );
        top_exit++;
    } else {
        pExit           =   exit_free;
        exit_free       =   exit_free->next;
    }

    pExit->u1.to_room   	=   NULL; 
    pExit->next         	=   NULL;
    pExit->exit_info    	=   0;
    pExit->key          	=   0;
    pExit->keyword      	=   &str_empty[0];
    pExit->description  	=   &str_empty[0];
    pExit->transition  	=   NULL;
    pExit->rs_flags     	=   0;
    pExit->invis_cond   	=   NULL;
    pExit->cond_dest	=   NULL;

    return pExit;
}



void free_exit( EXIT_DATA *pExit ) {
    if (pExit->keyword != &str_empty[0]) free_string( pExit->keyword );
    pExit->keyword = &str_empty[0];

    if (pExit->description != &str_empty[0]) free_string( pExit->description );
    pExit->description = &str_empty[0];

    free_string(pExit->transition);
 
    if (pExit->invis_cond != NULL) free_ec(pExit->invis_cond);
    pExit->invis_cond = NULL;

    if (pExit->cond_dest != NULL) free_cdd(pExit->cond_dest); 
    pExit->cond_dest = NULL; 

    pExit->next         =   exit_free;
    exit_free           =   pExit;
    return;
}



EXTRA_DESCR_DATA *new_extra_descr( void )
{
    EXTRA_DESCR_DATA *pExtra;

    if ( !extra_descr_free )
    {
        pExtra              =   (EXTRA_DESCR_DATA *) alloc_perm( sizeof(*pExtra) );
        top_ed++;
    }
    else
    {
        pExtra              =   extra_descr_free;
        extra_descr_free    =   extra_descr_free->next;
    }

    pExtra->keyword         =   NULL;
    pExtra->description     =   NULL;
    pExtra->next            =   NULL;
    pExtra->deed	    =	NULL;

    return pExtra;
}



void free_extra_descr( EXTRA_DESCR_DATA *pExtra ) {
    ECOND_DATA *ec;

    free_string( pExtra->keyword );
    free_string( pExtra->description );

    while (pExtra->ec != NULL) {
      ec = pExtra->ec->next;
      free_ec(pExtra->ec);
      pExtra->ec = ec;
    }

    if (pExtra->deed != NULL) {
      free_deed(pExtra->deed);
      pExtra->deed = NULL;
    }

    pExtra->next        =   extra_descr_free;
    extra_descr_free    =   pExtra;
    return;
}


ROOM_INDEX_DATA *new_room_index( void ) {
    ROOM_INDEX_DATA *pRoom;
    int door;

    if ( !room_index_free )
    {
        pRoom           =   (ROOM_INDEX_DATA *) alloc_perm( sizeof(*pRoom) );
        top_room++;
    }
    else
    {
        pRoom           =   room_index_free;
        room_index_free =   room_index_free->next;
    }

    pRoom->next             =   NULL;
    pRoom->next_collected   =   NULL;
    pRoom->people           =   NULL;
    pRoom->contents         =   NULL;
    pRoom->extra_descr      =   NULL;
    pRoom->area             =   NULL;
    pRoom->subarea          =   0;

    for ( door=0; door < DIR_MAX; door++ )
        pRoom->exit[door]   =   NULL;

    pRoom->name             =   &str_empty[0];
    pRoom->description      =   &str_empty[0];
    pRoom->vnum             =   0;
    pRoom->room_flags       =   0;
    pRoom->light            =   0;
    pRoom->sector_type      =   0;

    pRoom->owner	    =   NULL;
    pRoom->rented_by	    =   NULL;
    pRoom->room_rent 	    =   0;
    pRoom->paid_month	    =   time_info.month;
    pRoom->paid_day	    =   time_info.day;
    pRoom->paid_month	    =   time_info.year;

    pRoom->gadgets 	= NULL;
    pRoom->monitors	= NULL;
    pRoom->currents	= NULL;

    pRoom->recall	= 0;
    pRoom->respawn	= 0;
    pRoom->morgue	= 0;
    pRoom->dream	= 0;

    pRoom->tracks	= NULL;
    pRoom->ptracks	= NULL;

    pRoom->affected_by  = 0;

    pRoom->heal_rate    = 100;
    pRoom->mana_rate    = 100;

    return pRoom;
}



void free_room_index( ROOM_INDEX_DATA *pRoom ) {
    int door, c;
    EXTRA_DESCR_DATA *pExtra;
    RESET_DATA *pReset;

    free_string( pRoom->name );
    free_string( pRoom->description );

    for ( door = 0; door < DIR_MAX; door++ )  {
        if ( pRoom->exit[door] )
            free_exit( pRoom->exit[door] );
    }

    c=0;
    for ( pExtra = pRoom->extra_descr; pExtra; pExtra = pExtra->next )   {
        free_extra_descr( pExtra );
        if (c++ >100) break;
    }

    c=0;
    for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )   {
        free_reset_data( pReset );
        if (c++ >100) break;
    }

    pRoom->next     =   room_index_free;
    room_index_free =   pRoom;

    if (pRoom->tracks != NULL) {
      free_tracks(pRoom->tracks);
      pRoom->tracks = NULL;
    }

    if (pRoom->ptracks != NULL) {
      free_tracks(pRoom->ptracks);
      pRoom->ptracks = NULL;
    }

    return;
}



AFFECT_DATA *new_affect( void ) {
    AFFECT_DATA *pAf;

    if ( affect_free == NULL )   {
        pAf             =   (AFFECT_DATA *) alloc_perm( sizeof(*pAf) );
        top_affect++;
    } else {
        pAf             =   affect_free;
        affect_free     =   affect_free->next;
    }

    pAf->next       =   NULL;
    pAf->location   =   0;
    pAf->modifier   =   0;
    pAf->type       =   0;
    pAf->duration   =   0;
    pAf->where      =   APPLY_TO_AFFECTS;

    pAf->bitvector  =   0;

    return pAf;
}


void free_affect( AFFECT_DATA* pAf ) {
    pAf->next           = affect_free;
    affect_free         = pAf;
    return;
}


SHOP_DATA *new_shop( void ) {
    SHOP_DATA *pShop;
    int buy;

    if ( !shop_free ) {
        pShop           =   (SHOP_DATA *) alloc_perm( sizeof(*pShop) );
        top_shop++;

    } else  {
        pShop           =   shop_free;
        shop_free       =   shop_free->next;
    }

    pShop->next         =   NULL;
    pShop->keeper       =   0;

    for ( buy=0; buy<MAX_TRADE; buy++ )
        pShop->buy_type[buy]    =   0;

    pShop->profit_buy   	=   100;
    pShop->profit_sell  	=   100;
    pShop->open_hour    	=   0;
    pShop->close_hour   	=   23;
    pShop->currency 	=   -1;
    pShop->haggle 	=   50;
    pShop->customer_list 	=   NULL;
    return pShop;
}


void free_shop( SHOP_DATA *pShop ) {
CUSTOMER *cust;
CUSTOMER *cust_next;

    if (pShop->customer_list) {
        for (cust = pShop->customer_list; cust; cust = cust_next) {
               cust_next = cust->next;
               free_customer(cust);
        }
        pShop->customer_list = NULL;
    }

    pShop->next = shop_free;
    shop_free   = pShop;
    return;
}


OBJ_INDEX_DATA *new_obj_index( void ) {
ECOND_DATA *ec;
OBJ_INDEX_DATA *pObj;
int value;

    if ( !obj_index_free )    {
        pObj           =   (OBJ_INDEX_DATA *) alloc_perm( sizeof(*pObj) );
        top_obj_index++;
    } else  {
        pObj            =   obj_index_free;
        obj_index_free  =   obj_index_free->next;
    }

    pObj->next          =   NULL;
    pObj->extra_descr   =   NULL;
    pObj->affected      =   NULL;
    pObj->area          =   NULL;
    pObj->name          =   str_dup( "no name" );
    pObj->short_descr   =   str_dup( "(no short description)" );
    pObj->description   =   str_dup( "(no description)" );
    pObj->vnum          =   0;
    pObj->item_type     =   ITEM_TRASH;
    pObj->extra_flags   =   0;
    pObj->wear_flags    =   0;
    pObj->count         =   0;
    pObj->weight        =   0;
    pObj->cost          =   0;
    pObj->material      =   material_lookup( "" ); 
    pObj->condition     =   100;
    for ( value = 0; value < 5; value++ ) pObj->value[value]  =   0;
    pObj->zmask 	=   0;

    while (pObj->wear_cond) {
        ec = pObj->wear_cond->next;
        free_ec(pObj->wear_cond);
        pObj->wear_cond = ec;
    }

    pObj->new_format    = TRUE;

    return pObj;
}


void free_obj_index( OBJ_INDEX_DATA *pObj ) {
ECOND_DATA *ec;
EXTRA_DESCR_DATA *pExtra;
AFFECT_DATA *pAf;

    free_string( pObj->name );
    free_string( pObj->short_descr );
    free_string( pObj->description );

    for ( pAf = pObj->affected; pAf; pAf = pAf->next )  {
        free_affect( pAf );
    }

    for ( pExtra = pObj->extra_descr; pExtra; pExtra = pExtra->next )   {
        free_extra_descr( pExtra );
    }

    for (ec = pObj->wear_cond; ec; ec = ec->next )   {
        free_ec(ec);
    }

    if (pObj->anchors) pObj->anchors = NULL;
    if (pObj->artifact_data) pObj->artifact_data = NULL;

    pObj->next              = obj_index_free;
    obj_index_free          = pObj;
    return;
}


ARTIFACT *new_artifact( void ) {
ARTIFACT *art;

    art = (ARTIFACT *) alloc_perm( sizeof(*art));

    art->next	= NULL;
    art->power	= 0;
    art->attract	= 0;
    art->pulse	= -1;
    return art;
}


MOB_INDEX_DATA *new_mob_index( void ) {
    MOB_INDEX_DATA *pMob;

    if ( !mob_index_free )    {
        pMob           =   (MOB_INDEX_DATA *) alloc_perm( sizeof(*pMob) );
        top_mob_index++;

    }    else    {
        pMob            =   mob_index_free;
        mob_index_free  =   mob_index_free->next;
    }

    pMob->next          =   NULL;
    pMob->spec_fun      =   NULL;
    pMob->pShop         =   NULL;
    pMob->area          =   NULL;
    pMob->player_name   =   str_dup( "no name" );
    pMob->short_descr   =   str_dup( "(no short description)" );
    pMob->long_descr    =   str_dup( "(no long description)\r\n" );
    pMob->description   =   &str_empty[0];
    pMob->bio		=   &str_empty[0];
    pMob->vnum          =   0;
    pMob->count         =   0;
    pMob->killed        =   0;
    pMob->sex           =   0;
    pMob->level         =   0;
    pMob->act           =   ACT_IS_NPC;
    pMob->affected_by   =   0;
    pMob->alignment     =   0;
    pMob->hitroll	=   0;
    pMob->race          =   race_lookup( "human" );
    pMob->form          =   0;          
    pMob->parts         =   0;          
    pMob->imm_flags     =   0;     
    pMob->envimm_flags     =   0;     
    pMob->res_flags     =   0;           
    pMob->vuln_flags    =   0;         
    pMob->material      =   material_lookup( "" ); 
    pMob->off_flags     =   0;           
    pMob->size          =   SIZE_MEDIUM; 
    pMob->ac[AC_PIERCE]	=   0;           
    pMob->ac[AC_BASH]	=   0;           
    pMob->ac[AC_SLASH]	=   0;           
    pMob->ac[AC_EXOTIC]	=   0;          
    pMob->hit[DICE_NUMBER]	=   0;   
    pMob->hit[DICE_TYPE]	=   0;   
    pMob->hit[DICE_BONUS]	=   0;   
    pMob->mana[DICE_NUMBER]	=   0;   
    pMob->mana[DICE_TYPE]	=   0;   
    pMob->mana[DICE_BONUS]	=   0;   
    pMob->damage[DICE_NUMBER]	=   0;   
    pMob->damage[DICE_TYPE]	=   0;   
    pMob->damage[DICE_NUMBER]	=   0;   
    pMob->start_pos             =   POS_STANDING; 
    pMob->default_pos           =   POS_STANDING; 
    pMob->gold                  =   0;
    pMob->language                  =   0;
    pMob->atk_type = WDT_HIT;
    pMob->dam_type = attack_table[pMob->atk_type].damage;

    pMob->new_format            = TRUE;

    pMob->triggers = get_trs();

    pMob->nature = 0;

    pMob->monitors = NULL;
  
    pMob->time_wev = 0;

    pMob->fright = 0;

    pMob->group = 0;

    pMob->tracks = NULL;

    pMob->can_see = NULL;
    pMob->can_not_see = NULL;
  
    return pMob;
}


void free_mob_index( MOB_INDEX_DATA *pMob ) {
    free_string( pMob->player_name );
    pMob->player_name = NULL;

    free_string( pMob->short_descr );
    pMob->short_descr = NULL;

    free_string( pMob->long_descr );
    pMob->long_descr = NULL;

    free_string( pMob->description );
    pMob->description = NULL;

    if (pMob->pShop) {
        free_shop( pMob->pShop );
        pMob->pShop = NULL;
    }

    free_trs( pMob->triggers );
    pMob->triggers = NULL;

    if (pMob->monitors) {
        free_monitor_list(pMob->monitors);
        pMob->monitors = NULL;
    }

    if (pMob->skills) {
       freeSkillData(pMob->skills);
       pMob->skills = NULL;
    }

    if (pMob->tracks != NULL) {
      reduce_usage(pMob->tracks);
      pMob->tracks = NULL;
    }

    pMob->next              = mob_index_free;
    mob_index_free          = pMob;
    return;
}


HELP_DATA *new_help(void)
{
    HELP_DATA *NewHelp;

    if( help_free == NULL )
    {
        NewHelp = (HELP_DATA *) alloc_perm( sizeof(*NewHelp) );
        top_help++;
    }
    else
    {
        NewHelp         = help_free;
        help_free       = help_free->next;
    }

    NewHelp->next       = NULL;
    NewHelp->level      = 0;
    NewHelp->keyword    = &str_empty[0];
    NewHelp->text       = &str_empty[0];

    return NewHelp;
}


void free_help( HELP_DATA *pHelp )
{
    if (pHelp->keyword != NULL ) 
	    free_string( pHelp->keyword );
    if (pHelp->text != NULL )
            free_string( pHelp->text );

    pHelp->next = help_free;
    help_free   = pHelp;
    return;
}


CUSTOMER *new_customer(void ) {
CUSTOMER *cust;

    cust =(CUSTOMER *) alloc_perm( sizeof(*cust));
    cust->name = NULL;
    cust->rating = 0;

    return cust;
}


void free_customer(CUSTOMER *cust) {

      free_string(cust->name);
      cust->name 	= NULL;
      cust->rating 	= 0;
      cust->next 	= NULL;
      cust 		= NULL;

      return;
}


PASSPORT *new_pass_order(void) {
PASSPORT *pass;

     pass = (PASSPORT *) alloc_perm(sizeof(*pass));  
     pass->name 	= NULL;
     pass->next 	= passport_list;
     passport_list 	= pass;
     return pass;
}


void free_pass_order(PASSPORT *pass) {
PASSPORT *p1 = NULL;
PASSPORT *p2 = NULL;

     for (p1 = passport_list; p1; p1 = p1->next) {
          if (p1 == pass) break;
          p2 = p1;
     }     

     if (p1) {
         free_string(p1->name);
         p1->name = NULL;

         if (p2) {
            p2->next = p1->next;
            p1->next = NULL;
         } else {
             passport_list = p1->next;
             p1->next = NULL;
         }
     }
     return;
}

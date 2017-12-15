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
#include "olc.h"
#include "doors.h"
#include "gadgets.h"
#include "deeds.h"
#include "profile.h"
#include "text.h"

DECLARE_DO_FUN(do_goto);


void do_build( CHAR_DATA *ch, char *argument ) {
char arg[MAX_STRING_LENGTH];
char arg2[MAX_STRING_LENGTH];
char buf[MAX_STRING_LENGTH];
AREA_DATA *nArea = NULL;
MOB_INDEX_DATA *pMobIndex;
MOB_INDEX_DATA *nMobIndex;
ROOM_INDEX_DATA *pRoom = NULL;
ROOM_INDEX_DATA *nRoom;
ROOM_INDEX_DATA *tRoom;
OBJ_INDEX_DATA	*pObjIndex;
OBJ_INDEX_DATA	*nObjIndex;
OBJ_DATA	*obj;
RESET_DATA		*pReset;
RESET_DATA		*nReset;
EXTRA_DESCR_DATA *ed;
VNUM vff = 0;
VNUM vff2 =0;
int pattern =0;
int width = 0;
int startroom = 0;
long cost = 0;
bool clusterfree = FALSE;
bool sectorok = FALSE;
bool dobuild = TRUE;
int iHash, linkto;

    if ( argument[0] == '\0' )    {
	send_to_char( "Build what?\r\n", ch );
	send_to_char( "BUILD <building>\r\n", ch );
	send_to_char( "BUILD INFO <building>\r\n", ch );
	return;
    }

   if (IS_SET(ch->in_room->room_flags, ROOM_BUILDING)) {
	send_to_char( "Not in this room!\r\n", ch );
	return;
    }

    argument = one_argument(argument,arg);

    if (!strcmp(arg, "info")) {
          if ( argument[0] == '\0' )    {
              send_to_char("Allowed Buildings:\r\n",ch);
              for (vff = mud.pattern[0]; vff <= mud.pattern[1]; vff++) {
               	   pRoom = get_room_index(vff);
                   if (pRoom != NULL) {
                        if (IS_SET(pRoom->room_flags, ROOM_BUILDING)) {
                             sprintf(buf, "  * %s\r\n",capitalize(pRoom->name));
                             send_to_char(buf,ch);
                        }       
                   }
              }
              return;
          }
          argument = one_argument(argument,arg);
          dobuild = FALSE;
    }

    /* Looking for the appropriate template */
         for (vff = mud.pattern[0]; vff <= mud.pattern[1]; vff++) {
             	pRoom = get_room_index(vff);
                if (pRoom != NULL) {
                    if (!strcmp(pRoom->name, arg)
                    && IS_SET(pRoom->room_flags, ROOM_BUILDING))  break;
                }
         }
         pattern = vff;
         if (pattern >= mud.pattern[1]
         || pRoom == NULL) {
	send_to_char( "No appropriate building type found...\r\n", ch );
	return;
         }
         
         pRoom = get_room_index(pattern);
         ed = pRoom->extra_descr;
         while (ed != NULL) {
               if ( is_name("cost", ed->keyword)) {
                     one_argument(ed->description, arg2);
                     cost = atol(arg2);
               }
               ed = ed->next;
         }
         if (cost<1) {
	send_to_char( "Something wrong with building price - ask your Admin.\r\n", ch );
	return;
         }

         if (!dobuild) {
              sprintf(buf, "Building: %s\r\n",capitalize(pRoom->name));
              send_to_char(buf,ch);
              sprintf(buf, "Cost    : %ld\r\n",cost);
              send_to_char(buf,ch);
              send_to_char("Allowed Sectors:\r\n",ch);
              for (vff=0; vff < SECT_MAX; vff++) {
                     sectorok = FALSE;
                     switch (pRoom->sector_type) {
                           case SECT_CITY:
                           case SECT_ROAD:
                                 if (vff == SECT_CITY
                                 || vff == SECT_ROAD) sectorok = TRUE;
                                 break;
                           case SECT_FIELD:
                           case SECT_DESERT:
                           case SECT_PATH:
                           case SECT_PLAIN:
                                 if (vff == SECT_FIELD
                                 || vff == SECT_DESERT
                                 || vff == SECT_PATH
                                 || vff == SECT_PLAIN) sectorok = TRUE;
                                break;
                           case SECT_HILLS:
                           case SECT_MOUNTAIN:
                                if (vff == SECT_HILLS
                                || vff == SECT_MOUNTAIN) sectorok = TRUE;
                                break;
                     }
                     if (pRoom->sector_type == vff) sectorok = TRUE;
                     if (sectorok) {
                         sprintf(buf, "  * %s\r\n", flag_string(sector_name, vff)); 
                         send_to_char(buf,ch);
                     }
              }
              return;
         }

         sprintf(buf,"Building price: %ld\r\n", cost);
         send_to_char(buf,ch);

         if (ch->gold[0] < cost) {
	send_to_char( "You can't afford this building.\r\n", ch );
	return;
         }

         switch (pRoom->sector_type) {
             case SECT_CITY:
             case SECT_ROAD:
                  if (ch->in_room->sector_type == SECT_CITY
                  || ch->in_room->sector_type == SECT_ROAD) sectorok = TRUE;
                  break;
             case SECT_FIELD:
             case SECT_DESERT:
             case SECT_PATH:
             case SECT_PLAIN:
                  if (ch->in_room->sector_type == SECT_FIELD
                  || ch->in_room->sector_type == SECT_DESERT
                  || ch->in_room->sector_type == SECT_PATH
                  || ch->in_room->sector_type == SECT_PLAIN) sectorok = TRUE;
                  break;
             case SECT_HILLS:
             case SECT_MOUNTAIN:
                  if (ch->in_room->sector_type == SECT_HILLS
                  || ch->in_room->sector_type == SECT_MOUNTAIN) sectorok = TRUE;
                  break;
         }
         if (pRoom->sector_type == ch->in_room->sector_type) sectorok = TRUE;
 
         if (sectorok == FALSE) {
	send_to_char( "You can't build that here.\r\n", ch );
	return;
         }

         pattern += 1;   
         for (vff = pattern; vff <= mud.pattern[1]; vff++) {
             	pRoom = get_room_index(vff);
                if (pRoom == NULL) break;
                if (IS_SET(pRoom->room_flags, ROOM_BUILDING))  break;
         }
         
         width = vff - pattern - 1;
         
         if (width < 0) {
	send_to_char( "No correct width found - ask your Admin.\r\n", ch );
	return;
         }

         for (vff = mud.building[0]; vff <= mud.building[1] - width; vff++) {
               clusterfree = FALSE;
               pRoom = get_room_index(vff);
               if (pRoom == NULL) {
                   clusterfree = TRUE;
                   for (vff2 = vff; vff2 <= vff + width; vff2++) {
                         pRoom = get_room_index(vff2);
                         if (pRoom != NULL) {
                             if (IS_SET(pRoom->room_flags, ROOM_BUILDING)) clusterfree = FALSE;
                         }
                   }
               }
               if (clusterfree == TRUE) break;
         }
         
         if (clusterfree == FALSE) {
	send_to_char( "No free room cluster - ask your Admin.\r\n", ch );
	return;
         }

         startroom = vff;

             /* Create rooms */
             for (vff = startroom; vff <= startroom + width; vff++) {
             	   pRoom = get_room_index(pattern + vff - startroom);
                    nArea = get_vnum_area(vff);
                    if ( !nArea )    {      
	         send_to_char( "That vnum is not assigned an area.\r\n", ch );
	         return;
                    }

                    if ((nRoom= get_room_index(vff)) == NULL) {
                         nRoom			= new_room_index();
                         nRoom->area			= nArea;
                         nRoom->vnum			= vff;
                   
                         if ( vff  > top_vnum_room )  top_vnum_room = vff;
                         iHash			= vff % MAX_KEY_HASH;
                         nRoom->next			= room_index_hash[iHash];
                         room_index_hash[iHash]	= nRoom;
                         ch->desc->pEdit		= (void *)nRoom;
                    }
                    free_string(nRoom->name);
                    nRoom->name = str_dup(pRoom->name);
                    nRoom->affected = pRoom->affected;
                    nRoom->extra_descr = pRoom->extra_descr;
                    free_string(nRoom->description);
                    nRoom->description = str_dup(pRoom->description);
                    free_string(nRoom->image);
                    nRoom->image = str_dup(pRoom->image);
                    nRoom->room_flags = pRoom->room_flags;
                    nRoom->room_rent = pRoom->room_rent;
                    if (pRoom->rented_by) nRoom->rented_by = str_dup(ch->name);
                    SET_BIT( nRoom->room_flags, ROOM_BUILDING );
                    nRoom->light = pRoom->light;
                    nRoom->sector_type = pRoom->sector_type;
                    if (pRoom->day >0) nRoom->day = pRoom->day -pattern + startroom;
                    if (pRoom->night >0) nRoom->night = pRoom->night - pattern + startroom;
                    nRoom->affected_by = pRoom->affected_by;

                    send_to_char( "Room built.\r\n", ch );
             }
             /* Duplicate exits */
             for (vff = startroom; vff <= startroom + width; vff++) {
                    nRoom = get_room_index(vff);
              	    pRoom = get_room_index(pattern + vff - startroom);

                    for (vff2 = 0; vff2 <= DIR_MAX-1; vff2++) {
                           if (pRoom->exit[vff2] != NULL) {
      	                 nRoom->exit[vff2] = new_exit();
                                 tRoom = get_room_index(pRoom->exit[vff2]->u1.to_room->vnum - pattern + startroom);
		 nRoom->exit[vff2]->u1.to_room = tRoom;
                                 nRoom->exit[vff2]->exit_info = pRoom->exit[vff2]->exit_info;
                                 nRoom->exit[vff2]->key = pRoom->exit[vff2]->key;
                                 nRoom->exit[vff2]->keyword = str_dup(pRoom->exit[vff2]->keyword);
                                 nRoom->exit[vff2]->description = str_dup(pRoom->exit[vff2]->description);
                                 nRoom->exit[vff2]->rs_flags = pRoom->exit[vff2]->rs_flags;
                           }
                    }
                    send_to_char( "Room linked.\r\n", ch );
             }
             /* Duplicate obj templates */
             for (vff = startroom; vff <= startroom + width; vff++) {
                   if ( (pObjIndex = get_obj_index(pattern + vff - startroom)))    {
                        if ((nObjIndex= get_obj_index(vff)) == NULL) {
                              nObjIndex = new_obj_index();
                              nObjIndex->vnum = vff;
                              nObjIndex->area = nArea;
                              nObjIndex->repop = 100;
                              iHash = vff % MAX_KEY_HASH;
                              nObjIndex->next = obj_index_hash[iHash];
                              obj_index_hash[iHash] = nObjIndex;
                              ch->desc->pEdit = (void *)nObjIndex;
                       }
                       free_string(nObjIndex->name);
                       nObjIndex->name = str_dup(pObjIndex->name);
                       free_string(nObjIndex->short_descr);
                       nObjIndex->short_descr = str_dup(pObjIndex->short_descr);
                       free_string(nObjIndex->description);
                       nObjIndex->description = str_dup(pObjIndex->description);
                       free_string(nObjIndex->image);
                       nObjIndex->image = str_dup(pObjIndex->image);
                       nObjIndex->material = pObjIndex->material;
                       nObjIndex->item_type = pObjIndex->item_type;
                       nObjIndex->extra_flags = pObjIndex->extra_flags;
                       nObjIndex->wear_flags = pObjIndex->wear_flags;
                       nObjIndex->level = pObjIndex->level;
                       nObjIndex->condition = pObjIndex->condition;
                       nObjIndex->weight = pObjIndex->weight;
                       nObjIndex->size = pObjIndex->size;
                       nObjIndex->cost = pObjIndex->cost;
                       for (vff2 = 0; vff2 <= 5; vff2++) {
                             nObjIndex->value[vff2] = pObjIndex->value[vff2];
                       }
                       if (nObjIndex->item_type == ITEM_PORTAL) {
                              nObjIndex->value[0] = nObjIndex->value[0] -pattern + startroom;
                              if (nObjIndex->value[1] > 0) nObjIndex->value[1] = nObjIndex->value[1] -pattern + startroom;
                              if (nObjIndex->value[2] > 0) nObjIndex->value[2] = nObjIndex->value[2] -pattern + startroom;
                       }
                   }
                   send_to_char( "Object Created.\r\n", ch );
             }

             /* Duplicate mob templates */
             for (vff = startroom; vff <= startroom + width; vff++) {
                   if ( (pMobIndex = get_mob_index(pattern + vff - startroom)))    {
                        if ((nMobIndex= get_mob_index(vff)) == NULL) nMobIndex = new_mob_index();
                        nMobIndex->spec_fun = pMobIndex->spec_fun;
                        nMobIndex->new_format = pMobIndex->new_format;
                        free_string(nMobIndex->player_name);
                        nMobIndex->player_name = str_dup(pMobIndex->player_name);
                        free_string(nMobIndex->short_descr);
                        nMobIndex->short_descr = str_dup(pMobIndex->short_descr);
                        free_string(nMobIndex->long_descr);
                        nMobIndex->long_descr = str_dup(pMobIndex->long_descr);
                        free_string(nMobIndex->description);
                        nMobIndex->description = str_dup(pMobIndex->description);
                        free_string(nMobIndex->bio);
                        nMobIndex->bio = str_dup(pMobIndex->bio);
                        free_string(nMobIndex->image);
                        nMobIndex->image = str_dup(pMobIndex->image);
                        nMobIndex->act = pMobIndex->act;
                        nMobIndex->affected_by = pMobIndex->affected_by;
                        nMobIndex->alignment = pMobIndex->alignment;
                        nMobIndex->level = pMobIndex->level;
                        nMobIndex->hitroll = pMobIndex->hitroll;
                        nMobIndex->dam_type = pMobIndex->dam_type;
                        nMobIndex->off_flags = pMobIndex->off_flags;
                        nMobIndex->imm_flags = pMobIndex->imm_flags;
                        nMobIndex->res_flags = pMobIndex->res_flags;
                        nMobIndex->vuln_flags = pMobIndex->vuln_flags;
                        nMobIndex->start_pos = pMobIndex->start_pos;
                        nMobIndex->default_pos = pMobIndex->default_pos;
                        nMobIndex->sex = pMobIndex->sex;
                        nMobIndex->race = pMobIndex->race;
                        nMobIndex->gold = pMobIndex->gold;
                        nMobIndex->form = pMobIndex->form;
                        nMobIndex->parts = pMobIndex->parts;
                        nMobIndex->size = pMobIndex->size;
                        nMobIndex->material = pMobIndex->material;
                        nMobIndex->nature = pMobIndex->nature;
                        nMobIndex->fright = pMobIndex->fright;
                        for (vff2=0; vff2<=3; vff2++) {
                              nMobIndex->hit[vff2] = pMobIndex->hit[vff2];
                              nMobIndex->mana[vff2] = pMobIndex->mana[vff2];
                              nMobIndex->damage[vff2] = pMobIndex->damage[vff2];
                        }
                        for (vff2=0; vff2<=4; vff2++) {
                              nMobIndex->ac[vff2] = pMobIndex->ac[vff2];
                        }
                        nMobIndex->area = nArea;
                        nMobIndex->vnum = vff;
                        if (pMobIndex->pShop !=NULL) {
     	             nMobIndex->pShop = new_shop();
     	             nMobIndex->pShop->keeper = nMobIndex->vnum;
                             for(vff2=0; vff2<MAX_TRADE; vff2++) {
                                   nMobIndex->pShop->buy_type[vff2] = pMobIndex->pShop->buy_type[vff2];
                             }
                        }

                        if (!str_cmp("spec_homeguard", spec_string(pMobIndex->spec_fun))
                        || !str_cmp("spec_playershop", spec_string(pMobIndex->spec_fun))
                        || !str_cmp("spec_miner", spec_string(pMobIndex->spec_fun))) {
                               sprintf(buf, "%s _plr: %s", pMobIndex->player_name, ch->name);
                               free_string(nMobIndex->player_name);
                               nMobIndex->player_name = strdup(buf);
                       }
                        iHash = vff % MAX_KEY_HASH;
                        nMobIndex->next	= mob_index_hash[iHash];
                        mob_index_hash[iHash]	= nMobIndex;
                        ch->desc->pEdit		= (void *)nMobIndex;

                   }
                   send_to_char( "Mob Created.\r\n", ch );
             }

             /* Create entrance portal, OUT exit */
             if ( !(nObjIndex = get_obj_index(startroom)))    {
                   send_to_char( "No Entrance - ask your Admin.\r\n", ch );
                   return;
             }
       
              nReset = new_reset_data();
              nReset->command	= 'O';
              nReset->arg1	= nObjIndex->vnum;
              nReset->arg2	= 1;
              nReset->arg3	= ch->in_room->vnum;
              nReset->count	= 1;
              add_reset( ch->in_room, nReset, 0);
              obj = create_object( nObjIndex, number_fuzzy(nObjIndex->level ) );
              obj->reset = nReset;
              obj_to_room(obj, ch->in_room);             
              linkto=obj->value[0];

               if (obj->value[2] > 0) {
                     if ( !(nObjIndex = get_obj_index(obj->value[2])))    {
                           send_to_char( "No Key - ask your Admin.\r\n", ch );
                           return;
                     }
                      nReset = new_reset_data();
                      nReset->command	= 'O';
                      nReset->arg1	= obj->value[2];
                      nReset->arg2	= 1;
                      nReset->arg3	= startroom;
                      nReset->count	= 1;
                      add_reset(get_room_index(startroom), nReset, 0);
                      obj = create_object( nObjIndex, number_fuzzy(nObjIndex->level ) );
                      obj->reset = nReset;
                      obj_to_room(obj, get_room_index(startroom));             
                      obj = create_object( nObjIndex, number_fuzzy(nObjIndex->level ) );
                      obj_to_room(obj, ch->in_room);             
              }

               nRoom = get_room_index(linkto);
               nRoom->exit[DIR_OUT] = new_exit();
               nRoom->exit[DIR_OUT]->u1.to_room = ch->in_room;
 
               send_to_char( "Entrance Created.\r\n", ch );

             for (vff = startroom; vff <= startroom + width; vff++) {
                    nRoom = get_room_index(vff);
              	    pRoom = get_room_index(pattern + vff - startroom);
                    for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )    {
                           if (pReset != NULL) {
                                nReset = new_reset_data();
                                nReset->command = pReset->command ;
                                nReset->arg1 = pReset->arg1 - pattern + startroom;
                                nReset->arg2 = pReset->arg2;
                                nReset->arg3 = pReset->arg3 - pattern + startroom;
                                nReset->count = 0;
                                add_reset( nRoom, nReset, 0);
      	           }
                    }
             }


            if (!strcmp(arg, "home")) {

            } else if (!strcmp(arg, "shop")) {
                  vff = number_range(66,64000);
                   nObjIndex = get_obj_index(vff);
                   while (nObjIndex == NULL
                    || nObjIndex->cost == 0
                    || nObjIndex->level <1
                    || nObjIndex->level >61
                    || nObjIndex->zmask !=0
                    || nObjIndex->item_type == ITEM_PORTAL
                    || nObjIndex->item_type == ITEM_MONEY
                    || nObjIndex->item_type == ITEM_CORPSE_NPC
                    || nObjIndex->item_type == ITEM_KEY
                    || nObjIndex->item_type == ITEM_LOCKER_KEY
                    || !IS_SET(nObjIndex->wear_flags, ITEM_TAKE)) {
                        vff = number_range(66,64000);
                        nObjIndex = get_obj_index(vff);
                   }
                   nReset = new_reset_data();
                   nReset->command = 'G';
                   nReset->arg1 = nObjIndex->vnum;
                   nReset->arg2 = 1;
                   nReset->arg3 = WEAR_NONE;
                   nReset->count = 0;
                   add_reset(get_room_index(startroom), nReset, 0);
            }                                      


             SET_BIT( nArea->area_flags, AREA_CHANGED );
             SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
             ch->gold[0] -=cost;
             return;
     
}


void create_vehicle(CHAR_DATA *ch, OBJ_DATA *obj) {
    ROOM_INDEX_DATA *pRoom = NULL;
    ROOM_INDEX_DATA *nRoom;
    AREA_DATA *nArea = NULL;
    OBJ_INDEX_DATA	*pObjIndex;
    OBJ_INDEX_DATA	*nObjIndex;
    char buf[MAX_STRING_LENGTH];
    int vff, iHash;
    bool clusterfree = FALSE;

         for (vff = mud.building[0]; vff <= mud.building[1] - 1; vff++) {
               clusterfree = TRUE;
               pRoom = get_room_index(vff);
               if (pRoom != NULL) {
                   if (IS_SET(pRoom->room_flags, ROOM_BUILDING)) clusterfree = FALSE;
               }
               if (clusterfree == TRUE) break;
         }
         
         if (clusterfree == FALSE) {
	send_to_char( "No free room cluster - ask your Admin.\r\n", ch );
	return;
         }

         pRoom = get_room_index(obj->pIndexData->vnum);
         nArea = get_vnum_area(vff);
         if ( !nArea )    {      
              send_to_char( "That vnum is not assigned an area.\r\n", ch );
              return;
         }

          if ((nRoom= get_room_index(vff)) == NULL) {
                nRoom			= new_room_index();
                nRoom->area			= nArea;
                nRoom->vnum			= vff;
                   
                if ( vff  > top_vnum_room )  top_vnum_room = vff;
                iHash			= vff % MAX_KEY_HASH;
                nRoom->next			= room_index_hash[iHash];
                room_index_hash[iHash]	= nRoom;
                ch->desc->pEdit		= (void *)nRoom;
          }

         free_string(nRoom->name);
         nRoom->name = str_dup(pRoom->name);
         nRoom->affected = pRoom->affected;
         nRoom->extra_descr = pRoom->extra_descr;
         free_string(nRoom->description);
         nRoom->description = str_dup(pRoom->description);
         free_string(nRoom->image);
         nRoom->image = str_dup(pRoom->image);
         nRoom->room_flags = pRoom->room_flags;
         SET_BIT( nRoom->room_flags, ROOM_BUILDING );
         SET_BIT( nRoom->room_flags, ROOM_VEHICLE );
         nRoom->light = pRoom->light;
         nRoom->sector_type = pRoom->sector_type;
         nRoom->day = 0;
         nRoom->night = 0;
         nRoom->affected_by = pRoom->affected_by;
         send_to_char( "Room built.\r\n", ch );

         pObjIndex = obj->pIndexData;
         if ((nObjIndex= get_obj_index(vff)) == NULL) {
               nObjIndex = new_obj_index();
               nObjIndex->vnum = vff;
               nObjIndex->area = nArea;
               nObjIndex->repop = 100;
               iHash = vff % MAX_KEY_HASH;
               nObjIndex->next = obj_index_hash[iHash];
               obj_index_hash[iHash] = nObjIndex;
               ch->desc->pEdit = (void *)nObjIndex;
         }   
         sprintf(buf, "%s _plr: %s", pObjIndex->name, ch->name);
         free_string(nObjIndex->name);
         nObjIndex->name = str_dup(buf);
         free_string(nObjIndex->short_descr);
         nObjIndex->short_descr = str_dup(pObjIndex->short_descr);
         free_string(nObjIndex->description);
         nObjIndex->description = str_dup(pObjIndex->description);
         free_string(nObjIndex->image);
         nObjIndex->image = str_dup(pObjIndex->image);
         nObjIndex->material = pObjIndex->material;
         nObjIndex->item_type = pObjIndex->item_type;
         nObjIndex->extra_flags = pObjIndex->extra_flags;
         nObjIndex->wear_flags = pObjIndex->wear_flags;
         nObjIndex->level = pObjIndex->level;
         nObjIndex->condition = pObjIndex->condition;
         nObjIndex->weight = pObjIndex->weight;
         nObjIndex->size = pObjIndex->size;
         nObjIndex->cost = pObjIndex->cost;
         nObjIndex->value[0] = vff;
         nObjIndex->value[1] = 0;
         nObjIndex->value[2] = 0;
         nObjIndex->value[3] = 0;
         SET_BIT(nObjIndex->value[3], CONT_CLOSEABLE);
         SET_BIT(nObjIndex->value[3], CONT_CLOSED);
         SET_BIT(nObjIndex->value[3], CONT_LOCKED);
         nObjIndex->value[4] = PORTAL_VEHICLE;
         send_to_char( "Object Created.\r\n", ch );

          if ( !(nObjIndex = get_obj_index(vff)))    {
                   send_to_char( "No Entrance - ask your Admin.\r\n", ch );
                   return;
          }

          nRoom = get_room_index(vff);
          nRoom->exit[DIR_OUT] = new_exit();
          nRoom->exit[DIR_OUT]->u1.to_room = ch->in_room;

          obj->value[0] = vff; 
          obj->pIndexData = nObjIndex;
          obj->pIndexData->value[0] = vff;
          SET_BIT(obj->value[3], CONT_CLOSEABLE);
          SET_BIT(obj->value[3], CONT_CLOSED);
          SET_BIT(obj->value[3], CONT_LOCKED);
          sprintf(buf, "%s _plr: %s", pObjIndex->name, ch->name);
          free_string(obj->name);
          obj->name = strdup(buf);
          send_to_char( "Entrance Created.\r\n", ch );

          SET_BIT( nRoom->area->area_flags, AREA_CHANGED );
          SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
          
          return;
}


void do_home(CHAR_DATA *ch, char *argument)	{
ROOM_INDEX_DATA *pRoom;
char arg[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
VNUM room;

                if (IS_NPC(ch)) return;

	if (argument == NULL || argument[0] == '\0')		{
                        pRoom = get_room_index(ch->pcdata->home_room);
                        if (!pRoom) {
	            send_to_char ("You have no home room.\r\n", ch);
         	            send_to_char ("SYNTAX: HOME\r\n", ch);
	            if (IS_IMMORTAL(ch)) send_to_char ("        HOME set <vnum>\r\n", ch);
                            else send_to_char ("        HOME set\r\n", ch);
                            send_to_char ("        HOME desc\r\n", ch);
	            return;
	        }

                        if (!IS_IMMORTAL(ch)) {
                                if (str_cmp(pRoom->rented_by, ch->name)) {
	   	    send_to_char ("You have no home room.\r\n", ch);
                                    send_to_char ("SYNTAX: HOME\r\n", ch);
                                    send_to_char ("        HOME set\r\n", ch);
                                    send_to_char ("        HOME desc\r\n", ch);
		    return;
	                }
                        }

    	        sprintf(arg, "%ld", ch->pcdata->home_room);	
	        do_goto(ch, arg);
	        return;
	}

	argument = one_argument(argument, arg);
	argument = one_argument(argument, arg2);
   
                 if  (!str_cmp(arg, "set")) {
                      if (IS_IMMORTAL(ch)
                      && arg2[0] != '\0'
                      && is_number(arg2)) {
                            room = atol(arg2);
                      } else {
                            room = ch->in_room->vnum;
                      }

                      pRoom = get_room_index(room);
	      if (!pRoom) {
		send_to_char ("That room does not exist.\r\n", ch);
		return;
	      }
	
                      if (!IS_IMMORTAL(ch)) {
                           if (!ch->in_room->rented_by 
                           || (str_cmp(ch->in_room->rented_by, ch->name) && str_cmp(ch->in_room->rented_by, ch->pcdata->spouse))) {
	   	send_to_char ("You don't own this room.\r\n", ch);
		return;
                           }
                      }

	      ch->pcdata->home_room = room;	
	      send_to_char ("Home room set.\r\n", ch);

                 } else if  (!str_cmp(arg, "desc")) {
                      if (IS_SET(ch->in_room->room_flags, ROOM_RENT)
                      || !ch->in_room->rented_by 
                      || (str_cmp(ch->in_room->rented_by, ch->name) && str_cmp(ch->in_room->rented_by, ch->pcdata->spouse))) {
	           send_to_char ("You don't own this room.\r\n", ch);
	           return;
                      }

      	      string_append( ch, &ch->in_room->description );
                      SET_BIT(ch->in_room->area->area_flags, AREA_CHANGED);

                 } else {
	      send_to_char ("SYNTAX: HOME\r\n", ch);
	      if (IS_IMMORTAL(ch)) send_to_char ("        HOME set <vnum>\r\n", ch);
                      else send_to_char ("        HOME set\r\n", ch);
                      send_to_char ("        HOME desc\r\n", ch);
                 }
                 return;
}

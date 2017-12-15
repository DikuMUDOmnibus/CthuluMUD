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
#include "fight.h"
#include "skill.h"
#include "affect.h"
#include "exp.h"
#include "doors.h"
#include "mob.h"
#include "wev.h"
#include "tracks.h"
#include "olc.h"
#include "gsn.h"
#include "econd.h"


/* command procedures needed */
DECLARE_DO_FUN(do_flee);
DECLARE_DO_FUN(do_get);
DECLARE_DO_FUN(do_recall);
DECLARE_DO_FUN(do_yell);
DECLARE_DO_FUN(do_sacrifice);
DECLARE_DO_FUN(do_look);


/*
 * Local functions.
 */

char 	*quality_by_skill	(int skill);
void        kill_msg          	(CHAR_DATA *ch);  


void do_forge (CHAR_DATA *ch, char *argument)	{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
     OBJ_DATA *obj;
     OBJ_DATA *rawm;
    AFFECT_DATA *rAf;
    AFFECT_DATA *pAf;
     int roll;
     int skill;

     argument = one_argument(argument, arg1);
     argument = one_argument(argument, arg2);

    if (IS_AFFECTED(ch, AFF_MIST)) {
	send_to_char( "You can't do that in mist form?\r\n", ch );
	return;
    }

 obj = ch->carrying;

    while ( obj != NULL
         && ( obj->item_type != ITEM_FORGING
           || obj->wear_loc != -1
           || !can_see_obj(ch, obj) ) ) {
      obj = obj->next_content;
    }

    if ( obj == NULL ) {
      obj = ch->in_room->contents;

      while ( obj != NULL
           && ( obj->item_type != ITEM_FORGING
             || !can_see_obj(ch, obj) ) ) {
        obj = obj->next_content;
      }
    }

    if ( obj == NULL ) {
       send_to_char("You have no suitable forging object!\r\n", ch);
       return;
    }

    skill = get_skill(ch, gsn_forging);

     if (skill < 1 ) {
        send_to_char ("You have no clue how to forge a weapon.\r\n",ch);
        return;
     } 	

     if ( ( rawm = get_eq_char( ch, WEAR_HOLD ) ) == NULL ) {
        send_to_char ("You have to hold some raw material to forge a weapon.\r\n",ch);
        return;
     }  

     if (rawm->item_type!=ITEM_RAW) {
        send_to_char ("You cannot forge a weapon out of this.\r\n",ch);
        return;
     }  

     if (ch->move<200) {
        send_to_char ("You are too exhausted to forge a weapon.\r\n",ch);
        return;
     }  
     
    ch->move -=200;

    WAIT_STATE(ch, skill_array[gsn_forging].beats);

    obj = create_object ( get_obj_index (46), 0);
    obj->enchanted = TRUE;

     if ( arg2[0] == '\0' ) { 
          obj->level = ch->level;
     } else {
           if (is_number(arg2)) {
               obj->level=atoi(arg2);
           } else {
               obj->level = ch->level;
           }
     }
    obj->level = UMAX(1, obj->level);
    obj->level = UMIN(obj->level, ch->level);

    obj->cost = (200 + obj->level* 2) * (obj->level + 1);
    obj->weight = obj->level/5;
    obj->value[1] = UMIN(obj->level/4+1, 8);
    obj->value[2] = ((obj->level+5)/(obj->value[1]+2))+1;
    obj->material=rawm->material;
    for (rAf = rawm->pIndexData->affected; rAf != NULL; rAf = rAf->next) {
           pAf = new_affect();
           pAf->location   =   rAf->location;
           pAf->skill	       =   rAf->skill;
           pAf->modifier   =   rAf->modifier;
           pAf->type	       =   rAf->type;
           pAf->afn           =   rAf->afn;
           pAf->duration  =   rAf->duration;
           pAf->bitvector =  rAf->bitvector;
           pAf->next          =   obj->affected;
           obj->affected    =   pAf;
    }

    free_string(obj->name);
    free_string(obj->short_descr);
    free_string(obj->description);

     switch (UPPER(arg1[0])) {
          case 'S':
              if (!str_cmp(arg1, "staff")){
                   obj->value[0]=WEAPON_STAFF;
                   sprintf(buf, "%s %s staff", quality_by_skill(skill), flag_string( material_string, obj->material ));
                   obj->name = str_dup("forged staff");
                   obj->short_descr = str_dup(buf);
                   obj->description = str_dup("A forged staff lies here.\n");
                   obj->value[3]=WDT_THWACK;

              } else if (!str_cmp(arg1, "spear")){
                   obj->value[0]=WEAPON_SPEAR;
                   sprintf(buf, "%s %s spear",quality_by_skill(skill), flag_string( material_string, obj->material ));
                   obj->name = str_dup("forged spear");
                   obj->short_descr = str_dup(buf);
                   obj->description = str_dup("A forged spear lies here.\n");
                   obj->value[3]=WDT_PIERCE;

              } else if (!str_cmp(arg1, "shield")){
                   obj->item_type = ITEM_ARMOR;
                   REMOVE_BIT(obj->wear_flags, ITEM_WIELD);
                   SET_BIT(obj->wear_flags, ITEM_WEAR_SHIELD);
                   obj->cost = (150 + obj->level* 2) * (obj->level + 1);
                   obj->value[0] = obj->level / 13 + 1;
                   obj->value[1] = obj->value[0] * 8 / 10;
                   obj->value[2] = obj->value[0] * 8 / 10;
                   obj->value[3] = obj->value[0] / 2;
                   sprintf(buf, "%s %s shield",quality_by_skill(skill), flag_string( material_string, obj->material ));
                   obj->name = str_dup("forged shield");
                   obj->short_descr = str_dup(buf);
                   obj->description = str_dup("A forged shield lies here.\n");

               } else {
                   obj->value[0]=WEAPON_SWORD;
                   sprintf(buf, "%s %s sword",quality_by_skill(skill), flag_string( material_string, obj->material ));
                   obj->name = str_dup("forged sword");
                   obj->short_descr =str_dup(buf);
                   obj->description = str_dup("A forged sword lies here.\n");
                   obj->value[3]=WDT_SLASH;
               }
               break;
          case 'D':
               obj->value[0]=WEAPON_DAGGER;
               obj->weight /= 2;
                rawm->value[0] -=1;
                sprintf(buf, "%s %s dagger",quality_by_skill(skill), flag_string( material_string, obj->material ));
                obj->name = str_dup("forged dagger");
                obj->short_descr = str_dup(buf);
                obj->description = str_dup("A forged dagger lies here.\n");
                 obj->value[3]=WDT_STAB;
                break;
          case 'A':
             if (!str_cmp(arg1, "axe")) {
                  obj->value[0]=WEAPON_AXE;
                  obj->weight = (obj->weight * 125)/100;
                  sprintf(buf, "%s %s axe",quality_by_skill(skill), flag_string( material_string, obj->material ));
                  obj->name = str_dup("forged axe");
                  obj->short_descr =str_dup(buf);
                  obj->description = str_dup("A forged axe lies here.\n");
                  obj->value[3]=WDT_CLEAVE;

              } else if (!str_cmp(arg1, "armor")){
                  obj->item_type = ITEM_ARMOR;
                   REMOVE_BIT(obj->wear_flags, ITEM_WIELD);
                   SET_BIT(obj->wear_flags, ITEM_WEAR_BODY);
                  obj->cost = (150 + obj->level* 2) * (obj->level + 1);
                  obj->value[0] = obj->level / 11 + 2;
                  obj->value[1] = obj->value[0];
                  obj->value[2] = obj->value[0];
                  obj->value[3] = obj->value[0] / 2;
                  sprintf(buf, "%s %s armor",quality_by_skill(skill), flag_string( material_string, obj->material ));
                  obj->name = str_dup("forged armor");
                  obj->short_descr = str_dup(buf);
                  obj->description = str_dup("A forged armor lies here.\n");
               }
                break;
          case 'H':
                obj->item_type = ITEM_ARMOR;
                 REMOVE_BIT(obj->wear_flags, ITEM_WIELD);
                 SET_BIT(obj->wear_flags, ITEM_WEAR_HEAD);
                obj->cost = (150 + obj->level* 2) * (obj->level + 1);
                obj->value[0] = obj->level / 15 + 3;
                obj->value[1] = obj->value[0] * 12 / 10;
                obj->value[2] = obj->value[0] * 8 / 10;
                obj->value[3] = obj->value[0] / 2;
                sprintf(buf, "%s %s helmet",quality_by_skill(skill), flag_string( material_string, obj->material ));
                obj->name = str_dup("forged helmet");
                obj->short_descr = str_dup(buf);
                obj->description = str_dup("A forged helmet lies here.\n");
                break;
          case 'W':
                obj->value[0]=WEAPON_WHIP;
                obj->weight = (obj->weight * 3)/10;
                 rawm->value[0] -=2;
                sprintf(buf, "%s %s whip",quality_by_skill(skill), flag_string( material_string, obj->material ));
                obj->name = str_dup("whip");
                obj->short_descr =str_dup(buf);
                obj->description = str_dup("A self made whip lies here.\n");
                 obj->value[3]=WDT_WHIP;
                 break;
          case 'P':
                 obj->value[0]=WEAPON_POLEARM;
                 obj->weight = (obj->weight * 11)/10;
                 sprintf(buf, "%s %s polearm",quality_by_skill(skill), flag_string( material_string, obj->material ));
                 obj->name = str_dup("forged polearm");
                 obj->short_descr =str_dup(buf);
                 obj->description = str_dup("A forged polearm lies here.\n");
                 obj->value[3]=WDT_CLEAVE;
                 break;
          case 'B':
                 obj->value[0]=WEAPON_BOW;
                 sprintf(buf, "%s %s bow",quality_by_skill(skill), flag_string( material_string, obj->material ));
                 obj->name = str_dup("bow");
                 obj->short_descr = str_dup(buf);
                 obj->description = str_dup("A self made bow lies here.\n");
                 obj->weight = (obj->weight * 6)/10;
                 obj->value[3]=WDT_SHOT;
                 SET_BIT(obj->value[4],WEAPON_ARROW);
                 break;
          case 'M':     
                 obj->value[0]=WEAPON_MACE;
                 obj->weight = (obj->weight * 15)/10;
                 rawm->value[0] +=1;
                 sprintf(buf, "%s %s mace",quality_by_skill(skill), flag_string( material_string, obj->material ));
                 obj->name = str_dup("forged mace");
                 obj->short_descr =str_dup(buf);
                 obj->description = str_dup("A forged mace lies here.\n");
                 obj->value[3]=WDT_CRUSH;
                 break;
          case 'F':
                 obj->value[0]=WEAPON_FLAIL;
                 obj->weight = (obj->weight * 15)/10;
                 rawm->value[0] +=1;
                 sprintf(buf, "%s %s flail",quality_by_skill(skill), flag_string( material_string, obj->material ));
                 obj->name = str_dup("forged flail");
                 obj->short_descr =str_dup(buf);
                 obj->description = str_dup("A forged flail lies here.\n");
                 obj->value[3]=WDT_CRUSH;
                 break;
          default:
             send_to_char ("Dagger / Sword / Whip / Mace / Flail / Axe / Spear / Polearm /  Bow\r\n",ch);
             send_to_char ("Armor / Shield / Helmet\r\n",ch);
             extract_obj(obj);	
             return;
      }
       
    roll = number_percent();

    if (obj->item_type == ITEM_WEAPON) {
        if (obj->level>120) {
           obj->value[2] +=rawm->value[0];
           obj->weight +=rawm->value[1];
           obj->cost *=2;
        }
        if (obj->level>100) {
           obj->value[2] +=rawm->value[0];
           obj->weight +=rawm->value[1];
           obj->cost *=2;
        }
        if (obj->level>50) {
           obj->value[2] +=rawm->value[0];
           obj->weight +=rawm->value[1];
           obj->cost *=2;
        }
        obj->value[2] +=rawm->value[0];
        obj->weight +=rawm->value[1];

        if (roll+80<skill) {
            obj->value[1] +=1;
            obj->cost *=2;
        }
         if (roll+70<skill) {
            obj->value[1] +=1;
            obj->cost = (obj->cost * 15)/10;
        }
        if (roll+50<skill) {
            obj->value[1] +=1;
            obj->cost = (obj->cost * 125)/100;
        }
         if (roll+30<skill) {
            obj->value[1] +=1;
            obj->cost = (obj->cost * 112)/100;
        }
        if (roll+10<skill) {
            obj->value[1] +=1;
        }

        if (obj->weight<1) {
            obj->weight=1;
        }
        if (obj->value[1]<1) {
            obj->value[1]=1;
        }
        if (obj->value[2]<1) {
            obj->value[2]=1;
         }

    } else {

        if (obj->level>120) {
           obj->value[0] +=rawm->value[0]*2;
           obj->value[1] +=rawm->value[0]*3/2;
           obj->value[2] +=rawm->value[0]*2;
           obj->value[3] +=rawm->value[0];
           obj->weight +=rawm->value[1];
           obj->cost *=2;
        }
        if (obj->level>100) {
           obj->value[0] +=rawm->value[0]*3/2;
           obj->value[1] +=rawm->value[0]*3/2;
           obj->value[2] +=rawm->value[0]*3/2;
           obj->value[3] +=rawm->value[0];
           obj->weight +=rawm->value[1];
           obj->cost *=2;
        }
        if (obj->level>50) {
           obj->value[0] +=rawm->value[0];
           obj->value[1] +=rawm->value[0];
           obj->value[2] +=rawm->value[0];
           obj->value[3] +=rawm->value[0];
           obj->weight +=rawm->value[1];
           obj->cost *=2;
        }

           obj->value[0] +=rawm->value[0]/2;
           obj->value[1] +=rawm->value[0]/2;
           obj->value[2] +=rawm->value[0]/2;
           obj->value[3] +=rawm->value[0]/2;
           obj->weight +=rawm->value[1];

        if (roll+80<skill) {
            obj->value[0] +=3;
            obj->value[1] +=3;
            obj->value[2] +=2;
            obj->value[3] +=1;
            obj->cost *=2;
        }
         if (roll+70<skill) {
            obj->value[0] +=2;
            obj->value[1] +=2;
            obj->value[2] +=2;
            obj->value[3] +=1;
            obj->cost = (obj->cost * 15)/10;
        }
        if (roll+50<skill) {
            obj->value[0] +=1;
            obj->value[1] +=1;
            obj->value[2] +=1;
            obj->cost = (obj->cost * 125)/100;
        }
         if (roll+30<skill) {
            obj->value[0] +=1;
            obj->value[1] +=1;
            obj->cost = (obj->cost * 112)/100;
        }
        if (roll+10<skill) {
            obj->value[0] +=1;
        }

        obj->value[0] = obj->value[0] * (17 + number_range(0,6)) / 20;
        obj->value[1] = obj->value[1] * (17 + number_range(0,6)) / 20;
        obj->value[2] = obj->value[2] * (17 + number_range(0,6)) / 20;
        obj->value[3] = obj->value[3] * (17 + number_range(0,6)) / 20;
    }

    if (obj->weight<1) obj->weight=1;
   
    obj_from_char(rawm);
    extract_obj(rawm);	
    roll = number_percent();
    if (skill<roll+15) {
        sprintf_to_char (ch, "You fail to forge %s.\r\n",obj->short_descr);
        act("$n tries to forge $p.",ch,obj,NULL,TO_ROOM);
        check_improve(ch, gsn_forging, FALSE, 1);
        extract_obj(obj);	
        return;
    }
    obj_to_room( obj, ch->in_room );
    sprintf_to_char (ch, "You forge %s\r\n", obj->short_descr);
    act("$n tries to forge $p.",ch,obj,NULL,TO_ROOM);
    check_improve(ch, gsn_forging, TRUE, 1);
    return;
}


void do_skin(CHAR_DATA *ch, char *argument)	{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    char *trans;
     OBJ_DATA *obj;
     OBJ_DATA *corpse;
    AFFECT_DATA *rAf;
    AFFECT_DATA *pAf;
    int roll;
    int skill;

     argument = one_argument(argument, arg1);
     argument = one_argument(argument, arg2);
     argument = one_argument(argument, arg3);

    if (IS_AFFECTED(ch, AFF_MIST)) {
	send_to_char( "You can't do that in mist form?\r\n", ch );
	return;
    }

    skill = get_skill(ch, gsn_tailor);

     if (skill < 1 ) {
        send_to_char ("You have no clue how to tailor an armor.\r\n",ch);
        return;
     } 	

      corpse = ch->in_room->contents;

      while ( corpse != NULL
      && ( corpse->item_type != ITEM_CORPSE_NPC
      || !can_see_obj(ch, corpse) ) ) {
                corpse = corpse->next_content;
      }
 
    if ( corpse == NULL ) {
        send_to_char ("You can't find that corpse.\r\n",ch);
        return;
     }  

     if (corpse->item_type != ITEM_CORPSE_NPC
     || corpse->pIndexData->vnum != 10) {
        send_to_char ("This is no corpse.\r\n",ch);
        return;
     }  

     if (ch->move<200) {
        send_to_char ("You are too exhausted to tailor an armor.\r\n",ch);
        return;
     }  
     
    ch->move -=200;

    WAIT_STATE(ch, skill_array[gsn_tailor].beats);

    obj = create_object (get_obj_index (34), 0);
    obj->enchanted = TRUE;
    REMOVE_BIT(obj->wear_flags, ITEM_WEAR_NECK);

     if ( arg3[0] == '\0' ) { 
          obj->level = ch->level;
     } else {
           if (is_number(arg3)) {
               obj->level=atoi(arg3);
           } else {
               obj->level = ch->level;
           }
     }
     if (obj->level > corpse->level) obj->level = corpse->level;

    obj->level = UMAX(1, obj->level);
    obj->level = UMIN(obj->level, ch->level);
    obj->weight = obj->level/5;

    obj->cost = (350 + obj->level* 2) * (obj->level + 1);
    obj->value[0] = obj->level / 10 + 3;
    obj->value[1] = obj->value[0];
    obj->value[2] = obj->value[0];
    obj->value[3] = obj->value[0] / 2;

    obj->material=corpse->material;
    for (rAf = corpse->pIndexData->affected; rAf != NULL; rAf = rAf->next) {
           pAf = new_affect();
           pAf->location   =   rAf->location;
           pAf->skill	       =   rAf->skill;
           pAf->modifier   =   rAf->modifier;
           pAf->type	       =   rAf->type;
           pAf->afn           =   rAf->afn;
           pAf->duration  =   rAf->duration;
           pAf->bitvector =  rAf->bitvector;
           pAf->next          =   obj->affected;
           obj->affected    =   pAf;
    }

    free_string(obj->name);
    free_string(obj->short_descr);
    free_string(obj->description);
    trans = str_dup(corpse->short_descr);
    trans = one_argument(trans, arg1);

     switch (UPPER(arg2[0])) {
          case 'A':
               SET_BIT(obj->wear_flags, ITEM_WEAR_BODY);
                sprintf(buf, "%s %s body armor tailored out %s",quality_by_skill(skill), flag_string( material_string, obj->material ), trans);
                obj->name = str_dup("body armor");
                obj->short_descr =str_dup(buf);
                obj->description = str_dup("A tailored body armor lies here.\n");
                break;
          case 'C':
                SET_BIT(obj->wear_flags, ITEM_WEAR_ABOUT);
                obj->weight = (obj->weight * 14)/10;
                obj->value[0] = (obj->value[0] * 12)/10;
                obj->value[1] = (obj->value[1] * 12)/10;
                obj->value[2] = (obj->value[2] * 12)/10;
                obj->value[3] = (obj->value[3] * 12)/10;
                sprintf(buf, "%s %s cloak tailored out %s", quality_by_skill(skill), flag_string( material_string, obj->material ), trans);
                obj->name = str_dup("cloak");
                obj->short_descr =str_dup(buf);
                obj->description = str_dup("A tailored cloak lies here.\n");
                 break;
          case 'P':
                SET_BIT(obj->wear_flags, ITEM_WEAR_LEGS);
                obj->weight = (obj->weight * 6)/10;
                obj->value[0] = (obj->value[0] * 8)/10;
                obj->value[1] = (obj->value[1] * 8)/10;
                obj->value[2] = (obj->value[2] * 8)/10;
                obj->value[3] = (obj->value[3] * 8)/10;
                sprintf(buf, "%s %s pair of pants tailored out %s",quality_by_skill(skill), flag_string( material_string, obj->material ), trans);
                 obj->name = str_dup("pair pants");
                 obj->short_descr =str_dup(buf);
                 obj->description = str_dup("Some tailored pants lie here.\n");
                 break;
          default:
             send_to_char ("Armor / Pants / Cloak\r\n",ch);
             extract_obj(obj);	
             return;
      }

      roll = number_percent();

      if (roll+80<skill) {
        obj->value[0] +=3;
        obj->value[1] +=3;
        obj->value[2] +=3;
        obj->value[3] +=3;
        obj->cost *=2;
    }
     if (roll+70<skill) {
        obj->value[0] +=2;
        obj->value[1] +=3;
        obj->value[2] +=2;
        obj->value[3] +=1;
        obj->cost = (obj->cost * 15)/10;
    }
    if (roll+50<skill) {
        obj->value[0] +=2;
        obj->value[1] +=1;
        obj->value[2] +=1;
        obj->value[3] +=1;
        obj->cost = (obj->cost * 125)/100;
    }
     if (roll+30<skill) {
        obj->value[0] +=2;
        obj->value[1] +=1;
        obj->value[2] +=1;
        obj->cost = (obj->cost * 112)/100;
    }
    if (roll+10<skill) {
        obj->value[0] +=1;
        obj->value[1] +=1;
        obj->value[2] +=1;
    }

    obj->value[0] = obj->value[0] * (17 + number_range(0,6)) / 20;
    obj->value[1] = obj->value[1] * (17 + number_range(0,6)) / 20;
    obj->value[2] = obj->value[2] * (17 + number_range(0,6)) / 20;
    obj->value[3] = obj->value[3] * (17 + number_range(0,6)) / 20;

    if (obj->weight<1) obj->weight=1;

    obj_from_room(corpse);
    extract_obj(corpse);	
    roll = number_percent();
    if (skill<roll+15) {
        send_to_char ("You fail to tailor a usable armor.\r\n",ch);
        act("$n tries to tailor $p.",ch,obj,NULL,TO_ROOM);
        check_improve(ch, gsn_tailor, FALSE, 1);
        extract_obj(obj);	
        return;
    }
    obj_to_room( obj, ch->in_room );
    send_to_char ("You tailor a usable armor.\r\n",ch);
    act("$n tries to tailor $p.",ch,obj,NULL,TO_ROOM);
    check_improve(ch, gsn_tailor, TRUE, 1);
    return;
}
		

void do_gunsmith (CHAR_DATA *ch, char *argument)	{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
     OBJ_DATA *obj;
     OBJ_DATA *rawm;
    AFFECT_DATA *rAf;
    AFFECT_DATA *pAf;
     int roll;
     int skill;

     argument = one_argument(argument, arg1);
     argument = one_argument(argument, arg2);
     argument = one_argument(argument, arg3);

    if (IS_AFFECTED(ch, AFF_MIST)) {
	send_to_char( "You can't do that in mist form?\r\n", ch );
	return;
    }

    obj = ch->carrying;

    while ( obj != NULL
         && ( obj->item_type != ITEM_FORGING
           || obj->wear_loc != -1
           || !can_see_obj(ch, obj) ) ) {
      obj = obj->next_content;
    }

    if ( obj == NULL ) {
      obj = ch->in_room->contents;

      while ( obj != NULL
           && ( obj->item_type != ITEM_FORGING
             || !can_see_obj(ch, obj) ) ) {
        obj = obj->next_content;
      }
    }

    if ( obj == NULL ) {
       send_to_char("You have no suitable forging object!\r\n", ch);
       return;
    }


 if (str_cmp(arg1, "ammo")) {

    skill = get_skill(ch, gsn_forging);

     if (skill < 1 ) {
        send_to_char ("You have no clue how to forge a gun.\r\n",ch);
        return;
     } 	

     if ( ( rawm = get_eq_char( ch, WEAR_HOLD ) ) == NULL ) {
        send_to_char ("You have to hold some raw material to forge a gun.\r\n",ch);
        return;
     }  

     if (rawm->item_type!=ITEM_RAW) {
        send_to_char ("You cannot forge a gun out of this.\r\n",ch);
        return;
     }  

     if (ch->move<200) {
        send_to_char ("You are too exhausted to forge a gun.\r\n",ch);
        return;
     }  
     
    ch->move -=200;

    WAIT_STATE(ch, skill_array[gsn_forging].beats);
    obj = create_object (get_obj_index (46), 0);
    obj->cost = 25;
    obj->enchanted = TRUE;
     if ( arg3[0] == '\0' ) { 
          obj->level = ch->level;
     } else {
           if (is_number(arg3)) {
               obj->level=atoi(arg3);
           } else {
               obj->level = ch->level;
           }
     }
    obj->level = UMAX(1, obj->level);
    obj->level = UMIN(obj->level, ch->level);
    
    obj->cost = (300+ obj->level* 2) * (obj->level + 1);
    obj->weight = obj->level/5;
    obj->value[1] = UMIN(obj->level/4+1, 8);
    obj->value[2] = ((obj->level+5)/(obj->value[1]+1))+1;
    obj->material=rawm->material;
    for (rAf = rawm->pIndexData->affected; rAf != NULL; rAf = rAf->next) {
           pAf = new_affect();
           pAf->location   =   rAf->location;
           pAf->skill	       =   rAf->skill;
           pAf->modifier   =   rAf->modifier;
           pAf->type	       =   rAf->type;
           pAf->afn           =   rAf->afn;
           pAf->duration  =   rAf->duration;
           pAf->bitvector =  rAf->bitvector;
           pAf->next          =   obj->affected;
           obj->affected    =   pAf;
    }
    free_string(obj->name);
    free_string(obj->short_descr);
    free_string(obj->description);

     switch (UPPER(arg1[0])) {

          case 'G':
                 obj->value[0]=WEAPON_GUN;
                 sprintf(buf, "%s %s rifle",quality_by_skill(skill), flag_string( material_string, obj->material ));
                 obj->name = str_dup("rifle");
                 obj->short_descr = str_dup(buf);
                 obj->description = str_dup("A self made rifle lies here.\n");
                 obj->value[3] = WDT_SHOT;
                 if (!str_cmp(arg2, ".45")) SET_BIT(obj->value[4], WEAPON_CAL45);
                 else if (!str_cmp(arg2, "458")) SET_BIT(obj->value[4], WEAPON_458WINCHESTER);
                 else if (!str_cmp(arg2, ".22")) SET_BIT(obj->value[4], WEAPON_22RIFLE);
                 else if (!str_cmp(arg2, "12guage")) SET_BIT(obj->value[4], WEAPON_12GUAGE);
                 else  SET_BIT(obj->value[4], WEAPON_30CARABINE);
                 SET_BIT(obj->value[4], WEAPON_SIX_SHOT);
                 skill += 10;
                 break;

          case 'H':
                 obj->value[0]=WEAPON_HANDGUN;
                 sprintf(buf, "%s %s pistol",quality_by_skill(skill), flag_string( material_string, obj->material ));
                 obj->name = str_dup("pistol");
                 obj->short_descr = str_dup(buf);
                 obj->description = str_dup("A self made pistol lies here.\n");
                 obj->weight = (obj->weight * 4)/10;
                 obj->value[3]=WDT_SHOT;
                 if (!str_cmp(arg2, ".45")) SET_BIT(obj->value[4], WEAPON_CAL45);
                 else if (!str_cmp(arg2, "357")) SET_BIT(obj->value[4], WEAPON_357MAGNUM);
                 else  SET_BIT(obj->value[4], WEAPON_PARABELLUM);
                 SET_BIT(obj->value[4], WEAPON_TWELVE_SHOT);
                 break;

          case 'S':
                 obj->value[0]=WEAPON_SMG;
                 sprintf(buf, "%s %s submachinegun",quality_by_skill(skill), flag_string( material_string, obj->material ));
                 obj->name = str_dup("submachinegun");
                 obj->short_descr = str_dup(buf);
                 obj->description = str_dup("A self made submachinegun lies here.\n");
                 obj->weight = (obj->weight * 11)/10;
                 obj->value[3]=WDT_SHOT;
                 SET_BIT(obj->value[4],WEAPON_PARABELLUM);
                 SET_BIT(obj->value[4], WEAPON_36_SHOT);
                 skill -= 10;
                 break;

          default:
             send_to_char ("Gun / Handgun / SMG\r\n",ch);
             extract_obj(obj);	
             return;
      }
       
    if (obj->level>120) {
       obj->value[2] +=rawm->value[0];
       obj->weight +=rawm->value[1];
       obj->cost *=2;
    }
    if (obj->level>100) {
       obj->value[2] +=rawm->value[0];
       obj->weight +=rawm->value[1];
       obj->cost *=2;
    }
    if (obj->level>50) {
       obj->value[2] +=rawm->value[0];
       obj->weight +=rawm->value[1];
       obj->cost *=2;
    }
    obj->value[2] +=rawm->value[0];
    obj->weight +=rawm->value[1];

    obj->material=rawm->material;
    roll = number_percent();

    if (roll+80<skill) {
        obj->value[1] +=1;
        obj->cost *=2;
    }
     if (roll+70<skill) {
        obj->value[1] +=1;
        obj->cost = (obj->cost * 15)/10;
    }
    if (roll+50<skill) {
        obj->value[1] +=1;
        obj->cost = (obj->cost * 125)/100;
    }
     if (roll+30<skill) {
        obj->value[1] +=1;
        obj->cost = (obj->cost * 112)/100;
    }
    if (roll+10<skill) {
        obj->value[1] +=1;
    }
    if (obj->weight<1) {
        obj->weight=1;
    }
    if (obj->value[1]<1) {
        obj->value[1]=1;
    }
    if (obj->value[2]<1) {
        obj->value[2]=1;
    }

    obj_from_char(rawm);
    extract_obj(rawm);	
    roll = number_percent();
    if (skill<roll+15) {
        sprintf_to_char (ch, "You fail to forge %s.\r\n", obj->short_descr);
        act("$n tries to forge $p.",ch,obj,NULL,TO_ROOM);
        check_improve(ch, gsn_forging, FALSE, 1);
        extract_obj(obj);	
        return;
    }
    obj_to_room( obj, ch->in_room );
    sprintf_to_char (ch, "You forge %s.\r\n", obj->short_descr);
    act("$n tries to forge $p.",ch,obj,NULL,TO_ROOM);
    check_improve(ch, gsn_forging, TRUE, 1);
    return;

    } else {

         if (str_cmp(arg2, "9mm")
         && str_cmp(arg2, ".45")
         && str_cmp(arg2, "357")
         && str_cmp(arg2, ".458")
         && str_cmp(arg2, ".22")
         && str_cmp(arg2, ".30")
         && str_cmp(arg2, "12guage")) {
                 send_to_char ("Invalid ammo type.\r\n",ch);
                 send_to_char ("9mm / .45 / 357 / 458 / .22 / .30 / 12guage\r\n",ch);
                 return;
         }
       
        if (get_skill(ch, gsn_explosives) == 0
        || get_skill(ch, gsn_chemistry  == 0)) {
             send_to_char ("You don't know how to make ammo.\r\n",ch);
             return;
        }

          if ((rawm = get_eq_char( ch, WEAR_HOLD ) ) == NULL ) {
               send_to_char ("You must hold some gunpowder.\r\n",ch);
               return;
         }  

          if (rawm->pIndexData->vnum != OBJ_VNUM_GUNPOWDER) {
                send_to_char ("You cannot make ammo without gunpowder.\r\n",ch);
                return;
          }  

        if (ch->move<200) {
             send_to_char ("You are too exhausted to make ammo.\r\n",ch);
             return;
        }  
     
        ch->move -=200;
        WAIT_STATE(ch, skill_array[gsn_forging].beats);

        if (!check_skill(ch, gsn_explosives, 0)
        || !check_skill(ch, gsn_chemistry, 0)) {
             send_to_char ("You fail to make ammo.\r\n",ch);
             return;
        }

        obj = create_object (get_obj_index (OBJ_VNUM_AMMO), 0);
        obj->enchanted = TRUE;
        obj->level = 1;
        obj->weight = 2;
        free_string(obj->name);
        free_string(obj->short_descr);
        free_string(obj->description);

         if (!str_cmp(arg2, "9mm")) {
             SET_BIT(obj->value[4], WEAPON_PARABELLUM);
             obj->name = str_dup("clip 9mm parabellum");
             obj->short_descr = str_dup("a 9mm parabellum clip");
             obj->description = str_dup("An ammo clip lies here.\n");

         } else if (!str_cmp(arg2, ".45")) {
             SET_BIT(obj->value[4], WEAPON_CAL45);
             obj->name = str_dup("clip .45");
             obj->short_descr = str_dup("a .45 clip");
             obj->description = str_dup("An ammo clip lies here.\n");

         } else if (!str_cmp(arg2, "357")) {
             SET_BIT(obj->value[4], WEAPON_357MAGNUM);
             obj->name = str_dup("clip 357 magnum");
             obj->short_descr = str_dup("a 357 magnum clip");
             obj->description = str_dup("An ammo clip lies here.\n");

         } else if (!str_cmp(arg2, ".458")) {
             SET_BIT(obj->value[4], WEAPON_458WINCHESTER);
             obj->name = str_dup("clip 458 winchester");
             obj->short_descr = str_dup("a 458 winchester clip");
             obj->description = str_dup("An ammo clip lies here.\n");

         } else if (!str_cmp(arg2, ".22")) {
             SET_BIT(obj->value[4], WEAPON_22RIFLE);
             obj->name = str_dup("clip .22");
             obj->short_descr = str_dup("a .22 clip");
             obj->description = str_dup("An ammo clip lies here.\n");

         } else if (!str_cmp(arg2, ".30")) {
             SET_BIT(obj->value[4], WEAPON_30CARABINE);
             obj->name = str_dup("clip .30");
             obj->short_descr = str_dup("a .30 clip");
             obj->description = str_dup("An ammo clip lies here.\n");

         } else {
             SET_BIT(obj->value[4], WEAPON_12GUAGE);
             obj->name = str_dup("clip 9mm parabellum");
             obj->short_descr = str_dup("a 9mm parabellum clip");
             obj->description = str_dup("An ammo clip lies here.\n");
         }

         obj_from_char(rawm);
         extract_obj(rawm);	
         obj_to_room( obj, ch->in_room );
         send_to_char ("You make some ammo.\r\n",ch);
         act("$n tries to make $p.",ch,obj,NULL,TO_ROOM);
         check_improve(ch, gsn_chemistry, TRUE, 1);
         return;
    }
}


void do_fix (CHAR_DATA *ch, char *argument) {
char target[MAX_INPUT_LENGTH];
int roll;
OBJ_DATA *obj;
int move=100;
int player_chance, skill;

    if (IS_AFFECTED(ch, AFF_MIST)) {
	send_to_char( "You can't do that in mist form?\r\n", ch );
	return;
    }

 obj = ch->carrying;

    while ( obj != NULL
         && ( obj->item_type != ITEM_FORGING
           || obj->wear_loc != -1
           || !can_see_obj(ch, obj) ) ) {
      obj = obj->next_content;
    }

    if ( obj == NULL ) {
      obj = ch->in_room->contents;

      while ( obj != NULL
           && ( obj->item_type != ITEM_FORGING
             || !can_see_obj(ch, obj) ) ) {
        obj = obj->next_content;
      }
    }

   if ( obj == NULL ) {
       send_to_char("You have no suitable forging object!\r\n", ch);
       return;
    }


                skill = get_skill(ch, gsn_forging);

	if (skill < 1){
		send_to_char ("You have no clue how to repair your equipement.\r\n",ch);
		return;
	} 	

	one_argument (argument, target);	
	if ( target[0] == '\0' )	    {
		send_to_char( "Fix what?\r\n", ch );
		return;
   	}
          
	if (ch->move < move) {
		send_to_char ("You do not have enough movement.\r\n",ch);
		return;
	}
	
	if (( obj = get_obj_carry (ch, target)) == NULL) {
		send_to_char ("You are not carrying that.\r\n", ch);
		return;
	}

	if (obj->wear_loc != -1) {
		send_to_char ("The object must be carried to be repaired.\r\n",ch);
		return;
	}

                if ( obj->item_type == ITEM_CONTAINER
                || obj->item_type == ITEM_POTION)  {
                                send_to_char ("You cannot repair a that.\r\n",ch);
		return;
	}
 
                if (IS_SET(obj->extra_flags, ITEM_NO_COND)
                || IS_SET(obj->extra_flags, ITEM_ARTIFACT)) {
                                send_to_char ("You don't need to repair that..\r\n",ch);
		return;
	}

                if (obj->material==MATERIAL_LIQUID
                || obj->material==MATERIAL_FOOD
                || obj->material==MATERIAL_MEAT
                || obj->material==MATERIAL_FLESH
                || obj->material==MATERIAL_EARTH
                || obj->material==MATERIAL_AIR
                || obj->material==MATERIAL_FIRE
                || obj->material==MATERIAL_WATER) {
                     send_to_char("You cannot work with that material!\r\n", ch);
                     return;
                }
	
                ch->move -= move; 
	
	WAIT_STATE(ch, skill_array[gsn_forging].beats);
	roll = number_percent();

	player_chance = (ch->level/4) - (obj->level/4) + (skill/2) + ((get_curr_stat (ch, STAT_STR)/4) - 20) + ((get_curr_stat (ch, STAT_DEX)/4) - 18) + 20 ;
	
	if (roll <= 5) {
		send_to_char ("You have ruined the object. Now it is worthless!\r\n",ch);
		extract_obj(obj);	
		act("$n tries to repair $p and fails miserably.",ch,obj,NULL,TO_ROOM);
		check_improve(ch, gsn_forging, FALSE, 1);
		return;
	} else if (roll <= player_chance) {
		char to_ch[128];
		char to_room[128];
		sprintf (to_ch, "With utmost care, you repair that object.\r\n");
		sprintf (to_room, "$n works with great care and repairs $s equipement.");
		send_to_char (to_ch, ch);
		act(to_room,ch,obj,NULL,TO_ROOM);
		check_improve(ch, gsn_forging, TRUE,1);
                                player_chance -=roll;
                                player_chance *=3;
		obj->condition +=player_chance;
                                if (obj->condition >100) obj->condition=100;
                }  else {
		   send_to_char ("You fail to improve your equipement.\r\n", ch);
		   check_improve(ch, gsn_forging, FALSE,1);
		   obj->condition -=5;
                                   if (obj->condition<1)    extract_obj(obj);	
                                   return;
	}
}


void do_refit (CHAR_DATA *ch, char *argument) {
char target[MAX_INPUT_LENGTH];
int roll;
OBJ_DATA *obj;
int move=100;
int player_chance;
int skill;

    if (IS_AFFECTED(ch, AFF_MIST)) {
	send_to_char( "You can't do that in mist form?\r\n", ch );
	return;
    }

 obj = ch->carrying;

    while ( obj != NULL
         && ( obj->item_type != ITEM_FORGING
           || obj->wear_loc != -1
           || !can_see_obj(ch, obj) ) ) {
      obj = obj->next_content;
    }

    if ( obj == NULL ) {
      obj = ch->in_room->contents;

      while ( obj != NULL
           && ( obj->item_type != ITEM_FORGING
             || !can_see_obj(ch, obj) ) ) {
        obj = obj->next_content;
      }
    }

   if ( obj == NULL ) {
       send_to_char("You have no suitable forging object!\r\n", ch);
       return;
    }

    skill = get_skill(ch, gsn_forging);

    if (skill < 1 ) {
           send_to_char ("You have no clue how to repair your equipement.\r\n",ch);
           return;
    } 	

    one_argument (argument, target);	
    if ( target[0] == '\0' )   {
	send_to_char( "Refit what?\r\n", ch );
	return;
    }
          
    if (ch->move < move) {
	send_to_char ("You do not have enough movement.\r\n",ch);
	return;
    }
	
    if (( obj = get_obj_carry (ch, target)) == NULL) {
	send_to_char ("You are not carrying that.\r\n", ch);
	return;
    }

    if (obj->wear_loc != -1)	{
	send_to_char ("The object must be carried to be refit.\r\n",ch);
	return;
    }

                if (obj->material==MATERIAL_LIQUID
                || obj->material==MATERIAL_FOOD
                || obj->material==MATERIAL_MEAT
                || obj->material==MATERIAL_FLESH
                || obj->material==MATERIAL_EARTH
                || obj->material==MATERIAL_AIR
                || obj->material==MATERIAL_FIRE
                || obj->material==MATERIAL_WATER) {
                     send_to_char("You cannot work with that material!\r\n", ch);
                     return;
                }
	
    ch->move -= move;
    WAIT_STATE(ch, skill_array[gsn_forging].beats);
    roll = number_percent();

    player_chance = (ch->level/4)  - (obj->level/5) + (skill/2)  + ((get_curr_stat (ch, STAT_STR)/4) - 20) + ((get_curr_stat (ch, STAT_DEX)/4) - 18) + 35 ;
	
                if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)) {
		char to_ch[128];
		char to_room[128];
		sprintf (to_ch, "With utmost care, you refit that object.\r\n");
		sprintf (to_room, "$n works with great care and refits $s equipement.");
		send_to_char (to_ch, ch);
		act(to_room,ch,obj,NULL,TO_ROOM);
                                obj->size = get_char_size(ch);
                                return;
                }

	if (roll <= 5) {
   	      send_to_char ("You have ruined the object. Now it is worthless!\r\n",ch);
    	      extract_obj(obj);	
	      act("$n tries to refit $p and fails miserably.",ch,obj,NULL,TO_ROOM);
	      check_improve(ch, gsn_forging, FALSE, 1);
	      return;
	} else if (roll <= player_chance) {
		char to_ch[128];
		char to_room[128];
		sprintf (to_ch, "With utmost care, you refit that object.\r\n");
		sprintf (to_room, "$n works with great care and refits $s equipement.");
		send_to_char (to_ch, ch);
		act(to_room,ch,obj,NULL,TO_ROOM);
		check_improve(ch, gsn_forging, TRUE,1);
                                obj->size = get_char_size(ch);
                }  else {
		   send_to_char ("You fail to improve your equipement.\r\n", ch);
		   check_improve(ch, gsn_forging, FALSE,1);
                                   return;
	}
	return;
}


void do_trap (CHAR_DATA *ch, char *argument)	{
OBJ_DATA *obj;
OBJ_DATA *container = NULL;
MOB_CMD_CONTEXT *mcc;
WEV *wev;
int skill, diff;
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
char arg3[MAX_INPUT_LENGTH];
char buf[MAX_STRING_LENGTH];

  /* Check for help... */

    if ( argument[0] == '\0' ) { 
      send_to_char(
        "Syntax: Trap\r\n"
        "        Trap lay <type> [container]\r\n"
        "        Trap disarm [container]\r\n"
        "        Trap info [container]\r\n"
        "        Trap trigger [container]\r\n", ch);
        return;
    }

    argument = one_argument(argument, arg1);

    if ( !str_cmp(arg1, "lay") ) {
           argument = one_argument(argument, arg2);
           if ( arg2[0] == '\0' ) { 
             send_to_char("No trap type given! blade / tangle / dart / mystic / magic / scare / polymorph / rust\r\n", ch);
              return;
           }          
 
           argument = one_argument(argument, arg3);
           if (arg3[0] != '\0') container = get_obj_room(ch->in_room, arg3);

           if (container) {
                if (container->item_type != ITEM_CONTAINER
                || !IS_SET(container->value[1], CONT_CLOSEABLE)
                || IS_SET(container->wear_flags, ITEM_TAKE)) {
                     send_to_char("The trap won't fit in there!\r\n", ch);
                     return;
                }          
                      
                if (IS_SET(container->value[1], CONT_CLOSED)) {
                     send_to_char("You need to open the container first!\r\n", ch);
                     return;
                }          
           }

          skill = get_skill(ch, gsn_traps);
          if (skill<1) {
              send_to_char("You don't know how to handle traps!\r\n", ch);
              return;
           }          

           if (container) {
                skill -=25;
                obj = container->contains;
           } else {
                obj = ch->in_room->contents;
           }

           while ( obj != NULL
           && obj->item_type != ITEM_TRAP) {
                obj = obj->next_content;
           }
    
           if ( obj != NULL ) {
                  if (container) send_to_char("This container is already trapped!\r\n", ch);
                  else send_to_char("This room is already trapped!\r\n", ch);
                  return;
          }

          if (ch->in_room->sector_type == SECT_WATER_SWIM
          || ch->in_room->sector_type == SECT_WATER_NOSWIM
          || ch->in_room->sector_type == SECT_AIR
          || ch->in_room->sector_type == SECT_SPACE
          || ch->in_room->sector_type == SECT_UNDERWATER) {
                  send_to_char("You can't lay a trap here!\r\n", ch);
                  return;
          }

          if (IS_SET(ch->in_room->room_flags, ROOM_LAW)
          || IS_SET(ch->in_room->room_flags, ROOM_SAFE)
          || IS_SET(ch->in_room->room_flags, ROOM_NEWBIES_ONLY)
          || IS_SET(ch->in_room->area->area_flags, AREA_NEWBIE)
          || IS_SET(ch->in_room->area->area_flags, AREA_LAW)) {
                  send_to_char("Trapping in city areas is immoral!\r\n", ch);
                  return;
          }
  
           if (ch->move<300) {
              send_to_char("You are much too exhaused!\r\n", ch);
              return;
           }          

           ch->move -=300;

           if (!check_skill(ch, gsn_traps, 0)) {           
              send_to_char("You fail.\r\n", ch);
              return;
           }          

           WAIT_STATE(ch, skill_array[gsn_traps].beats);
           obj = create_object ( get_obj_index (OBJ_VNUM_TRAP), 0);
           obj->level = (ch->level) -16 + (skill/5);
           if (obj->level<1) obj->level=1;
           obj->value[0]=0;
           free_string(obj->name);
           free_string(obj->short_descr);
           free_string(obj->description);

           if ( !str_cmp(arg2, "tangle") ) {
               obj->value[0] = TRAP_TANGLE;
               obj->name = str_dup("tangle");
               obj->short_descr = str_dup("a tangle trap");
               obj->description = str_dup("A skillfully concealed tangle trap is here.\n");

           } else if ( !str_cmp(arg2, "dart") ) {
               obj->value[0] = TRAP_DART;
               obj->name = str_dup("dart");
               obj->short_descr = str_dup("a dart trap");
               obj->description = str_dup("A skillfully concealed dart trap is here.\n");

           } else if ( !str_cmp(arg2, "mystic") ) {
               obj->value[0] = TRAP_MYSTIC;
               obj->name = str_dup("mystic");
               obj->short_descr = str_dup("a mystic trap");
               obj->description = str_dup("A skillfully concealed mystic trap is here.\n");
               obj->value[1] = UMIN(obj->level/4+1, 8);
               if (obj->value[1]<1) obj->value[1]=1;
               obj->value[2] = ((obj->level+5)/(obj->value[1]))+1;
               obj->value[1] +=(skill/10)-7;
               if (check_skill(ch, gsn_traps, 0)) obj->value[1] *=2;
               if (check_skill(ch, gsn_traps, 0)) obj->value[1] *=2;
               if (obj->value[1]<1) obj->value[1]=1;

           } else if ( !str_cmp(arg2, "magic") ) {
               if (check_skill(ch, gsn_spell_casting, 0)) {
                     obj->value[0] = TRAP_MAGIC;
                     obj->name = str_dup("magic");
                     obj->short_descr = str_dup("a magic trap");
                     obj->description = str_dup("A skillfully concealed magic trap is here.\n");
                } else {
                     extract_obj(obj);
                     return;
                }

           } else if ( !str_cmp(arg2, "polymorph") ) {
               if (check_skill(ch, gsn_spell_casting, 0)) {
                     obj->value[0] = TRAP_POLY;
                     obj->name = str_dup("polymorph");
                     obj->short_descr = str_dup("a polymorph trap");
                     obj->description = str_dup("A skillfully concealed polymorph trap is here.\n");
                } else {
                     extract_obj(obj);
                     return;
                }

           } else if ( !str_cmp(arg2, "rust") ) {
                     obj->value[0] = TRAP_RUST;
                     obj->name = str_dup("rust");
                     obj->short_descr = str_dup("a rust trap");
                     obj->description = str_dup("A skillfully concealed rust trap is here.\n");

           } else if ( !str_cmp(arg2, "scare") ) {
                if (!container) {
                     send_to_char ("You need a container to lay a scare trap.\r\n",ch);
                     extract_obj(obj);
                     return;
                }
                obj->value[0] = TRAP_SCARE;
                obj->name = str_dup("scare");
                obj->short_descr = str_dup("a scare trap");
                obj->description = str_dup("A skillfully concealed scare trap is here.\n");

           } else if ( !str_cmp(arg2, "blade") ) {
               obj->value[0] = TRAP_WOUND;
               obj->name = str_dup("blade");
               obj->short_descr = str_dup("a blade trap");
               obj->description = str_dup("A skillfully concealed blade trap is here.\n");
               obj->value[1] = UMIN(obj->level/4+1, 8);
                if (obj->value[1]<1) obj->value[1]=1;
               obj->value[2] = ((obj->level+5)/(obj->value[1]))+1;
               obj->value[1] +=(skill/10)-7;
               if (check_skill(ch, gsn_traps, 0)) obj->value[1] *=3;
               if (check_skill(ch, gsn_traps, 0)) obj->value[1] *=3;
               if (obj->value[1]<1) obj->value[1]=1;
           }

           if (obj->value[0]==0) {
              send_to_char("Types: blade / tangle / dart / mystic / magic / scare / polymorph / rust\r\n", ch);
              extract_obj(obj);
              return;
           }          

           if (container) {
                 mcc = get_mcc(ch, ch, NULL, NULL, obj, container, obj->value[0], obj->name);
                 wev = get_wev(WEV_TRAP, WEV_TRAP_LAY, mcc,
                      "You lay a @t0 trap in @s2.\r\n",
                      NULL,
                     "@a2 lays a @t0 trap in @s2.\r\n");
           } else {
                 mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, obj->value[0], obj->name);
                 wev = get_wev(WEV_TRAP, WEV_TRAP_LAY, mcc,
                      "You lay a @t0 trap.\r\n",
                      NULL,
                     "@a2 lays a @t0 trap.\r\n");
          }

           if (!room_issue_wev_challange(ch->in_room, wev)) {
                 free_wev(wev);
                 return;
           }

           if (container) obj_to_obj(obj, container);
           else obj_to_room( obj, ch->in_room );

           check_improve(ch, gsn_traps, TRUE, 1);

           room_issue_wev(ch->in_room, wev);
           free_wev(wev);

           if (!check_skill(ch, gsn_traps, 0)) {           
              send_to_char("Oh No! {RYou triggered your own trap!{x\r\n", ch);
              trap_trigger (ch, container);
           }          

            return;
    }

    if ( !str_cmp(arg1, "disarm") ) {
           argument = one_argument(argument, arg3);
           if (arg3[0] != '\0') container = get_obj_room(ch->in_room, arg3);

           if (container) {
                if (IS_SET(container->value[1], CONT_CLOSED)) {
                     send_to_char("You need to open the container first!\r\n", ch);
                     return;
                }          
           }

          skill = get_skill(ch, gsn_traps);
          if (skill<1) {
              send_to_char("You don't know how to handle traps!\r\n", ch);
              return;
           }          

           if (container) {
                skill -=10;
                obj = container->contains;
           } else {
                obj = ch->in_room->contents;
           }

          while ( obj != NULL
           && ( obj->item_type != ITEM_TRAP) ) {
                obj = obj->next_content;
           }
    
           if ( obj == NULL ) {
                  send_to_char("There is no trap here!\r\n", ch);
                  return;
          }
 
           if (ch->move<300) {
              send_to_char("You are much too exhaused!\r\n", ch);
              return;
           }          

           WAIT_STATE(ch, skill_array[gsn_traps].beats);
           ch->move -=300;
           diff=65+(obj->level-ch->level)/2;

           if (skill + number_percent() > diff + number_percent()) {
              if (container) {
                 mcc = get_mcc(ch, ch, NULL, NULL, obj, container, obj->value[0], obj->name);
                 wev = get_wev(WEV_TRAP, WEV_TRAP_DISARM, mcc,
                      "You disarm a @t0 trap in @s2.\r\n",
                      NULL,
                     "@a2 disarms a @t0 trap in @s2.\r\n");
              } else {
                 mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, obj->value[0], obj->name);
                 wev = get_wev(WEV_TRAP, WEV_TRAP_DISARM, mcc,
                      "You disarm a @t0 trap.\r\n",
                      NULL,
                     "@a2 disarms a @t0 trap.\r\n");
              }

              if (!room_issue_wev_challange(ch->in_room, wev)) {
                   free_wev(wev);
                   return;
              }
              
              room_issue_wev(ch->in_room, wev);
              free_wev(wev);

              check_improve(ch, gsn_traps, TRUE, 1);
              extract_obj(obj);	

           } else {
               check_improve(ch, gsn_traps, FALSE, 1);
               trap_trigger (ch, NULL);     
           }

          return;
    }

    if ( !str_cmp(arg1, "info") ) {

           argument = one_argument(argument, arg3);
           if (arg3[0] !='\0') container = get_obj_room(ch->in_room, arg3);

           if (container) {
                if (IS_SET(container->value[1], CONT_CLOSED)) {
                     send_to_char("You need to open the container first!\r\n", ch);
                     return;
                }          
           }

          skill = get_skill(ch, gsn_traps);
          if (skill<1) {
              send_to_char("You don't know how to handle traps!\r\n", ch);
              return;
           }          

           if (container) {
                skill -=10;
                obj = container->contains;
           } else {
                obj = ch->in_room->contents;
           }

          while ( obj != NULL
           && ( obj->item_type != ITEM_TRAP
           || !can_see_obj(ch, obj) ) ) {
                obj = obj->next_content;
           }
    
           if ( obj == NULL ) {
                  send_to_char("There is no trap here!\r\n", ch);
                  return;
          }
 
           if (ch->move<30) {
              send_to_char("You are much too exhaused!\r\n", ch);
              return;
           }          

           ch->move -=30;

           if (!check_skill(ch, gsn_traps, 0)) {           
              send_to_char("You fail to identify it!{x\r\n", ch);
              return;
           }          

           if (container) {
                 mcc = get_mcc(ch, ch, NULL, NULL, obj, container, obj->value[0], obj->name);
                 wev = get_wev(WEV_TRAP, WEV_TRAP_INFO, mcc,
                      "You examine a @t0 trap in @s2.\r\n",
                      NULL,
                     "@a2 examines a @t0 trap in @s2.\r\n");
           } else {
                 mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, obj->value[0], obj->name);
                 wev = get_wev(WEV_TRAP, WEV_TRAP_INFO, mcc,
                      "You examine a @t0 trap.\r\n",
                      NULL,
                     "@a2 examines a @t0 trap.\r\n");
           }

            if (!room_issue_wev_challange(ch->in_room, wev)) {
                   free_wev(wev);
                   return;
            }
              
             room_issue_wev(ch->in_room, wev);
             free_wev(wev);

	send_to_char("Trap type is ",ch);
	switch (obj->value[0]) {
	  case(TRAP_WOUND) : 
                       send_to_char("Blade Trap\r\n",ch);
	       sprintf(buf,"Level is %d\r\n",obj->level);
                       send_to_char(buf,ch);
  	       sprintf(buf,"Damage is %ld d%ld\r\n", obj->value[1], obj->value[2]);
                       send_to_char(buf,ch);
                       break;

	  case(TRAP_TANGLE)  :
                       send_to_char("Tangle Trap.\r\n",ch);
	       sprintf(buf,"Level is %d\r\n",obj->level);
                       send_to_char(buf,ch);
	       break;	

	  case(TRAP_MAGIC)  :
                       send_to_char("Magic Trap.\r\n",ch);
	       sprintf(buf,"Level is %d\r\n",obj->level);
                       send_to_char(buf,ch);
	       break;	

	  case(TRAP_POLY)  :
                       send_to_char("Polymorph Trap.\r\n",ch);
	       sprintf(buf,"Level is %d\r\n",obj->level);
                       send_to_char(buf,ch);
	       break;	

	  case(TRAP_RUST)  :
                       send_to_char("Rust Trap.\r\n",ch);
	       sprintf(buf,"Level is %d\r\n",obj->level);
                       send_to_char(buf,ch);
	       break;	

	  case(TRAP_SCARE)  :
                       send_to_char("Scare Trap.\r\n",ch);
	       sprintf(buf,"Level is %d\r\n",obj->level);
                       send_to_char(buf,ch);
	       break;	

	  case(TRAP_DART)  :
                       send_to_char("Dart Trap.\r\n",ch);
	       sprintf(buf,"Level is %d\r\n",obj->level);
                       send_to_char(buf,ch);
	       break;	

	  case(TRAP_MYSTIC)  :
                       send_to_char("Mystic Trap.\r\n",ch);
	       sprintf(buf,"Level is %d\r\n",obj->level);
                       send_to_char(buf,ch);
	       sprintf(buf,"Triggerlevel %d\r\n",obj->level-25);
                       send_to_char(buf,ch);
  	       sprintf(buf,"Damage is %ld d%ld\r\n", obj->value[1], obj->value[2]);
                       send_to_char(buf,ch);
	       break;	

	  default	      :
                       send_to_char("unknown.\r\n",ch);	
                       break;
 	}

          return;
    }

    if ( !str_cmp(arg1, "trigger") ) {
           argument = one_argument(argument, arg3);
           if (arg3[0] !='\0') container = get_obj_room(ch->in_room, arg3);

           if (container) {
                if (IS_SET(container->value[1], CONT_CLOSED)) {
                     send_to_char("You need to open the container first!\r\n", ch);
                     return;
                }          
           }

          trap_trigger (ch, container);
          return;
    }

     send_to_char(
        "Syntax: Trap\r\n"
        "        Trap lay <type> [container]\r\n"
        "        Trap disarm [container]\r\n"
        "        Trap info [container]\r\n"
        "        Trap trigger [container]\r\n", ch);
     return;
}


void trap_trigger (CHAR_DATA *ch, OBJ_DATA *container) {
OBJ_DATA *obj;
OBJ_DATA *bond;
OBJ_DATA *aobj;
ROOM_INDEX_DATA *pRoomIndex, *pRoom2;
MOB_INDEX_DATA *pMob;
AFFECT_DATA af;
MOB_CMD_CONTEXT *mcc;
WEV *wev;
int hdam, skill, diff, count;

     if (container) obj = container->contains;
     else obj = ch->in_room->contents;

     while ( obj != NULL
     && ( obj->item_type != ITEM_TRAP ) ) {
               obj = obj->next_content;
     }

     if ( obj == NULL ) {
             if (container) send_to_char("There is no trap in this container!\r\n", ch);
             else send_to_char("There is no trap in this room!\r\n", ch);
             return;
      }

      skill = (get_skill(ch, gsn_detection)+get_skill(ch, gsn_traps))/2;
      diff=30+(obj->level-ch->level)/2;
       if (skill + number_percent() > diff + number_percent()) {
            if (container) send_to_char("Man! This container is {rbooby-trapped{x.\r\n", ch);
            else send_to_char("Man! This room is {rbooby-trapped{x.\r\n", ch);
            return;
       } else {

           if (container) {
                 mcc = get_mcc(ch, ch, NULL, NULL, obj, container, obj->value[0], obj->name);
                 wev = get_wev(WEV_TRAP, WEV_TRAP_TRIGGER, mcc,
                      "You set off a @t0 trap in @s2.\r\n",
                      NULL,
                     "@a2 sets off a @t0 trap in @s2.\r\n");
           } else {
                 mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, obj->value[0], obj->name);
                 wev = get_wev(WEV_TRAP, WEV_TRAP_TRIGGER, mcc,
                      "You set off a @t0 trap.\r\n",
                      NULL,
                     "@a2 sets off a @t0 trap.\r\n");
           }

            if (!room_issue_wev_challange(ch->in_room, wev)) {
                   free_wev(wev);
                   return;
            }
              
             room_issue_wev(ch->in_room, wev);
             free_wev(wev);

            if (obj->value[0]== TRAP_WOUND) {
                 hdam = dice(obj->value[1], obj->value[2]);
                 ch->hit -=hdam;
                 if (hdam > ch->max_hit / 8
                 && !IS_SET(ch->act, ACT_UNDEAD))  SET_BIT(ch->form,FORM_BLEEDING);

                 if ( (!IS_NPC(ch))
                && (IS_IMMORTAL(ch))
                && !IS_AFFECTED(ch, AFF_INCARNATED)
                && (ch->hit < 1) ) {
                      ch->hit = 1;
                }

                send_to_char("You are hit by a {Wsharp{x blade!{x.\r\n",ch);

                update_pos( ch );
                kill_msg( ch);
  
          } else if (obj->value[0]== TRAP_MYSTIC) {
                 if (ch->level >obj->level-25) { 
                      hdam = dice(obj->value[1], obj->value[2]);
                      ch->hit -=hdam;
                      if (hdam > ch->max_hit / 8
                      && !IS_SET(ch->act, ACT_UNDEAD))  SET_BIT(ch->form,FORM_BLEEDING);

                      if ( (!IS_NPC(ch))
                     && (IS_IMMORTAL(ch))
                     && !IS_AFFECTED(ch, AFF_INCARNATED)
                     && (ch->hit < 1) ) {
                           ch->hit = 1;
                     }

                     send_to_char("You are hit by a {Ymystical energies{x!{x.\r\n",ch);

                     update_pos( ch );
                     kill_msg( ch);
                 }

          } else if (obj->value[0]== TRAP_MAGIC) {
                     act ("$n sets off a wound trap.",ch,NULL,NULL,TO_ROOM);
                     send_to_char("You trigger a magic trap.\r\n",ch);

                     for (count = 0 ; count < 50 ; count++ ) {
                           pRoomIndex = get_room_index( number_range( 0, 32767 ) );
                           if ( pRoomIndex == NULL 
                           ||  pRoomIndex->area == NULL ) {
                                   continue;
                           }
  
                           if ( ch->in_room->area != NULL
                           && ch->in_room->area->zone != pRoomIndex->area->zone ) {
                                         continue;
                           }   

                           if (can_see_room(ch, pRoomIndex)
                          && !IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY)
                          && !IS_SET(pRoomIndex->area->area_flags, AREA_LOCKED) ) {
	                   break;
                          }
                     }

                     if ( count >= 50
                     || pRoomIndex == NULL ) {
                            bug("Count reached %d", count);
                            send_to_char("You feel dizzy and disorientated!\r\n", ch);
                            return;
                     }

                     if ( IS_SET(time_info.flags, TIME_NIGHT) ) {
                         if (pRoomIndex->night != 0) { 
                              pRoom2 = get_room_index(pRoomIndex->night);
                              if ( pRoom2 != NULL )  pRoomIndex = pRoom2;
                         }
                     } else if ( IS_SET(time_info.flags, TIME_DAY) ) {
                         if (pRoomIndex->day != 0) { 
                              pRoom2 = get_room_index(pRoomIndex->day);
                              if ( pRoom2 != NULL ) pRoomIndex = pRoom2;
                         }
                     }

                     if (ch->fighting != NULL) stop_fighting(ch,TRUE);
                    send_to_char("You have been teleported!\r\n", ch);
                    act( "$n vanishes!", ch, NULL, NULL, TO_ROOM );
                    add_tracks(ch, DIR_NONE, DIR_OTHER);
                   char_from_room(ch);
                   char_to_room( ch, pRoomIndex );
                   add_tracks(ch, DIR_OTHER, DIR_HERE);
                   act( "$n appears in a blinding flash of light!", ch, NULL, NULL, TO_ROOM );
                   do_look(ch, "auto" );


          } else if (obj->value[0]== TRAP_POLY) {
                     act ("$n sets off a polymorph trap.",ch,NULL,NULL,TO_ROOM);
                     send_to_char("You trigger a polymorph trap.\r\n",ch);

                     if (IS_SET(ch->act, ACT_WERE)
                     || IS_AFFECTED(ch, AFF_POLY)
                     || is_affected(ch, gafn_mask_self)
                     || is_affected(ch, gafn_mask_hide)) {
                         send_to_char("But nothing happens.\r\n",ch);
                         return;
                     }

                     for (count = 0 ; count < 50 ; count++ ) {
                           pMob = get_mob_index(number_range(0, 32767 ) );
                           if (pMob) break;
                     }

                     if (!pMob) {
                            send_to_char("You feel dizzy and disorientated!\r\n", ch);
                            return;
                     }

                     af.type 	= SKILL_UNDEFINED;
                     af.afn	 	= gafn_mask_force;
                     af.level     	= obj->level;
                     af.duration  	= obj->level;
    	     af.location  	= APPLY_NONE;
                     af.modifier  	= 0;
                     af.bitvector 	= AFF_POLY;
    	     affect_to_char( ch, &af );

	     free_string(ch->description_orig);
	     free_string(ch->short_descr_orig);
	     free_string(ch->long_descr_orig);
	     ch->description_orig = str_dup (ch->description);
	     ch->short_descr_orig = str_dup (ch->short_descr);
	     ch->long_descr_orig  = str_dup (ch->long_descr);
                     ch->race_orig=ch->race;

	     free_string(ch->description);
	     free_string(ch->short_descr);
	     free_string(ch->long_descr);
	     ch->description = str_dup (pMob->description);
                     ch->race = pMob->race;
	     ch->short_descr = str_dup (pMob->short_descr);
	     ch->long_descr  = str_dup (pMob->long_descr);
	     free_string(ch->poly_name);
	     ch->poly_name = str_dup(pMob->player_name);

                     act( "$n appears changed!", ch, NULL, NULL, TO_ROOM );
                     send_to_char("You feel different!\r\n", ch);
     

          } else if (obj->value[0]== TRAP_RUST) {
                     act ("$n is hit by a gush of acid.",ch,NULL,NULL,TO_ROOM);
                     send_to_char("You are hit by a gush of acid.\r\nYor equipment rusts.\r\n",ch);
                     harmful_effect(ch, 3*obj->level, TAR_CHAR_OFFENSIVE, DAM_ACID);


          } else if (obj->value[0]== TRAP_TANGLE) {
                 bond = create_object (get_obj_index (29), 0);
                 bond->cost=0;
                 bond->value[0]=50+(obj->level/25);
                 free_string(bond->name);
                 free_string(bond->short_descr);
                 free_string(bond->description);

                 bond->name=str_dup("tangle rope");
                 bond->short_descr=str_dup("a tangle rope");
                 bond->description=str_dup("A long sticky rope lies here.");
                 obj_to_room (bond, ch->in_room);
                 bond->timer = number_range(10,20);

                 if ( ( aobj = get_eq_char( ch, WEAR_ARMS ) ) != NULL ) {
                         unequip_char( ch, aobj );
                 }  
                 obj_from_room( bond );
                 obj_to_char( bond, ch );
                 equip_char( ch, bond, WEAR_ARMS );
                 SET_BIT (ch->act, ACT_BOUND);
                 set_activity( ch, POS_STUNNED, NULL, ACV_NONE, NULL);

                 send_to_char("{yYou can't move!{x.\r\n",ch);

            } else if (obj->value[0]== TRAP_DART) {
                 send_to_char("{yA little dart hits you!{x.\r\n",ch);
                 af.type	    =	SKILL_UNDEFINED;
                 af.afn	 = gafn_poison;
                 af.level     = obj->level;
                 af.duration  = 1 + (obj->level/10);
                 af.location  = APPLY_STR;
                 af.modifier  = -1 * dice(2,4);
                 af.bitvector = AFF_POISON;
                 affect_join( ch, &af );
                 send_to_char( "{GYou feel very sick.{x\r\n", ch );
                 act("$n looks very {Gill{x.",ch,NULL,NULL,TO_ROOM);

            } else if (obj->value[0]== TRAP_SCARE) {
                 sprintf_to_char(ch, "A jack in the box jumps out of %s.\r\n",container->short_descr);

                 if (IS_AFFECTED(ch, AFF_CALM)
                 || IS_AFFECTED(ch, AFF_RELAXED)) {
                         do_say(ch, "Oh my. that was funny!");
                         return;
                 }
            
                 if (obj->level - ch->sanity*4 > number_percent()
                 && !IS_IMMORTAL(ch)) {
                     if (number_percent() < 26) {
                          send_to_char("{yYou have a heart attack!{x.\r\n",ch);
                          do_say(ch, "Aaargh!");
                          ch->hit = -50;
                     } else {
                         check_sanity(ch, obj->level);
                     }
                 }

                 update_pos( ch );
                 kill_msg( ch);
            }
            extract_obj(obj);	
       }

       return;
}


char *quality_by_skill(int skill) {

    if (skill < 5) return "a junk";
    if (skill < 20) return "a roughly hewn";
    if (skill < 45) return "a standard";
    if (skill < 75) return "a fine";
    if (skill < 100) return "an exceptional";
    if (skill < 130) return "a masterfully crafted";
    return "a supernatural";
}


void do_bomb (CHAR_DATA *ch, char *argument)	{
               	OBJ_DATA *obj;
                int skill;
                int settimer;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

  /* Check for help... */

    if ( argument[0] == '\0' ) { 
      send_to_char(
        "Syntax: Bomb\r\n"
        "        Bomb lay <timer>\r\n"
        "        Bomb disarm <bomb>\r\n"
        "        Bomb info <bomb>\r\n", ch);
        return;
    }

    argument = one_argument(argument, arg1);

    if ( !str_cmp(arg1, "lay") ) {
        
           argument = one_argument(argument, arg2);

           if ( arg2[0] == '\0' ) { 
             send_to_char("No timer given!\r\n", ch);
              return;
           }          
 
          settimer=atoi(arg2);
          if (settimer  <= 0) {
              send_to_char("This timer setting is impossible.\r\n", ch);
              return;
           }          
        
          skill = get_skill(ch, gsn_explosives);
          if (skill<1) {
              send_to_char("You don't know how to handle explosives!\r\n", ch);
              return;
           }          

            if ((obj = get_eq_char( ch, WEAR_HOLD ) ) == NULL ) {
                 send_to_char ("You have to hold some kind of explosive.\r\n",ch);
                 return;
            }  

            if (obj->item_type != ITEM_EXPLOSIVE ) {
                 send_to_char ("You have to hold some kind of explosive.\r\n",ch);
                 return;
            }  

            if (obj->value[3] <=0
            && obj->value[4] <=0) {
              send_to_char("This explosive can't be timed.\r\n", ch);
              return;
           }          

            if (settimer < obj->value[3]
            || settimer > obj->value[4]) {
              send_to_char("This timer setting is impossible.\r\n", ch);
              return;
           }          

          if (ch->in_room->sector_type == SECT_WATER_SWIM
          || ch->in_room->sector_type == SECT_WATER_NOSWIM
          || ch->in_room->sector_type == SECT_AIR
          || ch->in_room->sector_type == SECT_SPACE
          || ch->in_room->sector_type == SECT_UNDERWATER) {
                  send_to_char("You can't lay a bomb here!\r\n", ch);
                  return;
          }

           if (ch->move<300) {
              send_to_char("You are much too exhaused!\r\n", ch);
              return;
           }          

           ch->move -=300;

           if (!check_skill(ch, gsn_explosives, 0)) {           
              send_to_char("You fail.\r\n", ch);
              obj_from_char( obj );
              extract_obj( obj );
              return;
           }          

           WAIT_STATE(ch, skill_array[gsn_explosives].beats);
            obj->item_type=ITEM_TIMEBOMB;
            obj->timer = settimer;
            SET_BIT(obj->extra_flags, ITEM_CONCEALED);
            REMOVE_BIT(obj->wear_flags, ITEM_TAKE);
            obj_from_char( obj );
            obj_to_room( obj, ch->in_room );
            send_to_char ("You lay a time bomb.\r\n",ch);
            act("$n lays a time bomb.",ch,NULL,NULL,TO_ROOM);
            check_improve(ch, gsn_explosives, TRUE, 1);

           if (!check_skill(ch, gsn_explosives, 0)) {           
              send_to_char("Oh No! {RYou triggered your own timebomb!{x\r\n", ch);
              bomb_explode(obj);
           }          
           return;

   } else if ( !str_cmp(arg1, "disarm") ) {
           argument = one_argument(argument, arg2);

           if ( arg2[0] == '\0' ) { 
             send_to_char("Disarm what?\r\n", ch);
              return;
           }          

           obj = get_obj_list(ch, arg2, ch->in_room->contents );
           if ( obj == NULL )  {
	act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	return;
           }

            if (obj->item_type != ITEM_TIMEBOMB ) {
                 send_to_char ("That's no real bomb.\r\n",ch);
                 return;
            }  
           
            skill = get_skill(ch, gsn_explosives);
            if (skill<1) {
                send_to_char("Don't be stupid!!\r\n", ch);
                return;
            }          

           if (ch->move<300) {
              send_to_char("You are much too exhaused!\r\n", ch);
              return;
           }          

           ch->move -=300;

           WAIT_STATE(ch, skill_array[gsn_explosives].beats);

           if (!check_skill(ch, gsn_explosives, 0)) {           
              send_to_char("You fail.\r\n", ch);
              if (number_percent() < 50) {
                   send_to_char("The bomb triggers.\r\n", ch);
                   bomb_explode(obj);
              }
              return;
           }          

            obj->item_type=ITEM_EXPLOSIVE;
            obj->timer = -1;
            SET_BIT(obj->wear_flags, ITEM_TAKE);
            REMOVE_BIT(obj->extra_flags, ITEM_CONCEALED);
            obj_from_room( obj );
            obj_to_char( obj, ch );
            send_to_char ("You disarm a time bomb.\r\n",ch);
            act("$n disarms a time bomb.",ch,NULL,NULL,TO_ROOM);
            return;

   } else if ( !str_cmp(arg1, "info") ) {
           argument = one_argument(argument, arg2);

           if ( arg2[0] == '\0' ) { 
             send_to_char("Info about what?\r\n", ch);
              return;
           }          

           obj = get_obj_list(ch, arg2, ch->in_room->contents );
           if ( obj == NULL )  {
	act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	return;
           }

            if (obj->item_type != ITEM_TIMEBOMB ) {
                 send_to_char ("That's no real bomb.\r\n",ch);
                 return;
            }  
           
            skill = get_skill(ch, gsn_explosives);
            if (skill<1) {
                send_to_char("Don't be stupid!!\r\n", ch);
                return;
            }          

            sprintf( buf, "The bomb is going to detonate in %d hours.\r\n", obj->timer);
            send_to_char(buf, ch);
            return;

     } else {
                 send_to_char(
        "Syntax: Bomb\r\n"
        "        Bomb lay <timer>\r\n"
        "        Bomb disarm <bomb>\r\n"
        "        Bomb info <bomb>\r\n", ch);
        return;
     }

     return;
}


void bomb_explode (OBJ_DATA *bomb) {
CHAR_DATA *vch;
CHAR_DATA *vch_next;
int damage;

                for ( vch = char_list; vch != NULL; vch = vch_next )    {
	        vch_next	= vch->next;
    	        if ( vch->in_room == NULL ) continue;
                        if ( vch->in_room->area == bomb->in_room->area) {
      	               if ( vch->in_room->subarea == bomb->in_room->subarea) {
                                   damage = dice(bomb->value[1], bomb->value[2]);
                                    if (vch->in_room != bomb->in_room) damage /=5;
                                    if (damage>0) {
                                         env_damage(vch, damage, DAM_FIRE,
                                         "",
                                         "{RThe blast of explosion {yhits{R you! [@n0]{x\r\n");
                                    }
                               } else {
   	                   send_to_char( "The earth trembles and shivers.\r\n", vch );
                               }
                        }
                }
                obj_from_room(bomb);
                extract_obj(bomb);
                return;
}


void do_envenom(CHAR_DATA *ch, char *argument) {
OBJ_DATA *obj;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

   /* Must have the skill to try it... */

    if (get_skill(ch, gsn_envenom) < 1) {
      send_to_char("Are you crazy? You'd poison yourself!\r\n",ch);
      return;
    }

   /* Should also check that they have a bottle of poison... */

   /* What are we poisoning? */

    if (argument == '\0') {
      send_to_char("Envenom what item?\r\n",ch);
      return;
    }

    obj =  get_obj_list(ch,argument,ch->carrying);

    if (!obj) {
      send_to_char("You don't have that item.\r\n",ch);
      return;
    }

   /* Ok, get context and build the wev... */

    mcc = get_mcc(ch, ch, NULL, NULL, obj, NULL, 0, NULL);

   /* Work out what happened... */

    if ( obj->item_type == ITEM_FOOD
    || obj->item_type == ITEM_DRINK_CON ) {
 
      wev = get_wev(WEV_POISON, WEV_POISON_FOOD, mcc,
                   "You pour some poison into @p2!\r\n",
                   "@a2 pours some poison into @p2!\r\n",
                   "@a2 pours some poison into @p2!\r\n");

    } else if ( obj->item_type == ITEM_FOUNTAIN ) {
 
      wev = get_wev(WEV_POISON, WEV_POISON_FOUNTAIN, mcc,
                   "You pour a lot of poison into @p2!\r\n",
                   "@a2 pours a lot of poison into @p2!\r\n",
                   "@a2 pours a lot of poison into @p2!\r\n");

    } else if ( obj->item_type == ITEM_WEAPON ) {

      wev = get_wev(WEV_POISON, WEV_POISON_WEAPON, mcc,
                   "You smeer some poison over @p2!\r\n",
                   "@a2 smeers some poison over @p2!\r\n",
                   "@a2 smeers some poison over @p2!\r\n");
   
    } else {

      wev = get_wev(WEV_POISON, WEV_POISON_OTHER, mcc,
                   "You covers @p2 with poison!\r\n",
                   "@a2 covers @p2 with poison!\r\n",
                   "@a2 covers @p2 with poison!\r\n");
    } 

   /* Challange... */

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

   /* Poison food and drink... */

    if ( obj->item_type == ITEM_FOOD 
    || obj->item_type == ITEM_DRINK_CON) {
     
     /* Won't work on blessed or already poisoned food... */

      if (!IS_OBJ_STAT(obj,ITEM_BLESS)
      && obj->value[3] == 0 ) {

        if (check_skill(ch, gsn_envenom, 0)) {
          obj->value[3] = 1;
          check_improve(ch,gsn_envenom,TRUE,4);
        } else {
          check_improve(ch,gsn_envenom,FALSE,4);
        }
      }
    } 

   /* Poison fountains... */

    if ( obj->item_type == ITEM_FOUNTAIN ) { 
     
     /* Won't work on blessed or already poisoned fountains... */

      if (!IS_OBJ_STAT(obj,ITEM_BLESS)
      && obj->value[1] == 0 ) {

        if (check_skill(ch, gsn_envenom, 0)) {
          obj->value[1] = 1;
          check_improve(ch,gsn_envenom,TRUE,4);
        } else {
          check_improve(ch,gsn_envenom,FALSE,4);
        }
      }
    } 

   /* Poison weapons... */

    if (obj->item_type == ITEM_WEAPON) {

     /* Won't work on branded weapons, blessed weapons, blunt weapons,
        or already poisoned weapons... */ 
     
      if ( !IS_WEAPON_STAT(obj, WEAPON_FLAMING)
        && !IS_WEAPON_STAT(obj, WEAPON_FROST)
        && !IS_WEAPON_STAT(obj, WEAPON_VAMPIRIC)
        && !IS_WEAPON_STAT(obj, WEAPON_SHARP)
        && !IS_WEAPON_STAT(obj, WEAPON_VORPAL)
        && !IS_WEAPON_STAT(obj, WEAPON_ACID)
        && !IS_WEAPON_STAT(obj, WEAPON_LIGHTNING)
        && !IS_WEAPON_STAT(obj, WEAPON_PLAGUE)
        &&  obj->value[3] >= 0
        &&  attack_table[obj->value[3]].damage != DAM_BASH
        && !IS_WEAPON_STAT(obj,WEAPON_POISON)
        && !IS_OBJ_STAT(obj,ITEM_BLESS))  {
        
        if (check_skill(ch, gsn_envenom, 0)) {
          SET_BIT(obj->value[4], WEAPON_POISON);
          check_improve(ch,gsn_envenom,TRUE,3);
        } else {
          check_improve(ch,gsn_envenom,FALSE,3);
        }
      }  
    }

    WAIT_STATE(ch, skill_array[gsn_envenom].beats);

    room_issue_wev( ch->in_room, wev);
    free_wev(wev);
    return;
}


void do_combine(CHAR_DATA *ch, char *argument) {
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
char arg3[MAX_INPUT_LENGTH];
char arg4[MAX_INPUT_LENGTH];
OBJ_INDEX_DATA *pObj, *pProduct;
OBJ_DATA *obj1 = NULL;
OBJ_DATA *obj2 = NULL;
OBJ_DATA *obj3 = NULL;
OBJ_DATA *obj4 = NULL;
OBJ_DATA *product;
MOB_CMD_CONTEXT *mcc;
WEV *wev;
VNUM i;
VNUM cn[4];
int k, l, nk, nl;
bool check[4];
bool ok;

     for (i = 0; i < 4; i++) cn[i] = 0;

     argument = one_argument(argument, arg1);
     argument = one_argument(argument, arg2);
     argument = one_argument(argument, arg3);
     argument = one_argument(argument, arg4);

     if (arg1[0] != '\0') {
         if ((obj1 = get_obj_here(ch, arg1)) == NULL) {
            sprintf_to_char(ch, "You don't have a %s.\r\n", arg1);
            return;
         }
         cn[0] = obj1->pIndexData->vnum;
     }
     if (arg2[0] != '\0') {
         if ((obj2 = get_obj_here(ch, arg2)) == NULL) {
            sprintf_to_char(ch, "You don't have a %s.\r\n", arg2);
            return;
         }
         cn[1] = obj2->pIndexData->vnum;
     }
     if (arg3[0] != '\0') {
         if ((obj3 = get_obj_here(ch, arg3)) == NULL) {
            sprintf_to_char(ch, "You don't have a %s.\r\n", arg3);
            return;
         }
         cn[2] = obj3->pIndexData->vnum;
     }
     if (arg4[0] != '\0') {
         if ((obj4 = get_obj_here(ch, arg4)) == NULL) {
            sprintf_to_char(ch, "You don't have a %s.\r\n", arg4);
            return;
         }
         cn[3] = obj4->pIndexData->vnum;
     }

     for (i =1; i < 64000; i++) {
         if ((pObj = get_obj_index(i)) != NULL) {
             if (pObj->item_type == ITEM_PROTOCOL) {
                 nk =0;
                 nl = 0;
                 for (k = 0; k < 4; k++) check[k] = FALSE;
                 for (k = 0; k < 4; k++) {
                      if (cn[k] != 0) nk++;
                      if (pObj->value[k+1] != 0) nl++;
                 }
                 if (nk != nl) continue;   
              
                 for (k = 0; k < 4; k++) {
                      for (l = 1; l < 5; l++) {
                           if (check[k]) break;
                           if (cn[k] == pObj->value[l]) check[k] = TRUE;
                      }
                 }             

                 ok = TRUE;
                 for (k = 0; k < nk+1; k++) {
                      if (!check[k]) ok = FALSE;
                 }
                 if (ok) break;
             }
         }
     }

     if (!pObj) {
         send_to_char("You have no idea what to do with these objects.\r\n", ch);
         return;
     }

     if ((pProduct = get_obj_index(pObj->value[0])) == NULL) {
         send_to_char("You're not sure what this should become.\r\n", ch);
         return;
     }

     mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL);

     if (pObj->wear_cond) {
        if (!ec_satisfied_mcc(mcc, pObj->wear_cond, TRUE)) {
            free_mcc(mcc);
            sprintf_to_char(ch, "You don't feel ready to build %s.\r\n", pProduct->short_descr );
            return;
        }
     }

     free_mcc(mcc);
     product = create_object(pProduct, pProduct->level);

     mcc = get_mcc(ch, ch, NULL, NULL, product, NULL, 0, pObj->description);
     if (!str_cmp(pObj->description, "(no description)")) {
          wev = get_wev(WEV_OPROG, WEV_OPROG_COMBINE, mcc,
                "You combine those objects to @p2.\r\n",
                "@a2 combines some objects to @p2.\r\n",
                "@a2 combines some objects to @p2.\r\n");
     } else {
          wev = get_wev(WEV_OPROG, WEV_OPROG_COMBINE, mcc,
                "You combine those objects to @p2.\r\n@t0\r\n",
                "@a2 combines some objects to @p2.\r\n@t0\r\n",
                "@a2 combines some objects to @p2.\r\n@t0\r\n");
     }
     if (!room_issue_wev_challange(ch->in_room, wev)) {
         free_wev(wev);
         extract_obj(product);
         return;
     }

     room_issue_wev( ch->in_room, wev);
     free_wev(wev);

     if (obj1) extract_obj(obj1);
     if (obj2) extract_obj(obj2);
     if (obj3) extract_obj(obj3);
     if (obj4) extract_obj(obj4);
     obj_to_char(product, ch);
     return;
}


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
#include "deeds.h"

DECLARE_DO_FUN(do_give);

/* Object vnums for Quest Rewards */

#define QUEST_ITEM2 20052
#define QUEST_ITEM3 20051
#define QUEST_ITEM4 20050

/* Local functions */

void 			generate_quest    	(CHAR_DATA *ch, CHAR_DATA *questman );
void 			quest_update       	(void);
bool 			quest_level_diff  	( int clevel, int mlevel);
ROOM_INDEX_DATA      *find_location	(CHAR_DATA *ch, char *arg);


void do_mission(CHAR_DATA *ch, char *argument) {
    CHAR_DATA *questman;
    CHAR_DATA *victim;
    CHAR_DATA *assassin;
    MOB_INDEX_DATA *questinfo;
    OBJ_INDEX_DATA *questinfo2;
    OBJ_DATA *obj = NULL;
    OBJ_DATA *obj_next = NULL;
    char buf [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    bool obj_found = FALSE;

    if (IS_NPC(ch)) return;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0')    {
        send_to_char("Mission commands:\r\n",ch);
        send_to_char("  mission points\r\n",ch);
        send_to_char("  mission info\r\n",ch);
        send_to_char("  mission time\r\n",ch);
        send_to_char("  mission request\r\n",ch);
        send_to_char("  mission complete\r\n",ch);
        send_to_char("  mission list\r\n",ch);
        send_to_char("  mission buy <item>\r\n",ch);
        if (IS_IMMORTAL(ch)) send_to_char("  mission reset <char>\r\n",ch);
        return;
    }

     if (!str_cmp(arg1, "reset"))  {
         if (!IS_IMMORTAL(ch)) {
                send_to_char("You are no Immortal.\r\n",ch);
                return;
         }
        
         if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
         }

         if (IS_NPC(victim)) {
	send_to_char( "Not on an NPC.\r\n", ch );
	return;
         }

         REMOVE_BIT(victim->plr, PLR_QUESTOR);
         victim->pcdata->questgiver = NULL;
         victim->pcdata->countdown = 0;
         victim->pcdata->questmob = 0;
         victim->pcdata->questobj = 0;
         victim->pcdata->nextquest = 0;
         send_to_char("Reset.\r\n",ch);
         send_to_char("Your mission status has been reset.\r\n",victim);
         return;
    }

    if (!str_cmp(arg1, "info"))  {
        if (IS_SET(ch->plr, PLR_QUESTOR))   {
            if (ch->pcdata->questmob == -1 && ch->pcdata->questgiver->short_descr != NULL) {
                  sprintf(buf, "Your mission is {ralmost{x complete!\r\nGet back to %s before your time runs out!\r\n",ch->pcdata->questgiver->short_descr);
                  send_to_char(buf, ch);

            } else if (ch->pcdata->questmob == -100) {
                  sprintf(buf, "You are on a mission to slay the dreaded %s!\r\n",ch->pcdata->questplr);
                  send_to_char(buf, ch);

            } else if (ch->pcdata->questmob > 0)  {
                  questinfo = get_mob_index(ch->pcdata->questmob);
                  if (questinfo != NULL)   {
                      if (IS_SET(questinfo->act, ACT_CRIMINAL)) {
                          sprintf(buf, "You are on a mission to slay the dreaded %s!\r\n",questinfo->short_descr);
                      } else {
                          sprintf(buf, "You are on a mission to transport some documents to %s!\r\n",questinfo->short_descr);
                      }
                      send_to_char(buf, ch);
                  } else { 
                      send_to_char("You aren't currently on a mission.\r\n",ch);
                  }
                  return;

            } else if (ch->pcdata->questobj > 0)  {
                  questinfo2 = get_obj_index(ch->pcdata->questobj);
                  if (questinfo2 != NULL)   {
                      sprintf(buf, "You are on a mission to find the stolen %s!\r\n",questinfo2->short_descr);
                      send_to_char(buf, ch);
                  } else { 
                      send_to_char("You aren't currently on a mission.\r\n",ch);
                  }
                  return;
            }

        }  else {
            send_to_char("You aren't currently on a mission.\r\n",ch);
        }
        return;
    }
    if (!str_cmp(arg1, "points")) {
        sprintf(buf, "You have %d fame.\r\n",ch->pcdata->questpoints);
        send_to_char(buf, ch);
        return;
    }     else if (!str_cmp(arg1, "time"))    {
        if (!IS_SET(ch->plr, PLR_QUESTOR))         {
            send_to_char("You aren't currently on a mission.\r\n",ch);
            if (ch->pcdata->nextquest > 1)             {
                sprintf(buf, "There are %d minutes remaining until you can go on another mission.\r\n",ch->pcdata->nextquest);
                send_to_char(buf, ch);
            }  else if (ch->pcdata->nextquest == 1)  {
                sprintf(buf, "There is less than a minute remaining until you can go on another mision.\r\n");
                send_to_char(buf, ch);
            }
        }  else if (ch->pcdata->countdown > 0)  {
            sprintf(buf, "Time left for current mission: %d\r\n",ch->pcdata->countdown);
            send_to_char(buf, ch);
        }
        return;
    }

    for ( questman = ch->in_room->people; questman != NULL; questman = questman->next_in_room ) {
        if (!IS_NPC(questman)) continue;
        if (questman->spec_fun == spec_lookup( "spec_questmaster" )) break;
    }

    if (questman == NULL || questman->spec_fun != spec_lookup( "spec_questmaster" )) {
        send_to_char("You can't do that here.\r\n",ch);
        return;
    }

    if ( questman->fighting != NULL) {
        send_to_char("Wait until the fighting stops.\r\n",ch);
        return;
    }

    ch->pcdata->questgiver = questman;

    if (!str_cmp(arg1, "list")) {
        act( "$n asks $N for a list of rewards.", ch, NULL, questman, TO_ROOM);
        act ("You ask $N for a list of rewards.",ch, NULL, questman, TO_CHAR);
        sprintf(buf, "Bounty Hunters Guild currently offering:\r\n\
100 fame......... {cGlass{x of Leng\r\n\
100 fame......... {cBlanket{x of Dreams\r\n\
90 fame.......... Hired Assasin {cMaster{x\r\n\
75 fame.......... Yuggoth Scrying {cCrystal{x\r\n\
75 fame.......... {cIntrepidity{x\r\n\
55 fame.......... The Master {cKey{x\r\n\
45 fame.......... 10 {cPractices{x\r\n\
40 fame.......... 100,000 {cCopper{x pieces\r\n\
30 fame.......... Hired {cAssassin{x\r\n\
30 fame.......... {cBodyguard{x\r\n\
15 fame.......... Full {cRepair{x\r\n\
15 fame.......... Guild {cMembership{x\r\n\
10 fame.......... {cCorpse{x Recovery\r\n\
To buy an item, type 'MISSION BUY <item>'.\r\n");
        send_to_char(buf, ch);
        return;
    } else if (!str_cmp(arg1, "buy"))    {
        if (arg2[0] == '\0') {
            send_to_char("To buy an item, type 'MISSION BUY <item>'.\r\n",ch);
            return;
        }

        if (is_name(arg2, "membership"))  {
            if (ch->pcdata->questpoints >= 15)  {
                ch->pcdata->questpoints -= 15;
                 add_deed(ch, 10565, 2, "Official Bounty Hunter");
           } else {
                sprintf(buf, "Sorry, %s, but you don't have enough fame for that.",ch->name);
                do_say(questman,buf);
                return;
            }

        } else if (is_name(arg2, "corpse"))  {
            if (ch->pcdata->questpoints >= 10) {
                for (obj = object_list; obj; obj = obj->next) {
                     if (obj->item_type == ITEM_CORPSE_PC) {
                           if (!str_cmp(obj->owner, ch->name)) break;
                     }
                }

                if (!obj) {
                      sprintf_to_char(ch, "Your corpse could not be found.\r\n");
                      return;
                }

                if (obj->in_room) obj_from_room(obj);
                else if (obj->carried_by) obj_from_char(obj);
                else if (obj->in_obj) obj_from_obj(obj);

                obj_to_room(obj, ch->in_room);
                sprintf_to_room(ch->in_room, "{mA corpse is carried in by some strange robed creatures.{x\r\n");
                ch->pcdata->questpoints -= 10;
                return;
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough fame for that.",ch->name);
                do_say(questman,buf);
                 return;
             }      

        } else if (is_name(arg2, "repair"))  {
            if (ch->pcdata->questpoints >= 15) {
                for (obj = ch->carrying; obj != NULL; obj = obj_next ) {
	    obj_next = obj->next_content;
	    if (can_see_obj( questman, obj )) {
                         obj->condition=100;
                         act( "$n repairs $p.", questman, obj, NULL, TO_ROOM );
                    }
                }
                ch->pcdata->questpoints -= 15;
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough fame for that.",ch->name);
                do_say(questman,buf);
                 return;
             }      

        } else if (is_name(arg2, "intrepidity")) {
            if (ch->pcdata->questpoints >= 75)  {
                 if (IS_SET(ch->act, ACT_BRAINSUCKED)) {
                      send_to_char( "Your are already kind of fearless.\r\n", ch );
                      return;
                 }

                 if (IS_AFFECTED(ch, AFF_POLY)) {
                       send_to_char( "Reveal your real face first?\r\n", ch );
                       return;
                 }
                ch->pcdata->questpoints -= 75;
                SET_BIT(ch->act,ACT_BRAINSUCKED);
                send_to_char( "You feel so calm now.\r\n",ch);
            }  else  {
                sprintf(buf, "Sorry, %s, but you don't have enough fame for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        
        } else if (is_name(arg2, "crystal"))  {
            if (ch->pcdata->questpoints >= 75)  {
                ch->pcdata->questpoints -= 75;
                obj = create_object(get_obj_index(QUEST_ITEM3),ch->level);
            } else  {
                sprintf(buf, "Sorry, %s, but you don't have enough fame for that.",ch->name);
                do_say(questman,buf);
                return;
            }

        } else if (is_name(arg2, "key"))  {
            if (ch->pcdata->questpoints >= 55)  {
                ch->pcdata->questpoints -= 55;
                obj = create_object(get_obj_index(OBJ_VNUM_MASTER_KEY),ch->level);
                obj->value[0] = number_range(3, 7);
            } else  {
                sprintf(buf, "Sorry, %s, but you don't have enough fame for that.",ch->name);
                do_say(questman,buf);
                return;
            }

        } else if (is_name(arg2, "glass"))  {
            if (ch->pcdata->questpoints >= 100)  {
                ch->pcdata->questpoints -= 100;
                obj = create_object(get_obj_index(QUEST_ITEM4),ch->level);
            }  else  {
                sprintf(buf, "Sorry, %s, but you don't have enough fame for that.",ch->name);
                do_say(questman,buf);
                return;
            }

        } else if (is_name(arg2, "blanket"))  {
            if (ch->pcdata->questpoints >= 100)  {
                ch->pcdata->questpoints -= 100;
                obj = create_object(get_obj_index(QUEST_ITEM2),ch->level);
            } else  {
                sprintf(buf, "Sorry, %s, but you don't have enough fame for that.",ch->name);
                do_say(questman,buf);
                return;
            }

        } else if (is_name(arg2, "practices")) {
            if (ch->pcdata->questpoints >= 45)  {
                ch->pcdata->questpoints -= 45;
                ch->practice += 10;
                act( "$N gives 10 practices to $n.", ch, NULL, questman, TO_ROOM );
                act( "$N gives you 10 practices.",   ch, NULL, questman, TO_CHAR );
                return;
            }  else  {
                sprintf(buf, "Sorry, %s, but you don't have enough fame for that.",ch->name);
                do_say(questman,buf);
                return;
            }

        } else if (is_name(arg2, "copper")) {
            if (ch->pcdata->questpoints >= 40)  {
                ch->pcdata->questpoints -= 40;
                ch->gold[0] += 100000;
                act( "$N gives 100,000 {rCopper{x pieces to $n.", ch, NULL, questman, TO_ROOM );
                act( "$N has 100,000 in {rCopper{x transfered from $s Swiss account to your balance.",   ch, NULL, questman, TO_CHAR );
                return;
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough fame for that.",ch->name);
                do_say(questman,buf);
                return;
            }

        } else if (is_name(arg2, "master")) {
            if (ch->pcdata->questpoints >= 90)  {
   
                if ( argument[0] == '\0')    {
	         send_to_char( "You need to specify a target for the assassin.\r\n", ch );
	         return;
                }

                if ( ( victim = get_char_world( ch, argument ) ) == NULL )    {
	        send_to_char( "They are not here.\r\n", ch );
	        return;
                }

                ch->pcdata->questpoints -= 90;
                assassin = create_mobile_level(get_mob_index( MOB_VNUM_ASSASSIN),"npc soldier", 149);
                char_to_room( assassin, victim->in_room );
                assassin->comm = COMM_NOCHANNELS;
                act( "An assassin master steps out of the shadows...", assassin, NULL, NULL, TO_ROOM );
                act( "... and attacks!", assassin, NULL, NULL, TO_ROOM );
                multi_hit( assassin, victim, TYPE_UNDEFINED );
                check_assist(assassin,victim);
                act( "$n hires an assassin master to kill $N.", ch, NULL, victim, TO_ROOM );
                act( "You hire an assassin master to kill $N.",   ch, NULL, victim, TO_CHAR );
                return;
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough fame for that.",ch->name);
                do_say(questman,buf);
                return;
            }

        } else if (is_name(arg2, "assassin")) {
            if (ch->pcdata->questpoints >= 30)  {
   
                if ( argument[0] == '\0')    {
	         send_to_char( "You need to specify a target for the assassin.\r\n", ch );
	         return;
                }

                if ( ( victim = get_char_world( ch, argument ) ) == NULL )    {
	        send_to_char( "They are not here.\r\n", ch );
	        return;
                }

                ch->pcdata->questpoints -= 30;
                assassin = create_mobile_level( get_mob_index( MOB_VNUM_ASSASSIN ), "npc soldier", 80+ch->level/5 );
                char_to_room( assassin, victim->in_room );
                assassin->comm = COMM_NOCHANNELS;
                act( "An assassin steps out of the shadows...", assassin, NULL, NULL, TO_ROOM );
                act( "... and attacks!", assassin, NULL, NULL, TO_ROOM );
                multi_hit( assassin, victim, TYPE_UNDEFINED );
                check_assist(assassin,victim);
                act( "$n hires an assassin to kill $N.", ch, NULL, victim, TO_ROOM );
                act( "You hire an assassin to kill $N.",   ch, NULL, victim, TO_CHAR );
                return;
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough fame for that.",ch->name);
                do_say(questman,buf);
                return;
            }

        } else if (is_name(arg2, "bodyguard")) {
            if (ch->pcdata->questpoints >= 30)  {

                 if( ch->pet != NULL ) {
                       send_to_char( "You already have a pet.\r\n", ch );
                       return;
                 }
                 ch->pcdata->questpoints -= 30;

                 assassin = create_mobile_level( get_mob_index( MOB_VNUM_BODYGUARD ), "npc soldier", ch->level+15 );
                 char_to_room( assassin, ch->in_room );
                 assassin->comm = COMM_NOCHANNELS;
                 SET_BIT(assassin->affected_by, AFF_CHARM);
                 SET_BIT(assassin->act, ACT_FOLLOWER);
                 add_follower(assassin, ch );
                 assassin->leader = ch;
                 ch->pet = assassin;
                 act( "$N steps out of the shadows...", ch, NULL, assassin, TO_ROOM );
                 act( "$N steps out of the shadows...", ch, NULL, assassin, TO_CHAR );
                 do_emote(assassin, "looks at you expectently" );
                return;
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough fame for that.",ch->name);
                do_say(questman,buf);
                return;
            }

        }  else  {
            sprintf(buf, "I don't have that item, %s.",ch->name);
            do_say(questman, buf);
        }
        if (obj != NULL)  {
            act( "$N gives $p to $n.", ch, obj, questman, TO_ROOM );
            act( "$N gives you $p.",   ch, obj, questman, TO_CHAR );
            obj_to_char(obj, ch);
        }
        return;

    } else if (!str_cmp(arg1, "request"))   {
        act( "$n asks $N for a mission.", ch, NULL, questman, TO_ROOM);
        act ("You ask $N for a mission.",ch, NULL, questman, TO_CHAR);
        if (IS_SET(ch->plr, PLR_QUESTOR))  {
            sprintf(buf, "But you're already on a mission!");
            do_say(questman, buf);
            return;
        }
        if (ch->pcdata->nextquest > 0)   {
            sprintf(buf, "There is nothing to do right now, %s.",ch->name);
            do_say(questman, buf);
            sprintf(buf, "Come back later.");
            do_say(questman, buf);
            return;
        }

        sprintf(buf, "Let's see if I can find a dangerous mission for you, %s!",ch->name);
        do_say(questman, buf);
        ch->pcdata->questmob = 0;
        ch->pcdata->questobj = 0;

        generate_quest(ch, questman);

        if (ch->pcdata->questmob > 0
        || ch->pcdata->questmob == -100
        || ch->pcdata->questobj >0)     {
            ch->pcdata->countdown = number_range(20,40);
            SET_BIT(ch->plr, PLR_QUESTOR);
            sprintf(buf, "You have %d minutes to complete this mission.",ch->pcdata->countdown);
            do_say(questman, buf);
            sprintf(buf, "Fight hard and shoot straight!");
            do_say(questman, buf);
        }
        return;
    }     else if (!str_cmp(arg1, "complete"))    {
        act( "$n informs $N $e has completed $s mission.", ch, NULL, questman, TO_ROOM);
        act ("You inform $N you have completed $s mission.",ch, NULL, questman, TO_CHAR);
        if (ch->pcdata->questgiver != questman)        {
            sprintf(buf, "I never sent you on a mission! Perhaps you're thinking of someone else.");
            do_say(questman,buf);
            return;
        }

        if (IS_SET(ch->plr, PLR_QUESTOR))    {
            if (ch->pcdata->questobj >0 && ch->pcdata->countdown > 0) {
                for (obj = ch->carrying; obj != NULL; obj= obj_next) {
                    obj_next = obj->next_content;
                    if (obj != NULL && obj->pIndexData->vnum == ch->pcdata->questobj) {
                        obj_found = TRUE;
                        obj_from_char(obj);
                        extract_obj(obj);
                        break;
                    }
                }
            }
            if ((ch->pcdata->questmob == -1 && ch->pcdata->countdown > 0)
            || (obj_found == TRUE && ch->pcdata->countdown >0)) {
                int reward, pointreward, pracreward;

                reward = number_range(1000,10000);
                pointreward = number_range(2,5);

                sprintf(buf, "Congratulations on completing your mission!");
                do_say(questman,buf);
                sprintf(buf,"As a reward, I am giving you some fame, and %d {rCopper{x.",reward);
                do_say(questman,buf);
                if (chance(15))  {
                    pracreward = number_range(1,3);
                    sprintf(buf, "You gain %d practices!\r\n",pracreward);
                    send_to_char(buf, ch);
                    ch->practice += pracreward;
                }

                REMOVE_BIT(ch->plr, PLR_QUESTOR);
                ch->pcdata->questgiver = NULL;
                free_string(ch->pcdata->questplr);
                ch->pcdata->questplr = strdup("");
                ch->pcdata->countdown = 0;
                ch->pcdata->questmob = 0;
                ch->pcdata->questobj = 0;
                ch->pcdata->nextquest = 30;
                ch->gold[0] += reward;
                ch->pcdata->questpoints += pointreward;

                return;
            } else if ((ch->pcdata->questmob > 0 && ch->pcdata->countdown >0)  
                           || (obj_found == FALSE && ch->pcdata->countdown >0)
                           || (ch->pcdata->questmob == -100 && ch->pcdata->countdown >0))   {
                                  sprintf(buf, "You haven't completed the mission yet, but there is still time!");
                                  do_say(questman, buf);
                                  return;
            }
        }
        if (ch->pcdata->nextquest > 0) sprintf(buf,"But you didn't complete your mission in time!");
        else sprintf(buf, "You have to REQUEST a mission first, %s.",ch->name);
        do_say(questman, buf);
        return;
    }

    send_to_char("MISSION commands: POINTS INFO TIME REQUEST COMPLETE LIST BUY.\r\n",ch);
    send_to_char("For more information, type 'HELP MISSION'.\r\n",ch);
    return;
}


void generate_quest(CHAR_DATA *ch, CHAR_DATA *questman) {
CHAR_DATA *victim;
OBJ_DATA *obj;
ROOM_INDEX_DATA *room;
char buf[MAX_STRING_LENGTH];
char arg[MAX_INPUT_LENGTH];
int count;
int selector = number_percent();


   if (selector < 60) {
        for (victim = char_list; victim != NULL; victim = victim->next) {
            if (!IS_NPC(victim)) {
                if (quest_level_diff(ch->level, victim->level) == TRUE
                && IS_SET(victim->act, ACT_CRIMINAL)
                && !IS_IMMORTAL(victim)
                && ch != victim
                && !IS_SET(victim->affected_by, AFF_CHARM)
                && chance(10)) break;
            } else {
                if (quest_level_diff(ch->level, victim->level) == TRUE
                && victim->pIndexData != NULL
                && IS_SET(victim->act, ACT_CRIMINAL)
                && !IS_SET(victim->act, ACT_PROTECTED)
                && !IS_SET(victim->affected_by, AFF_CHARM)
                && chance(10)) break;
            }
        }

        if ( victim == NULL || victim==ch)  {
            do_say(questman, "I'm sorry, but I don't have any missions for you at this time.");
            do_say(questman, "Try again later.");
            ch->pcdata->nextquest = 5;
            return;
        }

        if ( ( room = find_location( ch, victim->name ) ) == NULL ) {
            sprintf(buf, "I'm sorry, but I don't have any missions for you at this time.");
            do_say(questman, buf);
            sprintf(buf, "Try again later.");
            do_say(questman, buf);
            ch->pcdata->nextquest = 5;
            return;
        }

        sprintf(buf, "There is a bounty on the criminal %s.",victim->short_descr);
        do_say(questman, buf);
        sprintf(buf, "Try to find & kill him, before %s can escape.",victim->short_descr);
        do_say(questman, buf);

        if (room->name != NULL) {
             sprintf(buf, "Seek %s out somewhere in the vicinity of %s!",victim->short_descr,room->name);
             do_say(questman, buf);
         }
         if (IS_NPC(victim)) {
               ch->pcdata->questmob = victim->pIndexData->vnum;
               free_string(ch->pcdata->questplr);
               ch->pcdata->questplr = strdup("");
         } else {
               ch->pcdata->questmob = -100;
               free_string(ch->pcdata->questplr);
               ch->pcdata->questplr = strdup(victim->short_descr);
         }       

   } else if (selector < 80) {
         for (count = 0 ; count < 50; count++ ) {
               room = get_room_index(number_range (0, 32767));
               if ( room == NULL ||  room->area == NULL ) continue;
  
               if (!IS_SET(room->area->area_flags, AREA_SOLITARY)
               && !IS_SET(room->area->area_flags, AREA_LOCKED)
               && !IS_SET(room->room_flags, ROOM_SOLITARY)
               && !IS_SET(room->room_flags, ROOM_GODS_ONLY)
               && !IS_SET(room->room_flags, ROOM_IMP_ONLY)
               && !IS_SET(room->room_flags, ROOM_NEWBIES_ONLY)
               && !IS_SET(room->room_flags, ROOM_HEROES_ONLY)
               && (room->sector_type == SECT_INSIDE
                     || room->sector_type == SECT_UNDERGROUND)) {
	    break;
               }
         }

          if ( room == NULL || count > 50)  {
              sprintf(buf,"Obj set countout by %s.",ch->name);
              log_string(buf); 
              bug("Count reached %d", count);
              do_say(questman, "I'm sorry, but I don't have any missions for you at this time.");
              do_say(questman, "Try again later.");
              ch->pcdata->nextquest = 5;
              return;
          }

         obj = create_object(get_obj_index(number_range(20060,20069)),1);     
         obj_to_room (obj, room);
         obj->timer = 50;         
         victim = create_mobile_level(get_mob_index( MOB_VNUM_THIEF ), "npc soldier", ch->level);
         char_to_room( victim, room );

        sprintf(buf, "%s has been stolen and hidden somewhere underground.",obj->short_descr);
        do_say(questman, buf);
        sprintf(buf, "Try to find it before they can transport it abroad.");
        do_say(questman, buf);

        if (room->name != NULL) {
             sprintf(buf, "Seek %s out somewhere in the vicinity of %s!",obj->short_descr,room->name);
             do_say(questman, buf);
         }
         ch->pcdata->questobj = obj->pIndexData->vnum;

    } else {
        for (victim = char_list; victim != NULL; victim = victim->next) {
                if (victim->pIndexData
                && IS_SET(victim->act, ACT_PROTECTED)
                && !IS_SET(victim->act, ACT_CRIMINAL)
                && !IS_SET(victim->affected_by, AFF_CHARM)
                && chance(5)) break;
        }

        if (!victim  || victim==ch)  {
            do_say(questman, "I'm sorry, but I don't have any missions for you at this time.");
            do_say(questman, "Try again later.");
            ch->pcdata->nextquest = 5;
            return;
        }

        if ( ( room = find_location( ch, victim->name ) ) == NULL ) {
            sprintf(buf, "I'm sorry, but I don't have any missions for you at this time.");
            do_say(questman, buf);
            sprintf(buf, "Try again later.");
            do_say(questman, buf);
            ch->pcdata->nextquest = 5;
            return;
        }

        sprintf(buf, "We've got these important documents for %s.",victim->short_descr);
        do_say(questman, buf);
        do_say(questman, "Deliver them in time but be aware dark forces will try to intercept you.");

        if (room->name != NULL) {
             sprintf(buf, "Seek %s out somewhere in the vicinity of %s!",victim->short_descr,room->name);
             do_say(questman, buf);
         }
         ch->pcdata->questmob = victim->pIndexData->vnum;
         free_string(ch->pcdata->questplr);
         ch->pcdata->questplr = strdup("");
         obj = create_object(get_obj_index(OBJ_VNUM_QUEST_DOCUMENT), ch->level);
         obj->timer = 50;         
         obj_to_char(obj, questman);
         one_argument(obj->name, arg);
         sprintf(buf, "%s %s", arg, ch->name);
         do_give(questman, buf);
    }
    return;
}


bool quest_level_diff(int clevel, int mlevel) {
    if (clevel < 10 && mlevel < 15) return TRUE;
    else if (clevel > 9 && clevel < 25 && mlevel < 35) return TRUE;
    else if (clevel > 24 && clevel < 50 && mlevel> 20 &&  mlevel < 75) return TRUE;
    else if (clevel > 49 && clevel < 100 && mlevel> 30 && mlevel < 120) return TRUE;
    else if (clevel > 99 && mlevel> 40) return TRUE;
    else return FALSE;
}
                

void quest_update(void) {
    DESCRIPTOR_DATA *d;
    CHAR_DATA *ch;

    for ( d = descriptor_list; d != NULL; d = d->next )    {
        if (d->character != NULL && d->connected == CON_PLAYING)  {
        ch = d->character;

        if (IS_NPC(ch)) continue;
        
       if (SET_BIT(ch->plr, PLR_QUESTOR)
       && ch->pcdata->questmob == 0
       && ch->pcdata->questobj == 0) {
               REMOVE_BIT(ch->plr, PLR_QUESTOR);
               ch->pcdata->questgiver = NULL;
               ch->pcdata->countdown = 0;
       }

        if (ch->pcdata->nextquest > 0)  {
            ch->pcdata->nextquest--;
            if (ch->pcdata->nextquest == 0)   {
                send_to_char("You may now go on a mission again.\r\n",ch);
                return;
            }
        }  else if (IS_SET(ch->plr,PLR_QUESTOR))  {
            if (--ch->pcdata->countdown <= 0)   {
                char buf [MAX_STRING_LENGTH];

                ch->pcdata->nextquest = 30;
                sprintf(buf, "You have run out of time for your mission!\r\nObviously you target could escape.\r\nYou may try again in %d minutes.\r\n",ch->pcdata->nextquest);
                send_to_char(buf, ch);
                REMOVE_BIT(ch->plr, PLR_QUESTOR);
                ch->pcdata->questgiver = NULL;
                ch->pcdata->countdown = 0;
                ch->pcdata->questmob = 0;
                free_string(ch->pcdata->questplr);
                ch->pcdata->questplr = strdup("");
                ch->pcdata->questobj = 0;
            }
            if (ch->pcdata->countdown > 0 && ch->pcdata->countdown < 6)  {
                send_to_char("Better hurry, you're almost out of time for your mission!\r\n",ch);
                return;
            }
        }
        }
    }
    return;
}



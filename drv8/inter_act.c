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
#include "affect.h"
#include "skill.h"
#include "profile.h"
#include "exp.h"
#include "olc.h"
#include "race.h"
#include "gsn.h"

/*
 * Local functions.
 */

void vampire_suck 	(CHAR_DATA *ch, char *arg2);
void vampire_embrace 	(CHAR_DATA *ch, char *arg2);
void vampire_strength 	(CHAR_DATA *ch, char *arg2);
void vampire_celerity 	(CHAR_DATA *ch, char *arg2);
void vampire_fortitude 	(CHAR_DATA *ch, char *arg2);
void vampire_heal 	(CHAR_DATA *ch, char *arg2);
void vampire_dark 	(CHAR_DATA *ch, char *arg2);
void vampire_majesty 	(CHAR_DATA *ch, char *arg2);
void vampire_mesmerize 	(CHAR_DATA *ch, CHAR_DATA *victim, char *argument);
void vampire_mist               (CHAR_DATA *ch, char *arg2);
void vampire_levitate         	(CHAR_DATA *ch, char *arg2);

void were_change 	(CHAR_DATA *ch);
void were_rage 	                (CHAR_DATA *ch);
void were_growl 	                (CHAR_DATA *ch);

void lich_touch                    (CHAR_DATA *ch, char *arg2);
void lich_dominate              (CHAR_DATA *ch, char *arg2);
void lich_regenerate              (CHAR_DATA *ch, char *arg2);

void do_bountylist              (CHAR_DATA *ch);
void kill_msg                         (CHAR_DATA *ch);  
char* check_mob_owner    (CHAR_DATA *mob);


#define YITH_START	10320

DECLARE_DO_FUN(do_flee);
DECLARE_DO_FUN(do_echo);
DECLARE_DO_FUN(do_list);
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(yith_adapt);
DECLARE_DO_FUN(yith_abduct);

struct who_slot {
	CHAR_DATA *ch;
	struct who_slot *next;
};



void do_shop( CHAR_DATA *ch, char * argument ) {
char buf[MAX_STRING_LENGTH];
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
char arg3[MAX_INPUT_LENGTH];
CHAR_DATA *keeper;
SHOP_DATA *pShop;
OBJ_INDEX_DATA *nObjIndex = NULL;
OBJ_DATA     *obj;
RESET_DATA *nReset;
RESET_DATA *prev = NULL;
int vff = 0;
int iTrade, value;
char* lastarg;
bool rfound;
int currency = find_currency(ch);
	
    argument=one_argument(argument, arg1);
    argument=one_argument(argument, arg2);
    argument=one_argument(argument, arg3);
	
    if (arg1[0]=='\0') { 
               send_to_char ("\r\nShop commands:\r\n",ch);
               send_to_char ("\r\n",ch);
               send_to_char ("  SHOP REPORT\r\n",ch);
               send_to_char ("  SHOP PROFIT <buy> <sell>\r\n",ch);
               send_to_char ("  SHOP TYPE <slot> <type>\r\n",ch);
               send_to_char ("  SHOP GOLD DEPOSIT <ammount>\r\n",ch);
               send_to_char ("  SHOP GOLD WITHDRAW <ammount>\r\n",ch);
               send_to_char ("  SHOP GOLD BALANCE\r\n",ch);
               send_to_char ("  SHOP EXPAND\r\n",ch);
               send_to_char ("  SHOP DUMP <object>\r\n",ch);
               return;
    }

    keeper = find_keeper(ch, TRUE);
    if (keeper == NULL) return;

    lastarg = check_mob_owner(keeper);
    if (!str_cmp(lastarg, "not found")) return;
      
   if (str_cmp(ch->short_descr, lastarg)) {
               send_to_char ("This is not your shop.\r\n",ch);
               return;
    }

    pShop = keeper->pIndexData->pShop;
    
    if (!str_cmp(arg1, "report")) {
        do_list(ch, "");
        sprintf( buf, "Shop data for [{C%s{w]:\r\n"
	              "  Markup for purchaser: {c%d{x%%\r\n"
	              "  Markdown for seller:  {c%d{x%%\r\n", keeper->short_descr, pShop->profit_buy, pShop->profit_sell );
        send_to_char( buf, ch );
        sprintf( buf, "  Hours: {c%d{x to {c%d{x.\r\n", pShop->open_hour, pShop->close_hour );
        send_to_char( buf, ch );

        for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ ) {
	    if ( pShop->buy_type[iTrade] != 0 )    {
		if ( iTrade == 0 ) {
		    send_to_char( "  Number Trades Type\r\n", ch );
		    send_to_char( "  ------ -----------\r\n", ch );
		}
                                 if (pShop->buy_type[iTrade] == 999) sprintf(buf, "  [%4d] Bank\r\n", iTrade);
                                 else if (pShop->buy_type[iTrade] == 998) sprintf(buf, "  [%4d] Hotel\r\n", iTrade);
		 else sprintf(buf, "  [%4d] %s\r\n", iTrade, flag_string( type_flags, pShop->buy_type[iTrade] ) );
		send_to_char( buf, ch );
	    }
         }
         return;

    } else if (!str_cmp(arg1, "profit")) {
          pShop->profit_buy = atoi(arg2);
          pShop->profit_sell = atoi(arg3);
          sprintf(buf,"Buy : %d\r\n"
                              "Sell: %d\r\n", pShop->profit_buy, pShop->profit_sell);
          send_to_char( buf, ch );
          send_to_char( "Shop profit set.\r\n", ch);
          return;

    } else if (!str_cmp(arg1, "type")) {
          if (pShop->buy_type[atoi( arg2 )] == 998
          || pShop->buy_type[atoi( arg2 )] == 999) {
	send_to_char("You can't change this type!\r\n", ch );
	return;
          }

          if ( atoi( arg2 ) >= MAX_TRADE )	{
	sprintf( buf, "May sell %d item types max.\r\n", MAX_TRADE );
	send_to_char( buf, ch );
	return;
          }

           if ((value = flag_value( type_flags, arg3 ) ) == NO_FLAG ) {
	 send_to_char( "That type of item is not known.\r\n", ch );
	 return;
           }

           pShop->buy_type[atoi( arg2 )] = value;
           send_to_char( "Shop type set.\r\n", ch);
           return;

    } else if (!str_cmp(arg1, "gold")) {
           if (!str_cmp(arg2, "deposit")) {
                value = atoi(arg3);
                if (value < 1) {
                    send_to_char( "Be reasonable.\r\n", ch);
                    return;
                }
                if (value > ch->gold[currency]) {
                    sprintf_to_char(ch, "You haven't got that much %s.\r\n", flag_string(currency_type, currency));
                    return;
                }
                ch->gold[currency] -=value;
                keeper->gold[currency] +=value;
                sprintf(buf,"%d %s given to %s", value, flag_string(currency_type, currency), keeper->short_descr);
                send_to_char(buf,ch);
                return;
           } else if (!str_cmp(arg2, "withdraw")) {
                value = atoi(arg3);
                if (value < 1) {
                    send_to_char( "Be reasonable.\r\n", ch);
                    return;
                }
                if (value > keeper->gold[currency]) {
                    sprintf_to_char(ch, "The shopkeeper hasn't got that much %s.\r\n", flag_string(currency_type, currency));
                    return;
                }
                ch->gold[currency] +=value;
                keeper->gold[currency] -=value;
                sprintf_to_char(ch,"%d %s gained from %s", value, flag_string(currency_type, currency), keeper->short_descr);
                send_to_char(buf,ch);
                return;
           } else if (!str_cmp(arg2, "balance")) {
                send_to_char( "You ask the shopkeeper about his current balance.\r\n", ch);
                sprintf(buf, "I've got %ld %s at the moment.", keeper->gold[currency], flag_string(currency_type, currency));
                do_say(keeper, buf);
           } else {
              send_to_char ("No such SHOP GOLD command.\r\n",ch);
              return;
           }
    } else if (!str_cmp(arg1, "expand")) {
           if (ch->gold[currency] < 250000) {
                    sprintf_to_char(ch, "You don't have 250K %s.\r\n", flag_string(currency_type, currency));
                    return;
           }
           ch->gold[currency] -= 250000;
           vff = number_range(66,64000);
           nObjIndex = get_obj_index(vff);
           while (nObjIndex == NULL
           || nObjIndex->cost == 0
           || nObjIndex->level <1
           || nObjIndex->level >61
           || nObjIndex->zmask !=0
           || nObjIndex->item_type == ITEM_PORTAL
           || nObjIndex->item_type == ITEM_MONEY
           || nObjIndex->item_type == ITEM_KEY
           || nObjIndex->item_type == ITEM_LOCKER_KEY
           || nObjIndex->item_type == ITEM_CORPSE_NPC
           || IS_SET(nObjIndex->area->area_flags, AREA_LOCKED)
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
           add_reset(ch->in_room, nReset, 0);
           send_to_char( "Ok.\r\n", ch);
           send_to_char( "Your new goods will arrive at next repop.\r\n", ch);
           SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
           return;
    } else if (!str_cmp(arg1, "dump")) {

           if ( !ch->in_room->reset_first ) {
	send_to_char( "This is nothing being sold here.\r\n", ch );
	return;
           }

           rfound = FALSE;

           for (nReset = ch->in_room->reset_first; nReset != NULL; nReset = nReset->next ) {
                  nObjIndex = get_obj_index(nReset->arg1);
                  if (is_name(arg2, nObjIndex->name)) break;
                  prev = nReset;
           }

           if (!nReset) {
	send_to_char( "This is not being sold here.\r\n", ch );
	return;
           }

           if (prev)  prev->next = prev->next->next;
           else  ch->in_room->reset_first = ch->in_room->reset_first->next;

           for ( ch->in_room->reset_last = ch->in_room->reset_first;  ch->in_room->reset_last->next;  ch->in_room->reset_last = ch->in_room->reset_last->next );

           sprintf(buf, "%s deleted.\r\n", capitalize(nObjIndex->short_descr));
           send_to_char(buf,ch);

           obj = get_obj_carry(keeper, arg2);
           if (obj == NULL) {
	send_to_char( "Obj only existed as a reset.\r\n", ch );
	return;
           }
  
           obj_from_char(obj);
           extract_obj(obj);
           return;

      } else {
              send_to_char ("No such SHOP command.\r\n",ch);
              return;
    }
    return;
}


void do_bountylist( CHAR_DATA *ch ) {

    char buf[MAX_STRING_LENGTH];
    char fname[MAX_STRING_LENGTH]; 
    char form[MAX_STRING_LENGTH];
    char output[10 * MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int immMatch=0;
    int mortMatch=0;
    struct who_slot *table[MAX_LEVEL + 1]; /*want 1 to 100, not 0 to 99*/ 
    int counter;

    for (counter=1;counter <= MAX_LEVEL;counter++) {
        table[counter] = NULL;
    }

   /* Zeran - form table */

    for ( d = descriptor_list; d != NULL; d = d->next ) {
            CHAR_DATA *wch;
	
             /* Check for match against restrictions... */
             /* Don't use trust as that exposes trusted mortals... */ 
            wch   = ( d->original != NULL ) ? d->original : d->character;
  
             if (  d->connected != CON_PLAYING) { 
                    continue;
             }

             if (IS_IMMORTAL(wch)) {
                  immMatch++;
             } else {
	  mortMatch++;
             }
	
              if (table[wch->level] == NULL) { 
                   table[wch->level] = (struct who_slot *)malloc(sizeof(struct who_slot)); 
                   table[wch->level]->ch = wch;
	   table[wch->level]->next = NULL;
              } else {
                   struct who_slot *tmp = table[wch->level];

	   for (;tmp->next != NULL;tmp = tmp->next); {
	          tmp->next = (struct who_slot *)malloc(sizeof(struct who_slot)); 
                   }
                   tmp->next->ch = wch;
	   tmp->next->next = NULL;
              }
        } 

         /* Now show matching chars... */

         buf[0] = '\0';
         output[0] = '\0';
         sprintf(buf, "\r\n{C--- Bounty ---{x\r\n");
         strcat(output, buf);
   
         for ( counter=MAX_LEVEL;counter>=1;counter--) {
                 CHAR_DATA *wch;
                 struct who_slot *tmp=table[counter];

                 if (tmp == NULL) continue;
                 for ( ;tmp != NULL;tmp = tmp->next) {	
                        wch = tmp->ch;
                        if (wch == NULL) {
	               log_string ("Got null table->ch, argh.");
	               continue;
	        }
                        if (IS_IMMORTAL(wch)) {
                                continue;
                        }
                        if (wch->pcdata->bounty==0) {
                             continue;
                        }
	        strcpy (form, "%20s  %s%s (%ld)");
                        sprintf(fname, "{x[{Y%3d{x]                   ", wch->level);
                        fname[25] = '\0';
                        sprintf( buf, form, fname, IS_SET(wch->plr, PLR_AFK) ? "{r(AFK){x " : "", wch->name, wch->pcdata->bounty);
                        strcat (buf, "\r\n");

                        if ( (strlen(output) + strlen(buf)) > (8*MAX_STRING_LENGTH+1) ) {
                                page_to_char (output,ch);
                                output[0]='\0';
                        }
                        strcat(output,buf);
                   }

            }
            page_to_char( output, ch );
            return;
}



void do_bounty( CHAR_DATA *ch, char *argument ) {
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

        if ( !IS_SET(ch->in_room->room_flags, ROOM_BOUNTY_OFFICE) ) {
             send_to_char( "You can't do that here.\r\n", ch );
             return;
        }
 
        if (arg1[0] == '\0') {
            if (IS_NPC(ch)) return;
            do_bountylist(ch);

        } else if (!str_cmp(arg1, "bribe")) {
            if (IS_NPC(ch)) return;

            if (ch->pcdata->bounty <= 0) {
                send_to_char( "There is no bounty on you!\r\n", ch );
                return;
            }
               
            if (ch->gold[0] < 2*ch->pcdata->bounty) {
                send_to_char( "You don't have enough {rcopper{x!\r\n", ch );
                return;
            }

            sprintf_to_char(ch, "You pay the bounty hunters %d {rcopper{x to remove you from the list.\r\n",ch->pcdata->bounty);
            ch->gold[0] -= 2*ch->pcdata->bounty;
            ch->pcdata->bounty = 0;

        } else {

         if (arg2[0] == '\0') {
            if (IS_NPC(ch)) return;
            do_bountylist(ch);
            return;
         }

         if ( ( victim = get_char_world( ch, arg1 ) ) == NULL) {
           send_to_char( "They are currently not logged in!\r\n", ch );
           return;
        }
  
        if (ch==victim) {
           send_to_char( "You shouldn't do that!\r\n", ch );
           return;
        }
       
      if (IS_NPC(victim)) {
         send_to_char( "You cannot put a bounty on NPCs!\r\n", ch );
         return;
       }

        if (IS_IMMORTAL(victim)) {
         send_to_char( "On an Immortal?!\r\n", ch );
         return;
       }

        if (!IS_SET(victim->act, ACT_CRIMINAL)) {
         send_to_char( "This is no known criminal.\r\n", ch );
         return;
       }

       
        if (is_number( arg2 ) ) {
            long amount;
            amount   = atol(arg2);
             if (ch->gold[0] < amount) {
                send_to_char( "You don't have that much {rCopper{x!\r\n", ch );
                return;
             }

             ch->gold[0] -= amount;
             victim->pcdata->bounty +=amount;
             if (amount >20000) {
                    sprintf(buf, "The bounty on %s has been raised to a total of %ld {rCopper{x.\r\nGet it before someone else does...",victim->name, victim->pcdata->bounty );
                    make_note ("Observer", "Dylath-Leen Bounty Office", "all", "Bounty Announce", 2, buf);
             }
             sprintf_to_char(ch, "You have placed a %ld {rCopper{x bounty on %s{g.\r\n%s now has a bounty of %ld {rCopper{x.\r\n", amount,victim->name,victim->name,victim->pcdata->bounty );
             return;
        }
   }
   return;
}


void do_brainsuck( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *head;
    int ldif;
    bool ininv;

    if (str_cmp(race_array[ch->race].name,"mi-go")
    && str_cmp(race_array[ch->race].name,"mindflayer")) {
        send_to_char( "You can't do that.\r\n", ch );
        return;
      }
 
    one_argument( argument, arg );

    if ( arg[0] == '\0' )    {
	send_to_char( "Brainsuck whom?\r\n", ch );
	return;
    }

    ininv = TRUE;
    if (( victim = get_char_room( ch, arg ) ) == NULL )    {
          head = ch->carrying;
          while ( head != NULL
          && ( head->pIndexData->vnum != 12
          || head->wear_loc != -1)) {
               head = head->next_content;
          }

          if ( head == NULL ) {
               ininv = FALSE;
               head = ch->in_room->contents;

               while ( head != NULL
               &&  head->pIndexData->vnum != 12) {
                    head = head->next_content;
               }
          }

          if ( head == NULL ) {
	send_to_char( "They aren't here.\r\n", ch );
	return;
          }

          if (!IS_NPC(ch) ) {
               if ( ch->condition[COND_FOOD] >= COND_STUFFED ) gain_condition( ch, COND_FAT, 2 );
               gain_condition( ch, COND_FOOD, 300 );
          }

          if (ininv) {
                obj_from_char(head);
          } else {
	obj_from_room(head);
          }
          extract_obj(head);

          send_to_char( "You extract the brain.\r\n",ch);
          act("$n sucks the brain out of $p.",ch, head, NULL,TO_ROOM);

          return;
     }

     if (IS_NEWBIE(victim)) {
	send_to_char( "A Newbie brain is much too small!\r\n",ch);
	return;
     }

     if (ch==victim) {
        send_to_char( "Don't eat your own brain.\r\n", ch );
        return;
      }

      if ( victim->position  > POS_SLEEPING) {
          send_to_char( "Your victim won't like that.\r\n", ch );
          return;
      }

      if (IS_SET(victim->act, ACT_BRAINSUCKED)) {
        send_to_char( "Your victim's brain is damaged.\r\n", ch );
        return;
      }

      if (IS_AFFECTED(victim, AFF_POLY)) {
        send_to_char( "Your victim's mask confuses you - is that brain usable?\r\n", ch );
        return;
      }

      if (!IS_NPC(victim)) {
          ldif=(victim->level - ch ->level+10)/5;
          if (ldif<0) ldif=0;
          ch->perm_stat[STAT_INT] +=ldif;
          ch->perm_stat[STAT_INT] = URANGE(STAT_MINIMUM, ch->perm_stat[STAT_INT], STAT_MAXIMUM); 
          victim->perm_stat[STAT_INT] -= dice(2,5);
          victim->perm_stat[STAT_INT] = URANGE(STAT_MINIMUM, victim->perm_stat[STAT_INT], STAT_MAXIMUM); 
          SET_BIT(victim->act,ACT_BRAINSUCKED);
          send_to_char( "You suck out the brain.\r\n",ch);
          send_to_char( "You feel so calm now.\r\n",victim);
          act("$n sucks out $Ns brain.",ch,NULL,victim,TO_ROOM);
      } else {
          if (get_curr_stat (victim, STAT_INT) < 90) {
                send_to_char( "There isn't much brain to suck.\r\n",ch);
                return;
          }
          ldif=(victim->level - ch ->level+10)/5;
          if (ldif<0) ldif=0;
          ch->mana += ldif*20;
          if (ch->mana > ch->max_mana) ch->mana = ch->max_mana;
          victim->perm_stat[STAT_INT] = 12;
          SET_BIT(victim->act,ACT_BRAINSUCKED);
          send_to_char( "You suck out the brain.\r\n",ch);
          act("$n sucks out $Ns brain.",ch,NULL, victim,TO_ROOM);
      }
       return;
}


void do_bandage( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int skill;
    int roll;
    int chance;
    int tolimb;
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' )    {
	send_to_char( "Bandage whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
     }

     skill = get_skill(ch, gsn_bandage);

     if (skill < 1 )              {
	send_to_char ("You would not dare that.\r\n",ch);
	return;
     } 	

     if (ch->move < 60) {
	send_to_char ("You do not have enough movement.\r\n",ch);
	return;
     }

     WAIT_STATE(ch, skill_array[gsn_bandage].beats);
     roll = number_percent();
     chance=skill+(ch->level/4)-(victim->level/4);

      if (victim->hit<victim->max_hit/2) {
          chance -=5;
      }
      if (victim->hit<victim->max_hit/4) {
          chance -=10;
      }
       if (victim->hit<victim->max_hit/10) {
          chance -=15;
      }
       if (ch->hit<ch->max_hit/3) {
          chance -=10;
      }
       if (ch->move<ch->max_move/5) {
          chance -=10;
      }
      
     if (ch==victim) chance-=10;
               
      ch->move -= 60;
      
      if (chance<0) {
          check_improve(ch, gsn_bandage, FALSE, 1);
          if (ch!=victim) {
             send_to_char ("Ouch. That rather hurt your patient!\r\n",ch);
             send_to_char ("Aaargh. This is no real doctor!\r\n",victim);
          } else {
             send_to_char ("Ouch. Better get a real Doc!\r\n",ch);
          }
          victim->hit -=victim->max_hit/20;
          if (victim->hit<1) {
              victim->hit=1;
          }
          return;
       }
       if (chance<roll) {
          check_improve(ch, gsn_bandage, FALSE, 1);
          send_to_char ("You fail to help your patient.\r\n",ch);
       } else {
          check_improve(ch, gsn_bandage, TRUE, 1);
          if (ch!=victim) {
              send_to_char ("You carefully bandage your patient.\r\n",ch);
              sprintf(buf, "%s carefully bandages you.\r\n", ch->name );
              send_to_char (buf,victim);
          } else {
              send_to_char ("You carefully bandage yourself.\r\n",ch);
          }
          REMOVE_BIT(victim->form, FORM_BLEEDING);
           tolimb=number_range(0,5);
           if (ch->limb[tolimb]>0 && ch->limb[tolimb]<10) {
                   if (number_range(0,ch->limb[tolimb])/3 < 2) ch->limb[tolimb] -=1;
           }
           update_wounds(ch);
          victim->hit +=victim->max_hit*skill/800;
          if (victim->hit>victim->max_hit) {
              victim->hit=victim->max_hit;
          }
       }
       
       return;
}

void do_psychology( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int skill;
    int roll;
    int chance;

    one_argument( argument, arg );

    if ( arg[0] == '\0' ) {
	send_to_char( "Give therapy to whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL ) {
	send_to_char( "They aren't here.\r\n", ch );
	return;
     }

     if (ch==victim) {
        send_to_char( "Giving therapy to your imaginary fried is not considered as {rnormal{x.\r\n", ch );
        return;
      }

     skill = get_skill(ch, gsn_psychology);

     if (skill < 1 ) {
	send_to_char ("You would not dare that.\r\n",ch);
	return;
     }
 	
     if (ch->move < 250) {
	send_to_char ("You do not have enough movement.\r\n",ch);
	return;
      }

      if (get_sanity(ch) < get_sanity(victim)) {
          	send_to_char ("You doubt you can help.\r\n",ch);
	return;
      }

      if (get_sanity(victim) >60) {
          	send_to_char ("Your patient is completely normal.\r\n",ch);
	return;
      }

     WAIT_STATE(ch, skill_array[gsn_psychology].beats);
     roll = number_percent();
     chance=skill+(ch->level/4)-(victim->level/4)+get_sanity(victim)-32;
      if (ch->hit<ch->max_hit/4) {
          chance -=10;
      }
       if (ch->move<ch->max_move/6) {
          chance -=10;
      }
       if (get_sanity(ch) < 32) chance -=20;
       if (get_sanity(ch) >100) chance +=20;
       
      ch->move -= 250;
      
      if (chance<0) {
          check_improve(ch, gsn_psychology, FALSE, 1);
          send_to_char ("Your patient is so terribly convincing!\r\n",ch);
          send_to_char ("You explain the way the world is to your therapist!\r\n",victim);
          ch->sanity +=chance;
          return;
       }
       if (chance<roll) {
          check_improve(ch, gsn_psychology, FALSE, 1);
          send_to_char ("You fail to help your patient.\r\n",ch);
          ch->sanity -=3;
       } else {
          check_improve(ch, gsn_psychology, TRUE, 1);
          send_to_char ("You help your patient with some of his conflicts.\r\n",ch);
          sprintf(buf, "%s talks with you for a long while.\r\n", ch->name );
          send_to_char (buf,victim);
          victim->sanity +=(ch->sanity-victim->sanity)*skill/600;
       }
       
       return;
}

void do_bind (CHAR_DATA *ch, char *argument)
	{
                char arg[MAX_INPUT_LENGTH];
               	OBJ_DATA *obj;
                OBJ_DATA *aobj;
                CHAR_DATA *victim;
          

     if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) == NULL ) {
        send_to_char ("You have no bondage object in your hands.\r\n",ch);
        return;
     }  

     if (obj->item_type!=ITEM_BONDAGE) {
        send_to_char ("You have no bondage object in your hands.\r\n",ch);
        return;
     }  

    one_argument( argument, arg );

    if ( arg[0] == '\0' )  {
	send_to_char( "Bind whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL ) {
	send_to_char( "They aren't here.\r\n", ch );
	return;
     }

     if (ch==victim) {
        send_to_char( "You got not time for masochism now.\r\n", ch );
        return;
      }

      if (victim->position != POS_STUNNED) {
        send_to_char( "Your victim won't like that.\r\n", ch );
        return;
      }

      if ( ( aobj = get_eq_char( victim, WEAR_ARMS ) ) != NULL ) {
        unequip_char( victim, aobj );
      }  

      unequip_char( ch, obj );
      obj_from_char( obj );
      obj_to_char( obj, victim );
      equip_char( victim, obj, WEAR_ARMS );

      SET_BIT (victim->act, ACT_BOUND);

      send_to_char( "You bind your victim.\r\n",ch);
      send_to_char( "You've been bound.\r\n",victim);

      return;
}


void do_untie (CHAR_DATA *ch, char *argument)	{
               char arg[MAX_INPUT_LENGTH];
               	OBJ_DATA *aobj;
                CHAR_DATA *vic;
          
    one_argument( argument, arg );

    if ( arg[0] == '\0' )    {
	send_to_char( "Untie whom?\r\n", ch );
	return;
    }

    if ( ( vic = get_char_room( ch, arg ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
     }

     if (ch==vic) {
        send_to_char( "You are not bound.\r\n", ch );
        return;
      }

      if (vic->position != POS_STUNNED) {
        send_to_char( "No need to do that.\r\n", ch );
        return;
      }

      if ((aobj = get_eq_char( vic, WEAR_ARMS ) ) !=NULL ) {

        if (IS_SET(aobj->extra_flags, ITEM_NOREMOVE))    {
	act( "You can't remove $p.", ch, aobj, NULL, TO_CHAR );
	return;
        }

        unequip_char( vic, aobj );
        obj_from_char( aobj );
        obj_to_char( aobj, ch );

        REMOVE_BIT (vic->act, ACT_BOUND);

        if (vic->hit>0 && vic->move>0) {
               set_activity( vic, POS_STANDING, NULL, ACV_NONE, NULL);
        }

        send_to_char( "You untie your victim.\r\n",ch);
        send_to_char( "You've been freed.\r\n",vic);
        act("$n unties $N.",ch, NULL, vic,TO_ROOM);
     } else {
          send_to_char( "No need to do that.\r\n",ch);
     }

      return;
}


void do_rob( CHAR_DATA *ch, char *argument ) {
   char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *vic;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"from"))
	argument = one_argument(argument,arg2);

    if ( arg1[0] == '\0' )    {
	send_to_char( "Rob what?\r\n", ch );
	return;
    }

    if ( ( vic = get_char_room( ch, arg2 ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
     }

      if (vic->position != POS_STUNNED) {
        send_to_char( "Your victim won't let you.\r\n", ch );
        return;
      }

      if ( ( obj = get_obj_carry( vic, arg1 ) ) == NULL ) {
         send_to_char( "You don't find that item.\r\n", ch );
         return;
      }

      if (!can_see_obj( ch, obj )
      || obj->wear_loc != WEAR_NONE) {
         send_to_char( "You don't find that item.\r\n", ch );
         return;
      }

      if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) ) {
          send_to_char( "You can't carry that much weight.\r\n", ch );
          return;
      }

      obj_from_char( obj );
      obj_to_char( obj, ch );

      send_to_char ("You rob your victim.\r\n",ch);
      send_to_char ("Aargh! You've been robbed.\r\n",vic);
      act("$n gets $p from his helpless victim.",ch,obj,NULL,TO_ROOM);

       return;
}


void do_carry (CHAR_DATA *ch, char *argument) {
char arg[MAX_INPUT_LENGTH];
CHAR_DATA *victim;
          

    one_argument( argument, arg );

    if ( arg[0] == '\0' )  {
	send_to_char( "Carry whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL ) {
	send_to_char( "They aren't here.\r\n", ch );
	return;
     }

     if (ch==victim) {
        send_to_char( "You can't do that.\r\n", ch );
        return;
      }

      if ( victim->position  > POS_STUNNED) {
           send_to_char( "Your victim won't like that.\r\n", ch );
           return;
      }

      if ( ch->carry_weight + 150 > can_carry_w( ch ) ) {
            act( "$d: you can't carry that much weight.", ch, NULL, victim->name, TO_CHAR );
             return;
      }

      if ( victim->master != NULL) {
          if (ch != victim->master) {
               send_to_char( "Someone else is carrying your victim.\r\n",victim);
          }
          return;
       }

      ch->carry_weight +=150;
      add_follower( victim, ch );
      send_to_char( "You grab your victim.\r\n",ch);
      send_to_char( "You feel carried away.\r\n",victim);
      act("$n throws $N over the shoulder.",ch,NULL,victim,TO_ROOM);
      return;
}

void do_intimidate( CHAR_DATA *ch, char *argument ) {
   char arg1[MAX_INPUT_LENGTH];
   CHAR_DATA *vic;

    argument = one_argument( argument, arg1 );

     if (get_skill(ch, gsn_intimidate)<1){
	send_to_char( "You don't know how to do that.\r\n", ch );
	return;
     }

    if ( arg1[0] == '\0' )    {
	send_to_char( "Intimidate whom?\r\n", ch );
	return;
    }

    if ( ( vic = get_char_room( ch, arg1 ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
     }

     if (ch->move<30){
	send_to_char( "You are to exhausted.\r\n", ch );
	return;
     }

     ch->move -=30;
     
      do_emote(ch,"growls intimidatingly!");
      send_to_char ("You really feel dangerous.\r\n",ch);
    
      if (ch->level+get_skill(ch, gsn_intimidate) >vic->level+number_percent()+20) {
          do_emote(vic,"gets pale and begins to shiver!");
          send_to_char ("Run before you get killed.\r\n",vic);
          check_improve(ch, gsn_intimidate, TRUE, 2);
          if (IS_NPC(vic)) {
              if (vic->pIndexData->pShop == NULL) do_flee( vic, "");
          } else {
              do_flee( vic, "");
          }
      } else {
          do_emote(vic,"laughs at that pathetic demonstration of power!");
          send_to_char ("That can't frighten you.\r\n",vic);
          if (IS_NPC(vic)) {
               if (number_percent()<25) {
                    if ( !IS_AFFECTED( vic, AFF_CHARM )
                    && vic->position != POS_FIGHTING 
                    && vic->pIndexData->pShop == NULL
                    && !IS_SET(vic->act, ACT_PRACTICE) 
                    && !IS_SET(vic->act, ACT_NOPURGE) 
                    && !IS_SET(vic->act, ACT_IS_ALIENIST) 
                    && !IS_SET(vic->act, ACT_IS_HEALER)) {
                                 multi_hit( vic, ch, TYPE_UNDEFINED );
                    }
               }
          }
      }
       return;
}


void do_cut (CHAR_DATA *ch, char *argument)	{
char arg[MAX_INPUT_LENGTH];
char buf [MAX_INPUT_LENGTH];
CHAR_DATA *victim;
OBJ_DATA  *obj;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )  {
	send_to_char( "Cut whose hair?\r\n", ch );
	return;
    }

    if ( !str_cmp(arg,"hair") )  argument = one_argument( argument, arg );

    if ( ( victim = get_char_room( ch, arg ) ) == NULL ) {
	send_to_char( "They aren't here.\r\n", ch );
	return;
     }

     if (ch==victim) {
        send_to_char( "No way!\r\n", ch );
        return;
      }

     if (IS_NPC(victim)) {
        send_to_char( "You'll never see him again.\r\n", ch );
        return;
      }

     if (IS_IMMORTAL(victim)) {
        send_to_char( "Don't be ridiculous!.\r\n", ch );
        return;
      }

      if ( victim->position  > POS_SLEEPING) {
          send_to_char( "Your victim won't like that.\r\n", ch );
          return;
      }

      obj = create_object( get_obj_index( OBJ_VNUM_HAIR ), 0 );

      sprintf(buf,"hair streak %s",victim->name);
      free_string(obj->name);
      obj->name=str_dup(buf);
      sprintf(buf,"a streak of %ss hair",victim->name);
      free_string(obj->short_descr);
      obj->short_descr=str_dup(buf);

      obj_to_char(obj,ch);
      send_to_char( "You feel some strange sensation on your head.\r\n",victim);
      act("$n cuts $Ns hair.",ch,NULL,victim,TO_ROOM);
      act("You cut $Ns hair.",ch,NULL,victim,TO_CHAR);
      return;
}


void do_voodoo( CHAR_DATA *ch, char *argument ) {
    CHAR_DATA *victim;
    OBJ_DATA  *doll;
    char      arg1 [MAX_INPUT_LENGTH];
    char      arg2 [MAX_INPUT_LENGTH];
    char       buf [MAX_INPUT_LENGTH];
    char     part1 [MAX_INPUT_LENGTH];
    char     part2 [MAX_INPUT_LENGTH];
    int dam;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0')    {
                send_to_char( "Possible voodoo commands: stab / twist / tear\r\n", ch );
	return;
    }

    if ( arg2[0] == '\0')    {
	send_to_char( "Do voodoo on whom?\r\n", ch );
	return;
    }

    if ( ( doll = get_eq_char( ch, WEAR_HOLD ) ) == NULL )    {
	send_to_char( "You are not holding a voodoo doll.\r\n", ch );
	return;
    }

    if (doll->item_type !=ITEM_DOLL) {
	send_to_char( "You are not holding a voodoo doll.\r\n", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg2 ) ) == NULL )    {
	send_to_char( "They are not here.\r\n", ch );
	return;
    }

    if ( IS_NPC(victim) )    {
	send_to_char( "That wouldn't help.\r\n", ch );
	return;
    }

    sprintf(part2,doll->name);
    sprintf(part1,"%s voodoo doll",victim->name);

    if ( str_cmp(part1,part2) )    {
	sprintf(buf,"But you are holding %s, not %s!\r\n",part2,part1);
	send_to_char( buf, ch );
	return;
    }

    if ( arg2[0] == '\0')    {
	send_to_char( "Possible voodoo commands: stab/tear\r\n", ch );
    }    else if ( !str_cmp(arg1, "stab") )    {
	WAIT_STATE(ch,12);
	act("You stab a pin through $p.", ch, doll, NULL, TO_CHAR); 
 	act("$n stabs a pin through $p.", ch, doll, NULL, TO_ROOM);
	act("You feel an agonising pain in your chest!", victim, NULL, NULL, TO_CHAR);
 	act("$n clutches $s chest in agony!", victim, NULL, NULL, TO_ROOM);
                if (victim->move > victim->max_move/3) victim->move -=5;              

    }    else if ( !str_cmp(arg1, "tear") )    {
	WAIT_STATE(ch,12);
	act("You tear $p.", ch, doll, NULL, TO_CHAR);
 	act("$n gleefully tears $p.", ch, doll, NULL, TO_ROOM);
	if (victim->position == POS_FIGHTING) stop_fighting(victim, TRUE);
	act("A strange force picks you up and tears you apart!", victim, NULL, NULL, TO_CHAR);
 	act("$n is torn apart by a strange force.", victim, NULL, NULL, TO_ROOM);
	victim->position = POS_RESTING;
                dam = ch->level *doll->value[0]/10;
	victim->hit -= dam;
	obj_from_char(doll);
                extract_obj(doll);
	update_pos(victim);
                kill_msg(victim);

    }    else if ( !str_cmp(arg1, "twist") )    {
	WAIT_STATE(ch,12);
	act("You twist $p.", ch, doll, NULL, TO_CHAR);
 	act("$n giggles while he slowly twists $p.", ch, doll, NULL, TO_ROOM);
	if (victim->position == POS_FIGHTING) stop_fighting(victim, TRUE);
	act("A strange force brutally twists your limbs!", victim, NULL, NULL, TO_CHAR);
 	act("$n is beint twisted by a strange force.", victim, NULL, NULL, TO_ROOM);
	victim->position = POS_RESTING;
                dam = ch->level *doll->value[0]/10;
	victim->move -= dam;
                doll->condition -=number_percent();
                if (doll->condition < 1) {
      	         act("You tear $p apart.", ch, doll, NULL, TO_CHAR);
   	         obj_from_char(doll);
                         extract_obj(doll);
                }
	update_pos(victim);
                kill_msg(victim);

    }    else    {
          send_to_char( "Possible voodoo commands: stab / twist / tear\r\n", ch );
    }
    return;
}


void do_sermonize (CHAR_DATA *ch, char *argument)	{
                char arg[MAX_INPUT_LENGTH];
                char buf [MAX_INPUT_LENGTH];
                CHAR_DATA *victim;
                int skill;
                int transal;
                int levd;

    argument = one_argument( argument, arg );

    skill = get_skill(ch, gsn_theology)-30;
    if (skill<1) {
	send_to_char( "Your theological knowledge is insufficuient.\r\n", ch );
	return;
     }

    if ( arg[0] == '\0' )  {
	send_to_char( "Alone?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL ) {
	send_to_char( "They aren't here.\r\n", ch );
	return;
     }

     if (ch==victim) {
        send_to_char( "Better pray to your god!\r\n", ch );
        return;
      }

      if (IS_AFFECTED(victim, AFF_BERSERK)
      || victim->position < POS_RESTING) {
              sprintf(buf, "%s won't pay attention to you.\r\n",capitalize(victim->short_descr));
              send_to_char(buf, ch);
              return;
      } 

       if (victim->master != ch) {
            send_to_char("You can only sermonize people that are grouped to you.\r\n", ch);
            return;
       }

       if (ch->move<50) {
          send_to_char( "You are too exhausted!\r\n", ch );
          return;
       }

       if (ch->mana<50) {
          send_to_char( "You doubt you're very convincing!\r\n", ch );
          return;
       }

       ch->mana -=50;
       ch->move -=50;

            if (ch->alignment>300) {
                send_to_char( "{mYou hold e sermon about the sake of being {Mgood{m.{x\r\n", ch );
                act("{m$n holds a sermon about the sake of being {Mgood{m.{x",ch,NULL,NULL,TO_ROOM);
            } else if (ch->alignment>-300) {
                send_to_char( "{mYou hold e sermon about the sake of being {Mneutral{m.{x\r\n", ch );
                act("{m$n holds a sermon about the sake of being {Mneutral{m.{x",ch,NULL,NULL,TO_ROOM);
            } else {
                send_to_char( "{mYou hold e sermon about the sake of being {Mevil{m.{x\r\n", ch );
                act("{m$n holds a sermon about the sake of being {Mevil{m.{x",ch,NULL,NULL,TO_ROOM);
            }
        levd=victim->level - ch->level + 20;
        if (levd<0 ) levd=0;
        transal = (ch->alignment - victim->alignment)/10;

        if (!check_skill(ch, gsn_theology, levd)) {
                send_to_char( "But in vain...\r\n", ch );
                act("But who cares...",ch,NULL,NULL,TO_ROOM);
                ch->alignment -=transal/2;
        	check_improve(ch, gsn_theology, FALSE, 1);
         } else {
                victim->alignment +=transal;
                send_to_char( "And you're really convincing!.\r\n", ch );
                act("And you see truth in what he's saying.",ch,NULL,NULL,TO_ROOM);
        	check_improve(ch, gsn_theology, TRUE, 1);
         }

         return;
}


void do_tame( CHAR_DATA *ch, char *argument ) {
char arg[MAX_INPUT_LENGTH];
char buf[MAX_STRING_LENGTH];
CHAR_DATA *victim;
int skill;
int roll;
int chance;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )    {
	send_to_char( "Tame whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
     }

     if (ch==victim) {
        send_to_char( "You feel very tame.\r\n", ch );
        return;
      }

     if (ch->pet !=NULL) {
        send_to_char( "You already got a pet.\r\n", ch );
        return;
      }

    if (IS_SET(victim->act,ACT_AGGRESSIVE)
    || IS_SET(victim->imm_flags, IMM_CHARM)
    || IS_SET(victim->act, ACT_UNDEAD)
    || IS_SET(victim->form, FORM_MACHINE)
    || !str_cmp(race_array[victim->race].name,"undead")
    || !str_cmp(race_array[victim->race].name,"machine")
    || !IS_NPC(victim)) {
	send_to_char ("You can't tame that.\r\n",ch);
	return;
    }

    if (IS_SET(victim->form,FORM_SENTIENT)) {     
        send_to_char( "You won't be able to tame that.\r\n", ch );
        return;
      }

     skill = get_skill(ch, gsn_tame);

     if (skill < 1 ) {
	send_to_char ("You would not dare that.\r\n",ch);
	return;
     } 	

     if (ch->move < 200) {
	send_to_char ("You are too exhausted.\r\n",ch);
	return;
     }

     WAIT_STATE(ch, skill_array[gsn_tame].beats);

     roll = number_percent();
     chance= skill + get_curr_stat(ch, STAT_CHA) + (ch->level/4) - 50 - (victim->level/3);

      if (ch->hit<ch->max_hit/4) chance -=10;
      if (ch->move<ch->max_move/6) chance -=10;
       
      ch->move -= 200;
      
      if (chance<roll) {
          check_improve(ch, gsn_tame, FALSE, 1);
          send_to_char ("{rOh-Oh...{x\r\n",ch);
          SET_BIT (victim->act, ACT_AGGRESSIVE);
          victim->move=victim->max_move;
          victim->position = POS_STANDING;
          return;
       } else {
          check_improve(ch, gsn_tame, TRUE, 1);
          sprintf(buf, "You tame %s.\r\n", victim->name );
          send_to_char (buf,victim);
          SET_BIT(victim->act, ACT_FOLLOWER);
          SET_BIT(victim->affected_by, AFF_CHARM);
          SET_BIT(ch->plr, PLR_BOUGHT_PET);
          SET_BIT(victim->act, ACT_PET);
          victim->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
          add_follower( victim, ch );
          victim->leader = ch;
          act( "$n now follows $N!", victim, NULL, ch, TO_ROOM );
          victim->move=victim->max_move;
          victim->position = POS_STANDING;
       }
       
       return;
}

 void do_marry( CHAR_DATA *ch, char *argument ) {
char buf[MAX_STRING_LENGTH];
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
CHAR_DATA *vict1;
CHAR_DATA *vict2;
OBJ_INDEX_DATA *obj_index;
OBJ_DATA *ring;
OBJ_DATA *band;
 
     argument = one_argument( argument, arg1 );
     argument = one_argument( argument, arg2 );
     
     if (get_skill(ch, gsn_theology) < 60) {
 	send_to_char( "You can't do that.\r\n", ch );
 	return;
     }

     if (!IS_RAFFECTED(ch->in_room, RAFF_HOLY)
     && ch->in_room->sector_type != SECT_HOLY) {
 	send_to_char( "Only on holy ground.\r\n", ch );
 	return;
     }

     if ( arg1[0] == '\0' || arg2[0] == '\0' )     {
 	send_to_char( "Syntax:  marry <player> <player>\r\n", ch );
 	return;
     }
 
     if ( ( vict1 = get_char_room( ch, arg1 ) ) == NULL )     {
 	sprintf( buf, "%s is not here.\r\n", capitalize(arg1) );
 	send_to_char( buf, ch );
 	return;
     }
 
     if ( ( vict2 = get_char_room( ch, arg2 ) ) == NULL )     {
 	sprintf( buf, "%s is not here.\r\n", capitalize(arg2) );
 	send_to_char( buf, ch );
 	return;
     }
 
     if ( IS_NPC(vict1) || IS_NPC(vict2) )    {
 	send_to_char( "You cannot marry a player to a mob, silly!\r\n", ch );
 	return;
     }
 
     if ( vict1 == vict2 )     {
 	send_to_char( "You cannot do that.\r\n", ch );
 	return;
     }
 
     if (vict1->master != vict2
     && vict2->master != vict1) {
         send_to_char("They need to be grouped.\r\n", ch);
         return;
     }

     if (str_cmp(vict1->pcdata->spouse,""))     {
 	sprintf( buf, "%s is already married to %s!\r\n", vict1->name, vict1->pcdata->spouse );
 	send_to_char( buf, ch );
 	return;
     }
 
     if (str_cmp(vict2->pcdata->spouse,""))     {
 	sprintf( buf, "%s is already married to %s!\r\n", vict2->name, vict2->pcdata->spouse );
 	send_to_char( buf, ch );
 	return;
     }
 
     if (vict1->sex == vict2->sex) {
 	send_to_char( "Think again! Not in the 1920s!!\r\n", ch );
 	return;
     }
 
     free_string(vict1->pcdata->spouse);
     free_string(vict2->pcdata->spouse);
     vict1->pcdata->spouse = str_dup(vict2->name);
     vict2->pcdata->spouse = str_dup(vict1->name);
 
     if ( ( ( obj_index = get_obj_index(OBJ_VNUM_WEDDING_RING) ) != NULL )
     && ( ( ring = create_object( obj_index, 25 ) ) != NULL ) )
             obj_to_char( ring, ch );
 
     if ( ( ( obj_index = get_obj_index(OBJ_VNUM_WEDDING_BAND) ) != NULL )
     && ( ( band = create_object( obj_index, 25 ) ) != NULL ) )
             obj_to_char( band, ch );
 
     sprintf( buf, "{M%s and %s are now married.{x\r\n", vict1->name, vict2->name );
     do_echo( ch, buf );
     return;
 }


void do_camp( CHAR_DATA *ch, char *argument ) {
OBJ_DATA *obj;
int move_cost;

     if (get_skill(ch, gsn_survival) <= 0) {
 	send_to_char( "You don't know how.\r\n", ch );
 	return;
     }

     if (IS_SET(ch->in_room->room_flags, ROOM_INDOORS)
     || ch->in_room->sector_type == SECT_INSIDE
     || ch->in_room->sector_type == SECT_CITY
     || ch->in_room->sector_type == SECT_UNDERGROUND
     || ch->in_room->sector_type == SECT_WATER_SWIM
     || ch->in_room->sector_type == SECT_WATER_NOSWIM
     || ch->in_room->sector_type == SECT_UNDERWATER
     || ch->in_room->sector_type == SECT_AIR
     || ch->in_room->sector_type == SECT_SPACE) {
 	send_to_char( "You can't do that here.\r\n", ch );
 	return;
     }

     for (obj = ch->in_room->contents; obj; obj =obj->next_content) {
           if (obj->item_type == ITEM_DECORATION) break;
     }

     if (obj) {
 	send_to_char( "You can't do that here.\r\n", ch );
 	return;
     }

     move_cost = 50 + UMAX(10 - get_skill(ch, gsn_survival), 0) *50;
     move_cost += UMAX(50 - get_skill(ch, gsn_survival), 0) *3;
     move_cost += UMAX(150 - get_skill(ch, gsn_survival), 0);

     if (move_cost > ch->move) {
 	send_to_char( "You don't find enough dry wood.\r\n", ch );
 	return;
     }

     ch->move -= move_cost;
     WAIT_STATE(ch, 24);

     obj = create_object(get_obj_index(OBJ_CAMP), 1);
     obj->timer = number_range(8, 16);
     obj->value[0] = number_range(1, ch->level);
     obj->value[1] = number_range(1, ch->level);
     obj_to_room(obj, ch->in_room);

     send_to_char( "{mYou build a big campfire.{x\r\n", ch );
     act("{m$n builds a big campfire.{x",ch, NULL, NULL, TO_ROOM);
     return;
}



/* Weres*/

void do_were (CHAR_DATA *ch, char *argument)	{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	
                if (!IS_SET(ch->act,ACT_WERE) ) {
                       send_to_char ("You are no Were!\r\n",ch);
                       return;
                 }                

                argument=one_argument(argument, arg1);
	argument=one_argument(argument, arg2);
                argument=one_argument(argument, arg3);
	
          if (arg1[0]=='\0') { 
               send_to_char ("\r\nWere commands:\r\n",ch);
               send_to_char ("\r\n",ch);
               send_to_char ("  WERE change\r\n",ch);
               send_to_char ("  WERE rage\r\n",ch);
               send_to_char ("  WERE growl\r\n",ch);
               return;
           }

           if (!str_cmp(arg1, "change"   )) {
                were_change(ch);
            } else if (!str_cmp(arg1, "rage"     )) {
                were_rage(ch);
            } else if (!str_cmp(arg1, "growl"     )) {
                were_growl(ch);
          } else {
              send_to_char ("No such were command.\r\n",ch);
              return;
            }

           return;
           }


void were_growl( CHAR_DATA *ch) {
     CHAR_DATA *victim;
     CHAR_DATA *v_next;

     if (ch->move < 50
     || ch->mana < 200) {
	send_to_char( "Your are too exhausted.\r\n", ch );
	return;
     }

     ch->mana -= 200;
     ch->move -= 50;
     WAIT_STATE(ch, 12);

     if (!IS_AFFECTED(ch, AFF_POLY)) {
          send_to_char( "Not in this form!\r\n", ch );
          return;
     }

    act("You growl menacingly.",ch, NULL, NULL, TO_CHAR);

    for ( victim = char_list; victim != NULL; victim = v_next )    {
         v_next = victim->next;

         if (victim->in_room == NULL ) continue;
         if (IS_NEWBIE(victim)) {
              if (number_percent() < 15) do_say(victim, "Ah, you're not real...");
              continue;
         }
         if (IS_NPC(victim)
         || ch == victim) continue;
         if (victim->in_room == ch->in_room) {
                act("$n begins to growl.",ch, NULL, NULL, TO_ROOM);
                  if (!check_sanity(victim, UMIN(ch->level, 100)))  {
                        send_to_char("{mYou panic...{x\r\n", victim);
                        do_flee(victim, "");
                  }
                  continue;
         }

         if (victim->in_room->area == ch->in_room->area) {
                sprintf_to_char(victim, "You hear the howling of a %s.", race_array[ch->race].name);
         }
    }
    return;
}

void were_change( CHAR_DATA *ch) {
     OBJ_DATA *aobj;
    AFFECT_DATA *paf;

     if (ch->move<200) {
	send_to_char( "Your are too exhausted.\r\n", ch );
	return;
     }

     ch->move -=200;
      WAIT_STATE(ch, 32);

     if (!IS_AFFECTED(ch, AFF_POLY)) {
          if ( is_affected(ch, gafn_frenzy) 
          || IS_AFFECTED(ch, AFF_BERSERK) ) {
                send_to_char( "Your are much too angry!\r\n", ch );
          } else {
          if ( ( aobj = get_eq_char( ch, WEAR_WIELD ) ) != NULL ) {
               unequip_char( ch, aobj );
           }  
           if ( ( aobj = get_eq_char( ch, WEAR_WIELD2 ) ) != NULL ) {
               unequip_char( ch, aobj );
           }  
           if ( ( aobj = get_eq_char( ch, WEAR_SHIELD ) ) != NULL ) {
               unequip_char( ch, aobj );
           }  

          SET_BIT( ch->affected_by, AFF_POLY );
          if (str_cmp(race_array[ch->race].name, race_array[ch->were_type].name)) {
             send_to_char( "You change into your animal form!\r\n", ch );
             act("$n changes into an animal.",ch,NULL,NULL,TO_ROOM);
  
              free_string(ch->description_orig);
              free_string(ch->short_descr_orig);
              free_string(ch->long_descr_orig);
              ch->description_orig = str_dup (ch->description);
              ch->short_descr_orig = str_dup (ch->short_descr);
              ch->long_descr_orig  = str_dup (ch->long_descr);
              ch->race_orig=ch->race;

               /*apply poly descriptions*/
               free_string(ch->description);
               free_string(ch->short_descr);
               free_string(ch->long_descr);

               switch(ch->were_type) {
                     default:
                        ch->description = str_dup("A large hairy wolf is here.\n");
                        ch->race = 31;
                        ch->were_type = 31;
                        ch->short_descr = str_dup ("Wolf");
                        ch->long_descr  = str_dup ("A large hairy wolf is here.\n");
                        free_string(ch->poly_name);
                        ch->poly_name = str_dup("wolf");
                        break;
        
                     case 31:
                        ch->description = str_dup("A large hairy wolf is here.\n");
                        ch->race = ch->were_type;
                        ch->short_descr = str_dup ("Wolf");
                        ch->long_descr  = str_dup ("A large hairy wolf is here.\n");
                        free_string(ch->poly_name);
                        ch->poly_name = str_dup("wolf");
                        break;

                     case 12:
                        ch->description = str_dup("A deep black panther is here.\n");
                        ch->race = ch->were_type;
                        ch->short_descr = str_dup ("Panther");
                        ch->long_descr  = str_dup ("A deep black panther is here.\n");
                        free_string(ch->poly_name);
                        ch->poly_name = str_dup("panther");
                        break;

                     case 17:
                        ch->description = str_dup("A swift grey fox is here.\n");
                        ch->race = ch->were_type;
                        ch->short_descr = str_dup ("Fox");
                        ch->long_descr  = str_dup ("A swift grey dox is here.\n");
                        free_string(ch->poly_name);
                        ch->poly_name = str_dup("fox");
                        break;
               }

               /*Wolf Affects*/
   
               paf             =   new_affect();
               paf->location   =   APPLY_STR;
               paf->skill	    =   0;
               paf->modifier   =   ch->level/15;
               paf->type	    =	SKILL_UNDEFINED;
               paf->afn        =   0;
               paf->duration   =   -1;
               paf->bitvector  =   AFF_HASTE;
               affect_modify( ch, paf, TRUE ); 

               paf             =   new_affect();
               paf->location   =   APPLY_INT;
               paf->skill	    =   0;
               paf->modifier   =   -ch->level/15;
               paf->type	    =	SKILL_UNDEFINED;
               paf->afn        =   0;
               paf->duration   =   -1;
               paf->bitvector  =   AFF_MELD;
               affect_modify( ch, paf, TRUE ); 

            }
            }
      } else {
          if (weather_info.moon == MOON_FULL) {
                send_to_char( "You feel trapped in this form!\r\n", ch );
          } else {
                send_to_char( "You change back to your normal form!\r\n", ch );
                act("$n changes back to his normal form.",ch,NULL,NULL,TO_ROOM);
               REMOVE_BIT( ch->affected_by, AFF_POLY );
               if (ch->race_orig!=0 ) {
                free_string(ch->description);
                free_string(ch->short_descr);
                free_string(ch->long_descr);
                free_string(ch->poly_name);
           
                ch->description = str_dup (ch->description_orig);
                ch->short_descr = str_dup (ch->short_descr_orig); 
                ch->long_descr  = str_dup (ch->long_descr_orig);
                ch->poly_name	= str_dup ("");
                ch->race=ch->race_orig;
                ch->race_orig = 0;

               /*Wolf affects*/
   
               paf             =   new_affect();
               paf->location   =   APPLY_STR;
               paf->skill	    =   0;
               paf->modifier   =   ch->level/15;
               paf->type	    =	SKILL_UNDEFINED;
               paf->afn        =   0;
               paf->duration   =   -1;
               paf->bitvector  =   AFF_HASTE;
               affect_modify( ch, paf, FALSE ); 

               paf             =   new_affect();
               paf->location   =   APPLY_INT;
               paf->skill	    =   0;
               paf->modifier   =   -ch->level/15;
               paf->type	    =	SKILL_UNDEFINED;
               paf->afn        =   0;
               paf->duration   =   -1;
               paf->bitvector  =   AFF_MELD;
               affect_modify( ch, paf, FALSE ); 
               }
           }
       }
      return;
   }


void were_rage( CHAR_DATA *ch) {
    AFFECT_DATA af;

    if (!IS_AFFECTED(ch, AFF_POLY) ) {
	send_to_char( "Not in this form.\r\n", ch );
	return;
     }

    if ( is_affected(ch, gafn_frenzy) 
    || IS_AFFECTED(ch, AFF_BERSERK) ) {
           send_to_char("You are already raging.\r\n",ch);
           return;
    }

     if (ch->move<50) {
	send_to_char( "Your are too exhausted.\r\n", ch );
	return;
     }

     ch->move -=50;

    if ( is_affected(ch, gafn_calm) ) {
         send_to_char("Why don't you just relax for a while?\r\n",ch);
         return;
    }

    af.type 	 = SKILL_UNDEFINED;
    af.afn	 = gafn_frenzy;
    af.level	 = ch->level;
    af.duration	 = ch->level;
    af.modifier  = ch->level / 3;
    af.bitvector = 0;

    af.location  = APPLY_HITROLL;
    affect_to_char(ch,&af);

    af.location  = APPLY_DAMROLL;
    affect_to_char(ch,&af);

    send_to_char("You are a raging animal!\r\n",ch);
    act("$n gets a wild look in $s eyes!",ch,NULL,NULL,TO_ROOM);
}




/*Vampires*/

void do_vampire (CHAR_DATA *ch, char *argument) {
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
char arg3[MAX_INPUT_LENGTH];
	
                if (!ch->pcdata) return;

                if (!IS_SET(ch->act,ACT_VAMPIRE) ) {
                       send_to_char ("You are no Vampire!\r\n",ch);
                       return;
                 }                

                argument=one_argument(argument, arg1);
	argument=one_argument(argument, arg2);
                argument=one_argument(argument, arg3);
	
          if (arg1[0]=='\0') { 
               send_to_char ("\r\nVampire commands:\r\n",ch);
               send_to_char ("\r\n",ch);
               send_to_char ("  VAMPIRE suck <victim>\r\n",ch);
               send_to_char ("  VAMPIRE embrace <victim>\r\n",ch);
               send_to_char ("\r\n",ch);
               send_to_char ("  VAMPIRE mesmerize <victim> <command>\r\n",ch);
               send_to_char ("  VAMPIRE strength <blood>\r\n",ch);
               send_to_char ("  VAMPIRE celerity <blood>\r\n",ch);
               send_to_char ("  VAMPIRE fortitude <blood>\r\n",ch);
               send_to_char ("  VAMPIRE majesty <blood>\r\n",ch);
               send_to_char ("  VAMPIRE heal <blood>\r\n",ch);
               send_to_char ("  VAMPIRE darkness <blood>\r\n",ch);
               send_to_char ("  VAMPIRE mistform <blood>\r\n",ch);
               send_to_char ("  VAMPIRE levitate <blood>\r\n",ch);
               return;
           }

           if (!str_cmp(arg1, "suck"   )) {
                vampire_suck(ch,arg2);
            } else if (!str_cmp(arg1, "embrace"     )) {
                vampire_embrace(ch,arg2);
            } else if (!str_cmp(arg1, "strength"     )) {
                vampire_strength(ch,arg2);
            } else if (!str_cmp(arg1, "celerity"     )) {
                vampire_celerity(ch,arg2);
          } else if (!str_cmp(arg1, "fortitude"     )) {
                vampire_fortitude(ch,arg2);
          } else if (!str_cmp(arg1, "heal"     )) {
                vampire_heal(ch,arg2);
          } else if (!str_cmp(arg1, "darkness"     )) {
                vampire_dark(ch,arg2);
          } else if (!str_cmp(arg1, "majesty"     )) {
                vampire_majesty(ch,arg2);
          } else if (!str_cmp(arg1, "mistform"     )) {
                vampire_mist(ch,arg2);
          } else if (!str_cmp(arg1, "levitate"     )) {
                vampire_levitate(ch,arg2);
          } else if (!str_cmp(arg1, "mesmerize"     )) {
                CHAR_DATA *victim;

                if (arg2[0]=='\0') { 
	     send_to_char( "Who do you want to mesmerize.\r\n", ch );
	     return;
                }

                if ((victim = get_char_room(ch, arg2)) == NULL) {
	     send_to_char( "They aren't here.\r\n", ch );
	     return;
                }
                vampire_mesmerize(ch,victim, arg3);
          } else {
              send_to_char ("No such vampire command.\r\n",ch);
              return;
            }

           return;
           }


void vampire_suck( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
        
    one_argument( argument, arg );

    if ( arg[0] == '\0' )    {
	send_to_char( "Bite whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
     }

      if ( victim->position  > POS_SLEEPING) {
	send_to_char( "Your victim wont let you.\r\n", ch );
	return;
     }

      if (IS_SET(victim->act,ACT_VAMPIRE) ) {
                 send_to_char ("Remember! Vampires got no blood!\r\n",ch);
                  return;
     }                

     if (ch->move<30) {
	send_to_char( "Your are too exhausted.\r\n", ch );
	return;
     }

     ch->move -=30;
      WAIT_STATE(ch, 32);

      ch->pcdata->blood +=victim->hit/50;
      victim->hit=1;
      if (ch->pcdata->blood >ch->level) ch->pcdata->blood=ch->level;
      
      send_to_char ("You drink your victims {rblood{x!\r\n",ch);
      send_to_char ("The evil vampire drinks all your {rblood{x!\r\n",victim);
      
      if (ch->pcdata->blood >0
      && is_affected(ch, gafn_frenzy)) {
          if (check_dispel(255,ch, gafn_frenzy)) {
               send_to_char ("You really needed that drink...\r\n",ch);
               REMOVE_BIT(ch->parts, PART_FANGS);
          }
      }

       return;
   }


void vampire_mesmerize( CHAR_DATA *ch, CHAR_DATA *victim, char *argument) {
      char arg[MAX_INPUT_LENGTH];
      int cost;     

      if ( argument[0] == '\0' )  {
	send_to_char( "What will you command your victim?\r\n", ch );
	return;
      }

      one_argument(argument, arg);
      if (str_cmp(argument, arg)) {
	send_to_char( "Command too long.\r\n", ch );
	return;
      }

      send_to_char("You focus your thoughts...\r\n", ch);
      act("$n tries to mesmerize $N!", ch, NULL, victim, TO_ROOM);

      WAIT_STATE(ch, 12);

      cost = victim->level * get_curr_stat (victim, STAT_INT) /  get_curr_stat (ch, STAT_INT);
      cost = cost * number_range(17, 23) / 20;
      if (IS_SET(victim->act, ACT_UNDEAD)) cost = (cost * 15)/10;

      if (cost > ch->pcdata->blood) {
           ch->pcdata->blood = 1;
           send_to_char("... and fail.\r\n", ch);
           return;
      } else {
           ch->pcdata->blood -= cost;
           do_say(ch, arg);
           interpret(victim, arg);
      }
      
      return;
}


void vampire_embrace( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    long loss;
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' )    {
	send_to_char( "Embrace whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
     }

      if (IS_NPC(victim)) {
                 send_to_char ("They are not worthy!\r\n",ch);
                  return;
     }                

      if (IS_SET(victim->act,ACT_VAMPIRE)
      || IS_SET(victim->act,ACT_UNDEAD)) {
                 send_to_char ("That's already an undead!\r\n",ch);
                  return;
     }                

    if (!IS_IMMORTAL(ch)) {
       if (ch->pcdata->blood<victim->level*2) {
	send_to_char( "You don't have enough blood to do so.\r\n", ch );
	return;
       }

       if (victim->level<15) {
	send_to_char( "Your victim is not worthy to become a Vampire.\r\n", ch );
	return;
        }

    }

    if ( victim->position  != POS_STUNNED  ) {
	send_to_char( "Your victim wont let you.\r\n", ch );
	return;
     }

     if (ch->move<300) {
	send_to_char( "Your are too exhausted.\r\n", ch );
	return;
     }

     ch->move -=300;
     ch->pcdata->blood -=victim->level*2;
     ch->pcdata->blood = UMAX(0, ch->pcdata->blood);
      WAIT_STATE(ch, 64);

      loss = victim->exp;
      gain_exp(ch, -1 * loss, FALSE);

      SET_BIT(victim->act,ACT_VAMPIRE);
      SET_BIT(victim->act,ACT_UNDEAD);
      victim->pcdata->blood = victim->level/2;
      victim->move=1;
      victim->pcdata->bloodline = strdup(ch->short_descr);
      send_to_char ("You embrace your victim!\r\n",ch);
      send_to_char ("The evil vampire gives you a bit of his power!\r\n",victim);
      send_to_char ("{rYou feel different!{x\r\n",victim);
      
      return;
   }


void vampire_strength( CHAR_DATA *ch, char *argument ) {
      AFFECT_DATA af;
      int amount;	
	
      if ( is_affected(ch, gafn_giant_strength)) {
            send_to_char ("Your strength is already at max.\r\n",ch);
            return;
      }

      if (argument[0]=='\0')	{
            send_to_char ("You must specify an amount of {rblood{x you want to spend.\r\n",ch);
            return;
      }

      amount=atoi(argument);

      if (amount < 1) {
              send_to_char ("That does not make sense..\r\n",ch);
              return;
      }

      if (amount>ch->pcdata->blood) amount=ch->pcdata->blood;
      
      ch->pcdata->blood -=amount;

      af.type              = SKILL_UNDEFINED;
      af.afn	= gafn_giant_strength; 
      af.level              = amount;
      af.duration        = amount;
      af.location        = APPLY_STR;
      af.modifier         = amount/5;
      af.bitvector       = 0;
      affect_to_char( ch, &af );
      send_to_char ("You feel much stronger!\r\n", ch);
       act ("$n suddenly looks much stronger!", ch, NULL, NULL, TO_ROOM);
 
      return;
   }


void vampire_mist( CHAR_DATA *ch, char *argument ) {
      AFFECT_DATA af;
      int amount;	
	
      if (IS_AFFECTED(ch, AFF_MIST)) {
            send_to_char ("You're already in mistform.\r\n",ch);
            return;
      }

      if (argument[0]=='\0')	{
            send_to_char ("You must specify an amount of {rblood{x you want to spend.\r\n",ch);
            return;
      }

      amount=atoi(argument);

      if (amount < 1) {
              send_to_char ("That doesn't make sense.\r\n",ch);
              return;
      }

      if (amount>ch->pcdata->blood) amount=ch->pcdata->blood;

      if (amount < 10) {
              send_to_char ("That isn't enough blood.\r\n",ch);
              return;
      }
      
      ch->pcdata->blood -=amount;

      af.type              = SKILL_UNDEFINED;
      af.afn	= gafn_mist; 
      af.level              = amount;
      af.duration        = (amount - 9) / 10 + 1;
      af.location        = APPLY_NONE;
      af.modifier         = 0;
      af.bitvector       = AFF_MIST;
      affect_to_char( ch, &af );
      send_to_char ("You dissolve into mist!\r\n", ch);
       act ("$n dissolves into mist!", ch, NULL, NULL, TO_ROOM);
       return;
   }


void vampire_levitate(CHAR_DATA *ch, char *argument ) {
AFFECT_DATA af;
int amount;	
	
      if (IS_AFFECTED(ch, AFF_FLYING)) {
            send_to_char ("You're already flying.\r\n",ch);
            return;
      }

      if (argument[0]=='\0')	{
            send_to_char ("You must specify an amount of {rblood{x you want to spend.\r\n",ch);
            return;
      }

      amount=atoi(argument);

      if (amount < 1) {
              send_to_char ("That doesn't make sense.\r\n",ch);
              return;
      }

      if (amount>ch->pcdata->blood) amount=ch->pcdata->blood;

      if (amount < 10) {
              send_to_char ("That isn't enough blood.\r\n",ch);
              return;
      }
      
      ch->pcdata->blood -=amount;

      af.type              = SKILL_UNDEFINED;
      af.afn	= gafn_fly; 
      af.level              = amount;
      af.duration        = amount *3 /2;
      af.location        = APPLY_NONE;
      af.modifier         = 0;
      af.bitvector       = AFF_FLYING;
      affect_to_char( ch, &af );
      send_to_char ("You start to levitate!\r\n", ch);
       act ("$n starts to levitate!", ch, NULL, NULL, TO_ROOM);
       return;
   }


void vampire_celerity( CHAR_DATA *ch, char *argument ) {
      AFFECT_DATA af;
      int amount;	
	
      if ( is_affected(ch, gafn_haste)) {
            send_to_char ("Your celerity is already at max.\r\n",ch);
            return;
      }

      if (argument[0]=='\0')	{
            send_to_char ("You must specify an amount of {rblood{x you want to spend.\r\n",ch);
            return;
      }

      amount=atoi(argument);

      if (amount < 1) {
              send_to_char ("That does not make sense..\r\n",ch);
              return;
      }

      if (amount>ch->pcdata->blood) amount=ch->pcdata->blood;
      
      ch->pcdata->blood -=amount;

      af.type              = SKILL_UNDEFINED;
      af.afn	= gafn_haste; 
      af.level              = amount;
      af.duration        = amount;
      af.location        = APPLY_DEX;
      af.modifier         = amount/5;
      af.bitvector       = 0;
      affect_to_char( ch, &af );
      send_to_char ("You feel much faster!\r\n", ch);
       act ("$n suddenly moves much faster!", ch, NULL, NULL, TO_ROOM);
 
      return;
   }

void vampire_fortitude( CHAR_DATA *ch, char *argument ) {
      AFFECT_DATA af;
      int amount;	
	
      if ( is_affected(ch, gafn_hard_skin)) {
            send_to_char ("Your fortitude is already at max.\r\n",ch);
            return;
      }

      if (argument[0]=='\0')	{
            send_to_char ("You must specify an amount of {rblood{x you want to spend.\r\n",ch);
            return;
      }

      amount=atoi(argument);

      if (amount < 1) {
              send_to_char ("That does not make sense..\r\n",ch);
              return;
      }

      if (amount>ch->pcdata->blood) amount=ch->pcdata->blood;
      
      ch->pcdata->blood -=amount;

      af.type              = SKILL_UNDEFINED;
      af.afn	= gafn_hard_skin; 
      af.level              = amount;
      af.duration        = amount/2;
      af.location        = APPLY_AC;
      af.modifier         = -((amount * 15)/10);
      af.bitvector       = 0;
      affect_to_char( ch, &af );
      send_to_char ("You feel much safer!\r\n", ch);
       act ("$n suddenly seems very well protected!", ch, NULL, NULL, TO_ROOM);
 
      return;
   }


void vampire_majesty( CHAR_DATA *ch, char *argument ) {
      AFFECT_DATA af;
      int amount;	
	
      if ( is_affected(ch, gafn_aura)) {
            send_to_char ("Your aura is already at max.\r\n",ch);
            return;
      }

      if (argument[0]=='\0')	{
            send_to_char ("You must specify an amount of {rblood{x you want to spend.\r\n",ch);
            return;
      }

      amount=atoi(argument);

      if (amount < 1) {
              send_to_char ("That does not make sense..\r\n",ch);
              return;
      }

      if (amount>ch->pcdata->blood) amount=ch->pcdata->blood;
      
      ch->pcdata->blood -=amount;

       af.type        = SKILL_UNDEFINED;
       af.afn 	        = gafn_aura;
       af.level        = amount;
       af.duration  = amount/2;
       af.location  = APPLY_AC;
       af.modifier  = -amount/5;
       af.bitvector = AFF_AURA;
       affect_to_char(ch, &af );
       send_to_char( "You reveal your majestic aura.\r\n", ch );

    return;
   }


void vampire_dark( CHAR_DATA *ch, char *argument ) {
      AFFECT_DATA af;
      int amount;	

      if ( is_affected(ch, gafn_darkness)) {
            send_to_char ("You are already walking in darkness.\r\n",ch);
            return;
      }
	
      if (argument[0]=='\0')	{
            send_to_char ("You must specify an amount of {rblood{x you want to spend.\r\n",ch);
            return;
      }

      amount=atoi(argument);

      if (amount < 1) {
              send_to_char ("That does not make sense..\r\n",ch);
              return;
      }

      if (amount>ch->pcdata->blood) {
              amount=ch->pcdata->blood;
      }

      ch->pcdata->blood -=amount;

      af.type              = SKILL_UNDEFINED;
      af.afn	= gafn_darkness; 
      af.level              = amount;
      af.duration        = amount;
      af.location        = 0;
      af.modifier         = 0;
      af.bitvector       = AFF_DARKNESS;
      affect_to_char( ch, &af );
      send_to_char ("You feel safe in the darkness!\r\n", ch);
       act ("$n is covered by a veil of darkness!", ch, NULL, NULL, TO_ROOM);
 
      return;
   }

void vampire_heal( CHAR_DATA *ch, char *argument ) {
      int amount;	
	
      if (argument[0]=='\0')	{
            send_to_char ("You must specify an amount of {rblood{x you want to spend.\r\n",ch);
            return;
      }

      amount=atoi(argument);

      if (amount < 1) {
              send_to_char ("That does not make sense..\r\n",ch);
              return;
      }

      if (amount>ch->pcdata->blood) amount=ch->pcdata->blood;
      
      ch->pcdata->blood -=amount;

      ch->hit +=amount*50;
      ch->mana +=amount*50;
      ch->move +=amount*50;

      if (ch->hit>ch->max_hit) {
            ch->hit=ch->max_hit;
      }
      if (ch->mana>ch->max_mana) {
            ch->mana=ch->max_mana;
      }
      if (ch->move>ch->max_move) {
            ch->move=ch->max_move;
      }

      send_to_char ("You feel much better!\r\n", ch);
      act ("All of $ns wounds seem to close!", ch, NULL, NULL, TO_ROOM);
 
      return;
   }



/*Lich*/

void do_lich (CHAR_DATA *ch, char *argument) {
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	
               if (!IS_SET(ch->act,ACT_UNDEAD)
               ||  IS_SET(ch->act,ACT_VAMPIRE) ) {
                      send_to_char ("You're no Lich!\r\n",ch);
                      return;
               }                

                argument=one_argument(argument, arg1);
	argument=one_argument(argument, arg2);
                argument=one_argument(argument, arg3);
	
          if (arg1[0]=='\0') { 
               send_to_char ("\r\nLich commands:\r\n",ch);
               send_to_char ("\r\n",ch);
               send_to_char ("  LICH touch <victim>\r\n",ch);
               send_to_char ("  LICH dominate <victim>\r\n",ch);
               send_to_char ("  LICH regenerate <limb>\r\n",ch);
               return;
           }

           if (!str_cmp(arg1, "touch"   )) {
                lich_touch(ch,arg2);
          } else if (!str_cmp(arg1, "dominate"   )) {
                lich_dominate(ch,arg2);
          } else if (!str_cmp(arg1, "regenerate"   )) {
                lich_regenerate(ch,arg2);
          } else {
              send_to_char ("No such lich command.\r\n",ch);
              return;
          }

           return;
 }


void lich_regenerate( CHAR_DATA *ch, char *argument) {
char arg[MAX_INPUT_LENGTH];
int limb = 0;
int heal = 0;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )    {
	send_to_char( "Which limb?\r\n", ch );
	return;
    }

    if (!str_cmp(arg, "head")) limb =0;
    else if (!str_cmp(arg, "torso")) limb =1;
    else if (!str_cmp(arg, "larm")) limb =2;
    else if (!str_cmp(arg, "rarm")) limb =3;
    else if (!str_cmp(arg, "legs")) limb =4;
    
    if (ch->limb[limb] == 0) {
          sprintf_to_char(ch, "Your %s is completely healed.\r\n", flag_string(limb_name, limb));
          return;
    }

    heal = ch->limb[limb] * 100;
    
    if (heal > ch->hit) {
          send_to_char("You are to weak to close the wounds.\r\n",ch);
          return;
    }

    ch->hit -= heal;
    ch->limb[limb] = 0;
    sprintf_to_char (ch, "Your %s regenerates.\r\n",  flag_string(limb_name, limb));
    act ("$n begins to gegenerate.", ch, NULL, NULL, TO_ROOM);
    return;
}


void lich_touch ( CHAR_DATA *ch, char *argument) {
char arg[MAX_INPUT_LENGTH];
CHAR_DATA *victim;
AFFECT_DATA af;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )    {
	send_to_char( "Touch whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
     }

      if ( victim->position  != POS_STUNNED  ) {
	send_to_char( "Your victim wont let you.\r\n", ch );
	return;
     }

      if (IS_SET(victim->act, ACT_UNDEAD)) {
                 send_to_char ("You can't drain energy from that victim!\r\n",ch);
                  return;
     }                

     if (victim->mana < ch->max_mana/4) {
                 send_to_char ("Your victim is much too weak!\r\n",ch);
                  return;
     }                

     if (ch->move<50
     || ch->mana<50) {
	send_to_char( "Your are too exhausted.\r\n", ch );
	return;
     }

     ch->move -=50;
     ch->mana -=50;
     WAIT_STATE(ch, 24);
     
     ch->mana +=victim->mana/2;
     ch->mana=UMIN(ch->mana,ch->max_mana);
     victim->mana=0;
  
      af.type              = SKILL_UNDEFINED;
      af.afn	= gafn_weakness; 
      af.level              = ch->level;
      af.duration        = ch->level/3;
      af.location        = APPLY_STR;
      af.modifier         = -ch->level/10;
      af.bitvector       = 0;
      affect_to_char( victim, &af );

      send_to_char ("You feel powerful!\r\n", ch);
      send_to_char ("What happens to you!\r\n", victim);
      act ("With a malicious grin $n touches $N.", ch, NULL, victim, TO_ROOM);
      return;
}

void lich_dominate ( CHAR_DATA *ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )    {
	send_to_char( "Dominate whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
     }

     if (!IS_NPC(victim)
     || IS_SET(victim->imm_flags, IMM_CHARM)) {
	send_to_char( "You can't dominate thet..\r\n", ch );
	return;
     }


     if ((!IS_SET(victim->act, ACT_UNDEAD)
     && !IS_SET(victim->form, FORM_UNDEAD))
     || (IS_SET(victim->act, ACT_AGGRESSIVE)
     && victim->level+11 > ch->level)) {
	send_to_char( "You can't dominate that victim.\r\n", ch );
	return;
     }

    if( ch->pet != NULL ) {
        send_to_char( "You already have a pet.\r\n", ch );
        return;
    }

     if (ch->mana < 100) {
	send_to_char( "Not enough mana.\r\n", ch );
	return;
     }

     ch->mana -= 100;

     chance = 40 + ch->level - victim->level;
     if (number_percent() < chance) {
     
         act( "$n dominates $N...", ch, NULL, victim, TO_ROOM );
         act( "You dominate $N...", ch, NULL, victim, TO_CHAR );

   /* Enslave the familier... */

         SET_BIT(victim->affected_by, AFF_CHARM);
         SET_BIT(victim->act, ACT_FOLLOWER);
         victim->comm = COMM_NOCHANNELS;

         add_follower(victim, ch );
         victim->leader = ch;
         ch->pet = victim;
         do_emote(victim, "looks at you expectently." );
         return;
     } else {
         if (number_percent()>30) {
             do_flee(victim, "");
         } else {
             multi_hit( victim, ch, TYPE_UNDEFINED );
             check_assist(victim, ch);
         }
          return;
     }
     return;
}



void do_mindtransfer(CHAR_DATA *ch, char *argument) { 
char arg[MAX_INPUT_LENGTH];
CHAR_DATA *victim;
CHAR_DATA *old_char = NULL;
int chance;

    if (ch->desc == NULL) return;

    if (ch->desc->original == NULL) {
           if (str_cmp(race_array[ch->race].name,"yithian")) {
	send_to_char( "You're no Yithian!\r\n", ch );
	return;
           }
    } else {
           if (str_cmp(race_array[ch->race].name,"yithian")
           && str_cmp(race_array[ch->desc->original->race].name,"yithian")) {
	send_to_char( "You're no Yithian!\r\n", ch );
	return;
           }
    }

    argument = one_argument(argument, arg);

    if ( arg[0] == '\0' )    {
	send_to_char( "Possess whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
     }

    if ( victim == ch ) return;
    
    if (!IS_NPC(victim)
    || IS_SET(victim->act, ACT_AGGRESSIVE)
    || IS_SET(victim->act, ACT_INVADER)
    || IS_SET(victim->act, ACT_IS_HEALER)
    || IS_SET(victim->act, ACT_IS_ALIENIST)
    || IS_SET(victim->act, ACT_PRACTICE)
    || victim->pIndexData->pShop != NULL){
	send_to_char("You can't switch into that mob.\r\n",ch);
	return;
    }

    if ( victim->desc != NULL ) {
	send_to_char( "Character in use.\r\n", ch );
	return;
    }

    if (ch->move < victim->level * 10
    || ch->mana < victim->level * 10) {
	send_to_char( "You are too exhausted to concentrate.\r\n", ch );
	return;
    }

    ch->move -= victim->level *10;
    ch->mana -= victim->level *10;

    act("You begin to concentrate on $N.",ch, NULL, victim,TO_CHAR);
    act("$n begins to concentrate on $N.",ch, NULL, victim,TO_ROOM);

    WAIT_STATE(ch, 12);
	
    chance = (3 + ((12 * ch->level)/10)  - victim->level) * 5;
    if (victim->level <= 10) chance *= 2;
    if (victim->level <= 5
    && chance < 75) chance = 75;
         
    if (number_percent() < chance) {

        if ( ch->desc->original != NULL ) {
            old_char = ch->desc->original;
            REMOVE_BIT(ch->plr, PLR_COLOUR);
            SET_BIT(ch->comm, COMM_NOCHANNELS);
            ch->pcdata = NULL;
            ch->desc->character  = ch->desc->original;
            ch->desc->original = NULL;
            ch->desc->character->desc = ch->desc; 
            ch->desc = NULL;
        }

        if (old_char) ch = old_char;

        victim->pcdata = ch->pcdata;
        ch->desc->character = victim;
        ch->desc->original  = ch;
        victim->desc        = ch->desc;
        victim->comm = ch->comm;
        victim->lines = ch->lines;
        SET_BIT(victim->plr, PLR_COLOUR);   
        REMOVE_BIT(ch->comm, COMM_NOCHANNELS);

        send_to_char( "You transfer your mind.\r\n", victim );
        act("$n looks extremely bored.",ch, NULL, NULL,TO_ROOM);
        notify_message (ch, WIZNET_SWITCH, TO_IMM, victim->short_descr);
     } else {
        send_to_char( "You fail to transfer your mind.\r\n", victim );
        if (victim->level < ch->level) do_flee(victim,"");
        else SET_BIT(victim->act, ACT_AGGRESSIVE);
     }
     return;
}


void do_yith( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_INPUT_LENGTH];

    if (ch->desc == NULL) {
        send_to_char("You can't do that while in your own body\r\n", ch);
       return;
    }

    if (ch->desc->original == NULL ) {
        send_to_char("You can't do that while in your own body\r\n", ch);
        return;
    }

    if (str_cmp(race_array[ch->desc->original->race].name,"yithian")) {
        send_to_char("You can't do that.\r\n", ch);
        return;
    }

  argument = one_argument(argument, arg); 

    if (arg == '\0') {
        send_to_char("YITH commands:\r\n", ch);
        send_to_char("YITH adapt <skill> [count]\r\n", ch);
        send_to_char("YITH abduct\r\n", ch);
        return;
    }

  if (!str_cmp(arg, "adapt"   )) {
       yith_adapt(ch, argument);
  } else if (!str_cmp(arg, "abduct"     )) {
       yith_abduct(ch, argument);
  } else {
       send_to_char ("No such yith command.\r\n",ch);
        send_to_char("YITH commands:\r\n", ch);
        send_to_char("YITH adapt <skill> [count]\r\n", ch);
        send_to_char("YITH abduct\r\n", ch);
       return;
  }

  return;
}


void yith_abduct(CHAR_DATA *ch, char *argument)	{
 ROOM_INDEX_DATA *room;

  room = get_room_index(YITH_START);
  if (room == NULL) {
       send_to_char ("You don't know that place.\r\n",ch);
       return;
  }
  
  if ( ch->fighting != NULL ) stop_fighting( ch, TRUE );
    
  act("$n suddeny disappears!",ch, NULL, NULL, TO_ROOM);
  char_from_room(ch);
  char_to_room( ch, room);
  act("$n suddenly appears!",ch, NULL, NULL, TO_ROOM);

  do_look( ch, "auto" );
  return;
}
	





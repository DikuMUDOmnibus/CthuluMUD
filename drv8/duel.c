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
#include "exp.h"
#include "wev.h"
#include "mob.h"
#include "magic.h"
#include "affect.h"

DECLARE_DO_FUN(do_stand);
DECLARE_DO_FUN(do_duel);


void do_magical_duel(CHAR_DATA *ch, char *args) {
char buf[MAX_INPUT_LENGTH];
CHAR_DATA *victim;
int cpower, vpower;

    if ( ch->activity != ACV_DUEL 
    || args[0] != '*' ) {
      send_to_char("You are not in a duel!\r\n", ch);
      return;
    }

    victim = get_char_room(ch, ch->acv_text);

    if (victim == NULL) {
         do_duel_finish(ch, NULL, ch, NULL, 
                                      "They have left the room!\r\n",
                                       NULL,
                                       NULL);
         return;
    }  

    if ( victim->position < POS_STANDING
    || victim->activity != ACV_DUEL) {
         do_duel_finish(ch, NULL, ch, NULL, 
                                      "They have given up!\r\n",
                                       NULL,
                                       NULL);
         return;
    } 


    if ( victim->move < DUEL_COST ) {
        do_duel_finish(ch, victim, ch, victim, 
                                        "@v2 is to tired to continue!\r\n",
                                        "You are to tired to continue!\r\n",
                                        "@v2 gives up arguing with @a2!\r\n" ); 
        return;
    }
    victim->move -= DUEL_COST; 

    if ( ch->move < DUEL_COST ) {
        do_duel_finish(ch, victim, victim, ch, 
                                        "@v2 is to tired to continue!\r\n",
                                        "You are to tired to continue!\r\n",
                                        "@v2 gives up arguing with @a2!\r\n" ); 
        return;
    }
    ch->move -= DUEL_COST; 

    cpower = (ch->acv_int + 4*get_curr_stat(ch, STAT_INT) + 2*get_curr_stat(ch, STAT_WIS)) /10 * number_range(6, 14);
    if (ch->hit < ch->max_hit/2) cpower /= 2;
    if (ch->hit < ch->max_hit/8) cpower /= 2;
    if (ch->move < ch->max_move/3) cpower /= 2;
    if (ch->move < ch->max_move/10) cpower /= 2;

    vpower = (victim->acv_int + 4*get_curr_stat(victim, STAT_INT) + 2*get_curr_stat(victim, STAT_WIS)) /10 * number_range(6, 14);
    if (victim->hit < victim->max_hit/2) cpower /= 2;
    if (victim->hit < victim->max_hit/8) cpower /= 2;
    if (victim->move < victim->max_move/3) cpower /= 2;
    if (victim->move < victim->max_move/10) cpower /= 2;

    ch->acv_int = ch->acv_int *8 / 10;
    victim->acv_int = victim->acv_int *8 / 10;

    if (cpower > vpower) {
          if (cpower > vpower*5) {
               ch->acv_state +=4;
               victim->acv_state -=4;
          } else if (cpower > vpower*2) {
               ch->acv_state +=2;
               victim->acv_state -=2;
          } else {
               ch->acv_state++;
               victim->acv_state--;
          }
    } else {
          if (vpower > cpower*5) {
               victim->acv_state +=4;
               ch->acv_state -=4;
          } else if (vpower > cpower*2) {
               victim->acv_state +=2;
               ch->acv_state -=2;
          } else {
               victim->acv_state++;
               ch->acv_state--;
          }
    }

    if (IS_NPC(victim)) {
        if (victim->acv_state < 15 || victim->acv_state >35) {
             if (victim->acv_int < victim->mana/2) {
                  if (victim->mana/3 >0) {
                      sprintf(buf, "%d", victim->mana/3);
                      do_duel(victim, buf);
                  }         
             }
        } else {
             if (victim->acv_int < victim->mana/4) {
                  if (victim->mana/8 >0) {
                      sprintf(buf, "%d", victim->mana/8);
                      do_duel(victim, buf);
                  }         
             }
        }
    }

    if ( ch->acv_state < 10 ) {
        do_duel_finish( ch, victim, victim, ch, NULL, NULL, NULL );
    } else if ( ch->acv_state > 40 ) {
        do_duel_finish( ch, victim, ch, victim, NULL, NULL, NULL );
    } else {
        do_duel_prompt(ch, victim);
        schedule_activity( ch, DUEL_SPEED, "induel" );
    }
    return;
}


void do_duel_prompt(CHAR_DATA *ch, CHAR_DATA *victim) {
char score[100];
int i;

   /* Format a pretty prompt... */

    sprintf_to_char(ch, "        %5d                   %5d\r\n", ch->acv_int, victim->acv_int);
    sprintf(score, "Duel:   {g-----------------------------------{x\r\n");

    i = ch->acv_state;

    score[i++] = '{';
    score[i++] = 'Y';
    score[i++] = 'O';
    score[i++] = '{';
    score[i++] = 'y';

    send_to_char(score, ch);

    sprintf_to_char(victim, "        %5d                   %5d\r\n", victim->acv_int, ch->acv_int);
    sprintf(score, "Duel:   {g-----------------------------------{x\r\n");

    i = (50 - ch->acv_state);

    score[i++] = '{';
    score[i++] = 'Y';
    score[i++] = 'O';
    score[i++] = '{';
    score[i++] = 'y';

    send_to_char(score, victim);

    return;
}


void do_duel_finish(CHAR_DATA *ch, CHAR_DATA *victim, CHAR_DATA *winner, CHAR_DATA *loser, char *msg, char *msg2, char *msg3 ) {
MOB_CMD_CONTEXT *mcc;
WEV *wev;
AFFECT_DATA af;
int level;

    if (!ch) return;
    
    mcc = get_mcc(ch, winner, loser, NULL, NULL, NULL, 0, NULL);

    if ( msg != NULL ) {
      wev = get_wev(WEV_DEBATE, WEV_DUEL_FINISH, mcc, msg, msg2, msg3);

    } else {

      wev = get_wev(WEV_DEBATE, WEV_DUEL_FINISH, mcc,
                   "@v2 gives up, completely exhausted!\r\n", 
                   "You are defeated by @a2s magic.\r\n",
                   "@a2 wins the duel!\r\n");
    }

    if (loser) {
        level = (winner->acv_int - loser->acv_int) /5;
        level = UMAX(level, 10);
        if (level > 50) level = (level -50) /2 +50;
        if (level > 100) level = (level -100) /2 +100;
        if (level > 125) level = (level -125) /2 +125;
        if (level > 150) level = (level -150) /2 +150;

        af.type      	= 0;
        af.afn 	 	= gafn_fatigue;
        af.level	= level;
        af.duration  	= level /2;
        af.location  	= APPLY_INT;
        af.modifier  	= -level /10;
        af.bitvector 	= AFF_MELD;
        affect_to_char(loser, &af );

         winner->mana = UMIN(winner->mana + loser->mana, winner->max_mana);
         loser->mana = 0;
         winner->move = UMIN(winner->mana + loser->move/5, winner->max_mana);
         loser->move = 0;
         update_pos(loser);
    }
    room_issue_wev( ch->in_room, wev);
    free_wev(wev);
    set_activity(ch, ch->position, ch->pos_desc, ACV_NONE, NULL);

    if (victim != NULL
    && victim->activity == ACV_DUEL) { 
      set_activity(victim, victim->position, victim->pos_desc, ACV_NONE, NULL);
    }

    return;
}


void do_duel(CHAR_DATA *ch, char *argument) {
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
CHAR_DATA *victim = NULL;
MOB_CMD_CONTEXT *mcc;
WEV *wev;
int omana = 0;
int dmana = 0;

    if (ch->activity != ACV_DUEL) {
        send_to_char("You are not in a magical duel right now.\r\n", ch);
        return;
    } 

    if (ch->acv_text) victim = get_char_room(ch, ch->acv_text);
    if (!victim) do_stand(ch, "");

    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);

    if (is_number(arg1)) {
        omana = atoi(arg1);
    } else {
        if (!str_cmp(arg1, "off")) omana = atoi(arg2);
        else if (!str_cmp(arg1, "def")) dmana = atoi(arg2) /2;

    }
    
    if (dmana > victim->acv_int) dmana = victim->acv_int;

    if (omana > ch->mana || dmana > ch->mana) {
        send_to_char("You lack the mana.\r\n", ch);
        return;
    } 

    if (omana > 0) {
        mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, omana, NULL);
        wev = get_wev(WEV_DUEL, WEV_DUEL_OFFENSIVE, mcc,
                  "You invoke the powers of magic ({C@n0{x)!\r\n",
                  "@a2 invokes the powers of magic ({C@n0{x)!\r\n",
                  "@a2 invokes the powers of magic!\r\n");

        if (!room_issue_wev_challange( ch->in_room, wev)) {
            free_wev(wev);
            return;
        } 

        ch->mana -= omana;
        ch->acv_int += omana;

        room_issue_wev( ch->in_room, wev);
        free_wev(wev);

    } else if (dmana > 0) {
        int t;

        mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, omana, NULL);
        wev = get_wev(WEV_DUEL, WEV_DUEL_OFFENSIVE, mcc,
                  "You weaken @v2s offenses ({C@n0{x)!\r\n",
                  "@a2 weakens your offenses ({C@n0{x)!\r\n",
                  "@a2 consumes the powers of magic!\r\n");

        if (!room_issue_wev_challange( ch->in_room, wev)) {
            free_wev(wev);
            return;
        } 

        t = UMAX(victim->acv_int - 100, 0);
        if (t > dmana) {
            victim->acv_int -= dmana;
            dmana = 0;
        } else {
             victim->acv_int = 100;
             dmana -= t;
        }

        if (dmana > 0) {
            dmana /= 3;
            victim->acv_int = UMAX(victim->acv_int, 1);
        }

        room_issue_wev( ch->in_room, wev);
        free_wev(wev);
    }

    return;
}







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
#include "skill.h"
#include "wev.h"
#include "mob.h"
#include "gsn.h"

#define DEBATE_SPEED	3
#define DEBATE_COST	5

char *subject[] = {
 
   "nothing in particular",
   "the weather",
   "the time of day",
   "the existance of free will",
   "the existance of god",
   "the existance of satan",
   "the definition of sanity",
   "the nature of time",
   "the merits of various handguns",
   "the shortage of skirts",

   "prohibition",
   "the wonders of modern science",
   "the mad hatters tea party",
   "pulp fiction",
   "contempory horror writers",
   "the government",
   "evolution and creatonism",
   "strange religious cults",
   "the right to free speach",
   "the nature of dreams",

   "communism",
   "the flaws of democracy",
   "lawyers",
   "nihilism",
   "the merits of anacrhy",
   "local politics",
   "the presidents mistress",
   "J Edgar Hoovers predelictions",
   "the existance of dragons",
   "the nature of magic",

   "the grassy knoll",
   "the great Jewish conspiracy", 
   "the bavarian conspiracy",
   "the virtues of celebacy",
   "the temptations of priesthood",
   "the Jazz club",
   "interpreting dreams as premonitions",
   "the wealth of the church",
   "the Black problem",
   "the Italian problem",

   "the German problem",
   "the Dutch problem",
   "the Mexican problem",
   "the Cuban problem",
   "the Canadian problem",
   "gangsters",
   "the Spanish civil war",
   "the great war",
   "the evolution of knowledge",
   "the Bible as a teaching tool",

   "the Koran as a teaching tool",
   "the secrets of the Kabalah",
   "the nature of Fausts mistake",
   "the dark books in the library",
   "the value of eroticism",
   "predestination and fortune telling",
   "fishing",
   "nature vs nurture",
   "the reproductive cycle of the abasynian tree sloth",
   "socialism and high taxes",

   "social responsability",
   "cubism and surrealism",
   "virginity and marriage",
   "the importance of freedom of choice"

};

void do_debate_continue( CHAR_DATA *ch, CHAR_DATA *winner,	 CHAR_DATA *loser, int points, char *mwinner, char *mloser, char *mobs) {
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;
    int sn;

   /* Sanity... */

    if ( ch == NULL || winner == NULL || loser == NULL ) return;

   /* Generate WEV... */

    mcc = get_mcc(ch, winner, loser, NULL, NULL, NULL, points, NULL);
    wev = get_wev(WEV_DEBATE, WEV_DEBATE_CONTINUE, mcc, mwinner, mloser, mobs);

   /* Send to command issuers room... */

    room_issue_wev( ch->in_room, wev);

   /* Free the wev (will autofree the context)... */

    free_wev(wev);

   /* Update the score... */

    if ( ch == winner ) ch->acv_state += points;
    else ch->acv_state -= points;

   /* Skill checks... */

    sn = ch->acv_int;

    if ( sn != 0 ) {
      check_improve(winner, sn, TRUE, 8);
      check_improve(loser, sn, FALSE, 8);
    }
     
   /* All done... */ 

    return;
}

void do_debate_finish(CHAR_DATA *ch, CHAR_DATA *victim, CHAR_DATA *winner, CHAR_DATA *loser, char *msg, char *msg2, char *msg3 ) {
    MOB_CMD_CONTEXT *mcc;
    WEV *wev;

   /* Sanity... */

    if ( ch == NULL ) return;

   /* Generate WEV... */

    mcc = get_mcc(ch, winner, loser, NULL, NULL, NULL, 0, NULL);

    if ( msg != NULL ) {

      wev = get_wev(WEV_DEBATE, WEV_DEBATE_FINISH, mcc, msg, msg2, msg3);
    } else {

      wev = get_wev(WEV_DEBATE, WEV_DEBATE_FINISH, mcc,
                   "@v2 conceeds defeat!\r\n", 
                   "You are defeated by @a2s arguments.\r\n",
                   "@a2 wins the debate!\r\n");

      check_improve(winner, gsn_debating, TRUE, 6);

      gain_exp(winner, 40, FALSE );

      check_improve(loser, gsn_debating, FALSE, 4);
      if (IS_NPC(loser)) gain_exp(loser, -40, FALSE );
    }

   /* Send to command issuers room... */

    room_issue_wev( ch->in_room, wev);

   /* Free the wev (will autofree the context)... */

    free_wev(wev);

   /* Update activity... */
 
    set_activity(ch, ch->position, ch->pos_desc, ACV_NONE, NULL);

    if ( victim != NULL
      && victim->activity == ACV_DEBATING) { 
      set_activity(victim, victim->position, victim->pos_desc, ACV_NONE, NULL);
    }

    return;
}


void do_debate_prompt(CHAR_DATA *ch, CHAR_DATA *victim) {
    char score[100];
    int i;

   /* Format a pretty prompt... */

    sprintf(score, "Debate: {g-----------------------------------{x\r\n");

    i = ch->acv_state;

    score[i++] = '{';
    score[i++] = 'Y';
    score[i++] = 'O';
    score[i++] = '{';
    score[i++] = 'y';

    send_to_char(score, ch);

    sprintf(score, "Debate: {g-----------------------------------{x\r\n");

    i = (50 - ch->acv_state);

    score[i++] = '{';
    score[i++] = 'Y';
    score[i++] = 'O';
    score[i++] = '{';
    score[i++] = 'y';

    send_to_char(score, victim);

    return;
}

void do_debate(CHAR_DATA *ch, char *args) {
char parm[MAX_STRING_LENGTH];
char buf[MAX_STRING_LENGTH];
CHAR_DATA *victim;
CHAR_DATA *winner, *loser;
int sn, delta;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

   /* Retrieve key from command... */

    if (!check_activity_key(ch, args)) return;

   /* If already debating and not a recall, complain... */

    if ( ch->activity == ACV_DEBATING
    && args[0] != '*' ) {
      send_to_char("You are already debating!\r\n", ch);
      return;
    }

   /* Avoid problems switching activities... */

    if (ch->activity != ACV_DEBATING) ch->acv_state = 0;

   /* Ok, what are we doing... */

    if ( ch->acv_state == 0 ) {

     /* Trying to start a new debate... */

      args = one_argument(args, parm);

     /* Check the player can debate... */

      if (get_skill(ch, gsn_debating) < 1 ) {
        send_to_char("You have no skill in debating.\r\n", ch);
        return;
      }

      if (IS_AFFECTED(ch, AFF_MELD)) {
        send_to_char("You're feeling stupid.\r\n", ch);
        return;
      }

     /* Locate your opponent... */

      victim = get_char_room(ch, parm);

      if (victim == NULL) {
        send_to_char("They are not here!\r\n", ch);
        return;
      }  

      if (victim == ch || !IS_NPC(victim)) {
        send_to_char("Be reasonable!\r\n", ch);
        return;
      }  

     /* Check they are not busy... */

      if ( victim->position < POS_RESTING
      || victim->activity != ACV_NONE ) {
        send_to_char("They are to busy to debate with you now.\r\n", ch); 
        return;
      } 

     /* Check they can understand you... */

      sn = ch->speak[SPEAK_PUBLIC]; 

      if (get_skill(victim, sn) < 50) {
        send_to_char("They cannot understand what you are saying!\r\n", ch); 
        return;
      }

     /* Check you can understand them... */

      sn = victim->speak[SPEAK_PUBLIC]; 

      if (get_skill(ch, sn) < 50) {
        send_to_char("You cannot understand what they are saying!\r\n", ch); 
        return;
      }

     /* Check the victim can debate... */

      if (get_skill(victim, gsn_debating) < 1 ) {
        send_to_char("They have no skill in debating!\r\n", ch);
        return;
      }

     /* Check you're not to tired... */

      if ( ch->move < ( 10 * DEBATE_COST )) {
        send_to_char("You are to tired to start debating now.\r\n", ch);
        return;
      } 

     /* Check they are not to tired... */

      if ( victim->move < ( 10 * DEBATE_COST )) {
        send_to_char("They are to tired to start debating now.\r\n", ch);
        return;
      } 

     /* Now, what are we talking about... */

      if (args[0] =='\0') sn = get_skill_sn("debating");
      else sn = get_skill_sn(args);

      if (sn <= 0) {
         do_say(victim, "Let's debate something else.");
         return;
      }

     /* If it's a skill, check the debate is resonable... */

      if ( sn > 0 ) {
        delta = get_skill(ch, sn) - get_skill(victim, sn); 

        if ( delta > 30 ) {

          mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 0, args);
          wev = get_wev(WEV_DEBATE, WEV_DEBATE_FINISH, mcc,
                  "{W@v2 conceeds to your masterful arguments.{x\r\n",
                   NULL,
                  "@a2 utters something incomprehensible at you!\r\n");

         /* Send to command issuers room... */

          room_issue_wev( ch->in_room, wev);

         /* Free the wev (will autofree the context)... */

          free_wev(wev);

          return;

        } else if ( delta < -30 ) {
  
          mcc = get_mcc(ch, victim, ch, NULL, NULL, NULL, 0, args);
          wev = get_wev(WEV_DEBATE, WEV_DEBATE_FINISH, mcc,
                  "{W@You explain @t0 to @v2 at @a3 request!{x\r\n",
                   NULL,
                  "{W@a2s arguments are too complex for you to follow!\r\n{x");

         /* Send to command issuers room... */

          room_issue_wev( ch->in_room, wev);

         /* Free the wev (will autofree the context)... */

          free_wev(wev);

          return;

        }

       /* Cannot debate teach... */

        if ( sn == gsn_teach ) {
          send_to_char("That would be futile.\r\n", ch);
          return; 
        }

        if (skill_array[sn].learnable == FALSE
        || skill_array[sn].debate == FALSE
        || IS_SET(skill_array[sn].group, SKILL_GROUP_COMBAT)) {
          send_to_char("That would be futile.\r\n", ch);
          return; 
        }
      }

     /* Also check debating skill... */

      delta = get_skill(ch, gsn_debating) - get_skill(victim, gsn_debating); 

      if ( delta > 30 ) {

        mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 0, args);
        wev = get_wev(WEV_DEBATE, WEV_DEBATE_FINISH, mcc,
                "{W@v2 conceeds to your masterful arguments.{x\r\n",
                 NULL,
                "@a2 utters something incomprehensible at you!\r\n");

       /* Send to command issuers room... */

        room_issue_wev( ch->in_room, wev);

       /* Free the wev (will autofree the context)... */

        free_wev(wev);

        return;

      } else if ( delta < -30 ) {
  
        mcc = get_mcc(ch, victim, ch, NULL, NULL, NULL, 0, args);

        wev = get_wev(WEV_DEBATE, WEV_DEBATE_FINISH, mcc,
                "{W@You explain @t0 to @v2 at @a3 request!{x\r\n",
                 NULL,
                "{W@a2 arguments are too complex for you to follow!\r\n{x");

       /* Send to command issuers room... */

        room_issue_wev( ch->in_room, wev);

       /* Free the wev (will autofree the context)... */

        free_wev(wev);

        return;

      }

     /* Must have something to talk about... */

      if ( args[0] == '\0' ) {
        args = subject[number_bits(6)];
      }  

     /* Start the debate rolling... */

      mcc = get_mcc(ch, ch, victim, NULL, NULL, NULL, 0, args);

      wev = get_wev(WEV_DEBATE, WEV_DEBATE_START, mcc,
                  "You start debating @t0 with @v2!{x\r\n",
                  "@a2 starts debating @t0 with you!{x\r\n",
                  "@a2 and @v2 start arguing about @t0!\r\n{x");

     /* Check for permission... */

      if (!room_issue_wev_challange( ch->in_room, wev)) {
        free_wev(wev);
        return;
      } 

     /* Send to command issuers room... */

      room_issue_wev( ch->in_room, wev);

     /* Free the wev (will autofree the context)... */

      free_wev(wev);

     /* Characters activity... */

      if ( sn > 0 ) sprintf( buf, "debating %s with %s.",  skill_array[sn].name, victim->short_descr);
      else sprintf( buf, "debating %s with %s.", args, victim->short_descr);

      set_activity(ch, ch->position, ch->pos_desc, ACV_DEBATING, buf);

     /* Victims activity... */

      if ( sn > 0 ) sprintf( buf, "debating %s with %s.", skill_array[sn].name, ch->short_descr);
      else sprintf( buf, "debating %s with %s.", args, ch->short_descr);

      set_activity(victim, victim->position, victim->pos_desc,  ACV_DEBATING, buf);

     /* And we're off! */ 

      set_activity_key(ch);

      ch->acv_state = 25; 

      ch->acv_int = sn;

      free_string(ch->acv_text);
      ch->acv_text = str_dup(victim->true_name);

      do_debate_prompt(ch, victim);

      schedule_activity( ch, DEBATE_SPEED, "debate" );

    } else {

     /* Continuing an existing debate... */

     /* Locate your opponent... */

      victim = get_char_room(ch, ch->acv_text);

      if (victim == NULL) {
        do_debate_finish(ch, NULL, ch, NULL, 
                                      "They have left the room!\r\n",
                                       NULL,
                                      "@a2 stops arguing with @a4self!\r\n" );
        return;
      }  

     /* Check they have not been distracted... */

      if ( victim->position < POS_RESTING
        || victim->activity != ACV_DEBATING ) {
        do_debate_finish(ch, victim, ch, victim, 
                                        "@v2 is ignoring you!\r\n",
                                         NULL,
                                        "@a2 finally shuts up!\r\n" ); 
        return;
      } 

     /* Pay the price... */

      if ( victim->move < DEBATE_COST ) {
        do_debate_finish(ch, victim, ch, victim, 
                                        "@v2 are to tired to continue!\r\n",
                                        "You are to tired to continue!\r\n",
                                        "@v2 gives up arguing with @a2!\r\n" ); 
        return;
      }

      victim->move -= DEBATE_COST; 

      if ( ch->move < DEBATE_COST ) {
        do_debate_finish(ch, victim, victim, ch, 
                                        "@v2 is to tired to continue!\r\n",
                                        "You are to tired to continue!\r\n",
                                        "@v2 gives up arguing with @a2!\r\n" ); 
        return;
      }

      ch->move -= DEBATE_COST; 

     /* Make some rolls... */

      delta = number_open() - number_open();
 
      delta += get_skill(ch, gsn_debating) - get_skill(victim, gsn_debating);

     /* Add skills if not just shooting the wind... */

      if ( ch->acv_int > 0 ) {
        delta += get_skill(ch, ch->acv_int) - get_skill(victim, ch->acv_int);
      }  

     /* Now see who won (+ve is player, -ve is victim)... */ 

      if (delta > 0) {
        winner = ch;
        loser = victim;
      } else {
        winner = victim;
        loser = ch;
        delta *= -1;
      } 

     /* Message time... */

      if ( delta < 10 ) {
        do_debate_continue( ch, winner, loser, 0,
           "You make a fair argument!\r\n",
           "You make a fair argument!\r\n",
           "@v2 and @a2 make a lot of noise!\r\n" );
      } else if ( delta < 20 ) {
        do_debate_continue( ch, winner, loser, 0,
           "You make a good argument!\r\n",
           "You make a good argument!\r\n",
           "@v2 and @a2 make a lot of noise!\r\n" );
      } else if ( delta < 35 ) {
        do_debate_continue( ch, winner, loser, 1,
           "You raise your voice to make your point clear!\r\n",
           "@a2 starts shouting and spitting at you!\r\n",
           "@a2 starts shouting and spitting at @v2!\r\n" );
      } else if ( delta < 50 ) {
        do_debate_continue( ch, winner, loser, 1,
           "You use @v2s shady past against him!\r\n",
           "@a2 dredges up your past errors!\r\n",
           "@v2 makes a stunning revelation about @a2s past!\r\n" );
      } else if ( delta < 65 ) {
        do_debate_continue( ch, winner, loser, 2,
           "You turn @v2s circular arguments back upon @v4!\r\n",
           "Your circular arguments confuse you!\r\n",
           "@v2 turns @a2s circular arguments around!\r\n" );
      } else if ( delta < 80 ) {
        do_debate_continue( ch, winner, loser, 2,
           "You make a cutting remark about @v2s social life!\r\n",
           "@a2 makes a cutting remark about your social life!\r\n",
           "@a2 makes a cruel remark about @v2s social life!\r\n" );
      } else if ( delta < 95 ) {
        do_debate_continue( ch, winner, loser, 3,
           "You win a point when @v2 seems distracted!\r\n",
           "You are distracted by a big spot on @a2s nose!\r\n",
           "@v2 makes a good point which @a2 cannot answer!\r\n" );
      } else if ( delta < 110 ) {
        do_debate_continue( ch, winner, loser, 3,
           "You resolute stubbornness wears your opponent down!\r\n",
           "@a2 simply will not listen to reason!\r\n",
           "@a2 sticks to his guns, ignoring @v2s arguments!\r\n" );
      } else if ( delta < 125 ) {
        do_debate_continue( ch, winner, loser, 4,
           "Your masterful arguments convince your opponent!\r\n",
           "@a2 provides you with a remarkable insight!\r\n",
           "@a2 seems to be winning!\r\n" );
      } else if ( delta < 140 ) {
        do_debate_continue( ch, winner, loser, 4,
           "You force @v2 to conceed your main point!\r\n",
           "You are forced to conceed a major issue!\r\n",
           "@v2 demonstrates the poverty of @a2s intellect!\r\n" );
      } else if ( delta < 165 ) {
        do_debate_continue( ch, winner, loser, 5,
           "Your long, detailed explanation seems well received!\r\n",
           "You nod off during one of @a2s monologs!\r\n",
           "@a2 makes a very long and boring speach.\r\n" );
      } else {
        do_debate_continue( ch, winner, loser, 5,
           "Your crafty arguments derail @v2s train of thought!\r\n",
           "You are baffled by @a2s line of reasoning!\r\n",  
           "@v2 runs circles around @a2!\r\n");
      }
  
     /* Has anyone won? */
     

      if ( ch->acv_state < 10 ) {
        do_debate_finish( ch, victim, victim, ch, NULL, NULL, NULL );
     } else if ( ch->acv_state > 40 ) {
        do_debate_finish( ch, victim, ch, victim, NULL, NULL, NULL );
        if (get_skill(ch, ch->acv_int) !=0) {
            check_improve(ch, ch->acv_int, TRUE, 25);
        } else {
            check_improve(ch, ch->acv_int, TRUE, 50);
         }
      } else {

        do_debate_prompt(ch, victim);

        schedule_activity( ch, DEBATE_SPEED, "debate" );
      } 

    }

    return;
}

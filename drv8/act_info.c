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
#include "magic.h"
#include "statdesc.h"
#include "skill.h"
#include "spell.h"
#include "fight.h"
#include "affect.h"
#include "exp.h"
#include "prof.h"
#include "doors.h"
#include "society.h"
#include "profile.h"
#include "mob.h"
#include "race.h"
#include "vlib.h"
#include "cult.h"
#include "bank.h"
#include "partner.h"
#include "gsn.h"
#include "mob.h"
#include "wev.h"

/* command procedures needed */
DECLARE_DO_FUN(do_exits);
DECLARE_DO_FUN( do_look);
DECLARE_DO_FUN( do_read);
DECLARE_DO_FUN( do_smell);
DECLARE_DO_FUN( do_listen);
DECLARE_DO_FUN( do_help);
DECLARE_DO_FUN( do_scan);
DECLARE_DO_FUN( do_copy);
DECLARE_DO_FUN( do_show);
DECLARE_DO_FUN( do_pload);
DECLARE_DO_FUN( do_punload);
DECLARE_DO_FUN( do_play);


char *	const	where_name	[MAX_WEAR] =
{
    "  <used as light>     ",
    "  <worn on finger>    ",
    "  <worn on finger>    ",
    "  <worn around neck>  ",
    "  <worn around neck>  ",
    "  <worn on body>      ",
    "  <worn on head>      ",
    "  <worn on legs>      ",
    "  <worn on feet>      ",
    "  <worn on hands>     ",
    "  <worn on arms>      ",
    "  <worn as shield>    ",
    "  <worn about body>   ",
    "  <worn about waist>  ",
    "  <worn around wrist> ",
    "  <worn around wrist> ",
    "  <wielded>           ",
    "  <held>              ",
    "  <second wield>      ",
    "  <worn with pride>   ",
    "  <worn on face>      ",
    "  <worn on ears>      ",
    "  <floating nearby>   ",
    "  <worn over eyes >   ",
    "  <worn on back>      ",
    "  <tattoo >           "
};

char *	const	month_name	[] =
{
    "January", "February", "March", "April",
    "May", "June", "July", "August", "September",
    "October", "November", "December", 
};


int max_on = 0;

/*
 * Local functions.
 */
char 		*format_obj_to_char		(OBJ_DATA *obj, CHAR_DATA *ch, bool fShort);
void		show_list_to_char		(OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing);
void		show_char_to_char_0		(CHAR_DATA *victim, CHAR_DATA *ch);
void		show_char_to_char_1		(CHAR_DATA *victim, CHAR_DATA *ch, bool glance);
void		show_char_to_char		(CHAR_DATA *list, CHAR_DATA *ch);
bool		check_blind			(CHAR_DATA *ch);
void 		show_view			(CHAR_DATA *ch, int dir);
bool 		read_help_entry			(CHAR_DATA *ch, char *argument);
void 		show_charge			(CHAR_DATA *ch, AFFECT_DATA *af);
void 		meter				(CHAR_DATA *ch, char *name, int value);
void 		looksky 				(CHAR_DATA *ch);
int 		true_length			(char *name, int length);



char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort ) {
static char buf[MAX_STRING_LENGTH];
char vb[MAX_STRING_LENGTH];
int condition = 0;
buf[0] = '\0';

   if (!can_see_obj(ch, obj)) {
      if (fShort) {
          sprintf(buf, "{CA magical glow.{x");
      } else {
          sprintf(buf, "You spot a faint magical glow.");
      }
      return buf;
   }

   /* Add leading attributes... */

    if (!IS_NPC(ch)) {
        if (obj->pIndexData->vnum == ch->pcdata->questobj) {
          strcat( buf, "{r[{RTARGET{r]{x ");
        }
    }

    if (obj->item_type == ITEM_TIMEBOMB) {
      strcat( buf, "{x({Y!DANGER!{x) ");
    }
 
    if ( IS_OBJ_STAT(obj, ITEM_INVIS)) {  
       strcat( buf, "{x({BInvis{x) "     );
    }

    if ( IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
       if (IS_OBJ_STAT(obj, ITEM_EVIL)) strcat( buf, "{x({rRed Aura{x) "  );

       if ((IS_OBJ_STAT(obj, ITEM_NOREMOVE)
           || IS_OBJ_STAT(obj, ITEM_NODROP)
           || IS_OBJ_STAT(obj, ITEM_NOUNCURSE))
       && knows_discipline(ch, DISCIPLINE_CURSES)) {
               strcat( buf, "{x({rCursed{x) "     );
       }
    }

    if ( is_affected(ch, gafn_detect_good)
    && IS_OBJ_STAT(obj, ITEM_BLESS)) {  
      strcat( buf, "{x({gGreen Aura{x) "  );
    }

    if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC)
      && IS_OBJ_STAT(obj, ITEM_MAGIC)) {  
      strcat( buf, "{x({MMagical{x) "   );
    }

    if ( IS_OBJ_STAT(obj, ITEM_CONCEALED)) 	strcat( buf, "{x({yConcealed{x) "   );
    if ( IS_OBJ_STAT(obj, ITEM_VIS_DEATH)) 	strcat( buf, "{x({yVis_on_Death{x) "   );
    if ( IS_OBJ_STAT(obj, ITEM_GLOW)) 		strcat( buf, "{x({YGlowing{x) "   );
    if ( IS_OBJ_STAT(obj, ITEM_HUM)) 		strcat( buf, "{x({YHumming{x) "   );

    if (obj_affected(obj, gafn_obj_blade_affect)) {
        if (check_weapon_affect_at(obj) == WDT_FLAME) strcat( buf, "{x({RF{Yl{Ra{Ym{Ri{Yn{Rg{x) ");
    }

   /* Add appropriate description... */ 

    if ( fShort ) {

      if ( obj->short_descr != NULL ) {
        strcat( buf, obj->short_descr );
      }

    } else {

      if ( obj->description != NULL ) {
        strcat( buf, obj->description );
      }
    }

   /* Add condition if allowed... */ 

    if (!IS_SET(obj->extra_flags, ITEM_NO_COND) 
    && obj->item_type != ITEM_CORPSE_PC
    && obj->item_type != ITEM_CORPSE_NPC
    && buf[0] != '\0') {

      if (obj->condition == 100)
        condition = 0;
      else if (obj->condition > 90)
        condition = 1 ;
      else if (obj->condition > 75)
        condition = 2;
      else if (obj->condition > 50)
        condition = 3;
      else if (obj->condition > 25)
        condition = 4;
      else if (obj->condition > 10)
        condition = 5;
      else if (obj->condition >  0)
 	condition = 6;
      else if (obj->condition == 0)
 	condition = 7;

      if (condition > 0) {	
        strcat(buf, " {x({c");
        strcat(buf, cond_table[condition]);
        strcat(buf, "{x)");
      }
    }

   /* Open/closed */

    if ( (obj->item_type == ITEM_CONTAINER)
    || (obj->item_type == ITEM_KEY_RING)
    || (obj->item_type == ITEM_LOCKER)
    || (obj->item_type == ITEM_CLAN_LOCKER)) {

      if ( (IS_SET(obj->value[1], CONT_CLOSED))
      && (IS_SET(obj->value[1], CONT_CLOSEABLE))) {
        strcat(buf, " {x({yclosed{x)");
      } 
    }

    if (obj->item_type == ITEM_PORTAL
    && obj->value[4] == PORTAL_VEHICLE) {
         if ( (IS_SET(obj->value[3], CONT_CLOSED))
         && (IS_SET(obj->value[3], CONT_CLOSEABLE))) {
               strcat(buf, " {x({yclosed{x)");
         } 
    }

    if (obj->item_type == ITEM_LIGHT) {
         if (obj->value[2] != -1) {
            if (obj->value[2] < 5) {
               strcat(buf, " {x({bexpired{x)");
            } else if (obj->value[3] == TRUE) {
               strcat(buf, " {x({Ylit{x)");
            }
         }
    }

   /* Complain if empty... */

    if (buf[0] == '\0') {
      format_vnum(obj->pIndexData->vnum, vb);
      sprintf(buf,"Object #%s has no description. Please inform the IMP.", vb);
    }

    return buf;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing ) {
char buf[MAX_STRING_LENGTH];
char **prgpstrShow;
int *prgnShow;
char *pstrShow;
OBJ_DATA *obj;
int nShow;
int iShow;
int count;
bool fCombine;

    if ( ch->desc == NULL )	return;

    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content ) count++;
    
    prgpstrShow	= (char **) alloc_mem( count * sizeof(char *) );
    prgnShow    = (int *) alloc_mem( count * sizeof(int)    );
    nShow	= 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != NULL; obj = obj->next_content )    { 
          if ( obj->wear_loc == WEAR_NONE 
          && !IS_SET(obj->extra_flags, ITEM_SCENIC)
          && (ch->trans_obj != obj || !IS_AFFECTED(ch, AFF_MORF))
          && can_see_obj_aura(ch, obj)) {

	    pstrShow = format_obj_to_char( obj, ch, fShort );
	    fCombine = FALSE;

	    if ( IS_NPC(ch) 
                    || IS_SET(ch->comm, COMM_COMBINE) ) {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- ) {
		    if ( !str_cmp( prgpstrShow[iShow], pstrShow ) ) {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine ) {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ ) {
	if ( IS_NPC(ch) 
                || IS_SET(ch->comm, COMM_COMBINE) ) {
	    if ( prgnShow[iShow] != 1 ) {
		sprintf( buf, "(%2d) ", prgnShow[iShow] );
		send_to_char( buf, ch );
	    } else {
		send_to_char( "     ", ch );
	    }
	}
	send_to_char( prgpstrShow[iShow], ch );
	send_to_char( "\r\n", ch );
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 ) {
	if ( IS_NPC(ch) 
                || IS_SET(ch->comm, COMM_COMBINE) ) {
	     send_to_char( "     ", ch );
                } 
	send_to_char( "Nothing.\r\n", ch );
    }

    /*
     * Clean up.
     */
    free_mem( prgpstrShow, count * sizeof(char *) );
    free_mem( prgnShow,    count * sizeof(int)    );

    return;
}

char fright_msg[16][80] = {

  "{g  Ugggh! How disgusting...{x\r\n",
  "{g  How can it live like that?{x\r\n",
  "{g  What on earth is that!{x\r\n",
  "{g  Oh-No!{x\r\n",
  "{g  Yuck!{x\r\n",
  "{g  What? That looks so wierd...{x\r\n",
  "{g  OhMyGod...{x\r\n",
  "{g  Aaarrrrgggghhhh!!!!{x\r\n",
  "{g  You feel icicles down your spine!{x\r\n", 
  "{g  Your hair stands on end!{x\r\n",
  "{g  You have a bad attack of the goosebumps!{x\r\n",
  "{g  A shiver runs down your spine!{x\r\n",
  "{g  You feel full of dread!{x\r\n", 
  "{g  You feel butterflies in your stomach!{x\r\n",
  "{g  You feel very nervous!{x\r\n", 
  "{g  You feel sick inside...{x\r\n"

};


void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch ) {
char buf[MAX_STRING_LENGTH];
char buf2[MAX_STRING_LENGTH];
char msg[MAX_STRING_LENGTH];
int percent, diff;
MOB_CMD_CONTEXT *mcc;
 
    buf[0] = '\0';

    if (IS_AFFECTED(victim, AFF_MIST)) {
          send_to_char("A small cloud if mist is flying here.\r\n", ch);
          return;
    }

   /* Generate leading tags for short look... */ 

    if (!IS_NPC(ch)) {
        if ( IS_NPC(victim) 
        && ch->pcdata->questmob > 0 
        && victim->pIndexData->vnum == ch->pcdata->questmob) {
           strcat( buf, "{r[{RTARGET{r]{x ");
        }

        if ( !IS_NPC(victim) 
        && ch->pcdata->questmob ==-100 
        && !str_cmp(ch->pcdata->questplr, victim->short_descr)) {
           strcat( buf, "{r[{RTARGET{r]{x ");
        }
    }

   /*Migo can sense a brain */

    if ( !str_cmp(race_array[ch->race].name,"mi-go")
      || !str_cmp(race_array[ch->race_orig].name,"mi-go")
      || !str_cmp(race_array[ch->race].name,"mindflayer")
      || !str_cmp(race_array[ch->race_orig].name,"mindflayer")) {
        if ( ( !IS_NPC(victim) 
            && !IS_SET(victim->act, ACT_BRAINSUCKED))
          || get_curr_stat (victim, STAT_INT) > 90) {
             strcat ( buf, "{x({GBrain{x) ");
        }
    }
    
    if (IS_AFFECTED(ch, AFF_DETECT_MAGIC)
    && IS_SET(victim->act, ACT_UNDEAD))
      strcat( buf, "{x({bU{mn{bd{me{ba{md{x) ");

    if ( IS_SET(victim->form, FORM_BLEEDING))  strcat( buf, "{x(Losing {rBlood{x) ");
    if ( IS_AFFECTED(victim, AFF_INVISIBLE))  strcat( buf, "{x({bInvis{x) ");

    if ( !IS_NPC(victim) 
    && IS_SET(victim->plr, PLR_WIZINVIS)) 
      strcat ( buf, "{x({mWizi{x) ");

    if ( !IS_NPC(victim) 
    && IS_SET(victim->plr, PLR_CLOAK)) 
      strcat ( buf, "{x({mCloak{x) ");

    if (str_cmp(race_array[victim->race].name,"machine")
    && !IS_SET(victim->form, FORM_MACHINE)) {
       if ( IS_AFFECTED(victim, AFF_FIRE_SHIELD)) strcat( buf, "{x({RFlames{x) ");
       if ( IS_AFFECTED(victim, AFF_FROST_SHIELD)) strcat( buf, "{x({CFrost{x) ");
       if ( IS_AFFECTED(victim, AFF_LIGHTNING_SHIELD)) strcat( buf, "{x({WElectricity{x) ");
       if ( IS_AFFECTED(victim, AFF_HIDE)) strcat( buf, "{x({bHide{x) ");
       if ( IS_AFFECTED(victim, AFF_CHARM)) strcat( buf, "{x({gCharmed{x) ");
       if ( IS_AFFECTED(victim, AFF_PASS_DOOR)) strcat( buf, "{x({cTranslucent{x) ");
       if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE))  strcat( buf, "{x({rPink Aura{x) ");
       if ( IS_EVIL(victim)) {
             if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) strcat( buf, "{x({rRed Aura{x) " );
             if (IS_AFFECTED(victim, AFF_AURA)) strcat( buf, "{x({mDark Aura{x) " );
       }
       if ( IS_GOOD(victim)) {
            if (is_affected(ch, gafn_detect_good) ) strcat( buf, "{x({gGreen Aura{x) " );
             if (IS_AFFECTED(victim, AFF_AURA)) strcat( buf, "{x({WLight Aura{x) " );
       }
       if ( IS_AFFECTED(victim, AFF_SANCTUARY)) strcat( buf, "{m({wWhite Aura{m){x ");
       if ( IS_AFFECTED(victim, AFF_FLYING))  strcat( buf, "{x({yFlying{x) ");
       if ( IS_AFFECTED(victim, AFF_SNEAK)) strcat( buf, "{x({mSneaking{x) ");
   /* iff */
       if (is_foe(victim, ch)) {
           strcat(buf, "{x({rFOE{x) ");
       } else {
           if (is_friend(victim, ch)) {
               strcat(buf, "{x({gFriend{x) ");
           }
        }
    } else {
          if (victim->leader != NULL
          && victim->ridden != TRUE){
              sprintf (buf2,"{x({r%s{x) ",victim->master->name);
              strcat( buf, buf2);
          }
    }

   /* Calculate health... */

    if (victim->max_hit > 0) percent = victim->hit * 100 / victim->max_hit;
    else percent = -1;
     
    if (percent >= 100) 
      ; //strcat(buf,"{x({gexcellent{x) ");   
    else if (percent >= 85)
      strcat(buf,"{x({yScratches{x) ");
    else if (percent >= 70)
        strcat(buf,"{x({YBruises{x) ");
    else if (percent >= 55)
        strcat(buf,"{x({wCuts{x) ");
    else if (percent >= 40)
        strcat(buf,"{x({WWounds{x) ");
    else if (percent >= 30)
        strcat(buf,"{x({mWounds{x) ");
    else if (percent >= 20)
        strcat(buf,"{x({MBlood Covered{x) ");
    else if (percent >= 10)
        strcat(buf,"{x({rHurt{x) ");
    else if (percent >= 0)
        strcat(buf,"{x({rAwful{x) ");
    else
        strcat(buf,"{x({RBleeding{x) ");

    if (victim->max_move > 0) {
      percent = victim->move * 100 / victim->max_move;
    } else {
       percent = -1;
    }
 
    if (percent >= 90) 
      ; //strcat(buf,"{x({gexcellent{x) ");   
    else if (percent >= 75)
        strcat(buf,"{x({wTired{x) ");
    else if (percent >= 50)
        strcat(buf,"{x({WExhausted{x) ");
    else if (percent >= 30)
        strcat(buf,"{x({mWeak{x) ");
    else if (percent >= 15)
        strcat(buf,"{x({rStaggering{x) ");
    else if (percent >= 0)
        strcat(buf,"{x({BFainting{x) ");
    else
        strcat(buf,"{x({bStunned{x) ");

    if (IS_SET(ch->plr, PLR_AUTOCON)) {
          diff = get_effective_level(victim) - get_effective_level(ch);
          if ( diff <= -10 ) sprintf(msg, "{x({CToo weak{x) ");
          else if ( diff <=  -5 ) sprintf(msg, "{x({cWeak{x) ");
          else if ( diff <=  5 ) sprintf(msg, "{x({WFair opponent{x) ");
          else if ( diff <=  10 ) sprintf(msg, "{x({rStrong{x) ");
          else sprintf(msg, "{x({RToo strong{x) ");
          strcat(buf, msg);
    }

    if ( victim->desc != NULL
      && !victim->desc->ok ) {
      strcat(buf, "{x({clinkdead{x) ");
    }

   /* If it's in its starting position, just send its description... */

    if ( victim->position == victim->start_pos 
      && victim->long_descr[0] != '\0' ) {

      strcat( buf, victim->long_descr );

      send_to_char( buf, ch );

    } else {

     /* Copy PERSMASK (- persons name under mask) over... */ 
  
      strcat( buf, PERSMASK( victim, ch ) );

     /* Add player title... */ 

      if (!IS_NPC(victim)
      && IS_IMMORTAL(victim) 
      && !IS_SET(ch->comm, COMM_BRIEF) 
      && (!IS_AFFECTED(victim, AFF_POLY))) {
  	  strcat( buf, victim->pcdata->title );
      }
 
     /* Position qualifier... */ 

      strcat(buf, " is ");
      pos_text(victim, buf);

     /* Make it look nice... */ 
 
      strcat(buf, "{x\r\n" );

      buf[0] = toupper(buf[0]);

     /* Send the description... */

      send_to_char( buf, ch );
    }

   /* Some things are bad to look at... */

    if ( victim->fright != 0
    && victim->level >= (ch->level - 5)
    && victim->race != ch->race ) {
      mcc = get_mcc(ch, victim, NULL, NULL, NULL, NULL, 0, NULL);
      if (!insanity(mcc, victim->fright, NULL)) send_to_char(fright_msg[number_bits(4)], ch);
      free_mcc(mcc); 
    }

    if (IS_IMMORTAL(victim)
    && !IS_IMMORTAL(ch)
    && IS_AFFECTED(ch, AFF_INCARNATED)) {
      mcc = get_mcc(ch, victim, NULL, NULL, NULL, NULL, 0, NULL);
      if (!insanity(mcc, victim->level, NULL)) send_to_char(fright_msg[number_bits(4)], ch);
      free_mcc(mcc); 
    }

    return;
}



void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch, bool glance) {
    char buf[MAX_STRING_LENGTH];
    char msg[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int iWear;
    int percent, spercent, diff;
    bool found;
    MOB_CMD_CONTEXT *mcc;

    if (!glance) {
      if (ch == victim) {
        if (!IS_AFFECTED(ch, AFF_HIDE)) act( "$n looks at $mself.", ch, NULL, NULL, TO_ROOM);

      } else {

        if (!IS_AFFECTED(ch, AFF_HIDE)) act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
      }
    }
 
   /* Show name and effects... */

    strcpy(buf, "{C");
    strcat(buf, PERSMASK(victim, ch) );
    strcat(buf, "{x");

    buf[2] = toupper(buf[2]);

    if ( IS_AFFECTED(victim, AFF_INVISIBLE))
      strcat( buf, " {x({bInvis{x)");

    if ( !IS_NPC(victim) 
       && IS_SET(victim->plr, PLR_WIZINVIS)) 
      strcat ( buf, " {x({mWizi{x)");

    if ( !IS_NPC(victim) 
       && IS_SET(victim->plr, PLR_CLOAK)) 
      strcat ( buf, " {x({mCloak{x)");

    if ( IS_AFFECTED(victim, AFF_FIRE_SHIELD)) strcat( buf, "{x({RFlames{x) ");
    if ( IS_AFFECTED(victim, AFF_FROST_SHIELD)) strcat( buf, "{x({CFrost{x) ");
    if ( IS_AFFECTED(victim, AFF_LIGHTNING_SHIELD)) strcat( buf, "{x({WElectricity{x) ");
    if ( IS_AFFECTED(victim, AFF_HIDE)) strcat( buf, " {x({bHide{x)");
     if ( IS_AFFECTED(victim, AFF_CHARM)) strcat( buf, " {x({gCharmed{x)");
    if ( IS_AFFECTED(victim, AFF_PASS_DOOR)) strcat( buf, " {x({cTranslucent{x)");
    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE))  strcat( buf, " {x({rPink Aura{x)");
    if ( IS_EVIL(victim)) {
         if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) strcat( buf, " {x({rRed Aura{x)");
         if (IS_AFFECTED(victim, AFF_AURA)) strcat( buf, "{x({mDark Aura{x) " );
    }
    if ( IS_GOOD(victim)) {
         if (is_affected(ch, gafn_detect_good) ) strcat( buf, " {x({gGreen Aura{x)");
         if (IS_AFFECTED(victim, AFF_AURA)) strcat( buf, "{x({WLight Aura{x) " );
    }
    if ( IS_AFFECTED(victim, AFF_SANCTUARY)) strcat( buf, " {m({wWhite Aura{m){x");
    if ( IS_AFFECTED(victim, AFF_FLYING)) strcat( buf, " {x({yFlying{x)");
    if ( IS_AFFECTED(victim, AFF_SNEAK))  strcat( buf, " {x({mSneaking{x)");

   /* iff */

    if (is_foe(victim, ch)) {
      strcat(buf, " {x({rFOE{x)");
    } else {
      if (is_friend(victim, ch)) {
        strcat(buf, " {x({gFriend{x)");
      }
    }

    if (IS_SET(ch->plr, PLR_AUTOCON)) {
          diff = get_effective_level(victim) - get_effective_level(ch);
          if ( diff <= -10 ) sprintf(msg, "{x({CToo weak{x) ");
          else if ( diff <=  -5 ) sprintf(msg, "{x({cWeak{x) ");
          else if ( diff <=  5 ) sprintf(msg, "{x({WFair opponent{x) ");
          else if ( diff <=  10 ) sprintf(msg, "{x({rStrong{x) ");
          else sprintf(msg, "{x({RToo strong{x) ");
          strcat(buf, msg);
    }

   /* Show linkdead status... */

    if (  victim->desc != NULL
      && !victim->desc->ok ) {
      strcat( buf, " is {x({cLinkdead{x)\r\n");
    }
 
    buf[0] = UPPER(buf[0]);
    strcat(buf, "\r\n");
    send_to_char( buf, ch );

   /* Race? */

    if ( victim->race != ch->race ) {
      sprintf(buf, "{yThey are: %s!{x\r\n", race_array[victim->race].name);
      send_to_char(buf, ch);
    }

   /* Show description... */

    if ( victim->description[0] != '\0' ) {
	send_to_char( victim->description, ch );
    } else if (victim->fright > 0) {
        send_to_char("Looks really horrible!\r\n", ch);
    } else {
        send_to_char("Looks quite normal.\r\n", ch);
    }

   /* Some things are bad to look at... */

    if ( victim->fright != 0
      && victim->level >= (ch->level - 5) 
      && victim->race != ch->race ) {
      mcc = get_mcc(ch, victim, NULL, NULL, NULL, NULL, 0, NULL);
      if (!insanity(mcc, victim->fright, NULL)) send_to_char(fright_msg[number_bits(4)], ch);
      free_mcc(mcc); 
    }

    if (IS_IMMORTAL(victim)
    && !IS_IMMORTAL(ch)
    && IS_AFFECTED(ch, AFF_INCARNATED)) {
      mcc = get_mcc(ch, victim, NULL, NULL, NULL, NULL, 0, NULL);
      if (!insanity(mcc, victim->level, NULL)) send_to_char(fright_msg[number_bits(4)], ch);
      free_mcc(mcc); 
    }

   /* Show health... */

    if ( victim->max_hit > 0 ) {
	percent = ( 100 * victim->hit ) / victim->max_hit;
    } else {
	percent = -1;
    } 

    if ( victim->max_move > 0 ) {
	spercent = ( 100 * victim->move ) / victim->max_move;
    } else {
	spercent = -1;
    } 


   /* Summary of interesting bits... */

    sprintf( buf, PERSMASK(victim, ch) );
    buf[0] = toupper(buf[0]);
    strcat( buf, ":\r\n");

    send_to_char(buf, ch);

   /* Show health... */

    if (spercent>percent) {

    if (percent >= 100) 
	send_to_char("   is in {Gexcellent condition{x.\r\n", ch);
    else if (percent >= 85) 
	send_to_char("   has a {yfew scratches.{x\r\n", ch);
    else if (percent >= 70) 
	send_to_char("   has some {Ysmall wounds and bruises{x.\r\n", ch);
    else if (percent >= 55) 
	send_to_char("   has some {wdeep cuts{x.\r\n", ch);
    else if (percent >=  40) 
	send_to_char("   has {Wquite a few wounds{x.\r\n", ch);
    else if (percent >= 30)
	send_to_char("   has some {mbig nasty wounds and scratches{x.\r\n", ch);
    else if (percent >= 20)
	send_to_char("   is {Mcovered with blood{x.\r\n", ch);
    else if (percent >= 10)
	send_to_char("   looks {rpretty hurt{x.\r\n", ch);
    else if (percent >= 0 )
	send_to_char("   is in {Rawful condition{x.\r\n", ch);
    else
	send_to_char("   is {Rbleeding to death{x.\r\n", ch);

    } else {

    if (spercent >= 90) 
	send_to_char("   is in {Gexcellent condition{w.\r\n", ch);
    else if (spercent >= 75) 
	send_to_char("   feels somehow {wtired{w.\r\n", ch);
    else if (spercent >=  50) 
	send_to_char("   is a bit {Wexhausted{w.\r\n", ch);
    else if (spercent >= 30)
	send_to_char("   feels terribly {Mweak{w.\r\n", ch);
    else if (spercent >= 15)
	send_to_char("   is {rstaggering of exhaution{w.\r\n", ch);
    else if (spercent >= 0 )
	send_to_char("   is {Bfainting{w.\r\n", ch);
    else
	send_to_char("   is {bstunned{w.\r\n", ch);

    }


   /* Show activity/position... */

    sprintf(buf, "   is ");
    pos_text(victim, buf);
    strcat(buf, "\r\n");
   
    send_to_char(buf,ch);

   /* Show condition... */

    if (!IS_NPC(victim)) {

     /* Food */

      if ( victim->condition[COND_FOOD] > COND_STUFFED ) {
        send_to_char("   looks well fed.\r\n", ch);
      } else if ( victim->condition[COND_FOOD] <= COND_HUNGRY ) {
        if ( victim->condition[COND_FOOD] > COND_VERY_HUNGRY ) {
          send_to_char("   looks hungry.\r\n", ch);
        } else if ( victim->condition[COND_FOOD] > COND_STARVING ) {
          send_to_char("   looks very hungry!\r\n", ch);
        } else {
          send_to_char("   looks to be starving to death!\r\n", ch);
        }
      }

     /* Drink */

      if ( victim->condition[COND_DRINK] <= COND_THIRSTY ) {
        if ( victim->condition[COND_DRINK] > COND_VERY_THIRSTY ) {
          send_to_char("   looks thirsty.\r\n", ch);
        } else if ( victim->condition[COND_DRINK] > COND_DEHYDRATED ) {
          send_to_char("   looks very thirsty!\r\n", ch);
        } else {
          send_to_char("   looks to be dieing of thirst!\r\n", ch);
        }
      }  

     /* Alcohol */

      if ( victim->condition[COND_DRUNK] > 11 ) {
        sprintf( buf, "   looks to be intoxicated!");
        send_to_char( buf, ch );
      }

      if (is_affected(victim, gafn_hangover)) {
        send_to_char("   looks to be hungover!", ch);
      }

     /* Health */

      if ( victim->condition[COND_FAT] > 10 ) {
        send_to_char("   looks to be overweight!", ch);
      }

     /* Sanity */

      if (get_sanity(victim) < 16) {
        send_to_char("   looks a little deranged!\r\n", ch);
      }
    }
    
   /* That's all for glance... */

    if (glance) return;

    if (IS_AFFECTED(victim, AFF_POLY)
    && (!str_cmp(race_array[victim->race_orig].name,"mi-go")
       || victim->level > 1.5 * ch->level)) return;
    
   /* Show equipment worn... */

    found = FALSE;
    
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ ) {

	obj = get_eq_char( victim, iWear );

	if ( obj != NULL
	  && can_see_obj(ch, obj)) {

	    if ( !found ) {
                          sprintf( buf, PERSMASK(victim, ch) );
                          buf[0] = toupper(buf[0]);
                          strcat( buf, " is using:\r\n");
                          send_to_char(buf, ch);
	          found = TRUE;
	    }

	    send_to_char( where_name[iWear], ch );
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\r\n", ch );
	}
    }

   /* Peek at their inventory... */
 
    if (  victim != ch
      && !IS_NPC(ch) ) {

      if ( IS_IMMORTAL(ch) ) {
        if ( IS_SET(ch->plr, PLR_HOLYLIGHT) ) {
          send_to_char( "\r\nYou peek at the inventory:\r\n", ch );
          check_improve(ch,gsn_peek,TRUE,4);
          show_list_to_char( victim->carrying, ch, TRUE, TRUE );
        }
      } else { 
        if (check_skill(ch, gsn_peek, 0) ) {
          send_to_char( "\r\nYou peek at the inventory:\r\n", ch );
          check_improve(ch,gsn_peek,TRUE,4);
          show_list_to_char( victim->carrying, ch, TRUE, TRUE );
        }
      }
    }

    return;
}


void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch ) {
CHAR_DATA *rch;
CHAR_DATA *mch;
int mnum;

    for ( rch = list; rch != NULL; rch = rch->next_in_room )    {
	if ( rch == ch )    continue;
                if (IS_AFFECTED(rch, AFF_MORF)) continue;
                if ((IS_SET(rch->act, ACT_WATCHER) || IS_SET(rch->plr, PLR_AFK))
                && !IS_IMMORTAL(ch)) continue;


	if ( IS_SET(rch->plr, PLR_WIZINVIS)
                && !IS_IMMORTAL(ch)
                && get_trust( ch ) < rch->invis_level)
                       continue;

                if (IS_AFFECTED(ch,AFF_HALLUCINATING)
                && number_percent()<30) {
                     mnum = number_range(VNUM_NIGHTMARE_LOW, 20169);
                     mch = create_mobile_level(get_mob_index(mnum), NULL, rch->level );
   	     show_char_to_char_0( mch, ch );
                     if ( ch->waking_room != NULL 
                     && ((ch->nightmare && number_percent() <35)
                           || number_percent() < 5)) {
                          send_to_char( "{cYour nightmarish visions suddenly come true!{x\r\n", ch );
                          char_to_room( mch, ch->in_room );
                     } else {
      	          extract_char( mch, TRUE );
                     }
                 } else {
                      if ( can_see( ch, rch ) ) {
	              show_char_to_char_0( rch, ch );
	      }else if ( room_is_dark( ch->in_room )
	        && IS_AFFECTED(rch, AFF_INFRARED ) ) {
	              send_to_char( "You see glowing red eyes watching YOU!\r\n", ch );
	      }
                 }
    }

    return;
} 



bool check_blind( CHAR_DATA *ch ) {

    if ( IS_SET(ch->plr, PLR_HOLYLIGHT) ) {
	return TRUE;
    }

    if ( IS_AFFECTED(ch, AFF_BLIND) ) { 
	send_to_char( "You are blind - can't see a thing!\r\n", ch ); 
	return FALSE; 
    }

    return TRUE;
}


/* changes your scroll */
void do_scroll(CHAR_DATA *ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int lines;

    one_argument(argument,arg);
    
    if (arg[0] == '\0')
    {
	if (ch->lines == 0)
	    send_to_char("You do not page long messages.\r\n",ch);
	else
	{
	    sprintf(buf,"You currently display %d lines per page.\r\n",
		    ch->lines + 2);
	    send_to_char(buf,ch);
	}
	return;
    }

    if (!is_number(arg))
    {
	send_to_char("You must provide a number.\r\n",ch);
	return;
    }

    lines = atoi(arg);

    if (lines == 0)
    {
        send_to_char("Paging disabled.\r\n",ch);
        ch->lines = 0;
        return;
    }

    if (lines < 10 || lines > 100)
    {
	send_to_char("You must provide a reasonable number.\r\n",ch);
	return;
    }

    sprintf(buf,"Scroll set to %d lines.\r\n",lines);
    send_to_char(buf,ch);
    ch->lines = lines - 2;
}

/* RT does socials */
void do_socials(CHAR_DATA *ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    char *cols;
    int iSocial;
    int col;
     
    col = 0;
   
    for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)    {
                switch (social_table[iSocial].s_style) {
	        default:
	         cols = strdup("{W");
	         break;
	        case 1:
	         cols = strdup("{G");
	         break;
	        case 2:
	         cols = strdup("{R");
	         break;
	        case 3:
	         cols = strdup("{M");
	         break;
	}
	sprintf(buf,"%s%-12s{x",cols,social_table[iSocial].name);
	send_to_char(buf,ch);
	if (++col % 6 == 0)
	    send_to_char("\r\n",ch);
    }

    if ( col % 6 != 0) send_to_char("\r\n",ch);
    return;
}



void do_credits( CHAR_DATA *ch, char *argument )
{
    do_help(ch, "diku" );
    return;
}

void do_motd(CHAR_DATA *ch, char *argument) {
    do_help(ch,"motd");
}

void do_imotd(CHAR_DATA *ch, char *argument) {  
    do_help(ch,"imotd");
}

void do_rules(CHAR_DATA *ch, char *argument) {
    do_help(ch,"rules");
}

void do_story(CHAR_DATA *ch, char *argument) {
    do_help(ch,"story");
}

void do_wizlist(CHAR_DATA *ch, char *argument) {
    do_help(ch,"wizlist");
}


void do_autolist(CHAR_DATA *ch, char *argument) {
    if (IS_NPC(ch))   return;

    send_to_char("{C   action     status{x\r\n",ch);
    send_to_char("{C---------------------{x\r\n",ch);
 
    send_to_char("autoassist     (",ch);
    if (IS_SET(ch->plr, PLR_AUTOASSIST)) send_to_char("{CX{x)\r\n",ch);
    else send_to_char(" )\r\n",ch); 

    send_to_char("autoexit       (",ch);
    if (IS_SET(ch->plr, PLR_AUTOEXIT)) send_to_char("{CX{x)\r\n",ch);
    else send_to_char(" )\r\n",ch);

    send_to_char("autogold       (",ch);
    if (IS_SET(ch->plr, PLR_AUTOGOLD)) send_to_char("{CX{x)\r\n",ch);
    else send_to_char(" )\r\n",ch);

    send_to_char("autoloot       (",ch);
    if (IS_SET(ch->plr, PLR_AUTOLOOT)) send_to_char("{CX{x)\r\n",ch);
    else send_to_char(" )\r\n",ch);

    send_to_char("autosac        (",ch);
    if (IS_SET(ch->plr, PLR_AUTOSAC)) send_to_char("{CX{x)\r\n",ch);
    else send_to_char(" )\r\n",ch);

    send_to_char("autosplit      (",ch);
    if (IS_SET(ch->plr, PLR_AUTOSPLIT)) send_to_char("{CX{x)\r\n",ch);
    else send_to_char(" )\r\n",ch);
    
    send_to_char("autosave       (",ch);
    if (IS_SET(ch->plr, PLR_AUTOSAVE)) send_to_char("{CX{x)\r\n",ch);
    else send_to_char(" )\r\n",ch);

    send_to_char("autokill       (",ch);
    if (IS_SET(ch->plr, PLR_AUTOKILL))  send_to_char("{CX{x)\r\n",ch);
    else send_to_char(" )\r\n",ch);

    send_to_char("autoconsider   (",ch);
    if (IS_SET(ch->plr, PLR_AUTOCON)) send_to_char("{CX{x)\r\n",ch);
    else send_to_char(" )\r\n",ch);

    send_to_char("autotax        (",ch);
    if (IS_SET(ch->plr, PLR_AUTOTAX)) send_to_char("{CX{x)\r\n",ch);
    else send_to_char(" )\r\n",ch);

    send_to_char("haggle         (",ch);
    if (IS_SET(ch->plr, PLR_HAGGLE)) send_to_char("{CX{x)\r\n",ch);
    else send_to_char(" )\r\n",ch);

    send_to_char("prompt         (",ch);
    if (IS_SET(ch->comm, COMM_PROMPT))	send_to_char("{CX{x)\r\n",ch);
    else send_to_char(" )\r\n",ch);

    send_to_char("combine items  (",ch);
    if (IS_SET(ch->comm, COMM_COMBINE)) send_to_char("{CX{x)\r\n",ch);
    else send_to_char(" )\r\n",ch);

    send_to_char("\r\n",ch);
    if (!IS_SET(ch->plr, PLR_CANLOOT)) send_to_char("Your corpse is safe from thieves.\r\n",ch);
    else send_to_char("Your corpse may be looted.\r\n",ch);

    if (IS_SET(ch->plr, PLR_NOSUMMON)) send_to_char("You try to resist summoning.\r\n",ch);
    else send_to_char("You can be summoned.\r\n",ch);
   
    if (IS_SET(ch->plr, PLR_NOFOLLOW)) send_to_char("You do not welcome followers.\r\n",ch);
    else send_to_char("You accept followers.\r\n",ch);

    if (IS_SET(ch->plr, PLR_FEED)) send_to_char("You allow others to feed you.\r\n",ch);
    else send_to_char("You don't allow others to feed you.\r\n",ch);

    if (IS_SET(ch->plr, PLR_REASON)) send_to_char("You kill for good reasons.\r\n",ch);
    else send_to_char("You kill randomly.\r\n",ch);

    if (!IS_NPC(ch)) {
        if (ch->pcdata->pk_timer) sprintf_to_char(ch, "{rYou are protected from PK for %d minutes.{x\r\n",ch->pcdata->pk_timer);
    }
}


void do_autoassist(CHAR_DATA *ch, char *argument) {
    if (IS_NPC(ch))  return;
    
    if (IS_SET(ch->plr, PLR_AUTOASSIST))  {
      send_to_char("Autoassist removed.\r\n",ch);
      REMOVE_BIT(ch->plr, PLR_AUTOASSIST);

    } else {
      send_to_char("You will now assist when needed.\r\n",ch);
      SET_BIT(ch->plr, PLR_AUTOASSIST);
    }
}


void do_autoexit(CHAR_DATA *ch, char *argument) {
    if (IS_NPC(ch))  return;
 
    if (IS_SET(ch->plr, PLR_AUTOEXIT)) {
      send_to_char("Exits will no longer be displayed.\r\n",ch);
      REMOVE_BIT(ch->plr, PLR_AUTOEXIT);

    } else {
      send_to_char("Exits will now be displayed.\r\n",ch);
      SET_BIT(ch->plr, PLR_AUTOEXIT);
    }
}


void do_autogold(CHAR_DATA *ch, char *argument) {
    if (IS_NPC(ch)) return;
 
    if (IS_SET(ch->plr, PLR_AUTOGOLD)) {
      send_to_char("Autogold removed.\r\n",ch);
      REMOVE_BIT(ch->plr, PLR_AUTOGOLD);

    } else {
      send_to_char("Automatic gold looting set.\r\n",ch);
      SET_BIT(ch->plr, PLR_AUTOGOLD);
    }
}


void do_autoloot(CHAR_DATA *ch, char *argument) {
    if (IS_NPC(ch)) return;
 
    if (IS_SET(ch->plr, PLR_AUTOLOOT)) {
      send_to_char("Autolooting removed.\r\n",ch);
      REMOVE_BIT(ch->plr, PLR_AUTOLOOT);

    } else {
      send_to_char("Automatic corpse looting set.\r\n",ch);
      SET_BIT(ch->plr, PLR_AUTOLOOT);
    }
}


void do_haggle(CHAR_DATA *ch, char *argument) {
    if (IS_NPC(ch)) return;
 
    if (IS_SET(ch->plr, PLR_HAGGLE)) {
      send_to_char("Haggle removed.\r\n",ch);
      REMOVE_BIT(ch->plr, PLR_HAGGLE);

    } else {
      send_to_char("Haggle set.\r\n",ch);
      SET_BIT(ch->plr, PLR_HAGGLE);
    }
}


void do_autosac(CHAR_DATA *ch, char *argument) {
    if (IS_NPC(ch))  return;
 
    if (IS_SET(ch->plr, PLR_AUTOSAC))    {
      send_to_char("Autosacrificing removed.\r\n",ch);
      REMOVE_BIT(ch->plr, PLR_AUTOSAC);
    }    else    {
      send_to_char("Automatic corpse sacrificing set.\r\n",ch);
      SET_BIT(ch->plr, PLR_AUTOSAC);
    }
}


void do_autotax(CHAR_DATA *ch, char *argument) {
    if (IS_NPC(ch))      return;
 
    if (IS_SET(ch->plr, PLR_AUTOTAX))    {
      send_to_char("Tax display removed.\r\n",ch);
      REMOVE_BIT(ch->plr, PLR_AUTOTAX);
    }    else    {
      send_to_char("Tax display set.\r\n",ch);
      SET_BIT(ch->plr, PLR_AUTOTAX);
    }
}


void do_autosave(CHAR_DATA *ch, char *argument) {
    if (IS_NPC(ch))  return;
 
    if (IS_SET(ch->plr, PLR_AUTOSAVE)) {
      send_to_char("Autosave message will no longer be shown.\r\n",ch);
      REMOVE_BIT(ch->plr, PLR_AUTOSAVE);

    } else  {
      send_to_char("Autosave message turned on.\r\n",ch);
      SET_BIT(ch->plr, PLR_AUTOSAVE);
    }
}


void do_autokill(CHAR_DATA *ch, char *argument) {
    if (IS_NPC(ch))  return;
 
    if (IS_SET(ch->plr, PLR_AUTOKILL))    {
      send_to_char("You will try to knock mobs out.\r\n",ch);
      REMOVE_BIT(ch->plr, PLR_AUTOKILL);

    }    else    {
      if (ch->fighting && !IS_NPC(ch)) {
         if (!IS_NPC(ch->fighting)) {
             int ldif = ch->level - ch->fighting->level + 30;

             if (ldif > 50) {
                 if (!check_sanity(ch, ldif)) send_to_char("You are horrified by your intentions!\r\n", ch);
             }
         }
      }

      send_to_char("You will try to kill mobs.\r\n",ch);
      SET_BIT(ch->plr, PLR_AUTOKILL);
    }
}


void do_righteouskill(CHAR_DATA *ch, char *argument) {
    if (IS_NPC(ch))  return;
 
    if (IS_SET(ch->plr, PLR_REASON))    {
      send_to_char("You will kill randomly.\r\n",ch);
      REMOVE_BIT(ch->plr, PLR_REASON);

    }    else    {
      send_to_char("You will kill for good reasons.\r\n",ch);
      SET_BIT(ch->plr, PLR_REASON);
    }
}


void do_autocon(CHAR_DATA *ch, char *argument) {
    if (IS_NPC(ch))  return;
 
    if (IS_SET(ch->plr, PLR_AUTOCON))    {
      send_to_char("You don't try to consider mobs.\r\n",ch);
      REMOVE_BIT(ch->plr, PLR_AUTOCON);

    }    else    {
      send_to_char("You will now look closely at mobs.\r\n",ch);
      SET_BIT(ch->plr, PLR_AUTOCON);
    }
}


void do_afk (CHAR_DATA *ch, char *argument) {
    if (IS_NPC(ch)) return;

    if (IS_SET(ch->plr, PLR_AFK)) {
      send_to_char("AFK removed.\r\n",ch);
      REMOVE_BIT(ch->plr, PLR_AFK);

    } else {
      if (ch->fighting 
      || ch->activity > 0) {
          send_to_char("Please terminate all activity before you go AFK!\r\n",ch);
          return;
      }

      send_to_char("AFK Set..\r\n",ch);
      SET_BIT(ch->plr, PLR_AFK);
    }

    return;
}
    

void do_autosplit(CHAR_DATA *ch, char *argument) {
    if (IS_NPC(ch))  return;
 
    if (IS_SET(ch->plr, PLR_AUTOSPLIT)) {
      send_to_char("Autosplitting removed.\r\n",ch);
      REMOVE_BIT(ch->plr, PLR_AUTOSPLIT);

    } else {
      send_to_char("Automatic gold splitting set.\r\n",ch);
      SET_BIT(ch->plr, PLR_AUTOSPLIT);
    }
}


void do_brief(CHAR_DATA *ch, char *argument) {
    if (IS_SET(ch->comm,COMM_BRIEF)) {
      send_to_char("Full descriptions activated.\r\n",ch);
      REMOVE_BIT(ch->comm,COMM_BRIEF);

    } else {
      send_to_char("Short descriptions activated.\r\n",ch);
      SET_BIT(ch->comm,COMM_BRIEF);
    }
}


void do_fullfight(CHAR_DATA *ch, char *argument) {
    if (IS_SET(ch->comm,COMM_FULLFIGHT))  {
	send_to_char("Full battle descriptions removed.\r\n",ch);
	REMOVE_BIT(ch->comm,COMM_FULLFIGHT);

    } else {
	send_to_char("Full battle descriptions activated.\r\n",ch);
	SET_BIT(ch->comm,COMM_FULLFIGHT);
    }
}


void do_fullcast(CHAR_DATA *ch, char *argument) {
    if (IS_SET(ch->comm, COMM_FULLCAST))   {
	send_to_char("Full spell casting descriptions removed.\r\n",ch);
	REMOVE_BIT(ch->comm, COMM_FULLCAST);

    } else {
	send_to_char("Full spell casting descriptions activated.\r\n",ch);
	SET_BIT(ch->comm, COMM_FULLCAST);
    }
}


void do_fullweather(CHAR_DATA *ch, char *argument) {
    if (IS_SET(ch->comm, COMM_FULLWEATHER))   {
	send_to_char("Full weather descriptions removed.\r\n",ch);
	REMOVE_BIT(ch->comm, COMM_FULLWEATHER);

    } else  {
	send_to_char("Full weather descriptions activated.\r\n",ch);
	SET_BIT(ch->comm, COMM_FULLWEATHER);
    }
}


void do_compact(CHAR_DATA *ch, char *argument) {
    if (IS_SET(ch->comm,COMM_COMPACT)) {
      send_to_char("Compact mode removed.\r\n",ch);
      REMOVE_BIT(ch->comm,COMM_COMPACT);

    } else {
      send_to_char("Compact mode set.\r\n",ch);
      SET_BIT(ch->comm,COMM_COMPACT);
    }
}


void do_prompt(CHAR_DATA *ch, char *argument) {
char word[MAX_INPUT_LENGTH];	

                if (IS_NPC(ch)) return;

	if (strlen(argument) != 0) {
		one_argument (argument, word);
		if (!str_cmp(word, "default")) {
			free_string (ch->pcdata->prompt);
			ch->pcdata->prompt = strdup ("{G({W%h/%Hhp %m/%Mmn %v/%Vmv{G){x");
			return;
		} else {
			smash_tilde(argument);
			free_string (ch->pcdata->prompt);
			ch->pcdata->prompt = strdup (argument);
			return;
		}
	}
	
	 if (IS_SET(ch->comm,COMM_PROMPT))   {
                      send_to_char("You will no longer see prompts.\r\n",ch);
                      REMOVE_BIT(ch->comm,COMM_PROMPT);
                } else {
                      send_to_char("You will now see prompts.\r\n",ch);
                      SET_BIT(ch->comm,COMM_PROMPT);
                }
                return;
}


void do_autocombine(CHAR_DATA *ch, char *argument){
    if (IS_SET(ch->comm,COMM_COMBINE))    {
      send_to_char("Long inventory selected.\r\n",ch);
      REMOVE_BIT(ch->comm,COMM_COMBINE);
    } else {
      send_to_char("Combined inventory selected.\r\n",ch);
      SET_BIT(ch->comm,COMM_COMBINE);
    }
}


void do_noloot(CHAR_DATA *ch, char *argument) {
    if (IS_NPC(ch)) return;
 
    if (IS_SET(ch->plr, PLR_CANLOOT)) {
      send_to_char("Your corpse is now safe from thieves.\r\n",ch);
      REMOVE_BIT(ch->plr, PLR_CANLOOT);

    } else {
      send_to_char("Your corpse may now be looted.\r\n",ch);
      SET_BIT(ch->plr, PLR_CANLOOT);
    }
}


void do_nofollow(CHAR_DATA *ch, char *argument) {
    if (IS_NPC(ch))  return;
 
    if (IS_SET(ch->plr, PLR_NOFOLLOW)) {
      send_to_char("You now accept followers.\r\n",ch);
      REMOVE_BIT(ch->plr, PLR_NOFOLLOW);

    } else  {
      send_to_char("You no longer accept followers.\r\n",ch);
      SET_BIT(ch->plr, PLR_NOFOLLOW);
      die_follower( ch );
    }
}


void do_nosummon(CHAR_DATA *ch, char *argument) {
    if (IS_NPC(ch)) {
      if (IS_SET(ch->imm_flags,IMM_SUMMON)) {
	send_to_char("You are no longer immune to summon.\r\n",ch);
	REMOVE_BIT(ch->imm_flags,IMM_SUMMON);
      } else   {
	send_to_char("You are now immune to summoning.\r\n",ch);
	SET_BIT(ch->imm_flags,IMM_SUMMON);
      }
    } else {
      if (IS_SET(ch->plr, PLR_NOSUMMON)) {
        send_to_char("You are no longer immune to summon.\r\n",ch);
        REMOVE_BIT(ch->plr, PLR_NOSUMMON);
      } else {
        send_to_char("You are now immune to summoning.\r\n",ch);
        SET_BIT(ch->plr, PLR_NOSUMMON);
      }
    }
}


CHAR_DATA *find_original(CHAR_DATA *ch) {

  if (ch->desc == NULL) return NULL;

  if (ch->desc->original != NULL) return ch->desc->original;

  return ch;
}


int get_weather(ROOM_INDEX_DATA *room) {
int weather;

 /* No weather inside or in the void... */

  if ( room == NULL
    || room->area == NULL
    || IS_SET(room->room_flags, ROOM_INDOORS)) {
    return WEATHER_NONE;
  }

 /* Calculate outside weather... */

  weather = WEATHER_SUNNY_DAY;

  switch (room->sector_type) {

    case SECT_INSIDE:
    case SECT_UNDERGROUND:
    case SECT_UNDERWATER:
    case SECT_SPACE:
    case SECT_SMALL_FIRE:
    case SECT_FIRE:
    case SECT_BIG_FIRE:
      weather = WEATHER_NONE;
      break;  

    case SECT_MOUNTAIN:

      if ( room->area->climate == CLIMATE_EUROPEAN ) {

        if ( weather == WEATHER_RAINING ) {
          weather = WEATHER_SNOWING;
        } 

        if ( weather == WEATHER_STORM ) {
          weather = WEATHER_BLIZZARD;
        } 
      }
  
      break;
 
    default:
      weather = room->area->weather;

      break;
  }

 /* Mist means it can never be sunny... */
 
  if ( weather == WEATHER_SUNNY_DAY 
    && IS_SET(room->room_flags, ROOM_MISTY) ) {
    weather = WEATHER_CLOUDY;
  } 

  if ( weather == WEATHER_SUNNY_DAY
    && weather_info.sunlight == SUN_DARK ) {
    weather = WEATHER_CLEAR_NIGHT;
  } 

  return weather;
}

int get_surface(ROOM_INDEX_DATA *room) {

  int surface;

 /* No surface inside or in the void... */

  if ( room == NULL
    || room->area == NULL
    || IS_SET(room->room_flags, ROOM_INDOORS)) {
    return SURFACE_CLEAR;
  }

 /* Calculate outside weather... */

  surface = room->area->surface;

  if ( surface < SURFACE_CLEAR
    || surface >= SURFACE_MAX ) {
    bug("Bad surface value %d", surface);
    surface = SURFACE_CLEAR;
  } 

  switch (room->sector_type) {

    case SECT_INSIDE:
    case SECT_UNDERGROUND:
    case SECT_UNDERWATER:
    case SECT_SPACE:
    case SECT_SMALL_FIRE:
    case SECT_FIRE:
    case SECT_BIG_FIRE:
    case SECT_AIR:
    case SECT_ACID:
    case SECT_HOLY:
    case SECT_EVIL:
    case SECT_WATER_NOSWIM:
      surface = SURFACE_CLEAR;
      break;  

    case SECT_WATER_SWIM:
    case SECT_SWAMP:

      if ( surface == SURFACE_SNOW
        || surface == SURFACE_DEEP_SNOW ) {
        surface = SURFACE_FROZEN;
      }

      break;

    case SECT_MOUNTAIN:

      if ( room->area->climate == CLIMATE_EUROPEAN ) { 
        if ( surface == SURFACE_CLEAR
          || surface == SURFACE_PUDDLES ) {
          surface = SURFACE_SNOW;
        }
      }
 
      break; 

    case SECT_FOREST:
    case SECT_HILLS:
    
      if ( time_info.season == SEASON_FALL
        && room->area->climate == CLIMATE_EUROPEAN
        && surface == SURFACE_CLEAR ) {
        surface = SURFACE_LEAVES;
      } 
 
    default:
      break;
  }

  return surface;
}

char weather_short[WEATHER_MAX][20] = {
  { "{xnone{x"     },
  { "{ysunny{x"    },
  { "{cclear{x"    },
  { "{xcloudy{x"   },
  { "{braining{x"  },
  { "{mstorm{x"    },
  { "{wsnowing{x"  },
  { "{wblizzard{x" } 
};

char weather_long[WEATHER_MAX][70] = {
  { "There is no weather here.\r\n"                 },
  { "The sun is high in the clear blue sky.\r\n"    },
  { "You can see the stars twinkling overhead.\r\n" },
  { "There are a few clouds in the sky.\r\n"        },
  { "It is pouring with rain.\r\n"                  },
  { "A storm rages all around!\r\n"                 },
  { "Snow is gently falling from the sky.\r\n"      },
  { "A blizzard swirls all around!\r\n"             } 
};

char weather_moon[MOON_MAX][50] = {
  { "There is a new moon tonight.\r\n"            },
  { "A crescent moon hangs in the sky.\r\n"       },
  { "There is a half moon tonight.\r\n"           },
  { "A three quarters moon hangs in the sky.\r\n" },
  { "There is a full moon tonight.\r\n"           }
};

char weather_surface_short[SURFACE_MAX][20] = {
  { "{yclear{x"   },
  { "{bpuddles{x" }, 
  { "{wsnow{x"    },   
  { "{wSNOW{x"    },
  { "{wice{x"     },
  { "{wleaves{x"  }
};

char weather_surface[SURFACE_MAX][50] = {
  { "The ground is clear."                              },
  { "The ground is covered with puddles.\r\n"           },
  { "The ground is covered with snow.\r\n"              },
  { "The ground is covered with deep snow.\r\n"         },
  { "The water is frozen solid.\r\n"                    },
  { "The ground is covered in fallen leaves.\r\n"       }
};


bool investigate_room(CHAR_DATA *ch, char *argument) {
char buf  [MAX_STRING_LENGTH];
CHAR_DATA *orig;
OBJ_INDEX_DATA *portal;
int weather, surface;
bool brief;
char vb[MAX_STRING_LENGTH];	

   /* Players only... */

    if (ch->desc == NULL ) return TRUE;
    
   /* Safety check... */

    if (ch->in_room == NULL) {
      send_to_char("You are lost in the endless void...\r\n", ch);
      return TRUE;
    }  

   /* Find the original... */

    orig = find_original(ch);

   /* Find out what the weather is... */

    weather = get_weather(ch->in_room);
    surface = get_surface(ch->in_room);

   /* Ok, what are we investigating... */

    brief = FALSE;

    if ( IS_SET(ch->comm, COMM_BRIEF) )  brief = TRUE;
    
   /* Immortals may see the room number... */

    if (IS_IMMORTAL(orig)) {
      format_vnum(ch->in_room->vnum, vb);
      sprintf (buf, "{cRoom [%s]{x ", vb);
      send_to_char (buf, ch);
    }

   /* Everyone gets the description and the sector type... */

    send_to_char( "{c", ch ) ;
    send_to_char( ch->in_room->name, ch );
    send_to_char( " {c(", ch); 
    send_to_char( flag_string(sector_name, ch->in_room->sector_type), ch);

   /* Brief or short weather gets the weather... */

    if (  brief 
      || !IS_SET(ch->comm, COMM_FULLWEATHER) ) {
      if (is_outside(ch)) {
        send_to_char( "{c-", ch);
        send_to_char( weather_short[weather], ch); 
        send_to_char( "{c-", ch);
        send_to_char( weather_surface_short[surface], ch);
      }
    }

   /* Finish the title line... */

    send_to_char( "{c){x\r\n", ch );

   /* Send the full room description if not brief... */

    if ( !brief ) {

     /* Send the long description... */

      send_to_char( "  ", ch);

      if ( ch->in_room->description[0] != '\0' ) send_to_char( ch->in_room->description, ch );
      else send_to_char( "This area looks rather undistingushed.\r\n", ch);
      
     /* Send long weather details... */

      if ( IS_SET(ch->comm, COMM_FULLWEATHER) ) {
        if ( weather != WEATHER_NONE 
          && is_outside(ch) ) {
          send_to_char( "  ", ch);
          send_to_char( weather_long[weather], ch );

          if ( surface != SURFACE_CLEAR ) {
            send_to_char( "  ", ch);
            send_to_char( weather_surface[surface], ch );
          }

          if ( ( weather == WEATHER_CLEAR_NIGHT
              || weather == WEATHER_CLOUDY )
            && weather_info.sunlight != SUN_LIGHT ) {
            send_to_char( "  ", ch);
            send_to_char( weather_moon[weather_info.moon], ch );
          }
        }   
      }

     /* Notify of mist... */ 

      if ( IS_SET(ch->in_room->room_flags, ROOM_MISTY) ) send_to_char("  Mist swirls around you.\r\n", ch );
      
     /* Show conditional details... */
      show_extra_cond(ch, ch->in_room->extra_descr, "look room");
    }

   /* Show exits, if requested... */
    if ( IS_SET(orig->plr, PLR_AUTOEXIT) ) do_exits( ch, "auto" );
    
   /* Show the objects that are here... */
    if (ch->in_room->contents != NULL) show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
    
   /* Show the people that are here... */
    show_char_to_char( ch->in_room->people,   ch );

     if (IS_SET(ch->in_room->room_flags, ROOM_VEHICLE)) {
           ROOM_INDEX_DATA *outroom;

           if (ch->in_room->exit[DIR_OUT] == NULL) {
                send_to_char("Bad OUT exit\r\n", ch);
                return TRUE;
           }  
           outroom = ch->in_room->exit[DIR_OUT]->u1.to_room;
           portal = get_obj_index(ch->in_room->vnum);
           send_to_char("\r\n",ch);
           send_to_char("-------------\r\n",ch);
           sprintf(buf, "| Fuel: %3d%%|\r\n", portal->weight);
           send_to_char(buf,ch);
           send_to_char("-------------\r\n",ch);
           send_to_char("{CLooking through the front window you see:{x\r\n\r\n", ch);
           if (ch->in_room == NULL) {
                send_to_char("You are lost in the endless void...\r\n", ch);
                return TRUE;
           }  

           weather = get_weather(outroom);
           surface = get_surface(outroom);

           brief = FALSE;
           if ( IS_SET(ch->comm, COMM_BRIEF) )  brief = TRUE;
  
           if (IS_IMMORTAL(orig)) {
                 format_vnum(outroom->vnum,vb);
                 sprintf (buf, "{cRoom [%s]{x ", vb);
                 send_to_char (buf, ch);
           }

           send_to_char( "{c", ch ) ;
           send_to_char( outroom->name, ch );
           send_to_char( " {c(", ch); 
           send_to_char( flag_string(sector_name, outroom->sector_type), ch);

            if (  brief 
            || !IS_SET(ch->comm, COMM_FULLWEATHER) ) {
                       send_to_char( "{c-", ch);
                       send_to_char( weather_short[weather], ch); 
                       send_to_char( "{c-", ch);
                       send_to_char( weather_surface_short[surface], ch);
            }

            send_to_char( "{c){x\r\n", ch );

            if ( !brief ) {
                  send_to_char( "  ", ch);

                  if (outroom->description[0] != '\0' ) send_to_char(outroom->description, ch );
                  else send_to_char( "This area looks rather undistingushed.\r\n", ch);
      
                  if ( IS_SET(ch->comm, COMM_FULLWEATHER) ) {
                        if ( weather != WEATHER_NONE ) {
                               send_to_char( "  ", ch);
                               send_to_char( weather_long[weather], ch );

                               if ( surface != SURFACE_CLEAR ) {
                                      send_to_char( "  ", ch);
                                      send_to_char( weather_surface[surface], ch );
                               }

                               if ( ( weather == WEATHER_CLEAR_NIGHT
                               || weather == WEATHER_CLOUDY )
                               && weather_info.sunlight != SUN_LIGHT ) {
                                      send_to_char( "  ", ch);
                                      send_to_char( weather_moon[weather_info.moon], ch );
                               }
                        }   
                  }
            }

            if ( IS_SET(outroom->room_flags, ROOM_MISTY) ) send_to_char("  Mist swirls around you.\r\n", ch );
            show_extra_cond(ch, outroom->extra_descr, "look room");
            if ( IS_SET(orig->plr, PLR_AUTOEXIT) ) {
                       extern char * const dir_name[];
                      char buf[MAX_STRING_LENGTH];
                      char buf2[128];
                       EXIT_DATA *pexit;
                       ROOM_INDEX_DATA *dest;
                       bool found;
                       bool Hlight=FALSE;
                       int door;

                  if (IS_SET(orig->plr, PLR_XINFO)) Hlight=TRUE;
                  if (Hlight) strcpy (buf, "Exits:  [ flags ]\r\n");
                  else strcpy( buf, "{c[Exits:");
  
                  found = FALSE;

                  for ( door = 0; door < DIR_MAX; door++ ) {
                         pexit = outroom->exit[door];
                         if ( pexit == NULL ) continue;
                         dest = get_exit_destination(ch, ch->in_room, pexit, TRUE);
                         if (dest == NULL) continue;
                         if (!can_see_room(ch, dest)) continue;
    
                         if  ( Hlight) {
		found = TRUE;

                                sprintf (buf2, "%9s", dir_name[door] );
                                strcat (buf2, "   [ ");

                                 if (IS_SET(pexit->exit_info, EX_ISDOOR)) strcat (buf2, "door ");
                                 if (IS_SET(pexit->exit_info, EX_CLOSED)) strcat (buf2, "closed ");
                                 if (IS_SET(pexit->exit_info, EX_LOCKED)) strcat (buf2, "locked ");
                                 if (IS_SET(pexit->exit_info, EX_PICKPROOF)) strcat (buf2, "pickproof ");
                                 if (IS_SET(pexit->exit_info, EX_HIDDEN)) strcat (buf2, "hidden ");
                                 if (IS_SET(pexit->exit_info, EX_NO_PASS)) strcat (buf2, "no_pass ");
                                 if (IS_SET(pexit->exit_info, EX_ROBUST)
                                 || IS_SET(pexit->exit_info, EX_ARMOURED)) strcat (buf2, "{cre-enforced{x ");
                                 if (IS_SET(pexit->exit_info, EX_WALL)) strcat (buf2, "{ywall{x ");
                                 if (!exit_visible(ch, pexit)) strcat (buf2, "invisible ");
                                 if (IS_RAFFECTED(ch->in_room, RAFF_ENCLOSED)
                                 || IS_RAFFECTED(dest, RAFF_ENCLOSED)) {
                                        strcat (buf2, "{Cforce-field{x ");
                                 }

                                 strcat (buf2, "]\r\n");
                                 strcat (buf, buf2);

                         } else {

                                 if ( exit_visible(ch, pexit)) {

                                         if (IS_SET(pexit->exit_info, EX_WALL)
                                         && IS_SET(pexit->exit_info, EX_CLOSED)) {
                                                    found = FALSE;
                                         } else {
	                                    found = TRUE;
                                                     if ( !IS_SET(pexit->exit_info, EX_ISDOOR)
                                                     || !IS_SET(pexit->exit_info, EX_CLOSED)) {
                                                                        strcat(buf, " {g");
                                                     } else {
                                                                        if (IS_SET(pexit->exit_info, EX_ROBUST)
                                                                        || IS_SET(pexit->exit_info, EX_ARMOURED)) {
                                                                                   strcat(buf, " {c|#|");
                                                                        } else {
                                                                                   strcat(buf, " {y#");
                                                                        }
                                                    }                                 
                                                    strcat( buf, dir_name[door] );
                                         }
                                 }
                         }
                  }

                  if ( !found ) strcat( buf, " none!" );
                  if (Hlight) strcat (buf, "\r\n");
                  else strcat( buf, "{c]{x\r\n" );
                  send_to_char( buf, ch );
            }

            if (outroom->contents != NULL) {
                   show_list_to_char( outroom->contents, ch, FALSE, FALSE );
            }

            show_char_to_char(outroom->people,   ch );
     }
    
   /* All done... */
 
    return TRUE;
}

bool investigate(CHAR_DATA *ch, char *argument) {
char buf  [MAX_STRING_LENGTH];
char arg1 [MAX_INPUT_LENGTH];
char arg2 [MAX_INPUT_LENGTH];
char arg3 [MAX_INPUT_LENGTH];
CHAR_DATA *victim;
CHAR_DATA *mch;
CHAR_DATA *mob;
OBJ_DATA *obj;
ROOM_INDEX_DATA *location;
char *pdesc;
int door, number, count, mnum;
CHAR_DATA *orig;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

   /* Players only... */

    if ( ch->desc == NULL ) 	return TRUE;
    
   /* Find the original... */

    orig = find_original(ch);

   /* Ok, what are we investigating... */

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    number = number_argument(arg1,arg3);
    count = 0;

   /* Check for looking into a container... */

    if ( !str_cmp( arg1, "i" ) || !str_cmp( arg1, "in" ) )    {
	/* 'look in' */

	if ( arg2[0] == '\0' ) 	{
	    send_to_char( "Look in what?\r\n", ch );
	    return TRUE;
	}

	obj = get_obj_here( ch, arg2 );

	if ( obj == NULL ) return FALSE;
	
	switch ( obj->item_type )	{
	default:
	    send_to_char( "That is not a container.\r\n", ch );
	    break;

	case ITEM_DRINK_CON:
	    if ( obj->value[1] <= 0 )    {
		send_to_char( "It is empty.\r\n", ch );
		break;
	    }

	    sprintf( buf, "It %s of %s liquid.\r\n",
		obj->value[1] <=    obj->value[0] / 3
		    ? "contains a little" :
		obj->value[1] <= (2 * obj->value[0])/ 3
		    ? "is half full"    :
		obj->value[1] < obj->value[0]
		    ? "is almost full"  : "is full",
		liq_table[obj->value[2]].liq_color
		);

	    send_to_char( buf, ch );

                    if ( obj->value[3] != 0
                    && IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
                              send_to_char("The liquid seems, somehow, unwholesome.\r\n", ch);
                    }

	    break;

	case ITEM_FOUNTAIN:
	    sprintf( buf, "It's contains %s liquid.\r\n", liq_table[obj->value[0]].liq_color );
	    send_to_char( buf, ch );
                    if ( obj->value[1] != 0
                    && IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
                       send_to_char("The liquid seems, somehow, unwholesome.\r\n", ch);
                    }

	    break;

                case ITEM_PORTAL:
                     act("$n peers intently into $p.",ch,obj,NULL,TO_ROOM);
                     act("You peer into $p.",ch,obj,NULL,TO_CHAR);
   
                     switch (obj->value[4]) {
                         case PORTAL_MIRROR:
                         if (number_percent() > 20) {
                               location = get_room_index(obj->value[0]);
                               send_to_char("Through the shimmering portal you see:\r\n",ch);
                               sprintf(buf,"{w%s{x",location->description);
                               send_to_char(buf,ch);
                               show_list_to_char( location->contents, ch, FALSE, FALSE );
                               show_char_to_char( location->people,   ch );
                         } else {
                               send_to_char("Your mirror immage suddenly comes alive!\r\n", ch); 
                               mob = create_mobile_level( get_mob_index( MOB_VNUM_MIRROR ), NULL, ch->level );
                               mob->affected_by = mob->affected_by | ch->affected_by;
                               mob->max_hit = ch->max_hit;
                               mob->hit = ch->hit;
                               mob->max_move = ch->max_move;
                               mob->move = ch->move;
                               mob->max_mana = ch->max_mana;
                               mob->mana = ch->mana;
                               free_string(mob->name);
                               free_string(mob->short_descr);
                               free_string(mob->long_descr);
                               free_string(mob->description);
                               mob->name = strdup(ch->name);
                               mob->short_descr = strdup(ch->short_descr);
                               mob->long_descr = strdup(ch->long_descr);
                               mob->description = strdup(ch->description);
                              char_to_room( mob, ch->in_room );
                         }
                         break;

                         case PORTAL_MAGIC:
                              location = get_room_index(obj->value[0]);
                              send_to_char("Through the shimmering portal you see:\r\n",ch);
                              sprintf(buf,"{w%s{x",location->description);
                              send_to_char(buf,ch);
                              show_list_to_char( location->contents, ch, FALSE, FALSE );
                              show_char_to_char( location->people,   ch );
                              break;

                    }
                    break;

	case ITEM_CONTAINER:
	case ITEM_KEY_RING:
                case ITEM_LOCKER:
                case ITEM_CLAN_LOCKER:
	    if ( IS_SET(obj->value[1], CONT_CLOSED) )    {
		send_to_char( "It is closed.\r\n", ch );
		break;
	    }

                    if (obj->value[0] > 0) {
                       int full;
                       
                       full = 100 * (obj->value[0] - get_obj_weight(obj)) / obj->value[0];
                       meter(ch, "Capacity", full);
                    }
	    send_to_char("It contains:\r\n", ch);
	    show_list_to_char(obj->contains, ch, TRUE, TRUE );
	    break;

	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
                case ITEM_TREE:
	    send_to_char("It has:\r\n", ch);
	    show_list_to_char( obj->contains, ch, TRUE, TRUE );
	    break;

	case ITEM_JUKEBOX:
	    do_play(ch, "list");
	    break;

                case ITEM_CAMERA:
                     if (obj->contains != NULL) {
                          send_to_char("It contains:\r\n", ch);  
                          sprintf_to_char(ch, "%s {C[%s]{x\r\n", obj->contains->short_descr, flag_string(photo_state, obj->contains->value[0]));
                     }
                     break;
               }
	return TRUE;
    }

   /* Check for a character in the room... */

    victim = get_char_room(ch, arg1);

    if (victim) {
                if (arg2[0] != '\0') {
                      if ((obj = get_obj_list(ch, arg2, victim->carrying)) == NULL) {
                          send_to_char("You don't see that!\r\n", ch);
                          return TRUE;
                      }

                      if (!check_skill(ch, gsn_peek, 0)) {
                          send_to_char("You don't see that!\r\n", ch);
                          return TRUE;
                      }
                    
                      mcc = get_mcc(ch, ch, victim, NULL, obj, NULL, 0, NULL);
                      wev = get_wev(WEV_PEEK, WEV_PEEK_ITEM, mcc,
                      "You have a quick look at @v2's @p2.\r\n",
                      NULL,
                      NULL);

                      if (!room_issue_wev_challange(ch->in_room, wev)) {
                           free_wev(wev);
                           return TRUE;
                      }

                      room_issue_wev( ch->in_room, wev);
                      free_wev(wev);
                      examine_object(ch, obj);
                      return TRUE;
                }

                if (IS_AFFECTED(ch,AFF_HALLUCINATING)
                && number_percent()<30) {
                     mnum = number_range(VNUM_NIGHTMARE_LOW, 20169);
                     mch = create_mobile_level(get_mob_index( mnum ), NULL, victim->level );
  	     show_char_to_char_1( mch, ch, FALSE );
                     if ( ch->waking_room != NULL 
                     && ((ch->nightmare && number_percent() <35)
                           || number_percent() < 5)) {
                          send_to_char( "{cYour nightmarish visions suddenly come true!{x\r\n", ch );
                          char_to_room( mch, ch->in_room );
                     } else {
      	          extract_char( mch, TRUE );
                     }
                 } else {
                     if ( can_see( victim, ch ) ) act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
               	     show_char_to_char_1( victim, ch, FALSE );
                 }
                 return TRUE;
    }

   /* Check for an item in your inventory... */ 

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )    {
	if ( can_see_obj(ch, obj)) {

         /* See if it is the object itself... */ 
 
          if ( obj_called(obj, arg3) ) {
            if (++count == number) {
              examine_object(ch, obj);
              return TRUE;
            }
          }

         /* Check for an extended description on the instance... */

          pdesc = get_extra_descr( arg3, obj->extra_descr );
          if ( pdesc != NULL ) {
            if (++count == number) {
              if (show_extra_cond(ch, obj->extra_descr, arg3)) {
                return TRUE;
              }
            } 
          }

         /* Check for an extended description on the class... */ 

          pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
          if ( pdesc != NULL ) {
            if (++count == number) {
              if (show_extra_cond(ch, obj->pIndexData->extra_descr, arg3)) {
                return TRUE;
              }
            } else {
              continue;
            }
          }
	}
    }

   /* Check for an item in the room... */

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )    {
	if ( can_see_obj(ch, obj)) {

         /* If it's the object, give a full description... */

          if ( obj_called(obj, arg3) ) {
            if (++count == number) {
              examine_object(ch, obj);
              return TRUE;
            }
          }

         /* Check for extra detail on the instance... */

          pdesc = get_extra_descr( arg3, obj->extra_descr );
	  if ( pdesc != NULL ) {
            if (++count == number) {
              if (show_extra_cond(ch, obj->extra_descr, arg3)) {
	        return TRUE;
              } 
	    }
          }
         
         /* Check for extra detail on the class... */ 

          pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
          if ( pdesc != NULL ) {
            if (++count == number) {
              if (show_extra_cond(ch, obj->pIndexData->extra_descr, arg3)) {
                return TRUE;
              }
            }
          }
        }
    }

   /* Warn if failed because count exceeded. */ 
   
    if (count > 0 && count != number) {
    
    	if (count == 1) {
    	    sprintf(buf,"There is only one!\r\n");
    	} else {
    	    sprintf(buf,"There are only %d!\r\n", count);
        }
    	
    	send_to_char(buf,ch);
    	return FALSE;
    }

   /* Check for extra description on the room... */

    if (show_extra_cond(ch, ch->in_room->extra_descr, arg1)) return TRUE;
   
   /* 'look direction' */

    door = door_index(ch, arg1);

   /* -1 means no match in mapper of keywords... */

    if (door == DIR_NONE) 	return FALSE;
    
   /* -2 means no exit, -3 means not visible... */
      
    if (door < DIR_NONE) { 
	send_to_char( "Nothing special there.\r\n", ch );
	return TRUE;
    }

   /* Have a look at the view... */ 

    show_view(ch, door);

    return TRUE;
}

/* Work out if a character can see inside a room... */

bool can_see_in_room(CHAR_DATA *ch, ROOM_INDEX_DATA *room) {

  /* Holylight will show anything... */ 
   if (IS_SET(ch->plr, PLR_HOLYLIGHT)) return TRUE;
   
  /* Blind characters can't see anything... */
   if (!check_blind(ch)) return FALSE;
   
  /* Anyone else can see if the lights are on... */
   if (!room_is_dark(room)) return TRUE;
   
  /* Those with DARKVISION can see in the dark... */
   if ( IS_AFFECTED(ch, AFF_DARK_VISION) ) return TRUE;
   
  /*Being a Vampire might be a good reason to enjoy the dark */
   if (IS_SET(ch->act, ACT_VAMPIRE)) return TRUE;
   
  /* Those with infrared can see in the dark... */ 

   if ( IS_AFFECTED(ch, AFF_INFRARED) ) {

     switch (room->sector_type) {

      /* ...most of the time */

       case SECT_UNDERWATER:
       case SECT_SMALL_FIRE:
       case SECT_FIRE:
       case SECT_BIG_FIRE:
       case SECT_COLD:
       case SECT_SPACE:
         return FALSE;

       default:
         break;
     }

     return TRUE;
   }

  /* Nobody else can see anything... */

   return FALSE;  
 }

/* Have a look around a room... */

void do_look( CHAR_DATA *ch, char *argument ) {
CHAR_DATA *orig;

   /* Players only... */

    if ( ch->desc == NULL ) 	return;
    
   /* Find the original... */

    orig = find_original(ch);

   /* Watch out for the void... */

    if (ch->in_room == NULL) send_to_char("{MYou are drifting in limbo!{x\r\n", ch);
    
   /* Can't do it if you're stunned or worse... */

    if ( ch->position < POS_SLEEPING ) {
	send_to_char( "You can't see anything but stars!\r\n", ch );
	return;
    }

   /* Can't do it if you're sleeping... */

    if ( ch->position == POS_SLEEPING ) {
	send_to_char( "You can't see anything, you're sleeping!\r\n", ch );
	return;
    }

   /* Can't do it if you're blind... */

    if ( !IS_SET(ch->plr, PLR_HOLYLIGHT)
      && !check_blind( ch ) ) return;
    
    if ( !str_cmp(argument, "sky")) {
       looksky(ch);
       return;
    }

   /* Those who can't see, can't see... */
 
    if ( !can_see_in_room(ch, ch->in_room) ) {
	send_to_char( "It is pitch black ... \r\n", ch );
                show_extra_cond(ch, ch->in_room->extra_descr, "room");
	show_char_to_char( ch->in_room->people, ch );
	return;
    }

   /* Adjust for 'sneaky people'... */
 
    if ( !str_cmp(argument, "smell")) {
      do_smell(ch, "");
    }

    if ( !str_cmp(argument, "listen")) {
      do_listen(ch, "");
    }

    if ( !str_cmp(argument, "read")) {
      do_read(ch, "");
    } 

   /* Ok, see what can be seen... */

    if ( argument[0] == '\0'
      || !str_cmp(argument, "auto")
      || !str_cmp(argument, "room")) {

       investigate_room(ch, argument);

    } else {
        if (!investigate(ch, argument)) send_to_char("You do not see that here.\r\n", ch);
    }

    return;
 }


void do_examine( CHAR_DATA *ch, char *argument ) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

   /* Players only... */

    if ( ch->desc == NULL ) return;
    
   /* Chop up the parms... */
    one_argument( argument, arg );

    if ( arg[0] == '\0' ) {
	send_to_char( "Examine what?\r\n", ch );
	return;
    }

    do_look( ch, arg );

    obj = get_obj_here( ch, arg );

    if ( obj != NULL ) {

	switch ( obj->item_type ) {

          default:
	    break;

          case ITEM_DRINK_CON:
          case ITEM_FOUNTAIN:
          case ITEM_CONTAINER:
          case ITEM_KEY_RING:
          case ITEM_LOCKER:
          case ITEM_CLAN_LOCKER:
          case ITEM_CORPSE_NPC:
          case ITEM_CORPSE_PC:
	    sprintf( buf, "in '%s'", arg );
	    do_look( ch, buf );
            break;
	}
    }

    return;
}

void do_glance( CHAR_DATA *ch, char *argument ) {

    CHAR_DATA *victim;
    int weather, surface;
    char buf[MAX_STRING_LENGTH];
    char vb[MAX_STRING_LENGTH];

   /* Players only... */

    if ( ch->desc == NULL ) {
	return;
    }

   /* Find the victim... */

    if ( argument[0] == '\0' ) {
	send_to_char( "Glance at whom?\r\n", ch );
	return;
    }

    if (IS_IMMORTAL(ch)) {
      victim = get_char_world(ch, argument);
    } else {
      victim = get_char_room(ch, argument);
    }

    if (victim == NULL) {
      send_to_char("They aren't here.\r\n", ch);
      return;
    }

   /* A touch more info for remote glance... */

    if ( ch->in_room != victim->in_room ) {

      if ( victim->in_room == NULL ) {
        send_to_char("{YThey are drifting in limbo!\r\n{x", ch);
      } else {

       /* Find out what the weather is... */

        weather = get_weather(victim->in_room);
        surface = get_surface(victim->in_room);

       /* Immortals may see the room number... */

        if (IS_IMMORTAL(ch)) {
          format_vnum(victim->in_room->vnum, vb);
          sprintf (buf, "{cRoom [%s]{x ", vb);
          send_to_char (buf, ch);
        }

       /* Everyone gets the description and the sector type... */

        send_to_char( "{c", ch ) ;
        send_to_char( victim->in_room->name, ch );
        send_to_char( " {c(", ch); 
        send_to_char( flag_string(sector_name, 
                                        victim->in_room->sector_type), ch);

       /* Brief or short weather gets the weather... */

        if (is_outside(ch)) {
          send_to_char( "{c-", ch);
          send_to_char( weather_short[weather], ch); 
          send_to_char( "{c-", ch);
          send_to_char( weather_surface_short[surface], ch);
        }

       /* Finish the title line... */

        send_to_char( "{c){x\r\n", ch );

      }
    }

   /* Show time... */

    show_char_to_char_1(victim, ch, TRUE);

   /* A little superstition... */

    if (ch->in_room != victim->in_room) {
      send_to_char("Your nose itches.\r\n", victim);
    }

   /* All done... */

    return;
}

void do_scry( CHAR_DATA *ch, char *argument ) {

    CHAR_DATA *victim;

    ROOM_INDEX_DATA *save_room;

    OBJ_DATA *obj;

    int skill;
    int mana;

   /* Players only... */

    if ( ch->desc == NULL ) {
	return;
    }

   /* Not from the void... */

    if ( ch->in_room == NULL ) {
      send_to_char("Alas, you cannot scry from within the void!\r\n", ch);
      return;
    }

   /* Check for scrying dish... */

    obj = ch->carrying;

    while ( obj != NULL
         && ( obj->item_type != ITEM_SCRY
           || obj->wear_loc != -1
           || !can_see_obj(ch, obj))) {
      obj = obj->next_content;
    }

    if ( obj == NULL ) {
      obj = ch->in_room->contents;

      while ( obj != NULL
           && ( obj->item_type != ITEM_SCRY
             || !can_see_obj(ch, obj))) {
        obj = obj->next_content;
      }
    }

    if ( obj == NULL ) {
       send_to_char("You have no suitable scrying object!\r\n", ch);
       return;
    }

   /* Find the victim... */

    if ( argument[0] == '\0' ) {
	send_to_char( "Scry whom?\r\n", ch );
	return;
    }

    victim = get_char_world(ch, argument);

    if (victim == NULL) {
      send_to_char("You see only cloulds and mist...\r\n", ch);
      return;
    }

   /* Avoid the void... */

    if ( victim->in_room == NULL ) {
      send_to_char("They are drifting in an endless void!\r\n", ch);
      return;
    }

  if ( IS_IMMORTAL(victim)
  && !IS_IMMORTAL(ch)) { 
      send_to_char("You can see only cloudy Kadath!\r\n", ch);
      return;
    }

   /* Skill and Mana checks for mortals... */

    if ( !IS_IMMORTAL(ch) ) {

      mana = mana_cost(ch, get_spell_spn("power word")) + victim->level;

      skill = get_skill(ch, gsn_scrying);

      if ( victim->in_room->area != ch->in_room->area ) {
        mana = (mana * 120)/100;
        skill -= 20;
      } 

      if ( victim->in_room->area != NULL
        && ch->in_room->area != NULL 
        && ch->in_room->area->zone != victim->in_room->area->zone ) {
        mana *= 2;
        skill -= 50;
      }

      if ( ch->mana < mana ) {
        send_to_char("You feel drained by your efforts!\r\n", ch);
        ch->mana = 0;
        return;
      }

      if ( skill + number_open() < 110 ) {
        send_to_char("You see only a fleeting image of them!\r\n", ch);
        return; 
      }
    }

   /* Show time... */

    save_room = NULL;

    if ( ch->in_room != victim->in_room ) {
      save_room = ch->in_room;
      ch->in_room = victim->in_room;
    }

    investigate_room(ch, "");

    if ( save_room != NULL ) {
      ch->in_room = save_room;
    }

   /* A little superstition... */

    if (ch->in_room != victim->in_room) {
      send_to_char("The hair on the back of your neck stands up!\r\n", victim);
    }

   /* All done... */

    return;
}

void do_read (CHAR_DATA *ch, char *args ) {  

  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];

 /* Players only... */

  if ( ch->desc == NULL ) {
    return;
  }

 /* Get the args... */

  one_argument( args, arg );

  if (arg[0] == '\0') {
    sprintf(buf, "'read room'");
  } else {
    sprintf(buf, "'read %s'", arg);
  }

  do_look(ch, buf);

  return;
}

void do_smell(CHAR_DATA *ch, char *args) {

  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];

 /* Players only... */

  if ( ch->desc == NULL ) {
    return;
  }

 /* Get the args... */ 

  one_argument( args, arg );

 /* Can't do it if you're stunned or worse... */

  if ( ch->position < POS_SLEEPING ) {
    send_to_char( "You can smell death in the air!\r\n", ch );
    return;
  }

 /* What are we smelling? */

  if (arg[0] == '\0') {
    sprintf(buf, "'smell room'");
  } else {
    sprintf(buf, "'smell %s'", arg);
  }

 /* Go smell... */

  if (!investigate(ch, buf)) {

    if (arg[0] == '\0') {
      send_to_char("You smell nothing unusual.\r\n", ch);
    } else {
      send_to_char("You can't smell that!\r\n", ch);
    }
  }    
  
 /* All done... */

  return;
}

void do_listen(CHAR_DATA *ch, char *args) {

  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];

  one_argument( args, arg );

 /* Players only... */

  if ( ch->desc == NULL )
    return;

 /* Can't do it if you're stunned or worse... */

  if ( ch->position < POS_SLEEPING ) {
    send_to_char( "You can hear a scythe being sharpened!\r\n", ch );
    return;
  }

 /* What are we listening to? */ 

  if (arg[0] == '\0') {
    sprintf(buf, "'listen room'");
  } else {
    sprintf(buf, "'listen %s'", arg);
  }

 /* Go listen... */ 

  if (!investigate(ch, buf)) {

    if (arg[0] == '\0') {
      send_to_char("You hear nothing unusual.\r\n", ch);
    } else {
      send_to_char("You can't listen to that!\r\n", ch);
    }
  }

 /* All done... */

  return;
}


void do_show (CHAR_DATA *ch, char *argument) {
char arg1 [MAX_INPUT_LENGTH];
char arg2 [MAX_INPUT_LENGTH];
CHAR_DATA *sch = NULL;
OBJ_DATA *obj;
ROOM_INDEX_DATA *room;
MOB_CMD_CONTEXT *mcc;
WEV *wev;

 /* Corpses don't show things... */

  if ( ch->position < POS_SLEEPING ) {
    send_to_char( "You can't see anything but stars!\r\n", ch );
    return;
  }

 /* Not in your sleep... */ 

  if ( ch->position == POS_SLEEPING ) {
    send_to_char( "You can't do that, you're sleeping!\r\n", ch );
    return;
  }

 /* Nor when you are fighting... */

  if (ch->position == POS_FIGHTING) {
    send_to_char("You're to busy for that now...\r\n", ch);
    return; 
  }

 /* You have to be able to see it to show it... */ 

  if (!check_blind( ch )) return;

 /* Check we're in the known world... */ 

  room = ch->in_room;

  if (room == NULL) { 
    send_to_char("You're lost. VERY lost!\r\n", ch);
    return;
  } 

 /* Need light to find it... */

  if ( !IS_NPC(ch)
    && !IS_SET(ch->plr, PLR_HOLYLIGHT)
    && room_is_dark( ch->in_room ) ) {
    send_to_char( "It is pitch black ... \r\n", ch );
    return;
  }

 /* Now, what is it? */
  
  if (argument[0] == '\0') {
    send_to_char("Syntax: show object\r\n", ch);
    return;
  }

  argument = one_argument( argument, arg1);
  argument = one_argument( argument, arg2);

  if ((obj = get_obj_list(ch, arg1, ch->carrying)) == NULL) {
    send_to_char("You don't have that!\r\n", ch);
    return;
  }

  if (arg2[0] != '\0') {
    sch = get_char_room(ch, arg2); 
  }

    mcc = get_mcc(ch, ch, sch, NULL, obj, NULL, 0, NULL);
    if (sch) {
        if (obj->item_type == ITEM_PASSPORT) {
             wev = get_wev(WEV_SHOW, WEV_SHOW_PASSPORT, mcc,
                    "You show @p2 to @v2.\r\n",
                    "@a2 shows @p2 to you:\r\n",
                    NULL);
        } else {
             wev = get_wev(WEV_SHOW, WEV_SHOW_ITEM, mcc,
                    "You show @p2 to @v2.\r\n",
                    "@a2 shows @p2 to you:\r\n",
                    NULL);
        }
    } else {
        if (obj->item_type == ITEM_PASSPORT) {
             wev = get_wev(WEV_SHOW, WEV_SHOW_ITEM, mcc,
                    "You show @p2 to everyone.\r\n",
                    NULL,
                    "@a2 shows @p2 to you:\r\n");
        } else {
             wev = get_wev(WEV_SHOW, WEV_SHOW_PASSPORT, mcc,
                    "You show @p2 to everyone.\r\n",
                    NULL,
                    "@a2 shows @p2 to you:\r\n");
        }
    }

    if (!room_issue_wev_challange(ch->in_room, wev)) {
      free_wev(wev);
      return;
    }

    room_issue_wev( ch->in_room, wev);
    free_wev(wev);
    return;
}


/* Look at an objects URL... */
void do_view(CHAR_DATA *ch, char *args) {
    CHAR_DATA		*victim;
    OBJ_DATA		*obj;
    ROOM_INDEX_DATA	*room;
    char buf[MAX_STRING_LENGTH];

   /* No args, means the rooms view... */
 
    if ( args[0] == '\0' ) {
      
      room = ch->in_room;

      if ( room != NULL ) {
        if ( room->image == NULL ) {
          send_to_char("No image URL for this room.\r\n", ch);
        } else {  
          sprintf(buf, "<IMG SRC=%s ALT=%s>",
                       room->image,
                       room->name);
          send_to_char(buf, ch);
        }
      } else {
        send_to_char("Your mind cannot encompass the void!\r\n", ch);
      }

      return; 
    }  

   /* Another mob? */

    victim = get_char_room(ch, args);

    if ( victim != NULL ) {

      if ( victim->pcdata != NULL ) {
        if ( victim->pcdata->image != NULL ) {
          sprintf(buf, "<IMG SRC=%s ALT=%s>",
                       victim->pcdata->image,
                       victim->short_descr);
          send_to_char(buf, ch);
        } else {
          send_to_char("They do not have an image URL.\r\n", ch);
        }
      } else {  
        if ( victim->pIndexData != NULL ) {
          if ( victim->pIndexData->image != NULL ) {
            sprintf(buf, "<IMG SRC=%s ALT=%s>",
                         victim->pIndexData->image,
                         victim->short_descr);
            send_to_char(buf, ch);
          } else {
            send_to_char("They do not have an image URL.\r\n", ch);
          }
        } else {
          send_to_char("Such terrible depravity!\r\n", ch);
        }
      } 

      return;
    }

   /* An object? */

    obj = get_obj_here(ch, args);

    if ( obj == NULL ) {
      obj = get_obj_carry(ch, args);

      if ( obj == NULL ) {
        obj = get_obj_wear(ch, args);
      }
    }

    if ( obj != NULL ) {
      if ( obj->pIndexData != NULL ) {
        if ( obj->pIndexData->image != NULL ) {
          sprintf(buf, "<IMG SRC=%s ALT=%s>", obj->pIndexData->image, obj->short_descr);
          send_to_char(buf, ch);
        } else {
          send_to_char("It does not have an image URL.\r\n", ch);
        }
      } else {
        send_to_char("Such terrible depravity!\r\n", ch);
      }

      return;
    }

    send_to_char("Nothing by that name here.\r\n", ch);
    return;
}


/* Let a character look at an object.. */
/* It is assumed that the character can see the object. */

void examine_object( CHAR_DATA *ch, OBJ_DATA *obj) {
CHAR_DATA       *mob;
ROOM_INDEX_DATA *location;
char oname[MAX_INPUT_LENGTH];
static char buf[MAX_STRING_LENGTH];
int skill, sn;
char *lastarg;

  one_argument(obj->name, oname);

  if (oname[0] == '\0') return;
  
 /* First send the short description... */

  send_to_char(obj->description, ch );
  send_to_char("\r\n", ch);

 /* Show the extra description on object instance... */
 	
  show_extra_cond(ch, obj->extra_descr, oname);
  
 /* Show the extra description on object class... */ 

  show_extra_cond(ch, obj->pIndexData->extra_descr, oname);



  if (obj->item_type == ITEM_BOOK) {
    sn = obj->value[0];

    if (validSkill(sn)) {
      skill = get_skill(ch, sn);
      skill *= obj->condition;
      skill /= 100;

      if (skill < 10) {
        send_to_char("  It is written in a strange language...\r\n", ch);
      } else {
        sprintf_to_char(ch, "  The book is written in %s\r\n", skill_array[sn].name);
        if (obj->value[1] > 0) send_to_char("  You can 'study' it if you wish.\r\n", ch);

        if (skill < 50) {    
          show_extra_cond(ch, obj->extra_descr, "read_poor");
          show_extra_cond(ch, obj->pIndexData->extra_descr, "read_poor");
        } else {
          show_extra_cond(ch, obj->extra_descr, "read");
          show_extra_cond(ch, obj->pIndexData->extra_descr, "read");
        }
      }
    } 
  }

  if (obj->item_type == ITEM_LIGHT) {
        if (obj->value[1] > 0) sprintf_to_char(ch,"It's %s powered.\r\n", flag_string(refill_type, obj->value[1]));
        sprintf_to_char(ch,"It's currently %s.\r\n", obj->value[3] == 0 ? "dark" : "lit" );
  }

  if (obj->item_type == ITEM_PASSPORT) {
        AFFECT_DATA *af;

        if (obj->owner) sprintf_to_char(ch,"This %s document belongs to %s.\r\n", flag_string(pass_type, obj->value[0]), capitalize(obj->owner));
        if (obj->affected) {
            send_to_char("Crime record:\r\n",ch);
            for (af = obj->affected; af; af = af->next) {
                  if (af->location == APPLY_CRIME_RECORD) {
                       sprintf_to_char(ch, "    %s\r\n", flag_string(crime_type, af->type));
                  }
            }
        }
  }

  if (obj->item_type == ITEM_INSTRUMENT) {
        sprintf_to_char(ch, "Type: %s.\r\n", flag_string(instrument_type, obj->value[0]));
  }

  if (obj->item_type == ITEM_WEAPON) {

    send_to_char("  Weapon skill required: ",ch);

    switch (obj->value[0]) {

      default: 
        send_to_char("unknown\r\n",ch);	
        break;

      case(WEAPON_EXOTIC): 
        send_to_char("exotic\r\n",ch);	
        break;

      case(WEAPON_SWORD): 
        send_to_char("sword\r\n",ch);	
        break;	

      case(WEAPON_DAGGER): 
        send_to_char("dagger\r\n",ch);	
        break;

      case(WEAPON_SPEAR): 
        send_to_char("spear\r\n",ch);
  	break;

      case(WEAPON_MACE): 
        send_to_char("mace\r\n",ch);
        break;

      case(WEAPON_AXE): 
        send_to_char("axe\r\n",ch);
        break;

      case(WEAPON_FLAIL): 
        send_to_char("flail\r\n",ch);
        break;

      case(WEAPON_WHIP): 
        send_to_char("whip\r\n",ch);		
        break;

      case(WEAPON_POLEARM): 
        send_to_char("polearm\r\n",ch);	
        break;

      case(WEAPON_GUN): 
        send_to_char("gun\r\n", ch);
        break;

      case(WEAPON_BOW): 
        send_to_char("bow\r\n", ch);
        break;

      case(WEAPON_HANDGUN): 
        send_to_char("handgun\r\n", ch);
        break;

      case(WEAPON_SMG): 
        send_to_char("submachinegun\r\n", ch);
        break;
    }

    if (IS_SET(obj->wear_flags, ITEM_WIELD)) {
      send_to_char("  You can wield it as a weapon.\r\n", ch);  
    }

    if (IS_SET(obj->wear_flags, ITEM_HOLD)) {
      send_to_char("  You can hold it.\r\n", ch);  
    }

    if (IS_SET(obj->wear_flags, ITEM_TWO_HANDS)) {
      send_to_char("  You need to use both hands to wield it.\r\n", ch);  
    }
  }

  if ( obj->item_type == ITEM_TATTOO
  && IS_SET(obj->wear_flags, ITEM_WEAR_TATTOO)) send_to_char("  You can wear it as a tattoo.\r\n", ch);  
   
 /* Armor and clothes show if it can be worn... */ 

  if ( obj->item_type == ITEM_ARMOR
    || obj->item_type == ITEM_CLOTHING
    || obj->item_type == ITEM_TREASURE
    || obj->item_type == ITEM_PRIDE
    || obj->item_type == ITEM_JEWELRY
    || obj->item_type == ITEM_GEM) {

    if (IS_SET(obj->wear_flags, ITEM_WEAR_FINGER)) send_to_char("  You can wear it on your finger.\r\n", ch);  
    if (IS_SET(obj->wear_flags, ITEM_WEAR_NECK)) send_to_char("  You can wear it around your neck.\r\n", ch);  
    if (IS_SET(obj->wear_flags, ITEM_WEAR_BODY)) send_to_char("  You can wear it over your body.\r\n", ch);  
    if (IS_SET(obj->wear_flags, ITEM_WEAR_HEAD)) send_to_char("  You can wear it on your head.\r\n", ch);  
    if (IS_SET(obj->wear_flags, ITEM_WEAR_LEGS)) send_to_char("  You can wear them on your legs.\r\n", ch);  
    if (IS_SET(obj->wear_flags, ITEM_WEAR_FEET)) send_to_char("  You can wear them on your feet.\r\n", ch);  
    if (IS_SET(obj->wear_flags, ITEM_WEAR_HANDS)) send_to_char("  You can wear them on your hands.\r\n", ch);  
    if (IS_SET(obj->wear_flags, ITEM_WEAR_ARMS)) send_to_char("  You can wear them on your arms.\r\n", ch);  
    if (IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD)) send_to_char("  You can wear it as a shield.\r\n", ch);  
    if (IS_SET(obj->wear_flags, ITEM_WEAR_ABOUT)) send_to_char("  You can wear it somewhere about your body.\r\n", ch);  
    if (IS_SET(obj->wear_flags, ITEM_WEAR_WAIST)) send_to_char("  You can wear it around your waist.\r\n", ch);  
    if (IS_SET(obj->wear_flags, ITEM_WEAR_WRIST)) send_to_char("  You can wear it around you wrist.\r\n", ch);  
    if (IS_SET(obj->wear_flags, ITEM_WEAR_PRIDE)) send_to_char("  You can wear it with pride.\r\n", ch);  
    if (IS_SET(obj->wear_flags, ITEM_WEAR_FACE)) send_to_char("  You can wear it over your face.\r\n", ch);  
    if (IS_SET(obj->wear_flags, ITEM_WEAR_EARS)) send_to_char("  You can wear hanging from your ear.\r\n", ch);  
    if (IS_SET(obj->wear_flags, ITEM_WEAR_FLOAT)) send_to_char("  You can wear it floating around your head.\r\n", ch);  
    if (IS_SET(obj->wear_flags, ITEM_WEAR_EYES)) send_to_char("  You can wear it over your eyes.\r\n", ch);  
    if (IS_SET(obj->wear_flags, ITEM_WEAR_BACK)) send_to_char("  You can wear it on your back.\r\n", ch);  
  }  

 /* A hint ... */

  if (obj->item_type == ITEM_IDOL) send_to_char("  You can pray to it and offer it things.\r\n", ch); 
  if (obj->item_type == ITEM_SCRY) send_to_char("  You can use it for scrying.\r\n", ch); 
  if (obj->item_type == ITEM_GRIMOIRE) send_to_char("  You can use it to write down disciplines of magic.\r\n", ch); 
  if (obj->item_type == ITEM_TREE
  && obj->value[0] == 0) send_to_char("  You can plant it in the ground.\r\n", ch); 
  
 /* Format the special attributes... */

  buf[0] = '\0';

  strcat(buf, "  Attributes: ");

  if ( IS_OBJ_STAT(obj, ITEM_INVIS)) {  
     strcat( buf, "{x({BInvis{x) "     );
  }

  if ( IS_AFFECTED(ch, AFF_DETECT_EVIL)
    && IS_OBJ_STAT(obj, ITEM_EVIL)) {  
    strcat( buf, "{x({rRed Aura{x) "  );
  }

  if ( is_affected(ch, gafn_detect_good)
  && IS_OBJ_STAT(obj, ITEM_BLESS)) {  
    strcat( buf, "{x({gGreen Aura{x) "  );
  }

  if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC)
    && IS_OBJ_STAT(obj, ITEM_MAGIC)) {  
    strcat( buf, "{x({MMagical{x) "   );
  }

  if ( IS_OBJ_STAT(obj, ITEM_GLOW)) {  
    strcat( buf, "{x({YGlowing{x) "   );
  }

  if ( IS_OBJ_STAT(obj, ITEM_HUM)) {  
    strcat( buf, "{x({YHumming{x) "   );
  }

 /* Send the extra attributes... */

  if (buf[14] != '\0') { 
    strcat(buf, "\r\n"); 
    send_to_char(buf, ch); 
  }

 /* Now show brands... */

  if (obj->item_type == ITEM_WEAPON) {

    if (IS_SET(obj->value[4], WEAPON_FLAMING)) {
      send_to_char("  It has a {RFlaming{x brand upon it!\r\n", ch);
    }

    if (IS_SET(obj->value[4], WEAPON_FROST)) {
      send_to_char("  It has a {BIcy{x brand upon it!\r\n", ch);
    }

    if (IS_SET(obj->value[4], WEAPON_ACID)) {
      send_to_char("  It has a {GAcidic{x brand upon it!\r\n", ch);
    }

    if (IS_SET(obj->value[4], WEAPON_LIGHTNING)) {
      send_to_char("  It has a {CLightning{x brand upon it!\r\n", ch);
    }

    if (IS_SET(obj->value[4], WEAPON_SHARP)) {
      send_to_char("  It seems unusually sharp!\r\n", ch);
    }

    if (IS_SET(obj->value[4], WEAPON_VORPAL)) {
      send_to_char("  It seems incredibly sharp!\r\n", ch);
    }

    if (IS_SET(obj->value[4], WEAPON_VAMPIRIC)) {
      send_to_char("  It feels hungry!\r\n", ch);
    }

    if (IS_SET(obj->value[4], WEAPON_POISON)) {
      send_to_char("  The blade is covered in sticky poison!\r\n", ch);
    }

    if (IS_SET(obj->value[4], WEAPON_PLAGUE)) {
      send_to_char("  It feels {ginfested{g!\r\n", ch);
    }

  }

 /* Next show the condition... */

  show_obj_cond(ch, obj);

  if (IS_AFFECTED(ch, AFF_DETECT_MAGIC)) {
       AFFECT_DATA *af;

       if (!obj->enchanted) {
             for (af = obj->pIndexData->affected; af != NULL; af = af->next )   {
                    show_charge(ch, af);
             }
       } else {
             for (af = obj->affected; af != NULL; af = af->next )   {
                    show_charge(ch, af);
             }
       }
  }


 /* Now, for guns, we show its state... */

  if ( (obj->item_type == ITEM_WEAPON) 
    && ( (obj->value[0] == WEAPON_HANDGUN) 
      || (obj->value[0] == WEAPON_GUN)
      || (obj->value[0] == WEAPON_BOW)
      || (obj->value[0] == WEAPON_SMG))
    && (obj->carried_by == ch)) {

    if (obj->wear_loc == WEAR_WIELD) {
      switch (ch->gun_state[0]) {
        case GUN_JAMMED:
          sprintf_to_char(ch, "  Status: {rJammed{x\r\n");
          break;
        case GUN_NO_AMMO:
          sprintf_to_char(ch, "  Status: {yNo ammunition{x\r\n");
          break;
        case 1:
          sprintf_to_char(ch, "  Status: {c1 shot{x\r\n");
          break; 
        default:
          sprintf_to_char(ch, "  Status: {c%d shots{x\r\n", ch->gun_state[0]);
          break; 
      }  
    }

    if (obj->wear_loc == WEAR_WIELD2) {
      switch (ch->gun_state[1]) {
        case GUN_JAMMED:
          sprintf_to_char(ch, "  Status: {rJammed{x\r\n");
          break;
        case GUN_NO_AMMO:
          sprintf_to_char(ch, "  Status: {yNo ammunition{x\r\n");
          break;
        case 1:
          sprintf_to_char(ch, "  Status: {c1 shot{x\r\n");
          break; 
        default:
          sprintf_to_char(ch, "  Status: {c%d shots{x\r\n", ch->gun_state[1]);
          break; 
      }  
    }
  }

  if (obj->item_type == ITEM_PIPE) {
       if (obj->value[2] !=0
       && obj->value[3] != 0) {
             send_to_char("It's ready to smoke.\r\n", ch); 
       } else {
             obj->value[2]=0;
             obj->value[3]=0;
       }
  }

  if (obj->item_type == ITEM_CAMERA) {
       if (obj->contains != NULL) {
           send_to_char("It contains:\r\n", ch);  
           sprintf_to_char(ch, "%s {C[%s]{x\r\n", obj->contains->short_descr, flag_string(photo_state, obj->contains->value[0]));
       }
  }

  if (obj->item_type == ITEM_SHARE) {
        SHARE_DATA *share = get_share_by_id(obj->value[0]);
    
        sprintf_to_char(ch, "%d shares of %s.\r\n", obj->value[1], share->name);
  }


  if (obj->item_type == ITEM_JUKEBOX) {
      do_play(ch, "list");
  }

  if (obj->item_type == ITEM_EXPLOSIVE) {
       if (obj->value[3] !=0
       || obj->value[4] != 0) {
             sprintf_to_char(ch, "  Timer range from %ld to %ld\r\n", obj->value[3], obj->value[4]);
       }
  }

  if (obj->item_type == ITEM_TIMEBOMB) {
             sprintf_to_char(ch, "  Timer at %d\r\n", obj->timer);
  }

  if (obj->item_type == ITEM_GRIMOIRE) {
             send_to_char("\r\n", ch);
  }

  if (obj->item_type == ITEM_PORTAL) {
     act("$n peers intently into $p.",ch,obj,NULL,TO_ROOM);
     act("You peer into $p.",ch,obj,NULL,TO_CHAR);
   
    switch (obj->value[4]) {
     case PORTAL_MIRROR:
          if (number_percent() > 20) {
                location = get_room_index(obj->value[0]);
                send_to_char("Through the shimmering portal you see:\r\n",ch);
                sprintf(buf,"{w%s{x",location->description);
                send_to_char(buf,ch);
                show_list_to_char( location->contents, ch, FALSE, FALSE );
                show_char_to_char( location->people,   ch );
          } else {
                 send_to_char("Your mirror immage suddenly comes alive!\r\n", ch); 
                 mob = create_mobile_level( get_mob_index( MOB_VNUM_MIRROR ), NULL, ch->level );
                 mob->affected_by = mob->affected_by | ch->affected_by;
                 mob->max_hit = ch->max_hit;
                 mob->hit = ch->hit;
                 mob->max_move = ch->max_move;
                 mob->move = ch->move;
                 mob->max_mana = ch->max_mana;
                 mob->mana = ch->mana;
                 free_string(mob->name);
                 free_string(mob->short_descr);
                 free_string(mob->long_descr);
                 free_string(mob->description);
                 mob->name = strdup(ch->name);
                 mob->short_descr = strdup(ch->short_descr);
                 mob->long_descr = strdup(ch->long_descr);
                 mob->description = strdup(ch->description);
                 char_to_room( mob, ch->in_room );
          }
          break;

     case PORTAL_MAGIC:
          location = get_room_index(obj->value[0]);
          send_to_char("Through the shimmering portal you see:\r\n",ch);
          sprintf_to_char(ch,"{w%s{x",location->description);
          show_list_to_char( location->contents, ch, FALSE, FALSE );
          show_char_to_char( location->people,   ch );
          break;

     case PORTAL_VEHICLE:
          lastarg = check_obj_owner(obj);
          if (str_cmp(lastarg, "not found")) {
                sprintf_to_char(ch,"{wThis is %s's vehicle.{x", lastarg);
          }
          break;

     }
  }

  return;
} 

void do_scan (CHAR_DATA *ch, char *argument) {
	
  int door;

 /* Here first... */

  show_view(ch, DIR_HERE);

 /* ...then each direction... */
	
  for (door = 0; door < DIR_MAX; door++) {
    show_view(ch, door);
  }		
	
  return;
}

/* Descriptions for the directions... */

static const char doors[][17] = { 
   "{c[{wNorth {c]{x", 
   "{c[{wEast  {c]{x",
   "{c[{wSouth {c]{x",
   "{c[{wWest  {c]{x",
   "{c[{wUp    {c]{x",
   "{c[{wDown  {c]{x",
   "{c[{wN.East{c]{x",
   "{c[{wS.East{c]{x",
   "{c[{wS.West{c]{x",
   "{c[{wN.West{c]{x",
   "{c[{wIn    {c]{x",
   "{c[{wOut   {c]{x",
   "{c[{wHere  {c]{x"
}; 

void show_view(CHAR_DATA *ch, int dir) {
   ROOM_INDEX_DATA *room;
   ROOM_INDEX_DATA *to_room;
   EXIT_DATA *pexit;
   CHAR_DATA *rch;
   char buf[MAX_STRING_LENGTH];
   char doorbuf[MAX_STRING_LENGTH];
   char door_name[MAX_STRING_LENGTH];

   bool vis = TRUE;
   bool occupied =  FALSE;
   bool dark = FALSE;
   bool to_dark = FALSE;

   int i;
   int condition = 0;

  /* Are we looking somewhere or here?... */

   if ( dir < 0 || dir > DIR_MAX) {

     send_to_char("Swirling cloulds of smoke obscure your view!", ch);
     bug("show_view called for direction %d", (long) dir); 

     return;
   }   

  /* Find the room we're dealing with... */

   room = ch->in_room;

   if (room == NULL) {
     
     sprintf(buf, "%s {WEndless Void{x\r\n"
                          "  The endless void stretches beyond infinity...\r\n", doors[dir]);
     send_to_char(buf, ch);
     return;
   } 

  /* What's the light like in here? */

   dark = !can_see_in_room(ch, room);

  /* Find the exit... */
 
   if (dir != DIR_HERE) {
     pexit = ch->in_room->exit[dir];

     if (pexit == NULL) return;
    
    /* If it's invisible we don't show anything... */

     if (!exit_visible(ch, pexit)
     || (IS_SET(pexit->exit_info, EX_WALL)
     && IS_SET(pexit->exit_info, EX_CLOSED))) {
       send_to_char( "Nothing special there.\r\n", ch );
       return;
     }

    /* Does it go anywhere interesting... */ 

     to_room = get_exit_destination(ch, ch->in_room, pexit, TRUE);

     if (to_room == NULL) {
       return;
     }

    /* See if the destination exists in our view of the world... */

     if (!can_see_room(ch, to_room)) {
       return;
     }

    /* What's the light like in there? */

     to_dark = !can_see_in_room(ch, to_room);

    /* If the door is closed... */

     if (IS_SET(pexit->exit_info, EX_CLOSED)) {
     
      /* If this room is dark we don't show anything... */
 
       if (dark)  return;
       
      /* If we're inside or underground we can't see through it... */ 

       if ( (room->sector_type == SECT_INSIDE) 
         || (room->sector_type == SECT_UNDERGROUND)) {
         vis = FALSE;
         to_dark = TRUE;
       }
     } 

   } else {

    /* No exit if we're looking here... */

     pexit = NULL;
     to_room = room;
 
     if (dark) vis = FALSE;
     to_dark = dark;
   }

  /* Build up and send the header... */

   if (vis) {
     sprintf(buf, "%s {c%s (%s{c){x\r\n", doors[dir], to_room->name,  flag_string(sector_name, to_room->sector_type));
   } else {
     sprintf(buf, "%s\r\n", doors[dir]);
   }

   send_to_char(buf, ch);   

  /* Send over the exit stuff... */

   if ( pexit != NULL 
     && ( !dark 
       || !to_dark)) {

    /* Work out what it's called... */

     if ( (pexit->keyword != NULL)
       && (pexit->keyword[0] != '\0')) {

       strcpy(door_name, pexit->keyword);

       for(i = 0; door_name[i] != '\0';i++) {
         if (door_name[i] == ' ') {
           door_name[i] = '\0';
         }
       }   
     } else {
       strcpy(door_name, "door");
     }

    /* Send the exit description, if there is one... */ 
  
     if ( (pexit->description != NULL)  
       && (pexit->description[0] != '\0')) {
       send_to_char( pexit->description, ch);
     } else {
       if (IS_SET(pexit->exit_info, EX_ISDOOR)) {
          sprintf(buf, "There is a %s there.\r\n", door_name);
       }
     }

    /* Tell them if the door is open or closed... */

     if (IS_SET(pexit->exit_info, EX_ISDOOR)) {
       if (IS_SET(pexit->exit_info, EX_CLOSED)) {
          if (pexit->condition == 100) condition = 0;
          else if (pexit->condition > 90) condition = 1 ;
          else if (pexit->condition > 75) condition = 2;
          else if (pexit->condition > 50) condition = 3;
          else if (pexit->condition > 25) condition = 4;
          else if (pexit->condition > 10) condition = 5;
          else if (pexit->condition >  0) condition = 6;
          else if (pexit->condition == 0) condition = 7;
          if (condition > 0) {	
             sprintf(doorbuf, " {x({c");
             strcat(doorbuf, cond_table[condition]);
             strcat(doorbuf, "{x)");
             sprintf(buf, "The %s is closed. %s\r\n", door_name, doorbuf); 	
          } else {
             sprintf(buf, "The %s is closed.\r\n", door_name); 	
         }
         send_to_char(buf, ch);
         return;
       } else {
         sprintf(buf, "The %s stands open.\r\n", door_name);
         send_to_char(buf, ch);
       } 
     } 
   }

  /* Tell them if it's dark... */

   if (to_dark) {
     send_to_char("It is dark.\r\n", ch);
   }

  /* Tell them they can't see anything if it's misty... */

   if ( (to_room != room) 
     && (IS_SET(to_room->room_flags, ROOM_MISTY))) {
     if (!to_dark) {
       send_to_char("Swirling mists obscure your view\r\n", ch);
     }
     return;
   }

  /* Send a description each person in the room... */ 

   rch = to_room->people;

   for ( ; rch != NULL; rch = rch->next_in_room ) {

    /* Can't see yourself... */

     if (ch == rch) {
       continue; 
     }

    /* Can't see invisible higher level wizards... */ 

     if ( (!IS_NPC(rch))
       && (IS_SET(rch->plr, PLR_WIZINVIS))
       && (get_trust( ch ) < rch->invis_level)) {
       continue;
     }

    /* Show them if they can be seen normally... */

     if (!to_dark && can_see( ch, rch ) ) {
       show_char_to_char_0( rch, ch );
       occupied = TRUE;
       continue;
     } 
 
    /* Characters with infravision have glowing eyes... */  

     if ( room_is_dark(rch->in_room)
       && IS_AFFECTED(rch, AFF_INFRARED ) ) {
       send_to_char( "You see a pair of {rglowing red{x eyes!\r\n", ch );
       occupied = TRUE;
     }
   }

//   if ( !occupied 
//     && !to_dark) {
//     send_to_char("You do not see anyone.\r\n", ch);
//   }

   return;
 }

void do_lore (CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int chance;
    OBJ_DATA *obj;

    one_argument( argument,arg );

    if ( arg[0] == '\0' ) {
      send_to_char( "Lore what?\r\n", ch );
      return;
    }

    obj =  get_obj_list(ch,argument,ch->carrying);

    if (obj== NULL) {
      send_to_char("You don't have that item.\r\n",ch);
      return;
    }

   /* What are the odds? */ 
 
    chance = get_skill(ch, gsn_lore);

   /* If there's no chance, forget about it... */ 

    if (chance < 1) {
      send_to_char("You don't have any idea where to begin to look for information.",ch);
      return;
    }

   /* Can we do it? */

    if (ch->move < 100) {
      send_to_char("You are too tired to look up information on this object\r\n",ch);
      return;
    }

   /* Do the lore... */

    ch->move -= 100;

    WAIT_STATE( ch, skill_array[gsn_lore].beats );

   /* Modify for high int... */

    chance = chance + ((get_curr_stat (ch, STAT_INT)/4) - 20) * 5;

   /* See if it worked... */

    if (chance + number_open() <= 100 ) {
      send_to_char ("You don't discover anything.\r\n",ch);
      check_improve(ch, gsn_lore, FALSE, 1);
      return;
    }

   /* Success! */

    send_to_char("You manage to find some information.\r\n",ch);
    check_improve(ch, gsn_lore, TRUE, 1);

    spell_identify( gsn_lore, ch->level, ch, obj);
   
    return;
}


void do_worth( CHAR_DATA *ch, char *argument ) {
int i;
bool money = FALSE;

    send_to_char("You have\r\n", ch);
    for (i = 0; i < MAX_CURRENCY; i++) {
        if (ch->gold[i] >0) {
             sprintf_to_char(ch,"%ld %s.\r\n",ch->gold[i], flag_string(currency_type, i));
             money = TRUE;
        }
    }
    if (!money) send_to_char("{gNothing.{c\r\n", ch);

    return;
}


char *	const	day_name	[] =
{
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
    "Friday", "Saturday"
};

const char *get_statdesc(CHAR_DATA *ch, int stat) {

  int base_stat, curr_stat, mod;

  base_stat = ch->perm_stat[stat];
  curr_stat = get_curr_stat(ch, stat);

  if (base_stat > curr_stat) {
    mod = 3;
  } else {
    if (base_stat < curr_stat) {
      mod = 1;
    } else {
      mod = 2;
    }
  }  

  return statdesc[curr_stat/4][mod];
}
 

void oscore(CHAR_DATA *ch) {
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;
    char vb[MAX_STRING_LENGTH];

    obj = ch->trans_obj;

    sprintf( buf, "Name(s): %s\r\n", obj->name );
    send_to_char( buf, ch );
	
    format_vnum(obj->pIndexData->vnum, vb);
    sprintf( buf, "Vnum: %s  Format: %s  Type: %s  Resets: %d  Size: %s\r\n",
                   vb,
                   obj->pIndexData->new_format ? "new" : "old",
                   item_type_name(obj),
                   obj->pIndexData->reset_num,
                  (obj->size >=0 && obj->size <= 5) ? size_table[obj->size] : "unknown");
    send_to_char( buf, ch );

    sprintf( buf, "Short description: %s\r\nLong description: %s\r\n", obj->short_descr, obj->description );
    send_to_char( buf, ch );

    sprintf( buf, "Zones: %s\r\n", flag_string( zmask_flags, obj->zmask));  
    send_to_char( buf, ch);
	
    sprintf (buf, "Material: %s\r\n", material_name (obj->material));
    send_to_char( buf, ch);

    sprintf( buf, "Wear bits: %s\r\nExtra bits: %s\r\n",
                   wear_bit_name(obj->wear_flags),
                   extra_bit_name( obj->extra_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Number: %d/%d  Weight: %d/%d\r\n",
                   1,
                   get_obj_number( obj ),
                   obj->weight,
                   get_obj_weight( obj ) );
    send_to_char( buf, ch );

    sprintf( buf, "Level: %d  Cost: %d  Condition: %d  Timer: %d\r\n",
                   obj->level,
                   obj->cost,
                   obj->condition,
                   obj->timer );
    send_to_char( buf, ch );

    if (obj->in_room == NULL) {
      sprintf(vb,"0");
    } else {
      format_vnum(obj->in_room->vnum, vb);
    }

    sprintf( buf, "In room: %s  In object: %s  Carried by: %s  Wear_loc: %d\r\n",
                   vb,
                   obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr,
                   obj->carried_by == NULL ? "(none)" : 
                                           can_see(ch,obj->carried_by) ? obj->carried_by->name : "someone",
                   obj->wear_loc );
    send_to_char( buf, ch );
    
    sprintf( buf, "Values:      %ld %ld %ld %ld %ld\r\n",
                   obj->value[0],
                   obj->value[1],
                   obj->value[2],
                   obj->value[3],
                   obj->value[4] );
    send_to_char( buf, ch );
    
    /* now give out vital statistics as per identify */
    
    switch ( obj->item_type ) {
    	case ITEM_SCROLL: 
    	case ITEM_POTION:
    	case ITEM_PILL:
	    sprintf( buf, "Level %ld spells of:", obj->value[0] );
	    send_to_char( buf, ch );

	    if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL ) {
	    	send_to_char( " '", ch );
	    	send_to_char( effect_array[obj->value[1]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL ) {
	    	send_to_char( " '", ch );
	    	send_to_char( effect_array[obj->value[2]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL ) {
	    	send_to_char( " '", ch );
	    	send_to_char( effect_array[obj->value[3]].name, ch );
	    	send_to_char( "'", ch );
	    }

	     send_to_char( ".\r\n", ch );
	     break;

    	case ITEM_WAND: 
    	case ITEM_STAFF: 
	    sprintf( buf, "Has %ld(%ld) charges of level %ld",
                                                        obj->value[1], obj->value[2], obj->value[0] );
	    send_to_char( buf, ch );
      
	    if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL ) {
	    	send_to_char( " '", ch );
	    	send_to_char( effect_array[obj->value[3]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    send_to_char( ".\r\n", ch );
	    break;
      
    	case ITEM_WEAPON:
 	    send_to_char("Weapon type is ",ch);
	    switch (obj->value[0]) {
	    	case(WEAPON_EXOTIC): 
			send_to_char("exotic\r\n",ch);
                                   	break;
	    	case(WEAPON_SWORD): 
			send_to_char("sword\r\n",ch);	
			break;	
	    	case(WEAPON_DAGGER): 
			send_to_char("dagger\r\n",ch);	
			break;
	    	case(WEAPON_SPEAR): 
			send_to_char("spear\r\n",ch);
			break;
	    	case(WEAPON_MACE): 
			send_to_char("mace\r\n",ch);
			break;
	   	case(WEAPON_AXE): 
			send_to_char("axe\r\n",ch);
			break;
	    	case(WEAPON_FLAIL): 
			send_to_char("flail\r\n",ch);
			break;
	    	case(WEAPON_WHIP): 
			send_to_char("whip\r\n",ch);		
			break;
	    	case(WEAPON_POLEARM): 
			send_to_char("polearm\r\n",ch);	
			break;
		case(WEAPON_GUN): 
			send_to_char("gun\r\n", ch);
			break;
		case(WEAPON_BOW): 
			send_to_char("bow\r\n", ch);
			break;
		case(WEAPON_HANDGUN): 
			send_to_char("handgun\r\n", ch);
			break;
		case(WEAPON_SMG): 
			send_to_char("submachinegun\r\n", ch);
			break;
	    	default: 
			send_to_char("unknown\r\n",ch);	
			break;
 	    }

	    if (obj->pIndexData->new_format) {
                sprintf(buf, "Damage is %ldd%ld (average %ld)\r\n", 
                              obj->value[1],
                              obj->value[2],
                             (1 + obj->value[2]) * obj->value[1] / 2);
	    } else {
                sprintf( buf, "Damage is %ld to %ld (average %ld)\r\n",
                               obj->value[1],
                               obj->value[2],
                              (obj->value[1] + obj->value[2]) / 2 );
            }
	    send_to_char( buf, ch );
	   
            send_to_char("Damage type: ", ch);

            send_to_char(flag_string(weapon_flags, obj->value[3]), ch);
            send_to_char("\r\n", ch);

	    if (obj->value[4]) {
	        sprintf(buf,"Weapons flags: %s\r\n",weapon_bit_name(obj->value[4]));
	        send_to_char(buf,ch);
            }
	    break;

    	case ITEM_ARMOR:
	    sprintf( buf, "Armor class is %ld pierce, %ld bash, %ld slash, and %ld vs. magic\r\n",
                                             obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	    send_to_char( buf, ch );
	    break;
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )    {
	if (!paf->bitvector ) sprintf( buf, "Affects %s by %d, level %d.\r\n",
                                         affect_loc_name( paf->location ), paf->modifier, paf->level );
	else sprintf (buf, "Spell affect with bits [%s] for %d hours.\r\n",
                                         affect_bit_name(paf->bitvector), paf->duration );
	send_to_char( buf, ch );
    }

    if (!obj->enchanted) {
      for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) {
	if (!paf->bitvector) sprintf( buf, "Affects %s by %d, level %d.\r\n", 
                                           affect_loc_name( paf->location ), paf->modifier,paf->level );
	else sprintf (buf, "Spell affect with bits [%s] for %d hours.\r\n", 
                                           affect_bit_name(paf->bitvector), paf->duration );
	send_to_char( buf, ch );
      }
    }
    return;
}


void do_score( CHAR_DATA *ch, char *argument ) {
char buf[MAX_STRING_LENGTH];
char fs[MAX_STRING_LENGTH];
int i;
int day;
int next_xps;
int curr_xps;
char *xps;

    LPROF_DATA *prof;
    char *hd;

    if (IS_AFFECTED(ch, AFF_MORF)) {
         oscore(ch);
         return;
    }
    
    if (!str_cmp(argument, "real")) {
        CHAR_DATA *vic;

        if (!ch->desc->original) {
              send_to_char("There is no real character to review.\r\n",ch);
              return;
        }

        vic = ch->desc->original;

        send_to_char("{Y-------------------------------------------------------------------------{x\r\n",ch);
        sprintf_to_char(ch, "{Y|{w%70s{Y |{x\r\n", vic->short_descr);
        send_to_char("{Y-------------------------------------------------------------------------{x\r\n",ch);
        sprintf(buf,"{BSTR   [{W%s{B]  {YLEVEL   [{W%6d/{C%5d{Y]  {GHP     [{W%7d/%7d{G]{x\r\n",
           get_statdesc(vic, STAT_STR),
           vic->level,
           get_effective_level(vic),
           vic->hit,
           vic->max_hit);
        send_to_char(buf,ch);		

        sprintf(buf,"{BINT   [{W%s{B]  {YRACE    [{W%12s{Y]  {GMANA   [{W%7d/%7d{G]{x\r\n",
           get_statdesc(vic, STAT_INT),
           race_array[vic->race].name,
           vic->mana,
           vic->max_mana);
        send_to_char(buf,ch);	

        sprintf(buf,"{BWIS   [{W%s{B]  {YSEX     [{W%12s{Y]  {GMOVE   [{W%7d/%7d{G]{x\r\n",
           get_statdesc(vic, STAT_WIS),
           vic->sex == 0 ? "sexless" : vic->sex == 1 ? "male" : "female",
           vic->move,
           vic->max_move);
        send_to_char(buf,ch);	

        sprintf(buf,"{BDEX   [{W%s{B]  {YCLASS   [{W%12s{Y]  {GPRAC   [{W%15d{G]{x\r\n",
           get_statdesc(vic, STAT_DEX),
           IS_NPC(vic) ? "mobile" : ((vic->profs == NULL) ? class_table[vic->pc_class].name : "professional"),
           vic->practice);
        send_to_char(buf,ch);

         next_xps = exp_for_next_level(vic) - exp_for_level(vic->level);
         curr_xps = vic->exp - exp_for_level(vic->level); 
         next_xps = (curr_xps * 100)/next_xps;

         if (next_xps < 10) {
             xps = "{R";
         } else if (next_xps < 40) {
            xps = "{Y";
         } else if (next_xps < 90) {
            xps = "{G";
         } else {
            xps = "{C";
         }

        sprintf(buf,"{BCON   [{W%s{B]  {YNXT LVL [%s%11d%%{Y]  {GTRAIN  [{W%15d{G]{x\r\n", get_statdesc(vic, STAT_CON), xps, next_xps, vic->train);
        send_to_char(buf,ch);

        sprintf(buf,"{BLUCK  [{W%9s{B]  {BHITR    [{W%12d{B]{x\r\n", get_statdesc(vic, STAT_LUC), GET_HITROLL(vic));
        send_to_char(buf,ch);

        switch(find_currency(vic)) {
              default:
                  sprintf(buf,"{BCHA   [{W%9s{B]  DAMR    [{W%12d{B] Cash    {Y[{W%15ld{Y]{x\r\n", get_statdesc(vic, STAT_CHA), GET_DAMROLL(vic), get_full_cash(vic));
                  break;
              case 0:
                  sprintf(buf,"{BCHA   [{W%9s{B]  DAMR    [{W%12d{B] {rCopper  {Y[{W%15ld{Y]{x\r\n", get_statdesc(vic, STAT_CHA), GET_DAMROLL(vic), vic->gold[0]);
                  break;
              case 1:
                  sprintf(buf,"{BCHA   [{W%9s{B]  DAMR    [{W%12d{B] {gDollars {Y[{W%15ld{Y]{x\r\n", get_statdesc(vic, STAT_CHA), GET_DAMROLL(vic), vic->gold[1]);
                  break;
              case 2:
                  sprintf(buf,"{BCHA   [{W%9s{B]  DAMR    [{W%12d{B] {yGold    {Y[{W%15ld{Y]{x\r\n", get_statdesc(vic, STAT_CHA), GET_DAMROLL(vic), vic->gold[2]);
                  break;
              case 3:
                  sprintf(buf,"{BCHA   [{W%9s{B]  DAMR    [{W%12d{B] {cCrowns  {Y[{W%15ld{Y]{x\r\n", get_statdesc(vic, STAT_CHA), GET_DAMROLL(vic), vic->gold[3]);
                  break;
        }

        send_to_char(buf,ch);

         if (vic->wimpy_dir < DIR_NONE) {
            sprintf(buf,"{MWIMPY [{W%9d{M]  {MDIRECT  [{W%12s{M]{x\r\n", vic->wimpy,  "last");
         } else {
            sprintf(buf,"{MWIMPY [{W%9d{M]  {MDIRECT  [{W%12s{M]{x\r\n", vic->wimpy, (vic->wimpy_dir == DIR_NONE) ? "random" : dir_name[vic->wimpy_dir]);
         }
         send_to_char(buf, ch);
         send_to_char("{Y-------------------------------------------------------------------------{x\r\n",ch);
         return;
    }


if (IS_AFFECTED(ch, AFF_CHARM)
&& IS_NPC(ch)
&& ch->master !=NULL) {
     send_to_char("{Y-------------------------------------------------------------------------{x\r\n",ch->master);
     if (!IS_NPC(ch)) {
          if (IS_SET(ch->act, ACT_VAMPIRE)
          && ch->pcdata->bloodline != NULL) {
                 sprintf(buf, "{Y|{w%30s Chylde of  %29s{Y |{x\r\n",ch->short_descr, ch->pcdata->bloodline);
          } else {
                 sprintf(buf, "{Y|{w%70s{Y |{x\r\n",ch->short_descr);
          }
     } else {
          sprintf(buf, "{Y|{w%70s{Y |{x\r\n",ch->short_descr);
     }
     send_to_char(buf,ch->master);		
     send_to_char("{Y-------------------------------------------------------------------------{x\r\n",ch->master);
     sprintf(buf,"{BSTR   [{W%s{B]  {YLEVEL   [{W%6d/{C%5d{Y]  {GHP     [{W%7d/%7d{G]{x\r\n",
     get_statdesc(ch, STAT_STR),
     ch->level,
     get_effective_level(ch),
     ch->hit,
     ch->max_hit);
      send_to_char(buf,ch->master);		

      sprintf(buf,"{BINT   [{W%s{B]  {YRACE    [{W%12s{Y]  {GMANA   [{W%7d/%7d{G]{x\r\n",
      get_statdesc(ch, STAT_INT),
      race_array[ch->race].name,
      ch->mana,
      ch->max_mana);
      send_to_char(buf,ch->master);	

      sprintf(buf,"{BWIS   [{W%s{B]  {YSEX     [{W%12s{Y]  {GMOVE   [{W%7d/%7d{G]{x\r\n",
      get_statdesc(ch, STAT_WIS),
      ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
      ch->move,
      ch->max_move);
      send_to_char(buf,ch->master);	

      sprintf(buf,"{BDEX   [{W%s{B]  {YCLASS   [{W%12s{Y]  {GPRAC   [{W%15d{G]{x\r\n",
      get_statdesc(ch, STAT_DEX),
      IS_NPC(ch) ? "mobile" : ((ch->profs == NULL) ? class_table[ch->pc_class].name : "professional"),
      ch->practice);
      send_to_char(buf,ch->master);

      sprintf(buf,"{BCON   [{W%s{B]                          {GTRAIN  [{W%15d{G]{x\r\n",
      get_statdesc(ch, STAT_CON),
      ch->train);
      send_to_char(buf,ch->master);

       next_xps = exp_for_next_level(ch) - exp_for_level(ch->level);
       curr_xps = ch->exp - exp_for_level(ch->level); 
       next_xps = (curr_xps * 100)/next_xps;

       if (next_xps < 10) {
           xps = "{R";
        } else if (next_xps < 40) {
           xps = "{Y";
        } else if (next_xps < 90) {
           xps = "{G";
        } else {
           xps = "{C";
        }

        sprintf(buf,"{BLUCK  [{W%9s{B]  {BHITR    [{W%12d{B]  {YNXT LVL[%s%14d%%{Y]{x\r\n", get_statdesc(ch, STAT_LUC), GET_HITROLL(ch), xps, next_xps);
        send_to_char(buf,ch->master);

         xps = "      enough";

        switch(find_currency(ch)) {
              default:
                  sprintf(buf,"{BCHA   [{W%9s{B]  DAMR    [{W%12d{B] Cash    {Y[{W%15ld{Y]{x\r\n", get_statdesc(ch, STAT_CHA), GET_DAMROLL(ch), get_full_cash(ch));
                  break;
              case 0:
                  sprintf(buf,"{BCHA   [{W%9s{B]  DAMR    [{W%12d{B] {rCopper  {Y[{W%15ld{Y]{x\r\n", get_statdesc(ch, STAT_CHA), GET_DAMROLL(ch), ch->gold[0]);
                  break;
              case 1:
                  sprintf(buf,"{BCHA   [{W%9s{B]  DAMR    [{W%12d{B] {gDollars {Y[{W%15ld{Y]{x\r\n", get_statdesc(ch, STAT_CHA), GET_DAMROLL(ch), ch->gold[1]);
                  break;
              case 2:
                  sprintf(buf,"{BCHA   [{W%9s{B]  DAMR    [{W%12d{B] {yGold    {Y[{W%15ld{Y]{x\r\n", get_statdesc(ch, STAT_CHA), GET_DAMROLL(ch), ch->gold[2]);
                  break;
              case 3:
                  sprintf(buf,"{BCHA   [{W%9s{B]  DAMR    [{W%12d{B] {cCrowns  {Y[{W%15ld{Y]{x\r\n", get_statdesc(ch, STAT_CHA), GET_DAMROLL(ch), ch->gold[3]);
                  break;
        }

        send_to_char(buf,ch->master);

        if (ch->wimpy_dir < DIR_NONE) {
            sprintf(buf,"{MWIMPY [{W%9d{M]  {MDIRECT  [{W%12s{M]{x\r\n", ch->wimpy,  "last");
        } else {
            sprintf(buf,"{MWIMPY [{W%9d{M]  {MDIRECT  [{W%12s{M]{x\r\n", ch->wimpy, (ch->wimpy_dir == DIR_NONE) ? "random" : dir_name[ch->wimpy_dir]);
        }
        send_to_char(buf, ch->master);
        send_to_char("{Y-------------------------------------------------------------------------{x\r\n",ch->master);
        return;
}
if (!ch->pcdata) return;

    send_to_char("{Y-------------------------------------------------------------------------{x\r\n",ch);
    if (!IS_NPC(ch)) {
          if (IS_SET(ch->act, ACT_VAMPIRE)
          && ch->pcdata->bloodline != NULL) {
                 sprintf(buf, "{Y|{w%30s Chylde of  %29s{Y |{x\r\n",ch->short_descr, ch->pcdata->bloodline);
          } else {
                 sprintf(buf, "{Y|{w%70s{Y |{x\r\n",ch->short_descr);
          }
    } else {
          sprintf(buf, "{Y|{w%70s{Y |{x\r\n",ch->short_descr);
    }
    send_to_char(buf, ch);		
    send_to_char("{Y-------------------------------------------------------------------------{x\r\n",ch);

/* Strength Level Hits */

    sprintf(buf,"{BSTR   [{W%s{B]  {YLEVEL   [{W%6d/{C%5d{Y]  {GHP     [{W%7d/%7d{G]{x\r\n",
        get_statdesc(ch, STAT_STR),
        ch->level,
        get_effective_level(ch),
        ch->hit,
        ch->max_hit);
    send_to_char(buf,ch);		

/* Intelligence Race Mana */

    sprintf(buf,"{BINT   [{W%s{B]  {YRACE    [{W%12s{Y]  {GMANA   [{W%7d/%7d{G]{x\r\n",
        get_statdesc(ch, STAT_INT),
        race_array[ch->race].name,
        ch->mana,
        ch->max_mana);
    send_to_char(buf,ch);	

/* Wisdom Sex Move */  

    sprintf(buf,"{BWIS   [{W%s{B]  {YSEX     [{W%12s{Y]  {GMOVE   [{W%7d/%7d{G]{x\r\n",
        get_statdesc(ch, STAT_WIS),
        ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
        ch->move,
        ch->max_move);
    send_to_char(buf,ch);	

/* Dexterity Class Practices */

    sprintf(buf,"{BDEX   [{W%s{B]  {YCLASS   [{W%12s{Y]  {GPRAC   [{W%15d{G]{x\r\n",
        get_statdesc(ch, STAT_DEX),
        IS_NPC(ch) ? "mobile" : ((ch->profs == NULL) ? class_table[ch->pc_class].name : "professional"),
        ch->practice);
    send_to_char(buf,ch);

/* Constitution Age Sanity */

    sprintf(buf,"{BCON   [{W%s{B]  {YAGE     [{W%4d/%-4dHrs{Y]  {GTRAIN  [{W%15d{G]{x\r\n",
        get_statdesc(ch, STAT_CON),
        get_age(ch),
        ( ch->pcdata->played + (int) (current_time - ch->pcdata->logon) ) / 3600,
        ch->train);
    send_to_char(buf,ch);

/* Hit_Roll Next level Rate */

    next_xps = exp_for_next_level(ch) - exp_for_level(ch->level);

    curr_xps = ch->exp - exp_for_level(ch->level); 

    next_xps = (curr_xps * 100)/next_xps;

    if (next_xps < 10) {
      xps = "{R";
    } else if (next_xps < 40) {
      xps = "{Y";
    } else if (next_xps < 90) {
      xps = "{G";
    } else {
      xps = "{C";
    }

    sprintf(buf,"{BLUCK  [{W%s{B]  {YNXT LVL [%s%11d%%{Y]  {GSANITY [{W%15d{G]{x\r\n",
        get_statdesc(ch, STAT_LUC),
        xps,
        next_xps,
        get_sanity(ch));
    send_to_char(buf,ch);

/* Dam_Roll Xps Weight */

    switch(find_currency(ch)) {
         default:
            sprintf(buf,"{BCHA   [{W%s{B]  Cash    {Y[{W%12ld{Y]{x\r\n", get_statdesc(ch, STAT_CHA), get_full_cash(ch));
            break;
         case 0:
            sprintf(buf,"{BCHA   [{W%s{B]  {rCopper  {Y[{W%12ld{Y]{x\r\n", get_statdesc(ch, STAT_CHA), ch->gold[0]);
            break;
         case 1:
            sprintf(buf,"{BCHA   [{W%s{B]  {gDollars {Y[{W%12ld{Y]{x\r\n", get_statdesc(ch, STAT_CHA), ch->gold[1]);
            break;
         case 2:
            sprintf(buf,"{BCHA   [{W%s{B]  {yGold    {Y[{W%12ld{Y]{x\r\n", get_statdesc(ch, STAT_CHA), ch->gold[2]);
            break;
         case 3:
            sprintf(buf,"{BCHA   [{W%s{B]  {cCrowns  {Y[{W%12ld{Y]{x\r\n", get_statdesc(ch, STAT_CHA), ch->gold[3]);
            break;
    }
    send_to_char(buf,ch);

        sprintf(buf,"{BHITR  [{W%9d{B]  {GWEIGHT  [{W%5d/%6d{G]  {GITEMS  [{W%7d/%7d{M]{x\r\n",GET_HITROLL(ch), ch->carry_weight, can_carry_w(ch), ch->carry_number, can_carry_n(ch));
        send_to_char(buf,ch);

/* Wimpy Gold Items */

    if (ch->wimpy_dir < DIR_NONE) {
         sprintf(buf,"{BDAMR  [{W%9d{B]  {MWIMPY   [{W%12d{M]  {MDIRECT [{W%15s{M]{x\r\n", GET_DAMROLL(ch), ch->wimpy, "last");
    } else {
         sprintf(buf,"{BDAMR  [{W%9d{B]  {MWIMPY   [{W%12d{M]  {MDIRECT [{W%15s{M]{x\r\n",GET_DAMROLL(ch), ch->wimpy, (ch->wimpy_dir == DIR_NONE) ? "random" : dir_name[ch->wimpy_dir]);
    }
    send_to_char(buf,ch);

        if (ch->pcdata->nextquest !=0) {
              sprintf(buf,"{RFAME  [{W%9d{R]  {RMSTATUS [{W     WAITING{R]  {RCOUNT  [{W%15d{R]{x\r\n", ch->pcdata->questpoints, ch->pcdata->nextquest);
         } else {
               if (ch->pcdata->countdown !=0) {
                    sprintf(buf,"{RFAME  [{W%9d{R]  {RMSTATUS [{W     MISSION{R]  {RCOUNT  [{W%15d{R]{x\r\n", ch->pcdata->questpoints, ch->pcdata->countdown);
               } else {
                    sprintf(buf,"{RFAME  [{W%9d{R]  {RMSTATUS [{W       READY{R]  {RCOUNT  [{W%15d{R]{x\r\n", ch->pcdata->questpoints, 0);
               }
           }
           send_to_char(buf,ch);
           send_to_char("{Y-------------------------------------------------------------------------{x\r\n",ch);

   /* Professions... */

    prof = ch->profs;

    hd = "{wProfession: {C";

    while (prof != NULL) {

       sprintf(buf, "%s%s{w, level {W%d{w.\r\n", hd, prof->profession->name, get_professional_level(ch, prof->level));

       send_to_char(buf, ch); 
     
       prof = prof->next;

       hd = "            {C";
    }

   /* Spoken languages... */

    sprintf(buf, "{wSpeaking: public [{C%s{w]  private [{C%s{w]\r\n",
                 skill_array[ch->speak[SPEAK_PUBLIC]].name, 
                 skill_array[ch->speak[SPEAK_PRIVATE]].name);
    send_to_char(buf, ch); 

   /* Age and DoB... */

    day=ch->pcdata->startday+1;
    sprintf(buf,"{wBorn\t[{C%d{w, month of {C%s {win the Year {C%d{w]{x\r\n", day,  month_name[ch->pcdata->startmonth], ch->pcdata->startyear);
    send_to_char(buf,ch);

    if (str_cmp(ch->pcdata->spouse,"")) {
       sprintf(buf, "{wYou're married with {M%s{w.{x\r\n", ch->pcdata->spouse);
       send_to_char(buf,ch);
   }

   /* Game powers... */

    if (IS_CREATOR(ch)) {
      send_to_char("{yYou are a creator of places strange.{x\r\n", ch);
    } else if (IS_LESSER(ch)) {
      send_to_char("{yYou are a lesser god.{x\r\n", ch);
    } else if (IS_GOD(ch)) {
      send_to_char("{yYou are a god.{x\r\n", ch);
    } else if (IS_GREATER(ch)) {
      send_to_char("{yYou are a strange and powerful greater god.{x\r\n", ch);
    } else if (IS_IMP(ch)) {
      send_to_char("{yYou are the implementor.{x\r\n", ch);
    } else if (IS_HERO(ch)) {
       send_to_char("{yYou are a brave hero.{x\r\n", ch);
    } else if (IS_INVESTIGATOR(ch)) {
       send_to_char("{yYou are a steadfast investigator.{x\r\n", ch);
    } else if (IS_HUMAN(ch)) {
       send_to_char("{yYou are a normal player.{x\r\n", ch);
    } else if (IS_NEWBIE(ch)) {
       send_to_char("{yYou are a lowly newbie. Behave.{x\r\n", ch);
    } else {
       send_to_char("{yYou are somewhat confused.{x\r\n", ch);
    }

   /* Perma Death... */

    if (IS_SET(mud.flags, MUD_PERMAPK)) {
      send_to_char("{yThe mud is in perma death player combat mode.{x\r\n", ch);
    }  
 
    if (IS_SET(ch->plr, PLR_PERMADEATH)) {
      send_to_char("{R*** If you die, you will be DELETED ***{x\r\n", ch);
    }

   /* Current position... */  

    sprintf( buf, "You are " );
    pos_text(ch, buf);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

   /* Sanity... */

    if (get_sanity(ch) > 0) {
      int sanity = get_sanity(ch);

      if (sanity < 2) {
        send_to_char("{MMaaaaa{RHaaaaaa{GHaaaaaa{CHaaaaaa{Y!!!{x\r\n", ch);
      } else  if (sanity < 4) {
        send_to_char("{RYou are feeling absolutely terrified!{x\r\n", ch);
      } else  if (sanity < 8) {
        send_to_char("{YYou are feeling paranoid!{x\r\n", ch);
      } else  if (sanity < 16) {
        send_to_char("{yYou are feeling nervous...{x\r\n", ch);
      } else  if (sanity < 32) {
        send_to_char("{wYou are feeling a little on edge...{x\r\n", ch);
      } 
    }

   /* Current condition... */

    if (!IS_NPC(ch)) {

     /* Food */

      if ( ch->condition[COND_FOOD] > COND_STUFFED ) {
        send_to_char("{wYou feel absolutely stuffed!\r\n", ch );
      } else if ( ch->condition[COND_FOOD] > COND_HUNGRY ) {
        ;
      } else if ( ch->condition[COND_FOOD] > COND_VERY_HUNGRY ) {
        send_to_char("{wYou are {Yhungry{w!\r\n", ch );
      } else if ( ch->condition[COND_FOOD] > COND_STARVING ) {
        send_to_char("{wYou are {Yvery hungry{w!\r\n", ch );
      } else {
        send_to_char("{wYou are {Ystarving to death{w!\r\n", ch );
      }

     /* Drink */

      if ( ch->condition[COND_DRINK] > COND_SLUSHY ) {
        send_to_char("{wYou can feel fluids sloshing about in your guts!\r\n",
                                                                          ch );
      } else if ( ch->condition[COND_DRINK] > COND_THIRSTY ) {
        ;
      } else if ( ch->condition[COND_DRINK] > COND_VERY_THIRSTY ) {
        send_to_char("{wYou are {Ythirsty{w!\r\n", ch );
      } else if ( ch->condition[COND_DRINK] > COND_DEHYDRATED ) {
        send_to_char("{wYou are {Yvery thirsty{w!\r\n", ch );
      } else {
        send_to_char("{wYou are {Ydieing of thirst{w!\r\n", ch );
      }

     /* Alcohol */

      if ( ch->condition[COND_DRUNK] > 11 ) {
        send_to_char( "{wYou are {Bd{yr{cu{Mn{rk{W!{w\r\n",   ch );
      }

      if (is_affected(ch, gafn_hangover)) {
        send_to_char( "{wYou have a headache and an upset stomach.\r\n", ch); 
      }

     /* Health */

      if ( ch->condition[COND_FAT] > 10 ) {
        send_to_char( "{wYou are overweight!\r\n",   ch );
      }

    }

   /* Dreaming... */

    if ( ch->waking_room != NULL ) {
      if (ch->nightmare == TRUE) sprintf(buf, "{cYou're on a nightmare (from '%s'){x\r\n",ch->waking_room->name);
      else sprintf(buf, "{cYou are dreaming (from '%s'){x\r\n",ch->waking_room->name);
      send_to_char(buf, ch);
    } 

   /* Print hunt/prey values */

    if (ch->level >= 30) {

      if (ch->huntedBy != NULL) {
         sprintf(buf,"{wYou are Hunted by: {Y%s{x\r\n", ch->huntedBy->short_descr);
         send_to_char(buf, ch);
      }

      if (ch->prey != NULL) {
         sprintf(buf,"{wYou are Hunting: {Y%s{x\r\n", ch->prey->short_descr);
         send_to_char(buf, ch);
      }
    }  

/* Vampires */

    if (IS_SET(ch->act,ACT_VAMPIRE)) {
      sprintf(buf,"Bloodpool: {R%d{x.\r\n", ch->pcdata->blood);
      send_to_char(buf, ch);
    }

/* Worship section */

    sprintf(buf,"{cYou are a worshipper of {R%s{x.\r\n", ch->pcdata->cult);
    send_to_char(buf, ch);

    if (IS_IMMORTAL(ch)) {
      int rn = get_cult_rn(ch->name);
      if (rn > 0) {
          sprintf_to_char(ch,"Mana Pool {R%ld{x.\r\n", cult_array[rn].power);
      }
    }

    if (IS_ARTIFACTED(ch, ART_ABHOTH)) {
      int rn = get_cult_rn("abhoth");
      if (rn > 0) {
          sprintf_to_char(ch,"Mana Pool {R%ld{x.\r\n", cult_array[rn].power);
      }
    }

     if (get_skill(ch, gsn_black_belt) >0
     && ch->race==1) {
         switch (ch->pcdata->style) {
             case STYLE_TIGER:
                 sprintf(fs,"Tiger style");
                 break;
             case STYLE_CRANE:
                 sprintf(fs,"Crane style");
                 break;
             case STYLE_SNAKE:
                 sprintf(fs,"Snake style");
                 break;
             case STYLE_DRUNKEN_BUDDHA:
                 sprintf(fs,"Drunken Buddha style");
                 break;
             case STYLE_DRAGON:
                 sprintf(fs,"Dragon style");
                 break;
             case STYLE_NINJITSU:
                 sprintf(fs,"Ninjitsu style");
                 break;
             case STYLE_TOUCH:
                 sprintf(fs,"Touch Mastery style");
                 break;
             default:
                 sprintf(fs,"Iron Fist style");
                 break;
         }
         sprintf(buf,"{cYou're a master of the secret %s{x.\r\n", fs);
         send_to_char(buf, ch);
     }
    
    strcpy(buf, "{wArmour: "); 

    for (i = 0; i < 4; i++) {

        int ac;

	switch(i) {
          case(AC_PIERCE):
            strcat(buf, "{wPiercing [");
            break;

	  case(AC_BASH):
            strcat(buf, "{wBashing [");
            break;

	  case(AC_SLASH):
            strcat(buf, "{wSlashing [");
            break;

	  case(AC_EXOTIC):
            strcat(buf, "{wMagic [");
            break;

	  default:
            strcat(buf, "{wError [");
            break;
	}
	
        ac = GET_AC(ch, i);

	if (ac >=  99 ) {
                   strcat(buf, "{RNone{w] ");
	} else if (ac >= 70) {
                   strcat(buf, "{RExposed{w] ");
	} else if (ac >= 25) {
                   strcat(buf, "{YX.Poor{w] ");
	} else if (ac >= -5) {
                   strcat(buf, "{YV.Poor{w] ");
                } else if (ac >= -65) {
                   strcat(buf, "{yPoor{w] ");
	} else if (ac >= -125) {
                   strcat(buf, "{wAverage] ");
	} else if (ac >= -200) {
                   strcat(buf, "{gFair{w] ");
	} else if (ac >= -300) {
	   strcat(buf, "{gGood{w] ");
	} else if (ac >= -400) {
	   strcat(buf, "{GV.Good{w] ");
	} else if (ac >= -500) {
	   strcat(buf, "{cX.Good{w] ");
	} else if (ac >= -650) {
	   strcat(buf, "{cSuperb{w] ");
	} else {
	   strcat(buf, "{CExtreme{w] ");
                }
    }  

    strcat(buf, "\r\n");

    send_to_char(buf,ch);

    /* RT wizinvis and holy light */
    if ( IS_IMMORTAL(ch))    {
      send_to_char("{wHoly Light: ",ch);
      if (IS_SET(ch->plr, PLR_HOLYLIGHT))
        send_to_char("on   {x",ch);
      else
        send_to_char("off  {x",ch);
 
      if (IS_SET(ch->plr, PLR_WIZINVIS))      {
        sprintf( buf, "{wInvisible: level {C%d{x   ",ch->invis_level);
        send_to_char(buf,ch);
      }      

       if (IS_SET(ch->plr, PLR_CLOAK))      {
        sprintf( buf, "{wCloak: level {C%d{x",ch->cloak_level);
        send_to_char(buf,ch);
      }
		
      send_to_char("\r\n",ch);
    }

    sprintf_to_char(ch, "{wYou are %s\r\n", get_align(ch->alignment));
    return;
}


static AFFECT_DATA af_found[MAX_AFFECT][APPLY_MAX];

void do_affect (CHAR_DATA *ch, char *argument) {
    AFFECT_DATA *paf;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    OBJ_DATA *tmp;
    bool noaffect=TRUE;
    int afn, loc;

    CHAR_DATA *victim;

    victim = ch;

    if ( IS_IMMORTAL(ch)
    && argument[0] != '\0') {
        victim = get_char_room(ch, argument); 
        if (victim == NULL ) victim = get_char_world(ch, argument); 

        if (victim == NULL) {
           send_to_char("Not found.\r\n", ch);
           return;
        }
    }  
 
    if (victim == ch) {
        send_to_char("You are affected by:\r\n",ch);
    } else {
        send_to_char("They are affected by:\r\n", ch);
    }

    if ( victim->affected != NULL ) {
        noaffect=FALSE;

        for( afn = 0; afn < MAX_AFFECT; afn++) {
            for( loc = 0; loc < APPLY_MAX; loc++) {
                   af_found[afn][loc].afn = 0;
            }
        } 
    	
        paf = victim->affected;

        while ( paf != NULL ) {
            afn = paf->afn;
            loc = paf->location;

            if (af_found[afn][loc].afn == 0) {
                af_found[afn][loc] = *paf;
            } else {
                af_found[afn][loc].duration = UMAX(paf->duration, af_found[afn][loc].duration); 
                af_found[afn][loc].level = UMAX(paf->level, af_found[afn][loc].level); 
                af_found[afn][loc].modifier += paf->modifier;
            } 
            paf = paf->next;
        }

              for ( afn = 0; afn < MAX_AFFECT; afn++) {
	  sprintf(buf, "Affect: '%s'\r\n", affect_array[afn].name );
                  tail_chain();
                  for( loc = 0; loc < APPLY_MAX; loc++) {
                      if ( af_found[afn][loc].afn == 0 ) continue;
                      if ( buf[0] != '\0' ) send_to_char( buf, ch );
              	      if ( ch->level >= 20 ) {
                           if ( af_found[afn][loc].duration != -1 ) {
                                  sprintf( buf2, " for %3d hours", af_found[afn][loc].duration );
                           } else { 
                                  buf2[0] = '\0';
                           } 
	 
	           sprintf( buf, "        modifies %-30.30s by %6d%s\r\n", affect_loc_name( af_found[afn][loc].location ), af_found[afn][loc].modifier, buf2);
	           send_to_char( buf, ch );
	      }
                      buf[0] = '\0';
                  } 
              }
    }

   /* now, loop through object affects */

    for (tmp = victim->carrying ; tmp != NULL ; tmp = tmp->next_content) {
          if (tmp->wear_loc != WEAR_NONE) {
		
                    if(tmp->enchanted) {
   	         for (paf = tmp->affected ; paf != NULL; paf = paf->next) {
		if (paf->bitvector) { /* check for spell vs just normal bonus affect */
		    noaffect=FALSE;
		    sprintf( buf, "Object Spell: bits [%s]", affect_bit_name(paf->bitvector) );
		    send_to_char( buf, ch );
		    sprintf( buf, " modifies %s by %d\r\n",  affect_loc_name( paf->location ),  paf->modifier);
		    send_to_char( buf, ch );
		}
	         }

	    } else{
	         for (paf = tmp->pIndexData->affected ; paf != NULL; paf = paf->next) {
		    if (paf->bitvector) {
			noaffect=FALSE;
			sprintf( buf, "Object Spell: bits [%s]", affect_bit_name(paf->bitvector) );
			send_to_char( buf, ch );
			sprintf( buf, " modifies %s by %d\r\n", affect_loc_name( paf->location ), paf->modifier );
			send_to_char( buf, ch );
		    }
	         } 
                    }
          } 
    }
			
    if (noaffect)  send_to_char ("Nothing.\r\n\r\n",ch);

    if (ch->artifacted_by) {
        send_to_char("Artifact Powers:\r\n",ch);
        sprintf_to_char(ch, "[{c%s{x]\r\n", flag_string( artifact_flags, ch->artifacted_by ));
    }

    return;
}


void do_room_affect (CHAR_DATA *ch, char *argument) {
	
    AFFECT_DATA *paf;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    bool noaffect=TRUE;
    int afn, loc;

    
    send_to_char("The room is affected by:\r\n",ch);
    
    sprintf( buf, "Affect:  [{c%s{x]\r\n", flag_string( raffect_flags, ch->in_room->affected_by ) );
    send_to_char( buf, ch );

    if ( ch->in_room->affected != NULL ) {
        noaffect=FALSE;

        for( afn = 0; afn < MAX_AFFECT; afn++) {
          for( loc = 0; loc < APPLY_MAX; loc++) {
            af_found[afn][loc].afn = 0;
          }
        } 
    	
        paf = ch->in_room->affected;
        while ( paf != NULL ) {
          afn = paf->afn;
          loc = paf->location;

          if (af_found[afn][loc].afn == 0) {
            af_found[afn][loc] = *paf;
          } else {
             af_found[afn][loc].duration = UMAX(paf->duration, af_found[afn][loc].duration); 
             af_found[afn][loc].level = UMAX(paf->level, af_found[afn][loc].level); 
             af_found[afn][loc].modifier += paf->modifier;
          } 
           paf = paf->next;
        }
	for ( afn = 0; afn < MAX_AFFECT; afn++) {
	  sprintf( buf, "Affect: '%s'\r\n", affect_array[afn].name );
                  for( loc = 0; loc < APPLY_MAX; loc++) {
                      if ( af_found[afn][loc].afn == 0 ) continue;
                      if ( buf[0] != '\0' ) send_to_char( buf, ch );
               	      if ( ch->level >= 20 ) {
	            if ( af_found[afn][loc].duration != -1 ) {
                                   sprintf( buf2, " for %3d hours", af_found[afn][loc].duration );
                            } else { 
                                   buf2[0] = '\0';
                            } 
	            sprintf( buf, "        modifies %-30.30s by %6d%s\r\n", affect_loc_name( af_found[afn][loc].location ), af_found[afn][loc].modifier,	 buf2);
	            send_to_char( buf, ch );
	      }
                      buf[0] = '\0';
                  } 
	}
    }

    if (noaffect) send_to_char ("Nothing.\r\n",ch);
    return;
}


char season_names[SEASON_MAX][15] = {
 { "Spring" },
 { "Summer" },
 { "Fall"   },
 { "Winter" }
};
 

void do_time( CHAR_DATA *ch, char *argument ) {
    extern char str_boot_time[];
    char *suf;
    int day;

    day     = time_info.day + 1;

         if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    sprintf_to_char(ch, "It is %d%s, %s, %d%s %s, %s %d.\r\n"
                  "%s started up at %s\r"
                  "The system time is %s\r",
                  (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
                   time_info.hour >= 12 ? "pm" : "am",
                   day_name[day % 7],
                   day,
                   suf,
                   month_name[time_info.month],
                   season_names[time_info.season],
                   time_info.year,
                   mud.name,
                   str_boot_time,
                  (char *) ctime( &current_time ) );
    if (IS_IMMORTAL(ch)) sprintf_to_char(ch, "Max {g%d{x player were connected simultaneously during this period.\r\n", mud.player_count);
    return;
}


static char * const sky_look[4] = {
	"cloudless",
	"cloudy",
	"rainy",
	"lit by flashes of lightning"
    };

static char * const sky_look_winter[4] = {
        "crystal clear",
        "cloudy",
        "icy",
        "snowing heavily"
    }; 


void do_help( CHAR_DATA *ch, char *argument ) {
HELP_DATA *pHelp;
MOB_CMD_CONTEXT *mcc;
char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];
char buf[MAX_STRING_LENGTH];


    if ( argument[0] == '\0' ) {
         if (IS_SET( ch->plr, PLR_IMP_HTML)) {
              argument = "impsummary";
         } else {
              argument = "summary";
         }
     }

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while (argument[0] != '\0' )     {
	argument = one_argument(argument,argone);
	if (argall[0] != '\0') strcat(argall," ");
	strcat(argall,argone);
    }

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )    {
	if ( pHelp->level > get_trust( ch ))  continue;

	if ( is_name( argall, pHelp->keyword ) )	{
	    if ( pHelp->level >= 0 && str_cmp( argall, "imotd" ) )	    {
		send_to_char( pHelp->keyword, ch );
		send_to_char( "\r\n", ch );
	    }

                    mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, pHelp->level, pHelp->keyword);
                    expand_by_context(pHelp->text, mcc, buf);

	    /*
	     * Strip leading '.' to allow initial blanks.
	     */

	    if (buf[0] == '.' )	page_to_char(buf+1, ch );
	    else page_to_char(buf  , ch );
                    free_mcc(mcc);
	    return;
	}
    }

    if (!read_help_entry(ch,argall)) {
        send_to_char( "No local help found on that word.\r\n", ch );
        if (IS_SET( ch->plr, PLR_IMP_HTML)) {
           do_help (ch, "impsummary");
        } else {
           do_help (ch, "nohelp");
        }
    }
    return;
}


void do_whois (CHAR_DATA *ch, char *argument) {
char arg[MAX_INPUT_LENGTH];
char output[MAX_STRING_LENGTH * 4];
char buf[MAX_STRING_LENGTH];
char buf2[MAX_STRING_LENGTH];
DESCRIPTOR_DATA *d;
LPROF_DATA *prof;
CHAR_DATA *wch;
SOCIETY *society;
SOCIALITE *socmemb;
MEMBERSHIP *memb;
bool found = FALSE;

    one_argument(argument,arg);
  
    if (arg[0] == '\0')    {
	send_to_char("You must provide a name.\r\n",ch);
	return;
    }

    output[0] = '\0';

    for (d = descriptor_list; d != NULL; d = d->next)    {

 	if (d->connected != CON_PLAYING) continue;
	wch = ( d->original != NULL ) ? d->original : d->character;

	if (!str_prefix(arg,wch->name)) {
                    found = TRUE;
                    if (str_cmp(wch->pcdata->immtitle, "-GOD-") && str_cmp(wch->pcdata->immtitle, "SuperHero")) {
                         sprintf(buf2, "%s", wch->pcdata->immtitle);
                    } else {
                        char_div(wch, buf2);
                    }   

	    sprintf(buf, "{w[{Y%2d {W%s{w]{w %s%s\r\n",wch->level,buf2,wch->name,IS_NPC(wch) ? "" : wch->pcdata->title);
	    strcat(output,buf);

                    prof=wch->profs;
                    sprintf(buf, "  the %s %s %s.\r\n",wch->sex == 0 ? "sexless" : wch->sex == 1 ? "male" : "female",race_array[wch->race].name,prof->profession->name);
                    strcat(output, buf);

                    memb = wch->memberships;
                    while (memb) {

                        if (memb->level >= 0) {
                             society = memb->society;
                             if (!society->secret) {
                                 if (IS_NPC(wch)) {
                                     socmemb = find_socialite(society, wch->short_descr);
                                 } else {
                                     socmemb = find_socialite(society, wch->name);
                                 }
                                 if (socmemb) {
                                     if (socmemb->rank == SOC_RANK_LEADER) {
                                         sprintf(buf, "%s is Leader of %s.\r\n", wch->short_descr, society->name);
                                         strcat(output, buf);
                                     } else {
                                         if (IS_SET(socmemb->authority, SOC_AUTH_INVITE)) {
                                             sprintf(buf, "%s is Recruiter of %s.\r\n", wch->short_descr, society->name);
                                             strcat(output, buf);
                                         }
                                         if (IS_SET(socmemb->authority, SOC_AUTH_PROMOTE)
                                         && IS_SET(socmemb->authority, SOC_AUTH_DEMOTE)) {
                                             sprintf(buf, "%s is Master of Ceremony of %s.\r\n", wch->short_descr, society->name);
                                             strcat(output, buf);
                                         }
                                         if (IS_SET(socmemb->authority, SOC_AUTH_FOE)
                                         && IS_SET(socmemb->authority, SOC_AUTH_PARDON)) {
                                             sprintf(buf, "%s is Commander of %s.\r\n", wch->short_descr, society->name);
                                             strcat(output, buf);
                                         }
                                         if (IS_SET(socmemb->authority, SOC_AUTH_BANK)
                                         && IS_SET(socmemb->authority, SOC_AUTH_TAX)) {
                                             sprintf(buf, "%s is Cashier of %s.\r\n", wch->short_descr, society->name);
                                            strcat(output, buf);
                                         }
                                     }                                                                                 
                                 }
                             }
                        }
                        if (memb) memb = memb->next;
                    }   

                    if( wch->pcdata->image != NULL
                    && wch->pcdata->image[0] != '\0' ) {
                           if ( IS_SET(ch->plr, PLR_IMP_HTML) ) {
                                  sprintf(buf,"<IMG SRC=\"%s\" ALT=\"%s's image\">\r\n",wch->pcdata->image,wch->name);
                           } else {
                                  sprintf(buf, "{W%s's image is..: {c%s\r\n",wch->name,wch->pcdata->image);
                           } 
                           strcat(output, buf);
                    }

                     if( wch->pcdata->url != NULL
                     && wch->pcdata->url[0] != '\0') {
                           if ( IS_SET(ch->plr, PLR_IMP_HTML) ) {
                                    sprintf(buf, "<A HREF=\"%s\">%s's homepage</A>\r\n",wch->pcdata->url,wch->name);
                           } else {
                                    sprintf(buf, "{W%s's hompage is: {c%s\r\n", wch->name,  wch->pcdata->url);
                           } 
                           strcat(output, buf);
                     }

                     if( wch->pcdata->email != NULL
                     && wch->pcdata->email[0] != '\0') {
                            if ( IS_SET(ch->plr, PLR_IMP_HTML) ) {
                                     sprintf(buf,"{W%s's email is: <A HREF=\"mailto:%s\">{C%s</A>\r\n",wch->name, wch->pcdata->email,wch->pcdata->email);
                            } else {
                                     sprintf(buf, "{W%s's email is..: {c%s\r\n", wch->name, wch->pcdata->email);
                            } 
                            strcat(output, buf);
                     }

                      if ( wch->bio[0] != '\0' ) {
                            strcat(output, "{WBio:{c\r\n");
                            strcat(output, wch->bio);  
                            strcat(output, "{x");
                      }

	}
    }

    if (!found) {
        do_pload(ch, arg);
         if ((wch = get_char_world( ch, arg ) ) == NULL )    {
               int rn;

               send_to_char("Not found locally...\r\n", ch);
               for (rn=0; rn < MAX_PARTNER; rn++) {
                    if (!partner_array[rn].name) continue;
                    sprintf_to_partner(rn, "mudwhois %s %s\r\n", ch->name, arg);
               }
	return;
         }

          if (IS_NPC(wch)) {
              send_to_char( "Not on a NPC.\r\n", ch );
              return;
          }

          if (str_cmp(wch->pcdata->immtitle, "-GOD-") && str_cmp(wch->pcdata->immtitle, "SuperHero")) {
                        sprintf(buf2, "%s", wch->pcdata->immtitle);
           } else {
                        char_div(wch, buf2);
           }   

            sprintf(buf, "{w[{Y%2d {W%s{w]{w %s%s\r\n",wch->level,buf2,wch->name,IS_NPC(wch) ? "" : wch->pcdata->title);
            strcat(output,buf);
            prof=wch->profs;
            sprintf(buf, "  the %s %s %s.\r\n",wch->sex == 0 ? "sexless" : wch->sex == 1 ? "male" : "female",race_array[wch->race].name,prof->profession->name);
            strcat(output, buf);

                    memb = wch->memberships;
                    while (memb) {

                        if (memb->level >= 0) {
                             society = memb->society;
                             if (!society->secret) {
                                 if (IS_NPC(wch)) {
                                     socmemb = find_socialite(society, wch->short_descr);
                                 } else {
                                     socmemb = find_socialite(society, wch->name);
                                 }
                                 if (socmemb) {
                                     if (socmemb->rank == SOC_RANK_LEADER) {
                                         sprintf(buf, "%s is Leader of %s.\r\n", wch->short_descr, society->name);
                                         strcat(output, buf);
                                     } else {
                                         if (IS_SET(socmemb->authority, SOC_AUTH_INVITE)) {
                                             sprintf(buf, "%s is Recruiter of %s.\r\n", wch->short_descr, society->name);
                                             strcat(output, buf);
                                         }
                                         if (IS_SET(socmemb->authority, SOC_AUTH_PROMOTE)
                                         && IS_SET(socmemb->authority, SOC_AUTH_DEMOTE)) {
                                             sprintf(buf, "%s is Master of Ceremony of %s.\r\n", wch->short_descr, society->name);
                                             strcat(output, buf);
                                         }
                                         if (IS_SET(socmemb->authority, SOC_AUTH_FOE)
                                         && IS_SET(socmemb->authority, SOC_AUTH_PARDON)) {
                                             sprintf(buf, "%s is Commander of %s.\r\n", wch->short_descr, society->name);
                                             strcat(output, buf);
                                         }
                                         if (IS_SET(socmemb->authority, SOC_AUTH_BANK)
                                         && IS_SET(socmemb->authority, SOC_AUTH_TAX)) {
                                             sprintf(buf, "%s is Cashier of %s.\r\n", wch->short_descr, society->name);
                                            strcat(output, buf);
                                         }
                                     }                                                                                 
                                 }
                             }
                        }
                        if (memb) memb = memb->next;
                    }   

            if( wch->pcdata->image != NULL
            && wch->pcdata->image[0] != '\0' ) {
                  if ( IS_SET(ch->plr, PLR_IMP_HTML) ) {
                                  sprintf(buf,"<IMG SRC=\"%s\" ALT=\"%s's image\">\r\n",wch->pcdata->image,wch->name);
                  } else {
                                  sprintf(buf, "{W%s's image is..: {c%s\r\n",wch->name,wch->pcdata->image);
                  } 
                  strcat(output, buf);
            }

            if( wch->pcdata->url != NULL
            && wch->pcdata->url[0] != '\0') {
                           if ( IS_SET(ch->plr, PLR_IMP_HTML) ) {
                                    sprintf(buf, "<A HREF=\"%s\">%s's homepage</A>\r\n",wch->pcdata->url,wch->name);
                           } else {
                                    sprintf(buf, "{W%s's hompage is: {c%s\r\n", wch->name,  wch->pcdata->url);
                           } 
                           strcat(output, buf);
            }

            if( wch->pcdata->email != NULL
            && wch->pcdata->email[0] != '\0') {
                            if ( IS_SET(ch->plr, PLR_IMP_HTML) ) {
                                     sprintf(buf,"{W%s's email is: <A HREF=\"mailto:%s\">{C%s</A>\r\n",wch->name, wch->pcdata->email,wch->pcdata->email);
                            } else {
                                     sprintf(buf, "{W%s's email is..: {c%s\r\n", wch->name, wch->pcdata->email);
                            } 
                            strcat(output, buf);
            }

            if ( wch->bio[0] != '\0' ) {
                            strcat(output, "{WBio:{c\r\n");
                            strcat(output, wch->bio);  
                            strcat(output, "{x");
            }
            do_punload(ch,arg);
    }

    page_to_char(output,ch);
    return;
}


void do_who( CHAR_DATA *ch, char *argument ) {
char buf[MAX_STRING_LENGTH];
char buf2[MAX_STRING_LENGTH];
char fname[MAX_INPUT_LENGTH]; 
char clanbuf[MAX_STRING_LENGTH];
char form[MAX_STRING_LENGTH];
char output[10 * MAX_STRING_LENGTH];
char arg[MAX_STRING_LENGTH];
char arg2[MAX_STRING_LENGTH];
DESCRIPTOR_DATA *d;
int iLevelLower;
int iLevelUpper;
int nNumber;
int immMatch=0;
int mortMatch=0;
bool fImmortalOnly;
struct who_slot *table[MAX_LEVEL + 1];
int counter, nl;
MEMBERSHIP *memb;
SOCIETY *society;

    for (counter=1;counter <= MAX_LEVEL;counter++) {
        table[counter] = NULL;
    }

   /* Set default arguments... */

    iLevelLower		= 0;
    iLevelUpper		= MAX_LEVEL;
    fImmortalOnly	= FALSE;

   /* Parse arguments... */

    one_argument( argument, arg2);
    if (arg2[0] !='\0'
    && !is_number(arg2)) {
        CHAR_DATA *mud;

        argument = one_argument( argument, arg2);
        if ((mud = get_char_world(ch, arg2)) != NULL) {
               int rn = am_i_partner(mud);

               sprintf_to_partner(rn, "mudwho %s\r\n", ch->name);
               return;
         }
    }        

    nNumber = 0;
    for ( ;; ) {
    
      argument = one_argument( argument, arg );

      if ( arg[0] == '\0' )
        break;

      if ( is_number( arg ) ) {
	switch ( ++nNumber ) {
	  case 1: 
            iLevelLower = atoi( arg ); 
            break;

	  case 2: 
            iLevelUpper = atoi( arg ); 
            break;

	  default:
	    send_to_char( "Only two level numbers allowed.\r\n", ch );
	    return;
	}
      } else {

       /* Look for classes to turn on... */
        if ( arg[0] == 'i' ) fImmortalOnly = TRUE;
      }
    }


    for ( d = descriptor_list; d != NULL; d = d->next ) {
    
      CHAR_DATA *wch;
	
     /* Check for match against restrictions... */
     /* Don't use trust as that exposes trusted mortals... */ 

      wch   = ( d->original != NULL ) ? d->original : d->character;
  
      if (d->connected != CON_PLAYING 
      || (!can_see(ch, wch ) && !IS_SET(wch->plr, PLR_AFK))) {
        continue;
      }

      if ( wch->level < iLevelLower
      ||   wch->level > iLevelUpper
      || ( fImmortalOnly  && !IS_DIVINE(wch))) {
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

    sprintf(buf, "\r\n{C--- The Gods of %s ---{x\r\n", mud.name);
    strcat(output, buf);
   
    for ( counter=MAX_LEVEL;counter>=1;counter--) { /*outside list loop*/
    
      CHAR_DATA *wch;
      struct who_slot *tmp=table[counter];

      if (tmp == NULL) continue;

     /* now, follow each chain to end */

      for ( ;tmp != NULL;tmp = tmp->next) {	
	
        wch = tmp->ch;

	if (wch == NULL) {
	  log_string ("Got null table->ch, argh.");
	  continue;
	}

        if (IS_MORTAL(wch)) continue;
		
	strcpy (form, "%20s  %s%s%s%s%s%s %s");

       /* Get clan details... */
	
        clanbuf[0] = '\0';

        memb = wch->memberships;

        while (memb != NULL) {

          if (memb->level >= 0) {
              society = memb->society;
    
              if ( society != NULL 
              && society->secret 
              && !IS_IMMORTAL(ch)
              && !is_member(ch, society)) {
                     society = NULL;
              } 
    
              if ( society != NULL
              && society->type == SOC_TYPE_CLAN) {
                    if (society->secret) {
	       sprintf(clanbuf, "[{r%s{x]", society->name);	
                    } else {
	       sprintf(clanbuf, "[{g%s{x]", society->name);	
                    }
                    memb = NULL;
              } 
          }
          if (memb) memb = memb->next;
        }   

        if ( !IS_NPC(wch)
        &&  IS_IMMORTAL(wch)
        &&  (str_cmp(wch->pcdata->immtitle, "-GOD-")) && (str_cmp(wch->pcdata->immtitle, "SuperHero"))) {
          sprintf(buf2, "%s", wch->pcdata->immtitle);
        } else {
          char_div(wch, buf2);
        }   

        sprintf(fname, "{x[{Y%3d {W%s{x]                        ", wch->level, buf2);

        nl = true_length(fname, 20);
        fname[nl] = '\0';

        if (wch != ch) {
              sprintf( buf, form, fname,
	      IS_SET(wch->plr, PLR_AFK) ? "{r(AFK){x " : "",
	      IS_SET(wch->plr, PLR_ANNOYING) ? "{r({mANNOYING{r){x " : "",
                      "", "",
	      wch->name,  IS_NPC(wch) ? "" : wch->pcdata->title, clanbuf);
         } else {
              sprintf( buf, form, fname,
	      IS_SET(wch->plr, PLR_AFK) ? "{r(AFK){x " : "",
                      "", "", "",
	      wch->name,  IS_NPC(wch) ? "" : wch->pcdata->title, clanbuf);
         }

  	if (IS_SET(wch->plr, PLR_WIZINVIS)) strcat (buf, " {r({cWizi{r){x");
	if (IS_SET(wch->plr, PLR_CLOAK)) strcat (buf, " {r({cCloak{r){x");
	if (!IS_NPC(wch) && (wch->desc!=NULL) && wch->desc->editor) strcat (buf, " {M(OLC){x ");

        if ( wch->desc != NULL
        && !wch->desc->ok ) {
          strcat(buf, " {c(linkdead){x");
        }

  	strcat (buf, "\r\n");

	if ( (strlen(output) + strlen(buf)) > (8*MAX_STRING_LENGTH+1) ) {
	  page_to_char (output,ch);
	  output[0]='\0';
	}

	strcat(output,buf);
      } /*end inner list loop*/

    } /*end countdown loop*/

  /* Now display the mortals... */
 
    sprintf(buf, "\r\n{G--- The Players of %s ---{x\r\n", mud.name);
    strcat(output, buf);
   
    for ( counter=MAX_LEVEL;counter>=1;counter--) /*outside list loop*/
    {
      CHAR_DATA *wch;
      struct who_slot *tmp=table[counter];

      if (tmp == NULL) /* no one at this level */
        continue;

     /* now, follow each chain to end */
      for ( ;tmp != NULL;tmp = tmp->next) {	
	
	wch = tmp->ch;

	if (wch == NULL) {
	  log_string ("Got null table->ch, argh.");
	  continue;
	}

                if (IS_IMMORTAL(wch)) continue;
		
	strcpy (form, "%20s  %s%s%s%s%s%s %s");
		 	
       /* Get clan details... */
	
        clanbuf[0] = '\0';

        memb = wch->memberships;

        while (memb != NULL) {

          if (memb->level >= 0) {
              society = memb->society;
    
              if ( society != NULL 
              && society->secret 
              && !IS_IMMORTAL(ch)
              && !is_member(ch, society)) {
                     society = NULL;
              } 
    
              if ( society != NULL
              && society->type == SOC_TYPE_CLAN) {
                    if (society->secret) {
	       sprintf(clanbuf, "[{r%s{x]", society->name);	
                    } else {
	       sprintf(clanbuf, "[{g%s{x]", society->name);	
                    }
                    memb = NULL;
              } 
          }
          if (memb) memb = memb->next;
        }   

        if ( !IS_NPC(wch)
        && get_divinity(wch) > DIV_LEV_HERO) {
          sprintf(buf2, "%s", wch->pcdata->immtitle);
        } else {
           if (wch->level>201) {
             sprintf(buf2, "%s", wch->pcdata->immtitle);
           } else {
             char_div(wch, buf2);
           }
        }   

        sprintf(fname, "{x[{Y%3d {W%s{x]                        ", wch->level, buf2);

        nl = true_length(fname, 20);
        fname[nl] = '\0';

        if (wch !=ch) {
                      sprintf( buf, form,
	      fname,
	      IS_SET(wch->plr, PLR_AFK) ? "{r(AFK){x " : "",
	      IS_SET(wch->plr, PLR_ANNOYING) ? "{r({mANNOYING{r){x " : "",
                      "", "",
	      wch->name, IS_NPC(wch) ? "" : wch->pcdata->title, clanbuf);
        } else {
                      sprintf( buf, form,
	      fname,
	      IS_SET(wch->plr, PLR_AFK) ? "{r(AFK){x " : "",
                      "", "", "",
	      wch->name, IS_NPC(wch) ? "" : wch->pcdata->title, clanbuf);
         }

  	if (IS_SET(wch->plr, PLR_WIZINVIS))
	  strcat (buf, " {r({cWizi{r){x");

	if (IS_SET(wch->plr, PLR_CLOAK))
	  strcat (buf, " {r({cCloak{r){x");
	
	if (!IS_NPC(wch) && (wch->desc!=NULL) && wch->desc->editor)
	  strcat (buf, " {M(OLC){x ");

        if ( wch->desc != NULL
          && !wch->desc->ok ) {
          strcat(buf, " {c(linkdead){x");
        }

  	strcat (buf, "\r\n");

	if ( (strlen(output) + strlen(buf)) > (8*MAX_STRING_LENGTH+1) ) {
	  page_to_char (output,ch);
	  output[0]='\0';
	}

	strcat(output,buf);
      } /*end inner list loop*/

    } /*end countdown loop*/

    sprintf( buf2, "\r\n"
                   "{yGODS found: {C%d{x  {yPlayers found:   {G%d{x\r\n",
					 immMatch, mortMatch );
    strcat(output,buf2);

    page_to_char( output, ch );

    return;
}


void char_div(CHAR_DATA *ch, char *buf) {

  if (IS_NEWBIE(ch)) {
    sprintf(buf, "Newbie");
  } else if (IS_CREATOR(ch)) {
    sprintf(buf, "Creator");
  } else if (IS_LESSER(ch)) {
    sprintf(buf, "Lesser");
  } else if (IS_GOD(ch)) {
    sprintf(buf, "God");
  } else if (IS_GREATER(ch)) {
    sprintf(buf, "Greater");
  } else if (IS_IMP(ch)) {
    sprintf(buf, "Supreme");
  } else if (IS_HERO(ch)) {
    sprintf(buf, "Hero");
  } else if (IS_INVESTIGATOR(ch)) {
    sprintf(buf, "Investigator"); 
  } else if (IS_HUMAN(ch)) {
    sprintf(buf, "Player"); 
  } else {
    sprintf(buf, "Unknown");
  }  

  return;
} 


void do_count ( CHAR_DATA *ch, char *argument ) {
    int count;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    count = 0;

    for ( d = descriptor_list; d != NULL; d = d->next )
        if ( d->connected == CON_PLAYING && can_see( ch, d->character ) )
	    count++;

    max_on = UMAX(count,max_on);

    if (max_on == count)
        sprintf(buf,"There are %d characters on, the most so far today.\r\n",
	    count);
    else
	sprintf(buf,"There are %d characters on, the most on today was %d.\r\n",
	    count,max_on);

    send_to_char(buf,ch);
}

void do_inventory( CHAR_DATA *ch, char *argument ) {
    send_to_char( "You are carrying:\r\n", ch );
    show_list_to_char( ch->carrying, ch, TRUE, TRUE );
    return;
}



void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int iWear;
    bool found;

    send_to_char( "You are using:\r\n", ch );
    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	    continue;

	send_to_char( where_name[iWear], ch );
	if ( can_see_obj( ch, obj)) {
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\r\n", ch );
	}
	else
	{
	    send_to_char( "something.\r\n", ch );
	}
	found = TRUE;
    }

    if ( !found )
	send_to_char( "Nothing.\r\n", ch );

    return;
}



void do_compare( CHAR_DATA *ch, char *argument ) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Compare what to what?\r\n", ch );
	return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\r\n", ch );
	return;
    }

    if (arg2[0] == '\0')
    {
	for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
	{
	    if (obj2->wear_loc != WEAR_NONE
	    &&  can_see_obj(ch,obj2)
	    &&  obj1->item_type == obj2->item_type
	    &&  (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
		break;
	}

	if (obj2 == NULL)
	{
	    send_to_char("You aren't wearing anything comparable.\r\n",ch);
	    return;
	}
    } 

    else if ( (obj2 = get_obj_carry(ch,arg2) ) == NULL )
    {
	send_to_char("You do not have that item.\r\n",ch);
	return;
    }

    msg		= NULL;
    value1	= 0;
    value2	= 0;

    if ( obj1 == obj2 )
    {
	msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
	msg = "You can't compare $p and $P.";
    }
    else
    {
	switch ( obj1->item_type )
	{
	default:
	    msg = "You can't compare $p and $P.";
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
	    value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
	    break;

	case ITEM_WEAPON:
	    if (obj1->pIndexData->new_format)
		value1 = (1 + obj1->value[2]) * obj1->value[1];
	    else
	    	value1 = obj1->value[1] + obj1->value[2];

	    if (obj2->pIndexData->new_format)
		value2 = (1 + obj2->value[2]) * obj2->value[1];
	    else
	    	value2 = obj2->value[1] + obj2->value[2];
	    break;
	}
    }

    if ( msg == NULL )
    {
	     if ( value1 == value2 ) msg = "$p and $P look about the same.";
	else if ( value1  > value2 ) msg = "$p looks better than $P.";
	else                         msg = "$p looks worse than $P.";
    }

    act( msg, ch, obj1, obj2, TO_CHAR );
    return;
}


/*
 * New do_areas to use helpfile.
 */

void do_areas( CHAR_DATA *ch, char *argument )
{
    do_help( ch, "areas" );
    return;
}


void do_where (CHAR_DATA *ch, char *argument)	{
char buf[MAX_STRING_LENGTH];

    if ( ch->in_room->area->name != NULL
    && ch->in_room->area->name[0] != '\0' ) sprintf (buf, "You are currently in: {%s\r\n", ch->in_room->area->name);
    else   		                                             sprintf (buf, "You are in an unnamed area, inform the IMPs asap.\r\n");
    send_to_char (buf, ch);

    if (str_cmp(ch->in_room->area->map, "None")) {
          sprintf(buf,"<IMG SRC=\"%s\" ALT=\"%s\">\r\n",ch->in_room->area->map, ch->in_room->area->name);
          send_to_char (buf, ch);
          return;
    } 
    if (str_cmp(zones[ch->in_room->area->zone].map, "None")) {
          sprintf(buf,"<IMG SRC=\"%s\" ALT=\"%s\">\r\n",zones[ch->in_room->area->zone].map, zones[ch->in_room->area->zone].name);
          send_to_char (buf, ch);
          return;
    }
    if (str_cmp(mud.map, "None")) {
          sprintf(buf,"<IMG SRC=\"%s\" ALT=\"CthulhuMud\">\r\n",mud.map);
          send_to_char (buf, ch);
          return;
    } else {
          send_to_char ("You got no map of that region...\r\n", ch);
    }
     return;
}


void do_consider( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char *msg = '\0';
    char buf[MAX_STRING_LENGTH];
    int diff;
    int hpdiff;

    int off1, off2, def1, def2;
    int skill, delta, armor;
    int eh1, eh2;
    int el1, el2;
  
    char *est;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Consider killing whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They're not here.\r\n", ch );
	return;
    }

    if (is_safe(ch,victim))
    {
	send_to_char("Don't even think about it.\r\n",ch);
	return;
    }

    diff = victim->level - ch->level;

         if ( diff <= -10 ) msg = "$N is not even worthy of your attention.";
    else if ( diff <=  -5 ) msg = "You could kill $N even on a bad day.";
    else if ( diff <=  -2 ) msg = "$N shouldn't be too hard.";
    else if ( diff <=   1 ) msg = "The {Yperfect match{x!";
    else if ( diff <=   4 ) msg = "With some luck & skill you could kill $N.";
    else if ( diff <=   9 ) msg = "Maybe you should purchase life insurance first.";
    else                    msg = "Would you like to borrow a pick & shovel?";

    act( msg, ch, NULL, victim, TO_CHAR );

    /* additions by king@tinuviel.cs.wcu.edu */
    hpdiff = ( ch->hit - victim->hit );

    if ( ( ( diff >= 0) && ( hpdiff <= 0 ) )
    || ( ( diff <= 0 ) && ( hpdiff >= 0 ) ) )
    {
        send_to_char( "Also,", ch );
    }
    else
    {
        send_to_char( "However,", ch );
    }

    if ( hpdiff >= 101 )
        sprintf(buf, " you are currently much healthier than $E.");
    if ( hpdiff <= 100 )
        sprintf(buf, " you are currently healthier than $E.");
    if ( hpdiff <= 50 )
        sprintf(buf, " you are currently slightly healthier than $E.");
    if ( hpdiff <= 25 )
        sprintf(buf, " you are a teensy bit healthier than $E.");
    if ( hpdiff <= 0 )
        sprintf(buf, " $E is a teensy bit healthier than you.");
    if ( hpdiff <= -25 )
        sprintf(buf, " $E is slightly healthier than you.");
    if ( hpdiff <= -50 )
        sprintf(buf, " $E is healthier than you.");
    if ( hpdiff <= -100 )
        sprintf(buf, " $E is much healthier than you.");

    act( buf, ch, NULL, victim, TO_CHAR );

   /* Work out your statistics... */

    off1 = get_skill(ch, get_sn_for_weapon(get_eq_char(ch, WEAR_WIELD)));
 
    off1 += ch->level + 50;

    def1 = get_skill(ch, gsn_parry);

    skill = get_skill(ch, gsn_shield_block);
    def1 = UMAX(def1, skill);

    skill = get_skill(ch, gsn_dodge);
    def1 = UMAX(def1, skill);

    def1 = (def1 * 5) / 4;

    def1 += ch->level + 50;

   /* Work out their statistics... */

    off2 = get_skill(victim, get_sn_for_weapon(get_eq_char(victim, WEAR_WIELD)));
 
    off2 += victim->level + 50;

    def2 = get_skill(victim, gsn_parry);

    skill = get_skill(victim, gsn_shield_block);
    def2 = UMAX(def2, skill);

    skill = get_skill(victim, gsn_dodge);
    def2 = UMAX(def2, skill);

    def2 = (def2 * 5) / 4;

    def2 += victim->level + 50;

   /* Work out you attacking them... */

    delta = off1 - def2;

    if (delta < -40) {
      est = "{RVery Bad{x";
    } else if (delta < -5) {
      est = "{rBad{x";
    } else if (delta > 40) {
      est = "{GVery Good{x";
    } else if (delta > 5) {
      est = "{gGood{x";
    } else {
      est = "{w50/50{x";
    }

    sprintf(buf, "You -> Them: Offence {W%6d{x vs Defence {Y%6d{x - %s\r\n",
                 off1, def2, est ); 

    send_to_char(buf, ch);       

   /* Work out them attacking you... */

    delta = off2 - def1;

    if (delta < -40) {
      est = "{GVery Good{x";
    } else if (delta < -5) {
      est = "{gGood{x";
    } else if (delta > 40) {
      est = "{RVery Bad{x";
    } else if (delta > 5) {
      est = "{rBad{x";
    } else {
      est = "{w50/50{x";
    }

    sprintf(buf, "Them -> You: Offence {Y%6d{x vs Defence {W%6d{x - %s\r\n",
                  off2, def1, est );

    send_to_char(buf, ch);

   /* Effective hits for player... */

    eh1 = ch->hit;

    armor = GET_AC(ch, AC_SLASH);

   /* Convert to linear scale... */ 

    armor = 100 - armor;

   /* Maximum is 600 pts - 1/64th damage */

    armor = UMIN(armor, ARMOR_DAM_MAX);

    while (armor > ARMOR_DAM_PTS) {

      eh1 *= 2;

      armor -= ARMOR_DAM_PTS;
    } 

    eh1 = ((ARMOR_DAM_PTS + armor) * eh1)/ (ARMOR_DAM_PTS);

   /* Effective hits for opponent... */

    eh2 = victim->hit;

    armor = GET_AC(victim, AC_SLASH);

   /* Convert to linear scale... */ 

    armor = 100 - armor;

   /* Maximum is 600 pts - 1/64th damage */

    armor = UMIN(armor, ARMOR_DAM_MAX);

    while (armor > ARMOR_DAM_PTS) {

      eh2 *= 2;

      armor -= ARMOR_DAM_PTS;
    } 

    eh2 = ((ARMOR_DAM_PTS + armor) * eh2)/ (ARMOR_DAM_PTS);

   /* Compare and contrast... */

    delta = (100 * eh1)/eh2;

    if (delta > 200) {
      est = "{GVery Good{x";
    } else if (delta > 125) {
      est = "{gGood{x";
    } else if (delta > 80) {
      est = "{x50/50{x";
    } else if (delta > 50) {
      est = "{rBad{x";
    } else {
      est = "{RVery Bad{x";
    }

    sprintf(buf, "Effective hits: Your {W%6d{x vs   their {Y%6d{x - %s\r\n",
                 eh1, eh2, est);

    send_to_char(buf, ch);

   /* Effective levels... */

    el1 = get_effective_level(ch);

    el2 = get_effective_level(victim);

    delta = el2 - el1;

    if (delta < -10) {
      est = "{CTo easy{x";
    } else if (delta < -3) {
      est = "{GEasy Fight{x";
    } else if (delta < -1) {
      est = "{GFair Fight{x";
    } else if (delta < 2) {
      est = "{YGood Fight{g"; 
    } else if (delta < 4) {
      est = "{YTough Fight{x"; 
    } else if (delta < 11) {
      est = "{RUphill Battle{x";
    } else {
      est = "{RForget it{x";
    } 

    sprintf(buf, "Effective level: Your {W%5d{x vs   their  {Y%5d{x - %s\r\n",
                 el1, el2, est);

    send_to_char(buf, ch);

   /* All done... */

    return;
}


void set_worship( CHAR_DATA *ch, char *cult ){
   char buf[MAX_STRING_LENGTH];
   int ct;

    if (IS_NPC(ch) )    {
	bug( "Worship: NPC.", 0 );
	return;
    }

    for(ct=1; ct<=MAX_CULT; ct++) {
      if (is_name(cult, cult_array[ct].name)) break;
    }

    if (ct > MAX_CULT
    || cult_array[ct].name == NULL) ct=1;

    sprintf(buf, "You now worship %s.\r\n", cult_array[ct].name);
    send_to_char(buf, ch );
    free_string(ch->pcdata->cult );
    ch->pcdata->cult = str_dup(cult_array[ct].name);
    return;
}


void set_title( CHAR_DATA *ch, char *title ) {
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    if ( title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?' )    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }    else    {
	strcpy( buf, title );
    }

    free_string( ch->pcdata->title );
    ch->pcdata->title = str_dup( buf );
    return;
}


void do_title( CHAR_DATA *ch, char *argument ) {

    if ( IS_NPC(ch) ) return;

    if ( argument[0] == '\0' )    {
	send_to_char( "Change your title to what?\r\n", ch );
	return;
    }

    if ( strlen(argument) > 45 )
	argument[45] = '\0';

    smash_tilde( argument );
    set_title( ch, argument );
    send_to_char( "Ok.\r\n", ch );
}


void do_worship( CHAR_DATA *ch, char *argument ) {
    char arg[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) ) return;

    if ( argument[0] == '\0' )    {
                list_cult(ch);
	return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg );
    if (!str_cmp(arg, "info")) {
       info_cult(ch, argument);
       return;
    }

    set_worship( ch, arg);
    send_to_char( "Ok.\r\n", ch );
}


void do_immtitle( CHAR_DATA *ch, char *argument ){
    if ( IS_NPC(ch)) return;
    
   if (ch->level < 201
   && !IS_IMMORTAL(ch)){
        send_to_char( "Wait until Level 201!\r\n", ch );
        return;
     }

    if ( argument[0] == '\0' )   {
        send_to_char( "Change your immtitle to what?\r\n", ch );
        return;
    }

    if ( strlen(argument) > 15 ) argument[15] = '\0';

    smash_tilde( argument );
    free_string( ch->pcdata->immtitle );
    ch->pcdata->immtitle = str_dup(argument);
    send_to_char( "ImmTitle set.\r\n", ch );
}



void do_email( CHAR_DATA *ch, char *argument ) {
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) ) {
        return;
    }

    if ( argument[0] == '\0' ) {
        if ( ch->pcdata->email == NULL
          || ch->pcdata->email[0] == '\0' ) { 
          send_to_char( "Your email address has not been set.\r\n", ch );
        } else {
          sprintf(buf, "{wYour email address is:{c%s{x\r\n",
                        ch->pcdata->email);
          send_to_char(buf, ch);
        } 
        return;
    }

    if ( strlen(argument) > 70 ) {
        argument[70] = '\0';
    }

    free_string( ch->pcdata->email );

    ch->pcdata->email = str_dup(argument);

    send_to_char( "Your email address has been set.\r\n", ch );

    return;
}

void do_image( CHAR_DATA *ch, char *argument ) {

    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) ) {
        return;
    }

    if ( argument[0] == '\0' ) {
        if ( ch->pcdata->image == NULL
          || ch->pcdata->image[0] == '\0' ) { 
          send_to_char( "Your image URL has not been set.\r\n", ch );
        } else {
          sprintf(buf, "{wYour image URL is:{c%s{x\r\n",
                        ch->pcdata->image);
          send_to_char(buf, ch);
          if (IS_SET(ch->plr, PLR_IMP_HTML)) {
            sprintf(buf, "<IMG SRC=\"%s\" ALT=\"%s's image\">\r\n",
                          ch->pcdata->image,
                          ch->short_descr); 
            send_to_char(buf, ch);
          }
        } 
        return;
    }

    if ( strlen(argument) > 70 ) {
        argument[70] = '\0';
    }

    free_string( ch->pcdata->image );

    ch->pcdata->image = str_dup(argument);

    send_to_char( "Your image URL has been set.\r\n", ch );

    return;
}


void do_homepage( CHAR_DATA *ch, char *argument ) {

    if ( IS_NPC(ch)) return;

    if ( argument[0] == '\0' ) {
        if ( ch->pcdata->url == NULL
          || ch->pcdata->url[0] == '\0' ) { 
          send_to_char( "Your homepage URL has not been set.\r\n", ch );
        } else {
          sprintf_to_char(ch, "{wYour homepage URL is:{c%s{x\r\n", ch->pcdata->url);
        } 
        return;
    }

    if ( strlen(argument) > 70 ) argument[70] = '\0';

    free_string( ch->pcdata->url );
    ch->pcdata->url = str_dup(argument);
    send_to_char( "Your homepage URL has been set.\r\n", ch );
    return;
}


void do_bio(CHAR_DATA *ch, char *args) {

    if ( ch->desc == NULL ) {
      send_to_char("You may not edit your biography!\r\n", ch);
      return;
    }

    string_append(ch, &ch->bio);
    return;
}


void do_description( CHAR_DATA *ch, char *argument ) {

    if ( ch->desc == NULL ) {
      send_to_char("You may not edit your description!\r\n", ch);
      return;
    }

    string_append(ch, &ch->description);
    return;
}


void do_report( CHAR_DATA *ch, char *argument ) {
char buf[MAX_INPUT_LENGTH];

    sprintf_to_char(ch, "You say 'I have %d/%d hp %d/%d mana %d/%d mv.'\r\n", ch->hit,  ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move);
    sprintf( buf, "$n says 'I have %d/%d hp %d/%d mana %d/%d mv.'", ch->hit,  ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move);
    act( buf, ch, NULL, NULL, TO_ROOM );
    return;
}


void do_wimpy( CHAR_DATA *ch, char *argument ) {
char buf[MAX_STRING_LENGTH];
char arg[MAX_INPUT_LENGTH];
int wimpy;
int door; 

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' ) {
	wimpy = ch->max_hit / 5;
    } else {
	wimpy = atoi( arg );
    }

    if ( wimpy < 0 ) {
	send_to_char( "Your courage exceeds your wisdom.\r\n", ch );
	return;
    }

    if ( wimpy > ch->max_hit/2 ) {
	send_to_char( "Such cowardice ill becomes you.\r\n", ch );
	return;
    }

    ch->wimpy	= wimpy;
    sprintf( buf, "Wimpy set to %d hit points.\r\n", wimpy );
    send_to_char( buf, ch );

    if (argument[0] != '\0') {
      if (!str_cmp(argument, "last")) {
          ch->wimpy_dir = -1 + DIR_NONE;
          send_to_char("Wimpy direction set to last move.\r\n", ch);
          return;
      }

      door = door_index2(ch, argument);

      if ( door > DIR_NONE && door < DIR_MAX ) {
        ch->wimpy_dir = door;
        sprintf(buf, "Wimpy direction set to '%s'.\r\n", dir_name[door]);
        send_to_char(buf, ch);
      } else {
        ch->wimpy_dir = DIR_NONE;
        send_to_char("Wimpy direction set to random.\r\n", ch);
      }
    }

    return;
}


void do_password( CHAR_DATA *ch, char *argument ) {
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
char *pArg;
char *pwdnew;
char *p;
char cEnd;

    if ( IS_NPC(ch)) return;

    pArg = arg1;
    while ( isspace(*argument)) argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' ) cEnd = *argument++;

    while ( *argument != '\0' ) {
	if ( *argument == cEnd ) {
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace(*argument)) argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' ) cEnd = *argument++;

    while ( *argument != '\0' ) {
	if ( *argument == cEnd ) {
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )  {
                if (ch->pcdata->moddiv >= DIV_CREATOR) {
                   sprintf_to_char(ch, "PASSWORD: %s\r\n", ch->pcdata->pwd);
                }
	send_to_char( "Syntax: password <old> <new>.\r\n", ch );
	return;
    }

    if ( str_cmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )    {
	WAIT_STATE( ch, 40 );
	send_to_char( "Wrong password.  Wait 10 seconds.\r\n", ch );
	return;
    }

    if ( strlen(arg2) < 5 ) {
	send_to_char("New password must be at least five characters long.\r\n", ch );
	return;
    }

    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ ) {
	if ( *p == '~' ) {
	    send_to_char("New password not acceptable, try again.\r\n", ch );
	    return;
	}
    }

    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    save_char_obj( ch );
    send_to_char( "Ok.\r\n", ch );
    return;
}


void do_xinfo (CHAR_DATA *ch, char *argument) {

    if (IS_SET(ch->plr, PLR_XINFO)) {
        send_to_char ("Xinfo is now off.\r\n",ch);
        ch->plr -= PLR_XINFO;
    } else {
        send_to_char ("Xinfo is now on.\r\n",ch);
        ch->plr += PLR_XINFO;
    }

    return;
}


bool is_outside (CHAR_DATA *ch) {
	
    if (IS_SET(ch->in_room->room_flags, ROOM_INDOORS)) {
        return FALSE;
    }

    switch (ch->in_room->sector_type) {
      case SECT_INSIDE:
      case SECT_UNDERGROUND:
      case SECT_UNDERWATER:
      case SECT_SPACE:
      case SECT_SMALL_FIRE:
      case SECT_FIRE:
      case SECT_BIG_FIRE:
        return FALSE;

      default:
        break;
    }

    return TRUE;
} 


void do_petname(CHAR_DATA *ch, char *argument) {
  char buf[MAX_STRING_LENGTH];
  char command[MAX_INPUT_LENGTH];

  smash_tilde(argument);

  if ( ch->pet == NULL )  {
      send_to_char("You dont have a pet!\r\n", ch);
      return;
  }

  if (IS_SET(ch->pet->form, FORM_SENTIENT)) {
         send_to_char("He's quite happy with his name!\r\n", ch);
         return;
  }

  if (!str_cmp(race_array[ch->pet->race].name,"machine")
  || IS_SET(ch->pet->form, FORM_MACHINE)) {
          send_to_char( "Name a machine?!\r\n", ch );
          return;
  }

  if ( ch->in_room != ch->pet->in_room )  {
      send_to_char("Hard for your pet to learn his new name\r\n",ch);
      send_to_char("if he's not even with you!\r\n", ch);
      return;
  }

  argument = one_argument(argument, command);

  if ( command[0] == '\0' ||  argument[0] == '\0' )  {
      send_to_char("\r\nsyntax: petname [name|short|long] <argument>\r\n",ch);
      send_to_char("See \"help petname\" and \"help color\" for more information.\r\n",ch);
      return;
  }

  if ( !str_prefix(command, "name") )  {
      if ( argument[0] == '\0' ) return;
      free_string(ch->pet->name);
      ch->pet->name = str_dup(argument);
      sprintf(buf, "%s's name set.\r\n", ch->pet->name);
      send_to_char(buf, ch);

  } else  if ( !str_prefix(command, "short") )  {
      if ( argument[0] == '\0' ) return;
      free_string(ch->pet->short_descr);
      ch->pet->short_descr = str_dup(argument);
      sprintf(buf, "%s's short description set to: \r\n%s\r\n", ch->pet->name, ch->pet->short_descr);
      send_to_char(buf, ch);

  } else  if ( !str_prefix(command, "long") )   {
      if ( argument[0] == '\0' ) return;
      free_string(ch->pet->long_descr);
      sprintf(buf, "%s\r\n", argument);
      ch->pet->long_descr = str_dup(buf);
      sprintf(buf, "%s's long description set to: \r\n%s\r\n", ch->pet->name, ch->pet->long_descr);
      send_to_char(buf, ch);
  } else   do_help(ch, "petname");

  return;
}


void do_body( CHAR_DATA *ch, char *argument ) {
char buf[MAX_STRING_LENGTH];
char arg[MAX_STRING_LENGTH];
CHAR_DATA *victim;

  if ( argument[0] == '\0') {
        victim=ch;
  } else {
        one_argument(argument,arg);
        if ( ( victim = get_char_room( ch, arg ) ) == NULL )    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
        }
  }
  sprintf(buf,"{g*********************************{x\r\n");
  send_to_char( buf, ch );
  sprintf(buf,"{g* {W%15s : %15s {g*{x\r\n","Head",  flag_string(limb_status,victim->limb[0])); 
  send_to_char( buf, ch );
  sprintf(buf,"{g* {W%15s : %15s {g*{x\r\n","Torso",  flag_string(limb_status,victim->limb[1]));
  send_to_char( buf, ch );
  sprintf(buf,"{g* {W%15s : %15s {g*{x\r\n","Left Arm",  flag_string(limb_status,victim->limb[2]));
  send_to_char( buf, ch );
  sprintf(buf,"{g* {W%15s : %15s {g*{x\r\n","Right Arm",  flag_string(limb_status,victim->limb[3]));
  send_to_char( buf, ch );
  sprintf(buf,"{g* {W%15s : %15s {g*{x\r\n","Legs",  flag_string(limb_status,victim->limb[4]));
  send_to_char( buf, ch );
  sprintf(buf,"{g* {W%15s : %15s {g*{x\r\n","","");
  send_to_char( buf, ch );
  sprintf(buf,"{g*********************************{x\r\n");
  send_to_char( buf, ch );

  return;
}


void do_remember( CHAR_DATA *ch, char *argument ) {
OBJ_DATA *obj;
char arg[MAX_STRING_LENGTH];

    if (argument[0] == '\0') {
           send_to_char( "Which object do you want to remember.\r\n", ch );
           return;
    }

    one_argument(argument, arg);
    obj = get_obj_here(ch, arg);

    if (obj == NULL) {
           send_to_char( "That's not here.\r\n", ch );
           return;
    }

    if (obj->level > ch->level + 15) {
           send_to_char( "You doubt you'll remember that for long.\r\n", ch );
           return;
    }

   if (IS_SET(obj->extra_flags, ITEM_ARTIFACT)) {
      sprintf_to_char(ch, "%s is much too powerful!\r\n", capitalize(obj->short_descr));
      return;
   }

    if (obj->item_type == ITEM_CONTAINER
    || obj->item_type == ITEM_DRINK_CON
    || (obj->item_type == ITEM_PORTAL && obj->value[4] == PORTAL_BUILDING)
    || (obj->item_type == ITEM_PORTAL && obj->value[4] == PORTAL_VEHICLE)) {
           send_to_char( "That object is too complex to remember.\r\n", ch );
           return;
    }

    ch->trans_obj = obj;
    send_to_char( "You try to memorize what the object looks like.\r\n", ch );
    act("$n stares at $p for quite a while.", ch, obj, NULL, TO_ROOM );

    return;
}


void show_charge(CHAR_DATA *ch, AFFECT_DATA *af) {
       if (af->location == APPLY_EFFECT
       && af->skill > 0) {
           meter(ch, effect_array[af->skill].name, af->modifier);
       }
       return;
}


char *get_align(int align) {

    if (align >  900 ) return "{Wangelic{x";
    else if (align >  700 ) return "{Wsaintly{x";
    else if (align >  350 ) return "{Cgood{x";
    else if (align >  100 ) return "{Ckind{x";
    else if (align > -100 ) return "neutral{x";
    else if (align > -350 ) return "{rmean{x";
    else if (align > -700 ) return "{revil{x";
    else if (align > -900 ) return "{Rdemonic{x";
    else return "{Rsatanic{x";
}


long get_full_cash(CHAR_DATA *ch) {
int i;
int usual_c;
long cash = 0;
float trans;

    usual_c = find_currency(ch);
    
    for (i = 0; i < MAX_CURRENCY; i++) {
          if (ch->gold[i] > 0) {
                if (i == usual_c) {
                     cash += ch->gold[i];
                } else {
                     trans = ch->in_room->area->worth[i] / 100.0;
                     cash += (long) (ch->gold[i] * trans);
                }
          }
    }

    return cash;
}


int find_currency(CHAR_DATA *ch) {
int i;
int usual_c = 0;

    for (i = 0; i < MAX_CURRENCY; i++) {
          if (!ch->in_room) return 0;
          if (!ch->in_room->area) return 0;

          if (ch->in_room->area->worth[i] == 100) {
              usual_c = i;
              break;
          }
    }
    return usual_c;
}


int find_currency_unreset(CHAR_DATA *ch) {
int i;
int usual_c = 0;

    if (!IS_NPC(ch)) return 0;

    for (i = 0; i < MAX_CURRENCY; i++) {
          if (!ch->pIndexData) return 0;
          if (!ch->pIndexData->area) return 0;

          if (ch->pIndexData->area->worth[i] == 100) {
              usual_c = i;
              break;
          }
    }
    return usual_c;
}


int find_obj_currency(OBJ_DATA *obj) {
int i;
int usual_c = 0;

    for (i = 0; i < MAX_CURRENCY; i++) {
          if (obj->pIndexData->area->worth[i] == 100) {
              usual_c = i;
              break;
          }
    }
    return usual_c;
}


float get_currency_modifier(AREA_DATA *area, int currency) {
float modi;

      if (area->worth[currency] == 0) return 0;

      if (area->worth[currency] == -1) {
         modi = mud.worth[currency] / 100.0;
      } else {
         modi = area->worth[currency] / 100.0;
      }
      return modi;
}


void meter(CHAR_DATA *ch, char *name, int value) {
char buf[MAX_STRING_LENGTH];
char buf2[MAX_STRING_LENGTH];
char buf3[80];
int i;

            value = URANGE(0, value, 100);

            sprintf(buf3,"|");

            for (i = 0; i < 20; i++) {
               if (i < value /5) {
                  if (i < 9) {
                      strcat(buf3, "{r=");
                  } else if (i < 16) {
                      strcat(buf3, "{y=");
                  } else {
                      strcat(buf3, "{g=");
                  }
               } else {
                  strcat(buf3, " ");
               }
            }
            strcat(buf3, "{x|\r\n");

            sprintf(buf, "{x--------------------------------------------\r\n");
            sprintf(buf2, "|{W %20s{x", name);
            strcat(buf, buf2);
            sprintf(buf2, "%20s", buf3);
            strcat(buf, buf2);
            strcat(buf, "{x--------------------------------------------\r\n");
            send_to_char(buf, ch);
            return;
}


int true_length(char *name, int length) {
int tl = 0;
int count = 0;

     while (tl < length) {
        if (name[count] == '\0') return count;

        if (name[count] == '{') tl--;
        else tl++;

        count++;       
     }

     return count;
}

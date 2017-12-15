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
#include "music.h"
#include "mob.h"
#include "wev.h"
#include "gsn.h"
#include "skill.h"

DECLARE_DO_FUN(do_music);
DECLARE_DO_FUN(do_make_music);


int channel_songs[MAX_GLOBAL + 1];

struct song_data song_table[MAX_SONGS];
struct mstyle_data music_styles[MAX_MSTYLES];


void song_update(void) {
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *room;
    char buf[MAX_STRING_LENGTH];
    char *line;
    int i;

    /* do the global song, if any */
    if (channel_songs[1] >= MAX_SONGS) channel_songs[1] = -1;

    if (channel_songs[1] > -1)  {
        if (channel_songs[0] >= MAX_LINES
        ||  channel_songs[0] >= song_table[channel_songs[1]].lines) {
	    channel_songs[0] = -1;

            /* advance songs */
	    for (i = 1; i < MAX_GLOBAL; i++) channel_songs[i] = channel_songs[i+1];
	    channel_songs[MAX_GLOBAL] = -1;
	} else {
	    if (channel_songs[0] < 0) {
	    	sprintf(buf,"%s, %s", song_table[channel_songs[1]].group,  song_table[channel_songs[1]].name);
               	                channel_songs[0] = 0;
	    } else {
		sprintf(buf,"%s", song_table[channel_songs[1]].lyrics[channel_songs[0]]);
		channel_songs[0]++;
	    }

                    do_music(NULL, buf);
	}
    }

    for (obj = object_list; obj != NULL; obj = obj->next) {
	if (obj->item_type != ITEM_JUKEBOX || obj->value[1] < 0) continue;

 	if (obj->value[1] >= MAX_SONGS) {
	    obj->value[1] = -1;
	    continue;
	}

	/* find which room to play in */

	if ((room = obj->in_room) == NULL) {
	    if (obj->carried_by == NULL) {
                         continue;
	    } else {
	    	if ((room = obj->carried_by->in_room) == NULL)  continue;
                    }
	}

	if (obj->value[0] < 0) {
	    sprintf(buf,"$p starts playing %s, %s.", song_table[obj->value[1]].group,song_table[obj->value[1]].name);
	    if (room->people != NULL) act(buf,room->people,obj,NULL,TO_ALL);
	    obj->value[0] = 0;
	    continue;
	} else {
	    if (obj->value[0] >= MAX_LINES 
	    || obj->value[0] >= song_table[obj->value[1]].lines) {
		obj->value[0] = -1;

		/* scroll songs forward */
		obj->value[1] = obj->value[2];
		obj->value[2] = obj->value[3];
		obj->value[3] = obj->value[4];
		obj->value[4] = -1;
		continue;
	    }

	    line = song_table[obj->value[1]].lyrics[obj->value[0]];
	    obj->value[0]++;
	}

	sprintf(buf,"$p bops: '%s'",line);
	if (room->people != NULL) act(buf,room->people,obj,NULL,TO_ALL);
    }
}

	    

void load_songs(void) {
    FILE *fp;
    int count = 0, lines, i;
    char letter;

    /* reset global */
    for (i = 0; i <= MAX_GLOBAL; i++) channel_songs[i] = -1;

    fp = fopen(MUSIC_FILE,"r");

    if (fp == NULL) {
	bug("Couldn't open music file, no songs.",0);
	return;
    }

    for (count = 0; count < MAX_SONGS; count++) {
        letter = fread_letter(fp);
        if (letter == '#')  {
            if (count < MAX_SONGS)
                song_table[count].name = NULL;
            fclose(fp);
            return;
        } else ungetc(letter,fp);

	song_table[count].group = fread_string(fp);
	song_table[count].name 	= fread_string(fp);

	/* read lyrics */
	lines = 0;

	for ( ; ;)	{
	    letter = fread_letter(fp);

	    if (letter == '~')    {
		song_table[count].lines = lines;
		break;
	    } else	ungetc(letter,fp);
		
	    if (lines >= MAX_LINES)   {
		bug("Too many lines in a song -- limit is  %d.",MAX_LINES);
		break;
	    }

	    song_table[count].lyrics[lines] = fread_string_eol(fp);
	    lines++;
	}
    }
}


void do_play(CHAR_DATA *ch, char *argument) {
OBJ_DATA *juke;
OBJ_DATA *instrument;
char *str,arg[MAX_INPUT_LENGTH];
char buf[MAX_STRING_LENGTH];
int song,i;
bool global = FALSE;

    str = one_argument(argument,arg);

    for (juke = ch->in_room->contents; juke != NULL; juke = juke->next_content)
	if (juke->item_type == ITEM_JUKEBOX && can_see_obj(ch,juke))    break;

    if (argument[0] == '\0')    {
	send_to_char("Syntax:\r\n",ch);
	send_to_char("  PLAY LIST\r\n",ch);
	send_to_char("  PLAY <song>\r\n",ch);
	return;
    }

    if (!juke)    {
        int i;

        if (argument[0] =='*') {
            do_make_music(ch, argument);
            return;
        }

        if ((instrument = get_obj_here(ch, arg)) != NULL) {
             if (instrument->level > ch->level +10) {
	send_to_char("You are too inexperienced to use that instrument.\r\n",ch);
	return;
             }
 
             if (instrument->item_type == ITEM_INSTRUMENT) {
                if (str[0] == '\0') {
                    sprintf_to_char(ch, "Styles you can play with %s:\r\n", instrument->short_descr);
                    for (i = 1; i < MAX_MSTYLES; i++) {
                       if (music_styles[i].instr[instrument->value[0]]) sprintf_to_char(ch, "%s /", music_styles[i].name);
                    }
                    send_to_char("\r\n",ch);
                    return;
                } 

                if (ch->in_room->sector_type == SECT_SPACE
                && instrument->value[0] != INSTR_CRYSTAL) {
                      send_to_char("You can't play that in space.\r\n",ch);
                      return;
                }

                 sprintf(buf, "play %s", argument);
                 do_make_music(ch, buf);
                 return;
             }
        }

        send_to_char("You see nothing to play.\r\n",ch);
        return;
    }

     if (juke->condition < 5) {
             send_to_char( "It's much too damaged.\r\n", ch );
             return;
     }

    if (!str_cmp(arg,"list"))    {
                char buffer[10*MAX_STRING_LENGTH];
  	char buf[MAX_STRING_LENGTH];
	int col = 0;
	bool artist = FALSE, match = FALSE;

	argument = str;
	argument = one_argument(argument,arg);

	if (!str_cmp(arg,"artist"))
	    artist = TRUE;

	if (argument[0] != '\0')
	    match = TRUE;

	sprintf(buffer,"{c%s has the following songs available:{x\r\n", capitalize(juke->short_descr));

	for (i = 0; i < MAX_SONGS; i++) {
	    if (song_table[i].name == NULL)	break;

	    if (artist && (!match 
	    || !str_prefix(argument,song_table[i].group)))
		sprintf(buf,"%-39s %-39s\r\n",  song_table[i].group,song_table[i].name);
	    else if (!artist && (!match 
	    || !str_prefix(argument,song_table[i].name)))
	    	sprintf(buf,"%-35s ",song_table[i].name);
	    else continue;
                    strcat(buffer,buf);
	    if (!artist && ++col % 2 == 0) strcat(buffer, "\r\n");
        }
        if (!artist && col % 2 != 0)  strcat(buffer, "\r\n");
        page_to_char(buffer,ch);
        return;
    }

    if (argument[0] == '\0')  {
        send_to_char("Play what?\r\n",ch);
        return;
    }

    global = TRUE;

    if ((global && channel_songs[MAX_GLOBAL] > -1) 
    ||  (!global && juke->value[4] > -1))   {
        send_to_char("The jukebox is full up right now.\r\n",ch);
        return;
    }

    for (song = 0; song < MAX_SONGS; song++) {
	if (song_table[song].name == NULL) {
	    send_to_char("That song isn't available.\r\n",ch);
	    return;
	}
	if (!str_prefix(argument,song_table[song].name))  break;
    }

    if (song >= MAX_SONGS) {
	send_to_char("That song isn't available.\r\n",ch);
	return;
    }

    send_to_char("Coming right up.\r\n",ch);

    if (global) {
	for (i = 1; i <= MAX_GLOBAL; i++)
	    if (channel_songs[i] < 0)  {
		if (i == 1)  channel_songs[0] = -1;
		channel_songs[i] = song;
		return;
	    }
    } else  {
	for (i = 1; i < 5; i++)
	    if (juke->value[i] < 0) {
		if (i == 1) juke->value[0] = -1;
		juke->value[i] = song;
		return;
	     }
    }
}


void do_sing(CHAR_DATA *ch, char *argument ) {
char buf[MAX_STRING_LENGTH];
int i;

    if (argument[0] == '\0') {
       send_to_char("Styles you can sing:\r\n", ch);
       for (i = 1; i < MAX_MSTYLES; i++) {
           if (music_styles[i].instr[0]) sprintf_to_char(ch, "%s /", music_styles[i].name);
       }
       send_to_char("\r\n",ch);
       return;
    }

    if (ch->in_room->sector_type == SECT_UNDERWATER
    && (!IS_AFFECTED(ch, AFF_SWIM) || !IS_AFFECTED(ch, AFF_WATER_BREATHING))) {
       send_to_char("You can't do that underwater.\r\n",ch);
       return;
    }

    if (ch->in_room->sector_type == SECT_SPACE) {
       send_to_char("You can't do that in space.\r\n",ch);
       return;
    }

     if (argument[0] =='*') {
            do_make_music(ch, argument);
     } else {
            sprintf(buf, "sing %s", argument);
            do_make_music(ch, buf);
     }
     return;
}


void do_make_music(CHAR_DATA *ch, char *argument ) {
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
char arg3[MAX_INPUT_LENGTH];
char buf[MAX_STRING_LENGTH];
OBJ_DATA *instrument;
MOB_CMD_CONTEXT *mcc;
WEV *wev;
int style, skill, eff;

    if (!check_activity_key(ch, argument)) return;

    if (ch->activity == ACV_MUSIC && argument[0] != '*' ) {
      send_to_char("You are already making some music!\r\n", ch);
      return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if (arg1[0] == '\0' ) {
	send_to_char( "What do you want to do?\r\n", ch );
	return;
    }

    if (ch->activity != ACV_MUSIC) ch->acv_state = 0;

    if ((skill = get_skill(ch, gsn_music)) == 0) {
          send_to_char("You don't like music very much!\r\n", ch);
          return;
    }
    
    switch ( ch->acv_state ) {
      default:
          if (!str_cmp(arg1, "sing")) {
              if ((style = music_number(arg2)) == 0) {
                  send_to_char("Never heard of such music!\r\n", ch);
                  return;
              }

              if (!music_styles[style].instr[0]) {
                  send_to_char("This is a style you can't sing!\r\n", ch);
                  return;
              }

              if (!check_skill(ch, gsn_music, music_styles[style].diff)) {
                  send_to_char("You're not sure if you can sing that style!\r\n", ch);
                  ch->move = UMAX(1, ch->move -90);
                  WAIT_STATE (ch, 24);
                  return;
              }

              mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, style, music_styles[style].title);
              wev = get_wev(WEV_MUSIC, WEV_MUSIC_SING, mcc,
                      "You start singing @t0.\r\n",
                       NULL,
                      "@a2 starts singing @t0.\r\n");

              sprintf(buf, "singing %s.", music_styles[style].title);

              if (!room_issue_wev_challange(ch->in_room, wev)) {
                 free_wev(wev);
                 clear_activity(ch);
                 return;
              }

              set_activity( ch, ch->position, NULL, ACV_MUSIC, buf);
              free_wev(wev);
              set_activity_key(ch); 
 
              free_string(ch->acv_text);
              ch->acv_text = str_dup(music_styles[style].name);

              ch->acv_state = ACV_MUSIC_SING;
              ch->acv_int = style;

              schedule_activity(ch, 10, "Sing" );

          } else if (!str_cmp(arg1, "play")) {
              if ((instrument = get_obj_here(ch, arg2)) == NULL) {
                   act( "There is no $T here.", ch, NULL, arg2, TO_CHAR );
                  return;
              } 

              if (instrument->item_type != ITEM_INSTRUMENT) {
                  send_to_char("This is no instrument!\r\n", ch);
                  return;
              }

              if ((style = music_number(arg3)) == 0) {
                  send_to_char("Never heard of such music!\r\n", ch);
                  return;
              }

              if (!music_styles[style].instr[instrument->value[0]]) {
                  send_to_char("This is a style you can't play with this instrument!\r\n", ch);
                  return;
              }

              if (!check_skill(ch, gsn_music, music_styles[style].diff)) {
                  send_to_char("You're not sure if you can play that style!\r\n", ch);
                  ch->move = UMAX(1, ch->move -60);
                  WAIT_STATE (ch, 24);
                  return;
              }

              mcc = get_mcc(ch, ch, NULL, NULL, instrument, NULL, style, music_styles[style].title);
              wev = get_wev(WEV_MUSIC, WEV_MUSIC_PLAY, mcc,
                      "You start playing @t0 on @p2.\r\n",
                       NULL,
                      "@a2 starts playing @t0 on @p2.\r\n");

              sprintf(buf, "playing %s on %s.", music_styles[style].title, instrument->short_descr);

              if (!room_issue_wev_challange(ch->in_room, wev)) {
                 free_wev(wev);
                 clear_activity(ch);
                 return;
              }

              set_activity( ch, ch->position, NULL, ACV_MUSIC, buf);
              free_wev(wev);
              set_activity_key(ch); 
 
              free_string(ch->acv_text);
              ch->acv_text = str_dup(instrument->name);

              ch->acv_state = ACV_MUSIC_PLAY;
              ch->acv_int = style;

              schedule_activity(ch, 10, "Play" );

          } else {
                  send_to_char("What do you want to do?\r\n", ch);
                  return;
          }
          break;

      case ACV_MUSIC_SING: 
           style = ch->acv_int;
           if (style <= 0) {
               send_to_char( "You lose your text!\r\n", ch );
               set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
               return;
           }

           if (ch->move < 30) {
               send_to_char( "Your throat gets sore!\r\n", ch );
               set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
               return;
           }

           ch->move -= 30;

           sprintf(buf, "You sing %s.",music_styles[style].title);
           act( buf, ch, NULL, NULL, TO_CHAR );
           sprintf(buf, "$n sings %s.",music_styles[style].title);
           act( buf, ch, NULL, NULL, TO_ROOM );

           eff = calculate_music_effect(ch, NULL) - music_styles[style].diff;
           ch->acv_int2 = eff;

           evaluate_eff(ch, eff);

           schedule_activity(ch, 10, "Sing" );
           break;

      case ACV_MUSIC_PLAY: 
           if (ch->acv_text == NULL ) {
              send_to_char( "You can no longer find it!\r\n", ch );
              set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
              return;
           }

           if ((instrument = get_obj_here(ch, ch->acv_text )) == NULL) {
              send_to_char( "You can no longer find it!\r\n", ch );
              set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
              return;
           }

           if (ch->move < 20) {
               send_to_char( "You get tired!\r\n", ch );
               set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
               return;
           }

           ch->move -= 20;

           style = ch->acv_int;
           if (style <= 0) {
               send_to_char( "You lose the melody!\r\n", ch );
               set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
               return;
           }

           sprintf(buf, "You play %s on %s.",music_styles[style].title, instrument->short_descr);
           act( buf, ch, NULL, NULL, TO_CHAR );
           sprintf(buf, "$n plays %s on %s.",music_styles[style].title, instrument->short_descr);
           act( buf, ch, NULL, NULL, TO_ROOM );

           eff = calculate_music_effect(ch, instrument) - music_styles[style].diff;
           ch->acv_int2 = eff;

           evaluate_eff(ch, eff);

           if (style == music_number("improvise") && check_skill(ch, gsn_music, 15)) {
               if (instrument->value[4] > 0) {
                   set_activity(ch, ch->position, NULL, ACV_NONE, NULL);
                   obj_cast_spell(instrument->value[4], UMAX(eff/5, 1), ch, ch, instrument, NULL );
                   return;
               }
           }

           schedule_activity(ch, 10, "Play" );
           break;
    }
    return;
}


void evaluate_eff(CHAR_DATA *ch, int eff) {
      if (eff < -10) sprintf_to_room(ch->in_room, "{CIt sounds abysmal.{x\r\n");
      else if (eff < 10) sprintf_to_room(ch->in_room, "{CIt sounds horrible.{x\r\n");
      else if (eff < 30) sprintf_to_room(ch->in_room, "{CIt sounds uninteresting.{x\r\n");
      else if (eff < 50) sprintf_to_room(ch->in_room, "{CIt sounds ok.{x\r\n");
      else if (eff < 70) sprintf_to_room(ch->in_room, "{CIt sounds nice.{x\r\n");
      else if (eff < 90) sprintf_to_room(ch->in_room, "{CIt sounds impressive.{x\r\n");
      else sprintf_to_room(ch->in_room, "{CIt sounds magnificent.{x\r\n");

      return;
}


void load_mstyles() {
FILE *fp;
char *kwd;
char code;
bool done;
bool ok;
char *name;
int sn;
int num;

  char buf[MAX_STRING_LENGTH];

 /* Initialize incase of problems... */

  for(sn = 0; sn < MAX_MSTYLES; sn++) {
    music_styles[sn].loaded = FALSE;
  } 

  sn = 0;

  music_styles[sn].name 	= "none";
  music_styles[sn].title 	= "none";
  music_styles[sn].diff 	= 0;
  music_styles[sn].volume 	= 0;
  music_styles[sn].loaded 	= TRUE;

  for (num = 0; num < MAX_INSTRUMENTS; num++) {
    music_styles[sn].instr[num] = FALSE;
  }

  fp = fopen(MSTYLE_FILE, "r");

  if (fp == NULL) {
    log_string("No music-style file!");
    exit(1);
    return;
  }

 /* Read through it and see what we've got... */
 
  done = FALSE;

  while(!done) {
    kwd = fread_word(fp);
    code = kwd[0];
    ok = FALSE;

    switch (code) {

    /* Comments */
      case '#':
        ok = TRUE;
        fread_to_eol(fp); 
        break; 

      case 'D':

        if (!str_cmp(kwd, "Diff")) {
           ok = TRUE;

          num = fread_number(fp);

          if ( num < 0 || num > 100) {
            sprintf(buf, "Bad Diff value %d for style %s.",  num, music_styles[sn].name);
            bug(buf, 0);
            exit(1);
          } else {
            music_styles[sn].diff = num;
          }
        }
        break;

      case 'I':

        if (!str_cmp(kwd, "Inst")) {
           ok = TRUE;

          name = fread_word(fp);

          for (num = 0; num < MAX_INSTRUMENTS; num++) {
              if (num > (int) sizeof(name)) {
                  music_styles[sn].instr[num] = FALSE;
              } else {
                  if (name[num] == 'X') {
                      music_styles[sn].instr[num] = TRUE;
                  } else {
                      music_styles[sn].instr[num] = FALSE;
                  }
              }
          }
        }
        break;

      case 'S':
 
       /* STYLE name~*/

        if (!str_cmp(kwd, "STYLE")) {
          ok = TRUE;
          name = fread_word(fp); 

          if (name[0] == '\0') {
            bug("Unnammed style in music style file!", 0);
            exit(1);
          }

          sn++;

          if (sn >= MAX_MSTYLES) { 
            bug("To many styles in music style file!",0);
            exit(1);
          }

         /* Initialize data... */ 

          music_styles[sn].name 	= str_dup(name);
          name = fread_string(fp);
          music_styles[sn].title 	= str_dup(name);
          music_styles[sn].diff 	= 0;
          music_styles[sn].volume 	= 0;
          music_styles[sn].loaded 	= TRUE;
        }
         break;

      case 'V':
 
        if (!str_cmp(kwd, "Vol")) {
          ok = TRUE;
          num = fread_number(fp);

          if (num < 0 || num > 100) {
            sprintf(buf, "Bad Volume value %d for style %s.",  num, music_styles[sn].name);
            bug(buf, 0);
            exit(1);
          } else {
            music_styles[sn].volume = num;
          }
        }
        break;


     /* File ends with an E */

      case 'E':
        ok = TRUE;
        done = TRUE;
        break; 

     /* Anything else is an error... */

      default:
        break;
    }

    if (!ok) {
      sprintf(buf, "Unrecognized keyword in music styles file: %s", kwd);
      bug(buf, 0); 
      exit(1);
    }
 
  }

  fclose(fp);
  log_string("...music styles loaded");
  return;
}


int music_number(char *name) {
int music;

     for (music = 1; music < MAX_MSTYLES; music++) {
         if (!music_styles[music].name) continue;
         if (!str_cmp(music_styles[music].name, name)) return music;
     }
     return 0;
}


int instrument_skill(OBJ_DATA *obj) {

    if (!obj) return gsn_singing;

    if (obj->item_type != ITEM_INSTRUMENT) return gsn_music;

    switch (obj->value[0]) {
       default:
          return gsn_music;

       case INSTR_VOCAL:
          return gsn_singing;

       case INSTR_PERCUSSION:
          return gsn_percussion;

       case INSTR_STRINGS:
          return gsn_strings;

       case INSTR_FLUTE:
          return gsn_flute;

       case INSTR_BRASS:
          return gsn_brass;

       case INSTR_PIANO:
          return gsn_piano;

       case INSTR_ORGAN:
          return gsn_organ;

       case INSTR_CRYSTAL:
          return gsn_crystal;
    }

    return gsn_music;
}


int calculate_music_effect(CHAR_DATA *ch, OBJ_DATA *instrument) {
int sn =  instrument_skill(instrument);
int skill;

     skill = get_skill(ch, sn);

     if (instrument) skill += instrument->value[1];
     skill += (get_curr_stat(ch, STAT_CHA) - 50)/2;
     skill += (number_percent() - 100);

     return skill;
}
     

int get_room_music(ROOM_INDEX_DATA *room, int style) {
CHAR_DATA *ch;
OBJ_DATA *instrument;
int powerp = 0;
int powern = 0;
int i;
bool band[MAX_INSTRUMENTS];
bool bandok = TRUE;
int fullband = 0;

     for (i = 0; i < MAX_INSTRUMENTS; i++) band[i] = FALSE;

     for (ch = room->people; ch; ch = ch->next_in_room) {
          if (ch->activity != ACV_MUSIC) continue;
          if (ch->acv_int == 0 || ch->acv_int2 == 0) continue;

          if (ch->acv_int == style) {
                powerp += ch->acv_int2;

                if (ch->acv_text) {
                    if ((instrument = get_obj_here(ch, ch->acv_text )) != NULL) {
                         if (instrument->item_type == ITEM_INSTRUMENT) band[instrument->value[0]] = TRUE;
                    }                
                } else {
                    band[0] = TRUE;
                }

          } else {
                powern += UMIN(ch->acv_int2, music_styles[ch->acv_int].volume);
          }
     }

     for (i = 0; i < MAX_INSTRUMENTS; i++) {
           if (music_styles[style].instr[i] == TRUE) {
              fullband++;
              if (band[i] == FALSE) bandok = FALSE;
           }
     }

     if (bandok) {
        if (fullband > 1) powerp = powerp *3 /2;
        if (fullband > 3) powerp = powerp *3 /2;
        if (fullband > 5) powerp = powerp *2;
     }

     powerp -=powern;     

     return powerp;
}

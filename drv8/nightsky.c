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

/*
 * Local functions.
 */

void looksky(CHAR_DATA *ch);
void sky(CHAR_DATA *ch, int line);



void looksky(CHAR_DATA *ch) {
char buf[MAX_STRING_LENGTH];
int count;

  if (ch->in_room->sector_type == SECT_UNDERGROUND
 || ch->in_room->sector_type == SECT_INSIDE
 || IS_SET(ch->in_room->room_flags, ROOM_INDOORS)) {
     send_to_char( "You can't see the nightsky.\r\n",ch );
     return;
 }

 if (!IS_SET(time_info.flags, TIME_NIGHT)) {
     send_to_char( "You can't see the nightsky.\r\n",ch );
     return;
 }

send_to_char( "You study the nightsky.\r\n",ch );
send_to_char( " \r\n",ch );
act( "$n studies the sky.\r\n", ch, NULL, NULL, TO_ROOM);

 if (weather_info.moon == MOON_NEW) {
     sprintf(buf, "                                      {c@@@{x  \r\n");
     send_to_char(buf,ch);
     sprintf(buf, "                                     {c@@@@@{x \r\n");
     send_to_char(buf,ch);
     sprintf(buf, "                                      {c@@@{x  \r\n");
     send_to_char(buf,ch);
  }

 if (weather_info.moon == MOON_CRESCENT) {
     sprintf(buf, "                                      {c@@{y@{x   \r\n");
     send_to_char(buf,ch);
     sprintf(buf, "                                     {c@@@@{y@{x  \r\n");
     send_to_char(buf,ch);
     sprintf(buf, "                                      {c@@{y@{x   \r\n");
     send_to_char(buf,ch);
  }

 if (weather_info.moon == MOON_HALF) {
     sprintf(buf, "                                      {c@{y@@{x  \r\n");
      send_to_char(buf,ch);
    sprintf(buf, "                                     {c@@{y@@@{x \r\n");
     send_to_char(buf,ch);
     sprintf(buf, "                                      {c@{y@@{x  \r\n");
     send_to_char(buf,ch);
  }

 if (weather_info.moon == MOON_3Q) {
     sprintf(buf, "                                       {c@{y@@{x  \r\n");
     send_to_char(buf,ch);
     sprintf(buf, "                                      {c@{y@@@@{x \r\n");
     send_to_char(buf,ch);
     sprintf(buf, "                                       {c@{y@@{x  \r\n");
     send_to_char(buf,ch);
  }

 if (weather_info.moon == MOON_FULL) {
     sprintf(buf, "                                      {y@@@{x  \r\n");
     send_to_char(buf,ch);
     sprintf(buf, "                                     {y@@@@@{x \r\n");
     send_to_char(buf,ch);
     sprintf(buf, "                                      {y@@@{x  \r\n");
     send_to_char(buf,ch);
  }

send_to_char("\r\n",ch);
for (count = 1; count < 10; count++) {
     sky(ch, count);
}

return;
}


void sky(CHAR_DATA *ch, int line) {
char buf[MAX_INPUT_LENGTH];
char buf2[MAX_INPUT_LENGTH];
char buf3[MAX_INPUT_LENGTH];
char buf4[MAX_INPUT_LENGTH];
int count, countday, in, out, absday;

    if (line==1) sprintf(buf,"                                 .                              .         .           {W@{x              {W*{x                  *                  *                *       *        .                  ");
    if (line==2) sprintf(buf,"                       .   .            *                           .                                          {W*{x      *               .                                .                        ");
    if (line==3) sprintf(buf,"                     .                           .    .   .                 .                                                   .                           {g*{x                 .     *        .  ");
    if (line==4) sprintf(buf,"       .                          .                                              .                                                                     .           .                            ");
    if (line==5) sprintf(buf,"                                                              {y*{x       .                                                 *       .            {y*{x                  .         *                     ");
    if (line==6) sprintf(buf,"                                         {W*{x   *                         .                               *       {W*{x                          {r@{x                                                     ");
    if (line==7) sprintf(buf,"          *                          .     {r*{x    *                          .                *                                    {y*{x                                                 {y*{x       .    ");
    if (line==8) sprintf(buf,"                *                                                                                   *                        {y*{x                   .       .                                      ");
    if (line==9) sprintf(buf," .                    .    .                           *                       .             .       *                   {y*{x                        .          .          .                .      ");

    absday = ((time_info.month - 1) * 32 + time_info.day)/2;

    countday = 0;
    count = 0;
    while (countday < absday) {
        if (buf[count] == '{') countday--;
        else countday++;
        count++;
    }
    countday = count;
    buf2[0] = '\0';

    out =0;
    while(buf[count] != '\0') {
        buf2[out] = buf[count];
        out++;
        count++;
    }
    for(count = 0; count < countday; count++) {
        buf2[out] = buf[count];
        out++;
    }

    buf3[0] = '\0';
    in = 0; 
    countday = 0;
    while(countday < 70) {
         buf3[in] = buf2[in];
         if (buf2[in] == '{') countday--;
         else countday++;
         in++;
    }

    buf3[in] = '\0';
    sprintf(buf4,"%s{x\r\n",buf3);
    send_to_char(buf4, ch);
    return;
}

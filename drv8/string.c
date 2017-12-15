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

/***************************************************************************
 *  File: string.c                                                         *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/


#include "everything.h"
#include "text.h"


/*****************************************************************************
 Name:		string_edit
 Purpose:	Clears string and puts player into editing mode.
 Called by:	none
 ****************************************************************************/
void string_edit( CHAR_DATA *ch, char **pString ) {
    send_to_char( "-========- Entering EDIT Mode -=========-\r\n", ch );
    send_to_char( "    Type .h on a new line for help\r\n", ch );
    send_to_char( " Terminate with a @ on a blank line.\r\n", ch );
    send_to_char( "-=======================================-\r\n", ch );

    if ( *pString == NULL )  {
        *pString = str_dup( "" );
    } else {
        **pString = '\0';
    }

    ch->desc->pString = pString;
    return;
}



/*****************************************************************************
 Name:		string_append
 Purpose:	Puts player into append mode for given string.
 Called by:	(many)olc_act.c
 ****************************************************************************/
void string_append( CHAR_DATA *ch, char **pString ) {
    send_to_char( "-=======- Entering APPEND Mode -========-\r\n", ch );
    send_to_char( "    Type .h on a new line for help\r\n", ch );
    send_to_char( " Terminate with a ~ or @ on a blank line.\r\n", ch );
    send_to_char( "-=======================================-\r\n", ch );

    if ( *pString == NULL )  {
        *pString = str_dup( "" );
    }
    send_to_char( *pString, ch );
    
    if ( *(*pString + strlen( *pString ) - 1) != '\r' )  send_to_char( "\r\n", ch );

    ch->desc->pString = pString;
    return;
}


/*****************************************************************************
 Name:		string_replace
 Purpose:	Substitutes one string for another.
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
char   * string_replace (char *orig, char *old, char *newx){
        char    xbuf[MAX_STRING_LENGTH];
        int     i;

        xbuf[0] = '\0';
        strcpy (xbuf, orig);
        if (strstr (orig, old) != NULL) {
                i = strlen (orig) - strlen (strstr (orig, old));
                xbuf[i] = '\0';
                strcat (xbuf, newx);
                strcat (xbuf, &orig[i + strlen (old)]);
                free_string (orig);
        }

        return str_dup (xbuf);
}


/*****************************************************************************
 Name:          string_replacet
 Purpose:       Replaces a line of text.
 Called by:     string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
char  * string_replacet(CHAR_DATA * ch, char *orig, int line, char *newx) {
char   *rdesc;
char    xbuf[MAX_STRING_LENGTH];
int     current_line = 1;
int     i;
bool    fReplaced = FALSE;

        xbuf[0] = '\0';
        strcpy (xbuf, orig);

        i = 0;

        for (rdesc = orig; *rdesc; rdesc++)  {
                if (current_line == line && !fReplaced)  {
                        xbuf[i] = '\0';

                        if (*newx)  strcat (xbuf, newx);
                        strcat (xbuf, "\r\n");
                        fReplaced = TRUE;
                }

                if (current_line == line + 1)  {
                        strcat (xbuf, &orig[i]);
                        free_string (orig);

                        send_to_char ("Line replaced.\r\n", ch);

                        return str_dup (xbuf);
                }

                i++;

                if (*rdesc == '\r')  current_line++;
        }

        if (current_line - 1 != line)  {
                send_to_char ("That line does not exist.\r\n", ch);
                return str_dup (xbuf);
        }

        free_string (orig);
        send_to_char ("Line replaced.\r\n", ch);

        return str_dup (xbuf);
}


/*****************************************************************************
 Name:          string_insertline
 Purpose:       Inserts a line, blank or containing text.
 Called by:     string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
char   *string_insertline (CHAR_DATA * ch, char *orig, int line, char *addstring) {
        char   *rdesc;
        char    xbuf[MAX_STRING_LENGTH];
        int     current_line = 1;
        int     i;

        xbuf[0] = '\0';
        strcpy (xbuf, orig);

        i = 0;

        for (rdesc = orig; *rdesc; rdesc++)   {
                if (current_line == line)  break;

                i++;

                if (*rdesc == '\r')  current_line++;
        }

        if (!*rdesc)  {
                send_to_char ("That line does not exist.\r\n", ch);
                return str_dup (xbuf);
        }

        xbuf[i] = '\0';

        if (*addstring) strcat (xbuf, addstring);
        strcat (xbuf, "\r\n");

        strcat (xbuf, &orig[i]);
        free_string (orig);

        send_to_char ("Line inserted.\r\n", ch);

        return str_dup (xbuf);
}


/*****************************************************************************
 Name:          string_deleteline
 Purpose:       Deletes a specified line of the string.
 Called by:     string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
char   *string_deleteline (char *orig, int line) {
        char   *rdesc;
        char    xbuf[MAX_STRING_LENGTH];
        int     current_line = 1;
        int     i = 0;

        xbuf[0] = '\0';

        for (rdesc = orig; *rdesc; rdesc++)  {
                if (current_line != line) {
                        xbuf[i] = *rdesc;
                        i++;
                }

                if (*rdesc == '\r')  current_line++;
        }

        free_string (orig);
        xbuf[i] = 0;

        return str_dup (xbuf);
}


/*
 * Thanks to Kalgen for the new procedure (no more bug!)
 * Original wordwrap() written by Surreality.
 */
/*****************************************************************************
 Name:		format_string
 Purpose:	Special string formating and word-wrapping.
 Called by:	string_add(string.c) (many)olc_act.c
 ****************************************************************************/
char *format_string( char *oldstring /*, bool fSpace */) {
  char xbuf[MAX_STRING_LENGTH];
  char xbuf2[MAX_STRING_LENGTH];
  char *rdesc;
  int i=0;
  bool cap=TRUE;
  
  xbuf[0]=xbuf2[0]=0;
  
  i=0;
  
  for (rdesc = oldstring; *rdesc; rdesc++)  {
    if (*rdesc=='\n')    {
      if (xbuf[i-1] != ' ')   {
        xbuf[i]=' ';
        i++;
      }
    }
    else if (*rdesc=='\r') ;
    else if (*rdesc==' ')   {
      if (xbuf[i-1] != ' ')  {
        xbuf[i]=' ';
        i++;
      }
    }
    else if (*rdesc==')')
    {
      if (xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!'))
      {
        xbuf[i-2]=*rdesc;
        xbuf[i-1]=' ';
        xbuf[i]=' ';
        i++;
      }
      else
      {
        xbuf[i]=*rdesc;
        i++;
      }
    }
    else if (*rdesc=='.' || *rdesc=='?' || *rdesc=='!') {
      if (xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!')) {
        xbuf[i-2]=*rdesc;
        if (*(rdesc+1) != '\"')
        {
          xbuf[i-1]=' ';
          xbuf[i]=' ';
          i++;
        }
        else
        {
          xbuf[i-1]='\"';
          xbuf[i]=' ';
          xbuf[i+1]=' ';
          i+=2;
          rdesc++;
        }
      }
      else
      {
        xbuf[i]=*rdesc;
        if (*(rdesc+1) != '\"')
        {
          xbuf[i+1]=' ';
          xbuf[i+2]=' ';
          i += 3;
        }
        else
        {
          xbuf[i+1]='\"';
          xbuf[i+2]=' ';
          xbuf[i+3]=' ';
          i += 4;
          rdesc++;
        }
      }
      cap = TRUE;
    }
    else
    {
      xbuf[i]=*rdesc;
      if ( cap )
        {
          cap = FALSE;
          xbuf[i] = UPPER( xbuf[i] );
        }
      i++;
    }
  }
  xbuf[i]=0;
  strcpy(xbuf2,xbuf);
  
  rdesc=xbuf2;
  
  xbuf[0]=0;
  
  for ( ; ; )
  {
    for (i=0; i<77; i++)
    {
      if (!*(rdesc+i)) break;
    }
    if (i<77)
    {
      break;
    }
    for (i=(xbuf[0]?76:73) ; i ; i--)
    {
      if (*(rdesc+i)==' ') break;
    }
    if (i)
    {
      *(rdesc+i)=0;
      strcat(xbuf,rdesc);
      strcat(xbuf,"\r\n");
      rdesc += i+1;
      while (*rdesc == ' ') rdesc++;
    }
    else
    {
      bug ("No spaces", 0);
      *(rdesc+75)=0;
      strcat(xbuf,rdesc);
      strcat(xbuf,"-\r\n");
      rdesc += 76;
    }
  }
  while (*(rdesc+i) && (*(rdesc+i)==' '||
                        *(rdesc+i)=='\n'||
                        *(rdesc+i)=='\r'))
    i--;
  *(rdesc+i+1)=0;
  strcat(xbuf,rdesc);
  if (xbuf[strlen(xbuf)-2] != '\n')
    strcat(xbuf,"\r\n");

  free_string(oldstring);
  return(str_dup(xbuf));
}



/*
 * Used above in string_add.  Because this function does not
 * modify case if fCase is FALSE and because it understands
 * parenthesis, it would probably make a nice replacement
 * for one_argument.
 */
/*****************************************************************************
 Name:		first_arg
 Purpose:	Pick off one argument from a string and return the rest.
 		Understands quates, parenthesis (barring ) ('s) and
 		percentages.
 Called by:	string_add(string.c)
 ****************************************************************************/
char *first_arg( char *argument, char *arg_first, bool fCase ){
    char cEnd;

    while ( *argument == ' ' )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"'
    || *argument == '%'  || *argument == '(' ) {
        if ( *argument == '(' ) {
            cEnd = ')';
            argument++;
        } else cEnd = *argument++;
    }

    while ( *argument != '\0' ) {
	if ( *argument == cEnd ) {
	    argument++;
	    break;
	}

                if ( fCase ) *arg_first = LOWER(*argument);
                else *arg_first = *argument;
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( *argument == ' ' ) argument++;

    return argument;
}


/*
 * Used in olc_act.c for aedit_builders.
 */
char * string_unpad( char * argument ) {
    char buf[MAX_STRING_LENGTH];
    char *s;

    s = argument;

    while ( *s == ' ' ) s++;

    strcpy( buf, s );
    s = buf;

    if ( *s != '\0' )  {
        while ( *s != '\0' )  s++;
        s--;

        while( *s == ' ' ) s--;
        s++;
        *s = '\0';
    }

    free_string( argument );
    return str_dup( buf );
}



/*
 * Same as capitalize but changes the pointer's data.
 * Used in olc_act.c in aedit_builder.
 */
char * string_proper( char * argument ) {
    char *s;

    s = argument;

    while ( *s != '\0' )   {
        if ( *s != ' ' )  {
            *s = UPPER(*s);
            while ( *s != ' ' && *s != '\0' )  s++;

        } else  {
            s++;
        }
    }

    return argument;
}


/*****************************************************************************
 Name:          string_add
 Purpose:       Interpreter for string editing.
 Called by:     game_loop_xxxx(comm.c).
 ****************************************************************************/
void    string_add (CHAR_DATA * ch, char *argument) {
        char    buf[MAX_STRING_LENGTH];

        smash_tilde (argument);

        if (*argument == '.') {
                char    arg1[MAX_INPUT_LENGTH];
                char    arg2[MAX_INPUT_LENGTH];
                char    arg3[MAX_INPUT_LENGTH];

                argument = one_argument (argument, arg1);
                
                if (!str_cmp (arg1, "./")) {
                        interpret(ch, argument);
                        send_to_char ("Command performed.\r\n", ch);
                        return;
                }
                
                argument = first_arg (argument, arg2, FALSE);
                argument = first_arg (argument, arg3, FALSE);

                if (!str_cmp (arg1, ".c")) {
                        send_to_char ("String cleared.\r\n", ch);
                        **ch->desc->pString = '\0';
                        return;
                }

                if (!str_cmp (arg1, ".s")) {
                        char   *rdesc;
                        int     i = 1;

                        sprintf_to_char (ch, "`5%2d`` ", i);

                        for (rdesc = *ch->desc->pString; *rdesc; rdesc++)  {
                                if (*rdesc != '`') {
                                        sprintf_to_char (ch, "%c", rdesc[0]);
                                } else {
                                        if (rdesc[1] == 'Z') send_to_char ("{Z}", ch);
                                        else  sprintf_to_char (ch, "%c%c", rdesc[0], rdesc[1]);
                                        rdesc++;
                                }


                                if (*rdesc == '\r' && *(rdesc + 1)) {
                                        i++;
                                        sprintf_to_char (ch, "`5%2d`` ", i);
                                }
                        }

                        return;
                }

                if (!str_cmp (arg1, ".r"))   {
                        if (arg2[0] == '\0')   {
                                send_to_char ("usage:  .r \"old string\" \"new string\"\r\n", ch);
                                return;
                        }

                        smash_tilde (arg3);
                        *ch->desc->pString = string_replace (*ch->desc->pString, arg2, arg3);
                        sprintf_to_char(ch, "'%s' replaced with '%s'.\r\n", arg2, arg3);
                        return;
                }

                if (!str_cmp (arg1, ".rl")) {
                        if (arg2[0] == '\0' || !is_number (arg2)) {
                                send_to_char ("usage:  .rl <line> <text>\r\n", ch);
                                return;
                        }

                        smash_tilde (arg3); 
                        *ch->desc->pString =  string_replacet (ch, *ch->desc->pString, atoi (arg2), arg3);
                        return;
                }


                if (!str_cmp (arg1, ".i")) {
                        if (arg2[0] == '\0' || !is_number (arg2))
                        {
                                send_to_char (
                                   "usage:  .i <line> {text}\r\n", ch);
                                return;
                        }

                        smash_tilde (arg3);     
                        *ch->desc->pString = string_insertline (ch, *ch->desc->pString, atoi (arg2), arg3);
                        return;
                }


                if (!str_cmp (arg1, ".d")) {
                        if (arg2[0] == '\0' || !is_number (arg2))  {
                                send_to_char (
                                   "usage:  .d <line>\r\n", ch);
                                return;
                        }

                        *ch->desc->pString = string_deleteline (*ch->desc->pString, atoi (arg2));
                        sprintf_to_char(ch, "Line %d deleted.\r\n", atoi (arg2));
                        return;
                }


	if ( !str_cmp( arg1, ".dl")) {
                     int tot_len, i, count=0;
                     char newbuf[MAX_STRING_LENGTH + 2];
	     char buf[MAX_STRING_LENGTH];

	     send_to_char ("Deleting last line.\r\n",ch);

                     strcpy( buf, *ch->desc->pString );
                     tot_len=strlen(buf);

                     if (tot_len < 3) {
                           send_to_char ("No lines left to delete.\r\n", ch);
                           return;
                     }

                     for (i=(tot_len);((i>=0)&&(count<2));i--) {
                            if (buf[i]=='\n') count++;
                     }

                     if (!(i<=0)) {
                            strncpy (newbuf, buf, (i+2));
                            newbuf[i+2]='\0';
                     } else {
                            newbuf[0]='\0';
                     } 

                     free_string( *ch->desc->pString );
                     *ch->desc->pString = str_dup( newbuf );
                     send_to_char( "Ok.\r\n", ch );
                     return;
                }


                if (!str_cmp (arg1, ".f")) {
                        *ch->desc->pString = format_string (*ch->desc->pString);
                        send_to_char ("String formatted.\r\n", ch);
                        return;
                }

                if (!str_cmp (arg1, ".?")) {
                        send_to_char( "-========- Entering EDIT Mode -=========-\r\n", ch );
                        send_to_char( "    Type .h on a new line for help\r\n", ch );
                        send_to_char( " Terminate with a ~ or @ on a blank line.\r\n", ch );
                        send_to_char( "-=======================================-\r\n", ch );
                        send_to_char(*ch->desc->pString, ch );
                        if ( *(*ch->desc->pString + strlen( *ch->desc->pString ) - 1) != '\r' )  send_to_char( "\r\n", ch );
                        return;
                }

                if (!str_cmp (arg1, ".h")) {
                        send_to_char ("Sedit help (commands on blank line):   \r\n", ch);
                        send_to_char (".r 'old' 'new'   - replace a substring \r\n", ch);
                        send_to_char (".rl <line> <text> - replaces a line    \r\n", ch);
                        send_to_char (".h               - get help (this info)\r\n", ch);
                        send_to_char (".s               - show string so far  \r\n", ch);
                        send_to_char (".f               - (word wrap) string  \r\n", ch);
                        send_to_char (".c               - clear string so far \r\n", ch);
                        send_to_char (".d <line>        - deletes a line      \r\n", ch);
                        send_to_char (".dl              - deletes last line   \r\n", ch);
                        send_to_char (".i <line> {text} - inserts a line          \r\n", ch);
                        send_to_char ("./ <command>     - do a regular command\r\n", ch);
                        send_to_char ("?                - show string          \r\n", ch);
                        send_to_char ("@                - end string          \r\n", ch);
                        return;
                }


                send_to_char ("SEdit:  Invalid dot command.\r\n", ch);
                return;
        }

        if ( *argument == '~' || *argument == '@' ) {
             ch->desc->pString = NULL;
             return;
        }

        strcpy( buf, *ch->desc->pString );

        /*
         * Truncate strings to MAX_STRING_LENGTH.
         * --------------------------------------
         */
        if (strlen (buf) + strlen (argument) >= (MAX_STRING_LENGTH - 4)) {
                send_to_char ("String too long, last line skipped.\r\n", ch);

                /* Force character out of editing mode. */
                ch->desc->pString = NULL;
                return;
        }

        /*
         * Ensure no tilde's inside string.
         * --------------------------------
         */
        smash_tilde (argument);

        strcat (buf, argument);
        strcat (buf, "\r\n");
        free_string (*ch->desc->pString);
        *ch->desc->pString = str_dup (buf);
        return;
}


/*****************************************************************************
 Name:          wrap_string
 Purpose:       String word-wrapping for those whose terms don't have it.
 Called by:     (many)act_comm.c (act_new)comm.c
 ****************************************************************************/

char   *wrap_string ( char *oldstring, int length ) {
        char    xbuf[MAX_STRING_LENGTH];
        static char    xbuf2[MAX_STRING_LENGTH];
        char   *rdesc;
        int     i = 0;
        int     end_of_line;
        
        if (!length) {
                strcpy (xbuf2, oldstring);
                return xbuf2;
        }
        
        xbuf[0] = xbuf2[0] = '\0';

        i = 0;

        rdesc = oldstring;

        for (;;)  {
                end_of_line = length;
                for (i = 0; i < end_of_line; i++)    {
                        if (*(rdesc + i) == '`')   {
                                end_of_line += 2;
                                i++;
                        }

                        if (!*(rdesc + i))  break;

                        if (*(rdesc + i) == '\r')  end_of_line = i;
                }
                if (i < end_of_line) break;

                if (*(rdesc + i - 1) != '\r')  {
                        for (i = (xbuf[0] ? (end_of_line - 1) : (end_of_line - 4)); i; i--)   {
                                if (*(rdesc + i) == ' ')  break;
                        }
                        if (i) {
                                *(rdesc + i) = 0;
                                strcat (xbuf, rdesc);
                                strcat (xbuf, "\r\n");
                                rdesc += i + 1;
                                while (*rdesc == ' ')  rdesc++;

                        } else {
                                bug ("No spaces", 0);
                                *(rdesc + (end_of_line - 2)) = 0;
                                strcat (xbuf, rdesc);
                                strcat (xbuf, "-\r\n");
                                rdesc += end_of_line - 1;
                        }
                } else {
                        *(rdesc + i - 1) = 0;
                        strcat (xbuf, rdesc);
                        strcat (xbuf, "\r");
                        rdesc += i;
                        while (*rdesc == ' ') rdesc++;
                }
        }
        while (*(rdesc + i) && (*(rdesc + i) == ' ' || *(rdesc + i) == '\n' || *(rdesc + i) == '\r'))  i--;
        *(rdesc + i + 1) = 0;
        strcat (xbuf, rdesc);
        if (xbuf[strlen (xbuf) - 2] != '\n') strcat (xbuf, "\r\n");
        strcpy(xbuf2, xbuf);
        return (xbuf2);
}



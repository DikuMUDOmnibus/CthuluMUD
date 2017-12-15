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
#include "text.h"

// ---------------------------------------------------------------------------
//
// Utilities for handling test and formatting...
//
// ---------------------------------------------------------------------------

//
// Nicely format a text string 
//

bool format_text(const char *in, char *out) {

  int ipos, opos, spos, tpos, lln, ls, tgpos;

  bool capital, sentence, formatting, in_tag;

  char tag[MAX_STRING_LENGTH];

 /* We must have both buffers... */

  if ( in == NULL
    || out == NULL ) {
    return FALSE;
  }

 /* Initialize... */

  opos = 0;
  ipos = 0;
  spos = 0;
  tpos = 0;
  lln = 2;
  ls = 2;
  tgpos = 0;

  capital = FALSE;
  sentence = FALSE;
  formatting = TRUE;
  in_tag = FALSE;

 /* Copy and format... */

 /* Chew through any leading spaces... */

  while ( in[ipos] != '\0' 
       && isspace(in[ipos]) ) {
       ipos += 1;
  }

 /* Ok, start of the body of the text... */

  while ( in[ipos] != '\0'
       && ipos < MAX_STRING_LENGTH 
       && opos < (MAX_STRING_LENGTH - 3) ) { 

   /* If we're in a tag, we just copy it over... */

    if ( in_tag ) {

      switch ( in[ipos] ) {

       /* End of tag? */

        case '>':

          in_tag = FALSE; 

          tag[tgpos++] = '\0';

          out[opos++] = in[ipos++]; 

         /* Does the tag affect formatting? */

          if ( !str_cmp(tag, "pre")) {
            formatting = FALSE;
          } else if ( !str_cmp(tag, "/pre")) { 
            formatting = TRUE;
          } else if ( !str_cmp(tag, "p")
                   || !str_cmp(tag, "br")  ) {
            
            spos = 0;  
            lln = 2;
            ls = 2; 
            sentence = FALSE; 

           /* We also need to strip following spaces... */

            ipos += 1;

            while ( in[ipos] != '\0' 
                 && isspace(in[ipos]) ) {
              ipos += 1;
            }

            ipos -= 1;
          }

          break;

       /* Newlines we ignore... */

        case '\r':

          ipos += 1;

          break;

       /* Newlines we replace with a space... */

        case '\n':

          tag[tgpos++] = ' ';
          out[opos++] = ' ';

          ipos += 1;

          break;

       /* Just copy and continue... */       

        default:

          tag[tgpos++] = in[ipos];

          out[opos++] = in[ipos++];

          break;
      }   

   /* If we're formatting, we have to fix things... */

    } else if (formatting) {

      switch (in[ipos]) {

       /* Newlines we ignore... */

        case '\r':

          ipos += 1;

          break;

       /* Spaces and linefeeds we copy, remember and count... */
 
        case '\n':
        case ' ':

         /* In a line, we write a space and remember it... */

          if ( lln < LINE_LENGTH ) { 
            out[opos] = ' ';
            spos = opos;
            lln += 1;
            ls = 0; 
        
         /* If at end of line, write lf, zero line length... */

          } else  {
            out[opos] = '\n'; 
            spos = 0;
            lln = 0; 
            ls = 0; 
          }

         /* Increment counters... */

          opos += 1;
          ipos += 1;

         /* Chew through any stray spaces in the input... */

          while ( in[ipos] != '\0' 
               && isspace(in[ipos]) ) {
            ipos += 1;
          }

          break;

       /* HTML we swith to tag mode... */

        case '<':
 
          in_tag = TRUE;

          tgpos = 0;

          out[opos++] = in[ipos++];
 
          break;

       /* Control code, we just copy... */

        case '{':

         /* Copy the curly bracket... */

          out[opos] = in[ipos];

         /* Increment pointers... */

          opos += 1;
          ipos += 1;

         /* If control code present... */

          if (in[ipos] != '\0') {

           /* Copy the control code... */

            out[opos] = in[ipos];

           /* So what was before the control code? */

            tpos = opos - 2;

            if (tpos >= 0) {
              if ( out[tpos] == '.'
                || out[tpos] == '!'
                || out[tpos] == '?' ) {
                capital = TRUE;
                sentence = TRUE;
              }
            }

           /* If its a forced linefeed, reset counts... */

            if (in[ipos] == '/') {
              spos = 0;  
              lln = 2;
              ls = 2; 
              sentence = FALSE; 

             /* We also need to strip following spaces... */

              ipos += 1;

              while ( in[ipos] != '\0' 
                   && isspace(in[ipos]) ) {
                ipos += 1;
              }

              ipos -= 1;
            }
           
           /* If its a F, we can turn formatting off... */

            if (in[ipos] == 'F') { 
              formatting = FALSE;
            }

           /* Increment pointers... */

            opos += 1;
            ipos += 1;

           /* If we've just done .{x we need to add a space... */

            if (sentence) {

              if (lln < LINE_LENGTH) {
                out[opos] = ' ';
                spos = opos;
                opos += 1;         
                lln += 1;
               ls = 0;
              }

              sentence = FALSE; 
            }   
          
          }
 
          break;

       /* Periods should be followed by a double space... */ 

        case '.':
        case '?':
        case '!':
 
         /* Copy and count the period... */

          out[opos] = in[ipos];

          lln += 1;
          ls += 1;

         /* Increment pointers... */

          opos += 1;
          ipos += 1;

         /* If followed by a single space... */

          if ( ( in[ipos] == ' ' 
              || in[ipos] == '\n' )
            && in[ipos] != '\0' ) {
       
           /* Insert a second one and bump up the output counters... */

            if (lln < LINE_LENGTH) {
              out[opos] = ' ';
              spos = opos;
              opos += 1;         
              lln += 1;
              ls = 0;
            }

           /* Next normal char should be a capital... */
   
            capital = TRUE;

          }

          break;

       /* Normal characters we copy and count... */

        default:

         /* Copy the character... */

          out[opos] = in[ipos];

         /* Capitalize as needed... */

          if ( capital 
            && isalnum(out[opos]) ) {
            out[opos] = toupper(out[opos]);
            capital = FALSE;
          }

         /* Increment line and last space counts... */

          lln += 1;
          ls += 1;

         /* If we're off the end of a line, backfill with a linefeed... */ 

          if ( lln >= LINE_LENGTH
            && spos != 0) {
            out[spos] = '\n';
            lln = ls;
            spos = 0;
          }

         /* Increment counts... */
  
          opos += 1;
          ipos += 1;

          break;
      }

   /* If we're not formatting, then just sit back and watch... */

    } else {

      switch (in[ipos]) {

       /* Newlines we ignore... */

        case '\r':

          ipos += 1;

          break;

       /* Linefeeds we copy, remember and count... */
 
        case '\n':

         /* In a line, we write a space and remember it... */

          out[opos] = '\n';
          spos = 0;
          lln = 0;
          ls = 0; 
        
         /* Increment counters... */

          opos += 1;
          ipos += 1;

          break;

       /* Spaces we copy, remember and count... */
 
        case ' ':

         /* In a line, we write a space and remember it... */

          out[opos] = ' ';
          spos = opos;
          lln += 1;
          ls = 0; 
        
         /* Increment counters... */

          opos += 1;
          ipos += 1;

          break;

       /* HTML we swith to tag mode... */

        case '<':
 
          in_tag = TRUE;

          tgpos = 0;

          out[opos++] = in[ipos++];
 
          break;

       /* Control code, we just copy... */

        case '{':

         /* Copy the curly bracket... */

          out[opos] = in[ipos];

         /* Increment pointers... */

          opos += 1;
          ipos += 1;

         /* If control code present... */

          if (in[ipos] != '\0') {

           /* Copy the control code... */

            out[opos] = in[ipos];

           /* If its a forced linefeed, reset counts... */

            if (in[ipos] == '/') {
              spos = 0;  
              lln = 2;
              ls = 2; 
            }

           /* If its an, we can turn formatting back on... */

            if (in[ipos] == 'F') {
              formatting = TRUE;
            }

           /* Increment pointers... */

            opos += 1;
            ipos += 1;
          
          }
 
          break;

       /* Periods should be followed by a double space... */ 

        case '.':
        case '?':
        case '!':
 
         /* Copy and count the period... */

          out[opos] = in[ipos];

          lln += 1;
          ls += 1;

         /* Increment pointers... */

          opos += 1;
          ipos += 1;

          break;

       /* Normal characters we copy and count... */

        default:

         /* Copy the character... */

          out[opos] = in[ipos];

         /* Increment line and last space counts... */

          lln += 1;
          ls += 1;

         /* Increment counts... */
  
          opos += 1;
          ipos += 1;

          break;
      }
    }
  }

 /* Check for bail out... */

  if ( ipos >= MAX_STRING_LENGTH 
    || opos >= (MAX_STRING_LENGTH - 3) ) { 
    return FALSE;
  }

 /* Finalize... */

 /* Strip trailing blanks... */

  opos -= 1;
 
  while ( opos > 0
       && isspace(out[opos]) ) {
    opos -= 1;
  }

  opos += 1;

 /* Add trailing \r\n... */ 

  out[opos++] = '\n';
  out[opos++] = '\r';
  out[opos] = '\0';

 /* All done... */

  return TRUE;
}

//
//  Work out the value of a tags parameter...
//
//  IMG SRC="http://www.cthulhumud.cx/a.jpg" ALT="Image of Sommit"
//
//  Value for 'SRC' is: http://www.cthulhumud.cx/a.jpg
//  Value for 'ALT' is: Image of Sommit
//

bool html_tag_value(const char *tag, const char *in, char *out) {

  char test_tag[MAX_STRING_LENGTH];

  int ipos, opos, tpos;

  bool in_quote, in_value, found_it, done;

 /* Check we have all we need... */

  if ( tag == NULL
    || in == NULL
    || out == NULL ) {
    return FALSE;
  } 

 /* Run through, looking for the tag... */
 
  ipos = 0;
  opos = 0;
  tpos = 0;

  in_quote = FALSE;
  in_value = FALSE; 
  found_it = FALSE;
  done	   = FALSE;

  out[0] = '\0';

  while ( in[ipos] != '\0'
       && !done ) {

    if (!isspace(in[ipos])) {

      switch (in[ipos]) {

       /* Normal characters... */ 

        default:

          if (!in_value) {
  
            test_tag[tpos++] = toupper(in[ipos]);

          } else {
  
            if ( found_it ) {
              out[opos++] = in[ipos];
            } 
          }
         
          break;

       /* Start or end of quotes... */

        case '"':
          
          if ( !in_quote ) {
            in_quote = TRUE;
          } else {
            in_quote = FALSE; 
            if (found_it) {
              done = TRUE;
              in_value = FALSE;
              out[opos++] = '\0'; 
            }
          }

          break;

       /* An = is a switch between tag and value... */

        case '=':

          if (in_quote) {
            if (found_it) { 
              out[opos++] = in[ipos];
            }
          } else {
            in_value = TRUE;
            if (tpos > 0) {
              test_tag[tpos++] = '\0';
              if (!strcmp(test_tag, tag)) {
                found_it = TRUE;
              }
            }
          }

          break;
      }

    } else {
      if (!in_value) {
        tpos = 0;
      } else {
        if ( found_it ) {
          if ( in_quote ) {
            out[opos++] = in[ipos];
          } else {
            done = TRUE;
            in_value = FALSE;
            out[opos++] = '\0'; 
          }
        } else {
          if ( !in_quote ) {
            in_value = FALSE;
            tpos = 0;
          }
        }
      }  
    }

    ipos += 1;
  }

  if ( found_it 
    && !done ) {
    done = TRUE;
    out[opos++] = '\0';
  }
   
  return done;
}

//
// Remember 'font' attributes for conversion
//

char	font_normal;
char	font_bold;
char	font_italic;
char	font_bold_italic;

int font_state;

char	*old_font;

#define FONT_BOLD	1
#define FONT_ITALIC	2

//
// Convert an HTML tag into a text_tag 
//

bool html_tag_to_text(const char *in, char *out) {

  char w1[MAX_STRING_LENGTH];
  char v1[MAX_STRING_LENGTH];
  char v2[MAX_STRING_LENGTH];

  int ipos, wpos, opos, vpos;

  char *font;

  bool copy;

 /* We must have both buffers... */

  if ( in == NULL
    || out == NULL ) {
    return FALSE;
  }

 /* Extract the first word of the tag... */
  
  ipos = 0;
  wpos = 0;

  while ( wpos < MAX_STRING_LENGTH 
       && ipos < MAX_STRING_LENGTH
       && in[ipos] != '\0'
       && !isspace(in[ipos]) ) {
    w1[wpos++] = toupper(in[ipos++]);
  }

 /* Detect failure... */

  if ( wpos >= MAX_STRING_LENGTH 
    || ipos >= MAX_STRING_LENGTH ) {
    return FALSE;
  }

  w1[wpos] = '\0';  
 
 /* Translate the tag... */

  out[0] = '\0';

  copy = TRUE;

  switch (w1[0]) {

   /* default is no translation... */

    default:
      break;

    case 'A':

     /* <A ...> is for ANCHOR */

      if ( w1[1] == '\0' ) { 

        copy = FALSE;

        if ( html_tag_value("HREF", in, v1) ) {
          sprintf(out, "\r\n[URL: %s ]\r\n", v1);
        }

        break; 
      }  

      break;
 
    case 'B':

     /* <B> is for BOLD */

      if ( w1[1] == '\0' ) { 

        copy = FALSE;

        out[0] = '{';

        if ( IS_SET(font_state, FONT_ITALIC) ) {
          out[1] = font_bold_italic;
        } else {
          out[1] = font_bold;
        }
 
        out[2] = '\0';

        SET_BIT(font_state, FONT_BOLD);

        break; 
      }  
 
     /* <BR> is for BREAK */

      if ( w1[1] == 'R'
        && w1[2] == '\0' ) {

        copy = FALSE;

        out[0] = '{';
        out[1] = '/';
        out[2] = '\0';

        break; 
      }  
 
      break; 

    case 'F':

     /* <F> is for FONT */

      if ( w1[1] == 'O'
        && w1[2] == 'N'
        && w1[3] == 'T'
        && w1[4] == '\0' ) {

        copy = FALSE;

        if ( html_tag_value("COLOR", in, v1) ) {

          vpos = 0;

          while ( v1[vpos] != '\0' ) {
            v1[vpos] = toupper(v1[vpos]);
            vpos += 1;
          }

          font = NULL;

          switch ( v1[0] ) {

            default:
              break;

            case 'Y':
              font = FONT_YELLOW;
              break;

            case 'R':
              font = FONT_RED;
              break;

            case 'W':
              font = FONT_WHITE;
              break;

            case 'G':
              font = FONT_GREEN;
              break;  

            case 'B': 
              font = FONT_BLUE; 
              break;  

            case 'C':
              font = FONT_CYAN;
              break;

            case 'M':
              font = FONT_MAGENTA;
              break;

          }
 
         /* Activate the new font... */

          if ( font != NULL ) {

            font_normal		= font[0];
            font_bold		= font[1];
            font_italic		= font[2];
            font_bold_italic	= font[3];

            out[0] = '{';

            if ( IS_SET(font_state, FONT_BOLD) ) {
              if ( IS_SET(font_state, FONT_ITALIC) ) {
                out[1] = font_bold_italic;
              } else {
                out[1] = font_bold;
              }
            } else {
              if ( IS_SET(font_state, FONT_ITALIC) ) {
                out[1] = font_italic;
              } else {
                out[1] = font_normal;
              }
            }

            out[2] = '\0';

          } 
        }

        break; 
      }  
 
      break; 

    case 'I':

     /* <I> is for ITALIC */

      if ( w1[1] == '\0' ) {

        copy = FALSE;

        out[0] = '{';

        if ( IS_SET(font_state, FONT_BOLD) ) {
          out[1] = font_bold_italic;
        } else {
          out[1] = font_italic;
        } 

        out[2] = '\0';

        SET_BIT(font_state, FONT_ITALIC);

        break; 
      }  
 
     /* <IMG ...> is for IMAGE */

      if ( w1[1] == 'M'
        && w1[2] == 'G'
        && w1[3] == '\0' ) {

        copy = FALSE;

        if ( html_tag_value("SRC", in, v1) ) {
          if ( html_tag_value("ALT", in, v2) ) {
            sprintf(out, "\r\n[Image: %s - %s]\r\n", v1, v2);
          } else {
            sprintf(out, "\r\n[Image: %s ]\r\n", v1);
          }
        } else {
          sprintf(out, "[**Unparsable image tag**]"); 
        }

        break; 
      }  
 
      break; 

    case 'P':

     /* <P> is for PARAGRAPH */

      if ( w1[1] == '\0' ) {

        copy = FALSE;

        out[0] = '{';
        out[1] = '/';
        out[2] = '\0';

        break; 
      }  
 
     /* <PRE> is for PREFORMATTED */

      if ( w1[1] == 'R'
        && w1[2] == 'E'
        && w1[3] == '\0' ) {

        copy = FALSE;

        out[0] = '{';
        out[1] = 'F';
        out[2] = '\0';

        break; 
      }  
 
      break; 

    case 'S':

     /* <ST ...> is for STATUS */

      if ( w1[1] == 'T' 
        && w1[2] == '\0' ) { 

        copy = FALSE;

        break; 
      }  

     /* <STATUS ...> is for STATUS */

      if ( w1[1] == 'T' 
        && w1[2] == 'A' 
        && w1[3] == 'T' 
        && w1[4] == 'U' 
        && w1[5] == 'S' 
        && w1[6] == '\0' ) { 

        copy = FALSE;

        break; 
      }  

      break;
 
   /* </....> turns things off... */ 

    case '/':

      switch ( w1[1] ) {
 
        case 'A':
   
         /* </A> is to stop an ANCHOR */

          if ( w1[2] == '\0' ) {

            copy = FALSE;

            break; 
         }  
 
         break; 

        case 'B':
   
         /* </B> is to stop BOLD */

          if ( w1[2] == '\0' ) {

            copy = FALSE;

            REMOVE_BIT(font_state, FONT_BOLD);

            out[0] = '{';

            if (IS_SET(font_state, FONT_ITALIC)) {
              out[1] = font_italic;
            } else {
              out[1] = font_normal;
            }

            out[2] = '\0';

            break; 
         }  
 
         break; 

        case 'F':

         /* </FONT> is to stop a FONT */
 
          if ( w1[2] == 'O'    
            && w1[3] == 'N'    
            && w1[4] == 'T'    
            && w1[5] == '\0' ) {

            copy = FALSE;

            font = old_font;

            font_normal		= font[0];
            font_bold		= font[1];
            font_italic		= font[2];
            font_bold_italic	= font[3];

            break; 
          }  
     
        case 'I':

         /* </I> is to stop ITALIC */
 
          if ( w1[2] == '\0' ) {

            copy = FALSE;

            REMOVE_BIT(font_state, FONT_ITALIC);

            out[0] = '{';

            if (IS_SET(font_state, FONT_BOLD)) {
              out[1] = font_bold;
            } else {
              out[1] = font_normal;
            }

            out[2] = '\0';

            break; 
          }  
     
        case 'P':

         /* </P> is to stop a PARAGRAPH */

          if ( w1[2] == '\0' ) {

            copy = FALSE;

            break; 
         }  
 
         /* </PRE> is to stop PREFORMATTED */

          if ( w1[2] == 'R'
            && w1[3] == 'E'
            && w1[4] == '\0' ) {

            copy = FALSE;

            out[0] = '{';
            out[1] = 'F';
            out[2] = '\0';

            break; 
          }  
 
          break; 

        default:
          break;

      } 

      break; 

  }

 /* Copy the tag over if it is unrecognized... */

  if ( copy ) {

    opos = 0;

    out[opos++] = '<';

    ipos = 0;

    while ( in[ipos] != '\0' ) {

      out[opos++] = in[ipos++];

    }

    out[opos++] = '>';

    out[opos++] = '\0'; 

  }
 
 /* Return success... */ 

  return TRUE;
}

//
// Remove HTML from text (and optionally replace with ANSI codes) 
//

bool strip_html(const char *in, char *out, bool convert, int max, 
                char *font) {

  char tag[MAX_STRING_LENGTH];
  char text_tag[MAX_STRING_LENGTH];

  int ipos, opos, tpos;

  bool in_tag;

 /* We must have both buffers... */

  if ( in == NULL
    || out == NULL ) {
    return FALSE;
  }

 /* Initialize... */

  ipos = 0;
  opos = 0;
  tpos = 0;

  in_tag = FALSE;  

 /* Initialize font for grey/white... */
 
  font_normal		= font[0];
  font_bold		= font[1];
  font_italic		= font[2];
  font_bold_italic	= font[3];

  old_font = font;

 /* Process... */

  while ( opos < max
       && in[ipos] != '\0' ) {

   /* Now what do we do with the input... */

    switch (in[ipos]) {

     /* Default is just to copy... */
 
      default:

        if ( in_tag ) {
          tag[tpos++] = in[ipos];
        } else {
          out[opos++] = in[ipos];
        }

        break;

     /* Colour codes update the font... */
 
      case '{':

        if ( in_tag ) {
          tag[tpos++] = in[ipos];
        } else {
          out[opos++] = in[ipos];

         /* See if we have a new font... */

          switch (in[ipos+1]) {
 
            case 'r':
            case 'R':
       
              font_normal	= 'r';
              font_bold		= 'R';
              font_italic	= 'Y';

              break;

            case 'y':
            case 'Y':
       
              font_normal	= 'y';
              font_bold		= 'Y';
              font_italic	= 'R';

              break;

            case 'g':
            case 'G':
       
              font_normal	= 'g';
              font_bold		= 'G';
              font_italic	= 'Y';

              break;

            case 'b':
            case 'B':
       
              font_normal	= 'b';
              font_bold		= 'B';
              font_italic	= 'C';

              break;

            case 'c':
            case 'C':
       
              font_normal	= 'c';
              font_bold		= 'C';
              font_italic	= 'W';

              break;

            case 'm':
            case 'M':
       
              font_normal	= 'm';
              font_bold		= 'M';
              font_italic	= 'R';

              break;

            case 'w':
            case 'W':
       
              font_normal	= 'w';
              font_bold		= 'W';
              font_italic	= 'C';

              break;

            case 'x':
       
              font_normal	= 'x';
              font_bold		= 'W';
              font_italic	= 'C';

              break;

            default:
              break;

          }

        }

        break;

     /* Tag start means we change target... */  

      case '<':

        if ( !in_tag ) {
          tpos = 0; 
        } 

        in_tag = TRUE;

        break;

     /* Tag end means switch and, maybe, replace... */

      case '>':

        if ( in_tag ) {

          in_tag = FALSE;

          tag[tpos++] = '\0';

         /* Convert to text? */

          if ( convert ) {

            if ( html_tag_to_text(tag, text_tag) ) {

              tpos = 0;

              while ( opos < max
                   && text_tag[tpos] != '\0' ) {

                out[opos++] = text_tag[tpos++];

              }
            }
          }
        } else {
          out[opos++] = in[ipos];
        }

        break;
    } 

    ipos += 1;

  }

 /* Check for incomplete tag... */
 
  if ( in_tag ) {

    tag[tpos++] = '\0';

    out[opos++] = '<';

    tpos = 0;

    while ( opos < max
         && tag[tpos] != '\0' ) {

      out[opos++] = tag[tpos++];

    } 
    
  } 

 /* Check for length overrun... */

  if ( opos >= max ) {
    return FALSE;
  }

 /* Complete the output string... */

  out[opos++] = '\0';

 /* Report success... */

  return TRUE;
}
 
//
// Strip isolated line feeds...
//
// Works, but a bit to hungry...
//

bool strip_lf(const char *in, char *out, int max) {

  int ipos, opos;

 /* We must have both buffers... */

  if ( in == NULL
    || out == NULL ) {
    return FALSE;
  }

 /* Initialize... */

  ipos = 0;
  opos = 0;

 /* Process... */

  while ( opos < max
       && in[ipos] != '\0' ) {

   /* Now what do we do with the input... */

    switch (in[ipos]) {

     /* Default is just to copy... */
 
      default:

        out[opos++] = in[ipos];

        break;

     /* Some linefeeds can be removed... */  

      case '\n':

        if ( ipos > 0 
          && in[ipos+1] != '\0'
          && ( in[ipos-1] == '\r'
            || !isspace(in[ipos-1])
             )
          && ( !isspace(in[ipos+1])
            || ( in[ipos+1] == '\r'
              && in[ipos+2] != '\0'
              && !isspace(in[ipos+2]) 
               )
             ) 
           ) {  
          out[opos++] = ' ';
          if ( in[ipos+1] == '\r' ) { 
            ipos += 1;
          }
        } else {
          out[opos++] = in[ipos];
        }

        break;

    } 

    ipos += 1;

  }

 /* Check for length overrun... */

  if ( opos >= max ) {
    return FALSE;
  }

 /* Complete the output string... */

  out[opos++] = '\0';

 /* Report success... */

  return TRUE;
}
 

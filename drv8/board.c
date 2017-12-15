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
#include "colordef.h"
#include "board.h"
#include "partner.h"

DECLARE_DO_FUN( do_help );
DECLARE_DO_FUN( do_afk  );
DECLARE_DO_FUN(do_pload);
DECLARE_DO_FUN(do_punload);

/*
 
 Note Board system, (c) 1995-96 Erwin S. Andreasen, erwin@pip.dknet.dk
 =====================================================================
 
 Basically, the notes are split up into several boards. The boards do not
 exist physically, they can be read anywhere and in any position.
 
 Each of the note boards has its own file. Each of the boards can have its own
 "rights": who can read/write.
 
 Each character has an extra field added, namele the timestamp of the last note
 read by him/her on a certain board.
 
 The note entering system is changed too, making it more interactive. When
 entering a note, a character is put AFK and into a special CON_ state.
 Everything typed goes into the note.
 
 For the immortals it is possible to purge notes based on age. An Archive
 options is available which moves the notes older than X days into a special
 board. The file of this board should then be moved into some other directory
 during e.g. the startup script and perhaps renamed depending on date.
 
 Note that write_div MUST be >= read_div or else there will be strange
 output in certain functions.
 
 Board DEFAULT_BOARD must be at least readable by *everyone*.
 
*/ 

/*
Short Name,	Long Name,		read_div, 	write_div,
def_recipient,	DEF_ACTION,		expire, 	NULL, UNUSED
*/

BOARD_DATA boards[MAX_BOARD] =
{

{ "Announce", 	
"Announcements from Immortals",  	DIV_NEWBIE,	DIV_CREATOR,	"all",	DEF_NORMAL,	60,	NULL,	FALSE, 	TRUE },

{ "Global", 	
"Stuff from other Muds",	 		DIV_NEWBIE,	DIV_MUD,	"all", 	DEF_NORMAL,	21, 	NULL, 	FALSE,	TRUE },

{ "General",  	
"General discussion",         			DIV_NEWBIE,	DIV_NEWBIE,     	"all", 	DEF_INCLUDE,	21, 	NULL, 	FALSE,	TRUE },

{ "Observer",	
"The Arkham Observer (Role-Play)",    	DIV_NEWBIE, 	DIV_NEWBIE,	"all", 	DEF_NORMAL, 	21, 	NULL, 	FALSE,	TRUE },

{ "Ideas",	
"Suggestion for improvement",	 	DIV_NEWBIE,     	DIV_NEWBIE,     	"all", 	DEF_NORMAL, 	28, 	NULL, 	FALSE,	TRUE }, 

{ "Bugs",	
"Typos, bugs, errors",		 	DIV_NEWBIE,     	DIV_NEWBIE,     	"imm", 	DEF_NORMAL, 	60, 	NULL, 	FALSE, 	TRUE },

{ "Personal", 	
"Personal messages",		 	DIV_NEWBIE,     	DIV_HUMAN,     	"all", 	DEF_EXCLUDE,	28, 	NULL, 	FALSE, 	TRUE },

{ "Immortals", 	
"Messages for immortals only",		DIV_CREATOR,	DIV_CREATOR,	"all", 	DEF_EXCLUDE,	60, 	NULL, 	FALSE, 	TRUE },

{ "Society", 	
"Society Stuff",	 			DIV_HUMAN,	DIV_HUMAN,	"all", 	DEF_NORMAL,	21, 	NULL, 	FALSE,	TRUE },

{ "MailPortal", 	
"Outgoing Email Portal", 			DIV_HUMAN,	DIV_HUMAN,	"all", 	DEF_EXCLUDE,	7, 	NULL, 	FALSE,	FALSE }
};

/* The prompt that the character is given after finishing a note with ~ or END */
const char * szFinishPrompt = "({GC{x)ontinue, ({GD{x)elete Line ({GV{x)iew, ({GP{x)ost or ({GF{x)orget it?";

long last_note_stamp = 0; /* To generate unique timestamps on notes */

#define BOARD_NOACCESS -1
#define BOARD_NOTFOUND -1

static bool next_board (CHAR_DATA *ch);


/* recycle a note */
void free_note (NOTE_DATA *note) {

	if (note->sender != NULL) {
	    free_string(note->sender); 
                    note->sender = NULL;
                }

	if (note->to_list != NULL) {
	    free_string(note->to_list); 
                    note->to_list = NULL;
                } 

	if (note->subject != NULL) {
	    free_string(note->subject);
                    note->subject = NULL;
                }

	if (note->date != NULL) {
                    free_string(note->date);
                    note->date = NULL;
                }

	if (note->text != NULL) {
	    free_string(note->text);
                    note->text = NULL;
                }
		
	note->next = note_free;
	note_free = note;	
                return;
}

/* allocate memory for a new note or recycle */

NOTE_DATA *new_note () {

	NOTE_DATA *note;
	
	if (note_free != NULL) {
		note = note_free;
		note_free = note_free->next;
	} else {
		note = (NOTE_DATA *) alloc_mem (sizeof(NOTE_DATA));
        }

       /* Zero all the field - Envy does not gurantee zeroed memory */	

	note->next		= NULL;
	note->sender		= NULL;		
	note->expire		= 0;
	note->to_list		= NULL;
	note->subject		= NULL;
	note->date		= NULL;
	note->date_stamp	= 0;
	note->to_partner		= 0;
	note->text		= NULL;
	
	return note;
}

/* append this note to the given file */
static void append_note (FILE *fp, NOTE_DATA *note) {
	fprintf (fp, "Sender  %s~\n", note->sender);
	fprintf (fp, "Date    %s~\n", note->date);
	fprintf (fp, "Stamp   %ld\n", note->date_stamp);
	fprintf (fp, "Expire  %ld\n", note->expire);
	fprintf (fp, "To      %s~\n", note->to_list);
	fprintf (fp, "Subject %s~\n", note->subject);
	fprintf (fp, "Text\n%s~\n\n", note->text);
}

/* Save a note in a given board */
void finish_note (BOARD_DATA *board, NOTE_DATA *note) {
	FILE *fp;
	NOTE_DATA *p;
	char filename[200];
	
	/* The following is done in order to generate unique date_stamps */

	if (last_note_stamp >= current_time)
		note->date_stamp = ++last_note_stamp;
	else {
	    note->date_stamp = current_time;
	    last_note_stamp = current_time;
	}
	
	if (board->note_first) {
		for (p = board->note_first; p->next; p = p->next )
			; /* empty */
		
		p->next = note;
	}
	else /* nope. empty list. */
		board->note_first = note;

	/* append note to note file */		
	
	sprintf (filename, "%s%s", NOTE_DIR, board->short_name);
	
	fp = fopen (filename, "a");
	if (!fp)	{
		bug ("Could not open one of the note files in append mode",0);
		board->changed = TRUE; /* set it to TRUE hope it will be OK later? */
		return;
	}
	
	append_note (fp, note);
	fclose (fp);
}

/* Find the number of a board */
int board_number (const BOARD_DATA *board) {
	int i;
	
	for (i = 0; i < MAX_BOARD; i++)
		if (board == &boards[i]) return i;

	return -1;
}

/* Find a board number based on  a string */
int board_lookup (const char *name) {
	int i;
	
	for (i = 0; i < MAX_BOARD; i++)
		if (!str_cmp (boards[i].short_name, name))	return i;

	return -1;
}

/* Remove list from the list. Do not free note */
static void unlink_note (BOARD_DATA *board, NOTE_DATA *note) {
	NOTE_DATA *p;
	
	if (board->note_first == note) board->note_first = note->next;
	else{
		for (p = board->note_first; p && p->next != note; p = p->next);
		if (!p) bug ("unlink_note: could not find note.",0);
		else p->next = note->next;
	}
}

/* Find the nth note on a board. Return NULL if ch has no access to that note */
static NOTE_DATA* find_note (CHAR_DATA *ch, BOARD_DATA *board, int num) {
	int count = 0;
	NOTE_DATA *p;
	
	for (p = board->note_first; p ; p = p->next)
			if (++count == num) break;
	
	if ( (count == num) && is_note_to (ch, p)) return p;
	else return NULL;
	
}

/* save a single board */
static void save_board (BOARD_DATA *board) {
	FILE *fp;
	char filename[200];
	char buf[200];
	NOTE_DATA *note;
	
	sprintf (filename, "%s%s", NOTE_DIR, board->short_name);
	
	fp = fopen (filename, "w");
	if (!fp)	{
		sprintf (buf, "Error writing to: %s", filename);
		bug (buf, 0);
	} else {
		for (note = board->note_first; note ; note = note->next)
			append_note (fp, note);
			
		fclose (fp);
	}
}

/* Show one not to a character */
static void show_note_to_char (CHAR_DATA *ch, NOTE_DATA *note, int num) {
	char buf[4*MAX_STRING_LENGTH];

	sprintf (buf,
	 "[{W%4d{x] {Y%s{x: {G%s{x\r\n{YDate{x:  %s\r\n{YTo{x: %s\r\n{G==========================================================================={x\r\n%s\r\n",
	         num, note->sender, note->subject,
	         note->date,
	         note->to_list,
	         note->text);

	send_to_char (buf,ch);	         
}

/* Save changed boards */
void save_notes () {
	int i;
	 
	for (i = 0; i < MAX_BOARD; i++)
		if (boards[i].changed) save_board (&boards[i]);
}

/* Load a single board */
static void load_board (BOARD_DATA *board) {
	FILE *fp, *fp_archive;
	NOTE_DATA *last_note;
	char filename[200];
	
	sprintf (filename, "%s%s", NOTE_DIR, board->short_name);
	
	fp = fopen (filename, "r");
	
	/* Silently return */
	if (!fp)
		return;		
		
	/* Start note fetching. copy of db.c:load_notes() */

    last_note = NULL;

    for ( ; ; )
    {
        NOTE_DATA *pnote;
        char letter;

        do
        {
            letter = getc( fp );
            if ( feof(fp) )
            {
                fclose( fp );
                return;
            }
        }
        while ( isspace(letter) );
        ungetc( letter, fp );

        pnote             = (NOTE_DATA *) alloc_perm( sizeof(*pnote) );

        if ( str_cmp( fread_word( fp ), "sender" ) )
            break;
        pnote->sender     = fread_string( fp );

        if ( str_cmp( fread_word( fp ), "date" ) )
            break;
        pnote->date       = fread_string( fp );

        if ( str_cmp( fread_word( fp ), "stamp" ) )
            break;
        pnote->date_stamp = fread_number( fp );

        if ( str_cmp( fread_word( fp ), "expire" ) )
            break;
        pnote->expire = fread_number( fp );

        if ( str_cmp( fread_word( fp ), "to" ) )
            break;
        pnote->to_list    = fread_string( fp );

        if ( str_cmp( fread_word( fp ), "subject" ) )
            break;
        pnote->subject    = fread_string( fp );

        if ( str_cmp( fread_word( fp ), "text" ) )
            break;
        pnote->text       = fread_string( fp );
        
        pnote->next = NULL; /* jic */
        
        /* Should this note be archived right now ? */
        
        if (pnote->expire < current_time)   {
			char archive_name[200];

			sprintf (archive_name, "%s%s.old", NOTE_DIR, board->short_name);
			fp_archive = fopen (archive_name, "a");
			if (!fp_archive) {
				bug ("Could not open archive boards for writing",0);
			} else {
				append_note (fp_archive, pnote);
				fclose (fp_archive); /* it might be more efficient to close this later */
			}

			free_note (pnote);
			board->changed = TRUE;
			continue;
			
        }
        

        if ( board->note_first == NULL )
            board->note_first = pnote;
        else
            last_note->next     = pnote;

        last_note         = pnote;
    }

    bug( "Load_notes: bad key word.", 0 );
    return; /* just return */
}

/* Initialize structures. Load all boards. */
void load_boards ()
{
	int i;
	
	for (i = 0; i < MAX_BOARD; i++)
		load_board (&boards[i]);
}

/* Returns TRUE if the specified note is address to ch */
bool is_note_to (CHAR_DATA *ch, NOTE_DATA *note) {
	if (!str_cmp (ch->name, note->sender))
		return TRUE;
	
	if (is_full_name ("all", note->to_list))
		return TRUE;

	if (IS_NEWBIE(ch) 
                && (is_full_name ("newbie", note->to_list) 
                     || is_full_name ("newbies", note->to_list) 
                     || is_full_name ("beginner", note->to_list) 
                     || is_full_name ("beginners", note->to_list)))
		return TRUE;
		
	if (IS_HERO(ch) 
                && (is_full_name ("hero", note->to_list) 
                     || is_full_name ("heroes", note->to_list)))
		return TRUE;

	if (IS_HERO(ch) && ch->level > 100
                && (is_full_name ("superhero", note->to_list) 
                     || is_full_name ("superheroes", note->to_list)))
		return TRUE;

	if (IS_IMMORTAL(ch)
                && (is_full_name ("imm", note->to_list) 
                     || is_full_name ("imms", note->to_list) 
                     || is_full_name ("immortal", note->to_list) 
                     || is_full_name ("immortals", note->to_list)))
		return TRUE;

               if (IS_IMMORTAL(ch) && get_divinity(ch) >= DIV_LESSER
               && (is_full_name ("god", note->to_list) 
                     || is_full_name ("gods", note->to_list)))
		return TRUE;

	if (IS_IMP(ch) 
                && (is_full_name ("imp", note->to_list) 
                     || is_full_name ("imps", note->to_list) 
                     || is_full_name ("implementor", note->to_list) 
                     || is_full_name ("implementors", note->to_list)))
		return TRUE;
		
	if (is_full_name (ch->name, note->to_list))
		return TRUE;

               /*Numbers now direct to societies*/

	if (is_number(note->to_list)
                && SOC_BOARD(ch, atoi(note->to_list))) return TRUE;

	return FALSE;
}

/* return max # of notes on board */
int max_note_num (CHAR_DATA *ch, BOARD_DATA *board) {
	NOTE_DATA *note;
	int count = 0;

        if (board->read_div > get_divinity(ch))
                return BOARD_NOACCESS;

        for (note = board->note_first; note; note = note->next)
                if (is_note_to(ch, note)) 
		count++;

        return count;
}

/* Return the number of unread notes 'ch' has in 'board' */
/* Returns BOARD_NOACCESS if ch has no access to board */
int unread_notes (CHAR_DATA *ch, BOARD_DATA *board) {
	NOTE_DATA *note;
	time_t last_read;
	int count = 0;
	
	if (board->read_div > get_divinity(ch)) return BOARD_NOACCESS;
		
	last_read = ch->pcdata->last_note[board_number(board)];
	
	for (note = board->note_first; note; note = note->next) {
		if (is_note_to(ch, note) && ((long)last_read < (long)note->date_stamp)) count++;
                }			
	return count;
}

/*
 * COMMANDS
 */

/* Start writing a note */
static void do_nwrite (CHAR_DATA *ch, char *argument) {
char arg1[MAX_INPUT_LENGTH];
char *strtime;
char buf[200];
	
	if (IS_NPC(ch)) return;

                if (!ch->pcdata->board->true_board && !ch->pcdata->email) {
		send_to_char ("You must set your own email first.\r\n",ch);
		return;
	}

	if (get_divinity(ch) < ch->pcdata->board->write_div
                && ch->pcdata->board->write_div != DIV_MUD) {
		send_to_char ("You cannot post notes on this board.\r\n",ch);
		return;
	}

	/* continue previous note, if any text was written*/ 
	if ( ch->pcdata->in_progress != NULL  
                && ch->pcdata->in_progress->text == NULL)	{
		send_to_char ("Note in progress cancelled because you did not manage to write any text \r\n" "before losing link.\r\n\r\n",ch);
		free_note (ch->pcdata->in_progress);		              
		ch->pcdata->in_progress = NULL;
	}
	
	
	if (!ch->pcdata->in_progress) {
		ch->pcdata->in_progress = new_note();
		ch->pcdata->in_progress->sender = str_dup (ch->name);

		/* convert to ascii. ctime returns a string which last character is \n, so remove that */	
		strtime = ctime (&current_time);
		strtime[strlen(strtime)-1] = '\0';
	
		ch->pcdata->in_progress->date = str_dup (strtime);
	}

	do_afk(ch,"");

	act ("{G$n starts writing a note.{x", ch, NULL, NULL, TO_ROOM);
	
	/* Begin writing the note ! */
	sprintf (buf, "You are now %s a new note on the {W%s{x board.\r\n"
	              "Please turn {ROFF{x your triggers if using a client!\r\n\r\n",
	               ch->pcdata->in_progress->text ? "continuing" : "posting",
	               ch->pcdata->board->short_name);
	send_to_char (buf,ch);
	
	sprintf (buf, "{YFrom{x:    %s\r\n\r\n", ch->name);
	send_to_char (buf,ch);

	if (!ch->pcdata->in_progress->text) 	{
		switch (ch->pcdata->board->force_type) {
		case DEF_NORMAL:
			sprintf (buf, "If you press Return, default recipient {W%s{x will be chosen.\r\n",  ch->pcdata->board->names);
			break;
		case DEF_INCLUDE:
			sprintf (buf, "The recipient list MUST include {W%s{x. If not, it will be added automatically.\r\n", ch->pcdata->board->names);
			break;
	
		case DEF_EXCLUDE:
			sprintf (buf, "The recipient of this note must NOT include: {W%s{x.", ch->pcdata->board->names);
			break;
		}			
		
		send_to_char (buf,ch);
		send_to_char ("\r\n{YTo{x:      ",ch);
	
		ch->desc->connected = CON_NOTE_TO;
		/* nanny takes over from here */
		
	}
	else /* we are continuing, print out all the fields and the note so far*/
	{
		sprintf (buf, "{YTo{x:      %s\r\n"
		              "{YExpires{x: %s\r\n"
		              "{YSubject{x: %s\r\n", 
		               ch->pcdata->in_progress->to_list,
		               ctime(&ch->pcdata->in_progress->expire),
		               ch->pcdata->in_progress->subject);
		send_to_char (buf,ch);
		send_to_char ("{GYour note so far:{x\r\n",ch);
		send_to_char (ch->pcdata->in_progress->text,ch);
		
		send_to_char ("\r\n"
                              "Enter text. Type {Y~{x, {Y@{x, or {YEND{x on an empty line to end note\r\n"
                            "{W=========================================================={x\r\n",ch);
		ch->desc->connected = CON_NOTE_TEXT;		            

	}

                 if (ch->pcdata->board->write_div == DIV_MUD) {
                     int rn;

                     one_argument(argument, arg1);
                     ch->pcdata->in_progress->to_partner = -1;

                     if (arg1[0] != '\0') {
                          CHAR_DATA *mud;
                          mud = get_char_world(ch, arg1);
                          if (mud) {
                                rn = am_i_partner(mud); 
                                if (rn < MAX_PARTNER && rn >=0) {
                                    if (partner_array[rn].name) ch->pcdata->in_progress->to_partner = rn;
                               }
                          }
                     }

                     if (ch->pcdata->in_progress->to_partner == -1) {
                          for (rn=0; rn < MAX_PARTNER; rn++) {
                               if (partner_array[rn].name) sprintf_to_partner(rn, "mudnoteprepare %s\r\n", ch->name);
                          }
                      } else {
                          if (partner_array[ch->pcdata->in_progress->to_partner].name) sprintf_to_partner(ch->pcdata->in_progress->to_partner, "mudnoteprepare %s\r\n", ch->name);
                      }
                 }

	 return;
}


/* Read next note in current group. If no more notes, go to next board */
static void do_nread (CHAR_DATA *ch, char *argument) {
NOTE_DATA *p;
int count = 0, number;
time_t *last_note = &ch->pcdata->last_note[board_number(ch->pcdata->board)];
	
	if (!str_cmp(argument, "again")) { 
	
	} else if (is_number (argument)) {
		number = atoi(argument);
		
		for (p = ch->pcdata->board->note_first; p; p = p->next)
			if (++count == number)
				break;
		
		if (!p || !is_note_to(ch, p)) {
			send_to_char ("No such note.\r\n",ch);
		} else {
			show_note_to_char (ch,p,count);
			*last_note =  UMAX (*last_note, p->date_stamp);
		}
	} else {
		char buf[200];
		
		count = 1;
		for (p = ch->pcdata->board->note_first; p ; p = p->next, count++)
			if ((p->date_stamp > *last_note) && is_note_to(ch,p)) {
				show_note_to_char (ch,p,count);
				/* Advance if new note is newer than the currently newest for that char */
				*last_note =  UMAX (*last_note, p->date_stamp);
				return;
			}
		
		send_to_char ("No new notes in this board.\r\n",ch);
		
		if (next_board (ch))
			sprintf (buf, "Changed to next board, %s.\r\n", ch->pcdata->board->short_name);
		else
			sprintf (buf, "There are no more boards.\r\n");			
			
		send_to_char (buf,ch);
	}
}


void do_nforward(CHAR_DATA *ch, char *argument) {
NOTE_DATA *p;
int count = 0, number;
time_t *last_note = &ch->pcdata->last_note[board_number(ch->pcdata->board)];
	
	
	if (is_number (argument)) {
		number = atoi(argument);
		
		for (p = ch->pcdata->board->note_first; p; p = p->next)
			if (++count == number) break;
		
		if (!p || !is_note_to(ch, p)) send_to_char ("No such note.\r\n",ch);
		else {
			forward_note(ch, p);
			*last_note =  UMAX (*last_note, p->date_stamp);
		}
	} else {
		char buf[200];
		
		count = 1;
		for (p = ch->pcdata->board->note_first; p ; p = p->next, count++)
			if ((p->date_stamp > *last_note) && is_note_to(ch,p)) {
				forward_note(ch,p);
				*last_note =  UMAX (*last_note, p->date_stamp);
				return;
			}
		
		send_to_char ("No new notes in this board.\r\n",ch);
		
		if (next_board (ch)) sprintf (buf, "Changed to next board, %s.\r\n", ch->pcdata->board->short_name);
		else sprintf (buf, "There are no more boards.\r\n");			
		send_to_char (buf,ch);
	}
                return;
}


void do_nreply(CHAR_DATA *ch, char *argument) {
NOTE_DATA *p;
int count = 0, number;
time_t *last_note = &ch->pcdata->last_note[board_number(ch->pcdata->board)];
	
	
	if (get_divinity(ch) < ch->pcdata->board->write_div
                && ch->pcdata->board->write_div != DIV_MUD) {
		send_to_char ("You cannot post notes on this board.\r\n",ch);
		return;
	}

	if (is_number (argument)) {
		number = atoi(argument);
		
		for (p = ch->pcdata->board->note_first; p; p = p->next)
			if (++count == number) break;
		
		if (!p || !is_note_to(ch, p)) send_to_char ("No such note.\r\n",ch);
		else {
			reply_note(ch, p);
			*last_note =  UMAX (*last_note, p->date_stamp);
		}
	} else {
		char buf[200];
		
		count = 1;
		for (p = ch->pcdata->board->note_first; p ; p = p->next, count++)
			if ((p->date_stamp > *last_note) && is_note_to(ch,p)) {
				reply_note(ch,p);
				*last_note =  UMAX (*last_note, p->date_stamp);
				return;
			}
		
		send_to_char ("No new notes in this board.\r\n",ch);
		
		if (next_board (ch)) sprintf (buf, "Changed to next board, %s.\r\n", ch->pcdata->board->short_name);
		else sprintf (buf, "There are no more boards.\r\n");			
		send_to_char (buf,ch);
	}
                return;
}


/* Remove a note */
static void do_nremove (CHAR_DATA *ch, char *argument) {
	NOTE_DATA *p;
	
	if (!is_number(argument))	{
		send_to_char ("Remove which note?\r\n",ch);
		return;
	}

	p = find_note (ch, ch->pcdata->board, atoi(argument));
	if (!p) {
		send_to_char ("No such note.\r\n",ch);
		return;
	}
	
	if (str_cmp(ch->name, p->sender) && !IS_IMP(ch)) {
		send_to_char ("You are not authorized to remove this note.\r\n",ch);
		return;
	}
	
	unlink_note (ch->pcdata->board,p);
	free_note (p);
	send_to_char ("Note removed!\r\n",ch);
	
	save_board(ch->pcdata->board); /* save the board */
}


/* List all notes or if argument given, list N of the last notes */
/* Shows REAL note numbers! */
static void do_nlist (CHAR_DATA *ch, char *argument)
{
	int count= 0, show = 0, num = 0, has_shown = 0;
	time_t last_note;
	NOTE_DATA *p;
	char buf[MAX_STRING_LENGTH];
	
	
	if (is_number(argument))	 /* first, count the number of notes */
	{
		show = atoi(argument);
		
		for (p = ch->pcdata->board->note_first; p; p = p->next)
			if (is_note_to(ch,p))
				count++;
	}
	
	send_to_char ("{GNotes on this board:{x\r\n"
		      "{RNum) Author        Subject{x\r\n",ch);
	              
	last_note = ch->pcdata->last_note[board_number (ch->pcdata->board)];
	
	for (p = ch->pcdata->board->note_first; p; p = p->next)
	{
		num++;
		if (is_note_to(ch,p))
		{
			has_shown++; 		/* note that we want to see X VISIBLE note, not just last X */
			if (!show || ((count-show) < has_shown))
			{
				sprintf (buf, "{W%3d{x){B%c{Y%-13s %s{x\r\n",
				               num, 
				               last_note < p->date_stamp ? '*' : ' ',
				               p->sender, p->subject);
				send_to_char (buf,ch);
			}
		}
				              
	}
}

/* catch up with some notes */
static void do_ncatchup (CHAR_DATA *ch, char *argument) {
	NOTE_DATA *p;

	/* Find last note */	
	for (p = ch->pcdata->board->note_first; p && p->next; p = p->next);
	
	if (!p) {
		send_to_char ("Alas, there are no notes in that board.\r\n",ch);
	} else {
		ch->pcdata->last_note[board_number(ch->pcdata->board)] = p->date_stamp;
		send_to_char ("All mesages skipped.\r\n",ch);
	}
}


void do_note (CHAR_DATA *ch, char *argument) {
char arg[MAX_INPUT_LENGTH];

	if (IS_NPC(ch)) return;
	
	argument = one_argument (argument, arg);
	
	if ((!arg[0]) || (!str_cmp(arg, "read"))) 	do_nread (ch, argument);
	else if (!str_cmp (arg, "list")) 		do_nlist (ch, argument);
	else if (!str_cmp (arg, "forward")) 		do_nforward (ch, argument);
	else if (!str_cmp (arg, "write")) 		do_nwrite (ch, argument);
	else if (!str_cmp (arg, "reply")) 		do_nreply(ch, argument);
	else if (!str_cmp (arg, "remove")) 		do_nremove (ch, argument);
	else if (!str_cmp (arg, "purge")) 		send_to_char ("Obsolete.\r\n",ch);
	else if (!str_cmp (arg, "archive")) 		send_to_char ("Obsolete.\r\n",ch);
	else if (!str_cmp (arg, "catchup")) 		do_ncatchup (ch, argument);
	else do_help (ch, "note");
}


/* Show all accessible boards with their numbers of unread messages OR
   change board. New board name can be given as a number or as a name (e.g.
    board personal or board 4 */

void do_board (CHAR_DATA *ch, char *argument) {
	int i, count, number;
	char buf[200];
	
	if (IS_NPC(ch))
		return;
	
	if (!argument[0]) /* show boards */
	{
		int unread;
		int maxnum;
		
		count = 1;
		send_to_char ("{RNum         Name Unread Last  Description{x\r\n"
		              "{G=== ============ ====== ===== ==========={x\r\n",ch);
		for (i = 0; i < MAX_BOARD; i++) {
			unread = unread_notes (ch,&boards[i]); 
				/* how many unread notes? */
			maxnum = max_note_num (ch,&boards[i]);
			if (unread != BOARD_NOACCESS)
			{ 
				sprintf (buf, "{W%2d{x) {G%12s{x [{%c%4d{x] [{W%4d{x] {Y%s{x\r\n",
				     count, boards[i].short_name,unread ? 'r' : 'g', 
				     unread,
				     maxnum,
				     boards[i].long_name);
				send_to_char (buf,ch);
				count++;
			} /* if has access */
			
		} /* for each board */
		
		sprintf (buf, "\r\nYou current board is {W%s{x.\r\n",
			ch->pcdata->board->short_name);
		send_to_char (buf,ch);

		/* Inform of rights */		
		if (ch->pcdata->board->read_div > get_divinity(ch))
			send_to_char ("You cannot read nor write notes on this board.\r\n",ch);
		else if (ch->pcdata->board->write_div > get_divinity(ch))
			send_to_char ("You can only read notes from this board.\r\n",ch);
		else
			send_to_char ("You can both read and write on this board.\r\n",ch);

		return;			
	} /* if empty argument */
	
	/* Change board based on its number */
	if (is_number(argument))	{
		count = 0;
		number = atoi(argument);
		for (i = 0; i < MAX_BOARD; i++)
			if (unread_notes(ch,&boards[i]) != BOARD_NOACCESS)		
				if (++count == number)
					break;
		
		if (count == number) /* found the board.. change to it */
		{
			ch->pcdata->board = &boards[i];
			sprintf (buf, "Current board changed to {W%s{x. %s.\r\n",
				boards[i].short_name,
			        (get_divinity(ch) < boards[i].write_div) 
			        ? "You can only read here" 
			        : "You can both read and write here");
			send_to_char (buf,ch);
		}			
		else /* so such board */
			send_to_char ("No such board.\r\n",ch);
			
		return;
	}

	/* Non-number given, find board with that name */
	
	for (i = 0; i < MAX_BOARD; i++)
		if (!str_cmp(boards[i].short_name, argument))
			break;
			
	if (i == MAX_BOARD)
	{
		send_to_char ("No such board.\r\n",ch);
		return;
	}

	/* Does ch have access to this board? */	
	if (unread_notes(ch,&boards[i]) == BOARD_NOACCESS)
	{
		send_to_char ("No such board.\r\n",ch);
		return;
	}
	
	ch->pcdata->board = &boards[i];
	sprintf (buf, "Current board changed to {W%s{x. %s.\r\n",boards[i].short_name,
	              (get_divinity(ch) < boards[i].write_div) 
	              ? "You can only read here" 
	              : "You can both read and write here");
	send_to_char (buf,ch);
}

/* Send a note to someone on the personal board */
void personal_message (const char *sender, const char *to, const char *subject, const int expire_days, const char *text) {
	make_note ("Personal", sender, to, subject, expire_days, text);
}

void make_note (const char* board_name, const char *sender, const char *to, const char *subject, const int expire_days, const char *text) {
	int board_index = board_lookup (board_name);
	BOARD_DATA *board;
	NOTE_DATA *note;
	char *strtime;
	
	if (board_index == BOARD_NOTFOUND)	{
		bug ("make_note: board not found",0);
		return;
	}
	
	if (strlen(text) > MAX_NOTE_TEXT) {
		bug ("make_note: text too long (%d bytes)", strlen(text));
		return;
	}
	
	
	board = &boards [board_index];
	
	note = new_note(); /* allocate new note */
	
	note->sender = str_dup (sender);
	note->to_list = str_dup(to);
	note->subject = str_dup (subject);
	note->expire = current_time + expire_days * 60 * 60 * 24;
	note->text = str_dup (text);

	/* convert to ascii. ctime returns a string which last character is \n, so remove that */	
	strtime = ctime (&current_time);
	strtime[strlen(strtime)-1] = '\0';
	
	note->date = str_dup (strtime);
	
	finish_note (board, note);
	
}


/* tries to change to the next accessible board */
static bool next_board (CHAR_DATA *ch) {
	int i = board_number (ch->pcdata->board) + 1;
	
	while ((i < MAX_BOARD) && (unread_notes(ch,&boards[i]) <= 0)) i++;
		
	if (i == MAX_BOARD) {
                                return FALSE;
	} else {
		ch->pcdata->board = &boards[i];
		return TRUE;
	}
}


void handle_con_note_to (DESCRIPTOR_DATA *d, char * argument) {
char buf [MAX_INPUT_LENGTH];
CHAR_DATA *ch = d->character;

	if (!ch->pcdata->in_progress) {
		d->connected = CON_PLAYING;
		bug ("nanny: In CON_NOTE_TO, but no note in progress",0);
		return;
	}

	strcpy (buf, argument);
	smash_tilde (buf); /* change ~ to - as we save this field as a string later */

                if (ch->pcdata->board->true_board) {
  	     switch (ch->pcdata->board->force_type) {
		case DEF_NORMAL: /* default field */
			if (!buf[0]) {
				ch->pcdata->in_progress->to_list = str_dup (ch->pcdata->board->names);
				sprintf (buf, "Assumed default recipient: {W%s{x\r\n", ch->pcdata->board->names);
				send_to_char(buf,ch);
			} else {
				ch->pcdata->in_progress->to_list = str_dup (buf);
			}
			break;
		
		case DEF_INCLUDE: /* forced default */
			if (!is_full_name (ch->pcdata->board->names, buf)) {
				strcat (buf, " ");
				strcat (buf, ch->pcdata->board->names);
				ch->pcdata->in_progress->to_list = str_dup(buf);

				sprintf (buf, "\r\nYou did not specify %s as recipient, so it was automatically added.\r\n"
						"{YNew To{x :  %s\r\n",
						 ch->pcdata->board->names, ch->pcdata->in_progress->to_list);
				send_to_char (buf,ch);
			}
			else
				ch->pcdata->in_progress->to_list = str_dup (buf);
			break;
		
		case DEF_EXCLUDE: /* forced exclude */
			if (is_full_name (ch->pcdata->board->names, buf))
			{
				sprintf (buf, "You are not allowed to send notes to %s on this board. Try again.\r\n"
				         "{YTo{x:      ",
					ch->pcdata->board->names);
				send_to_char(buf,ch);
				return; /* return from nanny, not changing to the next state! */
			}
			else
				ch->pcdata->in_progress->to_list = str_dup (buf);
			break;
		
	     }		
               } else {
                     CHAR_DATA *wch;

                     one_argument(argument, buf);
    	     smash_tilde(buf);

                     if ((wch = get_char_world_player(ch, buf)) == NULL ) do_pload(ch, buf);
                     if ((wch = get_char_world_player(ch, buf)) == NULL ) {
                            send_to_char("{cCharacter not found - aborting.{x\r\n",ch); 
                            d->connected = CON_PLAYING;
                            REMOVE_BIT(ch->plr, PLR_AFK);
                            return;
                     }
                     
                     if (IS_NPC(wch)
                     || !wch->pcdata->email) {
                            send_to_char("{cNo valid email found - aborting.{x\r\n",ch); 
                            d->connected = CON_PLAYING;
                            REMOVE_BIT(ch->plr, PLR_AFK);
                            if (!wch->desc) do_punload(ch, buf);
                            return;
                     }
                     ch->pcdata->in_progress->to_list = strdup(wch->name);
                     if (!wch->desc) do_punload(ch, buf);
               }

                if (ch->pcdata->in_progress->subject) {
                     handle_con_note_finish (d, "p");
                } else {
	     send_to_char("\r\n{YSubject{x: ",ch); 
	     d->connected = CON_NOTE_SUBJECT;
                }
                return;
}

void handle_con_note_subject (DESCRIPTOR_DATA *d, char * argument) {
	char buf [MAX_INPUT_LENGTH];
	CHAR_DATA *ch = d->character;

	if (!ch->pcdata->in_progress) {
		d->connected = CON_PLAYING;
		bug ("nanny: In CON_NOTE_SUBJECT, but no note in progress",0);
		return;
	}

	strcpy (buf, argument);
	smash_tilde (buf); /* change ~ to - as we save this field as a string later */
	
	/* Do not allow empty subjects */
	
	if (!buf[0]) {
		send_to_char("Please enter a subject!\r\n",ch);
		send_to_char("{YSubject{x: ",ch);
	} else  if (strlen(buf)>60) {
            write_to_buffer (d, "No, no. This is just the Subject. You are not writing the note yet.\r\n",0);
	}
	else
	/* advance to next stage */
	{
		ch->pcdata->in_progress->subject = str_dup(buf);
		if (IS_IMMORTAL(ch)) /* immortals get to choose number of expire days */
		{
			sprintf (buf,"\r\nHow many days do you want this note to expire in?\r\n"
			             "Press Enter for default value for this board, {W%d{x days.\r\n"
           				 "{YExpire{x:  ",
		                 ch->pcdata->board->purge_days);
			/* write_to_buffer (d, buf, 0); */
			send_to_char(buf,ch);
			d->connected = CON_NOTE_EXPIRE;
		}
		else
		{
			ch->pcdata->in_progress->expire = 
				current_time + ch->pcdata->board->purge_days * 24L * 3600L;				
			sprintf (buf, "This note will expire %s\r",ctime(&ch->pcdata->in_progress->expire));
			/* write_to_buffer (d,buf,0); */
			send_to_char(buf,ch);
			send_to_char("\r\nEnter text. Type {Y~{x, {Y@{x or {YEND{x on an empty line to end note.\r\n"
				     "{W======================================================={x\r\n",ch);
			d->connected = CON_NOTE_TEXT;
		}
	}

                 if (ch->pcdata->board->write_div == DIV_MUD) {
                     int rn;

                     if (ch->pcdata->in_progress->to_partner == -1) {
                          for (rn=0; rn < MAX_PARTNER; rn++) {
                               if (partner_array[rn].name) sprintf_to_partner(rn, "mudnotesubject %s\r\n", ch->pcdata->in_progress->subject);
                          }
                      } else {
                          if (partner_array[ch->pcdata->in_progress->to_partner].name) sprintf_to_partner(ch->pcdata->in_progress->to_partner, "mudnotesubject %s\r\n", ch->pcdata->in_progress->subject);
                      }
                 }
                 return;
}


void handle_con_note_expire(DESCRIPTOR_DATA *d, char * argument) {
	CHAR_DATA *ch = d->character;
	char buf[MAX_STRING_LENGTH];
	time_t expire;
	int days;

	if (!ch->pcdata->in_progress)
	{
		d->connected = CON_PLAYING;
		bug ("nanny: In CON_NOTE_EXPIRE, but no note in progress",0);
		return;
	}
	
	/* Numeric argument. no tilde smashing */
	strcpy (buf, argument);
	if (!buf[0]) /* assume default expire */
		days = 	ch->pcdata->board->purge_days;
	else /* use this expire */
		if (!is_number(buf))
		{
			write_to_buffer (d,"Write the number of days!\r\n",0);
			send_to_char("{YExpire{x:  ",ch);
			return;
		}
		else
		{
			days = atoi (buf);
			if (days <= 0)
			{
				write_to_buffer (d, "This is a positive MUD. Use positive numbers only! :)\r\n",0);
				send_to_char("{YExpire{x:  ",ch);
				return;
			}
		}
			
	expire = current_time + (days*24L*3600L); /* 24 hours, 3600 seconds */

	ch->pcdata->in_progress->expire = expire;
	
	/* note that ctime returns XXX\n so we only need to add an \r */

	send_to_char("\r\nEnter text. Type {Y~{x, {Y@{x or {YEND{x on an empty line to end note.\r\n"
		     "{W======================================================={x\r\n",ch);

	d->connected = CON_NOTE_TEXT;
}



void handle_con_note_text (DESCRIPTOR_DATA *d, char * argument) {
	CHAR_DATA *ch = d->character;
	char buf[MAX_STRING_LENGTH];
	char letter[4*MAX_STRING_LENGTH];
	
	if (!ch->pcdata->in_progress) {
		d->connected = CON_PLAYING;
		bug ("nanny: In CON_NOTE_TEXT, but no note in progress",0);
		return;
	}

	/* First, check for EndOfNote marker */

	strcpy (buf, argument);
	if ( !str_cmp(buf, "~")
	  || !str_cmp(buf, "END") 
	  || !str_cmp(buf, "@") ) {
		write_to_buffer (d, "\r\n\r\n",0);
		send_to_char(szFinishPrompt, ch);
		write_to_buffer (d, "\r\n", 0);
		d->connected = CON_NOTE_FINISH;
		return;
	}
	
	smash_tilde (buf); /* smash it now */

	/* Check for too long lines. Do not allow lines longer than 80 chars */
	
	if (strlen (buf) > MAX_LINE_LENGTH) {
		write_to_buffer (d, "Too long line rejected. Do NOT go over 80 characters!\r\n",0);
		return;
	}
	
	/* Not end of note. Copy current text into temp buffer, 
		add new line, and copy back */

	/* How would the system react to strcpy( , NULL) ? */		
               if (ch->pcdata->board->write_div == DIV_MUD) {
                     int rn;

                     if (ch->pcdata->in_progress->to_partner == -1) {
                          for (rn=0; rn < MAX_PARTNER; rn++) {
                               if (partner_array[rn].name) sprintf_to_partner(rn, "mudnoteline %s %s\r\n", ch->name, prepare_for_transfer(buf));
                          }
                      } else {
                          if (partner_array[ch->pcdata->in_progress->to_partner].name) sprintf_to_partner(ch->pcdata->in_progress->to_partner, "mudnoteline %s %s\r\n", ch->name, prepare_for_transfer(buf));
                      }
               }

	if (ch->pcdata->in_progress->text)	{
		strcpy (letter, ch->pcdata->in_progress->text);
		free_string (ch->pcdata->in_progress->text);
		ch->pcdata->in_progress->text = NULL; /* be sure we don't free it twice */
	} else
		strcpy (letter, "");
		
	/* Check for overflow */
	
	if ((strlen(letter) + strlen (buf)) > MAX_NOTE_TEXT)
	{ /* Note too long, take appropriate steps */
		write_to_buffer (d, "Note too long!\r\n", 0);
                if (ch->pcdata->in_progress != NULL) {  
  		  free_note (ch->pcdata->in_progress);
		  ch->pcdata->in_progress = NULL;
                } 
		d->connected = CON_PLAYING;
		return;			
	}
	
	/* Add new line to the buffer */
	
	strcat (letter, buf);
	strcat (letter, "\r\n"); /* new line. \r first to make note files better readable */

	/* allocate dynamically */		
	ch->pcdata->in_progress->text = str_dup (letter);
}

void handle_con_note_finish (DESCRIPTOR_DATA *d, char * argument) {
	CHAR_DATA *ch = d->character;
	int tot_len, i, count=0;
	char newbuf[1024];
	char buf[MAX_STRING_LENGTH];

	
	if (!ch->pcdata->in_progress) {
		d->connected = CON_PLAYING;
		bug ("nanny: In CON_NOTE_FINISH, but no note in progress",0);
		return;
	}
		
	switch (tolower(argument[0])) {
		case 'c': 
			write_to_buffer (d,"Continuing note...\r\n",0);
			d->connected = CON_NOTE_TEXT;
			break;

		case 'v': 
			if (ch->pcdata->in_progress->text)	{
				send_to_char("{GText of your note so far:{x\r\n",ch);
				send_to_char(ch->pcdata->in_progress->text, ch);
			} else
				write_to_buffer (d,"You have not written a thing!\r\n\r\n",0);
			send_to_char(szFinishPrompt, ch);
			write_to_buffer (d, "\r\n",0);
			break;

		case 'd':
               		send_to_char ("Deleting last line.\r\n",ch);

                                                if (!ch->pcdata->in_progress->text) {
           		                    send_to_char ("No lines left to delete.\r\n", ch);
		                    return;
           		                }

       			strcpy( buf, ch->pcdata->in_progress->text );
       			tot_len=strlen(buf);

       			if (tot_len < 3) {
           		                    send_to_char ("No lines left to delete.\r\n", ch);
		                    return;
           		                }

		        for (i=(tot_len);((i>=0)&&(count<2));i--) {
		            if (buf[i]=='\n') {
		                count++;
                            }
           		}

		        if (!(i<=0)) {
		            strncpy (newbuf, buf, (i+2));
		            newbuf[i+2]='\0';
           		        } else {
                                            newbuf[0]='\0';
                                        }
                        if (ch->pcdata->in_progress->text != NULL) {
		          free_string( ch->pcdata->in_progress->text);
                                          ch->pcdata->in_progress->text = NULL;
                        }
		        ch->pcdata->in_progress->text = str_dup( newbuf );
		        send_to_char( "Ok.\r\n", ch );
		        break;

		case 'p': /* post note */
                                               if (ch->pcdata->board->write_div == DIV_MUD) {
                                                     int rn;

                                                     if (ch->pcdata->in_progress->to_partner == -1) {
                                                          for (rn=0; rn < MAX_PARTNER; rn++) {
                                                              if (partner_array[rn].name) sprintf_to_partner(rn, "mudnotestop\r\n");
                                                          }
                                                     } else {
                                                          if (partner_array[ch->pcdata->in_progress->to_partner].name) sprintf_to_partner(ch->pcdata->in_progress->to_partner, "mudnotestop\r\n");
                                                     }
                                               }

                                                if (ch->pcdata->board->true_board) {
 			      finish_note (ch->pcdata->board, ch->pcdata->in_progress);
			      write_to_buffer (d, "Note posted.\r\n",0);
                                                } else {
                                                      CHAR_DATA *wch;
                                                      char subj[MAX_INPUT_LENGTH];

                                                      if ((wch = get_char_world_player(ch, ch->pcdata->in_progress->to_list)) == NULL ) do_pload(ch, ch->pcdata->in_progress->to_list);
                                                      wch = get_char_world_player(ch, ch->pcdata->in_progress->to_list);
                                                      one_argument(ch->pcdata->in_progress->subject, subj);

                                                      send_rep_out(ch, subj, wch->pcdata->email, ch->pcdata->in_progress->text, TRUE);
                                                      if (!wch->desc) do_punload(ch, ch->pcdata->in_progress->to_list);
                                                }
			d->connected = CON_PLAYING;
			REMOVE_BIT(ch->plr, PLR_AFK);
			ch->pcdata->in_progress = NULL;
			act ("{G$n finishes $s note.",ch, NULL, NULL, TO_ROOM);
			break;
			
		case 'f':
			write_to_buffer (d, "Note cancelled!\r\n",0);
                                                if (ch->pcdata->in_progress != NULL) {
			  free_note (ch->pcdata->in_progress);
			  ch->pcdata->in_progress = NULL;
                                                } 
			d->connected = CON_PLAYING;
			/* remove afk status */
			do_afk(ch,"");
			break;
		
		default: /* invalid response */
			write_to_buffer (d, "Huh? Valid answers are:\r\n\r\n",0);
			send_to_char(szFinishPrompt, ch);
			write_to_buffer (d, "\r\n",0);
				
		}
	}



bool  SOC_BOARD  (CHAR_DATA *ch, int chnum) {
  MEMBERSHIP *memb;
 
    if (IS_IMMORTAL(ch)) return TRUE;
    memb = ch->memberships;
    while (memb != NULL) {
         if ( memb->society != NULL) {
              if (chnum==memb->society->id) return TRUE;
         }
      memb = memb->next;
     }
     return FALSE;

}


void send_email( const char * m_address, const  char * m_subject, const char * mfilename ) {
  char mailbuf[MAX_STRING_LENGTH];
  char delbuf[MAX_STRING_LENGTH];
  char dbbuf[MAX_STRING_LENGTH];
  int forkval;

  sprintf(mailbuf, "mail -s \"%s\" %s <%s/%s", m_subject, m_address, MAIL_DIR, mfilename );
  signal( SIGCHLD, SIG_IGN );

  if (( forkval = fork() ) > 0 ) {
    sprintf( dbbuf, "Just sent email: %s", mailbuf );
    bug( dbbuf, 0);
    return;

  } else if ( forkval < 0 ) {
    sprintf( dbbuf, "Error in fork for sent email: %s", mailbuf );
    bug( dbbuf, 0);
    return;
  }

  system( mailbuf );
  sprintf( delbuf, "rm %s/%s", MAIL_DIR, mfilename );
  system( delbuf );
  exit(0);
  return;

}


bool save_mail_file( const char * mfilename, char * mtext ) {
  FILE * mailfp;
  char mailfpfilename[MAX_STRING_LENGTH];

  fclose( fpReserve );
  sprintf( mailfpfilename, "%s/%s", MAIL_DIR, mfilename );

  if (( mailfp = fopen( mailfpfilename, "w" ) ) == NULL ) {
    fpReserve = fopen( NULL_FILE, "r" );
    return FALSE;
  }

  fprintf( mailfp, "%s\n", mtext);
  fflush( mailfp );
  fclose( mailfp ); 
  fpReserve = fopen( NULL_FILE, "r" );
  return TRUE;
}  


void send_rep_out( CHAR_DATA * ch, char * msub, char *to, char * outbuf, bool mailme) {
char buf[MAX_STRING_LENGTH];
char mailfilename[MAX_STRING_LENGTH];
char outbuf2[MAX_STRING_LENGTH];
char transbuf[5*MAX_STRING_LENGTH];

    if ( mailme ) {
      bool saved_mail = FALSE;

      if (IS_NPC(ch)) return;
      if (ch->pcdata->email){
        sprintf( mailfilename, "%s.mail", ch->name );
        sprintf(buf, "Email from: %s\n", ch->name);
        strcpy(transbuf, buf);
        strcat(transbuf, outbuf);
        saved_mail = save_mail_file(mailfilename, transbuf);

        if (saved_mail) {
          sprintf( outbuf2, "Email sent to %s.\r\n", to);
          send_to_char( outbuf2, ch );
          send_email(to, msub, mailfilename );

        } else {
          send_to_char( outbuf, ch );
          send_to_char( "\r\nUNABLE TO SEND SYSTEM MAIL.\r\nCheck your sendmail settings.\r\n", ch );
        }

      } else {
        send_to_char( outbuf, ch );
      }

    } else {
      send_to_char( outbuf, ch );
    }
    return;
}


void forward_note(CHAR_DATA *ch, NOTE_DATA *p) {
char buf[5*MAX_STRING_LENGTH];
char *strtime;

         sprintf_to_char(ch, "{YFrom{x:    %s\r\n\r\n", ch->name);
         if (ch->pcdata->in_progress) {
	free_note (ch->pcdata->in_progress);		              
	ch->pcdata->in_progress = NULL;
         }

         ch->pcdata->in_progress = new_note();
         ch->pcdata->in_progress->sender = str_dup(p->sender);

         strtime = ctime (&current_time);
         strtime[strlen(strtime)-1] = '\0';
         ch->pcdata->in_progress->date = str_dup(strtime);
         ch->pcdata->in_progress->expire = p->expire;
         ch->pcdata->in_progress->subject = str_dup (p->subject);

         sprintf(buf, "({GForwarded to you by %s{x)\r\n%s", ch->name, p->text);
         ch->pcdata->in_progress->text = strdup(buf);

         send_to_char("{YForward to board nr.:{x\r\n", ch);
         ch->desc->connected = CON_NOTE_BOARD;
         return;
}


void reply_note(CHAR_DATA *ch, NOTE_DATA *p) {
char buf[MAX_STRING_LENGTH];
char *strtime;

         if (ch->pcdata->in_progress) {
	free_note (ch->pcdata->in_progress);		              
	ch->pcdata->in_progress = NULL;
         }

         ch->pcdata->in_progress = new_note();
         ch->pcdata->in_progress->sender = str_dup(ch->name);

         strtime = ctime (&current_time);
         strtime[strlen(strtime)-1] = '\0';
         ch->pcdata->in_progress->date = str_dup(strtime);
         ch->pcdata->in_progress->expire = p->expire;

         sprintf(buf, "Re: %s", p->subject);
         ch->pcdata->in_progress->subject = str_dup(buf);

         if (is_name("all", p->to_list)) {
              sprintf(buf, "all %s", p->sender);
         } else if (is_name("imm", p->to_list)) {
              sprintf(buf, "imm %s", p->sender);
         } else if (is_name("hero", p->to_list)) {
              sprintf(buf, "hero %s", p->sender);
         } else if (is_name("god", p->to_list)) {
              sprintf(buf, "god %s", p->sender);
         } else {
              sprintf(buf, "%s", p->sender);
         }
         ch->pcdata->in_progress->to_list = str_dup(buf);

         send_to_char ("\r\n"
         "Enter text. Type {Y~{x, {Y@{x, or {YEND{x on an empty line to end note\r\n"
         "{W=========================================================={x\r\n",ch);
         ch->desc->connected = CON_NOTE_TEXT;
         return;
}


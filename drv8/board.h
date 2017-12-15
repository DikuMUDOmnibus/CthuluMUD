/*
 * CthulhuMud
 */

/* Includes for board system */
/* This is version 2 of the board system, (c) 1995-96 erwin@pip.dknet.dk */

#define NOTE_DIR  	"../msgbase/" /* set it to something you like */

#define DEF_NORMAL  0 /* No forced change, but default (any string)   */
#define DEF_INCLUDE 1 /* 'names' MUST be included (only ONE name!)    */
#define DEF_EXCLUDE 2 /* 'names' must NOT be included (one name only) */

#define DEFAULT_BOARD 0 /* default board is board #0 in the boards      */
                        /* It should be readable by everyone!           */
                        
#define MAX_LINE_LENGTH 80 /* enforce a max length of 80 on text lines, reject longer lines */
						   /* This only applies in the Body of the note */                        
						   
#define MAX_NOTE_TEXT (4*MAX_STRING_LENGTH - 1000)
						
#define BOARD_NOTFOUND -1 /* Error code from board_lookup() and board_number */

/* Data about a board */


/* External variables */

extern BOARD_DATA boards[MAX_BOARD]; /* Declare */


/* Prototypes */

void 	finish_note 	(BOARD_DATA *board, NOTE_DATA *note); /* attach a note to a board */
void 	free_note   	(NOTE_DATA *note); /* deallocate memory used by a note */
void 	load_boards 	(void); /* load all boards */
int 	board_lookup 	(const char *name); /* Find a board with that name */
bool 	is_note_to 	(CHAR_DATA *ch, NOTE_DATA *note); /* is tha note to ch? */
void 	personal_message (const char *sender, const char *to, const char *subject, const int expire_days, const char *text);
void 	save_notes 	();
void 	send_rep_out	(CHAR_DATA * ch, char * msub, char *to, char * outbuf, bool mailme);
bool 	save_mail_file	(const char * mfilename, char * mtext );
void 	send_email	(const char * m_address, const  char * m_subject, const char * mfilename );
void 	forward_note	(CHAR_DATA *ch, NOTE_DATA *p);
void 	reply_note	(CHAR_DATA *ch, NOTE_DATA *p);
int 	unread_notes 	(CHAR_DATA *ch, BOARD_DATA *board);


/* for nanny */
void handle_con_note_to 	(DESCRIPTOR_DATA *d, char * argument);
void handle_con_note_subject 	(DESCRIPTOR_DATA *d, char * argument);
void handle_con_note_expire 	(DESCRIPTOR_DATA *d, char * argument);
void handle_con_note_text 	(DESCRIPTOR_DATA *d, char * argument);
void handle_con_note_finish 	(DESCRIPTOR_DATA *d, char * argument);
bool  SOC_BOARD  ( CHAR_DATA *ch, int chnum);
NOTE_DATA *new_note (void);

/* Commands */

DECLARE_DO_FUN (do_note);
DECLARE_DO_FUN (do_board);
DECLARE_DO_FUN (do_nforward);
DECLARE_DO_FUN (do_nreply);

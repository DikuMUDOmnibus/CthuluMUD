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
#include <stdarg.h>
#include <signal.h>
/*
 * Socket and TCP/IP stuff.
 */
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/telnet.h>
#include "statdesc.h"
#include "skill.h"
#include "exp.h"
#include "prof.h"
#include "wev.h"
#include "mob.h"
#include "profile.h"
#include "version.h"
#include "olc.h"
#include "text.h"
#include "race.h"
#include "econd.h"
#include "cult.h"
#include "partner.h"
#include "gsn.h"
#include "board.h"

/* command procedures needed */
DECLARE_DO_FUN(do_help);
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_skills);
DECLARE_DO_FUN(do_outfit);
DECLARE_DO_FUN( do_worship);
DECLARE_DO_FUN(do_yell);

void default_memberships(CHAR_DATA *ch);

const	char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const	char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const	char 	go_ahead_str	[] = { IAC, GA, '\0' };

DESCRIPTOR_DATA *   	descriptor_free;		/* Free list for descriptors	*/
DESCRIPTOR_DATA *   	descriptor_list;		/* All open descriptors		*/
DESCRIPTOR_DATA *   	d_next;			/* Next descriptor in loop	*/
FILE *		    	fpReserve;		/* Reserved file handle		*/
bool		    	god;			/* All new chars are gods!	*/
bool		    	merc_down;
char		    	str_boot_time[MAX_INPUT_LENGTH];
time_t		    	current_time;		/* time of this pulse */	
char * 			research_queue;
AUCTION_DATA *	auction;


/*
 * OS-dependent local functions.
 */
void	game_loop_unix		args( ( int control ) );
int	init_socket		args( ( int port ) );
void	new_descriptor		args( ( int control ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );

bool		check_parse_name	(char *name);
bool		check_reconnect		(DESCRIPTOR_DATA *d, char *name, bool fConn);
bool		check_playing		(DESCRIPTOR_DATA *d, char *name);
int		main			(int argc, char **argv);
void		nanny			(DESCRIPTOR_DATA *d, char *argument);
bool		process_output		(DESCRIPTOR_DATA *d, bool fPrompt);
void		read_from_buffer	 	(DESCRIPTOR_DATA *d);
void		stop_idling		(CHAR_DATA *ch);
bool		check_ban		(char *site);
void		prepare_newbie		(CHAR_DATA *ch);
CUSTOMER 	*new_customer		(void);
int		get_cost			(CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy);
void 		haggle_emote		(CHAR_DATA *ch);
bool 		name_exists		(char *name);
void		init_signals		(void);
void 		sig_handler		(int sig);
void 		startchat			(char*);
void 		prompt_menu		(DESCRIPTOR_DATA *d);


int port;
int control;

int main( int argc, char **argv ) {
struct timeval now_time;
bool fCopyOver = FALSE;
FILE *fp;
char *lb;

    /*
     * Init time.
     */
    gettimeofday( &now_time, NULL );
    current_time 	= (time_t) now_time.tv_sec;
    strcpy( str_boot_time, ctime( &current_time ) );

    /*
     * Reserve one channel for our use.
     */
    if ((fpReserve = fopen( NULL_FILE, "r")) == NULL)    {
	perror( NULL_FILE );
	exit( 1 );
    }

#ifdef ELIZA_CHAT
    startchat(CHAT_FILE);
#endif

   /* Get the port number from input... */

    port = -1;

    if ( argc > 1 ) {
      if ( !is_number( argv[1] ) ) {
        fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
        exit( 1 );
      } else {

        port = atoi(argv[1] );

        if ( port <= 1024 ) {
          fprintf( stderr, "Port number must be above 1024.\n" );
          exit( 1 );
        }
      }
    }

     if (argv[2] && argv[2][0]) {
          fCopyOver = TRUE;
          control = atoi(argv[3]);
     } else {
          fCopyOver = FALSE;
     }

    log_string("...Loading Database"); 
    boot_db(fCopyOver);

   /* Check port address... */
    if (port <= 0) {
        port = mud.port;
        fCopyOver = FALSE;
    }

    log_string("Opening socket...");
    if (!fCopyOver) control = init_socket (port);

    sprintf( log_buf, "%s accepting connections on port %d.", mud.name, port );
    log_string( log_buf );

    fp = fopen(LASTCOMMAND_FILE, "r");

/* Commented out the portion below. It is a crash recover
 * and a hard code of the local IP and port. -Kyndig
 
    if (fp) {
        lb = fread_string(fp);
        fclose(fp);
        if (!fCopyOver) make_note ("Immortals", "Driver", "imp", "crash report", 3, lb);
    } else {
         int fd = connect_mud ("195.34.155.132", 9999);
         if (fd >= 0) {
             write_mud (fd, "y\r\n");
             write_mud (fd, "register_me_ping\r\n");
         }
    }
 */

   /* Run the main game loop... */ 

    log_string("Connecting partners..."); 
    connect_partners();

    if (IS_SET(mud.flags, MUD_RECOVER)) init_signals();

    game_loop_unix( control );

   /* Close the socket... */
    close (control);

   /* End of MUD... */
    log_string( "Normal termination of game." );
    exit(0);
    return 0;
}


int init_socket( int port ){
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;

#if defined(DEBUGINFO)
    log_string("DEBUG: int init_socket: begin");
#endif

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )  {
	perror( "Init_socket: socket" );
	exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof(x) ) < 0 )  {
	perror( "Init_socket: SO_REUSEADDR" );
	close(fd);
	exit( 1 );
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
	struct	linger	ld;

	ld.l_onoff  = 1;
	ld.l_linger = 1000;

	if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
	(char *) &ld, sizeof(ld) ) < 0 )
	{
	    perror( "Init_socket: SO_DONTLINGER" );
	    close(fd);
	    exit( 1 );
	}
    }
#endif

    sa		    = sa_zero;
    sa.sin_family   = AF_INET;
    sa.sin_port	    = htons( port );

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 ) {
	perror("Init socket: bind" );
	close(fd);
	exit(1);
    }

    if ( listen( fd, 3 ) < 0 )   {
	perror("Init socket: listen");
	close(fd);
	exit(1);
    }

    return fd;
}


void game_loop_unix( int control ) {
    static struct timeval null_time;
    struct timeval last_time;

    signal( SIGPIPE, SIG_IGN );
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

#if defined(DEBUGINFO)
    log_string("DEBUG: void game_loop_unix: begin");
#endif

    /* Main loop */
    while (!merc_down) {
    
	fd_set in_set;
	fd_set out_set;
	fd_set exc_set;
	DESCRIPTOR_DATA *d;
	int maxdesc;

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( control, &in_set );
	maxdesc	= control;
	for ( d = descriptor_list; d; d = d->next ) {
	    maxdesc = UMAX( maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	}

	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 ){
	    perror( "Game_loop: select: poll" );
	    exit( 1 );
	}

	/*
	 * New connection?
	 */
	if ( FD_ISSET( control, &in_set ))  new_descriptor( control );

	/*
	 * Kick out the freaky folks.
	 */
	for ( d = descriptor_list; d; d = d_next ) {
	    d_next = d->next;   
	    if ( FD_ISSET( d->descriptor, &exc_set ))   {
		FD_CLR( d->descriptor, &in_set);
		FD_CLR( d->descriptor, &out_set);
		if (d->character)  save_char_obj( d->character );
		d->outtop	= 0;
		close_socket(d, FALSE );

	    }

	}

	/*
	 * Process input.
	 */

	for ( d = descriptor_list; d; d = d_next ) {
	    d_next	= d->next;
	    d->fcommand	= FALSE;
                    d->multi_comm = FALSE;

	    if ( FD_ISSET( d->descriptor, &in_set ) ) {
	    
                           if (d->ok) {

                                  if (d->connected != CON_TRANSFER || d->character->timer > 5) {
  		       if ( !read_from_descriptor( d )) {
  		  
  		              FD_CLR( d->descriptor, &out_set );
  		              if ( d->character != NULL ) save_char_obj( d->character );
  	  	              d->outtop	= 0;
  		              close_socket(d, FALSE);
                                              d->ok = FALSE;
		              continue;
                                       } else {
		              if ( d->character != NULL ) d->character->timer = 0;
	                       } 
                                  }
                           }
	    }

                    if (d->connected == CON_TRANSFER && d->character->timer > 5) {
                         d->outtop	= 0;
                         if (d->character) extract_char(d->character, TRUE);
                         d->character = NULL;
                         close_socket(d, FALSE);
                         d->ok = FALSE;
                         continue;
                    }

	    if ( d->character && d->character->wait > 0 ) {
		--d->character->wait;
		continue;
	    }

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' ) {
		d->fcommand	= TRUE;
		stop_idling( d->character );

                                if ( d->showstr_point ) {
                                    show_string( d, d->incomm );
                                } else {
                                    if ( d->pString ) {
                                         string_add(d->character, d->incomm );
                                    } else {
                                         switch ( d->connected )   {
                                              case CON_PLAYING:
                                                  if ( !run_olc_editor( d ) )  interpret( d->character, d->incomm );
                                                  break;

                                              default:
                                                  nanny( d, d->incomm );
                                                  break;
                                         }
                                    }
                                }
     	                if (!d->multi_comm) d->incomm[0] = '\0';
	    }
	}

	/*
	 * Autonomous game motion.
	 */
	update_handler( );

	/*
	 * Output.
	 */
	for ( d = descriptor_list; d; d = d_next ) {
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 )
	    &&   FD_ISSET(d->descriptor, &out_set))   {
		if ( !process_output( d, TRUE )) {
		    if ( d->character && d->character->level > 1) save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket(d, FALSE);
		}
	    }
	}


	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
	{
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

	    gettimeofday( &now_time, NULL );
	    usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
	    while ( usecDelta < 0 )
	    {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while ( usecDelta >= 1000000 )
	    {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	    {
		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;
		if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
		{
		    perror( "Game_loop: select: stall" );
		    exit( 1 );
		}
	    }
	}

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}


void init_descriptor (DESCRIPTOR_DATA *dnew, int desc) {
static DESCRIPTOR_DATA d_zero;
 
	*dnew = d_zero;
 	dnew->descriptor = desc;
 	dnew->character = NULL;
 	dnew->connected = CON_CHOOSE_TERM;
 	dnew->connected_old = 0;
 	dnew->showstr_head = str_dup ("");
 	dnew->showstr_point = 0;
 	dnew->pEdit = NULL;
 	dnew->pString = NULL;
 	dnew->editor = 0;		
 	dnew->outsize = 2000;
 	dnew->outbuf = (char *) alloc_mem( dnew->outsize );
                dnew->ok		= TRUE;
 	
     return;  
}


void new_descriptor( int control ) {
char 	buf[MAX_STRING_LENGTH];
DESCRIPTOR_DATA *dnew;
struct 	sockaddr_in sock;
int desc;
unsigned int size;
extern int h_errno;

    size = sizeof(sock);
    getsockname( control, (struct sockaddr *) &sock, &size );
    if ( ( desc = accept( control, (struct sockaddr *) &sock, &size) ) < 0 ) {
	perror( "New_descriptor: accept" );
	return;
    }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )  {
	perror( "New_descriptor: fcntl: FNDELAY" );
	return;
    }

    /*
     * Cons a new descriptor.
     */
    if ( !descriptor_free ) {
	dnew		= (DESCRIPTOR_DATA *) alloc_perm( sizeof(*dnew) );

    } else {
	dnew		= descriptor_free;
	descriptor_free	= descriptor_free->next;
    }

     init_descriptor (dnew, desc);

    size = sizeof(sock);
    if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )   {
	perror( "New_descriptor: getpeername" );
	dnew->host = str_dup( "(unknown)" );

    } else {
	/*
	 * Would be nice to use inet_ntoa here but it takes a struct arg,
	 * which ain't very compatible between gcc and system libraries.
	 */
	int addr;

	addr = ntohl( sock.sin_addr.s_addr );
	sprintf( buf, "%d.%d.%d.%d",
	    ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	    ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
	    );
	sprintf( log_buf, "Sock.sinaddr:  %s", buf );
	log_string( log_buf );

        dnew->host = str_dup(buf);

    }
	
    if (check_ban(dnew->host)) {
	    write_to_descriptor(desc, "Your site has been banned from this Mud.\r\n", 0 );
	    close(desc);
	    free_string(dnew->host );
                    free_mem(dnew->outbuf, dnew->outsize );
                    dnew->next	= descriptor_free;
	    descriptor_free = dnew;
	    return;
    }

    /*
     * Init descriptor data.
     */
    dnew->next		= descriptor_list;
    descriptor_list		= dnew;

    /*
     * Send client challenge 
     */
	write_to_buffer( dnew, "This world is Pueblo 1.10 enhanced.\r\n" "Autodetecting IMP...v1.30 \r\n"
	  "This world supports " C_GREEN "C" C_RED "O" C_YELLOW "L"
	   C_BLUE "O" C_MAGENTA "R" CLEAR "!.\r\n"
	  "If the word COLOR is in color, enter Y, otherwise, enter N (y/n)\r\n", 0 );
    return;
}


void close_socket(DESCRIPTOR_DATA *dclose, bool save_first ) {
CHAR_DATA *ch;
DESCRIPTOR_DATA *d;


#if defined(DEBUGINFO)
    log_string("DEBUG: void close_socket: begin");
#endif

    if ( dclose->outtop > 0 ) process_output( dclose, FALSE );
    
    if (dclose->snoop_by ) write_to_buffer( dclose->snoop_by, "Your victim has left the game.\r\n", 0 );
    
    for ( d = descriptor_list; d; d = d->next ) {
	if ( d->snoop_by == dclose ) d->snoop_by = NULL;
    }
  
    ch = dclose->character;
 
    if ( ch != NULL )    {
	sprintf( log_buf, "Closing link to %s.", ch->name );
	log_string( log_buf );

       	if ( (dclose->connected == CON_PLAYING) ||
                   ((dclose->connected >= CON_NOTE_TO) &&
                   (dclose->connected <= CON_NOTE_FINISH))) {
      	         act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
	         notify_message(ch, NOTIFY_LOSTLINK, TO_CLAN, NULL);
	         sprintf (log_buf, "%s has lost link.", ch->name);
	         notify_message( ch, WIZNET_LINK, TO_IMM, log_buf);
                         if (save_first) {
                             save_char_obj(ch);
                             log_string("Playing character saved...");
                         }
	}


     	if ( (dclose->connected == CON_PLAYING)
                || ((dclose->connected >= CON_NOTE_TO) && (dclose->connected <= CON_NOTE_FINISH)))	{
	    return;
                } else {
                    free_char( dclose->original ? dclose->original : dclose->character );
                }
    }

    if ( d_next == dclose ) d_next = d_next->next;

    if ( dclose == descriptor_list )    {
	descriptor_list = descriptor_list->next;
    }    else    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d && d->next != dclose; d = d->next )  ;

	if ( d ) {
                      d->next = dclose->next;
	} else {
	    bug( "Close_socket: dclose not found.", 0 );
                    return;
                }  
    }

    if (dclose->connected != CON_TRANSFER) close( dclose->descriptor );
    free_string( dclose->host );
    free_mem(dclose->outbuf,dclose->outsize);
    dclose->next	= descriptor_free;
    descriptor_free	= dclose;
    return;
}



bool read_from_descriptor( DESCRIPTOR_DATA *d ) {
    unsigned int iStart;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if ( iStart >= sizeof(d->inbuf) - 10 )    {
	sprintf( log_buf, "%s input overflow!", d->host );
	log_string( log_buf );
	write_to_descriptor( d->descriptor, "\r\n*** PUT A LID ON IT!!! ***\r\n", 0 );
	return FALSE;
    }

    /* Snarf input. */

    for ( ; ; )    {
	int nRead;

	nRead = read( d->descriptor, (void *) (d->inbuf + iStart),  sizeof(d->inbuf) - 10 - iStart );
	if ( nRead > 0 ){
	    iStart += nRead;
	    if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    log_string( "EOF encountered on read." );
	    return FALSE;
	}
	else if ( errno == EWOULDBLOCK || errno == EAGAIN )
	    break;
	else
	{
	    perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }

    d->inbuf[iStart] = '\0';
    return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d ) {
    int i, j, k;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )    {
	if ( d->inbuf[i] == '\0' )  return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )   {
	if ( k >= MAX_INPUT_LENGTH - 2 ) {
	    write_to_descriptor( d->descriptor, "Line too long.\r\n", 0 );

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )   {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 ) d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' ) strcpy( d->incomm, d->inlast );
    else strcpy( d->inlast, d->incomm );

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}


static const char hour_names[24][16] = {
   "Midnight",      "Witching Hour",   "Late Night",          // 12,  1,  2
   "Late Night",    "Late Night",      "Twilight",           //  3,  4,  5
   "Dawn",          "Early Morning",   "Early Morning",      //  6,  7,  8
   "Morning",       "Morning",         "Late Morning",       //  9, 10, 11
   "Noon",          "Early Afternoon", "Early Afternoon",    // 12, 13, 14
   "Afternoon",     "Afternoon",       "Late Afternoon",     // 15, 16, 17
   "Dusk",          "Twilight",        "Early Evening",      // 18, 19, 20
   "Evening",       "Late Evening",    "Night"               // 21, 22, 23  
};


/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt ) {
extern bool merc_down;
int temp;
CHAR_DATA *ch, *orig;
CHAR_DATA *victim;

 /* No output for the link dead (but we pretend it worked)... */

  if (!d->ok) return TRUE;

 /* Work out who's who... */

  ch = d->character;

  orig = d->original;

  if (orig == NULL) orig = ch;

 /* Busted prompt... */ 

  if ( !merc_down ) {
    if ( d->showstr_point ) {
      write_to_buffer( d, "[Hit Return to continue]\r\n", 0 );
    } else if ( fPrompt 
             && d->pString 
             && d->connected == CON_PLAYING ) {
      write_to_buffer( d, "> ", 2 );
    } else if ( fPrompt 
             && d->connected == CON_PLAYING ) {
    
     /* battle prompt */

      victim = ch->fighting;

      if (victim != NULL) {
        int percent;
        int spercent;
        char wound[100];
	char *pbuff;
	char buf[ MAX_STRING_LENGTH ];
	char buffer[ MAX_STRING_LENGTH*2 ];
 
       /* Rough health of player... */

        if (ch->max_hit > 0) {
          percent = ch->hit * 100 / ch->max_hit;
        } else {
          percent = -1;
        }

        if (ch->max_move > 0) {
          spercent = ch->move * 100 / ch->max_move;
        } else {
          spercent = -1;
        }

        if  (spercent>percent) {

        if (percent >= 100)
          sprintf(wound,"You are in {Gexcellent condition{x.");
        else if (percent >= 85)
          sprintf(wound,"You have a {yfew scratches{x.");
        else if (percent >= 70)
          sprintf(wound,"You have some {ysmall wounds and bruises{x.");
        else if (percent >= 55) 
          sprintf(wound,"You have some {wdeep cuts{x.");
        else if (percent >= 40)
          sprintf(wound,"You have {Wquite a few wounds{x.");
        else if (percent >= 30)
          sprintf(wound,"You have some {mbig nasty wounds and scratches{x.");
        else if (percent >= 20)
          sprintf(wound,"You are {Mcovered with blood{x.");
        else if (percent >= 10)
          sprintf(wound,"You look {rpretty hurt{x.");
        else if (percent >= 0)
          sprintf(wound,"You are in {Rawful condition{x!");
        else
          sprintf(wound,"You are {Rbleeding to death{x!");
        } else {

         if (spercent >= 90)
          sprintf(wound,"You are in {Gexcellent condition{x.");
        else if (spercent >= 75)
          sprintf(wound,"You are a bit {wtired{x.");
        else if (spercent >= 50)
          sprintf(wound,"You feel completely {Wexhausted{x.");
        else if (spercent >= 30)
          sprintf(wound,"You feel really {Mweak{x.");
        else if (spercent >= 15)
          sprintf(wound,"You are {rstaggering{x dizzily.");
        else if (spercent >= 0)
          sprintf(wound,"You are {Bfainting{x!");
        else
          sprintf(wound,"You are {bknocked out{x!");
       }
 
        sprintf(buf, "%s\r\n", wound);

       /* Output time... */

	buf[0] = UPPER(buf[0]);

	pbuff = buffer;

	colourconv( pbuff, buf, orig );

        write_to_buffer( d, buffer, 0 );

       /* Rough health of opponent... */

        if (victim->max_hit > 0) {
          percent = victim->hit * 100 / victim->max_hit;
        } else {
          percent = -1;
        } 

         if (victim->max_move > 0) {
          spercent = victim->move * 100 / victim->max_move;
        } else {
          spercent = -1;
        }

        if  (spercent>percent) {

        if (percent >= 100)
          sprintf(wound,"is in {Gexcellent condition{x.");
        else if (percent >= 85)
          sprintf(wound,"has a {yfew scratches{x.");
        else if (percent >= 70)
          sprintf(wound,"has some {ysmall wounds and bruises{x.");
        else if (percent >= 55) 
          sprintf(wound,"has some {wdeep cuts{x.");
        else if (percent >= 40)
          sprintf(wound,"has {Wquite a few wounds{x.");
        else if (percent >= 30)
          sprintf(wound,"has some {mbig nasty wounds and scratches{x.");
        else if (percent >= 20)
          sprintf(wound,"is {Mcovered with blood{x.");
        else if (percent >= 10)
          sprintf(wound,"looks {rpretty hurt{x.");
        else if (percent >= 0)
          sprintf(wound,"is in {Rawful condition{x!");
        else
          sprintf(wound,"is {Rbleeding to death{x!");

        } else {

         if (spercent >= 90)
          sprintf(wound,"is in {Gexcellent condition{x.");
        else if (spercent >= 75)
          sprintf(wound,"is a bit {wtired{x.");
        else if (spercent >= 50)
          sprintf(wound,"is completely {Wexhausted{x.");
        else if (spercent >= 30)
          sprintf(wound,"is really {Mweak{x.");
        else if (spercent >= 15)
          sprintf(wound,"is {rstaggering{x dizzily.");
        else if (spercent >= 0)
          sprintf(wound,"is {Bfainting{x!");
        else
          sprintf(wound,"is {bknocked out{x!");
       }


        sprintf(buf,"%s %s \r\n", victim->short_descr,wound);
	  
       /* Output the buffer... */
  
	buf[0] = UPPER(buf[0]);

	pbuff = buffer;

	colourconv( pbuff, buf, orig );

        write_to_buffer( d, buffer, 0 );

      }

     /* Add a blank line if not compact... */

      if (!IS_SET(orig->comm, COMM_COMPACT) ) {
        write_to_buffer( d, "\r\n", 2 );
      }

     /* Format and output prompt... */ 

      if ( IS_SET(orig->comm, COMM_PROMPT)) {
                char buffer[MAX_STRING_LENGTH*2];
	char buf[MAX_STRING_LENGTH];
	char prompt_st[MAX_STRING_LENGTH];
	char *prompt_str=prompt_st;
	char *pbuff;
	int count, prompt_len; 
                int i, counter;
                char *xps;
	char st[512];

	buf[0]='\0';

                if (orig->pcdata) {
                     if (orig->pcdata->prompt) sprintf(prompt_st, "%s", orig->pcdata->prompt);
                }
	prompt_len = strlen(prompt_str);

        for (count=0;count<=prompt_len;count++) {
			
	  if (prompt_str[count]!='%') { 
	    int temp=strlen(buf);	
	    buf[temp]=prompt_str[count];
	    buf[temp+1]='\0'; 
	  } else { 
	    switch (prompt_str[count+1]) {

             /* %p is position */

	      case 'p': 
	        strcat(buf, pos_text_short(ch));
	        count += 1;
	        break;
		
             /* %a is activity */

	      case 'a': 
	        strcat(buf, acv_text_short(ch));
	        count += 1;
	        break;
		
             /* %c is a new line */

	      case 'c':
	        sprintf(st, "\r\n");
	        strcat(buf, st);
	        count += 1;
	        break;
		 
             /* %h is current hitpoints */

	      case 'h':	
                        if (ch->hit > ch->max_hit / 2) sprintf(st, "%d", ch->hit);
                        else if (ch->hit > ch->max_hit / 5) sprintf(st, "{y%d{x", ch->hit);
                        else sprintf(st, "{r%d{x", ch->hit);
	        strcat(buf, st);
	        count += 1;
	        break;
		  
             /* %H is maximum hitpoints */

	      case 'H':
		sprintf(st, "%d", ch->max_hit);
		strcat(buf, st);
		count += 1;
		break;
		  
             /* %m is current mana */

	      case 'm': 
                        if (ch->mana > ch->max_mana / 2) sprintf(st, "%d", ch->mana);
                        else if (ch->mana > ch->max_mana / 5) sprintf(st, "{y%d{x", ch->mana);
                        else sprintf(st, "{r%d{x", ch->mana);
	        strcat(buf, st);
	        count += 1;
	        break;
		
             /* %M is maximum mana */

	      case 'M':
	        sprintf (st, "%d", ch->max_mana);
	        strcat (buf, st);
	        count+=1;
	        break;

             /* %N is notes unread */

	      case 'N':
                        counter = 0;
	        for (i = 0; i < MAX_BOARD; i++) {
	            counter += UMAX(unread_notes(ch,&boards[i]), 0);
                        }
	        sprintf (st, "%d", counter);
	        strcat (buf, st);
	        count+=1;
	        break;
		

             /* %v is current move */

	      case 'v':
                        if (ch->move > ch->max_move / 2) sprintf(st, "%d", ch->move);
                        else if (ch->move > ch->max_move / 5) sprintf(st, "{y%d{x", ch->move);
                        else sprintf(st, "{r%d{x", ch->move);
	        strcat(buf, st);
	        count += 1;
	        break;
		
             /* %V is maximum move */

	      case 'V':
	        sprintf(st, "%d", ch->max_move);
	        strcat(buf, st);
	        count += 1;
	        break;
		
             /* %l is current level */

	      case 'l':
	        sprintf(st, "%d", ch->level);
	        strcat(buf, st);
	        count += 1;
	        break;
		
             /* %g is current cash */

	      case 'g':
	        sprintf(st, "%ld", get_full_cash(ch));
	        strcat(buf, st);	
	        count += 1;
	        break;
		
             /* %w is current wait states */

	      case 'w':
	        sprintf(st, "%d", ch->wait);
	        strcat(buf, st);
	        count += 1;
                        break;
		
             /* %t is current hour */

              case 't':
                strcat(buf, hour_names[time_info.hour]);
                count += 1;
                break;

             /* %T is current time */

              case 'T':
                sprintf(st, "%02d:%02d",time_info.hour, time_info.minute);
                strcat(buf, st);
                count += 1;
                break;

             /* %n is players name */

              case 'n':
                strcat(buf, ch->short_descr);
                count += 1;
                break; 

             /* %R is room vnum for immortals */

              case 'R':
		if (IS_IMMORTAL(ch)) {
                                    if (ch->in_room == NULL) {
                                        strcat(buf, "Limbo");
                                    } else {
		        sprintf(st, "%ld", ch->in_room->vnum);
		        strcat(buf, st);
                                    }
		} else {
		  temp = strlen(buf);	
		  buf[temp] = prompt_str[count];
		  buf[temp+1] = prompt_str[count+1];
		  buf[temp+2] = '\0';
                                }
		count += 1;
	                break;
		
             /* %r is room name */

	      case 'r':
		sprintf(st, "%s", ch->in_room == NULL ? "limbo" : ch->in_room->name);
		strcat(buf, st);
		count += 1;
		break;

                      case 'x':
                               i = exp_for_next_level(ch) - exp_for_level(ch->level);
                               counter = ch->exp - exp_for_level(ch->level); 
                               i = (counter * 100) /i;

                               if (i < 10) {
                                   xps = "{R";
                               } else if (i < 40) {
                                   xps = "{Y";
                               } else if (i < 90) {
                                   xps = "{G";
                               } else {
                                   xps = "{C";
                               }

		sprintf(st, "%s%d%%", xps, i);
		strcat(buf, st);
		count += 1;
		break;

		
            /* ...anything else is unregocnized... */

	     default:
		 
		temp = strlen(buf);	
		buf[temp] = prompt_str[count];
		buf[temp+1] = prompt_str[count+1];
    	                buf[temp+2] = '\0';
	                count += 1;
	    } 
	  } 
	}

	if (IS_SET(ch->plr, PLR_WIZINVIS)) {
	  char tmpbuf[32];
                  sprintf(tmpbuf, " {r({cWizi %d{r){x", ch->invis_level);
	  strcat (buf, tmpbuf);
	}

	if (IS_SET(ch->plr, PLR_CLOAK)) {
	  char tmpbuf[32];
	  sprintf (tmpbuf, " {r({cCloak %d{r){x", ch->cloak_level);
	  strcat (buf, tmpbuf);
	}

	if (IS_SET (ch->plr, PLR_AFK)) strcat (buf, " {R(AFK){x");
		
        if (ch->desc->editor != 0) {
          switch (ch->desc->editor) {

            case ED_AREA:
              strcat (buf, " {M<OLC area>{x");
              break;

            case ED_ROOM:
              strcat (buf, " {M<OLC room>{x");
              break;

            case ED_OBJECT:
              strcat (buf, " {M<OLC object>{x");
              break;

            case ED_MOBILE:
              if (ch->pcdata->macro) {
                  char buf2[MAX_INPUT_LENGTH];
                
                  sprintf(buf2, " {M<OLC mobile> [{C*REC (Id %d)*{M]{x", ch->pcdata->macro_script);
                  strcat (buf, buf2);
              } else {
                  strcat (buf, " {M<OLC mobile>{x");
              }
              break;

            case ED_HELP:
              strcat (buf, " {M<OLC help>{x");
              break;

            case ED_SOCIAL:
              strcat (buf, " {M<OLC social>{x");
              break;

            default:
              strcat (buf, " {M<OLC ?!?!>{x");
              break;
          }
        }

	strcat (buf, " ");

	pbuff=buffer;

	colourconv( pbuff, buf, orig );

	write_to_buffer( d, pbuff, 0 );
      } 

     /* Send telnet goahead if we must... */

      if (IS_SET(orig->comm, COMM_TELNET_GA)) {
        write_to_buffer(d,go_ahead_str,0);
      }
    }
 
   /* Short-circuit if nothing to write... */ 
     
    if ( d->outtop == 0 ) return TRUE;
        
   /* Snoop-o-rama... */
     
    if ( d->snoop_by ) {
        if (!IS_IMMORTAL(d->snoop_by->character)) d->snoop_by->character->mana -= 25;
        if (d->snoop_by->character->mana < 0) {
             d->snoop_by->character->mana = 0;
             d->snoop_by = NULL;
             return FALSE;
        }
      
        if (d->character) write_to_buffer(d->snoop_by, d->character->name,0);
        write_to_buffer( d->snoop_by, "> ", 2 );
        write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

   /* OS-dependent output... */
    
    if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) ) {
      d->outtop = 0;
      return FALSE;
    } else {
      d->outtop = 0;
      return TRUE;
    }
 
  }

  return TRUE;
}



/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length ) {

    /* Check the desciptor is ok... */

     if (!d->ok) return;

    /*
     * Find length in case caller didn't.
     */
    if ( length <= 0 ) length = strlen(txt);

    /*
     * Initial \r\n if needed.
     */
    if ( d->outtop == 0 && !d->fcommand ) {
	d->outbuf[0]	= '\n';
	d->outbuf[1]	= '\r';
	d->outtop	= 2;
    }

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )  {
	char *outbuf;

        if (d->outsize > 32000)	{
	    bug("Buffer overflow. Closing.\r\n",0);
	    close_socket(d, TRUE);
	    return;
 	}

	outbuf      = (char *) alloc_mem( 2 * d->outsize );
	strncpy( outbuf, d->outbuf, d->outtop );
	free_mem( d->outbuf, d->outsize );
	d->outbuf   = outbuf;
	d->outsize *= 2;
    }

    /*
     * Copy.
     */
    strncpy( d->outbuf + d->outtop, txt, length+1 );
    d->outtop += length;
    return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( int desc, char *txt, int length ) {
int iStart;
int nWrite;
size_t nBlock;

    if ( length <= 0 ) length = strlen(txt);

    for ( iStart = 0; iStart < length; iStart += nWrite )    {
	nBlock = UMIN( length - iStart, 4096 );
	if ( ( nWrite = write( desc, (void *) (txt + iStart), nBlock ) ) < 0 ) {
                        perror( "Write_to_descriptor" );
                        return FALSE;
                }
    } 

    return TRUE;
}



/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument ) {
DESCRIPTOR_DATA *d_old, *d_next;
CHAR_DATA *partner;
char buf[MAX_STRING_LENGTH];
char arg[MAX_INPUT_LENGTH];
char modpass[MAX_INPUT_LENGTH];
CHAR_DATA *ch;
CHAR_DATA *spouse;
char *pwdnew;
char *p;
int iClass, race, chfound;
bool fOld;
int rn, j;
MOB_CMD_CONTEXT *mcc;
WEV *wev;
ROOM_INDEX_DATA *start_room, *start_room2;
CUSTOMER *cust;
CHAR_DATA *keeper;
int currency, real_price;
 
    race = 0;
    /* Delete leading spaces UNLESS character is writing a note */
    if (d->connected != CON_NOTE_TEXT)    {
	while ( isspace(*argument) )
	    argument++;
    }

    ch = d->character;

    switch ( d->connected )    {

    default:
	bug( "Nanny: bad d->connected %d.", d->connected );
	close_socket(d, FALSE);
	return;


    case CON_CONFIRM_SELL:
               currency = find_currency(ch);
               keeper = ch->pcdata->store_char;
               if (keeper->pIndexData->pShop->currency > -1) currency = keeper->pIndexData->pShop->currency;

               real_price = get_cost(keeper, ch->pcdata->store_obj, FALSE );
               
               if (!str_cmp(argument, "y") || !str_cmp(argument, "yes")) {
                     do_say(ch, "Yes");

                     if (keeper->pIndexData->pShop->haggle > 90
                     && number_percent() < 50
                     && ch->pcdata->store_number[0] == real_price) {
                            do_say(keeper, "Do you think, I'm stupid? You got to haggle!");
                            sprintf(buf, "I'll give you %d %s for %s.", ch->pcdata->store_number[0], flag_string(currency_type, currency), ch->pcdata->store_obj->short_descr);
                            do_say(keeper, buf);
                            send_to_char("Accept?\r\n", ch);
                            return;
                     }

                     d->connected = CON_PLAYING;
                     execute_sell(ch, keeper, ch->pcdata->store_obj, ch->pcdata->store_number[0]);
                     ch->pcdata->store_char = NULL;
                     ch->pcdata->store_obj = NULL;
                     ch->pcdata->store_number[0] = 0;
                     ch->pcdata->store_number[1] = 0;

               } else if (!str_cmp(argument, "n") || !str_cmp(argument, "no")) {

                     do_say(ch, "No");

                     if (get_skill(ch, gsn_haggle) + number_percent() + get_curr_stat(ch, STAT_CHA) - get_skill(keeper, gsn_haggle) - get_curr_stat(keeper, STAT_WIS) + 10*get_cust_rating(keeper->pIndexData->pShop, ch->name) - ((ch->pcdata->store_number[0] - real_price) * real_price / 10) > number_percent()
                     || (keeper->pIndexData->pShop->haggle > 90 && ch->pcdata->store_number[0] == real_price)) {
                           if (number_percent() < 75) {
                               ch->pcdata->store_number[0] = ch->pcdata->store_number[0] * number_range(10, 12) /10;
                               sprintf(buf, "I'll give you %d %s for %s.", ch->pcdata->store_number[0], flag_string(currency_type, currency), ch->pcdata->store_obj->short_descr);
                               do_say(keeper, buf);
        	               check_improve(ch, gsn_haggle, TRUE, 1); 
                               send_to_char("Accept?\r\n", ch);
                           } else {
                               haggle_emote(keeper);
                               sprintf(buf, "I'll give you %d %s for %s.", ch->pcdata->store_number[0], flag_string(currency_type, currency), ch->pcdata->store_obj->short_descr);
                               do_say(keeper, buf);
                               send_to_char("Accept?\r\n", ch);
                           }
                     } else {
                           if (number_percent() > keeper->pIndexData->pShop->haggle) {
                                 if (number_percent() > keeper->pIndexData->pShop->haggle) {
                                       cust  = get_customer(keeper->pIndexData->pShop, ch->name);
                                       if (!cust) {
                                           cust =new_customer();
                                           cust->next = keeper->pIndexData->pShop->customer_list;
                                           keeper->pIndexData->pShop->customer_list = cust;
                                           cust->name = str_dup(ch->name);
                                           cust->rating = 0;
                                       }
                                       cust->rating--;
                                 }
        
                                 do_yell(keeper, "You're offending me! Go away!");
                                 ch->pcdata->store_char = NULL;
                                 ch->pcdata->store_obj = NULL;
                                 ch->pcdata->store_number[0] = 0;
                                 ch->pcdata->store_number[1] = 0;
                                 d->connected = CON_PLAYING;
                           } else {                                 
                                 sprintf(buf, "This is your last chance! I'll give you %d %s for %s.", ch->pcdata->store_number[0], flag_string(currency_type, currency), ch->pcdata->store_obj->short_descr);
                                 do_say(keeper, buf);
                                 send_to_char("Accept?\r\n", ch);
                           }
                     }

               } else {
                    sprintf(buf, "I'll give you %d %s for %s.", ch->pcdata->store_number[0], flag_string(currency_type, currency), ch->pcdata->store_obj->short_descr);
                    do_say(keeper, buf);
                    send_to_char("Accept?\r\n", ch);
               }
               return;


    case CON_CONFIRM_BUY:
               currency = find_currency(ch);
               keeper = ch->pcdata->store_char;
               if (keeper->pIndexData->pShop->currency > -1) currency = keeper->pIndexData->pShop->currency;

               real_price = get_cost(keeper, ch->pcdata->store_obj, TRUE);
               
               if (!str_cmp(argument, "y") || !str_cmp(argument, "yes")) {
                     do_say(ch, "Yes");

                     if (keeper->pIndexData->pShop->haggle > 90
                     && number_percent() < 50
                     && ch->pcdata->store_number[0] == real_price) {
                            do_say(keeper, "Do you think, I'm stupid? You got to haggle!");
                            sprintf(buf, "I want %d %s for %s.", ch->pcdata->store_number[0], flag_string(currency_type, currency), ch->pcdata->store_obj->short_descr);
                            do_say(keeper, buf);
                            send_to_char("Accept?\r\n", ch);
                            return;
                     }

                     d->connected = CON_PLAYING;
                     execute_buy(ch, keeper, ch->pcdata->store_obj, ch->pcdata->store_number[0], ch->pcdata->store_number[1]);
                     ch->pcdata->store_char = NULL;
                     ch->pcdata->store_obj = NULL;
                     ch->pcdata->store_number[0] = 0;
                     ch->pcdata->store_number[1] = 0;

               } else if (!str_cmp(argument, "n") || !str_cmp(argument, "no")) {

                     do_say(ch, "No");

                     if (get_skill(ch, gsn_haggle) + number_percent() + get_curr_stat(ch, STAT_CHA) - get_skill(keeper, gsn_haggle) - get_curr_stat(keeper, STAT_WIS) + 10*get_cust_rating(keeper->pIndexData->pShop, ch->name) - ((real_price - ch->pcdata->store_number[0]) * real_price / 10) > number_percent()
                     || (keeper->pIndexData->pShop->haggle > 90 && ch->pcdata->store_number[0] == real_price)) {
                           if (number_percent() < 75) {
                               ch->pcdata->store_number[0] = ch->pcdata->store_number[0] *10 /number_range(10, 12);
                               sprintf(buf, "I want %d %s for %s.", ch->pcdata->store_number[0], flag_string(currency_type, currency), ch->pcdata->store_obj->short_descr);
                               do_say(keeper, buf);
            	               check_improve(ch, gsn_haggle, TRUE, 1); 
                               send_to_char("Accept?\r\n", ch);
                           } else {
                               haggle_emote(keeper);
                               sprintf(buf, "I want %d %s for %s.", ch->pcdata->store_number[0], flag_string(currency_type, currency), ch->pcdata->store_obj->short_descr);
                               do_say(keeper, buf);
                               send_to_char("Accept?\r\n", ch);
                           }
                     } else {
                           if (number_percent() > keeper->pIndexData->pShop->haggle) {
                                 if (number_percent() > keeper->pIndexData->pShop->haggle) {
                                       cust  = get_customer(keeper->pIndexData->pShop, ch->name);
                                       if (!cust) {
                                           cust =new_customer();
                                           cust->next = keeper->pIndexData->pShop->customer_list;
                                           keeper->pIndexData->pShop->customer_list = cust;
                                           cust->name = str_dup(ch->name);
                                           cust->rating = 0;
                                       }
                                       cust->rating--;
                                 }
        
                                 do_yell(keeper, "You're offending me! Go away!");
                                 ch->pcdata->store_char = NULL;
                                 ch->pcdata->store_obj = NULL;
                                 ch->pcdata->store_number[0] = 0;
                                 ch->pcdata->store_number[1] = 0;
                                 d->connected = CON_PLAYING;
                           } else {                                 
                                 sprintf(buf, "This is your last chance! I want %d %s for %s.", ch->pcdata->store_number[0], flag_string(currency_type, currency), ch->pcdata->store_obj->short_descr);
                                 do_say(keeper, buf);
                                 send_to_char("Accept?\r\n", ch);
                           }
                     }

               } else {
                    sprintf(buf, "I want %d %s for %s.", ch->pcdata->store_number[0], flag_string(currency_type, currency), ch->pcdata->store_obj->short_descr);
                    do_say(keeper, buf);
                    send_to_char("Accept?\r\n", ch);
               }
               return;


    case CON_GET_NAME:
	if (argument[0] == '\0' ) {
	    close_socket(d, FALSE);
	    return;
	}

                if (!str_cmp(argument, "register_me_ping")) {
                    make_note ("Immortals", "Driver", "imp", "Registration", 14, d->host);
	    close_socket(d, FALSE);
	    return;
	}

                for (j=1; argument[j] != '\0'; j++) {
                         argument[j] = LOWER(argument[j]);
                }

	argument[0] = UPPER(argument[0]);

	if ( !check_parse_name( argument )) {
	write_to_buffer(d,
	 "\r\nThat name is unacceptable. Acceptable names will:"
	 "\r\n o Not be a word from a dictionary."
	 "\r\n o Be a modern name"
       	 "\r\n o Not be a name from a well-known book or movie."
	 "\r\n o Not be considered vulgar or profane."
	 "\r\n o Be between 3 and 12 characters in length, with no numerics."
	 "\r\n o Not have been deemed otherwise inappropriate."
	 "\r\n\r\nPlease enter a new name:\r\n",0);
	    return;
	}
	
	fOld = load_char_obj( d, argument );
	ch   = d->character;

       /* Work out the graphics settings... */

	REMOVE_BIT( ch->plr, PLR_COLOUR);
	REMOVE_BIT( ch->plr, PLR_PUEBLO);
	REMOVE_BIT( ch->plr, PLR_IMP_HTML);

	if (d->ansi == TRUE) SET_BIT(ch->plr, PLR_COLOUR );

	if (d->pueblo == TRUE) {
	    SET_BIT(ch->plr, PLR_COLOUR );
	    SET_BIT(ch->plr, PLR_PUEBLO );
	} 

	if ( d->imp_html == TRUE ) {
	    SET_BIT(    ch->plr, PLR_COLOUR );
	    SET_BIT(    ch->plr, PLR_IMP_HTML );
       	}  

       /* Evict if not wanted... */

	if ( IS_SET(ch->plr, PLR_DENY))	{
	    sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
	    log_string( log_buf );
	    write_to_buffer( d, "You are denied access.\r\n", 0 );
	    close_socket(d, FALSE);
	    return;
	}

                if (IS_MUD(ch)) {
                    int rn;

                    if ((rn =am_i_partner(ch)) >= 0) {
                        connect_single_partner(rn);
                    } else {
                        REMOVE_BIT(ch->pcdata->divinity, DIV_MUD);
   	        sprintf( log_buf, "Denying access to fake mud from %s.", d->host );
	        log_string( log_buf );
   	        close_socket(d, FALSE);
                        return;
                    }
                }
                        
	if ( check_reconnect( d, argument, FALSE ) )	{
	    fOld = TRUE;

	} else {
	    if ( IS_SET(mud.flags, MUD_WIZLOCK) 
                    && !IS_HERO(ch)) {
		write_to_buffer( d, "The game is wizlocked.\r\n", 0 );
		close_socket(d, FALSE);
		return;
	    }
	}

	if ( fOld ) {
	    write_to_buffer( d, "Password:\r\n", 0 );
	    write_to_buffer( d, echo_off_str, 0 );
	    d->connected = CON_GET_OLD_PASSWORD;
	    return;
	} else {
                    if ( IS_SET(mud.flags, MUD_NEWLOCK) ) {
                       write_to_buffer( d, "The game is newlocked.\r\n", 0 );
                       close_socket(d, FALSE);
                       return;
                    }
		/* check for login duplication cheat */
	    {
		DESCRIPTOR_DATA *tmp;
		tmp = descriptor_list;
		for (; tmp != NULL ; tmp=tmp->next) {
			CHAR_DATA *wch;
			wch = tmp->original ? tmp->original : tmp->character;
			if ( wch &&  (!str_cmp(wch->name, argument)) && (wch != ch) ) {
				sprintf (buf, "That character is already in the process of creation.\r\n");  
				write_to_buffer( d, buf, 0);	
				close_socket (d, FALSE);
				return;
			}
		}
    	    }

                    for (rn=0; rn < MAX_PARTNER; rn++) {
                          if (partner_array[rn].name
                          && partner_array[rn].status == PARTNER_CONNECTED) {
                                 partner = get_char_world(NULL, partner_array[rn].name);
                                 if (partner) {
                                      if (partner->pcdata->answer != -1) {
                                           if (!research_queue) research_queue = strdup(argument);
                                           return;
                                      }
                                 }
                          }                                
                    }

	    d->connected = CON_WAITING_NAME;
                    write_to_buffer( d, buf, 0 );

                    for (rn=0; rn < MAX_PARTNER; rn++) {
                          if (partner_array[rn].name) sprintf_to_partner(rn, "mudplayerexists %s\r\n", prepare_for_transfer(argument));
                    }

                    sprintf(d->inbuf, "\r\n");
	    return;
	}
	break;


    case CON_TRANSFER:
                sprintf(d->inbuf, "%s\r\n", argument);
                d->connected = CON_PLAYING;    
                 return;
                 break;

    case CON_WAITING_NAME:
                    for (rn=0; rn < MAX_PARTNER; rn++) {
                          if (partner_array[rn].name
                          && partner_array[rn].status == PARTNER_CONNECTED) {
                                 partner = get_char_world(NULL, partner_array[rn].name);
                                 if (partner) {
                                      if (partner->pcdata->answer == -1) {
                                           sprintf(d->inbuf, "%s\r\n", argument);
                                           return;
                                      }
                                 }
                          }                                
                    }

                    chfound = -1;
                    for (rn=0; rn < MAX_PARTNER; rn++) {
                          if (partner_array[rn].name
                          && partner_array[rn].status == PARTNER_CONNECTED) {
                                 partner = get_char_world(NULL, partner_array[rn].name);
                                 if (partner) {
                                      if (partner->pcdata->answer == 1) chfound = rn;
                                      partner->pcdata->answer = -1;
                                      if (research_queue) sprintf_to_partner(rn, "mudplayerexists %s\r\n", prepare_for_transfer(research_queue));
                                 }
                          }                                
                    }
                    free_string(research_queue);

                    if (chfound > -1) {
                        if (IS_SET(ch->plr, PLR_IMP_HTML)) {
                            sprintf(buf, "Character found on: %s %d\r\n",partner_array[chfound].ip, partner_array[chfound].port);
                            write_to_buffer( d, buf, 0);
                            write_to_buffer( d, "Re-Routing....\r\n", 0);
                            sprintf(buf, "<DISCONNECT><CNTN NAME=\"%s\" HOST=\"%s\" PORT=\"%d\"><BUTTONNAME><CT><RECONNECT>\r\n", partner_array[chfound].name, partner_array[chfound].ip, partner_array[chfound].port);
                            write_to_buffer( d, buf, 0);
                        } else {
                            sprintf(buf, "Character found on: %s %d\r\n",partner_array[chfound].ip, partner_array[chfound].port);
                            write_to_buffer( d, buf, 0);
                            write_to_buffer( d, "Please connect there.\r\n", 0);
                        }
                        close_socket (d, FALSE);
	        return;
                    }

                    sprintf( buf, "Names should not be well-known fictional names, but should be modern.\r\n"
	                          "Does your name fit this criteria, %s (Y/N)?\r\n", argument );
                    write_to_buffer( d, buf, 0 );
                    d->connected = CON_CONFIRM_NEW_NAME;
                    return;
                    break;


    case CON_WAITING_NAME2:
                    for (rn=0; rn < MAX_PARTNER; rn++) {
                          if (partner_array[rn].name
                          && partner_array[rn].status == PARTNER_CONNECTED) {
                                 partner = get_char_world(NULL, partner_array[rn].name);
                                 if (partner) {
                                      if (partner->pcdata->answer == -1) {
                                           sprintf(d->inbuf, "%s\r\n", argument);
                                           return;
                                      }
                                 }
                          }                                
                    }

                    chfound = -1;
                    for (rn=0; rn < MAX_PARTNER; rn++) {
                          if (partner_array[rn].name
                          && partner_array[rn].status == PARTNER_CONNECTED) {
                                 partner = get_char_world(NULL, partner_array[rn].name);
                                 if (partner) {
                                      if (partner->pcdata->answer == 1) chfound = rn;
                                      partner->pcdata->answer = -1;
                                      if (research_queue) sprintf_to_partner(rn, "mudplayerexists %s\r\n", prepare_for_transfer(research_queue));
                                 }
                          }                                
                    }
                    free_string(research_queue);

                    if (chfound > -1) {
                        sprintf(buf, "Character found on: %s %d\r\n",partner_array[chfound].ip, partner_array[chfound].port);
                        write_to_buffer( d, buf, 0);
                        close_socket (d, FALSE);
	        return;
                    }

                    
                    if (mud.approval_timer != 0) inform_approval(ch);
                    ch->pcdata->store_number[0] = 0;
                    ch->pcdata->store_number[1] = 0;
                    d->connected = CON_CHECK_APPROVAL;
                    sprintf(d->inbuf, "\r\n");
                    return;
                    break;


    case CON_GET_OLD_PASSWORD:
	write_to_buffer( d, "\r\n", 2 );

               if (IS_IMMORTAL(ch)) {
                  char outbuf[256];

                   if (IS_MUD(ch)) sprintf(modpass,"%s", DRIVER_VERSION);
                   else sprintf(modpass,"%s",ch->pcdata->pwd);

   	   if (str_cmp( crypt(argument, modpass ), modpass )) 	{

                       if (IS_MUD(ch)) {
                            sprintf(outbuf, "mudbug Driver Version %s expected.", DRIVER_VERSION);
                            log_string(outbuf);
                            close_socket (d, FALSE);
	            return;
                       }
   	       write_to_buffer( d, "Wrong password.\r\n", 0 );
		ch->pcdata->pwd_tries++;
		switch (ch->pcdata->pwd_tries) {
			case 1:
					write_to_buffer (d, "Try again:\r\n", 0);
					return;
					break;
			case 2:
					write_to_buffer (d, "Last try:\r\n", 0);
					return;
					break;
			case 3:	
					sprintf (outbuf, "[Alert!] Repeated login failures for %s.\r\n", d->character->name); 
					log_string (outbuf);
					close_socket (d, FALSE);
					return;
			}
 	   }
               } else {
                  if ( str_cmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd )) 	{
	         char outbuf[256];
    	          write_to_buffer( d, "Wrong password.\r\n", 0 );
		ch->pcdata->pwd_tries++;
		switch (ch->pcdata->pwd_tries) {
			case 1:
					write_to_buffer (d, "Try again:\r\n", 0);
					return;
					break;
			case 2:
					write_to_buffer (d, "Last try:\r\n", 0);
					return;
					break;
			case 3:	
					sprintf (outbuf, "[Alert!] Repeated login failures for %s.\r\n", d->character->name); 
					log_string (outbuf);
					close_socket (d, FALSE);
					return;
			}
	}

           }
           write_to_buffer( d, echo_on_str, 0 );

           if ( check_playing( d, ch->name ))	    return;

           prompt_menu(d);
           d->connected = CON_MENU;
           break;

    case CON_BREAK_CONNECT:
	switch( *argument ) {
	case 'y' : case 'Y':
                   for ( d_old = descriptor_list; d_old != NULL; d_old = d_next )    {
		d_next = d_old->next;
		if (d_old == d || d_old->character == NULL)    continue;

		if (str_cmp(ch->name,d_old->character->name))    continue;

	    }
	    if (check_reconnect(d,ch->name,TRUE))    	return;
	    write_to_buffer(d,"Reconnect attempt failed.\r\nName:\r\n",0);

            if ( d->character != NULL )            {
                free_char( d->character );
                d->character = NULL;
            }
	    d->connected = CON_GET_NAME;
	    break;

	case 'n' : case 'N':
	    write_to_buffer(d,"Name:\r\n",0);
            if ( d->character != NULL )      {
                free_char( d->character );
                d->character = NULL;
            }
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer(d,"Please type Y or N?\r\n",0);
	    break;
	}
	break;


    case CON_CONFIRM_NEW_NAME:
	switch ( *argument )	{
	case 'y': case 'Y':
                    if (mud.approval_timer == 0)  sprintf(buf, "New character.\r\nGive me a password for %s: %s\r\n", ch->name, echo_off_str );
                    else sprintf(buf, "New character.\r\nYour name is being checked for confirmation, while you proceed with character generation.\r\nGive me a password for %s: %s\r\n", ch->name, echo_off_str );

	    write_to_buffer( d, buf, 0 );
                    if (mud.approval_timer != 0) inform_approval(ch);
                    ch->pcdata->store_number[0] = 0;
                    ch->pcdata->store_number[1] = 0;
	    d->connected = CON_GET_NEW_PASSWORD;
	    break;

	case 'n': case 'N':
	    write_to_buffer( d, "Ok, what IS it, then?\r\n", 0 );
	    free_char( d->character );
	    d->character = NULL;
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "Please type Yes or No?\r\n", 0 );
	    break;
	}
	break;

    case CON_GET_NEW_PASSWORD:
	write_to_buffer( d, "\r\n", 2 );

	if ( strlen(argument) < 5 ) {
	    write_to_buffer( d, "Password must be at least five characters long.\r\nPassword:\r\n",	0 );
	    return;
	}

	pwdnew = crypt( argument, ch->name );
	for ( p = pwdnew; *p != '\0'; p++ ) {
	    if ( *p == '~' )    {
		write_to_buffer( d,
		    "New password not acceptable, try again.\r\nPassword:\r\n",
		    0 );
		return;
	    }
	}

	free_string( ch->pcdata->pwd );
	ch->pcdata->pwd	= str_dup( pwdnew );
	write_to_buffer( d, "Please retype password:\r\n", 0 );
	d->connected = CON_CONFIRM_NEW_PASSWORD;
	break;

    case CON_CONFIRM_NEW_PASSWORD:

	write_to_buffer( d, "\r\n", 2 );

	if ( str_cmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd )) {
	    write_to_buffer( d, "Passwords don't match.\r\nRetype password:\r\n", 0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    return;
	}

       /* Visible input... */
 
	write_to_buffer( d, echo_on_str, 0 );

       /* Prompt for player race... */ 

       sprintf(buf, "<a href=%s/mud/races.html>Races OnlineHelp</a>\r\n\r\n", mud.url);
       if (IS_SET(ch->plr, PLR_IMP_HTML)) {
             write_to_buffer( d, IMP_CLS "If you're new, consult:\r\n", 0 );
             write_to_buffer( d, buf, 0 );
       } else {
            write_to_buffer( d, VT_CLS, 0 );
       }

       prompt_menu(d);
       d->connected = CON_MENU;
       break;


    case CON_CHATTING:
        
        if (argument[0] == '\0') return;
        if (argument[0] == '/') {
            DESCRIPTOR_DATA *ds;

            switch(argument[1]) {
               default:
                  break;

               case '?':
                  do_help(ch, "chat");
                  break;

               case 'w':
               case 'W':
                  for (ds = descriptor_list; ds; ds = ds->next) {
                        if (d == ds) continue;

                        if (ds->connected == CON_CHATTING) {
                             if (ds->character) sprintf_to_char(ch, "%s\r\n", ds->character->name);
                        } else if (ds->connected == CON_PLAYING) {
                            if (ds->original) {
                                if (!IS_SET(ds->original->comm, COMM_NOCHAT)) sprintf_to_char(ch, "%s\r\n", ds->original->name);
                            } else if (ds->character) {
                                if (!IS_SET(ds->character->comm, COMM_NOCHAT)) sprintf_to_char(ch, "%s\r\n", ds->character->name);
                            }
                        }
                  }
                  break;

               case 'q':
               case 'Q':
                  d->connected = d->connected_old;
                  d->connected_old = 0;
                  sprintf(d->inbuf, "\r\n");
                  break;
             }
             return;
        }

        mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, gsn_english, argument);
        wev = get_wev(WEV_OOCC, WEV_OOCC_CHAT, mcc,
                 "{m[{M@a2{m]: @t0{x\r\n",
                  NULL,
                 "{m[{M@a2{m]: @t0{x\r\n");


        world_issue_wev(wev, "Chat");
        free_wev(wev);
        break;


    case CON_MENU:

           if (argument[0] == '\0' || argument[0] == 'P' || argument[0] == 'p') {
               if (ch->level < 3 && ch->pcdata->divinity == 0) {
                   prompt_race(d);
                   d->connected = CON_GET_NEW_RACE;
                   return;
               } else {
	   sprintf( log_buf, "%s@%s has connected.", ch->name, d->host );
	   log_string( log_buf );
	   notify_message (ch, WIZNET_SITES, TO_IMM, log_buf);

	   if ( IS_IMMORTAL(ch)) {
	       do_help( ch, "imotd" );
	       d->connected = CON_READ_IMOTD;

	   } else if (IS_HERO(ch)) {
	       do_help( ch, "hmotd" );
	       d->connected = CON_READ_IMOTD;

     	   } else if (IS_NEWBIE(ch)) {
	       do_help( ch, "nmotd" );
	       d->connected = CON_READ_IMOTD;

 	   } else {
	       do_help( ch, "motd" );
	       d->connected = CON_READ_MOTD;
	   }
                   return;
              }
           } else if (argument[0] == 'C' || argument[0] == 'c') {
                   if (ch->level < 3 && ch->pcdata->divinity == 0) d->connected_old = CON_MENU;
                   else d->connected_old = CON_READ_MOTD;
                   REMOVE_BIT(ch->comm, COMM_NOCHAT);
                   d->connected = CON_CHATTING;
                   do_help(ch, "chat");
                   return;
           }

           prompt_menu(d);
           break;


    case CON_GET_NEW_RACE:
        one_argument(argument,arg);

        if (argument[0] == '?') {
          if (argument[1] == '\0') {
            prompt_race(d);
          } else {
            prompt_race_info(d, argument + 2);
          } 
          return;  
        }

        if (!str_cmp(argument, "chat")) {
              d->connected_old = d->connected;
              REMOVE_BIT(ch->comm, COMM_NOCHAT);
              d->connected = CON_CHATTING;
              do_help(ch, "chat");
              break;
        }

        race = race_lookup(argument);
        if ( race == 0 || !race_array[race].pc_race) {
            prompt_race(d);
            break;
        }

                ch->race	= race;
	ch->affected_by 		= ch->affected_by | race_array[race].aff;
	ch->act	                	= ch->act | race_array[race].act;
	ch->imm_flags		= ch->imm_flags | race_array[race].imm;
	ch->envimm_flags	= ch->envimm_flags | race_array[race].envimm;
	ch->res_flags		= ch->res_flags | race_array[race].res;
	ch->vuln_flags		= ch->vuln_flags | race_array[race].vuln;
	ch->form		= race_array[race].form;
	ch->parts		= race_array[race].parts;

	/* add cost */
	ch->size = race_array[race].size;

        if (IS_SET(ch->plr, PLR_IMP_HTML)) write_to_buffer( d, IMP_CLS "What is your sex (M/F)?\r\n", 0 );
        else write_to_buffer( d, VT_CLS "What is your sex (M/F)?\r\n", 0 );

        d->connected = CON_GET_NEW_SEX;
        break;
        

    case CON_GET_NEW_SEX:
	switch ( argument[0] ) {
	case 'm': case 'M': ch->sex = SEX_MALE;    
			    ch->pcdata->true_sex = SEX_MALE;
			    break;
	case 'f': case 'F': ch->sex = SEX_FEMALE; 
			    ch->pcdata->true_sex = SEX_FEMALE;
			    break;
	default:
	    write_to_buffer( d, "That's not a gender.\r\nWhat IS your gender?\r\n" , 0 );
	    return;
	}

       /* Ask for alignment... */

	sprintf( log_buf, "%s@%s new player.", ch->name, d->host );
	log_string( log_buf );
	notify_message (ch, WIZNET_SITES, TO_IMM, log_buf);
	write_to_buffer( d, "You may be good, neutral, or evil.\r\n",0);
	write_to_buffer( d, "If you are unsure, choose neutral or good.\r\n",0);
	write_to_buffer( d, "Which alignment (G/N/E)?\r\n",0);

        d->connected = CON_GET_ALIGNMENT;
        break;

  /* Omitted by rewired state machine... */

    case CON_GET_NEW_CLASS:
	iClass = class_lookup(argument);

	if ( iClass == -1 )
	{
	    write_to_buffer( d,
		"That's not a class.\r\nWhat IS your class?\r\n", 0 );
	    return;
	}

        ch->pc_class = iClass;

	sprintf( log_buf, "%s@%s new player.", ch->name, d->host );
	log_string( log_buf );
	notify_message (ch, WIZNET_SITES, TO_IMM, log_buf);
	write_to_buffer( d, "\r\n", 2 );
	write_to_buffer( d, "You may be good, neutral, or evil.\r\n",0);
	write_to_buffer( d, "If you are unsure, choose neutral for now.\r\n",0);
	write_to_buffer( d, "Which alignment (G/N/E)?\r\n",0);
	d->connected = CON_GET_ALIGNMENT;
	break;

    case CON_GET_ALIGNMENT:
	switch( argument[0])
	{
	    case 'g': 
            case 'G': 
              ch->alignment = 750;  
              break;

	    case 'n': 
            case 'N': 
              ch->alignment = 0;
              break;

	    case 'e': 
            case 'E': 
              ch->alignment = -750; 
              break;

	    default:
	      write_to_buffer(d,"That's not a valid alignment.\r\n",0);
	      write_to_buffer(d,"Which alignment (G/N/E)?\r\n",0);
	      return;
	}

        if (IS_SET(ch->plr, PLR_IMP_HTML)) write_to_buffer( d, IMP_CLS, 0 );
        else write_to_buffer( d, VT_CLS, 0 );

        prompt_profession(d);

        d->connected = CON_GET_PROFESSION; 
        break;


    case CON_GET_PROFESSION:
        
        if (ch->profs == NULL) ch->profs = get_lprof();

        if (argument[0] == '?') {
            if (argument[1] == '\0') {
                prompt_profession(d);
            } else {
                prompt_profession_info(d, argument + 2);
            } 
            return;  
        }

        if (!str_cmp(argument, "chat")) {
              d->connected_old = d->connected;
              REMOVE_BIT(ch->comm, COMM_NOCHAT);
              d->connected = CON_CHATTING;
              do_help(ch, "chat");
              break;
        }

        ch->profs->profession = get_profession(argument);

        if (ch->profs->profession == NULL) {
            write_to_buffer(d, "That is not a valid profession!\r\n", 0);
            prompt_profession(d);
            return;
        }

        if ( !ch->profs->profession->pc
        || !ch->profs->profession->initial) {
            write_to_buffer(d, "That is not a valid profession!\r\n", 0);
            ch->profs->profession = NULL;
            prompt_profession(d);
            return;
        }

        mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL);

        if ( !ec_satisfied_mcc(mcc, ch->profs->profession->iec, TRUE)
        || !ec_satisfied_mcc(mcc, ch->profs->profession->ec, TRUE)) {
            write_to_buffer(d, "That is not a valid profession!\r\n", 0);
            ch->profs->profession = NULL;
            free_mcc(mcc);
            prompt_profession(d);
            return;
        }

        free_mcc(mcc);

        ch->profs->level = 1;
        ch->profs->next = NULL;

        if (IS_SET(ch->plr, PLR_IMP_HTML)) write_to_buffer( d, IMP_CLS, 0 );
        else write_to_buffer(d,VT_CLS, 0);

        prepare_newbie(d->character);

        if (!str_cmp(race_array[ch->race].name, "human")) {
             ch->practice += number_fuzzy(2) +1;
             prompt_amateur(d);
             d->connected = CON_GET_AMATEUR;
        } else {
             d->connected = CON_CHECK_APPROVAL;
             sprintf(d->inbuf, "\r\n");
        }
        break;


    case CON_CHANGE_NAME:
	if (argument[0] == '\0' ) {
	    close_socket(d, FALSE);
	    return;
	}

                for (j=1; argument[j] != '\0'; j++) {
                         argument[j] = LOWER(argument[j]);
                }

	argument[0] = UPPER(argument[0]);

	if ( !check_parse_name( argument )) {
	write_to_buffer(d,
	 "\r\nThat name is unacceptable. Acceptable names will:"
	 "\r\n o Not be a word from a dictionary."
	 "\r\n o Be a modern name"
       	 "\r\n o Not be a name from a well-known book or movie."
	 "\r\n o Not be considered vulgar or profane."
	 "\r\n o Be between 3 and 12 characters in length, with no numerics."
	 "\r\n o Not have been deemed otherwise inappropriate."
	 "\r\n\r\nPlease enter a new name:\r\n",0);
	    return;
	}
	
                if (name_exists(argument)) {
     	    write_to_buffer(d, "That name already exists locally.\r\n", 0);
                    write_to_buffer( d, "\r\nPlease enter your name:\r\n", 0);
                    return;
                }

                 for (rn=0; rn < MAX_PARTNER; rn++) {
                          if (partner_array[rn].name
                          && partner_array[rn].status == PARTNER_CONNECTED) {
                                 partner = get_char_world(NULL, partner_array[rn].name);
                                 if (partner) {
                                      if (partner->pcdata->answer != -1) {
                                           if (!research_queue) research_queue = strdup(argument);
                                           return;
                                      }
                                 }
                          }                                
                 }

	    d->connected = CON_WAITING_NAME2;

                    for (rn=0; rn < MAX_PARTNER; rn++) {
                          if (partner_array[rn].name) sprintf_to_partner(rn, "mudplayerexists %s\r\n", prepare_for_transfer(argument));
                    }

                    one_argument(argument, arg);
                    sprintf(buf, "@p%p_%s", ch, capitalize(arg));
                    free_string(ch->true_name);
                    ch->true_name = str_dup(buf);
                    free_string(ch->name);
                    ch->name = str_dup(capitalize(arg));
                    free_string(ch->short_descr);
                    ch->short_descr = str_dup (capitalize(arg));
                    sprintf(d->inbuf, "\r\n");
	    return;
   	    break;


    case CON_CHECK_APPROVAL:

        if (!str_cmp(argument, "chat")) {
              d->connected_old = d->connected;
              REMOVE_BIT(ch->comm, COMM_NOCHAT);
              d->connected = CON_CHATTING;
              do_help(ch, "chat");
              break;
        }
 
        if (ch->pcdata->store_number[0] == 1
         || mud.approval_timer == 0) {
             do_help(ch, "motd");
             d->connected = CON_READ_MOTD;
             return;

         } else if (ch->pcdata->store_number[0] == -1) {
             if (str_cmp(ch->pcdata->questplr, "")) sprintf(buf, "Your name has been disapproved.\r\nHow about %s?\r\n", ch->pcdata->questplr);
             else sprintf(buf, "Your name has been disapproved.\r\nTry a better one.\r\n");
             write_to_buffer(d, buf, 0);
             write_to_buffer( d, "\r\nPlease enter your name:\r\n", 0);

             free_string(ch->pcdata->questplr);
             ch->pcdata->questplr = str_dup("");
             ch->pcdata->store_number[0] = 0;
             ch->pcdata->store_number[1] = 0;

             d->connected = CON_CHANGE_NAME;
             return;

         } else {
              if (ch->pcdata->store_number[1] >= mud.approval_timer) {
                  do_help(ch, "motd");
                  d->connected = CON_READ_MOTD;
                  return;
              }
         }
        
         send_to_char("{gThere was no approval given yet.{x\r\n", ch);
         send_to_char("{gType CHAT to spend the time in chat mode.\r\nWhen you return, you should have been approved.{x\r\n", ch);
         sprintf_to_char(ch, "{r%d {gminute(s) until automatic approval.{x\r\n", mud.approval_timer - ch->pcdata->store_number[1]);
         break;

    case CON_GET_AMATEUR:
        char arg[MAX_INPUT_LENGTH];
        int sn;


        if (argument[0] == '?') {
                argument = one_argument(argument, arg);
                if (argument[0] == '\0') {
                    prompt_amateur(d);
                    return;  
                } else {
                    do_help(ch, argument);
                    write_to_buffer(d, "Enter <skill name>, ?, ? <skill>, CHAT or DONE\r\n", 0);
                    return;  
                }
        }

        if (!str_cmp(argument, "chat")) {
              d->connected_old = d->connected;
              REMOVE_BIT(ch->comm, COMM_NOCHAT);
              d->connected = CON_CHATTING;
              do_help(ch, "chat");
              break;
        }

        if (!str_cmp(argument, "done")) {
             if (ch->practice > 0) {
                 ch->sanity += number_range(5, 9) * ch->practice;
                 ch->practice = 0;
             }
             d->connected = CON_CHECK_APPROVAL;
             sprintf(d->inbuf, "\r\n");
             return;
        }

        sn = get_skill_sn(argument);
        if (sn < 0 || !skill_array[sn].amateur || !skill_array[sn].learnable) {
            write_to_buffer(d, "That's no real amateur skill.\r\n", 0);
            write_to_buffer(d, "Enter <skill name>, ?, ? <skill> or DONE\r\n", 0);
            return;
        }

        practice_amateur(d->character, sn);
        if (ch->practice <= 0) {
             prompt_amateur(d);
             d->connected = CON_CHECK_APPROVAL;
             sprintf(d->inbuf, "\r\n");
             return;
        }
        break;


    case CON_READ_IMOTD:
        write_to_buffer(d,"\r\n",2);
        do_help( ch, "motd" );
        d->connected = CON_READ_MOTD;
	break;

    case CON_CHOOSE_TERM:
        write_to_buffer(d,"\r\n",2);
//  	 sprintf( log_buf, "Loginarg:  %s", argument );
//	log_string( log_buf );

	if ( !strncmp (argument, "v1.50" , 5 ))	{
		d->ansi = TRUE;
		d->imp_html = TRUE;
		write_to_buffer( d, "<B>IMP HTML Enabled!</B>\r\n", 0 );
	} else if ( !strncmp (argument, "PUEBLOCLIENT", 12 ) ) {
		d->ansi = TRUE;
		d->pueblo = TRUE;
		write_to_buffer( d, "Pueblo Enabled!\r\n", 0 );
	} else {
                     switch ( argument[0] ) {
    	           case 'y': case 'Y':
	    	d->ansi = TRUE;
		d->pueblo = FALSE;
		d->imp_html = FALSE;
                                write_to_buffer( d, C_B_YELLOW "ANSI Color!" CLEAR "\r\n", 0 );
		break;
                           case 'n': case 'N':
		d->ansi = FALSE;
		d->pueblo = FALSE;
		d->imp_html = FALSE;
                                write_to_buffer( d, "Green on green\r\n", 0 );
		break;
                          default:
                               write_to_buffer( d, "Please answer (Y/N)?\r\n", 0 );
                               return;
                  }

        }

        for (d_old = descriptor_list; d_old; d_old = d_next) {
              d_next = d_old->next;
              if (!str_cmp(d_old->host, d->host)
              && d_old->connected == CON_TRANSFER) {
                      d_old->character->desc = d;
                      d->character = d_old->character;
                      d_old->character = NULL;
                      d_old->ok = FALSE;
                      close_socket(d_old, FALSE);
                      d->connected = CON_PLAYING;
                      sprintf(d->inbuf, "look\r\n");
                      return;
              }                     
        }


	/*
         * Send the greeting.
         *
         * BUG: If the help greeting is changed online, this pointer
         *      is left dangling...
         */
        {
            extern char * help_greeting;
            extern char * help_impgreeting;

           write_to_buffer ( d, "\r\n", 0 );

            if (d->imp_html == TRUE) {
                write_to_buffer ( d, IMP_CLS "\r\n", 0 );
                if ( help_impgreeting[0] == '.' ) write_to_buffer( d, help_impgreeting+1, 0 );
                else write_to_buffer( d, help_impgreeting  , 0 );
            } else {
                write_to_buffer ( d, "\r\n", 0 );
                if ( help_greeting[0] == '.' ) write_to_buffer( d, help_greeting+1, 0 );
                else write_to_buffer( d, help_greeting  , 0 );
            }
            write_to_buffer( d, "\r\nPlease enter your name:\r\n", 0);
        }
        d->connected = CON_GET_NAME;
        break;


    case CON_READ_MOTD:

        free_string(ch->pcdata->questplr);
        ch->pcdata->questplr = str_dup("");
        ch->pcdata->store_number[0] = 0;
        ch->pcdata->store_number[1] = 0;

        if (IS_SET(ch->plr, PLR_IMP_HTML)) write_to_buffer( d, IMP_CLS, 0 );
        else write_to_buffer(d,VT_CLS, 0);

	if ( ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0') {
	    write_to_buffer( d, "Warning! Null password!\r\n",0 );
	    write_to_buffer( d, "Please report this error!\r\n",0);
	    write_to_buffer( d, "Type 'password null <new password>' to fix.\r\n",0);
	}

       /* Say G'Day... */

        sprintf(buf, "\r\n"  "Welcome to '%s' (CthulhuMud Driver %s)\r\n", mud.name, DRIVER_VERSION );
        write_to_buffer( d, buf, 0);

       /* Inform on PK settings... */

        if (IS_SET(mud.flags, MUD_PERMAPK)) write_to_buffer( d, "The mud is in perma-death player combat mode.\r\n", 0);
        else write_to_buffer( d, "The mud is in free player combat mode.\r\n", 0);
        if (mud.pk_timer > 0) {
            sprintf(buf, "(PK Timer = %d minutes)\r\n", mud.pk_timer);
            write_to_buffer(d, buf, 0);
        }

        notify_message (ch, WIZNET_NEWBIE, TO_IMM, NULL);

	ch->next	= char_list;
	char_list	= ch;
	d->connected	= CON_PLAYING;
	reset_char(ch);

       /* Work out the mobs true_name... */

       /* Pointer for uniqueness
          Name incase they logoff and another player gets the same char_data */

        sprintf(buf, "@p%p_%s", ch, ch->name);

        ch->true_name = str_dup(buf);

       /* Ok, now for a newbie there are still some things missing... */

            if ( ch->level == 0 ) {
                ch->version		= 7;

           /* Training and sanity... */

    	ch->level	= 1;
    	ch->exp		= exp_for_level(1);
                ch->practice 	= 5;
                ch->train	= 1;
                ch->max_hit  	= 18 + (get_curr_stat(ch, STAT_CON)/8);
                ch->max_mana 	= 15 + ( get_curr_stat(ch, STAT_INT) + get_curr_stat(ch, STAT_WIS))/16;
                ch->max_move 	= 18 + (get_curr_stat(ch,STAT_DEX)/8);
                ch->hit		= ch->max_hit;
                ch->mana	= ch->max_mana;
                ch->move	= ch->max_move;

                for(sn = 0; sn < MOB_MEMORY; sn++) {
                     ch->memory[sn] = NULL;
                }

                default_memberships(ch);

           /* Initial auto settings... */

	 SET_BIT( ch->plr, PLR_AUTOEXIT );
	 SET_BIT( ch->plr, PLR_AUTOASSIST );
	 SET_BIT( ch->plr, PLR_AUTOSPLIT );
	 SET_BIT( ch->plr, PLR_AUTOSAVE );
	 SET_BIT( ch->plr, PLR_AUTOLOOT );
	 SET_BIT( ch->plr, PLR_AUTOGOLD );
	 SET_BIT( ch->plr, PLR_AUTOSAC );
                 SET_BIT( ch->plr, PLR_AUTOKILL );
                  SET_BIT( ch->plr, PLR_REASON);

                 ch->pcdata->blood=0;

                 set_title( ch, "the hopeful" );
                 free_string( ch->pcdata->cult );
                 ch->pcdata->cult = str_dup(cult_array[1].name);
                 do_worship(ch, race_array[ch->race].cult);
     
                 if (!str_cmp(race_array[ch->race].name, "zoog")) {
                      set_title( ch, "the shadow of the woods" );
                 }
                 if (!str_cmp(race_array[ch->race].name, "mi-go")) {
                      set_title( ch, "the unseen" );
                 }
                 if (!str_cmp(race_array[ch->race].name, "deep one")) {
                      set_title( ch, "the lurker of the reef" );
                 }
                 if (!str_cmp(race_array[ch->race].name, "yithian")) {
                      set_title( ch, "the puppet master" );
                 }

           /* Free equipment... */

	equip_char_by_prof(ch, TRUE);

           /* Set permanent recall location... */

	 ch->recall_perm = mud.recall;

           /* Find the right start room... */

                 start_room = NULL;

                 if (ch->profs->profession->start != 0) {
                      start_room = get_room_index(ch->profs->profession->start);
                 }

                 if (start_room == NULL) start_room = get_room_index( mud.start );

                 if ( IS_SET(time_info.flags, TIME_NIGHT) ) {
                      if (start_room->night != 0) {
                           start_room2 = get_room_index(start_room->night);
                           if (start_room2 != NULL) start_room = start_room2;
                      }
                 } else if ( IS_SET(time_info.flags, TIME_DAY) ) {
                      if (start_room->day != 0) {
                           start_room2 = get_room_index(start_room->day);
                           if (start_room2 != NULL) start_room = start_room2;
                      }
                 }

           /* Move to the start room... */

	 char_to_room( ch, start_room );

           /* Save the character... */

	 save_char_obj( ch ); 

           /* Give them a hint... */

	 send_to_char("\r\n",ch);
	 do_help(ch,"summary");
	 send_to_char("\r\n",ch);

           /* ...and they're on their own... */

                 send_to_char("Good luck, monster fodder...\r\n", ch);

         } else if ( ch->in_room != NULL ){
            start_room = ch->in_room;

            if ( IS_SET(time_info.flags, TIME_NIGHT) ) {

              if (start_room->night != 0) {

                start_room2 = get_room_index(start_room->night);

                if (start_room2 != NULL) start_room = start_room2;
              }
            } else if ( IS_SET(time_info.flags, TIME_DAY) ) {

              if (start_room->day != 0) {

                start_room2 = get_room_index(start_room->day);

                if (start_room2 != NULL) {
                  start_room = start_room2;
                }
              }
            }

	    char_to_room( ch, start_room );

          } else {
	    start_room = get_room_index( mud.start );

            if ( IS_SET(time_info.flags, TIME_NIGHT) ) {

              if (start_room->night != 0) {

                start_room2 = get_room_index(start_room->night);

                if (start_room2 != NULL) start_room = start_room2;
              }
            } else if ( IS_SET(time_info.flags, TIME_DAY) ) {

              if (start_room->day != 0) {

                start_room2 = get_room_index(start_room->day);

                if (start_room2 != NULL) {
                  start_room = start_room2;
                }
              }
            }

	    char_to_room( ch, start_room );
	}

        mcc = get_mcc(ch, ch, NULL, NULL, NULL, NULL, 0, NULL);
        wev = get_wev(WEV_CONTROL, WEV_CONTROL_LOGIN, mcc,
                     "{MYou step out of the shadows.{x\r\n",
                      NULL,
                     "{M@a2 steps out of the shadows.{x\r\n");

        room_issue_wev(ch->in_room, wev);
 
        free_wev(wev);

         if (!IS_IMMORTAL(ch)) notify_message (ch, NOTIFY_LOGIN, TO_ALL, NULL);
         
        do_board (ch, "");
        do_look( ch, "auto" );
        if (str_cmp(ch->pcdata->spouse,"")) {
            if ((spouse = get_char_world( ch, ch->pcdata->spouse) ) != NULL) {
                 sprintf(buf, "{r%s{x is currently online...\r\n", spouse->name);
                 send_to_char(buf, ch );
            }
        }


       /* Adjust to minimum character level... */

        if (ch->level < NEWBIE_START_LEVEL) {
            gain_exp( ch, exp_for_level(NEWBIE_START_LEVEL) - ch->exp + 100 + number_bits(7), TRUE);
            send_to_char("You enter the world as a young adult.\r\n"
                                    "Remember to practice your skills.\r\n", ch);
        } 

       /* Fix English... */

        if ( ch->effective->skill[gsn_english] < 50 ) {
         ch->effective->skill[gsn_english] = 51; 
        }

       /* Fix Racial Language... */
 
        if ( ch->race > 0
          && race_array[ch->race].language > 0 ) {
          if ( ch->effective->skill[race_array[ch->race].language] < 50 ) {
           ch->effective->skill[race_array[ch->race].language] = 51; 
          }
        }

       /* Fix Dreaming... */

        if ( ch->effective->skill[gsn_dreaming] < 1 ) ch->effective->skill[gsn_dreaming] = 1; 
        if (IS_SET(ch->act,ACT_DREAM_NATIVE)) ch->effective->skill[gsn_dreaming] =0;

       /* Try and avoid pets in limbo... */ 

        if ( ch->pet != NULL ) {

          if ( ch->pet->in_room == NULL) {
            char_to_room(ch->pet, ch->in_room);
          } else { 
            char_to_room(ch->pet, ch->pet->in_room);
          }

         /* Issue an arrival WEV for the pet... */

          mcc = get_mcc(ch->pet, ch->pet, NULL, NULL, NULL, NULL, 0, NULL);
          wev = get_wev(WEV_CONTROL, WEV_CONTROL_LOGIN, mcc,
                       "{MYou step out of the shadows.{x\r\n",
                        NULL,
                       "{M@a2 steps out of the shadows.{x\r\n");

          room_issue_wev(ch->pet->in_room, wev);
 
          free_wev(wev);
	}

	break;

        case CON_NOTE_BOARD:
                if (is_number(argument)) {
                    do_board(ch, argument);
   	    send_to_char ("{YTo{x:      ",ch);
                    d->connected = CON_NOTE_TO;
                } else {
                    send_to_char("Forward to board nr.:\r\n", ch);
                }
                break;

        case CON_NOTE_TO:
                handle_con_note_to (d, argument);
                break;

        case CON_NOTE_SUBJECT:
                handle_con_note_subject (d, argument);
                break;

        case CON_NOTE_EXPIRE:
                handle_con_note_expire (d, argument);
                break;

        case CON_NOTE_TEXT:
                handle_con_note_text (d, argument);
                break;

        case CON_NOTE_FINISH:
                handle_con_note_finish (d, argument);
                break;

    }

    return;
}

/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name ) {
    char buf[MAX_STRING_LENGTH];

    /*
     * Reserved words.
     */
    buf[0]='\0';

    sprintf(buf,"grep -x %s bad.names > /dev/null",name);
    if(0==system(buf))  {
	buf[0]='\0';
	return FALSE;
    }

    if ( strlen(name) <  3 )
	return FALSE;

    if ( strlen(name) > 12 )
	return FALSE;

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) ) return FALSE;
	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' ) fIll = FALSE;
	}

	if ( fIll )  return FALSE;
    }

    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn ) {
CHAR_DATA *ch;

#if defined(DEBUGINFO)
    log_string("DEBUG: bool check_reconnect: begin");
#endif

    for ( ch = char_list; ch != NULL; ch = ch->next )   {
	if ( !IS_NPC(ch)
                &&   (ch != d->character)
	&&   !str_cmp( d->character->name, ch->name )) {
	    if ( fConn == FALSE ) {
		free_string( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
	    } else  {
		OBJ_DATA *obj;
                                DESCRIPTOR_DATA *old_d;

               /* Save old descriptor... */

                old_d = ch->desc;

               /* Lose the new character... */

		d->character->quitting = TRUE;
		free_char( d->character );

               /* Pick up the old character... */

                ch->desc = d;
                d->character = ch;

               /* Close the old socket... */
 
                old_d->character = NULL;
	close_socket(old_d, FALSE);
  	
               /* Reset the terminal flags... */
  
  		if (d->ansi == TRUE) {
       		    SET_BIT( ch->plr, PLR_COLOUR );
  		    REMOVE_BIT( ch->plr, PLR_PUEBLO );

  		} else if (d->ansi == FALSE){
  		    REMOVE_BIT( ch->plr, PLR_COLOUR);
  		    REMOVE_BIT( ch->plr, PLR_PUEBLO);
  		}

  		if (d->pueblo == TRUE){
  		    SET_BIT( ch->plr, PLR_COLOUR );
  		    SET_BIT( ch->plr, PLR_PUEBLO );
  		}
  
               /* Reset the mobs timer... */

		ch->timer	 = 0;
		d->connected = CON_PLAYING;

                if ( ( obj = get_eq_char( d->character, WEAR_LIGHT ) ) != NULL
	          &&   obj->item_type == ITEM_LIGHT
	          &&   obj->value[2] != 0 ) d->character->in_room->light++;
	       
		send_to_char( "Reconnecting.\r\n", ch );
		act( "$n eyes lose their glazed appearance.", ch, NULL, NULL, TO_ROOM );

		notify_message(ch, NOTIFY_RECONNECT, TO_CLAN, NULL );
		sprintf( log_buf, "%s@%s reconnected.", ch->name, d->host );
		notify_message (ch, WIZNET_LINK, TO_IMM, log_buf);
		log_string( log_buf );

                                if (ch->pcdata->in_progress)  send_to_char ("You have a note in progress. Type NWRITE to continue it.\r\n",ch);
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
	if ( dold != d
	&&   dold->character != NULL
	&&   dold->connected != CON_GET_NAME
	&&   dold->connected != CON_GET_OLD_PASSWORD
	&&   !str_cmp( name, dold->original
	         ? dold->original->name : dold->character->name ) )
	{
	    write_to_buffer( d, "That character is already playing.\r\n",0);
	    write_to_buffer( d, "Do you wish to connect anyway (Y/N)?\r\n",0);
	    d->connected = CON_BREAK_CONNECT;
	    return TRUE;
	}
    }

    return FALSE;
}



void stop_idling( CHAR_DATA *ch ) {

    if (ch == NULL) return;

    ch->timer = 0;

    if ( ch->desc == NULL 
      || ch->desc->connected != CON_PLAYING
      || ch->was_in_room == NULL 
      || ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
	return;

    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room	= NULL;

    act( "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}



/*
 * Write to one char.
 */
void send_to_char_bw( const char *txt, CHAR_DATA *ch ) {

    if ( txt == NULL 
      && ch->desc == NULL
      && ch != ch->desc->character ) {
      return;
    }
  
    write_to_buffer( ch->desc, txt, strlen(txt) );

    return;
}


/* Write to one char, new colour version, by Lope... */
/* Fixes for switch by Mik... */
/* HTML translation by Mik... */

void send_to_char( const char *txt, CHAR_DATA *ch ) {
const char 	*point;
char 	*point2;
char 	buf[ MAX_STRING_LENGTH*4 ];
char 	buf2[ MAX_STRING_LENGTH*4 ];
int		skip = 0;

    CHAR_DATA *orig;
 
   /* Validate... */

    if ( txt == NULL
      || ch->desc == NULL
      || ch != ch->desc->character) {
      return;
    } 

   /* Find the original... */

    orig = ch->desc->original;

    if (orig == NULL) {
      orig = ch;
    }

   /* Prepare for formatting... */

    buf[0] = '\0';
    point = txt;
    point2 = buf;

   /* Format html to text... */

    if ( !IS_SET( orig->plr, PLR_IMP_HTML ) ) {
      if ( strip_html(txt, buf2, TRUE, MAX_STRING_LENGTH * 4, FONT_CLEAR ) ) { 
        point = buf2; 
      }
    }

   /* Format text to Ansi... */

    if( IS_SET( orig->plr, PLR_COLOUR ) ) {

      for( ; *point ; point++ ) {

        if( *point == '{' ) {
          point++;

          skip = colour( *point, orig, point2 );

          while( skip-- > 0 ) {
            ++point2;
          }

          continue;
        }
        *point2 = *point;
        *++point2 = '\0';
      }			

      *point2 = '\0';

      write_to_buffer( ch->desc, buf, point2 - buf );

    } else {
      for( ; *point ; point++ ) {
        if( *point == '{' ) {
          point++;
          continue;
        }

        *point2 = *point;
        *++point2 = '\0';
      } 

      *point2 = '\0';
      write_to_buffer( ch->desc, buf, point2 - buf );
    }

    return;
}

/*
 * Send a page to one char.
 */
void page_to_char_bw( const char *txt, CHAR_DATA *ch ) {
    if ( txt == NULL || ch->desc == NULL) return;
	
    ch->desc->showstr_head = (char *) alloc_mem(strlen(txt) + 1);
    strcpy(ch->desc->showstr_head,txt);
    ch->desc->showstr_point = ch->desc->showstr_head;
    show_string(ch->desc,"");
}


//
// Page to one char, new colour version, by Lope.
//
// Extended for HTML by Mik
//

void page_to_char( const char *txt, CHAR_DATA *ch ) {
const char	*point;
char	*point2;
char	buf[ MAX_STRING_LENGTH * 4 ];
char 	buf2[ MAX_STRING_LENGTH*4 ];
int		skip = 0;
CHAR_DATA *orig;

   /* Validate... */

    if ( txt == NULL
      || ch->desc == NULL
      || ch != ch->desc->character) {
      return;
    } 

   /* Find the original... */

    orig = ch->desc->original;

    if (orig == NULL) {
      orig = ch;
    }

    buf[0] = '\0';
    point = txt;
    point2 = buf;

   /* Format html to text... */

    if ( !IS_SET( orig->plr, PLR_IMP_HTML ) ) {
      if ( strip_html(txt, buf2, TRUE, MAX_STRING_LENGTH * 4, FONT_CLEAR) ) { 
        point = buf2; 
      }
    }

   /* Format text to ansi... */

    if( IS_SET( orig->plr, PLR_COLOUR ) ) {

      for( ; *point ; point++ ) {
        
        if( *point == '{' ) {
          point++;

          skip = colour( *point, ch, point2 );
 
          while( skip-- > 0 ) {
            ++point2;
          }

          continue;
        }

        *point2 = *point;
        *++point2 = '\0';
      }
			
      *point2 = '\0';
		
      if (ch->desc->showstr_head) {
         free_string( ch->desc->showstr_head );
         ch->desc->showstr_head = 0;
      }

      ch->desc->showstr_head  = str_dup( buf );
      ch->desc->showstr_point = ch->desc->showstr_head;
      show_string( ch->desc, "" );

   } else {
     
      for(  ; *point ; point++ ) {

        if( *point == '{' ) {
          point++;
          continue;
        }

        *point2 = *point;
        *++point2 = '\0';
      }

      *point2 = '\0';

      if (ch->desc->showstr_head) {
        free_string( ch->desc->showstr_head );
        ch->desc->showstr_head = 0;
      }

      ch->desc->showstr_head  = str_dup( buf );
      ch->desc->showstr_point = ch->desc->showstr_head;
      show_string( ch->desc, "" );
    }

    return;
}


/* string pager */
void show_string(struct descriptor_data *d, char *input) {
char buffer[4*MAX_STRING_LENGTH];
char buf[MAX_INPUT_LENGTH];
register char *scan, *chk;
int lines = 0, toggle = 1;
int show_lines;

    one_argument(input,buf);
    if (buf[0] != '\0') {
	if (d->showstr_head) {
	    free_string(d->showstr_head);
	    d->showstr_head = 0;
	}
    	d->showstr_point  = 0;
	return;
    }

    if (d->character) show_lines = d->character->lines;
    else show_lines = 0;

    for (scan = buffer; ; scan++, d->showstr_point++)  {
	if (((*scan = *d->showstr_point) == '\n' || *scan == '\r')  && (toggle = -toggle) < 0)  lines++;
	else if (!*scan || (show_lines > 0 && lines >= show_lines)) {
	    *scan = '\0';
	    write_to_buffer(d,buffer,strlen(buffer));

	    for (chk = d->showstr_point; isspace(*chk); chk++);
	    {
		if (!*chk) {
		    if (d->showstr_head)   {
            	    	          free_string(d->showstr_head);
            		          d->showstr_head = 0;
        	                    }
        	                    d->showstr_point  = 0;
    		}
	    }
	    return;
	}
    }
    return;
}
	
/* Cursor Manipulation Commands */
/* GotoXY */
/* args: ( ch, line , column ) */

void gotoxy(CHAR_DATA *ch, int arg1, int arg2) {
    char buf[20];

    if ( !IS_SET( ch->plr, PLR_CURSOR ))   {
	log_string("Error! GotoXY called when PLR_CURSOR was turned off!");
	return;
    }
    sprintf (buf, "\033[%d;%df", arg1, arg2);
    send_to_char(buf, ch);
}

/* quick sex fixer */
void fix_sex(CHAR_DATA *ch) {
    if (ch->sex < 0 || ch->sex > 2)
    	ch->sex = IS_NPC(ch) ? 0 : ch->pcdata->true_sex;
}


void act (const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2,  int type) {
    act_new(format,ch,arg1,arg2,type,POS_RESTING);
}


void act_new( const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type, int min_pos ) {
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
 
    CHAR_DATA 		*to;
    CHAR_DATA 		*vch = ( CHAR_DATA * ) arg2;
    OBJ_DATA 		*obj1 = ( OBJ_DATA  * ) arg1;
    OBJ_DATA 		*obj2 = ( OBJ_DATA  * ) arg2;
    ROOM_INDEX_DATA  *room = NULL;
    const 	char 	*str;
    const char 		*i;
    char 		*point;
    char 		*pbuff;
    char 		buf[ MAX_STRING_LENGTH   ];
    char 		buffer[ MAX_STRING_LENGTH*2 ];
    char 		fname[ MAX_INPUT_LENGTH  ];
    bool		fColour = FALSE;

    if( !format || !*format ) return;

    if( !ch || !ch->in_room) {
       if (obj1) {
           if (obj1->in_room) {
               room = obj1->in_room;
           } else if (obj1->in_obj) {
               if (obj1->in_obj->in_room) {
                   room =obj1->in_obj->in_room;
               } else if (obj1->in_obj->carried_by) {
                   room =obj1->in_obj->carried_by->in_room;
               }
           } else if (obj1->carried_by) {
               room = obj1->carried_by->in_room;
           }
       }
    } else {
       room = ch->in_room;
    }

    if (!room) return;

    to = room->people;
    if( type == TO_VICT )   {
        if( !vch )       {
            bug( "Act: null vch with TO_VICT.", 0 );
            return;
        }
        if( !vch->in_room )    return;
        to = vch->in_room->people;
    }
 
    for( ; to ; to = to->next_in_room )    {
        if( !to->desc || to->position < min_pos )   continue;
        if( type == TO_CHAR && to != ch )   continue;
        if( type == TO_VICT && ( to != vch || to == ch ) )  continue;
        if( type == TO_ROOM && to == ch )   continue;
        if( type == TO_NOTVICT && ( to == ch || to == vch ) )   continue;
 
        point   = buf;
        str     = format;
        while( *str )
        {
            if( *str != '$' )
            {
                *point++ = *str++;
                continue;
            }

	    i = NULL;
	    switch( *str )
	    {
		case '$':
		    fColour = TRUE;
		    ++str;
		    i = " <@@@> ";
		    if ( !arg2 && *str >= 'A' && *str <= 'Z' )
		    {
			bug( "Act: missing arg2 for code %d.", *str );
			i = " <@@@> ";
		    }
		    else
		    {
			switch ( *str )
			{
			    default:  
				bug( "Act: bad code %d.", *str );
				i = " <@@@> ";                                
				break;

			    case 't': 
				i = (char *) arg1;                            
				break;

			    case 'T': 
				i = (char *) arg2;                            
				break;

			    case 'n': 
				if (type == TO_ROOM) {
					i = PERSMASK( ch,  to  );                         
					break;
				}
				i = PERS( ch,  to  );                         
				break;

			    case 'N': 
				if (type == TO_ROOM) {
					i = PERSMASK( vch,  to  );                         
					break;
				}
				i = PERS( vch, to  );                         
				break;

			    case 'o': 
				if (type == TO_ROOM) {
					i = PERSORIG(ch,  to);                         
					break;
				}
				i = PERS(ch,  to);                         
				break;

			    case 'O': 
				if (type == TO_ROOM) {
					i = PERSORIG(vch,  to);                         
					break;
				}
				i = PERS(vch, to);                         
				break;

			    case 'e': 
				i = he_she  [URANGE(0, ch  ->sex, 2)];        
				break;

			    case 'E': 
				i = he_she  [URANGE(0, vch ->sex, 2)];        
				break;

			    case 'm': 
				i = him_her [URANGE(0, ch  ->sex, 2)];        
				break;

			    case 'M': 
				i = him_her [URANGE(0, vch ->sex, 2)];        
				break;

			    case 's': 
				i = his_her [URANGE(0, ch  ->sex, 2)];        
				break;

			    case 'S': 
				i = his_her [URANGE(0, vch ->sex, 2)];        
				break;
 
			    case 'p':
				i = can_see_obj( to, obj1 )
				  ? obj1->short_descr
				  : "something";
				break;
 
			    case 'P':
				i = can_see_obj( to, obj2 )
				  ? obj2->short_descr
				  : "something";
				break;
 
			    case 'd':
				if ( !arg2 || ((char *) arg2)[0] == '\0' )
				{
				    i = "door";
				}
				else
				{
				    one_argument( (char *) arg2, fname );
				    i = fname;
				}
				break;
			}
		    }
		    break;

		default:
		    fColour = FALSE;
		    *point++ = *str++;
		    break;
	    }

            ++str;
	    if( i )
	    {
		while( ( *point = *i ) != '\0' )
		{
		    ++point;
		    ++i;
		}
	    }
        }
 
        *point++	= '\n';
        *point++	= '\r';
        *point		= '\0';
	buf[0]		= UPPER( buf[0] );
	pbuff		= buffer;
	colourconv( pbuff, buf, to );
	if( to->desc && (to->desc->connected == CON_PLAYING))
	    write_to_buffer( to->desc, buffer, 0 );

/* MOB Action triggers moved to seperate section, as this bit of     Mik Fix */
/* the code is NEVER REACHED for NPCs! (the !to->desc gets them)     Mik Fix */

    }

/* Now we loop through the NPCs and trigger their ACT progs...       Mik Fix */
/* MOBtrigger is false when issuing MP commands...                   Mik Fix */

    if (MOBtrigger) {                                             /* Mik Fix */

     /* Reset the list of who we send it to...                       Mik Fix */ 

      to = room->people;                                   /* Mik Fix */

     /* For TO_VICT we only send to the victim...                    Mik Fix */ 
     /* This was validated before the NPC send loop...               Mik Fix */ 
 
      if( type == TO_VICT ) to = vch->in_room->people;

      for( ; to ; to = to->next_in_room ) {

      /* Live NPCs only...                                           Mik Fix */

        if( to->desc || to->position < min_pos ) continue; 

      /* No direct feedback...                                       Mik Fix */
 
        if( type == TO_CHAR ) continue; 

      /* Only the victim get the victims message...                  Mik Fix */

        if( type == TO_VICT && ( to != vch || to == ch )) continue;

      /* Room messages don't go to their source...                   Mik Fix */

        if( type == TO_ROOM && to == ch ) continue;  

      /* Only observers get observer messages...                     Mik Fix */

        if( type == TO_NOTVICT && ( to == ch || to == vch )) continue; 
  
      /* Only format if the MOB has an ACT MOB PROG...               Mik Fix */

        if (!to->pIndexData) continue;
        if (!(to->pIndexData->progtypes & ACT_PROG)) continue; 

      /* Format the message for the mob (code from above)            Mik Fix */

        point   = buf;
        str     = format;
        while( *str )
        {
            if( *str != '$' )
            {
                *point++ = *str++;
                continue;
            }

	    i = NULL;
	    switch( *str )
	    {
		case '$':
		    fColour = TRUE;
		    ++str;
		    i = " <@@@> ";
		    if ( !arg2 && *str >= 'A' && *str <= 'Z' )
		    {
			bug( "Act: missing arg2 for code %d.", *str );
			i = " <@@@> ";
		    }
		    else
		    {
			switch ( *str )
			{
			    default:  
				bug( "Act: bad code %d.", *str );
				i = " <@@@> ";                                
				break;

			    case 't': 
				i = (char *) arg1;                            
				break;

			    case 'T': 
				i = (char *) arg2;                            
				break;

			    case 'n': 
				if (type == TO_ROOM) {
					i = PERSMASK( ch,  to  );                         
					break;
					}
				i = PERS( ch,  to  );                         
				break;

			    case 'N': 
				if (type == TO_ROOM) {
					i = PERSMASK( vch,  to  );                         
					break;
					}
				i = PERS( vch, to  );                         
				break;

			    case 'o': 
				if (type == TO_ROOM) {
					i = PERSORIG( ch,  to  );                         
					break;
					}
				i = PERS( ch,  to  );                         
				break;

			    case 'O': 
				if (type == TO_ROOM) {
					i = PERSORIG( vch,  to  );                         
					break;
					}
				i = PERS( vch, to  );                         
				break;

			    case 'e': 
				i = he_she  [URANGE(0, ch  ->sex, 2)];        
				break;

			    case 'E': 
				i = he_she  [URANGE(0, vch ->sex, 2)];        
				break;

			    case 'm': 
				i = him_her [URANGE(0, ch  ->sex, 2)];        
				break;

			    case 'M': 
				i = him_her [URANGE(0, vch ->sex, 2)];        
				break;

			    case 's': 
				i = his_her [URANGE(0, ch  ->sex, 2)];        
				break;

			    case 'S': 
				i = his_her [URANGE(0, vch ->sex, 2)];        
				break;
 
			    case 'p':
				i = can_see_obj( to, obj1 )
				  ? obj1->short_descr
				  : "something";
				break;
 
			    case 'P':
				i = can_see_obj( to, obj2 )
				  ? obj2->short_descr
				  : "something";
				break;
 
			    case 'd':
				if ( !arg2 || ((char *) arg2)[0] == '\0' )	{
				    i = "door";
				} else {
				    one_argument( (char *) arg2, fname );
				    i = fname;
				}
				break;
			}
		    }
		    break;

		default:
		    fColour = FALSE;
		    *point++ = *str++;
		    break;
	    }

            ++str;
	    if( i )   {
		while( ( *point = *i ) != '\0' ){
		    ++point;
		    ++i;
		}
	    }
        }
 
        *point++	= '\r';
        *point++	= '\n';
        *point		= '\0';

      /* Now we get to see if the MOB has anything to do with it...  Mik Fix */
     
        mprog_act_trigger( buf, to, ch, obj1, vch );         

      }
    }  

    MOBtrigger=TRUE;

    return;
}


int colour( char type, CHAR_DATA *ch, char *string ) {
    char	code[ 20 ];
    char	*p = '\0';

    if( IS_NPC( ch ) )
	return( 0 );

    switch( type )
    {
	default:
	    sprintf( code, "{%c", type );
	    break;
	case 'x':
	    sprintf( code, CLEAR );
	    break;
	case 'b':
	    sprintf( code, C_BLUE );
	    break;
	case 'c':
	    sprintf( code, C_CYAN );
	    break;
	case 'd':
	    sprintf( code, FG_BLACK );
	    break;
	case 'g':
	    sprintf( code, C_GREEN );
	    break;
	case 'm':
	    sprintf( code, C_MAGENTA );
	    break;
	case 'r':
	    sprintf( code, C_RED );
	    break;
	case 'w':
	    sprintf( code, C_WHITE );
	    break;
	case 'y':
	    sprintf( code, C_YELLOW );
	    break;
	case 'B':
	    sprintf( code, C_B_BLUE );
	    break;
	case 'C':
	    sprintf( code, C_B_CYAN );
	    break;
	case 'G':
	    sprintf( code, C_B_GREEN );
	    break;
	case 'M':
	    sprintf( code, C_B_MAGENTA );
	    break;
	case 'R':
	    sprintf( code, C_B_RED );
	    break;
	case 'W':
	    sprintf( code, C_B_WHITE );
	    break;
	case 'Y':
	    sprintf( code, C_B_YELLOW );
	    break;
	case 'D':
	    sprintf( code, C_D_GREY );
	    break;
	case '*':
	    sprintf( code, "%c", 007 );
	    break;
	case '/':
	    sprintf( code, "\r\n  " );
	    break;
	case '{':
	    sprintf( code, "{" );
	    break;
	case '3':
	    sprintf( code, MOD_UNDERLINE );
	    break;
	case '4':
	    sprintf( code, MOD_REVERSE );
	    break;
	case '&':
	    sprintf( code, MOD_BLINK );
	    break;
        case 'F':
            code[0] = '\0';
            break;
    }

    p = code;
    while( *p != '\0' )  {
	*string = *p++;
	*++string = '\0';
    }

    return( strlen( code ) );
}


void colourconv( char *buffer, const char *txt , CHAR_DATA *ch ) {
const	char	*point;
int	skip = 0;

    if( ch->desc && txt ) {
	if( IS_SET( ch->plr, PLR_COLOUR ) ) {
	    for( point = txt ; *point ; point++ )   {
		if( *point == '{' ) {
		    point++;
		    skip = colour( *point, ch, buffer );
		    while( skip-- > 0 )
			++buffer;
		    continue;
		}
		*buffer = *point;
		*++buffer = '\0';
	    }			
	    *buffer = '\0';
	}
	else
	{
	    for( point = txt ; *point ; point++ )
	    {
		if( *point == '{' )
		{
		    point++;
		    continue;
		}
		*buffer = *point;
		*++buffer = '\0';
	    }
	    *buffer = '\0';
	}
    }
    return;
}

int advatoi (const char *s) {
  char string[MAX_INPUT_LENGTH]; /* a buffer to hold a copy of the argument */
  char *stringptr = string; /* a pointer to the buffer so we can move around */
  char tempstring[2];       /* a small temp buffer to pass to atoi*/
  int number = 0;           /* number to be returned */
  int multiplier = 0;       /* multiplier used to get the extra digits right */


  strcpy (string,s);        /* working copy */

  while ( isdigit (*stringptr)) /* as long as the current character is a digit */
  {
      strncpy (tempstring,stringptr,1);           /* copy first digit */
      number = (number * 10) + atoi (tempstring); /* add to current number */
      stringptr++;                                /* advance */
  }

  switch (UPPER(*stringptr)) {
      case 'K'  : multiplier = 1000;    number *= multiplier; stringptr++; break;
      case 'M'  : multiplier = 1000000; number *= multiplier; stringptr++; break;
      case '\0' : break;
      default   : return 0; /* not k nor m nor NUL - return 0! */
  }

  while ( isdigit (*stringptr) && (multiplier > 1)) /* if any digits follow k/m, add those too */
  {
      strncpy (tempstring,stringptr,1);           /* copy first digit */
      multiplier = multiplier / 10;  /* the further we get to right, the less are the digit 'worth' */
      number = number + (atoi (tempstring) * multiplier);
      stringptr++;
  }

  if (*stringptr != '\0' && !isdigit(*stringptr)) /* a non-digit character was found, other than NUL */
    return 0; /* If a digit is found, it means the multiplier is 1 - i.e. extra
                 digits that just have to be ignore, liked 14k4443 -> 3 is ignored */


  return (number);
}


int parsebet (const int currentbet, const char *argument) {
  int newbet = 0;                
  char string[MAX_INPUT_LENGTH]; 
  char *stringptr = string;      

  strcpy (string,argument); 

  if (*stringptr)  {
    if (isdigit (*stringptr)) {
         newbet = advatoi (stringptr); 
    } else {
         if (*stringptr == '+') {
               if (strlen (stringptr) == 1) newbet = (currentbet * 125) / 100;
               else newbet = (currentbet + atoi (++stringptr)); 
         }else {
               printf ("considering: * x \r\n");
               if ((*stringptr == '*') || (*stringptr == 'x')) {
                       if (strlen (stringptr) == 1) newbet = currentbet * 2 ; 
                       else newbet = currentbet * atoi (++stringptr);
               }
         }
    }
  }

  return newbet;        /* return the calculated bet */
}


void sprintf_to_world(char *fmt, ...) {
 char buf[2*MAX_STRING_LENGTH];
 va_list args;
 DESCRIPTOR_DATA *d;
 
 va_start(args, fmt);
 vsprintf(buf, fmt, args);
 va_end(args);
 
 for(d = descriptor_list; d; d = d->next) {
    if ( d->character == NULL ) continue;
    send_to_char(buf, d->character);
 }
 return;
}


void sprintf_to_room(ROOM_INDEX_DATA *rm, char *fmt, ...) {
 char buf[2*MAX_STRING_LENGTH];
 va_list args;
 CHAR_DATA *ch;
 
 va_start(args,fmt);
 vsprintf(buf,fmt,args);
 va_end(args);
 for( ch = rm->people; ch; ch=ch->next_in_room ) send_to_char(buf, ch);
 return;
}


void sprintf_to_char (CHAR_DATA *ch, char *fmt, ...) {
        char buf [MAX_STRING_LENGTH];
        va_list args;
        va_start (args, fmt);
        vsprintf (buf, fmt, args);
        va_end (args);
        
        send_to_char (buf, ch);
}


void sprintf_to_partner (int rn, char *fmt, ...) {
        char buf [MAX_STRING_LENGTH];
        va_list args;
        va_start (args, fmt);
        vsprintf (buf, fmt, args);
        va_end (args);

        if (partner_array[rn].status == PARTNER_CONNECTED) write_mud (partner_array[rn].fd, buf);
}


bool check_ban(char *site) {
BAN_DATA *pban;

    for ( pban = ban_list; pban != NULL; pban = pban->next ) {

        if (pban->type == BAN_NORMAL
        &&  !str_cmp(pban->name, site))  return TRUE;

        if (pban->type == BAN_PREFIX
        &&  !str_suffix(pban->name, site)) return TRUE;

        if (pban->type == BAN_SUFFIX
        &&  !str_prefix(pban->name, site)) return TRUE;
    }

    return FALSE;
}


char *strip_heading(char *kwd, int feed) {
char buf[MAX_STRING_LENGTH];
char *out;
int ipos = 0;
int opos = 0;

   while (opos < MAX_STRING_LENGTH && kwd[ipos] != '\0' ) {
         if (ipos > feed) buf[opos++] = kwd[ipos];
         ipos++;
   }
   buf[opos++] = '\0';

   out = str_dup(buf);
   return out;
}


bool chance(int num) {
    if (number_range(1,100) <= num) return TRUE;
    else return FALSE;
}


void copyover (bool crash) {
FILE *fp;
DESCRIPTOR_DATA *d, *d_next;
char buf [100], buf2[100];
	
	fp = fopen (COPYOVER_FILE, "w");
	
	if (!fp) {
		perror ("do_copyover:fopen");
		return;
	}
	
	if (crash) {
                    sprintf (buf, "*** C R A S H ***\r\nAttempting to recover.");
                    fprintf (fp, "C 1");
                } else {
                    sprintf (buf, "*** REFRESHING ***\r\nPrepare for 30 seconds of lag.");
                    fprintf (fp, "C 0");
	}

	for (d = descriptor_list; d ; d = d_next) {
		CHAR_DATA * och = CH (d);
		d_next = d->next;
		
		if (!d->character || d->connected > CON_PLAYING) {
			write_to_descriptor (d->descriptor, "\r\nSorry, we are rebooting. Come back in a minute.\r\n", 0);
			close_socket (d, TRUE);
		} else {
			fprintf (fp, "D %d %s %s\n", d->descriptor, och->name, d->host);
			save_char_obj(och);
			write_to_descriptor (d->descriptor, buf, 0);
		}
	}
	
	fprintf (fp, "End\n");
	fclose (fp);
	
	/* Close reserve and other always-open files and release other resources */
	
	fclose (fpReserve);
	
	/* exec - descriptors are inherited */
	
	sprintf (buf, "%d", port);
	sprintf (buf2, "%d", control);
	execl (EXE_FILE, "CthulhuMud", buf, "copyover", buf2, (char *) NULL);

	perror("do_copyover: execl");
	
}


void prepare_newbie(CHAR_DATA *ch) {
int base = 0;
int sn = 0; 
int ps, i;

    REMOVE_BIT(ch->act, ACT_IS_NPC);
    ch->notify		= NOTIFY_WEATHER;
    ch->pcdata->divinity	= DIV_NEWBIE;

    ch->affected_by 	= ch->affected_by|race_array[ch->race].aff;
    ch->act	                = ch->act|race_array[ch->race].act;
    ch->nature		= ch->nature|race_array[ch->race].nature;
    ch->imm_flags		= ch->imm_flags|race_array[ch->race].imm;
    ch->res_flags		= ch->res_flags|race_array[ch->race].res;
    ch->vuln_flags		= ch->vuln_flags|race_array[ch->race].vuln;
    ch->form		= race_array[ch->race].form;
    ch->parts		= race_array[ch->race].parts;
    ch->size		= race_array[ch->race].size;
    ch->pcdata->lifespan     = (race_array[ch->race].lifespan + number_percent())/2;

    ch->pc_class = 1;

    if (ch->profs == NULL) {
          PROF_DATA *prof = NULL;
          bug("Newbie without profession!", 0); 

	      switch (ch->pc_class) {
	        case CLASS_MAGE:
	          prof = get_profession(PROF_MAGE);
	        break;
	        case CLASS_GOOD_PRIEST:
	          prof = get_profession(PROF_GOOD_PRIEST);
	          break;
	        case CLASS_THIEF:
	          prof = get_profession(PROF_THIEF);
	          break;
	        case CLASS_WARRIOR:
	          prof = get_profession(PROF_WARRIOR);
	          break;
	        case CLASS_CHAOSMAGE:
	          prof = get_profession(PROF_CHAOSMAGE);
	          break;
	        case CLASS_MONK:
	          prof = get_profession(PROF_MONK);
	          break;
	        case CLASS_EVIL_PRIEST:
	          prof = get_profession(PROF_EVIL_PRIEST);
	          break;
	        default:
	          break;
	      }
	
	      if (prof == NULL) {
   	           bug("Unable to find profession for class %d!", ch->pc_class);
	           exit(1);
	      } 
	
	     /* Create an LPROF for the player... */
	
	      ch->profs = get_lprof();
	
	      if (ch->profs == NULL) {
  	          bug("Allocate for new lprof failed!", 0); 
	          exit(1);
	      }
	
                      ch->profs->profession = prof; 
	      ch->profs->level = ch->level;
	      ch->profs->next = NULL;
    } 

    if (ch->profs == NULL) {
         ps = STAT_STR; 
            
         for(i = 0; i < MAX_STATS; i++) {
	ch->perm_stat[i] = dice(1,4);
                    
                if (i == ps) {
                    ch->perm_stat[i] += 52 + 4 * dice(2,4);
                } else {
                    if (i == STAT_INT || i == STAT_WIS) {
                          ch->perm_stat[i] += 32 + 4 * dice(3,4);
                    } else {  
 	          ch->perm_stat[i] += 16 + 4 * dice(4,4);
                    }
                } 
         }

    } else {
         for(i = 0; i < MAX_STATS; i++) {
              ch->perm_stat[i] = 10 + dice(4,4);
              ch->perm_stat[i] += 4 * dice(ch->profs->profession->stats[i], 8);
              ch->perm_stat[i] += 6 * (ch->profs->profession->stats[i]-1);
              if (ch->perm_stat[i] > 65) ch->perm_stat[i] = (ch->perm_stat[i] - 65)/2 + 65;
              if (ch->perm_stat[i] < 35) ch->perm_stat[i] = 35- ((35 - ch->perm_stat[i] )/2);
         }
    }

    adjust_stats_by_nature(ch);

    ch->sanity = ch->perm_stat[STAT_WIS] + dice(1,10); 
	
   /* Ensure minimum scores in base skills... */
	
     for(sn = 0; sn < MAX_SKILL; sn++) {
           if (!skill_array[sn].loaded) break;
	
           if (ch->profs->profession->levels->skill[sn] == 0) {
	base = get_base_skill(sn, ch) + ch->level;
	if (ch->effective->skill[sn] < base) setSkill(ch->effective, sn, base); 
           }
     }
     return;
}


void init_signals() {
   signal(SIGBUS,sig_handler);
   signal(SIGILL,sig_handler);
   signal(SIGSEGV,sig_handler);
   return;
}


void sig_handler(int sig) {

     signal(sig, SIG_DFL);
     if (fork() == 0) return;

     switch(sig) {
        case SIGBUS:
          bug("Sig handler SIGBUS.",0);
          break;
        case SIGILL:
          bug("Sig handler SIGILL.",0);
          break;
        case SIGSEGV:
          bug("Sig handler SIGSEGV",0);
          break;
     }

     copyover(TRUE);
     close (control);
     exit(1);
}

void clear_screen(DESCRIPTOR_DATA *d) {
CHAR_DATA *ch = d->character;

        if (IS_SET(ch->plr, PLR_IMP_HTML)) write_to_buffer( d, IMP_CLS, 0 );
        else write_to_buffer( d, VT_CLS, 0 );

        return;
}

void prompt_menu(DESCRIPTOR_DATA *d) {
CHAR_DATA *ch = d->character;

        clear_screen(d);
        send_to_char("*******************************\r\n", ch);
        send_to_char("*                             *\r\n", ch);
        send_to_char("*   P ... Play the Game       *\r\n", ch);
        send_to_char("*   C ... Chat for a while    *\r\n", ch);
        send_to_char("*                             *\r\n", ch);
        send_to_char("*******************************\r\n", ch);
        return;
}

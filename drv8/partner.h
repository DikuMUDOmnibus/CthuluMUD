/*
 * CthulhuMud
 */

typedef struct  partner_type  PARTNER_TYPE;

struct partner_type {
    char 		*name;
    char		*ip;
    char		*title;
    char		*login;
    char		*passwd;
    int        	port;
    int        	fd;
    int        	last;
    VNUM        	start;
    int        	dominance;
    int        	status;
    bool     	loaded;
};

#define MAX_PARTNER		9
#define INTER_BOARD		"2"

extern PARTNER_TYPE partner_array[MAX_PARTNER];
#define PARTNER_UNDEFINED -1

#define PARTNER_DOWN 	0
#define PARTNER_CONNECTED	1

struct who_slot {
	CHAR_DATA *ch;
	struct who_slot *next;
};


void 		load_partners			(void);
void 		list_partners			(CHAR_DATA *ch);
int 		get_partner_rn			(char *name);
int		am_i_partner			(CHAR_DATA *ch);
int    		connect_mud 			(const char *hostname, int port);
void 		disconnect_mud 			(int fd);
void 		write_mud 			(int fd, const char *fmt, ...);
void 		connect_partners			(void);
void 		connect_single_partner		(int rn);
void 		char_div				(CHAR_DATA *ch, char *buf);
void 		sprintf_to_partner 		(int rn, char *fmt, ...);
char        		*prepare_for_transfer		(char * in);
CHAR_DATA 	*get_gate_partner		(ROOM_INDEX_DATA *room);
VNUM 		get_gate_dest			(ROOM_INDEX_DATA *room);
void 		transmove_player		(CHAR_DATA *ch, CHAR_DATA *partner, VNUM dest);
void 		set_index			(CHAR_DATA *ch, OBJ_DATA *obj);
bool 		reindex_obj			(OBJ_DATA *obj);
void 		lock_away_incompatible		(CHAR_DATA *ch, OBJ_DATA *obj);
void 		load_trans			(CHAR_DATA *ch);





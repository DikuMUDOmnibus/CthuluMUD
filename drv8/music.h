/*
* CthulhuMud
*/

#define MAX_SONGS		20
#define MAX_MSTYLES		16
#define MAX_INSTRUMENTS	8

#define MAX_LINES	100 /* this boils down to about 1k per song */
#define MAX_GLOBAL	10  /* max songs the global jukebox can hold */
#define PULSE_MUSIC	( 6 * PULSE_PER_SECOND)
#define MUSIC_FILE	"../config/music.txt"
#define MSTYLE_FILE	"../config/musicstyle.txt"

#define ACV_MUSIC_SING	1
#define ACV_MUSIC_PLAY	2


struct song_data {
    char *group;
    char *name;
    char *lyrics[MAX_LINES];
    int lines;
};

extern struct song_data song_table[MAX_SONGS];

struct mstyle_data {
    char	*name;
    char 	*title;
    bool 	instr[MAX_INSTRUMENTS];
    int 	diff;
    int	volume;
    bool 	loaded;
};

extern struct mstyle_data music_styles[MAX_MSTYLES];

void 		song_update 		(void);
void 		load_songs		(void);
void 		load_mstyles		(void);
int 		calculate_music_effect	(CHAR_DATA *ch, OBJ_DATA *instrument);
int 		instrument_skill		(OBJ_DATA *instrument);
int 		get_room_music		(ROOM_INDEX_DATA *room, int style);
int 		music_number		(char *name);
void 		evaluate_eff		(CHAR_DATA *ch, int eff);



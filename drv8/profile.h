/*
 * CthulhuMud
 */

#define NUM_ZONES	32

/* Zone definition... */

struct zone_data {
	int		 zid;
	int		 flags;
	char		*name;
	VNUM		 dream;
	VNUM		 recall;
	VNUM		 respawn;
	VNUM		 morgue;
	long		 mask;
	char		*map;
	bool		 loaded;
};

struct mud_data {
	char		*name;
	char		*url;
	int		port;
	long		flags;
	VNUM		recall;
	VNUM		respawn;
	VNUM		morgue;
	VNUM		start;		 
	VNUM		dream;		 
	VNUM             	mare;
      	VNUM             	monitor;
      	VNUM             	home;
        	VNUM   		pattern[2];
        	VNUM		building[2];
                short 		diff;
                short		pk_timer;
 	short		approval_timer;
                short		player_count;
	char		*map;
                int		worth[MAX_CURRENCY];
 	short		reboot;
};

#define DIFF_EASY     		1
#define DIFF_BALANCED	0
#define DIFF_HARD		2

#define ZONE_SECRET	0x0001
#define ZONE_NOMAGIC	0x0002
#define ZONE_LOWMAGIC	0x0004
#define ZONE_HIGHMAGIC	0x0008
#define ZONE_SUPERMAGIC	0x0010

#define MUD_WIZLOCK		                (A)
#define MUD_NEWLOCK		                (B)
#define MUD_PERMAPK		                (C)
#define MUD_PERMADEATH		(D)
#define MUD_EFFXPS		                (E)
#define MUD_AUTOOLC		                (F)
#define MUD_CRON		                (G)
#define MUD_RECOVER	                	(H)

extern ZONE_DATA zones[];

extern MUD_DATA mud;

bool room_ok_for_recall(int vnum);

void load_profile();


/*
 * CthulhuMud
 */

typedef struct  race_type  RACE_TYPE;

struct race_type {
    char 	*name;			/* call name of the race */
    char	*track_name;		/* used for tracking */
    long long	 aff;			/* aff bits for the race */
    long long	 act;			/* act bits for the race */
    long	 off;			/* off bits for the race */
    long	 imm;			/* imm bits for the race */
    long	 envimm;			/* imm bits for the race */
    long     res;			/* res bits for the race */
    long	 vuln;			/* vuln bits for the race */
    long	 form;			/* default form flag for the race */
    long	 parts;			/* default parts for the race */
    long	 nature;		/* Default mob nature */
    int	 size;			/* Size of the race */
    int 	 number;		/* Old race number */
    int 	 society;		/* Society */
    int	 language;		/* Racial language */
    short  lifespan;
    short  material;
    char     *cult;
    bool	 loaded;		/* Flag if it is loaded */
    bool	 pc_race;		/* Available for PCs */
};


extern RACE_TYPE race_array[MAX_RACE];

void load_races();

int get_race_rn(char *name);

bool valid_race(int rn);

#define RACE_UNDEFINED -1

void prompt_race(DESCRIPTOR_DATA *d);

void prompt_race_info(DESCRIPTOR_DATA *d, char *args);



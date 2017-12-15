/*
 * CthulhuMud
 */

typedef struct  cult_type  CULT_TYPE;

struct cult_type {
    char 	*name;
    char	*desc;
    int        number;
    int        alignment;
    long     power;
    bool     loaded;
    ECOND_DATA    *ec;
};


extern CULT_TYPE cult_array[MAX_CULT];
#define CULT_UNDEFINED -1

void load_cults(void);
void save_cults(void);
void list_cult(CHAR_DATA *ch);
int get_cult_rn(char *name);
void info_cult(CHAR_DATA *ch, char* argument);
void sac_align(CHAR_DATA *ch, int points);
char *get_align(int align);




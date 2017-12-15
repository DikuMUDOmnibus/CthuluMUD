/*
 * CthulhuMud
 */

typedef struct help_index HELP_INDEX;

struct	help_index { 
   HELP_INDEX	*next;
   char		*title;
   short		type;
   char		*keywords;
   char		*file;
   ECOND_DATA	*ec;
};

HELP_INDEX 	*new_hindex();
void 		free_hindex( HELP_INDEX *pHelp );
void 		load_hindex();
void 		read_hindex(char *filename);
char		*html_dup(char *kwd);


#define HELP_UNKNOWN 	0
#define HELP_BASICS 		1
#define HELP_CONCEPTS		2
#define HELP_COMMANDS 	3
#define HELP_SKILLS 		4
#define HELP_SPELLS 		5

extern          int                     top_hindex;






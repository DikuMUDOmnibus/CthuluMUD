/*
 * CthulhuMud 
 */

/* Get and free MCBs... */

MOB_CMD_BUF *get_mcb();

/* Free an MCB... */

void free_mcb(MOB_CMD_BUF *mcb);

/* Enqueue an action to a mob... */

void enqueue_mob_cmd(CHAR_DATA *ch, char *cmd, int delay, int cmd_id);

/* Purge unwanted commands... */

void purge_mcbs(CHAR_DATA *ch, int cmd_id);

/* Release all MCBs... */

void release_mcbs(CHAR_DATA *ch);

/* Execute an MCB... */

void execute_mcb(CHAR_DATA *ch);

/* Get Scripts... */

MOB_SCRIPT *get_mscr();

/* Free Scripts... */

void free_mscr(MOB_SCRIPT *mscr);

/* Release all scripts... */

void release_scripts(MOB_SCRIPT *base);

/* Enqueue a script to a mob... */

void enqueue_script(	CHAR_DATA	*mob, 
			MOB_SCRIPT 	*mscr,
			MOB_CMD_CONTEXT *mcc);

/* Locate a specific script... */

MOB_SCRIPT *find_script(CHAR_DATA *mob, int id);

/* Locate a specific script... */

MOB_SCRIPT *find_script2(MOB_SCRIPT *scr, int id);

/* Save a mobs scripts... */ 

void save_script(FILE *fp, MOB_SCRIPT *scr);

/* Get Script lines... */

MOB_SCRIPT_LINE *get_mscrl();

/* Free Script lines... */

void free_mscrl(MOB_SCRIPT_LINE *mscrl);

/* Release all script lines... */

void release_script_lines(MOB_SCRIPT *mscr);

/* Enqueue a script line to a mob... */

void inline enqueue_script_line(CHAR_DATA 	*mob, 
				MOB_SCRIPT_LINE *scrl,
				MOB_CMD_CONTEXT *mcc);

/* Locate a specific script line... */

MOB_SCRIPT_LINE *find_script_line(MOB_SCRIPT *mscr, int seq);

/* Save a scripts lines... */ 

void save_script_line(FILE *fp, MOB_SCRIPT_LINE *scr);

/* Insert a line into a script... */

void insert_script_line(MOB_SCRIPT *scr, MOB_SCRIPT_LINE *scrl);

/* Set up a mob command context */

MOB_CMD_CONTEXT *get_mcc(	CHAR_DATA 	*mob, 
				CHAR_DATA 	*actor, 
				CHAR_DATA 	*victim, 
                                CHAR_DATA	*random,
				OBJ_DATA 	*obj,
				OBJ_DATA 	*obj2,
			 	int 		 number, 
				char 		*text);

/* Free a used moc command context... */

void free_mcc(MOB_CMD_CONTEXT *mcc);

/* Pick a random character... */

CHAR_DATA *random_char(CHAR_DATA *ch);

/* Resolve substitutions in a command... */

void expand_dollar_code(char *cmd, MOB_CMD_CONTEXT *mcc, char *rescmd);

void expand_by_context(char *cmd, MOB_CMD_CONTEXT *mcc, char *rescmd);




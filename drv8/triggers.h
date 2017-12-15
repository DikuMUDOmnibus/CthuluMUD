/*
 * CthulhuMud
 */

/* Get a new trigger */

MOB_TRIGGER *get_trigger();

/* Free a trigger... */

void free_trigger(MOB_TRIGGER *trigger);

/* Free all triggers a mob has... */

void release_triggers(CHAR_DATA *ch);

/* Add a trigger to a structure... */

MOB_TRIGGER *add_trigger(MOB_TRIGGER *base, MOB_TRIGGER *new_trigger);

/* Remove and delete a trigger from a structure... */

MOB_TRIGGER *del_trigger(MOB_TRIGGER *base, MOB_TRIGGER *junk);

/* Find a trigger chain within the structure... */

MOB_TRIGGER *find_trigger_chain(MOB_TRIGGER *base, int type);

/* Find a trigger within a chain by sequence number... */

MOB_TRIGGER *find_trigger(MOB_TRIGGER *chain, int seq);

/* Renumber all of the triggers in a structure... */

MOB_TRIGGER *renum_triggers(MOB_TRIGGER *base);

/* Process an event against the structure... */

bool check_trigger(MOB_TRIGGER *base, WEV *wev);

/* Write out a trigger structure... */

void write_triggers(MOB_TRIGGER *base, FILE *fp, char *hdr);

/* Read a single trigger in... */

MOB_TRIGGER *read_trigger(FILE *fp);

/* Trigger sets */

MOB_TRIGGER_SET *get_trs();

void free_trs(MOB_TRIGGER_SET *old_trs);

MOB_TRIGGER_SET *mobs_trs(CHAR_DATA *mob);


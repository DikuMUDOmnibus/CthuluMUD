/*
 * CthulhuMud
 */


MONITOR_LIST *get_monitor_list();

void free_monitor_list(MONITOR_LIST *old_list);

MONITORING_LIST *get_monitoring_list();

void free_monitoring_list(MONITORING_LIST *old_list);

/* Start a mobs monitoring... */

void start_monitors(CHAR_DATA *mob);

/* Stop a mobs monitoring... */

void stop_monitors(CHAR_DATA *mob);

/* Save a monitor list... */

void write_monitors(MONITOR_LIST *list, FILE *fp, char *hdr);

/* Read a monitor... */

MONITOR_LIST *read_monitor(FILE *fp);

/* Insert a monitor list... */

MONITOR_LIST *insert_monitor(MONITOR_LIST *chain, MONITOR_LIST *new_monitor);

/* Delete a monitor list */

MONITOR_LIST *delete_monitor(MONITOR_LIST *chain, int id);


/*
 * CthulhuMud
 */

#define CONV_NONE	0

#define CONV_SUB_NONE	0

#define CONV_STATE_NONE	0

extern CONV_RECORD *get_conv_record();

extern void free_conv_record(CONV_RECORD *cr);

extern CONV_SUB_RECORD *get_conv_sub_record();

extern void free_conv_sub_record(CONV_SUB_RECORD *csr);

extern CONV_SUB_RECORD *find_csr(CHAR_DATA *ch, int conv_id, int sub_id);

extern void set_csr(CHAR_DATA *ch, int conv_id, int sub_id, int state);
 
extern CONV_DATA *get_conv_data();

extern void free_conv_data(CONV_DATA *cd);

extern CONV_SUB_DATA *get_conv_sub_data();

extern void free_conv_sub_data(CONV_SUB_DATA *csd);

extern CONV_SUB_TRANS_DATA *get_conv_sub_trans_data();

extern void free_conv_sub_trans_data(CONV_SUB_TRANS_DATA *cstd);

extern CONV_DATA *find_cd(MOB_INDEX_DATA *mob, int conv_id);
 
extern CONV_SUB_DATA *find_csd(CONV_DATA *cd, int sub_id);

extern CONV_SUB_TRANS_DATA *find_cstd(CONV_SUB_DATA *csd, int seq);

extern void insert_cstd(CONV_SUB_TRANS_DATA *cstd, CONV_SUB_DATA *csd);
 
extern void do_converse(CHAR_DATA *ch, char *argument);

extern void write_conv(FILE *fp, CONV_DATA *cd);

void chatperform(CHAR_DATA *ch, CHAR_DATA *victim, char* msg);



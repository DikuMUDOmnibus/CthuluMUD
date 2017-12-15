/*
 * CthulhuMud
 */

#define SOC_TYPE_SECRET		-1
#define SOC_TYPE_NONE		 0
#define SOC_TYPE_CLAN		 1
#define SOC_TYPE_RACE		 2
#define SOC_TYPE_POLITICAL	                 3
#define SOC_TYPE_CULT		                 4
#define SOC_TYPE_GUARD	                 5
#define SOC_TYPE_MAX		                 5

#define SOC_RANK_SUBSCRIBE		 -2
#define SOC_RANK_SUSPENDED		 -1
#define SOC_RANK_NONE		 0
#define SOC_RANK_MEMBER		 1
#define SOC_RANK_COUNCIL	                 2
#define SOC_RANK_LEADER		 3

#define SOC_AUTH_NONE		 0x0000
#define SOC_AUTH_INVITE		 0x0001
#define SOC_AUTH_EXPEL		 0x0002
#define SOC_AUTH_BANK		 0x0004
#define SOC_AUTH_AUTH		 0x0008
#define SOC_AUTH_PROMOTE	                 0x0010
#define SOC_AUTH_DEMOTE		 0x0020
#define SOC_AUTH_TEST		 0x0040
#define SOC_AUTH_FOE		                 0x0080
#define SOC_AUTH_PARDON		 0x0100
#define SOC_AUTH_TAX		 0x0200
#define SOC_AUTH_LAW		 0x0400

#define SOC_LEVEL_MEMBERSHIP 	 0
#define SOC_LEVEL_EXPEL		-1
#define SOC_LEVEL_INVITED	-1
#define SOC_LEVEL_FOE		-2

/* Management for socialites... */

SOCIALITE 	*get_socialite		(void);
void 		free_socialite		(SOCIALITE *old_socialite);
SOCADV 	*get_socadv		(void);
void 		free_socadv		(SOCADV *old_socadv);
SOCIETY 	*get_society		(void);
void 		free_society		(SOCIETY *old_society);
MEMBERSHIP 	*get_membership		(void);
void 		free_membership		(MEMBERSHIP *old_membership);
SOCIETY 	*find_society_by_id	(int id);
SOCIALITE 	*find_socialite		(SOCIETY *society, char *name);
MEMBERSHIP 	*find_membership	(CHAR_DATA *ch, SOCIETY *society);
void 		load_societies		(void);
void 		load_society_members	(void);
void 		save_char_society	(CHAR_DATA *ch, FILE *fp);
MEMBERSHIP 	*read_char_society	(FILE *fp);
bool 		is_member		(CHAR_DATA *ch, SOCIETY *society);
void 		create_membership	(CHAR_DATA *ch, SOCIETY *society, int rank);
void 		default_memberships	(CHAR_DATA *ch);
void 		check_memberships	(CHAR_DATA *ch);
void 		save_socialites		(void);
bool 		skill_available_by_society	(int sn, CHAR_DATA *ch);
bool 		is_friend			(CHAR_DATA *mob1, CHAR_DATA *mob2);
bool 		is_foe			(CHAR_DATA *mob1, CHAR_DATA *mob2);
void 		resolve_revolt		( SOCIETY *society);
void 		do_society_tax		(CHAR_DATA *ch, char *argument);
int 		soc_tax			(CHAR_DATA *ch, int gold, int currency);



/*
 * CthulhuMud
 */

/* Defaults... */

#define WEV_TYPE_NOT_FOUND		-1
#define WEV_SUBTYPE_NOT_FOUND	-1

#define WEV_TYPE_NONE		 0

#define WEV_SUBTYPE_NONE	 	0

#define WEV_SUBTYPE_MAX		14
#define WEV_SUBTYPE_MAX_PLUS	15

/* Gifts... */

#define WEV_GIVE			2

#define WEV_GIVE_GOLD		1
#define WEV_GIVE_ITEM			2

/* Getting... */

#define WEV_GET			3

#define WEV_GET_GOLD			1
#define WEV_GET_ITEM			2

/* Putting... */

#define WEV_PUT			4

#define WEV_PUT_ITEM			1

/* Dropping... */		

#define WEV_DROP			5

#define WEV_DROP_GOLD		1
#define WEV_DROP_ITEM		2		
#define WEV_DROP_PLANT		3		
#define WEV_DROP_ITEM_MELT		4		

/* Envenom... */

#define WEV_POISON			6

#define WEV_POISON_FOOD		1
#define WEV_POISON_WEAPON		2
#define WEV_POISON_FOUNTAIN	3
#define WEV_POISON_OTHER		9

/* FILL... */

#define WEV_FILL			7

#define WEV_FILL_FOUNTAIN		1
#define WEV_FILL_LIGHT		2

/* DRINK... */

#define WEV_DRINK			8

#define WEV_DRINK_FOUNTAIN_OK	1
#define WEV_DRINK_FOUNTAIN_BAD	2
#define WEV_DRINK_ITEM_OK		3
#define WEV_DRINK_ITEM_BAD		4
#define WEV_DRINK_FEED_OK		5
#define WEV_DRINK_FEED_BAD		6

/* EAT... */

#define WEV_EAT			9

#define WEV_EAT_FOOD_OK		1
#define WEV_EAT_FOOD_BAD		2
#define WEV_EAT_PILL			3
#define WEV_EAT_HERB			4
#define WEV_EAT_CORPSE		5
#define WEV_EAT_FEED_OK		6
#define WEV_EAT_FEED_BAD		7

/* SACRIFICE... */

#define WEV_SAC			10

#define WEV_SAC_PC_CORPSE		1
#define WEV_SAC_CORPSE		2
#define WEV_SAC_TRASH		3
#define WEV_SAC_TREASURE		4
#define WEV_SAC_ITEM			9

/* Gadgets... */

#define WEV_GADGET			11

#define WEV_GADGET_OK		1
#define WEV_GADGET_BAD		2

/* Searching */

#define WEV_SEARCH			12

#define WEV_SEARCH_ITEM		1
#define WEV_SEARCH_ROOM		2

#define WEV_FIND_ITEM		5
#define WEV_FIND_MOB			6
#define WEV_FIND_DOOR		7
#define WEV_FIND_HOUND		8
#define WEV_CONCEAL			9

/* Movement... */

#define WEV_DEPART			13

#define WEV_DEPART_WALK		1
#define WEV_DEPART_SNEAK		2
#define WEV_DEPART_FLY		3
#define WEV_DEPART_SWIM		4
#define WEV_DEPART_SAIL		5
#define WEV_DEPART_PORTAL		6
#define WEV_DEPART_MAGIC		7
#define WEV_DEPART_CURRENT		8
#define WEV_DEPART_FLEE		9

/* Movement... */

#define WEV_ARRIVE			14

#define WEV_ARRIVE_WALK		1
#define WEV_ARRIVE_SNEAK		2
#define WEV_ARRIVE_FLY		3
#define WEV_ARRIVE_SWIM		4
#define WEV_ARRIVE_SAIL		5
#define WEV_ARRIVE_PORTAL		6
#define WEV_ARRIVE_MAGIC		7
#define WEV_ARRIVE_CURRENT		8
#define WEV_ARRIVE_FLEE		9

/* Pulses... */

#define WEV_PULSE			15

#define WEV_PULSE_1			1
#define WEV_PULSE_3			2
#define WEV_PULSE_4			3
#define WEV_PULSE_5			4
#define WEV_PULSE_10			5
#define WEV_PULSE_30			6
#define WEV_PULSE_AREA		7

/* Time... */

#define WEV_TIME			16

#define WEV_TIME_HOUR		1
#define WEV_TIME_DAY			2
#define WEV_TIME_SUNRISE		3
#define WEV_TIME_SUNSET		4
#define WEV_TIME_DAWN		5
#define WEV_TIME_DUSK		6

#define WEV_CONTROL			17

#define WEV_CONTROL_LOGIN		1
#define WEV_CONTROL_LOGOUT	2
#define WEV_CONTROL_LINKDEAD	3
#define WEV_CONTROL_RECONNECT	4

#define WEV_MOB			18

#define WEV_MOB_STOP		1
#define WEV_MOB_SELECT		2
#define WEV_MOB_ECHO		3

#define WEV_DEATH			19

#define WEV_DEATH_SLAIN		1
#define WEV_DEATH_STUN		2

#define WEV_ATTACK			20

#define WEV_ATTACK_KILL		1
#define WEV_ATTACK_KICK		2
#define WEV_ATTACK_TRIP		3
#define WEV_ATTACK_BASH		4
#define WEV_ATTACK_DIRT		5
#define WEV_ATTACK_BACKSTAB	6
#define WEV_ATTACK_DISARM		7
#define WEV_ATTACK_CIRCLE		8
#define WEV_ATTACK_ROTATE		9
#define WEV_ATTACK_MURDER		10
#define WEV_ATTACK_TAIL		11
#define WEV_ATTACK_CRUSH		12
#define WEV_ATTACK_DISTRACT	13
#define WEV_ATTACK_SPRINKLE	14

#define WEV_COMBAT			21

#define WEV_COMBAT_MISS		1
#define WEV_COMBAT_HIT		2
#define WEV_COMBAT_BLOCK		3
#define WEV_COMBAT_PARRY		4
#define WEV_COMBAT_DODGE		5
#define WEV_COMBAT_ABSORB		6
#define WEV_COMBAT_IMMUNE	7
#define WEV_COMBAT_FADE		8

#define WEV_DAMAGE			22

#define WEV_DAMAGE_INJURED	1
#define WEV_DAMAGE_HURT		2
#define WEV_DAMAGE_DIRT		3
#define WEV_DAMAGE_TRIP		4
#define WEV_DAMAGE_BASH		5
#define WEV_DAMAGE_ENV		6
#define WEV_DAMAGE_TAIL		7
#define WEV_DAMAGE_CRUSH		8

/* Communications... */

#define WEV_OOCC			23

#define WEV_OOCC_BEEP		1
#define WEV_OOCC_GOSSIP		2
#define WEV_OOCC_MUSIC		3
#define WEV_OOCC_IMMTALK		4
#define WEV_OOCC_QUESTION		5
#define WEV_OOCC_ANSWER		6
#define WEV_OOCC_TELL		7
#define WEV_OOCC_GTELL		8
#define WEV_OOCC_HERO		9
#define WEV_OOCC_SOCIAL		10
#define WEV_OOCC_STELL		11
#define WEV_OOCC_MTELL		12
#define WEV_OOCC_INVESTIGATOR	13
#define WEV_OOCC_CHAT		14

#define WEV_ICC			24

#define WEV_ICC_TELL			1
#define WEV_ICC_SAY			2
#define WEV_ICC_SHOUT		3
#define WEV_ICC_SCREAM		4
#define WEV_ICC_YELL			5
#define WEV_ICC_MTELL		6
#define WEV_ICC_TELEPATHY		7
#define WEV_ICC_PRAY			8

#define WEV_SOCIAL			25

#define WEV_SOCIAL_EMOTE		1
#define WEV_SOCIAL_FRIENDLY		2
#define WEV_SOCIAL_NEUTRAL		3
#define WEV_SOCIAL_HOSTILE		4
#define WEV_SOCIAL_SOCIETY		5

#define WEV_IDOL			26

#define WEV_IDOL_HELD		1
#define WEV_IDOL_PRAY		2
#define WEV_IDOL_OFFER_ITEM		3
#define WEV_IDOL_OFFER_GOLD	4

#define WEV_LOCK			27

#define WEV_LOCK_OPEN_DOOR	1
#define WEV_LOCK_OPEN_ITEM		2
#define WEV_LOCK_CLOSE_DOOR	3
#define WEV_LOCK_CLOSE_ITEM	4	
#define WEV_LOCK_LOCK_DOOR	5
#define WEV_LOCK_LOCK_ITEM		6
#define WEV_LOCK_UNLOCK_DOOR	7
#define WEV_LOCK_UNLOCK_ITEM	8
#define WEV_LOCK_PICK_DOOR		9
#define WEV_LOCK_PICK_ITEM		10

#define WEV_SOCIETY			28

#define WEV_SOCIETY_INVITE		1
#define WEV_SOCIETY_JOIN		2
#define WEV_SOCIETY_ADVANCE	3
#define WEV_SOCIETY_RESIGN		4
#define WEV_SOCIETY_EXPEL		5
#define WEV_SOCIETY_DEMOTE		6
#define WEV_SOCIETY_TEST		7
#define WEV_SOCIETY_FOE		8
#define WEV_SOCIETY_PARDON		9

#define WEV_DREAM			29

#define WEV_DREAM_WALK		1
#define WEV_DREAM_AWAKEN		2
#define WEV_DREAM_CAST		3
#define WEV_DREAM_PCAST		4
#define WEV_DREAM_SAY		5
#define WEV_DREAM_PSAY		6
#define WEV_DREAM_EMOTE		7
#define WEV_DREAM_PEMOTE		8

#define WEV_ACTIVITY			30

#define WEV_ACTIVITY_START		1
#define WEV_ACTIVITY_STOP		2
#define WEV_ACTIVITY_POSCHANGE	3

#define WEV_SPELL			31

#define WEV_SPELL_PREP		1
#define WEV_SPELL_START		2
#define WEV_SPELL_CHANT		3
#define WEV_SPELL_CAST		4
#define WEV_SPELL_RECOVER		5
#define WEV_SPELL_FAIL		6
#define WEV_SPELL_CONSUME		7
#define WEV_SPELL_ECHO		8
#define WEV_SPELL_POWER		9
#define WEV_SPELL_YELL		10

#define WEV_HEAL			32

#define WEV_HEAL_HITS		1
#define WEV_HEAL_MANA		2
#define WEV_HEAL_MOVE		3
#define WEV_HEAL_HEALER		4
#define WEV_HEAL_THERAPY		5

#define WEV_DEBATE			33

#define WEV_DEBATE_START		1
#define WEV_DEBATE_CONTINUE	2
#define WEV_DEBATE_FINISH		3

#define WEV_KNOCK			34

#define WEV_KNOCK_DOOR		1

#define WEV_INTERPRET		35

#define WEV_INTERPRET_STRANGE	1

#define WEV_OPROG			36

#define WEV_OPROG_USE		1
#define WEV_OPROG_WEAR		2
#define WEV_OPROG_REMOVE     	3
#define WEV_OPROG_EXPLOSION     	4
#define WEV_OPROG_PHOTO     		5
#define WEV_OPROG_DESTROY   		6
#define WEV_OPROG_REBUILD    		7
#define WEV_OPROG_EFFECT       		8
#define WEV_OPROG_COMBINE       	9

#define WEV_LEARN			37

#define WEV_LEARN_PRACTICE		1
#define WEV_LEARN_LEARN		2

#define WEV_DUEL			38

#define WEV_DUEL_START		1
#define WEV_DUEL_CONTINUE		2
#define WEV_DUEL_FINISH		3
#define WEV_DUEL_OFFENSIVE		4

#define WEV_TRAP			39

#define WEV_TRAP_LAY			1
#define WEV_TRAP_DISARM		2
#define WEV_TRAP_INFO		3
#define WEV_TRAP_TRIGGER		4

#define WEV_SHOW			40

#define WEV_SHOW_ITEM		1
#define WEV_SHOW_PASSPORT		2

#define WEV_MUSIC			41

#define WEV_MUSIC_SING		1
#define WEV_MUSIC_PLAY		2

#define WEV_PEEK			42

#define WEV_PEEK_ITEM		1

#define MAX_WEV 			43

/* See also ev_map in triggers.c */

/* Scopes of issuing WEVs by area... */

#define WEV_SCOPE_NONE		0 
#define WEV_SCOPE_ROOM		1
#define WEV_SCOPE_ADJACENT	 	2
#define WEV_SCOPE_SUBAREA	 	3
#define WEV_SCOPE_SUBAREA_PLUS	4
#define WEV_SCOPE_AREA		5
#define WEV_SCOPE_AREA_PLUS	6
#define WEV_SCOPE_ZONE		7
#define WEV_SCOPE_UNIVERSE	 	8
#define WEV_SCOPE_GROUP		9

#define WEV_SCOPE_BAD		-1

/* WEV management... */

WEV *get_wev(int type, int subtype, MOB_CMD_CONTEXT *context, char *msg_actor, char *msg_victim, char *msg_observer);

void free_wev(WEV *wev);

/* Wev distribution... */

void room_echo_wev(ROOM_INDEX_DATA *room, WEV *wev, char *source, bool dist_further);

void room_issue_wev(ROOM_INDEX_DATA *room, WEV *wev);

bool room_issue_wev_challange(ROOM_INDEX_DATA *room, WEV *wev);

void room_issue_door_wev(ROOM_INDEX_DATA *room, WEV *wev, char *msg);

void world_issue_wev(WEV *wev, char *source);

void idol_issue_wev(OBJ_DATA *idol, WEV *wev);

void ch_issue_idol_wev(CHAR_DATA *ch, WEV *wev);

void area_issue_wev(ROOM_INDEX_DATA *room, WEV *wev, int scope);

/* Wev processing... */

void mob_handle_wev(CHAR_DATA *ch, WEV *wev, char *source);
bool mob_handle_wev_challange(CHAR_DATA *ch, WEV *wev);
void mob_handle_chat(CHAR_DATA *mob, WEV *wev, char *source);
void photo_write_wev(OBJ_DATA *photo, WEV *wev);

/* Issue a wev to a specific mob... */

void issue_update_wev(CHAR_DATA *mob, int wev_type, int wev_sub_type, int value);

/* Issue a wev to interested mobs... */

void issue_time_wev(int sub_flag, int wev_type, int wev_sub_type, int value);

/* Triggers */

MOB_TRIGGER *get_trigger();

void free_trigger(MOB_TRIGGER *trigger);

void release_triggers(CHAR_DATA *ch);

MOB_TRIGGER *add_trigger(MOB_TRIGGER *base, MOB_TRIGGER *new_trigger);

MOB_TRIGGER *del_trigger(MOB_TRIGGER *base, MOB_TRIGGER *junk);

MOB_TRIGGER *find_trigger_chain(MOB_TRIGGER *base, int type);

MOB_TRIGGER *find_trigger(MOB_TRIGGER *chain, int seq);

MOB_TRIGGER *renum_triggers(MOB_TRIGGER *base);

bool check_trigger(MOB_TRIGGER *base, WEV *wev);


struct ev_mapping {
  char *name;
  char *events[WEV_SUBTYPE_MAX + 2];
};


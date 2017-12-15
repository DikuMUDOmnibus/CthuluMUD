/*********************************************************
 *                   CTHULHUMUD                       	 *
 * CthulhuMud  driver  version 8.x copyright (C) 2000 	 *
 * by Mik Clarke (mykael@vianet.net.au)                  *
 * and Joachim Häusler (mystery@chello.at).              *
 *                                                       *
 * While the code is original, many concepts and the     *
 * typical look & feel are derived from MERC and its     *
 * derivatives - especially the SunderMud 1.0 prototype. *
 *                                                       *
 * Therefore we'd like to thank:                         *
 * Lotherius                                             *
 * Russ Taylor, Gabrielle Taylor, Brian Moore            *
 * Michael Chastain, Michael Quan, and Mitchell Tse      *
 *                                                       *
 * Please keep this code open-source and share your      *
 * ideas. It's still a long way to go.                   *
 *********************************************************/
  

#include "everything.h"
#include "society.h"
#include "profile.h"


struct flag_stat_type {
    const struct flag_type *structure;
    bool stat;
};


/*****************************************************************************
 Name:		flag_stat_table
 Purpose:	This table catagorizes the tables following the lookup
 		functions below into stats and flags.  Flags can be toggled
 		but stats can only be assigned.  Update this table when a
 		new set of flags is installed.
 ****************************************************************************/
const struct flag_stat_type flag_stat_table[] =
{
/*  {	structure		stat	}, */
    {connect_type,		TRUE	},
    {zmask_flags,		FALSE	},
    {zone_flags,		FALSE	},
    {area_flags,		FALSE	},
    {sex_flags,		TRUE	},
    {exit_flags,		FALSE	},
    {door_resets,		TRUE	},
    {room_flags,		FALSE	},
    {sector_flags,		TRUE	},
    {sector_name, 		TRUE	},
    {type_flags,		TRUE	},
    {discipline_type,	FALSE	},
    {extra_flags,		FALSE	},
    {artifact_flags,		FALSE	},
    {wear_flags,		FALSE	},
    {imm_auth,		FALSE	},
    {act_flags,		FALSE	},
    {plr_flags,		FALSE	},
    {affect_flags,		FALSE	},
    {raffect_flags,		FALSE	},
    {apply_flags,		TRUE	},
    {wear_loc_flags,	TRUE	},
    {wear_loc_strings,	TRUE	},
    {weapon_flags,	TRUE	},
    {container_flags,	FALSE	},
    {liquid_flags,		TRUE	},
    {furniture_class,	FALSE	},
    {material_type,   	TRUE    },
    {material_string,       	TRUE    },
    {form_flags,           	FALSE   },
    {part_flags,            	FALSE   },
    {ac_type,              	TRUE    },
    {size_flags,           	TRUE    },
    {position_flags,       	TRUE    },
    {off_flags,            	FALSE   },
    {imm_flags,            	FALSE   },
    {immapp_flags,      	TRUE   },
    {res_flags,            	FALSE   },
    {vuln_flags,           	FALSE   },
    {weapon_class,         	TRUE    },
    {portal_class,          	TRUE    },
    {tree_size,         		TRUE    },
    {tree_type,          	TRUE    },
    {figurine_behavior,         TRUE    },
    {figurine_type,	         	TRUE    },
    {photo_state,        	TRUE    },
    {pool_type,        	TRUE    },
    {refill_type,        	TRUE    },
    {trap_class,          	TRUE    },
    {weapon_type,          	FALSE   },
    {attack_type,	 	TRUE    },	
    {damage_type,		TRUE	},
    {attack_types,	 	TRUE    },	
    {mob_nature,		FALSE	},
    {area_climate,		TRUE	},
    {limb_status,		TRUE	},
    {limb_name,		TRUE	},
    {currency_type,          	TRUE    },
    {currency_accept,          	TRUE    },
    {pass_type,          	TRUE    },
    {crime_type,          	TRUE    },
    {instrument_type,          	TRUE    },
    {   0,			0	}
};
    


/*****************************************************************************
 Name:		is_stat( table )
 Purpose:	Returns TRUE if the table is a stat table and FALSE if flag.
 Called by:	flag_value and flag_string.
 Note:		This function is local and used only in bit.c.
 ****************************************************************************/
bool is_stat( const struct flag_type *flag_table ) {
    int flag;

    for (flag = 0; flag_stat_table[flag].structure; flag++)
    {
	if ( flag_stat_table[flag].structure == flag_table
	  && flag_stat_table[flag].stat )
	    return TRUE;
    }
    return FALSE;
}



/*
 * This function is Russ Taylor's creation.  Thanks Russ!
 * All code copyright (C) Russ Taylor, permission to use and/or distribute
 * has NOT been granted.  Use only in this OLC package has been granted.
 */
/*****************************************************************************
 Name:		flag_lookup( flag, table )
 Purpose:	Returns the value of a single, settable flag from the table.
 Called by:	flag_value and flag_string.
 Note:		This function is local and used only in bit.c.
 ****************************************************************************/
long flag_lookup (const char *name, const struct flag_type *flag_table) {
    int flag;
 
    for (flag = 0; flag_table[flag].name[0] != '\0'; flag++)   {
        if ( !str_cmp( name, flag_table[flag].name )
          && flag_table[flag].settable )
            return flag_table[flag].bit;
    }
 
    return NO_FLAG;
}



/*****************************************************************************
 Name:		flag_value( table, flag )
 Purpose:	Returns the value of the flags entered.  Multi-flags accepted.
 Called by:	olc.c and olc_act.c.
 ****************************************************************************/
long flag_value( const struct flag_type *flag_table, char *argument) {
    char word[MAX_INPUT_LENGTH];
    long  bit;
    int  marked = 0;
    bool found = FALSE;

    if ( is_stat( flag_table ))    {
	if ( ( bit = flag_lookup( argument, flag_table ) ) != NO_FLAG )  return bit;
	else  return NO_FLAG;
    }

    /*
     * Accept multiple flags.
     */
    for (; ;)  {
        argument = one_argument( argument, word );

        if ( word[0] == '\0' )  break;

        if ( ( bit = flag_lookup( word, flag_table ) ) != NO_FLAG )  {
            SET_BIT( marked, bit );
            found = TRUE;
        }
    }

    if ( found ) return marked;
    else return NO_FLAG;
}

/*****************************************************************************
 Name:		flag_lookup( flag, table )
 Purpose:	Returns the value of a single, settable flag from the table.
 Called by:	flag_value and flag_string.
 Note:		This function is local and used only in bit.c.
 ****************************************************************************/
long long flag_lookup_long (const char *name, const struct flag_type *flag_table) {
    int flag;

    for (flag = 0; flag_table[flag].name[0] != '\0'; flag++)   {
        if (!str_cmp(name, flag_table[flag].name )
        && flag_table[flag].settable ) {
            return flag_table[flag].bit;
        }
    }

    return NO_FLAG;
}

/*****************************************************************************
 Name:		flag_value_long( table, flag )
 Purpose:	Returns the value of the flags entered.  Multi-flags accepted.
 Called by:	olc.c and olc_act.c.
 ****************************************************************************/
long long flag_value_long( const struct flag_type *flag_table, char *argument) {
    char word[MAX_INPUT_LENGTH];
    long long  bit;
    long long  marked = 0;
    bool found = FALSE;

    if ( is_stat( flag_table ))    {
	if ((bit = flag_lookup_long( argument, flag_table ) ) != NO_FLAG )    return bit;
	else  return NO_FLAG;
    }

    /*
     * Accept multiple flags.
     */
    for (; ;)  {
        argument = one_argument( argument, word );

        if ( word[0] == '\0' )  break;

        if ((bit = flag_lookup_long( word, flag_table)) != NO_FLAG )    {
            SET_BIT( marked, bit );
            found = TRUE;
        }
    }

    if ( found ) return marked;
    else return NO_FLAG;
}


/* Using a static buffer like this isn't a very cleaver idea at all. The 
 * problem is that each call to the function overwrites the result of the
 * previous call. This only shows up when, for instance, trying to pass
 * to results from the function to another function (like sprintf). Mik.
 */

/*****************************************************************************
 Name:		flag_string( table, flags/stat )
 Purpose:	Returns string with name(s) of the flags or stat entered.
 Called by:	act_olc.c, olc.c, and olc_save.c.
 ****************************************************************************/
char *flag_string( const struct flag_type *flag_table, long long bits ){
static char buf[MAX_STRING_LENGTH]; 
int  flag;

    buf[0] = '\0';

    for (flag = 0; flag_table[flag].name[0] != '\0'; flag++)    {
	if ( !is_stat( flag_table ) && IS_SET(bits, flag_table[flag].bit) )	{
	    strcat( buf, " " );
	    strcat( buf, flag_table[flag].name );
	} else if ( flag_table[flag].bit == bits ) {
	    strcat( buf, " " );
	    strcat( buf, flag_table[flag].name );
	    break;
	}
    }
    return (char*)( (buf[0] != '\0') ? buf+1 : "none" );
}


const struct flag_type connect_type[] =
{
    {"copyover",		CON_COPYOVER_RECOVER,		TRUE},
    {"playing",		CON_PLAYING,				TRUE},
    {"name",		CON_GET_NAME,			TRUE},
    {"old password",	CON_GET_OLD_PASSWORD,		TRUE},
    {"confirm name",	CON_CONFIRM_NEW_NAME,		TRUE},
    {"new password",	CON_GET_NEW_PASSWORD,		TRUE},
    {"confirm password",	CON_CONFIRM_NEW_PASSWORD,	TRUE},
    {"race",		CON_GET_NEW_RACE,			TRUE},
    {"sex",		CON_GET_NEW_SEX,			TRUE},
    {"class",		CON_GET_NEW_CLASS,			TRUE},
    {"alignment",		CON_GET_ALIGNMENT,			TRUE},
    {"change name",	CON_CHANGE_NAME,			TRUE},
    {"terminal",		CON_CHOOSE_TERM,			TRUE},
    {"amateur",		CON_GET_AMATEUR,			TRUE},
    {"IMOTD",		CON_READ_IMOTD,			TRUE},
    {"MOTD",		CON_READ_MOTD,			TRUE},
    {"break connect",	CON_BREAK_CONNECT,			TRUE},
    {"note to",		CON_NOTE_TO,				TRUE},
    {"note subject",	CON_NOTE_SUBJECT,			TRUE},
    {"note expire",		CON_NOTE_EXPIRE,			TRUE},
    {"note text",		CON_NOTE_TEXT,			TRUE},
    {"note finish",		CON_NOTE_FINISH,			TRUE},
    {"profession",		CON_GET_PROFESSION,			TRUE},
    {"waiting name",	CON_WAITING_NAME,			TRUE},
    {"waiting name loop",	CON_WAITING_NAME2,			TRUE},
    {"transfer",		CON_TRANSFER,			TRUE},
    {"check approval",	CON_CHECK_APPROVAL,		TRUE},
    {"note board",		CON_NOTE_BOARD,			TRUE},
    {"selling",		CON_CONFIRM_SELL,			TRUE},
    {"buying",		CON_CONFIRM_BUY,			TRUE},
    {"menu",		CON_MENU,				TRUE},
    {"chatting",		CON_CHATTING,			TRUE},
    {"",			0,				0	}
};


const struct flag_type zmask_flags[] =
{
    {	"all",		0x00000000,		FALSE	},
    {	"0",		0x00000001,		TRUE	},
    {	"1",		0x00000002,		TRUE	},
    {	"2",		0x00000004,		TRUE	},
    {	"3",		0x00000008,		TRUE	},
    {	"4",		0x00000010,		TRUE	},
    {          "5",               	0x00000020,		TRUE    },
    {          "6",               	0x00000040,		TRUE    },
    {          "7",               	0x00000080,		TRUE    },
    {	"8",		0x00000100,		TRUE	},
    {	"9",		0x00000200,		TRUE	},
    {	"10",		0x00000400,		TRUE	},
    {	"11",		0x00000800,		TRUE	},
    {	"12",		0x00001000,		TRUE	},
    {          "13",               	0x00002000,		TRUE    },
    {          "14",               	0x00004000,		TRUE    },
    {          "15",               	0x00008000,		TRUE    },
    {	"16",		0x00010000,		TRUE	},
    {	"17",		0x00020000,		TRUE	},
    {	"18",		0x00040000,		TRUE	},
    {	"19",		0x00080000,		TRUE	},
    {	"20",		0x00100000,		TRUE	},
    {          "21",               	0x00200000,		TRUE    },
    {          "22",               	0x00400000,		TRUE    },
    {          "23",               	0x00800000,		TRUE    },
    {	"24",		0x01000000,		TRUE	},
    {	"25",		0x02000000,		TRUE	},
    {	"26",		0x04000000,		TRUE	},
    {	"27",		0x08000000,		TRUE	},
    {	"28",		0x10000000,		TRUE	},
    {          "29",               	0x20000000,		TRUE    },
    {          "30",               	0x40000000,		TRUE    },
    {          "31",               	0x80000000,		TRUE    },
    {	"",			0,			0	}
};


const struct flag_type zone_flags[] =
{
    {"none",		0,				FALSE	},
    {"secret",		ZONE_SECRET,			TRUE	},
    {"no-magic",		ZONE_NOMAGIC,		TRUE	},
    {"low-magic",		ZONE_LOWMAGIC,		TRUE	},
    {"high-magic",		ZONE_HIGHMAGIC,		TRUE	},
    {"super-magic",	ZONE_SUPERMAGIC,		TRUE	},
    {"",			0,				0	}
};


const struct flag_type area_flags[] =
{
    {"none",		AREA_NONE,			FALSE	},
    {"changed",		AREA_CHANGED,		TRUE	},
    {"added",		AREA_ADDED,			TRUE	},
    {"loading",		AREA_LOADING,		FALSE	},
    {"locked",             	AREA_LOCKED,            		TRUE    },
    {"secret",              	AREA_SECRET,          		TRUE    },
    {"edlock",             	AREA_EDLOCK,           	 	TRUE    },
    {"no-magic",		AREA_NOMAGIC,		TRUE    },
    {"low-magic",		AREA_LOWMAGIC,		TRUE    },
    {"high-magic",		AREA_HIGHMAGIC,		TRUE    },
    {"super-magic",	AREA_SUPERMAGIC,		TRUE    },
    {"newbie",		AREA_NEWBIE,			TRUE    },
    {"law",		AREA_LAW,			TRUE    },
    {"solitary",		AREA_SOLITARY,		TRUE    },
    {	"",			0,			0	}
};


const struct flag_type sex_flags[] =
{
    {	"male",			SEX_MALE,		TRUE	},
    {	"female",		SEX_FEMALE,		TRUE	},
    {	"neutral",		SEX_NEUTRAL,		TRUE	},
    {          "random",                              SEX_RANDOM,                 TRUE    },  
    {	"none",			SEX_NEUTRAL,		TRUE	},
    {	"",			0,			0	}
};


const struct flag_type limb_status[] =
{
    {	"{gOK{x",		0,		TRUE	},
    {	"{yInjured{x",		1,		TRUE	},
    {	"{yInjured{x",		2,		TRUE	},
    {	"{yInjured{x",		3,		TRUE	},
    {	"{yInjured{x",		4,		TRUE	},
    {	"{mDisabled{x",		5,		TRUE	},
    {	"{mDisabled{x",		6,		TRUE	},
    {	"{mDisabled{x",		7,		TRUE	},
    {	"{mDisabled{x",		8,		TRUE	},
    {	"{mDisabled{x",		9,		TRUE	},
    {          "{rChopped off{x",             10,                            TRUE    },
    {	"",			-1,	          	 TRUE    }
};


const struct flag_type limb_name[] =
{
    {	"Head",			0,		TRUE	},
    {	"Torso",			1,		TRUE	},
    {	"Left_Arm",		2,		TRUE	},
    {	"Right_Arm",		3,		TRUE	},
    {	"Legs",			4,		TRUE	},
    {	"",			-1,	          	 TRUE    }
};

const struct flag_type exit_flags[] =
{
    {          "door",			EX_ISDOOR,		TRUE    },
    {	"closed",		EX_CLOSED,		TRUE	},
    {	"locked",		EX_LOCKED,		TRUE	},
    {	"pickproof",		EX_PICKPROOF,		TRUE	},
    {	"hidden",		EX_HIDDEN,		TRUE	},
    {	"no_pass",		EX_NO_PASS,		TRUE	},
    {	"echo_all",		EX_ECHO_ALL,		TRUE	},
    {	"echo_sound",		EX_ECHO_SOUND,	TRUE	},
    {	"echo_vision",		EX_ECHO_VISION,	TRUE	},
    {	"robust",		EX_ROBUST,		TRUE	},
    {	"armoured",		EX_ARMOURED,	TRUE	},
    {	"wall",		                EX_WALL,		TRUE	},
    {	"creaky",		EX_CREAKY,		TRUE	},
    {	"tricky",		                EX_TRICKY,		TRUE	},
    {	"random",		EX_RANDOM,		TRUE	},
    {	"",			0,			0	}
};



const struct flag_type door_resets[] =
{
    {	"open and unlocked",	0,		TRUE	},
    {	"closed and unlocked",	1,		TRUE	},
    {	"closed and locked",	2,		TRUE	},
    {	"",			0,		0	}
};

const struct flag_type room_flags[] =
{
    {	"dark",			ROOM_DARK,			TRUE},
    {	"dump",			ROOM_DUMP,			TRUE},
    {	"no_mob",		ROOM_NO_MOB,		TRUE},
    {	"inside",		                ROOM_INDOORS,		TRUE},
    {	"private",		ROOM_PRIVATE,		TRUE},
    {	"safe",			ROOM_SAFE,			TRUE},
    {	"solitary",		ROOM_SOLITARY,		TRUE},
    {	"pet_shop",		ROOM_PET_SHOP,		TRUE},
    {	"no_recall",		ROOM_NO_RECALL,		TRUE},
    {	"imp_only",		ROOM_IMP_ONLY,		TRUE},
    {	"gods_only",	                ROOM_GODS_ONLY,		TRUE},
    {	"heroes_only",		ROOM_HEROES_ONLY,		TRUE},
    {	"newbies_only",		ROOM_NEWBIES_ONLY,		TRUE},
    {	"law",			ROOM_LAW,			TRUE},
    {          "fastheal",		ROOM_FASTHEAL,          		TRUE},
    {          "rent",			ROOM_RENT,			TRUE},
    {	"auction",		ROOM_AUCTION_HALL,	TRUE},
    {	"bounty_office",		ROOM_BOUNTY_OFFICE,	TRUE},
    {          "pkill", 		                ROOM_PKILL,			TRUE},
    {          "misty",                                 ROOM_MISTY,   			TRUE},
    {          "echo",			ROOM_ECHO,             		TRUE},
    {          "building_pattern",	ROOM_BUILDING,            		TRUE},
    {          "vehicle",		ROOM_VEHICLE,            		TRUE},
    {          "gas",		                ROOM_GAS,            		TRUE},
    {          "dream_safe",		ROOM_DREAM_SAFE,            	TRUE},
    {	"no_flee",		ROOM_NO_FLEE,		TRUE},
    {	"tree",	                    	ROOM_TREE,			TRUE},
    {	"no_dream",	               	ROOM_NO_DREAM,		TRUE},
    {	"always_reset",	               	ROOM_ALWAYS_RESET,	TRUE},
    {	"nomagic",	               	ROOM_NOMAGIC,		TRUE},
    {	"mudgate",	               	ROOM_MUDGATE,		TRUE},
    {	"",			0,			0	}
};

const struct flag_type sector_name[] = /* this version used in move_char */
{
    {   "{winside{x",       		SECT_INSIDE,  			TRUE    },
    {   "{wcity{x",         		SECT_CITY,            		TRUE    },
    {   "{gfields{x",       		SECT_FIELD,          		TRUE    },
    {   "{gforest{x",       		SECT_FOREST,        		TRUE    },
    {   "{ghills{x",        		SECT_HILLS,           		TRUE    },
    {   "{mmountains{x",    		SECT_MOUNTAIN,       		TRUE    },
    {   "{bwater{x",        		SECT_WATER_SWIM,      	TRUE    },
    {   "{bocean{x",        		SECT_WATER_NOSWIM,    	TRUE    },
    {   "{csky{x",          		SECT_AIR,              		TRUE    },
    {   "{ydesert{x",       		SECT_DESERT,          		TRUE    },
    {   "{munderground{x",  	SECT_UNDERGROUND,    	TRUE    },
    {   "{gswamp{x",   		SECT_SWAMP,           		TRUE    },
    {   "{ymoors{x",        		SECT_MOORS,          		TRUE    },
    {   "{wspace{x",        		SECT_SPACE,           		TRUE    },
    {   "{bunderwater{x",   		SECT_UNDERWATER,      	TRUE    },
    {   "{rsmall flames{x", 		SECT_SMALL_FIRE,     		TRUE    },
    {   "{rflames{x",       		SECT_FIRE,           		TRUE    },
    {   "{Rbig flames{x",   		SECT_BIG_FIRE,        		TRUE    },
    {   "{wicy cold{x",     		SECT_COLD,          		TRUE    },
    {   "{gbubbling acid{x",		SECT_ACID,            		TRUE    },
    {   "{Clightning{x",    		SECT_LIGHTNING,      		TRUE    },
    {   "{Yholy ground{x",  		SECT_HOLY,           		TRUE    },
    {   "{Revil ground{x",  		SECT_EVIL,           		TRUE    },
    {   "{gjungle{x",	  	SECT_JUNGLE,          		TRUE    },
    {   "{ypath{x",	  		SECT_PATH,			TRUE    },
    {   "{yroad{x",	  		SECT_ROAD,			TRUE    },
    {   "{yplain{x",	  		SECT_PLAIN,			TRUE    },
    {   "",             0,                      0       }
};


const struct flag_type sector_flags[] = /* this version used in OLC */
{
    {   "inside",		SECT_INSIDE,		TRUE	},
    {   "city",		SECT_CITY,		TRUE	},
    {   "field",		SECT_FIELD,		TRUE	},
    {   "forest",		SECT_FOREST,		TRUE	},
    {   "hills",		SECT_HILLS,		TRUE	},
    {   "mountain",		SECT_MOUNTAIN,	TRUE	},
    {   "swim",		SECT_WATER_SWIM,	TRUE	},
    {   "noswim",		SECT_WATER_NOSWIM,TRUE	},
    {   "air",		SECT_AIR,		TRUE	},
    {   "desert",		SECT_DESERT,		TRUE	},
    {   "underground",	SECT_UNDERGROUND,	TRUE	},
    {   "swamp",		SECT_SWAMP,       	TRUE	},
    {   "moors",		SECT_MOORS,     	TRUE	},
    {   "space",		SECT_SPACE,      	TRUE	},
    {   "underwater",   	SECT_UNDERWATER,     TRUE    },
    {   "small-fire",   	SECT_SMALL_FIRE,        	TRUE    },
    {   "fire",         		SECT_FIRE,              	TRUE    },
    {   "big-fire",     		SECT_BIG_FIRE,          	TRUE    },
    {   "cold",         		SECT_COLD,              	TRUE    },
    {   "acid",         		SECT_ACID,              	TRUE    },
    {   "lightning",    	SECT_LIGHTNING,         	TRUE    },
    {   "holy",         		SECT_HOLY,              	TRUE    },
    {   "evil",         		SECT_EVIL,              	TRUE    },
    {   "jungle",		SECT_JUNGLE,		TRUE    },
    {   "path",		SECT_PATH,		TRUE    },
    {   "road",		SECT_ROAD,		TRUE    },
    {   "plain",		SECT_PLAIN,		TRUE    },
    {	"",		0,			0	}
};


const struct flag_type discipline_type[] =
{
    {"fire",		DISCIPLINE_FIRE,		TRUE	},
    {"water",		DISCIPLINE_WATER,		TRUE	},
    {"air",			DISCIPLINE_AIR,		TRUE	},
    {"earth",		DISCIPLINE_EARTH,		TRUE	},
    {"enchantment",	DISCIPLINE_ENCHANTMENT,	TRUE	},
    {"life",		DISCIPLINE_LIFE,		TRUE	},
    {"necromancy",	DISCIPLINE_NECROMANCY,	TRUE	},
    {"curses",		DISCIPLINE_CURSES,		TRUE	},
    {"eldermagic",		DISCIPLINE_ELDER_MAGIC,	TRUE	},
    {"demonology",	DISCIPLINE_DEMONOLOGY,	TRUE	},
    {"combat",		DISCIPLINE_COMBAT,		TRUE	},
    {"knowledge",		DISCIPLINE_KNOWLEDGE,	TRUE	},
    {"protection",		DISCIPLINE_PROTECTION,	TRUE	},
    {"lunar-power",	DISCIPLINE_LUNAR,		TRUE	},
    {"transdimensional",	DISCIPLINE_TRANSDIMENSIONAL,TRUE	},
    {"power",		DISCIPLINE_POWER,		TRUE	},
    {	"",			0,		0	}
};


const struct flag_type type_flags[] =
{
    {"light",		ITEM_LIGHT,		TRUE	},
    {"light_refill",		ITEM_LIGHT_REFILL,	TRUE	},
    {"scroll",		ITEM_SCROLL,		TRUE	},
    {"wand",		ITEM_WAND,		TRUE	},
    {"staff",		ITEM_STAFF,		TRUE	},
    {"weapon",		ITEM_WEAPON,	TRUE	},
    {"treasure",		ITEM_TREASURE,	TRUE	},
    {"armor",		ITEM_ARMOR,		TRUE	},
    {"potion",		ITEM_POTION,		TRUE	},
    {"clothing",		ITEM_CLOTHING,	TRUE	},
    {"furniture",		ITEM_FURNITURE,	TRUE	},
    {"trash",		ITEM_TRASH,		TRUE	},
    {"container",		ITEM_CONTAINER,	TRUE	},
    {"drink",		ITEM_DRINK_CON,	TRUE	},
    {"key",		ITEM_KEY,		TRUE	},
    {"food",		ITEM_FOOD,		TRUE	},
    {"money",		ITEM_MONEY,		TRUE	},
    {"boat",		ITEM_BOAT,		TRUE	},
    {"npc_corpse",	ITEM_CORPSE_NPC,	TRUE	},
    {"pc_corpse",		ITEM_CORPSE_PC,	FALSE	},
    {"fountain",		ITEM_FOUNTAIN,	TRUE	},
    {"pill",		ITEM_PILL,		TRUE	},
    {"protect",		ITEM_PROTECT,	TRUE	},
    {"map",		ITEM_MAP,		TRUE	},
    {"pride",               	ITEM_PRIDE,		TRUE    },
    {"component", 	ITEM_COMPONENT,	TRUE    },
    {"portal", 	 	ITEM_PORTAL,		TRUE    },
    {"locker",          		ITEM_LOCKER,            	TRUE    },
    {"locker_key",         	ITEM_LOCKER_KEY,        TRUE    },
    {"clan_locker",        	ITEM_CLAN_LOCKER,     TRUE    }, 
    {"keyring",		ITEM_KEY_RING,	TRUE	},
    {"book",		ITEM_BOOK,		TRUE	},
    {"idol",		ITEM_IDOL,		TRUE	},
    {"scry",		ITEM_SCRY,		TRUE	},
    {"dig",		ITEM_DIGGING,		TRUE	},
    {"forge",		ITEM_FORGING,		TRUE	},
    {"raw_material",	ITEM_RAW,		TRUE	},
    {"bondage_obj",	ITEM_BONDAGE,	TRUE	},
    {"tattoo",		ITEM_TATTOO,		TRUE	},
    {"trap",		ITEM_TRAP,		TRUE	},
    {"voodoo_doll",	ITEM_DOLL,		TRUE	},
    {"paper",		ITEM_PAPER,		TRUE	},
    {"explosive",		ITEM_EXPLOSIVE,	TRUE	},
    {"ammo",		ITEM_AMMO,		TRUE	},
    {"slotmachine",        	ITEM_SLOTMACHINE,	TRUE	},
    {"herb",              	ITEM_HERB,		TRUE	},
    {"pipe",               	ITEM_PIPE,		TRUE	},
    {"time_bomb",           	ITEM_TIMEBOMB,	TRUE	},
    {"tree",		ITEM_TREE,		TRUE	},
    {"camera",		ITEM_CAMERA,	TRUE	},
    {"photograph",	ITEM_PHOTOGRAPH,	TRUE	},
    {"warp_stone",	ITEM_WARP_STONE,	TRUE	},
    {"room_key",		ITEM_ROOM_KEY,	TRUE	},
    {"gem",		ITEM_GEM,		TRUE	},
    {"jewelry",		ITEM_JEWELRY,	TRUE	},
    {"jukebox",		ITEM_JUKEBOX,	TRUE	},
    {"share",		ITEM_SHARE,		TRUE	},
    {"figurine",		ITEM_FIGURINE,	TRUE	},
    {"pool",		ITEM_POOL,		TRUE	},
    {"grimoire",		ITEM_GRIMOIRE,	TRUE	},
    {"focus",		ITEM_FOCUS,		TRUE	},
    {"protocol",		ITEM_PROTOCOL,	TRUE	},
    {"decoration",		ITEM_DECORATION,	TRUE	},
    {"passport",		ITEM_PASSPORT,	TRUE	},
    {"instrument",		ITEM_INSTRUMENT,	TRUE	},
    {	"",			0,		0	}
};


const struct flag_type extra_flags[] =
{
    {"glow",			ITEM_GLOW,			TRUE	},
    {"hum",			ITEM_HUM,			TRUE	},
    {"dark",			ITEM_DARK,			TRUE	},
    {"neutral",			ITEM_NEUTRAL,		TRUE	},
    {"evil",			ITEM_EVIL,			TRUE	},
    {"invis",			ITEM_INVIS,			TRUE	},
    {"magic",			ITEM_MAGIC,			TRUE	},
    {"nodrop",			ITEM_NODROP,			TRUE	},
    {"bless",			ITEM_BLESS,			TRUE	},
    {"anti-good",			ITEM_ANTI_GOOD,		TRUE	},
    {"anti-evil",			ITEM_ANTI_EVIL,		TRUE	},
    {"anti-neutral",			ITEM_ANTI_NEUTRAL,		TRUE	},
    {"noremove",			ITEM_NOREMOVE,		TRUE	},
    {"inventory",			ITEM_INVENTORY,		TRUE	},
    {"nopurge",			ITEM_NOPURGE,		TRUE	},
    {"rot-after-death",		ITEM_ROT_DEATH,		TRUE	},
    {"vis-after-death",		ITEM_VIS_DEATH,		TRUE	},
    {"no_sac",			ITEM_NO_SAC,			TRUE	},
    {"concealed",			ITEM_CONCEALED,		TRUE	},
    {"no_cond",			ITEM_NO_COND,		TRUE	},
    {"scenic",			ITEM_SCENIC,			TRUE	},
    {"nodisarm",			ITEM_NODISARM,		TRUE	},
    {"usable",			ITEM_USABLE,			TRUE	},
    {"usable_infinite",		ITEM_USABLE_INF,		TRUE	},
    {"hypertech",			ITEM_HYPER_TECH,		TRUE	},
    {"animated",			ITEM_ANIMATED,		TRUE	},
    {"melt-drop",			ITEM_MELT_DROP,		TRUE	},
    {"no-locate",			ITEM_NOLOCATE,		TRUE	},
    {"sell-extract",			ITEM_SELL_EXTRACT,		TRUE	},
    {"no-uncurse",			ITEM_NOUNCURSE,		TRUE	},
    {"burn-proof",			ITEM_BURN_PROOF,		TRUE	},
    {"artifact",	                    	ITEM_ARTIFACT,		TRUE	},
    {"vanish",	                    	ITEM_VANISH,			TRUE	},
    {	"",			0,			0	}
};


const struct flag_type artifact_flags[] =
{
{"necronomicon",		ART_NECRONOMICON,		TRUE	},
{"mist",				ART_MIST,			TRUE	},
{"ring_eibon",		               	ART_EIBON,			TRUE	},
{"blessing_nephran",		ART_BLESSING_NEPHRAN,	TRUE	},
{"power_abhoth",		ART_ABHOTH,			TRUE	},
{	"",			0,				0	}
};

const struct flag_type plr_flags[] = {
    {"is_npc",			PLR_IS_NPC,			TRUE},
    {"owner",			PLR_BOUGHT_PET, 		TRUE},
    {"autoassist",			PLR_AUTOASSIST,		TRUE},
    {"autoexit", 			PLR_AUTOEXIT,		TRUE},
    {"autoloot",			PLR_AUTOLOOT,		TRUE},
    {"autosac", 			PLR_AUTOSAC,  		TRUE},
    {"autogold", 			PLR_AUTOGOLD,		TRUE},
    {"autosplit", 			PLR_AUTOSPLIT,		TRUE},
    {"player_killer", 		PLR_PK,			TRUE},
    {"player_injured", 		PLR_INJURED,			TRUE},
    {"perma-death", 		PLR_PERMADEATH,		TRUE},
    {"imp-html", 			PLR_IMP_HTML,		TRUE},
    {"autokill", 			PLR_AUTOKILL,			TRUE},
    {"holylight", 			PLR_HOLYLIGHT,		TRUE},
    {"wizinvis", 			PLR_WIZINVIS,			TRUE},
    {"loot_corpse", 		PLR_CANLOOT,			TRUE},
    {"nosummon", 			PLR_NOSUMMON,		TRUE},
    {"nofollow", 			PLR_NOFOLLOW,		TRUE},
    {"xinfo"	, 		PLR_XINFO,			TRUE},
    {"colour", 			PLR_COLOUR,			TRUE},
    {"cursor", 			PLR_CURSOR,			TRUE},
    {"pueblo", 			PLR_PUEBLO,			TRUE},
    {"log", 			PLR_LOG,			TRUE},
    {"deny", 			PLR_DENY,			TRUE},
    {"freeze"	, 		PLR_FREEZE,			TRUE},
    {"autoconsider", 		PLR_AUTOCON,			TRUE},
    {"annoying", 			PLR_ANNOYING,		TRUE},
    {"afk", 			PLR_AFK,			TRUE},
    {"helper", 			PLR_HELPER,			TRUE},
    {"autosave", 			PLR_AUTOSAVE,		TRUE},
    {"cloak", 			PLR_CLOAK,			TRUE},
    {"feed", 			PLR_FEED,			TRUE},
    {"questing", 			PLR_QUESTOR,			TRUE},
    {"autotax", 			PLR_AUTOTAX,			TRUE},
    {"reason", 			PLR_REASON,			TRUE},
    {"haggle",			PLR_HAGGLE,			TRUE},
    {	"",			0, 0			}
};

const struct flag_type wear_flags[] =
{
    {	"take",			ITEM_TAKE,			TRUE	},
    {	"finger",			ITEM_WEAR_FINGER,		TRUE	},
    {	"neck",			ITEM_WEAR_NECK,		TRUE	},
    {	"body",			ITEM_WEAR_BODY,		TRUE	},
    {	"head",			ITEM_WEAR_HEAD,		TRUE	},
    {	"legs",			ITEM_WEAR_LEGS,		TRUE	},
    {	"feet",			ITEM_WEAR_FEET,		TRUE	},
    {	"hands",		ITEM_WEAR_HANDS,		TRUE	},
    {	"arms",			ITEM_WEAR_ARMS,		TRUE	},
    {	"shield",			ITEM_WEAR_SHIELD,		TRUE	},
    {	"about",			ITEM_WEAR_ABOUT,		TRUE	},
    {	"waist",			ITEM_WEAR_WAIST,		TRUE	},
    {	"wrist",			ITEM_WEAR_WRIST,		TRUE	},
    {	"wield",			ITEM_WIELD,			TRUE	},
    {	"hold",			ITEM_HOLD,			TRUE	},
    {   	"two-hands",            	ITEM_TWO_HANDS,         	TRUE    },
    {	"pride",			ITEM_WEAR_PRIDE,		TRUE	},
    {	"ears",			ITEM_WEAR_EARS,		TRUE	},
    {	"face",			ITEM_WEAR_FACE,		TRUE	},
    {	"float",			ITEM_WEAR_FLOAT,		TRUE	},
    {   	"eyes",                 		ITEM_WEAR_EYES,         		TRUE    },
    {   	"back",                 		ITEM_WEAR_BACK,         	TRUE    },
    {   	"tattoo",			ITEM_WEAR_TATTOO,		TRUE    },
    {	"",			0,			0	}
};


const struct flag_type imm_auth[] =
{
    {	"builder",		IMMAUTH_BUILDER,		TRUE	},
    {	"questor",		IMMAUTH_QUESTOR,		TRUE	},
    {	"enforcer",		IMMAUTH_ENFORCER,		TRUE	},
    {	"admin",			IMMAUTH_ADMIN,		TRUE	},
    {	"",			0,			0	}
};


const struct flag_type act_flags[] =
{
    {"npc",			ACT_IS_NPC,			FALSE	},
    {"sentinel",			ACT_SENTINEL,			TRUE	},
    {"scavenger",			ACT_SCAVENGER,		TRUE	},
    {"night-active",		ACT_NIGHT,			TRUE	},
    {"aggressive",			ACT_AGGRESSIVE,		TRUE	},
    {"stay-area",			ACT_STAY_AREA,		TRUE	},
    {"wimpy",			ACT_WIMPY,			TRUE	},
    {"pet",			ACT_PET,			TRUE	},
    {"follower",			ACT_FOLLOWER,		FALSE	},
    {"practice",			ACT_PRACTICE,		TRUE	},
    {"vampire",			ACT_VAMPIRE,			TRUE	},
    {"bound",			ACT_BOUND,			TRUE	},
    {"were",			ACT_WERE,			TRUE	},
    {"undead",			ACT_UNDEAD,			TRUE	},
    {"mount",			ACT_MOUNT,			TRUE	},
    {"cleric",			ACT_CLERIC,			TRUE	},
    {"mage",			ACT_MAGE,			TRUE	},
    {"thief",			ACT_THIEF,			TRUE	},
    {"warrior",			ACT_WARRIOR,		TRUE	},
    {"noalign",			ACT_NOALIGN,			TRUE	},
    {"nopurge",			ACT_NOPURGE,			TRUE	},
    {"dream-native",		ACT_DREAM_NATIVE,		TRUE	},
    {"brainsucked",		ACT_BRAINSUCKED,		TRUE	},
    {"criminal",			ACT_CRIMINAL,		TRUE	},
    {"alienist",			ACT_IS_ALIENIST,		TRUE	},
    {"healer",			ACT_IS_HEALER,		TRUE	},
    {"telepop",			ACT_TELEPOP,			TRUE	},
    {"update-always",		ACT_UPDATE_ALWAYS,	TRUE	},
    {"random-kit",           		ACT_RAND_KIT,           		TRUE,   },         
    {"stay-subarea",		ACT_STAY_SUBAREA,       	TRUE,   },         
    {"auto-home",			ACT_AUTOHOME,		TRUE,   },         
    {"invader",			ACT_INVADER,			TRUE,   },         
    {"watcher",			ACT_WATCHER,		TRUE,   },         
    {"protected",			ACT_PROTECTED,		TRUE,   },         
    {"martial",			ACT_MARTIAL,			TRUE,   },         
    {"",			0,			0	}
};



const struct flag_type affect_flags[] =
{
    {	"blind",	       		AFF_BLIND,			TRUE	},
    {	"invisible",		AFF_INVISIBLE,			TRUE	},
    {	"detect-evil",		AFF_DETECT_EVIL,		TRUE	},
    {	"detect-invis",		AFF_DETECT_INVIS,		TRUE	},
    {	"detect-magic",		AFF_DETECT_MAGIC,		TRUE	},
    {	"detect-hidden",		AFF_DETECT_HIDDEN,		TRUE	},
    {	"mind-meld",		AFF_MELD,			TRUE	},
    {	"sanctuary",		AFF_SANCTUARY,		TRUE	},
    {	"faerie-fire",		AFF_FAERIE_FIRE,		TRUE	},
    {	"infrared",		AFF_INFRARED,		TRUE	},
    {	"curse",			AFF_CURSE,			TRUE	},
    {	"fear", 			AFF_FEAR,			TRUE	},
    {	"poison",		AFF_POISON,			TRUE	},
    {	"protect-good",		AFF_PROTECTG,		TRUE	},
    {	"protect-evil",		AFF_PROTECTE,		TRUE	},
    {	"sneak",			AFF_SNEAK,			TRUE	},
    {	"hide",			AFF_HIDE,			TRUE	},
    {	"sleep",			AFF_SLEEP,			TRUE	},
    {	"charm",			AFF_CHARM,			TRUE	},
    {	"flying",			AFF_FLYING,			TRUE	},
    {	"pass-door",		AFF_PASS_DOOR,		TRUE	},
    {	"haste",			AFF_HASTE,			TRUE	}, 
    {	"calm",			AFF_CALM,			TRUE	},
    {	"plague",		AFF_PLAGUE,			TRUE	},
    {	"weaken",		AFF_WEAKEN,			TRUE	},
    {	"dark-vision",		AFF_DARK_VISION,		TRUE	},
    {	"berserk",		AFF_BERSERK,			TRUE	},
    {	"swim",			AFF_SWIM,			TRUE	},
    {	"regeneration",		AFF_REGENERATION,		TRUE	},
    {	"absorb",		AFF_ABSORB,			TRUE	},
    {	"water-breathing",	AFF_WATER_BREATHING,	TRUE    },
    {	"darkness",		AFF_DARKNESS,		TRUE    },
    {	"aura",			AFF_AURA,			TRUE    },
    {	"hallucinating",		AFF_HALLUCINATING,		TRUE    },
    {	"relaxed",		AFF_RELAXED,			TRUE    },
    {	"fire-shield",		AFF_FIRE_SHIELD,		TRUE    },
    {	"frost-shield",		AFF_FROST_SHIELD,		TRUE    },
    {	"slowed",		AFF_SLOW,			TRUE    },
    {	"globe-of-protection",	AFF_GLOBE,			TRUE    },
    {	"incarnated",		AFF_INCARNATED,		TRUE    },
    { 	"mist",			AFF_MIST,			TRUE    },
    { 	"elder-shield",		AFF_ELDER_SHIELD,		TRUE    },
    { 	"mask",			AFF_POLY,			TRUE    },
    { 	"farsight",		AFF_FARSIGHT,			TRUE    },
    { 	"asceticism",		AFF_ASCETICISM,		TRUE    },
    {	"",			0,			0	}
};

const struct flag_type raffect_flags[] =
{
    {	"holy_ground",		RAFF_HOLY,		TRUE	},
    {	"evil_ground",		RAFF_EVIL,		TRUE	},
    {	"silence",		RAFF_SILENCE,		TRUE	},
    {	"darkness",		RAFF_DARKNESS,	TRUE	},
    {	"wild-magic",		RAFF_WILD_MAGIC,	TRUE	},
    {	"low-magic",		RAFF_LOW_MAGIC,	TRUE	},
    {	"high-magic",		RAFF_HIGH_MAGIC,	TRUE	},
    {	"no_breathe",		RAFF_NOBREATHE,	TRUE	},
    {	"drain",	                	RAFF_DRAIN,		TRUE	},
    {	"mare",			RAFF_MARE,		TRUE	},
    {	"enclosed",		RAFF_ENCLOSED,	TRUE	},
    {	"no_morgue",		RAFF_NOMORGUE,	TRUE	},
    {	"fatigue",		RAFF_FATIGUE,		TRUE	},
    {	"destructive",		RAFF_DESTRUCTIVE,	TRUE	},
    {	"",			0,			0	}
};

const struct flag_type furniture_class[] =
{
    {	"sit",		FURN_SIT,		TRUE	},
    {	"rest",		FURN_REST,		TRUE	},
    {	"sleep",	FURN_SLEEP,             TRUE	},
    {	"",			0,			0	}
};

/*
 * Used when adding an affect to tell where it goes.
 * See addaffect and delaffect in act_olc.c
 */
const struct flag_type apply_flags[] =
{
    {	"none",			APPLY_NONE,			TRUE	},
    {	"strength",		APPLY_STR,			TRUE	},
    {	"dexterity",		APPLY_DEX,			TRUE	},
    {	"intelligence",		APPLY_INT,			TRUE	},
    {	"wisdom",		APPLY_WIS,			TRUE	},
    {	"constitution",		APPLY_CON,			TRUE	},
    {	"luck",			APPLY_LUCK,			TRUE	},
    {	"charisma",		APPLY_CHA,			TRUE	},
    {	"max-strength",		APPLY_MAX_STR,		TRUE	},
    {	"max-dexterity",		APPLY_MAX_DEX,		TRUE	},
    {	"max-intelligence",	APPLY_MAX_INT,		TRUE	},
    {	"max-wisdom",		APPLY_MAX_WIS,		TRUE	},
    {	"max-constitution",	APPLY_MAX_CON,		TRUE	},
    {	"max-luck",		APPLY_MAX_LUCK,		TRUE	},
    {	"max-charisma",		APPLY_MAX_CHA,		TRUE	},
    {	"sex",			APPLY_SEX,			TRUE	},
    {	"class",			APPLY_CLASS,			TRUE	},
    {	"level",			APPLY_LEVEL,			TRUE	},
    {	"age",			APPLY_AGE,			TRUE	},
    {	"height",		APPLY_HEIGHT,			TRUE	},
    {	"weight",		APPLY_WEIGHT,		TRUE	},
    {	"mana",			APPLY_MANA,			TRUE	},
    {	"hp",			APPLY_HIT,			TRUE	},
    {	"move",			APPLY_MOVE,			TRUE	},
    {	"gold",			APPLY_GOLD,			TRUE	},
    {	"experience",		APPLY_EXP,			TRUE	},
    {	"ac",			APPLY_AC,			TRUE	},
    {	"hitroll",			APPLY_HITROLL,		TRUE	},
    {	"damroll",		APPLY_DAMROLL,		TRUE	},
    {	"saving-para",		APPLY_SAVING_PARA,		TRUE	},
    {	"saving-rod",		APPLY_SAVING_ROD,		TRUE	},
    {	"saving-petri",		APPLY_SAVING_PETRI,		TRUE	},
    {	"saving-breath",		APPLY_SAVING_BREATH,	TRUE	},
    {	"saving-spell",		APPLY_SAVING_SPELL,		TRUE	},
    {	"sanity",		APPLY_SANITY,			TRUE	},
    {	"skill",		    	APPLY_SKILL,			FALSE	},
    {	"immunity",		APPLY_IMMUNITY,		TRUE	},
    {	"resistance",		APPLY_RESISTANCE,		TRUE	},
    {	"effect",	                 	APPLY_EFFECT,			TRUE	},
    {	"magic",			APPLY_MAGIC,			TRUE	},
    {	"align",	                 	APPLY_ALIGN,			TRUE	},
    {	"crime",	                 	APPLY_CRIME_RECORD,	FALSE	},
    {	"object",                 	APPLY_OBJECT,			FALSE	},
    {	"sanity-gain",                 	APPLY_SANITY_GAIN,		TRUE	},
    {	"env-immunity",                 	APPLY_ENV_IMMUNITY,	TRUE	},
    {	"",			0,			0	}
};



/*
 * What is seen.
 */
const struct flag_type wear_loc_strings[] =
{
    {	"in the inventory",	WEAR_NONE,		TRUE	},
    {	"as a light",		WEAR_LIGHT,		TRUE	},
    {	"on the left finger",	WEAR_FINGER_L,	TRUE	},
    {	"on the right finger",	WEAR_FINGER_R,	TRUE	},
    {	"around the neck (1)",	WEAR_NECK_1,		TRUE	},
    {	"around the neck (2)",	WEAR_NECK_2,		TRUE	},
    {	"on the body",		WEAR_BODY,		TRUE	},
    {	"over the head",		WEAR_HEAD,		TRUE	},
    {	"on the legs",		WEAR_LEGS,		TRUE	},
    {	"on the feet",		WEAR_FEET,		TRUE	},
    {	"on the hands",		WEAR_HANDS,		TRUE	},
    {	"on the arms",		WEAR_ARMS,		TRUE	},
    {	"as a shield",		WEAR_SHIELD,		TRUE	},
    {	"about the shoulders",	WEAR_ABOUT,		TRUE	},
    {	"around the waist",	WEAR_WAIST,		TRUE	},
    {	"on the left wrist",	WEAR_WRIST_L,	TRUE	},
    {	"on the right wrist",	WEAR_WRIST_R,	TRUE	},
    {	"wielded",		WEAR_WIELD,		TRUE	},
    {	"with pride",		WEAR_PRIDE,		TRUE	},
    {	"on the ears",		WEAR_EARS,		TRUE	},
    {	"on the face",		WEAR_FACE,		TRUE	},
    {	"floating nearby",	WEAR_FLOAT,		TRUE	},
    {  	"over eyes",            	WEAR_EYES,      		TRUE    }, 
    {   	"on back",              	WEAR_BACK,      	TRUE    }, 
    {   	"tattoo",              		WEAR_TATTOO,      	TRUE    }, 
    {	"",			0			}
};


const struct flag_type wear_loc_flags[] =
{
    {	"none",		WEAR_NONE,		TRUE	},
    {	"light",		WEAR_LIGHT,		TRUE	},
    {	"lfinger",	WEAR_FINGER_L,	TRUE	},
    {	"rfinger",	WEAR_FINGER_R,	TRUE	},
    {	"neck1",		WEAR_NECK_1,		TRUE	},
    {	"neck2",		WEAR_NECK_2,		TRUE	},
    {	"body",		WEAR_BODY,		TRUE	},
    {	"head",		WEAR_HEAD,		TRUE	},
    {	"legs",		WEAR_LEGS,		TRUE	},
    {	"feet",		WEAR_FEET,		TRUE	},
    {	"hands",	WEAR_HANDS,		TRUE	},
    {	"arms",		WEAR_ARMS,		TRUE	},
    {	"shield",		WEAR_SHIELD,		TRUE	},
    {	"about",		WEAR_ABOUT,		TRUE	},
    {	"waist",		WEAR_WAIST,		TRUE	},
    {	"lwrist",		WEAR_WRIST_L,	TRUE	},
    {	"rwrist",		WEAR_WRIST_R,	TRUE	},
    {	"wielded",	WEAR_WIELD,		TRUE	},
    {	"hold",		WEAR_HOLD,		TRUE	},
    {	"pride",		WEAR_PRIDE,		TRUE	},
    {	"ears",		WEAR_EARS,		TRUE	},
    {	"face",		WEAR_FACE,		TRUE	},
    {	"float",		WEAR_FLOAT,		TRUE	},
    {   	"eyes",         	WEAR_EYES,      		TRUE    },
    {   	"back",         	WEAR_BACK,      	TRUE    },
    {   	"tattoo",         	WEAR_TATTOO,      	TRUE    },
    {	"",		0,		0	}
};



const struct flag_type weapon_flags[] =
{
    {   "hit",		WDT_HIT,		TRUE	},
    {   "slice",		WDT_SLICE,		TRUE	},
    {   "stab",		WDT_STAB,		TRUE	},
    {   "slash",		WDT_SLASH,		TRUE	},
    {   "whip",		WDT_WHIP,		TRUE	},
    {   "claw",		WDT_CLAW,		TRUE	},
    {   "blast",		WDT_BLAST,		TRUE	},
    {   "pound",		WDT_POUND,		TRUE	},
    {   "crush",		WDT_CRUSH,		TRUE	},
    {   "grep",		WDT_GREP,		TRUE	},
    {   "bite",		WDT_BITE,		TRUE	},
    {   "pierce",		WDT_PIERCE,		TRUE	},
    {   "suction",		WDT_SUCTION,		TRUE	},
    {   "beating",		WDT_BEATING,		TRUE	},
    {   "digestion",		WDT_DIGESTION,	TRUE	},
    {   "charge",		WDT_CHARGE,		TRUE	},
    {   "slap",		WDT_SLAP,		TRUE	},
    {   "punch",		WDT_PUNCH,		TRUE	},
    {   "wrath",		WDT_WRATH,		TRUE	},
    {   "magic",		WDT_MAGIC,		TRUE	},
    {   "divine-power",	WDT_DIVINE_POWER,	TRUE	},
    {   "cleave",		WDT_CLEAVE,		TRUE	},
    {   "scratch",		WDT_SCRATCH,	TRUE	},
    {   "peck-pierce",  	WDT_PECK_PIERCE,	TRUE	},
    {   "peck-bash",	WDT_PECK_BASH,	TRUE	},
    {   "chop",		WDT_CHOP,		TRUE	},
    {   "sting",		WDT_STING,		TRUE	},
    {   "smash",		WDT_SMASH,		TRUE	},
    {   "shocking-bite", 	WDT_SHOCKING_BITE,	TRUE	},
    {   "flaming-bite",	WDT_FLAMING_BITE,	TRUE	},
    {   "freezing-bite", 	WDT_FREEZING_BITE,	TRUE    },
    {   "acidic-bite",	WDT_ACIDIC_BITE,	TRUE	},
    {   "chomp",		WDT_CHOMP,		TRUE	},
    {   "shot",         		WDT_SHOT,		TRUE    },  
    {   "life-drain",         	WDT_LIFE_DRAIN,	TRUE     }, 
    {   "thirst",         	WDT_THIRST,		TRUE     }, 
    {   "slime",         	WDT_SLIME,		TRUE     }, 
    {   "thwack",         	WDT_THWACK,	TRUE     }, 
    {   "flame",         	WDT_FLAME,		TRUE     }, 
    {   "chill",         		WDT_CHILL,		TRUE     }, 
    {   "elder-power",         	WDT_ELDER,		TRUE     }, 
    {   "screech",         	WDT_SCREECH,		TRUE     }, 
    {	"",			0,		TRUE	}
};


const struct flag_type container_flags[] =
{
    {	"closeable",		1,		TRUE	},
    {	"pickproof",		2,		TRUE	},
    {	"closed",		4,		TRUE	},
    {	"locked",		8,		TRUE	},
    {	"",			0,		0	}
};



const struct flag_type liquid_flags[] =
{
    {	"water",			LIQ_WATER,		TRUE	},
    {	"beer",			LIQ_BEER,		TRUE	},
    {	"red wine",		LIQ_RED_WINE,		TRUE	},
    {	"ale",			LIQ_ALE,		TRUE	},
    {	"dark ale",		LIQ_DARK_ALE,	TRUE	},
    {	"whisky",		LIQ_WHISKY,		TRUE	},
    {	"lemonade",		LIQ_LEMONADE,	TRUE	},
    {	"firebreather",		LIQ_FIREBREATHER,	TRUE	},
    {	"local specialty",		LIQ_SPECIAL,		TRUE	},
    {	"slime mold juice",	LIQ_JUICE,		TRUE	},
    {	"milk",			LIQ_MILK,		TRUE	},
    {	"tea",			LIQ_TEA,		TRUE	},
    {	"coffee",		LIQ_COFFEE,		TRUE	},
    {	"blood",			LIQ_BLOOD,		TRUE	},
    {	"salt water",		LIQ_SALT_WATER,	TRUE	},
    {	"coke",			LIQ_CHERRY_COLA,	TRUE	},
    {   	"dirty water",		LIQ_DIRTY_WATER,	TRUE    },
    {   	"ambrosia",		LIQ_AMBROSIA,	TRUE    },
    {   	"slime",			LIQ_SLIME,	        	TRUE    },
    {   	"dark coffee",		LIQ_DARK_COFFEE,	TRUE    },
    {   	"vodka",	        	LIQ_VODKA,	        	TRUE    },
    {   	"soda water",		LIQ_SODA_WATER,	TRUE    },
    {	"petrol",			LIQ_PETROL,		TRUE	},
    {	"root beer",		LIQ_ROOT_BEER,	TRUE	},
    {	"elvish wine",		LIQ_ELF_WINE,		TRUE	},
    {	"white wine",		LIQ_WHITE_WINE,	TRUE	},
    {	"champagne",		LIQ_CHAMPAGNE,	TRUE	},
    {	"mead",			LIQ_MEAD,		TRUE	},
    {	"rose wine",		LIQ_ROSE_WINE,	TRUE	},
    {	"benedictine wine",	LIQ_BENEDICTINE,	TRUE	},
    {	"cranberry juice",		LIQ_CRANBERRY,	TRUE	},
    {	"orange juice",		LIQ_ORANGE,		TRUE	},
    {	"absinthe",		LIQ_ABSINTHE,		TRUE	},
    {	"brandy",		LIQ_BRANDY,		TRUE	},
    {	"aquavit",		LIQ_AQUAVIT,		TRUE	},
    {	"schnapps",		LIQ_SCHNAPPS,		TRUE	},
    {	"icewine",		LIQ_ICEWINE,		TRUE	},
    {	"amontillado",		LIQ_AMONTILLADO,	TRUE	},
    {	"sherry",		LIQ_SHERRY,		TRUE	},
    {	"framboise",		LIQ_FRAMBOISE,	TRUE	},
    {	"rum",			LIQ_RUM,		TRUE	},
    {	"cordial",		LIQ_CORDIAL,		TRUE	},
    {	"port",			LIQ_PORT,		TRUE	},
    {	"wheatgrass",		LIQ_WHEATGRASS,	TRUE	},
    {	"",			0,			0	}
};


const struct flag_type form_flags[] =
{
    {   "edible",        	FORM_EDIBLE,          		TRUE    },
    {   "poison",        	FORM_POISON,          		TRUE    },
    {   "magical",       	FORM_MAGICAL,         		TRUE    },
    {   "decay",         	FORM_INSTANT_DECAY,   	TRUE    },
    {   "other",         	FORM_OTHER,           		TRUE    },
    {   "bleeding",         	FORM_BLEEDING,           		TRUE    },
    {   "animal",        	FORM_ANIMAL,          		TRUE    },
    {   "sentient",      	FORM_SENTIENT,        		TRUE    },
    {   "undead",        	FORM_UNDEAD,          		TRUE    },
    {   "construct",     	FORM_CONSTRUCT,       		TRUE    },
    {   "mist",          		FORM_MIST,            		TRUE    },
    {   "intangible",    	FORM_INTANGIBLE,      		TRUE    },
    {   "biped",         	FORM_BIPED,           		TRUE    },
    {   "centaur",       	FORM_CENTAUR,         		TRUE    },
    {   "insect",        	FORM_INSECT,          		TRUE    },
    {   "spider",        	FORM_SPIDER,          		TRUE    },
    {   "crustacean",    	FORM_CRUSTACEAN,      	TRUE    },
    {   "worm",          	FORM_WORM,            		TRUE    },
    {   "blob",          	FORM_BLOB,            		TRUE    },
    {   "plant",          	FORM_PLANT,            		TRUE    },
    {   "noweapon",          	FORM_NOWEAPON,        		TRUE    },
    {   "mammal",        	FORM_MAMMAL,          		TRUE    },
    {   "bird",          		FORM_BIRD,            		TRUE    },
    {   "reptile",       	FORM_REPTILE,         		TRUE    },
    {   "snake",         	FORM_SNAKE,           		TRUE    },
    {   "dragon",        	FORM_DRAGON,          		TRUE    },
    {   "amphibian",     	FORM_AMPHIBIAN,       		TRUE    },
    {   "fish",          		FORM_FISH,            		TRUE    },
    {   "cold-blood",    	FORM_COLD_BLOOD,      	TRUE    },
    {   "machine",    	FORM_MACHINE,      		TRUE    },
    {   "",              0,                    0       }
};

const struct flag_type part_flags[] =
{
    {   "head",          	PART_HEAD,            	TRUE    },
    {   "arms",          	PART_ARMS,            	TRUE    },
    {   "legs",          		PART_LEGS,            	TRUE    },
    {   "heart",         	PART_HEART,           	TRUE    },
    {   "brains",        	PART_BRAINS,          	TRUE    },
    {   "guts",          	PART_GUTS,            	TRUE    },
    {   "hands",         	PART_HANDS,           	TRUE    },
    {   "feet",          		PART_FEET,            	TRUE    },
    {   "fingers",       	PART_FINGERS,         	TRUE    },
    {   "ear",           		PART_EAR,             	TRUE    },
    {   "eye",           		PART_EYE,             	TRUE    },
    {   "long-tongue",   	PART_LONG_TONGUE,   	TRUE    },
    {   "eyestalks",     	PART_EYESTALKS,       	TRUE    },
    {   "tentacles",     	PART_TENTACLES,       	TRUE    },
    {   "fins",          		PART_FINS,            	TRUE    },
    {   "wings",         	PART_WINGS,           	TRUE    },
    {   "tail",		PART_TAIL,	       	TRUE    },
    {   "leaves",		PART_LEAVES,	       	TRUE    },
    {   "claws",         	PART_CLAWS,           	TRUE    },
    {   "fangs",         	PART_FANGS,           	TRUE    },
    {   "horns",         	PART_HORNS,           	TRUE    },
    {   "scales",        	PART_SCALES,          	TRUE    },
    {   "tusks",         	PART_TUSKS,           	TRUE    },
    {   "long-tail",	 	PART_LONG_TAIL,       	TRUE    },
    {   "stinger",	 	PART_STINGER,         	TRUE    },
    {   "",              0,                    0       }
};


const struct flag_type ac_type[] =
{
    {   "pierce",        	AC_PIERCE,            	TRUE    },
    {   "bash",          	AC_BASH,              	TRUE    },
    {   "slash",         	AC_SLASH,             	TRUE    },
    {   "exotic",        	AC_EXOTIC,            	TRUE    },
    {   "",              0,                    0       }
};


const struct flag_type size_flags[] =
{
    {   "tiny",          	SIZE_TINY,            	TRUE    },
    {   "small",         SIZE_SMALL,           	TRUE    },
    {   "medium",    	SIZE_MEDIUM,          	TRUE    },
    {   "large",         	SIZE_LARGE,           	TRUE    },
    {   "huge",         SIZE_HUGE,            	TRUE    },
    {   "giant",         SIZE_GIANT,           	TRUE    },
    {   "",              0,                    0       },
};


const struct flag_type weapon_class[] =
{
    {   "exotic",       		WEAPON_EXOTIC,         	TRUE    },
    {   "sword",        	WEAPON_SWORD,          	TRUE    },
    {   "dagger",       	WEAPON_DAGGER,         	TRUE    },
    {   "spear",        		WEAPON_SPEAR,          	TRUE    },
    {   "mace",         	WEAPON_MACE,           	TRUE    },
    {   "axe",          		WEAPON_AXE,            	TRUE    },
    {   "flail",        		WEAPON_FLAIL,          	TRUE    },
    {   "whip",         		WEAPON_WHIP,           	TRUE    },
    {   "polearm",      	WEAPON_POLEARM,      TRUE    },
    {   "gun",          		WEAPON_GUN,            	TRUE    },
    {   "handgun",      	WEAPON_HANDGUN,     TRUE    },  
    {   "submachingun", 	WEAPON_SMG,            	TRUE    },  
    {   "bow",		WEAPON_BOW,            	TRUE    },  
    {   "staff",		WEAPON_STAFF,          	TRUE    },  
    {   "",              0,                    0       }
};

const struct flag_type portal_class[] =
{
    {   "magic",       		PORTAL_MAGIC,         	TRUE    },
    {   "building",        	PORTAL_BUILDING,         TRUE    },
    {   "vehicle",        	PORTAL_VEHICLE,          	TRUE    },
    {   "mirror",       		PORTAL_MIRROR,         	TRUE    },
    {   "",              0,                    0       }
};

const struct flag_type tree_size[] =
{
    {   "seed",           	TREE_SEED,         	TRUE    },
    {   "sprout",        	TREE_SPROUT,          	TRUE    },
    {   "small",           	TREE_SMALL,          	TRUE    },
    {   "tree",             	TREE_NORMAL,         	TRUE    },
    {   "large",           	TREE_LARGE,          	TRUE    },
    {   "ancient",       	TREE_ANCIENT,         	TRUE    },
    {   "",              0,                    0       }
};

const struct flag_type currency_type[] =
{
    {   "{rCopper{x",           	CURRENCY_DEFAULT,   	TRUE    },
    {   "{gDollar{x",        	CURRENCY_DOLLAR,  	TRUE    },
    {   "{yGold{x",           	CURRENCY_GOLD,          	TRUE    },
    {   "{cCrown{x",	CURRENCY_CROWN,	TRUE    },
    {   "2",           		CURRENCY_EMPTY,        	TRUE    },
    {   "",              -1,                    0       }
};

const struct flag_type currency_accept[] =
{
    {   "copper",           	CURRENCY_DEFAULT,   	TRUE    },
    {   "dollar",        	CURRENCY_DOLLAR,  	TRUE    },
    {   "gold",           	CURRENCY_GOLD,          	TRUE    },
    {   "crown",      		CURRENCY_CROWN,	TRUE    },
    {   "2",           		CURRENCY_EMPTY,        	TRUE    },
    {   "",              -1,                    0       }
};

const struct flag_type figurine_behavior[] =
{
    {   "ignore",                	FIGURINE_IGNORE,         		TRUE    },
    {   "aggressive",        	FIGURINE_AGGRESSIVE,          	TRUE    },
    {   "tame",                   	FIGURINE_TAME,          		TRUE    },
    {   "",              0,                    0       }
};


const struct flag_type figurine_type[] =
{
    {   "false",                	FIGURINE_FALSE,         		TRUE    },
    {   "true",        		FIGURINE_TRUE,          		TRUE    },
    {   "container",                 	FIGURINE_CONT,          		TRUE    },
    {   "",              0,                    0       }
};

const struct flag_type tree_type[] =
{
    {   "ordinary",       TREE_ORDINARY,         TRUE    },
    {   "light",              TREE_LIGHT,          	TRUE    },
    {   "darkness",      TREE_DARKNESS,         TRUE    },
    {   "chaos",           TREE_CHAOS,         	TRUE    },
    {   "order",            TREE_ORDER,          	TRUE    },
    {   "nature",          TREE_NATURE,          	TRUE    },
    {   "",              0,                    0       }
};

const struct flag_type photo_state[] =
{
    {   "blank",         PHOTO_BLANK,         	TRUE    },
    {   "exposing",   PHOTO_EXPOSING,          TRUE    },
    {   "taken",         PHOTO_TAKEN,          	TRUE    },
    {   "",              0,                    0       }
};

const struct flag_type pool_type[] =
{
    {   "generic",    	POOL_GENERIC,         	TRUE    },
    {   "health",   	POOL_HEALTH,          	TRUE    },
    {   "move",        	POOL_MOVES,          	TRUE    },
    {   "mana",        	POOL_MAGIC,          	TRUE    },
    {   "",              0,                    0       }
};

const struct flag_type refill_type[] =
{
    {   "none",    	REFILL_NONE,         	TRUE    },
    {   "oil",   	REFILL_OIL,          	TRUE    },
    {   "paraffin",    REFILL_PARAFFIN,          	TRUE    },
    {   "kerosene",  REFILL_KEROSENE,          	TRUE    },
    {   "battery",     REFILL_BATTERY,          	TRUE    },
    {   "",              0,                    0       }
};

const struct flag_type trap_class[] =
{
    {   "tangle",       	TRAP_TANGLE,         	TRUE    },
    {   "blade",        		TRAP_WOUND,          	TRUE    },
    {   "dart",           	TRAP_DART,         	TRUE    },
    {   "mystic",        	TRAP_MYSTIC,          	TRUE    },
    {   "magic",        	TRAP_MAGIC,          	TRUE    },
    {   "scare",        		TRAP_SCARE,          	TRUE    },
    {   "polymorph",        	TRAP_POLY,          	TRUE    },
    {   "rust",        		TRAP_RUST,          	TRUE    },
    {   "",              0,                    0       }
};

/* Following table, each key must be one word... */

const struct flag_type weapon_type[] =
{
    {   "flaming",		WEAPON_FLAMING,		TRUE    },
    {   "frost",		WEAPON_FROST,		TRUE    },
    {   "vampiric",		WEAPON_VAMPIRIC,		TRUE    },
    {   "sharp",		WEAPON_SHARP,		TRUE    },
    {   "vorpal",		WEAPON_VORPAL,		TRUE    },
    {   "plague",		WEAPON_PLAGUE,		TRUE    },
    {   "acid",		WEAPON_ACID,		TRUE    },
    {   "lightning",		WEAPON_LIGHTNING,		TRUE    },
    {   "poisoned",		WEAPON_POISON,		TRUE    },
    {   "one-shot",		WEAPON_ONE_SHOT,		TRUE    },
    {   "two-shot",		WEAPON_TWO_SHOT,		TRUE    }, 
    {   "six-shot",		WEAPON_SIX_SHOT,		TRUE    },
    {   "twelve-shot",	WEAPON_TWELVE_SHOT,	TRUE    },
    {   "36-shot",		WEAPON_36_SHOT,		TRUE    },
    {   "108-shot",		WEAPON_108_SHOT,		TRUE    },
    {   "stun",		WEAPON_STUN,		TRUE    },
    {   "9mm_parabellum",	WEAPON_PARABELLUM,	TRUE    },
    {   ".45",		WEAPON_CAL45,		TRUE    },
    {   "357_magnum",	WEAPON_357MAGNUM,		TRUE    },
    {   "458_winchester",	WEAPON_458WINCHESTER,	TRUE    },
    {   ".22_rifle",		WEAPON_22RIFLE,		TRUE    },
    {   ".30_carabine",	WEAPON_30CARABINE,		TRUE    },
    {   "12_guage",		WEAPON_12GUAGE,		TRUE    },
    {   "arrow",		WEAPON_ARROW,		TRUE    },
    {   "energy",		WEAPON_ENERGY,		TRUE    },
    {   "",			0,			0       }
};

/* See also const.c: attack_table, can_block_dam */

const struct flag_type damage_type[] =
{
    {"none", 	   	DAM_NONE,		TRUE	},
    {"bash",		DAM_BASH,		TRUE	},
    {"pierce",	   	DAM_PIERCE,		TRUE	},
    {"slash",	   	DAM_SLASH,		TRUE	},
    {"fire",		DAM_FIRE,		TRUE	},
    {"cold",		DAM_COLD,		TRUE	},
    {"lightning",	   	DAM_LIGHTNING,	TRUE	},
    {"acid",		DAM_ACID,		TRUE	},
    {"poison",	   	DAM_POISON,		TRUE	},
    {"negative power",  	DAM_NEGATIVE,	TRUE	},
    {"holy power",	  	DAM_HOLY,		TRUE	},
    {"energy",         	DAM_ENERGY,		TRUE	},
    {"mental power",   	DAM_MENTAL,		TRUE	},
    {"disease",	  	DAM_DISEASE,		TRUE	},
    {"drowning",	  	DAM_DROWNING,	TRUE	},
    {"light",	  	DAM_LIGHT,		TRUE	},
    {"chaos",	  	DAM_OTHER,		TRUE	},
    {"harm",		DAM_HARM,		TRUE	},
    {"shot",           		DAM_SHOT,   		TRUE    },
    {"old",           		DAM_OLD,   		TRUE    },
    {"sound",         		DAM_SOUND,   		TRUE    },
    {   "",                	0,     		0       }
};

const struct flag_type attack_type[] =
{
    {   "hit",      		WDT_HIT,		TRUE 	 },  
    {   "slice",    		WDT_SLICE,		TRUE     },
    {   "stab",     		WDT_STAB, 		TRUE	 },
    {   "slash",    		WDT_SLASH,  		TRUE	 },
    {   "whip",     		WDT_WHIP,  		TRUE	 },
    {   "claw",     		WDT_CLAW,  		TRUE	 },
    {   "blast",    		WDT_BLAST,   		TRUE	 },
    {   "pound",    		WDT_POUND,   		TRUE	 },
    {   "crush",    		WDT_CRUSH,   		TRUE	 },
    {   "grep",     		WDT_GREP,  		TRUE	 },
    {   "bite",     		WDT_BITE, 		TRUE	 },  
    {   "pierce",   		WDT_PIERCE, 		TRUE	 },
    {   "suction",  		WDT_SUCTION,   	TRUE	 },
    {   "beating",  		WDT_BEATING,   	TRUE	 },
    {   "digestion",		WDT_DIGESTION, 	TRUE	 },
    {   "charge",   		WDT_CHARGE,   	TRUE	 },  
    {   "slap",     		WDT_SLAP,   		TRUE	 },
    {   "punch",    		WDT_PUNCH,   		TRUE	 },
    {   "wrath",    		WDT_WRATH, 		TRUE	 },
    {   "magic",    		WDT_MAGIC, 		TRUE	 },
    {   "divine power", 	WDT_DIVINE_POWER,   	TRUE	 },  
    {   "cleave",   		WDT_CLEAVE,	  	TRUE	 },
    {   "scratch",  		WDT_SCRATCH,	TRUE	 },
    {   "peck pierce",	WDT_PECK_PIERCE, 	TRUE	 },
    {   "peck bash",  	WDT_PECK_BASH,	TRUE	 },
    {   "chop",     		WDT_CHOP,	  	TRUE	 },  
    {   "sting",    		WDT_STING,	 	TRUE	 },
    {   "smash",    		WDT_SMASH,	   	TRUE	 },
    {   "shocking bite",	WDT_SHOCKING_BITE,	TRUE     },
    {   "flaming bite", 	WDT_FLAMING_BITE,   	TRUE	 },
    {   "freezing bite",	WDT_FREEZING_BITE,   	TRUE	 },  
    {   "acidic bite",  	WDT_ACIDIC_BITE,   	TRUE	 },
    {   "chomp",    		WDT_CHOMP, 		TRUE	 },
    {   "shot",         		WDT_SHOT,		TRUE     }, 
    {   "life drain",         	WDT_LIFE_DRAIN,	TRUE     }, 
    {   "thirst",         	WDT_THIRST,		TRUE     }, 
    {   "slime",         	WDT_SLIME,		TRUE     }, 
    {   "thwack",         	WDT_THWACK,	TRUE     }, 
    {   "flame",         	WDT_FLAME,		TRUE     }, 
    {   "chill",         		WDT_CHILL,		TRUE     }, 
    {   "elder power",         	WDT_ELDER,		TRUE     }, 
    {   "screech",         	WDT_SCREECH,		TRUE     }, 
    {	"",			0,			0	 }
};	


const struct flag_type attack_types[] =
{
    {   "none",      		WDT_HIT,		TRUE	},  
    {   "slice",    		WDT_SLICE,		TRUE    },
    {   "stab",     		WDT_STAB, 		TRUE	 },
    {   "slash",    		WDT_SLASH,  		TRUE	 },
    {   "whip",     		WDT_WHIP,  		TRUE	 },
    {   "claw",     		WDT_CLAW,  		TRUE	 },  
    {   "blast",    		WDT_BLAST,   		TRUE	 },
    {   "pound",    		WDT_POUND,   		TRUE	 },
    {   "crush",    		WDT_CRUSH,   		TRUE	 },
    {   "grep",     		WDT_GREP,  		TRUE	 },
    {   "bite",     		WDT_BITE, 		TRUE	 },  
    {   "pierce",   		WDT_PIERCE, 		TRUE	 },
    {   "suction",  		WDT_SUCTION,   	TRUE	 },
    {   "beating",  		WDT_BEATING,   	TRUE	 },
    {   "digestion",		WDT_DIGESTION, 	TRUE	 },
    {   "charge",   		WDT_CHARGE,   	TRUE	 },  
    {   "slap",     		WDT_SLAP,   		TRUE	 },
    {   "punch",    		WDT_PUNCH,   		TRUE	 },
    {   "wrath",    		WDT_WRATH, 		TRUE	 },
    {   "magic",    		WDT_MAGIC, 		TRUE	 },
    {   "divine",	 	WDT_DIVINE_POWER,   	TRUE	 },  
    {   "cleave",   		WDT_CLEAVE,	  	TRUE	 },
    {   "scratch",  		WDT_SCRATCH,	TRUE	 },
    {   "peck",		WDT_PECK_PIERCE, 	TRUE	 },
    {   "peckb",  		WDT_PECK_BASH,	TRUE	 },
    {   "chop",     		WDT_CHOP,	  	TRUE	 },  
    {   "sting",    		WDT_STING,	 	TRUE	 },
    {   "smash",    		WDT_SMASH,	   	TRUE	 },
    {   "shbite",		WDT_SHOCKING_BITE,	TRUE     },
    {   "flbite",	 	WDT_FLAMING_BITE,   	TRUE	 },
    {   "frbite",		WDT_FREEZING_BITE,   	TRUE	 },  
    {   "acbite", 	 	WDT_ACIDIC_BITE,   	TRUE	 },
    {   "chomp",    		WDT_CHOMP, 		TRUE	 },
    {   "shot",         		WDT_SHOT,		TRUE     }, 
    {   "drain",         	WDT_LIFE_DRAIN,	TRUE     }, 
    {   "thirst",         	WDT_THIRST,		TRUE     }, 
    {   "slime",         	WDT_SLIME,		TRUE     }, 
    {   "thwack",         	WDT_THWACK,	TRUE     }, 
    {   "flame",         	WDT_FLAME,		TRUE     }, 
    {   "chill",         	 	WDT_CHILL,		TRUE     }, 
    {   "epower",         	WDT_ELDER,		TRUE     }, 
    {   "screech",         	WDT_SCREECH,		TRUE     }, 
    {	"",			0,			0	 }
};	


const struct flag_type off_flags[] =
{
    {   "area-attack",   	OFF_AREA_ATTACK,     TRUE    },
    {   "backstab",      	OFF_BACKSTAB,         	TRUE    },
    {   "bash",          	OFF_BASH,             	TRUE    },
    {   "berserk",       	OFF_BERSERK,          	TRUE    },
    {   "disarm",        	OFF_DISARM,           	TRUE    },
    {   "dodge",         	OFF_DODGE,            	TRUE    },
    {   "fade",          		OFF_FADE,             	TRUE    },
    {   "fast",          		OFF_FAST,             	TRUE    },
    {   "kick",          		OFF_KICK,             	TRUE    },
    {   "kick-dirt",     	OFF_KICK_DIRT,        	TRUE    },
    {   "parry",         	OFF_PARRY,            	TRUE    },
    {   "rescue",        	OFF_RESCUE,           	TRUE    },
    {   "tail",          		OFF_TAIL,             	TRUE    },
    {   "trip",          		OFF_TRIP,             	TRUE    },
    {   "crush",         	OFF_CRUSH,            	TRUE    },
    {   "assist-all",    	ASSIST_ALL,           	TRUE    },
    {   "assist-align",  	ASSIST_ALIGN,         	TRUE    },
    {   "assist-race",   	ASSIST_RACE,          	TRUE    },
    {   "assist-player", 	ASSIST_PLAYERS,       	TRUE    },
    {   "assist-guard",  	ASSIST_GUARD,         	TRUE    },
    {   "assist-vnum",   	ASSIST_VNUM,          	TRUE    },
    {   "stun",   		OFF_STUN,          	TRUE    },
    {   "distract",   		OFF_DISTRACT,          	TRUE    },
    {   "mercy",   		OFF_MERCY,          	TRUE    },
    {   "",              0,                    0       }
};


const struct flag_type imm_flags[] =
{
    {   "summon",        	IMM_SUMMON,           	TRUE    },
    {   "charm",         	IMM_CHARM,            	TRUE    },
    {   "magic",         	IMM_MAGIC,            	TRUE    },
    {   "weapon",        	IMM_WEAPON,           	TRUE    },
    {   "bash",          	IMM_BASH,             	TRUE    },
    {   "pierce",        	IMM_PIERCE,           	TRUE    },
    {   "slash",         	IMM_SLASH,            	TRUE    },
    {   "fire",          		IMM_FIRE,             	TRUE    },
    {   "cold",          		IMM_COLD,             	TRUE    },
    {   "lightning",     	IMM_LIGHTNING,        	TRUE    },
    {   "acid",          		IMM_ACID,             	TRUE    },
    {   "poison",        	IMM_POISON,           	TRUE    },
    {   "negative",      	IMM_NEGATIVE,         	TRUE    },
    {   "holy",          	IMM_HOLY,             	TRUE    },
    {   "energy",        	IMM_ENERGY,           	TRUE    },
    {   "mental",        	IMM_MENTAL,           	TRUE    },
    {   "disease",       	IMM_DISEASE,          	TRUE    },
    {   "drowning",      	IMM_DROWNING,         	TRUE    },
    {   "light",         		IMM_LIGHT,            	TRUE    },
    {   "bullets",       	IMM_BULLETS,          	TRUE    },
    {   "mask",       		IMM_MASK,          	TRUE    },
    {   "old",       		IMM_OLD,          		TRUE    },
    {   "sound",      		IMM_SOUND,    		TRUE    },
    {   "",          0,            0    }
};


const struct flag_type immapp_flags[] =
{
    {   "summon",        	IMMAPP_SUMMON,       	TRUE    },
    {   "charm",         	IMMAPP_CHARM,            TRUE    },
    {   "magic",         	IMMAPP_MAGIC,            	TRUE    },
    {   "weapon",        	IMMAPP_WEAPON,        	TRUE    },
    {   "bash",          	IMMAPP_BASH,             	TRUE    },
    {   "pierce",        	IMMAPP_PIERCE,           	TRUE    },
    {   "slash",         	IMMAPP_SLASH,            	TRUE    },
    {   "fire",          		IMMAPP_FIRE,             	TRUE    },
    {   "cold",          		IMMAPP_COLD,             	TRUE    },
    {   "lightning",     	IMMAPP_LIGHTNING,    	TRUE    },
    {   "acid",          		IMMAPP_ACID,             	TRUE    },
    {   "poison",        	IMMAPP_POISON,           	TRUE    },
    {   "negative",      	IMMAPP_NEGATIVE,       TRUE    },
    {   "holy",          	IMMAPP_HOLY,             	TRUE    },
    {   "energy",        	IMMAPP_ENERGY,           	TRUE    },
    {   "mental",        	IMMAPP_MENTAL,         	TRUE    },
    {   "disease",       	IMMAPP_DISEASE,          TRUE    },
    {   "drowning",      	IMMAPP_DROWNING,    	TRUE    },
    {   "light",         		IMMAPP_LIGHT,            	TRUE    },
    {   "bullets",      	IMMAPP_BULLETS,          TRUE    },
    {   "mask",       		IMMAPP_MASK,          	TRUE    },
    {   "old",       		IMMAPP_OLD,          	TRUE    },
    {   "sound",     		IMMAPP_SOUND,          	TRUE    },
    {   "",          0,            0    }
};

const struct flag_type res_flags[] =
{
    {   "charm",         	RES_CHARM,            	TRUE    },
    {   "magic",         	RES_MAGIC,            	TRUE    },
    {   "weapon",        	RES_WEAPON,           	TRUE    },
    {   "bash",          	RES_BASH,             	TRUE    },
    {   "pierce",        	RES_PIERCE,           	TRUE    },
    {   "slash",         	RES_SLASH,            	TRUE    },
    {   "fire",          		RES_FIRE,             	TRUE    },
    {   "cold",          		RES_COLD,             	TRUE    },
    {   "lightning",     	RES_LIGHTNING,        	TRUE    },
    {   "acid",          		RES_ACID,             	TRUE    },
    {   "poison",        	RES_POISON,           	TRUE    },
    {   "negative",      	RES_NEGATIVE,         	TRUE    },
    {   "holy",          	RES_HOLY,             	TRUE    },
    {   "energy",        	RES_ENERGY,           	TRUE    },
    {   "mental",        	RES_MENTAL,           	TRUE    },
    {   "disease",       	RES_DISEASE,          	TRUE    },
    {   "drowning",      	RES_DROWNING,         	TRUE    },
    {   "light",         		RES_LIGHT,            	TRUE    },
    {   "bullets",       	RES_BULLETS,          	TRUE    },
    {   "mask",       		RES_MASK,          	TRUE    },
    {   "old",       		RES_OLD,          		TRUE    },
    {   "sound",     		RES_SOUND,      		TRUE    },
   {   "",          0,            0    }
};


const struct flag_type vuln_flags[] =
{
    {   "magic",         	VULN_MAGIC,           	TRUE    },
    {   "weapon",        	VULN_WEAPON,          	TRUE    },
    {   "bash",          	VULN_BASH,            	TRUE    },
    {   "pierce",        	VULN_PIERCE,          	TRUE    },
    {   "slash",         	VULN_SLASH,           	TRUE    },
    {   "fire",          		VULN_FIRE,            	TRUE    },
    {   "cold",          		VULN_COLD,            	TRUE    },
    {   "lightning",     	VULN_LIGHTNING,       	TRUE    },
    {   "acid",          		VULN_ACID,            	TRUE    },
    {   "poison",        	VULN_POISON,          	TRUE    },
    {   "negative",      	VULN_NEGATIVE,        	TRUE    },
    {   "holy",          	VULN_HOLY,            	TRUE    },
    {   "energy",        	VULN_ENERGY,          	TRUE    },
    {   "mental",        	VULN_MENTAL,          	TRUE    },
    {   "disease",       	VULN_DISEASE,         	TRUE    },
    {   "drowning",      	VULN_DROWNING,        	TRUE    },
    {   "light",         		VULN_LIGHT,           	TRUE    },
    {   "wood",          	VULN_WOOD,            	TRUE    },
    {   "silver",        		VULN_SILVER,          	TRUE    },
    {   "iron",          		VULN_IRON,            	TRUE    },
    {   "mithril",	 	VULN_MITHRIL,	       	TRUE    },
    {   "adamantite",	VULN_ADAMANTITE,     TRUE    },
    {   "steel",	 	VULN_STEEL,	       	TRUE    },
    {   "aluminium",	VULN_ALUMINIUM,	TRUE    },
    {   "copper",	 	VULN_COPPER,	       	TRUE    },
    {   "old",	 	VULN_OLD,	       	TRUE    },
    {   "sound",	 	VULN_SOUND,	       	TRUE    },
    {   "",              0,                    0       }
};


/* Also in const.c */

const struct flag_type material_type[] =    
{
    {   "none", 	         0,            			TRUE    },
    {   "wood",		MATERIAL_WOOD,		TRUE	},
    {   "iron",		MATERIAL_IRON,		TRUE	},
    {   "silver",               	MATERIAL_SILVER,		TRUE	},
    {   "gold",		MATERIAL_GOLD,		TRUE	},
    {   "adamantite",	MATERIAL_ADAMANTITE,	TRUE	},
    {   "cloth",		MATERIAL_CLOTH,		TRUE	},
    {   "glass",		MATERIAL_GLASS ,     		TRUE	},
    {   "food",		MATERIAL_FOOD  , 		TRUE    },
    {   "liquid",		MATERIAL_LIQUID, 		TRUE    },
    {   "mithril",		MATERIAL_MITHRIL,		TRUE    },
    {   "steel",		MATERIAL_STEEL , 		TRUE    },
    {   "paper",		MATERIAL_PAPER , 		TRUE    },
    {   "meat",		MATERIAL_MEAT  , 		TRUE    },
    {   "flesh",		MATERIAL_FLESH , 		TRUE    },
    {   "leather",		MATERIAL_LEATHER,		TRUE    },
    {   "pill",		MATERIAL_PILL   ,		TRUE    },
    {   "vellum",		MATERIAL_VELLUM ,		TRUE    },
    {   "bronze",		MATERIAL_BRONZE ,		TRUE    },
    {   "brass",                	MATERIAL_BRASS  ,		TRUE    },
    {   "stone",                	MATERIAL_STONE  ,		TRUE    },
    {   "bone",		MATERIAL_BONE	,	TRUE	},
    {   "unique",       	MATERIAL_UNIQUE ,		TRUE    },
    {   "crystal",		MATERIAL_CRYSTAL,		TRUE    },
    {   "diamond",		MATERIAL_DIAMOND,		TRUE	},
    {   "fur",		MATERIAL_FUR,		TRUE	},
    {   "plastic",		MATERIAL_PLASTIC,		TRUE	},
    {   "marble",		MATERIAL_MARBLE,		TRUE	},
    {   "granite",		MATERIAL_GRANITE,		TRUE	},
    {   "rubber",		MATERIAL_RUBBER,		TRUE	},
    {   "ivory",		MATERIAL_IVORY,		TRUE	},
    {   "earth",	        	MATERIAL_EARTH,		TRUE	},
    {   "air",		MATERIAL_AIR,		TRUE	},
    {   "fire",		MATERIAL_FIRE,		TRUE	},
    {   "water",	        	MATERIAL_WATER,		TRUE	},
    {   "sandstone",	MATERIAL_SANDSTONE,	TRUE	},
    {   "limestone",		MATERIAL_LIMESTONE,	TRUE	},
    {   "chalk",	        	MATERIAL_CHALK,		TRUE	},
    {   "hair",		MATERIAL_HAIR,		TRUE	},
    {   "skin",		MATERIAL_SKIN,		TRUE	},
    {   "silk",		MATERIAL_SILK,		TRUE	},
    {   "reptile leather",	MATERIAL_REPTILE_LEATHER,	TRUE	},
    {   "chitin",		MATERIAL_CHITIN,		TRUE	},
    {   "leathery skin",	MATERIAL_LEATHERY_SKIN,	TRUE	},
    {   "feathers",		MATERIAL_FEATHERS,		TRUE	},
    {   "protoplasma",	MATERIAL_PROTOPLASMA,	TRUE	},
    {   "scales",		MATERIAL_SCALES,		TRUE	},
    {   "plant",		MATERIAL_PLANT,		TRUE	},
    {   "pearl",	                MATERIAL_PEARL,		TRUE	},
    {   "essence of time",	MATERIAL_TIME,		TRUE	},
    {   "aluminium",	MATERIAL_ALUMINIUM,	TRUE	},
    {   "brick",		MATERIAL_BRICK,		TRUE	},
    {   "copper",		MATERIAL_COPPER,		TRUE	},



    {   "",             	 0,            			0       }
};

const struct flag_type material_string[] =    
{
    {"a boring",		0,            				TRUE    },
    {"wooden",		MATERIAL_WOOD,		TRUE	},
    {"iron",		MATERIAL_IRON,		TRUE	},
    {"{Wsilver{x",                 MATERIAL_SILVER,		TRUE	},
    {"{ygolden{x",		MATERIAL_GOLD,		TRUE	},
    {"adamantite",        	MATERIAL_ADAMANTITE,	TRUE	},
    {"cloth",                            MATERIAL_CLOTH,		TRUE	},
    {"{Cglass{x",                   	MATERIAL_GLASS ,     		TRUE	},
    {"edible",                          	MATERIAL_FOOD  , 		TRUE    },
    {"{Bliquid{x",                  	MATERIAL_LIQUID, 		TRUE    },
    {"{Wmithril{x",         	MATERIAL_MITHRIL,		TRUE    },
    {"steel",                             MATERIAL_STEEL , 		TRUE    },
    {"paper",	               	MATERIAL_PAPER , 		TRUE    },
    {"meaty",                         	MATERIAL_MEAT  , 		TRUE    },
    {"fleshy",                         	MATERIAL_FLESH , 		TRUE    },
    {"leather",                        	MATERIAL_LEATHER,		TRUE    },
    {"strange",                      	MATERIAL_PILL   ,		TRUE    },
    {"vellum",                         	MATERIAL_VELLUM ,		TRUE    },
    {"{Mbronze{x",               	MATERIAL_BRONZE ,		TRUE    },
    {"brass",                           	MATERIAL_BRASS  ,		TRUE    },
    {"stone",                          	MATERIAL_STONE  ,		TRUE    },
    {"bone",		MATERIAL_BONE	,	TRUE	},
    {"unique",                         MATERIAL_UNIQUE ,		TRUE    },
    {"{ccrystal{x",       	MATERIAL_CRYSTAL,		TRUE    },
    {"{Cdiamond{x",   	MATERIAL_DIAMOND,		TRUE	},
    {"furry",		MATERIAL_FUR,		TRUE	},
    {"plastic",		MATERIAL_PLASTIC,		TRUE	},
    {"marble",		MATERIAL_MARBLE,		TRUE	},
    {"granite",		MATERIAL_GRANITE,		TRUE	},
    {"rubber",		MATERIAL_RUBBER,		TRUE	},
    {"ivory",		MATERIAL_IVORY,		TRUE	},
    {"earthen",		MATERIAL_EARTH,		TRUE	},
    {"{cairy{x",		MATERIAL_AIR,		TRUE	},
    {"{rhot{x",		MATERIAL_FIRE,		TRUE	},
    {"{bwet{x",		MATERIAL_WATER,		TRUE	},
    {"sandstone",		MATERIAL_SANDSTONE,	TRUE	},
    {"limestone",		MATERIAL_LIMESTONE,	TRUE	},
    {"chalk",		MATERIAL_CHALK,		TRUE	},
    {"hairy",		MATERIAL_HAIR,		TRUE	},
    {"skin",   		MATERIAL_SKIN,		TRUE	},
    {"silky",		MATERIAL_SILK,		TRUE	},
    {"reptilian leather",	MATERIAL_REPTILE_LEATHER,	TRUE	},
    {"chitinous",		MATERIAL_CHITIN,		TRUE	},
    {"leathery",		MATERIAL_LEATHERY_SKIN,	TRUE	},
    {"feathered",		MATERIAL_FEATHERS,		TRUE	},
    {"slimy",		MATERIAL_PROTOPLASMA,	TRUE	},
    {"scale",		MATERIAL_SCALES,		TRUE	},
    {"{ggreen{x",		MATERIAL_PLANT,		TRUE	},
    {"{Wpearl{x",	                MATERIAL_PEARL,		TRUE	},
    {"phasing",	                MATERIAL_TIME,		TRUE	},
    {"{Caluminium{x",	MATERIAL_ALUMINIUM,	TRUE	},
    {"brick",		MATERIAL_BRICK,		TRUE	},
    {"{Rcopper{x",		MATERIAL_COPPER,		TRUE	},
    {   "",			0,            			0       }
};


const struct flag_type position_flags[] =
{
    {   "dead",           	POS_DEAD,            	FALSE   },
    {   "mortal",         	POS_MORTAL,          	FALSE   },
    {   "incap",          	POS_INCAP,           	FALSE   },
    {   "stunned",        	POS_STUNNED,         	FALSE   },
    {   "sleeping",       	POS_SLEEPING,        	TRUE    },
    {   "resting",        	POS_RESTING,         	TRUE    },
    {   "sitting",        	POS_SITTING,         	TRUE    },
    {   "fighting",       	POS_FIGHTING,        	FALSE   },
    {   "standing",       	POS_STANDING,        	TRUE    },
    {   "",              0,                    0       }
};


const struct flag_type mob_nature[] =
{
    {	"strong",		NATURE_STRONG,		TRUE	},
    {	"feeble",			NATURE_FEEBLE,		TRUE	},
    {	"smart",			NATURE_SMART,		TRUE	},
    {   	"dumb",			NATURE_DUMB,		TRUE    },   
    {	"agile",			NATURE_AGILE,		TRUE	},
    {	"lumbering",		NATURE_LUMBERING,		TRUE	},
    {	"robust",		NATURE_ROBUST,		TRUE	},
    {	"sickly",			NATURE_SICKLY,		TRUE	},
    {	"sly",			NATURE_SLY,			TRUE	},
    {	"gullible",		NATURE_GULLIBLE,		TRUE	},
    {	"sturdy",		NATURE_STURDY,		TRUE	},
    {	"fragile",		NATURE_FRAGILE,		TRUE	},
    {	"magical",		NATURE_MAGICAL,		TRUE	},
    {	"mundain",		NATURE_MUNDAIN,		TRUE	},
    {	"viscious",		NATURE_VISCIOUS,		TRUE	},
    {	"harmless",		NATURE_HARMLESS,		TRUE	},
    {	"armoured",		NATURE_ARMOURED,		TRUE	},
    {	"exposed",		NATURE_EXPOSED,		TRUE	},
    {	"monsterous",		NATURE_MONSTEROUS,	TRUE	},
    {	"lucky",			NATURE_LUCKY,		TRUE	},
    {	"unlucky",		NATURE_UNLUCKY,		TRUE	},
    {	"charismatic",		NATURE_CHARISMATIC,	TRUE	},
    {	"disgusting",		NATURE_DISGUSTING,		TRUE	},
    {	"",			0,			0	}
};

const struct flag_type area_climate[] =
{
    {	"european",		CLIMATE_EUROPEAN,	TRUE	},
    {	"desert",		CLIMATE_DESERT,	TRUE	},
    {	"rainy",			CLIMATE_RAINY,	TRUE	},
    {          "arctic",			CLIMATE_ARCTIC,	TRUE    },   
    {	"",			0,			0	}
};

const struct flag_type time_sub[] =
{
    {	"pulse_1",		SUB_PULSE_1,		TRUE	},
    {	"pulse_3",		SUB_PULSE_3,		TRUE	},
    {	"pulse_4",		SUB_PULSE_4,		TRUE	},
    {	"pulse_5",		SUB_PULSE_5,		TRUE	},
    {	"pulse_10",		SUB_PULSE_10,		TRUE	},
    {	"pulse_30",		SUB_PULSE_30,		TRUE	},
    {	"pulse_AREA",		SUB_PULSE_AREA,	TRUE	},
    {	"time_hour",		SUB_TIME_HOUR,	TRUE	},
    {	"time_day",		SUB_TIME_DAY,	TRUE	},
    {	"time_dawn",		SUB_TIME_DAWN,	TRUE	},
    {	"time_sunrise",		SUB_TIME_SUNRISE,	TRUE	},
    {	"time_sunset",		SUB_TIME_SUNSET,	TRUE	},
    {	"time_dusk",		SUB_TIME_DUSK,	TRUE	},
    {	"",			0,			0	}
};


const struct flag_type soc_auth[] =
{
    {	"invite",		SOC_AUTH_INVITE,		TRUE	},
    {	"expel",		SOC_AUTH_EXPEL,		TRUE	},
    {	"bank",		SOC_AUTH_BANK,		TRUE	},
    {	"auth",		SOC_AUTH_AUTH,		TRUE	},
    {	"promote",	SOC_AUTH_PROMOTE,		TRUE	},
    {	"demote",	SOC_AUTH_DEMOTE,		TRUE	},
    {	"test",		SOC_AUTH_TEST,		TRUE	},
    {	"foe",		SOC_AUTH_FOE,		TRUE	},
    {	"pardon",	SOC_AUTH_PARDON,		TRUE	},
    {	"tax",		SOC_AUTH_TAX,		TRUE	},
    {	"",			0,			0	}
};


const struct flag_type instrument_type[] =
{
    {	"vocal",		INSTR_VOCAL,			TRUE	},
    {	"percussion",	INSTR_PERCUSSION,		TRUE	},
    {	"strings",	INSTR_STRINGS,		TRUE	},
    {	"flute",		INSTR_FLUTE,			TRUE	},
    {	"brass",		INSTR_BRASS,			TRUE	},
    {	"piano",		INSTR_PIANO,			TRUE	},
    {	"organ",		INSTR_ORGAN,			TRUE	},
    {	"sound-crystal",	INSTR_CRYSTAL,		TRUE	},
    {	"",		0,		0	}
};


const struct flag_type pass_type[] =
{
    {	"normal",	0,		TRUE	},
    {	"vip",		1,		TRUE	},
    {	"diplomatic",	2,		TRUE	},
    {	"",		0,		0	}
};


const struct flag_type crime_type[] =
{
    {	"vagabondage",	1,		TRUE	},
    {	"theft",		2,		TRUE	},
    {	"murder",	3,		TRUE	},
    {	"",		0,		0	}
};



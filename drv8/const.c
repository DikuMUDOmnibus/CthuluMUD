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
#include "magic.h"
#include "fight.h"

/* Zeran - administrative staff table (useful for note to admin) */


/* Zeran - material table */

/* Also in bit.c */

const struct material_data material_table [] =
{
/*{   material name,  	material_type,		vuln_flag		} */
  {  "wood",	 	MATERIAL_WOOD, 		VULN_WOOD		},
  {  "iron",		MATERIAL_IRON, 		VULN_IRON		},
  {  "silver",		MATERIAL_SILVER,		VULN_SILVER		},
  {  "gold",		MATERIAL_GOLD,		0		  	},
  {  "adamantite",		MATERIAL_ADAMANTITE,	VULN_ADAMANTITE	},
  {  "cloth",		MATERIAL_CLOTH,		0			},
  {  "glass",		MATERIAL_GLASS,		0			},
  {  "food", 		MATERIAL_FOOD,		0			},
  {  "liquid",		MATERIAL_LIQUID,		0			},
  {  "mithril",		MATERIAL_MITHRIL,		VULN_MITHRIL		},
  {  "steel",		MATERIAL_STEEL,		VULN_STEEL		},
  {  "paper",		MATERIAL_PAPER,		0			},
  {  "meat",		MATERIAL_MEAT,		0			},
  {  "flesh",		MATERIAL_FLESH,		0			},
  {  "leather",		MATERIAL_LEATHER,		0			},
  {  "pill",		MATERIAL_PILL,		0			},
  {  "vellum",		MATERIAL_VELLUM,		0			},
  {  "bronze",		MATERIAL_BRONZE,		0			},
  {  "brass",		MATERIAL_BRASS,		0			},
  {  "stone",		MATERIAL_STONE,		0			},
  {  "bone",		MATERIAL_BONE,		0			},
  {  "unique",		MATERIAL_UNIQUE,		0			},
  {  "crystal",		MATERIAL_CRYSTAL,		0			},
  {  "diamond",		MATERIAL_DIAMOND,		0			},
  {  "fur",		MATERIAL_FUR,		0			},
  {  "plastic",		MATERIAL_PLASTIC,		0			},
  {  "marble",		MATERIAL_MARBLE,		0			},
  {  "granite",		MATERIAL_GRANITE,		0			},
  {  "rubber",		MATERIAL_RUBBER,		0			},
  {  "ivory",		MATERIAL_IVORY,		0			},
  {  "earth",		MATERIAL_EARTH,		0			},
  {  "air",			MATERIAL_AIR,		0			},
  {  "fire",		MATERIAL_FIRE,		0			},
  {  "water",		MATERIAL_WATER,		0			},
  {  "sandstone",		MATERIAL_SANDSTONE,	0			},
  {  "limestone",		MATERIAL_LIMESTONE,	0			},
  {  "chalk",		MATERIAL_CHALK,		0			},
  {  "hair",		MATERIAL_HAIR,		0			},
  {  "skin",		MATERIAL_SKIN,		0			},
  {  "silk",		MATERIAL_SILK,		0			},
  {  "reptile leather", 	MATERIAL_REPTILE_LEATHER,  0			},
  {  "chitin",		MATERIAL_CHITIN,		0			},
  {  "leathery skin",	MATERIAL_LEATHERY_SKIN,	0			},
  {  "feathers",		MATERIAL_FEATHERS,		0			},
  {  "protoplasma",	MATERIAL_PROTOPLASMA,	VULN_NEGATIVE	},
  {  "scales",	        	MATERIAL_SCALES,		0			},
  {  "plant",		MATERIAL_PLANT,		0			},
  {  "pearl",	                MATERIAL_PEARL,		0			},
  {  "essence of time",	MATERIAL_TIME,		0			},
  {  "aluminium",	                MATERIAL_ALUMINIUM,	VULN_ALUMINIUM	},
  {  "brick",	                MATERIAL_BRICK,		0			},
  {  "copper",	                MATERIAL_COPPER,		VULN_COPPER		},
  {  "unknown",		 0,			0			}
};
 

/* Zeran - condition table */
const char *cond_table []   =
{
	"perfect",
	"almost perfect",
	"slightly worn",
	"moderately worn",
	"heavily worn",
	"badly worn",
	"barely usable",
	"worthless"
};
	
/* Zeran - size name table */
const char *size_table []   =
{
	"tiny",
	"small",
	"medium",
	"large",
	"huge",
	"giant"
};

/* See also bit.c: damage_type, attack_type */

/* attack table  -- not very organized :( */
const 	struct attack_type	attack_table	[]		=
{
    {"hit",			DAM_BASH	},  /*  0 */
    {"slice", 		DAM_SLASH	},	
    {"stab",		DAM_PIERCE	},
    {"slash",		DAM_SLASH	},
    {"whip",		DAM_SLASH	},
    {"claw",		DAM_SLASH	},  /*  5 */
    {"blast",		DAM_BASH	},
    {"pound",		DAM_BASH	},
    {"crush",		DAM_BASH	},
    {"grep",		DAM_OTHER	},
    {"bite",		DAM_PIERCE	},  /* 10 */
    {"pierce",		DAM_PIERCE	},
    {"suction",		DAM_BASH	},
    {"beating",		DAM_BASH	},
    {"digestion",		DAM_ACID        },
    {"charge",		DAM_BASH	},  /* 15 */
    {"slap",		DAM_BASH	},
    {"punch",		DAM_BASH	},
    {"wrath",		DAM_NEGATIVE	},
    {"magic",		DAM_ENERGY	},
    {"divine power",	DAM_HOLY	},  /* 20 */
    {"cleave",		DAM_PIERCE	},
    {"scratch",		DAM_SLASH	},
    {"peck",		DAM_PIERCE	},
    {"peck",		DAM_BASH	},
    {"chop",		DAM_SLASH	},  /* 25 */
    {"sting",		DAM_PIERCE	},
    {"smash",		DAM_BASH	},
    {"shocking bite",	DAM_LIGHTNING	},
    {"flaming bite", 	DAM_FIRE	},
    {"freezing bite", 	DAM_COLD	},  /* 30 */
    {"acidic bite", 		DAM_ACID	},
    {"chomp",		DAM_BASH	},
    {"shot",                 	DAM_SHOT        },
    {"life drain", 		DAM_NEGATIVE 	},
    {"thirst",		DAM_OTHER 	},  /* 35 */
    {"slime",		DAM_ACID 	},  
    {"thwack", 		DAM_BASH 	},  
    {"flame",		DAM_FIRE	},
    {"chill",		DAM_COLD	},
    {"elder power",		DAM_OLD	},
    {"screech",		DAM_SOUND	}
};


/*
 * Class table - compatibility use only
 */
const	struct	class_type	class_table	[MAX_CLASS]	=
{
    {
	"occultist", "Ocu"
    },

    {
	"priest", "Pri"
    },

    {
	"archaeologist", "Arc"
    },

    {
	"soldier", "Sol"
    },


    {
        "chaosmage", "Cha"
    },

    {
        "monk", "Mon"
    },

    {
	"defiler", "Def"
    }

};


/*
 * Attribute bonus tables.
 */
const	struct	str_app_type	str_app		[112]		=
{
/*  	To Hit	To Dam	Weight	Wield	*/
/*	------	------	------	-----	*/
    { 	-5,	 -4,	   0,	  0	},  /*   0  */
    { 	-5,	 -4,	   1,	  1	}, 
    { 	-5,	 -4,	   2,	  1	}, 
    { 	-5,	 -4,	   2,	  1	}, 
    { 	-5,	 -4,	   3,	  1	}, 
    { 	-5,	 -4,	   3,	  1	}, 
    { 	-5,	 -4,	   4,	  1	}, 
    { 	-5,	 -4,	   5,	  1	}, 
    { 	-3,	 -2,	   6,	  2	},
    { 	-3,	 -2,	   7,	  2	},
    { 	-3,	 -2,	   8,	  2	},
    { 	-3,	 -2,	   9,	  2	},
    { 	-3,	 -1,	  10,	  3	}, 
    { 	-3,	 -1,	  13,	  3	}, 
    { 	-3,	 -1,	  16,	  3	}, 
    { 	-3,	 -1,	  20,	  3	}, 
    { 	-2,	 -1,	  25,	  4	},
    { 	-2,	 -1,	  31,	  4	},
    { 	-2,	 -1,	  38,	  4	},
    { 	-2,	 -1,	  47,	  4	},
    { 	-2,	 -1,	  55,	  5	},  /*  20 */
    { 	-2,	 -1,	  62,	  5	}, 
    { 	-2,	 -1,	  69,	  5	}, 
    { 	-2,	 -1,	  75,	  5	}, 
    { 	-1,	  0,	  80,	  6	},
    { 	-1,	  0,	  83,	  6	},
    { 	-1,	  0,	  85,	  6	},
    { 	-1,	  0,	  88,	  6	},
    { 	-1,	  0,	  90,	  7	},
    { 	-1,	  0,	  93,	  7	},
    { 	-1,	  0,	  95,	  7	},
    { 	-1,	  0,	  98,	  7	},
    { 	 0,	  0,	 100,	  8	},
    { 	 0,	  0,	 102,	  8	},
    { 	 0,	  0,	 104,	  8	},
    { 	 0,	  0,	 108,	  8	},
    { 	 0,	  0,	 110,	  9	},
    { 	 0,	  0,	 112,	  9	},
    { 	 0,	  0,	 113,	  9	},
    { 	 0,	  0,	 114,	  9	},
    { 	 0,	  0,	 115,	 10	}, /*  40  */
    { 	 0,	  0,	 116,	 10	}, 
    {	 0,	  0,	 118,	 10	}, 
    {	 0,	  0,	 120,	 10	}, 
    {	 0,	  0,	 122,	 11	},
    { 	 0,	  0,	 124,	 11	},
    { 	 0,	  0,	 126,	 11	},
    { 	 0,	  0,	 128,	 11	},
    { 	 0,	  0,	 130,	 12	},
    { 	 0,	  0,	 131,	 12	},
    { 	 0,	  0,	 132,	 12	}, /* 50 */
    { 	 0,	  0,	 134,	 12	},
    { 	 0,	  0,	 135,	 13	}, 
    { 	 0,	  0,	 136,	 13	}, 
    { 	 0,	  0,	 138,	 13	}, 
    { 	 0,	  0,	 139,	 13	}, 
    { 	 0,	  1,	 140,	 14	},
    { 	 0,	  1,	 143,	 14	},
    { 	 0,	  1,	 145,	 14	},
    { 	 0,	  1,	 148,	 14	},
    { 	 1,	  1,	 150,	 15	}, /*  60  */
    { 	 1,	  1,	 153,	 15	}, 
    { 	 1,	  1,	 157,	 16	}, 
    { 	 1,	  1,	 161,	 17	}, 
    { 	 1,	  2,	 165,	 18	},
    { 	 1,	  2,	 168,	 19	},
    { 	 1,	  2,	 172,	 20	},
    { 	 1,	  2,	 176,	 21	},
    { 	 2,	  3,	 180,	 22	},
    { 	 2,	  3,	 185,	 23	},
    { 	 2,	  3,	 190,	 24	}, /* 70 */
    { 	 2,	  3,	 195,	 25	},
    { 	 2,	  3,	 200,	 26	}, 
    { 	 2,	  3,	 206,	 27	}, 
    { 	 2,	  3,	 212,	 28	}, 
    { 	 2,	  3,	 218,	 29	}, 
    { 	 3,	  4,	 225,	 30	},
    { 	 3,	  4,	 231,	 31	},
    { 	 3,	  4,	 237,	 33	},
    { 	 3,	  4,	 244,	 34	},
    { 	 3,	  5,	 250,	 35	}, /*  80  */
    { 	 3,	  5,	 262,	 36	}, 
    { 	 3,	  5,	 275,	 38	}, 
    { 	 3,	  5,	 287,	 39	}, 
    { 	 4,	  6,	 300,	 40	},
    { 	 4,	  6,	 312,	 41	},
    { 	 4,	  6,	 325,	 43	},
    { 	 4,	  6,	 337,	 44	},
    { 	 4,	  6,	 350,	 45	},
    { 	 4,	  6,	 362,	 46	},
    { 	 4,	  6,	 375,	 48	},
    { 	 4,	  6,	 387,	 49	},
    { 	 5,	  7,	 400,	 50	},
    { 	 5,	  7,	 412,	 51	},
    { 	 5,	  7,	 425,	 53	},
    { 	 5,	  7,	 437,	 54	},
    { 	 5,	  8,	 450,	 55	},
    { 	 5,	  8,	 462,	 56	},
    { 	 5,	  8,	 475,	 58	},
    { 	 5,	  8,	 487,	 59	}, 
    { 	 6,	  9,	 500,	 60	}, /* 100  */
    { 	 6,	  9,	 550,	 62	}, 
    { 	 7,	 10,	 600,	 64	}, 
    { 	 7,	 10,	 700,	 66	},  /* 103!! */
    { 	 8,	 11,	 800,	 70	},
    { 	 9,	 12,	 900,	 74	}, /* 105 */
    { 	10,	 14,	1000,	 80	},
    { 	12,	 16,	1150,	 88	},
    { 	14,	 19,	1300,	 98	},
    { 	16,	 22,	1500,	110	}, 
    { 	19,	 25,	2000,	125	},
    { 	22,	 30,	2500,	140	}  /* 111  */
};



const	struct	int_app_type	int_app		[112]		=
{
/*	Learn	*/
/*	-----	*/
    { 	 4 },	  /*   0           =  1 */
    { 	 4 },	  /*   1           =  1 */
    { 	 4 },	  /*   2           =  1 */
    { 	 4 },	  /*   3           =  1 */
    { 	 5 },	  /*   4           =  1 */
    { 	 5 },	  /*   5           =  1 */
    { 	 6 },	  /*   6           =  1 */
    { 	 6 },	  /*   7           =  1 */
    { 	 7 },     /*   8           =  1 */
    { 	 7 },     /*   9           =  1 */
    { 	 7 },     /*  10           =  1 */
    { 	 7 },     /*  11           =  1 */
    { 	 8 },	  /*  12           =  2 */
    { 	 8 },	  /*  13           =  2 */
    { 	 8 },	  /*  14           =  2 */
    { 	 8 },	  /*  15           =  2 */
    { 	 9 },     /*  16           =  2 */      
    { 	 9 },     /*  17           =  2 */      
    { 	 9 },     /*  18           =  2 */      
    { 	 9 },     /*  19           =  2 */      
    { 	10 },	  /*  20           =  2 */
    { 	10 },	  /*  21           =  2 */
    { 	10 },	  /*  22           =  2 */
    { 	10 },	  /*  23           =  2 */
    { 	11 },     /*  24 very low  =  2 */
    { 	11 },     /*  25 very low  =  2 */
    { 	11 },     /*  26 very low  =  2 */
    { 	11 },     /*  27 very low  =  2 */
    { 	12 },     /*  28 low       =  3 */ 
    { 	12 },     /*  28 low       =  3 */ 
    { 	12 },     /*  30 low       =  3 */ 
    { 	12 },     /*  31 low       =  3 */ 
    { 	13 },     /*  32 low       =  3 */
    { 	14 },     /*  33 low       =  3 */
    { 	14 },     /*  34 low       =  3 */
    { 	15 },     /*  35 low       =  3 */
    { 	16 },     /*  36 poor      =  4 */ 
    { 	16 },     /*  37 poor      =  4 */ 
    { 	16 },     /*  38 poor      =  4 */ 
    { 	16 },     /*  39 poor      =  4 */ 
    { 	17 },	  /*  40           =  4 */
    { 	17 },	  /*  41           =  4 */
    { 	18 },	  /*  42           =  4 */
    { 	18 },	  /*  43           =  4 */
    { 	19 },     /*  44 poor      =  4 */
    { 	19 },     /*  45 poor      =  4 */
    { 	20 },     /*  46 poor      =  4 */
    { 	21 },     /*  47 poor      =  4 */
    { 	21 },     /*  48 average   =  5 */     
    { 	22 },     /*  49 average   =  5 */     
    { 	22 },     /*  50 average   =  5 */     
    { 	23 },     /*  51 average   =  5 */     
    { 	23 },     /*  52 average   =  5 */
    { 	24 },     /*  53 average   =  5 */
    { 	25 },     /*  54 average   =  5 */
    { 	25 },     /*  55 average   =  5 */
    { 	26 },     /*  56 good      =  6 */
    { 	27 },     /*  57 good      =  6 */
    { 	29 },     /*  58 good      =  6 */
    { 	30 },     /*  59 good      =  6 */
    { 	31 },	  /*  60           =  7 */
    { 	32 },	  /*  61           =  7 */
    { 	32 },	  /*  62           =  7 */
    { 	33 },	  /*  63           =  7 */
    { 	34 },     /*  64 good      =  8 */
    { 	35 },     /*  65 good      =  8 */
    { 	35 },     /*  66 good      =  8 */
    { 	36 },     /*  67 good      =  8 */
    { 	37 },     /*  68 high      =  9 */
    { 	38 },     /*  69 high      =  9 */
    { 	38 },     /*  70 high      =  9 */
    { 	39 },     /*  71 high      =  9 */
    { 	40 },	  /*  72 high      = 10 */
    { 	41 },	  /*  73 high      = 10 */
    { 	42 },	  /*  74 high      = 10 */
    { 	43 },	  /*  75 high      = 10 */
    { 	44 },     /*  76           = 11 */
    { 	45 },     /*  77           = 11 */
    { 	47 },     /*  78           = 11 */
    { 	48 },     /*  79           = 11 */
    { 	49 },	  /*  80           = 12 */
    { 	49 },	  /*  81           = 12 */
    { 	50 },	  /*  82           = 12 */
    { 	51 },	  /*  83           = 12 */
    { 	52 },     /*  84           = 13 */
    { 	53 },     /*  85           = 13 */
    { 	54 },     /*  86           = 13 */
    { 	55 },     /*  87           = 13 */
    { 	56 },     /*  88           = 14 */
    { 	57 },     /*  89           = 14 */
    { 	58 },     /*  90           = 14 */
    { 	59 },     /*  91           = 14 */
    { 	60 },     /*  92           = 15 */
    { 	61 },     /*  93           = 15 */
    { 	62 },     /*  94           = 15 */
    { 	63 },     /*  95           = 15 */
    { 	64 },     /*  96           = 16 */
    { 	65 },     /*  97           = 16 */
    { 	66 },     /*  98           = 16 */
    { 	67 },     /*  99           = 16 */
    { 	68 },	  /* 100 very high = 17 */
    { 	69 },	  /* 101 very high = 17 */
    { 	70 },	  /* 102 very high = 17 */
    { 	72 },	  /* 103 very high = 17 */
    { 	74 },     /* 104 */
    { 	76 },     /*  105*/
    { 	79 },     /*  106 */
    { 	82 },     /*  107 */
    { 	85 },	  /* 108 */
    { 	89 },	  /* 109 */
    { 	93 },
    { 	98 }	  /* 111 */
};	



const	struct	wis_app_type	wis_app		[112]		=
{
/*	Practice	*/
/*	--------	*/
    {	 1	 },	/*  0 */
    {	 1	 },	/*  1 */
    {	 1	 },	/*  2 */
    {	 1	 },	/*  3 */
    {	 1	 },	/*  4 */
    {	 1	 },	/*  5 */
    {	 1	 },	/*  6 */
    {	 1	 },	/*  7 */
    {	 1	 },
    {	 1	 },
    {	 1	 },
    {	 1	 },
    {	 1	 },	/* 12 */
    {	 1	 },	/* 13 */
    {	 1	 },	/* 14 */
    {	 1	 },	/* 15 */
    {	 1	 },
    {	 1	 },
    {	 1	 },
    {	 1	 },
    {	 1	 },	/* 20 */
    {	 1	 },	/* 21 */
    {	 1	 },	/* 22 */
    {	 1	 },	/* 23 */
    {	 1	 },
    {	 1	 },
    {	 1	 },
    {	 1	 },
    {	 1	 },      /* low */
    {	 1	 },      /* low */
    {	 1	 },      /* low */
    {	 1	 },      /* low */
    {	 1	 },
    {	 1	 },
    {	 1	 },
    {	 1	 },
    {	 2	 },      /* poor */
    {	 2	 },      /* poor */
    {	 2	 },      /* poor */
    {	 2	 },      /* poor */
    {	 2	 },	/* 40 */
    {	 2	 },	/* 41 */
    {	 2	 },	/* 42 */
    {	 2	 },	/* 43 */
    {	 2	 },
    {	 2	 },
    {	 2	 },
    {	 2	 },
    {	 2	 },      /* average */
    {	 2	 },      /* average */
    {	 2	 },      /* average */
    {	 2	 },      /* average */
    {	 2	 },
    {	 2	 },
    {	 2	 },
    {	 2	 },
    {	 3	 },      /* good */
    {	 3	 },      /* good */
    {	 3	 },      /* good */
    {	 3	 },      /* good */
    {	 3	 },	/* 60 */
    {	 3	 },	/* 61 */
    {	 3	 },	/* 62 */
    {	 3	 },	/* 63 */
    {	 3	 },      /* high */
    {	 3	 },      /* high */
    {	 3	 },      /* high */
    {	 3	 },      /* high */
    {	 3	 },
    {	 3	 },
    {	 3	 },
    {	 3	 },
    {	 3	 },	/* 72 */
    {	 3	 },	/* 73 */
    {	 3	 },	/* 74 */
    {	 3	 },	/* 75 */
    {	 4	 },      /* very high */
    {	 4	 },      /* very high */
    {	 4	 },      /* very high */
    {	 4	 },      /* very high */
    {	 4	 },	/* 80 */
    {	 4	 },	/* 81 */
    {	 4	 },	/* 82 */
    {	 4	 },	/* 83 */
    {	 4	 },
    {	 4	 },
    {	 4	 },
    {	 4	 },
    {	 5	 },
    {	 5	 },
    {	 5	 },
    {	 5	 },
    {	 5	 },
    {	 5	 },
    {	 5	 },
    {	 5	 },
    {	 5	 },
    {	 5	 },
    {	 5	 },
    {	 5	 },
    {	 6	 },	/* 100 */
    {	 6	 },	/* 101 */
    {	 7	 },	/* 102 */
    {	 7	 },	/* 103 */
    {	 7	 },
    {	 8	 },
    {	 8	 },
    {	 8	 },
    {	 9	 },
    {	 9	 },
    {	10	 },
    {	11	 }
};



const	struct	dex_app_type	dex_app		[112]		=
{
/*	To Def	*/
/*	-----	*/
    {	   70	 },   /* 0 */
    {	   67	 },   /* 1 */
    {	   65	 },   /* 2 */
    {	   62	 },   /* 3 */
    {	   60	 },   /* 4 */
    {	   57	 },   /* 5 */
    {	   55	 },   /* 6 */
    {	   52	 },   /* 7 */
    {	   50	 },
    {	   47	 },
    {	   45	 },
    {	   42	 },
    {	   40	 },
    {	   37	 },
    {	   35	 },
    {	   32	 },
    {	   30	 },
    {	   27	 },
    {	   25	 },
    {	   22	 },
    {	   20	 },   /* 20 */
    {	   17	 },   /* 21 */
    {	   15	 },   /* 22 */
    {	   12	 },   /* 23 */
    {	   10	 },
    {	    7	 },
    {	    5	 },
    {	    2	 },
    {	    2	 },
    {	    2	 },
    {	    1	 },
    {	    1	 },
    {	    1	 },
    {	    0	 },
    {	    0	 },
    {	    0	 },
    {	    0	 },
    {	    0	 },
    {	    0	 },
    {	    0	 },
    {	    0	 },   /* 40 */
    {	    0	 },   /* 41 */
    {	    0	 },   /* 42 */
    {	    0	 },   /* 43 */
    {	    0	 },
    {	    0	 },
    {	    0	 },
    {	    0	 },
    {	    0	 },
    {	    0	 },
    {	    0	 },
    {	    0	 },
    {	    0	 },
    {	    1	 },
    {	    1	 },
    {	    1	 },
    {	    2	 },
    {	    2	 },
    {	    5	 },
    {	    7	 },
    {	 - 10	 },   /* 60 */
    {	 - 11	 },   /* 61 */
    {	 - 13	 },   /* 62 */
    {	 - 14	 },   /* 63 */
    {	 - 15	 },
    {	 - 16	 },
    {	 - 18	 },
    {	 - 19	 },
    {	 - 20	 },
    {	 - 22	 },
    {	 - 25	 },
    {	 - 27	 },
    {	 - 30	 },
    {	 - 32	 },
    {	 - 35	 },
    {	 - 37	 },
    {	 - 40	 },
    {	 - 42	 },
    {	 - 45	 },
    {	 - 47	 },
    {	 - 50	 },   /* 80 */
    {	 - 52	 },   /* 81 */
    {	 - 55	 },   /* 82 */
    {	 - 57	 },   /* 83 */
    {	 - 60	 },
    {	 - 63	 },
    {	 - 67	 },
    {	 - 71	 },
    {	 - 75	 },
    {	 - 78	 },
    {	 - 82	 },
    {	 - 86	 },
    {	 - 90	 },
    {	 - 93	 },
    {	 - 97	 },
    {	 -101	 },
    {	 -105	 },
    {	 -108	 },
    {	 -112	 },
    {	 -116	 },
    {	 -120	 },   /* 100 */
    {	 -125	 },   /* 101 */
    {	 -130	 },   /* 102 */
    {	 -135	 },    /* 103 */
    {	 -140	 },
    {	 -145	 },
    {	 -155	 },
    {	 -165	 },
    {	 -178	 }, 
    {	 -195	 }, 
    {	 -220	 },
    {	 -250	 }
};


const	struct	con_app_type	con_app		[112]		=
{
/*	Hits	Shock	*/
/*	----	-----	*/
    {	 -4,	 20 },   /*   0 */
    {	 -4,	 20 },   /*   1 */
    {	 -4,	 20 },   /*   2 */
    {	 -4,	 20 },   /*   3 */
    {	 -3,	 25 },   /*   4 */
    {	 -3,	 25 },   /*   5 */
    {	 -3,	 25 },   /*   6 */
    {	 -3,	 25 },   /*   7 */
    {	 -2,	 30 },
    {	 -2,	 30 },
    {	 -2,	 30 },
    {	 -2,	 30 },
    {	 -2,	 35 },	 /*  12 */
    {	 -2,	 35 },	 /*  13 */
    {	 -2,	 35 },	 /*  14 */
    {	 -2,	 35 },	 /*  15 */
    {	 -1,	 40 },
    {	 -1,	 40 },
    {	 -1,	 40 },
    {	 -1,	 40 },
    {	 -1,	 45 },   /*  20 */
    {	 -1,	 45 },   /*  21 */
    {	 -1,	 45 },   /*  22 */
    {	 -1,	 45 },   /*  23 */
    {	 -1,	 50 },
    {	 -1,	 50 },
    {	 -1,	 50 },
    {	 -1,	 50 },
    {	  0,	 55 },
    {	  0,	 55 },
    {	  0,	 55 },
    {	  0,	 55 },
    {	  0,	 60 },
    {	  0,	 60 },
    {	  0,	 60 },
    {	  0,	 60 },
    {	  0,	 65 },
    {	  0,	 65 },
    {	  0,	 65 },
    {	  0,	 65 },
    {	  0,	 70 },   /*  40 */
    {	  0,	 70 },   /*  41 */
    {	  0,	 70 },   /*  42 */
    {	  0,	 70 },   /*  43 */
    {	  0,	 75 },
    {	  0,	 75 },
    {	  0,	 75 },
    {	  0,	 75 },
    {	  0,	 80 },
    {	  0,	 80 },
    {	  0,	 80 },
    {	  0,	 80 },
    {	  0,	 85 },
    {	  0,	 85 },
    {	  0,	 85 },
    {	  0,	 85 },
    {	  0,	 88 },
    {	  0,	 88 },
    {	  0,	 88 },
    {	  0,	 88 },
    {	  1,	 90 },   /*  60 */
    {	  1,	 90 },   /*  61 */
    {	  1,	 90 },   /*  62 */
    {	  1,	 90 },   /*  63 */
    {	  2,	 95 },
    {	  2,	 95 },
    {	  2,	 95 },
    {	  2,	 95 },
    {	  2,	 97 },
    {	  2,	 97 },
    {	  2,	 97 },
    {	  2,	 97 },
    {	  3,	 99 },   /*  72 */
    {	  3,	 99 },   /*  73 */
    {	  3,	 99 },   /*  74 */
    {	  3,	 99 },   /*  75 */
    {	  3,	 99 },
    {	  3,	 99 },
    {	  3,	 99 },
    {	  3,	 99 },
    {	  4,	 99 },   /*  80 */
    {	  4,	 99 },   /*  81 */
    {	  4,	 99 },   /*  82 */
    {	  4,	 99 },   /*  83 */
    {	  4,	 99 },
    {	  4,	 99 },
    {	  4,	 99 },
    {	  4,	 99 },
    {	  5,	 99 },
    {	  5,	 99 },
    {	  5,	 99 },
    {	  5,	 99 },
    {	  6,	 99 },
    {	  6,	 99 },
    {	  6,	 99 },
    {	  6,	 99 },
    {	  7,	 99 },
    {	  7,	 99 },
    {	  7,	 99 },
    {	  7,	 99 },
    {	  8,	 99 },   /* 100 */
    {	  8,	 99 },   /* 101 */
    {	  9,	 99 },   /* 102 */
    {	  9,	 99 },    /* 103 */
    {	 10,	 99 },
    {	 10,	 99 },
    {	 11,	 99 },
    {	 12,	 99 },
    {	 13,	 99 },
    {	 14,	 99 }, 
    {	 15,	 99 },
    {	 16,	 99 }
};



/*
 * Liquid properties.
 */

const	struct	liq_type	liq_table	[LIQ_MAX]	=
{
    { "water",			"a still clear",		{   0, 10,  0 }, 	0,		0		},
    { "beer",			"a frothy golden",	{   1,  5,  3 },	 IMM_POISON,	0		},
    { "red wine",			"a still red",		{   0,  2,  5 }, 	IMM_POISON,	0		},
    { "ale",			"a frothy brown",		{   1,  4,  5 }, 	IMM_POISON,	0		},
    { "dark ale",			"a frothy black",		{   1,  1,  5 }, 	IMM_POISON,	0		},
    { "whisky",			"a still golden",		{   0,  1, 10 }, 	IMM_POISON,	0		},
    { "lemonade",			"a fizzy pink",		{   1,  6,  0 }, 	0,		0		},
    { "firebreather",		"a boiling amber",	{   0,  1, 15 }, 	IMM_POISON,	0		},
    { "local specialty",		"a frothy orange",	{   1,  1,  7 }, 	IMM_POISON,	0		},
    { "slime mold juice",		"a bubbling green",	{   4,  4,  0 }, 	0,		0		},
    { "milk",			"a thick white",		{   2,  6,  0 }, 	0,		0		},
    { "tea",			"a pale tan",		{   0,  3,  0 }, 	0,		0		},
    { "coffee",			"a pale brown",		{   0,  3, -1 }, 	0,		0		},
    { "blood",			"a thick red",		{   2, -1,  0 }, 	IMM_NEGATIVE,0		},
    { "salt water",			"a still clear",		{   0, -2,  0 }, 	IMM_DROWNING,0		},
    { "coke",			"a fizzy brown",		{   1,  5,  0 }, 	0,		0		},
    { "dirty water",			"a dirty brown",		{   1,  8,  0 }, 	0,		0		},
    { "ambrosia",			"a shimmering golden",	{  10, 10,  3 }, 	0,		0		},
    { "slime",			"a snotty green",		{  -2, -2,  0 }, 	0,		0		},
    { "dark coffee",			"a still black",		{   0,  2, -5 }, 	0,		0		},
    { "vodka",			"a still clear",		{   0,  1, 10 }, 	IMM_POISON,	0		},
    { "soda water",			"a fizzy clear",		{   0,  5,  0 }, 	0,		0		},
    { "petrol",		        	"an aromatic",		{   0,  0,  0 }, 	IMM_POISON,	0		},
    { "root beer",			"a frothy brown",		{   0,  9,  0 }, 	0,		0		},
    { "elvish wine",		"a fizzy golden",		{   5,  3, 10 }, 	0,		0		},
    { "white wine",			"a still golden",		{   0,  2,  5 }, 	0,		0		},
    { "champagne",		"a fizzy golden",		{   0,  1,  5 }, 	0,		0		},
    { "mead",			"a golden honey",	{   2,  2,  7 }, 	0,		0		},
    { "rose wine",			"a still pink",		{   0,  2,  5 }, 	0,		0		},
    { "benedictine wine",		"a still red",		{   1,  2,  7 }, 	0,		0		},
    { "cranberry juice",		"a still red",		{   2,  8,  0 }, 	0,		0		},
    { "orange juice",		"a still orange",		{   4,  8,  0 }, 	0,		0		},
    { "absinthe",			"a still green",		{   0,  1, 25 }, 	0,		0		},
    { "brandy",			"a still golden",		{   0,  1, 10 }, 	0,		0		},
    { "aquavit",			"a still clear",		{   0,  1, 12 }, 	0,		0		},
    { "schnapps",			"a still clear",		{   0,  1,  7 }, 	0,		0		},
    { "icewine",			"a fizzy clear",		{   0,  2,  5 }, 	0,		0		},
    { "amontillado",		"a thick red",		{   0,  2,  5 }, 	0,		0		},
    { "sherry",			"a still brown",		{   1,  1,  7 }, 	0,		0		},
    { "framboise",			"a thick red",		{   3,  1,  5 }, 	0,		0		},
    { "rum",			"a still clear",		{   0,  1, 10 }, 	0,		0		},
    { "cordial",			"a still green",		{   3,  9,  0 }, 	0,		0		},
    { "port",			"a thick red",		{   1,  1,  7 }, 	0,		0		},
    { "wheatgrass",		"a frothy green",		{   4,  1, -1 }, 	0,		0		}

};




/* Define which defences can be used against which attack type... */

int can_block_dam[] = {

/* DAM_NONE		*/	(BLOCK_ALL),
/* DAM_BASH		*/	(BLOCK_COMBAT),
/* DAM_PIERCE		*/	(BLOCK_COMBAT),
/* DAM_SLASH		*/	(BLOCK_COMBAT),
/* DAM_FIRE		*/	(BLOCK_BALL),
/* DAM_COLD		*/	(BLOCK_BOLT),
/* DAM_LIGHTNING	*/	(BLOCK_LIGHTNING),
/* DAM_ACID		*/	(BLOCK_NOPARRY),
/* DAM_POISON	*/	(BLOCK_ABSORB),
/* DAM_NEGATIVE	*/	(BLOCK_NOPARRY),
/* DAM_HOLY		*/	(BLOCK_NOPARRY),
/* DAM_ENERGY	*/	(BLOCK_NOPARRY),
/* DAM_MENTAL	*/	(BLOCK_ABSORB),
/* DAM_DISEASE	*/	(BLOCK_NOPARRY),
/* DAM_DROWNING	*/	(BLOCK_ABSORB),
/* DAM_LIGHT		*/	(BLOCK_NOPARRY),
/* DAM_OTHER		*/	(BLOCK_ALL),
/* DAM_HARM		*/	(BLOCK_ABSORB),
/* DAM_SHOT		*/	(BLOCK_NOPARRY),
/* DAM_OLD		*/	(BLOCK_NONE),
/* DAM_SOUND		*/	(BLOCK_LIGHT)

};


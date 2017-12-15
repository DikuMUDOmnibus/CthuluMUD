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

/***************************************************************************
 *  File: olc.c                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/


#include "everything.h"
#include "olc.h"
#include "profile.h"


/* Movement xedit_ functions */

bool xedit_north( CHAR_DATA *ch, char *argument ) {
    interpret(ch, "north");
    return FALSE;
}

bool xedit_east( CHAR_DATA *ch, char *argument ) {
    interpret(ch, "east");
    return FALSE;
}

bool xedit_south( CHAR_DATA *ch, char *argument ) {
    interpret(ch, "south");
    return FALSE;
}

bool xedit_west( CHAR_DATA *ch, char *argument ) {
    interpret(ch, "west");
    return FALSE;
}

bool xedit_up( CHAR_DATA *ch, char *argument ) {
    interpret(ch, "up");
    return FALSE;
}

bool xedit_down( CHAR_DATA *ch, char *argument ) {
    interpret(ch, "down");
    return FALSE;
}

bool xedit_northeast( CHAR_DATA *ch, char *argument ) {
    interpret(ch, "ne");
    return FALSE;
}

bool xedit_southeast( CHAR_DATA *ch, char *argument ) {
    interpret(ch, "se");
    return FALSE;
}

bool xedit_southwest( CHAR_DATA *ch, char *argument ) {
    interpret(ch, "sw");
    return FALSE;
}

bool xedit_northwest( CHAR_DATA *ch, char *argument ) {
    interpret(ch, "nw");
    return FALSE;
}

bool xedit_in( CHAR_DATA *ch, char *argument ) {
    interpret(ch, "in");
    return FALSE;
}

bool xedit_out( CHAR_DATA *ch, char *argument ) {
    interpret(ch, "out");
    return FALSE;
}


/* Executed from comm.c.  Minimizes compiling when changes are made. */
bool run_olc_editor( DESCRIPTOR_DATA *d ) {
    switch ( d->editor ) {
    case ED_AREA:
	aedit( d->character, d->incomm );
	break;
    case ED_ROOM:
	redit( d->character, d->incomm );
	break;
    case ED_OBJECT:
	oedit( d->character, d->incomm );
	break;
    case ED_MOBILE:
	medit( d->character, d->incomm );
	break;
    case ED_HELP:
	hedit( d->character, d->incomm );
	break;
    case ED_SOCIAL:
	sedit( d->character, d->incomm );
	break;
    default:
	return FALSE;
    }
    return TRUE;
}



char *olc_ed_name( CHAR_DATA *ch ) {
    static char buf[10];
    
    buf[0] = '\0';
    switch (ch->desc->editor) {
    case ED_AREA:
	sprintf( buf, "AEdit" );
	break;
    case ED_ROOM:
	sprintf( buf, "REdit" );
	break;
    case ED_OBJECT:
	sprintf( buf, "OEdit" );
	break;
    case ED_MOBILE:
	sprintf( buf, "MEdit" );
	break;
    case ED_HELP:
        sprintf( buf, "%d", 0 ); 
        break;
    case ED_SOCIAL:
        sprintf( buf, "%d", 0 ); 
        break;
    default:
	sprintf( buf, " " );
	break;
    }
    return buf;
}


// This returns the real (long) VNUM for the item

char *olc_ed_vnum( CHAR_DATA *ch ) {
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    OBJ_INDEX_DATA *pObj;
    MOB_INDEX_DATA *pMob;
    static char buf[10];
	
    buf[0] = '\0';
    switch ( ch->desc->editor ) {
    case ED_AREA:
	pArea = (AREA_DATA *)ch->desc->pEdit;
	sprintf( buf, "%d", pArea ? pArea->vnum : 0 );
	break;
    case ED_ROOM:
	pRoom = ch->in_room;
	sprintf( buf, "%ld", pRoom ? pRoom->vnum : 0 );
	break;
    case ED_OBJECT:
	pObj = (OBJ_INDEX_DATA *)ch->desc->pEdit;
	sprintf( buf, "%ld", pObj ? pObj->vnum : 0 );
	break;
    case ED_MOBILE:
	pMob = (MOB_INDEX_DATA *)ch->desc->pEdit;
	sprintf( buf, "%ld", pMob ? pMob->vnum : 0 );
	break;
    default:
	sprintf( buf, " " );
	break;
    }

    return buf;
}



/*****************************************************************************
 Name:		show_olc_cmds
 Purpose:	Format up the commands from given table.
 Called by:	show_commands(olc_act.c).
 ****************************************************************************/
void show_olc_cmds( CHAR_DATA *ch, const struct olc_cmd_type *olc_table ) {
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  cmd;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    for (cmd = 0; olc_table[cmd].name[0] != '\0'; cmd++)   {
	sprintf( buf, "%-15.15s", olc_table[cmd].name );
	strcat( buf1, buf );
	if ( ++col % 5 == 0 )
	    strcat( buf1, "\r\n" );
    }
 
    if ( col % 5 != 0 ) strcat( buf1, "\r\n" );
    send_to_char( buf1, ch );
    return;
}



/*****************************************************************************
 Name:		show_commands
 Purpose:	Display all olc commands.
 Called by:	olc interpreters.
 ****************************************************************************/
bool show_commands( CHAR_DATA *ch, char *argument ) {
    switch (ch->desc->editor) {
	case ED_AREA:
	    show_olc_cmds( ch, aedit_table );
	    break;
	case ED_ROOM:
	    show_olc_cmds( ch, redit_table );
	    break;
	case ED_OBJECT:
	    show_olc_cmds( ch, oedit_table );
	    break;
	case ED_MOBILE:
	    show_olc_cmds( ch, medit_table );
	    break;
	case ED_HELP:
	    show_olc_cmds( ch, hedit_table );
	    break;
	case ED_SOCIAL:
	    show_olc_cmds( ch, sedit_table );
	    break;
    }

    return FALSE;
}



/*****************************************************************************
 *                           Interpreter Tables.                             *
 *****************************************************************************/
const struct olc_cmd_type aedit_table[] =
{
/*  {   command		function	}, */

  /* Movement first... */

    {   "north",		xedit_north	},
    {   "east",		xedit_east	},
    {   "south",		xedit_south	},
    {   "west",		xedit_west	},
    {   "up",		xedit_up	},
    {   "down",		xedit_down	},
    {   "in",		xedit_in	},
    {   "out",		xedit_out	},
    {   "northeast",		xedit_northeast	},
    {   "southeast",		xedit_southeast	},
    {   "southwest",	xedit_southwest	},
    {   "northwest",	xedit_northwest	},
    {   "neast",		xedit_northeast	},
    {   "seast",		xedit_southeast	},
    {   "swest",		xedit_southwest	},
    {   "nwest",		xedit_northwest	},

   /* The editing... */

    {   "age",		aedit_age	},
    {   "builder",		aedit_builder	}, 
    {   "map",		aedit_map	}, 
    {   "commands",	show_commands	},
    {   "copyright",		aedit_copyr	},
    {   "create",		aedit_create	},
    {   "filename",		aedit_file	},
    {   "flags",        		aedit_flags     	}, 
    {   "name",		aedit_name	},
    {   "recall",		aedit_recall	},  
    {   "respawn",		aedit_respawn	},  
    {   "morgue",		aedit_morgue	},  
    {   "dream",		aedit_dream	},  
    {   "prison",		aedit_prison	},  
    {   "mare",		aedit_mare	},  
    {   "reset",		aedit_reset	},
    {   "security",		aedit_security	},
    {   "show",		aedit_show	},
    {   "vnum",		aedit_vnum	},
    {   "zone",		aedit_zone	},
    {   "lvnum",		aedit_lvnum	},
    {   "uvnum",		aedit_uvnum	},
    {   "climate",		aedit_climate	},
    {   "currency",		aedit_currency	},
    {   "law",		aedit_law	},

    {   "otypes",		show_types	},
    {   "?",		show_help	},
    {   "version",		show_version	},

    {	"",		0,		}
};



const struct olc_cmd_type redit_table[] =
{
/*  {   command		function	}, */
  
   /* Movement first... */
 
    {   "north",		redit_north	},
    {   "east",		redit_east	},
    {   "south",		redit_south	},
    {   "west",		redit_west	},
    {   "up",		redit_up	},
    {   "down",		redit_down	},
    {   "in",		redit_in	},
    {   "out",		redit_out	},
    {   "northeast",		redit_northeast	},
    {   "southeast",		redit_southeast	},
    {   "southwest",	redit_southwest	},
    {   "northwest",	redit_northwest	},
    {   "neast",		redit_northeast	},
    {   "seast",		redit_southeast	},
    {   "swest",		redit_southwest	},
    {   "nwest",		redit_northwest	},

   /* Then the editing commands... */ 

    {   "commands",	show_commands	},
    {   "create",		redit_create	},
    {   "delete",  		redit_delete	},
    {   "desc",		redit_desc	},
    {   "ed",		redit_ed	},
    {   "flags",        		redit_flags     },
    {   "format",		redit_format	},
    {   "gadgets",		redit_gadget	},
    {   "name",		redit_name	},
    {   "subarea",		redit_subarea	},
    {   "sector",       	redit_sector    },
    {   "show",		redit_show	},
    {   "used",		redit_used      },
    {   "unused",		redit_unused    },
    {   "currents",		redit_current   },
    {   "recall",		redit_recall	},
    {   "respawn",		redit_respawn	},
    {   "morgue",		redit_morgue	},
    {   "dream",		redit_dream	},
    {   "mare",		redit_mare	},
    {   "night",		redit_night	},
    {   "day",		redit_day	},
    {   "affect",		redit_affect	},
    {   "revnum",		redit_revnum	},
    {   "ptracks",		redit_ptracks	},
    {   "image",		redit_image	},
    {   "rent",  		redit_rent	},
    {   "heal",  		redit_heal	},
    {   "mana",  		redit_mana	},

    /* New reset commands. */
    {   "mreset",		redit_mreset	},
    {   "oreset",		redit_oreset	},
    {   "mlist",		redit_mlist	},
    {   "olist",		redit_olist	},
    {   "mshow",		redit_mshow	},
    {   "oshow",		redit_oshow	},

    {   "otypes",		show_types	},
    {   "?",		show_help	},
    {   "version",		show_version	},
    {	"",		0,		}
};



const struct olc_cmd_type oedit_table[] =
{
/*  {   command		function	}, */

  /* Movement first... */

    {   "north",		xedit_north	},
    {   "east",		xedit_east	},
    {   "south",		xedit_south	},
    {   "west",		xedit_west	},
    {   "up",		xedit_up	},
    {   "down",		xedit_down	},
    {   "in",		xedit_in	},
    {   "out",		xedit_out	},
    {   "northeast",		xedit_northeast	},
    {   "southeast",		xedit_southeast	},
    {   "southwest",	xedit_southwest	},
    {   "northwest",	xedit_northwest	},
    {   "neast",		xedit_northeast	},
    {   "seast",		xedit_southeast	},
    {   "swest",		xedit_southwest	},
    {   "nwest",		xedit_northwest	},

   /* The editing... */

    {   "addaffect",		oedit_addaffect	},
    {   "addapply",		oedit_addapply	},
    {   "addskill",		oedit_addskill	},
    {   "addeffect",		oedit_addeffect	},
    {   "addmax",		oedit_addmax	},
    {   "commands",	show_commands	},
    {   "cost",		oedit_cost	},
    {   "create",		oedit_create	},
    {   "delete",		oedit_delete	},
    {   "used",		oedit_used      },
    {   "unused",		oedit_unused    },
    {   "default",      	oedit_default   },
    {   "delaffect",		oedit_delaffect	},
    {   "delapply",		oedit_delaffect	},
    {   "delskill",		oedit_delaffect	},
    {   "ed",		oedit_ed	},
    {   "long",		oedit_long	},
    {   "name",		oedit_name	},
    {   "repop",		oedit_repop	},
    {   "short",		oedit_short	},
    {   "show",		oedit_show	},
    {   "v0",		oedit_value0	},
    {   "v1",		oedit_value1	},
    {   "v2",		oedit_value2	},
    {   "v3",		oedit_value3	},
    {   "v4",		oedit_value4	},  	
    {   "weight",		oedit_weight	},
    {   "revnum",		oedit_revnum	},
    {   "anchors",		oedit_anchor	},
    {   "artifact",		oedit_artifact	},
    {   "extra",        		oedit_extra     },  
    {   "wear",         		oedit_wear      },
    {   "type",         		oedit_type      }, 
    {   "material",     	oedit_material  },
    {   "level",        		oedit_level     },  
    {   "condition",    	oedit_condition },  
    {   "image",        	oedit_image     },  
    {   "zone",         		oedit_zone      },  

    {   "otypes",		show_types	},
    {   "?",		show_help	},
    {   "version",		show_version	},
    {	"",		0,		}
};



const struct olc_cmd_type medit_table[] =
{
/*  {   command		function	}, */

  /* Movement first... */

    {   "north",		xedit_north	},
    {   "east",		xedit_east	},
    {   "south",		xedit_south	},
    {   "west",		xedit_west	},
    {   "up",		xedit_up	},
    {   "down",		xedit_down	},
    {   "in",		xedit_in	},
    {   "out",		xedit_out	},
    {   "northeast",		xedit_northeast	},
    {   "southeast",		xedit_southeast	},
    {   "southwest",	xedit_southwest	},
    {   "northwest",	xedit_northwest	},
    {   "neast",		xedit_northeast	},
    {   "seast",		xedit_southeast	},
    {   "swest",		xedit_southwest	},
    {   "nwest",		xedit_northwest	},

   /* The editing... */

    {   "alignment",		medit_align	},
    {   "commands",	show_commands	},
    {   "create",		medit_create	},
    {   "delete",		medit_delete	},
    {   "used",		medit_used      },
    {   "unused",		medit_unused    },
    {   "default",      	medit_default   },
    {   "desc",		medit_desc	},
    {   "mbio",		medit_bio	},
    {   "level",		medit_level	},
    {   "long",		medit_long	},
    {   "name",		medit_name	},
    {   "shop",		medit_shop	},
    {   "short",		medit_short	},
    {   "show",		medit_show	},
    {   "spec",		medit_spec	},
    {   "sex",          		medit_sex       },  
    {   "act",          		medit_act       },  
    {   "affect",       		medit_affect    },
    {   "form",         		medit_form      },
    {   "part",         		medit_part      },  
    {   "imm",          		medit_imm       },
    {   "envimm",    		medit_envimm       },
    {   "eimm",    		medit_envimm       },
    {   "res",          		medit_res       },  
    {   "vuln",         		medit_vuln      }, 
    {   "material",     	medit_material  },
    {   "off",          		medit_off       },  
    {   "size",         		medit_size      },  
    {   "race",         		medit_race      },  
    {   "language",  	medit_language},  
    {   "position",     	medit_position  }, 
    {   "gold",         		medit_gold      }, 
    {   "fright",       		medit_fright    }, 
    {   "hitroll",      		medit_hitroll   },  
    {   "attack",		medit_attack    },  
    {   "damtype",		medit_damtype   },  
    {   "skills",       		medit_skills    },
    {   "setprof",      	medit_setprof   },
    {   "conv",         	medit_conv	},
    {   "scripts",      	medit_script	},
    {   "macro",      		medit_macro	},
    {   "events",       	medit_events    },
    {   "triggers",     	medit_trigger   },
    {   "nature",		medit_nature	},
    {   "timesub",		medit_timesub	},
    {   "monitors",		medit_monitor	},
    {   "revnum",		medit_revnum	},
    {   "chat",		medit_chat	},
    {   "vision",		medit_vision	},
    {   "image",		medit_image	},
    {   "society",		medit_society	},
    
    {   "otypes",		show_types	},
    {   "?",		show_help	},
    {   "version",		show_version	},
    {	"",		0,		}
};

/*****************************************************************************
 *                          End Interpreter Tables.                          *
 *****************************************************************************/



/*****************************************************************************
 Name:		get_area_data
 Purpose:	Returns pointer to area with given vnum.
 Called by:	do_aedit(olc.c).
 ****************************************************************************/
AREA_DATA *get_area_data( int vnum ) {
    AREA_DATA *pArea;

    for (pArea = area_first; pArea; pArea = pArea->next )  {
        if (pArea->vnum == vnum) return pArea;
    }

    return NULL;
}



/*****************************************************************************
 Name:		edit_done
 Purpose:	Resets builder information on completion.
 Called by:	aedit, redit, oedit, medit(olc.c)
 ****************************************************************************/
bool edit_done( CHAR_DATA *ch ) {
    ch->desc->pEdit = NULL;
    ch->desc->editor = 0;
    return FALSE;
}



/*****************************************************************************
 *                              Interpreters.                                *
 *****************************************************************************/


/* Area Interpreter, called by do_aedit. */
void aedit( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int  cmd;

    EDIT_AREA(ch, pArea);
    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( !IS_BUILDER( ch, pArea ) ) 
 	send_to_char( "AEdit:  Insufficient security to modify area.\r\n", ch );

    if ( command[0] == '\0' )
    {
	aedit_show( ch, argument );
	return;
    }

    if ( !str_cmp(command, "done") )
    {
	edit_done( ch );
	return;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
	interpret( ch, arg );
	return;
    }

    if ( arg[0] == '-' ) {
      interpret( ch, (arg + 1));
      return;
    }


    /* Search Table and Dispatch Command. */
    for ( cmd = 0; aedit_table[cmd].name[0] != '\0'; cmd++ )    {
	if ( !str_prefix( command, aedit_table[cmd].name ) )	{
	    if ( (*aedit_table[cmd].olc_fun) ( ch, argument ) )	    {
		SET_BIT( pArea->area_flags, AREA_CHANGED );
		return;
	    }	    else
		return;
	}
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}



/* Room Interpreter, called by do_redit. */
void redit( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *pRoom;
    AREA_DATA *pArea;
    char arg[MAX_STRING_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int  cmd;

    EDIT_ROOM(ch, pRoom);
    pArea = pRoom->area;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( !IS_BUILDER( ch, pArea ) )
        send_to_char( "REdit:  Insufficient security to modify room.\r\n", ch );

    if ( command[0] == '\0' )
    {
	redit_show( ch, argument );
	return;
    }

    if ( !str_cmp(command, "done") )  {
	edit_done( ch );
	return;
    }

    if ( !IS_BUILDER( ch, pArea ) )  {
        interpret( ch, arg );
        return;
    }

    if ( arg[0] == '-' ) {
      interpret( ch, (arg + 1));
      return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; redit_table[cmd].name[0] != '\0'; cmd++ )  {
	if ( !str_prefix( command, redit_table[cmd].name ) ) {
	    if ( (*redit_table[cmd].olc_fun) ( ch, argument ) )    {
		SET_BIT( pArea->area_flags, AREA_CHANGED );
		return;
	    }   else return;
	}
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}



/* Object Interpreter, called by do_oedit. */
void oedit( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    OBJ_INDEX_DATA *pObj;
    char arg[MAX_STRING_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int  cmd;
/*  int  value;   ROM */

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    EDIT_OBJ(ch, pObj);
    pArea = pObj->area;

    if ( !IS_BUILDER( ch, pArea ) )
	send_to_char( "OEdit: Insufficient security to modify area.\r\n", ch );

    if ( command[0] == '\0' )   {
	oedit_show( ch, argument );
	return;
    }

    if ( !str_cmp(command, "done"))    {
	edit_done( ch );
	return;
    }

    if ( !IS_BUILDER( ch, pArea ) )  {
	interpret( ch, arg );
	return;
    }

    if ( arg[0] == '-' ) {
      interpret( ch, (arg + 1));
      return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; oedit_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( !str_prefix( command, oedit_table[cmd].name ) )
	{
	    if ( (*oedit_table[cmd].olc_fun) ( ch, argument ) )
	    {
		SET_BIT( pArea->area_flags, AREA_CHANGED );
		return;
	    }
	    else
		return;
	}
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}



/* Mobile Interpreter, called by do_medit. */
void medit( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    MOB_INDEX_DATA *pMob;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_STRING_LENGTH];
    int  cmd;
/*  int  value;    ROM */

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    EDIT_MOB(ch, pMob);
    pArea = pMob->area;

    if ( !IS_BUILDER( ch, pArea ) ) send_to_char( "MEdit: Insufficient security to modify area.\r\n", ch );

    if ( command[0] == '\0' )   {
        medit_show( ch, argument );
        return;
    }

    if ( !str_cmp(command, "done"))  {
                if (ch->pcdata->macro) {
	    ch->pcdata->macro = FALSE;
	    ch->pcdata->macro_script = 0;
	    ch->pcdata->macro_line = 0;
	    ch->pcdata->macro_delay = 0;
                }
	edit_done( ch );
	return;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
	interpret( ch, arg );
	return;
    }

    if ( arg[0] == '-' ) {
      interpret( ch, (arg + 1));
      return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; medit_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( !str_prefix( command, medit_table[cmd].name ) )
	{
	    if ( (*medit_table[cmd].olc_fun) ( ch, argument ) )
	    {
		SET_BIT( pArea->area_flags, AREA_CHANGED );
		return;
	    }
	    else
		return;
	}
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}




const struct editor_cmd_type editor_table[] =
{
/*  {   command		function	}, */

    {   "area",		do_aedit	},
    {   "room",		do_redit	},
    {   "object",	do_oedit	},
    {   "mobile",	do_medit	},
    {   "help",		do_hedit	},
    {   "social",		do_sedit  	},
    {	"",		0,		}
};


/* Entry point for all editors. */
void do_olc( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    int  cmd;

    argument = one_argument( argument, command );

    if ( command[0] == '\0' )
    {
        do_help( ch, "olc" );
        return;
    }
 
    /* Search Table and Dispatch Command. */
    for ( cmd = 0; editor_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( !str_prefix( command, editor_table[cmd].name ) )
	{
	    (*editor_table[cmd].do_fun) ( ch, argument );
	    return;
	}
    }

    /* Invalid command, send help. */
    do_help( ch, "olc" );
    return;
}



/* Entry point for editing area_data. */
void do_aedit( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    int value;

    if (!IS_IMP(ch))
    {
	send_to_char ("You must have an implementor modify areas.\r\n.",ch);
	return;
    }

    pArea = ch->in_room->area;

    if ( is_number( argument ) ) {
	value = atoi( argument );
	if ( !( pArea = get_area_data( value ) ) ) {
	    send_to_char( "That area does not exist, see alist.\r\n", ch );
	    return;
	}
    }
    else
    {
	if ( !str_cmp( argument, "create" ) )
	{
	    pArea               =   new_area();
	    area_last->next     =   pArea;
	    area_last		=   pArea;	/* Thanks, Walker. */
	    SET_BIT( pArea->area_flags, AREA_ADDED );
	}
    }

    if (IS_SET(pArea->area_flags, AREA_EDLOCK)) {
      send_to_char("{YWARNING: Area is edit locked!{x\r\n", ch);
    }

    ch->desc->pEdit = (void *)pArea;
    ch->desc->editor = ED_AREA;
    return;
}



/* Entry point for editing room_index_data. */
void do_redit( CHAR_DATA *ch, char *argument ) {
ROOM_INDEX_DATA *pRoom;
char arg1[MAX_STRING_LENGTH];
AREA_DATA *pArea;
VNUM value;

    argument = one_argument( argument, arg1 );

    pRoom = ch->in_room;

    if ( !str_cmp( arg1, "reset" ))  {
               reset_room( pRoom, FALSE, TRUE);
               send_to_char( "Room FORCEreset.\r\n", ch );
               return;

    } else if ( !str_cmp( arg1, "create" )) {
	if ( argument[0] == '\0' || atoi( argument ) == 0 ) {
	    send_to_char( "Syntax:  edit room create [vnum]\r\n", ch );
	    return;
	}

                value = atol(argument);
	pArea = get_vnum_area( value );

        if (pArea == NULL) {
            send_to_char("That is not inside an existing area!\r\n", ch);
            return;
        }

        if (IS_SET(pArea->area_flags, AREA_EDLOCK)) {
            send_to_char("{YArea is edit locked!{x\r\n", ch);
            return;
        }

	if ( redit_create( ch, argument ) )
	{
	    char_from_room( ch );
	    char_to_room( ch, (ROOM_INDEX_DATA *) ch->desc->pEdit );
	    SET_BIT( pRoom->area->area_flags, AREA_CHANGED );
	}
    } else {
        if (IS_SET(pRoom->area->area_flags, AREA_EDLOCK)) {
            send_to_char("{YRooms area is edit locked!{x\r\n", ch);
            return;
        }
    } 

    ch->desc->editor = ED_ROOM;
    return;
}



/* Entry point for editing obj_index_data. */
void do_oedit( CHAR_DATA *ch, char *argument ) {
OBJ_INDEX_DATA *pObj;
AREA_DATA *pArea;
char arg1[MAX_STRING_LENGTH];
VNUM value;

    if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg1 );

    if ( is_number( arg1 ) )
    {
	value = atoi( arg1 );
	pObj = get_obj_index( value );

	if ( pObj == NULL ) {
	    send_to_char( "OEdit:  That vnum does not exist.\r\n", ch );
	    return;
	}

        if (IS_SET(pObj->area->area_flags, AREA_EDLOCK)) {
            send_to_char("{YObjects area is edit locked!{x\r\n", ch);
            return;
        }

	ch->desc->pEdit = (void *)pObj;
	ch->desc->editor = ED_OBJECT;
	return;
    }
    else
    {
	if ( !str_cmp( arg1, "create" ) )
	{
	    value = atol( argument );

	    if ( argument[0] == '\0' || value == 0 )
	    {
		send_to_char( "Syntax:  edit object create [vnum]\r\n", ch );
		return;
	    }

	    pArea = get_vnum_area( value );

            if (pArea == NULL) {
                send_to_char("That is not inside an existing area!\r\n", ch);
                return;
            }

            if (IS_SET(pArea->area_flags, AREA_EDLOCK)) {
                send_to_char("{YArea is edit locked!{x\r\n", ch);
                return;
            }

	    if ( oedit_create( ch, argument ) )
	    {
		SET_BIT( pArea->area_flags, AREA_CHANGED );
		ch->desc->editor = ED_OBJECT;
	    }

	    return;
	}
    }

    send_to_char( "OEdit:  There is no default object to edit.\r\n", ch );
    return;
}



/* Entry point for editing mob_index_data. */
void do_medit( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;
    AREA_DATA *pArea;
    int value;
    char arg1[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg1 );

    if ( is_number( arg1 ) )
    {
	value = atoi( arg1 );

	pMob = get_mob_index( value );

	if ( pMob == NULL ) {
	    send_to_char( "MEdit:  That vnum does not exist.\r\n", ch );
	    return;
	}

        if (IS_SET(pMob->area->area_flags, AREA_EDLOCK)) {
            send_to_char("{YMobiles area is edit locked!{x\r\n", ch);
            return;
        }

	ch->desc->pEdit = (void *)pMob;
	ch->desc->editor = ED_MOBILE;
	return;
    }
    else
    {
	if ( !str_cmp( arg1, "create" ) )
	{
	    value = atoi( argument );
	    if ( arg1[0] == '\0' || value == 0 )
	    {
		send_to_char( "Syntax:  edit mobile create [vnum]\r\n", ch );
		return;
	    }

	    pArea = get_vnum_area( value );

            if (pArea == NULL) {
                send_to_char("That is not inside an existing area!\r\n", ch);
                return;
            }

            if (IS_SET(pArea->area_flags, AREA_EDLOCK)) {
                send_to_char("{YArea is edit locked!{x\r\n", ch);
                return;
            }

	    if ( medit_create( ch, argument ) )
	    {
		SET_BIT( pArea->area_flags, AREA_CHANGED );
		ch->desc->editor = ED_MOBILE;
	    }
	    return;
	}
    }

    send_to_char( "MEdit:  There is no default mobile to edit.\r\n", ch );
    return;
}



void display_resets( CHAR_DATA *ch ) {
ROOM_INDEX_DATA	*pRoom;
RESET_DATA		*pReset;
MOB_INDEX_DATA	*pMob = NULL;
char 		buf   [ MAX_STRING_LENGTH ];
char 		final [ MAX_STRING_LENGTH ];
int 		iReset = 0;

    EDIT_ROOM(ch, pRoom);
    final[0]  = '\0';
    
    send_to_char ( 
  " No.  Loads    Description       Location         Vnum    Max  Description"
  "\r\n"
  "==== ======== ============= =================== ======== ====== ==========="
  "\r\n", ch );

    for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )  {
	OBJ_INDEX_DATA  *pObj;
	MOB_INDEX_DATA  *pMobIndex;
	OBJ_INDEX_DATA  *pObjIndex;
	OBJ_INDEX_DATA  *pObjToIndex;
	ROOM_INDEX_DATA *pRoomIndex;
	
	final[0] = '\0';
	sprintf( final, "{x[%2d] ", ++iReset );

	switch ( pReset->command ) {
	default:
	    sprintf( buf, "Bad reset command: %c\r\n", pReset->command );
	    strcat( final, buf );
	    break;

	case 'M':
	    if ( !( pMobIndex = get_mob_index( pReset->arg1 ) ) )  {
                sprintf( buf, "Load Mobile - Bad Mob %ld\r\n", pReset->arg1 );
                strcat( final, buf );
                break;
	    }

	    if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )   {
                sprintf( buf, "Load Mobile - Bad Room %ld\r\n", pReset->arg3 );
                strcat( final, buf );
                break;
	    }

            pMob = pMobIndex;
            sprintf( buf, "M[%5ld] %-13.13s in room             R[%5ld] [%4d] %-15.15s \r\n",  pReset->arg1, pMob->short_descr, pReset->arg3, pReset->arg2, pRoomIndex->name );
            strcat( final, buf );

	    /*
	     * Check for pet shop.
	     * -------------------
	     */
	    {
		ROOM_INDEX_DATA *pRoomIndexPrev;

		pRoomIndexPrev = get_room_index( pRoomIndex->vnum - 1 );
		if ( pRoomIndexPrev
		    && IS_SET( pRoomIndexPrev->room_flags, ROOM_PET_SHOP ) )
                    final[5] = 'P';
	    }

	    break;

	case 'O':
	    if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) ) {
                sprintf( buf, "Load Object - Bad Object %ld\r\n",   pReset->arg1 );
                strcat( final, buf );
                break;
	    }

            pObj       = pObjIndex;

	    if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )   {
                sprintf( buf, "Load Object - Bad Room %ld\r\n", pReset->arg3 );
                strcat( final, buf );
                break;
	    }

            sprintf( buf, "O[%5ld] %-13.13s in room             "
                          "R[%5ld] [%4d] %-15.15s \r\n",
                          pReset->arg1, pObj->short_descr,
                          pReset->arg3, pReset->arg2, pRoomIndex->name );
            strcat( final, buf );

	    break;

	case 'P':
	    pObjIndex = get_obj_index( pReset->arg1 );

	    if ( pObjIndex == NULL ) {
                sprintf( buf, "Put Object - Bad Object %ld\r\n", pReset->arg1 );
                strcat( final, buf );
                break;
	    }

            pObj       = pObjIndex;

	    pObjToIndex = get_obj_index( pReset->arg3 );

	    if ( pObjToIndex == NULL ) {
                sprintf( buf, "Put Object - Bad To Object %ld\r\n", pReset->arg3 );
                strcat( final, buf );
                break;
	    }

	    sprintf( buf, "P[%5ld] %-13.13s inside              O[%5ld] [%4d] %-15.15s \r\n", pReset->arg1, pObj->short_descr, pReset->arg3, pReset->arg2, pObjToIndex->short_descr );
                    strcat( final, buf );
	    break;

	case 'G':
	case 'E':
	    if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) ) {
                sprintf( buf, "Give/Equip Object - Bad Object %ld\r\n", pReset->arg1 );
                strcat( final, buf );
                break;
	    }

            pObj       = pObjIndex;

	    if ( !pMob )  {
                sprintf( buf, "Give/Equip Object - No Previous Mobile\r\n" );
                strcat( final, buf );
                break;
	    }

	    if ( pMob->pShop ) {
	    sprintf( buf,
		"%c[%5ld] %-13.13s in the inventory of S[%5ld] [%4d] %-15.15s \r\n", pReset->command, pReset->arg1, pObj->short_descr, pMob->vnum, pReset->arg2, pMob->short_descr  );
	    }
	    else
	    sprintf( buf, "%c[%5ld] %-13.13s %-19.19s M[%5ld] [%4d] %-15.15s \r\n", pReset->command, pReset->arg1, pObj->short_descr, (pReset->command == 'G') ? flag_string( wear_loc_strings, WEAR_NONE ) : flag_string( wear_loc_strings, pReset->arg3 ), pMob->vnum, pReset->arg2,  pMob->short_descr );
	    strcat( final, buf );

	    break;

	case 'R':
	    if ( !( pRoomIndex = get_room_index( pReset->arg1 ) ) ) {
		sprintf( buf, "Randomize Exits - Bad Room %ld \r\n", pReset->arg1 );
		strcat( final, buf );
		continue;
	    }

	    sprintf( buf, "R[%5ld] Exits 0 through %d are randomized in %s \r\n", pReset->arg1, (pReset->arg2) - 1, pRoomIndex->name );
	    strcat( final, buf );

	    break;

        case 'D':
           sprintf(buf, "Suppressed %c reset.\r\n", pReset->command);  
	   strcat( final, buf );
           break;

	}
	send_to_char( final, ch );
    }

    return;
}



/*****************************************************************************
 Name:		add_reset
 Purpose:	Inserts a new reset in the given index slot.
 Called by:	do_resets(olc.c).
 ****************************************************************************/
void add_reset( ROOM_INDEX_DATA *room, RESET_DATA *pReset, int index ) {
    RESET_DATA *reset;
    int iReset = 0;

    if ( !room->reset_first )   {
	room->reset_first	= pReset;
	room->reset_last	= pReset;
	pReset->next		= NULL;
	return;
    }

    index--;

    if ( index == 0 )	{
	pReset->next = room->reset_first;
	room->reset_first = pReset;
	return;
    }

    /*
     * If negative slot( <= 0 selected) then this will find the last.
     */
    for ( reset = room->reset_first; reset->next; reset = reset->next )
    {
	if ( ++iReset == index )
	    break;
    }

    pReset->next	= reset->next;
    reset->next		= pReset;
    if ( !pReset->next )
	room->reset_last = pReset;
    return;
}



void do_resets( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    char arg5[MAX_INPUT_LENGTH];
    RESET_DATA *pReset = NULL;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    argument = one_argument( argument, arg4 );
    argument = one_argument( argument, arg5 );

    /*
     * Display resets in current room.
     * -------------------------------
     */

    if ( arg1[0] == '\0' ) {
	if ( ch->in_room->reset_first ) {
	    display_resets( ch );
	} else {
	    send_to_char( "No resets in this room.\r\n", ch );
       }
    }

    if ( !IS_BUILDER( ch, ch->in_room->area ) ) {
	send_to_char( "Resets: Invalid security for editing this area.\r\n",
                      ch );
	return;
    }

    /*
     * Take index number and search for commands.
     * ------------------------------------------
     */
    if ( is_number( arg1 ) )
    {
	ROOM_INDEX_DATA *pRoom = ch->in_room;

	/*
	 * Delete a reset.
	 * ---------------
	 */
	if ( !str_cmp( arg2, "delete" ) )
	{
	    int insert_loc = atoi( arg1 );

	    if ( !ch->in_room->reset_first )
	    {
		send_to_char( "No resets in this area.\r\n", ch );
		return;
	    }

	    if ( insert_loc-1 <= 0 )
	    {
		pReset = pRoom->reset_first;
		pRoom->reset_first = pRoom->reset_first->next;
		if ( !pRoom->reset_first )
		    pRoom->reset_last = NULL;
	    }
	    else
	    {
		int iReset = 0;
		RESET_DATA *prev = NULL;

		for ( pReset = pRoom->reset_first;
		  pReset;
		  pReset = pReset->next )
		{
		    if ( ++iReset == insert_loc )
			break;
		    prev = pReset;
		}

		if ( !pReset )
		{
		    send_to_char( "Reset not found.\r\n", ch );
		    return;
		}

		if ( prev )
		    prev->next = prev->next->next;
		else
		    pRoom->reset_first = pRoom->reset_first->next;

		for ( pRoom->reset_last = pRoom->reset_first;
		  pRoom->reset_last->next;
		  pRoom->reset_last = pRoom->reset_last->next );
	    }

	    free_reset_data( pReset );
	    send_to_char( "Reset deleted.\r\n", ch );
	}
	else
	/*
	 * Add a reset.
	 * ------------
	 */
	if ( (!str_cmp( arg2, "mob" ) && is_number( arg3 ))
	  || (!str_cmp( arg2, "obj" ) && is_number( arg3 )) )
	{
/*        send_to_char ("Use ORESET or MRESET please.\r\n.",ch);
	return; */
	    /*
	     * Check for Mobile reset.
	     * -----------------------
	     */
	    if ( !str_cmp( arg2, "mob" ) )
	    {
		pReset = new_reset_data();
		pReset->command = 'M';
		pReset->arg1    = atoi( arg3 );
		pReset->arg2    = is_number( arg4 ) ? atoi( arg4 ) : 1;
		pReset->arg3    = ch->in_room->vnum;
	    }
	    else
	     /*
	     * Check for Object reset.
	     * -----------------------
             */
	    if ( !str_cmp( arg2, "obj" ) )
	    {
		pReset = new_reset_data();
		pReset->arg1    = atoi( arg3 );
	         /*
		 * Inside another object.
		 * ----------------------
                  */
		if ( !str_prefix( arg4, "inside" ) )
		{
		    pReset->command = 'P';
		    pReset->arg2    = 0;
		    pReset->arg3    = is_number( arg5 ) ? atoi( arg5 ) : 1;
		}
		else
		 /*
		 * Inside the room.
		 * ----------------
		 */
		if ( !str_cmp( arg4, "room" ) )
		{
/*		    pReset           = new_reset_data(); */
		    pReset->command  = 'O';
		    pReset->arg2     = 0;
		    pReset->arg3     = ch->in_room->vnum;
		}
		else
		 /*
		 * Into a Mobile's inventory.
		 * --------------------------
		 */
		{
		    if ( flag_value( wear_loc_flags, arg4 ) == NO_FLAG )
		    {
			send_to_char( "Resets: '? wear-loc'\r\n", ch );
			return;
		    }
		 /*   pReset = new_reset_data(); */
		    pReset->arg3 = flag_value( wear_loc_flags, arg4 );
		    if ( pReset->arg2 == WEAR_NONE )
			pReset->command = 'G';
		    else
			pReset->command = 'E';
		}
	    }

	    add_reset( ch->in_room, pReset, atoi( arg1 ) );
	    send_to_char( "Reset added.\r\n", ch );
	}
	else
	{
 	send_to_char( "Syntax: RESET <number> OBJ <vnum> <wear_loc>\r\n", ch );
	send_to_char( "        RESET <number> OBJ <vnum> in <vnum>\r\n", ch );
	send_to_char( "        RESET <number> OBJ <vnum> room\r\n", ch );
	send_to_char( "        RESET <number> MOB <vnum> [<max #>]\r\n", ch );
	send_to_char( "Syntax: RESET <number> DELETE\r\n", ch );
	}
    }

    return;
}



/*****************************************************************************
 Name:		do_alist
 Purpose:	Normal command to list areas and display area information.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_alist( CHAR_DATA *ch, char *argument ) {
char buf    [ MAX_STRING_LENGTH ];
char result [ MAX_STRING_LENGTH*8 ];	
AREA_DATA *pArea;
bool show;
char *acol;
char *zcol;
int zid;

    zid = -1;

    if (argument[0] != '\0') {
      zid = atoi(argument);
    } 

    if ( zid < -1 
      || zid >= NUM_ZONES ) {
      send_to_char("Invalid zone id!\r\n", ch);
      return;
    }

    strcpy(result,
       "Aid Levels  Name                           Zid Zone\r\n"
       "--- ------- ------------------------------ --- ---------------\r\n");

    for ( pArea = area_first; pArea; pArea = pArea->next ) {
    
      show = TRUE;
      acol = "{g";
      zcol = "{g";

      if ( IS_SET(pArea->area_flags, AREA_SECRET)) {

        acol = "{r";

        if ( !IS_IMMORTAL(ch) ) {
      
          show = FALSE;

          if ( ch->in_room != NULL
            && ch->in_room->area != NULL
            && ch->in_room->area == pArea ) {
            show = TRUE;
            acol = "{g";
          }
        }
      } 
        
      if ( IS_SET(pArea->area_flags, AREA_LOCKED)) {

        acol = "{m";

        if ( !IS_IMMORTAL(ch) ) {
      
          show = FALSE;

          if ( ch->in_room != NULL
            && ch->in_room->area != NULL
            && ch->in_room->area == pArea ) {
            show = TRUE;
            acol = "{m";
          }
        }
      } 
        
      if ( IS_SET(zones[pArea->zone].flags, ZONE_SECRET)) {

        zcol = "{r";

        if ( !IS_IMMORTAL(ch) ) {
      
          show = FALSE;

          if ( ch->in_room != NULL
            && ch->in_room->area != NULL
            && ch->in_room->area == pArea ) {
            zcol = "{g";
            show = TRUE; 
          }
        }
      } 
 
      if ( zid != -1
        && pArea->zone != zid ) {
         show = FALSE;
      }  
     
      if (show) {  
        sprintf(buf, "{y%3d %s%-38.38s {y%3d %s%s{x\r\n", pArea->vnum, acol, pArea->name, pArea->zone, zcol, zones[pArea->zone].name);
        strcat(result, buf);

      if (IS_IMMORTAL(ch)) {
          sprintf(buf, "    {cvnums: %5ld - %5ld,  Security: %2d,  Builders: %s{x\r\n", pArea->lvnum, pArea->uvnum, pArea->security, pArea->builders );
	  strcat( result, buf );
        }
      }
    }

    if ( ch->in_room == NULL
      || ch->in_room->area == NULL ) {
      strcat(result, "\r\n{RYou are not currently in any area!{x\r\n"); 
    } else {
      sprintf(buf, "You are currently in area {y%d{x - {g%s{x\r\n",
                    ch->in_room->area->vnum,
                    ch->in_room->area->name);
      strcat(result, buf);
    }
 
    page_to_char( result, ch );
    return;
}


/*****************************************************************************
 Name:		do_ainfo
 Purpose:	Different format alist
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_ainfo( CHAR_DATA *ch, char *argument ) {
char buf    [ MAX_STRING_LENGTH ];
char result [ MAX_STRING_LENGTH*8 ];	/* May need tweaking. */
AREA_DATA *pArea;
bool show;
char *acol;

    strcpy(result,
       "Aid LVnum UVnum File         Zid Level   Name\r\n"
       "--- ----- ----- ------------ --- ------- --------------------\r\n");

    for ( pArea = area_first; pArea; pArea = pArea->next ) {
    
      show = TRUE;
      acol = "{g";

      if ( IS_SET(pArea->area_flags, AREA_SECRET)) {

        acol = "{r";

        if ( !IS_IMMORTAL(ch) ) {
      
          show = FALSE;

          if ( ch->in_room != NULL
            && ch->in_room->area != NULL
            && ch->in_room->area == pArea ) {
            show = TRUE;
            acol = "{g";
          }
        }
      } 
        
      if ( IS_SET(pArea->area_flags, AREA_LOCKED)) {

        acol = "{m";

        if ( !IS_IMMORTAL(ch) ) {
      
          show = FALSE;

          if ( ch->in_room != NULL
            && ch->in_room->area != NULL
            && ch->in_room->area == pArea ) {
            show = TRUE;
            acol = "{m";
          }
        }
      } 
        
      if (show) {  
        sprintf(buf, "{y%3d {c%5ld %5ld {w%-12.12s {y%3d %s%-38.38s{x\r\n",  pArea->vnum, pArea->lvnum, pArea->uvnum, pArea->filename, pArea->zone, acol, pArea->name);
        strcat(result, buf);
      } 
    }

    if ( ch->in_room == NULL
      || ch->in_room->area == NULL ) {
      strcat(result, "\r\n{RYou are not currently in any area!{x\r\n"); 
    } else {
      sprintf(buf, "You are currently in area {y%d{x - {g%s{x\r\n",  ch->in_room->area->vnum, ch->in_room->area->name);
      strcat(result, buf);
    }
 
    page_to_char( result, ch );

    return;
}

/*****************************************************************************
 Name:		do_asplit
 Purpose:	Split an area in two
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_asplit(CHAR_DATA *ch, char *args) {

  AREA_DATA *base_area, *high_area, *area ;

  char num[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
 
  int avnum;		// Vnum of area
  int svnum;		// Vnum to split at

  bool all_above;
  bool all_below;

  int hash;

  ROOM_INDEX_DATA *room;
  OBJ_INDEX_DATA *obj;
  MOB_INDEX_DATA *mob;

  CHAR_DATA *mb;

 /* Check for help... */

  if ( args[0] == '\0'
    || args[0] == '?' ) {
    send_to_char("Syntax: asplit area_vnum split_vnum\r\n", ch);
    return;
  }

 /* Find out what we are chopping up... */ 

  args = one_argument(args, num);

  if (!is_number(num)) {
    send_to_char("That is not a valid area vnum!\r\n", ch);
    return;
  }
 
  avnum = atoi(num);

 /* Now find the split vnum... */

  if (!is_number(args)) {
    send_to_char("That is not a proper split vnum!\r\n", ch);
    return;
  } 

  svnum = atoi(args);

 /* Now find the area... */

  base_area = NULL;

  area = area_first;

  while ( area != NULL 
       && base_area == NULL ) {

    if ( area->vnum == avnum ) {
      base_area = area;
    }
  
    area = area->next;
  }

 /* Complain if not found... */  

  if ( base_area == NULL ) {
    send_to_char("No area with that vnum!\r\n", ch);
    return;
  }

 /* Are we allowed to do this? */

  if ( IS_SET(base_area->area_flags, AREA_EDLOCK) ) {
    send_to_char("{YArea is edit locked!{x\r\n", ch);
    return;
  }

 /* Check we have a sensible split request... */

  if ( svnum <= (base_area->lvnum + 1)
    || svnum >=  base_area->uvnum ) {
    send_to_char("Split vnum must be inside the area!\r\n", ch);   
    return;
  }

  if ( base_area->lvnum >= (base_area->uvnum - 3) ) {
    send_to_char("Area is to small to split!\r\n", ch);
    return;
  }

 /* Now check for a simple request... */

  send_to_char("Checking...\r\n", ch);

  all_above = TRUE;
  all_below = TRUE;

 /* Check rooms... */

  hash = 0;

  while ( hash < MAX_KEY_HASH
       && ( all_above | all_below ) ) {

    room  = room_index_hash[hash];

    while ( room != NULL
         && ( all_above | all_below ) ) {

      if ( room->area == base_area ) {

        if ( room->vnum >= svnum ) {
          all_below = FALSE;
        } else {
          all_above = FALSE;
        }  
      }

      room  = room->next;
    }

    hash++;
  }	

 /* Check objects... */ 
 
  hash = 0;

  while ( hash < MAX_KEY_HASH
       && ( all_above | all_below ) ) {

    obj  = obj_index_hash[hash];

    while ( obj != NULL
         && ( all_above | all_below ) ) {

      if ( obj->area == base_area ) {

        if ( obj->vnum >= svnum ) {
          all_below = FALSE;
        } else {
          all_above = FALSE;
        }  
      }

      obj  = obj->next;
    }

    hash++;
  }	
 
 /* Check mobiles... */ 
 
  hash = 0;

  while ( hash < MAX_KEY_HASH
       && ( all_above | all_below ) ) {

    mob  = mob_index_hash[hash];

    while ( mob != NULL
         && ( all_above | all_below ) ) {

      if ( mob->area == base_area ) {

        if ( mob->vnum >= svnum ) {
          all_below = FALSE;
        } else {
          all_above = FALSE;
        }  
      }

      mob  = mob->next;
    }

    hash++;
  }	

 /* Ok, what did we find... */ 

  if ( all_below & all_above ) {
    base_area->uvnum = (svnum - 1);
    SET_BIT(base_area->area_flags, AREA_CHANGED);
    send_to_char("{YArea empty, upper vnum reduced.{x\r\n", ch);
    send_to_char("{YRemember to ASAVE CHANGED{x\r\n", ch);
    return;
  } 
  
  if ( all_below ) {
    base_area->uvnum = (svnum - 1);
    SET_BIT(base_area->area_flags, AREA_CHANGED);  
    send_to_char("{YArea empty above split, upper vnum reduced.{x\r\n", ch);
    send_to_char("{YRemember to ASAVE CHANGED{x\r\n", ch);
    return;
  } 
  
  if ( all_above ) {
    base_area->lvnum = svnum;
    SET_BIT(base_area->area_flags, AREA_CHANGED);  
    send_to_char("{YArea empty below split, lower vnum increased.{x\r\n", ch);
    send_to_char("{YRemember to ASAVE CHANGED{x\r\n", ch);
    return;
  } 

 /* Oh-Oh, time for do something... */

  send_to_char("{YSplitting...\r\n", ch); 

 /* First we need a new area... */

  high_area		=   new_area();
  area_last->next	=   high_area;
  area_last		=   high_area;	/* Thanks, Walker. */

 /* Put them into aedit on the new area... */

  ch->desc->pEdit     =   (void *)high_area;

 /* Then we clone some of the areas fields... */

  sprintf(buf, "%s - split", base_area->name);

  free_string(high_area->name);
  high_area->name = str_dup(buf);

  free_string(high_area->copyright);
  high_area->copyright = str_dup(base_area->copyright);

  free_string(high_area->builders);
  high_area->builders = str_dup(base_area->builders);

  free_string(high_area->map);
  high_area->map = str_dup(base_area->map);

  high_area->area_flags	= base_area->area_flags;
  high_area->security	= base_area->security;
  high_area->age		= base_area->age;

  high_area->lvnum 	= svnum;
  high_area->uvnum	= base_area->uvnum;

  base_area->uvnum	= (svnum - 1);

  base_area->nplayer 	= 0;
  base_area->empty	= TRUE;

  high_area->nplayer	= 0;
  high_area->empty 	= TRUE;

  high_area->zone	= base_area->zone;
  high_area->recall	= base_area->recall;
  high_area->respawn	= base_area->respawn;
  high_area->morgue	= base_area->morgue;
  high_area->dream	= base_area->dream;
  high_area->mare   	= base_area->mare;

  SET_BIT( high_area->area_flags, AREA_ADDED );

 /* Well, that was the easy bit. 
    Now we have to find all of the indexes that are in the wrong area... */

 /* Split rooms... */

  hash = 0;

  while ( hash < MAX_KEY_HASH ) {

    room  = room_index_hash[hash];

    while ( room != NULL ) {

      if ( room->area == base_area ) {

        if ( room->vnum > base_area->uvnum ) {
          room->area = high_area;
        }

        mb = room->people;

        while ( mb != NULL ) {

          if ( !IS_NPC(mb) ) {
            room->area->nplayer += 1;
            room->area->empty = FALSE;
          }

          mb = mb->next_in_room;
        }
      }

      room  = room->next;
    }

    hash++;
  }	

 /* Check objects... */ 
 
  hash = 0;

  while ( hash < MAX_KEY_HASH ) {

    obj  = obj_index_hash[hash];

    while ( obj != NULL ) {

      if ( obj->area == base_area
        && obj->vnum > base_area->uvnum ) {
        obj->area = high_area;
      }

      obj  = obj->next;
    }

    hash++;
  }	
 
 /* Check mobiles... */ 
 
  hash = 0;

  while ( hash < MAX_KEY_HASH ) {

    mob  = mob_index_hash[hash];

    while ( mob != NULL ) {

      if ( mob->area == base_area
        && mob->vnum > base_area->uvnum ) {
        mob->area = high_area;
      }

      mob  = mob->next;
    }

    hash++;
  }	

 /* Tell them we are done and get out... */
  
  SET_BIT(base_area->area_flags, AREA_CHANGED);  
  SET_BIT(high_area->area_flags, AREA_CHANGED);  

  send_to_char("{YArea split...{x\r\n", ch);
  send_to_char("{YASAVE WORLD to complete...{x\r\n", ch);

  return;
};

/* External rebase functions, coded in olc_act.c */

extern void rebase_room(	ROOM_INDEX_DATA *pRoom, 
				int new_vnum, 
				AREA_DATA *new_area);

extern void rebase_object(	OBJ_INDEX_DATA *pObj, 
				int new_vnum, 
				AREA_DATA *new_area);

extern void rebase_mob(		MOB_INDEX_DATA *pMob, 
				int new_vnum, 
				AREA_DATA *new_area);

/*****************************************************************************
 Name:		do_arebase
 Purpose:	Change ALL the vnums in an area
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_arebase(CHAR_DATA *ch, char *args) {

  AREA_DATA *base_area, *area ;

  char num[MAX_STRING_LENGTH];
 
  int avnum;		// Vnum of area
  int svnum;		// Vnum to rebase at
  int range;		// Size of the area

  ROOM_INDEX_DATA *room;
  OBJ_INDEX_DATA *obj;
  MOB_INDEX_DATA *mob;

  int i;

 /* Check for help... */

  if ( args[0] == '\0'
    || args[0] == '?' ) {
    send_to_char("Syntax: arebase area_vnum new_base_vnum\r\n", ch);
    return;
  }

 /* Find out what we are chopping up... */ 

  args = one_argument(args, num);

  if (!is_number(num)) {
    send_to_char("That is not a valid area vnum!\r\n", ch);
    return;
  }
 
  avnum = atoi(num);

 /* Now find the split vnum... */

  if (!is_number(args)) {
    send_to_char("That is not a proper base vnum!\r\n", ch);
    return;
  } 

  svnum = atoi(args);

 /* Now find the area... */

  base_area = NULL;

  area = area_first;

  while ( area != NULL 
       && base_area == NULL ) {

    if ( area->vnum == avnum ) {
      base_area = area;
    }
  
    area = area->next;
  }

 /* Complain if not found... */  

  if ( base_area == NULL ) {
    send_to_char("No area with that vnum!\r\n", ch);
    return;
  }

 /* Are we allowed to do this? */

  if ( IS_SET(base_area->area_flags, AREA_EDLOCK) ) {
    send_to_char("{YArea is edit locked!{x\r\n", ch);
    return;
  }

 /* Now check if the request is possible... */

  range = base_area->uvnum - base_area->lvnum;

  area = area_first;

  while ( area != NULL ) {

    if ( area->uvnum >= svnum 
      && area->lvnum <= svnum + range ) {
      send_to_char("{YArea will not fit at that base!{x\r\n", ch);
      return;
    }
  
    area = area->next;
  }

 /* Now we know it is possible... */

  for ( i = 0; i <= range; i++) {

    room = get_room_index(base_area->lvnum + i);

    if ( room != NULL ) {
      rebase_room( room, svnum + i, base_area);
    }

    mob = get_mob_index(base_area->lvnum + i);

    if ( mob != NULL ) {
      rebase_mob( mob, svnum + i, base_area);
    }

    obj = get_obj_index(base_area->lvnum + i);

    if ( obj != NULL ) {
      rebase_object( obj, svnum + i, base_area);
    }

  }

  base_area->lvnum = svnum;
  base_area->uvnum = svnum + range;

 /* Hopefully that worked... */

  send_to_char("{YArea rebased...{x\r\n", ch);
  send_to_char("{YASAVE WORLD and REBOOT to complete...{x\r\n", ch);

  return;
}
 
/*****************************************************************************
 Name:		do_amerge
 Purpose:	Join two contiguous areas together
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_amerge(CHAR_DATA *ch, char *args) {

  AREA_DATA *base_area, *high_area, *area;

  char num[MAX_STRING_LENGTH];
 
  int avnum;		// Vnum of first area
  int svnum;		// Vnum to second area

  int i;

  ROOM_INDEX_DATA *room;
  OBJ_INDEX_DATA *obj;
  MOB_INDEX_DATA *mob;

 /* Check for help... */

  if ( args[0] == '\0'
    || args[0] == '?' ) {
    send_to_char("Syntax: amerge low_area_vnum high_area_vnum\r\n", ch);
    return;
  }

 /* Find out what we are chopping up... */ 

  args = one_argument(args, num);

  if (!is_number(num)) {
    send_to_char("That is not a valid area vnum!\r\n", ch);
    return;
  }
 
  avnum = atoi(num);

 /* Now find the split vnum... */

  if (!is_number(args)) {
    send_to_char("That is not a valid area vnum!\r\n", ch);
    return;
  }

  svnum = atoi(args);

 /* Now find the area... */

  base_area = NULL;
  high_area = NULL;

  area = area_first;

  while ( area != NULL 
       && base_area == NULL
       && high_area == NULL ) {

    if ( area->vnum == avnum ) {
      base_area = area;
    }
  
    if ( area->vnum == svnum ) {
      high_area = area;
    }
  
    area = area->next;
  }

 /* Complain if not found... */  

  if ( base_area == NULL ) {
    send_to_char("Low area not found!\r\n", ch);
    return;
  }

  if ( high_area == NULL ) {
    send_to_char("High area not found!\r\n", ch);
    return;
  }

 /* Are we allowed to do this? */

  if ( IS_SET(base_area->area_flags, AREA_EDLOCK) ) {
    send_to_char("{YLow area is edit locked!{x\r\n", ch);
    return;
  }

  if ( IS_SET(high_area->area_flags, AREA_EDLOCK) ) {
    send_to_char("{YHigh area is edit locked!{x\r\n", ch);
    return;
  }

 /* Check we have a sensible request... */

  if (high_area->lvnum != base_area->uvnum + 1 ) {
    send_to_char("Areas are not contiguous!\r\n", ch);
    send_to_char("Low Area Upper Vnum != High Area Low Vnum - 1\r\n", ch);
    return;
  }

 /* Oh-Oh, time for do something... */

  send_to_char("{YSplitting...\r\n", ch); 

 /* Now we have to find all of the indexes that are in the wrong area... */

 /* Merge everything... */

  for ( i = high_area->lvnum; i <= high_area->uvnum; i++) {

    room = get_room_index(i);

    if ( room != NULL ) {
      room->area = base_area;
    }

    mob = get_mob_index(i);

    if ( mob != NULL ) {
      mob->area = base_area;
    }

    obj = get_obj_index(i);

    if ( obj != NULL ) {
      obj->area = base_area;
    }

  }

  base_area->uvnum = high_area->uvnum;

  high_area->lvnum = 0;
  high_area->uvnum = 0;

 /* Tell them we are done and get out... */
  
  SET_BIT(base_area->area_flags, AREA_CHANGED);  
  SET_BIT(high_area->area_flags, AREA_CHANGED);  

  send_to_char("{YAreas merged...{x\r\n", ch);
  send_to_char("{YASAVE WORLD to complete...{x\r\n", ch);

  return;
};


/* Find next free mob vnum in an area... */

int next_mob_vnum(AREA_DATA *area, int vnum) {

  MOB_INDEX_DATA *mob;

  while ( vnum >= area->lvnum
       && vnum <= area->uvnum ) {

    mob = get_mob_index(vnum);

    if (mob == NULL) {
      return vnum;
    }

    vnum++;
  }

  return 0;
}


/* Find next free object vnum in an area... */

int next_obj_vnum(AREA_DATA *area, int vnum) {

  OBJ_INDEX_DATA *obj;

  while ( vnum >= area->lvnum
       && vnum <= area->uvnum ) {

    obj = get_obj_index(vnum);

    if (obj == NULL) {
      return vnum;
    }

    vnum++;
  }

  return 0;
}


/* Find out if a vnum comes from the system or limbo areas... */

bool system_vnum(int vnum) {

  if ( vnum >= VNUM_LIMBO_LOW
    && vnum <= VNUM_LIMBO_HIGH ) {
    return TRUE;
  }
 
  if ( vnum >= VNUM_SYSTEM_LOW
    && vnum <= VNUM_SYSTEM_HIGH ) {
    return TRUE;
  }
 
  return FALSE;
}

/*****************************************************************************
 Name:		do_asuck
 Purpose:	Suck ALL referenced vnums into an area
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_asuck(CHAR_DATA *ch, char *args) {

  AREA_DATA *area, *base_area;

  char num[MAX_STRING_LENGTH];
 
  int avnum;		// Vnum of area
  int mob_vnum;
  int obj_vnum;

  ROOM_INDEX_DATA *room;
  RESET_DATA *reset;

  OBJ_INDEX_DATA *obj;
  MOB_INDEX_DATA *mob;

  int i;

 /* Check for help... */

  if ( args[0] == '\0'
    || args[0] == '?' ) {
    send_to_char("Syntax: asuck area_vnum\r\n", ch);
    return;
  }

 /* Find out what we are chopping up... */ 

  args = one_argument(args, num);

  if (!is_number(num)) {
    send_to_char("That is not a valid area vnum!\r\n", ch);
    return;
  }
 
  avnum = atoi(num);

 /* Now find the area... */

  base_area = NULL;

  area = area_first;

  while ( area != NULL 
       && base_area == NULL ) {

    if ( area->vnum == avnum ) {
      base_area = area;
    }
  
    area = area->next;
  }

 /* Complain if not found... */  

  if ( base_area == NULL ) {
    send_to_char("No area with that vnum!\r\n", ch);
    return;
  }

 /* Are we allowed to do this? */

  if ( IS_SET(base_area->area_flags, AREA_EDLOCK) ) {
    send_to_char("{YArea is edit locked!{x\r\n", ch);
    return;
  }

 /* Go for it... */

  mob_vnum = base_area->lvnum;
  obj_vnum = base_area->lvnum;

  for ( i = base_area->lvnum; i <= base_area->uvnum; i++) {

    room = get_room_index(i);

    if ( room != NULL ) {

      reset = room->reset_first;

      while ( reset != NULL ) {

        switch ( reset->command ) {
          
          case 'M':

            if ( !system_vnum(reset->arg1) ) {

              mob = get_mob_index(reset->arg1);

              if ( mob != NULL
                && mob->area != base_area ) {

                mob_vnum = next_mob_vnum(base_area, mob_vnum);
 
                if ( mob_vnum == 0 ) {
                   send_to_char("{YNot enough mob vnums!{x\r\n", ch);
                   send_to_char("{YUpdate partially made!{x\r\n", ch);
                   send_to_char("{YASAVE WORLD and REBOOT save!{x\r\n", ch);
                   return;
                }

                rebase_mob( mob, mob_vnum, base_area);
              }
            }
           
            break;

          case 'O':
          case 'G':
          case 'E':
          case 'P':

            if ( !system_vnum(reset->arg1) ) {

              obj = get_obj_index(reset->arg1);

              if ( obj != NULL
                && obj->area != base_area ) {

                obj_vnum = next_obj_vnum(base_area, obj_vnum);
 
                if ( obj_vnum == 0 ) {
                   send_to_char("{YNot enough object vnums!{x\r\n", ch);
                   send_to_char("{YUpdate partially made!{x\r\n", ch);
                   send_to_char("{YASAVE WORLD and REBOOT save!{x\r\n", ch);
                   return;
                }

                rebase_object( obj, obj_vnum, base_area);
              }
            }
           
            break;

          default:
            break;
        }

        reset = reset->next;
      }
    }
  }

 /* Hopefully that worked... */

  send_to_char("{YArea sucked...{x\r\n", ch);
  send_to_char("{YASAVE WORLD and REBOOT to complete...{x\r\n", ch);

  return;
}
 

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
 *  File: olc_act.c                                                        *
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
#include "skill.h"
#include "spell.h"
#include "prof.h"
#include "econd.h"
#include "conv.h"
#include "mob.h"
#include "doors.h"
#include "gadgets.h"
#include "deeds.h"
#include "profile.h"
#include "text.h"
#include "race.h"
#include "vlib.h"
#include "society.h"

/* Return TRUE if area changed, FALSE if not. */
#define REDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define OEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define MEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define AEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )

void unlink_mob_index( MOB_INDEX_DATA *pMob );
void unlink_room_index( ROOM_INDEX_DATA *pRoom );
void unlink_obj_index( OBJ_INDEX_DATA *pObj );
void unlink_reset( ROOM_INDEX_DATA *pRoom, RESET_DATA *pReset );

DECLARE_DO_FUN(do_skills);

struct olc_help_type {
    char *command;
    const void *structure;
    char *desc;
};

bool show_version( CHAR_DATA *ch, char *argument ) {
    send_to_char( VERSION, ch );
    send_to_char( "\r\n", ch );
    send_to_char( AUTHOR, ch );
    send_to_char( "\r\n", ch );
    send_to_char( DATE, ch );
    send_to_char( "\r\n", ch );
    send_to_char( CREDITS, ch );
    send_to_char( "\r\n", ch );
    return FALSE;
}    

/*
 * This table contains help commands and a brief description of each.
 * ------------------------------------------------------------------
 */
const struct olc_help_type help_table[] =
{
    {	"area",		area_flags,	 "Area attributes."		 },
    {	"room",		room_flags,	 "Room attributes."		 },
    {	"sector",	sector_flags,	 "Sector types, terrain."	 },
    {	"exit",		exit_flags,	 "Exit types."			 },
    {	"type",		type_flags,	 "Types of objects."		 },
    {	"extra",		extra_flags,	 "Object attributes."		 },
    {	"wear",		wear_flags,	 "Where to wear object."	 },
    {	"spec",		spec_table,	 "Available special programs." 	 },
    {	"sex",		sex_flags,	 "Sexes."			 },
    {	"act",		act_flags,	 "Mobile attributes."		 },
    {	"affect",		affect_flags,	 "Mobile affects."		 },
    {	"raffect",	raffect_flags,	 "Room affects."		 },
    {	"artifact",	artifact_flags,	 "Artifact special affects."		 },
    {	"wear-loc",	wear_loc_flags,	 "Where mobile wears object."	 },
    {	"spells",		spell_array,	 "Names of current spells." 	 },
    {	"weapon",	weapon_flags,	 "Type of weapon." 		 },
    {	"container",	container_flags,	 "Container status."		 },
    {	"liquid",		liquid_flags,	 "Types of liquids."		 },
    {	"ac",		ac_type,		 "Ac for different attacks."	 },
    {	"form",		form_flags,	 "Mobile body form."	         },
    {	"part",		part_flags,	 "Mobile body parts."		 },
    {	"imm",		imm_flags,	 "Mobile immunity."		 },
    {	"res",		res_flags,	 "Mobile resistance."	         },
    {	"vuln",		vuln_flags,	 "Mobile vlnerability."	         },
    {	"off",		off_flags,	 "Mobile offensive behaviour."	 },
    {	"size",		size_flags,	 "Mobile size."			 },
    { 	"position", 	position_flags, 	 "Mobile positions."             },
    {	"material",	material_type,	 "Material mob/obj is made from."},
    { 	"wclass", 	weapon_class, 	 "Weapon class."                 }, 
    { 	"wtype",  	weapon_type, 	 "Special weapon type."          },
    {  	"apply", 	apply_flags,  	 "Item stat apply types."        },
    {	"attack",		attack_type,	 "Mobile attack type."	 	 },
    {	"damtype",	damage_type,	 "Mobile damage type."	 	 },
    { 	"nature", 	mob_nature, 	 "Mobile nature."		 },
    { 	"timesub",  	time_sub,	 "Time event subscriptions."	 },
    { 	"discipline",  	discipline_type,	 "Disciplines of magic."	 },
    { 	"tree",  		tree_type,	 "Types of trees."	 },
    { 	"trap",  		trap_class,	 "Types of traps."	 },
    { 	"instrument",	instrument_type,	 "Types of instruments."	 },
    {	"",		0,		 ""				 }
};



/*****************************************************************************
 Name:		show_flag_cmds
 Purpose:	Displays settable flags and stats.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_flag_cmds( CHAR_DATA *ch, const struct flag_type *flag_table ) {
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  flag;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    for (flag = 0; flag_table[flag].name[0] != '\0'; flag++) {
	if ( flag_table[flag].settable ) {
	    sprintf( buf, "%-19.18s", flag_table[flag].name );
	    strcat( buf1, buf );
	    if ( ++col % 4 == 0 ) strcat( buf1, "\r\n" );
	}
    }
 
    if ( col % 4 != 0 ) strcat( buf1, "\r\n" );
    send_to_char( buf1, ch );
    return;
}


/*****************************************************************************
 Name:		show_skill_cmds
 Purpose:	Displays all skill functions.
 		Does remove those damn immortal commands from the list.
 		Could be improved by:
 		(1) Adding a check for a particular class.
 		(2) Adding a check for a level range.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/

// This is actually showsing spells, not skills...

void show_skill_cmds( CHAR_DATA *ch, int tar )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH*2 ];
    int  spn;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    for (spn = 1; spn < MAX_SPELL; spn++)
    {
	if ( !valid_spell(spn))
	    break;

	if ( tar == -1 || spell_array[spn].target == tar )
	{
	    sprintf( buf, "%-19.18s", spell_array[spn].name );
	    strcat( buf1, buf );
	    if ( ++col % 4 == 0 )
		strcat( buf1, "\r\n" );
	}
    }
 
    if ( col % 4 != 0 )
	strcat( buf1, "\r\n" );

    send_to_char( buf1, ch );
    return;
}



/*****************************************************************************
 Name:		show_spec_cmds
 Purpose:	Displays settable special functions.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_spec_cmds( CHAR_DATA *ch )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  spec;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    send_to_char( "Preceed special functions with 'spec_'\r\n\r\n", ch );
    for (spec = 0; spec_table[spec].spec_fun != NULL; spec++)
    {
	sprintf( buf, "%-19.18s", &spec_table[spec].spec_name[5] );
	strcat( buf1, buf );
	if ( ++col % 4 == 0 )
	    strcat( buf1, "\r\n" );
    }
 
    if ( col % 4 != 0 )
	strcat( buf1, "\r\n" );

    send_to_char( buf1, ch );
    return;
}



/*****************************************************************************
 Name:		show_help
 Purpose:	Displays help for many tables used in OLC.
 Called by:	olc interpreters.
 ****************************************************************************/
bool show_help( CHAR_DATA *ch, char *argument ) {
char buf[MAX_STRING_LENGTH];
char arg[MAX_INPUT_LENGTH];
char spell[MAX_INPUT_LENGTH];
int cnt;

    argument = one_argument( argument, arg );
    one_argument( argument, spell );

    /*
     * Display syntax.
     */
    if ( arg[0] == '\0' ) {
	send_to_char( "Syntax:  ? [command]\r\n\r\n", ch );
	send_to_char( "[command]  [description]\r\n", ch );
	for (cnt = 0; help_table[cnt].command[0] != '\0'; cnt++) {
	    sprintf( buf, "%-10.10s -%s\r\n", capitalize( help_table[cnt].command ), help_table[cnt].desc );
	    send_to_char( buf, ch );
	}
	return FALSE;
    }

    /*
     * Find the command, show changeable data.
     * ---------------------------------------
     */

    if (!str_cmp(arg, "race")) {
                int race = RACE_UNDEFINED;
                SOCIETY *society;

                if (spell[0] != '\0') race = get_race_rn(spell);

                if (race != RACE_UNDEFINED) {
                    sprintf_to_char(ch, "{C%s{x\r\n", race_array[race].name);
                    send_to_char("{g------------------------------{x\r\n", ch);
                    sprintf_to_char(ch, "{gAff:{x     %s\r\n", flag_string(affect_flags, race_array[race].aff));
                    sprintf_to_char(ch, "{gAct:{x     %s\r\n", flag_string(act_flags, race_array[race].act));
                    sprintf_to_char(ch, "{gOff:{x     %s\r\n", flag_string(off_flags, race_array[race].act));
                    sprintf_to_char(ch, "{gImm:{x     %s\r\n", flag_string(imm_flags, race_array[race].imm));
                    sprintf_to_char(ch, "{gRes:{x     %s\r\n", flag_string(res_flags, race_array[race].res));
                    sprintf_to_char(ch, "{gVuln:{x    %s\r\n", flag_string(vuln_flags, race_array[race].vuln));
                    sprintf_to_char(ch, "{gForm:{x    %s\r\n", flag_string(form_flags, race_array[race].form));
                    sprintf_to_char(ch, "{gParts:{x   %s\r\n", flag_string(part_flags, race_array[race].parts));
                    sprintf_to_char(ch, "{gNature:{x  %s\r\n", flag_string(mob_nature, race_array[race].nature));
                    sprintf_to_char(ch, "{gSize:{x    %s\r\n\r\n", flag_string(size_flags, race_array[race].size));

                    society = find_society_by_id(race_array[race].society);
                    if (society) sprintf_to_char(ch, "%s\r\n", society->desc);

                } else {
   	    send_to_char( "Available races are:", ch );

	    for ( race = 0; race_array[race].name != NULL; race++ ) {
	        if ( ( race % 3 ) == 0 ) send_to_char( "\r\n", ch );
	        sprintf( buf, " %-15s", race_array[race].name );
	        send_to_char( buf, ch );
	    }
	    send_to_char( "\r\n", ch );
                }
	return FALSE;
    }

    for (cnt = 0; help_table[cnt].command[0] != '\0'; cnt++) {
        if (arg[0] == help_table[cnt].command[0] && !str_prefix( arg, help_table[cnt].command ))	{
	    if ( help_table[cnt].structure == spec_table)   {
		show_spec_cmds( ch );
		return FALSE;

	    } else if ( help_table[cnt].structure != spell_array ) {
		show_flag_cmds( ch, (flag_type *) help_table[cnt].structure );
		return FALSE;
	    }

	    if ( spell[0] == '\0' )  {
		send_to_char( "Syntax:  ? spells "
		           "[ignore/attack/defend/self/object/all]\r\n", ch );
		return FALSE;
	    }

	    if ( !str_prefix( spell, "all" )) show_skill_cmds( ch, -1 );
	    else if ( !str_prefix( spell, "ignore")) show_skill_cmds( ch, TAR_IGNORE );
	    else if ( !str_prefix( spell, "attack")) show_skill_cmds( ch, TAR_CHAR_OFFENSIVE );
	    else if ( !str_prefix( spell, "defend")) show_skill_cmds( ch, TAR_CHAR_DEFENSIVE );
	    else if ( !str_prefix( spell, "self")) show_skill_cmds( ch, TAR_CHAR_SELF );
	    else if ( !str_prefix( spell, "object")) show_skill_cmds( ch, TAR_OBJ_INV );
	    else send_to_char( "Syntax:  ? spell "
		        "[ignore/attack/defend/self/object/all]\r\n", ch );
		    
	    return FALSE;
	}
    }

    show_help( ch, "" );
    return FALSE;
}

bool show_types (CHAR_DATA *ch, char *args) {

  int i;

  char buf[MAX_STRING_LENGTH];

  send_to_char(" Type   Meaning                     "
               " Type   Meaning\r\n"
               " -----  --------------------------- "
               " -----  ---------------------------\r\n", ch);

  i = 0;

  while ( type_flags[i].name != NULL
       && type_flags[i].name[0] != '\0'  ) {

    sprintf(buf, " %5lld  %-27s ",
                  type_flags[i].bit,
                  type_flags[i].name);
 
    send_to_char(buf, ch);

    if (i % 2 == 1) {
      send_to_char("\r\n", ch);
    } 

    i += 1;
  }

  send_to_char("\r\n", ch);

  return FALSE;
}


REDIT( redit_mlist ) {
    MOB_INDEX_DATA	*pMobIndex;
    AREA_DATA		*pArea;
    char		buf  [ MAX_STRING_LENGTH   ];
    char		buf1 [ MAX_STRING_LENGTH*2 ];
    char		vb   [ MAX_STRING_LENGTH   ];
    char		arg  [ MAX_INPUT_LENGTH    ];
    bool fAll, found;
    int vnum;
    int  col = 0;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax:  mlist <all/name>\r\n", ch );
	return FALSE;
    }

    pArea = ch->in_room->area;
    buf1[0] = '\0';
    fAll    = !str_cmp( arg, "all" );
    found   = FALSE;

    for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )  {
	if ( ( pMobIndex = get_mob_index( vnum ) ) )	{
	    if ( fAll || is_name( arg, pMobIndex->player_name ) )
	    {
		found = TRUE;
		format_vnum(pMobIndex->vnum, vb);
		sprintf( buf, "[%11s] %-23.16s",  vb, capitalize( pMobIndex->short_descr ) );
		strcat( buf1, buf );
		if ( ++col % 2 == 0 )
		    strcat( buf1, "\r\n" );
	    }
	}
    }

    if ( !found )
    {
	send_to_char( "Mobile(s) not found in this area.\r\n", ch);
	return FALSE;
    }

    if ( col % 3 != 0 )
	strcat( buf1, "\r\n" );

    send_to_char( buf1, ch );
    return FALSE;
}



REDIT( redit_olist )
{
    OBJ_INDEX_DATA	*pObjIndex;
    AREA_DATA		*pArea;
    char		buf  [ MAX_STRING_LENGTH   ];
    char		vb   [ MAX_STRING_LENGTH   ];
    char		buf1 [ MAX_STRING_LENGTH*2 ];
    char		arg  [ MAX_INPUT_LENGTH    ];
    bool fAll, found;
    int vnum;
    int  col = 0;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax:  olist <all/name/item_type>\r\n", ch );
	return FALSE;
    }

    pArea = ch->in_room->area;
    buf1[0] = '\0';
    fAll    = !str_cmp( arg, "all" );
    found   = FALSE;

    for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) )
	{
	    if ( fAll || is_name( arg, pObjIndex->name )
	    || flag_value( type_flags, arg ) == pObjIndex->item_type )
	    {
		found = TRUE;
		format_vnum(pObjIndex->vnum, vb);
		sprintf( buf, "[%11s] %-23.16s",
		    vb, capitalize( pObjIndex->short_descr ) );
		strcat( buf1, buf );
		if ( ++col % 2 == 0 )
		    strcat( buf1, "\r\n" );
	    }
	}
    }

    if ( !found )
    {
	send_to_char( "Object(s) not found in this area.\r\n", ch);
	return FALSE;
    }

    if ( col % 3 != 0 )
	strcat( buf1, "\r\n" );

    send_to_char( buf1, ch );
    return FALSE;
}



REDIT( redit_mshow )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] == '\0' ) {
	send_to_char( "Syntax:  mshow <vnum>\r\n", ch );
	return FALSE;
    }

    if ( is_number( argument ) ) {

	value = atoi( argument );

	pMob = get_mob_index( value );

	if ( pMob == NULL ) {
	    send_to_char( "REdit:  That mobile does not exist.\r\n", ch );
	    return FALSE;
	}

	ch->desc->pEdit = (void *)pMob;
 
        medit_show( ch, argument );

        ch->desc->pEdit = (void *)ch->in_room;

    }

    send_to_char( "Syntax:  mshow <vnum>\r\n", ch );

    return FALSE; 
}



REDIT( redit_oshow )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] == '\0' ) {
	send_to_char( "Syntax:  oshow <vnum>\r\n", ch );
	return FALSE;
    }

    if ( is_number( argument ) ) {

	value = atoi( argument );

	pObj = get_obj_index( value );

	if ( pObj == NULL ) {
	    send_to_char( "REdit:  That object does not exist.\r\n", ch );
	    return FALSE;
	}

	ch->desc->pEdit = (void *)pObj;
 
        oedit_show( ch, argument );

        ch->desc->pEdit = (void *)ch->in_room;
    }

    send_to_char( "Syntax:  oshow <vnum>\r\n", ch );

    return FALSE; 
}



/*****************************************************************************
 Name:		check_range( lower vnum, upper vnum )
 Purpose:	Ensures the range spans only one area.
 Called by:	aedit_vnum(olc_act.c).
 ****************************************************************************/
bool check_range(VNUM lower, VNUM upper ) {
AREA_DATA *pArea;
int cnt = 0;

    for ( pArea = area_first; pArea; pArea = pArea->next ) {
        if ( ( lower <= pArea->lvnum && pArea->lvnum <= upper )
        ||   ( lower <= pArea->uvnum && pArea->uvnum <= upper ) )  ++cnt;

        if ( cnt > 1 )   return FALSE;
    }
    return TRUE;
}


AREA_DATA *get_vnum_area(VNUM vnum ) {
AREA_DATA *pArea;

    for ( pArea = area_first; pArea; pArea = pArea->next )   {
        if ( vnum >= pArea->lvnum && vnum <= pArea->uvnum ) return pArea;
    }

    return NULL;
}



/*
 * Area Editor Functions.
 */
AEDIT( aedit_show ) {
AREA_DATA *pArea;
char vb   [MAX_STRING_LENGTH];
int i;

    EDIT_AREA(ch, pArea);

    sprintf_to_char(ch, "Name:      [{c%3d{x] {c%s{x\r\n", pArea->vnum, pArea->name );
    sprintf_to_char(ch, "Zone:      [{c%5d{x] {c%s{x\r\n", pArea->zone,  zones[pArea->zone].name );
    sprintf_to_char(ch, "Climate:   [{c%s{x]\r\n", flag_string( area_climate, pArea->climate ) );
    sprintf_to_char(ch, "File:      [{c%s{x]\r\n", pArea->filename );
    sprintf_to_char(ch, "Copyright: [{c%s{x]\r\n", pArea->copyright );

    format_vnum(pArea->recall, vb);
    sprintf_to_char(ch, "Recall:    [{c%11s{x] {c%s{x\r\n", vb, get_room_index( pArea->recall ) ? get_room_index( pArea->recall )->name : "none" );
    format_vnum(pArea->respawn, vb);
    sprintf_to_char(ch, "Respawn:   [{c%11s{x] {c%s{x\r\n", vb, get_room_index( pArea->respawn ) ? get_room_index( pArea->respawn )->name : "none" );
    format_vnum(pArea->morgue, vb);
    sprintf_to_char(ch, "Morgue:    [{c%11s{x] {c%s{x\r\n", vb, get_room_index( pArea->morgue )  ? get_room_index( pArea->morgue )->name : "none" );
    format_vnum(pArea->dream, vb);
    sprintf_to_char(ch, "Dream:     [{c%11s{x] {c%s{x\r\n",  vb, get_room_index( pArea->dream )  ? get_room_index( pArea->dream )->name : "none" );
    format_vnum(pArea->mare, vb);
    switch (int(pArea->mare)) {
       default:
          sprintf_to_char(ch, "Mare:      [{c%11s{x] {c%s{x\r\n",  vb, get_room_index(pArea->mare )->name);
          break;
  
       case 0:
          sprintf_to_char(ch, "Mare:      [{c%11s{x] {c%s{x\r\n",  vb, "none");
          break;

       case -1:
          sprintf_to_char(ch, "Mare:      [{c%11s{x] {c%s{x\r\n",  vb, "safe");
          break;
    }
    format_vnum(pArea->prison, vb);
    sprintf_to_char(ch, "Prison:    [{c%11s{x] {c%s{x\r\n",  vb, get_room_index( pArea->prison)  ? get_room_index( pArea->prison)->name : "none" );
    sprintf_to_char(ch, "Vnums:     [{c%5d{x-{c%5d{x]\r\n",  pArea->lvnum, pArea->uvnum );
    sprintf_to_char(ch, "Age:       [{c%5d{x]\r\n",	pArea->age );
    sprintf_to_char(ch, "Players:   [{c%5d{x]\r\n", pArea->nplayer );
    sprintf_to_char(ch, "Security:  [{c%5d{x]\r\n", pArea->security );
    sprintf_to_char(ch, "Builders:  [{c%s{x]\r\n", pArea->builders );
    sprintf_to_char(ch, "Map:       [{c%s{x]\r\n", pArea->map );
    sprintf_to_char(ch, "Flags:     [{c%s{x]\r\n",  flag_string( area_flags, pArea->area_flags ) );
    if (pArea->martial > 0) {
        send_to_char("Law:       [{cmartial{x}\r\n", ch);
    }
    send_to_char("Currency Excange:\r\n", ch);
    for (i = 0; i < MAX_CURRENCY; i++) {
          if (pArea->worth[i] == 100) sprintf_to_char(ch, "%15s: [{clocal currency{x]\r\n", flag_string(currency_type, i)); 
          else if (pArea->worth[i] == 0) sprintf_to_char(ch, "%15s: [{cnot accepted{x]\r\n", flag_string(currency_type, i)); 
          else if (pArea->worth[i] == -1) sprintf_to_char(ch, "%15s: [{cuses global value{x]\r\n", flag_string(currency_type, i)); 
          else sprintf_to_char(ch, "%15s: [{c%d{x]\r\n", flag_string(currency_type, i), pArea->worth[i]); 
    }

    return FALSE;
}



AEDIT( aedit_reset ) {
AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if (!str_cmp(argument, "!")) {
        reset_area(pArea, TRUE);
        send_to_char( "Area FORCEreset.\r\n", ch );
    } else {
        reset_area(pArea, FALSE);
        send_to_char( "Area reset.\r\n", ch );
    }
    return FALSE;
}


AEDIT( aedit_create ) {
    AREA_DATA *pArea;

    pArea               =   new_area();
    area_last->next     =   pArea;
    area_last		=   pArea;
    ch->desc->pEdit     =   (void *)pArea;

    SET_BIT( pArea->area_flags, AREA_ADDED );
    send_to_char( "Area Created.\r\n", ch );
    return FALSE;
}


AEDIT( aedit_name ) {
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( argument[0] == '\0' )    {
	send_to_char( "Syntax:   name [$name]\r\n", ch );
	return FALSE;
    }

    free_string( pArea->name );
    pArea->name = str_dup( argument );

    send_to_char( "Name set.\r\n", ch );
    return TRUE;
}


AEDIT( aedit_map ) {
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( argument[0] == '\0' )    {
	send_to_char( "Syntax:   map [$name]\r\n", ch );
	return FALSE;
    }

    free_string( pArea->map );
    pArea->map = str_dup( argument );

    send_to_char( "Map set.\r\n", ch );
    return TRUE;
}


AEDIT( aedit_copyr ) {
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( argument[0] == '\0' )  {
	send_to_char( "Syntax:   copyright [copyright]\r\n", ch );
	return FALSE;
    }

    if (pArea->copyright != NULL) {
      free_string( pArea->copyright );
    }

    pArea->copyright = str_dup( argument );

    send_to_char( "Copyright set.\r\n", ch );
    return TRUE;
}


AEDIT( aedit_file ) {
    AREA_DATA *pArea;
    char file[MAX_STRING_LENGTH];
    int i, length;

    EDIT_AREA(ch, pArea);

    one_argument( argument, file );	/* Forces Lowercase */

    if ( argument[0] == '\0' ) {
	send_to_char( "Syntax:  filename [$file]\r\n", ch );
	return FALSE;
    }

    /*
     * Simple Syntax Check.
     */
    length = strlen( argument );
    if ( length > 8 ) {
	send_to_char( "No more than eight characters allowed.\r\n", ch );
	return FALSE;
    }
    
    /*
     * Allow only letters and numbers.
     */
    for ( i = 0; i < length; i++ )   {
	if ( !isalnum( file[i] )) {
	    send_to_char( "Only letters and numbers are valid.\r\n", ch );
	    return FALSE;
	}
    }    

    free_string( pArea->filename );
    strcat( file, ".are" );
    pArea->filename = str_dup( file );

    send_to_char( "Filename set.\r\n", ch );
    return TRUE;
}


AEDIT( aedit_flags ) {
    AREA_DATA *pArea;
    char flags[MAX_INPUT_LENGTH];
    int value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, flags );

    if ( argument[0] == '\0' )   {
        send_to_char( "Syntax:   flags <area flag>\r\n", ch );
        return FALSE;
    }

    if ( ( value = flag_value( area_flags, flags ) ) != NO_FLAG )  {
        TOGGLE_BIT(pArea->area_flags, value);

        send_to_char( "Flag toggled.\r\n", ch );
        return TRUE;
    }

     send_to_char( "Unrecognized flag.\r\n", ch );
    return FALSE;


}


AEDIT( aedit_climate ) {
    AREA_DATA *pArea;
    char climate[MAX_INPUT_LENGTH];
    int value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, climate );

    if ( argument[0] == '\0' ) {
        send_to_char( "Syntax:   climate <climate name>\r\n", ch );
        return FALSE;
    }

    value = flag_value( area_climate, climate );

    if ( value != NO_FLAG ) {
        pArea->climate = value;

        send_to_char( "Climate changed.\r\n", ch );
        return TRUE;
    }

    send_to_char( "Unrecognized climate.\r\n", ch );

    return FALSE;
}


AEDIT( aedit_age ) {
    AREA_DATA *pArea;
    char age[MAX_STRING_LENGTH];

    EDIT_AREA(ch, pArea);

    one_argument( argument, age );

    if ( !is_number( age ) || age[0] == '\0' )   {
	send_to_char( "Syntax:  age [#age]\r\n", ch );
	return FALSE;
    }

    pArea->age = atoi( age );

    send_to_char( "Age set.\r\n", ch );
    return TRUE;
}


AEDIT( aedit_law) {
    AREA_DATA *pArea;
    char arg[MAX_STRING_LENGTH];

    EDIT_AREA(ch, pArea);

    one_argument( argument, arg );

    if (!str_cmp(arg, "martial")) pArea->martial = 1;
    else pArea->martial = 0;

    send_to_char( "Law set.\r\n", ch );
    return TRUE;
}


AEDIT( aedit_currency ) {
AREA_DATA *pArea;
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
int cn;

    EDIT_AREA(ch, pArea);

    argument = one_argument( argument, arg1);
    argument = one_argument( argument, arg2);

    if (!is_number(arg1) || arg1[0] == '\0' )  {
	send_to_char( "Syntax:  currency [number] [worth]\r\n", ch );
	return FALSE;
    }

    cn = atoi(arg1) - 1;
    if (cn < 0 || cn >= MAX_CURRENCY) {
	send_to_char( "Syntax:  currency [number] [worth]\r\n", ch );
	return FALSE;
    }

    pArea->worth[cn] = atoi(arg2);
    sprintf_to_char(ch, "%s worth set.\r\n", flag_string(currency_type, cn));
    return TRUE;
}


/* Recall... */

AEDIT( aedit_recall ) {

    AREA_DATA *pArea;
    char room[MAX_STRING_LENGTH];
    int  value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, room );

    if ( !is_number( argument ) 
      || argument[0] == '\0' ) {
	send_to_char("Syntax:  recall [#rvnum]\r\n", ch );
	return FALSE;
    }

    value = atoi( room );

    if ( value != 0
      && !room_ok_for_recall( value ) ) {
	send_to_char("AEdit:  Room cannot be used for recall!\r\n", ch);
	return FALSE;
    }

    pArea->recall = value;

    if ( value == 0 ) {
      send_to_char("Area Recall location cleared.\r\n", ch);
    } else { 
      send_to_char("Area Recall location set.\r\n", ch );
    }

    return TRUE;
}

/* Respawn... */

AEDIT( aedit_respawn ) {

    AREA_DATA *pArea;
    char room[MAX_STRING_LENGTH];
    int  value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, room );

    if ( !is_number( argument ) 
      || argument[0] == '\0' ) {
	send_to_char( "Syntax:  respawn [#rvnum]\r\n", ch );
	return FALSE;
    }

    value = atoi( room );

    if ( value != 0
      && !room_ok_for_recall( value ) ) {
	send_to_char("AEdit:  Room cannot be used for respawn!\r\n", ch);
	return FALSE;
    }

    pArea->respawn = value;

    if ( value == 0 ) {
      send_to_char("Area Respawn location cleared.\r\n", ch);
    } else { 
      send_to_char("Area Respawn location set.\r\n", ch );
    }

    return TRUE;
}

/* Morgue... */

AEDIT( aedit_morgue ) {

    AREA_DATA *pArea;
    char room[MAX_STRING_LENGTH];
    int  value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, room );

    if ( !is_number( argument ) 
      || argument[0] == '\0' ) {
	send_to_char( "Syntax:  morgue [#rvnum]\r\n", ch );
	return FALSE;
    }

    value = atoi( room );

    if ( value != 0
      && !room_ok_for_recall( value ) ) {
	send_to_char("AEdit:  Room cannot be used for morgue!\r\n", ch);
	return FALSE;
    }

    pArea->morgue = value;

    if ( value == 0 ) {
      send_to_char("Area Morgue location cleared.\r\n", ch);
    } else { 
      send_to_char("Area Morgue location set.\r\n", ch );
    }

    return TRUE;
}

/* Dream... */

AEDIT( aedit_dream ) {

    AREA_DATA *pArea;
    char room[MAX_STRING_LENGTH];
    int  value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, room );

    if ( !is_number( argument ) 
      || argument[0] == '\0' ) {
	send_to_char( "Syntax:  dream [#rvnum]\r\n", ch );
	return FALSE;
    }

    value = atoi( room );

    if ( value != 0
      && !room_ok_for_recall( value ) ) {
	send_to_char("AEdit:  Room cannot be used for dream!\r\n", ch);
	return FALSE;
    }

    pArea->dream = value;

    if ( value == 0 ) {
      send_to_char("Area Dream location cleared.\r\n", ch);
    } else { 
      send_to_char("Area Dream location set.\r\n", ch );
    }

    return TRUE;
}


AEDIT(aedit_prison) {
AREA_DATA *pArea;
ROOM_INDEX_DATA *pRoom;
char room[MAX_STRING_LENGTH];
int  value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, room );

    if ( !is_number( argument ) 
      || argument[0] == '\0' ) {
	send_to_char( "Syntax:  prison [#rvnum]\r\n", ch );
	return FALSE;
    }

    value = atoi( room );
    if (value != 0) {
        pRoom = get_room_index(value);
        if (!pRoom) {
	send_to_char("AEdit:  Room does not exist!\r\n", ch);
	return FALSE;
        }

        if (!IS_SET(pRoom->room_flags, ROOM_NO_RECALL)) {
	send_to_char("AEdit:  Prison must be no_recall!\r\n", ch);
	return FALSE;
        }
    }
    pArea->prison = value;

    if ( value == 0 ) send_to_char("Area Prison location cleared.\r\n", ch);
    else send_to_char("Area Prison location set.\r\n", ch );

    return TRUE;
}


AEDIT( aedit_mare) {
    AREA_DATA *pArea;
    char room[MAX_STRING_LENGTH];
    int  value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, room );

    if ( !is_number( argument ) 
      || argument[0] == '\0' ) {
	send_to_char( "Syntax:  mare [#rvnum]\r\n", ch );
	return FALSE;
    }

    value = atoi( room );

    if ( value  > 0
    && !room_ok_for_recall( value ) ) {
	send_to_char("AEdit:  Room cannot be used for dream!\r\n", ch);
	return FALSE;
    }

    value = UMAX(value, -1);
    pArea->mare = value;

    if ( value == 0 ) {
      send_to_char("Area mare location cleared.\r\n", ch);
    } else if (value == -1) {
      send_to_char("Area Mare safe.\r\n", ch);
    } else { 
      send_to_char("Area Mare location set.\r\n", ch );
    }

    return TRUE;
}

AEDIT( aedit_zone )
{
	AREA_DATA *pArea;
	char zone_str[MAX_STRING_LENGTH];
	int zone;
	char buf[MAX_STRING_LENGTH];
	
	EDIT_AREA (ch, pArea);	

	if (argument == NULL || argument[0] == '\0')
		{
		send_to_char ("Syntax:  zone <number>\r\n",ch);
		return FALSE;
		}	
	
	one_argument (argument, zone_str);
	zone = atoi (zone_str);
	
	if ( zone < 0
          || zone >= NUM_ZONES ) {

		sprintf (buf, "Valid zone range is 0 to %d.\r\n", NUM_ZONES);
		send_to_char (buf,ch);
		return FALSE;
        }

	pArea->zone = zone;

	sprintf (buf, "Zone set to %d.\r\n", zone);
	send_to_char (buf, ch);

	return TRUE;	
}
		

AEDIT( aedit_security )
{
    AREA_DATA *pArea;
    char sec[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int  value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, sec );

    if ( !is_number( sec ) || sec[0] == '\0' )
    {
	send_to_char( "Syntax:  security [#level]\r\n", ch );
	return FALSE;
    }

    value = atoi( sec );

    if ( value > ch->pcdata->security || value < 0 )
    {
	if ( ch->pcdata->security != 0 )
	{
	    sprintf( buf, "Security is 0-%d.\r\n", ch->pcdata->security );
	    send_to_char( buf, ch );
	}
	else
	    send_to_char( "Security is 0 only.\r\n", ch );
	return FALSE;
    }

    pArea->security = value;

    send_to_char( "Security set.\r\n", ch );
    return TRUE;
}



AEDIT( aedit_builder )
{
    AREA_DATA *pArea;
    char name[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    EDIT_AREA(ch, pArea);

    one_argument( argument, name );

    if ( name[0] == '\0' )
    {
	send_to_char( "Syntax:  builder [$name]  -toggles builder\r\n", ch );
	send_to_char( "Syntax:  builder All      -allows everyone\r\n", ch );
	return FALSE;
    }

    name[0] = UPPER( name[0] );

    if ( strstr( pArea->builders, name ) != '\0' )
    {
	pArea->builders = string_replace( pArea->builders, name, "\0" );
	pArea->builders = string_unpad( pArea->builders );

	if ( pArea->builders[0] == '\0' )
	{
	    free_string( pArea->builders );
	    pArea->builders = str_dup( "None" );
	}
	send_to_char( "Builder removed.\r\n", ch );
	return TRUE;
    }
    else
    {
	buf[0] = '\0';
	if ( strstr( pArea->builders, "None" ) != '\0' )
	{
	    pArea->builders = string_replace( pArea->builders, "None", "\0" );
	    pArea->builders = string_unpad( pArea->builders );
	}

	if (pArea->builders[0] != '\0' )
	{
	    strcat( buf, pArea->builders );
	    strcat( buf, " " );
	}
	strcat( buf, name );
	free_string( pArea->builders );
	pArea->builders = string_proper( str_dup( buf ) );

	send_to_char( "Builder added.\r\n", ch );
	return TRUE;
    }

    return FALSE;
}



AEDIT( aedit_vnum ) {
AREA_DATA *pArea;
char lower[MAX_STRING_LENGTH];
char upper[MAX_STRING_LENGTH];
VNUM  ilower;
VNUM  iupper;

    EDIT_AREA(ch, pArea);

    argument = one_argument( argument, lower );
    one_argument( argument, upper );

    if ( !is_number( lower ) || lower[0] == '\0'  || !is_number( upper ) || upper[0] == '\0' )    {
	send_to_char( "Syntax:  vnum [#lower] [#upper]\r\n", ch );
	return FALSE;
    }

    if ( ( ilower = atol( lower ) ) > ( iupper = atol( upper ) ) )    {
	send_to_char( "AEdit:  Upper must be larger then lower.\r\n", ch );
	return FALSE;
    }
    
    if ( !check_range(atol( lower ), atol( upper ) ) )    {
	send_to_char( "AEdit:  Range must include only this area.\r\n", ch );
	return FALSE;
    }

    if ( get_vnum_area( ilower )
    && get_vnum_area( ilower ) != pArea )  {
	send_to_char( "AEdit:  Lower vnum already assigned.\r\n", ch );
	return FALSE;
    }

    pArea->lvnum = ilower;
    send_to_char( "Lower vnum set.\r\n", ch );

    if ( get_vnum_area( iupper )
    && get_vnum_area( iupper ) != pArea )
    {
	send_to_char( "AEdit:  Upper vnum already assigned.\r\n", ch );
	return TRUE;	/* The lower value has been set. */
    }

    pArea->uvnum = iupper;
    send_to_char( "Upper vnum set.\r\n", ch );

    return TRUE;
}



AEDIT( aedit_lvnum ) {
AREA_DATA *pArea;
char lower[MAX_STRING_LENGTH];
VNUM  ilower;
VNUM  iupper;

    EDIT_AREA(ch, pArea);

    one_argument( argument, lower );

    if ( !is_number( lower ) || lower[0] == '\0' )   {
	send_to_char( "Syntax:  lvnum [#lower]\r\n", ch );
	return FALSE;
    }

    if ( ( ilower = atol( lower ) ) > ( iupper = pArea->uvnum ) )  {
	send_to_char( "AEdit:  Value must be less than the uvnum.\r\n", ch );
	return FALSE;
    }
    
    if ( !check_range( ilower, iupper ))  {
	send_to_char( "AEdit:  Range must include only this area.\r\n", ch );
	return FALSE;
    }

    if ( get_vnum_area(ilower)
    && get_vnum_area( ilower ) != pArea ) {
	send_to_char( "AEdit:  Lower vnum already assigned.\r\n", ch );
	return FALSE;
    }

    pArea->lvnum = ilower;
    send_to_char( "Lower vnum set.\r\n", ch );
    return TRUE;
}



AEDIT( aedit_uvnum ) {
AREA_DATA *pArea;
char upper[MAX_STRING_LENGTH];
VNUM  ilower;
VNUM  iupper;

    EDIT_AREA(ch, pArea);

    one_argument( argument, upper );

    if ( !is_number( upper ) || upper[0] == '\0' ) {
	send_to_char( "Syntax:  uvnum [#upper]\r\n", ch );
	return FALSE;
    }

    if ( ( ilower = pArea->lvnum ) > ( iupper = atol( upper ) ) ) {
	send_to_char( "AEdit:  Upper must be larger then lower.\r\n", ch );
	return FALSE;
    }
    
    if ( !check_range( ilower, iupper ) ) {
	send_to_char( "AEdit:  Range must include only this area.\r\n", ch );
	return FALSE;
    }

    if ( get_vnum_area( iupper )
    && get_vnum_area( iupper ) != pArea )
    {
	send_to_char( "AEdit:  Upper vnum already assigned.\r\n", ch );
	return FALSE;
    }

    pArea->uvnum = iupper;
    send_to_char( "Upper vnum set.\r\n", ch );

    return TRUE;
}



/*
 * Room Editor Functions.
 */
REDIT( redit_unused ) {

	ROOM_INDEX_DATA *pRoom;
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *tmp;
	char outbuf[10*MAX_STRING_LENGTH];
	char buf[128];
	VNUM counter; 
        int fcount;

	EDIT_ROOM(ch, pRoom);

	pArea=pRoom->area;

	sprintf (outbuf, "Area [%3d] %s\r\n", pArea->vnum, pArea->name);

	strcat (outbuf, "UNUSED ROOM VNUMS\r\n"
                        "-----------------\r\n");

	counter = pArea->lvnum;
        fcount = 0;

	while (counter <= pArea->uvnum) {
	  if ((tmp = get_room_index(counter)) == NULL) {
	    sprintf (buf, " %5ld,", counter);
	    strcat (outbuf, buf);
                       
            if (++fcount % 10 == 0) {
              strcat(outbuf, "\r\n");
	    }
          }

          counter += 1;
	}
	
        if (fcount % 10 != 0) {
          strcat(outbuf, "\r\n");
        }

	page_to_char (outbuf, ch);

	return TRUE;
}

REDIT( redit_used ) {

	ROOM_INDEX_DATA *pRoom;
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *tmp;
	char outbuf[10*MAX_STRING_LENGTH];
	char buf[128];
	VNUM counter; 

	EDIT_ROOM(ch, pRoom);

	pArea=pRoom->area;

	sprintf (outbuf, "Area [%3d] %s\r\n", pArea->vnum, pArea->name );

	strcat (outbuf, "USED ROOM VNUMS\r\n"
                        "---------------\r\n");
	counter = pArea->lvnum;

        while ( counter < pArea->uvnum 
             && counter - pArea->lvnum < 400) {
	     tmp = get_room_index(counter);
  	     if ( tmp != NULL) {
                         sprintf (buf, "[%5ld] %s\r\n", counter,  ((tmp->name) ? (tmp->name) : ""));
                         strcat (outbuf, buf);
                     }
                     counter += 1;
                }
	page_to_char (outbuf, ch);
	return TRUE;
}

REDIT( redit_show ) {
ROOM_INDEX_DATA	*pRoom;
char buf  [MAX_STRING_LENGTH];
char vb   [MAX_STRING_LENGTH];
char vb2  [MAX_STRING_LENGTH];
char vb3  [MAX_STRING_LENGTH];
char buf1 [2*MAX_STRING_LENGTH];
OBJ_DATA *obj;
CHAR_DATA *rch;
int door;
bool fcnt;
AREA_DATA *area;

    EDIT_ROOM(ch, pRoom);

    buf1[0] = '\0';
    
    sprintf( buf, "Description:\r\n%s", pRoom->description );
    strcat( buf1, buf );

    if ( pRoom->image != NULL ) {
      sprintf(buf, "Image URL: %s\r\n", pRoom->image);
      strcat(buf1, buf);
    }

    area = pRoom->area;

    format_vnum(pRoom->vnum, vb);
    sprintf( buf,   "Room:       [{c%11s{x] {c%s{x\r\n", vb, pRoom->name );
    strcat( buf1, buf );

    if (area != NULL) {
      sprintf( buf, "Area:       [{c%5d{x] {c%s{x  Subarea: [{c%5d{x]\r\n"
                    "Zone:       [{c%5d{x] {c%s{x\r\n",
                     area->vnum, area->name, pRoom->subarea,
                     area->zone, zones[area->zone].name);
      strcat( buf1, buf );
    }

    format_vnum(pRoom->recall, vb);
    format_vnum(pRoom->respawn, vb2);
    sprintf(buf, "Recall:     [{c%11s{x]   Respawn: [{c%11s{x]\r\n", vb, vb2);
    strcat( buf1, buf );

    format_vnum(pRoom->morgue, vb);
    format_vnum(pRoom->dream, vb2);
    format_vnum(pRoom->mare, vb3);
    sprintf(buf, "Morgue:     [{c%11s{x]   Dream  : [{c%11s{x]  Mare  : [{c%11s{x]\r\n", vb,vb2, (pRoom->mare > -1) ? vb3:"safe");
    strcat( buf1, buf );

    format_vnum(pRoom->night, vb);
    format_vnum(pRoom->day, vb2);
    sprintf(buf, "Night :     [{c%11s{x]   Day    : [{c%11s{x]  Rent  : [{c%11ld{x]\r\n",
                  vb,
                  vb2,
                  IS_SET(pRoom->room_flags, ROOM_RENT) ? pRoom->room_rent : 0);
    strcat( buf1, buf );

    sprintf( buf, "Sector:     [{c%s{x]  Heal Rate: [{c%3d{x]  Mana Rate: [{c%3d{x]\r\n",
                   flag_string( sector_flags, pRoom->sector_type ),
                   pRoom->heal_rate,
                   pRoom->mana_rate );
    strcat( buf1, buf );

    sprintf( buf, "Affecting:  [{c%s{x]\r\n", flag_string( raffect_flags, pRoom->affected_by ) );
    strcat( buf1, buf );

    sprintf( buf, "Room flags: [{c%s{x]\r\n",  flag_string( room_flags, pRoom->room_flags ) );
    strcat( buf1, buf );

    if ( pRoom->extra_descr )    {
	EXTRA_DESCR_DATA *ed;

	strcat( buf1, "Desc Kwds: " );
	for ( ed = pRoom->extra_descr; ed; ed = ed->next ) {
                    strcat( buf1, " ["); 
	    strcat( buf1, ed->keyword );
	    strcat( buf1, "]" );
	}
	strcat( buf1, "\r\n" );
    }

    strcat( buf1, "Characters: [" );
    fcnt = FALSE;
    for ( rch = pRoom->people; rch; rch = rch->next_in_room )    {
	one_argument( rch->name, buf );
	strcat( buf1, buf );
	strcat( buf1, " " );
	fcnt = TRUE;
    }

    if ( fcnt )    {
	int end;

	end = strlen(buf1) - 1;
	buf1[end] = ']';
	strcat( buf1, "\r\n" );
    }    else
	strcat( buf1, "none]\r\n" );

    strcat( buf1, "Objects:    [" );
    fcnt = FALSE;
    for ( obj = pRoom->contents; obj; obj = obj->next_content )    {
	one_argument( obj->name, buf );
	strcat( buf1, buf );
	strcat( buf1, " " );
	fcnt = TRUE;
    }

    if ( fcnt )    {
	int end;

	end = strlen(buf1) - 1;
	buf1[end] = ']';
	strcat( buf1, "\r\n" );
    }    else
	strcat( buf1, "none]\r\n" );

    for ( door = 0; door < DIR_MAX; door++ )    {
	EXIT_DATA *pexit;

	pexit = pRoom->exit[door];

	if ( pexit != NULL ) {
	    char word[MAX_INPUT_LENGTH];
	    char reset_state[MAX_STRING_LENGTH];
	    char *state;
	    int i, length;

                    if (pexit->u1.to_room != NULL) {
                       format_vnum( pexit->u1.to_room->vnum, vb);
                    } else {
                       sprintf(vb, "0");
                    }

	    sprintf( buf, "-{c%-9s{x to [{c%11s{x] Key: [{c%5ld{x]", capitalize(dir_name[door]), vb, pexit->key );
	    strcat( buf1, buf );

	    strcpy( reset_state, flag_string( exit_flags, pexit->rs_flags ) );
	    state = flag_string( exit_flags, pexit->exit_info );
	    strcat( buf1, " Exit flags: [{c" );
	    for (; ;) {
		state = one_argument( state, word );

		if ( word[0] == '\0' ) {
		    int end;

		    end = strlen(buf1) - 1;
		    buf1[end] = '{';
		    strcat( buf1, "x]\r\n" );
		    break;
		}

		if ( str_infix( word, reset_state )) {
		    length = strlen(word);
		    for (i = 0; i < length; i++) word[i] = UPPER(word[i]);
		}
		strcat( buf1, word );
		strcat( buf1, " " );
	    }

	    if (pexit->keyword && pexit->keyword[0] != '\0' ) {
		sprintf( buf, "Kwds: [{c%s{x]\r\n", pexit->keyword );
		strcat( buf1, buf );
	    }
	    if (pexit->description && pexit->description[0] != '\0' )  {
		sprintf( buf, "%s", pexit->description );
		strcat( buf1, buf );
	    }
	    if (pexit->transition && pexit->transition[0] != '\0' )  {
		sprintf( buf, "{B%s{x", pexit->transition);
		strcat( buf1, buf );
	    }

                    if (pexit->invis_cond != NULL) { 
                                buf[0] = '\0';
                                print_econd_to_buffer(buf, pexit->invis_cond, "  vis - ", NULL); 
                                strcat(buf1, buf);
                    }
	}
    }

    send_to_char( buf1, ch );
    return FALSE;
}


/* Local function. */
bool change_exit( CHAR_DATA *ch, char *argument, int door ) {
ROOM_INDEX_DATA *pRoom;
char command[MAX_INPUT_LENGTH];
char arg[MAX_INPUT_LENGTH];
int  value, value1;
EXIT_DATA *pexit;
ECOND_DATA *new_ec;
int seq;
COND_DEST_DATA *cdd, *old_cdd;
char buf[MAX_STRING_LENGTH];
char vb[MAX_STRING_LENGTH];
int vnum; 
ROOM_INDEX_DATA *pToRoom;

   /* Find the room we're editing... */ 

    EDIT_ROOM(ch, pRoom);

    /*
     * Set the exit flags, needs full argument.
     * ----------------------------------------
     */

    value = flag_value( exit_flags, argument );

    if ( value != NO_FLAG ) {
    
	short rev;                               

	if ( !pRoom->exit[door] ) {
                     send_to_char ("You must create an exit first!\r\n",ch);
                     return FALSE;
	}

#define EX_NOT_DOOR (EX_ISDOOR | EX_ECHO_ALL | EX_ECHO_SOUND | EX_ECHO_VISION | EX_RANDOM)

        if ( !IS_SET(EX_NOT_DOOR, value)  	
        && !IS_SET(pRoom->exit[door]->rs_flags, EX_ISDOOR) ) {
	  send_to_char ("Redit: exit must be flagged as DOOR first.\r\n",ch);
	  return FALSE;
        }

        if ((IS_SET(value, EX_ISDOOR) && IS_SET(pRoom->exit[door]->rs_flags, EX_RANDOM))  	
        || (IS_SET(value, EX_RANDOM) && IS_SET(pRoom->exit[door]->rs_flags, EX_ISDOOR))) {
	  send_to_char ("Redit: DOOR / RANDOM conflict.\r\n",ch);
	  return FALSE;
        }
  	
  
	/*
	 * Connected room.
	 */
	pToRoom = pRoom->exit[door]->u1.to_room;  
	rev = rev_dir[door];

        if (IS_SET(pToRoom->area->area_flags, AREA_EDLOCK)) {
          send_to_char("{YREDIT: Connecting rooms area is edit locked!{x\r\n", ch);
          return FALSE;
        } 
	
	/*
	 * This room.
	 */
	TOGGLE_BIT(pRoom->exit[door]->rs_flags,  value);
	pRoom->exit[door]->exit_info = pRoom->exit[door]->rs_flags;
	value1 = pRoom->exit[door]->rs_flags;
	if (!pToRoom->exit[rev])  {
		send_to_char ("Exit flags on ONE-WAY exit toggled.\r\n",ch);
		return TRUE;
	}

	pToRoom->exit[rev]->rs_flags=value1;
	pToRoom->exit[rev]->exit_info=value1; 
	
	send_to_char( "Exit flags on TWO-way exit toggled.\r\n", ch );
	return TRUE;
    }

    /*
     * Now parse the arguments.
     */
    argument = one_argument( argument, command );
    one_argument( argument, arg );

    if ( command[0] == '\0' 
    && argument[0] == '\0' ) {
	move_char( ch, door, TRUE );      
	return FALSE;
    }

    if ( command[0] == '?' ) {

	send_to_char("Syntax: [exit]\r\n", ch);
        send_to_char("        [exit] dig [vnum]\r\n", ch);
        send_to_char("        [exit] link [vnum]\r\n", ch);
        send_to_char("        [exit] room [vnum]\r\n", ch);
        send_to_char("        [exit] key [vnum]\r\n", ch);
        send_to_char("        [exit] name [string]\r\n", ch);
        send_to_char("        [exit] description\r\n", ch);
        send_to_char("        [exit] transition\r\n", ch);
        send_to_char("        [exit] vis [condition]\r\n", ch);
        send_to_char("        [exit] delete\r\n", ch);
        send_to_char("        [exit] listcd\r\n", ch);
        send_to_char("        [exit] listcd [seq]\r\n", ch);
        send_to_char("        [exit] addcd [seq] [vnum] [name]\r\n", ch);
        send_to_char("        [exit] setcdcond [seq] [condition]\r\n", ch);
        send_to_char("        [exit] removecd [seq]\r\n", ch);

	return FALSE;
    }

    if ( !str_cmp( command, "delete" ))  {
	short rev;                              
	
	if ( !pRoom->exit[door] ) {
	    send_to_char( "REdit:  Cannot delete a null exit.\r\n", ch );
	    return FALSE;
	}

	rev = rev_dir[door];
	pToRoom = pRoom->exit[door]->u1.to_room;  
	
	if ( !IS_BUILDER( ch, pToRoom->area ) ){
	    send_to_char( "REdit:  Cannot delete link from that area.\r\n", ch );
	    return FALSE;
	}

                if (IS_SET(pToRoom->area->area_flags, AREA_EDLOCK)) {
                    send_to_char("{YREdit: Connecting rooms area is edit locked!{x\r\n", ch);
                    return FALSE;
                } 
	
	if ( pToRoom->exit[rev]) {
	    free_exit( pToRoom->exit[rev] );
	    pToRoom->exit[rev] = NULL;
	}

	free_exit( pRoom->exit[door] );
	pRoom->exit[door] = NULL;

	send_to_char( "Exit unlinked.\r\n", ch );
	return TRUE;

    } if ( !str_cmp( command, "link")) {
	EXIT_DATA *pExit;

	if ( arg[0] == '\0' || !is_number( arg)) {
	    send_to_char( "Syntax:  [direction] link [vnum]\r\n", ch );
	    return FALSE;
	}

	value = atoi( arg );
                pToRoom = get_room_index(value);

	if ( pToRoom == NULL ) {
	    send_to_char( "REdit:  Cannot link to non-existant room.\r\n", ch );
	    return FALSE;
	}

	if ( !IS_BUILDER( ch, pToRoom->area)) {
	    send_to_char( "REdit:  Cannot link to that area.\r\n", ch );
	    return FALSE;
	}

	if ( pToRoom->exit[rev_dir[door]] ) {
	    send_to_char( "REdit:  Remote side's exit already exists.\r\n", ch );
	    return FALSE;
	}

                if (IS_SET(pToRoom->area->area_flags, AREA_EDLOCK)) {
                    send_to_char("{YREdit: Connecting rooms area is edit locked!{x\r\n", ch);
                    return FALSE;
                } 
	
	if ( !pRoom->exit[door]) {
	    pRoom->exit[door] = new_exit();
	}

	pRoom->exit[door]->u1.to_room = pToRoom;

	door                    		= rev_dir[door];
	pExit                   		= new_exit();
	pExit->u1.to_room       	= ch->in_room;
	pExit->orig_door		= door;
	pToRoom->exit[door]     	= pExit;

	send_to_char( "Two-way link established.\r\n", ch );
	return TRUE;

    } if ( !str_cmp( command, "dig" ) ) {
	char buf[MAX_STRING_LENGTH];
	
	if ( arg[0] == '\0' || !is_number( arg )) {
	    send_to_char( "Syntax: [direction] dig <vnum>\r\n", ch );
	    return FALSE;
	}
	
	if (redit_create( ch, arg )) {
 	  sprintf( buf, "link %s", arg );
	  change_exit( ch, buf, door);
                }
 
	return TRUE;

    } if ( !str_cmp( command, "room" ))  {
	if ( arg[0] == '\0' || !is_number( arg ))	{
	    send_to_char( "Syntax:  [direction] room [vnum]\r\n", ch );
	    return FALSE;
	}

	value = atoi( arg );
                pToRoom = get_room_index(value);

	if ( pToRoom == NULL ) {
	    send_to_char( "REdit:  Cannot link to non-existant room.\r\n", ch );
	    return FALSE;
	}

	if ( !IS_BUILDER( ch, pToRoom->area )) {
	    send_to_char( "REdit:  Cannot link to that area.\r\n", ch );
	    return FALSE;
	}

                if (IS_SET(pToRoom->area->area_flags, AREA_EDLOCK)) {
                    send_to_char("{YREdit: Connecting rooms area is edit locked!{x\r\n", ch);
                    return FALSE;
                } 
	
	if ( !pRoom->exit[door]) {
	    pRoom->exit[door] = new_exit();
	}

	pRoom->exit[door]->u1.to_room = pToRoom;
	send_to_char( "One-way link established.\r\n", ch );
	return TRUE;

    } if ( !str_cmp( command, "key")) {
	if ( arg[0] == '\0' || !is_number( arg )) 	{
	    send_to_char( "Syntax:  [direction] key [vnum]\r\n", ch );
	    return FALSE;
	}

	if ( !pRoom->exit[door] ) {
                    send_to_char ("You must create an exit first!\r\n",ch);
                    return FALSE;
	}

	value = atoi( arg );

	if (!get_obj_index( value )) {
	    send_to_char( "REdit:  Item doesn't exist.\r\n", ch );
	    return FALSE;
	}

	if ( get_obj_index( atoi( argument ) )->item_type != ITEM_KEY ) {
	    send_to_char( "REdit:  Key doesn't exist.\r\n", ch );
	    return FALSE;
	}

	pRoom->exit[door]->key = value;
	send_to_char( "Exit key set.\r\n", ch );
	return TRUE;

    } if ( !str_cmp( command, "name")) {
	if ( arg[0] == '\0' ) {
	    send_to_char( "Syntax:  [direction] name [string]\r\n", ch );
	    return FALSE;
	}

	if ( !pRoom->exit[door]) {
                    send_to_char ("You must create an exit first!\r\n",ch);
                    return FALSE;
	}

	free_string( pRoom->exit[door]->keyword );
	pRoom->exit[door]->keyword = str_dup( arg );
	send_to_char( "Exit name set.\r\n", ch );
	return TRUE;

    } if ( !str_prefix( command, "description")) {
	if ( arg[0] == '\0' ) {
	    if ( !pRoom->exit[door]) {
                        send_to_char ("You must create an exit first!\r\n",ch);
                        return FALSE;
	    }
	    string_append( ch, &pRoom->exit[door]->description );
	    return TRUE;
	}

	send_to_char( "Syntax:  [direction] desc\r\n", ch );
	return FALSE;

    } if ( !str_prefix( command, "transition")) {
	if ( arg[0] == '\0' ) {
	    if ( !pRoom->exit[door]) {
                        send_to_char ("You must create an exit first!\r\n",ch);
                        return FALSE;
	    }
	    string_append( ch, &pRoom->exit[door]->transition);
	    return TRUE;
	}

	send_to_char( "Syntax:  [direction] trans\r\n", ch );
	return FALSE;

    } if ( !str_cmp( command, "vis" ))  {
	if ( arg[0] == '\0' ) {
	    send_to_char( "Syntax: [direction] vis <condition>\r\n", ch );
	    return FALSE;
	}

	if ( !pRoom->exit[door] ){
                    send_to_char ("You must create an exit first!\r\n",ch);
                    return FALSE;
	}

       /* None means we should remove all of the conditions... */
 
        if (!str_cmp(arg, "none")) {
            while (pRoom->exit[door]->invis_cond != NULL) {
                new_ec = pRoom->exit[door]->invis_cond->next;
                free_ec(pRoom->exit[door]->invis_cond);
                pRoom->exit[door]->invis_cond = new_ec;
            }
      
            send_to_char("Visability conditions removed.\r\n", ch);
            return TRUE;
        }
 
       /* ..otherwise we go make a new one... */

        new_ec = create_econd(argument, ch);
        if (new_ec != NULL) {
           new_ec->next = pRoom->exit[door]->invis_cond;
           pRoom->exit[door]->invis_cond = new_ec;
        }

        return TRUE;

    } else if (!str_cmp(command, "listcd")) {

     /* Check that there is an exit... */

      pexit = pRoom->exit[door];

      if (pexit == NULL) {
        send_to_char("No exit defined!\r\n", ch);
        return FALSE;
      } 

      if (pexit->cond_dest == NULL) {
        send_to_char("No conditional destinations defined!\r\n", ch);
        return FALSE;
      }

      seq = atoi(arg);

      if (seq == 0) {
        
        send_to_char("Seq   Vnum  Name\r\n"
                     "----------------------------------------------\r\n", ch);

        cdd = pexit->cond_dest;

        while (cdd != NULL) {

          format_vnum(cdd->vnum, vb);
          sprintf(buf, "%5d %11s %s\r\n", cdd->seq,  vb, cdd->name);
          send_to_char(buf,ch);  

          cdd = cdd->next;
        }

        return TRUE;
      }

     /* Ok, listing a specific one... */

      cdd = pexit->cond_dest;

      while ( cdd != NULL  && cdd->seq != seq ) {
          cdd = cdd->next;
      }

     /* Check we found one... */

      if (cdd == NULL) { 
        send_to_char("No cond dest with that sequence number!\r\n", ch);
        return FALSE;
      }

     /* Display time... */

      format_vnum(cdd->vnum, vb);
      sprintf(buf, "Cond Dest [%5d] to room [%11s] - [%s]\r\n", cdd->seq, vb, cdd->name);
      send_to_char(buf, ch);
      print_econd(ch, cdd->cond, "  cond - ");
      return TRUE;

    } else if (!str_cmp(command, "addcd")) {

      pexit = pRoom->exit[door];

      if (pexit == NULL) {
        send_to_char("No exit defined!\r\n", ch);
        return FALSE;
      } 

     /* Grab the sequence number... */

      seq = atoi(arg);

      if (seq <= 0) {
        send_to_char("Syntax: [direction] addcd <seq> <vnum> <name>", ch);
        return FALSE;
      }

     /* Extract vnum... */
 
      argument = one_argument(argument, arg);	// skip seq 
      argument = one_argument(argument, arg);   // this gets vnum

      vnum = atoi(arg);

      if (vnum < 1) {
        send_to_char("Vnum must be > 0\r\n", ch);
        return FALSE;
      }

     /* Check we've got a name... */

      if (argument[0] == '\0') {
        send_to_char("You must provide a name.\r\n", ch);
        return FALSE;
      }   

     /* Go find it... */

      cdd = pexit->cond_dest;

      while ( cdd != NULL && cdd->seq != seq ) {
          cdd = cdd->next;
      }

     /* If none, we need to make a new one... */

      if (cdd == NULL) {
         cdd = get_cdd();
         cdd->seq = seq;
         insert_cdd(cdd, pexit); 
      }

     /* Now update the values... */

      cdd->vnum = vnum;
      cdd->dest = get_room_index(vnum);
      
      if (cdd->name != NULL) free_string(cdd->name);
      cdd->name = strdup(argument);
      return TRUE;

    } else if (!str_cmp(command, "setcdcond")) {

     /* Check that there is an exit... */

      pexit = pRoom->exit[door];

      if (pexit == NULL) {
        send_to_char("No exit defined!\r\n", ch);
        return FALSE;
      } 

     /* Grab the sequence number... */

      seq = atoi(arg);

      if (seq <= 0) {
        send_to_char("Syntax: [direction] setcdcond <seq> <condition>", ch);
        return FALSE;
      }

     /* Go find it... */

      cdd = pexit->cond_dest;

      while ( cdd != NULL && cdd->seq != seq ) {
         cdd = cdd->next;
      }

     /* Check we found one... */

      if (cdd == NULL) { 
        send_to_char("No cond dest with that sequence number!\r\n", ch);
        return FALSE;
      }

     /* Extract condition string and details... */

      argument = one_argument(argument, arg);	// Skip seq
      one_argument(argument, arg);		// Peek at first work

     /* None means we should remove all of the conditions... */
 
      if (!str_cmp(arg, "none")) {
        while (cdd->cond != NULL) {
          new_ec = cdd->cond->next;
          free_ec(cdd->cond);
          cdd->cond = new_ec;
        }
     
        send_to_char("Cond Dest conditions removed.\r\n", ch);
        return TRUE;
      }
 
     /* ..otherwise we go make a new one... */

      new_ec = create_econd(argument, ch);
 
      if (new_ec != NULL) {
        new_ec->next = cdd->cond;
        cdd->cond = new_ec;
      }
   
      return TRUE;

    }else if (!str_cmp(command, "removecd")) {

      pexit = pRoom->exit[door];

      if (pexit == NULL) {
        send_to_char("No exit defined!\r\n", ch);
        return FALSE;
      } 

     /* Grab the sequence number... */

      seq = atoi(arg);

      if (seq <= 0) {
        send_to_char("Syntax: [direction] removecd <seq>", ch);
        return FALSE;
      }

     /* Go find it... */

      cdd = pexit->cond_dest;

      if (cdd == NULL) {
        send_to_char("No cond dests defined!\r\n", ch);
        return FALSE;
      }

     /* Easy case... */

      if (cdd->seq == seq) {
        pexit->cond_dest = cdd->next;
        cdd->next = NULL;		// Important
        free_cdd(cdd);
        
        send_to_char("Cond dest removed.\r\n", ch);
  
        return TRUE;
      }

     /* Gotta search... */

      old_cdd = cdd;
      cdd = cdd->next;

      while ( cdd != NULL && cdd->seq != seq ) {
         old_cdd = cdd; 
         cdd = cdd->next;
      }

     /* Check we found one... */

      if (cdd == NULL) { 
        send_to_char("No cond dest with that sequence number!\r\n", ch);
        return FALSE;
      }

     /* Ok, zappin time... */

      old_cdd->next = cdd->next;
      cdd->next = NULL;
      free_cdd(cdd); 
      send_to_char("Cond dest removed.\r\n", ch);
      return TRUE;
    } 

    do_help(ch, "EXIT");
    return FALSE;
}



REDIT( redit_north )
{
    if ( change_exit( ch, argument, DIR_NORTH ) )
	return TRUE;

    return FALSE;
}



REDIT( redit_south )
{
    if ( change_exit( ch, argument, DIR_SOUTH ) )
	return TRUE;

    return FALSE;
}



REDIT( redit_east )
{
    if ( change_exit( ch, argument, DIR_EAST ) )
	return TRUE;

    return FALSE;
}



REDIT( redit_west )
{
    if ( change_exit( ch, argument, DIR_WEST ) )
	return TRUE;

    return FALSE;
}



REDIT( redit_up )
{
    if ( change_exit( ch, argument, DIR_UP ) )
	return TRUE;

    return FALSE;
}



REDIT( redit_down )
{
    if ( change_exit( ch, argument, DIR_DOWN ) )
	return TRUE;

    return FALSE;
}


REDIT( redit_northeast )
{
    if ( change_exit( ch, argument, DIR_NE ) )
	return TRUE;

    return FALSE;
}


REDIT( redit_southeast )
{
    if ( change_exit( ch, argument, DIR_SE ) )
	return TRUE;

    return FALSE;
}


REDIT( redit_southwest )
{
    if ( change_exit( ch, argument, DIR_SW ) )
	return TRUE;

    return FALSE;
}


REDIT( redit_northwest )
{
    if ( change_exit( ch, argument, DIR_NW ) )
	return TRUE;

    return FALSE;
}

REDIT( redit_in )
{
    if ( change_exit( ch, argument, DIR_IN ) )
	return TRUE;

    return FALSE;
}

REDIT( redit_out )
{
    if ( change_exit( ch, argument, DIR_OUT ) )
	return TRUE;

    return FALSE;
}



REDIT( redit_ed ) {
    ROOM_INDEX_DATA *pRoom;
    EXTRA_DESCR_DATA *ed;
    char command[MAX_INPUT_LENGTH];
    char keyword[MAX_INPUT_LENGTH];
    char *remkeys;
    int id;

    DEED *deed; 
    char *dtype; 

    char buf[MAX_STRING_LENGTH];

    EDIT_ROOM(ch, pRoom);

    argument = one_argument( argument, command );
    remkeys = one_argument( argument, keyword );

    if (command[0] == '\0')    {
        send_to_char( "Syntax:  ed list\r\n", ch);
        send_to_char( "         ed show [id]\r\n", ch);
	send_to_char( "         ed add [keywords]\r\n", ch);
        send_to_char( "         ed change [id] [keywords]\r\n",ch);
	send_to_char( "         ed edit [id]\r\n", ch );
	send_to_char( "         ed format [id]\r\n", ch);
        send_to_char( "         ed cond [id] [condition]\r\n", ch);
	send_to_char( "         ed delete [id]\r\n", ch);
        send_to_char( "         ed deed [id] [deed] [type] [title]", ch);
	return FALSE;
    }

   /* Apply sequential ids to all of the ed records... */

    id = 1;
    for(ed = pRoom->extra_descr; ed != NULL; ed = ed->next) {
      ed->id = id++;
    } 

   /* ed add keywords */

    if ( !str_cmp( command, "add" ) )    {
	if ( keyword[0] == '\0' )	{
	    send_to_char( "Syntax:  ed add [keywords]\r\n", ch );
	    return FALSE;
	}

	ed			=   new_extra_descr();
	ed->keyword		=   str_dup( argument ); /* Mik */
	ed->description		=   str_dup( "" );
	ed->next		=   pRoom->extra_descr;
	pRoom->extra_descr	=   ed;

	string_append( ch, &ed->description );

	return TRUE;
    }

   /* ed change id keywords */

    if ( !str_cmp( command, "change" ) )    {
	if ( keyword[0] == '\0' || remkeys[0] == '\0' )	{
	    send_to_char( "Syntax:  ed change [id] [keywords]\r\n", ch );
	    return FALSE;
	}

        id = atoi(keyword);

	for ( ed = pRoom->extra_descr; ed; ed = ed->next ) {
	    if ( ed->id == id) {
		break;
            } 
	}

	if ( !ed )
	{
	    send_to_char( "No extra description exists with that id.\r\n", ch );
	    return FALSE;
	}

        ed->keyword = str_dup(remkeys); 	

	return TRUE;
    }

   /* ed edit keywords */

    if ( !str_cmp( command, "edit" ) )    {
	if ( keyword[0] == '\0' )	{
	    send_to_char( "Syntax:  ed edit [id]\r\n", ch );
	    return FALSE;
	}

        id = atoi(keyword);

	for ( ed = pRoom->extra_descr; ed; ed = ed->next ) {
	    if ( ed->id == id) {
		break;
            } 
	}

	if ( !ed )
	{
	    send_to_char( "No extra description exists with that id.\r\n", ch );
	    return FALSE;
	}

	string_append( ch, &ed->description );

	return TRUE;
    }

   /* ed delete id */

    if ( !str_cmp( command, "delete" ) )    {
	EXTRA_DESCR_DATA *ped = NULL;

	if ( keyword[0] == '\0' )	{
	    send_to_char( "Syntax:  ed delete [id]\r\n", ch );
	    return FALSE;
	}

        id = atoi(keyword);

        ped = NULL;
	for ( ed = pRoom->extra_descr; ed; ed = ed->next ) {
	    if ( ed->id == id) {
		break;
            } 
            ped = ed;
	}

	if ( !ed )
	{
	    send_to_char( "No extra description exists with that id.\r\n", ch );
	    return FALSE;
	}

	if ( ped == NULL )
	    pRoom->extra_descr = ed->next;
	else
	    ped->next = ed->next;

	free_extra_descr( ed );

	send_to_char( "Extra description deleted.\r\n", ch );
	return TRUE;
    }

   /* ed format keywords */

    if ( !str_cmp( command, "format" ) )    {
	if ( keyword[0] == '\0' )	{
	    send_to_char( "Syntax:  ed format [keyword]\r\n", ch );
	    return FALSE;
	}

        id = atoi(keyword);

	for ( ed = pRoom->extra_descr; ed; ed = ed->next ) {
	    if ( ed->id == id) {
		break;
            } 
	}

	if ( !ed )
	{
	    send_to_char( "No extra description exists with that id.\r\n", ch );
	    return FALSE;
	}

        if (format_text(ed->description, buf)) {
          free_string(ed->description);
          ed->description = str_dup(buf);
        }

	send_to_char( "Extra description formatted.\r\n", ch );
	return TRUE;
    }

   /* ed list */

    if ( !str_cmp( command, "list" ) ) {

      if (pRoom->extra_descr == NULL) {
        send_to_char("There are no extra descriptions defined.\r\n", ch);
        return TRUE;
      }

      for(ed = pRoom->extra_descr; ed != NULL; ed = ed->next) {

        sprintf(buf, " %3d) [%s]\r\n", ed->id, ed->keyword);

        send_to_char(buf, ch);

        if (ed->ec != NULL) { 
           print_econd(ch, ed->ec, "      Condition - "); 
        }

        deed = ed->deed;
       
        while (deed != NULL) {

          switch (deed->type) {
            case DEED_SECRET:
              dtype = "{rsecret {x";
              break;
            case DEED_PUBLIC:
              dtype = "{gpublic {x";
              break;
            default:
              dtype = "{yprivate{x";
          }    

          sprintf_to_char(ch, "Deed [%5d] [%s - %s]\r\n",  deed->id,  dtype,  deed->title);
          deed = deed->next; 
        }
      }

      return TRUE;
    }

   /* ed show id */

    if ( !str_cmp( command, "show" ) ) {
    
	if ( keyword[0] == '\0' )	{
	    send_to_char( "Syntax:  ed show [id]\r\n", ch );
	    return FALSE;
	}

        id = atoi(keyword);

	for ( ed = pRoom->extra_descr; ed; ed = ed->next ) {
	    if ( ed->id == id) {
		break;
            } 
	}

	if ( !ed )
	{
	    send_to_char( "No extra description exists with that id.\r\n", ch );
	    return FALSE;
	}

	sprintf_to_char(ch, "{WExtra description %d\r\nKeywords:{x [%s]\r\n%s", id, ed->keyword, ed->description);

        if (ed->ec != NULL) { 
           send_to_char("{WConditions:{x\r\n", ch);
           print_econd(ch, ed->ec, "{WCondition -{x "); 
        }

        deed = ed->deed;

        if (deed != NULL) {
          send_to_char("{WDeeds:{x\r\n", ch);
        }
       
        while (deed != NULL) {

          switch (deed->type) {
            case DEED_SECRET:
              dtype = "{rsecret {x";
              break;
            case DEED_PUBLIC:
              dtype = "{gpublic {x";
              break;
            default:
              dtype = "{yprivate{x";
          }    

          sprintf_to_char(ch, "[%5d] [%s - %s]\r\n", deed->id,  dtype, deed->title);
          deed = deed->next; 
        }

	return TRUE;
    }

   /* ed cond id condition */

    if ( !str_cmp( command, "cond" ) ) {

      ECOND_DATA *new_ec;

     /* keyword must be supplied... */

      if ( keyword[0] == '\0' || keyword[0] == '?' ) {
        send_to_char( "Syntax:  ed cond [id] [condition]\r\n", ch );
	send_to_char( "         ed cond [id] ?\r\n", ch );
	return FALSE;
      }

     /* Check that the extra description exists first... */

      id = atoi(keyword);

      for ( ed = pRoom->extra_descr; ed; ed = ed->next ) {
        if ( ed->id == id) {
	  break;
        } 
      }

      if ( !ed ) {
	send_to_char( "No extra description exists with that id.\r\n", ch );
	return FALSE;
      }

     /* None means we should remove all of the conditions... */
 
      if (!str_cmp(remkeys, "none")) {
        while (ed->ec != NULL) {
          new_ec = ed->ec->next;
          free_ec(ed->ec);
          ed->ec = new_ec;
        }
      
        send_to_char("Conditions removed.\r\n", ch);
    
        return TRUE;
      }
 
     /* ..otherwise we go make a new one... */

      new_ec = create_econd(remkeys, ch);
 
      if (new_ec != NULL) {
        new_ec->next = ed->ec;
        ed->ec = new_ec;
      }
   
      return TRUE;
    }

   /* ed deed id deed type title */

    if ( !str_cmp( command, "deed" )) {
    
        int deed_id;
        int deed_type;
        DEED *new_deed;

	if ( keyword[0] == '\0' || remkeys[0] == '\0' ) {
	    send_to_char("Syntax:  ed deed [id] [deed] [type] [title]\r\n", ch);
	    return FALSE;
	}

       /* Find the extra description... */ 

        id = atoi(keyword);

	for ( ed = pRoom->extra_descr; ed; ed = ed->next ) {
	    if ( ed->id == id) break;
	}

	if ( !ed ) {
	    send_to_char( "No extra description exists with that id.\r\n", ch );
	    return FALSE;
	}

       /* Extract the deed id... */

        remkeys = one_argument(remkeys, keyword);

        deed_id = atoi(keyword);

       /* Extract the deed type... */

        remkeys = one_argument(remkeys, keyword);

        if (deed_id < 1) {
             long value;

             value = flag_value(discipline_type, remkeys);
             if ( value == NO_FLAG ) {
	    send_to_char( "Invalid discipline deed.\r\n", ch );
	    return FALSE;
             }
        }

        deed_type = 0;

        if (!str_cmp(keyword, "secret")) {
          deed_type = DEED_SECRET;
        } else if (!str_cmp(keyword, "public")) {
          deed_type = DEED_PUBLIC;
        } else if (!str_cmp(keyword, "private")) {
          deed_type = 0;
        } else {
          send_to_char("Deed type is secret, private or public.\r\n", ch);
          return FALSE;
        } 

       /* Check we have a title... */ 
        
        if ( remkeys[0] == '\0' ) {
	    send_to_char("Syntax:  ed deed [id] [deed] [type] [title]\r\n", ch);
	    return FALSE;
        }

       /* Deed adding time... */

        new_deed = NULL;
     
        deed = ed->deed;

        while (deed != NULL) {
          if (deed->id == deed_id) {
            new_deed = deed;
            break;
          }
          deed = deed->next;
        }

        if (new_deed == NULL) {  
          new_deed = get_deed();
          new_deed->id = deed_id;

          new_deed->next = ed->deed;
          ed->deed = new_deed;
        }

        new_deed->type = deed_type;
        new_deed->title = str_dup(remkeys);  
       
        send_to_char( "Extra description deed added/updated.\r\n", ch );
        return TRUE;
    }
 
   /* Nothing found, means give help... */

    redit_ed( ch, "" );
    return FALSE;
}



REDIT( redit_create )
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    int value;
    int iHash;
    
    EDIT_ROOM(ch, pRoom);

    value = atoi( argument );

    if ( argument[0] == '\0' || value <= 0 )
    {
	send_to_char( "Syntax:  create [vnum > 0]\r\n", ch );
	return FALSE;
    }

    pArea = get_vnum_area( value );
    if ( !pArea )
    {
	send_to_char( "REdit:  That vnum is not assigned an area.\r\n", ch );
	return FALSE;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char( "REdit:  Vnum in an area you cannot build in.\r\n", ch );
	return FALSE;
    }

    if ( get_room_index( value ) )
    {
	send_to_char( "REdit:  Room vnum already exists.\r\n", ch );
	return FALSE;
    }

    if ( IS_SET(pArea->area_flags, AREA_EDLOCK) ) {
        send_to_char("REdit:  Rooms area is Edit Locked!", ch);
        return FALSE;
    }

    pRoom			= new_room_index();
    pRoom->area			= pArea;
    pRoom->vnum			= value;

    if ( value > top_vnum_room )
        top_vnum_room = value;

    iHash			= value % MAX_KEY_HASH;
    pRoom->next			= room_index_hash[iHash];
    room_index_hash[iHash]	= pRoom;
    ch->desc->pEdit		= (void *)pRoom;

    send_to_char( "Room created.\r\n", ch );
    return TRUE;
}

void rebase_room( ROOM_INDEX_DATA *pRoom, int nvnum, AREA_DATA *new_area) {

    ROOM_INDEX_DATA *room, *prev_room;
    COND_DEST_DATA *cdd;
    RESET_DATA *reset;
    OBJ_INDEX_DATA *obj;

    AREA_DATA *old_area, *area;

    int i, iHash, ovnum;

    old_area = pRoom->area;  
    ovnum = pRoom->vnum;
 
    pRoom->vnum = nvnum;
    pRoom->area = new_area;

   /* Remove from the old index... */

    iHash = ovnum % MAX_KEY_HASH;

    if ( room_index_hash[iHash] == pRoom ) {
      room_index_hash[iHash] = pRoom->next;
    } else {
      prev_room = room_index_hash[iHash];

      while ( prev_room != NULL
           && prev_room->next != NULL ) {

        if ( prev_room->next == pRoom ) {
          prev_room->next = pRoom->next;
        }

        prev_room = prev_room->next;
      } 
    }

   /* Add to the new one... */ 

    iHash = nvnum % MAX_KEY_HASH;

    pRoom->next = room_index_hash[iHash];
    room_index_hash[iHash] = pRoom;

   /* Fix the rooms resets... */
 
    reset = pRoom->reset_first;

    while ( reset != NULL ) {

      switch ( reset->command ) {

        case 'M':
        case 'O':
          reset->arg3 = nvnum;
          break;

        case 'D':
        case 'R':
          reset->arg1 = nvnum;
          break;

        default:
          break;
      }

      reset = reset->next;
    }

   /* Now we have to go and fix ALL references to the room... */

    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++ ) {

     /* For rooms we must check the exits... */

      room = room_index_hash[iHash];

      while ( room != NULL ) { 

       /* Fix exits... */ 

        for ( i = 0; i < DIR_MAX; i++ ) {

          if ( room->exit[i] != NULL 
            && room->exit[i]->cond_dest != NULL ) {
            
             cdd = room->exit[i]->cond_dest;

             while ( cdd != NULL ) {

               if ( cdd->dest == pRoom ) {
                 cdd->vnum = nvnum;
               }
 
               cdd = cdd->next;
             } 
          }
        }

       /* Fix special rooms... */

        if (room->recall == ovnum) {
          room->recall = nvnum;
        }

        if (room->respawn == ovnum) {
          room->respawn = nvnum;
        }

        if (room->morgue == ovnum) {
          room->morgue = nvnum;
        }

        if (room->dream == ovnum) {
          room->dream = nvnum;
        }

        if (room->night == ovnum) {
          room->night = nvnum;
        }

        if (room->day == ovnum) {
          room->day = nvnum;
        }

        room = room->next; 
      }

     /* Check objects... */

      obj = obj_index_hash[iHash];

      while ( obj != NULL ) { 

        switch (obj->item_type) {

         /* For portals we must check the destination room... */

          case ITEM_PORTAL:

            if ( obj->value[0] == ovnum ) {
              obj->value[0] = nvnum;
            }

            break;
 
         /* For idols we must check the destination room... */

          case ITEM_IDOL:

            if ( obj->value[0] == ovnum ) {
              obj->value[0] = nvnum;
            }

            if ( obj->value[1] == ovnum ) {
              obj->value[1] = nvnum;
            }

            if ( obj->value[2] == ovnum ) {
              obj->value[2] = nvnum;
            }

            if ( obj->value[3] == ovnum ) {
              obj->value[3] = nvnum;
            }

            if ( obj->value[4] == ovnum ) {
              obj->value[4] = nvnum;
            }

            break;
 
          default:
            break;
        }

        obj = obj->next; 
      }
    }

   /* Fix areas... */

    area = area_first;

    while ( area != NULL ) {

      if (area->recall == ovnum) {
        area->recall = nvnum;
      }

      if (area->respawn == ovnum) {
        area->respawn = nvnum;
      }

      if (area->morgue == ovnum) {
        area->morgue = nvnum;
      }

      if (area->dream == ovnum) {
        area->dream = nvnum;
      }

      area = area->next;
    }

   /* Fix zones... */
 
    for (i = 0; i < NUM_ZONES; i++ ) {

      if (zones[i].recall == ovnum) {
        zones[i].recall = nvnum;
        bug("Zone %d recall room renumbered!", i);
      }

      if (zones[i].respawn == ovnum) {
        zones[i].respawn = nvnum;
        bug("Zone %d respawn room renumbered!", i);
      }

      if (zones[i].morgue == ovnum) {
        zones[i].morgue = nvnum;
        bug("Zone %d morgue room renumbered!", i);
      }

      if (zones[i].dream == ovnum) {
        zones[i].dream = nvnum;
        bug("Zone %d dream room renumbered!", i);
      }

    }

   /* Fix the MUD... */

    if (mud.recall == ovnum) {
      mud.recall = nvnum;
      bug("MUD recall room renumbered!", i);
    }

    if (mud.respawn == ovnum) {
      mud.respawn = nvnum;
      bug("MUD respawn room renumbered!", i);
    }

    if (mud.morgue == ovnum) {
      mud.morgue = nvnum;
      bug("MUD morgue room renumbered!", i);
    }

    if (mud.dream == ovnum) {
      mud.dream = nvnum;
      bug("MUD dream room renumbered!", i);
    }

    if (mud.start == ovnum) {
      mud.start = nvnum;
      bug("MUD start room renumbered!", i);
    }

    return;
}

REDIT( redit_revnum ) {
ROOM_INDEX_DATA *pRoom;
ROOM_INDEX_DATA *dest;
AREA_DATA *new_area, *old_area;
EXIT_DATA *pExit;
int i;
VNUM nvnum;

    EDIT_ROOM(ch, pRoom);

   /* Are they allowed to do this? */

    if ( !IS_IMP(ch) ) {
      send_to_char( "Only an implementor may do this...\r\n", ch );
      return FALSE;
    }

   /* Must have a parameter... */

    if ( argument[0] == '\0' ) {
      send_to_char( "Syntax:  revnum [new_vnum]\r\n", ch );
      return FALSE;
    }

   /* Get and check the new vnum... */

    nvnum = atoi(argument);
 
    if ( nvnum < 1 ) {
      send_to_char( "Invalid new vnum!\r\n", ch );
      return FALSE;
    }

    if ( get_room_index(nvnum) != NULL ) {
      send_to_char( "There is already a room at that vnum!\r\n", ch );
      return FALSE;
    }

   /* Now check the areas... */

    old_area = pRoom->area;

    new_area = get_vnum_area( nvnum );

    if ( new_area == NULL ) {
      send_to_char( "New vnum must be within an existing area!\r\n", ch );
      return FALSE;
    }

    if ( IS_SET(old_area->area_flags, AREA_EDLOCK) ) {
      send_to_char( "This area is edit locked!\r\n", ch );
      return FALSE;
    } 

    if ( IS_SET(new_area->area_flags, AREA_EDLOCK) ) {
      send_to_char( "The new area is edit locked!\r\n", ch );
      return FALSE;
    } 

   /* Check exits... */ 

    for ( i = 0;  i < DIR_MAX;  i++ ) {
    
      pExit = pRoom->exit[i];

      if ( pExit != NULL 
        && pExit->u1.to_room != NULL ) {

        dest = pExit->u1.to_room;
 
        if ( dest->area != NULL 
          && dest->area != old_area 
          && dest->area != new_area 
          && IS_SET(dest->area->area_flags, AREA_EDLOCK) ) {
          send_to_char( "Room connects to an edit locked area!\r\n", ch );
          return FALSE;
        }
      }
    }

   /* Ok, probably safe to go ahead with the renum... */

    rebase_room(pRoom, nvnum, new_area);

   /* Ok, maybe done... */

    send_to_char("{YRoom vnum changed...{x\r\n", ch);
    send_to_char("{YASAVE WORLD and REBOOT to complete...{x\r\n", ch);

    return TRUE;
}

REDIT( redit_name )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  name [name]\r\n", ch );
	return FALSE;
    }

    free_string( pRoom->name );
    pRoom->name = str_dup( argument );

    send_to_char( "Name set.\r\n", ch );
    return TRUE;
}

REDIT( redit_image )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  image [image URL]\r\n", ch );
	return FALSE;
    }

    free_string( pRoom->image );
    pRoom->image = str_dup( argument );

    send_to_char( "Image URL set.\r\n", ch );
    return TRUE;
}


REDIT( redit_subarea )
{
    ROOM_INDEX_DATA *pRoom;

    int sanum;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' ) {
	send_to_char( "Syntax:  subarea [number]\r\n", ch );
	return FALSE;
    }

    sanum = atoi(argument);

    if (sanum <0) { 
      send_to_char( "Subarea number must be >= 0\r\n", ch);
      return FALSE;
    }

    pRoom->subarea = sanum;

    send_to_char( "Subarea set.\r\n", ch );
    return TRUE;
}

REDIT( redit_heal )
{
    ROOM_INDEX_DATA *pRoom;

    int sanum;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' ) {
	send_to_char( "Syntax:  heal [number]\r\n", ch );
	return FALSE;
    }

    sanum = atoi(argument);

    if (sanum < 0 | sanum > 999) { 
      send_to_char( "Heal rate must be between 0 and 999\r\n", ch);
      return FALSE;
    }

    pRoom->heal_rate = sanum;

    send_to_char( "Healing rate set.\r\n", ch );
    return TRUE;
}

REDIT( redit_mana )
{
    ROOM_INDEX_DATA *pRoom;

    int sanum;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' ) {
	send_to_char( "Syntax:  mana [number]\r\n", ch );
	return FALSE;
    }

    sanum = atoi(argument);

    if (sanum < 0 | sanum > 999) { 
      send_to_char( "Mana rate must be between 0 and 999\r\n", ch);
      return FALSE;
    }

    pRoom->mana_rate = sanum;

    send_to_char( "Mana rate set.\r\n", ch );
    return TRUE;
}

REDIT( redit_rent )
{
    ROOM_INDEX_DATA *pRoom;

    int sanum;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' ) {
	send_to_char( "Syntax:  rent [number]\r\n", ch );
	return FALSE;
    }

    sanum = atoi(argument);

    if (sanum < 0) { 
      send_to_char( "Rent number must be >= 0\r\n", ch);
      return FALSE;
    }

    if (sanum > 0
    && !IS_SET(pRoom->room_flags, ROOM_RENT)) { 
      send_to_char( "This room can't be rented.\r\n", ch);
      return FALSE;
    }

    pRoom->room_rent = sanum;

    send_to_char( "Rent set.\r\n", ch );
    return TRUE;
}

REDIT( redit_recall ) {

    ROOM_INDEX_DATA *pRoom;

    int value;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' ) {
	send_to_char( "Syntax:  recall [rvnum]\r\n", ch );
	return FALSE;
    }

    value = atoi(argument);

    if ( value != 0
      && !room_ok_for_recall(value)) { 
      send_to_char( "That room is unaceptable.\r\n", ch);
      return FALSE;
    }

    pRoom->recall = value;
 
    if ( value == 0 ) {
      send_to_char("Room Recall location cleared.\r\n", ch);
    } else {
      send_to_char( "Room Recall location set.\r\n", ch );
    }

    return TRUE;
}

REDIT( redit_respawn ) {

    ROOM_INDEX_DATA *pRoom;

    int value;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' ) {
	send_to_char( "Syntax:  respawn [rvnum]\r\n", ch );
	return FALSE;
    }

    value = atoi(argument);

    if ( value != 0
      && !room_ok_for_recall(value)) { 
      send_to_char( "That room is unaceptable.\r\n", ch);
      return FALSE;
    }

    pRoom->respawn = value;
 
    if ( value == 0 ) {
      send_to_char("Room Respawn location cleared.\r\n", ch);
    } else {
      send_to_char( "Room Respawn location set.\r\n", ch );
    }

    return TRUE;
}

REDIT( redit_morgue ) {

    ROOM_INDEX_DATA *pRoom;

    int value;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' ) {
	send_to_char( "Syntax:  morgue [rvnum]\r\n", ch );
	return FALSE;
    }

    value = atoi(argument);

    if ( value != 0
      && !room_ok_for_recall(value)) { 
      send_to_char( "That room is unaceptable.\r\n", ch);
      return FALSE;
    }

    pRoom->morgue = value;
 
    if ( value == 0 ) {
      send_to_char("Room Morgue location cleared.\r\n", ch);
    } else {
      send_to_char( "Room Morgue location set.\r\n", ch );
    }

    return TRUE;
}

REDIT( redit_dream ) {

    ROOM_INDEX_DATA *pRoom;

    int value;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' ) {
	send_to_char( "Syntax:  dream [rvnum]\r\n", ch );
	return FALSE;
    }

    value = atoi(argument);

    if ( value != 0
      && !room_ok_for_recall(value)) { 
      send_to_char( "That room is unaceptable.\r\n", ch);
      return FALSE;
    }

    pRoom->dream = value;
 
    if ( value == 0 ) {
      send_to_char("Room Dream location cleared.\r\n", ch);
    } else {
      send_to_char( "Room Dream location set.\r\n", ch );
    }

    return TRUE;
}

REDIT( redit_mare ) {
    ROOM_INDEX_DATA *pRoom;
    int value;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' ) {
	send_to_char( "Syntax:  mare [rvnum]\r\n", ch );
	return FALSE;
    }

    value = atoi(argument);

    if ( value > 0
    && !room_ok_for_recall(value)) { 
      send_to_char( "That room is unaceptable.\r\n", ch);
      return FALSE;
    }

    pRoom->mare = value;
 
    if ( value == 0 ) {
      send_to_char("Room Nightmare location cleared.\r\n", ch);
    } else if ( value == -1 ) {
      send_to_char("Room Nightmare safe set.\r\n", ch);
    } else {
      send_to_char( "Room Dream location set.\r\n", ch );
    }

    return TRUE;
}

REDIT( redit_night ) {

    ROOM_INDEX_DATA *pRoom;

    int value;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' ) {
	send_to_char( "Syntax:  night [rvnum]\r\n", ch );
	return FALSE;
    }

    value = atoi(argument);

    if ( value != 0
      && !room_ok_for_recall(value)) { 
      send_to_char( "That room is unaceptable.\r\n", ch);
      return FALSE;
    }

    pRoom->night = value;
 
    if ( value == 0 ) {
      send_to_char("Room Night location cleared.\r\n", ch);
    } else {
      send_to_char( "Room Night location set.\r\n", ch );
    }

    return TRUE;
}

REDIT( redit_day ) {

    ROOM_INDEX_DATA *pRoom;

    int value;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' ) {
	send_to_char( "Syntax:  day [rvnum]\r\n", ch );
	return FALSE;
    }

    value = atoi(argument);

    if ( value != 0
      && !room_ok_for_recall(value)) { 
      send_to_char( "That room is unaceptable.\r\n", ch);
      return FALSE;
    }

    pRoom->day = value;
 
    if ( value == 0 ) {
      send_to_char("Room Day location cleared.\r\n", ch);
    } else {
      send_to_char( "Room Day location set.\r\n", ch );
    }

    return TRUE;
}

REDIT( redit_affect )  {
    ROOM_INDEX_DATA *pRoom;
    long value;

    if ( argument[0] != '\0' )    {
                EDIT_ROOM(ch, pRoom);

	if ( ( value = flag_value_long( raffect_flags, argument ) ) != NO_FLAG )
	{
	    pRoom->affected_by ^= value;
	    send_to_char( "Affect flag toggled.\r\n", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax: affect [flag]\r\n"
		"Type '? raffect' for a list of flags.\r\n", ch );
    return FALSE;
}

REDIT( redit_desc )
{
    ROOM_INDEX_DATA *pRoom;
	ROOM_INDEX_DATA *copyRoom;
	char arg[MAX_INPUT_LENGTH];
    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' )
    {
	string_append( ch, &pRoom->description );
	return TRUE;
    }
		
	argument = one_argument ( argument, arg);
	if (!str_cmp(arg, "copy") && (argument[0] != '\0'))
		{
		one_argument (argument, arg);
		if ( is_number(arg) )
			{
   			copyRoom = get_room_index( atoi( arg ) );
			if (copyRoom != NULL)
				{
				if (pRoom->description != NULL)
					free_string (pRoom->description);
				pRoom->description = str_dup (copyRoom->description);
				return TRUE;
				}
			}
		}

    send_to_char( "Syntax:  desc\r\n", ch );
	send_to_char( "Syntax:  desc copy <vnum>\r\n",ch);
    return FALSE;
}


REDIT( redit_flags ) {
ROOM_INDEX_DATA *pRoom;
char flags[40];
int value;

    EDIT_ROOM(ch, pRoom);

    one_argument( argument, flags );
    if ( argument[0] == '\0' ) {
        send_to_char( "Syntax:   flags <room flag>\r\n", ch );
        return FALSE;
    }

    if ((value = flag_value_long(room_flags, flags ) ) != NO_FLAG ) {
        if (IS_SET(value, ROOM_MUDGATE)
        && !IS_SET(get_authority(ch), IMMAUTH_ADMIN)) {
               send_to_char( "Authority Admin is nescessary to apply this flag.\r\n", ch );
               return FALSE;
        }

        TOGGLE_BIT(pRoom->room_flags, value);

        send_to_char( "Room flag toggled.\r\n", ch );
        return TRUE;
    }
     send_to_char( "Unrecognized flag.\r\n", ch );
    return FALSE;
}

REDIT( redit_sector )
{
    ROOM_INDEX_DATA *pRoom;
    char flags[40];
    int value;

    EDIT_ROOM(ch, pRoom);

    one_argument( argument, flags );
    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:   flags <sector flag>\r\n", ch );
        return FALSE;
    }

    if ( ( value = flag_value( sector_flags, flags ) ) != NO_FLAG )
    {
        pRoom->sector_type  = value;

       /* Nasty hard coded hack for now... */

        switch (value) {
          case SECT_INSIDE:
            REMOVE_BIT(pRoom->room_flags, ROOM_DARK);
            SET_BIT(pRoom->room_flags, ROOM_INDOORS);
            break;
          case SECT_UNDERWATER:
          case SECT_EVIL:
            SET_BIT(pRoom->room_flags, ROOM_DARK);
            break;
          case SECT_UNDERGROUND:
            SET_BIT(pRoom->room_flags, ROOM_DARK);
            SET_BIT(pRoom->room_flags, ROOM_INDOORS);
            break;
          default:
            REMOVE_BIT(pRoom->room_flags, ROOM_DARK);
            REMOVE_BIT(pRoom->room_flags, ROOM_INDOORS);
        }

        send_to_char( "Sector type set, room flags updated.\r\n", ch );

        return TRUE;
    }

    send_to_char( "Unrecognized sector type.\r\n", ch );

    return FALSE;
}


REDIT( redit_format )
{
    ROOM_INDEX_DATA *pRoom;

    char buf[MAX_STRING_LENGTH];

    EDIT_ROOM(ch, pRoom);

    if (format_text(pRoom->description, buf)) {
      free_string(pRoom->description);
      pRoom->description = str_dup(buf);
    }
//    pRoom->description = format_string( pRoom->description );

    send_to_char( "String formatted.\r\n", ch );
    return TRUE;
}



REDIT( redit_mreset )
{
    ROOM_INDEX_DATA	*pRoom;
    MOB_INDEX_DATA	*pMobIndex;
    CHAR_DATA		*newmob;
    char		arg [ MAX_INPUT_LENGTH ];

    RESET_DATA		*pReset;
    char		output [ MAX_STRING_LENGTH ];
    char		vb[ MAX_STRING_LENGTH ];

    EDIT_ROOM(ch, pRoom);

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number( arg ) )
    {
	send_to_char ( "Syntax:  mreset <vnum> <max #>\r\n", ch );
	return FALSE;
    }

    if ( !( pMobIndex = get_mob_index( atoi( arg ) ) ) )
    {
	send_to_char( "REdit: No mobile has that vnum.\r\n", ch );
	return FALSE;
    }

    if ( pMobIndex->area != pRoom->area )
    {
//	send_to_char( "REdit: No such mobile in this area.\r\n", ch );
//	return FALSE;
        send_to_char("{YREdit: Mobile is from outside this area!\r\n{x", ch);
    }

    /*
     * Create the mobile reset.
     */
    pReset              = new_reset_data();
    pReset->command	= 'M';
    pReset->arg1	= pMobIndex->vnum;
    pReset->arg2	= is_number( argument ) ? atoi( argument ) : MAX_MOB;
    pReset->arg3	= pRoom->vnum;
    add_reset( pRoom, pReset, 0/* Last slot*/ );

    /*
     * Create the mobile.
     */
    newmob = create_mobile( pMobIndex );
	newmob->reset = pReset;	
	newmob->reset->count++;
    char_to_room( newmob, pRoom );

    format_vnum(pMobIndex->vnum,vb);
    sprintf( output, "%s (%s) has been loaded and added to resets.\r\n"
	"There will be a maximum of %d loaded to this room.\r\n",
	capitalize( pMobIndex->short_descr ),
	vb,
	pReset->arg2 );
    send_to_char( output, ch );
    act( "$n has created $N!", ch, NULL, newmob, TO_ROOM );
    return TRUE;
}



struct wear_type
{
    int	wear_loc;
    int	wear_bit;
};



const struct wear_type wear_table[] =
{
    {	WEAR_NONE,	ITEM_TAKE		},
    {	WEAR_LIGHT,	ITEM_LIGHT		},
    {	WEAR_FINGER_L,	ITEM_WEAR_FINGER	},
    {	WEAR_FINGER_R,	ITEM_WEAR_FINGER	},
    {	WEAR_NECK_1,	ITEM_WEAR_NECK		},
    {	WEAR_NECK_2,	ITEM_WEAR_NECK		},
    {	WEAR_BODY,	ITEM_WEAR_BODY		},
    {	WEAR_HEAD,	ITEM_WEAR_HEAD		},
    {	WEAR_LEGS,	ITEM_WEAR_LEGS		},
    {	WEAR_FEET,	ITEM_WEAR_FEET		},
    {	WEAR_HANDS,	ITEM_WEAR_HANDS		},
    {	WEAR_ARMS,	ITEM_WEAR_ARMS		},
    {	WEAR_SHIELD,	ITEM_WEAR_SHIELD	},
    {	WEAR_ABOUT,	ITEM_WEAR_ABOUT		},
    {	WEAR_WAIST,	ITEM_WEAR_WAIST		},
    {	WEAR_WRIST_L,	ITEM_WEAR_WRIST		},
    {	WEAR_WRIST_R,	ITEM_WEAR_WRIST		},
    {	WEAR_WIELD,	ITEM_WIELD		},
    {	WEAR_HOLD,	ITEM_HOLD		},
    {	WEAR_PRIDE,	ITEM_WEAR_PRIDE		},
    {	WEAR_FACE,	ITEM_WEAR_FACE		},
    {	WEAR_EARS,	ITEM_WEAR_EARS		},
    {	WEAR_FLOAT,	ITEM_WEAR_FLOAT		},
    {	WEAR_EYES,	ITEM_WEAR_EYES		},
    {	WEAR_BACK,	ITEM_WEAR_BACK		},
    {	WEAR_TATTOO,	ITEM_WEAR_TATTOO		},
    {	NO_FLAG,	NO_FLAG			}
};



/*****************************************************************************
 Name:		wear_loc
 Purpose:	Returns the location of the bit that matches the count.
 		1 = first match, 2 = second match etc.
 Called by:	oedit_reset(olc_act.c).
 ****************************************************************************/
int wear_loc(int bits, int count) {
    int flag;
 
    for (flag = 0; wear_table[flag].wear_bit != NO_FLAG; flag++)    {
        if ( IS_SET(bits, wear_table[flag].wear_bit) && --count < 1)
            return wear_table[flag].wear_loc;
    }
 
    return NO_FLAG;
}



/*****************************************************************************
 Name:		wear_bit
 Purpose:	Converts a wear_loc into a bit.
 Called by:	redit_oreset(olc_act.c).
 ****************************************************************************/
int wear_bit(int loc)
{
    int flag;
 
    for (flag = 0; wear_table[flag].wear_loc != NO_FLAG; flag++)
    {
        if ( loc == wear_table[flag].wear_loc )
            return wear_table[flag].wear_bit;
    }
 
    return 0;
}



REDIT( redit_oreset )
{
    ROOM_INDEX_DATA	*pRoom;
    OBJ_INDEX_DATA	*pObjIndex;
    OBJ_DATA		*newobj;
    OBJ_DATA		*to_obj;
    CHAR_DATA		*to_mob;
    char		arg1 [ MAX_INPUT_LENGTH ];
    char		arg2 [ MAX_INPUT_LENGTH ];
    char		max [MAX_INPUT_LENGTH ];
    int			olevel = 0;

    RESET_DATA		*pReset;
    char		output [ MAX_STRING_LENGTH ];
    char		vb[ MAX_STRING_LENGTH ];
    char		vb2[ MAX_STRING_LENGTH ];

    EDIT_ROOM(ch, pRoom);

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, max  );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number( arg1 )
		 || max[0]=='\0' || !is_number( max  ) )
    {
	send_to_char ( "Syntax:  oreset <vnum> <max> <args>\r\n", ch );
	send_to_char ( "        -no_args               = into room\r\n", ch );
	send_to_char ( "        -<obj_name>            = into obj\r\n", ch );
	send_to_char ( "        -<mob_name> <wear_loc> = into mob\r\n", ch );
	return FALSE;
    }

    if ( !( pObjIndex = get_obj_index( atoi( arg1 ) ) ) )
    {
	send_to_char( "REdit: No object has that vnum.\r\n", ch );
	return FALSE;
    }

    if ( pObjIndex->area != pRoom->area )
    {
//        send_to_char( "REdit: No such object in this area.\r\n", ch );
//        return FALSE;
        send_to_char("{YREdit: Object is from outside this area!\r\n{x", ch);
   }

    /*
     * Load into room.
     */
    if ( arg2[0] == '\0' )
    {
	pReset		= new_reset_data();
	pReset->command	= 'O';
	pReset->arg1	= pObjIndex->vnum;
	pReset->arg2	= atoi(max);
	pReset->arg3	= pRoom->vnum;
	pReset->count	= 1;
	add_reset( pRoom, pReset, 0/* Last slot*/ );

	newobj = create_object( pObjIndex, number_fuzzy( olevel ) );
	newobj->reset = pReset;
	obj_to_room( newobj, pRoom );

        format_vnum(pObjIndex->vnum,vb);
	sprintf( output, "%s (%s) has been loaded and added to resets.\r\n",
	    capitalize( pObjIndex->short_descr ),
	    vb );
	send_to_char( output, ch );
    }
    else
    /*
     * Load into object's inventory.
     */
    if ( argument[0] == '\0'
    && ( ( to_obj = get_obj_list( ch, arg2, pRoom->contents ) ) != NULL ) )
    {
	pReset		= new_reset_data();
	pReset->command	= 'P';
	pReset->arg1	= pObjIndex->vnum;
	pReset->arg2	= atoi(max);
	pReset->arg3	= to_obj->pIndexData->vnum;
	pReset->count	= 1;
	add_reset( pRoom, pReset, 0/* Last slot*/ );

	newobj = create_object( pObjIndex, number_fuzzy( olevel ) );
	newobj->reset = pReset;
	newobj->cost = 0;
	obj_to_obj( newobj, to_obj );

        format_vnum(newobj->pIndexData->vnum,vb);
        format_vnum(to_obj->pIndexData->vnum,vb2);
	sprintf( output, "%s (%s) has been loaded into "
	                 "%s (%s) and added to resets.\r\n",
                          capitalize( newobj->short_descr ),
	                  vb,
	                  to_obj->short_descr,
	                  vb2 );
	send_to_char( output, ch );
    }
    else
    /*
     * Load into mobile's inventory.
     */
    if ( ( to_mob = get_char_room( ch, arg2 ) ) != NULL )
    {
	int	wear_loc;

	/*
	 * Make sure the location on mobile is valid.
	 */
	if ( (wear_loc = flag_value( wear_loc_flags, argument )) == NO_FLAG )
	{
	    send_to_char( "REdit: Invalid wear_loc.  '? wear-loc'\r\n", ch );
	    return FALSE;
	}

	/*
	 * Disallow loading a sword(WEAR_WIELD) into WEAR_HEAD.
	 */
	if ( !IS_SET( pObjIndex->wear_flags, wear_bit(wear_loc) ) )
	{
            format_vnum(pObjIndex->vnum,vb);
	    sprintf( output, "%s (%s) has wear flags: [%s]\r\n",
	                      capitalize( pObjIndex->short_descr ),
	                      vb,
	                      flag_string( wear_flags, pObjIndex->wear_flags ) );
	    send_to_char( output, ch );
	    return FALSE;
	}

	/*
	 * Can't load into same position.
	 */
/* - we're checking this too soon 
	if ( get_eq_char( to_mob, wear_loc ) )
	{
	    send_to_char( "REdit:  Object already equipped.\r\n", ch );
	    return FALSE;
	}
*/

	pReset		= new_reset_data();
	pReset->arg1	= pObjIndex->vnum;
	pReset->arg3	= wear_loc;
	pReset->count	= 1;
	if ( pReset->arg3 == WEAR_NONE )
	    pReset->command = 'G';
	else
	{
	    pReset->command = 'E';

	/* elfren added below from above :) */

		if ( get_eq_char (to_mob, wear_loc ) )
		{
			send_to_char( "REdit: Already equipped that location.\r\n", ch);
			return FALSE;
		}
	}
	pReset->arg2	= atoi(max);

	add_reset( pRoom, pReset, 0/* Last slot*/ );

	olevel  = URANGE( 0, to_mob->level - 2, MAX_LEVEL );
        newobj = create_object( pObjIndex, number_fuzzy( olevel ) );

	if ( to_mob->pIndexData->pShop )	{
	    switch ( pObjIndex->item_type )
	    {
	    default:		olevel = 0;				break;
	    case ITEM_PILL:	olevel = number_range(  0, 10 );	break;
	    case ITEM_POTION:	olevel = number_range(  0, 10 );	break;
	    case ITEM_SCROLL:	olevel = number_range(  5, 15 );	break;
	    case ITEM_WAND:	olevel = number_range( 10, 20 );	break;
	    case ITEM_STAFF:	olevel = number_range( 15, 25 );	break;
	    case ITEM_ARMOR:	olevel = number_range(  5, 15 );	break;
	    case ITEM_WEAPON:	if ( pReset->command == 'G' )
	    			    olevel = number_range( 5, 15 );
				else
				    olevel = number_fuzzy( olevel );
		break;
	    }

	    newobj = create_object( pObjIndex, olevel );
		newobj->reset = pReset;
		newobj->reset->count = 1;
	    if ( pReset->arg3 == WEAR_NONE )
		SET_BIT( newobj->extra_flags, ITEM_INVENTORY );
	}
	else
	    newobj = create_object( pObjIndex, number_fuzzy( olevel ) );
	newobj->reset = pReset;
	newobj->reset->count = 1;
	obj_to_char( newobj, to_mob );
	if ( pReset->command == 'E' )
	    equip_char( to_mob, newobj, pReset->arg3 );

        format_vnum(pObjIndex->vnum,vb);
        format_vnum(to_mob->pIndexData->vnum,vb2);
	sprintf( output, "%s (%s) has been loaded "
	                 "%s of %s (%s) and added to resets.\r\n",
	                  capitalize( pObjIndex->short_descr ),
	                  vb,
	                  flag_string( wear_loc_strings, pReset->arg3 ),
	                  to_mob->short_descr,
	                  vb2 );
	send_to_char( output, ch );
    }
    else	/* Display Syntax */
    {
	send_to_char( "REdit:  That mobile isn't here.\r\n", ch );
	return FALSE;
    }

    act( "$n has created $p!", ch, newobj, NULL, TO_ROOM );
    return TRUE;
}



/*
 * Object Editor Functions.
 */
void show_obj_values( CHAR_DATA *ch, OBJ_INDEX_DATA *obj ) {
char buf[MAX_STRING_LENGTH];
char vb[MAX_STRING_LENGTH];
MOB_INDEX_DATA *pMob;

    switch( obj->item_type )
    {
	default:	/* No values. */
	    break;

        case ITEM_PORTAL:
            if (obj->value[0] > 0) sprintf_to_char(ch, "[v0] Destination room:   [%ld]\r\n", obj->value[0]);
            else send_to_char("[v0] Destination room:   [random]\r\n", ch);
            sprintf_to_char(ch, "[v1] Link object vnum:   [%ld]\r\n", obj->value[1]);
            sprintf_to_char(ch, "[v2] Portal Key         :   [%ld]\r\n", obj->value[2]);
            sprintf_to_char(ch, "[v4] Portal Class      :    %s\r\n", flag_string( portal_class, obj->value[4] ));
            break;

	case ITEM_TREE:
            sprintf_to_char(ch, "[v0] Tree Size:   [%s]\r\n", flag_string(tree_size,obj->value[0]));
            sprintf_to_char(ch, "[v1] Tree Type:   [%s]\r\n", flag_string(tree_size,obj->value[1]));
            break;	

	case ITEM_SHARE:
            sprintf_to_char(ch, "[v0] Share Id:    %d\r\n", obj->value[0]);
            sprintf_to_char(ch, "[v1] Number:      %d\r\n", obj->value[1]);
            break;	

	case ITEM_FIGURINE:
            if ((pMob = get_mob_index(obj->value[0])) != NULL) sprintf_to_char(ch, "[v0] Mob Template:    %s (%d)\r\n", pMob->short_descr, obj->value[0]);
            else sprintf_to_char(ch, "[v0] Mob Template:    %d\r\n", obj->value[0]);
            sprintf_to_char(ch, "[v1] Mob Behavior:    %s\r\n", flag_string(figurine_behavior, obj->value[1]));
            sprintf_to_char(ch, "[v2] Figurine Type :   %s\r\n",  flag_string(figurine_type, obj->value[2]));
            break;	

	case ITEM_INSTRUMENT:
	      sprintf_to_char(ch,"[v0] Type  : [%s]\r\n", flag_string(instrument_type, obj->value[0]));
	      sprintf_to_char(ch,"[v1] Power : [%d]\r\n", obj->value[1]);
	      sprintf_to_char(ch,"[v4] Spell : [%s]\r\n", obj->value[4] > 0 ? effect_array[obj->value[4]].name : "none");
  	      break;

	case ITEM_LIGHT:
	      sprintf_to_char(ch,"[v0] Switch: [%s]\r\n", obj->value[0] == 0 ? "yes" : "no" );
	      sprintf_to_char(ch,"[v1] Refill: [%s]\r\n", flag_string(refill_type, obj->value[1]));
                      if ( obj->value[2] == -1 
                      || obj->value[2] == 999 ) {
		sprintf_to_char(ch, "[v2] Light:  Infinite[-1]\r\n" );
                      } else {
		sprintf_to_char(ch, "[v2] Light:  [%ld]\r\n", obj->value[2] );
                      }
	      sprintf_to_char(ch,"[v3] Status: [%s]\r\n", obj->value[3] == 0 ? "off" : "on" );
                      sprintf_to_char(ch,"[v4] Max:    [%ld]\r\n", obj->value[4] );
  	      break;

	case ITEM_LIGHT_REFILL:
	      sprintf_to_char(ch,"[v1] Refill: [%s]\r\n", flag_string(refill_type, obj->value[1]));
	      sprintf_to_char(ch, "[v2] Light:  [%ld]\r\n", obj->value[2] );
  	      break;

	case ITEM_PROTOCOL:
                      format_vnum(obj->value[0], vb);
	      sprintf_to_char(ch,"[v0] Product:[%s]\r\n", vb);
                      format_vnum(obj->value[1], vb);
	      sprintf_to_char(ch,"[v1] Part:   [%s]\r\n", vb);
                      format_vnum(obj->value[2], vb);
	      sprintf_to_char(ch,"[v2] Part:   [%s]\r\n", vb);
                      format_vnum(obj->value[3], vb);
	      sprintf_to_char(ch,"[v3] Part:   [%s]\r\n", vb);
                      format_vnum(obj->value[4], vb);
	      sprintf_to_char(ch,"[v4] Part:   [%s]\r\n", vb);
  	      break;

	case ITEM_WAND:
	case ITEM_STAFF:
            sprintf( buf,
		"[v0] Level:          [%ld]\r\n"
		"[v1] Charges Total:  [%ld]\r\n"
		"[v2] Charges Left:   [%ld]\r\n"
		"[v3] Spell:          %s\r\n",
		obj->value[0],
		obj->value[1],
		obj->value[2],
		obj->value[3] > 0 ? effect_array[obj->value[3]].name : "none" );
	    send_to_char( buf, ch );
	    break;

	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
            sprintf( buf,
		"[v0] Level:  [%ld]\r\n"
		"[v1] Spell:  %s\r\n"
		"[v2] Spell:  %s\r\n"
		"[v3] Spell:  %s\r\n"
		"[v4] Spell:  %s\r\n",
		obj->value[0],
		obj->value[1] > 0 ? effect_array[obj->value[1]].name
		                  : "none",
		obj->value[2] > 0 ? effect_array[obj->value[2]].name
                                  : "none",
		obj->value[3] > 0 ? effect_array[obj->value[3]].name
		                  : "none",
		obj->value[4] > 0 ? effect_array[obj->value[4]].name
		                  : "none" );
	    send_to_char( buf, ch );
	    break;

	case ITEM_HERB:
                      sprintf( buf,
		"[v0] Level:     [%ld]\r\n"
		"[v1] Spell:     %s\r\n"
		"[v2] Spell:     %s\r\n"
 	        "[v3] Poisoned:  %s\r\n"
                "[v4] Telepop:   %s\r\n",
		obj->value[0],
		obj->value[1] > 0 ? effect_array[obj->value[1]].name 
                                  : "none",
		obj->value[2] > 0 ? effect_array[obj->value[2]].name 
                                  : "none",
	        obj->value[3] != 0 ? "Yes" : "No",
	        obj->value[4] != 0 ? "Yes" : "No" );
	       send_to_char( buf, ch );
	       break;

	case ITEM_PIPE:
                      sprintf( buf,
		"[v0] LevelMod:     [%ld]\r\n"
		"[v1] Multiple Use: %s\r\n"
		"[v2] LOADLevel     [%ld]\r\n"
		"[v3] LOADSpell:   %s\r\n"
		"[v4] LOADSpell:   %s\r\n",
		obj->value[0],
	        obj->value[1] != 0 ? "Yes" : "No",
		obj->value[2],
		obj->value[3] > 0 ? effect_array[obj->value[3]].name 
                                  : "none",
		obj->value[4] > 0 ? effect_array[obj->value[4]].name 
                                  : "none");
	       send_to_char( buf, ch );
	       break;


/* ARMOR for ROM */

        case ITEM_ARMOR:
	    sprintf( buf,
		"[v0] Ac pierce       [%ld]\r\n"
		"[v1] Ac bash         [%ld]\r\n"
		"[v2] Ac slash        [%ld]\r\n"
		"[v3] Ac exotic       [%ld]\r\n",
		obj->value[0],
		obj->value[1],
		obj->value[2],
		obj->value[3] );
	    send_to_char( buf, ch );
	    break;

case ITEM_RAW:
         sprintf_to_char(ch, "[v0] Combat Mod:  [%ld]\r\n", obj->value[0]  );
         sprintf_to_char(ch, "[v1] Weight Mod:  [%ld]\r\n", obj->value[1]  );
         break;

case ITEM_BONDAGE:
         sprintf_to_char(ch, "[v0] Strength:  [%ld]\r\n", obj->value[0]  );
         break;

case ITEM_DOLL:
         sprintf_to_char(ch, "[v0] Power:  [%ld]\r\n", obj->value[0]  );
         break;

case ITEM_FOCUS:
         sprintf_to_char(ch, "[v0] Zones:  [%s]\r\n", flag_string(zmask_flags, obj->value[0]));
         break;

case ITEM_PHOTOGRAPH:
         sprintf_to_char(ch, "[v0] State:   %s\r\n", flag_string(photo_state, obj->value[0]));
         break;

case ITEM_AMMO:
         sprintf( buf, "[v0] Shots:  [%ld]\r\n", obj->value[0]  );
         send_to_char( buf, ch );
         sprintf( buf, "[v4] Special type:   %s\r\n",  flag_string( weapon_type,  obj->value[4] ) );
         send_to_char( buf, ch );
         break;

case ITEM_SLOTMACHINE:
          sprintf( buf, "[v0] Gold Cost:       [%ld]\r\n"
                        "[v1] Jackpot Value:   [%ld]\r\n"
                        "[v2] Owner         :  [%ld]\r\n",
                         obj->value[0],
                         obj->value[1],
                         obj->value[2] );
          send_to_char(buf, ch);
          break;

/* WEAPON changed in ROM: */
/* I had to split the output here, I have no idea why, but it helped -- Hugin */
/* It somehow fixed a bug in showing scroll/pill/potions too ?! */
/* It's because some AH coded flag_string to always return its data in */
/* the SAME bloody buffer. Mik. */

	case ITEM_WEAPON:
            sprintf( buf, "[v0] Weapon class:   %s\r\n", flag_string( weapon_class, obj->value[0] ) );
	    send_to_char( buf, ch );
	    sprintf( buf, "[v1] Number of dice: [%ld]\r\n", obj->value[1] );
	    send_to_char( buf, ch );
	    sprintf( buf, "[v2] Type of dice:   [%ld]\r\n", obj->value[2] );
	    send_to_char( buf, ch );
	    sprintf( buf, "[v3] Type:           %s\r\n",
		    flag_string( weapon_flags, obj->value[3] ) );
	    send_to_char( buf, ch );
 	    sprintf( buf, "[v4] Special type:   %s\r\n",
		     flag_string( weapon_type,  obj->value[4] ) );
	    send_to_char( buf, ch );
	    break;

	case ITEM_EXPLOSIVE:
                    sprintf( buf, "[v0] Explosive\r\n");
	    send_to_char( buf, ch );
	    sprintf( buf, "[v1] Number of dice: [%ld]\r\n", obj->value[1] );
	    send_to_char( buf, ch );
	    sprintf( buf, "[v2] Type of dice:   [%ld]\r\n", obj->value[2] );
	    send_to_char( buf, ch );
	    sprintf( buf, "[v3] TimerMin    :   [%ld]\r\n", obj->value[3] );
	    send_to_char( buf, ch );
	    sprintf( buf, "[v4] TimerMax    :   [%ld]\r\n", obj->value[4] );
	    send_to_char( buf, ch );
	    break;

	case ITEM_TRAP:
                    sprintf_to_char(ch, "[v0] Trap class:   %s\r\n", flag_string( trap_class, obj->value[0] ) );

                    if ( obj->value[0]==TRAP_WOUND
                    || obj->value[0]==TRAP_MYSTIC) {
   	        sprintf_to_char(ch, "[v1] Number of dice: [%ld]\r\n", obj->value[1] );
	        sprintf_to_char(  ch, "[v2] Type of dice:   [%ld]\r\n", obj->value[2] );
                    }
	    break;

	case ITEM_FURNITURE:
 	    sprintf( buf, "[v0] Furniture Class:   %s\r\n", flag_string(furniture_class,  obj->value[0] ) );
	    send_to_char( buf, ch );
	    sprintf( buf, "[v1] Regen Factor: [%ld]\r\n", obj->value[1]+100 );
	    send_to_char( buf, ch );
	    break;

	case ITEM_DECORATION:
	    sprintf_to_char(ch, "[v0] Regen+      : [%ld]\r\n", obj->value[0] );
	    sprintf_to_char(ch, "[v0] Mana+       : [%ld]\r\n", obj->value[1] );
	    break;

	case ITEM_PASSPORT:
	    sprintf_to_char(ch, "[v0] Type        : [%s]\r\n", flag_string(pass_type, obj->value[0]));
	    break;

	case ITEM_CONTAINER:
	    sprintf( buf,
		"[v0] Weight:    [%ld]\r\n"
		"[v1] Flags:     [%s]\r\n"
		"[v2] Key:       %s [%ld]\r\n"
		"[v3] Reduction: [%ld]\r\n",
		obj->value[0],
		flag_string( container_flags, obj->value[1] ),
                get_obj_index(obj->value[2])
                    ? get_obj_index(obj->value[2])->short_descr
                    : "none", obj->value[2],
                obj->value[3]);
	    send_to_char( buf, ch );
	    break;

	case ITEM_LOCKER:
                case ITEM_CAMERA:
            break;

        case ITEM_CLAN_LOCKER:
	    sprintf( buf,
		"[v4] Society:    [%ld]\r\n",
                 obj->value[4]);
	    send_to_char( buf, ch );
            break;

	case ITEM_DRINK_CON:
	    sprintf( buf,
	        "[v0] Liquid Total: [%ld]\r\n"
	        "[v1] Liquid Left:  [%ld]\r\n"
	        "[v2] Liquid:        %s\r\n"
	        "[v3] Poisoned:      %s\r\n"
	        "[v4] Spell:         %s\r\n",
	        obj->value[0],
	        obj->value[1],
	        flag_string( liquid_flags, obj->value[2] ),
	        obj->value[3] != 0 ? "Yes" : "No",
	        obj->value[4] > 0  ? effect_array[obj->value[4]].name 
                                   : "none");
	    send_to_char( buf, ch );
	    break;

	case ITEM_FOOD:
	    sprintf( buf,
		"[v0] Food hours: [%ld]\r\n"
     	        "[v1] Level:      [%ld]\r\n"
   	        "[v2] Spell:       %s\r\n"
		"[v3] Poisoned:    %s\r\n",
		obj->value[0],
      	        obj->value[1],
	        obj->value[2] > 0 ? effect_array[obj->value[2]].name 
                                  : "none",
		obj->value[3] != 0 ? "Yes" : "No" );
	    send_to_char( buf, ch );
	    break;

	case ITEM_FOUNTAIN:
	    sprintf( buf,
	        "[v0] Liquid:       %s\r\n"
	        "[v1] Poisoned:     %s\r\n"
    	        "[v2] Level:       [%ld]\r\n"
	        "[v3] Spell:        %s\r\n",
	        flag_string( liquid_flags, obj->value[0] ),
	        obj->value[1] != 0 ? "Yes" : "No",
	        obj->value[2],
	        obj->value[3] != -1 ? effect_array[obj->value[3]].name : "none");
	    send_to_char( buf, ch );
	    break;

	case ITEM_POOL:
	    sprintf_to_char(ch, "[v0] Liquid:       %s\r\n", flag_string( liquid_flags, obj->value[0]));
                    sprintf_to_char(ch, "[v1] Poisoned:     %s\r\n", obj->value[1] != 0 ? "Yes" : "No");
                    sprintf_to_char(ch, "[v2] Type:       [%s]\r\n", flag_string(pool_type, obj->value[2]));
	    break;

	case ITEM_MONEY:
                    sprintf_to_char(ch, "[v0] Gold:   [%ld]\r\n", obj->value[0] );
                    sprintf_to_char(ch, "[v1] Currency: %s\r\n", flag_string(currency_accept,obj->value[1]));
	    break;

	case ITEM_KEY_RING:
	    sprintf( buf,
		"[v0] Weight:    [%ld kg]\r\n"
		"[v1] Flags:     [%s]\r\n"
		"[v2] Key:        %s [%ld]\r\n",
		obj->value[0],
		flag_string( container_flags, obj->value[1] ),
                get_obj_index(obj->value[2])
                    ? get_obj_index(obj->value[2])->short_descr
                    : "none", obj->value[2]);
	    send_to_char( buf, ch );
	    break;

        case ITEM_BOOK:
          sprintf(buf,
               "[v0] Language: [%s]\r\n"
               "[v1] Rating  : [%ld]\r\n"
               "[v2] Skill 1 : [%s]\r\n"
               "[v3] Skill 2 : [%s]\r\n"
               "[v4] Skill 3 : [%s]\r\n",
                skill_array[obj->value[0]].name,
                obj->value[1],
                skill_array[obj->value[2]].name,
                skill_array[obj->value[3]].name,
                skill_array[obj->value[4]].name);
          send_to_char(buf, ch);
          break;

        case ITEM_GRIMOIRE:
          sprintf(buf,"[v0] Disciplines: [%s]\r\n",  flag_string(discipline_type, obj->value[0]));
          send_to_char(buf, ch);
          break;

        case ITEM_KEY:
          sprintf_to_char(ch,"[v0] Uses       : %d\r\n", obj->value[0]);
          break;

        case ITEM_IDOL:
          sprintf(buf,
               "[v0] Room: [%ld]\r\n"
               "[v1] Room: [%ld]\r\n"
               "[v2] Room: [%ld]\r\n"
               "[v3] Room: [%ld]\r\n"
               "[v4] Room: [%ld]\r\n",
                obj->value[0],
                obj->value[1],
                obj->value[2],
                obj->value[3],
                obj->value[4]);
          send_to_char(buf, ch);
          break;

    }

    return;
}



bool set_obj_values( CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, int value_num, char *argument) {

    switch( pObj->item_type ) {
        default:
            break;

        case ITEM_INSTRUMENT:
	    switch ( value_num )   {
	        default:
		    do_help( ch, "ITEM_INSTRUMENT" );
	            return FALSE;
	        case 0:
	             send_to_char( "INSTRUMENT TYPE SET.\r\n\r\n", ch );
	             pObj->value[0] = flag_value(instrument_type, argument );
	             break;
	        case 1:
	            send_to_char( "POWER SET.\r\n\r\n", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 4:
	            send_to_char( "SPELL TYPE  SET.\r\n\r\n", ch );
	            pObj->value[4] = get_effect_efn(argument );
	            break;
	    }
            break;
            
        case ITEM_LIGHT:
	    switch ( value_num )   {
	        default:
		    do_help( ch, "ITEM_LIGHT" );
	            return FALSE;
	        case 0:
	            send_to_char( "SWITCHABLE TOGGLED.\r\n\r\n", ch );
	            pObj->value[0] = (pObj->value[0] == 0 ) ? 1 : 0;
	            break;
	        case 1:
	             send_to_char( "REFILL TYPE SET.\r\n\r\n", ch );
	             pObj->value[1] = flag_value(refill_type, argument );
	             break;
	        case 2:
	            send_to_char( "HOURS OF LIGHT SET.\r\n\r\n", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	        case 3:
	            send_to_char( "on/off  SWITCH TOGGLED.\r\n\r\n", ch );
	            pObj->value[3] = (pObj->value[3] == 0 ) ? 1 : 0;
	            break;
	        case 4:
	            send_to_char( "MAX HOURS OF LIGHT SET.\r\n\r\n", ch );
	            pObj->value[4] = atoi( argument );
	            break;
	    }
            break;

        case ITEM_LIGHT_REFILL:
	    switch ( value_num )   {
	        default:
		    do_help( ch, "ITEM_LIGHT" );
	            return FALSE;
	            break;
	        case 1:
	             send_to_char( "PREFILL TYPE SET.\r\n\r\n", ch );
	             pObj->value[1] = flag_value(refill_type, argument );
	             break;
	        case 2:
	            send_to_char( "HOURS OF LIGHT SET.\r\n\r\n", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	    }
            break;

        case ITEM_PROTOCOL:
	    switch ( value_num )   {
	        default:
      	            do_help( ch, "ITEM_PROTOCOL" );
	            return FALSE;
	        case 0:
	            send_to_char( "PRODUCT SET.\r\n\r\n", ch );
	            pObj->value[0] = (atol(argument) <= 0 ) ? 0 : atol(argument);
	            break;
	        case 1:
	            send_to_char( "COMPONENT SET.\r\n\r\n", ch );
	            pObj->value[1] = (atol(argument) <= 0 ) ? 0 : atol(argument);
	            break;
	        case 2:
	            send_to_char( "COMPONENT SET.\r\n\r\n", ch );
	            pObj->value[2] = (atol(argument) <= 0 ) ? 0 : atol(argument);
	            break;
	        case 3:
	            send_to_char( "COMPONENT SET.\r\n\r\n", ch );
	            pObj->value[3] = (atol(argument) <= 0 ) ? 0 : atol(argument);
	            break;
	        case 4:
	            send_to_char( "COMPONENT SET.\r\n\r\n", ch );
	            pObj->value[4] = (atol(argument) <= 0 ) ? 0 : atol(argument);
	            break;
	    }
            break;

        case ITEM_WAND:
        case ITEM_STAFF:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_STAFF_WAND" );
	            return FALSE;
	        case 0:
	            send_to_char( "SPELL LEVEL SET.\r\n\r\n", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "TOTAL NUMBER OF CHARGES SET.\r\n\r\n", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "CURRENT NUMBER OF CHARGES SET.\r\n\r\n", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	        case 3:
	            send_to_char( "SPELL TYPE SET.\r\n", ch );
	            pObj->value[3] = get_effect_efn( argument );
	            break;
	    }
            break;

        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
	    switch ( value_num )	    {
	        default:
		    do_help( ch, "ITEM_SCROLL_POTION_PILL" );
	            return FALSE;
	        case 0:
	            send_to_char( "SPELL LEVEL SET.\r\n\r\n", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "SPELL TYPE 1 SET.\r\n\r\n", ch );
	            pObj->value[1] = get_effect_efn( argument );
	            break;
	        case 2:
	            send_to_char( "SPELL TYPE 2 SET.\r\n\r\n", ch );
	            pObj->value[2] = get_effect_efn( argument );
	            break;
	        case 3:
	            send_to_char( "SPELL TYPE 3 SET.\r\n\r\n", ch );
	            pObj->value[3] = get_effect_efn( argument );
	            break;
 	    }
	    break;

        case ITEM_HERB:
	    switch ( value_num )	    {
	        default:
		    do_help( ch, "ITEM_SCROLL_POTION_PILL" );
     	                     return FALSE;
	        case 0:
	            send_to_char( "SPELL LEVEL SET.\r\n\r\n", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "SPELL TYPE 1 SET.\r\n\r\n", ch );
	            pObj->value[1] = get_effect_efn( argument );
	            break;
	        case 2:
	            send_to_char( "SPELL TYPE 2 SET.\r\n\r\n", ch );
	            pObj->value[2] = get_effect_efn( argument );
	            break;
	        case 3:
	            send_to_char( "POISON VALUE TOGGLED.\r\n\r\n", ch );
	            pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
	            break;
	        case 4:
	            send_to_char( "TELEPOP VALUE TOGGLED.\r\n\r\n", ch );
	            pObj->value[4] = ( pObj->value[4] == 0 ) ? 1 : 0;
	            break;
 	    }
	    break;

        case ITEM_PIPE:
	    switch ( value_num )	    {
	        default:
		    do_help( ch, "ITEM_SCROLL_POTION_PILL" );
     	                     return FALSE;
	        case 0:
	            send_to_char( "SPELL MOD SET.\r\n\r\n", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "USES TOGGLED.\r\n\r\n", ch );
	            pObj->value[1] = ( pObj->value[1] == 0 ) ? 1 : 0;
	            break;
	        case 2:
	            send_to_char( "LOAD SPELL LEVEL SET.\r\n\r\n", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	        case 3:
	            send_to_char( "LOAD SPELL TYPE 1 SET.\r\n\r\n", ch );
	            pObj->value[3] = get_effect_efn( argument );
	            break;
	        case 4:
	            send_to_char( "LOAD SPELL TYPE 2 SET.\r\n\r\n", ch );
	            pObj->value[4] = get_effect_efn( argument );
	            break;

 	    }
	    break;

	case ITEM_PORTAL:
                        switch (value_num) {
		default:
		    send_to_char ("Only value0 and value1 used for portals.\r\n",ch);
		    break;
		case 0:
                                    if (!str_cmp(argument, "random")
                                    || atoi(argument) == 0) {
                                               pObj->value[0] = 0;
		    } else if (get_room_index (atoi(argument)) == NULL) {
                                               send_to_char ("Invalid room vnum.\r\n",ch);
		    } else {   
			pObj->value[0]=atoi(argument); 	
			send_to_char ("Destination room set.\r\n",ch);
		    }
		    break;
		case 1:
		    if (get_obj_index (atoi(argument)) == NULL
                                    && atoi(argument) != 0) {
			send_to_char ("Invalid link object vnum.\r\n",ch);
		    }    else    {
			pObj->value[1]=atoi(argument);
			send_to_char ("Link object set.\r\n",ch);
		    }
		    break;
		case 2:
		    if (get_obj_index (atoi(argument)) == NULL)		    {
			send_to_char ("Invalid portal key object vnum.\r\n",ch);
		    }    else    {
			pObj->value[2]=atoi(argument);
			send_to_char ("Key object set.\r\n",ch);
		    }
		    break;
                                 case 4:
		    send_to_char( "PORTAL CLASS SET.\r\n\r\n", ch );
		    pObj->value[4] = flag_value( portal_class, argument );
		    break;
                         } 
                         break;


	case ITEM_TREE:
                        switch (value_num) {
                           case 0:
	              send_to_char( "TREE SIZE SET.\r\n\r\n", ch );
	              pObj->value[0] = flag_value(tree_size, argument );
	              break;
                           case 1:
	              send_to_char( "TREE TYPE SET.\r\n\r\n", ch );
	              pObj->value[1] = flag_value(tree_type, argument );
	              break;
                         } 
                         break;

	case ITEM_SHARE:
                        switch (value_num) {
                           case 0:
	              send_to_char( "SHARE ID SET.\r\n\r\n", ch );
	              pObj->value[0] = atoi(argument);
	              break;
                           case 1:
	              send_to_char( "SHARE NUMBER SET.\r\n\r\n", ch );
	              pObj->value[1] = atoi(argument);
	              break;
                         } 
                         break;

	case ITEM_FIGURINE:
                        switch (value_num) {
                           case 0:
	              send_to_char( "TEMPLATE VNUM SET.\r\n\r\n", ch );
	              pObj->value[0] = atoi(argument);
	              break;
                           case 1:
	              send_to_char( "FIGURINE BEHAVIOR SET.\r\n\r\n", ch );
	              pObj->value[1] = flag_value( figurine_behavior, argument );
  	              break;
	           case 2:
	               send_to_char( "TRUE TYPE SET.\r\n\r\n", ch );
	              pObj->value[2] = flag_value(figurine_type, argument );
  	              break;
                         } 
                         break;

/* ARMOR for ROM: */

        case ITEM_ARMOR:
	    switch ( value_num )	    {
	        default:
		    do_help( ch, "ITEM_ARMOR" );
		    return FALSE;
	        case 0:
		    send_to_char( "AC PIERCE SET.\r\n\r\n", ch );
		    pObj->value[0] = atoi( argument );
		    break;
	        case 1:
		    send_to_char( "AC BASH SET.\r\n\r\n", ch );
		    pObj->value[1] = atoi( argument );
		    break;
	        case 2:
		    send_to_char( "AC SLASH SET.\r\n\r\n", ch );
		    pObj->value[2] = atoi( argument );
		    break;
	        case 3:
		    send_to_char( "AC EXOTIC SET.\r\n\r\n", ch );
		    pObj->value[3] = atoi( argument );
		    break;
	    }
	    break;

        case ITEM_RAW:
	    switch ( value_num )	    {
                        default:
                          return FALSE;
	        case 0:
	            send_to_char( "DICE MOD SET.\r\n\r\n", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "WEIGHT MOD SET.\r\n\r\n", ch );
	            pObj->value[1] = atoi( argument );
	            break;
                        }
                        break;          

         case ITEM_BONDAGE:
	    send_to_char( "STRENGTH SET.\r\n\r\n", ch );
	    pObj->value[0] = atoi( argument );
	    break;

         case ITEM_DOLL:
	    send_to_char( "POWER SET.\r\n\r\n", ch );
	    pObj->value[0] = atoi( argument );
	    break;

         case ITEM_FOCUS:
	    send_to_char( "ZMASK TOGGLED.\r\n\r\n", ch );
	    TOGGLE_BIT(pObj->zmask, flag_value( zmask_flags, argument ));
	    break;

         case ITEM_AMMO:
	    switch ( value_num )   {
	        default:
		    do_help( ch, "ITEM_AMMO" );
	            return FALSE;
                         case 0:
	    send_to_char( "SHOTS SET.\r\n\r\n", ch );
	    pObj->value[0] = atoi( argument );
                     break;
	        case 4:
				{
				long value=0;
                    send_to_char( "SPECIAL WEAPON TYPE TOGGLED.\r\n\r\n", ch );
				value = flag_value (weapon_type, argument);
				if (value != NO_FLAG)
					TOGGLE_BIT (pObj->value[4], value);
				}
		    break;
                          }
                          break;


     case ITEM_SLOTMACHINE:
       switch (value_num)  {
         case 0:
           send_to_char( "GOLD COST SET.\r\n\r\n", ch);
           pObj->value[0] = atoi( argument );
           break;
         case 1:
           send_to_char( "JACKPOT VALUE SET.\r\n\r\n", ch);
           pObj->value[1] = atoi( argument );
           break;
         case 2:
           send_to_char( "OWNER SET.\r\n\r\n", ch);
           pObj->value[2] = atoi( argument );
           break;
      }
      break;


/* WEAPONS changed in ROM */

        case ITEM_WEAPON:
	    switch ( value_num )   {
	        default:
		    do_help( ch, "ITEM_WEAPON" );
	            return FALSE;
	        case 0:
		    send_to_char( "WEAPON CLASS SET.\r\n\r\n", ch );
		    pObj->value[0] = flag_value( weapon_class, argument );
		    break;
	        case 1:
	            send_to_char( "NUMBER OF DICE SET.\r\n\r\n", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "TYPE OF DICE SET.\r\n\r\n", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	        case 3:
	            send_to_char( "WEAPON TYPE SET.\r\n\r\n", ch );
	            pObj->value[3] = flag_value( weapon_flags, argument );
	            break;
	        case 4:
				{
				long value=0;
                    send_to_char( "SPECIAL WEAPON TYPE TOGGLED.\r\n\r\n", ch );
				value = flag_value (weapon_type, argument);
				if (value != NO_FLAG)
					TOGGLE_BIT (pObj->value[4], value);
				}
		    break;
	    }
            break;

        case ITEM_TRAP:
	    switch ( value_num )   {
	        default:
                            do_help( ch, "ITEM_TRAP" );
	            return FALSE;
	        case 0:
		    send_to_char( "TRAP CLASS SET.\r\n\r\n", ch );
		    pObj->value[0] = flag_value( trap_class, argument );
		    break;
	        case 1:
	            send_to_char( "NUMBER OF DICE SET.\r\n\r\n", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "TYPE OF DICE SET.\r\n\r\n", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	    }
            break;

        case ITEM_PHOTOGRAPH:
	    switch ( value_num )   {
	        default:
                            do_help( ch, "ITEM_PHOTOGRAPH" );
	            return FALSE;
	        case 0:
		    send_to_char( "PHOTO STATE SET.\r\n\r\n", ch );
		    pObj->value[0] = flag_value( photo_state, argument );
		    break;
	    }
            break;

        case ITEM_FURNITURE:
	    switch ( value_num )   {
	        default:
                            do_help( ch, "ITEM_FURNITURE" );
	            return FALSE;
	        case 0:
    	          {
      	           long value=0;
                           send_to_char( "FURNITURE TYPE TOGGLED.\r\n\r\n", ch );
	           value = flag_value (furniture_class, argument);
	           if (value != NO_FLAG) TOGGLE_BIT (pObj->value[0], value);
	           }
	           break;

	        case 1:
	            send_to_char( "REGENERATION SET.\r\n\r\n", ch );
	            pObj->value[1] = atoi( argument ) - 100;
	            break;
	    }
            break;

        case ITEM_DECORATION:
	    switch ( value_num )   {
	        default:
                            do_help( ch, "ITEM_DECORATION" );
	            return FALSE;
	        case 0:
	            send_to_char( "REGENERATION + SET.\r\n\r\n", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "MANA + SET.\r\n\r\n", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	    }
            break;

        case ITEM_PASSPORT:
	    switch ( value_num )   {
	        default:
                            do_help( ch, "ITEM_PASSPORT" );
	            return FALSE;
	        case 0:
                           send_to_char( "PASSPORT TYPE SET.\r\n\r\n", ch );
	           pObj->value[0] = flag_value (pass_type, argument);
	           break;
	    }
            break;


        case ITEM_KEY:
	    switch ( value_num )   {
	        default:
	            return FALSE;
	        case 0:
	            send_to_char( "USES SET.\r\n\r\n", ch );
	            pObj->value[0] = atoi(argument );
	            break;
	    }
            break;

        case ITEM_GRIMOIRE:
	    switch ( value_num )   {
	        default:
	            return FALSE;
	        case 0:
                           {
      	           long value = 0;
                           send_to_char( "DISCIPLINE TYPE TOGGLED.\r\n\r\n", ch );
	           value = flag_value (discipline_type, argument);
	           if (value != NO_FLAG) TOGGLE_BIT (pObj->value[0], value);
                           }
	           break;
	    }
            break;

        case ITEM_EXPLOSIVE:
	    switch ( value_num )   {
	        default:
                            do_help( ch, "ITEM_WEAPON" );
	            return FALSE;
	        case 1:
	            send_to_char( "NUMBER OF DICE SET.\r\n\r\n", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "TYPE OF DICE SET.\r\n\r\n", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	        case 3:
	            send_to_char( "LOWER TIMER SET.\r\n\r\n", ch );
	            pObj->value[3] = atoi( argument );
	            break;
	        case 4:
	            send_to_char( "HIGHER TIMER SET.\r\n\r\n", ch );
	            pObj->value[4] = atoi( argument );
	            break;
	    }
            break;

        case ITEM_CONTAINER:
	    switch ( value_num )    {
		int value;
		
		default:
		    do_help( ch, "ITEM_CONTAINER" );
	            return FALSE;
		case 0:
	            send_to_char( "WEIGHT CAPACITY SET.\r\n\r\n", ch );
	            pObj->value[0] = atoi( argument );
	            break;
		case 1:
	            if ( ( value = flag_value( container_flags, argument ) )
	              != NO_FLAG )
	        	TOGGLE_BIT(pObj->value[1], value);
		    else
		    {
			do_help ( ch, "ITEM_CONTAINER" );
			return FALSE;
		    }
	            send_to_char( "CONTAINER TYPE SET.\r\n\r\n", ch );
	            break;
		case 2:
		    if ( atoi(argument) != 0 )
		    {
			if ( !get_obj_index( atoi( argument ) ) )
			{
			    send_to_char( "THERE IS NO SUCH ITEM.\r\n\r\n", ch );
			    return FALSE;
			}

			if ( get_obj_index( atoi( argument ) )->item_type != ITEM_KEY ) {
			    send_to_char( "THAT ITEM IS NOT A KEY.\r\n\r\n", ch );
			    return FALSE;
			}
		    }
		    send_to_char( "CONTAINER KEY SET.\r\n\r\n", ch );
		    pObj->value[2] = atoi( argument );
		    break;
		case 3:
	            send_to_char( "WEIGHT REDUCTION SET.\r\n\r\n", ch );
	            pObj->value[3] = atoi( argument );
                    if ( pObj->value[3] <= 25 
                      || pObj->value[3] > 200 ) {
                      pObj->value[3] = 100;
                      send_to_char("{YWarning: Range is 25 to 200\r\n", ch);
                    }
	            break;
	    }
	    break;

	case ITEM_DRINK_CON:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_DRINK" );
/* OLC		    do_help( ch, "liquids" );    */
	            return FALSE;
	        case 0:
	            send_to_char( "MAXIMUM AMOUT OF LIQUID HOURS SET.\r\n\r\n", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "CURRENT AMOUNT OF LIQUID HOURS SET.\r\n\r\n", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "LIQUID TYPE SET.\r\n\r\n", ch );
	            pObj->value[2] = flag_value( liquid_flags, argument );
	            break;
	        case 3:
	            send_to_char( "POISON VALUE TOGGLED.\r\n\r\n", ch );
	            pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
	            break;
	        case 4:
	            send_to_char( "SPELL TYPE  SET.\r\n\r\n", ch );
	            pObj->value[4] = get_effect_efn( argument );
	            break;
	    }
            break;

        case ITEM_CLAN_LOCKER:
	    switch ( value_num )
	    {
		default:
		    do_help( ch, "ITEM_CLAN_LOCKER" );
	            return FALSE;
		case 4:
	            send_to_char( "SOCIETY SET.\r\n\r\n", ch );
	            pObj->value[4] = atoi( argument );
	            break;
            }
       
            break;
 
	case ITEM_FOOD:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_FOOD" );
	            return FALSE;
	        case 0:
	            send_to_char( "HOURS OF FOOD SET.\r\n\r\n", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "SPELL LEVEL SET.\r\n\r\n", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "SPELL TYPE 1 SET.\r\n\r\n", ch );
	            pObj->value[2] = get_effect_efn( argument );
	            break;
	        case 3:
	            send_to_char( "POISON VALUE TOGGLED.\r\n\r\n", ch );
	            pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
	            break;
	    }
            break;

	case ITEM_FOUNTAIN:
	    switch ( value_num )    {
	        default:
                            do_help( ch, "ITEM_FOUNTAIN" );
	            return FALSE;
	        case 0:
	            send_to_char( "LIQUID TYPE SET.\r\n\r\n", ch );
	            pObj->value[0] = flag_value( liquid_flags, argument );
	            break;
	        case 1:
	            send_to_char( "POISON VALUE TOGGLED.\r\n\r\n", ch );
	            pObj->value[1] = ( pObj->value[1] == 0 ) ? 1 : 0;
	            break;
	        case 2:
	            send_to_char( "SPELL LEVEL SET.\r\n\r\n", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	        case 3:
	            send_to_char( "SPELL TYPE  SET.\r\n\r\n", ch );
	            pObj->value[3] = get_effect_efn( argument );
	            break;
	    }
            break;

	case ITEM_POOL:
	    switch ( value_num )    {
	        default:
                            do_help( ch, "ITEM_FOUNTAIN" );
	            return FALSE;
	        case 0:
	            send_to_char( "LIQUID TYPE SET.\r\n\r\n", ch );
	            pObj->value[0] = flag_value( liquid_flags, argument );
	            break;
	        case 1:
	            send_to_char( "POISON VALUE TOGGLED.\r\n\r\n", ch );
	            pObj->value[1] = ( pObj->value[1] == 0 ) ? 1 : 0;
	            break;
	        case 2:
	            send_to_char( "POOL TYPE SET.\r\n\r\n", ch );
	            pObj->value[2] = flag_value(pool_type, argument );
                            break;
	    }
            break;

	case ITEM_MONEY:
	    switch ( value_num ) {
	        default:
		    do_help( ch, "ITEM_MONEY" );
	            return FALSE;
	        case 0:
	            send_to_char( "GOLD AMOUNT SET.\r\n\r\n", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "CURRENCY SET.\r\n\r\n", ch );
	            pObj->value[1] = flag_value(currency_accept, argument );
                            break;
	    }
            break;

        case ITEM_KEY_RING:
	    switch ( value_num )
	    {
		int value;
		
		default:
		    do_help( ch, "ITEM_KEY_RING" );
	            return FALSE;
		case 0:
	            send_to_char( "WEIGHT CAPACITY SET.\r\n\r\n", ch );
	            pObj->value[0] = atoi( argument );
	            break;
		case 1:
	            if ( ( value = flag_value( container_flags, argument ) )
	              != NO_FLAG )
	        	TOGGLE_BIT(pObj->value[1], value);
		    else
		    {
			do_help ( ch, "ITEM_KEY_RING" );
			return FALSE;
		    }
	            send_to_char( "KEY RING TYPE SET.\r\n\r\n", ch );
	            break;
		case 2:
		    if ( atoi(argument) != 0 )
		    {
			if ( !get_obj_index( atoi( argument ) ) )
			{
			    send_to_char( "THERE IS NO SUCH ITEM.\r\n\r\n", ch );
			    return FALSE;
			}

			if ( get_obj_index( atoi( argument ) )->item_type != ITEM_KEY )
			{
			    send_to_char( "THAT ITEM IS NOT A KEY.\r\n\r\n", ch );
			    return FALSE;
			}
		    }
		    send_to_char( "KEY RING KEY SET.\r\n\r\n", ch );
		    pObj->value[2] = atoi( argument );
		    break;
	    }
	    break;

        case ITEM_BOOK:
            switch (value_num) {

              int val;

              default:
                do_help(ch, "ITEM_BOOK");
                return FALSE;
        
              case 1:
                val = atoi(argument);  

                if (val < 0 || val > 100) {
                   send_to_char("V1 from 0 to 100.\r\n", ch);
                   return FALSE;
                }

                pObj->value[1] = val;
                break;    
            
              case 0: 
              case 2:
              case 3:
              case 4:
                val = get_skill_sn(argument);

                if (val < 0) {
                  send_to_char("That is not a valid skill.\r\n", ch);
                  return FALSE;
                } 

                pObj->value[value_num] = val;
                break;

            }
            break;

        case ITEM_IDOL:
            switch (value_num) {

              int val;
              ROOM_INDEX_DATA *room; 

              default:
                do_help(ch, "ITEM_IDOL");
                return FALSE;
        
              case 0: 
              case 1:
              case 2:
              case 3:
              case 4:
                val = atoi(argument);

                room = get_room_index(val);

                if (room == NULL) {
                  send_to_char("That is not a valid room.\r\n", ch);
                  return FALSE;
                } 

                pObj->value[value_num] = val;
                break;

            }
            break;

    }

    show_obj_values( ch, pObj );

    return TRUE;
}

OEDIT( oedit_unused ) {
	ROOM_INDEX_DATA *pRoom;
	AREA_DATA *pArea;
	OBJ_INDEX_DATA *tmp;
	char outbuf[MAX_STRING_LENGTH];
	char buf[128];
	int counter;
        int fcount; 

	EDIT_ROOM(ch, pRoom);
	pArea=pRoom->area;
	sprintf (outbuf, "Area name: %s\r\n",((pArea->name) ? (pArea->name) : "none"));
	strcat (outbuf, "UNUSED OBJECT VNUMS\r\n-------------------\r\n");

	counter = pArea->lvnum;
        fcount = 0;

	for (; (counter <= pArea->uvnum); counter++) {

	  if ((tmp = get_obj_index(counter)) == NULL) {
	    sprintf (buf, " %5d,", counter);
	    strcat (outbuf, buf);
	  }

          if (++fcount % 10 == 0) {
            strcat(outbuf, "\r\n");
	  }
	}
	
        if (fcount % 10 != 0) {
          strcat(outbuf, "\r\n");
        }

	page_to_char (outbuf, ch);
	return TRUE;
}

OEDIT( oedit_used ) {
	ROOM_INDEX_DATA *pRoom;
	AREA_DATA *pArea;
	OBJ_INDEX_DATA *tmp;
	char outbuf[5*MAX_STRING_LENGTH];
	char buf[128];
	int counter; 

	EDIT_ROOM(ch, pRoom);
	pArea=pRoom->area;
	sprintf (outbuf, "Area name: %s\r\n",((pArea->name) ? (pArea->name) : "none"));
	strcat  (outbuf, "USED OBJECT VNUMS\r\n---- --- -----\r\n");
	counter = pArea->lvnum;
	for (; (counter <= pArea->uvnum); counter++)
		{
		if ((tmp = get_obj_index(counter)) != NULL)
			{
			sprintf (buf, "[%d] %s [level %d]\r\n", counter, ((tmp->short_descr) ? (tmp->short_descr) : ""), tmp->level);
			strcat (outbuf, buf);
			}
		}
	
	page_to_char (outbuf, ch);
	return TRUE;
}


OEDIT( oedit_show ) {
OBJ_INDEX_DATA *pObj;
char buf[MAX_STRING_LENGTH];
char apply_buf[MAX_STRING_LENGTH];
char affect_buf[MAX_STRING_LENGTH];
char skill_buf[MAX_STRING_LENGTH];
char tmp_buf[MAX_STRING_LENGTH];
char vb[MAX_STRING_LENGTH];
AFFECT_DATA *paf;
int cnt;
bool Fapply=FALSE;
bool Faffect=FALSE;
bool Fskill=FALSE;
	
    apply_buf[0]='\0';
    affect_buf[0]='\0';
    skill_buf[0]='\0';
    tmp_buf[0]='\0';

    EDIT_OBJ(ch, pObj);
	
    if (pObj == NULL) {
        return FALSE;
    }

    sprintf_to_char(ch, "Name: [{c%s{x]  Area: [{c%3d{x] %s\r\n", pObj->name, !pObj->area ? -1: pObj->area->vnum, !pObj->area ? "No Area" : pObj->area->name );

    format_vnum(pObj->vnum, vb);
    sprintf_to_char(ch, "Vnum: [{c%11s{x]  Type: [{c%s{x] "
                  "Level: [{c%3d{x] Repop: [{c%3d{x]\r\n", vb, flag_string( type_flags, pObj->item_type ), pObj->level, pObj->repop );

    sprintf_to_char(ch, "Zones: [{c%s{x]\r\n", flag_string( zmask_flags, pObj->zmask));  
     
    sprintf_to_char(ch, "Wear flags:  [{c%s{x]\r\n", flag_string( wear_flags, pObj->wear_flags ) );
    if (pObj->wear_cond) { 
         buf[0] = '\0';
         print_econd_to_buffer(buf, pObj->wear_cond, " wear - ", NULL); 
         send_to_char(buf, ch);
    }

    sprintf_to_char(ch, "Extra flags: [{c%s{x]\r\n", flag_string( extra_flags, pObj->extra_flags ) );

    if (IS_SET(pObj->extra_flags, ITEM_ARTIFACT)) {
         if (pObj->artifact_data) {
             sprintf_to_char(ch, "Artifact:    [{c%s{x]\r\n", flag_string(artifact_flags, pObj->artifact_data->power ) );
             sprintf_to_char(ch, "Attract:     [{c%3d{x]     Pulse:  [{c%3d{x]\r\n", pObj->artifact_data->attract, pObj->artifact_data->pulse);
         } else {
             send_to_char("Artifact:    [none]\r\n", ch);
             send_to_char("Attract:     [{c  0{x]     Pulse:  [{c  0{x]\r\n", ch);
         }
    }

    sprintf_to_char(ch, "Material: [{c%s{x] Condition: [{c%3d{x] "
                  "Weight: [{c%5d{x] Cost: [{c%5d{x]\r\n", flag_string( material_type, pObj->material ), pObj->condition, pObj->weight, pObj->cost );

    if ( pObj->extra_descr )    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Ex desc kwd:", ch );

	for ( ed = pObj->extra_descr; ed; ed = ed->next )	{
	    send_to_char( " [", ch );
	    send_to_char( ed->keyword, ch );
	    send_to_char( "]", ch );
	}

	send_to_char( "\r\n", ch );
    }

   /* Summarize the extra stuff... */

    strcpy(buf, "Smarts: [{x");

    if (pObj->anchors != NULL) {
      strcat(buf, "Anchors ");
    }

    if (buf[11] == '\0') { 
      strcat(buf, "None ");
    }

    strcat(buf, "{x]\r\n");
    send_to_char(buf, ch);

   /* Descriptions, short and long... */

    if ( pObj->image != NULL ) {
      sprintf(buf, "Mob image:  %s\r\n", pObj->image);
      send_to_char(buf, ch);
    }

    sprintf( buf, "Short desc:  %s\r\nLong desc:\r\n     %s\r\n", pObj->short_descr, pObj->description );

    send_to_char( buf, ch );

   /* Affects, applies and skills... */

    for ( cnt = 0, paf = pObj->affected; paf; paf = paf->next ) {

        if ( paf->location == APPLY_SKILL ) {
            if ( validSkill(paf->skill)  && paf->skill > 0 ) {
	 sprintf( tmp_buf, "[%4d] %-8d %s\r\n", cnt, paf->modifier, skill_array[paf->skill].name);
                 strcat (skill_buf, tmp_buf);
	 Fskill=TRUE;
            }

        } else if ( paf->location == APPLY_EFFECT) {
            if ( valid_effect(paf->skill)  && paf->skill > 0 ) {
	 sprintf( tmp_buf, "[%4d] %s (Recharge Speed: %d)\r\n", cnt, effect_array[paf->skill].name, (-paf->duration));
                 strcat (skill_buf, tmp_buf);
	 Fskill=TRUE;
            }

        } else if ( paf->location == APPLY_IMMUNITY   ||  paf->location == APPLY_RESISTANCE || paf->location == APPLY_ENV_IMMUNITY) {
	    sprintf( tmp_buf, "[%4d] %-8s", cnt, flag_string(immapp_flags, paf->modifier));
	    strcat (apply_buf, tmp_buf);
	    sprintf( tmp_buf, " %s\r\n", flag_string( apply_flags, paf->location ) );
	    strcat (apply_buf, tmp_buf);
	    Fapply=TRUE;

          } else if (paf->bitvector) {
                    sprintf (tmp_buf, "[%4d]          %s\r\n",  cnt,  flag_string(affect_flags, paf->bitvector) );
	    strcat (affect_buf, tmp_buf);
	    Faffect=TRUE;

	} else {
	    sprintf( tmp_buf, "[%4d] %-8d %s\r\n",
                               cnt,
	                       paf->modifier,
                               flag_string( apply_flags, paf->location ) );
	    strcat (apply_buf, tmp_buf);
	    Fapply=TRUE;
	}
	cnt++;
    }

    if (Faffect) {
        send_to_char( "Number          Spell Affect\r\n",ch);
        send_to_char( "------          ------------------------\r\n",ch);
        send_to_char(affect_buf, ch);
    }

    if (Fapply) {
        send_to_char( "Number Modifier Affects\r\n", ch );
        send_to_char( "------ -------- ------------------------\r\n", ch );
        send_to_char(apply_buf, ch);
    }

    if (Fskill) {
        send_to_char( "Number Modifier Skill Affected\r\n",ch);
        send_to_char( "------ -------- ------------------------\r\n",ch);
        send_to_char(skill_buf, ch);
    }

    send_to_char("Values\r\n------\r\n",ch);

    show_obj_values( ch, pObj );

    return FALSE;
}


/*
 * Need to issue warning if flag isn't valid. -- does so now -- Hugin.
 */
OEDIT( oedit_addapply ) {
int value;
OBJ_INDEX_DATA *pObj;
AFFECT_DATA *pAf;
char loc[MAX_STRING_LENGTH];
char mod[MAX_STRING_LENGTH];
int immtype;

    EDIT_OBJ(ch, pObj);

    argument = one_argument( argument, loc );
    one_argument( argument, mod );

    if ( loc[0] == '\0' || mod[0] == '\0')    {
	send_to_char( "Syntax:  addapply [location] [#mod]\r\n", ch );
	send_to_char( "? apply for valid locations.\r\n", ch);
	send_to_char( "Ex:  addapply hp 100\r\n",ch);
	return FALSE;
    }

    if ( ( value = flag_value( apply_flags, loc ) ) == NO_FLAG ) {
        send_to_char( "Valid applies are:\r\n", ch );
	show_help( ch, "? apply"  );
	return FALSE;
    }

    switch (value) {

        case APPLY_IMMUNITY:
            immtype=0;
            immtype = flag_value(immapp_flags, mod);

            if (immtype>0) {
               pAf             =   new_affect();
               pAf->where      =   APPLY_TO_IMMUNE;
               pAf->location   =   value;
               pAf->skill      =   0;
               pAf->modifier   =   immtype;
               pAf->type       =   SKILL_UNDEFINED;
               pAf->afn        =   0;
               pAf->duration   =   -1;
               pAf->bitvector  =   0;
               pAf->next       =   pObj->affected;
               pObj->affected  =   pAf;

               send_to_char( "Apply added.\r\n", ch);
            }
            break;

        case APPLY_ENV_IMMUNITY:
            immtype=0;
            immtype = flag_value(immapp_flags, mod);

            if (immtype>0) {
               pAf             	=   new_affect();
               pAf->where      	=   APPLY_TO_ENV_IMMUNE;
               pAf->location   	=   value;
               pAf->skill      	=   0;
               pAf->modifier   	=   immtype;
               pAf->type       	=   SKILL_UNDEFINED;
               pAf->afn       	=   0;
               pAf->duration   	=   -1;
               pAf->bitvector  	=   0;
               pAf->next       	=   pObj->affected;
               pObj->affected  	=   pAf;

               send_to_char( "Apply added.\r\n", ch);
            }
            break;

        case APPLY_RESISTANCE:
            immtype=0;
            immtype = flag_value(immapp_flags, mod);

            if (immtype>0) {
               pAf             =   new_affect();
               pAf->where      =   APPLY_TO_RESIST;
               pAf->location   =   value;
               pAf->skill      =   0;
               pAf->modifier   =   immtype;
               pAf->type       =   SKILL_UNDEFINED;
               pAf->afn        =   0;
               pAf->duration   =   -1;
               pAf->bitvector  =   0;
               pAf->next       =   pObj->affected;
               pObj->affected  =   pAf;

               send_to_char( "Apply added.\r\n", ch);
            }
            break;

        default:
            if (!is_number(mod)) {
                send_to_char( "Syntax:  addapply [location] [#mod]\r\n", ch );
                send_to_char( "? apply for valid locations.\r\n", ch);
                send_to_char( "Ex:  addapply hp 100\r\n",ch);
                return FALSE;
            }

            pAf             =   new_affect();
            pAf->location   =   value;
            pAf->skill	    =   0;
            pAf->modifier   =   atoi( mod );
            pAf->type       =   SKILL_UNDEFINED;
            pAf->afn        =   0;
            pAf->duration   =   -1;
            pAf->bitvector  =   0;
            pAf->next       =   pObj->affected;
            pObj->affected  =   pAf;

            send_to_char( "Apply added.\r\n", ch);
            break;
    }

    return TRUE;
}


OEDIT( oedit_addmax ) {
    char loc[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char *trans;

    argument = one_argument( argument, loc );
    one_argument( argument, mod );

    if ( loc[0] == '\0' || mod[0] == '\0')    {
	send_to_char( "Syntax:  addmax [location] [#mod]\r\n", ch );
	send_to_char( "? apply for valid locations.\r\n", ch);
	send_to_char( "Ex:  addmax strength 3\r\n",ch);
	return FALSE;
    }

    sprintf(buf, "max-%s %d", loc, atoi(mod));
    trans = strdup(buf);
    oedit_addapply(ch, trans);   
    return TRUE;
}


/*
 * Need to issue warning if flag isn't valid. -- does so now -- Hugin.
 */
OEDIT( oedit_addskill )
{
    int value;
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    char loc[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];

    EDIT_OBJ(ch, pObj);

    argument = one_argument( argument, loc );
    one_argument( argument, mod );

    if ( loc[0] == '\0' || mod[0] == '\0' || !is_number( mod ) )
    {
	send_to_char( "Syntax:  addskill 'skill' #mod\r\n", ch );
	send_to_char( "skill ? valid skills.\r\n", ch);
	send_to_char( "Ex:  addskill 'spell casting' 100\r\n",ch);
	return FALSE;
    }

    value = get_skill_sn(loc );

    if ( value < 1 ) {
        send_to_char( "Invalid skill!\r\n", ch );
	return FALSE;
    }

    pAf             =   new_affect();
    pAf->location   =   APPLY_SKILL; 
    pAf->skill	    =   value;
    pAf->modifier   =   atoi( mod );
    pAf->type       =   SKILL_UNDEFINED;
    pAf->afn        =   0;
    pAf->duration   =   -1;
    pAf->bitvector  =   0;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

    send_to_char( "Skill added.\r\n", ch);
    return TRUE;
}

OEDIT( oedit_addeffect)
{
    int value;
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    char loc[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];
    int regen;

    EDIT_OBJ(ch, pObj);

    argument = one_argument( argument, loc );
    one_argument( argument, mod );
    regen = atoi(mod);

    if ( loc[0] == '\0' || regen < 1 || regen > 9)    {
	send_to_char( "Syntax:  addeffect 'effect' <recharge pulse>\r\n", ch );
	send_to_char( "Ex:  addeffect 'bless' 2\r\n",ch);
	return FALSE;
    }

    value = get_effect_efn(loc );

    if ( value < 1 ) {
        send_to_char( "Invalid effect!\r\n", ch );
        return FALSE;
    }

    pAf             =   new_affect();
    pAf->location   =   APPLY_EFFECT; 
    pAf->skill	    =   value;
    pAf->modifier   =   0;
    pAf->type       =   SKILL_UNDEFINED;
    pAf->afn        =   0;
    pAf->duration   =   -regen;
    pAf->bitvector  =   0;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

    send_to_char( "Pulse Effect added.\r\n", ch);
    return TRUE;
}

OEDIT( oedit_addaffect )
{
    long long value;
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    char affect_name[MAX_STRING_LENGTH];

    EDIT_OBJ(ch, pObj);

    one_argument( argument, affect_name);

    if ( affect_name[0] == '\0' )
    {
	send_to_char( "Syntax:  addaffect [affect_name]\r\n", ch );
	send_to_char( "? affect for valid affects.\r\n", ch);
	send_to_char( "Ex:  addaffect sanctuary\r\n",ch);
	return FALSE;
    }

    value = flag_value_long( affect_flags, affect_name );

    if ( value == NO_FLAG ) /* Hugin */
    {
        send_to_char( "Valid affects are:\r\n", ch );
	show_help( ch, "? affect"  );
	return FALSE;
    }

    pAf             =   new_affect();
    pAf->location   =   0;
    pAf->skill	    =   0;
    pAf->modifier   =   0;
    pAf->type	    =	SKILL_UNDEFINED;
    pAf->afn        =   0;
    pAf->duration   =   -1;
    pAf->bitvector  =   value;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

    send_to_char( "Affect added.\r\n", ch);
    return TRUE;
}



/*
 * My thanks to Hans Hvidsten Birkeland and Noam Krendel(Walker)
 * for really teaching me how to manipulate pointers.
 */
OEDIT( oedit_delaffect )
{
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    AFFECT_DATA *pAf_next;
    char affect[MAX_STRING_LENGTH];
    int  value;
    int  cnt = 0;

    EDIT_OBJ(ch, pObj);

    one_argument( argument, affect );

    if ( !is_number( affect ) || affect[0] == '\0' )
    {
	send_to_char( "Syntax:  delaffect [#affect]\r\n", ch );
	return FALSE;
    }

    value = atoi( affect );

    if ( value < 0 )
    {
	send_to_char( "Only non-negative affect-numbers allowed.\r\n", ch );
	return FALSE;
    }

    if ( !( pAf = pObj->affected ) )
    {
	send_to_char( "OEdit:  Non-existant affect.\r\n", ch );
	return FALSE;
    }

    if( value == 0 )	/* First case: Remove first affect */
    {
	pAf = pObj->affected;
	pObj->affected = pAf->next;
	free_affect( pAf );
    }
    else		/* Affect to remove is not the first */
    {
	while ( ( pAf_next = pAf->next ) && ( ++cnt < value ) )
	     pAf = pAf_next;

	if( pAf_next )		/* See if it's the next affect */
	{
	    pAf->next = pAf_next->next;
	    free_affect( pAf_next );
	}
	else                                 /* Doesn't exist */
	{
	     send_to_char( "No such affect.\r\n", ch );
	     return FALSE;
	}
    }

    send_to_char( "Affect removed.\r\n", ch);
    return TRUE;
}

void rebase_object(OBJ_INDEX_DATA *pObj, int nvnum, AREA_DATA *new_area ) { 

    OBJ_INDEX_DATA *prev_obj, *obj;
    ROOM_INDEX_DATA *room;
    RESET_DATA *reset;

    int iHash, ovnum, i;

    ovnum = pObj->vnum;
 
    pObj->vnum = nvnum;
    pObj->area = new_area;

   /* Remove from the old index... */

    iHash = ovnum % MAX_KEY_HASH;

    if ( obj_index_hash[iHash] == pObj ) {
      obj_index_hash[iHash] = pObj->next;
    } else {
      prev_obj = obj_index_hash[iHash];

      while ( prev_obj != NULL
	   && prev_obj->next != NULL ) {

	if ( prev_obj->next == pObj ) {
	  prev_obj->next = pObj->next;
	}

	prev_obj = prev_obj->next;
      } 
    }

   /* Add to the new one... */ 

    iHash = nvnum % MAX_KEY_HASH;

    pObj->next = obj_index_hash[iHash];
    obj_index_hash[iHash] = pObj;

   /* Now we have to go and fix ALL references to the object... */

    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++ ) {

     /* For rooms... */

      room = room_index_hash[iHash];

      while ( room != NULL ) { 

       /* Fix the rooms resets... */
 
	reset = room->reset_first;

	while ( reset != NULL ) {

	  switch ( reset->command ) {

	    case 'O':
	    case 'G':
	    case 'E':

	      if ( reset->arg1 == ovnum ) {
		reset->arg1 = nvnum;
	      }

	      break;

	    case 'P':

	      if ( reset->arg1 == ovnum ) {
		reset->arg1 = nvnum;
	      }

	      if ( reset->arg3 == ovnum ) {
		reset->arg3 = nvnum;
	      }

	      break;

	    default:
	      break;
	  }

	  reset = reset->next;
	} 
 
       /* Fix keys on room exits... */

        for (i = 0; i < DIR_MAX; i++ ) {

          if ( room->exit[i] != NULL
            && room->exit[i]->key == ovnum ) {
            room->exit[i]->key = nvnum;
          }
        }
 
	room = room->next; 
      } 
 
     /* For objects... */

      obj = obj_index_hash[iHash];

      while ( obj != NULL ) { 

        switch (obj->item_type) {

         /* For portals, fix companion portal... */

          case ITEM_PORTAL:
            if ( obj->value[1] == ovnum ) {
              obj->value[1] = nvnum;
            }
            break;

         /* For containers and keyrings, fix the key to open them... */ 

          case ITEM_CONTAINER:
          case ITEM_KEY_RING:
            if ( obj->value[2] == ovnum ) {
              obj->value[2] = nvnum;
            }
            break;

         /* Hopefully everything else will be ok... */
 
          default:
            break;
        }

        obj = obj->next; 
      }
    }

    return;
}

OEDIT( oedit_revnum ) {
OBJ_INDEX_DATA *pObj;
AREA_DATA *new_area, *old_area;
VNUM nvnum;

    EDIT_OBJ(ch, pObj);

   /* Are they allowed to do this? */

    if ( !IS_IMP(ch) ) {
      send_to_char( "Only an implementor may do this...\r\n", ch );
      return FALSE;
    }

   /* Must have a parameter... */

    if ( argument[0] == '\0' ) {
      send_to_char( "Syntax:  revnum [new_vnum]\r\n", ch );
      return FALSE;
    }

   /* Get and check the new vnum... */

    nvnum = atoi(argument);
 
    if ( nvnum < 1 ) {
      send_to_char( "Invalid new vnum!\r\n", ch );
      return FALSE;
    }

    if ( get_obj_index(nvnum) != NULL ) {
      send_to_char( "There is already an object at that vnum!\r\n", ch );
      return FALSE;
    }

   /* Now check the areas... */

    old_area = pObj->area;

    new_area = get_vnum_area( nvnum );

    if ( new_area == NULL ) {
      send_to_char( "New vnum must be within an existing area!\r\n", ch );
      return FALSE;
    }

    if ( IS_SET(old_area->area_flags, AREA_EDLOCK) ) {
      send_to_char( "This area is edit locked!\r\n", ch );
      return FALSE;
    } 

    if ( IS_SET(new_area->area_flags, AREA_EDLOCK) ) {
      send_to_char( "The new area is edit locked!\r\n", ch );
      return FALSE;
    } 

   /* Ok, probably safe to go ahead with the renum... */

    rebase_object( pObj, nvnum, new_area);

   /* Ok, maybe done... */

    send_to_char("{YObject vnum changed...{x\r\n", ch);
    send_to_char("{YASAVE WORLD to complete...{x\r\n", ch);

    return TRUE;
}



OEDIT( oedit_name )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  name [string]\r\n", ch );
	return FALSE;
    }

    free_string( pObj->name );
    pObj->name = str_dup( argument );

    send_to_char( "Name set.\r\n", ch);
    return TRUE;
}

OEDIT( oedit_repop)
	{
	OBJ_INDEX_DATA *pObj;
	int value;
	
	EDIT_OBJ(ch, pObj);
	
	if (argument[0] == '\0' )
		{
		send_to_char( "Syntax:  repop [percentage]\r\n", ch);
		return FALSE;
		}
	value = atoi (argument);
	if (value < 1 || value > 100)
		{	
		send_to_char( "Repop range is 1-100.\r\n", ch);
		return FALSE;
		}
	pObj->repop = value;
	send_to_char ("Repop percentage set.\r\n", ch);
	return TRUE;
	}	
	
OEDIT( oedit_short )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  short [string]\r\n", ch );
	return FALSE;
    }

    free_string( pObj->short_descr );
    pObj->short_descr = str_dup( argument );

    send_to_char( "Short description set.\r\n", ch);
    return TRUE;
}

OEDIT( oedit_image )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  image [image url]\r\n", ch );
	return FALSE;
    }

    free_string( pObj->image );
    pObj->image = str_dup( argument );

    send_to_char( "Image URL set\r\n", ch);
    return TRUE;
}

OEDIT( oedit_long )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  long [string]\r\n", ch );
	return FALSE;
    }
        
    free_string( pObj->description );
    pObj->description = str_dup( argument );
    pObj->description[0] = UPPER( pObj->description[0] );

    send_to_char( "Long description set.\r\n", ch);
    return TRUE;
}



bool set_value( CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, char *argument, int value )
{
    if ( argument[0] == '\0' )
    {
	set_obj_values( ch, pObj, -1, "" );     /* '\0' changed to "" -- Hugin */
	return FALSE;
    }

    if ( set_obj_values( ch, pObj, value, argument ) )
	return TRUE;

    return FALSE;
}



/*****************************************************************************
 Name:		oedit_values
 Purpose:	Finds the object and sets its value.
 Called by:	The four valueX functions below. (now five -- Hugin )
 ****************************************************************************/
bool oedit_values( CHAR_DATA *ch, char *argument, int value )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( set_value( ch, pObj, argument, value ) )
        return TRUE;

    return FALSE;
}


OEDIT( oedit_value0 )
{
    if ( oedit_values( ch, argument, 0 ) )
        return TRUE;

    return FALSE;
}



OEDIT( oedit_value1 )
{
    if ( oedit_values( ch, argument, 1 ) )
        return TRUE;

    return FALSE;
}



OEDIT( oedit_value2 )
{
    if ( oedit_values( ch, argument, 2 ) )
        return TRUE;

    return FALSE;
}



OEDIT( oedit_value3 )
{
    if ( oedit_values( ch, argument, 3 ) )
        return TRUE;

    return FALSE;
}



OEDIT( oedit_value4 )
{
    if ( oedit_values( ch, argument, 4 ) )
        return TRUE;

    return FALSE;
}



OEDIT( oedit_weight )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  weight [number]\r\n", ch );
	return FALSE;
    }

    pObj->weight = atoi( argument );

    send_to_char( "Weight set.\r\n", ch);
    return TRUE;
}



OEDIT( oedit_cost )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  cost [number]\r\n", ch );
	return FALSE;
    }

    pObj->cost = atoi( argument );

    send_to_char( "Cost set.\r\n", ch);
    return TRUE;
}



OEDIT( oedit_create ) {
OBJ_INDEX_DATA *pObj;
AREA_DATA *pArea;
VNUM  value;
int  iHash;

    value = atol(argument );
    if ( argument[0] == '\0' || value == 0 )    {
	send_to_char( "Syntax:  oedit create [vnum]\r\n", ch );
	return FALSE;
    }

    pArea = get_vnum_area( value );
    if ( !pArea )   {
	send_to_char( "OEdit:  That vnum is not assigned an area.\r\n", ch );
	return FALSE;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char( "OEdit:  Vnum in an area you cannot build in.\r\n", ch );
	return FALSE;
    }

    if ( get_obj_index( value ) )
    {
	send_to_char( "OEdit:  Object vnum already exists.\r\n", ch );
	return FALSE;
    }
       
    if ( IS_SET(pArea->area_flags, AREA_EDLOCK) ) {
        send_to_char("OEdit:  Objects area is Edit Locked!", ch);
        return FALSE;
    }

    pObj			= new_obj_index();

    pObj->vnum			= value;
    pObj->area			= pArea;
    pObj->repop			= 100;
       
    if ( value > top_vnum_obj )
	top_vnum_obj = value;

    iHash			= value % MAX_KEY_HASH;
    pObj->next			= obj_index_hash[iHash];
    obj_index_hash[iHash]	= pObj;
    ch->desc->pEdit		= (void *)pObj;

    send_to_char( "Object Created.\r\n", ch );
    return TRUE;
}


OEDIT( oedit_default )
{
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *paf;

    int level;
    int number, type;  /* for dice-conversion */
    int countaff = 0;
    int countspell = 0;

    EDIT_OBJ(ch, pObj);

   /* Check object level has been set... */

    pObj->level    = UMAX( 0, pObj->level ); /* just to be sure */

    level = pObj->level;

    if ( level == 0 ) {
        send_to_char( "Set the object to desired level first.\r\n", ch );
        return FALSE;
    }

   /* Count affects and applie for cost... */

    for ( paf = pObj->affected; paf != NULL; paf = paf->next ) {

      if (!paf->bitvector ) {

	    switch ( paf->location ) {

                default:  
		    ++countaff;
            	    break; 
		case APPLY_SEX: /* these have no cost affect */
		case APPLY_NONE:
		case APPLY_LEVEL:
		case APPLY_CLASS:
		case APPLY_HEIGHT:
		case APPLY_WEIGHT:
		case APPLY_ALIGN:
		    break;
		case APPLY_STR: /* these have 2 counters per */
		case APPLY_DEX:
		case APPLY_WIS:
		case APPLY_INT:
		case APPLY_CON:
		case APPLY_HITROLL:
		case APPLY_DAMROLL:
		case APPLY_SANITY:
		case APPLY_SKILL:
		    countaff += paf->modifier*2; /* 2 points per */
		    break;
		case APPLY_MANA:
		case APPLY_HIT:
		case APPLY_MOVE:
		case APPLY_AC:
		case APPLY_GOLD:
		case APPLY_EXP:
		case APPLY_MAGIC:
		    countaff += (paf->modifier * 1)/5; /* 1 point per 5 */
		    break;
		case APPLY_SAVING_PARA:
		case APPLY_SAVING_ROD:
		case APPLY_SAVING_PETRI:
		case APPLY_SAVING_BREATH:
		case APPLY_SAVING_SPELL:
		    ++countaff;
		    break;
	     }
        } else {
	    ++countspell; 
	}
    }

    pObj->cost = ( (50*level) + (400*countaff) + (5000*countspell) );

   /* Set object specific values... */

    switch ( pObj->item_type ) {
        default:
            bug( "Obj_default: bad type (%d).", pObj->item_type );
            break;

        case ITEM_LIGHT:
        case ITEM_LIGHT_REFILL:
        case ITEM_TREASURE:
        case ITEM_TRASH:
        case ITEM_CONTAINER:
        case ITEM_KEY_RING:
        case ITEM_DRINK_CON:
        case ITEM_KEY:
        case ITEM_FOOD:
        case ITEM_BOAT:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
        case ITEM_FOUNTAIN:
        case ITEM_POOL:
        case ITEM_MAP:
        case ITEM_CLOTHING:
        case ITEM_SCROLL:
        case ITEM_WAND:
        case ITEM_STAFF:
        case ITEM_LOCKER_KEY:
        case ITEM_BOOK:
        case ITEM_IDOL:
        case ITEM_SCRY:
        case ITEM_DIGGING:
        case ITEM_FORGING:
        case ITEM_PAPER:
        case ITEM_AMMO:
        case ITEM_SLOTMACHINE:
        case ITEM_HERB:
        case ITEM_PIPE:
        case ITEM_FURNITURE:
        case ITEM_DECORATION:
        case ITEM_TREE:
        case ITEM_WARP_STONE:
        case ITEM_ROOM_KEY:
        case ITEM_GEM:
        case ITEM_JEWELRY:
        case ITEM_JUKEBOX:
        case ITEM_SHARE:
        case ITEM_FIGURINE:
        case ITEM_GRIMOIRE:
        case ITEM_FOCUS:
        case ITEM_PORTAL:
        case ITEM_PROTOCOL:
        case ITEM_INSTRUMENT:
            break;


       case ITEM_LOCKER:         
       case ITEM_CLAN_LOCKER:         
            pObj->value[0]=-1;
            pObj->value[1] =   CONT_CLOSEABLE | CONT_CLOSED  | CONT_LOCKED    | CONT_PICKPROOF;
            break;

       case ITEM_CAMERA:         
            pObj->value[0]=-1;
            pObj->value[1] =   CONT_CLOSEABLE | CONT_CLOSED;
            break;

        case ITEM_RAW:
           pObj->value[0]=0;
           pObj->value[1]=0;
           break;

        case ITEM_EXPLOSIVE:
            number = 10 + (level/3);
            type   = 20 + (level/3);
            pObj->value[1] = number;
            pObj->value[2] = type;
            pObj->value[3] = 0;
            pObj->value[4] = 0;
            break;

        case ITEM_TRAP:
            number = (level/4)+1;
            type   = (level/3)+1;
            pObj->value[1] = number;
            pObj->value[2] = type;
            break;

        case ITEM_BONDAGE:
           pObj->value[0]=50+(level/4);
           break;

        case ITEM_PHOTOGRAPH:
           pObj->value[0] = 0 ;
           break;

        case ITEM_DOLL:
           pObj->value[0]=level/30;
           break;

        case ITEM_WEAPON:

           /* Same damage as for a mob of this level... */

            number = 1 + (level/8);
            type   = 5 + (level/6);

            pObj->value[1] = number;
            pObj->value[2] = type;
            break;

        case ITEM_ARMOR:
            pObj->value[0] = level / 8 + 3;
            pObj->value[1] = pObj->value[0];
            pObj->value[2] = pObj->value[0];
	    pObj->value[3] = pObj->value[0];
            break;

        case ITEM_POTION:
        case ITEM_PILL:
        case ITEM_MONEY:
            break;
    }

    send_to_char ( "Default Values Set.\r\n", ch );

    return TRUE;
}


OEDIT( oedit_ed ) {
    OBJ_INDEX_DATA *pObj;
    EXTRA_DESCR_DATA *ed;
    char command[MAX_INPUT_LENGTH];
    char keyword[MAX_INPUT_LENGTH];
    char *remkeys;
    int id;

    DEED *deed; 
    char *dtype; 

    char buf[MAX_STRING_LENGTH];

    EDIT_OBJ(ch, pObj);

    argument = one_argument( argument, command );
    remkeys = one_argument( argument, keyword );

    if (command[0] == '\0')
    {
        send_to_char( "Syntax:  ed list\r\n", ch);
        send_to_char( "         ed show [id]\r\n", ch);
	send_to_char( "         ed add [keywords]\r\n", ch );
        send_to_char( "         ed change [id] [keywords]\r\n",ch);
	send_to_char( "         ed edit [id]\r\n", ch );
	send_to_char( "         ed format [id]\r\n", ch );
        send_to_char( "         ed cond [id] [condition]\r\n", ch );
	send_to_char( "         ed delete [id]\r\n", ch );
        send_to_char( "         ed deed [id] [deed] [type] [title]", ch);
	return FALSE;
    }

   /* Apply sequential ids to all of the ed records... */

    id = 1;
    for(ed = pObj->extra_descr; ed != NULL; ed = ed->next) {
      ed->id = id++;
    } 

   /* ed add keywords */

    if ( !str_cmp( command, "add" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed add [keywords]\r\n", ch );
	    return FALSE;
	}

	ed			=   new_extra_descr();
	ed->keyword		=   str_dup( argument ); /* Mik */
	ed->description		=   str_dup( "" );
	ed->next		=   pObj->extra_descr;
	pObj->extra_descr	=   ed;

	string_append( ch, &ed->description );

	return TRUE;
    }

   /* ed change id keywords */

    if ( !str_cmp( command, "change" ) )
    {
	if ( keyword[0] == '\0' || remkeys[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed change [id] [keywords]\r\n", ch );
	    return FALSE;
	}

        id = atoi(keyword);

	for ( ed = pObj->extra_descr; ed; ed = ed->next ) {
	    if ( ed->id == id) {
		break;
            } 
	}

	if ( !ed )
	{
	    send_to_char( "No extra description exists with that id.\r\n", ch );
	    return FALSE;
	}

        ed->keyword = str_dup(remkeys); 	

	return TRUE;
    }

   /* ed edit keywords */

    if ( !str_cmp( command, "edit" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed edit [id]\r\n", ch );
	    return FALSE;
	}

        id = atoi(keyword);

	for ( ed = pObj->extra_descr; ed; ed = ed->next ) {
	    if ( ed->id == id) {
		break;
            } 
	}

	if ( !ed )
	{
	    send_to_char( "No extra description exists with that id.\r\n", ch );
	    return FALSE;
	}

	string_append( ch, &ed->description );

	return TRUE;
    }

   /* ed delete id */

    if ( !str_cmp( command, "delete" ) )
    {
	EXTRA_DESCR_DATA *ped = NULL;

	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed delete [id]\r\n", ch );
	    return FALSE;
	}

        id = atoi(keyword);

        ped = NULL;
	for ( ed = pObj->extra_descr; ed; ed = ed->next ) {
	    if ( ed->id == id) {
		break;
            } 
            ped = ed;
	}

	if ( !ed )
	{
	    send_to_char( "No extra description exists with that id.\r\n", ch );
	    return FALSE;
	}

	if ( ped == NULL )
	    pObj->extra_descr = ed->next;
	else
	    ped->next = ed->next;

	free_extra_descr( ed );

	send_to_char( "Extra description deleted.\r\n", ch );
	return TRUE;
    }

   /* ed format keywords */

    if ( !str_cmp( command, "format" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed format [keyword]\r\n", ch );
	    return FALSE;
	}

        id = atoi(keyword);

	for ( ed = pObj->extra_descr; ed; ed = ed->next ) {
	    if ( ed->id == id) {
		break;
            } 
	}

	if ( !ed )
	{
	    send_to_char( "No extra description exists with that id.\r\n", ch );
	    return FALSE;
	}

        if (format_text(ed->description, buf)) {
          free_string(ed->description);
          ed->description = str_dup(buf);
        }

	send_to_char( "Extra description formatted.\r\n", ch );
	return TRUE;
    }

   /* ed list */

    if ( !str_cmp( command, "list" ) ) {

      if (pObj->extra_descr == NULL) {
        send_to_char("There are no extra descriptions defined.\r\n", ch);
        return TRUE;
      }

      for(ed = pObj->extra_descr; ed != NULL; ed = ed->next) {

        sprintf(buf, " %3d) [%s]\r\n", ed->id, ed->keyword);

        send_to_char(buf, ch);

        if (ed->ec != NULL) { 
           print_econd(ch, ed->ec, "      Condition - "); 
        }

        deed = ed->deed;
       
        while (deed != NULL) {

          switch (deed->type) {
            case DEED_SECRET:
              dtype = "{rsecret {x";
              break;
            case DEED_PUBLIC:
              dtype = "{gpublic {x";
              break;
            default:
              dtype = "{yprivate{x";
          }    

          sprintf(buf, "Deed [%5d] [%s - %s]\r\n", deed->id, dtype, deed->title);
          send_to_char(buf, ch);

          deed = deed->next; 
        }
      }

      return TRUE;
    }

   /* ed show id */

    if ( !str_cmp( command, "show" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed show [id]\r\n", ch );
	    return FALSE;
	}

        id = atoi(keyword);

	for ( ed = pObj->extra_descr; ed; ed = ed->next ) {
	    if ( ed->id == id) {
		break;
            } 
	}

	if ( !ed )
	{
	    send_to_char( "No extra description exists with that id.\r\n", ch );
	    return FALSE;
	}

	sprintf(buf, "{WExtra description %d\r\nKeywords:{x [%s]\r\n%s",
                      id,
                      ed->keyword,
                      ed->description);
        send_to_char(buf, ch);

        if (ed->ec != NULL) { 
           send_to_char("{WConditions:{x\r\n", ch);
           print_econd(ch, ed->ec, "{WCondition -{x "); 
        }

        deed = ed->deed;

        if (deed != NULL) {
          send_to_char("{WDeeds:{x\r\n", ch);
        }
       
        while (deed != NULL) {

          switch (deed->type) {
            case DEED_SECRET:
              dtype = "{rsecret {x";
              break;
            case DEED_PUBLIC:
              dtype = "{gpublic {x";
              break;
            default:
              dtype = "{yprivate{x";
          }    

          sprintf(buf, "[%5d] [%s - %s]\r\n",
                       deed->id,
                       dtype,
                       deed->title);

          send_to_char(buf, ch);

          deed = deed->next; 
        }

	return TRUE;
    }

   /* ed cond id condition */

    if ( !str_cmp( command, "cond" ) ) {

      ECOND_DATA *new_ec;

     /* keyword must be supplied... */

      if ( keyword[0] == '\0' || keyword[0] == '?' ) {
        send_to_char( "Syntax:  ed cond [id] [condition]\r\n", ch );
	send_to_char( "         ed cond [id] ?\r\n", ch );
	return FALSE;
      }

     /* Check that the extra description exists first... */

      id = atoi(keyword);

      for ( ed = pObj->extra_descr; ed; ed = ed->next ) {
        if ( ed->id == id) {
	  break;
        } 
      }

      if ( !ed ) {
	send_to_char( "No extra description exists with that id.\r\n", ch );
	return FALSE;
      }

     /* None means we should remove all of the conditions... */
 
      if (!str_cmp(remkeys, "none")) {
        while (ed->ec != NULL) {
          new_ec = ed->ec->next;
          free_ec(ed->ec);
          ed->ec = new_ec;
        }
      
        send_to_char("Conditions removed.\r\n", ch);
    
        return TRUE;
      }
 
     /* ..otherwise we go make a new one... */

      new_ec = create_econd(remkeys, ch);
 
      if (new_ec != NULL) {
        new_ec->next = ed->ec;
        ed->ec = new_ec;
      }
   
      return TRUE;
    }

    if ( !str_cmp( command, "deed" ) ) {
    
        int deed_id;
        int deed_type;
        DEED *new_deed;

	if ( keyword[0] == '\0' || remkeys[0] == '\0' ) {
	    send_to_char("Syntax:  ed deed [id] [deed] [type] [title]\r\n", ch);
	    return FALSE;
	}

       /* Find the extra description... */ 

        id = atoi(keyword);

	for ( ed = pObj->extra_descr; ed; ed = ed->next ) {
	    if ( ed->id == id) {
		break;
            } 
	}

	if ( ed == NULL ) {
	    send_to_char( "No extra description exists with that id.\r\n", ch );
	    return FALSE;
	}

       /* Extract the deed id... */

        remkeys = one_argument(remkeys, keyword);

        deed_id = atoi(keyword);

       /* Extract the deed type... */

        remkeys = one_argument(remkeys, keyword);

        if (deed_id < 1) {
             long value;

             value = flag_value(discipline_type, remkeys);
             if ( value == NO_FLAG ) {
	    send_to_char( "Invalid discipline deed.\r\n", ch );
	    return FALSE;
             }
        }

        deed_type = 0;

        if (!str_cmp(keyword, "secret")) {
          deed_type = DEED_SECRET;
        } else if (!str_cmp(keyword, "public")) {
          deed_type = DEED_PUBLIC;
        } else if (!str_cmp(keyword, "private")) {
          deed_type = 0;
        } else {
          send_to_char("Deed type is secret, private or public.\r\n", ch);
          return FALSE;
        } 

       /* Check we have a title... */ 
        
	if ( remkeys[0] == '\0' ) {
	    send_to_char("Syntax:  ed deed [id] [deed] [type] [title]\r\n", ch);
	    return FALSE;
	}

       /* Deed adding time... */

        new_deed = NULL;
     
        deed = ed->deed;

        while (deed != NULL) {
          if (deed->id == deed_id) {
            new_deed = deed;
            break;
          }
          deed = deed->next;
        }

        if (new_deed == NULL) {  
          new_deed = get_deed();
          new_deed->id = deed_id;

          new_deed->next = ed->deed;
          ed->deed = new_deed;
        }

        new_deed->type = deed_type;
        new_deed->title = str_dup(remkeys);  
        
	send_to_char( "Extra description deed added/updated.\r\n", ch );
	return TRUE;
    }
 
   /* Nothing found, means give help... */

    oedit_ed( ch, "" );
    return FALSE;
}



OEDIT( oedit_extra )  {
OBJ_INDEX_DATA *pObj;
long long value;

    if ( argument[0] != '\0' )    {
	EDIT_OBJ(ch, pObj);

	if (( value = flag_value_long( extra_flags, argument )) != NO_FLAG ){
	    pObj->extra_flags ^= value;

	    send_to_char( "Extra flag toggled.\r\n", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax:  extra [flag]\r\n"
		  "Type '? extra' for a list of flags.\r\n", ch );
    return FALSE;
}


OEDIT( oedit_artifact )  {
OBJ_INDEX_DATA *pObj;
ARTIFACT *art;
char arg[MAX_STRING_LENGTH];
int value;

    EDIT_OBJ(ch, pObj);

    if (!IS_SET(pObj->extra_flags, ITEM_ARTIFACT)) return FALSE;

    if ( argument[0] == '\0' )  {
          send_to_char("Syntax:\r\n", ch);
          send_to_char("   ARTIFACT POWER <flags>\r\n", ch );
          send_to_char("   ARTIFACT ATTRACT <old one attraction %>\r\n", ch );
          send_to_char("   ARTIFACT PULSE <act on pulse %>\r\n", ch );
          return FALSE;
    }
    
    argument = one_argument(argument, arg);

    if (!str_cmp(arg, "power")) {
          if (!pObj->artifact_data) {
	art = new_artifact();
	art->next = pObj->artifact_data;
	pObj->artifact_data = art;
                art->pulse = -1;
          }

          if ((value = flag_value( artifact_flags, argument ) ) != NO_FLAG ) {
	TOGGLE_BIT(pObj->artifact_data->power, value);
                send_to_char( "Artifact flag toggled.\r\n", ch);
	return TRUE;
          }

    } else if (!str_cmp(arg, "attract")) {
          if ( argument[0] == '\0' || !is_number( argument ) )  {
                send_to_char("Syntax:\r\n", ch);
                send_to_char("   ARTIFACT ATTRACT <old one attraction %>\r\n", ch );
	return FALSE;
          }

          if (!pObj->artifact_data) {
	art = new_artifact();
	art->next = pObj->artifact_data;
	pObj->artifact_data = art;
                art->pulse = -1;
          }

          pObj->artifact_data->attract = atoi( argument );
          pObj->artifact_data->attract = URANGE(0, pObj->artifact_data->attract, 99);
          send_to_char( "Attract % set.\r\n", ch);

    } else if (!str_cmp(arg, "pulse")) {
          if ( argument[0] == '\0' || !is_number( argument ) )  {
                send_to_char("Syntax:\r\n", ch);
                send_to_char("   ARTIFACT PULSE <act on pulse %>\r\n", ch );
	return FALSE;
          }

          if (!pObj->artifact_data) {
	art = new_artifact();
	art->next = pObj->artifact_data;
	pObj->artifact_data = art;
                art->pulse = -1;
          }

          pObj->artifact_data->pulse = atoi( argument );
          pObj->artifact_data->pulse = URANGE(-1, pObj->artifact_data->pulse, 100);
          send_to_char( "Pulse % set.\r\n", ch);
    } else {
          send_to_char("Syntax:\r\n", ch);
          send_to_char("   ARTIFACT POWER <flags>\r\n", ch );
          send_to_char("   ARTIFACT ATTRACT <old one attraction %>\r\n", ch );
          send_to_char("   ARTIFACT PULSE <act on pulse %>\r\n", ch );
          return FALSE;
     }
     return TRUE;
}


OEDIT( oedit_zone )  {
    OBJ_INDEX_DATA *pObj;
    int value;

     if ( argument[0] != '\0' )    {
	EDIT_OBJ(ch, pObj);

	if ( ( value = flag_value( zmask_flags, argument ) ) != NO_FLAG ) {
	    TOGGLE_BIT(pObj->zmask, value);

	    send_to_char( "Zone setting changed.\r\n", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax:  zmask [zone]\r\n"
		  "Type 'zlist' for a list of zones.\r\n", ch );
    return FALSE;
}


OEDIT( oedit_wear ) {
OBJ_INDEX_DATA *pObj;
ECOND_DATA *new_ec;
char arg[MAX_INPUT_LENGTH];
int value;

    argument = one_argument(argument, arg);

    if (arg[0] != '\0' ) {
        EDIT_OBJ(ch, pObj);

        if (!str_cmp(arg, "cond")) {

            if (!str_cmp(argument, "none")) {
                while (pObj->wear_cond) {
                     new_ec = pObj->wear_cond->next;
                     free_ec(pObj->wear_cond);
                     pObj->wear_cond = new_ec;
                }
                send_to_char("Wear conditions removed.\r\n", ch);
                return TRUE;
            }
 
            new_ec = create_econd(argument, ch);
 
            if (new_ec != NULL) {
                new_ec->next = pObj->wear_cond;
                pObj->wear_cond = new_ec;
            }
            return TRUE;

        } else {
	value = flag_value(wear_flags, arg);

	if ( value != NO_FLAG ) {
	    TOGGLE_BIT(pObj->wear_flags, value);
	    send_to_char( "Wear flag toggled.\r\n", ch);

                    if ( IS_SET(pObj->wear_flags, ITEM_WEAR_PRIDE)
                    && pObj->item_type != ITEM_PRIDE ) {
                           send_to_char("{YWARNING: Wear pride and item not type PRIDE{x\r\n", ch);
                    }

                    if ( IS_SET(pObj->wear_flags, ITEM_WEAR_TATTOO)
                    && pObj->item_type != ITEM_TATTOO ) {
                           send_to_char("{YWARNING: Wear tattoo and item not type TATTOO{x\r\n", ch);
                    }

	    return TRUE;
	}
         }
    }

    send_to_char(  "Syntax:  wear cond [condition]\r\n"
                                "Syntax:  wear [flag]\r\n"
		"Type '? wear' for a list of flags.\r\n", ch );
    return FALSE;
}


OEDIT( oedit_type )      /* Moved out of oedit() due to naming conflicts -- Hugin */
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_OBJ(ch, pObj);

	if ( ( value = flag_value( type_flags, argument ) ) != NO_FLAG )
	{
	    pObj->item_type = value;

	    send_to_char( "Type set.\r\n", ch);
 
            switch (pObj->item_type) {

              case ITEM_LOCKER:
              case ITEM_CLAN_LOCKER:
                pObj->value[0] = -1;
                pObj->value[1] =   CONT_CLOSEABLE | CONT_CLOSED
                                 | CONT_LOCKED    | CONT_PICKPROOF;
                pObj->value[2] = 0;
                pObj->value[3] = 0;
                pObj->value[4] = 0;
                break;
 
              case ITEM_CAMERA:
                pObj->value[0] = -1;
                pObj->value[1] =   CONT_CLOSEABLE | CONT_CLOSED;
                pObj->value[2] = 0;
                pObj->value[3] = 0;
                pObj->value[4] = 0;
                break;

              default:
	        pObj->value[0] = 0;
	        pObj->value[1] = 0;
	        pObj->value[2] = 0;
	        pObj->value[3] = 0;
	        pObj->value[4] = 0; 
                break;
            }

	    return TRUE;
	}
    }

    send_to_char( "Syntax:  type [flag]\r\n"
		  "Type '? type' for a list of flags.\r\n", ch );
    return FALSE;
}



OEDIT( oedit_material )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_OBJ(ch, pObj);

	if ( ( value = flag_value( material_type, argument ) ) != NO_FLAG )
	{
	    pObj->material = value;
	    send_to_char( "Material type set.\r\n", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax:  material [material-name]\r\n"
		  "Type '? material' for a list of materials.\r\n", ch );
    return FALSE;
}



OEDIT( oedit_level )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  level [number]\r\n", ch );
	return FALSE;
    }

    pObj->level = atoi( argument );

    send_to_char( "Level set.\r\n", ch);
    return TRUE;
}



OEDIT( oedit_condition )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] != '\0'
    && ( value = atoi (argument ) ) >= 0
    && ( value <= 100 ) )
    {
	EDIT_OBJ( ch, pObj );

	pObj->condition = value;
	send_to_char( "Condition set.\r\n", ch );

	return TRUE;
    }

    send_to_char( "Syntax:  condition [number]\r\n"
		  "Where number can range from 0 (ruined) to 100 (perfect).\r\n",
		  ch );
    return FALSE;
}





/*
 * Mobile Editor Functions.
 */
MEDIT( medit_unused )
{
	ROOM_INDEX_DATA *pRoom;
	AREA_DATA *pArea;
	MOB_INDEX_DATA *tmp;
	char outbuf[MAX_STRING_LENGTH];
	char buf[128];
	int counter;
        int fcount; 

	EDIT_ROOM(ch, pRoom);
	pArea=pRoom->area;
	sprintf (outbuf, "Area name: %s\r\n",((pArea->name) ? (pArea->name) : "none"));
	strcat (outbuf, "UNUSED MOBILE VNUMS\r\n-------------------\r\n");

	counter = pArea->lvnum;
        fcount = 0;

	for (; (counter <= pArea->uvnum); counter++) {
	  if ((tmp = get_mob_index(counter)) == NULL) {
	    sprintf (buf, " %5d,", counter);
	    strcat (outbuf, buf);
	  }

          if (++fcount % 10 == 0) {
            strcat(outbuf, "\r\n");
	  }
	}
	
        if (fcount % 10 != 0) {
          strcat(outbuf, "\r\n");
        }

	page_to_char (outbuf, ch);

	return TRUE;
}

MEDIT( medit_used ) {
	ROOM_INDEX_DATA *pRoom;
	AREA_DATA *pArea;
	MOB_INDEX_DATA *tmp;
	char outbuf[5*MAX_STRING_LENGTH];
	char buf[128];
	int counter; 

	EDIT_ROOM(ch, pRoom);
	pArea=pRoom->area;
	sprintf (outbuf, "Area name: %s\r\n",((pArea->name) ? (pArea->name) : "none"));
	strcat  (outbuf, "USED MOB VNUMS\r\n---- --- -----\r\n");
	counter = pArea->lvnum;
	for (; (counter <= pArea->uvnum); counter++){
		if ((tmp = get_mob_index(counter)) != NULL) 	{
			sprintf (buf, "[%d] %s [level %d]\r\n", counter, ((tmp->short_descr) ? (tmp->short_descr) : ""), tmp->level);
			strcat (outbuf, buf);
			}
		}
	
	page_to_char (outbuf, ch);
	return TRUE;
}


MEDIT( medit_show ) {
MOB_INDEX_DATA *pMob;
SOCIETY *society = NULL;
char buf[MAX_STRING_LENGTH];
char vb[MAX_STRING_LENGTH];

    EDIT_MOB(ch, pMob);

    sprintf_to_char(ch, "Name: [{c%s{x]  Area: [{c%3d{x] %s\r\n", pMob->player_name, !pMob->area ? -1 : pMob->area->vnum, !pMob->area ? "No Area" : pMob->area->name );
    if (pMob->spec_fun > 0) {
        if (!str_cmp("spec_clanguard", spec_string(pMob->spec_fun))
        || !str_cmp("spec_clanhealer", spec_string(pMob->spec_fun))
        || !str_cmp("spec_clanmember", spec_string(pMob->spec_fun))) {
             if (pMob->group > 0) society = find_society_by_id(pMob->group);
             if (society) {
                 sprintf_to_char(ch, "Society: [{c%s{x]\r\n", society->name);
             } else {
                 send_to_char("Society: [{cnone{x]\r\n", ch);
                 pMob->group = 0;
             }
        }
    }
    sprintf_to_char(ch, "Act: [{c%s{x]\r\n", flag_string( act_flags, pMob->act ));

    format_vnum(pMob->vnum, vb);
    sprintf_to_char(ch, "Vnum: [{c%11s{x]  Sex: [{c%s{x]  Race: [{c%s{x]\r\n", vb, pMob->sex == SEX_MALE    ? "male" : pMob->sex == SEX_FEMALE  ? "female" : pMob->sex == 3  ? "random" : "neutral", race_array[pMob->race].name );
    sprintf_to_char(ch, "Level: [{c%2d{x]  Align: [{c%4d{x]  "
                                       "Hitroll: [{c%d{x]  Fright: [{c%d{x]\r\n", pMob->level, pMob->alignment, pMob->hitroll, pMob->fright );
    sprintf_to_char(ch, "Nature: [{c%s{x]\r\n", flag_string( mob_nature, pMob->nature ));
    sprintf_to_char(ch, "Affected by: [{c%s{x]\r\n", flag_string( affect_flags, pMob->affected_by ));
    sprintf_to_char(ch, "Form: [{c%s{x]\r\n", flag_string( form_flags, pMob->form ));
    sprintf_to_char(ch, "Parts: [{c%s{x]\r\n", flag_string( part_flags, pMob->parts ));
    sprintf_to_char(ch, "Imm: [{c%s{x]\r\n", flag_string( imm_flags, pMob->imm_flags ));
    sprintf_to_char(ch, "EImm:[{c%s{x]\r\n", flag_string(imm_flags, pMob->envimm_flags ));
    sprintf_to_char(ch, "Res: [{c%s{x]\r\n", flag_string( res_flags, pMob->res_flags ));
    sprintf_to_char(ch, "Vuln:[{c%s{x]\r\n", flag_string( vuln_flags, pMob->vuln_flags ));
    sprintf_to_char(ch, "Off: [{c%s{x]\r\n", flag_string( off_flags,  pMob->off_flags ));
    sprintf_to_char(ch, "Atk Type: [{c%s{x]  ", flag_string( attack_type,  pMob->atk_type ));
    sprintf_to_char(ch, "Dam Type: [{c%s{x]  ", flag_string( damage_type,  pMob->dam_type ));
    sprintf_to_char(ch, "Size: [{c%s{x]  Gold: [{c%5ld{x]\r\n", flag_string( size_flags, pMob->size ), pMob->gold);
    sprintf_to_char(ch, "Start pos: [{c%s{x]  ", flag_string( position_flags, pMob->start_pos ));
    sprintf_to_char(ch, "Default pos: [{c%s{x]\r\n", flag_string( position_flags, pMob->default_pos ));
    if (pMob->skills == NULL) {
      sprintf_to_char(ch, "Skills: [{yUnskilled{x]  expected: [{cNone{x]  Language: [{c%s{x]\r\n", pMob->language <= 0 ? "default" : skill_array[pMob->language].name);
    } else {
      sprintf_to_char(ch, "Skills: [{c%5d{x]  expected: [{c%5d{x]  Language: [{c%s{x]\r\n", pMob->skills->total_points, pMob->level * 25 + 200, pMob->language <= 0 ? "default" : skill_array[pMob->language].name);
    } 

    if (pMob->time_wev != 0) {
      sprintf_to_char(ch, "Time_subs: [{c%s{x]\r\n", flag_string( time_sub, pMob->time_wev ) );
    } 

    if ( pMob->spec_fun != NULL) {
	sprintf_to_char(ch, "Spec fun: [{c%s{x]\r\n", spec_string( pMob->spec_fun ) );
    }

   /* Summarize the extra stuff... */

    strcpy(buf, "Smarts: [{x");

    if (pMob->cd != NULL) {
      strcat(buf, "Conversations ");
    }   

    if (pMob->triggers->scripts != NULL) {
      strcat(buf, "Scripts ");
    } 

    if (pMob->triggers->challange != NULL) {
      strcat(buf, "cTriggers ");
    } 

    if (pMob->triggers->reaction != NULL) {
      strcat(buf, "rTriggers ");
    } 

    if (pMob->monitors != NULL) {
      strcat(buf, "Monitors ");
    }

    if (pMob->triggers->chat != NULL) {
      strcat(buf, "Chats ");
    } 

    if ( pMob->can_see != NULL
      || pMob->can_not_see != NULL ) {
      strcat(buf, "Vision "); 
    }

    if (buf[11] == '\0') { 
      strcat(buf, "None ");
    }

    strcat(buf, "{x]\r\n");

    send_to_char(buf, ch);

   /* Descriptive time... */

    if ( pMob->image != NULL ) {
      sprintf_to_char(ch, "Image URL   : {c%s{x\r\n", pMob->image);
    }

    sprintf_to_char(ch, "Short descr: {c%s{x\r\nLong descr:\r\n{c%s{x", pMob->short_descr, pMob->long_descr );
    sprintf_to_char(ch, "Description:\r\n{c%s{x", pMob->description );

    if ( pMob->pShop ) {
	SHOP_DATA *pShop;
	int iTrade;
                char haggle_string[MAX_INPUT_LENGTH];

	pShop = pMob->pShop;

                format_vnum(pMob->vnum, vb);
	sprintf_to_char(ch, "Shop data for [{C%11s{w] (%s):\r\n"
	                                   "  Markup for purchaser: {c%d{x%%\r\n"
	                                   "  Markdown for seller:  {c%d{x%%\r\n", vb, pShop->currency != -1 ? flag_string(currency_type, pShop->currency) : "default currency", pShop->profit_buy, pShop->profit_sell );
	sprintf_to_char(ch, "  Hours: {c%d{x to {c%d{x.\r\n", pShop->open_hour, pShop->close_hour );

                if (pShop->haggle < 10) sprintf(haggle_string, "{Rforbidden{x");
                else if (pShop->haggle < 30) sprintf(haggle_string, "{rfrowned upon{x");
                else if (pShop->haggle < 70) sprintf(haggle_string, "{ytolerated{x");
                else if (pShop->haggle < 90) sprintf(haggle_string, "{gaccepted{x");
                else sprintf(haggle_string, "{Grequired{x");

	sprintf_to_char(ch, "  Haggle: {c%d - %s{x\r\n", pShop->haggle, haggle_string);


	for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ ) {
	    if ( pShop->buy_type[iTrade] != 0 )	    {
		if ( iTrade == 0 ) {
		    send_to_char( "  Number Trades Type\r\n", ch );
		    send_to_char( "  ------ -----------\r\n", ch );
		}
                                 if (pShop->buy_type[iTrade] == 999) sprintf(buf, "  [%4d] Bank\r\n", iTrade);
                                 else if (pShop->buy_type[iTrade] == 998) sprintf(buf, "  [%4d] Hotel\r\n", iTrade);
                                 else if (pShop->buy_type[iTrade] == 997) sprintf(buf, "  [%4d] Stock Exchange\r\n", iTrade);
                                 else if (pShop->buy_type[iTrade] == 996) sprintf(buf, "  [%4d] Material Exchange\r\n", iTrade);
                                 else if (pShop->buy_type[iTrade] == 995) sprintf(buf, "  [%4d] Money Exchange\r\n", iTrade);
                                 else if (pShop->buy_type[iTrade] == 994) sprintf(buf, "  [%4d] Magistrate\r\n", iTrade);
		 else sprintf(buf, "  [%4d] %s\r\n", iTrade, flag_string( type_flags, pShop->buy_type[iTrade] ) );
		send_to_char( buf, ch );
	    }
	}
    }

    return FALSE;
}



MEDIT( medit_create ) {
MOB_INDEX_DATA *pMob;
AREA_DATA *pArea;
VNUM  value;
int  iHash;

    value = atol( argument );
    if ( argument[0] == '\0' || value == 0 )   {
	send_to_char( "Syntax:  medit create [vnum]\r\n", ch );
	return FALSE;
    }

    pArea = get_vnum_area( value );

    if ( !pArea )    {
	send_to_char( "MEdit:  That vnum is not assigned an area.\r\n", ch );
	return FALSE;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char( "MEdit:  Vnum in an area you cannot build in.\r\n", ch );
	return FALSE;
    }

    if ( get_mob_index( value ) )
    {
	send_to_char( "MEdit:  Mobile vnum already exists.\r\n", ch );
	return FALSE;
    }

    if ( IS_SET(pArea->area_flags, AREA_EDLOCK) ) {
        send_to_char("MEdit:  Mobs area is Edit Locked!", ch);
        return FALSE;
    }

    pMob			= new_mob_index();
    pMob->vnum			= value;
    pMob->area			= pArea;
        
    if ( value > top_vnum_mob )
	top_vnum_mob = value;        

    pMob->act			= ACT_IS_NPC;
    iHash			= value % MAX_KEY_HASH;
    pMob->next			= mob_index_hash[iHash];
    mob_index_hash[iHash]	= pMob;
    ch->desc->pEdit		= (void *)pMob;

    send_to_char( "Mobile Created.\r\n", ch );
    return TRUE;
}

/* iffy test function to set mobile default values.*/

MEDIT( medit_default ) {
    int i;
    int type, number, bonus;
    int level;
    MOB_INDEX_DATA *pMob;

   EDIT_MOB(ch, pMob);

   if (pMob->level == 0 ) {
		send_to_char( "Set the mobile to desired level first.\r\n", ch );
     		return FALSE;
	}
    
    level = pMob->level;

    /*
     * Calculate hit dice.  Gives close to the hitpoints
     * of old format mobs created with create_mobile()  (db.c)
     * A high number of dice makes for less variance in mobiles
     * hitpoints.
     * (might be a good idea to reduce the max number of dice)
     *
     * The conversion below gives:

       level:     dice         min         max        diff       mean
         1:       1d2+6       7(  7)     8(   8)     1(   1)     8(   8)
         2:       1d3+15     16( 15)    18(  18)     2(   3)    17(  17)
         3:       1d6+24     25( 24)    30(  30)     5(   6)    27(  27)
         5:      1d17+42     43( 42)    59(  59)    16(  17)    51(  51)
        10:      3d22+96     99( 95)   162( 162)    63(  67)   131(    )
        15:     5d30+161    166(159)   311( 311)   145( 150)   239(    )
        30:    10d61+416    426(419)  1026(1026)   600( 607)   726(    )
        50:    10d169+920   930(923)  2610(2610)  1680(1688)  1770(    )

        The values in parenthesis give the values generated in create_mobile.
        Diff = max - min.  Mean is the arithmetic mean.
        (hmm.. must be some roundoff error in my calculations.. smurfette got
         1d6+23 hp at level 3 ? -- anyway.. the values above should be
         approximately right..)

     */

    type   = level*level*27/40;
    number = UMIN(type/40 + 1, 10); /* how do they get 11 ??? */
    type   = UMAX(2, type/number);
    bonus  = UMAX(0, ((level*(8 + level)) * 9)/10 - number*type);
    number = (number * 17)/10;

    pMob->hit[DICE_NUMBER]    = number;
    pMob->hit[DICE_TYPE]      = type;
    pMob->hit[DICE_BONUS]     = bonus;


    pMob->mana[DICE_NUMBER]   = level;
    pMob->mana[DICE_TYPE]     = 10;
    pMob->mana[DICE_BONUS]    = 100;

    /*
     * Calculate dam dice.  Gives close to the damage
     * of old format mobs in damage()  (fight.c)
     */
/*    type   = level*7/4;
    number = UMIN(type/4 + 1, 5);
    type   = UMAX(2, type/number);

*/ /* elfren - old mob damage amounts adjusted */

    number = UMIN(level/4 + 1, 5);
    type   = (level + 7)/number;
    bonus  = UMAX(0, level*9/4 - number*type);

    pMob->damage[DICE_NUMBER] = number;
    pMob->damage[DICE_TYPE]   = type;
    pMob->damage[DICE_BONUS]  = bonus;

    switch ( number_range( 1, 3 ) )
    {
        case (1):
            pMob->atk_type = WDT_PUNCH;
            pMob->dam_type = DAM_BASH;
            break; 

        case (2):
            pMob->atk_type = WDT_PIERCE;
            pMob->dam_type = DAM_PIERCE;
            break; 

        case (3):
            pMob->atk_type = WDT_SLASH;
            pMob->dam_type = DAM_SLASH;
            break; 
    }

    for (i = 0; i < 3; i++)
        pMob->ac[i]         = interpolate( level, 100, -100);
    pMob->ac[3]             = interpolate( level, 100, 0);    /* exotic */

    pMob->gold             /= 100;

    pMob->nature = 0;

   send_to_char( "Default Values Set.\r\n", ch );
   return TRUE;
}

MEDIT( medit_spec )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  spec [special function]\r\n", ch );
	return FALSE;
    }


    if ( !str_cmp( argument, "none" ) )
    {
        pMob->spec_fun = NULL;

        send_to_char( "Spec removed.\r\n", ch);
        return TRUE;
    }

    if ( spec_lookup( argument ) )
    {
	pMob->spec_fun = spec_lookup( argument );
	send_to_char( "Spec set.\r\n", ch);
	return TRUE;
    }

    send_to_char( "MEdit: No such special function.\r\n", ch );
    return FALSE;
}



MEDIT( medit_align )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  alignment [number]\r\n", ch );
	return FALSE;
    }

    pMob->alignment = atoi( argument );

    send_to_char( "Alignment set.\r\n", ch);
    return TRUE;
}

MEDIT( medit_level ) {
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ))    {
	send_to_char( "Syntax:  level [number]\r\n", ch );
	return FALSE;
    }

    pMob->level = atoi( argument );
    pMob->hitroll = UMAX(0, (pMob->level - 40) * 2);
    
    send_to_char( "Level set.\r\n", ch);
    return TRUE;
}


MEDIT( medit_society) {
MOB_INDEX_DATA *pMob;
SOCIETY *society;
int soc;

    EDIT_MOB(ch, pMob);

    if (str_cmp("spec_clanguard", spec_string(pMob->spec_fun))
    && str_cmp("spec_clanhealer", spec_string(pMob->spec_fun))
    && str_cmp("spec_clanmember", spec_string(pMob->spec_fun))) {
	send_to_char( "Mob doesn't need a membership\r\n", ch );
	return FALSE;
    }
               
    if ( argument[0] == '\0' || !is_number( argument ))    {
	send_to_char( "Syntax:  society [number]\r\n", ch );
	return FALSE;
    }

    soc = atoi(argument );
    society = find_society_by_id(soc);

    if (society == NULL) {
        sprintf_to_char(ch, "Invalid society id!\r\n");
        return TRUE;
    } 

    pMob->group = soc;
    send_to_char( "Society set.\r\n", ch);
    return TRUE;
}


MEDIT( medit_bio )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	string_append( ch, &pMob->bio );
	return TRUE;
    }

    send_to_char( "Syntax:  bio    - line edit\r\n", ch );
    return FALSE;
}


MEDIT( medit_desc )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	string_append( ch, &pMob->description );
	return TRUE;
    }

    send_to_char( "Syntax:  desc    - line edit\r\n", ch );
    return FALSE;
}




MEDIT( medit_long )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  long [string]\r\n", ch );
	return FALSE;
    }

    free_string( pMob->long_descr );
    strcat( argument, "\r\n" );
    pMob->long_descr = str_dup( argument );
    pMob->long_descr[0] = UPPER( pMob->long_descr[0]  );

    send_to_char( "Long description set.\r\n", ch);
    return TRUE;
}



MEDIT( medit_short )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  short [string]\r\n", ch );
	return FALSE;
    }

    free_string( pMob->short_descr );
    pMob->short_descr = str_dup( argument );

    send_to_char( "Short description set.\r\n", ch);
    return TRUE;
}

MEDIT( medit_image )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  image [image_url]\r\n", ch );
	return FALSE;
    }

    free_string( pMob->image );
    pMob->image = str_dup( argument );

    send_to_char( "Image URL set.\r\n", ch);
    return TRUE;
}

void rebase_mob(MOB_INDEX_DATA *pMob, int nvnum, AREA_DATA *new_area ) {

    MOB_INDEX_DATA *prev_mob;
    ROOM_INDEX_DATA *room;
    OBJ_INDEX_DATA *obj;
    RESET_DATA *reset;

    int iHash, ovnum;

    ovnum = pMob->vnum;
 
    pMob->vnum = nvnum;
    pMob->area = new_area;

   /* Remove from the old index... */

    iHash = ovnum % MAX_KEY_HASH;

    if ( mob_index_hash[iHash] == pMob ) {
      mob_index_hash[iHash] = pMob->next;
    } else {
      prev_mob = mob_index_hash[iHash];

      while ( prev_mob != NULL
           && prev_mob->next != NULL ) {

        if ( prev_mob->next == pMob ) {
          prev_mob->next = pMob->next;
        }

        prev_mob = prev_mob->next;
      } 
    }

   /* Add to the new one... */ 

    iHash = nvnum % MAX_KEY_HASH;

    pMob->next = mob_index_hash[iHash];
    mob_index_hash[iHash] = pMob;

   /* Now we have to go and fix ALL references to the room... */

    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++ ) {

      room = room_index_hash[iHash];

      while ( room != NULL ) { 

       /* Fix the rooms resets... */
 
        reset = room->reset_first;

        while ( reset != NULL ) {

          switch ( reset->command ) {

            case 'M':
              if ( reset->arg1 == ovnum ) {
                reset->arg1 = nvnum;
              }
              break;

            default:
              break;
          }

          reset = reset->next;
        } 
 
        room = room->next; 
      } 
 
     /* Check objects... */

      obj = obj_index_hash[iHash];

      while ( obj != NULL ) { 

        switch (obj->item_type) {

         /* For NPC corpses we must check the originator... */

          case ITEM_CORPSE_NPC:

            if ( obj->value[0] == ovnum ) {
              obj->value[0] = nvnum;
            }

            break;
 
          default:
            break;
        }

        obj = obj->next; 
      }
    }

    return;
}

MEDIT( medit_revnum ) {
MOB_INDEX_DATA *pMob;
AREA_DATA *new_area, *old_area;
VNUM nvnum;

    EDIT_MOB(ch, pMob);

   /* Are they allowed to do this? */

    if ( !IS_IMP(ch) ) {
      send_to_char( "Only an implementor may do this...\r\n", ch );
      return FALSE;
    }

   /* Must have a parameter... */

    if ( argument[0] == '\0' ) {
      send_to_char( "Syntax:  revnum [new_vnum]\r\n", ch );
      return FALSE;
    }

   /* Get and check the new vnum... */

    nvnum = atol(argument);
 
    if ( nvnum < 1 ) {
      send_to_char( "Invalid new vnum!\r\n", ch );
      return FALSE;
    }

    if ( get_mob_index(nvnum) != NULL ) {
      send_to_char( "There is already a mobile at that vnum!\r\n", ch );
      return FALSE;
    }

   /* Now check the areas... */

    old_area = pMob->area;

    new_area = get_vnum_area( nvnum );

    if ( new_area == NULL ) {
      send_to_char( "New vnum must be within an existing area!\r\n", ch );
      return FALSE;
    }

    if ( IS_SET(old_area->area_flags, AREA_EDLOCK) ) {
      send_to_char( "This area is edit locked!\r\n", ch );
      return FALSE;
    } 

    if ( IS_SET(new_area->area_flags, AREA_EDLOCK) ) {
      send_to_char( "The new area is edit locked!\r\n", ch );
      return FALSE;
    } 

   /* Ok, probably safe to go ahead with the renum... */

    rebase_mob(pMob, nvnum, new_area);

   /* Ok, maybe done... */

    send_to_char("{YMob vnum changed...{x\r\n", ch);
    send_to_char("{YASAVE WORLD to complete...{x\r\n", ch);

    return TRUE;
}


MEDIT( medit_name )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  name [string]\r\n", ch );
	return FALSE;
    }

    free_string( pMob->player_name );
    pMob->player_name = str_dup( argument );

    send_to_char( "Name set.\r\n", ch);
    return TRUE;
}




MEDIT( medit_shop ) {
MOB_INDEX_DATA *pMob;
char command[MAX_INPUT_LENGTH];
char arg1[MAX_INPUT_LENGTH];

    argument = one_argument( argument, command );
    argument = one_argument( argument, arg1 );

    EDIT_MOB(ch, pMob);

    if ( command[0] == '\0' )    {
	send_to_char( "Syntax:  shop hours [#opening] [#closing]\r\n", ch );
	send_to_char( "         shop profit [#buying%] [#selling%]\r\n", ch );
	send_to_char( "         shop haggle [#tolerance%]\r\n", ch );
	send_to_char( "         shop type [#0-4] [item type]\r\n", ch );
	send_to_char( "         shop delete [#0-4]\r\n", ch );
	return FALSE;
    }


    if ( !str_cmp( command, "hours" ))   {
	if ( arg1[0] == '\0' || !is_number( arg1 )
	|| argument[0] == '\0' || !is_number( argument )) {
	    send_to_char( "Syntax:  shop hours [#opening] [#closing]\r\n", ch );
	    return FALSE;
	}

	if ( !pMob->pShop ) {
	    pMob->pShop         = new_shop();
	    pMob->pShop->keeper = pMob->vnum;
	    shop_last->next     = pMob->pShop;
	}

	pMob->pShop->open_hour = atoi( arg1 );
	pMob->pShop->close_hour = atoi( argument );

	send_to_char( "Shop hours set.\r\n", ch);
	return TRUE;
    }

    if ( !str_cmp( command, "haggle" ))   {
	if ( arg1[0] == '\0' || !is_number( arg1 )) {
	    send_to_char( "Syntax:  shop haggle [#tolerance%]\r\n", ch );
	    return FALSE;
	}

	if (!pMob->pShop) {
	    pMob->pShop         = new_shop();
	    pMob->pShop->keeper = pMob->vnum;
	    shop_last->next     = pMob->pShop;
	}

	pMob->pShop->haggle = atoi(arg1);
	pMob->pShop->haggle = URANGE(1, pMob->pShop->haggle, 99);

	send_to_char( "Shop haggle tolerance set.\r\n", ch);
	return TRUE;
    }


    if ( !str_cmp( command, "currency" )) {
                int currency;

	if ( arg1[0] == '\0') {
	    send_to_char( "Syntax:  shop currency [#currency]\r\n", ch );
	    return FALSE;
	}

                if (!str_cmp(arg1, "default")) {
                    currency = -1;
                } else {
                    currency = flag_value(currency_accept, arg1);
                    if (currency < 0) {
	        send_to_char( "Syntax:  shop currency [#currency]\r\n", ch );
	        return FALSE;
	    }
                }

	if ( !pMob->pShop ) {
	    pMob->pShop         = new_shop();
	    pMob->pShop->keeper = pMob->vnum;
	    shop_last->next     = pMob->pShop;
	}

	pMob->pShop->currency     = currency;

	send_to_char( "Shop currency set.\r\n", ch);
	return TRUE;
    }


    if ( !str_cmp( command, "profit" )) {
	if ( arg1[0] == '\0' || !is_number( arg1 )
	|| argument[0] == '\0' || !is_number( argument )) {
	    send_to_char( "Syntax:  shop profit [#buying%] [#selling%]\r\n", ch );
	    return FALSE;
	}

	if ( !pMob->pShop ) {
	    pMob->pShop         = new_shop();
	    pMob->pShop->keeper = pMob->vnum;
	    shop_last->next     = pMob->pShop;
	}

	pMob->pShop->profit_buy     = atoi( arg1 );
	pMob->pShop->profit_sell    = atoi( argument );
                if (pMob->pShop->profit_buy < pMob->pShop->profit_sell) {
	    send_to_char( "Shop_Buy can't be < Shop_Sell!\r\n", ch );
                    pMob->pShop->profit_buy = pMob->pShop->profit_sell;
	}

	send_to_char( "Shop profit set.\r\n", ch);
	return TRUE;
    }


    if ( !str_cmp( command, "type" ))    {
	char buf[MAX_INPUT_LENGTH];
	int value;

	if ( arg1[0] == '\0' || !is_number( arg1 )
	|| argument[0] == '\0' ) {
	    send_to_char( "Syntax:  shop type [#0-4] [item type]\r\n", ch );
	    return FALSE;
	}

	if ( atoi( arg1 ) >= MAX_TRADE ) {
	    sprintf( buf, "REdit:  May sell %d items max.\r\n", MAX_TRADE );
	    send_to_char( buf, ch );
	    return FALSE;
	}

	if (( value = flag_value( type_flags, argument ) ) == NO_FLAG 
                && str_cmp(argument, "bank")
                && str_cmp(argument, "stock")
                && str_cmp(argument, "change")
                && str_cmp(argument, "material")
                && str_cmp(argument, "magistrate")
                && str_cmp(argument, "hotel")) {
	    send_to_char( "REdit:  That type of shop is not known.\r\n", ch );
	    return FALSE;
	}

	if ( !pMob->pShop ) {
	    pMob->pShop         = new_shop();
	    pMob->pShop->keeper = pMob->vnum;
	    shop_last->next     = pMob->pShop;
	}

                if (!str_cmp(argument, "bank")) pMob->pShop->buy_type[atoi( arg1 )] = 999;
                else if (!str_cmp(argument, "hotel")) pMob->pShop->buy_type[atoi( arg1 )] = 998;
                else if (!str_cmp(argument, "stock")) pMob->pShop->buy_type[atoi( arg1 )] = 997;
                else if (!str_cmp(argument, "material")) pMob->pShop->buy_type[atoi( arg1 )] = 996;
                else if (!str_cmp(argument, "change")) pMob->pShop->buy_type[atoi( arg1 )] = 995;
                else if (!str_cmp(argument, "magistrate")) pMob->pShop->buy_type[atoi( arg1 )] = 994;
                else pMob->pShop->buy_type[atoi( arg1 )] = value;
	send_to_char( "Shop type set.\r\n", ch);
	return TRUE;
    }


    if ( !str_cmp( command, "delete" )) {
	SHOP_DATA *pShop;
	SHOP_DATA *pShop_next;
	int value;
	int cnt = 0;
	
	if ( arg1[0] == '\0' || !is_number( arg1 )) {
	    send_to_char( "Syntax:  shop delete [#0-4]\r\n", ch );
	    return FALSE;
	}

	value = atoi( argument );
	
	if ( !pMob->pShop ) {
	    send_to_char( "REdit:  Non-existant shop.\r\n", ch );
	    return FALSE;
	}

	if ( value == 0 ) {
	    pShop = pMob->pShop;
	    pMob->pShop = pMob->pShop->next;
	    free_shop( pShop );
	} else {
	    for ( pShop = pMob->pShop, cnt = 0; pShop; pShop = pShop_next, cnt++ ) {
	        pShop_next = pShop->next;
	        if ( cnt+1 == value )   {
		pShop->next = pShop_next->next;
		free_shop( pShop_next );
		break;
	        }
                    }
	}

	send_to_char( "Shop deleted.\r\n", ch);
	return TRUE;
    }

    medit_shop( ch, "" );
    return FALSE;
}


/* ROM medit functions: */


MEDIT( medit_sex )          /* Moved out of medit() due to naming conflicts -- Hugin */
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( sex_flags, argument ) ) != NO_FLAG )
	{
	    pMob->sex = value;

	    send_to_char( "Sex set.\r\n", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax: sex [sex]\r\n"
		  "Type '? sex' for a list of flags.\r\n", ch );
    return FALSE;
}


MEDIT( medit_act ) {
    MOB_INDEX_DATA *pMob;
    long long value;

    if ( argument[0] != '\0' )    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value_long( act_flags, argument ) ) != NO_FLAG ) {
	    pMob->act ^= value;
	    SET_BIT( pMob->act, ACT_IS_NPC );

	    send_to_char( "Act flag toggled.\r\n", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax: act [flag]\r\n"
		  "Type '? act' for a list of flags.\r\n", ch );
    return FALSE;
}


MEDIT( medit_affect )  {
    MOB_INDEX_DATA *pMob;
    long long value;

    if ( argument[0] != '\0' )    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value_long( affect_flags, argument ) ) != NO_FLAG )	{
	    pMob->affected_by ^= value;

	    send_to_char( "Affect flag toggled.\r\n", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax: affect [flag]\r\n"
		  "Type '? affect' for a list of flags.\r\n", ch );
    return FALSE;
}



MEDIT( medit_ac )
{
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH];
    int pierce, bash, slash, exotic;

    do   /* So that I can use break and send the syntax in one place */
    {
	if ( argument[0] == '\0' )  break;

	EDIT_MOB(ch, pMob);
	argument = one_argument( argument, arg );

	if ( !is_number( arg ) )  break;
	pierce = atoi( arg );
	argument = one_argument( argument, arg );

	if ( arg[0] != '\0' )
	{
	    if ( !is_number( arg ) )  break;
	    bash = atoi( arg );
	    argument = one_argument( argument, arg );
	}
	else
	    bash = pMob->ac[AC_BASH];

	if ( arg[0] != '\0' )
	{
	    if ( !is_number( arg ) )  break;
	    slash = atoi( arg );
	    argument = one_argument( argument, arg );
	}
	else
	    slash = pMob->ac[AC_SLASH];

	if ( arg[0] != '\0' )
	{
	    if ( !is_number( arg ) )  break;
	    exotic = atoi( arg );
	}
	else
	    exotic = pMob->ac[AC_EXOTIC];

	pMob->ac[AC_PIERCE] = pierce;
	pMob->ac[AC_BASH]   = bash;
	pMob->ac[AC_SLASH]  = slash;
	pMob->ac[AC_EXOTIC] = exotic;
	
	send_to_char( "Ac set.\r\n", ch );
	return TRUE;
    } while ( FALSE );    /* Just do it once.. */

    send_to_char( "Syntax:  ac [ac-pierce [ac-bash [ac-slash [ac-exotic]]]]\r\n"
		  "help MOB_AC  gives a list of reasonable ac-values.\r\n", ch );
    return FALSE;
}

MEDIT( medit_form )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( form_flags, argument ) ) != NO_FLAG )
	{
	    pMob->form ^= value;
	    send_to_char( "Form toggled.\r\n", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: form [flags]\r\n"
		  "Type '? form' for a list of flags.\r\n", ch );
    return FALSE;
}

MEDIT( medit_part )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( part_flags, argument ) ) != NO_FLAG )
	{
	    pMob->parts ^= value;
	    send_to_char( "Parts toggled.\r\n", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: part [flags]\r\n"
		  "Type '? part' for a list of flags.\r\n", ch );
    return FALSE;
}


MEDIT( medit_imm ) {
MOB_INDEX_DATA *pMob;
int value;

    if ( argument[0] != '\0' ) {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( imm_flags, argument ) ) != NO_FLAG ) {
	    pMob->imm_flags ^= value;
	    send_to_char( "Immunity toggled.\r\n", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: imm [flags]\r\n"
		  "Type '? imm' for a list of flags.\r\n", ch );
    return FALSE;
}


MEDIT( medit_envimm ) {
MOB_INDEX_DATA *pMob;
int value;

    if ( argument[0] != '\0' ) {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( imm_flags, argument ) ) != NO_FLAG ) {
	    pMob->envimm_flags ^= value;
	    send_to_char( "Environment Immunity toggled.\r\n", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: envimm [flags]\r\n"
		  "Type '? envimm' for a list of flags.\r\n", ch );
    return FALSE;
}


MEDIT( medit_res ) {
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' ) {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( res_flags, argument ) ) != NO_FLAG ) {
	    pMob->res_flags ^= value;
	    send_to_char( "Resistance toggled.\r\n", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: res [flags]\r\n"
		  "Type '? res' for a list of flags.\r\n", ch );
    return FALSE;
}


MEDIT( medit_vuln ) {
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' ) {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( vuln_flags, argument ) ) != NO_FLAG ) {
	    pMob->vuln_flags ^= value;
	    send_to_char( "Vulnerability toggled.\r\n", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: vuln [flags]\r\n"
		  "Type '? vuln' for a list of flags.\r\n", ch );
    return FALSE;
}


MEDIT( medit_material )
{
    MOB_INDEX_DATA *pMob;
    int value;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  material [material-name]\r\n"
		      "Type '? material' for a list of materials.\r\n", ch );
	return FALSE;
    }

    if ( ( value = flag_value( material_type, argument ) ) != NO_FLAG )
    {
        pMob->material = value;
        send_to_char( "Material type set.\r\n", ch);
        return TRUE;
    }

    send_to_char( "Unknown material type, '? material' for a list.\r\n", ch );
    return FALSE;
}

MEDIT( medit_off )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( off_flags, argument ) ) != NO_FLAG )
	{
	    pMob->off_flags ^= value;
	    send_to_char( "Offensive behaviour toggled.\r\n", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: off [flags]\r\n"
		  "Type '? off' for a list of flags.\r\n", ch );
    return FALSE;
}

MEDIT( medit_attack )
{
	MOB_INDEX_DATA *pMob;
	int value;
    
	if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( attack_type, argument ) ) != NO_FLAG )
	{
	    pMob->atk_type = value;
	    pMob->dam_type = attack_table[value].damage;
	    send_to_char( "Attack and damage types set.\r\n",ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax: Attack [type]\r\n"
		  "Type '? attack' for a list of attack types.\r\n", ch );
    return FALSE;
}
	
MEDIT( medit_damtype )
{
	MOB_INDEX_DATA *pMob;
	int value;
    
	if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( damage_type, argument ) ) != NO_FLAG )
	{
	    pMob->dam_type = value;
	    send_to_char( "Damage type set.\r\n",ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax: Damtype [type]\r\n"
		  "Type '? damtype' for a list of damage types.\r\n", ch );
    return FALSE;
}

MEDIT( medit_size )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( size_flags, argument ) ) != NO_FLAG )
	{
	    pMob->size = value;
	    send_to_char( "Size set.\r\n", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: size [size]\r\n"
		  "Type '? size' for a list of sizes.\r\n", ch );
    return FALSE;
}

MEDIT( medit_hitdice )
{
    static char syntax[] = "Syntax:  hitdice <number> d <type> + <bonus>\r\n";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    num = cp = argument;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) )  *(cp++) = '\0';

    type = cp;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) ) *(cp++) = '\0';

    bonus = cp;

    while ( isdigit( *cp ) ) ++cp;
    if ( *cp != '\0' ) *cp = '\0';

    if ( ( !is_number( num   ) || atoi( num   ) < 1 )
    ||   ( !is_number( type  ) || atoi( type  ) < 1 ) 
    ||   ( !is_number( bonus ) || atoi( bonus ) < 0 ) )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    pMob->hit[DICE_NUMBER] = atoi( num   );
    pMob->hit[DICE_TYPE]   = atoi( type  );
    pMob->hit[DICE_BONUS]  = atoi( bonus );

    send_to_char( "Hitdice set.\r\n", ch );
    return TRUE;
}

MEDIT( medit_manadice )
{
    static char syntax[] = "Syntax:  manadice <number> d <type> + <bonus>\r\n";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    num = cp = argument;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) )  *(cp++) = '\0';

    type = cp;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) ) *(cp++) = '\0';

    bonus = cp;

    while ( isdigit( *cp ) ) ++cp;
    if ( *cp != '\0' ) *cp = '\0';

    if ( !( is_number( num ) && is_number( type ) && is_number( bonus ) ) )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    if ( ( !is_number( num   ) || atoi( num   ) < 1 )
    ||   ( !is_number( type  ) || atoi( type  ) < 1 ) 
    ||   ( !is_number( bonus ) || atoi( bonus ) < 0 ) )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    pMob->mana[DICE_NUMBER] = atoi( num   );
    pMob->mana[DICE_TYPE]   = atoi( type  );
    pMob->mana[DICE_BONUS]  = atoi( bonus );

    send_to_char( "Manadice set.\r\n", ch );
    return TRUE;
}

MEDIT( medit_damdice )
{
    static char syntax[] = "Syntax:  damdice <number> d <type> + <bonus>\r\n";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    num = cp = argument;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) )  *(cp++) = '\0';

    type = cp;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) ) *(cp++) = '\0';

    bonus = cp;

    while ( isdigit( *cp ) ) ++cp;
    if ( *cp != '\0' ) *cp = '\0';

    if ( !( is_number( num ) && is_number( type ) && is_number( bonus ) ) )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    if ( ( !is_number( num   ) || atoi( num   ) < 1 )
    ||   ( !is_number( type  ) || atoi( type  ) < 1 ) 
    ||   ( !is_number( bonus ) || atoi( bonus ) < 0 ) )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    pMob->damage[DICE_NUMBER] = atoi( num   );
    pMob->damage[DICE_TYPE]   = atoi( type  );
    pMob->damage[DICE_BONUS]  = atoi( bonus );

    send_to_char( "Damdice set.\r\n", ch );
    return TRUE;
}


MEDIT( medit_race ) {
MOB_INDEX_DATA *pMob;
int race;

    if ( argument[0] != '\0'
    && ( race = race_lookup( argument ) ) != 0 )  {
	EDIT_MOB( ch, pMob );

	pMob->race = race;

	pMob->off_flags   = race_array[race].off;
	pMob->imm_flags   = race_array[race].imm;
	pMob->res_flags   = race_array[race].res;
	pMob->vuln_flags  = race_array[race].vuln;
	pMob->form        = race_array[race].form;
	pMob->parts       = race_array[race].parts;
                pMob->material = race_array[race].material;
                pMob->nature = race_array[race].nature;
                if (race_array[race].size >= SIZE_TINY && race_array[race].size < SIZE_MAXI) pMob->size = race_array[race].size;
                if (race_array[race].language > 0) pMob->language = race_array[race].language;

	send_to_char( "Race set.\r\n", ch );
	return TRUE;
    }

    if ( argument[0] == '?' )    {
	char buf[MAX_STRING_LENGTH];

	send_to_char( "Available races are:", ch );

	for ( race = 0; race_array[race].name != NULL; race++ ) {
	    if ( ( race % 3 ) == 0 ) send_to_char( "\r\n", ch );
	    sprintf( buf, " %-15s", race_array[race].name );
	    send_to_char( buf, ch );
	}

	send_to_char( "\r\n", ch );
	return FALSE;
    }

    send_to_char( "Syntax:  race [race]\r\n"
		  "Type 'race ?' for a list of races.\r\n", ch );
    return FALSE;
}


MEDIT( medit_language) {
MOB_INDEX_DATA *pMob;
int lang;

    if ( argument[0] != '\0'
    && (lang = get_skill_sn(argument)) != 0)  {
	EDIT_MOB( ch, pMob );

                if (str_cmp(get_skill_group_name(lang), "language")) {
                      sprintf_to_char(ch, "%s is no language.\r\n", skill_array[lang].name);
                      return FALSE;
                }

	pMob->language = lang;

	send_to_char( "Language set.\r\n", ch );
	return TRUE;
    }

    if ( argument[0] == '?' )    {
                do_skills(ch, "lang");
	return FALSE;
    }

    send_to_char( "Syntax:  language [language]\r\n"
		  "Type 'language ?' for a list of languages.\r\n", ch );
    return FALSE;
}


MEDIT( medit_position ) {
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH];
    int value;

    argument = one_argument( argument, arg );

    switch ( arg[0] )
    {
    default:
	break;

    case 'S':
    case 's':
	if ( str_prefix( arg, "start" ) )
	    break;

	if ( ( value = flag_value( position_flags, argument ) ) == NO_FLAG )
	    break;

	EDIT_MOB( ch, pMob );

	pMob->start_pos = value;
	send_to_char( "Start position set.\r\n", ch );
	return TRUE;

    case 'D':
    case 'd':
	if ( str_prefix( arg, "default" ) )
	    break;

	if ( ( value = flag_value( position_flags, argument ) ) == NO_FLAG )
	    break;

	EDIT_MOB( ch, pMob );

	pMob->default_pos = value;
	send_to_char( "Default position set.\r\n", ch );
	return TRUE;
    }

    send_to_char( "Syntax:  position [start/default] [position]\r\n"
		  "Type '? position' for a list of positions.\r\n", ch );
    return FALSE;
}


MEDIT( medit_nature )
{
    MOB_INDEX_DATA *pMob;
    int value;

    EDIT_MOB(ch, pMob);

    value = flag_value( mob_nature, argument );

    if ( value == NO_FLAG ) {
	send_to_char( "Enter: ? nature for details\r\n", ch );
	return FALSE;
    }

    pMob->nature ^= value;

    send_to_char( "Nature toggled.\r\n", ch);
    return TRUE;
}


MEDIT( medit_timesub )
{
    MOB_INDEX_DATA *pMob;
    int value;

    EDIT_MOB(ch, pMob);

    value = flag_value( time_sub, argument );

    if ( value == NO_FLAG ) {
	send_to_char( "Enter: ? timesub for details\r\n", ch );
	return FALSE;
    }

    pMob->time_wev ^= value;

    send_to_char( "Time Event Subscription toggled.\r\n", ch);
    return TRUE;
}


MEDIT( medit_fright )
{
    MOB_INDEX_DATA *pMob;

    int fright;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' 
      || !is_number( argument ) ) {
	send_to_char( "Syntax:  fright [number]\r\n", ch );
	return FALSE;
    }

    fright = atoi( argument );

    if ( fright != 0 
      && ( fright < 50 
        || fright > 200 )) {
	send_to_char( "Fright value 0 or between 50 and 200.\r\n", ch );
	return FALSE;
    }

    pMob->fright = fright;

    send_to_char( "Fright set.\r\n", ch);
    return TRUE;
}


MEDIT( medit_gold )
{
    MOB_INDEX_DATA *pMob;

    int gold;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' 
      || !is_number( argument ) ) {
	send_to_char( "Syntax:  gold [number]\r\n", ch );
	return FALSE;
    }

    gold = atoi( argument );

    if ( gold < 0 
      || gold > 10000) {
	send_to_char( "Gold value between 0 and 10000.\r\n", ch );
	return FALSE;
    }

    pMob->gold = gold;

    send_to_char( "Gold set.\r\n", ch);
    return TRUE;
}


MEDIT( medit_hitroll )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  hitroll [number]\r\n", ch );
	return FALSE;
    }

    pMob->hitroll = atoi( argument );

    send_to_char( "Hitroll set.\r\n", ch);
    return TRUE;
}

MEDIT( medit_vision ) {

    MOB_INDEX_DATA *pMob;

    char cmd[MAX_STRING_LENGTH];

    ECOND_DATA *ec;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' 
      || argument[0] == '?' ) {
	send_to_char( "Syntax:  vision list\r\n"
                      "         vision can_see condition\r\n"
                      "         vision can_not_see condition\r\n", ch );
	return FALSE;
    }

    argument = one_argument(argument, cmd);

    if (!str_cmp(cmd, "list")) {

      send_to_char("can_see:\r\n", ch);
 
      if (pMob->can_see == NULL) {
        send_to_char("    No conditions\r\n", ch);
      } else {
        print_econd(ch, pMob->can_see, "    ");
      } 

      send_to_char("can_not_see:\r\n", ch);
 
      if (pMob->can_not_see == NULL) {
        send_to_char("    No conditions\r\n", ch);
      } else {
        print_econd(ch, pMob->can_not_see, "    ");
      } 

    }

    if (!str_cmp(cmd, "can_see")) {
   
     /* Delete all conditions? */

      if (!str_cmp(argument, "none")) {

        while (pMob->can_see != NULL) {
          ec = pMob->can_see;
          pMob->can_see = ec->next;
          free_ec(ec);
        }

        send_to_char("All conditions removed.\r\n", ch);

        return TRUE;
      } 

     /* Now get the condition... */

      ec = create_econd(argument, ch);

      if (ec == NULL) {
        return TRUE;
      }

     /* Ok, now we add the condition... */

      ec->next = pMob->can_see;
      pMob->can_see = ec;

    }

    if (!str_cmp(cmd, "can_not_see")) {
   
     /* Delete all conditions? */

      if (!str_cmp(argument, "none")) {

        while (pMob->can_not_see != NULL) {
          ec = pMob->can_not_see;
          pMob->can_not_see = ec->next;
          free_ec(ec);
        }

        send_to_char("All conditions removed.\r\n", ch);

        return TRUE;
      } 

     /* Now get the condition... */

      ec = create_econd(argument, ch);

      if (ec == NULL) {
        return TRUE;
      }

     /* Ok, now we add the condition... */

      ec->next = pMob->can_not_see;
      pMob->can_not_see = ec;

    }

    return TRUE;
}


MEDIT( medit_skills ) {
MOB_INDEX_DATA *pMob;
int sn, col, skill;
char outbuf[MAX_SKILL * 50];
char arg1[MAX_INPUT_LENGTH];
char buf[MAX_STRING_LENGTH];
 
 /* Find the Mob we're editing... */

  EDIT_MOB(ch, pMob);

  outbuf[0] = '\0';

 /* Give help... */

  if (argument[0] == '?') {

    if (argument[1] == '?') {

      col = 0;
      for ( sn = 0; sn < MAX_SKILL; sn++) {

        if (!skill_array[sn].loaded) {
          break;
        }

        if (!(skill_array[sn].group & (SKILL_GROUP_CNG))) {
          continue;
        } 

        sprintf( buf, "%-18s ", skill_array[sn].name);
 
        strcat(outbuf, buf);
  
        if ( ++col % 4 == 0 ) {
          strcat(outbuf, "{x\r\n" );
        } 
      }

      page_to_char(outbuf, ch);

    } else {
      send_to_char("Syntax: Skills\r\n", ch);
      send_to_char("        Skills <skill> <points>\r\n", ch);
      send_to_char("        Skills ??", ch);
      send_to_char("\r\nSee 'help MOBSKILLS'\r\n", ch);
    }

    return TRUE;
  }

 /* No parms mean list current skills... */

  if ( argument[0] == '\0' ) {
    
    if (pMob->skills == NULL) {
      send_to_char("Mob is unskilled.\r\n", ch);
      return TRUE;
    }

    outbuf[0] = '\0';

    col    = 0;
    for ( sn = 0; sn < MAX_SKILL; sn++ ) {
	
      if (!skill_array[sn].loaded) {
        break;
      }

      if (pMob->skills->skill[sn] == 0) 
        continue;

      skill = pMob->skills->skill[sn];

      if (skill < 1) {
        strcat(outbuf, "{x");
      } else if (skill < SKILL_ADEPT) {
        strcat(outbuf, "{g");
      } else if (skill < SKILL_MASTER) {
        strcat(outbuf, "{c");
      } else if (skill < SKILL_EXEMPLAR) {
        strcat(outbuf, "{w");
      } else {
        strcat(outbuf, "{Y");
      }

      sprintf( buf, "%-18s %3d   ", skill_array[sn].name, skill);

      strcat(outbuf, buf);

      if ( ++col % 3 == 0 ) {
        strcat(outbuf, "{x\r\n" );
      } 
    }

    if ( col % 3 != 0 ) {
      strcat(outbuf, "{x\r\n");
    }
 
    sprintf( buf, "{wMob has {C%ld{w skill points in total, "
                  "expected number for level is {C%d{w.{x\r\n",
                   pMob->skills->total_points,
                   pMob->level * 25 + 200);

    strcat( outbuf, buf);

    page_to_char( outbuf, ch );

  } else {

   /* Otherwise it's skill setting time... */

    argument = one_argument(argument, arg1); 

    if (argument[0] != '\0') {
      skill = atoi(argument);
    } else {
      skill = pMob->level;
    }

    if (skill < 0) {
      send_to_char("Syntax: skills <skill> <value>\r\n", ch);
      return TRUE;
    } 

    sn = get_skill_sn( arg1 );

    if (  sn < 0  ) {
      send_to_char( "No such skill.\r\n", ch );
      return TRUE;
    }

   /* Do we need to become skilled? */

    if (pMob->skills == NULL) {
      pMob->skills = getSkillData();
      send_to_char("Mob is now skilled.\r\n"
                   "{YRemember to set ALL necessary skills!{x\r\n", ch);
    }

   /* Update the skill... */

    setSkill(pMob->skills, sn, skill);

    send_to_char("Done.\r\n", ch);

    return TRUE;
  }

  return TRUE;
}


void do_end (CHAR_DATA *ch, char *arg) {
    ch->desc->editor = 0;
    return;
}


MEDIT( medit_setprof ) {
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
MOB_INDEX_DATA *pMob;
PROF_DATA *prof, *prof2;
int sn, lev, modlev;
int num_prac;
int practice[MAX_SKILL];

 /* Find the Mob we're editing... */

  EDIT_MOB(ch, pMob);

 /* Give help... */

  if ( argument[0] == '\0' || argument[0] == '?') {
    send_to_char("Syntax: SETPROF '<profession>' ['<secondary profession>']\r\n", ch);
    send_to_char("Sets a mobs skills to those for the profession.\r\n", ch);
    return TRUE;
  }
  
  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

 /* Find the profession... */
 
  if (!str_cmp(arg1, "none")) {
        if (pMob->skills) {
           for(sn = 0; sn < MAX_SKILL; sn++) {
               setSkill(pMob->skills, sn, 0);
           }
           freeSkillData(pMob->skills);
           pMob->skills = NULL;
        }
        send_to_char("Skills removed.\r\n", ch);
        return TRUE;
  }

  prof = get_profession(arg1);
  prof2 = NULL;
  if (arg2[0] != '\0') prof2 = get_profession(arg2);

  if (!prof) {
    if (!prof2) {
        send_to_char("No such profession.\r\n", ch);
        return TRUE;
    } else {
        prof = prof2;
        prof2 = NULL;
    }
  }

  if (prof2) modlev = pMob->level/2;
  else modlev = pMob->level;

 /* Can't train 0 level mobs... */

  if (modlev == 0) {
    send_to_char("Set the mobs level first!\r\n", ch);
    return TRUE;
  }

 /* Do we need to become skilled? */

  if (pMob->skills == NULL) {
    pMob->skills = getSkillData();
    send_to_char("Mob is now skilled.\r\n"
                 "{YPlease review skills after training...{x\r\n", ch);
  }

 /* Now, work out which skills we can practice... */

  num_prac = -1;

 /* Now erase all of the mobs old skills and set the new base skills... */

  for(sn = 0; sn < MAX_SKILL; sn++) {

    if (prof->levels->skill[sn] == 0) {
      setSkill(pMob->skills, sn, number_fuzzy(42));
      practice[++num_prac] = sn;
    } else {
      setSkill(pMob->skills, sn, 0);
    }
  }

 /* Now, level by level practicing... */

  for(lev = 1;lev < modlev; lev++) {

    for(sn = 0; sn < MAX_SKILL; sn++) {
      if (prof->levels->skill[sn] == lev) {
        practice[++num_prac] = sn;
      }
    } 

   /* 4 sets of 8 = 32 points/level, but beware of reduction... */

    sn = practice[number_range(0, num_prac)];
    addMobSkill(pMob, sn, number_fuzzy(8));
    sn = practice[number_range(0, num_prac)];
    addMobSkill(pMob, sn, number_fuzzy(8));
    sn = practice[number_range(0, num_prac)];
    addMobSkill(pMob, sn, number_fuzzy(8));
    sn = practice[number_range(0, num_prac)];
    addMobSkill(pMob, sn, number_fuzzy(8));

  }

  sprintf_to_char(ch, "Mob trained to level %d in profession '%s'\r\n", modlev, arg1);
  if (!prof2) return TRUE;

  num_prac = -1;

  for(sn = 0; sn < MAX_SKILL; sn++) {
    if (prof2->levels->skill[sn] == 0) {
      setSkill(pMob->skills, sn, number_fuzzy(42));
      practice[++num_prac] = sn;
    }
  }

  for(lev = 1;lev < modlev; lev++) {
     for(sn = 0; sn < MAX_SKILL; sn++) {
        if (prof2->levels->skill[sn] == lev) practice[++num_prac] = sn;
     } 

    sn = practice[number_range(0, num_prac)];
    addMobSkill(pMob, sn, number_fuzzy(8));
    sn = practice[number_range(0, num_prac)];
    addMobSkill(pMob, sn, number_fuzzy(8));
    sn = practice[number_range(0, num_prac)];
    addMobSkill(pMob, sn, number_fuzzy(8));
    sn = practice[number_range(0, num_prac)];
    addMobSkill(pMob, sn, number_fuzzy(8));
  }

  sprintf_to_char(ch, "Mob trained to level %d in profession '%s'\r\n", modlev, arg2);
  return TRUE;
}


/* Mob conversational editor routines... */

void do_conv_list(CHAR_DATA *ch, MOB_INDEX_DATA *mob, char *args) {

  char cid[MAX_INPUT_LENGTH];
  char sid[MAX_INPUT_LENGTH];

  char buf[MAX_STRING_LENGTH];

  CONV_DATA *cd;
  CONV_SUB_DATA *csd;
  CONV_SUB_TRANS_DATA *cstd;

  int conv_id, sub_id;

 /* Argument is optional cid and optional sid */

  args = one_argument(args, cid);
  args = one_argument(args, sid);

 /* No cid, means we list all conversations... */

  if (cid[0] == '\0') {
    
    cd = mob->cd;

    if (cd == NULL) {
      send_to_char("Mob has no conversations defined.\r\n", ch);
      return;
    }

    send_to_char(" Conv  - Name\r\n"
                 " ------------------------------------------------\r\n", ch); 

    while (cd != NULL) {

      sprintf(buf, " %5d - %s\r\n",
                    cd->conv_id,
                    cd->name);
      send_to_char(buf, ch);

      cd = cd->next;
    }

    return;
  }

 /* Ok, find a specific conversation... */

  conv_id = atoi(cid);

  cd = find_cd(mob, conv_id);

  if (cd == NULL) {
    send_to_char("Mob does not have that conversation defined.\r\n", ch);
    return;
  }

  sprintf(buf, "Conversation: [%5d] Name: [%s]\r\n",
                cd->conv_id,
                cd->name);

  send_to_char(buf, ch);

  csd = cd->csd;

  if (csd == NULL) {
    send_to_char("No conversation subjects defined.\r\n", ch);
    return;
  }

 /* No sid means we list them all... */ 

  if (sid[0] == '\0') {

    send_to_char(" Subj  - Name\r\n"
                 " ------------------------------------------------\r\n", ch); 

    while (csd != NULL) {

      sprintf(buf, " %5d - %s\r\n         %s\r\n",
                    csd->sub_id,
                    csd->name,
                    csd->kwds);
      send_to_char(buf, ch);

      csd = csd->next;
    }

    return;
  }

 /* Ok, find a specific conversation subject... */

  sub_id = atoi(sid);

  csd = find_csd(cd, sub_id);

  if (csd == NULL) {
    send_to_char("Mob does not have that conversation subjet defined.\r\n", ch);
    return;
  }

 /* Ok, list all of the cstds... */

  sprintf(buf, "Subject: [%5d] Name: [%s]\n"
               "Keywords: [%s]\n",
                csd->sub_id,
                csd->name,
                csd->kwds);
  send_to_char(buf, ch);

  cstd = csd->cstd;

 /* Quick check... */

  if (cstd ==  NULL) {
    send_to_char("Mob has no transitions defined.\r\n", ch);
    return;
  }

 /* List 'em all... */

  send_to_char(" Seq   - In  Out Action\r\n"
               " ------------------------------------------------\r\n", ch); 

  while (cstd != NULL) {

    sprintf(buf, " %5d - %3d %3d %s\r\n",
                  cstd->seq,
                  cstd->in,
                  cstd->out,
                  cstd->action);
    send_to_char(buf, ch);

    print_econd(ch, cstd->cond, "  condition - ");

    cstd = cstd->next;
  } 

 /* All done... */

  return;
} 

void do_conv_add_conv(CHAR_DATA *ch, MOB_INDEX_DATA *mob, char *args) {

  char cid[MAX_INPUT_LENGTH];
  char name[MAX_INPUT_LENGTH];

  CONV_DATA *cd;

  int conv_id;

 /* Argument (for now) is optional cid */

  args = one_argument(args, cid);
  args = one_argument(args, name);

 /* No name, means user got it wrong... */

  if (name[0] == '\0') {
    medit_conv(ch, "");
    return;
  }

 /* Check the conv_id... */

  conv_id = atoi(cid);

  if ( conv_id <= 0
    || conv_id < mob->area->lvnum
    || conv_id > mob->area->uvnum) {
    send_to_char("Conversation id must be a valid vnum for the area!\r\n", ch);
    return;
  } 

 /* Now see if it already exists... */

  cd = find_cd(mob, conv_id);

 /* ...and make a new one if it doesn't... */

  if (cd == NULL) {
   
    cd = get_conv_data();

    cd->next = mob->cd;
    mob->cd = cd;

    cd->conv_id = conv_id;
  }

  if (cd->name != NULL) {
    free_string(cd->name);
  }

  cd->name = strdup(name); 
  smash_tilde(cd->name);

  send_to_char("Conversation added.\r\n", ch);

  return;
} 

void do_conv_add_sub(CHAR_DATA *ch, MOB_INDEX_DATA *mob, char *args) {

  char cid[MAX_INPUT_LENGTH];
  char sid[MAX_INPUT_LENGTH];
  char name[MAX_INPUT_LENGTH];
  char kwds[MAX_INPUT_LENGTH];

  CONV_DATA *cd;
  CONV_SUB_DATA *csd;

  int conv_id, sub_id;

 /* Argument is optional cid sid name kwds */

  args = one_argument(args, cid);
  args = one_argument(args, sid);
  args = one_argument(args, name);
  args = one_argument(args, kwds);

 /* No kwds, means user got it wrong... */

  if (kwds[0] == '\0') {
    medit_conv(ch, "");
    return;
  }

 /* Check the conv_id... */

  conv_id = atoi(cid);

  if ( conv_id <= 0
    || conv_id < mob->area->lvnum
    || conv_id > mob->area->uvnum) {
    send_to_char("Conversation id must be a valid vnum for the area!\r\n", ch);
    return;
  } 

 /* Check the sub_id... */

  sub_id = atoi(sid);

  if (sub_id <= 0) {
    send_to_char("Subject id must be > 0\r\n", ch);
    return;
  }

 /* Now see if the conversation exists... */

  cd = find_cd(mob, conv_id);

 /* ...and make a new one if it doesn't... */

  if (cd == NULL) {
    send_to_char("Mob does not have that conversation defined.\r\n", ch);
    return;	
  }

 /* Now find the csd... */

  csd = find_csd(cd, sub_id);

 /* Make a new one if we must... */

  if (csd == NULL) {

    csd = get_conv_sub_data();

    csd->next = cd->csd;
    cd->csd = csd;

    csd->sub_id = sub_id;
  } 

 /* Set/replace name... */

  if (csd->name != NULL) {
    free_string(csd->name);
  }

  csd->name = strdup(name); 
  smash_tilde(csd->name);

 /* Set/replace keywords... */

  if (csd->kwds != NULL) {
    free_string(csd->kwds);
  }

  csd->kwds = strdup(kwds); 

  send_to_char("Conversation subject added.\r\n", ch);

  return;
} 

void do_conv_add_trans(CHAR_DATA *ch, MOB_INDEX_DATA *mob, char *args) {

  char cid[MAX_INPUT_LENGTH];
  char sid[MAX_INPUT_LENGTH];
  char sseq[MAX_INPUT_LENGTH];
  char sin[MAX_INPUT_LENGTH];
  char sout[MAX_INPUT_LENGTH];

  CONV_DATA *cd;
  CONV_SUB_DATA *csd;
  CONV_SUB_TRANS_DATA *cstd;

  int conv_id, sub_id, seq, in, out;

 /* Argument is cid sid seq in out action */

  args = one_argument(args, cid);
  args = one_argument(args, sid);
  args = one_argument(args, sseq);
  args = one_argument(args, sin);
  args = one_argument(args, sout);

 /* No action, means user got it wrong... */

  if (args[0] == '\0') {
    medit_conv(ch, "");
    return;
  }

 /* Check the conv_id... */

  conv_id = atoi(cid);

  if ( conv_id <= 0
    || conv_id < mob->area->lvnum
    || conv_id > mob->area->uvnum) {
    send_to_char("Conversation id must be a valid vnum for the area!\r\n", ch);
    return;
  } 

 /* Check the sub_id... */

  sub_id = atoi(sid);

  if (sub_id <= 0) {
    send_to_char("Subject id must be > 0\r\n", ch);
    return;
  }

 /* Check the Sequence number... */

  seq = atoi(sseq);

  if (seq < 0) {
    send_to_char("Sequence number must be >= 0\r\n", ch);
    return;
  }

 /* Check the input state number... */

  in = atoi(sin);

  if (in < 0) {
    send_to_char("Input state must be >= 0\r\n", ch);
    return;
  }

 /* Check the output state number... */

  out = atoi(sout);

  if (out < 0) {
    send_to_char("Output state must be >= 0\r\n", ch);
    return;
  }

 /* Now see if the conversation exists... */

  cd = find_cd(mob, conv_id);

 /* ...and make a new one if it doesn't... */

  if (cd == NULL) {
    send_to_char("Mob does not have that conversation defined.\r\n", ch);
    return;	
  }

 /* Now find the csd... */

  csd = find_csd(cd, sub_id);

 /* Then find the subject... */

  if (csd == NULL) {
    send_to_char("Mob does not have that conversation defined.\r\n", ch);
    return;	
  } 

 /* Finally find the cstd... */

  cstd = find_cstd(csd, seq);

 /* Create a new one if we must... */

  if (cstd == NULL) {
    cstd = get_conv_sub_trans_data();

    cstd->seq = seq;

    insert_cstd(cstd, csd);
  } 
 
 /* Set input/output states... */

  cstd->in = in;
  cstd->out = out;

 /* Set/replace action... */

  if (cstd->action != NULL) {
    free_string(cstd->action);
  }

  cstd->action = strdup(args); 
  smash_tilde(cstd->action);

  send_to_char("Conversation subject transition added.\r\n", ch);

  return;
} 

void do_conv_renum(CHAR_DATA *ch, MOB_INDEX_DATA *mob, char *args) {

  char cid[MAX_INPUT_LENGTH];
  char sid[MAX_INPUT_LENGTH];

  CONV_DATA *cd;
  CONV_SUB_DATA *csd;
  CONV_SUB_TRANS_DATA *cstd;

  int conv_id, sub_id, seq;

 /* Argument is cid sid */

  args = one_argument(args, cid);
  args = one_argument(args, sid);

 /* No action, means user got it wrong... */

  if (sid[0] == '\0') {
    medit_conv(ch, "");
    return;
  }

 /* Check the conv_id... */

  conv_id = atoi(cid);

  if ( conv_id <= 0
    || conv_id < mob->area->lvnum
    || conv_id > mob->area->uvnum) {
    send_to_char("Conversation id must be a valid vnum for the area!\r\n", ch);
    return;
  } 

 /* Check the sub_id... */

  sub_id = atoi(sid);

  if (sub_id <= 0) {
    send_to_char("Subject id must be > 0\r\n", ch);
    return;
  }

 /* Now see if the conversation exists... */

  cd = find_cd(mob, conv_id);

  if (cd == NULL) {
    send_to_char("Mob does not have that conversation defined.\r\n", ch);
    return;	
  }

 /* Now find the csd... */

  csd = find_csd(cd, sub_id);

  if (csd == NULL) {
    send_to_char("Mob does not have that subject defined.\r\n", ch);
    return;	
  } 

 /* Now, renumber the cstds... */

  seq = 10;

  cstd = csd->cstd;

  while (cstd != NULL) {
    cstd->seq = seq;
    seq += 10;
    cstd = cstd->next;
  }

  send_to_char("Conversation subject transitions renumbered.\r\n", ch);

  return;
} 

void do_conv_cond(CHAR_DATA *ch, MOB_INDEX_DATA *mob, char *args) {

  char cid[MAX_INPUT_LENGTH];
  char sid[MAX_INPUT_LENGTH];
  char sseq[MAX_INPUT_LENGTH];

  CONV_DATA *cd;
  CONV_SUB_DATA *csd;
  CONV_SUB_TRANS_DATA *cstd;

  ECOND_DATA *ec;
  
  int conv_id, sub_id, seq;

 /* Argument is cid sid seq cond */

  args = one_argument(args, cid);
  args = one_argument(args, sid);
  args = one_argument(args, sseq);

 /* No action, means user got it wrong... */

  if (args[0] == '\0') {
    medit_conv(ch, "");
    return;
  }

 /* Check the conv_id... */

  conv_id = atoi(cid);

  if ( conv_id <= 0
    || conv_id < mob->area->lvnum
    || conv_id > mob->area->uvnum) {
    send_to_char("Conversation id must be a valid vnum for the area!\r\n", ch);
    return;
  } 

 /* Check the sub_id... */

  sub_id = atoi(sid);

  if (sub_id <= 0) {
    send_to_char("Subject id must be > 0\r\n", ch);
    return;
  }

 /* Check the Sequence number... */

  seq = atoi(sseq);

  if (seq < 0) {
    send_to_char("Sequence number must be >= 0\r\n", ch);
    return;
  }

 /* Now see if the conversation exists... */

  cd = find_cd(mob, conv_id);

 /* ...and make a new one if it doesn't... */

  if (cd == NULL) {
    send_to_char("Mob does not have that conversation defined.\r\n", ch);
    return;	
  }

 /* Now find the csd... */

  csd = find_csd(cd, sub_id);

 /* Then find the subject... */

  if (csd == NULL) {
    send_to_char("Mob does not have that subject defined.\r\n", ch);
    return;	
  } 

 /* Finally find the cstd... */

  cstd = find_cstd(csd, seq);

 /* Create a new one if we must... */

  if (cstd == NULL) {
    send_to_char("Mob does not have the transition defined.\r\n", ch);
    return;
  } 
 
 /* Delete all conditions? */

  if (!str_cmp(args,"none")) {

    while (cstd->cond != NULL) {
      ec = cstd->cond;
      cstd->cond = ec->next;
      free_ec(ec);
    }

    send_to_char("All conditions removed.\r\n", ch);

    return;
  } 

 /* Now get the condition... */

  ec = create_econd(args, ch);

  if (ec == NULL) {
    return;
  }

 /* Ok, now we add the condition... */

  ec->next = cstd->cond;
  cstd->cond = ec;

 /* All done... */

  send_to_char("Conversation subject transition condition.\r\n", ch);

  return;
} 

void do_conv_delete(CHAR_DATA *ch, MOB_INDEX_DATA *mob, char *args) {

  char cid[MAX_INPUT_LENGTH];
  char sid[MAX_INPUT_LENGTH];
  char seq[MAX_INPUT_LENGTH];

  CONV_DATA *cd, *last_cd;
  CONV_SUB_DATA *csd, *last_csd;
  CONV_SUB_TRANS_DATA *cstd, *last_cstd;

  int conv_id, sub_id, seq_no;

  bool found;

 /* Argument (for now) is optional cid */

  args = one_argument(args, cid);
  args = one_argument(args, sid);
  args = one_argument(args, seq);

 /* No name, means user got it wrong... */

  if (args[0] != '\0') {
    medit_conv(ch, "");
    return;
  }

 /* Check the conv_id... */

  conv_id = atoi(cid);
  sub_id = atoi(sid);
  seq_no = atoi(seq);

 /* Now see if it already exists... */

  cd = mob->cd;

  found = FALSE;
 
  last_cd = NULL;
 
  while (cd != NULL && !found) {

    if (cd->conv_id == conv_id) {
      found = TRUE;
    } else {
      last_cd = cd;
      cd = cd->next;
    }

  }

 /* We're done if it doesn't... */

  if (cd == NULL) {
    send_to_char("Mob does not have that conversation defined.\r\n", ch);
    return;
  }

 /* Zapping time... */

  if (sub_id == 0) {
    if (last_cd != NULL) {
      last_cd->next = cd->next;
    } else {
      mob->cd = cd->next;
    }

    free_conv_data(cd); 

    send_to_char("Conversation deleted.\r\n", ch);

    return;
  }

 /* Now search for the subject id... */
 
  csd = cd->csd;

  found = FALSE;
 
  last_csd = NULL;
 
  while (csd != NULL && !found) {

    if (csd->sub_id == sub_id) {
      found = TRUE;
    } else {
      last_csd = csd;
      csd = csd->next;
    }

  }

 /* We're done if it doesn't... */

  if (csd == NULL) {
    send_to_char("Mob does not have that subject defined.\r\n", ch);
    return;
  }

 /* Zapping time... */

  if (seq_no == 0) {
    if (last_csd != NULL) {
      last_csd->next = csd->next;
    } else {
      cd->csd = csd->next;
    }

    free_conv_sub_data(csd); 

    send_to_char("Conversation subject deleted.\r\n", ch);

    return;
  }

 /* Ok, find the cstd... */

  cstd = csd->cstd;

  found = FALSE;
 
  last_cstd = NULL;
 
  while (cstd != NULL && !found) {

    if (cstd->seq == seq_no) {
      found = TRUE;
    } else {
      last_cstd = cstd;
      cstd = cstd->next;
    }

  }

 /* We're done if it doesn't... */

  if (cstd == NULL) {
    send_to_char("Mob does not have that subject defined.\r\n", ch);
    return;
  }

 /* Zapping time... */

  if (last_cstd != NULL) {
    last_cstd->next = cstd->next;
  } else {
    csd->cstd = cstd->next;
  }

  free_conv_sub_trans_data(cstd); 

  send_to_char("Conversation subject transition deleted.\r\n", ch);

 /* All done... */

  return;
} 

/* Mob Conversational editor... */

MEDIT( medit_conv )
{
    MOB_INDEX_DATA *pMobIndex;

    char command[MAX_INPUT_LENGTH];

   /* Find the mob we're editing... */

    EDIT_MOB(ch, pMobIndex);

   /* Extract parms... */

    argument = one_argument( argument, command );

   /* Give help... */

    if (command[0] == '\0') {
        send_to_char( 
        "Syntax:  conv list\r\n"
        "         conv list [conv_id]\r\n"
        "         conv list [conv_id] [sub_id]\r\n"
	"         conv addc [conv_id] [name]\r\n"
	"         conv adds [conv_id] [sub_id] [name] [kwds]\r\n"
	"         conv addt [conv_id] [sub_id] [seq] [in] [out] [action]\r\n"
	"         conv cond [conv_id] [sub_id] [seq] [condition]\r\n"
	"         conv renum [conv_id] [sub_id]\r\n"
	"         conv delete [conv_id]\r\n"
	"         conv delete [conv_id] [sub_id]\r\n", ch );
	return FALSE;
    }

   /* Dispatch to command subroutine... */

    if (!str_cmp(command, "list")) {
      do_conv_list(ch, pMobIndex, argument);
      return TRUE;
    }  
     
    if (!str_cmp(command, "addc")) { 
      do_conv_add_conv(ch, pMobIndex, argument);
      return TRUE;
    }

    if (!str_cmp(command, "adds")) { 
      do_conv_add_sub(ch, pMobIndex, argument);
      return TRUE;
    }

    if (!str_cmp(command, "addt")) { 
      do_conv_add_trans(ch, pMobIndex, argument);
      return TRUE;
    }

    if (!str_cmp(command, "cond")) { 
      do_conv_cond(ch, pMobIndex, argument);
      return TRUE;
    }

    if (!str_cmp(command, "renum")) { 
      do_conv_renum(ch, pMobIndex, argument);
      return TRUE;
    }

    if (!str_cmp(command, "delete")) {
      do_conv_delete(ch, pMobIndex, argument);     
      return TRUE;
    }

   /* General help if not found... */

    medit_conv(ch, "");

   /* All done... */ 

    return FALSE;
}

/* Mob script editor routines... */

void do_script_list(CHAR_DATA *ch, MOB_INDEX_DATA *pMobIndex, char *arg) {

  char scr_id[MAX_INPUT_LENGTH];

  char buf[MAX_STRING_LENGTH];

  MOB_SCRIPT *scr;
  MOB_SCRIPT_LINE *scrl;

  int id;

 /* Extract parms... */

  arg = one_argument(arg, scr_id);

 /* Check for stupidity... */

  if (arg[0] != '\0') {
    medit_script(ch, "");
    return;
  }

 /* Now, find the mobs scripts... */ 

  scr = pMobIndex->triggers->scripts;

 /* Inform if there are none... */

  if (scr == NULL) {
    send_to_char("Mob has no scripts defined!\r\n", ch);
    return;
  }

 /* No script id, means we list all of the scripts... */

  if (scr_id[0] == '\0') {

    send_to_char("scr id  Name\r\n"
                 "--------------------------------------------------\r\n", ch);

    while (scr != NULL) {
      
      sprintf(buf, " %5d  %s\r\n",
                   scr->id,
                   scr->name);
      send_to_char(buf, ch);
  
      scr = scr->next;
    }

    return;
  }

 /* Ok, so work out what the id is... */

  id = atoi(scr_id);

 /* Ok, go find the script... */

  scr = find_script2(scr, id);

 /* Warn if not found... */

  if (scr == NULL) {
    send_to_char("Mob does not have that script defined!\r\n", ch);
    return;
  }

 /* Ok, say what we found... */

  sprintf(buf, "Script [%5d] Name: [%s]\r\n", scr->id, scr->name); 

  send_to_char(buf, ch);

 /* Check we have some lines... */

  scrl = scr->lines;

  if (scrl == NULL) {
    send_to_char("Script has no lines defined!\r\n", ch);
    return;
  }

 /* Ok, now we list the lines... */

  send_to_char("Seq   Delay CmdId Command\r\n"
               "----------------------------------------------------\r\n", ch);

  while (scrl != NULL) {

    sprintf(buf, "%5d %5d %5d %s\r\n",
                  scrl->seq,
                  scrl->delay,
                  scrl->cmd_id,
                  scrl->cmd); 

    send_to_char(buf, ch);

    scrl = scrl->next;
  }

  return;
}

void do_script_add_script(CHAR_DATA *ch, MOB_INDEX_DATA *pMobIndex, char *args) {

  char scr_id[MAX_INPUT_LENGTH];

  MOB_SCRIPT *scr;

  int id;

 /* Extract script id... */

  args = one_argument(args, scr_id);

 /* Rest of args is name of script... */

  if (args[0] == '\0') {
    medit_script(ch, "");
    return;
  } 

 /* Translate the script id... */

  id = atoi(scr_id);

 /* Validate it... */

  if (id < 1) {
    send_to_char("Script id must be >= 1!\r\n", ch);
    return;
  }

 /* Go see if we can find it... */

  scr = find_script2(pMobIndex->triggers->scripts, id);

 /* Not found means we need a new one... */

  if (scr == NULL) {
    scr = get_mscr();

    scr->id = id;

    scr->next = pMobIndex->triggers->scripts;
    pMobIndex->triggers->scripts = scr;
  }

 /* Now set the input values... */

  if (scr->name != NULL) {
    free_string(scr->name); 
  }

  scr->name = strdup(args);
  smash_tilde(scr->name);

 /* Tell the user it's ok... */

  send_to_char("Script added.\r\n", ch);

 /* All done... */

  return;
}

void do_script_add_line(CHAR_DATA *ch, MOB_INDEX_DATA *pMobIndex, char *args) {

  char scr_id[MAX_INPUT_LENGTH];
  char seq_id[MAX_INPUT_LENGTH];
  char delay[MAX_INPUT_LENGTH];
  char cmd_id[MAX_INPUT_LENGTH];

  MOB_SCRIPT *scr;
  MOB_SCRIPT_LINE *scrl;

  int id, seq, dly, cid;

 /* Extract script id, sequence id, delay and cmd_id... */

  args = one_argument(args, scr_id);
  args = one_argument(args, seq_id);
  args = one_argument(args, delay);
  args = one_argument(args, cmd_id);

 /* Rest of args is command to issue... */

  if (args[0] == '\0') {
    medit_script(ch, "");
    return;
  } 

 /* Translate the script id... */

  id = atoi(scr_id);

 /* Validate it... */

  if (id < 1) {
    send_to_char("Script id must be >= 1!\r\n", ch);
    return;
  }

 /* Translate the sequence id... */

  seq = atoi(seq_id);

 /* Validate it... */

  if (seq < 1) {
    send_to_char("Sequence id must be >= 1!\r\n", ch);
    return;
  }

 /* Translate the delay... */

  dly = atoi(delay);

 /* Validate it... */

  if (dly < 0) {
    send_to_char("Delay must be >= 0!\r\n", ch);
    return;
  }

 /* Translate the cmd_id... */

  cid = atoi(cmd_id);

 /* Validate it... */

  if (cid < 0) {
    send_to_char("Command id must be >= 0!\r\n", ch);
    return;
  }

 /* Go see if we can find it... */

  scr = find_script2(pMobIndex->triggers->scripts, id);

 /* Not found means we have a problem... */

  if (scr == NULL) {
    send_to_char("Mob does not have that script defined!\r\n", ch);
    return;
  }

 /* Now go find the line... */

  scrl = find_script_line(scr, seq);

 /* Get a new one if we need it... */

  if (scrl == NULL) {
    scrl = get_mscrl();

    scrl->seq = seq;

    insert_script_line(scr, scrl);
  } 

 /* Now set the input values... */

  scrl->delay 	= dly;
  scrl->cmd_id	= cid;

  if (scrl->cmd != NULL) {
    free_string(scrl->cmd); 
  }

  scrl->cmd = strdup(args);
  smash_tilde(scrl->cmd);

 /* Tell the user it's ok... */

  send_to_char("Script line added.\r\n", ch);

 /* All done... */

  return;
}

void do_script_renum(CHAR_DATA *ch, MOB_INDEX_DATA *pMobIndex, char *args) {

  char scr_id[MAX_INPUT_LENGTH];

  MOB_SCRIPT *scr;
  MOB_SCRIPT_LINE *scrl;

  int id, seq;

 /* Extract script id... */

  args = one_argument(args, scr_id);

 /* Rest of args are junk... */

  if (args[0] != '\0') {
    medit_script(ch, "");
    return;
  } 

 /* Translate the script id... */

  id = atoi(scr_id);

 /* Validate it... */

  if (id < 1) {
    send_to_char("Script id must be >= 1!\r\n", ch);
    return;
  }

 /* Go see if we can find it... */

  scr = find_script2(pMobIndex->triggers->scripts, id);

 /* Not found means we have a problem... */

  if (scr == NULL) {
    send_to_char("Mob does not have that script defined!\r\n", ch);
    return;
  }

 /* Renumbering time... */

  scrl = scr->lines;

  seq = 10;

  while (scrl != NULL) {

    scrl->seq = seq;

    seq += 10;

    scrl = scrl->next;
  }

 /* Tell the user it's ok... */

  send_to_char("Script lines renumbered.\r\n", ch);

 /* All done... */

  return;
}


void do_script_delete(CHAR_DATA *ch, MOB_INDEX_DATA *pMobIndex, char *args) {

  char scr_id[MAX_INPUT_LENGTH];
  char seq_id[MAX_INPUT_LENGTH];

  MOB_SCRIPT *scr, *old_scr;
  MOB_SCRIPT_LINE *scrl, *old_scrl;

  int id, seq;

 /* Extract script id, sequence id, delay and cmd_id... */

  args = one_argument(args, scr_id);
  args = one_argument(args, seq_id);

 /* Rest of args should be null... */

  if (args[0] != '\0') {
    medit_script(ch, "");
    return;
  } 

 /* Translate the script id... */

  id = atoi(scr_id);

 /* Validate it... */

  if (id < 1) {
    send_to_char("Script id must be >= 1!\r\n", ch);
    return;
  }

 /* Translate the sequence id... */

  if (seq_id[0] == '\0') {
    seq = 0;
  } else {
    seq = atoi(seq_id);

   /* Validate it... */

    if (seq < 1) {
      send_to_char("Sequence id must be >= 1!\r\n", ch);
      return;
    }
  }

 /* Go see if we can find it... */

  scr = find_script2(pMobIndex->triggers->scripts, id);

 /* Not found means we have a problem... */

  if (scr == NULL) {
    send_to_char("Script not defined - cannot be deleted!\r\n", ch);
    return;
  }

 /* Now, if we're deleting the whole script... */

  if (seq == 0) {

   /* Easy case first... */

    if (pMobIndex->triggers->scripts == scr) {

      pMobIndex->triggers->scripts = scr->next;
      scr->next = NULL;

      free_mscr(scr);

      send_to_char("Script deleted.\r\n", ch);  

      return;
    }

   /* Then the tricky one... */

    old_scr = pMobIndex->triggers->scripts;

    while (old_scr != NULL) {

      if (old_scr->next == scr) {
        old_scr->next = scr->next;
        scr->next = NULL;
        free_mscr(scr);
        old_scr = NULL;
      } else {
        old_scr = old_scr->next;
      }
    } 

    send_to_char("Script deleted.\r\n", ch);  

    return;
  }

 /* Check there are some lines... */

  if (scr->lines == NULL) {
    send_to_char("Script has no lines!\r\n", ch);
    return;
  }

 /* Easy case for lines... */

  scrl = scr->lines;

  if (scr->lines->seq == seq) {

    scr->lines = scrl->next;
    scrl->next = NULL;

    free_mscrl(scrl);

    send_to_char("Script line deleted.\r\n", ch);

    return;
  }

 /* Search time... */

  old_scrl = scr->lines;

  while (old_scrl != NULL) {

    if ( old_scrl->next != NULL
      && old_scrl->next->seq == seq ) {

      scrl = old_scrl->next;

      old_scrl->next = scrl->next;
      scrl->next = NULL;

      free_mscrl(scrl);

    } else { 
      old_scrl = old_scrl->next;
    }
  }
  
 /* Tell the user it's ok... */

  send_to_char("Script line deleted.\r\n", ch);

 /* All done... */

  return;
}

/* Mob Script editor... */

MEDIT( medit_script )
{
    MOB_INDEX_DATA *pMobIndex;

    char command[MAX_INPUT_LENGTH];

   /* Find the mob we're editing... */

    EDIT_MOB(ch, pMobIndex);

   /* Extract parms... */

    argument = one_argument( argument, command );

   /* Give help... */

    if (command[0] == '\0') {
        send_to_char( "Syntax:  script list\r\n", ch);
        send_to_char( "         script list [script_id]\r\n", ch);
        send_to_char( "         script adds [script_id] [name]\r\n", ch);
        send_to_char( "         script addl [script_id] [seq] [delay] [cmd_id] [cmd]\r\n", ch);
        send_to_char( "         script renum [script_id]\r\n", ch);
        send_to_char( "         script delete [script_id]\r\n", ch);
        send_to_char( "         script delete [script_id] [seq]\r\n", ch);
	return FALSE;
    }

   /* Dispatch to command subroutine... */

    if (!str_cmp(command, "list")) {
      do_script_list(ch, pMobIndex, argument);
      return TRUE;
    }  
     
    if (!str_cmp(command, "adds")) {
      do_script_add_script(ch, pMobIndex, argument);
      return TRUE;
    }  
     
    if (!str_cmp(command, "addl")) {
      do_script_add_line(ch, pMobIndex, argument);
      return TRUE;
    }  
     
    if (!str_cmp(command, "renum")) {
      do_script_renum(ch, pMobIndex, argument);
      return TRUE;
    }  
     
    if (!str_cmp(command, "delete")) {
      do_script_delete(ch, pMobIndex, argument);
      return TRUE;
    }  
     
   /* General help if not found... */

    medit_script(ch, "");

   /* All done... */ 

    return FALSE;
}

/* Map from gadget actions to gadget names... */

static char *gad_act[] = {
  "none", 
  "pull",
  "push",
  "twist",
  "turn",
  "turnback",
  "move",
  "lift",
  "press",
  "dig"
};


int gadget_action_number(char *gname) {

  int gact;

  for(gact = 0; gact < GADGET_ACTION_MAX; gact++) {

    if (!str_cmp(gname, gad_act[gact])) {
      return gact;
    }

  }

  return GADGET_ACTION_NONE;
}

char *gadget_action_name(int gact) {

  if ( gact > 0
    && gact < GADGET_ACTION_MAX ) {
    return gad_act[gact];
  } 

  return gad_act[0];
}

/* Gadgets */

void do_gadget_list(CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *args) {
  GADGET_DATA *gadget;

 /* Args should be null... */

  if (args[0] != '\0') {
    redit_gadget(ch, "");
    return;
  } 

 /* Anything to show? */

  gadget = room->gadgets;

  if (gadget == NULL) {
    send_to_char("No gadgets defined.\r\n", ch);
    return;
  }

 /* Showtime... */

  send_to_char("Id    Names\r\n"
               "----- --------------------------------------------\r\n", ch);

  while (gadget != NULL) {
   
    sprintf_to_char(ch, "%5d %s\r\n", gadget->id, gadget->name);
    if (gadget->link_room > 0
    && gadget->link_id > 0) {
         sprintf_to_char(ch, "  =>  Gadget: %d / %d\r\n", gadget->link_room, gadget->link_id);
    }

    gadget = gadget->next;
  }

 /* All done... */

  return;
}

void do_gadget_show(CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *args) {
  char c_id[MAX_INPUT_LENGTH];
  int id;
  GADGET_DATA *gadget;
  GADGET_TRANS_DATA *gdt;

 /* Extract gadget id... */

  args = one_argument(args, c_id);

 /* Rest of args should be null... */

  if (args[0] != '\0') {
    redit_gadget(ch, "");
    return;
  } 

 /* Extract the id... */

  id = atoi(c_id);

 /* Now find the gadget... */ 

  gadget = find_gadget_by_id(room, id);

  if (gadget == NULL) {
    send_to_char("No such gadget.\r\n", ch);
    return;
  }

 /* Show and tell... */

  sprintf_to_char(ch, "{wGadget [{c%5d{w] Names [{c%s{w]{x\r\n", gadget->id, gadget->name);

 /* Find the transitions... */

  gdt = gadget->gdt;

  if (gdt == NULL) {
    send_to_char("No transitions defined.\r\n", ch); 
    return;
  }
  
  
 /* Display them... */

  send_to_char("Seq   Action   In    Out   Message\r\n"
               "----- -------- ----- ----- -------------------------\r\n", ch); 

  while (gdt != NULL) {  

    sprintf_to_char(ch, "%5d %8s %5d %5d %s\r\n", gdt->seq, gadget_action_name(gdt->action), gdt->in, gdt->out, gdt->message);

    if (gdt->cond != NULL) {
      print_econd(ch, gdt->cond, "  condition - ");
    }  

    gdt = gdt->next;
  }

  return;
}


void do_gadget_add(CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *args) {
  char c_id[MAX_INPUT_LENGTH];
  int id;

  GADGET_DATA *gadget;

 /* Extract gadget id... */

  args = one_argument(args, c_id);

 /* Rest of args should be the names... */

  if (args[0] == '\0') {
    redit_gadget(ch, "");
    return;
  } 

 /* Extract the id... */

  id = atoi(c_id);

 /* Now find the gadget... */ 

  gadget = find_gadget_by_id(room, id);

 /* New or old? */

  if (gadget == NULL) {
    gadget = get_gadget(); 

    gadget->id = id;
    gadget->gdt = NULL;
    gadget->next = NULL;

    gadget->next = room->gadgets;
    room->gadgets = gadget; 
  }

 /* Update the name... */
 
  if (gadget->name != NULL) {
    free_string(gadget->name);
    gadget->name = NULL;
  }

  gadget->name = str_dup(args);

 /* All done... */
  send_to_char("Gadget added.\r\n", ch);
  return;
}


void do_gadget_addl(CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *args) {
  char c_id[MAX_INPUT_LENGTH];
  char c_room[MAX_INPUT_LENGTH];
  char c_id2[MAX_INPUT_LENGTH];
  int id, iroom, id2;
  GADGET_DATA *gadget;
  ROOM_INDEX_DATA *room2;
  GADGET_DATA *gadget2;

  args = one_argument(args, c_id);
  args = one_argument(args, c_room);
  args = one_argument(args, c_id2);

 /* Extract the id... */

  id = atoi(c_id);
  iroom = atol(c_room);
  id2 = atoi(c_id2);

  gadget = find_gadget_by_id(room, id);

  if (gadget == NULL) {
    send_to_char("No such gadget.\r\n", ch);
    return;
  }

  if ((room2 = get_room_index(iroom)) == NULL) {
    send_to_char("Link-to gadget not found.\r\n", ch);
    return;
  }

  if ((gadget2 = find_gadget_by_id(room2, id2)) == NULL) {
    send_to_char("Link-to gadget not found.\r\n", ch);
    return;
  }

  gadget->link_room = iroom;
  gadget->link_id = id2;
  gadget2->link_room = room->vnum;
  gadget2->link_id = id;

  send_to_char("Gadget link added.\r\n", ch);
  return;
}


void do_gadget_addt(CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *args) {

  char c_id[MAX_INPUT_LENGTH];
  char c_seq[MAX_INPUT_LENGTH];
  char c_action[MAX_INPUT_LENGTH];
  char c_in[MAX_INPUT_LENGTH];
  char c_out[MAX_INPUT_LENGTH];

  int id;
  int seq; 
  int action;
  int in;
  int out;

  GADGET_DATA *gadget;
  GADGET_TRANS_DATA *gdt;

 /* Extract gadget id... */

  args = one_argument(args, c_id);
  args = one_argument(args, c_seq);
  args = one_argument(args, c_action);
  args = one_argument(args, c_in);
  args = one_argument(args, c_out);

 /* Rest of args should be the message... */

  if (args[0] == '\0') {
    redit_gadget(ch, "");
    return;
  } 

 /* Extract the id... */

  id = atoi(c_id);

 /* Now find the gadget... */ 

  gadget = find_gadget_by_id(room, id);

 /* New or old? */

  if (gadget == NULL) {
    send_to_char("No such gadget!\r\n", ch);
    return; 
  }

 /* Now chew through the rst of the parms... */

  seq = atoi(c_seq);
  
  if (seq < 1) {
    send_to_char("Sequence number must be >= 1.\r\n", ch);
    return;
  }

  action = gadget_action_number(c_action);

  if (action == GADGET_ACTION_NONE) {
    send_to_char("Action: pull push twist turn turnback move lift press dig\r\n", ch);
    return;
  }

  in = atoi(c_in);

  if (in < -1) {
    send_to_char("Input state must be >= -1\r\n", ch);
    return;
  }

  out = atoi(c_out);

  if (out < -1) {
    send_to_char("Output state must be >= -1\r\n", ch);
    return;
  }

 /* Ok, all parms are right.  Lets go find the gdt... */

  gdt = find_gdt_by_seq(gadget, seq);

 /* New or old? */
 
  if (gdt == NULL) {
    gdt = get_gdt();

    gdt->seq	= seq;

    gdt->cond	= NULL;
    gdt->next	= NULL;

    insert_gdt(gdt, gadget);
  }

 /* Replace int data... */

  gdt->action	= action;
  gdt->in	= in;
  gdt->out	= out;

 /* Copy the string... */

  if (gdt->message != NULL) {
    free_string(gdt->message);
    gdt->message = NULL;
  }

  gdt->message = str_dup(args);

 /* All done... */

  send_to_char("Gadget transition added.\r\n", ch);

  return;
}

void do_gadget_addtc(CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *args) {

  char c_id[MAX_INPUT_LENGTH];
  char c_seq[MAX_INPUT_LENGTH];

  int id;
  int seq; 

  GADGET_DATA *gadget;
  GADGET_TRANS_DATA *gdt;

  ECOND_DATA *new_ec;

 /* Extract gadget id... */

  args = one_argument(args, c_id);
  args = one_argument(args, c_seq);

 /* Rest of args should be the condition... */

  if (args[0] == '\0') {
    redit_gadget(ch, "");
    return;
  } 

 /* Extract the id... */

  id = atoi(c_id);

 /* Now find the gadget... */ 

  gadget = find_gadget_by_id(room, id);

 /* New or old? */

  if (gadget == NULL) {
    send_to_char("No such gadget!\r\n", ch);
    return; 
  }

 /* Now chew through the rst of the parms... */

  seq = atoi(c_seq);
  
  if (seq < 1) {
    send_to_char("Sequence number must be >= 1.\r\n", ch);
    return;
  }

 /* Now find the gdt... */

  gdt = find_gdt_by_seq(gadget, seq);

  if (gdt == NULL) {
    send_to_char("Gadget transition not defined.\r\n", ch);
    return;
  }

 /* Now, fix the condition... */

  if (!str_cmp(args, "none")) {

    while (gdt->cond != NULL) {
      new_ec = gdt->cond->next;
      free_ec(gdt->cond);
      gdt->cond = new_ec;
    }
      
    send_to_char("Conditions removed.\r\n", ch);
    
    return;
  }
 
 /* ..otherwise we go make a new one... */

  new_ec = create_econd(args, ch);
 
  if (new_ec != NULL) {
    new_ec->next = gdt->cond;
    gdt->cond = new_ec;
  }
   
  return;
}

void do_gadget_delete(CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *args) {

  char c_id[MAX_INPUT_LENGTH];

  int id;

  GADGET_DATA *gadget, *last_gadget;

 /* Extract gadget id... */

  args = one_argument(args, c_id);

 /* Rest of args should be the null... */

  if (args[0] != '\0') {
    redit_gadget(ch, "");
    return;
  } 

 /* Extract the id... */

  id = atoi(c_id);

 /* Now find the gadget... */ 

  gadget = find_gadget_by_id(room, id);

 /* Not found is not a problem... */

  if (gadget == NULL) {
    send_to_char("Gadget not defined!\r\n", ch);
    return;
  }

 /* Ok, now we must splice it out of the list... */

  if (room->gadgets == gadget) {
    room->gadgets = gadget->next;
    gadget->next = NULL;
  } else {

    last_gadget = room->gadgets;

    while (last_gadget->next != NULL) {

      if (last_gadget->next == gadget) {
        last_gadget->next = gadget->next;
        gadget->next = NULL;
        break;
      }
     
      last_gadget = last_gadget->next;
    }
  }  

 /* Finally we free the isolated gadget... */

  free_gadget(gadget);

  send_to_char("Gadget deleted.\r\n", ch); 

  return;
}

void do_gadget_deletet(CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *args) {

  char c_id[MAX_INPUT_LENGTH];
  char c_seq[MAX_INPUT_LENGTH];

  int id;
  int seq; 

  GADGET_DATA *gadget;
  GADGET_TRANS_DATA *gdt;

 /* Extract gadget id... */

  args = one_argument(args, c_id);
  args = one_argument(args, c_seq);

 /* Rest of args should be null... */

  if (args[0] != '\0') {
    redit_gadget(ch, "");
    return;
  } 

 /* Extract the id... */

  id = atoi(c_id);

 /* Now find the gadget... */ 

  gadget = find_gadget_by_id(room, id);

 /* New or old? */

  if (gadget == NULL) {
    send_to_char("No such gadget!\r\n", ch);
    return; 
  }

 /* Now chew through the rst of the parms... */

  seq = atoi(c_seq);
  
  if (seq < 1) {
    send_to_char("Sequence number must be >= 1.\r\n", ch);
    return;
  }

 /* Now find the gdt... */

  gdt = find_gdt_by_seq(gadget, seq);

  if (gdt == NULL) {
    send_to_char("Gadget transition not defined.\r\n", ch);
    return;
  }

 /* Ok, go zap it... */

  delete_gdt(gadget, seq);

  send_to_char("Gadget transition deleted.\r\n", ch);

  return;
}

void do_gadget_renum(CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *args) {

  char c_id[MAX_INPUT_LENGTH];

  int id, i;

  GADGET_DATA *gadget;
  GADGET_TRANS_DATA *gdt;

 /* Extract gadget id... */

  args = one_argument(args, c_id);

 /* Rest of args should be the null... */

  if (args[0] != '\0') {
    redit_gadget(ch, "");
    return;
  } 

 /* Extract the id... */

  id = atoi(c_id);

 /* Now find the gadget... */ 

  gadget = find_gadget_by_id(room, id);

 /* Not found is not a problem... */

  if (gadget == NULL) {
    send_to_char("Gadget not defined!\r\n", ch);
    return;
  }

 /* Ok, renum time... */

  i = 10;

  gdt = gadget->gdt;

  while (gdt != NULL) {
 
    gdt->seq = i;

    i += 10;

    gdt = gdt->next; 
  }
 
  send_to_char("Gadget transitions renumbered.\r\n", ch);

  return;
}

REDIT( redit_gadget )
{
    ROOM_INDEX_DATA *pRoom;
    char command[MAX_INPUT_LENGTH];
    char *args;

    EDIT_ROOM(ch, pRoom);

    args = one_argument( argument, command );

    if (command[0] == '\0')
    {
        send_to_char( "Syntax:  gadget list\r\n", ch);
        send_to_char( "         gadget show [id]\r\n", ch);
	send_to_char( "         gadget add [id] [names]\r\n", ch );
	send_to_char( "         gadget addl [id] [room] [id]\r\n", ch );
	send_to_char( "         gadget addt [id] [seq] [action] [in] [out] [msg]\r\n", ch );
	send_to_char( "         gadget addtc [id] [seq] [cond]\r\n", ch );
                send_to_char( "         gadget delete [id]\r\n", ch );
	send_to_char( "         gadget delete [id] [seq]\r\n", ch );
	send_to_char( "         gadget renum [id] [seq]\r\n", ch );
	return FALSE;
    }

   /* Dispatch... */

    if (!str_cmp( command, "list")) {
      do_gadget_list(ch, pRoom, args);
      return TRUE;
    }
 
    if (!str_cmp(command, "show")) {
      do_gadget_show(ch, pRoom, args);
      return TRUE;
    }

    if (!str_cmp(command, "add")) {
      do_gadget_add(ch, pRoom, args);
      return TRUE;
    }

    if (!str_cmp(command, "addl")) {
      do_gadget_addl(ch, pRoom, args);
      return TRUE;
    }

    if (!str_cmp(command, "addt")) {
      do_gadget_addt(ch, pRoom, args);
      return TRUE;
    }

    if (!str_cmp(command, "addtc")) {
      do_gadget_addtc(ch, pRoom, args);
      return TRUE;
    }

    if (!str_cmp(command, "delete")) {
      do_gadget_delete(ch, pRoom, args);
      return TRUE;
    }

    if (!str_cmp(command, "deletet")) {
      do_gadget_deletet(ch, pRoom, args);
      return TRUE;
    }

    if (!str_cmp(command, "renum")) {
      do_gadget_renum(ch, pRoom, args);
      return TRUE;
    }

    redit_gadget(ch, "");

    return FALSE;
 }


OEDIT( oedit_delete )
{
	OBJ_DATA *obj, *obj_next;
	OBJ_INDEX_DATA *pObj;
	RESET_DATA *pReset, *wReset;
	ROOM_INDEX_DATA *pRoom;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int index, rcount, ocount, i, iHash;

	one_argument( argument, arg );

	if ( arg[0] == '\0' ) {
                     EDIT_OBJ(ch, pObj);
                     index = pObj->vnum;
	} else if( is_number( arg ) ) {
	     index = atoi( arg );
	} else {
	     send_to_char( "That is not a number.\r\n", ch );
	     return FALSE;
	}

	pObj = get_obj_index( index );

	if( !pObj )
	{
		send_to_char( "No such object.\r\n", ch );
		return FALSE;
	}

	SET_BIT( pObj->area->area_flags, AREA_CHANGED );

	if( top_vnum_obj == index )
		for( i = 1; i < index; i++ )
			if( get_obj_index( i ) )
				top_vnum_obj = i;


	top_obj_index--;

	/* remove objects */
	ocount = 0;
	for( obj = object_list; obj; obj = obj_next )
	{
		obj_next = obj->next;

		if( obj->pIndexData == pObj )
		{
			extract_obj( obj );
			ocount++;
		}
	}

	/* crush resets */
	rcount = 0;
	for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
		for( pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
		{
			for( pReset = pRoom->reset_first; pReset; pReset = wReset )
			{
				wReset = pReset->next;
				switch( pReset->command )
				{
				case 'O':
				case 'E':
				case 'P':
				case 'G':
					if( ( pReset->arg1 == index ) ||
						( ( pReset->command == 'P' ) && (
					pReset->arg3 == index ) ) )
					{
						unlink_reset( pRoom, pReset );
						free_reset_data( pReset );

						rcount++;
						SET_BIT( pRoom->area->area_flags,
						AREA_CHANGED );

					}
				}
			}
		}
	}

	unlink_obj_index( pObj );

	pObj->area = NULL;
	pObj->vnum = 0;

	free_obj_index( pObj );

	sprintf( buf, "Removed object vnum {C%d{x and"
		" {C%d{x resets.\r\n", index,rcount );

	send_to_char( buf, ch );

	sprintf( buf, "{C%d{x occurences of the object"
		" were extracted from the mud.\r\n", ocount );

	send_to_char( buf, ch );
               	edit_done( ch );
	return TRUE;
}


MEDIT( medit_delete )
{
	CHAR_DATA *wch, *wnext;
	MOB_INDEX_DATA *pMob;
	RESET_DATA *pReset, *wReset;
	ROOM_INDEX_DATA *pRoom;
	char arg[MAX_INPUT_LENGTH];
	int index, mcount, rcount, iHash, i;
	bool foundmob = FALSE;
	bool foundobj = FALSE;

	one_argument( argument, arg );

	if( arg[0] == '\0' )	{
                      EDIT_MOB(ch, pMob);
                      index = pMob->vnum;
	} else if(is_number( arg ) )	{
	      index = atoi( arg );
	} else {
	      send_to_char( "That is not a number.\r\n", ch );
	      return FALSE;
	}

	pMob = get_mob_index( index );

	if( !pMob ) {
		send_to_char( "No such mobile.\r\n", ch );
		return FALSE;
	}

	SET_BIT( pMob->area->area_flags, AREA_CHANGED );

	if( top_vnum_mob == index )
		for( i = 1; i < index; i++ )
			if( get_mob_index( i ) )
				top_vnum_mob = i;

	top_mob_index--;

	/* Now crush all resets and take out mobs while were at it */
	rcount = 0;
	mcount = 0;
	
	for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
		for( pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
		{

			for( wch = pRoom->people; wch; wch = wnext )
			{
				wnext = wch->next_in_room;
				if( wch->pIndexData == pMob )
				{
					extract_char( wch, TRUE );
					mcount++;
				}
			}

			for( pReset = pRoom->reset_first; pReset; pReset = wReset )
			{
				wReset = pReset->next;
				switch( pReset->command )
				{
				case 'M':
					if( pReset->arg1 == index )
					{
						foundmob = TRUE;

						unlink_reset( pRoom, pReset );
						free_reset_data( pReset );

						rcount++;
						SET_BIT( pRoom->area->area_flags,
						AREA_CHANGED );

					}
					else
						foundmob = FALSE;

					break;
				case 'E':
				case 'G':
					if( foundmob )
					{
						foundobj = TRUE;

						unlink_reset( pRoom, pReset );
						free_reset_data( pReset );

						rcount++;
						SET_BIT( pRoom->area->area_flags,
						AREA_CHANGED );

					}
					else
						foundobj = FALSE;

					break;
				case '0':
					foundobj = FALSE;
					break;
				case 'P':
					if( foundobj && foundmob )
					{
						unlink_reset( pRoom, pReset );
						free_reset_data( pReset );

						rcount++;
						SET_BIT( pRoom->area->area_flags,
						AREA_CHANGED );
					}
				}
			}
		}
	}

	unlink_mob_index( pMob );

	pMob->area = NULL;
	pMob->vnum = 0;

	free_mob_index( pMob );

	sprintf_to_char( ch, "Removed mobile vnum {C%d{x and"
                                                     " {C%d{x resets.\r\n", index, rcount );
	sprintf_to_char( ch, "{C%d{x mobiles were extracted"
		                     " from the mud.\r\n",mcount );
               	edit_done( ch );
	return TRUE;
}

REDIT( redit_delete )
{
	ROOM_INDEX_DATA *pRoom, *pRoom2;
	RESET_DATA *pReset;
	EXIT_DATA *ex;
	OBJ_DATA *Obj, *obj_next;
	CHAR_DATA *wch, *wnext;
	EXTRA_DESCR_DATA *pExtra;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int index, i, iHash, rcount, ecount, mcount, ocount, edcount;

	one_argument( argument, arg );

	if ( arg[0] == '\0' ){
                    EDIT_ROOM(ch, pRoom);
                    index = pRoom->vnum;
	} else if( is_number( arg ))	{
	    index = atoi( arg );
	} else {
	    send_to_char( "That is not a number.\r\n", ch );
	    return FALSE;
	}

	pRoom = get_room_index( index );

	if( !pRoom ) {
		send_to_char( "No such room.\r\n", ch );
		return FALSE;
	}

	/* Move the player out of the room. */
	if( ch->in_room->vnum == index )
	{
		send_to_char( "Moving you out of the room"
			" you are deleting.\r\n", ch);
		if( ch->fighting != NULL )
			stop_fighting( ch, TRUE );

		char_from_room( ch );
		char_to_room( ch, get_room_index(2)); 
		ch->was_in_room = ch->in_room;
	}

	SET_BIT( pRoom->area->area_flags, AREA_CHANGED );

	/* Count resets. They are freed by free_room_index. */
	rcount = 0;

	for( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
	{
		rcount++;
	}

	/* Now contents */
	ocount = 0;
	for( Obj = pRoom->contents; Obj; Obj = obj_next )
	{
		obj_next = Obj->next_content;

		extract_obj( Obj );
		ocount++;
	}

	/* Now PCs and Mobs */
	mcount = 0;
	for( wch = pRoom->people; wch; wch = wnext )
	{
		wnext = wch->next_in_room;
		if( IS_NPC( wch ) )
		{
			extract_char( wch, TRUE );
			mcount++;
		}
		else
			{
			send_to_char( "This room is being deleted. Moving" 
				" you somewhere safe.\r\n", ch );
			if( wch->fighting != NULL )
				stop_fighting( wch, TRUE );

			char_from_room( wch );

			char_to_room( wch, get_room_index(3123)); 
			wch->was_in_room = wch->in_room;
		}
	}

	/* unlink all exits to the room. */
	ecount = 0;
	for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )	{
		for( pRoom2 = room_index_hash[iHash]; pRoom2; pRoom2 = pRoom2->next )
		{
			for( i = 0; i <= MAX_DIR; i++ )
			{
				if( !( ex = pRoom2->exit[i] ) )
					continue;

				if( pRoom2 == pRoom )
				{
					/* these are freed by free_room_index */
					ecount++;
					continue;
				}

				if( ex->u1.to_room == pRoom )
				{
					free_exit( pRoom2->exit[i] );
					pRoom2->exit[i] = NULL;
					SET_BIT( pRoom2->area->area_flags, AREA_CHANGED );
					ecount++;
				}
			}
		}
	}

	/* count extra descs. they are freed by free_room_index */
	edcount = 0;
	for ( pExtra = pRoom->extra_descr; pExtra; pExtra = pExtra->next )
	{
		edcount++;
	}

	if( top_vnum_room == index )
		for( i = 1; i < index; i++ )
			if( get_room_index( i ) )
				top_vnum_room = i;

	top_room--;

	unlink_room_index( pRoom );

	pRoom->area = NULL;
	pRoom->vnum = 0;

	free_room_index( pRoom );

	/* Na na na na! Hey Hey Hey, Good Bye! */

	sprintf( buf, "Removed room vnum {C%d{x, %d resets, %d extra "
		"descriptions and %d exits.\r\n", index, rcount, edcount, ecount );
	send_to_char( buf, ch );
	sprintf( buf, "{C%d{x objects and {C%d{x mobiles were extracted "
		"from the room.\r\n", ocount, mcount );
	send_to_char( buf, ch );
               	edit_done( ch );
	return TRUE;
}


/* unlink a given reset from a given room */
void unlink_reset( ROOM_INDEX_DATA *pRoom, RESET_DATA *pReset ) {
	RESET_DATA *prev, *wReset;

	prev = pRoom->reset_first;
	for( wReset = pRoom->reset_first; wReset; wReset = wReset->next ) {
		if( wReset == pReset ) {
			if( pRoom->reset_first == pReset )	{
				pRoom->reset_first = pReset->next;
				if( !pRoom->reset_first ) pRoom->reset_last = NULL;
			} else if( pRoom->reset_last == pReset ) {
				pRoom->reset_last = prev;
				prev->next = NULL;
			} else {
				prev->next = prev->next->next;
                                                }
		}

		prev = wReset;
	}
}


void unlink_obj_index( OBJ_INDEX_DATA *pObj ) {
	int iHash;
	OBJ_INDEX_DATA *iObj, *sObj;

	iHash = pObj->vnum % MAX_KEY_HASH;

	sObj = obj_index_hash[iHash];

	if( sObj->next == NULL ) /* only entry */
		obj_index_hash[iHash] = NULL;
	else if( sObj == pObj ) /* first entry */
		obj_index_hash[iHash] = pObj->next;
	else /* everything else */
	{
		for( iObj = sObj; iObj != NULL; iObj = iObj->next )
		{
			if( iObj == pObj )
			{
				sObj->next = pObj->next;
				break;
			}
			sObj = iObj;
		}
	}
}


void unlink_room_index( ROOM_INDEX_DATA *pRoom ) {
	int iHash;
	ROOM_INDEX_DATA *iRoom, *sRoom;

	iHash = pRoom->vnum % MAX_KEY_HASH;

	sRoom = room_index_hash[iHash];

	if( sRoom->next == NULL ) /* only entry */
		room_index_hash[iHash] = NULL;
	else if( sRoom == pRoom ) /* first entry */
		room_index_hash[iHash] = pRoom->next;
	else /* everything else */
	{
		for( iRoom = sRoom; iRoom != NULL; iRoom = iRoom->next )
		{
			if( iRoom == pRoom )
			{
				sRoom->next = pRoom->next;
				break;
			}
			sRoom = iRoom;
		}
	}
}


void unlink_mob_index( MOB_INDEX_DATA *pMob ) {
	int iHash;
	MOB_INDEX_DATA *iMob, *sMob;

	iHash = pMob->vnum % MAX_KEY_HASH;

	sMob = mob_index_hash[iHash];

	if( sMob->next == NULL ) /* only entry */
		mob_index_hash[iHash] = NULL;
	else if( sMob == pMob ) /* first entry */
		mob_index_hash[iHash] = pMob->next;
	else /* everything else */
	{
		for( iMob = sMob; iMob != NULL; iMob = iMob->next )
		{
			if( iMob == pMob )
			{
				sMob->next = pMob->next;
				break;
			}
			sMob = iMob;
		}
	}
}


MEDIT( medit_macro) {
MOB_INDEX_DATA *pMobIndex;
MOB_SCRIPT *scr;
MOB_SCRIPT_LINE *scr_line;
char command[MAX_INPUT_LENGTH];
char arg[MAX_INPUT_LENGTH];

    EDIT_MOB(ch, pMobIndex);
    argument = one_argument( argument, command );
    argument = one_argument( argument, arg );

    if (command[0] == '\0') {
        send_to_char( "Syntax:\r\n", ch);
        send_to_char( "   macro start [script_id]\r\n", ch);
        send_to_char( "   macro wait [seconds]\r\n", ch);
        send_to_char( "   macro stop\r\n", ch);
        return FALSE;
    }

    if (!str_cmp(command, "start")) {
      int scrn;

      if (ch->pcdata->macro == TRUE) {
          send_to_char( "You're already recording a MACRO.\r\n", ch);
          return FALSE;
      }

      scrn = atoi(arg);
      if (scrn < 1) {
          send_to_char( "Invalid script number.\r\n", ch);
          return FALSE;
      }
      
      if (!pMobIndex->triggers) {
          send_to_char( "Script doesn't exist.\r\n", ch);
          return FALSE;
      }
      if (!pMobIndex->triggers->scripts) {
          send_to_char( "Script doesn't exist.\r\n", ch);
          return FALSE;
      }

      for(scr = pMobIndex->triggers->scripts; scr; scr = scr->next) {
            if (scr->id == scrn) break;
      }
      if (!scr) {
          send_to_char( "Script doesn't exist.\r\n", ch);
          return FALSE;
      }

      ch->pcdata->macro = TRUE;
      ch->pcdata->macro_script = scrn;
      ch->pcdata->macro_line = 0;
      for(scr_line = scr->lines; scr_line; scr_line = scr_line->next) {
           if (scr_line->seq > ch->pcdata->macro_line) {
               ch->pcdata->macro_line = scr_line->seq;
               ch->pcdata->macro_delay = scr_line->delay;
           }
      }

      ch->pcdata->macro_line +=10;
      return TRUE;

    } else if (!str_cmp(command, "wait")) {
      int macro_wait;

      if (ch->pcdata->macro == FALSE) {
          send_to_char( "You're not recording a MACRO.\r\n", ch);
          return FALSE;
      }

      macro_wait = atoi(arg);
      if (macro_wait < 1) {
          send_to_char( "Invalid wait time.\r\n", ch);
          return FALSE;
      }

      ch->pcdata->macro_delay += macro_wait;
      return TRUE;

    } else if (!str_cmp(command, "stop")) {

      if (ch->pcdata->macro == FALSE) {
          send_to_char( "You're not recording a MACRO.\r\n", ch);
          return FALSE;
      }

      ch->pcdata->macro = FALSE;
      ch->pcdata->macro_script = 0;
      ch->pcdata->macro_line = 0;
      ch->pcdata->macro_delay = 0;
      return TRUE;

    } else {
        send_to_char( "Syntax:\r\n", ch);
        send_to_char( "   macro start [script_id]\r\n", ch);
        send_to_char( "   macro wait [seconds]\r\n", ch);
        send_to_char( "   macro stop\r\n", ch);
        return FALSE;
    }
    return FALSE;
}
     


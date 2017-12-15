/*
 * CthulhuMud
 */

#define START_VISION		100
#define MAP_WIDTH		21

short const mapmod[DIR_MAX] =
{
-MAP_WIDTH,
1,
MAP_WIDTH,
-1,
0,
0,
-MAP_WIDTH+1,
MAP_WIDTH+1,
MAP_WIDTH-1,
-MAP_WIDTH-1,
0,
0
};

short const vismod[SECT_MAX]	=
{
    15,		// Inside
    20,		// City
    15,		// Fields
    40,		// Forest
    25,		// Hills
    35,		// Mountains
    25,		// Water - swim 
    25,		// Water - no_swim
    10,		// ? 	
    10,		// Air 
    15,		// Desert 
    20,		// Underground 
    30,		// Swamp 
    35,		// Moor 
    10,		// Space
    50,		// Underwater
    35,		// Small fire
    40,		// Fire
    45,		// Big Fire	
    20,		// Cold
    30,		// Acid
    20,		// Lightning
    20,		// Holy
    20,		// Evil
    50,		// Jungle
    15,		// Path
    10,		// Road
    10		// Plains
};

char* const sectchar[SECT_MAX]	=
{
    "{x.",		// Inside
    "{W.",	// City
    "{g.",		// Fields
    "{g|",		// Forest
    "{G^",		// Hills
    "{x^",		// Mountains
    "{B~",	// Water - swim 
    "{b~",		// Water - no_swim
    "{x.",		// ? 	
    "{C%",	// Air 
    "{y.",		// Desert 
    "{r.",		// Underground 
    "{m|",		// Swamp 
    "{M|",		// Moor 
    "{c%",	// Space
    "{b~",		// Underwater
    "{R*",	// Small fire
    "{R*",	// Fire
    "{R*",	// Big Fire	
    "{C*",	// Cold
    "{G*",	// Acid
    "{Y*",	// Lightning
    "{W+",	// Holy
    "{m-",		// Evil
    "{G|",		// Jungle
    "{x#",		// Path
    "{W#",	// Road
    "{G."		// Plains
};

void 	trace_visibility		(CHAR_DATA* ch);
void 	map_sector		(CHAR_DATA* ch, int vision);
void 	clear_distance		(AREA_DATA *area);
bool 	set_room_distance	(CHAR_DATA *ch, AREA_DATA *area, ROOM_INDEX_DATA *room, short generation);
void	trace_distance		(CHAR_DATA* ch, CHAR_DATA *target, AREA_DATA *area);
void	track			(CHAR_DATA *ch, CHAR_DATA *target);





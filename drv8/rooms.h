/* 
 * Cthulhumud
 */

/* Collect all rooms potentially adjacent to this one... */

ROOM_INDEX_DATA *collect_adjacent(ROOM_INDEX_DATA *room);

/* Collect all rooms in the same area->subarea... */

ROOM_INDEX_DATA *collect_subarea(ROOM_INDEX_DATA *room);

/* Collect all rooms in and adjacent to the same area->subarea... */

ROOM_INDEX_DATA *collect_subarea_plus(ROOM_INDEX_DATA *room);

/* Collect all rooms in an area... */

ROOM_INDEX_DATA *collect_area(ROOM_INDEX_DATA *room);

/* Collect all rooms in and adjacent to an area... */

ROOM_INDEX_DATA *collect_area_plus(ROOM_INDEX_DATA *room);

/* Collect all rooms in a zone... */

ROOM_INDEX_DATA *collect_zone(ROOM_INDEX_DATA *room);

/* Collect all rooms... */

ROOM_INDEX_DATA *collect_all_rooms(ROOM_INDEX_DATA *room);

/* Collect all rooms containing members of a mobs group... */

ROOM_INDEX_DATA *collect_group_rooms(ROOM_INDEX_DATA *room, CHAR_DATA *ch);

/* Collect all rooms within a scope of a room... */

ROOM_INDEX_DATA *collect_rooms(	ROOM_INDEX_DATA *room,
				int		 scope,
				CHAR_DATA	*actor);


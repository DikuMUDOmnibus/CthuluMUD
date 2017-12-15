/*
 * CthulhuMud
 */


// Macros

#define NUM_VNUM(x)	((int) ((x) & 0x0000FFFF))
#define LIB_VNUM(x)	((int) (((x) & 0xFFFF0000) >> 16 ))
#define FUL_VNUM(x,y)	((VNUM) (((((x) & 0x0000FFFF) << 16 ) | ((y) & 0x0000FFFF )))

// Functions

void format_vnum(VNUM vnum, char *outbuf);


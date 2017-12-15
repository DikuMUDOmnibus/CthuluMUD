/*
 * CthulhuMud
 */

/* Foreground colors */

#define RED		"{r"
#define BLUE		"{b"
#define GREEN		"{g"
#define BLACK		"{D"
#define WHITE		"{w"
#define MAGENTA		"{m" /* purple? */
#define YELLOW		"{Y"
#define CYAN		"{c"

/* Background colors */

#define RED_BG			""
#define BLUE_BG			""
#define GREEN_BG		""
#define BLACK_BG		""
#define WHITE_BG		""
#define MAGENTA_BG		""
#define YELLOW_BG		""
#define CYAN_BG			""

/* Other codes */

#define BOLD			""
#define NO_COLOR		"{x" /* Default color */
#define BLINK                   "{&"
#define REVERSE			"{4"
#define TILDE                   "-"


#define COLOR_FOREGROUND 7 /* & with color_t to get foreground color */

typedef enum {	color_white 	= 0, /*  000 */
				color_black		= 1, /*  001 */
				color_red		= 2, /*  010 */
				color_blue		= 3, /*  011 */
				color_green		= 4, /*  100 */
				color_magenta	= 5, /*  101 */
				color_yellow	= 6, /*  110 */
				color_cyan		= 7, /*  111 */
				color_bold      = 8  /* 1000  - to safely AND with the other enums */
			} color_t;



/*
 * CthulhuMud
 */

#define LINE_LENGTH 77

bool format_text(const char *in, char *out);

bool html_tag_to_text(const char *in, char *out);

bool strip_html(const char *in, char *out, bool convert, int max,
                char *font);

bool strip_lf(const char *in, char *out, int max);

#define FONT_CLEAR	"xWcC"

#define FONT_RED	"rRyY"
#define FONT_YELLOW	"yYrR"
#define FONT_GREEN	"gGyY"
#define FONT_BLUE	"bBcC"
#define FONT_CYAN	"cCwW"
#define FONT_MAGENTA	"mMrR"
#define FONT_WHITE	"wWcC"


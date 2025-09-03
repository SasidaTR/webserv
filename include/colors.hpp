#ifndef COLORS_HPP
#define COLORS_HPP

// 0–9 : Reset et styles de base
#define RESET				"\033[0m"
#define BOLD				"\033[1m"
#define FAINT				"\033[2m"
#define ITALIC				"\033[3m"
#define UNDERLINE			"\033[4m"
#define BLINK_SLOW			"\033[5m"
#define BLINK_FAST			"\033[6m"
#define INVERSE				"\033[7m"
#define HIDDEN				"\033[8m"
#define STRIKETHROUGH		"\033[9m"

// 10–19 : Polices (peu supportées)
#define FONT_DEFAULT		"\033[10m"
#define FONT_ALT1			"\033[11m"
#define FONT_ALT2			"\033[12m"
#define FONT_ALT3			"\033[13m"
#define FONT_ALT4			"\033[14m"
#define FONT_ALT5			"\033[15m"
#define FONT_ALT6			"\033[16m"
#define FONT_ALT7			"\033[17m"
#define FONT_ALT8			"\033[18m"
#define FONT_ALT9			"\033[19m"

// 20–29 : Annulation des styles
#define FRAKTUR				"\033[20m"
#define BOLD_OFF			"\033[21m"
#define NORMAL_INTENSITY	"\033[22m"
#define ITALIC_OFF			"\033[23m"
#define UNDERLINE_OFF		"\033[24m"
#define BLINK_OFF			"\033[25m"
#define RESERVED_26			"\033[26m"
#define INVERSE_OFF			"\033[27m"
#define HIDDEN_OFF			"\033[28m"
#define STRIKETHROUGH_OFF	"\033[29m"

// 30–39 : Couleurs de texte
#define BLACK				"\033[30m"
#define RED					"\033[31m"
#define GREEN				"\033[32m"
#define YELLOW				"\033[33m"
#define BLUE				"\033[34m"
#define MAGENTA				"\033[35m"
#define CYAN				"\033[36m"
#define WHITE				"\033[37m"

#define DEFAULT_FG			"\033[39m"

// 40–49 : Couleurs de fond
#define BG_BLACK			"\033[40m"
#define BG_RED				"\033[41m"
#define BG_GREEN			"\033[42m"
#define BG_YELLOW			"\033[43m"
#define BG_BLUE				"\033[44m"
#define BG_MAGENTA			"\033[45m"
#define BG_CYAN				"\033[46m"
#define BG_WHITE			"\033[47m"

#define DEFAULT_BG			"\033[49m"

#endif

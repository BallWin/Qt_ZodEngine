#ifndef ZFONT_H
#define ZFONT_H



#include "qzod_dnseparate_global.h"
#include "zsdl.h"

// -- FONT -------------------------------------
#ifndef DEF_RC_FOLDER_FONT
    #define DEF_RC_FOLDER_FONT "assets/fonts"
#endif

#define MAX_CHARACTERS 255
enum font_type
{
    BIG_WHITE_FONT, SMALL_WHITE_FONT, GREEN_BUILDING_FONT,
    LOADING_WHITE_FONT, YELLOW_MENU_FONT,
    MAX_FONT_TYPES
};

const string font_type_string[MAX_FONT_TYPES] =
{
    "big_white", "small_white", "green_building", "loading_white",
    "yellow_menu"
};

class QZOD_DNSEPARATESHARED_EXPORT ZFont
{
	public:
		ZFont();

		void Init();
		void SetType(int type_);
		SDL_Surface *Render(const char *message);
	private:
		int finished_init;
		int type;

		SDL_Surface *char_img[MAX_CHARACTERS];
};

#endif

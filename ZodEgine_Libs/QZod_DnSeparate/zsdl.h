#ifndef ZSDL_H
#define ZSDL_H

#include "qzod_dnseparate_global.h"

#include "common.h"
using namespace COMMON;
#include "SDL_rotozoom.h"
#include "zsdl_opengl.h"

enum sound_setting
{
	SOUND_0, SOUND_25, SOUND_50, SOUND_75, SOUND_100, MAX_SOUND_SETTINGS
};

const string sound_setting_string[MAX_SOUND_SETTINGS] = 
{
	"0%", "25%", "50%", "75%", "100%"
};

class QZOD_DNSEPARATESHARED_EXPORT SDL_RotoZoomSurface
{
public:
	SDL_RotoZoomSurface();

	void LoadBaseImage(string filename);
	SDL_Surface *GetImage(int angle, double zoom);
	SDL_Surface *CreateImage(int angle, double zoom);

private:
	SDL_Surface *base_surface;
	SDL_Surface *the_surface[360][100];
};

class QZOD_DNSEPARATESHARED_EXPORT SDL_ZoomSurface
{
public:
	SDL_ZoomSurface();

	void LoadBaseImage(string filename);
	SDL_Surface *GetImage(double zoom);
	SDL_Surface *CreateImage(double zoom);

private:
	SDL_Surface *base_surface;
	SDL_Surface *zoom_surface[100];
};

class QZOD_DNSEPARATESHARED_EXPORT SDL_RotateSurface
{
public:
	SDL_RotateSurface();

	void LoadBaseImage(string filename);
	SDL_Surface *GetImage(int angle);

private:
	SDL_Surface *base_surface;
	SDL_Surface *rotated_surface[360];
};

QZOD_DNSEPARATESHARED_EXPORT SDL_Surface *ZSDL_ConvertImage(SDL_Surface *src);
QZOD_DNSEPARATESHARED_EXPORT SDL_Surface *ZSDL_IMG_Load(string filename);
QZOD_DNSEPARATESHARED_EXPORT SDL_Surface *IMG_Load_Error(string filename);
QZOD_DNSEPARATESHARED_EXPORT Mix_Music *MUS_Load_Error(string filename);
QZOD_DNSEPARATESHARED_EXPORT Mix_Chunk *MIX_Load_Error(string filename);
QZOD_DNSEPARATESHARED_EXPORT SDL_Surface *CopyImage(SDL_Surface *original);
QZOD_DNSEPARATESHARED_EXPORT SDL_Surface *CopyImageShifted(SDL_Surface *original, int x, int y);
QZOD_DNSEPARATESHARED_EXPORT void put32pixel(SDL_Surface *surface, int x, int y, SDL_Color color);
QZOD_DNSEPARATESHARED_EXPORT SDL_Color get32pixel(SDL_Surface *surface, int x, int y);
QZOD_DNSEPARATESHARED_EXPORT int ZSDL_PlayMusic(Mix_Music *music, int eh);
QZOD_DNSEPARATESHARED_EXPORT int ZMix_PlayChannel(int ch, Mix_Chunk *wav, int repeat);
QZOD_DNSEPARATESHARED_EXPORT void ZSDL_SetMusicOn(bool iton);
QZOD_DNSEPARATESHARED_EXPORT void draw_box(SDL_Surface *surface, SDL_Rect dim, SDL_Color color, int max_x, int max_y);
QZOD_DNSEPARATESHARED_EXPORT void draw_selection_box(SDL_Surface *surface, SDL_Rect dim, SDL_Color color, int max_x, int max_y);
QZOD_DNSEPARATESHARED_EXPORT int AngleFromLoc(float dx, float dy);
QZOD_DNSEPARATESHARED_EXPORT SDL_Surface *ZSDL_NewSurface(int w, int h);
QZOD_DNSEPARATESHARED_EXPORT void ZSDL_ModifyBlack(SDL_Surface *surface);
QZOD_DNSEPARATESHARED_EXPORT void ZSDL_BlitSurface(SDL_Surface *src, int fx, int fy, int fw, int fh, SDL_Surface *dest, int x, int y);
QZOD_DNSEPARATESHARED_EXPORT void ZSDL_BlitHitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, bool render_hit = true);
QZOD_DNSEPARATESHARED_EXPORT void ZSDL_BlitTileSurface(SDL_Surface *src, int fx, int fy, SDL_Surface *dest, int x, int y);
QZOD_DNSEPARATESHARED_EXPORT void ZSDL_FreeSurface(SDL_Surface *&surface);
QZOD_DNSEPARATESHARED_EXPORT void ZSDL_Quit();




#endif

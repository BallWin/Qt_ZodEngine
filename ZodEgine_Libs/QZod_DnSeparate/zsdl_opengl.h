#ifndef ZSDL_OPENGL_H
#define ZSDL_OPENGL_H


#include <QtGlobal>
// TODO Qt global
#ifdef Q_OS_WIN              //if windows
#include <windows.h>		 //win for Sleep(1000)
#endif

#ifndef DISABLE_OPENGL
#include <SDL/SDL_opengl.h>
#endif
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "SDL_rotozoom.h"

#include <string>

#include "qzod_dnseparate_global.h"

#include "constants.h"
#include "common.h"

using namespace std;
using namespace COMMON;



#ifdef __cplusplus
extern "C" {
#endif

class QZOD_DNSEPARATESHARED_EXPORT ZSDL_Surface
{
public:
	ZSDL_Surface();
	~ZSDL_Surface();

	static void SetUseOpenGL(bool use_opengl_);
	static void SetMainSoftwareSurface(SDL_Surface *screen_);
	static void SetScreenDimensions(int w_, int h_);
    static void ZSDL_FillRect(SDL_Rect *dstrect, Uint8 r, Uint8 g, Uint8 b, ZSDL_Surface *dst = nullptr);
	static void SetMapPlace(int x, int y);
	static void SetHasHud(bool has_hud_);
	static void GetScreenDimensions(int &w_, int &h_) { w_ = screen_w; h_ = screen_h; }

	static int GetMapBlitInfo(SDL_Surface *src, int x, int y, SDL_Rect &from_rect, SDL_Rect &to_rect);

	void Unload();
	void LoadBaseImage(string filename);
	void LoadBaseImage(SDL_Surface *sdl_surface_, bool delete_surface = true);
	void LoadNewSurface(int w, int h);
	void UseDisplayFormat();
	void MakeAlphable();
	SDL_Surface *GetBaseSurface();

	void SetSize(float size_);
	void SetAngle(float angle_);
    void SetAlpha(uint8_t  alpha_); // !!TODO !! char alpha_);

	void RenderSurface(int x, int y, bool render_hit = false, bool about_center = false);
	void RenderSurfaceHorzRepeat(int x, int y, int w_total, bool render_hit = false);
	void RenderSurfaceVertRepeat(int x, int y, int h_total, bool render_hit = false);
	void RenderSurfaceAreaRepeat(int x, int y, int w, int h, bool render_hit = false);
    void BlitSurface(SDL_Rect *srcrect, SDL_Rect *dstrect, ZSDL_Surface *dst = nullptr);
	void BlitSurface(int fx, int fy, int fw, int fh, ZSDL_Surface *dst, int x, int y);
	void BlitSurface(ZSDL_Surface *dst, int x, int y);
    void BlitHitSurface(SDL_Rect *srcrect, SDL_Rect *dstrect, ZSDL_Surface *dst = nullptr, bool render_hit = false);

	void BlitOnToMe(SDL_Rect *srcrect, SDL_Rect *dstrect, SDL_Surface *src);
    void FillRectOnToMe(SDL_Rect *dstrect, Uint8 r, Uint8 g, Uint8 b);

	bool WillRenderOnScreen(int x, int y, bool about_center);

    // TODO!!! ZSDL_ModifyBlack in zsdl.h, zsdl.h include zsdl-opengl!
    void SDL_ModifyBlack(SDL_Surface *surface);

	//operator overloads
	ZSDL_Surface& operator=(const ZSDL_Surface &rhs);
	ZSDL_Surface& operator=(SDL_Surface *rhs);
private:
	static bool use_opengl;
	static SDL_Surface *screen;
	static int screen_w;
	static int screen_h;
	static int map_place_x;
	static int map_place_y;
	static bool has_hud;

	bool LoadGLtexture();
	bool LoadRotoZoomSurface();

	string image_filename;
	SDL_Surface *sdl_surface;
	SDL_Surface *sdl_rotozoom;
#ifndef DISABLE_OPENGL
	GLuint gl_texture;
#endif
	bool gl_texture_loaded;
	bool rotozoom_loaded;

	float size, angle;
    // TODO !!  change type, char alpha;
    uint8_t alpha;
};

void InitOpenGL();
void ResetOpenGLViewPort(int width, int height);
inline void ZSDL_FillRect(SDL_Rect *dstrect, Uint8 r, Uint8 g, Uint8 b, ZSDL_Surface *dst = nullptr)
	{ ZSDL_Surface::ZSDL_FillRect(dstrect, r, g, b, dst); }


#ifdef __cplusplus
}
#endif

#endif

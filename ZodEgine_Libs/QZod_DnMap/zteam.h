#ifndef ZTEAM_H
#define ZTEAM_H

#include "qzod_dnmap_global.h"

#include <lib_qZod_DnSeparate/constants.h>
#include <lib_qZod_DnSeparate/zsdl.h>
#include <lib_qZod_DnSeparate/zsdl_opengl.h>





#define ZTEAM_BASE_TEAM RED_TEAM

extern SDL_Color team_color[MAX_TEAM_TYPES];

#define ZTEAM_PALETTE_MAX 16

class QZOD_DNMAPSHARED_EXPORT ZTeam_Palette
{
public:
	bool LoadSurfacePalette(SDL_Surface *surface);
	bool SaveSurfacePalette(string filename);
	bool AddColor(SDL_Color &bc, SDL_Color &rc);
	bool GetReplacement(Uint8 &r, Uint8 &g, Uint8 &b, SDL_Color &c);

	//vector<SDL_Color> base_color;
	//vector<SDL_Color> replace_color;
	SDL_Color base_color[ZTEAM_PALETTE_MAX];
	SDL_Color replace_color[ZTEAM_PALETTE_MAX];
};

class QZOD_DNMAPSHARED_EXPORT ZTeam
{
public:
	static void Init();
	static void Setup_team_color();

	static void LoadPalette(int team);
	static void SavePalette(int team);
	static void SaveAllPalettes();
	static void AppendPalette(int team, SDL_Surface *base_version, SDL_Surface *this_version);
	static void LoadZSurface(int team, ZSDL_Surface &bv, ZSDL_Surface &rv, string filename);

	static SDL_Surface *Make(int team, SDL_Surface *base_version);

private:
	static ZTeam_Palette team_palette[MAX_TEAM_TYPES];
};

#endif

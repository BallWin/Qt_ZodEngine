#ifndef ETANKDIRT_H
#define ETANKDIRT_H

#include "qzod_effect_old.h"

class QZOD_DNEFFECTSHARED_EXPORT tank_dirt_graphics
{
public:
	tank_dirt_graphics() { dirts = i_max = 0; }

	void LoadGraphics(int palette);

	ZSDL_Surface tank_dirt[2][6];

	int dirts;
	int i_max;
};

class QZOD_DNEFFECTSHARED_EXPORT ETankDirt : ZEffect
{
public:
	ETankDirt(ZTime *ztime_, int cx_, int cy_, int palette_, bool do_pre_render_);

	static void Init();

	void Process();
	void DoRender(ZMap &zmap, SDL_Surface *dest) { if(!do_pre_render) TheRender(zmap, dest); }
	void DoPreRender(ZMap &zmap, SDL_Surface *dest)  { if(do_pre_render) TheRender(zmap, dest); }
	void TheRender(ZMap &zmap, SDL_Surface *dest);

private:
	static bool finished_init;
	static tank_dirt_graphics td_graphics[MAX_PLANET_TYPES];

	int palette;

	int cx, cy;
	int dirt_i;
	int ni;

	bool do_pre_render;

	double next_ni_time;
};

#endif

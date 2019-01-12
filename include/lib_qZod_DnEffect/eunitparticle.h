#ifndef EUNITPARTICLE_H
#define EUNITPARTICLE_H

#include "qzod_effect_old.h"

class QZOD_DNEFFECTSHARED_EXPORT EUnitParticle : ZEffect
{
	public:
		EUnitParticle(ZTime *ztime_, int x_, int y_, int max_horz = 25, int max_vert = 25);

		static void Init();

		void Process();
		void DoRender(ZMap &zmap, SDL_Surface *dest);
	private:
		static bool finished_init;
		static ZSDL_Surface base_img[20];

		int x, y;
		int ex, ey;
		int sx, sy;
		double dx, dy;
		double next_process_time;
		double init_time, final_time;
		double rise;

		int render_i;
		int max_render_i;
		double size;
};

#endif

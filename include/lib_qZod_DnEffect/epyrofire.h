#ifndef EPYROFIRE_H
#define EPYROFIRE_H

#include "qzod_effect_old.h"

class QZOD_DNEFFECTSHARED_EXPORT EPyroFire : ZEffect
{
	public:
		EPyroFire(ZTime *ztime_, int x_, int y_);

		static void Init();

		void Process();
		void DoRender(ZMap &zmap, SDL_Surface *dest);
	private:
		static bool finished_init;
		static ZSDL_Surface fire_img[5][6];

		int x, y;
		double next_process_time;

		int fire_i, fire_j;
};

#endif

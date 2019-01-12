#ifndef ELIGHTINITFIRE_H
#define ELIGHTINITFIRE_H

#include "qzod_effect_old.h"

class QZOD_DNEFFECTSHARED_EXPORT ELightInitFire : ZEffect
{
	public:
		ELightInitFire(ZTime *ztime_, int x_, int y_);

		static void Init();

		void Process();
		void DoRender(ZMap &zmap, SDL_Surface *dest);
	private:
		static bool finished_init;
		static ZSDL_Surface render_img[4];

		int x, y;
		double next_process_time;

		int render_i;
};

#endif

#ifndef ETANKSPARK_H
#define ETANKSPARK_H

#include "qzod_effect_old.h"

class QZOD_DNEFFECTSHARED_EXPORT ETankSpark : ZEffect
{
public:
	ETankSpark(ZTime *ztime_, int cx_, int cy_, int direction_);

	static void Init();

	void Process();
	void DoPreRender(ZMap &zmap, SDL_Surface *dest);
private:
	static bool finished_init;
	static ZSDL_Surface tank_spark[6];

	void SetCoords(int cx_, int cy_, int dir_);

	int cx, cy;
	int direction;

	int ni_max;
	int ni_i;
	int ni;

	double next_ni_time;
};

#endif

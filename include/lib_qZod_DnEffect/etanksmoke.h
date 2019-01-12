#ifndef ETANKSMOKE_H
#define ETANKSMOKE_H

#include "qzod_effect_old.h"

class QZOD_DNEFFECTSHARED_EXPORT ETankSmoke : ZEffect
{
public:
	ETankSmoke(ZTime *ztime_, int cx_, int cy_, int direction_, bool do_spark_);

	static void Init();

	void Process();
	void DoPreRender(ZMap &zmap, SDL_Surface *dest);

	static void SetCoords(int cx_, int cy_, int dir_, int &x_, int &y_);
private:
	static bool finished_init;
	static ZSDL_Surface tank_smoke[MAX_ANGLE_TYPES][7];
	static ZSDL_Surface tank_spark[MAX_ANGLE_TYPES][4];

	int cx, cy;
	int ni;
	int direction;
	int x, y;
	bool do_spark;

	double next_ni_time;
};

#endif

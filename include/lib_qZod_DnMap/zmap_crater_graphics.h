#ifndef ZMAP_CRATER_GRAPHICS_H
#define ZMAP_CRATER_GRAPHICS_H

#include "qzod_dnmap_global.h"

#include <lib_qZod_DnSeparate/constants.h>
#include <lib_qZod_DnSeparate/zsdl.h>
#include <lib_qZod_DnSeparate/zsdl_opengl.h>

//#include <constants.h>
//#include <zsdl.h>
//#include <zsdl_opengl.h>

#define MAX_KNOWN_CRATER_TYPES 7
#define MAX_KNOWN_CRATER_N 7

class QZOD_DNMAPSHARED_EXPORT ZMapCraterGraphics
{
public:
	ZMapCraterGraphics() {}

	static void Init();

	static bool CraterExists(bool is_small, int palette, int crater_type);
	static ZSDL_Surface &RandomCrater(bool is_small, int palette, int crater_type);
//private:
	static bool finished_init;

	static ZSDL_Surface crater_small[MAX_PLANET_TYPES][MAX_KNOWN_CRATER_TYPES][MAX_KNOWN_CRATER_N];
	static ZSDL_Surface crater_large[MAX_PLANET_TYPES][MAX_KNOWN_CRATER_TYPES][MAX_KNOWN_CRATER_N];

	static int crater_types[MAX_PLANET_TYPES];
	static int crater_small_n[MAX_PLANET_TYPES][MAX_KNOWN_CRATER_TYPES];
	static int crater_large_n[MAX_PLANET_TYPES][MAX_KNOWN_CRATER_TYPES];
};

#endif

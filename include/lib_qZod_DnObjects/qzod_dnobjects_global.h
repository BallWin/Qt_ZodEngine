#ifndef QZOD_DNOBJECTS_GLOBAL_H
#define QZOD_DNOBJECTS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QZOD_DNOBJECTS_LIBRARY)
#  define QZOD_DNOBJECTSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QZOD_DNOBJECTSSHARED_EXPORT Q_DECL_IMPORT
#endif

// --- C++ ----------------------------------------
#include <cstdio> //TODO stdio.h>
#include <string>
#include <vector>
#include <math.h>
#include <time.h>
using namespace std;

// --- SDL ----------------------------------------
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

// --- ZodEngine libs ------------------------------
#include <lib_qZod_DnSeparate/constants.h>
#include <lib_qZod_DnSeparate/common.h>
#include <lib_qZod_DnSeparate/zsdl.h>
#include <lib_qZod_DnSeparate/zsdl_opengl.h>
#include <lib_qZod_DnSeparate/zfont_engine.h>
#include <lib_qZod_DnSeparate/SDL_rotozoom.h>
#include <lib_qZod_DnSeparate/zencrypt_aes.h>
#include <lib_qZod_DnSeparate/zmysql.h>
#include <lib_qZod_DnSeparate/zpsettings.h>
#include <lib_qZod_DnSeparate/zdamagemissile.h>
#include <lib_qZod_DnSeparate/ztime.h>
#include <lib_qZod_DnSeparate/event_handler.h>

#include <lib_qZod_DnMap/qzod_map.h>
#include <lib_qZod_DnMap/finding/zpath_finding_old.h>
#include <lib_qZod_DnMap/zteam.h>


#include <lib_qZod_DnSettings/qzod_settings_old.h>
#include <lib_qZod_DnSettings/zbuildlist.h>

#include <lib_qZod_DnSoundEngine/qzod_soundengine_old.h>

#include <lib_qZod_DnEffect/zod_effect.hpp>





using namespace COMMON;



#endif // QZOD_DNOBJECTS_GLOBAL_H

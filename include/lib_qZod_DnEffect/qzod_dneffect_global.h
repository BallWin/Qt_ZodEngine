#ifndef QZOD_DNEFFECT_GLOBAL_H
#define QZOD_DNEFFECT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QZOD_DNEFFECT_LIBRARY)
#  define QZOD_DNEFFECTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QZOD_DNEFFECTSHARED_EXPORT Q_DECL_IMPORT
#endif

#include <lib_qZod_DnSeparate/constants.h>
#include <lib_qZod_DnSeparate/common.h>
#include <lib_qZod_DnSeparate/zsdl_opengl.h>
#include <lib_qZod_DnSeparate/zsdl.h>
#include <lib_qZod_DnSeparate/ztime.h>
#include <lib_qZod_DnMap/qzod_map.h>
#include <lib_qZod_DnSoundEngine/qzod_soundengine_old.h>
#include <lib_qZod_DnSettings/qzod_settings_old.h>

#include <math.h>

using namespace COMMON;
using namespace std;



#endif // QZOD_DNEFFECT_GLOBAL_H

#ifndef QZOD_DNGUI_GLOBAL_H
#define QZOD_DNGUI_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QZOD_DNGUI_LIBRARY)
#  define QZOD_DNGUISHARED_EXPORT Q_DECL_EXPORT
#else
#  define QZOD_DNGUISHARED_EXPORT Q_DECL_IMPORT
#endif

#include <string>
#include <cstdlib>
#include <algorithm>
using namespace std;

#include <lib_qZod_DnSeparate/zsdl.h>
#include <lib_qZod_DnSeparate/zsdl_opengl.h>
#include <lib_qZod_DnSeparate/constants.h>
#include <lib_qZod_DnSeparate/common.h>
#include <lib_qZod_DnSeparate/ztime.h>
#include <lib_qZod_DnSeparate/zfont_engine.h>
#include <lib_qZod_DnMap/qzod_map.h>
using namespace COMMON;

#endif // QZOD_DNGUI_GLOBAL_H

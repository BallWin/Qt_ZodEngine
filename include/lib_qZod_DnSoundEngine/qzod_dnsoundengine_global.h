#ifndef QZOD_DNSOUNDENGINE_GLOBAL_H
#define QZOD_DNSOUNDENGINE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QZOD_DNSOUNDENGINE_LIBRARY)
#  define QZOD_DNSOUNDENGINESHARED_EXPORT Q_DECL_EXPORT
#else
#  define QZOD_DNSOUNDENGINESHARED_EXPORT Q_DECL_IMPORT
#endif

#include <string>

#include <lib_qZod_DnMap/qzod_map.h>
#include <lib_qZod_DnSeparate/zsdl.h>
#include <lib_qZod_DnSeparate/common.h>
#include <lib_qZod_DnSeparate/zfont_engine.h>



using namespace COMMON;



#endif // QZOD_DNSOUNDENGINE_GLOBAL_H

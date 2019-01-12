#ifndef QZOD_DNSETTINGS_GLOBAL_H
#define QZOD_DNSETTINGS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QZOD_DNSETTINGS_LIBRARY)
#  define QZOD_DNSETTINGSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QZOD_DNSETTINGSSHARED_EXPORT Q_DECL_IMPORT
#endif


#include <vector>
#include <cstdio> // TODO stdio.h>
#include <string>

#include <lib_qZod_DnSeparate/constants.h>
#include <lib_qZod_DnSeparate/common.h>
#include <lib_qZod_DnMap/qzod_map.h>

using namespace COMMON;
using namespace std;



#endif // QZOD_DNSETTINGS_GLOBAL_H

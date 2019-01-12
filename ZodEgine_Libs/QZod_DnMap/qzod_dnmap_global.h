#ifndef QZOD_DNMAP_GLOBAL_H
#define QZOD_DNMAP_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QZOD_DNMAP_LIBRARY)
#  define QZOD_DNMAPSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QZOD_DNMAPSHARED_EXPORT Q_DECL_IMPORT
#endif
#include <cstdio>
#include <string>
#include <math.h>
#include <memory>
#include <iostream>
#include <vector>
#include <cstdlib>
using namespace std;



#endif // QZOD_DNMAP_GLOBAL_H

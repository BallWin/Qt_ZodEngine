#ifndef QZOD_DNSEPARATE_GLOBAL_H
#define QZOD_DNSEPARATE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QZOD_DNSEPARATE_LIBRARY)
#  define QZOD_DNSEPARATESHARED_EXPORT Q_DECL_EXPORT
#else
#  define QZOD_DNSEPARATESHARED_EXPORT Q_DECL_IMPORT
#endif

#include <errno.h>
#include <string>
#include <vector>
#include <ctype.h>
#include <math.h>
//#include <string.h>


#include <iostream>       // std::cout
#include <locale>         // std::locale, std::tolower

#include <QtGlobal>

#ifdef Q_OS_WIN
#include <windows.h>
#include <time.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>
#endif
using namespace std;


#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>




//namespace COMMON {}





#endif // QZOD_DNSEPARATE_GLOBAL_H

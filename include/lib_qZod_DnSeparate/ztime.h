#ifndef ZTIME_H
#define ZTIME_H

//#include <string>
//#include <vector>
//#include <stdlib.h>
#include "qzod_dnseparate_global.h"
#include "common.h"
using namespace COMMON;
// =========================================================
// *********************************************************
// =========================================================
#ifdef Q_OS_UNIX
#include <sys/time.h>
#endif


class QZOD_DNSEPARATESHARED_EXPORT ZTime
{
public:
    ZTime();

    void UpdateTime();
    void Pause();
    void Resume();
    inline bool IsPaused() { return paused; }
    void SetGameSpeed(double new_speed);
    inline double GameSpeed() { return game_speed; }


    bool paused{};
    double game_speed{};
    double ztime{};
    double last_change_front_time{};
    double last_change_back_time{};
};

#endif // ZTIME_H

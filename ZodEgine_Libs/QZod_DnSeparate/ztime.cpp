#include "ztime.h"


ZTime::ZTime()
    : paused(false), game_speed(1.)
{
}

void ZTime::UpdateTime()
{
    //if(!paused) ztime = current_time() - lost_time;

    if(!paused) ztime = last_change_front_time + ((current_time() - last_change_back_time) * game_speed);
}

void ZTime::Pause()
{
    if(paused) return;

    paused = true;

    //pause_time = current_time();

    double the_time = current_time();
    last_change_front_time = last_change_front_time + ((the_time - last_change_back_time) * game_speed);
    last_change_back_time = the_time;
}

void ZTime::Resume()
{
    if(!paused) return;

    paused = false;

    //lost_time += current_time() - pause_time;

    double the_time = current_time();
    //last_change_front_time = last_change_front_time + ((the_time - last_change_back_time) * 0);
    last_change_back_time = the_time;
}

void ZTime::SetGameSpeed(double new_speed)
{
    if(new_speed < 0) new_speed = 0;

    if(!paused)
    {
        double the_time = current_time();
        last_change_front_time = last_change_front_time + ((the_time - last_change_back_time) * game_speed);
        last_change_back_time = the_time;
    }

    game_speed = new_speed;
}

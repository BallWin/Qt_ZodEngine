#ifndef ZDAMGEMISSILE_H
#define ZDAMGEMISSILE_H


#include "qzod_dnseparate_global.h"
#include "common.h"
using namespace COMMON;

class QZOD_DNSEPARATESHARED_EXPORT damage_missile
{
public:
    damage_missile(){}
	damage_missile(int x_, int y_, int damage_, int radius_, double explode_time_)
	{
		x = x_;
		y = y_;
		damage = damage_;
		radius = radius_;
		explode_time = explode_time_;
	}

	void CalcExplodeTimeTo(int sx, int sy, int missile_speed, double the_time)
	{
		float mag;
		int dx, dy;

		dx = x - sx;
		dy = y - sy;
        mag = static_cast<float>(sqrt(dx * dx + dy * dy)); // TODO static_cast

        explode_time = the_time + (static_cast<double>(mag) / missile_speed);
	}

	int x, y;
	int damage;
	int team;
	int radius;
	double explode_time;
	int attacker_ref_id;
	bool attack_player_given;
	int target_ref_id;
};

#endif

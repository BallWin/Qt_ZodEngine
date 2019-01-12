#ifndef ZMUSIC_ENGINE_H
#define ZMUSIC_ENGINE_H


#include "qzod_dnobjects_global.h"

#include "zobject.h"

enum music_danger_level
{
	M_CALM, M_ATTACKING, M_FORT, M_MAX_DANGER_LEVELS
};

class QZOD_DNOBJECTSSHARED_EXPORT d_level_start_info
{
public:
	d_level_start_info() {}
	d_level_start_info(double position_, double length_) 
	{
		position = position_;
		length = length_;
	}

	double position;
	double length;
};

class QZOD_DNOBJECTSSHARED_EXPORT ZMusicEngine
{
public:
	ZMusicEngine() {}

	static void Init();

	static void SetMusicOn(bool music_on_) { sound_system_on = music_on_; }
	static void PlaySplashMusic();
	static void PlayPlanetMusic(int p_type_);

	static void Process(vector<ZObject*> &object_list, ZMap &tmap, int our_team, int fort_ref_id);
	static void SetDangerLevel(int d_level_);
	static void ResetMusic();
	static int GetDangerLevel() { return d_level; }

private:
	static void InitDLevelStarts();

	static bool sound_system_on;

	static Mix_Music *splash_music;
	static Mix_Music *planet_music[MAX_PLANET_TYPES];

	static bool playing_planet_music;
	static int d_level;
	static int p_type;
	static vector<d_level_start_info> d_level_start[MAX_PLANET_TYPES][M_MAX_DANGER_LEVELS];

	static bool do_next_reset;
	static double next_reset_time;
	static double next_change_d_level_time;

};

#endif

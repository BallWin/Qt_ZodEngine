#ifndef RPSYCHO_H
#define RPSYCHO_H

#include "zrobot.h"

class QZOD_DNOBJECTSSHARED_EXPORT RPsycho : public ZRobot
{
	public:
		RPsycho(ZTime *ztime_, ZSettings *zsettings_ = nullptr);
		
		static void Init();
		
// 		SDL_Surface *GetRender();
		void DoRender(ZMap &the_map, SDL_Surface *dest, int shift_x = 0, int shift_y = 0);
		void DoAfterEffects(ZMap &the_map, SDL_Surface *dest, int shift_x, int shift_y);
		int Process();
		void PlaySelectedWav();
		void PlaySelectedAnim(ZPortrait &portrait);
	private:
		static ZSDL_Surface fire[MAX_TEAM_TYPES][MAX_ANGLE_TYPES][2];
};

#endif

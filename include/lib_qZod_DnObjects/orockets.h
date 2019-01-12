#ifndef OROCKETS_H
#define OROCKETS_H

#include "zobject.h"

class QZOD_DNOBJECTSSHARED_EXPORT ORockets : public ZObject
{
	public:
		ORockets(ZTime *ztime_, ZSettings *zsettings_ = nullptr);
		
		static void Init();

		void DoRender(ZMap &the_map, SDL_Surface *dest, int shift_x = 0, int shift_y = 0);
		int Process();

		void SetOwner(team_type owner_);
	private:
		static ZSDL_Surface render_img;
};

#endif

#ifndef OFLAG_H
#define OFLAG_H

#include "zobject.h"

class QZOD_DNOBJECTSSHARED_EXPORT OFlag : public ZObject
{
	public:
		OFlag(ZTime *ztime_, ZSettings *zsettings_ = nullptr);
		
		static void Init();
		
// 		SDL_Surface *GetRender();
		void DoRender(ZMap &the_map, SDL_Surface *dest, int shift_x = 0, int shift_y = 0);
		int Process();
		void SetZone(map_zone_info *the_zone, ZMap &zmap, vector<ZObject*> &object_list);
		vector<ZObject*> &GetLinkedObjects();
		map_zone_info *GetLinkedZone();
		bool HasRadar();
	private:
		int flag_i;
		
		static ZSDL_Surface flag_img[MAX_TEAM_TYPES][4];
		
		vector<ZObject*> connected_object_list;
};

#endif

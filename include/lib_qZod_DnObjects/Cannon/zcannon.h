#ifndef ZCANNON_H
#define ZCANNON_H

#include "../zobject.h"

class QZOD_DNOBJECTSSHARED_EXPORT ZCannon : public ZObject
{
	public:
		ZCannon(ZTime *ztime_, ZSettings *zsettings_ = nullptr);
		
		static void Init();

		void SetAttackObject(ZObject *obj);
		virtual bool CanEjectDrivers();
		virtual void SetInitialDrivers();
		virtual bool CanSetWaypoints()  { return true; }
		void SetEjectableCannon(bool ejectable_);
		virtual bool CanBeSniped();
		
	protected:
		
		static ZSDL_Surface init_place[3];

		bool ejectable;
};

#endif

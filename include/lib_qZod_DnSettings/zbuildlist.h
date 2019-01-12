#ifndef ZBUILDLIST_H
#define ZBUILDLIST_H



#include "qzod_dnsettings_global.h"
#include "qzod_settings_old.h"



class QZOD_DNSETTINGSSHARED_EXPORT buildlist_object
{
public:
	buildlist_object() { clear(); }
	buildlist_object(unsigned char ot_, unsigned char oid_) {ot = ot_; oid = oid_;}

	void clear()
	{
		ot = 0;
		oid = 0;
	}

	unsigned char ot, oid;
};

class QZOD_DNSETTINGSSHARED_EXPORT ZBuildList
{
public:
	ZBuildList();

	void LoadDefaults();
	void SetZSettings(ZSettings *zsettings_);
	vector<buildlist_object> &GetBuildList(int building_type, int level);
	bool UnitInBuildList(int building_type, int level, unsigned char ot, unsigned char oid);
	bool GetFirstUnitInBuildList(int building_type, int level, unsigned char &ot, unsigned char &oid);

	int UnitBuildTime(unsigned char ot, unsigned char oid);
private:
	void ClearData();
	vector<buildlist_object> buildlist_data[MAX_BUILDING_TYPES][MAX_BUILDING_LEVELS];

	ZSettings *zsettings;
};

#endif

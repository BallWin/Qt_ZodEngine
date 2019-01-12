#ifndef ZUNITRATING_H
#define ZUNITRATING_H

#include "qzod_dnobjects_global.h"


enum unit_cross_reference
{
	UCR_WILL_DIE, UCR_EVEN, UCR_WILL_KILL
};

class QZOD_DNOBJECTSSHARED_EXPORT ZUnitRating
{
public:
	ZUnitRating();

	void Init();
	void InitMallocUCR();
	void InitPopulateUCR();

	bool IsUnit(unsigned char ot, unsigned char oid);
	void InsertCrossReference(unsigned char a_ot, unsigned char a_oid, unsigned char v_ot, unsigned char v_oid, int ucr);
	int CrossReference(unsigned char a_ot, unsigned char a_oid, unsigned char v_ot, unsigned char v_oid);

private:
	char ****unit_cr;
};

#endif

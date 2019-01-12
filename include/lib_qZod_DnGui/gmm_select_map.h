#ifndef ZGMM_SELECT_MAP_H
#define ZGMM_SELECT_MAP_H

#include "qzod_dngui_global.h"
#include "zgui_main_menu_base.h"

class QZOD_DNGUISHARED_EXPORT GMMSelectMap : public ZGuiMainMenuBase
{
public:
	GMMSelectMap();

	void Process();
private:
	GMMWList map_list;
	GMMWButton select_button;
	GMMWButton reset_button;

	void SetupLayout1();

	void HandleWidgetEvent(int event_type, ZGMMWidget *event_widget);

	void SetupList();
};

#endif

#ifndef ZGMM_MAIN_MENU_H
#define ZGMM_MAIN_MENU_H

#include "qzod_dngui_global.h"
#include "zgui_main_menu_base.h"
#include "gui_structures.h"


class QZOD_DNGUISHARED_EXPORT GMMMainMenu : public ZGuiMainMenuBase
{
public:
	GMMMainMenu();
private:
	GMMWButton menu_button[MAX_GMMMM_BUTTONS];

	void HandleWidgetEvent(int event_type, ZGMMWidget *event_widget);
};

#endif

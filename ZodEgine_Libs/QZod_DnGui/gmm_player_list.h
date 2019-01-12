#ifndef ZGMM_PLAYER_LIST_H
#define ZGMM_PLAYER_LIST_H

#include "qzod_dngui_global.h"
#include "zgui_main_menu_base.h"

class QZOD_DNGUISHARED_EXPORT GMMPlayerList : public ZGuiMainMenuBase
{
public:
	GMMPlayerList();

	void Process();
private:
	GMMWList player_list;
	GMMWLabel players_on_label;
	GMMWLabel players_onnum_label;

	void SetupLayout1();

	void HandleWidgetEvent(int event_type, ZGMMWidget *event_widget);

	void SetupList();
};

#endif

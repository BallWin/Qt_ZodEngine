#ifndef ZGMM_MANAGE_BOTS_H
#define ZGMM_MANAGE_BOTS_H

#include "qzod_dngui_global.h"
#include "zgui_main_menu_base.h"

class GMMManageBots : public ZGuiMainMenuBase
{
public:
	GMMManageBots();

	void Process();
private:
	GMMWLabel team_label[MAX_TEAM_TYPES];
	GMMWButton start_button[MAX_TEAM_TYPES];
	GMMWButton stop_button[MAX_TEAM_TYPES];

	void SetupLayout1();
	void CheckBots();

	void HandleWidgetEvent(int event_type, ZGMMWidget *event_widget);
};

#endif

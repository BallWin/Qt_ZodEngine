#ifndef ZGMM_OPTIONS_H
#define ZGMM_OPTIONS_H

#include "qzod_dngui_global.h"
#include "zgui_main_menu_base.h"



#ifndef MAX_GMMOPTIONS_SPEED_SETTINGS
#define MAX_GMMOPTIONS_SPEED_SETTINGS 7
#endif

const double gmmoption_speed_setting_value[MAX_GMMOPTIONS_SPEED_SETTINGS] =
{
	0.25, 0.5, 0.75, 1.0, 1.5, 2.0, 4.0
};

class QZOD_DNGUISHARED_EXPORT GMMOptions : public ZGuiMainMenuBase
{
public:
	GMMOptions();

	void Process();
private:
	GMMWButton reshuffle_button;
	GMMWButton reset_button;
	GMMWButton pause_button;
	GMMWLabel volume_label;
	GMMWRadio volume_radio;
	GMMWLabel speed_label;
	GMMWRadio speed_radio;

	void SetupLayout1();

	void HandleWidgetEvent(int event_type, ZGMMWidget *event_widget);

	void SetVolumeStatus();
	void SetTimeStatuses();
};

#endif

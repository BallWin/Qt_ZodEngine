#ifndef ZGMM_WARNING_H
#define ZGMM_WARNING_H

#include "qzod_dngui_global.h"
#include "zgui_main_menu_base.h"

class QZOD_DNGUISHARED_EXPORT GMMWarning : public ZGuiMainMenuBase
{
public:
	GMMWarning(gmm_warning_flag warning_flags_ = gmm_warning_flag());

	void DoRender(ZMap &the_map, SDL_Surface *dest);
private:
	GMMWButton ok_button;
	GMMWButton cancel_button;
	GMMWLabel text1_label;
	GMMWLabel text2_label;

	void SetupLayout1();

	void HandleWidgetEvent(int event_type, ZGMMWidget *event_widget);
};

#endif

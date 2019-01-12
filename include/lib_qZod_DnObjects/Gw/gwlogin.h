#ifndef ZGWLOGIN_H
#define ZGWLOGIN_H

#include "../qzod_dnobjects_global.h"
#include "../zgui_window.h"

class QZOD_DNOBJECTSSHARED_EXPORT GWLogin : public ZGuiWindow
{
public:
	GWLogin(ZTime *ztime_);

	static void Init();

	void DoRender(ZMap &the_map, SDL_Surface *dest);
	bool Click(int x_, int y_);
	bool UnClick(int x_, int y_);
	bool KeyPress(int c);

private:
	static int finished_init;

	static ZSDL_Surface base_img;

	void DoLogin();
	void DoCreate();

	ZGuiButton login_button;
	ZGuiButton create_button;

	ZGuiTextBox loginname_box;
	ZGuiTextBox loginpass_box;
};

#endif

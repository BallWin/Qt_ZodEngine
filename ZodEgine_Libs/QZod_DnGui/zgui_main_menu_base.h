#ifndef ZGUIMAINMENUBASE_H
#define ZGUIMAINMENUBASE_H


#include "qzod_dngui_global.h"
#include "zgui_main_menu_widgets.h"
#include "gui_structures.h"

class QZOD_DNGUISHARED_EXPORT gmm_warning_flag
{
public:
	gmm_warning_flag() { clear(); }

	void clear()
	{
		text1.clear();
		text2.clear();
		quit_game = false;
		reset_map = false;
	}

	string text1;
	string text2;
	bool quit_game;
	bool reset_map;
};

class QZOD_DNGUISHARED_EXPORT gmm_flag
{
public:
	gmm_flag() { clear(); }

	void clear()
	{
		open_main_menu = false;
		open_main_menu_type = -1;
		reshuffle_teams = false;
		change_team = false;
		change_team_type = -1;
		start_bot = false;
		stop_bot = false;
		start_bot_team = -1;
		stop_bot_team = -1;
		change_map = false;
		change_map_number = -1;
		reset_map = false;
		quit_game = false;
		set_volume = false;
		set_volume_value = -1;
		pause_game = false;
		set_game_speed = false;
		set_game_speed_value = 1.0;

		warning_flags.clear();
	}

	bool open_main_menu;
	int open_main_menu_type;
	bool reshuffle_teams;
	bool change_team;
	int change_team_type;
	bool start_bot;
	int start_bot_team;
	bool stop_bot;
	int stop_bot_team;
	bool change_map;
	int change_map_number;
	bool reset_map;
	bool quit_game;
	bool set_volume;
	int set_volume_value;
	bool pause_game;
	bool set_game_speed;
	float set_game_speed_value;

	gmm_warning_flag warning_flags;
};

class QZOD_DNGUISHARED_EXPORT ZGuiMainMenuBase
{
public:
	ZGuiMainMenuBase();
	virtual ~ZGuiMainMenuBase() {}

	static void Init();

	virtual void Process();
	virtual void DoRender(ZMap &the_map, SDL_Surface *dest);

	virtual bool Click(int x_, int y_);
	virtual bool UnClick(int x_, int y_);
	virtual bool Motion(int x_, int y_);
	virtual bool KeyPress(int c);
	virtual bool WheelUpButton();
	virtual bool WheelDownButton();

	bool KillMe() { return killme; }
	int GetMenuType() { return menu_type; }
	void SetCenterCoords(int cx_, int cy_);
	bool WithinDimensions(int x_, int y_);
	gmm_flag &GetGMMFlags() { return gmm_flags; }
	void GetCoords(int &x_, int &y_) { x_ = x; y_ = y; }
	void GetDimensions(int &w_, int &h_) { w_ = w; h_ = h; }
	bool IsOverHUD(int hud_left, int hud_top);
	void SetPlayerInfoList(vector<p_info> *player_info_) { player_info = player_info_; }
	void SetSelectableMapList(vector<string> *selectable_map_list_) { selectable_map_list = selectable_map_list_; }
	void SetPlayerTeam(int *player_team_) { player_team=player_team_; }
	void SetSoundSetting(int *sound_setting_) { sound_setting = sound_setting_; }
	void SetZTime(ZTime *ztime_) { ztime=ztime_; }
	void Move(double px, double py);
	void SetWarningFlags(gmm_warning_flag warning_flags_) { warning_flags=warning_flags_; }

protected:
	static bool finished_init;
	static ZSDL_Surface menu_top_left_img;
	static ZSDL_Surface menu_top_right_img;
	static ZSDL_Surface menu_top_img;
	static ZSDL_Surface menu_left_img;
	static ZSDL_Surface menu_right_img;
	static ZSDL_Surface menu_bottom_img;
	static ZSDL_Surface menu_center_img;
	static ZSDL_Surface menu_warning_img;

	void UpdateDimensions();
	void ProcessWidgets();

	void MakeTitle();
	void AddWidget(ZGMMWidget *new_widget) { widget_list.push_back(new_widget); }

	void RenderBase(ZMap &the_map, SDL_Surface *dest, int tx, int ty);
	void RenderWidgets(ZMap &the_map, SDL_Surface *dest);

	virtual void HandleWidgetEvent(int event_type, ZGMMWidget *event_widget);

	bool killme;
	int menu_type;
	int x, y, w, h;
	string title;
	ZSDL_Surface title_img;
	gmm_flag gmm_flags;
	vector<p_info> *player_info;
	vector<string> *selectable_map_list;
	int *player_team;
	int *sound_setting;
	ZTime *ztime;
	gmm_warning_flag warning_flags;

	bool click_grabbed;
	int grab_x, grab_y;

	vector<ZGMMWidget*> widget_list;

	GMMWButton close_button;
};

#endif

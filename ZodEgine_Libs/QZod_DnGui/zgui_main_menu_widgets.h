#ifndef ZGUIMAINMENUWIDGETS_H
#define ZGUIMAINMENUWIDGETS_H


#include "qzod_dngui_global.h"
#include "gui_structures.h"



class QZOD_DNGUISHARED_EXPORT ZGMMWidget
{
public:
	ZGMMWidget();
    virtual ~ZGMMWidget()=0;

	virtual void Process() {}
    virtual void DoRender(ZMap &the_map, SDL_Surface *dest, int tx, int ty) {
        if(the_map.isEmpty()||(dest==nullptr)||(tx<0)||(ty<0))
            std::cerr<<"\n--<error>"<<__FILE__<<" "<<__LINE__<<"\t x or y < 0!\n";
    }

	void SetActive(bool active_) { active = active_; }
	void ToggleActive() { active = !active; }
	bool IsActive() { return active; }
	int GetRefID() { return ref_id; }
	int GetWidgetType() { return widget_type; }
	int GetWidth() { return w; }
	int GetHeight() { return h; }
	gmmw_flag &GetGMMWFlags() { return the_flags; }

	void SetDimensions(int w_, int h_) { w=w_; h=h_; }
	void SetCoords(int x_, int y_) { x=x_; y=y_; }
	bool WithinDimensions(int x_, int y_);

    virtual bool Click(int x_, int y_) {
        if((x_<0)||(y_<0)) std::cerr<<"\n--<error>"<<__FILE__<<" "<<__LINE__<<"\t x or y < 0!\n";
        return false; }
    virtual bool UnClick(int x_, int y_) {
        if((x_<0)||(y_<0)) std::cerr<<"\n--<error>"<<__FILE__<<" "<<__LINE__<<"\t x or y < 0!\n";
        return false; }
    virtual bool Motion(int x_, int y_) {
        if((x_<0)||(y_<0)) std::cerr<<"\n--<error>"<<__FILE__<<" "<<__LINE__<<"\t x or y < 0!\n";
        return false; }
    virtual bool KeyPress(int c) {
        if(c<0)std::cerr<<"\n--<error>"<<__FILE__<<" "<<__LINE__<<"\t c < 0!\n";
        return false; }
	virtual bool WheelUpButton() { return false; }
	virtual bool WheelDownButton() { return false; }
protected:
	int x,y,w,h;
	bool active;
	int ref_id;
	int widget_type;
	gmmw_flag the_flags;

private:
	static int next_ref_id;
};


class QZOD_DNGUISHARED_EXPORT GMMWButton : public ZGMMWidget
{
public:
	GMMWButton();

	static void Init();

	void DoRender(ZMap &the_map, SDL_Surface *dest, int tx, int ty);

	bool Click(int x_, int y_);
	bool UnClick(int x_, int y_);

	void SetText(string text_) { text = text_; rerender_text = true; }
	void SetType(int type_) { type = type_; DetermineDimensions(); }
	void SetGreen(bool is_green_) { is_green = is_green_; }
private:
	static bool finished_init;

	static ZSDL_Surface generic_top_left_img[MAX_MMBUTTON_STATES];
	static ZSDL_Surface generic_top_img[MAX_MMBUTTON_STATES];
	static ZSDL_Surface generic_top_right_img[MAX_MMBUTTON_STATES];
	static ZSDL_Surface generic_left_img[MAX_MMBUTTON_STATES];
	static ZSDL_Surface generic_center_img[MAX_MMBUTTON_STATES];
	static ZSDL_Surface generic_right_img[MAX_MMBUTTON_STATES];
	static ZSDL_Surface generic_bottom_left_img[MAX_MMBUTTON_STATES];
	static ZSDL_Surface generic_bottom_img[MAX_MMBUTTON_STATES];
	static ZSDL_Surface generic_bottom_right_img[MAX_MMBUTTON_STATES];
	static ZSDL_Surface non_generic_img[MAX_MMBUTTON_TYPES][MAX_MMBUTTON_STATES];

	void RenderGeneric(ZMap &the_map, SDL_Surface *dest, int tx, int ty);

	void MakeTextImage();
	void DetermineDimensions();

	string text;
	ZSDL_Surface text_img;
	bool rerender_text;

	int state;
	int type;
	bool is_green;
	
};


class QZOD_DNGUISHARED_EXPORT GMMWLabel : public ZGMMWidget
{
public:
	GMMWLabel();

	void DoRender(ZMap &the_map, SDL_Surface *dest, int tx, int ty);

	void SetText(string text_);
	void SetJustification(int justify_) { justify = justify_; }
	void SetFont(int font_) { font=font_; }
private:
	void MakeTextImage();

	int justify;
	string text;
	ZSDL_Surface text_img;
	string rendered_text;
	bool rerender_text;
	int font;
};



class QZOD_DNGUISHARED_EXPORT mmlist_entry
{
public:
	mmlist_entry() { clear(); }
	mmlist_entry(string text_, int ref_id_, int sort_number_) 
	{ 
		text = text_; 
		ref_id = ref_id_; 
		sort_number= sort_number_; 
		state = MMLIST_NORMAL;
	}

	void clear()
	{
		text.clear();
		ref_id = -1;
		sort_number = -1;
		state = MMLIST_NORMAL;
	}

	string text;
	int ref_id;
	int sort_number;
	int state;

	friend bool sort_mmlist_entry_func (const mmlist_entry &a, const mmlist_entry &b);
};

extern bool sort_mmlist_entry_func (const mmlist_entry &a, const mmlist_entry &b);

class QZOD_DNGUISHARED_EXPORT GMMWList : public ZGMMWidget
{
public:
	GMMWList();

	static void Init();

	void Process();
	void DoRender(ZMap &the_map, SDL_Surface *dest, int tx, int ty);

	bool Click(int x_, int y_);
	bool UnClick(int x_, int y_);
	bool WheelUpButton() { return MoveUp(); }
	bool WheelDownButton() { return MoveDown(); }

	void SetVisibleEntries(int visible_entries_);
	vector<mmlist_entry> &GetEntryList() { return entry_list; }
	void CheckViewI();
	void UnSelectAll(int except_entry = -1);
	int GetFirstSelected();
private:
	static bool finished_init;

	static ZSDL_Surface list_top_left_img;
	static ZSDL_Surface list_top_img;
	static ZSDL_Surface list_top_right_img;
	static ZSDL_Surface list_left_img;
	static ZSDL_Surface list_right_img;
	static ZSDL_Surface list_bottom_left_img;
	static ZSDL_Surface list_bottom_img;
	static ZSDL_Surface list_bottom_right_img;

	static ZSDL_Surface list_entry_top_img[MAX_MMLIST_STATES];
	static ZSDL_Surface list_entry_left_img[MAX_MMLIST_STATES];
	static ZSDL_Surface list_entry_center_img[MAX_MMLIST_STATES];
	static ZSDL_Surface list_entry_right_img[MAX_MMLIST_STATES];
	static ZSDL_Surface list_entry_bottom_img[MAX_MMLIST_STATES];

	static ZSDL_Surface list_button_up_img[MAX_MMLIST_STATES];
	static ZSDL_Surface list_button_down_img[MAX_MMLIST_STATES];

	static ZSDL_Surface list_scroller_img;

	void RenderBackround(ZMap &the_map, SDL_Surface *dest, int tx, int ty);
	void RenderEntries(ZMap &the_map, SDL_Surface *dest, int tx, int ty);
	void RenderControls(ZMap &the_map, SDL_Surface *dest, int tx, int ty);
	void RenderEntry(ZMap &the_map, SDL_Surface *dest, int ix, int iy, int iw, ZSDL_Surface *text, int state);

	void SetHeight();
	bool MoveUp();
	bool MoveDown();
	bool WithinUpButton(int x_, int y_);
	bool WithinDownButton(int x_, int y_);
	int WithinEntry(int x_, int y_);

	int visible_entries;
	int view_i;

	int up_button_state;
	int down_button_state;

	double button_down_time;

	vector<mmlist_entry> entry_list;
};


class QZOD_DNGUISHARED_EXPORT GMMWRadio : public ZGMMWidget
{
public:
	GMMWRadio();
	static void Init();

	void DoRender(ZMap &the_map, SDL_Surface *dest, int tx, int ty);

	bool Click(int x_, int y_);

	void SetMaxSelections(int selections_);
	void SetSelected(int si_) { si=si_; CheckSI(); }
	int GetSelected() { return si; }

private:
	static bool finished_init;

	static ZSDL_Surface radio_left_img;
	static ZSDL_Surface radio_center_img;
	static ZSDL_Surface radio_right_img;
	static ZSDL_Surface radio_selector_img;

	void CheckSI();

	int selections;
	int si;
};


class QZOD_DNGUISHARED_EXPORT GMMWTeamColor : public ZGMMWidget
{
public:
	GMMWTeamColor();

	static void Init();

	void DoRender(ZMap &the_map, SDL_Surface *dest, int tx, int ty);

	void SetTeam(int team_);
private:
	static bool finished_init;

	static ZSDL_Surface team_color_img[MAX_TEAM_TYPES];

	int team;
};

#endif

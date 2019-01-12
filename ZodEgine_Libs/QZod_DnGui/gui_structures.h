#ifndef GUI_STRUCTURES_H
#define GUI_STRUCTURES_H

#include "qzod_dngui_global.h"



//// -- CURSOR -----------------------------------
//#ifndef DEF_RC_FOLDER_CURSOR
//    #define DEF_RC_FOLDER_CURSOR "assets/cursors"
//#endif

// -- MENU -------------------------------------
#ifndef GMM_SIDE_MARGIN
#define GMM_SIDE_MARGIN 5
#endif
#ifndef GMM_TOP_MARGIN
#define GMM_TOP_MARGIN 5
#endif
#ifndef GMM_BOTTOM_MARGIN
#define GMM_BOTTOM_MARGIN 5
#endif
#ifndef GMM_TITLE_HEIGHT
#define GMM_TITLE_HEIGHT 18
#endif
#ifndef GMM_TITLE_MARGIN
#define GMM_TITLE_MARGIN (GMM_TITLE_HEIGHT + GMM_TOP_MARGIN)
#endif


#ifndef DEF_RC_FOLDER_MENU
    #define DEF_RC_FOLDER_MENU "assets/other/main_menu_gui"
#endif


enum menu_type
{
    GMM_MAIN_MAIN, GMM_CHANGE_TEAMS, GMM_MANAGE_BOTS,
    GMM_PLAYER_LIST, GMM_SELECT_MAP, GMM_OPTIONS, GMM_WARNING,
    MAX_MENU_TYPES
};

enum gmm_event_type
{
    GMM_UNKNOWN_EVENT, GMM_CLICK_EVENT, GMM_UNCLICK_EVENT,
    GMM_MOTION_EVENT, GMM_KEYPRESS_EVENT, GMM_WHEELUP_EVENT,
    GMM_WHEELDOWN_EVENT, MAC_GMM_EVENTS
};

const string gmm_event_type_string[MAC_GMM_EVENTS] =
{
    "unknown", "click", "unclick", "motion", "keypress",
    "wheelup", "wheeldown"
};

// -- MWIDGETS -------------------------------------
#ifndef GMMWBUTTON_HEIGHT
#define GMMWBUTTON_HEIGHT 15
#endif
#ifndef MMLABEL_HEIGHT
#define MMLABEL_HEIGHT    10
#endif
#ifndef MMLIST_ENTRY_HEIGHT
#define MMLIST_ENTRY_HEIGHT 13
#endif
#ifndef MMLIST_MIN_ENTRIES
#define MMLIST_MIN_ENTRIES 4
#endif
#ifndef MMLIST_UP_BUTTON_FROM_TOP
#define MMLIST_UP_BUTTON_FROM_TOP 3
#endif
#ifndef MMLIST_UP_BUTTON_FROM_RIGHT
#define MMLIST_UP_BUTTON_FROM_RIGHT 12
#endif
#ifndef MMLIST_DOWN_BUTTON_FROM_BOTTOM
#define MMLIST_DOWN_BUTTON_FROM_BOTTOM 11
#endif
#ifndef MMLIST_DOWN_BUTTON_FROM_RIGHT
#define MMLIST_DOWN_BUTTON_FROM_RIGHT 12
#endif
#ifndef MMLIST_SCROLLER_FROM_RIGHT
#define MMLIST_SCROLLER_FROM_RIGHT 9
#endif

#ifndef MMRADIO_HEIGHT
#define MMRADIO_HEIGHT 9
#endif
#ifndef MMRADIO_LEFT_WIDTH
#define MMRADIO_LEFT_WIDTH 16
#endif
#ifndef MMRADIO_CENTER_WIDTH
#define MMRADIO_CENTER_WIDTH 13
#endif
#ifndef MMRADIO_RIGHT_WIDTH
#define MMRADIO_RIGHT_WIDTH 15
#endif
#ifndef MMRADIO_MIN_SELECTIONS
#define MMRADIO_MIN_SELECTIONS 2
#endif

#ifndef MMTEAM_COLOR_HEIGHT
#define MMTEAM_COLOR_HEIGHT 12
#endif
#ifndef MMTEAM_COLOR_WIDTH
#define MMTEAM_COLOR_WIDTH 19
#endif

class QZOD_DNGUISHARED_EXPORT gmmw_flag
{
public:
    gmmw_flag() { clear(); }

    void clear()
    {
        mmlist_entry_selected = -1;
        mmradio_si_selected = -1;
    }

    int mmlist_entry_selected;
    int mmradio_si_selected;
};

enum mmwidget_type
{
    MMUNKNOWN_WIDGET, MMBUTTON_WIDGET, MMLABEL_WIDGET,
    MMLIST_WIDGET, MMRADIO_WIDGET, MMTEAM_COLOR_WIDGET, MAX_MMWIDGETS
};

const string mmwidget_type_string[MAX_MMWIDGETS] =
{
    "unknown", "button", "label", "list", "radio", "team_color"
};


enum mmbutton_type
{
    MMGENERIC_BUTTON, MMCLOSE_BUTTON, MAX_MMBUTTON_TYPES
};

const string mmbutton_type_string[MAX_MMBUTTON_TYPES] =
{
    "", "close"
};

enum mmbutton_state
{
    MMBUTTON_NORMAL, MMBUTTON_PRESSED, MMBUTTON_GREEN, MAX_MMBUTTON_STATES
};

enum mmlabel_justify_type
{
    MMLABEL_NORMAL, MMLABEL_CENTER, MMLABEL_RIGHT, MAX_MMLABEL_JUSTIFIES
};


enum mmlist_state
{
    MMLIST_NORMAL, MMLIST_PRESSED, MAX_MMLIST_STATES
};


// -- MAIN MENU -------------------------------------
enum gmm_main_menu_button
{
    GMMMM_CHANGE_TEAMS_BUTTON, GMMMM_MANAGE_BOTS_BUTTON, GMMMM_PLAYER_LIST_BUTTON,
    GMMMM_SELECT_MAP_BUTTON, GMMMM_MULTIPLAYER_BUTTON, GMMMM_OPTIONS_BUTTON,
    GMMMM_QUIT_GAME_BUTTON,
    MAX_GMMMM_BUTTONS
};

const string gmm_main_menu_button_string[MAX_GMMMM_BUTTONS] =
{
    "Change Teams", "Manage Bots", "Player List",
    "Select Map", "Multiplayer", "Options", "Quit Game"
};


//// -- VOTE -------------------------------------
//#ifndef DEF_RC_FOLDER_VOTE_MENU
//    #define DEF_RC_FOLDER_VOTE_MENU "assets/other/menus"
//#endif

//#ifndef MAX_VOTE_TIME
//#define MAX_VOTE_TIME 30
//#endif

//enum vote_type
//{
//    PAUSE_VOTE, RESUME_VOTE, CHANGE_MAP_VOTE, START_BOT, STOP_BOT,
//    RESET_GAME_VOTE, RESHUFFLE_TEAMS_VOTE, CHANGE_GAME_SPEED,
//    MAX_VOTE_TYPES
//};

//const string vote_type_string[MAX_VOTE_TYPES] =
//{
//    "Pause Game", "Resume Game", "Change Map", "Start Bot", "Stop Bot",
//    "Reset Game", "Reshuffle Teams", "Set Game Speed"
//};

#endif // GUI_STRUCTURES_H

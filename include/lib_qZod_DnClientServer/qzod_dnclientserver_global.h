#ifndef QZOD_DNCLIENTSERVER_GLOBAL_H
#define QZOD_DNCLIENTSERVER_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QZOD_DNCLIENTSERVER_LIBRARY)
#  define QZOD_DNCLIENTSERVERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QZOD_DNCLIENTSERVERSHARED_EXPORT Q_DECL_IMPORT
#endif


#include <errno.h>
#include <string>
#include <algorithm>
#include <vector>
#include <time.h>
#include <cstdio>
#include <cstdlib>
using namespace std;

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>

// --  Separate
#include <lib_qZod_DnSeparate/common.h>
#include <lib_qZod_DnSeparate/constants.h>
#include <lib_qZod_DnSeparate/event_handler.h>
#include <lib_qZod_DnSeparate/zdamagemissile.h>
#include <lib_qZod_DnSeparate/zpsettings.h>
#include <lib_qZod_DnSeparate/zmysql.h>
#include <lib_qZod_DnSeparate/zfont_engine.h>
#include <lib_qZod_DnSeparate/zsdl.h>
#include <lib_qZod_DnSeparate/zsdl_opengl.h>

// -- Map
#include <lib_qZod_DnMap/qzod_map.h>
#include <lib_qZod_DnMap/zteam.h>

// -- Sound
#include <lib_qZod_DnSoundEngine/zsound_engine.hpp>

// -- Effects
#include <lib_qZod_DnEffect/zod_effect.hpp>
//#include <lib_qZod_DnEffect/qzod_effect_old.h>


// -- Objects
#include <lib_qZod_DnObjects/zod_objects_old.hpp>
#include <lib_qZod_DnObjects/cursor.h>
#include <lib_qZod_DnObjects/zmini_map.h>
#include <lib_qZod_DnObjects/zcomp_message_engine.h>
#include <lib_qZod_DnObjects/zgui_window.h>
#include <lib_qZod_DnObjects/zmusic_engine.h>
#include <lib_qZod_DnObjects/Gw/gwcreateuser.h>
#include <lib_qZod_DnObjects/Gw/gwproduction.h>
#include <lib_qZod_DnObjects/Gw/gwfactory_list.h>
#include <lib_qZod_DnObjects/Gw/gwlogin.h>

// -- Gui
#include <lib_qZod_DnGui/z_gui.hpp>
//#include <lib_qZod_DnGui/gmm_change_teams.h>
//#include <lib_qZod_DnGui/gmm_main_menu.h>
//#include <lib_qZod_DnGui/gmm_manage_bots.h>
//#include <lib_qZod_DnGui/gmm_player_list.h>
//#include <lib_qZod_DnGui/gmm_select_map.h>
//#include <lib_qZod_DnGui/gmm_options.h>
//#include <lib_qZod_DnGui/gmm_warning.h>
//#include <lib_qZod_DnGui/zgui_main_menu_base.h>


using namespace  COMMON;




#endif // QZOD_DNCLIENTSERVER_GLOBAL_H

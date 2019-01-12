#ifndef MAIN_OPTIONS_H
#define MAIN_OPTIONS_H


#ifdef _WIN32
//xgetopt stuff
//this is only needed for some compilers
#include "xgetopt.h"
int optind, opterr;
TCHAR *optarg;
//end xgetopt

#else
#include <unistd.h>
#endif


#include <string>
#include <cstdlib>
using namespace std;

#include <lib_qZod_DnSeparate/constants.h>

class input_options
{
public:
    input_options();

    int getoptions(int argc, char **argv);
    int checkoptions();
    void setdefaults();

    bool read_connect_address;
    bool read_map_name;
    bool read_map_list;
    bool read_is_windowed;
    bool read_resolution;
    bool read_is_dedicated;
    bool read_display_help;
    bool read_display_version;
    bool read_player_name;
    bool read_player_team;
    bool read_sound_off;
    bool read_music_off;
    bool read_disable_zcursor;
    bool read_settings;
    bool read_p_settings;
    bool read_opengl_off;
    bool read_start_bot[MAX_TEAM_TYPES];
    bool read_run_tray;
    bool read_loginname;
    bool read_password;
    string connect_address;
    string map_name;
    string map_list;
    string resolution;
    string player_name;
    string player_team_str;
    string settings_filename;
    string p_settings_filename;
    string loginname;
    string password;
    int team;
    int resolution_width;
    int resolution_height;

};

#endif // MAIN_OPTIONS_H

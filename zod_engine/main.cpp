#include <QCoreApplication>
// ===================================
#include <string>
#include <cstdlib>
using namespace std;

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

#include <lib_qZod_DnSeparate/common.h>
#include <lib_qZod_DnMap/qzod_map.h>
#include <lib_qZod_DnSoundEngine/qzod_soundengine_old.h>
#include <lib_qZod_DnSettings/qzod_settings_old.h>
#include <lib_qZod_DnGui/zgui_main_menu_base.h>
#include <lib_qZod_DnEffect/zod_effect.hpp>
#include <lib_qZod_DnObjects/zobject.h>
#include <lib_qZod_DnClientServer/zclient.h>
#include <lib_qZod_DnClientServer/zserver.h>
#include <lib_qZod_DnClientServer/zplayer.h>
#include <lib_qZod_DnClientServer/zbot.h>
#include <lib_qZod_DnClientServer/ztray.h>


#include "main_options.h"

void display_help(char *shell_command);
void display_version();
int run_server_thread(void *nothing);
int run_bot_thread(void *nothing);
void run_player_thread();
void run_tray_app();

static input_options starting_conditions;

static char bot_bypass_data[MAX_BOT_BYPASS_SIZE];
static int bot_bypass_size;


//int main(int argc, char *argv[])
//{
//    QCoreApplication a(argc, argv);

//    return a.exec();
//}
// =============================================================
// ***** command line for canpaing start
//      -l map_list.txt -n zlover -t red -r 800x484 -o -b blue
// =============================================================
int main(int argc, char **argv)
{
    SDL_Thread *server_thread;

    printf("Welcome to the Zod Engine\n");

    if(argc<=1) starting_conditions.setdefaults();

    //read in the arguments
    starting_conditions.getoptions(argc, argv);

    //make sure there is nothing conflicting,
    //like we are trying to make a dedicated server that is supposed to connect to another server
    starting_conditions.checkoptions();

    //init this for the bots
    ZCore::CreateRandomBotBypassData(bot_bypass_data, bot_bypass_size);

    //now see what we have
    if(starting_conditions.read_display_version) display_version();
    if(starting_conditions.read_display_help) display_help(argv[0]);

    //now what do we really run?
    else if(starting_conditions.read_run_tray)
    {
        run_tray_app();
    }
    else if(starting_conditions.read_is_dedicated)
    {
        //run only a server
        //server_thread = SDL_CreateThread(run_server_thread, nullptr);
        run_server_thread(nullptr);
    }
    else if(starting_conditions.read_connect_address)
    {
        //connect to a server
        run_player_thread();
    }
    else
    {
        //run a server, then connect to it
        server_thread = SDL_CreateThread(run_server_thread, nullptr);
        run_player_thread();
    }

    return 1;
}

int run_server_thread(void *nothing)
{
    int i;
    vector<SDL_Thread*> bot_thread;
    ZServer zserver;

    for(i=0;i<MAX_TEAM_TYPES;i++)
        if(starting_conditions.read_start_bot[i])
        {
            zserver.InitBot(i);

            //int *team = new int;
            //printf("start bot for team %s\n", team_type_string[i].c_str());

            //*team = i;
            //bot_thread.push_back(SDL_CreateThread(run_bot_thread, (void*)team));
        }

    if(starting_conditions.read_map_name)
        zserver.SetMapName(starting_conditions.map_name);
    else if(starting_conditions.read_map_list)
        zserver.SetMapList(starting_conditions.map_list);

    if(starting_conditions.read_settings)
        zserver.SetSettingsFilename(starting_conditions.settings_filename);
    if(starting_conditions.read_p_settings)
        zserver.SetPerpetualSettingsFilename(starting_conditions.p_settings_filename);

    zserver.SetBotBypassData(bot_bypass_data, bot_bypass_size);

    zserver.Setup();
    zserver.Run();

    return 1;
}

int run_bot_thread(void *nothing)
{
    int *team;
    ZBot zbot;

    team = (int*)nothing;

    zbot.SetDesiredTeam((team_type)*team);
    if(starting_conditions.read_connect_address)
        zbot.SetRemoteAddress(starting_conditions.connect_address);

    zbot.SetBotBypassData(bot_bypass_data, bot_bypass_size);

    zbot.Setup();
    zbot.Run();

    return 1;
}

void run_player_thread()
{
    ZPlayer zplayer;

    zplayer.DisableCursor(starting_conditions.read_disable_zcursor);
    zplayer.SetSoundsOff(starting_conditions.read_sound_off);
    zplayer.SetMusicOff(starting_conditions.read_music_off);
    zplayer.SetWindowed(starting_conditions.read_is_windowed);
    zplayer.SetUseOpenGL(!starting_conditions.read_opengl_off);
    if(starting_conditions.read_player_team)
        zplayer.SetDesiredTeam((team_type)starting_conditions.team);
    if(starting_conditions.read_player_name)
        zplayer.SetPlayerName(starting_conditions.player_name);
    if(starting_conditions.read_loginname)
        zplayer.SetLoginName(starting_conditions.loginname);
    if(starting_conditions.read_password)
        zplayer.SetLoginPassword(starting_conditions.password);
    if(starting_conditions.read_connect_address)
        zplayer.SetRemoteAddress(starting_conditions.connect_address);
    if(starting_conditions.read_resolution)
        zplayer.SetDimensions(starting_conditions.resolution_width, starting_conditions.resolution_height);

    zplayer.Setup();
    zplayer.Run();
}

void run_tray_app()
{
    ZTray ztray;

    if(starting_conditions.read_connect_address)
        ztray.SetRemoteAddress(starting_conditions.connect_address);

    ztray.Setup();
    ztray.Run();
}

void display_help(char *shell_command)
{
    printf("\n==================================================================\n");
    printf("Command list...\n");
    printf("-c ip_address        - game host address\n");
    printf("-m filename          - map to be used\n");
    printf("-l filename          - map list to be used\n");
    printf("-z filename          - settings file to be used\n");
    printf("-e filename          - main server settings file to be used\n");
    printf("-n player_name       - your player name\n");
    printf("-g login_name        - your login name\n");
    printf("-i login_password    - your login password\n");
    printf("-t team              - your team\n");
    printf("-b team              - connect a bot player\n");
    printf("-w                   - run game in windowed mode\n");
    printf("-r resolution        - resolution to run the game at\n");
    printf("-d                   - run a dedicated server\n");
    printf("-h                   - display command help\n");
    printf("-s                   - no sound\n");
    printf("-u                   - no music\n");
    printf("-o                   - no opengl\n");
    printf("-k                   - use faster and blander cursor\n");
    printf("-v                   - display version and credits\n");
    printf("-a                   - run shell based tray app\n");

    printf("\nExample usage...\n");
    printf("%s -c localhost -r 800x600 -w\n", shell_command);
    printf("%s -m level1.map -b 1 -p 1\n", shell_command);
    printf("==================================================================\n");
}

void display_version()
{
#ifndef DISABLE_OPENGL
    printf("\nZod: A Zed Engine, Version Alpha\n");
#else
    printf("\nZod: A Zed Engine, Version Alpha (OpenGL Disabled)\n");
#endif
    printf("By Michael Bok\n");
    printf("Please visit http://zod.sourceforge.net/ and http://zzone.lewe.com/\n");
}




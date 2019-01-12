#include "main_options.h"

input_options::input_options()
{
    {
        read_connect_address = false;
        read_map_name = false;
        read_map_list = false;
        read_is_windowed = false;
        read_resolution = false;
        read_is_dedicated = false;
        read_display_help = false;
        read_display_version = false;
        read_sound_off = false;
        read_music_off = false;
        read_settings = false;
        read_p_settings = false;
        read_opengl_off = false;
        read_run_tray = false;
        read_loginname = false;
        read_password = false;
        team = 0;

        for(int i=0;i<MAX_TEAM_TYPES;i++)
            read_start_bot[i] = false;
    }
}

int input_options::checkoptions()
{
    //nothing?
    if(!read_connect_address &&
        !read_map_name &&
        !read_is_windowed &&
        !read_resolution &&
        !read_is_dedicated &&
        !read_display_help &&
        !read_display_version &&
        !read_map_list)
    {
        read_display_help = true;
    }

    if(read_connect_address && read_is_dedicated)
    {
        printf("cannot be a dedicated server and have a connect address\n");
        return 0;
    }

    if(read_is_dedicated && read_resolution)
    {
        printf("cannot be a dedicated server and have a screen resolution\n");
        return 0;
    }

    if(read_is_dedicated && read_is_windowed)
    {
        printf("cannot be a dedicated server and be in windowed mode\n");
        return 0;
    }

    if(read_connect_address && (read_map_name || read_map_list))
    {
        printf("cannot have a connect address and set the map\n");
        return 0;
    }

    if(read_map_name && read_map_list)
    {
        printf("cannot a read map list and a specific map file\n");
        return 0;
    }

    if(read_run_tray && !read_connect_address)
    {
        printf("need a connect address to run the tray app\n");
        return 0;
    }

    return 1;
}

void input_options::setdefaults()
{
    //-c hestia.nighsoft.net -n zlover -t red -r 800x600 -w -o
    printf("no arguments set, using defaults of '-c hestia.nighsoft.net -n zlover -t red -r 800x600 -w -o'\n");

    read_connect_address = true;
    connect_address = "hestia.nighsoft.net";

    read_player_name = true;
    player_name = "zlover";

    read_player_team = true;
    player_team_str = "red";
    team = RED_TEAM;

    resolution = "800x600";
    resolution_width = 800;
    resolution_height = 600;
    read_resolution = true;

    read_is_windowed = true;

    read_opengl_off = true;
}


int input_options::getoptions(int argc, char **argv)
{
    int c;
    int i;
    int temp_int;
    extern char *optarg;
    extern int optind;

    while ((c = getopt(argc, argv, "c:m:l:n:t:b:z:e:g:i:wr:dhvksuoa")) != -1)
    {
        switch(c)
        {
            case 'c':
                if(!optarg) return 0;
                read_connect_address = true;
                connect_address = optarg;
                break;
            case 'm':
                if(!optarg) return 0;
                read_map_name = true;
                map_name = optarg;
                break;

            case 'l':
                if(!optarg) return 0;
                read_map_list = true;
                map_list = optarg;
                break;

            case 'n':
                if(!optarg) return 0;
                read_player_name = true;
                player_name = optarg;
                break;

            case 'z':
                if(!optarg) return 0;
                read_settings = true;
                settings_filename = optarg;
                break;

            case 'e':
                if(!optarg) return 0;
                read_p_settings = true;
                p_settings_filename = optarg;
                break;

            case 'g':
                if(!optarg) return 0;
                read_loginname = true;
                loginname = optarg;
                break;

            case 'i':
                if(!optarg) return 0;
                read_password = true;
                password = optarg;
                break;

            case 't':
                if(!optarg) return 0;
                read_player_team = true;
                player_team_str = optarg;

                for(i=0;i<MAX_TEAM_TYPES;i++)
                    if(team_type_string[i] == player_team_str)
                        break;

                if(i!=MAX_TEAM_TYPES)
                    team = i;
                else
                    printf("could not find the team '%s', perhaps try lowercase?\n", player_team_str.c_str());
                break;

            case 'b':
                if(!optarg) return 0;
                //read_bot_count = true;
                //bot_count = atoi(optarg);
                if(!optarg) return 0;

                for(i=0;i<MAX_TEAM_TYPES;i++)
                    if(team_type_string[i] == optarg)
                        break;

                if(i!=MAX_TEAM_TYPES)
                    read_start_bot[i] = true;
                else
                    printf("could not find the team '%s', perhaps try lowercase?\n", optarg);
                break;

            case 'r':
                if(!optarg) return 0;
                resolution = optarg;

                temp_int = resolution.find('x');

                if(temp_int != string::npos)
                {
                    resolution_width = atoi(resolution.substr(0, temp_int).c_str());
                    resolution_height = atoi(resolution.substr(temp_int+1, 10).c_str());
                    read_resolution = true;
                }
                else
                    read_resolution = false;

                break;

            case 'w':
                read_is_windowed = true;
                break;

            case 'd':
                read_is_dedicated = true;
                break;

            case 'h':
                read_display_help = true;
                break;

            case 'v':
                read_display_version = true;
                break;

            case 's':
                read_sound_off = true;
                break;

            case 'u':
                read_music_off = true;
                break;

            case 'k':
                read_disable_zcursor = true;
                break;

            case 'o':
                read_opengl_off = true;
                break;

            case 'a':
                read_run_tray = true;
                break;

            case '?':
                printf("unrecognized option -%c\n", c);
                read_display_help = true;
                return 0;
        }

    }

    return 1;
}

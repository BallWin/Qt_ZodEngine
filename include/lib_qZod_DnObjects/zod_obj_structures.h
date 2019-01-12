#ifndef ZOD_OBJ_STRUCTURES_H
#define ZOD_OBJ_STRUCTURES_H

#include "qzod_dnobjects_global.h"


// -- CURSOR -----------------------------------
#ifndef DEF_RC_FOLDER_CURSOR
    #define DEF_RC_FOLDER_CURSOR "assets/cursors"
#endif


// -- OBJECTS -----------------------------------
enum object_mode
{
    NULL_MODE, JUST_PLACED_MODE, ROTATING_MODE,
    STATIONARY_MODE, R_WALKING, R_STANDING, R_CIGARETTE, R_FULLSCAN,
    R_HEADSTRETCH, R_BEER, R_ATTACKING, R_PICKUP_UP_GRENADES, R_PICKUP_DOWN_GRENADES,
    C_ATTACKING,
    MAX_OBJECT_MODES
};

enum waypoint_mode
{
    MOVE_WP, ENTER_WP, ATTACK_WP, FORCE_MOVE_WP, CRANE_REPAIR_WP,
    UNIT_REPAIR_WP, AGRO_WP, ENTER_FORT_WP, DODGE_WP, PICKUP_GRENADES_WP,
    MAX_WAYPOINT_MODES
};

enum crane_repair_wp_stage
{
    GOTO_ENTRANCE_CRWS, ENTER_BUILDING_CRWS, EXIT_BUILDING_CRWS
};

enum unit_repair_wp_stage
{
    GOTO_ENTRANCE_URWS, ENTER_BUILDING_URWS, EXIT_BUILDING_URWS, WAIT_URWS
};

enum agro_wp_stage
{
    ATTACK_AWS, RETURN_URWS
};

enum enter_fort_wp_stage
{
    GOTO_ENTRANCE_EFWS, ENTER_BUILDING_EFWS, EXIT_BUILDING_EFWS
};

class QZOD_DNOBJECTSSHARED_EXPORT waypoint_information
{
public:
    int stage;
    int x, y;

    int sx, sy;
    int adx, ady;

    int crane_exit_x, crane_exit_y;
    int agro_center_x, agro_center_y;
    int fort_exit_x, fort_exit_y;
    int init_attack_x, init_attack_y;

    //pathfinding stuff
    int path_finding_id;
    bool got_pf_response;
    vector<pf_point> pf_point_list;

    waypoint_information() {clear();}

    void clear()
    {
        stage = 0;
        x = 0;
        y = 0;
        sx = 0;
        sy = 0;
        adx = 0;
        ady = 0;
        crane_exit_x = 0;
        crane_exit_y = 0;
        agro_center_x = 0;
        agro_center_y = 0;
        fort_exit_x = 0;
        fort_exit_y = 0;
        init_attack_x = 0;
        init_attack_y = 0;
        path_finding_id = 0;
        got_pf_response = false;
        pf_point_list.clear();
    }

    void clear_and_kill(ZMap &tmap)
    {
        if(path_finding_id) tmap.GetPathFinder().Kill_Thread_Id(path_finding_id);
        clear();
    }
};

//these transfer packets must have a uniform makeup / size
#pragma pack(1)

class QZOD_DNOBJECTSSHARED_EXPORT waypoint
{
public:
    char mode;
    int ref_id;
    int x, y;
    bool attack_to;
    bool player_given;

    waypoint() { clear(); }

    void clear()
    {
        mode = -1;
        ref_id = -1;
        x = y = 0;
        attack_to = false;
        player_given = false;
    }

    bool operator==(const waypoint &other) const
    {
        if(this == &other) return true;

        if(other.mode != mode) return false;
        if(other.ref_id != ref_id) return false;
        if(other.x != x) return false;
        if(other.y != y) return false;
        if(other.attack_to != attack_to) return false;
        if(other.player_given != player_given) return false;

        return true;
    }

    bool operator!=(const waypoint &other) const
    {
        return !(*this == other);
    }
};

struct QZOD_DNOBJECTSSHARED_EXPORT object_location
{
    int x, y;
    float dx, dy;
};

struct QZOD_DNOBJECTSSHARED_EXPORT fire_missile_info
{
    double missile_offset_time;
    int missile_x, missile_y;
};

struct QZOD_DNOBJECTSSHARED_EXPORT driver_info_s
{
    int driver_health;
    double next_attack_time;
};

#pragma pack()

class QZOD_DNOBJECTSSHARED_EXPORT server_flag
{
public:
    server_flag() {clear();}

    bool updated_location;
    bool updated_velocity;
    bool updated_waypoints;
    bool updated_attack_object;
    bool updated_attack_object_health;
    bool updated_attack_object_driver_health;
    bool updated_open_lid;
    bool fired_missile;
    int missile_x, missile_y;
    //ZObject* entered_target;
    int entered_target_ref_id;
    bool build_unit;
    unsigned char bot, boid;
    bool auto_repair;
    bool set_crane_anim;
    bool crane_anim_on;
    int crane_rep_ref_id;
    //ZObject* entered_repair_building;
    int entered_repair_building_ref_id;
    bool repair_unit;
    unsigned char rot, roid;
    int rdriver_type;
    vector<driver_info_s> rdriver_info;
    vector<waypoint> rwaypoint_list;
    bool recheck_lid_status;
    //ZObject* destroy_fort_building;
    int destroy_fort_building_ref_id;
    bool updated_grenade_amount;
    bool updated_leader_grenade_amount;
    //ZObject* delete_grenade_box;
    int delete_grenade_box_ref_id;
    bool do_pickup_grenade_anim;
    int portrait_anim_ref_id;
    int portrait_anim_value;

    void clear()
    {
        updated_location = false;
        updated_velocity = false;
        updated_waypoints = false;
        updated_attack_object = false;
        updated_attack_object_health = false;
        updated_attack_object_driver_health = false;
        updated_open_lid = false;
        fired_missile = false;
        build_unit = false;
        //entered_target = nullptr;
        entered_target_ref_id = -1;
        auto_repair = false;
        set_crane_anim = false;
        crane_anim_on = false;
        crane_rep_ref_id = -1;
        //entered_repair_building = nullptr;
        entered_repair_building_ref_id = -1;
        repair_unit = false;
        //destroy_fort_building = nullptr;
        destroy_fort_building_ref_id = -1;
        updated_grenade_amount = false;
        updated_leader_grenade_amount = false;
        //delete_grenade_box = nullptr;
        delete_grenade_box_ref_id = -1;
        do_pickup_grenade_anim = false;
        portrait_anim_ref_id = -1;
        portrait_anim_value = -1;
    }
};




#define ZPORTRAIT_BASE_WIDTH 86
#define ZPORTRAIT_BASE_HEIGHT 74

#define MAX_EYES 11
#define MAX_HANDS 9
#define MAX_MOUTHS 16

enum look_direction
{
    LOOK_STRAIGHT, LOOK_RIGHT, LOOK_LEFT, MAX_LOOK_DIRECTIONS
};

enum portrait_anim
{
    YES_SIR_ANIM, YES_SIR3_ANIM, UNIT_REPORTING1_ANIM,
    UNIT_REPORTING2_ANIM, GRUNTS_REPORTING_ANIM,
    PSYCHOS_REPORTING_ANIM, SNIPERS_REPORTING_ANIM,
    TOUGHS_REPORTING_ANIM, LASERS_REPORTING_ANIM,
    PYROS_REPORTING_ANIM, WERE_ON_OUR_WAY_ANIM,
    HERE_WE_GO_ANIM, YOUVE_GOT_IT_ANIM, MOVING_IN_ANIM,
    OKAY_ANIM, ALRIGHT_ANIM, NO_PROBLEM_ANIM, OVER_N_OUT_ANIM,
    AFFIRMATIVE_ANIM, GOING_IN_ANIM, LETS_DO_IT_ANIM,
    LETS_GET_EM_ANIM, WERE_UNDER_ATTACK_ANIM,
    I_SAID_WERE_UNDER_ATTACK_ANIM, HELP_HELP_ANIM,
    THEYRE_ALL_OVER_US_ANIM, WERE_LOSEING_IT_ANIM,
    AAAHHH_ANIM, FOR_CHRIST_SAKE_ANIM,
    YOURE_JOKING_ANIM, TARGET_DESTROYED_ANIM, BLINK_ANIM,
    WINK_ANIM, SURPRISE_ANIM, ANGER_ANIM, GRIN_ANIM,
    SCARED_ANIM, EYES_LEFT_ANIM, EYES_RIGHT_ANIM,
    EYES_UP_ANIM, EYES_DOWN_ANIM, WHISTLE_ANIM,
    LOOK_LEFT_ANIM, LOOK_RIGHT_ANIM, SALUTE_ANIM,
    THUMBS_UP_ANIM, YES_SIR_SALUTE_ANIM, GOING_IN_THUMBS_UP_ANIM,
    FORGET_IT_ANIM, GET_OUTTA_HERE_ANIM, NO_WAY_ANIM,
    GOOD_HIT_ANIM, NICE_ONE_ANIM, OH_YEAH_ANIM, GOTCHA_ANIM,
    SMOKIN_ANIM, COOL_ANIM, WIPE_OUT_ANIM, TERRITORY_TAKEN_ANIM,
    FIRE_EXTINGUISHED_ANIM, GUN_CAPTURED_ANIM, VEHICLE_CAPTURED_ANIM,
    GRENADES_COLLECTED_ANIM, ENDW1_ANIM, ENDW2_ANIM, ENDW3_ANIM,
    ENDL1_ANIM, ENDL2_ANIM, ENDL3_ANIM,
    MAX_PORTRAIT_ANIMS
};

const string portrait_anim_string[MAX_PORTRAIT_ANIMS] =
{
    "YES_SIR_ANIM", "YES_SIR3_ANIM", "UNIT_REPORTING1_ANIM",
    "UNIT_REPORTING2_ANIM", "GRUNTS_REPORTING_ANIM",
    "PSYCHOS_REPORTING_ANIM", "SNIPERS_REPORTING_ANIM",
    "TOUGHS_REPORTING_ANIM", "LASERS_REPORTING_ANIM",
    "PYROS_REPORTING_ANIM", "WERE_ON_OUR_WAY_ANIM",
    "HERE_WE_GO_ANIM", "YOUVE_GOT_IT_ANIM", "MOVING_IN_ANIM",
    "OKAY_ANIM", "ALRIGHT_ANIM", "NO_PROBLEM_ANIM", "OVER_N_OUT_ANIM",
    "AFFIRMATIVE_ANIM", "GOING_IN_ANIM", "LETS_DO_IT_ANIM",
    "LETS_GET_EM_ANIM", "WERE_UNDER_ATTACK_ANIM",
    "I_SAID_WERE_UNDER_ATTACK_ANIM", "HELP_HELP_ANIM",
    "THEYRE_ALL_OVER_US_ANIM", "WERE_LOSEING_IT_ANIM",
    "AAAHHH_ANIM", "FOR_CHRIST_SAKE_ANIM",
    "YOURE_JOKING_ANIM", "TARGET_DESTROYED_ANIM", "BLINK_ANIM",
    "WINK_ANIM", "SURPRISE_ANIM", "ANGER_ANIM", "GRIN_ANIM",
    "SCARED_ANIM", "EYES_LEFT_ANIM", "EYES_RIGHT_ANIM",
    "EYES_UP_ANIM", "EYES_DOWN_ANIM", "WHISTLE_ANIM",
    "LOOK_LEFT_ANIM", "LOOK_RIGHT_ANIM", "SALUTE_ANIM",
    "THUMBS_UP_ANIM", "YES_SIR_SALUTE_ANIM", "GOING_IN_THUMBS_UP_ANIM",
    "FORGET_IT_ANIM", "GET_OUTTA_HERE_ANIM", "GOOD_HIT_ANIM",
    "NO_WAY_ANIM", "NICE_ONE_ANIM", "OH_YEAH_ANIM", "GOTCHA_ANIM",
    "SMOOKIN_ANIM", "COOL_ANIM", "WIPE_OUT_ANIM", "TERRITORY_TAKEN_ANIM",
    "FIRE_EXTINGUISHED_ANIM", "GUN_CAPTURED_ANIM", "VEHICLE_CAPTURED_ANIM",
    "GRENADES_COLLECTED_ANIM", "ENDW1_ANIM", "ENDW2_ANIM", "ENDW3_ANIM",
    "ENDL1_ANIM", "ENDL2_ANIM", "ENDL3_ANIM",
};

class QZOD_DNOBJECTSSHARED_EXPORT ZPortrait_Frame
{
public:
    ZPortrait_Frame()
    {
        look_direction = LOOK_STRAIGHT;
        mouth = 0;
        eyes = 0;
        hand_do_render = false;
        head_x = 4;
        head_y = 2;
    }

    char look_direction;
    char mouth;
    char eyes;
    char hand;
    char hand_x;
    char hand_y;
    bool hand_do_render;
    char head_x;
    char head_y;
    double duration;
};


#endif // ZOD_OBJ_STRUCTURES_H

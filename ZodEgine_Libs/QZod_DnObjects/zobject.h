#ifndef ZOBJECT_H
#define ZOBJECT_H
// ==================================================================
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// ==================================================================
#include "qzod_dnobjects_global.h"

#include "cursor.h"
#include "zod_obj_structures.h"



//gui
//#include "zgui_window.h"
//#include "gwproduction.h"
class ZGuiWindow;



//#include "zolists.h"
//#include "zportrait.h"
class ZPortrait;
class ZOLists;



// ==================================================================
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// ==================================================================
class QZOD_DNOBJECTSSHARED_EXPORT ZObject
{
	public:
		ZObject(ZTime *ztime_, ZSettings *zsettings_ = nullptr);
		virtual ~ZObject();

		static void Init(TTF_Font *ttf_font);
		static void SetDamageMissileList(vector<damage_missile> *damage_missile_list_) { damage_missile_list = damage_missile_list_; }
		
		string GetObjectName();
		void SetMap(ZMap *zmap_);
		void SetEffectList(vector<ZEffect*> *effect_list_) { effect_list = effect_list_; }
		virtual void SetOwner(team_type owner_);
		inline team_type GetOwner() { return owner; }
		inline void GetDimensions(int &w, int &h) { w = width; h = height; }
		inline void GetDimensionsPixel(int &w_pix, int &h_pix) { w_pix = width_pix; h_pix = height_pix; }
		virtual int Process();
		int ProcessObject();
// 		virtual SDL_Surface *GetRender();
		void RenderWaypointLine(int sx, int sy, int ex, int ey, int view_h, int view_w);
		virtual void DoRenderWaypoints(ZMap &the_map, SDL_Surface *dest, vector<ZObject*> &object_list, bool is_rally_points = false, int shift_x = 0, int shift_y = 0);
		virtual void DoPreRender(ZMap &the_map, SDL_Surface *dest, int shift_x = 0, int shift_y = 0);
		virtual void DoRender(ZMap &the_map, SDL_Surface *dest, int shift_x = 0, int shift_y = 0);
		virtual void DoAfterEffects(ZMap &the_map, SDL_Surface *dest, int shift_x = 0, int shift_y = 0);
		void SetDirection(int direction_);
		virtual void PlaySelectedWav();
		virtual void PlaySelectedAnim(ZPortrait &portrait);
		virtual void PlayAcknowledgeWav();
		virtual void PlayAcknowledgeAnim(ZPortrait &portrait, bool no_way = false);
		bool IntersectsObject(ZObject &obj);
		double DistanceFromObject(ZObject &obj);
		double DistanceFromCoords(int x, int y);
		ZObject* NearestObjectFromList(vector<ZObject*> &the_list);
		static void RemoveObjectFromList(ZObject* the_object, vector<ZObject*> &the_list);
		static ZObject* NextSelectableObjectAboveID(vector<ZObject*> &the_list, int unit_type, int only_team, int min_ref_id);
		static ZObject* NearestSelectableObject(vector<ZObject*> &the_list, int unit_type, int only_team, int x, int y);
		static ZObject* NearestObjectToCoords(vector<ZObject*> &the_list, int x, int y);
		static void ClearAndDeleteList(vector<ZObject*> &the_list);
		static void ProcessList(vector<ZObject*> &the_list);
		virtual void FireMissile(int x_, int y_);
		virtual void FireTurrentMissile(int x_, int y_, double offset_time);
		virtual ZGuiWindow *MakeGuiWindow();
		void SetBuildList(ZBuildList *buildlist_);
		void SetUnitLimitReachedList(bool *unit_limit_reached_);
		virtual void SetLevel(int level_);
		virtual int GetLevel();
		virtual int GetBuildState();
		virtual void AddGroupMinion(ZObject *obj);
		virtual void RemoveGroupMinion(ZObject *obj);
		virtual void SetGroupLeader(ZObject *obj);
		virtual ZObject* GetGroupLeader();
		virtual vector<ZObject*> &GetMinionList();
		virtual void ClearGroupInfo();
		virtual void CreateGroupInfoData(char *&data, int &size);
		virtual void ProcessGroupInfoData(char *data, int size, vector<ZObject*> &object_list);
		virtual void CloneMinionWayPoints();
		virtual bool IsMinion();
		virtual bool IsApartOfAGroup();
		virtual bool CanAttack();
		virtual bool HasExplosives();
		virtual bool AttackedOnlyByExplosives();
		virtual bool CanBeRepairedByCrane(int repairers_team);
		virtual bool GetCraneEntrance(int &x, int &y, int &x2, int &y2);
		virtual bool GetCraneCenter(int &x, int &y);
		virtual void SetDriverType(int driver_type_);
		virtual void AddDriver(int driver_health_);
		virtual void AddDriver(driver_info_s new_driver);
		virtual vector<driver_info_s> &GetDrivers();
		virtual void ClearDrivers();
		virtual int GetDriverType();
		virtual int GetDriverHealth();
		virtual bool CanEjectDrivers();
		virtual void SetInitialDrivers();
		virtual void SetEjectableCannon(bool ejectable_);
		virtual void ResetDamageInfo();
		virtual unsigned short GetExtraLinks();
		void DoHitEffect();
		virtual void DoCraneAnim(bool on_, ZObject *rep_obj = nullptr);
		virtual void DoRepairBuildingAnim(bool on_, double remaining_time_);
		virtual bool CanBeRepaired();
		virtual bool CanRepairUnit(int units_team);
		virtual bool RepairingAUnit();
		virtual bool GetRepairEntrance(int &x, int &y);
		virtual bool GetRepairCenter(int &x, int &y);
		virtual bool SetRepairUnit(ZObject *unit_obj);
		virtual void SetLidState(bool lid_open_);
		virtual bool GetLidState();
		virtual bool CanSnipe();
		virtual bool CanBeSniped();
		virtual bool HasLid();
		bool HasProcessedDeath();
		void SetHasProcessedDeath(bool processed_death_);
		void DoDriverHitEffect();
		virtual bool CanBeEntered();
		void SetDamagedByFireTime(double the_time);
		void SetDamagedByMissileTime(double the_time);
		double GetDamagedByFireTime();
		double GetDamagedByMissileTime();
		virtual bool CanEnterFort(int team);
		virtual bool CanSetWaypoints();
		virtual bool CanSetRallypoints() { return false; }
		virtual bool CanMove();
		virtual bool CanPickupGrenades() { return false; }
		virtual bool CanHaveGrenades() { return false; }
		virtual void SetGrenadeAmount(int grenade_amount_) { return; }
		virtual int GetGrenadeAmount() { return 0; }
		virtual bool CanThrowGrenades() { return false; }
		virtual void DoPickupGrenadeAnim() { }
		bool CanAttackObject(ZObject *obj);
		virtual void TryDropTracks() {}
		virtual bool AddBuildingQueue(unsigned char ot, unsigned char oid, bool push_to_front = true) { return false; }
		virtual bool CancelBuildingQueue(int list_i, unsigned char ot, unsigned char oid) { return false; }
		virtual bool ProducesUnits() { return false; }
		
		static int DirectionFromLoc(float dx, float dy);
		void SetCords(int x, int y);
		inline void GetCords(int &x, int &y) { x = loc.x; y = loc.y; }
		inline void GetCenterCords(int &x, int &y) { x = center_x; y = center_y; }
		virtual void SetDestroyed(bool is_destroyed);
		void GetObjectID(unsigned char &object_type_, unsigned char &object_id_);
		bool Selectable();
		void SetRefID(int id);
		void ShowWaypoints();
		int GetRefID();
		vector<waypoint> &GetWayPointList();
		vector<waypoint> &GetWayPointDevList();
		vector<waypoint> &GetRallyPointList() { return rallypoint_list; }
		void RenderHover(ZMap &zmap, SDL_Surface *dest, team_type viewers_team);
		void RenderHealth(ZMap &zmap, SDL_Surface *dest);
		void RenderAttackRadius(ZMap &zmap, SDL_Surface *dest, vector<ZObject*> &avoid_list);
		virtual void RenderSelection(ZMap &zmap, SDL_Surface *dest);
		virtual bool UnderCursor(int &map_x, int &map_y);
		virtual bool UnderCursorCanAttack(int &map_x, int &map_y) { return true; }
		virtual bool UnderCursorFortCanEnter(int &map_x, int &map_y) { return false; }
		virtual bool WithinSelection(int &map_left, int &map_right, int &map_top, int &map_bottom);
		virtual bool CannonNotPlacable(int &map_left, int &map_right, int &map_top, int &map_bottom);
		void SetLoc(object_location new_loc);
		void SetGroup(int group_num_);
		int GetHealth();
		int GetMaxHealth();
		void SetHealth(int new_health, ZMap &tmap);
		void SetHealthPercent(int health_percent, ZMap &tmap);
		int GetInitialHealthPercent() { return initial_health_percent; }
		int DamageHealth(int damage_amount, ZMap &tmap);
		int DamageDriverHealth(int damage_amount);
		virtual void DoDeathEffect(bool do_fire_death, bool do_missile_death);
		virtual void DoReviveEffect();
		virtual bool IsDestroyed();
		bool WithinAgroRadius(int ox, int oy);
		bool WithinAgroRadius(ZObject *obj);
		bool WithinAutoEnterRadius(int ox, int oy);
		bool WithinAutoGrabFlagRadius(int ox, int oy);
		bool WithinAttackRadius(int ox, int oy);
		bool WithinAttackRadius(ZObject *obj);
		bool WithinAttackRadiusOf(vector<ZObject*> &avoid_list, int ox, int oy);
		static ZObject* GetObjectFromID_BS(int ref_id, vector<ZObject*> &the_list);
		static ZObject* GetObjectFromID(int ref_id, vector<ZObject*> &the_list);
		bool StopMove();
		virtual void SetAttackObject(ZObject *obj);
		ZObject* GetAttackObject();
		void RemoveObject(ZObject *obj);
		void SmoothMove(double &the_time);
		ZSDL_Surface &GetHoverNameImg();
		static string GetHoverName(unsigned char ot, unsigned char oid);
		static ZSDL_Surface &GetHoverNameImgStatic(unsigned char ot, unsigned char oid);
		virtual void ProcessSetBuildingStateData(char *data, int size);
		virtual void ProcessSetBuiltCannonData(char *data, int size);
		virtual bool GetBuildingCreationPoint(int &x, int &y);
		virtual bool GetBuildingCreationMovePoint(int &x, int &y);
		virtual bool GetBuildUnit(unsigned char &ot, unsigned char &oid);
		virtual map_zone_info *GetConnectedZone();
		virtual void SetConnectedZone(map_zone_info *connected_zone_);
		virtual void SetConnectedZone(ZMap &the_map);
		void DontStamp(bool dont_stamp_);
		virtual void SetMapImpassables(ZMap &tmap);
		virtual void UnSetMapImpassables(ZMap &tmap);
		virtual void SetDestroyMapImpassables(ZMap &tmap);
		virtual void UnSetDestroyMapImpassables(ZMap &tmap);
		virtual void SetupRockRender(bool **rock_list, int map_w, int map_h);
		virtual bool CanBeDestroyed();
		virtual void CreationMapEffects(ZMap &tmap);
		virtual void DeathMapEffects(ZMap &tmap);
		virtual bool ResetBuildTime(float zone_ownage);
		virtual void SetZoneOwnage(float zone_ownage_) {;}
		virtual bool RecalcBuildTime();
		double SpeedOffsetPercent();
		double SpeedOffsetPercentInv() { return 1.0 / SpeedOffsetPercent(); }
		

		//server only stuff
		virtual int ProcessServer(ZMap &tmap, ZOLists &ols);//vector<ZObject*> &object_list);
		virtual void ProcessServerLid();
		virtual void ProcessAttackDamage(ZMap &tmap, bool attack_player_given);
		virtual void ProcessRunStamina(double time_dif);
		server_flag &GetSFlags();
		void SetVelocity(ZObject *target_object = nullptr);
		void CreateLocationData(char *&data, int &size);
		void CreateAttackObjectData(char *&data, int &size);
		void ProcessMoveWP(vector<waypoint>::iterator &wp, double time_dif, bool is_new, ZOLists &ols, ZMap &tmap, bool stoppable);
		void ProcessDodgeWP(vector<waypoint>::iterator &wp, double time_dif, bool is_new, ZOLists &ols, ZMap &tmap);
		void ProcessAttackWP(vector<waypoint>::iterator &wp, double time_dif, bool is_new, ZOLists &ols, ZMap &tmap);
		void ProcessAgroWP(vector<waypoint>::iterator &wp, double time_dif, bool is_new, ZOLists &ols, ZMap &tmap);
		void ProcessPickupWP(vector<waypoint>::iterator &wp, double time_dif, bool is_new, ZOLists &ols, ZMap &tmap);
		void ProcessEnterWP(vector<waypoint>::iterator &wp, double time_dif, bool is_new, ZOLists &ols, ZMap &tmap);
		void ProcessEnterFortWP(vector<waypoint>::iterator &wp, double time_dif, bool is_new, ZOLists &ols, ZMap &tmap);
		void ProcessCraneRepairWP(vector<waypoint>::iterator &wp, double time_dif, bool is_new, ZOLists &ols, ZMap &tmap);
		void ProcessUnitRepairWP(vector<waypoint>::iterator &wp, double time_dif, bool is_new, ZOLists &ols, ZMap &tmap);
		bool CheckAttackTo(vector<waypoint>::iterator &wp, ZOLists &ols);
		void KillWP(vector<waypoint>::iterator &wp);
		bool CanOverwriteWP();
		bool DoAttackImpassableAtCoords(ZOLists &ols, int x, int y);
		bool DodgeMissile(int tx, int ty, double time_till_explode);
		bool ProcessMoveOrKillWP(double time_dif, ZMap &tmap, vector<waypoint>::iterator &wp, ZOLists &ols, bool stoppable = true);
		bool ProcessMove(double time_dif, ZMap &tmap, bool stoppable = true) { int sx, sy; return ProcessMove(time_dif, tmap, sx, sy, stoppable); }
		bool ProcessMove(double time_dif, ZMap &tmap, int &stop_x, int &stop_y, bool stoppable = true);
		bool ReachedTarget();
		void SetTarget(int x, int y);
		void SetTarget() { SetTarget(cur_wp_info.x, cur_wp_info.y); }
		bool KillMe(double the_time);
		void DoKillMe(double killtime = 0);
		void Engage(ZObject *attack_object_);
		bool Disengage();
		void CheckPassiveEngage(double &the_time, ZOLists &ols);
		bool IsMoving();
		//virtual bool ServerFireTurrentMissile(int &x_, int &y_, int &damage, int &radius, double &offset_time);
		virtual vector<fire_missile_info> ServerFireTurrentMissile(int &damage, int &radius);
		virtual bool SetBuildingDefaultProduction() { return false; }
		virtual bool SetBuildingProduction(unsigned char ot, unsigned char oid) { return false; }
		virtual void CreateBuildingStateData(char *&data, int &size) { data = nullptr; size = 0; }
		virtual void CreateBuildingQueueData(char *&data, int &size) { data = nullptr; size = 0; }
		virtual void ProcessBuildingQueueData(char *data, int size) {}
		virtual bool StopBuildingProduction(bool clear_queue_list = true) { return false; }
		virtual bool BuildUnit(double &the_time, unsigned char &ot, unsigned char &oid);
		virtual bool RepairUnit(double &the_time, unsigned char &ot, unsigned char &oid, int &driver_type_, vector<driver_info_s> &driver_info_, vector<waypoint> &rwaypoint_list_);
		virtual bool StoreBuiltCannon(unsigned char oid);
		virtual void CreateBuiltCannonData(char *&data, int &size) { data = nullptr; size = 0; }
		virtual void CreateTeamData(char *&data, int &size);
		virtual void CreateRepairAnimData(char *&data, int &size, bool play_sound = true) { data = nullptr; size = 0; }
		virtual void ResetProduction();
		virtual bool RemoveStoredCannon(unsigned char oid);
		virtual bool HaveStoredCannon(unsigned char oid);
		virtual int CannonsInZone(ZOLists &ols);
		virtual vector<unsigned char> &GetBuiltCannonList();
		bool DoAutoRepair(ZMap &tmap, ZOLists &ols);
		void StopAutoRepair();
		bool HasDestroyedFortInZone(ZOLists &ols);
		virtual void ProcessKillObject();
		void InitRealMoveSpeed(ZMap &tmap);
		void PostPathFindingResult(ZPath_Finding_Response* response);
		virtual void SignalLidShouldOpen();
		virtual void SignalLidShouldClose();
		bool EstimateMissileTarget(ZObject *target, int &x, int &y);
		void SetJustLeftCannon(bool just_left_cannon_) { just_left_cannon = just_left_cannon_; }
		virtual bool NearestAttackLoc(int sx, int sy, int &ex, int &ey, int aa_radius, bool is_robot, ZMap &tmap);
		virtual bool IsDestroyableImpass() { return false; }
		virtual bool CausesImpassAtCoord(int x, int y) { return false; }
		double DamagedSpeed();
		double RunSpeed();
		bool CanReachTargetRunning(int x, int y);
		virtual bool ShowDamaged() { return false; }
		virtual bool ShowPartiallyDamaged() { return false; }
		void AttemptStartRun(int x, int y) { if(CanReachTargetRunning(x,y)) AttemptStartRun(); }
		void AttemptStartRun();

		//AI stuff
		vector<ZObject*> &GetAIList() { return ai_list; }
		vector<ZObject*> &GetAIOrigList() { return ai_orig_list; }
		void AddToAIList(ZObject *obj) { ai_list.push_back(obj); obj->GetAIList().push_back(this); }
	private:
		
	protected:
		void InitTypeId(unsigned char ot, unsigned char oid);
		virtual void RecalcDirection();

		static ZSettings default_zsettings;

		static ZSDL_Surface group_tag[10];
		//static Mix_Chunk *selected_wav[6];
		//static Mix_Chunk *selected_robot_wav[MAX_ROBOT_TYPES];
		//static Mix_Chunk *acknowledge_wav[12];

		static vector<damage_missile> *damage_missile_list;

		ZBuildList *buildlist;
		ZSettings *zsettings;
		ZMap *zmap;
		ZTime *ztime;
		vector<ZEffect*> *effect_list;

		bool can_be_destroyed;
		bool destroyed;
		double last_process_time;
		double next_process_time;
		double process_time_int;
		double next_waypoint_time;
		double render_death_time;
		double last_process_server_time;
		double next_move_time;
		double last_radius_time;
		double next_attack_time;
		double last_loc_set_time;
		double next_check_passive_attack_time;
		double next_loc_update_time;
		double loc_update_int;
		object_location loc;
		object_location last_loc;
		int center_x, center_y;
		float xover, yover;
		server_flag sflags;
		int move_speed;
		double real_move_speed;
		int width, height;
		int width_pix, height_pix;
		team_type owner;
		string object_name;
		int mode;
		int direction;
		unsigned char object_type;
		unsigned char object_id;
		bool selectable;
		int ref_id;
		int waypoint_i;
		int move_i;
		int attack_i;
		bool show_waypoints;
		vector<waypoint> waypoint_list;
		vector<waypoint> waypoint_dev_list;
		vector<waypoint> rallypoint_list;
		ZCursor waypoint_cursor;
		waypoint_information cur_wp_info;
		int group_num;
		string hover_name;
		ZSDL_Surface hover_name_img;
		ZSDL_Surface hover_name_star_img;
		int hover_name_x_shift;
		int hover_name_star_x_shift;
		double radius_i;
		int max_health;
		int health;
		map_zone_info *connected_zone;
		bool *unit_limit_reached;
		bool dont_stamp;
		bool has_explosives;
		bool attacked_by_explosives;
		bool do_hit_effect;
		bool do_driver_hit_effect;
		bool can_snipe;
		bool has_lid;
		bool processed_death;
		bool can_be_sniped;
		double last_damaged_by_fire_time;
		double last_damaged_by_missile_time;
		bool just_left_cannon;
		int initial_health_percent;
		double next_drop_track_time;
		double max_stamina;
		double stamina;
		bool is_running;

		//driver
		int driver_type;
		vector<driver_info_s> driver_info;

		//attacking stuff
		ZObject *attack_object;
		double next_damage_time;
		double damage_int_time;
		double damage_chance;
		int damage;
		bool damage_is_missile;
		int damage_radius;
		int missile_speed;
		int attack_radius;
		double snipe_chance;

		//server only stuff
		bool killme;
		double killme_time;
		waypoint last_wp;

		//group stuff
		vector<ZObject*> minion_list;
		ZObject *leader_obj;

		//AI Stuff
		vector<ZObject*> ai_list;
		vector<ZObject*> ai_orig_list;

		bool do_auto_repair;
		double next_auto_repair_time;

		friend bool sort_objects_func (ZObject *a, ZObject *b);
};
// ==================================================================
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// ==================================================================
extern bool sort_objects_func (ZObject *a, ZObject *b);

// ==================================================================
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// ==================================================================
class QZOD_DNOBJECTSSHARED_EXPORT ZOLists
{
public:
    ZOLists();

    void Init(vector<ZObject*> *object_list_) { object_list = object_list_; }

    void AddObject(ZObject *obj);
    void DeleteObject(ZObject *obj);
    void RemoveObject(ZObject *obj);
    static void DeleteObjectFromList(ZObject *obj, vector<ZObject*> *olist);
    static void RemoveObjectFromList(ZObject *obj, vector<ZObject*> *olist);
    void DeleteAllObjects();

    void SetupFlagList();

    vector<ZObject*> *object_list;
    vector<ZObject*> flag_olist;
    vector<ZObject*> cannon_olist;
    vector<ZObject*> building_olist;
    vector<ZObject*> rock_olist;
    vector<ZObject*> passive_engagable_olist;
    vector<ZObject*> mobile_olist;
    vector<ZObject*> prender_olist;
    vector<ZObject*> non_mapitem_olist;
    vector<ZObject*> grenades_olist;
};

// ==================================================================
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// ==================================================================
class QZOD_DNOBJECTSSHARED_EXPORT ZPortrait_Anim
{
public:
    ZPortrait_Anim();

    void AddFrame(ZPortrait_Frame &new_frame);

    vector<ZPortrait_Frame> frame_list;
    double total_duration;

    ZPortrait_Anim& operator=(const ZPortrait_Anim &rhs)
    {
        if(this == &rhs) return *this;

        frame_list = rhs.frame_list;
        total_duration = rhs.total_duration;

        return *this;
    }
};

class QZOD_DNOBJECTSSHARED_EXPORT ZPortrait_Unit_Graphics
{
public:
    void Load(robot_type oid, team_type team, ZPortrait_Unit_Graphics &bc);

    ZSDL_Surface head[MAX_LOOK_DIRECTIONS];
    ZSDL_Surface eyes[MAX_EYES];
    ZSDL_Surface hand[MAX_HANDS];
    ZSDL_Surface mouth[MAX_MOUTHS];
    ZSDL_Surface shoulders;
};

class QZOD_DNOBJECTSSHARED_EXPORT ZPortrait
{
public:
    ZPortrait();

    static void Init();

    void SetCords(int x_, int y_);
    void SetOverMap(bool over_map_);
    void SetTerrainType(planet_type terrain_);
    void SetInVehicle(bool in_vehicle_);
    void SetRobotID(unsigned char oid_);
    void SetTeam(int team_);
    void SetRefID(int ref_id_) { ref_id = ref_id_; }
    int GetRefID() { return ref_id; }
    void SetObject(ZObject *obj);
    void SetDoRandomAnims(bool do_random_anims_) { do_random_anims = do_random_anims_; }
    void ClearRobotID();

    void StartAnim(int anim_);
    bool DoingAnim();

    int Process();
    void DoRender();

private:
    static void SetupFrames();

    int GetBlitInfo(SDL_Surface *src, int x, int y, SDL_Rect &from_rect, SDL_Rect &to_rect);

    void StartRandomAnim();
    void RenderFace();
    void PlayAnimSound();

    static bool finished_init;

    static ZSDL_Surface backdrop[MAX_PLANET_TYPES];
    static ZSDL_Surface backdrop_vehicle;
    static ZPortrait_Unit_Graphics unit_graphics[MAX_ROBOT_TYPES][MAX_TEAM_TYPES];
    static ZPortrait_Anim anim_info[MAX_PORTRAIT_ANIMS];

    //frames
    static ZPortrait_Frame still_frame;

    int x, y;
    bool over_map;
    bool in_vehicle;
    bool do_render;
    unsigned char oid;
    int team;
    planet_type terrain;
    int ref_id;
    bool do_random_anims;

    int cur_anim;
    double anim_start_time;
    ZPortrait_Frame *render_frame;
    double next_random_anim_time;
};






// ==================================================================
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// ==================================================================
#endif

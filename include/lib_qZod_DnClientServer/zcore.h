#ifndef ZCORE_H
#define ZCORE_H


#include "qzod_dnclientserver_global.h"
#include "socket_handler.h"


class QZOD_DNCLIENTSERVERSHARED_EXPORT ZCore
{
	public:
		ZCore();
        virtual ~ZCore() {}

		virtual void Setup();
		virtual void Run();
		void SetBotBypassData(char *data, int size);
		static void CreateRandomBotBypassData(char *data, int &size);
		virtual void AllowRun(bool allow_run_ = true) { allow_run = allow_run_; }
	protected:
		void InitEncryption();
		void SetupRandomizer();
		bool CheckRegistration();
		int GetObjectIndex(ZObject* &the_object, vector<ZObject*> &the_list);
		static ZObject* GetObjectFromID_BS(int ref_id, vector<ZObject*> &the_list);
		static ZObject* GetObjectFromID(int ref_id, vector<ZObject*> &the_list);
		void CreateWaypointSendData(int ref_id, vector<waypoint> &waypoint_list, char* &data, int &size);
		ZObject* ProcessWaypointData(char *data, int size, bool is_server = false, int ok_team = -1);
		ZObject* ProcessRallypointData(char *data, int size, bool is_server = false, int ok_team = -1);
		virtual void DeleteObjectCleanUp(ZObject *obj);
		bool CannonPlacable(ZObject *building_obj, int tx, int ty);
		bool AreaIsFortTurret(int tx, int ty);
		void ResetUnitLimitReached();
		bool CheckUnitLimitReached();
		virtual void ResetZoneOwnagePercentages(bool notify_players = false);
		bool CheckWaypoint(ZObject *obj, waypoint *wp);
		bool CheckRallypoint(ZObject *obj, waypoint *wp);
		static bool UnitRequiresActivation(unsigned char ot, unsigned char oid);
		bool CreateObjectOk(unsigned char ot, unsigned char oid, int x, int y, int owner, int blevel, unsigned short extra_links);

		//voting stuff
		int VotesNeeded();
		int VotesFor();
		int VotesAgainst();
		string VoteAppendDescription();

		ZMap zmap;
		ZBuildList buildlist;
		ZSettings zsettings;
		ZUnitRating zunitrating;
		ZEncryptAES zencrypt;
		ZOLists ols;

		ZTime ztime;
		ZVote zvote;
		
		vector<p_info> player_info;
		vector<ZObject*> object_list;
		//vector<string> player_name;
		//vector<team_type> player_team;
		vector<string> selectable_map_list;

		bool allow_run;

		bool unit_limit_reached[MAX_TEAM_TYPES];
		int team_units_available[MAX_TEAM_TYPES];
		float team_zone_percentage[MAX_TEAM_TYPES];

		int max_units_per_team;

		char bot_bypass_data[MAX_BOT_BYPASS_SIZE];
		int bot_bypass_size;

		bool is_registered;
};

#endif

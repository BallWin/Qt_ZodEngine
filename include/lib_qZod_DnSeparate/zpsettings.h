#ifndef ZPSETTINGS_H
#define ZPSETTINGS_H

#include "qzod_dnseparate_global.h"

using namespace std;

class QZOD_DNSEPARATESHARED_EXPORT ZPSettings
{
public:
	ZPSettings();

	void LoadDefaults();
	bool LoadSettings(string filename);
	bool SaveSettings(string filename);

	bool loaded_from_file;

	bool ignore_activation;
	bool require_login;
	bool use_database;
	bool use_mysql;
	bool start_map_paused;
	bool bots_start_ignored;
	bool allow_game_speed_change;
	string selectable_map_list;

	string mysql_root_password;
	string mysql_user_name;
	string mysql_user_password;
	string mysql_hostname;
	string mysql_dbname;
};

#endif

#include "zobject.h"

ZSettings ZObject::default_zsettings;

ZSDL_Surface ZObject::group_tag[10];

//vector<ZEffect*> *ZObject::effect_list = nullptr;
vector<damage_missile> *ZObject::damage_missile_list = nullptr;

ZObject::ZObject(ZTime *ztime_, ZSettings *zsettings_)
{
	if(!zsettings_)
		zsettings = &default_zsettings;
	else
		zsettings = zsettings_;

	ztime = ztime_;

	zmap = nullptr;
	effect_list = nullptr;

	double &the_time = ztime->ztime;

	object_name = "object";
	owner = NULL_TEAM;
	last_process_time = the_time;
	width = 0;
	height = 0;
	width_pix = 0;
	height_pix = 0;
	mode = NULL_MODE;
	destroyed = false;
	direction = 4;
	selectable = false;
	ref_id = -1;
	waypoint_i = 0;
	attack_i = 0;
	next_waypoint_time = 0;
	render_death_time = 0;
	show_waypoints = false;
	loc.x = loc.y = 0;
	loc.dx = loc.dy = 0;
	center_x = center_y = 0;
	move_speed = 0;
	real_move_speed = 0;
	xover = yover = 0;
	last_process_server_time = last_process_time;
	last_radius_time = last_process_time;
	last_loc_set_time = last_process_time;
	waypoint_cursor.SetTeam(NULL_TEAM);
	group_num = -1;
	//hover_name_img = nullptr;
	//hover_name_star_img = nullptr;
	attack_radius = 0;
	radius_i = 0;
	max_health = 1;
	health = max_health;
	last_wp.mode = -1;
	next_check_passive_attack_time = 0;
	buildlist = nullptr;
	connected_zone = nullptr;
	unit_limit_reached = nullptr;
	dont_stamp = false;
	do_auto_repair = false;
	can_be_destroyed = true;
	has_explosives = false;
	attacked_by_explosives = false;
	do_hit_effect = false;
	do_driver_hit_effect = false;
	can_snipe = false;
	has_lid = false;
	processed_death = false;
	can_be_sniped = false;
	last_damaged_by_fire_time = 0;
	last_damaged_by_missile_time = 0;
	just_left_cannon = false;
	initial_health_percent = 100;
	next_drop_track_time = 0;
	max_stamina = 0;
	stamina = max_stamina;

	driver_type = GRUNT;

	//to make sure loc updates from the server are not over frequent
	loc_update_int = 0.8 + (0.001 * (rand() % 25));
	next_loc_update_time = 0;

	//attack stuff
	attack_object = nullptr;
	next_damage_time = 0;
	damage_int_time = 0;
	damage_chance = 0;
	damage = 0;
	damage_is_missile = false;
	damage_radius = 1;
	snipe_chance = 0;

	//needed for when an object needs to be removed after some time
	killme = false;
	killme_time = 0;

	ClearGroupInfo();
}

ZObject::~ZObject()
{
	//kill any pathfinding threads
	if(zmap) cur_wp_info.clear_and_kill(*zmap);

	//alittle mem mangement
	//if(hover_name_img) SDL_FreeSurface(hover_name_img);
	//if(hover_name_star_img) SDL_FreeSurface(hover_name_star_img);
}

void ZObject::InitTypeId(unsigned char ot, unsigned char oid)
{
	object_type = ot;
	object_id = oid;

	if(!zsettings)
	{
		printf("ZObject::InitTypeId:zsettings not set\n");
		return;
	}

	if(ot == BUILDING_OBJECT)
	{
		switch(oid)
		{
		case FORT_FRONT:
		case FORT_BACK:
			max_health = zsettings->fort_building_health * MAX_UNIT_HEALTH;
			break;
		case RADAR:
			max_health = zsettings->radar_building_health * MAX_UNIT_HEALTH;
			break;
		case REPAIR:
			max_health = zsettings->repair_building_health * MAX_UNIT_HEALTH;
			break;
		case ROBOT_FACTORY:
			max_health = zsettings->robot_building_health * MAX_UNIT_HEALTH;
			break;
		case VEHICLE_FACTORY:
			max_health = zsettings->vehicle_building_health * MAX_UNIT_HEALTH;
			break;
		case BRIDGE_VERT:
		case BRIDGE_HORZ:
			max_health = zsettings->bridge_building_health * MAX_UNIT_HEALTH;
			break;
		}
	}
	else if (ot == MAP_ITEM_OBJECT)
	{
		switch(oid)
		{
		case ROCK_ITEM:
			max_health = zsettings->rock_item_health * MAX_UNIT_HEALTH;
			break;
		case GRENADES_ITEM:
			max_health = zsettings->grenades_item_health * MAX_UNIT_HEALTH;
			break;
		case ROCKETS_ITEM:
			max_health = zsettings->rockets_item_health * MAX_UNIT_HEALTH;
			break;
		case HUT_ITEM:
			max_health = zsettings->hut_item_health * MAX_UNIT_HEALTH;
			break;
		}

		if(oid >= MAP0_ITEM && oid < MAP0_ITEM + MAP_ITEMS_AMOUNT)
			max_health = zsettings->map_item_health * MAX_UNIT_HEALTH;
	}
	else
	{
		ZUnit_Settings &unit_settings = zsettings->GetUnitSettings(object_type, object_id);

		move_speed = unit_settings.move_speed;
		attack_radius = unit_settings.attack_radius;
		damage = unit_settings.attack_damage * MAX_UNIT_HEALTH;
		damage_chance = unit_settings.attack_damage_chance;
		damage_radius = unit_settings.attack_damage_radius;
		missile_speed = unit_settings.attack_missile_speed;
		snipe_chance = unit_settings.attack_snipe_chance;
		damage_int_time = unit_settings.attack_speed;
		max_health = unit_settings.health * MAX_UNIT_HEALTH;
		max_stamina = unit_settings.max_run_time;
	}

	//set health and stamina to full
	health = max_health;
	stamina = max_stamina;
}

void ZObject::Init(TTF_Font *ttf_font)
{
	int i;
	char filename_c[500];
	SDL_Color textcolor;

	for(i=0;i<10;i++)
	{
		textcolor.r = 200;
		textcolor.g = 200;
		textcolor.b = 200;

		sprintf(filename_c, "%d", i);
		//group_tag[i] = TTF_RenderText_Solid(ttf_font, filename_c, textcolor);
		group_tag[i].LoadBaseImage(TTF_RenderText_Solid(ttf_font, filename_c, textcolor));
	}

	//for(i=0;i<6;i++)
	//{
	//	sprintf(filename_c, "assets/sounds/selected_%02d.wav", i);
	//	selected_wav[i] = MIX_Load_Error(filename_c);
	//}

	//for(i=0;i<MAX_ROBOT_TYPES;i++)
	//{
	//	sprintf(filename_c, "assets/sounds/selected_%s.wav", robot_type_string[i].c_str());
	//	selected_robot_wav[i] = MIX_Load_Error(filename_c);
	//}

	//for(i=0;i<12;i++)
	//{
	//	sprintf(filename_c, "assets/sounds/acknowledge_%02d.wav", i);
	//	acknowledge_wav[i] = MIX_Load_Error(filename_c);
	//}
}

void ZObject::InitRealMoveSpeed(ZMap &tmap)
{
	int &x = loc.x;
	int &y = loc.y;

	//real_move_speed = move_speed * tmap.GetTileWalkSpeed(x + (width_pix >> 1), y + (height_pix >> 1));
	real_move_speed = move_speed * tmap.GetTileWalkSpeed(center_x, center_y);
}

void ZObject::SetMap(ZMap *zmap_)
{
	zmap = zmap_;
}

void ZObject::DontStamp(bool dont_stamp_)
{
	dont_stamp = dont_stamp_;
}

void ZObject::DoDeathEffect(bool do_fire_death, bool do_missile_death)
{

}

void ZObject::DoReviveEffect()
{

}

void ZObject::FireMissile(int x_, int y_)
{

}

bool ZObject::KillMe(double the_time)
{
	return (killme && the_time >= killme_time);
}

void ZObject::DoKillMe(double killtime)
{
	//they were already set to die
	//make sure they die at their original time
	if(killme) return;

	killme = true;
	killme_time = killtime;
}

bool ZObject::IsDestroyed()
{
	return (health <= 0 && max_health > 0);
}

int ZObject::GetHealth()
{
	return health;
}

int ZObject::GetMaxHealth()
{
	return max_health;
}

void ZObject::SetHealthPercent(int health_percent, ZMap &tmap)
{
	if(health_percent > 100) health_percent = 100;
	if(health_percent < 0) health_percent = 0;

	//for the map editor
	initial_health_percent = health_percent;

	SetHealth(health_percent * max_health / 100, tmap);
}

void ZObject::SetHealth(int new_health, ZMap &tmap)
{
	bool was_destroyed;

	was_destroyed = IsDestroyed();

	health = new_health;

	if(health < 0) health = 0;
	if(health > max_health) health = max_health;

	if(was_destroyed && !IsDestroyed())
	{
		DoReviveEffect();
		UnSetDestroyMapImpassables(tmap);
	}
	else if(!was_destroyed && IsDestroyed())
	{
		SetDestroyMapImpassables(tmap);
		ProcessKillObject();
	}

	//if(!health) DoDeathEffect();
}

int ZObject::DamageDriverHealth(int damage_amount)
{
	if(!driver_info.size()) return 0;

	driver_info_s &the_driver = *driver_info.begin();

	if(the_driver.driver_health <= 0) return 0;

	the_driver.driver_health -= damage_amount;

	if(the_driver.driver_health <= 0)
	{
		//we killed the driver

		//clear drivers
		ClearDrivers();

		//go to NULL_TEAM
		SetOwner(NULL_TEAM);

		//disengage, clear waypoints, and stop movement
		//done at the server level
	}
	
	return 1;
}

int ZObject::DamageHealth(int damage_amount, ZMap &tmap)
{
	//already dead foo
	if(health <= 0) return health;

	SetHealth(health - damage_amount, tmap);

	//health -= damage_amount;

	//if(health <= 0)
	//{
	//	health = 0;
	//	ProcessKillObject(current_time());
	//}
	//if(health > max_health) health = max_health;

	//got to recalc the build time if this is a building
	RecalcBuildTime();

	return health;
}

void ZObject::SetGroup(int group_num_)
{
	group_num = group_num_;
}

void ZObject::PlaySelectedWav()
{
	//int ch;

	//ch = rand() % 6;

	//ZMix_PlayChannel(-1, selected_wav[ch], 0);
}

void ZObject::PlaySelectedAnim(ZPortrait &portrait)
{
	switch(rand() % 4)
	{
	case 0: portrait.StartAnim(YES_SIR_ANIM); break;
	case 1: portrait.StartAnim(YES_SIR3_ANIM); break;
	case 2: portrait.StartAnim(UNIT_REPORTING1_ANIM); break;
	case 3: portrait.StartAnim(UNIT_REPORTING2_ANIM); break;
	}
}

void ZObject::PlayAcknowledgeWav()
{
	//int ch;

	//ch = rand() % 12;

	//ZMix_PlayChannel(-1, acknowledge_wav[ch], 0);
}

void ZObject::PlayAcknowledgeAnim(ZPortrait &portrait, bool no_way)
{
	if(no_way)
	{
		switch(rand() % 3)
		{
		case 0: portrait.StartAnim(FORGET_IT_ANIM); break;
		case 1: portrait.StartAnim(GET_OUTTA_HERE_ANIM); break;
		case 2: portrait.StartAnim(NO_WAY_ANIM); break;
		}
	}
	else
	{
		switch(rand() % 12)
		{
		case 0: portrait.StartAnim(WERE_ON_OUR_WAY_ANIM); break;
		case 1: portrait.StartAnim(HERE_WE_GO_ANIM); break;
		case 2: portrait.StartAnim(YOUVE_GOT_IT_ANIM); break;
		case 3: portrait.StartAnim(MOVING_IN_ANIM); break;
		case 4: portrait.StartAnim(OKAY_ANIM); break;
		case 5: portrait.StartAnim(ALRIGHT_ANIM); break;
		case 6: portrait.StartAnim(NO_PROBLEM_ANIM); break;
		case 7: portrait.StartAnim(OVER_N_OUT_ANIM); break;
		case 8: portrait.StartAnim(AFFIRMATIVE_ANIM); break;
		case 9: portrait.StartAnim(GOING_IN_ANIM); break;
		case 10: portrait.StartAnim(LETS_DO_IT_ANIM); break;
		case 11: portrait.StartAnim(LETS_GET_EM_ANIM); break;
		}
	}
}

vector<waypoint> &ZObject::GetWayPointList()
{
	return waypoint_list;
}

vector<waypoint> &ZObject::GetWayPointDevList()
{
	return waypoint_dev_list;
}

void ZObject::SetRefID(int id)
{
	ref_id = id;
}

int ZObject::GetRefID()
{
	return ref_id;
}

ZSDL_Surface &ZObject::GetHoverNameImg()
{
	if(hover_name_img.GetBaseSurface()) return hover_name_img;

	//make it?
	if(hover_name.size())
	{
		hover_name_img.Unload();
		hover_name_star_img.Unload();

		hover_name_img.LoadBaseImage(ZFontEngine::GetFont(SMALL_WHITE_FONT).Render(hover_name.c_str()));
		hover_name_star_img.LoadBaseImage(ZFontEngine::GetFont(SMALL_WHITE_FONT).Render((hover_name + " *").c_str()));

		if(hover_name_img.GetBaseSurface())
			hover_name_x_shift = (width_pix - hover_name_img.GetBaseSurface()->w) >> 1;

		if(hover_name_star_img.GetBaseSurface())
			hover_name_star_x_shift = (width_pix - hover_name_star_img.GetBaseSurface()->w) >> 1;
	}

	return hover_name_img;
}

ZSDL_Surface &ZObject::GetHoverNameImgStatic(unsigned char ot, unsigned char oid)
{
	const int max_units_in_type = 7;
	static ZSDL_Surface static_hover_name_img[MAX_MAP_OBJECT_TYPES][max_units_in_type];
	static ZSDL_Surface null_return;

	if(ot < 0) return null_return;
	if(ot >= MAX_MAP_OBJECT_TYPES) return null_return;
	if(oid < 0) return null_return;
	if(oid >= max_units_in_type) return null_return;

	if(!static_hover_name_img[ot][oid].GetBaseSurface())
	{
		string render_str;

		render_str = GetHoverName(ot, oid);
		if(render_str.length())
			static_hover_name_img[ot][oid].LoadBaseImage(ZFontEngine::GetFont(SMALL_WHITE_FONT).Render(render_str.c_str()));
	}

	return static_hover_name_img[ot][oid];
}

string ZObject::GetHoverName(unsigned char ot, unsigned char oid)
{
	switch(ot)
	{
		case CANNON_OBJECT:
			switch(oid)
			{
				case GATLING: return "Gatling"; break;
				case GUN: return "Gun"; break;
				case HOWITZER: return "Howitzer"; break;
				case MISSILE_CANNON: return "Missile"; break;
			}
			break;
		case VEHICLE_OBJECT:
			switch(oid)
			{
				case JEEP: return "Jeep"; break;
				case LIGHT: return "Light"; break;
				case MEDIUM: return "Medium"; break;
				case HEAVY: return "Heavy"; break;
				case APC: return "APC"; break;
				case MISSILE_LAUNCHER: return "M Missile"; break;
				case CRANE: return "Crane"; break;
			}
			break;
		case ROBOT_OBJECT:
			switch(oid)
			{
				case GRUNT: return "Grunt"; break;
				case PSYCHO: return "Psychos"; break;
				case SNIPER: return "Sniper"; break;
				case TOUGH: return "Tough"; break;
				case PYRO: return "Pyros"; break;
				case LASER: return "Laser"; break;
			}
			break;
	}

	return "";
}

void ZObject::RenderHover(ZMap &zmap, SDL_Surface *dest, team_type viewers_team)
{
	int &x = loc.x;
	int &y = loc.y;
	SDL_Rect from_rect, to_rect;
	const int hover_name_y_shift = -19;

	if(hover_name_img.GetBaseSurface())
	{
		if(viewers_team == owner && viewers_team != NULL_TEAM)
		{
			zmap.RenderZSurface(&hover_name_star_img, x + hover_name_star_x_shift, y + hover_name_y_shift);
			//if(zmap.GetBlitInfo(hover_name_star_img, x + hover_name_star_x_shift, y + hover_name_y_shift, from_rect, to_rect))
			//	SDL_BlitSurface( hover_name_star_img, &from_rect, dest, &to_rect);
		}
		else
		{
			zmap.RenderZSurface(&hover_name_star_img, x + hover_name_x_shift, y + hover_name_y_shift);
			//if(zmap.GetBlitInfo(hover_name_img, x + hover_name_x_shift, y + hover_name_y_shift, from_rect, to_rect))
			//	SDL_BlitSurface( hover_name_img, &from_rect, dest, &to_rect);
		}
	}
	else if(hover_name.size())
	{
		hover_name_img.Unload();
		hover_name_star_img.Unload();

		hover_name_img.LoadBaseImage( ZFontEngine::GetFont(SMALL_WHITE_FONT).Render(hover_name.c_str()));
		hover_name_star_img.LoadBaseImage( ZFontEngine::GetFont(SMALL_WHITE_FONT).Render((hover_name + " *").c_str()));

		if(hover_name_img.GetBaseSurface())
			hover_name_x_shift = (width_pix - hover_name_img.GetBaseSurface()->w) >> 1;

		if(hover_name_star_img.GetBaseSurface())
			hover_name_star_x_shift = (width_pix - hover_name_star_img.GetBaseSurface()->w) >> 1;

		if(viewers_team == owner && viewers_team != NULL_TEAM)
		{
			if(hover_name_star_img.GetBaseSurface())
				zmap.RenderZSurface(&hover_name_star_img, x + hover_name_star_x_shift, y + hover_name_y_shift);
			//if(zmap.GetBlitInfo(hover_name_star_img, x + hover_name_star_x_shift, y + hover_name_y_shift, from_rect, to_rect))
			//	SDL_BlitSurface( hover_name_star_img, &from_rect, dest, &to_rect);
		}
		else
		{
			if(hover_name_img.GetBaseSurface())
				zmap.RenderZSurface(&hover_name_img, x + hover_name_x_shift, y + hover_name_y_shift);
			//if(zmap.GetBlitInfo(hover_name_img, x + hover_name_x_shift, y + hover_name_y_shift, from_rect, to_rect))
			//	SDL_BlitSurface( hover_name_img, &from_rect, dest, &to_rect);
		}
	}

	//if you are rendering the hover name, then you are also rendering the health
	RenderHealth(zmap, dest);
}

void ZObject::RenderHealth(ZMap &zmap, SDL_Surface *dest)
{
	int &x = loc.x;
	int &y = loc.y;
	SDL_Rect the_box;
	SDL_Rect from_rect, to_rect;
	//int shift_x, shift_y, view_w, view_h;
	const char g_r = 82, g_g = 190, g_b = 33;
	const char y_r = 247, y_g = 203, y_b = 107;
	const char b_r = 0, b_g = 0, b_b = 0;
	//int green_map = SDL_MapRGB(dest->format, 82, 190, 33);
	//int yellow_map = SDL_MapRGB(dest->format, 247, 203, 107);
	//int black_map = SDL_MapRGB(dest->format, 0, 0, 0);
	const int bar_x_shift = -3;
	const int bar_y_shift = -8;
	const int max_dist = 30 + 6;
	int green_dist, yellow_dist, total_dist;


	if(!hover_name.size()) return;

	//calculate
	green_dist = max_dist * (1.0 * health / MAX_UNIT_HEALTH);
	yellow_dist = max_dist * (1.0 * max_health / MAX_UNIT_HEALTH);

	if(green_dist <= 0) green_dist = 1;
	if(yellow_dist <= 0) yellow_dist = 1;
	total_dist = yellow_dist + 2;

	//zmap.GetViewShiftFull(shift_x, shift_y, view_w, view_h);

	//draw black
	//the_box.x = (x - shift_x) + bar_x_shift;
	//the_box.y = (y - shift_y) + bar_y_shift;
	//the_box.w = total_dist;
	//the_box.h = 4;

	//SDL_FillRect(dest, &the_box, black_map);

	if(zmap.GetBlitInfo(x + bar_x_shift, y + bar_y_shift, total_dist, 4, from_rect, to_rect))
	{
		the_box.x = to_rect.x;
		the_box.y = to_rect.y;
		the_box.w = from_rect.w;
		the_box.h = from_rect.h;

		//SDL_FillRect(dest, &the_box, black_map);
		ZSDL_FillRect(&the_box, b_r, b_g, b_b);
	}

	//draw green
	//the_box.x = (x - shift_x) + bar_x_shift + 1;
	//the_box.y = (y - shift_y) + bar_y_shift + 1;
	//the_box.w = green_dist;
	//the_box.h = 2;

	//SDL_FillRect(dest, &the_box, green_map);

	if(zmap.GetBlitInfo(x + bar_x_shift + 1, y + bar_y_shift + 1, green_dist, 2, from_rect, to_rect))
	{
		the_box.x = to_rect.x;
		the_box.y = to_rect.y;
		the_box.w = from_rect.w;
		the_box.h = from_rect.h;

		//SDL_FillRect(dest, &the_box, green_map);
		ZSDL_FillRect(&the_box, g_r, g_g, g_b);
	}

	//draw yellow
	//the_box.x = (x - shift_x) + bar_x_shift + 1 + green_dist;
	//the_box.y = (y - shift_y) + bar_y_shift + 1;
	//the_box.w = yellow_dist - green_dist;
	//the_box.h = 2;

	//if(the_box.w)
	//	SDL_FillRect(dest, &the_box, yellow_map);

	if(yellow_dist - green_dist)
	if(zmap.GetBlitInfo(x + bar_x_shift + 1 + green_dist, y + bar_y_shift + 1, yellow_dist - green_dist, 2, from_rect, to_rect))
	{
		the_box.x = to_rect.x;
		the_box.y = to_rect.y;
		the_box.w = from_rect.w;
		the_box.h = from_rect.h;

		//SDL_FillRect(dest, &the_box, yellow_map);
		ZSDL_FillRect(&the_box, y_r, y_g, y_b);
	}
}

void ZObject::RenderAttackRadius(ZMap &zmap, SDL_Surface *dest, vector<ZObject*> &avoid_list)
{
	int &x = loc.x;
	int &y = loc.y;
	int cx, cy;
	int mx, my;
	int shift_x, shift_y, view_w, view_h;
	SDL_Rect to_rect;
	//int rgb_map;
	int i;
	const int dots = 10;
	const double PI_shift = (PI / 2) / dots;
	double deg;

	if(!attack_radius) return;

	zmap.GetViewShiftFull(shift_x, shift_y, view_w, view_h);

	//find the center
	cx = (x - shift_x) + (width_pix >> 1);
	cy = (y - shift_y) + (height_pix >> 1);

	//is any of this even on the map?
	if(cx + attack_radius < 0) return;
	if(cy + attack_radius < 0) return;
	if(cx - attack_radius > view_w) return;
	if(cy - attack_radius > view_h) return;

	//init stuff
	//team_type &t = owner;
	//rgb_map = SDL_MapRGB(dest->format, team_color[t].r, team_color[t].g, team_color[t].b); 
	to_rect.w = 3;
	to_rect.h = 3;

	deg = radius_i;
	for(i=0;i<dots && deg <= PI / 2;i++,deg+=PI_shift)
	{
		mx = (attack_radius + 3) * sin(deg);
		my = (attack_radius + 3) * cos(deg);

		to_rect.x = mx + cx;
		to_rect.y = my + cy;
		to_rect.w = 2;
		to_rect.h = 2;
		if((to_rect.x >= 0 && to_rect.x + 2 < view_w) && (to_rect.y >= 0 && to_rect.y + 2 < view_h))
			if(!WithinAttackRadiusOf(avoid_list, to_rect.x + shift_x, to_rect.y + shift_y))
				ZSDL_FillRect(&to_rect, team_color[owner].r, team_color[owner].g, team_color[owner].b);
				//SDL_FillRect(dest, &to_rect, rgb_map);

		to_rect.x = -mx + cx;
		to_rect.y = -my + cy;
		to_rect.w = 2;
		to_rect.h = 2;
		if((to_rect.x >= 0 && to_rect.x + 2 < view_w) && (to_rect.y >= 0 && to_rect.y + 2 < view_h))
			if(!WithinAttackRadiusOf(avoid_list, to_rect.x + shift_x, to_rect.y + shift_y))
				ZSDL_FillRect(&to_rect, team_color[owner].r, team_color[owner].g, team_color[owner].b);
				//SDL_FillRect(dest, &to_rect, rgb_map);

		to_rect.x = -mx + cx;
		to_rect.y = my + cy;
		to_rect.w = 2;
		to_rect.h = 2;
		if((to_rect.x >= 0 && to_rect.x + 2 < view_w) && (to_rect.y >= 0 && to_rect.y + 2 < view_h))
			if(!WithinAttackRadiusOf(avoid_list, to_rect.x + shift_x, to_rect.y + shift_y))
				ZSDL_FillRect(&to_rect, team_color[owner].r, team_color[owner].g, team_color[owner].b);
				//SDL_FillRect(dest, &to_rect, rgb_map);

		to_rect.x = mx + cx;
		to_rect.y = -my + cy;
		to_rect.w = 2;
		to_rect.h = 2;
		if((to_rect.x >= 0 && to_rect.x + 2 < view_w) && (to_rect.y >= 0 && to_rect.y + 2 < view_h))
			if(!WithinAttackRadiusOf(avoid_list, to_rect.x + shift_x, to_rect.y + shift_y))
				ZSDL_FillRect(&to_rect, team_color[owner].r, team_color[owner].g, team_color[owner].b);
				//SDL_FillRect(dest, &to_rect, rgb_map);
	}
}

void ZObject::RenderSelection(ZMap &zmap, SDL_Surface *dest)
{
	int &x = loc.x;
	int &y = loc.y;
	int shift_x, shift_y, view_w, view_h;
	SDL_Rect dim;

	zmap.GetViewShiftFull(shift_x, shift_y, view_w, view_h);

	dim.x = x - shift_x;
	dim.y = y - shift_y;
	dim.w = width_pix;
	dim.h = height_pix;

	draw_selection_box(dest, dim, team_color[owner], view_w, view_h);

	//draw group number
	if(group_num != -1)
	{
		zmap.RenderZSurface(&group_tag[group_num], x-2, y-3);
		//dim.x -= 2;
		//dim.y -= 3;

		//SDL_BlitSurface(group_tag[group_num], nullptr, dest, &dim);
	}

	//render health too
	RenderHealth(zmap, dest);
}

bool ZObject::WithinAutoEnterRadius(int ox, int oy)
{
	int ae_radius = zsettings->auto_grab_vehicle_distance;
	int &x = center_x;
	int &y = center_y;

	return points_within_distance(x, y, ox, oy, ae_radius);
}

bool ZObject::WithinAutoGrabFlagRadius(int ox, int oy)
{
	int agf_radius = zsettings->auto_grab_flag_distance;
	int &x = center_x;
	int &y = center_y;

	return points_within_distance(x, y, ox, oy, agf_radius);
}

bool ZObject::WithinAgroRadius(ZObject *obj)
{
	if(!obj) return false;

	int &x = center_x;
	int &y = center_y;
	int &ox = obj->center_x;
	int &oy = obj->center_y;

	if(!WithinAgroRadius(ox, oy)) return false;

	if(!obj->IsDestroyableImpass() && zmap && zmap->EngageBarrierBetweenCoords(x,y,ox,oy)) return false;

	return true;
}

bool ZObject::WithinAgroRadius(int ox, int oy)
{
	int agro_radius = attack_radius + zsettings->agro_distance;
	int &x = center_x;
	int &y = center_y;

	return points_within_distance(x, y, ox, oy, agro_radius);
}

bool ZObject::WithinAttackRadius(ZObject *obj)
{
	if(!obj) return false;

	int &x = center_x;
	int &y = center_y;
	int &ox = obj->center_x;
	int &oy = obj->center_y;

	if(!WithinAttackRadius(ox, oy)) return false;

	if(!obj->IsDestroyableImpass() && zmap && zmap->EngageBarrierBetweenCoords(x,y,ox,oy)) return false;

	return true;
}

bool ZObject::WithinAttackRadius(int ox, int oy)
{
	int &x = center_x;
	int &y = center_y;

	return points_within_distance(x, y, ox, oy, attack_radius);
}

bool ZObject::WithinAttackRadiusOf(vector<ZObject*> &avoid_list, int ox, int oy)
{
	vector<ZObject*>::iterator i;

	for(i=avoid_list.begin();i!=avoid_list.end();i++)
		if(*i != this && (*i)->WithinAttackRadius(ox, oy))
			return true;

	return false;
}

bool ZObject::Selectable()
{
	//minions are not selectable
	if(leader_obj) return false;

	return selectable;
}

bool ZObject::UnderCursor(int &map_x, int &map_y)
{
	int &x = loc.x;
	int &y = loc.y;
	if(map_x < x) return false;
	if(map_y < y) return false;
	if(map_x > x + width_pix) return false;
	if(map_y > y + height_pix) return false;

	return true;
}

double ZObject::DistanceFromCoords(int x, int y)
{
	int dx, dy;

	dx = loc.x - x;
	dy = loc.y - y;

	return sqrt((double)((dx * dx) + (dy * dy)));
}

double ZObject::DistanceFromObject(ZObject &obj)
{
	int dx, dy;

	dx = loc.x - obj.loc.x;
	dy = loc.y - obj.loc.y;

	return sqrt((double)((dx * dx) + (dy * dy)));
}

bool ZObject::IntersectsObject(ZObject &obj)
{
	int &x = loc.x;
	int &y = loc.y;
	if(obj.loc.x >= x + width_pix) return false;
	if(obj.loc.x + obj.width_pix <= x) return false;
	if(obj.loc.y >= y + height_pix) return false;
	if(obj.loc.y + obj.height_pix <= y) return false;

	return true;
}

bool ZObject::WithinSelection(int &map_left, int &map_right, int &map_top, int &map_bottom)
{
	int &x = loc.x;
	int &y = loc.y;

	if(map_left >= x + width_pix) return false;
	if(map_right <= x) return false;
	if(map_top >= y + height_pix) return false;
	if(map_bottom <= y) return false;

	return true;
}

bool ZObject::CannonNotPlacable(int &map_left, int &map_right, int &map_top, int &map_bottom)
{
	return WithinSelection(map_left, map_right, map_top, map_bottom);
}

string ZObject::GetObjectName()
{
	return object_name;
}

void ZObject::SetDirection(int direction_)
{
	direction = direction_;
}

void ZObject::SetCords(int x_, int y_)
{
	int &x = loc.x;
	int &y = loc.y;
	x = x_;
	y = y_;

	//set centers
	center_x = x + (width_pix >> 1);
	center_y = y + (height_pix >> 1);
}

//void ZObject::GetCords(int &x_, int &y_)
//{
//	x_ = loc.x;
//	y_ = loc.y;
//}

//void ZObject::GetCenterCords(int &x_, int &y_)
//{
//	//int &x = loc.x;
//	//int &y = loc.y;
//	//x_ = x + (width_pix >> 1);
//	//y_ = y + (height_pix >> 1);
//	x_ = center_x;
//	y_ = center_y;
//}

void ZObject::SetOwner(team_type owner_)
{
	owner = owner_;
}

// SDL_Surface *ZObject::GetRender()
// {
// 	return nullptr;
// }

//void ZObject::GetDimensionsPixel(int &w_pix, int &h_pix)
//{
//	w_pix = width_pix;
//	h_pix = height_pix;
//}

//void ZObject::GetDimensions(int &w, int &h)
//{
//	w = width;
//	h = height;
//}

int ZObject::Process()
{
	printf("ZObject::Process:%s\n", object_name.c_str());
	return 0;
}

int ZObject::ProcessObject()
{
	double &the_time = ztime->ztime;
	double time_dif;

	//smooth the walk because of lag
	SmoothMove(the_time);

	//attack radius
	{
		const int dots = 10;
		const double PI_shift = (PI / 2) / dots;

		time_dif = the_time - last_radius_time;
		last_radius_time = the_time;

		radius_i += (PI_shift) * time_dif;

		//pull it back down
		while(radius_i > PI_shift)
			radius_i -= PI_shift;
	}

	if(the_time >= next_waypoint_time)
	{
		waypoint_i++;
		if(waypoint_i >= 4) waypoint_i = 0;

		next_waypoint_time = the_time + 0.1;
	}

	if(show_waypoints)
	{
		waypoint_cursor.Process(the_time);

		if(the_time >= render_death_time)
			show_waypoints = false;
	}

	TryDropTracks();

	return 1;
}

void ZObject::ShowWaypoints()
{
	double &the_time = ztime->ztime;
	vector<waypoint>::iterator i;

	render_death_time = the_time + 3.0;
	show_waypoints = true;

	//set cursor
	if(waypoint_list.size())
	{
		i=waypoint_list.end();
		i--;

		switch(i->mode)
		{
		case MOVE_WP:
		case FORCE_MOVE_WP:
		case ENTER_FORT_WP:
		case DODGE_WP:
			waypoint_cursor.SetCursor(PLACED_C);
			break;
		case PICKUP_GRENADES_WP:
			waypoint_cursor.SetCursor(GRABBED_C);
			break;
		case ENTER_WP:
			waypoint_cursor.SetCursor(ENTERED_C);
			break;
		case ATTACK_WP:
		case AGRO_WP:
			waypoint_cursor.SetCursor(ATTACKED_C);
			break;
		case CRANE_REPAIR_WP:
		case UNIT_REPAIR_WP:
			waypoint_cursor.SetCursor(REPAIRED_C);
			break;
		default:
			waypoint_cursor.SetCursor(PLACED_C);
			break;
		}
	}
	else
		waypoint_cursor.SetCursor(PLACED_C);
}

void ZObject::DoRenderWaypoints(ZMap &the_map, SDL_Surface *dest, vector<ZObject*> &object_list, bool is_rally_points, int shift_x, int shift_y)
{
	int x, y;
	int nx, ny;
	const int glvl = 170;
	SDL_Rect the_box;
	vector<waypoint>::iterator i;
	//Uint32 mappedrgb = SDL_MapRGB(dest->format, glvl, glvl, glvl);
	double tx,ty;
	double txp, typ;
	double dx, dy;
	double dist;
	int ox, oy;
	int maxj;
	int j;
	int mshift_x, mshift_y;
	int view_h, view_w;
	ZObject *target_object;
	vector<waypoint> *render_waypoint_list;

	if(!is_rally_points && !show_waypoints) return;

	if(is_rally_points)
	{
		if(!GetBuildingCreationPoint(x, y))
		{
			x = center_x;
			y = center_y;
		}
		render_waypoint_list = &rallypoint_list;
	}
	else
	{
		x = center_x;
		y = center_y;
		render_waypoint_list = &waypoint_list;
	}

	//do we even have waypoints to render?
	if(!render_waypoint_list->size()) return;

	the_map.GetViewShiftFull(mshift_x, mshift_y, view_w, view_h);

	view_w -= 2;
	view_h -= 2;

	tx = x - mshift_x;
	ty = y - mshift_y;

	//center
	//tx += width_pix >> 1;
	//ty += height_pix >> 1;

	//draw from inside
	if(is_rally_points && GetBuildingCreationMovePoint(nx, ny))
	{
		dx = nx - x;
		dy = ny - y;

		dist = sqrt((dx * dx) + (dy * dy));

		txp = 4 * dx / dist;
		typ = 4 * dy / dist;

		tx += txp * (waypoint_i / 4.0);
		ty += typ * (waypoint_i / 4.0);

		maxj = dist / 4;
		maxj++;

		if(waypoint_i) maxj--;

		//draw this segments points
		for(j=0;j<maxj;j++)
		{
			if((int)tx < view_w && (int)ty < view_h)
			{
				the_box.x = (int)tx;
				the_box.y = (int)ty;
				the_box.w = 2;
				the_box.h = 2;

				//SDL_FillRect(dest, &the_box, mappedrgb);
				ZSDL_FillRect(&the_box, glvl, glvl, glvl);
			}

			tx += txp;
			ty += typ;
		}

		//setup for next round
		tx = (nx - mshift_x);
		ty = (ny - mshift_y);
	}
	
	//for(i=waypoint_list.begin();i!=waypoint_list.end();i++)
	for(i=render_waypoint_list->begin();i!=render_waypoint_list->end();i++)
	{
		switch(i->mode)
		{
		case MOVE_WP:
		case FORCE_MOVE_WP:
		case ENTER_WP:
		case CRANE_REPAIR_WP:
		case UNIT_REPAIR_WP:
		case ENTER_FORT_WP:
		case DODGE_WP:
		case PICKUP_GRENADES_WP:
			//dx = (i->x - mshift_x) - tx;
			//dy = (i->y - mshift_y) - ty;
			nx = (i->x - mshift_x);
			ny = (i->y - mshift_y);
			break;
		
			//dx = (i->x - mshift_x) - tx;
			//dy = (i->y - mshift_y) - ty;
			//break;
		case ATTACK_WP:
		case AGRO_WP:
			target_object = GetObjectFromID(i->ref_id, object_list);

			if(target_object)
			{
				target_object->GetCenterCords(ox, oy);

				//dx = (ox - mshift_x) - tx;
				//dy = (oy - mshift_y) - ty;
				nx = (ox - mshift_x);
				ny = (oy - mshift_y);
			}
			else
			{
				//dx = (i->x - mshift_x) - tx;
				//dy = (i->y - mshift_y) - ty;
				nx = (i->x - mshift_x);
				ny = (i->y - mshift_y);
			}
			
			break;
		}

		//render
		RenderWaypointLine(tx, ty, nx, ny, view_h, view_w);

		//set old to new
		tx = nx;
		ty = ny;

		/*
		//dx = (i->x - mshift_x) - tx;
		//dy = (i->y - mshift_y) - ty;
		dist = sqrt((dx * dx) + (dy * dy));

		txp = 4 * dx / dist;
		typ = 4 * dy / dist;

		tx += txp * (waypoint_i / 4.0);
		ty += typ * (waypoint_i / 4.0);

		maxj = dist / 4;
		maxj++;

		if(waypoint_i) maxj--;

		//draw this segments points
		for(j=0;j<maxj;j++)
		{
			if((int)tx < view_w && (int)ty < view_h)
			{
				the_box.x = (int)tx;
				the_box.y = (int)ty;
				the_box.w = 2;
				the_box.h = 2;

				//SDL_FillRect(dest, &the_box, mappedrgb);
				ZSDL_FillRect(&the_box, glvl, glvl, glvl);
			}

			tx += txp;
			ty += typ;
		}

		//setup for next round
		tx = (i->x - mshift_x);
		ty = (i->y - mshift_y);
		*/
	}

	//render cursor
	//if(waypoint_list.size())
	if(render_waypoint_list->size())
	{
		int kx, ky;
		//i=waypoint_list.end();
		i=render_waypoint_list->end();
		i--;

		switch(i->mode)
		{
		case MOVE_WP:
		case FORCE_MOVE_WP:
		case ENTER_FORT_WP:
		case DODGE_WP:
			kx = (i->x);// - mshift_x;
			ky = (i->y);// - mshift_y;
			waypoint_cursor.SetCursor(PLACED_C);
			break;
		case ENTER_WP:
			kx = (i->x);// - mshift_x;
			ky = (i->y);// - mshift_y;
			waypoint_cursor.SetCursor(ENTERED_C);
			break;
		case PICKUP_GRENADES_WP:
			kx = (i->x);// - mshift_x;
			ky = (i->y);// - mshift_y;
			waypoint_cursor.SetCursor(GRABBED_C);
			break;
		case CRANE_REPAIR_WP:
		case UNIT_REPAIR_WP:
			kx = (i->x);// - mshift_x;
			ky = (i->y);// - mshift_y;
			waypoint_cursor.SetCursor(REPAIRED_C);
			break;
		case ATTACK_WP:
		case AGRO_WP:
			target_object = GetObjectFromID(i->ref_id, object_list);

			if(target_object)
			{
				target_object->GetCenterCords(ox, oy);

				kx = ox;// - mshift_x;
				ky = oy;// - mshift_y;

				waypoint_cursor.SetCursor(ATTACKED_C);
			}
			else
			{
				kx = (i->x);// - mshift_x;
				ky = (i->y);// - mshift_y;
				waypoint_cursor.SetCursor(PLACED_C);
			}
			
			break;
		}

		waypoint_cursor.Render(the_map, dest, kx, ky, true);
	}
}

void ZObject::RenderWaypointLine(int sx, int sy, int ex, int ey, int view_h, int view_w)
{
	const int glvl = 170;
	double tx, ty;
	double txp, typ;
	double dx, dy;
	double dist;
	int maxj;
	SDL_Rect the_box;

	tx = sx;
	ty = sy;

	dx = ex - sx;
	dy = ey - sy;

	dist = sqrt((dx * dx) + (dy * dy));

	txp = 4 * dx / dist;
	typ = 4 * dy / dist;

	tx += txp * (waypoint_i / 4.0);
	ty += typ * (waypoint_i / 4.0);

	maxj = dist / 4;
	maxj++;

	if(waypoint_i) maxj--;

	//draw this segments points
	for(int j=0;j<maxj;j++)
	{
		if((int)tx < view_w && (int)ty < view_h)
		{
			the_box.x = (int)tx;
			the_box.y = (int)ty;
			the_box.w = 2;
			the_box.h = 2;

			//SDL_FillRect(dest, &the_box, mappedrgb);
			ZSDL_FillRect(&the_box, glvl, glvl, glvl);
		}

		tx += txp;
		ty += typ;
	}
}

void ZObject::DoRender(ZMap &the_map, SDL_Surface *dest, int shift_x, int shift_y)
{
	printf("ZObject::DoRender:%s\n", object_name.c_str());
}

void ZObject::DoAfterEffects(ZMap &the_map, SDL_Surface *dest, int shift_x, int shift_y)
{
// 	printf("ZObject::DoAfterEffects:%s\n", object_name.c_str());
}

void ZObject::SetDestroyed(bool is_destroyed)
{
	destroyed = is_destroyed;
}

void ZObject::GetObjectID(unsigned char &object_type_, unsigned char &object_id_)
{
   object_type_ = object_type;
   object_id_ = object_id;
}

//team_type ZObject::GetOwner()
//{
//   return owner;
//}

int ZObject::ProcessServer(ZMap &tmap, ZOLists &ols)
{
	double &the_time = ztime->ztime;
	double time_dif;
	vector<waypoint>::iterator wp;
	bool is_new_waypoint;
	bool attack_player_given = false;

	time_dif = the_time - last_process_server_time;
	last_process_server_time = the_time;

	sflags.clear();

	//it being repaired?
	sflags.auto_repair = DoAutoRepair(tmap, ols);

	//increase / decrease stamina
	ProcessRunStamina(time_dif);

	if(IsDestroyed()) return 0;

	//was a unit created?
	sflags.build_unit = BuildUnit(the_time, sflags.bot, sflags.boid);

	//repair unit
	sflags.repair_unit = RepairUnit(the_time, sflags.rot, sflags.roid, sflags.rdriver_type, sflags.rdriver_info, sflags.rwaypoint_list);

	//lid business
	ProcessServerLid();

	//go through waypoints
	if(waypoint_list.size())
	{
		wp = waypoint_list.begin();

		if(*wp != last_wp)
		{
			is_new_waypoint = true;
			last_wp = *wp;
			xover = yover = 0;

			cur_wp_info.clear_and_kill(tmap);

			//because all waypoints use cur_wp_info.x+y for movement
			//we set it to our current location to set ourselves to
			//an initial "stop"
			//cur_wp_info.x = loc.x + (width_pix >> 1);
			//cur_wp_info.y = loc.y + (height_pix >> 1);
			//cur_wp_info.x = center_x;
			//cur_wp_info.y = center_y;
			SetTarget(center_x, center_y);

			//stop running
			is_running = false;
		}
		else
			is_new_waypoint = false;

		switch(wp->mode)
		{
		case MOVE_WP:
			ProcessMoveWP(wp, time_dif, is_new_waypoint, ols, tmap, true);
			break;
		case FORCE_MOVE_WP:
			ProcessMoveWP(wp, time_dif, is_new_waypoint, ols, tmap, false);
			break;
		case DODGE_WP:
			ProcessDodgeWP(wp, time_dif, is_new_waypoint, ols, tmap);
			break;
		case ENTER_WP:
			ProcessEnterWP(wp, time_dif, is_new_waypoint, ols, tmap);
			break;
		case ATTACK_WP:
			attack_player_given = wp->player_given;
			ProcessAttackWP(wp, time_dif, is_new_waypoint, ols, tmap);
			break;
		case AGRO_WP:
			ProcessAgroWP(wp, time_dif, is_new_waypoint, ols, tmap);
			break;
		case CRANE_REPAIR_WP:
			ProcessCraneRepairWP(wp, time_dif, is_new_waypoint, ols, tmap);
			break;
		case UNIT_REPAIR_WP:
			ProcessUnitRepairWP(wp, time_dif, is_new_waypoint, ols, tmap);
			break;
		case ENTER_FORT_WP:
			ProcessEnterFortWP(wp, time_dif, is_new_waypoint, ols, tmap);
			break;
		case PICKUP_GRENADES_WP:
			ProcessPickupWP(wp, time_dif, is_new_waypoint, ols, tmap);
			break;
		default:
			printf("ZObject::ProcessServer::killing unknown waypoint:%d\n", wp->mode);
			KillWP(wp);
			break;
		}
	}
	else
	{
		//!waypoint_list.size()
		//StopMove();
	}

	//see if they want to attack someone nearby
	CheckPassiveEngage(the_time, ols);

	//let them do their damage
	ProcessAttackDamage(tmap, attack_player_given);

	//update location only happens in intervals
	//update velocity always happens
	if(sflags.updated_location)
	{
		if(the_time >= next_loc_update_time)
			next_loc_update_time = the_time + loc_update_int;
		else
			sflags.updated_location = false;
	}

	return 1;
}

void ZObject::Engage(ZObject *attack_object_)
{
	//are we really disengaging by chance?
	if(!attack_object_)
	{
		Disengage();
		return;
	}

	//attack then
	if(attack_object != attack_object_)
	{
		attack_object = attack_object_;
		sflags.updated_attack_object = true;

		//if we can be sniped by the target, we should open our lid!
		if(attack_object->CanSnipe())
			SignalLidShouldOpen();
	}
}

bool ZObject::Disengage()
{
	if(attack_object)
	{
		attack_object = nullptr;
		sflags.updated_attack_object = true;

		//we only open our lid while we're engaged.
		SignalLidShouldClose();

		return true;
	}

	return false;
}

bool ZObject::IsMoving()
{
	const double z = 0.00001;

	return !((loc.dx > -z && loc.dx < z) && (loc.dy > -z && loc.dy < z));
}

void ZObject::CheckPassiveEngage(double &the_time, ZOLists &ols)
{
	//is it time, since this is a "tiny bit intensive"
	if(the_time < next_check_passive_attack_time) return;

	//set for next time
	next_check_passive_attack_time = the_time + 0.5;

	//can we even attack
	if(!CanAttack()) return;
	if(owner == NULL_TEAM) return;
	if(!(object_type == CANNON_OBJECT || object_type == VEHICLE_OBJECT || object_type == ROBOT_OBJECT)) return;
	if(object_type == ROBOT_OBJECT && IsMoving()) return;

	//are we already attacking someone?
	if(attack_object) 
	{
		int ox, oy;

		//is it still within range?
		attack_object->GetCenterCords(ox, oy);

		//if(!WithinAttackRadius(ox, oy))
		if(!WithinAttackRadius(attack_object))
			Disengage();
		
		return;
	}

	vector<ZObject*>::iterator obj;

	vector<ZObject*> agro_choices;
	vector<ZObject*> enter_vehicle_choices;
	vector<ZObject*> grab_flag_choices;
	vector<ZObject*> grab_grenades_choices;

	//lets see if there are any enemies to attack..
	if(CanAttack())
	for(obj=ols.passive_engagable_olist.begin();obj!=ols.passive_engagable_olist.end();obj++)
	{
		int ox, oy;
		unsigned char ot, oid;

		(*obj)->GetObjectID(ot, oid);
		(*obj)->GetCenterCords(ox, oy);

		if((*obj)->GetOwner() != NULL_TEAM && (*obj)->GetOwner() != owner && CanAttackObject((*obj)))
		{
			//do not auto attack buildings
			if(!(ot == CANNON_OBJECT || ot == VEHICLE_OBJECT || ot == ROBOT_OBJECT)) continue;

			//if(WithinAttackRadius(ox, oy))
			if(WithinAttackRadius(*obj))
			{
				Engage(*obj);
				//attack_object = *obj;
				//sflags.updated_attack_object = true;
				return;
			}

			//start an agro waypoint?
			//if we have no waypoints and are "not a cannon" / "able to move"
			if(!waypoint_list.size() && object_type != CANNON_OBJECT)
			{
				//if(WithinAgroRadius(ox, oy))
				if(WithinAgroRadius(*obj))
					agro_choices.push_back(*obj);
			}
		}

		//add to the auto enter or grab lists?
		if(!waypoint_list.size() && object_type != CANNON_OBJECT)
		{
			if(!IsMinion() && !agro_choices.size())
			{
				//auto enter vehicle (not apc though)?
				if(object_type == ROBOT_OBJECT && (*obj)->CanBeEntered() && WithinAutoEnterRadius(ox, oy) && !(ot == VEHICLE_OBJECT && oid == APC) && !(this->just_left_cannon && ot == CANNON_OBJECT))
					enter_vehicle_choices.push_back(*obj);
				//else if((ot == MAP_ITEM_OBJECT && oid == FLAG_ITEM) && (*obj)->GetOwner() != owner && WithinAutoGrabFlagRadius(ox, oy))
				//	grab_flag_choices.push_back(*obj);
				//else if((ot == MAP_ITEM_OBJECT && oid == GRENADES_ITEM) && CanPickupGrenades() && WithinAutoEnterRadius(ox, oy))
				//	grab_grenades_choices.push_back(*obj);
			}
		}
	}

	if(!IsMinion() && !agro_choices.size() && !waypoint_list.size())
	{
		//collect grabbable flags
		if(CanMove())
		for(obj=ols.flag_olist.begin();obj!=ols.flag_olist.end();obj++)
		{
			int ox, oy;
			unsigned char ot, oid;

			(*obj)->GetObjectID(ot, oid);
			(*obj)->GetCenterCords(ox, oy);

			if(ot != MAP_ITEM_OBJECT) continue;
			if(oid != FLAG_ITEM) continue;
			if((*obj)->GetOwner() == owner) continue;
			if(!WithinAutoGrabFlagRadius(ox, oy)) continue;

			grab_flag_choices.push_back(*obj);
		}

		//collect grenades list
		if(CanPickupGrenades())
		for(obj=ols.grenades_olist.begin();obj!=ols.grenades_olist.end();obj++)
		{
			int ox, oy;
			unsigned char ot, oid;

			(*obj)->GetObjectID(ot, oid);
			(*obj)->GetCenterCords(ox, oy);

			//if((ot == MAP_ITEM_OBJECT && oid == GRENADES_ITEM) && CanPickupGrenades() && WithinAutoEnterRadius(ox, oy))
			if(ot != MAP_ITEM_OBJECT) continue;
			if(oid != GRENADES_ITEM) continue;
			if(!WithinAutoEnterRadius(ox, oy)) continue;

			grab_grenades_choices.push_back(*obj);
		}
	}

	if(agro_choices.size())
	{
		ZObject *agro_choice;
		int ox, oy;
		double least_distance;

		//find choice (closest)
		agro_choice = agro_choices[0];
		least_distance = DistanceFromObject(*agro_choice);
		for(obj=agro_choices.begin()++;obj!=agro_choices.end();obj++)
		{
			double this_distance = DistanceFromObject(**obj);

			if(this_distance < least_distance)
			{
				least_distance = this_distance;
				agro_choice = *obj;
			}
		}
		

		agro_choice->GetCenterCords(ox, oy);

		waypoint new_movepoint;

		new_movepoint.mode = AGRO_WP;
		new_movepoint.ref_id = agro_choice->GetRefID();
		new_movepoint.x = ox;
		new_movepoint.y = oy;

		waypoint_list.push_back(new_movepoint);
	}
	else
	{
		vector<ZObject*> *obj_choices = nullptr;
		int waypoint_mode = MOVE_WP;
		bool do_auto_enter = false;
		bool do_auto_grab_flag = false;
		bool do_auto_grab_grenades = false;

		if(enter_vehicle_choices.size() && grab_flag_choices.size() && grab_grenades_choices.size())
		{
			switch(rand() % 3)
			{
				case 0: do_auto_enter = true; break;
				case 1: do_auto_grab_flag = true; break;
				case 2: do_auto_grab_grenades = true; break;
			}
		}
		else if(enter_vehicle_choices.size() && grab_flag_choices.size())
		{
			if(rand() % 2) do_auto_enter = true;
			else do_auto_grab_flag = true;
		}
		else if(enter_vehicle_choices.size() && grab_grenades_choices.size())
		{
			if(rand() % 2) do_auto_enter = true;
			else do_auto_grab_grenades = true;
		}
		else if(grab_flag_choices.size() && grab_grenades_choices.size())
		{
			if(rand() % 2) do_auto_grab_flag = true;
			else do_auto_grab_grenades = true;
		}
		else if(enter_vehicle_choices.size())
			do_auto_enter = true;
		else if(grab_flag_choices.size())
			do_auto_grab_flag = true;
		else if(grab_grenades_choices.size())
			do_auto_grab_grenades = true;

		if(do_auto_enter)
		{
			obj_choices = &enter_vehicle_choices;
			waypoint_mode = ENTER_WP;
		}
		else if(do_auto_grab_flag)
		{
			obj_choices = &grab_flag_choices;
			waypoint_mode = MOVE_WP;
		}
		else if(do_auto_grab_grenades)
		{
			obj_choices = &grab_grenades_choices;
			waypoint_mode = PICKUP_GRENADES_WP;
		}

		if(obj_choices && obj_choices->size())
		{
			//vector<ZObject*> &obj_choices = enter_vehicle_choices;
			ZObject *obj_choice;
			int ox, oy;
			double least_distance;

			//find choice (closest)
			obj_choice = *obj_choices->begin();
			least_distance = DistanceFromObject(*obj_choice);
			for(obj=obj_choices->begin()++;obj!=obj_choices->end();obj++)
			{
				double this_distance = DistanceFromObject(**obj);

				if(this_distance < least_distance)
				{
					least_distance = this_distance;
					obj_choice = *obj;
				}
			}

			obj_choice->GetCenterCords(ox, oy);

			waypoint new_movepoint;

			new_movepoint.mode = waypoint_mode;
			new_movepoint.ref_id = obj_choice->GetRefID();
			new_movepoint.x = ox;
			new_movepoint.y = oy;
			new_movepoint.attack_to = true;

			waypoint_list.push_back(new_movepoint);

			//doesn't hurt to call this if we don't have minions
			//but needed if we do
			CloneMinionWayPoints();
		}
	}
}

bool ZObject::EstimateMissileTarget(ZObject *target, int &tx, int &ty)
{
	//some checks
	if(missile_speed <= 0) return false;

	//don't need to estimate if
	//the target isn't moving
	if(isz(target->loc.dx) && isz(target->loc.dy)) return false;

	double dx, dy;
	double dx2, dy2;
	int ixo, iyo;
	double xo, yo;
	double x2o, y2o;
	double Cu, Cd;

	dx = target->loc.dx;
	dy = target->loc.dy;
	target->GetCenterCords(ixo, iyo);
	xo = ixo;
	yo = iyo;

	x2o = center_x;
	y2o = center_y;

	Cu = yo - y2o;
	Cd = xo - x2o;

	double a, b, c, d;

	a = (Cu * Cu) + (Cd * Cd);
	b = (2 * Cu * Cd * dy) - (2 * Cu * Cu * dx);
	c = (Cd * Cd * dy * dy) - (2 * Cu * Cd * dx) + (Cu * Cu * dx * dx) - (Cd * Cd * missile_speed * missile_speed);
	d = (b * b) - (4 * a * c);

	if(d <= 0.00001) 
	{
		printf("EstimateMissileTarget::not solvable\n");
		return false;
	}
	if(isz(a))
	{
		printf("EstimateMissileTarget::a is zero?\n");
		return false;
	}

	dx2 = (-1 * b - sqrt(d)) / (2 * a);

	double dy2_guts;

	dy2_guts = (double)(missile_speed * missile_speed) - (dx2 * dx2);

	//printf("dy2_guts:m:%d dx2_1:%lf\n", missile_speed, (-1 * b - sqrt(d)) / (2 * a));
	//printf("dy2_guts:m:%d dx2_2:%lf\n", missile_speed, (-1 * b + sqrt(d)) / (2 * a));

	if(dy2_guts <= 0.00001) 
	{
		//printf("EstimateMissileTarget::trying other dx2\n");

		dx2 = (-1 * b + sqrt(d)) / (2 * a);

		dy2_guts = (missile_speed * missile_speed) - (dx2 * dx2);

		if(dy2_guts <= 0.00001) 
		{
			printf("EstimateMissileTarget::bad dy2_guts value\n");
			return false;
		}
	}

	dy2 = sqrt(dy2_guts);

	double t;
	double dd;

	//dd = dx - dx2;
	if(!isz(dd = dx - dx2))
		t = -1 * Cd / dd;
	else if(!isz(dd = dy - dy2))
		t = -1 * Cu / dd;
	else
	{
		printf("EstimateMissileTarget::bad dd value\n");
		return false;
	}

	//printf("t:%lf\n", t);

	if(t < 0)
	{
		//printf("EstimateMissileTarget::2nd\n");

		dx2 = (-1 * b + sqrt(d)) / (2 * a);

		double dy2_guts;

		dy2_guts = (double)(missile_speed * missile_speed) - (dx2 * dx2);

		if(dy2_guts <= 0.00001) 
		{
			printf("EstimateMissileTarget::bad dy2_guts value 2nd\n");
			return false;
		}

		dy2 = sqrt(dy2_guts);

		//dd = dx - dx2;
		if(!isz(dd = dx - dx2))
			t = -1 * Cd / dd;
		else if(!isz(dd = dy - dy2))
			t = -1 * Cu / dd;
		else
		{
			printf("EstimateMissileTarget::bad dd value\n");
			return false;
		}

		if(t < 0)
		{
			printf("EstimateMissileTarget::bad t value\n");
			return false;
		}
	}

	tx = dx * t + xo;
	ty = dy * t + yo;

	return true;
}

bool ZObject::NearestAttackLoc(int sx, int sy, int &ex, int &ey, int aa_radius, bool is_robot, ZMap &tmap)
{
	int &x = loc.x;
	int &y = loc.y;
	int &cx = center_x;
	int &cy = center_y;

	//line test
	

	//normal checks
	{
		//center ok?
		ex = cx; ey = cy;
		if(tmap.GetPathFinder().ShouldBeAbleToMoveTo(sx, sy, ex, ey, is_robot)) return true;

		//+8 ok?
		ex = x+8; ey = y+8;
		if((ex != cx && ey != cy) && tmap.GetPathFinder().ShouldBeAbleToMoveTo(sx, sy, ex, ey, is_robot)) return true;
	}

	//quick cannon fort checks
	{
		//up 2 tiles ok?
		ex = x+8; ey = (y+8)-32;
		if((ex != cx && ey != cy) && tmap.GetPathFinder().ShouldBeAbleToMoveTo(sx, sy, ex, ey, is_robot)) return true;

		//right 3 tiles ok?
		ex = x+8+48; ey = y+8;
		if((ex != cx && ey != cy) && tmap.GetPathFinder().ShouldBeAbleToMoveTo(sx, sy, ex, ey, is_robot)) return true;

		//left 3 tiles ok?
		ex = (x+8)-48; ey = y+8;
		if((ex != cx && ey != cy) && tmap.GetPathFinder().ShouldBeAbleToMoveTo(sx, sy, ex, ey, is_robot)) return true;
	}

	//full direct line check
	{
		ZPath_Finding_Bresenham ls;
		int lx, ly;

		ls.Init(cx>>4, cy>>4, sx>>4, sy>>4, tmap.GetMapBasics().width, tmap.GetMapBasics().height);

		while(ls.GetNext(lx, ly))
		{
			ex = (lx<<4)+8;
			ey = (ly<<4)+8;

			if(!points_within_distance(cx, cy, ex, ey, aa_radius)) break;

			if(tmap.GetPathFinder().ShouldBeAbleToMoveTo(sx, sy, ex, ey, is_robot)) return true;
		}
	}

	return false;
}

double ZObject::DamagedSpeed()
{
	if(ShowPartiallyDamaged()) return zsettings->partially_damaged_unit_speed;
	if(ShowDamaged()) return zsettings->damaged_unit_speed;

	return 1.0;
}

double ZObject::RunSpeed() 
{
	if(leader_obj)
		return leader_obj->is_running && !ShowDamaged() ? zsettings->run_unit_speed : 1.0; 
	else
		return is_running && !ShowDamaged() ? zsettings->run_unit_speed : 1.0; 
}

bool ZObject::CanReachTargetRunning(int x, int y)
{
	double distance_runnable;

	//an estimate because the real speed can change with terrain
	distance_runnable = move_speed * stamina;

	return points_within_distance(center_x, center_y, x, y, distance_runnable);
}

void ZObject::ProcessRunStamina(double time_dif)
{
	if(is_running)
	{
		stamina -= time_dif;

		if(stamina < 0)
		{
			stamina = 0;
			is_running = false;
		}
	}
	else
	{
		stamina += time_dif * zsettings->run_recharge_rate;

		if(stamina > max_stamina) stamina = max_stamina;
	}
}

void ZObject::AttemptStartRun()
{
	const double min_stamina = 0.3;

	if(is_running) return;

	//one in five we don't run
	if(!(rand() % 5)) return;

	if(stamina < min_stamina) return;

	is_running = true;
}

void ZObject::ProcessAttackDamage(ZMap &tmap, bool attack_player_given)
{
	double &the_time = ztime->ztime;
	if(!attack_object || !damage) return;
	if(the_time < next_damage_time) return;

	next_damage_time = the_time + damage_int_time;

	if(!this->CanAttackObject(attack_object))
	{
		Disengage();
		return;
	}

	bool can_attack_with_grenades;

	can_attack_with_grenades = (GetGrenadeAmount() || (GetGroupLeader() && GetGroupLeader()->GetGrenadeAmount()));

	if(damage_is_missile || can_attack_with_grenades)
	{
		int tx, ty;
		//int w, h;
		damage_missile new_missile;

		if(!EstimateMissileTarget(attack_object, tx, ty))
			attack_object->GetCenterCords(tx, ty);

		if(can_attack_with_grenades)
		{
			tx += (rand() % 48) - 24;
			ty += (rand() % 48) - 24;

			new_missile.x = tx;
			new_missile.y = ty;
			new_missile.damage = zsettings->grenade_damage * MAX_UNIT_HEALTH;
			new_missile.radius = zsettings->grenade_damage_radius;
			new_missile.team = owner;
			new_missile.attacker_ref_id = ref_id;
			new_missile.attack_player_given = attack_player_given;
			new_missile.target_ref_id = attack_object->GetRefID();
			new_missile.CalcExplodeTimeTo(center_x, center_y, zsettings->grenade_missile_speed, the_time);

			//remove a grenade
			if(GetGrenadeAmount())
			{
				SetGrenadeAmount(GetGrenadeAmount()-1);
				sflags.updated_grenade_amount = true;
			}
			else if(GetGroupLeader() && GetGroupLeader()->GetGrenadeAmount())
			{
				GetGroupLeader()->SetGrenadeAmount(GetGroupLeader()->GetGrenadeAmount()-1);
				sflags.updated_leader_grenade_amount = true;
			}

			//set a different next-time
			next_damage_time = the_time + zsettings->grenade_attack_speed;
		}
		else
		{
			tx += (rand() % 32) - 16;
			ty += (rand() % 32) - 16;

			new_missile.x = tx;
			new_missile.y = ty;
			new_missile.damage = damage;
			new_missile.radius = damage_radius;
			new_missile.team = owner;
			new_missile.attacker_ref_id = ref_id;
			new_missile.attack_player_given = attack_player_given;
			new_missile.target_ref_id = attack_object->GetRefID();
			//new_missile.CalcExplodeTimeTo(loc.x + (width_pix >> 1), loc.y + (height_pix >> 1), missile_speed);
			new_missile.CalcExplodeTimeTo(center_x, center_y, missile_speed, the_time);
		}

		damage_missile_list->push_back(new_missile);

		sflags.fired_missile = true;
		sflags.missile_x = tx;
		sflags.missile_y = ty;

		//tell the unit to dodge
		attack_object->DodgeMissile(tx, ty, new_missile.explode_time - the_time);
	}
	else
	{
		if((rand() % 10000) / 10000.0 > damage_chance) return;

		//attack vehicle or driver?
		if(can_snipe && attack_object->CanBeSniped() && ((rand() % 10000) / 10000.0 <= snipe_chance))
		{
			attack_object->DamageDriverHealth(damage);
			sflags.updated_attack_object_driver_health = true;
		}
		else
		{
			attack_object->DamageHealth(damage, tmap);
			sflags.updated_attack_object_health = true;
		}

		//are we a pyro? have to set victom's last hurt by fire time 
		//(so that we can do the melt death effect)
		if(object_type == ROBOT_OBJECT && object_id == PYRO)
			attack_object->SetDamagedByFireTime(the_time);

		//if destroyed give a target destroyed
		if(attack_object->IsDestroyed() && attack_player_given)
		{
			sflags.portrait_anim_ref_id = ref_id;
			sflags.portrait_anim_value = TARGET_DESTROYED_ANIM;
		}
	}
}

void ZObject::ProcessKillObject()
{
	double &the_time = ztime->ztime;

	if(object_type == BUILDING_OBJECT && (object_id != FORT_FRONT && object_id != FORT_BACK))
	{
		do_auto_repair = true;
		//next_auto_repair_time = the_time + BUILDING_AUTO_REPAIR_TIME + (rand() % 61);
		next_auto_repair_time = the_time + zsettings->building_auto_repair_time;

		if(zsettings->building_auto_repair_random_additional_time > 0)
			next_auto_repair_time += (rand() % (zsettings->building_auto_repair_random_additional_time + 1));
	}

	//stop production?
	if(ProducesUnits()) StopBuildingProduction();
}

void ZObject::ProcessAgroWP(vector<waypoint>::iterator &wp, double time_dif, bool is_new, ZOLists &ols, ZMap &tmap)
{
	int &x = loc.x;
	int &y = loc.y;
	float &dx = loc.dx;
	float &dy = loc.dy;
	int ox, oy;
	ZObject *target_object;

	if(is_new)
	{
		//set the agro center
		//cur_wp_info.agro_center_x = x + (width_pix >> 1);
		//cur_wp_info.agro_center_y = y + (height_pix >> 1);
		cur_wp_info.agro_center_x = center_x;
		cur_wp_info.agro_center_y = center_y;
		cur_wp_info.stage = ATTACK_AWS;
	}

	target_object = GetObjectFromID(wp->ref_id, *ols.object_list);

	//we still attacking this target?
	//if(!target_object || 
	//	target_object->IsDestroyed() || 
	//	owner == target_object->GetOwner() ||
	//	(!HasExplosives() && target_object->AttackedOnlyByExplosives()))
	if(!CanAttackObject(target_object))
	{
		KillWP(wp);
		return;
	}

	target_object->GetCenterCords(ox, oy);

	if(!points_within_distance(ox, oy, cur_wp_info.agro_center_x, cur_wp_info.agro_center_y, attack_radius + zsettings->agro_distance))
	{
		//go back to center
		//if we aren't already
		if(cur_wp_info.stage != RETURN_URWS)
		{
			cur_wp_info.stage = RETURN_URWS;
			//cur_wp_info.x = cur_wp_info.agro_center_x;
			//cur_wp_info.y = cur_wp_info.agro_center_y;
			SetTarget(cur_wp_info.agro_center_x, cur_wp_info.agro_center_y);
			SetVelocity();
		}
	}
	else
	{
		//we are within distance
		//so continue the attack if we are not already
		if(cur_wp_info.stage != ATTACK_AWS)
		{
			cur_wp_info.stage = ATTACK_AWS;
			//cur_wp_info.x = ox;
			//cur_wp_info.y = oy;
			SetTarget(ox, oy);
			SetVelocity();
		}
	}

	if(cur_wp_info.stage == ATTACK_AWS)
	{
	//we're there
		//if(WithinAttackRadius(ox, oy))
		if(WithinAttackRadius(target_object))
		{
			StopMove();

			//attack!
			Engage(target_object);
		}
		else
		{
			//hunt down the bastard

			//move there
			//cur_wp_info.x = ox;
			//cur_wp_info.y = oy;
			SetTarget(ox, oy);
			SetVelocity();

			//did we halt movement?
			if(!ProcessMoveOrKillWP(time_dif, tmap, wp, ols)) return;
		}
	}
	else //if(cur_wp_info.stage == RETURN_URWS)
	{
		if(!ProcessMoveOrKillWP(time_dif, tmap, wp, ols)) return;

		if(!ReachedTarget()) return;

		KillWP(wp);
		return;
	}
}

void ZObject::ProcessAttackWP(vector<waypoint>::iterator &wp, double time_dif, bool is_new, ZOLists &ols, ZMap &tmap)
{
	int &x = loc.x;
	int &y = loc.y;
	float &dx = loc.dx;
	float &dy = loc.dy;
	int ox, oy;
	ZObject *target_object;

	target_object = GetObjectFromID(wp->ref_id, *ols.object_list);

	//we still attacking this target?
	//if(!target_object || 
	//	target_object->IsDestroyed() || 
	//	owner == target_object->GetOwner() ||
	//	(!HasExplosives() && target_object->AttackedOnlyByExplosives()))
	if(!CanAttackObject(target_object))
	{
		KillWP(wp);
		return;
	}

	target_object->GetCenterCords(ox, oy);

	//we're there
	//if(WithinAttackRadius(ox, oy))
	if(WithinAttackRadius(target_object))
	{
		StopMove();

		//attack!
		Engage(target_object);
	}
	else
	{
		//hunt down the bastard

		//are we waiting for a path finding response?
		if(cur_wp_info.path_finding_id)
			return;
		else if(!cur_wp_info.got_pf_response)
		{
			//we are not waiting for waypoints
			//so start doing that
			if(!target_object->NearestAttackLoc(x, y, cur_wp_info.x, cur_wp_info.y, attack_radius, (object_type == ROBOT_OBJECT), tmap))
			{
				target_object->GetCords(ox, oy);
				//cur_wp_info.x = ox + 8;
				//cur_wp_info.y = oy + 8;
				SetTarget(ox+8, oy+8);
			}
			else
				SetTarget();

			//needed to check if we should recalc a path
			cur_wp_info.init_attack_x = cur_wp_info.x;
			cur_wp_info.init_attack_y = cur_wp_info.y;

			//cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(x + (width_pix >> 1), y + (height_pix >> 1), cur_wp_info.x, cur_wp_info.y, (object_type == ROBOT_OBJECT), ref_id);
			//cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(center_x, center_y, cur_wp_info.x, cur_wp_info.y, (object_type == ROBOT_OBJECT), ref_id);
			cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(x + 8, y + 8, cur_wp_info.x, cur_wp_info.y, (object_type == ROBOT_OBJECT), HasExplosives(), ref_id);

			//don't wait for thread if it wasn't created
			if(cur_wp_info.path_finding_id)
				StopMove();
			else
			{
				cur_wp_info.got_pf_response = true;
				SetVelocity();
			}

			return;
		}

		//redo path finding?
		{
			int adx, ady;

			adx = ox - cur_wp_info.init_attack_x;
			ady = oy - cur_wp_info.init_attack_y;

			//has it moved too far from our current destination?
			if(abs(adx) > attack_radius || abs(ady) > attack_radius)
			{
				//reset the waypoint
				StopMove();
				cur_wp_info.clear();

				return;
			}
		}

		//move there
		//cur_wp_info.x = ox;
		//cur_wp_info.y = oy;
		SetVelocity();
		//did we halt movement?
		if(!ProcessMoveOrKillWP(time_dif, tmap, wp, ols)) return;

		{
			if(!ReachedTarget()) return;

			//go to the next pf_point, or kill this waypoint?
			if(cur_wp_info.pf_point_list.size())
			{
				//cur_wp_info.x = cur_wp_info.pf_point_list.begin()->x;
				//cur_wp_info.y = cur_wp_info.pf_point_list.begin()->y;
				SetTarget(cur_wp_info.pf_point_list.begin()->x, cur_wp_info.pf_point_list.begin()->y);
				SetVelocity();

				cur_wp_info.pf_point_list.erase(cur_wp_info.pf_point_list.begin());
			}
			else
			{
				//we are out of waypoints

				//reset the waypoint
				StopMove();
				cur_wp_info.clear();

				/*
				//find new route to target
				if(!target_object->NearestAttackLoc(x, y, cur_wp_info.x, cur_wp_info.y, attack_radius, (object_type == ROBOT_OBJECT), tmap))
				{
					target_object->GetCords(ox, oy);
					//cur_wp_info.x = ox + 8;
					//cur_wp_info.y = oy + 8;
					SetTarget(ox+8,oy+8);
				}
				else
					SetTarget();

				cur_wp_info.got_pf_response = false;
				//cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(x + (width_pix >> 1), y + (height_pix >> 1), cur_wp_info.x, cur_wp_info.y, (object_type == ROBOT_OBJECT), ref_id);
				//cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(center_x, center_y, cur_wp_info.x, cur_wp_info.y, (object_type == ROBOT_OBJECT), ref_id);
				cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(x + 8, y + 8, cur_wp_info.x, cur_wp_info.y, (object_type == ROBOT_OBJECT), ref_id);

				//don't wait for thread if it wasn't created
				if(cur_wp_info.path_finding_id)
					StopMove();
				else
				{
					cur_wp_info.got_pf_response = true;
					SetVelocity();
				}
				*/
			}
		}
	}
}

void ZObject::ProcessPickupWP(vector<waypoint>::iterator &wp, double time_dif, bool is_new, ZOLists &ols, ZMap &tmap)
{
	const double z = 0.000001;
	int &x = loc.x;
	int &y = loc.y;
	float &dx = loc.dx;
	float &dy = loc.dy;
	ZObject *target_object;

	target_object = GetObjectFromID(wp->ref_id, *ols.object_list);

	//can pickup grenades?
	if(!CanPickupGrenades())
	{
		KillWP(wp);
		return;
	}

	//grenades still exist?
	if(!target_object)
	{
		KillWP(wp);
		return;
	}

	//are they grenades?
	unsigned char ot, oid;
	target_object->GetObjectID(ot, oid);
	if(ot != MAP_ITEM_OBJECT || oid != GRENADES_ITEM)
	{
		KillWP(wp);
		return;
	}

	//are we there?
	if(target_object->UnderCursor(center_x, center_y))
	{
		KillWP(wp);

		if(!IsMinion()) 
		{
			sflags.do_pickup_grenade_anim = true;
			sflags.updated_grenade_amount = true;
			SetGrenadeAmount(GetGrenadeAmount() + target_object->GetGrenadeAmount());
			target_object->SetGrenadeAmount(0);
			target_object->SetHealth(0, tmap);

			sflags.delete_grenade_box_ref_id = target_object->GetRefID();
		}
		return;
	}

	//disengage if this is a robot, because they can not fire and walk
	//also setvelocity
	if(is_new)
	{
		//force move waypoints are expected to always go
		//straight to their targets
		//cur_wp_info.x = wp->x;
		//cur_wp_info.y = wp->y;
		SetTarget(wp->x, wp->y);

		//cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(x + (width_pix >> 1), y + (height_pix >> 1), wp->x, wp->y, (object_type == ROBOT_OBJECT), ref_id);
		//cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(center_x, center_y, cur_wp_info.x, cur_wp_info.y, (object_type == ROBOT_OBJECT), ref_id);
		cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(x + 8, y + 8, cur_wp_info.x, cur_wp_info.y, (object_type == ROBOT_OBJECT), HasExplosives(), ref_id);

		//don't wait for thread if it wasn't created
		if(cur_wp_info.path_finding_id)
			StopMove();
		else
		{
			cur_wp_info.got_pf_response = true;
			SetVelocity();
		}

		//run to pick things up
		AttemptStartRun(wp->x, wp->y);
	}

	//attack to
	if(CheckAttackTo(wp, ols)) return;

	if(!cur_wp_info.got_pf_response) return;

	//did we halt movement?
	if(!ProcessMoveOrKillWP(time_dif, tmap, wp, ols)) return;

	if(!ReachedTarget()) return;

	//go to the next pf_point, or kill this waypoint?
	if(cur_wp_info.pf_point_list.size())
	{
		//cur_wp_info.x = cur_wp_info.pf_point_list.begin()->x;
		//cur_wp_info.y = cur_wp_info.pf_point_list.begin()->y;
		SetTarget(cur_wp_info.pf_point_list.begin()->x, cur_wp_info.pf_point_list.begin()->y);
		SetVelocity();

		cur_wp_info.pf_point_list.erase(cur_wp_info.pf_point_list.begin());
	}
	else
		KillWP(wp);
}

void ZObject::ProcessEnterWP(vector<waypoint>::iterator &wp, double time_dif, bool is_new, ZOLists &ols, ZMap &tmap)
{
	const double z = 0.000001;
	int &x = loc.x;
	int &y = loc.y;
	float &dx = loc.dx;
	float &dy = loc.dy;
	ZObject *target_object;

	if(is_new)
	{
		//cur_wp_info.x = wp->x;
		//cur_wp_info.y = wp->y;
		SetTarget(wp->x, wp->y);

		//cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(x + (width_pix >> 1), y + (height_pix >> 1), wp->x, wp->y, (object_type == ROBOT_OBJECT), ref_id);
		//cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(center_x, center_y, cur_wp_info.x, cur_wp_info.y, (object_type == ROBOT_OBJECT), ref_id);
		cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(x + 8, y + 8, cur_wp_info.x, cur_wp_info.y, (object_type == ROBOT_OBJECT), HasExplosives(), ref_id);

		//don't wait for thread if it wasn't created
		if(cur_wp_info.path_finding_id)
			StopMove();
		else
		{
			cur_wp_info.got_pf_response = true;
			SetVelocity();
		}

		//if(object_type == ROBOT_OBJECT) Disengage();

		//run to enter vehicles
		AttemptStartRun(wp->x, wp->y);
	}

	target_object = GetObjectFromID(wp->ref_id, *ols.object_list);

	if(!target_object || !target_object->CanBeEntered())//target_object->GetOwner() != NULL_TEAM || target_object->IsDestroyed())
	{
		KillWP(wp);
		return;
	}

	int &cx = center_x;
	int &cy = center_y;

	//cx = x + (width_pix >> 1);
	//cy = y + (height_pix >> 1);

	//are we at the target?
	if(target_object->UnderCursor(cx, cy))
	{
		KillWP(wp);

		if(!IsMinion())
		{
			sflags.entered_target_ref_id = target_object->GetRefID();
		}
		return;
	}

	//attack to
	if(CheckAttackTo(wp, ols)) return;

	//don't actually move without the pf_waypoints
	if(!cur_wp_info.got_pf_response) return;

	//did we halt movement?
	if(!ProcessMoveOrKillWP(time_dif, tmap, wp, ols)) return;

	if(!ReachedTarget()) return;

	//go to the next pf_point, or kill this waypoint?
	if(cur_wp_info.pf_point_list.size())
	{
		//cur_wp_info.x = cur_wp_info.pf_point_list.begin()->x;
		//cur_wp_info.y = cur_wp_info.pf_point_list.begin()->y;
		SetTarget(cur_wp_info.pf_point_list.begin()->x, cur_wp_info.pf_point_list.begin()->y);
		SetVelocity();

		cur_wp_info.pf_point_list.erase(cur_wp_info.pf_point_list.begin());
	}
	else
		KillWP(wp);
}

void ZObject::ProcessDodgeWP(vector<waypoint>::iterator &wp, double time_dif, bool is_new, ZOLists &ols, ZMap &tmap)
{
	const double z = 0.000001;
	int &x = loc.x;
	int &y = loc.y;
	float &dx = loc.dx;
	float &dy = loc.dy;

	if(is_new)
	{
		//cur_wp_info.x = wp->x;
		//cur_wp_info.y = wp->y;
		SetTarget(wp->x, wp->y);

		SetVelocity();

		AttemptStartRun();
	}

	//did we halt movement?
	if(!ProcessMoveOrKillWP(time_dif, tmap, wp, ols)) return;

	if(!ReachedTarget()) return;

	KillWP(wp);
}

void ZObject::ProcessMoveWP(vector<waypoint>::iterator &wp, double time_dif, bool is_new, ZOLists &ols, ZMap &tmap, bool stoppable)
{
	const double z = 0.000001;
	int &x = loc.x;
	int &y = loc.y;
	float &dx = loc.dx;
	float &dy = loc.dy;

	//disengage if this is a robot, because they can not fire and walk
	//also setvelocity
	if(is_new)
	{
		//force move waypoints are expected to always go
		//straight to their targets
		if(stoppable)
		{
			//cur_wp_info.x = wp->x;
			//cur_wp_info.y = wp->y;
			SetTarget(wp->x, wp->y);

			//cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(x + (width_pix >> 1), y + (height_pix >> 1), wp->x, wp->y, (object_type == ROBOT_OBJECT), ref_id);
			//cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(center_x, center_y, cur_wp_info.x, cur_wp_info.y, (object_type == ROBOT_OBJECT), ref_id);
			cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(x + 8, y + 8, cur_wp_info.x, cur_wp_info.y, (object_type == ROBOT_OBJECT), HasExplosives(), ref_id);

			//don't wait for thread if it wasn't created
			if(cur_wp_info.path_finding_id)
				StopMove();
			else
			{
				cur_wp_info.got_pf_response = true;
				SetVelocity();
			}
		}
		else
		{
			//cur_wp_info.x = wp->x;
			//cur_wp_info.y = wp->y;
			SetTarget(wp->x, wp->y);
			SetVelocity();
		}

		//if(object_type == ROBOT_OBJECT) Disengage();

		//run if we are going for a flag
		{
			for(vector<ZObject*>::iterator obj=ols.flag_olist.begin()++;obj!=ols.flag_olist.end();obj++)
				if((*obj)->DistanceFromCoords(wp->x, wp->y) <= 32)
				{
					AttemptStartRun(wp->x, wp->y);
					break;
				}
		}
	}

	//attack to
	if(CheckAttackTo(wp, ols)) return;

	if(!cur_wp_info.got_pf_response && stoppable) return;

	//did we halt movement?
	if(!ProcessMoveOrKillWP(time_dif, tmap, wp, ols, stoppable)) return;

	if(!ReachedTarget()) return;

	//go to the next pf_point, or kill this waypoint?
	if(cur_wp_info.pf_point_list.size())
	{
		//cur_wp_info.x = cur_wp_info.pf_point_list.begin()->x;
		//cur_wp_info.y = cur_wp_info.pf_point_list.begin()->y;
		SetTarget(cur_wp_info.pf_point_list.begin()->x, cur_wp_info.pf_point_list.begin()->y);
		SetVelocity();

		cur_wp_info.pf_point_list.erase(cur_wp_info.pf_point_list.begin());
	}
	else
		KillWP(wp);
}

void ZObject::ProcessEnterFortWP(vector<waypoint>::iterator &wp, double time_dif, bool is_new, ZOLists &ols, ZMap &tmap)
{
	int &x = loc.x;
	int &y = loc.y;
	float &dx = loc.dx;
	float &dy = loc.dy;
	ZObject *target_object;
	bool stoppable;

	target_object = GetObjectFromID(wp->ref_id, ols.building_olist);

	//target still exist?
	//target still need repaired?
	if(!target_object)
	{
		KillWP(wp);
		return;
	}

	if(is_new)
	{
		cur_wp_info.stage = GOTO_ENTRANCE_EFWS;

		//goto entrace
		if(target_object->GetBuildingCreationMovePoint(cur_wp_info.x, cur_wp_info.y))
		{
			//these will be our exit cords for the last stage
			cur_wp_info.fort_exit_x = cur_wp_info.x;
			cur_wp_info.fort_exit_y = cur_wp_info.y;

			SetTarget();

			//cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(center_x, center_y, cur_wp_info.x, cur_wp_info.y, (object_type == ROBOT_OBJECT), ref_id);
			cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(x + 8, y + 8, cur_wp_info.x, cur_wp_info.y, (object_type == ROBOT_OBJECT), HasExplosives(), ref_id);

			//don't wait for thread if it wasn't created
			if(cur_wp_info.path_finding_id)
				StopMove();
			else
			{
				cur_wp_info.got_pf_response = true;
				SetVelocity();
			}
		}
		else
		{
			//didn't get the entrance points?
			KillWP(wp);
			return;
		}
	}

	if(!target_object->CanEnterFort(owner))
	{
		if(cur_wp_info.stage == GOTO_ENTRANCE_EFWS)
		{
			KillWP(wp);
			return;
		}
		else if(cur_wp_info.stage == ENTER_BUILDING_EFWS)
		{
			//if the fort gets destroyed while we are entering it
			//then execute the exit stage of this WP
			//cur_wp_info.x = cur_wp_info.fort_exit_x;
			//cur_wp_info.y = cur_wp_info.fort_exit_y;
			SetTarget(cur_wp_info.fort_exit_x, cur_wp_info.fort_exit_y);

			cur_wp_info.stage = EXIT_BUILDING_EFWS;
			SetVelocity();
		}
	}

	//attack to
	if(CheckAttackTo(wp, ols)) return;

	//don't move if we do not have a response
	if(!cur_wp_info.got_pf_response) return;

	switch(cur_wp_info.stage)
	{
		case GOTO_ENTRANCE_EFWS:
			stoppable = true;
			break;
		case ENTER_BUILDING_EFWS:
		case EXIT_BUILDING_EFWS:
			stoppable = false;
			break;
	}

	if(!ProcessMoveOrKillWP(time_dif, tmap, wp, ols, stoppable)) return;

	if(!ReachedTarget()) return;

	//so we reached our current target... now enter the next stage
	switch(cur_wp_info.stage)
	{
	case GOTO_ENTRANCE_EFWS:
		//go to the next pf_point, or next stage?
		if(cur_wp_info.pf_point_list.size())
		{
			//cur_wp_info.x = cur_wp_info.pf_point_list.begin()->x;
			//cur_wp_info.y = cur_wp_info.pf_point_list.begin()->y;
			SetTarget(cur_wp_info.pf_point_list.begin()->x, cur_wp_info.pf_point_list.begin()->y);
			SetVelocity();

			cur_wp_info.pf_point_list.erase(cur_wp_info.pf_point_list.begin());
		}
		else
		{
			if(!target_object->GetBuildingCreationPoint(cur_wp_info.x, cur_wp_info.y))
				KillWP(wp);
			else
			{
				SetTarget();
				SetVelocity();

				cur_wp_info.stage = ENTER_BUILDING_EFWS;
			}
		}
		break;
	case ENTER_BUILDING_EFWS:
		//cur_wp_info.x = cur_wp_info.fort_exit_x;
		//cur_wp_info.y = cur_wp_info.fort_exit_y;
		SetTarget(cur_wp_info.fort_exit_x, cur_wp_info.fort_exit_y);

		cur_wp_info.stage = EXIT_BUILDING_EFWS;

		//destroy the fort
		sflags.destroy_fort_building_ref_id = target_object->GetRefID();

		SetVelocity();
		break;
	case EXIT_BUILDING_EFWS:
		KillWP(wp);
		break;
	default:
		KillWP(wp);
		break;
	}
}

void ZObject::ProcessCraneRepairWP(vector<waypoint>::iterator &wp, double time_dif, bool is_new, ZOLists &ols, ZMap &tmap)
{
	const double z = 0.000001;
	int &x = loc.x;
	int &y = loc.y;
	float &dx = loc.dx;
	float &dy = loc.dy;
	ZObject *target_object;
	bool stoppable;

	target_object = GetObjectFromID(wp->ref_id, ols.building_olist);

	//target still exist?
	//target still need repaired?
	if(!target_object)
	{
		KillWP(wp);
		return;
	}

	//begin movement towards the entrance
	if(is_new)
	{
		int ent_x, ent_y, ent_x2, ent_y2;

		cur_wp_info.stage = GOTO_ENTRANCE_CRWS;

		if(target_object->GetCraneEntrance(ent_x, ent_y, ent_x2, ent_y2))
		{
			if(ent_x == ent_x2 && ent_y == ent_y2)
			{
				//building has one entrance
				//cur_wp_info.x = ent_x;
				//cur_wp_info.y = ent_y;
				SetTarget(ent_x, ent_y);
			}
			else
			{
				//bridge has two entrances, which is closer?
				double d1, d2;

				d1 = sqrt(pow((double)(x - ent_x), 2) + pow((double)(y - ent_y), 2));
				d2 = sqrt(pow((double)(x - ent_x2), 2) + pow((double)(y - ent_y2), 2));

				if(d1 < d2)
				{
					//cur_wp_info.x = ent_x;
					//cur_wp_info.y = ent_y;
					SetTarget(ent_x, ent_y);
				}
				else
				{
					//cur_wp_info.x = ent_x2;
					//cur_wp_info.y = ent_y2;
					SetTarget(ent_x2, ent_y2);
				}
			}
		}
		else //could not get building entrance info?
		{
			KillWP(wp);
			return;
		}

		//these will be our exit cords for the last stage
		cur_wp_info.crane_exit_x = cur_wp_info.x;
		cur_wp_info.crane_exit_y = cur_wp_info.y;

		//SetVelocity();

		//find our way to the entrance
		//cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(x + (width_pix >> 1), y + (height_pix >> 1), cur_wp_info.x, cur_wp_info.y, (object_type == ROBOT_OBJECT), ref_id);
		//cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(center_x, center_y, cur_wp_info.x, cur_wp_info.y, (object_type == ROBOT_OBJECT), ref_id);
		cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(x + 8, y + 8, cur_wp_info.x, cur_wp_info.y, (object_type == ROBOT_OBJECT), HasExplosives(), ref_id);

		//don't wait for thread if it wasn't created
		if(cur_wp_info.path_finding_id)
			StopMove();
		else
		{
			cur_wp_info.got_pf_response = true;
			SetVelocity();
		}
	}

	if(!target_object->CanBeRepairedByCrane(owner))
	{
		if(cur_wp_info.stage == GOTO_ENTRANCE_CRWS)
		{
			KillWP(wp);
			return;
		}
		else if(cur_wp_info.stage == ENTER_BUILDING_CRWS)
		{
			//if the building gets repaired while we are entering it
			//then execute the exit stage of this WP
			//cur_wp_info.x = cur_wp_info.crane_exit_x;
			//cur_wp_info.y = cur_wp_info.crane_exit_y;
			SetTarget(cur_wp_info.crane_exit_x, cur_wp_info.crane_exit_y);

			cur_wp_info.stage = EXIT_BUILDING_CRWS;
			SetVelocity();
		}
	}

	//don't move if we do not have a response
	if(!cur_wp_info.got_pf_response) return;

	switch(cur_wp_info.stage)
	{
		case GOTO_ENTRANCE_CRWS:
			stoppable = true;
			break;
		case ENTER_BUILDING_CRWS:
		case EXIT_BUILDING_CRWS:
			stoppable = false;
			break;
	}

	if(!ProcessMoveOrKillWP(time_dif, tmap, wp, ols, stoppable)) return;

	if(!ReachedTarget()) return;

	//so we reached our current target... now enter the next stage
	switch(cur_wp_info.stage)
	{
	case GOTO_ENTRANCE_CRWS:
		//go to the next pf_point, or next stage?
		if(cur_wp_info.pf_point_list.size())
		{
			//cur_wp_info.x = cur_wp_info.pf_point_list.begin()->x;
			//cur_wp_info.y = cur_wp_info.pf_point_list.begin()->y;
			SetTarget(cur_wp_info.pf_point_list.begin()->x, cur_wp_info.pf_point_list.begin()->y);
			SetVelocity();

			cur_wp_info.pf_point_list.erase(cur_wp_info.pf_point_list.begin());
		}
		else
		{
			if(!target_object->GetCraneCenter(cur_wp_info.x, cur_wp_info.y))
				KillWP(wp);
			else
			{
				cur_wp_info.stage = ENTER_BUILDING_CRWS;

				SetTarget();
				SetVelocity();

				sflags.set_crane_anim = true;
				sflags.crane_anim_on = true;
				sflags.crane_rep_ref_id = wp->ref_id;
			}
		}
		break;
	case ENTER_BUILDING_CRWS:
		//cur_wp_info.x = cur_wp_info.crane_exit_x;
		//cur_wp_info.y = cur_wp_info.crane_exit_y;
		SetTarget(cur_wp_info.crane_exit_x, cur_wp_info.crane_exit_y);
		SetVelocity();

		cur_wp_info.stage = EXIT_BUILDING_CRWS;
		break;
	case EXIT_BUILDING_CRWS:
		//set the building to auto repair immediately...
		target_object->do_auto_repair = true;
		target_object->next_auto_repair_time = 0;

		sflags.set_crane_anim = true;
		sflags.crane_anim_on = false;
		sflags.crane_rep_ref_id = wp->ref_id;
		KillWP(wp);
		break;
	default:
		KillWP(wp);
		break;
	}
}

void ZObject::ProcessUnitRepairWP(vector<waypoint>::iterator &wp, double time_dif, bool is_new, ZOLists &ols, ZMap &tmap)
{
	const double z = 0.000001;
	int &x = loc.x;
	int &y = loc.y;
	float &dx = loc.dx;
	float &dy = loc.dy;
	ZObject *target_object;
	bool stoppable;
	int ent_x, ent_y;

	target_object = GetObjectFromID(wp->ref_id, ols.building_olist);

	//target still exist?
	if(!target_object)
	{
		KillWP(wp);
		return;
	}

	//begin movement towards the entrance
	if(is_new)
	{
		cur_wp_info.stage = GOTO_ENTRANCE_URWS;

		if(target_object->GetRepairEntrance(ent_x, ent_y))
		{
			cur_wp_info.x = ent_x;
			cur_wp_info.y = ent_y;
			SetTarget(ent_x, ent_y);
		}
		else
		{
			KillWP(wp);
			return;
		}

		//SetVelocity();

		//find our way to the entrance
		//cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(x + (width_pix >> 1), y + (height_pix >> 1), cur_wp_info.x, cur_wp_info.y, (object_type == ROBOT_OBJECT), ref_id);
		//cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(center_x, center_y, cur_wp_info.x, cur_wp_info.y, (object_type == ROBOT_OBJECT), ref_id);
		cur_wp_info.path_finding_id = tmap.GetPathFinder().Find_Path(x + 8, y + 8, cur_wp_info.x, cur_wp_info.y, (object_type == ROBOT_OBJECT), HasExplosives(), ref_id);

		//don't wait for thread if it wasn't created
		if(cur_wp_info.path_finding_id)
			StopMove();
		else
		{
			cur_wp_info.got_pf_response = true;
			SetVelocity();
		}
	}

	//still ok to do this wp?
	if(!target_object->CanRepairUnit(owner) || !CanBeRepaired())
	{
		//if the building can not repair us anymore (got destroyed or changed teams?)
		//then if we are entering this building we must now leave it
		if(cur_wp_info.stage == ENTER_BUILDING_URWS)
		{
			if(target_object->GetRepairEntrance(ent_x, ent_y))
			{
				cur_wp_info.x = ent_x;
				cur_wp_info.y = ent_y;
				SetTarget(ent_x, ent_y);
			}
			else
			{
				KillWP(wp);
				return;
			}
			cur_wp_info.stage = EXIT_BUILDING_URWS;
			SetVelocity();
		}
		else
		{
			KillWP(wp);
			return;
		}
	}

	//attack to
	if(CheckAttackTo(wp, ols)) return;

	//don't move if we do not have a response
	if(!cur_wp_info.got_pf_response) return;

	//do we need to wait?
	if(target_object->RepairingAUnit())
	{
		//if the building can repair us but it is repairing someone else,
		//then we will wait if we are in the wait state
		//and we will leave the building if we are currently entering it
		switch(cur_wp_info.stage)
		{
			case ENTER_BUILDING_URWS:
				if(target_object->GetRepairEntrance(ent_x, ent_y))
				{
					//cur_wp_info.x = ent_x;
					//cur_wp_info.y = ent_y;
					SetTarget(ent_x, ent_y);
				}
				else
				{
					KillWP(wp);
					return;
				}
				cur_wp_info.stage = EXIT_BUILDING_URWS;
				SetVelocity();
				break;
			case WAIT_URWS:
				return;
				break;
		}
	}

	switch(cur_wp_info.stage)
	{
		case GOTO_ENTRANCE_URWS:
			stoppable = true;
			break;
		case ENTER_BUILDING_URWS:
		case EXIT_BUILDING_URWS:
			stoppable = false;
			break;
		default:
			stoppable = true;
			break;
	}

	if(!ProcessMoveOrKillWP(time_dif, tmap, wp, ols, stoppable)) return;

	if(!ReachedTarget()) return;

	switch(cur_wp_info.stage)
	{
	case GOTO_ENTRANCE_URWS:
		//go to the next pf_point, or next stage?
		if(cur_wp_info.pf_point_list.size())
		{
			//cur_wp_info.x = cur_wp_info.pf_point_list.begin()->x;
			//cur_wp_info.y = cur_wp_info.pf_point_list.begin()->y;
			SetTarget(cur_wp_info.pf_point_list.begin()->x, cur_wp_info.pf_point_list.begin()->y);
			SetVelocity();

			cur_wp_info.pf_point_list.erase(cur_wp_info.pf_point_list.begin());
		}
		else
		{
			cur_wp_info.stage = WAIT_URWS;
			SetVelocity();
		}
		break;
	case EXIT_BUILDING_URWS:
		cur_wp_info.stage = WAIT_URWS;
		SetVelocity();
		break;
	case ENTER_BUILDING_URWS:
		sflags.entered_repair_building_ref_id = target_object->GetRefID();
		
		//set this to leave the building incase it isn't accepted
		//because of another unit entering at the exact same interval / "time"
		if(target_object->GetRepairEntrance(ent_x, ent_y))
		{
			//cur_wp_info.x = ent_x;
			//cur_wp_info.y = ent_y;
			SetTarget(ent_x, ent_y);
		}
		else
		{
			KillWP(wp);
			return;
		}
		cur_wp_info.stage = EXIT_BUILDING_URWS;
		SetVelocity();
		break;
	case WAIT_URWS:
		if(!target_object->GetRepairCenter(cur_wp_info.x, cur_wp_info.y))
			KillWP(wp);

		cur_wp_info.stage = ENTER_BUILDING_URWS;

		SetTarget();
		SetVelocity();
		break;
	default:
		KillWP(wp);
		break;
	}
}

bool ZObject::CheckAttackTo(vector<waypoint>::iterator &wp, ZOLists &ols)
{
	if(wp->attack_to)
	{
		if(!CanOverwriteWP()) return false;

		//look for attack choices
		vector<ZObject*> agro_choices;
		for(vector<ZObject*>::iterator obj=ols.passive_engagable_olist.begin();obj!=ols.passive_engagable_olist.end();obj++)
		{
			int ox, oy;
			unsigned char ot, oid;

			(*obj)->GetObjectID(ot, oid);
			(*obj)->GetCenterCords(ox, oy);

			if((*obj)->GetOwner() != NULL_TEAM && (*obj)->GetOwner() != owner)
			{
				//do not auto attack buildings
				if(!(ot == CANNON_OBJECT || ot == VEHICLE_OBJECT || ot == ROBOT_OBJECT)) continue;

				//push it?
				//if(WithinAgroRadius(ox, oy)) agro_choices.push_back(*obj);
				if(WithinAgroRadius(*obj)) agro_choices.push_back(*obj);
			}
		}

		//choose a random agro choice if we have one
		if(agro_choices.size())
		{
			ZObject *agro_choice;
			int ox, oy;

			//find choice (closest)
			agro_choice = agro_choices[rand() % agro_choices.size()];

			agro_choice->GetCenterCords(ox, oy);

			waypoint new_waypoint;

			new_waypoint.mode = ATTACK_WP;
			new_waypoint.ref_id = agro_choice->GetRefID();
			new_waypoint.x = ox;
			new_waypoint.y = oy;

			//push this attack waypoint to the front,
			//and exit this current waypoint
			waypoint_list.insert(waypoint_list.begin(), new_waypoint);

			return true;
		}
	}

	return false;
}

void ZObject::KillWP(vector<waypoint>::iterator &wp)
{
	sflags.updated_waypoints = true;
	waypoint_list.erase(wp);

	StopMove();
	//SetVelocity();
	//sflags.updated_location = true;

	//clean this
	last_wp.clear();
}

bool ZObject::CanOverwriteWP()
{
	vector<waypoint>::iterator wp;

	//check current wp
	if(waypoint_list.size())
	{
		wp = waypoint_list.begin();

		switch(wp->mode)
		{
		case FORCE_MOVE_WP:
			return false;
			break;
		case CRANE_REPAIR_WP:
			if(cur_wp_info.stage != GOTO_ENTRANCE_CRWS)
				return false;
			break;
		case UNIT_REPAIR_WP:
			if(cur_wp_info.stage != GOTO_ENTRANCE_URWS && cur_wp_info.stage != WAIT_URWS)
				return false;
			break;
		case ENTER_FORT_WP:
			if(cur_wp_info.stage != GOTO_ENTRANCE_EFWS)
				return false;
			break;
		}
	}

	return true;
}

bool ZObject::StopMove()
{
	float &dx = loc.dx;
	float &dy = loc.dy;
	//const float z = 0.00001;

	//if((dx < z && dx > -z) && (dy < z && dy > -z)) return false;
	if(!IsMoving()) return false;

	dx = 0;
	dy = 0;

	sflags.updated_velocity = true;

	return true;
}

void ZObject::SetTarget(int x, int y)
{
	//to
	cur_wp_info.x = x;
	cur_wp_info.y = y;

	//from
	cur_wp_info.sx = center_x;
	cur_wp_info.sy = center_y;

	//distance to go
	cur_wp_info.adx = abs(cur_wp_info.x - cur_wp_info.sx);
	cur_wp_info.ady = abs(cur_wp_info.y - cur_wp_info.sy);
}

bool ZObject::ReachedTarget()
{
	int &x = loc.x;
	int &y = loc.y;
	int &cx = center_x;
	int &cy = center_y;
	float &dx = loc.dx;
	float &dy = loc.dy;
	
	//we at the target?
	if(cx == cur_wp_info.x && cy == cur_wp_info.y) 
	{
		xover = yover = 0;
		return true;
	}


	//test
	//int adx, ady;
	//adx = abs(cx - cur_wp_info.sx);
	//ady = abs(cy - cur_wp_info.sy);
	//printf("adx:%d vs wp.adx:%d ... ady:%d wp.ady:%d\n", adx, cur_wp_info.adx, ady, cur_wp_info.ady);

	//we over pass the target?
	if(abs(cx - cur_wp_info.sx) >= cur_wp_info.adx && abs(cy - cur_wp_info.sy) >= cur_wp_info.ady)
	{
		x = cur_wp_info.x - (width_pix >> 1);
		y = cur_wp_info.y - (height_pix >> 1);
		xover = yover = 0;
		return true;
	}

	return false;

	////change dx/dy if we arrive at the end of this waypoint
	//if(dx > 0 && cx < cur_wp_info.x) return false;
	//if(dx < 0 && cx > cur_wp_info.x) return false;
	//if(dy > 0 && cy < cur_wp_info.y) return false;
	//if(dy < 0 && cy > cur_wp_info.y) return false;

	////only fix at the last position if we were actually moving
	////if(!((dx < z && dx > -z) && (dy < z && dy > -z)))
	//if(!isz(dx) || !isz(dy))
	//{
	//	x = cur_wp_info.x - (width_pix >> 1);
	//	y = cur_wp_info.y - (height_pix >> 1);
	//	xover = yover = 0;
	//}

	//return true;
}

bool ZObject::ProcessMoveOrKillWP(double time_dif, ZMap &tmap, vector<waypoint>::iterator &wp, ZOLists &ols, bool stoppable)
{
	int stop_x, stop_y;
	if(!ProcessMove(time_dif, tmap, stop_x, stop_y, stoppable))
	{
		//attack our way through?
		if(DoAttackImpassableAtCoords(ols, stop_x, stop_y)) return false;

		//otherwise kill the move and leave
		KillWP(wp);
		return false;
	}

	return true;
}

bool ZObject::ProcessMove(double time_dif, ZMap &tmap, int &stop_x, int &stop_y, bool stoppable)
{
	int &x = loc.x;
	int &y = loc.y;
	float &dx = loc.dx;
	float &dy = loc.dy;
	double nx, ny;
	int inx, iny;
	//const double z = 0.00001;

	//are we moving?
	//if((dx < z && dx > -z) && (dy < z && dy > -z))
	if(isz(dx) && isz(dy)) return true;

	nx = x + (dx * time_dif) + xover;
	ny = y + (dy * time_dif) + yover;

	inx = (int)floor(nx);
	iny = (int)floor(ny);

	//minions never stoppable (for now)
	if(IsMinion()) stoppable = false;

	//is the new x and y kosher?
	//if(stoppable && tmap.WithinImpassable((inx + (width_pix>>1)) - 7, (iny + (height_pix>>1)) - 7, 6, 6, object_type == ROBOT_OBJECT))
	if(stoppable && tmap.WithinImpassable(inx+1, iny+1, width_pix-2, height_pix-2, stop_x, stop_y, object_type == ROBOT_OBJECT))
	{
		if(ReachedTarget())
			return true;
		else
			return false;
	}

	//robots don't attack while moving
	if(object_type == ROBOT_OBJECT) Disengage();

	//capture the overflow loss from double to int conversion
	xover = nx - inx;
	yover = ny - iny;

	x = inx;
	y = iny;

	//set centers
	center_x = x + (width_pix >> 1);
	center_y = y + (height_pix >> 1);

	//set new real move speed
	//if(dx > z || dx < -z || dy > z || dy < -z)
	if(!isz(dx) || !isz(dy))
	{
		double previous_real_speed = real_move_speed;

		real_move_speed = move_speed * tmap.GetTileWalkSpeed(center_x, center_y) * DamagedSpeed() * RunSpeed();
		//real_move_speed = move_speed * tmap.GetTileWalkSpeed(x + (width_pix >> 1), y + (height_pix >> 1));
		//real_move_speed = move_speed * tmap.GetTileWalkSpeed(x + 8, y + 8);

		//update in speed?
		if(real_move_speed != previous_real_speed)
		{
			if(previous_real_speed > 0)
			{
				dx /= previous_real_speed;
				dy /= previous_real_speed;
			}
			else
			{
				printf("error:previous_real_speed(%lf) not above zero!\n", previous_real_speed);
			}
			dx *= real_move_speed;
			dy *= real_move_speed;

			sflags.updated_velocity = true;
		}
	}

	return true;
}

void ZObject::SetVelocity(ZObject *target_object)
{
	int &x = loc.x;
	int &y = loc.y;
	int ox, oy;
	float &dx = loc.dx;
	float &dy = loc.dy;
	vector<waypoint>::iterator wp;
	float mag;
	float old_dx = loc.dx;
	float old_dy = loc.dy;
	

	//do we have a waypoint?
	if(waypoint_list.size())
	{
		int &cx = center_x;
		int &cy = center_y;

		//cx = x + (width_pix >> 1);
		//cy = y + (height_pix >> 1);

		dx = cur_wp_info.x - cx;
		dy = cur_wp_info.y - cy;
		if(!isz(dx) || !isz(dy))
		{
			mag = sqrt((dx * dx) + (dy * dy));
			dx /= mag;
			dy /= mag;
			dx *= real_move_speed;
			dy *= real_move_speed;
		}

		//wp = waypoint_list.begin();

		//switch(wp->mode)
		//{
		//case MOVE_WP:
		//case FORCE_MOVE_WP:
		//case ENTER_WP:
		//case CRANE_REPAIR_WP:
		//case UNIT_REPAIR_WP:
		//case ATTACK_WP:
		//	dx = cur_wp_info.x - cx;
		//	dy = cur_wp_info.y - cy;
		//	if(!isz(dx) || !isz(dy))
		//	{
		//		mag = sqrt((dx * dx) + (dy * dy));
		//		dx /= mag;
		//		dy /= mag;
		//		dx *= real_move_speed;
		//		dy *= real_move_speed;
		//	}
		//	break;
		////case ATTACK_WP:
		////	if(target_object)
		////	{
		////		target_object->GetCenterCords(ox, oy);
		////		dx = ox - cx;
		////		dy = oy - cy;
		////		if(!isz(dx) || !isz(dy))
		////		{
		////			mag = sqrt((dx * dx) + (dy * dy));
		////			dx /= mag;
		////			dy /= mag;
		////			dx *= real_move_speed;
		////			dy *= real_move_speed;
		////		}
		////		//garr
		////		//yover = xover = 0;
		////	}

		//	break;
		//}
	}
	else
	{
		//we aint moving if we have no waypoints
		StopMove();
	}

	if(fabs(dx - old_dx) < 0.1) dx = old_dx;
	if(fabs(dy - old_dy) < 0.1) dy = old_dy;

	//update velocity?
	if(dx != old_dx || dy != old_dy)
	{
		sflags.updated_velocity = true;
		//yover = xover = 0;
	}
}

bool ZObject::DodgeMissile(int tx, int ty, double time_till_explode)
{
	int nx, ny;

	//1 in 3 then don't do
	//if(!(rand() % 3)) return false;
	if(!CanOverwriteWP()) return false;
	if(move_speed <= 0) return false;
	if(real_move_speed <= 0) return false;
	if(object_type == ROBOT_OBJECT && attack_object) return false;
	if(owner == NULL_TEAM) return false;

	double dist; 
	if(time_till_explode > stamina) dist = time_till_explode * real_move_speed;
	else dist = time_till_explode * real_move_speed * zsettings->run_unit_speed;
	int m_move_dist = dist * 2.0 / 4.0;
	int f_move_dist = dist * 4.0 / 4.0;
	if(m_move_dist <= 0) m_move_dist = 1;
	int move_dist = f_move_dist + (rand() % m_move_dist);
	double theta = (2 * PI) * (1000.0 / (rand() % 1000));

	//move_dist = dist;

	nx = center_x + (move_dist * cos(theta));
	ny = center_y + (move_dist * sin(theta));

	//if(rand()%2)
	//	nx = center_x + (f_move_dist + (rand() % m_move_dist));
	//else
	//	nx = center_x - (f_move_dist + (rand() % m_move_dist));

	//if(rand()%2)
	//	ny = center_y + (f_move_dist + (rand() % m_move_dist));
	//else
	//	ny = center_y - (f_move_dist + (rand() % m_move_dist));

	//modify current dodge or make new?
	if(waypoint_list.size() && waypoint_list.begin()->mode == DODGE_WP)
	{
		//altering this should make it
		//recognize as a new waypoint later
		waypoint_list.begin()->x = nx;
		waypoint_list.begin()->y = ny;
	}
	else
	{
		waypoint dodge_waypoint;

		dodge_waypoint.mode = DODGE_WP;
		dodge_waypoint.ref_id = -1;
		dodge_waypoint.x = nx;
		dodge_waypoint.y = ny;

		waypoint_list.insert(waypoint_list.begin(), dodge_waypoint);
	}

	return true;
}

bool ZObject::DoAttackImpassableAtCoords(ZOLists &ols, int x, int y)
{
	//all impasses require explosives to destroy
	if(!HasExplosives()) return false;

	for(vector<ZObject*>::iterator o=ols.object_list->begin(); o!=ols.object_list->end();++o)
	{
		if(!(*o)->IsDestroyableImpass()) continue;
		if(!(*o)->CausesImpassAtCoord(x, y)) continue;
		if(!CanAttackObject(*o)) continue;

		//ok attack it
		{
			waypoint new_waypoint;

			new_waypoint.mode = ATTACK_WP;
			new_waypoint.ref_id = (*o)->GetRefID();
			(*o)->GetCenterCords(new_waypoint.x, new_waypoint.y);

			//push this attack waypoint to the front,
			//and exit this current waypoint
			waypoint_list.insert(waypoint_list.begin(), new_waypoint);

			//a kind of solution for the minions
			CloneMinionWayPoints();

			return true;
		}
	}

	return false;
}

server_flag &ZObject::GetSFlags()
{
	return sflags;
}

void ZObject::CreateLocationData(char *&data, int &size)
{
	size = 4 + sizeof(object_location);
	data = (char*)malloc(size);

	((int*)data)[0] = ref_id;
	memcpy(data+4, &loc, sizeof(object_location));
}

void ZObject::CreateAttackObjectData(char *&data, int &size)
{
	attack_object_packet send_data;

	size = sizeof(attack_object_packet);
	data = (char*)malloc(size);

	send_data.ref_id = ref_id;
	if(attack_object)
		send_data.attack_object_ref_id = attack_object->ref_id;
	else
		send_data.attack_object_ref_id = -1;

	memcpy(data, &send_data, sizeof(attack_object_packet));
}

void ZObject::SetLoc(object_location new_loc)
{
	double &the_time = ztime->ztime;
	int &x = loc.x;
	int &y = loc.y;

	if(new_loc.dx != loc.dx || new_loc.dy != loc.dy)
	{
		loc = new_loc;
		RecalcDirection();
	}
	else
		loc = new_loc;

	//for estimating where it really is
	last_loc = loc;
	last_loc_set_time = the_time;

	//set centers
	center_x = x + (width_pix >> 1);
	center_y = y + (height_pix >> 1);
}

void ZObject::SmoothMove(double &the_time)
{
	int &x = loc.x;
	int &y = loc.y;
	float &dx = loc.dx;
	float &dy = loc.dy;

	//move if it is moving
	if(!isz(dx)) loc.x = last_loc.x + floor(dx * (the_time - last_loc_set_time));
	if(!isz(dy)) loc.y = last_loc.y + floor(dy * (the_time - last_loc_set_time));

	//set centers
	center_x = x + (width_pix >> 1);
	center_y = y + (height_pix >> 1);
}

void ZObject::RecalcDirection()
{
	int new_dir;

	new_dir = DirectionFromLoc(loc.dx, loc.dy);

	printf("ZObject::RecalcDirection():%d\n", new_dir);

	if(new_dir != -1) direction = new_dir;
}

int ZObject::DirectionFromLoc(float dx, float dy)
{
	//const float z = 0.000001;
	float a;
	int dir;

	//are we going anywhere?
	//if((dx > -z && dx < z) && (dy > -z && dy < z))
	if(isz(dx) && isz(dy))
		return -1;

	a = atan2(dy,dx);

	//atan2 is kind of funky
	if(a < 0) a += PI + PI;
	a += PI * 1 / 8;

	if(a < PI / 4) dir = 0;
	else if(a < PI / 2) dir = 7;
	else if(a < PI * 3 / 4) dir = 6;
	else if(a < PI) dir = 5;
	else if(a < 5 * PI / 4) dir = 4;
	else if(a < 6 * PI / 4) dir = 3;
	else if(a < 7 * PI / 4) dir = 2;
	else if(a < 2 * PI) dir = 1;
	else dir = 0;

	//printf("DirectionFromLoc:a:%f dir:%d\n", a, dir);

	return dir;
}

ZObject* ZObject::GetObjectFromID_BS(int ref_id_, vector<ZObject*> &the_list)
{
	int low, high, midpoint;

	low = 0;
	high = the_list.size() - 1;
	midpoint = 0;

	//printf("ref id list: ");
	//for(vector<ZObject*>::iterator obj=the_list.begin(); obj!=the_list.end();obj++)
	//	printf("%d, ", (*obj)->GetRefID());
	//printf("\n");

	while (low <= high)
	{
		int tref_id;

		//midpoint = low + ((high - low) / 2);
		midpoint = low + ((high - low) >> 1);

		tref_id = the_list[midpoint]->GetRefID();

		if (ref_id_ == tref_id)
			return the_list[midpoint];
		else if (ref_id_ < tref_id)
			high = midpoint - 1;
		else
			low = midpoint + 1;
	}

	return nullptr;
}

ZObject* ZObject::GetObjectFromID(int ref_id_, vector<ZObject*> &the_list)
{
	vector<ZObject*>::iterator obj;

	return GetObjectFromID_BS(ref_id_, the_list);

	//for(obj=the_list.begin(); obj!=the_list.end();obj++)
	//	if((*obj)->ref_id == ref_id_)
	//		return *obj;

	//return nullptr;
}

void ZObject::SetAttackObject(ZObject *obj)
{
	attack_object = obj;
}

ZObject* ZObject::GetAttackObject()
{
	return attack_object;
}

ZObject* ZObject::NearestObjectFromList(vector<ZObject*> &the_list)
{
	ZObject *obj_choice;
	double least_distance;

	if(!the_list.size()) return nullptr;

	obj_choice = the_list[0];
	least_distance = DistanceFromObject(*obj_choice);
	for(vector<ZObject*>::iterator obj=the_list.begin()++;obj!=the_list.end();obj++)
	{
		double this_distance = DistanceFromObject(**obj);

		if(this_distance < least_distance)
		{
			least_distance = this_distance;
			obj_choice = *obj;
		}
	}

	return obj_choice;
}

void ZObject::RemoveObjectFromList(ZObject* the_object, vector<ZObject*> &the_list)
{
	vector<ZObject*>::iterator i;

	for(i=the_list.begin();i!=the_list.end();)
	{
		if(*i == the_object)
			i = the_list.erase(i);
		else
			i++;
	}
}

ZObject* ZObject::NearestObjectToCoords(vector<ZObject*> &the_list, int x, int y)
{
	ZObject *obj_choice;
	double least_distance;

	if(!the_list.size()) return nullptr;

	obj_choice = the_list[0];
	least_distance = obj_choice->DistanceFromCoords(x,y);
	for(vector<ZObject*>::iterator i=the_list.begin();i!=the_list.end();i++)
	{
		double this_distance;

		this_distance = (*i)->DistanceFromCoords(x,y);
		if(this_distance < least_distance)
		{
			least_distance = this_distance;
			obj_choice = *i;
		}
	}

	return obj_choice;
}

void ZObject::ClearAndDeleteList(vector<ZObject*> &the_list)
{
	for(vector<ZObject*>::iterator obj=the_list.begin(); obj!=the_list.end(); ++obj)
		if(*obj) delete *obj;

	the_list.clear();
}

void ZObject::ProcessList(vector<ZObject*> &the_list)
{
	for(vector<ZObject*>::iterator obj=the_list.begin(); obj!=the_list.end(); ++obj)
		if(*obj) (*obj)->Process();
}

ZObject* ZObject::NearestSelectableObject(vector<ZObject*> &the_list, int unit_type, int only_team, int x, int y)
{
	ZObject *obj_choice;
	double least_distance;

	if(!the_list.size()) return nullptr;

	obj_choice = nullptr;
	//obj_choice = the_list[0];
	//least_distance = obj_choice->DistanceFromCoords(x,y);
	for(vector<ZObject*>::iterator i=the_list.begin();i!=the_list.end();i++)
	{
		unsigned char ot, oid;
		double this_distance;

		if((*i)->GetOwner() != only_team) continue;
		if((*i)->IsMinion()) continue;

		(*i)->GetObjectID(ot, oid);
		if(ot != unit_type) continue;

		//this the first found?
		if(!obj_choice)
		{
			obj_choice = *i;
			least_distance = obj_choice->DistanceFromCoords(x,y);
			continue;
		}

		//is this one closer then the previous?
		this_distance = (*i)->DistanceFromCoords(x,y);
		if(this_distance < least_distance)
		{
			obj_choice = *i;
			least_distance = this_distance;
		}
	}

	return obj_choice;
}

ZObject* ZObject::NextSelectableObjectAboveID(vector<ZObject*> &the_list, int unit_type, int only_team, int min_ref_id)
{
	for(vector<ZObject*>::iterator i=the_list.begin();i!=the_list.end();i++)
	{
		unsigned char ot, oid;

		if((*i)->GetRefID() <= min_ref_id) continue;
		if((*i)->GetOwner() != only_team) continue;
		if((*i)->IsMinion()) continue;

		(*i)->GetObjectID(ot, oid);
		if(ot != unit_type) continue;
			
		return *i;
	}

	return nullptr;
}

void ZObject::RemoveObject(ZObject *obj)
{
	if(attack_object == obj)
	{
		SetAttackObject(nullptr);

		//only the server needs this
		//but it doesn't hurt to do it in the client
		SignalLidShouldClose();
	}

	//this is kind of crude but it needs to be done
	for(vector<ZObject*>::iterator i=minion_list.begin();i!=minion_list.end();i++)
		if(*i == obj) *i = nullptr;

	//crude
	if(leader_obj == obj) leader_obj = nullptr;
}

//bool ZObject::ServerFireTurrentMissile(int &x_, int &y_, int &damage, int &radius, double &offset_time)
//{
//	x_ = 0;
//	y_ = 0;
//	damage = 0;
//	radius = 0;
//	offset_time = 0;
//	return false;
//}

vector<fire_missile_info> ZObject::ServerFireTurrentMissile(int &damage, int &radius)
{
	vector<fire_missile_info> ret;

	damage = 0;
	radius = 0;

	return ret;
}

void ZObject::FireTurrentMissile(int x_, int y_, double offset_time)
{

}

ZGuiWindow *ZObject::MakeGuiWindow()
{
	return nullptr;
}

void ZObject::ProcessSetBuiltCannonData(char *data, int size)
{

}

void ZObject::ProcessSetBuildingStateData(char *data, int size)
{

}

bool ZObject::GetBuildingCreationPoint(int &x, int &y)
{
	return false;
}

bool ZObject::GetBuildingCreationMovePoint(int &x, int &y)
{
	return false;
}

bool ZObject::GetBuildUnit(unsigned char &ot, unsigned char &oid)
{
	return false;
}

bool ZObject::BuildUnit(double &the_time, unsigned char &ot, unsigned char &oid)
{
	return false;
}

void ZObject::ResetProduction()
{

}

void ZObject::SetBuildList(ZBuildList *buildlist_)
{
	buildlist = buildlist_;
}

bool ZObject::StoreBuiltCannon(unsigned char oid)
{
	return false;
}

map_zone_info *ZObject::GetConnectedZone()
{
	return connected_zone;
}

void ZObject::SetConnectedZone(map_zone_info *connected_zone_)
{
	connected_zone = connected_zone_;
}

void ZObject::SetConnectedZone(ZMap &the_map)
{
	connected_zone = the_map.GetZone(loc.x, loc.y);
}

bool ZObject::RemoveStoredCannon(unsigned char oid)
{
	return false;
}

bool ZObject::HaveStoredCannon(unsigned char oid)
{
	return false;
}

int ZObject::CannonsInZone(ZOLists &ols)
{
	int cannons_found = 0;

	for(vector<ZObject*>::iterator i=ols.object_list->begin();i!=ols.object_list->end();i++)
		if(this != *i && connected_zone == (*i)->GetConnectedZone())
		{
			unsigned char ot, oid;

			(*i)->GetObjectID(ot, oid);

			if(ot != CANNON_OBJECT) continue;

			cannons_found++;
		}

	return cannons_found;
}

vector<unsigned char> &ZObject::GetBuiltCannonList()
{
	static vector<unsigned char> arg;

	return arg;
}

void ZObject::SetUnitLimitReachedList(bool *unit_limit_reached_)
{
	unit_limit_reached = unit_limit_reached_;
}

void ZObject::SetLevel(int level_)
{

}

int ZObject::GetLevel()
{
	return 0;
}

void ZObject::DoPreRender(ZMap &the_map, SDL_Surface *dest, int shift_x, int shift_y)
{

}

bool ZObject::HasDestroyedFortInZone(ZOLists &ols)
{
	if(!connected_zone) return false;

	for(vector<ZObject*>::iterator i=ols.building_olist.begin(); i!=ols.building_olist.end(); ++i)
	{
		unsigned char ot, oid;

		if(connected_zone != (*i)->GetConnectedZone()) continue;
		if(!(*i)->IsDestroyed()) continue;

		(*i)->GetObjectID(ot, oid);

		if(ot == BUILDING_OBJECT && (oid == FORT_FRONT || oid == FORT_FRONT)) return true;
	}

	return false;
}

bool ZObject::DoAutoRepair(ZMap &tmap, ZOLists &ols)
{
	double &the_time = ztime->ztime;

	if(!do_auto_repair) return false;

	if(the_time >= next_auto_repair_time)
	{
		do_auto_repair = false;

		//nevermind?
		if(HasDestroyedFortInZone(ols)) return false;
		
		//do the repair
		//health = max_health;
		SetHealth(max_health, tmap);

		//let them know
		return true;
	}

	return false;
}

void ZObject::StopAutoRepair()
{
	do_auto_repair = false;
}

int ZObject::GetBuildState()
{
	return -1;
}

void ZObject::SetMapImpassables(ZMap &tmap)
{

}

void ZObject::UnSetMapImpassables(ZMap &tmap)
{

}

void ZObject::SetDestroyMapImpassables(ZMap &tmap)
{

}

void ZObject::UnSetDestroyMapImpassables(ZMap &tmap)
{

}

void ZObject::AddGroupMinion(ZObject *obj)
{
	if(!obj) return;
	if(obj == this) return;

	minion_list.push_back(obj);
}

void ZObject::RemoveGroupMinion(ZObject *obj)
{
	for(vector<ZObject*>::iterator i=minion_list.begin();i!=minion_list.end();)
	{
		if(!*i || *i == obj)
			i = minion_list.erase(i);
		else
			i++;
	}
}

vector<ZObject*> &ZObject::GetMinionList()
{
	return minion_list;
}

ZObject* ZObject::GetGroupLeader()
{
	return leader_obj;
}

void ZObject::SetGroupLeader(ZObject *obj)
{
	//cant be your own leader
	if(obj == this) return;

	//if(leader_obj) minion_list.clear();

	leader_obj = obj;
}

void ZObject::ClearGroupInfo()
{
	minion_list.clear();
	leader_obj = nullptr;
}

bool ZObject::IsMinion()
{
	return leader_obj;
}

bool ZObject::IsApartOfAGroup()
{
	return leader_obj || minion_list.size();
}

void ZObject::CloneMinionWayPoints()
{
	for(vector<ZObject*>::iterator i=minion_list.begin();i!=minion_list.end();i++)
	{
		if(!*i) continue;

		(*i)->GetWayPointList() = waypoint_list;
		(*i)->SetVelocity();

		//just left cannon...
		(*i)->SetJustLeftCannon(just_left_cannon);
	}
}

void ZObject::CreateGroupInfoData(char *&data, int &size)
{
	//only robots can create groups so
	if(object_type != ROBOT_OBJECT)
	{
		data = nullptr;
		size = 0;
		return;
	}

	//int ref_id;
	int leader_ref_id;
	int minions;

	if(leader_obj)
		leader_ref_id = leader_obj->GetRefID();
	else
		leader_ref_id = -1;

	minions = minion_list.size();

	size = 12 + (4 * minions);

	data = (char*)malloc(size);
	((int*)data)[0] = ref_id;
	((int*)data)[1] = leader_ref_id;
	((int*)data)[2] = minions;

	for(int i=0;i<minions;i++)
	{
		if(minion_list[i])
			((int*)data)[2+i] = minion_list[i]->GetRefID();
		else
			((int*)data)[2+i] = -1;
	}
}

void ZObject::CreateTeamData(char *&data, int &size)
{
	object_team_packet packet_header;

	size = sizeof(object_team_packet) + (driver_info.size() * sizeof(driver_info_s));
	data = (char*)malloc(size);

	packet_header.ref_id = ref_id;
	packet_header.owner = owner;
	packet_header.driver_type = driver_type;
	packet_header.driver_amount = driver_info.size();

	memcpy(data, &packet_header, sizeof(object_team_packet));

	int shift_amt = sizeof(object_team_packet);
	for(vector<driver_info_s>::iterator i=driver_info.begin();i!=driver_info.end();i++)
	{
		memcpy(data + shift_amt, &(*i), sizeof(driver_info_s));
		shift_amt += sizeof(driver_info_s);
	}
}

void ZObject::ProcessGroupInfoData(char *data, int size, vector<ZObject*> &object_list)
{
	int ref_id_;
	int leader_ref_id;
	int minions;

	//meet min requirements?
	if(size < 12) return;

	ref_id_ = ((int*)data)[0];
	leader_ref_id = ((int*)data)[1];
	minions = ((int*)data)[2];

	//other requirements?
	if(ref_id_ != ref_id) return;
	if(size != 12 + (4 * minions)) return;

	ClearGroupInfo();

	leader_obj = GetObjectFromID(leader_ref_id, object_list);

	//fill the minion list
	for(int i=0;i<minions;i++)
	{
		ZObject *new_minion;

		new_minion = GetObjectFromID(((int*)data)[2+i], object_list);

		if(new_minion)
		{
			minion_list.push_back(new_minion);
		}
		else
		{
			printf("ProcessGroupInfoData:could not find a minion\n");
		}
	}
}

void ZObject::SetupRockRender(bool **rock_list, int map_w, int map_h)
{

}

bool ZObject::CanBeDestroyed()
{
	return can_be_destroyed;
}

void ZObject::CreationMapEffects(ZMap &tmap)
{

}

void ZObject::DeathMapEffects(ZMap &tmap)
{

}

bool ZObject::ResetBuildTime(float zone_ownage)
{
	return false;
}

bool ZObject::RecalcBuildTime()
{
	return false;
}

double ZObject::SpeedOffsetPercent()
{
	if(!move_speed) return 1.0;
	if(!IsMoving()) return 1.0;

	return sqrt((loc.dx*loc.dx)+(loc.dy*loc.dy)) / move_speed;
}

bool ZObject::CanBeRepairedByCrane(int repairers_team)
{
	if(object_type != BUILDING_OBJECT) return false;
	if(object_id == FORT_FRONT) return false;
	if(object_id == FORT_BACK) return false;

	if(owner != NULL_TEAM && repairers_team != owner) return false;
	if(!IsDestroyed()) return false;

	return true;
}

bool ZObject::CanAttack()
{
	return damage && !IsDestroyed();
}

bool ZObject::HasExplosives()
{
	if(has_explosives) return true;
	if(GetGrenadeAmount()) return true;
	if(GetGroupLeader() && GetGroupLeader()->GetGrenadeAmount()) return true;

	return false;
	//return has_explosives;
}

bool ZObject::AttackedOnlyByExplosives()
{
	return attacked_by_explosives;
}

void ZObject::SetDriverType(int driver_type_)
{
	driver_type = driver_type_;

	if(driver_type < 0) driver_type = 0;
	if(driver_type >= MAX_ROBOT_TYPES) driver_type = MAX_ROBOT_TYPES-1;

	//drivers can mess with damage info
	ResetDamageInfo();
}

void ZObject::AddDriver(int driver_health_)
{
	driver_info_s new_driver;

	new_driver.driver_health = driver_health_;

	AddDriver(new_driver);
}

void ZObject::AddDriver(driver_info_s new_driver)
{
	new_driver.next_attack_time = 0;

	driver_info.push_back(new_driver);

	//drivers can mess with damage info
	ResetDamageInfo();
}

vector<driver_info_s> &ZObject::GetDrivers()
{
	return driver_info;
}

void ZObject::ClearDrivers()
{
	driver_info.clear();

	//drivers can mess with damage info
	ResetDamageInfo();
}

int ZObject::GetDriverType()
{
	return driver_type;
}

int ZObject::GetDriverHealth()
{
	if(driver_info.size())
		return driver_info.begin()->driver_health;
	else
		return 0;
}

bool ZObject::CanEjectDrivers()
{
	return false;
}

void ZObject::SetInitialDrivers()
{
	driver_type = GRUNT;
	ClearDrivers();
}

void ZObject::SetEjectableCannon(bool ejectable_)
{

}

void ZObject::ResetDamageInfo()
{

}

unsigned short ZObject::GetExtraLinks()
{
	return 0;
}

void ZObject::DoHitEffect()
{
	do_hit_effect = true;
}

bool ZObject::GetCraneEntrance(int &x, int &y, int &x2, int &y2)
{
	return false;
}

bool ZObject::GetCraneCenter(int &x, int &y)
{
	return false;
}

bool ZObject::GetRepairEntrance(int &x, int &y)
{
	return false;
}

bool ZObject::GetRepairCenter(int &x, int &y)
{
	return false;
}

void ZObject::DoCraneAnim(bool on_, ZObject *rep_obj)
{

}

bool ZObject::CanBeRepaired()
{
	return false;
}

bool ZObject::CanRepairUnit(int units_team)
{
	return false;
}

bool ZObject::RepairingAUnit()
{
	return false;
}

bool ZObject::SetRepairUnit(ZObject *unit_obj)
{
	return false;
}

bool ZObject::GetLidState()
{
	return false;
}

void ZObject::SetLidState(bool lid_open_)
{

}

void ZObject::DoRepairBuildingAnim(bool on_, double remaining_time_)
{

}

bool ZObject::RepairUnit(double &the_time, unsigned char &ot, unsigned char &oid, int &driver_type_, vector<driver_info_s> &driver_info_, vector<waypoint> &rwaypoint_list)
{
	return false;
}

void ZObject::ProcessServerLid()
{

}

void ZObject::SignalLidShouldOpen()
{

}

void ZObject::SignalLidShouldClose()
{

}

bool ZObject::CanBeSniped()
{
	return can_be_sniped && driver_info.size();
}

bool ZObject::CanSnipe()
{
	return can_snipe;
}

bool ZObject::HasLid()
{
	return has_lid;
}

bool ZObject::HasProcessedDeath()
{
	return processed_death;
}

void ZObject::SetHasProcessedDeath(bool processed_death_)
{
	processed_death = processed_death_;
}

void ZObject::DoDriverHitEffect()
{
	do_driver_hit_effect = true;
}

bool ZObject::CanBeEntered()
{
	if(owner != NULL_TEAM) return false;
	if(IsDestroyed()) return false;
	if(!(object_type == VEHICLE_OBJECT || object_type == CANNON_OBJECT)) return false;

	return true;
}

void ZObject::SetDamagedByFireTime(double the_time)
{
	last_damaged_by_fire_time = the_time;
}

void ZObject::SetDamagedByMissileTime(double the_time)
{
	last_damaged_by_missile_time = the_time;
}

double ZObject::GetDamagedByFireTime()
{
	return last_damaged_by_fire_time;
}

double ZObject::GetDamagedByMissileTime()
{
	return last_damaged_by_missile_time;
}

bool ZObject::CanEnterFort(int team)
{
	return false;
}

bool ZObject::CanSetWaypoints()
{
	return false;
}

bool ZObject::CanMove()
{
	return move_speed;
}

bool ZObject::CanAttackObject(ZObject *obj)
{
	if(!obj) return false;

	if(!CanAttack()) return false;
	if(obj->IsDestroyed()) return false;
	if(owner == obj->GetOwner()) return false;
	if(!HasExplosives() && obj->AttackedOnlyByExplosives()) return false;

	return true;
}

void ZObject::PostPathFindingResult(ZPath_Finding_Response* response)
{
	if(!response) return;

	if(response->thread_id == cur_wp_info.path_finding_id)
	{
		//printf("got a good response - thread_id:%d\n", response->thread_id);
		cur_wp_info.got_pf_response = true;
		cur_wp_info.pf_point_list = response->pf_point_list;
		cur_wp_info.path_finding_id = 0;

		if(cur_wp_info.pf_point_list.size())
		{
			//cur_wp_info.x = cur_wp_info.pf_point_list.begin()->x;
			//cur_wp_info.y = cur_wp_info.pf_point_list.begin()->y;
			SetTarget(cur_wp_info.pf_point_list.begin()->x, cur_wp_info.pf_point_list.begin()->y);
			SetVelocity();

			cur_wp_info.pf_point_list.erase(cur_wp_info.pf_point_list.begin());
		}
	}
}

bool sort_objects_func (ZObject *a, ZObject *b)
{
	return ((a->loc.y + a->height_pix) < (b->loc.y + b->height_pix));
}




// ==================================================================
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// ==================================================================

ZOLists::ZOLists()
{
    object_list = nullptr;
}

void ZOLists::DeleteAllObjects()
{
    for(vector<ZObject*>::iterator o=object_list->begin(); o!=object_list->end(); o++)
        delete *o;

    //clear the lists
    object_list->clear();
    flag_olist.clear();
    cannon_olist.clear();
    building_olist.clear();
    rock_olist.clear();
    passive_engagable_olist.clear();
    mobile_olist.clear();
    prender_olist.clear();
    non_mapitem_olist.clear();
    grenades_olist.clear();
}

void ZOLists::RemoveObject(ZObject *obj)
{
    RemoveObjectFromList(obj, object_list);
    RemoveObjectFromList(obj, &flag_olist);
    RemoveObjectFromList(obj, &cannon_olist);
    RemoveObjectFromList(obj, &building_olist);
    RemoveObjectFromList(obj, &rock_olist);
    RemoveObjectFromList(obj, &passive_engagable_olist);
    RemoveObjectFromList(obj, &mobile_olist);
    RemoveObjectFromList(obj, &prender_olist);
    RemoveObjectFromList(obj, &non_mapitem_olist);
    RemoveObjectFromList(obj, &grenades_olist);
}

void ZOLists::DeleteObject(ZObject *obj)
{
    delete obj;

    RemoveObject(obj);
}

void ZOLists::AddObject(ZObject *obj)
{
    //already exist?
    for(vector<ZObject*>::iterator o=object_list->begin(); o!=object_list->end(); o++)
        if(*o == obj)
            return;

    //push back
    object_list->push_back(obj);

    unsigned char ot, oid;

    obj->GetObjectID(ot, oid);

    if(ot == CANNON_OBJECT) cannon_olist.push_back(obj);
    if(ot == BUILDING_OBJECT) building_olist.push_back(obj);
    if(ot == MAP_ITEM_OBJECT && oid == ROCK_OBJECT) rock_olist.push_back(obj);
    if(ot == CANNON_OBJECT) cannon_olist.push_back(obj);
    if(ot == CANNON_OBJECT || ot == ROBOT_OBJECT || ot == VEHICLE_OBJECT) passive_engagable_olist.push_back(obj);
    if(ot == ROBOT_OBJECT || ot == VEHICLE_OBJECT) mobile_olist.push_back(obj);
    prender_olist.push_back(obj);
    if(ot != MAP_ITEM_OBJECT) non_mapitem_olist.push_back(obj);
    if(ot == MAP_ITEM_OBJECT && oid == FLAG_ITEM) flag_olist.push_back(obj);
    if(ot == MAP_ITEM_OBJECT && oid == GRENADES_ITEM) grenades_olist.push_back(obj);
}

void ZOLists::DeleteObjectFromList(ZObject *obj, vector<ZObject*> *olist)
{
    delete obj;

    RemoveObjectFromList(obj, olist);
}

void ZOLists::RemoveObjectFromList(ZObject *obj, vector<ZObject*> *olist)
{
    for(vector<ZObject*>::iterator o=olist->begin(); o!=olist->end();)
    {
        if(*o == obj)
            o = olist->erase(o);
        else
            o++;
    }
}

void ZOLists::SetupFlagList()
{
    flag_olist.clear();

    for(vector<ZObject*>::iterator i=object_list->begin(); i!=object_list->end(); i++)
    {
        unsigned char ot, oid;
        (*i)->GetObjectID(ot, oid);

        if(ot == MAP_ITEM_OBJECT && oid == FLAG_ITEM)
            flag_olist.push_back(*i);
    }
}
// ==================================================================
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// ==================================================================
bool ZPortrait::finished_init = false;

ZSDL_Surface ZPortrait::backdrop[MAX_PLANET_TYPES];
ZSDL_Surface ZPortrait::backdrop_vehicle;
ZPortrait_Unit_Graphics ZPortrait::unit_graphics[MAX_ROBOT_TYPES][MAX_TEAM_TYPES];
ZPortrait_Anim ZPortrait::anim_info[MAX_PORTRAIT_ANIMS];
ZPortrait_Frame ZPortrait::still_frame;

ZPortrait::ZPortrait()
{
    x = 0;
    y = 0;
    over_map = false;
    terrain = DESERT;
    team = RED_TEAM;
    render_frame = nullptr;
    ref_id = -1;
    do_random_anims = true;

    ClearRobotID();
}

void ZPortrait::Init()
{
    int i, j;
    string filename;

    filename = "assets/other/hud/backdrop_vehicle.bmp";
    backdrop_vehicle.LoadBaseImage(filename);

    for(i=0;i<MAX_PLANET_TYPES;i++)
    {
        filename = "assets/other/hud/backdrop_" + planet_type_string[i] + ".bmp";
        backdrop[i].LoadBaseImage(filename);
    }

    for(i=0;i<MAX_ROBOT_TYPES;i++)
        for(j=0;j<MAX_TEAM_TYPES;j++)
            unit_graphics[i][j].Load((robot_type)i, (team_type)j, unit_graphics[i][ZTEAM_BASE_TEAM]);

    SetupFrames();

    finished_init = true;
}

void ZPortrait_Unit_Graphics::Load(robot_type oid, team_type team, ZPortrait_Unit_Graphics &bc)
{
    char filename_c[500];
    int old_z_face_id;
    int i, j;

    if(team == NULL_TEAM) return;

    switch(oid)
    {
    case GRUNT: old_z_face_id = 2; break;
    case PSYCHO: old_z_face_id = 3; break;
    case SNIPER: old_z_face_id = 4; break;
    case TOUGH: old_z_face_id = 0; break;
    case PYRO: old_z_face_id = 1; break;
    case LASER: old_z_face_id = 1; break;
    default:
        printf("ZPortrait_Unit_Graphics::invalid robot id!\n");
        return;
        break;
    }

    //load em
    i=0;
    sprintf(filename_c, "assets/other/hud/portraits/%s_%s/SHEADBI%d_%04d.png",  robot_type_string[oid].c_str(), team_type_string[team].c_str(), old_z_face_id, i++);
    //shoulders.LoadBaseImage(filename_c);
    ZTeam::LoadZSurface(team, bc.shoulders, shoulders, filename_c);

    sprintf(filename_c, "assets/other/hud/portraits/%s_%s/SHEADBI%d_%04d.png",  robot_type_string[oid].c_str(), team_type_string[team].c_str(), old_z_face_id, i++);
    //head[LOOK_STRAIGHT].LoadBaseImage(filename_c);
    ZTeam::LoadZSurface(team, bc.head[LOOK_STRAIGHT], head[LOOK_STRAIGHT], filename_c);

    sprintf(filename_c, "assets/other/hud/portraits/%s_%s/SHEADBI%d_%04d.png",  robot_type_string[oid].c_str(), team_type_string[team].c_str(), old_z_face_id, i++);
    //head[LOOK_RIGHT].LoadBaseImage(filename_c);
    ZTeam::LoadZSurface(team, bc.head[LOOK_RIGHT], head[LOOK_RIGHT], filename_c);

    sprintf(filename_c, "assets/other/hud/portraits/%s_%s/SHEADBI%d_%04d.png",  robot_type_string[oid].c_str(), team_type_string[team].c_str(), old_z_face_id, i++);
    //head[LOOK_LEFT].LoadBaseImage(filename_c);
    ZTeam::LoadZSurface(team, bc.head[LOOK_LEFT], head[LOOK_LEFT], filename_c);

    for(j=0;j<MAX_MOUTHS;j++)
    {
        sprintf(filename_c, "assets/other/hud/portraits/%s_%s/SHEADBI%d_%04d.png",  robot_type_string[oid].c_str(), team_type_string[team].c_str(), old_z_face_id, i++);
        //mouth[j].LoadBaseImage(filename_c);
        ZTeam::LoadZSurface(team, bc.mouth[j], mouth[j], filename_c);
    }

    for(j=0;j<MAX_EYES;j++)
    {
        sprintf(filename_c, "assets/other/hud/portraits/%s_%s/SHEADBI%d_%04d.png",  robot_type_string[oid].c_str(), team_type_string[team].c_str(), old_z_face_id, i++);
        //eyes[j].LoadBaseImage(filename_c);
        ZTeam::LoadZSurface(team, bc.eyes[j], eyes[j], filename_c);
    }

    for(j=0;j<MAX_HANDS;j++)
    {
        sprintf(filename_c, "assets/other/hud/portraits/%s_%s/SHEADBI%d_%04d.png",  robot_type_string[oid].c_str(), team_type_string[team].c_str(), old_z_face_id, i++);
        //hand[j].LoadBaseImage(filename_c);
        ZTeam::LoadZSurface(team, bc.hand[j], hand[j], filename_c);
    }
}

void ZPortrait::SetCords(int x_, int y_)
{
    x = x_;
    y = y_;
}

void ZPortrait::SetOverMap(bool over_map_)
{
    over_map = over_map_;
}

void ZPortrait::SetTerrainType(planet_type terrain_)
{
    terrain = terrain_;
}

void ZPortrait::SetInVehicle(bool in_vehicle_)
{
    in_vehicle = in_vehicle_;
}

void ZPortrait::SetTeam(int team_)
{
    team = team_;
}

void ZPortrait::SetRobotID(unsigned char oid_)
{
    oid = oid_;
    do_render = true;

    if(oid < 0) oid = GRUNT;
    if(oid >= MAX_ROBOT_TYPES) oid = GRUNT;
}

void ZPortrait::SetObject(ZObject *obj)
{
    ClearRobotID();

    if(!obj) return;

    unsigned char ot, oid;

    obj->GetObjectID(ot, oid);

    SetTeam(obj->GetOwner());

    ref_id = obj->GetRefID();

    switch(ot)
    {
    case ROBOT_OBJECT:
        SetInVehicle(false);
        SetRobotID(oid);
        break;
    case VEHICLE_OBJECT:
        SetRobotID(obj->GetDriverType());
        SetInVehicle(true);
        break;
    case CANNON_OBJECT:
        SetRobotID(obj->GetDriverType());
        SetInVehicle(true);
        break;
    }
}

void ZPortrait::ClearRobotID()
{
    oid = GRUNT;
    in_vehicle = false;
    do_render = false;
    render_frame = &still_frame;
    cur_anim = -1;
    anim_start_time = 0;
    ref_id = -1;
}

void ZPortrait::StartAnim(int anim_)
{
    if(!anim_info[anim_].frame_list.size()) return;

    cur_anim = anim_;
    render_frame = &anim_info[cur_anim].frame_list[0];
    anim_start_time = COMMON::current_time();

    PlayAnimSound();
    //printf("StartAnim::%d:%s\n", cur_anim, portrait_anim_string[cur_anim].c_str());
}

void ZPortrait::StartRandomAnim()
{
    if(!do_random_anims) return;

    switch(rand() % 13)
    {
    case 0: StartAnim(BLINK_ANIM); break;
    case 1: StartAnim(WINK_ANIM); break;
    case 2: StartAnim(SURPRISE_ANIM); break;
    case 3: StartAnim(ANGER_ANIM); break;
    case 4: StartAnim(GRIN_ANIM); break;
    case 5: StartAnim(SCARED_ANIM); break;
    case 6: StartAnim(EYES_LEFT_ANIM); break;
    case 7: StartAnim(EYES_RIGHT_ANIM); break;
    case 8: StartAnim(EYES_UP_ANIM); break;
    case 9: StartAnim(EYES_DOWN_ANIM); break;
    case 10: StartAnim(WHISTLE_ANIM); break;
    case 11: StartAnim(LOOK_LEFT_ANIM); break;
    case 12: StartAnim(LOOK_RIGHT_ANIM); break;
    }
}

bool ZPortrait::DoingAnim()
{
    return cur_anim != -1;
}

int ZPortrait::Process()
{
    double the_time = COMMON::current_time();
    static int test_anim = 0;
    //static double next_test_anim_time = 0;

    if(!finished_init) return 1;

    if(cur_anim != -1)
    {
        double time_in;

        time_in = the_time - anim_start_time;

        //anim over?
        if(time_in > anim_info[cur_anim].total_duration)
        {
            cur_anim = -1;
            render_frame = &still_frame;

            //next_test_anim_time = the_time + 0.5;
            next_random_anim_time = the_time + 0.5 + ((rand() % 50) * 0.1);
        }
        else
        {
            //set next frame in this animation
            render_frame = &still_frame;

            //set current frame
            double cur_duration = 0;
            vector<ZPortrait_Frame> *cur_list = &anim_info[cur_anim].frame_list;
            for(vector<ZPortrait_Frame>::iterator i=cur_list->begin(); i!=cur_list->end(); ++i)
            {
                if(cur_duration <= time_in)
                    render_frame = &(*i);
                else
                    break;

                cur_duration += i->duration;
            }
        }
    }
    else
    {
        //start a random animation?
        if(the_time >= next_random_anim_time) StartRandomAnim();

        /*
        if(the_time < next_test_anim_time) return 1;

        //we have no animation
        if(test_anim>=MAX_PORTRAIT_ANIMS) test_anim=0;
        StartAnim(test_anim++);
        //StartAnim(FOR_CHRIST_SAKE_ANIM);
        */
    }

    return 1;
}

void ZPortrait::DoRender()
{
    SDL_Rect from_rect, to_rect;

    if(do_render)
    {
        ZSDL_Surface *backdrop_surface;

        //backdrop
        if(in_vehicle)
            backdrop_surface = &backdrop_vehicle;
        else
            backdrop_surface = &backdrop[terrain];

        if(!over_map)
        {
            to_rect.x = x;
            to_rect.y = y;

            backdrop_surface->BlitSurface(nullptr, &to_rect);
        }
        else
            backdrop_surface->RenderSurface(x, y);

        RenderFace();
    }
    else
    {
        //clear
        if(!over_map)
        {
            to_rect.x = x;
            to_rect.y = y;
            to_rect.w = 86;
            to_rect.h = 74;

            ZSDL_FillRect(&to_rect, 0, 0, 0);
        }
        else
        {
            //different color and croppable when over the map.
        }
    }
}

void ZPortrait::RenderFace()
{
    SDL_Rect from_rect, to_rect;
    ZPortrait_Unit_Graphics *choosen_graphics;
    ZSDL_Surface *render_surface;
    int face_offset_y;

    //got face render info?
    if(!render_frame) return;

    choosen_graphics = &unit_graphics[oid][team];

    //head
    {
        //face
        {
            render_surface = &choosen_graphics->head[render_frame->look_direction];

            to_rect.x = render_frame->head_x;
            to_rect.y = render_frame->head_y;

            to_rect.x += x;
            to_rect.y += y;

            if(!over_map)
                render_surface->BlitSurface(nullptr, &to_rect);
            else
                render_surface->RenderSurface(to_rect.x, to_rect.y);
        }

        if(render_frame->look_direction == LOOK_STRAIGHT)
        {
            switch(oid)
            {
                case GRUNT: face_offset_y = 0; break;
                case SNIPER: face_offset_y = 4; break;
                default: face_offset_y = 2; break;
            }

            //eyes
            {
                render_surface = &choosen_graphics->eyes[render_frame->eyes];

                to_rect.x = 14 + render_frame->head_x;
                to_rect.y = 8 + render_frame->head_y + face_offset_y;

                to_rect.x += x;
                to_rect.y += y;

                if(!over_map)
                    render_surface->BlitSurface(nullptr, &to_rect);
                else
                    render_surface->RenderSurface(to_rect.x, to_rect.y);
            }

            //mouth
            {
                render_surface = &choosen_graphics->mouth[render_frame->mouth];

                to_rect.x = 22 + render_frame->head_x;
                to_rect.y = 24 + render_frame->head_y + face_offset_y;

                to_rect.x += x;
                to_rect.y += y;

                if(!over_map)
                    render_surface->BlitSurface(nullptr, &to_rect);
                else
                    render_surface->RenderSurface(to_rect.x, to_rect.y);
            }
        }
    }

    //shoulders
    if(choosen_graphics->shoulders.GetBaseSurface())
    {
        to_rect.x = 0;
        to_rect.y = 74 - choosen_graphics->shoulders.GetBaseSurface()->h;

        to_rect.x += x;
        to_rect.y += y;

        if(!over_map)
            choosen_graphics->shoulders.BlitSurface(nullptr, &to_rect);
        else
            choosen_graphics->shoulders.RenderSurface(to_rect.x, to_rect.y);
    }


    //hand
    if(render_frame->hand_do_render)
    {
        render_surface = &choosen_graphics->hand[render_frame->hand];

        to_rect.x = render_frame->hand_x;
        to_rect.y = render_frame->hand_y;

        if(GetBlitInfo(render_surface->GetBaseSurface(), to_rect.x, to_rect.y, from_rect, to_rect))
        {
            to_rect.x += x;
            to_rect.y += y;

            if(!over_map)
                render_surface->BlitSurface(&from_rect, &to_rect);
            else
                render_surface->RenderSurface(to_rect.x, to_rect.y);
        }
    }
}

ZPortrait_Anim::ZPortrait_Anim()
{
    total_duration = 0;
}

void ZPortrait::PlayAnimSound()
{
    static int last_end_anim = -1;
    int next_end_anim;

    switch(cur_anim)
    {
    case YES_SIR_ANIM:
        if(rand() % 2)
            ZSoundEngine::PlayWav(YES_SIR1_SND);
        else
            ZSoundEngine::PlayWav(YES_SIR2_SND);
        break;
    case YES_SIR3_ANIM: ZSoundEngine::PlayWav(YES_SIR3_SND); break;
    case UNIT_REPORTING1_ANIM: ZSoundEngine::PlayWav(UNIT_REPORTING1_SND); break;
    case UNIT_REPORTING2_ANIM:
        if(rand() % 2)
            ZSoundEngine::PlayWav(UNIT_REPORTING2_SND);
        else
            ZSoundEngine::PlayWav(UNIT_REPORTING3_SND);
        break;
    case GRUNTS_REPORTING_ANIM: ZSoundEngine::PlayWav(GRUNTS_REPORTING_SND); break;
    case PSYCHOS_REPORTING_ANIM: ZSoundEngine::PlayWav(PSYCHOS_REPORTING_SND); break;
    case SNIPERS_REPORTING_ANIM: ZSoundEngine::PlayWav(SNIPERS_REPORTING_SND); break;
    case TOUGHS_REPORTING_ANIM: ZSoundEngine::PlayWav(TOUGHS_REPORTING_SND); break;
    case LASERS_REPORTING_ANIM: ZSoundEngine::PlayWav(LASERS_REPORTING_SND); break;
    case PYROS_REPORTING_ANIM: ZSoundEngine::PlayWav(PYROS_REPORTING_SND); break;
    case WERE_ON_OUR_WAY_ANIM: ZSoundEngine::PlayWav(WERE_ON_OUR_WAY_SND); break;
    case HERE_WE_GO_ANIM: ZSoundEngine::PlayWav(HERE_WE_GO_SND); break;
    case YOUVE_GOT_IT_ANIM: ZSoundEngine::PlayWav(YOU_GOT_IT_SND); break;
    case MOVING_IN_ANIM: ZSoundEngine::PlayWav(MOVING_IN_SND); break;
    case OKAY_ANIM: ZSoundEngine::PlayWav(OKAY_SND); break;
    case ALRIGHT_ANIM: ZSoundEngine::PlayWav(ALRIGHT_SND); break;
    case NO_PROBLEM_ANIM: ZSoundEngine::PlayWav(NO_PROBLEM_SND); break;
    case OVER_N_OUT_ANIM: ZSoundEngine::PlayWav(OVER_AND_OUT_SND); break;
    case AFFIRMATIVE_ANIM: ZSoundEngine::PlayWav(AFFIRMATIVE_SND); break;
    case GOING_IN_ANIM: ZSoundEngine::PlayWav(GOING_IN_SND); break;
    case LETS_DO_IT_ANIM: ZSoundEngine::PlayWav(LETS_DO_IT_SND); break;
    case LETS_GET_EM_ANIM: ZSoundEngine::PlayWav(LETS_GET_THEM_SND); break;
    case WERE_UNDER_ATTACK_ANIM: ZSoundEngine::PlayWav(WERE_UNDER_ATTACK_SND); break;
    case I_SAID_WERE_UNDER_ATTACK_ANIM: ZSoundEngine::PlayWav(I_SAID_WERE_UNDER_ATTACK_SND); break;
    case HELP_HELP_ANIM: ZSoundEngine::PlayWav(HELP_HELP_SND); break;
    case THEYRE_ALL_OVER_US_ANIM: ZSoundEngine::PlayWav(THEYRE_ALL_OVER_US_SND); break;
    case WERE_LOSEING_IT_ANIM: ZSoundEngine::PlayWav(WERE_LOSING_IT_SND); break;
    case AAAHHH_ANIM: ZSoundEngine::PlayWav(AAAHHH_SND); break;
    //where is the "oh my god" anim? -> OH_MY_GOD_SND
    case FOR_CHRIST_SAKE_ANIM: ZSoundEngine::PlayWav(FOR_CHRIST_SAKE_SND); break;
    case YOURE_JOKING_ANIM: ZSoundEngine::PlayWav(YOUR_JOKING_SND); break;
    case TARGET_DESTROYED_ANIM: ZSoundEngine::PlayWav(TARGET_DESTROYED_SND); break;
    case YES_SIR_SALUTE_ANIM:
        if(rand() % 2)
            ZSoundEngine::PlayWav(YES_SIR1_SND);
        else
            ZSoundEngine::PlayWav(YES_SIR2_SND);
        break;
    case GOING_IN_THUMBS_UP_ANIM: ZSoundEngine::PlayWav(GOING_IN_SND); break;
    case FORGET_IT_ANIM: ZSoundEngine::PlayWav(FORGET_IT_SND); break;
    case GET_OUTTA_HERE_ANIM: ZSoundEngine::PlayWav(GET_OUTTA_HERE_SND); break;
    case GOOD_HIT_ANIM: ZSoundEngine::PlayWav(GOOD_HIT_SND); break;
    case NO_WAY_ANIM: ZSoundEngine::PlayWav(NO_WAY_SND); break;
    case NICE_ONE_ANIM: ZSoundEngine::PlayWav(NICE_ONE_SND); break;
    case OH_YEAH_ANIM: ZSoundEngine::PlayWav(OH_YEAH_SND); break;
    case GOTCHA_ANIM: ZSoundEngine::PlayWav(GOTCHA_SND); break;
    case SMOKIN_ANIM: ZSoundEngine::PlayWav(SMOKIN_SND); break;
    case COOL_ANIM: ZSoundEngine::PlayWav(COOL_SND); break;
    case WIPE_OUT_ANIM: ZSoundEngine::PlayWav(WIPE_OUT_SND); break;
    case TERRITORY_TAKEN_ANIM: ZSoundEngine::PlayWav(TERRITORY_TAKEN_SND); break;
    case FIRE_EXTINGUISHED_ANIM: ZSoundEngine::PlayWav(FIRE_EXTINGUISHED_SND); break;
    case GUN_CAPTURED_ANIM: ZSoundEngine::PlayWav(GUN_CAPTURED_SND); break;
    case VEHICLE_CAPTURED_ANIM: ZSoundEngine::PlayWav(VEHICLE_CAPTURED_SND); break;
    case GRENADES_COLLECTED_ANIM: ZSoundEngine::PlayWav(GRENADES_COLLECTED_SND); break;
    case ENDW1_ANIM:
    case ENDW2_ANIM:
    case ENDW3_ANIM:
        next_end_anim = rand() % 6;
        while(next_end_anim == last_end_anim) next_end_anim = rand() % 6;
        last_end_anim = next_end_anim;
        ZSoundEngine::PlayWav(YEEHAA_END_SND + next_end_anim); break;
    case ENDL1_ANIM:
    case ENDL2_ANIM:
    case ENDL3_ANIM:
        next_end_anim = rand() % 7;
        while(next_end_anim == last_end_anim) next_end_anim = rand() % 7;
        last_end_anim = next_end_anim;
        ZSoundEngine::PlayWav(YOURE_CRAP_LOSE_SND + next_end_anim); break;
    }
}

void ZPortrait_Anim::AddFrame(ZPortrait_Frame &new_frame)
{
    //some shifting
    new_frame.hand_x -= 4;
    new_frame.hand_y -= 4;

    //stick in the frame
    frame_list.push_back(new_frame);

    //recalc the total duration
    total_duration = 0;
    for(vector<ZPortrait_Frame>::iterator i=frame_list.begin(); i!=frame_list.end(); ++i)
        total_duration += i->duration;
}

void ZPortrait::SetupFrames()
{
    const double duration_multi = 0.015;
    ZPortrait_Anim *cur_anim;
    ZPortrait_Frame new_frame;

    //#00 yes-sir-1-and-2
    cur_anim = &anim_info[YES_SIR_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 2;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 0;
        new_frame.hand_x = 0;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 2 + 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 33 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 62;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 2 + 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 34 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 48;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 2 + 8;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 34 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 48;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 2 + 8;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 34 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 48;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 2 + 8;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 35 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 36;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 2 + 8;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 35 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 26;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 2 + 8;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 36 - 31;
        new_frame.hand_x = 50;
        new_frame.hand_y = 12;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 2 + 8;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 2 + 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 2 + 0;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 58;
        new_frame.hand_y = 4;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);
    }

    //#01 yes-sir-3
    cur_anim = &anim_info[YES_SIR3_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 12 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);
    }

    //#02 unit-reporting-1
    cur_anim = &anim_info[UNIT_REPORTING1_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 33 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 62;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 33 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 62;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 34 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 48;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 34 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 48;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 34 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 48;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 35 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 36;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 35 - 31;
        new_frame.hand_x = 48;
        new_frame.hand_y = 26;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 36 - 31;
        new_frame.hand_x = 50;
        new_frame.hand_y = 12;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 58;
        new_frame.hand_y = 4;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 58;
        new_frame.hand_y = 4;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //17
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //18
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //19
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //20
        new_frame.duration = 9 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //21
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //22
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //23
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);
    }

    //#03 unit-reporting-2
    cur_anim = &anim_info[UNIT_REPORTING2_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 8 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 9 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //17
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);
    }

    //#04 grunts-reporting
    cur_anim = &anim_info[GRUNTS_REPORTING_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 9 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //17
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //18
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);
    }

    //#05 psychos-reporting
    cur_anim = &anim_info[PSYCHOS_REPORTING_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //17
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //18
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);
    }

    //#06 snipers-reporting
    cur_anim = &anim_info[SNIPERS_REPORTING_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 33 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 62;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 33 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 62;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 34 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 48;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 34 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 48;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 35 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 36;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 35 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 26;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 36 - 31;
        new_frame.hand_x = 50;
        new_frame.hand_y = 12;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 50;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 58;
        new_frame.hand_y = 4;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //17
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //18
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //19
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //20
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //21
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //22
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);
    }

    //#07 toughs-reporting
    cur_anim = &anim_info[TOUGHS_REPORTING_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 60;
        new_frame.hand_y = 52;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 60;
        new_frame.hand_y = 52;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 60;
        new_frame.hand_y = 52;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 34;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 34;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 24;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 24;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 22;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //17
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //18
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //19
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 58;
        new_frame.hand_y = 36;
        cur_anim->AddFrame(new_frame);

        //20
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 58;
        new_frame.hand_y = 36;
        cur_anim->AddFrame(new_frame);

        //21
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 58;
        new_frame.hand_y = 36;
        cur_anim->AddFrame(new_frame);

        //22
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 44;
        cur_anim->AddFrame(new_frame);

        //23
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#08 lazers-reporting
    cur_anim = &anim_info[LASERS_REPORTING_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //17
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //18
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#09 pyros-reporting
    cur_anim = &anim_info[PYROS_REPORTING_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //17
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //18
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#10 were-on-our-way
    cur_anim = &anim_info[WERE_ON_OUR_WAY_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 21 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 24 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 24 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 24 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 8 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#11 here-we-go
    cur_anim = &anim_info[HERE_WE_GO_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#12 youve-got-it
    cur_anim = &anim_info[YOUVE_GOT_IT_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 21 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 21 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 21 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#13 moving-in
    cur_anim = &anim_info[MOVING_IN_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 8 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //17
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#14 okay
    cur_anim = &anim_info[OKAY_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 28 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 28 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 8 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#15 alright
    cur_anim = &anim_info[ALRIGHT_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 28 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 28 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 28 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#16 no-problem
    cur_anim = &anim_info[NO_PROBLEM_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#17 over-n-out
    cur_anim = &anim_info[OVER_N_OUT_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#18 affirmative
    cur_anim = &anim_info[AFFIRMATIVE_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 33 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 62;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 33 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 62;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 34 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 48;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 34 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 48;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 34 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 48;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 35 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 36;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 35 - 31;
        new_frame.hand_x = 48;
        new_frame.hand_y = 26;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 36 - 31;
        new_frame.hand_x = 50;
        new_frame.hand_y = 12;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 36 - 31;
        new_frame.hand_x = 50;
        new_frame.hand_y = 12;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //17
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 58;
        new_frame.hand_y = 4;
        cur_anim->AddFrame(new_frame);

        //18
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 58;
        new_frame.hand_y = 4;
        cur_anim->AddFrame(new_frame);

        //19
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //20
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);
    }

    //#19 going-in
    cur_anim = &anim_info[GOING_IN_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 12 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);
    }

    //#20 lets-do-it
    cur_anim = &anim_info[LETS_DO_IT_ANIM];
    {
        //1
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);
    }

    //#21 lets-get-em
    cur_anim = &anim_info[LETS_GET_EM_ANIM];
    {
        //1
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 9 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);
    }

    //#22 we're-under-attack
    cur_anim = &anim_info[WERE_UNDER_ATTACK_ANIM];
    {
        //1
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 22 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 22 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 22 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 22 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 22 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 22 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);
    }

    //#23 i-said-we're-under-attack
    cur_anim = &anim_info[I_SAID_WERE_UNDER_ATTACK_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 78;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 78;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 62;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 46;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 46;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 46;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 46;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 46;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 46;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 46;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 46;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 46;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 62;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 62;
        cur_anim->AddFrame(new_frame);

        //17
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 62;
        cur_anim->AddFrame(new_frame);

        //18
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 78;
        cur_anim->AddFrame(new_frame);

        //19
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 78;
        cur_anim->AddFrame(new_frame);

        //20
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 78;
        cur_anim->AddFrame(new_frame);

        //21
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //22
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //23
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //24
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //25
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //26
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //27
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //28
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //29
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //30
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //31
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //32
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //33
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //34
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#24 help-help
    cur_anim = &anim_info[HELP_HELP_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 78;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 78;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 62;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 46;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 46;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 46;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 46;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 46;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 46;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 46;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 46;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 62;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 62;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 78;
        cur_anim->AddFrame(new_frame);

        //17
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 78;
        cur_anim->AddFrame(new_frame);

        //18
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 78;
        cur_anim->AddFrame(new_frame);

        //19
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //20
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //21
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //22
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //23
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //24
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //25
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#25 theyer-all-over-us
    cur_anim = &anim_info[THEYRE_ALL_OVER_US_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //17
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //18
        new_frame.duration = 9 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#26 wereloseing-it
    cur_anim = &anim_info[WERE_LOSEING_IT_ANIM];
    {
        //1
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#27 wereloseing-it
    cur_anim = &anim_info[AAAHHH_ANIM];
    {
        //1
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 8 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 50 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#28 for-chris-sake
    cur_anim = &anim_info[FOR_CHRIST_SAKE_ANIM];
    {
        //1
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 8 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 8 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //17
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //18
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //19
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //20
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //21
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //22
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //23
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //24
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //25
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //26
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //27
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //28
        new_frame.duration = 9 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#29 you're-joking
    cur_anim = &anim_info[YOURE_JOKING_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 9 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#30 target-destroyed
    cur_anim = &anim_info[TARGET_DESTROYED_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 11 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //17
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //18
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#31 blink
    cur_anim = &anim_info[BLINK_ANIM];
    {
        //1
        new_frame.duration = 50 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#32 wink
    cur_anim = &anim_info[WINK_ANIM];
    {
        //1
        new_frame.duration = 50 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 9 - 4;
        new_frame.eyes = 28 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 9 - 4;
        new_frame.eyes = 28 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 18 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 9 - 4;
        new_frame.eyes = 28 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 9 - 4;
        new_frame.eyes = 28 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 9 - 4;
        new_frame.eyes = 28 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#33 surprise
    cur_anim = &anim_info[SURPRISE_ANIM];
    {
        //1
        new_frame.duration = 50 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 8 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 8 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 28 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 8 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 8 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 8 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#34 anger
    cur_anim = &anim_info[ANGER_ANIM];
    {
        //1
        new_frame.duration = 50 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 22 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 22 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 28 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 22 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 22 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 22 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#35 grin
    cur_anim = &anim_info[GRIN_ANIM];
    {
        //1
        new_frame.duration = 50 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 28 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#36 scared
    cur_anim = &anim_info[SCARED_ANIM];
    {
        //1
        new_frame.duration = 50 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 27 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 27 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 28 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 27 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 27 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 27 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#37 eyes-left
    cur_anim = &anim_info[EYES_LEFT_ANIM];
    {
        //1
        new_frame.duration = 50 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 60 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 21 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#38 eyes-right
    cur_anim = &anim_info[EYES_RIGHT_ANIM];
    {
        //1
        new_frame.duration = 50 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 60 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 24 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#39 eyes-up
    cur_anim = &anim_info[EYES_UP_ANIM];
    {
        //1
        new_frame.duration = 50 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 60 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 29 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#40 eyes-down
    cur_anim = &anim_info[EYES_DOWN_ANIM];
    {
        //1
        new_frame.duration = 50 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 60 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 26 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#41 whistle
    cur_anim = &anim_info[WHISTLE_ANIM];
    {
        //1
        new_frame.duration = 50 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 20 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 20 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#42 look-left
    cur_anim = &anim_info[LOOK_LEFT_ANIM];
    {
        //1
        new_frame.duration = 50 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 50 * duration_multi;
        new_frame.look_direction = LOOK_LEFT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#43 look-right
    cur_anim = &anim_info[LOOK_RIGHT_ANIM];
    {
        //1
        new_frame.duration = 50 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 50 * duration_multi;
        new_frame.look_direction = LOOK_RIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 31 - 31;
        new_frame.hand_x = 42;
        new_frame.hand_y = 94;
        cur_anim->AddFrame(new_frame);
    }

    //#44 salute
    cur_anim = &anim_info[SALUTE_ANIM];
    {
        //1
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 33 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 62;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 34 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 48;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 35 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 36;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 35 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 26;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 36 - 31;
        new_frame.hand_x = 50;
        new_frame.hand_y = 12;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 25 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 58;
        new_frame.hand_y = 4;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);
    }

    //#45 thumbsup
    cur_anim = &anim_info[THUMBS_UP_ANIM];
    {
        //1
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 60;
        new_frame.hand_y = 52;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 34;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 24;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 22;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 50 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 58;
        new_frame.hand_y = 36;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 44;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#46 yes-sir-salute
    cur_anim = &anim_info[YES_SIR_SALUTE_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 33 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 62;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 33 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 62;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 34 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 48;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 34 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 48;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 34 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 48;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 35 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 36;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 35 - 31;
        new_frame.hand_x = 48;
        new_frame.hand_y = 26;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 36 - 31;
        new_frame.hand_x = 50;
        new_frame.hand_y = 12;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 8 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 58;
        new_frame.hand_y = 4;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 58;
        new_frame.hand_y = 4;
        cur_anim->AddFrame(new_frame);

        //17
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);
    }

    //#47 goingin-thumbsup
    cur_anim = &anim_info[GOING_IN_THUMBS_UP_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 8 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 60;
        new_frame.hand_y = 52;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 60;
        new_frame.hand_y = 52;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 60;
        new_frame.hand_y = 52;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 34;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 34;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 8 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 24;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 22;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 22;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 22;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //17
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //18
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //19
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //20
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //21
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 58;
        new_frame.hand_y = 36;
        cur_anim->AddFrame(new_frame);

        //22
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 44;
        cur_anim->AddFrame(new_frame);

        //23
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 44;
        cur_anim->AddFrame(new_frame);

        //24
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //25
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //26
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#48 forget-it
    cur_anim = &anim_info[FORGET_IT_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#49 get-outta-here
    cur_anim = &anim_info[GET_OUTTA_HERE_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 11 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 16 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#50 good-hit
    cur_anim = &anim_info[GOOD_HIT_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 8 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 8 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#51 no-way
    cur_anim = &anim_info[NO_WAY_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 15 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 8 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 30 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#52 nice-one
    cur_anim = &anim_info[NICE_ONE_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 60;
        new_frame.hand_y = 52;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 60;
        new_frame.hand_y = 52;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 34;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 34;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 34;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 24;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 22;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 8 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 58;
        new_frame.hand_y = 36;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 44;
        cur_anim->AddFrame(new_frame);

        //17
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //18
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //19
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //20
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //21
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#53 oh-yeah
    cur_anim = &anim_info[OH_YEAH_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 14 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 15 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#54 gotche
    cur_anim = &anim_info[GOTCHA_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 10 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 12 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#55 smookin
    cur_anim = &anim_info[SMOKIN_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 12 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 18 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 24 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 9 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#56 cool
    cur_anim = &anim_info[COOL_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 20 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 8 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#57 wipe-out
    cur_anim = &anim_info[WIPE_OUT_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 60;
        new_frame.hand_y = 52;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 60;
        new_frame.hand_y = 52;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 60;
        new_frame.hand_y = 52;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 34;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 34;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 24;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 22;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 11 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 18 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 54;
        new_frame.hand_y = 28;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 39 - 31;
        new_frame.hand_x = 58;
        new_frame.hand_y = 36;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 62;
        new_frame.hand_y = 44;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#58 territory-taken
    cur_anim = &anim_info[TERRITORY_TAKEN_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 8 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 11 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 25 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#59 fire-extinguished
    cur_anim = &anim_info[FIRE_EXTINGUISHED_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 8 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#60 gun-captueed
    cur_anim = &anim_info[GUN_CAPTURED_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 23 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);

        //17
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 38 - 31;
        new_frame.hand_x = 68;
        new_frame.hand_y = 58;
        cur_anim->AddFrame(new_frame);
    }

    //#61 vehicle-captueed
    cur_anim = &anim_info[VEHICLE_CAPTURED_ANIM];
    {
        //1
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 33 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 62;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 33 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 62;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 34 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 48;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 34 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 48;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 34 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 48;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 35 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 36;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 35 - 31;
        new_frame.hand_x = 46;
        new_frame.hand_y = 26;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 6 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 36 - 31;
        new_frame.hand_x = 50;
        new_frame.hand_y = 12;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //15
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 52;
        new_frame.hand_y = 0;
        cur_anim->AddFrame(new_frame);

        //16
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 58;
        new_frame.hand_y = 4;
        cur_anim->AddFrame(new_frame);

        //17
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //18
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = true;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //19
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //20
        new_frame.duration = 3 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //21
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //22
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 14 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //23
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //24
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 10 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //25
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //26
        new_frame.duration = 1 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);
    }

    //#62 grenades-collected
    cur_anim = &anim_info[GRENADES_COLLECTED_ANIM];
    {
        //1
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //2
        new_frame.duration = 6 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 10 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //3
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //4
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 13 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //5
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //6
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //7
        new_frame.duration = 2 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 4 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //8
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //9
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 16 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //10
        new_frame.duration = 7 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //11
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 5 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //12
        new_frame.duration = 5 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 11 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //13
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 12 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);

        //14
        new_frame.duration = 4 * duration_multi;
        new_frame.look_direction = LOOK_STRAIGHT;
        new_frame.head_y = 6 - 4;
        new_frame.mouth = 7 - 4;
        new_frame.eyes = 20 - 20;
        new_frame.hand_do_render = false;
        new_frame.hand = 37 - 31;
        new_frame.hand_x = 66;
        new_frame.hand_y = 14;
        cur_anim->AddFrame(new_frame);
    }

    //homemade end anim
    anim_info[ENDW1_ANIM] = anim_info[LETS_GET_EM_ANIM];
    anim_info[ENDW2_ANIM] = anim_info[GUN_CAPTURED_ANIM];
    anim_info[ENDW3_ANIM] = anim_info[GOING_IN_ANIM];

    anim_info[ENDL1_ANIM] = anim_info[ENDW1_ANIM];
    anim_info[ENDL2_ANIM] = anim_info[ENDW2_ANIM];
    anim_info[ENDL3_ANIM] = anim_info[ENDW3_ANIM];

    ////
    //cur_anim = &anim_info[xxxxxxxxxxxxxxxxxxxxxxxxxxxx];
    //{
    //}

    //for(int i=0;i<MAX_PORTRAIT_ANIMS;i++)
    //	printf("SetupFrames::%d:Duration:%f \tName:%s\n", i, anim_info[i].total_duration, portrait_anim_string[i].c_str());
}

int ZPortrait::GetBlitInfo(SDL_Surface *src, int x, int y, SDL_Rect &from_rect, SDL_Rect &to_rect)
{
    if(!src) return 0;

    int view_w = 86;
    int view_h = 74;
    int shift_x = 0;
    int shift_y = 0;

    //is this visable at ??
    if(x > shift_x + view_w) return 0;
    if(y > shift_y + view_h) return 0;
    if(x + src->w < shift_x) return 0;
    if(y + src->h < shift_y) return 0;

    //setup to
    to_rect.x = x - shift_x;
    to_rect.y = y - shift_y;
    to_rect.w = 0;
    to_rect.h = 0;

    //setup from
    from_rect.x = shift_x - x;
    from_rect.y = shift_y - y;

    if(to_rect.x + src->w > view_w)
        from_rect.w = view_w - to_rect.x;
    else
        from_rect.w = to_rect.x - view_w;

    if(to_rect.y + src->h > view_h)
        from_rect.h = view_h - to_rect.y;
    else
        from_rect.h = to_rect.y - view_h;

    if(from_rect.x < 0) from_rect.x = 0;
    if(from_rect.y < 0) from_rect.y = 0;

    to_rect.x += from_rect.x;
    to_rect.y += from_rect.y;

    return 1;
}
















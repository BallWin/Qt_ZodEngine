#ifndef QZOD_MAP_H
#define QZOD_MAP_H

#include "qzod_dnmap_global.h"
#include <lib_qZod_DnSeparate/constants.h>
#include <lib_qZod_DnSeparate/zsdl.h>
#include <lib_qZod_DnSeparate/zsdl_opengl.h>
#include <lib_qZod_DnSeparate/common.h>

//#include "constants.h"
//#include "zsdl.h"
//#include "zsdl_opengl.h"
//#include "common.h"

#include "finding/zpath_finding_old.h"
#include "zteam.h"
#include "zmap_crater_graphics.h"
#include "zmap_structures_old.h"

using namespace COMMON;

class QZOD_DNMAPSHARED_EXPORT ZMap
{
    public:
        ZMap();
        ~ZMap();

        bool isEmpty(){
            return map_data==nullptr || !(map_data_size>0);
        }


        int Read(const char* filename);
        int Read(char* data, int size, bool scrap_data = true);
        int Write(const char* filename);
        bool GetMapData(char *&data, int &size);
        void RenderMap();
        void ClearMap();
        void MakeRandomMap();
        void MakeNewMap(const char *new_name, planet_type palette, int width, int height);
        ZSDL_Surface &GetRender();
        void ReplaceUnusableTiles();
        void DoRender(SDL_Surface *dest, int shift_x = 0, int shift_y = 0);
        void DebugMapInfo();
        int DoEffects(double the_time, SDL_Surface *dest, int shift_x = 0, int shift_y = 0);
        map_basics &GetMapBasics();
        map_tile &GetTile(int x, int y, bool is_shifted = false);
        map_tile &GetTile(unsigned int index);
        int GetTileIndex(int x, int y, bool is_shifted = false);
        void GetTile(unsigned int index, int &x, int &y, bool is_shifted = false);
        bool CoordIsRoad(int x, int y);
        double GetTileWalkSpeed(int x, int y, bool is_shifted = false);
        void DoZoneEffects(double the_time, SDL_Surface *dest, int shift_x = 0, int shift_y = 0);
        void SetupAllZoneInfo();
        map_zone_info *GetZone(int x, int y);
        vector<map_zone_info> &GetZoneInfoList();
        //void PlayMusic(bool normal = true);
        bool Loaded();
        void MarkAreaStamped(int x, int y, int w, int h);
        bool PermStamp(int x_, int y_, ZSDL_Surface *surface, bool mark_stamped = true);
        bool PermStamp(int x_, int y_, SDL_Surface *surface, bool mark_stamped = true);
        vector<map_effect_info> &GetMapWaterList() { return map_water_list; }
        void CreateCrater(int x, int y, bool is_big, double chance = 1.0);
        int CoordCraterType(int tx, int ty);


        //viewport / shift stuff
        void SetViewingDimensions(int w, int h);
        void SetViewShift(int x, int y);
        bool ShiftViewRight();
        bool ShiftViewUp();
        bool ShiftViewDown();
        bool ShiftViewLeft();
        bool ShiftViewRight(int amt);
        bool ShiftViewUp(int amt);
        bool ShiftViewDown(int amt);
        bool ShiftViewLeft(int amt);
        void GetViewShift(int &x, int &y);
        void GetViewShiftFull(int &x, int &y, int &view_w, int &view_h);
        void GetViewLimits(int &map_left, int &map_right, int &map_top, int &map_bottom);
        void GetMapCoords(int mouse_x, int mouse_y, int &map_x, int &map_y);
        int GetBlitInfo(SDL_Surface *src, int x, int y, SDL_Rect &from_rect, SDL_Rect &to_rect);
        int GetBlitInfo(int x, int y, int w, int h, SDL_Rect &from_rect, SDL_Rect &to_rect);
        int WithinView(int x, int y, int w, int h);
        void RenderZSurface(ZSDL_Surface *surface, int x, int y, bool render_hit = false, bool about_center = false);
        void RenderZSurfaceHorzRepeat(ZSDL_Surface *surface, int x, int y, int w_total, bool render_hit = false);
        void RenderZSurfaceVertRepeat(ZSDL_Surface *surface, int x, int y, int h_total, bool render_hit = false);

        //map editor stuff
        void ChangeTile(unsigned int index, map_tile new_tile);
        void PlaceObject(map_object new_object);
        vector<map_object> &GetObjectList();
        vector<map_zone> &GetZoneList();
        static ZSDL_Surface *GetZoneMarkers();
        int AddZone(map_zone new_zone);
        int RemoveZone(int x, int y);
        map_zone *GetZoneExact(int x, int y);

        static void Init();
        static void ServerInit();
        static SDL_mutex *init_mutex;

        //palette stuff
        static ZSDL_Surface &GetMapPalette(planet_type palette);
        static palette_tile_info &GetMapPaletteTileInfo(planet_type palette, int tile);
        static int WriteMapPaletteTileInfo(planet_type palette);
        static int UpdatePalettesTileFormat();
        static int GetPaletteTile(int x, int y);
        static int GetPaletteTile(unsigned short index, int &x, int &y);

        //pathfinding stuff
        void InitPathfinding();
        void DeletePathfindingInfo();
        inline void SetImpassable(int x, int y, bool impassable = true, bool destroyable = false)
            { path_finder.SetImpassable(x, y, impassable, destroyable); }
        inline bool WithinImpassable(int x, int y, int w, int h, int &stop_x, int &stop_y, bool is_robot)
            { return path_finder.WithinImpassable(x, y, w, h, stop_x, stop_y, is_robot); }
        inline ZPath_Finding_Engine &GetPathFinder()
            { return path_finder; }
        inline void RebuildRegions() { path_finder.RebuildRegions(); }


        bool EngageBarrierBetweenCoords(int x1, int y1, int x2, int y2);

        //submerge amounts
        void InitSubmergeAmounts();
        void DeleteSubmergeAmounts();
        int SubmergeAmount(int x, int y);

        //rock list
        void InitRockList();
        void DeleteRockList();
        bool **GetRockList();

        //stamp list
        void InitStampList();
        void DeleteStampList();
        void MakeSureStampListExists();
        bool CoordStamped(int x, int y);

    private:
        void DeRenderMap();
        void FreeMapData();
        bool CheckLoad();
        void InitEffects();
        void RenderTile(unsigned int index);
        double ShiftViewDifference();
        static void LoadPaletteInfo(int terrain_type);

        bool file_loaded;
        char *map_data;
        int map_data_size;


        //the basics that exist in the file
        map_basics basic_info;
        vector<map_zone> zone_list;
        vector<map_object> object_list;
        vector<map_tile> tile_list;

        //for speed increases
        int width_pix;
        int height_pix;

        //pathfinding stuff
        ZPath_Finding_Engine path_finder;

        //submerge info
        bool submerge_info_setup;
        int **submerge_amount;

        //rock list
        bool rock_list_setup;
        bool **rock_list;

        //stamped list
        bool stamp_list_setup;
        bool **stamp_list;
        unsigned short stamp_list_w;
        unsigned short stamp_list_h;

        ZSDL_Surface full_render;
        static ZSDL_Surface planet_template[MAX_PLANET_TYPES];
        static palette_tile_info planet_tile_info[MAX_PLANET_TYPES][MAX_PLANET_TILES];
        static ZSDL_Surface zone_marker[MAX_TEAM_TYPES];
        static ZSDL_Surface zone_marker_water[MAX_TEAM_TYPES];

        //music
        //static Mix_Music *music[MAX_PLANET_TYPES];

        //effect stuff
        vector<map_effect_info> map_effect_list;
        vector<map_effect_info> map_water_list;
        static vector<unsigned int> map_water_plist[MAX_PLANET_TYPES];
        static vector<unsigned int> map_water_effect_plist[MAX_PLANET_TYPES];

        //other runtime
        vector<map_zone_info> zone_list_info;

        //shift stuff
        int shift_x, shift_y;
        int view_w, view_h;
        double last_shift_time;
        double shift_overflow;

        //screen placement
        int place_x, place_y;
};

#endif // QZOD_MAP_H

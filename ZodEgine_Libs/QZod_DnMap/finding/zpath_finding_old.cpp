// =========================================================
// *********************************************************
// =========================================================
#include "zpath_finding_old.h"

int ZPath_Finding_Response::existing_responses = 0;
// =========================================================
// *********************************************************
// =========================================================
ZPath_Finding_Response::ZPath_Finding_Response()
{
    path_finder = nullptr;
    thread_id = 0;
    obj_ref_id = -1;
    tile_info = nullptr;
    w = h = 0;
    start_x = start_y = 0;
    end_x = end_y = 0;
    kill_thread = false;

    existing_responses++;
}

ZPath_Finding_Response::~ZPath_Finding_Response()
{
    /*
    if(tile_info)
    {
        for(int i=0;i<w;i++)
            free(tile_info[i]);
        free(tile_info);
    }
    */

    existing_responses--;
}
// =========================================================
// *********************************************************
// =========================================================
ZPath_Finding_RegionInfo::ZPath_Finding_RegionInfo()
{
    allocated = false;
    w = h = 0;
    robot_region = nullptr;
    vehicle_region = nullptr;
    ffQ = nullptr;
}

void ZPath_Finding_RegionInfo::Init(int w_, int h_)
{
    int i, j;

    //first make sure its deleted
    Delete();

    //set our w,h
    w = w_;
    h = h_;

    //alloc ffQ
    ffQ = (ffnode *)malloc(w * h * sizeof(ffnode));
    //printf("ffq nodes:%d\n", w * h);

    //alloc regions
    robot_region = (int **)malloc(w * sizeof(int *));
    vehicle_region = (int **)malloc(w * sizeof(int *));
    for(i=0;i<w;i++)
    {
        robot_region[i] = (int *)malloc(h * sizeof(int));
        vehicle_region[i] = (int *)malloc(h * sizeof(int));

        for(j=0;j<h;j++)
        {
            robot_region[i][j] = 0;
            vehicle_region[i][j] = 0;
        }
    }

    allocated = true;
}

void ZPath_Finding_RegionInfo::Delete()
{
    if(!allocated) return;

    //free ffQ
    free(ffQ);

    //free regions
    for(int i=0;i<w;i++)
    {
        free(robot_region[i]);
        free(vehicle_region[i]);
    }
    free(robot_region);
    free(vehicle_region);

    ffQ = nullptr;
    robot_region = nullptr;
    vehicle_region = nullptr;

    allocated = false;
}

void ZPath_Finding_RegionInfo::FloodFill_Recursive(int **region, int x, int y, int cur_region)
{
    //bounds?
    if(x < 0) return;
    if(y < 0) return;
    if(x >= w) return;
    if(y >= h) return;

    //this can be added?
    if(region[x][y] != -1) return;

    //add it
    region[x][y] = cur_region;

    //fill some more
    FloodFill_Recursive(region, x-1, y, cur_region); //left
    FloodFill_Recursive(region, x+1, y, cur_region); //right
    FloodFill_Recursive(region, x, y-1, cur_region); //up
    FloodFill_Recursive(region, x, y+1, cur_region); //down
}

void ZPath_Finding_RegionInfo::FloodFill_AddQ(int **region, int &i, int x, int y, int cur_region)
{
    if(x < 0) return;
    if(y < 0) return;
    if(x >= w) return;
    if(y >= h) return;

    //not what we want?
    if(region[x][y] != -1) return;

    //add it
    region[x][y] = cur_region;

    ffQ[i].x = x;
    ffQ[i].y = y;

    i++;
}

void ZPath_Finding_RegionInfo::FloodFill(int **region, int x, int y, int cur_region)
{
    int i;
    int ffQ_size;

    //clear ffQ
    //memset(ffQ, 0, w * h * sizeof(ffnode));

    ffQ_size = 0;
    FloodFill_AddQ(region, ffQ_size, x, y, cur_region);

    for(i=0;i<ffQ_size;i++)
    {
        ffnode n = ffQ[i];

        //push some more on!
        FloodFill_AddQ(region, ffQ_size, n.x-1, n.y, cur_region); //left
        FloodFill_AddQ(region, ffQ_size, n.x+1, n.y, cur_region); //right
        FloodFill_AddQ(region, ffQ_size, n.x, n.y-1, cur_region); //up
        FloodFill_AddQ(region, ffQ_size, n.x, n.y+1, cur_region); //down
    }

    //printf("Region:%d Size:%d\n", cur_region, ffQ_size);
}

void ZPath_Finding_RegionInfo::BuildRegions(map_pathfinding_info_tile **tile_info, int **region)
{
    int x, y;
    int cur_region;

    if(!allocated)
    {
        printf("ZPath_Finding_RegionInfo::BuildRegions: not allocated\n");
        return;
    }

    //clear out the regions
    for(x=0;x<w;x++)
        for(y=0;y<h;y++)
        {
            if(tile_info[x][y].passable)
                region[x][y] = -1;
            else
                region[x][y] = -2;
        }

    //first region num
    cur_region = 0;

    //fill the regions
    for(x=0;x<w;x++)
        for(y=0;y<h;y++)
            if(region[x][y] == -1)
                FloodFill(region, x, y, cur_region++);
}

bool ZPath_Finding_RegionInfo::InSameRegion(int sx, int sy, int ex, int ey, bool is_robot)
{
    if(!allocated)
    {
        printf("ZPath_Finding_RegionInfo::InSameRegion: not allocated\n");
        return true;
    }

    //divide all by 16
    sx >>= 4;
    sy >>= 4;
    ex >>= 4;
    ey >>= 4;

    if(sx < 0) return true;
    if(sy < 0) return true;
    if(ex < 0) return true;
    if(ey < 0) return true;
    if(sx >= w) return true;
    if(sy >= h) return true;
    if(ex >= w) return true;
    if(ey >= h) return true;

    if(is_robot)
    {
        //printf("R %d vs %d\n", robot_region[sx][sy], robot_region[ex][ey]);
        return robot_region[sx][sy] == robot_region[ex][ey];
    }
    else
        return vehicle_region[sx][sy] == vehicle_region[ex][ey];
}


// =========================================================
// *********************************************************
// =========================================================
ZPath_Finding_Engine::ZPath_Finding_Engine()
{
    list_mutex = SDL_CreateMutex();
    path_robot_tile = nullptr;
    path_vehicle_tile = nullptr;
    path_robot_norocks_tile = nullptr;
    path_vehicle_norocks_tile = nullptr;
    next_thread_id = 0;
}

ZPath_Finding_Engine::~ZPath_Finding_Engine()
{
    SDL_LockMutex(list_mutex);
    Clear_Response_List();
    SDL_UnlockMutex(list_mutex);

    SDL_DestroyMutex(list_mutex);
    list_mutex = nullptr;

    DeleteAllTileInfo();
}

// =========================================================
void ZPath_Finding_Engine::Push_Response(ZPath_Finding_Response *response)
{
    if(!response) return;

    SDL_LockMutex(list_mutex);
    respone_list.push_back(response);
    SDL_UnlockMutex(list_mutex);
}

void ZPath_Finding_Engine::DeleteTileInfo(map_pathfinding_info_tile ***the_list)
{
    map_pathfinding_info_tile **delete_list = *the_list;

    if(delete_list)
    {
        for(int i=0;i<w;i++)
            free(delete_list[i]);
        free(delete_list);
    }

    *the_list = nullptr;
}

void ZPath_Finding_Engine::DeleteAllTileInfo()
{
    //we must first stop all path finding threads
    Clear_Thread_List();

    DeleteTileInfo(&path_robot_tile);
    DeleteTileInfo(&path_robot_norocks_tile);
    DeleteTileInfo(&path_vehicle_tile);
    DeleteTileInfo(&path_vehicle_norocks_tile);
}

void ZPath_Finding_Engine::AllocAllTileInfo()
{
    int i, j;

    //alloc
    path_vehicle_tile = (map_pathfinding_info_tile **)malloc(w * sizeof(map_pathfinding_info_tile *));
    path_robot_tile = (map_pathfinding_info_tile **)malloc(w * sizeof(map_pathfinding_info_tile *));
    path_robot_norocks_tile = (map_pathfinding_info_tile **)malloc(w * sizeof(map_pathfinding_info_tile *));
    path_vehicle_norocks_tile = (map_pathfinding_info_tile **)malloc(w * sizeof(map_pathfinding_info_tile *));
    for(i=0;i<w;i++)
    {
        path_vehicle_tile[i] = (map_pathfinding_info_tile *)malloc(h * sizeof(map_pathfinding_info_tile));
        path_robot_tile[i] = (map_pathfinding_info_tile *)malloc(h * sizeof(map_pathfinding_info_tile));
        path_vehicle_norocks_tile[i] = (map_pathfinding_info_tile *)malloc(h * sizeof(map_pathfinding_info_tile));
        path_robot_norocks_tile[i] = (map_pathfinding_info_tile *)malloc(h * sizeof(map_pathfinding_info_tile));

        for(j=0;j<h;j++)
        {
            memset((void*)&path_vehicle_tile[i][j], 0, sizeof(map_pathfinding_info_tile));
            memset((void*)&path_robot_tile[i][j], 0, sizeof(map_pathfinding_info_tile));
            memset((void*)&path_vehicle_norocks_tile[i][j], 0, sizeof(map_pathfinding_info_tile));
            memset((void*)&path_robot_norocks_tile[i][j], 0, sizeof(map_pathfinding_info_tile));
        }
    }
}

void ZPath_Finding_Engine::ResetTileInfo(int w_, int h_)
{
    DeleteAllTileInfo();

    w = w_;
    h = h_;
    width_pix = w * 16;
    height_pix = h * 16;

    AllocAllTileInfo();

    if(!path_robot_tile) printf("ResetTileInfo::path_robot_tile not setup\n");
    if(!path_vehicle_tile) printf("ResetTileInfo::path_vehicle_tile not setup\n");
    if(!path_robot_norocks_tile) printf("ResetTileInfo::path_robot_norocks_tile not setup\n");
    if(!path_vehicle_norocks_tile) printf("ResetTileInfo::path_vehicle_norocks_tile not setup\n");

    //do region too
    region_info.Init(w, h);
}

void ZPath_Finding_Engine::SetTileInfo(int x, int y, int tile_type)
{
    float tile_speed;

    if(x < 0) return;
    if(y < 0) return;
    if(x >= w) return;
    if(y >= h) return;
    if(!path_robot_tile) return;
    if(!path_vehicle_tile) return;
    if(!path_robot_norocks_tile) return;
    if(!path_vehicle_norocks_tile) return;

    switch(tile_type)
    {
    case PF_NORMAL:
        tile_speed = 1.0;
        path_robot_tile[x][y].passable = true;
        path_vehicle_tile[x][y].passable = true;
        break;
    case PF_IMPASSABLE:
        tile_speed = 1.0;
        path_robot_tile[x][y].passable = false;
        path_vehicle_tile[x][y].passable = false;
        break;
    case PF_WATER:
        tile_speed = 1.0 / WATER_SPEED;
        path_robot_tile[x][y].passable = true;
        path_vehicle_tile[x][y].passable = false;
        break;
    case PF_ROAD:
        tile_speed = 1.0 / (ROAD_SPEED + 0.5);
        path_robot_tile[x][y].passable = true;
        path_vehicle_tile[x][y].passable = true;
        break;
    default:
        printf("ZPath_Finding_Engine::SetTileInfo:error invalid tile_type:%d\n", tile_type);
        break;
    }

    path_robot_tile[x][y].side_weight = 100 * tile_speed;
    path_robot_tile[x][y].diag_weight = (1.414 * 100) * tile_speed;

    path_vehicle_tile[x][y].side_weight = path_robot_tile[x][y].side_weight;
    path_vehicle_tile[x][y].diag_weight = path_robot_tile[x][y].diag_weight;

    path_robot_norocks_tile[x][y].side_weight = path_robot_tile[x][y].side_weight;
    path_robot_norocks_tile[x][y].diag_weight = path_robot_tile[x][y].diag_weight;

    path_vehicle_norocks_tile[x][y].side_weight = path_robot_tile[x][y].side_weight;
    path_vehicle_norocks_tile[x][y].diag_weight = path_robot_tile[x][y].diag_weight;

    path_robot_norocks_tile[x][y].passable = path_robot_tile[x][y].passable;
    path_vehicle_norocks_tile[x][y].passable = path_vehicle_tile[x][y].passable;
}

void ZPath_Finding_Engine::SetTileWideWeights()
{
    int i,j;
    int **wide_side_weight;
    int **wide_diag_weight;

    if(w <= 0 || h <= 0 || !path_vehicle_tile || !path_vehicle_norocks_tile)
    {
        printf("SetTileWideWeights::not setup");
        return;
    }

    //alloc
    wide_side_weight = (int**)malloc(w * sizeof(int*));
    wide_diag_weight = (int**)malloc(w * sizeof(int*));
    for(i=0;i<w;i++)
    {
        wide_side_weight[i] = (int*)malloc(h * sizeof(int));
        wide_diag_weight[i] = (int*)malloc(h * sizeof(int));
    }

    //fill wide_weight table
    for(i=0;i<w;i++)
        for(j=0;j<h;j++)
        {
            int the_side_weight;
            int the_diag_weight;

            the_side_weight = path_vehicle_tile[i][j].side_weight;
            the_diag_weight = path_vehicle_tile[i][j].diag_weight;

            if(i+1<w)
            {
                the_side_weight += path_vehicle_tile[i+1][j].side_weight;
                the_diag_weight += path_vehicle_tile[i+1][j].diag_weight;
            }

            if(j+1<h)
            {
                the_side_weight += path_vehicle_tile[i][j+1].side_weight;
                the_diag_weight += path_vehicle_tile[i][j+1].diag_weight;
            }

            if(i+1<w && j+1<h)
            {
                the_side_weight += path_vehicle_tile[i+1][j+1].side_weight;
                the_diag_weight += path_vehicle_tile[i+1][j+1].diag_weight;
            }

            wide_side_weight[i][j] = the_side_weight;
            wide_diag_weight[i][j] = the_diag_weight;
        }

    //move wide_weight table over to the vehicle's weight tables
    for(i=0;i<w;i++)
        for(j=0;j<h;j++)
        {
            path_vehicle_tile[i][j].side_weight = wide_side_weight[i][j];
            path_vehicle_tile[i][j].diag_weight = wide_diag_weight[i][j];

            path_vehicle_norocks_tile[i][j].side_weight = path_vehicle_tile[i][j].side_weight;
            path_vehicle_norocks_tile[i][j].diag_weight = path_vehicle_tile[i][j].diag_weight;
        }

    //dealloc
    for(i=0;i<w;i++)
    {
        free(wide_side_weight[i]);
        free(wide_diag_weight[i]);
    }
    free(wide_side_weight);
    free(wide_diag_weight);

}

bool ZPath_Finding_Engine::TileOnMap(int tx, int ty)
{
    if(tx < 0) return false;
    if(ty < 0) return false;
    if(tx >= w) return false;
    if(ty >= h) return false;

    return true;
}

bool ZPath_Finding_Engine::TilePassable(int tx, int ty, bool is_robot)
{
    if(!TileOnMap(tx, ty)) return false;

    if(is_robot)
        return path_robot_tile[tx][ty].passable;
    else
        return path_vehicle_tile[tx][ty].passable;
}

bool ZPath_Finding_Engine::ShouldBeAbleToMoveTo(int sx, int sy, int ex, int ey, bool is_robot)
{
    //same region?
    if(!InSameRegion(sx, sy, ex, ey, is_robot)) return false;

    //end target ok?
    {
        int tx, ty;
        int cx, cy;

        tx = ex >> 4;
        ty = ey >> 4;

        if(is_robot)
        {
            if(!TilePassable(tx, ty, is_robot)) return false;
        }
        else
        {
            if(!TilePassable(tx, ty, is_robot)) return false; //top left
            if(!TilePassable(tx+1, ty, is_robot)) return false; //top right
            if(!TilePassable(tx, ty+1, is_robot)) return false; //bottom left
            if(!TilePassable(tx+1, ty+1, is_robot)) return false; //bottom right
        }
    }

    return true;
}

bool ZPath_Finding_Engine::InSameRegion(int sx, int sy, int ex, int ey, bool is_robot)
{
    return region_info.InSameRegion(sx, sy, ex, ey, is_robot);
}

void ZPath_Finding_Engine::RebuildRegions()
{
    region_info.BuildRegions(path_robot_tile, region_info.robot_region);
    region_info.BuildRegions(path_vehicle_tile, region_info.vehicle_region);
}

void ZPath_Finding_Engine::SetImpassable(int x, int y, bool impassable, bool destroyable)
{
    if(x < 0) return;
    if(y < 0) return;
    if(x >= w) return;
    if(y >= h) return;
    if(!path_robot_tile) return;
    if(!path_vehicle_tile) return;

    path_robot_tile[x][y].passable = !impassable;
    path_vehicle_tile[x][y].passable = !impassable;

    path_robot_norocks_tile[x][y].passable = !impassable || destroyable;
    path_vehicle_norocks_tile[x][y].passable = !impassable || destroyable;

    //slow place for it
    //would be best placed after the point in which this is called
    //as this function is normally called in blocks
    //RebuildRegions();
}

bool ZPath_Finding_Engine::WithinImpassable(int x, int y, int w, int h, int &stop_x, int &stop_y, bool is_robot)
{
    map_pathfinding_info_tile **tile_check;
    int tx, ty, tex, tey, tw, th;
    int i, j;
    int i_pix, j_pix;

    //
    if(!path_robot_tile) return false;
    if(!path_vehicle_tile) return false;

    //simple map restrictions
    if(x < 0) return true;
    if(y < 0) return true;
    if(x + w >= width_pix) return true;
    if(y + h >= height_pix) return true;

    if(is_robot)
        tile_check = path_robot_tile;
    else
        tile_check = path_vehicle_tile;

    //set stuff
    //tx = x / 16;
    //ty = y / 16;
    //tw = w / 16;
    //th = h / 16;
    tx = x >> 4;
    ty = y >> 4;
    tw = w >> 4;
    th = h >> 4;

    if(w % 16) tw += 1;
    if(h % 16) th += 1;

    tex = tx + tw;
    tey = ty + th;

    //int max_tex = (width_pix / 16) - 1;
    //int max_tey = (height_pix / 16) - 1;
    int max_tex = (width_pix >> 4) - 1;
    int max_tey = (height_pix >> 4) - 1;
    if(tex > max_tex) tex = max_tex;
    if(tey > max_tey) tey = max_tey;

    //printf("WithinImpassable:(%d, %d)-(%d, %d)\n", tx, ty, tex, tey);

    //ok it is on the map, so what tiles might it intersect with?
    //for(i=tx, i_pix=tx*16; i<=tex; i++, i_pix+=16)
    //	for(j=ty, j_pix=ty*16; j<=tey; j++, j_pix+=16)
    for(i=tx, i_pix=tx<<4; i<=tex; i++, i_pix+=16)
        for(j=ty, j_pix=ty<<4; j<=tey; j++, j_pix+=16)
            if(!tile_check[i][j].passable)
            {
                //it is impassable, do we overlap it?
                if(x >= i_pix + 16) continue;
                if(y >= j_pix + 16) continue;
                if(x + w <= i_pix) continue;
                if(y + h <= j_pix) continue;

                stop_x = i << 4;
                stop_y = j << 4;

                //printf("caught at (%d, %d)\n", i, j);

                return true;
            }

    return false;
}

bool ZPath_Finding_Engine::HasDestroyableBarrier(int x, int y)
{
    if(!path_robot_tile) return false;
    if(!path_robot_norocks_tile) return false;

    if(x < 0) return false;
    if(y < 0) return false;
    if(x >= w) return false;
    if(y >= h) return false;

    return path_robot_norocks_tile[x][y].passable && !path_robot_tile[x][y].passable;
}

void ZPath_Finding_Engine::Clear_Thread_Id(int thread_id)
{
    vector<ZPath_Finding_Thread_Entry>::iterator t;
    for(t=thread_list.begin(); t!=thread_list.end(); t++)
    {
        if(t->thread_id == thread_id)
        {
            if(t->response) delete t->response;
            thread_list.erase(t);
            break;
        }
    }
}

void ZPath_Finding_Engine::Clear_Response_List()
{
    vector<ZPath_Finding_Response*>::iterator pf;
    for(pf=respone_list.begin(); pf!=respone_list.end(); pf++)
    {
        //Clear_Thread_Id((*pf)->thread_id);
        Kill_Thread_Id((*pf)->thread_id);

        delete *pf;
    }

    respone_list.clear();
}

void ZPath_Finding_Engine::Clear_Thread_List()
{
    vector<ZPath_Finding_Thread_Entry>::iterator t;
    for(t=thread_list.begin(); t!=thread_list.end();)
    {
        //printf("kill thread_id:%d\n", thread_id);
        if(t->the_thread)
        {
            int wait_status;

            //SDL_KillThread(t->the_thread);
            t->response->kill_thread = true;
            //printf("waiting for thread:%d\n", thread_id);
            SDL_WaitThread(t->the_thread, &wait_status);
            //printf("waiting for thread(exit):%d status:%d\n", thread_id, wait_status);

            //printf("kill thread_id:%d wait_status:%d\n", thread_id, wait_status);
        }
        t = thread_list.erase(t);
    }
}

void ZPath_Finding_Engine::Kill_Thread_Id(int thread_id)
{
    vector<ZPath_Finding_Thread_Entry>::iterator t;
    for(t=thread_list.begin(); t!=thread_list.end(); t++)
    {
        if(t->thread_id == thread_id)
        {
            //printf("kill thread_id:%d\n", thread_id);
            if(t->the_thread)
            {
                int wait_status;

                //SDL_KillThread(t->the_thread);
                t->response->kill_thread = true;
                //printf("waiting for thread:%d\n", thread_id);
                SDL_WaitThread(t->the_thread, &wait_status);
                //printf("waiting for thread(exit):%d status:%d\n", thread_id, wait_status);

                //printf("kill thread_id:%d wait_status:%d\n", thread_id, wait_status);
            }
            thread_list.erase(t);
            break;
        }
    }
}



int Find_Path_Thread(void *response_)
{
    ZPath_Finding_Response *response;

    //a check
    if(!response_) return 0;

    response = (ZPath_Finding_Response *)response_;

    //process
    ZPath_Finding_AStar::Do_Astar(response);

    //push for server to collect
    if(response->kill_thread)
        delete response;
    else
        response->path_finder->Push_Response(response);

    return 1;
}


int ZPath_Finding_Engine::Find_Path(int sx, int sy, int ex, int ey, bool is_robot, bool has_explosives, int obj_ref_id )
{
    ZPath_Finding_Response *response;
    SDL_Thread *the_thread = nullptr;
    map_pathfinding_info_tile **tile_check;

    //direct possible?
    if(Direct_Path_Possible(sx, sy, ex, ey, is_robot, has_explosives)) return 0;

    //not in same region?
    if(!InSameRegion(sx, sy, ex, ey, is_robot))
    {
        printf("!InSameRegion\n");
        return 0;
    }

    //inc
    next_thread_id++;

    //don't let it be zero
    if(!next_thread_id) next_thread_id++;

    //make data
    response = new ZPath_Finding_Response;

    //populate
    response->path_finder = this;
    response->thread_id = next_thread_id;
    response->obj_ref_id = obj_ref_id;
    response->start_x = sx;
    response->start_y = sy;
    response->end_x = ex;
    response->end_y = ey;
    response->is_robot = is_robot;
    response->kill_thread = false;

    //store in the tile info
    {
        if(is_robot)
        {
            if(has_explosives)
                tile_check = path_robot_norocks_tile;
            else
                tile_check = path_robot_tile;
        }
        else
        {
            if(has_explosives)
                tile_check = path_vehicle_norocks_tile;
            else
                tile_check = path_vehicle_tile;
        }

        if(tile_check)
        {
            int size;

            response->w = w;
            response->h = h;
            response->tile_info = tile_check;

            /*
            size = w * sizeof(map_pathfinding_info_tile *);
            response->tile_info = (map_pathfinding_info_tile **)malloc(size);
            size = h * sizeof(map_pathfinding_info_tile);
            for(int i=0;i<w;i++)
            {
                response->tile_info[i] = (map_pathfinding_info_tile *)malloc(size);
                memcpy(response->tile_info[i], tile_check[i], size);
            }
            */
        }
    }

    //start thread
    the_thread = SDL_CreateThread(Find_Path_Thread, (void*)response);

    //exit and return zero if thread couldn't start
    if(!the_thread)
    {
        printf("ZPath_Finding_Engine::Could not create Find_Path_Thread\n");
        delete response;
        return 0;
    }

    //printf("start thread:%d\n", next_thread_id);

    //add thread to thread list
    thread_list.push_back(ZPath_Finding_Thread_Entry(next_thread_id, the_thread, response));

    return next_thread_id;
}

bool ZPath_Finding_Engine::Direct_Path_Possible(int sx, int sy, int ex, int ey, bool is_robot, bool has_explosives)
{
    const int crawl_dist = 4;
    int A, B, C;
    double dx, dy;
    double x, y;
    int dist_left;
    double dist_check;
    double under;
    map_pathfinding_info_tile **tile_check;
    int bc;

    //already there...?
    if(sx == ex && sy == ey) return true;

    x = sx;
    y = sy;
    dx = ex - sx;
    dy = ey - sy;
    dist_left = sqrt((dx * dx) + (dy * dy));

    double hyp;

    if(fabs(dx) > fabs(dy))
        hyp = 1 / cos(atan2(fabs(dy), fabs(dx)));
    else
        hyp = 1 / sin(atan2(fabs(dy), fabs(dx)));

    //hyp = 1.414;
    //hyp += 0.08;

    //if(hyp > 1.414) hyp = 1.414;

    //printf("1.41:%lf\n", hyp );

    if(is_robot)
    {
        if(has_explosives)
            tile_check = path_robot_norocks_tile;
        else
            tile_check = path_robot_tile;

        dist_check = (8 + 8 + 1) * hyp;
        bc = 1;
    }
    else
    {
        if(has_explosives)
            tile_check = path_vehicle_norocks_tile;
        else
            tile_check = path_vehicle_tile;

        dist_check = (16 + 8 + 1) * hyp;
        bc = 2;
    }

    //line equation
    A = -1 * (sy - ey);
    B = (sx - ex);
    C = -1 * (A * sx + B * sy);

    //distance from line equals...
    //|Ax + Bn + C| / (A^2 + B^2)^0.5
    under = sqrt((double)((A * A) + (B * B)));

    //set right
    dx /= dist_left;
    dy /= dist_left;
    dx *= crawl_dist;
    dy *= crawl_dist;
    dist_check *= under;

    //go along the line and check
    //all the sectors around it
    while(dist_left > 0)
    {
        int ctx, cty;
        int tx, ty;
        int etx, ety;

        ctx = x / 16;
        cty = y / 16;

        etx = ctx+bc;
        ety = cty+bc;

        for(tx=ctx-bc;tx<=etx;tx++)
            for(ty=cty-bc;ty<=ety;ty++)
            {
                int center_x, center_y;

                if(tx < 0) continue;
                if(ty < 0) continue;
                if(tx >= w) continue;
                if(ty >= h) continue;

                //it good?
                if(tile_check[tx][ty].passable) continue;

                //distance smaller then our minimal?
                center_x = (tx * 16) + 8;
                center_y = (ty * 16) + 8;
                if(dist_check >= abs(A * center_x + B * center_y + C))
                {
                    //printf("Direct_Path_Possible::not possible...\n");
                    return false;
                }
            }

        x += dx;
        y += dy;
        dist_left -= crawl_dist;
    }

    //didn't find a false so I guess its good
    //printf("Direct_Path_Possible::possible...\n");
    return true;
}
// =========================================================
// *********************************************************
// =========================================================
void ZPath_Finding_Bresenham::Init(int stx_, int sty_, int etx_, int ety_, int w, int h)
{
    inited = false;

    if(stx_ < 0) return;
    if(sty_ < 0) return;
    if(stx_ >= w) return;
    if(sty_ >= h) return;
    if(etx_ < 0) return;
    if(ety_ < 0) return;
    if(etx_ >= w) return;
    if(ety_ >= h) return;

    stx = stx_;
    sty = sty_;
    etx = etx_;
    ety = ety_;

    nx = stx;
    ny = sty;

    dx = etx - stx;
    dy = ety - sty;

    if(dx < 0) step_x = -1;
    else step_x = 1;

    if(dy < 0) step_y = -1;
    else step_y = 1;

    dx = abs(dx*2);
    dy = abs(dy*2);

    if(dy>dx)
        fraction = dx * 2 - dy;
    else
        fraction = dy * 2 - dx;

    inited = true;
}

bool ZPath_Finding_Bresenham::GetNext(int &x, int &y)
{
    if(!inited) return false;

    if(dy>dx)
    {
        if(ny == ety) return false;

        if(fraction >= 0)
        {
            nx += step_x;
            fraction -= dy;
        }

        ny += step_y;
        fraction += dx;

        x = nx;
        y = ny;
    }
    else
    {
        if(nx == etx) return false;

        if(fraction >= 0)
        {
            ny += step_y;
            fraction -= dx;
        }

        nx += step_x;
        fraction += dy;

        x = nx;
        y = ny;
    }

    return true;
}
// =========================================================
// *********************************************************
// =========================================================
namespace ZPath_Finding_AStar
{
    int gf(int &cx, int &cy, int &nx, int &ny, ZPath_Finding_Response *response)
    {
        map_pathfinding_info_tile **tile_info = response->tile_info;

        if((cx == nx) || (cy == ny))
            return tile_info[nx][ny].side_weight;
        else
            return tile_info[nx][ny].diag_weight;
    }
    // ----------------------------------------------------
    int hf(int &sx, int &sy, int &fx, int &fy){
        return abs(sx - fx) + abs(sy - fy);
    }
    // ----------------------------------------------------
    bool tile_ok(int &sx, int &sy, int &ex, int &ey, ZPath_Finding_Response *response)
    {
        map_pathfinding_info_tile **tile_info = response->tile_info;

        if(response->is_robot)
        {
            if(ex < 0) return false;
            if(ey < 0) return false;
            if(ex >= response->w) return false;
            if(ey >= response->h) return false;

            if(!tile_info[ex][ey].passable) return false;

            //stop here if this is a side move
            if(ex == sx || ey == sy) return true;

            //do diag checks
            if(!tile_info[sx+(ex-sx)][sy].passable) return false;
            if(!tile_info[sx][sy+(ey-sy)].passable) return false;
        }
        else
        {
            if(ex < 0) return false;
            if(ey < 0) return false;
            if(ex+1 >= response->w) return false;
            if(ey+1 >= response->h) return false;

            if(!tile_info[ex][ey].passable) return false;
            if(!tile_info[ex+1][ey].passable) return false;
            if(!tile_info[ex][ey+1].passable) return false;
            if(!tile_info[ex+1][ey+1].passable) return false;

            //stop here if this is a side move
            if(ex == sx || ey == sy) return true;

            //what kind of diag is it?
            if(ex > sx && ey < sy) //upper right
            {
                if(!tile_info[sx][sy-1].passable) return false;
                if(!tile_info[sx+2][sy+1].passable) return false;
            }
            else if(ex < sx && ey < sy) //upper left
            {
                if(!tile_info[sx+1][sy-1].passable) return false;
                if(!tile_info[sx-1][sy+1].passable) return false;
            }
            else if(ex > sx && ey > sy) //lower right
            {
                if(!tile_info[sx+2][sy].passable) return false;
                if(!tile_info[sx][sy+2].passable) return false;
            }
            else //lower left
            {
                if(!tile_info[sx-1][sy].passable) return false;
                if(!tile_info[sx+1][sy+2].passable) return false;
            }
        }
        return true;
    }
    // ----------------------------------------------------
    void add_to_open_list(pf_point &cp, pf_point np, pf_point_array &open_list,
                          pf_point_array &closed_list, ZPath_Finding_Response *response)
    {
        int &end_x = response->end_x;
        int &end_y = response->end_y;

        if(!tile_ok(cp.x, cp.y, np.x, np.y, response)) return;


        if(open_list.point_index[np.x][np.y] != -1) return;

        if(int i = closed_list.point_index[np.x][np.y] != -1)
        {
            pf_point &ip = closed_list.list[i];

            if(ip.g > (gf(cp.x, cp.y, np.x, np.y, response) + cp.g))
            {
                ip.g = gf(cp.x, cp.y, np.x, np.y, response) + cp.g;
                ip.h = hf(np.x, np.y, end_x, end_y);
                ip.f = ip.g + ip.h;
                ip.px = cp.x;
                ip.py = cp.y;
                //debug_point("already on list", *i);
            }
            return;
        }

        np.g = gf(cp.x, cp.y, np.x, np.y, response) + cp.g;
        np.h = hf(np.x, np.y, end_x, end_y);
        np.f = np.g + np.h;
        np.px = cp.x;
        np.py = cp.y;

        //debug_point("new on list", np);
        open_list.AddPoint(np);
    }
    // ----------------------------------------------------
    void push_in_neighbors(pf_point &cp, pf_point_array &open_list,
                           pf_point_array &closed_list, ZPath_Finding_Response *response)
    {
        int &x = cp.x;
        int &y = cp.y;

        add_to_open_list(cp, pf_point(x-1, y-1), open_list, closed_list, response);
        add_to_open_list(cp, pf_point(x-1, y), open_list, closed_list, response);
        add_to_open_list(cp, pf_point(x-1, y+1), open_list, closed_list, response);

        add_to_open_list(cp, pf_point(x, y-1), open_list, closed_list, response);
        add_to_open_list(cp, pf_point(x, y+1), open_list, closed_list, response);

        add_to_open_list(cp, pf_point(x+1, y-1), open_list, closed_list, response);
        add_to_open_list(cp, pf_point(x+1, y), open_list, closed_list, response);
        add_to_open_list(cp, pf_point(x+1, y+1), open_list, closed_list, response);
    }
    // ----------------------------------------------------
    void remove_from_open(pf_point &tp, pf_point_array &open_list,
                          pf_point_array &closed_list)
    {
        open_list.RemovePoint(tp.x, tp.y);
        closed_list.AddPoint(tp);
    }
    // ----------------------------------------------------
    pf_point lowest_f_cost(pf_point_array &open_list)
    {
        pf_point lowest_found;
        lowest_found = *open_list.list;
        for(int i=0;i<open_list.size;i++)
        {
            pf_point &ip = open_list.list[i];
            if(lowest_found.f > ip.f)
                lowest_found = ip;
        }
        //not found?
        return lowest_found;
    }

    // ----------------------------------------------------

    void Do_Astar(ZPath_Finding_Response *response)
    {
        const int ticks_until_pause = 90;
        int total_ticks;
        int pause_ticks;
        double start_time, end_time;

        pf_point_array open_list;
        pf_point_array closed_list;
        pf_point cp;

        //check
        if(!response) return;

        int start_x = response->start_x / 16;
        int start_y = response->start_y / 16;
        int end_x = response->end_x / 16;
        int end_y = response->end_y / 16;

        //start and end ok?
        if(!tile_ok(start_x, start_y, start_x, start_y, response) ||
            !tile_ok(end_x, end_y, end_x, end_y, response))
        {
            response->pf_point_list.push_back(pf_point(response->end_x, response->end_y));
            return;
        }

        //printf("Do_Astar::start:[%d,%d]\n", response->start_x, response->start_y);
        //printf("Do_Astar::end:[%d,%d]\n", response->end_x, response->end_y);

        //init lists
        {
            int size;

            open_list.alloc_size = 1000;
            open_list.list = (pf_point*)malloc(open_list.alloc_size * sizeof(pf_point));
            open_list.size = 0;
            open_list.InitPointIndex(response->w, response->h);

            closed_list.alloc_size = 1000;
            closed_list.list = (pf_point*)malloc(closed_list.alloc_size * sizeof(pf_point));
            closed_list.size = 0;
            closed_list.InitPointIndex(response->w, response->h);
        }
        pf_point sp = pf_point(start_x, start_y);
        open_list.AddPoint(sp);

        total_ticks = 0;
        pause_ticks = 0;
        while(open_list.size)
        {
            cp = lowest_f_cost(open_list);

            if(response->kill_thread)
            {
                open_list.FreeList();
                open_list.FreePointIndex();
                closed_list.FreeList();
                closed_list.FreePointIndex();
                return;
            }

            if(cp.x == end_x && cp.y == end_y)
            {
                //printf("end found\n");
                break;
            }

            remove_from_open(cp, open_list, closed_list);

            if(response->kill_thread)
            {
                open_list.FreeList();
                open_list.FreePointIndex();
                closed_list.FreeList();
                closed_list.FreePointIndex();
                return;
            }

            push_in_neighbors(cp, open_list, closed_list, response);

            //printf("current_point:[%d,%d]\n", cp.x, cp.y);

            if(response->kill_thread)
            {
                open_list.FreeList();
                open_list.FreePointIndex();
                closed_list.FreeList();
                closed_list.FreePointIndex();
                return;
            }

            total_ticks++;
            pause_ticks++;
            if(pause_ticks >= ticks_until_pause)
            {
                pause_ticks = 0;
                SDL_Delay(static_cast<Uint32>(
                            10 + (ZPath_Finding_Response::existing_responses * 4)
                              ));
            }

            if(response->kill_thread)
            {
                open_list.FreeList();
                open_list.FreePointIndex();
                closed_list.FreeList();
                closed_list.FreePointIndex();
                return;
            }
        }
        //end_time = current_time();

        vector<pf_point> final_path;
        if(cp.x == end_x && cp.y == end_y)
        while(1)
        {
            int i;

            final_path.insert(final_path.begin(), cp);
            for(i=0;;i++)
            {
                pf_point &ip = closed_list.list[i];

                if(i>=closed_list.size)
                {
                    //printf("Do_Astar::could not reconstruct list\n");
                    break;
                }

                if(cp.px == ip.x && cp.py == ip.y)
                {
                    //printf("inserted point:[%d,%d]\n", i->x, i->y);
                    cp = ip;
                    break;
                }
            }

            if(i>=closed_list.size)
            {
                final_path.clear();
                break;
            }

            if(cp.x == start_x && cp.y == start_y)
                break;
        }

        //remove redundents
        int previous_dir = -1;
        int cur_dir;
        for(vector<pf_point>::iterator i=final_path.begin();i!=final_path.end();)
        {
            if(i!=final_path.begin() && i+1!=final_path.end())
            {
                previous_dir = the_dir(*(i-1), *(i));
                cur_dir = the_dir(*i, *(i+1));

                if(previous_dir == cur_dir)
                    i = final_path.erase(i);
                else
                    i++;
            }
            else
                i++;
        }


        //convert list to actual x,y cords
        for(vector<pf_point>::iterator i=final_path.begin();i!=final_path.end();i++)
        {
            i->x *= 16;
            i->y *= 16;

            if(response->is_robot)
            {
                i->x += 8;
                i->y += 8;
            }
            else
            {
                i->x += 16;
                i->y += 16;
            }
            //printf("Do_Astar::found point:[%d,%d]\n", i->x, i->y);
        }

        final_path.push_back(pf_point(response->end_x, response->end_y));

        response->pf_point_list = final_path;
        //return final_path;

        //printf("Do_Astar::finished, ticks:%d time:%lf ... [%d,%d] to [%d,%d]\n", total_ticks, end_time - start_time, response->start_x, response->start_y, response->end_x, response->end_y);

        //dealloc
        {
            open_list.FreeList();
            open_list.FreePointIndex();
            closed_list.FreeList();
            closed_list.FreePointIndex();
        }
    }
};
// =========================================================
// *********************************************************
// =========================================================

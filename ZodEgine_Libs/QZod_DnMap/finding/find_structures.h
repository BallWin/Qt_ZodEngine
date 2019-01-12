#ifndef FIND_STRUCTURES_H
#define FIND_STRUCTURES_H

#include "../qzod_dnmap_global.h"


enum pf_tile_types
{
    PF_NORMAL, PF_IMPASSABLE, PF_WATER, PF_ROAD, MAX_PF_TILE_TYPES
};

struct QZOD_DNMAPSHARED_EXPORT map_pathfinding_info_tile
{
    int side_weight;
    int diag_weight;
    bool passable;
};


class QZOD_DNMAPSHARED_EXPORT ffnode
{
public:
    ffnode(int x_, int y_) { x=x_; y=y_; }
    int x, y;
};

class QZOD_DNMAPSHARED_EXPORT pf_point
{
public:
    pf_point() { x = y = 0; }
    pf_point(int x_, int y_) {x=x_; y=y_; f=0; g=0; h=0;}

    int x, y;
    int f, g, h;
    int px, py;
};

class QZOD_DNMAPSHARED_EXPORT pf_point_array
{
public:
    pf_point_array()
    {
        list = nullptr; // TODO nullptr;
        point_index = nullptr; // TODO nullptr;
        size = 0;
        alloc_size = 0;
    }

    inline void InitPointIndex(int w_, int h_)
    {
        int ipi_size;

        w = w_;
        h = h_;

        ipi_size = w * sizeof(int *);
        point_index = (int **)malloc(ipi_size);
        ipi_size = h * sizeof(int);
        for(int i=0;i<w;i++)
        {
            point_index[i] = (int *)malloc(ipi_size);

            for(int j=0;j<h;j++)
                point_index[i][j] = -1;
        }
    }

    inline void FreePointIndex()
    {
        for(int i=0;i<w;i++) free(point_index[i]);
        free(point_index);
    }

    inline void FreeList()
    {
        free(list);
    }

    inline void AddPoint(pf_point &np)
    {
        if(size >= alloc_size)
        {
            alloc_size += 1000;
            list = (pf_point*)realloc(list, alloc_size * sizeof(pf_point));
        }

        list[size] = np;
        point_index[np.x][np.y] = size;

        size++;
    }

    inline void RemovePoint(int x, int y)
    {
        int i = point_index[x][y];

        if(i != -1)
        {
            point_index[x][y] = -1;

            size--;
            list[i] = list[size];

            point_index[list[i].x][list[i].y] = i;
        }
    }

    pf_point* list;
    int **point_index;
    int w, h;
    int size;
    int alloc_size;
};






// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
inline int the_dir(pf_point &lp, pf_point &cp)
{
    int res(-1);
    if(cp.x > lp.x)
    {
        if(cp.y == lp.y)
            res= 0;
        else if(cp.y < lp.y)
            res= 1;
        else
            res= 7;
    }
    else if(cp.x == lp.x)
    {
        if(cp.y > lp.y)
            res= 2;
        else
            res= 6;
    }
    else
    {
        if(cp.y < lp.y)
            res= 3;
        else if(cp.y > lp.y)
            res= 5;
        else
            res= 4;
    }

    if( res < 0 )
        std::cerr<<"\n--<error>\t ZPath_Finding_AStar::dir = -1?!\n";

    return res;
}

#endif

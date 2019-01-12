#ifndef QZOD_EFFECT_OLD_H
#define QZOD_EFFECT_OLD_H

#include "qzod_dneffect_global.h"



class QZOD_DNEFFECTSHARED_EXPORT effect_flags
{
public:
    effect_flags(){ Clear(); }

    bool unit_particles;
    int unit_particles_radius;
    int unit_particles_amount;
    int x, y;

    void Clear()
    {
        unit_particles = false;
        unit_particles_radius = 0;
        unit_particles_amount = 0;
    }
};

class QZOD_DNEFFECTSHARED_EXPORT ZEffect
{
    public:
        ZEffect(ZTime *ztime_);
        virtual ~ZEffect() {}

        static void SetEffectList(vector<ZEffect*> *effect_list_);
        static void SetSettings(ZSettings *zsettings_);
        static void SetMap(ZMap *zmap_) { zmap = zmap_; }

        virtual void Process();
        virtual void DoPreRender(ZMap &the_map, SDL_Surface *dest) {
            if( !the_map.isEmpty() ){
                map_basics mb=  the_map.GetMapBasics();
                std::cerr<<"\n--\t name map:\t"<< mb.map_name<<"\n";
            }
//            if(the_map.isEmpty())
//                std::cerr<<"\n--<error>\t"<<__FILE__<<"\t"<<__LINE__
//                        <<"\n\t ZMap is empty!\n";
            if( (dest==nullptr) || !(dest->w > 0) || !(dest->h > 0)  )
                std::cerr<<"\n--<error>\t"<<__FILE__<<"\t"<<__LINE__
                        <<"\n\t ZSDL_Surface is corrupt!\n";
        }
        virtual void DoRender(ZMap &the_map, SDL_Surface *dest){
            if( !the_map.isEmpty() ){
                map_basics mb=  the_map.GetMapBasics();
                std::cerr<<"\n--\t name map:\t"<< mb.map_name<<"\n";
            }
//            if(the_map.isEmpty())
//                std::cerr<<"\n--<error>\t"<<__FILE__<<"\t"<<__LINE__
//                        <<"\n\t ZMap is empty!\n";
            if( (dest==nullptr) || !(dest->w > 0) || !(dest->h > 0)  )
                std::cerr<<"\n--<error>\t"<<__FILE__<<"\t"<<__LINE__
                        <<"\n\t ZSDL_Surface is corrupt!\n";
        }
        bool KillMe();
        effect_flags &GetEFlags();
    protected:
        bool killme;

        static ZSettings *zsettings;
        static ZSettings default_settings;
        static vector<ZEffect*> *effect_list;
        static ZMap *zmap;
        ZTime *ztime;

        effect_flags eflags;
};


#endif // QZOD_EFFECT_OLD_H

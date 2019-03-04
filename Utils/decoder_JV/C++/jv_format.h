#ifndef JV_FORMAT_H
#define JV_FORMAT_H
// =========================================================
// *********************************************************
// =========================================================
#include "general_head.h"
// =========================================================
// *********************************************************
// =========================================================
#define MAGIC " Compression by John M Phillips Copyright (C) 1995 The Bitmap Brothers Ltd."
// =========================================================
// *********************************************************
// =========================================================
#pragma pack ( 1 )
// from https://wiki.multimedia.cx/index.php/JV
// ** this page has error - size header 0x68 (104 bytes), page present a 0x6C(108 bytes)
// ** 4 members extra by end
struct JV_FileHeader{
    uint8_t  sig[2];           //-- File signature, "JV"
    uint8_t  palmode1;         //-- Initial palette mode 'W' - all White, '0' (or any other value) - Black.
    uint8_t  palmode2;         //-- Final palette mode (see above)
    uint8_t  copyright[0x4C];  //-- MAGIC  (76 bytes)
    int16_t  width;            //-- Frame width
    int16_t  height;           //-- Frame height
    int16_t  frames;           //-- Number of --- chunks ------
    int16_t  rate;             //-- Frame rate. Delay between successive frames in ms
    int32_t  maxchunk;         //-- Maximal size of a single frame chunk
    int32_t  freq;             //-- Audio frequency
    uint8_t  flags0_1;
    uint8_t  volume_7;         //-- Default sound volume (0 - max)
    uint8_t  unused[6];

    JV_FileHeader(){ clear(); }
    bool isSuccess();
    void clear();

    std::string printf_head();
};

//File header is followed by frame index table, one entry per frame.
// Table entry format (0x10  16 bytes):
struct JV_TableFormat{
    int32_t chunk_size;        //-- Size of the chunk data
    int32_t audio_size;        //-- Size of the audio portion of the frame
    int32_t video_size;        //-- Size of the video portion of the frame
    int8_t  has_palette;       //-- New palette is present
    int8_t  audio_type;        //-- Audio format. Always 0 - mono, 8-bit PCM
    int8_t  video_type;        //-- Video compression method (see below)
    int8_t  unused;

    JV_TableFormat();
    bool isSuccess();
    void clear();
    std::string printf_tb_format();
};

// ------------------------------------------
// -- color components
struct Triplet{
    uint8_t trip[3];
    Triplet(){clear();}
    template<typename T>
    Triplet(T fill_val){ memset(trip,static_cast<uint8_t>(fill_val),3);}
    template<typename T>
    Triplet(T val0, T val1, T val2 ){
        trip[0]=static_cast<uint8_t>(val0);
        trip[1]=static_cast<uint8_t>(val1);
        trip[2]=static_cast<uint8_t>(val2);
    }
    Triplet(char *p_src);
    void operator *(const uint& val){ trip[0]*= val; trip[1]*= val; trip[2]*= val;}
    Triplet& operator *=(const uint& val){trip[0]*= val; trip[1]*= val; trip[2]*= val; return *this;}
    uint8_t& operator[](const uint& idx){ return trip[idx]; }
    void swapRB(){ std::swap(trip[0],trip[2]); }
    void copy_toBGR(uint8_t* dst){ uint8_t tr[]={trip[2], trip[1], trip[0]}; memcpy(dst, &tr[0], 3); }
    void copy_toRGB(uint8_t* dst){ memcpy(dst, &trip[0], 3); }
    void clear(){ memset(trip,0,3); }
    std::string printf_data();
};
// -- palette, array color components
struct jv_Palette{
    Triplet palette[256];   // data
    // -------------------------
    jv_Palette(){}
    jv_Palette(char *p_src);
    void init(char *p_src);
    Triplet& operator[](const uint& idx){ return palette[idx]; }
    void swapRB(){ for(int i=0; i<256; ++i){ palette[i].swapRB();} }
    void clear(){ memset(reinterpret_cast<char*>(&palette[0]),0,sizeof(Triplet)*256); }
    std::string printf_data();
};
#pragma pack()

// ------------------------------------------
class JV_Chunk{
public:
    enum type_video{T_Unknown=-1, T_RAW=0, T_BTC, T_SOLID};

    JV_Chunk();
    JV_Chunk(JV_TableFormat tab);
    JV_Chunk(JV_TableFormat &tab, char *p_data);

    bool isEmpty(){ return ((_audio_size + _video_size + _video_size) <= 0);}
    bool hasPaletteData(){ return (_palette_size>0) && (_palette_buff.get() != nullptr);}
    bool hasAudioData(){ return (_audio_size>0) && (_audio_buff.get() != nullptr);}
    bool hasVideoData(){ return (_video_size>0) && (_video_buff.get() != nullptr);}
    bool hasPaddingData(){ return (_padding_size>0); }
    int  typeVideo(){ return _type_video; }

    // - get data buffers or ptr
    char* audio_buff(){ if(_audio_buff) return reinterpret_cast<char*>(_audio_buff.get()); return nullptr; }
    std::shared_ptr<uint8_t>& audio_ptr(){ return _audio_buff; }
    char* video_buff(){ if(_video_buff) return reinterpret_cast<char*>(_video_buff.get()); return nullptr; }
    std::shared_ptr<uint8_t>& video_ptr(){ return _video_buff; }
    char* palette_buff(){ if(_palette_buff) return reinterpret_cast<char*>(_palette_buff.get()); return nullptr; }
    std::shared_ptr<uint8_t>& palette_ptr(){ return _palette_buff; }
    char* padding_buff(){ if(_padding_buff) return reinterpret_cast<char*>(_padding_buff.get()); return nullptr; }
    std::shared_ptr<uint8_t>& padding_ptr(){ return _padding_buff; }

    // - size buffers
    int32_t size_audio(){ return _audio_size; }
    int32_t size_video(){ return _video_size; }
    int32_t size_palette(){ return _palette_size; }
    int32_t size_padding(){ return _padding_size; }

    int32_t size_chunk_data(){return _audio_size+_video_size+_palette_size+_padding_size;}

    void clear();
protected:
    bool init_sizes(JV_TableFormat tab);
    bool init_copy_data(JV_TableFormat tab, char *p_data);
protected:
    int32_t                     _audio_size;
    int32_t                     _video_size;
    int32_t                     _palette_size;
    int32_t                     _padding_size;

    std::shared_ptr<uint8_t>    _audio_buff;        // -- Audio data
    std::shared_ptr<uint8_t>    _palette_buff;      // -- Palette
    std::shared_ptr<uint8_t>    _video_buff;        // -- Video data
    std::shared_ptr<uint8_t>    _padding_buff;      // -- sizeof(audio + palette + video + padding) == chunk_size

    int                         _type_video;        // -- raw btc solid
};
// ------------------------------------------
class JV_File{
public:
    JV_File(){clear();}
    JV_File(std::string file_jv_path);

    bool init(std::string file_jv_path, std::string *p_error= nullptr);

    bool is_empty(){ return _v_chunk.empty() || !_jv_head.isSuccess(); }
    size_t count_chunks(){ return  _v_chunk.size(); }

    bool get_chunk(size_t idx, JV_Chunk &chunk);
    JV_Chunk* operator[](const size_t &idx);
    JV_Chunk* at(size_t idx){ return (is_empty() || !(idx<_v_chunk.size()))? nullptr : &_v_chunk[idx]; }



    int frame_width(){return (is_empty())? 0 : _jv_head.width; }
    int frame_height(){return (is_empty())? 0 : _jv_head.height; }
    int frame_rate(){return (is_empty())? 0 : _jv_head.rate; }

    int max_chunk_size(){ return (is_empty())? 0 : _jv_head.maxchunk; }
    int sound_frequency(){ return (is_empty())? 0 : _jv_head.freq; }

    int count_audio_chunk(){ return _count_audio_chunk; }
    int count_video_chunk(){ return _count_video_chunk; }
    size_t full_audio_raw_size(){ return _full_audio_raw_size; }

    JV_FileHeader header(){ return _jv_head; }
    static long file_size(std::ifstream &file_jv, std::string *p_error= nullptr);
    std::string printf_file_info();
    void clear();

protected:
    std::streampos read_info_jv( std::ifstream &file_jv,
                                 JV_FileHeader & jv_head,
                                 std::string *p_error= nullptr);

    std::streampos read_tables_jv( std::ifstream &file_jv,
                                   std::vector<JV_TableFormat> &v_tables,
                                   JV_FileHeader jv_head,
                                   std::string *p_error= nullptr);
    size_t read_chunks(
            std::ifstream &file_jv,
            std::vector<JV_Chunk> &v_chunk,
            std::vector<JV_TableFormat> v_tables,
            std::string *p_error= nullptr);
protected:
    std::string                 _file_path;
    long                        _file_size{0};
    JV_FileHeader               _jv_head;
    int                         _count_video_chunk{0};
    int                         _count_audio_chunk{0};
    size_t                      _full_audio_raw_size{0};
    std::vector<JV_TableFormat> _v_tables;
    std::vector<JV_Chunk>       _v_chunk;

};









// =========================================================
// *********************************************************
// =========================================================
#endif // JV_FORMAT_H

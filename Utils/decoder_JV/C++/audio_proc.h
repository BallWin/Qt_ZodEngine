#ifndef AUDIO_PROC_H
#define AUDIO_PROC_H
// =========================================================
// *********************************************************
// =========================================================
#include "general_head.h"

#include "jv_format.h"
#include "bitstream.h"
#include "to_bmp.h"

// =========================================================
// *****************AUDIO***********************************
// =========================================================
#pragma pack ( 1 )
struct St_WAV_head
{
    char        sigRIFF[4]= {'R','I','F','F'};
    uint32_t    sizeRIFFch{0};     // size chunk RIFF = (36 + sizeDATAch) or (size wav - 8)
    char        sigWAVE[4]= {'W','A','V','E'};
    char        sigFMT[4]= {'f','m','t',' '};
    uint32_t    sizeFMTch{16};     // 16 for PCM
    uint16_t    formatTag{1};      // 1 for audio PCM  - no compressing
    uint16_t    nChannels{1};      // 1 - mono for audio JV
    uint32_t    sampleRate{0};     // samples-per-sec - frequency sound
    uint32_t    byteRate{0};       // count bytes per second (=sampleRate)for audio JV
    uint16_t    sampleSize{1};     // data block size in bytes (1)for audio JV
    uint16_t    bitsPerSample{8};  //  8-bit PCM for audio JV
    char        sigDATA[4]= {'d','a','t','a'};
    uint32_t    sizeDATAch{0};     // size data in bytes (1 channel)
    // --------------------------------------------
    bool is_not_init(){ return (sampleRate<=0)||(sampleSize<=0); }
    bool is_empty(){ return (sizeDATAch <= 0) || is_not_init(); }
    void clear(){ sizeRIFFch=sampleRate=byteRate=sizeDATAch= 0; }
    std::string printf_wav_head(){
        std::stringstream os;
        os<<"\n********* St_WAV_head **********"
         <<"\n\t"<<"sigRIFF:\t"<<std::string(&sigRIFF[0],4)<<"\t"<<"sizeRIFFch:\t"<<sizeRIFFch
           <<"\n\t"<<"sigWAVE:\t"<<std::string(&sigWAVE[0],4)<<"\t"<<"sigFMT:\t\t"<<std::string(&sigFMT[0],4)
             <<"\n\t"<<"sizeFMTch:\t"<<sizeFMTch<<"\t"<<"formatTag:\t\t"<<formatTag
               <<"\n\t"<<"nChannels:\t"<<nChannels<<"\t"<<"sampleRate:\t"<<sampleRate
                 <<"\n\t"<<"byteRate:\t"<<byteRate<<"\t"<<"blockAlign:\t"<<sampleSize
                   <<"\n\t"<<"bitsPerSample:\t"<<bitsPerSample
                     <<"\n\t"<<"sigDATA:\t"<<std::string(&sigDATA[0],4)<<"\t"<<"sizeDATAch:\t"<<sizeDATAch
                    <<std::endl;
        return os.str();
    }
};
#pragma pack()
// =========================================================
struct St_Audio_Label{
    int                         jv_chunk_num{-1};
    uint32_t                    size_audio{0};
    std::shared_ptr<uint8_t>    ptr_audio_chunk{nullptr};
    // -----------------------------------------------------
    St_Audio_Label(){}
    St_Audio_Label(int n, uint32_t sz, std::shared_ptr<uint8_t> ptr){ init(n,sz,ptr); }
    St_Audio_Label(int n, JV_Chunk* p_jv_chunk){ init(n, p_jv_chunk); }
    bool is_empty(){ return (jv_chunk_num<0) || !size_audio || (ptr_audio_chunk==nullptr);}
    // n(number chunk), sz(size data in bytes)
    bool init(int n, uint32_t sz, std::shared_ptr<uint8_t> ptr, std::string *p_error=nullptr){
        std::string er;
        if((n<0) || !sz || (ptr==nullptr)) er= MSG_FLINE_STR("\n--<error>\t the input data is incorrect!\n");
        else { jv_chunk_num= n; size_audio= sz; ptr_audio_chunk= ptr; }
        if(!er.empty() ){
            if(p_error != nullptr ) *p_error = er;
            else std::cerr<< er << std::endl;
        }
        return er.empty();
    }
    // true if chunk contains audio data and no errors, else false (check p_error to make sure there are errors)
    bool init( int number_chunk, JV_Chunk* p_jv_chunk, std::string *p_error=nullptr){
        std::string er;
        if( (p_jv_chunk == nullptr) || (p_jv_chunk->isEmpty())) er= MSG_FLINE_STR("\n--<error>\t JV chunk is empty!\n");
        else if( number_chunk < 0 ) er= MSG_FLINE_STR("\n--<error>\t number JV chunk is incorrect!\n");
        else if( p_jv_chunk->hasAudioData() ){
            if( !p_jv_chunk->size_audio() ) er= MSG_FLINE_STR("\n--<error>\t size audio JV chunk is incorrect!\n");
            else if((ptr_audio_chunk=p_jv_chunk->audio_ptr())==nullptr)
                er= MSG_FLINE_STR("\n--<error>\t data audio JV chunk is incorrect!\n");
            else{
                jv_chunk_num= number_chunk;
                size_audio= static_cast<uint32_t>(p_jv_chunk->size_audio());
            }
        }
        if(!er.empty() ){
            if(p_error != nullptr ) *p_error = er;
            else std::cerr<< er << std::endl;
        }
        return er.empty() && !is_empty();
    }
};
// =========================================================
struct St_AData{
    size_t                   size_data{0};
    std::shared_ptr<uint8_t> ptr_data{nullptr};
    St_AData(){}
    St_AData(size_t sz, std::shared_ptr<uint8_t> ptr){ size_data= sz; ptr_data=ptr; }
};

// =========================================================
class AudioWorker_JV
{
public:
    enum {MODE_WAV=0, MODE_RAW};
    AudioWorker_JV(){}
    AudioWorker_JV( JV_File* jv_file, int product_type= MODE_WAV );
    ~AudioWorker_JV();

    bool init( JV_File* jv_file, int product_type= MODE_WAV, std::string *p_error=nullptr);
    bool is_empty(){ return (_p_src_jv==nullptr)||(_p_src_jv->is_empty())||(_v_labels.empty()); }

    // -- sound frequency (Hz) sample-per-sec
    uint32_t frequency(){ return _freq; }

    // -- total
    // ------------------------------------------------------
    // -- total data playback time in milliseconds
    int calc_total_play_time_ms();
    // -- the total number of audio data in bytes
    size_t total_sound_size(){ return (is_empty())? 0 : _p_src_jv->full_audio_raw_size(); }    
    // -- save total audio data to wav file
    bool total_save_as( std::string save_as, std::string *p_error=nullptr);
    // -- produces a buffer with total audio data (RAW)
    bool total_raw_buff(St_AData &dst, std::string *p_error=nullptr);
    // -- produces a buffer with total audio data (WAV= WAV header + total RAW buffer)
    bool total_wav_buff(St_AData &dst, std::string *p_error=nullptr);


    // -- to work with parts
    // ------------------------------------------------------
    bool the_end_data(){ return (is_empty())? true : _data_start_pos==_data_end_pos; }

    void set_mode_WAV(){ _product_type= MODE_WAV; }
    void set_mode_RAW(){ _product_type= MODE_RAW; }
    int get_product_type(){ return _product_type; }

    // -- the current size of the read data in bytes
    size_t current_size_read(){ return _data_start_pos; }

    // -- number of sound fragments produced in the iteration
    void set_amount_produced( int val ){ _amount_produced= static_cast<size_t>(val); }
    int  get_amount_produced(){ return static_cast<int>(_amount_produced);}

    // -- the size of one audio fragment, in bytes
    void set_length_produced(int val){_length_produced= static_cast<size_t>(val); }
    int  get_length_produced(){ return static_cast<int>(_length_produced);}

    // -- the size of the audio data for the second playback in bytes
    int calc_data_size_per_frame(int fps);
    // -- audio data size for the specified playback time in milliseconds (in bytes)
    int calc_data_size_over_time_ms( int ms_time);
    // -- audio playback time for a given number of frames and FPS (milliseconds)
    int calc_time_playback_for_frames(int count_frames, int fps);

    // -- to create the _amount_produced number of audio fragments of a
    // _length_produced size at the _data_start_pos position of the read audio data
    std::vector<St_AData> get_product(std::string *p_error=nullptr);





    void clear();
protected:
    bool initTable( JV_File *p_jv_file, std::string *p_error=nullptr);
    int find_start_label( size_t &start_label_data_pos );

private:
    int             _product_type{0};   // type output data (WAV or RAW)
    JV_File*        _p_src_jv{nullptr};

    size_t          _label_pos{0};      // number of the last read chunk in table audio data
    size_t          _data_start_pos{0}; // the start position in the total array of audio data (bytes)
    size_t          _data_end_pos{0};   // the end position in the total array of audio data (bytes)

    size_t          _amount_produced{0};// number of sound fragments produced in the iteration
    size_t          _length_produced{0};// the size of one audio fragment, in bytes

    uint32_t        _freq{0};           // sound frequency (Hz) sample-per-sec

    // table labels JV chanks with audio data
    std::vector<St_Audio_Label>     _v_labels;

    St_WAV_head     _wav_head_jv;       // general WAV header - template for make wav parts

};



// =========================================================
// *********************************************************
// =========================================================
#endif // AUDIO_PROC_H

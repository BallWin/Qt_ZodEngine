#include "audio_proc.h"

// =========================================================
// *****************AUDIO***********************************
// =========================================================
AudioWorker_JV::AudioWorker_JV( JV_File* p_jv_file, int product_type)
{
    init( p_jv_file, product_type);
}
// =========================================================
AudioWorker_JV::~AudioWorker_JV()
{
    clear();
}
// =========================================================
// =========================================================
bool AudioWorker_JV::init(JV_File* p_jv_file, int product_type, std::string *p_error )
{
    clear();
    std::string er;
    if(p_jv_file == nullptr)
        er= MSG_FLINE_STR("\n--<error>\t the JV file pointer is empty!\n");
    else if( !(product_type==MODE_WAV || product_type==MODE_RAW) )
        er= MSG_FLINE_STR("\n--<error>\t unknown product type!!\n");
    else if( !p_jv_file->full_audio_raw_size() )
        std::cerr<<MSG_FLINE_STR("\n--<warning>\t this file has no audio data!")<<std::endl;
    else if( !initTable( p_jv_file, &er ) )
        er= MSG_FLINE_STR("\n--<error>\t failed to get audio data table!\n"+er+"\n");
    else{
        _p_src_jv= p_jv_file;
        _product_type= product_type;
        _label_pos= 0;
        _data_start_pos= 0;
        _data_end_pos= p_jv_file->full_audio_raw_size()-1;
        _freq= static_cast<uint32_t>(p_jv_file->sound_frequency());
        // -- by default, all audio data is read into one audio fragment
        _amount_produced= 1;
        _length_produced= p_jv_file->full_audio_raw_size();
        _wav_head_jv.sampleRate= static_cast<uint32_t>(_freq);
        _wav_head_jv.byteRate= _wav_head_jv.sampleRate;
    }
    if( !er.empty() ){
        if( p_error != nullptr ) *p_error= er;
        else    std::cerr<< er << std::endl;
    }
    return er.empty();
}

// =========================================================
bool AudioWorker_JV::initTable( JV_File *p_jv_file, std::string *p_error)
{
    std::string er;
    _v_labels.clear();
    if( p_jv_file->is_empty() )
        er=MSG_FLINE_STR("\n--<error>\t the input JV File is empty!\n");
    else{
        JV_Chunk* p_chunk(nullptr);
        size_t sz= p_jv_file->count_chunks();
        for( size_t i=0; i<sz; ++i )
        {
            p_chunk= p_jv_file->at(i);
            if( (p_chunk == nullptr) || p_chunk->isEmpty() ){
                er=MSG_FLINE_STR("\n--<error>["+std::to_string(i)+"]\t the data chunk is incorrect!\n");
                break;
            }
            else{
                if( p_chunk->hasAudioData() ){
                    _v_labels.push_back(St_Audio_Label(static_cast<int>(i), p_chunk));
                    if( _v_labels.back().is_empty() ){
                        er=MSG_FLINE_STR("\n--<error>\t error generating table data!\n");
                        break;
                    }
                }
            }
        }
    }
    if( !er.empty() ){
        if( p_error != nullptr ) *p_error= er;
        else    std::cerr<< er << std::endl;
    }
    return er.empty();
}
// =========================================================
// =========================================================
// -- the size of the audio data for the second playback in bytes
int AudioWorker_JV::calc_data_size_per_frame(int fps)
{
    return (!_freq || !(fps>0))?  0 : static_cast<int>(std::floor(_freq*1./fps + 0.5));
}
// -- total data playback time in milliseconds
int AudioWorker_JV::calc_total_play_time_ms()
{
    return (!_freq)? 0 : static_cast<int>(std::floor(total_sound_size()*1000./_freq + 0.5));
}
// -- audio data size for the specified playback time in milliseconds (in bytes)
int AudioWorker_JV::calc_data_size_over_time_ms( int ms_time)
{
    return (!_freq)? 0 : static_cast<int>(std::floor( ms_time* 1.*_freq/1000. + 0.5));
}
// -- audio playback time for a given number of frames and FPS (milliseconds)
int AudioWorker_JV::calc_time_playback_for_frames(int count_frames, int fps)
{
    return calc_data_size_per_frame(fps) * count_frames;
}
// =========================================================
// =========================================================
std::vector<St_AData> AudioWorker_JV::get_product(std::string *p_error)
{
    std::string er;
    int idx(0);
    size_t start_data_label(0);
    std::vector<St_AData> product;
    if( is_empty() )
        er=MSG_FLINE_STR("\n--<error>\t the handler not initialized!!\n");
    else if( !_amount_produced || !_length_produced )
        er=MSG_FLINE_STR("\n--<error>\t the number or size of created audio fragments are not specified!\n");
    else if(the_end_data())
        std::cerr<<MSG_FLINE_STR("\n--<warning> no audio data!\n")<<std::endl;
    else if( (idx=find_start_label(start_data_label)) < 0 )
        er=MSG_FLINE_STR("\n--<error>\t the initial position of the audio fragment data could "
                         "not be determined!\n");
    else{
        size_t sz_write(0), sz_read(0), sz_label(0);
        int count_labels = static_cast<int>(_v_labels.size());
        St_Audio_Label *p_label(nullptr);
        for( size_t am=0; am<_amount_produced; ++am )
        {
            if( the_end_data() || !er.empty() )
                break;
            sz_write= 0;
            sz_read= _length_produced;
            if( _data_start_pos + sz_read > total_sound_size() )
                sz_read= total_sound_size() - _data_start_pos;

            std::shared_ptr<uint8_t> ptr_raw(new uint8_t[sz_read]);
            memset( ptr_raw.get(), 0, sz_read);
            while( (sz_write<sz_read) && !(the_end_data()) )
            {
                if( idx < count_labels ){
                    p_label= &_v_labels[static_cast<uint>(idx)];
                    sz_label= p_label->size_audio;
                    if( start_data_label + sz_read <= sz_label  )
                    {
                        memcpy(ptr_raw.get() + sz_write,
                               p_label->ptr_audio_chunk.get() + start_data_label,
                               sz_read);
                        sz_write += sz_read;
                        start_data_label += sz_read;
                    }
                    else{
                        memcpy(ptr_raw.get() + sz_write,
                               p_label->ptr_audio_chunk.get() + start_data_label,
                               sz_label- start_data_label);
                        sz_write += sz_label- start_data_label;
                        start_data_label= 0;
                    }
                }
                else{
                    er=MSG_FLINE_STR("\n--<error>\t out of range table!\n");
                    break;
                }
                ++idx;
            }
            if(er.empty()){
                _data_start_pos += sz_write;
                product.push_back(St_AData(sz_write, ptr_raw));
            }
        }

        if( !product.empty() && (_product_type == MODE_WAV) )
        {
            St_WAV_head head = _wav_head_jv;
            size_t sz_head= sizeof(St_WAV_head);
            size_t sz_wav(0);
            for( St_AData &st : product ){
                sz_wav= sz_head + st.size_data;
                head.sizeRIFFch= static_cast<uint32_t>(sz_wav - 8);
                head.sizeDATAch = static_cast<uint32_t>(st.size_data);
                std::shared_ptr<uint8_t> ptr_raw(new uint8_t[sz_wav]);
                memcpy(ptr_raw.get(), reinterpret_cast<uint8_t*>(&head), sz_head );
                memcpy(ptr_raw.get()+sz_head, st.ptr_data.get(), st.size_data );
                st.ptr_data.swap( ptr_raw );
                st.size_data += sz_head;
            }
        }

    }
    if( !er.empty() ){
        if( p_error != nullptr ) *p_error= er;
        else    std::cerr<< er << std::endl;
    }
    return product;
}

// =========================================================
// -- produces a buffer with total audio data (RAW)
bool AudioWorker_JV::total_raw_buff(St_AData &dst, std::string *p_error)
{
    std::string er;
    size_t sz= total_sound_size();
    if( is_empty() )
        er=MSG_FLINE_STR("\n--<error>\t the handler not initialized!\n");
    else if(!sz)
        std::cerr<<MSG_FLINE_STR("\n--<warning>\t no audio data available!")<<std::endl;
    else{
        dst.size_data= 0;
        dst.ptr_data= std::shared_ptr<uint8_t>(new uint8_t[sz]);
        uint8_t *p_data(nullptr);
        p_data= dst.ptr_data.get();
        if( p_data == nullptr )
            er=MSG_FLINE_STR("\n--<error>\t memory for data recording is not allocated!\n");
        else{
            size_t sz_label(0);
            for( St_Audio_Label &label : _v_labels )
            {
                sz_label= label.size_audio;
                if( dst.size_data + sz_label <= sz ){
                    memcpy(p_data + dst.size_data, label.ptr_audio_chunk.get(), sz_label);
                    dst.size_data += sz_label;
                }
                else{
                    er=MSG_FLINE_STR("\n--<error>\t incorrect total size of data!\n");
                    break;
                }
            }
        }
    }
    if( !er.empty() ){
        if( p_error != nullptr ) *p_error= er;
        else    std::cerr<< er << std::endl;
    }
    return er.empty();
}
// -- produces a buffer with total audio data (WAV= WAV header + total RAW buffer)
bool AudioWorker_JV::total_wav_buff(St_AData &dst, std::string *p_error)
{
    std::string er;
    size_t sz= total_sound_size();
    if( is_empty() )
        er=MSG_FLINE_STR("\n--<error>\t the handler not initialized!\n");
    else if(!sz)
        std::cerr<<MSG_FLINE_STR("\n--<warning>\t no audio data available!")<<std::endl;
    else{
        size_t sz_head= sizeof(St_WAV_head);
        size_t sz_wav= sz_head + sz;
        dst.size_data= 0;
        dst.ptr_data= std::shared_ptr<uint8_t>(new uint8_t[sz_wav]);
        uint8_t *p_data(nullptr);
        p_data= dst.ptr_data.get();
        if( p_data == nullptr )
            er=MSG_FLINE_STR("\n--<error>\t memory for data recording is not allocated!\n");
        else{
            St_WAV_head head = _wav_head_jv;
            head.sizeRIFFch= static_cast<uint32_t>(sz_wav - 8);
            head.sizeDATAch = static_cast<uint32_t>(sz);
            memcpy(p_data, reinterpret_cast<uint8_t*>(&head), sz_head );
            dst.size_data += sz_head;
            size_t sz_label(0);
            for( St_Audio_Label &label : _v_labels )
            {
                sz_label= label.size_audio;
                if( dst.size_data + sz_label <= sz ){
                    memcpy(p_data + dst.size_data, label.ptr_audio_chunk.get(), sz_label);
                    dst.size_data += sz_label;
                }
                else{
                    er=MSG_FLINE_STR("\n--<error>\t incorrect total size of data!\n");
                    break;
                }
            }
        }
    }
    if( !er.empty() ){
        if( p_error != nullptr ) *p_error= er;
        else    std::cerr<< er << std::endl;
    }
    return er.empty();
}
// -- save total audio data to wav file
bool AudioWorker_JV::total_save_as( std::string save_as, std::string *p_error )
{
    std::string er;
    std::ofstream wav_file;
    wav_file.open (save_as, std::ios::out | std::ios::binary);
    if( is_empty() )
        er=MSG_FLINE_STR("\n--<error>\t the handler not initialized!!\n");
    else if( !wav_file.is_open())
        er= MSG_FLINE_STR("\n--<error>\t can't open file to write WAV!\n\t"
                          +save_as+"\n");
    else{
        try {
            St_WAV_head head = _wav_head_jv;
            size_t sz_head= sizeof(St_WAV_head);
            size_t sz_wav= sz_head + total_sound_size();
            head.sizeRIFFch= static_cast<uint32_t>(sz_wav - 8);
            head.sizeDATAch = static_cast<uint32_t>(total_sound_size());

            wav_file.write( reinterpret_cast<char*>(&head), static_cast<long>(sz_head));
            for( St_Audio_Label &label : _v_labels )
            {
                wav_file.write(
                            reinterpret_cast<char*>(label.ptr_audio_chunk.get()),
                            static_cast<long>(label.size_audio)
                            );
            }
            wav_file.close();
        } catch (...) {
            er= MSG_FLINE_STR("\n--<exception>\t unknown exception while writing WAV file!\n\t"
                              +save_as+"\n");
        }
    }
    if( !er.empty() ){
        if( p_error != nullptr ) *p_error= er;
        else    std::cerr<< er << std::endl;
    }
    return er.empty();
}
// =========================================================
int AudioWorker_JV::find_start_label( size_t &start_label_data_pos )
{
    int idx_start_label(-1);
    start_label_data_pos= 0;
    size_t sz(0);
    for( size_t i=0; i<_v_labels.size(); ++i )
    {
        sz += static_cast<size_t>(_v_labels[i].size_audio);
        if( sz >= _data_start_pos ){
            idx_start_label= static_cast<int>(i);
            start_label_data_pos= _data_start_pos - sz + static_cast<size_t>(_v_labels[i].size_audio);
            break;
        }
    }
    return idx_start_label;
}

// =========================================================
// =========================================================
void AudioWorker_JV::clear()
{
    _p_src_jv= nullptr;
    _product_type= MODE_WAV;
    _label_pos= 0;
    _data_start_pos= 0;
    _data_end_pos= 0;
    _freq= 0;
    _amount_produced= 0;
    _length_produced= 0;
    _v_labels.clear();
}
// =========================================================
// =========================================================





//bool AudioWorker_JV::init(size_t frequency, size_t max_chunk_size,
//                          size_t frame_rate, size_t sz_wav_chunk, std::string *p_error )
//{
//    clear();
//    std::string er;
//    if( !frequency || !max_chunk_size || !frame_rate )
//        er= MSG_FLINE_STR("\n--<error>\t the input data is incorrect!\n");
//    else {

//        _support_count_wav_chunk= sz_wav_chunk; // desired number of wav chumk in the data vector

//        std::cerr<<"\n--<debug>\t"<<"frequency= "<<frequency
//                <<"\t"<<"max_chunk_size= "<<max_chunk_size
//               <<"\t"<<"frame_rate= "<<frame_rate<<std::endl;

//        _wav_head_jv.sampleRate= static_cast<uint32_t>(frequency);
//        _wav_head_jv.byteRate= _wav_head_jv.sampleRate;


//        // -- size audio data for one frame time period
//        _size_raw_chunk = static_cast<size_t>(std::floor(frame_rate*frequency/1000. + 0.5));


//        // -- the maximum possible size of the data
//        _max_size_audio_data = max_chunk_size;

//        _frame_per_second= static_cast<size_t>(std::floor(1000./frame_rate + 0.5));


//        std::cerr<< _wav_head_jv.printf_wav_head()
//                 <<"\n--------------------------"
//                <<"\n--\t"<<"_size_raw_chunk=\t"<<_size_raw_chunk
//               <<"\n--\t"<<"_max_size_audio_data=\t"<<_max_size_audio_data
//              <<"\n--\t"<<"_frame_per_second=\t"<<_frame_per_second
//             <<std::endl;



//        //        --<debug>	frequency= 22050	max_chunk_size= 131073	 frame_rate= 84
//        //   131073/22050 = 5,94430839 s!
//        //   1852/22050 =     0.084 s = 84 ms   frame_rate= 84 ms
//        // 1000/22050 = 0.0454          frame_rate *22050/1000 = 1852

//        /*        ********* St_WAV_head **********
//            sigRIFF:	RIFF	sizeRIFFch:	0
//            sigWAVE:	WAVE	sigFMT:		fmt
//            sizeFMTch:	16	formatTag:		1
//            nChannels:	1	sampleRate:	22050
//            byteRate:	22050	blockAlign:	1
//            bitsPerSample:	8
//            sigDATA:	data	sizeDATAch:	0

//        --------------------------
//        --	_size_audio_data_by_frame=	1852
//        --	_full_audio_data_by_frame=	1896
//        --	_max_size_audio_data=	131073
//        --	_frame_per_second=	12
//        --	_wave_samples_per_second=	22224

//        --	 size sound data:	131072
//        --	 size sound data:	1852
//*/
//    }


//    if( !er.empty() ){
//        if( p_error != nullptr ) *p_error= er;
//        else    std::cerr<< er << std::endl;
//    }
//    return er.empty();
//}




//bool AudioWorker_JV::initTable( JV_File &jv_file, std::string *p_error)
//{
//    std::string er;
//    _v_labels.clear();
//    if( jv_file.is_empty() )
//        er=MSG_FLINE_STR("\n--<error>\t the input JV File is empty!\n");
//    else{
//        size_t sz= jv_file.count_chunks();
//        for( size_t i=0; i<sz; ++i )
//        {
//            if( !jv_file[i]->isEmpty() ){
//                if( jv_file[i]->hasAudioData() ){
//                    _v_labels.push_back(St_Audio_Label(static_cast<int>(i), jv_file[i]));
//                    if( _v_labels.back().is_empty() ){
//                        er=MSG_FLINE_STR("\n--<error>\t error generating table data!\n");
//                        break;
//                    }
//                }
//            }
//            else{
//                er=MSG_FLINE_STR("\n--<error>\t the input JV chunk is ["
//                                 +std::to_string(i)+"] incorrect!\n");
//                break;
//            }
//        }
//    }
//    if( !er.empty() ){
//        if( p_error != nullptr ) *p_error= er;
//        else    std::cerr<< er << std::endl;
//    }
//    return er.empty();
//}
//// =========================================================
//void AudioWorker_JV::run_make_wav( size_t N_frames )
//{
//    if( !_f_isRun  ){
//        _f_Stop= false;
//        _thread = std::thread( &AudioWorker_JV::create_wav_data, this, N_frames );
//    }
//}


//// =========================================================
//void AudioWorker_JV::create_wav_data( size_t N_frames )
//{
//    _f_isRun= true;
//    if(!N_frames)  N_frames = 30;
//    while( !is_empty() && (count_wav() < _support_count_wav_chunk) )
//    {
//        if( _f_Stop )
//            break;

//        size_t full_sz_raw(0);
//        for(St_Audio_Label &label : _v_labels )
//            full_sz_raw += label.size_audio;

//        double frames = full_sz_raw * 1./_size_raw_chunk;
//        if( frames < N_frames ){
//            N_frames = static_cast<size_t>(std::ceil(frames));
//        }
//        else if( frames <  N_frames*2 ){
//            N_frames = static_cast<size_t>(std::ceil(frames*0.5));
//        }
//        size_t max_sz = N_frames * _size_raw_chunk;
//        size_t sz_raw(0);
//        std::vector<std::shared_ptr<uint8_t> > v_raw;
//        std::vector<size_t> v_sizes;
//        while( (sz_raw<max_sz) && !_v_labels.empty() )
//        {
//            v_raw.push_back( _v_labels.front().ptr_audio_chunk );
//            v_sizes.push_back( _v_labels.front().size_audio );
//            sz_raw += v_sizes.back();
//            _v_labels.erase(_v_labels.begin());
//        }
//        if( sz_raw > 0  && !v_raw.empty() )
//        {
//            size_t sz_head= sizeof(St_WAV_head);
//            size_t sz_wav= sz_raw + sz_head;
//            St_WAV_head head= _wav_head_jv;
//            head.sizeRIFFch= static_cast<uint32_t>(sz_wav - 8);
//            head.sizeDATAch = static_cast<uint32_t>(sz_raw);

//            std::shared_ptr<uint8_t> wav_ptr(new uint8_t[sz_wav]);
//            uint8_t *p_data= wav_ptr.get();
//            memcpy(p_data,reinterpret_cast<uint8_t*>(&head), sz_head );
//            size_t pos(sz_head);
//            for(size_t i=0; i<v_raw.size(); ++i)
//            {
//                sz_raw= v_sizes[i];
//                memcpy( p_data + pos, v_raw[i].get(), sz_raw);
//                pos += sz_raw;
//            }

//            std::lock_guard<std::mutex> lock(_mutex);
//            _wav_data.push_back( wav_ptr );
//        }
//    }
//    _f_isRun= false;
//}
//// =========================================================
//bool AudioWorker_JV::get_front_wav_data(uint8_t *p_dst_audio_buff, size_t &sz_buf, std::string *p_error)
//{
//    std::string er;
//    if( p_dst_audio_buff== nullptr )
//        er= MSG_FLINE_STR("\n--<error>\t input buffer is empty!\n");
//    else if( !_wav_data.empty() )
//    {
//        std::shared_ptr<uint8_t> p_af= _wav_data.front();
//        _wav_data.erase(_wav_data.begin());
//        St_WAV_head w_head;
//        memcpy( reinterpret_cast<char*>(&w_head), p_af.get(), sizeof(St_WAV_head) );
//        if( w_head.is_not_init() )
//            er= MSG_FLINE_STR("\n--<error>\t incorrect write WAV header!\n");
//        else if( w_head.sizeDATAch + sizeof(St_WAV_head) > sz_buf )
//            er= MSG_FLINE_STR("\n--<error>\t insufficient input buffer size["
//                              +std::to_string(sz_buf)+"], required ["
//                              +std::to_string(w_head.sizeDATAch + sizeof(St_WAV_head))+"]!\n");
//        else {
//            sz_buf= w_head.sizeDATAch + sizeof(St_WAV_head);
//            memcpy(p_dst_audio_buff, p_af.get(), sz_buf );
//        }


//    }
//    if( !er.empty() ){
//        if( p_error != nullptr ) *p_error= er;
//        else    std::cerr<< er << std::endl;
//    }
//    return er.empty();
//}
//// =========================================================
//bool AudioWorker_JV::get_front_wav_ptr(std::shared_ptr<uint8_t> &p_front_wav)
//{
//    if( !_wav_data.empty() ){
//        p_front_wav.swap(_wav_data.front());
//        _wav_data.erase(_wav_data.begin());
//        return true;
//    }
//    return false;
//}
//// =========================================================


//// =========================================================
//bool AudioWorker_JV::save_front_wav_to_file(std::string save_as)
//{
//    std::string er;
//    if( !_wav_data.empty() )
//    {
//        FILE *file= fopen(save_as.c_str(), "wb+");
//        if( file == nullptr )
//            er= MSG_FLINE_STR("\n--<error>\t can't open file to write WAV!\n\t"
//                              +save_as+"\n");
//        else {
//            std::shared_ptr<uint8_t> wav_buff=  _wav_data.front();
//            St_WAV_head w_head;
//            memcpy(&w_head, wav_buff.get(), sizeof(St_WAV_head) );
//            size_t sz_wav_data= w_head.sizeDATAch + sizeof(St_WAV_head);
//            size_t sz_write = fwrite( wav_buff.get(), 1, sz_wav_data, file );
//            if( sz_write != sz_wav_data )
//                er= MSG_FLINE_STR("\n--<error>\t problems writing data. Written ["
//                                  +std::to_string(sz_write)+"] from ["+std::to_string(sz_wav_data)
//                                  +"] bytes.!\n\t"+save_as+"\n");
//        }
//    }
//    else {
//        std::cerr<<"\n--<warning>\t the data WAV vector is empty.\n";
//    }
//    if( !er.empty() )
//        std::cerr<< er << std::endl;
//    return er.empty();
//}
// =========================================================
// *********************************************************
// =========================================================

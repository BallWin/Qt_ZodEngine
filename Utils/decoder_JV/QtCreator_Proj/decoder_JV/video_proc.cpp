#include "video_proc.h"

// =========================================================
// *****************VIDEO***********************************
// =========================================================
VideoDecoder_JV::VideoDecoder_JV( JV_File* p_src_jv )
{
    std::string er;
    if( p_src_jv == nullptr )
        er= MSG_FLINE_STR("\n--<error>\t a pointer to the video source is NULL!\n");
    else if( p_src_jv->is_empty() )
        er= MSG_FLINE_STR("\n--<error>\t the video source is empty!\n");
    else {
        _p_src_jv= p_src_jv;
        _W= static_cast<size_t>(p_src_jv->frame_width());
        _H= static_cast<size_t>(p_src_jv->frame_height());
        _rate= p_src_jv->frame_rate();
        _pp_idx_matrix= new uint8_t*[_H];
        _pp_data_frame= new Triplet*[_H];
        if( (_pp_idx_matrix==nullptr) || (_pp_data_frame==nullptr) )
            er= MSG_FLINE_STR("\n--<error>\t could not memory allocated!\n");
        else{
            for(size_t i=0; i<_H; ++i){
                _pp_data_frame[i]= new Triplet[_W];
            }
            _p_buff_idx_map= new uint8_t[_H*_W];
            if( _p_buff_idx_map == nullptr )
                er= MSG_FLINE_STR("\n--<error>\t could not memory allocated!\n");
            else {
                BMP bmp(_W, _H);
                _size_buff_frame= bmp.calc_size_matrix_bmp(&_line_size);
            }
        }
    }
    if( !er.empty() ){
        std::cerr<< er << std::endl;
        clear();
    }
}
VideoDecoder_JV::~VideoDecoder_JV()
{
    clear();
}

// =========================================================
// * counting the total number of frames
int  VideoDecoder_JV::total_count_frames()
{
    int counter(0);
    if(!is_empty()){
        size_t count_chunks= _p_src_jv->count_chunks();
        JV_Chunk* p_chunk(nullptr);
        for(size_t i=0; i < count_chunks; ++i ){
            p_chunk= _p_src_jv->at(i);
            if( p_chunk==nullptr ){
                std::cerr<<MSG_FLINE_STR("\n--<error>\t JV chunk is NULL!\n")<<std::endl;
                break;
            }
            else if( p_chunk->isEmpty() ){
                std::cerr<<MSG_FLINE_STR("\n--<error>\t JV chunk is empty!\n")<<std::endl;
                break;
            }
            else if( p_chunk->hasVideoData() && p_chunk->size_video() ){
                ++counter;
            }
        }
    }
    return counter;
}
// =========================================================
// * decode all video frames to the provided container
bool VideoDecoder_JV::total_video_buff(std::vector<std::shared_ptr<uint8_t> > &v_dst, std::string *p_error)
{
    std::string er;
    v_dst.clear();
    if( is_empty() )
        er= MSG_FLINE_STR("\n--<error>\t video producer is empty!\n");
    else if(!_size_buff_frame)
        er= MSG_FLINE_STR("\n--<error>\t the full size of the video frame buffer is incorrect!!\n");
    else {
        jv_Palette  pal;
        size_t sz_chunks= _p_src_jv->count_chunks();
        JV_Chunk *p_chunk(nullptr);
        for( size_t i=0; i < sz_chunks; ++i )
        {
            p_chunk = _p_src_jv->at(i);
            if( p_chunk==nullptr){
                er= MSG_FLINE_STR("\n--<error>\t unable to retrieve chunk data!\n");
            }
            else if( p_chunk->isEmpty() )
                er= MSG_FLINE_STR("\n--<error>\t JV chunk is empty!\n");
            else{
                if( p_chunk->hasPaletteData() )
                    pal.init( p_chunk->palette_buff());
                if( p_chunk->hasVideoData() ){
                    v_dst.push_back( std::shared_ptr<uint8_t>(new uint8_t[_size_buff_frame]) );
                    if( v_dst.back()==nullptr )
                        er= MSG_FLINE_STR("\n--<error>\t no memory allocated for the frame!\n");
                    else if( !decode_video( *p_chunk, pal, &er ) )
                        er= MSG_FLINE_STR("\n--<error>\t to decode video data failed!\n"+er+"\n");
                    else if(!get_bmp_matrix( v_dst.back().get(), _size_buff_frame, true, &er ))
                        er= MSG_FLINE_STR("\n--<error>\t get bmp matrix problem!\n"+er+"\n");
                }
            }
            if(!er.empty())
                break;
        }
    }
    if(!er.empty()){
        if(p_error!=nullptr) *p_error= er;
        else std::cerr<< er <<std::endl;
    }
    return er.empty();
}
// =========================================================
// * save all frames to the ordered folder (as BMP files)
bool VideoDecoder_JV::total_save_frames_as_bmp(
        std::string path_folder_to_save, std::string prefix, std::string *p_error)
{
    std::string er;
    std::vector<std::shared_ptr<uint8_t> > v_frames;
    if( is_empty() )
        er= MSG_FLINE_STR("\n--<error>\t video producer is empty!\n");
    else if( !total_video_buff(v_frames, &er) )
        er= MSG_FLINE_STR("\n--<error>\t video frames could not be decoded!\n"+er+"\n");
    else{
        std::string name_file, pref, suff, save_as;
        pref= (!prefix.empty())? prefix+"-" : prefix;
        suff= ".bmp";
        int idx(0);
        for( std::shared_ptr<uint8_t> &ptr : v_frames )
        {
            name_file = pref+std::to_string(++idx)+suff;
            save_as= path_folder_to_save + name_file;
            BMP bmp(_W, _H, ptr.get());
            if(!bmp.write_matrix_to_file(save_as, &er)){
                er= MSG_FLINE_STR("\n--<error>\t failure when writing to the video frame in the file!!\n"
                                  +save_as+"\n"+ er +"\n");
                break;
            }
        }
    }
    if(!er.empty()){
        if(p_error!=nullptr) *p_error= er;
        else std::cerr<< er <<std::endl;
    }
    return er.empty();
}

// =========================================================
bool VideoDecoder_JV::read_frames(
        int max_count_read,
        std::vector<std::shared_ptr<uint8_t> > &v_frames,
        std::string *p_error
        )
{
    std::string er;
    v_frames.clear();
    if( is_empty() )
        er= MSG_FLINE_STR("\n--<error>\t video producer is empty!\n");
    else if( max_count_read <= 0 )
        er= MSG_FLINE_STR("\n--<error>\t the maximum number of frames to read is incorrect!\n");
    else {
        size_t sz_chunks= _p_src_jv->count_chunks();
        JV_Chunk *p_chunk(nullptr);
        for( int i=0; _jv_chunk_pos < sz_chunks; ++_jv_chunk_pos, ++i )
        {
            if( i>max_count_read )
                break;

            p_chunk = _p_src_jv->at(_jv_chunk_pos);
            if( p_chunk==nullptr){
                er= MSG_FLINE_STR("\n--<error>\t unable to retrieve chunk data!\n");
            }
            else if( !p_chunk->isEmpty() ){
                // -- load palette
                if( p_chunk->hasPaletteData() )
                    _palette.init( p_chunk->palette_buff());

                // - read and to decode video data
                if( p_chunk->hasVideoData() )
                {
                    v_frames.push_back( std::shared_ptr<uint8_t>(new uint8_t[_size_buff_frame]) );
                    if( v_frames.back()==nullptr )
                        er= MSG_FLINE_STR("\n--<error>\t no memory allocated for the frame!\n");
                    else if( !decode_video( *p_chunk, _palette, &er ) )
                        er= MSG_FLINE_STR("\n--<error>\t to decode video data failed!\n"+er+"\n");
                    else if(!get_bmp_matrix( v_frames.back().get(), _size_buff_frame, true, &er ))
                        er= MSG_FLINE_STR("\n--<error>\t get bmp matrix problem!\n"+er+"\n");
                }
            }
            if(!er.empty())
                break;
        }
    }
    if(!er.empty()){
        if(p_error!=nullptr) *p_error= er;
        else std::cerr<< er <<std::endl;
    }
    return er.empty();
}

// =========================================================
bool VideoDecoder_JV::decode_video(JV_Chunk& chunk, jv_Palette& palette, std::string *p_error)
{
    std::string er;
    if(chunk.isEmpty() )
        er= MSG_FLINE_STR("\n--<error>\t input shunk is empty!\n");
    else if( !chunk.hasVideoData() || !chunk.size_video() )
        er= MSG_FLINE_STR("\n--<error>\t input chunk does not contain video data!\n");
    else if(!make_index_frame(chunk, &er))
        er= MSG_FLINE_STR("\n--<error>\t could not make indexes frame!\n"+er+"\n");

    else{
        for(size_t i=0; i<_H; ++i)
            _pp_idx_matrix[i]= &_p_buff_idx_map[i*_W];

        uint idx, r(0), c(0);
        for( r=0; r<_H; ++r){
            for( c=0; c<_W; ++c){
                idx= _pp_idx_matrix[r][c];
                _pp_data_frame[r][c]= palette[idx];
            }
        }
    }
    if(!er.empty()){
        if(p_error!=nullptr) *p_error= er;
        else std::cerr<< er <<std::endl;
    }
    return er.empty();
}

// =========================================================
bool VideoDecoder_JV::make_index_frame(JV_Chunk& chunk, std::string *p_error )
{
    std::string er;
    int32_t video_size= chunk.size_video();
    int video_type=  chunk.typeVideo();
    size_t i(0),j(0);
    if( video_size > chunk.size_chunk_data() )
        er= MSG_FLINE_STR("\n--<error>\t incorrect size info: video size["
                          +std::to_string(video_size)+"] > size shunk["
                          +std::to_string(chunk.size_chunk_data()) +"]!\n");
    else if (video_type == 0 || video_type == 1)
    {
        JV_BitStream bitstr;
        if( !bitstr.init( chunk.video_buff(), static_cast<size_t>(video_size) ) )
            er= MSG_FLINE_STR("\n--<error>\t could not initialize bit stream!\n");
        else{
            for ( j = 0; j < _H; j += 8 ){
                for (i = 0; i < _W; i += 8){
                    bitstr.decode8x8( &_p_buff_idx_map[0] + j * _W + i, _W );
                }
            }
        }
        // what to do if there is data left in the chunk!??
    }
    else if (video_type == 2)
    {
        const uint8_t *buf = reinterpret_cast<const uint8_t*>(chunk.video_buff());
        int v = *buf++;
        for (j = 0; j < _H; j++){
            memset(&_p_buff_idx_map[0] + j * _W, v, _W);
        }
    }
    else
        er= MSG_FLINE_STR("\n--<error>\t unsupported video frame type ["
                          +std::to_string(video_type)+"]!\n");

    if(!er.empty()){
        if(p_error!=nullptr) *p_error= er;
        else std::cerr<< er <<std::endl;
    }
    return er.empty();
}
// =========================================================
bool VideoDecoder_JV::get_bmp_matrix_params(
        size_t &full_matrix_size, size_t &line_size, int &depth, int &width, int &height)
{
    std::string er;
    full_matrix_size=line_size=0;
    depth=  width=  height= 0;
    if( !is_empty() ){
        depth= _depth;
        width= static_cast<int>(_W);
        height= static_cast<int>(_H);
        full_matrix_size= _size_buff_frame;
        line_size= _line_size;
    }
    else
        er= MSG_FLINE_STR("\n--<error>\t decoder could not initialize!\n");
    if(!er.empty())
        std::cerr<< er <<std::endl;
    return er.empty();
}
// =========================================================
bool VideoDecoder_JV::get_bmp_matrix(uint8_t *p_dst_pixel_buff, size_t &sz_buf,
                                     bool f_bottom_up, std::string *p_error)
{
    std::string er;
    BMP bmp;
    if( is_empty())
        er= MSG_FLINE_STR("\n--<error>\t decoder is empty!\n");
    else if( p_dst_pixel_buff==nullptr)
        er= MSG_FLINE_STR("\n--<error>\t input buffer is empty!\n");
    else if( (bmp= make_bmp()).is_empty() )
        er= MSG_FLINE_STR("\n--<error>\t failed to get BMP structure!\n");
    else if( sz_buf != bmp.calc_size_matrix_bmp() )
        er= MSG_FLINE_STR("\n--<error>\t invalid size input buffer ["
                          +std::to_string(sz_buf)+"] != this["+std::to_string(bmp.calc_size_matrix_bmp())+"]!\n");
    else  if( !bmp.make_bmp_matrix(p_dst_pixel_buff, sz_buf, f_bottom_up, &er) )
        er= MSG_FLINE_STR("\n--<error>\t failed to get BMP matrix!\n");

    if(!er.empty()){
        if(p_error!=nullptr) *p_error= er;
        else std::cerr<< er << std::endl;
    }
    return er.empty();
}
// =========================================================
BMP VideoDecoder_JV::make_bmp()
{
    std::string er;
    if( is_empty())
        std::cerr<< MSG_FLINE_STR("\n--<error>\t decoder is empty!\n");
    else {
        BMP bmp( _W, _H );
        bmp.data = std::vector<uint8_t>(_W *_H * 3, 0);
        int idx_row= static_cast<int>(_H-1);
        int w_col= static_cast<int>(_W * 3);
        size_t dst_idx(0);
        for( int i=0; !(idx_row<0); --idx_row, ++i ){
            for( size_t j=0, J=0; j<_W ; ++j, J+= 3 ){
                dst_idx= static_cast<size_t>(i*w_col) + J;
                _pp_data_frame[idx_row][j].copy_toBGR( &bmp.data[dst_idx] );
            }
        }
        return bmp;
    }
    return BMP();
}

// =========================================================
bool VideoDecoder_JV::save_to_BMP(std::string save_as, std::string *p_error)
{
    std::string er;
    BMP bmp;
    if( is_empty())
        er= MSG_FLINE_STR("\n--<error>\t video producer is empty!\n");
    else if(save_as.empty())
        er= MSG_FLINE_STR("\n--<error>\t input path for save is empty!\n");
    else if( (bmp= make_bmp()).is_empty() )
        er= MSG_FLINE_STR("\n--<error>\t failed to get BMP matrix!\n");
    else  if( !bmp.write_matrix_to_file(save_as, &er) )
        er= MSG_FLINE_STR("\n--<error>\t Failed to write frame data to BMP file!\n["
                          + save_as +"]\n"+er+"\n");
    if(!er.empty()){
        if(p_error!=nullptr) *p_error= er;
        else std::cerr<< er <<std::endl;
    }
    return er.empty();
}
// =========================================================
void VideoDecoder_JV::clear()
{
    _W=_H=_size_buff_frame=_line_size=_jv_chunk_pos= 0;
    if( _pp_data_frame != nullptr ){
        for(size_t i=0; i<_H; ++i)
            delete[] _pp_data_frame[i];
        _pp_data_frame= nullptr;
    }
    if(_p_buff_idx_map != nullptr){
        delete[] _p_buff_idx_map;
        _p_buff_idx_map= nullptr;
    }
    _pp_idx_matrix = nullptr;
}
// =========================================================
// =========================================================
// =========================================================
// =========================================================
// =========================================================



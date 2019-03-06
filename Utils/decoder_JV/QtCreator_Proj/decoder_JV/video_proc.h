#ifndef VIDEO_PROC_H
#define VIDEO_PROC_H
// =========================================================
// *********************************************************
// =========================================================
#include "general_head.h"

#include "jv_format.h"
#include "bitstream.h"
#include "to_bmp.h"


// =========================================================
// *****************VIDEO***********************************
// =========================================================
class VideoDecoder_JV
{
public:
    VideoDecoder_JV( JV_File* p_src_jv);
    ~VideoDecoder_JV();

    bool is_empty(){return (_p_src_jv==nullptr)||((_W*_H)==0)||
                (_p_buff_idx_map==nullptr)||(_pp_data_frame==nullptr);}

    // -- info
    // ------------------------------------------------------
    size_t  width(){ return _W;}                            // width of the frame
    size_t  height(){ return _H;}                           // height of the frame
    size_t  size_buff_frame(){ return _size_buff_frame; }   // in bytes
    size_t  size_line_frame(){ return _line_size; }         // in bytes
    int     depth(){ return _depth; }                       // bit-per-pixel
    int     rate_video(){ return _rate; }                   // recommended frame rate (from JV file info)


    // -- total
    // ------------------------------------------------------
    // * counting the total number of frames
    int  total_count_frames();
    // * decode all video frames to the provided container
    bool total_video_buff(std::vector<std::shared_ptr<uint8_t> > &v_dst, std::string *p_error=nullptr);
    // * save all frames to the ordered folder (as BMP files)
    bool total_save_frames_as_bmp(std::string path_folder_to_save,
                                  std::string prefix=std::string(),
                                  std::string *p_error=nullptr);



    // -- to work with parts
    // ------------------------------------------------------
    bool the_end(){
        if( _p_src_jv != nullptr ){
            size_t ct= _p_src_jv->count_chunks();
            bool end= _jv_chunk_pos >= ct;
            return end;
        }
        return true;
    }

    bool read_frames(
            int max_count_read,
            std::vector<std::shared_ptr<uint8_t> > &v_frames,
            std::string *p_error= nullptr
            );

    /**
     * @brief get_bmp_matrix_params Calculation of parameters of the matrix BMP
     * @param full_matrix_size - full matrix size (pixel data only) including placeholders
     * @param line_size        - the size of the row of pixels according to the alignment
     * @param depth            - bit-per-pixel
     * @param width            - the width of the image (without taking into account the data alignment)
     * @param height           - the height of the image (without taking into account the data alignment)
     * @return true if successful, otherwise false
     */
    bool get_bmp_matrix_params(size_t &full_matrix_size, size_t &line_size,
                               int &depth, int &width, int &height);

    /**
     * @brief get_bmp_matrix    Get matrix pixel without taking into account the data alignment
     * @param p_dst_pixel_buff  - destination buffer (memory must be allocated in advance)
     * @param sz_buf            - the size of the allocated memory buffer
     * @param f_bottom_up       - fill direction from bottom to top (true) or top to bottom (false)
     * @param p_error           - saving error information
     * @return true if successful, otherwise false
     */
    bool get_bmp_matrix(uint8_t *p_dst_pixel_buff, size_t &sz_buf,
                        bool f_bottom_up= true, std::string *p_error=nullptr);

    // saving to BMP file
    bool save_to_BMP(std::string save_as, std::string *p_error=nullptr);


    void clear();

protected:
    bool decode_video( JV_Chunk& chunk, jv_Palette& palette, std::string *p_error=nullptr);
    bool make_index_frame(JV_Chunk& chunk, std::string *p_error=nullptr);
    BMP make_bmp();

protected:
    JV_File*    _p_src_jv{nullptr};
    jv_Palette  _palette;

    size_t      _W{0};     // width frame
    size_t      _H{0};     // height frame
    size_t      _size_buff_frame{0};
    size_t      _line_size{0};
    int         _depth{24};
    int         _rate{0};

    uint8_t     *_p_buff_idx_map{nullptr};       // data for index-frame
    Triplet     **_pp_data_frame{nullptr};       // pixel data frame
    uint8_t     **_pp_idx_matrix{nullptr};       // matrix index-frame

    size_t      _jv_chunk_pos{0};
};

#endif // VIDEO_PROC_H

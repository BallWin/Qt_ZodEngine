#ifndef TO_BMP_H
#define TO_BMP_H
// =========================================================
// *********************************************************
// =========================================================
#include "general_head.h"
// =========================================================
// *********************************************************
// =========================================================
#pragma pack ( 1 )
struct BMPFileHeader {
    uint16_t file_type{0x4D42};
    uint32_t file_size{0};
    uint16_t reserved1{0};
    uint16_t reserved2{0};
    uint32_t offset_data{0};

    bool is_empty(){return !file_size; }
    std::string printf_bmp_header(){
        std::stringstream os;
        os<<"\n----- BMPFileHeader ----"
         <<"\n\t"<<"file_type:\t"<<(reinterpret_cast<char*>(&file_type))[0]
        <<(reinterpret_cast<char*>(&file_type))[1]
        <<"\n\t"<<"file_size:\t"<< file_size
        <<"\n\t"<<"offset_data:\t"<< offset_data<<std::endl;
        return os.str();
    }

};
struct BMPInfoHeader {
    uint32_t size{ 0 };
    int32_t width{ 0 };
    int32_t height{ 0 };
    uint16_t planes{ 1 };
    uint16_t bit_count{ 0 };
    uint32_t compression{ 0 };
    uint32_t size_image{ 0 };
    int32_t x_pixels_per_meter{ 0 };
    int32_t y_pixels_per_meter{ 0 };
    uint32_t colors_used{ 0 };
    uint32_t colors_important{ 0 };

    bool is_empty(){ return !size || (width*height<=0); }
    std::string printf_bmp_info(){
        std::stringstream os;
        os<<"\n----- BMPInfoHeader ----"
         <<"\n\t"<<"size:\t"<<size
        <<"\n\t"<<"width:\t"<< width<<"\t height"<<height
        <<"\n\t"<<"planes:\t"<< planes
        <<"\n\t"<<"bit_count:\t"<< bit_count
          <<"\n\t"<<"compression:\t"<< compression
            <<"\n\t"<<"size_image:\t"<< size_image
              <<"\n\t"<<"x_pixels_per_meter:\t"<< x_pixels_per_meter
                <<"\n\t"<<"y_pixels_per_meter:\t"<< y_pixels_per_meter
                  <<"\n\t"<<"colors_used:\t"<< colors_used
        <<"\n\t"<<"colors_important:\t"<< colors_important
        <<std::endl;
        return os.str();
    }

};
#pragma pack()

struct BMP {
    BMPFileHeader file_header;
    BMPInfoHeader bmp_info_header;
    std::vector<uint8_t> data;

    BMP(){}

    // -- create from data (only 24 bit-per-pixel)
    BMP( size_t w, size_t h, uint8_t *p_frame_data )
    {
        std::string er;
        if(w*h == 0){
            er=MSG_FLINE_STR("\n--<error>\t incorrect input size frame ["
                                     +std::to_string(w)+"x"+std::to_string(h)+"]!\n");
        }
        else if( p_frame_data == nullptr )
            er=MSG_FLINE_STR("\n--<error>\t input buffer data is empty!\n");
        else {
            bmp_info_header.width = static_cast<int32_t>(w);
            bmp_info_header.height = static_cast<int32_t>(h);
            bmp_info_header.size = sizeof(BMPInfoHeader);
            file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

            bmp_info_header.bit_count = 24;
            bmp_info_header.planes= 1;
            bmp_info_header.compression = 0;
            row_stride = static_cast<uint>(w) * 3;
            data.resize(row_stride * static_cast<uint>(h));
            uint32_t new_stride = make_stride_aligned(4);
            file_header.file_size = static_cast<uint32_t>(file_header.offset_data);
            file_header.file_size+= static_cast<uint32_t>(data.size());
            file_header.file_size+= static_cast<uint32_t>(h * (new_stride - row_stride));

           for( uint i=0; i<data.size(); ++i )
                data[i]= p_frame_data[i];
        }

        if( !er.empty() ){
            std::cerr<< er <<std::endl;
            throw std::runtime_error("Could not initialize BMP structure!");
        }
    }
    // -- create zeros data BMP (24 bit-per-pixel)
    BMP(size_t w, size_t h) {
        std::string er;
        if(w*h == 0){
            er=MSG_FLINE_STR("\n--<error>\t incorrect input size frame ["
                                     +std::to_string(w)+"x"+std::to_string(h)+"]!\n");
        }
        else {
            bmp_info_header.width = static_cast<int32_t>(w);
            bmp_info_header.height = static_cast<int32_t>(h);
            bmp_info_header.size = sizeof(BMPInfoHeader);
            file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

            bmp_info_header.bit_count = 24;
            bmp_info_header.planes= 1;
            bmp_info_header.compression = 0;
            row_stride = static_cast<uint>(w) * 3;
            data.resize(row_stride * h);

            uint32_t new_stride = make_stride_aligned(4);
            file_header.file_size = static_cast<uint32_t>(file_header.offset_data);
            file_header.file_size+= static_cast<uint32_t>(data.size());
            file_header.file_size+= static_cast<uint32_t>(h * (new_stride - row_stride));
        }
        if( !er.empty() ){
            std::cerr<< er <<std::endl;
            throw std::runtime_error("Could not make BMP structure!");
        }
    }

    bool is_empty(){ return data.empty()||file_header.is_empty()||bmp_info_header.is_empty(); }


    // -- full size - all headers and size matrix for length row = data_row + aligned_padding_row
    size_t calc_size_file_bmp(){
        if( !is_empty() ){
            uint32_t new_stride = make_stride_aligned(4);
            return sizeof(BMPFileHeader) + sizeof(BMPInfoHeader)
                    +static_cast<size_t>(bmp_info_header.height)*new_stride;
        }
        return 0;
    }
    // -- full size matrix BMP pixel data
    size_t calc_size_matrix_bmp(size_t *p_line_size= nullptr){
        if( !is_empty() ){
            size_t line_size = make_stride_aligned(4);
            if(p_line_size != nullptr)
                *p_line_size = line_size;
            return static_cast<size_t>(bmp_info_header.height)*line_size;
        }
        return 0;
    }


    /**
     * @brief make_bmp_matrix  -- Copy frame data to buffer with in matrix BMP format
     * @param p_dst            -- target buffer, memory must be allocated
     * @param sz_buf           -- the size of the allocated memory target buffer
     * @param f_bottom_up      -- fill direction from bottom to top (true) or top to bottom (false)
     * @param p_error          -- saving error information
     * @return true if successful, otherwise false
     */
    bool make_bmp_matrix( uint8_t* p_dst, size_t sz_buf, bool f_bottom_up= true, std::string *p_error=nullptr)
    {
        std::string er;
        size_t sz(0);
        if(is_empty())
            er= MSG_FLINE_STR("\n--<error>\t BMP structute could not initialize!\n");
        else if( p_dst == nullptr )
            er= MSG_FLINE_STR("\n--<error>\t destination buffer is empty!\n");
        else if( !(sz= calc_size_file_bmp() || (sz != sz_buf) ) )
            er= MSG_FLINE_STR("\n--<error>\t could not calculate size output buffer!\n");
        else {
            memset(p_dst, 0, sz);
            if((bmp_info_header.width % 4 == 0) && !f_bottom_up ) {
                memcpy( &p_dst[0], data.data(), data.size());
            }
            else {
                uint32_t new_stride = make_stride_aligned(4);
                if( f_bottom_up )
                {
                    // filling lines from bottom to top
                    int pos1(0), pos2(0), y1(bmp_info_header.height-1), y2(0);
                    for (; y1>=0; --y1, ++y2) {
                        pos1= static_cast<int>(new_stride)*y1;
                        pos2= static_cast<int>(row_stride)*y2;
                        memcpy( &p_dst[0]+ pos1, data.data()+ pos2, row_stride );
                    }
                }
                else{
                    // filling lines from top to bottom
                    size_t H= static_cast<size_t>(bmp_info_header.height);
                    for (size_t y = 0; y < H; ++y) {
                        memcpy( &p_dst[0]+ y*new_stride, data.data()+ row_stride*y, row_stride );
                    }
                }
            }
        }
        if(!er.empty()){
            if(p_error != nullptr) *p_error= er;
            else  std::cerr<< er <<std::endl;
        }
        return er.empty();
    }


    // -- write matrix to BMP file (24 bit-per-pixel)
    bool write_matrix_to_file(std::string save_as, std::string *p_error=nullptr)
    {
        std::string er;
        uint8_t* p_dst= nullptr;
        size_t sz(0);
        std::ofstream of{ save_as.c_str(), std::ios_base::binary };
        if( !of )
            er= MSG_FLINE_STR("\n--<error>\t unable to open the output image file!\n["
                              + save_as + "]\n");
        else if( is_empty() )
            er= MSG_FLINE_STR("\n--<error>\t BMP structure is empty!\n");
        else if( bmp_info_header.bit_count != 24 )
                er= MSG_FLINE_STR("\n--<error>\t this is can treat only 24 bits per pixel BMP files!\n");
        else if( !(sz= calc_size_file_bmp() ) )
            er= MSG_FLINE_STR("\n--<error>\t could not calculate size output buffer!\n");
        else if( (p_dst= new uint8_t[sz]) == nullptr )
            er= MSG_FLINE_STR("\n--<error>\t can't memory allocated!\n");
        else if( !make_bmp_matrix(p_dst, sz, true, &er) )
            er= MSG_FLINE_STR("\n--<error>\t could not make BMP matrix!\n"+er+"\n");
        else {
            write_headers(of);
            const char* p= nullptr;
            p= reinterpret_cast<const char*>(p_dst);
            if( p != nullptr )
                of.write( p, static_cast<long>(sz));
        }
        delete[] p_dst;
        if(!er.empty()){
            if(p_error != nullptr) *p_error= er;
            else  std::cerr<< er <<std::endl;
        }
        return er.empty();
    }

    // -- write to file BMP (24 bit-per-pixel)
    bool write( std::string save_as )
    {
        std::string er;
        std::ofstream of{ save_as.c_str(), std::ios_base::binary };
        if( !of )
            er= MSG_FLINE_STR("\n--<error>\t unable to open the output image file!\n["
                              + save_as + "]\n");
        else if( is_empty() )
            er= MSG_FLINE_STR("\n--<error>\t BMP structure is empty!\n");
        else if( bmp_info_header.bit_count != 24 )
                er= MSG_FLINE_STR("\n--<error>\t this is can treat only 24 bits per pixel BMP files!\n");
        else {

            try {
                if (bmp_info_header.width % 4 == 0) {
                    write_headers_and_data(of);
                }
                else {
                    uint32_t new_stride = make_stride_aligned(4);
                    std::vector<uint8_t> padding_row(new_stride - row_stride);
                    write_headers(of);
                    size_t H= static_cast<size_t>(bmp_info_header.height);
                    for (size_t y = 0; y < H; ++y) {
                        const char* p= reinterpret_cast<const char*>((data.data() + row_stride * y));
                        of.write(p, row_stride);
                        const char* pp= reinterpret_cast<const char*>(padding_row.data());
                        of.write(pp, static_cast<long>(padding_row.size()));
                    }
                }
            } catch (...) {
                er= MSG_FLINE_STR("\n--<exception>\t unknown exception unknown exception while saving data to BMP file!\n");
            }
        }
        if( !er.empty() ){
            std::cerr<< er <<std::endl;
            throw std::runtime_error("Could not save data to BMP file!");
        }
        return er.empty();
    }

protected:
    void write_headers(std::ofstream &of) {
        const char* p= reinterpret_cast<const char*>(&file_header);
        of.write(p, sizeof(file_header));
        const char* pp= reinterpret_cast<const char*>(&bmp_info_header);
        of.write(pp, sizeof(bmp_info_header));
    }
    void write_headers_and_data(std::ofstream &of) {
        write_headers(of);
        const char* p= reinterpret_cast<const char*>(data.data());
        of.write(p, static_cast<long>(data.size()));
    }
    uint32_t make_stride_aligned(uint32_t align_stride) {
        uint32_t new_stride = row_stride;
        while (new_stride % align_stride != 0) {
            new_stride++;
        }
        return new_stride;
    }
private:
    uint32_t row_stride{ 0 };
};

// =========================================================
// *********************************************************
// =========================================================
#endif // TO_BMP_H

#include "bitstream.h"
// =========================================================
// *********************************************************
// =========================================================
JV_BitStream::JV_BitStream(char* p_buff, size_t sz_buff)
{
    init(p_buff, sz_buff);
}
// =========================================================
// =========================================================
bool JV_BitStream::init( char* p_buff, size_t sz_buff, std::string *p_error)
{
    std::string er;
    if(p_buff==nullptr || !sz_buff)
        er="\n--<error>\t bad parameters!\n";
    else {
        _size_bits_buff= sz_buff*8;
        _bits_buff = std::shared_ptr<char>(new char[_size_bits_buff]);
        std::string str_bits;
        for(size_t i=0, j=0; i<sz_buff; ++i, j+=8)
        {
            str_bits= std::bitset<8>(static_cast<uint8_t>(p_buff[i])).to_string();
            memcpy(_bits_buff.get()+j, str_bits.c_str(), 8 );
        }
        _pos= 0;
    }
    if(!er.empty()){
        if(p_error!=nullptr) *p_error= er;
        else    std::cerr<< er << std::endl;
    }
    return er.empty();
}
// =========================================================
size_t JV_BitStream::read_bitstream(size_t count_bits, std::string &dst_bits_str)
{
    size_t pos( _pos );
    dst_bits_str.clear();
    if( !is_end() ){
        if( _pos + count_bits >= _size_bits_buff )
            count_bits= _size_bits_buff - pos;
        dst_bits_str= std::string( &_bits_buff.get()[_pos], count_bits);
        _pos += count_bits;
    }
    return _pos-pos;
}
std::string JV_BitStream::get_string(int length)
{
    if(!is_empty()){
        size_t len= static_cast<size_t>(length);
        return (std::string(_bits_buff.get())).substr(0,(len<_size_bits_buff)? len:_size_bits_buff);
    }
    return std::string();
}
// =========================================================
void JV_BitStream::clear()
{
    _bits_buff = nullptr;
    _pos = 0;
    _size_bits_buff = 0;
}
// =========================================================
// =========================================================
ulong JV_BitStream::read_mark(){
    std::string str= std::string( &_bits_buff.get()[_pos], 2);
    _pos += 2;
    return (std::bitset<2>(str)).to_ulong();
}
uint8_t JV_BitStream::read_bit1()
{
    std::string str= std::string( &_bits_buff.get()[_pos],1);
    uint8_t b= static_cast<uint8_t>( (std::bitset<1>(str)).to_ulong());
    _pos ++;
    return b;
}
uint8_t JV_BitStream::read_bit8()
{
    std::string str= std::string( &_bits_buff.get()[_pos], 8);
    _pos += 8;
    return static_cast<uint8_t>((std::bitset<8>(str)).to_ulong());
}
// =========================================================
// =========================================================
void JV_BitStream::decode2x2( uint8_t *dst, size_t linesize)
{
    size_t i, j;
    uint8_t v[2];
    switch (read_mark()) {
    case 1:
        v[0] = read_bit8();
        for (j = 0; j < 2; j++)
            memset(dst + j * linesize, v[0], 2);
        break;
    case 2:
        v[0] = read_bit8();
        v[1] = read_bit8();
        for (j = 0; j < 2; j++)
            for (i = 0; i < 2; i++)
                dst[j * linesize + i] = v[read_bit1()];
        break;
    case 3:
        for (j = 0; j < 2; j++)
            for (i = 0; i < 2; i++)
                dst[j * linesize + i] = read_bit8();
    }
}
void JV_BitStream::decode4x4(uint8_t *dst, size_t linesize )
{
    int i, j;
    uint8_t v[2];
    switch (read_mark()) {
    case 1:
        v[0] = read_bit8();
        for (j = 0; j < 4; j++)
            memset(dst + static_cast<int>(linesize)*j, v[0], 4);
        break;
    case 2:
        v[0] = read_bit8();
        v[1] = read_bit8();
        for (j = 2; j >= 0; j -= 2) {
            for (i = 0; i < 4; i++)
                dst[static_cast<int>(linesize)*j + i] = v[read_bit1()];
            for (i = 0; i < 4; i++)
                dst[static_cast<int>(linesize)*(j+1) + i] = v[read_bit1()];
        }
        break;
    case 3:
        for (j = 0; j < 4; j += 2)
            for (i = 0; i < 4; i += 2)
                decode2x2(static_cast<int>(linesize)*j + dst + i, linesize);
    }
}
void JV_BitStream::decode8x8( uint8_t *dst, size_t linesize )
{
    if( !is_end()  && (_pos + 2 < _size_bits_buff) )
    {
        int i, j;
        uint8_t v[2];
        switch (read_mark()) {
        case 1:
            if( _pos + 8 < _size_bits_buff){
                v[0] = read_bit8();
                for(i=0; i<8; ++i)
                    memset(dst + static_cast<int>(linesize)*i, v[0], 8);
            }
            else
                std::cerr<<"\n--<error>\t_pos["<<_pos<<"+8 >size_buff["<<_size_bits_buff<<"]!\n";
            break;
        case 2:
            if(_pos + 16 < _size_bits_buff){
                v[0] = read_bit8();
                v[1] = read_bit8();
                for (j = 7; j >= 0; j--)
                    for (i = 0; i < 8; i++)
                        dst[static_cast<int>(linesize)*j + i] = v[read_bit1()];
            }
            else
                std::cerr<<"\n--<error>\t_pos["<<_pos<<"+16+64 >size_buff["<<_size_bits_buff<<"]!\n";
            break;
        case 3:
            for (j = 0; j < 8; j += 4)
                for (i = 0; i < 8; i += 4)
                    decode4x4( static_cast<int>(linesize)*j + dst + i, linesize);
            break;
        }
    }
}

// =========================================================
// *********************************************************
// =========================================================

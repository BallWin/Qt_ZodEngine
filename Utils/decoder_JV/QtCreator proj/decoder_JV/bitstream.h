#ifndef BITSTREAM_H
#define BITSTREAM_H
// =========================================================
// *********************************************************
// =========================================================
#include "general_head.h"
// =========================================================
// *********************************************************
// =========================================================
class JV_BitStream
{
public:
    JV_BitStream(){}
    JV_BitStream(char* p_buff, size_t sz_buff);

    bool is_empty(){ return !_size_bits_buff; }
    bool is_end(){ return (is_empty() || (_pos>=(_size_bits_buff-1))); }
    bool init( char* p_buff, size_t sz_buff, std::string *p_error= nullptr);

    size_t get_size_bits_buff(){ return _size_bits_buff; }
    size_t get_read_pos(){ return _pos; }

    void decode8x8(uint8_t *dst, size_t linesize);

    ulong       read_mark();
    uint8_t     read_bit1();
    uint8_t     read_bit8();
    size_t read_bitstream(size_t count_bits, std::string &dst_bit_str);


    std::string get_string(int length);

    void clear();
protected:
    void decode4x4(uint8_t *dst, size_t linesize);
    void decode2x2(uint8_t *dst, size_t linesize);


private:
    size_t                _size_bits_buff{0};
    size_t                _pos{0};
    std::shared_ptr<char> _bits_buff{nullptr};
};





// =========================================================
// *********************************************************
// =========================================================
#endif // BITSTREAM_H

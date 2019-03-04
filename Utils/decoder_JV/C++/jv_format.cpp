#include "jv_format.h"
// =========================================================
// *********************************************************
// =========================================================
bool JV_FileHeader::isSuccess(){
    std::string str_magic= reinterpret_cast<const char*>(copyright);
    bool f_sg=  sig[0]==74 && sig[1]==86 ; //("JV");
    bool f_mg= (str_magic == MAGIC);
    return f_sg && f_mg;
}
void JV_FileHeader::clear(){
    palmode1=palmode2=width=height=rate=maxchunk=freq =0;
    flags0_1=volume_7= 0;
    memset(sig, 0, 2); memset(copyright, 0, 0x4C); memset(unused, 0, 6);
}
std::string JV_FileHeader::printf_head()
{
    std::stringstream os;
    os<<"\n--\t"<<"sig"<<":\t"<<sig[0]<<sig[1];
    os<<"\n--\t"<<"palmode1"<<":\t"<<palmode1;
    os<<"\n--\t"<<"palmode2"<<":\t"<<palmode2;
    os<<"\n--\t"<<"copyright"<<":\t"<<reinterpret_cast<const char*>(copyright);
    os<<"\n--\t"<<"width"<<":\t"<<static_cast<uint>(width);
    os<<"\n--\t"<<"height"<<":\t"<<static_cast<uint>(height);
    os<<"\n--\t"<<"frames"<<":\t"<<static_cast<uint>(frames);
    os<<"\n--\t"<<"rate"<<":\t"<<static_cast<uint>(rate)<<" ms";
    os<<"\n--\t"<<"maxchunk"<<":\t"<<static_cast<uint>(maxchunk);
    os<<"\n--\t"<<"freq"<<":\t"<<static_cast<uint>(freq);
    os<<"\n--\t"<<"flags0_1"<<":\t"<< static_cast<uint>(flags0_1);
    os<<"\n--\t"<<"volume_7"<<":\t"<< static_cast<uint>(volume_7);
    os<<"\n--\t"<<"unused"<<":\t"<<reinterpret_cast<const char*>(unused);
    return os.str();
}

JV_TableFormat::JV_TableFormat()
    :chunk_size(0),audio_size(0),video_size(0),has_palette(0)
  ,audio_type(0),video_type(0),unused(0)
{

}
bool JV_TableFormat::isSuccess(){
    return (chunk_size>0) && (audio_size+video_size+ has_palette*768 == chunk_size);
}
void JV_TableFormat::clear(){
    chunk_size=audio_size=video_size= 0;
    has_palette=audio_type=video_type=unused= 0;
}
std::string JV_TableFormat::printf_tb_format()
{
    std::stringstream os;
    os<<"\n--\t"<<"chunk_size"<<":\t"<<chunk_size;
    os<<"\n--\t"<<"audio_size"<<":\t"<<audio_size;
    os<<"\n--\t"<<"video_size"<<":\t"<<video_size;
    os<<"\n--\t"<<"has_palette"<<":\t"<< static_cast<uint>(has_palette);
    os<<"\n--\t"<<"audio_type"<<":\t"<< static_cast<uint>(audio_type);
    os<<"\n--\t"<<"video_type"<<":\t"<< static_cast<uint>(video_type);
    os<<"\n--\t"<<"unused"<<":\t"<< static_cast<uint>(unused);
    return os.str();
}
// =========================================================
// *********************************************************
// =========================================================
Triplet::Triplet(char *p_src){
    if(p_src != nullptr)
        memcpy(reinterpret_cast<char*>(trip),p_src,3);
}
std::string Triplet::printf_data(){
    std::stringstream os; os<<"["<<static_cast<int>(trip[0])<<", "<<static_cast<int>(trip[1])
            <<", "<<static_cast<int>(trip[2])<<"] "; return os.str();}
// =========================================================
jv_Palette::jv_Palette(char *p_src){
    init(p_src);
}
void jv_Palette::init(char *p_src){
    if(p_src != nullptr){
        memcpy(reinterpret_cast<char*>(&palette[0]),p_src,sizeof(Triplet)*256);
        for(int i=0; i<256; ++i){ palette[i]*= 4;}
    }
    else{
        std::cerr<<MSG_FLINE_STR("\n--<error>\t pointer for data is NULL!\n")<<std::endl;
    }
}
std::string jv_Palette::printf_data(){
    std::stringstream os; os<<"\n--- jv_Palette ---"<<std::endl;
    for(int i=0; i<256; ++i){ os<<palette[i].printf_data();}  os<<std::endl;
    return os.str();
}
// =========================================================
// *********************************************************
// =========================================================
JV_Chunk::JV_Chunk():_audio_size(0),_video_size(0),_palette_size(0),_padding_size(0),
    _audio_buff(nullptr),_palette_buff(nullptr),_video_buff(nullptr),_padding_buff(nullptr),
    _type_video(T_Unknown)
{}
JV_Chunk::JV_Chunk(JV_TableFormat tab)
    :_audio_size(0),_video_size(0),_palette_size(0),_padding_size(0),
      _audio_buff(nullptr),_palette_buff(nullptr),_video_buff(nullptr),_padding_buff(nullptr),
      _type_video(T_Unknown)

{
     init_sizes(tab);
}
JV_Chunk::JV_Chunk(JV_TableFormat &tab, char *p_data)
    :_audio_size(0),_video_size(0),_palette_size(0),_padding_size(0),
      _audio_buff(nullptr),_palette_buff(nullptr),_video_buff(nullptr),_padding_buff(nullptr),
      _type_video(T_Unknown)
{
     init_copy_data(tab,p_data);
}
void JV_Chunk::clear(){
    _audio_size=_video_size=_palette_size=_padding_size= (0);
    if(_audio_buff)_audio_buff.reset();
    if(_palette_buff)_palette_buff.reset();
    if(_video_buff)_video_buff.reset();
    if(_padding_buff)_padding_buff.reset();
}

bool JV_Chunk::init_sizes(JV_TableFormat tab){
    if( tab.isSuccess() ){
        _type_video= tab.video_type;
        _audio_size= tab.audio_size;
        _video_size= tab.video_size;
        if(tab.has_palette) _palette_size= 768;
        _padding_size= tab.chunk_size - _audio_size-_palette_size-_video_size;
        if(_padding_size < 0 ){
            std::cerr<<MSG_FLINE_STR("\n--<error>\t invalid data in table: "
                    "chunk_size= "+std::to_string(tab.chunk_size)+" != ["
                   "audio_size("+std::to_string(_audio_size)+")+video_size("+std::to_string(_video_size)+
                  ")+palette("+std::to_string(_palette_size)+")+padding], padding= "+std::to_string(_padding_size)+"!\n");
            _audio_size=_video_size=_palette_size= 0;
        }
        else{
            if( _audio_size > 0) _audio_buff= std::shared_ptr<uint8_t>(new uint8_t[_audio_size]);
            if( _video_size > 0) _video_buff= std::shared_ptr<uint8_t>(new uint8_t[_video_size]);
            if( _palette_size > 0) _palette_buff= std::shared_ptr<uint8_t>(new uint8_t[_palette_size]);
            if( _padding_size > 0) _padding_buff= std::shared_ptr<uint8_t>(new uint8_t[_padding_size]);
        }
        return !(_padding_size < 0);
    }
    else {
        std::cerr<<MSG_FLINE_STR("\n--<error>\t invalid this table entry data!")<<std::endl;
    }
    return false;
}
bool JV_Chunk::init_copy_data(JV_TableFormat tab, char *p_data){
    if( p_data != nullptr ){
        if(init_sizes(tab) ){
            if( _audio_buff != nullptr ) memcpy(_audio_buff.get(), p_data, static_cast<uint>(_audio_size));
            if( _palette_buff != nullptr ) memcpy(_palette_buff.get(), &p_data[_audio_size], static_cast<uint>(_palette_size));
            if( _video_buff != nullptr ) memcpy(_video_buff.get(), &p_data[_audio_size], static_cast<uint>(_video_size));
            if( _padding_buff != nullptr ) memcpy(_padding_buff.get(), &p_data[_audio_size+_palette_size+_video_size], static_cast<uint>(_video_size));
            return true;
        }
        else
            std::cerr<<MSG_FLINE_STR("\n--<error>\t sizes it's not initialize!")<<std::endl;
    }
    else
        std::cerr<<MSG_FLINE_STR("\n--<error>\t invalid data pointer (NULL)!")<<std::endl;
    return false;
}
// =========================================================
// *********************************************************
// =========================================================

// =========================================================
// *********************************************************
// =========================================================
JV_File::JV_File(std::string file_jv_path)
{
    clear();
    init(file_jv_path);
}

bool JV_File::init(std::string file_jv_path, std::string *p_error)
{
    clear();
    long len(0);
    std::string er;
    std::ifstream file_jv(file_jv_path, std::ios::in | std::ios::binary );
    if(file_jv_path.empty())
        er= MSG_FLINE_STR("\n--<error>\t this file path is empty!\n");
    else if( !file_jv.is_open() )
        er= MSG_FLINE_STR("\n--<error>\t can't open file!\n["
                          +file_jv_path+"]\n");
    else if( (len=file_size(file_jv,&er)) <= static_cast<int>(sizeof(JV_FileHeader)+768) )
        er= MSG_FLINE_STR("\n--<error>\t incorrect file size ["+std::to_string(len)
                          +"!\n[" +file_jv_path+"]\n" + er);
    else if(read_info_jv(file_jv,_jv_head, &er) < 0 )
        er= MSG_FLINE_STR("\n--<error>\t can't to read header in file!\n["
                          +file_jv_path+"]\n" + er);
    else if(read_tables_jv(file_jv,_v_tables,_jv_head, &er) < 0)
        er= MSG_FLINE_STR("\n--<error>\t can't to read table in file!\n["
                          +file_jv_path+"]\n" + er);
    else if(read_chunks( file_jv, _v_chunk, _v_tables, &er) == 0 )
        er= MSG_FLINE_STR("\n--<error>\t can't to read chunks in file!\n["
                          +file_jv_path+"]\n" + er);
    else {
        _file_path= file_jv_path;
        _file_size= len;
    }

    if(!er.empty()){
        if(p_error!=nullptr) *p_error= er;
        else std::cerr<< er <<std::endl;
    }
    return er.empty();
}
// =========================================================
std::streampos JV_File::read_info_jv( std::ifstream &file_jv,
                             JV_FileHeader & jv_head,
                             std::string *p_error)
{
    std::string er;
    std::streampos pos(-1);
    jv_head.clear();
    if( !file_jv.is_open() )
        er= MSG_FLINE_STR("\n--<error>\t it's file not open!");
    else {
        file_jv.seekg(0, std::ios::beg);
        file_jv.read( reinterpret_cast<char*>(&jv_head), sizeof(JV_FileHeader) );
        if( file_jv.eof() )
            er= MSG_FLINE_STR("\n--<error>\t unexpected end of file!");
        else if(!jv_head.isSuccess())
            er= MSG_FLINE_STR("\n--<error>\t this file it's not \'JV\' format!");
        else
            pos= file_jv.tellg();
    }
    if(!er.empty()){
        if( p_error != nullptr ) *p_error= er;
        else std::cerr<< er << std::endl;
    }
    return pos;
}
// =========================================================
std::streampos JV_File::read_tables_jv( std::ifstream &file_jv,
                               std::vector<JV_TableFormat> &v_tables,
                               JV_FileHeader jv_head,
                               std::string *p_error)
{
    std::string er;
    long pos(-1);
    v_tables.clear();
    if( !file_jv.is_open() )
        er= MSG_FLINE_STR("\n--<error>\t it's file not open!");
    else if(!jv_head.isSuccess())
        er= MSG_FLINE_STR("\n--<error>\t JV header it's not success!");
    else {
        JV_TableFormat table;
        file_jv.seekg(sizeof(JV_FileHeader), std::ios::beg);
        long sz_table= static_cast<long>(sizeof(JV_TableFormat));
        for(int i=0; i<jv_head.frames; ++i)
        {
            pos= file_jv.tellg();
            if( file_jv.eof() ){
                er= MSG_FLINE_STR("\n--<error>\t unexpected end of file!");
                pos= (-1);
                break;
            }
            file_jv.read( reinterpret_cast<char*>(&table), sz_table);
            v_tables.push_back(table);
            file_jv.seekg(pos + sz_table);
        }
        if( er.empty() )
            pos= file_jv.tellg();
    }
    if(!er.empty()){
        if( p_error != nullptr ) *p_error= er;
        else std::cerr<< er << std::endl;
    }
    return pos;
}
// =========================================================
size_t JV_File::read_chunks(
        std::ifstream &file_jv,
        std::vector<JV_Chunk> &v_chunk,
        std::vector<JV_TableFormat> v_table,
        std::string *p_error)
{
    long pos, chunk_pos;
    char *p_data= nullptr;
    long size_data(0);
    v_chunk.clear();
    _count_audio_chunk=_count_video_chunk= 0;
    _full_audio_raw_size= 0;
    std::string er;
    if( !file_jv.is_open() )
        er= MSG_FLINE_STR("\n--<error>\t it's file not open!");
    else if(v_table.empty() )
        er= MSG_FLINE_STR("\n--<error>\t The table is empty!");
    else {
        size_t data_offset= sizeof(JV_FileHeader) + sizeof(JV_TableFormat) * v_table.size();
        file_jv.seekg(static_cast<long>(data_offset), std::ios::beg);

        for(size_t i=0; i<v_table.size(); ++i)
        {
            pos=chunk_pos= file_jv.tellg();
            if(v_table[i].chunk_size > 0){

                JV_Chunk chunk( v_table[i] );
                if( !chunk.isEmpty() ){
                    p_data = chunk.audio_buff();
                    size_data= chunk.size_audio();
                    if( (size_data > 0) && (p_data != nullptr)){
                        file_jv.read(p_data, size_data);
                        file_jv.seekg(chunk_pos+size_data);
                        chunk_pos += size_data;
                        _count_audio_chunk++;
                        _full_audio_raw_size+= static_cast<size_t>(size_data);
                    }
                    p_data = chunk.palette_buff();
                    size_data= chunk.size_palette();
                    if( (size_data > 0) && (p_data != nullptr) ){
                        file_jv.read(p_data, size_data);
                        file_jv.seekg(chunk_pos+size_data);
                        chunk_pos += size_data;
                    }
                    p_data = chunk.video_buff();
                    size_data= chunk.size_video();
                    if( (size_data > 0) && (p_data != nullptr) ){
                        file_jv.read(p_data, size_data);
                        file_jv.seekg(chunk_pos+size_data);
                        chunk_pos += size_data;
                        _count_video_chunk++;
                    }
                    v_chunk.push_back(chunk);
                }
                else {
                    std::cerr<<"\n--<error>\t it's not chunk initialize!\n";
                }
                file_jv.seekg(pos + v_table[i].chunk_size);
            }
            else {
                std::cerr<<"\n--<error>\t invalid table block - chunk_size <= 0!\n";
                break;
            }
        }
    }
    if(!er.empty()){
        if( p_error != nullptr ) *p_error= er;
        else std::cerr<< er << std::endl;
    }
    return v_chunk.size();
}
// =========================================================
bool JV_File::get_chunk(size_t idx, JV_Chunk &chunk)
{
    if(is_empty())
        std::cerr<<MSG_FLINE_STR("\n--<error>\t object a JV_File is empty!");
    else if( idx >= _v_chunk.size() )
        std::cerr<<MSG_FLINE_STR("\n--<error>\t input index ["+std::to_string(idx)+"] is corrupt! "
                                 "JV_File has ["+std::to_string(_v_chunk.size())+"] chunks.\n");
    else{
         chunk= _v_chunk[idx];
         return true;
    }
    return false;
}
JV_Chunk* JV_File::operator[](const size_t &idx)
{
    if(is_empty())
        std::cerr<<MSG_FLINE_STR("\n--<error>\t object a JV_File is empty!");
    else if( idx >= _v_chunk.size() )
        std::cerr<<MSG_FLINE_STR("\n--<error>\t input index ["+std::to_string(idx)+"] is corrupt! "
                                 "JV_File has ["+std::to_string(_v_chunk.size())+"] chunks.\n");
    else{
        return &_v_chunk[idx];
    }
    return nullptr;
}
// =========================================================
// =========================================================
// =========================================================
std::string JV_File::printf_file_info()
{
    if( !is_empty() ){
        std::stringstream os;
        os<<"\n**\t file ["<<_file_path <<"]"
         <<"\n**\t file size:\t"<<_file_size
         <<"\n**\t count chunks:\t"<<_v_chunk.size()
        << _jv_head.printf_head();
        return os.str();
    }
    return "\n--<error>\t object a file is not init!\n";
}
// =========================================================
long JV_File::file_size(std::ifstream &file_jv, std::string *p_error)
{
    long sz(-1);
    std::string er;
    if( !file_jv.is_open() )
        er= MSG_FLINE_STR("\n--<error>\t it's file not open!");
    else {
        std::streampos pos= file_jv.tellg();
        file_jv.seekg(0, std::ios::end);
        sz= file_jv.tellg();
        file_jv.seekg(0, std::ios::beg);
        file_jv.seekg(pos);
    }
    if(!er.empty()){
        if( p_error != nullptr ) *p_error= er;
        else std::cerr<< er << std::endl;
    }
    return sz;
}
// =========================================================
void JV_File::clear()
{
    _file_path.clear();
    _v_tables.clear();
    _v_chunk.clear();
    _jv_head.clear();
    _file_size= 0;
    _full_audio_raw_size = 0;
    _count_video_chunk=_count_audio_chunk =0;
}













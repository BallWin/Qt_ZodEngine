// ===========================================================
// ***********************************************************
// ===========================================================
#include "general_head.h"
#include "video_proc.h"
#include "audio_proc.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_video.h>


// ===========================================================
// ***********************************************************
// ===========================================================
#ifdef WIN32
static std::string slh= "\\";
#else
static std::string slh= "/";
#endif

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
static uint32_t amask = 0;                  // no alpha! (24 bit-per-pix)  //0xff000000;
static uint32_t rmask = 0x0000ff00;
static uint32_t gmask = 0x00ff0000;
static uint32_t bmask = 0xff000000;
#else
static uint32_t amask = 0;                  // no alpha! (24 bit-per-pix)  //0xff000000;
static uint32_t rmask = 0x00ff0000;
static uint32_t gmask = 0x0000ff00;
static uint32_t bmask = 0x000000ff;
#endif


static std::string folder_root;
// - default pathes
static std::string folder_cuts= slh+"Cuts";
static std::string folder_frames= slh+"frames";
static std::string path_to_file= folder_cuts + slh+ "E_AWINPL.JV";
// - comand line params
struct St_Params{
    std::string path_to_file_jv;
    std::string folder_to_save_frames;
    std::string name_wav_file;
    int         fps{-1};
    bool        f_show{true};
    uint8_t     aps[3];
    std::string printf_params(){
        std::stringstream os; std::boolalpha(os);
        os<<"\n\t* path_to_file_jv:\t["<<path_to_file_jv<<"]"
         <<"\n\t* folder_to_save_frames:\t["<<folder_to_save_frames<<"]"
        <<"\n\t* name_wav_file:\t["<<name_wav_file<<"]"
        <<"\n\t* fps:\t["<<std::to_string(fps)<<"]"
        <<"\n\t* f_show:\t["<<f_show<<"]"<<std::endl;
        return os.str();
    }
};


int show_cut(VideoDecoder_JV &video_ch, AudioWorker_JV &audio_ch, long custom_fps);
int save_frame(VideoDecoder_JV &video_ch, std::string folder_save_path);
int save_wav(AudioWorker_JV &audio_ch, std::string name_wav_file);

std::string comand_line_help();
int parser_comand_line(int argc, char* argv[], St_Params &params);
// ===========================================================
// ***********************************************************
// ===========================================================
int main(int argc, char* argv[])
{
    std::cout<<"\n************** START **************\n";
    St_Params st_params;
    if( parser_comand_line(argc,argv,st_params) < 0 )
        return -1;
    std::cout<<"\n&&&&\t User params:\t &&&&&&&&&&&&&"
            << st_params.printf_params() << std::endl;

    std::string error;


    // Init decoder
    // ----------------------------------------------------------
    // - load selected cut JV
    JV_File jv_file(st_params.path_to_file_jv);

    // - init video decoder JV
    VideoDecoder_JV video_ch(&jv_file);

    // - init audio worker JV
    AudioWorker_JV  audio_ch(&jv_file);

    if( jv_file.is_empty() || video_ch.is_empty() || audio_ch.is_empty() ){
        std::cerr<<"\n--<error>\t decoder is not initialize!\n";
        return -1;
    }

    // If you ordered the show
    if( st_params.f_show ){
        if( show_cut(video_ch,audio_ch, st_params.fps ) < 0 ){
            std::cerr<<"\n--<error>\t Show failed!\n";
            return -1;
        }
    }
    // If saving frames is ordered
    if( !st_params.folder_to_save_frames.empty() ){
        if(save_frame(video_ch, st_params.folder_to_save_frames) < 0 ){
            std::cerr<<"\n--<error>\t Save frames failed!\n";
            return -1;
        }
    }
    // If you ordered sound saving
    if( !st_params.name_wav_file.empty() ){
        if( save_wav(audio_ch, st_params.name_wav_file) < 0 ){
            std::cerr<<"\n--<error>\t Save sound failed!\n";
            return -1;
        }
    }

    std::cout<<"\n\n%%%%%%%%%%%%%%%% END  %%%%%%%%%%%%%%%%%%%%%%%%\n";
    return EXIT_SUCCESS;
}
// ===========================================================
// ***********************************************************
// ===========================================================
int show_cut(VideoDecoder_JV &video_ch, AudioWorker_JV &audio_ch, long custom_fps)
{

    std::string error;
    // Processing and display of JV video from SDL
    // -----------------------------------------------------------
    if (SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        std::cerr<<"\n--<error>\t SDL_Init: \n"<<SDL_GetError()<<std::endl;
        return -1;
    }
    atexit(SDL_Quit);


    // *********  video initialize ****************
    // -- video data info:
    int fW= static_cast<int>(video_ch.width());
    int fH= static_cast<int>(video_ch.height());
    int fSize= static_cast<int>(video_ch.size_buff_frame());
    int fLineSize= static_cast<int>(video_ch.size_line_frame());
    int fDepth= video_ch.depth();
    int fRate= video_ch.rate_video();
    int count_frames= video_ch.total_count_frames();
    std::cout<<"\n--------- video ----------------"
            <<"\n\t"<<"["<<fW<<"x"<<fH<<"] pix\t"<<"fSize= "<<fSize<<" bytes"
           <<"\n\t"<<"fLineSize= "<<fLineSize<<" bytes\t"<<"depth= "<< fDepth<<" bits-per-pix"
          <<"\n\t"<<"fRate= "<<fRate<<"\t count_frames= "<<count_frames
         << std::endl;

    // -- the declaration and creation of SDL surfaces (for video data)
    SDL_Surface *matrix_surf= nullptr;
    SDL_Surface *screen= nullptr;
    screen=SDL_SetVideoMode( fW, fH, fDepth, SDL_DOUBLEBUF );
    if (!screen){
        std::cerr<<"\n--<error>\t SDL mode failed: "<<SDL_GetError()<<std::endl;
        return -1;
    }
    SDL_WM_SetCaption("Decoder JV", nullptr);

    // -- to decode all video frames in the video buffer
    std::vector<std::shared_ptr<uint8_t> > v_buff_frames;
    if( !video_ch.total_video_buff( v_buff_frames, &error) ){
        std::cerr<<"\n--<error>\t failed to get frames!\n"<<error<<std::endl;
        return -1;
    }
    std::cout<<"\n--\t The video buffer is loaded:\t"<<v_buff_frames.size()<<std::endl;

    // *********  audio initialize ****************
    // --------------------------------------------
    St_AData audio_data;
    if( !audio_ch.total_wav_buff(audio_data, &error) ){
        std::cerr<<"\n--<error>\t failed to get total sound!\n"<<error<<std::endl;
        return -1;
    }
    uint frq= audio_ch.frequency();  // the frequency of the JV sound file
    int playback_ms= audio_ch.calc_total_play_time_ms(); // playback sound time in milliseconds

    if( !frq || (playback_ms<=0) || (audio_data.ptr_data==nullptr)||(!audio_data.size_data)){
        std::cerr<<"\n--<error>\t the audio settings is incorrect!\n";
        return -1;
    }
    std::cout<<"\n--------- audio ----------------"
            <<"\n\t"<<"size audio data: "<<audio_data.size_data<<" bytes"
           <<"\n\t"<<"frequency= "<<frq<<" Hz"
          <<"\n\t"<<"total play- playback_ms= "<<playback_ms<<" ms"
         <<std::endl;

    // Setup SDL audio mode
    if( (Mix_OpenAudio(static_cast<int>(frq), AUDIO_U8, 1, 512) != 0 ) ){
        std::cerr<<"\n--<error>\t could not open SDL Mix Audio!\n";
        return -1;
    }
    Mix_AllocateChannels( 1 );          // only 1 chanel
    Mix_Volume( 0, MIX_MAX_VOLUME);     // work chanel number 0
    Mix_Chunk *wav_chunk(nullptr);      // for one total wav


    // *** to synchronize audio and video streams ****
    // -------------------------------------------------
    long rate_frame_ms= static_cast<long>(playback_ms*1./v_buff_frames.size());
    long cur_fps= static_cast<long>(std::floor(1000./rate_frame_ms + 0.5));
    long jv_fps= static_cast<long>(std::floor(1000./fRate + 0.5));
    std::cerr<<"\n&&&\t"<<"(rate_frame_ms * count_frames)= "<<(rate_frame_ms * count_frames)
            <<"\t [playback sound= "<<playback_ms<<" ms]"
           <<"\n&&&\t"<<"cur_fps= "<<cur_fps<<"\t jv_fps= "<<jv_fps<<"\t custom_fps= "<<custom_fps
          <<std::endl;

    long resize(0);
    if( custom_fps < 0 ){
        // -- to change the buffer size with the recommended FPS from JV file
        resize= static_cast<long>(std::floor(playback_ms*1./fRate + 0.5));
        rate_frame_ms= fRate;
    }
    else if( custom_fps > 0 ){
        // -- to change the buffer size with custom FPS
        rate_frame_ms= static_cast<long>(std::floor(1000./custom_fps + 0.5));
        resize= static_cast<long>(playback_ms*1./rate_frame_ms);
    }
    std::cerr<<"\n&&&\t"<<"current resize buff: "<<resize<<"\t current rate: "<<rate_frame_ms<<std::endl;
    if( resize != 0 )
    {
        // -- change the buffer with the new FPS
        std::vector<std::shared_ptr<uint8_t> > v_buff_resize;
        double d_step(0), sm(0);
        if( resize > count_frames )
        {
            // + expand
            d_step= count_frames*1./resize;
            for(size_t i=0; i<v_buff_frames.size(); ++i)
            {
                while (sm<1.) {
                    v_buff_resize.push_back( v_buff_frames[i] );
                    sm+=d_step;
                }
                sm-=1.;
            }
        }
        else if( resize < count_frames )
        {
            // - compress
            d_step= count_frames *1./(resize);
            size_t is= static_cast<size_t>(std::floor(d_step));
            d_step -= is;

            for(size_t i=0; i<v_buff_frames.size(); i+= is)
            {
                if( sm > 1.)    sm-=1.;
                else{
                    v_buff_resize.push_back( v_buff_frames[i] );
                    sm += d_step;
                }
            }
        }
        v_buff_resize.swap( v_buff_frames );
        count_frames= static_cast<int>(v_buff_frames.size());
    }
    std::cerr<<"\n&&&\t"<<"current count frames in buffer:\t "<<count_frames<<std::endl;

    auto all_timing= std::chrono::high_resolution_clock::now();
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // Output processing
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    std::shared_ptr<uint8_t> ptr_frame(nullptr);
    long df_rate_ms(0);
    long cur_delay(0);
    bool quit = false;
    SDL_Event event;
    int ktp(0), debug_out(0);
    for( size_t i=0; (i<v_buff_frames.size())&& !quit; ++i )
    {
        SDL_PollEvent( &event );
        // ---- video frame ----------
        ptr_frame= v_buff_frames[i];
        if( ptr_frame !=nullptr )
        {
            matrix_surf= SDL_CreateRGBSurfaceFrom(
                        ptr_frame.get(),
                        fW, fH,
                        fDepth, fLineSize,
                        rmask, gmask, bmask, amask
                        );

            SDL_BlitSurface(matrix_surf, nullptr, screen, nullptr);
            SDL_FreeSurface(matrix_surf);
            SDL_Flip(screen);
            // ---------------------------------------
        }
        else{
            std::cerr<<"\n--<error>\t incorrect processing of video data!\n";
            break;
        }
        // ------ audio ------------------
        if( !Mix_Playing(0) )
        {
            wav_chunk= Mix_QuickLoad_WAV(audio_data.ptr_data.get());
            if(wav_chunk == nullptr) {
                std::cerr<<"\n--<error>\t failed to load audio data from Mix_QuickLoad_WAV: "
                        << Mix_GetError()<<std::endl;
            }
            else if(Mix_PlayChannel( 0, wav_chunk, 0 ) == -1)
            {
                std::cerr<<"\n--<error>\t failed to start playback of a sound channel [0](Mix_PlayChannel)!\n<"
                        << Mix_GetError()<<">"<<std::endl;
            }
        }
        if( event.type == SDL_KEYDOWN )
        {
            ktp= event.key.keysym.sym;
            if(ktp==SDLK_ESCAPE || ktp==SDLK_q)
                quit = true;
            else if( ktp== SDLK_SPACE){
                BMP bmp(fW,fH, ptr_frame.get());
                if( !bmp.is_empty() ){
                    std::string name_file= std::to_string(i)+"-shapshot.bmp";
                    bmp.write_matrix_to_file(name_file);
                }
            }
        }
        // synchronize video&audio
        // ---------------------------------------
        auto end = std::chrono::high_resolution_clock::now();
        auto diff_ms= std::chrono::duration_cast<std::chrono::milliseconds>( end - all_timing );

        df_rate_ms= diff_ms.count();
        cur_delay= static_cast<long>(i)*rate_frame_ms - df_rate_ms;
        if( cur_delay > 1)
        {
            // -- too fast
            SDL_Delay( static_cast<uint32_t>(cur_delay) );
            if( debug_out < 100 ){
                ++debug_out;
                auto f_secs = std::chrono::duration_cast<std::chrono::duration<float>>(diff_ms);
                std::cerr<<"\n**<"<<i<<">\t"<<"--- recomended rate:\t"<<fRate<<" ms"
                        <<"\n**\t"<<"--- calc rate:\t"<<rate_frame_ms<<" ms"
                       <<"\n**\t"<<"diff_ms.count()= "<<diff_ms.count()
                      <<"\n**\t"<<"f_secs.count()= "<<f_secs.count()
                     <<"\n**\t"<<"df_rate_ms= "<<df_rate_ms
                    <<"\n**\t"<<"cur_delay= "<<cur_delay
                   <<std::endl;
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto diff_ms= std::chrono::duration_cast<std::chrono::milliseconds>(end - all_timing);
    std::cerr<<"\n--\t Total duration:\t"<<diff_ms.count()<<" ms\n";


    // ----------------------------------------------
    SDL_FreeSurface(screen);
    Mix_FreeChunk(wav_chunk);
    Mix_CloseAudio();
    SDL_Quit();

    return 0;
}
// ===========================================================
int save_frame(VideoDecoder_JV &video_ch, std::string folder_save_frames)
{
    // Check the save data of the video clip
    // ----------------------------------------------------------
    // *** Example of saving all video frames as BMP.
    // Note:    Not used for display! Just checking.
    if( !video_ch.total_save_frames_as_bmp(folder_save_frames) ){
        std::cerr<<"\n--<ERROR>\t could not save frames to BMP files!\n"
                << folder_save_frames<< "\n" << std::endl;
        return -1;
    }
    std::cout<<"\n++\t"<<"Test save successful. All video frames are decoded to the folder:"
            <<"\n\t"<< folder_save_frames << std::endl;
    return 0;
}
// ===========================================================
int save_wav(AudioWorker_JV &audio_ch, std::string name_wav_file)
{
    // *** An example of saving all sounds of a JV file to a single WAV file.
    // Note:    Not used to output the sound in the stream! Just checking.
    std::string error;
    if( !audio_ch.total_save_as( name_wav_file, &error ) )
    {
        std::cerr<<"\n--<ERROR>\t failed to record a test sound file!!\n"
                << error<<std::endl;
        return -1;
    }
    std::cout<<"\n++\t"<<"Test save successful. WAV file:\t"<< name_wav_file << std::endl;
    return 0;
}
// ===========================================================
std::string comand_line_help()
{
    std::stringstream os;
    os<< "\n=====================  help  ==========================="
      << "\n Command line arguments:"
      << "\n --help  This help"
      << "\n --file  Path to an existing video file JV"
      << "\n --fps   Frame-per-second what you want to get (5..100)"
      << "\n --save  Path to an existing folder for saving frames"
      << "\n --wav   To save the sound in WAV file"
      << "\n --show  Plays the video in the window (true) or no (false)"
      << "\n\n Example comand line params for app:"
      << "\n --file=/home/my_Cuts/video.JV --fps=25 --save=/home/my_frames --wav=my.wav \n"
      << "\n *** Notes ****************************"
      << "\n If there are no options --save or --wav, the corresponding files will not be saved."
      << "\n If the --file parameter is missing, the default path will be used."
      << "\n If there is --show=false, the parameters --save or --wav must be set"
      << "\n If the --show parameter is missing, use by default --show=true"
      << "\n If the --fps parameter is missing, the value will be calculated by the frameRate in file."
      << "\n Regarding FPS-the real value depends on your hardware."
      << "\n If the command line is empty, the default settings will be used:"
      << "\n--file\t"<<path_to_file
      << "\n--fps\t"<<"calculated by the frameRate in file"
      << "\n--save\t"<<"empty"
      << "\n--wav\t"<<"empty"
      << "\n--show=true"
      << "\n---------------------------------------"
      << "\n\n Control keys, press with playback:"
      << "\n Esc or \"q\" \t- quit"
      << "\n Space        \t- save shapshot"
      << std::endl;

    return os.str();
}
// ===========================================================
int parser_comand_line(int argc, char* argv[], St_Params &params)
{
    int res(0);
    // -- current folder app
    char c_folder_path[FILENAME_MAX];
    getcwd(c_folder_path, FILENAME_MAX);
    folder_root= std::string(c_folder_path);
    folder_cuts= folder_root + folder_cuts;
    folder_frames= folder_root + folder_frames;
    path_to_file= folder_root + path_to_file;
    std::string str_help;
    std::vector<std::string> v_prms({"--help","--file","--fps","--save","--wav","--show"});
    if( argc > 1 )
    {
        std::string str_prm;
        std::string::size_type pos;
        for( int i=1; (i<argc) && (res==0); ++i )
        {
            str_prm= std::string(argv[i]);
            for( size_t j=0; j<v_prms.size(); ++j ){
                if( str_prm.find(v_prms[j]) != std::string::npos ){
                    if( v_prms[j]== "--help"){
                        str_help= comand_line_help();
                        break;
                    }
                    else if(v_prms[j]== "--file"){
                        pos=v_prms[j].length()+1;
                        params.path_to_file_jv= str_prm.substr(pos);
                        break;
                    }
                    else if(v_prms[j]== "--fps"){
                        pos=v_prms[j].length()+1;
                        std::string tmp= str_prm.substr(pos);
                        params.fps= std::stoi(tmp);
                        break;
                    }
                    else if(v_prms[j]== "--save"){
                        pos=v_prms[j].length()+1;
                        params.folder_to_save_frames= str_prm.substr(pos);
                        break;
                    }
                    else if(v_prms[j]== "--wav"){
                        pos=v_prms[j].length()+1;
                        params.name_wav_file= str_prm.substr(pos);
                        break;
                    }
                    else if(v_prms[j]== "--show"){
                        pos=v_prms[j].length()+1;
                        params.f_show= (str_prm.substr(pos)=="true")? true : false;
                        break;
                    }
                    else {
                        std::cerr<<"\n--<error>\t unknown parameter ["<<str_prm<<"]!\n";
                        if(str_help.empty()) str_help= comand_line_help();
                        std::cerr << str_help <<std::endl;
                        res= -1;
                        break;
                    }
                }
            }
        }
    }
    else {
        str_help= comand_line_help();
    }
    if( !res  ){
        if( !str_help.empty() )
            std::cout<< str_help << std::endl;
        if( params.path_to_file_jv.empty() )
            params.path_to_file_jv= path_to_file;
    }
    return res;
}


// ===========================================================
// ===========================================================
// ===========================================================


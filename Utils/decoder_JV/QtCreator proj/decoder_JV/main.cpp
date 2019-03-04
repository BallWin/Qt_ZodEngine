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


#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
// ===========================================================
// ***********************************************************
// ===========================================================
#ifdef WIN32
static std::string slh= "\\";
#else
static std::string slh= "/";
#endif

static std::string folder_root;
static std::string folder_cuts= slh+"Cuts";
static std::string folder_frames= slh+"frames";

static jv_Palette palette;


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

// ===========================================================
// ***********************************************************
// ===========================================================
int main()
{
    std::cout<<"\n************** START **************\n";

    // -- current folder app
    char c_folder_path[FILENAME_MAX];
    getcwd(c_folder_path, FILENAME_MAX);
    folder_root= std::string(c_folder_path);
    std::cout<<"\n--\t folder app:\t"<<folder_root <<std::endl;

    folder_cuts= folder_root + folder_cuts;
    folder_frames= folder_root + folder_frames;
    std::string path_file, error;

    // test cuts, example: E_AWINPL.JV, E_AWINT1.JV, E_INTRO.JV etc
    path_file= folder_cuts + slh+ "E_AWINPL.JV";



    // Init decoder
    // ----------------------------------------------------------
    // - load selected cut JV
    JV_File jv_file(path_file);

    // - init video decoder JV
    VideoDecoder_JV video_ch(&jv_file);

    // - init audio worker JV
    AudioWorker_JV  audio_ch(&jv_file);



    // Check the save data of the video clip
    // ----------------------------------------------------------
    // *** Example of saving all video frames as BMP.
    // Note:    Not used for display! Just checking.
    std::string folder_save_frames= folder_frames+slh;
    if( !video_ch.total_save_frames_as_bmp(folder_save_frames) ){
        std::cerr<<"\n--<ERROR>\t could not save frames to BMP files!\n"
                << folder_save_frames<< "\n" << std::endl;
        return -1;
    }
    std::cout<<"\n++\t"<<"Test save successful. All video frames are decoded to the folder:"
            <<"\n\t"<< folder_save_frames << std::endl;


    // *** An example of saving all sounds of a JV file to a single WAV file.
    // Note:    Not used to output the sound in the stream! Just checking.
    std::string name_sound_file= "TEST.wav";
    if( !audio_ch.total_save_as( name_sound_file, &error ) )
    {
        std::cerr<<"\n--<ERROR>\t failed to record a test sound file!!\n"
                << error<<std::endl;
        return -1;
    }
    std::cout<<"\n++\t"<<"Test save successful. WAV file:\t"<<  name_sound_file << std::endl;



    // Processing and display of JV video from SDL
    // -----------------------------------------------------------
    if (SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        std::cerr<<"\n--<error>\t SDL_Init: \n"<<SDL_GetError()<<std::endl;
        return 1;
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
    std::cout<<"\n--------- video ----------------"
            <<"\n\t"<<"["<<fW<<"x"<<fH<<"] pix\t"<<"fSize= "<<fSize<<" bytes"
           <<"\n\t"<<"fLineSize= "<<fLineSize<<" bytes\t"<<"depth= "<< fDepth<<" bits-per-pix"
          <<"\n\t"<<"fRate= "<<fRate
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


    // *********  audio initialize ****************
    // --------------------------------------------
    St_AData audio_data;
    if( audio_ch.total_wav_buff(audio_data, &error) ){
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
          <<"\n\t"<<"total_play= "<<playback_ms<<" ms"
         <<std::endl;


    // Setup SDL audio mode
    if( (Mix_OpenAudio(static_cast<int>(frq), AUDIO_U8, 1, 2048) != 0 ) ){
        std::cerr<<"\n--<error>\t could not open SDL Mix Audio!\n";
        return -1;
    }
    Mix_AllocateChannels( 1 );          // only 1 chanel
    Mix_Volume( 0, MIX_MAX_VOLUME);     // work chanel number 0
    Mix_Chunk *wav_chunk(nullptr);      // for one total wav


    // *** to synchronize audio and video streams ****
    long rate_frame_ms= static_cast<long>(std::floor(playback_ms*1./v_buff_frames.size() +0.5));
    auto start_ms= std::chrono::high_resolution_clock::now();
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // Output processing
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    std::shared_ptr<uint8_t> ptr_frame(nullptr);
    long df_rate_ms(0);
    uint cur_delay(0);
    for( size_t i=0; i<v_buff_frames.size(); ++i )
    {
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

            // synchronize video&audio
            // ---------------------------------------
            auto end = std::chrono::high_resolution_clock::now();
            auto diff_ms= std::chrono::duration_cast<std::chrono::milliseconds>(end - start_ms);
            auto f_secs = std::chrono::duration_cast<std::chrono::duration<float>>(diff_ms);
            start_ms= end;
            df_rate_ms= diff_ms.count();
            if( df_rate_ms <  rate_frame_ms ){
                cur_delay= static_cast<uint>(rate_frame_ms - df_rate_ms);
                SDL_Delay( cur_delay );
                std::cerr<<"\n**\t"<<"--- recomended rate:\t"<<fRate<<" ms"
                        <<"\n**\t"<<"--- calc rate:\t"<<rate_frame_ms<<" ms"
                        <<"\n**\t"<<"diff_ms.count()= "<<diff_ms.count()
                         <<"\n**\t"<<"f_secs.count()= "<<f_secs.count()
                        <<"\n**\t"<<"df_rate_ms= "<<df_rate_ms
                          <<"\n**\t"<<"cur_delay= "<<cur_delay
                         <<std::endl;
            }
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
    }

    SDL_FreeSurface(screen);
    Mix_FreeChunk(wav_chunk);
    Mix_CloseAudio();
    SDL_Quit();

    std::cout<<"\n\n%%%%%%%%%%%%%%%% END  %%%%%%%%%%%%%%%%%%%%%%%%\n";
    return EXIT_SUCCESS;
}
// ===========================================================
// ***********************************************************
// ===========================================================

// ===========================================================
// ===========================================================

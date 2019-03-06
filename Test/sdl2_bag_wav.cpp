#include "general_head.h"

#include "video_proc.h"
#include "audio_proc.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
// ===========================================================
// ***********************************************************
// ===========================================================



// ===========================================================
// ***********************************************************
// ===========================================================
int main(int argc, char* argv[])
{
    std::cout<<"\n************** START **************\n";
    uint32_t amask = 0;                  // no alpha! (24 bit-per-pix)  //0xff000000;
    uint32_t rmask = 0x00ff0000;
    uint32_t gmask = 0x0000ff00;
    uint32_t bmask = 0x000000ff;



    JV_File jv_file("E_AWINPL.JV");

    VideoDecoder_JV video_ch(&jv_file);

    // *********  video initialize ****************
    // -- video data info:
    int fW = static_cast<int>(video_ch.width());
    int fH = static_cast<int>(video_ch.height());
    int fSize = static_cast<int>(video_ch.size_buff_frame());
    int fLineSize = static_cast<int>(video_ch.size_line_frame());
    int fDepth = video_ch.depth();
    int fRate = video_ch.rate_video();

    if (SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        std::cerr<<"\n--<error>\t SDL_Init: \n"<<SDL_GetError()<<std::endl;
        return -1;
    }



    // -- the declaration and creation of SDL surfaces (for video data)
    SDL_Surface *matrix_surf = nullptr;

    // -- to decode all video frames in the video buffer
    std::vector<std::shared_ptr<uint8_t> > v_buff_frames;
    std::string error;


    AudioWorker_JV  m_audio;
    if( !m_audio.init(&jv_file, AudioWorker_JV::MODE_WAV, &error) )
    {
        std::cerr<<"\n--<error>\t AudioWorker_JV not initialize!\n"<<SDL_GetError()<<std::endl;
        return -1;
    }

    uint frq = m_audio.frequency();
    int playback_ms= m_audio.calc_total_play_time_ms();

    std::cout<<"\n--\t"<<"frq= "<<frq<<"\t playback_ms= "<<playback_ms<<std::endl;

    //SDL_SetVideoMode(fW, fH, fDepth, SDL_DOUBLEBUF);
    SDL_CreateWindow("My Game Window",
                     SDL_WINDOWPOS_UNDEFINED,
                     SDL_WINDOWPOS_UNDEFINED,
                     640, 480, NULL);

    // Setup SDL audio mode
    if ((Mix_OpenAudio(frq, AUDIO_U8, 1, 4096) != 0)) {
        std::cerr << "\n--<error>\t could not open SDL Mix Audio!\n";
        return -1;
    }

    Mix_Volume(-1, MIX_MAX_VOLUME/2);

    St_AData m_audio_data;

    Mix_Chunk *m_wav_chunk;


    if(!m_audio.total_wav_buff(m_audio_data, &error))
    {
        //Failure
        std::cerr << "\n--<error>\t total_wav_buff()  Failure!\n"
                  << error << std::endl;
        return -1;
    }


    m_wav_chunk = Mix_QuickLoad_WAV(m_audio_data.ptr_data.get());
    if (!m_wav_chunk)
    {
        //Failure
        std::cerr<<"\n--<error>\t m_wav_chunk == NULL Mix_QuickLoad_WAV Failure!\n"
                <<std::endl;
        return -1;
    }

    if (Mix_PlayChannel(0, m_wav_chunk, 0) == -1)
    {
        //Failure
        std::cerr<<"\n--<error>\t Mix_PlayChannel Failure!\n"<<std::endl;
        return -1;
    }

    SDL_Delay(5000);


    SDL_Quit();


    std::cout<<"\n************** END **************\n";
    return EXIT_SUCCESS;
}
// ===========================================================

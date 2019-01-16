#include <stdio.h>
#include <iostream>
#include <chrono>
#include <thread>

#ifdef WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif


#include <SDL/SDL.h>


extern "C"{
#include <libavformat/avio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

#include <libavutil/opt.h>
#include <libavutil/file.h>
#include <libavutil/timestamp.h>
#include <libavutil/mathematics.h>
}


static bool flag_DEBUG_MODE= false;

static std::string default_folder_data= "/../../Data";

static char err_buf[AV_ERROR_MAX_STRING_SIZE];
std::string get_string_ffmpeg_error(int ernum ){
    memset(err_buf, '\0', AV_ERROR_MAX_STRING_SIZE);
    if( av_strerror(ernum, err_buf, AV_ERROR_MAX_STRING_SIZE) < 0 ){
        return "\n--<error>\t Unknown error in ffmpeg (avcodec|avformat)!\n";
    }
    return err_buf;
}
void sleepcp(int milliseconds);

void sleepcp(int milliseconds) // Cross-platform sleep function
{
    #ifdef WIN32
        Sleep(milliseconds);
    #else
        usleep(milliseconds * 1000);
    #endif // win32
}
// ===================================================

int main(int argc, char* argv[])
{

    std::cout<<"\n************** START **************"
            <<"\n\t"<<"ffmpeg avcodec vers:\t"<<av_version_info()
           <<"\n*************************************\n";

    // ** Get path to selected video file
    // -------------------------------------------------------------
    // -- current folder app
    char c_folder_path[FILENAME_MAX];
    getcwd(c_folder_path, FILENAME_MAX);
    std::string path_to_video_file(c_folder_path);
    std::cout<<"\n--\t folder app:\t"<<path_to_video_file <<std::endl;

    if(!flag_DEBUG_MODE){
        if (argc < 2) {
            printf("\n=====================  help  ===========================\n"
                   "Usage: [%s] \n-use relative path by app folder to video file.\n"
                   "This is test for files \".JV\" however, you can try any other video files\n"
                   "Example: (linux) /../../Data/video.JV "
                   "(windows) \\..\\..\\Data\\video.JV \n"
                   "\n=========================================================\n"
                   , argv[0]);
            return 0;
        }
        path_to_video_file.append(argv[1]);
    }
    else{
        // <debug> selected file to load
        std::string name_file= "/E_INTRO.JV";      // E_ALOST1.JV 7866
        path_to_video_file.append(default_folder_data);
        path_to_video_file.append(name_file);
    }
    std::cout<<"\n**\t input video file: "<<path_to_video_file<<std::endl;


    // -------------------------------------------------------------
    // ** Init SDL, ffmpeg::avcodec(s) *****************************
    // -------------------------------------------------------------

    // Register all available file formats and codecs
    av_register_all();

    int err;
    // Init SDL with video support
    err = SDL_Init(SDL_INIT_VIDEO);
    if (err < 0) {
        std::cerr<<"\n--<error> No init SDL: \t"<< SDL_GetError()<<std::endl;
        return -1;
    }



    // Open video file by ffmpeg
    // -------------------------------------------------------------
    const char* filename = path_to_video_file.c_str();
    AVFormatContext* format_context = nullptr;
    err = avformat_open_input(&format_context, filename, nullptr, nullptr);
    if (err < 0) {
        std::cerr<<"\n--<error> ffmpeg: Unable to open input file\n"
                << get_string_ffmpeg_error(err) << std::endl;
        return -1;
    }

    // Retrieve stream information
    err = avformat_find_stream_info(format_context, nullptr);
    if (err < 0) {
        std::cerr<<"\n--<error> ffmpeg: Unable to find stream info\n"
                << get_string_ffmpeg_error(err) << std::endl;
        return -1;
    }

    // Dump information about file onto standard error
    av_dump_format(format_context, 0, argv[1], 0);

    // Find the first video stream
    uint video_stream(0);
    for (video_stream = 0; video_stream < format_context->nb_streams; ++video_stream) {
        if (format_context->streams[video_stream]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            break; // we found
        }
    }
    if (video_stream == format_context->nb_streams) {
        std::cerr<<"\n--<error> ffmpeg: nable to find video stream\n" << std::endl;
        return -1;
    }

    // Get features video stream
    AVCodecParameters *codec_params = nullptr;
    AVCodec* codec                  = nullptr;
    AVCodecContext* codec_context   = nullptr;

    codec_params = format_context->streams[video_stream]->codecpar;
    codec = avcodec_find_decoder(codec_params->codec_id);
    if( !codec ){
        std::cerr<<"\n--<error> ffmpeg: no find decoder\n" << std::endl;
        return -1;
    }
    codec_context = avcodec_alloc_context3(codec);

    err= avcodec_parameters_to_context(codec_context, codec_params);
    if( err < 0 ){
        std::cerr<<"\n--<error> ffmpeg: No making context by params\n"
                << get_string_ffmpeg_error(err) << std::endl;
        return -1;
    }

    // Open this video codec
    err = avcodec_open2(codec_context, codec, nullptr);
    if (err < 0) {
        std::cerr<<"\n--<error> ffmpeg: Unable to open codec\n"
                << get_string_ffmpeg_error(err) << std::endl;
        return -1;
    }


    // Make SDL surfaces
    // -------------------------------------------------------------
    SDL_Surface* screen = SDL_SetVideoMode(codec_context->width, codec_context->height, 0, 0);
    if (screen == nullptr) {
        std::cerr<<"\n--<error> SDL: Couldn't set video mode\n";
        return -1;
    }

    SDL_Overlay* img = SDL_CreateYUVOverlay(
                codec_context->width,
                codec_context->height,
                SDL_YV12_OVERLAY,
                screen);

    /// --- Reference by SDL Overlay keys (formats)
    //      SDL_YV12_OVERLAY  0x32315659	/**< Planar mode: Y + V + U  (3 planes) */
    //      SDL_IYUV_OVERLAY  0x56555949	/**< Planar mode: Y + U + V  (3 planes) */
    //      SDL_YUY2_OVERLAY  0x32595559	/**< Packed mode: Y0+U0+Y1+V0 (1 plane) */
    //      SDL_UYVY_OVERLAY  0x59565955	/**< Packed mode: U0+Y0+V0+Y1 (1 plane) */
    //      SDL_YVYU_OVERLAY  0x55595659	/**< Packed mode: Y0+V0+Y1+U0 (1 plane) */
    // ------------------------------



    // Make ffmpeg converter (any of the SDL format list)
    // -------------------------------------------------------------
    AVFrame* p_frame_in= nullptr;

    struct SwsContext* img_convert_context= nullptr;
    img_convert_context = sws_getCachedContext(
                nullptr,
                codec_context->width,
                codec_context->height,
                codec_context->pix_fmt,
                codec_context->width,
                codec_context->height,
                AV_PIX_FMT_YUV420P,
                SWS_BICUBIC,
                nullptr, nullptr, nullptr);


/// --- Reference by some ffmpeg codec formats keys (see the full list in ffmpeg ) - different effect (funny %)
///  any of these suits us [ 4:2:0 ]
//            AV_PIX_FMT_YUV420P,   ///< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
//            AV_PIX_FMT_YUYV422,   ///< packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
//            AV_PIX_FMT_YUV422P,   ///< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
//            AV_PIX_FMT_YUV444P,   ///< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
//            AV_PIX_FMT_YUV410P,   ///< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
//            AV_PIX_FMT_YUV411P,   ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
//            AV_PIX_FMT_YUVJ420P,  ///< planar YUV 4:2:0, 12bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV420P and setting color_range
//            AV_PIX_FMT_YUVJ422P,  ///< planar YUV 4:2:2, 16bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV422P and setting color_range
//            AV_PIX_FMT_YUVJ444P,  ///< planar YUV 4:4:4, 24bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV444P and setting color_range
//            AV_PIX_FMT_YUV440P,   ///< planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples)
//            AV_PIX_FMT_YUVJ440P,  ///< planar YUV 4:4:0 full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV440P and setting color_range
//            AV_PIX_FMT_YUV420P16LE,  ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
//            AV_PIX_FMT_YUV420P16BE,  ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
//            AV_PIX_FMT_YUV422P16LE,  ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
//            AV_PIX_FMT_YUV422P16BE,  ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
//            AV_PIX_FMT_YUV444P16LE,  ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
//            AV_PIX_FMT_YUV444P16BE,  ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian


    if (img_convert_context == nullptr) {
        std::cerr<<"\n--<error> ffmpeg: Cannot initialize the conversion context\n";
        return -1;
    }

    p_frame_in = av_frame_alloc();
    if( p_frame_in == nullptr ){
        std::cerr<<"\n--<error> ffmpeg: Cannot memory allocated for frame input\n";
        return -1;
    }


    AVPacket packet= {};
    int N(0);
    std::string str_title;
    while (av_read_frame(format_context, &packet) >= 0)
    {

        if (packet.stream_index == static_cast<int>(video_stream) )
        {
            // Video stream packet
            // Use avcodec_send_packet() and avcodec_receive_frame().
            int frame_finished;

            avcodec_decode_video2(codec_context, p_frame_in, &frame_finished, &packet);

            if (frame_finished) {
                SDL_LockYUVOverlay(img);

                // Convert frame to YV12 pixel format for display in SDL overlay
                // use AVFrame or imgutils functions instead
                AVPicture pict;
                pict.data[0] = img->pixels[0];
                pict.data[1] = img->pixels[2];  // it's because YV12
                pict.data[2] = img->pixels[1];

                pict.linesize[0] = img->pitches[0];
                pict.linesize[1] = img->pitches[2];
                pict.linesize[2] = img->pitches[1];

                sws_scale(img_convert_context,
                          p_frame_in->data, p_frame_in->linesize,
                          0, codec_context->height,
                          pict.data, pict.linesize);

                SDL_UnlockYUVOverlay(img);

                SDL_Rect rect;
                rect.x = 0;
                rect.y = 0;
                rect.w = codec_context->width;
                rect.h = codec_context->height;
                SDL_DisplayYUVOverlay(img, &rect);
                str_title= "frame "+std::to_string((++N));
                SDL_WM_SetCaption(str_title.c_str(), nullptr);
            }


        }

        // Free the packet that was allocated by av_read_frame
        av_packet_unref(&packet);

        // Handling SDL events there
        SDL_Event event;
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                break;
            }
        }

        sleepcp(20);
    }

    sws_freeContext(img_convert_context);

    // Free the YUV frame
    av_free(p_frame_in);

    // Close the codec
    avcodec_close(codec_context);

    // Close the video file
    avformat_close_input(&format_context);

    // Quit SDL
    SDL_Quit();


    std::cout<<"\n\n************** END **************\n";
    return 0;
}


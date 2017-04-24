//========================================================================================================================================================================================================================
// DEMO 100.3: OpenGL Video Player based on SDL
//========================================================================================================================================================================================================================
extern "C" 
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/avstring.h>
    #include <libswscale/swscale.h>
    #include <SDL/SDL.h>
    #include <SDL/SDL_thread.h>
}

#include <thread>
#include <mutex>
#include <condition_variable>

#ifdef __MINGW32__
#undef main 
#endif

#include <cstdio>
#include <cassert>
#include <cmath>

#include "log.hpp"

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

#define MAX_AUDIOQ_SIZE (5 * 16 * 1024)
#define MAX_VIDEOQ_SIZE (5 * 256 * 1024)

#define FF_REFRESH_EVENT (SDL_USEREVENT)
#define FF_QUIT_EVENT (SDL_USEREVENT + 1)

#define VIDEO_PICTURE_QUEUE_SIZE 1

typedef struct PacketQueue
{
    AVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    std::mutex mutex;
    std::condition_variable condition_variable;
} PacketQueue;


typedef struct VideoPicture
{
    SDL_Overlay *bmp;
    int width, height;                                                                                      // source height & width
    int allocated;
} VideoPicture;

typedef struct VideoState
{
    AVFormatContext *pFormatCtx;
    int             videoStream, audioStream;
    AVStream        *audio_st;
    AVCodecContext  *audio_ctx;
    PacketQueue     audioq;
    uint8_t         audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
    unsigned int    audio_buf_size;
    unsigned int    audio_buf_index;
    AVFrame         audio_frame;
    AVPacket        audio_pkt;
    uint8_t         *audio_pkt_data;
    int             audio_pkt_size;
    AVStream        *video_st;
    AVCodecContext  *video_ctx;
    PacketQueue     videoq;
    struct SwsContext *sws_ctx;
    
    VideoPicture    pictq[VIDEO_PICTURE_QUEUE_SIZE];
    int             pictq_size, pictq_rindex, pictq_windex;
    std::mutex               pictq_mutex;
    std::condition_variable  pictq_cond;
    
    std::thread     parse_tid;
    std::thread     video_tid;
    
    char            filename[1024];
    int             quit;
} VideoState;

SDL_Surface *screen;
std::mutex screen_mutex;
VideoState *global_video_state;                                                                             // Since we only have one decoding thread, the Big Struct can be global

void packet_queue_init(PacketQueue *q)
{
    memset(q, 0, sizeof(PacketQueue));
    //q->mutex = SDL_CreateMutex();
    //q->cond = SDL_CreateCond();
}

int packet_queue_put(PacketQueue *q, AVPacket *pkt)
{
    if(av_dup_packet(pkt) < 0) return -1;
    AVPacketList *pkt1 = (AVPacketList*) av_malloc(sizeof(AVPacketList));
    if (!pkt1) return -1;
    pkt1->pkt = *pkt;
    pkt1->next = NULL;
    
    q->mutex.lock(); //    SDL_LockMutex(q->mutex);
    
    if (!q->last_pkt)
        q->first_pkt = pkt1;
    else
        q->last_pkt->next = pkt1;
    q->last_pkt = pkt1;
    q->nb_packets++;
    q->size += pkt1->pkt.size;
    q->mutex.unlock();
    q->condition_variable.notify_one(); //SDL_CondSignal(q->cond);
    
    return 0;
}

static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
{
    AVPacketList *pkt1;
    int ret = -1;

    debug_msg("packet_queue_get :: before locking mutex");
    std::unique_lock<std::mutex> lock(q->mutex);
    debug_msg("packet_queue_get :: after locking mutex");
    //q->mutex.lock(); //SDL_LockMutex(q->mutex);
  
    while(1)
    {
        if(global_video_state->quit) break;

        pkt1 = q->first_pkt;
        if (pkt1)
        {
            q->first_pkt = pkt1->next;
            if (!q->first_pkt)
                q->last_pkt = NULL;
            q->nb_packets--;
            q->size -= pkt1->pkt.size;
            *pkt = pkt1->pkt;
            av_free(pkt1);
            ret = 1;
            break;
        }
        else if (!block)
        {
            ret = 0;
            break;
        }
        else
        {
            //SDL_CondWait(q->cond, q->mutex);
            debug_msg("packet_queue_get :: before condition_variable wait");
            q->condition_variable.wait(lock);
            debug_msg("packet_queue_get :: after condition_variable wait");
        }
    }
    // SDL_UnlockMutex(q->mutex); unlocked in destructor of lock
    return ret;
}

int audio_decode_frame(VideoState *is, uint8_t *audio_buf, int buf_size)
{
    AVPacket *pkt = &is->audio_pkt;
    while(1)
    {
        while(is->audio_pkt_size > 0)
        {
            int got_frame = 0;
            int len1 = avcodec_decode_audio4(is->audio_ctx, &is->audio_frame, &got_frame, pkt);
            if(len1 < 0)
            {
                is->audio_pkt_size = 0;                                                                     // if error, skip frame
                break;
            }
            int data_size = 0;
            if(got_frame)
            {
                data_size = av_samples_get_buffer_size(NULL, is->audio_ctx->channels, is->audio_frame.nb_samples, is->audio_ctx->sample_fmt, 1);
                assert(data_size <= buf_size);
                memcpy(audio_buf, is->audio_frame.data[0], data_size);
            }
            is->audio_pkt_data += len1;
            is->audio_pkt_size -= len1;
            if(data_size <= 0)                                                                              // No data yet, get more frames
                continue;
            
            return data_size;                                                                               // We have data, return it and come back for more later
        }
        if(pkt->data)
            av_free_packet(pkt);

        if(is->quit)
            return -1;
        
        if(packet_queue_get(&is->audioq, pkt, 1) < 0)                                                       // next packet
            return -1;
        is->audio_pkt_data = pkt->data;
        is->audio_pkt_size = pkt->size;
    }
}

void audio_callback(void *userdata, Uint8 *stream, int len)
{
    debug_msg("Audio callback invoked. len = %d", len);
    VideoState *is = (VideoState *)userdata;
    while(len > 0)
    {
        if(is->audio_buf_index >= is->audio_buf_size)
        {
            int audio_size = audio_decode_frame(is, is->audio_buf, sizeof(is->audio_buf));                  // We have already sent all our data; get more
            if(audio_size < 0)
            {
                
                is->audio_buf_size = 1024;                                                                  // If error, output silence
                memset(is->audio_buf, 0, is->audio_buf_size);
            }
            else
                is->audio_buf_size = audio_size;
            is->audio_buf_index = 0;
        }
        int len1 = is->audio_buf_size - is->audio_buf_index;
        if(len1 > len)
            len1 = len;
        memcpy(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, len1);
        len -= len1;
        stream += len1;
        is->audio_buf_index += len1;
    }
    debug_msg("Audio callback done.");
}

static Uint32 sdl_refresh_timer_cb(Uint32 interval, void *opaque)
{
    SDL_Event event;
    event.type = FF_REFRESH_EVENT;
    event.user.data1 = opaque;
    SDL_PushEvent(&event);
    return 0;                                                                                               // 0 means stop timer
}

static void schedule_refresh(VideoState *is, int delay)                                                     // schedule a video refresh in 'delay' ms
{
    SDL_AddTimer(delay, sdl_refresh_timer_cb, is);
}

void video_display(VideoState *is)
{
    VideoPicture *vp = &is->pictq[is->pictq_rindex];
    if(vp->bmp)
    {
        float aspect_ratio = (is->video_ctx->sample_aspect_ratio.num == 0) ?
                0.0f : av_q2d(is->video_ctx->sample_aspect_ratio) * is->video_ctx->width / is->video_ctx->height;
        if(aspect_ratio <= 0.0)
            aspect_ratio = (float)is->video_ctx->width / (float)is->video_ctx->height;

        SDL_Rect rect;

        rect.h = screen->h;
        rect.w = ((int)rint(rect.h * aspect_ratio)) & -3;
        if(rect.w > screen->w)
        {
            rect.w = screen->w;
            rect.h = ((int)rint(rect.w / aspect_ratio)) & -3;
        }
        rect.x = (screen->w - rect.w) / 2;
        rect.y = (screen->h - rect.h) / 2;
        //SDL_LockMutex(screen_mutex);
        screen_mutex.lock();
        SDL_DisplayYUVOverlay(vp->bmp, &rect);
//        SDL_UnlockMutex(screen_mutex);
        screen_mutex.unlock();
    }
}

void video_refresh_timer(void *userdata)
{
    VideoState *is = (VideoState *)userdata;
  
    if(is->video_st)
    {
        if(is->pictq_size == 0)
            schedule_refresh(is, 1);
        else
        {
            VideoPicture *vp = &is->pictq[is->pictq_rindex];
            schedule_refresh(is, 40);
            video_display(is);                                                                              // show the picture!
            if(++is->pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE)                                              // update queue for next picture!
                is->pictq_rindex = 0;
                        
//            SDL_LockMutex(is->pictq_mutex);
            is->pictq_mutex.lock();
            is->pictq_size--;
//          SDL_CondSignal(is->pictq_cond);
            is->pictq_cond.notify_one();
//            SDL_UnlockMutex(is->pictq_mutex);
            is->pictq_mutex.unlock();
        }
    }
    else
        schedule_refresh(is, 100);
}
      
void alloc_picture(void *userdata)
{
    VideoState *is = (VideoState*) userdata;
    VideoPicture *vp = &is->pictq[is->pictq_windex];
    if(vp->bmp)
        SDL_FreeYUVOverlay(vp->bmp);                                                                        // we already have one make another, bigger/smaller
    
//    SDL_LockMutex(screen_mutex);                                                                            // Allocate a place to put our YUV image on that screen
    screen_mutex.lock();                                                                            // Allocate a place to put our YUV image on that screen
    vp->bmp = SDL_CreateYUVOverlay(is->video_ctx->width, is->video_ctx->height, SDL_YV12_OVERLAY, screen);
//    SDL_UnlockMutex(screen_mutex);
    screen_mutex.unlock();

    vp->width = is->video_ctx->width;
    vp->height = is->video_ctx->height;
    vp->allocated = 1;
}

int queue_picture(VideoState *is, AVFrame *pFrame)
{
    //SDL_LockMutex(is->pictq_mutex);                                                                         // wait until we have space for a new pic

{
    std::unique_lock<std::mutex> lock(is->pictq_mutex);

//    is->pictq_mutex.lock();                                                                                 // wait until we have space for a new pic

    while(is->pictq_size >= VIDEO_PICTURE_QUEUE_SIZE && !is->quit)
    {
        is->pictq_cond.wait(lock);
        //SDL_CondWait(is->pictq_cond, is->pictq_mutex);
    }
    //SDL_UnlockMutex(is->pictq_mutex);

}

    if(is->quit) return -1;
    VideoPicture *vp = &is->pictq[is->pictq_windex];                                                        // windex is set to 0 initially
    
    if(!vp->bmp || vp->width != is->video_ctx->width || vp->height != is->video_ctx->height)                // allocate or resize the buffer!
    {
        SDL_Event event;
        vp->allocated = 0;
        alloc_picture(is);
        if(is->quit)
            return -1;
    }
    
    if(vp->bmp)                                                                                             // We have a place to put our picture on the queue
    {
        AVPicture pict;
        SDL_LockYUVOverlay(vp->bmp);
        int dst_pix_fmt = AV_PIX_FMT_YUV420P;                                                               // point pict at the queue

        pict.data[0] = vp->bmp->pixels[0]; pict.linesize[0] = vp->bmp->pitches[0];
        pict.data[1] = vp->bmp->pixels[2]; pict.linesize[1] = vp->bmp->pitches[2];
        pict.data[2] = vp->bmp->pixels[1]; pict.linesize[2] = vp->bmp->pitches[1];
        
        sws_scale(is->sws_ctx, (uint8_t const * const *)pFrame->data,                                       // Convert the image into YUV format that SDL uses
              pFrame->linesize, 0, is->video_ctx->height, pict.data, pict.linesize);
        
        SDL_UnlockYUVOverlay(vp->bmp);
        
        if(++is->pictq_windex == VIDEO_PICTURE_QUEUE_SIZE)                                                  // now we inform our display thread that we have a pic ready
            is->pictq_windex = 0;
        
        //SDL_LockMutex(is->pictq_mutex);
        is->pictq_mutex.lock();
        is->pictq_size++;
        is->pictq_mutex.unlock();
//        SDL_UnlockMutex(is->pictq_mutex);
    }
    return 0;
}

int video_thread(VideoState *is)
{
//    VideoState *is = (VideoState *)arg;
    debug_msg("Video thread started ...");
    AVPacket pkt1, *packet = &pkt1;
    AVFrame *pFrame = av_frame_alloc();
    while(1)
    {
        debug_msg("Video thread loop ... 1");
        int frameFinished;
        if(packet_queue_get(&is->videoq, packet, 1) < 0) break;                                             // means we quit getting packets
        debug_msg("Video thread loop ... 2");

        avcodec_decode_video2(is->video_ctx, pFrame, &frameFinished, packet);                               // Decode video frame
        debug_msg("Video thread loop ... 3");

        if((frameFinished) && (queue_picture(is, pFrame) < 0)) break;                                         // Did we get a video frame?
        av_free_packet(packet);
    }
    av_frame_free(&pFrame);
    debug_msg("Video thread exiting ...");
    return 0;
}

int stream_component_open(VideoState *is, int stream_index)
{
    debug_msg("stream_component_open :: stream index = %d. begin", stream_index);
    AVFormatContext *pFormatCtx = is->pFormatCtx;

    if(stream_index < 0 || stream_index >= pFormatCtx->nb_streams)
    {
        debug_msg("Invalid stream index = %d. pFormatCtx->nb_streams = %d.\n", stream_index, pFormatCtx->nb_streams);
        return -1;
    }

    AVCodecParameters *pCodecParameters = pFormatCtx->streams[stream_index]->codecpar;                       // Get a pointer to the codec context for the video stream
    AVCodec *pCodec = avcodec_find_decoder(pCodecParameters->codec_id);                                     // Find the decoder for the video stream
    if(pCodec == 0)
    {
        debug_msg("Cannot find appropriate codec parameters for stream of index %d.\n", stream_index);
        return -1;
    }
    
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);                                             // Allocate an AVCodecContext and set its fields to default values. 
    if(avcodec_parameters_to_context(pCodecCtx, pCodecParameters) != 0)
    {
        debug_msg("Cannot initialize codec context with given parameters.");
        return -1;                                                                                          // Error copying codec context
    }

    if(pCodecCtx->codec_type == AVMEDIA_TYPE_AUDIO)
    {
        debug_msg("Opening AUDIO stream :: \n\tfrequency = %u.\n\tchannels = %u", pCodecCtx->sample_rate, pCodecCtx->channels);
        SDL_AudioSpec wanted_spec, spec;
        wanted_spec.freq = pCodecCtx->sample_rate;                                                           // Set audio settings from codec info
        wanted_spec.format = AUDIO_S16SYS;
        wanted_spec.channels = 2;                                                                           // codecCtx->channels;
        wanted_spec.silence = 0;
        wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
        wanted_spec.callback = audio_callback;
        wanted_spec.userdata = is;
    
        debug_msg("invoking SDL_OpenAudio");
        if(SDL_OpenAudio(&wanted_spec, &spec) < 0)
        {
            debug_msg("SDL_OpenAudio: %s\n", SDL_GetError());
            return -1;
        }
        debug_msg("SDL_OpenAudio returned");
    }

    if(avcodec_open2(pCodecCtx, pCodec, 0) < 0)
    {
        debug_msg("Unsupported codec!\n");
        return -1;
    }

    switch(pCodecCtx->codec_type)
    {
        case AVMEDIA_TYPE_AUDIO:
            is->audioStream = stream_index;
            is->audio_st = pFormatCtx->streams[stream_index];
            is->audio_ctx = pCodecCtx;
            is->audio_buf_size = 0;
            is->audio_buf_index = 0;
            memset(&is->audio_pkt, 0, sizeof(is->audio_pkt));
            packet_queue_init(&is->audioq);
            debug_msg("invoking SDL_PauseAudio(0)");
            SDL_PauseAudio(0);
            debug_msg("SDL_PauseAudio(0) returned");
            break;
        case AVMEDIA_TYPE_VIDEO:
            is->videoStream = stream_index;
            is->video_st = pFormatCtx->streams[stream_index];
            is->video_ctx = pCodecCtx;
            packet_queue_init(&is->videoq);
            debug_msg("launching video_thread");
            is->video_tid = std::thread(video_thread, is);

            is->sws_ctx = sws_getContext(is->video_ctx->width, is->video_ctx->height,
                                         is->video_ctx->pix_fmt, is->video_ctx->width, is->video_ctx->height, AV_PIX_FMT_YUV420P,
                                         SWS_BILINEAR, NULL, NULL, NULL);
            break;
        default:
            
            break;
    }
    debug_msg("stream_component_open :: stream index = %d. end", stream_index);    
}

int decode_thread(VideoState* is)
{
    debug_msg("Decoding thread started ...");
//    VideoState *is = (VideoState *)arg;
    AVFormatContext *pFormatCtx = 0;
    AVPacket pkt1, *packet = &pkt1;

    int video_index = -1;
    int audio_index = -1;
    is->videoStream = -1;
    is->audioStream = -1;

    global_video_state = is;

    debug_msg("Opening file : %s", is->filename);

    
    if(avformat_open_input(&pFormatCtx, is->filename, NULL, NULL) != 0)                                     // Open video file
    {
        debug_msg("Cannot open file : %s", is->filename);
        return -1; // Couldn't open file
    }
    debug_msg("Video file %s successfully opened", is->filename);

    is->pFormatCtx = pFormatCtx;
    
    if(avformat_find_stream_info(pFormatCtx, NULL) < 0)                                                     // Retrieve stream information
    {
        debug_msg("Couldn't find stream information");
        return -1;
    }
    
    av_dump_format(pFormatCtx, 0, is->filename, 0);                                                         // Dump information about file onto standard error

    for(int i = pFormatCtx->nb_streams - 1; i >= 0; --i)                                                      // Find the first video stream
    {
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) video_index = i;
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) audio_index = i;
    }

    debug_msg("Video strem index = %u. Audio stream index = %u", video_index, audio_index);

    if(audio_index >= 0)
        stream_component_open(is, audio_index);

    if(video_index >= 0)
        stream_component_open(is, video_index);

    if(is->videoStream < 0 || is->audioStream < 0)
    {
        debug_msg("%s: could not open codecs\n", is->filename);
        goto fail;
    }

    while(1)                                                                                                // main decode loop
    {
        if(is->quit) break;
        
        if(is->audioq.size > MAX_AUDIOQ_SIZE || is->videoq.size > MAX_VIDEOQ_SIZE)                          // seek stuff goes here
        {
            SDL_Delay(10);
            continue;
        }
        if(av_read_frame(is->pFormatCtx, packet) < 0)
        {
            if(is->pFormatCtx->pb->error == 0)
            {
                SDL_Delay(100);                                                                             // no error; wait for user input
                continue;
            }
            else break;
        }
        
        if(packet->stream_index == is->videoStream)                                                         // Is this a packet from the video stream?
            packet_queue_put(&is->videoq, packet);
        else if(packet->stream_index == is->audioStream)
            packet_queue_put(&is->audioq, packet);
        else
            av_free_packet(packet);
    }
    
    while(!is->quit)                                                                                        // all done - wait for it
        SDL_Delay(100);

  fail:
    SDL_Event event;
    event.type = FF_QUIT_EVENT;
    event.user.data1 = is;
    SDL_PushEvent(&event);
    return 0;
}

int main(int argc, char** argv)
{
    VideoState* is = (VideoState*) av_mallocz(sizeof(VideoState));

    if(argc < 2)
        exit_msg("Usage: player_3 <filename>\n");
    
    av_register_all();                                                                                      // Register all formats and codecs
  
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
        exit_msg("Could not initialize SDL :: %s\n", SDL_GetError());
    debug_msg("SDL initialized ... ");
    
    screen = SDL_SetVideoMode(1024, 768, 0, 0);                                                             // Make a screen to put our video

    if(!screen)
        exit_msg("SDL: could not set video mode - exiting\n");
    debug_msg("SDL :: video mode set.");

    //screen_mutex = SDL_CreateMutex();
    av_strlcpy(is->filename, argv[1], sizeof(is->filename));
//    is->pictq_mutex = SDL_CreateMutex();
//    is->pictq_cond = SDL_CreateCond();
    schedule_refresh(is, 40);

    debug_msg("main :: launching decoding thread ... ");
    is->parse_tid = std::thread(decode_thread, is);
    debug_msg("SDL :: main loop begin.");

    while(1)
    {
        SDL_Event event;
        SDL_WaitEvent(&event);
        switch(event.type)
        {
            case FF_QUIT_EVENT:
            case SDL_QUIT:
                is->quit = 1; SDL_Quit(); return 0; break;
            case FF_REFRESH_EVENT:
                video_refresh_timer(event.user.data1); break;
            default:
                break;
        }
    }
    return 0;
}
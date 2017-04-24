#include <cassert>
#include "decoder.hpp"

#ifndef _MSC_VER
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

extern "C" {
#include <libavutil/pixfmt.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

#include <iostream>
#include <chrono>

#include "log.hpp"

using lock_t = std::unique_lock<std::recursive_mutex>;


#ifdef MEASURENMENT
std::chrono::time_point<std::chrono::steady_clock> start;
#endif



struct av_data_t
{
    int video_stream;
    AVFormatContext* pFormatCtx;
    AVCodecContext* pCodecCtx;
    AVCodec* pCodec;
    AVFrame* pFrame;

    av_data_t() : video_stream(-1), pFormatCtx(0), pCodecCtx(0), pCodec(0), pFrame(0) {}

    void init(const char* video)
    {
        debug_msg("AvData::init::opening video input file");

        assert(avformat_open_input(&pFormatCtx, video, 0, 0) == 0);
        assert(avformat_find_stream_info(pFormatCtx, 0) == 0);

        av_dump_format(pFormatCtx, -1, video, 0);

        for (size_t i = 0; i < pFormatCtx->nb_streams; i++) 
        {
            if (pFormatCtx->streams[i]->codec->codec_type != AVMEDIA_TYPE_VIDEO) continue;
            video_stream = i;
            break;
        }

        assert(video_stream != -1);

        pCodecCtx = pFormatCtx->streams[video_stream]->codec;

        assert(pCodecCtx != nullptr);
        //pCodecCtx->active_thread_type = FF_THREAD_FRAME | FF_THREAD_SLICE; pCodecCtx->thread_type = FF_THREAD_FRAME | FF_THREAD_SLICE;

        pCodecCtx->thread_count = 0;

        assert((pCodec = avcodec_find_decoder(pCodecCtx->codec_id)) != nullptr);
        assert(avcodec_open2(pCodecCtx, pCodec, 0) == 0);
        assert((pFrame = av_frame_alloc()) != 0);

        debug_msg("av_data_t::init:: thread count           = %d", pCodecCtx->thread_count);
        debug_msg("av_data_t::init:: thread type            = %d", pCodecCtx->active_thread_type);
        debug_msg("av_data_t::init:: duration               = %f", double(pFormatCtx->duration) / 1000000.0);
        debug_msg("av_data_t::init:: frame count            = %ld", pFormatCtx->streams[video_stream]->nb_frames);
        debug_msg("av_data_t::init:: frame rate calculated  = %f", double(pFormatCtx->streams[video_stream]->nb_frames) * 1000000 / (pFormatCtx->duration));
        debug_msg("av_data_t::init:: frame rate from ffmpeg = %d/%d = %f", pCodecCtx->framerate.num, pCodecCtx->framerate.den, double(pCodecCtx->framerate.num) / pCodecCtx->framerate.den);
    }

    bool decode_frame(int64_t& timestamp)
    {
        AVPacket packet;
    
        int readFrame = av_read_frame(pFormatCtx, &packet);
    
        if (readFrame == (int) AVERROR_EOF)
        {
            timestamp = pFormatCtx->streams[video_stream]->nb_frames;
            av_free_packet(&packet);
            return false;
        }

        if (readFrame < 0)
        {
            debug_msg("av_data_t::decode_frame::av_read_frame error");
            av_free_packet(&packet);
            return false;
        }

        int frame_finished = 0;
        int ret = avcodec_decode_video2(pCodecCtx, pFrame, &frame_finished, &packet);

        if (ret < 0) 
        {
            char error[AV_ERROR_MAX_STRING_SIZE];
            av_make_error_string(error, AV_ERROR_MAX_STRING_SIZE, ret);
            debug_msg("av_data_t::decode_frame::avcodec_decode_video2 error:: %s", error);
            av_free_packet(&packet);
            return false;
        }

        if (!frame_finished) 
        {
            debug_msg("[ Decoder ] avcodec_decode_video2 end continue");
            av_free_packet(&packet);
            return false;
        }

        if (packet.stream_index != video_stream)
        {
            debug_msg("[ Decoder ] Incorrect stream! ");
            av_free_packet(&packet);
            return false;
        }

        timestamp = pFrame->best_effort_timestamp;
        av_free_packet(&packet);
        return true;
    }    
};

bool createFrameHandle(av_data_t & av_data, frame_handle_t & handle)
{
    AVFrame* f = av_frame_alloc();

    if(!f) return false;

    f->width  = av_data.pFrame->width;
    f->height = av_data.pFrame->height;
    f->format = av_data.pFrame->format;

    size_t numBytes = avpicture_get_size((AVPixelFormat)f->format, f->width, f->height);
    uint8_t* buffer = (uint8_t*) av_malloc(numBytes);

    if (!buffer) 
    {
        debug_msg("av_malloc failed to allocate %u bytes", (unsigned int) numBytes);
        return false;
    }

    avpicture_fill((AVPicture*)f, buffer, (AVPixelFormat)f->format, f->width, f->height);
    av_image_copy(f->data, f->linesize, (const uint8_t **) av_data.pFrame->data, av_data.pFrame->linesize, (AVPixelFormat) av_data.pFrame->format, f->width, f->height);

    handle.frame = f;
    handle.frame_buffer = buffer;
    return true;
}

template<typename Cntr, typename It> static void erase(Cntr & cntr, It begin, It end)
{
    for(It i = begin; i != end; ++i)
    {
        av_free(i->second.frame_buffer);
        av_frame_free(&i->second.frame);
    }
    cntr.erase(begin, end);
}

void decoder_thread(player_control_t* decoder)
{
    debug_msg("decoderThread::main func");

    av_register_all();
    av_data_t av_data;

    av_data.init(decoder->video);
    
    {
        lock_t l(decoder->lock);
        decoder->timer_interval = (1000.0f * av_data.pCodecCtx->framerate.den) / av_data.pCodecCtx->framerate.num;
        decoder->setFrameCount(av_data.pFormatCtx->streams[av_data.video_stream]->nb_frames);
        decoder->setDuration(av_data.pFormatCtx->duration / 1000);
        decoder->setState(playback_state_t::LOADED);
    }

    while(!decoder->quit)
    {
        int64_t effortTimestamp = -1;
        frame_handle_t handle;

        bool frameDecoded = av_data.decode_frame(effortTimestamp);

        lock_t lock(decoder->lock);

        // save last decoded frame
        if(frameDecoded)
        {
            frame_handle_t handle;
            if (decoder->buffer.find(effortTimestamp) == decoder->buffer.end() && createFrameHandle(av_data, handle))
                decoder->buffer[effortTimestamp] = handle;
        }

        // if video end is reached => do "nothing"..
        if(effortTimestamp == decoder->m_frame_count)
        {
            auto lastFrame = decoder->m_frame_position;
            while(lastFrame <= decoder->m_frame_position && !decoder->quit)
            {
                decoder->condWait.wait(lock);
                continue;
            }
        }

        uint32_t cache_range_min = decoder->m_frame_position - 10;
        uint32_t cache_range_max = decoder->m_frame_position + 30;


        // if buffer has cash_range => do "nothing"
        {
            bool range_is_decoded = false;

            auto timestamp = decoder->m_frame_position;
                       
            for(auto it = decoder->buffer.find(timestamp); it != decoder->buffer.end(); ++it, ++timestamp)
            {
                if(it->first != timestamp) break;

                if(timestamp >= std::min<uint32_t>(cache_range_max, decoder->m_frame_count - 1))
                {
                    range_is_decoded = true;
                    break;
                }
            }

            if(range_is_decoded)
                decoder->condWait.wait(lock);
        }

        //remove "old" frames
        {
            {
                for(auto lower = decoder->buffer.lower_bound(cache_range_min); lower != decoder->buffer.begin() && lower != decoder->buffer.end(); --lower)
                {
                    if (lower->first >= cache_range_min) continue;
                    erase(decoder->buffer, decoder->buffer.begin(), lower);
                    break;
                }
            }
        }

        {
            auto it = decoder->buffer.find(decoder->m_frame_position);

            if(it != decoder->buffer.end())
                continue;
            
            uint64_t seekTarget = decoder->m_frame_position * decoder->timer_interval * 1000;
            
            if (av_seek_frame(av_data.pFormatCtx, -1, seekTarget, AVSEEK_FLAG_BACKWARD) < 0)
            {
            }

            avcodec_flush_buffers(av_data.pCodecCtx);

            while(!decoder->quit)
            {
                int64_t effortTimestamp = -1;
                frame_handle_t handle;
                bool frameDecoded = av_data.decode_frame(effortTimestamp);                
                if(!frameDecoded) continue;
                createFrameHandle(av_data, handle);
                decoder->buffer[effortTimestamp] = handle;
                if(effortTimestamp > decoder->m_frame_position + 5) break;
            }

            // remove "very new" frames.
            auto upper = decoder->buffer.upper_bound(cache_range_max);
            
            if(upper != decoder->buffer.end())
                erase(decoder->buffer, upper, decoder->buffer.end());

        }
    }
    
    av_free(decoder->pictBuffer);
    av_free(av_data.pFrame);

    avcodec_close(av_data.pCodecCtx);
    avformat_close_input(&av_data.pFormatCtx);
}

player_control_t::player_control_t()
{
}

player_control_t::~player_control_t()
{
    deinit();
}

void player_control_t::init(const char* file, player_t*)
{    
    lock_t l(lock);
    video = file;
    setState(playback_state_t::LOADING);
    quit = false;
    thread = new std::thread(decoder_thread, this);
}

void player_control_t::deinit()
{
    if(thread)
    {
        quit = true;
        condWait.notify_all();
        thread->join();
        delete thread;
        thread = nullptr;
    }

    setState(playback_state_t::NOT_LOADED);
    
    clearBuffer();
    video = 0;
    buffer.clear();
    
    m_duration = 0;
    m_frame_count = 0;
    m_frame_position = 0;
    timer_interval = 0;

    pictBuffer = 0;
}

void player_control_t::play()
{
    {
        lock_t l(lock);

        if (playback_state != playback_state_t::PAUSED && playback_state != playback_state_t::LOADED)
            return;

        setState(playback_state_t::PLAYING);
    }
    condWait.notify_all();
}

void player_control_t::pause()
{
    lock_t l(lock);

    if (playback_state != playback_state_t::PLAYING) 
        return;

    setState(playback_state_t::PAUSED);
}

void player_control_t::set_frame(uint32_t frameIndex)
{
    {
        lock_t l(lock);
        auto oldPosition = m_frame_position;
        m_frame_position = std::min<uint32_t>(m_frame_count - 1, frameIndex);
        if(m_frame_position == oldPosition) return;
    }
    condWait.notify_all();
}

void player_control_t::next_frame()
{
    set_frame(m_frame_position + 1);
}

frame_handle_t player_control_t::current_frame() const
{
    lock_t l(lock);
    if (buffer.empty())
        return frame_handle_t();

    auto it = buffer.find(m_frame_position);

    if(buffer.end() == it) 
        return frame_handle_t();

    return it->second;
}

int64_t player_control_t::duration() const
{
    lock_t l(lock);
    return m_duration;
}

int64_t player_control_t::frame_count() const
{
    lock_t l(lock);
    return m_frame_count;
}

uint32_t player_control_t::frame_position() const
{
    lock_t l(lock);
    return m_frame_position;
}

playback_state_t player_control_t::state() const
{
    lock_t l(lock);
    return playback_state; 
}

void player_control_t::setState(playback_state_t state)
{
    lock_t l(lock);
    playback_state = state;
}

void player_control_t::setFrameCount(int64_t frame_count)
{
    m_frame_count = frame_count;
}

void player_control_t::setDuration(int64_t ms)
{
    m_duration = ms;
}


void player_control_t::clearBuffer()
{
    lock_t l(lock);

    auto frameIt = buffer.begin();
    while (frameIt != buffer.end()) 
    {
        av_free(frameIt->second.frame_buffer);
        av_frame_free(&frameIt->second.frame);

        frameIt = buffer.erase(frameIt);
    }
}

#ifndef _MSC_VER
    #pragma GCC diagnostic pop
#endif

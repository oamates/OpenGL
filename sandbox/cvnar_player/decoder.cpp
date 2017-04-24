#include "decoder.h"
#include "assert.hpp"

extern "C" {
#include <libavutil/pixfmt.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

#include <QDebug>
#include <iostream>
#include <QImage>

#include <QThread>

#include <chrono>

using lock_t = std::unique_lock<std::recursive_mutex>;

//#define DEBUG
//#define DEBUG_DECODER_POLICY
//#define DEBUG_DECODER_POLICY_SEEK
//#define DEBUG_DECODER_POLICY_WAIT
//#define MEASURENMENT

#ifdef MEASURENMENT
std::chrono::time_point<std::chrono::steady_clock> start;
#endif

static QString strErr(int err)
{
    char error[AV_ERROR_MAX_STRING_SIZE];
    av_make_error_string(error, AV_ERROR_MAX_STRING_SIZE, err);
    error[AV_ERROR_MAX_STRING_SIZE -1] = 0;
    return QString(error);
}

struct AvData
{
    int              videoStream = -1;
    AVFormatContext *pFormatCtx  = nullptr;
    AVCodecContext  *pCodecCtx   = nullptr;
    AVCodec         *pCodec      = nullptr;
    AVFrame         *pFrame      = nullptr;

    void init(const char* video);
};


void AvData::init(const char* video)
{
    qDebug() << Q_FUNC_INFO;

    ms_assert(avformat_open_input(&pFormatCtx, video, NULL, NULL) == 0);
    //qCritical() << "Couldn`t open video file " << decoder->video.c_str() << ". " << strErr(ret);
    ms_assert(avformat_find_stream_info(pFormatCtx, NULL) == 0);

    av_dump_format(pFormatCtx, -1, video, 0);

    for (size_t i = 0; i < pFormatCtx->nb_streams; i++) 
    {
        if (pFormatCtx->streams[i]->codec->codec_type != AVMEDIA_TYPE_VIDEO) 
          { continue; }
        
        videoStream = i;
        
        break;
    }

    ms_assert(videoStream != -1);

    pCodecCtx = pFormatCtx->streams[videoStream]->codec;

    ms_assert(pCodecCtx != nullptr);
    //pCodecCtx->active_thread_type = FF_THREAD_FRAME | FF_THREAD_SLICE; pCodecCtx->thread_type = FF_THREAD_FRAME | FF_THREAD_SLICE;

    pCodecCtx->thread_count = 0;

    ms_assert((pCodec = avcodec_find_decoder(pCodecCtx->codec_id)) != nullptr);

    ms_assert(avcodec_open2(pCodecCtx, pCodec, NULL) == 0);

    ms_assert((pFrame = av_frame_alloc()) != nullptr);

    qDebug() << "[ Decoder Init ] thread count           = " << pCodecCtx->thread_count;
    qDebug() << "[ Decoder Init ] thread type            = " << pCodecCtx->active_thread_type;
    qDebug() << "[ Decoder Init ] duration               = " << pFormatCtx->duration / 1000000;
    qDebug() << "[ Decoder Init ] frame count            = " << pFormatCtx->streams[videoStream]->nb_frames;
    qDebug() << "[ Decoder Init ] frame rate calculated  ="  << double(pFormatCtx->streams[videoStream]->nb_frames) * 1000000 / (pFormatCtx->duration);
    qDebug() << "[ Decoder Init ] frame rate from ffmpeg = " << pCodecCtx->framerate.den << " " << pCodecCtx->framerate.num;
}

static bool decodeOneFrame(AvData& avData, int64_t & effortTimestamp)
{
    AVPacket packet;

    struct AutoFree
    {
        AVPacket* packet;
        AutoFree(AVPacket& p) : packet(&p) { }
        ~AutoFree() { av_free_packet(packet); }
    } autoFree(packet);
    
    int readFrame = av_read_frame(avData.pFormatCtx, &packet);
    
    if (readFrame == (int)AVERROR_EOF)
    {
        effortTimestamp = avData.pFormatCtx->streams[avData.videoStream]->nb_frames;
        
        //qDebug() << "[ EOF: Decoder ] effortTimestamp = " << effortTimestamp;
        //qDebug() << "[ EOF: Decoder ] upperBound = " << decoder->m_playbackDecodePolicy.upperBound;
        return false;
    }

    if (readFrame < 0)
    {
        qDebug() << "[ Decoder ] av_read_frame error" ;
        return false;
    }

    int frameFinished = 0;
    int ret = avcodec_decode_video2(avData.pCodecCtx, avData.pFrame, &frameFinished, &packet);

    if (ret < 0) 
    {
        qCritical() << "[ Decoder ] " << strErr(ret);
        return false;
    }

    if (!frameFinished) 
    {
        qDebug() << "[ Decoder ] avcodec_decode_video2 end continue";
        return false;
    }

    if (packet.stream_index != avData.videoStream) {
        qDebug() << "[ Decoder ] Incorrect stream! ";
        return false;
    }

    effortTimestamp = avData.pFrame->best_effort_timestamp;

    //autoFree.packet = 0; // TODO: remove this line
    return true;
}

bool createFrameHandle(AvData & avData, FrameHandle & handle)
{
    AVFrame *f = av_frame_alloc();

    if(!f) return false;

    f->width  = avData.pFrame->width;
    f->height = avData.pFrame->height;
    f->format = avData.pFrame->format;

    size_t numBytes = avpicture_get_size((AVPixelFormat)f->format, f->width, f->height);
    uint8_t* buffer = (uint8_t*)av_malloc(numBytes);

    if(!buffer) 
    {
      qCritical() << "av_malloc failed to allocate " << numBytes << " bytes";
      return false;
    }

    avpicture_fill((AVPicture*)f, buffer, (AVPixelFormat)f->format, f->width, f->height);
    av_image_copy(f->data, f->linesize, (const uint8_t **)avData.pFrame->data, avData.pFrame->linesize,
                  (AVPixelFormat)avData.pFrame->format, f->width, f->height);

    handle.frame = f;
    handle.frameBuffer = buffer;
    return true;
}

//static std::pair<uint32_t, uint32_t> getRange(uint32_t value)
//{
//}

template<typename Cntr, typename It>
static void erase(Cntr & cntr, It begin, It end)
{
    for(It i = begin; i != end; ++i)
    {
        av_free(i->second.frameBuffer);
        av_frame_free(&i->second.frame);
    }
    cntr.erase(begin, end);
}


void decoderThread(PlayerControl *decoder)
{
    qDebug() << Q_FUNC_INFO;

    av_register_all();

    AvData avData;

    try { avData.init(decoder->video.c_str()); }
    
    catch(std::exception & ex)
    { 
        qCritical() << ex.what();

        lock_t l(decoder->lock);
        decoder->setState(StateHolder::PlaybackState::Error);
        return;
    }

    {
        lock_t l(decoder->lock);
        decoder->m_timerInterval = 1000.f / avData.pCodecCtx->framerate.num;
        decoder->setFrameCount(avData.pFormatCtx->streams[avData.videoStream]->nb_frames);
        decoder->setDuration(avData.pFormatCtx->duration / 1000);
        decoder->setState(StateHolder::PlaybackState::Loaded);
    }

    //uint32_t lastPlaybackFramePosition = 0;

    static PlayerControl *debug = decoder;
    #define Debug if(debug == decoder) qDebug() << "[ Decoder ] "

    for (;!decoder->quit;)
    {
        // decoder->buffer[effortTimestamp] = handle;
        
        int64_t effortTimestamp = -1;
        FrameHandle handle;

        bool frameDecoded = decodeOneFrame(avData, effortTimestamp);

        lock_t lock(decoder->lock);

        // save last decoded frame
        if(frameDecoded)
        {
            FrameHandle handle;

            if (decoder->buffer.find(effortTimestamp) == decoder->buffer.end() && createFrameHandle(avData, handle))
            {
                //Debug << "decoded frame " << effortTimestamp;
                decoder->buffer[effortTimestamp] = handle;
            }
        }

        // if video end is reached => do "nothing"..
        if(effortTimestamp == decoder->m_frameCount)
        {
           Debug << "Video end is reached. ";

           auto lastFrame = decoder->m_playbackFramePosition;

           while(lastFrame <= decoder->m_playbackFramePosition && !decoder->quit)
           {
               decoder->condWait.wait(lock);
               continue;
           }
           Debug << "[Decoder] Video end is reached: lastFrame = " << lastFrame << ", decoder->m_playbackFramePosition = " << decoder->m_playbackFramePosition;
        }

        std::pair<uint32_t, uint32_t> cash_range { decoder->m_playbackFramePosition - 10, decoder->m_playbackFramePosition + 30};

        // if buffer has cash_range => do "nothing"
        {
            bool range_is_decoded = false;

            auto timestamp = decoder->m_playbackFramePosition;
                       
            for(auto it = decoder->buffer.find(timestamp); it != decoder->buffer.end(); ++it, ++timestamp)
            {
                //if(timestamp >5880) Debug() << "it->first = " << it->first << ", timestamp = " << timestamp;
               
               if(it->first != timestamp)
               {
                   Debug << decoder << "  frame  " << timestamp << " doesn't exist";
                   break;
               }

               if(timestamp >= std::min<uint32_t>( cash_range.second, decoder->m_frameCount-1))
               {
                   range_is_decoded = true;
                   break;
               }
            }
            //if(timestamp > 5880) Debug() << "range_is_decoded = " << range_is_decoded;
            
            if(range_is_decoded)
              { decoder->condWait.wait(lock); }

            //Debug() << "[ Decoder ] " <<  decoder << "  range_is_decoded:  " << range_is_decoded << ", decoder->m_playbackFramePosition = " << decoder->m_playbackFramePosition;
            //Debug() << " ================================ "; for(auto & it : decoder->buffer) { Debug() << "[ Decoder ]  map contains frame " <<  it.first ; } Debug() << " ================================ ";
        }

        //remove "old" frames
        {
            //Debug << "removing old frames .."  ;
            //Debug << "decoder->m_playbackFramePosition = " << decoder->m_playbackFramePosition << ", decoder->m_frameCount " << decoder->m_frameCount;
            
            {
              for(auto lower = decoder->buffer.lower_bound(cash_range.first);
                lower != decoder->buffer.begin() && lower != decoder->buffer.end(); --lower)
              {
                  if(lower->first >= cash_range.first) continue;
                  //Debug << " to be removed [ " << decoder->buffer.begin()->first << ", " << lower->first <<"] ";
                  erase(decoder->buffer, decoder->buffer.begin(), lower);
                  break;
              }
            }
        }


        // frame (decoder->m_playbackFramePosition) not exists
        // doSeek(decoder->m_playbackFramePosition);
        // decode up to(decoder->m_playbackFramePosition + 5);
        {
            auto it = decoder->buffer.find(decoder->m_playbackFramePosition);

            if(it != decoder->buffer.end())
              { continue; }
            
            uint64_t seekTarget = decoder->m_playbackFramePosition * decoder->m_timerInterval * 1000;
            
            Debug << "  frame:  " << decoder->m_playbackFramePosition <<  " is not found.. seek to " << seekTarget;

            if (av_seek_frame(avData.pFormatCtx, -1, seekTarget, AVSEEK_FLAG_BACKWARD) < 0) { //! Should here be backward? AVSEEK_FLAG_BACKWARD
                Debug << "Couldn`t seek and this is pizdec";
                //todo: m_playbackFramePosition = end;
            }

            avcodec_flush_buffers(avData.pCodecCtx);

            while(!decoder->quit)
            {
                int64_t effortTimestamp = -1;

                FrameHandle handle;

                bool frameDecoded = decodeOneFrame(avData, effortTimestamp);
                
                if(!frameDecoded) continue;

                Debug << "  frame:  " << effortTimestamp <<  " has been decoded " ;

                createFrameHandle(avData, handle);

                decoder->buffer[effortTimestamp] = handle;

                if(effortTimestamp > decoder->m_playbackFramePosition + 5) break;
            }

            // remove "very new" frames.
            auto upper = decoder->buffer.upper_bound(cash_range.second);
            
            if(upper != decoder->buffer.end())
            {
                erase(decoder->buffer, upper, decoder->buffer.end());
            }
        }
    }

    qDebug() << "[ Decoder ] Leaving thread.";
    //av_free_packet(&packet);
    
    av_free(decoder->pictBuffer);
    av_free(avData.pFrame);

    avcodec_close(avData.pCodecCtx);
    avformat_close_input(&avData.pFormatCtx);
}

//#define DEBUG_DELETE_FRAME
//#define FRAME_EXIST

PlayerControl::PlayerControl()
    : QObject()
{
    frameRateTimer.setInterval(33);
    frameRateTimer.setSingleShot(false);
    frameRateTimer.setTimerType(Qt::PreciseTimer);

    QObject::connect(&frameRateTimer, &QTimer::timeout, [&] () { nextFrame(); });
}

PlayerControl::~PlayerControl()
{
    qDebug() << Q_FUNC_INFO;
    deinit();
}

void PlayerControl::init(const QString &file, Player*)
{    
    qDebug() << Q_FUNC_INFO;

    lock_t l(lock);

    video = file.toStdString();
    //ms_assert(!thread);

    emit sourceChanged();
    setState(StateHolder::PlaybackState::Loading);

    quit = false;
    thread = new std::thread(decoderThread, this);
}

void PlayerControl::deinit()
{
    //ms_assert(thread);
    qDebug() << Q_FUNC_INFO;

    if(thread)
    {
        qDebug() << "[ PlayerControl ] quit = true;";
        quit = true;
        qDebug() << "[ PlayerControl ] thread->join;";
        condWait.notify_all();
        thread->join();
        qDebug() << "[ PlayerControl ] delete thread;";
        delete thread;
        thread = nullptr;
    }

    frameRateTimer.stop();

    setState(StateHolder::PlaybackState::NotLoaded);
    
    clearBuffer();
    video.clear();
    buffer.clear();
    
    m_duration = 0;
    m_frameCount = 0;
    m_playbackFramePosition = 0;
    m_timerInterval = 0;

    pictBuffer = nullptr;
}

void PlayerControl::play()
{
    {
      lock_t l(lock);

      if (m_state != StateHolder::PlaybackState::Paused && m_state != StateHolder::PlaybackState::Loaded)
        { return; }

      frameRateTimer.start(m_timerInterval);
      setState(StateHolder::PlaybackState::Playing);
    }
    condWait.notify_all();
}

void PlayerControl::pause()
{
    lock_t l(lock);

    if (m_state != StateHolder::PlaybackState::Playing) 
      { return; }

    frameRateTimer.stop();
    setState(StateHolder::PlaybackState::Paused);
}

void PlayerControl::setCurrentFrame(uint32_t frameIndex)
{
    //qDebug() << "[ Seek ] m_playbackFramePosition = " << m_playbackFramePosition << " Frame index = " << frameIndex;

    {
      lock_t l(lock);
      auto oldPosition = m_playbackFramePosition;
      m_playbackFramePosition = std::min<uint32_t>(m_frameCount-1, frameIndex);
      if(m_playbackFramePosition == oldPosition) return;
    }

    condWait.notify_all();
    emit frameChanged(); //- incorrect, since the function might be called from
    //  different thread (see decoderThread function).
    //QMetaObject::invokeMethod(this, "frameChanged", QT::QueuedConnection);
    //TODO: fix all emit in the code
}

void PlayerControl::nextFrame()
{
    setCurrentFrame(m_playbackFramePosition+1);
}

//#define FRAME_EXIST
FrameHandle PlayerControl::currentFrame() const
{
    lock_t l(lock);
    if (buffer.empty()) return FrameHandle();

#ifdef FRAME_EXIST
    qDebug() << "[ currentFrame ] Is frame exist for this position = " << (buffer.find(m_playbackFramePosition) != buffer.end()) << m_playbackFramePosition;
#endif

    auto it = buffer.find(m_playbackFramePosition);

    if(buffer.end() == it) 
      { return FrameHandle(); }

    //  qDebug() << "m_playbackFramePosition = "  << m_playbackFramePosition;
    //  qDebug() << "Find frame = " << (buffer.find(m_playbackFramePosition) != buffer.end());
    //  qDebug() << "buffer size = " << buffer.size();
    //  qDebug() << "Finded frame is nullptr = " << (buffer.find(m_playbackFramePosition)->second == nullptr);
    //  qDebug() << "Get frame #"  << m_playbackFramePosition;
    return it->second;
}

int64_t PlayerControl::duration() const
{
    lock_t l(lock);
    return m_duration;
}

int64_t PlayerControl::frameCount() const
{
    lock_t l(lock);
    return m_frameCount;
}

uint32_t PlayerControl::framePosition() const
{
    lock_t l(lock);
    return m_playbackFramePosition;
}
StateHolder::PlaybackState PlayerControl::state() const
{
    lock_t l(lock);
    return m_state; 
}

void PlayerControl::setState(StateHolder::PlaybackState state)
{
    lock_t l(lock);

    if (m_state == state)
      { return; }

    m_state = state;
    emit stateChanged();

    //! Temp ugly workaround
    if (m_state == StateHolder::PlaybackState::Loaded)
      { emit frameChanged(); }
}

void PlayerControl::setFrameCount(int64_t frameCount)
{
    if (m_frameCount == frameCount) {
        return;
    }

    m_frameCount = frameCount;
    emit frameCountChanged();
}

void PlayerControl::setDuration(int64_t ms)
{
    if (m_duration == ms) {
        return;
    }

    m_duration = ms;
    emit durationChanged();
}


void PlayerControl::clearBuffer()
{
    lock_t l(lock);

    auto frameIt = buffer.begin();
    while (frameIt != buffer.end()) {
        av_free(frameIt->second.frameBuffer);
        av_frame_free(&frameIt->second.frame);

        frameIt = buffer.erase(frameIt);
    }
}

#ifndef _MSC_VER
    #pragma GCC diagnostic pop
#endif

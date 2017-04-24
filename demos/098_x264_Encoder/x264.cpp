#include "x264.hpp"
#include "log.hpp"
#include "utils.hpp"
 
x264_decoder::x264_decoder(h264_decoder_callback frameCallback, void* user) 
    :codec(0)
    ,codec_context(0)
    ,parser(0)
    ,fp(0)
    ,frame(0)
    ,cb_frame(frameCallback)
    ,cb_user(user)
    ,frame_timeout(0)
    ,frame_delay(0)
{
  avcodec_register_all();
}
 
x264_decoder::~x264_decoder()
{
    if(parser)
        av_parser_close(parser);
 
    if(codec_context)
    {
        avcodec_close(codec_context);
        av_free(codec_context);
    }
 
    if(picture)
        av_free(picture);
 
    if(fp) fclose(fp);
 
}
 
bool x264_decoder::load(const char* filepath, float fps) 
{
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);

    if(!codec)
    {
        debug_msg("Error: cannot find the x264 codec: %s\n", filepath);
        return false;
    }
 
    codec_context = avcodec_alloc_context3(codec);
 
    if(codec->capabilities & CODEC_CAP_TRUNCATED)
        codec_context->flags |= CODEC_FLAG_TRUNCATED;
 
    if(avcodec_open2(codec_context, codec, NULL) < 0)
    {
        debug_msg("Error: could not open codec.\n");
        return false;
    }
 
    fp = fopen(filepath, "rb");
 
    if(!fp) 
    {
        debug_msg("Error: cannot open: %s\n", filepath);
        return false;
    }
 
    picture = av_frame_alloc();
    parser = av_parser_init(AV_CODEC_ID_H264);
 
    if(!parser)
    {
        debug_msg("Erorr: cannot create H264 parser.\n");
        return false;
    }
 
    if(fps > 0.0001f)
    {
        frame_delay = (1.0f/fps) * 1000ull * 1000ull * 1000ull;
        frame_timeout = utils::timer::ns() + frame_delay;
    }
 
    readBuffer();
    return true;
}
 
bool x264_decoder::readFrame()
{
    uint64_t now = utils::timer::ns();
    if(now < frame_timeout)
        return false;

    bool needs_more = false;
 
    while(!update(needs_more))
        if(needs_more) readBuffer();
 
    if(frame_timeout == 0 && frame_delay == 0)
    {
        double fps = av_q2d(codec_context->time_base);
        if(fps > 0.0)
            frame_delay = fps * 1000ull * 1000ull * 1000ull;
    }
 
    if(frame_delay > 0)
        frame_timeout = utils::timer::ns() + frame_delay;

    return true;
}
 
void x264_decoder::decodeFrame(uint8_t* data, int size)
{
    AVPacket pkt;
    int got_picture = 0;
    int len = 0;

    av_init_packet(&pkt);
 
    pkt.data = data;
    pkt.size = size;

    len = avcodec_decode_video2(codec_context, picture, &got_picture, &pkt);
    if(len < 0)
        debug_msg("Error while decoding a frame.\n");
 
    if(got_picture == 0) return;
    ++frame;
 
    if(cb_frame)
        cb_frame(picture, &pkt, cb_user);
}
 
int x264_decoder::readBuffer()
{
    int bytes_read = (int)fread(inbuf, 1, H264_INBUF_SIZE, fp);

    if(bytes_read)
        std::copy(inbuf, inbuf + bytes_read, std::back_inserter(buffer));

    return bytes_read;
}
 
bool x264_decoder::update(bool& needsMoreBytes)
{
    needsMoreBytes = false;
 
    if(!fp)
    {
        debug_msg("Cannot update .. file not opened...\n");
        return false;
    }
 
    if(buffer.size() == 0)
    {
        needsMoreBytes = true;
        return false;
    }

    uint8_t* data = NULL;
    int size = 0;
    int len = av_parser_parse2(parser, codec_context, &data, &size, &buffer[0], buffer.size(), 0, 0, AV_NOPTS_VALUE);

    if(size == 0 && len >= 0)
    {
        needsMoreBytes = true;
        return false;
    }
 
    if(len)
    {
        decodeFrame(&buffer[0], size);
        buffer.erase(buffer.begin(), buffer.begin() + len);
        return true;
    }
    return false;
}
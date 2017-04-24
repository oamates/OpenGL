// Output a media file in any supported libavformat format. The default codecs are used.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

extern "C"
{ 
#include "libavutil/channel_layout.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavformat/avformat.h"
#include "libavresample/avresample.h"
#include "libswscale/swscale.h"
}

#include "log.hpp"


#define STREAM_DURATION   5.0                                                                                   // 5 seconds stream duration
#define STREAM_FRAME_RATE 25                                                                                    // 25 images/s
#define STREAM_NB_FRAMES  ((int)(STREAM_DURATION * STREAM_FRAME_RATE))
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P                                                                    // default pix_fmt
#define SCALE_FLAGS       SWS_BICUBIC

typedef struct OutputStream                                                                                     // a wrapper around a single output AVStream
{
    AVStream *st;    
    int64_t next_pts;                                                                                           // pts of the next frame that will be generated
    AVFrame *frame;
    AVFrame *tmp_frame;
    float t, tincr, tincr2;
    struct SwsContext *sws_ctx;
    AVAudioResampleContext *avr;
} OutputStream;

static void add_audio_stream(OutputStream *ost, AVFormatContext *oc, enum AVCodecID codec_id)                   // add an audio output stream
{
    AVCodec* codec = avcodec_find_encoder(codec_id);                                                            // find the audio encoder
    if (!codec)
        exit_msg("Codec not found.");

    ost->st = avformat_new_stream(oc, codec);
    if (!ost->st)
        exit_msg("Could not alloc stream.");

    AVCodecContext* c = ost->st->codec;    
    c->sample_fmt     = codec->sample_fmts           ? codec->sample_fmts[0]           : AV_SAMPLE_FMT_S16;     // put sample parameters
    c->sample_rate    = codec->supported_samplerates ? codec->supported_samplerates[0] : 44100;
    c->channel_layout = codec->channel_layouts       ? codec->channel_layouts[0]       : AV_CH_LAYOUT_STEREO;
    c->channels       = av_get_channel_layout_nb_channels(c->channel_layout);
    c->bit_rate       = 64000;
    ost->st->time_base = (AVRational){ 1, c->sample_rate };    
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)                                                                // some formats want stream headers to be separate
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;

    // initialize sample format conversion; to simplify the code, we always pass the data through lavr, even
    // if the encoder supports the generated format directly -- the price is some extra data copying;
    ost->avr = avresample_alloc_context();
    if (!ost->avr)
        exit_msg("Error allocating the resampling context");

    av_opt_set_int(ost->avr, "in_sample_fmt",      AV_SAMPLE_FMT_S16,   0);
    av_opt_set_int(ost->avr, "in_sample_rate",     44100,               0);
    av_opt_set_int(ost->avr, "in_channel_layout",  AV_CH_LAYOUT_STEREO, 0);
    av_opt_set_int(ost->avr, "out_sample_fmt",     c->sample_fmt,       0);
    av_opt_set_int(ost->avr, "out_sample_rate",    c->sample_rate,      0);
    av_opt_set_int(ost->avr, "out_channel_layout", c->channel_layout,   0);

    int ret = avresample_open(ost->avr);
    if (ret < 0)
        exit_msg("Error opening the resampling context.");
}

static AVFrame*  alloc_audio_frame(enum AVSampleFormat sample_fmt, uint64_t channel_layout, int sample_rate, int nb_samples)
{
    AVFrame* frame = av_frame_alloc();

    if (!frame)
        exit_msg("Error allocating an audio frame");
    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples)
    {
        int ret = av_frame_get_buffer(frame, 0);
        if (ret < 0)
            exit_msg("Error allocating an audio buffer.");
    }
    return frame;
}

static void open_audio(AVFormatContext *oc, OutputStream *ost)
{
    AVCodecContext* c = ost->st->codec;
    
    if (avcodec_open2(c, 0, 0) < 0)                                                                             // open it
        exit_msg("Could not open codec.");
    
    ost->t     = 0;                                                                                             // init signal generator
    ost->tincr = 2 * M_PI * 110.0 / c->sample_rate;    
    ost->tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;                                           // increment frequency by 110 Hz per second
    int nb_samples = (c->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE) ? 10000 : c->frame_size;
    ost->frame     = alloc_audio_frame(c->sample_fmt, c->channel_layout, c->sample_rate, nb_samples);
    ost->tmp_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, AV_CH_LAYOUT_STEREO, 44100, nb_samples);
}

static AVFrame *get_audio_frame(OutputStream *ost)                                                              // Prepare a 16 bit dummy audio frame of 'frame_size' samples and 'nb_channels' channels.
{
    AVFrame *frame = ost->tmp_frame;
    int16_t *q = (int16_t*)frame->data[0];

    // check if we want to generate more frames
    if (av_compare_ts(ost->next_pts, ost->st->codec->time_base, STREAM_DURATION, (AVRational){1, 1}) >= 0) return 0;

    for (int j = 0; j < frame->nb_samples; j++)
    {
        int v = (int)(sin(ost->t) * 10000);
        for (int i = 0; i < ost->st->codec->channels; i++) *q++ = v;
        ost->t     += ost->tincr;
        ost->tincr += ost->tincr2;
    }
    return frame;
}

static int encode_audio_frame(AVFormatContext *oc, OutputStream *ost, AVFrame *frame)                           // if a frame is provided, send it to the encoder, otherwise flush the encoder; return 1 when encoding is finished, 0 otherwise
{
    AVPacket pkt = {0};                                                                                         // data and size must be 0;
    av_init_packet(&pkt);

    int got_packet;
    avcodec_encode_audio2(ost->st->codec, &pkt, frame, &got_packet);

    if (got_packet)
    {
        pkt.stream_index = ost->st->index;
        av_packet_rescale_ts(&pkt, ost->st->codec->time_base, ost->st->time_base);        

        if (av_interleaved_write_frame(oc, &pkt) != 0)                                                          // Write the compressed frame to the media file.
            exit_msg("Error while writing audio frame");
    }
    return (frame || got_packet) ? 0 : 1;
}

static int process_audio_stream(AVFormatContext* oc, OutputStream* ost)                                         // Encode one audio frame and send it to the muxer. Return 1 when encoding is finished, 0 otherwise
{
    int got_output = 0;

    AVFrame* frame = get_audio_frame(ost);
    got_output |= !!frame;
    
    if (frame)                                                                                                  // feed the data to lavr
    {
        int ret = avresample_convert(ost->avr, NULL, 0, 0, frame->extended_data, frame->linesize[0], frame->nb_samples);
        if (ret < 0)
            exit_msg("Error feeding audio data to the resampler.");
    }

    while ((frame && avresample_available(ost->avr) >= ost->frame->nb_samples) || (!frame && avresample_get_out_samples(ost->avr, 0)))
    {        
        int ret = av_frame_make_writable(ost->frame);                                                           // when we pass a frame to the encoder, it may keep a reference to it internally; make sure we do not overwrite it here
        if (ret < 0) 
            exit_msg("Error frame make writable.");

        // the difference between the two avresample calls here is that the first one just reads the already converted data that is buffered in the lavr output buffer, while the second one also flushes the resampler
        ret = frame ? avresample_read(ost->avr, ost->frame->extended_data, ost->frame->nb_samples) : 
                      avresample_convert(ost->avr, ost->frame->extended_data, ost->frame->linesize[0], ost->frame->nb_samples, 0, 0, 0);

        if (ret < 0) 
            exit_msg("Error while resampling.");
        if (frame && ret != ost->frame->nb_samples)
            exit_msg("Too few samples returned from lavr.")

        ost->frame->nb_samples = ret;
        ost->frame->pts        = ost->next_pts;
        ost->next_pts         += ost->frame->nb_samples;
        got_output |= encode_audio_frame(oc, ost, ret ? ost->frame : 0);
    }

    return !got_output;
}

static void add_video_stream(OutputStream *ost, AVFormatContext *oc, enum AVCodecID codec_id)                   // Add a video output stream.
{
    AVCodec* codec = avcodec_find_encoder(codec_id);                                                            // find the video encoder
    if (!codec)
        exit_msg("Codec not found.")

    ost->st = avformat_new_stream(oc, codec);
    if (!ost->st)
        exit_msg("Could not alloc stream.");

    AVCodecContext* c = ost->st->codec;
    c->bit_rate = 400000;                                                                                       // Put sample parameters.
    c->width    = 352;                                                                                          // Resolution must be a multiple of two.
    c->height   = 288;

    // This is the fundamental unit of time (in seconds) in terms of which frame timestamps are represented.     
    ost->st->time_base = (AVRational){1, STREAM_FRAME_RATE};                                                    // For fixed-fps content, timebase should be 1/framerate and timestamp increments should be identical to 1.
    c->time_base       = ost->st->time_base;
    c->gop_size      = 12;                                                                                      // emit one intra frame every twelve frames at most
    c->pix_fmt       = STREAM_PIX_FMT;
    if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO)         
        c->max_b_frames = 2;                                                                                    // just for testing, we also add B frames

    // Needed to avoid using macroblocks in which some coeffs overflow. This does not happen with normal video, it just happens here as the motion of the chroma plane does not match the luma plane.
    if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO)
        c->mb_decision = 2;    
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)                                                                // Some formats want stream headers to be separate.
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;
}

static AVFrame* alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
    AVFrame* picture = av_frame_alloc();
    if (!picture) return 0;

    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;
    
    int ret = av_frame_get_buffer(picture, 32);                                                                 // allocate the buffers for the frame data
    if (ret < 0)
        exit_msg("Could not allocate frame data.");

    return picture;
}

static void open_video(AVFormatContext *oc, OutputStream *ost)
{
    AVCodecContext* c = ost->st->codec;    
    if (avcodec_open2(c, 0, 0) < 0)                                                                             // open the codec
        exit_msg("Could not open codec.");

    
    ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);                                                // Allocate the encoded raw picture.
    if (!ost->frame)
        exit_msg("Could not allocate picture.");

    // If the output format is not YUV420P, then a temporary YUV420P picture is needed too. It is then converted to the required output format.
    ost->tmp_frame = 0;
    if (c->pix_fmt != AV_PIX_FMT_YUV420P)
    {
        ost->tmp_frame = alloc_picture(AV_PIX_FMT_YUV420P, c->width, c->height);
        if (!ost->tmp_frame)
            exit_msg("Could not allocate temporary picture");
    }
}

static void fill_yuv_image(AVFrame *pict, int frame_index, int width, int height)                               // Prepare a dummy image.
{
    if (av_frame_make_writable(pict) < 0)                                                                       // when we pass a frame to the encoder, it may keep a reference to it internally; make sure we do not overwrite it here
        exit_msg("Frame make writable failed.");

    for (int y = 0; y < height; y++)                                                                            // Y
        for (int x = 0; x < width; x++)
            pict->data[0][y * pict->linesize[0] + x] = x + y + frame_index * 3;
    
    for (int y = 0; y < height / 2; y++)                                                                        // Cb and Cr
        for (int x = 0; x < width / 2; x++)
        {
            pict->data[1][y * pict->linesize[1] + x] = 128 + y + frame_index * 2;
            pict->data[2][y * pict->linesize[2] + x] = 64 + x + frame_index * 5;
        }
}

static AVFrame *get_video_frame(OutputStream *ost)
{
    AVCodecContext *c = ost->st->codec;

    // check if we want to generate more frames
    if (av_compare_ts(ost->next_pts, ost->st->codec->time_base, STREAM_DURATION, (AVRational){ 1, 1 }) >= 0) return 0;

    if (c->pix_fmt != AV_PIX_FMT_YUV420P)
    {        
        if (!ost->sws_ctx)                                                                                      // as we only generate a YUV420P picture, we must convert it to the codec pixel format if needed
        {
            ost->sws_ctx = sws_getContext(c->width, c->height, AV_PIX_FMT_YUV420P, c->width, c->height, c->pix_fmt, SCALE_FLAGS, 0, 0, 0);
            if (!ost->sws_ctx)
                exit_msg("Cannot initialize the conversion context");
        }
        fill_yuv_image(ost->tmp_frame, ost->next_pts, c->width, c->height);
        sws_scale(ost->sws_ctx, ost->tmp_frame->data, ost->tmp_frame->linesize, 0, c->height, ost->frame->data, ost->frame->linesize);
    }
    else
        fill_yuv_image(ost->frame, ost->next_pts, c->width, c->height);

    ost->frame->pts = ost->next_pts++;
    return ost->frame;
}

static int write_video_frame(AVFormatContext *oc, OutputStream *ost)                                            // Encode one video frame and send it to the muxer. Return 1 when encoding is finished, 0 otherwise
{
    int ret;
    AVCodecContext* c = ost->st->codec;
    AVFrame* frame = get_video_frame(ost);
    int got_packet = 0;

    if (oc->oformat->flags & AVFMT_RAWPICTURE)                                                                  // a hack to avoid data copy with some raw video muxers
    {        
        AVPacket pkt;
        av_init_packet(&pkt);

        if (!frame) return 1;

        pkt.flags        |= AV_PKT_FLAG_KEY;
        pkt.stream_index  = ost->st->index;
        pkt.data          = (uint8_t *)frame;
        pkt.size          = sizeof(AVPicture);
        pkt.pts = pkt.dts = frame->pts;
        av_packet_rescale_ts(&pkt, c->time_base, ost->st->time_base);

        ret = av_interleaved_write_frame(oc, &pkt);
    }
    else
    {
        AVPacket pkt = {0};
        av_init_packet(&pkt);
        
        ret = avcodec_encode_video2(c, &pkt, frame, &got_packet);                                               // encode the image
        if (ret < 0)
            exit_msg("Error encoding a video frame.");

        if (got_packet)
        {
            av_packet_rescale_ts(&pkt, c->time_base, ost->st->time_base);
            pkt.stream_index = ost->st->index;            
            ret = av_interleaved_write_frame(oc, &pkt);                                                         // Write the compressed frame to the media file.
        }
    }
    if (ret != 0)
        exit_msg("Error while writing video frame.");

    return (frame || got_packet) ? 0 : 1;
}

static void close_stream(AVFormatContext *oc, OutputStream *ost)
{
    avcodec_close(ost->st->codec);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    sws_freeContext(ost->sws_ctx);
    avresample_free(&ost->avr);
}

int main(int argc, char **argv)
{
    OutputStream video_st = {0}, audio_st = {0};
    const char *filename;
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    int have_video = 0, have_audio = 0;
    int encode_video = 0, encode_audio = 0;

    // Initialize libavcodec, and register all codecs and formats.
    av_register_all();

    if (argc != 2)
        exit_msg("usage: %s output_file\nAPI example program to output a media file with libavformat.\n"
                 "The output format is automatically guessed according to the file extension.\nRaw images can also be output by using '%%d' in the filename\n", argv[0]);

    filename = argv[1];

    // Autodetect the output format from the name. default is MPEG.
    fmt = av_guess_format(0, filename, 0);
    if (!fmt)
    {
        debug_msg("Could not deduce output format from file extension: using MPEG.");
        fmt = av_guess_format("mpeg", 0, 0);
    }
    if (!fmt)
        exit_msg("Could not find suitable output format.");

    // Allocate the output media context.
    oc = avformat_alloc_context();
    if (!oc) exit_msg("Memory error");
    oc->oformat = fmt;
    snprintf(oc->filename, sizeof(oc->filename), "%s", filename);

    // Add the audio and video streams using the default format codecs and initialize the codecs.
    if (fmt->video_codec != AV_CODEC_ID_NONE)
    {
        add_video_stream(&video_st, oc, fmt->video_codec);
        have_video = 1;
        encode_video = 1;
    }
    if (fmt->audio_codec != AV_CODEC_ID_NONE)
    {
        add_audio_stream(&audio_st, oc, fmt->audio_codec);
        have_audio = 1;
        encode_audio = 1;
    }

    // Now that all the parameters are set, we can open the audio and video codecs and allocate the necessary encode buffers.
    if (have_video) open_video(oc, &video_st);
    if (have_audio) open_audio(oc, &audio_st);

    av_dump_format(oc, 0, filename, 1);
    
    if (!(fmt->flags & AVFMT_NOFILE))                                                                                   // open the output file, if needed 
    {
        if (avio_open(&oc->pb, filename, AVIO_FLAG_WRITE) < 0) 
            exit_msg("Could not open file : %s", filename);
    }
    
    avformat_write_header(oc, 0);                                                                                       // Write the stream header, if any.

    while (encode_video || encode_audio)                                                                                // select the stream to encode
    {
        
        if (encode_video && (!encode_audio || av_compare_ts(video_st.next_pts, video_st.st->codec->time_base, audio_st.next_pts, audio_st.st->codec->time_base) <= 0))
            encode_video = !write_video_frame(oc, &video_st);
        else
            encode_audio = !process_audio_stream(oc, &audio_st);
    }

    // Write the trailer, if any. The trailer must be written before you close the CodecContexts open when you wrote the header; otherwise
    // av_write_trailer() may try to use memory that was freed on av_codec_close().
    av_write_trailer(oc);
    
    if (have_video) close_stream(oc, &video_st);                                                                        // Close codecs.
    if (have_audio) close_stream(oc, &audio_st);

    if (!(fmt->flags & AVFMT_NOFILE))        
        avio_close(oc->pb);                                                                                             // Close the output file.
    
    avformat_free_context(oc);                                                                                          // Free the stream

    return 0;
}
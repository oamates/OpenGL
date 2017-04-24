// libavcodec API use example.
// The library only handles codecs (mpeg, mpeg4, etc...), not file formats (avi, vob, etc...). 
// See library 'libavformat' for the format handling

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "log.hpp"

#ifdef HAVE_AV_CONFIG_H
#undef HAVE_AV_CONFIG_H
#endif

extern "C"
{ 
#include "libavcodec/avcodec.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"
}

void log_callback(void* ptr, int level, const char* format, va_list vargs)
{
    FILE *output = fopen("debug.log", "a+");
    if (!output) return;
    fprintf(output, "LibAV debug :: level = %d :: message = ", level);
    vfprintf(output, format, vargs);
    fprintf(output, "\n");
    fclose(output);
}

#define INBUF_SIZE 4096
#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

static int check_sample_fmt(AVCodec* codec, enum AVSampleFormat sample_fmt)                             // check that a given sample format is supported by the encoder
{
    const enum AVSampleFormat *p = codec->sample_fmts;

    while (*p != AV_SAMPLE_FMT_NONE)
    {
        if (*p == sample_fmt) return 1;
        p++;
    }
    return 0;
}

static int select_sample_rate(AVCodec *codec)                                                           // pick the highest supported samplerate
{
    if (!codec->supported_samplerates) 
        return 44100;

    const int* p = codec->supported_samplerates;

    int best_samplerate = 0;
    while (*p) 
    {
        best_samplerate = FFMAX(*p, best_samplerate);
        p++;
    }
    return best_samplerate;
}

static int select_channel_layout(AVCodec* codec)                                                        // select layout with the highest channel count
{
    uint64_t best_ch_layout = 0;
    int best_nb_channels = 0;

    if (!codec->channel_layouts)
        return AV_CH_LAYOUT_STEREO;

    const uint64_t* p = codec->channel_layouts;
    while (*p) 
    {
        int nb_channels = av_get_channel_layout_nb_channels(*p);
        if (nb_channels > best_nb_channels)
        {
            best_ch_layout = *p;
            best_nb_channels = nb_channels;
        }
        p++;
    }
    return best_ch_layout;
}

static void audio_encode_example(const char *filename)                                                  // Audio encoding example
{
    debug_msg("Audio encoding begin.");
    AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_MP2);                                             // find the MP2 encoder
    if (!codec) exit_msg("AV_CODEC_ID_MP2 not found.");

    AVCodecContext* c = avcodec_alloc_context3(codec);
    c->bit_rate = 64000;                                                                                // put sample parameters
    c->sample_fmt = AV_SAMPLE_FMT_S16;                                                                  // check that the encoder supports s16 pcm input
    if (!check_sample_fmt(codec, c->sample_fmt)) 
        exit_msg("encoder does not support %s", av_get_sample_fmt_name(c->sample_fmt));
    c->sample_rate    = select_sample_rate(codec);                                                      // select other audio parameters supported by the encoder
    c->channel_layout = select_channel_layout(codec);
    c->channels       = av_get_channel_layout_nb_channels(c->channel_layout);

    if (avcodec_open2(c, codec, 0) < 0)                                                                 // try to open it
        exit_msg("Could not open the codec.");

    FILE* f = fopen(filename, "wb");
    if (!f) exit_msg("could not open file : %s.", filename);

    AVFrame* frame = av_frame_alloc();                                                                  // frame containing input raw audio
    if (!frame)
        exit_msg("Could not allocate audio frame.");
    frame->nb_samples     = c->frame_size;
    frame->format         = c->sample_fmt;
    frame->channel_layout = c->channel_layout;
    
    int buffer_size = av_samples_get_buffer_size(0, c->channels, c->frame_size, c->sample_fmt, 0);      // the codec gives us the frame size, in samples, calculate the size of the samples buffer in bytes
    uint16_t* samples = static_cast<uint16_t*> (av_malloc(buffer_size));
    if (!samples)
        exit_msg("Could not allocate %d bytes for samples buffer.", buffer_size);

    // setup the data pointers in the AVFrame
    int ret = avcodec_fill_audio_frame(frame, c->channels, c->sample_fmt, (const uint8_t*) samples, buffer_size, 0);
    if (ret < 0)
        exit_msg("Could not setup audio frame.") 
    
    float t = 0;                                                                                        // encode a single tone sound
    float tincr = 2 * M_PI * 440.0 / c->sample_rate;
    AVPacket pkt;

    for(int i = 0; i < 200; i++) 
    {
        av_init_packet(&pkt);
        pkt.data = 0;                                                                                   // packet data will be allocated by the encoder
        pkt.size = 0;

        for (int j = 0; j < c->frame_size; j++)
        {
            samples[2*j] = (int)(sin(t) * 10000);

            for (int k = 1; k < c->channels; k++)
                samples[2*j + k] = samples[2*j];
            t += tincr;
        }

        int got_output;    
        ret = avcodec_encode_audio2(c, &pkt, frame, &got_output);                                       // encode the samples
        if (ret < 0)
            exit_msg("Error encoding audio frame.");

        if (got_output)
        {
            fwrite(pkt.data, 1, pkt.size, f);
            av_free_packet(&pkt);
        }
    }
    fclose(f);
    av_freep(&samples);
    av_frame_free(&frame);
    avcodec_close(c);
    av_free(c);
    debug_msg("Audio encoding done.");    
}


static void audio_decode_example(const char *outfilename, const char *filename)                         // Audio decoding example.
{
    debug_msg("Audio decoding begin.");

    uint8_t inbuf[AUDIO_INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];

    AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_MP2);                                             // find the MP2 encoder
    if (!codec) exit_msg("AV_CODEC_ID_MP2 not found.");

    AVCodecContext* c = avcodec_alloc_context3(codec);

    c->sample_fmt = AV_SAMPLE_FMT_S16;  

    int error_code = avcodec_open2(c, codec, 0);
    if (error_code < 0)                                                                                 // try to open it
        exit_msg("Could not open the codec(%p, %p). Error code = %d", c, codec, error_code);    


    FILE* f = fopen(filename, "rb");
    if (!f) exit_msg("could not open file : %s.", filename);

    FILE* outfile = fopen(outfilename, "wb");
    if (!outfile)
    {
        av_free(c);
        exit_msg("could not open file : %s.", outfilename);
    } 

    AVPacket avpkt;
    av_init_packet(&avpkt);
    avpkt.data = inbuf;                                                                                 // decode until eof
    avpkt.size = fread(inbuf, 1, AUDIO_INBUF_SIZE, f);
    AVFrame* decoded_frame = av_frame_alloc();
    if (!decoded_frame) 
        exit_msg("Cannot allocate frame. Not enough memory.");

    while (avpkt.size > 0)
    {
        int got_frame = 0;

        int len = avcodec_decode_audio4(c, decoded_frame, &got_frame, &avpkt);
        if (len < 0) 
            exit_msg("Error while decoding.");

        if (got_frame)                                                                                  // if a frame has been decoded, output it
        {
            int data_size = av_samples_get_buffer_size(0, c->channels, decoded_frame->nb_samples, c->sample_fmt, 1);
            fwrite(decoded_frame->data[0], 1, data_size, outfile);
        }
        avpkt.size -= len;
        avpkt.data += len;
        if (avpkt.size < AUDIO_REFILL_THRESH)
        {
            // Refill the input buffer, to avoid trying to decode incomplete frames. Instead of this, one could also use a parser, or use a proper container format through libavformat.
            memmove(inbuf, avpkt.data, avpkt.size);
            avpkt.data = inbuf;
            len = fread(avpkt.data + avpkt.size, 1, AUDIO_INBUF_SIZE - avpkt.size, f);
            if (len > 0)
                avpkt.size += len;
        }
    }

    fclose(outfile);
    fclose(f);
    avcodec_close(c);
    av_free(c);
    av_frame_free(&decoded_frame);
    debug_msg("Audio decoding done.");
}

static void video_encode_example(const char *filename)                                                  // Video encoding example
{
    int i, got_output;
    uint8_t endcode[] = {0, 0, 1, 0xb7};

    debug_msg("Video encoding begin.");
    
    AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_MPEG1VIDEO);                                      // find the mpeg1 video encoder
    if (!codec)
        exit_msg("Codec not found.");

    AVCodecContext* c = avcodec_alloc_context3(codec);
    AVFrame* picture = av_frame_alloc();    
    c->bit_rate = 400000;                                                                               // put sample parameters    
    c->width = 352;                                                                                     // resolution must be a multiple of two
    c->height = 288;    
    c->time_base = (AVRational) {1, 25};                                                                // frames per second
    c->gop_size = 10;                                                                                   // emit one intra frame every ten frames
    c->max_b_frames = 1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    
    if (avcodec_open2(c, codec, 0) < 0)                                                                 // open it
        exit_msg("Could not open codec.")

    FILE* f = fopen(filename, "wb");
    if (!f)
        exit_msg("Could not open %s.", filename);

    int ret = av_image_alloc(picture->data, picture->linesize, c->width, c->height, c->pix_fmt, 32);
    if (ret < 0)
        exit_msg("Could not alloc raw picture buffer.")
    picture->format = c->pix_fmt;
    picture->width  = c->width;
    picture->height = c->height;
    
    AVPacket pkt;                                                                                       // encode 1 second of video
    for(i = 0; i < 25; i++)
    {
        av_init_packet(&pkt);                                                                           // packet data will be allocated by the encoder
        pkt.data = 0;    
        pkt.size = 0;
        fflush(stdout);
        
        for(int y = 0; y < c->height; y++)                                                              // prepare a dummy image, Y component
            for(int x = 0; x < c->width; x++)
                picture->data[0][y * picture->linesize[0] + x] = x + y + i * 3;
        
        for(int y = 0; y < c->height / 2; y++)                                                          // Cb and Cr
            for(int x = 0; x < c->width / 2; x++)
            {
                picture->data[1][y * picture->linesize[1] + x] = 128 + y + i * 2;
                picture->data[2][y * picture->linesize[2] + x] = 64 + x + i * 5;
            }

        picture->pts = i;
        
        ret = avcodec_encode_video2(c, &pkt, picture, &got_output);                                     // encode the image
        if (ret < 0)
            exit_msg("Error encoding frame.")

        if (got_output)
        {
            debug_msg("encoding frame %3d (size = %5d)\n", i, pkt.size);
            fwrite(pkt.data, 1, pkt.size, f);
            av_free_packet(&pkt);
        }
    }
    
    for (got_output = 1; got_output; i++)                                                               // get the delayed frames
    {
        fflush(stdout);
        ret = avcodec_encode_video2(c, &pkt, 0, &got_output);
        if (ret < 0)
            exit_msg("Error encoding frame.");

        if (got_output)
        {
            debug_msg("encoding frame %3d (size = %5d)\n", i, pkt.size);
            fwrite(pkt.data, 1, pkt.size, f);
            av_free_packet(&pkt);
        }
    }

    fwrite(endcode, 1, sizeof(endcode), f);                                                             // add sequence end code to have a real mpeg file
    fclose(f);
    avcodec_close(c);
    av_free(c);
    av_freep(&picture->data[0]);
    av_frame_free(&picture);
}

static void pgm_save(unsigned char* buf, int wrap, int xsize, int ysize, char* filename)                // Video decoding example
{
    FILE* f = fopen(filename, "w");
    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
    for(int i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}

static void video_decode_example(const char* outfilename, const char* filename)
{
    debug_msg("Video decoding example.");    
    int got_picture;
    uint8_t inbuf[INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];
    char buf[1024];

    AVPacket avpkt;
    av_init_packet(&avpkt);
    memset(inbuf + INBUF_SIZE, 0, FF_INPUT_BUFFER_PADDING_SIZE);                                        // set end of buffer to 0 (this ensures that no overreading happens for damaged mpeg streams)

    AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_MPEG1VIDEO);                                      // find the mpeg1 video decoder
    if (!codec)
        exit_msg("Codec not found.")

    AVCodecContext* c = avcodec_alloc_context3(codec);
    AVFrame* picture = av_frame_alloc();

    if(codec->capabilities & CODEC_CAP_TRUNCATED) c->flags |= CODEC_FLAG_TRUNCATED;                     // we do not send complete frames

    // For some codecs, such as msmpeg4 and mpeg4, width and height MUST be initialized there because this information is not available in the bitstream.
    if (avcodec_open2(c, codec, 0) < 0)                                                                 // open it
        exit_msg("Could not open codec.")

    FILE* f = fopen(filename, "rb");                                                                    // the codec gives us the frame size, in samples
    if (!f)
        exit_msg("Could not open file : %s\n", filename);

    int frame = 0;

    while(1)
    {
        avpkt.size = fread(inbuf, 1, INBUF_SIZE, f);
        if (avpkt.size == 0) break;

        // NOTE1: some codecs are stream based (mpegvideo, mpegaudio) and this is the only method to use them because you cannot know the compressed data size before analysing it.
        // BUT some other codecs (msmpeg4, mpeg4) are inherently frame based, so you must call them with all the data for one frame exactly.
        // You must also initialize 'width' and 'height' before initializing them.

        // NOTE2: some codecs allow the raw parameters (frame size, sample rate) to be changed at any frame. We handle this, so you should also take care of it
        // here, we use a stream based decoder (mpeg1video), so we feed decoder and see if it could decode a frame */

        avpkt.data = inbuf;
        while (avpkt.size > 0)
        {
            int len = avcodec_decode_video2(c, picture, &got_picture, &avpkt);
            if (len < 0)
                exit_msg("Error while decoding frame %d.", frame);

            if (got_picture)
            {
                debug_msg("saving frame %3d.", frame);
                fflush(stdout);
                snprintf(buf, sizeof(buf), outfilename, frame);                                         // the picture is allocated by the decoder. no need to free it
                pgm_save(picture->data[0], picture->linesize[0], c->width, c->height, buf);
                frame++;
            }
            avpkt.size -= len;
            avpkt.data += len;
        }
    }

    // some codecs, such as MPEG, transmit the I and P frame with a latency of one frame. You must do the following to have a chance to get the last frame of the video
    avpkt.data = 0;
    avpkt.size = 0;
    int len = avcodec_decode_video2(c, picture, &got_picture, &avpkt);
    if (got_picture)
    {
        debug_msg("saving last frame %3d.", frame);
        fflush(stdout);
        snprintf(buf, sizeof(buf), outfilename, frame);                                                 // the picture is allocated by the decoder. no need to free it
        pgm_save(picture->data[0], picture->linesize[0], c->width, c->height, buf);
        frame++;
    }

    fclose(f);
    avcodec_close(c);
    av_free(c);
    av_frame_free(&picture);
    printf("\n");
}

int main(int argc, char **argv)
{
    av_log_set_level(AV_LOG_VERBOSE); 
    av_log_set_callback(log_callback);

    avcodec_register_all();                                                                             // register all the codecs

    audio_encode_example("res/test.mp2");
    audio_decode_example("res/test.sw", "/tmp/test.mp2");
    video_encode_example("res/test.mpg");
    video_decode_example("res/test%d.pgm", "/tmp/test.mpg");

    return 0;
}
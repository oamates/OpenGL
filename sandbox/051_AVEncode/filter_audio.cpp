// This example will generate a sine wave audio, pass it through a simple filter chain, and then compute the MD5 checksum of the output data.
// The filter chain it uses is:
// (input) -> abuffer -> volume -> aformat -> abuffersink -> (output)

// abuffer: This provides the endpoint where you can feed the decoded samples.
// volume: In this example we hardcode it to 0.90.
// aformat: This converts the samples to the samplefreq, channel layout, and sample format required by the audio device.
// abuffersink: This provides the endpoint where you can read the samples after they have passed through the filter chain.

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.hpp"

extern "C"
{ 
#include "libavutil/channel_layout.h"
#include "libavutil/md5.h"
#include "libavutil/mem.h"
#include "libavutil/opt.h"
#include "libavutil/samplefmt.h"

#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
}

#define INPUT_SAMPLERATE     48000
#define INPUT_FORMAT         AV_SAMPLE_FMT_FLTP
#define INPUT_CHANNEL_LAYOUT AV_CH_LAYOUT_5POINT0
#define VOLUME_VAL           0.90

static int init_filter_graph(AVFilterGraph **graph, AVFilterContext **src, AVFilterContext **sink)
{
    AVFilterGraph   *filter_graph;
    AVFilterContext *abuffer_ctx;
    AVFilter        *abuffer;
    AVFilterContext *volume_ctx;
    AVFilter        *volume;
    AVFilterContext *aformat_ctx;
    AVFilter        *aformat;
    AVFilterContext *abuffersink_ctx;
    AVFilter        *abuffersink;

    AVDictionary *options_dict = 0;
    uint8_t options_str[1024];
    uint8_t ch_layout[64];

    filter_graph = avfilter_graph_alloc();                                                                                  // Create a new filtergraph, which will contain all the filters.
    if (!filter_graph)
    {
        debug_msg("Unable to create filter graph.");
        return AVERROR(ENOMEM);
    }
                                                                                                                        
    abuffer = avfilter_get_by_name("abuffer");                                                                              // Create the abuffer filter, it will be used for feeding the data into the graph.
    if (!abuffer)
    {
        debug_msg("Could not find the abuffer filter.");
        return AVERROR_FILTER_NOT_FOUND;
    }

    abuffer_ctx = avfilter_graph_alloc_filter(filter_graph, abuffer, "src");
    if (!abuffer_ctx)
    {
        debug_msg("Could not allocate the abuffer instance.");
        return AVERROR(ENOMEM);
    }

    av_get_channel_layout_string((char *)ch_layout, sizeof(ch_layout), 0, INPUT_CHANNEL_LAYOUT);                            // Set the filter options through the AVOptions API.
    av_opt_set    (abuffer_ctx, "channel_layout", (char *) ch_layout,                   AV_OPT_SEARCH_CHILDREN);
    av_opt_set    (abuffer_ctx, "sample_fmt",     av_get_sample_fmt_name(INPUT_FORMAT), AV_OPT_SEARCH_CHILDREN);
    av_opt_set_q  (abuffer_ctx, "time_base",      (AVRational){1, INPUT_SAMPLERATE},    AV_OPT_SEARCH_CHILDREN);
    av_opt_set_int(abuffer_ctx, "sample_rate",    INPUT_SAMPLERATE,                     AV_OPT_SEARCH_CHILDREN);
                                                                                                                            
    int err = avfilter_init_str(abuffer_ctx, 0);                                                                            // Now initialize the filter; we pass NULL options, since we have already set all the options above.
    if (err < 0) 
    {
        debug_msg("Could not initialize the abuffer filter.");
        return err;
    }
    
    volume = avfilter_get_by_name("volume");                                                                                // Create volume filter.
    if (!volume)
    {
        debug_msg("Could not find the volume filter.");
        return AVERROR_FILTER_NOT_FOUND;
    }

    volume_ctx = avfilter_graph_alloc_filter(filter_graph, volume, "volume");
    if (!volume_ctx)
    {
        debug_msg("Could not allocate the volume instance.");
        return AVERROR(ENOMEM);
    }
    
    av_dict_set(&options_dict, "volume", AV_STRINGIFY(VOLUME_VAL), 0);                                                      // A different way of passing the options is as key/value pairs in a dictionary.
    err = avfilter_init_dict(volume_ctx, &options_dict);
    av_dict_free(&options_dict);
    if (err < 0)
    {
        debug_msg("Could not initialize the volume filter.");
        return err;
    }
    
    aformat = avfilter_get_by_name("aformat");                                                                              // Create the aformat filter; it ensures that the output is of the format we want.
    if (!aformat) {
        fprintf(stderr, "Could not find the aformat filter.\n");
        return AVERROR_FILTER_NOT_FOUND;
    }

    aformat_ctx = avfilter_graph_alloc_filter(filter_graph, aformat, "aformat");
    if (!aformat_ctx)
    {
        debug_msg("Could not allocate the aformat instance.");
        return AVERROR(ENOMEM);
    }

    // A third way of passing the options is in a string of the form key1 = value1 : key2 = value2....
    snprintf((char*) options_str, sizeof(options_str), "sample_fmts = %s : sample_rates = %d : channel_layouts=0x%" PRIx64, av_get_sample_fmt_name(AV_SAMPLE_FMT_S16), 44100, (uint64_t)AV_CH_LAYOUT_STEREO);
    err = avfilter_init_str(aformat_ctx, (char*) options_str);
    if (err < 0)
    {
        av_log(0, AV_LOG_ERROR, "Could not initialize the aformat filter.\n");
        return err;
    }
    
    abuffersink = avfilter_get_by_name("abuffersink");                                                                      // Finally create the abuffersink filter; it will be used to get the filtered data out of the graph.
    if (!abuffersink) 
    {
        debug_msg("Could not find the abuffersink filter.");
        return AVERROR_FILTER_NOT_FOUND;
    }

    abuffersink_ctx = avfilter_graph_alloc_filter(filter_graph, abuffersink, "sink");
    if (!abuffersink_ctx)
    {
        debug_msg("Could not allocate the abuffersink instance.");
        return AVERROR(ENOMEM);
    }
    
    err = avfilter_init_str(abuffersink_ctx, 0);                                                                            // This filter takes no options.
    if (err < 0)
    {
        debug_msg("Could not initialize the abuffersink instance.");
        return err;
    }
    
    err = avfilter_link(abuffer_ctx, 0, volume_ctx, 0);                                                                     // Connect the filters, in this simple case the filters just form a linear chain.
    if (err >= 0) err = avfilter_link(volume_ctx, 0, aformat_ctx, 0);
    if (err >= 0) err = avfilter_link(aformat_ctx, 0, abuffersink_ctx, 0);
    if (err < 0)
    {
        debug_msg("Error connecting filters.");
        return err;
    }

    
    err = avfilter_graph_config(filter_graph, 0);                                                                           // Configure the graph.
    if (err < 0)
    {
        av_log(0, AV_LOG_ERROR, "Error configuring the filter graph\n");
        return err;
    }

    *graph = filter_graph;
    *src   = abuffer_ctx;
    *sink  = abuffersink_ctx;

    return 0;
}

static int process_output(struct AVMD5 *md5, AVFrame *frame)                                                                // Do something useful with the filtered data: this simple example just prints the MD5 checksum of each plane to stdout.
{
    int planar     = av_sample_fmt_is_planar((AVSampleFormat) frame->format);
    int channels   = av_get_channel_layout_nb_channels(frame->channel_layout);
    int planes     = planar ? channels : 1;
    int bps        = (int) av_get_bytes_per_sample((AVSampleFormat) frame->format);
    int plane_size = bps * frame->nb_samples * (planar ? 1 : channels);

    for (int i = 0; i < planes; i++) 
    {
        uint8_t checksum[16];

        av_md5_init(md5);
        av_md5_sum(checksum, frame->extended_data[i], plane_size);

        debug_msg("plane %d: 0x", i);
        for (int j = 0; j < sizeof(checksum); j++)
            debug_msg("%02X", checksum[j]);
    }
    return 0;
}


static int get_input(AVFrame *frame, int frame_num)                                                                         // Construct a frame of audio data to be filtered, this simple example just synthesizes a sine wave.
{
    #define FRAME_SIZE 1024    
    frame->sample_rate    = INPUT_SAMPLERATE;                                                                               // Set up the frame properties and allocate the buffer for the data.
    frame->format         = INPUT_FORMAT;
    frame->channel_layout = INPUT_CHANNEL_LAYOUT;
    frame->nb_samples     = FRAME_SIZE;
    frame->pts            = frame_num * FRAME_SIZE;

    int err = av_frame_get_buffer(frame, 0);
    if (err < 0)
        return err;
    
    for (int i = 0; i < 5; i++)                                                                                             // Fill the data for each channel.
    {
        float* data = (float*)frame->extended_data[i];

        for (int j = 0; j < frame->nb_samples; j++)
            data[j] = sin(2 * M_PI * (frame_num + j) * (i + 1) / FRAME_SIZE);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    AVFilterGraph *graph;
    AVFilterContext *src, *sink;
    AVFrame *frame;
    uint8_t errstr[1024];
    int err, i;

    if (argc < 2)
        exit_msg("Usage: %s <duration>", argv[0]);

    float duration = atof(argv[1]);
    int nb_frames = duration * INPUT_SAMPLERATE / FRAME_SIZE;
    if (nb_frames <= 0) 
        exit_msg("Invalid duration: %s", argv[1])

    avfilter_register_all();
    
    frame  = av_frame_alloc();                                                                                              // Allocate the frame we will be using to store the data.
    if (!frame)
        exit_msg("Error allocating the frame.");

    struct AVMD5* md5 = av_md5_alloc();
    if (!md5)
        exit_msg("Error allocating the MD5 context.");
    
    err = init_filter_graph(&graph, &src, &sink);                                                                           // Set up the filtergraph.
    if (err < 0)
    {
        debug_msg("Unable to init filter graph:");
        goto fail;
    }
    
    for (i = 0; i < nb_frames; i++)                                                                                         // the main filtering loop
    {
        err = get_input(frame, i);                                                                                          // get an input frame to be filtered
        if (err < 0)
        {
            debug_msg("Error generating input frame:");
            goto fail;
        }
        
        err = av_buffersrc_add_frame(src, frame);                                                                           // Send the frame to the input of the filtergraph.
        if (err < 0)
        {
            av_frame_unref(frame);
            debug_msg("Error submitting the frame to the filtergraph:");
            goto fail;
        }
        
        while ((err = av_buffersink_get_frame(sink, frame)) >= 0)                                                           // Get all the filtered output that is available.
        {            
            err = process_output(md5, frame);                                                                               // now do something with our filtered frame
            if (err < 0)
            {
                debug_msg("Error processing the filtered frame:");
                goto fail;
            }
            av_frame_unref(frame);
        }

        if (err == AVERROR(EAGAIN))                                                                                         // Need to feed more frames in.    
            continue;
        else if (err == AVERROR_EOF)                                                                                        // Nothing more to do, finish.            
            break;
        else if (err < 0)
        {
            debug_msg("Error filtering the data:");
            goto fail;
        }
    }

    avfilter_graph_free(&graph);
    av_frame_free(&frame);
    av_freep(&md5);
    return 0;

  fail:
    av_strerror(err, (char*) errstr, sizeof(errstr));
    exit_msg("%s", (char*) errstr);
}

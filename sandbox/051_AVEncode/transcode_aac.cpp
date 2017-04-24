// Simple audio converter. Converts an input audio file to AAC in an MP4 container using Libav.

#include <stdio.h>

extern "C"
{ 
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavcodec/avcodec.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/avstring.h"
#include "libavutil/frame.h"
#include "libavutil/opt.h"
#include "libavresample/avresample.h"
}

#include "log.hpp"


#define OUTPUT_BIT_RATE 48000                                                                                   // The output bit rate in kbit/s
#define OUTPUT_CHANNELS 2                                                                                       // The number of output channels
#define OUTPUT_SAMPLE_FORMAT AV_SAMPLE_FMT_S16                                                                  // The audio sample output format

static char* const get_error_text(const int error)                                                              // Convert an error code into a text message.
{
    static char error_buffer[255];
    av_strerror(error, error_buffer, sizeof(error_buffer));
    return error_buffer;
}

// Open an input file and the required decoder.
static int open_input_file(const char *filename, AVFormatContext **input_format_context, AVCodecContext **input_codec_context)
{
    AVCodec *input_codec;
    int error;
    
    if ((error = avformat_open_input(input_format_context, filename, 0, 0)) < 0)                                // Open the input file to read from it.
    {
        debug_msg("Could not open input file '%s' (error '%s')\n", filename, get_error_text(error));
        *input_format_context = 0;
        return error;
    }
    
    if ((error = avformat_find_stream_info(*input_format_context, 0)) < 0)                                      // Get information on the input file (number of streams etc.).
    {
        debug_msg("Could not open find stream info (error '%s')", get_error_text(error));
        avformat_close_input(input_format_context);
        return error;
    }

    if ((*input_format_context)->nb_streams != 1)
    {
        fprintf(stderr, "Expected one audio input stream, but found %d.", (*input_format_context)->nb_streams); // Make sure that there is only one stream in the input file.
        avformat_close_input(input_format_context);
        return AVERROR_EXIT;
    }

    
    if (!(input_codec = avcodec_find_decoder((*input_format_context)->streams[0]->codec->codec_id)))            // Find a decoder for the audio stream.
    {
        debug_msg("Could not find input codec.");
        avformat_close_input(input_format_context);
        return AVERROR_EXIT;
    }

    if ((error = avcodec_open2((*input_format_context)->streams[0]->codec, input_codec, 0)) < 0)                // Open the decoder for the audio stream to use it later.
    {
        debug_msg("Could not open input codec (error '%s')", get_error_text(error));
        avformat_close_input(input_format_context);
        return error;
    }
    
    *input_codec_context = (*input_format_context)->streams[0]->codec;                                          // Save the decoder context for easier access later.

    return 0;
}

// Open an output file and the required encoder. Also set some basic encoder parameters. Some of these parameters are based on the input file's parameters.
static int open_output_file(const char *filename, AVCodecContext *input_codec_context, AVFormatContext **output_format_context, AVCodecContext **output_codec_context)
{
    AVIOContext *output_io_context = 0;
    AVStream *stream               = 0;
    AVCodec *output_codec          = 0;
    int error;
    
    if ((error = avio_open(&output_io_context, filename, AVIO_FLAG_WRITE)) < 0)                                 // Open the output file to write to it.
    {
        debug_msg("Could not open output file '%s' (error '%s')\n", filename, get_error_text(error));
        return error;
    }
    
    if (!(*output_format_context = avformat_alloc_context()))                                                   // Create a new format context for the output container format.
    {
        debug_msg("Could not allocate output format context.");
        return AVERROR(ENOMEM);
    }
    
    (*output_format_context)->pb = output_io_context;                                                           // Associate the output file (pointer) with the container format context.

    if (!((*output_format_context)->oformat = av_guess_format(0, filename, 0)))                                 // Guess the desired container format based on the file extension.
    {
        debug_msg("Could not find output file format.");
        goto cleanup;
    }

    av_strlcpy((*output_format_context)->filename, filename, sizeof((*output_format_context)->filename));
    
    if (!(output_codec = avcodec_find_encoder(AV_CODEC_ID_AAC)))                                                // Find the encoder to be used by its name.
    {
        debug_msg("Could not find an AAC encoder.");
        goto cleanup;
    }

    if (!(stream = avformat_new_stream(*output_format_context, output_codec)))                                  // Create a new audio stream in the output file container.
    {
        debug_msg("Could not create new stream.");
        error = AVERROR(ENOMEM);
        goto cleanup;
    }
    
    *output_codec_context = stream->codec;                                                                      // Save the encoder context for easiert access later.
    (*output_codec_context)->channels       = OUTPUT_CHANNELS;                                                  // Set the basic encoder parameters. The input file's sample rate is used to avoid a sample rate conversion.
    (*output_codec_context)->channel_layout = av_get_default_channel_layout(OUTPUT_CHANNELS);
    (*output_codec_context)->sample_rate    = input_codec_context->sample_rate;
    (*output_codec_context)->sample_fmt     = AV_SAMPLE_FMT_S16;
    (*output_codec_context)->bit_rate       = OUTPUT_BIT_RATE;
    
    if ((*output_format_context)->oformat->flags & AVFMT_GLOBALHEADER)                                          // Some container formats (like MP4) require global headers to be present. Mark the encoder so that it behaves accordingly.
        (*output_codec_context)->flags |= CODEC_FLAG_GLOBAL_HEADER;

    if ((error = avcodec_open2(*output_codec_context, output_codec, NULL)) >= 0) return 0;                      // Open the encoder for the audio stream to use it later.
    debug_msg("Could not open output codec (error '%s')", get_error_text(error));

cleanup:
    avio_close((*output_format_context)->pb);
    avformat_free_context(*output_format_context);
    *output_format_context = 0;
    return (error < 0) ? error : AVERROR_EXIT;
}

static void init_packet(AVPacket *packet)                                                                       // Initialize one data packet for reading or writing.
{
    av_init_packet(packet);    
    packet->data = 0;                                                                                           // Set the packet data and size so that it is recognized as being empty.
    packet->size = 0;
}

static int init_input_frame(AVFrame **frame)                                                                    // Initialize one audio frame for reading from the input file
{
    if (*frame = av_frame_alloc()) return 0;
    debug_msg("Could not allocate input frame.");
    return AVERROR(ENOMEM);
}

// Initialize the audio resampler based on the input and output codec settings. If the input and output sample formats differ, 
// a conversion is required libavresample takes care of this, but requires initialization.

static int init_resampler(AVCodecContext *input_codec_context, AVCodecContext *output_codec_context, AVAudioResampleContext **resample_context)
{
    // Only initialize the resampler if it is necessary, i.e if and only if the sample formats differ.
    if (input_codec_context->sample_fmt != output_codec_context->sample_fmt || input_codec_context->channels != output_codec_context->channels)
    {        
        if (!(*resample_context = avresample_alloc_context()))                                                  // Create a resampler context for the conversion.
        {
            debug_msg("Could not allocate resample context.");
            return AVERROR(ENOMEM);
        }

        // Set the conversion parameters. Default channel layouts based on the number of channels are assumed for simplicity (they are sometimes not detected properly by the demuxer and/or decoder).
        av_opt_set_int(*resample_context, "in_channel_layout", av_get_default_channel_layout(input_codec_context->channels), 0);
        av_opt_set_int(*resample_context, "out_channel_layout", av_get_default_channel_layout(output_codec_context->channels), 0);
        av_opt_set_int(*resample_context, "in_sample_rate", input_codec_context->sample_rate, 0);
        av_opt_set_int(*resample_context, "out_sample_rate", output_codec_context->sample_rate, 0);
        av_opt_set_int(*resample_context, "in_sample_fmt", input_codec_context->sample_fmt, 0);
        av_opt_set_int(*resample_context, "out_sample_fmt", output_codec_context->sample_fmt, 0);
        
        int error = avresample_open(*resample_context);
        if (error < 0)                                                                                          // Open the resampler with the specified parameters.
        {
            fprintf(stderr, "Could not open resample context\n");
            avresample_free(resample_context);
            return error;
        }
    }
    return 0;
}

static int init_fifo(AVAudioFifo **fifo)                                                                        // Initialize a FIFO buffer for the audio samples to be encoded.
{
    if (*fifo = av_audio_fifo_alloc(OUTPUT_SAMPLE_FORMAT, OUTPUT_CHANNELS, 1)) return 0;                        // Create the FIFO buffer based on the specified output sample format.
    debug_msg("Could not allocate FIFO.");
    return AVERROR(ENOMEM);
}

static int write_output_file_header(AVFormatContext *output_format_context)                                     // Write the header of the output file container.
{
    int error = avformat_write_header(output_format_context, 0);
    if (error >= 0) return 0; 
    debug_msg("Could not write output file header (error '%s')", get_error_text(error));
    return error;
}

// Decode one audio frame from the input file.
static int decode_audio_frame(AVFrame *frame, AVFormatContext *input_format_context, AVCodecContext *input_codec_context, int *data_present, int *finished)
{    
    AVPacket input_packet;                                                                                      // Packet used for temporary storage.
    int error;
    init_packet(&input_packet);
    
    if ((error = av_read_frame(input_format_context, &input_packet)) < 0)                                       // Read one audio frame from the input file into a temporary packet.
    {
        
        if (error == AVERROR_EOF)                                                                               // If we are the the end of the file, flush the decoder below.
            *finished = 1;
        else 
        {
            debug_msg("Could not read frame (error '%s')", get_error_text(error));
            return error;
        }
    }

    // Decode the audio frame stored in the temporary packet. The input audio stream decoder is used to do this. If we are at the end of the file, pass an empty packet to the decoder to flush it.
    if ((error = avcodec_decode_audio4(input_codec_context, frame, data_present, &input_packet)) < 0)
    {
        debug_msg("Could not decode frame (error '%s')", get_error_text(error));
        av_free_packet(&input_packet);
        return error;
    }

    if (*finished && *data_present)                                                                             // If the decoder has not been flushed completely, we are not finished, so that this function has to be called again.
        *finished = 0;
    av_free_packet(&input_packet);
    return 0;
}

// Initialize a temporary storage for the specified number of audio samples. The conversion requires temporary storage due to the different format.
// The number of audio samples to be allocated is specified in frame_size.
static int init_converted_samples(uint8_t ***converted_input_samples, AVCodecContext *output_codec_context, int frame_size)
{
    // Allocate as many pointers as there are audio channels. Each pointer will later point to the audio samples of the corresponding channels (although it may be 0 for interleaved formats).
    if (!(*converted_input_samples = (uint8_t**)calloc(output_codec_context->channels, sizeof(**converted_input_samples)))) 
    {
        debug_msg("Could not allocate converted input sample pointers.");
        return AVERROR(ENOMEM);
    }

    // Allocate memory for the samples of all channels in one consecutive block for convenience.
    int error = av_samples_alloc(*converted_input_samples, 0, output_codec_context->channels, frame_size, output_codec_context->sample_fmt, 0);
    if (error >= 0) return 0;
    debug_msg("Could not allocate converted input samples (error '%s')", get_error_text(error));
    av_freep(&(*converted_input_samples)[0]);
    free(*converted_input_samples);
    return error;
}

// Convert the input audio samples into the output sample format. The conversion happens on a per-frame basis, the size of which is specified  by frame_size.
static int convert_samples(uint8_t **input_data, uint8_t **converted_data, const int frame_size, AVAudioResampleContext *resample_context)
{
    // Convert the samples using the resampler.
    int error = avresample_convert(resample_context, converted_data, 0, frame_size, input_data, 0, frame_size);
    if (error < 0) 
    {
        debug_msg("Could not convert input samples (error '%s')", get_error_text(error));
        return error;
    }

    // Perform a sanity check so that the number of converted samples is not greater than the number of samples to be converted.
    // If the sample rates differ, this case has to be handled differently
    if (!avresample_available(resample_context)) return 0;
    debug_msg("Converted samples left over.");
    return AVERROR_EXIT;
}

// Add converted input audio samples to the FIFO buffer for later processing.
static int add_samples_to_fifo(AVAudioFifo *fifo, uint8_t **converted_input_samples, const int frame_size)
{
    // Make the FIFO as large as it needs to be to hold both, the old and the new samples.
    int error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + frame_size);
    if (error < 0) 
    {
        debug_msg("Could not reallocate FIFO.");
        return error;
    }

    // Store the new samples in the FIFO buffer.
    if (av_audio_fifo_write(fifo, (void **)converted_input_samples, frame_size) >= frame_size) return 0;
    debug_msg("Could not write data to FIFO.");
    return AVERROR_EXIT;
}

// Read one audio frame from the input file, decodes, converts and stores it in the FIFO buffer.
static int read_decode_convert_and_store(AVAudioFifo *fifo, AVFormatContext *input_format_context, AVCodecContext *input_codec_context, AVCodecContext *output_codec_context, AVAudioResampleContext *resampler_context, int *finished)
{
    AVFrame *input_frame = 0;                                                                                   // Temporary storage of the input samples of the frame read from the file.
    uint8_t **converted_input_samples = 0;                                                                      // Temporary storage for the converted input samples.
    int data_present;
    int ret = AVERROR_EXIT;
    
    if (init_input_frame(&input_frame)) goto cleanup;                                                           // Initialize temporary storage for one input frame.    
    if (decode_audio_frame(input_frame, input_format_context, input_codec_context, &data_present, finished))    // Decode one frame worth of audio samples.
        goto cleanup;
    // If we are at the end of the file and there are no more samples in the decoder which are delayed, we are actually finished. This must not be treated as an error.
    if (*finished && !data_present) 
    {
        ret = 0;
        goto cleanup;
    }
    
    if (data_present)                                                                                           // If there is decoded data, convert and store it
    {
        if (init_converted_samples(&converted_input_samples, output_codec_context, input_frame->nb_samples))    // Initialize the temporary storage for the converted input samples.
            goto cleanup;
        // Convert the input samples to the desired output sample format.This requires a temporary storage provided by converted_input_samples.
        if (convert_samples(input_frame->extended_data, converted_input_samples, input_frame->nb_samples, resampler_context)) goto cleanup;
        if (add_samples_to_fifo(fifo, converted_input_samples, input_frame->nb_samples))                        // Add the converted input samples to the FIFO buffer for later processing.
            goto cleanup;
    }
    ret = 0;

  cleanup:
    if (converted_input_samples)
    {
        av_freep(&converted_input_samples[0]);
        free(converted_input_samples);
    }
    av_frame_free(&input_frame);
    return ret;
}

// Initialize one input frame for writing to the output file. The frame will be exactly frame_size samples large.
static int init_output_frame(AVFrame **frame, AVCodecContext *output_codec_context, int frame_size)
{    
    if (!(*frame = av_frame_alloc()))                                                                           // Create a new frame to store the audio samples.
    {
        debug_msg("Could not allocate output frame.");
        return AVERROR_EXIT;
    }

    // Set the frame's parameters, especially its size and format. av_frame_get_buffer needs this to allocate memory for the audio samples of the frame.
    // Default channel layouts based on the number of channels are assumed for simplicity.
    (*frame)->nb_samples     = frame_size;
    (*frame)->channel_layout = output_codec_context->channel_layout;
    (*frame)->format         = output_codec_context->sample_fmt;
    (*frame)->sample_rate    = output_codec_context->sample_rate;

    // Allocate the samples of the created frame. This call will make sure that the audio frame can hold as many samples as specified.
    int error = av_frame_get_buffer(*frame, 0); 
    if (error >= 0) return 0;
    debug_msg("Could allocate output frame samples (error '%s')", get_error_text(error));
    av_frame_free(frame);
    return error;
}

// Encode one frame worth of audio to the output file.
static int encode_audio_frame(AVFrame *frame, AVFormatContext *output_format_context, AVCodecContext *output_codec_context, int *data_present)
{
    // Packet used for temporary storage.
    AVPacket output_packet;
    init_packet(&output_packet);

    // Encode the audio frame and store it in the temporary packet. The output audio stream encoder is used to do this.
    int error;
    if ((error = avcodec_encode_audio2(output_codec_context, &output_packet, frame, data_present)) < 0)
    {
        debug_msg("Could not encode frame (error '%s')", get_error_text(error));
        av_free_packet(&output_packet);
        return error;
    }

    if (*data_present)                                                                                          // Write one audio frame from the temporary packet to the output file.
    {
        if ((error = av_write_frame(output_format_context, &output_packet)) < 0)
        {
            debug_msg("Could not write frame (error '%s')", get_error_text(error));
            av_free_packet(&output_packet);
            return error;
        }
        av_free_packet(&output_packet);
    }
    return 0;
}

// Load one audio frame from the FIFO buffer, encode and write it to the output file.
static int load_encode_and_write(AVAudioFifo *fifo, AVFormatContext *output_format_context, AVCodecContext *output_codec_context)
{    
    AVFrame *output_frame;                                                                                      // Temporary storage of the output samples of the frame written to the file.
    // Use the maximum number of possible samples per frame. If there is less than the maximum possible frame size in the FIFO buffer use this number. Otherwise, use the maximum possible frame size
    const int frame_size = FFMIN(av_audio_fifo_size(fifo), output_codec_context->frame_size);
    int data_written;

    
    if (init_output_frame(&output_frame, output_codec_context, frame_size))                                     // Initialize temporary storage for one output frame.
        return AVERROR_EXIT;

    // Read as many samples from the FIFO buffer as required to fill the frame. The samples are stored in the frame temporarily.
    if (av_audio_fifo_read(fifo, (void **)output_frame->data, frame_size) < frame_size)
    {
        debug_msg("Could not read data from FIFO.");
        av_frame_free(&output_frame);
        return AVERROR_EXIT;
    }
    
    if (encode_audio_frame(output_frame, output_format_context, output_codec_context, &data_written))           // Encode one frame worth of audio samples.
    {
        av_frame_free(&output_frame);
        return AVERROR_EXIT;
    }
    av_frame_free(&output_frame);
    return 0;
}

static int write_output_file_trailer(AVFormatContext *output_format_context)                                    // Write the trailer of the output file container.
{
    int error = av_write_trailer(output_format_context);
    if (error >= 0) return 0;
    debug_msg("Could not write output file trailer (error '%s')", get_error_text(error));
    return error;
}

int main(int argc, char **argv)                                                                                 // Convert an audio file to an AAC file in an MP4 container.
{
    AVFormatContext *input_format_context = 0, *output_format_context = 0;
    AVCodecContext *input_codec_context = 0, *output_codec_context = 0;
    AVAudioResampleContext* resample_context = 0;
    AVAudioFifo* fifo = 0;
    int ret = AVERROR_EXIT;

    if (argc < 3)
        exit_msg("Usage: %s <input file> <output file>\n", argv[0]);
    
    av_register_all();                                                                                          // Register all codecs and formats so that they can be used.
    
    if (open_input_file(argv[1], &input_format_context, &input_codec_context)) goto cleanup;                    // Open the input file for reading.    
    if (open_output_file(argv[2], input_codec_context, &output_format_context, &output_codec_context))          // Open the output file for writing.    
        goto cleanup;
    if (init_resampler(input_codec_context, output_codec_context, &resample_context)) goto cleanup;             // Initialize the resampler to be able to convert audio sample formats.
    if (init_fifo(&fifo)) goto cleanup;                                                                         // Initialize the FIFO buffer to store audio samples to be encoded.
    if (write_output_file_header(output_format_context)) goto cleanup;                                          // Write the header of the output file container.

    // Loop as long as we have input samples to read or output samples to write; abort as soon as we have neither.
    while (1) 
    {
        const int output_frame_size = output_codec_context->frame_size;                                         // Use the encoder's desired frame size for processing.
        int finished                = 0;

        // Make sure that there is one frame worth of samples in the FIFO buffer so that the encoder can do its work. Since the decoder's and the encoder's frame size may differ, we
        // need to FIFO buffer to store as many frames worth of input samples that they make up at least one frame worth of output samples.
        while (av_audio_fifo_size(fifo) < output_frame_size)
        {
            // Decode one frame worth of audio samples, convert it to the output sample format and put it into the FIFO buffer.
            if (read_decode_convert_and_store(fifo, input_format_context, input_codec_context, output_codec_context, resample_context, &finished)) goto cleanup;            
            if (finished) break;                                                                                // If we are at the end of the input file, we continue encoding the remaining audio samples to the output file.
        }

        // If we have enough samples for the encoder, we encode them. At the end of the file, we pass the remaining samples to the encoder.
        while (av_audio_fifo_size(fifo) >= output_frame_size || (finished && av_audio_fifo_size(fifo) > 0))            
            if (load_encode_and_write(fifo, output_format_context, output_codec_context)) goto cleanup;         // Take one frame worth of audio samples from the FIFO buffer, encode it and write it to the output file.

        // If we are at the end of the input file and have encoded all remaining samples, we can exit this loop and finish.
        if (finished)
        {
            int data_written;
            
            do {                                                                                                // Flush the encoder as it may have delayed frames.
                if (encode_audio_frame(0, output_format_context, output_codec_context, &data_written)) goto cleanup;
            } while (data_written);
            break;
        }
    }

    // Write the trailer of the output file container.
    if (write_output_file_trailer(output_format_context)) goto cleanup;
    ret = 0;

cleanup:
    if (fifo) av_audio_fifo_free(fifo);
    if (resample_context)
    {
        avresample_close(resample_context);
        avresample_free(&resample_context);
    }
    if (output_codec_context) avcodec_close(output_codec_context);
    if (output_format_context)
    {
        avio_close(output_format_context->pb);
        avformat_free_context(output_format_context);
    }
    if (input_codec_context) avcodec_close(input_codec_context);
    if (input_format_context) avformat_close_input(&input_format_context);
    return ret;
}
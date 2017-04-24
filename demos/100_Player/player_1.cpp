//========================================================================================================================================================================================================================
// DEMO 100.1: OpenGL Video Player
//========================================================================================================================================================================================================================
#include <cstdio>

extern "C" 
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

#include "log.hpp"

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame)
{
    char szFilename[32];
    int  y;
    
    sprintf(szFilename, "frame%d.ppm", iFrame);                                                             // Open file
    FILE *pFile = fopen(szFilename, "wb");
    if (!pFile)
    {
        debug_msg("Cannot open %s.\n", szFilename);
        return;
    }
    
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);                                                      // Write header
    
    for(y = 0; y < height; y++)                                                                             // Write pixel data
        fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);
    
    fclose(pFile);                                                                                          // Close file
}

int main(int argc, char *argv[])
{    
    AVFormatContext *pFormatCtx = 0;                                                                        // Initalizing these to null pointer prevents segfaults!

    if(argc < 2)
        exit_msg("Please provide a movie file\n");
    
    av_register_all();                                                                                      // Register all formats and codecs
    
    if(avformat_open_input(&pFormatCtx, argv[1], 0, 0) != 0)                                                // Open video file
        exit_msg("Cannot open video file : %s", argv[1]);
    
    if(avformat_find_stream_info(pFormatCtx, 0) < 0)                                                        // Retrieve stream information
        exit_msg("Couldn't find stream information.\n");
    
    av_dump_format(pFormatCtx, 0, argv[1], 0);                                                              // Dump information about file onto standard error
    
    int videoStream = -1;
    for(int i = 0; i < pFormatCtx->nb_streams; i++)                                                         // Find the first video stream
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            break;
        }

    if (videoStream == -1)
        exit_msg("Source file does not contain any video stream"); 
    
    AVCodecParameters *pCodecParameters = pFormatCtx->streams[videoStream]->codecpar;                       // Get a pointer to the codec context for the video stream
    
    AVCodec *pCodec = avcodec_find_decoder(pCodecParameters->codec_id);                                     // Find the decoder for the video stream
    if(pCodec == 0)
        exit_msg("Cannot find appropriate codec!\n");
    
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);                                             // Copy context
    if(avcodec_parameters_to_context(pCodecCtx, pCodecParameters) != 0)
        exit_msg("Couldn't copy codec context");
    
    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0)                                                          // Open codec
        exit_msg("Could not open codec");
    
    AVFrame *pFrame = av_frame_alloc();                                                                     // Allocate video frame
    AVFrame *pFrameRGB = av_frame_alloc();                                                                  // Allocate an AVFrame structure
    if((pFrame == NULL) || (pFrameRGB == NULL))
        exit_msg("Could not allocate frame");
    
    int bufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);    // Determine the size in bytes required to store an image with the given parameters ...
    uint8_t *buffer = (uint8_t*) av_malloc(bufferSize * sizeof(uint8_t));                                   // and allocate the buffer
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);
    
    SwsContext *sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,                               // initialize SWS context for software scaling
                                         pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                                         AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
    
    int f = 0;
    AVPacket packet;

    while(av_read_frame(pFormatCtx, &packet) >= 0)                                                          // Read frames and save first five frames to disk
    {
        if(packet.stream_index == videoStream)                                                              // Is this a packet from the video stream?
        {
            avcodec_send_packet(pCodecCtx, &packet);
            while(avcodec_receive_frame(pCodecCtx, pFrame) >= 0)                                            // Decode video frame
            {
                sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data, pFrame->linesize, 0,              // Convert the image from its native format to RGB
                          pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
      
                if(++f <= 5)                                                                                // Save the frame to disk
                    SaveFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height, f);
            }
        }
        av_packet_unref(&packet);                                                                           // Wipe the packet. Unreference the buffer referenced by the packet and reset the remaining packet fields to their default values
    }
  
    av_free(buffer);                                                                                        // Free the RGB image
    av_frame_free(&pFrameRGB);
    av_frame_free(&pFrame);                                                                                 // Free the YUV frame
    avcodec_close(pCodecCtx);                                                                               // Close the codecs
    avcodec_parameters_free(&pCodecParameters);                                                             // Free an AVCodecParameters instance and everything associated with it and write NULL to the supplied pointer
    avformat_close_input(&pFormatCtx);                                                                      // Close the video file
    return 0;
}
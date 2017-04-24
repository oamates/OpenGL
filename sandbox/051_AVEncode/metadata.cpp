// Shows how the metadata API can be used in application programs.

#include <stdio.h>

extern "C"
{ 
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
}

#include "log.hpp"


int main (int argc, char **argv)
{
    AVFormatContext* fmt_ctx = 0;
    AVDictionaryEntry* tag = 0;

    if (argc != 2)
        exit_msg("usage: %s <input_file>\nexample program to demonstrate the use of the libavformat metadata API.\n", argv[0]);

    av_register_all();
    int ret = avformat_open_input(&fmt_ctx, argv[1], 0, 0);
    if (ret) return ret;

    while ((tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
        printf("%s = %s\n", tag->key, tag->value);

    avformat_free_context(fmt_ctx);
    return 0;
}

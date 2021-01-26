#include <iostream>
extern "C"{
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/file.h"
#include "libavutil/avutil.h"
}
#include "InputFile.h"

static int read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    struct buffer_data *bd = (struct buffer_data *)opaque;
    buf_size = FFMIN(buf_size, bd->size);

    if (!buf_size)
        return AVERROR_EOF;
    printf("ptr:%p size:%zu\n", bd->ptr, bd->size);

    /* copy internal buffer data to buf */
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr  += buf_size;
    bd->size -= buf_size;

    return buf_size;
}

void InputFile::openInputFile(const char* path){

	input_filename = path;

	int ret = 0;
	/* slurp file content into buffer */
	ret = av_file_map(input_filename, &buffer, &buffer_size, 0, NULL);
	if (ret < 0){
		fprintf(stderr, "file map error \n");
	}

	/* fill opaque structure used by the AVIOContext read callback */
	bd.ptr  = buffer;
	bd.size = buffer_size;

	if (!(fmt_ctx = avformat_alloc_context())) {
		ret = AVERROR(ENOMEM);
		fprintf(stderr, "file map error \n");
		return;
	}

	avio_ctx_buffer = av_malloc(avio_ctx_buffer_size);
	if (!avio_ctx_buffer) {
		ret = AVERROR(ENOMEM);
		fprintf(stderr, "file map error \n");
	}
	avio_ctx = avio_alloc_context((unsigned char*)avio_ctx_buffer, avio_ctx_buffer_size,
			0, &bd, &read_packet, NULL, NULL);
	if (!avio_ctx) {
		ret = AVERROR(ENOMEM);
		return ;
	}
	fmt_ctx->pb = avio_ctx;

	ret = avformat_open_input(&fmt_ctx, NULL, NULL, NULL);
	if (ret < 0) {
		fprintf(stderr, "Could not open input\n");
		return ;
	}

	ret = avformat_find_stream_info(fmt_ctx, NULL);
	if (ret < 0) {
		fprintf(stderr, "Could not find stream information\n");
		return ;
	}

	av_dump_format(fmt_ctx, 0, input_filename, 0);

}

void InputFile::clearInpute(){
	avformat_close_input(&fmt_ctx);

	/* note: the internal buffer could have changed, and be != avio_ctx_buffer */
	if (avio_ctx)
		av_freep(&avio_ctx->buffer);
	avio_context_free(&avio_ctx);

	av_file_unmap(buffer, buffer_size);

	if (ret < 0) {
		fprintf(stderr, "Error occurred: " );
	}
}


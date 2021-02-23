#include <iostream>
extern "C"{
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/file.h"
#include "libavutil/avutil.h"
}
#include "InputFile.h"

InputFile::InputFile(const char* path){

	input_file = path;

	int ret = 0;
	if (!(fmt_ctx = avformat_alloc_context())) {
		ret = AVERROR(ENOMEM);
		fprintf(stderr, "file map error \n");
		return;
	}

	ret = avformat_open_input(&fmt_ctx, input_file , NULL, NULL);
	if (ret < 0) {
		fprintf(stderr, "Could not open input\n");
		return ;
	}

	ret = avformat_find_stream_info(fmt_ctx, NULL);
	if (ret < 0) {
		fprintf(stderr, "Could not find stream information\n");
		return ;
	}

	av_dump_format(fmt_ctx, 0, input_file, 0);


}


InputFile::~InputFile(){
	avformat_close_input(&fmt_ctx);
	fmt_ctx = NULL;
}




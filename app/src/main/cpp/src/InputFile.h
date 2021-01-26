#ifndef _H_INPUT_FILE_H_
#define _H_INPUT_FILE_H_
#include <iostream>
extern "C"{
#include <libavformat/avformat.h>
}

struct buffer_data {
	uint8_t *ptr;
	size_t size; ///< size left in the buffer
};

class InputFile{
	public:

		AVFormatContext *fmt_ctx = NULL;
		AVIOContext *avio_ctx = NULL;
		uint8_t *buffer = NULL;
		void *avio_ctx_buffer = NULL;
		size_t buffer_size, avio_ctx_buffer_size = 4096;
		const char *input_filename = NULL;
		int ret = 0;
		struct buffer_data bd = { 0 };



		void openInputFile(const char* path);


		void clearInpute();
};

#endif

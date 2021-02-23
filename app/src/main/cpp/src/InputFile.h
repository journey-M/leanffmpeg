#ifndef _H_INPUT_FILE_H_
#define _H_INPUT_FILE_H_

#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
}

struct buffer_data {
    uint8_t *ptr;
    size_t size; ///< size left in the buffer
};

class InputFile {

public:

    AVFormatContext *fmt_ctx = NULL;
    const char *input_file = NULL;

    InputFile(const char* input_fname);
    ~InputFile();


};

#endif

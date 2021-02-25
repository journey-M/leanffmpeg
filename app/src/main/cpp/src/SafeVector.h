#ifndef _H__SAFE_VECTOR__H_
#define _H__SAFE_VECTOR__H_

#include "InputFile.h"
#include "VideoState.h"
#include <cstdint>
#include <vector>
#include <thread>
#include <mutex>


extern "C" {
#include <libyuv.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
template <class T>
class SafeVector {

private:
    std::vector<T> elems;     // 元素

    pthread_mutex_t mutex_list = PTHREAD_MUTEX_INITIALIZER;


public:

    SafeVector();

    ~SafeVector();

    void push_value(T t);

    T pop_value();

    int getSize();


};


#endif

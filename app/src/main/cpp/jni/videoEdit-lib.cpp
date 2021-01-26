#include <string>
#include <jni.h>
#include "native_log.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <pthread.h>
#include <unistd.h>
#include <android/bitmap.h>
#include "../src/Decoder.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

Decoder *decoder;
InputFile *inputFile;


extern "C"
JNIEXPORT jint
JNICALL
Java_gwj_dev_ffmpeg_videoEdit_VideoAPI_openVideoFile(JNIEnv *env, jobject
thiz, jstring jpath
) {
    const char *path = (env)->GetStringUTFChars(jpath, 0);
    if (strlen(path) == 0) {
        FFLOGE("请输入视频路径");
        return -1;
    }

    avformat_network_init();
    //first check and open InputFile
    inputFile = new InputFile();
    inputFile->openInputFile(path);

    decoder = new Decoder(inputFile);
    return 0;
}

extern "C"
JNIEXPORT jobject

JNICALL
Java_gwj_dev_ffmpeg_videoEdit_VideoAPI_getVideoPreviews(JNIEnv *env, jobject
thiz, jint start, jint interval, int maxCount) {
    if (!decoder) {
        FFLOGE("Decoder 未初始化");
        return NULL;
    }
    Options opt;
    opt.start = start;
    opt.per = interval;
    vector<FrameImage *> vectorList;
    int ret = -1;
    ret = decoder->getVideoImages(&opt, &vectorList, maxCount);
    if (ret <= 0 || vectorList.size() == 0) {
        FFLOGE("解码失败s");
        return NULL;
    }

    jclass class_arraylist = (env)->FindClass("java/util/ArrayList");
    jmethodID arraylist_construct_method = (env)->GetMethodID(class_arraylist, "<init>", "()V");
    jobject obj_arraylist = (env)->NewObject(class_arraylist, arraylist_construct_method);
    jmethodID arraylist_add_method = (env)->GetMethodID(class_arraylist, "add",
                                                        "(Ljava/lang/Object;)Z");


    for (int i = 0; i < vectorList.size(); ++i) {

        FrameImage *fImage = vectorList.at(i);

        //Bitmap.Config config = Bitmap.Config.valueOf(String configName);
        jstring configName = env->NewStringUTF("ARGB_8888");
        jclass bitmapConfigClass = env->FindClass("android/graphics/Bitmap$Config");
        jmethodID valueOfBitmapConfigFunction = env->GetStaticMethodID(bitmapConfigClass, "valueOf",
                                                                       "(Ljava/lang/Class;Ljava/lang/String;)Ljava/lang/Enum;");
        jobject bitmapConfig = env->CallStaticObjectMethod(bitmapConfigClass,
                                                           valueOfBitmapConfigFunction,
                                                           bitmapConfigClass, configName);

        // Bitmap newBitmap = Bitmap.createBitmap(int width,int height,Bitmap.Config config);
        jclass bitmap = env->FindClass("android/graphics/Bitmap");
        jmethodID createBitmapFunction = env->GetStaticMethodID(bitmap, "createBitmap",
                                                                "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
        jobject newBitmap = env->CallStaticObjectMethod(bitmap, createBitmapFunction, fImage->width,
                                                        fImage->height, bitmapConfig);

        int ret;
        unsigned char *newBitmapPixels;


        if ((ret = AndroidBitmap_lockPixels(env, newBitmap, (void **) &newBitmapPixels)) < 0) {
            FFLOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        }

        unsigned char *imgData = reinterpret_cast<unsigned char *>(fImage->buffer);


        for (int i = 0; i < fImage->width * fImage->height; i++) {
            newBitmapPixels[i * 4] = imgData[i * 3];
            newBitmapPixels[i * 4 + 1] = imgData[i * 3 + 1];
            newBitmapPixels[i * 4 + 2] = imgData[i * 3 + 2];
            newBitmapPixels[i * 4 + 3] = 0xff;
        }
        AndroidBitmap_unlockPixels(env, newBitmap);


        (env)->CallBooleanMethod(obj_arraylist, arraylist_add_method, newBitmap);
    }

    return obj_arraylist;
}

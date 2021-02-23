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
#include "libavutil/imgutils.h"
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


extern "C"
JNIEXPORT void JNICALL
Java_gwj_dev_ffmpeg_videoEdit_VideoAPI_seekPreviewPostion(JNIEnv *env, jobject thiz, jint pos,
                                                          jobject surface) {

    if (!decoder) {
        FFLOGE("Decoder 未初始化");
        return;
    }
    FrameImage *frameImage = decoder->decodeOneFrame(pos, 1);
    if (frameImage == NULL) {
        FFLOGE("没有找到帧");
        return;
    }


    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    ANativeWindow_setBuffersGeometry(nativeWindow, frameImage->width,
                                     frameImage->height, WINDOW_FORMAT_RGBA_8888);

    ANativeWindow_Buffer outBuffer;
    if (ANativeWindow_lock(nativeWindow, &outBuffer, 0) == 0) {

        uint8_t *dst_data = static_cast<uint8_t *>(outBuffer.bits);


        //单个像素拷贝。
        for (int i = 0; i < frameImage->width * frameImage->height; i++) {
            dst_data[i * 4] = frameImage->buffer[i * 3];
            dst_data[i * 4 + 1] = frameImage->buffer[i * 3 + 1];
            dst_data[i * 4 + 2] = frameImage->buffer[i * 3 + 2];
            dst_data[i * 4 + 3] = 0xff;
        }

        //unlock
        ANativeWindow_unlockAndPost(nativeWindow);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_gwj_dev_ffmpeg_videoEdit_VideoAPI_play(JNIEnv *env, jobject thiz, jfloat time) {

    if (!decoder) {
        return;
    }
//    decoder->

}


extern "C"
JNIEXPORT jobject JNICALL
Java_gwj_dev_ffmpeg_videoEdit_VideoAPI_createThumbs(JNIEnv *env, jobject thiz, jstring v_path, jstring joutpath) {
    // TODO: implement createThumbs()
    if (!decoder) {
        return nullptr;
    }
    vector<string> results = decoder->initVideoInfos();



    jclass list_jcls = env->FindClass("java/util/ArrayList");
    if (list_jcls == NULL) {
        FFLOGE("ArrayList没找到相关类!")
        return 0;
    }

    //获取ArrayList构造函数id
    jmethodID list_init = env->GetMethodID(list_jcls, "<init>", "()V");
    //创建一个ArrayList对象
//    jobject list_obj = env->NewObject(list_jcls, list_init, "");

    //获取ArrayList对象的add()的methodID
//    jmethodID list_add = env->GetMethodID(list_jcls, "add", "(Ljava/lang/Object;)Z");

//
//    for(int i=0 ; i< results.size(); i++){
//
//
//    }
//
//
//    jbyteArray jArrayData = env->NewByteArray(48);
//    env->SetByteArrayRegion(jArrayData, 0, 48, (jbyte *) ledData);
//    jclass claxx = (*envGloble)->GetObjectClass(envGloble, thizObj);
//    jmethodID method = (*envGloble)->GetMethodID(envGloble, claxx, "onCM39getLedData",
//                                                 "(II[B)V");
//    (*envGloble)->CallVoidMethod(envGloble, thizObj, method, bigstart, biglen, jArrayData);
//    (*envGloble)->ReleaseByteArrayElements(envGloble, jArrayData,
//                                           (*envGloble)->GetByteArrayElements(envGloble,
//                                                                              jArrayData,
//                                                                              JNI_FALSE), 0);

}
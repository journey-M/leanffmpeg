#include <string>
#include <jni.h>
#include "native_log.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <pthread.h>
#include <unistd.h>
#include <android/bitmap.h>
#include "../src/Decoder.h"
#include "../src/Player.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
}

Decoder *decoder;
InputFile *inputFile;
Player * player;


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
    inputFile = new InputFile(path);

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
JNIEXPORT jobject JNICALL
Java_gwj_dev_ffmpeg_videoEdit_VideoAPI_createThumbs(JNIEnv *env, jobject thiz, jstring v_path, jstring joutpath) {
    // TODO: implement createThumbs()
    if (!decoder) {
        return nullptr;
    }
    vector<string> results = decoder->createVideoThumbs();



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

ANativeWindow *nativeWindow = NULL;

static void frame_call_back(AVFrame *avframe){
    int dest_width = avframe->width / 4;
    int dest_height = avframe->height / 4;
    char *tmpData;
#ifdef android

    //unix use RGB565  格式
    int dest_len = dest_width * dest_height * 3 / 2;
    // int dst_u_size = dest_width /2* dest_height /2;

    uint8_t *destData = new uint8_t[dest_len];
    uint8_t *destU = destData + dest_width * dest_height;
    uint8_t *destV = destData + dest_width * dest_height * 5 / 4;

    // scale first
    libyuv::I420Scale(
            avframe->data[0], avframe->width, avframe->data[2],
            avframe->width / 2, avframe->data[1], avframe->width / 2,
            avframe->width, avframe->height, destData, dest_width, destU,
            dest_width / 2, destV, dest_width / 2, dest_width, dest_height,
            libyuv::kFilterNone);

    int destSize = dest_width * dest_height * 3;
    tmpData = new char[destSize];

    libyuv::I420ToRGB24(destData, dest_width, destU, dest_width / 2,
                        destV, dest_width / 2, (uint8_t *) tmpData,
                        dest_width * 3, dest_width, dest_height);

#endif

    FrameImage * frameImage = (FrameImage *) malloc(sizeof(struct FrameImage));

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

    player = new Player();
    player->addInputFile(inputFile);
    player->preper(frame_call_back);
    player->play();

}

extern "C"
JNIEXPORT void JNICALL
Java_gwj_dev_ffmpeg_videoEdit_VideoAPI_realTimePreview(JNIEnv *env, jobject thiz,jobject surface) {
    // TODO: implement realTimePreview()

    nativeWindow = ANativeWindow_fromSurface(env, surface);

    player = new Player();
    player->addInputFile(inputFile);
    player->preper(frame_call_back);
    player->play();

}
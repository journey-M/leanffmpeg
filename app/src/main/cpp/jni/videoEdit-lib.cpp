#include <string>
#include <jni.h>
#include "native_log.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <pthread.h>
#include <unistd.h>
#include <android/bitmap.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
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
Player *player;

JavaVM *jvm = NULL;
jobject thizObj;
int th_attached = 0;
JNIEnv *envGloble;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    jint result = -1;
    jvm = vm;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    result = JNI_VERSION_1_4;
    return result;
}


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
Java_gwj_dev_ffmpeg_videoEdit_VideoAPI_createThumbs(JNIEnv *env, jobject thiz, jstring v_path,
                                                    jstring joutpath) {
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

static void frame_call_back(AVFrame *avframe) {
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

    FrameImage *frameImage = (FrameImage *) malloc(sizeof(struct FrameImage));
    frameImage->width = dest_width;
    frameImage->height = dest_height;
    frameImage->buffer = tmpData;

    ANativeWindow_setBuffersGeometry(nativeWindow, frameImage->width,
                                     frameImage->height, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer outBuffer;
    int ret = ANativeWindow_lock(nativeWindow, &outBuffer, 0);
    if (ret == 0) {

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

/**
 * 音频数据的回调
 * @param data
 * @param size
 */
int slEnginCreated = 0 ;
SLObjectItf engineObject = NULL;//用SLObjectItf声明引擎接口对象
SLEngineItf engineEngine = NULL;//声明具体的引擎对象实例

/**
 * 混音乐器
 */
SLObjectItf outputMixObject = NULL;
//SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;////创建具体的混音器对象实例
static SLAndroidSimpleBufferQueueItf playerBufferQueueItf = NULL;

//播放器
static SLObjectItf playerObject = NULL;
static SLPlayItf playerPlay = NULL;

#define NUM_RECORDER_EXPLICIT_INTERFACES 2
#define NUM_BUFFER_QUEUE 1
#define SAMPLE_RATE 44100
#define PERIOD_TIME 20  // 20ms
#define FRAME_SIZE SAMPLE_RATE * PERIOD_TIME / 1000
#define CHANNELS 1
#define BUFFER_SIZE   (FRAME_SIZE * CHANNELS)



static void createEngine()
{
    SLresult result;//返回结果
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);//第一步创建引擎
    assert(SL_RESULT_SUCCESS == result);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);//实现（Realize）engineObject接口对象
    assert(SL_RESULT_SUCCESS == result);
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);//通过engineObject的GetInterface方法初始化engineEngine
    assert(SL_RESULT_SUCCESS == result);

}

static void createMixer(){
    // 创建混音器
    SLresult result;
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, 0, 0);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
}

// 创建音频播放器
static void createAudioPlayer(SLEngineItf engineEngine, SLObjectItf outputMixObject, SLObjectItf &audioPlayerObject){
    SLDataLocator_AndroidSimpleBufferQueue dataSourceLocator = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
            1
    };

    // PCM 数据源格式
    SLDataFormat_PCM dataSourceFormat = {
            SL_DATAFORMAT_PCM,
            CHANNELS,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            16,
            SL_SPEAKER_FRONT_CENTER,
            SL_BYTEORDER_LITTLEENDIAN
    };

    SLDataSource dataSource = {
            &dataSourceLocator,
            &dataSourceFormat
    };

    SLDataLocator_OutputMix dataSinkLocator = {
            SL_DATALOCATOR_OUTPUTMIX, // 定位器类型
            outputMixObject // 输出混合
    };

    SLDataSink dataSink = {
            &dataSinkLocator, // 定位器
            0,
    };

    // 需要的接口
    SLInterfaceID interfaceIDs[] = {
            SL_IID_BUFFERQUEUE
    };
    SLboolean requiredInterfaces[] = {
            SL_BOOLEAN_TRUE
    };

    // 创建音频播放对象
    SLresult result = (*engineEngine)->CreateAudioPlayer(
            engineEngine,
            &audioPlayerObject,
            &dataSource,
            &dataSink,
            1,
            interfaceIDs,
            requiredInterfaces
    );
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
}

//int64_t getPcmData(void **pcm, FILE *pcmFile, uint8_t *out_buffer) {
//    if(pcmFile == NULL){
//        pcmFile = fopen("/sdcard/acc.pcm","r");
//    }
//    while(!feof(pcmFile)) {
//        size_t size = fread(out_buffer, 1, 44100 * 2 , pcmFile);
//        *pcm = out_buffer;
//        return size;
//    }
//    return 0;
//}

uint8_t *buffer = new uint8_t[BUFFER_SIZE];
uint8_t *out_buffer = new uint8_t [44100*2];
FILE *pcmFile = NULL;

// 播放音频时的回调
static void AudioPlayerCallback(SLAndroidSimpleBufferQueueItf bufferQueueItf, void *context){
    //TODO 获取得到的数据
//    if (!feof(playerContext->pfile)) {
//        fread(playerContext->buffer, playerContext->bufferSize, 1, playerContext->pfile);
//        FFLOGE("read a frame audio data.");
//        (*bufferQueueItf)->Enqueue(bufferQueueItf, playerContext->buffer, playerContext->bufferSize);
//    } else {
//        fclose(playerContext->pfile);
//        delete playerContext->buffer;
//    }

    FFLOGE("AudioPlayerCallback", "callbackk  ------- AudioPlayerCallback  \n");
    if(player != NULL){
        int size = 0;
        uint8_t *buffer = new uint8_t[BUFFER_SIZE];
        player->getBufferData(&size, buffer);
        if(size > 0){
            (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, size);
        }
    }


//    int32_t size = getPcmData(reinterpret_cast<void **>(&buffer), pcmFile, out_buffer);
//    FFlog("pcmBufferCallBack, size=%d", size);
//    if (NULL != buffer && size > 0) {
//        SLresult result = (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, size);
//    }

}


static void audio_callback(unsigned char *data, int size) {

    FFlog("receive  --- data --- size = %d \n", size);
//    //attch thread
//    if (!th_attached) {
//        jvm->AttachCurrentThread(&envGloble, 0);
//        th_attached = 1;
//    }
//
//    jbyteArray jArrayData = envGloble->NewByteArray(size);
//    envGloble->SetByteArrayRegion(jArrayData, 0, size, (jbyte *) data);
//    jclass claxx = envGloble->GetObjectClass(thizObj);
//    jmethodID method = envGloble->GetMethodID(claxx, "audiuDataCallback",
//                                              "(I[B)V");
//    envGloble->CallVoidMethod(thizObj, method, size, jArrayData);
//    envGloble->ReleaseByteArrayElements(jArrayData,
//                                        envGloble->GetByteArrayElements(jArrayData,
//                                                                        JNI_FALSE), 0);




}


extern "C"
JNIEXPORT void JNICALL
Java_gwj_dev_ffmpeg_videoEdit_VideoAPI_play(JNIEnv *env, jobject thiz, jfloat time) {
    if (thizObj == NULL) {
        thizObj = env->NewGlobalRef(thiz);
        if (thizObj != NULL) {
            env->DeleteLocalRef(thiz);
        }
    }
    player = new Player();
    player->addInputFile(inputFile);
    player->preper(frame_call_back, audio_callback);
    player->play();

}

extern "C"
JNIEXPORT void JNICALL
Java_gwj_dev_ffmpeg_videoEdit_VideoAPI_realTimePreview(JNIEnv *env, jobject thiz, jobject surface) {
    // TODO: implement realTimePreview()
    if (thizObj == NULL) {
        thizObj = env->NewGlobalRef(thiz);
        if (thizObj != NULL) {
            env->DeleteLocalRef(thiz);
        }
    }
    nativeWindow = ANativeWindow_fromSurface(env, surface);
    if(!slEnginCreated){
        slEnginCreated = 1;
        createEngine();
        createMixer();

        //创建播放器
        SLresult result;
        createAudioPlayer(engineEngine, outputMixObject, playerObject);
        result = (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        result = (*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE,
                                               &playerBufferQueueItf);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        result = (*playerBufferQueueItf)->RegisterCallback(playerBufferQueueItf, AudioPlayerCallback,
                                                           NULL);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        result = (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerPlay);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);
        assert(SL_RESULT_SUCCESS == result);

        AudioPlayerCallback(playerBufferQueueItf, NULL);

    }
    player = new Player();
    player->addInputFile(inputFile);
    player->preper(frame_call_back, audio_callback);
    player->play();

//    while (1) {
//        if (player != NULL) {
//            int size = 0;
//            uint8_t *buffer = new uint8_t[BUFFER_SIZE];
//            player->getBufferData(&size, buffer);
//            if (size > 0) {
//                (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);
//                (*playerBufferQueueItf)->Enqueue(playerBufferQueueItf, buffer, size);
//
//            }
//        }
//    }

}
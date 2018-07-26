#include <jni.h>
#include <string>
#include "native_log.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <pthread.h>
#include <unistd.h>
#include "libyuv.h"

#define MAX_AUDIO_FRME_SIZE 48000 * 4

extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

extern "C"
JNIEXPORT jstring
JNICALL
Java_gwj_dev_ffmpeg_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT void
JNICALL
Java_gwj_dev_ffmpeg_MainActivity_decdoe(JNIEnv *env, jobject instance, jstring input_,
                                        jstring output_) {
    const char *input = env->GetStringUTFChars(input_, 0);
    const char *output = env->GetStringUTFChars(output_, 0);

    // TODO
    FFLOGE("输入路径 = %s",input);
    FFLOGE("输出路径 = %s",output);

    av_register_all();

    AVFormatContext *pFormateContext = avformat_alloc_context();

    if(avformat_open_input(&pFormateContext,input, NULL, NULL )){
        FFLOGE("%s", "无法打开视频文件");
        return;
    }

    if(avformat_find_stream_info(pFormateContext, NULL) < 0){
        FFLOGE("%s", "无法获取到视频信息");
        return;
    }

    //获取视频流的索引位置
    //遍历所有类型的流（音频流、视频流、字幕流），找到视频流
    int v_stream_idx = -1;
    int i = 0;
    for(; i < pFormateContext->nb_streams; i++){
        if(pFormateContext->streams[i] ->codec ->codec_type == AVMEDIA_TYPE_VIDEO){
            v_stream_idx = i;
            break;
        }
    }

    //只有知道视频的编码方式，才能够根据编码方式去找到解码器
    //获取视频流中的编解码上下文
    AVCodecContext *pCodecContxt = pFormateContext->streams[v_stream_idx]->codec;
    //4.根据编解码上下文中的编码id查找对应的解码
    AVCodec *pCodec = avcodec_find_decoder(pCodecContxt->codec_id);
    if(pCodec == NULL){
        FFLOGE("%", "找不大对应的解码器");
        return;
    }

    //打开解决码器
    if(avcodec_open2(pCodecContxt, pCodec, NULL)<0){
        FFLOGE("%s","解码器无法打开\n");
        return;
    }
    //输出视频信息
    FFLOGE("视频的文件格式：%s",pFormateContext->iformat->name);
    FFLOGE("视频时长：%d", (pFormateContext->duration)/1000000);
    FFLOGE("视频的宽高：%d,%d",pCodecContxt->width,pCodecContxt->height);
    FFLOGE("解码器的名称：%s",pCodec->name);

    //准备读取
    //AVPacket用于存储一帧一帧的压缩数据（H264）
    //缓冲区，开辟空间
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));;

    //AVFrame用于存储解码后的像素数据(YUV)
    //内存分配
    AVFrame *pFram = av_frame_alloc();
    //YUV420
    AVFrame *pFrameYUV = av_frame_alloc();
    //只有指定了AVFrame的像素格式、画面大小才能真正分配内存
    //缓冲区分配内存
    uint8_t  *out_buffer = (uint8_t *) av_malloc(
            avpicture_get_size(AV_PIX_FMT_YUV420P,pCodecContxt->width, pCodecContxt->height));

    avpicture_fill((AVPicture *) pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecContxt->width, pCodecContxt->height );

    //用于转码（缩放）的参数，转之前的宽高，转之后的宽高，格式等
    struct SwsContext *sws_ctx = sws_getContext(pCodecContxt->width,pCodecContxt->height,pCodecContxt->pix_fmt,
                                                pCodecContxt->width, pCodecContxt->height, AV_PIX_FMT_YUV420P,
                                                SWS_BICUBIC, NULL, NULL, NULL);
    int frame_count = 0;
    int got_picture, ret;
    FILE *fp_yuv = fopen(output, "wb+");
    //6.一帧一帧的读取压缩数据
    while (av_read_frame(pFormateContext, packet) >= 0)
    {
        //是视频的 数据
        if(packet->stream_index == v_stream_idx){
            ret = avcodec_decode_video2(pCodecContxt,pFram,  &got_picture, packet);
            if(ret < 0){
                FFLOGE("%s","解码错误");
                return;
            }

            //为0说明解码完成，非0正在解码
            if (got_picture)
            {
                //AVFrame转为像素格式YUV420，宽高
                //2 6输入、输出数据
                //3 7输入、输出画面一行的数据的大小 AVFrame 转换是一行一行转换的
                //4 输入数据第一列要转码的位置 从0开始
                //5 输入画面的高度
                sws_scale(sws_ctx, (const uint8_t *const *) pFram->data, pFram->linesize, 0, pCodecContxt->height,
                          pFrameYUV->data, pFrameYUV->linesize);

                //输出到YUV文件
                //AVFrame像素帧写入文件
                //data解码后的图像像素数据（音频采样数据）
                //Y 亮度 UV 色度（压缩了） 人对亮度更加敏感
                //U V 个数是Y的1/4
                int y_size = pCodecContxt->width * pCodecContxt->height;
                fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);
                fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);
                fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);

                frame_count++;
                FFLOGE("解码第%d帧",frame_count);
            }
        }
    }
    FFLOGE("解码完成");
    fclose(fp_yuv);
    av_frame_free(&pFram);
    avcodec_close(pCodecContxt);
    avformat_free_context(pFormateContext);
    env->ReleaseStringUTFChars(input_, input);
    env->ReleaseStringUTFChars(output_, output);
}

extern "C"
JNIEXPORT void
JNICALL
Java_gwj_dev_ffmpeg_MainActivity_playVideo(JNIEnv *env, jobject instance, jstring input_,
            jobject surface) {

    const char *input = env->GetStringUTFChars(input_, 0);
    FFLOGE("输入路径 = %s",input);
    av_register_all();
    AVFormatContext *pFormateContext = avformat_alloc_context();
    if(avformat_open_input(&pFormateContext,input, NULL, NULL )){
        FFLOGE("%s", "无法打开视频文件");
        return;
    }
    if(avformat_find_stream_info(pFormateContext, NULL) < 0){
        FFLOGE("%s", "无法获取到视频信息");
        return;
    }
    //获取视频流的索引位置
    //遍历所有类型的流（音频流、视频流、字幕流），找到视频流
    int v_stream_idx = -1;
    int i = 0;
    for(; i < pFormateContext->nb_streams; i++){
        if(pFormateContext->streams[i] ->codec ->codec_type == AVMEDIA_TYPE_VIDEO){
            v_stream_idx = i;
            break;
        }
    }

    //只有知道视频的编码方式，才能够根据编码方式去找到解码器
    //获取视频流中的编解码上下文
    AVCodecContext *pCodecContxt = pFormateContext->streams[v_stream_idx]->codec;
    //4.根据编解码上下文中的编码id查找对应的解码
    AVCodec *pCodec = avcodec_find_decoder(pCodecContxt->codec_id);
    if(pCodec == NULL){
        FFLOGE("%", "找不大对应的解码器");
        return;
    }

    //打开解决码器
    if(avcodec_open2(pCodecContxt, pCodec, NULL)<0){
        FFLOGE("%s","解码器无法打开\n");
        return;
    }
    //输出视频信息
    FFLOGE("视频的文件格式：%s",pFormateContext->iformat->name);
    FFLOGE("视频时长：%d", (pFormateContext->duration)/1000000);
    FFLOGE("视频的宽高：%d,%d",pCodecContxt->width,pCodecContxt->height);
    FFLOGE("解码器的名称：%s",pCodec->name);

    //准备读取
    //AVPacket用于存储一帧一帧的压缩数据（H264）
    //缓冲区，开辟空间
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));;

    //AVFrame用于存储解码后的像素数据(YUV)
    //内存分配
    AVFrame *yuv_frame = av_frame_alloc();
    //YUV420
    AVFrame *rgb_frame = av_frame_alloc();
    //只有指定了AVFrame的像素格式、画面大小才能真正分配内存
    //缓冲区分配内存

    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    ANativeWindow_Buffer outBuffer ;


    int got_picture, len, framCount;
    //6.一帧一帧的读取压缩数据
    while (av_read_frame(pFormateContext, packet) >= 0)
    {
        //是视频的 数据
        if(packet->stream_index == v_stream_idx){
            len = avcodec_decode_video2(pCodecContxt,yuv_frame,  &got_picture, packet);
            if(len < 0){
                FFLOGE("%s","解码错误");
                return;
            }
            //为0说明解码完成，非0正在解码
            if (got_picture)
            {
                ANativeWindow_setBuffersGeometry(nativeWindow, pCodecContxt->width, pCodecContxt->height,WINDOW_FORMAT_RGBA_8888);
                ANativeWindow_lock(nativeWindow, &outBuffer, NULL);
                //设置rgb_frame的属性（像素格式、宽高）和缓冲区
                //rgb_frame缓冲区与outBuffer.bits是同一块内存
                avpicture_fill((AVPicture *) rgb_frame, (const uint8_t *) outBuffer.bits, AV_PIX_FMT_RGBA, pCodecContxt->width, pCodecContxt->height);

                //YUV->RGBA_8888
                libyuv::I420ToARGB(yuv_frame->data[0],yuv_frame->linesize[0],
                           yuv_frame->data[2],yuv_frame->linesize[2],
                           yuv_frame->data[1],yuv_frame->linesize[1],
                           rgb_frame->data[0], rgb_frame->linesize[0],
                           pCodecContxt->width,pCodecContxt->height);

                ANativeWindow_unlockAndPost(nativeWindow);
                FFLOGE("showing  --  one fram  %d", framCount++);
//                sleep(1);
            }
        }
        av_free_packet(packet);
    }

    ANativeWindow_release(nativeWindow);
    av_frame_free(&yuv_frame);
    av_frame_free(&rgb_frame);
    avcodec_close(pCodecContxt);
    avformat_free_context(pFormateContext);
    env->ReleaseStringUTFChars(input_, input);

}

//播放音频文件
extern "C"
JNIEXPORT void JNICALL
Java_gwj_dev_ffmpeg_MainActivity_playAudio(JNIEnv *env, jobject instance, jstring input_, jstring output_) {
    const char *input = env->GetStringUTFChars(input_, 0);
    const char *output = env->GetStringUTFChars(output_, 0);


    // TODO
    FFLOGE("audio file path  = %s", input);
    av_register_all();
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    //打开音频文件
    if(avformat_open_input(&pFormatCtx,input,NULL,NULL) != 0){
        FFLOGE("%s","无法打开音频文件");
        return;
    }
    //获取音频流索引位置
    int i = 0, audio_stream_idx = -1;
    for(; i < pFormatCtx->nb_streams;i++){
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
            audio_stream_idx = i;
            break;
        }
    }

    //获取解码器
    AVCodecContext *codecCtx = pFormatCtx->streams[audio_stream_idx]->codec;
    AVCodec *codec = avcodec_find_decoder(codecCtx->codec_id);
    if(codec == NULL){
        FFLOGE("%s","无法获取解码器");
        return;
    }
    //打开解码器
    if(avcodec_open2(codecCtx,codec,NULL) < 0){
        FFLOGE("%s","无法打开解码器");
        return;
    }
//压缩数据
    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    //解压缩数据
    AVFrame *frame = av_frame_alloc();
    //frame->16bit 44100 PCM 统一音频采样格式与采样率
    SwrContext *swrCtx = swr_alloc();

    //重采样设置参数-------------start
    //输入的采样格式
    enum AVSampleFormat in_sample_fmt = codecCtx->sample_fmt;
    //输出采样格式16bit PCM
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    //输入采样率
    int in_sample_rate = codecCtx->sample_rate;
    //输出采样率
    int out_sample_rate = 44100;
    //获取输入的声道布局
    //根据声道个数获取默认的声道布局（2个声道，默认立体声stereo）
    //av_get_default_channel_layout(codecCtx->channels);
    uint64_t in_ch_layout = codecCtx->channel_layout;
    //输出的声道布局（立体声）
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

    swr_alloc_set_opts(swrCtx,
                       out_ch_layout,out_sample_fmt,out_sample_rate,
                       in_ch_layout,in_sample_fmt,in_sample_rate,
                       0, NULL);
    swr_init(swrCtx);

    //输出的声道个数
    int out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);

    //重采样设置参数-------------end
    //16bit 44100 PCM 数据
    uint8_t *out_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRME_SIZE);

    FILE *fp_pcm = fopen(output,"wb");

    int got_frame = 0,index = 0, ret;
    //不断读取压缩数据
    while(av_read_frame(pFormatCtx,packet) >= 0){
        //解码
        ret = avcodec_decode_audio4(codecCtx,frame,&got_frame,packet);

        if(ret < 0){
            FFLOGE("%s","解码完成");
        }
        //解码一帧成功
        if(got_frame > 0){
            FFLOGE("解码：%d",index++);
            swr_convert(swrCtx, &out_buffer, MAX_AUDIO_FRME_SIZE, (const uint8_t **) frame->data, frame->nb_samples);
            //获取sample的size
            int out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb,
                                                             frame->nb_samples, out_sample_fmt, 1);
            fwrite(out_buffer,1,out_buffer_size,fp_pcm);
        }

        av_free_packet(packet);
    }

    fclose(fp_pcm);
    av_frame_free(&frame);
    av_free(out_buffer);

    swr_free(&swrCtx);
    avcodec_close(codecCtx);
    avformat_close_input(&pFormatCtx);


    env->ReleaseStringUTFChars(input_, input);
}


JavaVM *javaVM;

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved){
    javaVM = vm;
    return JNI_VERSION_1_4;

}



/**
 * 在另外的线程中执行
 * 不同的线程  JNIEVE 对象不一致
 * @param args
 * @return
 */
void* play_func(void * args){

    FFLOGE("在单独的线程中执行");
//    JNIEnv* env = NULL;
//    javaVM->AttachCurrentThread(&env,NULL);
    int i;
    for(i=0; i< 4; i++){
        FFLOGE("在单独的线程中执行 %d",i);
        sleep(1);
    }

    for(;;){
        FFLOGE("这是个 死循环的 c线程 %d",i);
        i++;
        sleep(1);
    }

//    pthread_exit((void*)0);
}


void test_forkn(){
    int nPid = fork();

    FFLOGE("new fork thread =%d", nPid);
    if (nPid < 0) {
        FFLOGE("nPid 小于0");
    } else if (nPid == 0) {
        FFLOGE("nPid 等于0   子进程id= %d",nPid);

        int i=0;
        for(;;){
            FFLOGE("新的进程中执行 循环 %d", i);
            i++;
            sleep(1);
        }
    } else {
        FFLOGE("nPid 不等于0  还是在父进程中执行");
    }


}

void * myCplayThread(void * args){

    int num = 0;

    while(true){

        num++;
        FFLOGE("在子线程中执行");
        if(num > 3){
            break;
        }
        sleep(2);

    }

    return (void *) 1;
}

extern "C"
JNIEXPORT void JNICALL
Java_gwj_dev_ffmpeg_MainActivity_playVideoInPosixThread(JNIEnv *env, jobject instance,
                                                        jstring input_, jobject surface) {
    const char *input = env->GetStringUTFChars(input_, 0);

//    pthread_t  pid;
//    pthread_create(&pid,NULL ,play_func,(void*)"NO.1");
//    FFLOGE("sub thread pid = %d", pid);


//    void* rval;
//    pthread_join(pid, &rval);
//    pthread_exit(rval);

//    sleep(1);
//    pthread_exit(&pid);

    //创建一个新的进程执行计数操作
//    test_forkn();

    //创建新的线程去播放
    int ret = 0;
    pthread_t  pid ;
    ret = pthread_create(&pid,NULL, myCplayThread,NULL);

    if(ret){
        FFLOGE("线程创建失败！");
        return ;
    }

    pthread_join(pid, NULL);

    FFLOGE("释放之前-----！");
    env->ReleaseStringUTFChars(input_, input);
}



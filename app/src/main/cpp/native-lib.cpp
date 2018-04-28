#include <jni.h>
#include <string>
#include "native_log.h"

extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
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
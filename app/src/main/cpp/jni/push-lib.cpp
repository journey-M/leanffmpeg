#include <jni.h>
#include <string>
#include "native_log.h"

extern "C" {
#include "x264/x264.h"
#include "x264/x264_config.h"
}

//x264变慢处理器
x264_t *x264_encoder;
x264_picture_t pic_in;
x264_picture_t pic_out;
int y_len, u_len, v_len;

extern "C"
JNIEXPORT void
JNICALL
Java_gwj_dev_ffmpeg_PushUtil_initStreamParams(
        JNIEnv *env,
        jobject /* this */, jint width, jint height, jint bitrate, jint fps) {

    //设置编码器的参数
    x264_param_t param;
    x264_param_default_preset(&param, "ultrafast", "zerolatency");
    //编码输入的像素格式  YUV420p
    param.i_csp = X264_CSP_I420;
    param.i_width = width;
    param.i_height = height;
    y_len = width * height;
    u_len = y_len / 4;
    v_len = u_len;

    //参数 i_rc_method标识码率
    param.rc.i_rc_method = X264_RC_CRF;
    param.rc.i_bitrate = bitrate / 1000;
    param.rc.i_vbv_max_bitrate = bitrate / 1000 * 1.2;

    //码率控制不通过timebase和timestamp，而是fps
    param.b_vfr_input = 0;
    param.i_fps_num = fps;
    param.i_fps_den = 1;
    param.i_timebase_den = param.i_fps_num;
    param.i_timebase_num = param.i_fps_den;
    param.i_threads = 1; //并行编码线程数量,0默认为多线程

    //是否把 SPS和PPS放入每一个关键帧
    param.b_repeat_headers = 1;
    param.i_level_idc = 51;
    //baseline级别，  没有b帧
    x264_param_apply_profile(&param, "baseline");

    //x264_picture_t（输入图像）初始化
    x264_picture_alloc(&pic_in, param.i_csp, param.i_width, param.i_height);

    x264_encoder = x264_encoder_open(&param);
    if (x264_encoder) {
        FFLOGE("打开编码器成功...");
    }
    FFLOGE("x264编码测试  的log...");
    return;
}


extern "C"
JNIEXPORT void
JNICALL
Java_gwj_dev_ffmpeg_PushUtil_encodeStream(
        JNIEnv *env,
        jobject /* this */,
        jbyteArray data) {

    jbyte *nv21_buffer = env->GetByteArrayElements(data, NULL);
    uint8_t *u = pic_in.img.plane[1];
    uint8_t *v = pic_in.img.plane[2];
    //nv21 4:2:0 Formats, 12 Bits per Pixel
    //nv21与yuv420p，y个数一致，uv位置对调
    //nv21转yuv420p  y = w*h,u/v=w*h/4
    //nv21 = yvu yuv420p=yuv y=y u=y+1+1 v=y+1
    memcpy(pic_in.img.plane[0], nv21_buffer, y_len);
    int i;
    for (i = 0; i < u_len; i++) {
        *(u + i) = *(nv21_buffer + y_len + i * 2 + 1);
        *(v + i) = *(nv21_buffer + y_len + i * 2);
    }

    //h264编码得到NALU数组
    x264_nal_t *nal = NULL; //NAL
    int n_nal = -1; //NALU的个数

    //进行h264编码
    if (x264_encoder_encode(x264_encoder, &nal, &n_nal, &pic_in, &pic_out) < 0) {
        FFLOGE("%s", "编码失败");
        return;
    }


}



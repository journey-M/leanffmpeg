#include "Decoder.h"
#include "libyuv/convert_argb.h"
#include <cstdio>
#include <cstdlib>
#include <libavutil/error.h>
#include <string>
#include "../jni/native_log.h"


Decoder::Decoder(InputFile *input) {
    mInputFile = input;

    findAudioStream();
    findVideoStream();

    initVideoDecoder();
}

int Decoder::getVideoImages(Options *opt, vector<FrameImage *> *result, int max) {
    //如果有视频流

    if (videoStream) {
        int start = opt->start;
        for (int i = 0; i < max; i++) {
            FrameImage *tmp = decodeOneFrame(start, 10);
            if (tmp) {
                result->push_back(tmp);
            }
            start = start + opt->per;
        }
    }
    return result->size();
}

FrameImage *Decoder::decodeOneFrame(int start, int cut) {

    FrameImage *tmp = NULL;
    AVPacket avPacket;

    bool gotFrame = false;
    int ret = -1;
    AVRational time_base = videoStream->time_base;
    int64_t seekPos = start / av_q2d(time_base);

    ret = avformat_seek_file(mInputFile->fmt_ctx, videoStreamIndex, INT64_MIN, seekPos, seekPos,
                             AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
    if (ret < 0) {
        ret = avformat_seek_file(mInputFile->fmt_ctx, videoStreamIndex, INT64_MIN, seekPos, seekPos,
                                 0);
    }


    avcodec_flush_buffers(vCodecCtx);

    if (ret < 0) {
        fprintf(stderr, "av_seek_frame  faile \n");
        return tmp;
    }

    while (!gotFrame) {
        ret = av_read_frame(mInputFile->fmt_ctx, &avPacket);
        if (AVERROR(ret) == AVERROR_EOF) {
            fprintf(stderr, "av read frame faile %d \n", AVERROR(ret));
            break;
        } else if (ret < 0) {
            fprintf(stderr, "av read frame faile %d \n", AVERROR(ret));
            break;
        }

        if (avPacket.stream_index != videoStreamIndex) {
            continue;
        }

        ret = avcodec_send_packet(vCodecCtx, &avPacket);
        if (ret < 0) {
            fprintf(stderr, "error  send package \n");
            continue;
        }

        AVFrame *avframe = av_frame_alloc();
        ret = avcodec_receive_frame(vCodecCtx, avframe);
        if (ret < 0) {
            fprintf(stderr, "recive error \n");
            continue;
        } else {
            fprintf(stderr, "success receive frame \n");
            fprintf(stderr, " linesize : : %d, %d %d %d \n",
                    avframe->linesize[0], avframe->linesize[1],
                    avframe->linesize[2], avframe->linesize[3]);

            int dest_width = avframe->width / cut;
            int dest_height = avframe->height / cut;
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


#ifdef unix
            //unix use RGBA 格式
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
            char name[20] = {'\0'};
            sprintf(name, "./t%ld.jpg", seekPos);
            rgb2jpg(name, (char *)tmpData, dest_width, dest_height);
#endif

            tmp = (FrameImage *) malloc(sizeof(struct FrameImage));

            tmp->buffer = tmpData;
            tmp->width = dest_width;
            tmp->height = dest_height;
            gotFrame = true;
            fprintf(stderr, "yuv convert succes \n");
        }
    }
    av_packet_unref(&avPacket);
    return tmp;
}


#ifdef unix
int Decoder::rgb2jpg(char *jpg_file, char *pdata, int width, int height)
{


    int depth = 3;
    JSAMPROW row_pointer[1];
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);


    FILE *outfile;
    if ((outfile = fopen(jpg_file, "wb")) == NULL)
    {
        return -1;
    }


    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width      = width;
    cinfo.image_height     = height;
    cinfo.input_components = 3;
    cinfo.in_color_space   = JCS_EXT_RGB;
    jpeg_set_defaults(&cinfo);

    jpeg_set_quality(&cinfo, 80, TRUE );
    jpeg_start_compress(&cinfo, TRUE);

    int row_stride = width * depth;
    while (cinfo.next_scanline < cinfo.image_height)
    {
            row_pointer[0] = (JSAMPROW)(pdata + cinfo.next_scanline * row_stride);
            jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);

    return 0;
 
}
#endif

int Decoder::initVideoDecoder() {
    videoCodec = avcodec_find_decoder(videoStream->codecpar->codec_id);
    if (!videoCodec) {
        fprintf(stderr, "videoCode is not find");
        return -1;
    }
    vCodecCtx = avcodec_alloc_context3(videoCodec);
    if (!vCodecCtx) {
        fprintf(stderr, "video code ctx is not alloc");
        return -1;
    }

    int ret = avcodec_parameters_to_context(vCodecCtx, videoStream->codecpar);
    if (ret < 0) {
        fprintf(stderr, "faile to copy codec params to decoder context \n");
        return -1;
    }

    //init decoders
    ret = avcodec_open2(vCodecCtx, videoCodec, NULL);
    if (ret < 0) {
        fprintf(stderr, "Failed to open codec /n");
        return -1;
    }

    return 0;
}

int Decoder::findVideoStream() {
    if (!videoStream) {
        int ret = av_find_best_stream(mInputFile->fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
        if (ret < 0) {
            fprintf(stderr, "no video stream \n");
            return ret;
        }
        videoStream = mInputFile->fmt_ctx->streams[ret];
        videoStreamIndex = ret;
    }
    return 0;
}

int Decoder::findAudioStream() {
    if (!audioStream) {
        int ret = av_find_best_stream(mInputFile->fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
        if (ret < 0) {
            fprintf(stderr, "no video stream \n");
            return ret;
        }
        audioStream = mInputFile->fmt_ctx->streams[ret];
        audioStreamIndex = ret;
    }
    return 0;
}

/**
 * 将AVFrame(YUV420格式)保存为JPEG格式的图片
 *
 * @param width YUV420的宽
 * @param height YUV42的高
 *
 */
int Decoder::writeJPEG(AVFrame *pFrame, const char *path, int width, int height) {
    // 分配AVFormatContext对象
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    // 设置输出文件格式
    pFormatCtx->oformat = av_guess_format("mjpeg", NULL, NULL);
    // 创建并初始化一个和该url相关的AVIOContext
    if (avio_open(&pFormatCtx->pb, path, AVIO_FLAG_READ_WRITE) < 0) {
        return -1;
    }

    // 构建一个新stream
    AVStream *pAVStream = avformat_new_stream(pFormatCtx, 0);
    if (pAVStream == NULL) {
        return -1;
    }

    // 设置该stream的信息
    AVCodecContext *pCodecCtx = pAVStream->codec;

    pCodecCtx->codec_id = pFormatCtx->oformat->video_codec;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
    pCodecCtx->width = width;
    pCodecCtx->height = height;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;

    // Begin Output some information
    av_dump_format(pFormatCtx, 0, path, 1);
    // End Output some information

    // 查找解码器
    AVCodec *pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if (!pCodec) {
        return -1;
    }
    // 设置pCodecCtx的解码器为pCodec
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        return -1;
    }

    //Write Header
    avformat_write_header(pFormatCtx, NULL);

    int y_size = pCodecCtx->width * pCodecCtx->height;

    //Encode
    // 给AVPacket分配足够大的空间
    AVPacket pkt;
    av_new_packet(&pkt, y_size * 3);

    //
    int got_picture = 0;
    int ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_picture);
    if (ret < 0) {
        return -1;
    }
    if (got_picture == 1) {
        //pkt.stream_index = pAVStream->index;
        ret = av_write_frame(pFormatCtx, &pkt);
    }

    av_free_packet(&pkt);

    //Write Trailer
    av_write_trailer(pFormatCtx);

    if (pAVStream) {
        avcodec_close(pAVStream->codec);
    }
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);

    v_path_results.push_back(path);
    return 0;
}


void Decoder::push_n_avframe(AVFrame *frame) {
    double clockTimed = timedIndex * _time_val;
    //首帧直接保存
    if (timedIndex == 0) {
        scalAndSaveFrame(frame, clockTimed);
        timedIndex++;
        return;
    }
    double frameTimed = frame->pts * av_q2d(videoStream->time_base);

    //如果当前的图片大于时间值，则保存上次的frame
    if (frameTimed > clockTimed) {
        scalAndSaveFrame(preFrame, clockTimed);
        av_frame_free(&preFrame);
        preFrame = frame;
        timedIndex++;
    } else {
        if (preFrame != NULL) {
            av_frame_free(&preFrame);
        }
        preFrame = frame;
    }
}


void Decoder::scalAndSaveFrame(AVFrame *avframe, double playTime) {
    AVFrame *destFrame = av_frame_alloc();

    int dest_width = avframe->width / 10;
    int dest_height = avframe->height / 10;

    FFLOGE("origin width : %d ,  %d ,  dest : %d,  %d", avframe->width, avframe->height,
           dest_width, dest_height)

    destFrame->format = AV_PIX_FMT_YUV420P;
    destFrame->width = dest_width;
    destFrame->height = dest_height;
    int ret = av_frame_get_buffer(destFrame, 32);
    av_frame_make_writable(destFrame);

    if (ret < 0) {
        fprintf(stderr, "avpicture_fill  error---- \n");
    }

    // scale first
    libyuv::I420Scale(
            avframe->data[0], avframe->width, avframe->data[2],
            avframe->width / 2, avframe->data[1], avframe->width / 2,
            avframe->width, avframe->height, destFrame->data[0], dest_width,
            destFrame->data[2],
            dest_width / 2, destFrame->data[1], dest_width / 2, dest_width, dest_height,
            libyuv::kFilterNone);


    char pathfile[1000];
    sprintf(pathfile, "/sdcard/zzjk/test_%f.jpg", playTime);

    writeJPEG(destFrame, pathfile, destFrame->width, destFrame->height);
}


vector<string> Decoder::initVideoInfos() {

    v_path_results.clear();

    AVPacket avPacket;
    int ret = -1;
    //seek 到0位置，解码获取所有的图片，并保存
    ret = avformat_seek_file(mInputFile->fmt_ctx, videoStreamIndex, INT64_MIN, 0, 0,
                             AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
    if (ret < 0) {
        fprintf(stderr, "seek file  error \n");
    }

    avcodec_flush_buffers(vCodecCtx);

    //1秒内抽取10帧图片，直到最后
    while (timedIndex < 31) {
        ret = av_read_frame(mInputFile->fmt_ctx, &avPacket);
        if (AVERROR(ret) == AVERROR_EOF) {
            fprintf(stderr, "av read frame faile %d \n", AVERROR(ret));
            break;
        } else if (ret < 0) {
            fprintf(stderr, "av read frame faile %d \n", AVERROR(ret));
            break;
        }

        if (avPacket.stream_index != videoStreamIndex) {
            continue;
        }

        ret = avcodec_send_packet(vCodecCtx, &avPacket);
        if (ret < 0) {
            fprintf(stderr, "error  send package \n");
            continue;
        }

        AVFrame *avframe = av_frame_alloc();
        ret = avcodec_receive_frame(vCodecCtx, avframe);
        if (ret < 0) {
            fprintf(stderr, "recive error \n");
            continue;
        } else {
            push_n_avframe(avframe);
        }
    }
    push_n_avframe(NULL);
    av_packet_unref(&avPacket);

    return v_path_results;
}


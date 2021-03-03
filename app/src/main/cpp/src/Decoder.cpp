#include "Decoder.h"
#include "libyuv/convert_argb.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include "Log.h"


Decoder::Decoder(InputFile *input) {

    av_init_packet(&flush_pkt);
    videoPacketList.setTag("video packet");
    audioPacketList.setTag("audio packet");

    mInputFile = input;
    findAudioStream();
    findVideoStream();
    initVideoDecoder();
    initAudioDecoder();
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

int Decoder::initAudioDecoder(){
    audioCodec = avcodec_find_decoder(audioStream->codecpar->codec_id);
    if(!audioCodec){
        fprintf(stderr, "audioCodec is not find");
        return -1;
    }
    aCodecCtx = avcodec_alloc_context3(audioCodec);
    if (!aCodecCtx) {
        fprintf(stderr, "audio code ctx is not alloc");
        return -1;
    }
    
    int ret = avcodec_parameters_to_context(aCodecCtx, audioStream->codecpar);
    if (ret < 0) {
        fprintf(stderr, "faile to copy codec params to decoder context \n");
        return -1;
    }

    //init decoders
    ret = avcodec_open2(aCodecCtx, audioCodec, NULL);
    if (ret < 0) {
        fprintf(stderr, "Failed to open audioCodec /n");
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

    FFlog("origin width : %d ,  %d ,  dest : %d,  %d", avframe->width, avframe->height,
          dest_width, dest_height);

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


vector<string> Decoder::createVideoThumbs() {
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

void Decoder::setPlayState(VideoState *state) {
    this->videoState = state;
}

int Decoder::queue_picture(AVFrame *src_frame, double pts, double duration, int64_t pos) {

    if (video_display_list.size() > MAX_V_DISPLAY_FRAMS) {
        pthread_mutex_lock(&mutex_video_frame_list);
        pthread_cond_wait(&cond_video_frame_list, &mutex_video_frame_list);
        pthread_mutex_unlock(&mutex_video_frame_list);
    }

    Frame *vp = static_cast<Frame *>(malloc(sizeof(struct Frame)));
    vp->width = src_frame->width;
    vp->height = src_frame->height;
    vp->format = src_frame->format;
    vp->pts = pts;
    vp->duration = duration;
    vp->pos = pos;
    vp->srcFrame = av_frame_alloc();
    av_frame_move_ref(vp->srcFrame, src_frame);

    pthread_mutex_lock(&mutex_video_frame_list);
    video_display_list.push_back(vp);
    FFlog("showing frame size = %d \n", video_display_list.size());
    pthread_mutex_unlock(&mutex_video_frame_list);
    return 0;
}

int Decoder::pop_picture(Frame **frame) {
    if (video_display_list.size() <= MIN_V_DISPLAY_FRAMS) {
        pthread_mutex_lock(&mutex_video_frame_list);
        pthread_cond_signal(&cond_video_frame_list);
        pthread_mutex_unlock(&mutex_video_frame_list);
    }
    if (video_display_list.size() == 0) {
        return 0;
    }
    pthread_mutex_lock(&mutex_video_frame_list);
    *frame = video_display_list.front();
    video_display_list.erase(video_display_list.begin());
    pthread_cond_signal(&cond_video_frame_list);
    pthread_mutex_unlock(&mutex_video_frame_list);
    return 1;
}



/**
 * 缓存音频帧数据
 * @param src_frame
 * @param pts
 * @param duration
 * @param pos
 * @return
 */
int Decoder::queue_sample(AVFrame *src_frame, double pts, double duration, int64_t pos){

    if (video_display_list.size() > MAX_A_DISPLAY_FRAMS) {
        pthread_mutex_lock(&mutex_audio_frame_list);
        pthread_cond_wait(&cond_audio_frame_list, &mutex_audio_frame_list);
        pthread_mutex_unlock(&mutex_audio_frame_list);
    }

    Frame *vp = static_cast<Frame *>(malloc(sizeof(struct Frame)));
    vp->width = src_frame->width;
    vp->height = src_frame->height;
    vp->format = src_frame->format;
    vp->pts = pts;
    vp->duration = duration;
    vp->pos = pos;
    vp->srcFrame = av_frame_alloc();
    av_frame_move_ref(vp->srcFrame, src_frame);

    pthread_mutex_lock(&mutex_audio_frame_list);
    audio_display_list.push_back(vp);
    FFlog("showing audio frame size = %d \n", audio_display_list.size());
    pthread_mutex_unlock(&mutex_audio_frame_list);
    return 0;

}

int Decoder::pop_sample(Frame **frame){
    if (audio_display_list.size() <= MIN_A_DISPLAY_FRAMS) {
        pthread_mutex_lock(&mutex_audio_frame_list);
        pthread_cond_signal(&cond_audio_frame_list);
        pthread_mutex_unlock(&mutex_audio_frame_list);
    }
    if (audio_display_list.size() == 0) {
        return 0;
    }
    pthread_mutex_lock(&mutex_audio_frame_list);
    *frame = audio_display_list.front();
    audio_display_list.erase(audio_display_list.begin());
    pthread_cond_signal(&cond_audio_frame_list);
    pthread_mutex_unlock(&mutex_audio_frame_list);
    return 1;
}


int Decoder::decoder_decode_frame(AVFrame *frame, AVCodec *codec, AVCodecContext *codecCtx,
                                  SafeVector<AVPacket *> *queue) {
    int ret = AVERROR(EAGAIN);
    for (;;) {
        AVPacket *pkt;

        if (!queue->isDecodeing()) {
            do {
//                if (d->queue->abort_request)
//                    return -1;

                switch (codec->type) {
                    case AVMEDIA_TYPE_VIDEO:
                        ret = avcodec_receive_frame(codecCtx, frame);
                        if (ret >= 0) {
//                            if (decoder_reorder_pts == -1) {
//                                frame->pts = frame->best_effort_timestamp;
//                            } else if (!decoder_reorder_pts) {
//                                frame->pts = frame->pkt_dts;
//                            }
                        }
                        break;
                    case AVMEDIA_TYPE_AUDIO:
                        ret = avcodec_receive_frame(codecCtx, frame);
//                        if (ret >= 0) {
//                            AVRational tb = (AVRational){1, frame->sample_rate};
//                            if (frame->pts != AV_NOPTS_VALUE)
//                                frame->pts = av_rescale_q(frame->pts, d->avctx->pkt_timebase, tb);
//                            else if (d->next_pts != AV_NOPTS_VALUE)
//                                frame->pts = av_rescale_q(d->next_pts, d->next_pts_tb, tb);
//                            if (frame->pts != AV_NOPTS_VALUE) {
//                                d->next_pts = frame->pts + frame->nb_samples;
//                                d->next_pts_tb = tb;
//                            }
//                        }
                        break;
                }
                if (ret == AVERROR_EOF) {
//                    d->finished = d->pkt_serial;
                    avcodec_flush_buffers(codecCtx);
                    return 0;
                }
                if (ret >= 0)
                    return 1;
            } while (ret != AVERROR(EAGAIN));
        }

        do {
            if (queue->getSize() == 0) {
                continue;
            }
            if (queue->packet_pending) {
//                av_packet_move_ref(&pkt, &d->pkt);
                queue->packet_pending = 0;
            } else {
                //放弃解码应该返回-1（相当于快进后，原来的list列表）  此处记录
//                if (packet_queue_get(queue, &pkt, 1, &d->pkt_serial) < 0)
//                    return -1;
                //新写代码
                pkt = queue->pop_value();
                if (pkt == NULL) {
                    continue;
                }

                //如果packet中的size不足，则重新读取
                if (queue->not_enough()) {
                    pthread_mutex_lock(&mutex_read_th);
                    pthread_cond_signal(&cond_read_th);
                    pthread_mutex_unlock(&mutex_read_th);
                }
                break;
            }
//            av_packet_unref(&pkt);
        } while (1);

        if (pkt->data == flush_pkt.data) {
            avcodec_flush_buffers(codecCtx);
//            d->finished = 0;
//            d->next_pts = d->start_pts;
//            d->next_pts_tb = d->start_pts_tb;
        } else {
//            if (codec->type == AVMEDIA_TYPE_SUBTITLE) {
//                int got_frame = 0;
//                ret = avcodec_decode_subtitle2(d->avctx, sub, &got_frame, &pkt);
//                if (ret < 0) {
//                    ret = AVERROR(EAGAIN);
//                } else {
//                    if (got_frame && !pkt.data) {
//                        d->packet_pending = 1;
//                        av_packet_move_ref(&d->pkt, &pkt);
//                    }
//                    ret = got_frame ? 0 : (pkt.data ? AVERROR(EAGAIN) : AVERROR_EOF);
//                }
//            } else {
            if (avcodec_send_packet(codecCtx, pkt) == AVERROR(EAGAIN)) {
                FFlog("Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n",
                      AV_LOG_ERROR);
//                queue.packet_pending = 1;
//                    av_packet_move_ref(&d->pkt, &pkt);
            }
//            }
            av_packet_unref(pkt);
        }
    }
}


int Decoder::get_video_frame(AVFrame *frame) {
    int got_picture;

    if ((got_picture = decoder_decode_frame(frame, videoCodec, vCodecCtx, &videoPacketList)) < 0)
        return -1;

    if (got_picture) {
        double dpts = NAN;

        if (frame->pts != AV_NOPTS_VALUE)
            dpts = av_q2d(videoStream->time_base) * frame->pts;

        frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(mInputFile->fmt_ctx, videoStream,
                                                                  frame);

        //framedrop :  drop frames when cpu is too slow
        //此处应该是抛弃无用的帧
//        if (framedrop>0 || (framedrop && get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER)) {
//            if (frame->pts != AV_NOPTS_VALUE) {
//                double diff = dpts - get_master_clock(is);
//                if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD &&
//                    diff - is->frame_last_filter_delay < 0 &&
//                    is->viddec.pkt_serial == is->vidclk.serial &&
//                    is->videoq.nb_packets) {
//                    is->frame_drops_early++;
//                    av_frame_unref(frame);
//                    got_picture = 0;
//                }
//            }
//        }
    }

    return got_picture;
}


static void pth_read_packet(void *args) {
    FFlog("this is a read packet  thread \n");
    Decoder *decoder = static_cast<Decoder *>(args);

    int start = 0;
    int ret = -1;
    for (;;) {

        if (decoder->videoState && decoder->videoState->video_seek_flag) {
            AVRational time_base = decoder->videoStream->time_base;
            int64_t seekPos = start / av_q2d(time_base);

            ret = avformat_seek_file(decoder->mInputFile->fmt_ctx, decoder->videoStreamIndex,
                                     INT64_MIN,
                                     seekPos, seekPos,
                                     AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
            if (ret < 0) {
                FFlog("av_seek_frame  faile \n");
            }

        }

//        || decoder->audioPacketList.enough()
        if (decoder->videoPacketList.enough() ) {
            struct timeval now;
            struct timespec wtime;
            //阻塞10毫秒
            gettimeofday(&now, NULL);
            wtime.tv_sec = now.tv_sec + 2;
            //休眠5毫秒
            wtime.tv_nsec = (now.tv_usec + 5 * 1000) * 1000;
            pthread_mutex_lock(&decoder->mutex_read_th);
            pthread_cond_timedwait(&decoder->cond_read_th, &decoder->mutex_read_th, &wtime);
            pthread_mutex_unlock(&decoder->mutex_read_th);
            continue;
        }

        AVPacket *avPacket = av_packet_alloc();
        ret = av_read_frame(decoder->mInputFile->fmt_ctx, avPacket);
        if (AVERROR(ret) == AVERROR_EOF) {
            FFlog("av read frame faile %d \n", AVERROR(ret));
            av_packet_unref(avPacket);
            break;
        } else if (ret < 0) {
            FFlog("av read frame faile %d \n", AVERROR(ret));
            av_packet_unref(avPacket);
            break;
        }

        if (avPacket->stream_index == decoder->videoStreamIndex) {
            decoder->videoPacketList.push_value(avPacket);
        } else if (avPacket->stream_index == decoder->audioStreamIndex) {
            decoder->audioPacketList.push_value(avPacket);
        } else {
            av_packet_unref(avPacket);
            continue;
        }
    }
}


static void th_decode_video_packet(void *argv) {
    FFlog("this is a decode video packet  thread \n");
    Decoder *decoder = static_cast<Decoder *>(argv);

    AVFrame *frame = av_frame_alloc();
    double pts;
    double duration;
    int ret;
    AVRational tb = decoder->videoStream->time_base;
    AVRational frame_rate = av_guess_frame_rate(decoder->mInputFile->fmt_ctx, decoder->videoStream,
                                                NULL);
    if (!frame) {
        FFlog("decode thread avframe alloc error \n");
        return;
    }

    for (;;) {
        ret = decoder->get_video_frame(frame);
        if (ret < 0)
            continue;
        //TODO
//            goto the_end;
        if (!ret)
            continue;

#if CONFIG_AVFILTER
        if (   last_w != frame->width
            || last_h != frame->height
            || last_format != frame->format
            || last_serial != is->viddec.pkt_serial
            || last_vfilter_idx != is->vfilter_idx) {
            av_log(NULL, AV_LOG_DEBUG,
                   "Video frame changed from size:%dx%d format:%s serial:%d to size:%dx%d format:%s serial:%d\n",
                   last_w, last_h,
                   (const char *)av_x_if_null(av_get_pix_fmt_name(last_format), "none"), last_serial,
                   frame->width, frame->height,
                   (const char *)av_x_if_null(av_get_pix_fmt_name(frame->format), "none"), is->viddec.pkt_serial);
            avfilter_graph_free(&graph);
            graph = avfilter_graph_alloc();
            if (!graph) {
                ret = AVERROR(ENOMEM);
                goto the_end;
            }
            graph->nb_threads = filter_nbthreads;
            if ((ret = configure_video_filters(graph, is, vfilters_list ? vfilters_list[is->vfilter_idx] : NULL, frame)) < 0) {
                SDL_Event event;
                event.type = FF_QUIT_EVENT;
                event.user.data1 = is;
                SDL_PushEvent(&event);
                goto the_end;
            }
            filt_in  = is->in_video_filter;
            filt_out = is->out_video_filter;
            last_w = frame->width;
            last_h = frame->height;
            last_format = frame->format;
            last_serial = is->viddec.pkt_serial;
            last_vfilter_idx = is->vfilter_idx;
            frame_rate = av_buffersink_get_frame_rate(filt_out);
        }

        ret = av_buffersrc_add_frame(filt_in, frame);
        if (ret < 0)
            goto the_end;

        while (ret >= 0) {
            is->frame_last_returned_time = av_gettime_relative() / 1000000.0;

            ret = av_buffersink_get_frame_flags(filt_out, frame, 0);
            if (ret < 0) {
                if (ret == AVERROR_EOF)
                    is->viddec.finished = is->viddec.pkt_serial;
                ret = 0;
                break;
            }

            is->frame_last_filter_delay = av_gettime_relative() / 1000000.0 - is->frame_last_returned_time;
            if (fabs(is->frame_last_filter_delay) > AV_NOSYNC_THRESHOLD / 10.0)
                is->frame_last_filter_delay = 0;
            tb = av_buffersink_get_time_base(filt_out);
#endif
        duration = (frame_rate.num && frame_rate.den ? av_q2d(
                (AVRational) {frame_rate.den, frame_rate.num}) : 0);
        pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
        ret = decoder->queue_picture(frame, pts, duration, frame->pkt_pos);
//        av_frame_unref(frame);
#if CONFIG_AVFILTER
        if (is->videoq.serial != is->viddec.pkt_serial)
                break;
        }
#endif

        if (ret < 0)
            goto the_end;
    }
    the_end:
#if CONFIG_AVFILTER
    avfilter_graph_free(&graph);
#endif
    av_frame_free(&frame);
}

static void th_decode_audio_packet(void *argv) {
    FFlog("this is a decode audio packet  thread \n");
    Decoder *decoder = static_cast<Decoder *>(argv);

    AVFrame *frame = av_frame_alloc();
#if CONFIG_AVFILTER
    int last_serial = -1;
    int64_t dec_channel_layout;
    int reconfigure;
#endif
    int got_frame = 0;
    AVRational tb;
    int ret = 0;

    if (!frame)
        return ;

    do {
        if ((got_frame = decoder->decoder_decode_frame(frame, decoder->audioCodec, decoder->aCodecCtx , &decoder->audioPacketList)) < 0){
          //TODO 结束标志
            av_frame_unref(frame);
            return ;
        }
        if (got_frame) {
            tb = (AVRational){1, frame->sample_rate};

#if CONFIG_AVFILTER
            dec_channel_layout = get_valid_channel_layout(frame->channel_layout, frame->channels);

                reconfigure =
                    cmp_audio_fmts(is->audio_filter_src.fmt, is->audio_filter_src.channels,
                                   frame->format, frame->channels)    ||
                    is->audio_filter_src.channel_layout != dec_channel_layout ||
                    is->audio_filter_src.freq           != frame->sample_rate ||
                    is->auddec.pkt_serial               != last_serial;

                if (reconfigure) {
                    char buf1[1024], buf2[1024];
                    av_get_channel_layout_string(buf1, sizeof(buf1), -1, is->audio_filter_src.channel_layout);
                    av_get_channel_layout_string(buf2, sizeof(buf2), -1, dec_channel_layout);
                    av_log(NULL, AV_LOG_DEBUG,
                           "Audio frame changed from rate:%d ch:%d fmt:%s layout:%s serial:%d to rate:%d ch:%d fmt:%s layout:%s serial:%d\n",
                           is->audio_filter_src.freq, is->audio_filter_src.channels, av_get_sample_fmt_name(is->audio_filter_src.fmt), buf1, last_serial,
                           frame->sample_rate, frame->channels, av_get_sample_fmt_name(frame->format), buf2, is->auddec.pkt_serial);

                    is->audio_filter_src.fmt            = frame->format;
                    is->audio_filter_src.channels       = frame->channels;
                    is->audio_filter_src.channel_layout = dec_channel_layout;
                    is->audio_filter_src.freq           = frame->sample_rate;
                    last_serial                         = is->auddec.pkt_serial;

                    if ((ret = configure_audio_filters(is, afilters, 1)) < 0)
                        goto the_end;
                }

            if ((ret = av_buffersrc_add_frame(is->in_audio_filter, frame)) < 0)
                goto the_end;

            while ((ret = av_buffersink_get_frame_flags(is->out_audio_filter, frame, 0)) >= 0) {
                tb = av_buffersink_get_time_base(is->out_audio_filter);
#endif

            int64_t pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
            int64_t pos = frame->pkt_pos;
//            af->serial = is->auddec.pkt_serial;
            double duration = av_q2d((AVRational){frame->nb_samples, frame->sample_rate});
            decoder->queue_sample(frame, pts, duration, pos);

#if CONFIG_AVFILTER
            if (is->audioq.serial != is->auddec.pkt_serial)
                    break;
            }
            if (ret == AVERROR_EOF)
                is->auddec.finished = is->auddec.pkt_serial;
#endif
        }
    } while (ret >= 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF);
#if CONFIG_AVFILTER
    avfilter_graph_free(&is->agraph);
#endif

}


void Decoder::preperPlay() {
    pthread_create(&th_read, NULL, reinterpret_cast<void *(*)(void *)>(pth_read_packet), this);
    pthread_create(&th_decode_video, NULL,
                   reinterpret_cast<void *(*)(void *)>(th_decode_video_packet),
                   this);
    pthread_create(&th_decode_audio, NULL,
                   reinterpret_cast<void *(*)(void *)>(th_decode_audio_packet),
                   this);

//    pthread_join(th_read, NULL);
}

Frame *Decoder::getDisplayFrame() {

    Frame *frame;
    int ret = pop_picture(&frame);

    if (ret > 0) {
        return frame;
    }
    return NULL;

}


AudioBuffer* Decoder::getAudioData(){
    int data_size, resampled_data_size;
    int64_t dec_channel_layout;
    Frame *af;

    int ret = pop_sample(&af);
    if(ret > 0){
        uint64_t out_ch_layout = AV_CH_LAYOUT_MONO;
        if(swr_ctx == NULL) {
            swr_ctx = swr_alloc();
            //重采样设置选项-----------------------------------------------------------start
            //输入的采样格式
            enum AVSampleFormat in_sample_fmt = aCodecCtx->sample_fmt;
            //输出的采样格式
            enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
            //输入的采样率
            int in_sample_rate = aCodecCtx->sample_rate;
            printf("sample rate = %d \n" ,in_sample_rate);
            //输出的采样率
            int out_sample_rate = 44100;
            //输入的声道布局
            uint64_t in_ch_layout = aCodecCtx->channel_layout;
            //输出的声道布局
            //SwrContext 设置参数
            swr_alloc_set_opts(swr_ctx,out_ch_layout,out_sample_fmt,out_sample_rate,in_ch_layout,in_sample_fmt,in_sample_rate,0,NULL);
            //初始化SwrContext
            swr_init(swr_ctx);
        }

        uint8_t * out_buffer = (uint8_t *) av_malloc(44100 * 2);
        //    输出采样位数  16位
        enum AVSampleFormat out_formart=AV_SAMPLE_FMT_S16;
        //输出的采样率必须与输入相同
        int out_sample_rate = aCodecCtx->sample_rate;

        AudioBuffer* audioBuffer = static_cast<AudioBuffer *>(malloc(sizeof(struct AudioBuffer)));

        data_size = av_samples_get_buffer_size(NULL, af->srcFrame->channels,
                                               af->srcFrame->nb_samples,
                                               static_cast<AVSampleFormat>(af->srcFrame->format), 1);

        swr_convert(swr_ctx , &out_buffer , 2 * 44100 , (const uint8_t **)af->srcFrame->data , af->srcFrame->nb_samples);

        // int swr_convert(struct SwrContext *s, uint8_t **out, int out_count,
        //                 const uint8_t **in , int in_count);

//        dec_channel_layout =
//                (af->srcFrame->channel_layout && af->srcFrame->channels == av_get_channel_layout_nb_channels(af->srcFrame->channel_layout)) ?
//                af->srcFrame->channel_layout : av_get_default_channel_layout(af->srcFrame->channels);

//
//        AudioBuffer* audioBuffer = static_cast<AudioBuffer *>(malloc(sizeof(struct AudioBuffer)));
//        data_size = av_samples_get_buffer_size(NULL, af->srcFrame->channels,
//                                               af->srcFrame->nb_samples,
//                                               static_cast<AVSampleFormat>(af->srcFrame->format), 1);
//        uint8_t * out_buffer = (uint8_t *) malloc(af->srcFrame->nb_samples);

        audioBuffer->buffer = out_buffer;
        audioBuffer->size = data_size;

        av_frame_unref(af->srcFrame);
        return audioBuffer;
    }

    return NULL;
}


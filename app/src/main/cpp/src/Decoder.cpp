#include "Decoder.h"
#include "libyuv/convert_argb.h"
#include <bits/stdint-uintn.h>
#include <cstdio>
#include <cstdlib>
#include <libavutil/error.h>
#include <string>


Decoder::Decoder(InputFile *input){
	mInputFile = input;

	//TODO
	findAudioStream();

	findVideoStream();
	initVideoDecoder();
}

int Decoder::getVideoImages(Options* opt, vector<FrameImage*> *result){
	//如果有视频流
		
	if(videoStream){
		static AVPacket avPacket;
		AVRational time_base = videoStream->time_base;
		int start =  opt->start ;
		for(int i=0 ;i <3; i++){
			int64_t seekPos =start / av_q2d(time_base);
			FrameImage * tmp = decodeOneFrame(seekPos, &avPacket);
			if (tmp) {
				result->push_back(tmp);
			}
			start=start+2;
		}
		
	}
	return 0;
}

FrameImage* Decoder::decodeOneFrame(int64_t seekPos, AVPacket* avPacket){ 

	FrameImage* tmp = NULL;

	int ret = av_seek_frame(mInputFile->fmt_ctx, videoStreamIndex,seekPos, AVSEEK_FLAG_FRAME);
	if(ret <0 ){
		fprintf(stderr, "av_seek_frame  faile \n");
		return tmp;
	}

	ret = av_read_frame(mInputFile->fmt_ctx, avPacket);
	if(ret <0){
		fprintf(stderr, "av read frame faile %d \n", AVERROR(ret));
		return tmp;
	}

	ret = avcodec_send_packet(vCodecCtx, avPacket);
	if(ret <0){
		fprintf(stderr, "error  send package \n");
		return tmp;
	}

	AVFrame *avframe = av_frame_alloc();
	ret = avcodec_receive_frame(vCodecCtx, avframe);
	if(ret <0){
		fprintf(stderr, "recive error \n");
		return tmp;
	}else{
		fprintf(stderr, "success receive frame \n");
		fprintf(stderr, " linesize : : %d, %d %d %d \n",avframe->linesize[0],avframe->linesize[1],avframe->linesize[2] ,avframe->linesize[3]);

		// int destSize = avframe->width * avframe->height *3; 
		// char* tmpData = new char[destSize];

		// libyuv::I420ToRGB24(avframe->data[0], avframe->linesize[0],
		// 		avframe->data[1], avframe->linesize[1],
		// 		avframe->data[2], avframe->linesize[2],
		// 		(uint8_t *) tmpData, avframe->width * 3,
		// 		avframe->width, avframe->height);
		// rgb2jpg((char*)"./test.jpg",tmpData, avframe->width, avframe->height);


		int dest_width = avframe->width /10;
		int dest_height = avframe->height /10;
		int dest_len =  dest_width* dest_height *3 /2;
		// int dst_u_size = dest_width /2* dest_height /2;

		uint8_t* destData = new uint8_t[dest_len];
		uint8_t* destU = destData+ dest_width* dest_height;
		uint8_t* destV = destData+ dest_width* dest_height *5 /4;



		//scale first  
		libyuv::I420Scale(avframe->data[0], avframe->width, 
		avframe->data[1], avframe->width/2, 
		avframe->data[2], avframe->width/2, 
		avframe->width, avframe->height, 
		destData, dest_width, 
		destU, dest_width/2, 
		destV, dest_width/2, 
		dest_width, dest_height, libyuv::kFilterNone);

		int destSize = dest_width * dest_height *3; 
		char* tmpData = new char[destSize];

		libyuv::I420ToRGB24(destData, dest_width,
				destU, dest_width/2,
				destV, dest_width/2,
				(uint8_t *) tmpData, dest_width * 3,
				dest_width, dest_height);
		char name[20] = {'\0'};
		sprintf(name, "./t%ld.jpg", seekPos);
		rgb2jpg(name,(char*) tmpData, dest_width, dest_height);


		tmp =(FrameImage*) malloc(sizeof(struct FrameImage));
	
		tmp->buffer= tmpData;
		tmp->width = dest_width;
		tmp->height = dest_height;

		fprintf(stderr, "yuv convert succes \n");
	}
	return tmp;
}




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

int Decoder::initVideoDecoder(){
	videoCodec = avcodec_find_decoder(videoStream->codecpar->codec_id);
	if(!videoCodec){
		fprintf(stderr, "videoCode is not find");
		return -1;
	}
	vCodecCtx = avcodec_alloc_context3(videoCodec);
	if(!vCodecCtx){
		fprintf(stderr, "video code ctx is not alloc");
		return -1;
	}

	int ret = avcodec_parameters_to_context(vCodecCtx, videoStream->codecpar);
	if(ret < 0){
		fprintf(stderr, "faile to copy codec params to decoder context \n");
		return -1;
	}

	//init decoders
	ret = avcodec_open2(vCodecCtx,videoCodec, NULL);
	if(ret < 0){
		fprintf(stderr, "Failed to open codec /n");
		return -1;
	}

	return 0;
}

int Decoder::findVideoStream(){
	if(!videoStream){
		int ret = av_find_best_stream(mInputFile ->fmt_ctx,AVMEDIA_TYPE_VIDEO, -1, -1,NULL, 0);
		if(ret < 0){
			fprintf(stderr, "no video stream \n");
			return ret;
		}
		videoStream = mInputFile ->fmt_ctx->streams[ret];
		videoStreamIndex = ret;
	}
	return 0;
}

int Decoder::findAudioStream(){
	if(!audioStream){
		int ret = av_find_best_stream(mInputFile->fmt_ctx,AVMEDIA_TYPE_AUDIO, -1, -1,NULL, 0);
		if(ret < 0){
			fprintf(stderr, "no video stream \n");
			return ret;
		}
		audioStream = mInputFile->fmt_ctx->streams[ret];
		audioStreamIndex = ret;
	}
	return 0;
}

void Decoder::decodeAudio(InputFile *inputFile){


}

void Decoder::decodeVideo(InputFile *inputFile){


}


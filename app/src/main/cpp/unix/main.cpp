#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <bits/stdint-uintn.h>
#include <cstdlib>
#include <iostream> 
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include "src/InputFile.h"
#include "src/Decoder.h"
#include "src/Player.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_render.h>

SDL_Window *window;
SDL_Renderer *render;
shared_ptr<Player> shPlayer = NULL;

static int createSDLwindow(){
  if(SDL_Init(SDL_INIT_EVERYTHING)) {
    fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
    exit(1);
  }

  window = SDL_CreateWindow("视频",100, 100,1200, 800, 0);
  if(!window){
    exit(1);
  }
  
  render = SDL_CreateRenderer(window, -1,0);
  if(!render){
    exit(1);
  }

  return 0;
}

static int getImageThums(Decoder *decoder){
	Options opt;
	opt.start = 3;
	opt.per = 4;
	vector<FrameImage *> vectorList ;
	int ret = decoder->getVideoImages(&opt, &vectorList, 10);
  return 0;
}

static int seekImageFrame(Decoder *decoder){
  FrameImage *tmp1 = decoder->decodeOneFrame(6, 4);
  FrameImage *tmp2 =  decoder->decodeOneFrame(3, 4);

  SDL_Rect rect ;
  rect.x = 0;
  rect.y = 0;
  rect.w = tmp2->width;
  rect.h = tmp2->height;

  char destData[tmp1->width * tmp1->height *4];
  for(int i=0 ; i< tmp1->width *tmp1->height; i++){
    destData[4*i] = 0xff;
    destData[4*i+1] = tmp1->buffer[i*3 +2];
    destData[4*i+2] = tmp1->buffer[i*3 + 1];
    destData[4*i+3] = tmp1->buffer [i*3 ];
  }

  SDL_Surface *ptmpSurface = SDL_CreateRGBSurfaceFrom(destData,tmp1->width, tmp1->height,4*8, tmp1->width * 4,
    0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);

  SDL_Texture* ptmTexture = SDL_CreateTextureFromSurface(render,ptmpSurface);
	
  SDL_RenderClear(render);
  SDL_RenderCopy(render,ptmTexture,NULL, &rect);
  SDL_RenderPresent(render);

  return 0;
}

static void frame_call_back(AVFrame* avframe){


  int dest_width = avframe->width / 4;
  int dest_height = avframe->height / 4;
  char *tmpData;

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



  SDL_Rect rect ;
  rect.x = 0;
  rect.y = 0;
  rect.w = dest_width;
  rect.h = dest_height;

  char showData[dest_width * dest_height *4];
  for(int i=0 ; i< dest_width*dest_height; i++){
    showData[4*i] = 0xff;
    showData[4*i+1] = tmpData[i*3 +2];
    showData[4*i+2] = tmpData[i*3 + 1];
    showData[4*i+3] = tmpData[i*3 ];
  }

  SDL_Surface *ptmpSurface = SDL_CreateRGBSurfaceFrom(showData,dest_width, dest_height,4*8, dest_width* 4,
    0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);

  SDL_Texture* ptmTexture = SDL_CreateTextureFromSurface(render,ptmpSurface);

  SDL_RenderClear(render);
  SDL_RenderCopy(render,ptmTexture,NULL, &rect);
  SDL_RenderPresent(render);

  SDL_FreeSurface(ptmpSurface);
  SDL_DestroyTexture(ptmTexture);

  free(tmpData);

}


static Uint8 *pAudio_pos;

static uint8_t audioTmpData[1024 *4];
static int audioTmpLen = 0;

FILE *  f_r_pcm = NULL;


static void fill_audio_buffer(void *userdata, Uint8 * stream, int len){
  SDL_memset(stream, 0, len);
	// 判断是否有读到数据
	// if (audio_len == 0)
	// 	return;

	// len = (len > audio_len ? audio_len : len);
	// SDL_MixAudio(stream, pAudio_pos, len, SDL_MIX_MAXVOLUME);
	// pAudio_pos += len;
	// audio_len -= len;

//  if(len >0){
//    if(audioTmpLen == 0){
//      //读取音频数据  填充
//      if(shPlayer != NULL && shPlayer.get() !=NULL){
//        Player *player = shPlayer.get();
//        int size = 0;
//        player->getAudioBufferData( &size, audioTmpData);
//        if(size > 0){
//          audioTmpLen = size;
//          pAudio_pos = audioTmpData;
//        }
//      }
//    }
//
//    if(audioTmpLen > 0){
//      int toFill = audioTmpLen >= len ? len : audioTmpLen;
//      SDL_MixAudio(stream, pAudio_pos, toFill, SDL_MIX_MAXVOLUME);
//	    pAudio_pos += toFill;
//	    audioTmpLen -= toFill;
//    }
//
//  }


  if(shPlayer != NULL && shPlayer.get() !=NULL){
    Player *player = shPlayer.get();
    int size = 0;
      player->getAudioBufferData(&size, audioTmpData);
//      FFlog("got audo size = %d \n", size);
      if(size > 0){
      len = len > size ? size: len;
      SDL_MixAudio(stream,audioTmpData ,len, SDL_MIX_MAXVOLUME);
    }
  }

  // if(f_r_pcm == NULL){
  //   f_r_pcm = fopen("/home/guoweijie004/acc.pcm", "r");
  // }

  // while(!feof(f_r_pcm )) {
  //   size_t size = fread(audioTmpData, 1, len , f_r_pcm );
  //   SDL_MixAudio(stream,audioTmpData ,size, SDL_MIX_MAXVOLUME);
  //   return ;
  // }

}

/**
 * 音频数据的回调
 * @param data
 * @param size
 */
static void audio_callback(unsigned char* data, int size){

    FFlog("receive  ---audio_callback data --- size = %d \n", size);
}

static int initAudioPlayer(){
  /*** 初始化初始化SDL_AudioSpec结构体 ***/
	SDL_AudioSpec audioSpec;
	
	// 音频数据的采样率。常用的有48000，44100等
	audioSpec.freq = 44100; 
	
	// 音频数据的格式
	audioSpec.format = AUDIO_S16SYS;
	
	// 声道数。例如单声道取值为1，立体声取值为2
	audioSpec.channels = 1;
	
	// 设置静音的值
	// audioSpec.silence = 0;

	// 音频缓冲区中的采样个数，要求必须是2的n次方
	audioSpec.samples = 1024;

	// 填充音频缓冲区的回调函数
	audioSpec.callback = fill_audio_buffer;
	/************************************/
	audioSpec.userdata = audioTmpData;

	// 打开音频设备
	if (SDL_OpenAudio(&audioSpec, nullptr) < 0)
	{
		FFlog("Can not open audio! \n");
		return -1;
	}
	FFlog("pen audio sucess! \n");
  return 0;
}


int main (int argc, char* argv[]){

	if(argc < 2){
		std::fprintf(stderr, "参数太少");
		return -1;
	}
  
  int ret = createSDLwindow();
  if (ret < 0) {
    return -1;
  }

  //初始化音频播放器
  initAudioPlayer();

	avformat_network_init();
	//first check and open InputFile
	InputFile inputFile(argv[1]);

	// Decoder decoder(&inputFile);

  //获取视频的封面
  //getImageThums(&decoder);
	
	//seek DiaplayBufferFrame in video
	//seekImageFrame(&decoder);
	

	shPlayer =make_shared<Player>();
	Player* player = shPlayer.get();
	player->addInputFile(&inputFile);
	player->preper(frame_call_back, audio_callback);
  player->play();

  SDL_PauseAudio(0);


//   while(1){
//     SDL_Delay(1);
//   }

   sleep(35);

	return 0;
}


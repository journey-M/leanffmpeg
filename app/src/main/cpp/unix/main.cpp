#include <SDL2/SDL_surface.h>
#include <iostream> 
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include "InputFile.h"
#include "Decoder.h"
#include "Player.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_render.h>

SDL_Window *window;
SDL_Renderer *render;

static int createSDLwindow(){
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
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

int main (int argc, char* argv[]){

	if(argc < 2){
		std::fprintf(stderr, "参数太少");
		return -1;
	}
  
  int ret = createSDLwindow();
  if (ret < 0) {
    return -1;
  }

	avformat_network_init();
	//first check and open InputFile
	InputFile inputFile(argv[1]);

	// Decoder decoder(&inputFile);

  //获取视频的封面
  //getImageThums(&decoder);
	
	//seek Frame in video
	//seekImageFrame(&decoder);
	

	shared_ptr<Player> shPlayer =make_shared<Player>();
	Player* player = shPlayer.get();
	player->addInputFile(&inputFile);

  player->play();


  sleep(10);

	return 0;
}


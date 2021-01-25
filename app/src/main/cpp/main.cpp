#include <iostream> 
#include <vector>
#include "InputFile.h"
#include "Decoder.h"

int main (int argc, char* argv[]){

	if(argc < 2){
		std::fprintf(stderr, "参数太少");
		return -1;
	}

	avformat_network_init();
	//first check and open InputFile
	InputFile inputFile;
	inputFile.openInputFile(argv[1]);

	Decoder decoder(&inputFile);

	Options opt;
	opt.start = 3;
	opt.per = 4;
	vector<FrameImage *> vectorList ;
	int ret = decoder.getVideoImages(&opt, &vectorList);

	
	

	//second decode parse the option 
	

	//dcode and return 

	

	return 0;
}


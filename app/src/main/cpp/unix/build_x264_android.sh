	#!/bin/bash
	
	echo ">>>>>>>>> build x264 for android arm-v7a <<<<<<<<"
	
	#NDK位置，编译结果保存位置
	export NDK=/home/guoweijie004/Android/Sdk/ndk/21.3.6528147
	#C、C++编译器所在位置
	export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64
	export CC=$TOOLCHAIN/bin/armv7a-linux-androideabi29-clang
	export CXX=$TOOLCHAIN/bin/armv7a-linux-androideabi29-clang++
	
	
	function build
	{
	echo ">>>>>>>> build start <<<<<<<<<<"
	
	  ./configure \
	  --prefix=$PREFIX \
	  --enable-static \
	  --enable-shared \
	  --enable-pic \
	  --disable-asm \
	  --disable-opencl \
	  --disable-cli \
	  --host=arm-linux \
	  --cross-prefix=$CROSS_PREFIX \
	  --sysroot=$TOOLCHAIN/sysroot \
	
	make clean
	make
	make install
	
	echo ">>>>>> build done <<<<<<"
	}

	
	export PREFIX=./armeabi-v7a
	export CROSS_PREFIX=$TOOLCHAIN/bin/arm-linux-androideabi-
	#C、C++编译器所在位置
	build
 
	# export PREFIX=./arm64
	# export PLATFORM=android-arm64
	# build
  #
	# export PREFIX=./x86
	# export PLATFORM=arandroid-x86
	# build
  #
	# export PREFIX=./x86_64
	# export PLATFORM=android-x86_64
	# build
  #

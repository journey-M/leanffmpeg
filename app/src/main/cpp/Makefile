CC = gcc
FLAGS = -Wall -g
INCS = -I. -I../jniLibs/include
LIBS = -L. -Llibs

BUILDDIR=./build/
SRCDIR=./
SRCFILES_C=$(wildcard ./*.c) 
SRCFILES_CPP=$(wildcard ./*.cpp)
OBJS_ORIGIN += $(subst .cpp,.o, $(SRCFILES_CPP))
OBJS_ORIGIN += $(subst .c,.o, $(SRCFILES_C))
OBJS += $(subst $(SRCDIR), $(BUILDDIR), $(OBJS_ORIGIN))

all : preper $(BUILDDIR)a
	@echo success

preper: 
	$(shell if [ ! -d build ]; then mkdir build ; fi;)

$(BUILDDIR)a: $(OBJS)
	echo $(LD_LIBRARY_PATH)
	$(CC) $(FLAGS) $(INCS) $(LIBS) -o $@ $^ -ljpeg -lavformat -lavcodec -lswscale -lswresample -lavutil -lstdc++ -lm -lz -lpthread -lyuv

$(BUILDDIR)%.o:$(SRCDIR)%.cpp
	$(CC) $(FLAGS) $(INCS) -o $@ -c $^

clean:
	rm *.o a build -rf


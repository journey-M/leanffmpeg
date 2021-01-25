CC = gcc
FLAGS = -Wall -g
INCS = -I. -I../../jniLibs/include
LIBS = -L. -Llibs

BUILDDIR=./build
SRCDIR=../src
SRCFILES_C=$(wildcard $(SRCDIR)/*.c) 
SRCFILES_CPP=$(wildcard $(SRCDIR)/*.cpp)
OBJS_ORIGIN += $(subst .cpp,.o, $(SRCFILES_CPP))
OBJS_ORIGIN += $(subst .c,.o, $(SRCFILES_C))
OBJS += $(subst $(SRCDIR), $(BUILDDIR)/, $(OBJS_ORIGIN))


all : preper a
	@echo success

print:
	@echo $(SRCFILES_CPP)
	@echo $(SRCFILES_CPP)
	@echo $(OBJS)

preper: 
	$(shell if [ ! -d build ]; then mkdir build ; fi;)

a: $(OBJS)
	echo $(LD_LIBRARY_PATH)
	$(CC) $(FLAGS) $(INCS) $(LIBS) -o $@ $^ -ljpeg -lavformat -lavcodec -lswscale -lswresample -lavutil -lstdc++ -lm -lz -lpthread -lyuv

$(BUILDDIR)/%.o:$(SRCDIR)%.cpp
	$(CC) $(FLAGS) $(INCS) -o $@ -c $^

clean:
	rm *.o a build -rf


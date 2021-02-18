CC = gcc
FLAGS = -Wall -g
INCS = -I. -I../../jniLibs/include -I../src
LIBS = -L. -Llibs
LIBS += $(shell pkg-config --cflags --libs sdl2)

BUILDDIR=./build
SRCDIR=../src
SRCFILES_C=$(wildcard $(SRCDIR)/*.c) 
SRCFILES_CPP=$(wildcard $(SRCDIR)/*.cpp)
OBJS_ORIGIN += $(subst .cpp,.o, $(SRCFILES_CPP))
OBJS_ORIGIN += $(subst .c,.o, $(SRCFILES_C))
OBJS += $(subst $(SRCDIR), $(BUILDDIR)/, $(OBJS_ORIGIN))


all : preper $(BUILDDIR)/a
	@echo success

print:
	@echo $(SRCFILES_CPP)
	@echo $(OBJS)
	@echo $(BUILDDIR)%.o

preper: 
	$(shell if [ ! -d build ]; then mkdir build ; fi;)

$(BUILDDIR)/a: $(BUILDDIR)/main.o $(OBJS) 
	@echo $(OBJS) 
	$(CC) $(FLAGS) $(INCS) $(LIBS) -o $@ $^ -ljpeg -lavformat -lavcodec -lswscale -lswresample -lavutil -lstdc++ -lm -lz -lpthread -lyuv

$(BUILDDIR)/main.o : main.cpp
	$(CC) $(FLAGS) $(INCS) -o $@ -c $^

$(BUILDDIR)/%.o:$(SRCDIR)%.cpp
	$(CC) $(FLAGS) $(INCS) -o $@ -c $^

clean:
	rm *.o a build -rf


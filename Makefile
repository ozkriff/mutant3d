CC=gcc
#CC=tcc
#CC=nwcc
#CC=clang
CFLAGS=-g
CFLAGS+=-std=c89
CFLAGS+=-Wall --pedantic -Wextra
CFLAGS+=-Wconversion -Wswitch-default -Wshadow
CFLAGS += -I../sdl2/exec/include/SDL2
LDFLAGS += -L../sdl2/exec/lib -lSDL2 -lSDL_image -lSDL_ttf -lGL -lGLU -lm
OBJS=mutant3d.o md5.o math.o gl.o misc.o obj.o list.o path.o widgets.o
all: mutant3d
mutant3d: $(OBJS)
	$(CC) -g -o mutant3d $(OBJS) $(LDFLAGS)
md5.o: md5.h bool.h math.h misc.h mutant3d.h gl.h
mutant3d.o: mutant3d.h bool.h list.h math.h misc.h md5.h obj.h gl.h path.h
math.o: math.h bool.h mutant3d.h misc.h
gl.o: gl.h bool.h math.h mutant3d.h misc.h
misc.o: misc.h bool.h mutant3d.h
list.o: list.h bool.h
path.o: path.h bool.h list.h math.h mutant3d.h misc.h
obj.o: obj.h bool.h math.h mutant3d.h misc.h
widgets.o: widgets.h bool.h list.h math.h mutant3d.h gl.h
clean:
	rm mutant3d $(OBJS) -f

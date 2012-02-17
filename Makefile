CC=gcc
CFLAGS=-std=c89 -g -Wall --pedantic
OBJS=mutant3d.o md5.o math.o gl.o misc.o
all: mutant3d
mutant3d: $(OBJS)
	gcc -g -o mutant3d $(OBJS) -lglfw -lGL -lGLU -lm
md5.o: md5.h
mutant3d.o:
math.o: math.h
gl.o: gl.h
misc.o: misc.h
clean:
	rm mutant3d $(OBJS) -f

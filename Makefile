CC=cc
CFLAGS=-Wall -Wextra -std=c99 -pedantic -g
INC=-I/opt/X11/include/freetype2
LIBS=-lglfw3 -lglew -lchicken -lkazmath -lfreetype
FRAMEWORKS=-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

SOURCES=main.c render.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=main

$(EXECUTABLE): $(OBJECTS) autoload
	$(CC) $(LIBS) $(FRAMEWORKS) $(OBJECTS) autoload.o -o $@

.c.o:
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

autoload:
	csc -c -embedded autoload.scm

.PHONY:
all: $(SOURCES) $(EXECUTABLE)

.PHONY:
re: all
	./main

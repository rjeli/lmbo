CC=cc
CFLAGS=-Wall -Wextra -std=c99 -pedantic -g -Wunused-parameter
INC=-I/opt/X11/include/freetype2
LIBS=-lglfw3 -lglew -lkazmath -lfreetype
FRAMEWORKS=-framework OpenGL

SOURCES=main.c render.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=main

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LIBS) $(FRAMEWORKS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

.PHONY:
all: $(SOURCES) $(EXECUTABLE)

.PHONY:
re: all
	./main

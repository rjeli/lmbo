CC=cc
CFLAGS=-Wall -Wextra -std=c99 -pedantic -g -Wno-incompatible-pointer-types-discards-qualifiers
INC=-I/opt/X11/include/freetype2 -Iinc
LIBS=-lglfw3 -lglew -lchicken -lkazmath -lfreetype
FRAMEWORKS=-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

SOURCES=main.c panel.c render.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=main

$(EXECUTABLE): $(OBJECTS) scm
	$(CC) $(LIBS) $(FRAMEWORKS) $(OBJECTS) autoload.o -o $@

.c.o:
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

scm:
	csc -c -embedded $(INC) autoload.scm

.PHONY:
all: $(EXECUTABLE)

.PHONY:
re: all
	./main

.PHONY:
clean:
	rm *.o
	rm autoload.c

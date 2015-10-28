CC=cc
CFLAGS=-Wall -Wextra -std=c99 -pedantic -g -Wno-incompatible-pointer-types-discards-qualifiers
INC=-I/opt/X11/include/freetype2 -Iinc
LIBS=-lglfw3 -lglew -lchicken -lkazmath -lfreetype
FRAMEWORKS=-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

SOURCES=$(wildcard *.c)
HEADERS=$(wildcard inc/*.h)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=main

DOC_OPTS=--output html --language c

$(EXECUTABLE): $(OBJECTS) autoload
	$(CC) $(LIBS) $(FRAMEWORKS) $(OBJECTS) autoload.o -o $@

.c.o:
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

autoload:
	csc -c -embedded $(INC) autoload.scm

.PHONY:
doc:
	cldoc generate $(CFLAGS) $(INC) -- $(DOC_OPTS) $(SOURCES) $(HEADERS)

.PHONY:
serve:
	cldoc serve html

.PHONY:
all: $(SOURCES) $(EXECUTABLE)

.PHONY:
re: all
	./main

.PHONY:
clean:
	rm *.o

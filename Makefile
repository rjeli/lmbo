CC=cc
CFLAGS=-Wall -Wextra -pedantic -g
INC=-I/opt/X11/include/freetype2
LIBS=-lglfw3 -lglew -lchicken -lkazmath -lfreetype
FRAMEWORKS=-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

all:
	csc -c -embedded autoload.scm
	$(CC) main.c autoload.o $(INC) $(LIBS) $(FRAMEWORKS)

run:
	./a.out

re: all run

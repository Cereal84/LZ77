CC = cc
CFLAGS = -g -O2 -Wall -Werror
SOURCE = source/
INCLUDE = include/
OBJECTS = main.o option.o lz77encode.o lz77decode.o bitio.o  window.o tree.o 

lz77: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) -lm
	rm *.o

main.o:  $(INCLUDE)option.h $(INCLUDE)lz77.h

option.o: $(INCLUDE)option.h
	$(CC) -c $(CFLAGS) $(SOURCE)option.c

window.o: $(INCLUDE)window.h $(INCLUDE)option.h $(INCLUDE)bitio.h
	$(CC) -c $(CFLAGS) $(SOURCE)window.c

tree.o: $(INCLUDE)tree.h $(INCLUDE)window.h
	$(CC) -c $(CFLAGS) $(SOURCE)tree.c

lz77encode.o: $(INCLUDE)lz77.h $(INCLUDE) $(INCLUDE)tree.h $(INCLUDE)bitio.h
	$(CC) -c $(CFLAGS) $(SOURCE)lz77encode.c

lz77decode.o: $(INCLUDE)lz77.h $(INCLUDE)window.h $(INCLUDE)bitio.h
	$(CC) -c $(CFLAGS) $(SOURCE)lz77decode.c

bitio.o: $(INCLUDE)bitio.h
	$(CC) -c $(CFLAGS) $(SOURCE)bitio.c

clean all: 
	rm *.o lz77

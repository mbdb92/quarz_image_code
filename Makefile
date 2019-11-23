CC=clang
CFLAGS=-DDEBUG
OBJ = quarz.out
FILE = recording.c

LIBS=-lasound

all: $(FILE)
	$(CC) $(FILE) -o $(OBJ) $(CFLAGS) $(LIBS)

clean:
	rm -f *.out

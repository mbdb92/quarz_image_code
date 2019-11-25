CC=clang
CFLAGS=
OBJ = quarz.out
FILE = recorder.c

LIBS=-lasound

debug: $(FILE)
	$(CC) $(FILE) -o $(OBJ) $(CFLAGS) -DDEBUG $(LIBS)

all: $(FILE)
	$(CC) $(FILE) -o $(OBJ) $(CFLAGS) $(LIBS)

clean:
	rm -f *.out

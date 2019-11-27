CC=clang
CFLAGS=
OBJ = quarz.out
FILE = recorder.c

LIBS=-lasound

nosub: $(FILE)
	$(CC) $(FILE) -o $(OBJ) $(CFLAGS) -DNOSUB -DDEBUG -DVERBOSE $(LIBS)

verbose: $(FILE)
	$(CC) $(FILE) -o $(OBJ) $(CFLAGS) -DVERBOSE $(LIBS)

debug: $(FILE)
	$(CC) $(FILE) -o $(OBJ) $(CFLAGS) -DDEBUG $(LIBS)

all: $(FILE)
	$(CC) $(FILE) -o $(OBJ) $(CFLAGS) $(LIBS)

clean:
	rm -f *.out

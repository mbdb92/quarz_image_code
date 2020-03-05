CC=clang
CFLAGS=
OBJ = quarz.out
FILE = quarz.c magick.c fft.c

LIBS_FFT = -lm -lfftw3
LIBS_MAGICK = `pkg-config --cflags --libs MagickWand`

verbose: $(FILE)
	$(CC) $(FILE) -o $(OBJ) $(CFLAGS) $(LIBS_FFT) $(LIBS_MAGICK) -v

clean:
	rm -f *.out *.jpg

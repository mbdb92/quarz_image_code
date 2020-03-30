CC=clang
CFLAGS=
OBJ = quarz.out
FILE = quarz.c magick.c fft.c recorder.c

LIBS_ALSA = -lasound
LIBS_FFT = -lm -lfftw3
LIBS_MAGICK = `pkg-config --cflags --libs MagickWand`

all: $(FILE)
	$(CC) $(FILE) -o $(OBJ) $(CFLAGS) $(LIBS_ALSA) $(LIBS_FFT) $(LIBS_MAGICK)

verbose: $(FILE)
	$(CC) $(FILE) -o $(OBJ) $(CFLAGS) $(LIBS_ALSA) $(LIBS_FFT) $(LIBS_MAGICK) -v -DPRINT_DEBUG

clean:
	rm -f *.out *.jpg

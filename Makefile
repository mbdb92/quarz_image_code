CC=clang
CFLAGS=
OBJ = quarz.out
FILE = quarz.c magick.c fft.c alsa.c
FILE_DEV = quarz.c alsa.c error_n_info.c fft.c sighandler.c

LIBS_STATIC = -static
LIBS_ALSA = -lasound
LIBS_FFT = -lm -lfftw3
LIBS_MAGICK = `pkg-config --cflags --libs MagickWand`

LINK_ALSA = -I/usr/lib

all: $(FILE)
	$(CC) $(FILE) -o $(OBJ) $(CFLAGS) $(LIBS_ALSA) $(LIBS_FFT) $(LIBS_MAGICK)

foo: $(FILE_DEV)
	$(CC) $(FILE_DEV) -o $(OBJ) $(CFLAGS) $(LIBS_STATIC) $(LIBS_ALSA) $(LIBS_FFT) $(LIBS_MAGICK) $(LINK_ALSA) -v -DPRINT_DEBUG

test: $(FILE_DEV)
	$(CC) $(FILE_DEV) -o $(OBJ) $(CFLAGS) $(LIBS_ALSA) $(LIBS_FFT) $(LIBS_MAGICK) $(LINK_ALSA) -v -DPRINT_DEBUG
verbose: $(FILE)
	$(CC) $(FILE) -o $(OBJ) $(CFLAGS) $(LIBS_ALSA) $(LIBS_FFT) $(LIBS_MAGICK) -v -DPRINT_DEBUG

clean:
	rm -f *.out *.jpg

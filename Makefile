CC=clang
CFLAGS=
OBJ = quarz.out
FILE = quarz.c fft.c alsa.c error_n_info.c sighandler.c magick.c

LIBS_STATIC = -static
LIBS_ALSA = -lasound
LIBS_FFT = -lm -lfftw3
LIBS_WAND_MAGICK = `pkg-config --cflags --libs MagickWand` -DWAND
LIBS_CORE_MAGICK = `pkg-config --cflags --libs MagickWand`

LINK_ALSA = -I/usr/lib

all: $(FILE)
	$(CC) $(FILE) -o $(OBJ) $(CFLAGS) $(LIBS_ALSA) $(LIBS_FFT) $(LIBS_CORE_MAGICK) -v

static: $(FILE)
	$(CC) $(FILE) -o $(OBJ) $(CFLAGS) $(LIBS_STATIC) $(LIBS_ALSA) $(LIBS_FFT) $(LIBS_MAGICK) $(LINK_ALSA) -v -DPRINT_DEBUG

verbose: $(FILE)
	$(CC) $(FILE) -o $(OBJ) $(CFLAGS) $(LIBS_ALSA) $(LIBS_FFT) $(LIBS_MAGICK) -v -DPRINT_DEBUG -DPRINT_SIGNAL

debug: $(FILE)
	$(CC) $(FILE) -o $(OBJ) $(CFLAGS) $(LIBS_ALSA) $(LIBS_FFT) $(LIBS_CORE_MAGICK) -v -DPRINT_DEBUG -DPRINT_SIGNAL -DBREAKPOINTS -DPRINT_MAGICK_DEBUG

signals: $(FILE)
	$(CC) $(FILE) -o $(OBJ) $(CFLAGS) $(LIBS_ALSA) $(LIBS_FFT) $(LIBS_MAGICK) -v -DPRINT_SIGNAL

clean:
	rm -f *.out *.jpg

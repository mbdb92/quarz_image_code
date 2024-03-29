CC=clang
CFLAGS= -Wall -Wpedantic -Wextra -Wstrict-aliasing -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wunreachable-code -lm
OBJ = quarz.out
FILE = quarz.c fft.c alsa.c error_n_info.c sighandler.c magick.c
FILE_RECORDED = quarz.c fft.c magick.c
ARG = -DLIVE

LIBS_STATIC = -static
LIBS_ALSA = -lasound
LIBS_FFT = -lm -lfftw3
LIBS_PPM = -DPPM

FLAGS_DEBUG = -DPRINT_DEBUG -DPRINT_SIGNAL -DBREAKPOINTS -DPRINT_MAGICK_DEBUG -v
FLAGS_TIME = -DTIME

LINK_ALSA = -I/usr/lib

live: $(FILE)
	$(CC) $(FILE) -o $(OBJ) $(CFLAGS) $(LIBS_ALSA) $(LIBS_FFT) $(LIBS_PPM) $(FLAGS_DEBUG) -DLIVE -DPIPE

timemessure: $(FILE)
	$(CC) $(FILE) -o $(OBJ) $(CFLAGS) $(LIBS_ALSA) $(LIBS_FFT) $(LIBS_PPM) $(FLAGS_TIME) -DLIVE -DPIPE

recorded: $(FILE_RECORDED)
	$(CC) $(FILE_RECORDED) -o $(OBJ) $(CFLAGS) $(LIBS_FFT) $(LIBS_PPM) -DRECORDED -v

static: $(FILE)
	$(CC) $(FILE) -o $(OBJ) $(CFLAGS) $(LIBS_STATIC) $(LIBS_ALSA) $(LIBS_FFT) $(LIBS_MAGICK) $(LINK_ALSA) -v 

debug: $(FILE)
	$(CC) -g $(FILE) -o $(OBJ) $(CFLAGS) $(LIBS_ALSA) $(LIBS_FFT) $(LIBS_PPM) $(FLAGS_DEBUG) $(ARG)

help:
	@echo -e "Options:\n - live\n - recorded\n - static\n - debug\nFor using on prerecorded files, please add \"ARG=-DRECORDED\" as argument to debug\ncurrently it only supports .wav or 44Bit free space at the begining"

clean:
	rm -f *.out *.jpg *.ppm *.raw

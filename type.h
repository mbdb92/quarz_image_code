#ifndef TYPE_H
#define TYPE_H

#include <fftw3.h>
#include <MagickWand/MagickWand.h>
#include <stdbool.h>

struct fft_params {
    fftw_plan plan;
    int fft_size;
};

struct magick_params {
    MagickWand *m_wand;
    DrawingWand *d_wand;
    PixelWand *c_wand;
    char *color;
};

struct quarz_params {
    struct fft_params fft;
    struct magick_params magick;
};

struct fft_data {
    fftw_complex *fft_in;
    fftw_complex *fft_out;
};

struct quarz_data {
    struct fft_data fft;
};
#endif //TYPE_H

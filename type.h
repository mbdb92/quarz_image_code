#ifndef TYPE_H
#define TYPE_H

#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <fftw3.h>

struct pid_collection {
    pid_t pid_alsa;
    pid_t pid_fft_master;
    pid_t pid_quarz;
};

struct alsa_params {
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t frames;
    long *buffer;
    int size;
};

struct fft_params {
    fftw_plan plan;
    int size;
    int rank;
};

struct fft_data {
    double *fft_in;
    double *fft_out;
};
/*
#include <alsa/asoundlib.h>
#include <MagickWand/MagickWand.h>
#include <stdbool.h>

#define OK 0
#define ERR 7
#define E_MAL_FFT_IN 1
#define E_MAL_FFT_OUT 2
#define E_ADD_FFT_PLAN 3
#define E_W_COS 4
#define E_SET_COLOR 5
#define E_MAL_MAGICK_COLOR 6
#define E_MAL_BUF 8

struct magick_params {
    MagickWand *m_wand;
    DrawingWand *d_wand;
    PixelWand *c_wand;
    char color[8];
    long max;
    long min;
};

struct quarz_params {
    struct alsa_params alsa;
    struct fft_params fft;
    struct magick_params magick;
};

struct quarz_data {
    struct fft_data fft;
};
*/
#endif //TYPE_H

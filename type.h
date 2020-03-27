#ifndef TYPE_H
#define TYPE_H

#include <alsa/asoundlib.h>
#include <fftw3.h>
#include <MagickWand/MagickWand.h>
#include <stdbool.h>

#define ALSA_PCM_NEW_HW_PARAMS_API
#define DEVICE "default"
#define CHANNEL_NUMBER 1
#define RATE 44100u
#define FRAMES 32
#define CHILD 0
#define OK 0
#define ERR 7
#define E_MAL_FFT_IN 1
#define E_MAL_FFT_OUT 2
#define E_ADD_FFT_PLAN 3
#define E_W_COS 4
#define E_SET_COLOR 5
#define E_MAL_MAGICK_COLOR 6
#define E_MAL_BUF 8
#define E_PIPE 9

struct alsa_params {
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t frames;
};

struct fft_params {
    fftw_plan plan;
    int size;
    int rank;
};

struct magick_params {
    MagickWand *m_wand;
    DrawingWand *d_wand;
    PixelWand *c_wand;
    char color[8];
    long max;
    long min;
};

struct quarz_params {
    int pipedf[2];
    struct alsa_params alsa;
    struct fft_params fft;
    struct magick_params magick;
};

struct fork_params {
    struct fft_params fft;
    struct magick_params magick;
};

struct fft_data {
    double *fft_in;
    double *fft_out;
};

struct quarz_data {
    struct fft_data fft;
};
#endif //TYPE_H

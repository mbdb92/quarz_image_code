#ifndef TYPE_H
#define TYPE_H

// For sta
#include <stdlib.h>
// For alsa
#include <alsa/asoundlib.h>
//For fft
#include <fftw3.h>
// For magick
#include <MagickWand/MagickWand.h>
//typedef int bool;
#define true 1
#define false 0

int alsa_state;
int quarz_state;
int fft_pipe_state;

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

struct magick_params {
    MagickWand *m_wand;
    DrawingWand *d_wand;
    PixelWand *c_wand;
    char color[8];
    long max;
    long min;
};

#endif //TYPE_H

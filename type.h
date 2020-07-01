#ifndef TYPE_H
#define TYPE_H

// For sta
#include <stdlib.h>
// For alsa
#include <alsa/asoundlib.h>
//For fft
#include <fftw3.h>
// For magick
#ifdef WAND
#include <MagickWand/MagickWand.h>
#endif
#ifdef CORE
#include <MagickCore/MagickCore.h>
#endif

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

struct wav_header {
    int file_size;
    short format_type;
    short channel_number;
    int sample_rate;
    short bits_per_sample;
};

#ifdef WAND
struct magick_params {
    MagickWand *m_wand;
    DrawingWand *d_wand;
    PixelWand *c_wand;
    char color[8];
    long max;
    long min;
};
#endif
#ifdef CORE
struct magick_core_params {
    ExceptionInfo *exception;
    Image *image;
    ImageInfo *image_info;
    char *pixels;
    char path[50];
    char color[8];
    long max;
    long min;
};
#endif
#ifdef PPM
struct ppm_params {
    char path[50];
    long max;
    long min;
    unsigned int color;
    long divider;
    FILE *fd;
};
#endif

#endif //TYPE_H

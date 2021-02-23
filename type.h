#ifndef TYPE_H
#define TYPE_H

// For sta
#include <stdlib.h>
#ifdef LIVE
// For alsa
#include <alsa/asoundlib.h>
#endif
//For fft
#include <fftw3.h>
// For magick

//typedef int bool;
#define true 1
#define false 0

#ifdef LIVE
extern int alsa_state;
#endif
extern int quarz_state;
extern int fft_pipe_state;
//extern unsigned int wrote_size;

struct pid_collection {
    pid_t pid_alsa;
    pid_t pid_fft_master;
    pid_t pid_quarz;
};

#ifdef LIVE
struct alsa_params {
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t frames;
    long *buffer;
    int size;
};
#endif

struct fft_params {
    fftw_plan plan;
    int size;
    int rank;
    int total_size;
};

struct fft_data {
    double *fft_in;
    double *fft_out;
};

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

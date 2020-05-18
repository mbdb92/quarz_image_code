#ifndef CODES_H
#define CODES_H

#define DEVICE "default"
#define CHANNEL_NUMBER 1
#define RATE 11025u
#define FRAMES 32

#define PI 3.14159
#define RANK 1
#define SIZE 360

#define ZERO 0
#define ALSA_READY 1
#define FFT_READY 2
#define SHMEM_READ 4
#define RUNTIME 8
#define ALSA_DONE 16
#define TERMINATE 32

#define SHIFT_A_R 0
#define SHIFT_F_R 1
#define SHIFT_S_R 2
#define SHIFT_R 3
#define SHIFT_A_D 4
#define SHIFT_T 5

#define OK 0
#define ERR 1
#define E_SIGH_QUARZ 1
#define E_PIPE 2
#define E_FORK 3

#define E_MAL_FFT_IN 1
#define E_MAL_FFT_OUT 2
#define E_ADD_FFT_PLAN 3

#define E_SET_COLOR 5
#define E_MAL_MAGICK_COLOR 6
#define E_MAL_BUF 8

#endif /*CODES_H*/

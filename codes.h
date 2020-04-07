#ifndef CODES_H
#define CODES_H

#define DEVICE "default"
#define CHANNEL_NUMBER 1
#define RATE 44100u
#define FRAMES 32

#define ZERO 0
#define ALSA_READY 1
#define ALSA_DONE 2
#define FFT_READY 4
#define PIPE_DONE 8
#define SIZE_NEEDED 16
#define RUNTIME 32
#define READ_PIPE 64

#define SHIFT_A_R 0
#define SHIFT_A_D 1
#define SHIFT_F_R 2
#define SHIFT_P_D 3
#define SHIFT_S_N 4
#define SHIFT_R 5
#define SHIFT_R_P 6

#define OK 0
#define ERR 1
#define E_SIGH_QUARZ 1
#define E_PIPE 2
#define E_FORK 3

#define E_MAL_FFT_IN 1
#define E_MAL_FFT_OUT 2
#define E_ADD_FFT_PLAN 3
/*
#define E_SET_COLOR 5
#define E_MAL_MAGICK_COLOR 6
#define E_MAL_BUF 8
*/
#endif /*CODES_H*/

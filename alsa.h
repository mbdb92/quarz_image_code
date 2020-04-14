#ifndef RECORDER_H
#define RECORDER_H

#include <alsa/asoundlib.h>
#include "type.h"

#define OK 0
#define PCM_OPEN_FAIL 1
#define HW_ANY_PARAMS_FAIL 2
#define HW_SET_ACCESS_FAIL 3
#define HW_SET_FORMAT_FAIL 4
#define HW_SET_CHANNELS_FAIL 5
#define HW_SET_RATE_FAIL 6
#define HW_SET_PERIOD_FAIL 7
#define HW_PARAMS_ERROR 8
#define MALLOC_ERROR 9

void alsa_sig_handler( int signum );
int setup_pcm_struct( snd_pcm_t *handle, snd_pcm_hw_params_t *params );
snd_pcm_t * open_device( const char *name, snd_pcm_stream_t stream, int mode );
int close_device( struct alsa_params *alsa );
int record_to_buffer( long *buffer, struct alsa_params *alsa );
int run_loop( int loops, long *buffer, struct alsa_params *alsa );
int alsa_handler( int pipefd[2], void *shmem );

#endif /*RECORDER_H*/

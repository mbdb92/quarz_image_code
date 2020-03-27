//For Debugging
#include <assert.h>
// For alsa ahndling
#include <alsa/asoundlib.h>
// For processes
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
// For defined functions
#include "type.h"
#include "fft.h"
#include "magick.h"
#include "recorder.h"

int create_image_strip ( long *input ) {
    struct fork_params fparams;
    struct fft_data fdata;

    fparams = malloc( sizeof(struct fork_params) );
    fdata = malloc( sizeof(struct fft_data) );
}

void audio_process ( struct quarz_params *params ) {
    int return_value, dir;
    long *audio_buffer;

    params->alsa.handle = open_device( DEVICE, SND_PCM_STREAM_CAPTURE, 0 );
    snd_pcm_hw_params_malloc( &params->alsa.params );

    return_value = setup_pcm_struct( params->alsa.handle, params->alsa.params );
    return_value = snd_pcm_hw_params_get_period_size( params->alsa.params, &params->alsa.frames, &dir );
    params->fft.size = params->alsa.frames * 4;

    audio_buffer = (long *) malloc(params->fft.size);
    if( audio_buffer == NULL ) {
        return E_MAL_BUF;
    }
#ifdef PRINT_DEBUG
    printf("Size of frame: ");
    printf("%i\n", params->fft.size);
#endif

    while (true) {
        return_value = record_to_buffer( audio_buffer, params->alsa );
        if( return_value == OK ) {
            
}

void fft_process ( struct quarz_params *params ) {

}

int main() {

    /*
     * These two structs contain every value needed during runtime
     * Constants are excluded
     */
    struct quarz_params *params;
    struct quarz_data *data;
    int return_value;

    params = malloc( sizeof(struct quarz_params) );
    data = malloc( sizeof(struct quarz_data) );

    return_value = pipe(params.pipefd);

    if( return_vale != OK )
        exit E_PIPE;

    return_value = fork();
    if( return_value == CHILD ) {
        audio_process( params );
    } else {
        return_value = fork();
        if( return_value == CHILD ) {
            fft_process( params );
        }
    }

    
    if( return_value != 0 ) {
        long *faudio_buffer;

        faudio_buffer = (long *) malloc(params->fft.size);
        memcpy( faudio_buffer

    /*
     * this block calls the setup functions for startup
     */
    return_value = create_fft( &params->fft, &data->fft );
    if( return_value != OK )
        return ERR;
    return_value = setup_drawing( &params->magick );
    if( return_value != OK )
        return ERR;

    run_fft( &params->fft, &data->fft, audio_buffer );
    run_magick_from_fft( &params->magick, &data->fft, (unsigned long) params->fft.size );

    /*
     * This block contains the tear-down functions
     */
#ifdef PRINF_DEBUG
    printf("Starting cleanup\n");
#endif
    destroy_fft( &params->fft, &data->fft );
    destroy_drawing( &params->magick );

#ifdef PRINTF_DEBUG
    printf("Freeing Audio\n");
#endif
    snd_pcm_drain(params->alsa.handle);
    snd_pcm_close(params->alsa.handle);

#ifdef PRINTF_DEBUG
    printf("Freeing buffers\n");
#endif
    free(audio_buffer);
    free(params);
    free(data);
    return OK;
}

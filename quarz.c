//For Debugging
#include <assert.h>
// For alsa ahndling
#include <alsa/asoundlib.h>
// For defined functions
#include "type.h"
#include "fft.h"
#include "magick.h"
#include "recorder.h"

int main() {

    /*
     * These two structs contain every value needed during runtime
     * Constants are excluded
     */
    struct quarz_params *params;
    struct quarz_data *data;
    int return_value, dir;
    long *audio_buffer;

    params = malloc( sizeof(struct quarz_params) );
    data = malloc( sizeof(struct quarz_data) );

    params->alsa.handle = open_device( DEVICE, SND_PCM_STREAM_CAPTURE, 0 );
    snd_pcm_hw_params_malloc( &params->alsa.params );

    return_value = setup_pcm_struct( params->alsa.handle, params->alsa.params );
    return_value = snd_pcm_hw_params_get_period_size( params->alsa.params, &params->alsa.frames, &dir );
    params->fft.size = params->alsa.frames * 4;

    audio_buffer = (long *) malloc(params->fft.size);
    if( audio_buffer == NULL ) {
        return E_MAL_BUF;
    }

    return_value = record_to_buffer( audio_buffer, params->alsa );
    params->fft.size = return_value;
#ifdef PRINT_DEBUG
    printf("Size of frame: ");
    printf("%i\n", params->fft.size);
#endif

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
    printf("Starting cleanup\n");
    destroy_fft( &params->fft, &data->fft );
    destroy_drawing( &params->magick );

    printf("Freeing buffers\n");
//    free(audio_buffer);
    free(params);
    free(data);
    return OK;
}

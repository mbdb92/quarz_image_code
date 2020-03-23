#include "recorder.h"
#include <alsa/asoundlib.h>
// for file access
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

static snd_pcm_t * Handle;

#ifndef NOSUB
/*
 * This takes the pointer to the array of params and sets the needed parameters
 * Once this is done, the settings will be applied
 * Since I want to be able to easily change between MMAP and Direct Read
 * I don't want to rewrite this codepart every time.
 *
 * TODO: Resolving error with this function and removing this comment
 * TODO: Change so SND_PCM_ACCESS can be changed
 */
int setup_pcm_struct( snd_pcm_t *handle, snd_pcm_hw_params_t *params ) {
    int rc, dir;
    unsigned int rate = RATE;
    snd_pcm_uframes_t frames = FRAMES;
    /*
     * The initialisation of the pointer at this point didn't work, they weren't returned to
     * the calling function. removing them out of the function worked as intended
    */
    // Sets the struct with default values before refining
    rc = snd_pcm_hw_params_any(handle, params);
    if (rc < 0)
        return HW_ANY_PARAMS_FAIL;
    // Sets the access form
    rc = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (rc < 0)
        return HW_SET_ACCESS_FAIL;
    // How the incoming data is defined. I.e. it could also be 8-Bit
    // I think this is the resolution of the samples
    rc = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    if (rc < 0)
        return HW_SET_FORMAT_FAIL;
    // How many channels should be recorded
    rc = snd_pcm_hw_params_set_channels(handle, params, CHANNEL_NUMBER);
    if (rc < 0)
        return HW_SET_CHANNELS_FAIL;
    // This sets the sampling rate (TODO: check this comment!)
    rc = snd_pcm_hw_params_set_rate_near(handle, params, &rate, &dir);
    if (rc < 0)
        return HW_SET_RATE_FAIL;
    // How may frames will be in storage TODO: CHECK THIS COMMENT!!!!
    rc = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
    if (rc < 0)
        return HW_SET_PERIOD_FAIL;
#ifdef PRINT_DEBUG
    printf("Setting Params\n");
#endif
    // Once the struct is filled, the settings get applied to the device
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0)
        return HW_PARAMS_ERROR;
#ifdef PRINT_DEBUG
    printf("Prepare Device\n");
#endif
    // This shouldn't be needed, as the device should be prepared after the previous step
    snd_pcm_prepare( handle );
    if (rc < 0)
        return HW_PARAMS_ERROR;
    return OK;
}

/*
 * This function is a wrapper around snd_pcm_open so it returns the pointer
 * I'm using this, so I can define it at one point and reuse the function
 *
 * TODO: Is this even necessary??
 */
snd_pcm_t * open_device( const char *name, snd_pcm_stream_t stream, int mode ) {
    int rc;
    static snd_pcm_t *handle;
#ifdef PRINT_DEBUG
    printf("Open Device\n");
#endif
    rc = snd_pcm_open( &handle, DEVICE, SND_PCM_STREAM_CAPTURE, mode );
    if (rc < 0) {
//        print_error_code( PCM_OPEN_FAIL );
    }
#ifdef DEBUG
    Handle = handle;
#endif
    return handle;
}
#endif /*N-NOSUB*/



int record_to_file( long *buffer ) {
    static long loops;
    static int rc, dir, size;
    static snd_pcm_t *handle;
    static snd_pcm_hw_params_t *params;
    static snd_pcm_uframes_t frames;
//    static double *buffer;
#ifdef FILE
    static int fd;

    /*
     * This Block takes care of the filehandling
     */
    fd = open("output.raw", O_RDWR | O_CREAT);
    if (fd == -1) {
        fprintf(stderr, "unable to open file: %s\n", strerror(errno));
        exit(4);
    }
#endif


    /*
     * This block takes care of the device initialisation
     */
    handle = open_device( DEVICE, SND_PCM_STREAM_CAPTURE, 0 );
    if (rc < 0) {
//        print_error_code( PCM_OPEN_FAIL );
    }
#ifdef VERBOSE
    check_state( handle );
#endif
    // alloca hat keinen returnvalue da es ein Macro ist
    snd_pcm_hw_params_malloc(&params);



    /*
     * This Block prepares the device for usage
     */
#ifdef PRINT_DEBUG
    printf("Preparing Device\n");
#endif
    rc = setup_pcm_struct( handle, params );
    if (rc != OK) {
 //       print_error_code( rc );
        exit(1);
    }



    /*
     * This Block takes care of the buffer, the sound is read to, before used further
     */
#ifdef PRINT_DEBUG
    printf("Allocating Buffer\n");
#endif
    rc = snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    if (rc < 0) {
//        print_error_code( rc );
        exit(1);
    }
    size = frames * 4;
#ifdef PRINT_DEBUG
    printf("Size of Buffer is %i", size);
#endif /*VERBOSE*/
    buffer = (long *) malloc(size);
    if (buffer == NULL){
//        print_error_code( MALLOC_ERROR );
        return MALLOC_ERROR;
    }
#ifdef PRINT_DEBUG
    printf("Entering Loop\n");
#endif

    unsigned int val = 0;
    snd_pcm_hw_params_get_period_time(params, &val, &dir);
    snd_pcm_hw_params_get_rate(params, &val, &dir);

    snd_pcm_hw_params_free(params);

    loops = 1;
    while (loops > 0) {
        loops--;
        rc = snd_pcm_readi(handle, buffer, frames);

        if (rc == -EPIPE) {
            fprintf(stderr, "overrun occured\n");
            snd_pcm_prepare(handle);
        }else if (rc < 0) {
            fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
        }else if (rc != (int)frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        } else {
#ifdef PRINT_DEBUG
            printf("Frames read: %i\n", rc);
#endif
        }
#ifdef FILE
        rc = write(fd, buffer, size);
        if (rc != size) {
            fprintf(stderr, "short write: wrote %d bytes\n", rc);
        }
#endif
    }

    snd_pcm_drain(handle);
    snd_pcm_close(handle);
#ifdef FILE
    close(fd);
#endif
//:    free(buffer);
    printf("%i\n", size);
    return size;
}


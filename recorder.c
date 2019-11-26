#include "recorder.h"
#include <alsa/asoundlib.h>
// for time function
#ifdef VERBOSE
#include <time.h>
#endif
// for file access
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

// Helper function for debugging errors
void check_state( snd_pcm_t *handle ) {
    switch( (unsigned int) snd_pcm_state(handle) ) {
            case SND_PCM_STATE_OPEN:
                printf("State is OPEN\n");
                break;
            case SND_PCM_STATE_SETUP:
                printf("State is SETUP\n");
                break;
            case SND_PCM_STATE_PREPARED:
                printf("State is PREPARED\n");
                break;
            case SND_PCM_STATE_RUNNING:
                printf("State is RUNNING\n");
                break;
            case SND_PCM_STATE_XRUN:
                printf("State is XRUN\n");
                break;
            case SND_PCM_STATE_DRAINING:
                printf("State is DRAINING\n");
                break;
            case SND_PCM_STATE_PAUSED:
                printf("State is PAUSED\n");
                break;
            case SND_PCM_STATE_SUSPENDED:
                printf("State is SUSPENDED\n");
                break;
            case SND_PCM_STATE_DISCONNECTED:
                printf("State is DISCONNECTED\n");
                break;
            default: 
                break;
    }
}

// Central error code resolver, so I can change and edit them at one point
void print_error_code( int errnr ) {
    switch( errnr ) {
            case PCM_OPEN_FAIL:
                fprintf(stderr, "snd_pcm_open() failed: %s\n", snd_strerror(errnr));
                break;
            case HW_ANY_PARAMS_FAIL:
                fprintf(stderr, "snd_pcm_hw_params_any() failed: %s\n", snd_strerror(errnr));
                break;
            case HW_SET_ACCESS_FAIL:
                fprintf(stderr, "snd_pcm_hw_params_set_access() failed: %s\n", snd_strerror(errnr));
                break;
            case HW_SET_FORMAT_FAIL:
                fprintf(stderr, "snd_pcm_hw_params_set_format() failed: %s\n", snd_strerror(errnr));
                break;
            case HW_SET_CHANNELS_FAIL:
                fprintf(stderr, "snd_pcm_hw_params_set_channels() failed: %s\n", snd_strerror(errnr));
                break;
            case HW_SET_RATE_FAIL:
                fprintf(stderr, "snd_pcm_hw_params_set_rate_near() failed: %s\n", snd_strerror(errnr));
                break;
            case HW_SET_PERIOD_FAIL:
                fprintf(stderr, "snd_pcm_hw_params_set_period_size_near() failed: %s\n", snd_strerror(errnr));
                break;
            case HW_PARAMS_ERROR:
                fprintf(stderr, "Setting struct failed\n");
                fprintf(stderr, "snd_pcm_hw_params() failed: %s\n", snd_strerror(errnr));
                break;
            default: 
                break;
    }
}

int setup_pcm_struct( snd_pcm_t *handle, snd_pcm_hw_params_t *params ) {
    int rc, dir;
    unsigned int rate = RATE;
    snd_pcm_uframes_t frames = FRAMES;
    /*
     * The initialisation of the pointer at this point didn't work, they weren't returned to
     * the calling function. removing them out of the function worked as intended
    */
    rc = snd_pcm_hw_params_any(handle, params);
    if (rc < 0)
        return HW_ANY_PARAMS_FAIL;
    rc = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (rc < 0)
        return HW_SET_ACCESS_FAIL;
    rc = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    if (rc < 0)
        return HW_SET_FORMAT_FAIL;
    rc = snd_pcm_hw_params_set_channels(handle, params, CHANNEL_NUMBER);
    if (rc < 0)
        return HW_SET_CHANNELS_FAIL;
    rc = snd_pcm_hw_params_set_rate_near(handle, params, &rate, &dir);
    if (rc < 0)
        return HW_SET_RATE_FAIL;
    rc = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
    if (rc < 0)
        return HW_SET_PERIOD_FAIL;
    return OK;
}

int seting_device_values ( snd_pcm_t *handle, snd_pcm_hw_params_t *params ) {
    int rc, dir;
#ifdef DEBUG
    printf("Setting Params\n");
#endif
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0)
        return HW_PARAMS_ERROR;
#ifdef DEBUG
    printf("Prepare Device\n");
#endif
    snd_pcm_prepare( handle );
    if (rc < 0)
        return HW_PARAMS_ERROR;
    return OK;
}

/*
 * This function is a wrapper around snd_pcm_open so it returns the pointer
 * I'm using this, so I can define it at one point and reuse the function
 */
snd_pcm_t * open_device( const char *name, snd_pcm_stream_t stream, int mode ) {
    int rc;
    static snd_pcm_t *handle;
#ifdef DEBUG
    printf("Open Device\n");
#endif
    rc = snd_pcm_open( &handle, DEVICE, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK );
    if (rc < 0) {
        print_error_code( PCM_OPEN_FAIL );
        return NULL;
    }
    return handle;
}

int record_to_file () {
    static long loops;
    static int rc, dir, size;
    static snd_pcm_t *handle;
    static snd_pcm_hw_params_t *params;
    static snd_pcm_uframes_t frames;
    static char *buffer;
    static int fd;
#ifdef VERBOSE
    clock_t t, dt, cycle;
    double time_taken;
#endif

    fd = open("output.raw", O_WRONLY | O_CREAT);
    if (fd == -1) {
        fprintf(stderr, "unable to open file: %s\n", strerror(errno));
        exit(4);
    }

    handle = open_device( DEVICE, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK );
    if (handle == NULL)
        exit(1);
#ifdef VERBOSE
    check_state( handle );
#endif
    // alloca hat keinen returnvalue da es ein Macro ist
    snd_pcm_hw_params_alloca(&params);

#ifdef DEBUG
    printf("Filling PCM Struct\n");
#endif
    rc = setup_pcm_struct( handle, params );
    if (rc != OK) {
        print_error_code( rc );
        exit(1);
    }

    rc = seting_device_values( handle, params );
    if (rc != OK) {
        print_error_code( rc );
        exit(1);
    }

    rc = snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    if (rc < 0) {
        print_error_code( rc );
        exit(1);
    }
#ifdef DEBUG
    printf("Allocating Buffer\n");
#endif
    size = frames * 4;
    buffer = (char *) malloc(size);
    if (buffer == NULL){
        print_error_code( MALLOC_ERROR );
        return MALLOC_ERROR;
    }
#ifdef DEBUG
    printf("Entering Loop\n");
#endif

    loops = 2;
    while (loops > 0) {
        loops--;
#ifdef VERBOSE
        t = clock();
#endif
        rc = snd_pcm_readi(handle, buffer, frames);
#ifdef VERBOSE
        dt = clock() - t;
        cycle = dt;
        time_taken = ((double)dt)/CLOCKS_PER_SEC; // in seconds
        printf("read: %f seconds\n", time_taken); 
#endif

        if (rc == -EPIPE) {
            fprintf(stderr, "overrun occured\n");
            snd_pcm_prepare(handle);
        }else if (rc < 0) {
            fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
        }else if (rc != (int)frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        } else {
#ifdef DEBUG
            printf("Frames read: %i\n", rc);
#endif
        }
#ifdef VERBOSE
        t = clock();
#endif
        rc = write(fd, buffer, size);
        if (rc != size) {
            fprintf(stderr, "short write: wrote %d bytes\n", rc);
        }
#ifdef VERBOSE
        dt = clock() - t;
        cycle = cycle + dt;
        time_taken = ((double)dt)/CLOCKS_PER_SEC; // in seconds
        printf("write: %f seconds\n", time_taken); 
        time_taken = ((double)cycle)/CLOCKS_PER_SEC; // in seconds
        printf("cycle: %f seconds\n", time_taken); 
#endif
    }

    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    close(fd);
    free(buffer);

    return 0;
}

int main() {
    record_to_file();
}

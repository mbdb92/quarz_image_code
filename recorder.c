#include "recorder.h"
#include <alsa/asoundlib.h>
// for time function
#include <time.h>
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
#ifdef DEBUG
    printf("Rate is %i \n", rate);
#endif
    snd_pcm_uframes_t frames = FRAMES;
    /*
     * The initialisation of the pointer at this point didn't work, they weren't returned to
     * the calling function. removing them out of the divice worked as intended
    */
    /*
    rc = snd_pcm_open( &handle, DEVICE, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK );
    if (rc < 0)
        return PCM_OPEN_FAIL;
    snd_pcm_hw_params_alloca(&params);
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

int seting_device_values ( snd_pcm_t *handle, snd_pcm_hw_params_t *params, snd_pcm_uframes_t *frames) {
    int rc, dir;
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0)
        return HW_PARAMS_ERROR;
    rc = snd_pcm_hw_params_get_period_size(params, frames, &dir);
    if (rc < 0)
        return HW_PARAMS_ERROR;
    return OK;
}

int record_to_file () {
    long loops;
    int rc, dir, size;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t frames;
    char *buffer;
    int fd;
#ifdef DEBUG
    clock_t t, dt, cycle;
    double time_taken;
#endif

    fd = open("output.raw", O_WRONLY | O_CREAT);
    if (fd == -1) {
        fprintf(stderr, "unable to open file: %s\n", strerror(errno));
        exit(4);
    }

    rc = snd_pcm_open( &handle, DEVICE, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK );
    if (rc < 0)
        return PCM_OPEN_FAIL;
    // alloca hat keinen returnvalue da es ein Macro ist
    snd_pcm_hw_params_alloca(&params);

    rc = setup_pcm_struct( handle, params );
    if (rc != OK) {
        print_error_code( rc );
        exit(1);
    }

    rc = seting_device_values( handle, params, &frames );
    if (rc != OK) {
        print_error_code( rc );
        exit(1);
    }
    size = frames * 4;
    buffer = (char *) malloc(size);
    if (buffer == NULL)
        return MALLOC_ERROR;
#ifdef DEBUG
    printf("frames: %i\n", frames);
#endif

    loops = 2;
    while (loops > 0) {
        loops--;
#ifdef DEBUG
        t = clock();
#endif
        rc = snd_pcm_readi(handle, buffer, frames);
#ifdef DEBUG
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
#ifdef DEBUG
        t = clock();
#endif
        rc = write(fd, buffer, size);
        if (rc != size) {
            fprintf(stderr, "short write: wrote %d bytes\n", rc);
        }
#ifdef DEBUG
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

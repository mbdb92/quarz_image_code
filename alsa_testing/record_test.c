#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>
// For time function
#include <time.h>
// for file access
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

int main () {
    long loops;
    int rc;
    int size;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int val;
    int dir;
    snd_pcm_uframes_t frames;
    char *buffer;
    clock_t t, dt, cycle;
    int fd;
    double time_taken;

    fd = open("record.raw", O_WRONLY | O_CREAT);
    if (fd == -1) {
        fprintf(stderr, "unable to open file: %s\n", strerror(errno));
        exit(4);
    }

    rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_CAPTURE, 0);

    if (rc < 0){
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
        exit(1);
    }
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(handle, params, 2);
    val = 44100;
    snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
    frames = 32;
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
    
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        fprintf (stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
        exit(2);
    }

    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    size = frames * 4;
    buffer = (char *) malloc(size);
    snd_pcm_hw_params_get_period_time(params, &val, &dir);
    loops = 2;

    rc = snd_pcm_hw_params_get_rate(params, &val, &dir);
    printf("rate is: %i\n", val);

    while (loops > 0) {
        loops--;
        t = clock();
        rc = snd_pcm_readi(handle, buffer, frames);
        dt = clock() - t;
        cycle = dt;
        time_taken = ((double)dt)/CLOCKS_PER_SEC; // in seconds
        printf("read: %f seconds\n", time_taken); 

        if (rc == -EPIPE) {
            fprintf(stderr, "overrun occured\n");
            snd_pcm_prepare(handle);
        }else if (rc < 0) {
            fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
        }else if (rc != (int)frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        } else {
            printf("Frames read: %i\n", rc);
        }
        t = clock();
        rc = write(fd, buffer, size);
        if (rc != size) {
            fprintf(stderr, "short write: wrote %d bytes\n", rc);
        }
        dt = clock() - t;
        cycle = cycle + dt;
        time_taken = ((double)dt)/CLOCKS_PER_SEC; // in seconds
        printf("write: %f seconds\n", time_taken); 
        time_taken = ((double)cycle)/CLOCKS_PER_SEC; // in seconds
        //printf("cycle: %f seconds\n", time_taken); 
    }

    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    close(fd);
    free(buffer);

    return 0;
}

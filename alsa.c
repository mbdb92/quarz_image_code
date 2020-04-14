/*
#include "alsa.h"
#include <alsa/asoundlib.h>
// for file access
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "type.h"
*/
#include "alsa.h"
#include <alsa/asoundlib.h>
// For signals
#include <signal.h>
// Default
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
// For ?
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
// For sighandler
#include "sighandler.h"

#include "type.h"
#include "codes.h"
#include "error_n_info.h"

struct sigaction alsa_act;

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
    return handle;
}



int record_to_buffer( long *buffer, struct alsa_params *alsa ) {
    long loops;
    int rc, dir, size;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t frames;
//    handle = alsa.handle;
//    params = alsa.params;
//    frames = alsa.frames;
//    static double *buffer;


    /*
     * This block takes care of the device initialisation
     */
    alsa->handle = open_device( DEVICE, SND_PCM_STREAM_CAPTURE, 0 );
    if (rc < 0) {
        print_error_code( PCM_OPEN_FAIL );
    }
#ifdef VERBOSE
    check_state( alsa->handle );
#endif
    // alloca hat keinen returnvalue da es ein Macro ist
    /*
     * This is needed for alsa to work
     * if not included, it fails with:
     * "snd_pcm_open(): Operation not allowed"
     * no idea why or what part really is the problem
     */
    snd_pcm_hw_params_malloc(&params);
    alsa->params = params;



    /*
     * This Block prepares the device for usage
     */
#ifdef PRINT_DEBUG
    printf("Preparing Device\n");
#endif
    rc = setup_pcm_struct( alsa->handle, alsa->params );
    if (rc != OK) {
        print_error_code( rc );
        exit(1);
    }



    /*
     * This Block takes care of the buffer, the sound is read to, before used further
     */
#ifdef PRINT_DEBUG
    printf("Allocating Buffer\n");
#endif
    rc = snd_pcm_hw_params_get_period_size(alsa->params, &alsa->frames, &dir);
    if (rc < 0) {
        print_error_code( rc );
        exit(1);
    }
    alsa->size = alsa->frames * 4;

    /*
     * This block is needed to tell fft to continue
     * TODO: Move this to a better place and split this function
     */
//    alsa->size = size;
    kill( getpid(), SIGUSR2 );

#ifdef PRINT_DEBUG
    printf("Size of Buffer is %i\n", size);
#endif /*VERBOSE*/
    buffer = (long *) malloc(size);
    if (buffer == NULL){
        print_error_code( MALLOC_ERROR );
        return MALLOC_ERROR;
    }
#ifdef PRINT_DEBUG
    printf("Entering Loop\n");
#endif

    unsigned int val = 0;
    snd_pcm_hw_params_get_period_time(alsa->params, &val, &dir);
    snd_pcm_hw_params_get_rate(alsa->params, &val, &dir);

    snd_pcm_hw_params_free(alsa->params);

    loops = 1;
    while (loops > 0) {
        loops--;
        rc = snd_pcm_readi(alsa->handle, buffer, alsa->frames);

        if (rc == -EPIPE) {
            fprintf(stderr, "overrun occured\n");
            snd_pcm_prepare(alsa->handle);
        }else if (rc < 0) {
            fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
        }else if (rc != (int)alsa->frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        } else {
#ifdef PRINT_DEBUG
            printf("Frames read: %i\n", rc);
#endif
        }
    }

    snd_pcm_drain(alsa->handle);
    snd_pcm_close(alsa->handle);
//:    free(buffer);
//    printf("%i\n", size);
    return size;
}

int alsa_handler( int pipefd[2], void *shmem ) {
    struct pid_collection *pids;
    struct alsa_params *alsa;
    int rc, dir;
    void (*sigHandlerReturn)(int);
    long *buffer;
    bool loop;

    alsa_state = ZERO;
    pids = malloc( sizeof(struct pid_collection) );
    alsa = malloc( sizeof(struct alsa_params) );
    loop = true;

    // alsa_act is global defined!
    // The struct containes the needed informations
    memset( &alsa_act, 0, sizeof(alsa_act) );

    alsa_act.sa_sigaction = alsa_sig_handler;
    alsa_act.sa_flags = SA_SIGINFO;

    sigaction( SIGUSR1, &alsa_act, 0 );
    sigaction( SIGCONT, &alsa_act, 0 );

    kill (getppid(), SIGUSR1 );
#ifdef PRINT_DEBUG
    // Needed, as alsa currently doesn't know it's own pid
    pids->pid_alsa = getpid();
#endif

    if( !((alsa_state & SHMEM_READ) >> SHIFT_S_R ) ) {
#ifdef PRINT_DEBUG
        printf("(alsa) %i; Waiting for SIGUSR1!\n", pids->pid_alsa);
#endif
        suspend( &alsa_state, SHMEM_READ, SHIFT_S_R );
    }

    memcpy( pids, shmem, sizeof(struct pid_collection) );

    rc = record_to_buffer( buffer, alsa );

    memcpy( &shmem[ sizeof(pids) ], &alsa->size, sizeof(alsa->size) );
    kill( pids->pid_fft_master, SIGUSR1 );

    /*
     * Waiting for fft_master to complete it's setup
     * once this is done, alsa can continue
     */
    if( !((alsa_state & RUNTIME) >> SHIFT_R ) ) {
#ifdef PRINT_SIGNAL
        printf("(alsa) %i: Waiting for SIGCONT to start RUNTIME!\n", pids->pid_alsa);
#endif
        suspend( &alsa_state, RUNTIME, SHIFT_R );
    }

    /*
     * Main RUNTIME Loop!
     */

    char terminal = "\n";
    rc = write( pipefd[1], &buffer, sizeof(buffer) );
    printf("(alsa) %i: wrote %i to pipe\n", pids->pid_alsa, rc);
    write( pipefd[1], &terminal, sizeof(terminal) );
#ifdef PRINT_SIGNAL
    printf("(alsa) %i: Send SIGCONT to (fft) %i\n", pids->pid_alsa, pids->pid_fft_master);
#endif
    kill( pids->pid_fft_master, SIGCONT );

    /*
     * Cleanup code
     */
   // rc = close_device( alsa );
    free(alsa->buffer);
    free(alsa);
    free(pids);
    return OK;
}

// Include needed types and codes
#include "type.h"
#include "codes.h"
// FFTW
#include <fftw3.h>
// sin()
#include <stdio.h>
#include <math.h>
//gnuplot
#include <stdlib.h>
//malloc_usabel_size
#include <malloc.h>
#ifdef LIVE
// For Signals
#include <signal.h>
#include "sighandler.h"
#include <unistd.h>
#endif
// For open
#include <sys/stat.h>
#include <fcntl.h>
// To access magick runtime
#ifdef PPM
#include "magick.h"
#endif

#include "fft.h"

#ifdef LIVE
struct sigaction fft_act;
#endif /* LIVE */
#ifdef RINGBUFFER
unsigned int pc_count = ZERO;
#endif /* RINGBUFFER */
//extern int quarz_state;
//extern int alsa_state;
//extern int fft_pipe_state;
/*
 * This Function sets up the needed structs
 */
int create_fft( struct fft_params *fft_p, struct fft_data *fft_d ) {
    // Not needed anymore as size gets transferd by alsa or read from file
    //fft_p->size = SAMPLERATE;
    fft_p->rank = RANK;
    fft_d->fft_in = (double*) fftw_malloc(sizeof(fftw_complex) * fft_p->size);
    if (fft_d->fft_in == 0)
        return E_MAL_FFT_IN;
    fft_d->fft_out = (double*) fftw_malloc(sizeof(fftw_complex) * fft_p->size);
    if (fft_d->fft_out == 0)
        return E_MAL_FFT_OUT;

    if (fft_p->plan == NULL)
        return E_ADD_FFT_PLAN;
    return OK;
}

int destroy_fft( struct fft_params *fft_p, struct fft_data *fft_d ) {
    fftw_destroy_plan(fft_p->plan);
    fftw_free(fft_d->fft_in);
    fftw_free(fft_d->fft_out);
    return OK;
}

#ifdef LIVE /* LIVE */
#ifdef PIPE
int fft_handler( int pipefd[2], void *shmem ) {
#endif /* PIPE */
#ifdef RINGBUFFER
int fft_handler( int *ringbuffer, int *wrote_size, void *shmem ) {
    unsigned int buffercount = ZERO;
    sigset_t mask, oldmask;
#endif /* RINGBUFFER */
    struct pid_collection *pids;
    int c, child_pid;
    double *in;
    void (*sig_handler_return) (int);
#endif /* LIVE */
    struct fft_params *fft_p;
    struct fft_data *fft_d;
    fftw_plan plan;
    int rc;
    int nr = 0;

//    FILE *gnuplot = popen("gnuplot -persistent", "w");
#ifdef LIVE /* LIVE */
    /*
     * This block is needed for the state handling
     * The sig handlers are needed for inter process 
     * communication and the state for setup and
     * during runtime
     */
    fft_pipe_state = ZERO;

    memset( &fft_act, 0, sizeof(fft_act) );

    fft_act.sa_sigaction = fft_sig_handler;
    fft_act.sa_flags = SA_SIGINFO;

    sigaction( SIGUSR1, &fft_act, 0 );
    sigaction( SIGCONT, &fft_act, 0 );
    /* 
     * The needed structs
     */
    pids = malloc( sizeof(struct pid_collection) );
#ifdef PIPE
    close( pipefd[1] );
#endif /* PIPE */
#endif /* LIVE */

    fft_p = malloc( sizeof(struct fft_params) );
    fft_d = malloc( sizeof(struct fft_data) );

#ifdef RINGBUFFER
    sigemptyset (&mask);
    sigaddset (&mask, SIGCONT);
#endif /* RINGBUFFER */
#ifdef LIVE /*LIVE */
    /*
     * Get pid of parrent to be ready to complet init step
     * The wait for the signals to read the shared memory
     * once done, send signal to alsa to run main loop
     */
    kill( getppid(), SIGUSR2 );

    if( !((fft_pipe_state & SHMEM_READ) >> SHIFT_S_R) ) {
#ifdef PRINT_SIGNAL
        printf("(fft) %i: waiting for SIGUSR1 to get size\n", pids->pid_fft_master);
#endif
        suspend( &fft_pipe_state, SHMEM_READ, SHIFT_S_R );
    }
    memcpy( pids, shmem, sizeof(struct pid_collection) );
    memcpy( &fft_p->size, &shmem[ sizeof(struct pid_collection) ], sizeof(fft_p->size) );
#ifdef PRINT_DEBUG
    printf("(fft) %i: read from shmem: %i, %i, %i, %i \n", pids->pid_fft_master, pids->pid_quarz, pids->pid_alsa, pids->pid_fft_master, fft_p->size);
#endif
#endif /* LIVE */

    /*
     * This is currently needed, as using the fft_d->fft_in array doesn't work
     * TODO: Fix error
     */
    rc = create_fft( fft_p, fft_d );
    in = (double*) fftw_malloc((sizeof(fftw_complex) * fft_p->size));

    fft_p->plan = fftw_plan_r2r_1d(fft_p->size, in, fft_d->fft_out, FFTW_DHT, FFTW_ESTIMATE);

    fft_pipe_state += RUNTIME;

#ifdef LIVE /* LIVE */
    /*
     * Send SIGCONT to alsa, to start RUNTIME
     */
#ifdef PRINT_SIGNAL
    printf("(fft) %i: Send SIGCONT to (alsa) %i\n", pids->pid_fft_master, pids->pid_alsa);
#endif
    kill( pids->pid_alsa, SIGCONT );

    if( !((fft_pipe_state & ALSA_DONE) >> SHIFT_A_D) ) {
#ifdef PRINT_SIGNAL
        printf("(fft) %i: waiting for SIGCONT to read input\n", pids->pid_fft_master);
#endif
        suspend( &fft_pipe_state, ALSA_DONE, SHIFT_A_D );
    }

    //If this isn't set, sighandling is fubar
    fft_pipe_state+=CONTINUE;

#endif /* LIVE */
    while( (fft_pipe_state & RUNTIME) >> SHIFT_R ) {
#ifdef LIVE /* LIVE */
#ifdef RINGBUFFER
        if( pc_count < MAX_PC_COUNT ) {
#endif /* RINGBUFFER */
            c = 0;
            c = fork();
            if( c == 0 ) {
                long *buffer;
                child_pid = getpid();
                buffer = malloc( fft_p->size * sizeof(long) );
#ifdef PIPE
                rc = read( pipefd[0], buffer, (fft_p->size * sizeof(long) ));
#endif /* PIPE */
#ifdef RINGBUFFER
                rc = read( ringbuffer[buffercount], buffer, *wrote_size);
               // strncpy( ringbuffer[buffercount], buffer, *wrote_size);
#endif /* RINGBUFFER */
#ifdef PRINT_DEBUG
                printf("(fft) %i: read %i bytes\n", child_pid, rc);
                printf("(fft) %i: wrote_size is %i\n", child_pid, *wrote_size);
                printf("(fft) %i: wrote_size is at %i\n", child_pid, wrote_size);
#endif /* PRINT_DEBUG */
//                if( rc < 1 )
//                    exit(0);
                for( int i = 0; i < *wrote_size; i++ ){
                    in[i] = (double) buffer[i];
                    //in[i] = (double) ringbuffer[buffercount + i];
                }
#ifdef PRINT_DEBUG
                printf("(fft) %i: done copying data\n", child_pid);
#endif
                // This is in the child. It souldn't repeat the while loop!
                free(buffer);
#ifdef RINGBUFFER
                pc_count--;
#endif /* RINGBUFFER */
                fft_pipe_state = fft_pipe_state - RUNTIME;
            } else {
#ifdef RINGBUFFER
                pc_count++;
                buffercount + *wrote_size;
                if( buffercount == SAMPLERATE )
                    buffercount = 0;
#endif /* RINGBUFFER */
                // This is in the parrent
                nr++;
            }
#ifdef RINGBUFFER
        } else {
            nr++;
            buffercount + fft_p->size;
            if( buffercount == SAMPLERATE )
                buffercount = 0;
        }
        sigprocmask( SIG_BLOCK, &mask, &oldmask );
        while( !((fft_pipe_state & CONTINUE) >> SHIFT_C) )
            sigsuspend( &oldmask );
        sigprocmask( SIG_UNBLOCK, &mask, NULL );
        fft_pipe_state -= CONTINUE;
#endif /* RINGBUFFER */
    }
    printf("exited while loop\n");
#endif /* LIVE */
#ifdef LIVE /* LIVE */
    if ( c == 0 ) {
    
    /*
     * The previous created plan gets executed here
     */
    fftw_execute(fft_p->plan);
#ifdef PPM
    run_ppm_from_fft( fft_d, (unsigned long) fft_p->size, nr, MAX_COUNT_FOR_ZERO_PADDING );
#endif    

#ifdef PRINT_DEBUG
    printf("(fft) %i: done with ppm creation\n", child_pid);
#endif

#endif /* LIVE */

    fftw_free(in);
    destroy_fft( fft_p, fft_d );
    free(fft_d);
    free(fft_p);

#ifdef LIVE
    free(pids);
    } else {
     //   wait();
    }
#endif /* LIVE */

    return OK;
}


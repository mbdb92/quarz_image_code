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
// For Signals
#include <signal.h>
#include <unistd.h>
#include "sighandler.h"

#include "fft.h"

struct sigaction fft_act;
/*
 * This Function sets up the needed structs
 */
int create_fft( struct fft_params *fft_p, struct fft_data *fft_d ) {
    fft_p->size = SIZE;
    fft_p->rank = RANK;
    fft_d->fft_in = (double*) fftw_malloc(sizeof(fftw_complex) * fft_p->size);
    if (fft_d->fft_in == 0)
        return E_MAL_FFT_IN;
    fft_d->fft_out = (double*) fftw_malloc(sizeof(fftw_complex) * fft_p->size);
    if (fft_d->fft_out == 0)
        return E_MAL_FFT_OUT;

    //fft_p->plan = fftw_plan_dft_1d(fft_p->size, fft_d->fft_in, fft_d->fft_out, FFTW_FORWARD, FFTW_ESTIMATE);
//    fft_p->plan = fftw_plan_r2r_1d(fft_p->size, fft_d->fft_in, fft_d->fft_out, FFTW_DHT, FFTW_ESTIMATE);
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


int fft_handler( int pipefd[2], void *shmem ) {
    struct pid_collection *pids;
    struct fft_params *fft_p;
    struct fft_data *fft_d;
    double *in;
    fftw_plan plan;
    int retval;
    void (*sig_handler_return) (int);

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
    fft_p = malloc( sizeof(struct fft_params) );
    fft_d = malloc( sizeof(struct fft_data) );
    //plan = fftw_plan_dft_1d(fft_p->size, fft_d->fft_in, fft_d->fft_out, FFTW_FORWARD, FFTW_ESTIMATE);

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
    /*
     * This is currently needed, as using the fft_d->fft_in array doesn't work
     * TODO: Fix error
     */
    in = (double*) fftw_malloc(sizeof(fftw_complex) * fft_p->size);

    fft_p->plan = fftw_plan_r2r_1d(fft_p->size, in, fft_d->fft_out, FFTW_DHT, FFTW_ESTIMATE);

    fft_pipe_state += RUNTIME;
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

    char read_char;
    bool run_read = true;
    int i = 0;
    while( run_read == true ){
        read( pipefd[0], &read_char, sizeof(read_char) );
        if( read_char != "\n" ) {
            in[i] = (double) read_char;
            i++;
            printf("%c\n", read_char);
        } else {
            run_read = false;
        }
    }
    //read( pipefd[0], in, sizeof( fft_p->size * sizeof(fftw_complex)) );
/*
    for( int i = 0; i < fft_p->size; i++ ) {
    //    in[i] = (double) buffer[i];
        read( pipefd[0], &in[i], sizeof(long) );
#ifdef PRINT_DEBUG
        printf("(fft) %i: read long from pipe: %li\n", pids->fft_master_pid, (long) in[i]);
#endif
    }
*/

//    free(buffer);
//    retval = fill_input_struct( fft_p, fft_d, in );
    
    /*
     * The previous created plan gets executed here
     */
    fftw_execute(fft_p->plan);

    for( int i = 0; i < fft_p->size; i++ ) {
        printf("%f\n", fft_d->fft_out[i]);
    }
    fftw_free(in);
    destroy_fft( fft_p, fft_d );
    free(fft_d);
    free(fft_p);
    free(pids);

    return OK;
}


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

void fft_sig_handler( int signum );
//int fft_pipe_state;

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


int fft_handler( int pipefd[2] ) {
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

    sig_handler_return = signal( SIGPIPE, fft_sig_handler );
    if( sig_handler_return == SIG_ERR )
        return 1;

    sig_handler_return = signal( SIGCONT, fft_sig_handler );
    if( sig_handler_return == SIG_ERR )
        return 1;

    sig_handler_return = signal( SIGURG, fft_sig_handler );
    if( sig_handler_return == SIG_ERR )
        return 1;

    /* 
     * The needed structs
     */
    pids = malloc( sizeof(struct pid_collection) );
    fft_p = malloc( sizeof(struct fft_params) );
    fft_d = malloc( sizeof(struct fft_data) );
    //plan = fftw_plan_dft_1d(fft_p->size, fft_d->fft_in, fft_d->fft_out, FFTW_FORWARD, FFTW_ESTIMATE);

    /*
     * Get pid of parrent to be ready to complet init step
     * The wait for the signals to read the pipe
     * once done, return signal to quarz that everything is done
     */
    pids->pid_quarz = getppid();
    pids->pid_fft_master = getpid();
    // Should the signal from quarz been raised allready just continue
    printf("(fft) test %i\n", fft_pipe_state);
    if( !((fft_pipe_state & ALSA_DONE) >> SHIFT_A_D) ){
#ifdef PRINT_DEBUG
        printf("(fft) %i: waiting for SIGCONT, alsa not done\n", pids->pid_fft_master);
#endif
        //pause();
        suspend( &fft_pipe_state, ALSA_DONE, SHIFT_A_D );
    }
    // Tell quarz your are ready
    kill( pids->pid_quarz, SIGCONT );
#ifdef PRINT_DEBUG
    printf("(fft) %i: send SIGCONT to %i\n", pids->pid_fft_master, pids->pid_quarz);
#endif
    if( !((fft_pipe_state & READ_PIPE) >> SHIFT_R_P) ) {
#ifdef PRINT_DEBUG
        printf("(fft) %i: waiting for SIGPIPE to read pids\n", pids->pid_fft_master);
#endif
        //pause();
        suspend( &fft_pipe_state, READ_PIPE, SHIFT_R_P );
    }
    // IF this doesn't happen, the state won't be reseted! This is a problem later on
    // It changes here, as it should change as soon as possible if another signal gets
    // raised in the meantime
    fft_pipe_state = fft_pipe_state - READ_PIPE;
    // TODO: Add error handling for incomplete read
    retval = read( pipefd[0], pids, sizeof(pids) );
    if( retval < 0 ) {
        return ERR;
    }
    if( retval != sizeof(pids) ) {
        printf("(fft) %i: short read!\n", pids->pid_fft_master);
    }
    printf("(fft) %i: pids %i, %i, %i\n", pids->pid_fft_master, pids->pid_quarz, pids->pid_alsa, pids->pid_fft_master);
    // IF this doesn't happen, the state won't be reseted! This is a problem later on
    fft_pipe_state = fft_pipe_state - READ_PIPE;
    // Tell quarz you are done
    kill( pids->pid_quarz, SIGCONT );
#ifdef PRINT_DEBUG
    printf("(fft) %i: send SIGCONT to %i\n", pids->pid_fft_master, pids->pid_quarz);
#endif

    fft_pipe_state = SIZE_NEEDED;
    kill( pids->pid_alsa, SIGURG );
#ifdef PRINT_DEBUG
    printf("(fft) %i: send SIGURG to %i\n", pids->pid_fft_master, pids->pid_alsa);
#endif
    if( !((fft_pipe_state & READ_PIPE) >> SHIFT_R_P) ) {
#ifdef PRINT_DEBUG
        printf("(fft) %i: waiting for SIGPIPE to get size\n", pids->pid_fft_master);
#endif
        //pause();
        suspend( &fft_pipe_state, READ_PIPE, SHIFT_R_P );
    }
    read( pipefd[0], &fft_p->size, sizeof(fft_p->size) );

    /*
     * This is currently needed, as using the fft_d->fft_in array doesn't work
     * TODO: Fix error
     */
    in = (double*) fftw_malloc(sizeof(fftw_complex) * fft_p->size);

    fft_p->plan = fftw_plan_r2r_1d(fft_p->size, in, fft_d->fft_out, FFTW_DHT, FFTW_ESTIMATE);

    fft_pipe_state = RUNTIME;
    kill( pids->pid_alsa, SIGCONT );
#ifdef PRINT_DEBUG
    printf("(fft) %i: send SIGCONT to %i\n", pids->pid_fft_master, pids->pid_alsa);
#endif
    /*
    for( int i = 0; i < fft_p->size; i++ ) {
        in[i] = (double) buffer[i];
    }
    */
    read( pipefd[0], in, sizeof( in ) );

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


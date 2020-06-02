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
// To access magick runtime
#ifdef CORE
#include "magick.h"
#endif
#ifdef WAND
#include "magick.h"
#endif
#ifdef PPM
#include "magick.h"
#endif

#include "fft.h"

struct sigaction fft_act;
/*
 * This Function sets up the needed structs
 */
int create_fft( struct fft_params *fft_p, struct fft_data *fft_d ) {
    // Not needed anymore as size gets transferd by alsa
    //fft_p->size = SIZE;
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
#ifdef WAND
    struct magick_params *magick_p;
#endif
    double *in;
    fftw_plan plan;
    int rc;
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
#ifdef WAND
    magick_p = malloc( sizeof(struct magick_params) );
#endif
    //plan = fftw_plan_dft_1d(fft_p->size, fft_d->fft_in, fft_d->fft_out, FFTW_FORWARD, FFTW_ESTIMATE);

    close( pipefd[1] );
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
    rc = create_fft( fft_p, fft_d );
    in = (double*) fftw_malloc(sizeof(fftw_complex) * fft_p->size);

#ifdef WAND
    rc = setup_drawing( magick_p );
#endif

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

    int c;
    int nr = 0;
    while( (fft_pipe_state & RUNTIME) >> SHIFT_R ) {
        c = 0;
        c = fork();
        if( c == 0 ) {
            long *buffer;
            buffer = malloc( fft_p->size * sizeof(long) );
    //rc = read( pipefd[0], in, fft_p->size );
            rc = read( pipefd[0], buffer, (fft_p->size * sizeof(long) ));
            if( rc < 1 )
                exit(0);
            for( int i = 0; i< fft_p->size; i++ ){
                in[i] = (double) buffer[i];
            }
            free(buffer);
            fft_pipe_state = fft_pipe_state - RUNTIME;
        } else {
            //fft_pipe_state = fft_pipe_state - RUNTIME;
            nr++;
        }
    }
    if ( c == 0 ) {
#ifdef PRINT_DEBUG
    printf("(fft) %i: read %i from pipe\n", pids->pid_fft_master, rc);
#endif
    
    /*
     * The previous created plan gets executed here
     */
    fftw_execute(fft_p->plan);
#ifdef WAND
    run_magick_from_fft( magick_p, fft_d, (unsigned long) fft_p->size );
#endif
#ifdef CORE
    run_magick_from_fft( fft_d, (unsigned long) fft_p->size, nr );
#endif
#ifdef PPM
    run_ppm_from_fft( fft_d, (unsigned long) fft_p->size, nr );
#endif    

    fftw_free(in);
#ifdef WAND
    destroy_drawing( magick_p );
    free(magick_p);
#endif
    destroy_fft( fft_p, fft_d );
    free(fft_d);
    free(fft_p);
    free(pids);
    } else {
     //   wait();
    }

    return OK;
}

int fft_run( ) {
    struct fft_params *fft_p;
    struct fft_data *fft_d;
    double *in, *out;
    fftw_plan plan;
    int rc;
    void (*sig_handler_return) (int);
    FILE *gnuplot = popen("gnuplot -persistent", "w");

    /*
     * This block is needed for the state handling
     * The sig handlers are needed for inter process 
     * communication and the state for setup and
     * during runtime
     */
    fft_p = malloc( sizeof(struct fft_params) );
    fft_d = malloc( sizeof(struct fft_data) );
    //plan = fftw_plan_dft_1d(fft_p->size, fft_d->fft_in, fft_d->fft_out, FFTW_FORWARD, FFTW_ESTIMATE);

    /*
     * Get pid of parrent to be ready to complet init step
     * The wait for the signals to read the shared memory
     * once done, send signal to alsa to run main loop
     */

    fft_p->size = 800;

    /*
     * This is currently needed, as using the fft_d->fft_in array doesn't work
     * TODO: Fix error
     */
    rc = create_fft( fft_p, fft_d );
    in = (double*) fftw_malloc((sizeof(fftw_complex) * fft_p->size));
    out = (double*) fftw_malloc((sizeof(fftw_complex) * fft_p->size));

#ifdef WAND
    rc = setup_drawing( magick_p );
#endif

//    fft_p->plan = fftw_plan_r2r_1d(fft_p->size, in, fft_d->fft_out, FFTW_DHT, FFTW_ESTIMATE);
    fft_p->plan = fftw_plan_r2r_1d(fft_p->size, in, fft_d->fft_out, FFTW_DHT, FFTW_ESTIMATE);

    fft_pipe_state += RUNTIME;
    /*
     * Send SIGCONT to alsa, to start RUNTIME
     */
    int fd;
    int c;
    int nr = 0;
    short *buffer;
    fd = open("output.raw", O_RDONLY);
        buffer = malloc( fft_p->size * sizeof(short) );
    while( (fft_pipe_state & RUNTIME) >> SHIFT_R ) {
        c = 0;
//rc = read( pipefd[0], in, fft_p->size );
        //rc = pread( fd, buffer, (fft_p->size * sizeof(int) ), ((fft_p->size * sizeof(int)) * nr) );
        //rc = pread( fd, buffer, (300*sizeof(short)) , (16* sizeof(short) * nr) );
        rc = read( fd, buffer, (fft_p->size * sizeof(short)) );
        //rc = pread( fd, &in[nr], (16*sizeof(short)) , (16* sizeof(short) * nr) );
        if( rc < 1 )
            exit(0);
        for( int i = 0; i < fft_p->size; i++ ){
            in[i] = (double) buffer[i] ;
           // printf("%i\n",  buffer[i]);
            printf("%f\n",  in[i] );
        }
        nr++;
        fftw_execute(fft_p->plan);
#ifdef WAND
        run_magick_from_fft( magick_p, fft_d, (unsigned long) fft_p->size );
#endif
#ifdef CORE
        run_magick_from_fft( fft_d, (unsigned long) fft_p->size, nr );
#endif
#ifdef PPM
        run_ppm_from_fft( fft_d, (unsigned long) fft_p->size, nr );
#endif    
        if(nr == 20){
            fft_pipe_state = fft_pipe_state - RUNTIME;
        }

    }
    free(buffer);
    close(fd);
    /*
     * The previous created plan gets executed here
     */

    fprintf(gnuplot, "plot '-'\n");

    for( int j = 0; j < fft_p->size; j++) {
     //   printf("%f\n", fft_d->fft_out[j] );
        fprintf(gnuplot, "%g %g\n", (double) j, fft_d->fft_out[j]);
    }
    fprintf(gnuplot, "e\n");
    fflush(gnuplot);

    fftw_free(in);
    fftw_free(out);
#ifdef WAND
    destroy_drawing( magick_p );
    free(magick_p);
#endif
    destroy_fft( fft_p, fft_d );
    free(fft_d);
    free(fft_p);

    printf("fft done\n");
    return OK;
}


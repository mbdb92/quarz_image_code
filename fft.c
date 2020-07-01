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
int fft_handler( int pipefd[2], void *shmem ) {
    struct pid_collection *pids;
    int c;
#ifdef WAND
    struct magick_params *magick_p;
#endif /* WAND */
    double *in;
    void (*sig_handler_return) (int);
#endif /* LIVE */
#ifdef RECORDED /* RECORDED */
int fft_run( char *filename ) {
    double *in, *out;
    struct wav_header *file_header;
    int size_input = INPUT_SIZE;
    int fd;
    short *buffer;
    char *headbuf;
    int read_size = 0;
#endif /* RECORDED */
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
#ifdef WAND
    magick_p = malloc( sizeof(struct magick_params) );
#endif /* WAND */
    close( pipefd[1] );
#endif /* LIVE */

    fft_p = malloc( sizeof(struct fft_params) );
    fft_d = malloc( sizeof(struct fft_data) );

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

#ifdef RECORDED /* RECORDED */
    file_header = malloc( sizeof(struct wav_header) );
    fd = open(filename, O_RDONLY);

    // This Block deals with the wav header, so it isn't in the input data
    // additionally it sets some values based on the header instead
    // of the predifined values in codes.h
    headbuf = malloc( WAV_HEADER_SIZE * sizeof(char) );
    rc = read( fd, headbuf, (WAV_HEADER_SIZE * sizeof(char)) );
    rc = (int) strncpy( &file_header->file_size, &headbuf[4], 4*sizeof(char) );
    rc = (short) strncpy( &file_header->format_type, &headbuf[20], 2*sizeof(char) );
    rc = (short) strncpy( &file_header->channel_number, &headbuf[22], 2*sizeof(char) );
    rc = (int) strncpy( &file_header->sample_rate, &headbuf[24], 4*sizeof(char) );
    rc = (short) strncpy( &file_header->bits_per_sample, &headbuf[34], 2*sizeof(char) );
    // Timeframe Divisor is the divisor for this: timeslot = 1s / timeslot_divisor
    // For 10ms it is 100 i.e.
    size_input = file_header->sample_rate / TIMEFRAME_DIVISOR;
    fft_p->size = file_header->sample_rate;
#ifdef PRINT_DEBUG
    printf("HEADER:\nfilesize: %i\nformat type: %i\nchannel number: %i\nsample rate: %i\nbits per sample: %i\n", file_header->file_size, file_header->format_type, file_header->channel_number, file_header->sample_rate, file_header->bits_per_sample );
#endif
    free(headbuf);
#endif /* RECORDED */

    /*
     * This is currently needed, as using the fft_d->fft_in array doesn't work
     * TODO: Fix error
     */
    rc = create_fft( fft_p, fft_d );
    in = (double*) fftw_malloc((sizeof(fftw_complex) * fft_p->size));
#ifdef RECORDED /* RECORDED */
    out = (double*) fftw_malloc((sizeof(fftw_complex) * fft_p->size));
#endif /* RECORDED */

#ifdef WAND
    rc = setup_drawing( magick_p );
#endif

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
#endif /* LIVE */
#ifdef RECORDED
    buffer = malloc( fft_p->size * sizeof(short) );
#endif /* RECORDED */
    while( (fft_pipe_state & RUNTIME) >> SHIFT_R ) {
#ifdef LIVE /* LIVE */
        c = 0;
        c = fork();
        if( c == 0 ) {
            long *buffer;
            buffer = malloc( fft_p->size * sizeof(long) );
            rc = read( pipefd[0], buffer, (fft_p->size * sizeof(long) ));
            if( rc < 1 )
                exit(0);
            for( int i = 0; i< fft_p->size; i++ ){
                in[i] = (double) buffer[i];
            }
            free(buffer);
            fft_pipe_state = fft_pipe_state - RUNTIME;
        } else {
            nr++;
        }
    }
#endif /* LIVE */
#ifdef RECORDED /* RECORDED */
        rc = read( fd, buffer, (size_input * sizeof(short)) );
        if( rc < 1 )
            exit(0);
        read_size = read_size + rc;
        for( int i = 0; i < size_input; i++ ){
            in[i] = (double) buffer[i] ;
#ifdef PRINT_DEBUGS
            printf("%f\n",  in[i] );
#endif
        }
        for( int i = size_input; i < fft_p->size; i++ ){
            in[i] = 0.0;
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
#ifdef PRINT_DEBUG
        printf("read_size = %i\n", read_size);
#endif
        if(read_size >= (file_header->file_size - WAV_HEADER_SIZE)){
            fft_pipe_state = fft_pipe_state - RUNTIME;
        }
    }
#endif /* RECORDED */
#ifdef LIVE /* LIVE */
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

#endif /* LIVE */

    fftw_free(in);
#ifdef WAND
    destroy_drawing( magick_p );
    free(magick_p);
#endif
    destroy_fft( fft_p, fft_d );
    free(fft_d);
    free(fft_p);

#ifdef LIVE
    free(pids);
    } else {
     //   wait();
    }
#endif /* LIVE */

#ifdef RECORDED /* RECORDED */
    free(buffer);
    close(fd);

/* GNUPLOT
    fprintf(gnuplot, "plot '-'\n");

    for( int j = 0; j < fft_p->size; j++) {
     //   printf("%f\n", fft_d->fft_out[j] );
        fprintf(gnuplot, "%g %g\n", (double) j, fft_d->fft_out[j]);
    }
    fprintf(gnuplot, "e\n");
    fflush(gnuplot);
*/

    fftw_free(out);
    free(file_header);
#endif /* RECORDED */

    return OK;
}


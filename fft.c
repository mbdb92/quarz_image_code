// Include needed types
#include "type.h"
// FFTW
#include <fftw3.h>
// sin()
#include <stdio.h>
#include <math.h>
//gnuplot
#include <stdlib.h>
//malloc_usabel_size
#include <malloc.h>
#include "fft.h"

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

/*
 * these for-loops generate a mixed cos function
 * Used for testing and controlled dev
 */
int fill_input_struct( struct fft_params *fft_p, struct fft_data *fft_d, double *in  ) {
    double x, val;
    float c;
    val = PI / 180;

    for( int i = 0; i < fft_p->size; i++ ){
        x = 1.0 * i;
        c = cos(x*val);
        fft_d->fft_in[i] = c;
        in[i] = c;
        if (c != fft_d->fft_in[i] )
            return E_W_COS;
    }
    for( int i = 0; i < fft_p->size; i++ ){
        x = 10.0 * i;
        fft_d->fft_in[i] = fft_d->fft_in[i] + cos(x*val);
        in[i] = in[i] + cos(x*val);
    }
    for( int i = 0; i < fft_p->size; i++ ){
        x = 10.0 * i;
        fft_d->fft_in[i] = fft_d->fft_in[i] + cos(x*val);
        in[i] = in[i] + cos(x*val);
    }
    for( int i = 0; i < fft_p->size; i++ ){
        x = 50.0 * i;
        fft_d->fft_in[i] = fft_d->fft_in[i] + cos(x*val);
        in[i] = in[i] + cos(x*val);
    }
    for( int i = 0; i < fft_p->size; i++ ){
        x = 49.0 * i;
        fft_d->fft_in[i] = fft_d->fft_in[i] + cos(x*val);
        in[i] = in[i] + cos(x*val);
    }
    for( int i = 0; i < fft_p->size; i++ ){
        x = 48.0 * i;
        fft_d->fft_in[i] = fft_d->fft_in[i] + cos(x*val);
        in[i] = in[i] + cos(x*val);
    }
    for( int i = 0; i < fft_p->size; i++ ){
        x = 10.0 * i;
        fft_d->fft_in[i] = fft_d->fft_in[i] + cos(x*val);
        in[i] = in[i] + cos(x*val);
    }
    for( int i = 0; i < fft_p->size; i++ ){
        x = 47.0 * i;
        fft_d->fft_in[i] = fft_d->fft_in[i] + cos(x*val);
        in[i] = in[i] + cos(x*val);
    }
    for( int i = 0; i < fft_p->size; i++ ){
        x = 46.0 * i;
        fft_d->fft_in[i] = fft_d->fft_in[i] + cos(x*val);
        in[i] = in[i] + cos(x*val);
    }
    for( int i = 0; i < fft_p->size; i++ ){
        x = 45.0 * i;
        fft_d->fft_in[i] = fft_d->fft_in[i] + cos(x*val);
        in[i] = in[i] + cos(x*val);
    }

    for( int i = 0; i < fft_p->size; i++ ){
        x = 5.0 * i;
        fft_d->fft_in[i] = fft_d->fft_in[i] + cos(x*val);
        in[i] = in[i] + cos(x*val);
#ifdef PRINT_DEBUG
//        fprintf(outfile, "%lf\n", *fft_d->fft_in[i]);
//        fprintf(outfile, "%lf\n", *in[i]);
//        printf("%f\n", in[i][0]);
//        printf("%f\n", fft_d->fft_in[i][0]);
#endif
    }

    return OK;
}

int run_fft( struct fft_params *fft_p, struct fft_data *fft_d, long *buffer ) {
    double *in;
    fftw_plan plan;
    int retval;
#ifdef PRINT_PLOT
    FILE *gnuplot = popen("gnuplot -persistent", "w");
    FILE *outfile = fopen("values.raw", "w+");
#endif

    /*
     * This is currently needed, as using the fft_d->fft_in array doesn't work
     * TODO: Fix error
     */
    in = (double*) fftw_malloc(sizeof(fftw_complex) * fft_p->size);
    //plan = fftw_plan_dft_1d(fft_p->size, fft_d->fft_in, fft_d->fft_out, FFTW_FORWARD, FFTW_ESTIMATE);
    fft_p->plan = fftw_plan_r2r_1d(fft_p->size, in, fft_d->fft_out, FFTW_DHT, FFTW_ESTIMATE);

    for( int i = 0; i < fft_p->size; i++ ) {
        in[i] = (double) buffer[i];
    }
//    retval = fill_input_struct( fft_p, fft_d, in );
    
    /*
     * The previous created plan gets executed here
     */
    fftw_execute(fft_p->plan);

#ifdef PRINT_PLOT
    /*
     * the gnu-plot code is for debugging,
     * not needed in final code
     */
    fprintf(gnuplot, "plot '-'\n");

    for( int j = 0; j < fft_p->size; j++){
        fprintf(gnuplot, "%g %g\n", (double) j, fft_d->fft_out[j]);
    }

    fprintf(gnuplot, "e\n");
    fflush(gnuplot);
#endif

    for( int i = 0; i < fft_p->size; i++ ) {
        printf("%f\n", fft_d->fft_out[i]);
    }
    fftw_free(in);

    return OK;
}

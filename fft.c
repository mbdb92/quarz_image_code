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

#define PI 3.14159

int create_fft( struct fft_params *fft_p, struct fft_data *fft_d ) {
    fft_p->fft_size = 360;
    fft_d->fft_in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fft_p->fft_size);
    if (fft_d->fft_in == 0)
        return E_MAL_FFT_IN;
    fft_d->fft_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fft_p->fft_size);
    if (fft_d->fft_out == 0)
        return E_MAL_FFT_OUT;

    fft_p->plan = fftw_plan_dft_1d(fft_p->fft_size, fft_d->fft_in, fft_d->fft_out, FFTW_FORWARD, FFTW_ESTIMATE);
    if (fft_d->fft_in == 0)
        return E_ADD_FFT_PLAN;
    return OK;
}

int destroy_fft( struct fft_params *fft_p, struct fft_data *fft_d ) {
    fftw_destroy_plan(fft_p->plan);
    free(fft_p->plan);
    fftw_free(fft_d->fft_in);
    fftw_free(fft_d->fft_out);
    return OK;
}



int run_fft( struct fft_params *fft_p, struct fft_data *fft_d ) {
//    fftw_complex *fft_out;
    fftw_plan plan;
    double x, val;
    float c;
    val = PI / 180;
 //   FILE *gnuplot = popen("gnuplot -persistent", "w");
 //   FILE *outfile = fopen("values.raw", "w+");

//    fft_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fft_p->fft_size);

    /*
    printf("%li\n", malloc_usable_size(*fft_d->fft_in));
    printf("%li\n", malloc_usable_size(*in));
    printf("%li\n", malloc_usable_size(*fft_d->fft_out));
    printf("%li\n", malloc_usable_size(*out));
*/
    for( int i = 0; i < fft_p->fft_size; i++ ){
        x = 10.0 * i;
        c = cos(x*val);
        fft_d->fft_in[i][0] = c;
        if (c != fft_d->fft_in[i][0] )
            return E_W_COS;
        //fprintf(outfile, "%f\n", fft_d->fft_in[i][0]);
    }
    /*
    for( int j = 0; j < fft_p->fft_size; j++){
        fprintf(outfile, "%lf\n", *fft_d->fft_in[j]);
        fprintf(outfile, "%lf\n", *in[j]);
    }
    */
/*
    for( int i = 0; i < fft_p->fft_size; i++ ){
        x = 10.0 * i;
        fft_d->fft_in[i][0] = fft_d->fft_in[i][0] + cos(x*val);
    }

    for( int i = 0; i < fft_p->fft_size; i++ ){
        x = 5.0 * i;
        fft_d->fft_in[i][0] = fft_d->fft_in[i][0] + cos(x*val);
    }
    */
    
    plan = fftw_plan_dft_1d(fft_p->fft_size, fft_d->fft_in, fft_d->fft_out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_print_plan(fft_p->plan);
    fftw_execute(plan);

/*
//    fprintf(outfile, "Nach Plan\n");
    for( int j = 0; j < fft_p->fft_size; j++){
 //       fprintf(outfile, "%lf\n", *fft_d->fft_out[j]);
 //       fprintf(outfile, "%lf\n", *fft_out[j]);
    }

    fprintf(gnuplot, "plot '-'\n");

    for( int j = 0; j < fft_p->fft_size; j++){
        fprintf(gnuplot, "%g %g\n", (double) j, *fft_d->fft_out[j]);
//        fprintf(gnuplot, "%g %g\n", (double) j, *fft_out[j]);
    }

    fprintf(gnuplot, "e\n");
    fflush(gnuplot);
*/
    fftw_destroy_plan(plan);

    return OK;
}

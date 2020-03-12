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


int create_fft( struct fft_params *fft_p, struct fft_data *fft_d ) {
    fft_p->size = SIZE;
    fft_p->rank = RANK;
    fft_d->fft_in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fft_p->size);
    if (fft_d->fft_in == 0)
        return E_MAL_FFT_IN;
    fft_d->fft_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fft_p->size);
    if (fft_d->fft_out == 0)
        return E_MAL_FFT_OUT;

    fft_p->plan = fftw_plan_dft_1d(fft_p->size, fft_d->fft_in, fft_d->fft_out, FFTW_FORWARD, FFTW_ESTIMATE);
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



int run_fft( struct fft_params *fft_p, struct fft_data *fft_d ) {
//    fftw_complex *fft_out;
    fftw_complex *in, *out;
    fftw_plan plan;
    int N = 100;
    double x, val;
    float c;
    val = PI / 180;
    FILE *gnuplot = popen("gnuplot -persistent", "w");
    FILE *outfile = fopen("values.raw", "w+");

//    fft_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fft_p->size);

    /*
    printf("%li\n", malloc_usable_size(*fft_d->fft_in));
    printf("%li\n", malloc_usable_size(*in));
    printf("%li\n", malloc_usable_size(*fft_d->fft_out));
    printf("%li\n", malloc_usable_size(*out));
*/

    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fft_p->size);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fft_p->size);
    //plan = fftw_plan_dft_1d(fft_p->size, fft_d->fft_in, fft_d->fft_out, FFTW_FORWARD, FFTW_ESTIMATE);
    plan = fftw_plan_r2r_1d(fft_p->size, fft_d->fft_in, fft_d->fft_out, FFTW_DHT, FFTW_ESTIMATE);

    for( int i = 0; i < fft_p->size; i++ ){
        x = 1.0 * i;
        c = cos(x*val);
        fft_d->fft_in[i][0] = c;
        in[i][0] = c;
        if (c != fft_d->fft_in[i][0] )
            return E_W_COS;
        //fprintf(outfile, "%f\n", fft_d->fft_in[i][0]);
    }
    /*
    for( int j = 0; j < fft_p->size; j++){
        fprintf(outfile, "%lf\n", *fft_d->fft_in[j]);
        fprintf(outfile, "%lf\n", *in[j]);
    }
    */
    for( int i = 0; i < fft_p->size; i++ ){
        x = 10.0 * i;
        fft_d->fft_in[i][0] = fft_d->fft_in[i][0] + cos(x*val);
        in[i][0] = in[i][0] + cos(x*val);
    }

    for( int i = 0; i < fft_p->size; i++ ){
        x = 5.0 * i;
        fft_d->fft_in[i][0] = fft_d->fft_in[i][0] + cos(x*val);
        in[i][0] = in[i][0] + cos(x*val);
        printf("%f\n", in[i][0]);
        printf("%f\n", fft_d->fft_in[i][0]);
    }
    
//    plan = fftw_plan_dft_1d(fft_p->size, fft_d->fft_in, fft_d->fft_out, FFTW_FORWARD, FFTW_ESTIMATE);
//    fftw_print_plan(fft_p->plan);
    fftw_execute(plan);

/*
//    fprintf(outfile, "Nach Plan\n");
    for( int j = 0; j < fft_p->size; j++){
 //       fprintf(outfile, "%lf\n", *fft_d->fft_out[j]);
 //       fprintf(outfile, "%lf\n", *fft_out[j]);
    }
*/

    fprintf(gnuplot, "plot '-'\n");

    for( int j = 0; j < fft_p->size; j++){
//        fprintf(gnuplot, "%g %g\n", (double) j, *fft_d->fft_out[j]);
//        fprintf(gnuplot, "%g %g\n", (double) j, *out[j]);
        fprintf(gnuplot, "%g %g\n", (double) j, in[j]);
//        fprintf(gnuplot, "%g %g\n", (double) j, fft_d->fft_in[j]);
//        fprintf(gnuplot, "%g %g\n", (double) j, *fft_out[j]);
    }

    fprintf(gnuplot, "e\n");
    fflush(gnuplot);

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    return OK;
}

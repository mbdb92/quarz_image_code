// Include needed types
#include "type.h"
// FFTW
#include <fftw3.h>
// sin()
#include <stdio.h>
#include <math.h>
//gnuplot
#include<stdlib.h>

#define PI 3.14159

int create_fft( struct fft_params *fft_p, struct fft_data *fft_d ) {
    fft_p->fft_size = 360;
    fft_d->fft_in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fft_p->fft_size);
    fft_d->fft_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fft_p->fft_size);

    fft_p->plan = fftw_plan_dft_1d(fft_p->fft_size, fft_d->fft_in, fft_d->fft_out, FFTW_FORWARD, FFTW_ESTIMATE);
    return 0;
}

int destroy_fft( struct fft_params *fft_p, struct fft_data *fft_d ) {
    fftw_destroy_plan(fft_p->plan);
    fftw_free(fft_d->fft_in);
    fftw_free(fft_d->fft_out);
    return 0;
}



int run_fft( struct fft_params *fft_p, struct fft_data *fft_d ) {
    double x, val;
    val = PI / 180;
    FILE *gnuplot = popen("gnuplot -persistent", "w");
    FILE *outfile = fopen("values.raw", "w+");


    for( int i = 0; i < fft_p->fft_size; i++ ){
        x = 1.0 * i;
        fft_d->fft_in[i][0] = cos(x*val);
    }
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

    fftw_execute(fft_p->plan);


    for( int j = 0; j < fft_p->fft_size; j++){
        fprintf(outfile, "%lf\n", *fft_d->fft_out[j]);
    }

    fprintf(gnuplot, "plot '-'\n");

    for( int j = 0; j < fft_p->fft_size; j++){
        fprintf(gnuplot, "%g %g\n", (double) j, *fft_d->fft_out[j]);
    }

    fprintf(gnuplot, "e\n");
    fflush(gnuplot);

    return 0;
}

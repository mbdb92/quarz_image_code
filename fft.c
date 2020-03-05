#include "type.h"
// FFTW
#include <fftw3.h>
// sin()
#include <stdio.h>
#include <math.h>
//gnuplot
#include<stdlib.h>

#define PI 3.14159

int create_fft( struct Addresses *adr) {
    adr->N = 360
    adr->in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * adr->N);
    adr->out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * adr->N);

    p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
}

int destroy_fft( struct Addresses *adr ) {
    fftw_destroy_plan(adr->p);
    fftw_free(adr->in);
    fftw_free(adr->out);
}

int run_fft( struct Addresses *adr ) {
    double x, val;
    val = PI / 180;
    FILE *gnuplot = popen("gnuplot -persistent", "w");
    FILE *outfile = fopen("values.raw", "w+");


    for( int i = 0; i < adr->N; i++ ){
        x = 100.0 * i;
        in[i][0] = cos(x*val);
    }
    for( int i = 0; i < adr->N; i++ ){
        x = 20.0 * i;
        in[i][0] = in[i][0] + cos(x*val);
    }
    for( int i = 0; i < adr->N; i++ ){
        x = 5.0 * i;
        in[i][0] = in[i][0] + cos(x*val);
    }

    fftw_execute(adr->p);

    for( int j = 0; j < adr->N; j++){
        fprintf(outfile, "%lf\n", *out[j]);
    }

    fprintf(gnuplot, "plot '-'\n");

    for( int j = 0; j < N; j++){
        printf("%lf, at %i\n", *out[j], j);
        fprintf(gnuplot, "%g %g\n", (double) j, *out[j]);
    }

    fprintf(gnuplot, "e\n");
    fflush(gnuplot);
    return 0;
}

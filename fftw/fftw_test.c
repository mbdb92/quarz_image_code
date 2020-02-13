// FFTW
#include <fftw3.h>
// sin()
#include <stdio.h>
#include <math.h>
//gnuplot
#include<stdlib.h>

#define PI 3.14159

int main() {
    fftw_complex *in, *out;
    fftw_plan p;
    int N = 360;
    double x, val;
    val = PI / 180;
    FILE *gnuplot = popen("gnuplot -persistent", "w");
    FILE *outfile = fopen("values.raw", "w+");

    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    for( int i = 0; i < N; i++ ){
        x = 100.0 * i;
        in[i][0] = cos(x*val);
        //printf("x= %lf, x*val= %lf, %lf\n", x, x*val, in[i][0]);
    }
    for( int i = 0; i < N; i++ ){
        x = 20.0 * i;
        in[i][0] = in[i][0] + cos(x*val);
        //printf("%lf\n", in[i][0]);
    }
    for( int i = 0; i < N; i++ ){
        x = 5.0 * i;
        in[i][0] = in[i][0] + cos(x*val);
        //printf("%lf\n", in[i][0]);
    }

    
    /*
    fprintf(gnuplot, "plot '-'\n");

    for( int j = 0; j < N; j++){
        fprintf(gnuplot, "%g %g\n", (double) j, *in[j]);
//        printf("%lf\n", out[j][0]);
    }

    fprintf(gnuplot, "e\n");
    fflush(gnuplot);
*/


   
    fftw_execute(p);
    printf("After FFT\n");

    for( int j = 0; j < N; j++){
        //printf("%fd\n", *out[j]);
        fprintf(outfile, "%lf\n", *out[j]);
    }

    fprintf(gnuplot, "plot '-'\n");

    for( int j = 0; j < N; j++){
//        if (*out[j] > 0.5){
            printf("%lf, at %i\n", *out[j], j);
            fprintf(gnuplot, "%g %g\n", (double) j, *out[j]);
//        printf("%lf\n", out[j][0]);
//        }
    }

    fprintf(gnuplot, "e\n");
    fflush(gnuplot);
    

    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);

    return 0;
}

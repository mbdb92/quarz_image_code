#ifndef DRAWING_H
#define DRAWING_H

int find_min_max( double *data, long *max, long *min, unsigned long size );

#ifdef PPM
int run_ppm_from_fft( struct fft_data *fft_d, unsigned long size, int nr, int total_size );
#endif

#endif //DRAWING_H

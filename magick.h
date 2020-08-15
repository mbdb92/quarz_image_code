#ifndef DRAWING_H
#define DRAWING_H

int find_min_max( double *data, long *max, long *min, unsigned long size );

#ifdef WAND
int setup_drawing( struct magick_params *adr );
int destroy_drawing( struct magick_params *adr );

int run_magick_from_fft( struct magick_params *magick, struct fft_data *data, unsigned long size );
#endif
#ifdef CORE
int setup_drawing( char *path, struct magick_core_params *params );
int destory_drawing( struct magick_core_params *params );

int run_magick_from_fft( struct fft_data *data, unsigned long size, int nr);
#endif
#ifdef PPM
int run_ppm_from_fft( struct fft_data *fft_d, unsigned long size, int nr, int total_size );
#endif

#endif //DRAWING_H

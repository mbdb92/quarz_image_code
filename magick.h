#ifndef DRAWING_H
#define DRAWING_H

#define X_CORD 0
#define X_SIZE 1
#define BYTE_SIZE 255
#define HEX_MAX 16
#define HEX_MIN 0

int find_min_max( double *data, long *max, long *min, unsigned long size );

#ifdef WAND
int setup_drawing( struct magick_params *adr );
int destroy_drawing( struct magick_params *adr );

int run_magick_from_fft( struct magick_params *magick, struct fft_data *data, unsigned long size );
#else
int setup_drawing( char *path, struct magick_core_params *params );
int destory_drawing( struct magick_core_params *params );

int run_magick_from_fft( struct fft_data *data, unsigned long size, int nr);
#endif

#endif //DRAWING_H

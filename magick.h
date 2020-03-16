#ifndef DRAWING_H
#define DRAWING_H

#define X_CORD 0
#define X_SIZE 1
#define BYTE_SIZE 255
#define HEX_MAX 16
#define HEX_MIN 0

int setup_drawing( struct magick_params *adr );

int destroy_drawing( struct magick_params *adr );

int find_min_max( double *data, double *max, double *min, unsigned long size );

int run_magick_from_fft( struct magick_params *magick, struct fft_data *data, unsigned long size );

#endif //DRAWING_H

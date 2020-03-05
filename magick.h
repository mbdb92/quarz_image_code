#ifndef DRAWING_H
#define DRAWING_H

int setup_drawing( struct magick_params *adr );

int destroy_drawing( struct magick_params *adr );

int run_magick_from_fft( struct magick_params *magick, struct fft_data *data, unsigned long size );

//int run_magick_test( struct magick_params *adr );

#endif //DRAWING_H

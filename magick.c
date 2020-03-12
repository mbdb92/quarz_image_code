//Default input-output
#include <stdio.h>
#include <stdlib.h>
// Data type definition
#include <stdbool.h>
#include <string.h>
#include <time.h>
// Local defined files
#include "type.h"
#include "magick.h"

#define X_CORD 1
#define BYTE_SIZE 255

int setup_drawing( struct magick_params *magick ) {
    MagickWandGenesis();

    magick->m_wand = NewMagickWand();
    magick->d_wand = NewDrawingWand();
    magick->c_wand = NewPixelWand();

    magick->color = malloc( strlen("#123456") );
    magick->max = 0;
    magick->min = 0;

    return 0;
}


int destroy_drawing( struct magick_params *magick ) {
    DestroyPixelWand(magick->c_wand);
    DestroyMagickWand(magick->m_wand);
    DestroyDrawingWand(magick->d_wand);

    MagickWandTerminus();
    free( magick->color );
    return 0;
}

int find_min_max( fftw_complex *data, double *max, double *min, unsigned long size ) {
    *max = 0;
    *min = 0;

    for( int i = 0; i < size; i++ ){
        if( (double) *data[i] > *max )
            *max = (double) *data[i];

        if( (double) *data[i] < *min )
            *min = (double) *data[i];
    }
    return OK;
}

int run_magick_from_fft( struct magick_params *magick, struct fft_data *fft_d, unsigned long size ) {
    bool retval;
    double divider_green;
    unsigned int color_green;

    PixelSetColor( magick->c_wand, "white" );
    MagickNewImage( magick->m_wand, size, 1, magick->c_wand );

    DrawSetStrokeOpacity( magick->d_wand, 1 );
    DrawSetStrokeWidth( magick->d_wand, 1 );

    find_min_max( fft_d->fft_out, &magick->max, &magick->min, size);
    if( magick->min < 0 )
        divider_green = (magick->max + fabs(magick->min)) / BYTE_SIZE;
    else
        divider_green = (magick->max - fabs(magick->min)) / BYTE_SIZE;
/*
    printf("Going color\n");
    printf( "%f\n", divider_green);
    printf( "%f, %f\n", magick->max, magick->min);
    */
    for( int i = 0; i < size; i++ ) {
        color_green = (unsigned int)( (fft_d->fft_out[i][0] + fabs(magick->min)) / divider_green );
        if( color_green < 16 && color_green >= 0 )
            sprintf( magick->color, "#000%x%s", color_green, "00" );
        else
            sprintf( magick->color, "#00%x%s", color_green, "00" );
//        printf( "%i\n", color_green);
        printf( "%s\n", magick->color );
        DrawSetFillColor(magick->d_wand, magick->c_wand);
        DrawPoint( magick->d_wand, X_CORD, (double) i );
    }

    MagickDrawImage(magick->m_wand, magick->d_wand);
    MagickWriteImage(magick->m_wand, "test.jpg");

    return 0;
}


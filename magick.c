//Default input-output
#include <stdio.h>
#include <stdlib.h>
// Data type definition
#include <stdbool.h>
#include <string.h>
#include <time.h>
// Default Library inclution
#include <MagickWand/MagickWand.h>
// Local defined files
#include "type.h"
#include "magick.h"

int setup_drawing( struct magick_params *magick ) {
    MagickWandGenesis();

    magick->m_wand = NewMagickWand();
    magick->d_wand = NewDrawingWand();
    magick->c_wand = NewPixelWand();

    magick->color = malloc( strlen("#123456") );

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

int run_magick_from_fft( struct magick_params *magick, struct fft_data *data, unsigned long size ) {
    bool retval;

    PixelSetColor( magick->c_wand, "white" );
    MagickNewImage( magick->m_wand, size, 1, magick->c_wand );

    DrawSetStrokeOpacity( magick->d_wand, 1 );
    DrawSetStrokeWidth( magick->d_wand, 1 );

    for( int i = 0; i < size; i++ ) {
        sprintf( magick->color, "#00%x", (unsigned int) data->fft_out[i] );
//        printf( "%s\n", magick->color );
        DrawSetFillColor(magick->d_wand, magick->c_wand);
        DrawPoint( magick->d_wand, (double) i, 1 );
    }

    MagickDrawImage(magick->m_wand, magick->d_wand);
    MagickWriteImage(magick->m_wand, "test.jpg");

    return 0;
}

/*
int run_magick_test( struct magick_params *magick ) {
    unsigned long size;
    bool retval;

    size = 10;

    PixelSetColor( magick->c_wand, "white" );
    MagickNewImage( magick->m_wand, size, size, magick->c_wand );

    DrawSetStrokeOpacity( magick->d_wand, 1 );
    DrawSetStrokeWidth( magick->d_wand, 1 );

    for( int i = 0; i < size; i++ ) {
        sprintf( magick->color, "#00%x", i );
        retval = PixelSetColor(magick->c_wand, magick->color);
        DrawSetFillColor(magick->d_wand, magick->c_wand);
        DrawPoint( magick->d_wand, (double) i, (double) i );
    }

    MagickDrawImage(magick->m_wand, magick->d_wand);
    MagickWriteImage(magick->m_wand, "test.jpg");

    return 0;
}
*/


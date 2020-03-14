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

/*
 * This function should set up the needed Wands and stuff
 */
int setup_drawing( struct magick_params *magick ) {
    MagickWandGenesis();

    magick->m_wand = NewMagickWand();
    magick->d_wand = NewDrawingWand();
    magick->c_wand = NewPixelWand();

    magick->color = malloc( 8 * sizeof(char) );
    if( magick->color == NULL )
        return E_MAL_MAGICK_COLOR;

    magick->max = 0;
    magick->min = 0;

    return OK;
}

/*
 * Cleans up after run
 */
int destroy_drawing( struct magick_params *magick ) {
    DestroyPixelWand(magick->c_wand);
    DestroyMagickWand(magick->m_wand);
    DestroyDrawingWand(magick->d_wand);

    MagickWandTerminus();
    free( magick->color );
    return 0;
}

/*
 * This function is needed to find the range for the values out of the fft
 *
 * This is needed, to calculate the correct value for the color. To make the rest of the code
 * more readable, it's moved here
 */
int find_min_max( double *data, double *max, double *min, unsigned long size ) {
    *max = 0;
    *min = 0;

    for( int i = 0; i < size; i++ ){
        if( data[i] > *max )
            *max = data[i];

        if( data[i] < *min )
            *min = data[i];
    }
    return OK;
}

/*
 * Main function, computing the fft.
 */
int run_magick_from_fft( struct magick_params *magick, struct fft_data *fft_d, unsigned long size ) {
    bool retval;
    double divider_green;
    unsigned int color_green;
//    *magick->color = "#FFFFFF";

    // Adds the backround, black so colors can be seen better
    PixelSetColor( magick->c_wand, "black" );
    MagickNewImage( magick->m_wand, X_CORD, size, magick->c_wand );

    // Set ups some basic wand valus
    // TODO: Maybe move this to setup function
    DrawSetStrokeOpacity( magick->d_wand, 1 );
    DrawSetStrokeWidth( magick->d_wand, 1 );

    // This blocks prepares the factor, by which each value must be divided
    // to fit into 256 bit hex code
    // Only green is currently used as a color value
    find_min_max( fft_d->fft_out, &magick->max, &magick->min, size);
    if( magick->min < 0 )
        divider_green = (magick->max + fabs(magick->min)) / BYTE_SIZE;
    else
        divider_green = (magick->max - fabs(magick->min)) / BYTE_SIZE;

    printf("Going color\n");
    printf( "%f\n", divider_green);
    printf( "%f, %f\n", magick->max, magick->min);


    // Iteartion over the output value
    for( int i = 0; i < size; i++ ) {
        // Converts the double value in fft_out into an usigned int
        // The shift by magick->min is needed to have no negative values
        // This is needed to get the hex value
        color_green = (unsigned int)( (fft_d->fft_out[i] + fabs(magick->min)) / divider_green );
//        printf("%f\n", fft_d->fft_in[i]);
        // Sets the correct color string
        // Padding if the hex value would be single digit
//        printf( "%i\n", color_green);
        if( color_green < 16 && color_green >= 0 ) {
            sprintf( magick->color, "#000%x%s", color_green, "00" );
        } else {
            sprintf( magick->color, "#00%x%s", color_green, "00" );
        }
//        printf( "%s\n", magick->color );
        // This realy sets the color!
        retval = PixelSetColor(magick->c_wand, magick->color);
        if( retval != true ){
            return E_SET_COLOR;
        }
//        printf("%d\n", retval);
        // This should draw the correct color
        DrawSetFillColor(magick->d_wand, magick->c_wand);
        DrawPoint( magick->d_wand, X_CORD, (double) i );
    }

    // Writes the file to disc
    MagickDrawImage(magick->m_wand, magick->d_wand);
    MagickWriteImage(magick->m_wand, "test.jpg");

    return 0;
}

//Default input-output
#include <stdio.h>
#include <stdlib.h>
// Data type definition
#include <stdbool.h>
#include <string.h>
#include <time.h>
// For debugging
#include <assert.h>
// for fabs()
#include <math.h>
// Local defined files
#include "type.h"
#include "magick.h"

/*
 * This function should set up the needed Wands and stuff
 */
int setup_drawing( struct magick_params *magick ) {

    MagickWandGenesis();

    magick->m_wand = NewMagickWand();
    magick->d_wand = NewDrawingWand();
    magick->c_wand = NewPixelWand();

/*
 * This code block is legacy code. Needed, while color was holded,
 * in a seperated allocated memory part
 */

/*
    magick->color = malloc( 8 * sizeof(char) );
    if( magick->color == NULL )
        return E_MAL_MAGICK_COLOR;

    assert( magick->color != NULL );
*/
    magick->max = 0;
    magick->min = 0;
    
    // Set ups some basic wand valus
    DrawSetFillOpacity( magick->d_wand, 1 );
    DrawSetStrokeWidth( magick->d_wand, 1 );

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
    //free( magick->color );
    return 0;
}

/*
 * This function is needed to find the range for the values out of the fft
 *
 * This is needed, to calculate the correct value for the color. To make the rest of the code
 * more readable, it's moved here
 */
int find_min_max( double *data, long *max, long *min, unsigned long size ) {
    *max = 0;
    *min = 0;

    for( int i = 0; i < size; i++ ){
        if( (long) fabs(data[i]) > *max )
            *max = (long) fabs(data[i]);

        if( (long) fabs(data[i]) < *min ) 
            *min = (long) fabs(data[i]);
    }
    return OK;
}

/*
 * Main function, computing the fft.
 */
int run_magick_from_fft( struct magick_params *magick, struct fft_data *fft_d, unsigned long size ) {
    bool retval;
    long divider_green;
    unsigned int color_green;
    int len;

    // Adds the backround, black so colors can be seen better
    // This also sets the size of the image with the same size 
    // as the fft_in struct should have
    // X_SIZE is 1, as this is only one timeslot
    PixelSetColor( magick->c_wand, "black" );
    MagickNewImage( magick->m_wand, X_SIZE, size, magick->c_wand );

    // This blocks prepares the factor, by which each value must be divided
    // to fit into 256 bit hex code
    // Only green is currently used as a color value
    // TODO: Check if this works by high dynamic input, or if this 
    // has to be changed to have a longer stored maximun and minimum value
    find_min_max( fft_d->fft_out, &magick->max, &magick->min, size);
    if( magick->min < 0 )
        divider_green = (magick->max - magick->min) / BYTE_SIZE;
#ifdef PRINT_DEBUG
    printf("Going color\n");
    printf( "%li\n", divider_green);
    printf( "%li, %li\n", magick->max, magick->min);
#endif

    // Iteartion over the output value struct fft_out
    // TODO: remove hardcoded fft_out
    for( int i = 0; i < size; i++ ) {
        // Converts the double value in fft_out into an usigned int
        // The shift by magick->min is needed to have no negative values
        // This is needed to get the hex value
        color_green = (int)( ((long) fabs(fft_d->fft_out[i]) + magick->min) / divider_green );

#ifdef PRINT_DEBUG
        printf("%f\n", fft_d->fft_out[i]);
        printf("%i\n", color_green);
#endif  

        // Sets the correct color string
        // Padding if the hex value would be single digit
/*
        if( color_green < HEX_MAX && color_green >= HEX_MIN ) {
            sprintf( magick->color, "#000%x%s", color_green, "00" );
        } else {
            sprintf( magick->color, "#00%x%s", color_green, "00" );
        }
*/
        sprintf( magick->color, "%x", color_green );
        len = strlen( magick->color );
#ifdef PRINT_DEBUG
        printf("%i\n", len);
#endif  
        switch( len ) {
            case 4:     
                sprintf( magick->color, "#00%x", color_green );
                break;
            case 3:     
                sprintf( magick->color, "#000%x", color_green );
                break;
            case 2:     
                sprintf( magick->color, "#0000%x", color_green );
                break;
            case 1:     
                sprintf( magick->color, "#00000%x", color_green );
                break;
            default:     
                sprintf( magick->color, "#000000" );
                break;
        }
#ifdef PRINT_DEBUG
        printf("%s\n", magick->color);
#endif  
        // This realy sets the color!
        retval = PixelSetColor(magick->c_wand, magick->color);
        if( retval != true ){
            return E_SET_COLOR;
        }
#ifdef PRINT_DEBUG
        printf("Set color done\n");
#endif
        // This should draw the correct color
        // Both function have a void type,
        // therefor no return value is checked
        DrawSetFillColor(magick->d_wand, magick->c_wand);
        DrawPoint( magick->d_wand, (double) X_CORD, (double) i );
#ifdef PRINT_DEBUG
        printf("Point drawn\n");
#endif
    }
#ifdef PRINT_DEBUG
        printf("Points drawn!\n");
#endif

    // Writes the file to disc
    // TODO: Change Output filename to a given parameter to function
    MagickDrawImage(magick->m_wand, magick->d_wand);
    MagickWriteImage(magick->m_wand, "test.jpg");

    return OK;
}


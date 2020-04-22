#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// Data type definition
#include <stdbool.h>
#include <string.h>
// Time to be able to name the file accordingly
#include <time.h>
// For debugging
#include <assert.h>
// for fabs()
#include <math.h>
// Local defined files
#include "codes.h"
#include "type.h"
#include "magick.h"
#ifdef WAND
#include <MagickWand/MagickWand.h>
#else
#include <MagickCore/MagickCore.h>
#endif

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

#ifdef WAND
/*
 * This function should set up the needed Wands and stuff
 */
int setup_drawing( struct magick_params *magick ) {

    MagickWandGenesis();

    magick->m_wand = NewMagickWand();
    magick->d_wand = NewDrawingWand();
    magick->c_wand = NewPixelWand();

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
    return 0;
}

/*
 * Main function, computing the fft.
 */
int run_magick_from_fft( struct magick_params *magick, struct fft_data *fft_d, unsigned long size ) {
    struct magick_params *local_magick;
    struct timespec now;
    char filename[100];
    bool rc;
    long divider_green, ms;
    unsigned int color_green;
    int len;

    local_magick = malloc( sizeof(struct magick_params) );

    /*
     * with this the wands can be reused for each image,
     * so there is no need to create new ones.
     * Maybe this will be changed to Clone and
     * removing them at the end...
     * TODO Check is this or clone is needed!
     */
    local_magick->c_wand = ClonePixelWand( magick->c_wand );
    local_magick->d_wand = CloneDrawingWand( magick->d_wand );
    local_magick->m_wand = CloneMagickWand( magick->m_wand );

    // Adds the backround, black so colors can be seen better
    // This also sets the size of the image with the same size 
    // as the fft_in struct should have
    // X_SIZE is 1, as this is only one timeslot
    PixelSetColor( local_magick->c_wand, "black" );
    MagickNewImage( local_magick->m_wand, X_SIZE, size, local_magick->c_wand );

    // This blocks prepares the factor, by which each value must be divided
    // to fit into 256 bit hex code
    // Only green is currently used as a color value
    // TODO: Check if this works by high dynamic input, or if this 
    // has to be changed to have a longer stored maximun and minimum value
    find_min_max( fft_d->fft_out, &local_magick->max, &local_magick->min, size);
    if( local_magick->min < 0 )
        divider_green = (local_magick->max - local_magick->min) / BYTE_SIZE;
#ifdef PRINT_DEBUG
    printf("Going color\n");
    printf( "%li\n", divider_green);
    printf( "%li, %li\n", local_magick->max, local_magick->min);
#endif

    // Iteartion over the output value struct fft_out
    // TODO: remove hardcoded fft_out
    for( int i = 0; i < size; i++ ) {
        // Converts the double value in fft_out into an usigned int
        // The shift by magick->min is needed to have no negative values
        // This is needed to get the hex value
        color_green = (int)( ((long) fabs(fft_d->fft_out[i]) + local_magick->min) / divider_green );

#ifdef PRINT_DEBUG
        printf("%f\n", fft_d->fft_out[i]);
        printf("%i\n", color_green);
#endif  

        // Sets the correct color string
        // Padding if the hex value would be single digit
        sprintf( local_magick->color, "%x", color_green );
        len = strlen( local_magick->color );
#ifdef PRINT_DEBUG
        printf("%i\n", len);
#endif  
        switch( len ) {
            case 4:     
                sprintf( local_magick->color, "#00%x", color_green );
                break;
            case 3:     
                sprintf( local_magick->color, "#000%x", color_green );
                break;
            case 2:     
                sprintf( local_magick->color, "#0000%x", color_green );
                break;
            case 1:     
                sprintf( local_magick->color, "#00000%x", color_green );
                break;
            default:     
                sprintf( local_magick->color, "#000000" );
                break;
        }
#ifdef PRINT_DEBUG
        printf("%s\n", local_magick->color);
#endif  
        // This realy sets the color!
        rc = PixelSetColor(local_magick->c_wand, local_magick->color);
        if( rc != true ){
            return E_SET_COLOR;
        }
#ifdef PRINT_DEBUG
        printf("Set color done\n");
#endif
        // This should draw the correct color
        // Both function have a void type,
        // therefor no return value is checked
        DrawSetFillColor(local_magick->d_wand, local_magick->c_wand);
        DrawPoint( local_magick->d_wand, (double) X_CORD, (double) i );
#ifdef PRINT_DEBUG
        printf("Point drawn\n");
#endif
    }
#ifdef PRINT_DEBUG
        printf("Points drawn!\n");
#endif

    // Writes the file to disc
    // WARNING: CLOCK_REALTIME_COARSE is Linux specific!
    // According to man 3 cloack_gettime, it is only implemented in the Linux Kernel
    // This makes the code Linux specific. changeing it to CLOCK_REALTIME should
    // undo this. But the code was developed under Linux, so no idea if it compiles
    // on a Windows or Mac
    // I chose _COARSE as according to the manpage it's faster but less precise
    // I am more concerned for the runtime of a call than abióut the precission
    clock_gettime( CLOCK_REALTIME_COARSE, &now );
    ms = floor( now.tv_nsec / 1.0e3 );
    sprintf( filename, "%i%03ld.jpg", (int) now.tv_sec, ms );
    
    MagickDrawImage(local_magick->m_wand, local_magick->d_wand);
    MagickWriteImage(local_magick->m_wand, filename);

    DestroyMagickWand( local_magick->m_wand );
    DestroyDrawingWand( local_magick->d_wand );
    DestroyPixelWand( local_magick->c_wand );
    free( local_magick );
    return OK;
}

#else //If not definde Wand

int setup_drawing( char *path, struct magick_core_params *params ) {
    MagickCoreGenesis( path, MagickTrue );
    params->exception = AcquireExceptionInfo();
    params->image_info = AcquireImageInfo();
    GetImageInfo( params->image_info );

    (void) strcpy( params->image_info->filename, path );

    return OK;
}

int destroy_drawing( struct magick_core_params *params ) {
    params->image_info = DestroyImageInfo( params->image_info );
    params->image = DestroyImage( params->image );
    params->exception = DestroyExceptionInfo( params->exception );

    MagickCoreTerminus();

    return OK;
}

int run_magick_from_fft( struct fft_data *fft_d, unsigned long size ) {
    struct magick_core_params *local_magick;
    struct timespec now;
    long divider, ms;
    unsigned int color;
    int rc, len;

    local_magick = malloc( sizeof(struct magick_core_params) );
    local_magick->pixels = malloc( size * sizeof(int) );

    // >Setting the time as filename
    clock_gettime( CLOCK_REALTIME_COARSE, &now );
    ms = floor( now.tv_nsec / 1.0e3 );
    sprintf( local_magick->path, "%i%03ld.jpg", (int) now.tv_sec, ms );

    rc = setup_drawing( &local_magick->path, local_magick );

    // Um die double werte, die hier eh schon convertiert zu long sid, aus fft
    // suaber auf den Farbraum runterzurechnen wird hier ein Faktor genommen, 
    // der den Max. Hub beschreibt
    find_min_max( fft_d->fft_out, &local_magick->max, &local_magick->min, size );
    if( local_magick->min < 0 )
        divider = (local_magick->max - local_magick->min) /BYTE_SIZE;
    
    for( int i = 0; i < size; i++ ) {
        color = (int)( ((long) fabs(fft_d->fft_out[i]) + local_magick->min) / divider );

        /*
         * This part transforms the input into an char string
         * This char string is the input for the pixels
         * TODO couldn't be the type int be used?
         */
        sprintf( local_magick->color, "%x", color );
        len = strlen( local_magick -> color );
        switch( len ) {
            case 6:
                sprintf( &local_magick->pixels[i], "#%x", color );
                break;
            case 5:
                sprintf( &local_magick->pixels[i], "#0%x", color );
                break;
            case 4:
                sprintf( &local_magick->pixels[i], "#00%x", color );
                break;
            case 3:
                sprintf( &local_magick->pixels[i], "#000%x", color );
                break;
            case 2:
                sprintf( &local_magick->pixels[i], "#0000%x", color );
                break;
            case 1:
                sprintf( &local_magick->pixels[i], "#00000%x", color );
                break;
            default:
                sprintf( &local_magick->pixels[i], "#000000" );
                break;
        }
    }

    local_magick->image = ConstituteImage(X_SIZE, size, "RGB", CharPixel, local_magick->pixels, local_magick->exception );

    WriteImage( local_magick->image_info, local_magick->image, local_magick->exception );

    destroy_drawing( local_magick );
    return OK;
} 

#endif

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
// for fabs(), log10()
#include <math.h>
// Local defined files
#include "codes.h"
#include "type.h"
#include "magick.h"
#ifdef WAND
#include <MagickWand/MagickWand.h>
#endif
#ifdef CORE
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
          if( i > (size / 2) )
//        if( i > (size - 20) )
//        if( i > 2400 )
            data[i] = 0.0;
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

#endif
#ifdef CORE

int setup_drawing( char *path, struct magick_core_params *params ) {
    // Filename Max gets defined by stdio.h
    char dir[FILENAME_MAX];

    getcwd( dir, FILENAME_MAX );
    MagickCoreGenesis( dir, MagickTrue );
    params->exception = AcquireExceptionInfo();
    params->image_info = AcquireImageInfo();
    params->image_info = CloneImageInfo( (ImageInfo *) NULL );
//    GetImageInfo( params->image_info );

    (void) strcpy( params->image_info->filename, path );

#ifdef PRINT_DEBUG
    printf("(magick): Dir is %s, Filename is %s\n", dir, params->image_info->filename );
#endif
    return OK;
}

int destroy_drawing( struct magick_core_params *params ) {
    params->image_info = DestroyImageInfo( params->image_info );
    params->image = DestroyImage( params->image );
    params->exception = DestroyExceptionInfo( params->exception );

    MagickCoreTerminus();

    return OK;
}

int run_magick_from_fft( struct fft_data *fft_d, unsigned long size, int nr ) {
    struct magick_core_params *local_magick;
    struct timespec now;
    long divider, ms;
    unsigned int color;
    int rc, len;
    char *pixels;

    local_magick = malloc( sizeof(struct magick_core_params) );
    local_magick->pixels = malloc( size * sizeof(char) );
    pixels = (char*) malloc( size * sizeof(char) );

    // >Setting the time as filename
    clock_gettime( CLOCK_REALTIME_COARSE, &now );
    ms = floor( now.tv_nsec / 1.0e3 );
    //sprintf( local_magick->path, "%i%03ld.jpg", (int) now.tv_sec, ms );
    sprintf( local_magick->path, "%i.jpg", nr );

    rc = setup_drawing( &local_magick->path[0], local_magick );

    // Um die double werte, die hier eh schon convertiert zu long sid, aus fft
    // suaber auf den Farbraum runterzurechnen wird hier ein Faktor genommen, 
    // der den Max. Hub beschreibt
    find_min_max( fft_d->fft_out, &local_magick->max, &local_magick->min, size );
    printf("%li, %li\n", local_magick->min, local_magick->max);
    divider = (local_magick->max - local_magick->min) /BYTE_SIZE;

    // To create ppm file
    FILE *fd = fopen("data_p3.ppm", "w+");
    fprintf( fd, "P3\n%i %i\n%i\n", X_SIZE, (int) size, BYTE_SIZE );
    
    for( int i = 0; i < size; i++ ) {
        color = (int)( ((long) fabs(fft_d->fft_out[i]) - local_magick->min) / divider );

        /*
         * This part transforms the input into an char string
         * This char string is the input for the pixels
         * TODO couldn't be the type int be used?
         */
        sprintf( local_magick->color, "%x", color );
        len = strlen( local_magick->color );
        printf("%s\n", local_magick->color );
        switch( len ) {
            case 6:
                sprintf( &local_magick->pixels[i], "%x", color );
                break;
            case 5:
                sprintf( &local_magick->pixels[i], "0%x", color );
                break;
            case 4:
                sprintf( &local_magick->pixels[i], "00%x", color );
                break;
            case 3:
                sprintf( &local_magick->pixels[i], "000%x", color );
                break;
            case 2:
                sprintf( &local_magick->pixels[i], "0000%x", color );
                fprintf( fd, "0 %i 0\n", color );
                break;
            case 1:
                sprintf( &local_magick->pixels[i], "00000%x", color );
                fprintf( fd, "0 %i 0\n", color );
                break;
            default:
                sprintf( &local_magick->pixels[i], "000000" );
                fprintf( fd, "0 0 0 \n" );
                break;
        }
        printf("%s\n", &local_magick->pixels[i] );
        printf("%i\n", color );
    }

//    local_magick->image = ConstituteImage(X_SIZE, size, "RGB", CharPixel, local_magick->pixels, local_magick->exception );
    local_magick->image = BlobToImage(local_magick->image_info, local_magick->pixels, size, local_magick->exception );
    CatchException(local_magick->exception);
#ifdef PRINT_DEBUG
    printf("(magick): consituting image done\n");
#endif

    printf("Done building image\n");
    // Test ppm needs to be closed
    fclose(fd);
    printf("Done writing data\n");

    //WriteImage( local_magick->image_info, local_magick->image, local_magick->exception );
    WriteImages( local_magick->image_info, local_magick->image, local_magick->path, local_magick->exception );
    printf("Done writing image\n");

    destroy_drawing( local_magick );
    free(local_magick->pixels);
    free(local_magick);
    return OK;
} 

#endif
#ifdef PPM

int run_ppm_from_fft( struct fft_data *fft_d, unsigned long size, int nr, int total_size ) {
    struct ppm_params *ppm;
    int lead_count, base_current, lead_loop;

    ppm = malloc( sizeof(struct ppm_params) );

    /*
     * This takes care of the 0-padding for lower numbers
     * I use this to be able to use the *-opperator
     * in a linux shell. Else I would get 1 10 2 ...
     */
    // Takes the x in 10^x as how many 0 need to be padded
    if( total_size != 0 ){
        lead_count = floor( log10( (double) total_size ));
        base_current = floor( log10( (double) nr ));
        lead_loop = lead_count - base_current;

        char lead[lead_loop];
        
        for( int i = 0; i < lead_loop; i++) {
            lead[i] = '0';
        }
        // The last part needs to be a terminal sign
        // Else %s in the print wont stop adding chars
        lead[lead_loop] = '\0';

        // Generates the file name
        sprintf( ppm->path, "%s%i.ppm", lead, nr );
    }
    else {

        // Generates the file name
        sprintf( ppm->path, "%i.ppm", nr );

    }

    // Um die double werte, die hier eh schon convertiert zu long sid, aus fft
    // suaber auf den Farbraum runterzurechnen wird hier ein Faktor genommen, 
    // der den Max. Hub beschreibt
    find_min_max( fft_d->fft_out, &ppm->max, &ppm->min, size );
#ifdef PRINT_DEBUG
    printf("%li, %li\n", ppm->min, ppm->max);
#endif
    ppm->divider = (ppm->max - ppm->min) / BYTE_SIZE;
    if( ppm->divider == 0 )
        ppm->divider = 1;

    // To create ppm file
    ppm->fd = fopen(ppm->path, "w+");
    fprintf( ppm->fd, "P3\n%i %i\n%i\n", X_SIZE, (int) (size / 2), BYTE_SIZE );
    
    for( int i = 0; (i < (size / 2) || i < PPM_MAX); i++ ) {
        ppm->color = (int)( ((long) fabs(fft_d->fft_out[i]) - ppm->min) / ppm->divider );
        fprintf( ppm->fd, "0 %i 0\n", ppm->color );
    }

    fclose(ppm->fd);

    free(ppm);
    return OK;
} 

#endif

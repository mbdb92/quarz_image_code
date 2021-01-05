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

//extern int quarz_state;
//extern int alsa_state;
//extern int fft_master_state;

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

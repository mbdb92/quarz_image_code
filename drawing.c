#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <MagickWand/MagickWand.h>
#include "type.h"
#include "drawing.h"

int setup_drawing( struct Addresses *adr ) {
    MagickWandGenesis();

    adr->m_wand = NewMagickWand();
    adr->d_wand = NewDrawingWand();
    adr->c_wand = NewPixelWand();

    adr->color = malloc( strlen("#123456") );

    return 0;
}


int destroy_drawing( struct Addresses *adr ) {
    DestroyPixelWand(adr->c_wand);
    DestroyMagickWand(adr->m_wand);
    DestroyDrawingWand(adr->d_wand);

    MagickWandTerminus();
    free( adr->color );
    return 0;
}

int run_magick( struct Addresses *adr ) {
    unsigned long size;
    bool retval;

    size = 10;

    PixelSetColor( adr->c_wand, "white" );
    MagickNewImage( adr->m_wand, size, size, adr->c_wand );

    DrawSetStrokeOpacity( adr->d_wand, 1 );
    DrawSetStrokeWidth( adr->d_wand, 1 );

    for( int i = 0; i < size; i++ ) {
        sprintf( adr->color, "#00%x", i );
        retval = PixelSetColor(adr->c_wand, adr->color);
        DrawSetFillColor(adr->d_wand, adr->c_wand);
        DrawPoint( adr->d_wand, (double) i, (double) i );
    }

    MagickDrawImage(adr->m_wand, adr->d_wand);
    MagickWriteImage(adr->m_wand, "test.jpg");

    return 0;
}


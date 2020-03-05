#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#include <MagickCore/MagickCore.h>
#include <MagickWand/MagickWand.h>

int main() {
    
    MagickWand *m_wand;
    DrawingWand *d_wand;
    PixelWand *c_wand;
    unsigned long size;
    char *color;
    bool retval;

    color = malloc( strlen ("#123456") );

    size = 100;

    MagickWandGenesis();

    m_wand = NewMagickWand();
    d_wand = NewDrawingWand();
    c_wand = NewPixelWand();

    PixelSetColor( c_wand, "white" );
    MagickNewImage( m_wand, size, size, c_wand );

    DrawSetStrokeOpacity( d_wand, 1 );
    DrawSetStrokeWidth( d_wand, 1 );

    for( int i = 0; i < (size*size); i++ ) {
        sprintf( color, "#00%x", i );
        printf("%s\n", color);
        retval = PixelSetColor(c_wand, color);
        printf("%d\n", retval);
        DrawSetFillColor(d_wand, c_wand);
        DrawPoint( d_wand, (double) i, (double) i );
        //DrawColor( d_wand, (double) i, (double) i, PointMethod );
    }

    MagickDrawImage(m_wand, d_wand);
    MagickWriteImage(m_wand, "test.jpg");

    c_wand = DestroyPixelWand(c_wand);
    m_wand = DestroyMagickWand(m_wand);
    d_wand = DestroyDrawingWand(d_wand);

    MagickWandTerminus();
    free(color);
    return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#include <MagickCore/MagickCore.h>
#include <MagickWand/MagickWand.h>

int main() {
    /*
    ExceptionInfo *exception;
    Image *image;
    ImageInfo *image_info;
    int pixels[25];
    MagickBooleanType returnval;

    for( int i = 0; i < 25; i++ ){
        pixels[i] = 10*i;
    }
    image_info = AcquireMagickInfo( , "testimage"

    MagickCoreGenesis( *argv, MagickTrue);
    image = ConstituteImage( 5, 5, "I", IntegerPixel, &pixels, exception);

    returnval = WriteImage( , image, exception)
    */
    
    MagickWand *m_wand;
    DrawingWand *d_wand;
    PixelWand *c_wand;
    unsigned long size;
    char *color;

    color = malloc( strlen ("#123456") );

    size = 100;
    /*
    int i;
    double points[size];
    for( i = 0; i < size; i++ ) {
        points[i] = 255/i;
    }
    */

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
        PixelSetColor(c_wand, color);
        DrawSetStrokeColor(d_wand, c_wand);
        DrawColor( d_wand, (double) i, (double) i, PointMethod );
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


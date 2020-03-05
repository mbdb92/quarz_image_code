#ifndef TYPE_H
#define TYPE_H

#include <fftw3.h>
#include <MagickWand/MagickWand.h>
#include <stdbool.h>

struct Addresses {
    //in und out beinhalten den Imput aus alsa, und den output nach der fft
    //Der Plan wird initialisiert und anschließend von allen ffts verwendet
    //N ist die Größe des Inputvectors
    fftw_complex *fft_in, *fft_out;
    fftw_plan fft_plan;
    int N;

    // Diese drei Wands werden zum Zeichenen benötigt
    // vgl. Anleitung
    // Color um die Werte aus der fft auf dem Bild umzusetzen
    MagickWand *m_wand;
    DrawingWand *d_wand;
    PixelWand *c_wand;
    char *color;
};

#endif //TYPE_H

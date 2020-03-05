#include "type.h"
#include "drawing.h"

int main() {

    struct Addresses *adresse;
    adresse = malloc( sizeof(adresse) );

    setup_drawing( adresse );
    run_magick( adresse );
    destroy_drawing( adresse );

    free(adresse);
    return 0;
}

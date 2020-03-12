#include "type.h"
#include "fft.h"
#include "magick.h"

int main() {

    struct quarz_params *params;
    struct quarz_data *data;

    params = malloc( sizeof(struct quarz_params) );
    data = malloc( sizeof(struct quarz_data) );

    create_fft( &params->fft, &data->fft );
    setup_drawing( &params->magick );

    run_fft( &params->fft, &data->fft );
    run_magick_from_fft( &params->magick, &data->fft, (unsigned long) params->fft.size );

    destroy_fft( &params->fft, &data->fft );
    destroy_drawing( &params->magick );

    free(params);
    free(data);
    return 0;
}

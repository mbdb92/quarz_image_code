#include "type.h"
#include "fft.h"
#include "magick.h"

int main() {

    /*
     * These two structs contain every value needed during runtime
     * Constants are excluded
     */
    struct quarz_params *params;
    struct quarz_data *data;
    int return_value;

    params = malloc( sizeof(struct quarz_params) );
    data = malloc( sizeof(struct quarz_data) );

    /*
     * this block calls the setup functions for startup
     */
    return_value = create_fft( &params->fft, &data->fft );
    if( return_value != OK )
        return ERR;
    return_value = setup_drawing( &params->magick );
    if( return_value != OK )
        return ERR;

    run_fft( &params->fft, &data->fft );
    run_magick_from_fft( &params->magick, &data->fft, (unsigned long) params->fft.size );

    /*
     * This block contains the tear-down functions
     */
    destroy_fft( &params->fft, &data->fft );
    destroy_drawing( &params->magick );

    free(params);
    free(data);
    return OK;
}

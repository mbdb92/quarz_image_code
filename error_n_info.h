#ifndef ERROR_H
#define ERROR_H

void check_state( snd_pcm_t *handle );
void print_error_code( int errnr );

#endif

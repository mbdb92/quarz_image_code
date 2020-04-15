#ifndef SIGHANDLER_H
#define SIGHANDLER_H
#include <signal.h>

void quarz_sig_handler( int signum );

void alsa_sig_handler( int signum );

void fft_sig_handler( int signum );

void suspend( int *state_variable, int state, int shift );

#endif /* SIGHANDLER_H */

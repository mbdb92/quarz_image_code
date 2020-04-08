#ifndef SIGHANDLER_H
#define SIGHANDLER_H
#include <signal.h>

void quarz_sig_handler( int signum, siginfo_t *info, void *ptr );

void alsa_sig_handler( int signum );

void fft_handler_sig_pipe( int signum );

void suspend( int *state_variable, int state, int shift );

#endif /* SIGHANDLER_H */

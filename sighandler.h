#ifndef SIGHANDLER_H
#define SIGHANDLER_H

void quarz_sig_handler( int signum );

void alsa_sig_handler( int signum );

void fft_handler_sig_pipe( int signum );

void suspend( int *state_variable, int state, int shift );

#endif /* SIGHANDLER_H */

#ifndef SIGHANDLER_H
#define SIGHANDLER_H

void quarz_sig_handler( int signum );

void alsa_sig_handler( int signum );

void fft_handler_sig_pipe( int signum );

#endif /* SIGHANDLER_H */

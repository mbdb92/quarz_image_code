#include "sighandler.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
// For defines
#include "codes.h"
#include "type.h"

void quarz_sig_handler( int signum ) {
    /*
     * CHeck i.e. "man 3 wait" for more stuff
     * which can be done here. Especially the examples
     * TODO Fill code
     */
    if( signum == SIGCONT ) {
        switch (quarz_pipe_state) {
            case ZERO:
#ifdef PRINT_DEBUG
                printf("quarz caught SIGCONT, state = ALSA_READY\n");
#endif
                quarz_pipe_state = ALSA_READY;
                break;
            case ALSA_DONE:
#ifdef PRINT_DEBUG
                printf("quarz caught SIGCONT, state = FFT_READY\n");
#endif
                quarz_pipe_state = FFT_READY;
                break;
            case FFT_READY:
#ifdef PRINT_DEBUG
                printf("quarz caught SIGCONT, state = PIPE_DONE\n");
#endif
                quarz_pipe_state = PIPE_DONE;
                break;
            default:
                exit SIG_ERR;
                break;
        }
        return;
    } else {
        exit SIG_ERR;
    }
}

void alsa_sig_handler( int signum ) {
    if( signum == SIGPIPE ) {
#ifdef PRINT_DEBUG
                printf("asla caught SIGPIPE, state = READ_PIPE\n");
#endif
        alsa_state = READ_PIPE;
    }
    if( signum == SIGCONT ) {
#ifdef PRINT_DEBUG
                printf("asla caught SIGCONT, state = RUNTIME\n");
#endif
        if( alsa_state != RUNTIME ) {
            alsa_state = RUNTIME;
        }
    }
    if( signum == SIGURG ) {
#ifdef PRINT_DEBUG
                printf("asla caught SIGURG, state = SIZE_NEEDED\n");
#endif
        if( alsa_state != SIZE_NEEDED ) {
            alsa_state = SIZE_NEEDED;
        }
    }
    return;
}

void fft_sig_handler( int signum ) {
    if( signum == SIGPIPE ) {
        switch (fft_pipe_state) {
            case ALSA_DONE:
#ifdef PRINT_DEBUG
                printf("fft caught SIGPIPE, state = READ_PIPE\n");
#endif
                fft_pipe_state = READ_PIPE;
                break;
            case SIZE_NEEDED:
                fft_pipe_state = READ_PIPE;
            case RUNTIME:
                break;
            default:
                exit SIG_ERR;
                break;
        }
        return;
    }
    if( signum == SIGCONT ) {
        switch (fft_pipe_state) {
            case ZERO:
#ifdef PRINT_DEBUG
                printf("fft caught SIGCONT, state = ALSA_DONE\n");
#endif
                fft_pipe_state = ALSA_DONE;
                break;
            default:
                exit SIG_ERR;
                break;
        }
        return;
    } 
    if( signum == SIGURG ) {
#ifdef PRINT_DEBUG
                printf("fft caught SIGURG, state = READ_PIPE\n");
#endif
        fft_pipe_state = READ_PIPE;
    } else {
        exit SIG_ERR;
    }
}

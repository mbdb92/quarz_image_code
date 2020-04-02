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
                quarz_pipe_state = ALSA_READY;
#ifdef PRINT_DEBUG
                printf("quarz caught SIGCONT, state = %i\n", quarz_pipe_state);
#endif
                break;
            case ALSA_DONE:
                quarz_pipe_state = FFT_READY;
#ifdef PRINT_DEBUG
                printf("quarz caught SIGCONT, state = %i\n", quarz_pipe_state);
#endif
                break;
            case FFT_READY:
                quarz_pipe_state = PIPE_DONE;
#ifdef PRINT_DEBUG
                printf("quarz caught SIGCONT, state = %i\n", quarz_pipe_state);
#endif
                break;
            default:
             //   exit SIG_ERR;
                break;
        }
        return;
    } else {
       // exit SIG_ERR;
    }
}

void alsa_sig_handler( int signum ) {
    if( signum == SIGPIPE ) {
        alsa_state = READ_PIPE;
#ifdef PRINT_DEBUG
        printf("asla caught SIGPIPE, state = %i\n", alsa_state);
#endif
    }
    if( signum == SIGCONT ) {
        if( alsa_state != RUNTIME ) {
            alsa_state = RUNTIME;
#ifdef PRINT_DEBUG
            printf("asla caught SIGCONT, state = %i\n", alsa_state);
#endif
        }
    }
    if( signum == SIGURG ) {
        if( alsa_state != SIZE_NEEDED ) {
            alsa_state = SIZE_NEEDED;
#ifdef PRINT_DEBUG
            printf("asla caught SIGURG, state = %i\n", alsa_state);
#endif
        }
    }
    return;
}

void fft_sig_handler( int signum ) {
    if( signum == SIGPIPE ) {
        switch (fft_pipe_state) {
            case ALSA_DONE:
                fft_pipe_state = READ_PIPE;
#ifdef PRINT_DEBUG
                printf("fft caught SIGPIPE, state = %i\n", fft_pipe_state);
#endif
                break;
            case SIZE_NEEDED:
                fft_pipe_state = READ_PIPE;
#ifdef PRINT_DEBUG
                printf("fft caught SIGPIPE, state = %i\n", fft_pipe_state);
#endif
            case RUNTIME:
                break;
            default:
         //       exit SIG_ERR;
                break;
        }
        return;
    }
    if( signum == SIGCONT ) {
        switch (fft_pipe_state) {
            case ZERO:
                fft_pipe_state = ALSA_DONE;
#ifdef PRINT_DEBUG
                printf("fft caught SIGCONT, state = %i\n", fft_pipe_state);
#endif
                break;
            default:
           //     exit SIG_ERR;
                break;
        }
        return;
    } 
    if( signum == SIGURG ) {
        fft_pipe_state = READ_PIPE;
#ifdef PRINT_DEBUG
        printf("fft caught SIGURG, state = %i\n", fft_pipe_state);
#endif
    } else {
       // exit SIG_ERR;
    }
}

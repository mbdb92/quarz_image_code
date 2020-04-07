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
        if( quarz_pipe_state == ZERO ) {
            quarz_pipe_state += ALSA_READY;
#ifdef PRINT_DEBUG
            printf("quarz caught SIGCONT, state = %i\n", quarz_pipe_state);
#endif
            return ;
        }
            
        if( (quarz_pipe_state & ALSA_READY) >> SHIFT_A_R ) {
            quarz_pipe_state += ALSA_DONE;
#ifdef PRINT_DEBUG
            printf("quarz caught SIGCONT, state = %i\n", quarz_pipe_state);
#endif
            return ;
        }
        if( (quarz_pipe_state & ALSA_DONE) >> SHIFT_A_D ) {
            quarz_pipe_state += FFT_READY;
#ifdef PRINT_DEBUG
            printf("quarz caught SIGCONT, state = %i\n", quarz_pipe_state);
#endif
            return ;
        }
        if( (quarz_pipe_state & FFT_READY) >> SHIFT_F_R ) {
            quarz_pipe_state += PIPE_DONE;
#ifdef PRINT_DEBUG
            printf("quarz caught SIGCONT, state = %i\n", quarz_pipe_state);
#endif
            return ;
        }
    } else {
       // exit SIG_ERR;
    }
}

void alsa_sig_handler( int signum ) {
    if( signum == SIGPIPE ) {
        alsa_state += READ_PIPE;
#ifdef PRINT_DEBUG
        printf("asla caught SIGPIPE, state = %i\n", alsa_state);
#endif
    }
    if( signum == SIGCONT ) {
        if( !( (alsa_state & RUNTIME) >> SHIFT_R ) ) {
            alsa_state += RUNTIME;
#ifdef PRINT_DEBUG
            printf("asla caught SIGCONT, state = %i\n", alsa_state);
#endif
        }
    }
    if( signum == SIGURG ) {
        if( !( (alsa_state & SIZE_NEEDED) >> SHIFT_S_N ) ) {
            alsa_state += SIZE_NEEDED;
#ifdef PRINT_DEBUG
            printf("asla caught SIGURG, state = %i\n", alsa_state);
#endif
        }
    }
    return;
}

void fft_sig_handler( int signum ) {
    if( signum == SIGPIPE ) {
        if( (fft_pipe_state & ALSA_DONE) >> SHIFT_A_D ) {
            fft_pipe_state += READ_PIPE;
#ifdef PRINT_DEBUG
            printf("fft caught SIGPIPE, state = %i\n", fft_pipe_state);
#endif
            return ;
        }
        if( (fft_pipe_state & SIZE_NEEDED) >> SHIFT_S_N ) {
            fft_pipe_state += READ_PIPE;
#ifdef PRINT_DEBUG
            printf("fft caught SIGPIPE, state = %i\n", fft_pipe_state);
#endif
            return ;
        }
    }
    if( signum == SIGCONT ) {
        if( fft_pipe_state == ZERO ) {
            fft_pipe_state += ALSA_DONE;
#ifdef PRINT_DEBUG
            printf("fft caught SIGCONT, state = %i\n", fft_pipe_state);
#endif
            return ;
        }
    } 
    if( signum == SIGURG ) {
        fft_pipe_state += READ_PIPE;
#ifdef PRINT_DEBUG
        printf("fft caught SIGURG, state = %i\n", fft_pipe_state);
#endif
    } else {
       // exit SIG_ERR;
    }
}

void suspend( int *state_variable, int state, int shift ) {
    printf( "Waiting for %i to change to %i\n", state_variable, state );
    while( (*state_variable & state) >> shift ) {
        asm( "nop" );
    }
}

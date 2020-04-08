#include "sighandler.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
// For defines
#include "codes.h"
#include "type.h"

void quarz_sig_handler( int signum, siginfo_t *info, void *ptr ) {
    /*
     * CHeck i.e. "man 3 wait" for more stuff
     * which can be done here. Especially the examples
     * TODO Fill code
     */
    if( signum == SIGCONT ) {
        if( !((quarz_pipe_state & ALSA_READY) >> SHIFT_A_R) && (alsa_pid == (int) info->si_pid) ) {
            quarz_pipe_state += ALSA_READY;
#ifdef PRINT_SIGNAL
            printf("(quarz) caught SIGCONT from %i, state = %i\n", (int) info->si_pid, quarz_pipe_state);
#endif
            return ;
        }
            
        if( ((quarz_pipe_state & ALSA_READY) >> SHIFT_A_R) && (alsa_pid == (int) info->si_pid)) {
            quarz_pipe_state += ALSA_DONE;
#ifdef PRINT_SIGNAL
            printf("(quarz) caught SIGCONT from %i, state = %i\n", (int) info->si_pid, quarz_pipe_state);
#endif
            return ;
        }
        if( !((quarz_pipe_state & FFT_READY) >> SHIFT_F_R) && (alsa_pid != (int) info->si_pid)) {
            quarz_pipe_state += FFT_READY;
#ifdef PRINT_SIGNAL
            printf("(quarz) caught SIGCONT from %i, state = %i\n", (int) info->si_pid, quarz_pipe_state);
#endif
            return ;
        }
        if( ((quarz_pipe_state & FFT_READY) >> SHIFT_F_R) && (alsa_pid != (int) info->si_pid)) {
            quarz_pipe_state += PIPE_DONE;
#ifdef PRINT_SIGNAL
            printf("(quarz) caught SIGCONT from %i, state = %i\n", (int) info->si_pid, quarz_pipe_state);
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
#ifdef PRINT_SIGNAL
        printf("(alsa) caught SIGPIPE, state = %i\n", alsa_state);
#endif
    }
    if( signum == SIGCONT ) {
        if( !( (alsa_state & RUNTIME) >> SHIFT_R ) ) {
            alsa_state += RUNTIME;
#ifdef PRINT_SIGNAL
            printf("(alsa) caught SIGCONT, state = %i\n", alsa_state);
#endif
        }
    }
    if( signum == SIGURG ) {
        if( !( (alsa_state & SIZE_NEEDED) >> SHIFT_S_N ) ) {
            alsa_state += SIZE_NEEDED;
#ifdef PRINT_SIGNAL
            printf("(alsa) caught SIGURG, state = %i\n", alsa_state);
#endif
        }
    }
    return;
}

void fft_sig_handler( int signum ) {
    if( signum == SIGPIPE ) {
        if( (fft_pipe_state & ALSA_DONE) >> SHIFT_A_D ) {
            fft_pipe_state += READ_PIPE;
#ifdef PRINT_SIGNAL
            printf("(fft) caught SIGPIPE, state = %i\n", fft_pipe_state);
#endif
            return ;
        }
        if( (fft_pipe_state & SIZE_NEEDED) >> SHIFT_S_N ) {
            fft_pipe_state += READ_PIPE;
#ifdef PRINT_SIGNAL
            printf("(fft) caught SIGPIPE, state = %i\n", fft_pipe_state);
#endif
            return ;
        }
    }
    if( signum == SIGCONT ) {
        if( !((fft_pipe_state & ALSA_DONE) >> SHIFT_A_D) ) {
            fft_pipe_state += ALSA_DONE;
#ifdef PRINT_SIGNAL
            printf("(fft) caught SIGCONT, state = %i\n", fft_pipe_state);
#endif
            return ;
        }
    } 
    if( signum == SIGURG ) {
        fft_pipe_state += READ_PIPE;
#ifdef PRINT_SIGNAL
        printf("(fft) caught SIGURG, state = %i\n", fft_pipe_state);
#endif
    } else {
       // exit SIG_ERR;
    }
}

void suspend( int *state_variable, int state, int shift ) {
#ifdef PRINT_DEBUG
    printf( "Waiting for %i to add %i\n", state_variable, state );
    printf( "Sleep state: %i\n", !((*state_variable & state) >> shift) ); 
#endif
    // Needs to be inverted, as else it just leaves the while loop
    // If not, it would be while(0) -> exit
    while( !((*state_variable & state) >> shift) ) {
        asm( "nop" );
    }
#ifdef PRINT_DEBUG
    printf( "continue, with %i as %i\n", state_variable, *state_variable );
#endif
}

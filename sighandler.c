#include "sighandler.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
// For defines
#include "codes.h"
#include "type.h"

/* 
 * This declarations are needed, so the external Variables can be used
 * They get declarized in their respectable Files already,
 * but this doesn't solve the failed linker. 
 * When they are declared here, they will get overrided during runtime
 * but the linker won't fail anymore
 */

//int quarz_state = 0;
//int alsa_state = 0;
//int fft_pipe_state = 0;
//extern int quarz_state;
//extern int alsa_state;
//extern int fft_master_state;

void quarz_sig_handler( int signum ) {
    if( signum == SIGUSR1 ) {
        if( !( (quarz_state & ALSA_READY) >> SHIFT_A_R ) ) {
            quarz_state += ALSA_READY;
#ifdef PRINT_SIGNAL
            printf("(quarz) caught SIGUSR1, state = %i\n", quarz_state);
#endif
        }
    }
    if( signum == SIGUSR2 ) {
        if( !( (quarz_state & FFT_READY) >> SHIFT_F_R ) ) {
            quarz_state += FFT_READY;
#ifdef PRINT_SIGNAL
            printf("(quarz) caught SIGUSR2, state = %i\n", quarz_state);
#endif
        }
    }
    return ;
}

void alsa_sig_handler( int signum ) {
    // Gets raised by fft, once it's ready to start
    if( signum == SIGCONT ) {
        if( !( (alsa_state & RUNTIME) >> SHIFT_R ) ) {
            alsa_state += RUNTIME;
#ifdef PRINT_SIGNAL
            printf("(alsa) caught SIGCONT, state = %i\n", alsa_state);
#endif
        }
    }
    // This gets raised by quarz, once the shared memory is filled
    // and alsa can read from it
    if( signum == SIGUSR1 ) {
        if( !( (alsa_state & SHMEM_READ) >> SHIFT_S_R ) ) {
            alsa_state += SHMEM_READ;
#ifdef PRINT_SIGNAL
            printf("(alsa) caught SIGUSR1, state = %i\n", alsa_state);
#endif
        }
    }
    // This is raised by fft_master to signal alsa if it should write to
    // the pipe or not.
    // It works like a toggle switch
    if( signum == SIGUSR2 ) {
        if( ( (alsa_state & RUNTIME) >> SHIFT_R ) ) {
            alsa_state = alsa_state - RUNTIME;
#ifdef PRINT_SIGNAL
            printf("(alsa) caught SIGUSR2, state = %i\n", alsa_state);
#endif
        }
        if( !( (alsa_state & RUNTIME) >> SHIFT_R ) ) {
            alsa_state = alsa_state + RUNTIME;
#ifdef PRINT_SIGNAL
            printf("(alsa) caught SIGUSR2, state = %i\n", alsa_state);
#endif
        }
    }
    return;
}

void fft_sig_handler( int signum ) {
    if( signum == SIGCONT ) {
        if( !((fft_pipe_state & ALSA_DONE) >> SHIFT_A_D) ) {
            fft_pipe_state += ALSA_DONE;
#ifdef PRINT_SIGNAL
            printf("(fft) caught SIGCONT, state = %i\n", fft_pipe_state);
#endif
            return ;
        }
    } 
    if( signum == SIGUSR1 ) {
        if( !((fft_pipe_state & SHMEM_READ) >> SHIFT_S_R) ) {
            fft_pipe_state += SHMEM_READ;
#ifdef PRINT_SIGNAL
            printf("(fft) caught SIGUSR1, state = %i\n", fft_pipe_state);
#endif
        }
    }
    if( signum == SIGUSR2 ) {
        if( ((fft_pipe_state & RUNTIME) >> SHIFT_R) ) {
            fft_pipe_state -= RUNTIME;
#ifdef PRINT_SIGNAL
            printf("(fft) caught SIGUSR2, state = %i\n", fft_pipe_state);
#endif
        }
    }
}

void suspend( int *state_variable, int state, int shift ) {
#ifdef PRINT_DEBUG_V
    printf( "Waiting for %i to add %i\n", state_variable, state );
    printf( "Sleep state: %i\n", !((*state_variable & state) >> shift) ); 
#endif
    // Needs to be inverted, as else it just leaves the while loop
    // If not, it would be while(0) -> exit
    while( !((*state_variable & state) >> shift) ) {
        asm( "nop" );
    }
#ifdef PRINT_DEBUG_V
    printf( "continue, with %i as %i\n", state_variable, *state_variable );
#endif
}

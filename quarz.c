// For wait
#include <sys/wait.h>
// For signals
#include <signal.h>
// For shared memory#
#include <sys/mman.h>
// For pipe, signals, wait, mmam
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "sighandler.h"
// For strlen and strncmp
#include <string.h>
// For defines
#include "codes.h"
#include "type.h"
// For calling alsa child
#include "alsa.h"
#include "fft.h"

struct sigaction quarz_act;

int main () {
    int pipefd[2];
    int return_value, status;
    struct pid_collection pids;
    // To check if signalhandler is set correctly
    void (*sig_handler_return) (int);
    void *shmem;
    int protection, visibility;
    char temp_buffer;
    
    // Used for transfer of pid to childs later
    quarz_state = ZERO;

    memset( &quarz_act, 0, sizeof(quarz_act) );

    quarz_act.sa_sigaction = quarz_sig_handler;
    quarz_act.sa_flags = SA_SIGINFO;

    sigaction( SIGUSR1, &quarz_act, 0 );
    sigaction( SIGUSR2, &quarz_act, 0 );

    // Setup of the pipe
    return_value = pipe(pipefd);
    if( return_value != OK )
        return E_PIPE;

    // Setup for shared mamory
    protection = PROT_READ | PROT_WRITE;
    visibility = MAP_SHARED | MAP_ANONYMOUS;

    /*
     * The sizeof(int) is needed to transfer the size
     */
    shmem = mmap( NULL, (sizeof(pids) + sizeof(int)), protection, visibility, -1, 0 );

    // Forking for alsa
    pids.pid_alsa = fork();
    if( pids.pid_alsa == OK ) {
        int rc;

        rc = alsa_handler( pipefd, shmem );

    } else if( pids.pid_alsa == -1 ) {
        return E_FORK;
    // quarz continues here
    } else {
        // Fork for fft handler
        pids.pid_fft_master = fork();
        if( pids.pid_fft_master == OK ) {
            int rc;

            rc = fft_handler( pipefd, shmem );

        } else if( pids.pid_fft_master == -1 ){
            return E_FORK;
        /*
         * This part is the quarz main task
         */
        } else {

            /*
             * This part pipes the pids of the childs to
             * each other. With this I can prevent a
             * shared memory
             */
            // For ALSA
            pids.pid_quarz = getpid();
#ifdef PRINT_DEBUG
            printf("PIDS:\n");
            printf("quarz: %i, alsa: %i, fft: %i\n", pids.pid_quarz, pids.pid_alsa, pids.pid_fft_master);
            printf("State Adresses:\n");
            printf("quarz: %i, alsa: %i, fft: %i\n", &quarz_state, &alsa_state, &fft_pipe_state);
#endif
            
            memcpy( shmem, &pids, sizeof(pids) );

            while( !(((quarz_state & FFT_READY) >> SHIFT_F_R) && (quarz_state & ALSA_READY) >> SHIFT_A_R ) ) {
                asm( "nop" );
            }

            kill( pids.pid_alsa, SIGUSR1 );
#ifdef PRINT_SIGNAL
            printf("(quarz) %i send SIGUSR1 to (alsa) %i\n", pids.pid_quarz, pids.pid_alsa);
#endif
        /*
         * This part war staken from "man 3 wait"
         * Check it for references and additions
         * TODO Enhance this part to be more useful
         */
            do {
                waitpid( pids.pid_alsa, &status, WUNTRACED
            #ifdef WCONTINUED
                        | WCONTINUED 
            #endif
                        );
            } while( !WIFEXITED(status) );

        }
    }

    return OK;
}

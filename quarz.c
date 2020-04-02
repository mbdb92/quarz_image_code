// For wait
#include <sys/wait.h>
// For signals
#include <signal.h>
// For pipe, signals, wait
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "sighandler.h"
// For strlen
#include <string.h>
// For defines
#include "codes.h"
#include "type.h"
// For calling alsa child
#include "alsa.h"
#include "fft.h"

//Signal Handler Definition
void quarz_sig_handler( int signum );
//int quarz_pipe_state;

int main () {
    int pipefd[2];
    int return_value, status;
    struct pid_collection pids;
    // To check if signalhandler is set correctly
    void (*sig_handler_return) (int);
    char temp_buffer;

    // Used for transfer of pid to childs later
    quarz_pipe_state = ZERO;

    /*
     * Setting up the sighandler for the two used signals by its child
     * SIGCONT to show it's ready to recieve
     * SIGPIPE is currently not needed
     * TODO remove SIGPIPE if unused in the end
     */
    sig_handler_return = signal( SIGPIPE, quarz_sig_handler );
    if( sig_handler_return == SIG_ERR )
        return E_SIGH_QUARZ;
    sig_handler_return = signal( SIGCONT, quarz_sig_handler );
    if( sig_handler_return == SIG_ERR )
        return E_SIGH_QUARZ;

    // Setup of the pipe
    return_value = pipe(pipefd);
    if( return_value != OK )
        return E_PIPE;

    // Forking for alsa
    pids.pid_alsa = fork();
    if( pids.pid_alsa == OK ) {
        int rc;

        rc = alsa_handler( pipefd );

    } else if( pids.pid_alsa == -1 ) {
        return E_FORK;
    // quarz continues here
    } else {

        // Fork for fft handler
        pids.pid_fft_master = fork();
        if( pids.pid_fft_master == OK ) {
            int rc;

            rc = fft_handler( pipefd );

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
            printf("quarz: %i, alsa: %i, fft: %i\n", pids.pid_quarz, pids.pid_alsa, pids.pid_fft_master);
            // If alsa hasn't raised the continue signal
            if( quarz_pipe_state != ALSA_READY ) {
#ifdef PRINT_DEBUG
                printf("%i: waiting for SIGCONT!\n", pids.pid_quarz);
#endif
                pause();
            }
            write( pipefd[1], &pids, sizeof(pids) );
            // Tells alsa to read the pipe
            kill( pids.pid_alsa, SIGPIPE);
#ifdef PRINT_DEBUG
            printf("%i: send SIGPIPE to %i\n", pids.pid_quarz, pids.pid_alsa);
#endif
            quarz_pipe_state = ALSA_DONE;

            // To give alsa some time to read the pipe
            // and wait for fft_master

            // Waits for fft to be ready
            if ( quarz_pipe_state != FFT_READY ) {
#ifdef PRINT_DEBUG
                printf("%i: waiting for SIGCONT!\n", pids.pid_quarz);
#endif
                pause();
            }
            while (temp_buffer != EOF) {
                return_value = read( pipefd[0], &temp_buffer, sizeof(char) );
#ifdef PRINT_DEBUG
                printf("%i: read char from pipe with return_value: %i\n", pids.pid_quarz, return_value);
#endif
            } ;
#ifdef PRINT_DEBUG
            printf("%i: left loop, file should be clear\n", pids.pid_quarz);
#endif
            write( pipefd[1], &pids, sizeof(pids) );
            // Tells fft to read pipe
            kill( pids.pid_fft_master, SIGPIPE);
#ifdef PRINT_DEBUG
            printf("%i: send SIGPIPE to %i\n", pids.pid_quarz, pids.pid_fft_master);
#endif

            // For fft to wrap up
#ifdef PRINT_DEBUG
                printf("%i: waiting for SIGCONT!\n", pids.pid_quarz);
#endif
            pause();
            
#ifdef PRINT_DEBUG
            printf("%i setup done\n", pids.pid_quarz);
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

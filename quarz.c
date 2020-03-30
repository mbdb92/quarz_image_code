// For wait
#include <sys/wait.h>
// For signals
#include <signal.h>
// For pipe, signals, wait
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
// For strlen
#include <string.h>
// For defines
#include "codes.h"
#include "type.h"
// For calling alsa child
#include "alsa.h"

//Signal Handler Definition
void quarz_sig_handler( int signum );
int quarz_pipe_state;

int main () {
    int pipefd[2];
    int return_value, status;
    struct pid_collection pids;
    // To check if signalhandler is set correctly
    void (*sig_handler_return) (int);

    quarz_pipe_state = ZERO;

    sig_handler_return = signal( SIGPIPE, quarz_sig_handler );
    if( sig_handler_return == SIG_ERR )
        return E_SIGH_QUARZ;
    sig_handler_return = signal( SIGCONT, quarz_sig_handler );
    if( sig_handler_return == SIG_ERR )
        return E_SIGH_QUARZ;

    return_value = pipe(pipefd);
    if( return_value != OK )
        return E_PIPE;

    pids.pid_alsa = fork();
    if( pids.pid_alsa == OK ) {
        int rc;

        rc = alsa_handler( pipefd );

    } else if( pids.pid_alsa == -1 ) {
        return E_FORK;
    } else {

        pids.pid_fft_master = fork();
        if( pids.pid_fft_master == OK ) {
            int rc;

            rc = run_fft( pipefd );

        } else if( pids.pid_fft_master == -1 ){
            return E_FORK;
        } else {

            /*
             * This part pipes the pids of the childs to
             * each other. With this I can prevent a
             * shared memory
             */
            // For ALSA
            pids.pid_quarz = getpid();
            // If alsa hasn't raised the continue signal
            while( quarz_pipe_state != ALSA_READY ) {
                pause();
            }
            write( pipefd[1], &pids, sizeof(pids) );
            // Tells alsa to read the pipe
            kill( pids.pid_alsa, SIGPIPE);

            // To give alsa some time to read the pipe
            sleep(1);

            // For FFT Master
            // Tells him, that piping to alsa is done
            kill( pids.pid_fft_master, SIGCONT);
            // Waits for fft to be ready
            while( quarz_pipe_state != FFT_READY ) {
                pause();
            }
            write( pipefd[1], &pids, sizeof(pids) );
            // Tells fft to read pipe
            kill( pids.pid_fft_master, SIGPIPE);

            // For fft to wrap up
            sleep(1);
            
#ifdef PRINT_DEBUG
            printf("Quarz pipe state is: %i\n", quarz_pipe_state);
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

void handler_sig_pipe( int signum ) {
    /*
     * CHeck i.e. "man 3 wait" for more stuff
     * which can be done here. Especially the examples
     * TODO Fill code
     */
    if( signum == SIGCONT ) {
        switch (quarz_pipe_state) {
            case ZERO:
                quarz_pipe_state = ALSA_READY;
                break;
            case ALSA_READY:
                quarz_pipe_state = ALSA_DONE;
                break;
            case ALSA_DONE:
                quarz_pipe_state = FFT_READY;
                break;
            case FFT_READY:
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

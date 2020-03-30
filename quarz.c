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
void handler_sig_quit( int signum );

int main () {
    int pipefd[2];
    int return_value, status;
    struct pid_collection pids;
    // To check if signalhandler is set correctly
    void (*sig_handler_return) (int);

    sig_handler_return = signal( SIGQUIT, handler_sig_quit );
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
            //Load child code
        } else if( pids.pid_fft_master == -1 ){
            return E_FORK;
        } else {

            pids.pid_quarz = getpid();
            sleep(2);
            write( pipefd[1], &pids, sizeof(pids) );
            kill( pids.pid_alsa, SIGPIPE);

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

void handler_sig_quit( int signum ) {
    /*
     * CHeck i.e. "man 3 wait" for more stuff
     * which can be done here. Especially the examples
     * TODO Fill code
     */
    // Sending signal to childs
}

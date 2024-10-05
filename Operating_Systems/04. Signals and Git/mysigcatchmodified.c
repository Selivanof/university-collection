#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int sigint_count = 0;

void custom_sigint_handler(int signo) {
    if (sigint_count == 2) {
        // Restore default behavior after catching the signal twice
        signal(SIGINT, SIG_DFL);
        printf("Default behavior for SIGINT.\n");
        exit(0);
    }
    printf("Custom handler: Caught signal %d (SIGINT)\n", signo);
    sigint_count++;

}

int main() {
    if (signal(SIGINT, custom_sigint_handler) == SIG_ERR) {
        perror("Error setting up signal handler");
        return 1;
    }

    printf("Press Ctrl+C to generate a SIGINT signal.\n");

    while (1) {
        // Do nothing, wait for the signal
    }

    return 0;
}
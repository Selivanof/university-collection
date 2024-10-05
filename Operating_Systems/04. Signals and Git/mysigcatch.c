#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void signal_handler(int SIGNO) {
    printf("SIGINT signal caught!\n");
    exit(0);
}

int main() {
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("Error setting up signal handler");
        return 1;
    }

    printf("Press Ctrl+C to exit.\n");

    while (1) {
        // Do nothing, wait for the signal
    }

    return 0;
}
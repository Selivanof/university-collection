#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h> 
#include "child.h"
volatile sig_atomic_t child_exit_flag = 0;

void handle_signal(int signum) {
    // Handle the signal to set the exit flag
    if (signum == SIGUSR1) {
        child_exit_flag = 1;
    }
}

void child_process(int child_number, int write_fd) {
    char ch;
    printf("Child %d: Enter a character: ", child_number);
    scanf(" %c", &ch);
    pid_t child_pid = getpid();

    // Write the character to the pipe
    write(write_fd, &ch, sizeof(char));

    // Wait for the signal to print the character
    while (!child_exit_flag) {
        sleep(1); 
    }

    // Print the character and exit
    printf("I am child process %d (Global PID: %d) and my character was %c\n", child_number, child_pid, ch);
}

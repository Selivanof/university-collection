#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "child.h"
extern volatile sig_atomic_t child_exit_flag;

// For sorting PIDs
int comparePids(const void *a, const void *b) {
    return (*(pid_t*)a - *(pid_t*)b);
}

int main() {
    // Set up the signal handler
    signal(SIGUSR1, handle_signal);

    // Array of children PIDs
    pid_t pids[4];

    // Loop to create 4 child processes, each with its own pipe
    for (int i = 0; i < 4; ++i) {
        int pipe_fd[2];

        if (pipe(pipe_fd) == -1) {
            perror("pipe error");
            exit(EXIT_FAILURE);
        }

        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork error");
            exit(EXIT_FAILURE);
        } else if (pids[i] == 0) {
            // Child process code
            close(pipe_fd[0]);  // Close the read end of the pipe in the child
            child_process(i + 1, pipe_fd[1]);
            close(pipe_fd[1]);  // Close the write end of the pipe in the child
            exit(EXIT_SUCCESS);
        } else {
            // Parent process
            close(pipe_fd[1]);  // Close the write end of the pipe in the parent

            // Read the character from the pipe
            char received_char;
            read(pipe_fd[0], &received_char, sizeof(char));
            close(pipe_fd[0]);  // Close the read end of the pipe in the parent
        }
    }

    // Sort PIDs
    qsort(pids, 4, sizeof(pid_t), comparePids);

    // Tell each child to print their character and exit in order of their PIDs
    for (int i = 0; i < 4; ++i) {
        kill(pids[i], SIGUSR1);
        waitpid(pids[i], NULL, 0);
    }
    return 0;
}

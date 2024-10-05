#ifndef CHILD_H
#define CHILD_H

#include <signal.h>

extern volatile sig_atomic_t child_exit_flag;

void handle_signal(int signum);
void child_process(int child_number, int write_fd);

#endif /* CHILD_H */

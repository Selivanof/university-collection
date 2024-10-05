<div align="center">
<h3 align="center">Operating Systems</h3>

  <a href=""> ![Static Badge](https://img.shields.io/badge/Language-C-6866fb)</a>
  <a href=""> ![Static Badge](https://img.shields.io/badge/Language-Shell%20Script-6866fb)</a>
  <a href=""> ![Static Badge](https://img.shields.io/badge/Semester-7-green)</a>
</div>

|   Course Information     |                             |
|--------------------------|-----------------------------|
| Semester                 | 7                           |
| Attendance Year          | 2023-2024                   |
| Department               | Electronics and Computers   |
| Project Type             | Optional                    |
| Team Project             | No                          |
| Languages Used           | C, Shell Script (Bash, Fish)|
# Table of Contents
1. [Simple Linux Commands](#01-simple-linux-commands)
2. [Cleanup (Fish Shell)](#02-cleanup-fish-shell)
3. [Forks (C)](#03-forks-c)
4. [Signals and Git](#04-signals-and-git)
5. [Final Assignments](#05-final-assignments)


## 01. Simple Linux Commands

A very simple introduction to basic Linux commands. Includes creation of directories/files, file reading/writing and usage of pipes.


## 02. Cleanup (Fish Shell)
A function written in fish shell that cleans up unused files after a specific amount of days of inactivity. The function's main features are:
1) Recursive directory cleaning flag.
2) Dry-run mode to display the files that would be deleted, without actually deleting them.
3) Exclude certain types of files and directories by name (can be more than one).
4) Sorting the files by either largest to smallest or opposite.
5) Adding a confirmation prompt before actually deleting.
   

## 03. Forks (C)
A basic demonstration of forks in C.

A parent process forks 4 subprocesses. Each one of them gets a character as input from the user and reports it back to the parent process. When the parent receives all 4 inputs, it instructs the processes to print their received character in order of ascending PID. 


## 04. Signals and Git

#### a) Signals
A signal catcher that gracefully terminates the program upon receiving SIGINT and a "modified" signal catcher that catches SIGINT 2 times, after which it reverts to default handling of SIGINT.

#### b) Git
A simple introduction to Git and GitHub. This assignment involved the creation of a [separate repo](https://github.com/Selivanof/GitHashingExercise) and the usage of the Linux terminal to add/commit/push to both a local and the github repo.


## 05. Final Assignments
#### a) C Memory Management
A vector struct in C, similar to std::vector of C++. The required functions, apart from a proper a) destructor and b) constructor were c) vector_push_back(), d) vector_pop() and e) vector_remove().

#### b) Resource Limits
A shell function that runs until one of the predefined resource limits is reached.

#### b) Makefile
A Makefile for 3 C files (runner.c, test_runner.c and libdummy.c). Compiles runner and test_runner using libdummy as a shared library.

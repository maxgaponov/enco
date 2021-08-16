#pragma once

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ptrace.h>

#define die(message) do { \
    perror(message);      \
    exit(EXIT_FAILURE);   \
} while (0)

class SyscallManager {
public:
    SyscallManager(pid_t pid);
    bool resume();
private:
    pid_t pid;
};

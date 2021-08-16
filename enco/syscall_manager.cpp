#include <syscall_manager.hpp>

SyscallManager::SyscallManager(pid_t pid) : pid(pid) {
    // Wait for tracee
    int status;
    if (waitpid(pid, &status, 0) < 0) {
        die("wait");
    }

    // Check if tracee was stopped properly
    if (!WIFSTOPPED(status) || WSTOPSIG(status) != SIGSTOP) {
        fprintf(stderr, "Program was not stopped or it was stopped by wrong signal\n");
        exit(EXIT_FAILURE);
    }

    // Tracee will get (SIGTRAP | 0x80) signal before and after each syscall
    if (ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACESYSGOOD) < 0) {
        die("ptrace set options");
    }
}

bool SyscallManager::resume() {
    int status;
    do {
        // Tracee will run until it makes syscall
        if (ptrace(PTRACE_SYSCALL, pid, nullptr, nullptr) < 0) {
            die("ptrace syscall");
        }

        // Wait for tracee to stop
        if (waitpid(pid, &status, 0) < 0) {
            die("wait");
        }

        // Stop tracing if tracee exited
        if (WIFEXITED(status)) {
            return false;
        }

        // Continue tracing if tracee was not stopped by (SIGTRAP | 0x80)
    } while (!WIFSTOPPED(status) || WSTOPSIG(status) != (SIGTRAP | 0x80));
    return true;
}

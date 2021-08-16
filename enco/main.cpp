#include <lua_interface.hpp>
#include <syscall_manager.hpp>

#include <cstdio>
#include <cstdlib>
#include <sys/user.h>

int usage(const char *program_name) {
    fprintf(stderr, "Usage: %s [script] <program> <args...>\n", program_name);
    return 1;
}

void run_process(char **argv) {
    // Child process become tracee, parent process become tracer
    if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0) {
        die("ptrace traceme");
    }

    // Raise SIGSTOP to wait until tracer will be ready
    raise(SIGSTOP);

    // Execute given program with arguments
    if (execvp(argv[0], argv) < 0) {
        die("exec");
    }
}

void trace_process(pid_t pid, const char *script_file) {
    // Redirect stdout to stderr
    dup2(STDERR_FILENO, STDOUT_FILENO);

    LuaInterface lua;
    lua.run_script_file(script_file);

    SyscallManager manager(pid);

    const size_t ARG_COUNT = 6;

    bool before_syscall_stage = true;
    struct user_regs_struct regs;
    int64_t args[ARG_COUNT];
    unsigned long long int *syscall_res = &regs.rax;
    unsigned long long int *syscall_num = &regs.orig_rax;
    unsigned long long int *arg_regs[ARG_COUNT] = {&regs.rdi,
                                                   &regs.rsi,
                                                   &regs.rdx,
                                                   &regs.r10,
                                                   &regs.r8,
                                                   &regs.r9};
    lua.set_func("set_arg_value", [pid, &arg_regs, &regs](int arg_num, int64_t value) mutable {
        *arg_regs[arg_num - 1] = value;
        if (ptrace(PTRACE_SETREGS, pid, nullptr, &regs) < 0) {
            die("ptrace set regs");
        }
    });
    lua.set_func("set_res_value", [pid, &syscall_res, &regs](int64_t value) mutable {
        *syscall_res = value;
        if (ptrace(PTRACE_SETREGS, pid, nullptr, &regs) < 0) {
            die("ptrace set regs");
        }
    });
    lua.set_func("cancel_syscall", [pid, &syscall_num, &regs]() mutable {
        *syscall_num = -EPERM;
        if (ptrace(PTRACE_SETREGS, pid, nullptr, &regs) < 0) {
            die("ptrace set regs");
        }
    });

    while (manager.resume()) {
        // Get tracee registers
        if (ptrace(PTRACE_GETREGS, pid, nullptr, &regs) < 0) {
            die("ptrace get regs");
        }

        // Call a suitable hook
        if (before_syscall_stage) {
            for (size_t i = 0; i < ARG_COUNT; ++i) {
                args[i] = *arg_regs[i];
            }
            lua.set_var("args", &args);
            lua.call_func("before_syscall", static_cast<int64_t>(*syscall_num));
        } else {
            lua.call_func("after_syscall", static_cast<int64_t>(*syscall_res));
        }

        // Change stage (before_syscall <-> after_syscall)
        before_syscall_stage = !before_syscall_stage;
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        return usage(argv[0]);
    }
    pid_t pid;
    if ((pid = fork()) < 0) {
        die("fork");
    }
    if (pid == 0) {
        // Child process
        run_process(argv + 2);
    } else {
        // Parent process
        trace_process(pid, argv[1]);
    }
    return 0;
}

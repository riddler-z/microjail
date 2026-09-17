#define _GNU_SOURCE //needed for CLONE_* flags from <sched.h>
#include <sched.h> //to get flags for clone() in milestone 3
#include <stdio.h> // standard I/O 
#include <sys/types.h> //defines system types, including *_t
#include <unistd.h> //unix standard - gives fork() and getpid()
#include <stdlib.h> //exit() lives in this library
#include <sys/wait.h> //where macros for waitpid() live
#include <sys/stat.h>  // for mkdir()
#include <errno.h>
#include <seccomp.h>

#define STACK_SIZE (1024 * 1024)

int setup_seccomp(void) {
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL_PROCESS);    if (ctx == NULL) {
        fprintf(stderr, "seccomp_init failed\n");
        return -1;
    }

    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(execve), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(brk), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(set_tid_address), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(set_robust_list), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(rseq), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(prlimit64), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(readlinkat), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(getrandom), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(mprotect), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(newfstatat), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);

    if (seccomp_load(ctx) == -1)
    {
        perror("seccomp_load failed.");
        exit(1);
    }

    seccomp_release(ctx);
    return 0;
}

int write_to_file(const char *path, const char *value)
{
    FILE *f = fopen(path, "w");
    if (f == NULL)
    {
        perror(path);
        return -1;
    }
    fprintf(f, "%s", value);
    fclose(f);
    return 0;
}

int child_funct(void *args)
{
    printf("inside the new namespace, my PID is %d\n", getpid());
    char *exec_args[] = {"/bin/jailed_hello", NULL};
    int chroot_result = chroot("jail");
    if (chroot_result == -1)
    {
        perror("Root folder not found.");
        exit(1);
    }
    int chdir_result = chdir("/");
    if (chdir_result == -1)
    {
        perror("Couldn't change directory");
        exit(1);
    }
    if (setup_seccomp() == -1) 
    {
        exit(1);
    }
    if (execvp(exec_args[0], exec_args) == -1)
    {
        perror("Execvp failed");
        exit(1);
    }
    return 1;
}

int main(void) 
{
    if (mkdir("/sys/fs/cgroup/microjail", 0755) == -1 && errno != EEXIST) 
    {
        perror("mkdir cgroup folder failed.");
        exit(1);
    }
    if (write_to_file("/sys/fs/cgroup/cgroup.subtree_control", "+cpu +memory") == -1)
    {
        exit(1);
    }
    if (write_to_file("/sys/fs/cgroup/microjail/memory.max", "134217728") == -1)
    {
        exit(1);
    }
    if (write_to_file("/sys/fs/cgroup/microjail/cpu.max", "50000 100000") == -1)
    {
        exit(1);
    }
    if (write_to_file("/sys/fs/cgroup/microjail/memory.swap.max", "0") == -1) 
    {
        exit(1);
    }
    char *stack = malloc(STACK_SIZE);
    if (stack == NULL)
    {
        perror("malloc failed");
        exit(1);
    }
    char *stack_top = stack + STACK_SIZE; //stack grows downwards in memory so top is high address
    pid_t pid = clone(child_funct, stack_top, 
        CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWNET | SIGCHLD,
        NULL);
    if (pid == -1)
    {
        perror("clone failed");
        exit(1);
    }
    char pid_str[16];
    snprintf(pid_str, sizeof(pid_str), "%d", pid);
    if (write_to_file("/sys/fs/cgroup/microjail/cgroup.procs", pid_str) == -1) 
    {
        exit(1);
    }

    int status;
    pid_t wpid = waitpid(pid, &status, 0); //capture return value and compare directly to -1
    if (wpid == -1)
    {
        perror("Waitpid failed.");
        exit(1);
    } 
    if (WIFEXITED(status)) 
    {
        printf("Child exited normally with status %d\n", WEXITSTATUS(status));
    } 
    else if (WIFSIGNALED(status)) 
    {
        printf("Child was killed by signal %d\n", WTERMSIG(status));
    } 
    else 
    {
        printf("Child did not exit normally\n");
    }

    int rmdir_result = rmdir("/sys/fs/cgroup/microjail");
    if (rmdir_result == -1) {
        perror("rmdir cgroup folder failed");
    }

    free(stack);
    
    return 0;
}
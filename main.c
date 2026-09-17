#define _GNU_SOURCE //needed for CLONE_* flags from <sched.h>
#include <sched.h> //to get flags for clone() in milestone 3
#include <stdio.h> // standard I/O 
#include <sys/types.h> //defines system types, including *_t
#include <unistd.h> //unix standard - gives fork() and getpid()
#include <stdlib.h> //exit() lives in this library
#include <sys/wait.h> //where macros for waitpid() live
#include <sys/stat.h>  // for mkdir()
#include <errno.h>

#define STACK_SIZE (1024 * 1024)

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
    char *exec_args[] = {"/bin/jailed_hello", "hello from the child process", NULL};
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
    } else 
    {
        printf("Child did not exit normally\n");
    }
    return 0; 
}
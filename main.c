#include <stdio.h> // standard I/O 
#include <sys/types.h> //defines system types, including *_t
#include <unistd.h> //unix standard - gives fork() and getpid()
#include <stdlib.h> //exit() lives in this library
#include <sys/wait.h> //where macros for waitpid() live

int main(void) {
    printf("Before fork: I am process %d\n", getpid());
    pid_t pid = fork();
    /* fork() is a function that creates a new process
    returns a pid_t variable named pid - returns whatever 
    type the system uses to represent pid's
    */

    if (pid < 0){
        perror("fork failed.");
    } else if (pid == 0){
        printf("PID is %d, fork returned %d to me\n", getpid(), pid);
        char *args[] = {"/bin/jailed_hello", "hello from the child process", NULL};
        int chroot_result = chroot("jail");
        if (chroot_result == -1){
            perror("Root folder not found.");
            exit(1);
        }
        int chdir_result = chdir("/");
        if (chdir_result == -1){
            perror("failed.");
            exit(1);
        }
        if (execvp(args[0], args) == -1){ //exec always returns -1 on failure since its return type is int
            perror("Execvp failed");
            exit(1);
        }
    } else {
        printf("PID is %d, fork returned %d to me\n", getpid(), pid);
        
        int status;
        pid_t wpid = waitpid(pid, &status, 0); //capture return value and compare directly to -1

        if (wpid == -1){
            perror("Waitpid failed.");
            exit(1);
        }

        if (WIFEXITED(status)) {
            printf("Child exited normally with status %d\n", WEXITSTATUS(status));
        } else {
            printf("Child did not exit normally\n");
        }
    }
    return 0; 
}
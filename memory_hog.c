#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    size_t chunk = 10 * 1024 * 1024; // 10MB per chunk
    size_t total = 0;
    while (1) {
        char *block = malloc(chunk);
        if (block == NULL) {
            printf("malloc failed at %zu MB\n", total / (1024 * 1024));
            break;
        }
        memset(block, 1, chunk); // actually touch every byte — malloc alone
                                  // doesn't count against the cgroup until
                                  // the memory is genuinely written to
        total += chunk;
        printf("Allocated %zu MB so far\n", total / (1024 * 1024));
        fflush(stdout); // force this line to print immediately, since we
                         // might get killed before the normal buffer flushes
        sleep(1);
    }
    return 0;
}
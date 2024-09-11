#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main() {
    FILE* pidFile = fopen("server.pid", "r");
    if (pidFile == NULL) {
        perror("Could not open server.pid");
        exit(EXIT_FAILURE);
    }

    pid_t pid;
    fscanf(pidFile, "%d", &pid);
    fclose(pidFile);

    if (kill(pid, SIGTERM) == 0) {
        printf("Shutdown signal sent to server (PID: %d).\n", pid);
    } else {
        perror("Failed to send shutdown signal");
        exit(EXIT_FAILURE);
    }

    return 0;
}

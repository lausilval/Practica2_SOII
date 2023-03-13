#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int main() {
    pid_t pid;
    int status;

    // creación del proceso hijo (consumidor)
    if ((pid = fork()) == -1) {
        perror("Error al crear el proceso");
        exit(EXIT_FAILURE);
        
    } else if (pid == 0) {
        // proceso hijo (consumidor)
        consumidor();
        exit(EXIT_SUCCESS);
    }

    // proceso padre (productor)
    productor();

    // esperar a que finalice el proceso hijo
    if (wait(&status) == -1) {
        perror("Error al esperar al proceso hijo");
        exit(EXIT_FAILURE);
    }

    return 0;
}


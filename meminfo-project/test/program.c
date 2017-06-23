#include <stdio.h>
#include <stdlib.h>
#include "meminfo.h"
#include <sys/mman.h>
#include <sys/shm.h>

int main(void) {

    char *ptr1 = malloc(1024*70);
    char *ptr2 = malloc(1024*1024);

    int sd = shmget(IPC_PRIVATE, 1024*1024, 0666);
    void *addr_ipc = shmat(sd, NULL, 0);
    printf("\n");

    mem_sendtoque();

    shmdt(addr_ipc);

    free(ptr1);
    free(ptr2);

}

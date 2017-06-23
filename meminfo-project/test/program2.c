#include <stdio.h>
#include <stdlib.h>
#include "meminfo.h"
#include <sys/mman.h>
#include <sys/shm.h>
#include <malloc.h>

int main(void) {

    mallopt(M_MMAP_THRESHOLD, 1024*200);
    char *ptr1 = malloc(1024*170);
    char *ptr2 = malloc(1024*300);

    int sd = shmget(IPC_PRIVATE, 1024*1024, 0666);
    void *addr_ipc = shmat(sd, NULL, 0);
    printf("\n");

    mem_sendtoque();

    shmdt(addr_ipc);

    free(ptr1);
    free(ptr2);

}

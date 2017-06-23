#ifndef MEMPROJECT_H
#define MEMPROJECT_H

#include <stdbool.h>
#include "mydef.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <pthread.h>
#include <mqueue.h>
#include <malloc.h>
#include <string.h>

#define FLOGNAME "file.log"
#define QUEUENAME "/queue"
#define SIZE sizeof(struct mallinfo)

#define MMAP 1
#define BRK 2
#define MUNMAP 3
#define SHM 4

int PAGESIZE;

#define KB(size) ((size)/1024)
#define MB(size) (size)/(1024*1024)
#define PAGE(size) (size)/PAGESIZE

typedef struct mem_info {

    FILE* log_file;
    long long int mmap_mem_start;
    long long int mmap_mem_usr;
    long long int mmap_addr[20];
    long long int brk_start;
    long long int brk_usr;
    long long int munmap_mem;
    long long int shm_mem;
    long long int brk_ptr;
    struct mallinfo *minfo;
    bool start;

} Mem_info;

Mem_info* mem_generate(char *prog_name, char* option);
void mem_sendtoque();
void mem_clean(Mem_info* mem_info);
void mem_getstats(Mem_info *mem_info);
void print_stats(Mem_info *mem_info, char *opt);

#endif
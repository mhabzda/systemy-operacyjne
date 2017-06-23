#include "meminfo.h"

/********************auxiliary functions definitions**********************/
void print_mallinfostats(struct mallinfo minfo);
void *mem_get_mallinfo(void *arg);
int get_type(char *line);
int find_index(long long int *array);
long long int get_mmap_size(char *line, Mem_info *mem_info);
int find_addr(long long int *array, long long int addr);
long long int get_munmap_size(char *line, Mem_info *mem_info);
long long int get_brk_size(char *line, long long int *brk_ptr);
long long int get_shm_size(char *line);

/*************************************************************************/

void print_stats(Mem_info *mem_info, char *option) {

    printf("---------------------mem_info---------------------   KB         MB         PAGES\n");
    long long int mem_start = mem_info->mmap_mem_start + mem_info->brk_start;
    printf("Memory for starting the process:                 %6lld%11lld%14lld\n",
           KB(mem_start), MB(mem_start), PAGE(mem_start));
    printf("Memory allocated with mmap:                      %6lld%11lld%14lld\n",
           KB(mem_info->mmap_mem_usr), MB(mem_info->mmap_mem_usr), PAGE(mem_info->mmap_mem_usr));
    printf("Memory released with munmap:                     %6lld%11lld%14lld\n",
           KB(mem_info->munmap_mem), MB(mem_info->munmap_mem), PAGE(mem_info->munmap_mem));
    printf("Memory allocated with brk:                       %6lld%11lld%14lld\n",
           KB(mem_info->brk_usr), MB(mem_info->brk_usr), PAGE(mem_info->brk_usr));
    printf("Shared memory:                                   %6lld%11lld%14lld\n\n",
           KB(mem_info->shm_mem), MB(mem_info->shm_mem), PAGE(mem_info->shm_mem));


    if(strcmp(option, "-mallinfo") != 0)
        return;

    print_mallinfostats(*mem_info->minfo);

}

Mem_info *mem_generate(char *prog_name, char* option) {

    PAGESIZE = getpagesize();
    Mem_info *mem_info = calloc(1, sizeof(Mem_info));

    pthread_t tid;
    if(strcmp(option, "-mallinfo") == 0) {
        CALL_CORRECT(pthread_create(&tid, NULL, &mem_get_mallinfo, NULL), 0, "pthread_create");
    }

    int pid = fork();
    if(pid == 0) {
        if(execlp("strace", "strace", "-e", "trace=memory,ipc",
                  "-o", FLOGNAME, prog_name, (char*)NULL) == -1) {
            exit(-1);
        }
    }

    struct mallinfo *minfo;
    if(strcmp(option, "-mallinfo") == 0) {
        CALL_CORRECT(pthread_join(tid, (void **)(&minfo)), 0, "pthread_join")
        if(minfo == NULL)
            return NULL;
    }

    int status;
    wait(&status);

    if(!WIFEXITED(status) || WEXITSTATUS(status) == -1)
        return NULL;

    FILE* log_file;
    CALL(log_file, fopen(FLOGNAME, "r"), NULL, "open");

    mem_info->log_file = log_file;
    mem_info->minfo = minfo;
    mem_info->start = true;

    return mem_info;

    EXIT_BGN
        return NULL;
    EXIT_END
}

void mem_getstats(Mem_info *mem_info) {

    char *line = NULL;
    size_t len = 0;
    while(getline(&line, &len, mem_info->log_file) != -1) {
        switch(get_type(line)) {
            case MMAP:
                if(mem_info->start)
                    mem_info->mmap_mem_start += get_mmap_size(line, mem_info);
                else
                    mem_info->mmap_mem_usr += get_mmap_size(line, mem_info);
                break;
            case MUNMAP:
                mem_info->start=false;
                mem_info->munmap_mem += get_munmap_size(line, mem_info);
                break;
            case BRK:
                if(mem_info->start)
                    mem_info->brk_start += get_brk_size(line, &mem_info->brk_ptr);
                else
                    mem_info->brk_usr += get_brk_size(line, &mem_info->brk_ptr);
                break;
            case SHM:
                    mem_info->shm_mem += get_shm_size(line);
            default:
                continue;
        }

    }

}

void mem_sendtoque() {

    mqd_t qd;
    CALL(qd, mq_open(QUEUENAME, O_WRONLY), -1, "mq_open");;

    struct mallinfo minfo;
    minfo = mallinfo();
    char *mes = (char*) &minfo;

    CALL_CHECK(mq_send(qd, mes, SIZE, 1), -1, "mq_send");

    mq_close(qd);

    EXIT_BGN
        return;
    EXIT_END
}

void mem_clean(Mem_info* mem_info) {

    if(mem_info != NULL) {
        fclose(mem_info->log_file);
        free(mem_info);
        unlink(FLOGNAME);
    }
}

/*********************auxuliary functions***********************/

void *mem_get_mallinfo(void *arg) {

    mqd_t qd;

    struct mq_attr mqattr;
    mqattr.mq_msgsize = SIZE;
    mqattr.mq_maxmsg = 2;
    CALL(qd, mq_open(QUEUENAME, O_RDWR | O_CREAT, 0666, &mqattr), -1, "mq_open");

    struct mallinfo *minfo;
    static char mes[SIZE];
    unsigned int prio;

    CALL_CHECK(mq_receive(qd, mes, SIZE, &prio), -1, "mq_receive");
    minfo = (struct mallinfo*) mes;
    mq_close(qd);
    mq_unlink(QUEUENAME);

    return (void*)(minfo);

    EXIT_BGN
        mq_unlink(QUEUENAME);
        return NULL;
    EXIT_END
}


void print_mallinfostats(struct mallinfo minfo) {

    printf("---------------------mallinfo---------------------   KB         MB         PAGES\n");
    printf("Memory allocated with mmap:                      %6d%11d%14d\n",
           KB(minfo.hblkhd), MB(minfo.hblkhd), PAGE(minfo.hblkhd));
    printf("Memory allocated with brk:                       %6d%11d%14d\n",
           KB(minfo.arena), MB(minfo.arena), PAGE(minfo.arena));
    printf("Total allocated space:                           %6d%11d%14d\n",
           KB(minfo.uordblks), MB(minfo.uordblks), PAGE(minfo.uordblks));
    printf("Total free space:                                %6d%11d%14d\n",
           KB(minfo.fordblks), MB(minfo.fordblks), PAGE(minfo.fordblks));
    printf("Top-most, releasable space:                      %6d%11d%14d\n\n",
           KB(minfo.keepcost), MB(minfo.keepcost), PAGE(minfo.keepcost));

}

int get_type(char *line) {

    if(strncmp(line, "mmap", 4) == 0)
        return MMAP;
    if(strncmp(line, "brk", 3) == 0)
        return BRK;
    if(strncmp(line, "munmap", 6) == 0)
        return MUNMAP;
    if(strncmp(line, "shmget", 6) == 0)
        return SHM;

    return -1;
}

int find_index(long long int *array) {

    for(int i=0; i<20; i++) {
        if(array[i] == 0) {
            return i;
        }
    }

    return -1;
}

long long int get_mmap_size(char *line, Mem_info *mem_info) {

    line = strpbrk(line, ",");
    line++;
    char *end;
    long long int size = strtoll(line, &end, 0);

    line = end;
    for(int i=0; i<3; i++) {
        line = strpbrk(line, ",");
        line++;
    }
    long int fd = strtol(line, &end, 0);

    line = strpbrk(end, "=");
    line++;
    long long int addr = strtoll(line , NULL, 0);

    if(addr <=0 || fd != -1) {
        return 0;
    }

    mem_info->mmap_addr[find_index(mem_info->mmap_addr)] = addr;
    return size;
}

int find_addr(long long int *array, long long int addr) {

    for(int i=0; i<20; i++) {
        if(array[i]==addr) {
            return i;
        }
    }
    return 0;
}

long long int get_munmap_size(char *line, Mem_info *mem_info) {

    line = strpbrk(line, "(");
    line++;
    char *end;
    long long int addr = strtoll(line, &end, 0);

    line = end;
    line++;
    long long int size = strtoll(line, &end, 0);

    int ind = find_addr(mem_info->mmap_addr, addr);
    if(!ind)
        return 0;

    mem_info->mmap_addr[ind] = 0;
    return size;
}

long long int get_brk_size(char *line, long long int *brk_ptr) {

    line = strpbrk(line, "(");
    line++;
    char *end;
    long long int arg = strtoll(line, &end, 0);

    line = strpbrk(end, "=");
    line++;
    long long int ptr = strtoll(line, NULL, 0);

    if(arg) {
        long long int tmp = *brk_ptr;
        *brk_ptr = ptr;
        return (ptr - tmp);
    }
    else {
        *brk_ptr = ptr;
        return 0;
    }

}

long long int get_shm_size(char *line) {

    line = strpbrk(line, ",");
    line++;
    long long int size = strtoll(line, NULL, 0);

    return size;
}


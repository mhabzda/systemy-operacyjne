#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>

void generate(char *fileName, int record, int size) {

    int randomData = open("/dev/urandom", O_RDONLY);
    if(randomData==-1) {
        perror(NULL);
        exit(1);
    }
    char *myData = malloc(record*size*sizeof(*myData));
    size_t dataRead = 0;
    size_t ssize = (size_t)(record*size);

    while(dataRead < ssize) {

        ssize_t result = read(randomData, myData, ssize - dataRead);
        if(result < 0) {
            printf("Unable to read from /dev/random\n");
            exit(1);
        }
        dataRead += result;

    }
    close(randomData);

    int myFile = open(fileName, O_RDWR | O_CREAT, 0666);
    if(myFile==-1) {
        perror(fileName);
        exit(1);
    }

    size_t dataWritten = 0;
    while(dataWritten < ssize) {

        ssize_t result = write(myFile, myData, ssize - dataWritten);
        if(result==-1) {
            perror(fileName);
            exit(1);
        }
        dataWritten += result;
    }

    free(myData);

}

void shuffle_sys(char *fileName, int record, int size) {

    int file = open(fileName, O_RDWR);
    if(file==-1) {
        perror(fileName);
        exit(1);
    }
    unsigned char *first = malloc(size*sizeof(*first));
    unsigned char *second = malloc(size*sizeof(*second));
    for(int i=record-1; i>0; i--) {
        int j = rand()%i;

        if(lseek(file, i*size, SEEK_SET)==-1)
        {
            perror(fileName);
            exit(1);
        }
        if(read(file, first, size)==-1) {
            perror(fileName);
            exit(1);
        }
        if(lseek(file, j*size, SEEK_SET)==-1) {
            perror(fileName);
            exit(1);
        }
        if(read(file, second, size)==-1) {
            perror(fileName);
            exit(1);
        }

        if(lseek(file, i*size, SEEK_SET)==-1) {
            perror(fileName);
            exit(1);
        }
        if(write(file, second, size)==-1) {
            perror(fileName);
            exit(1);
        }
        if(lseek(file, j*size, SEEK_SET)==-1) {
            perror(fileName);
            exit(1);
        }
        if(write(file, first, size)==-1) {
            perror(fileName);
            exit(1);
        }

    }
    free(first);
    free(second);

}


void sort_sys(char *fileName, int record, int size) {

    int file = open(fileName, O_RDWR);
    if(file==-1) {
        perror(fileName);
        exit(1);
    }
    unsigned char *first = malloc(size*sizeof(*first));
    unsigned char *second = malloc(size*sizeof(*second));
    for(int i=0; i < record-1; i++) {
        for(int j=0; j<record-1; j++) {
            if(lseek(file, j*size, SEEK_SET)==-1) {
                perror(fileName);
                exit(1);
            }
            if(read(file, first, size)==-1) {
                perror(fileName);
                exit(1);
            }
            if(read(file, second, size)==-1) {
                perror(fileName);
                exit(1);
            }
            if(first[0]>second[0]){
                if(lseek(file, j*size, SEEK_SET)==-1) {
                    perror(fileName);
                    exit(1);
                }
                if(write(file, second, size)==-1) {
                    perror(fileName);
                    exit(1);
                }
                if(write(file, first, size)==-1) {
                    perror(fileName);
                    exit(1);
                }
            }
        }
        if(lseek(file, 0, SEEK_SET)==-1) {
            perror(fileName);
            exit(1);
        }
    }
    free(first);
    free(second);
}

void shuffle_lib(char *fileName, int record, int size) {

    FILE *file = fopen(fileName, "r+");
    if(file==NULL) {
        perror(fileName);
        exit(1);
    }
    unsigned char *first = malloc(size*sizeof(*first));
    unsigned char *second = malloc(size*sizeof(*second));
    for(int i=record-1; i>0; i--) {
        int j = rand()%i;

        if(fseek(file, i*size, 0)==-1) {
            perror(fileName);
            exit(1);
        }
        fread(first, size, 1, file);
        if(fseek(file, j*size, 0)==-1) {
            perror(fileName);
            exit(1);
        }
        fread(second, size, 1, file);

        if(fseek(file, i*size, 0)==-1) {
            perror(fileName);
            exit(1);
        }
        fwrite(second, size, 1, file);
        if(fseek(file, j*size, 0)==-1) {
            perror(fileName);
            exit(1);
        }
        fwrite(first, size, 1, file);

    }
    free(first);
    free(second);

}

void sort_lib(char *fileName, int record, int size) {

    FILE *file = fopen(fileName, "r+");
    if(file==NULL) {
        perror(fileName);
        exit(1);
    }
    unsigned char *first = malloc(size*sizeof(*first));
    unsigned char *second = malloc(size*sizeof(*second));
    for(int i=0; i < record-1; i++) {
        for(int j=0; j<record-1; j++) {
            if(fseek(file, j*size, 0)==-1) {
                perror(fileName);
                exit(1);
            }
            fread(first, size, 1, file);
            fread(second, size, 1, file);
            if(first[0]>second[0]){
                if(fseek(file, j*size, 0)==-1) {
                    perror(fileName);
                    exit(1);
                }
                fwrite(second, size, 1, file);
                fwrite(first, size, 1, file);
            }
        }
        rewind(file);
    }
    free(first);
    free(second);
}

void get_time(struct rusage *usage, struct timeval *ts, struct timeval *tu)
{
    getrusage(RUSAGE_SELF, usage);
    *tu=usage->ru_utime;
    *ts=usage->ru_stime;
}

void print_times(struct timeval tu1, struct timeval tu2, struct timeval ts1, struct timeval ts2) {

    long int stime = ts2.tv_sec-ts1.tv_sec;
    long int utime = tu2.tv_sec-tu1.tv_sec;
    printf("Czas uzytkownika: %ld s\n", utime);
    printf("Czas systemu: %ld s\n", stime);
}

int main(int argc, char **argv) {

    srand(time(NULL));

    struct timeval *ts_before, *tu_before, *ts, *tu;
    ts_before = malloc(sizeof(*ts_before));
    tu_before = malloc(sizeof(*tu_before));
    ts = malloc(sizeof(*ts));
    tu = malloc(sizeof(*tu));

    struct rusage *usage = malloc(sizeof(*usage));

    if(argc!=6) {
        printf("Wrong number of arguments.");
        exit(1);
    }

    char *variant = argv[1];
    char *function = argv[2];
    char *fileName = argv[3];
    int record = atoi(argv[4]);
    int size = atoi(argv[5]);
    if(strcmp(variant, "sys")==0) {

        if(strcmp(function, "generate")==0) generate(fileName, record, size);
        else if(strcmp(function, "shuffle")==0) {
            get_time(usage, ts_before, tu_before);
            shuffle_sys(fileName, record, size);
            get_time(usage, ts, tu);
            printf("--Sys shuffle--\n");
            print_times(*tu_before, *tu, *ts_before, *ts);

        }
        else if(strcmp(function, "sort")==0) {
            get_time(usage, ts_before, tu_before);
            sort_sys(fileName, record, size);
            get_time(usage, ts, tu);
            printf("--Sys sort--\n");
            print_times(*tu_before, *tu, *ts_before, *ts);
        }

    }
    else if(strcmp(variant, "lib")==0) {

        if(strcmp(function, "shuffle")==0) {
            get_time(usage, ts_before, tu_before);
            shuffle_lib(fileName, record, size);
            get_time(usage, ts, tu);
            printf("--Lib shuffle--\n");
            print_times(*tu_before, *tu, *ts_before, *ts);

        }
        else if(strcmp(function, "sort")==0) {
            get_time(usage, ts_before, tu_before);
            sort_lib(fileName, record, size);
            get_time(usage, ts, tu);
            printf("--Lib sort--\n");
            print_times(*tu_before, *tu, *ts_before, *ts);
        }

    }

    free(usage);
    free(ts_before);
    free(tu_before);
    free(ts);
    free(tu);
    return 0;

}



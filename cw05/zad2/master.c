#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char** argv) {

    if(argc!=3) {
        printf("Wrong number of arguments.\n");
        return 1;
    }

    int R = atoi(argv[2]);
    int **T = calloc((size_t) R, sizeof(int*));
    for(int i=0; i<R; i++) {
        T[i] = calloc((size_t) R, sizeof(int));
    }

    if(mkfifo(argv[1], S_IRUSR | S_IWUSR) == -1) {
        perror("Error with mkfifo");
        return 1;
    }

    FILE *fd = fopen(argv[1], "r");
    if(fd == NULL) {
        perror("Error with open");
        return 1;
    }

    char *line=NULL;
    size_t n = 0;

    double x, y;
    int itr;
    int c=0;

    while(c!='X') {
        while (getline(&line, &n, fd) != -1) {
            char *token = strtok(line, " \n");
            sscanf(token, "%lf", &x);
            token = strtok(NULL, " \n");
            sscanf(token, "%lf", &y);
            token = strtok(NULL, " \n");
            itr = atoi(token);

            int ix = (int) ((x + 2) * (R / 3));
            int iy = (int) ((y + 1) * (R / 2));

            if(ix<600 && iy<600) T[ix][iy] = itr;

            free(line);
            line = NULL;
        }
        free(line);
        line=NULL;
        c=getc(stdin);
    }

    fclose(fd);

    fd = fopen("data", "w+");
    if(fd == NULL) {
        perror("Error with open");
        return 1;
    }
    for(int i=0; i<R; i++) {
        for(int j=0; j<R; j++) {
            fprintf(fd, "%d %d %d\n", i, j, T[i][j]);
        }
    }

    fclose(fd);

    for(int i=0; i<R; i++) {
        free(T[i]);
    }
    free(T);


    FILE *gnu = popen("gnuplot", "w");
    if(gnu==NULL) {
        printf("Error with popen\n");
        return 1;
    }

    fputs("set view map\n", gnu);
    fprintf(gnu, "set xrange [0:%d]\n", R);
    fprintf(gnu, "set yrange [0:%d]\n", R);
    fputs("plot \'data\' with image\n", gnu);

    fflush(gnu);
    while((c = getc(stdin)) !='\n' && c != EOF);          //czyszczenie bufora
    getc(stdin);
    pclose(gnu);

    return 0;
}


#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

typedef struct complex {
    double re;
    double im;
} Complex;

Complex f(Complex z, Complex c) {
    Complex z2;
    z2.re = (z.re * z.re - z.im * z.im) + c.re;
    z2.im = 2 * z.re * z.im + c.im;
    return z2;
}

double absolute(Complex z) {
    return sqrt(z.re * z.re + z.im * z.im);
}

int iters(Complex c, int K) {

    int i=0;
    Complex z;
    z.re=0;
    z.im=0;
    while(i<K && absolute(z)<=2.0) {
        z = f(z, c);
        i++;
    }
    return i;
}

int size(char *array) {
    int i;
    for(i=0; array[i]!='\0'; i++);
    return i;
}

int main(int argc, char **argv) {

    if(argc != 4) {
        printf("Wrong number of arguments.\n");
        return 1;
    }

    srand((unsigned) time(NULL));

    int N = atoi(argv[2]);
    int K = atoi(argv[3]);

    int fd = open(argv[1], O_WRONLY);
    if(fd ==-1) {
        perror("Error with open");
        return 1;
    }

    double x, y;
    for(int i=0; i<N; i++) {
        /*if(rand()%2==0) {
            x = - ((double)rand()/(double)RAND_MAX) * 2.00000;
        }
        else {
            x = ((double)rand()/(double)RAND_MAX);
        }*/
        x = ((double)rand()/(double)RAND_MAX) * 3.00000 - 2.0;

        if(rand()%2==0) {
            y = - ((double)rand()/(double)RAND_MAX);
        }
        else {
            y = ((double)rand()/(double)RAND_MAX);
        }

        Complex c;
        c.re = x;
        c.im = y;
        int itr = iters(c, K);

        char line[50];
        snprintf(line, 50, "%lf %lf %d\n", x, y, itr);

        if(write(fd, line, (size_t) size(line))==-1) {
            perror("Error with write");
            return 1;
        }
    }

    close(fd);
    return 0;
}

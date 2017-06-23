#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {

    if(argc!=2) {
        printf("Bledna liczba argumentow\n");
        exit(1);
    }

    if(getenv(argv[1])!=NULL)
        printf("Odczyt zmiennej %s: %s\n", argv[1], getenv(argv[1]));
    else
        printf("Podana zmienna nie istnieje\n");

    return 0;
}


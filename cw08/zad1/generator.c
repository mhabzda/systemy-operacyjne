#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char j='0', d='0';

char *generateRecord(){
    char *str = malloc(1024*sizeof(char));
    str[0] = d;
    str[1] = j;
    if(j=='9') {
        d++;
        j='0';
    }
    else
        j++;

    for(int i = 2; i < 1024; i++){
        str[i] = (char) ((rand()%('z'-'a'))+'a');
    }
    return str;
}

int main(int argc, char **argv){

    int n = atoi(argv[2]);
    char *name = argv[1];
    FILE *fh;

    if((fh = fopen(name, "w+"))==NULL){
        perror("fopen");
        return 1;
    }
    srand((unsigned) time(NULL));
    for(int i=0; i<n; i++){
        char *str = generateRecord();
        fwrite(str, sizeof(char), 1024, fh);
        free(str);
    }

    fclose(fh);

    return 0;
}


#ifndef CS_H
#define CS_H

#define MAXSIZE 256

#define PROJ 4
#define PROJ2 11
#define PROJ3 14
#define PROJ4 18
#define PROJ5 23
#define PROJ6 34
#define PROJ7 45
#define PROJ8 48
#define PROJ9 51
#define PROJ10 56
#define PROJ11 59

#define ECHO 1
#define CAPS 2
#define TIME 3
#define END 4

typedef struct mymsg {
    long mtype;
    int mpid;
    char mtext[MAXSIZE-sizeof(int)];
} mymsg;

#endif

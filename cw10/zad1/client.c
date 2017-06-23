#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

int sock_fd;

typedef struct {
    char name[20];
    char type;
    unsigned char value[3];
} msg;

int create_net_socket(char* ip_adress, uint16_t port_number) {
    int sock_fd;
    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port_number);

    int res;
    if((res = inet_pton(AF_INET, ip_adress, &sock_addr.sin_addr))==0) {
        printf("Wrong adress\n");
        exit(1);
    }
    else if(res == -1) {
        perror("inet_pton");
        exit(1);
    }

    if(connect(sock_fd, (const struct sockaddr *) &sock_addr, sizeof(sock_addr)) == -1) {
        perror("connect");
        exit(1);
    }

    return sock_fd;
}

int create_unix_socket(char *path) {
    int sock_fd;
    if((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    struct sockaddr_un sock_addr;
    sock_addr.sun_family = AF_UNIX;
    strcpy(sock_addr.sun_path, path);
    if(connect(sock_fd, (const struct sockaddr *) &sock_addr, sizeof(sock_addr)) == -1) {
        perror("connect");
        exit(1);
    }

    return sock_fd;
}

unsigned char operation(unsigned char x, unsigned char z, unsigned char y) {
    if(z=='+')
        return x + y;
    if(z=='-')
        return x - y;
    if(z=='*')
        return x * y;
    if(z=='/')
        return x / y;
    return 0;
}

void handler(int signum) {
    if(signum==SIGINT) {
        exit(0);
    }
}

void shut(void) {
    shutdown(sock_fd, SHUT_RDWR);
    close(sock_fd);
}

int main(int argc, char **argv) {

    if(argc==4 && strcmp(argv[2], "local")==0) {
        sock_fd = create_unix_socket(argv[3]);
    }
    else if(argc==5 && strcmp(argv[2], "network")==0) {
        sock_fd = create_net_socket(argv[3], (uint16_t) atoi(argv[4]));
    }
    else {
        printf("Wrong arguments\n");
        exit(1);
    }

    struct sigaction act;
    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    sigaction(SIGINT, &act, NULL);
    atexit(&shut);

    char name[20];
    strcpy(name, argv[1]);
    printf("%s\n", name);

    msg message;
    message.type=1;
    strcpy(message.name, name);

    if(send(sock_fd, &message, sizeof(message), 0) == -1) {
        perror("send");
        exit(1);
    }

    int b=1;
    unsigned char x, y, z, res;
    while(b) {
        memset(&message, 0, sizeof(message));
        read(sock_fd, &message, sizeof(message));

        if(message.type==1) {
            printf("%s\n", message.name);
            exit(1);
        }
        else if(message.type==2) {
            x = message.value[0];
            z = message.value[1];
            y = message.value[2];
            res = operation(x, z, y);

            memset(&message, 0, sizeof(message));
            message.type = 2;
            strcpy(message.name, name);
            message.value[0] = res;
            send(sock_fd, &message, sizeof(message), 0);
        }
    }

}

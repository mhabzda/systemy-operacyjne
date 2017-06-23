#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

char clients[15][20];
int socknet_fd, sockunix_fd;
int sockets[15];
int num, c;

typedef struct {
    char name[20];
    char type;
    unsigned char value[3];
} msg;

int create_net_socket(uint16_t port_number) {
    int sock_fd;
    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port_number);
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sock_fd, (const struct sockaddr *) &sock_addr, sizeof(sock_addr)) == -1) {
        perror("bind");
        exit(1);
    }
    if(listen(sock_fd, 15)==-1) {
        perror("listen");
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
    if(bind(sock_fd, (const struct sockaddr *) &sock_addr, sizeof(sock_addr)) == -1) {
        perror("bind");
        exit(1);
    }
    if(listen(sock_fd, 15)==-1) {
        perror("listen");
        exit(1);
    }

    return sock_fd;
}

int client_nexits(char *client) {
    for(int i=0; i<15; i++) {
        if(strcmp(clients[i], client)==0)
            return 0;
    }
    return 1;
}

int clients_nonempty() {
    for(int i=0; i<15; i++) {
        if(clients[i][0] != '\0')
            return 1;
    }
    return 0;
}

void *network(void *arg) {

    int socket_max = socknet_fd+1;
    if(sockunix_fd>=socket_max)
        socket_max = sockunix_fd + 1;

    int b2=1, b=0;
    fd_set set;
    msg message;
    while(b2) {
        FD_ZERO(&set);
        FD_SET(sockunix_fd, &set);
        FD_SET(socknet_fd, &set);
        for(int j=0; j<num; j++) {
            FD_SET(sockets[j], &set);
        }

        if(select(socket_max, &set, NULL, NULL, NULL)==-1) {
            perror("select");
            exit(1);
        }

        if(FD_ISSET(socknet_fd, &set)) {
            sockets[num] = accept(socknet_fd, NULL, NULL);
            if(sockets[num] == -1) {
                perror("accept");
                exit(1);
            }

            b=0;
            if(socket_max<=sockets[num]) {
                socket_max = sockets[num] + 1;
            }
            num++;
        }

        if(FD_ISSET(sockunix_fd, &set)) {
            sockets[num] = accept(sockunix_fd, NULL, NULL);
            if(sockets[num] == -1) {
                perror("accept");
                exit(1);
            }

            b=0;
            if(socket_max<=sockets[num]) {
                socket_max = sockets[num] + 1;
            }
            num++;
        }

        for(int j=0; j<num; j++) {
            if(FD_ISSET(sockets[j], &set)) {
                memset(&message, 0, sizeof(message));
                read(sockets[j], &message, sizeof(message));
                if(message.type==1) {
                    if(client_nexits(message.name)) {
                        strcpy(clients[c], message.name);
                        printf("%s\n", clients[c]);
                        c++;
                        if(c==1) b=1;
                    }
                    else {
                        memset(&message, 0, sizeof(message));
                        message.type = 1;
                        strcpy(message.name, "Name already exists");
                        send(sockets[j], &message, sizeof(message), 0);
                        num--;
                    }
                }
                else {
                    printf("%s - result: %hhu\n", message.name, message.value[0]);
                    b=1;
                }
            }
        }

        if(clients_nonempty() && b) {
            int x, y;
            unsigned char z;
            printf("Enter operation:\n");
            scanf("%d %c %d", &x, &z, &y);
            memset(&message, 0, sizeof(message));
            message.type=2;
            message.value[0] = (unsigned char) x;
            message.value[1] = z;
            message.value[2] = (unsigned char) y;
            if(num!=0) {
                if (send(sockets[rand() % num], &message, sizeof(message), MSG_NOSIGNAL) == -1) {
                    perror("send");
                }
            }
        }
    }

    return NULL;
}

void move_clients(int i) {
    for(int j=i; j<14; j++) {
        sockets[j]=sockets[j+1];
        strcpy(clients[j], clients[j+1]);
        memset(clients[j+1], 0, 20);
    }

}

void *ping(void *arg) {

    msg message;
    message.type=3;
    int b=1;
    while(b) {
        for (int i = 0; i < 15; i++) {
            if (sockets[i] != 0 && clients[i][0]!=0)
                if (send(sockets[i], &message, sizeof(message), MSG_NOSIGNAL) == -1) {
                    printf("Client %s disconnected\n", clients[i]);
                    sockets[i] = 0;
                    memset(clients[i], 0, 20);
                    move_clients(i);
                    c--;
                    num--;
                }
        }
    }

    return NULL;
}

void handler(int signum) {
    if(signum==SIGINT) {
        shutdown(socknet_fd, SHUT_RDWR);
        shutdown(sockunix_fd, SHUT_RDWR);
        close(socknet_fd);
        close(sockunix_fd);
        exit(0);
    }
}

int main(int argc, char **argv) {

    if(argc!=3) {
        printf("Wrong number of arguments\n");
        return 1;
    }

    struct sigaction act;
    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    sigaction(SIGINT, &act, NULL);

    uint16_t port_number = (uint16_t) atoi(argv[1]);
    char *socket_path = argv[2];

    socknet_fd = create_net_socket(port_number);
    sockunix_fd = create_unix_socket(socket_path);

    pthread_t tids[2];
    pthread_create(&tids[0], NULL, &network, NULL);
    pthread_create(&tids[1], NULL, &ping, NULL);

    void *status;
    pthread_join(tids[0], &status);
    pthread_join(tids[1], &status);

    return 0;
}

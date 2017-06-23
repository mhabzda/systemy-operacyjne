#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

char clients_net[7][20];
char clients_unix[7][20];
int socknet_fd, sockunix_fd;

struct sockaddr_in net_clients[7];
struct sockaddr_un unix_clients[7];

int c_net, c_unix, in, un;

typedef struct {
    char name[20];
    char type;
    unsigned char value[3];
} msg;

int create_net_socket(uint16_t port_number) {
    int sock_fd;
    if((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
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

    return sock_fd;
}

int create_unix_socket(char *path) {
    int sock_fd;
    if((sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
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

    return sock_fd;
}

int client_nexits(char *client) {
    for(int i=0; i<7; i++) {
        if(strcmp(clients_net[i], client)==0)
            return 0;
    }
    for(int i=0; i<7; i++) {
        if(strcmp(clients_unix[i], client)==0)
            return 0;
    }
    return 1;
}

int clients_nonempty() {
    for(int i=0; i<7; i++) {
        if(clients_net[i][0] != '\0')
            return 1;
    }
    for(int i=0; i<7; i++) {
        if(clients_unix[i][0] != '\0')
            return 1;
    }
    return 0;
}

void *network(void *arg) {

    int socket_max = socknet_fd + 1;
    if(sockunix_fd>=socket_max)
        socket_max = sockunix_fd + 1;

    int b2=1, b=0;
    fd_set set;
    msg message;
    int sock;

    struct sockaddr_in addr_net;
    memset(&addr_net, 0, sizeof(addr_net));
    socklen_t len_net = sizeof(addr_net);
    struct sockaddr_un addr_unix;
    memset(&addr_unix, 0, sizeof(addr_unix));
    socklen_t len_unix = sizeof(addr_unix);

    while(b2) {
        FD_ZERO(&set);
        FD_SET(sockunix_fd, &set);
        FD_SET(socknet_fd, &set);

        if(select(socket_max, &set, NULL, NULL, NULL)==-1) {
            perror("select");
            exit(1);
        }

        for(int j=0; j<2; j++) {
            if(j==0)
                sock = socknet_fd;
            else
                sock = sockunix_fd;
            if(FD_ISSET(sock, &set)) {
                memset(&message, 0, sizeof(message));
                if(j==0)
                    recvfrom(sock, &message, sizeof(message), 0, (struct sockaddr *) &addr_net, &len_net);
                else
                    recvfrom(sock, &message, sizeof(message), 0, (struct sockaddr *) &addr_unix, &len_unix);
                if(message.type==1) {
                    if(client_nexits(message.name)) {
                        if (j == 0) {
                            net_clients[in] = addr_net;
                            in++;
                            strcpy(clients_net[c_net], message.name);
                            printf("%s\n", clients_net[c_net]);
                            c_net++;
                        } else {
                            unix_clients[un] = addr_unix;
                            un++;
                            strcpy(clients_unix[c_unix], message.name);
                            printf("%s\n", clients_unix[c_unix]);
                            c_unix++;
                        }

                        if((c_net==1 && c_unix!=1) || (c_unix==1 && c_net!=1)) b=1;
                        else b=0;
                    }
                    else {
                        memset(&message, 0, sizeof(message));
                        message.type = 1;
                        strcpy(message.name, "Name already exists");
                        if(j==0)
                            sendto(sock, &message, sizeof(message), 0, (const struct sockaddr *) &addr_net, len_net);
                        else
                            sendto(sock, &message, sizeof(message), 0, (const struct sockaddr *) &addr_unix, len_unix);
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
            if(c_net!=0 || c_unix!=0) {
                if(in!=0) {
                    sendto(socknet_fd, &message, sizeof(message), MSG_NOSIGNAL,
                           (const struct sockaddr *) &net_clients[rand() % in], len_net);
                }
                else if(un!=0) {
                    if(sendto(sockunix_fd, &message, sizeof(message), MSG_NOSIGNAL,
                           (const struct sockaddr *) &unix_clients[rand() % un], len_unix)==-1) {
                        perror("send");
                    }
                }
            }
        }
    }

    return NULL;
}

void move_clients(int i, int flag) {
    for(int j=i; j<6; j++) {
        if(flag) {
            memcpy(&net_clients[j], &net_clients[j + 1], sizeof(struct sockaddr_in));
            strcpy(clients_net[j], clients_net[j+1]);
            memset(clients_net[j+1], 0, 20);
        }
        else {
            memcpy(&unix_clients[j], &unix_clients[j + 1], sizeof(struct sockaddr_un));
            strcpy(clients_unix[j], clients_unix[j + 1]);
            memset(clients_unix[j + 1], 0, 20);
            printf("%s\n", clients_unix[j]);
        }
    }

}

void *ping(void *arg) {

    msg message;
    message.type=3;
    socklen_t len_net = sizeof(struct sockaddr_in);
    socklen_t len_unix = sizeof(struct sockaddr_un);

    int b=1;
    while(b) {
        for (int i = 0; i < 7; i++) {
            if (net_clients[i].sin_port != 0) {
                if (sendto(socknet_fd, &message, sizeof(message), 0,
                           (const struct sockaddr *) &net_clients[i], len_net) == -1) {
                    printf("Client %s disconnected\n", clients_net[i]);
                    memset(clients_net[i], 0, 20);
                    move_clients(i, 1);
                    c_net--;
                    in--;
                }
            }
        }
        for (int i = 0; i < 7; i++) {
            if (unix_clients[i].sun_family!=0) {
                if (sendto(sockunix_fd, &message, sizeof(message), MSG_NOSIGNAL,
                           (const struct sockaddr *) &unix_clients[i], len_unix) == -1) {
                    printf("Client %s disconnected\n", clients_unix[i]);
                    memset(clients_unix[i], 0, 20);
                    move_clients(i, 0);
                    c_unix--;
                    un--;
                }
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


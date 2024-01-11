#if 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#define MAXFD 10

int socket_init();

void fds_init(struct pollfd fds[]) {

    for (int i = 0; i < MAXFD; i++) {
        fds[i].fd = -1;
        fds[i].events = 0;
        fds[i].revents = 0;
    }
}

void fds_add(struct pollfd fds[], int fd) {

    for (int i = 0; i < MAXFD; i++) {

        if (fds[i].fd == -1) {

            fds[i].fd = fd;
            fds[i].events = POLLIN;
            fds[i].revents = 0;
            break;
        }
    }
}

void fds_del(struct pollfd fds[], int fd) {
    for (int i = 0; i < MAXFD; i++) {
        if (fds[i].fd == fd) {
            fds[i].fd = -1;
            fds[i].events = 0;
            fds[i].revents = 0;
            break;
        }
    }
}

void accept_client(int sockfd, struct pollfd fds[]) {
    int c = accept(sockfd, NULL, NULL);
    if (c < 0) {
        return;
    }

    printf("accept c=%d\n", c);
    fds_add(fds, c);
}

void recv_data(int c, struct pollfd fds[]) {
    char buff[128] = {0};
    int num = recv(c, buff, 127, 0);
    if (num <= 0) {
        fds_del(fds, c);
        close(c);
        return;
    }

    printf("recv:%s\n", buff);
    send(c, "ok", 2, 0);
}

int main() {
    int sockfd = socket_init();
    if (sockfd == -1) {
        exit(1);
    }

    struct pollfd fds[MAXFD]; // 收集描述符
    fds_init(fds);            // 空
    fds_add(fds, sockfd);

    while (1) {
        int n = poll(fds, MAXFD, 5000); //-1一直阻塞，0 立即返回,
        if (n == -1) {
            printf("poll err\n");
        }
        else if (n == 0) {
            printf("time out\n");
        }
        else {
            for (int i = 0; i < MAXFD; i++) {
                if (fds[i].fd == -1) {
                    continue;
                }

                if (fds[i].revents & POLLIN) {
                    //判断是监听套接字还是其他描述符
                    if (fds[i].fd == sockfd) {
                        // accept
                        accept_client(sockfd, fds);
                    }
                    else {
                        // recv
                        recv_data(fds[i].fd, fds);
                    }
                }
            }
        }
    }
}

int socket_init() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        return -1;
    }

    struct sockaddr_in saddr; // ip port
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(6000);
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int res = bind(sockfd, (struct sockaddr *)&saddr, sizeof(saddr));
    if (res == -1) {
        printf("bind err\n");
        return -1;
    }

    if (listen(sockfd, 5) == -1) {
        return -1;
    }

    return sockfd;
}

#endif
/*
#include <ctype.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <poll.h>

#define MAXFD 10

int socket_init();

void fds_init(struct pollfd fds[]) {
    for (int i = 0; i < MAXFD; i++) {
        fds[i].fd = -1;
        fds[i].events = 0;
        fds[i].revents = 0;
    }
}

void fds_add(struct pollfd fds[], int fd) {
    for (int i = 0; i < MAXFD; i++) {
        if (fds[i].fd == -1) {
            fds[i].fd = fd;
            fds[i].events = POLLIN;
            fds[i].revents = 0;
            break;
        }
    }
}

void fds_del(struct pollfd fds[], int fd) {
    for (int i = 0; i < MAXFD; i++)
    {
        if (fds[i].fd == fd)
        {
            fds[i].fd = -1;
            fds[i].events = 0;
            fds[i].revents = 0;
            break;
        }
    }
}

void accept_client(int sockfd, struct pollfd fds[]) {
    int c = accept(sockfd, NULL, NULL);
    //检错
    printf("accepr:%d", c);
    fds_add(fds, c);
}

void recv_data(int c, struct pollfd fds[]) {
    char buff[128] = {0};
    int num = recv(c, buff, 127, 0);
    if (num <= 0) {
        fds_del(fds, c);
        close(c);
        return;
    }

    printf("recv : %s\n", buff);
}


int main() {
    int sockfd = socket_init();
    if(sockfd == -1) {
        exit(1);
    }

    struct pollfd fds[MAXFD];
    fds_init(fds);
    fds_add(fds, sockfd);

    while (1) {
        int n = poll(fds, MAXFD, 5000);//-1一直阻塞，0立即返回

        if (n == -1 || n == 0) {

        }
        else {
            for (int i = 0; i < MAXFD; i++) {
                if (fds[i].fd == -1) {
                    continue;
                }
                
                if (fds[i].revents & POLLIN) {
                    if (fds[i].fd == sockfd) {
                        //accept
                        accept_client(sockfd, fds);
                    }
                    else {
                        //recv
                        recv_data(sockfd, fds);
                    }
                }
            }
        }
    }
}

int socket_init() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // tcp
    if (sockfd == -1)
    {
        return -1;
    }

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(6000);              // 网络字节序列
    saddr.sin_addr.s_addr = htonl(INADDR_ANY); // inet_addr("8.137.79.153");//"127.0.0.1"

    int res = bind(sockfd, (struct sockaddr *)&saddr, sizeof(saddr));
    if (res == -1)
    {
        printf("bind err\n");
        return -1;
    }

    res = listen(sockfd, 5);
    if (res == -1)
    {
        return -1;
    }

    return sockfd; // 创建的套接字返回
}
*/
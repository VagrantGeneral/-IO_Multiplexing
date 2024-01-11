#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define MAXFD 10

int socket_init();

void epoll_add(int epfd, int fd) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        printf("epoll add err\n");
    }
}

void epoll_del(int epfd, int fd) {
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) == -1) {
        printf("epoll del err\n");
    }
}

void accept_client(int sockfd, int epfd) {
    int c = accept(sockfd, NULL, NULL);
    if (c < 0) {
        return;
    }

    printf("accept c=%d\n", c);
    epoll_add(epfd, c);
}

void recv_data(int c, int epfd) {
    char buff[128] = {0};
    int num = recv(c, buff, 127, 0);
    if (num <= 0) {
        epoll_del(epfd, c);
        close(c);
    }

    printf("recv:%s\n", buff);
    send(c, "ok", 2, 0);
}

int main() {
    int sockfd = socket_init();
    if (sockfd == -1) {
        exit(1);
    }

    int epfd = epoll_create(MAXFD); // 创建内核事件表--红黑树
    if (epfd == -1) {
        exit(1);
    }

    epoll_add(epfd, sockfd); // 向内核事件表添加描述符

    struct epoll_event evs[MAXFD];

    while (1) {
        int n = epoll_wait(epfd, evs, MAXFD, 5000); // 获取就绪描述符
        if (n == -1) {
            printf("epoll wait err\n");
        }
        else if (n == 0) {
            printf("time out\n");
        }
        else {
            for (int i = 0; i < n; i++) {
                if (evs[i].events & EPOLLIN) {
                    if (evs[i].data.fd == sockfd) {
                        // accept
                        accept_client(sockfd, epfd);
                    }
                    else {
                        // recv
                        recv_data(evs[i].data.fd, epfd);
                    }
                }

                // if ( evs[i].events & EPOLLOUT)
                //{
                // }
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

#include "csapp.h"

void echo(int connfd);

int main(int argc, char **argv){
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;     /* Enough space for any address */
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if(argc != 2){ // 받은 argc가 2개가 아니라면 잘못된 입력
        fprintf(stderr, "usage: %s <port>\n", argv[0]); // 에러 메시지 출력
        exti(0); // 프로세스 종료
    }

    listenfd = Open_listenfd(argv[1]);
    while(1){
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE,
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        echo(connfd);
        Close(connfd);
    }
    exit(0); // 프로세스 종료
}
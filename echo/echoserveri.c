#include "csapp.h"

void echo(int connfd){ // echo 함수
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0){ // 리턴 코드 0을 받으면 종료
        printf("server received %d bytes\n", (int)n);
        Rio_writen(connfd, buf, n);
    }
}

int main(int argc, char **argv){
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;     /* Enough space for any address */ // accecpt로 보내지는 소켓 주소 구조체
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if(argc != 2){ // 받은 argc가 2개가 아니라면 잘못된 입력 (파일 경로, port)
        fprintf(stderr, "usage: %s <port>\n", argv[0]); // 에러 메시지 출력
        exit(0); // 프로세스 종료
    }

    listenfd = Open_listenfd(argv[1]); // 입력받은 port로 연결 요청 받을 준비 (듣기 식별자 오픈)
    while(1){ // 무한루프
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE,
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port); // 연결 성공 출력
        echo(connfd);
        Close(connfd);
    }

    exit(0); // 프로세스 종료
}
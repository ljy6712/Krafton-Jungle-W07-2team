#include "csapp.h"
 
void echo(int connfd);
 
int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; /* Enough space for any address */
    char client_hostname[MAXLINE], client_port[MAXLINE];
 
    if (argc != 2) //인자의 개수가 2개가 아닐때
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
 
    listenfd = Open_listenfd(argv[1]); //받은 포트번호로 듣기 소켓 열기
    while (1)
    {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //반복적으로 accept()를 실행해 연결을 받음
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, //소켓 주소 구조체를 대응되는 호스트와 서비스 이름을 스트링을 변환
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port); //변환된 호스트 이름과 포트번호 출력
        echo(connfd); 
        Close(connfd);
    }
    exit(0);
}
void echo(int connfd)
{   
    size_t n;
    rio_t rio;
    char buf[MAXLINE];
    rio_readinitb(&rio, connfd); //rio를 connfd로 초기화
    while ((n = rio_readlineb(&rio, buf, MAXLINE))!=NULL){ //클라이언트가 전송한 텍스트 읽어오기
        if(strcmp(buf, "\r\n") == 0) break;
        printf("server received %d bytes\n", (int)n); //읽어온 텍스트 바이트 수 출력
        Rio_writen(connfd, buf, n); //connfd로 buf의 내용을 n바이트만큼 전송
    }
}
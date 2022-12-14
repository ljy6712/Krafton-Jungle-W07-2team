#include "csapp.h"
 
int main(int argc, char **argv)
{
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;
 
    if (argc != 3) //인자가 3개가 아닐때
    {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]); //프로그램 이름
        exit(0);
    }
    host = argv[1]; //호스트
    port = argv[2]; //포트 번호
 
    clientfd = Open_clientfd(host, port); //클라이언트 소켓 생성
    Rio_readinitb(&rio, clientfd); //clientfd로 rio를 초기화
 
    while (Fgets(buf, MAXLINE, stdin) != NULL) //클라이언트가 텍스트줄을 입력함 fgets가 EOF 표준 입력을 받으면 종료
    {
        Rio_writen(clientfd, buf, strlen(buf)); //buf에서 clientfd로 strlen(buf) 바이트 만큼 전송
        Rio_readlineb(&rio, buf, MAXLINE); //rio에서 읽고 buf로 복사하고 텍스트라인을 NULL 문자로 종료시킴
        Fputs(buf, stdout); //buf가 가진 문자열을 표준 출력으로 표시함
    }
    Close(clientfd); //프로세스가 종료될때 커널이 알아서 fd를 모두 닫아주지만 올바른 프로그래밍 습관을 위해 직접 닫아줌
    exit(0);
}

#include "csapp.h"

int main(int argc, char **argv){
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if(argc != 3){ // 받은 argc가 3개가 아니라면 잘못된 입력 (파일 경로, host, port)
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0); // 프로세스 종료
    }
    host = argv[1]; // 두번째 받은 값 host로 저장
    port = argv[2]; // 세번째 받은 값 port로 저장

    clientfd = Open_clientfd(host, port); // 받은 host와 port에 연결 요청 듣고 있는 서버와 연결 설정 (반환값 IP 주소)
    Rio_readinitb(&rio, clientfd);

    while(Fgets(buf, MAXLINE, stdin) != NULL){
        Rio_writen(clientfd, buf, strlen(buf));
        Rio_readlineb(&rio, buf, MAXLINE);
        Fputs(buf, stdout);
    }
    Close(clientfd); // 열었던 모든 식별자들을 명시적으로 닫아주는 함수(불필요함)
    exit(0); // 프로세스 종료
}
#include "csapp.h"

// int argc = main()함수에 전달되는 데이터 갯수를 의미함
// char **argv = char *argv[]와 같으며 전달된 실제 인수 값 첫번째는 무조건 파일의 경로
int main(int argc, char **argv)
{
    struct addrinfo *p, *listp, hints; // addrinfo 구조체 생성
    char buf[MAXLINE];
    int rc, flags;

    if(argc != 2){ // 만약 argc가 2개가 아니라는 것은 잘못된 입력이므로 에러 출력
        fprintf(stderr, "usage: %s <domain name>\n", argv[0]); // 에러 메시지 출력
        exit(0); // 프로세스 정상 종료
    }

    /* Get a list of addrinfo records */
    memset(&hints, 0, sizeof(struct addrinfo)); // addrinfo의 크기 만큼 hints주소의 값 0으로 초기화
    hints.ai_family = AF_INET;          /* IPv4 only */ // IPv4 제한
    hints.ai_socktype = SOCK_STREAM;    /* Connections only */ // hints를 소켓 끝점으로
    //getaddrinfo(host, service, addrinfo, result)
    if((rc = getaddrinfo(argv[1], NULL, &hints, &listp)) != 0){ // 성공시 0을 반환하므로 0이 아니면 실패
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc)); // 에러 메시지 출력
        exit(1); // 에러로 인한 프로세스 종료
    }

    /* Walk the list and display each IP address */
    flags = NI_NUMERICHOST; /* Display address string instead of domain name */ // 호스트를 도메인 이름으로 변환하는 대신 IP주소 스트링으로 대신함
    for(p = listp; p; p = p->ai_next){ // 다음 addrinfo의 포인터로 이동
        Getnameinfo(p->ai_addr, p->ai_addrlen, buf, MAXLINE, NULL, 0, flags); // 인자값(소켓주소, 소켓 크기, host, host 크기, service, service 크기, flags)
        printf("%s\n", buf);
    }

    /* Clean up */
    Freeaddrinfo(listp); // 사용한 메모리 반환

    exit(0); // 프로세스 정상 종료
}
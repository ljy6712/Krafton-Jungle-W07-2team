#include "csapp.h"


// int argc =main()함수에 전달되는 데이터 갯수를 의미함 
// char **argv
int main(int argc, char** argv) {

    struct addrinfo* p, * listp, hints; // addrinfo 구조체 생성
    char buf[MAXLINE];
    int rc, flags;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <domain name>\n")


    }



}
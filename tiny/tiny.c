/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize, char *version, char *method);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs, char *version);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

int main(int argc, char **argv) {
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);  // line:netp:tiny:accept
    Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);   // line:netp:tiny:doit
    Close(connfd);  // line:netp:tiny:close
  }
}


void doit(int fd){ /* 응답 함수 */
    int is_static; // 정적 파일인지 아닌지 판단해주는 함수
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    /* Read request lune and headers */
    Rio_readinitb(&rio, fd);
    // 요청 라인을 읽고 분석
    Rio_readlineb(&rio, buf, MAXLINE); // rio_readlineb함수로 요청 라인 읽기
    printf("Request headers:\n");
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);

    // 클라이언트가 다른 메소드를 요청했을 경우
    // if(strcasecmp(method, "GET")){ // method == "GET" 이면 0이라 if문 넘어감
    //     clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method"); // method != "GET" 에러 메시지 출력
    //     return; // main으로 돌아감
    // }

    if(strcasecmp(method, "GET") && strcasecmp(method, "HEAD")){ // method == "GET" 이면 0이라 if문 넘어감
        clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method"); // method != "GET" 에러 메시지 출력
        return; // main으로 돌아감
    }

    read_requesthdrs(&rio); // 요청을 읽어들임(다른 요청 헤더들은 무시)

  /* Parse URI from GET request */
    is_static = parse_uri(uri, filename, cgiargs); // URI를 파일 이름과 비어 있을 수도 있는 CGI 인자 스트링으로 분석(정적인지 동적인지)
    if(stat(filename, &sbuf) < 0){ // (filename = 파일 경로) 파일이 디스크 상에 있지 않으면
        clienterror(fd, filename, "404", "Not found", "Tiny couldn't read the file"); // 에러 메시지
        return;
    }
  
    if(is_static){ /* Serve static content */ // 요청이 정적 컨텐츠를 위한 것이라면
        if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)){ // 이 파일이 보통 파일이 아니거나 읽기 권한이 없을 경우
        clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file"); // 에러 메시지
        return;
        }

        serve_static(fd, filename, sbuf.st_size, version, method); // 클라이언트에게 정적 컨텐츠를 제공

    }else{ /* Serve dynamic content */ // 요청이 동적 컨텐츠를 위한 것이라면
        if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)){ // 이 파일이 실행 가능한지 검증
        clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file"); // 에러 메시지
        return;
        }

        serve_dynamic(fd, filename, cgiargs, version); // 클라이언트에게 동적 컨텐츠를 제공

    }
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg){ /* 클라이언트에 에러를 전달하는 함수 */
    char buf[MAXLINE], body[MAXLINE];

    /* Build the HTTP response body */ //  브라우저에게 전달하는 에러 HTML
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg); // 에러 코드, 짧은 메시지
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause); // 긴 메시지, 오류 원인 파일 경로
    sprintf(body, "%s<hr><em>The Tiny web server</em>\r\n", body);

    /* Print the HTTP */ // 클라이언트 헤더에 전달하는 에러 상태
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body)); // 클라이언트 body에 응답
}

void read_requesthdrs(rio_t *rp){ /* 요청 헤더를 읽고 무시하는 함수 */
    char buf[MAXLINE]; // 요청헤더를 저장하기 위한 버퍼

    Rio_readlineb(rp, buf, MAXLINE); // 요청 해더 읽기

    while(strcmp(buf, "\r\n")){ // 요청 헤더를 종료하는 빈 텍스트 줄 검사
        Rio_readlineb(rp, buf, MAXLINE); // 버퍼 내용 읽기
        printf("%s", buf); // 버퍼의 내용 출력
    }

    return;
}

int parse_uri(char *uri, char *filename, char *cgiargs){ /* uri를 분석하는 함수 */
    char *ptr;

    if(!strstr(uri, "cgi-bin")){ /* Static content */ // uri != "cgi-bin" 정적 컨텐츠를 위한 홈 디렉토리
        strcpy(cgiargs, ""); // cgiargs를 공백으로
        strcpy(filename, "."); // 파일 경로 .로 변경(리눅스 환경에서 실행파일을 ./로 실행하기 때문)
        strcat(filename, uri); // .uri로 파일 경로 변경
        if(uri[strlen(uri)-1] == '/'){ // uri의 끝부분에 '/'가 붙었다면
            strcat(filename, "home.html"); // '.uri/home.html' 기본 파일 이름 추가
            return 1; // 1 반환
        }
    }else{ /* Dynamic content */ // uri == "cgi-bin" 동적 컨텐츠를 위한 홈 디렉토리
        ptr = index(uri, '?'); // ptr = '?'부터 나오는 문자열
        if(ptr){
            strcpy(cgiargs, ptr+1); // cgiargs에 '?'이후의 문자열 복사 (cgi 인자 추출)
            *ptr = '\0';
        }else{ // '?'가 없다면
            strcpy(cgiargs, "");
        }
        strcpy(filename, "."); // 파일경로 . 으로 변경
        strcat(filename, uri); // .uri로 변경
        return 0; // 0 반환
    }
}

void serve_static(int fd, char *filename, int filesize, char *version, char *method){
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /* Send response headers to client */
    get_filetype(filename, filetype); // 파일 이름의 접미어 부분을 검사해서 파일 타입을 결정

    sprintf(buf, "%s 200 OK\r\n", version);
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    
    Rio_writen(fd, buf, strlen(buf)); // 클라이언트에 응답 라인과 응답 헤더를 보냄

    printf("Response headers:\n");
    printf("%s", buf);

    if(strcasecmp(method, "HEAD")){
        /* Send response body to client */ // 요청 파일의 내용을 연결 식별자 fd로 복사해서 응답 본체를 보냄
        srcfd = Open(filename, O_RDONLY, 0); // static 파일을 읽어오기 위해 open해서 srcfd에 저장
        // srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); // 읽어온 파일을 mmap함수를 이용하여 메모리에 매핑
        srcp = (char*)malloc(filesize); // 해당 파일 크기만큼 malloc으로 메모리 할당
        Rio_readn(srcfd, srcp, filesize); // 메모리에서 읽어오기
        Close(srcfd); // 읽어온 파일을 메모리에 저장했기에 더 이상 srcfd는 필요 없으므로 닫음
        Rio_writen(fd, srcp, filesize); // 메모리에 저장한 static 파일을 클라이언트에게 전송
        // Munmap(srcp, filesize); // 매핑된 가상메모리 주소 반환
        free(srcp); // free로 할당했던 메모리 반환
    }
}

/*
* get_filetype - Derive file type from filename
*/
void get_filetype(char *filename, char *filetype){ /* 파일의 타입을 가져오는 함수 */
    if(strstr(filename, ".html")){
        strcpy(filetype, "text/html");
    }else if(strstr(filename, ".gif")){
        strcpy(filetype, "image/gif");
    }else if(strstr(filename, ".png")){
        strcpy(filetype, "image/png");
    }else if(strstr(filename, ".jpg")){
        strcpy(filetype, "image/jpg");
    }else if(strstr(filename, ".mp4")){
        strcpy(filetype, "video/mp4");
    }else{
        strcpy(filetype, "text/plain"); // 무형식 텍스트 파일
    }
}

void serve_dynamic(int fd, char *filename, char *cgiargs, char *version){
    char buf[MAXLINE], *emptylist[] = { NULL };

    /* Return first part of HTTP response */ // 클라이언트에 성공 응답을 알려주는 응답 라인을 보냄
    sprintf(buf, "%s 200 OK\r\n", version);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    if(Fork() == 0){ /* Child */ // 새로운 자식 프로세스를 포크
        /* Real server would set all CGI vars here */
        setenv("QUERY_STRING", cgiargs, 1); // "QUERY_STRING" 환경변수를 요청URI의 CGI 인자들로 초기화
        Dup2(fd, STDOUT_FILENO);        /* Redirect stdout to client */ // 자식은 자식의 표준 출력을 연결 파일 식별자로 재지정
        Execve(filename, emptylist, environ); /* RUN CGI program */ // CGI 프로그램을 로드 후 실행
    }
    Wait(NULL); /* Parent waits for and reaps child */ // 부모는 자식 프로세스가 종료될 때까지 기다림
}
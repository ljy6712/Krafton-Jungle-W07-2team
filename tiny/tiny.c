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
void serve_static(int fd, char *filename, int filesize, char *version);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2)
    { // 인자의 개수가 2개가 아니면
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1); // 종료
    }

    listenfd = Open_listenfd(argv[1]); // 듣기 소켓 오픈
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, // 연결 받기
                        &clientlen);                 // line:netp:tiny:accept
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                    0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd);
        Close(connfd); // 연결 닫기
    }
}

void doit(int fd)
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    /* Read request line and headers */
    Rio_readinitb(&rio, fd);                       // 읽기 버퍼 rio를 fd로 초기화
    Rio_readlineb(&rio, buf, MAXLINE);             // rio에서 읽어서 buf로 복사
    printf("%s", buf);                             // buf 출력 (GET /godzilla.gif HTTP/1.1)
    sscanf(buf, "%s %s %s", method, uri, version); // buf에서 읽어서 method, uri, version에 각각 넣어줌

    if (strcasecmp(method, "GET") && strcasecmp(method, "HEAD"))
    { // method가 GET이 아니면
        clienterror(fd, method, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return;             // 반환
    }                       // line:netp:doit:endrequesterr
    read_requesthdrs(&rio); // request header 읽고 사용X

    /* Parse URI from GET request */
    is_static = parse_uri(uri, filename, cgiargs); // URI를 파일명과 cgi 인자로 쪼개서 URI가 cgi-bin을 포함하면 동적 컨텐츠로 간주
    if (stat(filename, &sbuf) < 0)
    { // line:netp:doit:beginnotfound
        clienterror(fd, filename, "404", "Not found",
                    "Tiny couldn't find this file");
        return;
    } // line:netp:doit:endnotfound

    if (is_static)
    { // 정적 콘텐츠라면
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
        { // 보통 파일인지, 읽기 권한이 있는지 확인
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn't read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size, version); // HTTP response를 보내줌
    }
    else
    { /* Serve dynamic content */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
        { // 실행가능한지 확인
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn't run the CGI program");
            return;
        }
        serve_dynamic(fd, filename, cgiargs); // 동적 컨텐츠 제공
    }
}
/* $end doit */

/*
 * read_requesthdrs - read HTTP request headers
 */
/* $begin read_requesthdrs */
void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    while (strcmp(buf, "\r\n"))
    { // line:netp:readhdrs:checkterm
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}
/* $end read_requesthdrs */

/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */
/* $begin parse_uri */
int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;

    if (!strstr(uri, "cgi-bin"))
    {                                      // uri중 "cgi-bin"이 없으면 정적콘텐츠
        strcpy(cgiargs, "");               // cgi 인자 스트링 지우기
        strcpy(filename, ".");             // line:netp:parseuri:beginconvert1
        strcat(filename, uri);             // filename에 uri 이어붙이기
        if (uri[strlen(uri) - 1] == '/')   // uri의 마지막문자열이 "/"라면
            strcat(filename, "home.html"); // filename 끝에 home.html붙이기
        return 1;
    }
    else
    {                          //"cgi-bin"이 있으면 정적컨텐츠                     //line:netp:parseuri:isdynamic
        ptr = index(uri, '?'); // cgi 인자 가져오기
        if (ptr)
        {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        }
        else                     // cgi 인자가 없으면
            strcpy(cgiargs, ""); // line:netp:parseuri:endextract
        strcpy(filename, ".");   // line:netp:parseuri:beginconvert2
        strcat(filename, uri);   // line:netp:parseuri:endconvert2
        return 0;
    }
}
/* $end parse_uri */

/*
 * serve_static - copy a file back to the client
 */
/* $begin serve_static */
void serve_static(int fd, char *filename, int filesize, char *version)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /* Send response headers to client */
    get_filetype(filename, filetype);       // line:netp:servestatic:getfiletype
    sprintf(buf, "%s 200 OK\r\n", version); // line:netp:servestatic:beginserve
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf)); // line:netp:servestatic:endserve
    printf("Response headers:\n");
    printf("%s", buf);

    if (strcasecmp(method, "HEAD"))
    {
        /* Send response body to client */
        srcfd = Open(filename, O_RDONLY, 0); // line:netp:servestatic:open
        // srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); // file을 메모리에 대응시킨다
        srcp = (char *)malloc(filesize);
        Rio_readn(srcfd, srcp, filesize);
        Close(srcfd);                   // line:netp:servestatic:close
        Rio_writen(fd, srcp, filesize); // fd로 파일 내용 전송
        // Munmap(srcp, filesize);                                     // srcp 위치의 모든 맵핑 제거
        free(srcp);
    }
}
/*
 * get_filetype - derive file type from file name
 */
void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else if (strstr(filename, ".mp4"))
        strcpy(filetype, "video/mp4");
    else
        strcpy(filetype, "text/plain");
}
/* $end serve_static */

/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
/* $begin serve_dynamic */
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = {NULL};

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    if (Fork() == 0) // 자식 프로세스 생성
    { /* Child */    // line:netp:servedynamic:fork
        /* Real server would set all CGI vars here */
        setenv("QUERY_STRING", cgiargs, 1);   // 환경변수에 query_string이 없으면 cgiargs를 value로 추가함, 이미 있으면 value값을 바꿔줌                       // line:netp:servedynamic:setenv
        Dup2(fd, STDOUT_FILENO);              // fd를 표준 출력 파일로
        Execve(filename, emptylist, environ); // /cgi-bin/adder프로그램을 자식의 컨텍스트에서 실행시킴
    }
    Wait(NULL); // 자식이 종료되는 것을 기다림
}
/* $end serve_dynamic */

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor="
                  "ffffff"
                  ">\r\n",
            body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}
/* $end clienterror */
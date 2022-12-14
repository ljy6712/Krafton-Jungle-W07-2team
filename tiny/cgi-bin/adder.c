/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void) {
    char *buf, *p;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1=0, n2=0;

    /* Extract the two arguments */
    if ((buf = getenv("QUERY_STRING")) != NULL) { //환경변수 리스트에서 QUERY_STRING 찾음
	p = strchr(buf, '&'); //buf에서 '&'찾기
	*p = '\0';  //'&' 자리에 \0 넣어줌
    sscanf(buf, "a=%d", &n1);
    sscanf(p+1, "b=%d", &n2);
    }

    /* Make the response body */
    sprintf(content, "Welcome to add.com: ");
    sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
    // sprintf(content, "%s<form action='/cgi-bin/adder' method='GET'>",content);
    // sprintf(content, "%s<input type='text' name = 'a'>", content);
    // sprintf(content, "%s<input type='text' name = 'b'>", content);
    // sprintf(content, "%s<input type='submit' value='submit'>",content);
    // sprintf(content, "%s</form>", content);
    sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", 
	    content, n1, n2, n1 + n2);
    sprintf(content, "%sThanks for visiting!\r\n", content);
    // <form action="./adder" method="GET">
    //     <input type="text">
    //     <input type="text">
    //     <input type="submit" value="전송">
    // </form>
    /* Generate the HTTP response */
    printf("Connection: close\r\n");
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s", content);
    fflush(stdout);

    exit(0);
}
/* $end adder */

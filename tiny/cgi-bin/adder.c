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
  if((buf = getenv("QUERY_STRING")) != NULL){ // 환경변수 QUERY_STRING 가져오기
    p = strchr(buf, '&'); // buf 문자열에서 & 찾기
    *p = '\0';
    // strcpy(arg1, buf); // buf문자열 arg1에 복사
    // strcpy(arg2, p+1); // p+1에 들어있는 문자열 arg2에 복사
    // n1 = atoi(arg1); // 문자열 숫자로 변환
    // n2 = atoi(arg2); // 문자열 숫자로 변환
    sscanf(buf, "a_val=%d", &n1); // buf에 있는 값 중 %d부분을 n1주소에 저장
    sscanf(p+1, "b_val=%d", &n2); // p+1에 있는 값 중 %d부분을 n2주소에 저장
  }

  /* Make the response body */
  sprintf(content, "QUERY_STRING=%s", buf); // content 변수에 출력값 저장
  sprintf(content, "Welcome to add.com: ");
  sprintf(content, "%sTHE Internet addition portal.\r\n", content);
  sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", content, n1, n2, n1+n2);
  sprintf(content, "%sThanks for visiting!\r\n", content);

  /* Generate the HTTP response */
  printf("Connection: close\r\n");
  printf("Content-length: %d\r\n", (int)strlen(content));
  printf("Content-type: text/html\r\n\r\n");
  printf("%s", content);
  fflush(stdout); // 메모리 값 반환

  exit(0);
}
/* $end adder */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  int a;
  char b[] = "sample";
  char *c[10];
  char d[] = " ";
  c[0] = b;

  printf("%c\n", c[0][0]);
  if (strtok(d, " ") == NULL) printf("NULL END\n");
  exit(0);
}
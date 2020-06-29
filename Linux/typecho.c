#include <stdio.h>
#include <stdlib.h>

int main(void)
{
  char in[100];
  fgets(in, 100, stdin);
  printf("%s", in);
  exit(0);
}
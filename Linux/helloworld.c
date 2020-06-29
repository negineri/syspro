#include <unistd.h>
#include <stdlib.h>

int main(void)
{
  write(1, "Hello world\n", 12);
  exit(0);
}
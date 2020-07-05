#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char* argv[]) {
  struct stat st;
  char fad[] = "---------";

  if (argc != 2) exit(1);
  if (stat(argv[1], &st) != 0) exit(1);

  if (st.st_mode & S_IRUSR) fad[0] = 'r';
  if (st.st_mode & S_IWUSR) fad[1] = 'w';
  if (st.st_mode & S_IXUSR) fad[2] = 'x';
  if (st.st_mode & S_IRGRP) fad[3] = 'r';
  if (st.st_mode & S_IWGRP) fad[4] = 'w';
  if (st.st_mode & S_IXGRP) fad[5] = 'x';
  if (st.st_mode & S_IROTH) fad[6] = 'r';
  if (st.st_mode & S_IWOTH) fad[7] = 'w';
  if (st.st_mode & S_IXOTH) fad[8] = 'x';

  printf("%s\n", fad);
  exit(0);
}
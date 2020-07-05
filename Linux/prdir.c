#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>

int main(int argc, char* argv[]) {
  struct stat st;
  DIR* ds;
  struct dirent* de;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s directory\n", argv[0]);
    exit(1);
  }
  if (stat(argv[1], &st) < 0) {
    fprintf(stderr, "stat error\n");
    exit(1);
  }
  if (!(S_ISDIR(st.st_mode))) {
    fprintf(stderr, "%s: not dirctory\n", argv[1]);
    exit(1);
  }
  if ((ds = opendir(argv[1])) == NULL) {
    fprintf(stderr, "%s: cannot open\n", argv[1]);
    exit(1);
  }
  if ((de = readdir(ds)) == NULL) {
    fprintf(stderr, "readdir error\n");
    exit(1);
  }
  while ((de = readdir(ds)) != NULL) {
    printf("%s %ld\n", de->d_name, de->d_ino);
  }
  exit(0);
}
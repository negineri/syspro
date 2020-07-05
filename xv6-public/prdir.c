#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

int main(int argc, char *argv[]) {
  struct dirent de;
  struct stat cbuf;
  int fd;
  if (argc < 2) {
    printf(2, "Usage: %s directory\n", argv[0]);
    exit();
  }
  if (stat(argv[1], &cbuf) < 0) {
    printf(2, "stat error\n");
    exit();
  }
  if (!(cbuf.type & T_DIR)) {
    printf(2, "%s: not dirctory\n", argv[1]);
    exit();
  }
  if ((fd = open(argv[1], O_RDONLY)) < 0) {
    printf(2, "%s: cannot open\n", argv[1]);
    exit();
  }
  while ((read(fd, &de, sizeof(de))) == sizeof(de)) {
    if (de.inum == 0) break;
    printf(1, "%s\t%d\n", de.name, de.inum);
  }
  exit();
}

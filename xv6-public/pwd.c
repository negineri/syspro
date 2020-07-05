#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "stat.h"
#include "fs.h"

#define MAX_PATH 256

int rp_to_ap(char *rp, char *ap) {
  struct stat st;
  struct dirent de;
  int fd;
  int le;
  int rv;

  if (stat(rp, &st) < 0) return -1;
  strcpy(rp + strlen(rp), "/..");
  if ((fd = open(rp, O_RDONLY)) < 0) return -1;
  while (1) {
    if ((read(fd, &de, sizeof(de))) != sizeof(de) || de.inum == 0) {
      close(fd);
      return -1;
    }
    if (de.inum == st.ino) break;
  }
  close(fd);
  if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
    strcpy(ap, "/");
    return 1;
  }
  if ((rv = rp_to_ap(rp, ap)) < 0) return -1;
  le = strlen(ap);
  if (rv == 0) strcpy(ap + le++, "/");
  strcpy(ap + le, de.name);
  return 0;
}

int main() {
  char rp[MAX_PATH] = ".";
  char ap[MAX_PATH] = "";

  if (rp_to_ap(rp, ap) < 0) {
    printf(2, "rtp error\n");
    exit();
  }
  printf(1, "%s\n", ap);
  exit();
}
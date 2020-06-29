#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
int main(int ac, char *av[]) {
  int fds[2];
  int infd;
  int cc;
  char buf[512];
  pid_t pid;
  int status;
  if (ac != 2) {
    fprintf(stderr, "usage: %s file\n", av[0]);
    exit(1);
  }
  if ((infd = open(av[1], O_RDONLY)) < 0) {
    fprintf(stderr, "Cannot open: %s\n", av[1]);
    exit(1);
  }
  /* create pipe */
  if (pipe(fds) < 0) {
    perror("pipe");
    exit(1);
  }
  if ((pid = fork()) == 0) {
    /* execute "more" as child proc */
    close(0);
    if (dup(fds[0]) < 0) {
      perror("dup");
      exit(1);
    }
    close(fds[1]);
    execl("/bin/more", "more", (char *)NULL);
    exit(0);
  }
  close(fds[0]);
  while ((cc = read(infd, buf, sizeof(buf))) != 0) {
    write(fds[1], buf, cc);
  }
  close(infd);
  close(fds[1]);
  printf("wait\n");
  wait(&status);
  printf("finish\n");
  exit(0);
}
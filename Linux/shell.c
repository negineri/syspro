#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define ARG_MAX 8192
#define PATH_MAX 256

#define ARGT_FD_FILEIN 1
#define ARGT_FD_FILEOUT 2
#define ARGT_FD_PIPEIN 4
#define ARGT_FD_PIPEOUT 8

#define EXEC_FD_STDIN 1
#define EXEC_FD_STDOUT 2

int argtok(const char *args[], char *cmd[], char *fdp[]) {
  int fl = 0;
  static char **ar;
  int i;

  i = 0;
  if (args != NULL) ar = args;
  if (*ar == NULL) return -1;
  for (; *ar != NULL; ar++) {
    if (strcmp(*ar, "<") == 0) {
      ar++;
      fdp[0] = *ar;
      fl |= ARGT_FD_FILEIN;
    } else if (strcmp(*ar, ">") == 0) {
      ar++;
      fdp[1] = *ar;
      fl |= ARGT_FD_FILEOUT;
    } else if (strcmp(*ar, "|") == 0) {
      ar++;
      fl |= ARGT_FD_PIPEOUT;
      break;
    } else {
      cmd[i++] = *ar;
    }
  }
  cmd[i] = NULL;
  return fl;
}

int exec_p(const char *cm[], int flag, int p2c[2], int c2p[2]) {
  char cp[PATH_MAX];
  pid_t pid;

  strcpy(cp, "/bin/");
  strcat(cp, cm[0]);
  if ((pid = fork()) != 0) return pid;
  close(p2c[1]);
  close(c2p[0]);
  if (flag & EXEC_FD_STDIN) {
    close(p2c[0]);
  } else {
    close(0);
    if (dup(p2c[0]) < 0) {
      perror("dup");
      exit(1);
    }
  }
  if (flag & EXEC_FD_STDOUT) {
    close(c2p[1]);
  } else {
    close(1);
    if (dup(c2p[1]) < 0) {
      perror("dup");
      exit(1);
    }
  }
  execv(cp, cm);
}

int rdt_fdw(int fdf, int fdo, char *fdp) {
  int ffd;
  char buf[512];
  int cc;

  if (fdf & ARGT_FD_FILEIN) {
    if ((ffd = open(fdp, O_RDONLY)) < 0) {
      fprintf(stderr, "Cannot open: %s\n", fdp);
      return -1;
    }
    while ((cc = read(ffd, buf, sizeof(buf))) != 0) {
      write(fdo, buf, cc);
    }
    close(ffd);
  }
  return 0;
}

int rdt_fdr(int fdf, int fdi, int fdo, char *fdp) {
  int ffd;
  char buf[512];
  int cc;

  if (fdf & ARGT_FD_FILEOUT) {
    if ((ffd = open(fdp, O_CREAT | O_RDWR, 0644)) < 0) {
      fprintf(stderr, "Cannot open: %s\n", fdp);
      return -1;
    }
    while ((cc = read(fdi, buf, sizeof(buf))) != 0) {
      write(ffd, buf, cc);
    }
    close(ffd);
    if (fdf & ARGT_FD_PIPEOUT) {
      if ((ffd = open(fdp, O_RDONLY)) < 0) {
        fprintf(stderr, "Cannot open: %s\n", fdp);
        return -1;
      }
      while ((cc = read(ffd, buf, sizeof(buf))) != 0) {
        write(fdo, buf, cc);
      }
      close(ffd);
    }
  } else if (fdf & ARGT_FD_PIPEOUT) {
    while ((cc = read(fdi, buf, sizeof(buf))) != 0) {
      write(fdo, buf, cc);
    }
  }
  return 0;
}

int main() {
  char in[ARG_MAX];
  char *ar[1000];
  int i;
  int status;
  char *cm[100];
  char *fdp[2];
  char *fdpo;
  int p2c[2];
  int c2p[2];
  int fdi;
  int fdf, fdft;
  int rdf;

  while (1) {
    printf("shell:$ ");
    fgets(in, ARG_MAX, stdin);
    i = -1;
    while (in[++i] != '\0') {
      if (in[i] == '\n') in[i] = ' ';
    }
    if ((ar[0] = strtok(in, " ")) == NULL) continue;
    i = 0;
    while ((ar[++i] = strtok(NULL, " ")) != NULL)
      ;

    fdf = argtok(ar, cm, fdp);
    if (pipe(p2c) < 0) {
      perror("pipe");
      exit(1);
    }
    if (pipe(c2p) < 0) {
      perror("pipe");
      close(p2c[0]);
      close(p2c[1]);
      exit(1);
    }
    rdf = 0;
    if (!(fdf & ARGT_FD_FILEIN)) rdf |= EXEC_FD_STDIN;
    if (!(fdf & (ARGT_FD_PIPEOUT | ARGT_FD_FILEOUT))) rdf |= EXEC_FD_STDOUT;
    if (exec_p(cm, rdf, p2c, c2p) == -1) {
      close(p2c[0]);
      close(p2c[1]);
      close(c2p[0]);
      close(c2p[1]);
      continue;
    }
    close(p2c[0]);
    close(c2p[1]);
    if (!(rdf & EXEC_FD_STDIN)) {
      rdt_fdw(fdf, p2c[1], fdp[0]);
    }
    close(p2c[1]);
    if (!(fdf & ARGT_FD_PIPEOUT)) {
      rdt_fdr(fdf, c2p[0], 0, fdp[1]);
      close(c2p[0]);
      wait(&status);
      continue;
    }
    fdft = fdf;
    fdpo = fdp[1];
    while ((fdf = argtok(NULL, cm, fdp)) != -1) {
      fdi = c2p[0];
      if (pipe(p2c) < 0) {
        perror("pipe");
        close(fdi);
        exit(1);
      }
      if (pipe(c2p) < 0) {
        perror("pipe");
        close(fdi);
        close(p2c[0]);
        close(p2c[1]);
        exit(1);
      }
      rdf = 0;
      if (!(fdf & (ARGT_FD_PIPEOUT | ARGT_FD_FILEOUT))) rdf |= EXEC_FD_STDOUT;
      if (exec_p(cm, rdf, p2c, c2p) == -1) {
        close(fdi);
        close(p2c[0]);
        close(p2c[1]);
        close(c2p[0]);
        close(c2p[1]);
        break;
      }
      close(p2c[0]);
      close(c2p[1]);
      rdt_fdr(fdft, fdi, p2c[1], fdpo);
      close(fdi);
      rdt_fdw(fdf, p2c[1], fdp[0]);
      close(p2c[1]);
      wait(&status);
      fdi = c2p[0];
      if (rdf & EXEC_FD_STDOUT) {
        rdt_fdr(fdf, c2p[0], 0, fdp[1]);
        close(c2p[0]);
        break;
      }
    }
  }
}
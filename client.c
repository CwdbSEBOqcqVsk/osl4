#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

int seed = 213431;

const int PATH_LENGTH = 30, MESSAGE_LENGTH = 256;

char * generatefifoname(char * dest, int begin, int end) {
  char *string = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
  int len = strlen(string);
  srand(time(NULL) + ++seed);
  if (end - begin <= 0) {
    return dest;
  }
  for (int i = begin; i < end; ++i) {
    dest[i] = string[rand() % len];
  }
  dest[end] = '\0';
  strcat(dest, ".fifo");
  return dest;
}

int main(int argc, char ** argv) {
  char name[PATH_LENGTH];
  printf("enter name: ");
  scanf("%29s", name);
  if (fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK) != 0) {
    printf("can't set O_NONBLOCK for stdin\n");
    exit(2);
  }
  printf("connecting to server\n");
  int serverfifo = open("/tmp/wdkjf23c4zn,p[ews.fifo", O_WRONLY);
  if (serverfifo == -1) {
    printf("can't reach server\n");
    exit(3);
  }
  printf("connected\n");
  char fifoname1[PATH_LENGTH], fifoname2[PATH_LENGTH], message[MESSAGE_LENGTH];
  strcpy(fifoname1, "/tmp/");
  strcpy(fifoname2, "/tmp/");
  while (1) {
    if (mkfifo(generatefifoname(fifoname1, 5, 24), S_IFIFO | S_IRWXU | S_IRWXG | S_IRWXO) == 0) {
      break;
    }
    if (errno != EEXIST) {
      printf("problems with creating fifo 1\n");
      exit(1);
    }
  }
  while (1) {
    if (mkfifo(generatefifoname(fifoname2, 5, 24), S_IFIFO | S_IRWXU | S_IRWXG | S_IRWXO) == 0) {
      break;
    }
    if (errno != EEXIST) {
      printf("problems with creating fifo 2\n");
      unlink(fifoname1);
      exit(1);
    }
  }
  sprintf(message, "%s %s %s", fifoname1, fifoname2, name);
  write(serverfifo, message, 3 * PATH_LENGTH);
  close(serverfifo);
	int out = open(fifoname1, O_WRONLY);
  fcntl(out, F_SETFL, fcntl(out, F_GETFL, 0) | O_NONBLOCK);
  int in = open(fifoname2, O_RDONLY);
  fcntl(in, F_SETFL, fcntl(in, F_GETFL, 0) | O_NONBLOCK);
	while (1) {
		char message[MESSAGE_LENGTH];
    sprintf(message, "%s: ", name);
    for (int i = strlen(message); i < MESSAGE_LENGTH; ++i) {
      int result = read(STDIN_FILENO, message + i, 1);
      if (result == -1) {
        break;
      }
      if (message[i] == '\n' || i == MESSAGE_LENGTH - 1) {
        message[i] = '\0';
        write(out, message, MESSAGE_LENGTH);
        break;
      }
    }
    int result = read(in, message, MESSAGE_LENGTH);
    if (result == -1) {
      continue;
    }
    if (result == 0) {
      printf("server has gone\n");
      close(out);
      unlink(fifoname1);
      close(in);
      unlink(fifoname2);
      exit(0);
    }
    printf("%s\n", message);
	}
  return 0;
}

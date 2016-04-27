#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const int PATH_LENGTH = 30, MESSAGE_LENGTH = 256;

int main (int argc, char ** argv) {
	if (fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK) != 0) {
		printf("can't set O_NONBLOCK for stdin\n");
		exit(1);
	}
	char outname[PATH_LENGTH], inname[PATH_LENGTH];
	strcpy(inname, "/tmp/wdkjf23c4zn,p[ewo.fifo");
	strcpy(outname, "/tmp/wdkjf23c4zn,p[ewi.fifo");
	if (mkfifo(outname, S_IFIFO | S_IRWXU | S_IRWXG | S_IRWXO) == -1 && errno != EEXIST) {
		printf("problems with creating fifo 1\n");
		exit(2);
	}
	if (mkfifo(inname, S_IFIFO | S_IRWXU | S_IRWXG | S_IRWXO) == -1 && errno != EEXIST) {
		printf("problems with creating fifo 2\n");
		exit(3);
	}
	printf("waiting for other program to start\n");
	int out = open(outname, O_WRONLY),
		ores = fcntl(out, F_SETFL, fcntl(out, F_GETFL, 0) | O_NONBLOCK),
		in = open(inname, O_RDONLY),
		ires = fcntl(in, F_SETFL, fcntl(in, F_GETFL, 0) | O_NONBLOCK);
	if (out == -1 || ores == -1 || in == -1 || ires == -1) {
		printf("problems with opening nonblocking fifos\n");
		exit(4);
	}
	printf("done\n");
	while (1) {
		char message[MESSAGE_LENGTH];
		for (int i = 0; i < MESSAGE_LENGTH; ++i) {
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
      printf("another program has closed\n");
      close(out);
      unlink(outname);
      close(in);
      unlink(inname);
      exit(0);
    }
    printf(">%s\n", message);
	}
}

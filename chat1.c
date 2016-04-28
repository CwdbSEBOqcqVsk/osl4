#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

const int PATH_LENGTH = 30, MESSAGE_LENGTH = 256;

int main (int argc, char ** argv) {
	char nameout[PATH_LENGTH], namein[PATH_LENGTH];
	strcpy(namein, "/tmp/wdkjf23c4zn,p[ewo.fifo");
	strcpy(nameout, "/tmp/wdkjf23c4zn,p[ewi.fifo");
	if (mkfifo(nameout, S_IFIFO | S_IRWXU | S_IRWXG | S_IRWXO) == -1 && errno != EEXIST) {
		printf("problems with creating fifo 1\n");
		exit(1);
	}
	if (mkfifo(namein, S_IFIFO | S_IRWXU | S_IRWXG | S_IRWXO) == -1 && errno != EEXIST) {
		printf("problems with creating fifo 2\n");
		exit(2);
	}
	printf("waiting for other program to start\n");
	int pid = fork();
	if (pid == 0) {
		int out = open(nameout, O_WRONLY);
		printf("done\n");
		while (1) {
			char message[MESSAGE_LENGTH];
			for (int i = 0; i < MESSAGE_LENGTH; ++i) {
				read(STDIN_FILENO, message + i, 1);
				if (message[i] == '\n') {
					message[i] = '\0';
					write(out, message, MESSAGE_LENGTH);
					break;
				}
			}
		}
	}
	int in = open(namein, O_RDONLY);
	while (1) {
		char message[MESSAGE_LENGTH];
		int res;
		do {
			res = read(in, message, MESSAGE_LENGTH);
		} while (res == -1 && errno == EAGAIN);
		if (res == -1) {
			printf("error while reading\n");
			exit(3);
		}
		if (res == 0) {
			kill(pid, SIGKILL);
			printf("another chat has closed\n");
			exit(0);
		}
		printf(">%s\n", message);
	}
}

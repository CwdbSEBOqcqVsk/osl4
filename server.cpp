#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <signal.h>

const int PATH_LENGTH = 30, MESSAGE_LENGTH = 256;

using namespace std;

struct user {
  char name[PATH_LENGTH];
  int in, out;
};

int main() {
  signal(SIGPIPE, SIG_IGN);
  char sfifoname[PATH_LENGTH], namein[PATH_LENGTH];
	strcpy(sfifoname, "/tmp/wdkjf23c4zn,p[ews.fifo");
	if (mkfifo(sfifoname, S_IFIFO | S_IRWXU | S_IRWXG | S_IRWXO) == -1 && errno != EEXIST) {
		printf("problems with creating fifo 1\n");
		exit(1);
	}
  int serverfifo = open (sfifoname, O_RDONLY | O_NONBLOCK);
  if (serverfifo == -1) {
    printf("can't open created fifo\n");
    exit(2);
  }
  vector<user> users;
  while (1) {
    char smessage[3 * PATH_LENGTH], message[MESSAGE_LENGTH];
    int result = read(serverfifo, smessage, 3 * PATH_LENGTH);
    if (result > 0) {
      char fifo1name[PATH_LENGTH], fifo2name[PATH_LENGTH], uname[PATH_LENGTH];
      strncpy(fifo1name, smessage, PATH_LENGTH - 1);
      fifo1name[PATH_LENGTH - 1] = '\0';
      strncpy(fifo2name, smessage + PATH_LENGTH, PATH_LENGTH - 1);
      fifo2name[PATH_LENGTH - 1] = '\0';
      strcpy(uname, smessage + 2 * PATH_LENGTH);
      int in = open(fifo1name, O_RDONLY),
        out = open(fifo2name, O_WRONLY),
        result = fcntl(in, F_SETFL, fcntl(in, F_GETFL, 0) | O_NONBLOCK),
        result1 = fcntl(out, F_SETFL, fcntl(out, F_GETFL, 0) | O_NONBLOCK);
      user usr;
      strcpy(usr.name, uname);
      usr.in = in;
      usr.out = out;
      if (in == -1 || out == -1 || result == -1 || result1 == -1) {
        close(in);
        close(out);
      } else {
        sprintf(message, ">User %s connected", uname);
        for (auto u : users) {
          write(u.out, message, MESSAGE_LENGTH);
        }
        if (users.size() == 0) {
          sprintf(message, ">There are no users connected to this chat");
        } else {
          sprintf(message, ">Users connected to this chat: %s", users.begin()->name);
          for (vector<user>::iterator i = users.begin() + 1; i != users.end(); ++i) {
            sprintf(message, "%s, %s", message, i->name);
          }
        }
        message[MESSAGE_LENGTH - 1] = '\0';
        write(usr.out, message, MESSAGE_LENGTH);
        users.push_back(usr);
      }
    }
    for (vector<user>::iterator i = users.begin(); i != users.end(); ) {
      int result = read(i->in, message, MESSAGE_LENGTH);
      if (result == 0) {
        sprintf(message, ">User %s has gone", i->name);
        i = users.erase(i);
        for (auto u : users) {
          write(u.out, message, MESSAGE_LENGTH);
        }
        continue;
      }
      if (result > 0) {
        for (auto u : users) {
          if (strcmp(u.name, i->name) != 0) {
            write(u.out, message, MESSAGE_LENGTH);
          }
        }
      }
      i++;
    }
  }
}

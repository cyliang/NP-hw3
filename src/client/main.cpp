#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netdb.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "sendfile.h"
#include "recvfile.h"
#include "msg.h"
using namespace std;

#define CMD_MAX 1000

hostent *host;
int maxBuf = 300;
Msg msg;

static list<File *> fileList;
static int sock_fd;
static void go_exit();

static void line_read(char *line);
char **readline_complete_func(const char *text, int start, int end);
const char *cmds[] = { "/put", "/sleep", "/exit" };

int main(int argc, char *argv[]) {
	/************************************/
	/*     Prepare the connection       */
	/************************************/
	if(argc != 4) {
		printf("Usage: %s <host> <port> <username>\n", argv[0]);
		exit(1);
	}

	host = gethostbyname(argv[1]);
	if(host == NULL) {
			fprintf(stderr, "Cannot get host by %s.\n", argv[1]);
			exit(2);
	}

	struct sockaddr_in server_addr;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd == -1) {
		fputs("Fail to create socket.\n", stderr);
		exit(2);
	}

	if(connect(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
		fputs("Fail to connect.\n", stderr);
		exit(2);
	}

	/************************************/
	/*   Select for readline & socket   */
	/************************************/
	rl_attempted_completion_function = readline_complete_func;
	rl_callback_handler_install("", line_read);
	signal(SIGINT, SIG_IGN);
	char buf[maxBuf];
	int stdin_fd = fileno(rl_instream);
	fd_set r_set, w_set;

	sprintf(buf, "\033[1;33mWelcome to the dropbox-like server! : %s\033[m", argv[3]);
	msg.push(buf);

	sprintf(buf, "Username: %s", argv[3]);
	int n = write(sock_fd, buf, maxBuf);

	while(1) {
		int max_fd = (sock_fd > stdin_fd ? sock_fd : stdin_fd);
		FD_ZERO(&r_set);
		FD_ZERO(&w_set);

		FD_SET(stdin_fd, &r_set);
		FD_SET(sock_fd, &r_set);
		for(list<File *>::iterator it = fileList.begin(); it != fileList.end(); ++it) {
			int fd = (*it)->setCheckSet(&r_set, &w_set);
			if(fd > max_fd)
				max_fd = fd;
		}
		select(max_fd + 1, &r_set, &w_set, NULL, NULL);

		for(list<File *>::iterator it = fileList.begin(); it != fileList.end(); ) {
			if((*it)->checkDone(&r_set, &w_set)) {
				delete *it;
				fileList.erase(it++);
			} else {
				++it;
			}
		}

		if(FD_ISSET(stdin_fd, &r_set)) {
			rl_callback_read_char();
			/* What shell be done here are written in line_read(). */
		}

		if(FD_ISSET(sock_fd, &r_set)) {
			int n = read(sock_fd, buf, maxBuf);
			if(n == 0) {
				puts("Connection closed by server");
				go_exit();
				break;
			}

			char name[300];
			size_t size;
			unsigned short port;
			sscanf(buf, "Download> size:%zu; port:%hu; name:%s", &size, &port, name);

			struct sockaddr_in addr;
			memset(&addr, 0, sizeof(addr));
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			memcpy(&addr.sin_addr, host->h_addr, host->h_length);

			fileList.push_back(new RecvFile(name, size, &addr, msg.pushStatic()));
		}

		msg.refresh();
	}

	return 0;
}

void go_exit() {
	rl_callback_handler_remove();
	close(sock_fd);

	exit(0);
}

void line_read(char *line) {
	if(line == NULL || strncmp(line, "/exit", 5) == 0) {
		go_exit();
		return;
	}

	if(*line) {
		add_history(line);
		bool err = false;

		if(strncmp(line, "/put ", 5) == 0) {
			char filename[200];
			if(sscanf(line, "/put %200s", filename) == 1) {
				FILE *filePtr = fopen(filename, "rb");
				if(filePtr != NULL)
					fileList.push_back(new SendFile(filename, filePtr, sock_fd, msg.pushStatic()));
				else {
					string errMsg = "The file '";
					errMsg += filename;
					errMsg += "' does not exist";

					msg.push(errMsg);
				}
			} else {
				err = true;
			}
		} else if(strncmp(line, "/sleep ", 7) == 0) {
			int sec;
			if(sscanf(line, "/sleep %d", &sec) == 1) {
				msg.push("Client starts to sleep");
				msg.refresh();

				for(int i=0; i<sec; i++) {
					sleep(1);

					char buf[20];
					sprintf(buf, "Sleep %d", i+1);
					msg.push(buf);
					msg.refresh();
				}
				msg.push("Client wakes up");
				msg.refresh();
			} else {
				err = true;
			}
		} else {
			err = true;
		}

		if(err == true) {
			msg.push("\033[1;31mError command.\033[m");
		}
	}

	free(line);
}

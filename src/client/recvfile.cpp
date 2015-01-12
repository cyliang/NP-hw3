#include "recvfile.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>

extern Msg msg;

RecvFile::RecvFile(const char *name, size_t size, sockaddr_in *addr, list<string>::iterator it) {
	filename = name;
	filesize = size;
	doneCount = 0;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	content = malloc(size);
	msgIt = it;
	msgFor = "Download";

	if(connect(sockfd, (sockaddr *) addr, sizeof(sockaddr_in)) != 0)
		fputs("Cannot connect!\n", stderr);

	int flag = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flag | O_NONBLOCK);

	setMsg();
}

int RecvFile::setCheckSet(fd_set *rSet, fd_set *wSet) {
	FD_SET(sockfd, rSet);
	return sockfd;
}

bool RecvFile::checkDone(fd_set *rSet, fd_set *wSet) {
	if(FD_ISSET(sockfd, rSet)) {
		ssize_t n = read(sockfd, content + doneCount, filesize - doneCount);
		if(n == 0) {
			fputs("Server terminated unexpectedly.\n", stderr);
			exit(1);
		}

		if(n != -1 && (doneCount += n) == filesize) {
			FILE *f = fopen(filename.c_str(), "wb");
			fwrite(content, 1, filesize, f);
			fclose(f);

			free(content);
			close(sockfd);

			setMsg();
			string s = "Download ";
			s += filename;
			s += " complete!";
			msg.push(s);
			msg.removeStatic(msgIt);
			return true;
		}
		setMsg();
	}

	return false;
}

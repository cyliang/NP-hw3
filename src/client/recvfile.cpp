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
	fPtr = fopen(name, "wb");

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

		if(n != -1) {
			fwrite(content + doneCount, 1, n, fPtr);
			fflush(fPtr);
			doneCount += n;
			setMsg();

			if(doneCount == filesize) {
				fclose(fPtr);

				free(content);
				close(sockfd);

				setMsg();
				msg.removeStatic(msgIt);

				string s = "Download ";
				s += filename;
				s += " complete!";
				msg.push(s);
				return true;
			}
		}
	}

	return false;
}

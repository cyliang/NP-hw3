#include "sendfile.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>

extern int maxBuf;
extern hostent *host;
extern Msg msg;

SendFile::SendFile(const char *name, FILE *file, int fd, list<string>::iterator it) {
	filename = name;
	doneCount = 0;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	msgIt = it;
	msgFor = "Upload";

	fseek(file, 0, SEEK_END);
	filesize = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	content = malloc(filesize);
	fread(content, 1, filesize, file);
	fclose(file);

	char buf[maxBuf];
	sprintf(buf, "Put> size:%zu; name:%s", filesize, name);
	write(fd, buf, maxBuf);
	
	unsigned short port;
	read(fd, buf, maxBuf);
	printf("%s\n", buf); fflush(stdout);
	sscanf(buf, "Upload> port:%hu", &port);

	sockaddr_in addr;
	memset(&addr, 0x00, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	memcpy(&addr.sin_addr, host->h_addr, host->h_length);

	if(connect(sockfd, (sockaddr *) &addr, sizeof(addr)) != 0)
		fputs("Cannot connect!\n", stderr);

	int flag = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flag | O_NONBLOCK);

	setMsg();
}

int SendFile::setCheckSet(fd_set *rSet, fd_set *wSet) {
	FD_SET(sockfd, wSet);
	return sockfd;
}

bool SendFile::checkDone(fd_set *rSet, fd_set *wSet) {
	if(FD_ISSET(sockfd, wSet)) {
		ssize_t n = write(sockfd, content + doneCount, filesize - doneCount);

		if(n != -1 && (doneCount += n) == filesize) {
			free(content);
			close(sockfd);

			setMsg();
			msg.removeStatic(msgIt);

			string s = "Upload ";
			s += filename;
			s += " complete!";
			msg.push(s);
			return true;
		}
		setMsg();
	}

	return false;
}

#pragma once

#include <cstdio>
#include <netinet/in.h>
#include "file.h"
#include "msg.h"
using namespace std;

class RecvFile: public File {
public:
	RecvFile(const char *filename, size_t filesize, sockaddr_in *addr, list<string>::iterator);
	int setCheckSet(fd_set *rSet, fd_set *wSet);
	bool checkDone(fd_set *rSet, fd_set *wSet);

private:
	FILE *fPtr;
};

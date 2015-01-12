#pragma once

#include "file.h"
#include "msg.h"
using namespace std;

class SendFile: public File {
public:
	SendFile(const char *filename, FILE *file, int cmdfd, list<string>::iterator it);
	int setCheckSet(fd_set *rSet, fd_set *wSet);
	bool checkDone(fd_set *rSet, fd_set *wSet);

};

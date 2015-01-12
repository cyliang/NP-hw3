#pragma once

#include <sys/select.h>
#include <string>
#include <cstdio>
#include <list>
using namespace std;

class File {
public:
	virtual int setCheckSet(fd_set *rSet, fd_set *wSet) = 0;
	virtual bool checkDone(fd_set *rSet, fd_set *wSet) = 0;

protected:
	string filename;
	size_t filesize;
	int sockfd;

	size_t doneCount;
	list<string>::iterator msgIt;

	void *content;

	string msgFor;
	void setMsg() {
		*msgIt = msgFor;
		*msgIt += "ing file : ";
		*msgIt += filename;

		size_t per = doneCount * 25 / filesize;
		*msgIt += "\nProgress : [";
		for(size_t i=0; i<per; i++)
			*msgIt += "#";
		for(; per<25; per++)
			*msgIt += " ";
		*msgIt += "]\n\n";
	}
};

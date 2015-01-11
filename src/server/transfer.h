#pragma once

#include <list>
#include <sys/types.h>

class TransferringList {
public:
	typedef void (*Callback)(void *, ssize_t n);

	TransferringList();
	void pushReadJob(int fd, void *buf, size_t maxRead, Callback callback, void *callbackArg); 
	void pushWriteJob(int fd, void *buf, size_t nWrite, Callback callback, void *callbackArg); 

	int setCheckSet(fd_set *rSet, fd_set *wSet);
	void checkDone(fd_set *rSet, fd_set *wSet);

private:
	struct Job {
		enum Type {READ, WRITE} type;

		int fd;
		Callback callback;
		void *callbackArg;
		void *buf;
		size_t n;
		ssize_t total;
	};

	void pushJob(Job::Type type, int fd, void *buf, size_t n, Callback callback, void *callbackArg);

	std::list<Job> jobList;
	int max_fd;
};

#include "transfer.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
using namespace std;

TransferringList::TransferringList():
	max_fd(0)
{
}

void TransferringList::pushReadJob(int fd, void *buf, size_t maxRead, Callback callback, void *callbackArg) {
	pushJob(
		Job::READ,
		fd, buf, maxRead, callback, callbackArg
	);
}

void TransferringList::pushWriteJob(int fd, void *buf, size_t n, Callback callback, void *callbackArg) {
	pushJob(
		Job::WRITE,
		fd, buf, n, callback, callbackArg
	);
}

void TransferringList::pushJob(Job::Type type, int fd, void *buf, size_t n, Callback callback, void *callbackArg) {
	Job newJob;

	newJob.type = type;
	newJob.fd = fd;
	newJob.callback = callback;
	newJob.callbackArg = callbackArg;
	newJob.buf = buf;
	newJob.n = n;
	newJob.total = 0;

	int fflag = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, fflag | O_NONBLOCK);

	jobList.push_front(newJob);
	if(fd > max_fd)
		max_fd = fd;
}

int TransferringList::setCheckSet(fd_set *rSet, fd_set *wSet) {
	for(list<Job>::iterator it = jobList.begin(); it != jobList.end(); ) {
		if(fcntl(it->fd, F_GETFD) == -1) {
			jobList.erase(it++);
			continue;
		}
		
		if(it->type == Job::READ)
			FD_SET(it->fd, rSet);
		else
			FD_SET(it->fd, wSet);
		++it;
	}

	return max_fd;
}

void TransferringList::checkDone(fd_set *rSet, fd_set *wSet) {
	for(list<Job>::iterator it = jobList.begin(); it != jobList.end(); ) {
		ssize_t n = -1;
		if(it->type == Job::READ && FD_ISSET(it->fd, rSet)) {
			n = read(it->fd, it->buf + it->total, it->n - it->total);
			
			if(n == 0) {
				(*it->callback)(it->callbackArg, 0);
				jobList.erase(it++);
				continue;
			}

		} else if(it->type == Job::WRITE && FD_ISSET(it->fd, wSet)) {
			n = write(it->fd, it->buf + it->total, it->n - it->total);
		}

		if(n == -1 || (it->total += n) != it->n) {
			++it;
		} else {
			/* Job has finished */
			(*it->callback)(it->callbackArg, it->total);
			jobList.erase(it++);
		}
	}
}

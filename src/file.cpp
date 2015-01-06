#include "file.h"
#include "transfer.h"

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
using namespace std;

unsigned int File::nextID = 0;
char File::rootPath[30];
bool File::isInit = false;
list<File *> File::waitFreeFiles;

extern TransferringList transferringList;

void File::init() {
	int ret;

	srand(time(NULL));
	do {
		sprintf(rootPath, "/tmp/dbox-like-%d", rand());
		ret = mkdir(rootPath, 0777);
	} while(ret == -1 && errno == EEXIST);

	if(ret == -1) {
		fputs("Cannot access to create file database!.\n", stderr);
		exit(1);
	}

	isInit = true;
}

File::File(const char *name, size_t size):
	fileName(name), fileSize(size), fileID(nextID++),
	isOpen(false), isInDisk(false), timeToUse(0), content(NULL),
	isFreeing(false), isReading(false)
{
	if(!isInit)
		init();

	sprintf(filePath, "%s/%u", rootPath, fileID);
}

const char *File::getName() const {
	return fileName.c_str();
}

size_t File::getSize() const {
	return fileSize;
}

void *File::saveFileOpen() {
	if(isOpen)
		return content;

	isOpen = true;
	return content = malloc(fileSize);
}

void File::saveFileCommit(int TTU) {
	timeToUse = TTU;

	if(!isInDisk) {
		writeFd = creat(filePath, 0666);

		transferringList.pushWriteJob(
			writeFd,
			content,
			fileSize,
			writeCallback,
			this
		);
	}
}

void File::writeCallback(void *arg, ssize_t n) {
	File *fPtr = (File *) arg;
	fPtr->isInDisk = true;
	close(fPtr->writeFd);
}

void *File::readFile() {
	if(isOpen) {
		if(isFreeing) {
			waitFreeFiles.erase(waitFreeIt);
			isFreeing = false;
		}

		return content;
	}

	if(!isReading) {
		content = malloc(fileSize);

		readFd = open(filePath, O_RDONLY);

		transferringList.pushReadJob(
			readFd,
			content,
			fileSize,
			readCallback,
			this
		);

		isReading = true;
	}

	return NULL;
}

void File::readCallback(void *arg, ssize_t n) {
	File *fPtr = (File *) arg;
	fPtr->isReading = false;
	fPtr->isOpen = true;
	close(fPtr->readFd);
}

void File::readFileFinish() {
	if(timeToUse > 0) {
		timeToUse--;
	} else {
		waitFreeFiles.push_front(this);
		waitFreeIt = waitFreeFiles.begin();
		isFreeing = true;
	}
}

void File::checkEachFree() {
	for(list<File *>::iterator it = waitFreeFiles.begin(); it != waitFreeFiles.end(); ) {
		if((*it)->isInDisk) {
			(*it)->isOpen = false;
			(*it)->isFreeing = false;
			free((*it)->content);

			waitFreeFiles.erase(it++);
		} else {
			++it;
		}
	}
}


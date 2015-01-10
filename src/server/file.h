#pragma once

#include <list>
#include <string>
#include <sys/types.h>

class File {
public:
	File(const char *fileName, size_t fileSize);
	void *saveFileOpen();
	void saveFileCommit(int timeToUse = 0);
	void *readFile();
	void readFileFinish();

	const char *getName() const;
	size_t getSize() const;

	static void checkEachFree();
	static void init();
	static void writeCallback(void *, ssize_t);
	static void readCallback(void *, ssize_t);

private:
	const std::string fileName;
	const size_t fileSize;
	const unsigned int fileID;
	char filePath[40];

	static unsigned int nextID;
	static char rootPath[30];
	static bool isInit;
	static std::list<File *> waitFreeFiles;

	std::list<File *>::iterator waitFreeIt;
	bool isFreeing;
	bool isReading;
	bool isOpen;
	bool isInDisk;
	int readFd;
	int writeFd;
	int timeToUse;

	void *content;
};

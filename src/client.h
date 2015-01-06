#pragma once

#include <list>
#include <sys/types.h>

class User;
class File;

class Client {
public:
	Client(int sockfd);
	~Client();

	static void checkPullFile();

	static void recvUsername(void *, ssize_t);
	static void recvCmd(void *, ssize_t);
	static void finishSendCmd(void *, ssize_t);
	static void finishTransfer(void *, ssize_t);
	static void transferConnected(void *, ssize_t);
	
	friend class User;

private:
	void startRecvCmd();
	void pullFile();
	void putFile();

	static std::list<Client *> allClient;

	int sockfd;
	std::list<Client *>::iterator _selfIt, _userIt;
	User *user;
	std::list<File *> pullList;

	static const int maxBuf = 300;
	char recvBuffer[maxBuf];
	char sendBuffer[maxBuf];
	bool isSending;

	int fileTransferring;
	struct FileTransferringArg {
		enum Type {PULL, PUT} type;
		FileTransferringArg(Type type, File *file, Client *client);

		File *file;
		Client *client;
		int sockfd;
		unsigned short port;
	};
	FileTransferringArg *waitingPutArg;
	void transferByAnotherPort(FileTransferringArg *);
};

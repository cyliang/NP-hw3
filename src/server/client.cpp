#include "client.h"
#include "user.h"
#include "transfer.h"
#include "file.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;

extern TransferringList transferringList;

list<Client *> Client::allClient;

Client::Client(int fd):
	user(NULL), 
	sockfd(fd),
	fileTransferring(0),
	isSending(false),
	waitingPutArg(NULL)
{
	allClient.push_front(this);
	_selfIt = allClient.begin();

	transferringList.pushReadJob(
		sockfd,
		recvBuffer,
		maxBuf,
		recvUsername,
		this
	);
}

void Client::recvUsername(void *arg, ssize_t n) {
	Client *cPtr = (Client *) arg;
	if(n == 0) {
		delete cPtr;
		return;
	}

	char username[100];
	if(sscanf(cPtr->recvBuffer, "Username: %s", username) != 1) {
		delete cPtr;
		return;
	}

	string name(username);
	cPtr->user = User::allUser.count(name) ?
		User::allUser[name] :
		new User(name);
	cPtr->_userIt = cPtr->user->clientLogin(cPtr);

	cPtr->startRecvCmd();
}

void Client::startRecvCmd() {
	transferringList.pushReadJob(
		sockfd,
		recvBuffer,
		maxBuf,
		recvCmd, 
		this
	);
}

void Client::recvCmd(void *arg, ssize_t n) {
	Client *cPtr = (Client *) arg;

	if(n == 0) {
		delete cPtr;
		return;
	}

	char fileName[100];
	size_t fileSize;
	sscanf(cPtr->recvBuffer, "Put> size:%zu; name:%s", &fileSize, fileName);

	cPtr->waitingPutArg = new FileTransferringArg(FileTransferringArg::PUT, new File(fileName, fileSize), cPtr);
	cPtr->putFile();
}

void Client::putFile() {
	if(!waitingPutArg || isSending || fileTransferring >= 5)
		return;

	sprintf(sendBuffer, "Upload> port:%hu", waitingPutArg->port);
	transferByAnotherPort(waitingPutArg);
	waitingPutArg = NULL;

	startRecvCmd();
}

void Client::finishSendCmd(void *arg, ssize_t n) {
	Client *cPtr = (Client *) arg;
	cPtr->isSending = false;
	cPtr->putFile();
}

void Client::finishTransfer(void *arg, ssize_t n) {
	FileTransferringArg *ptr = (FileTransferringArg *) arg;
	ptr->client->fileTransferring--;

	if(ptr->type == FileTransferringArg::PULL)
		ptr->file->readFileFinish();
	else
		ptr->client->user->clientPutFile(ptr->file, ptr->client);

	close(ptr->sockfd);
	delete ptr;

	ptr->client->putFile();
}

void Client::checkPullFile() {
	for(list<Client *>::iterator it = allClient.begin(); it != allClient.end(); ++it) {
		(*it)->pullFile();
	}
}

void Client::pullFile() {
	if(isSending || pullList.empty() || fileTransferring >= 5)
		return;

	for(list<File *>::iterator it = pullList.begin(); it != pullList.end(); ++it) {
		if((*it)->readFile()) {
			FileTransferringArg *arg = new FileTransferringArg(FileTransferringArg::PULL, *it, this);

			sprintf(sendBuffer, "Download> size:%zu; port:%hu; name:%s",
				arg->file->getSize(), arg->port, arg->file->getName());

			transferByAnotherPort(arg);

			pullList.erase(it);
			return;
		}
	}
}

void Client::transferConnected(void *a, ssize_t n) {
	FileTransferringArg *arg = (FileTransferringArg *) a;

	int newfd = accept4(arg->sockfd, NULL, NULL, SOCK_NONBLOCK);
	close(arg->sockfd);
	arg->sockfd = newfd;

	if(arg->type == FileTransferringArg::PULL) {
		transferringList.pushWriteJob(
			arg->sockfd,
			arg->file->readFile(),
			arg->file->getSize(),
			finishTransfer,
			arg
		);
	} else {
		transferringList.pushReadJob(
			arg->sockfd,
			arg->file->saveFileOpen(),
			arg->file->getSize(),
			finishTransfer,
			arg
		);
	}
}

Client::~Client() {
	close(sockfd);
	allClient.erase(_selfIt);
	if(user)
		user->clientLogout(_userIt);
}

Client::FileTransferringArg::FileTransferringArg(Type t, File *f, Client *c):
	type(t), file(f), client(c), 
	sockfd(socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0))
{

	sockaddr_in bindAddr;
	memset(&bindAddr, 0x00, sizeof(bindAddr));
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = 0;
	bindAddr.sin_addr.s_addr = INADDR_ANY;

	socklen_t addrLen = sizeof(bindAddr);
	bind(sockfd, (sockaddr *) &bindAddr, sizeof(bindAddr));
	listen(sockfd, 1);

	getsockname(sockfd, (sockaddr *) &bindAddr, &addrLen);
	port = ntohs(bindAddr.sin_port);
}

void Client::transferByAnotherPort(FileTransferringArg *arg) {
	fileTransferring++;

	transferringList.pushWriteJob(
		sockfd,
		sendBuffer,
		maxBuf,
		finishSendCmd,
		this
	);
	isSending = true;

	transferringList.pushReadJob(
		arg->sockfd,
		NULL,
		0,
		transferConnected,
		arg
	);
}

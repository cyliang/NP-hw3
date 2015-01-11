#include "transfer.h"
#include "client.h"
#include "file.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>


TransferringList transferringList;

static int listenFd;
static void pushAcceptJob();
static void acceptingClient(void *, ssize_t);

int main(int argc, char *argv[]) {
	if(argc != 2) {
		fprintf(stderr, "Usage: %s <port>\n", argv[0]);
		exit(1);
	}

	/* Initialization */
	File::init();

	/* set listenFd */

	struct sockaddr_in listen_addr;
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_addr.sin_port = htons(atoi(argv[1]));

	listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenFd == -1) {
		fputs("Cannot create socket.\n", stderr);
		exit(2);
	}
	if(bind(listenFd, (struct sockaddr *) &listen_addr, sizeof(listen_addr)) == -1) {
		fputs("Cannot bind.\n", stderr);
		exit(2);
	}
	if(listen(listenFd, 1024) == -1) {
		fputs("Cannot listen.\n", stderr);
		exit(2);
	}

	/* push listen fd into transferList to wait for accept */
	pushAcceptJob();

	fd_set rSet, wSet;
	int maxFd;
	while(1) {
		/* Doing the non-blocking I/O */
		maxFd = -1;
		FD_ZERO(&rSet);
		FD_ZERO(&wSet);

		int transMaxFd = transferringList.setCheckSet(&rSet, &wSet);
		if(transMaxFd > maxFd)
			maxFd = transMaxFd;

		select(maxFd + 1, &rSet, &wSet, NULL, NULL);
		transferringList.checkDone(&rSet, &wSet);

		/* Routinely checking */
		Client::checkPullFile();
		File::checkEachFree();
	}
}

void pushAcceptJob() {
	transferringList.pushReadJob(
		listenFd,
		NULL,
		0,
		acceptingClient,
		NULL
	);
}

void acceptingClient(void *arg, ssize_t n) {
	int clientFd = accept4(listenFd, NULL, NULL, SOCK_NONBLOCK);
	new Client(clientFd);
	pushAcceptJob();
}

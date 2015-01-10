#include "transfer.h"
#include "client.h"
#include "file.h"

#include <sys/select.h>

TransferringList transferringList;

int main() {
	int listenFd;

	/* TODO: Initialization */
	/* TODO: set listenFd */
	/* TODO: push listen fd into transferList to wait for accept */

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

		select(maxFd, &rSet, &wSet, NULL, NULL);
		transferringList.checkDone(&rSet, &wSet);

		/* Routinely checking */
		Client::checkPullFile();
		File::checkEachFree();
	}
}

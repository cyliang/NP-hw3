#include "user.h"
#include "client.h"
#include "file.h"
using namespace std;

map<string, User *> User::allUser;

User::User(const string &n):
	name(n)
{
	allUser[name] = this;
}

list<Client *>::iterator User::clientLogin(Client *ptr) {
	clientList.push_front(ptr);
	ptr->pullList = fileList;
	return clientList.begin();
}

void User::clientLogout(const list<Client *>::iterator &it) {
	clientList.erase(it);
}

void User::clientPutFile(File *file, Client *client) {
	file->saveFileCommit(clientList.size() - 1);
	fileList.push_back(file);

	for(list<Client *>::iterator it = clientList.begin(); it != clientList.end(); ++it) {
		if(*it != client) {
			(*it)->pullList.push_back(file);
		}
	}
}

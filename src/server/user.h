#pragma once

#include <string>
#include <list>
#include <map>

class File;
class Client;

class User {
public:
	User(const std::string &name);
	std::list<Client *>::iterator clientLogin(Client *);
	void clientLogout(const std::list<Client *>::iterator &);
	void clientPutFile(File *, Client *);
	
	static std::map<std::string, User *> allUser;

private:
	std::string name;
	std::list<File *> fileList;
	std::list<Client *> clientList;
};

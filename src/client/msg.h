#pragma once
#include <string>
#include <deque>
#include <list>

class Msg {
public:
	void push(const std::string &);
	std::list<std::string>::iterator pushStatic();
	void removeStatic(std::list<std::string>::iterator);
	void refresh();

private:
	std::deque<std::string> strList;
	std::list<std::string> staticList;
	const static int maxSize;
};

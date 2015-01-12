#include "msg.h"
#include <cstdio>
#include <unistd.h>
#include <sys/ioctl.h>
#include <readline/readline.h>
using namespace std;

const int Msg::maxSize = 200;

void Msg::push(const string &str) {
	strList.push_back(str);
	if(strList.size() > maxSize)
		strList.pop_front();

	refresh();
}

list<string>::iterator Msg::pushStatic() {
	string s = "";
	staticList.push_back(s);
	list<string>::iterator it = staticList.end();
	return --it;
}

void Msg::removeStatic(list<string>::iterator it) {
	size_t pos = 0, oldpos = 0;
	while((pos = it->find('\n', oldpos)) != string::npos) {
		if(pos - oldpos > 0)
			push(it->substr(oldpos, pos - oldpos));
		oldpos = pos+1;
	}
	staticList.erase(it);
	refresh();
}

void Msg::refresh() {
	winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	int height = w.ws_row;
	int width = w.ws_col;

	printf("\033[2J\033[H"); fflush(stdout);
	for(list<string>::iterator it = staticList.begin(); it != staticList.end(); ++it) {
		fputs(it->c_str(), stdout);
	}
	fflush(stdout);

	printf("\033[44m");
	for(int i = 2; i < width; i++) {
		printf(" ");
	}
	printf("\033[m\n");
	fflush(stdout);

	height -= (staticList.size() * 3 + 5);
	for(
		int i = strList.size() > height ? strList.size() - height : 0;
		i < strList.size(); i++, height--)
	{
		puts(strList[i].c_str());
	}
	while(height--)
		puts("");
	fflush(stdout);

	printf("\n\n");
	printf("\033[42m");
	for(int i = 2; i < width; i++) {
		printf(" ");
	}
	printf("\033[m\n");
	fflush(stdout);

	rl_forced_update_display();
}

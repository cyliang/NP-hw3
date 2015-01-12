#include <cstdio>
#include <readline/readline.h>
#include <readline/history.h>
#include <cstdlib>

extern const char *cmds[];

char **readline_complete_func(const char *text, int start, int end);
static char *generator_func(const char *text, int state);
static const char **available_list;

char **readline_complete_func(const char *text, int start, int end) {
	if(start == 0) {
		rl_attempted_completion_over = 1;
		available_list = cmds;
		return rl_completion_matches(text, generator_func);
	}

	return (char **) NULL;
}

char *generator_func(const char *text, int state) {
	static const char **list;
	static int index;
	static int len;

	if(state == 0) {
		list = available_list;
		index = 0;
		len = strlen(text);
	}

	const char *content;
	while(content = list[index++]) {
		if(strncmp(text, content, len) == 0) {
			char *match = (char *) malloc(strlen(content) + 1);
			strcpy(match, content);
			return match;
		}
	}

	return (char *) NULL;
}

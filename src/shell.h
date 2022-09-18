#ifndef SHELL_H
#define SHELL_H

#include <stdbool.h>

typedef struct {
	char* prompt;
	bool terminate;
	int argc;
	char** argv;
	char** envp;
} Shell;

void run_shell(Shell* shell, int argc, char** argv, char** envp);

#endif

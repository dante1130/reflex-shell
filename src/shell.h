#ifndef SHELL_H
#define SHELL_H

#include <stdbool.h>

typedef struct {
	char* prompt;
	bool terminate;
} Shell;

void run_shell(Shell* shell);

#endif

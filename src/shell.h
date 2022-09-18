#ifndef SHELL_H
#define SHELL_H

#include <stdbool.h>

typedef struct {
	char* prompt;
	bool terminate;
} Shell;

void init_shell(Shell* shell);

void run_shell(Shell* shell);

#endif

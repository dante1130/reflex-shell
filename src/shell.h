#ifndef SHELL_H
#define SHELL_H

#include <stdbool.h>
#include "file_descriptor_helper.h"

/**
 * @struct Shell
 * @brief A simple custom unix shell, with a prompt and a command line.
 */
typedef struct {
	/// The prompt to display to the user.
	char* prompt;
	/// Whether the shell should terminate.
	bool terminate;
	/// The number of arguments passed to the shell.
	int argc;
	/// The arguments passed to the shell.
	char** argv;
	/// The environment variables passed to the shell.
	char** envp;
} Shell;

/**
 * @brief Runs the shell.
 *
 * @param shell The shell to run.
 * @param argc The number of arguments passed to the shell.
 * @param argv The arguments passed to the shell.
 * @param envp The environment variables passed to the shell.
 */
void run_shell(Shell* shell, int argc, char** argv, char** envp);

#endif

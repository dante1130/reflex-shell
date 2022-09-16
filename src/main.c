#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "string_utils.h"
#include "token.h"
#include "command.h"

void printCommand(Command c);
void runCommand(Command *c);

int main() {
	bool terminate = false;

	char *tokens[100];
	char input_line[100] = "Hello there everyone one\0";
	pid_t pid;

	Command commands[100];
	int nCommands = 0;

	do {
		printf("> ");
		if (fgets(input_line, 100, stdin) == NULL) {
			continue;
		}
		remove_newline(input_line);

		if (strcmp("exit", input_line) == 0) {
			terminate = true;
		}

		tokenise(&input_line[0], tokens);
		nCommands = separateCommands(tokens, &commands[0]);

		for (int count = 0; count < nCommands; count++) {
			pid = fork();
			if (pid == 0) {
				runCommand(&commands[count]);
			}
			// runCommand(&commands[count]);
			// printCommand(commands[count], tokens);
		}

		for (int count = 0; count < nCommands; count++) {
			wait((int *)0);
		}
	} while (!terminate);
}

void runCommand(Command *c) {
	execvp(c->argv[0], c->argv);
	exit(0);
}

void printCommand(Command c) {
	printf("Command: ");
	int index = 0;
	while (c.argv[index] != NULL) {
		printf("%s ", c.argv[index]);
		++index;
	}

	if (c.stdin_file != NULL) {
		printf("\tInputfile = %s", c.stdin_file);
	}
	if (c.stdout_file != NULL) {
		printf("\tOutputfile = %s", c.stdout_file);
	}

	printf("\n");
}

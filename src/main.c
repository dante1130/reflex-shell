#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "token.h"
#include "command.h"

void printCommand(Command c);
void runCommand(Command *c);

int main() {
	char *tokens[100];
	char inputLine[100] = "Hello there everyone one\0";
	int length = 0;
	pid_t pid;

	Command commands[100];
	int nCommands = 0;

	while (1) {
		printf("$ ");
		char *buff = fgets(inputLine, 100, stdin);
		if(buff == NULL) {
			perror("Fgets failed");
			break;
		}
		
		length = strlen(inputLine) - 1;
		if (inputLine[length] == '\n') {
			inputLine[length] = '\0';
		}

		if (strcmp("exit", inputLine) == 0) {
			break;
		}

		tokenise(&inputLine[0], tokens);
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
	}

	return 0;
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

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "string_utils.h"
#include "token.h"
#include "command.h"

void print_command(Command c);
void run_command(Command* c);

int main() {
	bool terminate = false;

	do {
		const size_t buffer_size = 256;
		char input_line[buffer_size];

		printf("> ");
		if (fgets(input_line, buffer_size, stdin) == NULL) {
			continue;
		}
		remove_newline(input_line);

		if (strcmp("exit", input_line) != 0) {
			const size_t max_tokens = 128;
			char* tokens[max_tokens];

			Command commands[max_tokens];

			tokenise(tokens, input_line, " ");
			const int command_size = tokenise_commands(commands, tokens);

			for (int i = 0; i < command_size; ++i) {
				pid_t pid = fork();
				if (pid == 0) {
					run_command(&commands[i]);
				}
				// run_command(&commands[count]);
				// print_command(commands[count], tokens);
			}

			for (int i = 0; i < command_size; ++i) {
				wait((int*)0);
			}
		} else {
			terminate = true;
		}
	} while (!terminate);
}

void run_command(Command* c) {
	execvp(c->argv[0], c->argv);
	exit(0);
}

void print_command(Command c) {
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

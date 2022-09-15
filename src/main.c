#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "string_utils.h"
#include "command.h"
#include "token.h"

int main() {
	bool terminate = false;

	do {
		const size_t buffer_size = 256;
		char line[buffer_size];

		printf("> ");
		if (fgets(line, buffer_size, stdin) == NULL) {
			continue;
		}
		remove_newline(line);

		if (strcmp(line, "exit") != 0) {
			const size_t max_tokens = 128;
			char* token[max_tokens];

			const size_t token_size = tokenise(token, line, " ");

			Command commands[token_size / 2];

			const int command_size =
			    tokenise_commands(commands, token, token_size);

			printf("Command size: %d\n", command_size);

			if (command_size >= 0) {
				for (int i = 0; i < command_size; ++i) {
					for (size_t j = commands[i].index_begin;
					     j < commands[i].index_end; ++j) {
						printf("%s ", token[j]);
					}
					printf("%s\n", commands[i].separator);
				}
			}

			for (size_t i = 0; i < token_size; ++i) {
				free(token[i]);
			}

			for (int i = 0; i < command_size; ++i) {
				free(commands[i].argv);
			}
		} else {
			terminate = true;
		}
	} while (!terminate);
}

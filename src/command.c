#include "command.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

bool is_separator(const char* token) {
	const size_t SEPARATOR_TYPE_SIZE = 3;
	const char* const SEPARATOR_TYPES[] = {"|", "&", ";"};

	for (size_t i = 0; i < SEPARATOR_TYPE_SIZE; ++i) {
		if (strcmp(token, SEPARATOR_TYPES[i]) == 0) {
			return true;
		}
	}

	return false;
}

int tokenise_commands(Command* commands, const char** tokens,
                      size_t token_size) {
	if (tokens == NULL || token_size == 0) {
		return 0;
	}

	// If the first token is a separator.
	if (is_separator(tokens[0])) {
		return -1;
	}

	int command_size = 0;

	size_t index_begin = 0;

	// Does not include the last token.
	for (size_t index_end = 0; index_end < token_size - 1; ++index_end) {
		if (is_separator(tokens[index_end])) {
			// If the current and previous tokens are both separators.
			if (strcmp(tokens[index_begin], tokens[index_end]) == 0) {
				return -2;
			}

			commands[command_size].index_begin = index_begin;
			commands[command_size].index_end = index_end;
			commands[command_size].separator = tokens[index_end];
			++command_size;

			index_begin = index_end + 1;
		}
	}

	// If the last token is a pipe separator.
	if (strcmp(tokens[token_size - 1], "|") == 0) {
		return -3;
	}

	// The last command.
	commands[command_size].index_begin = index_begin;
	commands[command_size].index_end = token_size;
	commands[command_size].separator = ";";
	++command_size;

	for (int i = 0; i < command_size; ++i) {
		search_redirection(&commands[i], tokens);
		build_argv(&commands[i], tokens);
	}

	return command_size;
}

void search_redirection(Command* command, const char** tokens) {
	command->stdin_file = NULL;
	command->stdout_file = NULL;

	for (size_t i = command->index_begin; i < command->index_end; ++i) {
		if (strcmp(tokens[i], "<") == 0) {
			command->stdin_file = tokens[++i];
		} else if (strcmp(tokens[i], ">") == 0) {
			command->stdout_file = tokens[++i];
		}
	}
}

void build_argv(Command* command, const char** tokens) {
	const size_t argv_size = command->index_end - command->index_begin;

	command->argv = malloc(sizeof(char*) * argv_size);
	assert(command->argv != NULL && "Failed to allocate memory for argv.");

	for (size_t i = 0; i < argv_size; ++i) {
		command->argv[i] = tokens[command->index_begin];
	}
}

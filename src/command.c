#include "command.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// static const size_t MAX_NUM_COMMANDS = 1000;

static const char* const PIPE_SEP = "|";
static const char* const CON_SEP = "&";
static const char* const SEQ_SEP = ";";

static void init_command(Command* command) {
	command->index_begin = 0;
	command->index_end = 0;
	command->separator = NULL;
	command->argv = NULL;
	command->stdin_file = NULL;
	command->stdout_file = NULL;
}

static void fill_command(Command* command, int first, int last, char* sep) {
	command->index_begin = first;
	command->index_end = last;
	command->separator = sep;
}

bool is_separator(const char* token) {
	const size_t SEPARATOR_TYPE_SIZE = 3;
	const char* const SEPARATOR_TYPES[] = {PIPE_SEP, CON_SEP, SEQ_SEP};

	for (size_t i = 0; i < SEPARATOR_TYPE_SIZE; ++i) {
		if (strcmp(token, SEPARATOR_TYPES[i]) == 0) {
			return true;
		}
	}

	return false;
}

void search_redirection(Command* command, char** tokens) {
	command->stdin_file = NULL;
	command->stdout_file = NULL;

	for (int i = command->index_begin; i < command->index_end; ++i) {
		if (strcmp(tokens[i], "<") == 0) {
			command->stdin_file = tokens[++i];
		} else if (strcmp(tokens[i], ">") == 0) {
			command->stdout_file = tokens[++i];
		}
	}
}

void build_argv(Command* command, char** tokens) {
	int max_size = command->index_end - command->index_begin + 2;
	if (command->stdin_file != NULL) {
		max_size += -2;
	}
	if (command->stdout_file != NULL) {
		max_size += -2;
	}

	command->argv = malloc(sizeof(char*) * max_size);
	if (command->argv == NULL) {
		perror("realloc");
		exit(1);
	}

	int last_index = 0;
	for (int i = command->index_begin; i < command->index_end; ++i) {
		if ((strcmp(tokens[i], ">") == 0) || (strcmp(tokens[i], "<") == 0)) {
			++i;
		} else {
			command->argv[last_index] = tokens[i];
			++last_index;
		}
	}
	command->argv[last_index] = NULL;
}

int tokenise_commands(Command* command, char** tokens) {
	// Basic set up
	int token_size = 0;
	{
		int token_count = 0;

		while (tokens[token_count] != NULL) {
			++token_count;
		}

		token_size = token_count;
	}

	// Return 0 if tokens are empty.
	if (token_size == 0) {
		return 0;
	}

	// Return -1 if the first token is a separator.
	if (is_separator(tokens[0])) {
		return -1;
	}

	if (!is_separator(tokens[token_size - 1])) {
		tokens[token_size] = (char*)SEQ_SEP;
		++token_size;
	}

	// Determining commands
	int first = 0;
	int num_commands = 0;

	for (int i = 0; i < token_size; ++i) {
		if (is_separator(tokens[i])) {
			// Return -2 if consecutive tokens are separators.
			if (first == i) {
				return -2;
			}

			init_command(&command[num_commands]);
			fill_command(&command[num_commands], first, i, tokens[i]);
			++num_commands;
			first = i + 1;
		}
	}

	// Return -3 if the last command separator is "|".
	if (strcmp(tokens[token_size - 1], PIPE_SEP) == 0) {
		return -3;
	}

	for (int i = 0; i < num_commands; ++i) {
		search_redirection(&command[i], tokens);
		build_argv(&command[i], tokens);
	}

	return num_commands;
}

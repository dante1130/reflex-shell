#include "command.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

void init_command(Command* cp) {
	cp->index_begin = 0;
	cp->index_end = 0;
	cp->separator = NULL;
	cp->argv = NULL;
	cp->stdin_file = NULL;
	cp->stdout_file = NULL;
}

void fill_command(Command* cp, int first, int last, char* sep) {
	cp->index_begin = first;
	cp->index_end = last;
	cp->separator = sep;
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

	command->argv = (char**)realloc(command->argv, sizeof(char*) * max_size);
	if (command->argv == NULL) {
		perror("realloc");
		exit(1);
	}

	int k = 0;
	for (int i = command->index_begin; i < command->index_end; ++i) {
		if ((strcmp(tokens[i], ">") == 0) || (strcmp(tokens[i], "<") == 0)) {
			++i;
		} else {
			command->argv[k] = tokens[i];
			++k;
		}
	}
	command->argv[k] = NULL;
}

int tokenise_commands(char* token[], Command* command) {
	// Basic set up
	int i = 0;
	int nTokens;

	while (token[i] != NULL) {
		++i;
	}
	nTokens = i;

	if (nTokens == 0) {
		return 0;
	}

	if (is_separator(token[0])) {
		return -3;
	}

	if (!is_separator(token[nTokens - 1])) {
		token[nTokens] = seqSep;
		++nTokens;
	}

	// Determining commands
	int first = 0;
	int last;
	char* sep;
	int c = 0;
	for (i = 0; i < nTokens; ++i) {
		last = i;
		if (is_separator(token[i])) {
			sep = token[i];
			if (first == last) {
				return -2;
			}
			init_command(&command[c]);
			fill_command(&command[c], first, last, sep);
			++c;
			first = i + 1;
		}
	}

	if (strcmp(token[last], pipeSep) == 0) {
		return -4;
	}

	int nCommands = c;
	for (int count = 0; count < nCommands; ++count) {
		search_redirection(&command[count], token);
		build_argv(&command[count], token);
	}

	return nCommands;
}

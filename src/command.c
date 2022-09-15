#include "command.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int separator(char *token) {
	int i = 0;
	char *commandSeparators[] = {pipeSep, conSep, seqSep, NULL};

	while (commandSeparators[i] != NULL) {
		if (strcmp(commandSeparators[i], token) == 0) {
			return 1;
		}
		++i;
	}

	return 0;
}

void initCommandStructure(Command *cp) {
	cp->first = 0;
	cp->last = 0;
	cp->sep = NULL;
	cp->argv = NULL;
	cp->stdin_file = NULL;
	cp->stdout_file = NULL;
}

void fillCommandStructure(Command *cp, int first, int last, char *sep) {
	cp->first = first;
	cp->last = last;
	cp->sep = sep;
}

void searchRedirection(char *token[], Command *command) {
	for (int count = command->first; count < command->last; ++count) {
		if (strcmp(token[count], "<") == 0) {
			++count;
			command->stdin_file = token[count];
		} else if (strcmp(token[count], ">") == 0) {
			++count;
			command->stdout_file = token[count];
		}
	}
}

void buildCommandArgumentArray(char *token[], Command *cp) {
	int max_size = cp->last - cp->first + 2;
	if (cp->stdin_file != NULL) {
		max_size += -2;
	}
	if (cp->stdout_file != NULL) {
		max_size += -2;
	}

	cp->argv = (char **)realloc(cp->argv, sizeof(char *) * max_size);
	if (cp->argv == NULL) {
		perror("realloc");
		exit(1);
	}

	int k = 0;
	for (int count = cp->first; count < cp->last; ++count) {
		if ((strcmp(token[count], ">") == 0) ||
		    (strcmp(token[count], "<") == 0)) {
			++count;
		} else {
			cp->argv[k] = token[count];
			++k;
		}
	}
	cp->argv[k] = NULL;

	return;
}

int separateCommands(char *token[], Command *command) {
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

	if (separator(token[0])) {
		return -3;
	}

	if (!separator(token[nTokens - 1])) {
		token[nTokens] = seqSep;
		++nTokens;
	}

	// Determining commands
	int first = 0;
	int last;
	char *sep;
	int c = 0;
	for (i = 0; i < nTokens; ++i) {
		last = i;
		if (separator(token[i])) {
			sep = token[i];
			if (first == last) {
				return -2;
			}
			initCommandStructure(&(command[c]));
			fillCommandStructure(&(command[c]), first, last, sep);
			++c;
			first = i + 1;
		}
	}

	if (strcmp(token[last], pipeSep) == 0) {
		return -4;
	}

	int nCommands = c;
	for (int count = 0; count < nCommands; ++count) {
		searchRedirection(token, &(command[count]));
		buildCommandArgumentArray(token, &(command[count]));
	}

	return nCommands;
}

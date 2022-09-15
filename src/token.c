#include "token.h"

#include <stdlib.h>
#include <string.h>

#include "string_utils.h"

size_t tokenise(char** token, const char* input_line, const char* delim) {
	if (input_line == NULL || token == NULL || delim == NULL) {
		return 0;
	}

	// The input line is duplicated as we don't want to modify the original.
	char* token_start = strdup(input_line);

	if (token_start == NULL) {
		return 0;
	}

	char* token_buf = strtok(token_start, delim);

	size_t token_size = 0;

	for (; token_buf != NULL; ++token_size) {
		token[token_size] = strdup(token_buf);
		token_buf = strtok(NULL, delim);
	}

	free(token_start);

	return token_size;
}

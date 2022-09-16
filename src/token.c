#include "token.h"

#include <stdlib.h>
#include <string.h>

size_t tokenise(char** token, char* input_line, const char* delim) {
	if (input_line == NULL || token == NULL || delim == NULL) {
		return 0;
	}

	token[0] = strtok(input_line, delim);

	size_t token_size = 1;

	for (; token[token_size] != NULL; ++token_size) {
		token[token_size] = strtok(NULL, delim);
	}

	return token_size;
}

#include "string_utils.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

void slice_string(char* dest, const char* src, size_t begin, size_t end) {
	assert(begin <= end && "begin must be less than or equal to end.");

	if (src == NULL || dest == NULL) {
		return;
	}

	size_t length = end - begin;
	strncpy(dest, src + begin, length);
	dest[length] = '\0';
}

void remove_newline(char* str) {
	size_t len = strlen(str);
	if (str[len - 1] == '\n') {
		str[len - 1] = '\0';
	}
}

char* strdup(const char* src) {
	if (src == NULL) {
		return NULL;
	}

	size_t slen = strlen(src);

	char* dup_str = malloc((slen + 1) * sizeof(char));

	if (dup_str == NULL) {
		return NULL;
	}

	memcpy(dup_str, src, slen + 1);
	return dup_str;
}
#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>

/**
 * @brief Takes a substring of a string.
 *
 * @param dest The destination string.
 * @param src The source string.
 * @param begin The index of the first character of the substring.
 * @param end The index of the last character of the substring.
 *
 * @pre dest != NULL
 * @pre src != NULL
 * @pre begin <= end
 * @pre strlen(str) >= end
 * @post dest != NULL
 */
void slice_string(char* dest, const char* src, size_t begin, size_t end);

/**
 * @brief Removes the new line character from the end of a string.
 *
 * @param str The string to remove the new line character from.
 */
void remove_newline(char* str);

/**
 * @brief Duplicate a string.
 *
 * @param src The source string to duplicate.
 * @return A pointer to the newly duplicated string, or NULL if there is an
 * error. The caller is responsible for freeing the memory.
 */
char* strdup(const char* src);

#endif
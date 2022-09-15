#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>

/**
 * @brief Tokenise a string into an array of strings based on a delimiter.
 *
 * @param token The array of strings to store the tokens in, the caller is
 * responsible for freeing the memory of each element.
 * @param input_line The input string to tokenise.
 * @param delim The delimiter to use.
 *
 * @pre input_line != NULL, token != NULL, delim != NULL.
 * @post token is populated with the tokens in input_line.
 *
 * @return The number of tokens in the array.
 */
size_t tokenise(char** token, const char* input_line, const char* delim);

#endif

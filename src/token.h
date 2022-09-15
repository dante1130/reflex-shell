/*
 * File: token.h
 * Author: Daniel Rodic
 * Date: 14/08/2022
 */

#define MAX_NUM_TOKENS 100
#define SEPERATOR " \t\n"

// Seperates an input (inputLine) into different tokens. Does this by
// storing the address for the first element into the token[] array.
//
// returns number of tokens stored or -1 if the number of tokens are
// greater than the limit defined
int tokenise(char *inputLine, char *token[]);

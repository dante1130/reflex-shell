/*
* File: token.c
* Author: Daniel Rodic
* Date: 14/08/2022
*/

#include <stdio.h>
#include <string.h>

#include "token.h"


int tokenise(char *inputLine, char *token[]) {
 int num = 0;

 token[0] = strtok(inputLine, SEPERATOR);

 while(token[num] != NULL) {
	num++;
	if(num == MAX_NUM_TOKENS) { return -1; }

	token[num] = strtok(NULL, SEPERATOR);
 }

 return num;
}

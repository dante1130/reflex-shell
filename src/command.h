#ifndef COMMAND_H
#define COMMAND_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @struct Command
 * @brief A command is a sequence of tokens separated by a separator. A command
 * contains indexes of the tokens in the original token string, as well as the
 * separator.
 */
typedef struct {
	/// The index of the beginning of the command in the original token.
	int index_begin;
	/// The index of the end of the command in the original token.
	int index_end;
	/// The separator.
	char* separator;
	/// The command arguments.
	char** argv;
	/// Points to the file name for stdin redirection.
	char* stdin_file;
	/// Points to the file name for stdout redirection.
	char* stdout_file;
} Command;

/**
 * @brief Checks whether the token is a separator.
 *
 * @param token The token to check.
 * @return true if the token is a separator ("|", "&", ";"), false otherwise.
 */
bool is_separator(const char* token);

/**
 * @brief Breaks up the tokens into an array of commands.
 *
 * @param commands The array of commands, the caller is responsible for calling
 * free_command() on each command in the array.
 * @param tokens The tokens to break up.
 * @param token_size The number of tokens.
 *
 * @pre tokens != NULL
 * @pre token_size > 0
 * @pre array size of token must match with token_size.
 * @pre commands != NULL
 * @post commands is populated.
 *
 * @return The size of the array of commands, or negative if an error occurred.
 * @retval 0 If the tokens were empty.
 * @retval -1 The first token is a separator.
 * @retval -2 Consecutive tokens are separators.
 * @retval -3 The last command separator is "|".
 */
int tokenise_commands(char* token[], Command* command);

/**
 * @brief Search for stdin and stdout redirection in the command.
 *
 * @param command A pointer to a single command structure.
 * @param tokens The token array.
 */
void search_redirection(Command* command, char** tokens);

/**
 * @brief Builds the command line argument array for the command.
 *
 * @param command A pointer to a single command structure.
 * @param tokens The token array.
 */
void build_argv(Command* command, char** tokens);

#endif

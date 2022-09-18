#include "shell.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#include "string_utils.h"
#include "token.h"
#include "command.h"

void init_shell(Shell* shell, int argc, char** argv, char** envp);
bool prompt_input(const char* prompt, char* input_buffer, size_t buffer_size);
bool wait_process(pid_t pid);
void run_command(Command* c);
void sig_init();
void catch_sig();
bool builtin_command(Command *c, char** envp, Shell* shell);

void run_shell(Shell* shell, int argc, char** argv, char** envp) {
	init_shell(shell, argc, argv, envp);
	sig_init();

	do {
		const size_t buffer_size = 256;
		char input_buffer[buffer_size];

		if (!prompt_input(shell->prompt, input_buffer, buffer_size)) {
			continue;
		}

		if (strcmp("exit", input_buffer) != 0) {
			const size_t max_tokens = 128;
			char* tokens[max_tokens];

			Command commands[max_tokens];

			tokenise(tokens, input_buffer, " ");
			const int command_size = tokenise_commands(commands, tokens);

			for (int i = 0; i < command_size; ++i) {
				pid_t pid = fork();
				if (pid == 0) {
					if(!builtin_command(&commands[i], envp, shell)) {
						run_command(&commands[i]);
					}
					exit(1);
				} else if (strcmp(commands[i].separator, ";") == 0) {
					wait_process(pid);
				} else if (strcmp(commands[i].separator, "&") == 0) {
					continue;
				}
			}
		} else {
			shell->terminate = true;
		}
	} while (!shell->terminate);
}

void init_shell(Shell* shell, int argc, char** argv, char** envp) {
	shell->prompt = "> ";
	shell->terminate = false;
	shell->argc = argc;
	shell->argv = argv;
	shell->envp = envp;
}

bool prompt_input(const char* prompt, char* input_buffer, size_t buffer_size) {
	bool reprompt = false;

	do {
		reprompt = false;

		printf("%s", prompt);
		if (fgets(input_buffer, buffer_size, stdin) == NULL) {
			if (errno == EINTR) {
				reprompt = true;
			} else {
				return false;
			}
		}
		remove_newline(input_buffer);
	} while (reprompt);

	return true;
}

void run_command(Command* c) { execvp(c->argv[0], c->argv); }

bool wait_process(pid_t pid) {
	int status = 0;
	if (waitpid(pid, &status, 0) == -1) {
		perror("waitpid");
		return false;
	}

	return true;
}

void sig_init() {
	struct sigaction act_catch, act_ignore;
	// Catch signals
	act_catch.sa_flags = 0;
	act_catch.sa_handler = catch_sig;
	sigemptyset(&act_catch.sa_mask);

	sigaction(SIGINT, &act_catch, NULL);
	sigaction(SIGQUIT, &act_catch, NULL);
	sigaction(SIGTSTP, &act_catch, NULL);

	// Ignore signals
	act_ignore.sa_flags = 0;
	act_ignore.sa_handler = SIG_IGN;
	sigemptyset(&act_ignore.sa_mask);
	// sigaction(SIGALRM, &act_ignore, NULL);
}

void catch_sig() { printf(" \n"); }

bool builtin_command(Command *c, char** envp, Shell* shell) {
	const char* const BUILTIN_TYPES[] = {"prompt", "pwd", "cd"};
	bool valid_command = false;

	//prompt
	if(strcmp(c->argv[0], BUILTIN_TYPES[0]) == 0) {
		valid_command = true;
		printf("prompt command found...\n");
	}

	//pwd
	if(strcmp(c->argv[0], BUILTIN_TYPES[1]) == 0) {
		valid_command = true;
		printf("pwd command found...\n");
	}

	//cd
	if(strcmp(c->argv[0], BUILTIN_TYPES[2]) == 0) {
		valid_command = true;
		printf("cd command found...\n");
	}

	return valid_command;
}
#include "shell.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

#include "string_utils.h"
#include "token.h"
#include "command.h"

static void run_command(Command* c);
static void sig_init();
static void catch_sig();

void init_shell(Shell* shell) {
	shell->prompt = "> ";
	shell->terminate = false;
}

void run_shell(Shell* shell) {
	init_shell(shell);
	sig_init();

	do {
		const size_t buffer_size = 256;
		char input_line[buffer_size];

		printf("%s", shell->prompt);
		if (fgets(input_line, buffer_size, stdin) == NULL) {
			continue;
		}
		remove_newline(input_line);

		if (strcmp("exit", input_line) != 0) {
			const size_t max_tokens = 128;
			char* tokens[max_tokens];

			Command commands[max_tokens];

			tokenise(tokens, input_line, " ");
			const int command_size = tokenise_commands(commands, tokens);

			for (int i = 0; i < command_size; ++i) {
				pid_t pid = fork();
				if (pid == 0) {
					run_command(&commands[i]);
				}
			}

			for (int i = 0; i < command_size; ++i) {
				wait((int*)0);
			}
		} else {
			shell->terminate = true;
		}
	} while (!shell->terminate);
}

void run_command(Command* c) {
	execvp(c->argv[0], c->argv);
	exit(0);
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

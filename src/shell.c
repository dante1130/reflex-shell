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

//Running commands
void run_command(Command* c);
bool builtin_command(Command *c, char** envp, Shell* shell);
void pwd_command(char** envp);

//Signals
void sig_init();
void catch_sig(int signo);

//Claiming child zombies
void claim_zombies();

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

bool builtin_command(Command *c, char** envp, Shell* shell) {
	bool valid_command = false;

	if(c->argv[0] == NULL) {
		return false;
	}

	//prompt
	if(strcmp(c->argv[0], "prompt") == 0) {
		valid_command = true;
		printf("prompt command found... NOT IMPLEMENTED YET\n");
		if(c->argv[1] == NULL) { return false; }
		shell->prompt = c->argv[1];
	}

	//pwd
	if(strcmp(c->argv[0], "pwd") == 0) {
		valid_command = true;
		pwd_command(envp);
	}

	//cd
	if(strcmp(c->argv[0], "cd") == 0) {
		valid_command = true;
		printf("cd command found...\n");
	}

	return valid_command;
}

void catch_sig(int signo) {
	if(signo == SIGCHLD) {
		claim_zombies();
	} else {
		printf("\n");
	}
}

void claim_zombies() {
	bool more = true;
	pid_t pid;
	int status;

	while(more) {
		pid = waitpid(-1, &status, WNOHANG);
		if(pid <= 0) {
			more = false;
		}
	}
}

void pwd_command(char** envp) {
	char pwd_key[4];
	pwd_key[3] = '\0';
	int count = 0;

	while(envp[count] != NULL) {
		slice_string(pwd_key, envp[count], 0, 3);
		if(strcmp(pwd_key, "PWD") == 0) {
			char pwd[1000];
			slice_string(pwd, envp[count], 4, strlen(envp[count]));
			printf("%s\n", pwd);
			break;
		}
		++count;
 	}
}
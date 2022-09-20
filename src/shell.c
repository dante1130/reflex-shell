#include "shell.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>

#include "string_utils.h"
#include "token.h"
#include "command.h"

void init_shell(Shell* shell, int argc, char** argv, char** envp);
bool prompt_input(const char* prompt, char* input_buffer, size_t buffer_size);
bool wait_process(pid_t pid);

// Running commands
void handle_commands(Shell* shell, Command* commands, int command_size);
void run_command(Command* command);
bool builtin_command(Command* command, char** envp, Shell* shell);
void pwd_command(char** envp);
void cd_command(char** envp, char* path);
bool file_redirection(Command* command);

// Signals
void sig_init();
void catch_sig(int signo);

// Claiming child zombies
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

			handle_commands(shell, commands, command_size);
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

void handle_commands(Shell* shell, Command* commands, int command_size) {
	for (int i = 0; i < command_size; ++i) {
		pid_t pid = fork();
		if (pid == 0) {
			if (!file_redirection(&commands[i])) {
				continue;
			}

			if (!builtin_command(&commands[i], shell->envp, shell)) {
				run_command(&commands[i]);
			}
			exit(1);
		} else if (strcmp(commands[i].separator, ";") == 0) {
			wait_process(pid);
		} else if (strcmp(commands[i].separator, "&") == 0) {
			continue;
		}
	}
}

void run_command(Command* command) { execvp(command->argv[0], command->argv); }

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

bool builtin_command(Command* command, char** envp, Shell* shell) {
	bool valid_command = false;

	if (command->argv[0] == NULL) {
		return false;
	}

	// prompt
	if (strcmp(command->argv[0], "prompt") == 0) {
		valid_command = true;
		printf("prompt command found... NOT IMPLEMENTED YET\n");
		if (command->argv[1] == NULL) {
			return false;
		}
		shell->prompt = command->argv[1];
	}

	// pwd
	if (strcmp(command->argv[0], "pwd") == 0) {
		valid_command = true;
		pwd_command(envp);
	}

	// cd
	if (strcmp(command->argv[0], "cd") == 0) {
		valid_command = true;
		printf("cd command found...\n");
	}

	return valid_command;
}

void catch_sig(int signo) {
	if (signo == SIGCHLD) {
		claim_zombies();
	} else {
		printf("\n");
	}
}

void claim_zombies() {
	bool more = true;
	pid_t pid;
	int status;

	while (more) {
		pid = waitpid(-1, &status, WNOHANG);
		if (pid <= 0) {
			more = false;
		}
	}
}

void pwd_command(char** envp) {
	char pwd_key[4];
	pwd_key[3] = '\0';

	for (int i = 0; envp[i] != NULL; ++i) {
		slice_string(pwd_key, envp[i], 0, 3);
		if (strcmp(pwd_key, "PWD") == 0) {
			char pwd[1000];
			slice_string(pwd, envp[i], 4, strlen(envp[i]));
			printf("%s\n", pwd);
			break;
		}
	}
}

void cd_command(char** envp, char* path) {}

bool file_redirection(Command* command) {
	if (command->stdin_file != NULL && command->stdout_file != NULL) {
		if (strcmp(command->stdin_file, command->stdout_file) == 0) {
			printf("Invalid redirection: input file is output file\n");
			return false;
		}
	}

	if (command->stdin_file != NULL) {
		int fd_input = open(command->stdin_file, O_RDONLY | O_CREAT, 0777);
		if (fd_input == -1) {
			printf("Failed to open/create %s for reading...\n",
			       command->stdin_file);
			return false;
		}
		dup2(fd_input, STDIN_FILENO);
	}

	if (command->stdout_file != NULL) {
		int fd_output = open(command->stdout_file, O_WRONLY | O_CREAT, 0777);
		if (fd_output == -1) {
			printf("Failed to open/create %s for writing...\n",
			       command->stdout_file);
			return false;
		}
		dup2(fd_output, STDOUT_FILENO);
	}

	return true;
}
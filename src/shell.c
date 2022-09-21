#include "shell.h"

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <limits.h>

#include "string_utils.h"
#include "token.h"
#include "command.h"
#include "file_descriptor_helper.h"

void init_shell(Shell* shell, int argc, char** argv, char** envp);
bool prompt_input(const char* prompt, char* input_buffer, size_t buffer_size);
bool wait_process(pid_t pid);
void exit_process(file_descriptors* fds);

// Running commands
void run_sequential(Command* command, Shell* shell, file_descriptors* fds);
void run_concurrent(Command* command, Shell* shell, file_descriptors* fds);
void run_external_command(char** cmd_argv, char** envp);
bool builtin_command(Command* command, Shell* shell);
void pwd_command();
void cd_command(char* directory);

// Redirection
bool file_redirection(Command* command, file_descriptors* fds);
void pipe_redirection(Command* command, file_descriptors* fds);

// Signals
void sig_init();
void catch_sig(int signo);

// Claiming child zombies
void claim_zombies();

void run_shell(Shell* shell, int argc, char** argv, char** envp) {
	init_shell(shell, argc, argv, envp);
	sig_init();

	file_descriptors fds;
	init_file_descriptors(&fds);

	do {
		const size_t buffer_size = 256;
		char input_buffer[buffer_size];

		// Get command input
		if (!prompt_input(shell->prompt, input_buffer, buffer_size)) {
			continue;
		}

		// If exit quit
		if (strcmp("exit", input_buffer) == 0) {
			shell->terminate = true;
			free(shell->prompt);
			continue;
		}

		// Run commands
		const size_t max_tokens = 128;
		char* tokens[max_tokens];

		Command commands[max_tokens];

		tokenise(tokens, input_buffer, " ");
		const int command_size = tokenise_commands(commands, tokens);

		for (int i = 0; i < command_size; ++i) {
			// Reset back to terminal and sets current & next file
			// descriptors
			set_current_file_descriptors(&fds);
			pipe_redirection(&commands[i], &fds);

			if (strcmp(commands[i].separator, ";") == 0) {  // If ;
				run_sequential(&commands[i], shell, &fds);
			} else {  // If & or |
				run_concurrent(&commands[i], shell, &fds);
			}
		}
		// Reset back to terminal
		reset_file_descriptors(&fds);

	} while (!shell->terminate);
}

void init_shell(Shell* shell, int argc, char** argv, char** envp) {
	shell->prompt = malloc(sizeof(char));
	shell->prompt[0] = '>';
	shell->terminate = false;
	shell->argc = argc;
	shell->argv = argv;
	shell->envp = envp;
}

bool prompt_input(const char* prompt, char* input_buffer, size_t buffer_size) {
	bool reprompt = false;

	printf("%s ", prompt);
	do {
		reprompt = false;
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

void run_sequential(Command* command, Shell* shell, file_descriptors* fds) {
	if (!file_redirection(command, fds)) {
		return;
	}

	set_std_file_descriptors(fds);

	if (!builtin_command(command, shell)) {
		pid_t pid = fork();
		if (pid == 0) {
			run_external_command(command->argv, shell->envp);
			exit_process(fds);
		} else {
			wait_process(pid);
		}
	}
}

void run_concurrent(Command* command, Shell* shell, file_descriptors* fds) {
	pid_t pid = fork();
	if (pid != 0) {
		return;
	}

	if (!file_redirection(command, fds)) {
		exit_process(fds);
	}

	set_std_file_descriptors(fds);

	if (!builtin_command(command, shell)) {
		run_external_command(command->argv, shell->envp);
	}

	exit_process(fds);
}

void run_external_command(char** cmd_argv, char** envp) {
	execvpe(cmd_argv[0], cmd_argv, envp);
}

bool wait_process(pid_t pid) {
	int status = 0;
	if (waitpid(pid, &status, 0) == -1) {
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
	sigaction(SIGCHLD, &act_catch, NULL);

	// Ignore signals
	act_ignore.sa_flags = 0;
	act_ignore.sa_handler = SIG_IGN;
	sigemptyset(&act_ignore.sa_mask);
	// sigaction(SIGALRM, &act_ignore, NULL);
}

bool builtin_command(Command* command, Shell* shell) {
	bool valid_command = false;

	if (command->argv[0] == NULL) {
		return false;
	}

	// prompt
	if (strcmp(command->argv[0], "prompt") == 0) {
		valid_command = true;
		if (command->argv[1] == NULL) {
			return false;
		}
		if (shell->prompt != NULL) {
			free(shell->prompt);
		}
		shell->prompt = strdup(command->argv[1]);
	}

	// pwd
	if (strcmp(command->argv[0], "pwd") == 0) {
		valid_command = true;
		pwd_command();
	}

	// cd
	if (strcmp(command->argv[0], "cd") == 0) {
		valid_command = true;
		if (command->argv[1] == NULL) {
			cd_command(getenv("HOME"));
		} else {
			cd_command(command->argv[1]);
		}
	}

	return valid_command;
}

void catch_sig(int signo) {
	if (signo == SIGCHLD) {
		claim_zombies();
	}
}

void claim_zombies() {
	bool more = true;

	while (more) {
		int status;
		pid_t pid = waitpid(-1, &status, WNOHANG);
		if (pid <= 0) {
			more = false;
		}
	}
}

void pwd_command() {
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		puts(cwd);
	} else {
		perror("getcwd");
	}
}

void cd_command(char* directory) {
	if (chdir(directory) != 0) {
		perror("chdir");
	}
}

bool file_redirection(Command* command, file_descriptors* fds) {
	if (command->stdin_file != NULL && command->stdout_file != NULL) {
		if (strcmp(command->stdin_file, command->stdout_file) == 0) {
			printf("Invalid redirection: input file is output file\n");
			return false;
		}
	}

	if (command->stdin_file != NULL) {
		int fd_input = open(command->stdin_file, O_RDONLY, 0777);
		if (fd_input == -1) {
			printf("Failed to open/create %s for reading...\n",
			       command->stdin_file);
			return false;
		}
		fds->curr_fd_input = fd_input;
	}

	if (command->stdout_file != NULL) {
		int fd_output = open(command->stdout_file, O_WRONLY | O_CREAT | O_EXCL, 0777);
		if (fd_output <= -1) {
			if(errno == EEXIST) {
				remove(command->stdout_file);
				fd_output = open(command->stdout_file, O_WRONLY | O_CREAT, 0777);
				if(fd_output >= 0) {
					fds->curr_fd_output = fd_output;
				} else {
					printf("Failed to open/create %s for writing...\n", command->stdout_file);
					return false;
				}
			} else {
				printf("Failed to open/create %s for writing...\n", command->stdout_file);
				return false;
			}
		} else {
			fds->curr_fd_output = fd_output;

		}
	}

	return true;
}

void pipe_redirection(Command* command, file_descriptors* fds) {
	// If this command has | seperator
	if (strcmp(command->separator, "|") == 0) {
		int p[2];
		if (pipe(p) < 0) {
			perror("Pipe call");
			fds->curr_fd_input = dup(fds->std_fd_input);
			fds->curr_fd_output = dup(fds->std_fd_output);
		}
		fds->next_fd_input = p[0];
		fds->curr_fd_output = p[1];
	}
}

void exit_process(file_descriptors* fds) {
	close_file_descriptors(fds);
	exit(0);
}
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

//Running commands
void run_command(Command* c);
bool builtin_command(Command *c, char** envp, Shell* shell);
void pwd_command(char** envp);
bool file_redirection(Command* c);
void pipe_redirection(Command* c, int* pipe_fd, int* p2);

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

			int pipe_fd[2] = {-1, -1};
			int prev_pipe_fd = -1;
			//printf("\ncommand size: %d\n", command_size);
			for (int i = 0; i < command_size; ++i) {
				prev_pipe_fd = pipe_fd[0];
				pipe_redirection(&commands[i], &pipe_fd[0], &pipe_fd[1]);
				pid_t pid = fork();
				if (pid == 0) {
					if(pipe_fd[1] != -1) {
						//printf("Chaning output for: %s\n", commands[i].argv[0]);
						dup2(pipe_fd[1], STDOUT_FILENO);
					}
					if(prev_pipe_fd != -1) {
						//printf("Chaning input for: %s\n", commands[i].argv[0]);
						dup2(prev_pipe_fd, STDIN_FILENO);
					}

					if(!file_redirection(&commands[i])) {
						continue;
					}

					if(!builtin_command(&commands[i], envp, shell)) {
						run_command(&commands[i]);
					}
					exit(0);
				} else if (strcmp(commands[i].separator, ";") == 0) {
					wait_process(pid);
				} else if (strcmp(commands[i].separator, "&") == 0) {
					continue;
				} else if (strcmp(commands[i].separator, "|") == 0) {
					wait_process(pid);
					close(pipe_fd[1]);
				}

				if(prev_pipe_fd != -1) {
					close(prev_pipe_fd);
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

void run_command(Command* c) { execvp(c->argv[0], c->argv); exit(1); }

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

	for(int count = 0; envp[count] != NULL; ++count) {
		slice_string(pwd_key, envp[count], 0, 3);
		if(strcmp(pwd_key, "PWD") == 0) {
			char pwd[1000];
			slice_string(pwd, envp[count], 4, strlen(envp[count]));
			printf("%s\n", pwd);
			break;
		}
 	}
}

bool file_redirection(Command *c) {
	int fd_input, fd_output;

	if(c->stdin_file != NULL && c->stdout_file != NULL) {
		if(strcmp(c->stdin_file, c->stdout_file) == 0) {
			printf("Invalid redirection: input file is output file\n");
			return false;
		}
	}

	if(c->stdin_file != NULL) {
		fd_input = open(c->stdin_file, O_RDONLY | O_CREAT, 0777);
		if(fd_input == -1) {
			printf("Failed to open/create %s for reading...\n", c->stdin_file);
			return false;
		}
		dup2(fd_input, STDIN_FILENO);
	}

	if(c->stdout_file != NULL) {
		fd_output = open(c->stdout_file, O_WRONLY | O_CREAT, 0777);
		if(fd_output == -1) {
			printf("Failed to open/create %s for writing...\n", c->stdout_file);
			return false;
		}
		dup2(fd_output, STDOUT_FILENO);
	}

	return true;
}

void pipe_redirection(Command* c, int* pipe_fd, int* p2) {
	//If this command has | seperator
	if(strcmp(c->separator, "|") == 0) {
		int p[2];
		if(pipe(p) < 0) {
			perror("Pipe call");
			*pipe_fd = -1;
			*p2 = -1;
		}
		*pipe_fd = p[0];
		*p2 = p[1];
	} else {
		*pipe_fd = -1;
		*p2 = -1;
	}
}
#include "shell.h"

int main(int argc, char** argv, char** envp) {
	Shell shell;
	run_shell(&shell, argc, argv, envp);
}

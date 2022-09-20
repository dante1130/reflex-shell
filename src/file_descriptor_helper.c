#include "file_descriptor_helper.h"


#include <unistd.h>

void init_file_descriptors(struct file_descriptors* fds) {
    fds->std_fd_input = dup(STDIN_FILENO);
	fds->std_fd_output = dup(STDOUT_FILENO);
}

void reset_file_descriptors(struct file_descriptors* fds) {
    close(fds->curr_fd_input);
    close(fds->curr_fd_output);
    dup2(fds->std_fd_input, STDIN_FILENO);
    dup2(fds->std_fd_output, STDOUT_FILENO);
}

void close_file_descriptors(struct file_descriptors* fds) {
    close(fds->curr_fd_input);
	close(fds->curr_fd_output);
	close(fds->std_fd_input);
	close(fds->std_fd_output);
	close(fds->next_fd_input);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
}

void set_current_file_descriptors(struct file_descriptors* fds) {
    reset_file_descriptors(fds);
    fds->curr_fd_input = dup(STDIN_FILENO);
    fds->curr_fd_output = dup(STDOUT_FILENO);
    if(fds->next_fd_input != -1) {
        fds->curr_fd_input = dup(fds->next_fd_input);
        fds->next_fd_input = -1;
    }
}

void set_std_file_descriptors(struct file_descriptors* fds) {
    dup2(fds->curr_fd_input, STDIN_FILENO);
    dup2(fds->curr_fd_output, STDOUT_FILENO);
}
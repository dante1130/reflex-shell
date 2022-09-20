#ifndef FILE_DESCRIPTOR_HELPER_H
#define FILE_DESCRIPTOR_HELPER_H

typedef struct file_descriptors {
	/// @brief The current file descriptor which is used for input
	int curr_fd_input;
	/// @brief The current file descriptor which is used for output
	int curr_fd_output;
	/// @brief The default (standard) file descriptor which is used for input
	int std_fd_input;
	/// @brief The default (standard) file descriptor which is used for output
	int std_fd_output;
	/// @brief The next file descriptor which is used for input (used for | operator)
	int next_fd_input;
} FILE_DESCRIPTOR;

/**
 * @brief
 * @param
 */
void init_file_descriptors(struct file_descriptors* fds);

/**
 * @brief
 * @param
 */
void reset_file_descriptors(struct file_descriptors* fds);

/**
 * @brief
 * @param
 */
void close_file_descriptors(struct file_descriptors* fds);

/**
 * @brief
 * @param
 */
void set_current_file_descriptors(struct file_descriptors* fds);

/**
 * @brief
 * @param
 */
void set_std_file_descriptors(struct file_descriptors* fds);

#endif
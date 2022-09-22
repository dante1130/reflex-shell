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
	/// @brief The next file descriptor which is used for input (used for |
	/// operator)
	int next_fd_input;
} file_descriptors;

/**
 * @brief Initialises the file descriptor values
 * @param fds The struct storing all information requried for the file
 * descriptors
 */
void init_file_descriptors(file_descriptors* fds);

/**
 * @brief Closes current file desciptors and sets the terminal back to std input
 * and output
 * @param fds The struct storing all information requried for the file
 * descriptors
 */
void reset_file_descriptors(file_descriptors* fds);

/**
 * @brief Closes all possible file descriptors including stdin and stdout
 * @param fds The struct storing all information requried for the file
 * descriptors
 */
void close_file_descriptors(file_descriptors* fds);

/**
 * @brief Sets the current file descriptors back to terminal stdio and stdout,
 * and moves next input to current input
 * @param fds The struct storing all information requried for the file
 * descriptors
 */
void set_current_file_descriptors(file_descriptors* fds);

/**
 * @brief Sets the stdin and stdout to the current file desciptors in fds
 * @param fds The struct storing all information requried for the file
 * descriptors
 */
void set_std_file_descriptors(file_descriptors* fds);

#endif
#define MAX_NUM_COMMANDS 1000

#define pipeSep "|"
#define conSep "&"
#define seqSep ";"

typedef struct Command {
 int first;
 int last;
 char *sep;
 char **argv;
 char *stdin_file;
 char *stdout_file;
} Command;

//Seperates tokens into seperate commands
//Returns number of commands found
int separateCommands(char *token[], Command *command);

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)

int tokenize_command(char* buff, char* tokens[])
{
	int i = 0;
	tokens[i++] = &buff[0];
	for(int buff_ind=0; buff_ind<COMMAND_LENGTH; buff_ind++)
	{
		if(buff[buff_ind-1]==' ')
			tokens[i++] = &buff[buff_ind];
	}
}

/**
* Read a command from the keyboard into the buffer 'buff' and tokenize it
* such that 'tokens[i]' points into 'buff' to the i'th token in the command.
* buff: Buffer allocated by the calling code. Must be at least
* COMMAND_LENGTH bytes long.
* tokens[]: Array of character pointers which point into 'buff'. Must be at
* least NUM_TOKENS long. Will strip out up to one final '&' token.
* 'tokens' will be NULL terminated.
* in_background: pointer to a boolean variable. Set to true if user entered
* an & as their last token; otherwise set to false.
*/
void read_command(char* buff, char* tokens[], _Bool* in_background)
{
	*in_background = false;

	// Read input
	int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);
	if ( (length < 0) && (errno !=EINTR) )
	{
		perror("Unable to read command. Terminating.\n");
		exit(-1); /* terminate with error */
	}

	// Null terminate and strip \n.
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}


	// Tokenize (saving original command string)
	int token_count = tokenize_command(buff, tokens);
	if (token_count == 0) {
		return;
	}

	// Extract if running in background:
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
		*in_background = true;
		tokens[token_count - 1] = 0;
	}
}

/**
* Main and Execute Commands
*/
int main(int argc, char* argv[])
{
	char input_buffer[COMMAND_LENGTH];
	char* tokens[NUM_TOKENS];
	while (true) {
		// Get command
		// Use write because we need to use read()/write() to work with
		// signals, and they are incompatible with printf().
		write(STDOUT_FILENO, "> ", strlen("> "));
		_Bool in_background = false;
		read_command(input_buffer, tokens, &in_background);

		/**
		* Steps For Basic Shell:
		* 1. Fork a child process
		* 2. Child process invokes execvp() using results in token array.
		* 3. If in_background is false, parent waits for
		* child to finish. Otherwise, parent loops back to
		* read_command() again immediately.
		*/
	}
	return 0;
}
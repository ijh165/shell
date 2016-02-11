#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2+1)

int tokenize_command(char* buff, char* tokens[]){
	int position = 0;
	char *token = strtok(buff, " \t\r\n\a");
	while(token != NULL){
		tokens[position] = token;
		position++;
		token = strtok(NULL, " \t\r\n\a");
	}
	tokens[position] = NULL;
	return position;
}
void read_command(char *buff, char *tokens[], _Bool *in_background)
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
	//printf("String: %s\n", buff);
	int token_count = tokenize_command(buff, tokens);
		//printf("%d\n", token_count);
	if (token_count == 0) {
	return;
	}
	
	// Extract if running in background:
	//int ret = 0;
	//ret = strcmp(tokens[token_count - 1], "&");
	//printf("here\n");
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
		//printf("there\n");
	*in_background = true;

	//printf("%d\n", token_count);
	tokens[token_count - 1] = 0;
	}	
	//return;
}

int main(int argc, char* argv[])
{
	char input_buffer[COMMAND_LENGTH];
	char *tokens[NUM_TOKENS];
	while (true) { 
		write(STDOUT_FILENO, "> ", strlen( "> "));
		_Bool in_background = false;
		read_command(input_buffer, tokens, &in_background);
		pid_t pid;
		pid_t wait_pid;
		int stat_val;
		pid = fork();
		if (pid < 0) {
			perror("SHELL ERROR (FORKING FAIL)");
		}
		else if (pid == 0) {
			if (execvp(tokens[0], tokens) == -1) {
				perror("SHELL ERROR");
			}
			return;
		}
		else {
			do {
				wait_pid = waitpid(pid, &stat_val, WUNTRACED);
			} while (!WIFEXITED(stat_val) && !WIFSIGNALED(stat_val));
		}
	}

	return 0;
}
	

// Filename: shell.c
// Created by Ivan Jonathan Hoo and Raymond Chan
// Copyright (c) 2016 Ivan Jonathan Hoo and Raymond Chan. All rights reserved.

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

//int defns
#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2+1)
#define HISTORY_DEPTH 10

//string defns
#define SHELL_EXIT "Exiting shell\n"
#define SHELL_ERR "SHELL ERROR"
#define FORK_FAIL_ERR "SHELL ERROR (FORKING FAIL)"
#define CANNOT_READ_CMD_ERR "Unable to read command. Terminating.\n"
#define GET_CWD_ERR "getcwd() error\n"
#define NO_PREV_CMD_ERR "SHELL ERROR: No previous command\n"
#define NOT_INTEGER_ERR "SHELL ERROR: Please input an integer after !\n"
#define CMD_NOT_EXIST "command doesn't exist\n"

//global variables
int cmd_count;
char* history[HISTORY_DEPTH];



//free each elements of history[] (prevent memory leak)
void free_history()
{
	for(int i=0; i<HISTORY_DEPTH; i++) {
		free(history[i]);
		history[i] = NULL;
	}
}

//update the history str arr
void update_history(char* buff)
{
	int index = cmd_count % HISTORY_DEPTH;
	if (cmd_count>=10) {
		free(history[index]);
		history[index] = NULL;
	}
	history[index] = strdup(buff);
	cmd_count++;
}

//prints the history
void print_history()
{
	if(cmd_count<=10) {
		for(int i = 0; i<cmd_count; i++)
		{
			char* num_str = malloc(16);
			snprintf(num_str, 16, "%d", i+1);
			write(STDOUT_FILENO, num_str, strlen(num_str));
			write(STDOUT_FILENO, ". ", strlen(". "));
			write(STDOUT_FILENO, history[i], strlen(history[i]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
			//free memory
			free(num_str);
			num_str = NULL;
		}
	}
	else {
		for(int i = cmd_count-10; i<cmd_count; i++)
		{
			char* num_str = malloc(16);
			snprintf(num_str, 16, "%d", i+1);
			write(STDOUT_FILENO, num_str, strlen(num_str));
			write(STDOUT_FILENO, ". ", strlen(". "));
			write(STDOUT_FILENO, history[i % HISTORY_DEPTH], strlen(history[i % HISTORY_DEPTH]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
			//free memory
			free(num_str);
			num_str = NULL;
		}
	}
}

//split the buff into tokens
int tokenize_command(char* buff, char* tokens[])
{
	int i = 0;
	char* token = strtok(buff, " \t\r\n\a");
	while(token != NULL)
	{
		tokens[i] = token;
		i++;
		token = strtok(NULL, " \t\r\n\a");
	}
	tokens[i] = NULL;
	return i;
}

//parse the input command
void parse_input(char* buff, int length, char* tokens[], _Bool* in_background)
{
	*in_background = false;

	// Null terminate and strip \n.
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}

	// Update history only if user enters something
	if (strlen(buff) != 0) {
		update_history(buff);
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
	// Read input
	int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);
	if ( (length < 0) && (errno !=EINTR) )
	{
		perror(CANNOT_READ_CMD_ERR);
		exit(-1); /* terminate with error */
	}

	// Parse input
	parse_input(buff, length, tokens, in_background);
}

//function to execute user commands
void exec_cmd(char* tokens[], _Bool in_background)
{
	// internal commands
	if (strcmp(tokens[0], "exit") == 0) {
		write(STDOUT_FILENO, SHELL_EXIT, strlen(SHELL_EXIT));
		free_history();
		exit(0);
	}
	else if (strcmp(tokens[0], "pwd") == 0) {
		char cwd[COMMAND_LENGTH];
		if (getcwd(cwd, sizeof(cwd)) != NULL) {
			write(STDOUT_FILENO, cwd, strlen(cwd));
			write(STDOUT_FILENO, "\n", strlen("\n"));
		}
		else {
			perror(GET_CWD_ERR);
		}
		return;
	}
	else if (strcmp(tokens[0], "cd") == 0) {
		if (chdir(tokens[1]) != 0) {
			perror(SHELL_ERR);
		}
		return;
	}
	else if (strcmp(tokens[0], "history") == 0) {
		print_history();
		return;
	}
	else if (strcmp(tokens[0], "!!") == 0) {
		cmd_count--;
		if(cmd_count == 0) {
			write(STDOUT_FILENO, NO_PREV_CMD_ERR, strlen(NO_PREV_CMD_ERR));
		}
		else {
			char* tmp_cmd_str = history[(cmd_count-1)%HISTORY_DEPTH];
			write(STDOUT_FILENO, tmp_cmd_str, strlen(tmp_cmd_str));
			write(STDOUT_FILENO, "\n", strlen("\n"));
			parse_input(tmp_cmd_str, strlen(tmp_cmd_str), tokens, &in_background);
			exec_cmd(tokens, in_background);
		}
		return;
	}
	else if (tokens[0][0]== '!') {
		cmd_count--;
		if(cmd_count == 0) {
			write(STDOUT_FILENO, NO_PREV_CMD_ERR, strlen(NO_PREV_CMD_ERR));
		}
		else {
			char* num_str = malloc(sizeof(char)*(strlen(tokens[0])-1));
			for(int i=1, j=0; i<strlen(tokens[0]); i++, j++)
				num_str[j] = tokens[0][i];
			num_str[strlen(tokens[0])] = '\0';
			int num = atoi(num_str);
			if (num == 0) {
				write(STDOUT_FILENO, NOT_INTEGER_ERR, strlen(NOT_INTEGER_ERR));
			}
			else if (num > cmd_count) {
				write(STDOUT_FILENO, SHELL_ERR, strlen(SHELL_ERR));
				write(STDOUT_FILENO, ": ", strlen(": "));
				write(STDOUT_FILENO, num_str, strlen(num_str));
				switch (num) {
					case 1:
						write(STDOUT_FILENO, "st ", strlen("st "));
						break;
					case 2:
						write(STDOUT_FILENO, "nd ", strlen("nd "));
						break;
					case 3:
						write(STDOUT_FILENO, "rd ", strlen("rd "));
						break;
					default:
						write(STDOUT_FILENO, "th ", strlen("th "));
				}
				write(STDOUT_FILENO, CMD_NOT_EXIST, strlen(CMD_NOT_EXIST));
			}
			else {
				char* tmp_cmd_str = history[(num-1)%HISTORY_DEPTH];
				write(STDOUT_FILENO, tmp_cmd_str, strlen(tmp_cmd_str));
				write(STDOUT_FILENO, "\n", strlen("\n"));
				parse_input(tmp_cmd_str, strlen(tmp_cmd_str), tokens, &in_background);
				exec_cmd(tokens, in_background);
			}
			//free memory
			free(num_str);
			num_str = NULL;
		}
		return;
	}

	/**
	 * Steps For Basic Shell:
	 * 1. Fork a child process
	 * 2. Child process invokes execvp() using results in token array.
	 * 3. If in_background is false, parent waits for
	 * child to finish. Otherwise, parent loops back to
	 * read_command() again immediately.
	 */
	int stat_val;
	pid_t pid = fork();
	if (pid < 0) {
		perror(FORK_FAIL_ERR);
	}
	else if (pid == 0) {
		if (execvp(tokens[0], tokens) == -1) {
			perror(SHELL_ERR);
			exit(-1);
		}
	}
	else if (!in_background) {
		// Wait for child to finish...
		do {
			pid_t wait_pid = waitpid(pid, &stat_val, WUNTRACED);
		} while (!WIFEXITED(stat_val) && !WIFSIGNALED(stat_val));


	}
	// Cleanup zombie processes
	while (waitpid(-1, NULL, WNOHANG) > 0);
}

//handle ctrl+c signal
void handle_SIGINT()
{
	write(STDOUT_FILENO, "\n", strlen("\n"));
	print_history();
}

/**
* Main and Execute Commands
*/
int main(int argc, char* argv[])
{
	/* set up the signal handler */
	struct sigaction handler;
	handler.sa_handler = handle_SIGINT;
	handler.sa_flags = 0;
	sigemptyset(&handler.sa_mask);
	sigaction(SIGINT, &handler, NULL);

	char input_buffer[COMMAND_LENGTH];
	char* tokens[NUM_TOKENS];
	cmd_count = 0;

	//event loop
	while (true)
	{
		// Get command
		// Use write because we need to use read()/write() to work with
		// signals, and they are incompatible with printf().
		char cwd[COMMAND_LENGTH];
		if (getcwd(cwd, sizeof(cwd)) != NULL) {
			write(STDOUT_FILENO, cwd, strlen(cwd));
		}
		else {
			perror(GET_CWD_ERR);
		}
		write(STDOUT_FILENO, "> ", strlen( "> "));
		_Bool in_background = false/*, internal_cmd = false*/;
		read_command(input_buffer, tokens, &in_background);

		//do nothing if user enter nothing
		if (strlen(input_buffer) == 0) {
			continue;
		}

		//execute user commands
		exec_cmd(tokens, in_background);

		//flush input_buffer (prevent executing last command upon ctrl+c)
		input_buffer[0] = '\n';
		for(int i=1; i<COMMAND_LENGTH; i++)
			input_buffer[i] = '\0';
	}

	

	return 0;
}

/*Matt Sampson, February 15th, 2016
This program is a shell for executing programs. Processes can be started, output and
input can be controlled with the >, <, and | operators.*/
#define _GNU_SOURCE
#include <stdio.h>       /* Input/Output */
#include <stdlib.h>      /* General Utilities */
#include <unistd.h>      /* Symbolic Constants */
#include <sys/types.h>   /* Primitive System Data Types */
#include <sys/wait.h>    /* Wait for Process Termination */
#include <errno.h>       /* Errors */
#include <string.h>     /* Strings */
#include <signal.h>
#include <ctype.h>
#include <stdbool.h>

/*This function takes the user input and parses it
Preconditions: input contains users input, args has been allocated
Postconditions: args will contain an array of input seperated and without end line character
Returns: the number of arguments found.*/
int parseInput(char *input, char **args) {
	char *token;
	int argCount = 0;

	token = strtok(input," \n");
	while (token != NULL){
		char *temp = malloc(sizeof(char)*10);
		strcpy(temp,token);
		token = strtok(NULL, " \n");
		args[argCount] = temp;
		argCount++;
	}
	return argCount;
}

/*This function checks if the input line has an &
Preconditions: user input has already been parsed into args
Postconditions: The & will be removed from the user input
Returns: a 1 if the & is found, otherwise a 0*/
int runInBackground(char **args, int status) {
	if ((*args[status - 1]) == '&') {
		args[status - 1] = NULL;
		return 1;
	}
	return 0;
}

/*This function determines if any file redirection is required
Preconditions: user input has already been parsed into args
Postconditions: The redirection symbol will beremoved from the args
Returns: a 1 for >, a 2 for <, and 3 for | and an int that says where the second
command starts. returns a 0 if no redirection is desired.*/
int redirectToFromFile(int runInBG, char **args, int status, int *commandTwo) {
	for (int i = 0; i < status - runInBG; i++) {
		if ((*args[i]) == '>') {
			args[i] = NULL;
			return 1;
		}
		else if ((*args[i]) == '<') {
			args[i] = NULL;
			return 2;
		}
		else if ((*args[i]) == '|') {
			args[i] = NULL;
			(*commandTwo) = i + 1;
			return 3;
		}
	}
	return 0;
}

/*This function opens the file to write the output of the command to
Preconditions: args has already been parsed
Postconditions: The command has run and written the output to a file
Returns: The status after execution to ensure it executed properly*/
int writeOptToFile(int runInBG, char **args, int status) {
	FILE *fp;
	int execStatus;
	if (runInBG == 1)
		fp = freopen(args[status - 2], "w", stdout);
	else 
		fp = freopen(args[status - 1], "w", stdout);
	execStatus = execvp(*args,args);
	fclose(fp);
	return execStatus;
}

/*This function opens the file and reads the file as input to the command
Preconditions: args has already been parsed
Postconditions: The command has run with input from the file
Returns: The status after execution to ensure it executed properly*/
int readIptFromFile(int runInBG, char **args, int status) {
	FILE *fp;
	int execStatus;
	if (runInBG == 1)
		fp = freopen(args[status - 2], "r", stdin);
	else 
		fp = freopen(args[status - 1], "r", stdin);
	execStatus = execvp(*args,args);
	fclose(fp);
	return execStatus;
}

//this is the signal catcher for when the child process finishes executing
void endSignal(int signal, siginfo_t *signalInfo, void *hold) {

	if (signal == SIGCHLD) {
		printf("[Process completed]\n");
	}
}

//This is the main loop that runs the shell and takes in the commands
int main(int argc, char const *argv[])
{
	char *input;
	pid_t childpid, pipeChild;
	int status, execStatus, commandTwo;
	int runInBG = 0, redirToFile = 0, pipefd[2];
	struct sigaction childFinished;

	childFinished.sa_sigaction = endSignal;

	input = malloc(sizeof(char)*50);
	printf("Welcome to MaSh, enter a command.\n");
	printf(">");
	fgets(input,50,stdin);
	sigaction(SIGCHLD, &childFinished, NULL);
	while(strcmp(input,"exit\n") != 0) {
		runInBG = 0;
		redirToFile = 0;
		commandTwo = 0;
		char **args = calloc(10,sizeof(char*));
		//parse user input into argument array
		status = parseInput(input,args);
		if (status > 1)
			runInBG = runInBackground(args,status);
		if (status > 2)
			redirToFile = redirectToFromFile(runInBG,args,status, &commandTwo);
		childpid = fork();
		if (childpid >= 0) { //fork success
			if (childpid == 0) {
				if (redirToFile == 1) {
					execStatus = writeOptToFile(runInBG,args,status);
				}
				else if (redirToFile == 2) {
					execStatus = readIptFromFile(runInBG,args,status);
				}
				else if (redirToFile == 3) {
					//create pipe
					if (pipe(pipefd) == -1) {
						printf("Pipe failed.\n");
					}
					pipeChild = fork();
					if (pipeChild >= 0) {
						//read from the pipe
						if (pipeChild == 0) {
							close(1);
							dup(pipefd[1]);
							close(pipefd[0]);
							execStatus = execvp(*args,args);
							exit(0);
						}
						//write to the pipe
						else {
							close(0);
							dup(pipefd[0]);
							close(pipefd[1]);
							execStatus = execvp(args[commandTwo],args+commandTwo);
						}
					}
					else {
						perror("Fork failed.");
						exit(-1);
					}
				}
				else {
					//this is supposed to make the output of commands not appear on screen, it doesnt work though
					if (runInBG == 1) {
						setpgid(0,0);
					}
					execStatus = execvp(*args,args);
				}
				if (execStatus < 0) {
					printf("Command \"%s\" not found.\n", input);
				}
				exit(0);
			}
			else {
				//may not need to do this
				if (runInBG == 1) {
					waitpid(-1,&status,WNOHANG | WUNTRACED);
					runInBG = 0;
				}
				else
					waitpid(childpid,&status,0);			
			}
		}
		else {
			printf("Fork failed.\n");
		}
		printf(">");
		fgets(input,50,stdin);
		free(args);
	}
	kill(childpid,SIGKILL);
	printf("logout\n");
	printf("\n");
	printf("[Process completed]\n");
	exit(0);
}
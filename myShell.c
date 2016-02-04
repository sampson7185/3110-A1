#define _GNU_SOURCE
#include <stdio.h>       /* Input/Output */
#include <stdlib.h>      /* General Utilities */
#include <unistd.h>      /* Symbolic Constants */
#include <sys/types.h>   /* Primitive System Data Types */
#include <sys/wait.h>    /* Wait for Process Termination */
#include <errno.h>       /* Errors */
#include <string.h>     /* Strings */

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

int runInBackground(char **args, int status) {
	if ((*args[status - 1]) == '&') {
		args[status - 1] = NULL;
		return 1;
	}
	return 0;
}

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

/*int readIptFromPipe(int pipe, char **args, int commandTwo) {
	FILE *fp;
	int execStatus;
	fp = fdopen(pipe, "r");
	execStatus = execvp(*args[commandTwo],args[commandTwo]);
}*/

int writeOptToPipe(int pipe, char **args) {
	FILE *fp;
	int execStatus;

	fp = fdopen(pipe, "w");
	execStatus = execvp(*args,args);
	fclose(fp);
	return execStatus;
}

int main(int argc, char const *argv[])
{
	char *input = malloc(sizeof(char)*50);
	pid_t childpid, pipeChild;
	int status, execStatus, commandTwo;
	int runInBG = 0, redirToFile = 0, pipefd[2];

	printf("Welcome to MaSh, enter a command or type h for help.\n");
	printf(">");
	fgets(input,50,stdin);
	while(strcmp(input,"exit\n") != 0) {
		runInBG = 0;
		redirToFile = 0;
		char **args = malloc(sizeof(char*)*10);
		childpid = fork();
		if (childpid >= 0) { //fork success
			status = parseInput(input,args);
			if (status > 1)
				runInBG = runInBackground(args,status);
			if (status > 2)
				redirToFile = redirectToFromFile(runInBG,args,status, &commandTwo);
			if (childpid == 0) {
				if (redirToFile == 1) {
					execStatus = writeOptToFile(runInBG,args,status);
				}
				else if (redirToFile == 2) {
					execStatus = readIptFromFile(runInBG,args,status);
				}
				//write code to pipe here
				else if (redirToFile == 3) {
					//create pipe
					if (pipe(pipefd) == -1) {
						printf("Pipe failed.\n");
					}
					pipeChild = fork();
					if (pipeChild >= 0) {
						if (pipeChild == 0) {
							close(pipefd[1]);
							exit(0);
						}
						else {
							close(pipefd[0]);
							execStatus = writeOptToPipe(pipefd[1],args);
						}
					}
					else {
						printf("Fork failed.\n");
					}
				}
				else
					execStatus = execvp(*args,args);
				if (execStatus < 0) {
					printf("Command \"%s\" not found.\n", input);
				}
				exit(0);
			}
			else {
				if (runInBG == 1) {
					waitpid(-1,&status,WNOHANG | WUNTRACED);
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
	printf("logout\n");
	printf("\n");
	printf("[Process completed]\n");
	exit(0);
}
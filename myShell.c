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

int main(int argc, char const *argv[])
{
	char *input = malloc(sizeof(char)*50);
	pid_t childpid;
	int status, execStatus, runInBG;

	printf("Welcome to MaSh, enter a command or type h for help.\n");
	printf(">");
	fgets(input,50,stdin);
	while(strcmp(input,"exit\n") != 0) {
		char **args = malloc(sizeof(char*)*10);
		childpid = fork();
		if (childpid >= 0) { //fork success
			status = parseInput(input,args);
			if ((*args[status - 1]) == '&') {
				runInBG = 1;
				args[status - 1] = NULL;
			}
			if (childpid == 0) {
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
		printf(">");
		fgets(input,50,stdin);
		free(args);
	}
	printf("logout\n");
	printf("\n");
	printf("[Process completed]\n");
	exit(0);
}
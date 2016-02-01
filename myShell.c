#include <stdio.h>       /* Input/Output */
#include <stdlib.h>      /* General Utilities */
#include <unistd.h>      /* Symbolic Constants */
#include <sys/types.h>   /* Primitive System Data Types */
#include <sys/wait.h>    /* Wait for Process Termination */
#include <errno.h>       /* Errors */
#include <string.h>     /* Strings */

int parseInput(char *input, char **args) {
	char *temp = malloc(sizeof(char)*10);
	strcpy(temp,"dicks");
	args[0] = temp;
	return 0;
}

int main(int argc, char const *argv[])
{
	char *input = malloc(sizeof(char)*50);
	char *args[64];
	pid_t childpid;
	int status;

	printf("Welcome to SampShell, enter a command or type h for help.\n");
	printf(">");
	scanf("%s", input);
	while(strcmp(input,"exit") != 0) {
		childpid = fork();
		if (childpid >= 0) { //fork success
			status = parseInput(input,args);
			//printf("%s\n", args[0]);
			if (childpid == 0) {
				exit(0);
			}
		}
		printf(">");
		scanf("%s", input);
	}
	printf("logout\n");
	exit(0);
}
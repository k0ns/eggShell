#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
       
#define MAX_LINE_LENGTH 513
#define MAX_ARG_LENGTH	8
#define MAX_ARG_NO		16
#define MAX_COM_NO		32
#define MAX_COM_LENGTH	MAX_ARG_NO*MAX_ARG_LENGTH

#define GREEN "\x1B[32m"
#define RESET "\x1B[0m"

void interactive();
void batch(char **argv);
int exec(char **args);
void parse_line(char *line,int *delim,char **commands);
int parse_command(char *command, char **args);
int egg(char *line);

int main(int argc, char **argv){
	
	printf("Welcome to EggShell!\n");
	
	if(argc == 1){
		interactive();
	}
	else if(argc == 2){
		batch(argv);
	}
	else{
		printf("Too many arguments.\nExiting...\n");
		printf("For Interactive Mode : No arguments needed\nFor Batch mode : 1 argument specifying batch file path");
		exit(1);
	}
	
}

void interactive(){

	char *line = (char *)malloc(MAX_LINE_LENGTH*sizeof(char));
	if(!line){
		printf("Allocation Error. Exitting...");
		exit(1);
	}
	
	int stop;	
	
	do{
		printf(GREEN "kokolakis_8415> " RESET);
		fgets(line,MAX_LINE_LENGTH,stdin);
		stop = egg(line);
	}while(!stop);
	
	free(line);
}

void batch(char **argv){

	//OPEN BATCH FILE
	FILE *fp;

	if(!(fp = fopen(argv[1],"r"))){
		printf("Error Opening File\nExiting...\n");
		exit(1);
	}
	
	char *line = (char *)malloc(MAX_LINE_LENGTH*sizeof(char));
	if(!line){
		printf("Allocation Error. Exitting...");
		exit(1);
	}
	
	int stop;	
	
	printf("\n\nSTARTING BATCH JOB\n\n");
	do{
		if(fgets(line,MAX_LINE_LENGTH,fp) == NULL)		break;
		stop = egg(line);
	}while(!stop);
	printf("\n\nJOB FINISHED. GOOD BYE...\n\n");
	
	free(line);
	fclose(fp);
}

int exec(char **args){
	
	int status;
	pid_t pid;

	
	if(!(pid = fork())){
		//CHILD
		if(execvp(args[0],args)){
			printf("Error. Could not execute command\n");
			exit(EXIT_FAILURE);
		}
	}
	else if(pid > 0){
		//PARENT
		wait(&status);
		if(WEXITSTATUS(status) == EXIT_FAILURE){
			printf("Execution Failed\nExit Status: %d\n", WEXITSTATUS(status));
			return EXIT_FAILURE;
		}
		else{
			return EXIT_SUCCESS;
		}
	}
	else{
		printf("Execution Failed.\nExiting...");
		return EXIT_FAILURE;
	}
}

#define AMPER 1
#define SEMI  2
void parse_line(char *line,int *delim,char **commands){	
	
	char *dup = strdup(line);
	//PARSE INPUT
	
	commands[0] = strtok(line,";&");
	
	//FIND DELIMITER IF IT EXISTS
	switch(dup[strlen(commands[0])]){
		case '&':
			*delim = AMPER;
			break;
		case ';':
			*delim = SEMI;
			break;
		default:
			*delim = 0;
	}
	
	int it = 1;
	do{
		if((commands[it++] = strtok(NULL,";&")) == NULL)	break;
	}while(it<MAX_COM_NO);
	
	free(dup);
}

#define QUIT 1
#define EMPTY 2
int parse_command(char *command, char **args){
	
	char *token;
	
	if(!strcmp(command,"quit\n")){
		return QUIT;
	}
	else if(command[0] == '\n' || command[0] == '\0'){
		return EMPTY;
	}
	else{
		//PARSE COMMAND
		token = strtok(command," \n");
		int it = 0;
		do{
			args[it++] = token;
			if(it == MAX_ARG_NO-1)	break;
			
			token = strtok(NULL," \n");
		}while(token != NULL);
		args[it] = NULL;
		return 0;
	}
}


int egg(char *line){

	char **commands = (char **)calloc(MAX_COM_NO,sizeof(char *));
	char *command = (char *)malloc(MAX_COM_LENGTH*sizeof(char));
	char **args = (char **)malloc(MAX_ARG_NO*sizeof(char *));
	if(!commands || !command || !args){
		printf("Allocation Error. Exitting...");
		exit(1);
	}
		
	int delim;
	
	parse_line(line,&delim,commands);
	
	//LOOP
	
	int status;
	int quit = 0;
	int i = 0;
	int cmd_check;
	do{
		strcpy(command,commands[i++]);
		cmd_check = parse_command(command,args);
		
		if(cmd_check == QUIT){
			quit = 1;
			break;
		}
		else if(cmd_check == EMPTY){
			status = EXIT_SUCCESS;
		}
		else{
			status = exec(args);		
		}
		
	}while(commands[i] && (delim == SEMI || status == EXIT_SUCCESS));

	//FREE UP MEMORY
	free(args);
	free(commands);
	free(command);
	return quit;
}

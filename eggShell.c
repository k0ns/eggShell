#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

//MAX ARRAY SIZES
#define MAX_LINE_LENGTH 513
#define MAX_ARG_LENGTH	8
#define MAX_ARG_NO	16
#define MAX_COM_NO	32
#define MAX_COM_LENGTH	MAX_ARG_NO*MAX_ARG_LENGTH


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
		printf("For Interactive Mode : No arguments needed\nFor Batch mode : 1 argument specifying batch file path\n");
		exit(1);
	}
	
}





//COLOURS FOR THE PROMPT
#define GREEN "\x1B[32m"
#define RESET "\x1B[0m"

void interactive(){
	
	//MALLOC LINE
	char *line = (char *)malloc(MAX_LINE_LENGTH*sizeof(char));
	if(!line){
		printf("Allocation Error. Exitting...");
		exit(1);
	}
	
	
	//PRINT PROMPT, READ LINE, EXECUTE COMMANDS IN LINE
	//LOOP UNTIL QUIT RETURNED FROM EGG FUNCTION
	int quit;	
	do{
		printf(GREEN "kokolakis_8415> " RESET);
		fgets(line,MAX_LINE_LENGTH,stdin);
		quit = egg(line);
	}while(!quit);
	
	free(line);
}

void batch(char **argv){

	char *line = (char *)malloc(MAX_LINE_LENGTH*sizeof(char));
	if(!line){
		printf("Allocation Error. Exitting...");
		exit(1);
	}
	
	//OPEN BATCH FILE
	FILE *fp;
	if(!(fp = fopen(argv[1],"r"))){
		printf("Error Opening File\nExiting...\n");
		exit(1);
	}
	int fd = fileno(fp);
	off_t pos;

	int quit;	
	printf("\n\nSTARTING BATCH JOB\n\n");
	//READ AND EXECUTE LINE BY LINE
	//STOP WHEN NO MORE LINES OR QUIT RETURNED BY EGG
	//SAVE POSITION IN FILE AND RESET AFTER EACH EXECUTION - FORKING MESSES UP FILE POINTER
	do{
		if(fgets(line,MAX_LINE_LENGTH,fp) == NULL)		break;
		pos = lseek(fd,0,SEEK_CUR);
		quit = egg(line);
		lseek(fd,pos,SEEK_SET);
	}while(!quit);
	printf("\n\nJOB FINISHED. GOOD BYE...\n\n");
	
	free(line);
	fclose(fp);
}

//EXECUTE A SIGNLE COMMAND
int exec(char **args){
	
	int status;
	pid_t pid;

	
	if(!(pid = fork())){
		//CHILD
		if(execvp(args[0],args) == -1){
			exit(EXIT_FAILURE);
		}
		exit(EXIT_SUCCESS);
	}
	else if(pid > 0){
		//PARENT
		wait(&status);
		if(WIFEXITED(status) && WEXITSTATUS(status) == EXIT_FAILURE){
			printf("Execution Failed\nContinuing...\n");
			return EXIT_FAILURE;
		}
		else{
			return EXIT_SUCCESS;
		}
	}
	else{
		printf("Execution Failed.\nContinuing...\n");
		return EXIT_FAILURE;
	}
}

//DEFINE VALUES TO TELL DELIMITERS APART
#define AMPER 1
#define SEMI  2
//INPUT = LINES
//OUTPUT = COMMANDS, DELIM
void parse_line(char *line,int *delim,char **commands){	
	
	char *dup = strdup(line);
	
	
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
	
	//CONTINUE PARSING
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
		//CHECK FOR QUIT COMMAND
		return QUIT;
	}
	else if(command[0] == '\n' || command[0] == '\0'){
		//CHECK FOR EMPTY COMMAND
		return EMPTY;
	}
	else{
		//ELSE PARSE COMMAND
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




/*CENTRAL FUNCTION

INPUT = LINE


1. PARSE LINE
2. PARSE COMMANDS IN LINE
3. EXECUTE COMMANDS


RETURN QUIT = 1 IF 'QUIT' COMMAND
*/
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
	
	//EXECUTE COMMANDS IN LINE
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

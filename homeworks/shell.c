#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

#define MAX_USER_IN 100
#define MAX_NUM_ARGS 5
#define MAX_ARG_LEN 20

//put string user types 
static char in_buf[MAX_USER_IN];
//parse the string to arguments
static char parse_args[MAX_NUM_ARGS][MAX_ARG_LEN];

//given code; keyboard input to buffer
int user_in ();
//your implementation: buffer (1t arg) to parsed args (2nd arg)
int parse_cmd (char* in_buf, char parse_args[MAX_NUM_ARGS][MAX_ARG_LEN]);
//your implementation: execute using parsed args (2nd args)
void execute_cmd (int num_args, char parse_args[MAX_NUM_ARGS][MAX_ARG_LEN]);


void main()
{
	int num_args;

	while((num_args = user_in()) != EOF)
	{
		execute_cmd(num_args, parse_args);
	}
	printf("\n");
}


int user_in ()
{
	int c, i;
	char cwd[1024];

	//initialization
	int count = 0;
	memset(in_buf, 0, MAX_USER_IN);
	for(i=0; i<MAX_NUM_ARGS; i++) {
		memset(parse_args[i], 0, MAX_ARG_LEN);
	}

	//display current directory prompt
	if (getcwd(cwd, sizeof(cwd)) != NULL)
		printf("%s$ ", cwd);

	//fill in_buf from user input
	while(1)
	{
		//get a character 
		c = getchar();
		in_buf[count] = c;
	
		//ctrl+D, then exit
		if(c  == EOF) {
			return EOF;
		}
	
		//return, then stop user input
		if(c == '\n') {
			in_buf[count] = '\0';
			break;
		}

		count ++;
	}
	
	//call parse_cmd()
	int num_args = parse_cmd(in_buf, parse_args);
	return num_args;
}


/**
  * get a string (in_buf) and parse it into multiple arguments (parse_args) 
  * input: in_buf (1st arg)
  * output1: parse_args (2nd args)
  * output2: num_args (return val)
**/

int parse_cmd(char *in_buf, char parse_args[MAX_NUM_ARGS][MAX_ARG_LEN])
{
	int i = 0;
	int fd[2];
	int num_args = MAX_NUM_ARGS;
	char in_buf_c[MAX_USER_IN];
	strcpy(in_buf_c, in_buf);
	pid_t pid;
	char *basecamp;
	char *command_front[MAX_USER_IN];
	char *command_end[MAX_USER_IN];
	char *tok;
	int bg;
	int j;
	int k;
	char *p_front=NULL;
	char *p_end=NULL;
	int status;
	//printf("input:%s\n", in_buf);
	
	/**tok = strtok(in_buf, " ");

	while (tok != NULL)
	{
		strcpy(parse_args[i++], tok);
		tok = strtok(NULL," ");
	}**/

	//num_args = i;
	//printf("output:\n");
	//for(i=0; i<num_args; i++) {
	//	printf("arg[%d]:%s\n", i, parse_args[i]);
	//}
	
	//background execution
	if(strchr(in_buf_c, '&')!=NULL){
                bg = -1;
		int status;
		in_buf_c[strlen(in_buf_c)] = '\0';
		i=0;
		basecamp = strtok(in_buf_c," ");
		while(basecamp != NULL)
		{
			if(strcmp(basecamp, "&")==0)
				bg=i;
			command_front[i]=basecamp;
			basecamp=strtok(NULL," ");
			i++;
		}
		command_front[i]='\0';
		command_front[bg]=NULL;
		pid = fork();
		if(command_front[0]!=NULL)
		{
			if(pid==0)
			{
				execvp(command_front[0], command_front);
				exit(0);
			}
		  	waitpid(-1, &status, WNOHANG);
		}
		//waitpid(-1, &status, WNOHANG);
		printf("%d\n", pid);

        }

	//pipe
	else if(strchr(in_buf_c, '|')!=NULL){
		
		pid_t pid;
		char *p_front = strtok(in_buf_c, "|");
		char *p_end = strtok(NULL, "|");
		strcat(p_front, "\0");
		strcat(p_end, "\0");
		j=0;
		basecamp=strtok(p_front, " ");
		while(basecamp != NULL)
		{
			command_front[j]=basecamp;
			basecamp=strtok(NULL," ");
			j++;
		}
		command_front[j]='\0';
		k=0;
		basecamp=strtok(p_end, " ");
		while(basecamp != NULL)
		{
			command_end[k]=basecamp;
			basecamp=strtok(NULL," ");
			k++;
		}
		command_end[k]='\0';
		if(pipe(fd)==-1){
			perror("pipe error.\n");
			exit(1);
		}
		switch (fork()){
			case -1:
				perror("fork1 error.\n");
				break;
			case 0:
				dup2(fd[1],1);	
				close(fd[0]);
				close(fd[1]);
				execvp(command_front[0], command_front);
				exit(0);
				
		}
		switch (fork()){
			case -1:
				perror("fork2 error.\n");	
				break;
			case 0:
				dup2(fd[0],0);
				close(fd[1]);
				close(fd[0]);
				execvp(command_end[0], command_end);
				exit(0);
		}
		close(fd[0]);
		close(fd[1]);
		while(wait(NULL)!=-1);
	}


	//change directory
	else{
		tok=strtok(in_buf," ");
       	        while (tok != NULL)
        	{
                	strcpy(parse_args[i++], tok);
               	       	tok = strtok(NULL," ");
       		 }

       	        num_args=i;
		
		if(strcmp(parse_args[0],"cd")==0){
			if(strcmp(parse_args[1],"")==0 || strcmp(parse_args[1],"~")==0)
                	        chdir(getenv("HOME"));
               	        else{
				if(chdir(parse_args[1])==0)
					;
				else
					printf("%s is an incorrect directory.\n", parse_args[1]);

			}
       		 }
	}
	return num_args;
}


/**
  * get parsed args (parse_args) and execute using them
  * input1: num_args (1st arg)
  * input2: parse_args (2nd args)
**/
void execute_cmd(int num_args, char parse_args[MAX_NUM_ARGS][MAX_ARG_LEN])
{
	int i = 0;
	int status;
	char *args[num_args+1];
	pid_t pid;

	for (;i < num_args;i++)
		args[i] = parse_args[i];

	args[i] = NULL;

	pid = fork();

	switch(pid)
	{
		case -1:
			perror("Fork Error");
			break;
		case 0:
			execvp(args[0], args);
			break;
		default:
			waitpid(pid,&status,0);
	}
}

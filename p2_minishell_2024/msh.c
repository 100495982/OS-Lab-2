//P2-SSOO-23/24

//  MSH main file
// Write your msh source code here

//#include "parser.h" ??
#include <stddef.h>			/* NULL */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_COMMANDS 8 // the maximum number of commands


// files in case of redirection
char filev[3][64];

//to store the execvp second parameter
char *argv_execvp[8];

void siginthandler(int param)
{
	printf("****  Exiting MSH **** \n");
	//signal(SIGINT, siginthandler); ??????
	exit(0);
}

/* myhistory */
// Check that the input "myhistory" is correctly spelled in the main
int myhistory(argvv){
    // If executed without arguments, show standard output error a list of the last 20 commands <N> <command>

    // If exectued with an argument number between 0 and 19, print and run the command corresponding to the number

    // If number doesn't exist or out of range, print "ERROR: Command not found"
}

/* mycalc */
int mycalc(char *argvv[8]){
    char messg[90];
    if (argvv[1] && argvv[2] && argvv[3]) {
        if (strcmp(argvv[2], "add") == 0) {

            int arg1,arg2,ans;
            arg1 = atoi(argvv[1]);
            arg2 = atoi(argvv[3]);
            ans = arg1 + arg2;

            int accum;
            char *acc = getenv("Acc");
            if (acc == NULL) {
                accum = 0;
            }
            else {
                accum = atoi(acc);
            }
            accum = accum + ans;
            char buf[20];
            sprintf(buf,"%d",accum);
            int accc = setenv("Acc",buf,1);
            if (accc == -1) {
                snprintf(messg,90,"[ERROR]Error setting an environment variable\n");
            }
            snprintf(messg,90,"[OK] %d + %d = %d; Acc %s\n", arg1, arg2, ans, getenv("Acc"));

        }
        else if (strcmp(argvv[2], "mul") == 0) {
            int arg1,arg2,ans;
            arg1 = atoi(argvv[1]);
            arg2 = atoi(argvv[3]);
            ans = arg1 * arg2;
            snprintf(messg,90,"[OK] %d * %d = %d\n",arg1,arg2,ans);
        }
        else if (strcmp(argvv[2], "div") == 0) {
            int arg1,arg2;
            arg1 = atoi(argvv[1]);
            arg2 = atoi(argvv[3]);
            if (arg2 == 0){
                snprintf(messg,90,"[ERROR] Denominator can't be 0\n");
            }
            else {
            snprintf(messg,90,"[OK] %d / %d = %d; Remainder %d\n",arg1,arg2,arg1/arg2,arg1%arg2);
            }
        }
        else {char messg[90];
            snprintf(messg,90,"[ERROR] The structure of the command is mycalc <operand 1> <add/mul/div> <operand 2>\n");
        }
    }
    else{
        char messg[90];
        snprintf(messg,90,"[ERROR] The structure of the command is mycalc <operand 1> <add/mul/div> <operand 2>\n");

    }
    if (messg[1] == 'O')
    {write(STDERR_FILENO,messg,strlen(messg));
    }
    else{
        write(STDOUT_FILENO,messg,strlen(messg));
    }
return 0;
}


struct command
{
  // Store the number of commands in argvv
  int num_commands;
  // Store the number of arguments of each command
  int *args;
  // Store the commands
  char ***argvv;
  // Store the I/O redirection
  char filev[3][64];
  // Store if the command is executed in background or foreground
  int in_background;
};

int history_size = 20;
struct command * history;
int head = 0;
int tail = 0;
int n_elem = 0;

void free_command(struct command *cmd)
{
    if((*cmd).argvv != NULL)
    {
        char **argv;
        for (; (*cmd).argvv && *(*cmd).argvv; (*cmd).argvv++)
        {
            for (argv = *(*cmd).argvv; argv && *argv; argv++)
            {
                if(*argv){
                    free(*argv);
                    *argv = NULL;
                }
            }
        }
    }
    free((*cmd).args);
}

void store_command(char ***argvv, char filev[3][64], int in_background, struct command* cmd)
{
    int num_commands = 0;
    while(argvv[num_commands] != NULL){
        num_commands++;
    }

    for(int f=0;f < 3; f++)
    {
        if(strcmp(filev[f], "0") != 0)
        {
            strcpy((*cmd).filev[f], filev[f]);
        }
        else{
            strcpy((*cmd).filev[f], "0");
        }
    }

    (*cmd).in_background = in_background;
    (*cmd).num_commands = num_commands-1;
    (*cmd).argvv = (char ***) calloc((num_commands) ,sizeof(char **));
    (*cmd).args = (int*) calloc(num_commands , sizeof(int));

    for( int i = 0; i < num_commands; i++)
    {
        int args= 0;
        while( argvv[i][args] != NULL ){
            args++;
        }
        (*cmd).args[i] = args;
        (*cmd).argvv[i] = (char **) calloc((args+1) ,sizeof(char *));
        int j;
        for (j=0; j<args; j++)
        {
            (*cmd).argvv[i][j] = (char *)calloc(strlen(argvv[i][j]),sizeof(char));
            strcpy((*cmd).argvv[i][j], argvv[i][j] );
        }
    }
}


/**
 * Get the command with its parameters for execvp
 * Execute this instruction before run an execvp to obtain the complete command
 * @param argvv
 * @param num_command
 * @return
 */
void getCompleteCommand(char*** argvv, int num_command) {
	//reset first
	for(int j = 0; j < 8; j++)
		argv_execvp[j] = NULL;

	int i = 0;
	for ( i = 0; argvv[num_command][i] != NULL; i++)
		argv_execvp[i] = argvv[num_command][i];
}


/**
 * Main shell  Loop
 */
int main(int argc, char* argv[])
{
	/**** Do not delete this code.****/
	int end = 0; 
	int executed_cmd_lines = -1;
	char *cmd_line = NULL; // command line is default to no command
	char *cmd_lines[10]; //

	if (!isatty(STDIN_FILENO)) {
		cmd_line = (char*)malloc(100);
		while (scanf(" %[^\n]", cmd_line) != EOF){ // while not CTRL-C (EOF)
			if(strlen(cmd_line) <= 0) // if no command is entered
                return 0;
			cmd_lines[end] = (char*)malloc(strlen(cmd_line)+1); //
			strcpy(cmd_lines[end], cmd_line);
			end++;
			fflush(stdin);
			fflush(stdout);
		}
	}

	/*********************************/

	char ***argvv = NULL;
	int num_commands;

	history = (struct command*) malloc(history_size *sizeof(struct command));
	int run_history = 0;

	while (1) 
	{
		int status = 0;
		int command_counter = 0;
		int in_background = 0;
		signal(SIGINT, siginthandler);

		if (run_history)
    {
        run_history=0;
    }
    else{
        // Prompt 
        write(STDERR_FILENO, "MSH>>", strlen("MSH>>"));

        // Get command
        //********** DO NOT MODIFY THIS PART. IT DISTINGUISH BETWEEN NORMAL/CORRECTION MODE***************
        executed_cmd_lines++;
        if( end != 0 && executed_cmd_lines < end) {
            command_counter = read_command_correction(&argvv, filev, &in_background, cmd_lines[executed_cmd_lines]);
        }
        else if( end != 0 && executed_cmd_lines == end)
            return 0;
        else
            command_counter = read_command(&argvv, filev, &in_background); //NORMAL MODE
    }
		//************************************************************************************************


		/************************ STUDENTS CODE ********************************/
	   if (command_counter > 0) {
			if (command_counter > MAX_COMMANDS){
				printf("Error: Maximum number of commands is %d \n", MAX_COMMANDS);
			}

            for (int i = 0; i < command_counter; i++) {
               getCompleteCommand(argvv, i);
            }

           if (strcmp(argv_execvp[0], "mycalc") == 0) {
               mycalc(argv_execvp);
           }

		}
	}
	
	return 0;
}

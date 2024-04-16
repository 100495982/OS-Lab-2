//P2-SSOO-23/24

//  MSH main file
// Write your msh source code here

//#include "parser.h"
#include <stddef.h>   		 /* NULL */
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

void executeCommands(int command_counter, char ***argvv, char filev[3][64],int in_background) {
    pid_t pid;
    int status=0;
    int fd,fd2,fd3;
    int fdp[command_counter-1][2];
    for (int i = 0; i < command_counter-1; i++) {
        if (pipe(fdp[i])== -1) {
            perror("Pipe cannot be created");
        }
    }
    for (int i = 0; i<command_counter; i++)
    {
        pid=fork();
        if (pid == -1){
            perror("Error creating child");
            return;
        }
        else if (pid==0){
            getCompleteCommand(argvv, i);
            if (i==0)
            {
                if (strcmp(filev[0],"0") != 0)
                {
                    fd = open(filev[0],O_RDONLY);
                    if (fd == -1)
                    {
                        perror("Error opening file");
                        return;
                    }
                    dup2(fd,STDIN_FILENO);
                    close(fd);
                }
                if (command_counter == 1)
                {
                    if (strcmp(filev[1],"0") != 0)
                    {
                        fd2 = open(filev[1],O_WRONLY | O_CREAT | O_TRUNC, 0664);
                        if (fd2 == -1)
                        {
                            perror("Error opening file");
                            return;
                        }
                        dup2(fd2,STDOUT_FILENO);
                        close(fd2);
                    }
                    if (strcmp(filev[2],"0") != 0)
                    {
                        fd3 = open(filev[2],O_WRONLY | O_CREAT | O_TRUNC, 0664);
                        if (fd3 == -1)
                        {
                            perror("Error opening file");
                            return;
                        }
                        dup2(fd3,STDOUT_FILENO);
                        close(fd3);
                    }
                }
                else{
                    dup2(fdp[0][1],STDOUT_FILENO);
                }
            }
            else if (i<command_counter - 1){
                dup2(fdp[i-1][0],STDIN_FILENO);
                dup2(fdp[i][1],STDOUT_FILENO);
            }
            else{
                dup2(fdp[i-1][0],STDIN_FILENO);
                if (strcmp(filev[1],"0") != 0)
                {
                    fd2 = open(filev[1],O_WRONLY | O_CREAT | O_TRUNC, 0664);
                    if (fd2 == -1)
                    {
                        perror("Error opening file");
                        return;
                    }
                    dup2(fd2,STDOUT_FILENO);
                    close(fd2);
                }
                if (strcmp(filev[2],"0") != 0)
                {
                    fd3 = open(filev[2],O_WRONLY | O_CREAT | O_TRUNC, 0664);
                    if (fd3 == -1)
                    {
                        perror("Error opening file");
                        return;
                    }
                    dup2(fd3,STDOUT_FILENO);
                    close(fd3);
                }
            }
            for (int i = 0; i < command_counter-1; i++) {
                close(fdp[i][0]);
                close(fdp[i][1]);
            }
            execvp(argv_execvp[0],argv_execvp);
        }
        else{
            if (i==0 && command_counter > 1)
            {
                close(fdp[0][1]);
            }
            if (i>0)
            {
                close(fdp[i-1][0]);
                close(fdp[i-1][1]);
            }
            if (in_background == 0)
            {
                waitpid(pid,&status,0);
            }
            else{
                if (i == command_counter - 1)
                {
                    printf("PID: %d\n",pid);
                }
            }
        }
    }
}

/* myhistory */
// Check that the input "myhistory" is correctly spelled in the main
int myhistory(char *argvv_1[8], struct command *history)
{
    if (argvv_1[1] == NULL)
    {
        // Print the last 20 commands
        char buffer[1024]; // Buffer to store the output
        int buffer_length = 0; // Length of the current buffer

        for (int i = 0; i < 20; i++)
        {
            if (history[i].argvv != NULL)
            {
                buffer_length += sprintf(buffer + buffer_length, "%d ",i);
                for (int q = 0; q < history[i].num_commands; q++) {
                    getCompleteCommand(history[i].argvv, q);
                }
                int array_length = sizeof(argv_execvp)/sizeof(argv_execvp[0]);
                int b = 0;

                while (b<array_length && argv_execvp[b] != NULL) {
                    buffer_length += sprintf(buffer + buffer_length, "%s ", argv_execvp[b]);
                    b++;
                }

                buffer_length += sprintf(buffer + buffer_length, "\n");
            }
        }

// Print the buffer to the standard error output
        write(STDERR_FILENO, buffer, buffer_length);

    }


    else if (argvv_1[1] != NULL)
    {
        int commandIndex = atoi(argvv_1[1]);
        if (commandIndex >= 0 && commandIndex < 20) {
            // Execute the command at index commandIndex in the history


            executeCommands(history[commandIndex].command_counter, history[commandIndex].argvv, history[commandIndex].filev,history[commandIndex].in_background);
        }
    } else {
        write(STDOUT_FILENO, "ERROR: Command not found\n", 24);
    }
}
return 0;
}

// If executed without arguments, show standard output error a list of the last 20 commands <N> <command>

// If exectued with an argument number between 0 and 19, print and run the command corresponding to the number

// If number doesn't exist or out of range, print "ERROR: Command not found"


/* mycalc */
int mycalc(char *argvv[8]){
    char messg[90];
    int buffer_length=0;
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
                buffer_length = sprintf(messg,"[ERROR]Error setting an environment variable\n");
            }
            buffer_length =sprintf(messg,"[OK] %d + %d = %d; Acc %s\n", arg1, arg2, ans, getenv("Acc"));

        }
        else if (strcmp(argvv[2], "mul") == 0) {
            int arg1,arg2,ans;
            arg1 = atoi(argvv[1]);
            arg2 = atoi(argvv[3]);
            ans = arg1 * arg2;
            buffer_length = sprintf(messg,"[OK] %d * %d = %d\n",arg1,arg2,ans);
        }
        else if (strcmp(argvv[2], "div") == 0) {
            int arg1,arg2;
            arg1 = atoi(argvv[1]);
            arg2 = atoi(argvv[3]);
            if (arg2 == 0){
                buffer_length = sprintf(messg,"[ERROR] Denominator can't be 0\n");
            }
            else {
                buffer_length = sprintf(messg,"[OK] %d / %d = %d; Remainder %d\n",arg1,arg2,arg1/arg2,arg1%arg2);
            }
        }
        else {
            buffer_length = sprintf(messg,"[ERROR] The structure of the command is mycalc <operand_1> <add/mul/div> <operand_2>\n");
        }
    }
    else{
        buffer_length= sprintf(messg,"[ERROR] The structure of the command is mycalc <operand_1> <add/mul/div> <operand_2>\n");

    }
    if (messg[1] == 'O')
    {write(STDERR_FILENO,messg,buffer_length);
    }
    else{
        write(STDOUT_FILENO,messg,buffer_length);
    }
    return 0;
}




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
            if (strcmp(argv_execvp[0],"myhistory") == 0){
                run_history = 1;
            }

            if (strcmp(argv_execvp[0], "mycalc") == 0) {
                mycalc(argv_execvp);
            }
            else if (strcmp(argv_execvp[0],"myhistory") == 0){
                run_history = 1;
                myhistory(argv_execvp,history);
            }
            else{
                executeCommands(command_counter, argvv, filev,in_background);
            }

            if (run_history == 0){
                if (n_elem >= history_size){
                    free_command(&history[tail]);
                    store_command(argvv,filev,in_background,&history[tail]);
                }
                else{
                    store_command(argvv,filev,in_background,&history[tail]);
                }
                tail = (tail+1)%history_size;
                n_elem++;
            }
        }
    }


    return 0;
}
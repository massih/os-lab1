    /*
 * Main source code file for lsh shell program
 *
 * You are free to add functions to this file.
 * If you want to add functions in a separate file 
 * you will need to modify Makefile to compile
 * your additional functions.
 *
 * Add appropriate comments in your code to make it
 * easier for us while grading your assignment.
 *
 * Submit the entire lab1 folder as a tar archive (.tgz).
 * Command to create submission archive: 
      $> tar cvf lab1.tgz lab1/
 *
 * All the best 
 */


#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parse.h"
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>

/*
 * Function declarations
 */

void PrintCommand(int, Command *);
void PrintPgm(Pgm *);
void stripwhite(char *);

//added function definitions
void command_handler(Command *);
void execute_command(Pgm *, int, char *, char *);
void sig_handler(int);

/* When non-zero, this global means the user is done using this program. */
int done = 0;

// Added Global variables
pid_t mainParent;
int background;

/*
 * Name: main
 *
 * Description: Gets the ball rolling...
 *
 */
int main(void){
    
    Command cmd;
    int n;
    mainParent = getpid();
    char buff[1024];
    
    // signal(SIGINT, sig_handler);
    signal(SIGCHLD,sig_handler);

    while (!done) {
        char* cwd;
        
        // Get the current working directoru
        cwd = getcwd( buff, sizeof(buff));
        char *line_dir;
        char *line;
        char *token;
        char *arr[5];
        int m = 0;
        
        line = readline(strcat(cwd,": lsh$ "));
        
        if (!line) {
            /* Encountered EOF at top level */
            done = 1;
        }
        else {
            /*
                * Remove leading and trailing whitespace from the line
                * Then, if there is anything left, add it to the history list
                * and execute it.
                */
            stripwhite(line);

            if(*line) {
                add_history(line);
                // Duplicate line to process it for built_in commands
                line_dir = strdup(line);
                // Tokenizing the line_dir
                arr[m]=strtok(line_dir, " ");
                // Loop over the tokenized strings
                while(arr[m] && m<4){
                    arr[++m]=strtok(NULL, " ");
                }
                
                
                n = parse(line, &cmd);
                if (n != -1){
                    // Check if change directory ("cd") is requested
                    if(strcmp(arr[0],"cd") == 0){
                        // Change the directory
                        chdir(arr[1]);
                    }
                    // Check if exit command is requested
                    else if(strcmp(arr[0],"exit") == 0){
                        exit(0);
                    }else
                        /* execute it */
                        command_handler(&cmd);
                }
                //         PrintCommand(n, &cmd);
            }
        }
    
        if(line) {
            free(line);
        }
    }
    return 0;
}

/*
 * Name: PrintCommand
 *
 * Description: Prints a Command structure as returned by parse on stdout.
 *
 */
void
PrintCommand (int n, Command *cmd)
{
  printf("Parse returned %d:\n", n);
  printf("   stdin : %s\n", cmd->rstdin  ? cmd->rstdin  : "<none>" );
  printf("   stdout: %s\n", cmd->rstdout ? cmd->rstdout : "<none>" );
  printf("   bg    : %s\n", cmd->bakground ? "yes" : "no");
  PrintPgm(cmd->pgm);
}

/*
 * Name: PrintPgm
 *
 * Description: Prints a list of Pgm:s
 *
 */
void
PrintPgm (Pgm *p)
{
  if (p == NULL) {
    return;
  }
  else {
    char **pl = p->pgmlist;

    /* The list is in reversed order so print
     * it reversed to get right
     */
    PrintPgm(p->next);
    printf("    [");
    while (*pl) {
    	// execute_command(*pl++);
      printf("%s ", *pl++);
    }
    printf("]\n");
  }
}

/*
 * Name: stripwhite
 *
 * Description: Strip whitespace from the start and end of STRING.
 */
void
stripwhite (char *string)
{
  register int i = 0;

  while (whitespace( string[i] )) {
    i++;
  }
  
  if (i) {
    strcpy (string, string + i);
  }

  i = strlen( string ) - 1;
  while (i> 0 && whitespace (string[i])) {
    i--;
  }

  string [++i] = '\0';
}
// ===========================Added Function(s)==========================

/*
 * Name: command_handler
 *
 * Description: Check all properties of command such as background, etc...
 * then call execute_command function to execute it.
 *
 */
void command_handler(Command *cmds){
	background = cmds->bakground;
	Pgm *lastCommand = cmds->pgm;
    char **cmd = cmds->pgm->pgmlist;
    char *rstdout = cmds->rstdout;
    char *rstdin = cmds->rstdin;
    pid_t parent;
  

    parent = fork();
    if (parent < 0){
        perror("unsuccesful forking");
    }
    else if(parent == 0){

        FILE* infile;
        FILE* outfile;
        if(rstdout){
            outfile = freopen (rstdout, "w", stdout);
            if(outfile == NULL){
                perror("could not create file");
                _Exit(EXIT_FAILURE);
            }
        }
        if(rstdin){
            infile = freopen (rstdin, "r", stdin);
            if(infile == NULL){
                perror("could not create file");
                _Exit(EXIT_FAILURE);
            }
        }
        if(background == 1){
        	signal(SIGINT, SIG_IGN);
        }else{
        	signal(SIGINT, sig_handler);
        }
        execute_command(lastCommand,background,rstdout, rstdin);
        
        if (infile != NULL){
            fclose(infile);
        }
        if (outfile != NULL){
            fclose(outfile);
        }
    }
    else{
    	// if the command is background then ignore SIGINT(Ctrl+C)
        if(background == 1){
	    	signal(SIGINT, SIG_IGN);
	    }
	    // if the command is not background then handle SIGINT(Ctrl+C)
	    else{
	    	signal(SIGINT, sig_handler);
	    	waitpid(parent,NULL,0);
	    }
    }
}

/*
 * Name: execute_command
 *
 * Description: Initiates pipes and then execute muliple commands recursively.
 *
 */
void execute_command(Pgm *command, int background,char *rstdout,char *rstdin){
    
    char **cmd = command->pgmlist;
    int pfd[2];
    pid_t child_pid;
    pipe(pfd);
    
    //check if the command is a single commad and execute it
    if(command->next == NULL){
    	if(execvp(cmd[0], cmd) == -1){
                printf("-lsh: %s : R U kidding?? \n", *cmd);
                _Exit(EXIT_FAILURE);
        }
    }

    // Execute multiple commands by recursion 
    else{
	    child_pid = fork();
	    if (child_pid < 0){
	        perror("unsuccesful forking");
	    }
	    // Child Process
	    else if (child_pid == 0) {
	        close(pfd[0]);
	        dup2(pfd[1], STDOUT_FILENO);
	        close(pfd[1]);
	        
	        // Check if recursion is required
	        if (command->next != NULL) {
	            execute_command(command->next,background,rstdout, rstdin);
	        }
	        // Execute a command and error if it is invalid command
	        else{
	            if(execvp(cmd[0], cmd) == -1){
	                printf("-lsh: %s : R U kidding?? \n", *cmd);
	                _Exit(EXIT_FAILURE);
	            }
	        }
	    }
	    // Parent Process
	    else{
	        if(background == 0) {
	            close(pfd[1]);
	            dup2(pfd[0], STDIN_FILENO);
	            close(pfd[0]);
	            waitpid(child_pid,NULL,0);
	            
	        }
	        // Execute a command and error if it is invalid command
	        if(execvp(cmd[0], cmd) == -1){
	            printf("-lsh: %s : R U kidding?? \n", *cmd);
	            _Exit(EXIT_FAILURE);
	        }

	    }
    	
    }
}

/*
 * Name: sig_handler
 *
 * Description: Handle the recived signals such as SIGINT(Ctrl+c) and SIGCHLD(zombie processes)
 */
void sig_handler(int recievedSignal){
    switch(recievedSignal){
        case SIGINT:
            // Kill all the processes except than the main process
            if(getpid() != mainParent){
            	kill(getpid(), SIGKILL);
            }
            break;
        case SIGCHLD:
            // Reap zombie processes
            while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
            break;
    }
}
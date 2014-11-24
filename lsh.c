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
void commandIO(Command *);
void execute_command(Pgm *, int);
void handle_sigchld(int);

/* When non-zero, this global means the user is done using this program. */
int done = 0;
int pfd[2];
struct sigaction sa;


/*
 * Name: main
 *
 * Description: Gets the ball rolling...
 *
 */
int main(void)
{
  Command cmd;
  int n;

  while (!done) {

    char *line;
    line = readline("Dude => ");

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
        /* execute it */
        n = parse(line, &cmd);
        if (n != -1)
        {
        	commandIO(&cmd);
            // execute_command(&cmd);
        }
        // PrintCommand(n, &cmd);
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

void handle_sigchld(int sig) {
    while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
    }

void commandIO(Command *cmds){
	int background = cmds->bakground;
	Pgm *lastCommand = cmds->pgm;
	pipe(pfd);
	execute_command(lastCommand,background);
}

void execute_command(Pgm *command, int background){
	pid_t child_pid;
    int status;
    
	
	char **cmd = command->pgmlist;

	if((child_pid = fork()) == -1){
		perror("unseccessful fork");

	}else if(child_pid  == 0){
		// printf("CHILD command list is %s \n",*cmd);
		if(command->next != NULL){
			execute_command(command->next,background);
			// close(pfd[0]);
			// dup2(pfd[1],STDOUT_FILENO);
			// close(pfd[1]);
			if(execvp(*cmd, cmd) == -1){
				// printf("-lsh: %s : R U kidding?? \n", *cmd);
				_Exit(EXIT_FAILURE);
			}
		}else{
			// execute_command(command->next,background);
			close(pfd[0]);
			dup2(pfd[1],STDOUT_FILENO);
			close(pfd[1]);
			if(execvp(*cmd, cmd) == -1){
				// printf("-lsh: %s : R U kidding?? \n", *cmd);
				_Exit(EXIT_FAILURE);
			}
		}

	}
    else if (child_pid > 0){
    	// printf("PARENT command list is %s \n",*cmd);
    	if(command->next != NULL){
  			waitpid(child_pid,&status,0); // wait for completion
            close(pfd[1]);
			dup2(pfd[0],STDIN_FILENO);
			close(pfd[0]);
			if(execvp(*cmd, cmd) == -1){
				// printf("-lsh: %s : R U kidding?? \n", *cmd);
				_Exit(EXIT_FAILURE);
			}
    	}
    	else{
 			waitpid(child_pid,&status,0); // wait for completion
   //          close(pfd[1]);
			// dup2(pfd[0],STDIN_FILENO);
			// close(pfd[0]);
		}
    	if(background == 0){
   			waitpid(child_pid,&status,0); // wait for completion
            // char buff[1000];
            // memset(buff,0,sizeof(buff));
			// close(0);
   //          read(pfd[0],buff,1000);
    		// printf("RECIEVED \n%s \n", buff);

            sa.sa_handler = &handle_sigchld;
            sigemptyset(&sa.sa_mask);
            sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
            if (sigaction(SIGCHLD, &sa, 0) == -1) {
                perror("SIGCHLD PROBLEM");
                exit(1);
            }
			
    	}else{
    	//TODO ******	
    	}
    }
}

// Pgm commandFinder(Pgm *cmds){
// 	Pgm *temp;
// 	while(cmds->next != NULL){
// 		temp = cmds->next;
// 		if(temp->next == NULL){

// 			return
// 		}
// 	}

// }
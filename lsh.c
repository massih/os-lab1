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
void execute_command(Pgm *, int, char *, char *);
void handle_sigchld(int);
int redirectIO(char *,char *);
/* When non-zero, this global means the user is done using this program. */
int done = 0;
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
      char* cwd;
      char buff[1024];
      
      cwd = getcwd( buff, sizeof(buff));
      
      char *line;
      char *token;
      printf("%s",cwd);
      line = readline(": Dude => ");
      char *arr[5];
      int m = 0;
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
          arr[m]=strtok(line, " ");
          while(arr[m] && m<4){
              
              arr[++m]=strtok(NULL, " ");
              
          }

        if(strcmp(arr[0],"cd") == 0){
            chdir(arr[1]);
        }else if(strcmp(arr[0],"exit") == 0){
            exit(0);
        }
        /* execute it */
        n = parse(line, &cmd);
//        commandIO(&cmd);
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
    char **cmd = cmds->pgm->pgmlist;
    char *rstdout = cmds->rstdout;
    char *rstdin = cmds->rstdin;
    pid_t parent;

    parent = fork();
    if (parent < 0){
        perror("unsuccesful forking");
    }
    else if(parent == 0){
        if (cmds->pgm->next != NULL) {
            execute_command(lastCommand,background,rstdout, rstdin);
        }else{
		    if(rstdout){
	        	FILE *fp;
	        	fp = freopen (rstdout, "w", stdout);
	        	if(fp == NULL){
	        		perror("could not create file");
	        		_Exit(EXIT_FAILURE);
	        	}
	    	}
	    	if(rstdin){
	        	FILE *fp;
	        	fp = freopen (rstdin, "r", stdin);
	        	if(fp == NULL){
	        		perror("could not create file");
	        		_Exit(EXIT_FAILURE);
	        	}
	        	// fclose(fp);
	    	}	
            if(execvp(cmd[0], cmd) == -1){
                printf("-lsh: %s : R U kidding?? \n", *cmd);
                _Exit(EXIT_FAILURE);
            }
        }
    }
    else{
        if (background == 0){
            wait(NULL);
        }
    }
    
}

void execute_command(Pgm *command, int background,char *rstdout,char *rstdin){
    char **cmd = command->pgmlist;
    int pfd[2];
    int status;
    pid_t child_pid;
    pipe(pfd);
    child_pid = fork();
    
    if (child_pid < 0){
        perror("unsuccesful forking");
    }
    // Child Process
    else if (child_pid == 0) {
        close(pfd[0]);
        dup2(pfd[1], STDOUT_FILENO);
        close(pfd[1]);
        if (command->next != NULL) {
            execute_command(command->next,background,rstdout, rstdin);
        }
        else{
        	if(rstdin){
	        	FILE *fp;
	        	fp = freopen (rstdin, "r", stdin);
	        	if(fp == NULL){
	        		perror("There is no such a file or directory");
	        		_Exit(EXIT_FAILURE);
	        	}
		    }
            if(execvp(cmd[0], cmd) == -1){
                printf("-lsh: %s : R U kidding?? \n", *cmd);
                _Exit(EXIT_FAILURE);
            }
        }
    }
    else{  // Parent Process
        if (background == 0) {
            close(pfd[1]);
            dup2(pfd[0], STDIN_FILENO);
            close(pfd[0]);
        	if(rstdout){
	        	FILE *fp;
	        	fp = freopen (rstdout, "w", stdout);
	        	if(fp == NULL){
	        		perror("Could not create file");
	        		_Exit(EXIT_FAILURE);
	        	}
	        	// fclose(fp);
	    	}


            if(execvp(cmd[0], cmd) == -1){
                printf("-lsh: %s : R U kidding?? \n", *cmd);
                _Exit(EXIT_FAILURE);
            }
//            waitpid(child_pid,NULL,0);
        }else{

        }
    }
}

int redirectIO(char *stdout,char *stdin){
	return 0;
}
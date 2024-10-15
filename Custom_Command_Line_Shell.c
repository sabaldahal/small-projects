#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>


/**************************************************************/
/* Text Processing / Parsing Routines                         */
/**************************************************************/

// Function: void parse(char *line, char **argv)
// Purpose : This function takes in a null terminated string pointed to by 
//           <line>.  It also takes in an array of pointers to char <argv>.
//           When the function returns, the string pointed to by the 
//           pointer <line> has ALL of its whitespace characters (space, 
//           tab, and newline) turned into null characters ('\0').  The
//           array of pointers to chars will be modified so that the zeroth
//           slot will point to the first non-null character in the string
//           pointed to by <line>, the oneth slot will point to the second 
//           non-null character in the string pointed to by <line>, and so
//           on. In other words, each subsequent pointer in argv will point
//           to each subsequent "token" (characters separated by white space)
//           IN the block of memory stored at the pointer <line>.  Since all
//           the white space is replaced by '\0', every one of these "tokens"
//           pointed to by subsequent entires of argv will be a valid string
//           The "last" entry in the argv array will be set to NULL.  This 
//           will mark the end of the tokens in the string.
//           
void  parse(char *line, char **argv)
{
     // We will assume that the input string is NULL terminated.  If it
     // is not, this code WILL break.  The rewriting of whitespace characters
     // and the updating of pointers in argv are interleaved.  Basically
     // we do a while loop that will go until we run out of characters in
     // the string (the outer while loop that goes until '\0').  Inside
     // that loop, we interleave between rewriting white space (space, tab,
     // and newline) with nulls ('\0') AND just skipping over non-whitespace.
     // Note that whenever we encounter a non-whitespace character, we record
     // that address in the array of address at argv and increment it.  When
     // we run out of tokens in the string, we make the last entry in the array
     // at argv NULL.  This marks the end of pointers to tokens.
    
     while (*line != '\0') // outer loop.  keep going until the whole string is read
        { // keep moving forward the pointer into the input string until
          // we encounter a non-whitespace character.  While we're at it,
          // turn all those whitespace characters we're seeing into null chars.

          while (*line == ' ' || *line == '\t' || *line == '\n' || *line == '\r')
           { *line = '\0';     
             line++;
           }

          // We are now looking at a non-whitespace character,
          // or, the beginning of a token.  So, let's record the address of this
          // beginning of token to the address I'm pointing at now. (Put it in *argv)
          // then we'll increment argv so that the next time I store an address, it 
          // will be in the next slot of the array of integers.

          *argv++ = line;          /* save the argument position     */

          // now let's just keep incrementing the input line pointer until
          // I'm looking at whitespace again.  This "eats" the token I just found
          // and sets me up to look for the next.

          while (*line != '\0' && *line != ' ' && 
                 *line != '\t' && *line != '\n' && *line !='\r') 
               line++;             /* skip the argument until ...    */
        }

     // I'm out of tokens.
     // slot of the array at argv I'm pointing at, put a NULL
     // there so we can mark the end of entries in the table.

     *argv = NULL;                 /* mark the end of argument list  */
}


/**************************************************************/
/* Signal Handling Routines                                   */
/**************************************************************/
void ignore_signal(int signum)
{
	//don't need any codes here
	//basically we are ignoring the registered signals
	return;
}

/**************************************************************/
/* Shell Command Processing Routines                          */
/**************************************************************/

char *doesFileExist(char *command){
	//first check if the program exists
	char *path = getenv("PATH");
	if(path == NULL){
		return NULL;
	}
	
	char *path_copy = malloc(strlen(path) + 1);
	strcpy(path_copy, path);
	char *currentDir;
	
	while((currentDir = strsep(&path_copy, ":")) != NULL){
		char *full_path = malloc(strlen(currentDir) + strlen(command) + 2);
		sprintf(full_path, "%s/%s", currentDir, command);
		
		if(access(full_path, X_OK) == 0){			
			return full_path;
		}
	}
	return NULL;
}

void execute(char **argv, char *raw_line_input)
{
     char* full_path = doesFileExist(*argv);
     if(full_path == NULL){
     
     	printf("Command %s not found\n", *argv);
     	return;
     }
     

     pid_t  pid,wpid;
     
     if ((pid = fork()) < 0) 
        { /* fork a child process           */
          printf("*** ERROR: forking child process failed\n");
          exit(1);
        } else 
     
     if (pid == 0) 
       { /* for the child process:         */
         /* The child is going to say what it should have done, then exit */
         execvp(full_path, argv);
         printf("The command line <%s> should have been executed here\n", raw_line_input);
         exit (0); 
       } else 
       
       { /* for the parent:      */
         // If we get here at all, this is the parent shell.  Maybe this would be a good
         // place to do a "wait" of some kind.  Be careful, though, there's different
         // kinds of waits...
         int status;
         
         waitpid(pid, &status, WUNTRACED);
         if(WIFEXITED(status)){
         	printf(">>>>>>> PID %d terminated normally\n", pid);
         }else if(WIFSTOPPED(status)){
         	printf(">>>>>>> PID %d is stopped\n", pid);
         }
         return;
       }
       
     return;
}

void printenv(char **envp){
	while(*envp != NULL){
		printf("%s\n", *envp++);
	}
	
}


void pwd(){
	char s[100];
	printf("%s\n", getcwd(s,100));
}

void cd(char* newdir){
	char s[100];
	chdir(newdir);
	setenv("PWD", getcwd(s, 100), 1);
}


int main(int argc, char **argv, char **envp)
{    
     pid_t            wpid;
     int              status;
     int              job;

     char  line[1024];   // This is the string buffer that will hold
                         // the string typed in by the user.  This 
                         // string will be parsed.  The shell will do
                         // what it needs to do based on the tokens it
                         // finds.  Note that a user may NOT type in 
                         // an input line of greater than 1024 characters
                         // because that's the size of the array.
                         
     char raw_line[1024];  // This will hold an unmodified copy of the user's
                           // input at the command line.
                                          
     char  *line_argv[64];    // This is a pointer to an array of 64 pointers to
                              // char, or, an array of pointers to strings. 
                              // after parsing, this array will hold pointers
                              // to memory INSIDE of the string pointed to by 
                              // the pointer line.  argv[0] will be the string
                              // version of the first token inside of line... 
                              // argv[1] will be the second... and so on... 

     char shell_prompt[15]; // This string will hold the shell prompt string

     int index = 0;
     
     int job_number_int;
     
     // set the default prompt
     strcpy(shell_prompt, "SillyShell");

     // Set signal handler
     signal(SIGINT, ignore_signal);
     signal(SIGTSTP, ignore_signal);
         
     while (1) 
       {  // once per processing loop we could check if there's
          // status signals coming from children, and then deal with them
          
          wpid = waitpid(-1, &status, WNOHANG);
          if(wpid > 0){
          	         
		printf(">>>>>>> PID %d died\n", wpid);
		
          }

       
          printf("%s> ",shell_prompt);  // display the shell prompt

          if(fgets(line, 1024, stdin) == NULL)
             { printf("\n");
               exit(0);
             };  
             
          line[strlen(line)-1]='\0'; 
          strncpy(raw_line, line, 1024);


          if ((*line != '\0') && (*line > 31)) // If something was actually typed, then do something...
            { 
            
              // First, get all the addresses of all of the tokens inside the input line
              
              parse(line, line_argv);     //   parse the line to break it into token references

              // Check the first token to see if there are any built in commands
              // we want to handle directly and will do it with an "if/then/else" ladder.
              // if we hit the end of the ladder, we assume the command line was requesting
              // an external program be run as a child process and do that....  
     
              
              if (strcmp(line_argv[0], "cd")	      == 0) cd(line_argv[1]); else
              
              if (strcmp(line_argv[0], "pwd")		== 0) pwd(); else
              
              if (strcmp(line_argv[0], "exit")         == 0) exit(0); else
              
              if (strcmp(line_argv[0], "done")         == 0) exit(0); else
              
              if (strcmp(line_argv[0], "quit")         == 0) exit(0); else
                            
              if (strcmp(line_argv[0], "printenv")  == 0) { printenv(envp);

                                                          }       else
              
              if (strcmp(line_argv[0], "newprompt") == 0) { if (line_argv[1] != NULL)
                                                                strncpy(shell_prompt, line_argv[1], 15); 
                                                            else
                                                                strncpy(shell_prompt, "SillyShell", 15);
                                                          } else 
                                                          
                                                                                                      
              
               execute(line_argv, raw_line);
               
            }
     }
}

                



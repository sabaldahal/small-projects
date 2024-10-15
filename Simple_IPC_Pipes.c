//Sabal Dahal

#include <stdlib.h>
#include <stdio.h> 
#include <sys/wait.h>
#include <unistd.h> 
#define MSGSIZE 12

int main(int argc, char **argv) 
{   
    FILE *fp;    
    pid_t child;
    char inbuf[MSGSIZE];
    int p[2], i;
    int status;
       
    //if the filename was provided from the terminal, argc count would be more than 1, because the first argument that gets passed to argv is the name of the program itself
    if (argc > 1)
       { 
         fp = fopen(argv[1], "w"); //create a file to write data into it with the name provided
         //if the file could not be created, print the error and exit the program
         if (fp == NULL) 
           { fprintf(stderr, "ERROR - FILE %s COULD NOT BE CREATED\n", argv[1]);
             exit(1);
           } 
       }
       
    //if no file name is provided, just direct the data to stdout
    else fp = stdout;
    
    if(pipe(p) < 0) exit(1);     //create a pipe
    child = fork();     //create a child process
    
    //child is responsible to get data from the pipe and 
    //1. add the data to the file if a file was opened for writing
    //2. print the same data to the terminal
    if(child == 0){
    	close(p[1]); //close the writing end
    	//read until EOF from the pipe
        while (read(p[0], inbuf, MSGSIZE)){
            //if any file was opened, then write the data received from the pipe to this file
            if(argc > 1) fputs(inbuf, fp);          
 	    printf("%s", inbuf); //also print the data to the terminal
           }
    	fclose(fp);//close the file after the writing is complete and exit
         exit(0);
       }
    //parent has the responsibility:
    //1. to get data from stdin and send it to the pipe   
    else{
    	close(p[0]); //close the reading end
    	char c[MSGSIZE]; //create a buffer to store small chunks of data obtained from stdin
        
        while (!feof(stdin)) //while eof is not reached for stdin, keep reading data in small chunks
       { 
       	 int bytesread = fread(c, 1, 1, stdin); //read the chunks of data from stdin
         if(!feof(stdin)) write(p[1], c, MSGSIZE); //write the data to the pipe
       }  
              
    	close(p[1]);  //after entire data is read, close the writing end of the pipe
    	waitpid(child, &status, 0); //wait for the child process to terminate
    }
    return 0;       
}

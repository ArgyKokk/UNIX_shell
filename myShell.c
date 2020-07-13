/* File name myShell.c
 *
 * Unix Shell that supports basic commands in two modes
 *
 * Interactive Mode:
 * 
 * User enters each command
 * Execution:  ./myShell
 *
 * Batch Mode:
 *
 * The commands are executed by reading a file
 * Execution:  ./myShell <filename or path>
 *
 * Operating Systems AUTH
 * December 2018
 *
 * Argyrios Kokkinis
 * 8459
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#define BUFF_SIZE 512
#define MAX_CHAR 512
#define MAX_ARGS 20
#define MAX_COMMANDS 200

void commandExecution();
void commandExecutionBatchMode(char*);
void extractArguments(char*,char**);
void batchFile(char*);
int  extractCommand(char*,char*);
int  semicolonDelim(char*,char**);
int  ambersandDelim(char*,char**);
int quitCheck(char*,char*);

/* if there is no argument then interactive mode is chosen
 * if the argument is the batchfile's name then we enter in the batch mode
 */
int main(int argc,char *argv[])
{
    if(argc==1)	
     {	fprintf(stdout,"Entering interactive mode\n");
        commandExecution();
     }
     else if(argc==2)
     {
       fprintf(stdout,"Batch file mode\n");
       batchFile(argv[1]);
       fprintf(stdout,"Batch mode is completed\n");
       fprintf(stdout,"Entering interactive mode ...\n");
       commandExecution();
     }
}

/* The command exexution in interactive mode
 * The special command 'quit' is executed by the parent process
 */
void commandExecution()
{
   int forkStatus,pid,forkStatusChild,pidChild;
   
   char fullString[MAX_CHAR];      //full command (included args,spaces,delimeters etc)
   char fullStringCopy[MAX_CHAR];
   char commandNoArgs[MAX_CHAR]; 
   char* singleCommand[MAX_COMMANDS];    
   char* args[MAX_ARGS];

    for(int i=0;i<MAX_COMMANDS;i++)
	   singleCommand[i]=(char*)malloc(sizeof(char)*MAX_CHAR);

    for(int i=0;i<MAX_ARGS;i++)
           args[i]=(char*) malloc(sizeof(char)*MAX_CHAR);

 while(1)
 {
    printf("kokkinis_8459>");
    if(fgets(fullString,MAX_CHAR,stdin)!=NULL){
    
     /* check for the special command quit done
      * by the parent process 
     */    
    if(quitCheck(fullString,commandNoArgs))
    {       printf("exiting \n:");
	    exit(EXIT_SUCCESS);
    }
   
    /* check the command for any ambersand delimeter
     * if there is no ambersand we enter in the first condition
     **/
   int numAmberCommands=ambersandDelim(fullString,singleCommand);    
   if(numAmberCommands==1) 
   { 
	// check for any semicolon delimeter
	int numCommands=semicolonDelim(fullString,singleCommand);
        for(int i=0;i<numCommands;i++)
        {
	     pid=fork();

	     if(pid==0)
	    {    strcpy(fullStringCopy,singleCommand[i]);

		 // extract a single command without args,spaces,etc   
		 int empty=extractCommand(singleCommand[i],commandNoArgs);

	          if(empty)
		       exit(EXIT_SUCCESS);
                 
		  // extract the arguments of the previous command
		  extractArguments(fullStringCopy,args);
		  int exeStatus=execvp(commandNoArgs,args);
		  if(exeStatus==-1)
		  {
		       perror("execution error\n");
		       exit(EXIT_FAILURE);
	          }
	    }

           else if(pid <0)
           {
		   perror("error\n");
		   exit(EXIT_FAILURE);
           }

	   else
           {
                  pidChild=fork();
		  if(pidChild<0)
		  {
		        perror("error\n");
		        exit(EXIT_FAILURE);
		  }
		  // wait for the command execution before exiting
	          if(waitpid(pid,&forkStatus,0)!=pid && waitpid(pidChild,&forkStatusChild,0)!=pidChild)
		        exit(EXIT_SUCCESS);		  
           }       
       }
   }
   else
   {
	// the pipe is used only if the command execution fails   
	int pipefd[2];
	char buf[BUFF_SIZE];

        for(int i=0;i<numAmberCommands;i++)
        {
	   pipe(pipefd);
           pid=fork();

           if(pid==0)
           {     strcpy(fullStringCopy,singleCommand[i]);
                 int empty=extractCommand(singleCommand[i],commandNoArgs);
		 close(pipefd[0]);

                 if(empty)
                      exit(EXIT_SUCCESS);

                  extractArguments(fullStringCopy,args);
                  int exeStatus=execvp(commandNoArgs,args);
                  if(exeStatus==-1)
                  {
                       perror("execution error\n");
		       write(pipefd[1],"e\n",2);
                       exit(EXIT_FAILURE);
                  }
           }
           else if(pid <0)
           {
                   perror("error\n");
                   exit(EXIT_FAILURE);
           }
           else
           {
		   close(pipefd[1]);
	           read(pipefd[0],buf,BUFF_SIZE);	
	           if(buf[0]=='e')
		   {
			buf[0]='\0';
			break;
		   }
		
		   pidChild=fork();
                   if(pidChild<0)
                   {
                        perror("error\n");
                        exit(EXIT_FAILURE);
                   }
                   if(waitpid(pid,&forkStatus,0)!=pid && waitpid(pidChild,&forkStatusChild,0)!=pidChild)
                        exit(EXIT_SUCCESS);
           }
       }
   }
  }
}
   for(int i=0;i<MAX_ARGS;i++)
	   free(args[i]);
   for(int i=0;i<MAX_COMMANDS;i++)
           free(singleCommand[i]);
}  



/* command execution in batch mode
 * similar to the interactive 
 **/
void commandExecutionBatchMode(char*line)
{
   int forkStatus,pid,forkStatusChild,pidChild;
   
   char fullString[MAX_CHAR];
   char fullStringCopy[MAX_CHAR];
   char commandNoArgs[MAX_CHAR];
   char* singleCommand[MAX_COMMANDS];
   char* args[MAX_ARGS];

    for(int i=0;i<MAX_COMMANDS;i++)
	   singleCommand[i]=(char*)malloc(sizeof(char)*MAX_CHAR);

    for(int i=0;i<MAX_ARGS;i++)
           args[i]=(char*) malloc(sizeof(char)*MAX_CHAR);

    strcpy(fullString,line);
   
  
    if(quitCheck(fullString,commandNoArgs))
    {       printf("exiting \n:");
	    exit(EXIT_SUCCESS);
    }

   int numAmberCommands=ambersandDelim(fullString,singleCommand);    
   if(numAmberCommands==1) 
   { 
	int numCommands=semicolonDelim(fullString,singleCommand);
        for(int i=0;i<numCommands;i++)
        {
	     pid=fork();
         
	     if(pid==0)
	     {   strcpy(fullStringCopy,singleCommand[i]);
		 int empty=extractCommand(singleCommand[i],commandNoArgs);

	         if(empty)
		      exit(EXIT_SUCCESS); 
                 
		  extractArguments(fullStringCopy,args);
		  int exeStatus=execvp(commandNoArgs,args);
		  if(exeStatus==-1)
		  {
		       perror("execution error\n");
		       exit(EXIT_FAILURE);
	          }
	     }

             else if(pid <0)
             {
		  perror("error\n");
		  exit(EXIT_FAILURE);
             }

	     else
             {     
                  pidChild=fork();
		  if(pidChild<0)
		  {
		       perror("error\n");
		       exit(EXIT_FAILURE);
		  }
	          if(waitpid(pid,&forkStatus,0)!=pid && waitpid(pidChild,&forkStatusChild,0)!=pidChild)
		       exit(EXIT_SUCCESS);		                     
	     }                 
        }
   }

   else
   {
	int pipefd[2];
	char buf[BUFF_SIZE];

        for(int i=0;i<numAmberCommands;i++)
        {
	   pipe(pipefd);
           pid=fork();

           if(pid==0)
           {     strcpy(fullStringCopy,singleCommand[i]);
                 int empty=extractCommand(singleCommand[i],commandNoArgs);
		 close(pipefd[0]);

                 if(empty)
                      exit(EXIT_SUCCESS);

                  extractArguments(fullStringCopy,args);
                  int exeStatus=execvp(commandNoArgs,args);
                  if(exeStatus==-1)
                   {
                       perror("execution error\n");
		       write(pipefd[1],"e\n",2);
                       exit(EXIT_FAILURE);
                   }
           }

           else if(pid <0)
           {
                   perror("error\n");
                   exit(EXIT_FAILURE);
           }

           else
           {     
		   close(pipefd[1]);
	           read(pipefd[0],buf,BUFF_SIZE);	
	           if(buf[0]=='e')
		   {
			buf[0]='\0';
			break;
		   }
		
		   pidChild=fork();
                   if(pidChild<0)
                   {
                        perror("error\n");
                        exit(EXIT_FAILURE);
                   }
                   if(waitpid(pid,&forkStatus,0)!=pid && waitpid(pidChild,&forkStatusChild,0)!=pidChild)
                        exit(EXIT_SUCCESS);                       
	   }                
       }   
 }
}  


/* check the inserted string 
 * for any semicolons and split it
 * in different commands
 * */
int  semicolonDelim(char* fullString,char* command[])
{
        char* singleCommand;
        int counter=0;

	while(singleCommand=strtok_r(fullString,";",&fullString))
	{
	        command[counter]=singleCommand;
		counter++;
        }
	return counter; 
}


/* check the inserted string
 * for any ambersands and split in
 * in different commands
 * */
int ambersandDelim(char* fullString,char* command[])
{
	char* singleCommand;
	int counter=0;

	while(singleCommand=strtok_r(fullString,"&&",&fullString))
	{
		command[counter]=singleCommand;
		counter++;
	}
	return counter;
}


// extract the command arguments
void extractArguments(char* command,char* args[])
{ 
   int commandCounter=0;
   int charFlag=0;
   int spaceFlag=0;
   int firstTimeFlag=0;
   int argCounter=0;
   int argBreakPoint=0;

   for(int i=0;command[i];i++)
   {
	   if(!isspace(command[i]))
	   {
		   charFlag=1;
		   command[commandCounter]=command[i];
		   commandCounter++;
	   }
	   else if(isspace(command[i]))
	   {
		  spaceFlag=1;  
		  if(charFlag==1 && firstTimeFlag==0)
		  {
			  for(int j=0;j<commandCounter;j++)
				  args[argCounter][j]=command[j];
			  

			   argBreakPoint=commandCounter;
			   argCounter++;
			   firstTimeFlag=1;
		  }
		  else if(charFlag==1 && firstTimeFlag==1)
		  {
			  int index=argBreakPoint;
			  if(commandCounter!=argBreakPoint)
			  {
			      for(int j=0;j<(commandCounter-argBreakPoint);j++)
			      {
			          args[argCounter][j]=command[index];
				  index++;
			      }
			         argCounter++;
			         argBreakPoint=commandCounter;
			   }
		  }
	   }
   }

  args[argCounter]='\0';
}



// extract the command
int  extractCommand(char* fullString,char* commandNoArgs)
{
   int commandCounter=0;
   int charFlag=0;
   int spaceFlag=0;
   int firstTimeFlag=0;

   for(int i=0;fullString[i];i++)
   {
           if(!isspace(fullString[i]))
           {
                   charFlag=1;
                   fullString[commandCounter]=fullString[i];
                   commandCounter++;
           }
           else if(isspace(fullString[i]))
           {
                  spaceFlag=1;
                  
                  if(charFlag==1 && firstTimeFlag==0)
                  {
                          for(int j=0;j<commandCounter;j++)
                                  commandNoArgs[j]=fullString[j];
                          
                          firstTimeFlag=1;
                          commandNoArgs[commandCounter]='\0';
                  }	 
           }
   }
   
   // check for 'space' (no command inserted)
   if(!firstTimeFlag)
	   return 1;
   return 0;
}


/* reading the batch file
 * and sending each line (that is not empty)
 * for execution by the commandExecutionBatchMode function
 */
void batchFile(char* fileName)
{
   FILE* batch=fopen(fileName,"r");
   ssize_t read;
   size_t len=0;
   char* allCommands[MAX_COMMANDS];
   char* line;
	   
   for(int i=0;i<MAX_COMMANDS;i++)
	   allCommands[i]=(char*)malloc(sizeof(char)*MAX_CHAR);

   line=(char*)malloc(sizeof(char)*MAX_CHAR);

   if(batch==NULL)
   {
	   perror("Error,Cannot open the file\n");
	   perror("aborting..\n");
	   exit(EXIT_FAILURE);
   }
   
   int counter=0;
   while((read=getline(&line,&len,batch))!=-1)
   {     
	  if(read>1){  
	     strcpy( allCommands[counter],line);
	     counter++;
	    }
   }

   for(int i=0;i<counter;i++)
	      commandExecutionBatchMode(allCommands[i]);  
   
   free(line);
   fclose(batch);
   return;
}


// check for the special command quit
int quitCheck(char* fullString,char* commandNoArgs)
{  
    char fullStringParentCopy[MAX_CHAR];
    char commandNoArgsParent[MAX_CHAR];

    strcpy(fullStringParentCopy,fullString);
    extractCommand(fullStringParentCopy,commandNoArgsParent);
    if(strcmp(commandNoArgsParent,"quit")==0)
	    return 1;
    return 0;

}

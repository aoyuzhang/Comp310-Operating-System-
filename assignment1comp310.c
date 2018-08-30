#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <limits.h>
#include  <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

//
// This code is given for illustration purposes. You need not include or follow this
// strictly. Feel free to writer better or bug free code. This example code block does not
// worry about deallocating memory. You need to ensure memory is allocated and deallocated
// properly so that your shell works without leaking memory.
//

// Define the  "process" variable used as entry in the process list table.
typedef struct process {
   char name[100];
   pid_t pidd;	
   int statuss; // used to tell if a process is still running.//0 means terminated 1 means running	
} process;
// Creating an array to store the processes
process joblist[100];
//Global pid to store the forground child process.
pid_t gchildpid;
//Backgroud process array index
int indexja=0;
// An array to store the processes
int processList[100];

//Signal handler function
static void sigHandler(int sig)
{
	if(sig==SIGINT)
	{
		//printf("ll: %zu", gchildpid);
		if(gchildpid!=0)
		{
			//printf("Hey! caught signal %d\n",sig);
			kill(gchildpid,SIGKILL);
			gchildpid=0;
		}
	}	
}
// get the command and store it in args
int getcmd(char *prompt, char *args[], int *background)
{
	int length, i = 0;
 	char *token, *loc;
 	char *line = NULL;
 	size_t linecap = 0;
 	printf("%s", prompt);
 	length = getline(&line, &linecap, stdin);
	//printf("(%s)\n",line);
 	if (length <= 0) 
	{
 		exit(-1);
	}

	 // Check if background is specified..
	 if ((loc = index(line, '&')) != NULL) 
	 {
	 	*background = 1;
	 	*loc = ' ';
	 }
	  else
	 	*background = 0;
	 while ((token = strsep(&line, " \t\n")) != NULL) 
	 {
	 	for (int j = 0; j < strlen(token); j++)
	 		if (token[j] <= 32)
	 			token[j] = '\0';
	 	if (strlen(token) > 0)
	 		args[i++] = token;
	 }
	 return i;
}

// Get the index of in args in which > appears
int containsredirection(char* args[])
{
	int index=0;
	int limit=50;
	for(index=0;index<limit;index++)
	{
		
		if(args[index]!=NULL && strcmp(args[index],">")==0)
			return index;
	}

	return -1;	
}

// Get the index of in args in which | appears
int containspipe(char* args[])
{
	int index=0;
	int limit=50;
	for(index=0;index<limit;index++)
	{
		if(args[index]!=NULL && (strcmp(args[index],"|")==0))
		{
			//printf("%s the index is %d \n",args[index],index);
			return index;
		}
	}
	return -1;	
}

int main(void)
{ 
		char *args[20]; // array to store the commands
 		int bg;		// use to see if the process has to run in the backgroud	
		int pid;
		int redindex; // Contains the index in args that has the symbole ">".
		int pipeindex;
		// signal handler funcitons	
		if (signal(SIGINT,sigHandler) == SIG_ERR) //if caught signals ctrl_c.
		{
			printf("ERROR! Could not bind the signal hander\n");
			exit(1);
		}
		if(signal(SIGTSTP, SIG_IGN) == SIG_ERR) // If caught signal ctrl_z
		{
			printf("ERROR! Could not bind the signal hander\n");
			exit(1);
		}
	
	 while(1) 	
	{
		
		bg = 0;				//Backgroup parameter
		for(int i=1;i<sizeof(args);i++) // Clear the args array.
			args[i]=NULL;
		int cnt = getcmd("\n>> ", args, &bg);// get the commands from the keyboard
		//printf("cnt is%d\n",cnt);
		if(cnt==0)// If nothing is entered.
			continue;	

		redindex=containsredirection(args); // get the index of ">"
		pipeindex=containspipe(args); // get the index of "|"
		//printf("pipe index is %d\n",pipeindex);
		//printf("redirection index is %d\n",redindex);
		
		
		if(strcmp(args[0],"exit") == 0)// If the command is "exit", then exit the shell
		{
			printf("Thank you for useing the SHELL, See you soon!\n");
			exit(EXIT_SUCCESS);
		}
		else if(pipeindex>0) // If we want piping
		{
			printf("in execution :pipe index is %d\n",pipeindex);
			args[pipeindex]=NULL; // set the "|" to null in args array
			int des_p[2];
			int stdoutt;
			int stdinn;
	        if(pipe(des_p) == -1) {
	          perror("Pipe failed");
	          exit(1);
	        }
	        pid_t cpid1=fork();
	        if(cpid1 == 0)            //first fork
	        {

	            close(STDOUT_FILENO);  //closing stdout
	            dup(des_p[1]);         //replacing stdout with pipe write 
	            close(des_p[0]);       //closing pipe read
	            close(des_p[1]);
	            
	            execvp(args[0], args);
	            perror("execvp of ls failed");
	            exit(1);
	        }
	        printf("execvp 1 executed\n");
	        pid_t cpid2=fork();
	        if( cpid2== 0)            //creating 2nd child
	        {
	            close(STDIN_FILENO);   //closing stdin
	            dup(des_p[0]);         //replacing stdin with pipe read
	            close(des_p[1]);       //closing pipe write
	            close(des_p[0]);
	            int k=0;
	            char* argscpy[20]; //create this array to copy the string in the args array	

				for(k=1;k<100;k++)
     				{
     					if(args[pipeindex+k]!=NULL)
     					argscpy[k-1]=args[pipeindex+k];
     				}
     			argscpy[k]=NULL;
     			printf(" %s\n",argscpy[0]);
         		execvp(argscpy[0],argscpy);
	            perror("execvp of wc failed");
	            exit(1);
	        }
	        close(des_p[0]);
	        close(des_p[1]);
	        //dup2()
	        wait(0);
	        wait(0);
	        //return 0;
		
		// 	pid_t child_pid0 = fork(); // fork a child to execute the command
		// 	if (child_pid0 == (pid_t) -1) // fail to fork a child
		// 	{
		// 		printf("Error occurred during fork, abort\n");
		// 		exit(EXIT_FAILURE);
		// 	} 
		// 	else if(child_pid0 == (pid_t) 0) // if it is the child process
		// 	{
		// 		int p[2]; //pipping array.
		// 		if (pipe(p) == -1) // fail to create pipe
		// 		{
  //      		 		printf("Error occurred during pipe, abort\n");
  //       			exit(EXIT_FAILURE);
  //   			}
    			

  //   			pid_t child_pid1 = fork(); // fork a new child to execute the next command
  //   			if (child_pid1 == (pid_t) -1) // failed to fork a child
  //   			{
  //       			printf("Error occurred during fork, abort\n");
  //       			exit(EXIT_FAILURE);
  //   			} 
  //   			else if (child_pid1 == (pid_t) 0) 
  //   			{
  //   				close(0);
  //   				dup(p[0]);fork
  //   				char* argscpy[100]; //create this array to copy the strign in the args array
  //   				int k=0;
  //   				for(k=1;k<100;k++)
  //   				{
  //   					if(args[pipeindex+k]!=NULL)
  //   					strcpy(argscpy[k-1],args[pipeindex+k]);
  //   				}	
  //       			excvp(argscpy[0],argscpy);
  //   			} 
  //   			else 
  //   			{
  //   				close(1); //close std output
  //   				dup(p[1]); // connect pipe end to std output
  //       			excvp(args[0],args);
  //   			}
  //   		}	
		// 	else
		// 	{ 
		// 		int status, status1;
		// 		pid_t pid0 = waitpid(child_pid0,&status, 0);
		// 		pid_t pid1 = waitpid(child_pid1,&status1,0);
		// 	}

		}
		else if (redindex>0)// he command contains redirection symbole.
		{

			pid_t child_pid = fork();
			if (child_pid == (pid_t) -1) 
			{
				printf("Error occurred during fork, abort\n");
				exit(EXIT_FAILURE);
			} 
			else if(child_pid == (pid_t) 0) 
			{
				close(1);
				int fd_1 = open(args[redindex+1], O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);	//Open the file
				args[redindex]= NULL;	// make ">" NULL
				execvp(args[0],args);	// Replace the child process
				printf("Error occurred during execvp\n");
				exit(EXIT_FAILURE);
			}
			else
			{ 
				int status;
				pid_t pid = waitpid(child_pid,&status, 0);
			}
		}
		else if(strcmp(args[0],"cd")==0)// if the command is "cd" then do change directory
		{
			chdir(args[1]);
		}
		else if(strcmp(args[0],"pwd")==0)// if the command is "pwd", do something else
		{
			char* cwd;
    			char buff[PATH_MAX + 1];
    			cwd = getcwd( buff, PATH_MAX + 1 );
    			if( cwd != NULL ) 
				{
        			printf( "My working directory is %s.\n", cwd );
    			}

		}
		else if(strcmp(args[0],"jobs")==0) // If the command is "job" then do the following
		{
			int i=0;
			for(i=0;i<indexja;i++)
			{
				int status;
				printf("%d\n", joblist[i].pidd);
				pid_t pid = waitpid(joblist[i].pidd, &status, WNOHANG | WUNTRACED);
				//if (pid == -1) {
                       //perror("waitpid");
                       //exit(EXIT_FAILURE);
                  // }

				//if (pid == (pid_t) -1) 
				//{
		    	//	printf("Error has occurred, abourt\n");
		    //		exit(EXIT_FAILURE);
				//}
	       			if (WIFSTOPPED(status)|| WIFSIGNALED(status)) 
		    			joblist[i].statuss=0;
					printf("%d",joblist[i].statuss);
				
				if(joblist[i].statuss==0)
				printf("number: %d pid: %d name: %s, Status: stopped\n",i,joblist[i].pidd,joblist[i].name);
				else
				printf("number: %d pid: %d name: %s, Status: Running\n",i,joblist[i].pidd,joblist[i].name);
			}

		}
		else if(strcmp(args[0],"fg")==0) // If the command is fg.
		{
			int index=atoi(args[1]); // change the argument to an integer
			int status;
			pid_t pid = waitpid(joblist[index].pidd, &status, WNOHANG | WUNTRACED);
				if (pid == (pid_t) -1) 
				{
		    		printf("Error has occurred, abort\n");
		    		exit(EXIT_FAILURE);
				}
	       			if (WIFSTOPPED(status)) {
				joblist[index].statuss=0;
		    		printf("Process %d have stopped\n",index);
				} 
			
			if(index<=indexja && joblist[index].statuss==1)
			{
				int status;
				joblist[index].statuss==0;
				waitpid(joblist[index].pidd,&status, 0);

			}
		}	
		/* If the command entered is not the commands that we have to implement then use fork() to create a child process*/
		else
		{
			pid_t child_pid = fork();
			if (child_pid == (pid_t) -1) 
			{
				printf("Error occurred during fork, abort\n");
				exit(EXIT_FAILURE);
			}
			else if (child_pid == (pid_t) 0) 
			{
				/*
				if (bg == 1) {
					if (signal(SIGINT,SIG_IGN) == SIG_ERR) //ignor ctrl_c when in backgroud.
					{
						printf("ERROR! Could not bind the signal hander\n");
						exit(1);
					}
				}
				*/
				execvp(args[0],args);
				printf("Error occurred during execvp\n");
				exit(EXIT_FAILURE);
			} 
			// Add the process to the array and update fg child pid.
			else  // if child process run in backgroud
			{
				if (bg==1)
				{
					joblist[indexja].pidd=child_pid; // put the child process in the bg process table
					// check if the process has terminated
					int status;
					pid_t pid = waitpid(joblist[indexja].pidd, &status, WNOHANG | WUNTRACED);
					if (pid == (pid_t) -1) 
					{
		    				printf("Error has occurred, abourt\n");
		    				exit(EXIT_FAILURE);
					}
	       			if (WIFSTOPPED(status)||WIFSIGNALED(status)) // If the process has stopped
					{
						joblist[indexja].statuss=0;
						strcpy(joblist[indexja].name, args[0]);
						//printf("childpid is %d %d \n",joblist[indexja].pidd,gchildpid);
						indexja++;
					} 
					else // If the process has not yet stopped
					{
						joblist[indexja].statuss=1;
						strcpy(joblist[indexja].name, args[0]);
						//printf("childpid is %d %d \n",joblist[indexja].pidd,gchildpid);
						indexja++;
					}
				}
				else // If we don't run in backgroud
				{
					gchildpid=child_pid;
					int status;
	       			//printf("Start waiting for child process to termiate\n");
					pid = waitpid(child_pid,&status, 0); 
	       			//printf("Child process terminated\n");
				}
			}
 
		}
}
}
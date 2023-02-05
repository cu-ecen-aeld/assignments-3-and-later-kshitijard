#include "systemcalls.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <syslog.h>


#define FILE_PERMISSION   (0644)
#define FAIL               ( -1 )
/**
 * @param cmd the command to execute with system()
 * @return true if the commands in ... with arguments @param arguments were executed 
 *   successfully using the system() call, false if an error occurred, 
 *   either in invocation of the system() command, or if a non-zero return 
 *   value was returned by the command issued in @param.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success 
 *   or false() if it returned a failure
*/
     int ret_status;
    
    ret_status = system(cmd);   // system call
    
    if (ret_status == FAIL )    // Error check for child process created
    {
    	perror("ERROR: system Error");
    	return false;
    }
    
    if ( ! (WIFEXITED(ret_status)) ||  (WEXITSTATUS(ret_status) ) )
    {
            perror("ERROR: Command execution fail");
            return false;
     }
        
    printf("System successfull");
    return true;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the 
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *   
*/

 int ret_status;
 
    pid_t pid;

    pid = fork();
    
    if (pid == FAIL)    // Error check if fork failed
    {
    	perror("ERROR: Fork failed");
    	return false;
    }
    
    else if (pid == 0)   // Inside child process
    {

    	printf("Child created successfully with pid %d", pid);
    	
    	execv(command[0], command);
    	
    	perror("ERROR: execv failed");  // This line executes if exec fails

    	exit(EXIT_FAILURE);  
    }
    
    else
    {

    	printf("Parent process with pid %d", pid);
    	if(waitpid(pid,&ret_status,0) == FAIL)      // Inside parent process
    	{

            perror("ERROR: Wait pid failed");
            return false;
        }
        
        if ( ! (WIFEXITED(ret_status)) || (WEXITSTATUS(ret_status) ) )  // Exit status of chold process
        {
            
            perror("ERROR: Wait pid failed");
            return false;
        }
    }

    va_end(args);

    printf("Execv success");

    return true;
}

/**
* @param outputfile - The full path to the file to write with command output.  
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *   
*/

    
    int status;
    pid_t pid;

    int fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, FILE_PERMISSION);
    if(fd == FAIL)
    {

        perror("ERROR: File not created/opened");
        return false;
    }

    pid = fork();
    
    if (pid == FAIL)  // Error check of child process
    {

    	perror("ERROR: Fork failed");
    	
    	return false;
    }
    
    else if (pid == 0) // Child process
    { 
	printf("Child created successfully with pid %d", pid);
	
        if(dup2(fd,1) < 0)
        {
       
            perror("ERROR: Dup2 error");
            return false;
        }
        
        close(fd);
        
        execv(command[0],command);

    	perror("ERROR: exev failed");   // This line executes if exec fails
    	
    	exit(EXIT_FAILURE);
    }
    else
    {
        close(fd);

    	printf("Parent process with pid %d", pid);
    	
    	if(waitpid(pid,&status,0) == FAIL)    // Inside parent process
    	{
            perror("ERROR: Waitpid failed");
            return false;
        }
        
        if ( ! (WIFEXITED(status)) ||  (WEXITSTATUS(status) ) )  // Exit status of chold process
        {
            perror("ERROR: Wait pid failed");
            return false;
        }

    }

    va_end(args);
    printf("Execv success");
    return true;
}

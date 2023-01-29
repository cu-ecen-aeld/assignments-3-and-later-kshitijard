/*******************************************************************************************
   File Name           : writer.c
   Author Name         : Kshitija Dhondage 
   Compiler            : gcc and aarch64-none-linux-gnu-gcc 
   Date                : 29th January 2023
   references:         :https://stackoverflow.com/questions/23092040/how-to-open-a-file-which-overwrite-existing-content/23092113
  
*******************************************************************************************/

/*includes*/

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/*Macros*/
#define TOTAL_ARGC         ( 3  )
#define FILE_PERMMISSION   (0644)
#define FAIL               ( -1 )

int main(int argc, char *argv[])
{

	char *writeFile;
	char *writeStr;
	
	int fd;
	int ret_status = 0;
	
	/* Initialize syslog connection */
	openlog(NULL, 0, LOG_USER);

        /* Validate arguments */
	if (argc != TOTAL_ARGC)
	{
		syslog(LOG_ERR, "Invalid arguments\n");
		syslog(LOG_INFO, "Argument 1 < File path>\n");
		syslog(LOG_INFO, "Argument 2 String to be added \n");
		exit(1);
	}

       /* Copy arguments in local variables */
	writeFile = argv[1];
	writeStr  = argv[2];

        /* Open file with the input path and file permissions */
	fd = open(writeFile, O_RDWR | O_CREAT | O_TRUNC, FILE_PERMMISSION);
	/* Validate if file is open sucessfully */
	if (fd == FAIL)
	{
		syslog(LOG_ERR, "Directory not found/created %s \n\rError %d!!\n", writeFile, errno);
		exit(1);
	}
	/* Execute write operation to file */	
	else {	
	        ret_status = write(fd, writeStr, strlen(writeStr));
		if (ret_status == FAIL)
		{
			syslog(LOG_ERR, "Write to file at %s failed \n\rError %d!!\n", writeFile, errno);
			exit(1);
		}
		/* Validate if total string is written sucessfully or failed */
		else if (ret_status != strlen(writeStr))
		{
			syslog(LOG_ERR, "Write to file at %s not completed \n\rError %d!!\n", writeFile, errno);
		}
		/* File written with input string sucessfully */
		else
		{
			syslog(LOG_DEBUG, "Writing %s to %s/n/r", writeStr, writeFile);
		}
	}

        /* CLose the file */
	ret_status = close(fd);
	/* Validate if file is closed sucessfully */
	if (ret_status == FAIL)
	{
		syslog(LOG_ERR, "File not closed  %d\n", errno);
	}

       /* Close syslog connection */
	closelog();
	return 0;
}

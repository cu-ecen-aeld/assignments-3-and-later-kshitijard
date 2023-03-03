/***********************************************************************
 *@file      aesdsocket.c
 *@version   0.1
 *@brief     Function implementation file.
 *
 *@author    Kshitija Dhondage, ksdh4214@Colorado.edu
 *@date      Feb 24, 2022
 *
 *@institution University of Colorado Boulder (UCB)
 *@assignment Assignment 5
 *@due        
 *
 *@resources   - Linux system programming book
 **********************************************************************/
/*
 *Header files
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/queue.h>
#include <sys/time.h>

/*
 *MACRO definition
 */
#define RESET             0
#define PORT             "9000"
#define FILE_PERMISSIONS (0644)
#define BUFF_SIZE        100
#define FILE_PATH        "/var/tmp/aesdsocketdata"

typedef struct
{
	int clifd;
	pthread_mutex_t * mutex;
	pthread_t thread_id;
	bool thread_complete;
}

thread_parameter;

//Linked list node
struct slist_data_s
{
	thread_parameter thread_params;
	SLIST_ENTRY(slist_data_s) entries;
};
typedef struct slist_data_s slist_data_t;
pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 *Global variables
 */
int sockfd, clifd, filefd;
int status;
char *cli_buff;
int packet_length = RESET;
char rx_buff[BUFF_SIZE];
slist_data_t *data_node = NULL;
SLIST_HEAD(slisthead, slist_data_s) head;

/*
 *Function Name: signal_handler (int signal)
 *Description: handles signal occurered during execution
 *return: void
 */
static void signal_handler(int signal)
{
	switch (signal)	// signal handler
	{
		case SIGINT:
			syslog(LOG_INFO, "SIGINT signal received\n");
			printf("SIGINT occured\n");
			break;

		case SIGTERM:
			syslog(LOG_INFO, "SIGTERM signal received\n");
			printf("SIGTERM occured\n");
			break;

		case SIGKILL:
			syslog(LOG_INFO, "SIGkill signal received\n");
			printf("SIGKILL occured\n");
			break;

		default:
			syslog(LOG_ERR, "Unknown Signal received\n");
			exit(EXIT_FAILURE);
			break;

	}

	while (SLIST_FIRST(&head) != NULL)
		{
			SLIST_FOREACH(data_node, &head, entries)
			{
				close(data_node->thread_params.clifd);
				pthread_join(data_node->thread_params.thread_id, NULL);
				SLIST_REMOVE(&head, data_node, slist_data_s, entries);
				free(data_node);
				break;
			}
		}
		
	// clear and free buffers
	syslog(LOG_INFO, "Clearing buffers and Exiting\n");
	printf("Clearing buffers and Exiting\n");
	unlink(FILE_PATH);
	close(sockfd);
	close(clifd);
	exit(EXIT_SUCCESS); 

}

/*
 *Function Name: void* thread_handler(void* thread_param)
 *Description: Handles thread specific functionalities
 *return: int
 */
void *thread_handler(void *thread_param)
{
	int i;
	bool packet_status = false;
	ssize_t rx_data = RESET;
	ssize_t wr_data = RESET;
	char char_read = RESET;

	thread_parameter *t_params = (thread_parameter*) thread_param;
	
	//Allocate appropriate buffer memory
	cli_buff = (char*) malloc((sizeof(char) *BUFF_SIZE));
	if (cli_buff == NULL)
	{
		syslog(LOG_ERR, "Failed to allocate memory\n");
		printf("malloc() failed\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		syslog(LOG_INFO, "Memory allocation successfull\n");
		printf("malloc() succed\n");
	}

	memset(cli_buff, RESET, BUFF_SIZE);

	while (!packet_status)
	{
		// Recieve data
		rx_data = recv(t_params->clifd, rx_buff, BUFF_SIZE, 0);
		if (rx_data < 0)
		{
			syslog(LOG_ERR, "Failed to receive:%d\n", status);
			printf("Receive data failed\n");
			exit(EXIT_FAILURE);
		}
		else if (rx_data == 0)
		{
			syslog(LOG_INFO, "Reception successfull\n");
			printf("Received data successfully\n");
		}

		//Traverse through end of packet
		for (i = 0; i < BUFF_SIZE; i++)
		{
			if (rx_buff[i] == '\n')
			{
				packet_status = true;
				i++;
				break;
			}
		}

		packet_length += i;

		//reallocate memory to appropriate size 
		cli_buff = (char*) realloc(cli_buff, (packet_length + 1));
		if (cli_buff == NULL)
		{
			syslog(LOG_ERR, "Realloc failed\n");
			printf("realloc failed\n");
			exit(EXIT_FAILURE);
		}
		else
		{
			syslog(LOG_INFO, "Reallocation successfull\n");
		}

		strncat(cli_buff, rx_buff, i);
		memset(rx_buff, 0, BUFF_SIZE);
	}

	status = pthread_mutex_lock(t_params->mutex);
	if (status)
	{
		syslog(LOG_ERR, "Mutex loxk failed %d\n", status);
		exit(EXIT_FAILURE);
	}

	//Open file to write data
	filefd = open(FILE_PATH, O_APPEND | O_WRONLY);
	if (filefd == -1)
	{
		syslog(LOG_ERR, "Failed to create file for appending with Error code:%d\n", filefd);
		printf("File open failed\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		syslog(LOG_INFO, "File created for appending\n");
		printf("File open succed\n \n");
	}

	//Write data to file
	wr_data = write(filefd, cli_buff, strlen(cli_buff));	//start writing to the file
	if (wr_data == -1)
	{
		syslog(LOG_ERR, "Failed to write file with Error code");
		printf("File write failed\n");
		exit(EXIT_FAILURE);
	}
	else if (wr_data != strlen(cli_buff))
	{
		syslog(LOG_ERR, "file written partially\n");
		printf("File written partially \n");
		exit(EXIT_FAILURE);
	}
	else
	{
		syslog(LOG_INFO, "File weitten successfully!\n");
		printf("File written success\n");
	}

	close(filefd);

	//Open file to read data
	filefd = open(FILE_PATH, O_RDONLY);
	if (filefd == -1)
	{
		syslog(LOG_ERR, "Fail to open file for reading\n");
		printf("File to open file for read \n");
		exit(EXIT_FAILURE);
	}
	else
	{
		syslog(LOG_INFO, "File opened successfully for reading\n");
		printf("File open to read success \n");
	}

	for (int i = 0; i < packet_length; i++)
	{
		//start reading from the file
		status = read(filefd, &char_read, 1);
		if (status == -1)
		{
			syslog(LOG_ERR, "Failed to read\n");
			printf("Failed to read \n");
			exit(EXIT_FAILURE);
		}
		else
		{
			syslog(LOG_INFO, "Read successfull\n");
		}

		//send the data
		status = send(t_params->clifd, &char_read, 1, 0);
		if (status == -1)
		{
			syslog(LOG_ERR, "Failed to send");
			printf("Failed to send \n");
			exit(EXIT_FAILURE);
		}
		else
		{
			syslog(LOG_INFO, "Send successfullyn");
		}
	}

	status = pthread_mutex_unlock(t_params->mutex);
	if (status)
	{
		syslog(LOG_ERR, "pthread_mutex_unlock() failed with error number %d\n", status);
		exit(EXIT_FAILURE);
	}

        t_params->thread_complete = true;
	//close file and clear buffer
	close(filefd);
	free(cli_buff);
	close(t_params->clifd);

	return t_params;

}

/*
 *Function Name: handle_socket()
 *Description: handles socket communication
 *return: void
 */
static void handle_socket()
{
	// local variables for socket communication

	//SLIST_HEAD(slisthead, slist_data_s) head;
	SLIST_INIT(&head);
	struct sockaddr_in client_address;
	struct addrinfo hints;
	struct addrinfo * param;
	struct itimerval t_interval;
	socklen_t client_address_length;

	memset(rx_buff, RESET, BUFF_SIZE);

	//Settings structure parameters
	memset(&hints, RESET, sizeof(hints));

	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	//get address information and store in structure
	status = getaddrinfo(NULL, PORT, &hints, &param);
	if (status != 0)
	{
		syslog(LOG_ERR, "getaddrinfo failed with Error code: %d\n", status);
		printf("getaddrinfo failed\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		syslog(LOG_INFO, "getaddrinfo SUCCESS\n");
		printf("getaddrinfo success\n\n");
	}

	// Open the socket connection
	sockfd = socket(AF_INET6, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		syslog(LOG_ERR, "Socket failed to open connection with Error code:%d\n", sockfd);
		printf("Socket open failed\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		syslog(LOG_INFO, "Socket connection opened successfully\n");
		printf("socket opened successfull\n");
	}

	status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int)
	{ 1 }, sizeof(int));
	if (status == -1)
	{
		syslog(LOG_ERR, "setsockopt() failed.\n");
		printf("setsockopt() failed\n");
		exit(EXIT_FAILURE);
	}

	syslog(LOG_INFO, "setsockopt() succeeded!\n");
	printf("setsockopt() succeeded\n");

	//Bind socket connection
	status = bind(sockfd, param->ai_addr, param->ai_addrlen);
	if (status == -1)
	{
		syslog(LOG_ERR, "Binding failed with Error code:%d\n", status);
		printf("Bind error\n");
		printf("Bind error:%s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	else
	{
		syslog(LOG_INFO, "Binding success\n");
		printf("Bind success\n");
	}

	freeaddrinfo(param);

	//create a file with desired file path and permissions
	filefd = creat(FILE_PATH, FILE_PERMISSIONS);
	if (filefd == -1)
	{
		syslog(LOG_ERR, "Failed to create file with Error code:%d\n", filefd);
		printf("File creaation failed\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		syslog(LOG_INFO, "File created successfully\n");
		printf("File create successfull\n");
	}

	// Close file
	close(filefd);

	t_interval.it_interval.tv_sec  = 10;	// 10 secs interval
	t_interval.it_interval.tv_usec = 0;
	t_interval.it_value.tv_sec     = 10;	//10 secs expiry
	t_interval.it_value.tv_usec    = 0;

	status = setitimer(ITIMER_REAL, &t_interval, NULL);
	if (status == -1)
	{
		syslog(LOG_ERR, "Failed in Set timer function");
		printf("setitimer() failed\n");
		exit(EXIT_FAILURE);
	}

	//Handles inital socket connection until interrupted by signal
	while (1)
	{
		//Listen to socket connection
		status = listen(sockfd, 10);
		if (status == -1)
		{
			syslog(LOG_ERR, "Listen failed:%d\n", status);
			printf("Listen Failed\n");
			exit(EXIT_FAILURE);
		}
		else
		{
			syslog(LOG_INFO, "Listen Successfull\n");
			printf("Listen Successfull\n");
		}

		//Accept the socket connection
		client_address_length = sizeof(struct sockaddr);

		clifd = accept(sockfd, (struct sockaddr *) &client_address, &client_address_length);
		if (clifd == -1)
		{
			syslog(LOG_ERR, "Accepting connection failed\n");
			printf("accept connection failed\n");
			exit(EXIT_FAILURE);
		}
		else
		{
			syslog(LOG_INFO, "accept succeeded! Accepted connection from: %s\n", inet_ntoa(client_address.sin_addr));
			printf("accept connection success\n");
		}

		//thread parameters
		data_node = (slist_data_t*) malloc(sizeof(slist_data_t));
		SLIST_INSERT_HEAD(&head, data_node, entries);
		data_node->thread_params.clifd = clifd;
		data_node->thread_params.thread_complete = false;
		data_node->thread_params.mutex = &mutex_lock;

		pthread_create(&(data_node->thread_params.thread_id),	//Thread ID
			NULL,	//Thread attribute
			thread_handler,	//Thread Handler
			&data_node->thread_params);	//Thread parameters

		printf("Threads Created\n");

		SLIST_FOREACH(data_node, &head, entries)
		{
			pthread_join(data_node->thread_params.thread_id, NULL);
			//if (data_node->thread_params.thread_complete == true)
			//{
//				pthread_join(data_node->thread_params.thread_id,NULL);
				SLIST_REMOVE(&head, data_node, slist_data_s, entries);
				free(data_node);
				break;
			//}
		}

		printf("Threads Excited\n");
		printf("Closed connection from %s\n", inet_ntoa(client_address.sin_addr));
	}

	//close file and clear buffer
	close(filefd);
	close(sockfd);
	syslog(LOG_INFO, "Closed connection with %s\n", inet_ntoa(client_address.sin_addr));
}

/*
 *Function Name: static void timer_handler(int signal)
 *Description: Signal handler for timer
 *return: void
 */
static void timer_handler(int signal)
{
	time_t Tmr;
	struct tm * t_ptr;
	int t_length = 0;
	char t_str[200];
	int fd, data_write;

	Tmr = time(NULL);

	t_ptr = localtime(&Tmr);
	if (t_ptr == NULL)
	{
		syslog(LOG_ERR, "localtime failed\n");
		perror("Local timer error!");
		exit(EXIT_FAILURE);
	}

	t_length = strftime(t_str, sizeof(t_str), "timestamp: %m/%d/%Y - %k:%M:%S\n", t_ptr);
	if (t_length == RESET)
	{
		syslog(LOG_ERR, "strftime failed\n");
		perror("strftime timer error!");
		exit(EXIT_FAILURE);
	}

	printf("%s\n", t_str);

	//writing the time to the file
	fd = open(FILE_PATH, O_APPEND | O_WRONLY);
	if (fd == -1)
	{
		syslog(LOG_ERR, "File opening failed\n");
		exit(EXIT_FAILURE);
	}

	status = pthread_mutex_lock(&mutex_lock);
	if (status)
	{
		syslog(LOG_ERR, "Mutex lock error %d\n", status);
		exit(EXIT_FAILURE);
	}

	data_write = write(fd, t_str, t_length);
	if (data_write == -1)
	{
		syslog(LOG_ERR, "Fail to write on file\n");
		exit(EXIT_FAILURE);
	}
	else if (data_write != t_length)
	{
		syslog(LOG_ERR, "File failed to write completely\n");
		exit(EXIT_FAILURE);
	}

	//Update variable for packet length
	packet_length += t_length;

	status = pthread_mutex_unlock(&mutex_lock);
	if (status)
	{
		syslog(LOG_ERR, "Mutex UNlock error %d\n", status);
		exit(EXIT_FAILURE);
	}

	close(fd);
}

/*
 *Function Name: main(int argc, char *argv[])
 *Description: Entry point function
 *return: int
 */
int main(int argc, char *argv[])
{
	//open syslog for logging
	openlog("aesd-socket", LOG_PID, LOG_USER);

	if ((argc > 1) && (!strcmp("-d", (char*) argv[1])))
	{
		status = daemon(0, 0);
		if (status == -1)
		{
			printf("Deamon failed!\n");
			syslog(LOG_DEBUG, "Daemon mode failed!");
		}
	}

	if (signal(SIGINT, signal_handler) == SIG_ERR)
	{
		syslog(LOG_ERR, "SIGINT handler failed\n");
		exit(EXIT_FAILURE);
	}

	if (signal(SIGTERM, signal_handler) == SIG_ERR)
	{
		syslog(LOG_ERR, "SIGTERM handler failed\n");
		exit(EXIT_FAILURE);
	}

	if (signal(SIGALRM, timer_handler) == SIG_ERR)
	{
		syslog(LOG_ERR, "SIGALRM handler failed\n");
		exit(EXIT_FAILURE);
	}

	/*if (signal (SIGKILL, signal_handler) == SIG_ERR) {
		syslog(LOG_ERR, "SIGKILL handler failed\n");
		exit (EXIT_FAILURE);
	}*/

	pthread_mutex_init(&mutex_lock, NULL);

	handle_socket();

	//closing syslog
	closelog();

	return 0;
}

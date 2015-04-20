#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <netdb.h>

// This program is to transfer file in form of packets from client to TCPD Client

#define MSS 1000 				
#define header_size 16

typedef struct troll_message 
	{
		struct sockaddr_in header;
		char body[MSS-header_size];
	} troll_message;
	
typedef struct 
	{
		int free_size;
	}	ack_buffer;
	
typedef struct
{
	char data[MSS];
}	send_buffer;

typedef struct
	{
	int file_size;
	char filename[20];
	char server_ip[100];
	int server_port;
	} first_message;

// Determine file size of the file
int det_file_size(FILE * fp)
	{
		int prev = ftell(fp);
		fseek(fp, 0L, SEEK_END);
		int file_size = ftell(fp);
		fseek(fp,prev,SEEK_SET); //go back to where we were
		return file_size;
	}
	
// Create a socket address based on the ip_adress and port 
struct sockaddr_in get_sockaddr_send(char * ip_address, int port)
{
	struct sockaddr_in server_address;
	struct hostent * server_addr;
	server_addr = gethostbyname("localhost");

	if (server_addr == NULL)
	{
			printf("Client: Could not get server hostanme: %d\n",errno);
			exit(0);
	}

	memcpy(&server_address.sin_addr, server_addr->h_addr_list[0],server_addr->h_length);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	return server_address;
}

// Create a socket address / for bindng the ports to listen on particlaur port (and Ip adress)
struct sockaddr_in get_sockaddr_recv(int port)
{
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = htons(INADDR_ANY);
	return address;
}
	
// Check Socket for Error
void check_socket(int client_socket, char * socketname, char * type)
{
if (client_socket < 0)
	{
	printf(" %s: %s error: %d\n", type,socketname,errno);
	exit(0);
	}
	printf("%s: %s succesfull\n", type, socketname);
}
	
// Create first message function to create parameters that needs to be passed 

void create_first_message(first_message * first_msg, int filesize , char * filename, char * server_ip, int port)
{
	first_msg->file_size = filesize;
	memcpy(first_msg->filename, filename, 20); 
	memcpy(first_msg->server_ip, filename, 100);
	first_msg->server_port = port;
}	

// check bind function
void check_bind(int bind_id, char * socketname, char * type)
{
if (bind_id < 0)
	{
	printf("%s: %s , Bind Error: %s\n", type,socketname, strerror(errno));
	exit(0);
	}
	printf("%s: %s successful\n", type, socketname);
}

/* Function calls to be implemented as per the project requirements */

int SOCKET()
	{
	int d = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	return d;
	}

int BIND(int socket, struct sockaddr_in address)
	{
	int bind_id = bind(socket, (struct sockaddr *)&address, sizeof(address));
	return bind_id;
	}

int ACCEPT()
	{
	printf(" Passing Accept function -- (which is a null function) \n");
	return 1;
	}

int CONNECT()
	{
	printf(" Passing Connect function -- (which is a null function) \n");
	return 1;
	}

int SEND(int client_socket, char * msg, int sizeofmsg, struct sockaddr_in address)
	{
	int d = sendto(client_socket,msg,sizeofmsg, 0, (struct sockaddr *)&address, sizeof(address));
	return d;
	}

int RECV(int socket, char * buffer, int buf_len ,struct sockaddr_in address, int server_name_len)
	{
	int d =recvfrom(socket, buffer, buf_len, 0, (struct sockaddr *)&address, &server_name_len);
	return d;
	}

int CLOSE()
	{
	printf(" Passing Close function -- (which is a null function) \n");
	return 1;	
	}	




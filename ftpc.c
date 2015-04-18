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

// This program is to create a file transfer protocol server
// This program is part of Homework 2

// Assumptions
// The port through which we are transferring files is PORT_NUM
// The following attributes define the given problem specifications.

#define MSS 1000 				
#define SIZE_OF_BYTES 4
#define NAME_OF_FILENAME 20
#define MAX_SIZE_OF_FNAME 256
#define BAD_FNAME 777
#define header_size 16

struct sockaddr_in dest, troll;
struct sockaddr_in header1;

// Structure for troll messages
typedef struct troll_message 
	{
		struct sockaddr_in header;
		char body[MSS-header_size];
	} troll_message;

/* 
	Determine the file size of the fp
  	Returns the no of bytes ( int)		
*/

int det_file_size(FILE * fp)
	{
		int prev = ftell(fp);
		fseek(fp, 0L, SEEK_END);
		int file_size = ftell(fp);
		fseek(fp,prev,SEEK_SET); //go back to where we were
		return file_size;
	}

/*
int send_udp(client_socket, msg, strlen(msg), 0, server_address, sizeof(server_address))
	{
		int d = sendto(client_socket, msg, strlen(msg), 0, server_address, sizeof(server_address));
		return d;
	}

int recv_udp(client_socket, msg, strlen(msg), 0, server_address, sizeof(server_address))
	{
		int d = recvfrom(client_socket, msg, strlen(msg), 0, server_address, sizeof(server_address));
		return d;
	}
*/

main(int argc, char * argv[] )
	{

	// Declarations 
	char * server_host;
	char * dest_host; 
	char * filename;
	int PORT_NUM;
	int PORT_NUM_DEST;
	PORT_NUM = 5555;

	struct 
	{
	struct sockaddr_in header;
	char body[MSS-header_size];
	} tr_msg1;

	// Check for input arguments
	if (argc > 1)
	{
		printf(" The remote, destination IP passed is %s\n",argv[1]);
		printf(" The Destination Port number passed  is %s\n",argv[2]);
		printf(" The file to be transferred is %s\n",argv[3]);

		PORT_NUM_DEST = atoi(argv[2]);
		server_host = argv[1];
		filename = argv[3];
	}

	else
	{
		printf(" The arguments passed are in suffiecient \n");
		printf(" Please pass valid IP, port number and filetobe transferred\n");
		exit(0);
	}

	// Every packet sent to troll should contain the destination adress 

	troll_message  tr_msg;
	struct hostent * dest_addr;
	dest_addr = gethostbyname(argv[1]);

	if (dest_addr == NULL)
	{
			printf("Client: Could not get dest hostanme: %d\n",errno);
	}
	tr_msg.header.sin_family = htons(AF_INET);
	tr_msg.header.sin_port = htons(PORT_NUM_DEST);
	bcopy((char *)dest_addr->h_addr, (char *)&tr_msg.header.sin_addr.s_addr, dest_addr->h_length);
	memset((void*)tr_msg.body,'\0',sizeof(tr_msg.body));

	// Adress declarations
	struct sockaddr_in server_address;
	int new_client,client_length;
	int pid;

	// Socket creation
	printf("Client: Creating socket at server end\n");

	int client_socket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if (client_socket < 0)
			{
			printf("Client: Socket error: %d\n",errno);
			exit(0);
			}
	printf("Client: Socket succesfull\n");

	struct hostent * server_addr;
	server_addr = gethostbyname("localhost");

	if (server_addr == NULL)
	{
			printf("Client: Could not get server hostanme: %d\n",errno);
	}


	// Multiple ways to assign server adress 
	memcpy(&server_address.sin_addr, server_addr->h_addr_list[0],server_addr->h_length);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT_NUM);


	char recvbuffer[MSS];
	int ack = 1;
	int c_name_len = sizeof(server_address);

	FILE * fp;

	fp = fopen(filename,"rb");
	if (fp == NULL)
		{
		printf("Can't open the given file %s : Errono %d\n",filename,errno);
		exit(1);
		}

	int buffer_size = MSS-header_size;
	int remaining = det_file_size(fp);
	int * file_size_pointer = &(remaining);

	char * buffer = malloc(sizeof(char)*buffer_size);
	memcpy(&tr_msg.body,file_size_pointer,4);
	memcpy(&tr_msg.body+4,filename,20);
	
	char filename1[20];
	char zza[4];
	memcpy(zza, &tr_msg.body,4);
	memcpy(filename1, &tr_msg.body+4, 20);

	int fdsa = *(int *)(zza);

	
	// send the first packet 
	int buf_count = 0;
	
	printf("Sending the first message\n");
	ack = sendto(client_socket,&tr_msg,sizeof(tr_msg) , 0, (struct sockaddr *)&server_address, sizeof(server_address));

	while ( ack < 0)
		{
		printf("Client: Failed -- Data transfer failed for buf_count  %d\n", buf_count);
		ack = sendto(client_socket,&tr_msg,sizeof(tr_msg) , 0, (struct sockaddr *)&server_address, sizeof(server_address));
		}

	time_t now;
	char buff[100];
	int len;

	while ( remaining/buffer_size >= 1)
	{
		printf("Server: a, %d, %d \n", remaining, remaining/buffer_size);
		printf(" Client :Buf_count %d \n",buf_count);
		len = fread(buffer,sizeof(char),buffer_size,fp);
		now = time(0);
		strftime (buff, 100, "%Y-%m-%d %H:%M:%S.000", localtime (&now));
		printf("Client: %d \n",remaining- buffer_size);
	
		if ( len < 0)
		{
		printf(" Client: Failed to read for buf_count %d\n", buf_count);
		}
		memcpy(&tr_msg.body,buffer,buffer_size );
		ack = sendto(client_socket,&tr_msg,sizeof(tr_msg) , 0, (struct sockaddr *)&server_address, sizeof(server_address));

		while ( ack < 0)
		{
		printf("Client: Failed -- Data transfer failed for buf_count  %d\n", buf_count);
		ack = sendto(client_socket,&tr_msg,sizeof(tr_msg) , 0, (struct sockaddr *)&server_address, sizeof(server_address));
		}
		remaining -= buffer_size;
		buf_count++;
		usleep(10000);
	}

	// Sending the last message 
	
	int buffer_size1 = remaining;
	len = fread(buffer,sizeof(char),buffer_size1,fp);
	
	if ( len < 0)
	{
	printf(" Client: Failed to read for buf_count %d\n", buf_count);
	}

	memcpy(&tr_msg.body,buffer,buffer_size1 );

	ack = sendto(client_socket,&tr_msg,sizeof(tr_msg) , 0, (struct sockaddr *)&server_address, sizeof(server_address));
	
	while ( ack < 0)
	{
	printf("Client: Failed -- Data transfer failed for buf_count  %d\n", buf_count);
	ack = sendto(client_socket,&tr_msg,sizeof(tr_msg) , 0, (struct sockaddr *)&server_address, sizeof(server_address));
	}
	
	printf("Client: Last Packet sent \n");

	// End of ftp

}


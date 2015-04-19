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

// Determine file size of the file
int det_file_size(FILE * fp)
	{
		int prev = ftell(fp);
		fseek(fp, 0L, SEEK_END);
		int file_size = ftell(fp);
		fseek(fp,prev,SEEK_SET); //go back to where we were
		return file_size;
	}

typedef struct first_message
	{
	int file_size;
	char filename[20];
	int p1;
	int p2;
	int p3;
	char serevr_ip[50];
	char client_ip[50];	
	} first_message;

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
	int d = sendto(client_socket,&msg,sizeofmsg, 0, (struct sockaddr *)&address, sizeof(address));
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
	
	
	/*  -- troll message --

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

	*/

	// Adress declarations
	
	int new_client,client_length;
	int pid;

	/* Socket creation */
	printf("Client: Creating socket at client end\n");

	int client_socket = SOCKET();
	if (client_socket < 0)
			{
			printf("Client: Socket error: %d\n",errno);
			exit(0);
			}
	printf("Client: Socket succesfull\n");
	
	
	// Get the adress of the TCPD_client which is running on the same machine as ftpc.c //
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
	server_address.sin_port = htons(PORT_NUM);

	
	//char recvbuffer[MSS];
	//int ack = 1;
	//int c_name_len = sizeof(server_address);

	FILE * fp;
	
	// Open the file which needs to be sent to server
	fp = fopen(filename,"rb");
	if (fp == NULL)
		{
		printf("Can't open the given file %s : Errono %d\n",filename,errno);
		exit(1);
		}
	
	
	int buffer_size = MSS;
	int remaining = det_file_size(fp);
	//int * file_size_pointer = &(remaining);

	char * buffer = malloc(sizeof(char)*buffer_size);
	//memcpy(buffer,file_size_pointer,4);
	//memcpy(buffer+4,filename,20);
	
	//char filename1[20];
	//char zza[4];
	//memcpy(zza, &tr_msg.body,4);
	//memcpy(filename1, &tr_msg.body+4, 20);

	//int fdsa = *(int *)(zza);

	// Create first message for sending critical info
	first_message * first_msg = (first_message *)malloc(sizeof(first_message));
	first_msg->file_size = 100;
	memcpy(first_msg->filename ,argv[3],20);		
	

	// send the first packet 
	int buf_count = 0;
	
	printf("Sending the first message\n");
	int ack = sendto(client_socket,first_msg, sizeof(first_message), 0, (struct sockaddr *)&server_address, sizeof(server_address));

	while ( ack < 0)
		{
		printf("Client: Failed -- Data transfer failed for buf_count  %d\n", buf_count);
		ack = sendto(client_socket,first_msg, sizeof(first_message), 0, (struct sockaddr *)&server_address, sizeof(server_address));
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
		ack = sendto(client_socket,buffer, buffer_size , 0, (struct sockaddr *)&server_address, sizeof(server_address));

		while ( ack < 0)
		{
		printf("Client: Failed -- Data transfer failed for buf_count  %d\n", buf_count);
		ack = sendto(client_socket,buffer, buffer_size, 0, (struct sockaddr *)&server_address, sizeof(server_address));
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

	ack = sendto(client_socket,buffer, buffer_size1, 0, (struct sockaddr *)&server_address, sizeof(server_address));
	
	while ( ack < 0)
	{
	printf("Client: Failed -- Data transfer failed for buf_count  %d\n", buf_count);
	ack = sendto(client_socket,buffer, buffer_size1 , 0, (struct sockaddr *)&server_address, sizeof(server_address));
	}
	
	printf("Client: Last Packet sent \n");

	// End of ftp

}



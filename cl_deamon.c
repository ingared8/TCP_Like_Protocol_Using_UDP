#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>

// This program is to create a file transfer protocol server

#define MSS 1000
#define HEADER_SIZE 16

typedef struct 
	{
	struct sockaddr_in header;
	char body[MSS-HEADER_SIZE];
	} troll_message;
	
typedef struct
{
	int free_size;
}ack_buffer;

typedef struct 
	{
		struct sockaddr_in header;
		char body[MSS-header_size];		
	} troll_message;

typedef struct 
	{
		struct sockaddr_in header;
		int packet_no;	
	} troll_ack;

typedef struct 
	{
		int packet_no;	
	} packet_ack;


int main(int argc, char *argv[] )
{
char * troll_host;

int PORT_NUM_OUT = 6000;
int PORT_NUM_OUT_TROLL = 7777;
int PORT_NUM_OUT_TIMER = 8888;
int PORT_NUM_OUT_TCPDS = 9999;
int PORT_NUM_IN = 5555;

if (argc > 1)
{
printf(" The TCPD starts running on %d \n", PORT_NUM);
printf(" The Port number at which it aims packets ( for TCPD or Troll) is %s\n",argv[1]);
PORT_NUM_OUT = atoi(argv[1]);
}

else
{
printf(" The arguments for portnumber and localhost are not passed  \n");
printf(" The TCPD client is running locally and port is %d\n",PORT_NUM);
}


// Variable declarations
struct sockaddr_in server_address, troll_address;
int new_client,client_length;
int pid;

// Socket creation
printf("TCPD client: Creating socket at Client to receive \n");
int server_socket;
server_socket = socket(AF_INET,SOCK_DGRAM,0);
if (server_socket < 0)
	{
	printf("TCPD Client: Socket error: %d\n",errno);
	exit(0);
	}

printf("TCPD Client: Socket succesfull\n");

server_address.sin_family = AF_INET;
server_address.sin_port = htons(PORT_NUM);
server_address.sin_addr.s_addr = htons(INADDR_ANY);

// Bind the socket
int bind_id = bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));
if (bind_id < 0)
{
printf("TCPD Client: Socket Bind Error: %s\n", strerror(errno));
exit(0);
}
printf("TCPD: Binding Server socket successful\n");


// Socket creation to send data to troll
printf("TCPD: Creating socket at Client to send \n");
int troll_socket;
troll_socket = socket(AF_INET,SOCK_DGRAM,0);
if (troll_socket < 0)
	{
	printf("TCPD troll: Socket error: %d\n",errno);
	exit(0);
	}
printf("TCPD troll: Socket succesfull\n");

struct hostent * troll_addr;
troll_addr = gethostbyname("localhost");
if (troll_addr == NULL)
{
        printf("TCPD Client: Could not get Troll hostanme: %d\n",errno);
}

// Multiple ways to assign server adress 
memcpy(&troll_address.sin_addr, troll_addr->h_addr_list[0],troll_addr->h_length);
troll_address.sin_family = AF_INET;
troll_address.sin_port = htons(PORT_NUM_OUT);

char recvbuffer[MSS];
first_message * first_msg = malloc(sizeof(first_message));
int buf_len = MSS;
int s_name_len = sizeof(server_address);
int buf_count = 0;
int sent_count = 0;

//first_message 

int mm = recvfrom(server_socket, (char *)recvbuffer, MSS, 0, (struct sockaddr *)&server_address,&s_name_len);
if ( mm < 0)
{
printf("TCPD Client : First Message receive failed \n");
}
printf("TCPD Client: First message received \n");

char * server_ip = malloc(sizeof(char)*100);

int ack = sendto(troll_socket, recvbuffer, buf_len, 0, (struct sockaddr *)&troll_address, sizeof(troll_address));
if ( ack < 0)
{
printf("TCPD troll: Failed --> The message sent failed , buf_count %d\n", sent_count);
}
printf("TCPD: The message sent is buf_count %d:\n",sent_count);

	
while ( 1==1)
{

// Receive from client 
mm = recvfrom(server_socket, (char *)first_msg, sizeof(first_message), 0, (struct sockaddr *)&server_address,&s_name_len);
if ( mm < 0)
{
printf("TCPD: Message receive failed \n");
}
printf("TCPD: The message received is buf_count %d:\n",mm);

buf_count += 1;
printf("TCPD: The message received is buf_count %d:\n",buf_count);
//first_msg = (first_message *)first_msg;
printf(" The first message is %d \n", first_msg->file_size);
printf(" The first message is %d \n", first_msg->p1);
printf(" The first message is %d \n", first_msg->p2);
printf(" The first message is %d \n", first_msg->p3);
printf(" The first message is %s \n", first_msg->filename);

// Send to Troll
int ack = sendto(troll_socket, recvbuffer, buf_len, 0, (struct sockaddr *)&troll_address, sizeof(troll_address));
if ( ack < 0)
{
printf("TCPD troll: Failed --> The message sent failed , buf_count %d\n", sent_count);
}
sent_count += 1;
printf("TCPD: The message sent is buf_count %d:\n",sent_count);

}

}

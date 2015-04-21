#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include "genlib.h"

main(int argc, char * argv[] )
{	
	int PORT_NUM_RECV = 1234;
	int PORT_NUM_SEND = 4321;

	if (argc >= 1)
		{
			printf(" Server: The Server is ( listening at) Port number  %s\n",argv[1]);
		}
	else
		{
			printf(" Server: The Server is ( listening at) Port number  %s\n",PORT_NUM_RECV);
		}
		
	char * server_host;
	char * dest_host; 
	char * filename;
	
	// Sockets Declarations
	printf("Server: Creating socket at client end\n");

	int server_socket_send = SOCKET();
	check_socket(server_socket_send,"Client Sending socket ","Client");
	
	int server_socket_listen = SOCKET();
	check_socket(server_socket_listen,"Client Receiving socket ","Client");
	
	// Get the adress of the TCPD_server which is running on the same machine as ftps.c //
	struct sockaddr_in tcpd_server_adress_send = get_sockaddr_send("localhost", PORT_NUM_SEND);
	struct sockaddr_in tcpd_server_adress_recv = get_sockaddr_recv(PORT_NUM_RECV);
	int tcpd_server_adress_recv_len = sizeof(tcpd_server_adress_recv);
	
	// Bind the socket
	int bind_id = BIND(server_socket_listen, tcpd_server_adress_recv);
	check_bind(bind_id, "TCPD Server receiving socket", "Server");
	
	// Varibales used 
	int filesize;
	int remaining;
	int packet_count = 0;
	int buffer_size = MSS;
	
// Socket creation
printf("Server: Creating socket at server end\n");
int server_socket;
server_socket = socket(AF_INET,SOCK_DGRAM,0);
if (server_socket < 0)
        {
        printf("Server: Socket error: %d\n",errno);
        exit(0);
        }
printf("Server: Socket succesfull\n");

server_address.sin_family = AF_INET;
server_address.sin_port = htons(PORT_NUM);
server_address.sin_addr.s_addr = htons(INADDR_ANY);

// Bind the socket

int bind_id = bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));

if (bind_id < 0)
	{
	printf("Server: Socket Bind Error: %d\n", errno);
	exit(0);
	}
printf("Server: Binding Server socket successful\n");

char recvbuffer[MSS];
int buf_len = MSS;
int s_name_len;
char filename2[20];
char filename[20];
char * n1 = "N";
char * n2 = "F";
filename2[0] = *(n1);
filename2[1] = *(n2);
filename2[2] = '\0';

int header_size = 16;
struct sockaddr_in header1;
struct 
{
struct sockaddr_in header;
char body[MSS-header_size];
} tr_msg;


// Recv filename and filesize 
char msg[buf_len];
s_name_len = sizeof(server_address);
int mm = recvfrom(server_socket, recvbuffer, buf_len, 0, (struct sockaddr *)&server_address,&s_name_len);

if ( mm < 0)
{
printf("Server: Fisrt Message receive failed \n");
}
printf("Server: Received first message\n");


char filesz_str[4];
memcpy(filesz_str,recvbuffer+16,4);
int filesize = *(int *)(filesz_str);
memcpy(filename,recvbuffer+20,20);
strcat(filename2,filename);

FILE * fp;
fp = fopen("gshdd","wb");
int file_remaining = filesize;
printf(" The name of file written is %s \n",filename2);
printf(" FIle size is %d\n",filesize);
printf(" File name2 is %s\n", filename2);
printf(" Original File name is %s\n", filename);

int buffer_size = MSS;
int buf_count = 0;
int m1;
int pay_load_buffer = MSS - header_size;

while (file_remaining/pay_load_buffer >= 1)
	{
	printf("Server: The message received is %d\n", file_remaining );
	mm = recvfrom(server_socket, recvbuffer, buf_len, 0, (struct sockaddr *)&server_address,&s_name_len);
	
	if ( mm < 0)
	{
	printf("Server: Message receive failed for buf count : %d \n", buf_count);
	}

	m1 = fwrite(recvbuffer+ header_size, sizeof(char),pay_load_buffer,fp);
	
	if ( m1 < 0)
	{
	printf("Server: Write to the file failed\n");
	} 

	printf("Server: Message Write to file succeeded -- %d\n", buf_count);

	buf_count++;
	
	printf("Server: data Message has arrived for buf_count, %d\n",buf_count);
	file_remaining -= (pay_load_buffer); 

}

	int buffer_size1 = file_remaining;

	mm = recvfrom(server_socket, recvbuffer, buffer_size1+20, 0, (struct sockaddr *)&server_address,&s_name_len);
	
	while (mm < 0)
	{
	mm = recvfrom(server_socket, recvbuffer, buffer_size, 0, (struct sockaddr *)&server_address,&s_name_len);
	printf("Server: Message receive failed \n");
	}
	
	m1 = fwrite(recvbuffer+16, sizeof(char),buffer_size1,fp);
	if ( m1 < 0)
	{
	printf("Server: Write to the file failed\n");
	} 

	buf_count++;
	file_remaining -= buffer_size1;  
	
	if(file_remaining == 0)
	{
	printf(" Server: File transfer complete\n");
	}

	int df = fclose(fp);

	printf("Completed . Good bye\n");
}



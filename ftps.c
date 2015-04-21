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
	int PORT_NUM_RECV = 7777;
	int PORT_NUM_SEND = 7700;

	if (argc >= 1)
		{
			printf(" Server: The Server is ( listening at) Port number  %s\n",argv[1]);
		}
	else
		{
			printf(" Server: The Server is ( listening at) Port number  %d\n",PORT_NUM_RECV);
		}
		
	// Sockets Declarations
	printf("Server: Creating socket at client end\n");

	int server_socket_send = SOCKET();
	check_socket(server_socket_send,"Client Sending socket ","Server");
	
	int server_socket_listen = SOCKET();
	check_socket(server_socket_listen,"Client Receiving socket ","Server");
	

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
	troll_message tr_msg;
	first_message * first_msg = malloc(sizeof(first_message));
	char * recvbuffer = malloc(sizeof(send_buffer));
	int buffer_size = sizeof(send_buffer);
	
	
	// Receive the first message and unblock the Accept call	
	int mm = ACCEPT(server_socket_listen, (char *)first_msg, sizeof(first_message),tcpd_server_adress_recv, tcpd_server_adress_recv_len );
	
	printf("Server :packet_count %d \n",packet_count);	
	printf("Server: The first message, file_size is %d \n", first_msg->file_size);
	printf(" The first message, port no is %d \n", first_msg->server_port);	
	printf(" The first message, server_ip %s \n", first_msg->server_ip);
	
	int file_remaining = first_msg->file_size;
	FILE * fp;
	fp = fopen(	strcat("new", first_msg->filename),"wb");

	while (file_remaining/buffer_size >= 1)
	{
		
		mm = RECV(server_socket_listen, recvbuffer, buffer_size, tcpd_server_adress_recv, tcpd_server_adress_recv_len);
		while ( mm < 0)
		{
		mm = RECV(server_socket_listen, recvbuffer,  buffer_size, tcpd_server_adress_recv, tcpd_server_adress_recv_len);
		printf("Server: Message receive failed for buf count : %d \n", packet_count);
		}
		printf("Server: Message RECV to server succeeded -- %d\n", packet_count);
		
		mm = fwrite(recvbuffer, sizeof(char), buffer_size,fp);
		if ( mm < 0)
		{
		printf("Server: Write to the file failed\n");
		mm = fwrite(recvbuffer, sizeof(char), buffer_size,fp);
		} 
		printf("Server: Message Write to file succeeded -- %d\n", packet_count);

		packet_count++;
		printf("Server: data Message has arrived for buf_count, %d\n",packet_count);
		file_remaining -= buffer_size; 
			
	}

	int buffer_size1 = file_remaining;
	mm = RECV(server_socket_listen, recvbuffer, buffer_size1, tcpd_server_adress_recv, tcpd_server_adress_recv_len);
	while (mm < 0)
	{
	printf("Server: Message receive failed for Last message \n");
	mm = RECV(server_socket_listen, recvbuffer, buffer_size1, tcpd_server_adress_recv, tcpd_server_adress_recv_len);
	}
	
	mm = fwrite(recvbuffer,sizeof(char),buffer_size1,fp);
	if ( mm < 0)
	{
	printf("Server: Write to the file failed in the Last message\n");
	} 

	packet_count++;
	file_remaining -= buffer_size1;  
	
	if(file_remaining == 0)
	{
	printf(" Server: File transfer complete\n");
	}

	int df = fclose(fp);
	
	free(recvbuffer);
	free(first_msg);
	printf("Completed . Good bye\n");
	
}


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

	int PORT_NUM_RECV = atoi(argv[1]);
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
	ack_buffer * ackbuffer = malloc(sizeof(ack_buffer));
	char * recvbuffer = malloc(sizeof(send_buffer));
	int buffer_size = sizeof(send_buffer);
		
	// Receive the first message and unblock the Accept call ( Connection establishment )
	int mm = ACCEPT(server_socket_listen, (char *)first_msg, sizeof(first_message),tcpd_server_adress_recv, tcpd_server_adress_recv_len );
	printf("Server: Connection Establishment successful\n");
	printf("Server :packet_count %d \n",packet_count);	
	printf("Server: The first message, file_size is %d \n", first_msg->file_size);
	printf(" The first message, port no is %d \n", first_msg->server_port);	
	printf(" The first message, server_ip %s \n", first_msg->server_ip);
	
	int file_remaining = first_msg->file_size;
	FILE * fp;
	char  newfilename[30];
	char  * extra = "new_";
	memcpy(newfilename,extra,4);
	memcpy(newfilename+4,first_msg->filename, 20); 
	fp = fopen(newfilename,"wb");
	printf("Server: The file is being written to %s \n",newfilename); 
	buffer_size = sizeof(send_buffer);

	while (file_remaining/buffer_size >= 1)
	{
		
		// Recv files 
		mm = RECV(server_socket_listen, recvbuffer, buffer_size, tcpd_server_adress_recv, tcpd_server_adress_recv_len);
		while ( mm < 0)
		{
		mm = RECV(server_socket_listen, recvbuffer,  buffer_size, tcpd_server_adress_recv, tcpd_server_adress_recv_len);
		printf("Server: Message receive failed for buf count : %d \n", packet_count);
		}
		printf("Server: Message Recievd from TCPD Server with packet count as -- %d\n", packet_count);
		
		// Send acknowledgement for the next packet with file size
		ackbuffer->free_size = min2(buffer_size, file_remaining);
		
		mm = SEND(server_socket_send,(char *)ackbuffer, sizeof(ack_buffer), tcpd_server_adress_send);
		while (mm < 0)
		{
			printf("Server: Acknowledgement for the tcpd (bytes received is failed \n");
			mm = SEND(server_socket_send,(char *)ackbuffer, sizeof(ack_buffer), tcpd_server_adress_send);
		}
		printf("Server: Requesting data from tcpd server of buffer size %d bytes \n", buffer_size);
		
		
		mm = fwrite(recvbuffer, sizeof(char), buffer_size,fp);
		if ( mm < 0)
		{
		printf("Server: Write to the file failed\n");
		mm = fwrite(recvbuffer, sizeof(char), buffer_size,fp);
		} 
		//printf("Server: Message Write to file succeeded -- %d\n", packet_count);
		
		packet_count++;
		file_remaining -= buffer_size; 
		printf("Server: Remaining file_size is, %d\n",file_remaining);
	}

	int buffer_size1 = file_remaining;
	
	mm = RECV(server_socket_listen, recvbuffer, buffer_size1, tcpd_server_adress_recv, tcpd_server_adress_recv_len);
	while (mm < 0)
	{
	printf("Server: Message receive failed for Last message \n");
	mm = RECV(server_socket_listen, recvbuffer, buffer_size1, tcpd_server_adress_recv, tcpd_server_adress_recv_len);
	}
	
	ackbuffer->free_size = buffer_size1;
	
	mm = SEND(server_socket_send,(char *)ackbuffer, sizeof(ack_buffer), tcpd_server_adress_send);
	while ( mm < 0)
	{
	mm = SEND(server_socket_send,(char *)ackbuffer, sizeof(ack_buffer), tcpd_server_adress_send);
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
	
	// Close the file
	int df = fclose(fp);
	
	// Close the sockets
	printf("Server: Closing the sockets\n");
	close(server_socket_listen);
	close(server_socket_send);
	
	//  De allocate the buffers
	free(recvbuffer);
	free(first_msg);
	printf("Server: Completed . Good bye\n");
}


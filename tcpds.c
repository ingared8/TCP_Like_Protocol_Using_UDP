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
#include "genlib.h"
#include "CB_server.h"
#include "crc.c"


// This program is to create a file transfer protocol server

int main(int argc, char *argv[] )
{
int PORT_NUM_IN_SERVER = 7700;
int PORT_NUM_OUT_SERVER = 7777;
int PORT_NUM_IN_TROLL = 6600;
int PORT_NUM_OUT_TROLL = 6666;
int PORT_NUM_OUT_TCPDC = 3300;
int packet_count = 0;

printf("TCPD Client: The TCPD Client started \n");

// Socket Creations
int tcpd_server_socket_send = SOCKET();
check_socket(tcpd_server_socket_send,"TCPD - Client Sending socket ","TCPD server");

int tcpd_server_socket_listen = SOCKET();
check_socket(tcpd_server_socket_listen,"TCPD Client Receiving socket ","TCPD server");

int tcpd_troll_socket_send = SOCKET();
check_socket(tcpd_troll_socket_send,"TCPD - Troll Sending socket ","TCPD server");

int tcpd_tcpdc_socket_listen = SOCKET();
check_socket(tcpd_tcpdc_socket_listen,"TCPD -TCPDS Receiving socket ","TCPD server");

// Get the adress of the TCPD_server which is running on the same machine as ftps.c //
struct sockaddr_in tcpd_server_adress_send = get_sockaddr_send("localhost", PORT_NUM_OUT_SERVER);
struct sockaddr_in tcpd_server_adress_recv = get_sockaddr_recv(PORT_NUM_IN_SERVER);
int tcpd_server_adress_recv_len = sizeof(tcpd_server_adress_recv);

struct sockaddr_in tcpd_troll_adress_send = get_sockaddr_send("localhost", PORT_NUM_OUT_TROLL);
struct sockaddr_in tcpd_troll_adress_recv = get_sockaddr_recv(PORT_NUM_IN_TROLL);
int tcpd_troll_adress_recv_len = sizeof(tcpd_troll_adress_recv);

// Bind the socket
int bind_id = BIND(tcpd_server_socket_listen, tcpd_server_adress_recv);
check_bind(bind_id, "TCPD-Server receiving socket", "TCPD Server");

int bind_id2 = BIND(tcpd_tcpdc_socket_listen, tcpd_troll_adress_recv);
check_bind(bind_id2, "TCPD-Troll receiving socket", "TCPD Server");


while ( 1==1 )
{

// Start listening for packets 
ack_buffer * ackbuffer = malloc(sizeof(ack_buffer));
ack2_buffer * ack2buffer = malloc(sizeof(ack2_buffer));

//send_buffer * sendbuffer = malloc(sizeof(send_buffer));
first_message * first_msg = malloc(sizeof(first_message));
char * sendbuffer = malloc(sizeof(send_buffer));
troll_message tr_msg;		// Recv from TCPDC
troll_message tr_msg2; 	   // Send to TCPDC

// Create a cyclic buffer and create the read and write pointers
circular_buffer_s * cb = malloc(sizeof(circular_buffer_s));
ring_buffer_init_s(cb);

// Wait for connection from Client
printf("TCPD Server: Waiting for Connection \n");

// First packet 
int mm =  RECV(tcpd_tcpdc_socket_listen, (char *)&tr_msg, sizeof(troll_message),tcpd_troll_adress_recv, tcpd_troll_adress_recv_len);
printf("TCPD Server: Forwarding request to server \n");
memcpy(first_msg,&tr_msg.body,sizeof(first_message));
mm = SEND(tcpd_server_socket_send,(char *)first_msg, sizeof(first_message),tcpd_server_adress_send); 
printf("TCPD Server: Connection successful \n");

// Know the IP adress and port number of server
printf("TCPD Server: File_size is %d \n", first_msg->file_size);
printf("TCPD Server: Port no is %d \n", first_msg->server_port);	
printf("TCPD Server: Server_ip %s \n", first_msg->server_ip);

// Modify the port of the client adress
tr_msg2 = tr_msg;
tr_msg2.header.sin_port = htons(PORT_NUM_OUT_TCPDC);
ack2buffer->header = tr_msg2.header;
tcpd_server_adress_send.sin_port = htons(first_msg->server_port);

// Create the header file to add to messages to troll
mm = SEND(tcpd_troll_socket_send,(char *)&tr_msg2, sizeof(troll_message),tcpd_troll_adress_send); 

// Start listening for files and start writing in the circular buffer
int ack;
int rem;
int usd;
int seq = 0;
int file_size_sent_to_client = 0;
int remaining_file_size = first_msg->file_size;
int packet_count_server = 0;
int packet_count_troll = 0;
int buffer_size = sizeof(send_buffer);

// Params for Select
fd_set readset;
struct timeval tv;
printf("Fail1\n");

while(1==1)
{
	int retval;
	int max_sd;

	// Select the maximum of the existing clients 
	max_sd = max2(tcpd_server_socket_listen, tcpd_tcpdc_socket_listen) + 1;

	// Watch stdin (fd 0) to see when it has input.
	FD_ZERO(&readset);  
	FD_SET(tcpd_server_socket_listen, &readset);
	FD_SET(tcpd_tcpdc_socket_listen, &readset);

	// Wait up to five seconds. 
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	retval = select(max_sd, &readset, NULL, NULL, &tv);

	// Check for error
    if (retval == -1)
		{
			perror("select()");
		}
		
	// Check for messages from TCPD Client via troll
	if FD_ISSET(tcpd_tcpdc_socket_listen, &readset)
	{	
		printf("TCPD Server : Received packet from Client with packet_count  as %d \n",packet_count_troll);
		mm =  RECV(tcpd_tcpdc_socket_listen, (char *)&tr_msg, sizeof(troll_message),tcpd_troll_adress_recv, tcpd_troll_adress_recv_len);
		
		// Check for packet checksum
		
		//if (tr_msg.checksum ==  crcFast((char *)&tr_msg.body,sizeof(tr_msg.body)) )
		if ( 1==1)
		{
			int rem = cb_push_data_s(cb, (char *)&tr_msg.body, sizeof(tr_msg.body), tr_msg.seq_no);
			// Send ack for ftpc/client with remaining size
			if (rem >= 0)
				{
					ack2buffer->seq_no = tr_msg.seq_no; 
					ack = SEND(tcpd_troll_socket_send, (char*)ack2buffer, sizeof(ack2_buffer),tcpd_troll_adress_send);
					printf(" TCPD Server: Ack sent for packet %d \n",packet_count_troll);
					packet_count_troll++;
				}
		}
		else
		{
			printf("TCPD Server: Packet garbled for packet no %d \n", tr_msg.seq_no);
		}	
	}
		
	usd = cb_pop_data_s(cb, (char *)sendbuffer, sizeof(send_buffer), packet_count_server);	
	if (usd >= 0)
	{
		ack = SEND(tcpd_server_socket_send,(char *)sendbuffer, sizeof(send_buffer), tcpd_server_adress_send);
		printf("TCPD Server : Sent data to Server for packet no %d with size  %d \n",packet_count_server , buffer_size);	
		mm = RECV(tcpd_server_socket_listen, (char *)ackbuffer, sizeof(ack_buffer),tcpd_server_adress_recv, tcpd_server_adress_recv_len);
		file_size_sent_to_client += ackbuffer->free_size;
		remaining_file_size -= ackbuffer->free_size;
		packet_count_server++;
		printf("TCPD Server: Remaining file size is %d\n",remaining_file_size);
	}
	
	// Closing function call-- send last packet and close the sockets 

	if ( file_size_sent_to_client >= first_msg->file_size)
	{
		printf("TCPD Server: Completed file transfer , every message is sent to server \n");
		break;
	}
	
	printf("------------TCPD Server-----------------\n");
	usleep(100);
	}
	
	// Clear all the buffers
	printf("TCPD Server: De allocating memory (free all) of all the memory buffers\n"); 
	free(cb);
	free(ackbuffer);
	free(ack2buffer);
	free(sendbuffer);
	
}

	// Shutdown started  and Closing the sockets
	close(tcpd_server_socket_listen);
	close(tcpd_server_socket_send);
	close(tcpd_troll_socket_send);
	close(tcpd_tcpdc_socket_listen);
	printf("TCPD Server: The sockets are closed\n");
	printf("TCPD Server: Shutdown successful\n");

}

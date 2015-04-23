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
#include "circular_buffer.h"

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

// Create a cyclic buffer and create the read and write pointers
circular_buffer * cb = malloc(sizeof(circular_buffer));
ring_buffer_init(cb, RING_BUFFER_SIZE);

// Start listening for packets 
ack_buffer * ackbuffer = malloc(sizeof(ack_buffer));
ack2_buffer * ack2buffer = malloc(sizeof(ack2_buffer));
//send_buffer * sendbuffer = malloc(sizeof(send_buffer));
first_message * first_msg = malloc(sizeof(first_message));
char * sendbuffer = malloc(sizeof(send_buffer));
int buffer_size = 1;
troll_message tr_msg;		// Recv from TCPDC
troll_message tr_msg2; 	   // Send to TCPDC

// First packet 
printf("TCPD Server: Read the first message\n");
int mm =  RECV(tcpd_tcpdc_socket_listen, (char *)&tr_msg, sizeof(troll_message),tcpd_troll_adress_recv, tcpd_troll_adress_recv_len);
printf("TCPD Server :packet_count %d \n",packet_count);
memcpy(first_msg,&tr_msg.body,sizeof(first_message));
mm = SEND(tcpd_server_socket_send,(char *)first_msg, sizeof(first_message),tcpd_server_adress_send); 

// Know the IP adress and port number of server
printf(" The first message, file_size is %d \n", first_msg->file_size);
printf(" The first message, port no is %d \n", first_msg->server_port);	
printf(" The first message, server_ip %s \n", first_msg->server_ip);

// Modify the port of the client adress
//tr_msg2.header = get_sockaddr_send_troll(PORT_NUM_OUT_TCPDC);
tr_msg2 = tr_msg;
tr_msg2.header.sin_port = htons(PORT_NUM_OUT_TCPDC);
ack2buffer->header = tr_msg2.header;

// Create the header file to add to messages to troll
mm = SEND(tcpd_troll_socket_send,(char *)&tr_msg2, sizeof(troll_message),tcpd_troll_adress_send); 

// Start listening for files and start writing in the circular buffer
int ack;
int rem;
int usd;
int seq = 0;

// Params for Select
fd_set readset;
struct timeval tv;
int retval;
int max_sd;

while(1==1)
{
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
	
	else 
	{
		
	mm =  RECV(tcpd_tcpdc_socket_listen, (char *)&tr_msg, sizeof(troll_message),tcpd_troll_adress_recv, tcpd_troll_adress_recv_len);
	printf("TCPD Server : Received packet from Client with packet_count  as %d \n",packet_count);

	int rem = cb_push_data(cb, (char *)&tr_msg.body, sizeof(tr_msg.body));
	
	// Send ack for ftpc/client with remaining size
	if (rem > 0)
		{
			ack2buffer->seq_no = tr_msg.seq_no; 
			ack = SEND(tcpd_troll_socket_send, (char*)ack2buffer, sizeof(ack2_buffer),tcpd_troll_adress_send);
			printf(" TCPD Server: Ack sent for packet %d \n",packet_count);
		}
		
	// Prepare data from buffer for troll to client
	int usd = cb_pop_data(cb, (char *)sendbuffer, sizeof(send_buffer));
	printf("The results is %d \n", usd);
	
	if (usd > 0)
		{
		ack = SEND(tcpd_server_socket_send,(char *)sendbuffer, sizeof(send_buffer), tcpd_server_adress_send);	
		}

	packet_count += 1;	
	//usleep(1000);
	
	}
}

}

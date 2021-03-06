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
#include "Jacobson.c"
#include "crc.c"

// This program is to create a file transfer protocol server

int main(int argc, char *argv[] )
{
		 
int PORT_NUM_IN_CLIENT = 2200;
int PORT_NUM_OUT_CLIENT = 2222;
int PORT_NUM_IN_TROLL = 3300;
int PORT_NUM_OUT_TROLL = 3333;
int PORT_NUM_IN_TIMER = 4400;
int PORT_NUM_OUT_TIMER = 4444;
int PORT_NUM_OUT_TCPDS = 6600;
int packet_count = 0;

printf("TCPD Client: The TCPD Client started \n");

// Socket Creations
int tcpd_client_socket_send = SOCKET();
check_socket(tcpd_client_socket_send,"TCPD - Client Sending socket ","TCPD Client");

int tcpd_client_socket_listen = SOCKET();
check_socket(tcpd_client_socket_listen,"TCPD Client Receiving socket ","TCPD Client");

int tcpd_troll_socket_send = SOCKET();
check_socket(tcpd_troll_socket_send,"TCPD - Troll Sending socket ","TCPD Client");

int tcpd_tcpds_socket_listen = SOCKET();
check_socket(tcpd_tcpds_socket_listen,"TCPD -TCPDS Receiving socket ","TCPD Client");

int tcpd_timer_socket_send = SOCKET();
check_socket(tcpd_timer_socket_send,"TCPD - Timer Sending socket ","TCPD Client");

int tcpd_timer_socket_listen = SOCKET();
check_socket(tcpd_timer_socket_listen,"TCPD -Timer Receiving socket ","TCPD Client");


// Get the adress of the TCPD_client which is running on the same machine as ftpc.c //
struct sockaddr_in tcpd_client_adress_send = get_sockaddr_send("localhost", PORT_NUM_OUT_CLIENT);
struct sockaddr_in tcpd_client_adress_recv = get_sockaddr_recv(PORT_NUM_IN_CLIENT);
int tcpd_client_adress_recv_len = sizeof(tcpd_client_adress_recv);

struct sockaddr_in tcpd_troll_adress_send = get_sockaddr_send("localhost", PORT_NUM_OUT_TROLL);
struct sockaddr_in tcpd_tcpds_adress_recv = get_sockaddr_recv(PORT_NUM_IN_TROLL);
int tcpd_tcpds_adress_recv_len = sizeof(tcpd_tcpds_adress_recv);

struct sockaddr_in tcpd_timer_adress_send = get_sockaddr_send("localhost", PORT_NUM_OUT_TIMER);
struct sockaddr_in tcpd_timer_adress_recv = get_sockaddr_recv(PORT_NUM_IN_TIMER);
int tcpd_timer_adress_recv_len = sizeof(tcpd_timer_adress_recv);


// Bind the socket
int bind_id = BIND(tcpd_client_socket_listen, tcpd_client_adress_recv);
check_bind(bind_id, "Client/TCPD receiving socket", "TCPD client");

int bind_id2 = BIND(tcpd_tcpds_socket_listen, tcpd_tcpds_adress_recv);
check_bind(bind_id2, "TCPD Client/ TCPDS server receiving socket", "TCPD client");

int bind_id3 = BIND(tcpd_timer_socket_listen, tcpd_timer_adress_recv);
check_bind(bind_id3, "TCPD Client/ Timer receiving socket", "TCPD client");

// Params for messages and formats
first_message * first_msg = malloc(sizeof(first_message));
troll_message tr_msg;  	// Send to TCPDS
troll_message tr_msg2; 	// Recv from TCPDS

while ( 1==1)
{

// Waiting for message from Client 
printf("TCPD Client: Waiting for Connection\n");
	
// Receive First packet for connect establishment
int mm =  RECV(tcpd_client_socket_listen, (char *)first_msg, sizeof(first_message),tcpd_client_adress_recv, tcpd_client_adress_recv_len);
printf("TCPD Client: Establishing Connection with server\n");

// Know the IP adress and port number of server
printf(" TCPD Client: File_size is %d \n", first_msg->file_size);
printf(" TCPD Client: port no is %d \n", first_msg->server_port);	
printf(" TCPD Client: File is to be transferred to server_ip %s \n", first_msg->server_ip);	

// Get the adress of the first message of 
tr_msg.header = get_sockaddr_send_troll(first_msg->server_ip, PORT_NUM_OUT_TCPDS);

// Create the header file to add to messages to troll
//memcpy(first_msg,&tr_msg2.body,sizeof(first_message));
memset((void*)tr_msg.body,'\0',sizeof(tr_msg.body));
memcpy(&tr_msg.body,first_msg,sizeof(first_message));

// Send first message to TCPDS via troll
mm = SEND(tcpd_troll_socket_send, (char *)&tr_msg,sizeof(troll_message),tcpd_troll_adress_send);
printf(" Message sent to the TCPD Server \n");
mm = RECV(tcpd_tcpds_socket_listen,(char *)&tr_msg2, sizeof(troll_message),tcpd_tcpds_adress_recv,tcpd_tcpds_adress_recv_len);

printf("TCPD Client: Establied Connection with server and Starting file transfer\n");

// Create a cyclic buffer and create the read and write pointers
circular_buffer * cb = malloc(sizeof(circular_buffer));
ring_buffer_init(cb);
const int complete_file_size = first_msg->file_size;
//memcpy(&complete_file_size,first_msg->file_size, sizeof(int));

// Initialise the CRC
crcInit();
printf("TCPD Client: Initializing the circular buffer\n");

// Start listening for packets 
ack_buffer * ackbuffer = malloc(sizeof(ack_buffer));
ack2_buffer * ack2buffer = malloc(sizeof(ack2_buffer));
send_buffer * recvbuffer = malloc(sizeof(send_buffer));

// Timer messages which will get updated
Timer_message timer_msg_recv;		// Message received by client for retransmission
Timer_message timer_msg_send;		// Informing about the packet sent to server
Timer_message timer_msg_ack;		// Informing acknowledgement of a packet number to timer

// Setting Default values for the two timer messages
timer_msg_send.port = 1234;
timer_msg_send.type = 's';
timer_msg_send.seq = 20;
timer_msg_send.time = 20;

timer_msg_recv.port = 1234;
timer_msg_recv.type = 'c';
timer_msg_recv.seq = 20;

// Settings for timestamp values
struct timeval sent_time;
struct timeval ack_time;

// RTT and RTPO values
double rtt;
int rto;

// Start listening for files and start writing in the circular buffer
int ack;
int rem;
int usd;
packet_count = 0;
int packet_count_client = 0;
int ack_no = 1;
int file_size_acknowleged = 0;

// Params for using select	
fd_set rfds;
fd_set readset;
struct timeval tv;
int retval;
int max_sd;

while(1==1)
{

// Select the maximum of the existing clients 
max_sd = max_fun(tcpd_client_socket_listen, tcpd_timer_socket_listen, tcpd_tcpds_socket_listen) + 1;

// Watch stdin (fd 0) to see when it has input.
FD_ZERO(&readset);  
FD_SET(tcpd_client_socket_listen, &readset);
FD_SET(tcpd_tcpds_socket_listen, &readset);
FD_SET(tcpd_timer_socket_listen, &readset);

// Wait up to five seconds. 
tv.tv_sec = 5;
tv.tv_usec = 0;
retval = select(max_sd, &readset, NULL, NULL, &tv);

// Don’t rely on the value of tv now! */

	// Check for error
    if (retval == -1)
		{
			perror("select()");
		}
        
	// Check for messages from Client and write the data to the wrap around buffer
    if FD_ISSET(tcpd_client_socket_listen, &readset)
    {

		mm =  RECV(tcpd_client_socket_listen, (char *)recvbuffer, sizeof(send_buffer),tcpd_client_adress_recv, tcpd_client_adress_recv_len);
		printf("TCPD Client :From Client received packet no %d \n",packet_count_client);
		
		rem = cb_push_data(cb, (char *)recvbuffer, sizeof(send_buffer));
		if (rem >= 0)
		{
			packet_count_client++;
		}
		
		// Send ack for ftpc/client with remaining size
		ackbuffer->free_size = rem; 
		ack = SEND(tcpd_client_socket_send, (char*)ackbuffer, sizeof(ack_buffer),tcpd_client_adress_send);
		
	}
	
	//mm = SEND2D(tcpd_troll_socket_send,tcpd_tcpds_socket_listen,(char *)&tr_msg ,sizeof(troll_message), tcpd_troll_adress_send, tcpd_tcpds_adress_recv);

	// Prepare data from buffer for troll 
	if  FD_ISSET(tcpd_tcpds_socket_listen, &readset)
	{
		mm = RECV(tcpd_tcpds_socket_listen, (char *)ack2buffer, sizeof(ack2_buffer) ,tcpd_tcpds_adress_recv , tcpd_tcpds_adress_recv_len);
		printf("TCPD Client: Received ack for %d \n",ack2buffer->seq_no);
		
		ack_data(cb, (int)ack2buffer->seq_no, sizeof(send_buffer));
		
		gettimeofday(&ack_time,NULL);
		sent_time = ack2buffer->time;
		
		// Update RTT and RTO
		rtt = calc_rtt(ack_time, sent_time);

		if (ack2buffer->seq_no < 1)
			{ initialize_srtt_rttvar(rtt); }
	
		rto = max2(computeRTO(),5);
		
		printf("RTO computed is  %d\n", rto);
		// Send message to timer about acknowlegdement of a packet number
		timer_msg_ack.seq = (long)ack2buffer->seq_no; 
		timer_msg_ack.type = 'c';
		mm = SEND(tcpd_timer_socket_send,(char *)&timer_msg_ack, sizeof(Timer_message), tcpd_timer_adress_send); 
		ack_no++;
		file_size_acknowleged += sizeof(send_buffer);		
	}
	
	// Messages from Timer tcpd_timer_socket_listen
	if FD_ISSET(tcpd_timer_socket_listen, &readset)
	{
		mm =  RECV(tcpd_timer_socket_listen, (char *)&timer_msg_recv, sizeof(Timer_message) ,tcpd_timer_adress_recv , tcpd_timer_adress_recv_len);
		printf("TCPD Client: Received message for Retransmission for sequence no -- %lu \n",timer_msg_recv.seq); 
		
		// Retransmit the message
		get_data_index(cb,(char *)&tr_msg.body, (int)timer_msg_recv.seq, sizeof(send_buffer));		
		tr_msg.seq_no = (int)timer_msg_recv.seq;
		gettimeofday(&sent_time,NULL);
		tr_msg.time = sent_time;
		tr_msg.checksum =  crcFast((char *)&tr_msg.body,sizeof(tr_msg.body));
		
		mm = SEND(tcpd_troll_socket_send,(char *)&tr_msg ,sizeof(troll_message), tcpd_troll_adress_send);
		printf("TCPD Client: Retransmission --> Sent packet for  %d \n",packet_count); 
		
		// Send timer message 
		timer_msg_send.seq = timer_msg_recv.seq;
		timer_msg_ack.time = rto;
		timer_msg_ack.type = 's';
		mm = SEND(tcpd_timer_socket_send,(char *)&timer_msg_send, sizeof(Timer_message), tcpd_timer_adress_send);
		printf("TCPD Client: Retransmission --> Sent timer message for  %lu \n",timer_msg_send.seq); 
		
	}
	
	usd = cb_pop_data(cb, (char *)&tr_msg.body, sizeof(tr_msg.body));
	printf( "TCPD Client: Collecting data from buffer to be sent to server, available buffer is	%d\n",usd);
	if (usd >= 0)
		{
		tr_msg.seq_no = packet_count;
		gettimeofday(&sent_time,NULL);
		tr_msg.time = sent_time;
		memcpy(&tr_msg.body, recvbuffer, sizeof(tr_msg.body));
		//tr_msg.checksum =  crcFast((char *)&tr_msg.body,sizeof(tr_msg.body));
		
		mm = SEND(tcpd_troll_socket_send,(char *)&tr_msg ,sizeof(troll_message), tcpd_troll_adress_send);
		printf("TCPD Client: Sent packet for  %d \n",packet_count); 
		timer_msg_send.seq = (long)packet_count;
		timer_msg_ack.time = rto;
		timer_msg_ack.type = 's';
		mm = SEND(tcpd_timer_socket_send,(char *)&timer_msg_send, sizeof(Timer_message), tcpd_timer_adress_send);
				
		printf("TCPD Client: Sent timer message for  %lu \n",timer_msg_send.seq); 
		packet_count++;
		}
	
	if ( file_size_acknowleged >= complete_file_size)
	{
		printf("TCPD Client: acked  %d , file size is %d \n",file_size_acknowleged, complete_file_size);
		printf("TCPD Client: Completed file transfer , every message is acknowleded \n");
		break;
	}
	
	printf("-------->>>>>>TCPD CLient<<<<<<<<<<----------\n");
	usleep(100);	
}
// Clear all buffers

printf("TCPD Client: De allocating memory (free all) of all the memory buffers\n"); 
free(cb);
free(ackbuffer);
free(ack2buffer);
free(recvbuffer);

}
// Shutdown Behaviour, needs tp close all the sockets

printf("TCPD Client: Closing the sockets\n");
close(tcpd_client_socket_send); 
close(tcpd_client_socket_listen);
close(tcpd_troll_socket_send);
close(tcpd_tcpds_socket_listen);
close(tcpd_timer_socket_send);
close(tcpd_timer_socket_listen);

printf("TCPD Client: Shutdown successful\n");

}

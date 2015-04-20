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

// This program is to create a file transfer protocol server

int main(int argc, char *argv[] )
{
		
int PORT_NUM_IN_CLIENT = 5500;
int PORT_NUM_OUT_CLIENT = 5555;
int PORT_NUM_IN_TROLL = 7700;
int PORT_NUM_OUT_TROLL = 7777;
int PORT_NUM_IN_TIMER = 8800;
int PORT_NUM_OUT_TIMER = 8888;
int PORT_NUM_OUT_TCPDS = 9999;
int packet_count = 0;

printf(" TCPD Client: The TCPD Client started \n");

// Socket Creations
int tcpd_client_socket_send = SOCKET();
check_socket(tcpd_client_socket_send,"TCPD - Client Sending socket ","TCPD");

int tcpd_client_socket_listen = SOCKET();
check_socket(tcpd_client_socket_listen,"Client Receiving socket ","Client");

// Get the adress of the TCPD_client which is running on the same machine as ftpc.c //
struct sockaddr_in tcpd_client_adress_send = get_sockaddr_send("localhost", PORT_NUM_OUT_CLIENT);
struct sockaddr_in tcpd_client_adress_recv = get_sockaddr_recv(PORT_NUM_IN_CLIENT);
int tcpd_client_adress_recv_len = sizeof(tcpd_client_adress_recv);
struct sockaddr_in tcpd_troll_adress_send = get_sockaddr_send("localhost", PORT_NUM_OUT_TROLL);
//struct sockaddr_in tcpd_header_adress = get_sockaddr_send("localhost", PORT_NUM_OUT_CLIENT);

// Bind the socket
int bind_id = BIND(tcpd_client_socket_listen, tcpd_client_adress_recv);
check_bind(bind_id, "Client/TCPD receiving socket", "client");

// Create a cyclic buffer and create the read and write pointers
char * W_BUFFER = malloc(sizeof(char)*Cyclci_buffer_size);
char * read_pointer = W_BUFFER;
char * write_pointer = W_BUFFER;

// Start listening for packets 

send_buffer * recvbuffer = malloc(sizeof(send_buffer));
ack_buffer * ackbuffer = malloc(sizeof(ack_buffer));
first_message * first_msg = malloc(sizeof(first_message));
	
// First packet 
printf("TCPD Client: Read the first message\n");
int mm =  RECV(tcpd_client_socket_listen, (char *)first_msg, sizeof(first_message),tcpd_client_adress_recv, tcpd_client_adress_recv_len);
printf("TCPD Client :packet_count %d \n",packet_count);

// Know the IP adress and port number of server
char server_ip[100]; 
memcpy(server_ip,first_msg->server_ip,100);
printf(" The first message, file_size is %d \n", first_msg->file_size);
printf(" The first message, port no is %d \n", first_msg->server_port);	
printf(" The first message, server_ip %s \n", first_msg->server_ip);	

}

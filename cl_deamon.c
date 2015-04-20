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

printf(" TCPD Client: The TCPD Client started \n");

// Socket Creations
int tcpd_client_socket_send = SOCKET();
check_socket(client_socket_send,"TCPD - Client Sending socket ","TCPD");

int tcpd_client_socket_listen = SOCKET();
check_socket(client_socket_listen,"Client Receiving socket ","Client");

// Variable declarations
struct sockaddr_in timer_adress, ftp_client_adress,troll_client_address;
// Get the adress of the TCPD_client which is running on the same machine as ftpc.c //
struct sockaddr_in tcpd_client_adress_send = get_sockaddr_send("localhost", PORT_NUM_SEND);
struct sockaddr_in tcpd_client_adress_recv = get_sockaddr_recv(PORT_NUM_RECV);
int tcpd_client_adress_recv_len = sizeof(tcpd_client_adress_recv);



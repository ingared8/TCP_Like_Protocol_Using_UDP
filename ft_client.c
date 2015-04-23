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
#include "genlib.h"

// This program is to transfer file in form of packets from client to TCPD Client

main(int argc, char * argv[] )
	{

	// Declarations 
	char * filename;
	int PORT_NUM_RECV;
	int PORT_NUM_SEND;
	int PORT_NUM_SERVER;
	PORT_NUM_RECV = 2222;
	PORT_NUM_SEND = 2200;

	// Check for input arguments
	if (argc > 1)
	{
		printf(" The remote, destination IP passed is %s\n",argv[1]);
		printf(" The Destination Port number passed  is %s\n",argv[2]);
		printf(" The file to be transferred is %s\n",argv[3]);	
		PORT_NUM_SERVER = atoi(argv[2]);
		filename = argv[3];
	}

	else
	{
		printf(" The arguments passed are in suffiecient \n");
		printf(" Please pass valid IP, port number and filetobe transferred\n");
		exit(0);
	}
	
	/* Socket creation */
	printf("Client: Creating socket at client end\n");

	int client_socket_send = SOCKET();
	check_socket(client_socket_send,"Client Sending socket ","Client");
	
	int client_socket_listen = SOCKET();
	check_socket(client_socket_listen,"Client Receiving socket ","Client");
	
	
	// Get the adress of the TCPD_client which is running on the same machine as ftpc.c //
	struct sockaddr_in tcpd_client_adress_send = get_sockaddr_send("localhost", PORT_NUM_SEND);
	struct sockaddr_in tcpd_client_adress_recv = get_sockaddr_recv(PORT_NUM_RECV);
	int tcpd_client_adress_recv_len = sizeof(tcpd_client_adress_recv);
	
	// Bind the socket
	int bind_id = BIND(client_socket_listen, tcpd_client_adress_recv);
	check_bind(bind_id, "Client/TCPD receiving socket", "client");
		
	// Open the file which needs to be sent to server
	FILE * fp;
	fp = fopen(filename,"rb");
	if (fp == NULL)
		{
		printf("Can't open the given file %s : Errono %d\n",filename,errno);
		exit(1);
		}
		
	// Variable useful
	int filesize = det_file_size(fp);
	int remaining = filesize;
	int packet_count = 0;
	
	send_buffer * sendbuffer = malloc(sizeof(send_buffer));
	ack_buffer * ackbuffer = malloc(sizeof(ack_buffer));	
	first_message * first_msg = malloc(sizeof(first_message));
	int buffer_size = sizeof(send_buffer);
	
	// send the first packet  
	create_first_message(first_msg, filesize ,filename, argv[1], PORT_NUM_SERVER);
	printf("Client: Sending the first message\n");
	
	int ack = SEND(client_socket_send,(char *)first_msg, sizeof(first_message), tcpd_client_adress_send); 
	while ( ack < 0)
		{
			printf("Client: Failed -- Data transfer failed for buf_count  %d\n", packet_count);
			int ack = SEND(client_socket_send,(char *)first_msg, sizeof(first_message), tcpd_client_adress_send);
		}
	printf(" Client :Packet_count %d \n",packet_count);
	
	time_t now;
	char buff[100];
	int len;
	int ack_tcpd;
	int ack_file_size;
	packet_count++;
	// Send the packets in a loop
	
	
	while ( remaining/buffer_size >= 1 )
	{
		printf("Client: Remaining file size  is %d,  packet count is %d \n", remaining, packet_count);
		
		len = fread(sendbuffer,sizeof(char),buffer_size,fp);
		now = time(0);
		strftime (buff, 100, "%Y-%m-%d %H:%M:%S.000", localtime (&now));
		
		printf("Client: %d \n",remaining- buffer_size);
	
		if ( len < 0)
		{
		printf("Client: Failed to read for buf_count %d\n", packet_count);
		}
		
		ack = SENDD(client_socket_send, client_socket_listen,(char *)sendbuffer, sizeof(send_buffer), tcpd_client_adress_send, tcpd_client_adress_recv);

		printf("Client :TCPD Message sent and ack message received for packet_count %d:\n",packet_count);

		remaining -= buffer_size;
		packet_count++;			
			
		//usleep(1000);
	}

	// Sending the last message 
	
	int buffer_size1 = remaining;
	len = fread(sendbuffer,sizeof(char),buffer_size1,fp);
	
	if ( len < 0)
	{
	printf(" Client: Failed to read for buf_count %d\n", packet_count);
	}
	
	ack = SENDD(client_socket_send, client_socket_listen, (char *)sendbuffer, sizeof(send_buffer), tcpd_client_adress_send, tcpd_client_adress_recv);
	
	printf("Client: Last Packet sent \n");
	printf("Client :TCPD ack message received is buf_count %d:\n",packet_count);
	
	// End of ftp

}

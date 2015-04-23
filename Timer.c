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
#include "linkedlist.h"


int main(int argc, char * argv[])
{

int PORT_NUM_SEND = 4400; 		// PORT at which timer sends messages  to client
int PORT_NUM_RECV = 4444;		// PORT at which timer recives messages from client

/* Create a linked list */

printf("Timer : Started and Linked List is initiated \n");

linked_list A = {.head = NULL, .tail = NULL , .len =0 };
linked_list * B = &A;
Timer_message * t_msg = malloc(sizeof(Timer_message));

// Socket Declarations 
int timer_client_send_socket = SOCKET();
check_socket(timer_client_send_socket, " Timer Client Send Socket", "Timer");

int timer_client_recv_socket = SOCKET();
check_socket(timer_client_recv_socket, " Timer Client recv Socket", "Timer");

// Adress declarations
struct sockaddr_in timer_client_adress_send = get_sockaddr_send("localhost", PORT_NUM_SEND);
struct sockaddr_in timer_client_adress_recv = get_sockaddr_recv(PORT_NUM_RECV);
int timer_client_adress_recv_len = sizeof(timer_client_adress_recv);

// Bind the socket
int bind_id = BIND(timer_client_recv_socket, timer_client_adress_recv);
check_bind(bind_id, "Client/TCPD receiving socket", "client");

// Listen ( Start listening to connections)
printf("Timer: starting to listen timer messages from Driver\n");

int id;
int time1=0;

while( 1==1)
	{
		struct timeval tv;
		tv.tv_sec = 2;
		tv.tv_usec = 000000;
		
		int nflag,rv;
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(timer_client_recv_socket, &readfds);
		nflag = timer_client_recv_socket + 1;
		rv = select(nflag,&readfds,NULL,NULL,&tv);

		if (rv == -1)
        {
                perror("select"); // error occurred in select()
        }

        if (rv == 0)
        {
            printf("-\n");
        }

        if ( (rv != -1) && (rv != 0))
        {
            if (FD_ISSET(timer_client_recv_socket, &readfds))
            {
				id = recvfrom(timer_client_recv_socket,(char*)t_msg,sizeof(Timer_message),0,(struct sockaddr *)&timer_client_adress_recv,&timer_client_adress_recv_len);
				//id = SEND(timer_client_send_socket, (char*)t_msg, sizeof(Timer_message),timer_client_adress_send);
				
				printf("Timer: Message received from Client :\n");
				
				node * b = calloc(1,sizeof(node));
					b->time = t_msg->time;
					b->port = t_msg->port;
					b->seq = t_msg->seq;
					
				if (t_msg->type == 's')
				{
					//printf("Timer: The time amd seq for head value is  %d , %lu\n",B->head->time, B->head->seq);
					add_node_to_list(B, b);
					printf("Adding - seq_number with time to List : %lu , time : %d\n", t_msg->seq, t_msg->time);
					printf("Timer: The time amd seq for head value is  %d , %lu\n",B->head->time, B->head->seq);
				}
				if (t_msg->type == 'c')
				{
					printf("Removing -node of seq_no = %lu from List\n", t_msg->seq);
					delete_node_of_seq_no(B, t_msg->seq);
					//print_linked_list(B);
				}
				//free(b);			
			}
			
		}
		//update_head(B);
		check_and_update_head_value(B, timer_client_send_socket, timer_client_adress_send);
		printf("time : %d\n",++time1);
		//printf(" Head value --> %d\n",B->head->seq);
		print_linked_list(B);
		usleep(1000); // sleep for 1 millisecond
	}
	
	close(timer_client_recv_socket);
	close(timer_client_send_socket);
	printf("Timer:Sockets Closed \n");
	printf("Timer: Program Ended\n");
}

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

/* Forward declare a type "node" to be a struct. */
typedef struct node node;

/* Declare the struct with integer members x, y */
struct node 
{
	
   int 	  time;
   int    port;
   long   seq;
   node * next;
   node * prev;   
};

typedef struct linked_list linked_list;
struct linked_list 
{
	node * head;
	node * tail;
	int  	len;
};

/* Insert c between a and b */
void add_in_between(node * a, node *c, node *b )
{
	if ( a == NULL)
		{
			b->next = c;
			b->prev = NULL;
			c->prev = b;
			c->time -= b->time; 
			return;
		}
	if ( c == NULL)
		{
			b->prev = a;
			b->next = NULL;
			a->next = b;
			b->time -= a->time;
		}
	
	b->prev = a;
	b->next = c;
	
	a->next = b;
	c->prev = b;
	
	b->time -= a->time;
	c->time -= b->time;
	
}

void add_node_to_list(linked_list * A, node * b)
{
	int val = b->time;
	
	if ( A->len == 0)
		{
		A->head = b;
		A->tail = b;
		A->len += 1;
		return ;	
		}
		
	node * h = A->head;
	node * t = A->tail;
	node * p = NULL;
	
	if (h->next == NULL)
		printf(" val is %d\n", val);
	
	while ( (h->next != NULL) && (val > h->time))
	{
		//printf( " The values encountered are %d , %d\n", val,h->time);
		p = h;
		val -= h->time;  
		h = h->next;
		//printf(" VV %d \n",val);
	}
	
	if ( val > h->time)
	{
	
		/* Update timer */
		//printf(" The values in grater loop are  %d %d\n", val, h->time);
		b->time = val - h->time;
		//printf(" The value of the new node is %d \n", b->time);	
		/* Update connections of b */
		b->prev = h;
		h->next = b;
		A->tail = b;	
	}
	else
	
	{
		//printf(" The values in smaller loop are  %d %d\n", val, h->time);
		/* Update timer */
		b->time = val;
		h->time -= val;
		
		/* Update connections of b */
		b->next = h;
		b->prev = h->prev;
			
		/* Update connections of linked lits */
		if ( p != NULL)
			{ 
				p->next = b;
			}
		else
			{
				A->head = b;
			}

		h->prev = b;
	}
	
	A->len += 1	;
	printf(" Added a new node \n");
	
}

void print_linked_list(linked_list * A)
{
	node * h = A->head;
	int count = 0;
	printf(" List is -> ");
	while ( h != NULL)
		{
			count += 1;
			//printf(" value at %d is : %d and seq_no is %lu\n",count,h->time,h->seq);
			//printf(" ( %d  : %lu) <-> ",h->time ,h->seq);
			h = h->next;
		}
	printf(" NIL. Length is %d \n",A->len);
}

void delete_node(linked_list * A)
{
	/* Always delete the head only */
	if (A== NULL)
		return;
	if (A->len == 0)
		return;
	if (A->len == 1)
	{
		A->head = NULL;
		A->tail = NULL;
		A->len = 0;
		return;
	}
	node * a = A->head;
	A->head = a->next;
	A->head->prev = NULL;
	A->len -= 1;
}

void update_head( linked_list * A)
{
	if ( A == NULL)
	{
		return;
	}
	if (A->head == NULL)
	{
		return;
	}
	if ( A->head != NULL)
	{
		//printf("Decremnting time \n");
		A->head->time -= 1;
		while( (A->head != NULL) && (A->head->time == 0))
		 {
			 printf(" Timer: Deleting node with %lu seq no from linked list \n",A->head->seq);
			 delete_node(A);
		 }
	 }
}

// Send messages to the  Client using the following list
int check_and_update_head_value(linked_list * A, int socket, struct sockaddr_in address )
{
	if ( A == NULL)
	{
		return -1;
	}
	if (A->head == NULL)
	{
		return -1;
	}
	if ( A->head != NULL)
	{
		printf("Timer: The time amd seq for head value is  %d , %lu\n",A->head->time, A->head->seq);
		A->head->time -= 1;
		
		while( (A->head != NULL) && (A->head->time == 0))
		 {
			 Timer_message * timer_msg = malloc(sizeof(Timer_message));
			 timer_msg->seq = A->head->seq;
			 timer_msg->port = A->head->port;
			 timer_msg->time = A->head->time;
			 
			 SEND(socket, (char *)timer_msg, sizeof(Timer_message), address);
			 printf(" Timer: Sent Message to timer for Retransmission of packet_no  %lu  \n",A->head->seq);
			 delete_node(A);
			 free(timer_msg);
		 }
	 }
}


void delete_node_of_seq_no(linked_list * A, long seq_no)
{

	node *p;
	node * h = A->head;
	
	if ( (A->len == 0) | (h == NULL ))
		{
		printf(" The linked list is empty, cant delete , sorry \n");
		return ;
		}

	if (h->seq == seq_no)
		{
		delete_node(A);
		printf(" The seq no %lu is deleted \n", seq_no);
		return;
		}
				
	while( h->next != NULL)
	{
		if (h->seq == seq_no)
		{ 
		p->next = h->next;
		h->next->prev = p;
		printf(" The seq no %lu is deleted \n", seq_no);
		A->len -= 1;
		return;
		}
		p = h;
		h = h->next;
	}
	
	if (h->seq == seq_no)
		{	
		p->next = NULL;
		printf(" The seq no %lu is deleted \n", seq_no);
		A->len -= 1;
		return;
		}						
	//printf(" The linked list could not find element\n");
}


// Implementation of Circular buffer //
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
#define RING_BUFFER_SIZE 64000
#define MAX_BF 640000
#define WS 20000

typedef struct
{
	int jaffa;
	// Needs to be written
} window;

typedef struct 
{
    char buffer[WS*MSS];   // data buffer
    void *buffer_end; 	// end of data buffer
    int capacity;  		// maximum number of items in the buffer
    int used;     		// used size of the buffer
    int notack;			// Not acked
    int available;		// Available size of the buffer
    int head;       	// pointer to head
    int tail;       	// pointer to tail
    int ack;			// Pointer to ack
    int win;			// Window
    int hole_table[WS];	// Maintain array for maintenance of holes
} circular_buffer;

int cb_empty(circular_buffer * cb)
{
	if ((cb->used == 0) && (cb->available == cb->capacity) && (cb->notack == 0))
		{
			return 1;
		}
	else
		{
			return -1;
		}		
}

void ring_buffer_init(circular_buffer *cb)
{
	printf("CB: Started initializing \n");
	cb->capacity = WS*MSS;
    cb->available = cb->capacity;
    cb->used = 0;
    cb->head = 0;	
    cb->tail = 0;
    cb->ack = 0;
    cb->win = WS;
    memset(cb->hole_table,0,WS);
    cb->notack = 0;
    printf("CB: Initialization completed\n");
}


void cb_free(circular_buffer *cb)
{
    free(cb->buffer);
    cb->buffer_end = NULL;
}

int get_data_index(circular_buffer *cb,char *item, int index, int size)
{
	index = (index%WS);
	if (cb->hole_table[index] ==1)
		{
			memcpy(item,cb->buffer + index*size , size);
			cb->notack += size;
			return 1;
		}
	else
	{
		printf("CB : The message is already acknowleged/ or it is empty \n");
		return -1;
	}
}

void put_data_index(circular_buffer *cb,char *item, int index, int size)
{
	index = (index%WS);
	if (cb->hole_table[index] != 1)
		{
			memcpy(cb->buffer + index*size ,item, size);
			cb->hole_table[index] = 1;
			cb->used += size;
			cb->available -= size;
		}
	else
	{
		printf("CB : The previous message is not acknowleged \n");
	}
}


int cb_push_data(circular_buffer *cb, char *item, int size)
{
    if((cb->available) < size)  
        {
			printf(" The data could not be added \n");
			return (cb->available - size  );
		}
	else
	{	
		cb->head = (cb->head + size)%(WS*MSS);
		memcpy(cb->buffer + cb->head, item, size);
		cb->available -= size;
		cb->used += size;
		cb->hole_table[((cb->head - size )/MSS)%WS] = 1;
		//printf("CB: Data of size %d units  is pushed , used is %d , free is %d \n", size,cb->used,cb->available);
		return cb->available; 
	}
}

/*
 * 
 * /
if (size <= (cb->capacity - cb->head))
			{
				memcpy(cb->buffer + cb->head, item, size);
				cb->head = cb->head + size;
			}
		else
			{
				int dd = cb->capacity - cb->head;
				memcpy(cb->buffer + cb->head, item, dd);
				cb->head = 0;
				memcpy(cb->buffer, item+dd, size-dd);
				cb->head = size-dd;
			}
*/

int cb_pop_data(circular_buffer *cb, char *item, int size)
{
    if((cb->used) < size)  
        {
			//printf("CB: The data could not be read , there is no enough data\n");
			return (cb->used - size);
		}
	else
	{	
		printf(" CB: Head is %d , tail is %d \n", cb->head , cb->tail);
		if (size <= (cb->capacity - cb->tail))	
			{
				memcpy(item, cb->buffer + cb->tail, size);
				cb->tail = cb->tail + size;
			}
		else
			{
				int dd = cb->capacity - cb->tail;
				memcpy(item,cb->buffer + cb->tail, dd);
				cb->tail = 0;
				memcpy(item+dd, cb->buffer, size-dd);
				cb->tail = size-dd;
			}
		//cb->available += size;
		cb->used -= size;
		cb->notack += size;
		cb->hole_table[((cb->tail - size )/MSS)%WS] = 0;
		printf("CB: Data of size %d units popped , used is %d , free is %d not acked is %d \n",size,cb->used,cb->available, cb->notack);
		return cb->used; 
	}
}

void update_hole_table(circular_buffer *cb, int size)
{
	int k = (cb->ack/MSS);   
	while ( cb->hole_table[k%WS] == 0)
	{
		//printf("CB: The Buffer freed the memory after ack "); 
		cb->notack -= size;
		cb->available += size;
		cb->ack = ((cb->ack + size)%(WS*MSS));
		cb->hole_table[k%WS] = 2;
		k++;
	}
}

int ack_data(circular_buffer *cb, int index, int size)
{
	index = (index%WS);
	printf("CB: The read, write & other  index is  %d,  %d ,  %d ,  %d  \n", index, cb->head, cb->tail, cb->ack);
    if((cb->notack) < size)  
        {
			printf("CB: The data could not be read , there is no enough data\n");
			return (cb->notack - size);
		}
	else
	{	
		int z = cb->notack; 
		cb->hole_table[((cb->ack - size)/MSS)%WS] = 2;
		//printf("CB: Data of size %d units popped , used is %d , free is %d \n",size,cb->used,cb->available);
		update_hole_table(cb, size);
		return z;
	}
}


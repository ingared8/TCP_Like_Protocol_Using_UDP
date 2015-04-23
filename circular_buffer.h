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
#define MAX_BF 64000000
#define MSS1 1000

typedef struct
{
	int jaffa;
	// Needs to be written
} window;

typedef struct 
{
    char buffer[MAX_BF];   // data buffer
    void *buffer_end; 	// end of data buffer
    int capacity;  		// maximum number of items in the buffer
    int used;     		// used size of the buffer
    int available;		// Available size of the buffer
    int head;       	// pointer to head
    int tail;       	// pointer to tail
} circular_buffer;

void ring_buffer_init(circular_buffer *cb, int capacity)
{
	printf("CB: Started initializing \n");
	cb->buffer_end = (char *)cb->buffer + capacity;
    cb->capacity = capacity;
    cb->available = capacity;
    cb->used = 0;
    cb->head = 0;	
    cb->tail = 0;
    printf("CB: Initialization completed\n");
}

void cb_free(circular_buffer *cb)
{
    free(cb->buffer);
    cb->buffer_end = NULL;
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
		cb->available -= size;
		cb->used += size;
		printf("CB: Data of size %d units  is pushed , used is %d , free is %d \n", size,cb->used,cb->available);
		return cb->available; 
	}
}

int cb_pop_data(circular_buffer *cb, char *item, int size)
{
    if((cb->used) < size)  
        {
			printf("CB: The data could not be read , there is no enough data\n");
			return (cb->used - size);
		}
	else
	{	
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
		cb->available += size;
		cb->used -= size;
		printf("CB: Data of size %d units popped , used is %d , free is %d \n",size,cb->used,cb->available);
		return cb->used; 
	}
}

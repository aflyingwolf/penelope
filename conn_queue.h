#ifndef CONN_QUEUE_H_INCLUDED
#define CONN_QUEUE_H_INCLUDED

/* An item in the connection queue. */
typedef struct conn_queue_item CQ_ITEM;
struct conn_queue_item {
	int			sfd;
	char		szAddr[16];
	int			port;
	CQ_ITEM		*next;
};


/* A connection queue. */
typedef struct conn_queue CQ;
struct conn_queue {
	CQ_ITEM			*head;
	CQ_ITEM			*tail;
	pthread_mutex_t	lock;
};


/*
 * Initializes a connection queue.
 */
void cq_init(CQ *cq);


/*
 * Looks for an item on a connection queue, but doesn't block if there isn't
 * one.
 * Returns the item, or NULL if no item is available
 */
CQ_ITEM *cq_pop(CQ *cq);

/*
 * Adds an item to a connection queue.
 */
void cq_push(CQ *cq, CQ_ITEM *item);

#endif // CONN_QUEUE_H_INCLUDED

#include <stdlib.h>
#include <pthread.h>
#include "conn_queue.h"

/*
 * Initializes a connection queue.
 */
void cq_init(CQ *cq) {
	pthread_mutex_init(&cq->lock, NULL);
	cq->head = NULL;
	cq->tail = NULL;
}


/*
 * Looks for an item on a connection queue, but doesn't block if there isn't
 * one.
 * Returns the item, or NULL if no item is available
 */
CQ_ITEM *cq_pop(CQ *cq) {
	CQ_ITEM *item;


	pthread_mutex_lock(&cq->lock);
	item = cq->head;
	if (NULL != item) {
		cq->head = item->next;
		if (NULL == cq->head)
			cq->tail = NULL;
	}
	pthread_mutex_unlock(&cq->lock);


	return item;
}

/*
 * Adds an item to a connection queue.
 */
void cq_push(CQ *cq, CQ_ITEM *item) {
	item->next = NULL;


	pthread_mutex_lock(&cq->lock);
	if (NULL == cq->tail)
		cq->head = item;
	else
		cq->tail->next = item;
	cq->tail = item;
	pthread_mutex_unlock(&cq->lock);
}

#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

//#define LOG_CURRENT_STATE

typedef struct {
	pthread_t thread_id;               /* unique ID of this thread */
	struct ev_loop *loop;              /* libev loop this thread uses */
	struct ev_async async_watcher;     /* async watcher for new connect */
	struct conn_queue *new_conn_queue; /* queue of new connections to handle */
} WORK_THREAD;


typedef struct {
	pthread_t thread_id;           /* unique ID of this thread */
	struct ev_loop *loop;          /* libev loop this thread uses */
	struct ev_io accept_watcher;   /* accept watcher for new connect */
} DISPATCHER_THREAD;

void thread_init();

void dispath_conn(int anewfd,struct sockaddr_in asin);

#endif // MAIN_H_INCLUDED

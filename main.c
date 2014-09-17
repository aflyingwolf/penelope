#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <ev.h>
#include <arpa/inet.h>
#include "conn_queue.h"
#include "fd.h"
#include "log4c_pu.h"
#include "client_function.h"
#include "user_state.h"
#include "room_state.h"
#include "main.h"

/*
 * libev default loop, with a accept_watcher to accept the new connect
 * and dispatch to WORK_THREAD.
 */
static	DISPATCHER_THREAD	dispatcher_thread;
/*
 * Each libev instance has a async_watcher, which other threads
 * can use to signal that they've put a new connection on its queue.
 */
static	WORK_THREAD			*work_threads;
/*
 * Number of worker threads that have finished setting themselves up.
 */
static	pthread_mutex_t		init_lock;
static	pthread_cond_t		init_cond;
pthread_mutex_t		fd_hash_lock;
pthread_mutex_t		room_state_lock;
pthread_mutex_t		user_state_lock;
pthread_mutex_t		connect_lock;

static	int		init_count		= 0;
static	int		round_robin		= 0;
static	int		MAX_WORK_THREAD	= 4;
static	int		USER_HASH_SIZE	= 40;

USER_ARRAY users;
SERVER	server;

//receive
const	int		buffer_size = 2048 + 1;

/*
 * Worker thread: main event loop
 */
static void * worker_libev(void *arg) {
	WORK_THREAD *me = arg;

	/* Any per-thread setup can happen here; thread_init() will block until
	 * all threads have finished initializing.
	 */

	pthread_mutex_lock(&init_lock);
	init_count++;
	pthread_cond_signal(&init_cond);
	pthread_mutex_unlock(&init_lock);

	me->thread_id = pthread_self();
	ev_loop(me->loop, 0);
	return NULL;
}

/*
 * Creates a worker thread.
 */
static void create_worker(void *(*func)(void *), void *arg)
{
	pthread_t       thread;
	pthread_attr_t  attr;
	int             ret;

	pthread_attr_init(&attr);

	if ((ret = pthread_create(&thread, &attr, func, arg)) != 0)
	{
		fprintf(stderr, "Can't create thread: %s\n",
				strerror(ret));
		exit(1);
	}
}

static void async_cb (EV_P_ ev_async *w, int revents)
{
	CQ_ITEM *item;

	item = cq_pop(((WORK_THREAD*)(w->data))->new_conn_queue);

	if (NULL != item)
	{
		ev_io* recv_watcher=malloc(sizeof(ev_io));
		ev_io_init(recv_watcher,recv_callback,item->sfd,EV_READ);
		ev_io_start(((WORK_THREAD*)(w->data))->loop,recv_watcher);

		printf("thread[%lu] accept: fd :%d  addr:%s port:%d\n",
				((WORK_THREAD*)(w->data))->thread_id,item->sfd,item->szAddr,item->port);

		free(item);
		item = NULL;
	}
}

/*
 * Set up a thread's information.
 */
static void setup_thread(WORK_THREAD *me)
{
	me->loop = ev_loop_new(0);
	if (! me->loop)
	{
		fprintf(stderr, "Can't allocate event base\n");
		exit(1);
	}

	me->async_watcher.data = me;
	/* Listen for notifications from other threads */
	ev_async_init(&me->async_watcher, async_cb);
	ev_async_start(me->loop, &me->async_watcher);

	me->new_conn_queue = malloc(sizeof(struct conn_queue));
	if (me->new_conn_queue == NULL)
	{
		perror("Failed to allocate memory for connection queue\n");
		exit(EXIT_FAILURE);
	}
	cq_init(me->new_conn_queue);
}

void thread_init()
{
        dispatcher_thread.loop = ev_default_loop(0);
	dispatcher_thread.thread_id = pthread_self();

	int nthreads = MAX_WORK_THREAD;
	pthread_mutex_init(&init_lock, NULL);
	pthread_cond_init(&init_cond, NULL);
	pthread_mutex_init(&fd_hash_lock, NULL);
	pthread_mutex_init(&room_state_lock, NULL);
	pthread_mutex_init(&user_state_lock, NULL);
	pthread_mutex_init(&connect_lock, NULL);

	work_threads = calloc(nthreads, sizeof(WORK_THREAD));
	if (! work_threads)
	{
		perror("Can't allocate thread descriptors\n");
		exit(1);
	}

	int i = 0;
	for (i = 0; i < nthreads; i++)
	{
		setup_thread(&work_threads[i]);
	}

	/* Create threads after we've done all the libevent setup. */
	for (i = 0; i < nthreads; i++)
	{
		create_worker(worker_libev, &work_threads[i]);
	}

	/* Wait for all the threads to set themselves up before returning. */
	pthread_mutex_lock(&init_lock);
	while (init_count < nthreads)
	{
		pthread_cond_wait(&init_cond, &init_lock);
	}
	pthread_mutex_unlock(&init_lock);
}

void
dispath_conn(int anewfd,struct sockaddr_in asin)
{
	// set the new connect item
	CQ_ITEM *lpNewItem = calloc(1, sizeof(CQ_ITEM));
	if (!lpNewItem) {
		perror("Can't allocate connection item\n");
		exit(1);
	}

	lpNewItem->sfd = anewfd;
	strcpy(lpNewItem->szAddr,inet_ntoa(asin.sin_addr));
	lpNewItem->port = asin.sin_port;

	// libev default loop, accept the new connection, round-robin
	// dispath to a work_thread.
	int robin = round_robin%init_count;
	cq_push(work_threads[robin].new_conn_queue,lpNewItem);
	ev_async_send(work_threads[robin].loop, &(work_threads[robin].async_watcher));
	round_robin++;
}

int main()
{
        thread_init();

	users = initialize_user_array(USER_HASH_SIZE);
        server = initialize_server();

	//日志记录器初始化
	if(1 == log4c_pu_init())
		printf ( "Log model init failed\n" );

	int listen = socket_init();
	ev_io_init(&(dispatcher_thread.accept_watcher), accept_callback,listen, EV_READ);
	ev_io_start(dispatcher_thread.loop,&(dispatcher_thread.accept_watcher));
	ev_loop(dispatcher_thread.loop,0);
	ev_loop_destroy(dispatcher_thread.loop);

	//日志记录器销毁
	if(1 == log4c_pu_fini())
		printf ( "Log model exit\n" );

	destory_server(server);
        destory_user_array(users);
	return 0;
}

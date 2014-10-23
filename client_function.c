#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <ev.h>
#include "log4c_pu.h"
#include "fd.h"
#include "main.h"
#include "user_state.h"
#include "room_state.h"
#include "web_socket.h"
#include "redis_pu.h"
#include "client_function.h"

const   int     SERVER_ID   = 1;
const	int		PORT		= 8051;
const	char	*ADDR_IP	= "192.168.48.152";

extern USER_ARRAY users;
extern SERVER	server;

static	int		connect_count		= 0;
static	int		connect_times		= 0;
extern pthread_mutex_t connect_lock;

int
socket_init()
{
    struct sockaddr_in my_addr;
    int listener;
    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }
    else
    {
        printf("SOCKET CREATE SUCCESS! \n");
    }
    int so_reuseaddr=1;
    setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&so_reuseaddr,sizeof(so_reuseaddr));
    int nZero=0;
    setsockopt(listener,SOL_SOCKET,SO_SNDBUF,(char *)&nZero,sizeof(nZero));
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = PF_INET;
    my_addr.sin_port = htons(PORT);
    my_addr.sin_addr.s_addr = inet_addr(ADDR_IP);

    if (bind(listener, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))== -1)
    {
        perror("bind error!\n");
        exit(1);
    }
    else
    {
        printf("IP BIND SUCCESS,IP:%s\n",ADDR_IP);
    }

    if (listen(listener, 1024) == -1)
    {
        perror("listen error!\n");
        exit(1);
    }
    else
    {
        printf("LISTEN SUCCESS,PORT:%d\n",PORT);
    }
    return listener;
}

void
accept_callback(struct ev_loop *loop, ev_io *w, int revents)
{
    int			newfd;
    struct		sockaddr_in sin;
    socklen_t 	addrlen = sizeof(struct sockaddr);

    while ((newfd = accept(w->fd, (struct sockaddr *)&sin, &addrlen)) < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            //these are transient, so don't log anything.
            continue;
        }
        else
        {
            printf("accept error.[%s]\n", strerror(errno));
            break;
        }
    }

    dispath_conn(newfd,sin);
    init_connect ( newfd );
}

void
recv_callback(struct ev_loop *loop, ev_io *w, int revents)
{
    int fd=w->fd;
    int connected = get_fd_connected(fd);
    int buffer_size_local = connected == 0 ? REQUEST_HEADER_LEN_MAX : buffer_size;
    char	buffer[buffer_size_local] ;
    int		ret =0, iret = 0;
    memset(buffer,0,buffer_size_local);
loop:
    ret=recv(fd, buffer + iret, buffer_size_local - iret , 0);
    iret += ret > 0 ? ret : 0;
    if(ret > 0)
    {
        if(connected == 0) {
            char* secWebSocketKey = computeAcceptKey(buffer);
            if(secWebSocketKey) {
                shakeHand(fd, secWebSocketKey);
                update_fd_connected(fd, 1);
                free(secWebSocketKey);
            } else {
                goto loop;
            }
        } else {
            //printf("thread[%lu] strlen(buffer): %d ret: %d sizeof(buffer): %d \n",
            //pthread_self(),strlen(buffer), ret, sizeof(buffer));
            unsigned long totalLen = getTotalDataLength(buffer, iret);
            if(totalLen <= 0 && totalLen != iret) {
                goto loop;
            }
            char* data = analyData(buffer, iret);
            if(data == NULL)
            {
                goto loop;
            }
            switch( dispath_center(loop , w, data, fd))
            {
            case 0:
                break;
            case -1:
                printf ( "function error\n" );
                break;
            }
            free(data);
        }
    }
    else if(ret ==0)
    {
        printf("thread[%lu] remote socket closed!socket fd: %d\n",pthread_self(),fd);

        leave_server_from_room(loop, w);

        return;
    }
    else
    {
        if(errno == EAGAIN ||errno == EWOULDBLOCK)
        {
            goto loop;
        }
        else
        {
            printf("thread[%lu] ret :%d ,close socket fd : %d\n",pthread_self(),ret,fd);

            leave_server_from_room(loop, w);

            return;
        }
    }

    printf("thread[%lu] socket fd : %d, turn to listen loop!\n",pthread_self(), fd);
}

void
write_callback(struct ev_loop *loop, ev_io *w, int revents)
{

}

void
init_connect ( int selffd )
{
    enable_fd(selffd);
    add_server_connect();
    return ;
}

int
dispath_center (struct ev_loop *loop, ev_io *w, char const *buffer, int selffd )
{
    char *connect		= "<connect>";
    char* msg           = "<msg>";
    int r = 0;
    if(0 == strncmp(buffer, msg, strlen(msg)) )
    {
        r = 0 == transmit(buffer, selffd, 0) ? 0 : -1;
    }
    else if(0 == strncmp(buffer, connect, strlen(connect)))
    {
        r = 0 == join_server(buffer, selffd) ? 0 : -1;
    }
    else
    {
        LOG("logid:%d fd:%d roomid:%d uid:%d, send unregnized msg\n",get_fd_logid(selffd), selffd, get_fd_roomid(selffd), get_fd_uid(selffd));
        printf ("logid:%d fd:%d roomid:%d uid:%d, send unregnized msg:%s\n",get_fd_logid(selffd), selffd,get_fd_roomid(selffd), get_fd_uid(selffd), buffer);
        r = -1;
    }
    return r;
}

int
transmit (char const *buffer, int selffd ,int type)
{
    char buffer_tmp[buffer_size];
    int i = 0;
    int *afd = NULL;
    int count = 0;

    snprintf(buffer_tmp,buffer_size,"%s",buffer);
    count = get_roommate_fd(get_fd_roomid(selffd), selffd, &afd, server);
    if(count > 0)
    {
        LOG("transmit msg: fd:%d fd_count:%d", selffd, count);
    }

    for(i = 0; i < count; i++ )
    {
        if(get_fd_status(afd[i]) == 1)
        {
            //send(afd[i], buffer_tmp, strlen(buffer_tmp), 0);
            response(afd[i], buffer_tmp);
        }
    }

    free(afd);

    return 0;
}

int
join_server ( char const *buffer, int selffd)
{
    int uid			= 0;
    int roomid		= 0;
    int logid	= 0;

    sscanf(buffer,"%*[^0-9]%d%*[^0-9]%d%*[^0-9]%d%*[^0-9]",&roomid, &uid, &logid);

    printf ("uid:%d,roomid:%d,logid:%d\n", uid, roomid, logid);
    LOG("roomid:%d (%d) logid:%d fd:%d buffer:%s", roomid, get_fd_roomid(selffd), logid, selffd, buffer);

    if(0 == uid || 0 == roomid || 0 == logid)
    {
        return -1;
    }

    if(-1 != get_fd_roomid(selffd))
        return -1;

    insert_fd(selffd, logid, uid, roomid);
    insert_into_room(selffd, roomid, server);
    insert_user(logid, selffd, roomid, users);

    log_cur_state();

    return 0;
}

void
leave_server_from_room ( struct ev_loop *loop, ev_io *w)
{
    char leave_room_buffer[buffer_size] ;
    int logid = get_fd_logid(w->fd);
    int roomid = get_fd_roomid(w->fd);
    int uid = get_fd_uid(w->fd);

    minus_server_connect();

    LOG("roomid:%d logid:%d fd:%d", roomid, logid, w->fd);

    if(roomid > 0)
    {
        snprintf(leave_room_buffer,buffer_size,"<msg><rid>%d</rid><disconnect>1</disconnect><uid>%d</uid></msg>", roomid, uid);
        transmit(leave_room_buffer, w->fd, 0);
        delete_user(logid, users);

        delete_from_room(w->fd, roomid, server);

        delete_fd(w->fd);

        log_cur_state();
    }

    close(w->fd);
    ev_io_stop(loop, w);
    free(w);
    return ;
}

void log_cur_state (  )
{
#ifdef LOG_CURRENT_STATE
    char *buffer = NULL;

    buffer = show_room(server);
    LOG("show_room:%s",buffer);
    free(buffer);

    buffer = NULL;
    buffer = show_users(users);
    LOG("show_users:%s",buffer);
    free(buffer);
#endif
    return ;
}

void
minus_server_connect()
{
    pthread_mutex_lock(&connect_lock);

    connect_times ++;
    connect_count --;
    if(connect_times == 3)
    {
        change_server_connect(SERVER_ID, connect_count);
        connect_times = connect_count = 0;
    }
    pthread_mutex_unlock(&connect_lock);

}

void
add_server_connect()
{
    pthread_mutex_lock(&connect_lock);

    connect_times ++;
    connect_count ++;
    if(connect_times == 3)
    {
        change_server_connect(SERVER_ID, connect_count);
        connect_times = connect_count = 0;
    }
    pthread_mutex_unlock(&connect_lock);
}

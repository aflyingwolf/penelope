#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "fd.h"

extern pthread_mutex_t fd_hash_lock;
FD_ITEM fd_array[MAX_FD];

void init_fd()
{
    memset(fd_array, 0 ,sizeof(fd_array));
    return;
}

int get_fd_status(unsigned int fd)
{
    return fd_enable == fd_array[fd].status ? 1 : 0;
}

int get_fd_roomid(unsigned int fd)
{
    if(fd_enable == fd_array[fd].status)
        return fd_array[fd].roomid;
    else
        return -1;
}

int get_fd_uid(unsigned int fd)
{
    if(fd_enable == fd_array[fd].status)
        return fd_array[fd].uid;
    else
        return -1;
}

int get_fd_connected(unsigned int fd)
{
    if(fd_enable == fd_array[fd].status)
    {
        return fd_array[fd].connected;
    }
    else
    {
        return -1;
    }
}

int get_fd_logid(unsigned int fd)
{
    if(fd_enable == fd_array[fd].status)
        return fd_array[fd].logid;
    else
        return -1;
}

void disable_fd(unsigned int fd)
{
    if(fd > 0)
    {
        pthread_mutex_lock(&fd_hash_lock);

        fd_array[fd].status = fd_disable;
        fd_array[fd].roomid = -1;
        fd_array[fd].connected = 0;

        pthread_mutex_unlock(&fd_hash_lock);
    }
    return;
}

void enable_fd(unsigned int fd)
{
    if(fd > 0)
    {
        pthread_mutex_lock(&fd_hash_lock);

        fd_array[fd].status = fd_enable;
        fd_array[fd].roomid = -1;
        fd_array[fd].connected = 0;

        pthread_mutex_unlock(&fd_hash_lock);
    }

    return;
}

int insert_fd(unsigned int fd,int logid, int uid, int roomid)
{
    if(fd > 0 && fd_enable == fd_array[fd].status)
    {
        pthread_mutex_lock(&fd_hash_lock);

        fd_array[fd].logid		= logid;
        fd_array[fd].uid		= uid;
        fd_array[fd].roomid	= roomid;

        pthread_mutex_unlock(&fd_hash_lock);
        return 0;
    }
    return -1;
}

int delete_fd(unsigned int fd)
{
    if(fd > 0 && fd_enable == fd_array[fd].status)
    {
        pthread_mutex_lock(&fd_hash_lock);

        fd_array[fd].status = fd_disable;

        pthread_mutex_unlock(&fd_hash_lock);
        return 0;
    }
    return -1;
}

int update_fd_roomid(unsigned int fd, int logid, int roomid)
{
    if(fd > 0 && fd_enable == fd_array[fd].status && logid == fd_array[fd].logid)
    {
        pthread_mutex_lock(&fd_hash_lock);

        fd_array[fd].roomid = roomid;

        pthread_mutex_unlock(&fd_hash_lock);
        return 0;
    }
    return -1;
}

int update_fd_connected(unsigned int fd, int connected)
{
    if(fd > 0 && fd_enable == fd_array[fd].status)
    {
        pthread_mutex_lock(&fd_hash_lock);

        fd_array[fd].connected = connected;

        pthread_mutex_unlock(&fd_hash_lock);
        return 0;
    }
    return -1;
}


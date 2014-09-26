#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include "fd.h"
#include "user_state.h"

extern pthread_mutex_t user_state_lock;

USER_ARRAY
initialize_user_array(unsigned int array_size )
{
    unsigned int i;
 	USER_ARRAY u;

    u =( USER_ARRAY ) malloc(sizeof( USER_ARRAY ));
    if (u == NULL)
        return NULL;

    u->array_size = array_size;
    u->user_count = 0;

    u->user_array = malloc(sizeof( USERS ) * (array_size ));
    if ( u->user_array == NULL ){
        printf("no memory\n");
        return NULL;
    }

    for(i = 0; i < u->array_size; i++){
        u->user_array[i].first = NULL;
		u->user_array[i].last = NULL;
    }
    return u;
}

//链表销毁
void
destory_user_array( USER_ARRAY u )
{
	free(u->user_array);
	free(u);
	return ;
}

void insert_user(int logid, int fd, int roomid, USER_ARRAY u)
{
	USER_ITEM *item;
	int slot	= -1;
	if(0 == is_user_exist(logid, u))
		return ;
	slot = logid % u->array_size;

	item =( USER_ITEM *) malloc(sizeof( USER_ITEM ));
    if ( item == NULL ){
        printf("no memory\n");
        return ;
    }
	item->logid	= logid;
	item->fd	= fd;
	item->roomid = roomid;
	item->next	= NULL;

	pthread_mutex_lock(&user_state_lock);

	if(NULL == u->user_array[slot].last)
	{
		//房间为空时的处理逻辑
		u->user_array[slot].first = item;
	}
	else
	{
		u->user_array[slot].last->next = item;
	}
	u->user_array[slot].last = item;
	u->user_count ++;

	pthread_mutex_unlock(&user_state_lock);

	return ;
}

//当用户断开连接后，将用户从链表中删除掉
void
delete_user(int logid, USER_ARRAY u)
{

	USER_ITEM *pre = NULL;
	USER_ITEM *self = NULL;
	int slot	= -1;

	slot = logid % u->array_size;
	pre = u->user_array[slot].first;
	//检查第一个结点
	//如果房间为空，则直接返回
	if(NULL == pre)
	{
		return;
	}
	else if( logid == pre->logid)
	{
		pthread_mutex_lock(&user_state_lock);

		u->user_array[slot].first = pre->next;
		//如果只有一个结点，需要修改last指针
		if(NULL == pre->next)
		{
			u->user_array[slot].last = NULL;
		}
		free(pre);
		pre = NULL;
		u->user_count --;

		pthread_mutex_unlock(&user_state_lock);
		return;
	}
	//如果有多个结点，且第一个结点不是要找的结点
	self = pre->next;
	while(NULL != self)
	{
		pthread_mutex_lock(&user_state_lock);

		if(logid == self->logid)
		{
			if(u->user_array[slot].last == self )
			{
				u->user_array[slot].last = pre;
			}
			pre->next = self->next;
			free(self);
			self = NULL;
			u->user_count --;

			pthread_mutex_unlock(&user_state_lock);
			return;
		}
		pre = self;
		self = self->next;

		pthread_mutex_unlock(&user_state_lock);
	}

	return;
}

void update_user_room(int logid, int roomid, USER_ARRAY u)
{
	int slot	= -1;
	USER_ITEM *self;

	slot = logid % u->array_size;
	self = u->user_array[slot].first;
	while(NULL != self && logid != self->logid)
	{
		self = self->next;
	}
	if(NULL != self)
	{
		pthread_mutex_lock(&user_state_lock);

		self->roomid = roomid;

		pthread_mutex_unlock(&user_state_lock);
		return ;
	}
}

int is_user_exist(int logid, USER_ARRAY u)
{
	int slot	= -1;
	USER_ITEM *self;

	slot = logid % u->array_size;
	self = u->user_array[slot].first;
	while(NULL != self && logid != self->logid)
	{
		self = self->next;
	}
	if(NULL == self)
		return -1;
	else
		return 0;
}

int get_user_roomid(int logid, USER_ARRAY u)
{
	int slot = -1;
	USER_ITEM *self;

	slot = logid % u->array_size;
	self = u->user_array[slot].first;
	while(NULL != self && logid != self->logid)
	{
		self = self->next;
	}
	if(NULL == self)
		return -1;
	else
		return self->roomid;

}

char * show_users ( USER_ARRAY u )
{
	char *log_buffer = NULL;
	int i = 0;
	USER_ITEM *item = NULL;

	log_buffer = (char *)malloc(sizeof(char)*4096);
	sprintf(log_buffer,"\n");
	for(i = 0; i< u->array_size; i++)
	{
		if(NULL != u->user_array[i].first)
		{
			sprintf(log_buffer,"%s slot[%d]logid:",log_buffer,i);
			item = u->user_array[i].first;
			while(NULL != item)
			{
				sprintf ( log_buffer,"%s ->%d",log_buffer,item->logid );
				item = item->next;
			}
			sprintf(log_buffer,"%s \n",log_buffer);
		}
	}
	return log_buffer;
}

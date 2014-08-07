#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include "room_state.h"
#include "fd.h"

unsigned int room_size = 10000;
extern pthread_mutex_t room_state_lock;

SERVER
initialize_server()
{
    unsigned int i;
 	SERVER s;

    s =( SERVER ) malloc(sizeof( SERVER ));
    if (s == NULL)
        return NULL;

    s->room_size = room_size + 1;

    s->the_rooms = malloc(sizeof( ROOM ) * (room_size + 1));
    if ( s->the_rooms == NULL ){
        printf("no memory\n");
        return NULL;
    }

    for(i = 0; i < s->room_size; i++){
        s->the_rooms[i].first = NULL;
		s->the_rooms[i].last = NULL;
		s->the_rooms[i].current_count = 0;
    }

    return s;
}

//链表销毁
void
destory_server( SERVER s )
{
	free(s->the_rooms);
	free(s);
	return ;
}

void
insert_into_room(int fd, int roomid, SERVER s )
{
	R_ITEM *item;

	item =(R_ITEM *) malloc(sizeof( R_ITEM ));
    if ( item == NULL ){
        printf("no memory\n");
        return ;
    }

	item->fd = fd;
	item->next = NULL;

	pthread_mutex_lock(&room_state_lock);

	if(NULL == s->the_rooms[roomid].last)
	{
		//房间为空时的处理逻辑
		s->the_rooms[roomid].first = item;
	}
	else
	{
		s->the_rooms[roomid].last->next = item;
	}
	s->the_rooms[roomid].last = item;
	s->the_rooms[roomid].current_count ++;

	pthread_mutex_unlock(&room_state_lock);
}

//当用户断开连接后，将用户从链表中删除掉
void
delete_from_room(int selffd, int roomid, SERVER s)
{

	R_ITEM *pre = NULL;
	R_ITEM *self = NULL;

	pre = s->the_rooms[roomid].first;
	//检查第一个结点
	//如果房间为空，则直接返回
	if(NULL == pre)
	{
		return;
	}
	else if(selffd == pre->fd)
	{
		pthread_mutex_lock(&room_state_lock);

		s->the_rooms[roomid].first = pre->next;
		//如果只有一个结点，需要修改last指针
		if(NULL == pre->next)
		{
			s->the_rooms[roomid].last = NULL;
		}
		free(pre);
		pre = NULL;

		s->the_rooms[roomid].current_count --;

		pthread_mutex_unlock(&room_state_lock);
		return;
	}
	//如果有多个结点，且第一个结点不是要找的结点
	self = pre->next;
	while(NULL != self)
	{
		pthread_mutex_lock(&room_state_lock);

		if(selffd == self->fd)
		{
			if(s->the_rooms[roomid].last == self )
			{
				s->the_rooms[roomid].last = pre;
			}
			pre->next = self->next;
			free(self);
			self = NULL;

			s->the_rooms[roomid].current_count --;

			pthread_mutex_unlock(&room_state_lock);
			return;
		}
		pre = self;
		self = self->next;

		pthread_mutex_unlock(&room_state_lock);
	}

	return;
}

//根据自己的房间编号，读取其他室友的socket句柄
int
get_roommate_fd(int roomid, int selffd, int **afd, SERVER s)
{
	int **alfd = afd;
	int i = 0;
	int j = 0;

	R_ITEM *item = NULL;

	if(-1 == roomid)
		return i;

	if(roomid <= s->room_size)
	{
		item = s->the_rooms[roomid].first;
		*alfd = (int *)malloc(sizeof(int)*(s->the_rooms[roomid].current_count) );
		for(i=0;NULL != item; item = item->next)
		{
			if(item->fd != selffd)
			{
				for(j = 0; j< i;j++)
				{
					if(item->fd == (*alfd)[j])
						break;
				}
				if(j == i)
				{
					(*alfd)[i] = item->fd;
					i++;
				}
			}
		}
	}
	return i;
}

int
fd_room_exist ( int selffd, int roomid, SERVER s )
{
	R_ITEM *item = NULL;

	item = s->the_rooms[roomid].first;
	while(NULL != item)
	{
		if(selffd == item->fd)
			return 0;
		else
			item = item->next;
	}
	return -1;
}

int
is_fd_in_room_Empty ( int selffd, SERVER s )
{
	int roomid;

	roomid = get_fd_roomid(selffd);
	if(-1 == roomid)
		return -1;
	if((s->the_rooms[roomid].first == s->the_rooms[roomid].last)
			&& (selffd == s->the_rooms[roomid].first->fd)  )
		return 0;
	else
		return -1;

}

char * show_room ( SERVER s )
{
	char *log_buffer = NULL;
	int i = 0;
	R_ITEM *item = NULL;

	log_buffer = (char *)malloc(sizeof(char)*8024);
	sprintf(log_buffer,"\n");
	for(i = 0; i< s->room_size; i++)
	{
		if(s->the_rooms[i].first != NULL)
		{
			sprintf(log_buffer,"%s room[%d]fd:",log_buffer,i);
			item = s->the_rooms[i].first;
			while(NULL != item)
			{
				sprintf ( log_buffer,"%s ->%d",log_buffer,item->fd );
				item = item->next;
			}
			sprintf(log_buffer,"%s \n",log_buffer);
		}
	}
	return log_buffer;
}

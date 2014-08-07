#ifndef USER_STATE_H_INCLUDED
#define USER_STATE_H_INCLUDED

typedef struct user USER_ITEM;
struct user{
	int		logid;
	int		fd;
	int		roomid;
	USER_ITEM *next;
};

struct hash_user{
	USER_ITEM *first;
	USER_ITEM *last;
};

typedef struct hash_user USERS;

struct user_array
{
	unsigned int array_size;
	unsigned int user_count;
	USERS *user_array;
};


typedef struct user_array *USER_ARRAY;

USER_ARRAY initialize_user_array(unsigned int array_size);
void destory_user_array( USER_ARRAY u );
void update_user_room(int logid, int room_id, USER_ARRAY u);
void insert_user(int logid, int fd, int roomid, USER_ARRAY u);
void delete_user(int logid, USER_ARRAY u);
int is_user_exist(int logid, USER_ARRAY u);
int get_user_roomid(int logid, USER_ARRAY u);
char* show_users ( USER_ARRAY u );
#endif // USER_STATE_H_INCLUDED

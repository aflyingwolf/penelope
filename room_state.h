#ifndef ROOM_STATE_H_INCLUDED
#define ROOM_STATE_H_INCLUDED

typedef struct hash_player R_ITEM;
struct hash_player{
	int		fd;
	R_ITEM *next;
};

struct hash_room{
	R_ITEM	*first;
	R_ITEM	*last;
	int	current_count;
};

typedef struct hash_room ROOM;

struct hash_server
{
	unsigned int room_size;
	ROOM *the_rooms;
};


typedef struct hash_server *SERVER;

SERVER initialize_server();
void destory_server( SERVER s );
void insert_into_room(int fd, int roomid, SERVER s );
void delete_from_room(int selffd, int roomid, SERVER s);
int get_roommate_fd(int roomid, int selffd, int **afd, SERVER s);
int fd_room_exist ( int selffd, int roomid, SERVER s );
int is_fd_in_room_Empty ( int selffd, SERVER s );
char* show_room(SERVER s);
#endif // ROOM_STATE_H_INCLUDED

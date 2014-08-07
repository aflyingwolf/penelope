#ifndef FD_H_INCLUDED
#define FD_H_INCLUDED

#define MAX_FD 40960

enum status_fd {fd_disable, fd_enable};

//每个fd对应的元素
typedef struct fd_item FD_ITEM;

struct fd_item {
	int		logid;
	int		uid;
	int		roomid;
	int     connected;
	enum status_fd status;
};

void init_fd();
int get_fd_roomid(unsigned int fd);
int get_fd_uid(unsigned int fd);
int get_fd_logid(unsigned int fd);
void enable_fd(unsigned int fd);
void disable_fd(unsigned int fd);
int insert_fd(unsigned int fd, int logid, int uid, int roomid);
int delete_fd(unsigned int fd);
int update_fd_roomid(unsigned int fd, int logid, int roomid);
int update_fd_connected(unsigned int fd, int connected);
int get_fd_connected(unsigned int fd);
int get_fd_status(unsigned int fd);
#endif // FD_H_INCLUDED

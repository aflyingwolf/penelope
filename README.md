##penelope
------------
###简介

基于libev开发。多个html5 web socket客户端可以通过penelope通信。

主进程启动时，初始化四个子线程，主进程启动loop循环, 四个子线程各启动自身的loop循环。

主进程负责监听客户端连接请求，客户端请求到达时，主进程将请求随机分配给某一子线程，

子线程负责和客户端的请求交互。

###数据结构关系

默认接收最大套接字描述符是40960. 每一个套接字描述符（对应某个客户端）指向结构：

    struct fd_item {
        int		logid;
    	int		uid;
    	int		roomid;
    	int     connected;     //fd目前是否处于连接状态
    	enum status_fd status; //fd是否可用
    };
    
roomid对应该客户端所在的房间号，多个客户端可以加入一个房间，在同一个房间中的客户端发

的消息可以让其它人接收到。房间对应的数据结构：

    typedef struct hash_player R_ITEM;
    struct hash_player{
        int    	fd;
    	R_ITEM *next;
    };
    struct hash_room{
    	R_ITEM	*first;
    	R_ITEM	*last;
    	int	current_count;
    };
    typedef struct hash_room ROOM;
    
每个服务器上可以包含多个房间：

    struct hash_server
    {
        unsigned int room_size;
    	ROOM *the_rooms;
    };
    typedef struct hash_server *SERVER;
    
用户：

    typedef struct user USER_ITEM;
    struct user{
        int    	logid;  //用户id
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
    
程序启动时，默认建立10000个房间，初始化40个用户组。用户连接上服务端选择房间后，根据

传递的logid,roomid将用户归入相应的用户组和房间。用户发送信息时，程序会将信息传递给同

一个房间的其它在线用户，这样就可以通信了。用户退出或断开连接时，要做好相应的清理操作。

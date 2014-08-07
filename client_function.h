#ifndef CLIENT_FUNCTION_H_INCLUDED
#define CLIENT_FUNCTION_H_INCLUDED

extern const int buffer_size;

int socket_init();
void dispath_conn(int anewfd,struct sockaddr_in asin);
void accept_callback(struct ev_loop *loop, ev_io *w, int revents);
void recv_callback(struct ev_loop *loop, ev_io *w, int revents);
void write_callback(struct ev_loop *loop, ev_io *w, int revents);
int dispath_center (struct ev_loop *loop, ev_io *w, char const *buffer, int selffd );
int transmit (char const *buffer, int selffd ,int type);
int join_server ( char const *buffer, int selffd);
void add_server_connect();
void minus_server_connect();
void init_connect ( int selffd );
void leave_server_from_room ( struct ev_loop *loop, ev_io *w);
void log_cur_state();
#endif // CLIENT_FUNCTION_H_INCLUDED

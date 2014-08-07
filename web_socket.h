#ifndef WEB_SOCKET_H_INCLUDED
#define WEB_SOCKET_H_INCLUDED

#define WEB_SOCKET_KEY_LEN_MAX 256
#define RESPONSE_HEADER_LEN_MAX 1024
#define REQUEST_HEADER_LEN_MAX 1024

void shakeHand(int connfd,const char *serverKey);
char * fetchSecKey(const char * buf);
char * computeAcceptKey(const char * buf);
char * analyData(const char * buf,const int bufLen);
char * packData(const char * message,unsigned long * len);
void response(const int connfd,const char * message);
unsigned long getTotalDataLength(const char * buf,const int bufLen);
#endif // WEB_SOCKET_H_INCLUDED

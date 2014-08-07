#ifndef BASE64_H_INCLUDED
#define BASE64_H_INCLUDED

char* base64_encode(const char* data, int data_len);
char *base64_decode(const char* data, int data_len);

#endif // BASE64_H_INCLUDED

/* tora.h*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PROXY       "127.0.0.1"
#define PROXYPORT    9050
#define USERNAME    "toraliz"
#define reqsize sizeof(struct socks5_request)
#define ressize sizeof(struct socks5_response)

#define SOCKS5_VERSION 0x05
#define SOCKS5_CMD_CONNECT 0x01
#define SOCKS5_ATYP_IPV4 0x01
#define SOCKS5_AUTH_NOAUTH 0x00

typedef unsigned char int8; 
typedef unsigned short int int16;
typedef unsigned int int32;


struct socks5_request {
    int8 version;
    int8 command;
    int8 reserved;
    int8 address_type;
    int32 dstip;
    int16 dstport;

};
typedef struct socks5_request S5Req;

struct socks5_response {
   int8 version;
   int8 reply;
   int8 reserved;
   int8 address_type;
   int32 bindip;
   int16 bindport;

};
typedef struct socks5_response S5Res;



S5Req *create_socks5_request(const char*, const int);
void execute_with_torsocks(int, char **);
int main(int,char**);

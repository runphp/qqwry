/**
 * 获取本机ip地址
 * 
 * 需要指定wsock32.lib
 * gcc需要指定libwsock32.a
 * @author runphp.net
 * @version 0.1
 */
 
#include <winsock2.h>
#include <wsipx.h>
//#include "wsnwlink.h"
#include <stdio.h>
 
#include "localIp.h"
 
int localIp(char *ip)
{
    ////////////////
    // Initialize windows sockets API. Ask for version 1.1
    //
    WORD wVersionRequested = MAKEWORD(1, 1);
    WSADATA wsaData;
    if (WSAStartup(wVersionRequested, &wsaData)) {
        printf("WSAStartup failed %s\n", WSAGetLastError());
        return -1;
    }
 
    //////////////////
    // Get host name.
    //
    char hostname[256];
    int res = gethostname(hostname, sizeof(hostname));
    if (res != 0) {
        printf("Error: %u\n", WSAGetLastError());
        return -1;
    }
 
    ////////////////
    // Get host info for hostname. 
    //
    struct hostent* pHostent = gethostbyname(hostname);
    if (pHostent==NULL) {
        printf("Error: %u\n", WSAGetLastError());
        return -1;
    }
 
 
    struct sockaddr_in sa;
    int nAdapter;
    char *addr;
    for (nAdapter=0; pHostent->h_addr_list[nAdapter]; nAdapter++) {
        memcpy ( &sa.sin_addr.s_addr, pHostent->h_addr_list[nAdapter],pHostent->h_length);
        
        addr =inet_ntoa(sa.sin_addr);
         // Output the machines IP Address.
      //  printf("Address: %s\n", addr =inet_ntoa(sa.sin_addr)); // display as string
    }
    memcpy(ip,addr,16);
    //////////////////
    // Terminate windows sockets API
    //
    WSACleanup();
    return 0;
}
/*
int main()
{
 
    char ip[16];
    localIp(ip);
    printf("%s",ip);
    return 0;
}*/
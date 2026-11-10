
#ifndef PROTOCOL_H
#define PROTOCOL_H
#pragma pack(1)

#define	__USE_BSD			/* Using BSD header */
#include <netinet/ip.h>		/* Internet Protocol */
#define	__FAVOR_BSD			/* Using BSD TCP header */
#include <netinet/tcp.h>	/* Transmission Control Protocol */
#include <string.h>			/* String operations */
#include <stdlib.h>			/* Standard library definitions */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>


#define WORD unsigned short //for tcp commands
#define BYTE char

//mainain client
#define QUERY_SRV_INFO 0x00			
#define MODIFY_LOG_LEVEL 0x01		

#define ADD_USER_REQ 0x01
#define ADD_USER_RES 0x02

#define DEL_USER_REQ 0x03
#define DEL_USER_RES 0x04

#define LIST_USER_REQ 0x05
#define LIST_USER_RES 0x06

//udp client
#define LOGIN_REQ 1
#define LOGIN_RES 2

#define CLISRV_REQ 3
#define CLISRV_RES 4

#define P2P_REQ 5
#define P2P_RES 6

#define P2P_CHAT_REQ 7
#define P2P_CHAT_RES 8


#define GROUP_CREATE_REQ  9
#define GROUP_CREATE_RES 10

#define GROUP_REQ 11
#define GROUP_RES 12

#define GROUP_CHAT_REQ 13
#define GROUP_CHAR_RES 14

/*
int cmdid; ID DO COMANDO EXECUTADO (udpclient)
int res; 0->SUCESSO 1->ERRO
WORD wCmd_id; ID DOS COMANDOS EXECUTADOS(tcpclient)
*/

typedef struct
{
	BYTE bVersion;
	WORD wCmd_id;
	char username[24];
	char ip[24];
	char userpasswd[24];
	char isCliSvr[8];
	char isP2p[8];
	char isGroup[8];
}ReqAddUserInfo;

typedef struct
{
	BYTE bVersion;
	WORD wCmd_id;
	int res;
}ResAddUserInfo;

typedef struct
{
	BYTE bVersion;
	WORD wCmd_id;
	char username[24];
}ReqDelUserInfo;

typedef struct
{
	BYTE bVersion;
	WORD wCmd_id;
	int res;
}ResDelUserInfo;

typedef struct
{
	BYTE bVersion;
	WORD wCmd_id;
	char username[24];
}ReqListInfo;

typedef struct
{
	char username[24];
	char ip[24];
	char userpasswd[24];
	char isCliSvr[8];
	char isP2p[8];
	char isGroup[8];
}ResListInfo;

typedef struct
{
	BYTE bVersion;
	WORD wCmd_id;
	ResListInfo arrListInfo[20];
	int cnt;
}ResListData;

typedef struct 
{
	int cmdid;
	char username[24];
	char passwd[24];
}StReqLoginMsg;

typedef struct 
{
	int cmdid;
	int res;
	int timestamp; //Timestamp
	char username[24];
	char isCliSvr[8];
	char isP2p[8];
	char isGroup[8];
}StResLoginMsg;

typedef struct 
{
	int cmdid;
	char srcusername[24];
	char destusername[24];
	char msg[256];
}StReqCliSrvMsg;

typedef struct 
{
	int cmdid;
	int res;
	int timestamp; //Timestamp
}StResCliSrvMsg;


typedef struct
{
	char username[24];
	char ip[24];
	char userpasswd[24];
	char isCliSvr[8];
	char isP2p[8];
	char isGroup[8];
}StUserInfo;

typedef struct 
{
	int cmdid;
	char srcusername[24];
	char destusername[24];
}StReqP2PMsg;

typedef struct 
{
	int cmdid;
	int res;
	int timestamp; //Timestamp
	char srcusername[24];
	char destusername[24];
	char ip[24];
	int port;
}StResP2PMsg;

typedef struct 
{
	int cmdid;
	char srcusername[24];
	char destusername[24];
	char msg[256];
}StReqP2PChatMsg;

typedef struct 
{
	int cmdid;
	int res;
	int timestamp; //Timestamp
	char srcusername[24];
	char destusername[24];
	char ip[24];
	int port;
}StResP2PChatMsg;


typedef struct 
{
	int cmdid;
	char groupname[24];
	char username[24];
}StReqCreateGroupMsg;

typedef struct 
{
	int cmdid;
	int res;
	int timestamp; //Timestamp
	char ip[24];
	unsigned short port;
}StResCreateGroupMsg;


typedef struct
{
	char ip[24];
	char groupname[24];
	char createusername[24];
	int port;
	int timestamp; //Timestamp
}StGroupInfo;

typedef struct 
{
	int cmdid;
	char snnder[24];
	char msg[256];
}StReqGroupChatMsg;


#endif


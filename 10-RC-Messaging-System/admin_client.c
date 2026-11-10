
#pragma pack(1)
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <malloc.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <math.h>
#include <signal.h>
#include <sys/epoll.h>
#include <errno.h>
#include <stdarg.h>
#include <mcheck.h>
#include "protocol.h"

static int ProcUserListMsg(void *pData)
{
	ResListData *resListData = (ResListData *)pData; //resposta do server para o comando list

	printf("PRINTING LIST OF USERS:\nUsers listed:%d\n", resListData->cnt);
	int i = 0;
	//Print user list message
	for (i = 0; i < resListData->cnt; i++)
	{
		printf("[i][username]=%s [userpasswd]=%s"
			   " [ip]=%s [isCliSvr]=%s [isP2p]=%s [isGroup]=%s\n",i+1,
			   resListData->arrListInfo[i].username,
			   resListData->arrListInfo[i].userpasswd,
			   resListData->arrListInfo[i].ip, resListData->arrListInfo[i].isCliSvr,
			   resListData->arrListInfo[i].isP2p, resListData->arrListInfo[i].isGroup);
	}
	printf("Done\n");
	return 0;
}

static int ProcessMaintainMsg(WORD wCmd_id, void *pData)
{
	int nRet = 0;

	switch (wCmd_id)
	{
	case LIST_USER_RES:
		nRet = ProcUserListMsg(pData); //Process user list messages
		break;
	default:
		break;
	}

	return nRet;
}

int main(int argc, char **argv)
{
	int nClient_sock = 0;
	int nRet = 0;
	int nSnd_len = 0;
	int nRecv_len = 0;
	char *pData = NULL;
	WORD wCmd_id = 0;

	int nPort = 0;

	if (argc < 3) //handle not enough args
	{
		printf("./maintain_client ip port\n");
		return 0;
	}

	nPort = atoi(argv[2]); //get port
	printf("CONNECTING: [server ip]=%s [server port]=%d\n", argv[1], nPort);

	struct sockaddr_in srvAddr;
	memset(&srvAddr, 0, sizeof(srvAddr));
	nClient_sock = socket(AF_INET, SOCK_STREAM, 0); //socket tcp (sock_stream)

	if (-1 == nClient_sock)
	{
		printf("main: Call socket error create socket fail\n");
		return -1;
	}

	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons((WORD)nPort);
	srvAddr.sin_addr.s_addr = inet_addr(argv[1]);

	// connect server
	nRet = connect(nClient_sock, (struct sockaddr *)&srvAddr, sizeof(srvAddr));
	if (nRet != 0)
	{
		printf("main: Call connect error connect server fail");
		return -1;
	}

	printf("CONNECT: Connection to server succeeded\n");
	char line[256];
	char *token;
	int num = 0;
	memset(line, 0, sizeof(line));
	ReqAddUserInfo reqAddUserInfo;
	ReqDelUserInfo reqDelUserInfo;
	ReqListInfo reqListInfo;
	printf("INTRODUZIR COMANDOS\n");
	memset(line, 0, sizeof(line));

	//Get data from the command line
	while (read(0, line, sizeof(line)) > 0) // read file line by line
	{
		int len = strlen(line);
		if (line[len - 1] == '\n')
			line[len - 1] = 0;
		num = 0;
		if (strstr(line, "ADD") != NULL) //Add user command
		{
			memset(&reqAddUserInfo, 0, sizeof(reqAddUserInfo));
			token = strtok(line, " "); //Handling of strings with spaces as separators
			while (token)
			{
				if (1 == num) //As long as the second string
				{
					snprintf(reqAddUserInfo.username, sizeof(reqAddUserInfo.username),
							 "%s", token); //get username
				}
				else if (2 == num)
				{
					snprintf(reqAddUserInfo.ip, sizeof(reqAddUserInfo.ip),
							 "%s", token); //get ip
				}
				else if (3 == num)
				{
					snprintf(reqAddUserInfo.userpasswd, sizeof(reqAddUserInfo.userpasswd),
							 "%s", token); //get passwd
				}
				else if (4 == num)
				{

					snprintf(reqAddUserInfo.isCliSvr, sizeof(reqAddUserInfo.isCliSvr),
							 "%s", token); //get cs permission
				}
				else if (5 == num)
				{

					snprintf(reqAddUserInfo.isP2p, sizeof(reqAddUserInfo.isP2p),
							 "%s", token); //get p2p permission
				}
				else if (6 == num)
				{

					snprintf(reqAddUserInfo.isGroup, sizeof(reqAddUserInfo.isGroup),
							 "%s", token); //get group permission
				}

				token = strtok(NULL, " ");
				num++;
			}

			reqAddUserInfo.bVersion = 1;
			reqAddUserInfo.wCmd_id = htons(ADD_USER_REQ);
			nSnd_len = sizeof(reqAddUserInfo);
			nSnd_len = htonl(nSnd_len);
			nRet = send(nClient_sock, (char *)&nSnd_len, sizeof(int), 0);				   //send length to server
			nRet = send(nClient_sock, (char *)&reqAddUserInfo, sizeof(reqAddUserInfo), 0); //send msg to server
			nRet = recv(nClient_sock, (char *)&nRecv_len, sizeof(int), 0);				   //recg length

			nRecv_len = ntohl(nRecv_len);
			pData = (char *)malloc(nRecv_len);
			if (NULL == pData)
			{
				printf("ADMIN ERROR: Call malloc error\n");
				return 0;
			}

			printf("COMPRIMENTO MENSAGEM RECEBIDA %d\n", nRecv_len);
			recv(nClient_sock, pData, nRecv_len, 0); //recv mesg

			ResAddUserInfo *res = (ResAddUserInfo *)pData;
			if (ntohl(res->res) == 0)
			{

				printf("ADD BEM SUCEDIDO\n");
			}
			if (NULL != pData)
			{
				free(pData);
				pData = NULL;
			}
		}
		else if (strstr(line, "DEL") != NULL) //process delete user message
		{
			memset(&reqDelUserInfo, 0, sizeof(reqDelUserInfo));
			num = 0;
			token = strtok(line, " "); //Handling of strings with spaces as separators
			while (token)
			{
				if (1 == num) //As long as the second string
				{
					//delete username
					snprintf(reqDelUserInfo.username, sizeof(reqDelUserInfo.username),
							 "%s", token);
				}
				token = strtok(NULL, " ");
				num++;
			}

			reqDelUserInfo.bVersion = 1;
			reqDelUserInfo.wCmd_id = htons(DEL_USER_REQ);
			nSnd_len = sizeof(reqDelUserInfo);
			nSnd_len = htonl(nSnd_len);
			nRet = send(nClient_sock, (char *)&nSnd_len, sizeof(int), 0);
			nRet = send(nClient_sock, (char *)&reqDelUserInfo, sizeof(reqDelUserInfo), 0); //send msg length
			nRet = recv(nClient_sock, (char *)&nRecv_len, sizeof(int), 0);				   //send msg

			nRecv_len = ntohl(nRecv_len); //recv length
			pData = (char *)malloc(nRecv_len);
			if (NULL == pData)
			{
				printf("main: Call malloc error\n");
				return 0;
			}

			recv(nClient_sock, pData, nRecv_len, 0); //recv message
			ResDelUserInfo *res = (ResDelUserInfo *)pData;
			if (ntohl(res->res) == 0)
			{

				printf("DELETE: BEM SUCEDIDO\n");
			}
			if (NULL != pData)
			{
				free(pData);
				pData = NULL;
			}
		}
		else if (strstr(line, "LIST") != NULL) //get user list
		{
			memset(&reqListInfo, 0, sizeof(reqListInfo));
			reqListInfo.bVersion = 1;
			reqListInfo.wCmd_id = htons(LIST_USER_REQ);
			snprintf(reqListInfo.username, sizeof(reqListInfo.username), "%s", "test");
			nSnd_len = sizeof(reqListInfo);
			nSnd_len = htonl(nSnd_len);
			nRet = send(nClient_sock, (char *)&nSnd_len, sizeof(int), 0);

			nRet = send(nClient_sock, (char *)&reqListInfo, sizeof(reqListInfo), 0);

			nRet = recv(nClient_sock, (char *)&nRecv_len, sizeof(int), 0); //comprimento msg rcebida

			nRecv_len = ntohl(nRecv_len);
			pData = (char *)malloc(nRecv_len);
			if (NULL == pData)
			{
				printf("ADMIN ERROR:Call malloc error\n");
				return 0;
			}

			printf("ADMIN: comprimento da mensagem recebida %d\n", nRecv_len);
			recv(nClient_sock, pData, nRecv_len, 0);
			wCmd_id = *(WORD *)(pData + 1);
			wCmd_id = ntohs(wCmd_id);

			printf("ADMIN: COMMAND ID: %d\n", wCmd_id);
			ProcessMaintainMsg(wCmd_id, (void *)pData);

			if (NULL != pData)
			{
				free(pData);
				pData = NULL;
			}
		}
		else if (strstr(line, "QUIT") != NULL) //Exit interface
		{
			close(nClient_sock);
			return 0;
		}

		printf("INTRODUZIR COMANDOS\n");
		memset(line, 0, sizeof(line));
	}
	return 0;
}

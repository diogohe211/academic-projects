#include "maintain_thd.h"
#include <stdio.h>
extern int mainport;
#pragma pack(1)

extern char userfilename[64];

#define WORD unsigned short
//liberta memoria alocada 
#define FREEIF(p) \
	if (p)        \
	{             \
		free(p);  \
		p = 0;    \
	}

//The size of the largest received data each time
#define RECV_LEN 2048

int InitListenSockInfo(int *pListenSock, WORD wPort)
{
	*pListenSock = socket(AF_INET, SOCK_STREAM, 0); //TCP SOCKET
	if (*pListenSock < 0)
	{
		return -1;
	}

	int nVal = 1;

	//Set the socket as a reusable port
	if (setsockopt(*pListenSock, SOL_SOCKET, SO_REUSEADDR, &nVal, sizeof(nVal)) != 0)
	{
		return -1;
	}
	//linger
	//*se existe dados quando close() é chamado close() é bloqueado até todos os dados serem transmitidos
	struct linger lin;
	lin.l_onoff = 1;  //Turn on the linegr switch-> *
	lin.l_linger = 0; // Set the delay time to 0 seconds, TCPIP is closed immediately, but another may appear
	int nRet = setsockopt(*pListenSock, SOL_SOCKET, SO_LINGER, (char *)&lin, sizeof(lin));
	if (nRet != 0)
	{
		return -1;
	}

	struct sockaddr_in srv_addr;
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	srv_addr.sin_port = htons(wPort);

	if (-1 == bind(*pListenSock, (struct sockaddr *)&srv_addr, sizeof(srv_addr)))
	{
		printf("TCP: Bind Error\n");
		return -1;
	}

	if (-1 == listen(*pListenSock, 5))
	{
		printf("Listen: Call listen error error[%d]=%s\n", errno, strerror(errno));
		return -1;
	}

	return 0;
}

//Function purpose: Receive message function
//Input parameters: socket id, message,the length of the received message
//Output parameter: the cache to store the message
//Return value: if the reception is successful, return TRUE, if the reception fails, return FALSE
int Recv(int nSock, char *pMsg, int nLen)
{
	int nTotal_len = nLen;
	int nRecv_len = 0;
	int nRet = 0;
	int nRemain_len = nTotal_len - nRecv_len;

	//Receiving messages in a continuous loop until the receiving message is completed
	while (nRemain_len > 0)
	{
		nRemain_len = nRemain_len > RECV_LEN ? RECV_LEN : nRemain_len; //
		nRet = recv(nSock, pMsg + nRecv_len, nRemain_len, 0);
		if (-1 == nRet || 0 == nRet)
		{
			printf("Recv: Call recv error [return value]=%d error[%d]=%s\n", nRet, errno, strerror(errno));
			return nRet;
		}

		nRecv_len += nRet;
		nRemain_len = nTotal_len - nRecv_len;
	}

	return nRecv_len;
}

//Return user information to the client
int ProcAddUserMsg(int nSock, void *pData)
{
	FILE *fp = fopen(userfilename, "a"); //Open the user file to append
	char line[1024];

	//get username, password, ip, can send p2p message create group or not , send p2p message or not
	ReqAddUserInfo *padduser_info = (ReqAddUserInfo *)pData; // struc com info do user a adicionar
	//adiciona ao ficheiro uma linha com a info do user registado
	snprintf(line, sizeof(line), "%s %s %s %s %s %s\n", padduser_info->username, padduser_info->ip, padduser_info->userpasswd, padduser_info->isCliSvr, padduser_info->isP2p, padduser_info->isGroup);
	fwrite(line, 1, strlen(line), fp);
	fclose(fp);

	ResAddUserInfo res;
	res.bVersion = 1;
	res.wCmd_id = htons(ADD_USER_RES); //id-comando add
	res.res = htonl(0);
	int nLen = sizeof(ResAddUserInfo);
	nLen = htonl(nLen);

	send(nSock, (char *)&nLen, sizeof(int), 0); //enviou n de argumentos do commando (conta com ADD)
	send(nSock, (char *)&res, sizeof(res), 0);	//enviou mensagem
	return 0;
}

//Process delete user request
int ProcDelUserMsg(int nSock, void *pData)
{

	char line[40][1024];
	ReqDelUserInfo *pReqDelUserInfo = (ReqDelUserInfo *)pData; //data do user a levar delete
	int cnt = 0;
	char tmp[1024];
	FILE *fp = fopen(userfilename, "r+"); //Open the user file
	while (!feof(fp))
	{
		memset(tmp, 0, sizeof(tmp));
		fgets(tmp, sizeof(tmp), fp);
		if (strstr(tmp, pReqDelUserInfo->username) != NULL) //User to delete
		{
			continue; //skip to next iteration (do not save on array this line)
		}

		snprintf(line[cnt], sizeof(line[cnt]), "%s", tmp); //store line in array if user_to_delete!=user_in_line
		cnt++;
	}

	ftruncate(fileno(fp), 0); //Empty file
	fseek(fp, 0L, SEEK_SET);

	//Rewrite users não apagados
	printf("Users to rewrite(count): %d\n", cnt);
	for (int i = 0; i < cnt; i++)
	{
		fwrite(line[i], 1, strlen(line[i]), fp); //Rewrite file
	}
	fclose(fp);

	ResDelUserInfo res;
	res.bVersion = 1;
	res.wCmd_id = htons(DEL_USER_RES);
	res.res = htonl(0);
	int nLen = sizeof(ResAddUserInfo);
	nLen = htonl(nLen);

	send(nSock, (char *)&nLen, sizeof(int), 0); //enviou n de argumentos do commando (conta com ADD)
	send(nSock, (char *)&res, sizeof(res), 0);	//enviou mensagem
	return 0;
}

//Retira info da linha e coloca na estrutura
int ParseLineData(char *line, StUserInfo *stUserInfo)
{
	int len = strlen(line);
	if (line[len - 1] == '\n')
	{
		line[len - 1] = 0;
	}
	int num = 0;
	char *token;
	token = strtok(line, " "); //Handling of strings with spaces as separators
	//Parse a row of data in the configuration file
	while (token)
	{
		if (0 == num) //As long as the second string
		{
			snprintf(stUserInfo->username, sizeof(stUserInfo->username),
					 "%s", token); //user name
		}
		else if (1 == num)
		{
			snprintf(stUserInfo->ip, sizeof(stUserInfo->ip),
					 "%s", token); //ip
		}
		else if (2 == num)
		{
			snprintf(stUserInfo->userpasswd, sizeof(stUserInfo->userpasswd),
					 "%s", token); //passwd
		}
		else if (3 == num)
		{

			snprintf(stUserInfo->isCliSvr, sizeof(stUserInfo->isCliSvr),
					 "%s", token); //is have CliSvr power
		}
		else if (4 == num)
		{

			snprintf(stUserInfo->isP2p, sizeof(stUserInfo->isP2p),
					 "%s", token); //is have P2p power
		}
		else if (5 == num)
		{

			snprintf(stUserInfo->isGroup, sizeof(stUserInfo->isGroup),
					 "%s", token); //is have group power
		}

		token = strtok(NULL, " ");
		num++;
	}
	return 0;
}

//Handle the get user list message
int ProcListUserMsg(int nSock)
{

	int cnt = 0;
	char line[1024];

	ResListData resListData;
	StUserInfo stUserInfo;
	if ((access(userfilename, F_OK)) == -1) //Se file n existe return to client
	{
		resListData.bVersion = 1;
		resListData.wCmd_id = htons(LIST_USER_RES);
		resListData.cnt = cnt;
		int nLen = sizeof(resListData);
		nLen = htonl(nLen);
		send(nSock, (char *)&nLen, sizeof(int), 0);				   //enviou n de argumentos do commando (conta com ADD)
		send(nSock, (char *)&resListData, sizeof(resListData), 0); //send resposta
		return -1;
	}

	FILE *fp = fopen(userfilename, "r"); //Open the file

	while (!feof(fp))
	{
		memset(line, 0, sizeof(line));
		fgets(line, sizeof(line), fp);
		int len = strlen(line);
		if ((1 == len && line[len - 1] == '\n') || 0 == len) //se linha está fazia vai para a próxima iteração do while
			continue;
		ParseLineData(line, &stUserInfo); //Get user-like data
		//username stored in response struc
		snprintf(resListData.arrListInfo[cnt].username, sizeof(resListData.arrListInfo[cnt].username), "%s", stUserInfo.username);
		//ip stored in response struc
		snprintf(resListData.arrListInfo[cnt].ip, sizeof(resListData.arrListInfo[cnt].ip), "%s", stUserInfo.ip);
		//password stored in response struc
		snprintf(resListData.arrListInfo[cnt].userpasswd, sizeof(resListData.arrListInfo[cnt].userpasswd), "%s", stUserInfo.userpasswd);
		// client-server permission stored in response struc
		snprintf(resListData.arrListInfo[cnt].isCliSvr, sizeof(resListData.arrListInfo[cnt].isCliSvr), "%s", stUserInfo.isCliSvr);
		// p2p permission stored in response struc
		snprintf(resListData.arrListInfo[cnt].isP2p, sizeof(resListData.arrListInfo[cnt].isP2p), "%s", stUserInfo.isP2p);
		// group permission stored in response struc
		snprintf(resListData.arrListInfo[cnt].isGroup, sizeof(resListData.arrListInfo[cnt].isGroup), "%s", stUserInfo.isGroup);
		cnt++;
		if (cnt >= 20)
			break;
	}

	fclose(fp);
	resListData.bVersion = 1;
	resListData.wCmd_id = htons(LIST_USER_RES);
	resListData.cnt = cnt;
	int nLen = sizeof(resListData);
	nLen = htonl(nLen);
	send(nSock, (char *)&nLen, sizeof(int), 0);				   //Return data to the client
	send(nSock, (char *)&resListData, sizeof(resListData), 0); //Enviou o array de users criado a partir do fich
	return 0;
}

static int ProcessMaintainMsg(WORD wCmd_id, int nSock, void *pData)
{
	if (NULL == pData)
	{
		printf("ProcessMaintainMsg: func param error\n");
		return -1;
	}

	int nRet = 0;

	printf("ProcessMaintainMsg: [command id]=%d [socket id]=%d\n", wCmd_id, nSock);
	switch (wCmd_id)
	{

	case ADD_USER_REQ:
		nRet = ProcAddUserMsg(nSock, pData); //Process add user message
		break;

	case DEL_USER_REQ:
		nRet = ProcDelUserMsg(nSock, pData); //Processing delete user message
		break;
	case LIST_USER_REQ:
		nRet = ProcListUserMsg(nSock); //Handle the get user list message
		break;
	default:
		break;
	}

	return nRet;
}

void *RunMaintainThread(void)
{
	int nListen_sock = 0;
	int nClient_sock = 0;
	int nLen = 0;
	int nRet = 0;
	char *pData = NULL;
	WORD wCmd_id = 0;
	if (InitListenSockInfo(&nListen_sock, mainport)) 
	{
		//FAILED TO LISTEN TO SOCK
		pthread_exit(NULL);
		return NULL;
	}

	while (1)
	{
		nClient_sock = accept(nListen_sock, NULL, NULL); //Accept connection
		if (nClient_sock < 0)
		{
			printf("RunMaintainThread: Call accept error so sleep 3s then accept client connection request again\n");
			sleep(3);
			continue;
		}

		printf("RunMaintainThread: maintain thread accept client connection request succeed\n");

		while (1)
		{
			nRet = Recv(nClient_sock, (char *)&nLen, sizeof(int)); //Message length received first
			if (nRet <= 0)
			{
				printf("RecvMaintainClientMsg: Call Recv error\n");
				close(nClient_sock);
				break;
				;
			}

			nLen = ntohl(nLen);
			pData = (char *)malloc(nLen);
			if (NULL == pData)
			{
				printf("RecvMaintainClientMsg: Call malloc error\n");
				close(nClient_sock);
				break;
			}

			nRet = Recv(nClient_sock, pData, nLen); //Receive data and store in pData
			if (nRet <= 0)
			{
				printf("RecvMaintainClientMsg: Call Recv error");
				close(nClient_sock);
				break;
			}

			wCmd_id = *(WORD *)(pData + 1); //Get command id
			wCmd_id = ntohs(wCmd_id);

			printf("RecvMaintainClientMsg: recv client message [data len]=%d [command id]=%d\n", nLen, wCmd_id);
			ProcessMaintainMsg(wCmd_id, nClient_sock, (void *)pData);
			FREEIF(pData);
		}
	}

	return NULL;
}

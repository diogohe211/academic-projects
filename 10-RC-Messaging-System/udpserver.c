//#pragma pack(1)
#include "maintain_thd.h"
#include "protocol.h"
#include "hash.h"

int mainport = 0;
char userfilename[64];
int fd;

/* msg buf queue item*/
typedef struct
{
	char msg[1024];	  // client data
	long nlen;		  // data len 
	time_t recv_time; // trade expire time 
	int cmdid; 		  //idcommand
	int needrecv; 	  
	char username[24];
	struct sockaddr_in conninfo; //address
} msg_item;

StHash_Table *loninInfoTable; //store logins
StHash_Table *multicastTable; //store group ips and names
int group_id = 1;
int group_port = 8000;

//envia msg para cliente
int sndMsg(char *msg, int len, struct sockaddr_in *client_addr)
{
	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0)
	{
		printf("socket() error[%s].\n", strerror(errno));
		return -1;
	}

	if (sendto(sock, msg, len, 0, (struct sockaddr *)client_addr, sizeof(*client_addr)) <= 0)
	{
		printf("sendto() ret[%d] error[%d][%s].\n", errno, strerror(errno));
	}

	close(sock);
	return 0;
}

//Handling login messages
int ProcLoginMsg(struct sockaddr_in *client_addr, char *sRecvBuff, msg_item *item)
{
	StReqLoginMsg *data = (StReqLoginMsg *)sRecvBuff;
	printf("debug: username=%s\n", data->username);
	StResLoginMsg resmsg;
	resmsg.cmdid = htonl(LOGIN_RES); //command id 
	resmsg.timestamp = htonl(time((time_t *)NULL));
	resmsg.res = htonl(0);
	char line[1024];
	StUserInfo stUserInfo; //struc informação do user
	int find = 0; //encontrou (s/n)
	memset(line, 0, sizeof(line));
	FILE *fp = fopen(userfilename, "r"); //Open the user file
	while (!feof(fp))
	{
		fgets(line, sizeof(line), fp);//get linha
		ParseLineData(line, &stUserInfo); //guarda info da linha na struc
		//validação do login- compara info da linha com info recebida do client
		if (0 == strcmp(data->username, stUserInfo.username) && 0 == strcmp(data->passwd, stUserInfo.userpasswd))
		{
			snprintf(resmsg.username, sizeof(resmsg.username),"%s", stUserInfo.username); //get user name
			snprintf(resmsg.isCliSvr, sizeof(resmsg.isCliSvr),"%s", stUserInfo.isCliSvr); //is have clisvr power
			snprintf(resmsg.isP2p, sizeof(resmsg.isP2p),"%s", stUserInfo.isP2p); // is have p2p power
			snprintf(resmsg.isGroup, sizeof(resmsg.isGroup),"%s", stUserInfo.isGroup); //is have create group power
			find = 1;
			break;
		}

		memset(line, 0, sizeof(line));
	}

	fclose(fp);
	if (0 == find) //If user is not allowed to log in
	{
		resmsg.res = htonl(1); //se client ler 1 faz print de login errado(no client)
	}

	sndMsg((char *)&resmsg, sizeof(resmsg), client_addr); //envia mensagem ao client
	snprintf(item->username, sizeof(item->username),"%s", stUserInfo.username);
	HashInsert(&loninInfoTable, stUserInfo.username, item);//guarda este login na hashtable
	return 0;
}

//Forward client server messages
int ProcCliSrvMsg(struct sockaddr_in *client_addr, char *sRecvBuff)
{
	StReqCliSrvMsg *data = (StReqCliSrvMsg *)sRecvBuff;
	StUserInfo stUserInfo;
	char line[1024];
	int err = 1;

	StResCliSrvMsg resmsg;
	resmsg.cmdid = htonl(CLISRV_RES);
	resmsg.timestamp = htonl(time((time_t *)NULL));
	resmsg.res = htonl(1); //1== user n tem permissão 
//PROCURA NO FICH USER PARA VERIFICAR SE TEM PERMISSÃO
	FILE *fp = fopen(userfilename, "r"); //Open the APPEND file
	while (!feof(fp))
	{
		fgets(line, sizeof(line), fp);
		ParseLineData(line, &stUserInfo); //Get user configuration information

		if (0 == strcmp(data->srcusername, stUserInfo.username))
		{
			if (0 == strcmp(stUserInfo.isCliSvr, "yes")) //User has permission to send
			{
				resmsg.res = htonl(0); //verificou que user tem permissão res=0
				err = 0;
				break;
			}
		}

		memset(line, 0, sizeof(line));
	}

	fclose(fp);

	if (1 == err) //User has not permission to send
	{
		//envia msg de erro de permissão
		sndMsg((char *)&resmsg, sizeof(resmsg), client_addr);
		return -1;
	}

	//procura na table a info do destinatario
	StHash_Item *pItem = HashGetItem(loninInfoTable, data->destusername);
	if (pItem != NULL) //encontrou destinatario
	{
		msg_item *pitem = (msg_item *)pItem->pMatch_msg;
		sndMsg((char *)sRecvBuff, sizeof(StReqCliSrvMsg), &pitem->conninfo); //Send a message to the other party
	}
	else //destinatario não registado
	{
		sndMsg((char *)&resmsg, sizeof(resmsg), client_addr); //Cannot find information Return error information
		return -1;
	}

	return 0;
}

//process p2p message
int ProcP2PMsg(struct sockaddr_in *client_addr, char *sRecvBuff)
{
	StReqP2PMsg *data = (StReqP2PMsg *)sRecvBuff;  
	StResP2PMsg resmsg; //Struc msg a enviar 
	resmsg.cmdid = htonl(P2P_RES);
	resmsg.timestamp = htonl(time((time_t *)NULL));
	resmsg.res = htonl(1); //encontrou dest- não
	snprintf(resmsg.srcusername, sizeof(resmsg.srcusername),"%s", data->srcusername); //Source user -> para struct
	snprintf(resmsg.destusername, sizeof(resmsg.destusername),"%s", data->destusername); //Send destination user -> para struct	
	StHash_Item *pItem = HashGetItem(loninInfoTable, data->destusername); //obtem info do destinatário da table 
	StUserInfo stUserInfo; //struc user info 
	char line[1024];
	int err = 1; //Permissão-não

//VERIFICAÇÃO DE PERMISSÃO PARA P2P
	FILE *fp = fopen(userfilename, "r"); //Open the APPEND file
	while (!feof(fp))
	{
		fgets(line, sizeof(line), fp);
		ParseLineData(line, &stUserInfo); 

		if (0 == strcmp(data->srcusername, stUserInfo.username))
		{
			if (0 == strcmp(stUserInfo.isP2p, "yes")) //The user has the function of sending p2p messages
			{
				resmsg.res = htonl(0);
				err = 0; //Permissão-sim
				break;
			}
		}

		memset(line, 0, sizeof(line));
	}

	fclose(fp);

	if (1 == err) //The user doesn't have permission 
	{
		sndMsg((char *)&resmsg, sizeof(resmsg), client_addr);
		return -1;
	}

	if (pItem != NULL) //Obteve destinatário na table
	{
		resmsg.res = htonl(0);
		msg_item *pitem = (msg_item *)pItem->pMatch_msg;
		//Return the other party's ip
		snprintf(resmsg.ip, sizeof(resmsg.ip), "%s", inet_ntoa(pitem->conninfo.sin_addr));//Other party's ip
		resmsg.port = pitem->conninfo.sin_port; //Return to the other party's port
	}
	//envia msg ao client
	sndMsg((char *)&resmsg, sizeof(resmsg), client_addr);
	return 0;
}

//Process the create group message
int ProcCreateGroupMsg(struct sockaddr_in *client_addr, char *sRecvBuff)
{
	StReqCreateGroupMsg *data = (StReqCreateGroupMsg *)sRecvBuff;  //msg recebida
	StGroupInfo *pGroupInfo = (StGroupInfo *)malloc(sizeof(StGroupInfo)); //struc create group
	pGroupInfo->timestamp = htonl(time((time_t *)NULL)); //timestamp
	snprintf(pGroupInfo->createusername, sizeof(pGroupInfo->createusername),"%s", data->username); //data-username->groupinfo-createusername
	snprintf(pGroupInfo->groupname, sizeof(pGroupInfo->groupname),"%s", data->groupname);//data-groupname->groupinfo-groupname
	snprintf(pGroupInfo->ip, sizeof(pGroupInfo->ip),"224.0.0.%d", group_id);//group ip = 224.0.0.i
	pGroupInfo->port = group_port; //group port =8000

	StUserInfo stUserInfo; //user in file line
	char line[1024];
	StResCreateGroupMsg resmsg; //msg to send
	resmsg.cmdid = htonl(GROUP_CREATE_RES); 
	resmsg.timestamp = htonl(time((time_t *)NULL));
	resmsg.res = htonl(0);
	snprintf(resmsg.ip, sizeof(resmsg.ip),"224.0.0.%d", group_id);
	resmsg.port = htons(group_port);
	group_port++; //Increment ip and port for next goup creation
	group_id++;

//VERIFICA PERMISSÃO DO USER PARA GRUPOS	
	FILE *fp = fopen(userfilename, "r"); //Open the APPEND file
	while (!feof(fp))
	{
		fgets(line, sizeof(line), fp);
		ParseLineData(line, &stUserInfo);

		if (0 == strcmp(data->username, stUserInfo.username))
		{
			if (0 == strcmp(stUserInfo.isGroup, "yes"))
			{
				resmsg.res = htonl(0);
				break;
			}
		}

		memset(line, 0, sizeof(line));
	}

	fclose(fp);
	sndMsg((char *)&resmsg, sizeof(resmsg), client_addr); //envia msg 
	HashInsert(&multicastTable, pGroupInfo->groupname, pGroupInfo); //Coloca na hash o nome e endereço do grupo
	return 0;
}

//Get group broadcast address
int ProcGroupMsg(struct sockaddr_in *client_addr, char *sRecvBuff)
{
	StReqCreateGroupMsg *data = (StReqCreateGroupMsg *)sRecvBuff; //data recieved
	StResCreateGroupMsg resmsg; //response message
	resmsg.cmdid = htonl(GROUP_RES); 
	resmsg.timestamp = htonl(time((time_t *)NULL));
	resmsg.res = htonl(1); //ainda n encontrou grupo
	StHash_Item *pItem = HashGetItem(multicastTable, data->groupname); //procura na hashtable o grupo pelo nome(key)
	if (pItem != NULL) //Encontrou a chave do grupo
	{
		StGroupInfo *pGroupInfo = (StGroupInfo *)pItem->pMatch_msg; //info do grupo ao qual se pretende enviar mensagem
		snprintf(resmsg.ip, sizeof(resmsg.ip), "%s", pGroupInfo->ip); //ip group to response msg
		resmsg.port = htons(pGroupInfo->port);//group port to response msg
		resmsg.res = htonl(0);//encontrou grupo
	}

	sndMsg((char *)&resmsg, sizeof(resmsg), client_addr);//envia mensagem
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 4) //handle falta de argumentos
	{
		printf("./udpserver portclients portconfig logfile\n");
		return -1;
	}
	int iRecvLen = 0; //length da msg recebida
	struct sockaddr_in my_addr; //addr do server
	char sClientIp[16];  //ip client
	struct sockaddr_in client_addr; //socket
	socklen_t cli_len = sizeof(client_addr); //addr size

	if (HashInit(&loninInfoTable, 32, NULL)) //Inicializa hashtable dos registos
	{
		printf("main: Call YDT_Hash_Init error\n");
		return -1;
	}

	if (HashInit(&multicastTable, 32, NULL)) //Inicializa hashtable dos gupos
	{
		printf("main: Call YDT_Hash_Init error\n");
		return -1;
	}

	mainport = atoi(argv[2]); //Save client port
	pthread_t thread_id = 0; //thread admin
	//Create admin thread
	int nRet = pthread_create(&thread_id, NULL, RunMaintainThread, NULL); //Create admin thread
	if (nRet != 0)
	{
		printf("Couldn't create thread\n");
		return -1;
	}

	snprintf(userfilename, sizeof(userfilename), "%s", argv[3]); //User file information

	printf("udpserver server start...\n");
	int listener;
	// Enable socket monitoring
	if ((listener = socket(PF_INET, SOCK_DGRAM, 0)) == -1)//socket udp
	{
		printf("error: socket Failed to create\n");
		return -1;
	}
	else
	{
		printf("Debug: Socket Created successfully\n");
	}

	//Set the socket attribute so that the port can be reused
	int opt = SO_REUSEADDR;
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	char sRecvBuff[1024];

	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = PF_INET; 
	my_addr.sin_port = htons(atoi(argv[1]));
	my_addr.sin_addr.s_addr = INADDR_ANY;
	
	//bind sock to addr
	if (bind(listener, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
	{
		printf("UDP:Bind error");
		return -1;
	}
	else
	{
		printf("Debug: IP Address and port binding successfully");
	}

	msg_item *item;

	while (1)
	{
		//leitura de mensagens
		iRecvLen = recvfrom(listener, (char *)sRecvBuff, sizeof(sRecvBuff), 0, (struct sockaddr *)&client_addr, &cli_len);
		
		//existe mensagens para ler 
		if (iRecvLen > 0)
		{
			memset(sClientIp, 0x00, sizeof(sClientIp)); 
			strncpy(sClientIp, inet_ntoa(client_addr.sin_addr), sizeof(sClientIp));//Obtém ip do cliente que se conectou
			int iClientPort = ntohs(client_addr.sin_port); //save client port
			printf("Debug: Socket %d Received a message from: %s:%d successfully, a total of %d bytes of data\n", listener, sClientIp, iClientPort, iRecvLen);

			int cmdid = *(int *)(sRecvBuff); // comando a executar
			cmdid = ntohl(cmdid); 
			printf("Debug: ID-Comando:%d\n", cmdid);
			
			if (LOGIN_REQ == cmdid) //Handling login messages
			{
				//cria item  com tamanho da msg, id do comando 
				item = (msg_item *)malloc(sizeof(msg_item));
				item->recv_time = time(NULL);
				item->nlen = iRecvLen;
				item->cmdid = cmdid;
				item->needrecv = 0;
				memcpy(item->msg, sRecvBuff, item->nlen);
				memcpy(&item->conninfo, &client_addr, sizeof(item->conninfo));
				ProcLoginMsg(&client_addr, sRecvBuff, item);
			}
			else if (CLISRV_REQ == cmdid) //Handle the function of sending client-server messages
			{
				ProcCliSrvMsg(&client_addr, sRecvBuff);
			}
			else if (P2P_REQ == cmdid) //Get the other party's ip and port
			{
				ProcP2PMsg(&client_addr, sRecvBuff);
			}
			else if (GROUP_CREATE_REQ == cmdid) //Create group
			{
				ProcCreateGroupMsg(&client_addr, sRecvBuff);
			}
			else if (GROUP_REQ == cmdid) //Request group ip and port
			{
				ProcGroupMsg(&client_addr, sRecvBuff);
			}
		}
		else
		{
			printf("ERROR: recvfrom() iRecvLen[%d]", iRecvLen);
		}
	}

	HashFree(&multicastTable, FreeFunc); //Release memory from multicast hashtable
	HashFree(&loninInfoTable, FreeFunc); //Release memory from logins hashtable

	return 0;
}

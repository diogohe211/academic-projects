#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/syscall.h>
#include <sys/un.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <errno.h>
#include <time.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <netinet/in.h>
#include "protocol.h"
#pragma pack(1)

char username[24];
char destusername[24];
char passwd[24];
int socket_fd = 0;
int group_fd = 0;
struct sockaddr_in client_addr;
struct sockaddr_in group_addr;

//Get login information
int GetLoginInfo(char *line)
{
	int num = 0;
	char *token;
	token = strtok(line, " "); //Handling of strings with spaces as separators
	while (token)
	{
		if (1 == num)
		{
			snprintf(username, sizeof(username), "%s", token); //Get username
		}
		else if (2 == num)
		{
			snprintf(passwd, sizeof(passwd), "%s", token); //Get password
		}

		token = strtok(NULL, " ");
		num++;
	}
	return 0;
}

//Get server forwarding information
int GetCsInfo(char *line, char *destuser, int destlen, char *sndmsg)
{
	int num = 0;
	char *token;
	token = strtok(line, " "); //Handling of strings with spaces as separators
	while (token)
	{
		if (1 == num)
		{
			snprintf(destuser, destlen, "%s", token); //Get the target user
		}
		else if (num >= 2) //Get sent message
		{
			strcat(sndmsg, token);
			strcat(sndmsg, " ");
		}

		token = strtok(NULL, " ");
		num++;
	}
	return 0;
}

//Obtain user information for p2p chat purposes
int GetP2PReqInfo(char *line, char *destuser, int destlen)
{
	int num = 0;
	char *token;
	token = strtok(line, " "); //Handling of strings with spaces as separators
	while (token)
	{
		if (1 == num)
		{
			snprintf(destuser, destlen, "%s", token);
			snprintf(destusername, destlen, "%s", token); //Get the target user
		}

		token = strtok(NULL, " ");
		num++;
	}

	return 0;
}

//p2p chat
//Get sent message
int GetP2PChatInfo(char *line, char *msg)
{
	int num = 0;
	char *token;
	token = strtok(line, " "); //Handling of strings with spaces as separators
	while (token)
	{
		if (num >= 1)
		{
			strcat(msg, token);
			strcat(msg, " ");
		}

		token = strtok(NULL, " ");
		num++;
	}

	return 0;
}

int GetGroupRecvSock(char *ip, unsigned short port)
{
	int s;
	struct sockaddr_in srv;

	struct ip_mreq mreq;

	bzero(&srv, sizeof(srv));
	srv.sin_family = AF_INET;
	srv.sin_addr.s_addr = htonl(INADDR_ANY); //endereço para receber mensagens
	//srv.sin_port=htons(port);
	srv.sin_port = port;

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Opening   socket ");
		return 0;
	}

	if (bind(s, (struct sockaddr *)&srv, sizeof(srv)) < 0)
	{
		perror("bind ");
		return 0;
	}

	mreq.imr_multiaddr.s_addr = inet_addr(ip);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) //join multicast
	{
		perror("setsockopt:   IP_ADD_MEMBERSHIP ");
		return 0;
	}

	return s;
}

int ProMsg(char *server_reply) //print respostas do server
{
	int cmdid = ntohl(*(int *)server_reply);
	//printf("ProMsg: cmdid=%d\n", cmdid); //id comando
	if (LOGIN_RES == cmdid) //login
	{
		StResLoginMsg *res = (StResLoginMsg *)server_reply; //Info recebida pelo server
		if (0 != ntohl(res->res)) //se ==1 login errado 
		{
			printf("SERVER: Username ou Password incorretos\n");
		}
		else
		{
			//print info lida do .txt pelo server
			//printf("ProMsg: cmdid=%d  timestamp=%d res=%d username=%s isCliSvr=%s isP2p=%s isGroup=%s\n",ntohl(res->cmdid), ntohl(res->timestamp), ntohl(res->res),res->username, res->isCliSvr, res->isP2p, res->isGroup);
			printf("[CONECTADO]\n Bem-vindo %s, as suas permissões são:\n\tClient-Server->%s\n\tP2PChat=%s\n\tGroupChating=%s\n", res->username, res->isCliSvr, res->isP2p, res->isGroup);
		}
	}
	else if (CLISRV_REQ == cmdid) //receber mensagem client-server
	{
		//printf("CLISRV_REQ\n");
		StReqCliSrvMsg *res = (StReqCliSrvMsg *)server_reply;
		printf("CLIENT-SERVER: Remetente=%s mensagem=%s\n", res->srcusername, res->destusername, res->msg);
	}
	else if (CLISRV_RES == cmdid)
	{
		StResCliSrvMsg *res = (StResCliSrvMsg *)server_reply;
		if (ntohl(res->res) != 0)
		{
			printf("snd data error\n");
			return -1;
		}

		//printf("ProMsg: src=%s dest=%s data=%s\n",
		//res->srcusername, res->destusername, res->msg);
	}
	else if (P2P_RES == cmdid)
	{
		StResP2PMsg *res = (StResP2PMsg *)server_reply;
		if (ntohl(res->res) != 0)
		{
			printf("p2p req error\n");
			return -1;
		}

		bzero(&client_addr, sizeof(client_addr));
		client_addr.sin_family = AF_INET;
		client_addr.sin_addr.s_addr = inet_addr(res->ip);
		client_addr.sin_port = res->port;
		socket_fd = socket(AF_INET, SOCK_DGRAM, 0); //socket udp
		if (socket_fd < 0)
		{
			perror("Create Socket Failed:");
			return -1;
		}
		printf("P2PREQ: Destinatário=%s ip=%s port=%d\n", res->destusername, res->ip, ntohs(res->port));
	}
	else if (P2P_CHAT_REQ == cmdid)
	{
		StReqP2PChatMsg *req = (StReqP2PChatMsg *)server_reply;
		printf("P2PMSG: Remetente=%s Mensagem=%s||\n", req->srcusername,req->msg);
	}
	else if (GROUP_CREATE_RES == cmdid) //create group
	{
		StResCreateGroupMsg *res = (StResCreateGroupMsg *)server_reply;
		if (ntohl(res->res) != 0)
		{
			printf("CreateGroup error\n");
			return -1;
		}
		bzero(&group_addr, sizeof(group_addr));
		group_addr.sin_family = AF_INET;
		group_addr.sin_addr.s_addr = inet_addr(res->ip);
		group_addr.sin_port = res->port;
		group_fd = GetGroupRecvSock(res->ip, res->port);
		printf("GRUPO: ip=%s port=%d\n", res->ip, ntohs(res->port));
	}
	else if (GROUP_RES == cmdid) //get group
	{
		StResCreateGroupMsg *res = (StResCreateGroupMsg *)server_reply;
		if (ntohl(res->res) != 0)
		{
			printf("get group error\n");
			return -1;
		}
		group_fd = GetGroupRecvSock(res->ip, res->port);
		bzero(&group_addr, sizeof(group_addr));
		group_addr.sin_family = AF_INET;
		group_addr.sin_addr.s_addr = inet_addr(res->ip);
		group_addr.sin_port = res->port;
		printf("Grupo: ip=%s port=%d\n", res->ip, ntohs(res->port));
	}
	else if (GROUP_CHAT_REQ == cmdid) //group chat
	{
		StReqGroupChatMsg *res = (StReqGroupChatMsg *)server_reply;
		printf("Remetente:%s Mensagem:%s||\n", res->snnder, res->msg);
	}

	return 0;
}

int main(int argc, char **argv)
{
	if (argc < 3) //Se o user n coloca tds os argumentos
	{
		printf("main: ./udpclient ip port\n");
		return -1;
	}
	printf("CLIENT: login <username> <password>\n");
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]); //primeiro arg = ip
	server_addr.sin_port = htons(atoi(argv[2]));	  //segundo arg = port
	int len = 0;
	int client_socket_fd = socket(AF_INET, SOCK_DGRAM, 0); //socket para udp
	if (client_socket_fd < 0)
	{
		perror("Create Socket Failed:");
		exit(1);
	}

	char message[1024], server_reply[1024];
	struct timeval timeout;
	fd_set rset;
	int keyboard = open("/dev/tty", O_RDONLY | O_NONBLOCK); //lê do terminal
	while (1)
	{
		int maxfd = client_socket_fd; //Initialize the parameter maxfd in the select function
		maxfd = maxfd > keyboard ? maxfd : keyboard;
		FD_ZERO(&rset);					 //Initialize the read set
		FD_SET(client_socket_fd, &rset); //Will listen on socket
		FD_SET(keyboard, &rset);
		if (socket_fd > 0)
		{
			maxfd = maxfd > socket_fd ? maxfd : socket_fd;
			FD_SET(socket_fd, &rset);
		}

		if (group_fd > 0)
		{
			maxfd = maxfd > group_fd ? maxfd : group_fd;
			FD_SET(group_fd, &rset);
		}

		timeout.tv_sec = 24 * 60 * 60;
		timeout.tv_usec = 0;

		int nready = select(maxfd + 1, &rset, NULL, NULL, &timeout); //Note that it is maxfd+1, which returns the number of ready
		if (nready <= 0)											 //Returning -1 indicates that an error occurred in the select() function, and the description word set becomes unpredictable at this time
		{
			sleep(1);						 //If select() goes wrong, empty the collection and start again
			FD_ZERO(&rset);					 //Initialize the read set
			FD_SET(client_socket_fd, &rset); //Will listen on socket
			FD_SET(keyboard, &rset);
			if (socket_fd > 0)
			{
				FD_SET(socket_fd, &rset);
			}
			if (group_fd > 0)
			{
				maxfd = maxfd > group_fd ? maxfd : group_fd;
				FD_SET(group_fd, &rset);
			}
			continue;
		}
		if (FD_ISSET(keyboard, &rset))
		{
			fgets(message, 1024, stdin);
		}
//LEITURA DE MENSAGENS DO SERVER------------------------------------------------------------------------		
		//client-server resposta
		else if (FD_ISSET(client_socket_fd, &rset))
		{
			if (recv(client_socket_fd, server_reply, 2000, 0) <= 0) //se não conseguir receber do server
			{
				printf("The program is closed and all connections are closed...\n");
				break;
			}

			ProMsg(server_reply); //print msg recebidas do server 
			continue;
		}
		//P2P
		else if (FD_ISSET(socket_fd, &rset))
		{
			if (recv(socket_fd, server_reply, 2000, 0) <= 0)
			{
				printf("The program is closed and all connections are closed...\n");
				break;
			}

			printf("P2PCHAT: p2pchat\n");
			continue;
		}
		//print mensagens recebidas do grupo
		else if (FD_ISSET(group_fd, &rset))
		{
			if (recv(group_fd, server_reply, 2000, 0) <= 0)
			{
				printf("The program is closed and all connections are closed...\n");
				break;
			}

			StReqGroupChatMsg *req = (StReqGroupChatMsg *)server_reply;
			printf("Groupchat Remetente:%s Mensagem:%s\n", req->snnder, req->msg);
			continue;
		}
//LEITURA DE COMANDOS DO USER------------------------------------------------------------------------
		len = strlen(message);
		if (1 == len && message[len - 1] == '\n') //Only the newline character
		{
			continue;
		}

		if (message[len - 1] == '\n')
		{
			message[len - 1] = 0;
		}

		//printf("message: msg=%s\n", message);
		//send login
		if (strstr(message, "login") != NULL) //se 1º palavra é login
		{
			GetLoginInfo(message); //retira info da linha introduzida
			StReqLoginMsg stReqLoginMsg;
			stReqLoginMsg.cmdid = htonl(LOGIN_REQ);											  //define idcommand para ser lido no lado do server
			snprintf(stReqLoginMsg.username, sizeof(stReqLoginMsg.username), "%s", username); //username da linha colocado na estrutura
			snprintf(stReqLoginMsg.passwd, sizeof(stReqLoginMsg.passwd), "%s", passwd);		  //password da linha colocado na estrutura
			//envia tentativa de login
			if (sendto(client_socket_fd, (char *)&stReqLoginMsg, sizeof(stReqLoginMsg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
			{
				perror("Send File Name Failed:");
				continue;
			}
		}
		//send client-server msg
		else if (strstr(message, "cs") != NULL) //client-server msg
		{
			StReqCliSrvMsg stReqCsMsg;
			GetCsInfo(message, stReqCsMsg.destusername, sizeof(stReqCsMsg.destusername), stReqCsMsg.msg); //coloca info da linha na estrutura
			stReqCsMsg.cmdid = htonl(CLISRV_REQ);														  //define idcommand para ser lido no lado do server
			snprintf(stReqCsMsg.srcusername, sizeof(stReqCsMsg.srcusername), "%s", username);			  //guarda na struc username do remetente
			//send server struct
			if (sendto(client_socket_fd, (char *)&stReqCsMsg, sizeof(stReqCsMsg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
			{
				perror("Send File Name Failed:");
				continue;
			}
		}
		//send p2p conection to user
		else if (strstr(message, "p2preq") != NULL)
		{
			StReqP2PMsg stReqP2PMsg; //struc de request
			GetP2PReqInfo(message, stReqP2PMsg.destusername, sizeof(stReqP2PMsg.destusername)); //Save in struc inf do linha 
			snprintf(stReqP2PMsg.srcusername, sizeof(stReqP2PMsg.srcusername), "%s", username);
			stReqP2PMsg.cmdid = htonl(P2P_REQ);//define idcommand
			//envia struc 
			if (sendto(client_socket_fd, (char *)&stReqP2PMsg, sizeof(stReqP2PMsg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
			{
				perror("Send File Name Failed:");
				continue;
			}
		}
		//send send p2p msg
		else if (strstr(message, "p2pchat") != NULL)
		{
			StReqP2PChatMsg req;//struc msgp2p to send
			GetP2PChatInfo(message, req.msg);
			snprintf(req.srcusername, sizeof(req.srcusername), "%s",username);
			snprintf(req.destusername, sizeof(req.destusername), "%s",destusername);
			req.cmdid = htonl(P2P_CHAT_REQ); // define idcommand para ser lido no lado do server
			//envia struc da mensagem 
			if (sendto(socket_fd, (char *)&req, sizeof(req), 0, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
			{
				perror("Send File Name Failed:");
				continue;
			}
		}
		//send create group
		else if (strstr(message, "gc") != NULL)
		{
			StReqCreateGroupMsg req; //struc criar grupo
			GetP2PChatInfo(message, req.groupname); //guarda nome do grupo na struc
			snprintf(req.username, sizeof(req.username), "%s", username);
			req.cmdid = htonl(GROUP_CREATE_REQ); //define para groupcreate idcommand para ser lido no lado do server
			if (sendto(client_socket_fd, (char *)&req, sizeof(req), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
			{
				perror("Send File Name Failed:");
				continue;
			}
		}
		//send get group=connect to group
		else if (strstr(message, "gg") != NULL)
		{
			StReqCreateGroupMsg req; //struc para info de getgroup
			GetP2PChatInfo(message, req.groupname);//coloca na struc o nome do grupo pedido
			snprintf(req.username, sizeof(req.username), "%s", username); //nome do user
			req.cmdid = htonl(GROUP_REQ); //iddcommand para o server ler
			if (sendto(client_socket_fd, (char *)&req, sizeof(req), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
			{
				perror("Send File Name Failed:");
				continue;
			}
		}
		//send enviar msg para grupo
		else if (strstr(message, "gt") != NULL)
		{
			int snd_group_fd = socket(AF_INET, SOCK_DGRAM, 0);
			if (snd_group_fd < 0)
			{
				perror("Create Socket Failed:");
				continue;
			}
			StReqGroupChatMsg req; //struc msg para grupo
			GetP2PChatInfo(message, req.msg);  //store line info in struc 
			snprintf(req.snnder, sizeof(req.snnder), "%s", username); //strore username in struc sender
			req.cmdid = htonl(GROUP_CHAT_REQ); //define iddcommand para server ler
			//envia struc
			if (sendto(snd_group_fd, (char *)&req, sizeof(req), 0, (struct sockaddr *)&group_addr, sizeof(group_addr)) < 0)
			{
				perror("Send File Name Failed:");
				continue;
			}
		}
	}

	return 0;
}

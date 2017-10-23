/*
 * vi: set ts=4 sw=4 cin scrolloff=2:
 *
 * This is an porting module of RMTP, Reliable Multisession Transfer Protocol.
 *
 * Author: kayz <kjum@hyuee.hanyang.ac.kr>
 *
 * History:
 * 2000.11.24  kayz    Design and start
 */
#include <stdio.h>
#include "rmtp_core.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct sockaddr_in Serv_addr1, Cli_addr1;
int Send_sock;

void req_initialize(int port1);

int main(int argc, char *argv[])
{
	int port;
	RMTP_REQUEST *req;
	char *p;
	int l;

	if (argc != 3) {
		printf("Usage: %s port_no len\n", argv[0]);
		return 1;
	}
	req_initialize(atoi(argv[1]));
	l = atoi(argv[2]);
	req = (RMTP_REQUEST *)MALLOC(sizeof(RMTP_REQUEST));
	req->header.TYPE = TYPE_REQUEST;
	req->header.FLAG = 0;
	req->header.LENGTH = sizeof(RMTP_REQUEST);
	req->header.SESID = 0;
	req->header.SEQ = 0;
	req->LENGTH = l;
	send_packet((char *)req, sizeof(RMTP_REQUEST));
	FREE(req);
	
	return 0;
}

void req_initialize(int port1)
{
	BZERO((char *)&Serv_addr1, sizeof(Serv_addr1));
	Serv_addr1.sin_family = AF_INET;
	Serv_addr1.sin_addr.s_addr = inet_addr("127.0.0.1");
	Serv_addr1.sin_port = htons(port1);

	if ((Send_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("Cannot open send socket\n");
		return;
	}
	BZERO((char *)&Cli_addr1, sizeof(Cli_addr1));
	Cli_addr1.sin_family = AF_INET;
	Cli_addr1.sin_addr.s_addr = htonl(INADDR_ANY);
	Cli_addr1.sin_port = htons(0);

	if (bind(Send_sock, (struct sockaddr *)&Cli_addr1, sizeof(Cli_addr1)) < 0) {
		printf("Cannot bind send socket\n");
		return;
	}
}

int send_packet(char *p, int l)
{
	if (sendto(Send_sock, p, l, 0, (struct sockaddr *)&Serv_addr1, sizeof(Serv_addr1)) != l) {
		printf("TX not match\n");
	}
	return 0;
}


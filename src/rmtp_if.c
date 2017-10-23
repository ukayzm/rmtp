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
#include "rmtp_if.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct sockaddr_in Serv_addr1, Serv_addr2, Cli_addr1, Cli_addr2;
int Recv_sock, Send_sock;


int main(int argc, char *argv[])
{
	if (argc == 1)
		porting_initialize(3000, 3001);
	else
		porting_initialize(3001, 3000);
	rmtp_main();
	
	return 0;
}

void porting_initialize(int port1, int port2)
{
	if ((Recv_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("Cannot open recv socket\n");
		return;
	}
	BZERO((char *)&Serv_addr1, sizeof(Serv_addr1));
	Serv_addr1.sin_family = AF_INET;
	Serv_addr1.sin_addr.s_addr = htonl(INADDR_ANY);
	Serv_addr1.sin_port = htons(port1);

	if (bind(Recv_sock, (struct sockaddr *)&Serv_addr1, sizeof(Serv_addr1)) < 0) {
		printf("Cannot bind recv socket\n");
		return;
	}

	BZERO((char *)&Serv_addr2, sizeof(Serv_addr2));
	Serv_addr2.sin_family = AF_INET;
	Serv_addr2.sin_addr.s_addr = inet_addr("127.0.0.1");
	Serv_addr2.sin_port = htons(port2);

	if ((Send_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("Cannot open send socket\n");
		return;
	}
	BZERO((char *)&Cli_addr2, sizeof(Cli_addr2));
	Cli_addr2.sin_family = AF_INET;
	Cli_addr2.sin_addr.s_addr = htonl(INADDR_ANY);
	Cli_addr2.sin_port = htons(0);

	if (bind(Send_sock, (struct sockaddr *)&Cli_addr2, sizeof(Cli_addr2)) < 0) {
		printf("Cannot bind send socket\n");
		return;
	}
	printf("\n\nPorting initialized. Listening from port %d.\n", port1);
}

RMTP_HEADER *receive_packet(void)
{
	RMTP_HEADER *msg;
	RMTP_REQUEST *req;
	int sz, i;
	char *p;

	msg = (RMTP_HEADER *)MALLOC(UNIT_WIN_SIZE + sizeof(RMTP_HEADER));
	if (msg == NULL)
		return NULL;
	sz = sizeof(Cli_addr1);
	recvfrom(Recv_sock, (char *)msg, UNIT_WIN_SIZE + sizeof(RMTP_HEADER), 0, (struct sockaddr *)&Cli_addr1, &sz);

	printf("\nPacket from port %d\n", ntohs(Cli_addr1.sin_port));
	dump_packet(msg);

	if (msg->TYPE == TYPE_REQUEST) {
		req = (RMTP_REQUEST *)msg;
		req->DATA = p = (char *)MALLOC(req->LENGTH);
		for (i = 0; i < req->LENGTH; i++) {
			p[i] = i;
		}
	}
		
	return (RMTP_HEADER *)msg;
}

int send_packet(char *p, int l)
{
	printf("\nPacket to port %d\n", ntohs(Serv_addr2.sin_port));
	dump_packet((RMTP_HEADER *)p);

	if (sendto(Send_sock, p, l, 0, (struct sockaddr *)&Serv_addr2, sizeof(Serv_addr2)) != l) {
		printf("TX not match\n");
	}
	return 0;
}


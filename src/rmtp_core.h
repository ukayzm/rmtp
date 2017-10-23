/*
 * vi: set ts=4 sw=4 cin scrolloff=2:
 *
 * This is an implementation of RMTP, Reliable Multisession Transfer Protocol.
 *
 * Author: kayz <kjum@hyuee.hanyang.ac.kr>
 *
 * History:
 * 2000.11.23  kayz  Design and start
 */
#ifndef _RMTP_H_
#define _RMTP_H_

#include "rmtp_if.h"

/*
 * Session States
 */
#define ST_WAIT_ACK			1
#define ST_WAIT_DATA		2
#define ST_WAIT_DONE		3

/*
 * Session Events
 * Session event must equal to message types below
 */
#define EV_REQUEST			0
#define EV_RX_DATA			1
#define EV_RX_ACK			2
#define EV_RX_DONE			3
#define EV_TIMEOUT			4
#define EV_3TIMEOUT			5
#define EV_ABORT			6
#define EV_MAX				7

/*
 * Message types
 */
#define TYPE_REQUEST		0
#define TYPE_DATA           1
#define TYPE_ACK            2
#define TYPE_DONE           3
#define TYPE_TIMEOUT        4
#define TYPE_3TIMEOUT       5
#define TYPE_ABORT          6
#define TYPE_MAX            7

/*
 * Message Flags
 */
#define FLAG_SEQ_IS_TOTAL_LEN		0x01
#define FLAG_ACK_PLEASE				0x02

typedef packed struct {
	unsigned char TYPE;
	unsigned char FLAG;
	unsigned short LENGTH;	/* LENGTH is payload length */
	unsigned long SESID;
	unsigned long SEQ;
} RMTP_HEADER;

typedef packed struct {
	RMTP_HEADER header;
	char *DATA;
	unsigned long LENGTH;
} RMTP_REQUEST;

typedef struct RMTP_SESSION_ {
	struct RMTP_SESSION_ *NEXT;

	unsigned long SESID;
	int STATE;
	int TIMEOUT;

	char *DATA;
	unsigned long TOTAL_LEN;
	unsigned long WIN_START;	/* used by sender-side */
	unsigned long WIN_SIZE;		/* used by sender-side */
	unsigned long SEQ;

	int RESULT;
} RMTP_SESSION;

typedef int (*RMTP_EV_PROC)(RMTP_SESSION *ses, RMTP_HEADER *msg);

#define UNIT_WIN_SIZE	1000
#define MAX_WIN_SIZE	10000
#define RMTP_TIMEOUT_MS	1000

#ifndef SUCCESS
#define SUCCESS 1
#endif
#ifndef FAIL
#define FAIL 2
#endif

/*
 * Action list:
 */
extern RMTP_SESSION *ac_new_session();
extern int ac_report();
extern int ac_send_data();
extern int ac_send_ack();
extern int ac_send_done();
extern int ac_remove_session();

void rmtp_main(void);

/*
 * Interface functions
 */
extern int rmtp_tx_request(char *data, int len);
extern void dump_packet(RMTP_HEADER *msg);
extern void dump_session(RMTP_SESSION *ses);

extern int send_packet();
extern RMTP_HEADER *receive_packet();

#endif


/*
 * vi: set ts=4 sw=4 cin scrolloff=2:
 *
 * This is an implementation of RMTP, Reliable Multisession Transfer Protocol.
 *
 * Author: kayz <kjum@hyuee.hanyang.ac.kr>
 *
 * History:
 * 2000.11.23  kayz    Design and start
 */
#include <stdio.h>
#include "rmtp_core.h"
#include "sestable.h"
#include "rmtp_if.h"


/*
 * Event Processing Functions.
 */
int ev_request(RMTP_SESSION *ses, RMTP_HEADER *msg);
int ev_rx_data(RMTP_SESSION *ses, RMTP_HEADER *msg);
int ev_rx_ack(RMTP_SESSION *ses, RMTP_HEADER *msg);
int ev_rx_done(RMTP_SESSION *ses, RMTP_HEADER *msg);
int ev_timeout(RMTP_SESSION *ses, RMTP_HEADER *msg);
int ev_3timeout(RMTP_SESSION *ses, RMTP_HEADER *msg); 
int ev_abort(RMTP_SESSION *ses, RMTP_HEADER *msg);

/*
 * Actions
 */
RMTP_SESSION *ac_new_session(RMTP_HEADER *msg);
int ac_send_data(RMTP_SESSION *ses);
int ac_send_ack(RMTP_SESSION *ses);
int ac_send_done(RMTP_SESSION *ses);
int ac_remove_session(RMTP_SESSION *ses);
int ac_report(RMTP_SESSION *ses);

RMTP_EV_PROC rmtp_ev_proc[] = {
	ev_request, ev_rx_data, ev_rx_ack, 
	ev_rx_done, ev_timeout, ev_3timeout, 
	ev_abort,
};


char *Rmtp_type_string[] = {
	"REQ", "DATA", "ACK", "DONE", "TIMEOUT", "3TIMEOUT", "ABORT"
};

int Rmtp_session_id;


void rmtp_initialize(void);


void rmtp_main(void)
{
	RMTP_HEADER *msg;
	RMTP_SESSION *ses;

	while (1) {
		msg = (RMTP_HEADER *)receive_packet();
		if (msg == NULL)
			continue;
		ses = get_session(msg);
		if (ses == NULL) {
		    if (msg->TYPE != TYPE_DATA && msg->TYPE != TYPE_REQUEST) {
				FREE(msg);
				continue;
			}
		}
		if (msg->TYPE == TYPE_TIMEOUT) {
			if (ses->TIMEOUT >= 3) {
				msg->TYPE = TYPE_3TIMEOUT;
			}
		}
		if (msg->TYPE >= TYPE_MAX) {
			printf("Invalid type %d\n", msg->TYPE);
			FREE(msg);
			continue;
		}
		dump_session(ses);
		rmtp_ev_proc[msg->TYPE](ses, msg);
		/* TODO: Should I free msg here? */
		dump_session(ses);
	}
}

/*
 * Events
 */

int ev_request(RMTP_SESSION *ses, RMTP_HEADER *msg)
{
	RMTP_REQUEST *req;

	req = (RMTP_REQUEST *)msg;
	ses = ac_new_session(msg);
	if (ses == NULL) {
		printf("Cannot insert session\n");
		return -1;
	}

	/* ses->SESID is set by ac_new_session(). */
	ses->STATE = ST_WAIT_ACK;
	ses->TOTAL_LEN = req->LENGTH;
	ses->DATA = req->DATA;
	if (ses->TOTAL_LEN < UNIT_WIN_SIZE) {
		ses->WIN_SIZE = ses->TOTAL_LEN;
	} else {
		ses->WIN_SIZE = UNIT_WIN_SIZE;
	}
	/* all other field is zero. */
	
	ac_send_data(ses);
	return 0;
}

int ev_rx_data(RMTP_SESSION *ses, RMTP_HEADER *msg)
{
	if (ses == NULL) {
		ses = ac_new_session(msg);
		if (ses == NULL) {
			printf("Cannot insert session\n");
			return -1;
		}
		ses->STATE = ST_WAIT_DATA;
	}
	if (msg->FLAG & FLAG_SEQ_IS_TOTAL_LEN) {
		ses->TOTAL_LEN = msg->SEQ;
		msg->SEQ = 0;
		/* all other field is zero. */
	}
	if (ses->DATA == NULL) {
		if (ses->TOTAL_LEN) {
			ses->DATA = (char *)MALLOC(ses->TOTAL_LEN);
			if (ses->DATA == NULL)
				return -1;
		} else {
			/*
			 * This means that the first packet is missed and the second packet
			 * is reached.
			 */
			return -1;
		}
	}

	if (ses->STATE != ST_WAIT_DATA)
		return -1;
	ses->TIMEOUT = 0;

	if (msg->SEQ == ses->SEQ) {
		/* This is valid data. Save it. */
		memcpy((char *)(ses->DATA + ses->SEQ), (char *)(msg + 1), msg->LENGTH);
		ses->SEQ = msg->SEQ + msg->LENGTH;
		if (msg->FLAG & FLAG_ACK_PLEASE) {
			ac_send_ack(ses);
		}
	} else if (msg->SEQ < ses->SEQ) {
		/* 
		 * In this case, this is retransmitted data.
		 * We just discard this packet.
		 */
	} else {
		/* TODO: Should I send ACK in this case? */
		ac_send_ack(ses);
	}
	return 0;
}

int ev_rx_ack(RMTP_SESSION *ses, RMTP_HEADER *msg)
{
	if (ses->STATE != ST_WAIT_ACK)
		return -1;
	ses->TIMEOUT = 0;

	/* 
	 * adjust WINDOW 
	 */
	ses->WIN_START = msg->SEQ;
	ses->WIN_SIZE *= 2;
	if (ses->WIN_SIZE > MAX_WIN_SIZE)
		ses->WIN_SIZE = MAX_WIN_SIZE;
	if (ses->WIN_START + ses->WIN_SIZE > ses->TOTAL_LEN)
		ses->WIN_SIZE = ses->TOTAL_LEN - ses->WIN_START;

	if (msg->SEQ == ses->TOTAL_LEN) {
		/* We received the last ack */
		ac_send_done(ses);
		ac_report(ses);
		ac_remove_session(ses);
		return 0;
	}
	ac_send_data(ses);
	return 0;
}

int ev_rx_done(RMTP_SESSION *ses, RMTP_HEADER *msg)
{
	if (ses->STATE != ST_WAIT_DONE)
		return -1;
	ses->TIMEOUT = 0;

	ac_report(ses);
	ac_remove_session(ses);
	return 0;
}

int ev_timeout(RMTP_SESSION *ses, RMTP_HEADER *msg)
{
	ses->TIMEOUT++;
	switch (ses->STATE) {
	case ST_WAIT_ACK:
		/* adjust WIN_SIZE */
		ses->WIN_SIZE /= 2;
		if (ses->WIN_SIZE < UNIT_WIN_SIZE)
			ses->WIN_SIZE = UNIT_WIN_SIZE;
		if (ses->WIN_SIZE > ses->TOTAL_LEN)
			ses->WIN_SIZE = ses->TOTAL_LEN;
		ac_send_data(ses);
		break;
	case ST_WAIT_DATA:
		ac_send_ack(ses);
		break;
	case ST_WAIT_DONE:
		ac_send_ack(ses);
		break;
	default:
		return -1;
	}
	return 0;
}

int ev_3timeout(RMTP_SESSION *ses, RMTP_HEADER *msg)
{
	if (ses->STATE == ST_WAIT_DONE) {
		ses->RESULT = SUCCESS;
	} else {
		ses->RESULT = FAIL;
	}
	ac_report(ses);
	ac_remove_session(ses);
	return 0;
}

int ev_abort(RMTP_SESSION *ses, RMTP_HEADER *msg)
{
	ac_remove_session(ses);
	return 0;
}

/*
 * Actions
 */

RMTP_SESSION *ac_new_session(RMTP_HEADER *msg)
{
	RMTP_SESSION *ses;

	ses = (RMTP_SESSION *)MALLOC(sizeof(RMTP_SESSION));
	if (ses == NULL)
		return NULL;
	BZERO((char *)ses, sizeof(RMTP_SESSION));
	insert_session(ses);
	ses->SESID = ++Rmtp_session_id;

	return ses;
}

int ac_send_data(RMTP_SESSION *ses)
{
	RMTP_HEADER *msg;
	unsigned short length;
	unsigned long cur_ptr;
	unsigned char flag;

	length = UNIT_WIN_SIZE;
	cur_ptr = ses->WIN_START;
	do {
		flag = 0;
		if (length >= ses->WIN_SIZE) {
			length = ses->WIN_SIZE;
		}
		if (cur_ptr + length > ses->WIN_START + ses->WIN_SIZE) {
			length = ses->WIN_START + ses->WIN_SIZE - cur_ptr;
		}
		if (cur_ptr + length == ses->WIN_START + ses->WIN_SIZE) {
			flag |= FLAG_ACK_PLEASE;
		}
		msg = (RMTP_HEADER *)MALLOC(sizeof(RMTP_HEADER) + length);
		if (msg == NULL)
			return -1;
		msg->TYPE = TYPE_DATA;
		msg->LENGTH = length;
		msg->SESID = ses->SESID;

		if (cur_ptr == 0) {
			flag |= FLAG_SEQ_IS_TOTAL_LEN;
			msg->SEQ = ses->TOTAL_LEN;
		} else {
			msg->SEQ = cur_ptr;
		}
		msg->FLAG = flag;

		memcpy((char *)(msg + 1), (char *)(ses->DATA + cur_ptr), length);
		send_packet((char *)msg, sizeof(RMTP_HEADER) + length);
		/* TODO: Should I free msg here? */

		cur_ptr += length;
	} while (cur_ptr < ses->WIN_START + ses->WIN_SIZE);

	return 0;
}

int ac_send_ack(RMTP_SESSION *ses)
{
	RMTP_HEADER *msg;

	msg = (RMTP_HEADER *)MALLOC(sizeof(RMTP_HEADER));
	if (msg == NULL)
		return -1;
	msg->TYPE = TYPE_ACK;
	msg->FLAG = 0;
	msg->LENGTH = 0;
	msg->SESID = ses->SESID;
	msg->SEQ = ses->SEQ;
	send_packet((char *)msg, sizeof(RMTP_HEADER));
	/* TODO: Should I free msg here? */

	if (msg->SEQ == ses->TOTAL_LEN) {
		/* This is last ACK. */
		ses->STATE = ST_WAIT_DONE;
	}
	return 0;
}

int ac_send_done(RMTP_SESSION *ses)
{
	RMTP_HEADER *msg;

	msg = (RMTP_HEADER *)MALLOC(sizeof(RMTP_HEADER));
	if (msg == NULL)
		return -1;
	msg->TYPE = TYPE_DONE;
	msg->FLAG = 0;
	msg->LENGTH = 0;
	msg->SESID = ses->SESID;
	msg->SEQ = ses->SEQ;
	send_packet((char *)msg, sizeof(RMTP_HEADER));
	/* TODO: Should I free msg here? */

	return 0;
}

int ac_remove_session(RMTP_SESSION *ses)
{
	return remove_session_by_object(ses);
}

int ac_report(RMTP_SESSION *ses)
{
	int i, final, remain, *ip;
	char *cp;
	int sum;

	final = ses->TOTAL_LEN / 4;
	remain = ses->TOTAL_LEN % 4;
	sum = 0;
	ip = (int *)ses->DATA;
	for (i = 0; i < final; i++) {
		sum += *ip++;
	}
	cp = (char *)ip;
	for (i = 0; i < remain; i++) {
		sum += *cp++;
	}
	printf("CHECKSUM: %08x\n", sum);

	return 0;
}

/*
 * Interface Functions
 * TODO
 */
int rmtp_tx_request(char *data, int len)
{
	return 0;
}

/*
 * Diagnostic Functions
 */
void dump_packet(RMTP_HEADER *msg)
{
	RMTP_REQUEST *req;

	printf("TYPE: %s (%02x)\t", Rmtp_type_string[msg->TYPE], msg->TYPE);
	printf("   FLAG: %02x\t", msg->FLAG);
	printf(" LENGTH: %d\t", msg->LENGTH);
	printf("  SESID: %d\t", msg->SESID);
	printf("    SEQ: %d\n", msg->SEQ);
	switch (msg->TYPE) {
	case TYPE_REQUEST:
		req = (RMTP_REQUEST *)msg;
		printf("TOTAL LENGTH: %d\t", req->LENGTH);
		printf("  DATA: %08x\n", req->DATA);
		break;
	}
}

void dump_session(RMTP_SESSION *ses)
{
	if (ses == NULL)
		return;
	printf("NEXT: %08x\t", ses->NEXT);
	printf("SESID: %d\t", ses->SESID);
	printf("STATE: %d\t", ses->STATE);
	printf("TIMEOUT: %d\t", ses->TIMEOUT);
	printf("DATA: %08x\n", ses->DATA);
	printf("TOTAL_LEN: %d\t", ses->TOTAL_LEN);
	printf("WIN_START: %d\t", ses->WIN_START);
	printf("WIN_SIZE: %d\t", ses->WIN_SIZE);
	printf("SEQ: %d\t", ses->SEQ);
	printf("RESULT: %d\n", ses->RESULT);
}


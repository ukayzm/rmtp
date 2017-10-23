/*
 * vi: set ts=4 sw=4 cin scrolloff=2:
 *
 * This is an implementation of session table.
 * This module is slightly modified from hash table implementation.
 *
 * Author: kayz <kjum@hyuee.hanyang.ac.kr>
 *
 * History:
 *
 */

#include <stdio.h>
#include "rmtp_core.h"
#include "rmtp_if.h"
#include "sestable.h"


int initialize_session_table(void);
int destroy_session_table(void);
int insert_session(RMTP_SESSION *ses);
int remove_session_by_id(unsigned long sesid);
int remove_session_by_object(RMTP_SESSION *ses);
RMTP_SESSION *get_session(RMTP_HEADER *msg);


RMTP_SESSION *Rmtp_session;


int initialize_session_table(void)
{
	destroy_session_table();
	return 0;
}

int destroy_session_table(void)
{
	RMTP_SESSION *ses, *next;
	
	ses = Rmtp_session;
	while (ses) {
		next = ses->NEXT;

		if (ses->DATA)
			FREE(ses->DATA);
			/* TODO: Should I free ses->DATA here? */

		FREE(ses);
		ses = next;
	}
	Rmtp_session = NULL;
	return 0;
}


RMTP_SESSION *get_session(RMTP_HEADER *msg)
{
	RMTP_SESSION *ses;
	unsigned long sesid;

	sesid = msg->SESID;
	for (ses = Rmtp_session; ses; ses = ses->NEXT) {
		if (ses->SESID == sesid)
			return ses;
	}
	return NULL;
}


/*
 * Allocate new entry and link it to session table. 
 */
int insert_session(RMTP_SESSION *ses)
{
	if (Rmtp_session)
		ses->NEXT = Rmtp_session;
	else
		ses->NEXT = NULL;
	Rmtp_session = ses;

	return 0;
}


/*
 * Unlink key-matched object and deallocate it.
 */
int remove_session_by_id(unsigned long sesid)
{
	RMTP_SESSION *ses, *temp_entry;

	ses = Rmtp_session;
	if (ses) {
		if (ses->SESID == sesid) {
			/*
			 * This is the first one.
			 */
			Rmtp_session = ses->NEXT;
			FREE(ses);	/* TODO: Should I free ses->DATA here? */
			return 0;
		} else {
			temp_entry = ses;
			ses = ses->NEXT;
			for (; ses; temp_entry = ses, ses = ses->NEXT) {
				if (ses->SESID == sesid) {
					temp_entry = ses->NEXT;
					FREE(ses);	/* TODO: Should I free ses->DATA here? */
					return 0;
				}
			}
		}
	}
	return -1;
}

/*
 * Unlink key-matched object and deallocate it.
 */
int remove_session_by_object(RMTP_SESSION *object)
{
	RMTP_SESSION *ses, *temp_ses;

	ses = Rmtp_session;

	if (ses) {
		if (ses == object) {
			Rmtp_session = ses->NEXT;
			FREE(ses);
			return 0;
		} else {
			temp_ses = ses;
			ses = ses->NEXT;
			for (; ses; temp_ses = ses, ses = ses->NEXT) {
				if (ses == object)  {
					temp_ses = ses->NEXT;
					FREE(ses);
					return 0;
				}
			}
		}
	}
	return 1;
}

/*
void dump_table()
{
	RMTP_SESSION *ses;

	printf("ENTRY[%d] = %08x", hash, table->ENTRY[hash]);
	for (ses = table->ENTRY[hash]->NEXT; ses; ses = ses->NEXT) {
		printf(" -> %08x", ses);
	}
	printf("\n");
}
*/


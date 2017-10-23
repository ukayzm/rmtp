/*
 * vi: set ts=4 sw=4 cin scrolloff=2:
 *
 * This is an implementation of session table.
 *
 * Author: kayz <kjum@hyuee.hanyang.ac.kr>
 *
 * Creation: 2000.9.13
 *
 * Fixes:
 */

#ifndef _HASH_H_
#define _HASH_H_

#include "rmtp_core.h"

#define MAX_HASH_SIZE		16
#define DEFAULT_KEY_OFFSET	12


/*
 * Session entry has the format line the following;
 *
 * +-------------+-------------+-------------+-----------------------------+
 * | NEXT ptr(4) |  RNEXT(4)   |  RPREV(4)   |  CONTENT (refer to rmtp.h)  |
 * +-------------+-------------+-------------+-----------------------------+
 *
 * CONTENT is not defined at this structure definition.
 */


/*
 * Base function definitions
 */
extern int initialize_session_table(void);
extern int destroy_session_table(void);
extern int insert_session(RMTP_SESSION *ses);
extern int remove_session_by_id(unsigned long sesid);
extern int remove_session_by_object(RMTP_SESSION *ses);
extern RMTP_SESSION *get_session(RMTP_HEADER *msg);

#endif


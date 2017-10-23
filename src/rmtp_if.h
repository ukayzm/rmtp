/*
 * vi: set ts=4 sw=4 cin scrolloff=2:
 *
 * This is the utilities of hash table for the portability.
 *
 * Author: kayz <kjum@hyuee.hanyang.ac.kr>
 *
 * Creation: 2000.9.13
 *
 * Fixes:
 */

#ifndef _PORTING_H_
#define _PORTING_H_

#include <stdlib.h>
#include <strings.h>

#define MALLOC(a)		malloc(a)
#define FREE(a)			free(a)
#define BZERO(a, b)		bzero((a), (b))

#define packed

void porting_initialize(int port1, int port2);

#endif


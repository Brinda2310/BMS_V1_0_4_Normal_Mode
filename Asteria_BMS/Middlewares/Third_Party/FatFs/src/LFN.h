/*
 * LFN.h
 *
 *  Created on: 07-Feb-2017
 *      Author: NIKHIL
 */

#ifndef _LFN_H_
#define _LFN_H_

#include "integer.h"

static const unsigned short Tbl[];

WCHAR ff_convert (	/* Converted character, Returns zero on error */
	WCHAR	src,	/* Character code to be converted */
	UINT	dir		/* 0: Unicode to OEMCP, 1: OEMCP to Unicode */
);

WCHAR ff_wtoupper(WCHAR chr);

#endif


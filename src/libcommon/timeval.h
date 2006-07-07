/*****************************************************************************\
 *  $Id: timeval.h,v 1.1 2006-07-07 18:14:16 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2006 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>
 *  UCRL-CODE-221226
 *  
 *  This file is part of Ipmiconsole, a set of IPMI 2.0 SOL libraries
 *  and utilities.  For details, see http://www.llnl.gov/linux/.
 *  
 *  Ipmiconsole is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by the 
 *  Free Software Foundation; either version 2 of the License, or (at your 
 *  option) any later version.
 *  
 *  Ipmiconsole is distributed in the hope that it will be useful, but 
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 *  for more details.
 *  
 *  You should have received a copy of the GNU General Public License along
 *  with Ipmiconsole; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
\*****************************************************************************/

#ifndef _TIMEVAL_H
#define _TIMEVAL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else  /* !TIME_WITH_SYS_TIME */
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else /* !HAVE_SYS_TIME_H */
#include <time.h>
#endif	/* !HAVE_SYS_TIME_H */
#endif /* !TIME_WITH_SYS_TIME */

void timeval_clear(struct timeval *a);

int timeval_gt(struct timeval *a, struct timeval *b);

int timeval_lt(struct timeval *a, struct timeval *b);

void timeval_add(struct timeval *a, struct timeval *b, struct timeval *result);

void timeval_sub(struct timeval *a, struct timeval *b, struct timeval *result);

void timeval_millisecond_init(struct timeval *a, unsigned int ms);

void timeval_add_ms(struct timeval *a, unsigned int ms, struct timeval *result);

void timeval_sub_ms(struct timeval *a, unsigned int ms, struct timeval *result);

void timeval_millisecond_calc(struct timeval *a, unsigned int *ms);

#endif /* _TIMEVAL_H */

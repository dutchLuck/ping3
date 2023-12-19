/*
 * I C M P F U N . H
 *
 * icmpFun.h last edited Sun Oct 22 22:28:30 2023
 *
 * Function headers for ICMP Protocol message (encapsulated in IPv4).
 *
 */

#ifndef  ICMP_FUN_H
#define  ICMP_FUN_H

#include <sys/types.h>
#include <sys/param.h>
#ifdef __CYGWIN__
#include "icmpDefn.h"
#else
#ifdef __MINGW64__
#include "icmpDefn.h"
#else
#include <netinet/ip_icmp.h>	/* struct icmp */
#endif
#endif

#define ICMP_TYPE_ECHO  0		/* ICMP Echo request with standard replies expected */
#define ICMP_TYPE_MASK  1		/* ICMP Mask request with standard replies expected */
#define ICMP_TYPE_TIME  2		/* ICMP Timestamp request with standard replies expected */
#define ICMP_TYPE_TIMEW  3		/* ICMP Timestamp request with little endian replies expected */


void  displayMaskReplyMask( struct icmp * );
void  displayTimeStampReplyTimestamps( struct icmp *, int );
void  displayTimeStampReply( struct icmp *  );
void  displayEchoReply( struct icmp *  );
void  displayICMP( struct icmp  * );
void  fill_ICMP_HdrPreChkSum( struct icmp *, u_char, u_char, u_short, u_short );
void  printTimeDifferenceFromICMP_TimestampReply( int, struct icmp *, double, int );

#endif

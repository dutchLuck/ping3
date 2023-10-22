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
#include <netinet/ip_icmp.h>	/* struct icmp */

void  displayMaskReplyMask( struct icmp * );
void  displayTimeStampReplyTimestamps( struct icmp *, int );
void  displayTimeStampReply( struct icmp *  );
void  displayEchoReply( struct icmp *  );
void  displayICMP( struct icmp  * );
void  fill_ICMP_HdrPreChkSum( struct icmp *, u_char, u_char, u_short, u_short );

#endif

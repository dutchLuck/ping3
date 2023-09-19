/*
 * I C M P F U N . H
 *
 * icmpFun.h last edited Mon Sep 18 19:54:40 2023
 *
 * Function headers to output ICMP Protocol field in the IP.
 *
 */

#ifndef  ICMP_FUN_H
#define  ICMP_FUN_H

#include  <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/signal.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>	/* struct icmp */
#include  "ipFun.h"


void  displayICMP( struct icmp  * );

#endif

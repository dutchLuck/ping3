/*
 * I C M P F U N . C
 *
 * icmpFun.c last edited Thu Oct  5 22:48:24 2023
 *
 * Functions to handle ICMP Protocol message in the IP datagram payload.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/signal.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include "ipFun.h"
#include "timeFun.h"	/* printMilliSecondsSinceMidnightInHMS_Format() */


void  displayEchoReply( struct icmp *  ptr )  {
	printf( "ICMP Echo Reply - Type: 0x%02x, Code: 0x%02x, Checksum 0x%04x, Sequence 0x%04x, Identifier 0x%04x\n",
		ptr->icmp_type, ptr->icmp_code, ntohs( ptr->icmp_cksum ),
		ntohs( ptr->icmp_seq ), /* seq and id must be reflected */
		ntohs( ptr->icmp_id ));
}


void  displayUnreachable( struct icmp *  ptr )  {
	printf( "ICMP Destination Unreachable\n" );
/*#define		ICMP_UNREACH_NET	0*/		/* bad net */
/*#define		ICMP_UNREACH_HOST	1*/		/* bad host */
/*#define		ICMP_UNREACH_PROTOCOL	2*/		/* bad protocol */
/*#define		ICMP_UNREACH_PORT	3*/		/* bad port */
/*#define		ICMP_UNREACH_NEEDFRAG	4*/		/* IP_DF caused drop */
/*#define		ICMP_UNREACH_SRCFAIL	5*/		/* src route failed */
/*#define		ICMP_UNREACH_NET_UNKNOWN 6*/		/* unknown net */
/*#define		ICMP_UNREACH_HOST_UNKNOWN 7*/		/* unknown host */
/*#define		ICMP_UNREACH_ISOLATED	8*/		/* src host isolated */
/*#define		ICMP_UNREACH_NET_PROHIB	9*/		/* prohibited access */
/*#define		ICMP_UNREACH_HOST_PROHIB 10*/		/* ditto */
/*#define		ICMP_UNREACH_TOSNET	11*/		/* bad tos for net */
/*#define		ICMP_UNREACH_TOSHOST	12*/		/* bad tos for host */
	printf( "\nICMP: Type %d, Code %d, Checksum 0x%04x\n", ptr->icmp_type,
		ptr->icmp_code, ntohs( ptr->icmp_cksum ));
}


void  displaySourceQuench( struct icmp *  ptr )  {
	printf( "ICMP Source Quench\n" );
	printf( "\nICMP: Type %d, Code %d, Checksum 0x%04x\n", ptr->icmp_type,
		ptr->icmp_code, ntohs( ptr->icmp_cksum ));
}


void  displayRedirect( struct icmp *  ptr )  {
	printf( "ICMP Redirect\n" );
/*#define		ICMP_REDIRECT_NET	0*/		/* for network */
/*#define		ICMP_REDIRECT_HOST	1*/		/* for host */
/*#define		ICMP_REDIRECT_TOSNET	2*/		/* for tos and net */
/*#define		ICMP_REDIRECT_TOSHOST	3*/		/* for tos and host */
	printf( "\nICMP: Type %d, Code %d, Checksum 0x%04x\n", ptr->icmp_type,
		ptr->icmp_code, ntohs( ptr->icmp_cksum ));
}


void  displayEchoRequest( struct icmp *  ptr )  {
	printf( "ICMP Echo Request\n" );
	printf( "\nICMP: Type %d, Code %d, Checksum 0x%04x\n", ptr->icmp_type,
		ptr->icmp_code, ntohs( ptr->icmp_cksum ));
}


void  displayRouterAdvert( struct icmp *  ptr )  {
	printf( "ICMP Router Advert\n" );
	printf( "\nICMP: Type %d, Code %d, Checksum 0x%04x\n", ptr->icmp_type,
		ptr->icmp_code, ntohs( ptr->icmp_cksum ));
}


void  displayRouterSolicit( struct icmp *  ptr )  {
	printf( "ICMP Router Solicit\n" );
	printf( "\nICMP: Type %d, Code %d, Checksum 0x%04x\n", ptr->icmp_type,
		ptr->icmp_code, ntohs( ptr->icmp_cksum ));
}


void  displayTimeExceeded( struct icmp *  ptr )  {
	printf( "ICMP Time To Live Exceeded\n" );
/*#define		ICMP_TIMXCEED_INTRANS	0*/		/* ttl==0 in transit */
/*#define		ICMP_TIMXCEED_REASS	1*/		/* ttl==0 in reass */
	printf( "\nICMP: Type %d, Code %d, Checksum 0x%04x\n", ptr->icmp_type,
		ptr->icmp_code, ntohs( ptr->icmp_cksum ));
}


void  displayParameterProblem( struct icmp *  ptr )  {
	struct ip  *problemPacketPtr;

	printf( "ICMP Parameter Problem at octet %d in the following packet ;-\n",
		ptr->icmp_hun.ih_pptr );
/*#define		ICMP_PARAMPROB_OPTABSENT 1*/		/* req. opt. absent */
	problemPacketPtr = ( struct ip * ) &ptr->icmp_dun.id_ip;
	printf( "\n--==##$$  Start of Parameter Problem Packet  $$##==--\n" );
	display_ip( problemPacketPtr, 64 );
	printf( "--==##$$  Finish of Parameter Problem Packet  $$##==--\n\n" );
}


void  displayTimeStampRequest( struct icmp *  ptr )  {
	printf( "ICMP Time Stamp Request\n" );
	printf( "\nICMP: Type %d, Code %d, Checksum 0x%04x\n", ptr->icmp_type,
		ptr->icmp_code, ntohs( ptr->icmp_cksum ));
}


void  displayTimeStampReplyTimestamps( struct icmp *  ptr, int  nonStdFlag )  {
	u_char *  u_Ptr;
	u_int32_t *  timePtr;

	u_Ptr = ( u_char * ) ptr;
	timePtr = ( u_int32_t * )( u_Ptr + 8 );
	printf( "tso " );
	printMilliSecondsSinceMidnightInHMS_Format( ntohl( *timePtr ));		/* We sent the correctly formatted time */
	printf( " tsr " );
	printMilliSecondsSinceMidnightInHMS_Format( ( nonStdFlag == 1 ) ? timePtr[ 1 ] : ntohl( timePtr[ 1 ] ));
	printf( " tst " );
	printMilliSecondsSinceMidnightInHMS_Format( ( nonStdFlag == 1 ) ? timePtr[ 2 ] : ntohl( timePtr[ 2 ] ));
}


void  displayTimeStampReply( struct icmp *  ptr )  {
	printf( "ICMP Time Stamp Reply\n" );
	printf( "\nICMP: Type %d, Code %d, Checksum 0x%04x\n", ptr->icmp_type,
		ptr->icmp_code, ntohs( ptr->icmp_cksum ));
}


void  displayInfoRequest( struct icmp *  ptr )  {
	printf( "ICMP Info Request\n" );
	printf( "\nICMP: Type %d, Code %d, Checksum 0x%04x\n", ptr->icmp_type,
		ptr->icmp_code, ntohs( ptr->icmp_cksum ));
}


void  displayInfoReply( struct icmp *  ptr )  {
	printf( "ICMP Info Reply\n" );
	printf( "\nICMP: Type %d, Code %d, Checksum 0x%04x\n", ptr->icmp_type,
		ptr->icmp_code, ntohs( ptr->icmp_cksum ));
}


void displayMaskRequest( struct icmp *  ptr )  {
	printf( "ICMP Mask Request\n" );
	printf( "\nICMP: Type %d, Code %d, Checksum 0x%04x\n", ptr->icmp_type,
		ptr->icmp_code, ntohs( ptr->icmp_cksum ));
}


void displayMaskReply( struct icmp *  ptr )  {
	printf( "ICMP Mask Reply\n" );
	printf( "\nICMP: Type %d, Code %d, Checksum 0x%04x\n", ptr->icmp_type,
		ptr->icmp_code, ntohs( ptr->icmp_cksum ));
}


void  displayICMP( struct icmp  *ptr )  {
	switch( ptr->icmp_type )  {
		case ICMP_ECHOREPLY :	/* echo reply */
			displayEchoReply( ptr ); break;
		case ICMP_UNREACH :	/* dest unreachable, codes: */
			displayUnreachable( ptr ); break;
		case ICMP_SOURCEQUENCH :	/* packet lost, slow down */
			displaySourceQuench( ptr ); break;
		case ICMP_REDIRECT :	/* shorter route, codes: */
			displayRedirect( ptr ); break;
		case ICMP_ECHO :	/* echo service */
			displayEchoRequest( ptr ); break;
		case ICMP_ROUTERADVERT :	/* router advertisement */
			displayRouterAdvert( ptr ); break;
		case ICMP_ROUTERSOLICIT :	/* router solicitation */
			displayRouterSolicit( ptr ); break;
		case ICMP_TIMXCEED :	/* time exceeded, code: */
			displayTimeExceeded( ptr ); break;
		case ICMP_PARAMPROB :	/* ip header bad */
			displayParameterProblem( ptr ); break;
		case ICMP_TSTAMP :	/* timestamp request */
			displayTimeStampRequest( ptr ); break;
		case ICMP_TSTAMPREPLY :	/* timestamp reply */
			displayTimeStampReply( ptr ); break;
		case ICMP_IREQ :	/* information request */
			displayInfoRequest( ptr ); break;
		case ICMP_IREQREPLY :	/* information reply */
			displayInfoReply( ptr ); break;
		case ICMP_MASKREQ :	/* address mask request */
			displayMaskRequest( ptr ); break;
		case ICMP_MASKREPLY :	/* address mask reply */
			displayMaskReply( ptr ); break;
		default : break;
	}
}

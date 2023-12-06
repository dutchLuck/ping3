/*
 * I C M P F U N . C
 *
 * icmpFun.c last edited Wed Dec  6 23:18:49 2023
 *
 * Functions to handle ICMP Protocol message in the IP datagram payload.
 *
 */

#include <stdio.h>		/* printf() */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>		/* */
#include "ipFun.h"
#include "timeFun.h"	/* printMilliSecondsSinceMidnightInHMS_Format() */
#include "icmpFun.h"	/* ICMP_TYPE_* FALSE */
#include "dbgFun.h"		/* printNamedByteArray() */


void  displayEchoReply( struct icmp *  ptr )  {
	printf( "ICMP Echo Reply: Type %d, Code %d, Checksum 0x%04x, Sequence 0x%04x, Identifier 0x%04x\n",
		ptr->icmp_type, ptr->icmp_code, ntohs( ptr->icmp_cksum ),
		ntohs( ptr->icmp_seq ), /* seq and id must be reflected */
		ntohs( ptr->icmp_id ));
}


void  displayEchoRequest( struct icmp *  ptr )  {
	printf( "ICMP Echo Request: Type %d, Code %d, Checksum 0x%04x, Sequence 0x%04x, Identifier 0x%04x\n",
		ptr->icmp_type, ptr->icmp_code, ntohs( ptr->icmp_cksum ),
		ntohs( ptr->icmp_seq ), /* seq and id must be reflected */
		ntohs( ptr->icmp_id ));
}


void  displayUnreachable( struct icmp *  ptr )  {
	printf( "ICMP Destination Unreachable: " );
	switch( ptr->icmp_code )  {
		case ICMP_UNREACH_NET : printf( "Bad Net" ); break;
		case ICMP_UNREACH_HOST : printf( "Bad Host" ); break;
		case ICMP_UNREACH_PROTOCOL : printf( "Bad Protocol" ); break;
		case ICMP_UNREACH_PORT : printf( "Bad Port" ); break;
		case ICMP_UNREACH_NEEDFRAG : printf( "Need Fragmentation" ); break;
		case ICMP_UNREACH_SRCFAIL : printf( "Source Route Failed" ); break;
		case ICMP_UNREACH_NET_UNKNOWN : printf( "Unknown Net" ); break;
		case ICMP_UNREACH_HOST_UNKNOWN : printf( "Unknown Host" ); break;
		case ICMP_UNREACH_ISOLATED : printf( "Source Host Isolated" ); break;
		case ICMP_UNREACH_NET_PROHIB : printf( "Prohibited Net" ); break;
		case ICMP_UNREACH_HOST_PROHIB : printf( "Prohibited Host" ); break;
		case ICMP_UNREACH_TOSNET : printf( "Bad TOS for Net" ); break;
		case ICMP_UNREACH_TOSHOST : printf( "Bad TOS for Host" ); break;
		case ICMP_UNREACH_FILTER_PROHIB : printf( "Filter Prohibited" ); break;
		case ICMP_UNREACH_HOST_PRECEDENCE : printf( "Filter Prohibited" ); break;
		case ICMP_UNREACH_PRECEDENCE_CUTOFF : printf( "Precedence Cutoff" ); break;
		default : printf( "? Unknown Code"); break;
	}
	printf( "\nICMP: Type %d, Code %d, Checksum 0x%04x\n", ptr->icmp_type,
		ptr->icmp_code, ntohs( ptr->icmp_cksum ));
}


void  displaySourceQuench( struct icmp *  ptr )  {
	printf( "ICMP Source Quench\n" );
	printf( "\nICMP: Type %d, Code %d, Checksum 0x%04x\n", ptr->icmp_type,
		ptr->icmp_code, ntohs( ptr->icmp_cksum ));
}


void  displayRedirect( struct icmp *  ptr )  {
	printf( "ICMP Redirect: " );
	switch( ptr->icmp_code )  {
		case ICMP_REDIRECT_NET : printf( "Redirect Net"); break;
		case ICMP_REDIRECT_HOST : printf( "Redirect Host"); break;
		case ICMP_REDIRECT_TOSNET : printf( "Redirect TOS Net"); break;
		case ICMP_REDIRECT_TOSHOST : printf( "Redirect TOS Host"); break;
		default : printf( "Unknown Code" ); break;
	}
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
	switch( ptr->icmp_code )  {
#ifndef __linux__
		case ICMP_PARAMPROB_ERRATPTR : printf( "Problem Errata Ptr"); break;
		case ICMP_PARAMPROB_LENGTH : printf( "Problem Length"); break;
#endif
		case ICMP_PARAMPROB_OPTABSENT : printf( "Problem Option Absent"); break;
		default : printf( "Unknown Problem"); break;
	}
	problemPacketPtr = ( struct ip * ) &ptr->icmp_dun.id_ip;
	printf( "\n--==##$$  Start of Parameter Problem Packet  $$##==--\n" );
	display_ip( problemPacketPtr, 64 );
	printf( "--==##$$  Finish of Parameter Problem Packet  $$##==--\n\n" );
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


void  displayTimeStampRequest( struct icmp *  ptr )  {
	printf( "ICMP Time Stamp Request: " );
	printf( "Type %d, Code %d, Checksum 0x%04x\n", ptr->icmp_type,
		ptr->icmp_code, ntohs( ptr->icmp_cksum ));
	displayTimeStampReplyTimestamps( ptr, FALSE );
	printf( "\n" );
}


void  displayTimeStampReply( struct icmp *  ptr )  {
	printf( "ICMP Time Stamp Reply: " );
	printf( "Type %d, Code %d, Checksum 0x%04x\n", ptr->icmp_type,
		ptr->icmp_code, ntohs( ptr->icmp_cksum ));
	displayTimeStampReplyTimestamps( ptr, FALSE );
	printf( "\n" );
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
	printf( "ICMP Mask Request: " );
	printf( "Type %d, Code %d, Checksum 0x%04x\n", ptr->icmp_type,
		ptr->icmp_code, ntohs( ptr->icmp_cksum ));
}


void  displayMaskReplyMask( struct icmp *  ptr )  {
	u_char *  u_Ptr;
	u_int32_t *  maskPtr;

	u_Ptr = ( u_char * ) ptr;
	maskPtr = ( u_int32_t * )( u_Ptr + 8 );
	printf( "mask " );
	printIPv4_AddressAsDottedQuad( (struct in_addr * ) maskPtr );
}


void displayMaskReply( struct icmp *  ptr )  {
	printf( "ICMP Mask Reply: " );
	printf( "ICMP: Type %d, Code %d, Checksum 0x%04x\n", ptr->icmp_type,
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


void  fill_ICMP_HdrPreChkSum( struct icmp *  hdrPtr, u_char  icmpType, u_char  icmpCode, u_short  sequncID, u_short  processID )  {
	hdrPtr->icmp_type = icmpType;
    hdrPtr->icmp_code = icmpCode;
	hdrPtr->icmp_cksum = 0;		/* compute checksum later, after data is in place */
	hdrPtr->icmp_seq = htons( sequncID ); /* seq and id must be reflected in Echo/Timestamp/Netmask Reply*/
	hdrPtr->icmp_id = htons( processID );
}


void  printTimeDifferenceFromICMP_TimestampReply( int  icmpType, struct icmp *  icmpHdrPtr, double  halfRndTripTime, int verbosityLevel )  {
	long  tsorig;		/* local device originate timestamp in # millisec since midnight UTC  */
	long  tsrecv; 		/* remote device receive timestamp in # millisec since midnight UTC */
	long  tstrnsmt; 	/* remote device transmit timestamp in # millisec since midnight UTC */

	tsorig = ntohl( icmpHdrPtr->icmp_dun.id_ts.its_otime );
	if( tsorig < 0 )  printf( "? Local device originate time is in Non standard time format %ld ms (0x%lx)", tsorig, tsorig );
	if( icmpType == ICMP_TYPE_TIME )  {
		tsrecv = ntohl( icmpHdrPtr->icmp_dun.id_ts.its_rtime );
		tstrnsmt = ntohl( icmpHdrPtr->icmp_dun.id_ts.its_ttime );
	}
	else if( icmpType == ICMP_TYPE_TIMEW )  {
		tsrecv = ( long ) ( icmpHdrPtr->icmp_dun.id_ts.its_rtime );
		tstrnsmt = ( long ) ( icmpHdrPtr->icmp_dun.id_ts.its_ttime );
	}
	else {	/* shouldn't get here, but just in case */
		tsrecv = ntohl( icmpHdrPtr->icmp_dun.id_ts.its_rtime );
		tstrnsmt = ntohl( icmpHdrPtr->icmp_dun.id_ts.its_ttime );
	}
	/* tsorig, tsrecv & tstrnsmt are both signed longs. The icmp time members in the structure are unsigned,
	 * but the max value for the # millisec since midnight is (24*60*60*1000 - 1) or 86,399,999,
	 * which easily fits into a signed long. We want them signed to compute a signed difference. */
	if( tsrecv < 0  )  printf( "? Remote Device receive time is in Non standard time format %ld ms (0x%lx)", tsrecv, tsrecv );
	if( tstrnsmt < 0 )  printf( "? Remote Device transmit time is in Non standard time format %ld ms (0x%lx)", tstrnsmt, tstrnsmt );
#ifdef DEBUG
	printNamedByteArray(( u_char *) icmpHdrPtr, 20 , 20, "printTimeDifferenceFromICMP..(): ICMP Message received" );
#endif
	 /* Now get time according to target when reply message left */
	 /* Assume reply travel time was same speed as request travel time */
 	 /* Thus time here when request was actually processed by target */
	 /* is  tsorig + 0.5 * tsRTT, now subtract that from target time */
	if( verbosityLevel && ( tsorig >= 0 ) && ( tsrecv >= 0 ))  {	/* time difference output only if verbose required */
		printf( "ICMP_TSTAMP: " );
		printSentVsReceiveDeviceClockDifferenceEstimate( tsrecv, tsorig, ( long ) ( -1.0 * halfRndTripTime ), verbosityLevel );
	}
	if( verbosityLevel > 1 )  printf( "ICMP_TSTAMP: Orig = %ld, Recv %ld, Trnsmt = %ld\n", tsorig, tsrecv, tstrnsmt );
}

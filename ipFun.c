/*
 * I P F U N . C
 *
 * ipFun.c last edited Mon Sep 18 20:24:31 2023
 *
 * Functions to handle aspects of IP datagrams
 *
 */

#include  <stdio.h>
#include  "ipFun.h"
#include  "icmpFun.h"
#include  "ipOptionsFun.h"


/*
 *	Checksum routine for Internet Protocol family headers (C Version)
 */
u_short  calcCheckSum( u_short *  addr, int  len)  {
	register int nleft = len;
	register u_short * wrdPtr = addr;
	register int sum = 0;
	u_short answer = 0;

	/*
	 * The algorithm isn't that complex,
	 * it uses a 32 bit accumulator (sum),
	 * sequential 16 bit words are added to it,
	 * and at the end, fold back all the
	 * carry bits from the top 16 bits
	 * into the lower 16 bits.
	 */
	while( nleft > 1 )  {
		sum += *wrdPtr++;
		nleft -= 2;
	}
	/* mop up an odd byte, if necessary */
	if( nleft == 1 ) {
		*(u_char *)(&answer) = *(u_char *)wrdPtr;
		sum += answer;
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ( u_short ) ( ~sum & 0xffff );		/* complement and truncate to 16 bits */
	return( answer );
}


void  pingVitalInfo( struct ip *  ipPkt, int  len )  {
	printf( "%d bytes from %s:", len, inet_ntoa( ipPkt->ip_src ));
}


void  pingTimeToLiveInfo( struct ip *  ipPkt )  {
	printf( "ttl %d", ipPkt->ip_ttl );
}


void  displayIpHeader( struct ip *  ipPkt )  {
	int  ipHeaderTotalLength;
	int  ipHeaderOptionsLength;
	u_char *  uc_ptr;

	ipHeaderTotalLength = 4 * ipPkt->ip_hl;
	ipHeaderOptionsLength = 4 * ( ipPkt->ip_hl - 5 );
	printf( "Version :\t\t%d\n", ipPkt->ip_v );
	printf( "Header Length :\t\t%d\n", ipHeaderTotalLength );
	printf( "Type of Service :\t0x%02x, ", ipPkt->ip_tos );
	switch( ipPkt->ip_tos & IPTOS_PREC_NETCONTROL )
	{
		case  IPTOS_PREC_NETCONTROL	: printf( "Net Control" ); break;
		case  IPTOS_PREC_INTERNETCONTROL: printf( "Internet Control" ); break;
		case  IPTOS_PREC_CRITIC_ECP	: printf( "Critical ECP" ); break;
		case  IPTOS_PREC_FLASHOVERRIDE	: printf( "Flash Override" ); break;
		case  IPTOS_PREC_FLASH		: printf( "Flash" ); break;
		case  IPTOS_PREC_IMMEDIATE	: printf( "Immediate" ); break;
		case  IPTOS_PREC_PRIORITY	: printf( "Priority" ); break;
		default : printf( "Routine" ); break;
	}
	printf( " Precedence, " );
	if(( ipPkt->ip_tos & IPTOS_LOWDELAY ) == IPTOS_LOWDELAY )
		printf( "Low Delay " );
	if(( ipPkt->ip_tos & IPTOS_THROUGHPUT ) == IPTOS_THROUGHPUT )
		printf( "High Throughput " );
	if(( ipPkt->ip_tos & IPTOS_RELIABILITY ) == IPTOS_RELIABILITY )
		printf( "High Reliability " );
	if(( ipPkt->ip_tos & ( IPTOS_RELIABILITY | IPTOS_THROUGHPUT | IPTOS_LOWDELAY )) == 0 )
		printf( "Normal " ); 
	printf( "Service\n" );
	printf( "Total Length :\t\t%u\n", ntohs( ipPkt->ip_len ));
	printf( "ID :\t\t\t%05u ( 0x%04x )\n", ntohs( ipPkt->ip_id ), ntohs( ipPkt->ip_id ));
	printf( "Don't Fragment :\t%d\n", ( ntohs( ipPkt->ip_off ) & IP_DF ) == IP_DF );
	printf( "More Fragments :\t%d\n", ( ntohs( ipPkt->ip_off )& IP_MF ) == IP_MF );
	printf( "Fragment Offset :\t%d\n", 8 * ( ntohs( ipPkt->ip_off ) & 0x1fff ));
	printf( "Time to Live :\t\t%d\n", ipPkt->ip_ttl );
	printf( "Protocol :\t\t%d", ipPkt->ip_p );
	displayIpProtocol( (unsigned char ) ipPkt->ip_p );
	printf( "\nChecksum :\t\t0x%04x\n", ntohs( ipPkt->ip_sum ));
	printf( "Source Address :\t%s\n", inet_ntoa( ipPkt->ip_src ));
	printf( "Destination Address :\t%s\n", inet_ntoa( ipPkt->ip_dst ));
	printf( "Options Length :\t%d\n", ipHeaderOptionsLength );
	if( ipHeaderOptionsLength > 0 )  {
		uc_ptr = ( u_char * ) ipPkt;
		uc_ptr += 20;
		displayIpOptionList( uc_ptr, ipHeaderOptionsLength );
		displayIpOptions( uc_ptr, ipHeaderOptionsLength, 1 );
	}
}


void  displayIpProtocol( unsigned char  protocolCode ) {
	switch( protocolCode )  {
		case 0 : printf( " (Reserved)" ); break;
		case 1 : printf( " (ICMP - Internet Control Message)" ); break;
		case 2 : printf( " (IGMP - Internet Group Management)" ); break;
		case 3 : printf( " (GGP - Gateway to Gateway)" ); break;
		case 4 : printf( " (IP - IP in IP)" ); break;
		case 5 : printf( " (ST - Stream)" ); break;
		case 6 : printf( " (TCP - Transmission Control)" ); break;
		case 7 : printf( " (UCL)" ); break;
		case 8 : printf( " (EGP - Exterior Gateway)" ); break;
		case 17 : printf( " (UDP - User Datagram)" ); break;
		default : printf( " (Unknown or Uncommon)" ); break;
	}
}


void  display_ip( struct ip *  datagram, int  len )  {
	unsigned char  *ptr;

	if( len < 20 )
		fprintf( stderr, "?? IP version 4 must have at least 20 bytes to be a valid datagram\n" );
	else  {
		displayIpHeader( datagram );
		ptr = ( unsigned char *) datagram + 4 * datagram->ip_hl;
		switch( datagram->ip_p ) {
			case 1 : displayICMP(( struct icmp *) ptr ); break;
			default : break;
		}
	}
}
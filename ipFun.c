/*
 * I P F U N . C
 *
 * ipFun.c last edited Wed Dec 20 19:15:35 2023
 *
 * Functions to handle aspects of IP datagrams
 *
 */

#include  <stdio.h>			/* printf() */
#include  <string.h>		/* strspn() strncpy() */
#include  <stdlib.h>		/* strtol() */
#include  <sys/socket.h>	/* getsockopt() setsockopt() */
#include  "genFun.h"		/* TRUE FALSE */
#include  "ipFun.h"
#include  "icmpFun.h"
#include  "ipOptionsFun.h"


/*
 *	Checksum routine for Internet Protocol family headers (C Version)
 */
u_short  calcCheckSum( unsigned short *  addr, int  len)  {
	register int nleft = len;
	register unsigned short * wrdPtr = addr;
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


int  strIsA_ValidDottedQuadIPv4_Address( char *  str )  {
	int  strLength;
	int  retVal;
	int  cnt;
	long  resultVal;
	char *  strPtr;
	char *  endPtrStore;
	char **  ssPtr;
	char *  tmpStrPtr;
	char *  subStrPtrs[ 4 ];
	char  tmpStr[ 16 ];

	strLength = strlen( str );
	/* Check if length is in the valid range i.e 7 (1.1.1.1) to 15 (255.255.255.255) */
	if(( retVal = (( strLength > 6 ) && ( strLength < 16 ))))  {	/* leading or trailing spaces will make the str invalid */
		/* Check if str only contains digits and full stops */
		cnt = strspn( str, "0123456789." );
		/* cnt should be the index of the terminating '\0' if only digits and . */
		if(( retVal = ( cnt == strLength )))  {
			/* Check if exactly 3 full stops in the str */
			for( cnt = 0, strPtr = str; *strPtr != '\0'; strPtr++ )  if( *strPtr == '.' )  cnt += 1;
			retVal = ( cnt == 3 );
			if( retVal )  {
				strncpy( tmpStr, str, sizeof( tmpStr ));	/* make copy to avoid changing original str */
				for( tmpStrPtr = tmpStr, ssPtr = subStrPtrs; retVal && (( *ssPtr = strsep( &tmpStrPtr, "." )) != NULL ); )
					if(( retVal = ( **ssPtr != '\0' )))  {
			    		resultVal = strtol( *ssPtr, &endPtrStore, 10 );		/* base 10 conversion of sub string to long integer */
						if(( retVal = ( *endPtrStore == '\0' )))	/* Did the whole sub string get converted ? */
							retVal = (( resultVal >= ( long ) 0 ) && ( resultVal <= ( long ) 255 ));
						if( ++ssPtr >= &subStrPtrs[4] )
							break;
					}
			}
		}
	}
#ifdef  DEBUG
	printf( "strIsA_ValidDottedQuadIPv4_Address( %s ): About to return a%svalid IPv4 address result\n",
		str, ( retVal ) ? " " : "n in" );
#endif
	return( retVal );
}


void  printIPv4_AddressAsDottedQuad( struct in_addr *  ipAddress )  {
	u_char *  bytePtr;

	bytePtr = ( u_char * ) ipAddress;
	printf( "%d.", *bytePtr++ & 0xff );
	printf( "%d.", *bytePtr++ & 0xff );
	printf( "%d.", *bytePtr++ & 0xff );
	printf( "%d", *bytePtr & 0xff );

}


void  printPingCommonInfo( struct ip *  ipPkt, int  len )  {
	printf( "%d bytes from ", len );
	printIPv4_AddressAsDottedQuad( &ipPkt->ip_src );
	printf( ":" );
}


void  printIPv4_TimeToLiveInfo( struct ip *  ipPkt )  {
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
	printf( "Source Address :\t" );
	printIPv4_AddressAsDottedQuad( &ipPkt->ip_src );
	printf( "\nDestination Address :\t" );
	printIPv4_AddressAsDottedQuad( &ipPkt->ip_dst );
	printf( "\nOptions Length :\t%d\n", ipHeaderOptionsLength );
	if( ipHeaderOptionsLength > 0 )  {
		uc_ptr = ( u_char * ) ipPkt;
		uc_ptr += 20;
		/* displayIpOptionList( uc_ptr, ipHeaderOptionsLength ); */
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
		fprintf( stderr, "Warning: IP version 4 must have at least 20 bytes to be a valid datagram\n" );
	else  {
		displayIpHeader( datagram );
		printf( "\n" );
		ptr = ( unsigned char *) datagram + 4 * datagram->ip_hl;
		switch( datagram->ip_p ) {
			case 1 : displayICMP(( struct icmp *) ptr ); break;
			default : break;
		}
	}
}


int  getIPv4_TimeToLive( int  scktID, int *  timeToLive, int  verbosityLvl )  {
	int  result, scktOpt;
	socklen_t  scktOptLen;

	scktOpt = 0;
	scktOptLen = sizeof( scktOpt );
	result = getsockopt( scktID, IPPROTO_IP, IP_TTL, &scktOpt, &scktOptLen );
	if( result < 0 )  perror( "Error: getsockopt( IP_TTL )" );
	else  {
		if( verbosityLvl > 5 )  printf( "TTL value is %d and size of TTL value is %u\n", scktOpt, scktOptLen );
		if( scktOptLen != sizeof( int ))  {
			fprintf( stderr, "Error: TTL value is unexpected size of %u bytes\n", scktOptLen );
			result = -2;	/* flag error */
		}
		else  {
			*timeToLive = scktOpt;
		}
	}
	return( result );
}


int  setIPv4_TimeToLive( int  scktID,  int  timeToLive, int  verbosityLvl )  {
	int  result, scktOpt;

	scktOpt = timeToLive;
	result = setsockopt( scktID, IPPROTO_IP, IP_TTL, &scktOpt, sizeof( scktOpt ));
	if( result < 0 )  {
		perror( "Error: setsockopt( IP_TTL )" );
		fprintf( stderr, "Error: Unable to set Time to Live to %d\n", scktOpt );
	}
	else if( verbosityLvl > 5 )  printf( "IPv4 Time to Live set to %d\n", scktOpt );
	return( result );
}


int  getIPv4_DontFragment( int  scktID, int *  dontFragmentSetting, int  verbosityLvl )  {
	int  result = -1;
#ifdef __linux__
	int  scktOpt = 0;
	socklen_t  scktOptLen;

	scktOptLen = sizeof( scktOpt );
	result = getsockopt( scktID, IPPROTO_IP, IP_MTU_DISCOVER, &scktOpt, &scktOptLen );
	if( result < 0 )  perror( "Error: getsockopt( IP_MTU_DISCOVER )" );
	else  {
		if( verbosityLvl > 5 )  printf( "IP_MTU_Discover value is %d and its size is %u\n", scktOpt, scktOptLen );
		if( scktOptLen != sizeof( int ))  {
			fprintf( stderr, "Error: IP_MTU_Discover value is unexpected size of %u bytes\n", scktOptLen );
			result = -2;	/* flag error */
		}
		else  {
			*dontFragmentSetting = (( scktOpt == IP_PMTUDISC_DO ) ? 1 : 0 );
		}
	}
#else
#ifdef IP_DONTFRAG
	int  scktOpt = 0;
	socklen_t  scktOptLen;

	scktOptLen = sizeof( scktOpt );
	result = getsockopt( scktID, IPPROTO_IP, IP_DONTFRAG, &scktOpt, &scktOptLen );
	if( result < 0 )  perror( "Error: getsockopt( IP_DONTFRAG )" );
	else  {
		if( verbosityLvl > 5 )  printf( "Don't Fragment value is %d and its size is %u\n", scktOpt, scktOptLen );
		if( scktOptLen != sizeof( int ))  {
			fprintf( stderr, "Error: Don't Fragment value is unexpected size of %u bytes\n", scktOptLen );
			result = -2;	/* flag error */
		}
		else  {
			*dontFragmentSetting = scktOpt;
		}
	}
#endif
#endif
	return( result );
}


int  setIPv4_DontFragment( int  scktID,  int  dontFragmentSetting, int  verbosityLvl )  {
	int  result = -1;
#ifdef __linux__
	int  scktOpt = 0;

	scktOpt = (( dontFragmentSetting == 1 ) ? IP_PMTUDISC_DO : IP_PMTUDISC_DONT );
	result = setsockopt( scktID, IPPROTO_IP, IP_MTU_DISCOVER, &scktOpt, sizeof( scktOpt ));
	if( result < 0 )  {
		perror( "Error: setsockopt( IP_MTU_DISCOVER)" );
		fprintf( stderr, "Error: Unable to set IP MTU Discover to %d\n", scktOpt );
	}
	else if( verbosityLvl > 5 )  printf( "IPv4 MTU Discover value set to %d\n", scktOpt );
#else
#ifdef IP_DONTFRAG
	int  scktOpt;

	scktOpt = dontFragmentSetting;
	result = setsockopt( scktID, IPPROTO_IP, IP_DONTFRAG, &scktOpt, sizeof( scktOpt ));
	if( result < 0 )  {
		perror( "Error: setsockopt( IP_DONTFRAG )" );
		fprintf( stderr, "Error: Unable to set Don't Fragment to %d\n", scktOpt );
	}
	else if( verbosityLvl > 5 )  printf( "IPv4 Don't Fragment value set to %d\n", scktOpt );
#endif
#endif
	return( result );
}


/* Apparently before Big Sur macOS doesn't support IP_DONTFRAG */
int  isIPv4_DontFragmentManipulatableOnThisOS_Version( void )  {
#ifdef  __APPLE__
#ifdef  IP_DONTFRAG
	return( TRUE );
#endif
#endif
#ifdef  __linux__
#ifdef  IP_MTU_DISCOVER
	return( TRUE );
#endif
#endif
/* defaults to telling caller that DF cannot be set */
return( FALSE );
}


int  ensureIPv4_DontFragmentSettingIsTheRequiredValue( int  scktID,  int  requiredDontFragmentValue, int  verbosityLvl )  {
	int  result = -1;
	int  scktOpt = 0;

/* What is the current setting */
	result = getIPv4_DontFragment( scktID, &scktOpt, verbosityLvl );
/* Were we able to read the DF setting ? */
    if( result < 0 )  {	/* No we were unable to read the DF setting so see if error message is required */
		if( verbosityLvl > 0 )  fprintf( stderr, "Error: Unable to read the current IPv4 header Don't Fragment setting\n" );
	}
	else  {		/* Yes we were able to read the DF setting */
		if( scktOpt != requiredDontFragmentValue )  {	/* not the required value so see if we can set it */
			result = setIPv4_DontFragment( scktID, requiredDontFragmentValue, verbosityLvl );
		    if( result < 0 )  {	/* No we were unable to set the DF setting so see if error message is required */
				if( verbosityLvl > 0 )
					fprintf( stderr, "Error: Unable to set the IPv4 header Don't Fragment setting to %d\n",  requiredDontFragmentValue );
			}
		}
	}
	return( result );
}


int  getSocketBroadcastPermission( int  scktID, int *  broadcastPermission, int  verbosityLvl )  {
	int  result, scktOpt;
	socklen_t  scktOptLen;

	scktOpt = 0;
	scktOptLen = sizeof( scktOpt );
	result = getsockopt( scktID, SOL_SOCKET, SO_BROADCAST, &scktOpt, &scktOptLen );
	if( result < 0 )  perror( "Error: getsockopt( Broadcast Permission )" );
	else  {
		if( verbosityLvl > 5 )  printf( "Broadcast Permission value is %d and size of Broadcast value is %u\n", scktOpt, scktOptLen );
		if( scktOptLen != sizeof( int ))  {
			fprintf( stderr, "Error: Socket Broadcast value is unexpected size of %u bytes\n", scktOptLen );
			result = -2;	/* flag error */
		}
		else  {
			*broadcastPermission = scktOpt;
		}
	}
	return( result );
}


int  setSocketBroadcastPermission( int  scktID,  int  broadcastPermission, int  verbosityLvl )  {
	int  result, scktOpt;

	scktOpt = broadcastPermission;
	result = setsockopt( scktID, SOL_SOCKET, SO_BROADCAST, &scktOpt, sizeof( scktOpt ));
	if( result < 0 )  {
		perror( "Error: setsockopt( Broadcast Permission )" );
		fprintf( stderr, "Error: Unable to set Broadcast Permission to %d\n", scktOpt );
	}
	else if( verbosityLvl > 5 )  printf( "Socket Broadcast Permission set to %d\n", scktOpt );
	return( result );
}


int  ensureSocketBroadcastPermissionSettingIsTheRequiredValue( int  scktID,  int  requiredBroadcastPermissionValue, int  verbosityLvl )  {
	int  result = -1;
	int  scktOpt = 0;

/* What is the current setting */
	result = getSocketBroadcastPermission( scktID, &scktOpt, verbosityLvl );
/* Were we able to read the Socket Broadcast Permission setting ? */
    if( result < 0 )  {	/* No we were unable to read the Socket Broadcast Permission setting so see if error message is required */
		if( verbosityLvl > 0 )  fprintf( stderr, "Error: Unable to read the current Socket Broadcast Permission setting\n" );
	}
	else  {		/* Yes we were able to read the Socket Broadcast Permission setting */
		if( scktOpt != requiredBroadcastPermissionValue )  {	/* not the required value so see if we can set it */
			result = setSocketBroadcastPermission( scktID, requiredBroadcastPermissionValue, verbosityLvl );
		    if( result < 0 )  {	/* No we were unable to set the Socket Broadcast Permission setting so see if error message is required */
				if( verbosityLvl > 0 )
					fprintf( stderr, "Error: Unable to set the Socket Broadcast Permission setting to %d\n",  requiredBroadcastPermissionValue );
			}
		}
	}
	return( result );
}

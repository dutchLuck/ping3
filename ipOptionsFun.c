/*
 * I P O P T I O N S F U N . C
 *
 * ipOptionsFun.c  last edited Mon Sep 18 20:29:11 2023
 *
 * Functions to handle IP Header Options in the IP packet.
 *
 */

#include <stdio.h>
#include <arpa/inet.h>			/* ntohl() */
#include <netinet/ip.h>			/* struct ip_timestamp */
#include <netinet/in_systm.h>	/* n_long */
#ifdef __linux__
#include "ipOptTS.h"	/* use a copy of the BSD option struct */
typedef  struct ipOptTimestamp  IP_TIMESTAMP;
#else
#include <netinet/ip_var.h>
typedef  struct ip_timestamp  IP_TIMESTAMP;
#endif
#include "timeFun.h"	/* convert...() */


int  getTimeStampOverflowCount( unsigned char *  ptr )  {
	IP_TIMESTAMP *  tsPtr;

	tsPtr = ( IP_TIMESTAMP * ) ptr;
	return(( int )( tsPtr->ipt_oflw ));
}


void  displayTimeStampOverflowCount( unsigned char *  ptr )  {
	int  timeStampListOverflowCount;

	timeStampListOverflowCount = getTimeStampOverflowCount( ptr );
	printf( "Time Stamp list overflow count is " );
	if( timeStampListOverflowCount == 0x0f )
		printf( "15 or more\n" );
	else
		printf( "%d\n", timeStampListOverflowCount );
}

void  displayOnlyGreaterThanZeroTimeStampOverflowCount( unsigned char *  ptr )  {
	int  timeStampListOverflowCount;

	timeStampListOverflowCount = getTimeStampOverflowCount( ptr );
	if( timeStampListOverflowCount > 0 )  displayTimeStampOverflowCount( ptr );
}


void  displayAbsTimeInMultipleFormats( unsigned int  timeStamp, int  verboseFlag )  {
	char  tmpStr[ 40 ];

	convertMilliSecondsSinceMidnightToHMS_String( timeStamp, tmpStr );
	printf( " %s ", tmpStr );
	if( verboseFlag )  printf( "( %u [mS] ( 0x%08x )) ", timeStamp, timeStamp );
}


void  displayDeltaTimeInMultipleFormats( int  deltaTimeStamp, int  verboseFlag )  {
	char  tmpStrDelta[ 40 ];

	if( deltaTimeStamp < 1000 && deltaTimeStamp > -1000 )
		printf( "%d [mS]", deltaTimeStamp );
	else {
		convertMilliSecondsToHMS_String( deltaTimeStamp, tmpStrDelta );
		printf( "%s [h:m:s]", tmpStrDelta );
	}
}


void  displayTimeAndDeltaTime( unsigned int  timeStamp, int  deltaTimeStamp, int  verboseFlag )  {
	displayAbsTimeInMultipleFormats( timeStamp, verboseFlag );
	printf( "( " );
	displayDeltaTimeInMultipleFormats( deltaTimeStamp, verboseFlag );
	printf( ")\n" );
}


void  displayTimeStamps( unsigned char *  ptr, int  len, int  startingTimeStamp, int  verboseFlag )  {
	int  byteCnt;
	unsigned int  timeStamp;
	int  deltaTimeStamp;
	IP_TIMESTAMP *  tsPtr;
	n_long *  timePtr;

	timePtr = ( n_long * )( ptr + 4 );
	tsPtr = ( IP_TIMESTAMP * ) ptr;
	deltaTimeStamp = startingTimeStamp;
	for( byteCnt = tsPtr->ipt_ptr - 5; byteCnt > 0; byteCnt -= sizeof( n_long ), timePtr++ )  {
		timeStamp = ntohl( *timePtr );
		deltaTimeStamp = ( int ) timeStamp - deltaTimeStamp;
		displayTimeAndDeltaTime( timeStamp, deltaTimeStamp, verboseFlag );
		deltaTimeStamp = ( int ) timeStamp;
	}
}


void  displayAddrTimeTimeStamps( unsigned char *  ptr, int  len, int  startingTimeStamp, int  verboseFlag )  {
	int  byteCnt, sizeofStructIpt_ta;
	unsigned int  timeStamp;
	int  deltaTimeStamp;
	IP_TIMESTAMP *  tsPtr;
	struct ipt_ta *  addr_timePtr;

	sizeofStructIpt_ta = sizeof( struct ipt_ta );
	tsPtr = ( IP_TIMESTAMP * ) ptr;
	addr_timePtr = ( struct ipt_ta * )( ptr + 4 );
	deltaTimeStamp = startingTimeStamp;
	for( byteCnt = tsPtr->ipt_ptr - 5; byteCnt > 0; byteCnt -= sizeofStructIpt_ta, addr_timePtr++ )  {
		timeStamp = ntohl( addr_timePtr->ipt_time );
		deltaTimeStamp = ( int ) timeStamp - deltaTimeStamp;
		printf( " %s,", inet_ntoa( addr_timePtr->ipt_addr ));
		displayTimeAndDeltaTime( timeStamp, deltaTimeStamp, verboseFlag );
		deltaTimeStamp = ( int ) timeStamp;
	}
}


void  displayIpOptionTimeStamps( unsigned char *  ptr, int  len, int  startingTimeStamp, int  verboseFlag )  {
	IP_TIMESTAMP *  tsPtr;

	tsPtr = ( IP_TIMESTAMP * ) ptr;
	if( tsPtr->ipt_flg == 0x3 )  displayAddrTimeTimeStamps( ptr, len, startingTimeStamp, verboseFlag );
	else if( tsPtr->ipt_flg == 0x1 )  displayAddrTimeTimeStamps( ptr, len, startingTimeStamp, verboseFlag );
	else if( tsPtr->ipt_flg == 0x0 )  displayTimeStamps( ptr, len, startingTimeStamp, verboseFlag );
	else  {
		printf( "?? Unknown IP4 option time stamp flag %d\n", tsPtr->ipt_flg );
	}
}


void  displayTimeStampOptionInHex( unsigned char *  bytePtr, int  optLen )  {
	int  byteCnt;

 /* Print the Option Header ( 4 bytes ) */
	for( byteCnt = 0; ( byteCnt < optLen ) && ( byteCnt < 4 ); byteCnt++ )  printf( " 0x%02x,", *bytePtr++ );
 /* Print the Option Body */
 	for( ; byteCnt < ( optLen - 1 ); byteCnt++ )  {
		if(( byteCnt - 4 ) % 8 != 0 )  printf( " 0x%02x,", *bytePtr++ );
		else  printf( "\n0x%02x,", *bytePtr++ );
	}
	printf( " 0x%02x\n", *bytePtr );
}


void  displayIpOptionTypeAndClass( unsigned char *  ptr )  {
	printf( "IP Option: ");
	switch( *ptr & 0x1f )  {
		case 0 : printf( " End of Option List" ); break;
		case 1 : printf( " No Operation" ); break;
		case 2 : printf( " Security" ); break;
		case 3 : printf( " Loose Source Routing" ); break;
		case 4 : printf( " Internet Time Stamp" ); break;
		case 7 : printf( " Record Route" ); break;
		case 8 : printf( " Stream ID" ); break;
		case 9 : printf( " Strict Source Routing" ); break;
		default : printf( "Unknown" ); break;
	}
	if(( *ptr & 0x80 ) == 0x80 )  printf( ", Copied" );
	else  printf( ", Not Copied" );
	switch(( *ptr & 0x60 ) >> 5 )  {
		case 0 : printf( ", Control Class" ); break;
		case 2 : printf( ", Debugging and Measurement Class" ); break;
		default : printf( ", Reserved for future use" ); break;
	}
	printf( "\n" );	
}


void  displayIpOptionList( unsigned char *  ptr, int  len )  {
	int  count, optLen;

	for( count = 0; count < len; count += optLen, ptr += optLen )  {
		/* Note that optLen = -1 represents variable length option */
		switch( *ptr & 0x1f )  {
			case 0 : optLen = 1; break;	/* End of Option List */
			case 1 : optLen = 1; break;	/* No Operation */
			case 2 : optLen = 11; break;	/* Security */
			case 3 : optLen = -1; break;	/* Loose Source Routing */
			case 4 : optLen = -1; break;	/* Internet Time Stamp */
			case 7 : optLen = -1; break;	/* Record Route */
			case 8 : optLen = 4; break;	/* Stream ID */
			case 9 : optLen = -1; break;	/* Strict Source Routing */
			default : optLen = 1; break;	/* Unknown */
		}
		if( optLen == -1 )  optLen = *( ptr + 1 );	/* assign the actual (variable) length */
		displayIpOptionTypeAndClass( ptr );
	}
}


void  displayIpOptions( unsigned char *  ptr, int  len, int  verboseFlag )  {
	int  count, optLen;
	IP_TIMESTAMP *  tsPtr;

	count = 0;
	while( count < len )  {
		optLen = -1;	/* -1 represents variable length option */
		switch( *ptr & 0x1f )  {
			case 0 : optLen = 1; break;	/* End of Option List */
			case 1 : optLen = 1; break;	/* No Operation */
			case 2 : optLen = 11; break;	/* Security */
			case 3 : optLen = -1; break;	/* Loose Source Routing */
			case 4 : optLen = -1;	/* Internet Time Stamp */
					tsPtr = ( IP_TIMESTAMP * ) ptr;
					printf( "Code %02x, Length %d, Pointer %d, Flag %x, Overflow %x\n",
						tsPtr->ipt_code, tsPtr->ipt_len, tsPtr->ipt_ptr, tsPtr->ipt_flg, tsPtr->ipt_oflw );
					displayIpOptionTimeStamps( ptr, len, 0, verboseFlag );
					if( verboseFlag )  {
						printf( "Time Stamp Option formatted as Hex; -\n");
						displayTimeStampOptionInHex( ptr, tsPtr->ipt_len );
					}
					break;
			case 7 : break;	/* Record Route */
			case 8 : optLen = 4; break;	/* Stream ID */
			case 9 : break;	/* Strict Source Routing */
			default : optLen = 1; break;	/* Unknown */
		}
		if( optLen == -1 )  optLen = *( ptr + 1 );
		count += optLen;
		printf( "Option Length %d, count %d, Total Length of option(s) %d\n", optLen, count, len );
	}
}
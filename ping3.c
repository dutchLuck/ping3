/*
 * P I N G 3 . C
 *
 * ping3.c last edited Thu Oct 12 23:49:22 2023
 * 
 */

/*
 * See if a remote Network Device is alive with (possibly multiple) ICMP
 * echo requests and replies encapsulated in IP version 4 datagrams. 
 *
 * On machines other than Apple Macs you will very likely need to have
 * priveleges provided by sudo to run this program ( or this program must
 * be suid to root) since it sends the ping datagram using a raw socket.
 * 
 */

#include <stdlib.h> 	/* exit() atexit() */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>	/* inet_addr() inet_ntoa() */
#include <sys/file.h>
#include <time.h>		/* clock_gettime() nanosleep() */
#include <sys/signal.h>
#include <libgen.h>		/* basename() */

#include <netinet/in_systm.h>
#include <netinet/in.h>	/* inet_addr() inet_ntoa() */
#include <arpa/inet.h>	/* inet_addr() inet_ntoa() */
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>	/* struct icmp */
#ifndef __linux__
#include <netinet/ip_var.h>
#endif
#include <netdb.h>		/* gethostbyname() */
#include <unistd.h>		/* gethostname() alarm() getopt() */
#include <stdio.h>		/* printf() fprintf() perror() */
#include <ctype.h>
#include <errno.h>
#include <string.h>		/* strncpy() */
#include <strings.h>	/* bzero() */
#include <math.h>		/* round() */
#include  "genFun.h"	/* clearByteArray() */
#include  "dbgFun.h"	/* printNamedByteArray() */
#include  "ipFun.h"		/* calcCheckSum() display_ip() */
#include  "ipOptionsFun.h"
#include  "icmpFun.h"	/* displayICMP() */
#include  "timeFun.h"	/* convertMilliSecondsSinceMidnightToHMS_String() calcTimeSpecClockDifferenceInSeconds() */

#ifndef  FALSE
#define  FALSE  ((int)0)
#endif
#ifndef  TRUE
#define  TRUE  (! FALSE)
#endif
#define EXTENDED_TIME_OUT_PERIOD  10	/* extended time out period in seconds */
#define DEFAULT_TIME_OUT_PERIOD  3	/* default time out period in seconds */
#define DEFAULT_PING_COUNT  2		/* default number of pings to send */
#define	MAX_IP4_PACKET_LEN	65535	/* max IP version 4 packet size - i.e. length is specified by 16 bit word */
#define	STD_IP4_HDR_LEN	20
#define MAX_IP4_HDR_OPTION_LEN	40	/* max IP version 4 extra header option length */
#define ICMP_HDR_LEN	8
#define	ICMP_PAYLOAD_LEN	12	/* default ICMP data length to 3 millisecond since midnight timestamps */
#define CONTRIVED_NUMBER 0x1	/* start number for packet identifier */

#define ICMP_TIMEW 3	/* ICMP Timestamp request with little endian replies expected */
#define ICMP_TIME 2		/* ICMP Timestamp request with standard replies expected */
#define ICMP_MASK 1		/* ICMP Mask request with standard replies expected */

#ifdef __linux__
#include "ipOptTS.h"
typedef  struct ipOptTimestamp  IP_TIMESTAMP;
#else
typedef  struct ip_timestamp  IP_TIMESTAMP;
#endif

/* Apple devices can use SOCK_DGRAM sockets to send ICMP without needing sudo privelege */
/* Other systems seem to require a raw socket for this purpose and also require sudo  privelege */
#ifdef __APPLE__
#define SOCK_TYPE_TO_USE SOCK_DGRAM
#else
#define SOCK_TYPE_TO_USE SOCK_RAW
#endif

extern int  errno;

/* Command line Optional Switches: */
/*  beginOffset, ByteCount, charAlternate, Cryptogram, decimal, Debug, */
/*  fieldSepWidth, help, Hex, Index, outFile, columnSeparator, space, verbosity, width */
const char  optionStr[] = "c:Dhl:M:qT:vw:";

int	 verboseFlag;	/* when true more info is output */
int	 quietFlag;		/* when true minimise the output info */
int	 helpFlag;		/* when true help message is printed and program exits */
int	 debugFlag;		/* when true output lots of info */
int	 haveLocalHostNameFlag;	/* Local host name is known if true */
int  c_Flag;		/* Ping count was found in the command line options */
char *  c_Strng = ( char * ) NULL;	/*  pointer to -c value as there should be a value */
int	 pingCount;		/* attempt pingCount pings */
int  l_Flag;		/* IP Header option length was found in the command line options */
char * l_Strng = ( char * ) NULL;	/* pointer to -l value */
int  optionLength;	/* IP Header Option length */
int  M_Flag;		/* ICMP Mask identifier was found in the command line options */
char *  M_Strng = ( char * ) NULL;	/* pointer to -M type value */
int  icmpTS_Value;	/* Specifies the time stamp replies as little endian i.e. Windows replies */
int  T_Flag;		/* IP4 Header options Time stamp identifier was found in the command line options */
char *  T_Strng = ( char * ) NULL;	/* pointer to -T value */
int  ip4_OptionTS_Value;	/* Specifies the type of time stamp request and is -1 for none */
int  w_Flag;		/*  Wait count was found in the command line options */
char *  w_Strng = ( char * ) NULL;	/* pointer to -w value */
int	 waitTimeInSec;		/* when true it waits for > 1 response */

char *  exeName;	/* name of this executable */
char *  exePath = ( char * ) NULL;	/* path of this executable */

int	 responsesToICMP_Request;	/* counts responses to the ICMP request */

int	 sckt;
char *  hostName;
u_char  ip4_DataToSend[ ICMP_HDR_LEN + ICMP_PAYLOAD_LEN + MAX_IP4_HDR_OPTION_LEN ];
u_short  ip4_Hdr_ID;	/* The identifier to put in the IP4 header */
int  packetLen;
u_char *  packet;		/* pointer to buffer to contain received IP4 ICMP reply packet */
u_short  process_id;	/* process ID */

struct timespec	 timeToBeStoredInSentICMP;	/* time stamp put in the sent Echo Request */
struct timespec	 timeStoredInReceivedICMP;	/* time stamp retrieved from the Echo Reply */
struct timespec	 origTime;	/* originate time stamp */
struct timespec  recvTime;	/* receive time stamp */
long  tsorig;	/* originate & receive timestamps */
long  tsrecv; 	/* timestamp = #millisec since midnight UTC */
long  tstarget;	/* both host byte ordered */
long  tsdiff;	/* adjustment must also be signed */
struct timespec  sleepTime;
struct timespec  sleepRemainder;

struct sockaddr	 remoteDeviceToPingInfo;	/* target network device to ping */
struct sockaddr	 preSpecDevice[ 4 ];	/* get timestamp from network up to 4 interfaces */

char  remoteDeviceNameBuffer[ MAXHOSTNAMELEN ];
char  localDeviceNameBuffer[ MAXHOSTNAMELEN ];
char  prespecDeviceNameBuffer[ 4 ][ MAXHOSTNAMELEN ];


void  sig_alrm( int  signo )  {
	if( responsesToICMP_Request == 0 )  {
		printf( "timeout\n" );
		exit( 1 );
	}
	else  exit( 0 );
}


void  printClockRealTimeRxTxTimes( void )  {
	printf( "Echo Request send time:  " );
	printTimeSpecTime( &origTime );
	printf( "\nEcho Reply receive time: " );
	printTimeSpecTime( &recvTime );
	printf( "\n" );
}


void  printClockRealTimeFlightTime( void )  {
	double  flightTimeInSec;

	flightTimeInSec = calcTimeSpecClockDifferenceInSeconds( &origTime, &recvTime );
	if( flightTimeInSec <= 0.5 )  printf( "RTT %lg [mS]", flightTimeInSec * 1000 );
	else   printf( "RTT %lg [Sec]", flightTimeInSec );
}


void  fillInICMP_HdrInfo( struct icmp *  hdrPtr, u_char  icmpType, u_short sequncID )  {
	hdrPtr->icmp_type = icmpType;
    hdrPtr->icmp_code = 0;
	hdrPtr->icmp_cksum = 0;	/* compute checksum later, after data is in place */
	hdrPtr->icmp_seq = htons( sequncID ); /* seq and id must be reflected in Echo/Timestamp/Netmask Reply*/
	hdrPtr->icmp_id = htons( process_id );
}


/*
 * Send the icmp mask request (RFC 950).
 */
int  sendICMP_MaskRequest( int  socketID, u_char *  ip4_Data, int  dataBytesCnt, u_short  seqID )  {
	ssize_t	 numberOfBytesSent;
	struct icmp	*  icmpHdrPtr;
	u_int32_t *  icmpMaskPtr;

	numberOfBytesSent = 0;
	if( debugFlag )  {
		printf( "Data Byte Count %d, Sequence ID 0x%04x", dataBytesCnt, seqID );
	}
/* Set up the ICMP info if there is enough room in the ip_Data buffer */
	if( dataBytesCnt > ICMP_HDR_LEN )  {
		clearByteArray( ip4_Data, dataBytesCnt );	/* ensure data is zero'd each time as this storage is reused */
		icmpHdrPtr = (struct icmp *) ip4_Data;
		fillInICMP_HdrInfo( icmpHdrPtr, ICMP_MASKREQ, seqID );
		icmpMaskPtr = ( u_int32_t * ) ( ip4_Data + 8 );		/* point to icmp Data area */
		if( dataBytesCnt >= 12 )  {	/* put in mask of zero if there is space */
			*icmpMaskPtr = htonl( 0 );
	    }
		 /* compute ICMP checksum */
		icmpHdrPtr->icmp_cksum = calcCheckSum((u_short *) icmpHdrPtr, dataBytesCnt );
		if( debugFlag )  {
			printNamedByteArray( ip4_Data, dataBytesCnt, 20, "ip4 Data payload (send array contents)" );
			printf( "\n" );
		}
		/* Send Packet */
		clock_gettime( CLOCK_REALTIME, &origTime );		/* remember time just before the packet is sent */
		numberOfBytesSent = sendto( socketID, (char *)ip4_Data, dataBytesCnt, 0, &remoteDeviceToPingInfo, sizeof( struct sockaddr ));
		tsorig = calcMillisecondsSinceMidnightFromTimeSpec( &origTime );
		if(( numberOfBytesSent < 0 ) && ( ! quietFlag ))  perror( "?? sendto error" );
		else if(( numberOfBytesSent != dataBytesCnt ) && ( ! quietFlag )) {
			printf("?? sendto() wrote %ld chars to %s, but it should have been %d chars\n",
				numberOfBytesSent, hostName, dataBytesCnt );
		}
	}
	return( numberOfBytesSent );
}


/*
 * Send the icmp timestamp request.
 */
int  sendICMP_TimestampRequest( int  socketID, u_char *  ip4_Data, int  dataBytesCnt, u_short  seqID )  {
	ssize_t	 numberOfBytesSent;
	struct icmp	*  icmpHdrPtr;
	u_int32_t *  icmpOrigTimestampPtr;

	numberOfBytesSent = 0;
	if( debugFlag )  {
		printf( "Data Byte Count %d, Sequence ID 0x%04x", dataBytesCnt, seqID );
	}
/* Set up the ICMP info if there is enough room in the ip_Data buffer */
	if( dataBytesCnt > ICMP_HDR_LEN )  {
		clearByteArray( ip4_Data, dataBytesCnt );	/* ensure data is zero'd each time as this storage is reused */
		icmpHdrPtr = (struct icmp *) ip4_Data;
		fillInICMP_HdrInfo( icmpHdrPtr, ICMP_TSTAMP, seqID );
		icmpOrigTimestampPtr = ( u_int32_t * ) ( ip4_Data + 8 );		/* point to icmp Data area */
		if( dataBytesCnt >= 20 )  {	/* put mS time in the icmp data section if there is space */
			clock_gettime( CLOCK_REALTIME, &timeToBeStoredInSentICMP );		/* get time to put in the packet about to be sent */
			*icmpOrigTimestampPtr = htonl( calcMillisecondsSinceMidnightFromTimeSpec( &timeToBeStoredInSentICMP ));
	    }
		 /* compute ICMP checksum */
		icmpHdrPtr->icmp_cksum = calcCheckSum((u_short *) icmpHdrPtr, dataBytesCnt );
		if( debugFlag )  {
			printNamedByteArray( ip4_Data, dataBytesCnt, 20, "ip4 Data payload (send array contents)" );
			printf( "\n" );
		}
		/* Send Packet */
		clock_gettime( CLOCK_REALTIME, &origTime );		/* remember time just before the packet is sent */
		numberOfBytesSent = sendto( socketID, (char *)ip4_Data, dataBytesCnt, 0, &remoteDeviceToPingInfo, sizeof( struct sockaddr ));
		tsorig = calcMillisecondsSinceMidnightFromTimeSpec( &origTime );
		if(( numberOfBytesSent < 0 ) && ( ! quietFlag ))  perror( "?? sendto error" );
		else if(( numberOfBytesSent != dataBytesCnt ) && ( ! quietFlag )) {
			printf("?? sendto() wrote %ld chars to %s, but it should have been %d chars\n",
				numberOfBytesSent, hostName, dataBytesCnt );
		}
	}
	return( numberOfBytesSent );
}


/*
 * Send the icmp echo request.
 */
int  sendICMP_EchoRequest( int  socketID, u_char *  ip4_Data, int  dataBytesCount, u_short  seqID )  {
	ssize_t	 numberOfBytesSent;
	struct icmp	*  icmpHdrPtr;
	u_char *  icmpDataPtr;

	numberOfBytesSent = 0;
/* Set up the ICMP info if there is enough room in the ip_Data buffer */
	if( dataBytesCount > ICMP_HDR_LEN )  {
		clearByteArray( ip4_Data, dataBytesCount );	/* ensure data is zero'd each time as this storage is reused */
		icmpHdrPtr = (struct icmp *) ip4_Data;
		fillInICMP_HdrInfo( icmpHdrPtr, ICMP_ECHO, seqID );
		icmpDataPtr = ip4_Data + 8;	/* point to icmp Data area */
		if( dataBytesCount >= 24 )  {	/* put nS time in the icmp data section if there is space */
			clock_gettime( CLOCK_REALTIME, &timeToBeStoredInSentICMP );		/* get time to put in the packet about to be sent */
			memcpy( (void *) icmpDataPtr, ( void * ) &timeToBeStoredInSentICMP, sizeof( struct timespec )); /* Don't worry about endianess */	
	    }
		 /* Compute ICMP checksum now that the data as well as the header are in place */
		icmpHdrPtr->icmp_cksum = calcCheckSum((u_short *) icmpHdrPtr, dataBytesCount );
		if( debugFlag )  {
			printNamedByteArray( ip4_Data, dataBytesCount, 20, "ip4 Data payload (send array contents)" );
			printf( "ICMP message check sum calculates as 0x%04x\n", calcCheckSum((u_short *) icmpHdrPtr, dataBytesCount ));
		}
		/* Send Packet */
		clock_gettime( CLOCK_REALTIME, &origTime );		/* remember time just before the packet is sent */
		numberOfBytesSent = sendto( socketID, (char *)ip4_Data, dataBytesCount, 0, &remoteDeviceToPingInfo, sizeof( struct sockaddr ));
		tsorig = calcMillisecondsSinceMidnightFromTimeSpec( &origTime );
		if(( numberOfBytesSent < 0 ) && ( ! quietFlag ))  perror( "?? sendto error" );
		else if(( numberOfBytesSent != dataBytesCount ) && ( ! quietFlag )) {
			printf("?? sendto() wrote %ld chars to %s, but it should have been %d chars\n",
				numberOfBytesSent, hostName, dataBytesCount );
		}
	}
	return( numberOfBytesSent );
}


/* Print the normal ping info */
/* E.g. XX bytes from AAA.BBB.CCC.DDD: seq YYYYY, ttl ZZZ, RTT 0.994 [mS] */
void  printStdPingInfo( struct ip *  ip, struct icmp *  icmpHdrPtr, int  ICMP_MsgSize )  {
	printPingCommonInfo( ip, ICMP_MsgSize );
	printf( " seq %d, ", ntohs( icmpHdrPtr->icmp_seq));
	printIPv4_TimeToLiveInfo( ip );
	printf( ", " );
	printClockRealTimeFlightTime();
}


void  printLineOfPingInfo( struct ip *  ip, struct icmp *  icmpHdrPtr, int  ICMP_MsgSize )  {
	printStdPingInfo( ip, icmpHdrPtr, ICMP_MsgSize );
	if( icmpHdrPtr->icmp_type == ICMP_TSTAMPREPLY )  {
		printf( " " );
		displayTimeStampReplyTimestamps( icmpHdrPtr, ( icmpTS_Value == ICMP_TIMEW ));
	}
	else if( icmpHdrPtr->icmp_type == ICMP_MASKREPLY )  {
		printf( " " );
		displayMaskReplyMask( icmpHdrPtr );
	}
	printf( "\n" );
	if( debugFlag )  printClockRealTimeRxTxTimes();
}


/*
 * Process a received datagram that should be an ICMP message.
 * Since we receive all ICMP messages that the kernel receives,
 * we may receive a type of ICMP message other than the
 * reply we're waiting for.
 */
int  processReceivedDatagram( char *  buf, int  datagramSize, struct sockaddr_in *  from )  {
	int  hdrLen;
	u_short  chkSumResult;
	struct icmp *  icmpHdrPtr;
	struct ip *  ip;
	IP_TIMESTAMP *  tsPntr;
	struct ipt_ta *	 ipt_taPtr;
	double  halfRoundTripTime;

	tsrecv = calcMillisecondsSinceMidnightFromTimeSpec( &recvTime );
 /* Check the IP datagram */
	ip = (struct ip *) buf;
	tsPntr = (IP_TIMESTAMP * ) ( buf + sizeof( struct ip ));
 /* Received datagram is assumed to be an IP4 datagram, but check to make sure */
    if( ip->ip_v != 4 )  {
		fprintf( stderr, "\n?? Datagram from %s version ( %x ) indicates it isn't an IP4 datagram\n", 
			inet_ntoa( *(struct in_addr *)&from->sin_addr.s_addr), ip->ip_v );
		printNamedByteArray(( u_char *) ip, datagramSize, 20, "processReceivedDatagram(): non IP4 datagram received" );
		return( -1 );
	}
	hdrLen = ip->ip_hl << 2;	/* determine the length of the IP datagram header */
	icmpHdrPtr = (struct icmp *)( buf + hdrLen );	/* point to the start of the datagram payload (i.e. ICMP message) */
	if( debugFlag )  {
		printf( "Datagram Length is %d - Header Length is %d\n", datagramSize, hdrLen );
		printNamedByteArray(( u_char *) ip, datagramSize, 20, "processReceivedDatagram(): Complete IP4 datagram received" );
	}
	ip->ip_len = htons( datagramSize );		/* restore total length in header in case it has been altered */
	if(( chkSumResult = calcCheckSum( (u_short * ) ip, hdrLen )) != 0 )  {
		if( hdrLen == STD_IP4_HDR_LEN || verboseFlag )  {	/* Don't report if header options are in use - FIX checksum for options use */
			fprintf( stderr, "\n?? Datagram from %s has invalid IP header checksum result 0x%04x\n", 
				inet_ntoa( *(struct in_addr *)&from->sin_addr.s_addr), chkSumResult );
		 /* Try zero the time option if there is one */
			if( debugFlag )
				printNamedByteArray(( u_char *) ip, datagramSize, 20, "processReceivedDatagram(): IP4 header checksum failed datagram" );
		}
	}
 /* Received datagram is assumed to be an ICMP datagram, but check to make sure */
    if( ip->ip_p != 0x01 )  {
		fprintf( stderr, "\n?? Datagram from %s header protocol ( %x ) isn't ICMP message\n", 
			inet_ntoa( *(struct in_addr *)&from->sin_addr.s_addr), ip->ip_p );
		if( verboseFlag )
			printNamedByteArray(( u_char *) ip, datagramSize, 20, "processReceivedDatagram(): non ICMP IP4 received" );
		return( -1 );
	}
	if ( datagramSize < ( hdrLen + ICMP_MINLEN )) {
		fprintf( stderr, "\n?? Datagram from %s too short (%d bytes) to contain ICMP message\n", 
			inet_ntoa( *(struct in_addr *)&from->sin_addr.s_addr), datagramSize );
		if( verboseFlag )
			printNamedByteArray(( u_char *) ip, datagramSize, 20, "processReceivedDatagram(): too short ICMP IP4 received" );
		return( -1 );
	}
	if( debugFlag )  {
		printf( "\n--==## processReceivedDatagram(): display IP4 datagram ##==--\n" );
		display_ip( ip, datagramSize );
		printf( "--==## processReceivedDatagram(): display IP4 datagram ##==--\n\n" );
	}
     /* Now the ICMP body + header options (if there are any) */
	
     /* With a raw ICMP socket we get all ICMP packets that come into the kernel. Only accept Echo or Timestamp reply */
	if( ! (( icmpHdrPtr->icmp_type == ICMP_ECHOREPLY ) ||
	       ( icmpHdrPtr->icmp_type == ICMP_TSTAMPREPLY ) ||
	       ( icmpHdrPtr->icmp_type == ICMP_MASKREPLY )))  {
		if( ! quietFlag )  {
			printf( "\n? unexpected icmp type 0x%x, sub code 0x%x, from %s\n", icmpHdrPtr->icmp_type,
				icmpHdrPtr->icmp_code, inet_ntoa(*(struct in_addr *) &from->sin_addr.s_addr ));
			if( verboseFlag )  displayICMP( icmpHdrPtr );
		}
		if( verboseFlag )
			printNamedByteArray(( u_char *) ip, datagramSize, 20, "processReceivedDatagram(): non Echo/Timestamp/Mask Reply ICMP IP4 received" );
		return( -1 );	/* some other type of ICMP message */
	}
	if( ntohs( icmpHdrPtr->icmp_id ) != process_id )  {
		printf("? processReceivedDatagram(): received id 0x%04x but expected 0x%04x\n",
			ntohs( icmpHdrPtr->icmp_id ), process_id );
		if( debugFlag )  display_ip( ip, datagramSize );
	}
	if( ntohs( icmpHdrPtr->icmp_seq ) != ip4_Hdr_ID )  {
		printf( "? processReceivedDatagram(): received sequence # %d but expected %d\n",
			ntohs( icmpHdrPtr->icmp_seq ), ip4_Hdr_ID );
		if( debugFlag )  display_ip( ip, datagramSize );
	}
	if( hdrLen == STD_IP4_HDR_LEN )  {
		printLineOfPingInfo( ip, icmpHdrPtr, datagramSize );
	}
	else  {
		/* Print the normal ping info E.g. XX bytes from AAA.BBB.CCC.DDD: seq YYYYY, ttl ZZZ, RTT 0.994 [mS] */
		printLineOfPingInfo( ip, icmpHdrPtr, datagramSize );
		if(( hdrLen - STD_IP4_HDR_LEN ) > 0 )  {	/* when true there is a option section in the IP4 header */
			if( debugFlag )  displayIpOptionList(( u_char *) tsPntr, hdrLen - STD_IP4_HDR_LEN );
		}
		if(( hdrLen - STD_IP4_HDR_LEN ) > 4 )  {	/* when true there is at least a valid option header to work on */
			if( verboseFlag )  {
				printf( " Local Tx time:" );
				displayAbsTimeInMultipleFormats( calcMillisecondsSinceMidnightFromTimeSpec( &origTime ), verboseFlag );
				printf( "\n" );
			}
			displayIpOptionTimeStamps( ( u_char *) tsPntr, hdrLen - STD_IP4_HDR_LEN,
				calcMillisecondsSinceMidnightFromTimeSpec( &origTime ), verboseFlag );
			if( verboseFlag )  {
				printf( " Local Rx time:" );
				displayAbsTimeInMultipleFormats( calcMillisecondsSinceMidnightFromTimeSpec( &recvTime ), verboseFlag );
				printf( "\n" );
			}
			displayOnlyGreaterThanZeroTimeStampOverflowCount(( u_char *) tsPntr );
		}
		/* tsorig and tsrecv are both signed longs.  The icmp time
		 * members in the structure are unsigned, but the max value
		 * for the #millisec since midnight is (24*60*60*1000 - 1)
		 * or 86,399,999, which easily fits into a signed long.
		 * We want them signed to compute a signed difference. */
		halfRoundTripTime = round( calcTimeSpecClockDifferenceInSeconds( &origTime, &recvTime ) * ( double ) 500.0 );
		tstarget = ntohl( tsPntr->ipt_timestamp.ipt_ta[ 0 ].ipt_time );
		ipt_taPtr = &tsPntr->ipt_timestamp.ipt_ta[ 0 ];
		ipt_taPtr += 1;
		/* ipt_taPtr->ipt_addr = ( struct in_addr ) ( rxdIP4_Packet->ip_dst ); */
		tsrecv = ntohl( ipt_taPtr->ipt_time );
#ifdef DEBUG
		if( debugFlag )  {
			printNamedByteArray(( u_char *) tsPntr, hdrLen - STD_IP4_HDR_LEN, 20, "processReceivedDatagram(): IP4 option received" );
			displayIpOptions(( u_char *) tsPntr, hdrLen - STD_IP4_HDR_LEN );
		}
#endif
		 /* Now get time according to target when return packet left */
    	 /* Assume packets travel was same speed in both directions */
    	 /* Thus time here when packet was actually processed by target */
    	 /* is  tsorig + 0.5 * tsRTT, now subtract that from target time */
		tstarget += ( int ) halfRoundTripTime;	/* adjusted time */		
		tsdiff = tsrecv - tstarget;		/* difference in millisec */
		if(( ip4_OptionTS_Value == IPOPT_TS_PRESPEC ) && verboseFlag ) { /* time difference output only if known and verbose required */
			if( tsrecv < 0 )
				printf( "? Non standard time format %ld ms (0x%lx)", tsrecv, tsrecv );
			printf( "Local device clock is " );
			if( tsdiff < 0 )  {
				displayDeltaTimeInMultipleFormats( -1 * tsdiff, verboseFlag );
				printf(" behind " );
			}
			else  {
				displayDeltaTimeInMultipleFormats( tsdiff, verboseFlag );
				printf(" ahead of " );
			}
			printf( "Remote device clock\n" );
		}
		if( debugFlag )
			printf( "Orig = %ld, Target %ld, Recv = %ld, Estimated diff = %ld\n",
				tsorig, tstarget, tsrecv, tsdiff );
	} 
	return( 0 );	/* done */
}


int sendICMP_Request(  int  socketID, u_char *  ip4_Data, int  dataBytesCount, u_short  seqID )  {
	int  successFlag;
	int  bytesSent;

	successFlag = FALSE;
	if( M_Flag && ( icmpTS_Value > 1 ))  {
		bytesSent = sendICMP_TimestampRequest( socketID, ip4_Data, 20, seqID );	/* ICMP Time stamp is 8 byte Header + 12 byte Data */
		successFlag = ( bytesSent == 20 );
		if( ! successFlag )
			printf( "?? Unable to send 20 bytes of ICMP Timestamp Request in the network datagram" );
	}
	else if( M_Flag && ( icmpTS_Value == 1 ))  {
		bytesSent = sendICMP_MaskRequest( socketID, ip4_Data, 12, seqID );	/* ICMP Mask is 8 byte Header + 4 byte Data */
		successFlag = ( bytesSent == 12 );
		if( ! successFlag )
			printf( "?? Unable to send 12 bytes of ICMP Mask Request in the network datagram" );
	}
	else  {
		bytesSent = sendICMP_EchoRequest( socketID, ip4_Data, dataBytesCount, seqID );
		successFlag = ( bytesSent == dataBytesCount );
		if( ! successFlag )
			printf( "?? Unable to send %d bytes of ICMP Echo Request in the network datagram", dataBytesCount );
	}
	return( successFlag );	
}


int sendICMP_RequestAndGetResponse( int socketID, int  dataByteCnt, u_short  ip4_HdrID )  {
	int  response = 0;
	socklen_t  fromLen;
	struct sockaddr_in  from;
	register int  numOfBytesRcvd;

	if( ! sendICMP_Request( sckt, ip4_DataToSend, dataByteCnt, ip4_HdrID ))  {	/* send the stock standard icmp echo request */
		printf( "?? Unable to send the expected number of bytes in the network datagram\n" );
	}
	else {
		if( waitTimeInSec > 0 )  alarm( waitTimeInSec );	/* Specify 1 to 15 second time limit */
		else  alarm( DEFAULT_TIME_OUT_PERIOD );	/* default to 2 second time limit for a response */
		for( response = 0;; )  {
			fromLen = sizeof( from );
			if(( numOfBytesRcvd = recvfrom( socketID, (char *)packet, packetLen, 0,
			    ( struct sockaddr *) &from, &fromLen )) < 0 ) {
				clock_gettime( CLOCK_REALTIME, &recvTime );	/* Remember time of error */
				if( errno == EINTR )
					continue;
				perror( "?? recvfrom error" );
				continue;
			}
			clock_gettime( CLOCK_REALTIME, &recvTime );		/* Remember time of received ICMP packet */
			/* process received ICMP message */
			if( processReceivedDatagram((char *)packet, numOfBytesRcvd, &from ) == 0 )
				response++;	/* terminate if reply received */
			if(( waitTimeInSec > 0 ) && ( response > 0 ))  break;
		}
		alarm( 0 );	/* Cancel timeout alarm */
	}
	return( response );
}

/*
 * IP Header Options are in RFC 791, but also see RFC 1349, RFC 2474, RFC 6864
 */
int  sendICMP_RequestWithTimeStampOptionInTheIP4_Hdr( struct sockaddr_in *  to, struct ip *  rxdIP4_Packet, u_short  ip4_HdrID )  {
	int  i, returnValue = 1;
	u_char  inetTimeStamp[ MAX_IP4_HDR_OPTION_LEN ];	/* Storage for optional IP4 header options */
	IP_TIMESTAMP *  tsPtr;		/* time stamp pointer */
	struct ipt_ta *	 ipt_taPtr;
	struct sockaddr_in *  sckAddrInPtr;
	
	tsPtr = (IP_TIMESTAMP *) inetTimeStamp;
	bzero((char *) tsPtr, MAX_IP4_HDR_OPTION_LEN );
	ipt_taPtr = &tsPtr->ipt_timestamp.ipt_ta[ 0 ];
	sckAddrInPtr = ( struct sockaddr_in * ) &preSpecDevice[ 0 ];
	ipt_taPtr->ipt_addr = sckAddrInPtr->sin_addr;
	if(( ip4_OptionTS_Value == IPOPT_TS_PRESPEC ) && ( ipt_taPtr->ipt_addr.s_addr == 0 ))  {		/* No prespecified addresses ? */
		tsPtr->ipt_timestamp.ipt_ta[0].ipt_addr = to->sin_addr;		/* insert remote network device IP address */
		tsPtr->ipt_timestamp.ipt_ta[0].ipt_time = 0;
		ipt_taPtr = &tsPtr->ipt_timestamp.ipt_ta[0];
		ipt_taPtr += 1;
		ipt_taPtr->ipt_addr = ( struct in_addr ) ( rxdIP4_Packet->ip_dst );
		ipt_taPtr->ipt_time = 0;
		optionLength = 20;	/* Over-ride option size to 4 octets for option header + 16 for two address + time pairs */
	}
	else if( ip4_OptionTS_Value == IPOPT_TS_PRESPEC )  {		/* Only set up prespecified addresses if 3 */
		optionLength = 0;
		for( i = 0; ( i < 4 ); i++ )  {
			sckAddrInPtr = ( struct sockaddr_in * ) &preSpecDevice[ i ];
			ipt_taPtr->ipt_addr = sckAddrInPtr->sin_addr;	/* insert remote network device IP address */
			if( ipt_taPtr->ipt_addr.s_addr == 0 )  break;	/* Drop out of loop if IP address just used was 0.0.0.0 */
			ipt_taPtr->ipt_time = 0;
			ipt_taPtr += 1;
			optionLength += 8;	/* Address + time pair is a total of 8 bytes */
		}
		if( optionLength > 0 )  optionLength += 4;	/* Add 4 octets for option header to address + time pairs */
	}
	else if( ip4_OptionTS_Value == IPOPT_TS_TSANDADDR )  {
		if( optionLength < 12 )  optionLength = 12;		/* Makes no sense to be smaller than header + 1 address + time pair */
		else  optionLength = 4 + ( 8 * (( optionLength - 4 ) / 8 ));	/* trim to header + an exact number of pairs */
	}
	/* set IPv4 Header Options header values */
	tsPtr->ipt_code = IPOPT_TS;
	tsPtr->ipt_ptr = 5;
	tsPtr->ipt_oflw = 0;
	tsPtr->ipt_len = optionLength;
	tsPtr->ipt_flg = ip4_OptionTS_Value;	/* tsonly is 0, tsandaddr 1, tsprespec 3 */
	if( setsockopt( sckt, IPPROTO_IP, IP_OPTIONS, tsPtr, optionLength ) != 0 )  {
		perror("?? main(): setsocketopt() unable to set up time stamp option" );
	}
	else  {
		if( debugFlag ) {
			printNamedByteArray( ( u_char *) tsPtr, optionLength, 20, "inetTimeStamp Structure" );
			printf( "\n" );
			displayIpOptions( ( u_char *) tsPtr, optionLength, verboseFlag );
		}
		sendICMP_RequestAndGetResponse( sckt, ICMP_HDR_LEN + ICMP_PAYLOAD_LEN, ip4_HdrID );
		returnValue = 0;
	}
	return( returnValue );
}


int  setUpIP_AddressAndName( struct sockaddr_in *  devInfoPtr, char *  devNameOrIP_Str, char *  nameBfr )   {
	int  i, resultOK = FALSE;
	struct hostent *  hostEntPtr;		/* host pointer */

/* Set up the address to send the icmp echo request */
	devInfoPtr->sin_family = AF_INET;
	hostEntPtr = NULL;	/* Make sure host entry pointer is initialized to NULL for later test */
     /* Try to convert command line Remote Network Device specifier as dotted quad (decimals) address, */
     /* else if that fails assume it's a Network Device name */
	resultOK = (( devInfoPtr->sin_addr.s_addr = inet_addr( devNameOrIP_Str )) != (u_int) -1 );	/* attempt to set address from a Dotted Quad IP address */
	if( ! resultOK )  {	/* test for failure to set address through inet_addr() */
		resultOK = (( hostEntPtr = gethostbyname( devNameOrIP_Str )) != NULL );	
		if( ! resultOK )  {
			if( verboseFlag )  fprintf( stderr, "?? gethostbyname() put h_errno = %d for target \"%s\"\n", h_errno, devNameOrIP_Str );
			fprintf( stderr, "?? Network Device \"%s\": %s\n", devNameOrIP_Str, hstrerror( h_errno ));
		}
		else  {
			devInfoPtr->sin_family = hostEntPtr->h_addrtype;
			bcopy( hostEntPtr->h_addr, (caddr_t)&devInfoPtr->sin_addr, hostEntPtr->h_length);
			strncpy( nameBfr, hostEntPtr->h_name, sizeof( nameBfr ) - 1);	/* copy official name of remote Network Device */
			if( verboseFlag )  {
				printf( "Remote Network Device official name is: \"%s\"\n", nameBfr );
			 /* Print other names of same Network Device if there are any */
				for( i = 0; hostEntPtr->h_aliases[ i ] != NULL; i++ )  {
					printf( "Remote Network Device name alias %d is: \"%s\"\n", i + 1, hostEntPtr->h_aliases[ i ] );
				}
			}
		}
	}
	if( resultOK && verboseFlag )  {
		printf( "Remote Network Device is: %s\n", inet_ntoa( *(struct in_addr *) &devInfoPtr->sin_addr.s_addr ));
	 /* Print other addresses if there are any */
	    if( hostEntPtr != NULL )  {
			for( i = 1; hostEntPtr->h_addr_list[ i ] != NULL; i++ )  {
				printf( "Remote Network Device alias %d is: \"%s\"\n", i, inet_ntoa( *(struct in_addr *) hostEntPtr->h_addr_list[ i ] ));
			}
		}
	}
	return( resultOK );
}


/* Help/Usage information */
void  useage( char *  name )  {
	printf( "\nuseage: %s [-cX][-D][-h][-lXX][-M ABC][-q][-T ABC][-v][-wX] NetworkDeviceName\n", name );
	printf( "or      %s [-cX][-D][-h][-lXX][-M ABC][-q][-T ABC][-v][-wX] NetworkDeviceIP_Number\n", name );
	printf( "\nwhere options; -\n" );
	printf( "        -cX  specifies number of times to ping remote network device\n" );
	printf( "        -D  switches on debug output\n" );
	printf( "        -h  switches on this help output and then terminates %s\n", name );
	printf( "        -lXX  specifies header option length (max accepted is 40 and should be a multiple of 4)\n" );
	printf( "        -M ABC  specifies ping with ICMP Mask/Timestamp request instead of ICMP Echo.\n" );
	printf( "          where ABC is a sting of characters.\n" );
	printf( "            If \"mask\" then send ICMP Mask request,\n" );
	printf( "            if \"time\" then send ICMP Time Stamp request,\n" );
	printf( "            if \"timew\" as above, but treat tsr and tst timestamps as little endian.\n" );
	printf( "            (i.e. Windows default response, if the ICMP Time Stamp request\n" );
	printf( "            is allowed through the Windows firewall. )\n" );
	printf( "        -q  forces quiet (minimum) output and overrides -v\n" );
	printf( "        -T ABC  specifies header option time stamp type.\n" );
	printf( "          where ABC is a sting of characters.\n" );
	printf( "            If \"tsonly\" then record Time Stamp Only list of time stamps,\n" );
	printf( "            if \"tsandaddr\" then record Address and Time Stamp pair list,\n" );
	printf( "            if \"tsprespec H.I.J.K [ L.M.N.O [ P.Q.R.S [ T.U.V.W ]]]\" then Time Stamp prespecified Addresses.\n" );
	printf( "        -v  switches on verbose output\n" );
	printf( "        -wX  ensures the program waits for X seconds for a response\n" );
	printf( "\n" );
} 


int  processCommandLineOptions( int  argc, char *  argv[] )  {
	int  i, result, returnValue;
	char ** argPtr;
	char *  argArray[ 5 ];

	/* Set all the global flags from command line options */
	opterr = 0;	/* Suppress error messages from getopt() to stderr */
	/* Process switch options from the command line */
	while(( result = getopt( argc, argv, optionStr )) != -1 )  {
    	switch( result )  {
    		case 'c' :  c_Flag = TRUE; c_Strng = optarg; break;
			case 'D' :  debugFlag = TRUE; break;
			case 'h' :  helpFlag = TRUE; break;
			case 'l' :  l_Flag = TRUE; l_Strng = optarg; break;
			case 'M' :  M_Flag = TRUE; M_Strng = optarg; break;
			case 'q' :  quietFlag = TRUE; break;
    		case 'T' :  T_Flag = TRUE; T_Strng = optarg; break;
    		case 'v' :  verboseFlag = TRUE; break;
    		case 'w' :  w_Flag = TRUE; w_Strng = optarg; break;
    		default :
        		printf( "Warning: command line option -%c is unrecognised or incomplete and has been ignored\n", optopt );
        		printf( "Information: For help on command line options run \"%s -h\"\n", exeName );
        		break;
    	}
	}

	pingCount = convertOptionStringToInteger( 3, c_Strng, "-c", &c_Flag, TRUE );
	pingCount = limitIntegerValueToEqualOrWithinRange( pingCount, 1, 128 );
	optionLength = convertOptionStringToInteger( MAX_IP4_HDR_OPTION_LEN, l_Strng, "-l", &l_Flag, TRUE );
	optionLength = 4 * (( optionLength + 3 ) / 4 );	/* force option length to be a multiple of 4 bytes */
	optionLength = limitIntegerValueToEqualOrWithinRange( optionLength, 8, MAX_IP4_HDR_OPTION_LEN );
	ip4_OptionTS_Value = -1;
	if( T_Flag && ( T_Strng != NULL ))  {
		if( strncmp( T_Strng, "tsonly", 6 ) == 0 )  ip4_OptionTS_Value = IPOPT_TS_TSONLY;
		else if( strncmp( T_Strng, "tsandaddr", 9 ) == 0 )  ip4_OptionTS_Value = IPOPT_TS_TSANDADDR;
		else if( strncmp( T_Strng, "tsprespec", 9 ) == 0 )  {
			ip4_OptionTS_Value = IPOPT_TS_PRESPEC;
			for( argPtr = argArray; ( *argPtr = strsep( &T_Strng, " \t,")) != NULL; )  {
				if( **argPtr != '\0' )
					if( ++argPtr >= &argArray[ 5 ])  break;
			}
			if( verboseFlag || debugFlag )
				for( i = 0; ( argArray[ i ] != NULL ) && ( i < 5 ); ++i )
					printf( "Time Stamp Prespec argArray[ %d ] is \"%s\"\n", i, argArray[ i ]);
			returnValue = TRUE;
			for( i = 0; ( returnValue && ( i < 4 ) && ( argArray[ i + 1 ] != NULL ) && ( *argArray[ i + 1 ] != '\0' )); i++ )
				returnValue = setUpIP_AddressAndName(( struct sockaddr_in *) &preSpecDevice[ i ], argArray[ i + 1 ], prespecDeviceNameBuffer[ i ] );
		}
	}
	ip4_OptionTS_Value = limitIntegerValueToEqualOrWithinRange( ip4_OptionTS_Value, -1, IPOPT_TS_PRESPEC );
	icmpTS_Value = 0;
	if( M_Flag && ( M_Strng != NULL ))  {
		if( strncmp( M_Strng, "mask", 4 ) == 0 ) icmpTS_Value = ICMP_MASK;
		else if( strncmp( M_Strng, "timew", 5 ) == 0 ) icmpTS_Value = ICMP_TIMEW;
		else if( strncmp( M_Strng, "time", 4 ) == 0 ) icmpTS_Value = ICMP_TIME;
	}
	icmpTS_Value = limitIntegerValueToEqualOrWithinRange( icmpTS_Value, 0, ICMP_TIMEW );
	waitTimeInSec = convertOptionStringToInteger( DEFAULT_TIME_OUT_PERIOD, w_Strng, "-w", &w_Flag, TRUE );
	waitTimeInSec = limitIntegerValueToEqualOrWithinRange( waitTimeInSec, 1, 15 );
 	if( debugFlag )  printf( "Option length is %d bytes\n", optionLength );
    /* quiet flag over-rides verbose flag if they are both TRUE */
	if( quietFlag )  verboseFlag = FALSE;
	/* return the index of the first positional argument (i.e. input file name?) */
	return( optind );
}


/* Preset switch option Flags and Data */
void  setGlobalFlagDefaults( char *  argv[] )  {	/* Set up any Global variables not already set at declaration */
	int  i;

	helpFlag = debugFlag = verboseFlag = quietFlag = FALSE;
	c_Flag = FALSE;
	pingCount = DEFAULT_PING_COUNT;
	l_Flag = FALSE;
	optionLength = MAX_IP4_HDR_OPTION_LEN;
	M_Flag = FALSE;		/* ICMP Mask/Time request */
	icmpTS_Value = 0;	/* No ICMP Mask/Time request */
	T_Flag = FALSE;		/* IP header option Timestamp */
	ip4_OptionTS_Value = -1;	/* I.E. no IP header option time stamp */
	w_Flag = FALSE;		/* Wait time */
	waitTimeInSec = DEFAULT_TIME_OUT_PERIOD;
	responsesToICMP_Request = 0;
	bzero((char *) &remoteDeviceToPingInfo, sizeof( struct sockaddr ));
	for( i = 0; i < 4; i++ )  {
		bzero((char *) &preSpecDevice[ i ], sizeof( struct sockaddr ));
		bzero( &prespecDeviceNameBuffer[ i ], sizeof( prespecDeviceNameBuffer[ i ] ));
	}
	bzero( remoteDeviceNameBuffer, sizeof( remoteDeviceNameBuffer ));
	bzero( localDeviceNameBuffer, sizeof( localDeviceNameBuffer ));
	process_id = (u_short)( getpid() & 0xffff );
    /* Isolate the name of the executable */
    exeName = argv[0];	/* set global variable to default */
    if(( exePath = strdup( argv[0] )) == NULL )
    	perror( "Warning: Unable to create duplicate of path to this executable" );
    else if(( exeName = basename( exePath )) == NULL )  {
    	perror( "Warning: Unable to obtain the name of this executable" );
    	exeName = argv[0];	/* set back to the default again */
    }
}


void  cleanupStorage( void )  {
	if( exePath != NULL )  free(( void *) exePath );
}


int  main( int  argc, char *  argv[] )  {
	int  i, returnValue, commandLineIndex;
	struct sockaddr_in *  to;	/* pointer to remote network device info */
	struct protoent	*  proto;	/* allows protocol to be set to ICMP */

/* Ensure any allocated memory is free'd */
	atexit( cleanupStorage );

/* Preset Global variables, switch option Flags and Data */
	setGlobalFlagDefaults( argv );	/* Set up any Global variables not already set at declaration */

/* Preset Local variables, option Flags and Data */
	returnValue = 1;

/* Process switch options from the command line */
	commandLineIndex = processCommandLineOptions( argc, argv );

/* Print useage message and exit if the "-h" flag was amongst the command line options */
/*  or if no icmp echo target is specified ) */
	if( helpFlag || ( argc == commandLineIndex )) {
		useage( exeName );
		return(0);
	}

/* Attempt to get the name of the Local network device */	
	haveLocalHostNameFlag = ( gethostname( localDeviceNameBuffer, MAXHOSTNAMELEN ) != -1 );
	if( ! haveLocalHostNameFlag )  {
		if( verboseFlag )  printf( "? unable to get actual Local host name, using 'localhost' instead.\n" );
		strncpy( localDeviceNameBuffer, "localhost", sizeof( localDeviceNameBuffer ));
	}
	else if( verboseFlag )  printf( "Local Network Device name is: '%s'\n", localDeviceNameBuffer );

/* Set up the address to send the icmp echo request */
	to = ( struct sockaddr_in *) &remoteDeviceToPingInfo;
	returnValue = setUpIP_AddressAndName( to, argv[ commandLineIndex ], remoteDeviceNameBuffer);
	if( returnValue == FALSE )  return( 1 );	/* Don't go further if no address can be determined */

	packetLen = STD_IP4_HDR_LEN + MAX_IP4_HDR_OPTION_LEN + sizeof( ip4_DataToSend );
	if(( packet = (u_char *) malloc( (u_int) packetLen )) == NULL ) {
		fprintf( stderr, "?? Unable to create storage of %d bytes for datagram data\n", packetLen );
	}
	else if(( proto = getprotobyname( "icmp" )) == NULL ) {
		fprintf(stderr, "?? Unknown protocol ICMP\n");
	}
	else if(( sckt = socket( AF_INET, SOCK_TYPE_TO_USE, proto->p_proto )) < 0 ) {
		perror("?? Unable to create socket to send ICMP packets");
	}
	else  {
		/* Do first ping WITHOUT IP4 header option, just to make sure ICMP echo/echo-reply is working */
		/*
		 * We send one request and wait TIME_OUT_PERIOD seconds for a reply.
		 */
		signal( SIGALRM, sig_alrm );

		ip4_Hdr_ID = ( u_short ) CONTRIVED_NUMBER;
		sendICMP_RequestAndGetResponse( sckt, ICMP_HDR_LEN + ICMP_PAYLOAD_LEN + MAX_IP4_HDR_OPTION_LEN, ip4_Hdr_ID );

 		/* Do more pings if required */
		for( i = 1; i < pingCount; i++ )  {
	 		/* Set up a break between echo requests */
    		sleepTime.tv_sec = 0L;
			sleepTime.tv_nsec = 500000000L;	/* half a second sleep time */
			nanosleep( &sleepTime, &sleepRemainder );
			ip4_Hdr_ID += 1;	/* bump the packet ID */
	    	if( ip4_OptionTS_Value < IPOPT_TS_TSONLY )  {
			/* Do second & greater pings as normal ICMP echo without any IP header options */
				sendICMP_RequestAndGetResponse( sckt, ICMP_HDR_LEN + ICMP_PAYLOAD_LEN + MAX_IP4_HDR_OPTION_LEN, ip4_Hdr_ID );	/* Another ping like the first */
				returnValue = 0;
			}
			else  {
			/* Do second & greater pings with IP4 header option requesting time stamp info */
				returnValue = sendICMP_RequestWithTimeStampOptionInTheIP4_Hdr( to, ( struct ip * ) packet, ip4_Hdr_ID );
			}
		}
	}
	return( returnValue );
}

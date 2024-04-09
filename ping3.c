/*
 * P I N G 3 . C
 *
 * ping3.c last edited Tue Apr  9 20:58:27 2024
 * 
 * v0.9.9 Added arrays indexed by sequence ID to track missing replies
 * v0.9.8 Added attempt to turn target given as an IPv4 address into a named host
 * v0.9.7 Reworked include files, added icmpDefn.h to project, compiles on CYGWIN
 * v0.9.5 Increased ICMP_PAYLOAD_MAX_PATTERN_SIZE to 28
 * v0.9.4 Fix compile error in Don't Fragment processing on macOS before Big Sur
 */

/*
 * See if a remote Network Device is alive with (possibly multiple) ICMP
 * echo requests and replies encapsulated in IP version 4 datagrams. 
 *
 * On machines other than Apple Macs you will very likely need to have
 * priveleges provided by sudo to run this program ( or this program must
 * be suid to root) since it sends the ping datagram using a raw socket.
 */

/*
 * May be compiled with gcc compatible compiler as follows; -
 *  cc -Wall -UDEBUG -o ping3 ping3.c genFun.c dbgFun.c ipFun.c icmpFun.c ipOptionsFun.c timeFun.c
 */

/*
 * Verify output of  ./ping3 -v1 NetworkDevice  on macOS
 * by comparing with the output of  ping -c3 NetworkDevice
 *
 * Verify output of  ./ping3 -M mask LocalNetworkDevice  on macOS
 * by comparing with the output of  ping -c3 -Mm -s0 LocalNetworkDevice
 * 
 * Verify output of  ./ping3 -M time NetworkDevice  on macOS
 * by comparing with the output of  ping -c3 -Mt -s0 NetworkDevice
 */

/*
 * Notable bugs, faults, weaknesses, short-comings or surprises; -
 * 1. Not all missing replies are reported except in the summary at the end
 * 2. Checksum is incorrect (see -v2 output on macOS) on IPv4 header option replies
 * 3. Reports total length of reply not ICMP payload length
 * 4. So many <abc.h> header files included - are they all still necessary?
 * 5. IPv4 Header option tsprespec with a single Device specified triggers overrun count
 * 6. IPv4 Header option tsprespec with no devices pre-specified has different first ping to subsequent pings
 * 7. No stand-alone Windows version, but CYGWIN version is semi-functional.
 * 8. ping3 doing a ping of the broadcast address on a multihomed macbook air
 *    sees multiple replies from the IP address associated with macbook air.
 */

/*
 * Not yet implemented; -
 * 0. Socket level time-stamping if available on the OS
 * 1. Multiple IPv4 Header options in the same ICMP request
 * 2. Separation of sending and receiving functions
 * 3. Reverse DNS lookup on the IP address of target
 * 4. ? Multicast ping option
 * 5. ? UDP port 7 (echo) ping?
 * 6. ? TCP port 7 (echo) ping?
 * 7. ? ARP ping for local subnet ping?
 * 8. Report what doesn't match if ICMP Echo request payload != reply payload
 */

/*
 * Verbosity levels
 * -v -9 .. No output to stdout. DNS name lookup errors reported to stderr. Success / failure indicated by return value.
 * -v -8 .. Report requests sent and replies received separated by a comma. E.g. 3, 3
 * -v -7 .. Report summary line "X requests sent, Y replies received, ZZZ.Z% loss
 * -v -6 .. Report summary line "X requests sent, Y replies received, ZZZ.Z% loss in S.SS [S]"
 * -v -5 .. Report previous ( -v -6 ) line plus RTT stats, smallest, mean, largest and standard dev if enough samples
 * -v -4 .. Same as -v -5
 * -v -3 .. Same as -v -4
 * -v -2 .. Same as -v -3
 * -v -1 .. Same as -v -2
 * -v 0  or no -v option .. Same as -v -1
 * -v 1 .. Report Names of local and remote network devices in addition to data and summary provided by -v 0
 * -v 2 .. Report any remote network device name alias' in addition to data and summary provided by -v 1
 * -v 3 .. Timestamp the replies in addition to data and summary provided by -v 2
 * -v 4 .. List up to the last 100 ping attempt timestamps in addition to timestamped data and summary provided by -v 3
 * -v 5 .. Report version in addition to information provided by -v 4
 * -v 6 .. Report Don't Fragment & Broadcast Permission status in addition to information provided by -v 5
 * -v 7 .. Report request & reply timestamps in greater detail in addition to information provided by -v 6
 * -v 8 .. Display the headers in received reply datagrams in addition to information provided by -v 7
 * -v 9 .. Reports copious amounts of information. Compile with -DDEBUG to enable Debug mode and the most info output.
 */


#include <stdlib.h> 		/* exit() atexit() arc4random() srandom() */
#include <stdio.h>			/* printf() fprintf() fflush() perror() */
#include <sys/types.h>		/* legacy: sendto() setsockopt() inet_addr() inet_ntoa() */
#include <sys/socket.h>		/* sendto() recvfrom() setsockopt() legacy: inet_addr() inet_ntoa() */
#include <signal.h>			/* signal() SIGUSR1 SIGALRM psignal() */
#include <libgen.h>			/* basename() */
#include <netdb.h>			/* gethostbyname() */
#include <unistd.h>			/* gethostname() getopt() */
#include <errno.h>			/* errno */
#include <string.h>			/* strncmp() strdup() */
#include <strings.h>		/* bzero() bcmp() */
#include <float.h>			/* DBL_MAX */
#include <math.h>			/* round() fmax() fmin() sqrt() */
#include "genFun.h"			/* clearByteArray() convertOptionStringToInteger() TRUE FALSE */
#include "dbgFun.h"			/* printNamedByteArray() */
#include "ipFun.h"			/* calcCheckSum() display_ip() */
#include "ipOptionsFun.h"	/* displayIpOptions() */
#include "icmpFun.h"		/* ICMP_TYPE_* displayICMP() */
#include "timeFun.h"		/* convertMilliSecondsSinceMidnightToHMS_String() calcTimeSpecClockDifferenceInSeconds() */


#define VERSION_STRING "0.9.9"			/* ping3 reported version */
#define MIN_PING_ATTEMPTS  0			/* smallest number of ICMP requests sent (limits X for -cX option) */
#define DEFAULT_PING_COUNT  3			/* default number of pings to send */
#define MAX_PING_ATTEMPTS  100			/* largest number of ICMP requests sent (limits X for -cX option) */
#define MIN_INTERVAL_BW_PINGS  0.1		/* smallest interval between pings in seconds (limits X.X for -iX.X option) */
#define DEFAULT_INTERVAL_BW_PINGS  1.0	/* default interval between pings in seconds */
#define MAX_INTERVAL_BW_PINGS  60.0		/* largest interval between pings in seconds (limits X.X for -iX.X option) */
#define MIN_WAIT_PERIOD  1				/* smallest wait period in seconds (limits X for -wX option) */
#define DEFAULT_WAIT_PERIOD  2			/* default wait after last ping time out period in seconds */
#define MAX_WAIT_PERIOD  20				/* largest wait after last ping (limits X for -wX option) */

#define DEFAULT_ICMP_EXTRA_DATA  56		/* default ICMP Payload size for ICMP Echo */
#define	MAX_IP4_PACKET_LEN	65535		/* max IP version 4 datagram size - i.e. length is specified by 16 bit word */
#define	STD_IP4_HDR_LEN  20				/* The header part of a IP datagram without options is 20 bytes */
#define MAX_IP4_HDR_OPTION_LEN	40		/* max IP version 4 extra header option length */
#define ICMP_HDR_LEN  8					/* The header part of an ICMP message is 8 bytes */
#define CONTRIVED_NUMBER  0				/* start number for ICMP message identifier */
#define USEC_CONV_ULONG_CONST 1000000L

#define ICMP_PAYLOAD_MAX_PATTERN_SIZE  28
#define ICMP_PAYLOAD_NO_PATTERN  0
#define ICMP_PAYLOAD_HEX_PATTERN  1
#define ICMP_PAYLOAD_TIME_PATTERN  2
#define ICMP_PAYLOAD_RANDOM_BYTE_PATTERN  3

#ifdef __linux__
#include "ipOptTS.h"
typedef  struct ipOptTimestamp  IP_TIMESTAMP;
#else
typedef  struct ip_timestamp  IP_TIMESTAMP;
#endif

/* Apple devices can use SOCK_DGRAM sockets to send ICMP without needing sudo privelege */
/* Other systems seem to require a raw socket for this purpose and also require sudo privelege */
#ifdef __APPLE__
#define SOCK_TYPE_TO_USE SOCK_DGRAM
#else
#define SOCK_TYPE_TO_USE SOCK_RAW
#endif

extern int  errno;

/* Command line Optional Switches: */
/*  count, Debug, help, interval, length, Mask/Timestamp, quiet,  */
/*  Record route, Time tp Live, Time stamp, verbosity, wait */
const char  optionStr[] = "abc:Dhi:l:M:p:Rs:t:T:v:w:";

/* Command Line options that may or may not be set by the user */
int	 quietFlag;					/* when TRUE minimise the output info */
int	 helpFlag;					/* when TRUE help message is printed and program exits */
int	 debugFlag;					/* when TRUE output lots of info */
int	 haveLocalHostNameFlag;			/* Local host name is known if true */
int  a_Flag;						/* Audible flag was found in the command line options */
int  b_Flag;						/* allow Broadcast flag was found in the command line options */
int  c_Flag;						/* Ping count was found in the command line options */
char *  c_Strng = ( char * ) NULL;	/*  pointer to -c value as there should be a value */
int	 pingSendAttemptsLeft;			/* attempt pingSendAttemptsLeft pings */
int  continousPingMode;				/* keep pinging until interrupted by user mode */
int  D_Flag;						/* Don't Fragment was found in the command line options */
int  i_Flag;						/* interval time option was found in the command line options */
char * i_Strng = ( char * ) NULL;	/* pointer to -i value */
double  intervalBwPingsInSeconds;	/* interval Option length */
u_long  intervalBwPingsInMicroSeconds;
int  l_Flag;						/* TRUE if IPv4 Header option length was found in the command line options */
char * l_Strng = ( char * ) NULL;	/* pointer to -l value */
int  ip4_HdrOptionSize;				/* IP Header Option length */
int  M_Flag;						/* TRUE if ICMP Mode (ICMP Type) identifier was found in the command line options */
char *  M_Strng = ( char * ) NULL;	/* pointer to -M type value */
int  icmpType;						/* Specifies the time stamp replies as little endian i.e. Windows replies */
int  p_Flag;						/* TRUE if there is a pattern to put into the ping ICMP payload section */
char *  p_Strng = ( char * ) NULL;	/* ICMP payload data pattern spec */
int  icmpPayloadPatternType;
int  icmpPayloadPatternSize;
u_char  icmpPayloadPattern[ ICMP_PAYLOAD_MAX_PATTERN_SIZE ];	/* Storage for the user specified pattern if there is one */
int  R_Flag;						/* TRUE if IPv4 Header option Record Route was found in the command line options */
int  s_Flag;						/* TRUE if ICMP payload size was found in the command line options */
char *  s_Strng = ( char * ) NULL;	/* pointer to -s value */
int  icmpExtraDataSizeValue;		/* Specifies the ICMP message payload size (i.e. NOT total length) */
int  icmpFirstExtraDataSizeValue;	/* Specifies the starting ICMP message payload size */
int  icmpSecondExtraDataSizeValue;	/* Specifies the turning point ICMP message payload size */
int  icmpStepExtraDataSizeValue;	/* Specifies the step size used to move from starting size to turning point size */
int  t_Flag;						/* TRUE if IP4 Header TTL found in the command line options */
char *  t_Strng = ( char * ) NULL;	/* pointer to -t value */
int  ip4_HeaderTTL_Value;			/* Specifies the datagram time to live parameter */
int  T_Flag;						/* TRUE if IP4 Header options Time stamp identifier was found in the command line options */
char *  T_Strng = ( char * ) NULL;	/* pointer to -T value */
int  ip4_OptionTS_Value;			/* Specifies the type of time stamp request and is -1 for none */
int  v_Flag;						/* TRUE if Wait count was found in the command line options */
char *  v_Strng = ( char * ) NULL;	/* pointer to -w value */
int	 verbosityLevel;				/* when TRUE more info is output */
int  w_Flag;						/* TRUE if Wait count was found in the command line options */
char *  w_Strng = ( char * ) NULL;	/* pointer to -w value */
int	 waitTimeInSec;					/* set to wait time value */

int  pingsSent;			/* Keep track of ICMP requests that have been sent for stats */
int  pingsReceived;		/* Keep track of ICMP replies that have been received for stats */

int  statsSamplesReceived;		/* track number of samples in the stats */
double  statsMinimumRTT;		/* track the minimum RTT */
double  statsRunningMeanRTT;	/* track the mean RTT time */
double  statsMaximumRTT;		/* track the maximum RTT */
double  statsRunningVarianceRTT;	/* track variance of RTT */

char *  exeName = ( char * ) NULL;	/* name of this executable */
char *  exePath = ( char * ) NULL;	/* path of this executable */

int	 sckt;							/* Global socket to use */
char *  hostName = ( char * ) NULL;
int  txICMP_BfrSize;				/* Size of the transmit buffer. It specifies the largest ICMP Message that can be sent */
u_char *  icmpMessageToTx = ( u_char * ) NULL;
int  rxIPv4_BfrSize;
u_char *  ip4_DatagramRxd = ( u_char * ) NULL;		/* pointer to buffer to contain received IP4 ICMP reply message */
u_short  icmpHdrID;			/* The identifier to put in the ICMP header */
u_short  process_id;		/* process ID */

struct timespec	 timeToBeStoredInSentICMP;	/* time stamp put in the sent Echo Request */
struct timespec	 timeStoredInReceivedICMP;	/* time stamp retrieved from the Echo Reply */
struct timespec  recvTime;			/* receive time stamp */
struct timespec  timeOfFirstPing;	/* 1st transmit time stamp */
long  tsorig;		/* originate timestamp in millisec since midnight UTC s */
long  tsrecv; 		/* receive timestamp in millisec since midnight UTC */

struct sockaddr	 remoteDeviceToPingInfo;	/* target network device to ping */
struct sockaddr	 preSpecDevice[ 4 ];		/* get timestamp from network up to 4 interfaces */
struct sockaddr_in *  to;					/* pointer to remote network device info */

char *  remoteDeviceNameBuffer = ( char * ) NULL;
char *  localDeviceNameBuffer = ( char * ) NULL;
char *  prespecDeviceNameBuffer[ 4 ];
n_short  sequenceID_Array[ MAX_PING_ATTEMPTS ];		/* Array of sequence ID numbers - not really needed until continuous ping mode is used */
struct timespec	 timeSentArray[ MAX_PING_ATTEMPTS ];	/* Array of send time stamps indexed by sequence id */
struct timespec	 timeReceivedArray[ MAX_PING_ATTEMPTS ];	/* Array of receive time stamps indexed by sequence id */


void  printVersion( void )  {
	printf( "ping3 version %s\n", VERSION_STRING );
}


/*
 * provide standardized access into wrap around array sequenceID_Array
 */
n_short *  returnPtrToSeqID_ArrayForSeqID( n_short *  basePtr, n_short  sequenceID )  {
	return( basePtr + ( sequenceID % MAX_PING_ATTEMPTS ));
}


/*
 * provide standardized access into wrap around arrays timeSentArray and timeReceivedArray
 */
struct timespec *  returnPtrToTimeStampForSeqID( struct timespec *  basePtr, n_short  sequenceID )  {
	return( basePtr + ( sequenceID % MAX_PING_ATTEMPTS ));
}


/*
 * Code adapted from; -
 * https://dsp.stackexchange.com/questions/811/determining-the-mean-and-standard-deviation-in-real-time
 */
void  trackStats( double  latestRTT )  {
	double  previousMeanRTT;

	/* Bump up sample count */
	statsSamplesReceived += 1;
	/* Is latest value bigger or smaller than all previous values ? */
	statsMaximumRTT = fmax( latestRTT, statsMaximumRTT );
	statsMinimumRTT = fmin( latestRTT, statsMinimumRTT );
	previousMeanRTT = statsRunningMeanRTT;
	statsRunningMeanRTT += ( latestRTT - statsRunningMeanRTT ) / ( double ) statsSamplesReceived;
	statsRunningVarianceRTT += ( latestRTT - statsRunningMeanRTT ) * ( latestRTT - previousMeanRTT );
}


/*
 * Set the timer that determines the time between interrupts
 */
void  setSendTimer( u_long  seconds, u_long  uSec )  {
	struct itimerval  val;
	struct itimerval  old;

	val.it_value.tv_sec = ( long ) seconds;
	val.it_value.tv_usec = ( long ) uSec;
	val.it_interval.tv_sec = ( long ) seconds;
	val.it_interval.tv_usec = ( long ) uSec;
	old.it_value.tv_sec = ( long ) 0;
	old.it_value.tv_usec = ( long ) 0;
	old.it_interval.tv_sec = ( long ) 0;
	old.it_interval.tv_usec = ( long ) 0;
	setitimer( ITIMER_REAL, &val, &old );
#ifdef DEBUG	
	if( debugFlag )  printf( "Debug: Timer set to: %lu [S]: %lu [uS]\n", seconds, uSec );
#endif
}


int  computeCheckSumAndSendIPv4_ICMP_Datagram( int  socketID, u_char *  icmpMsg, int  icmpMsgSize, u_short  seqID )  {
	ssize_t	 numberOfBytesSent = 0;
	struct icmp	*  icmpHdrPtr;
	struct timespec *  tsPtr;
	n_short *  nShrtPtr;

	icmpHdrPtr = (struct icmp *) icmpMsg;
	 /* compute ICMP checksum */
	icmpHdrPtr->icmp_cksum = calcCheckSum((u_short *) icmpHdrPtr, icmpMsgSize );
#ifdef DEBUG	
	if( debugFlag )  {
		printf( "Debug: ICMP Message size to transmit is %d\n", icmpMsgSize );
		printNamedByteArray( icmpMsg, icmpMsgSize, 20, "ICMP request messsage (send array contents)" );
		printf( "\n" );
	}
#endif
	/* Ensure time of zero is recorded for receive time for reply - this is really only needed for continuous ping mode */
	tsPtr = returnPtrToTimeStampForSeqID( timeReceivedArray, seqID );	/* index into seqID receive time with wrap around for continuous mode */
	tsPtr->tv_sec = 0;	/* reset seconds part of receive time for seqID */
	tsPtr->tv_nsec = 0;	/* reset nS part of receive time for seqID */
	nShrtPtr = returnPtrToSeqID_ArrayForSeqID( sequenceID_Array, seqID );
	*nShrtPtr = seqID;	/* set corresponding sequence array value - this is really only needed for continuous ping mode */
	/* Set time recorded for request message being sent */
	tsPtr = returnPtrToTimeStampForSeqID( timeSentArray, seqID );	/* index into time set array with wrap around for continuous mode */
	clock_gettime( CLOCK_REALTIME, tsPtr );		/* remember time just before the datagram is sent */
	/* Send ICMP request message */
	numberOfBytesSent = sendto( socketID, (char *) icmpMsg, icmpMsgSize, 0, &remoteDeviceToPingInfo, sizeof( struct sockaddr ));
	tsorig = calcMillisecondsSinceMidnightFromTimeSpec( tsPtr );
	if(( numberOfBytesSent < 0 ) && ( ! quietFlag ))  perror( "Warning: sendto() error" );
	else if(( numberOfBytesSent != icmpMsgSize ) && ( ! quietFlag )) {
		printf("Warning: sendto() wrote %ld chars to %s, but it should have been %d chars\n",
			numberOfBytesSent, hostName, icmpMsgSize );
	}
	return( numberOfBytesSent );
}


/*
 * Send the icmp mask request (RFC 950 Appendix I.).
 */
int  sendICMP_MaskRequest( int  socketID, u_char *  icmpMsgBfr, int  icmpMsgSize, u_short  seqID )  {
	ssize_t	 numberOfBytesSent = 0;
	struct icmp	*  icmpHdrPtr;
	u_int32_t *  icmpMaskPtr;

#ifdef DEBUG	
	if( debugFlag )  printf( "Debug: About to send %d byte ICMP Mask Request Message with Sequence ID 0x%04x\n", icmpMsgSize, seqID );
#endif
	/* Set up the ICMP info if there is enough room in the ip_Data buffer */
	if( icmpMsgSize > ICMP_HDR_LEN )  {
		clearByteArray( icmpMsgBfr, icmpMsgSize );	/* ensure data is zero'd each time as this storage is reused */
		icmpHdrPtr = (struct icmp *) icmpMsgBfr;
		fill_ICMP_HdrPreChkSum( icmpHdrPtr, ICMP_MASKREQ, 0, seqID, process_id );
		icmpMaskPtr = ( u_int32_t * ) ( icmpMsgBfr + 8 );		/* point to icmp Data area */
		if( icmpMsgSize >= 12 )  {	/* put in mask of zero if there is space */
			*icmpMaskPtr = htonl( 0 );
	    }
		numberOfBytesSent = computeCheckSumAndSendIPv4_ICMP_Datagram( socketID, icmpMsgBfr, icmpMsgSize, seqID );
	}
	return( numberOfBytesSent );
}


/*
 * Send the icmp timestamp request (RFC 792 pages 16-17).
 */
int  sendICMP_TimestampRequest( int  socketID, u_char *  icmpMsgBfr, int  icmpMsgSize, u_short  seqID )  {
	ssize_t	 numberOfBytesSent = 0;
	struct icmp	*  icmpHdrPtr;
	u_int32_t *  icmpOrigTimestampPtr;

#ifdef DEBUG	
	if( debugFlag )  printf( "Debug: About to send %d byte ICMP Time Stamp Request Message with Sequence ID 0x%04x\n", icmpMsgSize, seqID );
#endif
	/* Set up the ICMP info if there is enough room in the ip_Data buffer */
	if( icmpMsgSize > ICMP_HDR_LEN )  {
		clearByteArray( icmpMsgBfr, icmpMsgSize );	/* ensure data is zero'd each time as this storage is reused */
		icmpHdrPtr = (struct icmp *) icmpMsgBfr;
		fill_ICMP_HdrPreChkSum( icmpHdrPtr, ICMP_TSTAMP, 0, seqID, process_id );
		icmpOrigTimestampPtr = ( u_int32_t * ) ( icmpMsgBfr + 8 );		/* point to icmp Data area */
		if( icmpMsgSize >= 20 )  {	/* put mS time in the icmp data section if there is space */
			clock_gettime( CLOCK_REALTIME, &timeToBeStoredInSentICMP );		/* get time to put in the message about to be sent */
			*icmpOrigTimestampPtr = htonl( calcMillisecondsSinceMidnightFromTimeSpec( &timeToBeStoredInSentICMP ));
	    }
		numberOfBytesSent = computeCheckSumAndSendIPv4_ICMP_Datagram( socketID, icmpMsgBfr, icmpMsgSize, seqID );
	}
	return( numberOfBytesSent );
}


/*
 * Send the icmp echo request (RFC 792 page 14).
 */
int  sendICMP_EchoRequest( int  socketID, u_char *  icmpMsgBfr, int  icmpMsgSize, u_short  seqID )  {
	ssize_t	 numberOfBytesSent = 0;
	struct icmp	*  icmpHdrPtr;
	u_char *  icmpDataPtr;
	int *  intPtr;

#ifdef DEBUG	
	if( debugFlag )  printf( "Debug: About to send %d byte ICMP Echo Request Message with Sequence ID 0x%04x\n", icmpMsgSize, seqID );
#endif
	/* Set up the ICMP info if there is enough room in the buffer */
	if( icmpMsgSize < ICMP_HDR_LEN )
		fprintf( stderr, "Error: Not enough space allocated (%d bytes) to store the ICMP Echo Header\n", icmpMsgSize );
	else  {
		clearByteArray( icmpMsgBfr, icmpMsgSize );	/* ensure data is zero'd each time as this storage is reused */
		icmpHdrPtr = (struct icmp *) icmpMsgBfr;
		fill_ICMP_HdrPreChkSum( icmpHdrPtr, ICMP_ECHO, 0, seqID, process_id );
		icmpDataPtr = icmpMsgBfr + 8;	/* point to icmp Data area */
		clock_gettime( CLOCK_REALTIME, &timeToBeStoredInSentICMP );		/* get time to put in the message about to be sent */
		if( p_Flag )  {
			if( icmpPayloadPatternType == ICMP_PAYLOAD_RANDOM_BYTE_PATTERN )  {
#ifdef __linux__
				srandom( calcMillisecondsSinceMidnightFromTimeSpec( &timeToBeStoredInSentICMP ));	/* seed number generator */
				fillByteArrayWithPseudoRandomData( icmpDataPtr, icmpMsgSize - 8 );
#else
				arc4random_buf( icmpDataPtr, icmpMsgSize - 8 );		/* fill payload area with random bytes */
#endif
			}
			else if( icmpPayloadPatternType == ICMP_PAYLOAD_TIME_PATTERN )  {
				intPtr = ( int * ) icmpPayloadPattern;
				*intPtr = htonl( calcMillisecondsSinceMidnightFromTimeSpec( &timeToBeStoredInSentICMP ));
				fillFirstByteArrayByReplicatingSecondByteArray( icmpDataPtr, icmpMsgSize - 8, icmpPayloadPattern, 4 );
			}
			else  {
				fillFirstByteArrayByReplicatingSecondByteArray( icmpDataPtr, icmpMsgSize - 8, icmpPayloadPattern, icmpPayloadPatternSize );
			}
		}
		else  {		/* -p hasn't been specified so setup default payload pattern */
			fillByteArrayWithIncByOne( 0, icmpDataPtr, icmpMsgSize - 8 );	/* Incrementing byte pattern is the default for macOS & linux ping */
			if( icmpMsgSize >= ( ICMP_HDR_LEN + sizeof( struct timespec )))  {	/* no pattern specified so put nS time in the icmp data section if there is space */
				memcpy( (void *) icmpDataPtr, ( void * ) &timeToBeStoredInSentICMP, sizeof( struct timespec )); /* Don't worry about endianess */	
	    	}
		}
		numberOfBytesSent = computeCheckSumAndSendIPv4_ICMP_Datagram( socketID, icmpMsgBfr, icmpMsgSize, seqID );
	}
	return( numberOfBytesSent );
}


int sendICMP_Request( int  socketID, u_char *  icmpMsgBfr, int  icmpMsgSize, u_short  seqID )  {
	int  successFlag;
	int  bytesSent;

	successFlag = FALSE;
	if( M_Flag && ( icmpType >= ICMP_TYPE_TIME ))  {
		bytesSent = sendICMP_TimestampRequest( socketID, icmpMsgBfr, 20, seqID );	/* ICMP Time stamp is 8 byte Header + 12 byte Data */
		successFlag = ( bytesSent == 20 );
		if( ! successFlag )
			fprintf( stderr, "Warning: Unable to send 20 byte ICMP Timestamp Request in the network datagram?\n" );
	}
	else if( M_Flag && ( icmpType == ICMP_TYPE_MASK ))  {
		bytesSent = sendICMP_MaskRequest( socketID, icmpMsgBfr, 12, seqID );	/* ICMP Mask is 8 byte Header + 4 byte Data */
		successFlag = ( bytesSent == 12 );
		if( ! successFlag )
			fprintf( stderr, "Warning: Unable to send 12 byte ICMP Mask Request in the network datagram?\n" );
	}
	else  {
		bytesSent = sendICMP_EchoRequest( socketID, icmpMsgBfr, icmpMsgSize, seqID );
		successFlag = ( bytesSent == icmpMsgSize );
		if( ! successFlag )
			fprintf( stderr, "Warning: Unable to send %d byte ICMP Echo Request in the network datagram?\n", icmpMsgSize );
	}
	return( successFlag );	
}


/* Print the normal ping info */
/* E.g. XX bytes from AAA.BBB.CCC.DDD: seq YYYYY, ttl ZZZ, RTT 0.994 [mS] */
void  printStdPingInfo( struct ip *  ip, struct icmp *  icmpHdrPtr, int  ICMP_MsgSize )  {
	n_short  sequenceID;
	struct timespec *  tsOrigPtr;
	struct timespec *  tsRecvPtr;

	sequenceID = ntohs( icmpHdrPtr->icmp_seq);
	tsOrigPtr = returnPtrToTimeStampForSeqID( timeSentArray, sequenceID );
	tsRecvPtr = returnPtrToTimeStampForSeqID( timeReceivedArray, sequenceID );
	if( a_Flag )  printf( "\007" );	/* Output the bell character if audible reply is requested */
	if( verbosityLevel > 2 )  {		/* Output a receive time stamp at the start of the normal line */
		printTimeSpecTimeInHMS( tsRecvPtr, verbosityLevel );
		printf( " " );
	}
	printPingCommonInfo( ip, ICMP_MsgSize );
	printf( " seq %d, ", sequenceID );
	printIPv4_TimeToLiveInfo( ip );
	printf( ", " );
	printClockRealTimeFlightTime( tsOrigPtr, tsRecvPtr );
}


void  printLineOfPingInfo( struct ip *  ip, struct icmp *  icmpHdrPtr, int  ICMP_MsgSize )  {
	n_short  sequenceID;
	struct timespec *  tsOrigPtr;
	struct timespec *  tsRecvPtr;

	sequenceID = ntohs( icmpHdrPtr->icmp_seq);
	tsOrigPtr = returnPtrToTimeStampForSeqID( timeSentArray, sequenceID );
	tsRecvPtr = returnPtrToTimeStampForSeqID( timeReceivedArray, sequenceID );
	printStdPingInfo( ip, icmpHdrPtr, ICMP_MsgSize );
	if( icmpHdrPtr->icmp_type == ICMP_TSTAMPREPLY )  {
		printf( " " );
		displayTimeStampReplyTimestamps( icmpHdrPtr, ( icmpType == ICMP_TYPE_TIMEW ));
	}
	else if( icmpHdrPtr->icmp_type == ICMP_MASKREPLY )  {
		printf( " " );
		displayMaskReplyMask( icmpHdrPtr );
	}
	printf( "\n" );
	if( verbosityLevel >= 7 )  printClockRealTimeTxRxTimes( tsOrigPtr, tsRecvPtr, verbosityLevel );
}


void  printTimeDifferenceFromHeaderOption( IP_TIMESTAMP *  tsPntr, int  hdrLen, double  halfRndTripTime, int verbosityLvl )  {
	long  tstarget;	/* remote device send timestamp in # millisec since midnight UTC  */
	long  tsrecv; 	/* receive timestamp in # millisec since midnight UTC */
	struct ipt_ta *	 ipt_taPtr;
	struct ipt_ta *	 ipt_taPtr2;

	tsrecv = calcMillisecondsSinceMidnightFromTimeSpec( &recvTime );
	if( tsPntr->ipt_ptr < 13 )  {	/* Is there at least 1 timestamp in the option storage */
		if( verbosityLvl > 0 )  printf( "Warning: No timestamps found in IPv4 header options\n");
	}
	else  {
		/* tsorig and tsrecv are both signed longs. The icmp time members in the structure are unsigned,
		 * but the max value for the # millisec since midnight is (24*60*60*1000 - 1) or 86,399,999,
		 * which easily fits into a signed long. We want them signed to compute a signed difference. */
		tstarget = ntohl( tsPntr->ipt_timestamp.ipt_ta[ 0 ].ipt_time );
		if( tstarget < 0 )  printf( "Warning: Remote Device send time in Non standard time format %ld ms (0x%lx)", tstarget, tstarget );
		ipt_taPtr = &tsPntr->ipt_timestamp.ipt_ta[ 0 ];
		ipt_taPtr2 = ipt_taPtr + 1;
#ifdef DEBUG
		printNamedByteArray(( u_char *) tsPntr, hdrLen - STD_IP4_HDR_LEN, 20, "processReceivedDatagram(): IP4 option received" );
		displayIpOptions(( u_char *) tsPntr, hdrLen - STD_IP4_HDR_LEN, verbosityLvl );
#endif
		 /* Now get time according to target when reply message left */
		 /* Assume reply travel time was same speed as request travel time */
 		 /* Thus time here when request was actually processed by target */
		 /* is  tsorig + 0.5 * tsRTT, now subtract that from target time */
		if(( ip4_OptionTS_Value == IPOPT_TS_PRESPEC ) && ( verbosityLvl != 0 ) )  { /* time difference output only if known and verbose required */
			if(( tsPntr->ipt_ptr > 20 ) && ( ipt_taPtr->ipt_addr.s_addr != ipt_taPtr2->ipt_addr.s_addr ))  {	/* not same address twice ? */
				tsrecv = ntohl( ipt_taPtr2->ipt_time );		/* Local interface's timestamp or otherwise it remains recvTime based */
				if( tsrecv < 0 )  {
					printf( "Warning: Receive time in Non standard time format %ld ms (0x%lx)", tsrecv, tsrecv );
					tsrecv = calcMillisecondsSinceMidnightFromTimeSpec( &recvTime );	/* go back to using recvTime */
				}
			}
			printf( "IP_HDR_OPT_TS: ");
			printSentVsReceiveDeviceClockDifferenceEstimate( tstarget, tsrecv, ( long ) halfRndTripTime, ( verbosityLvl > 1 ));
		}
		if( verbosityLvl > 1 )  printf( "IP_HDR_OPT_TS: Orig = %ld, Target %ld, Recv = %ld\n", tsorig, tstarget, tsrecv );
	}
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
	double  roundTripTime, halfRoundTripTime;
	n_short  sequenceID;
	struct timespec *  tsOrigPtr;
	struct timespec *  tsRecvPtr;

 /* Check the IP datagram */
	ip = (struct ip *) buf;
	tsPntr = (IP_TIMESTAMP * ) ( buf + sizeof( struct ip ));
 /* Received datagram is assumed to be an IP4 datagram, but check to make sure */
    if( ip->ip_v != 4 )  {
		if( ! quietFlag )  {
			fprintf( stderr, "Warning: Datagram from %s isn't IP version 4, instead it is version %x\n", 
				inet_ntoa( *(struct in_addr *)&from->sin_addr.s_addr), ip->ip_v );
			printNamedByteArray(( u_char *) ip, datagramSize, 20, "processReceivedDatagram(): non IP4 datagram received" );
		}
		return( -1 );
	}
	hdrLen = ip->ip_hl << 2;	/* determine the length of the IP datagram header */
	icmpHdrPtr = (struct icmp *)( buf + hdrLen );	/* point to the start of the datagram payload (i.e. ICMP message) */
#ifdef DEBUG	
	if( debugFlag )  {
		printf( "Debug: Datagram Length is %d - Header Length is %d\n", datagramSize, hdrLen );
		printNamedByteArray(( u_char *) ip, datagramSize, 20, "processReceivedDatagram(): Complete IP4 datagram received" );
	}
#endif
	ip->ip_len = htons( datagramSize );		/* restore total length in header in case it has been altered */
 /* Received datagram is assumed to be an error free IPv4 datagram, but calculate the header check sum to make sure */
	if(( chkSumResult = calcCheckSum( (u_short * ) ip, hdrLen )) != 0 )  {
		if( hdrLen == STD_IP4_HDR_LEN && ( verbosityLevel > 0 ))  {	/* Don't report if header options are in use - FIX checksum for options use */
			fprintf( stderr, "Warning: Calculated checksum of IPv4 datagram from %s is 0x%04x, but should be 0x0000\n", 
				inet_ntoa( *(struct in_addr *)&from->sin_addr.s_addr), chkSumResult );
		 /* Try zero the time option if there is one */
#ifdef DEBUG	
			if( debugFlag )
				printNamedByteArray(( u_char *) ip, datagramSize, 20, "processReceivedDatagram(): IP4 header checksum failed datagram" );
#endif
		}
	}
 /* Received datagram is assumed to be an ICMP datagram, but check to make sure */
    if( ip->ip_p != 0x01 )  {
		if( ! quietFlag )  {
			fprintf( stderr, "Warning: IPv4 datagram from %s doesn't carry an ICMP message (protocol is 0x%x )\n", 
				inet_ntoa( *(struct in_addr *)&from->sin_addr.s_addr), ip->ip_p );
			if( verbosityLevel > 2 )
				printNamedByteArray(( u_char *) ip, datagramSize, 20, "processReceivedDatagram(): non ICMP IP4 received" );
		}
		return( -1 );
	}
 /* Received datagram is assumed to be big enough to be a ICMP datagram, but check to make sure */
	if ( datagramSize < ( hdrLen + ICMP_MINLEN )) {
		if( ! quietFlag )  {
			fprintf( stderr, "Warning: IPv4 datagram from %s is too short (%d bytes) to contain ICMP message\n", 
				inet_ntoa( *(struct in_addr *)&from->sin_addr.s_addr), datagramSize );
			if( verbosityLevel > 2)
				printNamedByteArray(( u_char *) ip, datagramSize, 20, "processReceivedDatagram(): too short ICMP IP4 received" );
		}
		return( -1 );
	}
	if( verbosityLevel >= 8 )  {
		printf( "\nInfo: Received IP4 datagram IPv4 & ICMP headers\n" );
		display_ip( ip, datagramSize );
		if( verbosityLevel > 8)
			printNamedByteArray(( u_char *) ip, datagramSize, 20, "Info: Received IP4 datagram" );
		else  printf( "\n");
	}
 /* Now the ICMP message + header options (if there are any) */
	if(( chkSumResult = calcCheckSum( (u_short * ) icmpHdrPtr, datagramSize - hdrLen )) != 0 )  {
		if( ! quietFlag )
			fprintf( stderr, "Warning: Calculated checksum of ICMP message from %s is 0x%04x, but should be 0x0000\n", 
				inet_ntoa( *(struct in_addr *)&from->sin_addr.s_addr), chkSumResult );
	}
     /* With a raw ICMP socket we get all ICMP datagrams that come into the kernel. Only accept Echo, Timestamp or Mask replies */
	if( ! (( icmpHdrPtr->icmp_type == ICMP_ECHOREPLY ) ||
	       ( icmpHdrPtr->icmp_type == ICMP_TSTAMPREPLY ) ||
	       ( icmpHdrPtr->icmp_type == ICMP_MASKREPLY )))  {
		if( ! quietFlag )  {
			printf( "Warning: %s returned unexpected ", inet_ntoa(*(struct in_addr *) &from->sin_addr.s_addr ));
			displayICMP( icmpHdrPtr );
		}
		if( verbosityLevel > 2 )
			printNamedByteArray(( u_char *) ip, datagramSize, 20, "processReceivedDatagram(): non Echo/Timestamp/Mask Reply ICMP IP4 received" );
		return( -1 );	/* some other type of ICMP message */
	}
	if( ntohs( icmpHdrPtr->icmp_id ) != process_id )  {
		printf("Warning: received icmp reply with id 0x%04x but expected 0x%04x\n",
			ntohs( icmpHdrPtr->icmp_id ), process_id );
		if( debugFlag )  display_ip( ip, datagramSize );
		return( -1 );	/* ICMP message sent by some other process - not this ping3 process */
	}
	if( ntohs( icmpHdrPtr->icmp_seq ) != icmpHdrID )  {
		printf( "Warning: received icmp reply with sequence number %d but expected %d\n",
			ntohs( icmpHdrPtr->icmp_seq ), icmpHdrID );
		if( debugFlag )  display_ip( ip, datagramSize );
	}
	/* If sequence ID's match then test that what was sent in the ICMP Echo request payload was returned in the ICMP Echo reply payload */
	else if(( icmpHdrPtr->icmp_type == ICMP_ECHOREPLY ) &&
		( bcmp( &icmpHdrPtr->icmp_seq, icmpMessageToTx + 6, ( ICMP_HDR_LEN + icmpExtraDataSizeValue - 6 )) != 0 ))  {
			printf( "Warning: ICMP Echo request payload data does not match Echo reply payload data?\n");
			if( debugFlag )  printf( " Buffer Size being compared is %d\n", ( ICMP_HDR_LEN + icmpExtraDataSizeValue - 6 ));
	}
	/* The ICMP datagram seems all ok - so safe to use sequence ID to index into arrays of timestamps */
	sequenceID = ntohs( icmpHdrPtr->icmp_seq);
	tsOrigPtr = returnPtrToTimeStampForSeqID( timeSentArray, sequenceID );
	tsRecvPtr = returnPtrToTimeStampForSeqID( timeReceivedArray, sequenceID );
	tsRecvPtr->tv_nsec = recvTime.tv_nsec;
	tsRecvPtr->tv_sec = recvTime.tv_sec;
	roundTripTime = calcTimeSpecClockDifferenceInSeconds( tsOrigPtr, tsRecvPtr );
	trackStats( roundTripTime );
	halfRoundTripTime = round( roundTripTime * ( double ) 500.0 );
	if( hdrLen == STD_IP4_HDR_LEN )  {
		if( ! quietFlag )  printLineOfPingInfo( ip, icmpHdrPtr, datagramSize );
	}
	else  {
		/* Print the normal ping info E.g. XX bytes from AAA.BBB.CCC.DDD: seq YYYYY, ttl ZZZ, RTT 0.994 [mS] */
		if( ! quietFlag )  printLineOfPingInfo( ip, icmpHdrPtr, datagramSize );
		if(( hdrLen - STD_IP4_HDR_LEN ) > 0 )  {	/* when true there is a option section in the IP4 header */
			if( debugFlag )  displayIpOptionList(( u_char *) tsPntr, hdrLen - STD_IP4_HDR_LEN );
		}
		if(( hdrLen - STD_IP4_HDR_LEN ) > 4 )  {	/* when true there is at least a valid option header to work on */
			if( verbosityLevel > 0 )  {	/* print extra line about Local transmit time */
				printf( " Local Tx time:" );
				displayAbsTimeInMultipleFormats( calcMillisecondsSinceMidnightFromTimeSpec( tsOrigPtr ), ( verbosityLevel > 0 ));
				printf( "\n" );
			}
			if( T_Flag )  {
				displayIpOptionTimeStamps(( u_char * ) tsPntr, hdrLen - STD_IP4_HDR_LEN,
					calcMillisecondsSinceMidnightFromTimeSpec( tsOrigPtr ), ( verbosityLevel > 0 ));
			}
			else if( R_Flag )  {
				displayIpOptionRecordRoute(( u_char * ) tsPntr, hdrLen - STD_IP4_HDR_LEN, ( verbosityLevel > 0 ) );
			}
			if( verbosityLevel > 0 )  {	/* print extra line about Local receive time */
				printf( " Local Rx time:" );
				displayAbsTimeInMultipleFormats( calcMillisecondsSinceMidnightFromTimeSpec( &recvTime ), ( verbosityLevel > 0 ));
				printf( "\n" );
			}
			if( T_Flag )  {
				displayOnlyGreaterThanZeroTimeStampOverflowCount(( u_char *) tsPntr );
				if(( ip4_OptionTS_Value == IPOPT_TS_TSANDADDR ) || ( ip4_OptionTS_Value == IPOPT_TS_PRESPEC ))	/* time difference output id device IP Addresses are captured */
					printTimeDifferenceFromHeaderOption( tsPntr, hdrLen , halfRoundTripTime, ( verbosityLevel > 0 ));
			}
		}
	}
	if( M_Flag && (( icmpType == ICMP_TYPE_TIME ) || ( icmpType == ICMP_TYPE_TIMEW )))  {
		printTimeDifferenceFromICMP_TimestampReply( icmpType, icmpHdrPtr, halfRoundTripTime, ( verbosityLevel > 0 ));
	} 
	return( 0 );	/* done */
}


int  getResponseAndProcessIt( int socketID )  {
	int  response = 0;
	int  numOfBytesRcvd;
	socklen_t  fromLen;
	struct sockaddr_in  from;

	fromLen = sizeof( from );
	if(( numOfBytesRcvd = recvfrom( socketID, (char *) ip4_DatagramRxd, rxIPv4_BfrSize, 0,
		( struct sockaddr *) &from, &fromLen )) < 0 ) {
		clock_gettime( CLOCK_REALTIME, &recvTime );	/* Remember time of error */
		if( errno == EINTR )
			printf( "Interrupt EINTR\n");
		else  perror( "Error: recvfrom error" );
	}
	else  {
		clock_gettime( CLOCK_REALTIME, &recvTime );		/* Remember time of received ICMP message */
		/* process received ICMP message */
		response = ( processReceivedDatagram((char *) ip4_DatagramRxd, numOfBytesRcvd, &from ) == 0 );
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
	if(( ip4_OptionTS_Value == IPOPT_TS_PRESPEC ) && ( ipt_taPtr->ipt_addr.s_addr == 0 ))  {	/* No prespecified addresses ? */
		tsPtr->ipt_timestamp.ipt_ta[0].ipt_addr = to->sin_addr;		/* insert remote network device IP address */
		tsPtr->ipt_timestamp.ipt_ta[0].ipt_time = 0;
		ipt_taPtr = &tsPtr->ipt_timestamp.ipt_ta[0];
		ipt_taPtr += 1;
		ipt_taPtr->ipt_addr = rxdIP4_Packet->ip_dst;		/* This requires Local interface is known from a received the reply */
		if( ipt_taPtr->ipt_addr.s_addr == 0 )  ipt_taPtr->ipt_addr = to->sin_addr;		/* if not known i.e. 0, then fudge with target */
		ipt_taPtr->ipt_time = 0;
		ip4_HdrOptionSize = 20;	/* Over-ride option size to 4 octets for option header + 16 for two address + time pairs */
	}
	else if( ip4_OptionTS_Value == IPOPT_TS_PRESPEC )  {		/* Only set up prespecified addresses if 3 */
		ip4_HdrOptionSize = 0;
		for( i = 0; ( i < 4 ); i++ )  {
			sckAddrInPtr = ( struct sockaddr_in * ) &preSpecDevice[ i ];
			ipt_taPtr->ipt_addr = sckAddrInPtr->sin_addr;	/* insert remote network device IP address */
			if( ipt_taPtr->ipt_addr.s_addr == 0 )  break;	/* Drop out of loop if IP address just used was 0.0.0.0 */
			ipt_taPtr->ipt_time = 0;
			ipt_taPtr += 1;
			ip4_HdrOptionSize += 8;	/* Address + time pair is a total of 8 bytes */
		}
		if( ip4_HdrOptionSize > 0 )  ip4_HdrOptionSize += 4;	/* Add 4 octets for option header to address + time pairs */
	}
	else if( ip4_OptionTS_Value == IPOPT_TS_TSANDADDR )  {
		if( ip4_HdrOptionSize < 12 )  ip4_HdrOptionSize = 12;		/* Makes no sense to be smaller than header + 1 address + time pair */
		else  ip4_HdrOptionSize = 4 + ( 8 * (( ip4_HdrOptionSize - 4 ) / 8 ));	/* trim to header + an exact number of pairs */
	}
	/* set IPv4 Header Options header values */
	tsPtr->ipt_code = IPOPT_TS;
	tsPtr->ipt_ptr = 5;
	tsPtr->ipt_oflw = 0;
	tsPtr->ipt_len = ip4_HdrOptionSize;
	tsPtr->ipt_flg = ip4_OptionTS_Value;	/* tsonly is 0, tsandaddr 1, tsprespec 3 */
	if( setsockopt( sckt, IPPROTO_IP, IP_OPTIONS, tsPtr, ip4_HdrOptionSize ) != 0 )  {
		perror("Error: setsocketopt() unable to set up IPv4 Header option for time stamp option" );
	}
	else  {
		if( debugFlag ) {
			printNamedByteArray( ( u_char *) tsPtr, ip4_HdrOptionSize, 20, "inetTimeStamp Structure" );
			printf( "\n" );
			displayIpOptions( ( u_char *) tsPtr, ip4_HdrOptionSize, ( verbosityLevel > 0 ));
		}
		if( sendICMP_Request( sckt, icmpMessageToTx, ( ICMP_HDR_LEN + icmpExtraDataSizeValue ), ip4_HdrID ))
			returnValue = 0;
	}
	return( returnValue );
}


/*
 * IP Header Options are in RFC 791, but also see RFC 1349, RFC 2474, RFC 6864
 */
int  sendICMP_RequestWithRecordRouteOptionInTheIP4_Hdr( struct sockaddr_in *  to, u_short  ip4_HdrID )  {
	int  returnValue = 1;
	u_char  inetRecordRoute[ MAX_IP4_HDR_OPTION_LEN ];	/* Storage for optional IP4 header options */
	u_char *  rrPtr;		/* record route pointer */
	
	rrPtr = inetRecordRoute;
	bzero((char *) rrPtr, MAX_IP4_HDR_OPTION_LEN );
	if( ip4_HdrOptionSize < 8 )  ip4_HdrOptionSize = 8;		/* Makes no sense to be smaller than header + 1 address */
	else  ip4_HdrOptionSize = 4 + ( 4 * (( ip4_HdrOptionSize - 1 ) / 4 ));	/* trim to a multiple of 4 */
	/* set IPv4 Header Options header values */
	*rrPtr++ = IPOPT_RR;
	*rrPtr++ = ( u_char )(( 3 + ( 4 * (( ip4_HdrOptionSize - 3 ) / 4 ))) & 0xff );
	*rrPtr = ( u_char )( 4 & 0xff );
	if( setsockopt( sckt, IPPROTO_IP, IP_OPTIONS, inetRecordRoute, ip4_HdrOptionSize ) != 0 )  {
		perror("Error: setsocketopt() unable to set up record route option" );
	}
	else  {
		if( debugFlag ) {
			printNamedByteArray( ( u_char *) inetRecordRoute, ip4_HdrOptionSize, 20, "inetRecordRoute Structure" );
			printf( "\n" );
			displayIpOptions( ( u_char *) inetRecordRoute, ip4_HdrOptionSize, ( verbosityLevel > 0 ));
		}
		if( sendICMP_Request( sckt, icmpMessageToTx, ( ICMP_HDR_LEN + icmpExtraDataSizeValue ), ip4_HdrID ))
			returnValue = 0;
	}
	return( returnValue );
}


void  printStats( void )  {
	int  indx;
	float percentageLoss;
	struct timespec  currentTime;	/* now time stamp */
	double  timeSinceFirstPingSent;

	printf( "%d", pingsSent );
	if( verbosityLevel > -8 )  printf( " reques%s sent", ( pingsSent == 1 ) ? "t" : "ts" );
	printf( ", " );
	printf( "%d", pingsReceived );
	if( verbosityLevel > -8 )  printf( " repl%s received", ( pingsReceived == 1 ) ? "y" : "ies" );
	/* Calculate % loss as long as some pings were sent and we will get a % between 0 & 100 */
	if(( verbosityLevel > -8 ) && ( pingsSent > 0 ) && ( pingsReceived <= pingsSent ))  {
		percentageLoss = 100.0 * (( float ) ( pingsSent - pingsReceived ) / ( float ) pingsSent );
		printf( ", %3.1f%c loss", percentageLoss, '%' );
	}
	if( verbosityLevel > -7 )  {
		clock_gettime( CLOCK_REALTIME, &currentTime );	/* get current time */
		timeSinceFirstPingSent = calcTimeSpecClockDifferenceInSeconds( &timeOfFirstPing, &currentTime );
		printf( " in %4.2f [S]", timeSinceFirstPingSent );
	}
	printf( "\n" );
	/* Print summary statistics such as minimum, average, maximum and if there is enough samples standard deviations */
	if(( verbosityLevel > -6 ) && ( statsSamplesReceived > 1 ))  {
		printf( "RTT Min %5.3f, Avg %5.3f, Max %5.3f",
			statsMinimumRTT * 1000.0, statsRunningMeanRTT * 1000.0, statsMaximumRTT * 1000.0 );
		if( statsSamplesReceived > 9 )  printf( ", StdDev %5.3f", sqrt( statsRunningVarianceRTT / ( double ) statsSamplesReceived ) * 1000.0 );
		printf( " [mS]\n" );
	}
	if( verbosityLevel > 3 )  {
		if( verbosityLevel > 4 )  printf( "\n" );	/* space it out a bit when more info is being output */
		for( indx = 0; indx < MAX_PING_ATTEMPTS; indx++ )  {
			if(( timeSentArray[ indx ].tv_nsec != 0 ) || ( timeSentArray[ indx ].tv_sec != 0 ))  {
				printf( "Info: ICMP " );
				if( M_Flag && ( icmpType >= ICMP_TYPE_TIME ))  printf( "TIMESTAMP" );
				else if( M_Flag && ( icmpType == ICMP_TYPE_MASK ))  printf( "MASK" );
				else  printf( "ECHO" );
				printf( " Request with Seq %d sent at ", sequenceID_Array[ indx ]);
				printTimeSpecTimeInHMS( &timeSentArray[ indx ], verbosityLevel );
				if(( timeReceivedArray[ indx ].tv_nsec != 0 ) || ( timeReceivedArray[ indx ].tv_sec != 0 ))  {
					printf( " & Reply received at " );
					printTimeSpecTimeInHMS( &timeReceivedArray[ indx ], verbosityLevel );
				}
				else  {
					printf( ", but no reply received" );
				}
				printf( "\n" );
			}
		}
	}
}


void  displayCurrentStatsInterrupt( int  signo )  {
	printStats();
	if( signo == SIGUSR1 )  {	/* if activated by user interrupt then reinstate it */
		signal( SIGUSR1, displayCurrentStatsInterrupt );
	}
}


void  finishOnUserInterrupt( int signo )  {
	if( signo == SIGALRM )  {	/* if activated by timer interrupt then turn off timer */
		signal( SIGALRM, SIG_IGN );
		setSendTimer( 0L, 0L );	/* set timer off */
	}
	else  {
		if( verbosityLevel > 0 )  psignal( signo, "\nTerminating program due to user generated signal" );
		else  printf( "\n" );
	}
	if( verbosityLevel > -9 )  {
		if( verbosityLevel > 4 )  printf( "\n" );
		printStats();	/* Unless super quiet print the stats about requests sent, replies received etc */
	}
	if( debugFlag )
		printf( "Debug: ping3 terminating with %s status from finishOnUserInterrupt( %d )\n",
			( pingsReceived < pingsSent ) ? "EXIT_FAILURE" : (( pingsSent == 0 ) ? "EXIT_FAILURE" : "EXIT_SUCCESS" ), signo );
	fflush( stdout );	/* Ensure anything printed to stdout is output to the user terminal */
	fflush( stderr );	/* Ensure anything printed is stderr output to the user terminal */
 /* Terminate the program & report a failure if no pings were sent or more requests were sent than replies received */
	exit( ( pingsReceived < pingsSent ) ? EXIT_FAILURE : (( pingsSent == 0 ) ? EXIT_FAILURE : EXIT_SUCCESS ));
}


/*
 * Send an ICMP request each time the timer interrupt activates this routine.
 */
void  sendPing( int signo )  {
	struct timespec  currentTime;

	/* Flag a request without a reply just before the next request is sent */
	if(( verbosityLevel > -5 ) && ( pingsSent > pingsReceived ))  {
		clock_gettime( CLOCK_REALTIME, &currentTime );	/* get current time */
		printf( "No ICMP " );
		if( M_Flag && ( icmpType >= ICMP_TYPE_TIME ))  printf( "TIMESTAMP" );
		else if( M_Flag && ( icmpType == ICMP_TYPE_MASK ))  printf( "MASK" );
		else  printf( "ECHO" );
		printf( " reply from %s: seq %d in %lg [S]\n", remoteDeviceNameBuffer, icmpHdrID,
			calcTimeSpecClockDifferenceInSeconds( &timeSentArray[ icmpHdrID ], &currentTime ));
	}
	icmpHdrID += 1;	/* bump the IPv4 ICMP datagram ID */
	if( s_Flag && ( icmpFirstExtraDataSizeValue != icmpSecondExtraDataSizeValue ))  {
		if( icmpHdrID != 1 )  icmpExtraDataSizeValue += icmpStepExtraDataSizeValue;		/* after the first ping start the sweep */
		if(( icmpFirstExtraDataSizeValue > icmpSecondExtraDataSizeValue ) &&
			(( icmpExtraDataSizeValue > icmpFirstExtraDataSizeValue ) ||		/* too high ? */
			( icmpExtraDataSizeValue < icmpSecondExtraDataSizeValue )))  {	/* too low ? */
				icmpStepExtraDataSizeValue = -1 * icmpStepExtraDataSizeValue;
				icmpExtraDataSizeValue += 2 * icmpStepExtraDataSizeValue;
		}
		else if(( icmpFirstExtraDataSizeValue < icmpSecondExtraDataSizeValue ) &&
			(( icmpExtraDataSizeValue > icmpSecondExtraDataSizeValue ) ||		/* too high ? */
			( icmpExtraDataSizeValue < icmpFirstExtraDataSizeValue )))  {	/* too low ? */
				icmpStepExtraDataSizeValue = -1 * icmpStepExtraDataSizeValue;
				icmpExtraDataSizeValue += 2 * icmpStepExtraDataSizeValue;
		}
	}
	if( T_Flag )  {		/* Do IP4 header option requesting time stamp info */
		if( sendICMP_RequestWithTimeStampOptionInTheIP4_Hdr( to, ( struct ip * ) ip4_DatagramRxd, icmpHdrID ) == 0 )
			pingsSent += 1;
	}
	else if( R_Flag )  {	/* Do IP4 header option requesting record route info */
		if( sendICMP_RequestWithRecordRouteOptionInTheIP4_Hdr( to, icmpHdrID ) == 0 )
			pingsSent += 1;
	}
   	else  {				/* Do pings without IP4 header options */
		if( sendICMP_Request( sckt, icmpMessageToTx, ( ICMP_HDR_LEN + icmpExtraDataSizeValue ), icmpHdrID ))	/* Another ping like the first */
			pingsSent += 1;
	}
 	/* Do more pings if required */
	if( ! continousPingMode && ( --pingSendAttemptsLeft <= 0 ))  {
		signal( SIGALRM, finishOnUserInterrupt );
		setSendTimer(( u_long ) waitTimeInSec, 0L );	/* set timer to wait a period of time after the last transmit (default 2 sec) */
	}
	else {
		signal( SIGALRM, sendPing );
		setSendTimer(( u_long ) intervalBwPingsInMicroSeconds / USEC_CONV_ULONG_CONST,
			( u_long ) intervalBwPingsInMicroSeconds % USEC_CONV_ULONG_CONST );	/* set send next ping interrupt */
			/* Set up a break between echo requests */
	}
}


int  setUpIP_AddressAndName( struct sockaddr_in *  devInfoPtr, char *  devNameOrIP_Str, char **  nameBfrPtr )   {
	int  i, resultOK = FALSE;
	struct hostent *  hostEntPtr;		/* host pointer */

/* Set up the address to send the icmp echo request */
	devInfoPtr->sin_family = AF_INET;
	hostEntPtr = NULL;	/* Make sure host entry pointer is initialized to NULL for later test */
     /* Try to convert command line Remote Network Device specifier as dotted quad (decimals) address, */
     /* else if that fails assume it's a Network Device name */
	resultOK = ( strIsA_ValidDottedQuadIPv4_Address( devNameOrIP_Str ) && 
		(( devInfoPtr->sin_addr.s_addr = inet_addr( devNameOrIP_Str )) != (u_int) -1 ));	/* attempt to set address from a Dotted Quad IP address */
	if( resultOK )  {
		if(( hostEntPtr = gethostbyaddr( &devInfoPtr->sin_addr.s_addr, 4, AF_INET )) == NULL )  {
			if( debugFlag )
				fprintf( stderr, "Warning: gethostbyaddr() put h_errno = %d for target IPv4 address \"%s\"\n",
					h_errno, inet_ntoa( *(struct in_addr *) &devInfoPtr->sin_addr.s_addr ));
			if( verbosityLevel > 1 )
				fprintf( stderr, "Warning: Unable to get name for IPv4 address \"%s\": %s\n", devNameOrIP_Str, hstrerror( h_errno ));
			*nameBfrPtr = strdup( inet_ntoa( *(struct in_addr *) &devInfoPtr->sin_addr.s_addr ));	/* copy IPv4 address of remote Network Device as the name */
		}
		else  {
			devInfoPtr->sin_family = hostEntPtr->h_addrtype;
			bcopy( hostEntPtr->h_addr, (caddr_t)&devInfoPtr->sin_addr, hostEntPtr->h_length);
			*nameBfrPtr = strdup( hostEntPtr->h_name );	/* copy official name of remote Network Device */
			if( verbosityLevel > 0 )  {
				printf( "Remote Network Device has official name: \"%s\"\n", *nameBfrPtr );
				if( verbosityLevel > 1 )  {
			 /* Print other names of same Network Device if there are any */
					for( i = 0; hostEntPtr->h_aliases[ i ] != NULL; i++ )  {
						printf( "Remote Network Device name alias %d is: \"%s\"\n", i + 1, hostEntPtr->h_aliases[ i ] );
					}
				}
			}
		}
	}
	else  {	/* Failed to set address through inet_addr() */
		resultOK = (( hostEntPtr = gethostbyname( devNameOrIP_Str )) != NULL );	
		if( ! resultOK )  {
			fprintf( stderr, "Warning: Network Device name \"%s\": %s\n", devNameOrIP_Str, hstrerror( h_errno ));
			if( verbosityLevel > 1 )  {
				switch( h_errno )  {
					case  HOST_NOT_FOUND :
						fprintf( stderr, "Warning: no such Network Device as \"%s\"\n", devNameOrIP_Str ); break;
					case  TRY_AGAIN :
						fprintf( stderr, "Warning: Network Device \"%s\", try again later\n", devNameOrIP_Str ); break;
					case  NO_RECOVERY :
						fprintf( stderr, "Warning: Network Device \"%s\" DNS error\n", devNameOrIP_Str ); break;
					case  NO_ADDRESS :
						fprintf( stderr, "Warning: no IP address for \"%s\"\n", devNameOrIP_Str ); break;
					default :
						fprintf( stderr, "Warning: unrecognized error with error number: %d for \"%s\"\n", h_errno, devNameOrIP_Str );
						break;
				}
			}
			if( verbosityLevel > 2 )  fprintf( stderr, "Warning: gethostbyname() put h_errno = %d for target \"%s\"\n", h_errno, devNameOrIP_Str );
		}
		else  {
			devInfoPtr->sin_family = hostEntPtr->h_addrtype;
			bcopy( hostEntPtr->h_addr, (caddr_t)&devInfoPtr->sin_addr, hostEntPtr->h_length);
			*nameBfrPtr = strdup( hostEntPtr->h_name );	/* copy official name of remote Network Device */
			if( verbosityLevel > 0 )  {
				printf( "Remote Network Device has official name: \"%s\"\n", *nameBfrPtr );
				if( verbosityLevel > 1 )  {
			 /* Print other names of same Network Device if there are any */
					for( i = 0; hostEntPtr->h_aliases[ i ] != NULL; i++ )  {
						printf( "Remote Network Device name alias %d is: \"%s\"\n", i + 1, hostEntPtr->h_aliases[ i ] );
					}
				}
			}
		}
	}
	if( resultOK && ( verbosityLevel > 0 ))  {
		printf( "Remote Network Device has IPv4 address: %s\n", inet_ntoa( *(struct in_addr *) &devInfoPtr->sin_addr.s_addr ));
	 /* Print other IP addresses if there are any */
		if( verbosityLevel > 1 )  {
		    if( hostEntPtr != NULL )  {
				for( i = 1; hostEntPtr->h_addr_list[ i ] != NULL; i++ )  {
					printf( "Remote Network Device alias %d is: \"%s\"\n",
						i, inet_ntoa( *(struct in_addr *) hostEntPtr->h_addr_list[ i ] ));
				}
			}
		}
	}
	return( resultOK );
}


/* Help/Usage information */
void  useage( char *  name )  {
	printf( "\nuseage: %s [options] NetworkDeviceName\n", name );
	printf( "or      %s [options] NetworkDeviceIP_Number\n", name );
	printf( "\nwhere options are; -\n" );
	printf( "        -a  switches on audible output notification of replies received\n" );
	printf( "        -b  switches on broadcast permission for requests to be sent to a broadcast address\n" );
	printf( "        -cX  specifies number of times to ping remote network device ( %d <= X <= %d )\n", MIN_PING_ATTEMPTS, MAX_PING_ATTEMPTS );
	printf( "          where a value of 0 invokes continuous ping mode. Stop this mode with control-C or control-\\.\n" );
	if( isIPv4_DontFragmentManipulatableOnThisOS_Version())
		printf( "        -D  switches on IPv4 header Don't Fragment setting in ICMP request datagrams\n" );
	printf( "        -h  switches on this help output and then terminates %s\n", name );
	printf( "        -iX.X  ensure X.X second interval between each ping ( %.1f <= X.X <= %.1f )\n", MIN_INTERVAL_BW_PINGS, MAX_INTERVAL_BW_PINGS );
	printf( "        -lXX  suggest desired IP header option length (max is 40 and should be a multiple of 4)\n" );
	printf( "        -M ABC  specifies ping with ICMP Mask/Timestamp request instead of ICMP Echo.\n" );
	printf( "          where ABC is a string of characters.\n" );
	printf( "            If \"mask\" then send ICMP Mask request,\n" );
	printf( "            if \"time\" then send ICMP Time Stamp request,\n" );
	printf( "            if \"timew\" as above, but treat tsr and tst timestamps as little endian.\n" );
	printf( "            (i.e. Windows default response, if the ICMP Time Stamp request\n" );
	printf( "            is allowed through the Windows firewall. )\n" );
	printf( "        -p ABC  specifies a pattern for the data payload included with the ping.\n" );
	printf( "          where ABC is a string of characters.\n" );
	printf( "            if \"time\" then send milliseconds since midnight as the data payload,\n" );
	printf( "            if \"random\" then send a random array of bytes as the data payload,\n" );
	printf( "            if neither of the above then specifying up to %d bytes in hexadecimal.\n", ICMP_PAYLOAD_MAX_PATTERN_SIZE );
	printf( "        -R  specifies header option Record Route (N.B. -T overrides -R when both are specified)\n" );
	printf( "        -s \"XX [YY [ZZ]]\" specifies ICMP Echo data section size (N.B. 16 <= XX <= 1472 works best on macOS)\n" );
	printf( "          where XX is an integer number and YY, ZZ are optional integer numbers to specify a size sweep.\n" );
	printf( "        -tXX  specifies IPv4 header Time-to-Live setting ( 1 <= XX <= 255 )\n" );
	printf( "        -T ABC  specifies header option time stamp type.\n" );
	printf( "          where ABC is a string of characters.\n" );
	printf( "            If \"tsonly\" then record Time Stamp Only list of time stamps,\n" );
	printf( "            if \"tsandaddr\" then record Address and Time Stamp pair list,\n" );
	printf( "            if \"tsprespec H.I.J.K [ L.M.N.O [ P.Q.R.S [ T.U.V.W ]]]\" then Time Stamp prespecified Addresses.\n" );
	printf( "        -vX  sets a verbosity level ( -9 <= X <= 9 ) More positive values of X provide more information.\n" );
	printf( "             a verbosity level of zero ( -v 0 ) is equivalent to not using -v.\n" );
	printf( "        -wX  wait for X seconds after last request for any replies ( %d <= X <= %d )\n", MIN_WAIT_PERIOD, MAX_WAIT_PERIOD );
	printf( "\n" );
} 


int  processCommandLineOptions( int  argc, char *  argv[] )  {
	int  i, result, returnValue, icmpMessageSize;
	char ** argPtr = NULL;
	char *  argArray[ 5 ] = { NULL, NULL, NULL, NULL, NULL };

	/* Set all the global flags from command line options */
	opterr = 0;	/* Suppress error messages from getopt() to stderr */
	/* Process switch options from the command line */
	while(( result = getopt( argc, argv, optionStr )) != -1 )  {
    	switch( result )  {
			case 'a' :  a_Flag = TRUE; break;					/* Audible reply receive */
			case 'b' :  b_Flag = TRUE; break;					/* Broadcast permission for requests */
    		case 'c' :  c_Flag = TRUE; c_Strng = optarg; break;	/* Count of ping requests to be sent */
			case 'D' :  D_Flag = isIPv4_DontFragmentManipulatableOnThisOS_Version(); break;		/* Don't Fragment setting on requests */
			case 'h' :  helpFlag = TRUE; break;
			case 'i' :  i_Flag = TRUE; i_Strng = optarg; break;	/* time interval between ping transmits */
			case 'l' :  l_Flag = TRUE; l_Strng = optarg; break;	/* IPv4 header option section length */
			case 'M' :  M_Flag = TRUE; M_Strng = optarg; break;	/* ICMP Mode - Echo, Timestamp or Network Mask */
    		case 'p' :  p_Flag = TRUE; p_Strng = optarg; break;	/* ICMP payload data spec */
			case 'R' :  R_Flag = TRUE; break;					/* IPv4 header option Record Route */
    		case 's' :  s_Flag = TRUE; s_Strng = optarg; break;	/* ICMP payload data size */
    		case 't' :  t_Flag = TRUE; t_Strng = optarg; break;	/* IPv4 ttl */
    		case 'T' :  T_Flag = TRUE; T_Strng = optarg; break;	/* IPv4 header option Time Stamp */
    		case 'v' :  v_Flag = TRUE; v_Strng = optarg; break;	/* set up a verbosity level */
    		case 'w' :  w_Flag = TRUE; w_Strng = optarg; break;	/* Wait time */
    		default :
        		printf( "Warning: command line option -%c is unrecognised or incomplete and has been ignored\n", optopt );
        		printf( "Information: For help on command line options run \"%s -h\"\n", exeName );
        		break;
    	}
	}
	/* -v level processing to set verbosity level and set quietFlag and debugFlag */
	verbosityLevel = convertOptionStringToInteger( 0, v_Strng, "-v", &v_Flag, TRUE );
	verbosityLevel = limitIntegerValueToEqualOrWithinRange( verbosityLevel, -9, 9 );
#ifdef DEBUG
	debugFlag = ( verbosityLevel >= 9 );	/* Report all debug information only if compiled with DEBUG defined */
#endif
	quietFlag = ( verbosityLevel <= -5 );
	/* -c count of requests to be sent */
	pingSendAttemptsLeft = convertOptionStringToInteger( DEFAULT_PING_COUNT, c_Strng, "-c", &c_Flag, TRUE );
	pingSendAttemptsLeft = limitIntegerValueToEqualOrWithinRange( pingSendAttemptsLeft, MIN_PING_ATTEMPTS, MAX_PING_ATTEMPTS );
	continousPingMode = ( pingSendAttemptsLeft == 0);	/* Treat user setting zero pings as the special case of continuos pings */
	/* -i time interval */
	intervalBwPingsInSeconds = convertOptionStringToDouble(( double ) DEFAULT_INTERVAL_BW_PINGS, i_Strng, "-i", &i_Flag, TRUE );
	intervalBwPingsInSeconds = limitDoubleValueToEqualOrWithinRange( intervalBwPingsInSeconds, ( double ) MIN_INTERVAL_BW_PINGS, ( double ) MAX_INTERVAL_BW_PINGS );
	intervalBwPingsInMicroSeconds = limitUnsignedLongValueToEqualOrWithinRange(( u_long )( intervalBwPingsInSeconds * 1000000.0 ),
		( u_long ) 100000L, ( u_long ) 60000000L );	/* Re-inforce limits in-case conversion somehow gave something unexpected */
	/* -l IP header option length */
	ip4_HdrOptionSize = convertOptionStringToInteger( 0, l_Strng, "-l", &l_Flag, TRUE );
	if( l_Flag )  {
		ip4_HdrOptionSize = 4 * (( ip4_HdrOptionSize + 3 ) / 4 );	/* force option length to be a multiple of 4 bytes */
		ip4_HdrOptionSize = limitIntegerValueToEqualOrWithinRange( ip4_HdrOptionSize, 8, MAX_IP4_HDR_OPTION_LEN );
	}
	/* -t IP time-to-live */
	ip4_HeaderTTL_Value = convertOptionStringToInteger( 1, t_Strng, "-t", &t_Flag, TRUE );
	ip4_HeaderTTL_Value = limitIntegerValueToEqualOrWithinRange( ip4_HeaderTTL_Value, 0, 255 );
	ip4_OptionTS_Value = -1;
	/* -T IP header option time */
	if( T_Flag && ( T_Strng != NULL ))  {
		if( strncmp( T_Strng, "tsonly", 6 ) == 0 )  {
			ip4_OptionTS_Value = IPOPT_TS_TSONLY;
			if( ! l_Flag )  ip4_HdrOptionSize = MAX_IP4_HDR_OPTION_LEN;
		}
		else if( strncmp( T_Strng, "tsandaddr", 9 ) == 0 )  {
			ip4_OptionTS_Value = IPOPT_TS_TSANDADDR;
			if( ! l_Flag )  ip4_HdrOptionSize = MAX_IP4_HDR_OPTION_LEN - 4;
		}
		else if( strncmp( T_Strng, "tsprespec", 9 ) == 0 )  {
			ip4_OptionTS_Value = IPOPT_TS_PRESPEC;
			for( argPtr = argArray; ( *argPtr = strsep( &T_Strng, " \t,")) != NULL; )  {
				if( **argPtr != '\0' )
					if( ++argPtr >= &argArray[ 5 ])  break;
			}
			if( debugFlag )
				for( i = 0; ( argArray[ i ] != NULL ) && ( i < 5 ); ++i )
					printf( "Time Stamp Prespec argArray[ %d ] is \"%s\"\n", i, argArray[ i ]);
			returnValue = TRUE;
			ip4_HdrOptionSize = 4;
			for( i = 0; ( returnValue && ( i < 4 ) && ( argArray[ i + 1 ] != NULL ) && ( *argArray[ i + 1 ] != '\0' )); i++ )  {
				returnValue = setUpIP_AddressAndName(( struct sockaddr_in *) &preSpecDevice[ i ], argArray[ i + 1 ], &prespecDeviceNameBuffer[ i ] );
				if( returnValue )  ip4_HdrOptionSize += 8;
			}
			/* if no devices are found in the list then the default is to automatically set the target and the localhost */
			if( ip4_HdrOptionSize == 4 )  ip4_HdrOptionSize = 20;
		}
		else  {
			T_Flag = FALSE;		/* Forget that user specified -T */
			ip4_OptionTS_Value = -1;	/* force default of no IPv4 header option timestamp */
			ip4_HdrOptionSize = 0;
			printf( "Warning: \"-T %s\" not recognized (-h for help) - attempting ICMP request without header option timestamps instead.\n", T_Strng );
		}
	}
	/* -p ICMP payload data pattern */
	if( p_Flag )  {		/* determine a pattern to use as the ICMP (Echo) payload data pattern */
		if( strncmp( p_Strng, "random", 6 ) == 0 )  icmpPayloadPatternType = ICMP_PAYLOAD_RANDOM_BYTE_PATTERN;
		else if( strncmp( p_Strng, "time", 4 ) == 0 )  icmpPayloadPatternType = ICMP_PAYLOAD_TIME_PATTERN;
		else  {
			icmpPayloadPatternType = ICMP_PAYLOAD_HEX_PATTERN;
			icmpPayloadPatternSize = convertHexByteStringToByteArray( p_Strng, icmpPayloadPattern, ICMP_PAYLOAD_MAX_PATTERN_SIZE );
			p_Flag = ( icmpPayloadPatternSize > 0 );	/* reset flag if pattern specified wasn't valid */
			if( ! p_Flag )  icmpPayloadPatternType = ICMP_PAYLOAD_NO_PATTERN;
			else  if( verbosityLevel > 2 )  printNamedByteArray( icmpPayloadPattern, icmpPayloadPatternSize, 10, "ICMP Payload Pattern" );
		}
	}
	/* -R IP header record route */
	/* By default set maximum available space for Record Route option */
	if( R_Flag && ! l_Flag )  ip4_HdrOptionSize = MAX_IP4_HDR_OPTION_LEN;
	/* Ensure any IP Header option timestamp is one of the known types */
	ip4_OptionTS_Value = limitIntegerValueToEqualOrWithinRange( ip4_OptionTS_Value, -1, IPOPT_TS_PRESPEC );
	/* -M ICMP type */
	icmpType = ICMP_TYPE_ECHO;		/* default to sending ICMP Echo request datagrams */
	icmpMessageSize = ICMP_HDR_LEN;
	if( M_Flag && ( M_Strng != NULL ))  {
		if( strncmp( M_Strng, "echo", 4 ) == 0 )  icmpType = ICMP_TYPE_ECHO;
		else if( strncmp( M_Strng, "mask", 4 ) == 0 )  {
			icmpType = ICMP_TYPE_MASK;
			icmpMessageSize = ICMP_HDR_LEN + 4;
		}
		else if( strncmp( M_Strng, "timew", 5 ) == 0 )  {
			icmpType = ICMP_TYPE_TIMEW;
			icmpMessageSize = ICMP_HDR_LEN + 12;
		}
		else if( strncmp( M_Strng, "time", 4 ) == 0 )  {
			icmpType = ICMP_TYPE_TIME;
			icmpMessageSize = ICMP_HDR_LEN + 12;
		}
		else if( strncmp( M_Strng, "info", 4 ) == 0 )
			printf( "Warning: \"-M info\" deprecated - using ICMP Echo request instead.\n" );
		else  {
			M_Flag = FALSE;		/* Forget that user specified -M */
			printf( "Warning: \"-M %s\" not recognized (-h for help) - attempting ICMP Echo request instead.\n", M_Strng );
		}
	}
	icmpType = limitIntegerValueToEqualOrWithinRange( icmpType, 0, ICMP_TYPE_TIMEW );
	/* -w wait between transmits */
	waitTimeInSec = convertOptionStringToInteger( DEFAULT_WAIT_PERIOD, w_Strng, "-w", &w_Flag, TRUE );
	waitTimeInSec = limitIntegerValueToEqualOrWithinRange( waitTimeInSec, MIN_WAIT_PERIOD, MAX_WAIT_PERIOD );
	/* -s ICMP payload data */
	/* Do size last so that ip4_HdrOptionSize is settled */
	for( argPtr = argArray; ( *argPtr = strsep( &s_Strng, " \t,")) != NULL; )  {
		if( **argPtr != '\0' )
			if( ++argPtr >= &argArray[ 3 ])  break;
	}
	if( debugFlag )
		for( i = 0; ( argArray[ i ] != NULL ) && ( i < 3 ); ++i )
			printf( "Size of ICMP payload: argArray[ %d ] is \"%s\"\n", i, argArray[ i ]);
	icmpFirstExtraDataSizeValue = convertOptionStringToInteger( DEFAULT_ICMP_EXTRA_DATA, argArray[ 0 ], "-s", &s_Flag, TRUE );
	icmpFirstExtraDataSizeValue = limitIntegerValueToEqualOrWithinRange( icmpFirstExtraDataSizeValue, 0,
		MAX_IP4_PACKET_LEN - ( STD_IP4_HDR_LEN + ip4_HdrOptionSize + icmpMessageSize ));
	icmpExtraDataSizeValue = icmpFirstExtraDataSizeValue;	/* use first value after -s */
	if( argArray[ 1 ] != NULL )  {
		icmpSecondExtraDataSizeValue = convertOptionStringToInteger( icmpFirstExtraDataSizeValue, argArray[ 1 ], "-s", &s_Flag, FALSE );
		icmpSecondExtraDataSizeValue = limitIntegerValueToEqualOrWithinRange( icmpSecondExtraDataSizeValue, 0,
			MAX_IP4_PACKET_LEN - ( STD_IP4_HDR_LEN + ip4_HdrOptionSize + icmpMessageSize ));
		if( icmpSecondExtraDataSizeValue > icmpFirstExtraDataSizeValue )  {
			icmpStepExtraDataSizeValue = 1;
		}
		else if( icmpSecondExtraDataSizeValue < icmpFirstExtraDataSizeValue )  {
			icmpStepExtraDataSizeValue = -1;
		}
		if( argArray[ 2 ] != NULL )  {
			icmpStepExtraDataSizeValue = convertOptionStringToInteger( icmpStepExtraDataSizeValue, argArray[ 2 ], "-s", &s_Flag, FALSE );
		}
		if( icmpSecondExtraDataSizeValue == icmpFirstExtraDataSizeValue )  icmpStepExtraDataSizeValue = 0;
		else if( icmpSecondExtraDataSizeValue > icmpFirstExtraDataSizeValue )  {
			if( icmpStepExtraDataSizeValue < 0 )  icmpStepExtraDataSizeValue = -1 * icmpStepExtraDataSizeValue;
		}
		else  {
			if( icmpStepExtraDataSizeValue > 0 )  icmpStepExtraDataSizeValue = -1 * icmpStepExtraDataSizeValue;
		}
		/* Don't allow step size bigger than the range */
		icmpStepExtraDataSizeValue = limitIntegerValueToEqualOrWithinRange( icmpStepExtraDataSizeValue,
			-1 * abs( icmpFirstExtraDataSizeValue - icmpSecondExtraDataSizeValue ),
			abs( icmpFirstExtraDataSizeValue - icmpSecondExtraDataSizeValue ));
	}
#ifdef  DEBUG
	if( debugFlag )  {
		printf( "Debug: First ( start ) -s value %d\n", icmpFirstExtraDataSizeValue );
		printf( "Debug: Second ( finish ) -s value %d\n", icmpSecondExtraDataSizeValue );
		printf( "Debug: Third ( step ) -s value %d\n", icmpStepExtraDataSizeValue );
	}
#endif
	/* make sure TX buffer is big enough to hold max amount of payload data */
	txICMP_BfrSize = icmpMessageSize + (( icmpSecondExtraDataSizeValue > icmpFirstExtraDataSizeValue ) ? icmpSecondExtraDataSizeValue : icmpFirstExtraDataSizeValue);
	rxIPv4_BfrSize = txICMP_BfrSize + STD_IP4_HDR_LEN + ip4_HdrOptionSize;	/* Receive Buffer must be bigger than Transmit Buffer */
	/* Provide feedback on what options are in play in Debug mode */
	if( debugFlag )  {
		printf( "Debug: -a option ( audible ) is %s\n", ( a_Flag ) ? "active" : "inactive" );
		printf( "Debug: -b option ( broadcast ) is %s\n", ( b_Flag ) ? "active" : "inactive" );
		printf( "Debug: -c option ( count ) is %s\n", ( c_Flag ) ? "active" : "inactive" );
	}
	if( isIPv4_DontFragmentManipulatableOnThisOS_Version())  {
		if( debugFlag )	printf( "Debug: -D option ( don't fragment ) is %s\n", ( D_Flag ) ? "active" : "inactive" );
	}
	else if( D_Flag )  {
		printf( "Warning: IPv4 header Don't Fragment manipulation is not available on this version of the OS" );
	}
	if( debugFlag )  {
		printf( "Debug: -h option ( help ) is %s\n", ( helpFlag ) ? "active" : "inactive" );
		printf( "Debug: -i option ( interval ) is %s\n", ( i_Flag ) ? "active" : "inactive" );
		printf( "Debug: -l option ( header option length ) is %s\n", ( l_Flag ) ? "active" : "inactive" );
		printf( "Debug: IPv4 Header Option length is %d bytes\n", ip4_HdrOptionSize );
		printf( "Debug: -M option ( ICMP Mode type ) is %s\n", ( M_Flag ) ? "active" : "inactive" );
		printf( "Debug: -p option ( pattern ) is %s\n", ( p_Flag ) ? "active" : "inactive" );
		printf( "Debug: -R option ( header Record Route ) is %s\n", ( R_Flag ) ? "active" : "inactive" );
		printf( "Debug: -s option ( echo payload size ) is %s\n", ( s_Flag ) ? "active" : "inactive" );
		printf( "Debug: -T option ( header time-stamp ) is %s\n", ( T_Flag ) ? "active" : "inactive" );
		printf( "Debug: -t option ( time-to-live ) is %s\n", ( t_Flag ) ? "active" : "inactive" );
		printf( "Debug: -v option ( verbosity level ) is %s\n", ( v_Flag ) ? "active" : "inactive" );
		printf( "Debug: -w option ( wait ) is %s\n", ( w_Flag ) ? "active" : "inactive" );
		printf( "Debug: Transmit Buffer Size is %d, Receive Buffer Size is %d\n", txICMP_BfrSize, rxIPv4_BfrSize );
	}
	/* return the option index of the first positional argument (i.e. network device name or IPv4 address?) */
	return( optind );
}


/* Preset switch option Flags and Data */
void  setGlobalFlagDefaults( char *  argv[] )  {	/* Set up any Global variables not already set at declaration */
	int  i;

	helpFlag = debugFlag = quietFlag = FALSE;
	a_Flag = FALSE;		/* Audible */
	b_Flag = FALSE;		/* Broadcast */
	c_Flag = FALSE;		/* Count */
	pingSendAttemptsLeft = DEFAULT_PING_COUNT;
	continousPingMode = FALSE;		/* normal mode is a specific number of pings */
	D_Flag = FALSE;		/* Don't Fragment flag */
	i_Flag = FALSE;		/* TRUE when user has specified interval between pings */
	intervalBwPingsInSeconds = ( double ) 1.0;
	intervalBwPingsInMicroSeconds = USEC_CONV_ULONG_CONST;
	l_Flag = FALSE;		/* TRUE when user has suggested length of the IP4 Header Option section */
	ip4_HdrOptionSize = MAX_IP4_HDR_OPTION_LEN;
	M_Flag = FALSE;		/* TRUE when user has specified ICMP Type (i.e. Echo, Mask or Time request ) */
	icmpType = ICMP_TYPE_ECHO;		/* ICMP Type Echo/Mask/Time request - Default is Echo */
	p_Flag = FALSE;		/* TRUE when a pattern is specified for the data in the ICMP payload */
	icmpPayloadPatternType = ICMP_PAYLOAD_NO_PATTERN;		/* Set to no pattern in the payload */
	icmpPayloadPatternSize = 0;
	bzero( icmpPayloadPattern, ICMP_PAYLOAD_MAX_PATTERN_SIZE );
	R_Flag = FALSE;		/* TRUE when user has specified IP header option Record Route */
	s_Flag = FALSE;		/* TRUE when user has specified ICMP payload data size in the command line options */
	icmpExtraDataSizeValue = 0;		/* Specifies the ICMP message payload size (i.e. not total length) */
	icmpFirstExtraDataSizeValue = 0;	/* Specifies the starting message payload size */
	icmpSecondExtraDataSizeValue = 0;	/* Specifies the turning point message payload size */
	icmpStepExtraDataSizeValue = 0;		/* Specifies the step size used to move from starting size to turning point size */
	t_Flag = FALSE;		/* IP header Time to Live is required to be other than default of 64 */
	ip4_HeaderTTL_Value = 1;
	T_Flag = FALSE;		/* TRUE when user has specified IPv4 header option Timestamp */
	ip4_OptionTS_Value = -1;	/* -1 means NO IP header option time stamp */
	v_Flag = FALSE;		/* Verbosity Level setting */
	verbosityLevel = 0;	/* not verbose */
	w_Flag = FALSE;		/* Wait time */
	waitTimeInSec = DEFAULT_WAIT_PERIOD;
	pingsSent = 0;		/* track pings requests sent */
	pingsReceived = 0;	/* track ping replies received */
	statsSamplesReceived = 0;				/* track number of samples in the stats */
	statsMinimumRTT = ( double ) DBL_MAX;	/* start with outrageously large value for the minimum RTT */
	statsRunningMeanRTT = ( double ) 0.0;	/* track the mean RTT time */
	statsMaximumRTT = ( double ) 0.0;		/* start with a small value for the maximum RTT */
	statsRunningVarianceRTT = ( double ) 0.0;	/* track variance of RTT */
	bzero((char *) &remoteDeviceToPingInfo, sizeof( struct sockaddr ));
	for( i = 0; i < 4; i++ )  {
		bzero((char *) &preSpecDevice[ i ], sizeof( struct sockaddr ));
		prespecDeviceNameBuffer[ i ] = ( char * ) NULL;
	}
	remoteDeviceNameBuffer = ( char * ) NULL;
	localDeviceNameBuffer = ( char * ) NULL;
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
	int i;

	if( exePath != NULL )  free(( void *) exePath );
	if( ip4_DatagramRxd != NULL )  free(( void *) ip4_DatagramRxd );
	if( icmpMessageToTx != NULL )  free(( void *) icmpMessageToTx );
	for( i = 0; i < 4; i++ )  {
		if( prespecDeviceNameBuffer[ i ] != NULL )  free(( void *) prespecDeviceNameBuffer[ i ] );
	}
	if( remoteDeviceNameBuffer != NULL )  free(( void *) remoteDeviceNameBuffer );
	if( localDeviceNameBuffer != NULL )  free(( void *) localDeviceNameBuffer );
}


int  getLocalDeviceName( char **  nameBufferPtr )  {
	int  returnVal;
	char  tempBuffer[ MAXHOSTNAMELEN ];

	returnVal = ( gethostname( tempBuffer, MAXHOSTNAMELEN ) != -1 );
	if( returnVal )  *nameBufferPtr = strdup( tempBuffer );
	return( returnVal );
}


int  main( int  argc, char *  argv[] )  {
	int  returnValue, commandLineIndex;
	int  scktOpt = 0;
	struct protoent	*  proto;	/* allows protocol to be set to ICMP */

/* Ensure any allocated memory is free'd */
	atexit( cleanupStorage );

/* Preset Global variables, switch option Flags and Data */
	setGlobalFlagDefaults( argv );	/* Set up any Global variables not already set at declaration */

/* Preset Local variables, option Flags and Data */
	returnValue = 1;

/* Process switch options from the command line */
	commandLineIndex = processCommandLineOptions( argc, argv );

/* Version information */
	if( verbosityLevel > 4 )  printVersion();

/* Print useage message and exit if the "-h" flag was amongst the command line options */
/*  or if no icmp echo target is specified ) */
	if( helpFlag || ( argc == commandLineIndex )) {
		useage( exeName );
		return( 3 );
	}

/* Attempt to get the name of the Local network device */	
	haveLocalHostNameFlag = getLocalDeviceName( &localDeviceNameBuffer );
	if( ! haveLocalHostNameFlag )  {
		if( verbosityLevel > 0 )  printf( "Warning: unable to get actual Local host name, using \"localhost\" instead.\n" );
		localDeviceNameBuffer = strdup( "localhost" );
	}
	else if( verbosityLevel > 0 )  printf( "Local Network Device has name: \"%s\"\n", localDeviceNameBuffer );

/* Set up the address to send the icmp echo request */
	to = ( struct sockaddr_in *) &remoteDeviceToPingInfo;
	returnValue = setUpIP_AddressAndName( to, argv[ commandLineIndex ], &remoteDeviceNameBuffer);
	if( returnValue == FALSE )  return( 2 );	/* Don't go further if no address can be determined */

	if(( ip4_DatagramRxd = (u_char *) malloc( (u_int) rxIPv4_BfrSize )) == NULL ) {
		fprintf( stderr, "Error: Unable to create storage of %d bytes for received datagram\n", rxIPv4_BfrSize );
	}
	else if(( icmpMessageToTx = (u_char *) malloc( (u_int) txICMP_BfrSize )) == NULL ) {
		fprintf( stderr, "Error: Unable to create storage of %d bytes for transmit datagram data\n", txICMP_BfrSize );
	}
	else if(( proto = getprotobyname( "icmp" )) == NULL ) {
		fprintf(stderr, "Error: Unknown protocol ICMP\n");
	}
	else if(( sckt = socket( AF_INET, SOCK_TYPE_TO_USE, proto->p_proto )) < 0 ) {
		perror("Error: Unable to create socket to send ICMP packets");
		/* Make suggestion of using sudo if not invoked as root or setuid root or with sudo privilege */
		if(( SOCK_TYPE_TO_USE == SOCK_RAW ) && ( getuid() != 0 ) && ( geteuid() != 0 ))
			fprintf( stderr, "This utility uses raw sockets and requires privilege to do so. Try using sudo\n" );
	}
	else  {
		signal( SIGINT, finishOnUserInterrupt );	/* Trap Control C user interrupt */
		signal( SIGQUIT, displayCurrentStatsInterrupt );	/* Trap "Control \" user interrupt to print stats mid-pings like linux */
		signal( SIGTERM, finishOnUserInterrupt );	/* Trap Del user interrupt */
		signal( SIGUSR1, displayCurrentStatsInterrupt );	/* kill -SIGUSR1 <pid> to print stats mid-pings */

		/*
		 * We send one request to kick off and then rely on a timer interrupt to send any subsequent request(s).
		 */
		icmpHdrID = ( u_short ) CONTRIVED_NUMBER;
		/* Set the Time to Live if required */
		if( t_Flag )  {	/* is the TTL Flag active ? */
			if( setIPv4_TimeToLive( sckt, ip4_HeaderTTL_Value, verbosityLevel ) < 0 )
				fprintf( stderr, "Error: Unable to set Time to Live to %d\n", ( int ) ip4_HeaderTTL_Value );
			else if( getIPv4_TimeToLive( sckt, &scktOpt, verbosityLevel ) < 0 )
				printf( "Warning: unable to get current TTL value\n" );
			else if( ip4_HeaderTTL_Value != scktOpt )
				printf( "Warning: IPv4 header Time-To-Live value remains %d\n", scktOpt );
		}
		if( debugFlag )  {
			if( getIPv4_TimeToLive( sckt, &scktOpt, verbosityLevel ) < 0 )  printf( "Warning: unable to get current TTL value\n" );
			else  printf( "Debug: IPv4 header Time-To-Live value is %d\n", scktOpt );
		}
		if( isIPv4_DontFragmentManipulatableOnThisOS_Version())  {
			/* Set or reset the Don't Fragment setting - Linux defaults on, macOS defaults off - so force it on or off */
			if( ensureIPv4_DontFragmentSettingIsTheRequiredValue( sckt, (( D_Flag ) ? 1 : 0 ), verbosityLevel ) < 0 )
				fprintf( stderr, "Error: Unable to set Don't Fragment setting to %s\n", ( D_Flag ) ? "True" : "False" );
			if( debugFlag )  {
				if( getIPv4_DontFragment( sckt, &scktOpt, verbosityLevel ) < 0 )  printf( "Warning: unable to get current Don't Fragment value\n" );
				else  printf( "Debug: IPv4 header Don't Fragment value is %s\n", ( scktOpt == 1 ) ? "True" : "False" );
			}
		}
		if( ensureSocketBroadcastPermissionSettingIsTheRequiredValue( sckt, (( b_Flag ) ? 1 : 0 ), verbosityLevel ) < 0 )
			fprintf( stderr, "Error: Unable to set Broadcast Permission setting to %s\n", ( b_Flag ) ? "True" : "False" );
		if( debugFlag )  {
			if( getSocketBroadcastPermission( sckt, &scktOpt, verbosityLevel ) < 0 )
				printf( "Warning: unable to get current Socket Broadcast permission value\n" );
			else  printf( "Debug: Socket broadcast value is %d\n", scktOpt );
		}
		clock_gettime( CLOCK_REALTIME, &timeOfFirstPing );	/* get current time as the time of the first ping request */
		sendPing( 0 );	/* Send first ping and start interrupt timer for further pings as necessary */
		for( ; pingSendAttemptsLeft >= 0; )
			if( getResponseAndProcessIt( sckt ))  pingsReceived += 1;
	}
	/* Probably will not terminate execution via this code as the interrupt signal code will kill it off */
	if( debugFlag )
		printf( "Debug: ping3 terminating with %s status from program end\n",
			( pingsReceived < pingsSent ) ? "EXIT_FAILURE" : (( pingsSent == 0 ) ? "EXIT_FAILURE" : "EXIT_SUCCESS" ));
 /* report a Failure if no pings were sent or more requests were sent than replies received */
	return( ( pingsReceived < pingsSent ) ? EXIT_FAILURE : (( pingsSent == 0 ) ? EXIT_FAILURE : EXIT_SUCCESS ) );
}

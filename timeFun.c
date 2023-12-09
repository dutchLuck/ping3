/*
 * T I M E F U N . C
 *
 * timeFun.c last edited on Sat Dec  9 16:25:45 2023
 *
 * Functions to handle time
 * 
 */

#include  <stdio.h>
#include  <time.h>	/* struct timespec ctime_r() */
#include  <sys/time.h>	/* struct timeval */
#include  <limits.h>	/* LONG_MAX */


void  convertTimeSpecTimeToTimeValTime( struct timespec *  tsPtr, struct timeval *  tvPtr )  {
	tvPtr->tv_sec = tsPtr->tv_sec;
	tvPtr->tv_usec = tsPtr->tv_nsec / 1000;
}


double  calcTimeSpecClockDifferenceInSeconds( struct timespec *  earlier, struct timespec *  later )  {
	return(( double )( later->tv_sec - earlier->tv_sec ) +
		( double )( later->tv_nsec - earlier->tv_nsec ) * ( double ) 1.0e-9 );
}


void  printTimeValTimeInStructForm( struct timeval *  timeValTimePtr )  {
#ifdef __APPLE__
	printf( "%ld [Sec] %06d [uS]", timeValTimePtr->tv_sec, timeValTimePtr->tv_usec );
#else
	printf( "%ld [Sec] %06ld [uS]", timeValTimePtr->tv_sec, timeValTimePtr->tv_usec );
#endif
}


void  printTimeSpecTimeInStructForm( struct timespec *  timeSpecTime )  {
	printf( "%ld [Sec] %09ld [nS]", timeSpecTime->tv_sec, timeSpecTime->tv_nsec );
}


void  printTimeValTimeInHMS( struct timeval *  timeValTimePtr, int  verbosityLvl )  {
	char *  chrPtr;
	char  dateAndTimeString[ 26 ];

	ctime_r( ( time_t * ) &timeValTimePtr->tv_sec, dateAndTimeString );
	chrPtr = dateAndTimeString + (( verbosityLvl > 4 ) ? 0 : 11 );	/* point to start of HMS or on debug start of string */
	dateAndTimeString[ 19 ] = '\0';		/* terminate string after HMS*/
#ifdef __APPLE__
	printf( "%s.%06d", chrPtr, timeValTimePtr->tv_usec );
#else
	printf( "%s.%06ld", chrPtr, timeValTimePtr->tv_usec );
#endif
}


void  printTimeSpecTimeInHMS( struct timespec *  timeSpecTime, int  verbosityLvl )  {
	char *  chrPtr;
	char  dateAndTimeString[ 26 ];

	ctime_r( ( time_t * ) &timeSpecTime->tv_sec, dateAndTimeString );
	chrPtr = dateAndTimeString + (( verbosityLvl > 4 ) ? 0 : 11 );	/* point to start of HMS or on debug start of string */
	dateAndTimeString[ 19 ] = '\0';		/* terminate string after HMS*/
	printf( "%s.%09ld", chrPtr, timeSpecTime->tv_nsec );
}


/* The max value for the number of millisec since midnight is
 * (24*60*60*1000 - 1) or 86,399,999, which easily fits into a
 * signed or unsigned long. */

void  convertMilliSecondsToHMS_String( long  millisecs, char *  str )  {
	long  hrs, mins, secs, mSecs;
	long  ms;

	if( millisecs == 0L )  sprintf( str, "00:00:00.000" );
	else  {
		if( millisecs > 0L )	/* handle negetive time values */
			ms = millisecs;
		else
			ms = -1L * millisecs;	/* change sign so we work on a positive number */
		mSecs = ms % 1000L;
		hrs = ms / 3600000L;
		mins = ( ms - ( hrs * 3600000L )) / 60000L;
		secs = ( ms - ( hrs * 3600000L + mins * 60000L )) / 1000L;
		if( millisecs > 0L )
			sprintf( str, "%02ld:%02ld:%02ld.%03ld", hrs, mins, secs, mSecs );
		else
			sprintf( str, "-%02ld:%02ld:%02ld.%03ld", hrs, mins, secs, mSecs );
	}
}


void  convertMilliSecondsSinceMidnightToHMS_String( unsigned long  millisecs, char *  str )  {
	if( millisecs == 0L )  sprintf( str, "00:00:00.000" );
	else if( millisecs > ( unsigned long ) LONG_MAX )  sprintf( str, "? Non Std time?" );	/* Non-std if MSB is a 1 */
	else if( millisecs > 86399999L )  sprintf( str, "? Time too big?" );
	else  convertMilliSecondsToHMS_String( millisecs, str );
}


void  printMilliSecondsSinceMidnightInHMS_Format( long  millisecs )  {
	char  hmsStr[ 20 ];

	convertMilliSecondsSinceMidnightToHMS_String( millisecs, hmsStr );
	printf( "%s", hmsStr );
}


int  calcMillisecondsSinceMidnightFromTimeSpec( struct timespec *  tStampInNanoSecs )  {
	return(( int )(( tStampInNanoSecs->tv_sec % (24*60*60)) * 1000 + tStampInNanoSecs->tv_nsec / 1000000 ));
}


void  printClockRealTimeTxRxTimes( struct timespec *  originateTime,  struct timespec *  receiveTime, int  verbosityLvl )  {
#ifdef __APPLE__
	struct timeval  tmpTimeValTime;
#endif

	printf( "ICMP Request send time:  " );
	printTimeSpecTimeInStructForm( originateTime );
	printf( "\nICMP Reply receive time: " );
	printTimeSpecTimeInStructForm( receiveTime );
	printf( "\n" );
#ifdef __APPLE__
	convertTimeSpecTimeToTimeValTime( originateTime, &tmpTimeValTime );
	printf( "ICMP Request send time:  " );
	printTimeValTimeInHMS( &tmpTimeValTime, verbosityLvl );
	printf( "\n" );
	convertTimeSpecTimeToTimeValTime( receiveTime, &tmpTimeValTime );
	printf( "ICMP Reply receive time: " );
	printTimeValTimeInHMS( &tmpTimeValTime, verbosityLvl );
	printf( "\n" );
#else
	printf( "ICMP Request send time:  " );
	printTimeSpecTimeInHMS( originateTime, verbosityLvl );
	printf( "\nICMP Reply receive time: " );
	printTimeSpecTimeInHMS( receiveTime, verbosityLvl );
	printf( "\n" );
#endif
}


void  printClockRealTimeFlightTime( struct timespec *  originateTime,  struct timespec *  receiveTime )  {
	double  flightTimeInSec;

	flightTimeInSec = calcTimeSpecClockDifferenceInSeconds( originateTime, receiveTime );
	if( flightTimeInSec <= 0.5 )  printf( "RTT %lg [mS]", flightTimeInSec * 1000 );
	else   printf( "RTT %lg [Sec]", flightTimeInSec );
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


/* Now get time according to target when reply message left */
/* Assume reply travel time was same speed as request travel time */
/* Thus time here when request was actually processed by receiver */
/* is  tsorig + 0.5 * tsRTT, now subtract that from target time */
void  printSentVsReceiveDeviceClockDifferenceEstimate( long  sentTime, long  recvTime, long  halfRoundTripTime, int  verboseFlag )  {
	long  timeDifference;

	sentTime += halfRoundTripTime;			/* adjust sent time to account for RTT */		
	timeDifference = recvTime - sentTime;	/* difference in millisec */
	printf( "Local device clock is " );
	if( timeDifference < 0 )  {
		displayDeltaTimeInMultipleFormats( -1 * timeDifference, verboseFlag );
		printf(" behind " );
	}
	else  {
		displayDeltaTimeInMultipleFormats( timeDifference, verboseFlag );
		printf(" ahead of " );
	}
	printf( "Remote device clock\n" );
}

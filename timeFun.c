/*
 * T I M E F U N . C
 *
 * timeFun.c last edited on Wed Sep 20 22:40:23 2023
 *
 * Functions to handle time
 * 
 */

#include  <stdio.h>
#include  <time.h>	/* struct timespec */


double  calcTimeSpecClockDifferenceInSeconds( struct timespec *  earlier, struct timespec *  later )  {
	return(( double )( later->tv_sec - earlier->tv_sec ) +
		( double )( later->tv_nsec - earlier->tv_nsec ) * ( double ) 1.0e-9 );
}


void  printTimeSpecTime( struct timespec *  timeSpecTime )  {
	printf( "%ld [Sec] %09ld [nS]", timeSpecTime->tv_sec, timeSpecTime->tv_nsec );
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


void  convertMilliSecondsSinceMidnightToHMS_String( long  millisecs, char *  str )  {
	if( millisecs == 0L )  sprintf( str, "00:00:00.000" );
	else if( millisecs < 0L )  sprintf( str, "? Non Std time" );
	else if( millisecs > 86399999L )  sprintf( str, "? Time too big" );
	else  convertMilliSecondsToHMS_String( millisecs, str );
}


void  printMilliSecondsSinceMidnightInHMS_Format( long  millisecs )  {
	char  hmsStr[ 20 ];

	convertMilliSecondsToHMS_String( millisecs, hmsStr );
	printf( "%s", hmsStr );
}


int  calcMillisecondsSinceMidnightFromTimeSpec( struct timespec *  tStampInNanoSecs )  {
	return(( int )(( tStampInNanoSecs->tv_sec % (24*60*60)) * 1000 + tStampInNanoSecs->tv_nsec / 1000000 ));
}

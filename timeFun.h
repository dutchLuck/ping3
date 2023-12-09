/*
 * T I M E F U N . H
 *
 * timeFun.h last edited on Sat Dec  9 16:25:47 2023
 * 
 * Functions to handle time
 *
 */

#ifndef  TIME_FUN_H
#define  TIME_FUN_H

#include  <time.h>  /* struct timespec */
#include  <sys/time.h>  /* struct timeval */

double  calcTimeSpecClockDifferenceInSeconds( struct timespec *, struct timespec * );
void  printTimeSpecTimeInStructForm( struct timespec * );
void  printTimeSpecTimeInHMS( struct timespec *, int );
void  printTimeValTimeInStructForm( struct timeval * );
void  printTimeValTimeInHMS( struct timeval *, int );
void  convertMilliSecondsToHMS_String( long, char * );
void  convertMilliSecondsSinceMidnightToHMS_String( long, char * );
void  printMilliSecondsSinceMidnightInHMS_Format( long  millisecs );
int  calcMillisecondsSinceMidnightFromTimeSpec( struct timespec *  );
void  printClockRealTimeTxRxTimes( struct timespec *,  struct timespec *, int );
void  printClockRealTimeFlightTime( struct timespec *,  struct timespec * );
void  displayAbsTimeInMultipleFormats( unsigned int, int );
void  displayDeltaTimeInMultipleFormats( int, int );
void  displayTimeAndDeltaTime( unsigned int, int, int );
void  printSentVsReceiveDeviceClockDifferenceEstimate( long, long, long, int );

#endif

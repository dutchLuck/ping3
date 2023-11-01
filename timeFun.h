/*
 * T I M E F U N . H
 *
 * timeFun.h last edited on Tue Oct 31 23:05:05 2023
 * 
 * Functions to handle time
 *
 */

#ifndef  TIME_FUN_H
#define  TIME_FUN_H

#include  <time.h>  /* struct timespec */

double  calcTimeSpecClockDifferenceInSeconds( struct timespec *, struct timespec * );
void  printTimeSpecTime( struct timespec * );
void  convertMilliSecondsToHMS_String( long, char * );
void  convertMilliSecondsSinceMidnightToHMS_String( long, char * );
void  printMilliSecondsSinceMidnightInHMS_Format( long  millisecs );
int  calcMillisecondsSinceMidnightFromTimeSpec( struct timespec *  );
void  printClockRealTimeTxRxTimes( struct timespec *,  struct timespec * );
void  printClockRealTimeFlightTime( struct timespec *,  struct timespec * );
void  displayAbsTimeInMultipleFormats( unsigned int, int );
void  displayDeltaTimeInMultipleFormats( int, int );
void  displayTimeAndDeltaTime( unsigned int, int, int );
void  printSentVsReceiveDeviceClockDifferenceEstimate( long, long, long, int );

#endif

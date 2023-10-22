/*
 * T I M E F U N . H
 *
 * timeFun.h last edited on Sun Oct 22 22:30:51 2023
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

#endif

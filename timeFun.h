/*
 * T I M E F U N . H
 *
 * timeFun.h last edited on Wed Sep 20 22:40:27 2023
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

#endif

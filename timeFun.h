/*
 * T I M E F U N . H
 *
 * timeFun.h last edited on Fri Sep  1 16:08:02 2023
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
int  calcMillisecondsSinceMidnightFromTimeSpec( struct timespec *  );

#endif

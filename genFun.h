/*
 * G E N F U N . H
 *
 * genFun.h last edited Tue Sep 19 23:36:33 2023
 * 
 */

#ifndef  GEN_FUN_H
#define  GEN_FUN_H

void  clearByteArray( unsigned char *, int );
void  printByteArray( unsigned char *, int, int );
long  limitLongValueToEqualOrMoreNegetiveThanBoundary( long, long );
long  limitLongValueToEqualOrMorePositiveThanBoundary( long, long );
unsigned long  limitUnsignedLongValueToEqualOrWithinRange( unsigned long, unsigned long, unsigned long );
long  limitLongValueToEqualOrWithinRange( long, long, long );
unsigned int  limitUnsignedIntegerValueToEqualOrWithinRange( unsigned int, unsigned int, unsigned int );
int  limitIntegerValueToEqualOrWithinRange( int, int, int );
unsigned char  limitUnsignedCharValueToEqualOrWithinRange( unsigned char, unsigned char, unsigned char );
char  limitCharValueToEqualOrWithinRange( char, char, char );
long  convertOptionStringToLong( long, char *, char *, int * );
int  convertOptionStringToInteger( int, char *, char *, int * );

#endif

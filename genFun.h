/*
 * G E N F U N . H
 *
 * genFun.h last edited Sat Dec  2 21:30:27 2023
 * 
 */

#ifndef  GEN_FUN_H
#define  GEN_FUN_H

void  clearByteArray( unsigned char *, int );
void  fillByteArray( unsigned char,  unsigned char *, int );
void  fillByteArrayWithIncByOne( unsigned char,  unsigned char *, int );
void  fillFirstByteArrayByReplicatingSecondByteArray( unsigned char *, int, unsigned char *, int );
void  fillByteArrayWithPseudoRandomData( unsigned char *, int );
void  printByteArray( unsigned char *, int, int );
double  limitDoubleValueToEqualOrMoreNegetiveThanBoundary( double, double );
double  limitDoubleValueToEqualOrMorePositiveThanBoundary( double, double );
double  limitDoubleValueToEqualOrWithinRange( double, double, double );
long  limitLongValueToEqualOrMoreNegetiveThanBoundary( long, long );
long  limitLongValueToEqualOrMorePositiveThanBoundary( long, long );
unsigned long  limitUnsignedLongValueToEqualOrWithinRange( unsigned long, unsigned long, unsigned long );
long  limitLongValueToEqualOrWithinRange( long, long, long );
unsigned int  limitUnsignedIntegerValueToEqualOrWithinRange( unsigned int, unsigned int, unsigned int );
int  limitIntegerValueToEqualOrWithinRange( int, int, int );
unsigned char  limitUnsignedCharValueToEqualOrWithinRange( unsigned char, unsigned char, unsigned char );
char  limitCharValueToEqualOrWithinRange( char, char, char );
long  convertOptionStringToLong( long, char *, char *, int *, int );
int  convertOptionStringToInteger( int, char *, char *, int *, int );
double  convertOptionStringToDouble( double, char *, char *, int *, int );
int  convertHexByteStringToByteArray( char *, unsigned char *, int );

#endif

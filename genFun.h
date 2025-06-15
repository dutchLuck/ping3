/*
 * G E N F U N . H
 *
 * genFun.h last edited Wed Jul 31 13:21:53 2024
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
int  convertHexChrNibbleToInt( char * );
int  convert2HexChrNibblesToInt( char * );
int  convert4HexChrNibblesToInt( char * );
int ringBffrFull( int, int );
int ringBffrEmpty( int, int, int );
void  dumpRingBffr( unsigned char [], size_t );
void  zeroRingBffr( unsigned char [], size_t );
int  ringBffrUsed( int, int, int );
int  loadRingBffrWithBytesFromLineOfAsciiHex( char *, int, unsigned char [], int *, int *, int );

#ifndef  FALSE
#define  FALSE  ((int)0)
#endif
#ifndef  TRUE
#define  TRUE  (! FALSE)
#endif

#endif

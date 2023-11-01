/*
 * I P O P T I O N S F U N . H
 *
 * ipOptionsFun.h last edited Tue Oct 31 23:05:33 2023
 *
 * Function headers for IP header options in the IP packet.
 *
 */

#ifndef  IPOPTIONS_FUN_H
#define  IPOPTIONS_FUN_H


void  displayIpOptionTimeStamps( unsigned char *, int, int, int );
void  displayTimeStampOptionInHex( unsigned char *, int );
void  displayIpOptionList( unsigned char *, int );
void  displayIpOptions( unsigned char *, int, int );
void  displayTimeStampOverflowCount( unsigned char *  ptr );
void  displayOnlyGreaterThanZeroTimeStampOverflowCount( unsigned char *  ptr );
void  displayIpOptionRecordRoute( unsigned char *, int, int );

#endif

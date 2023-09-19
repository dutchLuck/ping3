/*
 * I P O P T I O N S F U N . H
 *
 * ipOptionsFun.h last edited Mon Sep 18 20:29:15 2023
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
void  displayAbsTimeInMultipleFormats( unsigned int, int );
void  displayDeltaTimeInMultipleFormats( int, int );
void  displayTimeAndDeltaTime( unsigned int, int, int );
void  displayTimeStampOverflowCount( unsigned char *  ptr );
void  displayOnlyGreaterThanZeroTimeStampOverflowCount( unsigned char *  ptr );

#endif

/*
 * I P F U N . C
 *
 * ipFun.h last edited Wed Dec 13 22:10:04 2023
 *
 * header for ipFun.c functions
 *
 */

#ifndef  IP_FUN_H
#define  IP_FUN_H


#include  <sys/types.h>
#include  <sys/socket.h>
#include  <netinet/in_systm.h>
#include  <netinet/in.h>
#include  <arpa/inet.h>
#include  <netinet/ip.h>


/*
 * Definitions for IP precedence (also in ip_tos) (hopefully unused)
 */

#ifdef __sunos__
/*
 * These defns appear to be missing from the SunOS 4.x include file
 *  /usr/include/netinet/ip.h
 */
#define	IPTOS_PREC_NETCONTROL		0xe0
#define	IPTOS_PREC_INTERNETCONTROL	0xc0
#define	IPTOS_PREC_CRITIC_ECP		0xa0
#define	IPTOS_PREC_FLASHOVERRIDE	0x80
#define	IPTOS_PREC_FLASH		0x60
#define	IPTOS_PREC_IMMEDIATE		0x40
#define	IPTOS_PREC_PRIORITY		0x20
#define	IPTOS_PREC_ROUTINE		0x00

#define  IPTOS_LOWDELAY     0x10
#define  IPTOS_THROUGHPUT   0x08
#define  IPTOS_RELIABILITY  0x04
#endif

u_short  calcCheckSum( u_short *, int );
void  printIPv4_AddressAsDottedQuad( struct in_addr * );
void  display_ip( struct ip  *, int );
void  printPingCommonInfo( struct ip  *, int );
void  printIPv4_TimeToLiveInfo( struct ip  * );
void  displayIpProtocol( unsigned char );
int  setIPv4_TimeToLive( int,  int, int );
int  getIPv4_TimeToLive( int, int *, int );
int  setIPv4_DontFragment( int,  int, int );
int  getIPv4_DontFragment( int, int *, int );
int  isIPv4_DontFragmentManipulatableOnThisOS_Version( void );
int  ensureIPv4_DontFragmentSettingIsTheRequiredValue( int,  int, int );
int  getSocketBroadcastPermission( int, int *, int );
int  setSocketBroadcastPermission( int,  int, int );
int  ensureSocketBroadcastPermissionSettingIsTheRequiredValue( int,  int, int );

#endif

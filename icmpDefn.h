/*
 * I C M P D E F N . H
 *
 * icmpDefn.h last edited Thu Apr  4 22:38:17 2024
 *
 */

#include <netinet/in_systm.h> /* n_long */

#define   ICMP_MINLEN 8

/* RFC 792 details all ICMP except Mask */
#define		ICMP_ECHOREPLY  0
#define   ICMP_UNREACH  3
#define   ICMP_SOURCEQUENCH 4
#define   ICMP_REDIRECT 5
#define		ICMP_ECHO 8
#define   ICMP_TIMXCEED 11
#define   ICMP_PARAMPROB  12
#define		ICMP_TSTAMP 13
#define		ICMP_TSTAMPREPLY  14
#define		ICMP_IREQ 15
#define		ICMP_IREQREPLY  16
#define		ICMP_MASKREQ  17
#define		ICMP_MASKREPLY  18

/*

ICMP_PARAMPROB_ERRATPTR
ICMP_PARAMPROB_LENGTH

*/

/* Definitions for ICMP Destination Unreachable */
#define		ICMP_UNREACH_NET	0		    /* bad net */
#define		ICMP_UNREACH_HOST	1		    /* bad host */
#define		ICMP_UNREACH_PROTOCOL	2		/* bad protocol */
#define		ICMP_UNREACH_PORT	3		    /* bad port */
#define		ICMP_UNREACH_NEEDFRAG	4		/* IP_DF caused drop */
#define		ICMP_UNREACH_SRCFAIL	5		/* src route failed */
#define		ICMP_UNREACH_NET_UNKNOWN 6		/* unknown net */
#define		ICMP_UNREACH_HOST_UNKNOWN 7		/* unknown host */
#define		ICMP_UNREACH_ISOLATED	8		    /* src host isolated */
#define		ICMP_UNREACH_NET_PROHIB	9		  /* prohibited access */
#define		ICMP_UNREACH_HOST_PROHIB 10		/* ditto */
#define		ICMP_UNREACH_TOSNET	11		  /* bad tos for net */
#define		ICMP_UNREACH_TOSHOST	12		/* bad tos for host */
#define		ICMP_UNREACH_FILTER_PROHIB  13
#define		ICMP_UNREACH_HOST_PRECEDENCE  14
#define		ICMP_UNREACH_PRECEDENCE_CUTOFF  15

/* Definitions for ICMP Redirect */
#define		ICMP_REDIRECT_NET	0       /* for network */
#define		ICMP_REDIRECT_HOST	1     /* for host */
#define		ICMP_REDIRECT_TOSNET	2		/* for tos and net */
#define		ICMP_REDIRECT_TOSHOST	3		/* for tos and host */

/* Definitions for ICMP Time To Live Exceeded */
#define		ICMP_TIMXCEED_INTRANS	0		/* ttl==0 in transit */
#define		ICMP_TIMXCEED_REASS	1		  /* ttl==0 in reass */

/* Definitions for Parameter Problem */
#define		ICMP_PARAMPROB_OPTABSENT 1		/* req. opt. absent */

#define   ICMP_ROUTERADVERT 9
#define   ICMP_ROUTERSOLICIT  10


struct icmp  {
  unsigned char  icmp_type;
  unsigned char  icmp_code;
  n_short  icmp_cksum;
  n_short  icmp_id;
  n_short  icmp_seq;   /* Sequence number for ICMP Echo */
  union  {
    struct  {
      n_long  its_otime;
      n_long  its_rtime;
      n_long  its_ttime;
    } id_ts;
    n_long  id_mask;
    n_long  id_ip;    /* Parameter problem */
     unsigned char id_data[ 1 ];
  }  icmp_dun;
};

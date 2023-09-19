/*
 * I P O P T T S . H
 *
 * ipOptTS.h last edited Mon Sep 18 20:32:20 2023
 *
 */

#ifndef  IPOPTTS_H
#define  IPOPTTS_H

#include <netinet/in_systm.h>	/* n_long */

/* Get around Red Hat (at least?) linux ip.h defn bug */
#ifdef __linux__
struct	ipOptTimestamp {
	u_char	ipt_code;		/* IPOPT_TS */
	u_char	ipt_len;		/* size of structure (variable) */
	u_char	ipt_ptr;		/* index of current entry */
	u_char	ipt_flg:4,		/* little endian flags, see below */
			ipt_oflw:4;		/* little endian overflow counter */
	union ipt_timestamp {
		n_long	ipt_time[1];
		struct	ipt_ta {
			struct in_addr ipt_addr;
			n_long ipt_time;
		} ipt_ta[1];
	} ipt_timestamp;
};
#endif

#endif
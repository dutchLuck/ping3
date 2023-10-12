# ping3
Send multiple Internet Protocol version 4 (IPv4) ICMP Echo
requests or multiple IPv4 ICMP Time Stamp requests or multiple
IPv4 ICMP Address Mask requests to a remote network device and
display round trip time information if the corresponding ICMP
replies are received back.

This utility program is named "ping3" because it has the ability
to ping a remote network device with three types of ICMP requests
in similar fashion to the macOS system utility named "ping". These
three ICMP requests are Echo or Timestamp or Mask. "ping3" also
defaults to sending three ICMP echo/timestamp/mask request to a
network device and waits for the device to reply to each request.
If the device replies then some information on how long the round-
trip-time (RTT) was is printed out. If the device doesn't respond
then a time-out message is printed instead and ping3 exits.

By default "ping3" provides a (very much) cut down version of the
capability provided by the standard computer system utility "ping".
It does not provide any Internet Protocol version 6 (IPv6) capability.
It currently does not provide any summary statistics or congestion
testing or pattern testing or alternate interface specification or
multicast functionality.

The standard ping utility that comes with Apple Mac OSX, such as
macOS Ventura, provides the same default ability as the Linux ping
utility, but differs considerably for more rarely used modes of
operation. Linux ping provides a timestamp and record route
capability through IPv4 header options and macOS ping doesn't.
However macOS ping provides a timestamp capability through ICMP
timestamp requests. This mode requires elevated priveleges via sudo,
unless the "-s 0" option is included with the "-M time" option. The
macOS ping also provides a netmask capability through ICMP netmask
requests, but Linux ping has no similar capability. Once again
the "-M mask" option on macOS ping also should be teamed with
a "-s 0" to avoid requiring sudo priveleges to send the request.

The ping3 utility provides timestamp in the header options on a
macOS system and provides ICMP timestamp/mask pings on a Linux system.
On Linux ping3 requires the use of sudo to work, but it does not
require increased privilege via sudo to work on macOS.

The default output of ping3 on a macOS system is somewhat similar
to the ping utility output minus the summary statistics.
Both outputs follow; -
```
% ./ping3 www.apple.com
80 bytes from 23.202.170.41: seq 1, ttl 59, RTT 25.401 [mS]
80 bytes from 23.202.170.41: seq 2, ttl 59, RTT 27.296 [mS]
80 bytes from 23.202.170.41: seq 3, ttl 59, RTT 26.753 [mS] 
%
% ping -c 3 www.apple.com
PING e6858.dscx.akamaiedge.net (23.202.170.41): 56 data bytes
64 bytes from 23.202.170.41: icmp_seq=0 ttl=59 time=24.187 ms
64 bytes from 23.202.170.41: icmp_seq=1 ttl=59 time=26.968 ms
64 bytes from 23.202.170.41: icmp_seq=2 ttl=59 time=25.258 ms

--- e6858.dscx.akamaiedge.net ping statistics ---
3 packets transmitted, 3 packets received, 0.0% packet loss
round-trip min/avg/max/stddev = 24.187/25.471/26.968/1.145 ms
%
```
There doesn't appear to be equivalent options on the macOS ping
that give "tsonly" timestamps in the following fashion; -
```
% ./ping3 -T tsonly -c2 www.apple.com
60 bytes from 23.202.170.41: seq 0, ttl 59, RTT 21.581 [mS]
100 bytes from 23.202.170.41: seq 1, ttl 59, RTT 23.66 [mS]
 12:50:09.212 ( -103 [mS]))
 12:50:10.239 ( 107 [mS]))
 12:50:10.238 ( -1 [mS]))
 12:50:10.238 ( 0 [mS]))
 12:50:10.239  ( 1 [mS]))
 12:50:09.234  ( -105 [mS]))
 12:50:10.239 ( 105 [mS]))
%
```
Linux ping can output IPv4 header option timestamps in the
following way; -
```
$ ping -4 -n -c 1 -T tsonly www.apple.com
PING  (23.202.170.41) 56(124) bytes of data.
64 bytes from 23.202.170.41: icmp_seq=1 ttl=59 time=76.7 ms
TS: 	46977144 absolute
	-245
	340
	6
	0
	1
	-271
	246


---  ping statistics ---
1 packets transmitted, 1 received, 0% packet loss, time 0ms
rtt min/avg/max/mdev = 76.737/76.737/76.737/0.000 ms
$
```
Successful ICMP Timestamp request pings produce the following output; -
```
% ./ping3 -M time ntp-m.obspm.fr       
40 bytes from 145.238.187.55: seq 1, ttl 38, RTT 0.546179 [Sec] tso 05:19:35.508 tsr 05:19:35.836 tst 05:19:35.836
40 bytes from 145.238.187.55: seq 2, ttl 38, RTT 414.445 [mS] tso 05:19:36.559 tsr 05:19:36.769 tst 05:19:36.769
40 bytes from 145.238.187.55: seq 3, ttl 38, RTT 421.83 [mS] tso 05:19:37.475 tsr 05:19:37.689 tst 05:19:37.689
%
```
The comparison with macOS ping is; -
```
% ping -M time -s 0 -c 3 ntp-m.obspm.fr
ICMP_TSTAMP
PING korriban.obspm.fr (145.238.187.55): 0 data bytes
20 bytes from 145.238.187.55: icmp_seq=0 ttl=38 tso=05:19:53 tsr=05:19:53 tst=05:19:53
20 bytes from 145.238.187.55: icmp_seq=1 ttl=38 tso=05:19:54 tsr=05:19:54 tst=05:19:54
20 bytes from 145.238.187.55: icmp_seq=2 ttl=38 tso=05:19:55 tsr=05:19:55 tst=05:19:55

--- korriban.obspm.fr ping statistics ---
3 packets transmitted, 3 packets received, 0.0% packet loss
%
```
Successful ICMP Netmask request pings produce the following output; -
```
% ./ping3 -M mask 192.168.1.103
32 bytes from 192.168.1.103: seq 1, ttl 64, RTT 3.375 [mS] mask 255.255.255.0
32 bytes from 192.168.1.103: seq 2, ttl 64, RTT 5.805 [mS] mask 255.255.255.0
32 bytes from 192.168.1.103: seq 3, ttl 64, RTT 4.521 [mS] mask 255.255.255.0
%
```
The comparison with macOS ping is; -
```
% ping -M mask -s 0 -c 3 192.168.1.103
ICMP_MASKREQ
PING 192.168.1.103 (192.168.1.103): 0 data bytes
12 bytes from 192.168.1.103: icmp_seq=0 ttl=64 mask=255.255.255.0
12 bytes from 192.168.1.103: icmp_seq=1 ttl=64 mask=255.255.255.0
12 bytes from 192.168.1.103: icmp_seq=2 ttl=64 mask=255.255.255.0

--- 192.168.1.103 ping statistics ---
3 packets transmitted, 3 packets received, 0.0% packet loss
%
```
The help / usage information for ping3 can be seen by using the "-h"
command line option as follows; -
```
% ./ping3 -h

useage: ping3 [-cX][-D][-h][-lXX][-M ABC][-q][-T ABC][-v][-wX] NetworkDeviceName
or      ping3 [-cX][-D][-h][-lXX][-M ABC][-q][-T ABC][-v][-wX] NetworkDeviceIP_Number

where options; -
        -cX  specifies number of times to ping remote network device
        -D  switches on debug output
        -h  switches on this help output and then terminates ping3
        -lXX  specifies header option length (max accepted is 40 and should be a multiple of 4)
        -M ABC  specifies ping with ICMP Mask/Timestamp request instead of ICMP Echo.
          where ABC is a sting of characters.
            If "mask" then send ICMP Mask request,
            if "time" then send ICMP Time Stamp request,
            if "timew" as above, but treat tsr and tst timestamps as little endian.
            (i.e. Windows default response, if the ICMP Time Stamp request
            is allowed through the Windows firewall. )
        -q  forces quiet (minimum) output and overrides -v
        -T ABC  specifies header option time stamp type.
          where ABC is a sting of characters.
            If "tsonly" then record Time Stamp Only list of time stamps,
            if "tsandaddr" then record Address and Time Stamp pair list,
            if "tsprespec H.I.J.K [ L.M.N.O [ P.Q.R.S [ T.U.V.W ]]]" then Time Stamp prespecified Addresses.
        -v  switches on verbose output
        -wX  ensures the program waits for X seconds for a response

%
```
ping3 is released under the MIT license and must be used at your own risk.
However, unless it provides some specific capability you need then it
is probably better to use your operating systems standard ping utility.

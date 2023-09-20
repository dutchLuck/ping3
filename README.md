# ping3
Send multiple Internet Protocol version 4 ICMP echo requests to a
remote network device and display round trip time information if
echo replies are received back.

This utility program is named "ping3" because it sends an ICMP
echo request to a network device and waits for the device to reply.
If the device replies then some information on how long the round-
trip-time (RTT) was is printed out. If the device doesn't respond
then a time-out message is printed instead.
By default "ping3" provides a (somewhat) cut down version of the
capability provided by the standard computer system utility "ping".
The standard ping utility that comes with Apple Mac OSX, such as
macOS Ventura, provides the same default ability as the Linux ping
utility, but differs considerably for more rarely used modes of
operation. Linux ping provides a timestamp and record route
capability through header options and macOS ping doesn't. However
macOS ping provides a timestamp capability through ICMP timestamp
request, although this mode requires elevated priveleges via sudo. 
The ping3 utility provides timestamp in the header options on a
macOS system.

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
that give timestamps in the following fashion; -
```
% ./ping3 www.apple.com -t0 -c2
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
Linux ping can output timestamps in the following way; -
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
The help / usage information for ping3 can be seen by using the "-h"
command line option as follows; -
```
% ./ping3 -h

useage: ping3 NetworkDeviceName [-cX][-D][-h][-lXX][-q][-tX][-v][-wX]
or      ping3 NetworkDeviceIP_Number [-cX][-D][-h][-lXX][-q][-tX][-v][-wX]

where options; -
        -cX  specifies number of times to ping remote network device
        -D  switches on debug output
        -h  switches on this help output and then terminates ping3
        -lXX  specifies header option length (default is 40)
        -q  forces quiet (minimum) output and overrides -v
        -tX  specifies header option time stamp type (default is none)
        -v  switches on verbose output
        -wX  ensures the program waits for X seconds for a response

%
```
ping3 is released under the MIT license and must be used at your own risk.
However, unless it provides some specific capability you need then it
is probably better to use your operating systems standard ping utility.

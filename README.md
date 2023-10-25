# ping3
Send multiple Internet Protocol version 4 (IPv4) Internet
Control Message Protocol (ICMP) Echo requests or multiple
IPv4 ICMP Time Stamp requests or multiple IPv4 ICMP Address
Mask requests to a remote network device and display round
trip time (RTT) information if the corresponding ICMP
replies are received back.

This utility program is named "ping3" because it has the ability
to ping a remote network device with three types of ICMP requests
in similar fashion to the macOS system utility named "ping". These
three ICMP requests are Echo or Timestamp or Mask. "ping3" also
defaults to sending three ICMP echo/timestamp/mask request to a
network device and waits for the device to reply to each request.
If the device replies then some information on how long the round-
trip-time (RTT) was is printed out.

By default "ping3" provides a (very much) cut down version of the
capability provided by the standard computer system utility "ping".
It does not provide any Internet Protocol version 6 (IPv6) capability.
It currently provide only very limited summary statistics and doesn't
provide congestion testing or pattern testing or alternate interface
specification or multicast functionality.

The standard ping utility that comes with Apple Mac OSX, such as
macOS Sonoma, provides similar default capability to the Linux ping
utility, but both differ considerably for more rarely used modes of
operation. Linux ping provides a timestamp (-T) and record route
(-R) capability through IPv4 header options and macOS ping doesn't.
However macOS ping provides a timestamp capability through ICMP
timestamp requests (-M time) and a netmask capability through
ICMP mask requests (-M mask). Both modes require elevated priveleges
via sudo, unless the "-s 0" option is included with the "-M time"
or "-M mask" option. Linux ping has no similar capability.

The ping3 utility provides timestamp or record route in the IPv4
header options on a macOS system and provides ICMP timestamp or
ICMP mask pings on a Linux system. On Linux ping3 requires the
increased privelege via sudo to work, but it does not require
sudo to work on macOS.

The default output of ping3 on a macOS system is somewhat similar
to the ping utility output, but with very simple summary statistics.
Both outputs follow; -
```
% ./ping3 www.apple.com
80 bytes from 23.219.60.201: seq 1, ttl 59, RTT 21.114 [mS]
80 bytes from 23.219.60.201: seq 2, ttl 59, RTT 23.939 [mS]
80 bytes from 23.219.60.201: seq 3, ttl 59, RTT 21.598 [mS]
3 requests sent, 3 replies received, 0.0% loss in 2.01 [S]
%
% % ping -c3 www.apple.com
PING e6858.dscx.akamaiedge.net (23.219.60.201): 56 data bytes
64 bytes from 23.219.60.201: icmp_seq=0 ttl=59 time=21.132 ms
64 bytes from 23.219.60.201: icmp_seq=1 ttl=59 time=21.424 ms
64 bytes from 23.219.60.201: icmp_seq=2 ttl=59 time=20.837 ms

--- e6858.dscx.akamaiedge.net ping statistics ---
3 packets transmitted, 3 packets received, 0.0% packet loss
round-trip min/avg/max/stddev = 20.837/21.131/21.424/0.240 ms
%
```
There doesn't appear to be equivalent options on the macOS ping
that give "tsonly" (time stamp only) timestamps in the following fashion; -
```
% ./ping3 -T tsonly -c2 www.apple.com
80 bytes from 23.219.60.201: seq 1, ttl 59, RTT 21.201 [mS]
120 bytes from 23.219.60.201: seq 2, ttl 59, RTT 26.458 [mS]
 11:46:08.572 ( -646 [mS])
 11:46:09.263 ( 691 [mS])
 11:46:09.264 ( 1 [mS])
 11:46:09.264 ( 0 [mS])
 11:46:09.264 ( 0 [mS])
 11:46:08.597 ( -667 [mS])
 11:46:09.244 ( 647 [mS])
2 requests sent, 2 replies received, 0.0% loss in 1.51 [S]
%
```
Linux ping can output IPv4 header option timestamps in the
following way; -
```
$ ping -4 -n -c 1 -T tsonly www.apple.com
PING  (23.219.60.201) 56(124) bytes of data.
64 bytes from 23.219.60.201: icmp_seq=1 ttl=59 time=24.0 ms
TS: 	42496797 absolute
	-671
	692
	0
	0
	0
	-669
	672


---  ping statistics ---
1 packets transmitted, 1 received, 0% packet loss, time 0ms
rtt min/avg/max/mdev = 24.044/24.044/24.044/0.000 ms
$
```
Successful ICMP Timestamp request pings produce the following output; -
```
% ./ping3 -M time ntp-m.obspm.fr
40 bytes from 145.238.187.55: seq 1, ttl 37, RTT 480.594 [mS] tso 11:57:44.009 tsr 11:57:44.296 tst 11:57:44.296
40 bytes from 145.238.187.55: seq 2, ttl 37, RTT 383.802 [mS] tso 11:57:44.514 tsr 11:57:44.716 tst 11:57:44.716
40 bytes from 145.238.187.55: seq 3, ttl 37, RTT 494.956 [mS] tso 11:57:45.019 tsr 11:57:45.283 tst 11:57:45.283
3 requests sent, 3 replies received, 0.0% loss in 2.01 [S]
%
```
The comparison with macOS ping is; -
```
% ping -M time -s 0 -c 3 ntp-m.obspm.fr
ICMP_TSTAMP
PING korriban.obspm.fr (145.238.187.55): 0 data bytes
20 bytes from 145.238.187.55: icmp_seq=0 ttl=37 tso=11:58:55 tsr=11:58:55 tst=11:58:55
20 bytes from 145.238.187.55: icmp_seq=1 ttl=37 tso=11:58:56 tsr=11:58:56 tst=11:58:56
20 bytes from 145.238.187.55: icmp_seq=2 ttl=37 tso=11:58:57 tsr=11:58:57 tst=11:58:57

--- korriban.obspm.fr ping statistics ---
3 packets transmitted, 3 packets received, 0.0% packet loss
%
```
Successful ICMP Netmask request pings produce the following output; -
```
% ./ping3 -M mask 192.168.1.103
32 bytes from 192.168.1.103: seq 1, ttl 64, RTT 68.23 [mS] mask 255.255.255.0
32 bytes from 192.168.1.103: seq 2, ttl 64, RTT 78.252 [mS] mask 255.255.255.0
32 bytes from 192.168.1.103: seq 3, ttl 64, RTT 87.433 [mS] mask 255.255.255.0
3 requests sent, 3 replies received, 0.0% loss in 2.01 [S]
%
```
The comparison with macOS ping is; -
```
% ping -Mm -s0 -c3 192.168.1.103 
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

useage: ping3 [-cX][-D][-h][-lXX][-M ABC][-q][-R][-sXX][-tXX][-T ABC][-v][-wX] NetworkDeviceName
or      ping3 [-cX][-D][-h][-lXX][-M ABC][-q][-R][-sXX][-tXX][-T ABC][-v][-wX] NetworkDeviceIP_Number

where options; -
        -cX  specifies number of times to ping remote network device
        -D  switches on debug output
        -h  switches on this help output and then terminates ping3
        -lXX  suggest desired IP header option length (max is 40 and should be a multiple of 4)
        -M ABC  specifies ping with ICMP Mask/Timestamp request instead of ICMP Echo.
          where ABC is a sting of characters.
            If "mask" then send ICMP Mask request,
            if "time" then send ICMP Time Stamp request,
            if "timew" as above, but treat tsr and tst timestamps as little endian.
            (i.e. Windows default response, if the ICMP Time Stamp request
            is allowed through the Windows firewall. )
        -q  forces quiet (minimum) output and overrides -v
        -R  specifies header option Record Route (N.B. -T overrides -R when both are specified)
        -sXX  specifies ICMP Echo data section size (N.B. 16 <= XX <= 1472 works best on macOS)
        -tXX  specifies IPv4 header Time to Live (N.B. doesn't work on macOS)
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
Therefore unless it provides some specific capability you need then it is
almost certainly better to use your operating systems standard ping utility.

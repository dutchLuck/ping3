# ping3
The "ping3" utility program can be used to send multiple
Internet Protocol version 4 (IPv4) Internet Control Message
Protocol (ICMP) Echo requests or multiple IPv4 ICMP Time Stamp
requests or multiple IPv4 ICMP Address Mask requests to a
remote network device and display information about the round
trip time (RTT) if corresponding ICMP replies are received back.

This utility program is named "ping3" because it has the ability
to ping a network device with any one of three types of ICMP
requests in similar fashion to the macOS system utility named
"ping". The three possible ICMP requests are Echo, Timestamp and
Mask. By default Echo requests are sent to the target network
device. "ping3" also defaults to sending three requests to a
network device and waits for the device to reply to each request.
If the device replies then ping3 outputs information on the time
taken between the request being sent and the reply being received.
This time is known as the round-trip-time (RTT).

The "ping3" utility provides a cut down version of the standard
"ping" utility that is included in all major computer operating
systems. Unlike ping it does not provide any Internet Protocol
version 6 (IPv6) capability. It doesn't provide flood or preload
options for congestion testing, alternate interface specification,
alternate routing options or any specific broadcast or multicast
functionality.

The standard ping utility that comes with Apple Mac OSX, such as
macOS Sonoma, provides similar default capability to the Linux ping
utility, but both differ considerably for more rarely used modes of
operation. Linux ping provides a timestamp (-T) and record route
(-R) capability through IPv4 header options and macOS ping doesn't.
However macOS ping provides a timestamp capability through ICMP
timestamp requests (-M time) and a netmask capability through
ICMP mask requests (-M mask). Both modes require elevated priveleges
via sudo, unless the "-s 0" option is included with the "-M time"
or "-M mask" option. Linux ping has no ICMP timestamp or ICMP netmask
capability.

The ping3 utility provides timestamp or record route in the IPv4
header options on a macOS system and provides ICMP timestamp or
ICMP mask pings on a Linux system. On Linux ping3 requires the
increased privelege via sudo to work, but it does not require
sudo to work on macOS.

The default output of ping3 on a macOS system is somewhat similar
to the ping utility output, but with very simple summary statistics.
The ping3 length value at the start of each reply line is the total
size of the IPv4 (ICMP) datagram, not the size of the ICMP message
as is shown in the output lines from ping.
Both outputs follow; -
```
% ./ping3 www.apple.com
80 bytes from 23.48.156.212: seq 1, ttl 57, RTT 25.608 [mS]
80 bytes from 23.48.156.212: seq 2, ttl 57, RTT 25.56 [mS]
80 bytes from 23.48.156.212: seq 3, ttl 57, RTT 25.624 [mS]
3 requests sent, 3 replies received, 0.0% loss in 4.01 [S]
RTT Min 25.560, Mean 25.597, Max 25.624 [mS]
% 
% ping -c3 www.apple.com
PING e6858.dscx.akamaiedge.net (23.48.156.212): 56 data bytes
64 bytes from 23.48.156.212: icmp_seq=0 ttl=57 time=25.947 ms
64 bytes from 23.48.156.212: icmp_seq=1 ttl=57 time=77.821 ms
64 bytes from 23.48.156.212: icmp_seq=2 ttl=57 time=27.012 ms

--- e6858.dscx.akamaiedge.net ping statistics ---
3 packets transmitted, 3 packets received, 0.0% packet loss
round-trip min/avg/max/stddev = 25.947/43.593/77.821/24.207 ms
%
```
There doesn't appear to be equivalent options on the macOS ping
that give "tsonly" (time stamp only) timestamps in the following fashion; -
```
% ./ping3 -c1 -T tsonly www.apple.com
120 bytes from 23.219.60.201: seq 1, ttl 59, RTT 26.906 [mS]
 10:54:17.716 ( -745 [mS])
 10:54:18.482 ( 766 [mS])
 10:54:18.483 ( 1 [mS])
 10:54:18.483 ( 0 [mS])
 10:54:18.482 ( -1 [mS])
 10:54:17.738 ( -744 [mS])
 10:54:18.488 ( 750 [mS])
1 requests sent, 1 replies received, 0.0% loss in 2.01 [S]
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
% ./ping3 -M time 192.168.1.1 
40 bytes from 192.168.1.1: seq 1, ttl 64, RTT 5.467 [mS] tso 10:56:33.537 tsr 10:56:32.793 tst 10:56:32.793
40 bytes from 192.168.1.1: seq 2, ttl 64, RTT 5.447 [mS] tso 10:56:34.542 tsr 10:56:33.799 tst 10:56:33.799
40 bytes from 192.168.1.1: seq 3, ttl 64, RTT 3.908 [mS] tso 10:56:35.547 tsr 10:56:34.802 tst 10:56:34.802
3 requests sent, 3 replies received, 0.0% loss in 4.02 [S]
%
```
The output from macOS ICMP Timestamp request ping for
comparison purposes is (N.B. macOS prints integer Hrs:Min:Sec
since midnight GMT, but the reply has milliseconds since midnight
resolution ); -
```
% ping -c3 -Mt -s0 192.168.1.1
ICMP_TSTAMP
PING 192.168.1.1 (192.168.1.1): 0 data bytes
20 bytes from 192.168.1.1: icmp_seq=0 ttl=64 tso=10:58:21 tsr=10:58:20 tst=10:58:20
20 bytes from 192.168.1.1: icmp_seq=1 ttl=64 tso=10:58:22 tsr=10:58:21 tst=10:58:21
20 bytes from 192.168.1.1: icmp_seq=2 ttl=64 tso=10:58:23 tsr=10:58:22 tst=10:58:22

--- 192.168.1.1 ping statistics ---
3 packets transmitted, 3 packets received, 0.0% packet loss
%
```
The "timew" variant of -M enables special handling of ICMP timestamp replies from
Microsoft Windows devices. For example, see below for the comparison of ping3 and
normal macOS ping. The "tsr" receive timestamp and the "tst" transmit timestamp
make no sense as output by normal ping when the target is a Windows device
(192.168.1.121 is a Win10 machine with its firewall modified to allow ICMP timestamp
requests. Normally Windows devices do not respond to ICMP timestamp requests.)
```
% ./ping3 -M timew 192.168.1.121
40 bytes from 192.168.1.121: seq 1, ttl 128, RTT 35.471 [mS] tso 01:55:48.637 tsr 01:55:48.867 tst 01:55:48.867
40 bytes from 192.168.1.121: seq 2, ttl 128, RTT 2.479 [mS] tso 01:55:49.638 tsr 01:55:49.836 tst 01:55:49.836
40 bytes from 192.168.1.121: seq 3, ttl 128, RTT 1.447 [mS] tso 01:55:50.644 tsr 01:55:50.836 tst 01:55:50.836
3 requests sent, 3 replies received, 0.0% loss in 4.01 [S]
%
% ping -c3 -Mt -s0 192.168.1.121
ICMP_TSTAMP
PING 192.168.1.121 (192.168.1.121): 0 data bytes
20 bytes from 192.168.1.121: icmp_seq=0 ttl=128 tso=01:56:16 tsr=747:47:29 tst=747:47:29
20 bytes from 192.168.1.121: icmp_seq=1 ttl=128 tso=01:56:17 tsr=710:34:53 tst=710:34:53
20 bytes from 192.168.1.121: icmp_seq=2 ttl=128 tso=01:56:18 tsr=598:48:22 tst=598:48:22

--- 192.168.1.121 ping statistics ---
3 packets transmitted, 3 packets received, 0.0% packet loss
% 
```
Successful ICMP Netmask request pings produce the following output (N.B. Normally
Network Devices do not respond to ICMP Netmask requests. However, as noted in the
Apple macOS man page for ping, macOS can reply to Netmask requests after a change
to a system setting. The command to make a temporary setting change to macOS is; -
"sudo sysctl net.inet.icmp.maskrepl=1"
```
% ./ping3 -M mask 192.168.1.103
32 bytes from 192.168.1.103: seq 1, ttl 64, RTT 69.548 [mS] mask 255.255.255.0
32 bytes from 192.168.1.103: seq 2, ttl 64, RTT 86.521 [mS] mask 255.255.255.0
32 bytes from 192.168.1.103: seq 3, ttl 64, RTT 105.079 [mS] mask 255.255.255.0
3 requests sent, 3 replies received, 0.0% loss in 4.02 [S]
%
% ping -c3 -Mm -s0 192.168.1.103
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

useage: ping3 [options] NetworkDeviceName
or      ping3 [options] NetworkDeviceIP_Number

where options are; -
        -cX  specifies number of times to ping remote network device ( 0 <= X <= 100 )
          where a value of 0 invokes continuous ping mode. Stop this mode with control-C or control-\.
        -D  switches on debug output and over-rides -q
        -h  switches on this help output and then terminates ping3
        -iX.X  ensure X.X second interval between each ping ( 0.1 <= X.X <= 60.0 )
        -lXX  suggest desired IP header option length (max is 40 and should be a multiple of 4)
        -M ABC  specifies ping with ICMP Mask/Timestamp request instead of ICMP Echo.
          where ABC is a string of characters.
            If "mask" then send ICMP Mask request,
            if "time" then send ICMP Time Stamp request,
            if "timew" as above, but treat tsr and tst timestamps as little endian.
            (i.e. Windows default response, if the ICMP Time Stamp request
            is allowed through the Windows firewall. )
        -p ABC  specifies a pattern for the data payload included with the ping.
          where ABC is a string of characters.
            if "time" then send milliseconds since midnight as the data payload,
            if "random" then send a random array of bytes as the data payload,
            if neither of the above then specifying up to 16 bytes in hexadecimal.
        -q  forces quiet (minimum) output and overrides -v
        -R  specifies header option Record Route (N.B. -T overrides -R when both are specified)
        -s "XX [YY [ZZ]]" specifies ICMP Echo data section size (N.B. 16 <= XX <= 1472 works best on macOS)
          where XX is an integer number and YY, ZZ are optional integer numbers to specify a size sweep.
        -tXX  specifies IPv4 header Time to Live (N.B. doesn't work on macOS)
        -T ABC  specifies header option time stamp type.
          where ABC is a string of characters.
            If "tsonly" then record Time Stamp Only list of time stamps,
            if "tsandaddr" then record Address and Time Stamp pair list,
            if "tsprespec H.I.J.K [ L.M.N.O [ P.Q.R.S [ T.U.V.W ]]]" then Time Stamp prespecified Addresses.
        -v  switches on verbose output
        -wX  wait for X seconds after last request for any replies ( 1 <= X <= 20 )

%
```
ping3 is released under the MIT license and must be used at your own risk.
Therefore unless it provides some specific capability you need then it is
almost certainly better to use your operating systems standard ping utility.

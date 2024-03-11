# ping3
The "ping3" utility program can be used to send multiple
Internet Protocol version 4 (IPv4) Internet Control Message
Protocol (
<a href="https://datatracker.ietf.org/doc/html/rfc792">ICMP</a>
) Echo requests or multiple IPv4 ICMP Time Stamp
requests or multiple IPv4 ICMP Address Mask requests to a
remote network device and display information about the round
trip time if corresponding ICMP replies are received back.

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
alternate routing options or any specific multicast functionality.

The standard ping utility that comes with Apple Mac operating systems,
such as macOS Sonoma, provides similar default capability to the ping
utility from the iputils package that is typically used on linux,
but both differ considerably for more rarely used modes of
operation. Linux ping provides a timestamp (-T) and record route
(-R) capability through IPv4 header options and macOS ping doesn't.
(N.B. In the past macOS ping did have a working -R option and still
lists a -R option if you look at the output of ping -h, but the man
page for ping states "This option is deprecated and is now a no-op.")
However, even though -T and -R capability are missing from macOS ping,
it provides other capability such as a timestamp capability through
ICMP timestamp requests (-M time) and a netmask capability through
ICMP mask requests (-M mask). Both modes require elevated priveleges
via sudo, unless the "-s 0" option is included with the "-M time"
or "-M mask" option. (N.B. macOS ping does not output the full
timestamp information as it truncates the time it displays to
whole seconds and thus doesn't show the millisecond values. The
ICMP timestamp reply has milliseconds since midnight information,
so it is available. I have submitted feedback to Apple in FB13523685
about this lack of millisecond information.) Linux ping has no ICMP
timestamp or ICMP netmask capability. The ping utility that comes
with Microsoft Windows seems more like the iputils (linux) ping than
the macOS ping.

The ping3 utility provides timestamp or record route in the IPv4
header options on a macOS system and provides ICMP timestamp or
ICMP mask pings on a Linux system. On Linux ping3 requires the
increased privelege via sudo to work, but it does not require
sudo to work on macOS. In the Cygwin environment on Windows the
ping3 utility provides timestamp and record route, but doesn't
currently provide ICMP timestamp or mask pings.

The default output of ping3 is somewhat similar to the output of
ping, but with a different length number at the start of each
reply description. The ping3 length value is the total size of
the IPv4 (ICMP) datagram, not the size of the ICMP message
as is shown in the output lines from ping. In the following
example ping3 reports a total of 84 bytes received, which is
20 bytes of IPv4 header and 64 bytes of ICMP echo reply message.
In comparison ping reports 64 bytes of ICMP echo reply message,
which implies the total length of the datagram was 84 bytes
since the default IPv4 header size is 20 bytes.
Both outputs follow; -
```
% ./ping3 www.apple.com
84 bytes from 23.202.170.41: seq 1, ttl 52, RTT 21.764 [mS]
84 bytes from 23.202.170.41: seq 2, ttl 52, RTT 21.488 [mS]
84 bytes from 23.202.170.41: seq 3, ttl 52, RTT 21.277 [mS]
3 requests sent, 3 replies received, 0.0% loss in 4.02 [S]
RTT Min 21.277, Avg 21.510, Max 21.764 [mS]
% 
% ping -c3 www.apple.com 
PING e6858.dscx.akamaiedge.net (23.202.170.41): 56 data bytes
64 bytes from 23.202.170.41: icmp_seq=0 ttl=52 time=25.608 ms
64 bytes from 23.202.170.41: icmp_seq=1 ttl=52 time=21.081 ms
64 bytes from 23.202.170.41: icmp_seq=2 ttl=52 time=21.578 ms

--- e6858.dscx.akamaiedge.net ping statistics ---
3 packets transmitted, 3 packets received, 0.0% packet loss
round-trip min/avg/max/stddev = 21.081/22.756/25.608/2.027 ms
%
```
The summary information output by ping3 in the example above
does not show a Standard Deviation value as does the output
of the macOS ping. This is a deliberate choice as ping3 only
shows a StdDev value when it has 10 or more RTT values to put
into the standard deviation calculation.

The default echo request sent by ping3 is much the same as the
echo request sent by the linux ping utility, but differs from the
macOS ping echo request because the ICMP Echo message payload data
is different. The default payload for ping3 is 16 bytes of time
information and then 40 bytes of incrementing byte values. The
macOS ping echo request payload data only allocates the first 8
bytes to time and then has 48 bytes incrementing byte values.
The manual pages (man ping) for ping on both linux and macOS
state that the ICMP header is followed by a "struct timeval"
(i.e. structured time data) and then "pad" bytes to fill out
the packet. (?? It is not clear to me why the apparent size
of the time is 16 bytes on 64 bit linux and only 8 bytes on
64 bit macOS. I also note that both linux and macOS ping do
not show any RTT time values for operation with -s option
smaller than 16 (i.e. ping -s 15 www.apple.com), so maybe
apple ping does sometimes use 16 bytes of time - just not
when I captured the echo request shown below. ( I have
lodged feedback with Apple about the man page information
for macOS ping in regard to the size of the timestamp size
through feedback item FB13453152 ) N.B. the ping3
utility does not rely on the time being sent in echo requests
and so does not suffer the no RTT time shown problem for pings
with very small datagram size.)
The following datagram capture and display from tcpdump
illustrates an example of the default ping3 echo request
followed by an example of the default macOS ping echo request; -
```
22:48:16.105211 IP (tos 0x0, ttl 64, id 22724, offset 0, flags [none], proto ICMP (1), length 84)
    192.168.1.124 > 192.168.1.106: ICMP echo request, id 3996, seq 1, length 64
	0x0000:  4500 0054 58c4 0000 4001 9dae c0a8 017c
	0x0010:  c0a8 016a 0800 a7a2 0f9c 0001 8019 6b65
	0x0020:  0000 0000 4066 5608 0000 0000 1011 1213
	0x0030:  1415 1617 1819 1a1b 1c1d 1e1f 2021 2223
	0x0040:  2425 2627 2829 2a2b 2c2d 2e2f 3031 3233
	0x0050:  3435 3637

22:48:50.406844 IP (tos 0x0, ttl 64, id 50467, offset 0, flags [none], proto ICMP (1), length 84)
    192.168.1.124 > 192.168.1.106: ICMP echo request, id 40207, seq 0, length 64
	0x0000:  4500 0054 c523 0000 4001 314f c0a8 017c
	0x0010:  c0a8 016a 0800 c6e3 9d0f 0000 656b 19a2
	0x0020:  0006 29f6 0809 0a0b 0c0d 0e0f 1011 1213
	0x0030:  1415 1617 1819 1a1b 1c1d 1e1f 2021 2223
	0x0040:  2425 2627 2829 2a2b 2c2d 2e2f 3031 3233
	0x0050:  3435 3637
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
40 bytes from 192.168.1.1: seq 1, ttl 64, RTT 5.513 [mS] tso 12:08:58.942 tsr 12:08:58.572 tst 12:08:58.572
40 bytes from 192.168.1.1: seq 2, ttl 64, RTT 5.507 [mS] tso 12:08:59.947 tsr 12:08:59.577 tst 12:08:59.577
40 bytes from 192.168.1.1: seq 3, ttl 64, RTT 4.302 [mS] tso 12:09:00.948 tsr 12:09:00.576 tst 12:09:00.576
3 requests sent, 3 replies received, 0.0% loss in 4.01 [S]
RTT Min 4.302, Avg 5.107, Max 5.513 [mS]
%
```
The output from macOS ICMP Timestamp request ping for
comparison purposes is (N.B. macOS prints integer Hrs:Min:Sec
since midnight GMT even though the reply has milliseconds
since midnight resolution ); -
```
% ping -c3 -Mt -s0 192.168.1.1
ICMP_TSTAMP
PING 192.168.1.1 (192.168.1.1): 0 data bytes
20 bytes from 192.168.1.1: icmp_seq=0 ttl=64 tso=12:09:21 tsr=12:09:21 tst=12:09:21
20 bytes from 192.168.1.1: icmp_seq=1 ttl=64 tso=12:09:22 tsr=12:09:22 tst=12:09:22
20 bytes from 192.168.1.1: icmp_seq=2 ttl=64 tso=12:09:23 tsr=12:09:23 tst=12:09:23

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
40 bytes from 192.168.1.121: seq 1, ttl 128, RTT 7.207 [mS] tso 10:28:21.982 tsr 10:28:22.174 tst 10:28:22.174
40 bytes from 192.168.1.121: seq 2, ttl 128, RTT 4.422 [mS] tso 10:28:22.988 tsr 10:28:23.189 tst 10:28:23.189
40 bytes from 192.168.1.121: seq 3, ttl 128, RTT 5.974 [mS] tso 10:28:23.989 tsr 10:28:24.189 tst 10:28:24.189
3 requests sent, 3 replies received, 0.0% loss in 4.01 [S]
RTT Min 4.422, Avg 5.868, Max 7.207 [mS]
%
% ping -c 3 -M time -s 0 192.168.1.121
ICMP_TSTAMP
PING 192.168.1.121 (192.168.1.121): 0 data bytes
20 bytes from 192.168.1.121: icmp_seq=0 ttl=128 tso=10:28:27 tsr=919:47:02 tst=919:47:02
20 bytes from 192.168.1.121: icmp_seq=1 ttl=128 tso=10:28:28 tsr=803:20:54 tst=803:20:54
20 bytes from 192.168.1.121: icmp_seq=2 ttl=128 tso=10:28:29 tsr=766:08:18 tst=766:08:18

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
        -a  switches on audible output notification of replies received
        -b  switches on broadcast permission for requests to be sent to a broadcast address
        -cX  specifies number of times to ping remote network device ( 0 <= X <= 100 )
          where a value of 0 invokes continuous ping mode. Stop this mode with control-C or control-\.
        -D  switches on IPv4 header Don't Fragment setting in ICMP request datagrams
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
            if neither of the above then specifying up to 28 bytes in hexadecimal.
        -R  specifies header option Record Route (N.B. -T overrides -R when both are specified)
        -s "XX [YY [ZZ]]" specifies ICMP Echo data section size (N.B. 16 <= XX <= 1472 works best on macOS)
          where XX is an integer number and YY, ZZ are optional integer numbers to specify a size sweep.
        -tXX  specifies IPv4 header Time-to-Live setting ( 1 <= XX <= 255 )
        -T ABC  specifies header option time stamp type.
          where ABC is a string of characters.
            If "tsonly" then record Time Stamp Only list of time stamps,
            if "tsandaddr" then record Address and Time Stamp pair list,
            if "tsprespec H.I.J.K [ L.M.N.O [ P.Q.R.S [ T.U.V.W ]]]" then Time Stamp prespecified Addresses.
        -vX  sets a verbosity level ( -9 <= X <= 9 ) More positive values of X provide more information.
             a verbosity level of zero ( -v 0 ) is equivalent to not using -v.
        -wX  wait for X seconds after last request for any replies ( 1 <= X <= 20 )

%
```
Note that the IPv4 header Don't Fragment setting manipulation is not available on early (before Big Sur? )
macOS and so the -D option is not shown in the above ping3 useage message on versions of macOS that do not
give access to the IP_DONTFRAG setting in the setsockopt function call.

Further information about verbosity level follows; -
```
  -v -9 .. No output to stdout. DNS name lookup errors reported to stderr. Success / failure indicated by return value.
  -v -8 .. Report requests sent and replies received separated by a comma. E.g. 3, 3
  -v -7 .. Report summary line "X requests sent, Y replies received, ZZZ.Z% loss
  -v -6 .. Report summary line "X requests sent, Y replies received, ZZZ.Z% loss in S.SS [S]"
  -v -5 .. Report previous ( -v -6 ) line plus RTT stats, smallest, mean, largest and standard dev if enough samples
  -v -4 .. Same as -v -5
  -v -3 .. Same as -v -4
  -v -2 .. Same as -v -3
  -v -1 .. Same as -v -2
  -v 0  or no -v option .. Same as -v -1
  -v 1 .. Report Names of local and remote network devices in addition to data and summary provided by -v 0
  -v 2 .. Report any remote network device name alias' in addition to data and summary provided by -v 1
  -v 3 .. Timestamp the replies in addition to data and summary provided by -v 2
  -v 4 .. List up to the last 100 ping attempt timestamps in addition to timestamped data and summary provided by -v 3
  -v 5 .. Report version in addition to information provided by -v 4
  -v 6 .. Report Don't Fragment & Broadcast Permission status in addition to information provided by -v 5
  -v 7 .. Report request & reply timestamps in greater detail in addition to information provided by -v 6
  -v 8 .. Display the headers in received reply datagrams in addition to information provided by -v 7
  -v 9 .. Debug mode. Reports all manner of internal data. Compile with -DDEBUG flag to enable the most info output.
```
These verbosity level output indications are not to be considered as set-in-stone and may be altered in later versions of ping3.

ping3 is released under the MIT license and must be used at your own risk.
Therefore unless it provides some specific capability you need then it is
almost certainly better to use your operating systems standard ping utility.

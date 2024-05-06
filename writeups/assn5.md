Assignment 5 Writeup
=============

My name: Jiwoo Hong

My POVIS ID: jiwooh

My student ID (numeric): 20220871

This assignment took me about 5 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): None

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the NetworkInterface:
- 1. send_datagram(dgram, next_hop)
(1) make a EthernetFrame with given InternetDatagram
(2) if destination Ethernet address is already known (is in _arpTable), send the frame
    ONLY if the TTL of the mapping is not expired (30 secs)
(3) if destination Ethernet address is UNKNOWN (is not in _arpTable), send ARP request and queue the IP datagram
    push frame to _waitingFrames (queue of frames waiting for ARP reply)
    if there is already ARP request for same IP address in last 5 seconds, do not send a second request
    else, send ARP request

- 2. recv_frame(frame)
(1) ignore frames not destined for network interface:
    Ethernet destination should be broadcast address OR the MAC address of the network interface
(2) inbound frame is IPv4:
    Parse payload as InternetDatagram, if successful (got ParseResult::NoError), then return resulting InternetDatagram
(3) inbound frame is ARP:
    Parse payload as ARP, if successful (got ParseResult::NoError), then return resulting ARP
    Learn mapping of sender's MAC address to sender's IP address and TTL [using _arpTable]
        cf. _arpTable : mapping from uint32_t IP address to EthernetAddress MAC address and size_t ttl (struct _arp)
    If ARP request asks for our IP address, send ARP reply
    If there are associated waiting frames regarding IP address, update their destination and send them

- 3. tick(ms_since_last_tick)
    timer implementation for the NetworkInterface

Implementation Challenges:
- No particular challenges were encountered during the implementation of the NetworkInterface.

Remaining Bugs:
- I assume there are no remaining bugs.

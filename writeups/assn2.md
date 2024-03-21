Assignment 2 Writeup
=============

My name: Jiwoo Hong

My POVIS ID: jiwooh

My student ID (numeric): 20220871

This assignment took me about 5 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): None (I redid and completed assignment 1 after the best-submission policy was introduced.)

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:
1. TCPReceiver
a. segment_received()
- SYN recieved
    * before any SYN: do not start recieving (`return`)
    * on first SYN: set `_recievedSYN` flag and save seqno to `_isn`
    * later SYNs: ignore
- FIN recieved: set `_recievedFIN` flag
- calculate absolute seqno: use unwrap() with seqno+synm `_isn` and checkpoint = written bytes
- push substring: get input str by seg.payload().copy() and calculate index with absSeqno-1, and send fin value of a segment as `eof` to reassembler

b. calculating ackno()
- returns nothing if SYN is not recieved yet
- if SYN is recieved, add number of written bytes in the stream and number of flags together and wrap() it with saved `_isn` value. 

2. wrap/unwrap
- wrap(): simply make a new WarppingInt32 with value isn+n
- unwrap()
    * calculate sequence number by subtracting isn from n
    * if checkpoint is equal to or smaller, return seqno since no wraparound happened
    * calculate number of 2^32 wraparounds by ((checkpoint - seqno) >> 32)
    * calculate lower and upper 2^32 bounds and return the closer one to the checkpoint

Implementation Challenges:
Needed to remember that flags SYN and FIN also occupy a bit in ackno. 

Remaining Bugs:
I assume there are no remaining bugs.

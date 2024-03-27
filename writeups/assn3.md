Assignment 3 Writeup
=============

My name: Jiwoo Hong

My POVIS ID: jiwooh

My student ID (numeric): 20220871

This assignment took me about 9 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): None

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPSender:
0. General structure
- Send split segments to the receiver using fill_window(), send_segment()(, send_empty_segment())
- Keep resending if sent segments don't get acknowledged soon enough using ack_received()
- While keeping the passage of time using tick()

1. Sending segments
- fill_window()
  Need to send SYN first, so we set _SYN flag on first ever call.
  Save if window size is zero or not since we are going to see windows size 0 as 1.
  While no FIN is sent yet, calculate remaining window size and repeatedly send appropriate amount of input stream to the receiver. Raise FIN if eof and window size is enough to send FIN flag.

- send_segment()
  Actually send segments to the output stream. Set appropriate seqno and increment seqno and save the segment to variables regarding outstanding segments information. Send segment and start timer if it is not running.
    + send_empty_segment()
      Similar to send_segment() but no tracking of segment is needed since it is empty. Setting a valid seqno and pushing it into the output stream is enough. (= No use of _bytesInFlight & _sendingSegments & _isTimerRunning)

2. Resending segments in respect to ACKs
- ack_received()
  Calculate absolute ackno with unwrap() from assn2. If outstanding segments exist in the outstanding segments queue, check if those segments are ack'd already by comparing their absolute ackno. If they are, remove from the queue. Call fill_window() to fill up open space, and reset RTO, restart timer.

3. Keep passage of time
- tick()
  Increment timer by the value inserted, and check if RTO is elapsed while outstanding segments still exists.
  If they exist, retransmit, and do exponential backoff, restarting timer. Finally stop the timer when all outstanding segments are ack'd.

Implementation Challenges:
- I had an unexpected challenge with overflow of retx_timeout. While debugging errors on some tests, I found out that doubling retx_timeout with type `uint16_t` eventually results in an overflow, since retransmission timeout tends to exceed approximately 66k sometimes. Changing type to `uint64_t` solved the error, but given `const uint16_t retx_timeout` on the constructor in file `tcp_sender.cc`, I was confused whether type `uint16_t` was intended to work without errors I got. 

Remaining Bugs:
- I assume there are no remaining bugs.

Assignment 4 Writeup
=============

My name: Jiwoo Hong

My POVIS ID: jiwooh

My student ID (numeric): 20220871

This assignment took me about 20 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): None

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Your benchmark results (without reordering, with reordering): 0.59 Gbit/s, 0.62 Gbit/s

Program Structure and Design of the TCPConnection:
- 1. on receiving segments : segment_received(seg), write(data)
    [segment_received(seg)]
    * resets timer, do not receive when connection closed
    * responds to keep-alive segment, sending an empty segment
    * pass segment to the TCPReceiver
    * if ACK is set on incoming segment, tell TCPSender ackno & window size
    * if there is nothing to send in the outgoing queue, send an empty segment 
    * sets lingering policy: do not linger when inbound stream ends before outbound stream
    * lastly, send segments in reply to the incoming segments
    [write(data)]
    * writes data into the sender stream

- 2. sending segments : sendSegment()
    * Checks for existing 'waiting' segments on the sender queue, and repeats below for all waiting segments
    * Sets ACK and fields of segment referring to received ackno
    * Sets window size and RST if needed
    * Push out segment to the queue

- 3. unclean shutdown : uncleanShutdown()
    * Breakdown: sets RST flag, closes both streams
    * This also happens when RST is set on the header of receiving segments.

- 4. active status check : active()
    * If RST flag set: NOT active
    * If at least one of 3 shutdown options prerequisites is not false: still active
        * these 3 options should be met for the connection to be passively closed:
            1. inbound stream ended
            2. outbound stream ended and FIN sent
            3. outbound stream is fully acknowledged by peer
    * If lingering policy true: active (still connected)
    * Else: NOT active

- 5. timer : tick(ms_since_last_tick)
    * Tells TCPSender about passage of time
    * abort connection when number of consecutive retransmissions is more than a limit
    * check lingering conditions: both streams ended? time since last segment received >= 10 * timeout?
    * end connection cleanly if necessary


Implementation Challenges:
- I had an unexpected challenge with RST, SYN, ACK, and FIN conditions. With numerous attempts, I've found out that SYN is automatically handled by the sender (Assn3) so I do not need to care about it. Regarding RST and FIN, I've made flags indicating whether they were set(sent/received) and set/used them in the right circumstance they were needed. Plus, appropriately placing the check for lingering conditions inside the code flow was quite a challenge and took me some time.


Remaining Bugs:
- I assume there are no remaining bugs.

Assignment 6 Writeup
=============

My name: Jiwoo Hong

My POVIS ID: jiwooh

My student ID (numeric): 20220871

This assignment took me about 3 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): None

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the Router:
- _routeTable
  Table(`std::vector`) of routes(`Route`), containing route_prefix, prefix_length, next_hop, and interface_num

- add_route()
  Simply push_back given data into _routeTable

- route_one_datagram()
  1. choose matching route with biggest prefix_length, by using mask of prefix_length 1s
  2. if no route found or TTL equal or smaller than 1, drop datagram (return)
  3. decrement TTL
  4. send modified datagram on `interface_num` to target with interface().send_datagram()
    * `next_hop` if it has_value()
    * `dgram.header().dst` otherwise

Implementation Challenges:
- No particular challenges were encountered during the implementation of the NetworkInterface.

Remaining Bugs:
- I assume there are no remaining bugs.

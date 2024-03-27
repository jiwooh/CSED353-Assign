#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>
#include <iostream>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _retxTimeout(retx_timeout) {}

uint64_t TCPSender::bytes_in_flight() const { return _bytesInFlight; }

void TCPSender::fill_window() {
    // send SYN first
    if (!_SYN) {
        TCPSegment seg;
        seg.header().syn = true;
        send_segment(seg);
        _SYN = true;
        return;
    }
    _windowZero = _windowSize ? false : true; // need to save isZero information because we are seeing windowsize 0 as 1 below

    // send segments while window is not full and FIN is not sent yet
    size_t fWindowSize = _windowSize > 0 ? _windowSize : 1; // see _windowSize 0 as 1
    while (!_FIN) { // no FIN sent yet
        size_t remainingWindow = fWindowSize - (_next_seqno - _receievedAckno);
        if (!remainingWindow) { // stop sending if window is full
            return;
        }

        TCPSegment seg;
        size_t amount = min(remainingWindow, TCPConfig::MAX_PAYLOAD_SIZE);
        seg.payload() = Buffer(_stream.read(amount)); // payload() has type Buffer
        
        if (_stream.eof() && seg.length_in_sequence_space() < fWindowSize) { // raise FIN if eof and window is wide enough to send FIN
            seg.header().fin = true;
            _FIN = true;
        }
        if (seg.length_in_sequence_space() == 0) { // empty stream, stop sending
            break;
        }
        send_segment(seg);
    }
    return;
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) { 
    size_t absAckno = unwrap(ackno, _isn, _receievedAckno);
    if (absAckno > _next_seqno) { // invalid ackno recieved
        return;
    }
    _windowSize = window_size;
    if (absAckno <= _receievedAckno) { // absAckno is already ack'd
        return;
    }
    _receievedAckno = absAckno;

    while (_sendingSegments.size()) { // outstanding segments exist
        TCPSegment seg = _sendingSegments.front();
        if (unwrap(seg.header().seqno, _isn, _next_seqno) + seg.length_in_sequence_space() <= absAckno) { // this seg is ack'd already
            _bytesInFlight -= seg.length_in_sequence_space();
            _sendingSegments.pop();
        } else {
            break;
        }
    }

    fill_window();
    _retxTimeout = _initial_retransmission_timeout; // [3.1 7.a] reset RTO on receiving ack
    if (_sendingSegments.size()) { // [3.1 7.b] restart timer on existing outstanding data
        _timer = 0;
        _isTimerRunning = true;
    }
    _consecRetx = 0; // [3.1 7.c] reset consecutive retx
    return;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { 
    _timer += ms_since_last_tick; // [3.1 1.]
    if (_timer >= _retxTimeout && _sendingSegments.size()) { // [3.1 3.] RTO elapsed, timer expires
        _segments_out.push(_sendingSegments.front()); // [3.1 6.a] retransmit!
        _consecRetx++;
        if (!_windowZero) { // [3.1 6.b] "exponential backoff"
            _retxTimeout *= 2;
        }
        // [3.1 6.c]
        _timer = 0;
        _isTimerRunning = true;
    }
    if (_sendingSegments.empty()) { // [3.1 5.] stop timer when all outstanding segs are ack'd
        _isTimerRunning = false;
    }
    return;
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecRetx; }

void TCPSender::send_empty_segment() { // no tracking needed
    TCPSegment seg;
    seg.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(seg);
    return;
}

void TCPSender::send_segment(TCPSegment &seg) {
    seg.header().seqno = wrap(_next_seqno, _isn);
    _next_seqno += seg.length_in_sequence_space();
    _bytesInFlight += seg.length_in_sequence_space();
    _sendingSegments.push(seg); // backup
    _segments_out.push(seg); // actually send segment

    if (!_isTimerRunning) { // [3.1 4.] start timer after sending segment
        _isTimerRunning = true;
        _timer = 0;
    }
    return;
}
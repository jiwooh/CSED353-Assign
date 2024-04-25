#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _timeSinceLastSegRecv; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    _timeSinceLastSegRecv = 0;  // reset timer

    // 1. if RST set, set error on streams and kill connection => unclean shutdown
    if (seg.header().rst) {
        uncleanShutdown();
    }

    // do not receive when closed
    if (!active())
        return;

    // 2. respond to keep-alive segment, ONLY send empty segment (= returns)
    if (_receiver.ackno().has_value() && !seg.length_in_sequence_space() &&
        (seg.header().seqno == _receiver.ackno().value() - 1)) {
        _sender.send_empty_segment();
        sendSegment();
        return;
    }

    // 3. give segment to receiver (telling seqno & SYN & payload & FIN)
    _receiver.segment_received(seg);

    // 4. if ACK set, tell sender ackno & window_size
    if (seg.header().ack) {
        _sender.ack_received(seg.header().ackno, seg.header().win);
    } else {
        _sender.fill_window();
    }

    // send empty segment when recieved and nothing to send
    if (seg.length_in_sequence_space() && _sender.segments_out().empty()) {
        _sender.send_empty_segment();
    }

    // do not linger when inbound stream ends before outbound stream
    bool prereq1 = _receiver.stream_out().eof();
    bool prereq2 = _sender.stream_in().eof() && _FIN;
    if (prereq1 && !prereq2) {
        _linger_after_streams_finish = false;
    }

    // 5. send at least one segment in reply to incoming segment occupying seqno
    //    (reflect update on ackno & window_size)
    sendSegment();
    return;
}

bool TCPConnection::active() const {
    if (_RST)
        return false;
    // shutdown options prerequisites
    bool prereq1 = _receiver.stream_out().eof();
    bool prereq2 = _sender.stream_in().eof() && _FIN;
    bool prereq3 = !_sender.bytes_in_flight();
    if (!prereq1 || !prereq2 || !prereq3) {
        return true;
    }
    if (_linger_after_streams_finish)
        return true;
    return false;
}

size_t TCPConnection::write(const string &data) {
    if (!active())
        return 0;

    // write data to outbound bytestream in sender
    size_t writeBytes = _sender.stream_in().write(data);
    _sender.fill_window();
    sendSegment();
    return writeBytes;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    if (!active())
        return;

    // 1. tell TCPSender about passage of time
    _timeSinceLastSegRecv += ms_since_last_tick;
    _sender.tick(ms_since_last_tick);

    // 2. abort connection, send RST segment to peer,
    //    IF the number of consecutive retransmissions is more than
    //    an upper limit TCPConfig::MAX_RETX_ATTEMPTS
    if (_sender.consecutive_retransmissions() > _cfg.MAX_RETX_ATTEMPTS) {
        uncleanShutdown();
    }

    // linger after both streams end for 10*rt_timeout
    bool prereq1 = _receiver.stream_out().eof();
    bool prereq2 = _sender.stream_in().eof() && _FIN;
    bool prereq3 = !_sender.bytes_in_flight();
    bool prereq4 = _timeSinceLastSegRecv >= 10 * _cfg.rt_timeout;
    if (prereq1 && prereq2 && prereq3 && prereq4) {
        _linger_after_streams_finish = false;
    }

    // 3. end connection cleanly if necessary
    sendSegment();
    return;
}

void TCPConnection::end_input_stream() {
    if (!active())
        return;

    _sender.stream_in().end_input();
    _sender.fill_window();
    sendSegment();
    return;
}

void TCPConnection::connect() {
    _sender.fill_window();
    sendSegment();
    return;
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // send RST segment to peer
            uncleanShutdown();
            _sender.send_empty_segment();
            sendSegment();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

void TCPConnection::sendSegment() {
    // 1. any time the TCPSender has pushed a segment onto its outgoing queue
    //    having set the fields it’s responsible for on outgoing segments:
    //    seqno, SYN, payload, and FIN
    // send existing segments
    queue<TCPSegment> &waitingSegments = _sender.segments_out();

    while (!waitingSegments.empty()) {
        TCPSegment seg = waitingSegments.front();

        if (seg.header().fin)
            _FIN = true;

        // 2. before sending the segment, ask the TCPReceiver for ackno & window_size
        //    If there is ackno, it will set ACK flag and fields in TCPSegment
        // set ACK flag and fields in TCPSegment
        if (_receiver.ackno().has_value()) {
            seg.header().ack = true;
            seg.header().ackno = _receiver.ackno().value();
        }

        // set window size in TCPSegment
        // maximum size is upper limit of type `uint16_t`
        size_t maxWindowSize = static_cast<size_t>(numeric_limits<uint16_t>().max());
        seg.header().win = min(_receiver.window_size(), maxWindowSize);
        // set RST flag in TCPSegment if needed
        if (_RST)
            seg.header().rst = true;

        // send the segment
        _segments_out.push(seg);
        waitingSegments.pop();
    }
    return;
}

void TCPConnection::uncleanShutdown() {
    _RST = true;
    // close streams
    _receiver.stream_out().set_error();
    _sender.stream_in().set_error();
    return;
}

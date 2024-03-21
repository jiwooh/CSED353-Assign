#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    TCPHeader header = seg.header();

    // on first SYN
    if (header.syn && !_recievedSYN) {
        _recievedSYN = true;  // set SYN flag
        _isn = header.seqno;  // set ISN
    }
    if (!_recievedSYN) {  // do not start recieving before SYN
        return;
    }
    if (header.fin) {  // set FIN flag
        _recievedFIN = true;
    }

    // push str into reassembler
    size_t absSeqno = unwrap(header.seqno + header.syn, _isn, _reassembler.stream_out().bytes_written());
    _reassembler.push_substring(seg.payload().copy(), absSeqno - 1, header.fin);  // -1 to make index

    return;
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_recievedSYN)
        return {};
    size_t flagBits = 0;  // SYN and FIN flags also occupy bits
    if (_recievedSYN)
        flagBits++;
    if (_recievedFIN && _reassembler.empty())
        flagBits++;
    return wrap(flagBits + _reassembler.stream_out().bytes_written(), _isn);
}

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size(); }

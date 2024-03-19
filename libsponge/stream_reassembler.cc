#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : 
    _output(capacity), 
    _capacity(capacity),
    _buffer(capacity, 0),
    _bufferCheck(capacity, false) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    size_t waitingStart = _output.bytes_written();
    size_t bufferEnd = _capacity + waitingStart - _output.buffer_size();
    size_t dataStart = max(index, waitingStart);
    size_t dataEnd = max(index + data.size(), waitingStart);
    size_t dataBound = min(dataEnd, bufferEnd);

    // handle eof
    if ((dataEnd == dataBound) && eof) {
        _eofFlag = true;
    }

    // write data to buffer
    for (size_t i = dataStart; i < dataBound; i++) {
        size_t dataIndex = i - index;
        size_t bufferIndex = i - waitingStart;
        // insert to buffer if cell empty
        if (_bufferCheck[bufferIndex] == false) {
            _buffer[bufferIndex] = data[dataIndex];
            _bufferCheck[bufferIndex] = true;
            _waitingSize++;
        }
    }

    // read (and concat) data from buffer and write to output
    string inputData;
    for (size_t i = 0; i < _capacity && _bufferCheck.front(); i++) {
        // concat waiting strings
        inputData += _buffer.front();
        _waitingSize--;

        // move buffer
        _buffer.pop_front();
        _bufferCheck.pop_front();
        _buffer.push_back(0); // NULL
        _bufferCheck.push_back(false);
    }
    _output.write(inputData);

    // check end condition
    if (!_waitingSize && _eofFlag) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _waitingSize; }

bool StreamReassembler::empty() const { return _waitingSize == 0; }

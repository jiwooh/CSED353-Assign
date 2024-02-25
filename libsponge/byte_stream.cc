#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t min(const size_t a, const size_t b) {
    return a > b ? b : a;
}

ByteStream::ByteStream(const size_t capacity): _stream(), _capacity(capacity), _currentSize(0), _endInput(false), _totalReadBytes(0), _totalWriteBytes(0) {}

size_t ByteStream::write(const string &data) {
    size_t writeSize = min(data.size(), remaining_capacity());
    for (size_t i = 0; i < writeSize; i++) {
        _stream.push_back(data[i]);
    }
    _currentSize += writeSize;
    _totalWriteBytes += writeSize;
    return writeSize;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t outputSize = min(len, buffer_size());
    return string(_stream.begin(), _stream.begin() + outputSize);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t popSize = min(len, _currentSize);
    for (size_t i = 0; i < popSize; i++) {
        _stream.pop_front();
    }
    _currentSize -= popSize;
    _totalReadBytes += popSize;
    return;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    size_t readSize = min(len, _currentSize);
    std::string res = string(_stream.begin(), _stream.begin() + readSize);
    for (size_t i = 0; i < readSize; i++) {
        _stream.pop_front();
    }
    _currentSize -= readSize;
    _totalReadBytes += readSize;
    return res;
}

void ByteStream::end_input() {
    _endInput = true;
    return;
}

bool ByteStream::input_ended() const {
    return _endInput;
}

size_t ByteStream::buffer_size() const {
    return _currentSize;
}

bool ByteStream::buffer_empty() const {
    return ByteStream::buffer_size() == 0;
}

bool ByteStream::eof() const {
    return !_currentSize && input_ended();
}

size_t ByteStream::bytes_written() const {
    return _totalWriteBytes;
}

size_t ByteStream::bytes_read() const {
    return _totalReadBytes;
}

size_t ByteStream::remaining_capacity() const {
    return _capacity - _currentSize;
}

#include "stream_reassembler.hh"
#include <iostream> ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) 
    : _output(capacity)
    , _capacity(capacity)
    , _waiting({}) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // 0. save EOF information
    if (eof) {
        _eofFlag = true;
        _eofIndex = index + data.size();
    }

    // 1. insert substr or trim overlapping substr
    size_t startIndex = max(index, _nextIndex);
    size_t endIndex = min(index + data.length(), _capacity);
    if (startIndex > endIndex) return; // exception
    string trimData = data.substr(startIndex - index, endIndex - startIndex);

    // handle overlap, replace existing segments
    for (auto it = _waiting.begin(); it != _waiting.end() && it->first < endIndex;) {
        size_t itStart = it->first;
        size_t itEnd = it->first + it->second.size();
        if (startIndex < itEnd) {
            if (startIndex <= itStart && itEnd <= endIndex) { // if new substr covers the preexisting one
                _waitingBytes -= it->second.size();
                it = _waiting.erase(it);
            } else if (itStart < startIndex && endIndex < itEnd) { // if preexisting one covers the new substr
                string leftPart = it->second.substr(0, startIndex - itStart);
                string rightPart = it->second.substr(endIndex - itStart);
                _waitingBytes -= it->second.size();
                _waiting[itStart] = leftPart;
                _waiting[endIndex] = rightPart;
                _waitingBytes += leftPart.size() + rightPart.size();
                it = _waiting.erase(it);
            } else if (itStart < startIndex && itEnd <= endIndex) { // = till itEnd is overlapping
                string newPart = it->second.substr(0, startIndex - itStart);
                _waitingBytes -= it->second.size();
                _waiting[itStart] = newPart;
                _waitingBytes += newPart.size();
                it++;
            } else if (startIndex <= itStart && endIndex < itEnd) { // = from itStart is overlapping
                string newPart = it->second.substr(endIndex - itStart);
                _waitingBytes -= it->second.size();
                _waiting[endIndex] = newPart;
                _waitingBytes += newPart.size();
                it = _waiting.erase(it);
            }
        } else {
            it++; // move to next segment if no overlap
        }
    }

    // and insert or merge the new data
    _waiting[startIndex] = trimData;
    _waitingBytes += trimData.size();

    // 2. write contiguous waiting segments to output
    for (auto it = _waiting.begin(); it != _waiting.end() && it->first == _nextIndex;) {
        _output.write(it->second);
        _nextIndex += it->second.size();
        _waitingBytes -= it->second.size();
        it = _waiting.erase(it);
    }

    // 3. handle EOF
    if (_eofFlag && empty() && (_nextIndex == _eofIndex)) {
        _output.end_input();
    }
    return;
}

size_t StreamReassembler::unassembled_bytes() const { return _waitingBytes; }

bool StreamReassembler::empty() const { return _waitingBytes == 0; }

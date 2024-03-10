Assignment 1 Writeup
=============

My name: Jiwoo Hong

My POVIS ID: jiwooh

My student ID (numeric): 20220871

This assignment took me about 12 hours to do (including the time on studying, designing, and writing the code).

Program Structure and Design of the StreamReassembler:
1) Basic Variables
- _waiting (type: map<size_t, string>)
A list for storing substrings whose index is bigger than current index needs to be concatenated
Stores their index(size_t) and substring(string) itself
- _nextIndex: Index indicating where substring needs to start concatenating from, Grows as substrings get concatenated
- _eofFlag, _eofIndex: Flags when `eof` has been set up, and stores the index of eof in the substrings

2) Program Flow
- Store EOF information using _eofFlag & _eofIndex if `eof` is set up
- Trim `data`, excluding out-of-range parts
- Handle overlaps with new substrings and segments in _waiting
    - Replace existing segments, cutting out overlapping parts
    - Case 1: If new substring covers preexisting segment
    - Case 2: If preexisting segment covers the new substring
    - Case 3: Left part of new substring is overlapping
    - Case 4: Right part of new substring is overlapping
- Insert data and write contiguous waiting segments if possible
- handle EOF condition

Implementation Challenges:
- Dealing with EOF conditions
  Had some confusion when dealing with EOF conditions, i.e. when should it stop getting inputs or where (on what index) should EOF happen. 
  `eof` == true did not mean the reassembler should stop immediately. 
  Needed to flag `eof` set, and needed to save the index where EOF should happen (in _eofIndex). 
- Handling various cases of overlaps
  There were various cases of overlaps between input substring and the segments in _waiting buffer.
  Tried to handle them as much as possible, taking into account multiple cases but not sure if they are perfect or correct.
- Also tried fixing code due to failures of tests after tests, but expecially had challenges with test 23, 24, and 25.

Remaining Bugs:
- Cannot pass tests 23, 24, and 25 due to imcomplete implementation

- Optional: I had unexpected difficulty with: Dealing with different cases of handling overlaps

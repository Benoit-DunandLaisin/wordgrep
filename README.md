wordgrep
========

wordgrep implementation

========

This is a C++ test for a job appliance.
Specifications are written with latex.

========

Write a utility with the following syntax:
wordgrep dictfile [inputfile ...]
wordgrep parses a dictionary file as follows:
• a line with a single word defines a word to be found in the input stream
• empty lines and line starting with # are ignored
wordgrep then filters (to the standard output) its input stream, the concatenation of all
input files specified on the command line or standard input if none is given. wordgrep
outputs any line from the input streams that contains one of the words from the dictfile.
Matching is performed on a full word basis. Words are delimited by white space.
Implementation should be simple and efficient, dictionary size should be limited only
by memory,
An arbitrary limit on line length is OK for the dictionary, but should be unnecessary
on input stream.


/*
  +---------------------------------------+
  | BETH YW? WELSH GOVERNMENT DATA PARSER |
  +---------------------------------------+

  AUTHOR: Oliver Morris - 979663

  This file contains the code responsible for opening and closing file
  streams. The actual handling of the data from that stream is handled
  by the functions in data.cpp. See the header file for additional comments.
 */

#include "input.h"

/**
  Constructor for an InputSource.

  @param source
    A unique identifier for a source (i.e. the location).
*/
InputSource::InputSource(const std::string& _source) : source(_source) {}

/**
  This function should be callable from a constant context.

  @return
    A non-modifiable value for the source passed into the constructor.
*/
const std::string InputSource::getSource() {
	return source;
}

/**
  Constructor for a file-based source.

  @param path
    The complete path for a file to import.

  @example
    InputFile input("data/areas.csv");
*/
InputFile::InputFile(const std::string& filePath) : InputSource(filePath) {}

/**
  Open a file stream to the file path retrievable from getSource()
  and return a reference to the stream.

  @return
    A standard input stream reference

  @throws
    std::runtime_error if there is an issue opening the file, with the message:
    InputFile::open: Failed to open file <file name>

  @example
    InputFile input("data/areas.csv");
    input.open();
*/
std::istream& InputFile::open() {

	file_stream.open(getSource(), std::ios::in);

	//check to see if the file opened correctly
	if(!file_stream.is_open()) {
		throw std::runtime_error("InputFile::open: Failed to open file " + getSource());
	}

	return (std::istream&) file_stream;
}

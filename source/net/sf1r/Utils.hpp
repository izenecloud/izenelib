/* 
 * File:   Utils.hpp
 * Author: paolo
 *
 * Created on December 31, 2011, 2:40 PM
 */

#ifndef UTILS_HPP
#define	UTILS_HPP

#include <string>


/// Backported function from the ruby driver implementation.
/// @deprecated use std::string.length() instead
/*
 def num_bytes(str) #:nodoc:
      s = StringIO.new(str)
      s.seek(0, IO::SEEK_END)
      s.tell
 end
 */
size_t numBytes(std::string in) {
    return in.length();
}


/// Backported function from the ruby driver implementation.
/// @deprecated use const value \ref HEADER_SIZE
/*
 def header_size #:nodoc:
      2 * num_bytes([1].pack('N'))
 end
 */
size_t headerSize() {
    return 2 * sizeof(unsigned); // 8 bytes
}


#endif	/* UTILS_HPP */

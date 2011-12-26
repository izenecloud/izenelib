/*****************************************************************************
 The MIT License

 Copyright (c) 2009 Leandro T. C. Melo

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#ifndef IZENELIB_DRUM_EXCEPTION_HPP
#define IZENELIB_DRUM_EXCEPTION_HPP

#include "config.hpp"

#include <stdexcept>


DRUM_BEGIN_NAMESPACE

/*
 * Notes:
 *
 * - The error message is stored as a char pointer. Beware of the supplied strings life-time.
 *   String literals and any other static string fit this requirement. DrumException does not embed
 *   a std::string for safety. See details at: http://www.boost.org/community/error_handling.html.
 * - The naming convention used for the some functions below might look different from the one
 *   used throughout other DRUM pieces. In fact, it is not. It is based on the Google C++ Style
 *   Guide. Notice, however, that not everything is based on this guide.
 */


class DrumException : public std::exception
{
private:
    char const* error_msg_;
    int error_code_;

public:
    DrumException(char const* msg, int code = 0) : error_msg_(msg), error_code_(code) {}
    virtual ~DrumException() throw() {}

    virtual char const* what() const throw() { return get_error_msg(); }

    char const* get_error_msg() const throw() { return error_msg_; }
    int get_error_code() const throw() { return error_code_; }
};

DRUM_END_NAMESPACE

#endif //IZENELIB_DRUM_EXCEPTION_HPP

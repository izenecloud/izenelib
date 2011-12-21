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

#ifndef IZENELIB_DRUM_ELEMENT_IO_HPP
#define IZENELIB_DRUM_ELEMENT_IO_HPP

#include "config.hpp"
#include <fstream>
#include <string>


DRUM_BEGIN_NAMESPACE

template <class element_t>
struct ElementIO
{
    static void Serialize(element_t const& element, std::size_t & size, char const* & serial)
    {
        size = sizeof(element_t);
        serial = reinterpret_cast<char const*>(&element);
    }

    static void Deserialize(element_t & element, std::size_t size, char const* serial)
    {
        element = reinterpret_cast<element_t const&>(*serial);
    }
};

template <>
struct ElementIO<std::string>
{
    static void Serialize(std::string const& element, std::size_t & size, char const* & serial)
    {
        size = element.size();
        serial = element.c_str();
    }

    static void Deserialize(std::string & element, std::size_t size, char const* serial)
    {
        element.assign(serial, serial + size);
    }
};


DRUM_END_NAMESPACE

#endif //IZENELIB_DRUM_ELEMENT_IO_HPP

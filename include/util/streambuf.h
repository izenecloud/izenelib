#ifndef IZENELIB_STREAMBUF_H
#define IZENELIB_STREAMBUF_H

#include <types.h>

#include <boost/asio/streambuf.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>

#include <iostream>

NS_IZENELIB_UTIL_BEGIN

class izene_streambuf : public boost::asio::streambuf
{
public:		
    char* gptr(){ return std::streambuf::gptr();}

    size_t size() { return (std::streambuf::pptr() - std::streambuf::gptr());}

    char * eback () const { return std::streambuf::eback();}

    char * pptr () const { return std::streambuf::pptr();}

    void reserve(std::size_t n) { boost::asio::streambuf::reserve(n);}
};

std::size_t izene_read_until(std::istream& s, izene_streambuf& b, const std::string& delim);
std::size_t izene_read_until(std::istream& s, izene_streambuf& b, char delim);


NS_IZENELIB_UTIL_END

#endif //End of IZENELIB_STREAMBUF_H

#ifndef IZENELIB_STREAMBUF_H
#define IZENELIB_STREAMBUF_H

#include <boost/asio/streambuf.hpp>

NS_IZENELIB_UTIL_BEGIN

class izene_streambuf : public boost::asio::streambuf
{
public:
	char* data(){ return gptr();}
	size_t size()	{ return (pptr() - gptr());}
};

NS_IZENELIB_UTIL_END

#endif //End of IZENELIB_STREAMBUF_H

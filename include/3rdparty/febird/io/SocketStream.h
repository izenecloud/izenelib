/* vim: set tabstop=4 : */
#ifndef __febird_io_SocketStream_h__
#define __febird_io_SocketStream_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <stdio.h>
#include "../refcount.h"
#include "IStream.h"
#include "IOException.h"

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
#	if !defined(_WINSOCK2API_) && !defined(_WINSOCKAPI_)
#		include <WinSock2.h>
#	endif
#else
typedef int SOCKET;
#endif

namespace febird {

class FEBIRD_DLL_EXPORT SocketException : public IOException
{
public:
	explicit SocketException(const char* szMsg = "SocketException");
	explicit SocketException(int errCode, const char* szMsg = "SocketException");

	static int lastError();
};

class FEBIRD_DLL_EXPORT SocketStream : public RefCounter, public IDuplexStream
{
	DECLARE_NONE_COPYABLE_CLASS(SocketStream)
public:
	SocketStream(SOCKET socket, bool bAutoClose = true);
	~SocketStream();

public:
	size_t read(void* data, size_t length);
	size_t write(const void* data, size_t length);

	void flush() { }
	bool eof() const { return m_bEof; }

	size_t tellp() { return posp; }
	size_t tellg() { return posg; }

protected:
	virtual bool waitfor_again();

	::SOCKET socket;
	size_t posp; // sent size/pos
	size_t posg; // receive size/pos
	bool m_bEof; // for override IInputStream::eof
	bool m_bAutoClose;
};

class FEBIRD_DLL_EXPORT SocketAcceptor : public IAcceptor
{
	::SOCKET m_socket;
public:
	SocketAcceptor(const char* szBindAddr);
	SocketStream* accept();
};

FEBIRD_DLL_EXPORT SocketStream* ConnectSocket(const char* szServerAddr);

}

#endif // __febird_io_SocketStream_h__

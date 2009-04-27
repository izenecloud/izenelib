/* vim: set tabstop=4 : */
#ifndef __febird_io_DataIO_Parser_h__
#define __febird_io_DataIO_Parser_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include "StreamBuffer.h"
#include "DataInput.h"
#include <boost/spirit.hpp>

namespace febird {

template<class PrimInput>
class DataIO_TextReader : public DataInput<PrimInput, DataIO_TextReader<PrimInput> >
{
public:
	

protected:
	PrimInput* input;
	DataIO_Parser* parser;
};

template<class Final_Input>
class DataIO_XML_Reader
{
	InputBuffer* buf;
public:
	Final_Input& operator>>(int& x)
	{
		int ch = buf->readByte();
		switch (ch)
		{
		case 0: // oct
			break;

		}
		do {
			ch = 
		} while (isdigit(ch));
	}
};

class DataIO_Parser
{
public:

protected:
	const char* szMemberText;
	std::vector<std::string> memberNames;
};

}

#endif // __febird_io_DataIO_Parser_h__


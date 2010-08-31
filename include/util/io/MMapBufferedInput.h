#ifndef __MMAPBUFFEREDINPUT_H
#define __MMAPBUFFEREDINPUT_H

#include <util/io/BufferedInput.h>

#include <boost/iostreams/device/mapped_file.hpp>

#include <fstream>
#include <string>

NS_IZENELIB_UTIL_BEGIN

namespace io{

class MMapBufferedInput :	public BufferedInput
{
public:			
    MMapBufferedInput(const char* path);

    MMapBufferedInput(const MMapBufferedInput& clone);

    virtual ~MMapBufferedInput(void);
public:
    void readInternal(char* b,size_t length_,bool bCheck = true);

    void seekInternal(int64_t pos);
	
    BufferedInput* clone();
	
    void close();

private:
    int handle_;

    std::string filename_;

    uint8_t* data_;

    int64_t pos_;

    bool isClone_;
};


}

NS_IZENELIB_UTIL_END

#endif



#ifndef __MMAPINDEXINPUT_H
#define __MMAPINDEXINPUT_H

#include <ir/index_manager/store/IndexInput.h>

#include <boost/iostreams/device/mapped_file.hpp>

#include <fstream>
#include <string>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class MMapIndexInput :	public IndexInput
{
public:			
    MMapIndexInput(const char* path);

    MMapIndexInput(const MMapIndexInput& clone);

    virtual ~MMapIndexInput(void);
public:
    void readInternal(char* b,size_t length_,bool bCheck = true);

    void seekInternal(int64_t pos);
	
    IndexInput* clone();
	
    void close();

private:
    int handle_;

    string filename_;

    uint8_t* data_;

    int64_t pos_;

    bool isClone_;
};


}

NS_IZENELIB_IR_END

#endif



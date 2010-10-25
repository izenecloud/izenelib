/**
* @file        UDTFSIndexOutput.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief
*/
#ifndef UDTFSINDEXOUTPUT_H
#define UDTFSINDEXOUTPUT_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/utility/Utilities.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/store/UDTFSClient.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class UDTFSIndexOutput : public IndexOutput
{
public:
    UDTFSIndexOutput(const char* filename, const string& mode);

    UDTFSIndexOutput(const char* filename,char* buffer,size_t buffsize,const string& mode);

    UDTFSIndexOutput(const char* filename,size_t buffsize,const string& mode);

    virtual ~UDTFSIndexOutput(void);

public:
    void flushBuffer(char* b, size_t len);

    int64_t length()
    {
        return 0;
    }

    void close();

protected:
    void  seekInternal(int64_t pos);

private:
    UDTFile fileHandle_;

    std::string filename_;
};


}

NS_IZENELIB_IR_END

#endif


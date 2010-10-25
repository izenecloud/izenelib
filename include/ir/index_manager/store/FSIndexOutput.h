/**
* @file        FSIndexOutput.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief
*/
#ifndef FSINDEXOUTPUT_H
#define FSINDEXOUTPUT_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/utility/Utilities.h>
#include <ir/index_manager/store/IndexOutput.h>

#include <stdio.h>
#include <string>

using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class FSIndexOutput : public IndexOutput
{
public:
    FSIndexOutput(const char* filename, const string& mode, size_t buffsize = 0);

    virtual ~FSIndexOutput(void);

public:
    void flushBuffer(char* b, size_t len);

    int64_t length();

    void close();

    void trunc();

protected:
    void  seekInternal(int64_t pos);

private:
    FILE* fileHandle_;

    std::string filename_;
};


}

NS_IZENELIB_IR_END

#endif

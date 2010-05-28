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
    FSIndexOutput(const char* filename, const string& mode);

    FSIndexOutput(const char* filename, size_t buffsize, const string& mode);

    virtual ~FSIndexOutput(void);

public:
    void flushBuffer(char* b, size_t len);

    void seek(int64_t pos);

    int64_t length();

    void close();

    void trunc();
private:
    FILE* fileHandle_;

    std::string filename_;
};


}

NS_IZENELIB_IR_END

#endif

/**
* @file        FSBufferedOutput.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief
*/
#ifndef FSBUFFEREDOUTPUT_H
#define FSBUFFEREDOUTPUT_H

#include <util/io/BufferedOutput.h>

#include <stdio.h>
#include <string>

using namespace std;

NS_IZENELIB_UTIL_BEGIN

namespace io{

class FSBufferedOutput : public BufferedOutput
{
public:
    FSBufferedOutput(const char* filename, const string& mode);

    FSBufferedOutput(const char* filename, size_t buffsize, const string& mode);

    virtual ~FSBufferedOutput(void);

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

NS_IZENELIB_UTIL_END

#endif

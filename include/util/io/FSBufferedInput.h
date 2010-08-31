/**
* @file        FSBufferedInput.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief
*/
#ifndef FSBUFFEREDINPUT_H
#define FSBUFFEREDINPUT_H

#include <util/io/BufferedInput.h>

#include <fstream>
#include <string>

using namespace std;

NS_IZENELIB_UTIL_BEGIN

namespace io{

class FSBufferedInput : public BufferedInput
{
public:

    FSBufferedInput(const char* filename);

    FSBufferedInput(const char* filename,size_t buffsize);

    virtual ~FSBufferedInput();

public:
    void readInternal(char* b,size_t length,bool bCheck = true);

    BufferedInput* clone();

    void close();

    void seekInternal(int64_t position);

    void reopen();
private:
    FILE* fileHandle_;

    string filename_;

};

}

NS_IZENELIB_UTIL_END

#endif

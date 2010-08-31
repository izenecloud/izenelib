/**
* @file        BufferedOutput.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief
*/
#ifndef BUFFEREDOUTPUT_H
#define BUFFEREDOUTPUT_H

#include <types.h>
#include <util/io/BufferedInput.h>

#define BUFFEREDOUTPUT_BUFFSIZE 524288//32768//4096

NS_IZENELIB_UTIL_BEGIN

namespace io{
///BufferedOutput is a class that is responsible for the index data writing
///It provides basic utilites of encoding variable length data
class BufferedOutput
{
public:
    BufferedOutput(char* buffer,size_t buffsize);

    BufferedOutput(size_t buffsize=0);

    virtual ~BufferedOutput(void);
public:
    void write(BufferedInput* pInput,int64_t length);

    void write(const char* data,size_t length);

    void writeByte(uint8_t b);

    void writeBytes(uint8_t* b, size_t length);

    void writeInt(int32_t i);

    void writeInts(int32_t* pInts,size_t len);

    void writeVInt(int32_t i);

    void writeLong(int64_t i);

    void writeVLong(int64_t i);

    void writeString(const std::string& s);

    void writeChars(const char* s, size_t start, size_t length);

    static uint8_t getVIntLength(int32_t i);

    static uint8_t getVLongLength(int64_t i);

    void setBuffer(char* buf,size_t bufSize);

    int64_t getFilePointer();

    void flush();
public:
    virtual void  flushBuffer(char* b,size_t len) = 0;

    virtual int64_t length() = 0;

    virtual void  close();

    virtual void  seek(int64_t pos);

protected:
    char* buffer_;
    size_t buffersize_;
    int64_t bufferStart_;
    size_t bufferPosition_;
    bool bOwnBuff_;
};


}

NS_IZENELIB_UTIL_END

#endif

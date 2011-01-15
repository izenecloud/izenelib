/**
* @file        IndexInput.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief
*/
#ifndef INDEXINPUT_H
#define INDEXINPUT_H

#include <ir/index_manager/utility/system.h>

#include <string>


#define INDEXINPUT_BUFFSIZE		4096*4//4096

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
///IndexInput is a class that is responsible for the index data reading
///It provides basic utilites of decoding variable length data
class BarrelInfo;
class IndexInput
{
public:
    IndexInput(char* buffer,size_t buffsize);

    IndexInput(size_t buffsize=0);

    virtual ~IndexInput(void);
public:
    void read(char* data, size_t length);

    uint8_t readByte();

    void  readBytes(uint8_t* b,size_t len);

    int32_t readInt();

    void readInts(int32_t* i,size_t len);

    int32_t readVInt();

    int64_t readLong();

    int64_t readVLong();

    int32_t readIntBySmallEndian();

    int32_t readVIntByBigEndian();

    int64_t readLongBySmallEndian();

    void readString(std::string& s);

    void  readChars(char* buffer_, size_t start, size_t length);

    void skipVInt(size_t nNum);

    int64_t getFilePointer();

    void seek(int64_t pos);

    bool isEof();

    int64_t length()const;

    void setlength(int64_t newLen);

    void setBuffer(char* buf,size_t bufSize);

    void reset();

    void setDirty(bool dirty) { dirty_ = dirty; }

    bool isDirty() { return dirty_;}

    void setBarrelInfo(BarrelInfo* pBarrelInfo);
public:
    virtual void readInternal(char* b,size_t length_,bool bCheck = true) = 0;

    virtual void seekInternal(int64_t pos) = 0;

    virtual IndexInput* clone() = 0;

    virtual void close() = 0;
protected:
    void refill();

protected:
    char* buffer_;

    size_t bufferSize_;

    int64_t bufferStart_;		/// position in file of buffer_

    size_t bufferLength_;	/// end of valid bytes

    size_t bufferPosition_;	/// next byte to read

    int64_t length_;			/// set by subclasses

    bool bOwnBuff_;

    bool dirty_;

    BarrelInfo* pBarrelInfo_;
    friend class IndexOutput;
};

/////////////////////////////Big Endian By Default/////////////////////////////////////////////
//
inline uint8_t IndexInput::readByte()
{
    if (bufferPosition_ >= bufferLength_)
        refill();
    return buffer_[bufferPosition_++];
}
inline int32_t IndexInput::readInt()
{
    uint8_t b1 = readByte();
    uint8_t b2 = readByte();
    uint8_t b3 = readByte();
    uint8_t b4 = readByte();
    return ((b1 & 0xFF) << 24) | ((b2 & 0xFF) << 16) | ((b3 & 0xFF) <<  8)
           | (b4 & 0xFF);
}

inline int32_t IndexInput::readVInt()
{
    uint8_t b = readByte();
    int32_t i = b & 0x7F;
    for (int32_t shift = 7; (b & 0x80) != 0; shift += 7)
    {
        b = readByte();
        i |= (b & 0x7FL) << shift;
    }
    return i;
}

inline int64_t IndexInput::readLong()
{
    int32_t i1 = readInt();
    int32_t i2 = readInt();
    return (((int64_t)i1) << 32) | (i2 & 0xFFFFFFFFL);
}

inline int64_t IndexInput::readVLong()
{
    uint8_t b = readByte();
    int64_t i = b & 0x7F;
    for (int32_t shift = 7; (b & 0x80) != 0; shift += 7)
    {
        b = readByte();
        i |= (b & 0x7FLL) << shift;
    }
    return i;
}

inline void IndexInput::readString(std::string& s)
{
    size_t length = (size_t)readVInt();
    char* chars = new char[length + 1];
    readChars(chars, 0, length);
    chars[length] = '\0';
    s = chars;
    delete[] chars;
}

inline void IndexInput::readChars(char* buffer, size_t start, size_t length)
{
    size_t end = start + length;
    for (size_t i = start; i < end; i++)
    {
        uint8_t b = readByte();
        if ((b & 0x80) == 0)
            buffer[i] = (char) (b & 0x7F);
        else if ((b & 0xE0) != 0xE0)
        {
            buffer[i] = (char) (((b & 0x1F) << 6) | (readByte() & 0x3F));
        }
        else
            buffer[i] = (char) (((b & 0x0F) << 12) | ((readByte() & 0x3F) << 6) | (readByte() & 0x3F));
    }
}

/////////////////////////////Small Endian/////////////////////////////////////////////
//
inline int32_t IndexInput:: readIntBySmallEndian()
{
    uint8_t b1 = readByte();
    uint8_t b2 = readByte();
    uint8_t b3 = readByte();
    uint8_t b4 = readByte();
    return ((b4 & 0xFF) << 24) | ((b3 & 0xFF) << 16) | ((b2 & 0xFF) <<  8)
           | (b1 & 0xFF);
}

inline int32_t IndexInput::readVIntByBigEndian()
{
    uint8_t b[5];
    b[0] = readByte();
    uint8_t i = 0, shift = 0;
    for (; (b[i] & 0x80) != 0; shift += 7)
    {
        b[++i] = readByte();
    }
    int32_t r = 0;
    shift -= 7;
    for(uint8_t j = 0; j < i ; ++j, shift -= 7)
    {
        r |= (b[j] & 0x7FL) << shift;
    }

    return r;
}

inline int64_t IndexInput::readLongBySmallEndian()
{
    int32_t i1 = readIntBySmallEndian();
    int32_t i2 = readIntBySmallEndian();
    return (((int64_t)i2) << 32) | (i1 & 0xFFFFFFFFL);
}

inline void IndexInput::refill()
{
    if(dirty_)
        SF1V5_THROW(ERROR_FILEIO,"Index dirty.");

    bufferStart_ += bufferPosition_;
    bufferPosition_ = bufferLength_ = 0;

    if(bufferStart_ >= length_)
        SF1V5_THROW(ERROR_FILEIO,"IndexInput:read past EOF.");

    if(bufferStart_ + static_cast<int64_t>(bufferSize_) > length_)
        bufferLength_ = length_ - bufferStart_;
    else
        bufferLength_ = bufferSize_;

    if (buffer_ == NULL)
        buffer_ = new char[bufferSize_]; // allocate buffer_ lazily

    readInternal(buffer_,bufferLength_);
}
inline void IndexInput::skipVInt(size_t nNum)
{
    for (size_t i = 0;i<nNum;i++)
    {
        readVInt();
    }
}

inline int64_t IndexInput::getFilePointer()
{
    return bufferStart_ + (int64_t)bufferPosition_;
}

inline void IndexInput::seek(int64_t pos)
{
    if (pos > length_)
        SF1V5_THROW(ERROR_FILEIO,"IndexInput.seek():pos>length_");
    if (pos >= bufferStart_ && pos < (bufferStart_ + (int64_t)bufferLength_))
        bufferPosition_ = (size_t) (pos - bufferStart_);
    else
    {
        bufferStart_ = pos;
        bufferPosition_ = 0;
        bufferLength_ = 0; // trigger refill() on read()
        seekInternal(pos);
    }
}

inline bool IndexInput::isEof()
{
    return ( (bufferStart_ + (int64_t )bufferPosition_) >= length_);
}

inline int64_t IndexInput::length()const
{
    return length_;
}

inline void IndexInput::setlength(int64_t newLen)
{
    length_ = newLen;
}

}

NS_IZENELIB_IR_END

#endif

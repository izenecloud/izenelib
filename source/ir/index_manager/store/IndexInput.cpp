#include <ir/index_manager/store/IndexInput.h>
#include <ir/index_manager/index/BarrelInfo.h>

using namespace izenelib::ir::indexmanager;

IndexInput::IndexInput(char* buf,size_t buffsize)
{

    buffer_ = buf;
    bufferSize_ = buffsize;
    bOwnBuff_ = false;
    dirty_ = false;
    pBarrelInfo_ = 0;

    bufferStart_ = 0;
    bufferLength_ = 0;
    bufferPosition_ = 0;

    length_ = 0;
}
IndexInput::IndexInput(size_t buffsize)
{
    buffer_ = NULL;
    bufferSize_ = buffsize > 0 ? buffsize : INDEXINPUT_BUFFSIZE;

    bOwnBuff_ = true;
    dirty_ = false;
    pBarrelInfo_ = 0;

    bufferStart_ = 0;
    bufferLength_ = 0;
    bufferPosition_ = 0;

    length_ = 0;
}

IndexInput::~IndexInput(void)
{
    if (bOwnBuff_)
    {
        if (buffer_)
        {
            delete[]buffer_;
            buffer_ = NULL;
        }
    }
    if(pBarrelInfo_)
      pBarrelInfo_->unRegisterIndexInput(this);
}

void IndexInput::reset()
{
    bufferStart_ = 0;
    bufferLength_ = 0;
    bufferPosition_ = 0;
}

void IndexInput::setBarrelInfo(BarrelInfo* pBarrelInfo)
{
    pBarrelInfo_ = pBarrelInfo;
    pBarrelInfo_->registerIndexInput(this);
}

void IndexInput::read(char* data, size_t length)
{
    if(dirty_)
    {
        SF1V5_THROW(ERROR_FILEIO,"Index dirty.");
    }

    if(length == 0)
        return;

    if (bufferPosition_ >= (size_t)bufferLength_)
        refill();
    if (length <= (bufferLength_ - bufferPosition_))
    {
        memcpy(data,buffer_ + bufferPosition_,length);
        bufferPosition_ += length;
    }
    else
    {
        size_t start = bufferLength_ - bufferPosition_;
        if (start > 0)
        {
            memcpy(data,buffer_ + bufferPosition_,start);
        }

        readInternal(data + start,length - start);
        bufferStart_ += bufferPosition_ + length;
        bufferPosition_ = bufferLength_ = 0;
    }
}
void IndexInput::readBytes(uint8_t* b,size_t len)
{
    if(dirty_)
    {
        SF1V5_THROW(ERROR_FILEIO,"Index dirty.");
    }

    if (len < bufferSize_)
    {
        for (size_t i = 0; i < len; i++)
            // read byte-by-byte
            b[i] = (uint8_t) readByte();
    }
    else
    {
        // read all-at-once
        int64_t start = getFilePointer();
        seekInternal(start);
        readInternal((char*)b,len);

        bufferStart_ = start + len; // adjust stream variables
        bufferPosition_ = 0;
        bufferLength_ = 0; // trigger refill() on read
    }
}
void IndexInput::readInts(int32_t* i,size_t len)
{
    for (size_t l = 0;l < len;l++)
        i[l] = readInt();
}

void IndexInput::setBuffer(char* buf,size_t bufSize)
{
    if (bufferPosition_ != 0)
    {
        SF1V5_THROW(ERROR_UNSUPPORTED," void IndexInput::setBuffer(char* buf,size_t bufSize):you must call setBuffer() before reading any data.");
    }
    if (bOwnBuff_ && buffer_)
    {
        delete[] buffer_;
    }
    buffer_ = buf;
    bufferSize_ = bufSize;
    bOwnBuff_ = false;
}


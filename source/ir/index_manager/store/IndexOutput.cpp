#include <ir/index_manager/store/IndexOutput.h>

using namespace std;

using namespace izenelib::ir::indexmanager;

IndexOutput::IndexOutput(char* buf,size_t buffsize)
{
    if (buffer_)
    {
        buffer_ = buf;
        buffersize_ = buffsize;
        bOwnBuff_ = false;
    }
    else
    {
        SF1V5_THROW(ERROR_ILLEGALARGUMENT,":IndexOutput(char* buffer_,size_t buffsize)");
    }
    bufferStart_ = 0;
    bufferPosition_ = 0;
}

IndexOutput::IndexOutput(size_t buffsize)
{
    try
    {
        if (buffsize > 0)
        {
            buffer_ = new char[buffsize];
            buffersize_ = buffsize;
        }
        else
        {
            buffer_ = new char[INDEXOUTPUT_BUFFSIZE];
            buffersize_ = INDEXOUTPUT_BUFFSIZE;
        }

        bOwnBuff_ = true;

        bufferStart_ = 0;
        bufferPosition_ = 0;

    }

    catch (std::bad_alloc& be)
    {
        SF1V5_THROW(ERROR_OUTOFMEM,be.what());
    }

}


IndexOutput::~IndexOutput(void)
{
    if (bOwnBuff_)
    {
        if (buffer_)
        {
            delete[]buffer_;
            buffer_ = NULL;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//
void IndexOutput::write(const char* data,size_t length)
{
    if ((bufferPosition_>0) && ( (int64_t)(bufferPosition_ + length) >= (int64_t)buffersize_) )
        flush();
    if ((int64_t)buffersize_ < (int64_t)length)
    {
        flushBuffer((char*)data,length);
        bufferStart_+=length;
    }
    else
    {
        memcpy(buffer_ + bufferPosition_,data,length);
        bufferPosition_ += length;
    }
}
void IndexOutput::writeByte(uint8_t b)
{
    if (bufferPosition_ >= buffersize_)
        flush();
    buffer_[bufferPosition_++] = b;
}

void IndexOutput::writeBytes(uint8_t* b, size_t length)
{
    for (size_t i= 0; i < length; i++)
        writeByte(b[i]);
}

void  IndexOutput::writeInt(int32_t i)
{
    writeByte((uint8_t) (i >> 24));
    writeByte((uint8_t) (i >> 16));
    writeByte((uint8_t) (i >> 8));
    writeByte((uint8_t) i);
}
void IndexOutput::writeInts(int32_t* pInts,size_t len)
{
    for (size_t i= 0; i < len; i++)
        writeInt(pInts[i]);
}

void  IndexOutput::writeVInt(int32_t i)
{
    uint32_t ui = i;
    while ((ui & ~0x7F) != 0)
    {
        writeByte((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
    }
    writeByte( (uint8_t)ui );
}

void  IndexOutput::writeLong(int64_t i)
{
    writeInt((int32_t) (i >> 32));
    writeInt((int32_t) i);
}

void  IndexOutput::writeVLong(int64_t i)
{
    uint64_t ui = i;
    while ((ui & ~0x7F) != 0)
    {
        writeByte((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
    }
    writeByte((uint8_t)ui);
}
void  IndexOutput::writeString(const string & s)
{
    int32_t length = (int32_t)s.length();
    writeVInt(length);
    writeChars(s.c_str(), 0, length);
}
void  IndexOutput::writeChars(const char* s, size_t start, size_t length)
{
    uint64_t end = start + length;
    for (size_t i = start; i < end; i++)
    {
        int32_t code = (int32_t) s[i];
        if (code >= 0x01 && code <= 0x7F)
            writeByte((uint8_t) code);
        else if (((code >= 0x80) && (code <= 0x7FF)) || code == 0)
        {
            writeByte((uint8_t) (0xC0 | (code >> 6)));
            writeByte((uint8_t) (0x80 | (code & 0x3F)));
        }
        else
        {
            writeByte((uint8_t) (0xE0 | (((uint32_t) code) >> 12)));
            writeByte((uint8_t) (0x80 | ((code >> 6) & 0x3F)));
            writeByte((uint8_t) (0x80 | (code & 0x3F)));
        }
    }
}

uint8_t IndexOutput::getVIntLength(int32_t i)
{
    uint8_t l = 1;
    uint32_t ui = i;
    while ((ui & ~0x7F) != 0)
    {
        l++;
        ui >>= 7; //doing unsigned shift
    }
    return l;
}

uint8_t IndexOutput::getVLongLength(int64_t i)
{
    uint8_t l = 1;
    uint32_t ui = i;
    while ((ui & ~0x7F) != 0)
    {
        l++;
        ui >>= 7; //doing unsigned shift
    }
    return l;
}

void IndexOutput::setBuffer(char* buf,size_t bufSize)
{
    if (bufferStart_!=0 || bufferPosition_ != 0)
    {
        SF1V5_THROW(ERROR_UNSUPPORTED," void IndexOutput::setBuffer(char* buf,size_t bufSize):you must call setBuffer() before reading any data.");
    }
    if (bOwnBuff_ && buffer_)
    {
        delete[] buffer_;
    }
    buffer_ = buf;
    buffersize_ = bufSize;
    bOwnBuff_ = false;
}
void IndexOutput::flush()
{
    flushBuffer(buffer_, bufferPosition_);
    bufferStart_ += bufferPosition_;
    bufferPosition_ = 0;
}

int64_t IndexOutput::getFilePointer()
{
    return bufferStart_ + (int64_t)bufferPosition_;
}

int64_t IndexOutput::getLength()
{
    return bufferStart_ + (int64_t)bufferPosition_;
}

void IndexOutput::write(IndexInput* pInput,int64_t length)
{
    if ( (bufferPosition_ + length) >= buffersize_)
        flush();
    if (length <= (int64_t)(buffersize_ - bufferPosition_) )
    {
        pInput->readBytes((uint8_t*)(buffer_ + bufferPosition_),(size_t)length);
        bufferPosition_ += (size_t)length;
    }
    else
    {
        int64_t n=0;
        size_t nwrite=0;
        while (n < length)
        {
            nwrite = buffersize_;
            if ( (length - n) < (int64_t)nwrite)
                nwrite = (size_t)(length - n);

            //pInput->readInternal(buffer_,nwrite);
            //pInput->seek(pInput->getFilePointer() + nwrite);
            pInput->readBytes((uint8_t*)buffer_,(size_t)nwrite);

            if (nwrite == buffersize_)
            {
                bufferPosition_ = nwrite;
                flush();
            }

            else bufferPosition_ += nwrite;

            n += nwrite;
        }
    }

}

void IndexOutput::seek(int64_t pos)
{
    // avoid to seek operation if already pointing to destination
    if(getFilePointer() == pos)
        return;

    flush();
    bufferStart_ = pos;

    seekInternal(pos);
}

void IndexOutput::close()
{
    flush();
    bufferStart_ = 0;
    bufferPosition_ = 0;
}


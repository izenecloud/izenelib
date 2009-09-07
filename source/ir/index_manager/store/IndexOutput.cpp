#include <ir/index_manager/store/IndexOutput.h>

using namespace std;

using namespace izenelib::ir::indexmanager;

IndexOutput::IndexOutput(char* buf,size_t buffsize)
{
    if (buffer)
    {
        buffer = buf;
        buffersize = buffsize;
        bOwnBuff = false;
    }
    else
    {
        SF1V5_THROW(ERROR_ILLEGALARGUMENT,":IndexOutput(char* buffer,size_t buffsize)");
    }
    bufferStart = 0;
    bufferPosition = 0;
}

IndexOutput::IndexOutput(size_t buffsize)
{
    try
    {
        if (buffsize > 0)
        {
            buffer = new char[buffsize];
            buffersize = buffsize;
        }
        else
        {
            buffer = new char[INDEXOUTPUT_BUFFSIZE];
            buffersize = INDEXOUTPUT_BUFFSIZE;
        }

        bOwnBuff = true;

        bufferStart = 0;
        bufferPosition = 0;

    }

    catch (std::bad_alloc& be)
    {
        SF1V5_THROW(ERROR_OUTOFMEM,be.what());
    }

}


IndexOutput::~IndexOutput(void)
{
    if (bOwnBuff)
    {
        if (buffer)
        {
            delete[]buffer;
            buffer = NULL;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//
void IndexOutput::write(const char* data,size_t length)
{
    if ((bufferPosition>0) && ( (int64_t)(bufferPosition + length) >= (int64_t)buffersize) )
        flush();
    if ((int64_t)buffersize < (int64_t)length)
    {
        flushBuffer((char*)data,length);
        bufferStart+=length;
    }
    else
    {
        memcpy(buffer + bufferPosition,data,length);
        bufferPosition += length;
    }
}
void IndexOutput::writeByte(uint8_t b)
{
    if (bufferPosition >= buffersize)
        flush();
    buffer[bufferPosition++] = b;
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
    if (bufferStart!=0 || bufferPosition != 0)
    {
        SF1V5_THROW(ERROR_UNSUPPORTED," void IndexOutput::setBuffer(char* buf,size_t bufSize):you must call setBuffer() before reading any data.");
    }
    if (bOwnBuff && buffer)
    {
        delete[] buffer;
    }
    buffer = buf;
    buffersize = bufSize;
    bOwnBuff = false;
}
void IndexOutput:: flush()
{
    flushBuffer(buffer, bufferPosition);
    bufferStart += bufferPosition;
    bufferPosition = 0;
}
int64_t IndexOutput::getFilePointer()
{
    return bufferStart + (int64_t)bufferPosition;
}



void IndexOutput::write(IndexInput* pInput,int64_t length)
{
    if ( (bufferPosition + length) >= buffersize)
        flush();
    if (length <= (int64_t)(buffersize - bufferPosition) )
    {
        pInput->readBytes((uint8_t*)(buffer + bufferPosition),(size_t)length);
        bufferPosition += (size_t)length;
    }
    else
    {
        int64_t n=0;
        size_t nwrite=0;
        while (n < length)
        {
            nwrite = buffersize;
            if ( (length - n) < (int64_t)nwrite)
                nwrite = (size_t)(length - n);

            pInput->readInternal(buffer,nwrite);
            pInput->seek(pInput->getFilePointer() + nwrite);

            if (nwrite == buffersize)
            {
                bufferPosition = nwrite;
                flush();
            }

            else bufferPosition += nwrite;

            n += nwrite;
        }
    }

}

void IndexOutput::seek(int64_t pos)
{
    flush();
    bufferStart = pos;
}

void IndexOutput::close()
{
    flush();
    bufferStart = 0;
    bufferPosition = 0;
}


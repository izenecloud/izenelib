#include <util/io/BufferedOutput.h>
#include <util/io/FSBufferedOutput.h>

#include <stdio.h>

using namespace std;

NS_IZENELIB_UTIL_BEGIN
namespace io{

FSBufferedOutput::FSBufferedOutput(const char* filename, const string& mode)
    :BufferedOutput(0)
    ,filename_(filename)
{
    if (mode.compare("w+b") == 0)
        fileHandle_ = fopen(filename, "w+b");
    else if(mode.compare("r+") == 0)
    {
        fileHandle_ = fopen(filename, "r+");
        if(fileHandle_ == NULL)
            fileHandle_ = fopen(filename, "a+");
    }
    else if(mode.compare("a+") == 0)
        fileHandle_ = fopen(filename, "a+");
    else
        throw std::runtime_error(
            "Open file error: " + filename_);

    if (fileHandle_ == NULL)
    {
        throw std::runtime_error(
            "Open file error: " + filename_);
    }

    //setbuf(fileHandle_,NULL);
}

FSBufferedOutput::FSBufferedOutput(const char* filename, size_t buffsize, const string& mode)
    :BufferedOutput(buffsize)
    ,filename_(filename)
{
    if (mode.compare("w+b") == 0)
        fileHandle_ = fopen(filename, "w+b");
    else if(mode.compare("r+") == 0)
    {
        fileHandle_ = fopen(filename, "r+");
        if(fileHandle_ == NULL)
            fileHandle_ = fopen(filename, "a+");
    }
    else if(mode.compare("a+") == 0)
        fileHandle_ = fopen(filename, "a+");
    else
        throw std::runtime_error(
            "Open file error: " + filename_);

    if (fileHandle_ == NULL)
    {
        throw std::runtime_error(
             "Open file error: " + filename_);
    }

    //setbuf(fileHandle_,NULL);
}

FSBufferedOutput::~FSBufferedOutput()
{
    close();
}

void FSBufferedOutput::flushBuffer(char* b, size_t len)
{
    size_t ret = fwrite(b, 1, len, fileHandle_);
    fflush(fileHandle_);
    if (len > 0 && len != ret)
    {
        close();
        throw std::runtime_error("FSBufferedOutput::flushBuffer():file IO write error:");
    }
}

void FSBufferedOutput::seek(int64_t pos)
{
    BufferedOutput::seek(pos);
    if (0 != fseek(fileHandle_, pos, SEEK_SET))
    {
        close();
        throw std::runtime_error("FSBufferedOutput::seek():file IO seek error.");
    }
}

int64_t FSBufferedOutput::length()
{
    return bufferStart_ + (int64_t)bufferPosition_;
}

void FSBufferedOutput::close()
{
    if (fileHandle_)
    {
        BufferedOutput::close();
        fclose(fileHandle_);
        fileHandle_ = NULL;
    }
}

void FSBufferedOutput::trunc()
{
    if(fileHandle_)
    {
        close();
    }
    fileHandle_ = fopen(filename_.c_str(), "w+b");
}

}

NS_IZENELIB_UTIL_END


#include <util/io/FSBufferedInput.h>

NS_IZENELIB_UTIL_BEGIN
namespace io{

FSBufferedInput::FSBufferedInput(const char* filename)
{
    fileHandle_ = fopen(filename, "rb");

    if (fileHandle_ == NULL)
    {
        perror("error when opening file");
        perror(filename);
        string sFileName = filename;
        throw std::runtime_error(
            "Open file error: " + sFileName);
    }
    fseek(fileHandle_, 0, SEEK_END);
    length_ = ftell(fileHandle_);
    rewind(fileHandle_);

    filename_ = filename;
}

FSBufferedInput::FSBufferedInput(const char* filename,size_t buffsize)
        :BufferedInput(buffsize)
{
    fileHandle_ = fopen(filename, "rb");

    if (fileHandle_ == NULL)
    {
        perror("error when opening file");
        string sFileName = filename;
        throw std::runtime_error(
            "Open file error: " + sFileName);
    }

    fseek(fileHandle_, 0, SEEK_END);
    length_ = ftell(fileHandle_);
    rewind(fileHandle_);

    filename_ = filename;
}

FSBufferedInput::~FSBufferedInput()
{
    close();
}

void FSBufferedInput::readInternal(char* b,size_t length,bool bCheck/* = true*/)
{
    int ret = fread(b, 1, length, fileHandle_);
    if (ret != (int)length)
        if (!feof(fileHandle_))
        {
            close();
            throw std::runtime_error("FSBufferedInput::readInternal():file IO read error:" + filename_);
        }
}

void FSBufferedInput::close()
{
    if (fileHandle_ )
    {
        fclose(fileHandle_);
        fileHandle_ = NULL;
    }

}

BufferedInput* FSBufferedInput::clone()
{
    FSBufferedInput* pClone = new FSBufferedInput(filename_.c_str(),bufferSize_);
    pClone->seek(getFilePointer());
    return pClone;
}
void FSBufferedInput::seekInternal(int64_t position)
{
    if (0 != fseek(fileHandle_, position, SEEK_SET))
    {
        close();
        throw std::runtime_error("FSBufferedInput::seekInternal():file IO seek error: " + filename_);
    }
}

void FSBufferedInput::reopen()
{
    BufferedInput::reset();
    fclose(fileHandle_);
    fileHandle_ = fopen(filename_.c_str(), "rb");
    fseek(fileHandle_, 0, SEEK_END);
    length_ = ftell(fileHandle_);
    rewind(fileHandle_);
}

}

NS_IZENELIB_UTIL_END


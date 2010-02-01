#include <ir/index_manager/store/FSIndexInput.h>
#include <ir/index_manager/utility/Utilities.h>

using namespace izenelib::ir::indexmanager;

FSIndexInput::FSIndexInput(const char* filename)
{
    fileHandle_ = fopen(filename, "rb");
    //setbuf(fileHandle_,NULL);

    if (fileHandle_ == NULL)
    {
        perror("error when opening file");
        string sFileName = filename;
        SF1V5_THROW(ERROR_FILEIO,"Open file error: " + sFileName);
    }
    fseek(fileHandle_, 0, SEEK_END);
    length_ = ftell(fileHandle_);
    rewind(fileHandle_);

    filename_ = filename;
}

FSIndexInput::FSIndexInput(const char* filename,size_t buffsize)
        :IndexInput(buffsize)
{
    fileHandle_ = fopen(filename, "rb");
    //setbuf(fileHandle_,NULL);

    if (fileHandle_ == NULL)
    {
        perror("error when opening file");
        string sFileName = filename;
        SF1V5_THROW(ERROR_FILEIO,"Open file error: " + sFileName);
    }

    fseek(fileHandle_, 0, SEEK_END);
    length_ = ftell(fileHandle_);
    rewind(fileHandle_);

    filename_ = filename;
}

FSIndexInput::~FSIndexInput()
{
    close();
}

void FSIndexInput::readInternal(char* b,size_t length,bool bCheck/* = true*/)
{
    if (bCheck)
    {
        int64_t position = getFilePointer();
        if (0 != fseek(fileHandle_, 0, SEEK_CUR))
        {
            if (0 != fseek(fileHandle_, position, SEEK_SET))
            {
                close();
                SF1V5_THROW(ERROR_FILEIO,"FSIndexInput::readInternal():file IO seek error: " + filename_);
            }
        }
    }
    int ret = fread(b, 1, length, fileHandle_);
    if (ret != (int)length)
        if (!feof(fileHandle_))
        {
            close();
            SF1V5_THROW(ERROR_FILEIO,"FSIndexInput::readInternal():file IO read error:" + filename_);
        }
}

void FSIndexInput::close()
{
    if (fileHandle_ )
    {
        fclose(fileHandle_);
        fileHandle_ = NULL;
    }

}

IndexInput* FSIndexInput::clone()
{
    FSIndexInput* pClone = new FSIndexInput(filename_.c_str(),bufferSize_);
    pClone->seek(getFilePointer());
    return pClone;
}
void FSIndexInput::seekInternal(int64_t position)
{
    if (0 != fseek(fileHandle_, position, SEEK_SET))
    {
        close();
        SF1V5_THROW(ERROR_FILEIO,"FSIndexInput::seekInternal():file IO seek error: " + filename_);
    }
}

